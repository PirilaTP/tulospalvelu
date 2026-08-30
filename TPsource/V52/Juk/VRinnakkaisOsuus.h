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

// Paatoslogiikka "ensimmainen maaliin kaynnistaa seuraavan osuuden"
// -saannolle. Tama kaannosyksikko on tarkoituksella riippumaton
// globaaleista muuttujista (Sarjat[], ostiet[]), tietokannasta ja
// VCL:sta, jotta se voidaan kaantaa yksikkotesteihin
// (ks. Tests/VRinnakkaisOsuusTest.cpp). Kutsuja Juk/vkilp.cpp:ssa
// (kilptietue::ekaMaaliOsuus, kilptietue::tTulos) poimii parametrit
// globaaleista ja kutsuu tata.

#ifndef VRINNAKKAISOSUUS_DEFINED
#define VRINNAKKAISOSUUS_DEFINED

// Yhden rinnakkaisosuuden (esim. 2a) tila ensimmainen-maaliin
// -paatosta varten. Layout on kiinnitetty eksplisiittisesti (pack 1),
// koska tama struct nakyy seka vkilp.cpp:lle (VDef.h:n pack(1)-alueen
// jalkeen) etta VRinnakkaisOsuus.cpp:lle (ei VDef.h:ta lainkaan) - ilman
// tata pinnausta kaannosyksikot voivat paatya erisuuruiseen sizeof():iin
// ymparoivan #pragma pack -tilan mukaan, mika rikkoo taulukon indeksoinnin.
#pragma pack(push, 1)
struct RinnakkaisTila {
	bool onKilpailija;  // onko tahan paikkaan ilmoitettu kilpailija
	bool onMaalissa;    // onko kilpailija jo maalissa (merkitsevaa vain jos onKilpailija)
	long kulunutAika;   // kulunut aika lahdosta, mielivaltaisessa yhteismitallisessa
	                    // yksikossa; merkitseva vain jos onKilpailija && onMaalissa
	};
#pragma pack(pop)

// Palauttaa 0-pohjaisen indeksin osat-taulukkoon: se rinnakkaisosuus,
// johon on ilmoitettu kilpailija (onKilpailija) JA joka on ensimmaisena
// maalissa (pienin kulunutAika niista, jotka ovat maalissa). Kaytossa
// olematon paikka ei koskaan voita eika sita odoteta.
//
// Palauttaa -1, jos kukaan kaytossa-olevista ei ole viela maalissa
// (mukaan lukien se tapaus, ettei kukaan ole ilmoittautunut).
int EkaMaaliIndeksi(const RinnakkaisTila *osat, int n);

// Paattaa, onko rinnakkaisosuuden "puute-lisaaika" (odotusaika
// puuttuvien tulosten vuoksi) nollattava, eli saako osuutta pitaa
// valmiina seuraavan osuuden lahdon laskentaa varten.
//
// registered  kaytossa olevien (ilmoitettujen) paikkojen lkm osuudella
// finished    niista maalissa olevien lkm
// ekaMaaliLahettaa  onko "ensimmainen maaliin kaynnistaa seuraavan
//             osuuden" -saanto kaytossa talla osuudella
//
// Jos registered == 0 (ei ketaan ilmoitettu), osuutta ei koskaan
// odoteta - palauttaa aina true. Muuten: jos ekaMaaliLahettaa, riittaa
// etta yksikin on maalissa; muuten kaikkien ilmoitettujen on oltava.
bool PuutelisaNollataan(int registered, int finished, bool ekaMaaliLahettaa);

#endif
