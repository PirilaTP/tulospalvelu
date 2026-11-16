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

// Yksikkotestit henkilotietokannan apufunktioille (Hk/HkHenkKopio.h).

#include <wchar.h>
#include "doctest.h"
#include "HkHenkKopio.h"

TEST_CASE("kopioJosEiTyhja: ei-tyhja arvo kopioidaan")
{
	wchar_t dst[10] = L"vanha";
	kopioJosEiTyhja(dst, 10, L"uusi");
	CHECK(wcscmp(dst, L"uusi") == 0);
}

TEST_CASE("kopioJosEiTyhja: tyhja arvo ei pyyhi vanhaa")
{
	wchar_t dst[10] = L"vanha";
	kopioJosEiTyhja(dst, 10, L"");
	CHECK(wcscmp(dst, L"vanha") == 0);
	kopioJosEiTyhja(dst, 10, NULL);
	CHECK(wcscmp(dst, L"vanha") == 0);
}

TEST_CASE("kopioJosEiTyhja: liian pitka arvo katkaistaan loppunollalla")
{
	wchar_t dst[4];
	wmemset(dst, L'x', 4);
	kopioJosEiTyhja(dst, 4, L"abcdef");
	CHECK(wcscmp(dst, L"abc") == 0);
}

TEST_CASE("etsiOtsikkoKolumni: loytaa sarakkeen paikan")
{
	wchar_t a[] = L"Id", b[] = L"Sukunimi", c[] = L"Yhdistys", d[] = L"Sarja";
	wchar_t *f[] = {a, b, c, d};
	CHECK(etsiOtsikkoKolumni(f, 4, L"Yhdistys") == 2);
	CHECK(etsiOtsikkoKolumni(f, 4, L"Id") == 0);
}

TEST_CASE("etsiOtsikkoKolumni: puuttuva sarake antaa -1")
{
	wchar_t a[] = L"Id", b[] = L"Sukunimi";
	wchar_t *f[] = {a, b};
	CHECK(etsiOtsikkoKolumni(f, 2, L"Yhdistys") == -1);
	CHECK(etsiOtsikkoKolumni(f, 0, L"Id") == -1);
}

struct TestiKentta {
	int id;
	int akt;
};

TEST_CASE("oletusKenttajarjestys: vain aktiiviset kentat taulukon jarjestyksessa")
{
	const TestiKentta k[] = {{1, 1}, {2, 0}, {3, 1}, {8, 0}, {4, 1}};
	int out[10];
	int n = oletusKenttajarjestys(k, 5, out, 10);
	CHECK(n == 3);
	CHECK(out[0] == 1);
	CHECK(out[1] == 3);
	CHECK(out[2] == 4);
}

TEST_CASE("oletusKenttajarjestys: tyhja tai ei-aktiivinen taulukko antaa nolla")
{
	const TestiKentta k[] = {{1, 0}, {2, 0}};
	int out[4];
	CHECK(oletusKenttajarjestys(k, 2, out, 4) == 0);
	CHECK(oletusKenttajarjestys(k, 0, out, 4) == 0);
}

TEST_CASE("oletusKenttajarjestys: tulos rajataan maxOut-kenttaan")
{
	const TestiKentta k[] = {{1, 1}, {2, 1}, {3, 1}, {4, 1}};
	int out[2];
	CHECK(oletusKenttajarjestys(k, 4, out, 2) == 2);
	CHECK(out[0] == 1);
	CHECK(out[1] == 2);
}
