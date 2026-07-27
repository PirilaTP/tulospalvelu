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

// Yksikkotestit sarjaotsikoiden tekstinmuodostukselle (Juk/VOtsikot.cpp).

#include <string.h>
#include <wchar.h>
#include <tptype.h>
#include <TpDef.h>
#include "VOtsikot.h"
#include "WideString.h"

// "Lahto: " skandit \u-escapeina, jottei lahdetiedoston merkistokoodaus
// vaikuta odotettuihin arvoihin.
#define LAHTO_FI L"L\u00e4ht\u00f6: "

TEST_CASE("Lahtoaika lisataan otsikkorivin peraan pilkulla erotettuna")
{
	wchar_t rivi[200];
	wcscpy(rivi, L"H21");
	LisaaLahtoaikaTeksti(rivi, 200, 30L*60000L, 12, 0);
	CHECK_WSTR(rivi, L"H21, " LAHTO_FI L"12.30.00");
}

// Sarjan lahto on offset t0-nollahetkesta, joten sarja joka lahtee tasan
// t0-hetkella (esim. 12:00:00) saa taysin laillisesti arvon 0. Aiempi versio
// ohitti sen falsy-arvona (!Sarjat[srj].lahto) eika tulostanut lahtoaikaa
// lainkaan. Ks. PR #35.
TEST_CASE("Lahtoaika tulostuu myos kun sarja lahtee tasan t0-hetkella")
{
	wchar_t rivi[200];
	wcscpy(rivi, L"H21");
	LisaaLahtoaikaTeksti(rivi, 200, 0, 12, 0);
	CHECK_WSTR(rivi, L"H21, " LAHTO_FI L"12.00.00");
}

TEST_CASE("Vain TMAALI0 tarkoittaa ettei lahtoaikaa ole asetettu")
{
	wchar_t rivi[200];
	wcscpy(rivi, L"H21");
	LisaaLahtoaikaTeksti(rivi, 200, TMAALI0, 12, 0);
	CHECK_WSTR(rivi, L"H21");
}

TEST_CASE("Tyhjalle riville ei tule roikkuvaa pilkkua alkuun")
{
	wchar_t rivi[200] = L"";
	LisaaLahtoaikaTeksti(rivi, 200, 30L*60000L, 12, 0);
	CHECK_WSTR(rivi, LAHTO_FI L"12.30.00");
}

TEST_CASE("Rivin loppuvalilyonnit nipistetaan pois ennen pilkkua")
{
	wchar_t rivi[200];
	wcscpy(rivi, L"H21       ");
	LisaaLahtoaikaTeksti(rivi, 200, 0, 12, 0);
	CHECK_WSTR(rivi, L"H21, " LAHTO_FI L"12.00.00");
}

TEST_CASE("Pelkista valilyonneista koostuva rivi katsotaan tyhjaksi")
{
	wchar_t rivi[200];
	wcscpy(rivi, L"     ");
	LisaaLahtoaikaTeksti(rivi, 200, 0, 12, 0);
	CHECK_WSTR(rivi, LAHTO_FI L"12.00.00");
}

TEST_CASE("Englanninkielinen otsake")
{
	wchar_t rivi[200];
	wcscpy(rivi, L"H21");
	LisaaLahtoaikaTeksti(rivi, 200, 0, 12, 1);
	CHECK_WSTR(rivi, L"H21, Start: 12.00.00");
}

TEST_CASE("Sekunnin osat jatetaan pois")
{
	wchar_t rivi[200] = L"";
	LisaaLahtoaikaTeksti(rivi, 200, 30L*60000L + 12345L, 12, 0);
	CHECK_WSTR(rivi, LAHTO_FI L"12.30.12");
}

TEST_CASE("Ennen t0-hetkea alkava sarja")
{
	wchar_t rivi[200] = L"";
	LisaaLahtoaikaTeksti(rivi, 200, -30L*60000L, 12, 0);
	CHECK_WSTR(rivi, LAHTO_FI L"11.30.00");
}

TEST_CASE("Liian pieni puskuri jattaa rivin ennalleen eika ylivuoda")
{
	// "H21, Lahto: 12.00.00" on 20 merkkia, eli tarvitsee 21 merkin tilan.
	wchar_t rivi[64];

	wcscpy(rivi, L"H21");
	LisaaLahtoaikaTeksti(rivi, 20, 0, 12, 0);
	CHECK_WSTR(rivi, L"H21");

	wcscpy(rivi, L"H21");
	LisaaLahtoaikaTeksti(rivi, 21, 0, 12, 0);
	CHECK_WSTR(rivi, L"H21, " LAHTO_FI L"12.00.00");
}
