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

// Yksikkotestit kilpailun paivamaaran tarkistukselle (tputilv2/DateNo.cpp: DateNoTarkka).

#include <wchar.h>
#include "doctest.h"

int DateNo(int y, int m, int d);
int DateNoTarkka(const wchar_t *st);
wchar_t *stDateNo(wchar_t *st, int pvm);

TEST_CASE("DateNoTarkka: kelvolliset muodot")
{
	int pv = DateNo(2026, 9, 27);
	CHECK(DateNoTarkka(L"27.9.2026") == pv);
	CHECK(DateNoTarkka(L"27.09.2026") == pv);
	CHECK(DateNoTarkka(L"2026-09-27") == pv);
	CHECK(DateNoTarkka(L"27.9.26") == pv);
	CHECK(DateNoTarkka(L"  27.9.2026 ") == pv);
	CHECK(wcscmp(stDateNo(NULL, DateNoTarkka(L"1.3.2026")), L"2026-03-01") == 0);
}

TEST_CASE("DateNoTarkka: karkauspaiva vain karkausvuonna")
{
	CHECK(DateNoTarkka(L"29.2.2028") == DateNo(2028, 2, 29));
	CHECK(DateNoTarkka(L"29.2.2026") == 0);
	CHECK(DateNoTarkka(L"2028-02-30") == 0);
}

TEST_CASE("DateNoTarkka: virheelliset hylataan")
{
	CHECK(DateNoTarkka(NULL) == 0);
	CHECK(DateNoTarkka(L"") == 0);
	CHECK(DateNoTarkka(L"31.4.2026") == 0);
	CHECK(DateNoTarkka(L"0.1.2026") == 0);
	CHECK(DateNoTarkka(L"1.13.2026") == 0);
	CHECK(DateNoTarkka(L"27.9.") == 0);
	CHECK(DateNoTarkka(L"27/9/2026") == 0);
	CHECK(DateNoTarkka(L"2026-9-27") == 0);
	CHECK(DateNoTarkka(L"27.9.2026x") == 0);
	CHECK(DateNoTarkka(L"27.9.206") == 0);
	CHECK(DateNoTarkka(L"1.1.1900") == 0);
	CHECK(DateNoTarkka(L"1.1.2100") == 0);
}