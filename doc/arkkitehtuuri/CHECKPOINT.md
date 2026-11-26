# Tarkistuspiste / Checkpoint

Päivitetty / updated: 2026-11-26. Tämä tiedosto riittää työn jatkamiseen tyhjästä
istunnosta (esim. `/clear`:n jälkeen).
This file is enough to resume the work from an empty session (e.g. after `/clear`).

## Tila / State

| | |
|---|---|
| Haara / branch | `testit/vikojen-todentaminen` (tehty / created from `arkkitehtuuri-selvitys` @ `f5d793b`); dokumentaatiohaara / docs branch `arkkitehtuuri-selvitys` |
| Analysoitu revisio / analysed revision | `f7fe553` (`TPsource/`:n tuotantokoodi ei muuttunut sen jälkeen; vain C-11:n testit `Tests/` ja `VS/Tests/` lisätty, tarkistettu `git diff --stat f7fe553 HEAD -- TPsource` 2026-11-27 / production code unchanged since; only the C-11 tests added) |
| Commitit `main`in päällä / commits on top of `main` | `1b72ec4` dokumentit / docs, `c273b9f` CLAUDE.md, `c651980` agentit, rekisteri, erä 1 / agents, register, batch 1, `3b1d978` tarkistuspiste / checkpoint, `851129f` erä / batch C-2, `bcd0098` erä / batch C-3, `f5d793b` C-11-suunnitelma / plan, `79d9957` C-11 vaihe 1 / stage 1, `cad2781` C-11 vaihe 2 / stage 2, `c11231b` vaihe 3 / stage 3, `432ea3d` koeajot 0–1 / runs 0–1, `c6205d2` vaihe 4 esiehdot / stage 4 prerequisites ja C-11:n päätöscommit / and the C-11 closing commit (testihaaralla / on the test branch); sen jälkeen erät C-4–C-13 paikallisina committeina samalla haaralla, viimeisimmät C-12:n sulkeminen `3f378e2` ja C-13 (`git log --oneline main..`) / then batches C-4..C-13 as local commits on the same branch |
| Push / PR / issuet / issues | ei tehty; estetty paikallisesti / none; blocked locally. **Auditointihaaraa ei pushata julkiseen repositorioon ilman käyttäjän erillistä hyväksyntää** (aineistossa on korjaamattomien tietoturvavikojen yksityiskohtia); yksityinen varmuuskopio on tekemättä (`korjaussuunnitelma.md` 9.3) / the audit branch must not be pushed to a public repository without separate approval; a private backup has not been made |
| Vaiheen tila / phase state | **Vaihe C suljettu 2026-11-26** käyttäjän päätöksellä; **vaihe K käynnistetty 2026-11-27**: K-0 ja K-1 valmiit, K-2:n pilotti (12 korttia) odottaa käyttäjän hyväksyntää, K-3–K-5 ei aloitettu; D-vaihe ei aloitettu / phase K started: K-0 and K-1 done, K-2 pilot awaits approval; phase D not started |

## Valmiit erät / Completed batches

