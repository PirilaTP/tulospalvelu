# Moduulien kattavuus / Module coverage

Revisio / revision: `f7fe553`. Tiedostokohtaiset tiedot: `tiedostot.csv`.
Per-file data: `tiedostot.csv`.

Sarakkeet / columns: **Taso 2** = kohdennetusti luettuja tiedostoja / files read in a
targeted way (tasot / levels: `README.md`); **Arkkitehtuuri** = kuvattu dokumenteissa /
described in documents; **Riippuvuudet** = include/linkitys selvitetty / include and
linking analysed; **Todennus** = riippumaton tarkastus / independent review.

| Moduuli / Module | Tiedostot / Files | Taso 2 / Level 2 | Arkkitehtuuri / Architecture | Riippuvuudet / Dependencies | Todennus / Verification |
|---|--:|--:|---|---|---|
| `Hk/` | 27 | 14 | yleiskuva, päävirta, `tallenna`-samanaikaisuus / main flow, `tallenna` concurrency | include-kaavio / include graph | erä 1 tarkastettu / batch 1 reviewed |
| `Juk/` | 23 | 9 | päätelty HK:sta; `tallenna`-lukitus / inferred from HK; `tallenna` locking | include-kaavio | erä 1: lukitus tarkastettu / batch 1: locking reviewed |
| `Tp/` | 13 | 3 | säikeet, jonot / threads, queues | include-kaavio | erä 1 tarkastettu |
| `com/` | 3 | 0 | luettelo, säikeet / listed, threads | include-kaavio | ei / no |
| `cbHk/` | 149 | 8 | käynnistys, säikeet, logiikkalomakkeet tunnistettu / startup, threads, forms with logic identified | include-kaavio | erä 1 osittain / batch 1 partly |
| `ViestiWin/` | 114 | 0 | – | include-kaavio | ei |
| `cbTp/` | 73 | 2 | `writeerror_w`, `haeSQLTh` | include-kaavio | erä 1 tarkastettu |
| `HkConsole/` | 8 | 2 | käynnistys / startup | include-kaavio | ei |
| `JukConsole/` | 10 | 0 | – | include-kaavio | ei |
| `tputilv2/` | 169 | 6 | merkistöt, `dbindex` / encodings | include-kaavio | objektivertailu testattu / object comparison tested |
| `include/` | 21 | 4 | yleiskuva | include-kaavio | ei |
| `dbboxm/` | 13 | 1 | `getrec`/`putrec` | include-kaavio | erä 1 tarkastettu |
| `Tests/` | 5 | 0 | riippuvuudet; ajettu 46/46 / run 46/46 | – | vahvistettu ajamalla / confirmed by running |
| `RADStudio10/`, `VS/` | 3 | 1 | inventaario | projektitiedostot / project files | ei |

Yhteenveto / summary: 50 / 631 tiedostoa luettu kohdennetusti (taso 2), 581 vain
inventoitu (taso 1). Yhtäkään tiedostoa ei ole luettu kokonaan rakenteen tasolla
(taso 3) eikä katselmoitu (taso 4).

50 of 631 files read in a targeted way (level 2), 581 inventoried only (level 1).
No file has been read at structure level (3) or reviewed (4).
