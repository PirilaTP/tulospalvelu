# C-12 vaihe 1: ominaisuusvalitsimien ja asetusten raakatulokset / feature switch and settings raw results

Tuotettu 2026-11-26 revisiosta `ab10f88` (`TPsource/` ennallaan revisiosta `f7fe553`).
Tulkinta: `../../ominaisuusvalitsimet.md`.

Raportit ovat agenttien loppuraportteja, jotka pääistunto tallensi palautusviesteistä. Väitteet,
näyttö ja tunnisteet ovat ennallaan; sanamuotoa ja asettelua on tiivistetty, ja alkuun on lisätty
alkuperärivi. Agenttien tunnisteet ovat väliaikaisia ja osittain päällekkäisiä:

- A: N-1–N-14
- B: N-1–N-12
- C: D1–D8
- D: A1–A7, B1–B7, C1–C5

Vastaavuus rekisterin tunnuksiin on luvussa 11 ja tiedostossa `../../rekisteri/havainnot.md`.

Produced 2026-11-26 from revision `ab10f88`. The reports are the agents' final reports, saved by the
main session; claims, evidence and labels are unchanged and the wording is condensed. Agent labels are
provisional; the mapping to register IDs is in section 11.

| Tiedosto / file | Tekijä / by | Sisältö / content |
|---|---|---|
| `osaA0/` | `tyokalut/valitsimet.py`, pääistunto / main session | esikääntäjän valitsinten raakainventaario ja sen tarkistus (`osaA0/README.md`) / raw switch inventory and validation |
| `osaB0/` | `tyokalut/asetusavaimet.py`, pääistunto | asetusjäsentimien vertailut, peittoehdokkaat, kommentoidut (`osaB0/README.md`) / parser comparisons |
| `raportit/osaA-esikaantajavalitsimet.md` | `build-dependency-analyst`, Sonnet | valitsinten luokittelu, Q2, T-008, riippuvuudet / classification |
| `raportit/osaB-asetukset-komentorivi.md` | `architecture-analyst`, Sonnet | asetusavaimet, editorit, komentorivi / config keys, editors, command line |
| `raportit/osaC-muut-kytkimet.md` | `architecture-analyst`, Sonnet | tiedoston olemassaolo, GUI-asetukset, ympäristö, kilpailuparametrit / other switches |
| `raportit/osaD-tarkastus.md` | `independent-verifier`, Opus | riippumaton tarkastus: 12 vahvistettu, 7 osittain, 0 kumottu / independent review |
| `SHA256SUMS` | – | tiivisteet / hashes |

Rekisterit `../../rekisteri/valitsimet.csv` ja `../../rekisteri/asetusavaimet.csv` tuotetaan
näistä skriptillä `../../tyokalut/valitsinrekisteri.py`. / Registers built from these.

Toisto repon juuresta / reproduce from the repository root:

```
python -I doc/arkkitehtuuri/tyokalut/valitsimet.py . doc/arkkitehtuuri/todisteet/c12/osaA0
python -I doc/arkkitehtuuri/tyokalut/asetusavaimet.py . doc/arkkitehtuuri/todisteet/c12/osaB0
python -I doc/arkkitehtuuri/tyokalut/valitsinrekisteri.py .
```
