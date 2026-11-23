# Inventaario / Inventory

Polut ovat suhteessa hakemistoon `TPsource/V52/`. Luvut koskevat commitia `f7fe553`.
Paths are relative to `TPsource/V52/`. Figures refer to commit `f7fe553`.

## Miten tuotettu / How it was produced

Luvut on tuotettu skriptillä `tyokalut/inventaario.py`, joka lukee tiedostot ja
projektitiedostot muuttamatta mitään:

```sh
python -I doc/arkkitehtuuri/tyokalut/inventaario.py . inventaario.json
```

Lähdetiedostoja ovat `.c`, `.cpp`, `.h`, `.hpp`; `.dfm`-lomaketiedostot lasketaan
erikseen. Kommenttiriviksi lasketaan rivi, joka alkaa `//`, `/*` tai `*`
(likiarvo). Merkistö päätellään tavuista: pelkkä ASCII, kelvollinen UTF-8 (BOMilla
tai ilman), "sekoitettu" (sekä UTF-8- että cp1252-tavuja) tai muuten cp1252.

The figures were produced with `tyokalut/inventaario.py`, which reads files and
project files without changing anything (command above). Source files are `.c`,
`.cpp`, `.h`, `.hpp`; `.dfm` form files are counted separately. A comment line is
a line starting with `//`, `/*` or `*` (approximation). Encoding is inferred from
bytes: pure ASCII, valid UTF-8 (with or without BOM), "mixed" (both UTF-8 and
cp1252 bytes) or otherwise cp1252.

## Hakemistot / Directories

| Hakemisto / Directory | Tiedostot / Files | Rivit / Lines | Kommenttirivit / Comment lines | % | ASCII | cp1252 | UTF-8 | UTF-8 BOM | sekoitettu / mixed | .dfm |
|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| `Hk` | 27 | 46 032 | 1 749 | 4 | 4 | 19 | 2 | 2 | 0 | 0 |
| `HkConsole` | 8 | 9 605 | 295 | 3 | 2 | 6 | 0 | 0 | 0 | 0 |
| `Juk` | 23 | 38 605 | 1 527 | 4 | 3 | 19 | 1 | 0 | 0 | 0 |
| `JukConsole` | 10 | 8 271 | 318 | 4 | 1 | 8 | 1 | 0 | 0 | 0 |
| `RADStudio10` | 2 | 436 | 37 | 8 | 2 | 0 | 0 | 0 | 0 | 0 |
| `Tests` | 5 | 7 705 | 662 | 9 | 5 | 0 | 0 | 0 | 0 | 0 |
| `Tp` | 13 | 11 704 | 753 | 6 | 6 | 4 | 3 | 0 | 0 | 0 |
| `VS` | 1 | 45 | 12 | 27 | 1 | 0 | 0 | 0 | 0 | 0 |
| `ViestiWin` | 114 | 43 424 | 3 686 | 8 | 72 | 42 | 0 | 0 | 0 | 54 |
| `cbHk` | 149 | 67 530 | 5 183 | 8 | 94 | 55 | 0 | 0 | 0 | 71 |
| `cbTp` | 73 | 15 115 | 1 878 | 12 | 55 | 18 | 0 | 0 | 0 | 35 |
| `com` | 3 | 1 807 | 223 | 12 | 1 | 2 | 0 | 0 | 0 | 0 |
| `dbboxm` | 13 | 2 375 | 149 | 6 | 3 | 1 | 9 | 0 | 0 | 0 |
| `include` | 21 | 1 883 | 317 | 17 | 18 | 1 | 2 | 0 | 0 | 0 |
| `tputilv2` | 169 | 15 513 | 3 520 | 23 | 8 | 29 | 130 | 0 | 2 | 1 |
| **Yhteensä / Total** | 631 | 270 050 | 20 309 | 8 | 275 | 204 | 148 | 2 | 2 | 161 |

