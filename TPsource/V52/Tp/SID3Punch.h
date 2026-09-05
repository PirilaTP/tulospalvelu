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

// Decodes a SportIdent card's serial number (SIID) from the 3 payload bytes
// carried in a "D3" punch message - sent when a SIAC card punches wirelessly
// (Air+/extended protocol) or a BS-11 station relays a punch over direct
// serial (classic protocol). Both framings carry the same payload shape and
// call this same decoder (see Tp/TpLaitteet.cpp's lue_LUKIJA()). This
// translation unit is deliberately independent of global variables, the
// database, and Windows/VCL, so it can be compiled for unit tests
// (see Tests/SID3PunchTest.cpp).

#ifndef SID3PUNCH_DEFINED
#define SID3PUNCH_DEFINED

#include <tptype.h>

// Decodes a D3 message's SIID (SportIdent card serial number) from its
// three serial-number payload bytes: sn2 (data[3] in the message), sn1
// (data[4]), sn0 (data[5]) - a plain 24-bit big-endian number
// ((sn2<<16) | (sn1<<8) | sn0). This is the same for every card that can
// produce a D3 message (SI6, SI9+, SIAC): D3 is an Air+/wireless-capable
// punch format, and SI5 - the only SportIdent card generation whose own
// printed serial number needs a different formula (sn2*100000 + sn1sn0,
// used by the *classic contact* B1 readout response, not this format) -
// predates Air+/SIAC wireless hardware entirely and can never send one.
// Confirmed against a real captured SI6 punch (SIID 579671, sn2=0x08)
// that an sn2<10 special case here would have silently corrupted to
// 855383 - see git history for the incident this test guards against.
UINT32 decodeD3Siid(unsigned char sn2, unsigned char sn1, unsigned char sn0);

#endif