| Erä / Batch | Sisältö / Content | Dokumentit / Documents |
|---|---|---|
| **Vaihe C suljettu / phase C closed** (käyttäjän päätös / user decision 2026-11-26) | Erät C-1–C-13 suljettu; korjaus- ja testaussuunnitelma hyväksytty D-vaiheen lähtökohdaksi (ei lupa aloittaa korjauksia); D-vaiheen ennakkotehtävät E-1–E-4 kirjattu; vaihe K suunniteltu. **Koko lähdekoodia ei ole katselmoitu**: 36 % riveistä vain inventoitu, taso 4: 0 tuotantotiedostoa / batches closed; plans approved as the starting point of phase D; preparatory tasks recorded; phase K planned. The code base has not been fully reviewed | `auditoinnin-yhteenveto.md` 12, `korjaussuunnitelma.md` 9, `k-vaihe-suunnitelma.md` |
| **C-13 valmis ja suljettu / complete and closed** (käyttäjä hyväksyi / approved 2026-11-26; riippumattomasti tarkastettu, commit `e8daa65`) | Vaiheen C päätöserä, synteesi (ei uutta lähdeauditointia, mitään ei käännetty eikä ajettu): koneellinen pohja `tyokalut/synteesi.py` (tarkistettu ennen agenttikäyttöä), kattavuus ja todennuksen vahvuus, 15 juurisyytä JS-1–JS-15, yhtenäinen riskiarvio (P1 5, P2 35, P3 62, P4 47, P5 27; jokaisella avoimella H:lla prioriteetti), 34 korjauskokonaisuutta (10 aiempaa tunnusta + KK-C13-1–24), riippuvuusmalli ja aallot 0–6, regressiotestaussuunnitelma, D-vaiheen viisi vaihetta ja ensimmäinen toteutuserä (25 havaintoa), omistajan päätökset OP-1–OP-18; Opus-kokonaisarvio K1–K17 käsitelty (H-033 → P1); H-116 ja H-052 laajennettu; Q3, Q24, Q26, Q53 ratkaistu / final batch of phase C: synthesis, machine base, coverage, root causes, uniform risk assessment, 34 fix packages, order, test plan, development plan, owner decisions; independent review handled | `auditoinnin-yhteenveto.md`, `korjaussuunnitelma.md`, `regressiotestaussuunnitelma.md`, `omistajan-paatokset.md`, `rekisteri/riskiarvio.csv`, `rekisteri/korjauskokonaisuudet.csv`, `todisteet/c13/` |
| **C-9 valmis ja suljettu / complete and closed** (käyttäjä hyväksyi / approved 2026-11-26) | Vaiheet 1 (`f1eb195`) ja 2 (`920930c`), sulkeminen `4b48749`: virheenkäsittely ja resurssit; H-145–H-158, T-040–T-042; H-149 korkea; KK-C9-1–3 / error handling and resources | `virheenkasittely.md`, `todisteet/c9/` |
| **C-8 valmis ja suljettu / complete and closed** (käyttäjä hyväksyi / approved 2026-11-26) | Vaihe 1 `4884374`, sulkeminen `7aaca40`: globaali muuttuva tila; H-128–H-144, T-038–T-039; H-128 ja H-129 toistettu eristetyllä koeohjelmalla / global mutable state | `globaali-tila.md`, `todisteet/c8/` |
| **C-7 valmis ja suljettu / complete and closed** (käyttäjä hyväksyi / approved 2026-11-25) | Vaihe 1 `e3a8b67`, sulkeminen `be52dbf`: GUI:n liiketoimintalogiikka, kahdeksan lomakeyksikköä; H-109–H-127, T-034–T-037; jatkotyöjono J7-1–J7-7 / GUI business logic | `gui-liiketoimintalogiikka.md`, `todisteet/c7/` |
| **C-12 valmis ja suljettu / complete and closed** (käyttäjä hyväksyi ilman toista työerää / approved without a step 2, 2026-11-26) | Vaihe 1 `7b4ed46` ja sulkemiscommit / step 1 and closing commit; sulkeminen / closure `ominaisuusvalitsimet.md` luku 13: kattavuus ja rekisterien johdonmukaisuus, H-171 → korkea, P2 (sijoittelu bcc32-objektista), H-134 H-129:n kanssa, KK-C12-1–3, Q107/Q108/Q110 ajonaikaiseen todennukseen, Q111 omistajalle, Q112 avoin; testimatriisia ei ajettu / test matrix not run | `ominaisuusvalitsimet.md` 13 |
| **C-12 vaihe 1 valmis / step 1 complete** (riippumattomasti tarkastettu, commitoitu paikallisesti, odottaa katselmointia / independently reviewed, committed locally, awaiting review) | Esikääntäjän valitsimet (435 makroa, `tyokalut/valitsimet.py` tarkistettu ennen agenttikäyttöä), asetusavaimet ja komentorivi (`tyokalut/asetusavaimet.py`), muut kytkimet, rekisterit `rekisteri/valitsimet.csv` ja `asetusavaimet.csv`, ominaisuus- ja testimatriisi; Q2 ratkaistu, T-008 arvioitu; uudet H-171–H-177, T-050–T-052; H-036, H-134 laajennettu; Q107–Q112; 19 väitettä Opus-tarkastajalla / switches, keys, other switches, registers, matrices; Q2 resolved | `ominaisuusvalitsimet.md`, `todisteet/c12/` |
| **C-10 valmis ja suljettu / complete and closed** (käyttäjä hyväksyi ilman toista työerää / approved without a step 2, 2026-11-26) | Vaihe 1 `46509f9` ja sulkemiscommit / step 1 and closing commit; sulkeminen / closure `riippuvuudet.md` luku 19: P2 H-159, H-160 (P3 → P2), P3–P5 muut; korjauskokonaisuudet / fix packages KK-C10-1–4; lisenssit omistajalle / licences to the owner OM-C10-1; tuoteinventaario täsmäytetty, `/MT` tarkennettu / product inventory reconciled; Q105, Q106; ei ajonaikaista todennusta / not verified at run time | `riippuvuudet.md` 19 |
| **C-10 vaihe 1 valmis / step 1 complete** (riippumattomasti tarkastettu, commitoitu paikallisesti / independently reviewed, committed locally) | Tuotekohtainen riippuvuusinventaario (skripti tarkistettu ennen agenttikäyttöä), tietokanta ja Q5, HTTP ja selain, laiteprotokollat ja Q86, ajonaikaiset paketit ja jakelu (Q91, Q92, T-005), versiot ja modernisointiriskit; uudet H-159–H-170, T-043–T-049; 15 väitettä Opus-tarkastajalla / per-product inventory, database, HTTP and browser, devices, packages and deployment, versions; new findings; 15 claims reviewed | `riippuvuudet.md` luvut 7–18, `todisteet/c10/` |
| **C-6 valmis ja suljettu / complete and closed** (käyttäjä hyväksyi / approved 2026-11-25) | Vaiheet 1 (`bf5b566`) ja 2 (`2767145`); H-099 kumottu; korjausjärjestys P2 H-087, H-090, H-091, H-093, H-107; ajoa vaativat avoimet asiat (bcc32, H-087/H-103 ViestiWinissä, Q67:n apusäikeet, Q68, Q73) jatkotehtävinä `seuranta.md` / open items needing runs deferred | `saikeet-ja-lukot.md` 13.11, `seuranta.md` |
| **C-6 vaihe 2 valmis / step 2 complete** (riippumattomasti tarkastettu, commitoitu paikallisesti / independently reviewed, committed locally) | Viiden havainnon polut; H-090, H-091 (reunat), H-093 (rekursio ja kahden säikeen lukkiutuminen) ja H-094 (menetys) toistettu eristetyllä koeohjelmalla tuotantokoodista; Q65 Win32-kokeella; H-090 ja H-093 → korkea, H-099 kumottu; uudet H-103–H-108, T-033; VCL-säieturvallisuus; luvun 9 ehdokkaat; 11 väitettä + kaikki kokeet kahdella Opus-tarkastajalla / five findings' paths, isolated probes, VCL, new H-103–H-108 | `saikeet-ja-lukot.md` luku 13, `todisteet/c6/vaihe2/` |
| **C-6 vaihe 1 valmis / step 1 complete** (riippumattomasti tarkastettu, commitoitu paikallisesti / independently reviewed, committed locally) | Säiekartta (56 aktiivista käynnistyskohtaa, 29 säiefunktiota, ei yhtään odotusta), elinkaari ja lopetus, Q30 ratkaistu (vaiheen vaihto ei sulje `KILP.DAT`:ia), lukkokartta (22 lukkoa), lukitusjärjestys (3 todellista ABBA-paria), estävät kutsut lukon alla; uudet H-087–H-102, T-030–T-032; H-018, H-031, H-038 laajennettu; 20 väitettä kahdella Opus-tarkastajalla / thread map, lifecycle, Q30, lock map, lock order, blocking under locks; new findings; 20 claims reviewed | `saikeet-ja-lukot.md`, `todisteet/c6/` |
| Kierros 1 / round 1 | Koneellinen inventaario, yleiskuva, riippuvuudet, tietorakenteet, merkistöt, standardi, suunnitelma / scripted inventory, overview, dependencies, structures, encodings, standard, plan | `README.md` ja sen listaamat / and those it lists |
| Infrastruktuuri / infrastructure | 6 projektiagenttia, auditointirekisteri, validointiskripti / 6 project agents, audit register, validation script | `.claude/agents/`, `rekisteri/`, `tyokalut/tarkista_auditointi.py` |
| C-1 | `kilptietue::tallenna()`: säikeet, kutsupaikat, lukot, tallennus, SQL, replikointi / threads, call sites, locks, persistence, SQL, replication | `tallenna-samanaikaisuus.md` |
| C-2 | Työasemien tiedonsiirto: kuljetus, kehys, kuittaus, kättely, sarjallistaminen, välitys, jonot, vikatilanteet, HK/viesti-erot, käännösvalitsimet, testattavuus / workstation communication | `tiedonsiirto.md` |
| C-3 | Pysyvät tiedostot ja eheys: `KILP.DAT` (dbboxm, HK- ja viestitietue, vanha muoto), `COMFILE.DAT`, emit, AJAT, `SQLjonoVarm.dat`, konfiguraatio, mitatut koot MSVC/bcc32, vikatilanteet / persistent files and integrity | `tiedostomuodot.md` |
| **C-5 valmis ja suljettu / complete and closed** (käyttäjä hyväksyi / approved 2026-11-25) | Vaiheet 1 (`b17d536`) ja 2 (`55897eb`); avoinna Q60 (omistaja / owner), Q63, Q64; jatkotehtävät (161 jäsennysehdokasta, H-079:n uudelleenarviointi) ja korjausjono `seuranta.md` / follow-ups and fix queue | `kaantajaerot.md` 13.7, `seuranta.md` |
| **C-5 vaihe 2 valmis / step 2 complete** (riippumattomasti tarkastettu, commitoitu / independently reviewed, committed) | Q8: bcc32-objektien vertailu `tyokalut/vertaa_omf.py` (10 yksikköä; tarkastus löysi ja korjattiin COMENT FB -puute); GUI-tyyppejä ei kirjoiteta raakana; T-026:n kohdista vain 5 voi katketa; koeohjelmat Q57–Q59, Q61, Q62; uudet H-082–H-086 (GUI-lomakkeiden rajatarkistukset, `TextFl::Open` "a"), T-029; H-042, H-079 laajennettu / bcc32 object comparison; GUI-only types not raw-written; probes; new GUI form bound-check defects | `kaantajaerot.md` luku 13, `todisteet/c5/vaihe2/` |
| **C-5 vaihe 1 valmis / step 1 complete** (riippumattomasti tarkastettu, commitoitu / independently reviewed, committed) | Käännösasetusten matriisi, MSVC:n asetteluraportti 251 yksiköstä (ei eroja), bcc32:n pragmatila 405 yksiköstä (`-a1`/`-a8`, piilevä), merkistö (bcc32 lukee BOMittoman UTF-8:n väärin, Q7 ratkaistu, `merkistot.md` korjattu, 36 literaalia), tyypit, muotoilu, RTL-erot; uudet H-078–H-081, T-023–T-028, H-004/H-005 laajennettu; `utf8_literaalit.py` korjattu / option matrix, MSVC layout (no differences), bcc32 pack state (latent), encoding (bcc32 also wrong, Q7 resolved), types, formatting, RTL | `kaantajaerot.md`, `todisteet/c5/` |
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
| Taso 1, vain inventoitu / inventoried only | 474 |
| Taso 2, kohdennetusti luettu / read in a targeted way | 135 (erät C-10 ja C-12 eivät muuttaneet tiedostokohtaisia tasoja / batches C-10 and C-12 did not change per-file levels; C-4: varoitusten ja havaintojen kohdat; C-5: kääntäjäerojen kohdat; C-6: säikeiden ja lukkojen kohdat / warning, finding, compiler-difference, thread and lock sites) |
| Taso 3, rakenne luettu / structure read | 22 (luettelo / list: `todisteet/c13/raportit/osaA-kattavuus.md` 4.1; aiempi luku 12 oli erän C-4 tilanne / the earlier 12 was the C-4 figure) |
| Taso 4, kokonaan katselmoitu / fully reviewed | 0 tuotantotiedostoa / production files; 19 testitiedostoa / test files (C-11) |
| Havaintoja / findings | 177 vikaa / defects (H-001–H-177; H-099 kumottu / refuted), 52 teknistä velkaa / technical debt (T-001–T-052) |
| Lähdeviittauksia tarkistettu / source references checked | 2 477 (`tarkista_auditointi.py`, C-13; tarkistaa vain tiedoston ja rivin olemassaolon, ei sisältöä / checks existence and range only, not content) |
| Riskiarvio ja korjauskokonaisuudet / risk register and fix packages | 176 avointa H:ta arvioitu (P1 5, P2 35, P3 62, P4 47, P5 27); 34 kokonaisuutta; tarkistus / check `python -I doc/arkkitehtuuri/tyokalut/synteesi.py . <out> --tarkista` |

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
- C-5: MSVC `/d1reportAllClassLayout` 251 yksiköstä; bcc32 `cpp32`-esikäsittely 405 yksiköstä ja `-S`-koettimet; merkistö-, muotoilu-, tyyppi- ja RTL-koeohjelmat molemmilla kääntäjillä; `tdump` toimitetuista GUI-objekteista; merkkijonohaku `TPexe/`-ohjelmista. Käännös- ja koeohjelmanäyttöä, toimitettuja ohjelmia ei ajettu (`kaantajaerot.md`) / layout reports, pack-state trace, probes on both compilers, object dumps, string search in shipped binaries; products not run
- C-11 (koeohjelmat, ei vikojen toistamista / probes, no defect reproduced): HkMaali- ja JukMaali-lähdelistat ilman pääohjelmaa linkittyvät testiohjelmaksi; `writeerror_w(...,0)` palaa 0 ms:ssa ilman konsolia; doctest ei käänny bcc32:lla, `static_assert` kääntyy; C4789 `Juk/VTulostus.cpp:2228`, `:3981`; suljettuun loopback-porttiin yhdistäminen epäonnistuu 2000 ms:ssa; `LockCount`-odottajakoetin 200/200 / core lists link without main; `writeerror_w` returns in 0 ms; doctest fails on bcc32; C4789 reproduced; loopback refusal 2000 ms; waiter probe 200/200 (`testaus.md`)

