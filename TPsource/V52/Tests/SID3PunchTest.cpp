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

#include <string.h>
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

// ---------------------------------------------------------------------------
// SRR message framing (siEtsiSanoma) - the loop lue_SRRsanomat runs.

// One D3 punch message: [FF] 02 D3 0D <13 data bytes> CRC CRC 03.
static int teeD3(unsigned char *b, bool ff, unsigned char cn0,
	unsigned char sn2, unsigned char sn1, unsigned char sn0)
{
	int n = 0, i;
	if (ff)
		b[n++] = 0xFF;
	b[n++] = 0x02; b[n++] = 0xD3; b[n++] = 0x0D;
	b[n++] = 0x00; b[n++] = cn0;                     // CN1 CN0
	b[n++] = 0x00; b[n++] = sn2; b[n++] = sn1; b[n++] = sn0;
	b[n++] = 0x01; b[n++] = 0x12; b[n++] = 0x34; b[n++] = 0x80;  // TD TH TL TSS
	for (i = 0; i < 3; i++)
		b[n++] = 0x00;                               // MEM2..0
	b[n++] = 0xAB; b[n++] = 0xCD;                    // CRC (not checked)
	b[n++] = 0x03;
	return n;
}

// Runs the same loop as lue_SRRsanomat over a whole buffer: collects the
// control codes (CN0) of the complete D3 messages found, returns how many,
// and leaves in *jaljella the bytes still waiting for more data.
static int kayLapi(unsigned char *b, int n, int *koodit, int *jaljella)
{
	int nk = 0, alku, dlen, total, poista;
	while (n > 0) {
		int t = siEtsiSanoma(b, n, &alku, &dlen, &total);
		if (t == SISAN_KESKEN)
			break;
		if (t == SISAN_OHITA)
			poista = 1;
		else {
			if (b[alku + 1] == 0xD3 && dlen >= 6)
				koodit[nk++] = b[alku + 4];
			poista = total;
			}
		n -= poista;
		memmove(b, b + poista, n);
		}
	*jaljella = n;
	return nk;
}

TEST_CASE("siEtsiSanoma: complete message with and without the FF wake-up byte")
{
	unsigned char b[64];
	int alku, dlen, total, n;

	n = teeD3(b, true, 50, 0x02, 0x72, 0xD9);
	CHECK(siEtsiSanoma(b, n, &alku, &dlen, &total) == SISAN_OK);
	CHECK(alku == 1);
	CHECK(dlen == 13);
	CHECK(total == 20);
	CHECK(b[alku + 1] == 0xD3);

	n = teeD3(b, false, 50, 0x02, 0x72, 0xD9);
	CHECK(siEtsiSanoma(b, n, &alku, &dlen, &total) == SISAN_OK);
	CHECK(alku == 0);
	CHECK(total == 19);
}

TEST_CASE("siEtsiSanoma: a partial message waits for more bytes")
{
	unsigned char b[64];
	int alku, dlen, total, n = teeD3(b, false, 50, 0x02, 0x72, 0xD9);

	CHECK(siEtsiSanoma(b, 1, &alku, &dlen, &total) == SISAN_KESKEN);     // 02
	CHECK(siEtsiSanoma(b, 2, &alku, &dlen, &total) == SISAN_KESKEN);     // 02 D3
	CHECK(siEtsiSanoma(b, 10, &alku, &dlen, &total) == SISAN_KESKEN);    // header + some data
	CHECK(siEtsiSanoma(b, n - 1, &alku, &dlen, &total) == SISAN_KESKEN); // all but ETX
	CHECK(siEtsiSanoma(b, n, &alku, &dlen, &total) == SISAN_OK);
}

TEST_CASE("siEtsiSanoma: a buffer not starting with STX is skipped byte by byte")
{
	unsigned char b[4] = {0x55, 0x02, 0xD3, 0x0D};
	int alku, dlen, total;
	CHECK(siEtsiSanoma(b, 4, &alku, &dlen, &total) == SISAN_OHITA);
}

TEST_CASE("siEtsiSanoma: no ETX where the length says the message ends -> skip, not stall")
{
	unsigned char b[64];
	int alku, dlen, total, n = teeD3(b, false, 50, 0x02, 0x72, 0xD9);
	b[n - 1] = 0x00;            // corrupt ETX
	CHECK(siEtsiSanoma(b, n, &alku, &dlen, &total) == SISAN_OHITA);
}

