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

// SI5tp/SI6tp:n kentat ovat tavallisia char-kenttia. HkKisaWin kaannetaan
// etumerkillisella charilla (bcc32c ei noudata tputil.h:n "#pragma option
// -K":ta, eika tama tiedosto sita sisallyta), joten >= 0x80 tavut
// (0xEE-sentinelli, badgen/ajan tavut) luetaan aina taman kautta.
static inline long ub(char c) { return (unsigned char) c; }

// ===========================================================================
// SportIdent-korttien sukupolvet ja tulkintaprotokollat.
//
// Kortin tyyppi (ja siten SItype-arvo) maaraytyy badge-numeron (SIID)
// alueesta:
//   <1 000 000            SI6 (legacy-protokolla SItype 6, tai EXT-
//                          protokollan kautta SItype 12 - sama korttisukupolvi,
//                          kaksi eri langansiirtokoodausta)
//   1 000 000 - 1 999 999 SI9   (SItype 7)
//   2 000 000 - 3 999 999 SI8   (SItype 9)
//   4 000 000 - 5 999 999 pCard (SItype 10)
//   6 000 000 - 6 999 999 tCard (SItype 11)
//   >= 7 000 000          SI10/SI11 (SItype 8)
// SI5-kortit (SItype 5) eivat kuulu SIID-numerointiin - SItype paatetaan jo
// ennen tata funktiota, lukukomennon/protokollan perusteella (ks. kutsuja
// Tp/TpLaitteet.cpp:n lue_SI()).
//
// Kaksi eri laitteistoprotokollaa:
//  - Legacy-protokolla (SItype 5, 6): SI-asema lahettaa kortin sisallon
//    suoraan tunnetun struktin (SI5tp/SI6tp, ks. sitypes.h) mukaisena
//    tavupuskurina - tulkinta on pelkkaa struktin jasenien lukemista.
//  - EXT-protokolla (SItype 7-12): uudempi, laajennettava komentosarja,
//    jossa otsikko- ja leimatiedot ovat kiintein tavuoffsetein puskurissa
//    (ei struktin kautta) - ks. tulkExtOtsikko/tulkExtLeimat alla.
//
// Yhteista kaikille tyypeille:
//  - "Leima" (punch) = yksi rastikaynti. CN (control number, rastikoodi)
//    yksiloi rastin; leiman kellonaika tallennetaan sekunteina keskiyosta.
//  - SI tallentaa ajan vain 12h-jakson tarkkuudella (ei erottele AM/PM:aa
//    suoraan), joten jokainen leima-aika verrataan edelliseen: jos se on
//    pienempi, aika on "kiertanyt" seuraavaan 12h-jaksoon ja sille lisataan
//    43200 s (=12h). EXT-protokollassa tama on valmiiksi PTD-tavun bitissa 0
//    ("puolipaiva"-lippu); legacy-protokollassa (SI5/SI6) se paatellaan
//    aina vain edellisen leiman/lahdon ajasta (ks. myos case 5:n kommentti
//    61166:sta, SI5:n vastineesta samalle "ei arvoa" -ideaalle).
//  - EXT-protokollan tavupuskureissa CN=0xEE tarkoittaa "ei kaytossa"
//    (tyhja tai listan paattava tietue).
// ===========================================================================

