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

// Yksikkotestit kaikkien SportIdent-korttityyppien tulkinnalle
// (Tp/SITulkinta.cpp:tulkSI): SI5 (5), SI6 (6), SI9 (7), SI10/SI11 (8),
// SI8 (9), pCard (10), tCard (11), SI6 EXT-protokollan kautta (12).
//
// t0 annetaan tassa aina arvolla 0: se vaikuttaa vain lukija-kenttaan
// (t_time_l:n kautta), ei badge/check/finish/start/cc/ct-kenttiin, joita
// nama testit tarkastavat.
//
// SI5/SI6 rakennetaan oikeina SI5tp/SI6tp-struktuina (ei kasin lasketuin
// tavuoffsetein): nailla struktuilla ei ole omaa #pragma pack -maaritysta,
// joten kaantaja voi lisata niihin tayteta. Kun testi asettaa kentat nimella
// (tp.CN[0] = ...) ja tulkSI lukee samat struktin jasenet, molemmat nakevat
// saman - todellisen - muistiasettelun kaantajasta riippumatta. SI9/SI10-11/
// SI8/pCard/tCard sen sijaan ovat EXT-protokollan tavuprotokollaa: niissa
// kasin lasketut tavuoffsetit ovat oikea tapa, koska tulkSI itsekin lukee ne
// suoraan tavupuskurista eika struktin kautta.

#include <stdio.h>
#include <string.h>
#include <tptype.h>
#include <TpDef.h>
#include "sitypes.h"
#include "SITulkinta.h"
#include "doctest.h"

// Lukuhetki BIOS-tikkeina kuten lue_SI:n SIt (biostime, DAYTICKS = 1573040
// tikkia/vrk). SI5 tarvitsee lukuhetken aamu-/iltapaivan paattelyyn
// (ks. SITulkinta.cpp:tulkSI5Puolipaiva).
static INT32 siTics(int h, int m, int s)
{
	return (INT32) ((h*3600L + m*60L + s) * 1573040.0 / 86400.0 + 0.5);
}

// ===========================================================================
// SI5 (SItype == 5): legacy-protokolla, SI5tp-struktin kautta.
// ===========================================================================

TEST_CASE("SI5: badge puretaan CN[0..1]:sta, CNS jatkaa sen yli 65535:n jos > 1")
{
	SI5tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.CN[0] = 10; tp.CN[1] = 20; tp.CNS = 0;
	tulkSI((char *) &tp, &result, 0, 5, sizeof(tp), 0);
	CHECK(result.badge == 256L*10 + 20);
}

TEST_CASE("SI5: CNS > 1 lisaa CNS*100000 badgeen (SI5-sarjan jatko)")
{
	SI5tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.CN[0] = 10; tp.CN[1] = 20; tp.CNS = 2;
	tulkSI((char *) &tp, &result, 0, 5, sizeof(tp), 0);
	CHECK(result.badge == 256L*10 + 20 + 2*100000L);
}

TEST_CASE("SI5: start/check/finish puretaan ST/CT/FT-kentista")
{
	SI5tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.ST[0] = 11; tp.ST[1] = 0;   // 11*256 s
	tp.CT[0] = 12; tp.CT[1] = 0;
	tp.FT[0] = 13; tp.FT[1] = 0;
	tulkSI((char *) &tp, &result, siTics(1, 0, 0), 5, sizeof(tp), 0);  // luettu klo 1.00

	CHECK(result.start  == 11L*256);
	CHECK(result.check  == 12L*256);
	CHECK(result.finish == 13L*256);
}

TEST_CASE("SI5: ensimmainen rastileima (row[0].c[0]) puretaan ja verrataan starttiin")
{
	SI5tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.ST[0] = 0; tp.ST[1] = 100;             // start = 100 s (ei 61166)
	tp.row[0].c[0].cc = 31;
	tp.row[0].c[0].ct[0] = 0; tp.row[0].c[0].ct[1] = 50;  // 50 s < start=100
	tulkSI((char *) &tp, &result, siTics(12, 30, 0), 5, sizeof(tp), 0);  // luettu klo 12.30

	// 50 < 100 (start) ja start != 61166 -> +43200 kaannos
	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 50L + 43200L);
}

// Oikea SI5-kortti (SIID 229401, SI Configin lokista): badgen alatavu 0xD9,
// leima-ajat 0x84.., tyhja lahto/maali/tarkastus 0xEE-EE ja kayttamattomat
// leimapaikat 00-EE-EE. Tavut >= 0x80 on tulkittava etumerkittomina
// riippumatta siita, onko kaantajan char etumerkillinen (HkKisaWin/bcc32c,
// g++) vai ei (ViestiWin, VS /J).
static void buildSI5_229401(SI5tp *tp)
{
	static const unsigned char punches[3][3] = {
		{31, 0x84, 0x2F}, {32, 0x84, 0x82}, {50, 0x8B, 0xC7} };
	int r, i;

	memset(tp, 0, sizeof(*tp));
	tp->CN[0] = (char) 0x72; tp->CN[1] = (char) 0xD9; tp->CNS = 2;
	tp->ST[0] = tp->ST[1] = (char) 0xEE;
	tp->FT[0] = tp->FT[1] = (char) 0xEE;
	tp->CT[0] = tp->CT[1] = (char) 0xEE;
	for (r = 0; r < 6; r++)
		for (i = 0; i < 5; i++) {
			tp->row[r].c[i].ct[0] = tp->row[r].c[i].ct[1] = (char) 0xEE;
			}
	for (i = 0; i < 3; i++) {
		tp->row[0].c[i].cc = (char) punches[i][0];
		tp->row[0].c[i].ct[0] = (char) punches[i][1];
		tp->row[0].c[i].ct[1] = (char) punches[i][2];
		}
}

TEST_CASE("SI5: tavut >= 0x80 tulkitaan etumerkittomina (oikea kortti 229401)")
{
	SI5tp tp;
	SIResultTp result;

	buildSI5_229401(&tp);
	tulkSI((char *) &tp, &result, siTics(9, 44, 27), 5, sizeof(tp), 0);  // luettu aamulla

	CHECK(result.badge == 229401L);
	CHECK(result.start == TMAALI0);    // 0xEEEE = ei lahtoa
	CHECK(result.finish == TMAALI0);
	CHECK(result.check == TMAALI0);
	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 33839L);     // 09:23:59
	CHECK((int) (unsigned char) result.cc[2] == 32);
	CHECK(result.ct[2] == 33922L);     // 09:25:22
	CHECK((int) (unsigned char) result.cc[3] == 50);
	CHECK(result.ct[3] == 35783L);     // 09:56:23
}

// SI5:ssa ei ole aamu-/iltapaivatietoa: iltapaivalla luetun kortin leimat
// siirretaan +12 h, jotta ne ovat samalla asteikolla kuin PC:n kellosta
// laskettu lukuhetki (muuten HkIV.cpp:n 250-rivi menisi 12 h pieleen).
TEST_CASE("SI5: iltapaivalla luettu kortti -> leimat +12 h")
{
	SI5tp tp;
	SIResultTp result;

	buildSI5_229401(&tp);
	tulkSI((char *) &tp, &result, siTics(21, 44, 27), 5, sizeof(tp), 0);

	CHECK(result.ct[1] == 33839L + 43200L);   // 21:23:59
	CHECK(result.ct[2] == 33922L + 43200L);   // 21:25:22
	CHECK(result.ct[3] == 35783L + 43200L);   // 21:56:23
	CHECK(result.start == TMAALI0);           // tyhjat pysyvat tyhjina
	CHECK(result.finish == TMAALI0);
	CHECK(result.ct[4] == 0L);                // kayttamattomat eivat siirry
}

TEST_CASE("SI5: puolipaivan paattely toimii myos t0 != 0 (viestin oletus t0=12)")
{
	SI5tp tp;
	SIResultTp result;

	buildSI5_229401(&tp);
	tulkSI((char *) &tp, &result, siTics(21, 44, 27), 5, sizeof(tp), 12);
	CHECK(result.ct[1] == 33839L + 43200L);

	tulkSI((char *) &tp, &result, siTics(9, 44, 27), 5, sizeof(tp), 12);
	CHECK(result.ct[1] == 33839L);
}

TEST_CASE("SI5: leima juuri ennen puoltayota, luku puolenyon jalkeen")
{
	// Kortin 11:50:00 (= 23:50:00), luettu 00:10:00 -> leima 23:50:00.
	SI5tp tp;
	SIResultTp result;
	unsigned t = 11*3600 + 50*60;

	memset(&tp, 0, sizeof(tp));
	tp.ST[0] = tp.ST[1] = (char) 0xEE;
	tp.FT[0] = tp.FT[1] = (char) 0xEE;
	tp.CT[0] = tp.CT[1] = (char) 0xEE;
	tp.row[0].c[0].cc = 31;
	tp.row[0].c[0].ct[0] = (char) (t >> 8);
	tp.row[0].c[0].ct[1] = (char) t;
	tulkSI((char *) &tp, &result, siTics(0, 10, 0), 5, sizeof(tp), 0);

	CHECK(result.ct[1] == 23L*3600 + 50*60);

	// Leima 08:00, kortti luetaan vasta 15:00 (7 h myohemmin) -> 08:00,
	// ei 20:00 (joka olisi lukuhetkea lahempana, mutta tulevaisuudessa).
	t = 8*3600;
	tp.row[0].c[0].ct[0] = (char) (t >> 8);
	tp.row[0].c[0].ct[1] = (char) t;
	tulkSI((char *) &tp, &result, siTics(15, 0, 0), 5, sizeof(tp), 0);
	CHECK(result.ct[1] == 8L*3600);
}

TEST_CASE("SI5: kayttamattomat leimapaikat (00-EE-EE) jaavat nollaksi")
{
	SI5tp tp;
	SIResultTp result;
	int i;

	buildSI5_229401(&tp);
	tulkSI((char *) &tp, &result, 0, 5, sizeof(tp), 0);

	for (i = 4; i <= 30; i++) {
		CHECK(result.cc[i] == 0);
		CHECK(result.ct[i] == 0L);
		}
}

TEST_CASE("SI5: myohemmat leimat kaannetaan +12h jos pienempia kuin edellinen")
{
	SI5tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.row[0].c[0].cc = 31;
	tp.row[0].c[0].ct[0] = 0; tp.row[0].c[0].ct[1] = 120;  // ct[1] = 120
	tp.row[0].c[1].cc = 32;
	tp.row[0].c[1].ct[0] = 0; tp.row[0].c[1].ct[1] = 100;  // ct[2] = 100 < ct[1]
	tulkSI((char *) &tp, &result, siTics(12, 30, 0), 5, sizeof(tp), 0);  // luettu klo 12.30

	CHECK(result.ct[1] == 120L);
	CHECK(result.ct[2] == 100L + 43200L);
}

TEST_CASE("SI5: row[r].ccx tallentuu cc[31+r]:hen")
{
	SI5tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.row[2].ccx = 99;
	tulkSI((char *) &tp, &result, 0, 5, sizeof(tp), 0);
	CHECK((int) (unsigned char) result.cc[31+2] == 99);
}

// ===========================================================================
// SI6 (SItype == 6): legacy multi-block-protokolla, SI6tp-struktin kautta.
// ===========================================================================

TEST_CASE("SI6: badge puretaan CN[0..3]:sta big-endian (4 tavua)")
{
	SI6tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.CN[0] = 1; tp.CN[1] = 2; tp.CN[2] = 3; tp.CN[3] = 4;
	tulkSI((char *) &tp, &result, 0, 6, sizeof(tp), 0);
	CHECK(result.badge == ((1L*256+2)*256+3)*256+4);
}

TEST_CASE("SI6: start/check/finish puretaan PT-kentista, PTD-bitti 0 lisaa 12h")
{
	SI6tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.st.PT[0]  = 0; tp.st.PT[1]  = 50; tp.st.PTD  = 1;  // 50 + 12h
	tp.chk.PT[0] = 0; tp.chk.PT[1] = 60; tp.chk.PTD = 0;  // 60, ei kaannosta
	tp.fi.PT[0]  = 0; tp.fi.PT[1]  = 70; tp.fi.PTD  = 1;  // 70 + 12h
	tulkSI((char *) &tp, &result, 0, 6, sizeof(tp), 0);

	CHECK(result.start  == 50L + 43200L);
	CHECK(result.check  == 60L);
	CHECK(result.finish == 70L + 43200L);
}

TEST_CASE("SI6: ilman tarkastusleimaa (0xEEEE) nollausleima clr tulee check-kenttaan")
{
	SI6tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.chk.PT[0] = tp.chk.PT[1] = (char) 0xEE; tp.chk.CN = (char) 0xEE;
	tp.clr.CN = (char) 0xFF;
	tp.clr.PT[0] = (char) 0x83; tp.clr.PT[1] = (char) 0xB3; tp.clr.PTD = 1;
	tulkSI((char *) &tp, &result, 0, 6, sizeof(tp), 0);

	CHECK(result.check == 0x83B3L + 43200L);   // 21:21:55
}

