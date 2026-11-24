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

// Checks synthetic data files used by the black-box runs of HkMaali.exe
// (Tests/blackbox, doc/arkkitehtuuri/testaus.md section 17) with the same
// production reader, so a run is not spent on a malformed file.

#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include "HkDeclare.h"
#include "../fixtures/HkFixture.h"

// data/hk/run1_kilpsrj.xml: 1 race, 2 intermediaries, 1 class (H21).
// Expected: luesarjat returns 0 without messages; record size from
// luekentat is 360 + 1 * (152 + (2 + 2) * 8) = 544 bytes.
TEST_CASE("HK synthetic KilpSrj.xml for black-box run 1 is read")
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	REQUIRE(tp::fileSize("KilpSrj.xml") > 0);
	wcscpy(sarjaxmlfname, L"KilpSrj.xml");
	int r = luesarjat();
	tp::info(tp::fmt("luesarjat=%d n_pv=%d n_pv_akt=%d valuku=%d sarjaluku=%d messages=%d", r,
		kilpparam.n_pv, kilpparam.n_pv_akt, kilpparam.valuku, sarjaluku, (int) tp::uiMessages().size()));
	CHECK(r == 0);
	CHECK(kilpparam.n_pv == 1);
	CHECK(kilpparam.n_pv_akt == 1);
	CHECK(kilpparam.valuku == 2);
	CHECK(sarjaluku == 1);
	CHECK(tp::uiMessages().empty());
	luekentat(0);
	tp::info(tp::fmt("kilprecsize=%d", kilpparam.kilprecsize));
	CHECK(kilpparam.kilprecsize == 544);
}

// C-11 stage 4 (testaus.md section 18/19): data/hk/h041_v220.xml and
// h041_v221.xml are run1_kilpsrj.xml with <Races> 5 and <Intermediaries>
// 220 / 221 (RaceCount stays 1). The production reader must accept them
// without messages, and luekentat must give
// 360 + 5 * (152 + (valuku + 2) * 8) = 10000 / 10040 bytes, i.e. exactly
// KILPBUFSIZE (control) and KILPBUFSIZE + 40 (first size past the buffer).
// The test also prints the bytes the production pack() writes at offsets
// [KILPBUFSIZE, kilprecsize) for a cleared record, so the run report can
// tell whether a stack comparison against them is meaningful (all-zero
// bytes would be weak evidence).
static void checkH041Data(int valuku, int expectSize)
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	REQUIRE(KILPBUFSIZE == 10000);
	REQUIRE(tp::fileSize("KilpSrj.xml") > 0);
	wcscpy(sarjaxmlfname, L"KilpSrj.xml");
	int r = luesarjat();
	tp::info(tp::fmt("luesarjat=%d n_pv=%d n_pv_akt=%d valuku=%d sarjaluku=%d messages=%d", r,
		kilpparam.n_pv, kilpparam.n_pv_akt, kilpparam.valuku, sarjaluku, (int) tp::uiMessages().size()));
	CHECK(r == 0);
	CHECK(kilpparam.n_pv == 5);
	CHECK(kilpparam.n_pv_akt == 1);
	CHECK(kilpparam.valuku == valuku);
	CHECK(sarjaluku == 1);
	CHECK(tp::uiMessages().empty());
	luekentat(0);
	tp::info(tp::fmt("kilprecsize=%d KILPBUFSIZE=%d", kilpparam.kilprecsize, (int) KILPBUFSIZE));
	REQUIRE(kilpparam.kilprecsize == expectSize);
	// Constructed only now (Q34): pv[n_pv] and va[valuku+2] use the final counts.
	kilptietue *k = new kilptietue;
	k->nollaa();
	std::vector<unsigned char> buf(KILPBUFSIZE + 4096, 0xA5);
	k->pack((char *) &buf[0]);
	std::string tail;
	int nonzero = 0;
	for (int i = KILPBUFSIZE; i < kilpparam.kilprecsize; i++) {
		tail += tp::fmt("%02x", buf[i]);
		if (buf[i])
			nonzero++;
	}
	tp::info(tp::fmt("pack bytes [%d,%d) of a cleared record: %s (nonzero %d)", (int) KILPBUFSIZE,
		kilpparam.kilprecsize, tail.empty() ? "(none)" : tail.c_str(), nonzero));
	CHECK(buf[kilpparam.kilprecsize] == 0xA5);
	delete k;
}

TEST_CASE("HK synthetic KilpSrj.xml for H-041 control (valuku 220) is read")
{
	checkH041Data(220, 10000);
}

TEST_CASE("HK synthetic KilpSrj.xml for H-041 boundary (valuku 221) is read")
{
	checkH041Data(221, 10040);
}
