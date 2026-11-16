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

// Henkilotietokannan (Henkilot.csv) apufunktiot: tietojen paivitys
// kilpailijatiedoista ja Yhdistys-sarakkeen loytaminen otsikkorivilta.
// Ei VCL-riippuvuuksia, jotta funktiot voidaan yksikkotestata
// (Tests/HkHenkKopioTest.cpp).

#ifndef HkHenkKopioH
#define HkHenkKopioH

#include <wchar.h>

// Kopioi src-merkkijonon kohteeseen dst (koko dstSize merkkia loppunolla
// mukaan lukien), mutta vain jos src ei ole tyhja. Tyhja arvo ei siis
// pyyhi tietokantaan jo tallennettua tietoa.
inline void kopioJosEiTyhja(wchar_t *dst, int dstSize, const wchar_t *src)
{
	if (src && src[0] && dstSize > 0) {
		wcsncpy(dst, src, dstSize-1);
		dst[dstSize-1] = 0;
		}
}

// Palauttaa nimen nimi paikan (0-pohjainen) otsikkorivin kenttien joukossa
// tai -1, jos sita ei ole. Kentat ovat nfld pituinen taulukko.
inline int etsiOtsikkoKolumni(wchar_t **fields, int nfld, const wchar_t *nimi)
{
	for (int i = 0; i < nfld; i++) {
		if (fields[i] && wcscmp(fields[i], nimi) == 0)
			return(i);
		}
	return(-1);
}

#endif
