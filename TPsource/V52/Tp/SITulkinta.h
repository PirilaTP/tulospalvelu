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

// SportIdent-kortin (SI5/SI6/SI9/SI8/pCard/tCard/SI10/SI11) tavupuskurin
// tulkinta badge-, aika- ja rastileimatiedoiksi. Tama kaannosyksikko on
// tarkoituksella riippumaton globaaleista muuttujista, tietokannasta ja
// Windowsista/VCL:sta, jotta se voidaan kaantaa yksikkotesteihin
// (ks. Tests/TulkSITest.cpp). Kutsuja Tp/TpLaitteet.cpp:n lue_SI():ssa
// poimii t0:n globaalista ja kopioi tuloksen san_type-tyyppiin (ks. sielta
// tulkSI-kutsun ymparilla oleva sovituskoodi).

#ifndef SITULKINTA_DEFINED
#define SITULKINTA_DEFINED

#include <stddef.h>
#include <tptype.h>

// Sama kenttajoukko kuin san_type-unionin r21data (ks. HkDef.h/VDef.h,
// #ifdef SPORTIDENT), mutta itsenaisena struktina, jotta tama tiedosto ei
// tarvitse HkDef.h:ta/VDef.h:ta (jotka vetaisivat mukaan windows.h:n).
//
// Asettelu on lukittu pack(4):lla: tputil.h asettaa klassisella Borland-
// kaantajalla (bcc32) "#pragma option -a1" (1 tavun tasaus) kaikille sen
// jalkeen maaritellyille rakenteille. TpLaitteet.cpp (kutsuja) sisallyttaa
// tputil.h:n, SITulkinta.cpp ei - ilman lukitusta ct[] oli kutsujalla
// tavussa 86 ja tulkSI:lla tavussa 88, jolloin kaikki leima-ajat luettiin
// 2 tavua vinossa (SI5: ajat 0, SI6+: roska-ajat; badge ja koodit oikein).
#pragma pack(push, 4)
typedef struct {
	INT32 badge;
	INT32 lukija;
	INT32 start;
	INT32 check;
	INT32 finish;
	char cc[66];
	INT32 ct[66];
} SIResultTp;
#pragma pack(pop)

// Kaannosaikainen tarkistus: jokainen tata otsikkoa kayttava kaannosyksikko
// nakee saman asettelun (taulukon koko -1 = kaannosvirhe).
typedef char SIResultTp_asettelu_ct[offsetof(SIResultTp, ct) == 88 ? 1 : -1];
typedef char SIResultTp_asettelu_koko[sizeof(SIResultTp) == 352 ? 1 : -1];

// Tulkitsee yhden SportIdent-kortin tavupuskurin buf (kaytetty pituus buflen)
// tyyppia SItype (5=SI5, 6=SI6, 7=SI9, 8=SI10/11, 9=SI8, 10=pCard, 11=tCard)
// ja tayttaa result-rakenteen. SIt on lukuhetken BIOS-kelloaika (biostime()),
// t0 kilpailun nollahetki tunteina; molempia tarvitaan lukija-kentan
// laskentaan (ks. t_time_l). Palauttaa aina 0 (ei viela kaytossa olevaa
// virhesignalointia).
int tulkSI(char *buf, SIResultTp *result, INT32 SIt, int SItype, int buflen, int t0);

// ---------------------------------------------------------------------------
// Kortin numeron uudelleenkayton askel (BADGEASKEL, ks. BadgeAskel()
// TpLaitteet.cpp:ssa): luetun kortin numero siirretaan kilpailutiedoissa
// muotoon numero + n * askel, jotta sama kortti voidaan kayttaa uudelleen.
// Emit-numerot ovat alle 1 000 000, joten Emitilla askel on 1 000 000.
// SportIdent-numerot ulottuvat lahes 10 miljoonaan (SI9 1 M, SI8 2 M,
// pCard 4 M, tCard 6 M, SI10 7 M, SIAC 8 M, SI11 9 M), joten 1 000 000:n
// askeleella siirretty numero voisi olla toisen oikean kortin numero
// (esim. SI5 229401 -> 1229401 = SI9-kortti). SportIdent-kilpailussa
// (badgelaji 'I') askel on siksi 10 000 000.
long siBadgeAskel(wchar_t badgelaji);

