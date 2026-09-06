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

// Unit tests for parsing the SportIdent Center REST API's /punches
// response (Tp/SICenterJson.cpp) and for the control-code->piste lookup.

#include <string.h>
#include "SICenterJson.h"
#include "doctest.h"

TEST_CASE("parseSIPunches: empty array returns 0")
{
	SIPunchTp out[10];
	CHECK(parseSIPunches("[]", out, 10) == 0);
}

TEST_CASE("parseSIPunches: one punch, all fields parsed correctly")
{
	SIPunchTp out[10];
	const char *json =
		"[{\"id\":123,\"card\":\"1009090\",\"time\":1684987808000,"
		"\"code\":31,\"type\":\"Control\",\"modem\":\"9000041\","
		"\"receptionTimeUtc\":1684987809000}]";

	REQUIRE(parseSIPunches(json, out, 10) == 1);
	CHECK(out[0].id == 123);
	CHECK(!strcmp(out[0].card, "1009090"));
	CHECK(out[0].time == 1684987808000LL);
	CHECK(out[0].code == 31);
	CHECK(!strcmp(out[0].type, "Control"));
	CHECK(!strcmp(out[0].modem, "9000041"));
	CHECK(out[0].receptionTimeUtc == 1684987809000LL);
}

TEST_CASE("parseSIPunches: multiple punches are parsed in order")
{
	SIPunchTp out[10];
	const char *json =
		"[{\"id\":1,\"card\":\"100\",\"time\":1,\"code\":31,\"type\":\"Start\","
		"\"modem\":\"9000041\",\"receptionTimeUtc\":1},"
		"{\"id\":2,\"card\":\"100\",\"time\":2,\"code\":0,\"type\":\"Finish\","
		"\"modem\":\"9000041\",\"receptionTimeUtc\":2}]";

	REQUIRE(parseSIPunches(json, out, 10) == 2);
	CHECK(out[0].id == 1);
	CHECK(!strcmp(out[0].type, "Start"));
	CHECK(out[1].id == 2);
	CHECK(!strcmp(out[1].type, "Finish"));
}

TEST_CASE("parseSIPunches: key order does not affect the result")
{
	SIPunchTp out[10];
	// same fields as above but in a different order
	const char *json =
		"[{\"type\":\"Control\",\"code\":31,\"id\":123,"
		"\"receptionTimeUtc\":1684987809000,\"card\":\"1009090\","
		"\"time\":1684987808000,\"modem\":\"9000041\"}]";

	REQUIRE(parseSIPunches(json, out, 10) == 1);
	CHECK(out[0].id == 123);
	CHECK(!strcmp(out[0].card, "1009090"));
	CHECK(out[0].code == 31);
}

TEST_CASE("parseSIPunches: unknown keys are ignored")
{
	SIPunchTp out[10];
	const char *json =
		"[{\"id\":1,\"card\":\"100\",\"time\":1,\"code\":0,\"type\":\"Unknown\","
		"\"modem\":\"9000041\",\"receptionTimeUtc\":1,"
		"\"someNewField\":\"jotain\",\"anotherNumber\":42}]";

	REQUIRE(parseSIPunches(json, out, 10) == 1);
	CHECK(out[0].id == 1);
	CHECK(!strcmp(out[0].type, "Unknown"));
}

TEST_CASE("parseSIPunches: a bare numeric card (not a quoted string) is accepted")
{
	SIPunchTp out[10];
	const char *json =
		"[{\"id\":5581592,\"card\":8647177,\"time\":1717300000000,"
		"\"code\":31,\"type\":\"Control\",\"modem\":\"9000041\","
		"\"receptionTimeUtc\":1717300001000}]";

	REQUIRE(parseSIPunches(json, out, 10) == 1);
	CHECK(!strcmp(out[0].card, "8647177"));
}

TEST_CASE("parseSIPunches: an unexpected float or nested value in an unknown field doesn't break parsing")
{
	SIPunchTp out[10];
	const char *json =
		"[{\"id\":1,\"card\":\"100\",\"time\":1,\"code\":31,\"type\":\"Control\","
		"\"modem\":\"9000041\",\"receptionTimeUtc\":1,"
		"\"signalQuality\":97.5,\"verified\":true,"
		"\"reader\":{\"serial\":9000041,\"tags\":[1,2,3]}}]";

	REQUIRE(parseSIPunches(json, out, 10) == 1);
	CHECK(out[0].id == 1);
	CHECK(!strcmp(out[0].card, "100"));
}