// pblk[0] ja pblk[1] ovat kaksi ERI 32-leiman lohkoa (64 yhteensa), eivat
// sama alue kahteen kertaan. Aiemmin molemmat kirjoittivat samaan
// cc[1..32]/ct[1..32]-alueeseen, joten pblk[1] ylikirjoitti pblk[0]:n -
// korjattu (ks. SITulkinta.cpp:n case 6) niin, etta ne jatkuvat perakkain:
// pblk[0] -> cc[1..32], pblk[1] -> cc[33..64]. Leimalaskuri PP rajaa maaran.
TEST_CASE("SI6: pblk[0] ja pblk[1] jatkuvat perakkain, ei ylikirjoita")
{
	SI6tp tp;
	SIResultTp result;
	int i;

	memset(&tp, 0, sizeof(tp));
	for (i = 0; i < 32; i++) {
		tp.pblk[0].punch[i].CN = (char) (31 + i);
		tp.pblk[0].punch[i].PT[1] = (char) (10 + i);
		}
	tp.pblk[1].punch[0].CN = 99;
	tp.pblk[1].punch[0].PT[1] = 120;
	tp.pblk[1].punch[1].CN = 98;                 // PP:n ulkopuolella
	tp.PP = 33;
	tulkSI((char *) &tp, &result, 0, 6, sizeof(tp), 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 10L);
	CHECK((int) (unsigned char) result.cc[32] == 31+31);
	CHECK((int) (unsigned char) result.cc[33] == 99);
	CHECK(result.ct[33] == 120L);
	CHECK(result.cc[34] == 0);
}

TEST_CASE("SI6: leimalaskuri PP rajaa listan, CN=0xEE (rasti 238) on oikea leima")
{
	SI6tp tp;
	SIResultTp result;

	memset(&tp, 0xEE, sizeof(tp));
	tp.pblk[0].punch[0].PTD = 0; tp.pblk[0].punch[0].CN = 31;
	tp.pblk[0].punch[0].PT[0] = 0; tp.pblk[0].punch[0].PT[1] = 10;
	tp.pblk[0].punch[1].PTD = 0; tp.pblk[0].punch[1].CN = (char) 0xEE;   // rasti 238
	tp.pblk[0].punch[1].PT[0] = 0; tp.pblk[0].punch[1].PT[1] = 20;
	tp.pblk[0].punch[2].PTD = 0; tp.pblk[0].punch[2].CN = 32;            // vanha leima
	tp.pblk[0].punch[2].PT[0] = 0; tp.pblk[0].punch[2].PT[1] = 30;
	tp.PP = 2;
	tulkSI((char *) &tp, &result, 0, 6, sizeof(tp), 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK((int) (unsigned char) result.cc[2] == 238);
	CHECK(result.ct[2] == 20L);
	CHECK(result.cc[3] == 0);
	CHECK(result.ct[3] == 0L);
}

// ===========================================================================
// Yhteiset apufunktiot SI9/SI10-11/SI8/pCard/tCard -testeille (EXT-protokolla,
// tavupuskuriin suoraan indeksoiva osuus tulkSI:sta).
// ===========================================================================

// Rakentaa synteettisen EXT-protokollan lohkon (oletusarvoisesti 256 tavua,
// block0 + block1). Layout on varmistettu oikeaa laitteistoa/pcapia vasten:
//   [8]  PTD  [9]  CN   [10:12) time   -- Check-leimaus
//   [12] PTD  [13] CN   [14:16) time   -- Lahtoleimaus (CN=EE -> ei lahtoa)
//   [16] PTD  [17] CN   [18:20) time   -- Maalileimaus
//   [22]      RC leimalaskuri           -- buildBlock: 0, testi asettaa
//   [25:28)   SIID (3 tavua, big-endian)
//
// HUOM: SITulkinta.cpp:n case 7:n oma kommentti kuvaa tama jarjestyksen
// vielä vaarin (vaihtaa lahdon ja "ei kaytossa" -alueen keskenaan) - se on
// vain kommentti, ei vaikuta koodin ajokayttaytymiseen, jota tama testi
// seuraa.
static void buildBlock(unsigned char *b, int len, unsigned long siid)
{
	memset(b, 0xEE, len);  // CN=EE kaikkialla = "ei leimaa" oletusarvona
	b[22] = 0;             // RC: leimalaskuri, tyhja kortti (ks. tulkExtLeimat)
	b[25] = (unsigned char) (siid >> 16);
	b[26] = (unsigned char) (siid >> 8);
	b[27] = (unsigned char) siid;
}

static void setPunch(unsigned char *b, int offs, unsigned char ptd, unsigned char cn, unsigned t)
{
	b[offs]   = ptd;
	b[offs+1] = cn;
	b[offs+2] = (unsigned char) (t >> 8);
	b[offs+3] = (unsigned char) t;
}

// ===========================================================================
// SI9 (SItype == 7): EXT-protokolla, valiaikaleimat alkaen tavusta 56, askel 4.
// ===========================================================================

TEST_CASE("SI9: badge puretaan SIID:sta 3 tavusta big-endian")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	tulkSI((char *) buf, &result, 0, 7, 256, 0);
	CHECK(result.badge == 1009090L);
}

TEST_CASE("SI9: check/finish/start puretaan kun CN != EE")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	setPunch(buf, 8,  0, 200, 12*3600);   // check klo 12:00:00
	setPunch(buf, 16, 0, 200, 13*3600);   // maali klo 13:00:00
	setPunch(buf, 12, 0, 200, 11*3600);   // lahto klo 11:00:00
	tulkSI((char *) buf, &result, 0, 7, 256, 0);

	CHECK(result.check  == 12*3600L);
	CHECK(result.finish == 13*3600L);
	CHECK(result.start  == 11*3600L);
}

TEST_CASE("SI9: aika EE EE lahtotavussa tarkoittaa ettei lahtoa ole, arvo TMAALI0")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	// setPunch ei tassa kutsuta lahtotavuille -> aika pysyy EE EE:na
	tulkSI((char *) buf, &result, 0, 7, 256, 0);
	CHECK(result.start == TMAALI0);
}

TEST_CASE("SI9: aika EE EE check- ja maalitavussa tarkoittaa ettei arvoa ole, arvo TMAALI0")
{
	unsigned char buf[256];
	SIResultTp result;

	// Ennen tata korjausta check/finish palauttivat 0 (=keskiyo) CN=EE:lla,
	// mika HkIV.cpp/VIv.cpp tulkitsi VIRHEELLISESTI oikeaksi maaliajaksi
	// (ne vertaavat vain TMAALI0:aan, eivat 0:aan) - tuottaen vaaria
	// koodi-240-leimoja korteille joilla ei oikeasti ollut maalia.
	buildBlock(buf, 256, 1009090UL);
	tulkSI((char *) buf, &result, 0, 7, 256, 0);
	CHECK(result.check == TMAALI0);
	CHECK(result.finish == TMAALI0);
}

TEST_CASE("SI9: PTD-bitti 0 lisaa 12h (puolipaivan kaannos)")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	setPunch(buf, 8, 1 /* PTD bit0 */, 200, 100);  // 100s + 12h
	tulkSI((char *) buf, &result, 0, 7, 256, 0);
	CHECK(result.check == 100L + 43200L);
}

TEST_CASE("SI9: valiaikaleimat luetaan tavusta 56 alkaen, RC (tavu 22) rajaa listan")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	setPunch(buf, 56, 0, 31, 12*3600);       // 1. rasti: koodi 31, klo 12:00:00
	setPunch(buf, 60, 0, 32, 12*3600+30);    // 2. rasti: koodi 32, klo 12:00:30
	buf[22] = 2;                             // RC = 2 -> lista paattyy tahan
	tulkSI((char *) buf, &result, 0, 7, 256, 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 12*3600L);
	CHECK((int) (unsigned char) result.cc[2] == 32);
	CHECK(result.ct[2] == 12*3600L+30);
}

TEST_CASE("SI9: ei +12h-kaannosta kun rastiaika on pienempi kuin edellinen (PTD kertoo puolipaivan)")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	setPunch(buf, 12, 0, 200, 10*3600);      // lahto klo 10:00:00
	setPunch(buf, 56, 0, 31, 10*3600+600);   // 1. rasti klo 10:10:00
	setPunch(buf, 60, 0, 32, 10*3600+595);   // 2. rasti: leimasimen kello 10 s jaljessa
	setPunch(buf, 64, 0, 33, 9*3600+3000);   // 3. rasti ennen lahtoa (kello jaljessa)
	buf[22] = 3;
	tulkSI((char *) buf, &result, 0, 7, 256, 0);

	CHECK(result.ct[1] == 10*3600L+600);
	CHECK(result.ct[2] == 10*3600L+595);     // ennen: +12 h
	CHECK(result.ct[3] == 9*3600L+3000);     // ennen: +12 h
}

TEST_CASE("SI9: puolenyon ylitys - PTD-aika, siEmitLeimat laskee modulo 24 h")
{
	unsigned char buf[256];
	SIResultTp result;
	unsigned char cc[50];
	UINT16 ct[50];
	long lukuaika;

	buildBlock(buf, 256, 1009090UL);
	setPunch(buf, 12, 1, 200, 11*3600+3000); // lahto klo 23:50:00 (PTD pm)
	setPunch(buf, 56, 0, 31, 600);           // 1. rasti klo 00:10:00 (PTD am)
	buf[22] = 1;
	tulkSI((char *) buf, &result, siTics(0, 20, 0), 7, 256, 0);

	CHECK(result.start == 23*3600L+3000);
	CHECK(result.ct[1] == 600L);             // ennen: 600 + 12 h
	siEmitLeimat(&result, 0, cc, ct, 50, &lukuaika);
	CHECK(cc[1] == 31);
	CHECK(ct[1] == 20*60);                   // 20 min lahdosta
}

TEST_CASE("SI9: lahtoasema 238 (CN=0xEE) oikealla ajalla on lahtoleima")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	setPunch(buf, 12, 0, 0xEE, 10*3600);     // lahto, asemakoodi 238
	setPunch(buf, 16, 0, 0xEE, 11*3600);     // maali, asemakoodi 238
	tulkSI((char *) buf, &result, 0, 7, 256, 0);

	CHECK(result.start == 10*3600L);
	CHECK(result.finish == 11*3600L);

	// Asemakoodi != EE mutta aika EE EE -> ei leimaa.
	setPunch(buf, 12, 0, 3, 0xEEEE);
	tulkSI((char *) buf, &result, 0, 7, 256, 0);
	CHECK(result.start == TMAALI0);
}

TEST_CASE("SI6-EXT: otsikon leimat tunnistetaan ajasta, ei asemakoodista")
{
	unsigned char buf[512];
	SIResultTp result;

	memset(buf, 0xEE, sizeof(buf));
	setPunch(buf, 24, 0, 0xEE, 10*3600);     // lahto asemalta 238
	setPunch(buf, 28, 0, 5, 0xEEEE);         // tarkastus: koodi mutta ei aikaa
	setPunch(buf, 32, 0, 0xEE, 9*3600);      // nollaus asemalta 238
	tulkSI((char *) buf, &result, 0, 12, 384, 0);

	CHECK(result.start == 10*3600L);
	CHECK(result.check == 9*3600L);          // nollaus tarkastuksen tilalle
	CHECK(result.finish == TMAALI0);
}

// Dokumentoi olemassa olevan rajatapauksen: cc[]/ct[] on kokoa 66 (indeksit
// 0..65), mutta silmukka sallii kirjoituksen indeksiin 66 asti (r <= 66 taman
// jalkeen kun r on jo kasvatettu). 50 valiaikaleimaa on SI9:n 256-tavuisen
// lohkon teoreettinen maksimi ((256-56)/4), eli 66 ei ole SI9:lla
// saavutettavissa - taman testin tarkoitus on vain varmistaa ettei silmukka
// kaadu tai ylivuoda lahella lohkon todellista maksimia.
TEST_CASE("SI9: cc/ct-taulukot eivat ylivuoda lohkon todellisessa maksimissa")
{
	unsigned char buf[256];
	SIResultTp result;
	int i;

	buildBlock(buf, 256, 1009090UL);
	for (i = 0; i < 50 && 56 + i*4 + 3 < 256; i++)
		setPunch(buf, 56 + i*4, 0, (unsigned char) (33 + (i % 200)), 12*3600 + i);
	buf[22] = 50;
	tulkSI((char *) buf, &result, 0, 7, 256, 0);
	// Ei kaadu / ei ylivuotoa; viimeinen mahtuva leima on oikein tallessa.
	CHECK((int) (unsigned char) result.cc[1] == 33);
}

// ===========================================================================
// SI10/SI11 (SItype == 8): EXT-protokolla, valiaikaleimat alkaen tavusta 128,
// jatkuu lisalohkoissa buflen:iin asti (256-640 tavua).
// ===========================================================================

TEST_CASE("SI10/11: header (badge/check/finish/start) sama kuin SI9:lla")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 7000000UL);
	setPunch(buf, 8,  0, 200, 12*3600);
	setPunch(buf, 16, 0, 200, 13*3600);
	setPunch(buf, 12, 0, 200, 11*3600);   // lahto klo 11:00:00
	tulkSI((char *) buf, &result, 0, 8, 256, 0);

	CHECK(result.badge  == 7000000L);
	CHECK(result.check  == 12*3600L);
	CHECK(result.finish == 13*3600L);
	CHECK(result.start  == 11*3600L);
}

TEST_CASE("SI10/11: valiaikaleimat alkavat tavusta 128, ei 56:sta kuten SI9:lla")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 7000000UL);
	setPunch(buf, 56, 0, 99, 1*3600);   // SI9:n paikalla oleva data EI saa nakya
	setPunch(buf, 128, 0, 31, 12*3600); // oikea 1. rasti SI10/11:lla
	buf[22] = 1;
	tulkSI((char *) buf, &result, 0, 8, 256, 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 12*3600L);
}

