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

// Combines an Emit card's 3-byte badge number field (the "r12" record's
// badge[0..2], already un-XORed from the wire's 0xDF encoding by
// tall_emit()'s whole-record decode loop) into a plain 24-bit badge
// number. This is also exactly what a SportIdent D3 punch's synthetic
// Emit-format record (Tp/TpLaitteet.cpp's lue_LUKIJA(), which builds an
// r12-shaped buffer so tall_emit() can process an SI punch the same way
// as a real Emit card) needs to decode back to - see
// Tests/EmitBadgeTest.cpp for a test that exercises that exact boundary.
// This translation unit is deliberately independent of global variables,
// the database, and Windows/VCL, so it can be compiled for unit tests.

#ifndef EMITBADGE_DEFINED
#define EMITBADGE_DEFINED

#include <tptype.h>

// Combines 3 already-decoded badge bytes (b0 = least significant) into a
// 24-bit badge number, matching tall_emit()'s
// "*(UINT32*)vastaus->r12.badge & 0xffffffL" cast-and-mask (badge[0] is
// the low byte in memory on this little-endian target).
UINT32 combineBadge24LE(unsigned char b0, unsigned char b1, unsigned char b2);

#endif
