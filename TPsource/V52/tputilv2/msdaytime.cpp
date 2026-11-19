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

#include <tptype.h>
#include <stdio.h>
#include <windows.h>

// Palauttaa paikallisen ajan millisekunteina vuorokauden alusta (tunti*3600000 + ...).
long msdaytime(void)
   {
   SYSTEMTIME stm;

   GetLocalTime(&stm);
   return(3600000L*stm.wHour + 60000L*stm.wMinute + 1000L*stm.wSecond +
	  stm.wMilliseconds);
   }

// Palauttaa nykyisen paikallisen ajan ISO 8601 -muodossa laajamerkki-jonona.
// Parametrit: des=desimaalisekuntien määrä (0–3), zone=lisätäänkö aikavyöhyke.
// Palauttaa osoittimen staattiseen laajamerkki-puskuriin.
wchar_t *ISOdatetime(int des, bool zone)
{
   SYSTEMTIME stm, sstm;
   TIME_ZONE_INFORMATION tz;
   static wchar_t ln[100];
   int bias;


   GetSystemTime(&sstm);
   GetLocalTime(&stm);
   GetTimeZoneInformation(&tz);
   swprintf(ln, L"%04d-%02d-%02dT%02d:%02d:%02d.%03d", stm.wYear, stm.wMonth,
	   stm.wDay, stm.wHour, stm.wMinute, stm.wSecond, stm.wMilliseconds);
   if (des > 0 && des <= 3)
	   ln[20 + des] = 0;
   else
	   ln[19] = 0;
   bias = (sstm.wHour - stm.wHour + 36) % 24 - 12;
   if (zone)
	   swprintf(ln+wcslen(ln), L"%c%02d:00", bias > 0 ? L'-' : L'+',  abs(bias));
   return(ln);
}

// Aikavyöhykkeen siirtymä annetulle paikalliselle päivälle ja kellonajalle, esim. L"+03:00".
// Kesäaika määräytyy annetun päivän mukaan (ei nykyhetken), joten syyskuun kilpailu saa
// kesäajan siirtymän, vaikka tiedosto kirjoitettaisiin talvella.
// pvm: "YYYY-MM-DD..." (esim. ISO-päivämäärä tai -aikaleima) tai "D.M.YYYY",
// aika: "HH:MM..." tai NULL (klo 12).
// Jos päivämäärää ei voi tulkita, palautetaan nykyhetken siirtymä. Palauttaa staattisen puskurin.
wchar_t *ISOzoneLocal(const wchar_t *pvm, const wchar_t *aika)
{
   static wchar_t zone[12];
   SYSTEMTIME lt, ut;
   FILETIME lf, uf;
   int y = 0, mo = 0, d = 0, h = 12, mi = 0, minuutit;
   __int64 ero;

   if (pvm != NULL && swscanf(pvm, L"%d-%d-%d", &y, &mo, &d) != 3 &&
	   swscanf(pvm, L"%d.%d.%d", &d, &mo, &y) != 3)
	   y = 0;
   if (pvm == NULL || y < 1601 || mo < 1 || mo > 12 || d < 1 || d > 31) {
	   wchar_t *nyt = ISOdatetime(0, true);
	   wcscpy(zone, nyt + wcslen(nyt) - 6);
	   return(zone);
	   }
   if (aika)
	   swscanf(aika, L"%d:%d", &h, &mi);
   memset(&lt, 0, sizeof(lt));
   lt.wYear = (WORD) y;
   lt.wMonth = (WORD) mo;
   lt.wDay = (WORD) d;
   lt.wHour = (WORD) (h >= 0 && h < 24 ? h : 12);
   lt.wMinute = (WORD) (mi >= 0 && mi < 60 ? mi : 0);
   if (!TzSpecificLocalTimeToSystemTime(NULL, &lt, &ut) ||
	   !SystemTimeToFileTime(&lt, &lf) || !SystemTimeToFileTime(&ut, &uf)) {
	   wchar_t *nyt = ISOdatetime(0, true);
	   wcscpy(zone, nyt + wcslen(nyt) - 6);
	   return(zone);
	   }
   // FILETIME on 100 ns:n yksiköissä: paikallinen - UTC = siirtymä
   ero = (((__int64) lf.dwHighDateTime << 32) | lf.dwLowDateTime) -
	   (((__int64) uf.dwHighDateTime << 32) | uf.dwLowDateTime);
   minuutit = (int) (ero / 600000000);
   swprintf(zone, L"%c%02d:%02d", minuutit < 0 ? L'-' : L'+', abs(minuutit) / 60, abs(minuutit) % 60);
   return(zone);
}


