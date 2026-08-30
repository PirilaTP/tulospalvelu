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

// Yksikkotestit "ensimmainen maaliin kaynnistaa seuraavan osuuden"
// -paatoslogiikalle (Juk/VRinnakkaisOsuus.cpp).

#include "doctest.h"
#include "VRinnakkaisOsuus.h"

// ---------------------------------------------------------------------
// EkaMaaliIndeksi
// ---------------------------------------------------------------------

TEST_CASE("EkaMaaliIndeksi: tyhja taulukko palauttaa -1")
{
	CHECK(EkaMaaliIndeksi(nullptr, 0) == -1);
}

TEST_CASE("EkaMaaliIndeksi: yksi kilpailija, ei viela maalissa")
{
	RinnakkaisTila osat[1] = { { true, false, 0 } };
	CHECK(EkaMaaliIndeksi(osat, 1) == -1);
}

TEST_CASE("EkaMaaliIndeksi: yksi kilpailija, maalissa, riittaa heti")
{
	RinnakkaisTila osat[1] = { { true, true, 500 } };
	CHECK(EkaMaaliIndeksi(osat, 1) == 0);
}

TEST_CASE("EkaMaaliIndeksi: kolme kilpailijaa, vain keskimmainen maalissa")
{
	RinnakkaisTila osat[3] = {
		{ true, false, 0 },
		{ true, true, 700 },
		{ true, false, 0 },
		};
	CHECK(EkaMaaliIndeksi(osat, 3) == 1);
}

// Regressiotesti nykyista (ennen muutosta) kayttaytymista vastaan: vanha
// aTulos() valitsi SUURIMMAN ajan (hitaimman). Tama testi varmistaa etta
// uusi logiikka valitsee PIENIMMAN (nopeimman), ei suurinta.
TEST_CASE("EkaMaaliIndeksi: valitsee nopeimman, ei hitainta")
{
	RinnakkaisTila osat[3] = {
		{ true, true, 900 },
		{ true, true, 300 },
		{ true, true, 600 },
		};
	int i = EkaMaaliIndeksi(osat, 3);
	CHECK(i == 1);
	CHECK(osat[i].kulunutAika == 300);
}

TEST_CASE("EkaMaaliIndeksi: tasapelissa valitsee ensimmaisen loydetyn")
{
	RinnakkaisTila osat[3] = {
		{ true, true, 400 },
		{ true, true, 400 },
		{ true, false, 0 },
		};
	CHECK(EkaMaaliIndeksi(osat, 3) == 0);
}

TEST_CASE("EkaMaaliIndeksi: kaytossa olematon paikka ei koskaan voita")
{
	RinnakkaisTila osat[2] = {
		{ false, true, 100 },
		{ true, true, 900 },
		};
	CHECK(EkaMaaliIndeksi(osat, 2) == 1);
}

TEST_CASE("EkaMaaliIndeksi: vain yksi ilmoitettu kolmesta paikasta - se riittaa")
{
	RinnakkaisTila osat[3] = {
		{ false, false, 0 },
		{ true, true, 250 },
		{ false, false, 0 },
		};
	CHECK(EkaMaaliIndeksi(osat, 3) == 1);
}

TEST_CASE("EkaMaaliIndeksi: ei ketaan ilmoitettu, kukaan ei voi voittaa")
{
	RinnakkaisTila osat[2] = {
		{ false, false, 0 },
		{ false, false, 0 },
		};
	CHECK(EkaMaaliIndeksi(osat, 2) == -1);
}

// ---------------------------------------------------------------------
// PuutelisaNollataan
// ---------------------------------------------------------------------

TEST_CASE("PuutelisaNollataan: ei ketaan ilmoitettu - ei koskaan odoteta")
{
	CHECK(PuutelisaNollataan(0, 0, false) == true);
	CHECK(PuutelisaNollataan(0, 0, true) == true);
}

TEST_CASE("PuutelisaNollataan: vanha kaytos - odottaa kaikkia rekisteroityja")
{
	CHECK(PuutelisaNollataan(3, 3, false) == true);
	CHECK(PuutelisaNollataan(3, 2, false) == false);
	CHECK(PuutelisaNollataan(3, 0, false) == false);
}

TEST_CASE("PuutelisaNollataan: eka-maaliin -saanto - yksikin riittaa")
{
	CHECK(PuutelisaNollataan(3, 1, true) == true);
	CHECK(PuutelisaNollataan(3, 2, true) == true);
	CHECK(PuutelisaNollataan(3, 3, true) == true);
	CHECK(PuutelisaNollataan(3, 0, true) == false);
}

TEST_CASE("PuutelisaNollataan: yksi ilmoitettu paikka toimii molemmilla saannoilla")
{
	CHECK(PuutelisaNollataan(1, 1, true) == true);
	CHECK(PuutelisaNollataan(1, 1, false) == true);
	CHECK(PuutelisaNollataan(1, 0, true) == false);
	CHECK(PuutelisaNollataan(1, 0, false) == false);
}
