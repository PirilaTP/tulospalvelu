# Tarkistuspiste / Checkpoint

Päivitetty / updated: 2026-11-24. Tämä tiedosto riittää työn jatkamiseen tyhjästä
istunnosta (esim. `/clear`:n jälkeen).
This file is enough to resume the work from an empty session (e.g. after `/clear`).

## Tila / State

| | |
|---|---|
| Haara / branch | `testit/vikojen-todentaminen` (tehty / created from `arkkitehtuuri-selvitys` @ `f5d793b`); dokumentaatiohaara / docs branch `arkkitehtuuri-selvitys` |
| Analysoitu revisio / analysed revision | `f7fe553` (`TPsource/` ei muutettu sen jälkeen / unchanged since) |
| Commitit `main`in päällä / commits on top of `main` | `1b72ec4` dokumentit / docs, `c273b9f` CLAUDE.md, `c651980` agentit, rekisteri, erä 1 / agents, register, batch 1, `3b1d978` tarkistuspiste / checkpoint, `851129f` erä / batch C-2, `bcd0098` erä / batch C-3, `f5d793b` C-11-suunnitelma / plan, `79d9957` C-11 vaihe 1 / stage 1, `cad2781` C-11 vaihe 2 / stage 2 ja seuraava commit C-11 vaihe 3 / and the next commit stage 3 (testihaaralla / on the test branch) |
| Push / PR / issuet / issues | ei tehty; estetty paikallisesti / none; blocked locally |

## Valmiit erät / Completed batches

| Erä / Batch | Sisältö / Content | Dokumentit / Documents |
|---|---|---|
| Kierros 1 / round 1 | Koneellinen inventaario, yleiskuva, riippuvuudet, tietorakenteet, merkistöt, standardi, suunnitelma / scripted inventory, overview, dependencies, structures, encodings, standard, plan | `README.md` ja sen listaamat / and those it lists |
| Infrastruktuuri / infrastructure | 6 projektiagenttia, auditointirekisteri, validointiskripti / 6 project agents, audit register, validation script | `.claude/agents/`, `rekisteri/`, `tyokalut/tarkista_auditointi.py` |
| C-1 | `kilptietue::tallenna()`: säikeet, kutsupaikat, lukot, tallennus, SQL, replikointi / threads, call sites, locks, persistence, SQL, replication | `tallenna-samanaikaisuus.md` |
| C-2 | Työasemien tiedonsiirto: kuljetus, kehys, kuittaus, kättely, sarjallistaminen, välitys, jonot, vikatilanteet, HK/viesti-erot, käännösvalitsimet, testattavuus / workstation communication | `tiedonsiirto.md` |
| C-3 | Pysyvät tiedostot ja eheys: `KILP.DAT` (dbboxm, HK- ja viestitietue, vanha muoto), `COMFILE.DAT`, emit, AJAT, `SQLjonoVarm.dat`, konfiguraatio, mitatut koot MSVC/bcc32, vikatilanteet / persistent files and integrity | `tiedostomuodot.md` |
| C-11 vaihe 3 / stage 3 | Vastaanottojonon testit (relay, HK), `VTulostus.cpp`:n sisällytys, vanhan muodon sarjatiedostot, korvaajien hiljainen tila, TpCoreJuk Asan; H-026, H-032, H-022, H-023, H-007, H-001, H-002 toistettu testikehyksessä; HkMaali.exe:n ajosuunnitelma (ei ajettu) / seven findings reproduced in test harness; HkMaali.exe run plan (not run) | `testaus.md` luvut / sections 15, 16 |
| C-11 vaihe 2 / stage 2 | HkFixture, hallitut säikeet, kaatumistodisteet (ASan, poikkeuskäsittelijä), loopback-apu, Asan-konfiguraatio; H-041, H-031, H-008, H-024, H-027 toistettu testikehyksessä; neljä agenttia + riippumaton tarkastus / five findings reproduced in test harness | `testaus.md` luku / section 14 |
| C-11 vaihe 1 / stage 1 | Testiajuri (Python), TpCoreJuk/TpCoreHk, fixturet, synteettinen aineisto; H-025, H-040, H-042 toistettu testikehyksessä; kaksi tarkastusta / runner, projects, fixtures, data; three findings reproduced in test harness; two reviews | `testaus.md` luku / section 13 |
| C-11 (suunnittelu / planning) | Testausarkkitehtuuri, testiprojektit `TpCoreHk`/`TpCoreJuk`, fixturet, eristys, testisuunnitelmat 17 korkean havainnolle, ensimmäinen toteutuserä; ei toteutusta / test architecture, projects, fixtures, isolation, plans for 17 high findings, first batch; nothing implemented | `testaus.md` |

