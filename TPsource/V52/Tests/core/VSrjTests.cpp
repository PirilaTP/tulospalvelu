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

// H-002: the relay legacy series file reader (luesarjat, Juk/vSarjat.cpp:305,
// reached when KilpSrj.xml is missing) fills int paikat[] with '0' bytes
// (:376), so paikat[0] = 0x30303030, and then passes that int VALUE as the
// %s target of sscanf (:377-379 for 9 fields, :400-402 for exactly 6), so
// the string is written to address 0x30303030.
// Release build only: in the ASan build that address is ASan shadow memory.
// The test reserves the target page itself, so the write is observed
// directly (bytes compared), not via a crash. Plan: testaus.md 6 and 15.

#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include "VDeclare.h"
#include "../fixtures/VFixture.h"

int luesarjat(void);

#ifndef __SANITIZE_ADDRESS__

static char *const TARGET = (char *) 0x30303030;
static char *g_page = 0;

static void legacyFixture()
{
	REQUIRE(tp::sandboxOk());
	// Reserve the page the defect writes to before anything else allocates.
	g_page = (char *) VirtualAlloc((void *) 0x30300000, 0x10000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	REQUIRE(g_page == (char *) 0x30300000);
	memset(g_page, 0xCC, 0x10000);
	tpv::initBase();
	REQUIRE(tp::fileSize("KilpSrj.xml") == -1);   // forces the legacy KILP.SRJ path (:315-320)
	REQUIRE(tp::fileSize("KILP.SRJ") > 0);
}

static int changedBytes()
{
	int n = 0;
	for (int i = 0; i < 0x10000; i++)
		if ((unsigned char) g_page[i] != 0xCC)
			n++;
	return n;
}

// The parse must have reached the class parameter line, or the result
// means nothing: the class name and leg count were stored.
static void requireParsed()
{
	REQUIRE(strcmp(Sarjat[0].sarjanimi, "H21") == 0);
	REQUIRE(Sarjat[0].osuusluku == 1);
}

// Control: 7 numeric fields take neither %s path; the page is untouched.
TEST_CASE("H-002 control: legacy line with 7 fields writes nothing to 0x30303030")
{
	legacyFixture();
	int r = luesarjat();
	tp::info(tp::fmt("luesarjat=%d messages=%d", r, (int) tp::uiMessages().size()));
	requireParsed();
	CHECK(changedBytes() == 0);
}

static void legacyDefect(const char *mechanism, const char *token, const char *line)
{
	legacyFixture();
	int r = luesarjat();
	requireParsed();
	int changed = changedBytes();
	size_t l = strlen(token) + 1;
	bool exact = memcmp(TARGET, token, l) == 0;
	// Anything written must be exactly the token at 0x30303030.
	REQUIRE((changed == 0 || (exact && changed == (int) l)));
	tp::reportResult("H-002", mechanism, exact,
		tp::fmt("parameter line '%s': luesarjat=%d; bytes changed in the reserved page=%d; "
			"at 0x30303030: \"%.16s\" (expected \"%s\"); the address is the int value 0x30303030 of "
			"paikat[0] after memset '0'", line, r, changed, exact ? TARGET : "", token));
}

// Correct: the token is stored in Sarjat[0].paikat (or rejected).
// Defect: it is written to address 0x30303030.
TEST_CASE("H-002a relay legacy 6-field line writes through paikat[0] value")
{
	legacyDefect("sscanf-6-fields-writes-to-paikat-value", "4242", "1 1 1 0 0 4242");
}

TEST_CASE("H-002b relay legacy 9-field line writes through paikat[0] value")
{
	legacyDefect("sscanf-9-fields-writes-to-paikat-value", "TPH002", "1 1 1 0 0 0 0 0 TPH002");
}

#endif
