# C-9 vaihe 1: virheenkäsittelyn ja resurssien raakatulokset / error handling and resources raw results

Tuotettu 2026-11-26 revisiosta `7aaca40` (tuotantolähdekoodi ennallaan revisiosta `f7fe553`).
Tulkinta: `../../virheenkasittely.md`. Raportit ovat agenttien loppuraportteja sellaisinaan:
pääistunto kopioi ne palautusviesteistä muokkaamatta sisältöä ja lisäsi vain alkuperää kertovan
rivin. N-tunnukset ovat väliaikaisia; huom. C-8:ssa käytettiin samoja N-C-tunnuksia eri asioille.
Rekisterin H-tunnukset: `../../virheenkasittely.md` luku 7.

Produced 2026-11-26 from revision `7aaca40` (production sources unchanged since `f7fe553`).
Interpretation: `../../virheenkasittely.md`. Reports are the agents' final reports, copied
unmodified by the main session (one provenance line added). N IDs are provisional (C-8 used the
same N-C IDs for other items); mapping to H IDs in section 7 of the interpretation.

| Tiedosto / file | Tekijä / by | Sisältö / content |
|---|---|---|
| `osaA/kutsut.csv` | `tyokalut/resurssit.py` v1 | kutsukohdat ja paluuarvon käyttö (agenttien syöte) / call sites and return-value use (agents' input) |
| `osaA/writeerror.csv` | sama / same | mitä `writeerror`-kutsua seuraa / what follows each `writeerror` |
| `osaA/funktiot.csv`, `osaA/yhteenveto.txt` | sama / same | funktiokohtaiset määrät, yhteenveto / per-function counts, summary |
| `osaA/v2/` | `tyokalut/resurssit.py` v2 | samat tiedostot korjatulla skriptillä (S1–S3, S5–S8) ja `built`-sarakkeella / same files from the fixed script, with a `built` column |
| `osaA-luokittelu.md` | `architecture-analyst`, Sonnet | osa A: skriptin validointi, ajovaiheet, pysyvän datan kirjoittajat W1–W34, kohdelistat B/C/D / part A |
| `osaB-pysyva-data.md` | `defect-concurrency-auditor`, Sonnet | osa B: N-B1–N-B11 / part B |
| `osaC-elinkaari.md` | `defect-concurrency-auditor`, Sonnet | osa C: N-C1–N-C8 / part C |
| `osaD-virheen-jalkeen.md` | `defect-concurrency-auditor`, Sonnet | osa D: N-D1–N-D8, lopetuspolut / part D |
| `osaE-tarkastus.md` | `independent-verifier`, Opus | riippumaton tarkastus / independent review |
| `SHA256SUMS` | – | tiivisteet / hashes |

CSV-tiedostot voi tuottaa uudelleen / CSVs can be regenerated: `python -I
doc/arkkitehtuuri/tyokalut/resurssit.py . <hakemisto/dir>` repon juuresta / from the repository
root. Nykyinen skripti tuottaa v2:n; v1:n tuottanut versio on v2:n otsakkeen kuvaamia korjauksia
vailla. / The current script produces v2; v1 came from the version without the fixes listed in
its docstring.

Mitään toimitettua ohjelmaa ei käynnistetty, eikä `TPsource/`-puuhun tehty muutoksia. / No shipped
program was started and nothing under `TPsource/` was changed.