## Kattavuus / Coverage

| Mittari / Metric | Arvo / Value |
|---|---|
| Lähdetiedostoja / source files | 631 (270 050 riviä / lines) + 19 C-11-testitiedostoa / test files |
| Taso 1, vain inventoitu / inventoried only | 538 |
| Taso 2, kohdennetusti luettu / read in a targeted way | 81 |
| Taso 3, rakenne luettu / structure read | 12 (luettelo / list: `rekisteri/moduulit.md`) |
| Taso 4, kokonaan katselmoitu / fully reviewed | 0 tuotantotiedostoa / production files; 19 testitiedostoa / test files (C-11) |
| Havaintoja / findings | 54 vikaa / defects (H), 19 teknistä velkaa / technical debt (T) |
| Lähdeviittauksia tarkistettu / source references checked | 847 (`tarkista_auditointi.py`) |

Koko koodipohjaa ei ole katselmoitu. Hakemistotason ja skriptillä tehty
analyysi on taso 1. Moduulikohtaisesti: `rekisteri/moduulit.md`.

The whole code base has not been reviewed. Directory-level and scripted analysis
is level 1. Per module: `rekisteri/moduulit.md`.

## Todennuksen lajit / Kinds of verification

Rekisteri erottaa neljä asiaa. / The register separates four things:

1. **Staattinen analyysi / static analysis**: lähdekoodi luettu. Kaikki
   H- ja T-havainnot perustuvat tähän. / source read. All H and T findings rest on this.
2. **Riippumaton lähdetarkastus / independent source review**: toinen agentti luki
   koodin uudelleen ja yritti kumota väitteen. Tämä on edelleen staattista. /
   another agent re-read the code and tried to refute the claim. Still static.
3. **Toistettu testikehyksessä / reproduced in test harness** (C-11): muuttamattomat
   tuotannon funktiot, toimitetuin käännösasetuksin, näyttivät vikamekanismin
   automaattisessa testissä, jonka tila on asetettu käsin. **H-025, H-040, H-042 (vaihe 1);
   H-041, H-031, H-008, H-024, H-027 (vaihe 2); H-026, H-032, H-022, H-023, H-007, H-001,
   H-002 (vaihe 3).**
   Ei osoita vian laukeamista eikä seurauksia käynnissä olevassa ohjelmassa. /
   unchanged production functions with shipped build settings showed the defect
   mechanism in an automated test with hand-set state. Does not show triggering or
   effects in the running program.
4. **Ajonaikainen todennus / runtime verification**: vika toistettu käynnissä
   olevassa ohjelmassa. **Yhtäkään H-havaintoa ei ole todennettu ajossa.** /
   defect reproduced in the running program. **No H finding is verified at run time.**

Ajonaikaista tai käännösnäyttöä on vain näistä / runtime or build evidence exists
only for:

- Kaikki 6 ohjelmaa (HkKisaWin, ViestiWin, HkMaali, ViestiMaali, ViestiLuenta, TpTest) kääntyvät paikallisesti (MSVC v145, C++Builder 13 + Windows SDK 10.0.26100.4188) / all 6 programs build locally
- `TpTest.exe`: 46/46 testiä läpi / tests pass (testit eivät kata H-havaintoja / tests do not cover H findings)
- Merkistökäyttäytyminen: MSVC-testiohjelma ajettu ja binäärien tavut tarkistettu (H-005) / encoding behaviour: MSVC test program run and binary bytes inspected
- Kommenttimuutosten objektivertailu testattu `tputilv2`:lla (0/146 vs 1/146) / object comparison for comment changes tested
- C-3: levylle ja verkkoon kirjoitettavien rakenteiden `sizeof`/`offsetof` mitattu koeohjelmilla MSVC:llä ja bcc32:lla; bcc32:n `#pragma option -K`/`-a1`-käytös mitattu (Q17). Koskee kokoja, ei vikojen toistamista. / sizes measured with probes in both compilers; `-K`/`-a1` behaviour measured. Sizes only, no defect reproduced.
- C-11 (koeohjelmat, ei vikojen toistamista / probes, no defect reproduced): HkMaali- ja JukMaali-lähdelistat ilman pääohjelmaa linkittyvät testiohjelmaksi; `writeerror_w(...,0)` palaa 0 ms:ssa ilman konsolia; doctest ei käänny bcc32:lla, `static_assert` kääntyy; C4789 `Juk/VTulostus.cpp:2228`, `:3981`; suljettuun loopback-porttiin yhdistäminen epäonnistuu 2000 ms:ssa; `LockCount`-odottajakoetin 200/200 / core lists link without main; `writeerror_w` returns in 0 ms; doctest fails on bcc32; C4789 reproduced; loopback refusal 2000 ms; waiter probe 200/200 (`testaus.md`)

