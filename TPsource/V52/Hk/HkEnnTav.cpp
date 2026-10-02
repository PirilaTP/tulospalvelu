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
#include <wchar.h>
#include "tptype.h"
#include "TpDef.h"
#include "HkEnnTav.h"

// Maaritelty tiedostoissa tputilv2/aikatowst_s.cpp ja tputilv2/putint_r.cpp
// ja esitelty tputil.h:ssa. tputil.h:ta ei sisallyteta, koska se tuo
// Windows- ja tietoliikenneriippuvuudet.
wchar_t *aikatowstr_s(wchar_t *as, int aika, int tt0);
void putintright(int i, char *p);

wchar_t *ennTavTeksti(wchar_t *buf, int t)
{
	if (t == 0)
		buf[0] = 0;
	else
		aikatowstr_s(buf, t/SEK, 0);
	return(buf);
}

bool ennTavMuutettu(const wchar_t *solu, int tallennettu)
{
	wchar_t buf[ENNTAV_LEN];

	return(wcscmp(solu, ennTavTeksti(buf, tallennettu)) != 0);
}

void ennTavAvain(char *key, int t)
{
	key[0] = t > 0 ? '0' : '1';
	putintright(t, key + ENNTAV_AVAIN - 1);
}
