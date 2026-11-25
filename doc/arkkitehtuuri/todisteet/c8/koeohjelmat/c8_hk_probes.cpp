// C-8 closing: isolated probes for the HK bib index (Hk/HkIx.cpp).
// Every probe calls the real production code (linked as in TpCoreHk); only
// global state is prepared here. Protocol: TPsource/V52/Tests/fixtures/TpCore.h
// (preconditions with REQUIRE/CHECK, exactly one TPRESULT per defect test,
// a crash or timeout is never evidence). No network, no shipped program.
//
//  H-128a (N-C1) setpos never deletes: setpos(k, 0) after setpos(k, d)
//         inserts k -> 0 before k -> d, and a walk getpos(x, true) +
//         nextpos() stops at that entry (Hk/HkIx.cpp:164-226).
//  H-128b (N-C2) setpos skips the add when the next-greater key maps to the
//         same record: lowering a bib into a free number below it leaves
//         getpos(new) == -1 after the old bib is cleared.
//  H-129  (N-C3) teejarj(true) (via the production vaihdavaihe) zeroes the
//         idindex buffer but not idindex->nrec (Hk/HkIx.cpp:1610).

#include <winsock2.h>
#include "doctest.h"
#include "fixtures/TpCore.h"
#include "HkDeclare.h"
#include "fixtures/HkFixture.h"

// Fixture as H-031 (TPsource/V52/Tests/core/HkCoreTests.cpp
// stageChangeFixture): empty KILP.DAT, indexes and kilparr built by the
// production teejarj(false), one open link so that vaihdavaihe runs.
static void indexFixture()
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	kilpparam.n_pv_akt = 2;
	tph::setStages(2, 10);
	maxkilpno = 1000;
	sarjaluku = 0;
	nsarjayhd = 0;
	emitfl = 0;
	vaihevalinta = 0;
	k_pv = 0;
	arv_pv = 0;
	tph::makeEmptyKilpDat();
	REQUIRE(teejarj(false) == 0);
	REQUIRE(idindex != 0);
	REQUIRE(idindex->ix != 0);
	REQUIRE(idindex->nrec == 0);
	maxyhteys = 1;
	comopen[0] = 1;
	keyclose[0] = 0;
	lah_alku[0] = 0;
	lah_vaihto[0] = 0;
	cjseur[0] = 0;
	cjens[0] = 0;
}

// Index-only probes need record positions d < nrec (setpos/getpos bound).
// No record is read in these probes, so nrec is raised for the index calls
// only; nothing that walks kilparr runs afterwards.
static int walk(int from, std::string &seen)
{
	int n = 0;
	int d = getpos(from, true);
	while (d > 0 && n < 50) {
		seen += tp::fmt("%d ", d);
		n++;
		d = nextpos();
	}
	seen += tp::fmt("(stop %d)", d);
	return n;
}

TEST_CASE("C8 H-128 control: walk by bib visits every live entry")
{
	indexFixture();
	nrec = 20;
	REQUIRE(setpos(104, 6) == 0);
	REQUIRE(setpos(105, 7) == 0);
	REQUIRE(setpos(106, 8) == 0);
	std::string seen;
	int n = walk(104, seen);
	tp::info("walk from 104: " + seen + tp::fmt("; idindex nrec=%d", (int) idindex->nrec));
	CHECK(n == 3);
	CHECK(getpos(105) == 7);
}

// Defect: after the delete call of deltall (setpos(kilpno, 0),
// Hk/HkDat.cpp:760) the walk from 104 should still reach bib 106 (d=8).
TEST_CASE("C8 H-128a setpos to zero leaves a stale entry that stops the walk")
{
	indexFixture();
	nrec = 20;
	REQUIRE(setpos(104, 6) == 0);
	REQUIRE(setpos(105, 7) == 0);
	REQUIRE(setpos(106, 8) == 0);
	REQUIRE(idindex->nrec == 3);
	REQUIRE(setpos(105, 0) == 0);           // as deltall, Hk/HkDat.cpp:760
	int g105 = getpos(105);
	std::string seen;
	walk(104, seen);
	bool reached106 = seen.find("8 ") != std::string::npos;
	bool observed = !reached106 && idindex->nrec == 4;
	tp::reportResult("H-128", "setpos-no-delete-walk-stops", observed,
		tp::fmt("after setpos(105,0): idindex nrec=%d (3 before; correct 2 or 3), getpos(105)=%d; "
			"walk from 104: ", (int) idindex->nrec, g105) + seen + " (control reaches 8)");
}

// Defect: bib change 61 -> 60 as tallenna does it (setpos(new, d) at
// Hk/HkDat.cpp:1157, then setpos(old, 0) at :1180). Correct: getpos(60) == 7.
TEST_CASE("C8 H-128b lowering a bib into a free number drops it from the index")
{
	indexFixture();
	nrec = 20;
	REQUIRE(setpos(61, 7) == 0);
	REQUIRE(setpos(100, 8) == 0);
	REQUIRE(getpos(61) == 7);
	int n0 = idindex->nrec;
	REQUIRE(setpos(60, 7) == 0);            // new bib, same record
	int n1 = idindex->nrec;
	REQUIRE(setpos(61, 0) == 0);            // old bib cleared
	int g60 = getpos(60);
	int g61 = getpos(61);
	bool observed = g60 == -1 && n1 == n0;
	tp::reportResult("H-128", "setpos-skips-add-same-record", observed,
		tp::fmt("getpos(60)=%d (correct 7), getpos(61)=%d, idindex nrec %d -> %d after "
			"setpos(60,7) (no entry added) -> %d", g60, g61, n0, n1, (int) idindex->nrec));
}

// Control: the production luo_posarr (Hk/HkIx.cpp:314) resets nrec before
// rebuilding, so the same prefilled index is empty after it (KILP.DAT has no
// records).
TEST_CASE("C8 H-129 control: luo_posarr resets idindex->nrec")
{
	indexFixture();
	for (int k = 1; k <= 5; k++)
		REQUIRE(setpos(k, 0) == 0);
	REQUIRE(idindex->nrec == 5);
	luo_posarr();
	tp::info(tp::fmt("after luo_posarr: idindex nrec=%d nrec=%d", (int) idindex->nrec, (int) nrec));
	CHECK(idindex->nrec == 0);
}

// Defect: a stage change rebuilds the index from KILP.DAT (no records here),
// so idindex->nrec should be 0 afterwards; the defect leaves the old count,
// and a later setpos appends after it (at position nrec_old).
TEST_CASE("C8 H-129 stage change keeps idindex->nrec")
{
	indexFixture();
	for (int k = 1; k <= 5; k++)
		REQUIRE(setpos(k, 0) == 0);       // five entries, as deletes leave them
	REQUIRE(idindex->nrec == 5);
	REQUIRE(lahettamatta(0) == 0);
	int r = vaihdavaihe(1, 0);            // production stage change -> teejarj(true)
	REQUIRE(r == 0);
	REQUIRE(k_pv == 1);
	int nAfter = idindex->nrec;
	REQUIRE(setpos(50, 0) == 0);
	int nAdd = idindex->nrec;
	bool observed = nAfter == 5 && nAdd == 6;
	tp::reportResult("H-129", "stage-change-keeps-index-count", observed,
		tp::fmt("vaihdavaihe r=%d k_pv=%d; idindex nrec after stage change=%d (correct 0, records "
			"in KILP.DAT 0); after one setpos=%d; capacity maxrec+2=%d", r, k_pv, nAfter, nAdd,
			(int) maxrec + 2));
}
