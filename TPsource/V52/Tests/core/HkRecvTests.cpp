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

// HK receive-side tests (TpCoreHk). Packets are injected into the receive
// queue and processed by the production receive loop tarkcom
// (Hk/HkCom32.cpp:875) on a controlled worker thread, as the comm thread
// does in production; tark_kilp (static, :242) is reached through it.
//  H-022: stage packets (VAIN_TULOST / KILPPVT) are applied without any
//    competitor-number check (:282-289, :325), so data meant for another
//    competitor is stored on record dk.
//  H-023a: VAIN_TULOST is applied to the receiver's own stage k_pv; the
//    sender's d.v.k_pv is never read (:282-289).
//  H-023b: tark_alku (:821-871) accepts the link (returns false) even when
//    the requested stage change was refused (:845-848, return value of
//    pyyda_vaihdavaihe ignored).
//  H-007: tark_kilp reads the record (:260) before taking tall_CS (:326), so
//    a local change made in between is overwritten (lost update).
// Plans: doc/arkkitehtuuri/testaus.md sections 6 and 15.

#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include <process.h>
#include "HkDeclare.h"
#include "../fixtures/HkFixture.h"

extern combufrec *inbuf;                     // Hk/HkCom32.cpp:75
void initindex(void);                        // Hk/HkIx.cpp:102
bool tark_alku(const combufrec *const cbuf, int cn);   // Hk/HkCom32.cpp:821

static const int TP_VAIN_TULOST = 3;         // Hk/HkCom32.cpp:50

// ---------------------------------------------------------------- shared

// Competitor X: bib 12 in record 1, class 0, club "VANHA". Two stages.
static void recvFixture()
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	kilpparam.n_pv_akt = 2;
	tph::setStages(2, 10);
	loki = 0;
	k_pv = 0;
	sarjaluku = 1;
	emitfl = 0;
	maxyhteys = 0;               // tallenna sends nothing (laheta loops 0 times)
	maxrec = 10;
	minkilpno = 1;
	Sarjat[0].mno[0] = Sarjat[0].mno[1] = 1;
	kilparr = new kilptietue[maxrec + 1];   // after setStages (Q34)
	kilptietue &x = kilparr[1];
	x.nollaa();
	x.setIdAll(12);
	wcscpy(x.sukunimi, L"Xkoski");
	wcscpy(x.seura, L"VANHA");
	x.setSarjaAll(0);
	initindex();
	idindex->ix = (char *) calloc(maxrec + 2, idindex->recl);
	idindex->nrec = 0;
	tph::makeEmptyKilpDat();
	char buf[KILPBUFSIZE];
	x.pack(buf);
	DATAREF d = 0;
	addrec(&datf2, &d, buf);
	REQUIRE(d == 1);
	nrec = datf2.numrec;
	REQUIRE(nrec == 2);
	REQUIRE(setpos(12, 1) == 0);
	REQUIRE(getpos(12) == 1);
	REQUIRE(ToimintaTila == 2);
	inbuf = (combufrec *) calloc(INBUFL + 1, sizeof(combufrec));
	REQUIRE(inbuf != NULL);
}

// Stage result packet (VAIN_TULOST) for record dk; vali = valuku+3 stores
// the handicap value (tasoitus) of stage k_pv (:288-289): the smallest write.
static void putTasoitus(int slot, int dk, int bib, int pkt_k_pv, int aika)
{
	combufrec *c = inbuf + slot;
	memset(c, 0, sizeof(*c));
	c->pkgclass = (char) TP_VAIN_TULOST;
	c->portti = 1;
	c->d.v.dk = (INT16) dk;
	c->d.v.bib = (INT16) bib;
	c->d.v.k_pv = (INT16) pkt_k_pv;
	c->d.v.vali = (INT16) (kilpparam.valuku + 3);
	c->d.v.aika = aika;
}

static volatile LONG g_target = 0;
static volatile LONG g_consumed = 0;

static void runTarkcom(void *)
{
	tarkcom(NULL);
}

static unsigned __stdcall stopper(void *)
{
	for (int i = 0; i < 500 && inbens != g_target; i++)
		Sleep(10);
	InterlockedExchange(&g_consumed, inbens == g_target ? 1 : 0);
	comfl = 0;
	return 0;
}

