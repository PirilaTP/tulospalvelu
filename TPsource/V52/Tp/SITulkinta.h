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
#define SIPYY_SI6X_B1  6   // E1 lohko 1 (SI6-EXT)
#define SIPYY_SI6X_B6  7
#define SIPYY_SI6X_B7  8

// Aloittaa kortin luvun: datalen tyypin mukaan (SI5 133, SI6 402, SI9-
// perhe 256, SI6-EXT 512) ja *skip = vastauksen alusta ohitettavat
// otsikkotavut (SI9/SI6-EXT 6, muu EXT 2, vanha protokolla 0).
void siLukuAloita(SILukuTp *t, int SItype, int SIext, int *skip);

// Kutsutaan, kun SIbuf:iin on kertynyt l tavua. Kun lohko on taynna,
// paattaa seuraavan pyynnon: SI9-perheella korttityyppi SIID:sta
// (lohko 0, tavut 25..27) ja SI10/11:lla leimamaara (tavu 22) ->
// tarvittavat leimalohkot. Asettaa *skip = 9 aina kun pyynto lahtee
// (edellisen vastauksen CRC+ETX 3 tavua + seuraavan otsikko 6 tavua).
// Palauttaa SIPYY_*-arvon.
int siLukuSeuraava(SILukuTp *t, const unsigned char *buf, int l, int *skip);

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
