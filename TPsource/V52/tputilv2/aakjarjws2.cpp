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


// Muuntaa wide-merkkisen nimen hakemistojÃ¤rjestysmerkkijonoksi ilman Wâ†’V-muunnosta.
// knimi: kilpailijan nimi wide-muodossa; poistaa aateliprefiksit, muuntaa Ã¤/Ã¶/Ã¥; kÃ¤yttÃ¤Ã¤ towupper2-funktiota.
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
					case L'É' :
					case L'È' :
					case L'Ê' :
					case L'Ë' :
					case L'é' :
					case L'è' :
						*p = 'E';
						break;
					case L'Ü' :
					case L'ü' :
						*p = 'Y';
						break;
					case L'Å' :
					case L'å' :
						*p = 91;
						break;
					case L'Ä' :
					case L'ä' :
						*p = 92;
						break;
					case L'Ö' :
					case L'ö' :
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

