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

#include <string.h>
#include <ctype.h>
#include <tputil.h>


wchar_t *aakjarjwstr2(wchar_t *knimi)
	{
	wchar_t *p,*pmax,ch1,ch2;
	static wchar_t s[61] = L"                                                            ";

	pmax = s + 60;
	p = s;
	if (knimi[2] == L' ' && (((ch1 = towupper(knimi[0])) == L'A' &&
		((ch2 = towupper(knimi[1])) == L'V' || ch2 == L'F')) ||
		(ch1 == L'D' && towupper(knimi[1]) == L'E')) && knimi[3] > L' ')
		knimi += 3;
	if (knimi[3] == L' ' && towupper(knimi[0]) == L'V' &&
		towupper(knimi[2]) == L'N' &&
		((ch1 = towupper(knimi[1])) == L'O' || ch1 == L'A') && knimi[4] > L' ')
		knimi += 4;
	while (*knimi != 0 && p < pmax) {
		if (*knimi != L'-') {
			if (*knimi > 127) {
				switch (*knimi) {
					case L'\u00C9' : // É
					case L'\u00C8' : // È
					case L'\u00CA' : // Ê
					case L'\u00CB' : // Ë
					case L'\u00E9' : // é
					case L'\u00E8' : // è
						*p = 'E';
						break;
					case L'\u00DC' : // Ü
					case L'\u00FC' : // ü
						*p = 'Y';
						break;
					case L'\u00C5' : // Å
					case L'\u00E5' : // å
						*p = 91;
						break;
					case L'\u00C4' : // Ä
					case L'\u00E4' : // ä
						*p = 92;
						break;
					case L'\u00D6' : // Ö
					case L'\u00F6' : // ö
						*p = 93;
						break;
					default  :
						*p = 94;
					}
				}
			else if (*knimi == L'_')
				*p = L' ';
			else
				*p = towupper2(*knimi);
			p++;
			}
		knimi++;
		}
	return(s);
	}