`Tests/doctest.h` (noin 7 000 riviä) on ulkoinen kirjasto. Sen ja GUI-lomakkeiden
generoidun koodin jälkeen omaa koodia on noin 260 000 riviä. [päätelty]

`Tests/doctest.h` (about 7,000 lines) is a third-party library. Excluding it and
generated GUI form code, own code is roughly 260,000 lines. [inferred]

## Projektit / Projects

Lähdetiedostot (`.c`/`.cpp`) hakemistoittain. [todennettu: projektitiedostot]
Source files (`.c`/`.cpp`) per directory. [verified: project files]

| Projekti / Project | Tyyppi / Type | Lähteet hakemistoittain / Sources by directory |
|---|---|---|
| `RADStudio10/HkKisaWin.cbproj` | VCL-sovellus / application | `cbHk` 76, `cbTp` 36, `Hk` 19, `Tp` 6, `com` 3, `RADStudio10` 1 |
| `RADStudio10/ViestiWin.cbproj` | VCL-sovellus / application | `ViestiWin` 57, `cbTp` 32, `Juk` 18, `Tp` 6, `com` 3, `RADStudio10` 1 |
| `RADStudio10/Tputil-XE.cbproj` | staattinen kirjasto / static library | `tputilv2` 139 |
| `RADStudio10/DBboxm-XE.cbproj` | staattinen kirjasto / static library | `dbboxm` 8 |
| `VS/Hk/HkMaali/HkMaali.vcxproj` | konsoli / console | `Hk` 17, `HkConsole` 6, `Tp` 5, `com` 3, `tputilv2` 1 |
| `VS/V/JukMaali520.vcxproj` | konsoli / console | `Juk` 18, `JukConsole` 7, `Tp` 5, `com` 3, `tputilv2` 1 |
| `VS/V/JukLuenta520.vcxproj` | konsoli / console | `Juk` 16, `JukConsole` 7, `Tp` 5, `com` 2, `tputilv2` 2 |
| `VS/Libs/tputilv2.vcxproj` | staattinen kirjasto / static library | `tputilv2` 146 |
| `VS/Libs/dbboxm/dbboxm.vcxproj` | staattinen kirjasto / static library | `dbboxm` 7 |
| `VS/Tests/TpTest.vcxproj` | testit / tests | `Tests` 3, `Hk` 1, `Juk` 1, `tputilv2` 2 |
| `tputilv2/LibTest.cbproj` | VCL-testiohjelma / test app | `tputilv2` 3 [päätelty: vanha, ei käytössä / old, unused] |

Ratkaisut / Solutions: `VS/Libs/tputilv2.sln` (tputilv2 + dbboxm),
`VS/Hk/HkMaali520.sln`, `VS/V/JukMaali520.sln` (JukMaali + JukLuenta),
`VS/Tests/TpTest.sln`; C++Builder: `RADStudio10/Libs.groupproj`.

## Esikääntäjän ominaisuusvalitsimet / Preprocessor feature switches

Valitsimet poimittiin kaikista projektin konfiguraatioista yhteensä, joten
yksittäisessä konfiguraatiossa voi olla vähemmän. Osa samasta valitsimesta on
eri arvoilla eri konfiguraatioissa (esim. `MAXOSUUSLUKU=30` ja `=250`).

The switches were collected from all configurations of a project combined, so a
single configuration may have fewer. Some appear with different values in
different configurations (e.g. `MAXOSUUSLUKU=30` and `=250`).

