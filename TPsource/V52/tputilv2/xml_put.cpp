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

// Kirjoittaa XML-elementin ANSI-merkkijonoina FILE-tiedostoon käyttäen XML-entiteettejä.

#if defined(__BORLANDC__)
#pragma -K -a1
#endif
#include <stdio.h>
#include <tputil.h>

// Kirjoittaa XML-merkkijono-elementin FILE-tiedostoon; muuntaa OEM→ANSI ja &/</>-merkit entiteeteiksi.
// vafile: tiedosto, tag: elementin nimi, value: tekstiarvo.
void put_xml_s(FILE *vafile, char *tag, char *value)
   {
	char *p1, ch[2] = " ";

	if (!value[0])
	   fprintf(vafile, "<%s></%s>\n", tag, tag);
	else {
	   fprintf(vafile, "<%s>", tag);
		for (p1 = value; *p1; p1++) {
			ch[0] = *p1;
		   oemtoansi(ch, 0);
			switch (ch[0]) {
				case '&' :
					fputs("&amp;", vafile);
					break;
				case '<' :
					fputs("&lt;", vafile);
					break;
				case '>' :
					fputs("&gt;", vafile);
					break;
				default :
					fputc(ch[0], vafile);
					break;
				}
			}
		fprintf(vafile, "</%s>\n", tag);
		}
   }

// Kirjoittaa XML-kokonaislukuelementin FILE-tiedostoon muodossa <tag>value</tag>.
// vafile: tiedosto, tag: elementin nimi, value: INT32-arvo.
void put_xml_d(FILE *vafile, char *tag, INT32 value)
   {
   fprintf(vafile, "<%s>%ld</%s>\n", tag, value, tag);
   }

// Kirjoittaa XML-avaustunnisteen <tag> FILE-tiedostoon.
// vafile: tiedosto, tag: elementin nimi.
void put_tag(FILE *vafile, char *tag)
   {
   fprintf(vafile, "<%s>\n", tag);
   }

// Kirjoittaa XML-lopetustunnisteen </tag> FILE-tiedostoon.
// vafile: tiedosto, tag: elementin nimi.
void put_antitag(FILE *vafile, char *tag)
   {
   fprintf(vafile, "</%s>\n", tag);
	}


