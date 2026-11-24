# Tarkistuspiste / Checkpoint

Päivitetty / updated: 2026-11-24. Tämä tiedosto riittää työn jatkamiseen tyhjästä
istunnosta (esim. `/clear`:n jälkeen).
This file is enough to resume the work from an empty session (e.g. after `/clear`).

## Tila / State

| | |
|---|---|
| Haara / branch | `arkkitehtuuri-selvitys` (tehty / created from `main`) |
| Analysoitu revisio / analysed revision | `f7fe553` (`TPsource/` ei muutettu sen jälkeen / unchanged since) |
| Commitit `main`in päällä / commits on top of `main` | `1b72ec4` dokumentit / docs, `c273b9f` CLAUDE.md, `c651980` agentit, rekisteri, erä 1 / agents, register, batch 1, `3b1d978` tarkistuspiste / checkpoint, erä C-2 seuraava commit / batch C-2 in the next commit (`docs(audit): analyze workstation communication batch C-2`) |
| Push / PR / issuet / issues | ei tehty; estetty paikallisesti / none; blocked locally |

## Valmiit erät / Completed batches

| Erä / Batch | Sisältö / Content | Dokumentit / Documents |
|---|---|---|
| Kierros 1 / round 1 | Koneellinen inventaario, yleiskuva, riippuvuudet, tietorakenteet, merkistöt, standardi, suunnitelma / scripted inventory, overview, dependencies, structures, encodings, standard, plan | `README.md` ja sen listaamat / and those it lists |
| Infrastruktuuri / infrastructure | 6 projektiagenttia, auditointirekisteri, validointiskripti / 6 project agents, audit register, validation script | `.claude/agents/`, `rekisteri/`, `tyokalut/tarkista_auditointi.py` |
| C-1 | `kilptietue::tallenna()`: säikeet, kutsupaikat, lukot, tallennus, SQL, replikointi / threads, call sites, locks, persistence, SQL, replication | `tallenna-samanaikaisuus.md` |
| C-2 | Työasemien tiedonsiirto: kuljetus, kehys, kuittaus, kättely, sarjallistaminen, välitys, jonot, vikatilanteet, HK/viesti-erot, käännösvalitsimet, testattavuus / workstation communication | `tiedonsiirto.md` |

## Kattavuus / Coverage

| Mittari / Metric | Arvo / Value |
|---|---|
| Lähdetiedostoja / source files | 631 (270 050 riviä / lines) |
| Taso 1, vain inventoitu / inventoried only | 568 |
| Taso 2, kohdennetusti luettu / read in a targeted way | 59 |
| Taso 3, rakenne luettu / structure read | 4 (`Tp/TpComY.cpp`, `Hk/HkCom32.cpp`, `com/WinUDP.cpp`, `include/wincom.h`) |
| Taso 4, kokonaan katselmoitu / fully reviewed | 0 |
| Havaintoja / findings | 39 vikaa / defects (H), 14 teknistä velkaa / technical debt (T) |
| Lähdeviittauksia tarkistettu / source references checked | 531 (`tarkista_auditointi.py`) |

Koko koodipohjaa ei ole katselmoitu. Hakemistotason ja skriptillä tehty
analyysi on taso 1. Moduulikohtaisesti: `rekisteri/moduulit.md`.

The whole code base has not been reviewed. Directory-level and scripted analysis
is level 1. Per module: `rekisteri/moduulit.md`.

## Todennuksen lajit / Kinds of verification

Rekisteri erottaa kolme asiaa. / The register separates three things:

1. **Staattinen analyysi / static analysis**: lähdekoodi luettu. Kaikki
   H- ja T-havainnot perustuvat tähän. / source read. All H and T findings rest on this.
2. **Riippumaton lähdetarkastus / independent source review**: toinen agentti luki
   koodin uudelleen ja yritti kumota väitteen. Tämä on edelleen staattista. /
   another agent re-read the code and tried to refute the claim. Still static.
3. **Ajonaikainen todennus / runtime verification**: vika toistettu käynnissä
   olevassa ohjelmassa. **Yhtäkään H-havaintoa ei ole todennettu ajossa.** /
   defect reproduced in the running program. **No H finding is verified at run time.**