TEST_CASE("SI10/11: leimat jatkuvat lisalohkoissa buflen:iin asti (640 tavua, 4 lohkoa)")
{
	unsigned char buf[640];
	SIResultTp result;

	int i;

	// Leimat ovat kortilla perakkain i=128,132,136,... ja RC kertoo niiden
	// maaran; lohkorajat (256, 384) eivat katkaise listaa.
	buildBlock(buf, 640, 7000000UL);
	for (i = 128; i + 3 < 512; i += 4)
		setPunch(buf, i, 0, (unsigned char) (40 + ((i-128)/4) % 100), 10*3600 + (i-128)/4);
	buf[22] = 96;                        // lohkot 4-6 taynna
	tulkSI((char *) buf, &result, 0, 8, 640, 0);

	// r=1 -> i=128 (lohko 4, ensimmainen)
	CHECK((int) (unsigned char) result.cc[1] == 40);
	// r=33 -> i=128+4*32=256 (lohko 5:n ensimmainen)
	CHECK((int) (unsigned char) result.cc[33] == 40+32);
	CHECK(result.ct[33] == 10*3600L + 32);
	// r=65 -> i=128+4*64=384 (lohko 6:n ensimmainen)
	CHECK((int) (unsigned char) result.cc[65] == 40+64);
	CHECK(result.ct[65] == 10*3600L + 64);
}

TEST_CASE("SI10/11: buflen rajaa lukua - 256-tavuinen puskuri ei lue lohkoa 5:ta")
{
	unsigned char buf[640];
	SIResultTp result;
	int i;

	buildBlock(buf, 640, 7000000UL);
	for (i = 0; i < 32; i++)             // lohko 4 taynna
		setPunch(buf, 128 + i*4, 0, (unsigned char) (31 + i), 10*3600 + i);
	setPunch(buf, 256, 0, 99, 11*3600);  // 33. leima, buflen=256:n ulkopuolella
	buf[22] = 33;
	tulkSI((char *) buf, &result, 0, 8, 256, 0);  // buflen=256!

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK((int) (unsigned char) result.cc[32] == 31+31);
	// result on tulkSI:n omaa muistia (nollattu memset(result,0,...):lla alussa),
	// ei syotepuskuria, joten lukematon paikka on 0 - ei syotepuskurin 0xEE-tayte.
	CHECK((int) (unsigned char) result.cc[33] == 0);
}

// ===========================================================================
// SI8 (SItype == 9): EXT-protokolla, valiaikaleimat alkaen tavusta 136.
// ===========================================================================

TEST_CASE("SI8: header sama kuin SI9:lla, leimat alkavat tavusta 136")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 2000000UL);
	setPunch(buf, 8, 0, 200, 12*3600);
	setPunch(buf, 56, 0, 99, 1*3600);    // SI9:n paikka - EI saa nakya SI8:lla
	setPunch(buf, 136, 0, 31, 10*3600);  // oikea 1. rasti SI8:lla
	buf[22] = 1;
	tulkSI((char *) buf, &result, 0, 9, 256, 0);

	CHECK(result.badge == 2000000L);
	CHECK(result.check == 12*3600L);
	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 10*3600L);
}

// ===========================================================================
// pCard (SItype == 10): EXT-protokolla, valiaikaleimat alkaen tavusta 176.
// ===========================================================================

TEST_CASE("pCard: header sama kuin SI9:lla, leimat alkavat tavusta 176")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 4000000UL);
	setPunch(buf, 8, 0, 200, 12*3600);
	setPunch(buf, 136, 0, 99, 1*3600);   // SI8:n paikka - EI saa nakya pCardilla
	setPunch(buf, 176, 0, 31, 10*3600);  // oikea 1. rasti pCardilla
	buf[22] = 1;
	tulkSI((char *) buf, &result, 0, 10, 256, 0);

	CHECK(result.badge == 4000000L);
	CHECK(result.check == 12*3600L);
	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 10*3600L);
}

// ===========================================================================
// tCard (SItype == 11): EXT-protokolla, leimat alkaen tavusta 56 kuten SI9:lla,
// mutta 8-tavuisin tietuein (4 ylimaarasta alisekunti-/varatavua per leima).
// ===========================================================================

TEST_CASE("tCard: header sama kuin SI9:lla")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 6000000UL);
	setPunch(buf, 8, 0, 200, 12*3600);
	tulkSI((char *) buf, &result, 0, 11, 256, 0);

	CHECK(result.badge == 6000000L);
	CHECK(result.check == 12*3600L);
}

TEST_CASE("tCard: leimat askeltavat 8 tavua (ei 4:aa kuten SI9:lla)")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 6000000UL);
	setPunch(buf, 56, 0, 31, 10*3600);      // 1. leima, tavu 56
	setPunch(buf, 64, 0, 32, 11*3600);      // 2. leima ODOTETAAN tavusta 64 (56+8)
	setPunch(buf, 60, 0, 99, 1*3600);       // "valiin jaava" tavu 60 EI saa vaikuttaa
	buf[22] = 2;
	tulkSI((char *) buf, &result, 0, 11, 256, 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK((int) (unsigned char) result.cc[2] == 32);
	CHECK(result.ct[2] == 11*3600L);
}

// ===========================================================================
// SI6 EXT-protokollan kautta (SItype == 12): eri langansiirtokoodaus samalle
// korttisukupolvelle kuin legacy SI6 (SItype 6) - EI sama tavuasettelu kuin
// SI9+:lla (SItype 7-11). Badge tavuilla [10:14). SIbuf = lohko 0 + lohko 6
// [+ lohko 7]: leimat alkaen tavusta 128, 32 leimaa/lohko, kuten legacy SI6:n
// kaksi SI6PBLK-lohkoa.
//
// buildBlock/setPunch (SI9+:aa varten) eivat sovi tahan (badge/otsikko eri
// tavuilla), joten testit rakentavat puskurin suoraan.
// ===========================================================================

TEST_CASE("SI6-EXT: badge puretaan tavuista [10:14) big-endian (4 tavua)")
{
	unsigned char buf[512];
	SIResultTp result;

	memset(buf, 0xEE, sizeof(buf));
	buf[10] = 0x00; buf[11] = 0x08; buf[12] = 0xD8; buf[13] = 0x57;  // 579671
	tulkSI((char *) buf, &result, 0, 12, 384, 0);

	CHECK(result.badge == 579671L);
}

TEST_CASE("SI6-EXT: oikea kortti (SIID 579671) - finish/check puretaan, ei lahtoa")
{
	// Todellinen tavudumppi lohkosta 0 (varmennettu oikealla kortilla).
	unsigned char buf[512];
	SIResultTp result;

	memset(buf, 0xEE, sizeof(buf));
	buf[10] = 0x00; buf[11] = 0x08; buf[12] = 0xD8; buf[13] = 0x57;  // badge
	setPunch(buf, 20, 0x0D, 0x0A, 0x258B);          // maali: PTD=0D,CN=0A,aika=25 8B
	// lahto: tavut [24:28) jaavat 0xEE:ksi -> ei lahtoa
	setPunch(buf, 28, 0x0D, 0x03, 0x0E46);           // tarkastus: PTD=0D,CN=03,aika=0E 46
	tulkSI((char *) buf, &result, 0, 12, 384, 0);

	CHECK(result.badge  == 579671L);
	CHECK(result.finish == 52811L);   // 256*0x25+0x8B + 43200 (PTD&1=1)
	CHECK(result.start  == TMAALI0);
	CHECK(result.check  == 46854L);   // 256*0x0E+0x46 + 43200
}

TEST_CASE("SI6-EXT: ilman tarkastusleimaa nollausleima (tavu 32) tulee check-kenttaan")
{
	// Oikea kortti 579671 (SI Config): Clear 255 Sa 21.21.55, Check tyhja.
	unsigned char buf[512];
	SIResultTp result;

	memset(buf, 0xEE, sizeof(buf));
	setPunch(buf, 32, 0x0D, 0xFF, 0x83B3);
	tulkSI((char *) buf, &result, 0, 12, 384, 0);
	CHECK(result.check == 0x83B3L + 43200L);   // 21:21:55

	// Kun tarkastusleima on, se voittaa nollausleiman.
	setPunch(buf, 28, 0x0D, 0x03, 0x0E46);
	tulkSI((char *) buf, &result, 0, 12, 384, 0);
	CHECK(result.check == 46854L);
}

TEST_CASE("SI6-EXT: leimat alkavat tavusta 128 (lohko 6 lohkon 0 jalkeen), ei 56:sta")
{
	unsigned char buf[512];
	SIResultTp result;

	memset(buf, 0xEE, sizeof(buf));
	buf[10] = 0; buf[11] = 0; buf[12] = 0; buf[13] = 1;
	setPunch(buf, 56,  0, 99, 1*3600);   // SI9:n paikka - EI saa nakya
	setPunch(buf, 128, 0, 31, 12*3600);  // oikea 1. rasti SI6-EXT:lla (lohko 6)
	buf[18] = 1;                         // SI6-EXT:n leimalaskuri
	tulkSI((char *) buf, &result, 0, 12, 384, 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 12*3600L);
}

TEST_CASE("SI6-EXT: leimat jatkuvat lohkoon 7 (tavu 256), RC (tavu 18) rajaa")
{
	// Todellinen kortti: 8 leimaa lohkossa 6, loput 0xEE. Tama testi kattaa
	// lisaksi jatkumisen lohkoon 7, jota ei ollut tallessa oikeassa dumpissa.
	unsigned char buf[512];
	SIResultTp result;
	int i;

	memset(buf, 0xEE, sizeof(buf));
	buf[10] = 0; buf[11] = 0; buf[12] = 0; buf[13] = 1;
	// tayta lohko 6 kokonaan (32 leimaa, tavut 128..255) ja jatka lohkoon 7:aan
	for (i = 128; i + 3 < 256+16; i += 4)
		setPunch(buf, i, 0, (unsigned char) (40 + (i-128)/4), 10*3600 + (i-128)/4);
	buf[18] = 36;                        // 32 lohkossa 6 + 4 lohkossa 7
	tulkSI((char *) buf, &result, 0, 12, 384, 0);

	// r=32 -> i=128+4*31=252 (lohko 6:n viimeinen)
	CHECK((int) (unsigned char) result.cc[32] == 40+31);
	// r=33 -> i=256 (lohko 7:n ensimmainen)
	CHECK((int) (unsigned char) result.cc[33] == 40+32);
	CHECK(result.ct[33] == 10*3600L + 32);
}

// ===========================================================================
// Maksimileimamaarat jokaiselle korttityypille (SIResultTp.cc[66]/ct[66] -
// vain indeksit 0..65 kaytettavissa, 65 tallennettavaa leimaa).
//
// Tama joukko loydettiin/kirjattiin taman istunnon aikana, koska
// SITulkinta.cpp:ssa oli "if (r <= 66)" jokaisessa EXT-protokollan
// tapauksessa (7-11, ja uusi 12): kun r==66, koodi kirjoitti result->ct[66]:een,
// joka on YHDEN INT32:n verran SIResultTp-rakenteen VIIMEISEN jasenen ohi -
// siis rakenteen ULKOPUOLELLE (undefined behaviour, ei vain testipuskurissa
// vaan oikeassa HkMaali/HkKisaWin-ohjelmassa). SI10/SI11-kortti taydella 128
// leimalla laukaisi taman aidosti (65 < 128). Korjattu "if (r < 66)":ksi.
//
// Vain SI10/11 ylittaa 65 leimaa kaytannossa (SI9 50, SI8 30, pCard 20,
// tCard 25, SI6-EXT 64, SI5 30, legacy SI6 32 - kaikki jaavat rajan
// alapuolelle), mutta kaikki tyypit testataan tassa taydella maaralla, jotta
// vastaava bugi ei paase hiipimaan takaisin mihinkaan tyyppiin.
// ===========================================================================

TEST_CASE("SI5: maksimileimamaara (30, kaikki 6 rivia x 5) mahtuu")
{
	// HUOM: PT/ct-tavut luetaan SIGNED char -kenttina (ks. tiedoston alun
	// kommentti signed charista muualla istunnossa); +1 s/leima pitaa
	// molemmat tavut < 128:ssa koko kaavan ajan, jottei etumerkin laajennus
	// vaikuta odotettuihin arvoihin (60 s/leima olisi ylittanyt 128:n useaan
	// otteeseen).
	SI5tp tp;
	SIResultTp result;
	int r, i;

	memset(&tp, 0, sizeof(tp));
	for (r = 0; r < 6; r++) {
		for (i = 0; i < 5; i++) {
			unsigned t = 3600 + (r*5+i);   // nouseva, ei +12h-kaannoksia
			tp.row[r].c[i].cc = (char) (33 + r*5 + i);
			tp.row[r].c[i].ct[0] = (char) (t >> 8);
			tp.row[r].c[i].ct[1] = (char) t;
			}
		}
	tulkSI((char *) &tp, &result, siTics(2, 0, 0), 5, sizeof(tp), 0);  // luettu klo 2.00

	CHECK((int) (unsigned char) result.cc[1]  == 33);
	CHECK((int) (unsigned char) result.cc[30] == 33+29);
	CHECK(result.ct[30] == 3600L + 29L);
}

TEST_CASE("SI6: maksimileimamaara (64, pblk[0]+pblk[1] taynna) tallentuu kokonaan")
{
	// HUOM: sama signed char -varovaisuus kuin SI5:n maksimitestissa yalla.
	SI6tp tp;
	SIResultTp result;
	int i;

	memset(&tp, 0, sizeof(tp));
	for (i = 0; i < 32; i++) {
		unsigned t = 3600 + i;
		tp.pblk[0].punch[i].CN = (char) (33 + i);
		tp.pblk[0].punch[i].PT[0] = (char) (t >> 8);
		tp.pblk[0].punch[i].PT[1] = (char) t;
		}
	for (i = 0; i < 32; i++) {
		unsigned t = 7200 + i;
		tp.pblk[1].punch[i].CN = (char) (70 + i);
		tp.pblk[1].punch[i].PT[0] = (char) (t >> 8);
		tp.pblk[1].punch[i].PT[1] = (char) t;
		}
	tp.PP = 64;
	tulkSI((char *) &tp, &result, 0, 6, sizeof(tp), 0);

	CHECK((int) (unsigned char) result.cc[1]  == 33);
	CHECK((int) (unsigned char) result.cc[32] == 33+31);
	CHECK(result.ct[32] == 3600L + 31L);
	CHECK((int) (unsigned char) result.cc[33] == 70);
	CHECK((int) (unsigned char) result.cc[64] == 70+31);
	CHECK(result.ct[64] == 7200L + 31L);
}

