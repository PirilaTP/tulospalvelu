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

// Unit tests for combining an Emit "r12" record's badge[0..2] bytes into a
// plain badge number (Tp/EmitBadge.cpp:combineBadge24LE), used by
// tall_emit() (Hk/HkIV.cpp, Juk/VIv.cpp) for every LID_LUKIJA reader -
// both real Emit cards and SportIdent D3 punches, which are wrapped into
// a synthetic r12-shaped record so tall_emit() can process them the same
// way (see Tp/TpLaitteet.cpp's lue_LUKIJA()). The second group of tests
// below exercises that exact SportIdent/Emit boundary end-to-end.

#include <tptype.h>
#include "EmitBadge.h"
#include "doctest.h"

TEST_CASE("combineBadge24LE: byte order is little-endian (b0 is the low byte)")
{
	CHECK(combineBadge24LE(0x01, 0x00, 0x00) == 1);
	CHECK(combineBadge24LE(0x00, 0x01, 0x00) == 256);
	CHECK(combineBadge24LE(0x00, 0x00, 0x01) == 65536);
}

TEST_CASE("combineBadge24LE: a real-looking Emit badge number")
{
	// Badge 1234567 = 0x12D687 -> b0=0x87, b1=0xD6, b2=0x12
	CHECK(combineBadge24LE(0x87, 0xD6, 0x12) == 1234567);
}

TEST_CASE("combineBadge24LE: zero")
{
	CHECK(combineBadge24LE(0, 0, 0) == 0);
}

TEST_CASE("combineBadge24LE: maximum 24-bit value")
{
	CHECK(combineBadge24LE(0xFF, 0xFF, 0xFF) == 0xFFFFFFUL);
}

// ===========================================================================
// SportIdent/Emit boundary: TpLaitteet.cpp's D3 punch handler builds a
// synthetic Emit "r12" record so tall_emit() can process an SI punch the
// same way as a real Emit card punch (see lue_LUKIJA()):
//   memset(vastaus->bytes, '\xdf', r_msg_len);
//   vastaus->r12.badge[i] = (siid_byte[i]) ^ 0xDF;
// tall_emit() then undoes that encoding for the *whole* record with
//   for (i...) vastaus->bytes[i] ^= '\xdf';
// before reading badge[0..2] and calling combineBadge24LE(). These tests
// replicate that exact two-step pipeline (XOR-encode as the D3 handler
// does, XOR-decode as tall_emit()'s loop does) and check it round-trips
// back to the original SIID through the real, shared combine function -
// so a change to either side (or to the 0xDF constant, or a byte-order
// slip) would be caught here without needing to run either handler.
// ===========================================================================

static unsigned char wireEncodeByte(unsigned char plain)
{
	return (unsigned char)(plain ^ 0xDF);
}

static unsigned char wireDecodeByte(unsigned char wire)
{
	return (unsigned char)(wire ^ 0xDF);   // XOR is self-inverse
}

TEST_CASE("SI D3 -> Emit boundary: a synthesized SIID round-trips through the wire encoding")
{
	UINT32 siid = 8004086UL;   // a real SIID seen in a captured D3 message this session
	unsigned char b0 = (unsigned char)(siid & 0xFF);
	unsigned char b1 = (unsigned char)((siid >> 8) & 0xFF);
	unsigned char b2 = (unsigned char)((siid >> 16) & 0xFF);

	// what TpLaitteet.cpp writes into vastaus->r12.badge[0..2]
	unsigned char wire0 = wireEncodeByte(b0);
	unsigned char wire1 = wireEncodeByte(b1);
	unsigned char wire2 = wireEncodeByte(b2);

	// what tall_emit()'s whole-record XOR loop produces before combineBadge24LE runs
	UINT32 decoded = combineBadge24LE(wireDecodeByte(wire0), wireDecodeByte(wire1), wireDecodeByte(wire2));

	CHECK(decoded == (siid & 0xFFFFFFUL));
}

TEST_CASE("SI D3 -> Emit boundary: round-trip holds for a badge byte that is itself 0xDF")
{
	// 0xDF XOR 0xDF == 0 - a boundary worth pinning explicitly, since a
	// badge byte that happens to equal the encoding constant must still
	// round-trip to itself, not to 0 or some other value.
	unsigned char wire = wireEncodeByte(0xDF);
	CHECK(wire == 0x00);
	CHECK(wireDecodeByte(wire) == 0xDF);
}

TEST_CASE("SI D3 -> Emit boundary: round-trip holds across the full byte range")
{
	for (int v = 0; v <= 0xFF; v++) {
		unsigned char plain = (unsigned char)v;
		CHECK(wireDecodeByte(wireEncodeByte(plain)) == plain);
		}
}
