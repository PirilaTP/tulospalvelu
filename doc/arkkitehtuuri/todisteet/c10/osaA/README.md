# C-10 osa A: riippuvuusinventaarion raakatulos / Part A: raw dependency inventory

Tuotettu 2026-11-26 revisiosta `4b48749` (`TPsource/` ennallaan revisiosta `f7fe553`)
skriptillä `../../../tyokalut/riippuvuudet.py`. Tulkinta: `../../../riippuvuudet.md`.
Raakatulos on todiste, ei todennettu tulos.

Produced on 2026-11-26 from revision `4b48749` with `tyokalut/riippuvuudet.py`.
Interpretation: `riippuvuudet.md`. Raw evidence, not a verified result.

Toisto / reproduce: `python -I doc/arkkitehtuuri/tyokalut/riippuvuudet.py . doc/arkkitehtuuri/todisteet/c10/osaA`

| Tiedosto / file | Sisältö / content |
|---|---|
| `tuotteet.csv` | Projektit toimitettavalla kokoonpanolla: määritykset, include-polut, kirjastot, paketit, RTL / projects in the shipped configuration |
| `kaannosyksikot.csv` | Käännösyksiköt projekteittain / translation units per project |
| `tiedostot.csv` | Tiedosto → projekti (`compiled` / `included`) ehdot huomioiden / file → project, conditions considered |
| `makrot.csv` | Makrojen arvot ja tuntemattomat makrot projekteittain / macro values and unknown macros per project |
| `osumat.csv` | 3 779 riippuvuusmerkkiä: tiedosto, rivi, luokka, laji, sana, `#if`-ehto, saavutettavuus tuotteittain / dependency markers with condition and per-product reachability |
| `dfm.csv` | 161 lomaketta, 6 317 komponenttia ja kiinnostavat ominaisuudet / forms, components, key properties |
| `yhteenveto.txt` | Yhteenveto / summary |
| `tarkistus_c5.py`, `.out` | Tarkistus 1: lähdeluettelot ja määritykset C-5:n tallentamia käännöksiä vasten / check 1 against C-5 recorded builds |
| `tarkistus_kattavuus.py`, `.out` | Tarkistus 2: avain-API:en osumat raakahakua vasten, tuntemattomien ehtojen makrot, kuollut koodi / check 2 coverage, unknown conditions, dead files |

Saavutettavuuskenttä / reachability field: `C:y` = käännetään, ehto tosi / compiled, condition
true; `I:n` = sisällytetty otsikko, ehto epätosi / included header, false; `?` = tuntematon /
unknown; kirjaston nimi edessä = kirjaston kautta / library name prefix = via that library;
tyhjä = ei tuotteessa / empty = not in the product.

## Luotettavuuden varmistus ennen agenttikäyttöä / Validation before agent use

C-9:n kokemus: v1-skriptin systemaattiset virheet löytyivät vasta, kun agentit olivat käyttäneet
sitä. Siksi tämä skripti tarkistettiin ennen kuin osat B–E saivat sen tulokset.
Lesson from C-9: the v1 script's systematic errors surfaced only after agents had used it, so
this script was checked before parts B–E got its output.