## Havainnot / Findings

Täydet tiedot: `rekisteri/havainnot.md`. / Full details: `rekisteri/havainnot.md`.

| Todennuksen vahvuus / Strength of verification (C-13, vahvin näyttö / strongest evidence) | Havainnot / Findings |
|---|---|
| a. Kumottu / refuted | 1: H-099 |
| b. Vahvistettu toimitetussa ohjelmassa / confirmed in a shipped program | 1: H-041 (poikkeamapäätös / user-approved deviation, `testaus.md` 19.11) |
| c. Toistettu koeohjelmalla tai testikehyksessä / reproduced with a probe or the harness | 26: C-11 H-001, H-002, H-007, H-008, H-022–H-027, H-031, H-032, H-040, H-042; C-6 H-090, H-091, H-093, H-094; C-8 H-128, H-129; C-9 H-145, H-146, H-147, H-149, H-150, H-151 |
| d. Riippumaton lähdetarkastus, täysi / independent source review, full | 89 |
| e. Riippumaton lähdetarkastus, osittain / partly | 43 |
| f. Tarkastajan löytö, pääistunto todensi / reviewer's find | 3: H-132, H-158, H-162 |
| g. Ei riippumatonta tarkastusta / never independently reviewed | 13: H-003, H-006, H-019, H-039, H-108, H-122, H-123, H-124, H-126, H-127, H-142, H-143, H-144 |
| h. Ei ratkea staattisesti / not statically decidable | 1: H-020 |

