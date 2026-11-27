# C-5 vaihe 2: raakatulokset / Step 2 raw results

Tuotettu 2026-11-25 revisiosta `b17d536` (`TPsource/` ennallaan revisiosta `f7fe553`).
Tulkinta: `../../../kaantajaerot.md` luku 13. Arkistoissa ovat koeohjelmat, skriptit ja
tekstitulosteet muokkaamattomina; binäärit, lähdekoodikopiot (`src/`), yksikkökohtaiset
asm-listaukset (`all_hk/`, `all_v/`) ja yli 1 Mt:n tiedostot jätettiin pois, mutta
kaikkien tiedostojen tiivisteet ovat tiedostossa `raaka-manifest.sha256` (4 327 tiedostoa).

Produced on 2026-11-25 from revision `b17d536`. Interpretation: `kaantajaerot.md`
section 13. Archives hold the unmodified probes, scripts and text outputs; binaries,
source copies, per-unit asm listings and files over 1 MB were left out, but every file
is hashed in `raaka-manifest.sha256`.

| Tiedosto / file | Sisältö / content |
|---|---|
| `omf.tgz` | 2A (pääistunto / main session): `bcc_cmp.py`, `omf_cmp.py` (ensimmäinen versio / first version), `sens.py`, `pragma_test.py`, käännöstulosteet / compiler output |
| `run.txt`, `vertaa_omf_c5.out` | 2A: ensimmäisen työkaluversion tulokset (ennen tarkastusta) / results with the first tool version |
| `vertaa_omf_c5_fixed.out`, `pragma_test.out` | 2A: korjatun `../../../tyokalut/vertaa_omf.py`:n tulokset (rebuild 0/10, kommentit 1/10, koodi 10/10, `ct[9]`, pragmat) / results with the fixed tool |
| `dca.tgz` | 2B–2C (`defect-concurrency-auditor`): `t2b_raw_io_gui.csv`, `t2c_a_swprintf_sites.csv`, `t2c_a_step1_list_noncount.csv`, `t2c_b_reviewed_sites.csv`, `atoi_index_candidates.csv` |
| `lea.tgz` | 2D (`legacy-encoding-analyst`): koeohjelmat ja tulosteet / probes and outputs `q57/`, `q58/`, `q59/`, `q61/`, `q62/` (vertailuskriptit / comparison scripts `runall.py`, `cmp.py`, `odr2.py`) |
| `ver.tgz` | Riippumaton tarkastus / independent review: `exp1.py`, `exp2.py` (FB-alalajit / sub-types), `sw/`, `q57/`, `q59/` |
| `raaka-manifest.sha256` | Kaikkien väliaikaistiedostojen tiivisteet / hashes of all scratch files |
| `SHA256SUMS` | Tämän hakemiston tiedostojen tiivisteet / hashes of the files here |

Tarkistus / verify: `cd doc/arkkitehtuuri/todisteet/c5/vaihe2 && sha256sum -c SHA256SUMS`.