Ajonaikaista tai käännösnäyttöä on vain näistä / runtime or build evidence exists
only for:

- Kaikki 6 ohjelmaa (HkKisaWin, ViestiWin, HkMaali, ViestiMaali, ViestiLuenta, TpTest) kääntyvät paikallisesti (MSVC v145, C++Builder 13 + Windows SDK 10.0.26100.4188) / all 6 programs build locally
- `TpTest.exe`: 46/46 testiä läpi / tests pass (testit eivät kata H-havaintoja / tests do not cover H findings)
- Merkistökäyttäytyminen: MSVC-testiohjelma ajettu ja binäärien tavut tarkistettu (H-005) / encoding behaviour: MSVC test program run and binary bytes inspected
- Kommenttimuutosten objektivertailu testattu `tputilv2`:lla (0/146 vs 1/146) / object comparison for comment changes tested

## Havainnot / Findings

Täydet tiedot: `rekisteri/havainnot.md`. / Full details: `rekisteri/havainnot.md`.

| Todennustila / Verification state | Havainnot / Findings |
|---|---|
| Riippumaton lähdetarkastus / independent source review | H-007, H-008, H-009, H-011–H-016, H-018, H-022–H-029, H-031, H-033, H-036, H-037 |
| Riippumaton lähdetarkastus, osittain / partly | H-010, H-021, H-030, H-032, H-034, H-035 |
| Ei riippumatonta tarkastusta / no independent review | H-001–H-006, H-017, H-019, H-038, H-039 |
| Ei ratkea staattisesti / not statically decidable | H-020 |
| Ajonaikaisesti todennettu / verified at run time | – |

Vakavimmat / most severe (korkea / high, 13 kpl): H-001, H-002 (muistin korruptio /
memory corruption), H-007 (menetetty päivitys / lost update), H-008, H-024 (lukko jää
päälle / lock left held), H-009 (lukkiutumisvaara / deadlock risk), H-014 (MySQL
pysähtyy / SQL writer stops), H-022 (tieto väärälle kilpailijalle / data to wrong
competitor), H-023 (aika väärään vaiheeseen / time to wrong stage), H-025 (viestin
pakettien korruptio / relay packet corruption), H-026 (viestin vastaanotto
pysähtyy / relay receiving stops), H-027 (TCP ei yhdistä uudelleen / TCP never
reconnects), H-031 (konsolin vaiheen vaihto jumittuu / console stage change stuck).

Kumotut väitteet erässä C-2 / refuted in C-2: pakotetun paketin kierto HK-renkaassa,
WinUDP:n taulukon ylitys, uudelleenkäynnistyksen kuitattujen lähetys, etumerkillinen
`pkgclass` (`tiedonsiirto.md`).

Issue-luonnokset 01–05 ovat paikallisessa hakemistossa `issue-drafts/` (ei
versionhallinnassa) ja vastaavat havaintoja H-001–H-006.
Issue drafts 01–05 are in the local `issue-drafts/` folder (not version
controlled) and correspond to findings H-001–H-006.

## Avoimet kysymykset / Open questions

Täydet tiedot: `seuranta.md` (Q2–Q25; Q1, Q13 ja Q16 ratkaistu, Q3 osittain).
Tärkeimmät / most important:

- Q4: `KILP.DAT`-muoto ja vanhan muodon muunnos / format and old-format conversion
- Q7: kääntääkö `bcc32` UTF-8 BOMilla oikein / does bcc32 handle UTF-8 with BOM
- Q8: miten GUI-tiedostojen kommenttimuutokset todennetaan / how to verify GUI comment changes
- Q12: kumpi `add_sakko` linkitetään / which `add_sakko` is linked (H-020)
- Q15: voiko VCL-poikkeus jättää lukon päälle / can a VCL exception leave a lock held
- Q17: noudattaako bcc32 `#pragma option -K` -direktiiviä kesken käännösyksikön (tarkistussumma) / does bcc32 honour `-K` mid-unit (checksum)
- Q18: `sizeof(combufrec)` ja suurin `class_len` kussakin käännöksessä (H-025, H-032) / per build
- Q19: sovelletaanko AIKAT-kaksoiskappale kahdesti / is a duplicate AIKAT applied twice
- Rakenteiden asettelu: `#pragma -K -a1` 43 tiedostossa vain bcc32:lla (erä C-5) / struct layout with `-a1` in 43 files, bcc32 only (batch C-5)

