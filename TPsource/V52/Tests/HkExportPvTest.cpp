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

// Yksikkotestit Paiva/Vaihe-valinnalle Tietojen siirto tiedostoon
// -toiminnossa (Hk/HkExportPv.h).

#include "doctest.h"
#include "HkExportPv.h"

TEST_CASE("exportPvRange: Kaikki (0) kirjoittaa kaikki vaiheet")
{
	int lo = -1, hi = -1;
	exportPvRange(0, 4, &lo, &hi);
	CHECK(lo == 0);
	CHECK(hi == 4);
}

TEST_CASE("exportPvRange: yksittainen vaihe antaa vain sen vaiheen")
{
	int lo = -1, hi = -1;
	exportPvRange(1, 4, &lo, &hi);
	CHECK(lo == 0);
	CHECK(hi == 1);
	exportPvRange(3, 4, &lo, &hi);
	CHECK(lo == 2);
	CHECK(hi == 3);
}

TEST_CASE("exportPvRange: viimeinen vaihe (max) on sallittu")
{
	int lo = -1, hi = -1;
	exportPvRange(4, 4, &lo, &hi);
	CHECK(lo == 3);
	CHECK(hi == 4);
}

TEST_CASE("exportPvRange: kelvoton valinta palaa kaikkiin vaiheisiin")
{
	int lo = -1, hi = -1;
	exportPvRange(5, 4, &lo, &hi);
	CHECK(lo == 0);
	CHECK(hi == 4);
	exportPvRange(-1, 4, &lo, &hi);
	CHECK(lo == 0);
	CHECK(hi == 4);
}

TEST_CASE("exportPvRange: yksi vaihe, valinta Kaikki ja 1 antavat saman")
{
	int lo = -1, hi = -1;
	exportPvRange(0, 1, &lo, &hi);
	CHECK(lo == 0);
	CHECK(hi == 1);
	exportPvRange(1, 1, &lo, &hi);
	CHECK(lo == 0);
	CHECK(hi == 1);
}
