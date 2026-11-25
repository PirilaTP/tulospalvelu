# C-8 vaihe 1: globaalin muuttuvan tilan raakatulokset / global mutable state raw results

Tuotettu 2026-11-25–26 revisiosta `be52dbf` (tuotantolähdekoodi ennallaan revisiosta
`f7fe553`). Tulkinta: `../../globaali-tila.md`. Raportit ovat agenttien loppuraportteja
sellaisinaan. Osien B ja D agentit kirjoittivat tiedostonsa itse pääistunnon pyynnöstä. Osan A
raportin ja osien C ja E raportit pääistunto kopioi palautusviesteistä muokkaamatta sisältöä;
niihin lisättiin vain alkuperää kertova rivi. N-tunnukset ovat väliaikaisia; rekisterin
H-tunnukset: `../../globaali-tila.md` luku 7.

Produced 2026-11-25..26 from revision `be52dbf` (production sources unchanged since `f7fe553`).
Interpretation: `../../globaali-tila.md`. Reports are the agents' final reports; parts B and D
were written to file by the agents, part A, C and E copied unmodified by the main session (one
provenance line added). N IDs are provisional; mapping to H IDs in section 7 of the
interpretation.

| Tiedosto / file | Tekijä / by | Sisältö / content |
|---|---|---|
| `osaA/maarittelyt.csv` | `tyokalut/globaalit.py` | tiedostotason määrittelyt / file-level definitions (file, line, name, kind, declaration) |
| `osaA/staattiset_funktiossa.csv` | sama / same | funktion sisäiset staattiset / function-local statics |
| `osaA/viittaukset.csv` | sama / same | kaikki viittaukset funktioineen ja käyttötapoineen (W/A/R); `~funktio` = varamenetelmä / all references; `~` = fallback attribution |
| `osaA/kirjoittajat.csv` | sama / same | nimittäin kirjoittajafunktiot ja lukumäärät / writers per name |
| `osaA/yhteenveto.txt` | sama / same | määrät / counts |
| `osaA/luokittelu.csv` | `architecture-analyst`, Sonnet | luokat K1–K7, vaiheriippuvuus / categories, stage dependence (tehty ennen varamenetelmää / made before the fallback) |
| `osaA-luokittelu.md` | `architecture-analyst`, Sonnet | osan A raportti / part A report |
| `osaB-elinkaari.md` | `architecture-analyst`, Sonnet | osa B: alustus, nollaustaulukko, N-B1–N-B14 / part B |
| `osaC-eheys.md` | `defect-concurrency-auditor`, Sonnet | osa C: invariantit, N-C1–N-C15, L-1–L-7 / part C |
| `osaD-saikeet.md` | `defect-concurrency-auditor`, Sonnet | osa D: N-D1–N-D6, hylätyt / part D |
| `osaE-tarkastus.md` | `independent-verifier`, Opus | riippumaton tarkastus / independent review |
| `SHA256SUMS` | – | tiivisteet / hashes |

CSV-tiedostot voi tuottaa uudelleen / CSVs can be regenerated (except `luokittelu.csv`):
`python -I doc/arkkitehtuuri/tyokalut/globaalit.py . <hakemisto/dir>` repon juuresta / from the
repository root.