| Tarkistus / check | Tulos / result |
|---|---|
| Käännösyksiköt 9 projektissa vs. C-5:n tallennetut käännökset (MSVC-komentorivit `c5/msvc/units.json`, bcc32-yksiköt `c5/bda.tgz` `ana_*.json`) / units vs. recorded builds | **täsmää / match**: 32, 34, 32, 146, 7 (MSVC), 141, 117, 139, 8 (bcc32) |
| MSVC-määritykset vs. komentorivit / MSVC defines vs. command lines | täsmää; vain `_VC80_UPGRADE=0x0710` puuttuu skriptiltä (MSBuild-ominaisuus, koodi ei käytä sitä) / match except `_VC80_UPGRADE`, unused in code |
| bcc32-määritykset / bcc32 defines | täsmäävät C-5:n taulukoiden mitattuihin tosiasioihin (`_BORLAND_` puuttuu ViestiWinistä, Tputil-XE `_UNICODE;_MBCS`, DBboxm-XE ilman `-tU`, viestin `NREGNLY` 8 konsolissa / 32 GUI:ssa); bcc32:n täysiä komentorivejä ei ole arkistoitu / consistent with C-5 measured facts; full bcc32 command lines not archived |
| Ajonaikainen kirjasto / RTL | MSVC `/MT` täsmää; GUI `DynamicRTL=true`, `UsePackages=true` |
| Avain-API:t (14 nimeä) raakahakua vasten / key APIs vs. raw search | täsmää kommentit poissulkien; ainoa ero `SetCommState` merkkijonoliteraalissa (`com/winCom.cpp:92`), ei kutsu / match outside comments; one string literal |
| Otanta ehdollisista riveistä käsin / manual sample of conditional rows | 8 + 6 riviä luettu, arviot oikein korjausten jälkeen / correct after the fixes |

Validoinnissa korjatut viat / defects fixed during validation:

1. Include-vartijat (`#ifndef XH / #define XH`) tekivät koko otsikon ehdosta tuntemattoman
   (HkKisaWinissä 728 tuntematonta riviä) → vartija tunnistetaan ja ehto on tosi; myös
   tiedoston lopussa määritelty vartija (`include/butil.h` `DEF_BUTIL`). / include guards made
   whole headers unknown.
2. `#include`-rivin ympäröivä ehto ohitettiin → otsikoita liitettiin tuotteisiin, joihin ne eivät
   kuulu (esim. `cbTp/UnitBrowser.h` HkMaaliin) → sulkeuma lasketaan iteratiivisesti omistajan
   makroilla. / conditions around includes were ignored (over-attribution).
3. Järjestelmäotsikoiden ryhmittely: `.hpp`-sääntö ohitti dbExpress- ja selainryhmät. / header
   grouping order.
4. `.cpp`-tiedoston oma `#define` levisi koko projektiin (esim. `tputilv2/Inputaik.cpp:21`
   `_CONSOLE` → Tputil-XE) → lähdetiedoston määritys koskee vain sitä käännösyksikköä. /
   TU-local defines leaked project-wide.
5. Ehdolliset `#define`-rivit (`#ifndef NREGNLY / #define NREGNLY 32`) arvioidaan nyt omalla
   ehdollaan; tuntemattomia makroja jäi `MAKI` ja `HIIHTO` (HK) sekä `EMITFILE` (ViestiLuenta). /
   conditional defines evaluated; remaining unknown macros listed.

## Rajoitukset / Limitations

- Leksikaalinen: ei esikääntäjää; otsikoiden järjestystä, käytön jälkeen määriteltyjä makroja
  eikä funktiomakroja mallinneta (funktiomakro → tuntematon). / lexical, no preprocessor.
- Otsikon makro koskee koko projektia, jos otsikko on sulkeumassa (yliarvio). / header macros
  apply project-wide.
- Merkit ovat säännöllisiä lausekkeita: `send(`, `select(`, `MYSQL` voivat osua muuhun; agentit
  lukevat koodin. / regex markers can hit unrelated identifiers.
- Otsikko-osumat arvioidaan tuotteen omilla makroilla; kirjastojen omat määritykset vain
  kirjastojen tiedostoille. / headers evaluated with the product's macros.
- Tiedostot, joita mikään tuote ei käännä eikä sisällytä, puuttuvat `tiedostot.csv`:stä
  (31 kpl, luettelo `../raportit/osaA-luokittelu.md` luku 5). Niistä riippuvuusmerkkejä on
  neljässä: `Tp/TpComY32.cpp` (T-011), `dbboxm/Dbbox.cpp`, `dbboxm/BOXTEST_F.cpp`,
  `tputilv2/UnitTest.cpp` (`tarkistus_kattavuus.out` (c)). / 31 files are built or included by
  no product (listed in the part A report); four of them carry dependency markers.
