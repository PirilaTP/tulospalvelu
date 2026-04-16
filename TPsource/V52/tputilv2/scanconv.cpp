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

char *scanconv(char *line0)
   {
   char *line;

   for (line = line0; *line; line++) {
      switch (*line) {
         case '\x5b':
         case '\xc4':
            *line = 0x8E;
            break;
         case '\x5c':
         case '\xd6':
            *line = 0x99;
            break;
         case '\x5d':
         case '\xc5':
            *line = 0x8F;
            break;
         case '\x5e':
         case '\xdc':
            *line = 0x9A;
            break;
         case '\xc9':
            *line = 0x90;
            break;
         case '\x7b':
         case '\xe4':
            *line = 0x84;
            break;
         case '\x7c':
         case '\xf6':
            *line = 0x94;
            break;
         case '\x7d':
         case '\xe5':
            *line = 0x86;
            break;
         case '\x7e':
         case '\xfc':
            *line = 0x81;
            break;
         case '\xe9':
            *line = 0x82;
            break;
         }
      }
   return(line0);
   }