// EXT-protokollan otsikkolohkon tulkinta: SIID (badge), check-, maali- ja
// lahtoleimat. Sama tavuasettelu kaikilla SItype 7-11 (SI9, SI10/SI11, SI8,
// pCard, tCard) - vain valiaikaleimojen sijainti/koko eroaa (ks. kutsujat,
// tulkExtLeimat). SItype 12 (SI6-EXT) ei kayta tata: sen otsikko on eri
// tavuasettelu, ks. case 12.
//
// Block 0 layout (pcap-verified, card 1009090):
//   [8]  PTD  [9]  CN   [10] time_H [11] time_L  - Check punch; myos nollaus
//        (clear, CN=255) kirjoittaa tahan - SI Config nayttaa sen "Clear"-
//        rivina. HkIV/VIv kayttaa tata nollahetkena, jos lahtoleimaa ei ole.
//   [12] PTD  [13] CN   [14] time_H [15] time_L  - Start punch (CN=EE->no start)
//   [16] PTD  [17] CN   [18] time_H [19] time_L  - Finish punch
//   [24] CNS  [25:28] SIID (3 bytes, big-endian)
// PTD bit 0 = half-day flag: add 43200 s when set (times wrap at 12 h).
// CNS-tavua (24) ei kayteta tassa: badge tulee suoraan 3-tavuisesta
// SIID:sta, ei CN+CNS-yhdistelmasta kuten SI5:lla (case 5).
static void tulkExtOtsikko(const unsigned char *b, SIResultTp *result)
	{
	result->badge  = b[25]*65536L + b[26]*256L + b[27];
	result->check  = (b[9]  == 0xEE) ? TMAALI0 :
		256L*b[10] + b[11] + (b[8]  & 1) * 43200L;
	result->finish = (b[17] == 0xEE) ? TMAALI0 :
		256L*b[18] + b[19] + (b[16] & 1) * 43200L;
	result->start  = (b[13] == 0xEE) ? TMAALI0 :
		256L*b[14] + b[15] + (b[12] & 1) * 43200L;
	}