// Processes slots [0, n) with tarkcom on a worker; returns the run.
static tp::ThreadRun processPackets(int n)
{
	g_target = n;
	g_consumed = 0;
	inbens = 0;
	inbseur = n;
	comfl = 1;
	HANDLE hs = (HANDLE) _beginthreadex(NULL, 0, stopper, NULL, 0, NULL);
	REQUIRE(hs != 0);
	tp::ThreadRun r = tp::runOnThread(runTarkcom, NULL, 10000, "tarkcom");
	WaitForSingleObject(hs, 10000);
	CloseHandle(hs);
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	REQUIRE(g_consumed == 1);
	return r;
}

// ---------------------------------------------------------------- H-022

// Control: a packet with the matching bib (12) is stored on record 1.
TEST_CASE("H-022 control: stage packet for the right competitor is stored")
{
	recvFixture();
	putTasoitus(0, 1, 12, 0, 600);
	size_t m0 = tp::uiMessages().size();
	processPackets(1);
	CHECK(kilparr[1].pv[0].tasoitus == 600);
	CHECK(kilparr[1].kilpno == 12);
	CHECK(tp::uiMessages().size() == m0);
	CHECK(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);
}

// Correct: a stage packet whose bib (13) does not match record 1 (bib 12)
// is rejected. Defect: it is stored on record 1 exactly like the control.
TEST_CASE("H-022 HK stage packet applied to a different competitor")
{
	recvFixture();
	putTasoitus(0, 1, 13, 0, 600);
	size_t m0 = tp::uiMessages().size();
	processPackets(1);
	size_t msgs = tp::uiMessages().size() - m0;
	bool stored = kilparr[1].pv[0].tasoitus == 600;
	REQUIRE((stored || msgs > 0 || kilparr[1].pv[0].tasoitus == 0));
	bool observed = stored && kilparr[1].kilpno == 12 && msgs == 0;
	tp::reportResult("H-022", "stage-packet-no-bib-check", observed,
		tp::fmt("packet bib 13 for record 1 (bib %d): tasoitus=%ld, messages=%d; the packet's bib field is "
			"never read; two-station same-record-number allocation (Hk/HkDat.cpp:626-636) not run",
			(int) kilparr[1].kilpno, (long) kilparr[1].pv[0].tasoitus, (int) msgs));
}

// ---------------------------------------------------------------- H-023

// Correct: a VAIN_TULOST packet for stage 2 (d.v.k_pv = 1) is stored on
// stage 2 or rejected. Defect: it is stored on the receiver's stage k_pv 0.
// The control (H-022 control) shows the same packet for stage 1 is stored.
TEST_CASE("H-023a HK stage result applied to the receiver's own stage")
{
	recvFixture();
	REQUIRE(k_pv == 0);
	putTasoitus(0, 1, 12, 1, 600);
	size_t m0 = tp::uiMessages().size();
	processPackets(1);
	size_t msgs = tp::uiMessages().size() - m0;
	long s1 = kilparr[1].pv[0].tasoitus, s2 = kilparr[1].pv[1].tasoitus;
	bool observed = s1 == 600 && s2 == 0 && msgs == 0;
	tp::reportResult("H-023", "vain-tulost-to-receiver-stage", observed,
		tp::fmt("packet for stage 2 (d.v.k_pv=1), receiver k_pv=0: stage 1 tasoitus=%ld, stage 2 tasoitus=%ld, "
			"messages=%d", s1, s2, (int) msgs));
}

// H-023b fixture: as the H-031 stage-change fixture (Tests/core/HkCoreTests.cpp).
static void alkuFixture(int unsent)
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	kilpparam.n_pv_akt = 2;
	tph::setStages(2, 10);
	maxkilpno = 100;
	sarjaluku = 0;
	nsarjayhd = 0;
	emitfl = 0;
	vaihevalinta = 0;
	k_pv = 0;
	arv_pv = 0;
	tph::makeEmptyKilpDat();
	REQUIRE(teejarj(false) == 0);
	maxyhteys = 1;
	comopen[0] = 1;
	keyclose[0] = 0;
	cjseur[0] = unsent;
	cjens[0] = 0;
	REQUIRE(lahettamatta(0) == unsent);
	REQUIRE(vaiheenvaihto == false);
}

