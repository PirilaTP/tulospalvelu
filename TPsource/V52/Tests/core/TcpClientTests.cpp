// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
// Copyright (C) 2015 Pekka Pirila

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// H-027: the TCP client thread tiedonsiirtoTCP (Tp/TpComY.cpp:1257) gives up
// after one failed connection: in opencomTCP
// err = openportTCP(...) && (comtype[cn] == comtpTCP) (:1179) is 0 or 1, so
// the retry branch for WSAECONNREFUSED (:1201-1203) is never taken. For
// TCPXML err is 0 even on failure, comopen becomes TRUE and the loop calls
// reconnectTCP on a freed connection (com/WinUDP.cpp:812-817, :848): a NULL
// dereference. The Tp/ code is shared by HK and relay; the test runs in
// TpCoreHk. Client only, 127.0.0.1 only, no production network.
// Plan: doc/arkkitehtuuri/testaus.md sections 5.5 and 14.

#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include "HkDeclare.h"
#include "../fixtures/HkFixture.h"
#include <process.h>

extern unsigned int TCP_on[];   // Tp/TpComY.cpp:138, written by openportTCP
static const int WSAECONNREFUSED_VALUE = 10061;   // winsock2.h WSAECONNREFUSED

static int g_cn = 0;

static unsigned __stdcall clientThread(void *)
{
	tp::setThreadTag("tcp-client");
	tiedonsiirtoTCP(&g_cn);
	return 0;
}

static void clientFixture(int type, unsigned short port)
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	inLopetus = 0;
	keyclose[0] = 0;
	hComm[0] = 0;
	comopen[0] = 0;
	comtype[0] = type;
	memset(&ipparam[0], 0, sizeof(ipparam[0]));
	wcscpy(ipparam[0].destaddr, L"127.0.0.1");   // non-empty: client
	ipparam[0].destport = port;
	ipparam[0].srvport = 0;
	comfl = 1;
}

// Control: the attempt-counting listener sees and rejects a connection in
// phase 1 and accepts one in phase 2. Shows that the counts in the defect
// test mean real connection attempts.
TEST_CASE("H-027 control: attempt listener counts and rejects or accepts")
{
	REQUIRE(tp::sandboxOk());
	unsigned short port = tp::startAttemptListener();
	REQUIRE(port != 0);
	int results[2];
	for (int phase = 1; phase <= 2; phase++) {
		tp::attemptListenerAccept(phase == 2);
		results[phase - 1] = tp::loopbackConnect(port);
		Sleep(200);
	}
	tp::stopAttemptListener();
	tp::info(tp::fmt("phase1 connect=%d phase2 connect=%d attempts1=%d attempts2=%d", results[0],
		results[1], tp::attemptListenerCount(1), tp::attemptListenerCount(2)));
	// One rejected connect() may show as several SYNs in phase 1 (TCP-level
	// retransmission); in phase 2 the first SYN is accepted, so the phase 2
	// count equals the number of application connect() calls.
	CHECK(results[0] == WSAECONNREFUSED_VALUE);   // what the correct code retries on
	CHECK(results[1] == 0);
	CHECK(tp::attemptListenerCount(1) >= 1);
	CHECK(tp::attemptListenerCount(2) == 1);
}

// (a) comtpTCP. Phase 1: the listener rejects, so the client's first
// attempt fails; (i) the attempt was made and failed: the listener saw
// SYNs (one connect() may retransmit), the connection slot was allocated
// (hComm[0] == 1, com/WinUDP.cpp:737), the address was resolved (TCP_on[0],
// :768) and comopen[0] stayed FALSE. (ii) Then the listener accepts for 15 s, longer than
// two retry periods of the correct code (about 4-6 s, :1217). Defect: the
// client thread has ended and makes no further attempt. Correct: the
// thread is still running and connects in phase 2.
TEST_CASE("H-027a TCP client does not retry after a failed connect")
{
	unsigned short port = tp::startAttemptListener();
	REQUIRE(port != 0);
	clientFixture(comtpTCP, port);
	HANDLE h = (HANDLE) _beginthreadex(NULL, 0, clientThread, NULL, 0, NULL);
	REQUIRE(h != 0);
	DWORD t0 = GetTickCount();
	DWORD w = WaitForSingleObject(h, 10000);
	DWORD endedMs = GetTickCount() - t0;
	bool threadEnded = w == WAIT_OBJECT_0;
	int attempts1 = tp::attemptListenerCount(1);
	// (i) the attempt was made and failed (preconditions, not part of the
	// verdict): SYNs seen, slot allocated, address resolved, link not open.
	REQUIRE(attempts1 >= 1);
	REQUIRE(hComm[0] == (HANDLE) 1);
	REQUIRE(TCP_on[0] == 0x0100007F);        // 127.0.0.1 in network byte order
	int comopenAfterFailure = comopen[0];
	tp::attemptListenerAccept(true);
	Sleep(15000);
	int attempts2 = tp::attemptListenerCount(2);
	int comopenAfter = comopen[0];
	if (!threadEnded) {                      // correct behaviour: stop it
		comfl = 0;
		WaitForSingleObject(h, 10000);
	}
	CloseHandle(h);
	tp::stopAttemptListener();
	bool observed = comopenAfterFailure == 0 && threadEnded && attempts2 == 0;
	tp::reportResult("H-027", "tcp-client-no-retry", observed,
		tp::fmt("(i) SYNs seen while rejecting=%d, comopen after failure=%d, hComm0=%d, TCP_on0=0x%08X; "
			"(ii) client thread ended=%d after %lu ms; attempts in the following 15 s while accepting=%d; "
			"comopen at end=%d", attempts1, comopenAfterFailure, (int) (INT_PTR) hComm[0], TCP_on[0],
			threadEnded ? 1 : 0, (unsigned long) endedMs, attempts2, comopenAfter));
}

// (b) comtpTCPXML to a refused loopback port. Defect mechanism: a read
// access violation at the address of TCPyhteys[yhtno]->ipparam with
// TCPyhteys[yhtno] == NULL (offset 0x0C, com/WinUDP.cpp:91-96) in
// reconnectTCP, on the client thread. The vectored exception handler
// (TpCrash.cpp) prints the evidence; the runner checks it and the exit code.
TEST_CASE("H-027b TCPXML client dereferences a freed connection")
{
	unsigned short port = tp::holdRefusingLoopbackPort();   // bound, not listening: refused
	REQUIRE(port != 0);
	clientFixture(comtpTCPXML, port);
	tp::ExceptionExpect e;
	e.code = EXCEPTION_ACCESS_VIOLATION;
	e.access = 0;
	e.target = 0x0C;
	e.threadTag = "tcp-client";
	e.symbol = "reconnectTCP";
	REQUIRE(tp::installExceptionEvidence(e));
	tp::armCrash("H-027", "tcpxml-reconnect-null-deref", "exception");
	HANDLE h = (HANDLE) _beginthreadex(NULL, 0, clientThread, NULL, 0, NULL);
	REQUIRE(h != 0);
	// The fault is expected within a few seconds (connect refusal about
	// 2 s, then the loop). If the process is still alive after 30 s:
	DWORD w = WaitForSingleObject(h, 30000);
	tp::disarmCrash();
	comfl = 0;
	if (w != WAIT_OBJECT_0)
		WaitForSingleObject(h, 10000);
	CloseHandle(h);
	tp::releaseHeldPort();
	tp::reportResult("H-027", "tcpxml-reconnect-null-deref", false,
		tp::fmt("no access violation within 30 s; comopen0=%d hComm0=%d", comopen[0],
			(int) (INT_PTR) hComm[0]));
}