// Listalle tulostettava kortin numero, kun kentan leveys on len merkkia
// (0 = ei rajaa). putfld katkaisee liian pitkan tekstin lopusta, jolloin
// siirretty koodi (esim. 10229401 7 merkin kenttaan -> "1022940") nayttaisi
// toisen kortin numerolta. Jos numero ei mahdu, palautetaan kortin oma
// numero ilman siirtoja (badge % askel); muuten numero sellaisenaan.
long siNaytettavaBadge(long badge, long askel, int len);

// Montako kertaa kortin numeroa badge voi siirtaa askeleella askel niin,
// etta badge + n * askel pysyy alle 2 000 000 000:n. VIx.cpp ja HkIx.cpp
// merkitsevat indeksin toistot arvolla 2 000 000 000, ja INT32 ylivuotaa
// 2 147 483 647:n jalkeen. SportIdent-askeleella (10 000 000) raja tulee
// vastaan vain hyvin monen osuuden viestissa (MAXOSUUSLUKU=250): kortin
// 9 999 999 koodia voi siirtaa 199 kertaa.
long siMaxSiirrot(long badge, long askel);

// Emit-kortti, joka on menettanyt koodinsa, nayttaa koodia 200 ("viallinen
// kortti"). SportIdent-kilpailussa (badgelaji 'I') 200 on oikean SI5-kortin
// numero, joten sita ei kasitella viallisena.
bool siViallinenEmit200(long badge, wchar_t badgelaji);

// ---------------------------------------------------------------------------
// SI-aseman lukusekvenssi (Tp/TpLaitteet.cpp:n lue_SI). Puhtaat paatokset
// erotettu tanne yksikkotesteja varten; lue_SI hoitaa sarjaportin.

// Aseman ilmoitus lue_SI:n puskurin alussa (STX-tahdistuksen jalkeen,
// n = luettujen tavujen maara). Paluuarvo:
//   SIILM_ODOTA   liian vahan tavuja viela (lue_SI kasvattaa od-laskuria)
//   SIILM_EI      ei kasiteltava sanoma - hylataan
//   SIILM_SI5     vanha SI5-ilmoitus 02 46 49 03 -> pyynto SI5pyynto
//   SIILM_SI5EXT  EXT E5 (SI5)           -> pyynto B1
//   SIILM_SI9     EXT E8 (SI8/9/10/11, pCard, tCard) -> lohko 0
//   SIILM_SI6EXT  EXT E6 (SI6)           -> lohko 0
//   SIILM_SI6     vanha SI6-ilmoitus (tunnus 102, ETX tavussa 8)
//   SIILM_SI5AUTO vanhan protokollan SI5 auto-send (02 31 <data>)
//   SIILM_D3      online-rastiaseman leimaussanoma (02 D3 ...)
#define SIILM_ODOTA   0
#define SIILM_EI      1
#define SIILM_SI5     2
#define SIILM_SI5EXT  3
#define SIILM_SI9     4
#define SIILM_SI6EXT  5
#define SIILM_SI6     6
#define SIILM_SI5AUTO 7
#define SIILM_D3      8
int siTunnistaIlmoitus(const unsigned char *b, int n);

// Kortin lukutila: SItype (5..12, SI9-perheella tarkentuu lohkon 0
// jalkeen), luetut lisalohkot, SI10/11:n tarvitsemat leimalohkot ja
// kerattava kokonaispituus (tavua SIbuf:iin ennen tulkSI:ta).
typedef struct {
	int SItype;
	int nblock;
	int nblocks_needed;
	int datalen;
} SILukuTp;

// Seuraava lohkopyynto (siLukuSeuraava): SIPYY_EI = ei pyyntoa.
#define SIPYY_EI       0
#define SIPYY_SI9_B1   1   // EF lohko 1 (SI9/SI8/pCard/tCard)
#define SIPYY_SI11_B4  2   // EF lohko 4 (SI10/11 ensimmainen leimalohko)
#define SIPYY_SI11_B5  3
#define SIPYY_SI11_B6  4
#define SIPYY_SI11_B7  5
#define SIPYY_SI6X_B6  7   // E1 lohko 6 (SI6-EXT, leimat 1-32)
#define SIPYY_SI6X_B7  8   // E1 lohko 7 (SI6-EXT, leimat 33-64)
#define SIPYY_TUNTEMATON 9 // SIID ei minkaan tunnetun korttisarjan alueella

