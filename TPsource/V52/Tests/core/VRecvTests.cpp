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

// Relay receive-queue tests (TpCoreJuk):
//  H-032: the UDP length check (Tp/TpComY.cpp:1001) is skipped for packet
//    classes whose class_len is 0 (7, 11, 12 in the relay), so a packet of
//    any length up to the read size reaches addseur (:629), which copies
//    len+14 bytes (:654) into a receive slot of sizeof(combufrec) bytes
//    (inbuf = calloc(INBUFL+1, sizeof(combufrec)), :2399).
//  H-026: the relay receive loop tarkcom (Juk/VCom.cpp:893) has no case for
//    those classes; the default branch (:971-974) sets lisa = 0, so inbens
//    never advances (:981) and the inner loop, which does not check comfl,
//    processes the same packet forever: receiving stops permanently.
// Plans: doc/arkkitehtuuri/testaus.md sections 6 and 15.

#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include <process.h>
#include "VDeclare.h"
#include "../fixtures/VFixture.h"

extern int class_len[];         // Juk/VCom.cpp:105
extern int combufsize;          // Juk/VCom.cpp:95
extern int alkut;               // Juk/VCom.cpp:84
extern combufrec *inbuf;        // Juk/VCom.cpp:93
void init_class_len(void);
int addseur(int cn, const combufrec *const cbuf);

static const int TP_ALKUT = 0;   // Juk/VCom.cpp:63
static const int TP_TARKT = 5;   // Juk/VCom.cpp:68
static const int TP_UNKNOWN = 7; // no case in tarkcom, class_len 0

// ---------------------------------------------------------------- H-032

static void udpFixture()
{
	REQUIRE(tp::sandboxOk());
	tpv::initBase();
	tpv::computeRecordSizes();
	init_class_len();
	REQUIRE(class_len[7] == 0);
	REQUIRE(class_len[11] == 0);
	REQUIRE(class_len[12] == 0);
	REQUIRE(class_len[TP_TARKT] > 0);
	REQUIRE(combufsize <= (int) sizeof(combufrec));   // so the memset at :653 stays in the slot
	REQUIRE(alkut == TP_ALKUT);                         // no tark_alku for these classes
	keyclose[0] = 0;
	comtype[0] = comtpUDP;
}

// A received datagram as luepakettiUDP passes it on: header + len data
// bytes (the read buffer buf1 is LUEBUF 3000 + 6 bytes, Tp/TpComY.cpp:930).
static char g_src[3006];

static const combufrec *makePacket(int cls, int len)
{
	REQUIRE(len + 14 <= (int) sizeof(g_src));
	memset(g_src, 0, sizeof(g_src));
	combufrec *c = (combufrec *) g_src;
	c->so = 1;
	c->id = 1;
	c->iid = 254;            // id + iid == 255 as the receiver requires (:1000)
	c->pkgclass = (UCHAR) cls;
	c->len = (UINT16) len;
	memset(g_src + 14, 0x11, len);
	return c;
}

static const int SLOTS = INBUFL + 1;   // as allocated at :2399
static const int GUARD = 4096;

// Runs addseur on the last ring slot of a block laid out as production
// allocates it, followed by a guard area. Returns the number of guard bytes
// that changed (all of them must be the packet's 0x11 data).
static int guardedAddseur(int cls, int len, int *changedAs11)
{
	const int S = (int) sizeof(combufrec);
	char *g = (char *) malloc(SLOTS * S + GUARD);
	REQUIRE(g != NULL);
	memset(g, 0, SLOTS * S);
	memset(g + SLOTS * S, 0xA5, GUARD);
	inbuf = (combufrec *) g;
	inbens = inbseur = INBUFL - 1;   // last ring slot; UDP accepts when equal (:650)
	int r = addseur(0, makePacket(cls, len));
	REQUIRE(r == 0);                 // accepted
	REQUIRE(inbseur == 0);           // ring wrapped: the packet went to slot INBUFL-1
	int changed = 0, as11 = 0;
	for (int i = 0; i < GUARD; i++) {
		unsigned char b = (unsigned char) g[SLOTS * S + i];
		if (b != 0xA5) {
			changed++;
			if (b == 0x11)
				as11++;
		}
	}
	*changedAs11 = as11;
	return changed;
}

