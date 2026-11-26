# C-12 osa A0: esikääntäjän valitsinten raakainventaario / Part A0: raw preprocessor switch inventory

Tuotettu 2026-11-26 revisiosta `ab10f88` (`TPsource/` ennallaan revisiosta `f7fe553`)
skriptillä `../../../tyokalut/valitsimet.py`. Skripti käyttää C-10:n mallia
(`riippuvuudet.build_model`): projektien arviointi, include-sulkeuma, kolmiarvoinen `#if`-arvioija
ja omistajakohtaiset makrojoukot. Raakatulos on todiste, ei todennettu tulos; tulkinta
`../../../ominaisuusvalitsimet.md`.

Produced on 2026-11-26 from revision `ab10f88` with `tyokalut/valitsimet.py`, which reuses the
C-10 model. Raw evidence, not a verified result; interpretation in `ominaisuusvalitsimet.md`.

Toisto / reproduce (repon juuresta / from the repository root):
`python -I doc/arkkitehtuuri/tyokalut/valitsimet.py . doc/arkkitehtuuri/todisteet/c12/osaA0`

| Tiedosto / file | Sisältö / content |
|---|---|
| `valitsimet.csv` | 435 ehdoissa testattua makroa: ehdotettu luokka, ehtokohdat ja niiden tilat, arvo 9 omistajalla toimitettavassa kokoonpanossa, poikkeavat arvot muissa kokoonpanoissa, projektimääritykset, määrityskohdat ja ehdot / tested macros, values per owner and configuration, definition sites |
| `testikohdat.csv` | 2 491 ehtokohtaa (`#if/#ifdef/#ifndef/#elif/#else`): ympäröivä ehto, haaran rivimäärä, saavutettavuus (`reach_`) ja haaran voimassaolo (`take_`) tuotteittain, tila / directive sites with per-product reachability and status |
| `maaritykset.csv` | kaikki `#define`/`#undef`-rivit ehtoineen / every define and undef with its condition |
| `kokoonpanot.csv` | projektimääritykset omistajittain ja kokoonpanoittain sekä testiprojektit / project defines per owner and configuration |
| `yhteenveto.txt` | yhteenveto / summary |
| `tarkistus_a0.py`, `.out` | tarkistus ennen agenttikäyttöä / validation before agent use |

Tilat / status: `always` haara voimassa kaikissa tuotteissa, joissa tiedosto on; `varies`
tuotteittain; `dead` saavutettava mutta ei koskaan voimassa; `unreachable` ympäröivä ehto epätosi;
`unknown` arvioija ei ratkaise (`MAKI`, `HIIHTO` ym.); `not-in-product` tiedosto ei kuulu
mihinkään tuotteeseen. / Branch taken everywhere, per product, never although reachable, enclosing
condition false, unresolved, file in no product.

Kokoonpanot / configurations: toimitettava (Release, `.cbproj`-oletus) ja kaikki muut projektien
kokoonpanot (Debug, Demo, DebugV200, ReleaseVA200, Debug250, Release150os, 200Debug, 200Release,
Debug150os). Kirjastot arvioidaan Debug-kokoonpanossa vain ajossa, jossa kaikki omistajat ovat
Debugissa. / Libraries in Debug only in the all-Debug run.

## Luotettavuuden varmistus ennen agenttikäyttöä / Validation before agent use

| Tarkistus / check | Tulos / result |
|---|---|
| 1. Release-arvot vs. C-10 `makrot.csv` (435 makroa × 9 omistajaa) / Release values vs. C-10 | täsmää, 0 eroa / match, 0 differences |
| 2. Projektimääritysten yhdiste kaikista kokoonpanoista vs. `inventaario.md`:n valitsintaulukko / union of project defines vs. round-1 table | täsmää; ainoa ero `VERSIONO=510` HkKisaWinissä: se on vain Win64-ryhmässä (`Base_Win64`, `HkKisaWin.cbproj:193`), joten Win32-tuotteessa sitä ei ole, eikä mikään ehto testaa sitä / match; `VERSIONO` exists only in the Win64 group and is never tested |
| 3. Raakahaku `#if/#ifdef/#ifndef/#elif/#else` kommentit poistettuina vs. ehtokohdat / raw directive count | 2 491 = 2 491 |
| 4. Kokoonpanokohtaiset määritykset suoraan `.vcxproj`-XML:stä (15 kokoonpanoa) / per-configuration defines from the XML | täsmää kaikissa / match in all |
| 5. Otanta / sample | 24 satunnaista kohtaa (kaikki tilat); 5 luettu koodista, loput tarkistettu makron määrityshaulla; lisäksi korjauksen muuttamat 63 kohtaa käyty läpi. Virhe 1 löytyi ja korjattiin / 24 random sites, 5 read in code, rest by definition search; the 63 sites changed by the fix reviewed; one defect fixed |

Validoinnissa korjattu vika / defect fixed during validation:

1. Oletusarvolohkot (`#ifndef X / #define X v / #endif`, esim. `Juk/VDef.h:198` `LSAKOT`) näkyivät
   kuolleina, koska omistajan makrojoukossa oli jo lohkon oma määritys; sama teki `.cpp`-tiedoston
   `#ifndef _CONSOLE / #define _CONSOLE` -kaavan tuntemattomaksi (`tputilv2/Inp*.cpp`). Korjaus:
   lohkon sisällä X arvioidaan projektimäärityksen tai toisen otsikon ehdottoman määrityksen
   mukaan, ja `.cpp`:n oma `#ifndef X / #define X` on määritys. Ensimmäinen korjausyritys koski
   koko tiedostoa ja teki `tputilv2/Wrttext.cpp:510`:n väärin kuolleeksi; sääntö rajattiin
   lohkon riveihin. Muutos: 63 kohdan tila, kaikki tarkistettu. / Default-value blocks looked
   dead; fixed and limited to the block's own lines; 63 site states changed, all checked.

C-10:n tulosteisiin tämä ei vaikuta: `riippuvuudet.py`:hin lisättiin vain `build_model()`
(entinen `main()`:n alkuosa) ja `.cbproj`-kokoonpanon valinta; C-10:n seitsemän tulostetta ovat
tavulleen ennallaan (SHA-256 tarkistettu). / C-10 outputs unchanged byte for byte.

Rajoitukset / limits: leksikaalinen analyysi, otsikoiden järjestystä ei mallinneta; `MAKI` ja
`HIIHTO` jäävät HK:ssa tuntemattomiksi; ehdotettu luokka on sääntöpohjainen ja osa A tarkistaa sen.
Havaittu nimitörmäys: `GDIPRINTER` on tputilv2:n projektivalitsin ja otsikoissa tulostintyypin
vakio 7. / Lexical; header order not modelled; suggested class is rule-based; name collision
`GDIPRINTER` noted for part A.