## Seuraava suositeltu erä / Next recommended batch

**C-3: pysyvät tiedostot ja eheys** (`suunnitelma.md`, erä 3).

Perustelut / rationale:
- Erän C-2 vakavimmat ongelmat riippuvat tiedostoista: tietuenumero on asemien
  välinen avain (H-022), levyjono tallentaa tilannekuvia (H-030), uudelleenkäynnistys
  ja `COMFILE.DAT` (H-038), kirjoitusvirheet (H-015) ja emit-tiedosto ilman lukkoa (H-011).
- `KILP.DAT`-, `COMFILE.DAT`- ja emit-tiedostojen muodot ovat dokumentoimatta (Q4),
  ja niitä tarvitaan testiaineistojen rakentamiseen ajonaikaista todennusta varten.
- Kattavuus: `dbboxm/` ja tiedostomuotokoodi ovat vielä tasolla 1.

**Tarkka tehtävä / exact task:** "Jatka auditointia erällä C-3: pysyvät tiedostot ja
eheys. Selvitä `KILP.DAT` (dbboxm, `pack`/`unpack`, `kilprecsize`, vanha muoto
`Hk/HkInitVanha.cpp`), `COMFILE.DAT` (otsake, tietueiden sijainti, `lahetetty`,
uudelleenkäynnistys), emit-tiedosto (`putem`/`lueem`), `ajat.txt`/aikajono,
`SQLjonoVarm.dat` ja konfiguraatiotiedostojen luku. Ratkaise Q4 ja Q18. Agentit:
`legacy-encoding-analyst` (tiedostomuodot ja tavuasettelut), `defect-concurrency-auditor`
(eheys, virheenkäsittely, kaatumis- ja uudelleenkäynnistystilanteet),
`build-dependency-analyst` (käännösaikaiset koot ja valitsimet, `sizeof`-arvot),
lopuksi `independent-verifier`. Tuotos: `tiedostomuodot.md`, rekisterin ja
tarkistuspisteen päivitys. Ei muutoksia `TPsource/`-hakemistoon."

Vahva vaihtoehto / strong alternative: **C-11 testiautomaatio ja ajonaikainen
todennusvalmius**. 13 korkean vakavuuden löydöstä on todennettu vain
staattisesti. Kahden aseman testiympäristö, testikoukku (`tiedonsiirto.md`,
"Testattavuus") ja testit CI:hin muuttaisivat ne ajonaikaisesti todennetuiksi
ennen issueiksi vientiä.

**C-3: persistent files and integrity.** The most serious C-2 issues depend on
files (record number as cross-station key, disk-queue snapshots, restart, write
errors, emit file), and file formats are undocumented (Q4) and needed for test
fixtures. Strong alternative: C-11 test automation and runtime-verification
readiness, since 13 high-severity findings are verified only statically.

## Resurssien käyttö / Resource usage (C-2)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| A `architecture-analyst` | 229 991 | 48 |
| B `defect-concurrency-auditor` | 193 136 | 40 |
| C `build-dependency-analyst` | 127 302 | 30 |
| Tarkastaja / verifier 1 | 117 789 | 52 |
| Tarkastaja / verifier 2 | 114 341 | 77 |
| **Yhteensä / total** | **782 559** | **247** |

Luvut ovat agenttien raportoimia (ensimmäisen ajon osuus ennen käyttörajaa ei
välttämättä sisälly). Pääistunnon kontekstin käyttöä ei voi mitata tarkasti.
Figures as reported by the agents (the share used before the usage limit may not be
included). The main session's context use cannot be measured precisely.

## Jatkaminen `/clear`:n jälkeen / Resuming after `/clear`

