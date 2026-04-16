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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <tputil.h>


char *w_aakjarjstr(char *jrjstr, wchar_t *nimi, int len, int wasv, int aatelifl)
   {
   char *p,*pmax,*knimi, *pknimi;
   char *aatelit[] = {"VON ", "VAN DER ", "VAN ", "AV ", "AF ",
		"DE LA ", "DE "};
   unsigned int n;

   if ((knimi = (char *) malloc(len+6)) == NULL) {
	   *jrjstr = 0;
	  return(jrjstr);
	  }
   pknimi = knimi;
   memset(knimi, 0, len+6);
   wcstooem(knimi, nimi, len+5);
   upcasestr(knimi);
   if (aatelifl) {
	   for (n = 0; n < (sizeof(aatelit)/sizeof(char *)); n++) {
		   if (!memcmp(aatelit[n], knimi, strlen(aatelit[n])) && strlen(knimi) > strlen(aatelit[n])+1) {
			   memmove(knimi, knimi+strlen(aatelit[n]), strlen(knimi+strlen(aatelit[n])+1));
			   memset(knimi+strlen(knimi), 0, len+6-strlen(knimi));
			   }
		   }
	   }

   p = jrjstr;
   pmax = p + len;

   while (*knimi != 0 && p < pmax) {
	  if (*knimi != '-') {
		 if (*knimi > 127) {
			switch (*knimi) {
			   case 200 :
			   case 201 :
			   case 202 :
			   case 203 :
			   case 232 :
			   case 233 :
			   case 234 :
			   case 235 :
			   case 0x82 :
			   case 0x90 : *p = 'E'; break;
			   case 220 :
			   case 252 :
			   case 0x81 :
			   case 0x9A : *p = 'Y'; break;
			   case 197 :
			   case 229 :
			   case 0x86 :
			   case 0x8F : *p = 91; break;
			   case 196 :
			   case 228 :
			   case 0x84 :
			   case 0x8E : *p = 92; break;
			   case 214 :
			   case 246 :
			   case 0x94 :
			   case 0x99 : *p = 93; break;
			   default  : *p = 93;
			   }
			}
		 else if (*knimi == '_') *p = ' ';
		 else if (wasv && *knimi == 'W') *p = 'V';
		 else *p = *knimi;
		 p++;
		 }
	  knimi++;
	  }
   free(pknimi);
   return(jrjstr);
   }
