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

// Unit tests for decoding a SportIdent D3 punch message's card serial
// number (Tp/SID3Punch.cpp:decodeD3Siid), shared by both the Air+/extended
// protocol (SIAC wireless punch) and the classic-protocol (BS-11 direct
// serial) message framings in Tp/TpLaitteet.cpp's lue_LUKIJA().
//
// decodeD3Siid() always uses a plain 24-bit big-endian combine - an
// earlier version special-cased sn2 < 10 to mean "SI5-series", borrowing
// the *classic contact* B1 readout's sn2*100000+sn1sn0 formula. That was
// wrong: SI5 predates Air+/SIAC wireless hardware and can never produce a
// D3 message, so the special case could only ever misfire on a real
// SI6/SI9+ card whose serial number happens to have sn2 < 10 - which is
// exactly what happened (see the regression test below, from a real
// captured punch).

#include <tptype.h>
#include "SID3Punch.h"
#include "doctest.h"

TEST_CASE("decodeD3Siid: real captured SI6 punch regression (sn2 < 10 must not special-case)")
{
	// SIID 579671 = 0x08D857, captured from a real SI6 card punching via
	// SRR/Air+ (D3 message "02-D3-0D-00-32-00-08-D8-57-...", sn2/sn1/sn0 =
	// 08/D8/57). A prior sn2<10 "SI5-series" special case wrongly turned
	// this into 855383 instead - see git history for the incident.
	CHECK(decodeD3Siid(0x08, 0xD8, 0x57) == 579671);
}

TEST_CASE("decodeD3Siid: a plain 24-bit big-endian number regardless of sn2's value")
{
	// SIID 8647177 = 0x83F209
	CHECK(decodeD3Siid(0x83, 0xF2, 0x09) == 8647177);
	// sn2 small (would have hit the old, wrong SI5-series special case)
	CHECK(decodeD3Siid(0x05, 0x30, 0x39) == 0x053039);
	CHECK(decodeD3Siid(0x09, 0x00, 0x01) == 0x090001);
}

TEST_CASE("decodeD3Siid: zero card number")
{
	CHECK(decodeD3Siid(0, 0, 0) == 0);
}

TEST_CASE("decodeD3Siid: maximum 24-bit value doesn't overflow")
{
	CHECK(decodeD3Siid(0xFF, 0xFF, 0xFF) == 0xFFFFFFUL);
}

TEST_CASE("decodeD3Siid: sn1/sn0 byte order is big-endian (sn1 is the high byte)")
{
	// Swapping sn1/sn0 must change the result - guards against a
	// transposed byte-order regression.
	CHECK(decodeD3Siid(20, 0x01, 0x00) != decodeD3Siid(20, 0x00, 0x01));
	CHECK(decodeD3Siid(20, 0x01, 0x00) == 0x140100);
}

TEST_CASE("decodeD3SiidSI5: real SI5 card 229401 relayed over SRR")
{
	// Bytes 02 72 D9: series 2, number 0x72D9 = 29401. Plain 24-bit
	// decoding gave 160473 (0x0272D9) in HkMaali.
	CHECK(decodeD3SiidSI5(0x02, 0x72, 0xD9) == 229401);
	CHECK(decodeD3Siid(0x02, 0x72, 0xD9) == 160473);
}

TEST_CASE("decodeD3SiidSI5: SI5 cards use the SI5 formula")
{
	// SI5 card 12345, series (CNS) 1: raw 0x013039 = 77881
	CHECK(decodeD3SiidSI5(0x01, 0x30, 0x39) == 12345);
	// SI5 card 12345, series 0
	CHECK(decodeD3SiidSI5(0x00, 0x30, 0x39) == 12345);
	// SI5 card 312345: series 3, number 0x3039
	CHECK(decodeD3SiidSI5(0x03, 0x30, 0x39) == 312345);
	// Largest SI5 number: series 4, 0xFFFF
	CHECK(decodeD3SiidSI5(0x04, 0xFF, 0xFF) == 465535);
}

TEST_CASE("decodeD3SiidSI5: SI6 and newer cards are unchanged")
{
	// Same real SI6 punch as the decodeD3Siid regression above.
	CHECK(decodeD3SiidSI5(0x08, 0xD8, 0x57) == 579671);
	CHECK(decodeD3SiidSI5(0x83, 0xF2, 0x09) == 8647177);
	// 500000 = 0x07A120 is the first non-SI5 value
	CHECK(decodeD3SiidSI5(0x07, 0xA1, 0x20) == 500000);
}