TEST_CASE("SRR stream: valid messages are found around garbage and a corrupt message")
{
	unsigned char b[256];
	int koodit[8], jaljella, n = 0;

	b[n++] = 0x55;                                   // garbage
	n += teeD3(b + n, true, 31, 0x02, 0x72, 0xD9);   // control 31
	n += teeD3(b + n, false, 32, 0x08, 0xD8, 0x57);
	b[n - 1] = 0x00;                                 // control 32: corrupt ETX
	b[n++] = 0xFF;                                   // stray wake-up byte
	n += teeD3(b + n, false, 33, 0x83, 0xF2, 0x09);  // control 33
	n += teeD3(b + n, true, 100, 0x02, 0x72, 0xD9);  // finish, back to back

	CHECK(kayLapi(b, n, koodit, &jaljella) == 3);
	CHECK(koodit[0] == 31);
	CHECK(koodit[1] == 33);
	CHECK(koodit[2] == 100);
	CHECK(jaljella == 0);
}

TEST_CASE("SRR stream: a message split across two reads is found once the rest arrives")
{
	unsigned char b[64], tmp[64];
	int koodit[4], jaljella, n = teeD3(tmp, true, 45, 0x02, 0x72, 0xD9);

	memcpy(b, tmp, 7);                               // first read: 7 bytes
	CHECK(kayLapi(b, 7, koodit, &jaljella) == 0);
	CHECK(jaljella == 7);                            // kept, waiting
	memcpy(b + jaljella, tmp + 7, n - 7);            // second read: the rest
	CHECK(kayLapi(b, n, koodit, &jaljella) == 1);
	CHECK(koodit[0] == 45);
	CHECK(jaljella == 0);
}

TEST_CASE("SRR stream: a bogus length byte doesn't stall once enough bytes arrive")
{
	// 02 D3 FF looks like the header of a 261-byte message. Once that many
	// bytes are buffered and there's no ETX at the end, it's skipped and the
	// real messages behind it are found; the last one always survives.
	unsigned char b[400];
	int koodit[40], jaljella, n = 0, i, nk;

	b[n++] = 0x02; b[n++] = 0xD3; b[n++] = 0xFF;
	for (i = 0; i < 14; i++)
		n += teeD3(b + n, false, (unsigned char) (10 + i), 0x02, 0x72, 0xD9);
	nk = kayLapi(b, n, koodit, &jaljella);
	CHECK(nk >= 1);
	CHECK(koodit[nk - 1] == 23);
	CHECK(jaljella == 0);
}

// ---------------------------------------------------------------------------
// D3 punch fields (siPuraD3)

TEST_CASE("siPuraD3: control code, card number and station time")
{
	// data part of a D3 message: CN1 CN0 SI3 SI2 SI1 SI0 TD TH TL TSS MEM2..0
	unsigned char d[13] = {0x00, 0x32, 0x00, 0x02, 0x72, 0xD9,
		0x01, 0x12, 0x34, 0x80, 0x00, 0x00, 0x00};
	SID3Leima l;

	siPuraD3(d, 13, &l);
	CHECK(l.koodi == 50);
	CHECK(l.siid == 229401);        // SI5 card relayed over SRR
	// PM: 43200 s + 0x1234 = 4660 s -> 47860 s; TSS 0x80 = 500 ms
	CHECK(l.korttitms == 47860500L);
}

TEST_CASE("siPuraD3: control code uses CN1 as the high byte")
{
	unsigned char d[13] = {0x01, 0x2C, 0x00, 0x08, 0xD8, 0x57,
		0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x00};
	SID3Leima l;

	siPuraD3(d, 13, &l);
	CHECK(l.koodi == 300);
	CHECK(l.siid == 579671);        // SI6, unchanged
	CHECK(l.korttitms == 60000L);   // AM, 60 s
}

TEST_CASE("siPuraD3: a message too short for the time leaves korttitms at -1")
{
	unsigned char d[9] = {0x00, 0x32, 0x00, 0x83, 0xF2, 0x09, 0x01, 0x12, 0x34};
	SID3Leima l;

	siPuraD3(d, 9, &l);
	CHECK(l.siid == 8647177);
	CHECK(l.korttitms == -1);
}

// ---------------------------------------------------------------------------
// Repeat filter (siToistoLeima)

TEST_CASE("siToistoLeima: same card at the same control within 3 s is a repeat")
{
	CHECK(siToistoLeima(229401, 31, 36000000L, 229401, 31, 36000000L));
	CHECK(siToistoLeima(229401, 31, 36000000L, 229401, 31, 36002999L));
	CHECK_FALSE(siToistoLeima(229401, 31, 36000000L, 229401, 31, 36003000L));
}

TEST_CASE("siToistoLeima: last control and finish in quick succession are both kept")
{
	// The bug this replaced: a card-only filter dropped the finish punch
	// right after the last control.
	CHECK_FALSE(siToistoLeima(229401, 31, 36000000L, 229401, 100, 36000500L));
	CHECK_FALSE(siToistoLeima(229401, 31, 36000000L, 8647177, 31, 36000500L));
}

TEST_CASE("siToistoLeima: the 3 s window works across midnight")
{
	CHECK(siToistoLeima(229401, 31, 86399000L, 229401, 31, 500L));
	CHECK_FALSE(siToistoLeima(229401, 31, 86399000L, 229401, 31, 5000L));
}
