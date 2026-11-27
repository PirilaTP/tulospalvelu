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

// Yksikkotestit aikavyohykkeen siirtymalle annetulle paivalle (tputilv2/msdaytime.cpp:
// ISOzoneLocal), jota IOF XML 3.0 -vienti kayttaa. Tulos riippuu koneen aikavyohykkeesta:
// muoto tarkistetaan aina, kesa- ja talviajan ero vain, jos vyohykkeessa on kesaaika.

#include <windows.h>
#include <wchar.h>
#include <wctype.h>
#include "doctest.h"

wchar_t *ISOzoneLocal(const wchar_t *pvm, const wchar_t *aika);

static bool muotoOk(const wchar_t *z)
{
	return(wcslen(z) == 6 && (z[0] == L'+' || z[0] == L'-') && iswdigit(z[1]) && iswdigit(z[2]) &&
		z[3] == L':' && iswdigit(z[4]) && iswdigit(z[5]));
}

TEST_CASE("ISOzoneLocal: siirtyma on muotoa +HH:MM kaikilla syotteilla")
{
	wchar_t z[12];

	wcscpy(z, ISOzoneLocal(L"2026-07-01", L"12:00:00"));
	CHECK(muotoOk(z));
	wcscpy(z, ISOzoneLocal(L"2026-07-01T20:45:00", L"20:45:00"));
	CHECK(muotoOk(z));
	wcscpy(z, ISOzoneLocal(L"1.7.2026", L"12:00"));
	CHECK(muotoOk(z));
	// Tulkitsematon tai puuttuva paivamaara: nykyhetken siirtyma
	wcscpy(z, ISOzoneLocal(NULL, NULL));
	CHECK(muotoOk(z));
	wcscpy(z, ISOzoneLocal(L"ei paivamaaraa", NULL));
	CHECK(muotoOk(z));
}

TEST_CASE("ISOzoneLocal: siirtyma maaraytyy annetun paivan mukaan, ei nykyhetken")
{
	TIME_ZONE_INFORMATION tz;
	DWORD r = GetTimeZoneInformation(&tz);
	wchar_t kesa[12], talvi[12];

	wcscpy(kesa, ISOzoneLocal(L"2026-07-01", L"12:00"));
	wcscpy(talvi, ISOzoneLocal(L"2026-01-15", L"12:00"));
	// Paivamaaran molemmat kirjoitustavat antavat saman tuloksen
	CHECK(wcscmp(ISOzoneLocal(L"1.7.2026", L"12:00"), kesa) == 0);
	CHECK(wcscmp(ISOzoneLocal(L"15.1.2026", L"12:00"), talvi) == 0);
	if (r != TIME_ZONE_ID_INVALID && tz.DaylightDate.wMonth != 0 && tz.DaylightBias != 0)
		CHECK(wcscmp(kesa, talvi) != 0);
	else
		CHECK(wcscmp(kesa, talvi) == 0);
}