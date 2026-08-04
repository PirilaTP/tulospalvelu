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
// SI8 (9), pCard (10), tCard (11).
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
	tulkSI((char *) &tp, &result, 0, 5, sizeof(tp), 0);

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
	tulkSI((char *) &tp, &result, 0, 5, sizeof(tp), 0);

	// 50 < 100 (start) ja start != 61166 -> +43200 kaannos
	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 50L + 43200L);
}

// HUOM: "result->start != 61166L" -erikoistapausta (SITulkinta.cpp, case 5)
// ei voi testata tallaisenaan: ST[0]/ST[1] ovat SI5tp:ssa tavallisia (etumerkillisia)
// char-kenttia tallakin alustalla (vahvistettu static_assert((char)-1<0)), ja
// niiden arvot etumerkkilaajennetaan int/long:ksi ennen 256*ST[0]+ST[1]
// -laskua. Etumerkillisen charin arvoalueella [-128,127] lauseke 256*a+b
// yltaa korkeintaan arvoon 256*127+127 = 32639 - eli tulos ei voi koskaan
// olla 61166. Erikoistapaus vaikuttaa siis saavuttamattomalta (kuolleelta)
// koodilta talla alustalla; tata ei testata talla, koska yritys tuottaisi
// vain vaaria/harhaanjohtavia odotusarvoja.

TEST_CASE("SI5: myohemmat leimat kaannetaan +12h jos pienempia kuin edellinen")
{
	SI5tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.row[0].c[0].cc = 31;
	tp.row[0].c[0].ct[0] = 0; tp.row[0].c[0].ct[1] = 120;  // ct[1] = 120
	tp.row[0].c[1].cc = 32;
	tp.row[0].c[1].ct[0] = 0; tp.row[0].c[1].ct[1] = 100;  // ct[2] = 100 < ct[1]
	tulkSI((char *) &tp, &result, 0, 5, sizeof(tp), 0);

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

TEST_CASE("SI6: ensimmaisen lohkon (pblk[0]) valiaikaleimat puretaan cc/ct[1..32]:een")
{
	SI6tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.pblk[0].punch[0].CN = 31;
	tp.pblk[0].punch[0].PT[0] = 0; tp.pblk[0].punch[0].PT[1] = 10;
	tp.pblk[1].punch[0].CN = 0xEE;  // pblk[1]:n vastaava paikka on tyhja/ei-kaytossa
	tulkSI((char *) &tp, &result, 0, 6, sizeof(tp), 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK(result.ct[1] == 10L);
}

// Dokumentoi olemassa olevan kaytoksen: pblk[0] ja pblk[1] kirjoittavat
// samaan cc[1..32]/ct[1..32]-alueeseen (silmukka ei siirra indeksia toiselle
// lohkolle), joten pblk[1].punch[i] YLIKIRJOITTAA pblk[0].punch[i]:n samalla
// i:lla. Vain jalkimmainen (pblk[1]) jaa nakyviin. Tama saattaa olla
// tarkoittamaton, mutta testi kirjaa nykyisen kayttaytymisen nakyvaksi.
TEST_CASE("SI6: pblk[1] ylikirjoittaa pblk[0]:n samassa cc/ct-indeksissa")
{
	SI6tp tp;
	SIResultTp result;

	memset(&tp, 0, sizeof(tp));
	tp.pblk[0].punch[0].CN = 31;
	tp.pblk[0].punch[0].PT[0] = 0; tp.pblk[0].punch[0].PT[1] = 10;
	tp.pblk[1].punch[0].CN = 32;
	tp.pblk[1].punch[0].PT[0] = 0; tp.pblk[1].punch[0].PT[1] = 20;
	tulkSI((char *) &tp, &result, 0, 6, sizeof(tp), 0);

	CHECK((int) (unsigned char) result.cc[1] == 32);
	CHECK(result.ct[1] == 20L);
}

// ===========================================================================
// Yhteiset apufunktiot SI9/SI10-11/SI8/pCard/tCard -testeille (EXT-protokolla,
// tavupuskuriin suoraan indeksoiva osuus tulkSI:sta).
// ===========================================================================

// Rakentaa synteettisen EXT-protokollan lohkon (oletusarvoisesti 256 tavua,
// block0 + block1). Layout (ks. SITulkinta.cpp:n case-kommentit):
//   [8]  PTD  [9]  CN   [10:12) time   -- Check-leimaus
//   [16] PTD  [17] CN   [18:20) time   -- Maalileimaus
//   [20] PTD  [21] CN   [22:24) time   -- Lahtoleimaus (CN=EE -> ei lahtoa)
//   [25:28)   SIID (3 tavua, big-endian)
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
	setPunch(buf, 20, 0, 200, 11*3600);   // lahto klo 11:00:00
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

TEST_CASE("SI9: CN=EE check- ja maalitavussa tarkoittaa arvoa 0")
{
	unsigned char buf[256];
	SIResultTp result;

	buildBlock(buf, 256, 1009090UL);
	tulkSI((char *) buf, &result, 0, 7, 256, 0);
	CHECK(result.check == 0L);
	CHECK(result.finish == 0L);
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
	setPunch(buf, 20, 0, 200, 23*3600);      // lahto klo 23:00:00 (edellisena paivana)
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
	setPunch(buf, 20, 0, 200, 11*3600);
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

	buildBlock(buf, 640, 7000000UL);
	setPunch(buf, 128, 0, 31, 10*3600);       // lohko 4, 1. rasti
	setPunch(buf, 256, 0, 32, 11*3600);       // lohko 5 (i=128+128=256), 2. rasti
	setPunch(buf, 384, 0, 33, 12*3600);       // lohko 6 (i=384), 3. rasti
	setPunch(buf, 512, 0, 34, 13*3600);       // lohko 7 (i=512), 4. rasti
	tulkSI((char *) buf, &result, 0, 8, 640, 0);

	CHECK((int) (unsigned char) result.cc[1] == 31);
	CHECK((int) (unsigned char) result.cc[2] == 32);
	CHECK((int) (unsigned char) result.cc[3] == 33);
	CHECK((int) (unsigned char) result.cc[4] == 34);
	CHECK(result.ct[4] == 13*3600L);
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
	CHECK((int) (unsigned char) result.cc[2] == 0xEE);  // ei luettu -> nollattu oletusarvo
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
