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

// H-001: relay text result output clears the 40-byte local stsk[20] with
// memset(stsk, 0, sizeof(st)), where st is wchar_t[80] (160 bytes):
// Juk/VTulostus.cpp:2228 in textjtulos and :3981 in textosuus (both
// static; the public entry tulostasarja needs a full result index).
// This file includes Juk/VTulostus.cpp so the static functions can be
// called directly; TpCoreJuk leaves VTulostus.cpp out of its source list.
// Nothing in VTulostus.cpp is changed. The overrun is shown with
// AddressSanitizer (TpCoreJuk Asan build); the plain Release build has
// only the C4789 compile-time warning. Plan: testaus.md sections 6 and 15.

#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include "../../Juk/VTulostus.cpp"
#include "../fixtures/VFixture.h"

#ifdef __SANITIZE_ADDRESS__

// One class, one leg, shooting places on leg 1 (paikat[0] != 0, the
// condition at :2227 and :3980). No output file: lstf stays NULL.
static void outputFixture()
{
	REQUIRE(tp::sandboxOk());
	tpv::initBase();
	REQUIRE(kilpparam.maxnosuus == 1);   // textjtulos returns early otherwise (:2193)
	sarjaluku = 1;
	Sarjat[0].osuusluku = 1;
	Sarjat[0].ntosuus = 1;
	Sarjat[0].nosuus[0] = 1;
	Sarjat[0].paikat[0] = 1;
}

static tulostusparamtp g_prm;   // static: about 20 KB

// Correct: the memset stays inside stsk. Defect: ASan reports a
// stack-buffer-overflow WRITE of 160 bytes (sizeof(st)) at the memset line
// in this function. The variable is not used as evidence: MSVC x86 ASan
// lists this frame's objects with overlapping offsets, so its attribution
// to a variable is not reliable here.
TEST_CASE("H-001a relay textjtulos clears past stsk (ASan)")
{
	outputFixture();
	kilptietue k;
	k.nollaa();
	k.sarja = 0;
	g_prm.kohde = L'E';
	INT (*volatile f)(kilptietue *, tulostusparamtp *, int, INT) = textjtulos;
	static const char *const expect[] = { "stack-buffer-overflow", "WRITE of size 160",
		"VTulostus.cpp:2228 in textjtulos", 0 };
	tp::armCrash("H-001", "textjtulos-memset-past-stsk", "asan", expect);
	f(&k, &g_prm, 0, 1);
	tp::disarmCrash();
	tp::reportResult("H-001", "textjtulos-memset-past-stsk", false, "no ASan report from textjtulos");
}

TEST_CASE("H-001b relay textosuus clears past stsk (ASan)")
{
	outputFixture();
	kilptietue k;
	k.nollaa();
	k.sarja = 0;
	g_prm.kohde = L'I';
	g_prm.tulostettava = L'H';
	void (*volatile f)(kilptietue *, tulostusparamtp *, int, int) = textosuus;
	static const char *const expect[] = { "stack-buffer-overflow", "WRITE of size 160",
		"VTulostus.cpp:3981 in textosuus", 0 };
	tp::armCrash("H-001", "textosuus-memset-past-stsk", "asan", expect);
	f(&k, &g_prm, 0, 1);
	tp::disarmCrash();
	tp::reportResult("H-001", "textosuus-memset-past-stsk", false, "no ASan report from textosuus");
}

#endif
