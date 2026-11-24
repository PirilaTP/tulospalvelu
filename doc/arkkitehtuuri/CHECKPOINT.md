# Tarkistuspiste / Checkpoint

Päivitetty / updated: 2026-11-24. Tämä tiedosto riittää työn jatkamiseen tyhjästä
istunnosta (esim. `/clear`:n jälkeen).
This file is enough to resume the work from an empty session (e.g. after `/clear`).

## Tila / State

| | |
|---|---|
| Haara / branch | `testit/vikojen-todentaminen` (tehty / created from `arkkitehtuuri-selvitys` @ `f5d793b`); dokumentaatiohaara / docs branch `arkkitehtuuri-selvitys` |
| Analysoitu revisio / analysed revision | `f7fe553` (`TPsource/` ei muutettu sen jälkeen / unchanged since) |
| Commitit `main`in päällä / commits on top of `main` | `1b72ec4` dokumentit / docs, `c273b9f` CLAUDE.md, `c651980` agentit, rekisteri, erä 1 / agents, register, batch 1, `3b1d978` tarkistuspiste / checkpoint, `851129f` erä / batch C-2, `bcd0098` erä / batch C-3, `f5d793b` C-11-suunnitelma / plan, `79d9957` C-11 vaihe 1 / stage 1, `cad2781` C-11 vaihe 2 / stage 2, `c11231b` vaihe 3 / stage 3, `432ea3d` koeajot 0–1 / runs 0–1, `c6205d2` vaihe 4 esiehdot / stage 4 prerequisites ja C-11:n päätöscommit / and the C-11 closing commit (testihaaralla / on the test branch) |
| Push / PR / issuet / issues | ei tehty; estetty paikallisesti / none; blocked locally |

## Valmiit erät / Completed batches