TEST_CASE("SI9: maksimileimamaara (50) tallentuu kokonaan")
{
	unsigned char buf[256];
	SIResultTp result;
	int i;

	buildBlock(buf, 256, 1009090UL);
	for (i = 0; i < 50; i++)
		setPunch(buf, 56 + i*4, 0, (unsigned char) (33 + i), 3600 + i*60);
	buf[22] = 50;
	tulkSI((char *) buf, &result, 0, 7, 256, 0);

	CHECK((int) (unsigned char) result.cc[1]  == 33);
	CHECK((int) (unsigned char) result.cc[50] == 33+49);
	CHECK(result.ct[50] == 3600L + 49*60);
}

TEST_CASE("SI10/11: 128 leimaa (taysi 4 lohkoa) ei ylivuoda cc/ct[66]-taulukkoa")
{
	// Kriittinen regressiotesti (ks. yllaoleva selitys): tayttaa kortin ihan
	// oikeaan maksimiin (128 leimaa, 4 taytta lohkoa) ja tarkistaa, etta
	// viimeinen TALLENNETTAVA indeksi (65 - taulukon suurin sallittu) on
	// oikein eika mitaan kaadu/korruptoidu matkalla.
	unsigned char buf[640];
	SIResultTp result;
	int i;

	buildBlock(buf, 640, 7000000UL);
	for (i = 128; i + 3 < 640; i += 4)
		setPunch(buf, i, 0, (unsigned char) (33 + ((i-128)/4) % 200), 3600 + (i-128)/4*60);
	buf[22] = 128;
	tulkSI((char *) buf, &result, 0, 8, 640, 0);

	CHECK((int) (unsigned char) result.cc[1]  == 33);
	CHECK((int) (unsigned char) result.cc[65] == 33+64);
	CHECK(result.ct[65] == 3600L + 64*60);
}

TEST_CASE("SI8: maksimileimamaara (30) tallentuu kokonaan")
{
	unsigned char buf[256];
	SIResultTp result;
	int i;

	buildBlock(buf, 256, 2000000UL);
	for (i = 0; i < 30; i++)
		setPunch(buf, 136 + i*4, 0, (unsigned char) (33 + i), 3600 + i*60);
	buf[22] = 30;
	tulkSI((char *) buf, &result, 0, 9, 256, 0);

	CHECK((int) (unsigned char) result.cc[1]  == 33);
	CHECK((int) (unsigned char) result.cc[30] == 33+29);
	CHECK(result.ct[30] == 3600L + 29*60);
}

TEST_CASE("pCard: maksimileimamaara (20) tallentuu kokonaan")
{
	unsigned char buf[256];
	SIResultTp result;
	int i;

	buildBlock(buf, 256, 4000000UL);
	for (i = 0; i < 20; i++)
		setPunch(buf, 176 + i*4, 0, (unsigned char) (33 + i), 3600 + i*60);
	buf[22] = 20;
	tulkSI((char *) buf, &result, 0, 10, 256, 0);

	CHECK((int) (unsigned char) result.cc[1]  == 33);
	CHECK((int) (unsigned char) result.cc[20] == 33+19);
	CHECK(result.ct[20] == 3600L + 19*60);
}

TEST_CASE("tCard: maksimileimamaara (25) tallentuu kokonaan")
{
	unsigned char buf[256];
	SIResultTp result;
	int i;

	buildBlock(buf, 256, 6000000UL);
	for (i = 0; i < 25; i++)
		setPunch(buf, 56 + i*8, 0, (unsigned char) (33 + i), 3600 + i*60);
	buf[22] = 25;
	tulkSI((char *) buf, &result, 0, 11, 256, 0);

	CHECK((int) (unsigned char) result.cc[1]  == 33);
	CHECK((int) (unsigned char) result.cc[25] == 33+24);
	CHECK(result.ct[25] == 3600L + 24*60);
}

TEST_CASE("SI6-EXT: maksimileimamaara (64, lohkot 6+7 taynna) tallentuu kokonaan")
{
	unsigned char buf[512];
	SIResultTp result;
	int i;

	memset(buf, 0xEE, sizeof(buf));
	buf[10] = 0; buf[11] = 0; buf[12] = 0; buf[13] = 1;
	for (i = 128; i + 3 < 384; i += 4)
		setPunch(buf, i, 0, (unsigned char) (33 + ((i-128)/4) % 200), 3600 + (i-128)/4*60);
	buf[18] = 64;
	tulkSI((char *) buf, &result, 0, 12, 384, 0);

	CHECK((int) (unsigned char) result.cc[1]  == 33);
	CHECK((int) (unsigned char) result.cc[64] == 33+63);
	CHECK(result.ct[64] == 3600L + 63*60);
}

// ===========================================================================
// SI-aseman lukusekvenssi (lue_SI): siTunnistaIlmoitus, siLukuAloita,
// siLukuSeuraava, siAutosendAlku.
// ===========================================================================

TEST_CASE("siTunnistaIlmoitus: kortin ilmoitukset tunnistetaan")
{
	unsigned char si5[4]  = {0x02, 'F', 'I', 0x03};
	unsigned char e5[12]  = {0x02, 0xE5, 0x06, 0x00, 0x0A, 0x00, 0x02, 0x72, 0xD9, 0, 0, 0x03};
	unsigned char e8[12]  = {0x02, 0xE8, 0x06, 0x00, 0x0A, 0x00, 0x0F, 0x65, 0xC2, 0, 0, 0x03};
	unsigned char e6[12]  = {0x02, 0xE6, 0x06, 0x00, 0x0A, 0x00, 0x08, 0xD8, 0x57, 0, 0, 0x03};
	unsigned char si6[10] = {0x02, 102, 0x83, 0, 0, 0x08, 0xD8, 0x57, 0x03, 0};
	unsigned char a31[10] = {0x02, 0x31, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00};
	unsigned char d3[10]  = {0x02, 0xD3, 0x0D, 0x00, 0x32, 0x00, 0x02, 0x72, 0xD9, 0x01};

	CHECK(siTunnistaIlmoitus(si5, 4) == SIILM_SI5);
	CHECK(siTunnistaIlmoitus(e5, 10) == SIILM_SI5EXT);
	CHECK(siTunnistaIlmoitus(e8, 10) == SIILM_SI9);
	CHECK(siTunnistaIlmoitus(e6, 10) == SIILM_SI6EXT);
	CHECK(siTunnistaIlmoitus(si6, 10) == SIILM_SI6);
	CHECK(siTunnistaIlmoitus(a31, 10) == SIILM_SI5AUTO);
	CHECK(siTunnistaIlmoitus(d3, 10) == SIILM_D3);
}

TEST_CASE("siTunnistaIlmoitus: liian lyhyt puskuri odottaa, tuntematon hylataan")
{
	unsigned char e8[12] = {0x02, 0xE8, 0x06, 0x00, 0x0A, 0x00, 0x0F, 0x65, 0xC2, 0, 0, 0x03};
	unsigned char si6[10] = {0x02, 102, 0x83, 0, 0, 0x08, 0xD8, 0x57, 0x03, 0};
	unsigned char e7[10] = {0x02, 0xE7, 0x06, 0x00, 0x0A, 0x00, 0x0F, 0x65, 0xC2, 0};

	CHECK(siTunnistaIlmoitus(e8, 3) == SIILM_ODOTA);
	// vanha SI6-ilmoitus tarvitsee 10 tavua (ETX tavussa 8)
	CHECK(siTunnistaIlmoitus(si6, 9) == SIILM_ODOTA);
	si6[8] = 0x00;
	CHECK(siTunnistaIlmoitus(si6, 10) == SIILM_EI);
	// E7 = kortti poistettu - ei kasitella
	CHECK(siTunnistaIlmoitus(e7, 10) == SIILM_EI);
}

TEST_CASE("siLukuAloita: kerattava pituus tyypeittain")
{
	SILukuTp t;
	siLukuAloita(&t, 5);  CHECK(t.datalen == 133);
	siLukuAloita(&t, 6);  CHECK(t.datalen == 402);
	siLukuAloita(&t, 7);  CHECK(t.datalen == 256);
	siLukuAloita(&t, 12); CHECK(t.datalen == 384);
	CHECK(t.nblock == 0);
	CHECK(t.nblocks_needed == 1);
}

// Ajaa lukusekvenssin kuten lue_SI: l kasvaa tavu kerrallaan kunnes
// datalen tayttyy, ja kerataan lahetetyt pyynnot. buf = kortin data.
static int ajaLuku(int SItype, int SIext, const unsigned char *buf, int *pyynnot,
	SILukuTp *t)
{
	int l, n = 0;

	(void) SIext;
	siLukuAloita(t, SItype);
	for (l = 1; l <= 640; l++) {
		int p = siLukuSeuraava(t, buf, l);
		if (p != SIPYY_EI) {
			pyynnot[n++] = p;
			}
		if (l == t->datalen)
			break;
		}
	return n;
}

TEST_CASE("siLukuSeuraava: SI9-perheen tyyppi SIID:sta, yksi lisalohko")
{
	unsigned char b[640];
	int p[8], n;
	SILukuTp t;

	buildBlock(b, 640, 1009090);        // SI9
	n = ajaLuku(7, 1, b, p, &t);
	CHECK(n == 1); CHECK(p[0] == SIPYY_SI9_B1); CHECK(t.SItype == 7); CHECK(t.datalen == 256);

	buildBlock(b, 640, 1999999);        // SI9 ylaraja
	n = ajaLuku(7, 1, b, p, &t);
	CHECK(t.SItype == 7);

	buildBlock(b, 640, 2000123);        // SI8
	n = ajaLuku(7, 1, b, p, &t);
	CHECK(n == 1); CHECK(p[0] == SIPYY_SI9_B1); CHECK(t.SItype == 9); CHECK(t.datalen == 256);

	buildBlock(b, 640, 4000001);        // pCard
	n = ajaLuku(7, 1, b, p, &t);
	CHECK(t.SItype == 10); CHECK(p[0] == SIPYY_SI9_B1);

	buildBlock(b, 640, 6000001);        // tCard
	n = ajaLuku(7, 1, b, p, &t);
	CHECK(t.SItype == 11); CHECK(p[0] == SIPYY_SI9_B1);
}

TEST_CASE("siLukuSeuraava: SI10/11 lukee leimamaaran mukaan 1..4 leimalohkoa")
{
	unsigned char b[640];
	int p[8], n;
	SILukuTp t;

	buildBlock(b, 640, 7000000);        // SI10 alaraja
	b[22] = 0;                          // ei leimoja -> silti 1 lohko
	n = ajaLuku(7, 1, b, p, &t);
	CHECK(t.SItype == 8); CHECK(t.nblocks_needed == 1); CHECK(t.datalen == 256);
	CHECK(n == 1); CHECK(p[0] == SIPYY_SI11_B4);

	buildBlock(b, 640, 8647177);
	b[22] = 40;                         // 40 leimaa -> 2 lohkoa
	n = ajaLuku(7, 1, b, p, &t);
	CHECK(t.nblocks_needed == 2); CHECK(t.datalen == 384);
	REQUIRE(n == 2);
	CHECK(p[0] == SIPYY_SI11_B4); CHECK(p[1] == SIPYY_SI11_B5);

	b[22] = 128;                        // taysi kortti -> 4 lohkoa
	n = ajaLuku(7, 1, b, p, &t);
	CHECK(t.datalen == 640);
	REQUIRE(n == 4);
	CHECK(p[0] == SIPYY_SI11_B4); CHECK(p[1] == SIPYY_SI11_B5);
	CHECK(p[2] == SIPYY_SI11_B6); CHECK(p[3] == SIPYY_SI11_B7);

	b[22] = 200;                        // yli 128 -> rajataan 4:aan
	n = ajaLuku(7, 1, b, p, &t);
	CHECK(t.nblocks_needed == 4); CHECK(n == 4);
}

TEST_CASE("siLukuSeuraava: SI6-EXT lukee lohkon 6, ja lohkon 7 vain yli 32 leimalla")
{
	unsigned char b[640];
	int p[8], n;
	SILukuTp t;

	memset(b, 0, sizeof(b));
	b[18] = 4;                          // 4 leimaa -> vain lohko 6
	n = ajaLuku(12, 1, b, p, &t);
	REQUIRE(n == 1);
	CHECK(p[0] == SIPYY_SI6X_B6);
	CHECK(t.datalen == 256);

	b[18] = 32;                         // lohko 6 taynna, ei viela lohkoa 7
	n = ajaLuku(12, 1, b, p, &t);
	CHECK(n == 1); CHECK(t.datalen == 256);

	b[18] = 33;
	n = ajaLuku(12, 1, b, p, &t);
	REQUIRE(n == 2);
	CHECK(p[0] == SIPYY_SI6X_B6); CHECK(p[1] == SIPYY_SI6X_B7);
	CHECK(t.datalen == 384);

	b[18] = 192;                        // 192 leiman tila: lohkot 6-7 riittavat 64:aan
	n = ajaLuku(12, 1, b, p, &t);
	CHECK(n == 2); CHECK(t.datalen == 384);
}

