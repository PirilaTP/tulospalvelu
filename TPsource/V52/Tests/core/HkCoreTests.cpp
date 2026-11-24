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

// HK core component tests (TpCoreHk): smoke test, H-041 (record larger
// than the KILPBUFSIZE stack buffer) and H-031 (refused console stage
// change leaves the vaiheenvaihto flag set). Plans:
// doc/arkkitehtuuri/testaus.md sections 5.2, 6 and 14.
// Run only through Tests/run_core_tests.py (one process per test case).

#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include "HkDeclare.h"
#include "../fixtures/HkFixture.h"

TEST_CASE("TpCoreHk smoke: sandbox and build constants")
{
	REQUIRE(tp::sandboxOk());
	tp::info(tp::fmt("N_PV=%d VALUKU=%d KILPBUFSIZE=%d", (int) N_PV, (int) VALUKU,
		(int) KILPBUFSIZE));
	CHECK(tp::uiMessages().empty());
}

// ---------------------------------------------------------------- H-041

// Shipped HK Release defaults: N_PV 5, VALUKU 250 (Hk/HkDef.h:45-58), so
// KILPBUFSIZE is 10000 (Hk/HkDef.h:98-103). The only limit on the stage and
// split counts (Hk/HkSrj.cpp:158) is n_pv <= N_PV and valuku <= VALUKU.
static void setMaxStages()
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	REQUIRE(N_PV == 5);
	REQUIRE(VALUKU == 250);
	REQUIRE(KILPBUFSIZE == 10000);
	tph::setStages(N_PV, VALUKU);
	// Structural checks of the sizes luekentat computed (no hard-coded
	// field sums): Hk/HkDat.cpp:137 and :176-177.
	REQUIRE(kilpparam.vatpsize > 0);
	REQUIRE(kilpparam.kilppvtpsize == kilpparam.kilppvtpsize0 + (VALUKU + 2) * kilpparam.vatpsize);
	REQUIRE(kilpparam.kilprecsize == kilpparam.kilprecsize0 + N_PV * kilpparam.kilppvtpsize);
}

static std::string sizeDetail()
{
	return tp::fmt("n_pv=%d valuku=%d kilprecsize0=%d kilppvtpsize0=%d vatpsize=%d "
		"kilppvtpsize=%d kilprecsize=%d KILPBUFSIZE=%d", kilpparam.n_pv, kilpparam.valuku,
		kilpparam.kilprecsize0, kilpparam.kilppvtpsize0, kilpparam.vatpsize,
		kilpparam.kilppvtpsize, kilpparam.kilprecsize, (int) KILPBUFSIZE);
}

// (a) Correct: the record of an accepted configuration fits KILPBUFSIZE.
// Defect: kilprecsize > KILPBUFSIZE for n_pv N_PV, valuku VALUKU.
TEST_CASE("H-041a HK record size vs KILPBUFSIZE at shipped limits")
{
	setMaxStages();
	bool observed = kilpparam.kilprecsize > KILPBUFSIZE;
	tp::reportResult("H-041", "record-larger-than-kilpbufsize", observed, sizeDetail());
}

// (b3) The production pack() (Hk/HkDat.cpp:424-430) is run twice into a heap
// buffer pre-filled with two different patterns. Bytes equal in both runs
// were written by pack. The test shows that pack writes exactly
// kilprecsize bytes, i.e. beyond KILPBUFSIZE, the size of the stack buffer
// that write() (:583), read() (:565) and addfrec (:629) pass to it.
// Correct: pack writes at most KILPBUFSIZE bytes (or the configuration is
// refused). Deterministic; no crash.
TEST_CASE("H-041b3 HK pack writes past KILPBUFSIZE (heap guard)")
{
	setMaxStages();
	kilptietue *k = new kilptietue;   // after n_pv / valuku are final (Q34)
	k->nollaa();
	const int size = kilpparam.kilprecsize;
	const int total = KILPBUFSIZE + 4096;
	REQUIRE(size < total);
	std::vector<unsigned char> buf(total), a(total), b(total);
	memset(&buf[0], 0xA5, total);
	k->pack((char *) &buf[0]);
	a = buf;
	memset(&buf[0], 0x5A, total);
	k->pack((char *) &buf[0]);
	b = buf;
	int lastWritten = -1;
	int writtenBeyond = 0;
	for (int i = 0; i < total; i++) {
		if (a[i] == b[i]) {
			lastWritten = i;
			if (i >= KILPBUFSIZE)
				writtenBeyond++;
		}
	}
	// Precondition: pack writes exactly [0, kilprecsize) and nothing after.
	bool exact = true;
	for (int i = 0; i < total; i++) {
		bool written = a[i] == b[i];
		if (written != (i < size)) {
			exact = false;
			break;
		}
	}
	REQUIRE(exact);
	bool observed = size > KILPBUFSIZE && writtenBeyond > 0;
	tp::reportResult("H-041", "pack-writes-past-kilpbufsize", observed,
		sizeDetail() + tp::fmt("; pack wrote bytes [0,%d), %d bytes at or after offset %d",
			lastWritten + 1, writtenBeyond, (int) KILPBUFSIZE));
}

