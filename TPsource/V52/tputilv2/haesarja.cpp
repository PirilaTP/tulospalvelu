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

#if defined(__BORLANDC__)
#pragma -K -a1
#endif
#include <string.h>
#include <bstrings.h>
#include <tputil.h>
#define TRUE 1
#define FALSE 0


extern int sarjaluku;
extern char sarjanimi[][11];

// Etsii sarjan nimen perusteella sarjataulukosta ja palauttaa sen indeksin.
// Parametri: snimi=sarjan nimi (muunnetaan isoksi ja lyhennetään 6 merkkiin).
// Palauttaa sarjaindeksin tai -1, jos sarjaa ei löydy.
int haesarja(char *snimi)
{
   int i;

   i = 0;
   upcasestr(snimi);
   stpcvt(snimi, 6);
   do {
      if (!memcmp(snimi, sarjanimi[i], strlen(sarjanimi[i]) + 1))
         return(i);
   } while (++i < sarjaluku);
   return(-1);
}