TEST_CASE("parseSIPunches: whitespace and newlines in various places don't matter")
{
	SIPunchTp out[10];
	const char *json =
		"[\n  {\n    \"id\" : 1 ,\n    \"card\":\"100\",\n"
		"    \"time\":1,\"code\":31,\"type\":\"Control\",\n"
		"    \"modem\":\"9000041\",\"receptionTimeUtc\":1\n  }\n]";

	REQUIRE(parseSIPunches(json, out, 10) == 1);
	CHECK(out[0].id == 1);
	CHECK(out[0].code == 31);
}

TEST_CASE("parseSIPunches: a leading UTF-8 BOM is skipped")
{
	SIPunchTp out[10];
	const char *json =
		"\xEF\xBB\xBF[{\"id\":1,\"card\":\"100\",\"time\":1,\"code\":31,\"type\":\"Control\","
		"\"modem\":\"9000041\",\"receptionTimeUtc\":1}]";

	REQUIRE(parseSIPunches(json, out, 10) == 1);
	CHECK(out[0].id == 1);
	CHECK(!strcmp(out[0].type, "Control"));
}

TEST_CASE("parseSIPunches: maxout caps the returned count, no overflow crash")
{
	SIPunchTp out[2];
	const char *json =
		"[{\"id\":1,\"card\":\"1\",\"time\":1,\"code\":1,\"type\":\"Start\","
		"\"modem\":\"m\",\"receptionTimeUtc\":1},"
		"{\"id\":2,\"card\":\"2\",\"time\":2,\"code\":2,\"type\":\"Control\","
		"\"modem\":\"m\",\"receptionTimeUtc\":2},"
		"{\"id\":3,\"card\":\"3\",\"time\":3,\"code\":3,\"type\":\"Finish\","
		"\"modem\":\"m\",\"receptionTimeUtc\":3}]";

	CHECK(parseSIPunches(json, out, 2) == 2);
	CHECK(out[0].id == 1);
	CHECK(out[1].id == 2);
}

TEST_CASE("parseSIPunches: missing opening '[' returns -1")
{
	SIPunchTp out[10];
	CHECK(parseSIPunches("{\"id\":1}", out, 10) == -1);
	CHECK(parseSIPunches("", out, 10) == -1);
	CHECK(parseSIPunches("not json at all", out, 10) == -1);
}

TEST_CASE("parseSIPunches: unterminated string returns -1")
{
	SIPunchTp out[10];
	CHECK(parseSIPunches("[{\"id\":1,\"card\":\"100]", out, 10) == -1);
}

TEST_CASE("parseSIPunches: missing comma between elements returns -1")
{
	SIPunchTp out[10];
	CHECK(parseSIPunches("[{\"id\":1}{\"id\":2}]", out, 10) == -1);
}

TEST_CASE("parseSIPunches: a null value in a field doesn't crash")
{
	SIPunchTp out[10];
	const char *json =
		"[{\"id\":1,\"card\":\"100\",\"time\":1,\"code\":null,\"type\":\"Unknown\","
		"\"modem\":\"9000041\",\"receptionTimeUtc\":1}]";

	REQUIRE(parseSIPunches(json, out, 10) == 1);
	CHECK(out[0].code == 0);
}

TEST_CASE("koodi2piste: found at the correct index")
{
	int rastikoodi[] = {31, 32, 33, 34};
	CHECK(koodi2piste(rastikoodi, 4, 33) == 2);
	CHECK(koodi2piste(rastikoodi, 4, 31) == 0);
	CHECK(koodi2piste(rastikoodi, 4, 34) == 3);
}

TEST_CASE("koodi2piste: -1 if the code is not found")
{
	int rastikoodi[] = {31, 32, 33};
	CHECK(koodi2piste(rastikoodi, 3, 99) == -1);
}