// EXT-protokollan valiaikaleimalistan tulkinta: tietueet {PTD, CN, time_H,
// time_L, ...} alkaen tavusta start, tietueen koko step tavua (SI9/SI10-11/
// SI8/pCard/SI6-EXT: 4; tCard: 8 - lisatavut ohitetaan), enintaan bound
// tavuun asti. CN=0xEE paattaa listan. Sama silmukka kaikilla EXT-protokollan
// korttityypeilla (SItype 7-12) - vain start/step/bound eroaa kutsuittain.
static void tulkExtLeimat(const unsigned char *b, SIResultTp *result, int start, int step, int bound)
	{
	int i, r = 0;

	for (i = start; i + 3 < bound; i += step) {
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
	}

// SI5 tallentaa ajat vain 12 h jaksossa (0..43199 s) ilman aamu-/ilta-
// paiva-tietoa (vrt. EXT-korttien PTD-bitti 0). Ilman korjausta iltapaivalla
// luetun kortin leimat olisivat 12 h pielessa PC:n kellosta laskettuun
// lukuhetkeen (HkIV.cpp/VIv.cpp:n 250-rivi) nahden, jolloin myos kortilta
// laskettu maaliaika ja ennakko menisivat 12 h vaarin. Valitaan puolipaiva
// (+0 tai +12 h) niin, etta kortin viimeisin tapahtuma osuu lahimmaksi ennen
// lukuhetkea. Leimasinten kello saa olla enintaan SI5_KELLOERO (1 h) PC:n
// kelloa edella (tahdistamaton leimasin), ja kortti pitaa lukea alle
// 11 h viimeisen leiman jalkeen.
// Tyhjat lahto/tarkastus/maali (0xEEEE = 61166) muutetaan TMAALI0:ksi kuten
// EXT-korteilla; muuten +12 h siirretty todellinen aika 17966 s (04:59:26)
// sekoittuisi 61166-sentinelliin.
#define SI5_KELLOERO 3600L

static void tulkSI5Puolipaiva(SIResultTp *result, int t0)
	{
	long luku, ref = -1, d0, d1;
	int i;

	// Lukuhetki vuorokaudenaikana sekunteina, sama muunnos kuin HkIV.cpp:n
	// lukija_abs: lukija on kymmenyksina suhteessa t0:aan (+/-12 h).
	luku = ((result->lukija + (t0 + 48) * 36000L + 24L*36000L) % (24L*36000L)) / 10;
	for (i = 30; i >= 1 && ref < 0; i--)
		if (result->cc[i])
			ref = result->ct[i];
	if (ref < 0 && result->finish != 61166L)
		ref = result->finish;
	if (ref < 0 && result->check != 61166L)
		ref = result->check;
	if (ref < 0 && result->start != 61166L)
		ref = result->start;
	if (ref >= 0) {
		// Aika viimeisesta tapahtumasta lukuhetkeen kummallakin tulkinnalla,
		// valilla [-SI5_KELLOERO, 24 h - SI5_KELLOERO).
		d0 = ((luku - ref + SI5_KELLOERO) % 86400L + 86400L) % 86400L - SI5_KELLOERO;
		d1 = ((luku - ref - 43200L + SI5_KELLOERO) % 86400L + 86400L) % 86400L - SI5_KELLOERO;
		if (d1 < d0) {
			for (i = 1; i <= 30; i++)
				if (result->cc[i])
					result->ct[i] = (result->ct[i] + 43200L) % 86400L;
			if (result->start != 61166L)
				result->start = (result->start + 43200L) % 86400L;
			if (result->check != 61166L)
				result->check = (result->check + 43200L) % 86400L;
			if (result->finish != 61166L)
				result->finish = (result->finish + 43200L) % 86400L;
			}
		}
	if (result->start == 61166L)
		result->start = TMAALI0;
	if (result->check == 61166L)
		result->check = TMAALI0;
	if (result->finish == 61166L)
		result->finish = TMAALI0;
	}

// Tulkitsee SportIdent-korttidatan (buf) result-rakenteeseen. SItype:
// 5=SI5, 6=SI6, 7=SI9, 8=SI10/SI11, 9=SI8, 10=pCard, 11=tCard,
// 12=SI6 EXT-protokollan kautta (ks. yllaoleva yleiskatsaus SIID-alueista
// ja protokollista). SIt on lukuaika, buflen = SIbuf:n kaytetty pituus;
// palauttaa 0 onnistuessaan.
int tulkSI(char *buf, SIResultTp *result, INT32 SIt, int SItype, int buflen, int t0)
	{
	SI5tp *tp5;
	SI6tp *tp6;
	int r, i;

	memset(result, 0, sizeof(*result));
	result->lukija = t_time_l(SIt, t0);
	switch (SItype) {
		case 5:
			// SI5 (legacy-protokolla): kortin sisalto SI5tp-struktina (ks.
			// sitypes.h) - ei kiintein tavuoffsetein kuten EXT-protokollassa.
			// Badge (CN) on 2 tavua (0-65535); CNS > 1 jatkaa sarjaa
			// (badge += CNS*100000) uudemmilla SI5-korteilla, joita on
			// enemman kuin 65536 kpl.
			// Leimat ovat 6x5-matriisissa (row[6].c[5], 30 leimaa max):
			// row[r].ccx on rivin oma "ohituskoodi" (tallentuu cc[31..36]:
			// een, ei kaytannon leimoihin), row[r].c[i] varsinaiset leimat
			// (cc[1..30]/ct[1..30]).
			// Kaytamattomat leimapaikat ovat kortilla 00-EE-EE (CN=0, aika
			// 0xEEEE); ne jatetaan nollaksi, muuten 61166-aika nakyisi
			// HkIV.cpp:n suhteutuksen jalkeen luentanakymassa haamuriveina.
			tp5 = (SI5tp *) buf;
			result->badge = 256L * ub(tp5->CN[0]) + ub(tp5->CN[1]) +
				(ub(tp5->CNS) > 1 ? ub(tp5->CNS) * 100000L : 0);
			result->start = 256L * ub(tp5->ST[0]) + ub(tp5->ST[1]);
			result->check = 256L * ub(tp5->CT[0]) + ub(tp5->CT[1]);
			result->finish = 256L * ub(tp5->FT[0]) + ub(tp5->FT[1]);
			for (r = 0; r < 6; r++) {
				result->cc[31+r] = tp5->row[r].ccx;
				for (i = 0; i < 5; i++) {
					if (ub(tp5->row[r].c[i].cc) == 0)
						continue;
					result->cc[1+i+5*r] = tp5->row[r].c[i].cc;
					result->ct[1+i+5*r] =
						256L*ub(tp5->row[r].c[i].ct[0]) + ub(tp5->row[r].c[i].ct[1]);
					if (r+i == 0) {
						// 61166 = 0xEEEE (ST[0]=ST[1]=0xEE): SI5:n vastine
						// EXT-protokollan 0xEE-sentinellille ("ei lahtoa").
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
			tulkSI5Puolipaiva(result, t0);
			break;
		case 6: {
			// SI6 (legacy-protokolla): kortin sisalto SI6tp-struktina.
			// Badge (CN) on 4 tavua (big-endian) - laajempi arvoalue kuin
			// SI5:n 2 tavua, ei tarvitse CNS-tyyppista sarjajatketta.
			// st/chk/fi ovat yksittaiset leimat (lahto, tarkastus, maali),
			// kukin SI6P: {PTD, CN, PT[2]} - sama kolmikko kuin EXT-
			// protokollan leimatietueissa (PTD bitti 0 = puolipaivan
			// kaannos), mutta tassa omina nimettyina kenttinaan eika
			// listana.
			int cnt;
			tp6 = (SI6tp *) buf;
			result->badge =
				ub(tp6->CN[3]) + 256L * (ub(tp6->CN[2]) + 256L * (ub(tp6->CN[1]) + 256L * ub(tp6->CN[0])));
			result->start =
					256L*ub(tp6->st.PT[0]) + ub(tp6->st.PT[1]) +
					(tp6->st.PTD & 1) * 43200L;
			result->check =
					256L*ub(tp6->chk.PT[0]) + ub(tp6->chk.PT[1]) +
					(tp6->chk.PTD & 1) * 43200L;
			// Ei tarkastusleimaa (0xEEEE) -> nollausleima (clr) tilalle,
			// kuten EXT-korteilla (ks. tulkExtOtsikko).
			if (result->check == 61166L && ub(tp6->clr.CN) != 0xEE)
				result->check =
						256L*ub(tp6->clr.PT[0]) + ub(tp6->clr.PT[1]) +
						(tp6->clr.PTD & 1) * 43200L;
			result->finish =
					256L*ub(tp6->fi.PT[0]) + ub(tp6->fi.PT[1]) +
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
			// SI9 (SIID 1M-2M): valiaikaleimat alkaen tavusta 56, askel 4,
			// enintaan tavuun 256 (block 1 loppuun, buf[128:256]).
			unsigned char *b = (unsigned char *) buf;
			tulkExtOtsikko(b, result);
			tulkExtLeimat(b, result, 56, 4, 256);
			break;
			}
		case 8: {
			// SI10/SI11 (SIID >=7M): valiaikaleimat alkaen tavusta 128
			// (block 4), jatkuen lisalohkoissa buflen:iin asti
			// (256-640 tavua, 32 leimaa/lohko).
			unsigned char *b = (unsigned char *) buf;
			tulkExtOtsikko(b, result);
			tulkExtLeimat(b, result, 128, 4, buflen);
			break;
			}
		case 9: {
			// SI8 (SIID 2M-4M): valiaikaleimat alkaen tavusta 136
			// (block 1 offset 8), ei tavusta 56 kuten SI9:lla.
			unsigned char *b = (unsigned char *) buf;
			tulkExtOtsikko(b, result);
			tulkExtLeimat(b, result, 136, 4, 256);
			break;
			}
		case 10: {
			// pCard (SIID 4M-6M): valiaikaleimat alkaen tavusta 176
			// (block 1 byte 48), enintaan 20 kpl.
			unsigned char *b = (unsigned char *) buf;
			tulkExtOtsikko(b, result);
			tulkExtLeimat(b, result, 176, 4, 256);
			break;
			}
		case 11: {
			// tCard (SIID 6M-7M): valiaikaleimat alkaen tavusta 56,
			// enintaan 25 kpl, 8-tavuisin tietuein {PTD, CN, time_H,
			// time_L, sub_H, sub_L, 0x00, 0x00} - alisekunnit ohitetaan,
			// askel 8 (ei 4 kuten muilla EXT-tyypeilla).
			unsigned char *b = (unsigned char *) buf;
			tulkExtOtsikko(b, result);
			tulkExtLeimat(b, result, 56, 8, 256);
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
			//   [32] PTD [33] CN [34:36] time - Clear punch (-> check, jos
			//        check-leimaa ei ole; ks. tulkExtOtsikko)
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
			result->finish = (b[21] == 0xEE) ? TMAALI0 :
				256L*b[22] + b[23] + (b[20] & 1) * 43200L;
			result->start  = (b[25] == 0xEE) ? TMAALI0 :
				256L*b[26] + b[27] + (b[24] & 1) * 43200L;
			result->check  = (b[29] == 0xEE) ? TMAALI0 :
				256L*b[30] + b[31] + (b[28] & 1) * 43200L;
			if (result->check == TMAALI0 && b[33] != 0xEE)
				result->check = 256L*b[34] + b[35] + (b[32] & 1) * 43200L;
			tulkExtLeimat(b, result, 256, 4, buflen);
			break;
			}
		}
	return(0);
	}

// ===========================================================================
// SI-aseman lukusekvenssi (ks. SITulkinta.h ja TpLaitteet.cpp:n lue_SI).

int siTunnistaIlmoitus(const unsigned char *b, int n)
	{
	if (n < 4 || (b[1] == 102 && n < 10))
		return SIILM_ODOTA;
	if (b[0] == 0x02 && b[1] == 'F' && b[2] == 'I' && b[3] == 0x03)
		return SIILM_SI5;
	if (b[1] == 0xE5)
		return SIILM_SI5EXT;
	if (b[1] == 0xE8)
		return SIILM_SI9;
	if (b[1] == 0xE6)
		return SIILM_SI6EXT;
	if (b[1] == 102 && b[8] == 0x03)
		return SIILM_SI6;
	if (b[1] == 0x31)
		return SIILM_SI5AUTO;
	if (b[1] == 0xD3)
		return SIILM_D3;
	return SIILM_EI;
	}

// Kerattava pituus tyypeittain, indeksi SItype-5: SI5 133, SI6 402,
// SI9/SI10-11/SI8/pCard/tCard 256 (SI10/11 tarkentuu leimamaaran mukaan),
// SI6-EXT 512.
static const int SIdatalen[8] = {133, 402, 256, 256, 256, 256, 256, 512};

void siLukuAloita(SILukuTp *t, int SItype, int SIext, int *skip)
	{
	t->SItype = SItype;
	t->nblock = 0;
	t->nblocks_needed = 1;
	t->datalen = SIdatalen[SItype-5];
	*skip = (SItype == 7 || SItype == 12) ? 6 : (SIext ? 2 : 0);
	}

int siLukuSeuraava(SILukuTp *t, const unsigned char *buf, int l, int *skip)
	{
	int pyynto = SIPYY_EI;

	// SI9-perhe: lohko 0 luettu - korttityyppi SIID:sta (tavut 25..27).
	if (t->SItype == 7 && l == 128 && t->nblock == 0) {
		unsigned long siid = buf[25] * 65536L + buf[26] * 256L + buf[27];
		t->nblock = 1;
		*skip = 9;
		if (siid >= 7000000L) {
			// SI10/11: tavu 22 = leimamaara -> 1..4 leimalohkoa (32/lohko).
			unsigned char rc = buf[22];
			t->SItype = 8;
			t->nblocks_needed = (rc + 31) / 32;
			if (t->nblocks_needed < 1) t->nblocks_needed = 1;
			if (t->nblocks_needed > 4) t->nblocks_needed = 4;
			t->datalen = 128 + t->nblocks_needed * 128;
			pyynto = SIPYY_SI11_B4;
			}
		else {
			if (siid >= 6000000L)
				t->SItype = 11;   // tCard
			else if (siid >= 4000000L)
				t->SItype = 10;   // pCard
			else if (siid >= 2000000L)
				t->SItype = 9;    // SI8
			t->datalen = SIdatalen[t->SItype-5];
			pyynto = SIPYY_SI9_B1;
			}
		}
	// SI10/11: seuraava leimalohko, kunnes tarvittavat on luettu.
	if (t->SItype == 8 && t->nblock > 0 && t->nblock < t->nblocks_needed
		&& l == 128 + t->nblock * 128) {
		t->nblock++;
		*skip = 9;
		if (t->nblock == 2)
			pyynto = SIPYY_SI11_B5;
		else if (t->nblock == 3)
			pyynto = SIPYY_SI11_B6;
		else
			pyynto = SIPYY_SI11_B7;
		}
	// SI6-EXT: kiintea sarja lohkoja 0, 1, 6, 7.
	if (t->SItype == 12 && l == 128 && t->nblock == 0) {
		t->nblock = 1;
		*skip = 9;
		pyynto = SIPYY_SI6X_B1;
		}
	else if (t->SItype == 12 && l == 256 && t->nblock == 1) {
		t->nblock = 2;
		*skip = 9;
		pyynto = SIPYY_SI6X_B6;
		}
	else if (t->SItype == 12 && l == 384 && t->nblock == 2) {
		t->nblock = 3;
		*skip = 9;
		pyynto = SIPYY_SI6X_B7;
		}
	return pyynto;
	}

int siAutosendAlku(const unsigned char *pre, int prelen, unsigned char *buf, int *dle)
	{
	int n = 0, k;

	buf[n++] = 0x02;
	buf[n++] = 0x02;
	buf[n++] = 0x31;
	for (k = 2; k < prelen; k++) {
		if (*dle) {
			buf[n++] = pre[k];
			*dle = 0;
			}
		else if (pre[k] == 16)
			*dle = 1;
		else
			buf[n++] = pre[k];
		}
	return n;
	}

// ===========================================================================
// tulkSI:n tulos emittp:n leimoiksi (ks. SITulkinta.h ja tall_emit).

void siEmitLeimat(const SIResultTp *r, int t0, unsigned char *ctrlcode,
	UINT16 *ctrltime, int maxn, long *lukuaika)
	{
	INT32 lukija_abs, start, check, finish;
	int i;

	*lukuaika = -1;
	memset(ctrlcode, 0, maxn);
	memset(ctrltime, 0, maxn * sizeof(UINT16));
	// lukija on t_time_l-asteikolla (kymmenyksia suhteessa t0:aan, +/-12 h);
	// muunnetaan vuorokaudenajaksi sekunteina kuten leimat.
	lukija_abs = (INT32) (((r->lukija + ((INT32) t0 + 48) * 36000L + 24L*36000L) %
		(24L*36000L)) / 10);
	start = r->start == 61166L ? TMAALI0 : r->start;
	check = r->check == 61166L ? TMAALI0 : r->check;
	finish = r->finish == 61166L ? TMAALI0 : r->finish;
	// Ei lahtoleimaa: nollaus-/tarkastusleima, jos se on ennen ensimmaista
	// leimaa ja enintaan 12 h sita aiemmin; muuten ensimmainen leima (alla).
	if (start == TMAALI0 && check != TMAALI0) {
		for (i = 1; i < maxn && !r->ct[i]; i++) ;
		if (i >= maxn || (r->ct[i] - check + 86400L) % 86400L <= 43200L)
			start = check;
		}
	for (i = 0; i < maxn-2; i++) {
		ctrlcode[i] = (unsigned char) r->cc[i];
		ctrltime[i] = (UINT16) r->ct[i];
		if (start == TMAALI0 && r->ct[i])
			start = r->ct[i];
		if (r->ct[i] && start != TMAALI0)
			ctrltime[i] = (UINT16) ((r->ct[i] - start + 86400L) % 86400L);
		}
	for (i = maxn; i > 1; i--)
		if (ctrlcode[i-1])
			break;
	if (i < maxn-1 && i > 1) {
		if (finish != TMAALI0 && start != TMAALI0) {
			ctrlcode[i] = 240;
			ctrltime[i] = (UINT16) ((finish - start + 86400L) % 86400L);
			i++;
			}
		ctrlcode[i] = 250;
		if (start != TMAALI0)
			ctrltime[i] = (UINT16) ((lukija_abs - start + 86400L) % 86400L);
		else
			ctrltime[i] = (UINT16) ((lukija_abs + 86400L) % 86400L);
		if (start != TMAALI0)
			*lukuaika = (lukija_abs - start + 86400L) % 86400L;
		}
	}

// ===========================================================================
// Toistuvat leimat (ks. SITulkinta.h).

int siToistoAlkuun(const unsigned char *ctrlcode, int n, int j, int lukija,
	int *ohitetut, int *nohit)
	{
	int k;

	*nohit = 0;
	while ((k = (j+n-1)%n) != lukija && k != (lukija+1)%n && ctrlcode[k] == ctrlcode[j]) {
		ohitetut[(*nohit)++] = j;
		j = k;
		}
	return j;
	}

int siMaaliToistoAlkuun(const unsigned char *ctrlcode, int n, int lk, int l)
	{
	while (l > 1 && ctrlcode[(lk+l-1)%n] == ctrlcode[(lk+l)%n])
		l--;
	return l;
	}
