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

// Sarjaotsikoiden tekstinmuodostus. Tama kaannosyksikko on tarkoituksella
// riippumaton globaaleista muuttujista, tietokannasta ja VCL:sta, jotta se
// voidaan kaantaa yksikkotesteihin (ks. Tests/VOtsikotTest.cpp).
// Kutsujat VTulostus.cpp:ssa poimivat parametrit globaaleista.

#ifndef VOTSIKOT_DEFINED
#define VOTSIKOT_DEFINED

#include <stddef.h>
#include <tptype.h>

// Lisaa sarjan lahtoajan otsikkoriville muodossa "Lahto: tt.mm.ss"
// (language > 0: "Start: tt.mm.ss").
//
// wline    otsikkorivi, jonka peraan lisataan (in/out, nollaterminoitu)
// cap      wlinen kapasiteetti merkkeina; jos lisays ei mahdu, rivi jaa ennalleen
// lahto    sarjan lahtoaika offsettina t0-nollahetkesta, 1/1000 s.
//          HUOM: 0 on taysin laillinen lahtoaika (sarja lahtee tasan t0-hetkella).
//          Vain TMAALI0 tarkoittaa "lahtoaikaa ei ole asetettu".
// t0       kilpailun nollahetki tunteina (AIKATOWSTRS:n tt0-parametri)
// language 0 = suomi, > 0 = englanti
//
// Jos rivin lopussa on jo tekstia, lahtoaika erotetaan siita pilkulla. Rivin
// lopun valilyonnit nipistetaan pois, jottei pilkku jaa roikkumaan niiden peraan.
void LisaaLahtoaikaTeksti(wchar_t *wline, size_t cap, INT32 lahto, int t0, int language);

#endif