| Erä / Batch | Sisältö / Content | Dokumentit / Documents |
|---|---|---|
| Kierros 1 / round 1 | Koneellinen inventaario, yleiskuva, riippuvuudet, tietorakenteet, merkistöt, standardi, suunnitelma / scripted inventory, overview, dependencies, structures, encodings, standard, plan | `README.md` ja sen listaamat / and those it lists |
| Infrastruktuuri / infrastructure | 6 projektiagenttia, auditointirekisteri, validointiskripti / 6 project agents, audit register, validation script | `.claude/agents/`, `rekisteri/`, `tyokalut/tarkista_auditointi.py` |
| C-1 | `kilptietue::tallenna()`: säikeet, kutsupaikat, lukot, tallennus, SQL, replikointi / threads, call sites, locks, persistence, SQL, replication | `tallenna-samanaikaisuus.md` |
| C-2 | Työasemien tiedonsiirto: kuljetus, kehys, kuittaus, kättely, sarjallistaminen, välitys, jonot, vikatilanteet, HK/viesti-erot, käännösvalitsimet, testattavuus / workstation communication | `tiedonsiirto.md` |
| C-3 | Pysyvät tiedostot ja eheys: `KILP.DAT` (dbboxm, HK- ja viestitietue, vanha muoto), `COMFILE.DAT`, emit, AJAT, `SQLjonoVarm.dat`, konfiguraatio, mitatut koot MSVC/bcc32, vikatilanteet / persistent files and integrity | `tiedostomuodot.md` |
| **C-4 valmis ja suljettu / complete and closed** (vaihe 2 hyväksytty ja commitoitu / step 2 approved and committed) | Vaihe 2: H-055/H-056 syvemmin (verkkosaavutettavuus), 33 tutkittavaa riviä, 522 muunnosvaroitusta ryhmittäin, kattavuusarvio, kokonaisarvio ja priorisointi; uudet H-070–H-077, T-022; laajennettu H-055, H-056, H-059, H-069; kolme riippumatonta tarkastusta / step 2: deep analysis, investigate rows, conversions, coverage, overall assessment; new H-070–H-077, T-022 | `muistiturvallisuus.md` luvut / sections 9–17 |
| C-4 vaihe 1 / step 1 (hyväksytty, uudelleentarkastettu, commitoitu / approved, re-reviewed, committed) | Varoitusinventaario: MSVC 19.52 `/W3`, `/W3`+LTCG, `/W4 /analyze` (5 projektia, 7 kokoonpanoa), bcc32 7.80 tallennettu + `-w` (4 GUI-projektia); 11 027 riviä, 7 036 yksilöllistä; 275 tärkeintä luokiteltu (37 vahvistettu, 64 todennäköinen, 33 tutkittava, 141 väärä positiivinen; luvut korjattu riveittäisistä luokituksista); uudet H-055–H-069, T-020, T-021; kaksi riippumatonta tarkastusta / warning inventory on both compilers, triage, new findings, two independent reviews | `muistiturvallisuus.md`, `todisteet/c4/` |
| C-11 valmis / complete | Vaihe 4: H-041 toimitetussa HkMaali.exe:ssä debuggerin alla (220/221), käyttäjän poikkeamapäätöksellä "vahvistettu ajamalla"; loppuyhteenveto, raakatulosten arkisto (`todisteet/c11/`), siivous / stage 4: H-041 confirmed by running (user-approved deviation); final summary, archive, clean-up | `testaus.md` luvut / sections 18–20 |
| C-11 vaihe 3 / stage 3 | Vastaanottojonon testit (relay, HK), `VTulostus.cpp`:n sisällytys, vanhan muodon sarjatiedostot, korvaajien hiljainen tila, TpCoreJuk Asan; H-026, H-032, H-022, H-023, H-007, H-001, H-002 toistettu testikehyksessä; HkMaali.exe:n ajosuunnitelma (ei ajettu) / seven findings reproduced in test harness; HkMaali.exe run plan (not run) | `testaus.md` luvut / sections 15, 16 |
| C-11 vaihe 2 / stage 2 | HkFixture, hallitut säikeet, kaatumistodisteet (ASan, poikkeuskäsittelijä), loopback-apu, Asan-konfiguraatio; H-041, H-031, H-008, H-024, H-027 toistettu testikehyksessä; neljä agenttia + riippumaton tarkastus / five findings reproduced in test harness | `testaus.md` luku / section 14 |
| C-11 vaihe 1 / stage 1 | Testiajuri (Python), TpCoreJuk/TpCoreHk, fixturet, synteettinen aineisto; H-025, H-040, H-042 toistettu testikehyksessä; kaksi tarkastusta / runner, projects, fixtures, data; three findings reproduced in test harness; two reviews | `testaus.md` luku / section 13 |
| C-11 (suunnittelu / planning) | Testausarkkitehtuuri, testiprojektit `TpCoreHk`/`TpCoreJuk`, fixturet, eristys, testisuunnitelmat 17 korkean havainnolle, ensimmäinen toteutuserä; ei toteutusta / test architecture, projects, fixtures, isolation, plans for 17 high findings, first batch; nothing implemented | `testaus.md` |

## Kattavuus / Coverage

| Mittari / Metric | Arvo / Value |
|---|---|
| Lähdetiedostoja / source files | 631 (270 050 riviä / lines) + 19 C-11-testitiedostoa / test files |
| Taso 1, vain inventoitu / inventoried only | 499 |
| Taso 2, kohdennetusti luettu / read in a targeted way | 120 (C-4: varoitusten ja havaintojen kohdat / warning and finding sites) |
| Taso 3, rakenne luettu / structure read | 12 (luettelo / list: `rekisteri/moduulit.md`) |
| Taso 4, kokonaan katselmoitu / fully reviewed | 0 tuotantotiedostoa / production files; 19 testitiedostoa / test files (C-11) |
| Havaintoja / findings | 77 vikaa / defects (H), 22 teknistä velkaa / technical debt (T) |
| Lähdeviittauksia tarkistettu / source references checked | 1 089 (`tarkista_auditointi.py`) |

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
   olevassa ohjelmassa. **Vain H-041** (toimitettu HkMaali.exe, C-11 vaihe 4; ennalta
   kiinnitetty arvio "not confirmed", taso käyttäjän poikkeamapäätöksellä, `testaus.md`
   19.11). / defect reproduced in the running program. **Only H-041** (by a user-approved
   deviation from the pre-fixed verdict).

