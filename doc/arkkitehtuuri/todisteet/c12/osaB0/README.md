# C-12 osa B0: asetusjäsentimien merkkijonovertailut / Part B0: string comparisons in the configuration parsers

Tuotettu 2026-11-26 revisiosta `ab10f88` skriptillä `../../../tyokalut/asetusavaimet.py`, joka
käyttää C-10:n mallia ehtojen ja tuotekohtaisen voimassaolon arviointiin. Raakatulos on osan B
syöte; se ei päätä, mitkä vertailut ovat asetusavaimia.

Produced on 2026-11-26 from revision `ab10f88` with `tyokalut/asetusavaimet.py` (C-10 model for
conditions). Raw input for part B; it does not decide which comparisons are keys.

Toisto / reproduce: `python -I doc/arkkitehtuuri/tyokalut/asetusavaimet.py . doc/arkkitehtuuri/todisteet/c12/osaB0`

Jäsentimet / parsers: `Hk/HkInit.cpp`, `Juk/VInit.cpp`, `cbHk/UnitCfg.cpp`, `ViestiWin/UnitCfg.cpp`.

| Tiedosto / file | Sisältö / content |
|---|---|
| `vertailut.csv` | 957 vertailua `muuttuja` vs. literaali: funktio, vertailufunktio, verrattu pituus `n` ja sen suhde literaalin pituuteen (`<` = vain etuliite, `>` = loppunolla mukana tai yli), `#if`-ehto, voimassaolo tuotteittain / comparisons with compared length and per-product activity |
| `peittoehdokkaat.csv` | 239 etuliitepeiton ehdokasta: aiempi vertailu, jonka verrattu etuliite on myöhemmän literaalin etuliite samassa funktiossa ja muuttujassa / prefix-shadowing candidates |
| `kommentoidut.csv` | 166 vain kommenteissa olevaa vertailua (käytöstä poistetut avaimet), joista 150 `cbHk/UnitCfg.cpp`:ssä / comparisons only in comments |
| `yhteenveto.txt` | määrät jäsentimittäin, muuttujittain ja funktioittain / counts |

Tarkistus / check: vertailurivejä verrattiin raakahakuun (`grep` vertailufunktioista). Erot
selittyvät kokonaan kahdella tavalla: kommentoidut rivit, jotka kirjataan erikseen
`kommentoidut.csv`:hen, ja vertailut, joissa literaali on ensimmäinen argumentti
(`wmemcmpU(L"ikkunat.xml", US.c_str(), 12)`, `UnitCfg.cpp:241`), joten ne eivät ole avainten
jäsennystä. / Differences from a raw grep are fully explained by commented-out lines (listed
separately) and literal-first comparisons that are not key parsing.

Rajoitukset / limits: funktioraja tunnistetaan sarakkeesta 0 alkavasta määrittelystä;
peittoehdokas ei vielä tarkoita peittoa: ratkaisee, ovatko vertailut samassa `if/else if`
-ketjussa vai päättyykö aiempi haara `continue`-lauseeseen (osa B). / A candidate is not yet a
shadowed key; part B decides.
