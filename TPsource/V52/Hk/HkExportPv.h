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

// Paiva/Vaihe-valinnan muuttaminen vaihevaliksi Tietojen siirto tiedostoon
// -toiminnossa (cbHk/UnitExport.cpp). Ei VCL-riippuvuuksia, jotta funktio
// voidaan yksikkotestata (Tests/HkExportPvTest.cpp).

#ifndef HkExportPvH
#define HkExportPvH

// Muuntaa Paiva/Vaihe-valintalaatikon valinnan (sel: 0 = kaikki, k = vaihe k,
// k = 1..nPvAkt) kirjoitettavien vaiheiden valiksi [*pvMin, *pvMax), jossa
// vaiheet on numeroitu nollasta. Jos valinta ei ole kelvollinen vaihe
// (sel <= 0 tai sel > nPvAkt) kirjoitetaan kaikki vaiheet.
inline void exportPvRange(int sel, int nPvAkt, int *pvMin, int *pvMax)
{
	if (sel > 0 && sel <= nPvAkt) {
		*pvMin = sel - 1;
		*pvMax = sel;
		}
	else {
		*pvMin = 0;
		*pvMax = nPvAkt;
		}
}

#endif