## Havainnot / Findings

Täydet tiedot: `rekisteri/havainnot.md`. / Full details: `rekisteri/havainnot.md`.

| Todennustila / Verification state | Havainnot / Findings |
|---|---|
| Riippumaton lähdetarkastus / independent source review | H-007, H-008, H-009, H-011–H-016, H-018, H-022–H-029, H-031, H-033, H-036, H-037, H-040–H-043, H-045, H-046, H-048, H-050, H-053, H-054 |
| Riippumaton lähdetarkastus, osittain / partly | H-010, H-021, H-030, H-032, H-034, H-035, H-044, H-047, H-049, H-051, H-052 |
| Ei riippumatonta tarkastusta / no independent review | H-001–H-006, H-017, H-019, H-038, H-039 |
| Ei ratkea staattisesti / not statically decidable | H-020 |
| Toistettu testikehyksessä / reproduced in test harness | H-025, H-040, H-042 (`testaus.md`, luku / section 13); H-041, H-031, H-008, H-024, H-027 (luku / section 14); H-026, H-032, H-022, H-023, H-007, H-001, H-002 (luku / section 15) |
| Ajonaikaisesti todennettu / verified at run time | – (testisuunnitelma / test plan: `testaus.md`) |

Vakavimmat / most severe (korkea / high, 17 kpl): H-001, H-002 (muistin korruptio /
memory corruption), H-007 (menetetty päivitys / lost update), H-008, H-024 (lukko jää
päälle / lock left held), H-009 (lukkiutumisvaara / deadlock risk), H-014 (MySQL
pysähtyy / SQL writer stops), H-022 (tieto väärälle kilpailijalle / data to wrong
competitor), H-023 (aika väärään vaiheeseen / time to wrong stage), H-025 (viestin
pakettien korruptio / relay packet corruption), H-026 (viestin vastaanotto
pysähtyy / relay receiving stops), H-027 (TCP ei yhdistä uudelleen / TCP never
reconnects), H-031 (konsolin vaiheen vaihto jumittuu / console stage change stuck),
H-032 (viesti: väärennetty UDP-paketti ylivuotaa kasan / relay: forged UDP packet
overflows the heap; nostettu / raised C-3), H-040 (viestin `addfrec` ei vapauta lukkoa /
relay `addfrec` never releases its lock), H-041 (HK Release: tietue suurempi kuin
pinopuskuri / record larger than stack buffer), H-042 (viesti: XML:n määrät rajaamatta
→ ylivuoto / relay XML counts unclamped → overflow).

Kumotut väitteet erässä C-2 / refuted in C-2: pakotetun paketin kierto HK-renkaassa,
WinUDP:n taulukon ylitys, uudelleenkäynnistyksen kuitattujen lähetys, etumerkillinen
`pkgclass` (`tiedonsiirto.md`). Erässä C-3 / in C-3: viestin `pack` on suora
rakenteen kopio; HK:n Release-käännöksissä `VALUKU` 200; `COMFILE`-osoittimen
kilpailutilanne; kaksi ohjelmaa katkaisee `COMFILE.DAT`:n (`tiedostomuodot.md`).

Issue-luonnokset 01–05 ovat paikallisessa hakemistossa `issue-drafts/` (ei
versionhallinnassa) ja vastaavat havaintoja H-001–H-006.
Issue drafts 01–05 are in the local `issue-drafts/` folder (not version
controlled) and correspond to findings H-001–H-006.

## Avoimet kysymykset / Open questions

Täydet tiedot: `seuranta.md` (Q2–Q34; Q1, Q4, Q13, Q16, Q17 ja Q18 ratkaistu, Q3
osittain). Tärkeimmät / most important:

