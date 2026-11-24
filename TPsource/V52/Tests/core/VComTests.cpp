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

// H-024: relay tark_trk (static, Juk/VCom.cpp:680-720) enters
// tall_CriticalSection at :690 and, in announcer mode (kuuluttaja) with an
// 'I' status for a leg already marked K or H, returns at :701 without
// leaving it. Reached through the public receive loop tarkcom (:893), which
// production runs on its own thread (Tp/TpComY.cpp:2415); one TARKT packet
// is injected into the receive queue. Plan: testaus.md sections 6 and 14.

#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include <process.h>
#include "VDeclare.h"
#include "../fixtures/VFixture.h"

extern combufrec *inbuf;          // Juk/VCom.cpp:93
static const int TP_TARKT = 5;    // Juk/VCom.cpp:68 (local define)

// Team 12 in record 1 with leg 1 marked 'K' (keskeyttanyt / did not finish).
static void trkFixture()
{
	REQUIRE(tp::sandboxOk());
	tpv::initBase();
	tpv::computeRecordSizes();
	kuuluttaja = true;
	REQUIRE(tp::fileSize(dataf2name) == -1);
	REQUIRE(makefile(&datf2, dataf2name, kilpparam.kilprecsize) == 0);
	kilparr = (char *) calloc(4, kilpparam.kilprecsize);
	REQUIRE(kilparr != NULL);
	kilptietue k;
	k.nollaa();
	k.kilpno = 12;
	k.SetTark(0, 'K');
	char buf[KILPRECSIZE];
	DATAREF d = 0;
	addrec(&datf2, &d, k.pack(buf));
	REQUIRE(d == 1);
	PUTREC(&k, 1);
	kilptietue g;
	GETREC(&g, 1);
	REQUIRE(g.kilpstatus == 0);
	REQUIRE(g.kilpno == 12);
	REQUIRE(g.wTark(0) == L'K');
	inbuf = (combufrec *) calloc(INBUFL + 1, sizeof(combufrec));   // as Tp/TpComY.cpp:2399
	REQUIRE(inbuf != NULL);
}

static void injectTarkt(int kno, char keskhyl)
{
	combufrec *c = inbuf + 0;
	c->pkgclass = (UCHAR) TP_TARKT;
	c->portti = 1;
	c->d.trk.dk = 1;
	c->d.trk.kno = (INT16) kno;
	c->d.trk.osuus = 1;
	c->d.trk.keskhyl = keskhyl;
	c->d.trk.sakko = 0;
	c->d.trk.tarf = 0;
	c->d.trk.pakota = 0;
	inbens = 0;
	inbseur = 1;
	comfl = 1;
}

static void runTarkcom(void *)
{
	tarkcom(NULL);
}

// Stops the receive loop once the injected packet has been consumed
// (inbens advanced, Juk/VCom.cpp:981) or after 5 s.
static volatile LONG g_consumed = 0;

static unsigned __stdcall stopper(void *)
{
	for (int i = 0; i < 500 && inbens != 1; i++)
		Sleep(10);
	InterlockedExchange(&g_consumed, inbens == 1 ? 1 : 0);
	comfl = 0;
	return 0;
}

// Runs tarkcom on a controlled worker thread until the packet is consumed.
static tp::ThreadRun processOnePacket()
{
	HANDLE hs = (HANDLE) _beginthreadex(NULL, 0, stopper, NULL, 0, NULL);
	REQUIRE(hs != 0);
	tp::ThreadRun r = tp::runOnThread(runTarkcom, NULL, 10000, "tarkcom");
	WaitForSingleObject(hs, 10000);
	CloseHandle(hs);
	return r;
}

// Control: a TARKT packet whose bib does not match the record (13 vs 12)
// takes the er = 1 path, which leaves the lock (:706) and reports the
// mismatch. Shows the injection, the worker and the probe work.
TEST_CASE("H-024 control: mismatching TARKT packet releases the lock")
{
	trkFixture();
	CHECK(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);
	injectTarkt(13, 'I');
	size_t m0 = tp::uiMessages().size();
	tp::ThreadRun r = processOnePacket();
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	REQUIRE(g_consumed == 1);
	CHECK(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);
	CHECK(tp::uiMessages().size() == m0 + 2);   // the two mismatch messages
}

// Defect: with kuuluttaja on, a TARKT 'I' packet for a leg marked 'K'
// returns at :701 holding tall_CriticalSection. Correct: the lock is free
// after the packet has been processed. The worker has ended normally
// (comfl cleared), so the lock is orphaned.
TEST_CASE("H-024 relay tark_trk announcer path leaves tall_CriticalSection held")
{
	trkFixture();
	REQUIRE(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);
	injectTarkt(12, 'I');
	size_t m0 = tp::uiMessages().size();
	tp::ThreadRun r = processOnePacket();
	REQUIRE(r.outcome == tp::THREAD_FINISHED);   // the loop itself was not blocked
	REQUIRE(g_consumed == 1);                     // the packet was processed
	REQUIRE(tp::uiMessages().size() == m0);       // not the mismatch path
	int probe = tp::lockFreeForOtherThread(&tall_CriticalSection);
	REQUIRE(probe >= 0);
	DWORD owner = tp::lockOwnerThread(&tall_CriticalSection);
	REQUIRE((probe == 0 ? owner == r.threadId : owner == 0));
	kilptietue g;
	GETREC(&g, 1);
	REQUIRE(g.wTark(0) == L'K');   // returned at :701, before SetTark/tallenna
	bool observed = probe == 0 && owner == r.threadId;
	tp::reportResult("H-024", "tark-trk-announcer-path-leaves-lock-held", observed,
		tp::fmt("packet consumed; worker ended in %lu ms; other thread got lock=%d; owner=%lu "
			"worker=%lu (worker ended: lock orphaned); leg 1 status still %c",
			(unsigned long) r.elapsedMs, probe, (unsigned long) owner, (unsigned long) r.threadId,
			(char) g.wTark(0)));
}
