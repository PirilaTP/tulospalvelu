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
// pblk[0] -> cc[1..32], pblk[1] -> cc[33..64], CN=0xEE paattaa kummankin
// lohkon listan erikseen.
TEST_CASE("SI6: pblk[0] ja pblk[1] jatkuvat perakkain, ei ylikirjoita")
{
	SI6tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.pblk[0].punch[0].CN = 31;
	tp.pblk[0].punch[0].PT[0] = 0; tp.pblk[0].punch[0].PT[1] = 10;
	tp.pblk[0].punch[1].CN = (char) 0xEE;   // pblk[0]:ssa vain 1 leima
	tp.pblk[1].punch[0].CN = 32;
	tp.pblk[1].punch[0].PT[0] = 0; tp.pblk[1].punch[0].PT[1] = 20;
	tp.pblk[1].punch[1].CN = (char) 0xEE;   // pblk[1]:ssa vain 1 leima
	tulkSI((char *) &tp, &result, 0, 6, sizeof(tp), 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 10L);
	CHECK((int) (unsigned char) result.cc[2] == 32);
	CHECK(result.ct[2] == 20L);
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
//   [20:24)   EE EE EE EE               -- ei kaytossa
//   [25:28)   SIID (3 tavua, big-endian)
//
// HUOM: SITulkinta.cpp:n case 7:n oma kommentti kuvaa tama jarjestyksen
// vielä vaarin (vaihtaa lahdon ja "ei kaytossa" -alueen keskenaan) - se on
// vain kommentti, ei vaikuta koodin ajokayttaytymiseen, jota tama testi
// seuraa.
static void buildBlock(unsigned char *b, int len, unsigned long siid)
{
	memset(b, 0xEE, len);  // CN=EE kaikkialla = "ei leimaa" oletusarvona
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

TEST_CASE("SI9: CN=EE lahtotavussa tarkoittaa ettei lahtoa ole, arvo TMAALI0")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	// setPunch ei tassa kutsuta lahtotavuille -> CN pysyy 0xEE:na
	tulkSI((char *) buf, &result, 0, 7, 256, 0);
	CHECK(result.start == TMAALI0);
}

TEST_CASE("SI9: CN=EE check- ja maalitavussa tarkoittaa ettei arvoa ole, arvo TMAALI0")
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

TEST_CASE("SI9: valiaikaleimat luetaan tavusta 56 alkaen, CN=EE paattaa listan")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	setPunch(buf, 56, 0, 31, 12*3600);       // 1. rasti: koodi 31, klo 12:00:00
	setPunch(buf, 60, 0, 32, 12*3600+30);    // 2. rasti: koodi 32, klo 12:00:30
	// buf[64] jaa 0xEE:ksi -> lista paattyy tahan
	tulkSI((char *) buf, &result, 0, 7, 256, 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 12*3600L);
	CHECK((int) (unsigned char) result.cc[2] == 32);
	CHECK(result.ct[2] == 12*3600L+30);
}

TEST_CASE("SI9: rastiajan kaannos +12h kun aika on pienempi kuin edellinen")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	setPunch(buf, 12, 0, 200, 23*3600);      // lahto klo 23:00:00 (edellisena paivana)
	setPunch(buf, 56, 0, 31, 1*3600);        // 1. rasti klo 01:00:00 -> pitaa kaantaa +12h
	tulkSI((char *) buf, &result, 0, 7, 256, 0);

	CHECK(result.ct[1] == 1*3600L + 43200L);
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
	tulkSI((char *) buf, &result, 0, 8, 256, 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 12*3600L);
}

TEST_CASE("SI10/11: leimat jatkuvat lisalohkoissa buflen:iin asti (640 tavua, 4 lohkoa)")
{
	unsigned char buf[640];
	SIResultTp result;

	int i;

	// Silmukka skannaa jokaisen 4 tavun paikan i=128,132,136,... jatkuvasti
	// ja pysahtyy ensimmaiseen CN=EE:hen, joten leimat pitaa tayttaa
	// KESKEYTYKSETTA - ei riita asettaa yhta per 128-tavuinen lohko valiin
	// jaavine 0xEE-aukkoineen (silmukka pysahtyisi heti ensimmaisen leiman
	// jalkeiseen aukkoon).
	buildBlock(buf, 640, 7000000UL);
	for (i = 128; i + 3 < 512; i += 4)
		setPunch(buf, i, 0, (unsigned char) (40 + ((i-128)/4) % 100), 10*3600 + (i-128)/4);
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

	buildBlock(buf, 640, 7000000UL);
	setPunch(buf, 128, 0, 31, 10*3600);
	setPunch(buf, 256, 0, 32, 11*3600);  // tama on buflen=256:n ulkopuolella
	tulkSI((char *) buf, &result, 0, 8, 256, 0);  // buflen=256!

	CHECK((int) (unsigned char) result.cc[1] == 31);
	// result on tulkSI:n omaa muistia (nollattu memset(result,0,...):lla alussa),
	// ei syotepuskuria, joten lukematon paikka on 0 - ei syotepuskurin 0xEE-tayte.
	CHECK((int) (unsigned char) result.cc[2] == 0);
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
	tulkSI((char *) buf, &result, 0, 11, 256, 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK((int) (unsigned char) result.cc[2] == 32);
	CHECK(result.ct[2] == 11*3600L);
}

// ===========================================================================
// SI6 EXT-protokollan kautta (SItype == 12): eri langansiirtokoodaus samalle
// korttisukupolvelle kuin legacy SI6 (SItype 6) - EI sama tavuasettelu kuin
// SI9+:lla (SItype 7-11). Badge tavuilla [10:14), leimat alkaen tavusta 256
// (lohkot 6 ja 7), 32 leimaa/lohko, kuten legacy SI6:n kaksi SI6PBLK-lohkoa.
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
	tulkSI((char *) buf, &result, 0, 12, 512, 0);

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
	tulkSI((char *) buf, &result, 0, 12, 512, 0);

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
	tulkSI((char *) buf, &result, 0, 12, 512, 0);
	CHECK(result.check == 0x83B3L + 43200L);   // 21:21:55

	// Kun tarkastusleima on, se voittaa nollausleiman.
	setPunch(buf, 28, 0x0D, 0x03, 0x0E46);
	tulkSI((char *) buf, &result, 0, 12, 512, 0);
	CHECK(result.check == 46854L);
}