#ifdef __SANITIZE_ADDRESS__
// (b2) AddressSanitizer build only. The production kilptietue::write(d, 0)
// (Hk/HkDat.cpp:581-589) packs the record into its stack buffer
// char buf[KILPBUFSIZE]. ASan must report a stack-buffer-overflow WRITE on
// variable 'buf' of size KILPBUFSIZE in frame kilptietue::write. The report
// callback prints the TPRESULT; the runner checks the report itself.
// write() is called through a volatile member pointer so that whole
// program optimisation cannot inline it into this test.
TEST_CASE("H-041b2 HK write overflows its stack buffer (ASan)")
{
	setMaxStages();
	tph::makeEmptyKilpDat();
	kilptietue *k = new kilptietue;
	k->nollaa();
	int (kilptietue::*volatile wr)(int, int) = &kilptietue::write;
	static const char *const expect[] = { "stack-buffer-overflow", "WRITE of size", "'buf'",
		"kilptietue::write", 0 };
	tp::info(sizeDetail());
	tp::armCrash("H-041", "write-stack-buffer-overflow", "asan", expect);
	(k->*wr)(1, 0);
	tp::disarmCrash();
	// Survived: no overflow of write's stack buffer was seen. Reported as
	// observed=0 (XPASS, never accepted); the detail says whether the record
	// would still exceed KILPBUFSIZE (e.g. buffer moved or resized by a fix).
	tp::reportResult("H-041", "write-stack-buffer-overflow", false,
		sizeDetail() + "; no ASan report from kilptietue::write");
}
#endif

// ---------------------------------------------------------------- H-031

// Console vaihdavaihe (Hk/HkIx.cpp:1855-2005). Fixture: empty KILP.DAT,
// indexes built by the production teejarj(false), one open link.
static void stageChangeFixture()
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
	lah_alku[0] = 0;
	lah_vaihto[0] = 0;
	REQUIRE(vaiheenvaihto == false);
}

// Control: in a fresh process an allowed first change (empty queue) goes
// through: k_pv becomes 1 and the flag is cleared again (:1995). Shows that
// the allowed path runs in this fixture, so a difference in the defect test
// is due to the flag.
TEST_CASE("H-031 control: allowed stage change proceeds")
{
	stageChangeFixture();
	cjseur[0] = 0;
	cjens[0] = 0;
	REQUIRE(lahettamatta(0) == 0);
	int r = vaihdavaihe(1, 0);
	tp::info(tp::fmt("r=%d k_pv=%d arv_pv=%d vaiheenvaihto=%d comopen0=%d lah_alku0=%d ui=%d", r, k_pv,
		arv_pv, vaiheenvaihto ? 1 : 0, comopen[0], lah_alku[0], (int) tp::uiMessages().size()));
	CHECK(r == 0);
	CHECK(k_pv == 1);
	CHECK(arv_pv == 1);
	CHECK(vaiheenvaihto == false);
	CHECK(comopen[0] == 1);
	CHECK(lah_alku[0] == 1);
	CHECK(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);
	CHECK(tp::lockFreeForOtherThread(&ajat_CriticalSection) == 1);
	CHECK(tp::lockFreeForOtherThread(&emit_CriticalSection) == 1);
}

// Defect: a change refused because of unsent messages (:1870-1874) leaves
// vaiheenvaihto set, so a later change that is allowed (queue empty)
// returns at :1868-1869 without doing anything. Correct: the later change
// proceeds as in the control.
TEST_CASE("H-031 refused stage change blocks a later allowed change")
{
	stageChangeFixture();
	cjseur[0] = 1;
	cjens[0] = 0;
	REQUIRE(lahettamatta(0) == 1);
	size_t m0 = tp::uiMessages().size();
	int r1 = vaihdavaihe(1, 0);
	REQUIRE(r1 == 1);                         // refused
	REQUIRE(k_pv == 0);
	REQUIRE(tp::uiMessages().size() == m0 + 1);  // the refusal message
	bool flagAfterRefusal = vaiheenvaihto;

	cjens[0] = cjseur[0];                     // queue now empty: allowed
	REQUIRE(lahettamatta(0) == 0);
	size_t m1 = tp::uiMessages().size();
	int r2 = vaihdavaihe(1, 0);
	bool nothingDone = r2 == 0 && k_pv == 0 && arv_pv == 0 && lah_alku[0] == 0 &&
		tp::uiMessages().size() == m1;
	bool observed = flagAfterRefusal && vaiheenvaihto && nothingDone;
	tp::reportResult("H-031", "refused-change-blocks-later-change", observed,
		tp::fmt("refused: r1=%d flag_after=%d; later allowed call: r2=%d k_pv=%d arv_pv=%d "
			"vaiheenvaihto=%d lah_alku0=%d new_messages=%d (control: k_pv becomes 1)",
			r1, flagAfterRefusal ? 1 : 0, r2, k_pv, arv_pv, vaiheenvaihto ? 1 : 0, lah_alku[0],
			(int) (tp::uiMessages().size() - m1)));
}