TEST_CASE("siLukuSeuraava: tuntematon korttisarja E8:lla hylataan")
{
	unsigned char b[640];
	int p[8], n;
	SILukuTp t;
	unsigned long tuntemattomat[] = {999999UL, 2003000UL, 2003999UL, 3000000UL,
		3999999UL, 5000000UL, 5999999UL, 10000000UL, 14000000UL, 16711680UL};
	unsigned long tunnetut[] = {1000000UL, 1999999UL, 2000000UL, 2002999UL,
		2004000UL, 2999999UL, 4000000UL, 4999999UL, 6000000UL, 6999999UL,
		7000000UL, 8999999UL, 9999999UL};
	int tyypit[] = {7, 7, 9, 9, 9, 9, 10, 10, 11, 11, 8, 8, 8};
	size_t i;

	for (i = 0; i < sizeof(tuntemattomat) / sizeof(tuntemattomat[0]); i++) {
		CAPTURE(tuntemattomat[i]);
		buildBlock(b, 640, tuntemattomat[i]);
		n = ajaLuku(7, 1, b, p, &t);
		REQUIRE(n == 1);
		CHECK(p[0] == SIPYY_TUNTEMATON);
		}
	for (i = 0; i < sizeof(tunnetut) / sizeof(tunnetut[0]); i++) {
		CAPTURE(tunnetut[i]);
		buildBlock(b, 640, tunnetut[i]);
		n = ajaLuku(7, 1, b, p, &t);
		REQUIRE(n >= 1);
		CHECK(p[0] != SIPYY_TUNTEMATON);
		CHECK(t.SItype == tyypit[i]);
		}
}

TEST_CASE("siUusitaanko: virhe, NAK ja aikaraja uusitaan SIYRITYKSET kertaan asti, poisto ei")
{
	CHECK(SIYRITYKSET == 3);
	CHECK(siUusitaanko(SIKEHYS_VIRHE, 1) == 1);
	CHECK(siUusitaanko(SIKEHYS_VIRHE, 2) == 1);
	CHECK(siUusitaanko(SIKEHYS_VIRHE, 3) == 0);
	CHECK(siUusitaanko(SIKEHYS_NAK, 1) == 1);
	CHECK(siUusitaanko(SIKEHYS_NAK, 3) == 0);
	CHECK(siUusitaanko(SIKEHYS_KESKEN, 2) == 1);   // aikaraja
	CHECK(siUusitaanko(SIKEHYS_KESKEN, 3) == 0);
	CHECK(siUusitaanko(SIKEHYS_POISTO, 1) == 0);
	CHECK(siUusitaanko(SIKEHYS_OK, 1) == 0);
}

TEST_CASE("siVanhaKehysOk: vanhan protokollan STX/ETX-paikat")
{
	unsigned char b[402];
	int i;

	memset(b, 0x41, sizeof(b));
	b[0] = 0x02; b[132] = 0x03;
	CHECK(siVanhaKehysOk(b, 133, 5) == 1);
	CHECK(siVanhaKehysOk(b, 132, 5) == 0);   // pituus vaara
	b[132] = 0x41;
	CHECK(siVanhaKehysOk(b, 133, 5) == 0);   // ETX puuttuu (tavu hukassa)
	b[132] = 0x03; b[0] = 0x31;
	CHECK(siVanhaKehysOk(b, 133, 5) == 0);   // STX puuttuu

	memset(b, 0x41, sizeof(b));
	for (i = 0; i < 3; i++) {
		b[134*i] = 0x02;
		b[134*i + 133] = 0x03;
		}
	CHECK(siVanhaKehysOk(b, 402, 6) == 1);
	b[267] = 0x41;
	CHECK(siVanhaKehysOk(b, 402, 6) == 0);   // 2. lohko siirtynyt
	b[267] = 0x03;
	CHECK(siVanhaKehysOk(b, 401, 6) == 0);
	CHECK(siVanhaKehysOk(b, 402, 7) == 0);   // ei vanhan protokollan tyyppi

	// SI5 auto-send -puskuri (siAutosendAlku rakentaa otsikon 02 02 31).
	{
	SI5tp tp;
	memset(&tp, 0, sizeof(tp));
	tp.stx = 0x02; tp.CD49 = 0x02; tp.CSI = 0x31; tp.etx = 0x03;
	CHECK(siVanhaKehysOk((unsigned char *) &tp, sizeof(tp), 5) == 1);
	}
}

TEST_CASE("siLukuSeuraava: vanhan protokollan SI5/SI6 ei pyyda lisalohkoja")
{
	unsigned char b[640];
	int p[8];
	SILukuTp t;

	memset(b, 0, sizeof(b));
	CHECK(ajaLuku(5, 0, b, p, &t) == 0);
	CHECK(ajaLuku(5, 1, b, p, &t) == 0);
	CHECK(ajaLuku(6, 0, b, p, &t) == 0);
}

TEST_CASE("siAutosendAlku: SI5tp-otsikko ja DLE-koodauksen purku")
{
	unsigned char pre[6] = {0x02, 0x31, 0x41, 0x10, 0x05, 0x42};
	unsigned char buf[16];
	int dle = 0, n;

	n = siAutosendAlku(pre, 6, buf, &dle);
	REQUIRE(n == 6);
	CHECK(buf[0] == 0x02); CHECK(buf[1] == 0x02); CHECK(buf[2] == 0x31);
	CHECK(buf[3] == 0x41); CHECK(buf[4] == 0x05); CHECK(buf[5] == 0x42);
	CHECK(dle == 0);

	// puskuri loppuu DLE:hen -> tila siirtyy lukusilmukkaan
	n = siAutosendAlku(pre, 4, buf, &dle);
	CHECK(n == 4);
	CHECK(dle == 1);
}

TEST_CASE("SI5 auto-send: asemalta tuleva kehys tuottaa oikean kortin (229401)")
{
	// Kehys linjalla: 02 31 <128 datatavua DLE-koodattuna> CS 03. lue_SI
	// lukee ensin 10 tavua (r_msg_len), siAutosendAlku rakentaa niista
	// SIbuf:n alun ja loput luetaan auto-send-tilan lukusilmukalla.
	SI5tp tp;
	SIResultTp result;
	unsigned char *data, wire[400], buf[640];
	int nw = 0, i, n, dle = 0;

	buildSI5_229401(&tp);
	data = (unsigned char *) &tp;
	wire[nw++] = 0x02;
	wire[nw++] = 0x31;
	for (i = 3; i < 131; i++) {          // SI5tp:n data = tavut 3..130
		if (data[i] < 0x20)
			wire[nw++] = 0x10;
		wire[nw++] = data[i];
		}
	wire[nw++] = 0x10; wire[nw++] = 0x00;   // CS (koodattuna)
	wire[nw++] = 0x03;

	n = siAutosendAlku(wire, 10, buf, &dle);
	for (i = 10; i < nw && n < 133; i++) {   // lue_SI:n auto-send-silmukka
		if (dle) { buf[n++] = wire[i]; dle = 0; }
		else if (wire[i] == 16) dle = 1;
		else buf[n++] = wire[i];
		}
	CHECK(n == 133);
	tulkSI((char *) buf, &result, siTics(10, 0, 0), 5, 133, 0);
	CHECK(result.badge == 229401);
	CHECK(result.cc[1] == 31);
}

// ===========================================================================
// tulkSI:n tulos emittp:n leimoiksi (tall_emit): siEmitLeimat
// ===========================================================================

// lukija t_time_l-asteikolla (kymmenyksia), kun t0 = 0: lukija_abs = s.
static INT32 lukijaT(long s)
{
	return (INT32) (s * 10L);
}

static void tyhjaTulos(SIResultTp *r)
{
	memset(r, 0, sizeof(*r));
	r->start = r->check = r->finish = 61166L;
}

TEST_CASE("siEmitLeimat: leimat, maali- ja lukijarivi suhteessa lahtoon")
{
	SIResultTp r;
	unsigned char cc[50];
	UINT16 ct[50];
	long lukuaika;

	tyhjaTulos(&r);
	r.start = 36000;                                    // 10:00:00
	r.cc[1] = 31; r.ct[1] = 36100;
	r.cc[2] = 32; r.ct[2] = 36200;
	r.cc[3] = 33; r.ct[3] = 36300;
	r.finish = 36400;
	r.lukija = lukijaT(36500);
	siEmitLeimat(&r, 0, cc, ct, 50, &lukuaika);
	CHECK(cc[1] == 31); CHECK(ct[1] == 100);
	CHECK(cc[3] == 33); CHECK(ct[3] == 300);
	CHECK(cc[4] == 240); CHECK(ct[4] == 400);           // maali
	CHECK(cc[5] == 250); CHECK(ct[5] == 500);           // lukija
	CHECK(cc[6] == 0);
	CHECK(lukuaika == 500);
}

TEST_CASE("siEmitLeimat: ilman lahtoleimaa nollahetki on nollaus, jos enintaan 12 h ennen")
{
	SIResultTp r;
	unsigned char cc[50];
	UINT16 ct[50];
	long lukuaika;

	tyhjaTulos(&r);
	r.check = 35000;                                    // nollaus 1100 s ennen
	r.cc[1] = 31; r.ct[1] = 36100;
	r.lukija = lukijaT(36500);
	siEmitLeimat(&r, 0, cc, ct, 50, &lukuaika);
	CHECK(ct[1] == 1100);
	CHECK(lukuaika == 1500);

	// yli 12 h vanha nollaus (edellinen paiva) ei kelpaa -> ensimmainen leima
	r.check = 36100 - 43201 + 86400;
	siEmitLeimat(&r, 0, cc, ct, 50, &lukuaika);
	CHECK(ct[1] == 0);
	CHECK(lukuaika == 400);
}

TEST_CASE("siEmitLeimat: ilman lahtoa ja nollausta ensimmainen leima on nollahetki")
{
	SIResultTp r;
	unsigned char cc[50];
	UINT16 ct[50];
	long lukuaika;

	tyhjaTulos(&r);
	r.cc[1] = 31; r.ct[1] = 36100;
	r.cc[2] = 32; r.ct[2] = 36250;
	r.lukija = lukijaT(36500);
	siEmitLeimat(&r, 0, cc, ct, 50, &lukuaika);
	CHECK(ct[1] == 0); CHECK(ct[2] == 150);
	CHECK(cc[3] == 250); CHECK(ct[3] == 400);           // ei maalia -> suoraan lukija
}

TEST_CASE("siEmitLeimat: iltapaivan ajat (> 65535 s) ja puoliyon ylitys")
{
	SIResultTp r;
	unsigned char cc[50];
	UINT16 ct[50];
	long lukuaika;

	tyhjaTulos(&r);
	r.cc[1] = 31; r.ct[1] = 66600;                      // 18:30, ei mahdu 16 bittiin
	r.cc[2] = 32; r.ct[2] = 66900;
	r.lukija = lukijaT(67000);
	siEmitLeimat(&r, 0, cc, ct, 50, &lukuaika);
	CHECK(ct[1] == 0); CHECK(ct[2] == 300);

	tyhjaTulos(&r);
	r.start = 86000;                                    // 23:53:20
	r.cc[1] = 31; r.ct[1] = 200;                        // 00:03:20
	r.lukija = lukijaT(400);
	siEmitLeimat(&r, 0, cc, ct, 50, &lukuaika);
	CHECK(ct[1] == 600);
	CHECK(lukuaika == 800);
}

TEST_CASE("siEmitLeimat: yli 47 leimaa - maali- ja lukijarivi mahtuvat silti")
{
	SIResultTp r;
	unsigned char cc[50];
	UINT16 ct[50];
	long lukuaika;
	int i;

	tyhjaTulos(&r);
	r.start = 36000;
	for (i = 1; i <= 60; i++) {
		r.cc[i] = (char) (30 + i);
		r.ct[i] = 36000 + 10 * i;
		}
	r.finish = 37000;
	r.lukija = lukijaT(37100);
	siEmitLeimat(&r, 0, cc, ct, 50, &lukuaika);
	CHECK(cc[47] == 77);                                // viimeinen mahtuva leima
	CHECK(cc[48] == 240); CHECK(ct[48] == 1000);
	CHECK(cc[49] == 250); CHECK(ct[49] == 1100);
	CHECK(lukuaika == 1100);

	// tasan 47 leimaa: sama tulos, ei pudotuksia
	tyhjaTulos(&r);
	r.start = 36000;
	for (i = 1; i <= 47; i++) {
		r.cc[i] = (char) (30 + i);
		r.ct[i] = 36000 + 10 * i;
		}
	r.finish = 37000;
	r.lukija = lukijaT(37100);
	siEmitLeimat(&r, 0, cc, ct, 50, &lukuaika);
	CHECK(cc[47] == 77); CHECK(cc[48] == 240); CHECK(cc[49] == 250);
}

TEST_CASE("siEmitLeimat: tyhja kortti - ei lukijarivia eika 12 h -varoitusta")
{
	SIResultTp r;
	unsigned char cc[50];
	UINT16 ct[50];
	long lukuaika;

	tyhjaTulos(&r);
	r.check = 35000;
	r.lukija = lukijaT(36000);
	siEmitLeimat(&r, 0, cc, ct, 50, &lukuaika);
	CHECK(cc[1] == 0); CHECK(cc[2] == 0);
	CHECK(lukuaika == -1);
}