Havaintokohtainen tila: `todisteet/c13/osaA0/havainnot.csv` ja `rekisteri/riskiarvio.csv`
(sarake `todennus`). H-017 on tarkastettu erässä C-7; aiempi merkintä "ei tarkastettu" oli
vanhentunut. / Per-finding state in the two CSV files; H-017 was reviewed in C-7.

**C-13:** rekisterissä on 34 korkean vakavuuden riviä; riskiarviossa lisäksi H-033 (korkea, P1).
Prioriteetit: P1 H-032, H-033, H-061, H-071, H-072; P2 35 riviä (`rekisteri/riskiarvio.csv`).
Alla oleva luettelo on erien C-1–C-10 kertymä. / 34 high-severity rows in the register, plus H-033
in the risk register; the list below is the accumulation of batches C-1..C-10.

Vakavimmat / most severe (korkea / high, 22 kpl erän C-4 jälkeen / after C-4; C-4 lisäsi / added H-055 emiTag-indeksi pinoon, myös verkosta / emiTag index into stack, also from the network; H-056 laitelukijoiden ikuiset silmukat / device reader infinite loops; H-070 FEIG-lukijan kasaylivuoto / FEIG heap overflow; H-071 FILESEND: verkon pituus ja tiedostopolku / network length and file path; H-072 `EXTRA`-paketin indeksit / EXTRA packet indices): H-001, H-002 (muistin korruptio /
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
→ ylivuoto / relay XML counts unclamped → overflow). C-6 lisäsi / added (26 kpl yhteensä / in total):
H-087 (viestin HTML-emit-tuloste ei vapauta `print`-lukkoa → työsäikeet ja GUI jäätyvät /
relay HTML emit output never releases `print` → freeze), H-091 (HK tall ↔ print -lukkiutuminen
jatkuvalla sivutulosteella / tall↔print deadlock with continuous page printing), vaiheessa 2
nostetut / raised in step 2: H-090 (`tall_regnly` jättää `ajat`-lukon, `TUNNJONO` / keeps `ajat`),
H-093 (HK TCP-loki: rajaton rekursio → pinon ylivuoto / unbounded recursion → stack overflow).
C-10 lisäsi / added: H-159 (Sirit/Zebra-uudelleenyhteys lopettaa RFID-luennan / RFID reconnect ends reading), H-160 (SQL-virhe lopettaa GUI-ohjelman, korkea SQL-peilausta käytettäessä / SQL error ends the GUI program, high when the mirror is used); molemmat P2 sulkemisessa / both P2 at the closing.

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

**C-13:** kysymyksiä on Q1–Q112, joista 32 on ratkaistu. Loput 80 on luokiteltu vaiheen C
sulkemista varten: 39 ajonaikaiseen todennukseen D-vaiheessa, 16 omistajan päätökseksi ja 25
hyväksytty avoimiksi (`auditoinnin-yhteenveto.md` luku 6). / Q1..Q112; 32 resolved, the other 80
classified: 39 run-time verification in phase D, 16 owner decisions, 25 accepted as open.

Täydet tiedot: `seuranta.md` (alla oleva luettelo on erien C-1–C-7 tilanne / the list below is the
state of batches C-1..C-7: Q2–Q77; C-7:n Q75–Q77 `gui-liiketoimintalogiikka.md` luku 10; C-4:n Q43–Q48 `muistiturvallisuus.md`; C-6:n Q65–Q74 `saikeet-ja-lukot.md` luvut 10 ja 13.5; Q1, Q4, Q13, Q16, Q17, Q18, Q30, Q65, Q66, Q69–Q72 ja Q74 ratkaistu, Q3,
Q14 ja Q67 osittain). Tärkeimmät / most important:

- ~~Q5~~, ~~Q92~~ ratkaistu C-10:ssä / resolved in C-10; Q86, Q91 osittain / partly; uudet / new Q96–Q104 (`riippuvuudet.md` luku 17) ja Q105, Q106 (luku 19); Q101 omistajalle / to the owner (OM-C10-1)
- ~~Q7~~ ratkaistu C-5:ssä / resolved in C-5 (`kaantajaerot.md` luku 5)
- ~~Q8~~ ratkaistu C-5:ssä: `tyokalut/vertaa_omf.py` / resolved in C-5
- Q60: onko H-081:n `KUNTOSUUNN`-ero tarkoituksellinen – omistajan päätös / owner decision
- Q63, Q64: bcc32:n negatiivinen `new` (H-085), `UnitSakkoKierr`in kumoamisjäsennys (H-082) /
  open technical questions
- Q12: kumpi `add_sakko` linkitetään / which `add_sakko` is linked (H-020)
- Q15: voiko VCL-poikkeus jättää lukon päälle / can a VCL exception leave a lock held
- Q19: sovelletaanko AIKAT-kaksoiskappale kahdesti / is a duplicate AIKAT applied twice
- Q26: rajataanko HK:n XML:stä luetut `n_pv`/`valuku` (H-041:n toinen reitti) / are HK XML counts clamped
- Q29: käytetäänkö viestin vientiä `KILP.DAT`:na (H-046 → tietojen menetys) / is the relay export used as KILP.DAT
- ~~Q30~~ ratkaistu C-6:ssa / resolved in C-6 (`saikeet-ja-lukot.md` luku 5)
- ~~Q65, Q66~~ ratkaistu C-6 vaiheessa 2 / resolved in C-6 step 2; Q67 osittain / partly
  (`saikeet-ja-lukot.md` 13.5); avoinna Q68, Q73 / open Q68, Q73
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

**C-5 vaihe 1 valmis (2026-11-25), riippumattomasti tarkastettu, commitoitu paikallisesti.**
Raportti `kaantajaerot.md`; todisteet `todisteet/c5/`. Levylle ja verkkoon kirjoitettavat rakenteet ovat
molemmilla kääntäjillä samat jäsenten offsetteja myöten; MSVC:ssä ei asetteluerojä käännösyksiköiden välillä;
bcc32:n `-a1`/`-a8`-erot ovat piileviä (T-023). Merkittävimmät: H-078 (UTF-16BE kaataa konsolin), H-005
(UTF-8-tekstit väärin myös GUI:ssa; `merkistot.md` korjattu). Avoimet Q57–Q62.