1. Lue / read: `CLAUDE.md`, tämä tiedosto / this file, `doc/arkkitehtuuri/seuranta.md`,
   `doc/arkkitehtuuri/rekisteri/havainnot.md`. Muut dokumentit tarpeen mukaan /
   other documents as needed (`doc/arkkitehtuuri/README.md`).
2. Tarkista tila / check state:
   ```sh
   git branch --show-current          # arkkitehtuuri-selvitys
   git status --short                 # pitäisi olla tyhjä / should be empty
   git log --oneline main..           # 5 committia / commits
   python -I doc/arkkitehtuuri/tyokalut/tarkista_auditointi.py .   # OK
   ```
3. Rajoitukset (voimassa, kunnes käyttäjä toisin sanoo) / constraints (in force
   until the user says otherwise):
   - Ei pushia, PR:iä eikä GitHub-issueita. `.claude/settings.local.json` estää
     `git push` ja `gh pr create` (deny-säännöt). Tällä haaralla tiedosto on
     `--skip-worktree`-tilassa, joten se ei näy muutoksena; älä commitoi sitä. /
     No push, PRs or GitHub issues; deny rules in `.claude/settings.local.json`
     (marked skip-worktree on this branch; do not commit it).
   - Ei muutoksia `TPsource/`-hakemistoon ilman erillistä lupaa; vikakorjaukset
     omiin haaroihinsa, ei dokumentaation kanssa. / No `TPsource/` changes without
     separate approval; bug fixes in their own branches, not with docs.
   - Ei massakommentointia. / No bulk commenting.
   - Commitit vain paikallisesti ja pyydettäessä. / Commits only locally and when asked.
4. Kielisäännöt / language rules: dokumentit kaksikielisinä samassa tiedostossa,
   suomi ensin; uudet koodikommentit englanniksi ASCII-merkein; Pekan kommentit
   säilytetään (`dokumentointistandardi.md`, `merkistot.md`). / Docs bilingual,
   Finnish first; new code comments English ASCII; keep Pekka's comments.
5. Agentit / agents (`.claude/agents/`): `architecture-analyst`,
   `defect-concurrency-auditor`, `independent-verifier` (Opus);
   `build-dependency-analyst`, `legacy-encoding-analyst`, `doc-writer` (Sonnet).
   Enintään kolme analyysiagenttia rinnakkain; jokainen erä päättyy
   riippumattomaan tarkastukseen ja rekisterin päivitykseen. / At most three
   analysis agents in parallel; every batch ends with independent review and a
   register update.
6. Rekisterin päivitys erän jälkeen / register update after a batch:
   ```sh
   python -I doc/arkkitehtuuri/tyokalut/inventaario.py . <tmp>/inv.json
   python -I doc/arkkitehtuuri/tyokalut/rekisteri.py <tmp>/inv.json doc/arkkitehtuuri/rekisteri/tiedostot.csv <git-rev>
   ```
   Nosta analyysitasoja käsin vain luetuille tiedostoille; päivitä
   `havainnot.md`, `moduulit.md`, `seuranta.md` ja tämä tiedosto. / Raise analysis
   levels by hand only for files actually read; update the listed files.
7. Muut haarat / other branches: `untrack-claude-local-settings` (3 paikallista
   committia / local commits); sen commit `d8a7938` väittää virheellisesti, ettei
   VS 2022 riitä – korjattava ennen julkaisua. / its commit `d8a7938` wrongly
   claims VS 2022 is not enough – fix before publishing.
8. Agenttien käyttörajat / agent usage limits: erässä C-2 kaikki kolme
   analyysiagenttia pysähtyivät käyttörajaan. Ne jatkettiin `SendMessage`-viestillä
   samasta kohdasta, kun raja nollautui; uutta agenttia ei tarvita. / In C-2 all
   three analysis agents hit the usage limit and were resumed with `SendMessage`.
9. Ympäristö / environment: VS 18 Build Tools, C++Builder 13 CE (`Studio\37.0`),
   Windows SDK 10.0.26100.4188 (älä päivitä / do not upgrade: C++Builder ei tunnista
   uudempaa / does not recognise newer), Python 3.13.
