# Tarkistuspiste / Checkpoint

Päivitetty / updated: 2026-11-24. Tämä tiedosto riittää työn jatkamiseen tyhjästä
istunnosta (esim. `/clear`:n jälkeen).
This file is enough to resume the work from an empty session (e.g. after `/clear`).

## Tila / State

| | |
|---|---|
| Haara / branch | `arkkitehtuuri-selvitys` (tehty / created from `main`) |
| Analysoitu revisio / analysed revision | `f7fe553` (`TPsource/` ei muutettu sen jälkeen / unchanged since) |
| Commitit `main`in päällä / commits on top of `main` | `1b72ec4` dokumentit / docs, `c273b9f` CLAUDE.md, `c651980` agentit, rekisteri, erä 1 / agents, register, batch 1, + tämän tarkistuspisteen commit / + this checkpoint's commit (`git log --oneline main..`) |
| Push / PR / issuet / issues | ei tehty; estetty paikallisesti / none; blocked locally |

## Valmiit erät / Completed batches

| Erä / Batch | Sisältö / Content | Dokumentit / Documents |
|---|---|---|
| Kierros 1 / round 1 | Koneellinen inventaario, yleiskuva, riippuvuudet, tietorakenteet, merkistöt, standardi, suunnitelma / scripted inventory, overview, dependencies, structures, encodings, standard, plan | `README.md` ja sen listaamat / and those it lists |
| Infrastruktuuri / infrastructure | 6 projektiagenttia, auditointirekisteri, validointiskripti / 6 project agents, audit register, validation script | `.claude/agents/`, `rekisteri/`, `tyokalut/tarkista_auditointi.py` |
| C-1 | `kilptietue::tallenna()`: säikeet, kutsupaikat, lukot, tallennus, SQL, replikointi / threads, call sites, locks, persistence, SQL, replication | `tallenna-samanaikaisuus.md` |

## Kattavuus / Coverage

| Mittari / Metric | Arvo / Value |
|---|---|
| Lähdetiedostoja / source files | 631 (270 050 riviä / lines) |
| Taso 1, vain inventoitu / inventoried only | 581 |
| Taso 2, kohdennetusti luettu / read in a targeted way | 50 |
| Taso 3–4, rakenne tai kokonaan katselmoitu / structure or fully reviewed | 0 |
| Havaintoja / findings | 21 vikaa / defects (H), 10 teknistä velkaa / technical debt (T) |
| Lähdeviittauksia tarkistettu / source references checked | 426 (`tarkista_auditointi.py`) |

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
| Riippumaton lähdetarkastus / independent source review | H-007, H-008, H-009, H-011, H-012, H-013, H-014, H-015, H-016, H-018, H-021; osittain / partly H-010 |
| Ei riippumatonta tarkastusta / no independent review | H-001, H-002, H-003, H-004, H-005, H-006, H-017, H-019 |
| Ei ratkea staattisesti / not statically decidable | H-020 |
| Ajonaikaisesti todennettu / verified at run time | – |

Vakavimmat / most severe (korkea / high): H-001, H-002 (muistin korruptio / memory
corruption), H-007 (menetetty päivitys / lost update), H-008 (lukko jää päälle /
lock left held), H-009 (lukkiutumisvaara / deadlock risk), H-014 (MySQL pysähtyy /
SQL writer stops).

Issue-luonnokset 01–05 ovat paikallisessa hakemistossa `issue-drafts/` (ei
versionhallinnassa) ja vastaavat havaintoja H-001–H-006.
Issue drafts 01–05 are in the local `issue-drafts/` folder (not version
controlled) and correspond to findings H-001–H-006.

## Avoimet kysymykset / Open questions

Täydet tiedot: `seuranta.md` (Q2–Q16). Tärkeimmät / most important:

- Q3: tiedonsiirtosanomien tavumuoto ja versioyhteensopivuus / message byte format and version compatibility
- Q4: `KILP.DAT`-muoto ja vanhan muodon muunnos / format and old-format conversion
- Q7: kääntääkö `bcc32` UTF-8 BOMilla oikein / does bcc32 handle UTF-8 with BOM
- Q8: miten GUI-tiedostojen kommenttimuutokset todennetaan / how to verify GUI comment changes
- Q12: kumpi `add_sakko` linkitetään / which `add_sakko` is linked (H-020)
- Q13: ylikirjoittaako `unpack0` kaikki kentät / does `unpack0` overwrite every field
- Q15: voiko VCL-poikkeus jättää lukon päälle / can a VCL exception leave a lock held
- Q16: viestin vastaanottopolku / relay receive path
- Rakenteiden asettelu: `#pragma -K -a1` 43 tiedostossa vain bcc32:lla (erä C-5) / struct layout with `-a1` in 43 files, bcc32 only (batch C-5)

## Seuraava suositeltu erä / Next recommended batch

**C-2: työasemien tiedonsiirto** (`suunnitelma.md`, "Syväauditoinnin erät").
Riski on korkea, ja se jatkaa suoraan havaintoja H-007, H-016 ja H-021. Agentit:
`architecture-analyst`, `legacy-encoding-analyst` (sanomamuodot), `defect-concurrency-auditor`,
lopuksi `independent-verifier`. Tuotos: `tiedonsiirto.md`, rekisterin päivitys.

Vaihtoehto: **C-11 testiautomaatio** (nopea, korjaa H-006:n suunnan) tai
**ajonaikainen todennus** H-008:lle ja H-009:lle ennen issueiksi vientiä
(ohjeet: `tallenna-samanaikaisuus.md`, "Miten todennetaan ajossa").

**C-2: workstation communications.** High risk; continues findings H-007, H-016
and H-021. Alternatives: C-11 test automation, or runtime verification of H-008 and
H-009 before they become issues.

## Jatkaminen `/clear`:n jälkeen / Resuming after `/clear`

1. Lue / read: `CLAUDE.md`, tämä tiedosto / this file, `doc/arkkitehtuuri/seuranta.md`,
   `doc/arkkitehtuuri/rekisteri/havainnot.md`. Muut dokumentit tarpeen mukaan /
   other documents as needed (`doc/arkkitehtuuri/README.md`).
2. Tarkista tila / check state:
   ```sh
   git branch --show-current          # arkkitehtuuri-selvitys
   git status --short                 # tyhjä / empty
   git log --oneline main..           # 4 committia / commits
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
8. Ympäristö / environment: VS 18 Build Tools, C++Builder 13 CE (`Studio\37.0`),
   Windows SDK 10.0.26100.4188 (älä päivitä / do not upgrade: C++Builder ei tunnista
   uudempaa / does not recognise newer), Python 3.13.
