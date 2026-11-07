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

// Kieliversiot (fi/en/sv).
//
// Ohjelman tekstit kirjoitetaan lähdekoodiin suomeksi ja käännetään ajon aikana
// gettext-tyylisesti: avaimena on suomenkielinen teksti, käännökset luetaan
// tiedostoista <ohjelmahakemisto>\kielet\en.po ja sv.po. Jos käännöstä ei ole,
// käytetään suomenkielistä tekstiä, joten tekstejä voi kääntää vähitellen.
//
// Kieliä on kaksi erillistä:
//   kayttokieli   käyttöliittymän kieli (lomakkeet, ilmoitukset, konsolin kehotteet)
//   tulostekieli  tulosteiden oletuskieli (tulos- ja lähtöluettelot, html)
// Yksittäisen tulosteen kieli on tulostusparamtp::language.

#ifndef KIELET_DEFINED
#define KIELET_DEFINED

#include <wchar.h>

// Kielikoodit. Arvot 0 ja 1 ovat samat kuin vanhassa language-kentässä.
#define KIELI_FI 0
#define KIELI_EN 1
#define KIELI_SV 2
#define KIELILKM 3

extern int kayttokieli;
extern int tulostekieli;

// Tulkitsee kielikoodin (FI/SV/EN, myös vanhat KIELI=S ja KIELI=E). Palauttaa oletus, jos tuntematon.
int kieliKoodista(const wchar_t *koodi, int oletus);
// ISO-kielikoodi pienillä kirjaimilla, esim. L"sv" (html:n lang-attribuutti, .po-tiedoston nimi).
const wchar_t *kieliKoodi(int kieli);
// Kielen nimi kielellä itsellään: Suomi, English, Svenska.
const wchar_t *kieliNimi(int kieli);

// Hakemisto, josta .po-tiedostot luetaan. Oletus on <ohjelmahakemisto>\kielet.
void asetaKaannoshakemisto(const wchar_t *hakemisto);
// Lukee käännöstiedoston kielelle (korvaa aiemmin luetut). Palauttaa käännösten määrän tai -1.
int lataaKaannostiedosto(int kieli, const wchar_t *tiedosto);

// Käännös kielelle kieli. Palauttaa fi:n, jos kieli on suomi tai käännöstä ei ole.
// Palautettua merkkijonoa ei saa muuttaa. Paluuarvo on ei-const, jotta funktiota
// voi käyttää vanhoissa wchar_t *-parametreissa samoin kuin merkkijonovakiota.
wchar_t *TRL(int kieli, const wchar_t *fi);
// Käännös käyttöliittymän kielelle.
wchar_t *TR(const wchar_t *fi);
// Käännös, jonka avaimena on lisäksi konteksti (.po-tiedoston msgctxt). Käytetään,
// kun sama suomenkielinen teksti käännetään eri yhteyksissä eri tavoin.
wchar_t *TRCL(int kieli, const wchar_t *konteksti, const wchar_t *fi);
// 8-bittinen (Windows-1252) versio TR:stä vanhoille char *-teksteille.
char *TRA(const char *fi);

// Kääntää konsolin valintakehotteen ja sen pikanäppäimet käyttöliittymän kielelle.
// Pikanäppäimet tunnistetaan muodoista "S)euraava", "eR)ätiedot" ja "(K/E)".
// term: sallitut merkit, tterm: käännetyt merkit samassa järjestyksessä (koko >= wcslen(term)+1).
// Jos käännöstä ei ole tai näppäimiä ei voi vastaavuuttaa yksikäsitteisesti,
// palauttaa alkuperäisen kehotteen ja tterm = term.
const wchar_t *kaannaValinta(const wchar_t *prompt, const wchar_t *term, wchar_t *tterm, int koko);
// Etsii merkin c vastineen: c:n paikka mista-merkkijonossa -> sama paikka mihin-merkkijonossa.
wchar_t vastaavaMerkki(wchar_t c, const wchar_t *mista, const wchar_t *mihin);

// Asettaa käyttöliittymän kielen.
void asetaKayttokieli(int kieli);
// Käyttöliittymän kieli käyttäjän asetuksista (rekisteri HKCU\Software\Tulospalvelu),
// oletus KIELI_FI.
int lueKayttokieliAsetus(void);
// Tallentaa käyttöliittymän kielen käyttäjän asetuksiin. Palauttaa 0, jos onnistui.
int tallennaKayttokieliAsetus(int kieli);

#endif
