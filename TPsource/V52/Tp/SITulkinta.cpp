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

#include <stdio.h>
#include <string.h>
#include <tptype.h>
#include <TpDef.h>

#include "sitypes.h"
#include "SITulkinta.h"

// t_time_l esitellaan myos tputil.h:ssa, mutta sita ei voi sisallyttaa
// tahan: se vetaa mukanaan windows.h:n ja tekisi tasta kaannosyksikosta
// testeihin kaantymattoman. Toteutus (tputilv2/T_time_l.cpp) on riippumaton.
extern long t_time_l(long tics, int t0);

// Tulkitsee SportIdent SI5/SI6/SI8-11 -korttidata (buf) result-rakenteeseen.
// SIt on lukuaika, SItype 5-11, buflen = SIbuf:n kaytetty pituus; palauttaa
// 0 onnistuessaan.
int tulkSI(char *buf, SIResultTp *result, INT32 SIt, int SItype, int buflen, int t0)
	{
	SI5tp *tp5;
	SI6tp *tp6;
	int r, i;

	memset(result, 0, sizeof(*result));
	switch (SItype) {
		case 5:
			tp5 = (SI5tp *) buf;
			result->badge = 256L * tp5->CN[0] + tp5->CN[1] +
				(tp5->CNS > 1 ? tp5->CNS * 100000L : 0);
			result->lukija = t_time_l(SIt, t0);
			result->start = 256L * tp5->ST[0] + tp5->ST[1];
			result->check = 256L * tp5->CT[0] + tp5->CT[1];
			result->finish = 256L * tp5->FT[0] + tp5->FT[1];
			for (r = 0; r < 6; r++) {
				result->cc[31+r] = tp5->row[r].ccx;
				for (i = 0; i < 5; i++) {
					result->cc[1+i+5*r] = tp5->row[r].c[i].cc;
					result->ct[1+i+5*r] =
						256L*tp5->row[r].c[i].ct[0] + tp5->row[r].c[i].ct[1];
					if (r+i == 0) {
						if (result->start != 61166L && result->ct[1] &&
							result->ct[1] < result->start)
							result->ct[1] += 43200L;
						}
					else {
						if (result->ct[1+i+5*r] &&
							result->ct[1+i+5*r] < result->ct[i+5*r])
							result->ct[1+i+5*r] += 43200L;
						}
					}
				}
			break;
		case 6: {
			int cnt;
			tp6 = (SI6tp *) buf;
			result->badge =
				tp6->CN[3] + 256L * (tp6->CN[2] + 256L * (tp6->CN[1] + 256L * tp6->CN[0]));
			result->lukija = t_time_l(SIt, t0);
			result->start =
					256L*tp6->st.PT[0] + tp6->st.PT[1] +
					(tp6->st.PTD & 1) * 43200L;
			result->check =
					256L*tp6->chk.PT[0] + tp6->chk.PT[1] +
					(tp6->chk.PTD & 1) * 43200L;
			result->finish =
					256L*tp6->fi.PT[0] + tp6->fi.PT[1] +
					(tp6->fi.PTD & 1) * 43200L;
			// pblk[0] and pblk[1] are two SEPARATE 32-punch blocks (64 total
			// capacity), not two copies of the same 32 slots - they used to
			// both write cc[1..32]/ct[1..32], so pblk[1] silently overwrote
			// pblk[0]'s punches (a card with 33-64 punches lost its first 32;
			// a card with <=32 punches had its real punches overwritten by
			// pblk[1]'s unused/0xEE slots). Fixed to concatenate: pblk[0] ->
			// cc[1..32], pblk[1] -> cc[33..64], stopping at the first
			// CN=0xEE (unused slot), same convention as the EXT-protocol
			// cards below.
			cnt = 0;
			for (r = 0; r < 2; r++) {
				for (i = 0; i < 32; i++) {
					unsigned char cn = (unsigned char) tp6->pblk[r].punch[i].CN;
					if (cn == 0xEE)
						break;
					cnt++;
					if (cnt < 66) {
						result->cc[cnt] = cn;
						result->ct[cnt] =
							256L*(unsigned char) tp6->pblk[r].punch[i].PT[0] +
							(unsigned char) tp6->pblk[r].punch[i].PT[1] +
							(tp6->pblk[r].punch[i].PTD & 1) * 43200L;
						}
					}
				}
			break;
			}
		case 7: {
			// SI9 (SIID 1M-2M) via EXT protocol: blocks 0+1, 256 bytes total.
			//
			// Block 0 layout (pcap-verified, card 1009090):
			//   [8]  PTD  [9]  CN   [10] time_H [11] time_L  - Check punch
			//   [12:16]  EE EE EE EE                          - Clear (ignored)
			//   [16] PTD  [17] CN   [18] time_H [19] time_L  - Finish punch
			//   [20] PTD  [21] CN   [22] time_H [23] time_L  - Start punch (CN=EE->no start)
			//   [24] CNS  [25:28] SIID (3 bytes, big-endian)
			//   [56+] punch records: {PTD, CN, time_H, time_L} x n, terminated by CN=EE
			//
			// Block 1 continues the punch list (buf[128:256]).
			// PTD bit 0 = half-day flag: add 43200 s when set (times wrap at 12 h).
			unsigned char *b = (unsigned char *) buf;
			result->badge  = b[25]*65536L + b[26]*256L + b[27];
			result->lukija = t_time_l(SIt, t0);
			result->check  = (b[9]  == 0xEE) ? 0L :
				256L*b[10] + b[11] + (b[8]  & 1) * 43200L;
			result->finish = (b[17] == 0xEE) ? 0L :
				256L*b[18] + b[19] + (b[16] & 1) * 43200L;
			result->start  = (b[13] == 0xEE) ? TMAALI0 :
				256L*b[14] + b[15] + (b[12] & 1) * 43200L;
			r = 0;
			for (i = 56; i + 3 < 256; i += 4) {
				unsigned char cn = b[i+1];
				long pt;
				if (cn == 0xEE) break;
				pt = 256L*b[i+2] + b[i+3] + (b[i] & 1) * 43200L;
				r++;
				if (r < 66) {
					result->cc[r] = cn;
					if (r == 1) {
						if (result->start && pt < result->start)
							pt += 43200L;
						}
					else {
						if (result->ct[r-1] && pt < result->ct[r-1])
							pt += 43200L;
						}
					result->ct[r] = pt;
					}
				}
			break;
			}
		case 8: {
			// SI10/SI11 (SIID >=7M) via EXT protocol: block 0 + 1-4 punch blocks (256-640 bytes).
			// Block 0 header layout identical to SI9 (case 7): SIID, check, start, finish.
			// Punches start at buf[128] (block 4); each additional 128-byte block holds 32 more.
			// Same {PTD, CN, time_H, time_L} record format; CN=EE terminates the list.
			unsigned char *b = (unsigned char *) buf;
			result->badge  = b[25]*65536L + b[26]*256L + b[27];
			result->lukija = t_time_l(SIt, t0);
			result->check  = (b[9]  == 0xEE) ? 0L :
				256L*b[10] + b[11] + (b[8]  & 1) * 43200L;
			result->finish = (b[17] == 0xEE) ? 0L :
				256L*b[18] + b[19] + (b[16] & 1) * 43200L;
			result->start  = (b[13] == 0xEE) ? TMAALI0 :
				256L*b[14] + b[15] + (b[12] & 1) * 43200L;
			r = 0;
			for (i = 128; i + 3 < buflen; i += 4) {
				unsigned char cn = b[i+1];
				long pt;
				if (cn == 0xEE) break;
				pt = 256L*b[i+2] + b[i+3] + (b[i] & 1) * 43200L;
				r++;
				if (r < 66) {
					result->cc[r] = cn;
					if (r == 1) {
						if (result->start && pt < result->start)
							pt += 43200L;
						}
					else {
						if (result->ct[r-1] && pt < result->ct[r-1])
							pt += 43200L;
						}
					result->ct[r] = pt;
					}
				}
			break;
			}
		case 9: {
			// SI8 (SIID 2M-4M) via EXT protocol: blocks 0+1, 256 bytes total.
			// Block 0 header identical to SI9 (case 7).
			// Punches start at buf[136] (block 1 offset 8), not buf[56] like SI9.
			unsigned char *b = (unsigned char *) buf;
			result->badge  = b[25]*65536L + b[26]*256L + b[27];
			result->lukija = t_time_l(SIt, t0);
			result->check  = (b[9]  == 0xEE) ? 0L :
				256L*b[10] + b[11] + (b[8]  & 1) * 43200L;
			result->finish = (b[17] == 0xEE) ? 0L :
				256L*b[18] + b[19] + (b[16] & 1) * 43200L;
			result->start  = (b[13] == 0xEE) ? TMAALI0 :
				256L*b[14] + b[15] + (b[12] & 1) * 43200L;
			r = 0;
			for (i = 136; i + 3 < 256; i += 4) {
				unsigned char cn = b[i+1];
				long pt;
				if (cn == 0xEE) break;
				pt = 256L*b[i+2] + b[i+3] + (b[i] & 1) * 43200L;
				r++;
				if (r < 66) {
					result->cc[r] = cn;
					if (r == 1) {
						if (result->start && pt < result->start)
							pt += 43200L;
						}
					else {
						if (result->ct[r-1] && pt < result->ct[r-1])
							pt += 43200L;
						}
					result->ct[r] = pt;
					}
				}
			break;
			}
		case 10: {
			// pCard (SIID 4M-6M) via EXT protocol: blocks 0+1, 256 bytes total.
			// Block 0 header identical to SI9 (case 7): SIID, check, start, finish.
			// Punches start at buf[176] (block 1 byte 48), max 20, 4-byte {PTD, CN, time_H, time_L}.
			unsigned char *b = (unsigned char *) buf;
			result->badge  = b[25]*65536L + b[26]*256L + b[27];
			result->lukija = t_time_l(SIt, t0);
			result->check  = (b[9]  == 0xEE) ? 0L :
				256L*b[10] + b[11] + (b[8]  & 1) * 43200L;
			result->finish = (b[17] == 0xEE) ? 0L :
				256L*b[18] + b[19] + (b[16] & 1) * 43200L;
			result->start  = (b[13] == 0xEE) ? TMAALI0 :
				256L*b[14] + b[15] + (b[12] & 1) * 43200L;
			r = 0;
			for (i = 176; i + 3 < 256; i += 4) {
				unsigned char cn = b[i+1];
				long pt;
				if (cn == 0xEE) break;
				pt = 256L*b[i+2] + b[i+3] + (b[i] & 1) * 43200L;
				r++;
				if (r < 66) {
					result->cc[r] = cn;
					if (r == 1) {
						if (result->start && pt < result->start)
							pt += 43200L;
						}
					else {
						if (result->ct[r-1] && pt < result->ct[r-1])
							pt += 43200L;
						}
					result->ct[r] = pt;
					}
				}
			break;
			}
		case 11: {
			// tCard (SIID 6M-7M) via EXT protocol: blocks 0+1, 256 bytes total.
			// Block 0 header identical to SI9 (case 7): SIID, check, start, finish.
			// Punches start at buf[56], max 25, 8-byte records.
			// Record layout: {PTD, CN, time_H, time_L, sub_H, sub_L, 0x00, 0x00}
			// Sub-second bytes are ignored; step is 8 instead of 4.
			unsigned char *b = (unsigned char *) buf;
			result->badge  = b[25]*65536L + b[26]*256L + b[27];
			result->lukija = t_time_l(SIt, t0);
			result->check  = (b[9]  == 0xEE) ? 0L :
				256L*b[10] + b[11] + (b[8]  & 1) * 43200L;
			result->finish = (b[17] == 0xEE) ? 0L :
				256L*b[18] + b[19] + (b[16] & 1) * 43200L;
			result->start  = (b[13] == 0xEE) ? TMAALI0 :
				256L*b[14] + b[15] + (b[12] & 1) * 43200L;
			r = 0;
			for (i = 56; i + 3 < 256; i += 8) {
				unsigned char cn = b[i+1];
				long pt;
				if (cn == 0xEE) break;
				pt = 256L*b[i+2] + b[i+3] + (b[i] & 1) * 43200L;
				r++;
				if (r < 66) {
					result->cc[r] = cn;
					if (r == 1) {
						if (result->start && pt < result->start)
							pt += 43200L;
						}
					else {
						if (result->ct[r-1] && pt < result->ct[r-1])
							pt += 43200L;
						}
					result->ct[r] = pt;
					}
				}
			break;
			}
		case 12: {
			// SI6 (SIID <1M) via EXT protocol (cmd 0xE1, trigger 0xE6) - a
			// different wire encoding of the same card generation as legacy
			// SI6 (case 6), not the same block layout as SI9+ (cases 7-11).
			//
			// Block 0 layout (verified against a real card, SIID 579671):
			//   [10:14] CN (badge, 4-byte big-endian - not the 3-byte SIID
			//           used by SI9+)
			//   [20] PTD [21] CN [22:24] time - Finish punch
			//   [24] PTD [25] CN [26:28] time - Start punch (CN=EE -> no start)
			//   [28] PTD [29] CN [30:32] time - Check punch
			//   [32] PTD [33] CN [34:36] time - Clear punch (not stored)
			//   [48:128] surname/firstname/country/club text (ignored)
			//
			// Block 1 (buf[128:256]) holds extended personalisation data
			// (e.g. an email address on some cards); not needed and not
			// parsed here.
			//
			// Blocks 6/7 (buf[256:384], buf[384:512]) each hold up to 32
			// {PTD, CN, time_H, time_L} punch records, same format as SI9+
			// (case 7), terminated by CN=0xEE.
			unsigned char *b = (unsigned char *) buf;
			result->badge  = b[10]*16777216L + b[11]*65536L + b[12]*256L + b[13];
			result->lukija = t_time_l(SIt, t0);
			result->finish = (b[21] == 0xEE) ? 0L :
				256L*b[22] + b[23] + (b[20] & 1) * 43200L;
			result->start  = (b[25] == 0xEE) ? TMAALI0 :
				256L*b[26] + b[27] + (b[24] & 1) * 43200L;
			result->check  = (b[29] == 0xEE) ? 0L :
				256L*b[30] + b[31] + (b[28] & 1) * 43200L;
			r = 0;
			for (i = 256; i + 3 < buflen; i += 4) {
				unsigned char cn = b[i+1];
				long pt;
				if (cn == 0xEE) break;
				pt = 256L*b[i+2] + b[i+3] + (b[i] & 1) * 43200L;
				r++;
				if (r < 66) {
					result->cc[r] = cn;
					if (r == 1) {
						if (result->start && pt < result->start)
							pt += 43200L;
						}
					else {
						if (result->ct[r-1] && pt < result->ct[r-1])
							pt += 43200L;
						}
					result->ct[r] = pt;
					}
				}
			break;
			}
		}
	return(0);
	}