struct AlkuArg {
	combufrec cb;
	bool result;
};

static void runAlku(void *p)
{
	AlkuArg *a = (AlkuArg *) p;
	a->result = tark_alku(&a->cb, 0);
}

static AlkuArg *alkuPacket(int vaihe, unsigned flags)
{
	static AlkuArg a;
	memset(&a, 0, sizeof(a));
	a.cb.pkgclass = 0;
	a.cb.len = 10;                      // not 3 (:828)
	a.cb.d.alku.tunn = 1;
	a.cb.d.alku.vaihe = (UINT8) vaihe;
	a.cb.d.alku.flags = flags;
	a.cb.d.alku.nrec = (UINT16) datf2.numrec;
	// nrec equal: ln stays empty, so the wselectopt at :861-863 is not reached.
	REQUIRE(a.cb.d.alku.nrec == datf2.numrec);
	return &a;
}

// Control 1: a stage-2 link request without the change flag is refused
// (returns true, :850-857).
TEST_CASE("H-023b control: other-stage link without change request is refused")
{
	alkuFixture(0);
	AlkuArg *a = alkuPacket(2, 0);
	tp::ThreadRun r = tp::runOnThread(runAlku, a, 5000, "alku");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	CHECK(a->result == true);
	CHECK(k_pv == 0);
}

// Control 2: with an empty send queue the requested change is made
// (k_pv becomes 1) and the link is accepted (false): correct behaviour.
TEST_CASE("H-023b control: allowed stage change request is carried out")
{
	alkuFixture(0);
	AlkuArg *a = alkuPacket(2, 1);
	tp::ThreadRun r = tp::runOnThread(runAlku, a, 10000, "alku");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	CHECK(a->result == false);
	CHECK(k_pv == 1);
}

// Correct: when the requested stage change is refused (unsent messages),
// the link is refused too (true). Defect: the change is refused (k_pv stays
// 0, refusal message) but tark_alku returns false: the link is accepted
// although the stations are now in different stages.
TEST_CASE("H-023b HK link accepted although the stage change was refused")
{
	alkuFixture(1);
	AlkuArg *a = alkuPacket(2, 1);
	size_t m0 = tp::uiMessages().size();
	tp::ThreadRun r = tp::runOnThread(runAlku, a, 5000, "alku");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	size_t msgs = tp::uiMessages().size() - m0;
	REQUIRE(msgs == 1);
	std::string m = tp::uiMessages().back().text;
	REQUIRE(m.find("vaihdon") != std::string::npos);   // the refusal (Hk/HkIx.cpp:1873)
	REQUIRE(k_pv == 0);
	bool observed = a->result == false;
	tp::reportResult("H-023", "link-accepted-after-refused-change", observed,
		tp::fmt("stage change to 2 refused (k_pv stays %d, message: %s); tark_alku returned %s", k_pv,
			m.c_str(), a->result ? "true (link refused)" : "false (link accepted)"));
}

// ---------------------------------------------------------------- H-007

// Number of threads waiting for cs. Uses the LockCount encoding of
// Windows 8 and later ((-1 - LockCount) >> 2 waiters); undocumented, so the
// test also checks the encoding in its control.
static int lockWaiters(CRITICAL_SECTION *cs)
{
	LONG lc = cs->LockCount;
	return (int) ((-1 - lc) >> 2);
}

struct Ctl {
	HANDLE held;
	HANDLE go;            // main signals: tarkcom started
	volatile LONG sawWaiter;
	volatile LONG inbensWhileWaiting;
	volatile LONG changed;
	volatile LONG done;
};
static Ctl g_ctl;

