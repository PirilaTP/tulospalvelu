# Kieliversiot (suomi, ruotsi, englanti)

Ohjelman tekstit kirjoitetaan lähdekoodiin ja lomakkeisiin suomeksi. Ajon aikana ne
käännetään gettext-tyylisesti: avaimena on suomenkielinen teksti ja käännökset luetaan
tiedostoista `en.po` ja `sv.po`. Jos käännös puuttuu, teksti näytetään suomeksi, joten
puuttuva käännös ei koskaan riko ohjelmaa.

| Tiedosto | Sisältö |
|---|---|
| `tulospalvelu.pot` | Kaikki käännettävät tekstit, tuotetaan lähdekoodista (`kielet.py poimi`). Älä muokkaa käsin. |
| `en.po`, `sv.po` | Käännökset. Näitä muokataan, esim. tekstieditorilla tai [Poeditilla](https://poedit.net/). |
| `kielet.py` | Työkalu: poiminta, päivitys, tarkistus ja `TR()`-käärintä. |

Ohjelma etsii käännökset hakemistosta `<ohjelman hakemisto>\kielet\`, ja jos niitä ei
sieltä löydy, nykyhakemiston alta `kielet\`. Julkaisupaketissa tiedostot ovat valmiina
`console\kielet\`- ja `gui\kielet\`-hakemistoissa.

HkKisaWin- ja ViestiWin-projektit kopioivat käännöstiedostot automaattisesti
ohjelman viereen myös paikallisessa C++Builder-koonnissa (`RADStudio10/Kielet.targets`).
Jos kieli ei vaihdu uudelleenkäynnistyksessä, tarkista, että käytössä olevan `.exe`-tiedoston
vieressä on `kielet\en.po` ja `kielet\sv.po`. Kopioi ohjelmaa siirtäessäsi myös `kielet`-hakemisto.

## Kaksi eri kieltä

- **Käyttöliittymän kieli** (`kayttokieli`): lomakkeet, valikot, ilmoitukset ja konsolin kehotteet.
  - Windows-ohjelmat: *Tiedostot → Kieli / Språk / Language*. Valinta tallennetaan
    käyttäjän rekisteriin (`HKCU\Software\Tulospalvelu`, arvo `Kayttokieli`) ja tulee
    voimaan, kun ohjelma käynnistetään uudelleen.
  - Konsoliohjelmat: sama rekisteriasetus tai konfiguraatiotiedoston rivi
    `KAYTTOKIELI=FI|SV|EN`.
- **Tulosteiden kieli** (`tulostekieli`, tulosteen `tulostusparamtp::language`): tulos- ja
  lähtöluettelot, html-sivut, väliaikatulosteet ja muut tulosteet.
  - Windows-ohjelmat: tulostusikkunan valikko *Tulosteiden kieli*.
  - Konfiguraatiotiedosto: `KIELI=FI|SV|EN` (oletus myös automaattisille tulosteille).
    Vanhat arvot `KIELI=S` (suomi) ja `KIELI=E` (englanti) toimivat edelleen.

Käyttöliittymä voi siis olla suomeksi, vaikka tulokset julkaistaan ruotsiksi.

## Ohjelmoijalle

Kaikki on määritelty tiedostossa `include/Kielet.h` (sisältyy `tputil.h`:hon).

| Käyttö | Kirjoita |
|---|---|
| Käyttäjälle näkyvä teksti (ilmoitus, otsikko, kehote) | `TR(L"Tiedostoa ei löydy")` |
| Vanha `char *`-teksti (Windows-1252) | `TRA("Tiedostoa ei löydy")` |
| Tulosteeseen menevä teksti | `TRL(tulprm->language, L"Tulokset")` |
| Sama suomenkielinen teksti, eri käännös eri yhteydessä | `TRCL(kieli, L"lyhenne", L"Hyl.")` |
| Taulukon alustuksessa oleva teksti | `N_(L"Nimi :")` ja käyttökohdassa `TR(taulu[i])` |
| Konsolin valintakehote | `wselectopt(L"S)euraava, E)dellinen", L"SE", &ch)` sellaisenaan |

- **Lomakkeet (.dfm)** käännetään automaattisesti: jokaisen lomakkeen konstruktori alkaa
  rivillä `kaannaLomake(this);` (`cbTp/cbKielet.cpp`). Uuteen lomakkeeseen lisätään sama
  rivi ja `#include "cbKielet.h"`. Käännetään `Caption`, `Hint`, `Items`, `Lines`,
  `EditLabel`, `Title`, `Filter`, tilarivin paneelit ja ListView-sarakkeet.
  Käännöksen jälkeen `kaannaLomake` sovittaa pidemmät tekstit lomakkeelle:
  1. painike tai valintaruutu levennetään, jos sen oikealla puolella on vapaata tilaa
  2. jos teksti ei silti mahdu, käytetään tekstin **lyhyttä muotoa** (ks. alla), jos sellainen on
  3. muuten teksti katkaistaan "…"-merkkiin; automaattisesti levenevä teksti (TLabel) ei saa
     peittää viereistä kontrollia.

  Kun teksti on lyhennetty tai katkaistu, koko teksti näytetään vihjeenä (Hint).
  Valintaryhmän (TRadioGroup) vaihtoehdoille sama: lyhyt muoto ja vaihtoehdon oma vihje.
- **Lyhyt muoto** kirjoitetaan .po-tiedostoon omaksi merkinnäkseen kontekstilla `lyhyt`
  pääkäännöksen perään:

  ```
  msgid "Riviväli"
  msgstr "Line spacing"

  msgctxt "lyhyt"
  msgid "Riviväli"
  msgstr "Ln sp."
  ```

  Lyhyttä muotoa käytetään vain lomakkeilla ja vain silloin, kun pitkä ei mahdu; muualla
  (ilmoitukset, tulosteet, `TR()`) käytetään aina pitkää käännöstä. `kielet.py paivita`
  säilyttää `lyhyt`-merkinnät, kunhan suomenkielinen teksti on edelleen käytössä.
- **Konsolin valintakehotteet** (`wselectopt`, `selectopt`, `wselectxy`, `selectxy` ja
  GUI:n `Win_wselectopt`) kääntävät kehotteen itse. Pikanäppäimet tunnistetaan
  muodoista `S)euraava`, `eR)ätiedot` ja `(K/E)`, ja ne vaihdetaan käännöksen mukaisiksi.
  Kutsuja saa aina takaisin alkuperäisen (suomenkielisen) näppäimen, joten `switch (ch)`
  ei muutu. Jos käännetyn kehotteen näppäimiä ei voi vastaavuuttaa yksikäsitteisesti,
  kehote näytetään suomeksi.
- **Älä käännä** tekstejä, joita ohjelma vertaa tai jäsentää (`wcscmp(x, L"Hyl")`,
  `Cells[..] == L"COM"`), konfiguraatiotiedoston avainsanoja (`L"KIELI="`), xml-tageja,
  tiedostonimiä, CSV-tiedostojen sarakeavaimia tai lokitiedostojen rivejä. Jos teksti
  sekä näytetään että verrataan, käännä molemmat: `wcscmp(x, TR(L"Hyl"))`.
- Muokattavien taulukoiden **tilasarakkeet** (Kesk, Hyl, esItys, Eiläht., Laina ...) ovat
  suomeksi myös muilla kielillä: tallennettaessa tila luetaan solun alkukirjaimista
  (`FirstChar(Cells[c][r])`). `kielet.py kaari` ei käännä näitä sanoja tällaisissa tiedostoissa.
- Taulukoiden **sarakeotsikot** käännetään kirjoitettaessa ne soluun: `Cells[c][0] = TR(L"Nimi")`;
  `ColTp`-taulukoissa otsikko merkitään `N_(L"Nimi")` ja käännetään käyttökohdassa
  `TR(Cols[i].Caption)`.
- `TR`-funktioiden palauttamaa merkkijonoa ei saa muuttaa, ja se on voimassa koko ajon.
  Suomella `TR(s)` palauttaa saman osoittimen `s`.
- Muotoilumerkkijono käännetään ennen muotoilua: `swprintf(msg, TR(L"Kilpailija %d puuttuu"), kno)`.

### Työnkulku tekstejä muutettaessa

```
cd TPsource/V52
python kielet/kielet.py poimi       # päivittää tulospalvelu.pot
python kielet/kielet.py paivita     # lisää uudet tekstit en.po:hon ja sv.po:hon (käännökset säilyvät)
python kielet/kielet.py tarkista    # muotoilukoodit, pikanäppäimet, &-merkit, rivinvaihdot
python kielet/kielet.py tilasto     # käännettyjen määrät
python kielet/kielet.py mittaa      # käännökset, jotka eivät mahdu lomakkeille
```

`kielet.py mittaa [--kieli sv|en] [--lomake nimi] [--kaikki] [--tsv tiedosto]` mittaa
jokaisen lomaketekstin käännöksen leveyden kontrollin omalla fontilla (96 DPI) ja listaa
tekstit, jotka eivät mahdu ja joita `kaannaLomake` ei voi leventää eikä korvata lyhyellä
muodolla (`--kaikki` listaa myös ne). Vertailukohtana on suomenkielinen teksti: sen leveys
hyväksytään aina. Komento vaatii Pillow-kirjaston (`pip install pillow`) ja Windowsin
fontit, joten sitä ei ajeta CI:ssä. Lisää listatuille teksteille lyhyt muoto tai muuta
lomakkeen asettelua C++Builderin lomake-editorissa.

CI (`.github/workflows/build.yml`) tarkistaa, että `tulospalvelu.pot` vastaa lähdekoodia
ja että `kielet.py tarkista` menee läpi.

`python kielet/kielet.py kaari [--nayta] [--ei-ui] tiedostot...` käärii
käyttöliittymän merkkijonovakiot automaattisesti `TR()`-kutsuihin (ilmoitusfunktiot,
`Caption`/`Hint`-sijoitukset, `Items->Add`, `Cells[][]`, käyttöliittymätiedostoissa myös
muotoilufunktiot). `--nayta` näyttää muutokset kirjoittamatta niitä.

## Käännösten tarkistajalle

- `msgid` on suomenkielinen alkuteksti, `msgstr` käännös. Tyhjä `msgstr` = näytetään suomeksi.
- Rivi `#: tiedosto` kertoo, missä teksti esiintyy (`.dfm` = lomake, `HkConsole`/`JukConsole`
  = konsoliohjelma, `HkTulostus`/`VTulostus`/`*Emit*` = tuloste).
- `#, fuzzy` -merkittyjä käännöksiä ohjelma ei käytä. Merkintää voi käyttää epävarmoille.
- Säilytä:
  - muotoilukoodit samassa järjestyksessä (`%d`, `%s`, `%-20s`)
  - alku- ja loppuvälilyönnit sekä rivinvaihdot (`\n`)
  - valikoiden `&`-pikanäppäin (`&Tiedostot` → `&Arkiv`)
  - konsolin pikanäppäinmerkinnät: kehotteessa yhtä monta `X)`-kohtaa ja `(K/E)` →
    `(J/N)`, kirjaimet yksikäsitteisiä (rivi `#. valinta: ...` kertoo sallitut näppäimet).
    Konsolin päävalikon ja muiden valikkoruutujen (`HkConsHelp.cpp`, `lasktxt.cpp`)
    kirjainten on oltava samat kuin vastaavan valintakehotteen käännöksessä.
- Konsolissa ja tekstimuotoisissa tulosteissa rivin leveys on rajattu: pidä käännös
  suunnilleen alkutekstin pituisena.
- Lomakkeilla tila on mitoitettu suomenkielisille teksteille. Älä lyhennä pääkäännöstä
  lomakkeen takia: kirjoita lyhyt muoto erilliseksi `msgctxt "lyhyt"` -merkinnäksi
  (ks. yllä), jolloin pitkä muoto näkyy vihjeenä ja muualla ohjelmassa. `kielet.py mittaa`
  kertoo, mitkä käännökset eivät mahdu ja kuinka monta pikseliä tilaa on.
- Lyhenteet (esim. `Hyl.`, `Kesk.`, `Tls`) ovat sarakkeissa: käytä vakiintuneita lyhenteitä
  (en: DQ, DNF, DNS; sv: Disk, Utg, Ej start).
