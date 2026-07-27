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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <tptype.h>
#include <TpDef.h>

#include "VOtsikot.h"

// aikatowstr_ts esitellaan myos tputil.h:ssa, mutta sita ei voi sisallyttaa
// tahan: se vetaa mukanaan windows.h:n ja tekisi tasta kaannosyksikosta
// testeihin kaantymattoman.
extern wchar_t *aikatowstr_ts(wchar_t *as, INT32 aika, int tt0);

// Skandit \u-escapeina, jotta lahdetiedoston merkistokoodaus ei vaikuta
// tulosteeseen: "Lahto: ". Muu koodi on ISO-8859-1:ta, mutta tama yksikko
// kaannetaan myos testeissa, joissa kaantaja olettaa UTF-8:n.
static const wchar_t *LAHTO_FI = L"L\u00e4ht\u00f6: ";
static const wchar_t *LAHTO_EN = L"Start: ";

void LisaaLahtoaikaTeksti(wchar_t *wline, size_t cap, INT32 lahto, int t0, int language)
{
	if (lahto == TMAALI0)          // vain sentinel tarkoittaa "ei lahtoaikaa"
		return;

	wchar_t wtm[14];
	AIKATOWSTRS(wtm, lahto, t0);
	wtm[8] = 0;                    // tt.mm.ss, sekunnin osat pois

	// Nipista rivin loppuvalilyonnit, jottei pilkku jaa roikkumaan niiden peraan.
	size_t len = wcslen(wline);
	while (len > 0 && wline[len-1] == L' ')
		len--;
	wline[len] = 0;

	const wchar_t *sep = len ? L", " : L"";
	const wchar_t *otsake = language > 0 ? LAHTO_EN : LAHTO_FI;

	// Kirjoitetaan vain jos koko lisays mahtuu; muuten rivi jaa ennalleen.
	if (len + wcslen(sep) + wcslen(otsake) + wcslen(wtm) + 1 > cap)
		return;
	wcscat(wline, sep);
	wcscat(wline, otsake);
	wcscat(wline, wtm);
}