Ajonaikaista tai käännösnäyttöä on vain näistä / runtime or build evidence exists
only for:

- C-11 vaihe 4: toimitettu HkMaali.exe debuggerin alla, rajapari 220/221 → H-041
  vahvistettu ajamalla käyttäjän poikkeamapäätöksellä (`testaus.md` 19.9–19.11); ajot 0–1
  (luku 17) eivät vahvistaneet havaintoja / shipped HkMaali.exe under the debugger → H-041

- Kaikki 6 ohjelmaa (HkKisaWin, ViestiWin, HkMaali, ViestiMaali, ViestiLuenta, TpTest) kääntyvät paikallisesti (MSVC v145, C++Builder 13 + Windows SDK 10.0.26100.4188) / all 6 programs build locally
- `TpTest.exe`: 46/46 testiä läpi / tests pass (testit eivät kata H-havaintoja / tests do not cover H findings)
- Merkistökäyttäytyminen: MSVC-testiohjelma ajettu ja binäärien tavut tarkistettu (H-005) / encoding behaviour: MSVC test program run and binary bytes inspected
- Kommenttimuutosten objektivertailu testattu `tputilv2`:lla (0/146 vs 1/146) / object comparison for comment changes tested
- C-3: levylle ja verkkoon kirjoitettavien rakenteiden `sizeof`/`offsetof` mitattu koeohjelmilla MSVC:llä ja bcc32:lla; bcc32:n `#pragma option -K`/`-a1`-käytös mitattu (Q17). Koskee kokoja, ei vikojen toistamista. / sizes measured with probes in both compilers; `-K`/`-a1` behaviour measured. Sizes only, no defect reproduced.
- C-4: varoitusinventaario molemmilla kääntäjillä (MSVC `/W4 /analyze`, LTCG-linkitys; bcc32 `-w`); bcc32:n uudelleenajo tuotti täsmälleen tallennetut varoitukset; käännösnäyttöä, ei ajoja eikä vikojen toistamista (`muistiturvallisuus.md`) / warning inventory, build evidence only, nothing run
- C-11 (koeohjelmat, ei vikojen toistamista / probes, no defect reproduced): HkMaali- ja JukMaali-lähdelistat ilman pääohjelmaa linkittyvät testiohjelmaksi; `writeerror_w(...,0)` palaa 0 ms:ssa ilman konsolia; doctest ei käänny bcc32:lla, `static_assert` kääntyy; C4789 `Juk/VTulostus.cpp:2228`, `:3981`; suljettuun loopback-porttiin yhdistäminen epäonnistuu 2000 ms:ssa; `LockCount`-odottajakoetin 200/200 / core lists link without main; `writeerror_w` returns in 0 ms; doctest fails on bcc32; C4789 reproduced; loopback refusal 2000 ms; waiter probe 200/200 (`testaus.md`)

## Havainnot / Findings

Täydet tiedot: `rekisteri/havainnot.md`. / Full details: `rekisteri/havainnot.md`.

