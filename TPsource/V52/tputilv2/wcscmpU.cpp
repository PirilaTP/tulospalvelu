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

// Vertailee kahta wide-merkkijonoa kirjainkoosta riippumattomasti towupper2-funktiolla.
// Palauttaa negatiivisen, nollan tai positiivisen arvon kuten wcscmp.
int wcscmpU(const wchar_t *s1, const wchar_t *s2)
{
	for (; *s1 || *s2; s1++, s2++)
		if (towupper2(*s1) != towupper2(*s2))
			return(towupper2(*s1) - towupper2(*s2));
	return(0);
}

// Vertailee kahta wide-merkkijonoa kirjainkoosta riippumattomasti len merkin matkalta.
// Palauttaa negatiivisen, nollan tai positiivisen arvon kuten memcmp.
int wmemcmpU(const wchar_t *s1, const wchar_t *s2, int len)
{
	for (int i = 0; i < len; i++, s1++, s2++)
		if (towupper2(*s1) != towupper2(*s2))
			return(towupper2(*s1) - towupper2(*s2));
	return(0);
}

// Vertailee kahta ANSI/OEM-merkkijonoa kirjainkoosta riippumattomasti upcasesc-funktiolla.
// Palauttaa negatiivisen, nollan tai positiivisen arvon kuten strcmp.
int strcmpU(const char *s1, const char *s2)
{
	for (; *s1 || *s2; s1++, s2++)
		if (upcasesc(*s1) != upcasesc(*s2))
			return(upcasesc(*s1) - upcasesc(*s2));
	return(0);
}

// Vertailee kahta ANSI/OEM-merkkijonoa kirjainkoosta riippumattomasti len merkin matkalta.
// Palauttaa negatiivisen, nollan tai positiivisen arvon kuten memcmp.
int memcmpU(const char *s1, const char *s2, int len)
{
	for (int i = 0; i < len; i++, s1++, s2++)
		if (upcasesc(*s1) != upcasesc(*s2))
			return(upcasesc(*s1) - upcasesc(*s2));
	return(0);
}
