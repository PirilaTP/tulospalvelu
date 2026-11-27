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
// ((sn2<<16) | (sn1<<8) | sn0). This is right for SI6, SI9+ and SIAC
// cards, but NOT for SI5, whose printed number needs sn2*100000 + sn1sn0:
// an SI5 card can't punch wirelessly itself, but its contact punch at an
// SRR-capable control is relayed as a D3 message all the same. The
// callers therefore use decodeD3SiidSI5 below, which handles both.
// Confirmed against a real captured SI6 punch (SIID 579671, sn2=0x08)
// that an sn2<10 special case here would have silently corrupted to
// 855383 - see git history for the incident this test guards against.
UINT32 decodeD3Siid(unsigned char sn2, unsigned char sn1, unsigned char sn0);

// The card number to use for a D3 punch - both SRR/Air+ punches and a
// station wired directly to the PC. An SI5 card's number is carried in
// the SI5 card's own form: sn2 = CNS (series), sn1:sn0 = number. This
// does reach the SRR dongle: an SRR-capable control relays contact punches
// of any card, SI5 included - a real SI5 card 229401 (bytes 02 72 D9)
// showed up as 160473 with plain decodeD3Siid. A 24-bit value below
// 500000 is therefore decoded as an SI5 card: sn2 < 2 -> sn1sn0,
// otherwise sn2*100000 + sn1sn0 (same as tulkSI's SI5 case). Every other
// card generation's SIID is >= 500000 (SI6 579671 above stays 579671), so
// it is returned unchanged.
UINT32 decodeD3SiidSI5(unsigned char sn2, unsigned char sn1, unsigned char sn0);

// Framing of SportIdent extended-protocol messages from the SRR dongle:
//   [FF] 02 <cmd> <dlen> <data[dlen]> <crc_hi> <crc_lo> 03
// (FF is an optional wake-up byte). Looks at the n >= 1 buffered bytes
// b[0..n) and returns:
//   SISAN_KESKEN  only the start of a message so far - wait for more bytes
//   SISAN_OHITA   b[0] can't start a valid message (no STX, or no ETX where
//                 the length says the message ends) - drop one byte, retry
//   SISAN_OK      a complete message: *alku = 0 or 1 (FF present), *dlen =
//                 data length, *total = message length; the command byte
//                 is b[*alku + 1] and the data starts at b[*alku + 3]
// Dropping one byte on SISAN_OHITA resynchronises the stream, so one bad
// message can't stall the reader (see lue_SRRsanomat in TpLaitteet.cpp).
#define SISAN_KESKEN 0
#define SISAN_OHITA  1
#define SISAN_OK     2
int siEtsiSanoma(const unsigned char *b, int n, int *alku, int *dlen, int *total);

// Fields of one D3 punch message's data part (data[0..dlen)):
//   CN1 CN0 SI3 SI2 SI1 SI0 TD TH TL TSS [MEM2 MEM1 MEM0]
// koodi = control code (CN1 high byte), siid = card number
// (decodeD3SiidSI5), korttitms = the station's punch time in ms of day
// (TD bit 0 = PM, TH:TL = seconds within 12 h, TSS = 1/256 s), or -1 when
// the message is too short to carry it (dlen < 10). Requires dlen >= 6.
typedef struct {
	UINT32 siid;
	int koodi;
	INT32 korttitms;
} SID3Leima;
void siPuraD3(const unsigned char *data, int dlen, SID3Leima *leima);

// true if a punch (siid, koodi, tms) repeats the previous one stored from
// the same reader (edsiid, edkoodi, edtms): same card at the same control
// within 3 s (tms in ms of day, the difference taken across midnight).
// The same message can arrive more than once (e.g. via several SRR
// receivers); a different control or a later punch is never a repeat.
bool siToistoLeima(UINT32 edsiid, int edkoodi, INT32 edtms,
	UINT32 siid, int koodi, INT32 tms);

#endif