// Controller: holds tall_CS, waits until the worker blocks on it (so its
// read at :260 is done), changes the club locally with the production
// tallenna, then releases the lock.
static unsigned __stdcall controller(void *)
{
	EnterCriticalSection(&tall_CriticalSection);
	SetEvent(g_ctl.held);
	WaitForSingleObject(g_ctl.go, 5000);
	for (int i = 0; i < 500 && lockWaiters(&tall_CriticalSection) < 1; i++)
		Sleep(10);
	g_ctl.sawWaiter = lockWaiters(&tall_CriticalSection) >= 1 ? 1 : 0;
	g_ctl.inbensWhileWaiting = inbens;
	kilptietue t;
	t.GETREC(1);
	wcscpy(t.seura, L"UUSI");
	t.tallenna(1, 0, 0, 0, NULL);
	g_ctl.changed = wcscmp(kilparr[1].seura, L"UUSI") == 0 ? 1 : 0;
	LeaveCriticalSection(&tall_CriticalSection);
	for (int i = 0; i < 500 && inbens != 1; i++)
		Sleep(10);
	comfl = 0;
	g_ctl.done = 1;
	return 0;
}

// Control: the waiter count reads 0 when the lock is only held and 1 while
// another thread is blocked on it (checks the LockCount encoding).
TEST_CASE("H-007 control: waiter probe on a CRITICAL_SECTION")
{
	REQUIRE(tp::sandboxOk());
	static CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	CHECK(lockWaiters(&cs) == 0);
	EnterCriticalSection(&cs);
	CHECK(lockWaiters(&cs) == 0);
	struct W {
		static void f(void *p)
		{
			EnterCriticalSection((CRITICAL_SECTION *) p);
			LeaveCriticalSection((CRITICAL_SECTION *) p);
		}
	};
	struct T {
		static unsigned __stdcall f(void *p)
		{
			W::f(p);
			return 0;
		}
	};
	HANDLE t = (HANDLE) _beginthreadex(NULL, 0, T::f, &cs, 0, NULL);
	REQUIRE(t != 0);
	int w = 0;
	for (int i = 0; i < 500 && (w = lockWaiters(&cs)) < 1; i++)
		Sleep(10);
	CHECK(w == 1);
	LeaveCriticalSection(&cs);
	CHECK(WaitForSingleObject(t, 5000) == WAIT_OBJECT_0);
	CloseHandle(t);
	CHECK(lockWaiters(&cs) == 0);
}

// Correct: the local change made while the received packet waits for the
// lock survives (club "UUSI") together with the received handicap.
// Defect: the receive thread saves the copy it read before the lock, so
// the club reverts to "VANHA" (lost update). Ordering is forced: the
// worker is blocked at :326 (waiter seen) before the change is made.
TEST_CASE("H-007 HK received packet overwrites a concurrent local change")
{
	recvFixture();
	putTasoitus(0, 1, 12, 0, 600);
	memset((void *) &g_ctl, 0, sizeof(g_ctl));
	g_ctl.held = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_ctl.go = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE hc = (HANDLE) _beginthreadex(NULL, 0, controller, NULL, 0, NULL);
	REQUIRE(hc != 0);
	REQUIRE(WaitForSingleObject(g_ctl.held, 5000) == WAIT_OBJECT_0);   // lock held before the worker starts
	inbens = 0;
	inbseur = 1;
	comfl = 1;
	SetEvent(g_ctl.go);
	tp::ThreadRun r = tp::runOnThread(runTarkcom, NULL, 15000, "tarkcom");
	REQUIRE(WaitForSingleObject(hc, 10000) == WAIT_OBJECT_0);
	CloseHandle(hc);
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	REQUIRE(g_ctl.sawWaiter == 1);            // worker was blocked on tall_CS
	REQUIRE(g_ctl.inbensWhileWaiting == 0);   // ...while processing the packet
	REQUIRE(g_ctl.changed == 1);              // the local change was stored
	REQUIRE(kilparr[1].pv[0].tasoitus == 600);   // the packet was applied
	bool reverted = wcscmp(kilparr[1].seura, L"VANHA") == 0;
	bool kept = wcscmp(kilparr[1].seura, L"UUSI") == 0;
	REQUIRE((reverted || kept));
	tp::reportResult("H-007", "receive-read-before-lock-lost-update", reverted,
		tp::fmt("local club change to UUSI made while the receive thread waited at tall_CS; after the "
			"packet: club=%ls, tasoitus=%ld", kilparr[1].seura, (long) kilparr[1].pv[0].tasoitus));
}