| Todennustila / Verification state | Havainnot / Findings |
|---|---|
| Riippumaton lähdetarkastus / independent source review | H-007, H-008, H-009, H-011–H-016, H-018, H-022–H-029, H-031, H-033, H-036, H-037, H-040–H-043, H-045, H-046, H-048, H-050, H-053, H-054, H-055, H-056, H-058, H-059, H-061, H-062, H-064, H-066–H-073, H-075, H-076 |
| Riippumaton lähdetarkastus, osittain / partly | H-010, H-021, H-030, H-032, H-034, H-035, H-044, H-047, H-049, H-051, H-052, H-057, H-060, H-063, H-065, H-074, H-077 |
| Ei riippumatonta tarkastusta / no independent review | H-001–H-006, H-017, H-019, H-038, H-039 |
| Ei ratkea staattisesti / not statically decidable | H-020 |
| Toistettu testikehyksessä / reproduced in test harness | H-025, H-040, H-042 (`testaus.md`, luku / section 13); H-041, H-031, H-008, H-024, H-027 (luku / section 14); H-026, H-032, H-022, H-023, H-007, H-001, H-002 (luku / section 15) |
| Ajonaikaisesti todennettu / verified at run time | H-041 (toimitettu HkMaali.exe, poikkeamapäätös / user-approved deviation, `testaus.md` 19.11) |