**C-5 vaihe 2 valmis (2026-11-25), riippumattomasti tarkastettu, commitoitu paikallisesti.**
Q8 ratkaistu (`tyokalut/vertaa_omf.py`); uudet H-082–H-086 ja T-029; avoinna Q60, Q63, Q64.

**C-5 valmis ja suljettu (2026-11-25), käyttäjä hyväksyi.** Jatkotehtävät ja priorisoitu
korjausjono (H-071, H-072, H-055, H-070 kiireellisinä): `seuranta.md`.

**C-6 vaihe 1 valmis (2026-11-25), riippumattomasti tarkastettu, commitoitu paikallisesti.**
Raportti `saikeet-ja-lukot.md`; todisteet `todisteet/c6/`. Uudet H-087–H-102 (korkea 2,
keskitaso–korkea 2, keskitaso 5, matala–keskitaso 1, matala 6) ja T-030–T-032; kaikki staattisia.
Q30 ratkaistu; uudet Q65–Q74.

**C-6 vaihe 2 valmis (2026-11-25), riippumattomasti tarkastettu, commitoitu paikallisesti.**
Raportti `saikeet-ja-lukot.md` luku 13; koeohjelmat ja tulokset `todisteet/c6/vaihe2/`
(eristetyt, MSVC-konsolikäännös tuotantokoodista; ei toimitettuja ohjelmia, ei verkkoa).
H-090 ja H-093 → korkea; H-099 kumottu; uudet H-103–H-108 ja T-033.

**C-6 valmis ja suljettu (2026-11-25), käyttäjä hyväksyi.** Avoimet ajoa vaativat asiat
jatkotehtävinä (`seuranta.md` "Avoimet jatkotehtävät C-6:sta"); korjauksia ei aloitettu.

**C-7 vaihe 1 valmis (2026-11-25), riippumattomasti tarkastettu, commitoitu paikallisesti.**
Raportti `gui-liiketoimintalogiikka.md`; raakatulokset `todisteet/c7/`. Uudet H-109–H-127
(korkea 3: H-109 poisto ilman tarkistusta, H-110 viestin lähtöaikojen lukkovuoto, H-111
L-rivi maaliriviksi; keskitaso 6; matala–keskitaso 7; matala 3) ja T-034–T-037; H-017
kavennettu, H-085 ja H-069 laajennettu; 9 jäsennysehdokasta ratkaistu; Q15 osittain, uudet
Q75–Q77. Kaikki staattisia.

**C-7 valmis ja suljettu (2026-11-25), käyttäjä hyväksyi** kattavuustarkistuksen jälkeen
(`gui-liiketoimintalogiikka.md` luku 13). GUI-syötteiden tarkastus ei ole valmis kokonaisuutena:
jatkotyöjono J7-1–J7-7 (`seuranta.md`): arvonta-algoritmi `cbHk/HkArv2.cpp`, 41 muuta
kilpailutietoja kirjoittavaa lomaketta, 152 jäsennysehdokasta, H-109/H-110/H-116:n ajonaikaiset
vahvistukset, tarkastamattomat H-122–H-124/H-126/H-127, Q15/Q75–Q77, haarojen korjaussuunnittelu.

**C-8 vaihe 1 valmis (2026-11-26), riippumattomasti tarkastettu, commitoitu paikallisesti.**
Raportti `globaali-tila.md`; raakatulokset `todisteet/c8/`; skripti `tyokalut/globaalit.py`.
Uudet H-128–H-144 (korkea 1: H-129 kekoylivuoto vaiheen vaihdossa; keskitaso–korkea 1: H-128
`setpos`; keskitaso 2; muut matalampia) ja T-038–T-039; Q78–Q82. Kaikki staattisia.

**C-8 valmis ja suljettu (2026-11-26), käyttäjä hyväksyi** rajattujen tarkistusten jälkeen
(`globaali-tila.md` luvut 3.3, 11, 12): lukumäärät täsmäytetty; H-128 ja H-129:n mekanismi
toistettu eristetyllä koeohjelmalla `todisteet/c8/koeohjelmat/` (ei toimitetussa ohjelmassa);
H-129 korjausjonoon P2 (alustava); poikkeamat ja jatkotehtävät `seuranta.md`.

**C-9 vaihe 1 valmis (2026-11-26), riippumattomasti tarkastettu, commitoitu paikallisesti.**
Raportti `virheenkasittely.md`; raakatulokset ja raportit `todisteet/c9/`; skripti
`tyokalut/resurssit.py` (v2). Uudet H-145–H-157 (keskitaso 9, matala 4) ja T-040–T-041; Q83–Q90;
H-044, H-071 ja H-105 laajennettu. Kaikki staattisia.

**C-9 vaihe 2 valmis (2026-11-26), riippumattomasti tarkastettu, commitoitu paikallisesti.**
Eristetyt koeohjelmat `todisteet/c9/vaihe2/koeohjelmat/` (C9ProbeHk, C9ProbeJuk; Release ja Asan;
käännös repositorion ulkopuolelle `C:\temp\c9build`; ajo `aja.py`, lopullinen `tulokset/ajo5.txt`)
ja bcc32-RTL-koe. H-149 toistettu ja nostettu korkeaksi (konsolin viestiohjelmat kaatuvat
ensimmäiseen automaattiseen HTML-tulosteeseen); H-145, H-146, H-147, H-150, H-151 toistettu
testikehyksessä; uudet H-158, T-042, Q91–Q95; Q83, Q84, Q85, Q88 ratkaistu. Tulkinta
`virheenkasittely.md` luku 12.

**C-9 valmis ja suljettu (2026-11-26), käyttäjä hyväksyi** (`virheenkasittely.md` luku 13):
kattavuus tarkistettu rajausta vasten; todentamisen rajat ja MSVC/C++Builder-erot kirjattu
(koeohjelmien kaatumisia ei yleistetä toimitettuihin eikä C++Builder-ohjelmiin); lopullinen
korjausjärjestys P2 H-149, H-150 (P3 → P2 tiedostovirtojen loppumisen perusteella), P3 H-145,
H-146, H-151, H-147; korjauskokonaisuudet KK-C9-1 (`PRFILE`-elinkaari, T-042: H-149, H-151,
H-152, H-158), KK-C9-2 (tallennusvirheiden välitys ja palautus, T-040, T-041), KK-C9-3
(resurssit); jatkotehtävät `seuranta.md`.

