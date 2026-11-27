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

// H-008: HK tall_sak (Hk/HkSak.cpp:59-106) enters tall_CriticalSection at
// :89 and returns -3 at :99 without leaving it, when the shooting position
// is outside the class's lsak range. Reached through the public
// tulkKurvinen (:239-245, r_no 999) on a controlled worker thread, as the
// comm thread does in production (Hk/HkCom32.cpp:647).
// Plan: doc/arkkitehtuuri/testaus.md sections 6 and 14.

#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include "HkDeclare.h"
#include "../fixtures/HkFixture.h"

// Not declared in any header the core includes.
void initindex(void);                     // Hk/HkIx.cpp:102
void tulkKurvinen(char *sanoma, int kielto);
extern INT sak_com, sak_baud, sak_laji;   // include/TpLaitteet.h:133

// One competitor, bib 12, record 1, class 0 with lsak[0] = lsak0.
static void penaltyFixture(int lsak0)
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	tph::setStages(2, 10);
	kilpparam.maxasakko = 5;    // the message's penalty digit must be <= this
	sak_laji = 0;               // default 2 (Hk/HkGlobals.cpp:461)
	loki = 0;
	k_pv = 0;
	sarjaluku = 1;
	Sarjat[0].lsak[0] = lsak0;
	maxrec = 10;
	nrec = 2;                   // getpos needs d < nrec (Hk/HkIx.cpp:178)
	minkilpno = 1;
	kilparr = new kilptietue[maxrec + 1];   // after setStages (Q34)
	kilparr[1].nollaa();
	kilparr[1].setIdAll(12);
	initindex();
	idindex->ix = (char *) calloc(maxrec + 2, idindex->recl);
	idindex->nrec = 0;
	REQUIRE(setpos(12, 1) == 0);
	REQUIRE(getpos(12) == 1);
}

struct KurvArg {
	char msg[20];
};

static void runKurvinen(void *p)
{
	tulkKurvinen(((KurvArg *) p)->msg, 0);
}

// Control: the probe sees free/held/free, and an unknown bib (13) returns
// -2 before the lock (:84-88), leaving it free.
TEST_CASE("H-008 control: lock probe and early error path")
{
	penaltyFixture(0);
	CHECK(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);
	EnterCriticalSection(&tall_CriticalSection);
	CHECK(tp::lockFreeForOtherThread(&tall_CriticalSection) == 0);
	LeaveCriticalSection(&tall_CriticalSection);
	CHECK(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);

	static KurvArg arg;
	strcpy(arg.msg, "  13 1 3");
	size_t m0 = tp::uiMessages().size();
	tp::ThreadRun r = tp::runOnThread(runKurvinen, &arg, 5000, "kurvinen");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	CHECK(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);
	CHECK(tp::uiMessages().size() == m0 + 1);
}

// Control: a valid message (lsak[0] = 3, position 1 in range) takes the
// lock at :89, saves the record and leaves the lock, so the same entry
// and probe show a released lock on the normal path. The record is saved
// with the production tallenna into an empty KILP.DAT in the sandbox.
TEST_CASE("H-008 control: valid penalty message releases the lock")
{
	penaltyFixture(3);
	tph::makeEmptyKilpDat();
	char buf[KILPBUFSIZE];
	DATAREF d = 0;
	addrec(&datf2, &d, (kilparr[1].pack(buf), buf));
	REQUIRE(d == 1);
	nrec = datf2.numrec;
	maxyhteys = 0;
	comfl = 0;
	static KurvArg arg;
	strcpy(arg.msg, "  12 1 3");
	size_t m0 = tp::uiMessages().size();
	tp::ThreadRun r = tp::runOnThread(runKurvinen, &arg, 5000, "kurvinen");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	tp::info(tp::fmt("messages=%d asakot0=%ls", (int) (tp::uiMessages().size() - m0),
		kilparr[1].pv[0].asakot));
	// No message: neither the -2 nor the -3 error path (both report one),
	// so tall_sak went through the lock to tallenna. Where tallenna stores
	// the penalty is not part of this control (recorded in TPINFO only).
	CHECK(tp::uiMessages().size() == m0);
	CHECK(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);
	CHECK(tp::lockOwnerThread(&tall_CriticalSection) == 0);
}

// Message "  12 1 3": bib 12, position 1 (index 0), penalty 3. With
// lsak[0] = 0 the position is out of range, so tall_sak returns -3 at :99.
// Correct: the error path releases tall_CriticalSection. Defect: after the
// worker thread has returned (and ended), the lock is still owned by it and
// no other thread can take it.
TEST_CASE("H-008 HK tall_sak error path leaves tall_CriticalSection held")
{
	penaltyFixture(0);
	REQUIRE(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);
	static KurvArg arg;
	strcpy(arg.msg, "  12 1 3");
	size_t m0 = tp::uiMessages().size();
	tp::ThreadRun r = tp::runOnThread(runKurvinen, &arg, 5000, "kurvinen");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);   // the error path returns
	REQUIRE(tp::uiMessages().size() == m0 + 1);   // the -3 error message
	std::string msg = tp::uiMessages().back().text;
	REQUIRE(msg.find("virhe -3") != std::string::npos);   // the :99 path, not another error
	int probe = tp::lockFreeForOtherThread(&tall_CriticalSection);
	REQUIRE(probe >= 0);
	DWORD owner = tp::lockOwnerThread(&tall_CriticalSection);
	// Held: by the worker and nobody else. Free: no owner recorded.
	REQUIRE((probe == 0 ? owner == r.threadId : owner == 0));
	bool observed = probe == 0 && owner == r.threadId;
	tp::reportResult("H-008", "tall-sak-error-leaves-lock-held", observed,
		tp::fmt("worker returned in %lu ms; other thread got lock=%d; owner=%lu worker=%lu "
			"(worker has ended: lock orphaned); message: %s", (unsigned long) r.elapsedMs, probe,
			(unsigned long) owner, (unsigned long) r.threadId, msg.c_str()));
}
