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

#include "SID3Punch.h"

UINT32 decodeD3Siid(unsigned char sn2, unsigned char sn1, unsigned char sn0)
{
	return ((UINT32)sn2 << 16) | ((UINT32)sn1 << 8) | sn0;
}

UINT32 decodeD3SiidSI5(unsigned char sn2, unsigned char sn1, unsigned char sn0)
{
	UINT32 siid = decodeD3Siid(sn2, sn1, sn0);

	if (siid < 500000UL)
		siid = ((UINT32)sn1 << 8 | sn0) + (sn2 >= 2 ? sn2 * 100000UL : 0);
	return siid;
}

int siEtsiSanoma(const unsigned char *b, int n, int *alku, int *dlen, int *total)
{
	*alku = (b[0] == 0xFF && n >= 2 && b[1] == 0x02) ? 1 : 0;
	if (b[*alku] != 0x02)
		return SISAN_OHITA;
	if (n < *alku + 3)
		return SISAN_KESKEN;
	*dlen = b[*alku + 2];
	*total = *alku + 3 + *dlen + 3;
	if (n < *total)
		return SISAN_KESKEN;
	if (b[*total - 1] != 0x03)
		return SISAN_OHITA;
	return SISAN_OK;
}

void siPuraD3(const unsigned char *data, int dlen, SID3Leima *leima)
{
	leima->koodi = 256 * data[0] + data[1];
	leima->siid = decodeD3SiidSI5(data[3], data[4], data[5]);
	leima->korttitms = -1;
	if (dlen >= 10)
		leima->korttitms = 1000L * ((data[6] & 1) * 43200L + 256L * data[7] + data[8]) +
			(1000L * data[9]) / 256;
}

bool siToistoLeima(UINT32 edsiid, int edkoodi, INT32 edtms,
	UINT32 siid, int koodi, INT32 tms)
{
	return siid == edsiid && koodi == edkoodi &&
		(tms - edtms + 86400000L) % 86400000L < 3000L;
}