// Aloittaa kortin luvun: datalen tyypin mukaan (SI5 133, SI6 402, SI9-
// perhe 256, SI6-EXT 384; SI10/11 ja SI6-EXT tarkentuvat lohkon 0 jalkeen).
void siLukuAloita(SILukuTp *t, int SItype);

// Kutsutaan, kun SIbuf:iin on kertynyt l tavua. Kun lohko on taynna,
// paattaa seuraavan pyynnon:
//  - SI9-perhe: korttisarja SIID:sta (lohko 0, tavut 25..27; SportIdentin
//    numeroalueet, ks. SITulkinta.cpp). Tuntematon sarja -> SIPYY_TUNTEMATON.
//    SI10/11/SIAC: leimamaara (tavu 22) -> tarvittavat leimalohkot 4..7.
//  - SI6-EXT: lohko 6, ja lohko 7 vain kun leimoja (tavu 18) on yli 32.
//    Henkilotietolohkoa 1 ei lueta. SIbuf = lohko 0 + lohko 6 [+ lohko 7].
// Palauttaa SIPYY_*-arvon.
int siLukuSeuraava(SILukuTp *t, const unsigned char *buf, int l);

// Lohkopyynnon yritykset (SI Config+:n tapaan): virheellinen kehys, NAK tai
// aikaraja -> sama pyynto uudelleen, kunnes SIYRITYKSET yritysta on tehty.
// Kortin poisto (SIKEHYS_POISTO) keskeyttaa heti.
#define SIYRITYKSET 3

// Pyydetaanko lohko uudelleen: tulos = siExtTavu:n virhe (SIKEHYS_VIRHE,
// SIKEHYS_NAK tai SIKEHYS_POISTO) tai SIKEHYS_KESKEN aikarajan tullessa,
// yritys = tahan asti tehtyjen pyyntojen maara (ensimmainen = 1).
int siUusitaanko(int tulos, int yritys);

// Vanhan protokollan (SIext=0) koottu vastaus: STX ja ETX odotetuilla
// paikoilla (SI5: 133 tavua 02 .. 03; SI6: kolme 134 tavun kehysta
// 02 .. 03). Muuten data on siirtynyt, eika sita tulkita. Tarkistussummaa
// (CS) ei tarkisteta: vanhan protokollan summan laskutapaa ei ole
// dokumentoitu.
int siVanhaKehysOk(const unsigned char *b, int len, int SItype);

// ---------------------------------------------------------------------------
// EXT-protokollan vastauskehykset (lue_SI, SIext=1):
//   02 <cmd> <len> <asema_H> <asema_L> <data[len-2]> <crc_H> <crc_L> 03
// EF/E1-vastauksen datan 1. tavu on lohkonumero, jota seuraa 128 tavua.

// SportIdentin CRC (polynomi 0x8005, 16 bitin sanoina) tavuista p[0..n);
// kehyksessa lasketaan cmd:sta datan loppuun (ei STX:aa, CRC:ta eika ETX:aa).
unsigned int siCrc(const unsigned char *p, int n);

typedef struct {
	int n;                  // keratyt kehyksen tavut
	unsigned char k[140];   // kehys STX:sta ETX:aan (EF-lohko 137 tavua)
} SIKehysTp;

// siKehysTavu/siExtTavu-paluuarvot.
#define SIKEHYS_KESKEN   0   // kehys kesken
#define SIKEHYS_OK       1   // kehys valmis ja kunnossa
#define SIKEHYS_VIRHE   -1   // ETX, CRC, pituus, komento tai lohkonumero vaarin
#define SIKEHYS_POISTO  -2   // kortti poistettiin kesken luennan (E7)
#define SIKEHYS_NAK     -3   // asema hylkasi pyynnon (NAK)