// Control: an in-limit packet (TARKT, its class_len) on the same slot
// leaves the guard untouched. Shows the layout and the call path.
TEST_CASE("H-032 control: in-limit packet stays inside the receive block")
{
	udpFixture();
	int as11 = 0;
	int changed = guardedAddseur(TP_TARKT, class_len[TP_TARKT], &as11);
	tp::info(tp::fmt("class %d len %d: guard bytes changed=%d", TP_TARKT, class_len[TP_TARKT], changed));
	CHECK(changed == 0);
}

// Correct: a datagram of a class with class_len 0 is rejected or at least
// limited to the slot. Defect: addseur copies len+14 = 1014 bytes into the
// last slot, so exactly (INBUFL-1)*S + 1014 - (INBUFL+1)*S bytes past the
// production-sized block are overwritten with the packet data.
TEST_CASE("H-032a relay addseur copies an unchecked length past the receive block")
{
	udpFixture();
	const int S = (int) sizeof(combufrec);
	const int len = 1000;
	const int expected = (INBUFL - 1) * S + len + 14 - SLOTS * S;
	REQUIRE(expected > 0);
	REQUIRE(expected <= GUARD);
	int as11 = 0;
	int changed = guardedAddseur(TP_UNKNOWN, len, &as11);
	// Precondition: either nothing or exactly the packet tail landed there.
	REQUIRE((changed == 0 || (changed == expected && as11 == expected)));
	bool observed = changed == expected;
	tp::reportResult("H-032", "addseur-copies-past-receive-block", observed,
		tp::fmt("class %d (class_len %d) len %d into slot %d of %d x %d bytes: %d bytes past the block "
			"overwritten (expected %d), all packet data=%d; the UDP length check itself (:1001) is "
			"shown by class_len 0 only", TP_UNKNOWN, class_len[TP_UNKNOWN], len, INBUFL - 1, SLOTS, S,
			changed, expected, as11 == changed ? 1 : 0));
}

#ifdef __SANITIZE_ADDRESS__
// (b) AddressSanitizer build only: the production-sized calloc block; ASan
// must report a heap-buffer-overflow WRITE of len+14 bytes from addseur.
TEST_CASE("H-032b relay addseur heap-buffer-overflow (ASan)")
{
	udpFixture();
	inbuf = (combufrec *) calloc(SLOTS, sizeof(combufrec));
	REQUIRE(inbuf != NULL);
	inbens = inbseur = INBUFL - 1;
	int (*volatile f)(int, const combufrec *const) = addseur;
	static const char *const expect[] = { "heap-buffer-overflow", "WRITE of size 1014", "addseur", 0 };
	const combufrec *pkt = makePacket(TP_UNKNOWN, 1000);
	tp::armCrash("H-032", "addseur-heap-buffer-overflow", "asan", expect);
	f(0, pkt);
	tp::disarmCrash();
	tp::reportResult("H-032", "addseur-heap-buffer-overflow", false, "no ASan report from addseur");
}
#endif

// ---------------------------------------------------------------- H-026

static void runTarkcom(void *)
{
	tarkcom(NULL);
}

// Stopper: waits until both slots are consumed or 1500 ms, then clears comfl
// and records the queue state.
static volatile LONG g_inbensAtStop = -1;
static volatile LONG g_slot1Done = -1;

static unsigned __stdcall stopper(void *)
{
	for (int i = 0; i < 150 && inbens != 2; i++)
		Sleep(10);
	comfl = 0;
	InterlockedExchange(&g_inbensAtStop, inbens);
	InterlockedExchange(&g_slot1Done, inbuf[1].lahetetty);
	return 0;
}

