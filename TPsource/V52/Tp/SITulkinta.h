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

#endif
