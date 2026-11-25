# Moduulien kattavuus / Module coverage

Revisio / revision: `f7fe553`. Tiedostokohtaiset tiedot: `tiedostot.csv`.
Per-file data: `tiedostot.csv`.

Sarakkeet / columns: **T2 / T3** = kohdennetusti luettuja (taso 2) ja rakenteen tasolla
luettuja (taso 3) tiedostoja / files at level 2 and 3 (tasot / levels: `README.md`);
**Arkkitehtuuri** = kuvattu dokumenteissa / described in documents; **Todennus** =
riippumaton lähdetarkastus (ei ajonaikainen) / independent source review (not runtime).

| Moduuli / Module | Tiedostot / Files | T2 | T3 | Arkkitehtuuri / Architecture | Todennus / Verification |
|---|--:|--:|--:|---|---|
| `Hk/` | 27 | 19 | 2 | yleiskuva, `tallenna`, tiedonsiirto, tiedostomuodot / overview, `tallenna`, communications, file formats, säikeet ja lukot / threads and locks | C-1, C-2, C-3: lähdetarkastus / source review, C-6 |
| `Juk/` | 23 | 20 | 0 | vastaanotto, lukitus, `combufsize`, tietueen pakkaus / receive, locking, record packing, säikeet ja lukot / threads and locks | C-1 (lukitus / locking), C-2, C-3, C-6 |
| `Tp/` | 13 | 4 | 1 | säikeet, jonot, kehys, kättely / threads, queues, frame, handshake, säikeet ja lukot / threads and locks | C-1, C-2, C-6 |
| `com/` | 3 | 1 | 1 | kuljetus / transport (UDP, TCP, WinHTTP), säikeet ja lukot / threads and locks | C-2, C-6 |
| `cbHk/` | 149 | 18 | 4 | käynnistys, säikeet, vaiheen vaihto, vienti, tuonti, MySQL-jono / startup, threads, stage change, export, import, SQL queue, säikeet ja lukot, VCL-säieturvallisuus / threads and locks, VCL thread safety, GUI:n liiketoimintalogiikka / GUI business logic | C-1, C-2, C-3 osittain / partly, C-6, C-7 |
| `ViestiWin/` | 114 | 15 | 5 | vienti, tuonti, vanha muoto, MySQL-jono / export, import, old format, SQL queue, säikeet ja lukot, VCL-säieturvallisuus / threads and locks, VCL thread safety, GUI:n liiketoimintalogiikka / GUI business logic | C-3 osittain / partly, C-6, C-7 |
| `cbTp/` | 73 | 10 | 0 | `writeerror_w`, `haeSQLTh`, uudelleenlähetys / resend, säikeet ja lukot / threads and locks | C-1, C-2, C-6 |
| `HkConsole/` | 8 | 4 | 0 | käynnistys, vaiheen vaihto / startup, stage change, säikeet ja lukot / threads and locks | C-2, C-6 |
| `JukConsole/` | 10 | 5 | 0 | säikeet ja lukot / threads and locks | C-4: varoitusten triage / warning triage, C-6 |
| `tputilv2/` | 169 | 24 | 2 | merkistöt, `dbindex`, merkistömuunnokset, konfiguraation luku / encodings, conversions, config reading | objektivertailu testattu / object comparison tested; C-3 |
| `include/` | 21 | 4 | 3 | yleiskuva, tiedonsiirto, dbboxm-rakenteet, pragmat / overview, communications, dbboxm structs, pragmas | C-2 (`wincom.h`), C-3 (`boxr.h`, `boxm.h`, `tputil.h`) |
| `dbboxm/` | 13 | 5 | 2 | tiedostokerros, otsake, kuollut indeksikerros / file layer, header, dead index layer | C-1, C-3 |
| `Tests/` | 5 | 0 | 0 | riippuvuudet / dependencies | testiajo 46/46 (ajonaikainen) / test run 46/46 (runtime) |
| `RADStudio10/`, `VS/` | 3 | 1 | 0 | inventaario, käännösvalitsimet, toimitetut konfiguraatiot / inventory, build switches, shipped configurations | C-3: määritykset ja koot mitattu / defines and sizes measured |

Yhteenveto / summary (C-4): 631 tuotannon tiedostoa; taso 1: 499, taso 2: 120, taso 3: 12
(C-2: `Tp/TpComY.cpp`, `Hk/HkCom32.cpp`, `com/WinUDP.cpp`, `include/wincom.h`; C-3:
`dbboxm/dbbox1.cpp`, `dbboxm/dbbox2.cpp`, `include/boxr.h`, `include/boxm.h`,
`Hk/HkInitVanha.cpp`, `ViestiWin/ConvDat.cpp`, `tputilv2/oem2ansi.cpp`,
`tputilv2/MbsWcs.cpp`), taso 4: 0. Yhtäkään tiedostoa ei ole katselmoitu kokonaan
vikojen varalta. C-4 nosti tasolle 2 tiedostot, joiden varoituksia luokiteltiin;
niistä on luettu vain varoitusten kohdat. 19 C-11-testitiedostoa ovat tasolla 4.

631 production files; level 1: 499, level 2: 120, level 3: 12, level 4: 0. No file has
been fully reviewed for defects. C-4 raised files with triaged warnings to level 2;
only the warning sites were read. The 19 C-11 test files are at level 4.

C-7 (2026-11-25): taso 1: 477, taso 2: 134, taso 3: 20. Tasolle 3 nousivat kahdeksan
GUI-lomakeyksikköä (`cbHk/UnitEmit.cpp`, `UnitAjanotto.cpp`, `UnitArvonta.cpp`,
`UnitKilpailijatiedot.cpp`; `ViestiWin/UnitEmit.cpp`, `UnitAjanotto.cpp`, `UnitArvonta.cpp`,
`UnitJoukkuetiedot.cpp`); tasolle 2 `ViestiWin/OsuusTiedot.cpp` ja
`ViestiWin/UnitEmitMuutokset.cpp`. / C-7: level 1: 477, level 2: 134, level 3: 20; eight GUI
form units raised to level 3, two relay units to level 2.
