# C-5: kääntäjäkohtaisen käytöksen raakatulokset / Compiler-specific behaviour raw results

Tuotettu 2026-11-25 revisiosta `1457e26` (`TPsource/` ennallaan revisiosta `f7fe553`).
Kuvaus ja tulkinta: `../../kaantajaerot.md`. Arkistojen sisältö on agenttien ja
pääistunnon tuottamia koeohjelmia, skriptejä ja tulosteita sellaisinaan; niitä ei ole
muokattu. Suuret tai binääriset välitulokset jätettiin pois, mutta niiden tiivisteet
ovat tiedostossa `raaka-manifest.sha256`, joten alkuperä on todennettavissa niin kauan
kuin väliaikaishakemisto on tallessa, ja ne voi tuottaa uudelleen skripteillä.

Produced on 2026-11-25 from revision `1457e26` (`TPsource/` unchanged since `f7fe553`).
Interpretation: `../../kaantajaerot.md`. Archive contents are the unmodified probes,
scripts and outputs. Large or binary intermediates were left out; their hashes are in
`raaka-manifest.sha256`, and they can be regenerated with the scripts.

| Tiedosto / file | Sisältö / content |
|---|---|
| `msvc/units.json` | Käännösyksiköt ja käytetyt CL-valitsimet projekteittain (C-4:n tallennetuista komentoriveistä) / units and CL options per project |
| `msvc/layout_diffs.json`, `msvc/compare_sets.out` | `/d1reportAllClassLayout`-vertailun tulokset: avaimena nimi + jäsenet, ja yksikkökohtaisten asettelujoukkojen vertailu / layout comparison results |
| `msvc/msvc_run.txt` | Ajon yhteenveto: 251 yksikköä, 0 virhettä / run summary |
| `lea.tgz` | `legacy-encoding-analyst`: merkistö-, muotoilu-, tyyppi-, `char`- ja RTL-koeohjelmat ja tulosteet (`*.cpp`, `*-bcc.out`, `*-msvc.out`), `scan_lit.py`, `lits.json`, `incl.py`; pois / excluded: `.exe`, `.obj`, `.tds`, `.pch` |
| `bda.tgz` | `build-dependency-analyst`: optiomatriisi ja taulukot (`tbl*.csv`), pragmatilan jäljitys (`ana.py`), koettimet (`gt*.py`, `probe/`); pois / excluded: `cpp32`-esikäsittelytulosteet `.i` (205 Mt), `.pch`, `.obj` |
| `ver.tgz` | `independent-verifier`: omat koeohjelmat (`p_*`), skriptit (`vmt.py`, `xacc.py`, `states.py`, `u8scan.py`, `pad.py`, `wcstok.py`, `msvcname.py`) ja tulosteet; pois / excluded: `dump/` (`tdump`-tulosteet toimitetuista objekteista, 79 Mt), binäärit |
| `utf8_literaalit_c5.out` | Korjatun `../../tyokalut/utf8_literaalit.py`:n tulos: 36 merkkijonoa / output of the fixed tool |
| `litexe_main.out` | Mitkä UTF-8-merkkijonot ovat rikkinäisinä missäkin `TPexe/`-ohjelmassa / which broken literals are in which shipped program |
| `skriptit/` | `msvc_layout.py`, `compare_layout.py`, `compare_sets.py`, `litexe_main.py` |
| `vaihe2/` | Vaiheen 2 raakatulokset omine tiivisteineen (`vaihe2/README.md`) / step 2 raw results with their own hashes |
| `raaka-manifest.sha256` | Kaikkien väliaikaishakemiston raakatiedostojen tiivisteet (myös pois jätettyjen) / hashes of all raw scratch files, including excluded ones |
| `SHA256SUMS` | Tämän hakemiston tiedostojen tiivisteet / hashes of the files here |

Toisto / reproduce (Git Bash, väliaikaishakemistossa / in a scratch folder):

```sh
python -I skriptit/msvc_layout.py <C-4-lokihakemisto/logdir> msvc
python -I skriptit/compare_layout.py msvc HkMaali=HkMaali+tputilv2+dbboxm ViestiMaali=JukMaali+tputilv2+dbboxm ViestiLuenta=JukLuenta+tputilv2+dbboxm
python -I skriptit/compare_sets.py msvc
python -I skriptit/litexe_main.py <repo>
```

`msvc_layout.py` lukee CL-komentorivit C-4:n MSBuild-lokeista (`base-<projekti>-Release.log`)
ja sisältää tämän koneen MSVC-polut. Skriptit eivät muuta projektitiedostoja. /
`msvc_layout.py` reads the CL lines from the C-4 MSBuild logs and contains local MSVC
paths. The scripts do not change project files.

Tarkistus / verify: `cd doc/arkkitehtuuri/todisteet/c5 && sha256sum -c SHA256SUMS`.