**C-10 vaihe 1 valmis (2026-11-26), riippumattomasti tarkastettu, commitoitu paikallisesti.**
Raportti `riippuvuudet.md` luvut 7–18 (kierroksen 1 osuus korjattu); skripti
`tyokalut/riippuvuudet.py` (tarkistettu C-5:n tallentamia käännöksiä vasten ennen agenttikäyttöä,
viisi skriptivirhettä korjattu); raakatulos ja raportit `todisteet/c10/`. Uudet H-159–H-170
(korkea H-159 Sirit/Zebra-uudelleenyhteys; H-160 korkea SQL-peilausta käytettäessä) ja T-043–T-049;
T-005 tarkennettu; Q5 ja Q92 ratkaistu, Q86 ja Q91 osittain, uudet Q96–Q104. Suunnitelmaan
lisätty uudet erät C-12 (ominaisuusvalitsimet) ja C-13 (synteesi ja korjaussuunnitelma), ei
aloitettu. Kaikki staattisia.

**C-10 suljettu (2026-11-26)** käyttäjän hyväksynnällä ilman toista työerää (`riippuvuudet.md`
luku 19): kattavuus tarkistettu, ajonaikaisesti todentamattomat merkitty, prioriteetit P2 H-159 ja
H-160 (P3 → P2), P3 H-162, H-169, H-164, H-166, H-163, H-161, H-165, P4 H-167, H-168, P5 H-170;
korjauskokonaisuudet KK-C10-1 (SQL-integraatio), KK-C10-2 (laiteyhteydet), KK-C10-3 (jakelu),
KK-C10-4 (yhteyksien turvallisuus); lisenssit omistajan päätökseen OM-C10-1; tuoteinventaario
täsmäytetty (5 ohjelmaa ja 4 kirjastoa, testi-EXE:t rajattu pois) ja `/MT`-johtopäätös tarkennettu;
uudet Q105, Q106. C-12:n vaihe 1 rajattu (`suunnitelma.md`, "C-12 vaihe 1: rajaus").

**C-12 vaihe 1 valmis (2026-11-26), riippumattomasti tarkastettu, commitoitu paikallisesti.**
Raportti `ominaisuusvalitsimet.md`; rekisterit `rekisteri/valitsimet.csv`, `rekisteri/asetusavaimet.csv`;
skriptit `tyokalut/valitsimet.py`, `asetusavaimet.py`, `valitsinrekisteri.py`; `riippuvuudet.py`:hin
lisätty `build_model()` (C-10:n tulosteet ennallaan). Uudet H-171 (ViestiWin `cfgfilename`-ylitys joka
käynnistyksessä), H-172 (viestin konsolin X)ML lähes tyhjä), H-173–H-177, T-050–T-052; Q2 ratkaistu.

**C-12 suljettu (2026-11-26)** käyttäjän hyväksynnällä ilman toista työerää (`ominaisuusvalitsimet.md`
luku 13): H-171 korkea ja P2; P3 H-134 (H-129:n kanssa), H-172, H-173, H-174; P4 H-175; P5 H-176, H-177;
KK-C12-1 julkaisutiedostot, KK-C12-2 asetusten hallinta, KK-C12-3 valitsinhygienia; Q107, Q108, Q110
ajonaikaiseen todennukseen; Q111 omistajalle; Q112 avoin.

**C-13 valmis ja suljettu (2026-11-26), riippumattomasti tarkastettu, commit `e8daa65`; käyttäjä
hyväksyi.** Osat A0 ja A–I tehty rajauksen mukaan (`suunnitelma.md`, "C-13 vaihe 1:
rajaus"). Tulokset: `auditoinnin-yhteenveto.md` (kattavuus, juurisyyt, riskimalli, valmiusehdot,
tarkastuksen käsittely), `korjaussuunnitelma.md` (34 kokonaisuutta, järjestys, D-vaiheen viisi
vaihetta, ensimmäinen toteutuserä), `regressiotestaussuunnitelma.md`, `omistajan-paatokset.md`
(OP-1–OP-18). Mitään ei korjattu, käännetty eikä ajettu; D-vaihetta ei aloitettu.

**VAIHE C SULJETTU (2026-11-26)** käyttäjän päätöksellä. Kaikki kahdeksan sulkemisen edellytystä
täyttyvät (`auditoinnin-yhteenveto.md` luku 12, `seuranta.md`). Sulkeminen ei tarkoita, että koko
lähdekoodi olisi katselmoitu: 474 tiedostoa ja 36 % riveistä on vain inventoitu, yhtäkään
tuotantotiedostoa ei ole luettu kokonaan, GUI-lomakkeet ovat pääosin lukematta, ja havaintojen
määrä on alaraja.

**SEURAAVA TYÖVAIHE / NEXT STEP:**

1. **Vaihe K** (ohjelmiston tietopohja ja käsikirja) käyttäjän aloitusluvalla:
   `k-vaihe-suunnitelma.md`. Osat K-0 (rekisterien täydennys), K-1 (agentti `pirila-expert` ja
   skill `pirila-ask`), K-2 (229 selkokielistä vikakorttia), K-3 (toiminnallinen käsikirja), K-4
   (paikallinen staattinen portaali), K-5 (lopputarkastus). **Tila 2026-11-27:** K-0 ja K-1
   valmiit; K-2:n pilotti (H-033, H-041, H-071, H-093, H-099, H-109, H-129, H-145, H-159, H-171,
   T-038, T-042) Opus-tarkastettu ja korjattu, odottaa käyttäjän hyväksyntää ennen muita 217
   korttia (`doc/tietopohja/`). Agentti ja skill latautuvat vasta uudessa istunnossa
   (`doc/tietopohja/laatu/tarkastukset/k1-testiajo.md`).
2. **D-vaiheen ennakkotehtävät** (`korjaussuunnitelma.md` luku 9), ei aloitettu: E-1 rajattu
   riskiseulonta 62 riville (H-033:n perusteella), E-2 haarojen `origin/88`, `origin/90`,
   `origin/92` ja `origin/95` täsmäytys (ei yhdistetä, ei haeta verkosta), E-3 aineiston
   yksityinen varmuuskopio, E-4 päätökset OP-1–OP-3.
3. **D-vaihe** (korjaukset) vasta erillisellä luvalla `korjaussuunnitelma.md`:n luvun 7 pohjalta.