TEST_CASE("siEmitLeimat: lukuaika yli 12 h nollauksesta (varoitus)")
{
	SIResultTp r;
	unsigned char cc[50];
	UINT16 ct[50];
	long lukuaika;

	tyhjaTulos(&r);
	r.start = 3600;                                     // 01:00
	r.cc[1] = 31; r.ct[1] = 3700;
	r.lukija = lukijaT(3600 + 13 * 3600L);             // 14:00
	siEmitLeimat(&r, 0, cc, ct, 50, &lukuaika);
	CHECK(lukuaika == 13 * 3600L);
}

// ===========================================================================
// Toistuvat leimat (tarkista, e_maaliaika): siToistoAlkuun, siMaaliToistoAlkuun
// ===========================================================================

TEST_CASE("siToistoAlkuun: peraakkaisista saman rastin leimoista ensimmainen")
{
	// lukija indeksissa 6; rastin 32 leimat indekseissa 2..4
	unsigned char c[10] = {0, 31, 32, 32, 32, 100, 250, 0, 0, 0};
	int ohit[10], nohit, j;

	j = siToistoAlkuun(c, 10, 4, 6, ohit, &nohit);
	CHECK(j == 2);
	REQUIRE(nohit == 2);
	CHECK(ohit[0] == 4); CHECK(ohit[1] == 3);          // myohemmat = ylimaaraiset
}

TEST_CASE("siToistoAlkuun: ei toistoa -> leima ennallaan")
{
	unsigned char c[10] = {0, 31, 32, 33, 34, 100, 250, 0, 0, 0};
	int ohit[10], nohit;

	CHECK(siToistoAlkuun(c, 10, 4, 6, ohit, &nohit) == 4);
	CHECK(nohit == 0);
}

TEST_CASE("siToistoAlkuun: ei kulje lukijan (lukija, lukija+1) ohi rengaspuskurissa")
{
	unsigned char c[10] = {40, 40, 40, 0, 0, 0, 0, 0, 0, 40};
	int ohit[10], nohit;

	// lukija = 7: j = 1 -> 0 -> 9 (rengas), pysahtyy ennen 8:aa (lukija+1)
	CHECK(siToistoAlkuun(c, 10, 1, 7, ohit, &nohit) == 9);
	CHECK(nohit == 2);
	// lukija = 3: indeksi 4 on lukija+1 -> ei siirryta siihen
	unsigned char c2[10] = {0, 0, 0, 250, 40, 40, 0, 0, 0, 0};
	CHECK(siToistoAlkuun(c2, 10, 5, 3, ohit, &nohit) == 5);
	CHECK(nohit == 0);
}

TEST_CASE("siMaaliToistoAlkuun: perakkaisista maalileimoista ensimmainen")
{
	unsigned char c[10] = {0, 31, 100, 100, 100, 250, 0, 0, 0, 0};

	CHECK(siMaaliToistoAlkuun(c, 10, 0, 4) == 2);
	CHECK(siMaaliToistoAlkuun(c, 10, 0, 2) == 2);      // ei toistoa
}

TEST_CASE("siMaaliToistoAlkuun: ei mene alle 1:n")
{
	unsigned char c[10] = {100, 100, 100, 100, 0, 0, 0, 0, 0, 0};

	CHECK(siMaaliToistoAlkuun(c, 10, 0, 3) == 1);
}

// ===========================================================================
// Kortin numeron uudelleenkayton askel (BADGEASKEL): siBadgeAskel
// ===========================================================================

TEST_CASE("siBadgeAskel: SportIdent-kilpailussa 10 000 000, muuten 1 000 000")
{
	CHECK(siBadgeAskel(L'I') == 10000000L);
	CHECK(siBadgeAskel(L'E') == 1000000L);   // Emit
	CHECK(siBadgeAskel(L'T') == 1000000L);   // emiTag
	CHECK(siBadgeAskel(L'S') == 1000000L);   // Sirit
	CHECK(siBadgeAskel(L' ') == 1000000L);   // ei tunnistinta
}

TEST_CASE("siBadgeAskel: siirretty numero ei voi olla oikean kortin numero")
{
	// Suurimmat oikeat numerot: Emit < 1 000 000, SportIdent SI11 < 10 000 000.
	long si11 = 9999999L, emit = 999999L;
	// SI5 229401 + 1 000 000 = 1229401 olisi SI9-kortin numero; 10 000 000:n
	// askeleella pienin siirretty numero on suurempi kuin mikaan SI-numero.
	CHECK(229401L + siBadgeAskel(L'I') > si11);
	CHECK(0L + siBadgeAskel(L'I') > si11);
	CHECK(0L + siBadgeAskel(L'E') > emit);
	// 30 osuutta (MAXOSUUSLUKU): 29 siirtoa mahtuu 32-bittiseen lukuun.
	CHECK(si11 + 29L * siBadgeAskel(L'I') < 2147483647L);
}

TEST_CASE("siNaytettavaBadge: mahtuva numero sellaisenaan")
{
	CHECK(siNaytettavaBadge(229401L, 10000000L, 7) == 229401L);
	CHECK(siNaytettavaBadge(10229401L, 10000000L, 8) == 10229401L);
	CHECK(siNaytettavaBadge(1229401L, 1000000L, 7) == 1229401L);   // Emit ennallaan
	CHECK(siNaytettavaBadge(10229401L, 10000000L, 0) == 10229401L); // ei rajaa
}

TEST_CASE("siNaytettavaBadge: liian pitka siirretty koodi -> kortin oma numero")
{
	// 7 merkin kenttaan putfld katkaisisi 10229401 -> "1022940" (vaara kortti)
	CHECK(siNaytettavaBadge(10229401L, 10000000L, 7) == 229401L);
	CHECK(siNaytettavaBadge(28647177L, 10000000L, 7) == 8647177L);  // 2 siirtoa
	CHECK(siNaytettavaBadge(129647177L, 10000000L, 8) == 9647177L);  // 12 siirtoa
	CHECK(siNaytettavaBadge(0L, 10000000L, 3) == 0L);
}

TEST_CASE("siMaxSiirrot: siirretty koodi pysyy alle toistomerkin 2 000 000 000")
{
	// SportIdent-askel: suurin kortti 9 999 999 -> 199 siirtoa (1 999 999 999)
	CHECK(siMaxSiirrot(9999999L, 10000000L) == 199L);
	CHECK(9999999L + siMaxSiirrot(9999999L, 10000000L) * 10000000L < 2000000000L);
	CHECK(siMaxSiirrot(229401L, 10000000L) == 199L);
	// 30 osuuden viesti ei koskaan osu rajaan, 250 osuuden viesti osuu
	CHECK(siMaxSiirrot(9999999L, 10000000L) >= 29L);
	CHECK(siMaxSiirrot(9999999L, 10000000L) < 249L);
	// Emit-askel: raja ei kaytannossa tule vastaan
	CHECK(siMaxSiirrot(999999L, 1000000L) == 1999L);   // 999 999 + 1999 * 1 000 000 = 1 999 999 999
	CHECK(999999L + (siMaxSiirrot(999999L, 1000000L) + 1) * 1000000L >= 2000000000L);
	// askel 1 (koodi 200, laskemtn.cpp)
	CHECK(siMaxSiirrot(200L, 1L) == 1999999799L);
	// virheelliset syotteet
	CHECK(siMaxSiirrot(-1L, 10000000L) == 0L);
	CHECK(siMaxSiirrot(9999999L, 0L) == 0L);
	CHECK(siMaxSiirrot(2000000000L, 10000000L) == 0L);
}

TEST_CASE("siViallinenEmit200: koodi 200 viallinen vain muussa kuin SportIdent-kilpailussa")
{
	CHECK(siViallinenEmit200(200L, L'E'));
	CHECK(siViallinenEmit200(200L, L'T'));
	CHECK_FALSE(siViallinenEmit200(200L, L'I'));    // SI5-kortti 200
	CHECK_FALSE(siViallinenEmit200(201L, L'E'));
	CHECK_FALSE(siViallinenEmit200(10000200L, L'I')); // siirretty SI5 200
}

// ===========================================================================
// Oikeat kortit: SI-aseman lukemat tavut (LOKI-tiedoston "SI data" -rivit,
// BSM8 EXT-protokolla) ja niista ennen RC-muutosta saatu tulkinta ("SI
// tulkinta"/"SI leimat"). Kukin kortti luettu kahdesti, toisella kerralla
// lisaleimoin. Leimalaskurin (RC) kaytto ei saa muuttaa tulosta.
// Kortin henkilotiedot (nimi, seura, sahkoposti) on korvattu tayttomerkeilla;
// tulkSI ei lue niita tavuja. SI6-EXT-dumpit (luettu lohkot 0, 1, 6, 7) on
// muunnettu nykyiseen puskuriin: lohko 0 + lohko 6 (alle 33 leimaa).
// ===========================================================================

typedef struct {
	int SItype;
	long badge, check;
	int nleima;
	int cc[8];
	long ct[8];
	const char *hex;
} SIDumpTp;

