# Moduulien kattavuus / Module coverage

Revisio / revision: `f7fe553`. Tiedostokohtaiset tiedot: `tiedostot.csv`.
Per-file data: `tiedostot.csv`.

Sarakkeet / columns: **T2 / T3** = kohdennetusti luettuja (taso 2) ja rakenteen tasolla
luettuja (taso 3) tiedostoja / files at level 2 and 3 (tasot / levels: `README.md`);
**Arkkitehtuuri** = kuvattu dokumenteissa / described in documents; **Todennus** =
riippumaton lähdetarkastus (ei ajonaikainen) / independent source review (not runtime).

| Moduuli / Module | Tiedostot / Files | T2 | T3 | Arkkitehtuuri / Architecture | Todennus / Verification |
|---|--:|--:|--:|---|---|
| `Hk/` | 27 | 16 | 1 | yleiskuva, `tallenna`, tiedonsiirto / overview, `tallenna`, communications | C-1, C-2: lähdetarkastus / source review |
| `Juk/` | 23 | 12 | 0 | vastaanotto, lukitus, `combufsize` / receive, locking | C-1 (lukitus / locking), C-2 |
| `Tp/` | 13 | 4 | 1 | säikeet, jonot, kehys, kättely / threads, queues, frame, handshake | C-1, C-2 |
| `com/` | 3 | 1 | 1 | kuljetus / transport (UDP, TCP, WinHTTP) | C-2 |
| `cbHk/` | 149 | 8 | 0 | käynnistys, säikeet, vaiheen vaihto / startup, threads, stage change | C-1 osittain / partly, C-2 osittain |
| `ViestiWin/` | 114 | 0 | 0 | – | ei / no |
| `cbTp/` | 73 | 3 | 0 | `writeerror_w`, `haeSQLTh`, uudelleenlähetys / resend | C-1, C-2 |
| `HkConsole/` | 8 | 2 | 0 | käynnistys, vaiheen vaihto / startup, stage change | C-2 |
| `JukConsole/` | 10 | 0 | 0 | – | ei / no |
| `tputilv2/` | 169 | 6 | 0 | merkistöt, `dbindex` / encodings | objektivertailu testattu / object comparison tested |
| `include/` | 21 | 5 | 1 | yleiskuva, tiedonsiirto / overview, communications | C-2 (`wincom.h`) |
| `dbboxm/` | 13 | 1 | 0 | `getrec`/`putrec` | C-1 |
| `Tests/` | 5 | 0 | 0 | riippuvuudet / dependencies | testiajo 46/46 (ajonaikainen) / test run 46/46 (runtime) |
| `RADStudio10/`, `VS/` | 3 | 1 | 0 | inventaario, käännösvalitsimet / inventory, build switches | ei / no |

Yhteenveto / summary: 631 tiedostoa; taso 1: 568, taso 2: 59, taso 3: 4
(`Tp/TpComY.cpp`, `Hk/HkCom32.cpp`, `com/WinUDP.cpp`, `include/wincom.h`), taso 4: 0.
Yhtäkään tiedostoa ei ole katselmoitu kokonaan vikojen varalta. GUI-viestiohjelma
(`ViestiWin/`) ja viestin konsoli (`JukConsole/`) ovat yhä vain inventoituja.

631 files; level 1: 568, level 2: 59, level 3: 4, level 4: 0. No file has been fully
reviewed for defects. The relay GUI (`ViestiWin/`) and relay console (`JukConsole/`)
are still inventoried only.