Vakavimmat / most severe (korkea / high, 22 kpl; C-4 lisäsi / added H-055 emiTag-indeksi pinoon, myös verkosta / emiTag index into stack, also from the network; H-056 laitelukijoiden ikuiset silmukat / device reader infinite loops; H-070 FEIG-lukijan kasaylivuoto / FEIG heap overflow; H-071 FILESEND: verkon pituus ja tiedostopolku / network length and file path; H-072 `EXTRA`-paketin indeksit / EXTRA packet indices): H-001, H-002 (muistin korruptio /
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
Erässä C-4 / in C-4: HK:n `copydfile` ei käytä puskuria; `fclose(NULL)` ei kaada C++Builder-ohjelmia
(`Tp/TpApu.cpp:164`, GUI:n `seur_rata`); `tark_partio` ei koske GUI:ta; `TauluJako` (`nikk` ≤ 4);
HK:n lähdepistetiedoston `KAIKKI`-ylivuoto; H-061:n "ei `/J`" (`muistiturvallisuus.md`).

Issue-luonnokset 01–05 ovat paikallisessa hakemistossa `issue-drafts/` (ei
versionhallinnassa) ja vastaavat havaintoja H-001–H-006.
Issue drafts 01–05 are in the local `issue-drafts/` folder (not version
controlled) and correspond to findings H-001–H-006.

## Avoimet kysymykset / Open questions

Täydet tiedot: `seuranta.md` (Q2–Q48; C-4:n Q43–Q48 `muistiturvallisuus.md`; Q1, Q4, Q13, Q16, Q17 ja Q18 ratkaistu, Q3
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

**C-11 valmis (2026-11-24).** Loppuyhteenveto: `testaus.md` luku 20. Toistettu
testikehyksessä 15 havaintoa (H-001, H-002, H-007, H-008, H-022–H-027, H-031, H-032,
H-040–H-042); vahvistettu toimitetussa ohjelmassa 1 (H-041: HkMaali.exe, rajapari
220/221 debuggerin alla, 221 → 0xC0000409/2 `addfrec`in evästetarkistuksessa; ennalta
kiinnitetty arvio "not confirmed" kriteerin 7 loki-ikkunaosuman vuoksi, taso käyttäjän
poikkeamapäätöksellä, prosessi 26236 = kontrolliajon jälkikoeohjelma, luku 19.11); vain
staattinen 39. Avoimet jatkotehtävät: käsitestit H-009 ja H-014, kahden prosessin
loopback-kerros, H-041:n lukupolku, testit CI:hin (luku 20.4). Raakatulokset ja tiivisteet:
`todisteet/c11/`. Tilapäishakemistot poistettu; palomuurisääntöjen
`TP C11 HkMaali block in/out` poisto valmisteltu käyttäjälle (luku 20.6).

**C-4 vaihe 1 valmis (2026-11-24), käyttäjä hyväksyi; riippumaton uudelleentarkastus (K1–K9) ja korjaukset tehty; commitoitu paikallisesti.**
Raportti `muistiturvallisuus.md`; raakatulokset `todisteet/c4/`. Uudet havainnot
H-055–H-069 (2 korkeaa, 9 keskitasoa, 4 matalaa) ja T-020, T-021; kaikki staattisia.
Tunnetuista puskurivioista H-041, H-042 ja H-052 ei varoittanut mikään kääntäjä.

**C-4 valmis ja suljettu (2026-11-25), käyttäjä hyväksyi; vaihe 2 commitoitu paikallisesti.**
Kiireellisesti korjattavat / urgent fixes: **H-071, H-072, H-055, H-070** (P1–P5 muilta osin
ennallaan); väliaikaiset suojaussuositukset `muistiturvallisuus.md` luku 15.
Kokonaisarvio, priorisointi P1–P5, korjattavat vs. todennettavat ja loppuyhteenveto:
`muistiturvallisuus.md` luvut 15–17. Uudet H-070–H-077 ja T-022. Vakavimmat ovat
verkosta saavutettavia (H-071, H-072, H-055, H-032) ja laiteperäisiä (H-070, H-056).
Kaikki C-4:n havainnot ovat staattisesti vahvistettuja mekanismeja; yhtäkään ei ole
toistettu eikä ajossa vahvistettu (ainoa ajossa vahvistettu on H-041).

**SEURAAVA TYÖVAIHE / NEXT STEP: C-5 kääntäjäkohtainen käytös** (`suunnitelma.md` erä 5);
odottaa aloituslupaa / awaiting approval to start.
Aloitusvalmius: C-3:n mitatut rakennekoot ja `-K`/`-a1`-mittaukset, C-4:n `char`-etumerkki-
ja /GS-erot (luku 6) ja W8059-varoitukset (373 kpl, `todisteet/c4/`) ovat valmiina.

**C-11 complete.** 15 findings reproduced in the test harness, 1 confirmed in the shipped
program (H-041, by a user-approved deviation from the pre-fixed verdict), 39 static only.
Open: manual tests H-009/H-014, two-process loopback layer, H-041 read path, CI. Next:
C-4 memory safety (plan batch 4). **C-4 complete and closed:** step 1 (`81bcad9`) and step 2 committed locally. Urgent
fixes: H-071, H-072, H-055, H-070. Next step: C-5, awaiting approval to start.

## Resurssien käyttö / Resource usage (C-4 vaihe 2 / step 2)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `independent-verifier` K1–K9 (vaihe 1 / step 1) | 99 273 | 54 |
| `architecture-analyst` H-055, H-056 | 119 519 | 61 |
| `defect-concurrency-auditor` 33 tutkittavaa / investigate rows | 148 693 | 88 |
| `defect-concurrency-auditor` muunnosvaroitukset / conversions | 142 416 | 71 |
| `independent-verifier` E1–E6 | 96 314 | 58 |
| `independent-verifier` F1–F7 | 76 151 | 39 |
| **Yhteensä / total** | **682 366** | **371** |

## Resurssien käyttö / Resource usage (C-4 vaihe 1 / step 1)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `defect-concurrency-auditor` A1 (`Hk/`, `HkConsole/`, 96 riviä / rows) | 130 241 | 55 |
| `defect-concurrency-auditor` A2 (`Tp/`, `tputilv2/`, `dbboxm/`, `com/`, `cbHk/`, 79) | 119 286 | 58 |
| `defect-concurrency-auditor` B (`Juk/`, `JukConsole/`, `ViestiWin/`, 99) | 114 045 | 63 |
| `independent-verifier` C1–C7 | 80 944 | 44 |
| `independent-verifier` D1–D8 | 86 180 | 44 |
| **Yhteensä / total** | **530 696** | **264** |

Käännökset ja tilastot teki pääistunto; `build-dependency-analyst`ia ei tarvittu. /
Builds and statistics by the main session.

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
   git status --short                 # puhdas C-4:n jälkeen / clean after C-4
   git log --oneline main..           # 16 committia / commits (viimeisin / last: C-4 vaihe 2 / step 2)
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