| Projekti / Project | Valitsimet / Switches |
|---|---|
| HkKisaWin | `AUTOVA BIT32 DBGFILE DEMO EMIT EMITFILE EMITHTTP JOUKKUETUL KONETUNN LAJUNEN MAALI MERI5 MILLISEKAJAT N_PV=2 SEURAVAL STRICT TCPLUKIJA TCPSIIRTO VALUKIJA VALUKU=200 VERSIONO=510 WIN32 _BORLAND_` |
| ViestiWin | `ALGETAULU AMPU AUTOFILE BIT32 DEMO EMIT EMITAG EMITHTTP GAZ HAJONTA=8 ISODATA JUKOHJ JUKOLA KONETUNN LAJUNEN MAALI MAXOSUUSLUKU MAXSARJALUKU MAX_LAHPORTTI=64 PARTIO PROFILOI STRICT TCPLUKIJA TCPSIIRTO VAIKALUKU VALUKIJA VAMAALI WIN32` |
| HkMaali | `_CONSOLE AIKAVERT AUTOFILE AUTOVA BIT32 DBGFILE DEMO EMIT EMITFILE EMITHTTP ENNAKOI GAZ KONETUNN LAJUNEN MAAKNT MAALI MAX_LAHPORTTI=64 NREGNLY ORAVA PISTEET SAKKOCOM SERVER SEURAVAL TCPLUKIJA TCPSIIRTO UUSI VALUKIJA VALUKU=200 VERSIONO=510 VERTAA WIN32` |
| JukMaali | `_CONSOLE ALGETAULU AUTOFILE BIT32 DBGFILE EMIT EMITAG EMITFILE EMITHTTP ENNAKOI ESITARK GAZ HAJONTA=8 KONETUNN LAJUNEN MAALI MAXOSUUSLUKU MAXSARJALUKU MAX_LAHPORTTI=64 NREGNLY=8 PARTIO PROFILOI SERVER TCPLUKIJA TCPSIIRTO TESTIAJAT VAIKALUKU VALUKIJA WIN32` |
| JukLuenta | `_CONSOLE AUTOFILE BIT32 DBGFILE EMIT HAJONTA=8 KONETUNN LUENTA MAALI MAXOSUUSLUKU=30 MAXSARJALUKU=250 MAX_LAHPORTTI=64 NREGNLY=8 PARTIO PROFILOI SERVER TCPLUKIJA TCPSIIRTO VAIKALUKU=12 WIN32` |
| tputilv2 (MSVC) | `_CONSOLE CONSOLELIB GDIPRINTER UNICODE _UNICODE WIN32 _LIB` + `/J` (char unsigned) |
| Tputil-XE (C++Builder) | `_BORLAND_ _MBCS _UNICODE` |

Taulukko kattaa vain projektitiedostojen valitsimet. Otsikkotiedostot lisäävät
omiaan: esimerkiksi `Hk/HkDef.h` määrittelee aina `MILLISEKAJAT`, `MERI5` ja
`JOUKKUEPISTEET`, lisää `AUTOFILE`n kun `TCPSIIRTO` on päällä, ja määrittelee
`SAKKOCOM` (`Hk/HkDef.h:26-130`). Lopullinen joukko selviää siis vasta
esikääntäjän tuloksesta.

Konsoli-HK:n projektissa on `GAZ`, `PISTEET`, `ORAVA`, `VERTAA`, `AIKAVERT` ja
`ENNAKOI`, joita GUI-HK:n projektissa tai otsikoissa ei ole; GUI-HK:ssa on
`JOUKKUETUL`. Valitsinten merkitykset on selvitettävä (`seuranta.md`).
[todennettu: valitsimet projekteissa ja `grep '#define'` otsikoissa; tuntematon:
merkitykset]

The table covers project-file switches only. Headers add their own: for example
`Hk/HkDef.h` always defines `MILLISEKAJAT`, `MERI5` and `JOUKKUEPISTEET`, adds
`AUTOFILE` when `TCPSIIRTO` is on, and defines `SAKKOCOM` (`Hk/HkDef.h:26-130`).
The final set is therefore only known from the preprocessor output.

The console HK project has `GAZ`, `PISTEET`, `ORAVA`, `VERTAA`, `AIKAVERT` and
`ENNAKOI`, which neither the GUI HK project nor the headers define; GUI HK has
`JOUKKUETUL`. The meaning of each switch must be established (`seuranta.md`).
[verified: switches in projects and `grep '#define'` in headers; unknown:
meanings]

## Käynnistyskohdat / Entry points