static void recvFixture(int cls0)
{
	REQUIRE(tp::sandboxOk());
	tpv::initBase();
	tpv::computeRecordSizes();
	REQUIRE(status_on == 0);
	inbuf = (combufrec *) calloc(INBUFL + 1, sizeof(combufrec));
	REQUIRE(inbuf != NULL);
	inbuf[0].pkgclass = (UCHAR) cls0;
	inbuf[0].portti = 1;
	inbuf[1].pkgclass = (UCHAR) TP_ALKUT;   // a valid packet after it (no-op handler)
	inbuf[1].portti = 1;
	inbens = 0;
	inbseur = 2;
	comfl = 1;
}

// Control: two valid packets are consumed and the loop stops on comfl = 0.
TEST_CASE("H-026 control: valid packets are consumed and the loop stops")
{
	recvFixture(TP_ALKUT);
	HANDLE hs = (HANDLE) _beginthreadex(NULL, 0, stopper, NULL, 0, NULL);
	REQUIRE(hs != 0);
	tp::ThreadRun r = tp::runOnThread(runTarkcom, NULL, 4000, "tarkcom");
	WaitForSingleObject(hs, 5000);
	CloseHandle(hs);
	CHECK(r.outcome == tp::THREAD_FINISHED);
	CHECK(g_inbensAtStop == 2);
	CHECK(g_slot1Done == 1);
}

// Correct: an unknown class is skipped (or rejected) and the next packet is
// processed. Defect: inbens stays at the unknown packet, the following valid
// packet is never processed, and the loop keeps running after comfl = 0
// (it is alive: it marks the slot processed again and keeps reporting).
TEST_CASE("H-026 relay unknown packet class stops receiving permanently")
{
	recvFixture(TP_UNKNOWN);
	tp::setQuietStubTag("tarkcom-h026");   // production would Sleep(1000) per pass
	long calls0 = tp::stubCallCount();
	HANDLE hs = (HANDLE) _beginthreadex(NULL, 0, stopper, NULL, 0, NULL);
	REQUIRE(hs != 0);
	tp::ThreadRun r = tp::runOnThread(runTarkcom, NULL, 4000, "tarkcom-h026");
	WaitForSingleObject(hs, 5000);
	CloseHandle(hs);
	bool blocked = r.outcome == tp::THREAD_BLOCKED;
	bool alive = false;
	long callsAfterStop = 0;
	if (blocked) {
		// Liveness: the loop marks slot 0 processed again (:977) and keeps
		// calling the error routine, so it is spinning, not waiting.
		EnterCriticalSection(&inb_CriticalSection);
		inbuf[0].lahetetty = 0;
		LeaveCriticalSection(&inb_CriticalSection);
		long c1 = tp::stubCallCount();
		Sleep(300);
		alive = inbuf[0].lahetetty == 1;
		callsAfterStop = tp::stubCallCount() - c1;
	}
	if (blocked) {
		// A worker left running must be the spinning loop, nothing else.
		REQUIRE(alive);
		REQUIRE(callsAfterStop > 0);
	}
	bool observed = blocked && alive && callsAfterStop > 0 && g_inbensAtStop == 0 && g_slot1Done == 0;
	if (!blocked) {
		// Correct behaviour: the valid packet after the unknown one was taken.
		REQUIRE(g_inbensAtStop == 2);
	}
	tp::reportResult("H-026", "unknown-class-stops-receiving", observed,
		tp::fmt("worker %s after comfl=0; inbens=%ld (valid packet at slot 1 processed=%ld); loop alive=%d, "
			"error calls in 300 ms=%ld, total=%ld", blocked ? "still running" : "ended", (long) g_inbensAtStop,
			(long) g_slot1Done, alive ? 1 : 0, callsAfterStop, tp::stubCallCount() - calls0));
}
