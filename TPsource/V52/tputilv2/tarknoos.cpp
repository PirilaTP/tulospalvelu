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

// Laskee kilpailunumeron tarkistusmerkin ottaen huomioon osakilpailunumeron os.
// kno: kilpailunumero, os: osakilpailunumero; palauttaa tarkistusmerkin (0–9).
int tarkno_os(int kno, int os)
   {
   int t,s;

   t = kno / 1000;
   s = 7 * t;
   kno -= 1000 * t;
   t = kno / 100;
   s += 3 * t;
   kno -= 100 * t;
   t = kno / 10;
   s += 7 * kno - 69 * t + 3 * os;
   return((200 - s) % 10);
   }