| Tiedosto / File | Funktio / Function | Projekti / Project |
|---|---|---|
| `HkConsole/HkMain.cpp:178` | `wmain` | HkMaali |
| `JukConsole/laskenta.cpp:97` | `wmain` | JukMaali, JukLuenta |
| `RADStudio10/HkKisaWin.cpp:131` | `_tWinMain` | HkKisaWin |
| `RADStudio10/ViestiWin.cpp:110` | `wWinMain` | ViestiWin |
| `tputilv2/ctrlhndl.cpp:40`, `esnimet.cpp:85`, `vaihdanm.cpp:83`, `esnimet-2.cpp:60` | `main` | tputilv2 (kirjastossa! / in the library) |
| `tputilv2/LibTest.cpp:37` | `_tWinMain` | LibTest |
| `Tp/mttest.cpp:22`, `dbboxm/Boxtest.cpp:65`, `dbboxm/BOXTEST_F.cpp:66` | `main` | – (ei projektissa / no project) |

Kirjastoon käännetyt `main`-funktiot ovat todennäköisesti testi- tai apuohjelmia,
jotka on suojattu esikääntäjällä. Asia on tarkistettava. [tuntematon]

The `main` functions compiled into the library are probably test or helper
programs guarded by the preprocessor. This must be checked. [unknown]

## Orvot tiedostot / Orphan files

Seuraavat tiedostot eivät kuulu mihinkään projektiin, eikä mikään tiedosto
ota niitä mukaan `#include`-direktiivillä. Ne ovat ehdokkaita vanhentuneeksi
koodiksi, mutta poistoa ei saa tehdä ilman tarkistusta: osa voi olla käytössä
muissa haaroissa tai ulkoisissa käännöksissä. [päätelty]

The following files belong to no project, and no file includes them with
`#include`. They are candidates for obsolete code, but must not be removed
without checking: some may be used by other branches or external builds.
[inferred]

- `Hk/HkApuC.c`, `Hk/HkTV.cpp` (sisältää `opentv`, jota `Hk/HkInit.cpp` kutsuu / contains `opentv`, called by `Hk/HkInit.cpp` → todennäköisesti esikääntäjällä pois / likely excluded by the preprocessor)
- `JukConsole/lasknum5u.cpp`
- `Tp/TpApuC.c`, `Tp/TpComY32.cpp` (vanha `TpComY.cpp`:n kopio / old copy of `TpComY.cpp`), `Tp/TpMuot.cpp`, `Tp/mttest.cpp`
- `VS/Libs/MbsWcs.cpp`, `ViestiWin/VWinApu.cpp`, `cbTp/TpApu.cpp`
- `dbboxm/Boxtest.cpp`, `dbboxm/BOXTEST_F.cpp`, `dbboxm/Dbbox.cpp`, `dbboxm/DBINTF.H`
- `include/sitypes.h`
- `tputilv2/`: `TextFileClass-2.cpp`, `Utf8Conv.cpp`, `aakjarjs-2.cpp`, `aakjarjs-v.cpp`, `aakjarjs2-2.cpp`, `aakjarjs2-v.cpp`, `aakjarjws-2.cpp`, `aakjarjws-v.cpp`, `aakjarjws2-2.cpp`, `aakjarjws2-v.cpp`, `c-files/ioerr.c`, `globals.cpp`, `haesarja.cpp`, `haesarja_w.cpp`, `kbflush.cpp`, `scbox.cpp`, `testucode.cpp`

## Muut hakemistot / Other directories

- `doc/`: kehittäjäanalyysit (Zebra FX9600, rinnakkaisosuudet) / developer analyses
- `docs-site/`: käyttöohje, MkDocs, julkaistaan GitHub Pagesiin (`.github/workflows/docs.yml`) / user manual
- `.github/workflows/`: `build.yml` (konsoli MSVC v143 + GUI self-hosted), `release.yml` (`v*`-tagit / tags), `docs.yml` (käsin / manual)
- `TPexe/`, `vc10/`: käännöstulokset, eivät versionhallinnassa / build output, not in version control
