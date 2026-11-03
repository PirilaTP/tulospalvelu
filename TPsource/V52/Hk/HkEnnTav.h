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

// Ennatys- ja tavoiteajan (kilppvtp::enn, kilppvtp::tav) tekstiesitys
// Osanottajat-taulukossa. Ei VCL-riippuvuuksia, jotta funktiot voidaan
// yksikkotestata (Tests/HkEnnTavTest.cpp).

#ifndef HkEnnTavH
#define HkEnnTavH

#include <wchar.h>

// Puskurin vahimmaispituus ennTavTeksti-funktiolle (tt.mm.ss + loppunolla).
#define ENNTAV_LEN 9

// Kirjoittaa ajan t (sisaisina yksikkoina) puskuriin buf samassa muodossa
// kuin Kilpailijatiedot-kaavake: tt.mm.ss kokonaisina sekunteina. Sekunnin
// osat katkaistaan. t == 0 tarkoittaa, ettei aikaa ole annettu, ja tuottaa
// tyhjan merkkijonon. Palauttaa buf.
wchar_t *ennTavTeksti(wchar_t *buf, int t);

// Onko kayttaja muuttanut solua: true, jos solun teksti poikkeaa
// tallennetun arvon naytosta. Vain muutettu solu luetaan takaisin, jotta
// naytosta katkaistut sekunnin osat eivat katoa tallennettaessa muita
// muutoksia.
bool ennTavMuutettu(const wchar_t *solu, int tallennettu);

// Lajitteluavaimen ennTavAvain-funktion kirjoittaman alun pituus.
#define ENNTAV_AVAIN 12

// Kirjoittaa lajitteluavaimen alun (ENNTAV_AVAIN merkkia) ajalle t: annetut
// ajat nousevaan jarjestykseen ja puuttuvat (t == 0) viimeisiksi. Avaimen
// loppuun voi lisata tasatilanteiden ratkaisevan osan (esim. nimen).
// key on nollattava ennen kutsua.
void ennTavAvain(char *key, int t);

#endif