TEST_CASE("koodi2piste: rastiluku bounds the search beyond the array's actual content")
{
	// the array has room for 5, but only 2 are in use - code 33 must not
	// be found even though it's in the array's 3rd element (stale/garbage data).
	int rastikoodi[] = {31, 32, 33, 34, 35};
	CHECK(koodi2piste(rastikoodi, 2, 33) == -1);
}

// siResolvePunch tests use a course modeled on a real SportIdent Center
// feed observed in this repo's testing (course: Start=code 3, Controls=
// codes 33 and 99, Finish=code 4 - reported by the API as "Unknown" for
// the Start punch and "Control"/"Finish" for the rest, never "Start").

TEST_CASE("siResolvePunch: literal type \"Start\" wins regardless of code")
{
	int rastikoodi[] = {33, 99};
	CHECK(siResolvePunch("Start", 12345, 3, 0, rastikoodi, 2) == SI_PUNCH_START);
}

TEST_CASE("siResolvePunch: literal type \"Finish\" wins regardless of code")
{
	int rastikoodi[] = {33, 99};
	CHECK(siResolvePunch("Finish", 12345, 3, 0, rastikoodi, 2) == SI_PUNCH_FINISH);
}

TEST_CASE("siResolvePunch: \"Unknown\" + code matching SISTARTKOODI resolves to Start")
{
	int rastikoodi[] = {33, 99};
	CHECK(siResolvePunch("Unknown", 3, 3, 0, rastikoodi, 2) == SI_PUNCH_START);
}

TEST_CASE("siResolvePunch: \"Control\" + code matching SISTARTKOODI also resolves to Start")
{
	// defensive: the real API has been observed sending the Start punch as
	// type "Unknown", but nothing guarantees it won't be "Control" either.
	int rastikoodi[] = {33, 99};
	CHECK(siResolvePunch("Control", 3, 3, 0, rastikoodi, 2) == SI_PUNCH_START);
}

TEST_CASE("siResolvePunch: \"Unknown\" + isFinishCode resolves to Finish")
{
	int rastikoodi[] = {33, 99};
	CHECK(siResolvePunch("Unknown", 4, 3, 1, rastikoodi, 2) == SI_PUNCH_FINISH);
}

TEST_CASE("siResolvePunch: \"Unknown\" + a normal control code resolves to piste+1")
{
	int rastikoodi[] = {33, 99};
	CHECK(siResolvePunch("Unknown", 33, 3, 0, rastikoodi, 2) == 1);
	CHECK(siResolvePunch("Control", 99, 3, 0, rastikoodi, 2) == 2);
}

TEST_CASE("siResolvePunch: unresolvable code returns SI_PUNCH_NOTFOUND")
{
	int rastikoodi[] = {33, 99};
	CHECK(siResolvePunch("Unknown", 77, 3, 0, rastikoodi, 2) == SI_PUNCH_NOTFOUND);
	CHECK(siResolvePunch("Control", 77, 3, 0, rastikoodi, 2) == SI_PUNCH_NOTFOUND);
}

TEST_CASE("siResolvePunch: sistartkoodi==0 (not configured) never matches code 0")
{
	int rastikoodi[] = {33, 99};
	CHECK(siResolvePunch("Unknown", 0, 0, 0, rastikoodi, 2) == SI_PUNCH_NOTFOUND);
}

TEST_CASE("siResolvePunch: SISTARTKOODI takes precedence over isFinishCode")
{
	// a misconfiguration where the same code is (wrongly) both the
	// Start code and reported as the finish control - Start must win,
	// since SISTARTKOODI is checked first.
	int rastikoodi[] = {33, 99};
	CHECK(siResolvePunch("Unknown", 3, 3, 1, rastikoodi, 2) == SI_PUNCH_START);
}

TEST_CASE("siResolvePunch: isFinishCode takes precedence over a matching rastikoodi entry")
{
	// code 33 is both a course control and (per the caller's maalirasti()
	// result) the finish control for this competitor - Finish must win.
	int rastikoodi[] = {33, 99};
	CHECK(siResolvePunch("Unknown", 33, 3, 1, rastikoodi, 2) == SI_PUNCH_FINISH);
}
