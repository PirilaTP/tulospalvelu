# C-6 vaihe 1: säikeiden ja lukkojen raakatulokset / Threads and locks raw results

Tuotettu 2026-11-25 revisiosta `379640e` (`TPsource/` ennallaan revisiosta `f7fe553`).
Tulkinta: `../../saikeet-ja-lukot.md`. Tiedostot ovat pääistunnon ja `defect-concurrency-auditor`-
agentin tuottamia sellaisinaan; niitä ei ole muokattu. Väliaikainen `scan.pkl` (5 Mt) jätettiin
pois; sen tiiviste on tiedostossa `raaka-manifest.sha256`, ja sen voi tuottaa uudelleen
skriptillä `lukkoskriptit/scan.py`.

Produced on 2026-11-25 from revision `379640e` (`TPsource/` unchanged since `f7fe553`).
Interpretation: `../../saikeet-ja-lukot.md`. Files are unmodified outputs of the main session and
the `defect-concurrency-auditor` agent. The 5 MB `scan.pkl` intermediate was left out; its hash is
in `raaka-manifest.sha256` and `lukkoskriptit/scan.py` regenerates it.

| Tiedosto / file | Sisältö / content |
|---|---|
| `syotteet/thr_prod_proj.txt` | Tuotannon `_beginthread`/`CreateThread`-rivit (67) projektijäsenyyksineen (`[projects: -]` = ei käännetä) / production thread start lines with build membership |
| `syotteet/cs_prod_proj.txt` | Tuotannon `Enter/Leave/TryEnter/Initialize/DeleteCriticalSection`-osumat (1 056, 90 tiedostoa) projektijäsenyyksineen / production critical-section hits with build membership |
| `lukkoskriptit/` | Lukkoagentin kertakäyttöskriptit (`scan.py` sisäkkäiset parit, `reg.py` lukitut alueet, `blk.py` estävät kutsut, `unbal.py` vapauttamattomat polut, `cnt.py` laskenta) ja tulosteet (`live.txt` elävät osumat ilman kommentteja, `blk_*.txt`, `reg_*.txt`, `*0.txt`) / throwaway scripts and outputs of the lock agent |
| `raaka-manifest.sha256` | Kaikkien väliaikaistiedostojen tiivisteet (myös pois jätetyn) / hashes of all scratch files, including the excluded one |
| `SHA256SUMS` | Tämän hakemiston tiedostojen tiivisteet / hashes of the files here |

Syötteiden tuotanto / how the inputs were produced (Git Bash, `TPsource/V52`):

```sh
grep -rnoE "(Enter|Leave|TryEnter|Initialize|Delete)CriticalSection(AndSpinCount)?\s*\(\s*&?\s*[A-Za-z_][A-Za-z_0-9]*" \
  --include=*.cpp --include=*.h --include=*.c . | grep -v "^./Tests" > cs_prod.txt
grep -rnE "_beginthread(ex)?\s*\(|CreateThread\s*\(" --include=*.cpp --include=*.h . | grep -v "^./Tests" > thr_prod.txt
```

Projektijäsenyys lisättiin sarakkeesta `projektit` (`../../rekisteri/tiedostot.csv`). Skriptit
kattavat yhden kutsutason; jokainen raportoitu reuna vahvistettiin lukemalla.
Build membership was added from the `projektit` column. The scripts cover one call level; every
reported edge was confirmed by reading.