**C-11 complete.** 15 findings reproduced in the test harness, 1 confirmed in the shipped
program (H-041, by a user-approved deviation from the pre-fixed verdict), 39 static only.
Open: manual tests H-009/H-014, two-process loopback layer, H-041 read path, CI. Next:
C-4 memory safety (plan batch 4). **C-4 complete and closed:** step 1 (`81bcad9`) and step 2 committed locally. Urgent
fixes: H-071, H-072, H-055, H-070. **C-5 step 1 complete** (`kaantajaerot.md`, H-078–H-081, T-023–T-028);
**C-5 step 2 complete** (Q8 resolved, H-082–H-086, T-029). **C-5 closed** on approval.
**C-6 step 1 complete** (`saikeet-ja-lukot.md`, H-087–H-102, T-030–T-032, Q30 resolved).
**C-6 step 2 complete** (section 13; probes for H-090, H-091, H-093, H-094; H-099 refuted;
new H-103–H-108, T-033). **C-6 closed** on approval; open items needing runs deferred.
**C-7 step 1 complete** (`gui-liiketoimintalogiikka.md`, H-109–H-127, T-034–T-037; H-017
narrowed; 9 parse candidates resolved). **C-7 closed** on approval after a coverage check;
gaps in follow-up queue J7-1–J7-7. **C-8 step 1 complete** (`globaali-tila.md`, H-128–H-144, T-038–T-039, Q78–Q82;
H-129 high). **C-8 closed** on approval (counts reconciled; H-128 and H-129 mechanism reproduced with an
isolated probe, not in a shipped program; H-129 in the fix queue as P2). **C-9 step 1 complete**
(`virheenkasittely.md`, H-145–H-157, T-040–T-041, Q83–Q90; nothing high on static evidence, H-149
relay `autofile` double free may be high, Q85). **C-9 step 2 complete**
(isolated probes, `todisteet/c9/vaihe2/`; H-149 raised to high, six findings reproduced in the
test harness; H-158, T-042, Q91–Q95). **C-9 closed** on approval (section
13: limits of verification, final priority with H-150 moved to P2, fix packages KK-C9-1..3).
**C-10 step 1 complete** (`riippuvuudet.md` sections 7–18, script `tyokalut/riippuvuudet.py`
validated before agent use; H-159–H-170 with H-159 high and H-160 high when SQL is used,
T-043–T-049; Q5 and Q92 resolved, Q86 and Q91 partly, Q96–Q104 new). New batches C-12 (feature
switches) and C-13 (synthesis and fix plan) added to the plan, not started. **C-10 closed** on
approval without a step 2 (section 19: final priorities with H-160 moved to P2, fix packages
KK-C10-1..4, licences to owner decision OM-C10-1, product inventory reconciled, `/MT` refined,
Q105, Q106). **C-12 complete and closed** (`ominaisuusvalitsimet.md`; H-171..H-177, T-050..T-052;
H-171 high and P2). **C-13 complete, awaiting the user's approval**: synthesis of phase C (machine base,
coverage, 15 root causes, uniform risk assessment with P1 5 / P2 35 / P3 62 / P4 47 / P5 27, 34 fix
packages, dependency model and waves, regression test plan, five-stage development plan, first
implementation batch, owner decisions OP-1..OP-18; independent review K1..K17 handled, H-033 raised
to P1). Nothing fixed, built or run. **Phase C was closed on 2026-11-26 by the user's decision**; the
fix and test plans are approved as the starting point of phase D, which is not permission to start
fixing. Closing does not mean the code base was fully reviewed (36 % of the lines are inventory
only; no production file read in full). Next: phase K (knowledge base, `k-vaihe-suunnitelma.md`,
planned, not started) on the user's go-ahead, and the preparatory tasks E-1..E-4
(`korjaussuunnitelma.md` section 9). The audit branch must not be pushed to a public repository
without separate approval.

## Resurssien käyttö / Resource usage (C-13)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `architecture-analyst` (Sonnet; A kattavuus / coverage) | 365 458 | 107 |
| `architecture-analyst` (Sonnet; B inventointi / inventory) | 256 555 | 49 |
| `build-dependency-analyst` (Sonnet; F testausaineisto / test assets) | 372 601 | 83 |
| `doc-writer` (Sonnet; yhteenvedon luvut 1–6 / summary sections 1–6) | 71 935 | 7 |
| `architecture-analyst` (Opus; B juurisyyt / root causes) | 297 393 | 67 |
| `architecture-analyst` (Opus; C riskiarvio / risk assessment) | 282 967 | 55 |
| `legacy-encoding-analyst` (Opus; yhteensopivuuskysymykset / compatibility questions) | 159 460 | 82 |
| `independent-verifier` (Opus; H kokonaisarvio / overall review) | 324 582 | 106 |
| **Yhteensä / total** | **2 130 951** | **556** |

Käyttäjän ohjeella ei tokenbudjettia; enintään kolme analyysiagenttia rinnakkain. Pääistunto
(Fable) teki A0:n, korjauskokonaisuudet, järjestyksen, kehittämissuunnitelman, testaussuunnitelman,
omistajan päätösten koonnin ja agenttien ehdotusten arvioinnin. / No token budget by the user's
instruction; at most three analysis agents in parallel; packages, order, development plan, test
plan, owner decisions and the assessment of agent proposals by the main session.

## Resurssien käyttö / Resource usage (C-10 vaihe 1 / step 1)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `build-dependency-analyst` (Sonnet; A luokittelu / classification) | 185 942 | 64 |
| `architecture-analyst` (Sonnet; B tietokanta / database) | 169 255 | 53 |
| `defect-concurrency-auditor` (Sonnet; C HTTP ja selain / HTTP and browser) | 122 096 | 29 |
| `architecture-analyst` (Sonnet; D laitteet / devices) | 280 945 | 94 |
| `build-dependency-analyst` (Sonnet; E paketit ja jakelu / packages and deployment) | 230 964 | 94 |
| `independent-verifier` (Opus; F, K1–K15) | 208 904 | 105 |
| **Yhteensä / total** | **1 198 106** | **439** |

Käyttäjän ohjeella ei tokenbudjettia; skripti, sen tarkistus ja dokumentointi pääistunnossa. /
No token budget by the user's instruction; script, its validation and the write-up by the main session.

## Resurssien käyttö / Resource usage (C-6 vaihe 2 / step 2)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `defect-concurrency-auditor` (Opus; viisi havaintoa, Q66, Q69, Q74 / five findings) | 226 299 | 120 |
| `defect-concurrency-auditor` (Sonnet; luku 9, Q67, Q70–Q72 / section 9) | 169 166 | 68 |
| `architecture-analyst` (Sonnet; VCL) | 145 673 | 65 |
| `general-purpose` (Opus; koeohjelmat / probes) | 259 871 | 88 |
| `independent-verifier` (Opus; väitteet / claims C1–C11) | 98 458 | 70 |
| `independent-verifier` (Opus; koeohjelmat / probes) | 116 123 | 42 |
| **Yhteensä / total** | **1 015 590** | **453** |

