# C-10 vaihe 1: ulkoisten riippuvuuksien raakatulokset / external dependencies raw results

Tuotettu 2026-11-26 revisiosta `4b48749` (`TPsource/` ennallaan revisiosta `f7fe553`).
Tulkinta: `../../riippuvuudet.md` luvut 7–18. Raportit ovat agenttien loppuraportteja: pääistunto
tallensi ne palautusviesteistä; väitteet, näyttö ja tunnisteet ovat ennallaan, sanamuotoa ja
asettelua on tiivistetty, ja alkuun on lisätty alkuperärivi. Agenttien tunnisteet (B: D1–D13,
C: T1–T13, D: N-1–N-16, E: N-1–N-11, A: B1–B13, F: K1–K15, E1–E3) ovat väliaikaisia ja
osittain päällekkäisiä; vastaavuus rekisterin tunnuksiin: `../../riippuvuudet.md` luku 16 ja
`../../rekisteri/havainnot.md`.

Produced 2026-11-26 from revision `4b48749` (`TPsource/` unchanged since `f7fe553`).
Interpretation: `riippuvuudet.md` sections 7–18. Reports are the agents' final reports saved by the
main session; claims, evidence and labels unchanged, wording and layout condensed. Agent labels are
provisional and overlap between parts; mapping to register IDs in section 16.

| Tiedosto / file | Tekijä / by | Sisältö / content |
|---|---|---|
| `osaA/` | `tyokalut/riippuvuudet.py`, pääistunto / main session | raakainventaario ja sen tarkistus ennen agenttikäyttöä (`osaA/README.md`) / raw inventory and its validation |
| `raportit/osaA-luokittelu.md` | `build-dependency-analyst`, Sonnet | tuotekohtainen inventaario, valitsimet, API-alueet, versiot, korjaukset / per-product inventory |
| `raportit/osaB-tietokanta.md` | `architecture-analyst`, Sonnet | MySQL/dbExpress, Q5 |
| `raportit/osaC-http-selain.md` | `defect-concurrency-auditor`, Sonnet | WinHTTP, `TWebBrowser` |
| `raportit/osaD-laitteet.md` | `architecture-analyst`, Sonnet | laitteet ja protokollat, Q86 / devices, protocols |
| `raportit/osaE-paketit-jakelu.md` | `build-dependency-analyst`, Sonnet | BPL/DLL, jakelu, Q91, Q92, T-005 / packages, deployment |
| `osaE/*.py` | osan E agentti / part E agent | vain lukeva PE-tuontitaulujen jäsennin ja apuskriptit / read-only PE import parser and helpers |
| `raportit/osaF-tarkastus.md` | `independent-verifier`, Opus | riippumaton tarkastus K1–K15, E1–E3 / independent review |
| `SHA256SUMS` | – | tiivisteet / hashes |

Toisto / reproduce: `python -I doc/arkkitehtuuri/tyokalut/riippuvuudet.py . <hakemisto/dir>`
repon juuresta / from the repository root. `osaA/tarkistus_c5.py` tarvitsee lisäksi C-5:n
`bda.tgz`-arkiston puretun `bda/`-hakemiston / also needs the extracted `bda/` of
`../c5/bda.tgz`. PE-jäsentimet lukevat paikallisesti käännettyjä, versionhallinnan ulkopuolisia
`TPexe/`-ohjelmia (2026-11-24) / the PE parsers read the untracked local `TPexe/` builds.
Mitään ei ajettu eikä yhteyksiä muodostettu. / Nothing was run and no connections were made.