TEST_CASE("SI6-EXT: leimat alkavat tavusta 256 (lohko 6), ei 128:sta tai 56:sta")
{
	unsigned char buf[512];
	SIResultTp result;

	memset(buf, 0xEE, sizeof(buf));
	buf[10] = 0; buf[11] = 0; buf[12] = 0; buf[13] = 1;
	setPunch(buf, 56,  0, 99, 1*3600);   // SI9:n paikka - EI saa nakya
	setPunch(buf, 128, 0, 98, 2*3600);   // SI10/11:n paikka - EI saa nakya
	setPunch(buf, 256, 0, 31, 12*3600);  // oikea 1. rasti SI6-EXT:lla (lohko 6)
	tulkSI((char *) buf, &result, 0, 12, 512, 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 12*3600L);
}

TEST_CASE("SI6-EXT: leimat jatkuvat lohkoon 7 (tavu 384) asti, CN=EE paattaa")
{
	// Todellinen kortti: 8 leimaa lohkossa 6, loput 0xEE. Tama testi kattaa
	// lisaksi jatkumisen lohkoon 7, jota ei ollut tallessa oikeassa dumpissa.
	unsigned char buf[512];
	SIResultTp result;
	int i;

	memset(buf, 0xEE, sizeof(buf));
	buf[10] = 0; buf[11] = 0; buf[12] = 0; buf[13] = 1;
	// tayta lohko 6 kokonaan (32 leimaa, tavut 256..383) ja jatka lohkoon 7:aan
	for (i = 256; i + 3 < 384+16; i += 4)
		setPunch(buf, i, 0, (unsigned char) (40 + (i-256)/4), 10*3600 + (i-256)/4);
	tulkSI((char *) buf, &result, 0, 12, 512, 0);

	// r=32 -> i=256+4*31=380 (lohko 6:n viimeinen)
	CHECK((int) (unsigned char) result.cc[32] == 40+31);
	// r=33 -> i=384 (lohko 7:n ensimmainen)
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
	for (i = 256; i + 3 < 512; i += 4)
		setPunch(buf, i, 0, (unsigned char) (33 + ((i-256)/4) % 200), 3600 + (i-256)/4*60);
	tulkSI((char *) buf, &result, 0, 12, 512, 0);

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

TEST_CASE("siLukuAloita: kerattava pituus ja ohitettava otsikko tyypeittain")
{
	SILukuTp t;
	int skip;

	siLukuAloita(&t, 5, 0, &skip);  CHECK(t.datalen == 133); CHECK(skip == 0);
	siLukuAloita(&t, 5, 1, &skip);  CHECK(t.datalen == 133); CHECK(skip == 2);
	siLukuAloita(&t, 6, 0, &skip);  CHECK(t.datalen == 402); CHECK(skip == 0);
	siLukuAloita(&t, 7, 1, &skip);  CHECK(t.datalen == 256); CHECK(skip == 6);
	siLukuAloita(&t, 12, 1, &skip); CHECK(t.datalen == 512); CHECK(skip == 6);
	CHECK(t.nblock == 0);
	CHECK(t.nblocks_needed == 1);
}

// Ajaa lukusekvenssin kuten lue_SI: l kasvaa tavu kerrallaan kunnes
// datalen tayttyy, ja kerataan lahetetyt pyynnot. buf = kortin data.
static int ajaLuku(int SItype, int SIext, const unsigned char *buf, int *pyynnot,
	SILukuTp *t)
{
	int l, n = 0, skip;

	siLukuAloita(t, SItype, SIext, &skip);
	for (l = 1; l <= 640; l++) {
		int p = siLukuSeuraava(t, buf, l, &skip);
		if (p != SIPYY_EI) {
			CHECK(skip == 9);
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

TEST_CASE("siLukuSeuraava: SI6-EXT lukee lohkot 0, 1, 6 ja 7")
{
	unsigned char b[640];
	int p[8], n;
	SILukuTp t;

	memset(b, 0, sizeof(b));
	n = ajaLuku(12, 1, b, p, &t);
	REQUIRE(n == 3);
	CHECK(p[0] == SIPYY_SI6X_B1); CHECK(p[1] == SIPYY_SI6X_B6); CHECK(p[2] == SIPYY_SI6X_B7);
	CHECK(t.datalen == 512);
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
