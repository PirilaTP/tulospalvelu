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
	bool onMaalissa;    // onko paikalle kirjattu maaliaika (aika on aina mukana, vaikka nimi puuttuisi)
	long kulunutAika;   // kulunut aika lahdosta, mielivaltaisessa yhteismitallisessa
	                    // yksikossa; merkitseva vain jos onMaalissa
	};
#pragma pack(pop)

// Palauttaa 0-pohjaisen indeksin osat-taulukkoon: se paikka, joka on
// ensimmaisena maalissa (pienin kulunutAika niista, jotka ovat maalissa).
// Paikka, jolle on kirjattu aika, on aina mukana, vaikka nimea ei olisi
// (onKilpailija == false): aika tallennetaan ja nimi voidaan lisata
// myohemmin. Paikka ilman aikaa ei voi voittaa.
//
// Palauttaa -1, jos kukaan ei ole viela maalissa.
int EkaMaaliIndeksi(const RinnakkaisTila *osat, int n);

// Paattaa, onko rinnakkaisosuuden "puute-lisaaika" (odotusaika
// puuttuvien tulosten vuoksi) nollattava, eli saako osuutta pitaa
// valmiina seuraavan osuuden lahdon laskentaa varten.
//
// registered  kaytossa olevien (ilmoitettujen) paikkojen lkm osuudella
// finished    niista maalissa olevien lkm
// ekaMaaliRatkaisee  onko "vain ensimmaisena maaliin tullut ratkaisee"
//             -saanto kaytossa talla osuudella
//
// Jos registered == 0 (ei ketaan ilmoitettu), osuutta ei koskaan
// odoteta - palauttaa aina true. Muuten: jos ekaMaaliRatkaisee, riittaa
// etta yksikin on maalissa; muuten kaikkien ilmoitettujen on oltava.
bool PuutelisaNollataan(int registered, int finished, bool ekaMaaliRatkaisee);

// Onko rinnakkaisosuuden paikalla kilpailija, jota odotetaan ja jonka
// maaliaika voi ratkaista osuuden. Paikka on kaytossa, jos sille on
// annettu nimi, Emit-koodi tai aika. Pelkka erotin "|" (tai tyhja/
// valilyonnit) ei ole nimi: CSV-tuonti on tallentanut tyhjan nimen
// muodossa "|". Nimeton juoksija, jolla on aika tai Emit-koodi, on
// siis mukana - nimen puuttuminen ei saa hukata tulosta.
bool PaikkaKaytossa(const char *nimi, bool onBadge, bool onAika);

// Lahteeko osuuden jokainen paikka edellisen osuuden joukkuetuloksesta
// (yhteinen vaihto)? Kylla, jos jompikumpi osuus on tavallinen (yksi
// paikka) tai jos osuudella on kaytossa "lahtee edellisen rinnakkais-
// osuuden ensimmaisen maalista" -saanto. Muuten (rinnakkais-
// osuudelta rinnakkaisosuudelle ilman saantoa) k:s paikka lahtee
// edellisen osuuden k:nneksi nopeimman maaliajasta.
bool LahtoEdellisenTuloksesta(int nosuusNyt, int nosuusEd, bool lahtoEdEkaMaalista);

// Onko juoksijan tai osuuden tila hyvaksytty: '-' (avoin), 'T' tai 'I'.
// Muut ('K' keskeytti, 'H' hylatty, 'E' ei lahtenyt, ...) eivat ole.
bool TilaHyvaksytty(char tila);

// Rinnakkaisosuuden tila, kun osuudella on kaytossa "ensimmainen maaliin
// kaynnistaa seuraavan osuuden" -saanto. tila[i] on paikan i tila
// (keskhyl).
//
// Ratkaisee ensimmainen ajallinen tapahtuma. Tapahtuman aika on paikan
// maaliaika; hylkays tai keskeytys ratkaisee vain, jos sille on kirjattu
// aika. Ilman aikaa joukkuetta ei voi hylata eika keskeyttaa, kun joku
// on viela matkalla.
//
// - Kun jollakulla on aika, aikaisimman tila on osuuden tila: hanen
//   hylkayksensa tai keskeytyksensa on joukkueen, vaikka muut olisivat
//   myohemmin tulleet hyvaksyttyina maaliin, ja myohempien hylkays tai
//   keskeytys ei vaikuta, jos han on hyvaksytty.
// - Kun kenellakaan ei ole aikaa ja joku on viela mukana, osuus on
//   kesken (palautetaan hanen tilansa), vaikka joku toinen olisi jo
//   hylatty tai keskeyttanyt ilman aikaa.
// - Kun kenellakaan ei ole aikaa ja kaikki ovat ulkona, palautetaan
//   huonoin tila samassa jarjestyksessa kuin ilman saantoa: 'E' ennen
//   'K':ta ennen 'H':ta.
// - Jos osuudelle ei ole ilmoitettu ketaan, palautetaan '-'.
char EkaMaaliOsuudenTila(const RinnakkaisTila *osat, const char *tila, int n);

#endif
