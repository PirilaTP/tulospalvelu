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

// Yksikkotestit Osanottajat-taulukon Ennatys- ja Tavoite-sarakkeiden
// tekstiesitykselle (Hk/HkEnnTav.cpp).

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include "doctest.h"
#include "tptype.h"
#include "TpDef.h"
#include "HkEnnTav.h"

static int aika(int h, int min, int s)
{
	return(h*TUNTI + min*MINUUTTI + s*SEK);
}

// ---------------------------------------------------------------------
// ennTavTeksti
// ---------------------------------------------------------------------

TEST_CASE("ennTavTeksti: nolla tarkoittaa puuttuvaa aikaa ja antaa tyhjan")
{
	wchar_t buf[ENNTAV_LEN] = L"roskaa";
	CHECK(wcscmp(ennTavTeksti(buf, 0), L"") == 0);
}

TEST_CASE("ennTavTeksti: muoto tt.mm.ss kuten Kilpailijatiedot-kaavakkeella")
{
	wchar_t buf[ENNTAV_LEN];
	CHECK(wcscmp(ennTavTeksti(buf, aika(1, 2, 3)), L"01.02.03") == 0);
	CHECK(wcscmp(ennTavTeksti(buf, aika(0, 45, 30)), L"00.45.30") == 0);
	CHECK(wcscmp(ennTavTeksti(buf, aika(0, 0, 1)), L"00.00.01") == 0);
	CHECK(wcscmp(ennTavTeksti(buf, aika(23, 59, 59)), L"23.59.59") == 0);
}

TEST_CASE("ennTavTeksti: sekunnin osat katkaistaan, ei pyoristeta")
{
	wchar_t buf[ENNTAV_LEN];
	CHECK(wcscmp(ennTavTeksti(buf, aika(0, 12, 34) + SEK/2), L"00.12.34") == 0);
	CHECK(wcscmp(ennTavTeksti(buf, aika(0, 12, 34) + SEK - 1), L"00.12.34") == 0);
}

TEST_CASE("ennTavTeksti: alle sekunnin aika ei ole tyhja, koska arvo on annettu")
{
	wchar_t buf[ENNTAV_LEN];
	CHECK(wcscmp(ennTavTeksti(buf, 1), L"00.00.00") == 0);
}

TEST_CASE("ennTavTeksti: palauttaa annetun puskurin")
{
	wchar_t buf[ENNTAV_LEN];
	CHECK(ennTavTeksti(buf, aika(1, 0, 0)) == buf);
	CHECK(ennTavTeksti(buf, 0) == buf);
}

// ---------------------------------------------------------------------
// ennTavMuutettu
// ---------------------------------------------------------------------

TEST_CASE("ennTavMuutettu: koskematon solu ei ole muuttunut")
{
	CHECK_FALSE(ennTavMuutettu(L"01.02.03", aika(1, 2, 3)));
	CHECK_FALSE(ennTavMuutettu(L"", 0));
}

TEST_CASE("ennTavMuutettu: koskematon solu sailyttaa tallennetut sekunnin osat")
{
	// Naytossa 00.12.34, tallennettuna 12:34,5. Kun solua ei muuteta, sita ei
	// lueta takaisin, joten muiden muutosten tallennus ei pyorista arvoa.
	CHECK_FALSE(ennTavMuutettu(L"00.12.34", aika(0, 12, 34) + SEK/2));
}

TEST_CASE("ennTavMuutettu: muokattu arvo on muuttunut")
{
	CHECK(ennTavMuutettu(L"00.12.35", aika(0, 12, 34)));
	CHECK(ennTavMuutettu(L"00.45.00", 0));
}

TEST_CASE("ennTavMuutettu: tyhjennetty solu on muuttunut")
{
	CHECK(ennTavMuutettu(L"", aika(0, 45, 30)));
}

TEST_CASE("ennTavMuutettu: eri kirjoitusasu luetaan uudelleen")
{
	// Solusta poistuminen muotoilee tekstin, mutta jos tallennetaan suoraan
	// muokkauksen aikana, teksti voi olla kayttajan kirjoittamassa muodossa.
	CHECK(ennTavMuutettu(L"1:02:03", aika(1, 2, 3)));
	CHECK(ennTavMuutettu(L"45.30", aika(0, 45, 30)));
}

// ---------------------------------------------------------------------
// ennTavAvain
// ---------------------------------------------------------------------

// Vertaa kahden ajan lajitteluavaimia samoin kuin quicksort (tavuittain):
// < 0, jos a lajittuu ennen b:ta.
static int vertaa(int a, int b)
{
	char ka[ENNTAV_AVAIN], kb[ENNTAV_AVAIN];

	memset(ka, 0, sizeof(ka));
	memset(kb, 0, sizeof(kb));
	ennTavAvain(ka, a);
	ennTavAvain(kb, b);
	return(memcmp(ka, kb, ENNTAV_AVAIN));
}

TEST_CASE("ennTavAvain: ajat nousevaan jarjestykseen")
{
	CHECK(vertaa(aika(0, 0, 1), aika(0, 0, 2)) < 0);
	CHECK(vertaa(aika(0, 45, 30), aika(0, 45, 31)) < 0);
	CHECK(vertaa(aika(0, 12, 34), aika(0, 12, 34) + SEK/2) < 0);
	CHECK(vertaa(aika(1, 0, 0), aika(0, 59, 59)) > 0);
}

TEST_CASE("ennTavAvain: eri numeromaara ei sotke jarjestysta")
{
	// 9 s ja 10 s, 59:59 ja 1:00:00: lyhyempi luku lajittuu ensin
	CHECK(vertaa(aika(0, 0, 9), aika(0, 0, 10)) < 0);
	CHECK(vertaa(aika(0, 59, 59), aika(1, 0, 0)) < 0);
	CHECK(vertaa(1, aika(99, 0, 0)) < 0);
}

TEST_CASE("ennTavAvain: puuttuva aika lajittuu viimeiseksi")
{
	CHECK(vertaa(0, aika(0, 0, 1)) > 0);
	CHECK(vertaa(aika(99, 59, 59), 0) < 0);
	CHECK(vertaa(1, 0) < 0);
}

TEST_CASE("ennTavAvain: samat ajat antavat saman avaimen")
{
	CHECK(vertaa(0, 0) == 0);
	CHECK(vertaa(aika(0, 45, 30), aika(0, 45, 30)) == 0);
}

TEST_CASE("ennTavAvain: ei kirjoita avaimen alun ulkopuolelle")
{
	char key[ENNTAV_AVAIN + 4];

	memset(key, 0, sizeof(key));
	memset(key + ENNTAV_AVAIN, 'x', 4);
	ennTavAvain(key, 2147483647);
	CHECK(key[0] == '0');
	CHECK(memcmp(key + 1, "\0" "2147483647", ENNTAV_AVAIN - 1) == 0);
	CHECK(memcmp(key + ENNTAV_AVAIN, "xxxx", 4) == 0);
}
