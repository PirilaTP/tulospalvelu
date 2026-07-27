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

// Apuvalineet wchar_t-merkkijonojen vertailuun doctestissa.
//
// doctest ei osaa oletuksena tulostaa std::wstringia, joten pelkka
// CHECK(w == L"...") kertoisi failatessaan vain "is NOT correct!" ilman
// odotettua ja saatua arvoa. StringMaker-erikoistus muuntaa wide-merkkijonon
// UTF-8:ksi virheilmoitusta varten, jolloin tuloste on luettava. Vertailu
// tehdaan silti wide-merkkijonoina, joten merkistokoodaus ei sotke sita.

#ifndef WIDESTRING_TESTS_DEFINED
#define WIDESTRING_TESTS_DEFINED

#include <string>
#include "doctest.h"

// Muuntaa wide-merkkijonon UTF-8:ksi. Toteutettu kasin, jotta testiapuri ei
// riipu Windows-API:sta ja toimii seka 2- etta 4-tavuisella wchar_t:lla.
inline std::string WideToUtf8(const std::wstring& in)
{
	std::string out;
	for (size_t i = 0; i < in.size(); i++) {
		unsigned long c = (unsigned long)(unsigned int) in[i];
		if (c >= 0xD800 && c <= 0xDBFF && i + 1 < in.size()) {   // UTF-16-surrogaattipari
			unsigned long lo = (unsigned long)(unsigned int) in[i+1];
			if (lo >= 0xDC00 && lo <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (lo - 0xDC00);
				i++;
				}
			}
		if (c < 0x80)
			out += (char) c;
		else if (c < 0x800) {
			out += (char)(0xC0 | (c >> 6));
			out += (char)(0x80 | (c & 0x3F));
			}
		else if (c < 0x10000) {
			out += (char)(0xE0 | (c >> 12));
			out += (char)(0x80 | ((c >> 6) & 0x3F));
			out += (char)(0x80 | (c & 0x3F));
			}
		else {
			out += (char)(0xF0 | (c >> 18));
			out += (char)(0x80 | ((c >> 12) & 0x3F));
			out += (char)(0x80 | ((c >> 6) & 0x3F));
			out += (char)(0x80 | (c & 0x3F));
			}
		}
	return out;
}

namespace doctest {
template<> struct StringMaker<std::wstring> {
	static String convert(const std::wstring& in) {
		std::string s = WideToUtf8(in);
		return String(s.c_str());
		}
	};
}

// Vertaa kahta wide-merkkijonoa niin, etta virheilmoitus nayttaa molemmat.
#define CHECK_WSTR(saatu, odotettu) CHECK(std::wstring(saatu) == std::wstring(odotettu))

#endif