Käyttäjän ohjeella ei tokenrajoja. Q65-koe, projektipohja, ajuri ja dokumentointi pääistunnossa. /
No token limits by the user's instruction. Q65 probe, project, runner and write-up by the main session.

## Resurssien käyttö / Resource usage (C-6 vaihe 1 / step 1)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `architecture-analyst` (Sonnet; A + D säiekartta, elinkaari / thread map, lifecycle) | 315 799 | 162 |
| `defect-concurrency-auditor` (Sonnet; B + C lukkokartta, järjestys / lock map, order) | 378 168 | 131 |
| `independent-verifier` (Opus; L1–L10) | 122 321 | 81 |
| `independent-verifier` (Opus; T1–T10) | 133 585 | 65 |
| **Yhteensä / total** | **949 873** | **439** |

Käyttäjän ohjeella ei tokenbudjettia tälle erälle. Raakadata ja dokumentointi pääistunnossa. /
No token budget for this batch by the user's instruction. Raw data and write-up by the main session.

## Resurssien käyttö / Resource usage (C-5 vaihe 2 / step 2)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `defect-concurrency-auditor` (2B, 2C) | 276 656 | 120 |
| `legacy-encoding-analyst` (2D: Q57–Q59, Q61, Q62) | 207 063 | 118 |
| `independent-verifier` (A–D) | 178 522 | 101 |
| **Yhteensä / total** | **662 241** | **339** |

Tehtävä 2A (bcc32-vertailu) ja dokumentointi pääistunnossa. / Task 2A and write-up by the main session.

## Resurssien käyttö / Resource usage (C-5 vaihe 1 / step 1)

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `build-dependency-analyst` (optiomatriisi, ehdollinen koodi, bcc32-pakkaustila / option matrix, conditional code, pack state) | 305 039 | 139 |
| `legacy-encoding-analyst` (merkistö, muotoilu, tyypit, `char`, RTL / encoding, formatting, types, RTL) | 301 102 | 180 |
| `independent-verifier` (C1–C10) | 293 523 | 158 |
| **Yhteensä / total** | **899 664** | **477** |

MSVC-asetteluraportti, korjattu literaalihaku ja dokumentointi tehtiin pääistunnossa. / MSVC layout run, literal search
and write-up by the main session.

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
   `doc/arkkitehtuuri/auditoinnin-yhteenveto.md`, `doc/arkkitehtuuri/korjaussuunnitelma.md` ja
   rekisterit `doc/arkkitehtuuri/rekisteri/` (havainnot, riskiarvio, korjauskokonaisuudet). Muut
   dokumentit tarpeen mukaan / other documents as needed (`doc/arkkitehtuuri/README.md`).
2. Tarkista tila / check state:
   ```sh
   git branch --show-current          # testit/vikojen-todentaminen
   git status --short                 # puhdas / clean
   git log --oneline main..           # 36 committia / commits (viimeisin / last: vaiheen C sulkeminen / closing of phase C)
   python -I TPsource/V52/Tests/test_run_core_tests.py   # 46 OK
   python -I TPsource/V52/Tests/run_core_tests.py   # viimeisin tallennettu ajo / last stored run 2026-11-24: PASS=23, XFAIL=24 (vaatii Release- ja Asan-käännökset / needs the Release and Asan builds, testaus.md 13.6, 14; ei ajettu C-13:ssa / not run in C-13)
   python -I doc/arkkitehtuuri/tyokalut/tarkista_auditointi.py .   # OK
   python -I doc/arkkitehtuuri/tyokalut/synteesi.py . <tmp>/c13 --tarkista   # OK (rekisterit / registers)
   ```
3. Rajoitukset (voimassa, kunnes käyttäjä toisin sanoo) / constraints (in force
   until the user says otherwise):
   - Ei pushia, PR:iä eikä GitHub-issueita. `.claude/settings.local.json` estää
     pushin ja PR:n luonnin myös kiertoteitse (83 deny-sääntöä, `doc/tietopohja/laatu/tarkastukset/k1-testiajo.md` luku 6).
     Tiedosto on `.gitignore`:ssa; älä commitoi sitä. /
     No push, PRs or GitHub issues; deny rules in `.claude/settings.local.json`
     (the file is in .gitignore; do not commit it).
   - Ei muutoksia `TPsource/`-hakemistoon ilman erillistä lupaa; vikakorjaukset
     omiin haaroihinsa, ei dokumentaation kanssa. / No `TPsource/` changes without
     separate approval; bug fixes in their own branches, not with docs.
   - Ei massakommentointia. / No bulk commenting.
   - D-vaihetta (korjauksia) ja K-vaiheen toteutusta ei aloiteta ilman erillistä lupaa. /
     Neither phase D (fixes) nor the implementation of phase K starts without separate permission.
   - Haaroja `origin/88`, `90`, `92`, `95` ei yhdistetä eikä verkosta haeta ennen päätöstä. /
     No merge of those branches and no fetch before a decision.
   - Commitit vain paikallisesti ja pyydettäessä. / Commits only locally and when asked.
4. Kielisäännöt / language rules: dokumentit kaksikielisinä samassa tiedostossa,
   suomi ensin; uudet koodikommentit englanniksi ASCII-merkein; Pekan kommentit
   säilytetään (`dokumentointistandardi.md`, `merkistot.md`). / Docs bilingual,
   Finnish first; new code comments English ASCII; keep Pekka's comments.
5. Agentit / agents (`.claude/agents/`): `architecture-analyst`,
   `defect-concurrency-auditor`, `independent-verifier` (Opus);
   `build-dependency-analyst`, `legacy-encoding-analyst`, `doc-writer` (Sonnet).
   Enintään kolme analyysiagenttia rinnakkain; erä päättyy rekisterin päivitykseen.
   C-6:sta alkaen: Sonnet oletuksena (agenttikutsun `model: sonnet`), Opus vain
   perustellusti, rajaus ja tokenbudjetti etukäteen, olemassa olevat todisteet ensin,
   riippumaton tarkastus riskiperusteisesti (`suunnitelma.md`). / At most three analysis
   agents in parallel; from C-6: Sonnet by default, Opus only when justified, scope and
   token budget up front, existing evidence first, risk-based independent review.
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
