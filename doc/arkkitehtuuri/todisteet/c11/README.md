# C-11: raakatulosten arkisto / Raw result archive

Arkistoitu 2026-11-24 ennen tilapäishakemistojen poistoa. Kuvaus ja tulkinta:
`../../testaus.md` luvut 17 (ajot 0–1) ja 19–20 (vaihe 4, H-041, loppuyhteenveto).
Tiedostot ovat ajoaikaisia tulosteita sellaisinaan; niitä ei ole muokattu.

Archived on 2026-11-24 before the temporary directories were removed. Description and
interpretation: `../../testaus.md` sections 17 and 19–20. Files are unmodified run
output.

| Hakemisto / folder | Sisältö / content |
|---|---|
| `ajot-0-1/` | HkMaali.exe:n koeajot 0 ja 1 palomuurieristyksellä (`fwrun0/`, `fwrun1/`, `fwrun*_log.json`, `udp_verified.json`, `fwprobe*/`, `fwcheck*/`), konsolikoeohjelmat (`cprobe*/`, `probe*/`), ensimmäinen ajo 0 AppContainerissa (`run0/`) ja AppContainer-yritysten hakemistot (`appcontainer/hkmaali_run_*`), ajoskriptit `run0.json`, `run1.json` |
| `vaihe4/run_probe1/` | koeohjelma-ajo ja UDP-portti (`udp_verified.json`) |
| `vaihe4/run_v220/` | kontrolliajo 220: `run.json`, `screens.log`, `log_verify.json`, `sandbox_copy/KILP.DAT` |
| `vaihe4/run_v221/` | ylitysajo 221: samat tiedostot |
| `vaihe4/h041_evaluation.json` | `evaluate_h041` ennalta kiinnitetyin kriteerein: "not confirmed" |
| `vaihe4/selftest5/` | ajurin itsetesti ja kaatumiskontrolli (`self_test.json`, `crash_control.json`, `crash_*.log`) |
| `vaihe4/core_all/` | komponenttitestien koko ajo (`results.json`, `results.md`, epäonnistuneiden hiekkalaatikot) |
| `vaihe4/fwlog/pfirewall_copy.log` | palomuurin pudotuslokin viimeinen kopio (16 merkintää) |
| `SHA256SUMS` | arkistoitujen tiedostojen tiivisteet / hashes of the archived files |
| `BINARIES.sha256` | arkistoimattomat binäärit (HkMaali.exe ja PDB, koeohjelmat) / binaries not archived |

Tarkistus / verify: `cd doc/arkkitehtuuri/todisteet/c11 && sha256sum -c SHA256SUMS`.

Tiedostoissa on paikallisia polkuja (käyttäjäprofiili, `%TEMP%`), koska ne ovat
ajonaikaisia tallenteita. / The files contain local paths as recorded at run time.