void siKehysAloita(SIKehysTp *k);

// Lisaa tavun c kehykseen. STX:aa edeltavat tavut (herate FF, edellisen
// sanoman jaanteet) ohitetaan; NAK ennen kehysta = SIKEHYS_NAK. Valmiista
// kehyksesta tarkistetaan pituus, ETX ja CRC; E7 = SIKEHYS_POISTO.
int siKehysTavu(SIKehysTp *k, unsigned char c);

// Lue_SI:n EXT-tavu: keraa kehyksen (siKehysTavu) ja kun se on valmis,
// tarkistaa komennon kmd (B1, EF tai E1) ja EF/E1:lla lohkonumeron lohko,
// ja lisaa datan buf:iin kohtaan *l (enintaan maxl tavua), *l kasvaa:
//   B1 (SI5): len, asemakoodi, 128 tavua dataa ja CRC = 133 tavua, sama
//             asettelu kuin vanhan protokollan SI5tp (data tavusta 3)
//   EF/E1:    lohkon 128 tavua (lohkonumero ohitetaan)
// Kehys alustetaan uutta varten, kun se on kasitelty.
int siExtTavu(SIKehysTp *k, unsigned char c, int kmd, int lohko,
	unsigned char *buf, int *l, int maxl);

// SI5 auto-send (SIILM_SI5AUTO): rakentaa SIbuf:n alun jo luetuista
// tavuista pre[0..prelen) (02 31 <data...>): kolmen tavun SI5tp-otsikko
// (02 02 31) ja data pre[2]:sta DLE-koodaus purettuna. *dle kantaa
// DLE-tilan lukusilmukkaan. Palauttaa buf:iin kirjoitettujen tavujen maaran.
int siAutosendAlku(const unsigned char *pre, int prelen, unsigned char *buf, int *dle);

// ---------------------------------------------------------------------------
// tulkSI:n tulos emittp:n leimoiksi (HkIV.cpp/VIv.cpp:n tall_emit).

// Tayttaa ctrlcode[0..maxn)/ctrltime[0..maxn) (maxn = MAXNLEIMA) kortin
// leimoista: ajat suhteessa nollahetkeen (lahtoleima; sen puuttuessa
// nollaus-/tarkastusleima, jos se on enintaan 12 h ennen ensimmaista
// leimaa; muuten ensimmainen leima). Kaksi viimeista paikkaa jaa maali-
// (240) ja lukijariville (250), jotka lisataan leimojen peraan. r->start/
// check/finish arvo 61166 (0xEEEE) tai TMAALI0 = ei aikaa. t0 = kilpailun
// nollahetki tunteina (r->lukija on t_time_l-asteikolla).
// *lukuaika = lukuhetki - nollahetki sekunteina (12 h -varoitusta varten),
// tai -1 jos lukijarivia ei lisatty tai nollahetkea ei ole.
void siEmitLeimat(const SIResultTp *r, int t0, unsigned char *ctrlcode,
	UINT16 *ctrltime, int maxn, long *lukuaika);

// ---------------------------------------------------------------------------
// Toistuvat leimat (HkEmit.cpp/VEmit.cpp:n tarkista ja e_maaliaika).
// ctrlcode on emittp:n rengaspuskuri (n = MAXNLEIMA paikkaa).

// tarkista: j = radan rastiin sovitettu leima. Kulkee taaksepain saman
// koodin aiemmat perakkaiset leimat (ei lukijan lukija eika lukija+1
// ohi) ja palauttaa niista ensimmaisen indeksin. Ohitetut (myohemmat)
// indeksit kirjoitetaan ohitetut[]:iin, maara *nohit, jotta kutsuja voi
// merkita ne ylimaaraisiksi.
int siToistoAlkuun(const unsigned char *ctrlcode, int n, int j, int lukija,
	int *ohitetut, int *nohit);

// e_maaliaika: l = maalileiman sijainti lk:sta. Palauttaa perakkaisista
// saman koodin maalileimoista ensimmaisen sijainnin (ei alle 1:n).
int siMaaliToistoAlkuun(const unsigned char *ctrlcode, int n, int lk, int l);

#endif