- Q7: kääntääkö `bcc32` UTF-8 BOMilla oikein / does bcc32 handle UTF-8 with BOM
- Q8: miten GUI-tiedostojen kommenttimuutokset todennetaan / how to verify GUI comment changes
- Q12: kumpi `add_sakko` linkitetään / which `add_sakko` is linked (H-020)
- Q15: voiko VCL-poikkeus jättää lukon päälle / can a VCL exception leave a lock held
- Q19: sovelletaanko AIKAT-kaksoiskappale kahdesti / is a duplicate AIKAT applied twice
- Q26: rajataanko HK:n XML:stä luetut `n_pv`/`valuku` (H-041:n toinen reitti) / are HK XML counts clamped
- Q29: käytetäänkö viestin vientiä `KILP.DAT`:na (H-046 → tietojen menetys) / is the relay export used as KILP.DAT
- Q30: HK:n vaiheen vaihto ja tiedostojen uudelleenavaus säikeiden ollessa käynnissä / stage change reopen with threads running
- Q34: kilptietueen konstruktori ennen `valuku`n lukemista / constructor before `valuku` is read

## Seuraava askel / Next step

**C-11 vaihe 3 commitoitu (`c11231b`). HkMaali.exe:n koeajot 0 ja 1 tehty onnistuneesti
palomuurieristyksellä** (`testaus.md`, luvut 17.7–17.10). Ensimmäinen AppContainer-yritys
keskeytyi (konsolin luku estetty, luku 17.3); palomuurisäännöt (käyttäjän luomat) +
kirjoitusrajoitettu tunnus + Job-objekti; ennen käynnistystä TCP-esto ajon omalla
koeohjelmalla ja UDP-esto aiemmalla, enintään 2 h vanhalla lokitodennuksella; kunkin ajon
koepaketit (TCP ja UDP) todennettu pudotuslokista porttien ja ajan perusteella jälkikäteen; lokitusasetukset palautettu. Ajo 0: kaksi kehotetta, lopetus 0, ei tiedostoja.
Ajo 1: KILP.DAT 544 tavua, otsakekuvauksen mukainen, lopetus valikosta (poistumiskoodi
aina 0). Lokissa ei ohjelman polusta estettyä ulkoverkkoliikennettä (ei kata DNS:ää,
silmukkaa, muiden prosessien liikennettä). Riippumaton tarkastus: raakaluvut oikein,
liioittelut korjattu (luku 17.12). Yhtään H-havaintoa ei vahvistettu
ajamalla. Ajotyökalut, aineisto ja luku 17 commitoitu (`432ea3d`).

**C-11 vaihe 4 (H-041 toimitetussa ohjelmassa): käyttäjä hyväksyi menetelmät A + C,
rajaparin 220/221 (ei 250) ja debuggeritilan (ei WER-asetusmuutoksia).** Esiehdot tehty
(`testaus.md` luku 19): aineistot `h041_v220/221.xml` ja komponenttitestit (PASS; tietue
10 000 / 10 040; ylitystavut nollia), `H-041b4` addfrec-ASan (control PASS, defect XFAIL);
ajurin korjaukset (pid, Job-raja 1 + kirjanpito, iphlpapi-verkkoseuranta, 30 s
aikakatkaisu, ulkoinen TCP vain 10013, D:/G:/HKCU-kirjoitukset estetty, kahvat);
`hkdebug.py`-debuggeri, joka lopettaa prosessin ennen WER:iä, ja kertakäyttöiset
keskeytyskohdat `addfrec`in alkuun ja paluukohtaan; kaatuvalla koeohjelmalla 6/6
skenaariota oikein, WER ei käynnistynyt (WerSvc pysähdyksissä, ei prosesseja,
tapahtumia eikä raportteja). Ajurin itsetesti OK, offline-testit 21/21. Riippumaton
tarkastus tehty (luku 19.8), havainnot 1–9 korjattu; HkMaali-ajoja ei ole tehty. Seuraavaksi: commit esiehdoista, sitten
pudotuslokitus (käyttäjä) ja ajot luvun 19.7 mukaan.

**C-11 stage 3 committed. HkMaali.exe runs 0 and 1 done successfully under firewall
isolation** (sections 17.7–17.10): user-created firewall rules, write-restricted token,
job object; before start: TCP blocking by the run's own probe, UDP by an earlier (<= 2 h)
log verification; each run's probe packets matched in the drop log afterwards; logging restored. Independent review: raw numbers correct,
overstatements corrected (17.12). No H finding confirmed by running. Run
tools, data and section 17 committed (`432ea3d`). Stage 4 approved (methods A + C, pair
220/221, debugger mode, no WER settings change); prerequisites done (section 19): data and
component tests, driver fixes, debugger mode checked on a crashing probe (WER never
started), self-test OK. HkMaali runs not done yet.

