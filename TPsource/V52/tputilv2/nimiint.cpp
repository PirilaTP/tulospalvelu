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

#include "tputil.h"

// Laskee nimen kolmen ensimmäisen kirjaimen perusteella numeerisen tiivisteen aakkosjärjestystä varten.
// nimi: OEM-merkistön mukainen nimi; palauttaa luvun väliltä 0–64000.
unsigned int nimiint(char *nimi)
{
   char *ns;

   ns = aakjarjstr(nimi);
   return(1600 * chint(ns[0]) + 40 * chint(ns[1]) + chint(ns[2]));
}

// Laajamerkkiversio nimiint-funktiosta: muuntaa laajamerkki-nimen ensin OEM-merkistöön.
// wnimi: laajamerkki-nimi; palauttaa saman numeerisen tiivisteen kuin nimiint.
unsigned int wnimiint(wchar_t *wnimi)
{
	char nimi[62];

	wcstooem(nimi, wnimi, 60);
	return(nimiint(nimi));
}



