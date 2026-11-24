# C-4: varoitusinventaarion raakatulokset / Warning inventory raw results

Tuotettu 2026-11-24 revisiosta `f7fe553`. Kuvaus ja tulkinta:
`../../muistiturvallisuus.md`. Tiedostot ovat käännösten tulosteita sellaisinaan; niitä
ei ole muokattu (paitsi `warnings_unique.csv` pakattu gzipillä).

Produced on 2026-11-24 from revision `f7fe553`. Description and interpretation:
`../../muistiturvallisuus.md`. Files are unmodified build output.

| Tiedosto / file | Sisältö / content |
|---|---|
| `warnings_unique.csv.gz` | Kaikki varoitukset yksilöllisinä (kääntäjä, koodi, tiedosto, rivi, viesti, kokoonpanot, läheinen H-havainto ±3 riviä) / all warnings, unique, with configurations and nearby H finding |
| `triage_A1.csv`, `triage_A2.csv`, `triage_B.csv` | Luokitteluagenttien syötteet (275 riviä) / triage inputs |
| `triage_*_luokitus.csv` | Riveittäinen luokitus agenttien raporteista (vaihe 2) / per-row verdicts from the auditor reports |
| `commands.txt` | Ensimmäinen `CL.exe`- ja linkityskomento kustakin MSVC-käännöksestä; tallennettu ja uudelleenajettu bcc32-komento / first compile and link commands; recorded and re-run bcc32 commands |
| `coverage.json` | Kunkin kääntäjän käännösyksiköt ja kääntämättömät tiedostot / translation units per compiler, uncompiled files |
| `lokit/msvc-warn-logs.tgz` | MSBuildin varoituslokit ja konsolitulosteet / MSBuild warning logs and console output |
| `lokit/bcc32-outputs.json.gz` | bcc32:n tulosteet sekä tallennettu ja ajettu komento käännösyksiköittäin / bcc32 output, recorded and executed command per unit |
| `skriptit/` | `base.props`, `w4analyze.props` (MSBuild-ohjaus / redirection, `/W4 /analyze`), `build.sh`, `buildlink.sh`, `build_v145.sh`, `bcc_run.py`, `collect.py`, `maph.py`, `coverage.py` |
| `SHA256SUMS` | Tiivisteet / hashes |

Toisto / reproduce (Git Bash, väliaikaishakemistossa / in a scratch folder):

```sh
./build.sh base  HkMaali <repo>/TPsource/V52/VS/Hk/HkMaali/HkMaali.vcxproj Release
./build.sh w4a   HkMaali <repo>/TPsource/V52/VS/Hk/HkMaali/HkMaali.vcxproj Release
./buildlink.sh base JukMaali <repo>/TPsource/V52/VS/V/JukMaali520.vcxproj Release
python -I bcc_run.py <repo>/TPsource/V52/RADStudio10/build-logs/HkKisaWin.log <repo>/TPsource/V52/RADStudio10 bccout/wall/HkKisaWin wall
python -I collect.py . warnings_all.csv
```

Skripteissä on tämän koneen polkuja (`S=`-muuttuja, MSBuild-polku). bcc32-ajo lukee
komennot `build-gui.ps1`:n tallentamasta lokista. Tulosteet ohjataan väliaikaiseen
hakemistoon; projektitiedostoja ei muuteta. / The scripts contain local paths; bcc32
commands are read from the recorded build log; outputs are redirected and project files
are not changed.

Tarkistus / verify: `cd doc/arkkitehtuuri/todisteet/c4 && sha256sum -c SHA256SUMS`.