**Seuraava / next:** riippumaton tarkastus ja korjaukset → esiehtojen paikallinen commit →
käyttäjä ottaa pudotuslokituksen käyttöön → koeajo-, kontrolli- ja ylitysajo (luku 19.7) →
lokitus palautetaan. / independent review and fixes → local commit of the prerequisites →
the user enables drop logging → probe, control and boundary runs → logging restored.

Vaihtoehto / alternative: **C-4 muistiturvallisuus**; Asan-konfiguraatio on nyt
valmiina. / C-4 memory safety; the Asan configuration now exists.

## Resurssien käyttö / Resource usage (C-11 vaihe 3 / stage 3)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `defect-concurrency-auditor` (reseptit / recipes H-026, H-032, H-022, H-023, H-007) | 182 128 | 56 |
| `architecture-analyst` (H-001, H-002, HkMaali-suunnitelma / plan) | 220 496 | 129 |
| `build-dependency-analyst` (Sonnet; TpCoreJuk Asan, muistitarkistukset / memory checks) | 34 480 | 14 |
| `independent-verifier` (tarkastus / review; käyttöraja kesken, jatkettiin / limit hit, resumed) | 124 793 + ? | 16 + ~55 |

## Resurssien käyttö / Resource usage (C-11 vaihe 2 / stage 2)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `build-dependency-analyst` (Sonnet; ASan) | 50 667 | 21 |
| `defect-concurrency-auditor` (fixture-reseptit / recipes) | 204 831 | 32 |
| `architecture-analyst` (ajurin laajennus / runner design) | 134 226 | 27 |
| `independent-verifier` (toteutus / implementation) | 131 191 | 43 |
| **Yhteensä / total** | **520 915** | **123** |

Kaksi agenttia keskeytyi käyttörajaan ja jatkoi `SendMessage`-viestillä. / Two
agents hit the usage limit and were resumed with `SendMessage`.

## Resurssien käyttö / Resource usage (C-11 vaihe 1 / stage 1)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `build-dependency-analyst` (Sonnet; projektiasetukset / project settings) | 70 483 | 26 |
| `defect-concurrency-auditor` (testien oikeellisuus / test correctness) | 106 367 | 39 |
| `independent-verifier` (ajuri, eristys, toistettavuus / runner, isolation, reproducibility) | 108 110 | 35 |
| **Yhteensä / total** | **284 960** | **100** |

Toteutuksen (testikoodi, projektit, ajuri) teki pääistunto; `architecture-analyst`ia ei
tarvittu, koska rajapinnat oli selvitetty suunnitteluvaiheessa. / The main session
wrote the code; `architecture-analyst` was not needed since the seams were mapped
during planning.

## Resurssien käyttö / Resource usage (C-11 suunnittelu / planning)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `build-dependency-analyst` (TpTest, linkitys, CI / link, CI) | 122 865 | 40 |
| `architecture-analyst` (rajapinnat, fixturet / seams, fixtures) | 172 934 | 97 |
| `defect-concurrency-auditor` (testisuunnitelmat / test plans) | 158 326 | 68 |
| Tarkastaja / verifier V1 (H-040, H-041, H-042, H-025, H-027) | 134 500 | 71 |
| Tarkastaja / verifier V2 (infrastruktuuri, muut / infrastructure, others) | 99 786 | 67 |
| **Yhteensä / total** | **688 411** | **343** |

Luvut ovat ajoympäristön raportoimia. Erässä C-3 vastaava luku oli 745 489 / 360.
Figures as reported by the harness. The C-3 figure was 745 489 / 360.

## Jatkaminen `/clear`:n jälkeen / Resuming after `/clear`

1. Lue / read: `CLAUDE.md`, tämä tiedosto / this file, `doc/arkkitehtuuri/seuranta.md`,
   `doc/arkkitehtuuri/rekisteri/havainnot.md`. Muut dokumentit tarpeen mukaan /
   other documents as needed (`doc/arkkitehtuuri/README.md`).
2. Tarkista tila / check state:
   ```sh
   git branch --show-current          # testit/vikojen-todentaminen
   git status --short                 # commitoimatta / uncommitted: Tests/blackbox/, Tests/core/HkDataTests.cpp, Tests/data/hk/, manifest, VS/Tests/TpCoreHk.vcxproj, doc/ (luku 17)
   git log --oneline main..           # 10 committia / commits (viimeisin / last c11231b)
   python -I TPsource/V52/Tests/test_run_core_tests.py   # 46 OK
   python -I TPsource/V52/Tests/run_core_tests.py   # PASS=19, XFAIL=23 (vaatii Release- ja TpCoreHk Asan -käännöksen / needs Release and TpCoreHk Asan builds, testaus.md 13.6, 14)
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