static const SIDumpTp siDumpit[] = {
	{7, 1009090L, 74315L, 3, {31, 32, 50}, {74382L, 74434L, 74593L},
		"67F4A771EAEAEAEA0901798BEEEEEEEEEEEEEEEE003203AE010F65C20CFF74243B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B0000091F79CE09207A02"
		"09327AA1EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
	{9, 2170603L, 74336L, 3, {31, 32, 50}, {74393L, 74431L, 74599L},
		"C5ED809AEAEAEAEA090179A0EEEEEEEEEEEEEEEE0032039902211EEBFFFFB7353B3BEEEE00000000000000000000000000000000000000000000000000000000"
		"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
		"0000000000000000091F79D9092079FF09327AA7EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
	{8, 9081114L, 74322L, 3, {31, 32, 50}, {74388L, 74425L, 74596L},
		"4FA7109AEAEAEAEA09017992EEEEEEEEEEEEEEEE02FE03E30F8A911A0410A5833B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B00EEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"091F79D4092079F909327AA4EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
	{12, 579671L, 74332L, 3, {31, 32, 50}, {74385L, 74427L, 74606L},
		"01010101EDEDEDED55AA0008D857793D00320304EEEEEEEEEEEEEEEEEEEEEEEE0901799CFFFFFFFF000000012020202020202020202020202020202020202020"
		"20202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020"
		"091F79D1092079FB09327AAEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
	{8, 8004087L, 76936L, 5, {31, 31, 31, 32, 50}, {77040L, 77080L, 77085L, 77136L, 79108L},
		"CE9ED064EAEAEAEA0DFF83C8EEEEEEEEEEEEEEEE04B005AD0F7A21F70C1908673B3B3B3B3B3B3B3B3B3B3BEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"0D1F84300D1F84580D1F845D0D2084901D328C44EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
	{8, 8004086L, 74319L, 3, {31, 32, 50}, {74380L, 74423L, 74576L},
		"3CAFD064EAEAEAEA0901798FEEEEEEEEEEEEEEEE02FE03E30F7A21F60C195C203B3B3B3B3B3B3B3B3B3B3BEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"091F79CC092079F709327A90EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
	{7, 1009090L, 74315L, 5, {31, 32, 50, 50, 50}, {74382L, 74434L, 74593L, 80541L, 84055L},
		"67F4A771EAEAEAEA0901798BEEEEEEEEEEEEEEEE003205C7010F65C20CFF74243B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B0000091F79CE09207A02"
		"09327AA10B3291DD0B329F97EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
	{8, 9081114L, 74322L, 5, {31, 32, 50, 50, 50}, {74388L, 74425L, 74596L, 80550L, 84052L},
		"4FA7109AEAEAEAEA09017992EEEEEEEEEEEEEEEE04B005AD0F8A911A0410A5833B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B3B00EEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"091F79D4092079F909327AA40B3291E60B329F94EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
	{12, 579671L, 74332L, 5, {31, 32, 50, 50, 50}, {74385L, 74427L, 74606L, 80535L, 84043L},
		"01010101EDEDEDED55AA0008D857793D00320506EEEEEEEEEEEEEEEEEEEEEEEE0901799CFFFFFFFF000000012020202020202020202020202020202020202020"
		"20202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020"
		"091F79D1092079FB09327AAE0B3291D70B329F8BEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
	{9, 2170603L, 74336L, 6, {31, 32, 50, 50, 50, 50}, {74393L, 74431L, 74599L, 80384L, 80484L, 84048L},
		"C5ED809AEAEAEAEA090179A0EEEEEEEEEEEEEEEE0032064A02211EEBFFFFB7353B3BEEEE00000000000000000000000000000000000000000000000000000000"
		"00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
		"0000000000000000091F79D9092079FF09327AA70B3291400B3291A40B329F90EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
	{8, 8004086L, 74319L, 5, {31, 32, 50, 50, 50}, {74380L, 74423L, 74576L, 80529L, 84074L},
		"3CAFD064EAEAEAEA0901798FEEEEEEEEEEEEEEEE04B005AD0F7A21F60C195C203B3B3B3B3B3B3B3B3B3B3BEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"091F79CC092079F709327A900B3291D10B329FAAEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
	{8, 8004087L, 76936L, 6, {31, 31, 31, 32, 50, 50}, {77040L, 77080L, 77085L, 77136L, 79108L, 84068L},
		"CE9ED064EAEAEAEA0DFF83C8EEEEEEEEEEEEEEEE05AD068A0F7A21F70C1908673B3B3B3B3B3B3B3B3B3B3BEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"0D1F84300D1F84580D1F845D0D2084901D328C440B329FA4EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
		"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"},
};

static int hexPuskuriin(const char *hex, unsigned char *buf, int maxlen)
{
	int n = 0;
	unsigned v;

	while (hex[0] && hex[1] && n < maxlen) {
		sscanf(hex, "%2x", &v);
		buf[n++] = (unsigned char) v;
		hex += 2;
		}
	return n;
}

TEST_CASE("Oikeat kortit: tulkinta sama kuin oikealla lukijalla ennen RC-muutosta")
{
	unsigned char buf[640];
	SIResultTp result;
	size_t d;
	int i, len;

	for (d = 0; d < sizeof(siDumpit) / sizeof(siDumpit[0]); d++) {
		const SIDumpTp *k = &siDumpit[d];
		CAPTURE(k->badge);
		CAPTURE(k->nleima);
		len = hexPuskuriin(k->hex, buf, sizeof(buf));
		tulkSI((char *) buf, &result, 0, k->SItype, len, 0);
		CHECK(result.badge == k->badge);
		CHECK(result.check == k->check);
		CHECK(result.start == TMAALI0);
		CHECK(result.finish == TMAALI0);
		for (i = 0; i < k->nleima; i++) {
			CHECK((int) (unsigned char) result.cc[i+1] == k->cc[i]);
			CHECK(result.ct[i+1] == k->ct[i]);
			}
		CHECK(result.cc[k->nleima+1] == 0);
		CHECK(result.ct[k->nleima+1] == 0L);
		}
}

TEST_CASE("Oikeat kortit: leimalaskuri on kortin leimamaara (SI9/SI8/SI10-11: tavu 22, SI6: 18)")
{
	unsigned char buf[640];
	size_t d;

	for (d = 0; d < sizeof(siDumpit) / sizeof(siDumpit[0]); d++) {
		const SIDumpTp *k = &siDumpit[d];
		CAPTURE(k->badge);
		hexPuskuriin(k->hex, buf, sizeof(buf));
		CHECK((int) buf[k->SItype == 12 ? 18 : 22] == k->nleima);
		}
}

TEST_CASE("EXT: rastikoodi 238 (CN=0xEE) ei katkaise leimalistaa")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	setPunch(buf, 56, 0, 31, 12*3600);
	setPunch(buf, 60, 0, 0xEE, 12*3600+60);   // rasti 238
	setPunch(buf, 64, 0, 50, 12*3600+120);
	buf[22] = 3;
	tulkSI((char *) buf, &result, 0, 7, 256, 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK((int) (unsigned char) result.cc[2] == 238);
	CHECK(result.ct[2] == 12*3600L+60);
	CHECK((int) (unsigned char) result.cc[3] == 50);
	CHECK(result.ct[3] == 12*3600L+120);
}

TEST_CASE("EXT: leimalaskurin jalkeiset vanhat leimat ohitetaan")
{
	// Kortti, jonka leima-aluetta ei ole tyhjennetty: edellisen kayton
	// leimat jaavat RC:n jalkeen.
	unsigned char buf[640];
	SIResultTp result;

	buildBlock(buf, 640, 8004086UL);
	setPunch(buf, 128, 0, 31, 10*3600);
	setPunch(buf, 132, 0, 32, 10*3600+60);
	setPunch(buf, 136, 0, 33, 9*3600);        // vanha leima
	setPunch(buf, 140, 0, 34, 9*3600+60);     // vanha leima
	buf[22] = 2;
	tulkSI((char *) buf, &result, 0, 8, 256, 0);

	CHECK((int) (unsigned char) result.cc[2] == 32);
	CHECK(result.cc[3] == 0);
	CHECK(result.ct[3] == 0L);

	// SI6-EXT: sama leimalaskurilla tavussa 18.
	memset(buf, 0xEE, sizeof(buf));
	setPunch(buf, 128, 0, 31, 10*3600);
	setPunch(buf, 132, 0, 33, 9*3600);        // vanha leima
	buf[18] = 1;
	tulkSI((char *) buf, &result, 0, 12, 384, 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.cc[2] == 0);
}

TEST_CASE("EXT: RC = 0 -> ei leimoja, vaikka leima-alueella on dataa")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 2170603UL);
	setPunch(buf, 136, 0, 31, 10*3600);
	buf[22] = 0;
	tulkSI((char *) buf, &result, 0, 9, 256, 0);

	CHECK(result.cc[1] == 0);
	CHECK(result.ct[1] == 0L);
}

TEST_CASE("EXT: RC yli kortin kapasiteetin rajautuu puskurin loppuun")
{
	unsigned char buf[256];
	SIResultTp result;
	int i;

	buildBlock(buf, 256, 4000000UL);
	for (i = 0; i < 20; i++)
		setPunch(buf, 176 + i*4, 0, (unsigned char) (33 + i), 3600 + i*60);
	buf[22] = 200;                            // pCard: 20 paikkaa
	tulkSI((char *) buf, &result, 0, 10, 256, 0);

	CHECK((int) (unsigned char) result.cc[20] == 33+19);
	CHECK(result.cc[21] == 0);
}

// ===========================================================================
// EXT-protokollan kehykset (lue_SI): siCrc, siKehysTavu, siExtTavu.
// ===========================================================================

TEST_CASE("siCrc: lue_SI:n pyyntokehysten ja oikeiden vastausten CRC:t")
{
	// Pyynnot (CRC:t varmennettu pcap:sta / oikeasta lokista, ks. lue_SI).
	unsigned char ef0[] = {0xEF, 0x01, 0x00};
	unsigned char ef1[] = {0xEF, 0x01, 0x01};
	unsigned char ef4[] = {0xEF, 0x01, 0x04};
	unsigned char ef6[] = {0xEF, 0x01, 0x06};
	unsigned char e10[] = {0xE1, 0x01, 0x00};
	unsigned char e16[] = {0xE1, 0x01, 0x06};
	unsigned char f9[]  = {0xF9, 0x01, 0x01};
	unsigned char b1[]  = {0xB1, 0x00};
	// Aseman lahettama E8 (SI9 1009090 asetettu), LOKI1.LST:
	// 02 E8 06 00 FF 01 0F 65 C2 E4 5A 03
	unsigned char e8[]  = {0xE8, 0x06, 0x00, 0xFF, 0x01, 0x0F, 0x65, 0xC2};

	CHECK(siCrc(ef0, 3) == 0xE209);
	CHECK(siCrc(ef1, 3) == 0xE309);
	CHECK(siCrc(ef4, 3) == 0xE609);
	CHECK(siCrc(ef6, 3) == 0xE409);
	CHECK(siCrc(e10, 3) == 0x460A);
	CHECK(siCrc(e16, 3) == 0x400A);
	CHECK(siCrc(f9, 3)  == 0x170A);
	CHECK(siCrc(b1, 2)  == 0xB100);
	CHECK(siCrc(e8, 8)  == 0xE45A);
}

// SI5 229401:n B1-vastaus (LOKI1.LST "SI data"): len 82, asemakoodi 00 FF,
// 128 tavua dataa ja CRC 33 D5 - sama 133 tavun puskuri, jonka tulkSI saa.
static const char *si5B1Dumppi =
	"8200FFAA2E000172D9020000000000000000006572D9EEEEEEEE0456EEEE2802"
	"4D0007001F79D7207A04327AAB00EEEE00EEEE0000EEEE00EEEE00EEEE00EEEE"
	"00EEEE0000EEEE00EEEE00EEEE00EEEE00EEEE0000EEEE00EEEE00EEEE00EEEE"
	"00EEEE0000EEEE00EEEE00EEEE00EEEE00EEEE0000EEEE00EEEE00EEEE00EEEE"
	"00EEEE33D5";

// Rakentaa EXT-vastauskehyksen: 02 cmd len 00 FF [lohko] data crc crc 03.
// lohko < 0: ei lohkonumeroa. Palauttaa kehyksen pituuden.
static int teeKehys(unsigned char *k, int cmd, int lohko, const unsigned char *data, int dlen)
{
	int n = 0, i;
	unsigned int crc;

	k[n++] = 0x02;
	k[n++] = (unsigned char) cmd;
	k[n++] = (unsigned char) (2 + (lohko >= 0 ? 1 : 0) + dlen);
	k[n++] = 0x00;
	k[n++] = 0xFF;
	if (lohko >= 0)
		k[n++] = (unsigned char) lohko;
	for (i = 0; i < dlen; i++)
		k[n++] = data[i];
	crc = siCrc(k + 1, n - 1);
	k[n++] = (unsigned char) (crc >> 8);
	k[n++] = (unsigned char) crc;
	k[n++] = 0x03;
	return n;
}

static int syotaKehys(SIKehysTp *k, const unsigned char *kehys, int n, int *viimeinen)
{
	int i, t = SIKEHYS_KESKEN;

	*viimeinen = -1;
	for (i = 0; i < n; i++) {
		t = siKehysTavu(k, kehys[i]);
		if (t != SIKEHYS_KESKEN) {
			*viimeinen = i;
			break;
			}
		}
	return t;
}

TEST_CASE("siKehysTavu: oikea SI5-vastaus hyvaksytaan vasta ETX:n kohdalla")
{
	unsigned char dump[140], k[150];
	SIKehysTp kt;
	int n, i, viim;

	CHECK(hexPuskuriin(si5B1Dumppi, dump, sizeof(dump)) == 133);
	// 02 B1 + 133 tavua (len..CRC) + 03
	n = 0;
	k[n++] = 0x02; k[n++] = 0xB1;
	for (i = 0; i < 133; i++)
		k[n++] = dump[i];
	k[n++] = 0x03;

	siKehysAloita(&kt);
	CHECK(syotaKehys(&kt, k, n, &viim) == SIKEHYS_OK);
	CHECK(viim == n - 1);
}

TEST_CASE("siKehysTavu: herate ja jaanteet ennen STX:aa ohitetaan")
{
	unsigned char data[128], k[160], kk[170];
	SIKehysTp kt;
	int n, viim;

	memset(data, 0x31, sizeof(data));
	n = teeKehys(k, 0xEF, 0, data, 128);
	kk[0] = 0xFF; kk[1] = 0x7B; kk[2] = 0x03;   // herate + edellisen sanoman loppu
	memcpy(kk + 3, k, n);
	siKehysAloita(&kt);
	CHECK(syotaKehys(&kt, kk, n + 3, &viim) == SIKEHYS_OK);
	CHECK(viim == n + 2);
}

TEST_CASE("siKehysTavu: vaara CRC, vaara ETX, E7 ja NAK")
{
	unsigned char data[128], k[160];
	SIKehysTp kt;
	int n, viim;
	unsigned char e7[] = {0x00, 0xFF, 0x01, 0x0F, 0x65, 0xC2};
	unsigned char nak = 0x15;

	memset(data, 0x31, sizeof(data));
	n = teeKehys(k, 0xEF, 0, data, 128);
	k[40] ^= 0x01;                               // yksi bitti vaarin
	siKehysAloita(&kt);
	CHECK(syotaKehys(&kt, k, n, &viim) == SIKEHYS_VIRHE);

	n = teeKehys(k, 0xEF, 0, data, 128);
	k[n-1] = 0x02;                               // ETX puuttuu
	siKehysAloita(&kt);
	CHECK(syotaKehys(&kt, k, n, &viim) == SIKEHYS_VIRHE);

	n = teeKehys(k, 0xE7, -1, e7, 6);            // kortti poistettu
	siKehysAloita(&kt);
	CHECK(syotaKehys(&kt, k, n, &viim) == SIKEHYS_POISTO);

	siKehysAloita(&kt);
	CHECK(siKehysTavu(&kt, nak) == SIKEHYS_NAK);
}

TEST_CASE("siExtTavu: SI5-vastauksesta sama 133 tavun puskuri kuin ennen")
{
	unsigned char dump[140], k[150], buf[640];
	SIKehysTp kt;
	int n, i, l = 0, t = SIKEHYS_KESKEN;

	hexPuskuriin(si5B1Dumppi, dump, sizeof(dump));
	n = 0;
	k[n++] = 0x02; k[n++] = 0xB1;
	for (i = 0; i < 133; i++)
		k[n++] = dump[i];
	k[n++] = 0x03;

	siKehysAloita(&kt);
	for (i = 0; i < n; i++)
		t = siExtTavu(&kt, k[i], 0xB1, 0, buf, &l, sizeof(buf));
	CHECK(t == SIKEHYS_OK);
	CHECK(l == 133);
	CHECK(memcmp(buf, dump, 133) == 0);
}

TEST_CASE("siExtTavu: vaara komento tai lohkonumero hylataan")
{
	unsigned char data[128], k[160], buf[640];
	SIKehysTp kt;
	int n, i, l, t = SIKEHYS_KESKEN;

	memset(data, 0x31, sizeof(data));
	n = teeKehys(k, 0xEF, 1, data, 128);        // lohko 1, odotetaan 4
	siKehysAloita(&kt);
	l = 128;
	for (i = 0; i < n; i++)
		t = siExtTavu(&kt, k[i], 0xEF, 4, buf, &l, sizeof(buf));
	CHECK(t == SIKEHYS_VIRHE);
	CHECK(l == 128);                             // mitaan ei lisatty

	n = teeKehys(k, 0xE8, -1, data, 6);          // uusi kortti kesken luennan
	siKehysAloita(&kt);
	l = 0;
	for (i = 0; i < n; i++)
		t = siExtTavu(&kt, k[i], 0xEF, 0, buf, &l, sizeof(buf));
	CHECK(t == SIKEHYS_VIRHE);
	CHECK(l == 0);

	n = teeKehys(k, 0xEF, 5, data, 128);         // ei mahdu puskuriin
	siKehysAloita(&kt);
	l = 600;
	for (i = 0; i < n; i++)
		t = siExtTavu(&kt, k[i], 0xEF, 5, buf, &l, sizeof(buf));
	CHECK(t == SIKEHYS_VIRHE);
	CHECK(l == 600);
}

// Ajaa EXT-luennan kuten lue_SI: aseman vastaukset kehyksina tavu kerrallaan
// siExtTavu:n lapi, siLukuSeuraava paattaa seuraavan lohkon, lopuksi tulkSI.
// kortti = kortin lohkot 0..7 (128 tavua kukin).
static int ajaExtLuku(int SItype, int kmd, const unsigned char *kortti,
	unsigned char *buf, SILukuTp *t)
{
	unsigned char k[160];
	SIKehysTp kt;
	int l = 0, lohko = 0, n, i, tulos;

	siLukuAloita(t, SItype);
	siKehysAloita(&kt);
	for (;;) {
		n = teeKehys(k, kmd, lohko, kortti + 128 * lohko, 128);
		tulos = SIKEHYS_KESKEN;
		for (i = 0; i < n && tulos == SIKEHYS_KESKEN; i++)
			tulos = siExtTavu(&kt, k[i], kmd, lohko, buf, &l, 640);
		if (tulos != SIKEHYS_OK)
			return -1;
		switch (siLukuSeuraava(t, buf, l)) {
			case SIPYY_SI9_B1:  lohko = 1; break;
			case SIPYY_SI11_B4: lohko = 4; break;
			case SIPYY_SI11_B5: lohko = 5; break;
			case SIPYY_SI11_B6: lohko = 6; break;
			case SIPYY_SI11_B7: lohko = 7; break;
			case SIPYY_SI6X_B6: lohko = 6; break;
			case SIPYY_SI6X_B7: lohko = 7; break;
			default:
				return l == t->datalen ? l : -1;
			}
		}
}

TEST_CASE("Oikeat kortit: koko EXT-luenta kehyksineen tuottaa saman tuloksen")
{
	unsigned char dump[640], kortti[1024], buf[640];
	SIResultTp result;
	SILukuTp t;
	size_t d;
	int i, len, l;

	for (d = 0; d < sizeof(siDumpit) / sizeof(siDumpit[0]); d++) {
		const SIDumpTp *k = &siDumpit[d];
		CAPTURE(k->badge);
		len = hexPuskuriin(k->hex, dump, sizeof(dump));
		// Dumpin lohkot kortin lohkoiksi: SI9-perhe 0,1 (SI10/11: 0,4..),
		// SI6-EXT 0,6[,7].
		memset(kortti, 0xEE, sizeof(kortti));
		memcpy(kortti, dump, 128);
		if (k->SItype == 12)
			memcpy(kortti + 6*128, dump + 128, len - 128);
		else if (k->SItype == 8)
			memcpy(kortti + 4*128, dump + 128, len - 128);
		else
			memcpy(kortti + 128, dump + 128, 128);

		l = ajaExtLuku(k->SItype == 12 ? 12 : 7, k->SItype == 12 ? 0xE1 : 0xEF, kortti, buf, &t);
		CHECK(l == len);
		CHECK(t.SItype == k->SItype);
		if (l != len)
			continue;
		CHECK(memcmp(buf, dump, len) == 0);
		tulkSI((char *) buf, &result, 0, t.SItype, t.datalen, 0);
		CHECK(result.badge == k->badge);
		CHECK(result.check == k->check);
		for (i = 0; i < k->nleima; i++) {
			CHECK((int) (unsigned char) result.cc[i+1] == k->cc[i]);
			CHECK(result.ct[i+1] == k->ct[i]);
			}
		CHECK(result.cc[k->nleima+1] == 0);
		}
}

// ===========================================================================
// SI Config+ -kaappaukset: aseman lahettamat kehykset (SI Config+:n loki,
// "IN :::" -rivit) ja SI Config+:n oma tulkinta samasta kortista
// (Record-rivit). Kortin henkilotiedot (nimi, seura, sahkoposti) on korvattu
// tayttomerkeilla ja niiden kehysten CRC laskettu uudelleen.
// Luenta ajetaan kuten lue_SI: tunnistuskehys -> pyydetyt lohkot tavu
// kerrallaan siExtTavu:lle -> siLukuSeuraava -> tulkSI. SI Config+ ei lue
// kaikkia samoja lohkoja (SI10/11: myos asetuslohko 3); puuttuva lohko
// annettaisiin tyhjana (EE).
// ===========================================================================

typedef struct {
	const char *nimi;
	int h, m, s;               // lukuhetki (SI Config+ "Read at")
	long siid, check;          // check = Check, tai Clear jos Checkia ei ole
	int nleima;
	int cc[8];
	long ct[8];
	const char *kehykset[8];   // aseman kehykset, NULL paattaa
} SIConfigKorttiTp;

static const SIConfigKorttiTp siConfigKortit[] = {
	{"SI-Card-SIAC.txt", 9, 54, 11, 8004086L, 76932L, 6,
		{31, 31, 31, 32, 50, 50},
		{77040L, 77074L, 77076L, 77140L, 79098L, 79953L},
		{
			"02E80600FF0F7A21F6A26003",
			"02EF8300FF003CAFD064EAEAEAEA0DFF83C4EEEEEEEEEEEEEEEE05AD068A0F7A21F60C195C203B3B3B3B3B3B3B3B3B3B"
			"3BEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE7CD403",
			"02EA0300FF7E741003",
			"02EF8300FF03EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEE10070384107C96B90DFF83C4190C110107050409006C4D0C000500001800054A0800050A06AA01ECEEEE"
			"EEEEEEEEEEEEAAAACAACCCACCCCACAAAACACCCCCAAAA73696163FFFFFFFF0384007E063100009AAD03",
			"02EF8300FF040D1F84300D1F84520D1F84540D2084941D328C3A1D328F91EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE1CD203",
			"02E70600FF007A21F69E4203",
			NULL}},
	{"SI-Card10-11.txt", 9, 53, 11, 9081114L, 76927L, 3,
		{31, 32, 50},
		{77067L, 77132L, 79027L},
		{
			"02E80600FF0F8A911A0CC203",
			"02EF8300FF004FA7109AEAEAEAEA0DFF83BFEEEEEEEEEEEEEEEE02FE03E30F8A911A0410A5833B3B3B3B3B3B3B3B3B3B"
			"3B3B3B3B3B3B3B3B3B3B3B3B3B00EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEED1F303",
			"02EF8300FF03EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEE0DFF83BF10040F01010202080302000000290000600002110800050A05AA01DFEEEE"
			"EEEEEEEEEEEECCCAACACACCACCCAACCCACACCCCCAAAA73696163FFFFFFFF0744680206B10000600703",
			"02EF8300FF040D1F844B0D20848C1D328BF3EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE543F03",
			"02E70600FF008A911A30E003",
			NULL}},
	{"SI-Card5.txt", 9, 44, 27, 229401L, TMAALI0, 3,
		{31, 32, 50},
		{33839L, 33922L, 35783L},
		{
			"02E50600FF000272D952CA03",
			"02B18200FFAA2E000172D9020000000000000000006572D9EEEEEEEE0456EEEE28024D0007001F842F208482328BC700"
			"EEEE00EEEE0000EEEE00EEEE00EEEE00EEEE00EEEE0000EEEE00EEEE00EEEE00EEEE00EEEE0000EEEE00EEEE00EEEE00"
			"EEEE00EEEE0000EEEE00EEEE00EEEE00EEEE00EEEE0000EEEE00EEEE00EEEE00EEEE00EEEEEDBD03",
			"02E70600FF000272D972C603",
			NULL}},
	{"SI-Card6.txt", 9, 47, 32, 579671L, 76915L, 4,
		{31, 32, 50, 50},
		{77043L, 77125L, 78992L, 80738L},
		{
			"02E60600FF0008D8579D6603",
			"02E18300FF0001010101EDEDEDED55AA0008D857793D00320405EEEEEEEEEEEEEEEEEEEEEEEE0DFF83B3FFFFFFFF0000"
			"000120202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020"
			"2020202020202020202020202020202020202020202020202020202020202020202020202020A73C03",
			"02E18300FF01202020202020202020202020202020202020202020202020202020202020202020202020202020202020"
			"202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020202020"
			"2020202020202020202020202020202020202020202020202020FFFFFFFFFFFFFFFFFFFFFFFFC6DE03",
			"02E18300FF060D1F84330D2084851D328BD01D3292A2EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEED4AD03",
			"02E70600FF0008D8578D6003",
			NULL}},
	{"SI-Card8.txt", 9, 49, 42, 2170603L, 76920L, 3,
		{31, 32, 50},
		{77060L, 77127L, 79020L},
		{
			"02E80600FF02211EEBC0F203",
			"02EF8300FF00C5ED809AEAEAEAEA0DFF83B8EEEEEEEEEEEEEEEE0032039902211EEBFFFFB7353B3BEEEE000000000000"
			"000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
			"00000000000000000000000000000000000000000000000000000000000000000000000000008B4903",
			"02EF8300FF0100000000000000000D1F84440D2084871D328BECEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE17F103",
			"02E70600FF00211EEB98D303",
			NULL}},
	{"SI-Card9.txt", 9, 51, 50, 1009090L, 76924L, 3,
		{31, 32, 50},
		{77064L, 77130L, 79014L},
		{
			"02E80600FF010F65C2E45A03",
			"02EF8300FF0067F4A771EAEAEAEA0DFF83BCEEEEEEEEEEEEEEEE003203AE010F65C20CFF74243B3B3B3B3B3B3B3B3B3B"
			"3B3B3B3B3B3B3B3B3B3B3B3B00000D1F84480D20848A1D328BE6EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE6ACD03",
			"02EF8300FF01EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
			"EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE0FD603",
			"02E70600FF000F65C2807B03",
			NULL}},
};

static int siConfigKehys(const char *hex, unsigned char *k)
{
	return hexPuskuriin(hex, k, 300);
}

TEST_CASE("SI Config+ -kaappaukset: kaikkien aseman kehysten CRC ja ETX kunnossa")
{
	unsigned char k[300];
	SIKehysTp kt;
	size_t c;
	int i, j, n, t;

	for (c = 0; c < sizeof(siConfigKortit) / sizeof(siConfigKortit[0]); c++) {
		CAPTURE(siConfigKortit[c].nimi);
		for (i = 0; siConfigKortit[c].kehykset[i]; i++) {
			CAPTURE(i);
			n = siConfigKehys(siConfigKortit[c].kehykset[i], k);
			siKehysAloita(&kt);
			t = SIKEHYS_KESKEN;
			for (j = 0; j < n && t == SIKEHYS_KESKEN; j++)
				t = siKehysTavu(&kt, k[j]);
			CHECK(j == n);
			CHECK((t == SIKEHYS_OK || (t == SIKEHYS_POISTO && k[1] == 0xE7)));
			}
		}
}

TEST_CASE("SI Config+ -kaappaukset: luenta ja tulkinta samat kuin SI Config+:lla")
{
	unsigned char k[300], buf[640], tyhja[128];
	SIKehysTp kt;
	SILukuTp luku;
	SIResultTp r;
	size_t c;
	int i, j, n, t, kmd, lohko, l, SItype, pyynto;

	memset(tyhja, 0xEE, sizeof(tyhja));
	for (c = 0; c < sizeof(siConfigKortit) / sizeof(siConfigKortit[0]); c++) {
		const SIConfigKorttiTp *e = &siConfigKortit[c];
		CAPTURE(e->nimi);
		// Tunnistuskehys (E5/E6/E8) -> tyyppi ja lukukomento, kuten lue_SI.
		n = siConfigKehys(e->kehykset[0], k);
		REQUIRE(siTunnistaIlmoitus(k, n) != SIILM_EI);
		switch (siTunnistaIlmoitus(k, n)) {
			case SIILM_SI5EXT: SItype = 5;  kmd = 0xB1; break;
			case SIILM_SI6EXT: SItype = 12; kmd = 0xE1; break;
			default:           SItype = 7;  kmd = 0xEF; break;
			}
		siLukuAloita(&luku, SItype);
		siKehysAloita(&kt);
		l = 0;
		lohko = 0;
		for (;;) {
			n = 0;
			for (i = 1; e->kehykset[i]; i++) {
				n = siConfigKehys(e->kehykset[i], k);
				if (k[1] == kmd && (kmd == 0xB1 || k[5] == lohko))
					break;
				n = 0;
				}
			if (!n)
				n = teeKehys(k, kmd, lohko, tyhja, 128);  // tyhja lohko
			t = SIKEHYS_KESKEN;
			for (j = 0; j < n && t == SIKEHYS_KESKEN; j++)
				t = siExtTavu(&kt, k[j], kmd, lohko, buf, &l, sizeof(buf));
			REQUIRE(t == SIKEHYS_OK);
			pyynto = siLukuSeuraava(&luku, buf, l);
			if (pyynto == SIPYY_EI)
				break;
			lohko = pyynto == SIPYY_SI9_B1 ? 1 :
				pyynto == SIPYY_SI11_B4 ? 4 : pyynto == SIPYY_SI11_B5 ? 5 :
				pyynto == SIPYY_SI11_B6 || pyynto == SIPYY_SI6X_B6 ? 6 : 7;
			}
		REQUIRE(l == luku.datalen);
		tulkSI((char *) buf, &r, siTics(e->h, e->m, e->s), luku.SItype, luku.datalen, 0);

		CHECK(r.badge == e->siid);
		CHECK(r.check == e->check);
		CHECK(r.start == TMAALI0);
		CHECK(r.finish == TMAALI0);
		for (i = 0; i < e->nleima; i++) {
			CAPTURE(i);
			CHECK((int) (unsigned char) r.cc[i+1] == e->cc[i]);
			CHECK(r.ct[i+1] == e->ct[i]);
			}
		CHECK(r.cc[e->nleima+1] == 0);
		CHECK(r.ct[e->nleima+1] == 0L);
		}
}

