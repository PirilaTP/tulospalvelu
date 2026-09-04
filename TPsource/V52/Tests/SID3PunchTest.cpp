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

#include <tptype.h>
#include "SID3Punch.h"
#include "doctest.h"

TEST_CASE("decodeD3Siid: SI5-series card (sn2 < 10) uses sn2*100000 + sn1sn0")
{
	// A real-looking SI5 card number, e.g. 512345: sn2=5, sn1sn0=12345
	// (12345 = 0x3039 -> sn1=0x30, sn0=0x39).
	CHECK(decodeD3Siid(5, 0x30, 0x39) == 512345);
}

TEST_CASE("decodeD3Siid: sn2 == 9 still takes the SI5-series branch")
{
	CHECK(decodeD3Siid(9, 0x00, 0x01) == 900001);
}

TEST_CASE("decodeD3Siid: sn2 == 10 takes the SI9+ 24-bit branch, not SI5-series")
{
	// If this boundary were off by one, sn2==10 would wrongly produce
	// 10*100000 + 1 = 1000001 instead of the 24-bit value below.
	CHECK(decodeD3Siid(10, 0x00, 0x01) == 0x0A0001);
	CHECK(decodeD3Siid(10, 0x00, 0x01) != 1000001);
}

TEST_CASE("decodeD3Siid: SI9+ card (sn2 >= 10) is a plain 24-bit big-endian number")
{
	// SIID 8647177 = 0x83F209
	CHECK(decodeD3Siid(0x83, 0xF2, 0x09) == 8647177);
}

TEST_CASE("decodeD3Siid: SI5-series zero card number")
{
	CHECK(decodeD3Siid(0, 0, 0) == 0);
}

TEST_CASE("decodeD3Siid: SI9+ maximum 24-bit value doesn't overflow")
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
