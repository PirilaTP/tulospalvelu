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
