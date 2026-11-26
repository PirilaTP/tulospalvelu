# Seuranta / Tracking

Päivitä tätä tiedostoa jokaisen työvaiheen lopuksi. Dokumentointikattavuutta
seurataan erikseen testikattavuudesta.
Update this file at the end of every work step. Documentation coverage is tracked
separately from test coverage.

## Tila / Status

| Vaihe / Phase | Tila / State | Päivä / Date |
|---|---|---|
| A. Selvitys, kierros 1 (inventaario, yleiskuva, merkistöt, suunnitelma) / audit round 1 | valmis, odottaa katselmointia / done, awaiting review | 2026-11-24 |
| Auditointirekisteri ja projektin agentit / audit register and project agents (`rekisteri/`, `.claude/agents/`) | valmis / done | 2026-11-24 |
| B. Vikakorjaukset / bug fixes | ei aloitettu / not started; sisältyy D-vaiheeseen (`korjaussuunnitelma.md`) / part of phase D | – |
| C, erä 1: `tallenna()`-samanaikaisuus / batch 1: `tallenna()` concurrency | valmis, tarkastettu / done, reviewed (`tallenna-samanaikaisuus.md`); **suljettu / closed 2026-11-26** vaiheen C sulkemispäätöksellä | 2026-11-24 |
| C, erä 2: työasemien tiedonsiirto / batch 2: workstation communication | valmis, tarkastettu / done, reviewed (`tiedonsiirto.md`); **suljettu / closed 2026-11-26** vaiheen C sulkemispäätöksellä | 2026-11-24 |
| C, erä 3: pysyvät tiedostot ja eheys / batch 3: persistent files and integrity | valmis, tarkastettu / done, reviewed (`tiedostomuodot.md`); **suljettu / closed 2026-11-26** vaiheen C sulkemispäätöksellä | 2026-11-24 |
| C, erä 11: testiautomaatio, suunnittelu / batch 11: test automation, planning | valmis, tarkastettu, commitoitu `f5d793b` / done, reviewed, committed (`testaus.md`) | 2026-11-24 |
| C, erä 11, vaihe 1: H-025, H-040, H-042 / batch 11, stage 1 | valmis, hyväksytty, commitoitu `79d9957` / done, approved, committed (`testaus.md` luku 13) | 2026-11-24 |
| C, erä 11, vaihe 2: H-041, H-031, H-008, H-024, H-027 / batch 11, stage 2 | valmis, hyväksytty, commitoitu `cad2781` / done, approved, committed (`testaus.md` luku 14) | 2026-11-24 |
| C, erä 11, vaihe 3: H-026, H-032, H-022, H-023, H-007, H-001, H-002 / batch 11, stage 3 | valmis, hyväksytty, commitoitu / done, approved, committed (`testaus.md` luku 15) | 2026-11-24 |
| HkMaali.exe:n koeajot 0 ja 1 / black-box runs 0 and 1 | tehty palomuurieristyksellä, molemmat odotetusti; ensimmäinen AppContainer-yritys keskeytyi; ei H-vahvistuksia; done under firewall isolation, both as expected; first AppContainer attempt stopped; no H confirmed (`432ea3d`, `testaus.md` luku 17) | 2026-11-24 |
| C-11 vaihe 4: H-041 toimitetussa ohjelmassa / stage 4 | esiehdot `c6205d2`; ajot 220/221; ennalta kiinnitetty arvio "not confirmed" (kriteeri 7, loki-ikkuna); käyttäjän poikkeamapäätöksellä H-041 **vahvistettu ajamalla** / pre-fixed verdict not confirmed; H-041 confirmed by running by user-approved deviation (`testaus.md` 19.9–19.11) | 2026-11-24 |
| **C, erä 11 (C-11) kokonaisuutena / batch 11 as a whole** | **valmis ja suljettu / complete and closed** (päätös / decision 2026-11-24, vahvistettu vaiheen C sulkemisessa 2026-11-26); loppuyhteenveto / final summary `testaus.md` luku 20; raakatulokset / raw results `todisteet/c11/` | 2026-11-24 |
| **C, erä 4 (C-4) valmis / complete**, vaihe 2: syväanalyysi, tutkittavat rivit, muunnokset, kattavuus, kokonaisarvio / step 2 | **valmis, hyväksytty, suljettu, commitoitu** / done, approved, closed, committed (`muistiturvallisuus.md` luvut 9–17, H-070–H-077, T-022) | 2026-11-25 |
| C, erä 4 (C-4) muistiturvallisuus, vaihe 1: varoitusinventaario / memory safety, step 1: warning inventory | valmis, hyväksytty, riippumattomasti uudelleentarkastettu, commitoitu / done, approved, re-reviewed, committed (`muistiturvallisuus.md`, H-055–H-069, T-020, T-021, `todisteet/c4/`) | 2026-11-24 |
| **C, erä 5 (C-5) kääntäjäkohtainen käytös / compiler-specific behaviour, vaihe 1 / step 1** | **valmis, riippumattomasti tarkastettu, commitoitu** / done, independently reviewed, committed (`kaantajaerot.md`, H-078–H-081, T-023–T-028, H-004/H-005 laajennettu / extended, `todisteet/c5/`) | 2026-11-25 |
| C, erä 5 (C-5), vaihe 2 / step 2 | **valmis, riippumattomasti tarkastettu, commitoitu** / done, independently reviewed, committed `55897eb` (`kaantajaerot.md` luku 13: Q8 ratkaistu, `tyokalut/vertaa_omf.py`, H-082–H-086, T-029, H-042/H-079 laajennettu / extended; `todisteet/c5/vaihe2/`) | 2026-11-25 |
| **C, erä 5 (C-5) kokonaisuutena / batch 5 as a whole** | **valmis ja suljettu käyttäjän hyväksynnällä / complete and closed on approval** (`b17d536`, `55897eb`; sulkeminen `kaantajaerot.md` 13.7); avoinna Q60 (omistaja / owner), Q63, Q64; jatkotehtävät alla / follow-ups below | 2026-11-25 |
| **C, erä 6 (C-6) säikeet ja synkronointi, vaihe 1 / threads and synchronisation, step 1** | **valmis, riippumattomasti tarkastettu, commitoitu paikallisesti / done, independently reviewed, committed locally** (`saikeet-ja-lukot.md`, H-087–H-102, T-030–T-032, H-018/H-031/H-038 laajennettu / extended, Q30 ratkaistu / resolved, `todisteet/c6/`); vaihe 2 erillisellä päätöksellä / step 2 on a separate decision | 2026-11-25 |
| **C-6 vaihe 2 / step 2** (todennus ja jatkoanalyysi / verification and follow-up) | **valmis, riippumattomasti tarkastettu, commitoitu paikallisesti / done, independently reviewed, committed locally** (`saikeet-ja-lukot.md` luku 13; H-090, H-091 reunat, H-093, H-094 toistettu eristetyllä koeohjelmalla / reproduced with isolated probes, `todisteet/c6/vaihe2/`; H-090, H-093 → korkea; H-099 kumottu / refuted; uudet / new H-103–H-108, T-033; Q65, Q66, Q69, Q70–Q72, Q74 ratkaistu / resolved, Q67 osittain / partly); **C-6 suljettu / closed 2026-11-25** käyttäjän hyväksynnällä / with the user's approval; avoimet asiat jatkotehtävinä (alla / below) | 2026-11-25 |
| **C, erä 7 (C-7) GUI:n liiketoimintalogiikka, vaihe 1 / GUI business logic, step 1** | **valmis ja suljettu käyttäjän hyväksynnällä / done and closed on approval** (`e3a8b67`; `gui-liiketoimintalogiikka.md`; H-109–H-127, T-034–T-037; H-017 kavennettu / narrowed, H-085 ja H-069 laajennettu / extended; 9 jäsennysehdokasta ratkaistu / parse candidates resolved; Q15 osittain / partly, uudet / new Q75–Q77; `todisteet/c7/`); kattavuus ja jatkotyöjono J7-1–J7-7 luku 13 / coverage and follow-up queue in section 13 | 2026-11-25 |
| **C, erä 8 (C-8) globaali muuttuva tila, vaihe 1 / global mutable state, step 1** | **valmis ja suljettu käyttäjän hyväksynnällä / done and closed on approval** (`4884374` + sulkemiscommit / closing commit; `globaali-tila.md` luvut 11–12; H-128–H-144, T-038–T-039; Q78–Q82; H-128 ja H-129:n mekanismi toistettu eristetyllä koeohjelmalla / reproduced with an isolated probe; `todisteet/c8/`); jatkotehtävät alla / follow-ups below | 2026-11-26 |
| **C, erä 9 (C-9) virheenkäsittely ja resurssit, vaihe 1 / error handling and resources, step 1** | **valmis, riippumattomasti tarkastettu, commitoitu paikallisesti / done, independently reviewed, committed locally** (`virheenkasittely.md`; H-145–H-157, T-040–T-041, Q83–Q90; H-044, H-071, H-105 laajennettu / extended; skripti / script `tyokalut/resurssit.py`; `todisteet/c9/`; noin 1,22 milj. tokenia agenteille / about 1.22 M agent tokens); odottaa käyttäjän katselmointia / awaiting the user's review | 2026-11-26 |
| **C, erä 9 (C-9), vaihe 2: todentaminen eristetyillä koeohjelmilla / step 2: isolated probes** | **valmis, riippumattomasti tarkastettu, commitoitu paikallisesti / done, independently reviewed, committed locally** (`virheenkasittely.md` luku 12; `todisteet/c9/vaihe2/`; H-145, H-146, H-147, H-149, H-150, H-151 toistettu testikehyksessä / reproduced in test harness; H-149 → korkea / high; uudet / new H-158, T-042, Q91–Q95; Q83, Q84, Q85, Q88 ratkaistu / resolved); **C-9 valmis ja suljettu käyttäjän hyväksynnällä / C-9 done and closed on approval** (sulkeminen / closure `virheenkasittely.md` luku 13: kattavuus, todentamisen rajat, H-150 P3 → P2, korjauskokonaisuudet KK-C9-1–3) | 2026-11-26 |
| **C, erä 10 (C-10) ulkoiset riippuvuudet, vaihe 1 / external dependencies, step 1** | **valmis, riippumattomasti tarkastettu, commitoitu paikallisesti / done, independently reviewed, committed locally** (`riippuvuudet.md` luvut 7–18; skripti / script `tyokalut/riippuvuudet.py`, tarkistettu ennen agenttikäyttöä / validated before agent use; H-159–H-170, T-043–T-049; Q5 ja Q92 ratkaistu / resolved, Q86 ja Q91 osittain / partly, T-005 tarkennettu / refined; Q96–Q104; `todisteet/c10/`; noin 1,20 milj. tokenia agenteille / about 1.20 M agent tokens); **C-10 valmis ja suljettu käyttäjän hyväksynnällä ilman toista työerää / C-10 done and closed on approval without a step 2** (sulkeminen / closure `riippuvuudet.md` luku 19: kattavuus, todentamisen rajat, H-160 P3 → P2, korjauskokonaisuudet / fix packages KK-C10-1–4, lisenssit omistajalle / licences to the owner OM-C10-1, tuoteinventaarion täsmäytys / product inventory reconciliation; Q105, Q106) | 2026-11-26 |
| **C, erä 12 (C-12) ominaisuusvalitsimet, vaihe 1 / feature switches, step 1** | **valmis, riippumattomasti tarkastettu, commitoitu paikallisesti / done, independently reviewed, committed locally** (`ominaisuusvalitsimet.md`; skriptit / scripts `tyokalut/valitsimet.py`, `asetusavaimet.py`, `valitsinrekisteri.py`, tarkistettu ennen agenttikäyttöä / validated before agent use; rekisterit / registers `rekisteri/valitsimet.csv`, `rekisteri/asetusavaimet.csv`; H-171–H-177, T-050–T-052; H-036 ja H-134 laajennettu / extended (H-134 → keskitaso / medium); T-008 arvioitu / assessed; Q2 ratkaistu / resolved; Q107–Q112; `todisteet/c12/`; noin 0,73 milj. tokenia agenteille / about 0.73 M agent tokens); **C-12 valmis ja suljettu käyttäjän hyväksynnällä ilman toista työerää / C-12 done and closed on approval without a step 2** (sulkeminen / closure `ominaisuusvalitsimet.md` luku 13: kattavuus ja rekisterien johdonmukaisuus tarkistettu, H-171 → korkea ja P2 / high and P2, H-134 kytketty H-129:ään / linked to H-129, korjauskokonaisuudet / fix packages KK-C12-1–3, Q107, Q108, Q110 ajonaikaiseen todennukseen / to run-time verification, Q111 omistajalle / to the owner, Q112 avoin / open) | 2026-11-26 |
| **C, erä 13 (C-13) synteesi ja korjaussuunnitelma / synthesis and fix plan** | **valmis ja suljettu käyttäjän hyväksynnällä 2026-11-26 / done and closed on the user's approval** (commit `e8daa65`) (`auditoinnin-yhteenveto.md`, `korjaussuunnitelma.md`, `regressiotestaussuunnitelma.md`, `omistajan-paatokset.md`; rekisterit / registers `rekisteri/riskiarvio.csv`, `rekisteri/korjauskokonaisuudet.csv`; skripti / script `tyokalut/synteesi.py`, tarkistettu ennen agenttikäyttöä / validated before agent use; 15 juurisyytä / root causes; P1 5, P2 35, P3 62, P4 47, P5 27; 34 korjauskokonaisuutta / fix packages; OP-1–OP-18; Opus-tarkastus K1–K17 käsitelty / handled, H-033 → P1; H-116 ja H-052 laajennettu / extended; Q3, Q24, Q26, Q53 ratkaistu / resolved; `todisteet/c13/`; noin 2,13 milj. tokenia agenteille / about 2.13 M agent tokens). **Vaihe C suljettu / phase C closed 2026-11-26** | 2026-11-26 |
| **Vaihe C kokonaisuutena / phase C as a whole** (erät / batches C-1–C-13) | **suljettu käyttäjän päätöksellä / closed by the user's decision**; kattavuusrajoitteet `auditoinnin-yhteenveto.md` luvut 3, 10 ja 12: koko lähdekoodia ei ole katselmoitu / the code base has not been fully reviewed | 2026-11-26 |
| D-vaiheen ennakkotehtävät / preparatory tasks E-1–E-4 (riskiseulonta, haarojen täsmäytys, aineiston säilytys, OP-1–OP-3) | kirjattu, ei aloitettu / recorded, not started (`korjaussuunnitelma.md` luku 9) | 2026-11-26 |
| K. Ohjelmiston tietopohja ja käsikirja / knowledge base and handbook (K-0–K-5) | suunniteltu, ei aloitettu; odottaa aloituslupaa / planned, not started; awaits the go-ahead (`k-vaihe-suunnitelma.md`) | 2026-11-26 |
| D. Kehittämisvaihe / development phase (korjaukset `korjaussuunnitelma.md`:n mukaan; alkuperäisen suunnitelman kommentointi kuuluu sen vaiheeseen 3 / fixes per the fix plan; the commenting of the original plan belongs to its stage 3) | ei aloitettu; suunnitelma hyväksytty lähtökohdaksi, ei lupaa aloittaa / not started; plan approved as the starting point, no permission to start | – |

**Vaihe C on suljettu 2026-11-26 käyttäjän päätöksellä.** Sulkemisen edellytykset
(`suunnitelma.md`, "C-13 vaihe 1: rajaus") ja niiden toteutuma:

| # | Edellytys / Condition | Toteutuma / Outcome |
|--:|---|---|
| 1 | C-13 valmis ja käyttäjän hyväksymä | kyllä (commit `e8daa65`; hyväksyntä 2026-11-26) |
| 2 | Jokaisen erän tila "suljettu" | kyllä: C-4–C-10 ja C-12 aiemmilla päätöksillä, C-11 päätöksellä 2026-11-24, C-1–C-3 ja C-13 vaiheen C sulkemispäätöksellä |
| 3 | Avoimet kysymykset luokiteltu | kyllä: 32 ratkaistu, 39 ajonaikainen todennus (D), 16 omistajan päätös, 25 hyväksytty avoimiksi (`auditoinnin-yhteenveto.md` luku 6) |
| 4 | Korjaus- ja testaussuunnitelma hyväksytty D-vaiheen lähtökohdaksi | kyllä; ei lupa aloittaa korjauksia |
| 5 | Omistajan päätöslista toimitettu | kyllä (`omistajan-paatokset.md`); kaikki 18 kohtaa avoinna, OP-1–OP-3 ennen D-vaihetta |
| 6 | `CHECKPOINT.md` ja `README.md` kuvaavat lopputuloksen | kyllä |
| 7 | Validaattori menee läpi | kyllä |
| 8 | Kaikki commitoitu paikallisesti | kyllä; ei pushia |

Sulkeminen ei tarkoita, että koko lähdekoodi olisi katselmoitu: 36 % riveistä on vain inventoitu,
yhtäkään tuotantotiedostoa ei ole luettu kokonaan, ja havaintojen määrä on alaraja
(`auditoinnin-yhteenveto.md` luku 12).

**Seuraava askel / Next step:** (1) vaihe K käynnissä (2026-11-27): K-0 ja K-1 valmiit, K-2:n pilotti
odottaa käyttäjän hyväksyntää (`k-vaihe-suunnitelma.md`, `doc/tietopohja/`); (2) D-vaiheen ennakkotehtävät E-1–E-4
(`korjaussuunnitelma.md` luku 9): rajattu riskiseulonta 62 riville, haarojen `origin/88`, `90`,
`92` ja `95` täsmäytys, aineiston yksityinen varmuuskopio ja päätökset OP-1–OP-3. D-vaihetta ei
aloiteta ennen erillistä lupaa. Auditointihaaraa ei pushata julkiseen repositorioon ilman
erillistä hyväksyntää. Aiempien erien alla olevat jatkotehtäväluettelot ovat historiaa: jokainen
avoin kohta on sijoitettu korjauskokonaisuuteen, testaussuunnitelmaan tai omistajan päätöksiin.
/ Phase C was closed on 2026-11-26 by the user's decision; all eight closing conditions are met.
Closing does not mean the code base was fully reviewed. Next: phase K on the user's go-ahead and
the preparatory tasks E-1..E-4; phase D does not start without separate permission; the audit
branch is not pushed to a public repository without separate approval.

**Avoimet jatkotehtävät C-10:stä / open follow-ups from C-10** (eivät estäneet sulkemista / did not
block closing; `riippuvuudet.md` 19.7): ajokokeet Q96–Q98, Q102 ja Q104 vain erillisellä luvalla;
Q99 ja Q100 vaativat palvelimen tai laitteen dokumentaation; Q101 omistajalle (OM-C10-1); Q103
verkkotarkistus erikseen; uudet Q105 (H-161:n injektion saavutettavuus) ja Q106 (H-169:n
leviäminen ja palautus); haavoittuvuustietokantojen tarkastus erikseen; korjauskokonaisuudet
KK-C10-1–4 C-13:n syötteeksi; valitsinsyötteet (T-045, `MAKI`, `HIIHTO`, `EMITFILE`) C-12:lle. /
Runs only with permission; owner question Q101; new Q105, Q106; fix packages to C-13; switch inputs to
C-12.

**Avoimet jatkotehtävät C-9:stä / open follow-ups from C-9** (eivät estäneet sulkemista / did not
block closing; `virheenkasittely.md` 13.5). GUI-, C++Builder- tai laiteajoa vaativat / need a GUI,
C++Builder or device run:
- H-152: toisto GUI-käsittelijällä ja tulostettavilla tuloksilla / reproduce with the GUI handler
- Q91 ViestiWinin VCL-muistinhallinta ja H-149:n kaksoisvapautus (VCL-koeohjelma) / VCL memory manager
- Q92 bcc32-RTL:n `_wfopen_s`-jakotila (H-147); Q95 HkKisaWinin virrat ja vaiheen vaihto (H-150) / bcc32 share mode; HkKisaWin streams
- Q86 lukijan kuittaus (laite; C-10 osa D staattisesti) / reader ACK (device; statically in C-10 part D)
- H-153:n lukkiutuminen GUI-ajossa; H-149:n vahvistus toimitetulla konsoliohjelmalla vain erillisellä luvalla / H-153 deadlock in a GUI run; H-149 in a shipped console program only with permission
Muut / other:
- Q87 verkkolevyn kahva katkon jälkeen, Q89 konsolin näppäin työsäikeessä, Q90 muistipuskuri, Q93 `sendtcpfile` tiedonsiirtoyhteydellä, Q94 AJAT-rivit tuloksissa / other open questions
- matalat kokoomarivit H-156, H-157 yksitellen tarkastamatta / low roll-up rows not reviewed one by one
- korjauskokonaisuudet KK-C9-1 (`PRFILE`-elinkaari, T-042), KK-C9-2 (tallennusvirheiden välitys ja palautus, T-040, T-041; kokonaiskuva `virheenkasittely.md` 12.7), KK-C9-3 (resurssit) korjaussuunnittelun syötteeksi / fix packages as input to fix planning

**Avoimet jatkotehtävät C-11:stä / open follow-ups from C-11** (eivät estäneet
sulkemista / did not block closing; `testaus.md` 20.4):
- H-009: käsitesti GUI:ssa (pakotettu ajoitus) / manual GUI test (forced timing)
- H-014: käsitesti HkKisaWin + paikallinen MySQL / manual test with local MySQL
- kahden prosessin loopback-kerros (H-022, H-032, H-027); H-041:n lukupolku (Q37) ja 250;
  testit CI:hin (Q35, Q36, Q40) / two-process loopback layer; H-041 read path; CI
- palomuurisääntöjen `TP C11 HkMaali block in/out` poisto (käyttäjä, ylläpitäjä) /
  removal of the C-11 firewall rules (user, admin)

**Jatkotyöjono C-7:stä / follow-up queue from C-7** (eivät estäneet sulkemista / did not block
closing; `gui-liiketoimintalogiikka.md` luku 13). GUI-syötteiden tarkastus ei ole valmis
kokonaisuutena. / The GUI input review is not complete as a whole.

| # | Tehtävä / Task | Sisältö / Content | Tyyppi / Type |
|---|---|---|---|
| J7-1 | Arvonta-algoritmi / draw algorithm | `cbHk/HkArv2.cpp` (`arvo`, `arvovaihe`, `arvonumerot`; 1 274 riviä, taso 1): kaksoisnumerot ja -ajat, rajat, uudelleenarvonta, jo lähteneet; C-7:n kysymys 4 jäi tältä osin auki / C-7 question 4 open for the draw itself | staattinen / static |
| J7-2 | Muut kilpailutietoja kirjoittavat lomakkeet / other forms writing competition data | 41 lomaketiedostoa `cbHk/`, `ViestiWin/`, `cbTp/` (koneellinen haku: `tallenna`, `addtall`, `deltall`, `putem`, `tall_rivi`, `setMaali`, `tall_tulos`, `set_tulos`). Ensin: `cbHk/UnitSprinthiihto.cpp`, `cbHk/UnitAikaSiirto.cpp`, `cbTp/UnitAikakorjaus.cpp`, `cbHk/UnitNollaus.cpp`, `ViestiWin/UnitNollaus.cpp`, `cbTp/UnitFIS.cpp`, `cbHk/UnitIlm2.cpp`, `cbHk/LoppuKilpUnit.cpp`, `ViestiWin/UnitLuenta.cpp`, `cbHk/UnitPistelasku.cpp`, `cbHk/UnitKilpailijaOnline.cpp`, `cbHk/UnitEmitMuutokset.cpp`; tuonti (`UnitImport`) käsitelty osittain C-3:ssa / first the listed units; import partly covered in C-3 | staattinen / static |
| J7-3 | 152 jäsennysehdokasta / parse candidates | `todisteet/c5/vaihe2/dca.tgz` → `dca/atoi_index_candidates.csv` ilman C-7:n yhdeksää: lomakkeet 96 (`cbHk/` 55, `ViestiWin/` 30, `cbTp/` 11), ydin 56 (`Hk/` 32, `Juk/` 23, `Tp/` 1); voidaan yhdistää J7-2:een tiedostoittain / may be merged with J7-2 per file | staattinen / static |
| J7-4 | Ajonaikaiset vahvistukset / run-time confirmations | ensisijaisesti **H-109** (`kilparr[-1]`, kaksoispoisto ja sitä seuraavat lisäykset, `remjarjarr` irrotetulle tietueelle), **H-110** (työsäikeiden pysähtyminen "Lähtöajat"-painalluksen jälkeen), **H-116** (X → 2 tässä C++Builder-versiossa); lisäksi H-111, H-112, H-113, H-114, H-115, H-120, H-125 (luku 10) | ajo, erillinen lupa / run, separate permission |
| J7-5 | Riippumaton tarkastus / independent review | H-122, H-123, H-124, H-126, H-127 (yhden agentin tulokset) ennen niiden korjaamista / before fixing | staattinen / static |
| J7-6 | Avoimet kysymykset / open questions | Q75 (kuntosuunnistustilat), Q76 (viestin jonotallennus ja joukkuetietue), Q77 (haarojen ikä, versiohistoria), Q15:n jäljellä oleva osa (`tallenna`-polun poikkeukset itse ytimessä) | staattinen / static |
| J7-7 | Korjaussuunnittelu / fix planning | T-034: jokainen HK- tai viestilomakkeen korjaus tarkistetaan myös vastinhaarasta (luku 4); T-035, T-036, T-037 yhteisinä korjausmalleina / check every fix against the other fork | korjausvaihe / fix phase |

**Avoimet jatkotehtävät C-8:sta / open follow-ups from C-8** (eivät estäneet sulkemista / did
not block closing; `globaali-tila.md` luvut 9, 11, 12):
- H-129: itse kekoylivuodon toisto (noin 1 500 tietuetta, PageHeap tai ASan) ja vahvistus
  toimitetussa ohjelmassa; korjausjonossa P2 (alustava) / reproduce the overflow itself and
  confirm in a shipped program; fix queue P2 (provisional)
- H-128: vahvistus toimitetussa ohjelmassa (poisto keskeltä, sitten vienti numerojärjestyksessä) /
  confirm in a shipped program
- H-132 (`AJAT=`-nimi yksivaiheisessa kilpailussa) ja Q80 (uusintayritys) vaativat GUI-ajon /
  need a GUI run
- Q78–Q82 avoinna / open
- tarkastamattomat kokoomarivit H-142–H-144 (yhden agentin tuloksia) / unreviewed roll-up rows
- `Tp/`-, `tputilv2/`-, `cbTp/`- ja `com/`-hakemistojen 153 funktion staattista luokittelematta;
  kuusi C-6:n säiettä osan D ulkopuolella; lukematta `lue_parametrit` ja viestin funktion
  staattiset / 153 function statics unclassified, six threads not covered, parameter reader unread
- T-038 ja T-039 arkkitehtuurin kehityskohteina (alla) / as architectural development items

**Avoimet jatkotehtävät C-6:sta / open follow-ups from C-6** (eivät estäneet sulkemista /
did not block closing; `saikeet-ja-lukot.md` 13.10, 13.11). Siirretty myöhempään testaus- tai
korjausvaiheeseen; vaativat toimitetun ohjelman tai bcc32-käännöksen ajamista:
- bcc32-käännöksen (HkKisaWin, ViestiWin) ajonaikainen todennus; erityisesti H-091:n sykli
  GUI:ssa (koeohjelmat ajoivat vain MSVC-konsolikäännöstä) / run-time verification of the bcc32 build
- H-087 ja H-103 ViestiWinissä (GUI-koodia ei voi linkittää konsolikoeohjelmaan; ehdotus:
  debuggeri tai Application Verifier, lukon `OwningThread`/`RecursionCount`) / H-087, H-103 in ViestiWin
- Q67: `aloitus`in kutsumien funktioiden apusäikeet ennen epäonnistumiskohtaa / helper threads
- Q68 (`opencomfile(1)` yhdellä yhteydellä) ja Q73 (konsolin `CTRL_CLOSE`, `maaliajat`)
- muut ajoa vaativat: H-090:n esiintyvyys oikeilla laitteilla, H-093:n `comfl`-ehto oikealla
  kokoonpanolla, H-094:n vapautetun käytön kilpa, H-106:n ristisäikeinen `TEdit::SetText`,
  H-107:n `ToimintaTila==1` + lukija, `SendMessage` tuhottavaan ikkunaan (lomake-elinikä),
  Q65 vanhemmilla Windowseilla ja odottajien kanssa / other items needing runs
- Q65:n vaikutus arviointiin on kirjattu H-088:aan ja H-089:ään (`rekisteri/havainnot.md`):
  tällä Windowsilla ylimääräinen tai väärän säikeen `Leave` ei jumita vaan vapauttaa lukon,
  jolloin seuraus on suojaamaton kriittinen alue; vakavuudet ennallaan (keskitaso) / Q65's
  effect on H-088/H-089 is recorded; severities unchanged

**Avoimet jatkotehtävät C-5:stä / open follow-ups from C-5** (eivät estäneet sulkemista /
did not block closing; `kaantajaerot.md` 13.6–13.7):
- **Syötteiden jäsennyksen läpikäynti / integer-parse review:** 161 lukematonta
  kokonaislukujäsennysehdokasta (`_wtoi`, `_wtol`, `atoi`; indeksi, varaus tai silmukan raja)
  GUI-ohjelmiin käännettävissä yksiköissä: lomakkeet `cbHk/` 60, `ViestiWin/` 34, `cbTp/` 11;
  ydin `Hk/` 32, `Juk/` 23, `Tp/` 1 (ydin käännetään myös konsoliin). Lista:
  `todisteet/c5/vaihe2/dca.tgz` → `dca/atoi_index_candidates.csv`. Erillinen jatkotehtävä;
  ei vielä numeroitu erä, voidaan yhdistää erään 7 (GUI:n liiketoimintalogiikka) päätöksellä;
  C-7:n vaihe 1 ratkaisi kohdetiedostojen 9 ehdokasta (`gui-liiketoimintalogiikka.md` luku 7:
  7 turvallisia, 1 = H-085, 1 osittain H-126), muut 152 ovat C-7:n jatkotyöjonossa (J7-3). /
  161 unread candidates (105 in form units, 56 in core units); separate follow-up, may be
  merged with batch 7 by decision.
- **H-079: saavutettavuus ja vakavuuden uudelleenarviointi / reachability and severity
  re-assessment:** nykyinen vakavuus matala. Selvitettävä (staattisesti): (1) avaako jokin
  saavutettava polku tavallisessa käytössä UTF-16-tiedoston uudelleen (`UnitSakkoKierr`
  molemmissa GUI:issa, `HkConsole/HkCons3.cpp:1564`) ja voiko ohjelma itse tuottaa sellaisen
  tiedoston; (2) ovatko ylivuotavat merkit aina kiinteä `", ccs=UTF-16LE"`/`BE`-loppu, jolloin
  vaikutus rajautuu kaatumiseen [päätelty, ei todennettu]; (3) voiko tiedosto tulla verkosta
  (esim. H-071:n tiedostokirjoitus) – ei analysoitu. Ajonaikainen vahvistus toimitetulla
  ohjelmalla vain erillisellä luvalla. / Static questions: reachable re-open in normal use,
  whether the overflowing bytes are always the fixed suffix (crash only), whether the file
  can arrive over the network (not analysed); run-time check only with permission.
- **Q60** omistajan päätettäväksi (`KUNTOSUUNN`-ero, H-081); **Q63** ja **Q64** avoimia
  teknisiä kysymyksiä (pieni koeohjelma tai lukeminen riittää). / Q60 owner decision; Q63, Q64
  open technical questions.
- Ajonaikaiset vahvistukset toimitetuilla ohjelmilla (FPU-sana / `VersioInt`, H-078, H-079)
  vain erillisellä luvalla. / Run-time confirmations only with permission.

Havainnot (H/T) ja niiden todennustila: `rekisteri/havainnot.md`.
Findings (H/T) and their verification status: `rekisteri/havainnot.md`.

## Priorisoitu korjausjono / Prioritised fix queue

**C-13 (2026-11-26):** ajantasainen prioriteetti ja korjauskokonaisuus jokaiselle havainnolle on
`rekisteri/riskiarvio.csv`:ssä ja toteutusjärjestys `korjaussuunnitelma.md`:n luvussa 4. Tämän
luvun taulukko ja kappaleet ovat erien C-4–C-12 sulkemisten tilanne ja säilyvät historiana; niistä
poikkeavat C-13:n muutokset: H-033 uusi P1; H-111, H-116, H-151, H-164 P3 → P2; H-064, H-152, H-153
P4 → P3; H-158 P5 → P3; H-069 P5 → P4; 61 aiemmin priorisoimatonta havaintoa sai prioriteetin
(`auditoinnin-yhteenveto.md` luku 9). / The current priority and fix package of every finding is in
`rekisteri/riskiarvio.csv`; this section is the history of the batch closings.

Lähde: `muistiturvallisuus.md` luku 15 (C-4); tämä tiivistelmä ei muuta sen prioriteetteja.
Korjaukset tehdään omissa haaroissaan erillisellä päätöksellä, eikä yhtäkään ole aloitettu.
Väliaikaiset suojaussuositukset: `muistiturvallisuus.md` luku 15. / Source: memory-safety
section 15; this summary does not change its priorities. Fixes go in their own branches on a
separate decision; none started. Rivi 4b (H-129) lisätty C-8:n sulkemisessa / row 4b added at the C-8 closing.

| # | Havainto / Finding | Syöte / Input | Tila / Status |
|--:|---|---|---|
| 1 | **H-071** FILESEND: pituus ja tiedostopolku / length and file path | verkko, oletusasetukset / network, default settings | **kiireellinen / urgent** (P1) |
| 2 | **H-072** `EXTRA`-paketin indeksit / indices | verkko, oletusasetukset / network, default settings | **kiireellinen / urgent** (P1) |
| 3 | **H-055** emiTag `P`-kentän indeksi / index | lukija, GUI `TCPS`, HTTP-pollaus (asetus) / reader, TCPS, polling (setting) | **kiireellinen / urgent** (P2) |
| 4 | **H-070** FEIG-lukijan kasaylivuoto / heap overflow | laite / device | **kiireellinen / urgent** (P2) |
| 4b | **H-129** vaiheen vaihto: `idindex->nrec` ei nollaudu → kekoylivuoto / stage change heap overflow (C-8) | HK:n vaiheen vaihto noin 1 500 kilpailijasta / HK stage change from about 1,500 competitors | **kiireellinen / urgent** (P2, alustava / provisional); yksirivinen korjaus, testi valmiina / one-line fix, probe ready (`todisteet/c8/koeohjelmat/`) |
| 5 | H-032, H-061 | verkko (UDP; sarja, binääri-TCP) / network | P1; H-061:n `pkglen` todennettava ensin (Q43) / verify first |
| 6 | H-056, H-060, H-073 | laite tai verkko asetuksella / device or network with a setting | P2 |
| 7 | P3–P5 | tavallinen data, paikallinen tiedosto, operaattori / ordinary data, local file, operator | `muistiturvallisuus.md` luku 15 |

C-10:n havainnot (H-159–H-170), lopullinen [päätelty] (`riippuvuudet.md` 19.3): **P2** H-159
(RFID-lukijan uudelleenyhteys lopettaa luennan kilpailun aikana), H-160 (muutos P3 → P2: SQL-peilin
ollessa käytössä palvelinkatko, heittomerkki tai H-161:n lause lopettaa koko ohjelman; vakavuus
ennallaan); **P3** H-162 (kärkeen), H-169, H-164, H-166, H-163, H-161, H-165; **P4** H-167, H-168;
**P5** H-170. Korjauskokonaisuudet (19.4): **KK-C10-1** SQL-integraatio (H-160–H-163, H-169),
**KK-C10-2** laiteyhteyksien hallinta (H-159, H-164, T-048, H-166, H-167), **KK-C10-3** jakelu ja
ulkoiset riippuvuudet (H-165, T-005, T-044), **KK-C10-4** ulkoisten yhteyksien turvallisuus (T-043,
H-161:n injektioriski, ei vahvistettu); lisenssit omistajan päätökseen **OM-C10-1** (19.5). / C-10
final: P2 H-159, H-160 (changed from P3); P3 H-162 (first), H-169, H-164, H-166, H-163, H-161, H-165;
P4 H-167, H-168; P5 H-170; fix packages KK-C10-1..4; licences to owner decision OM-C10-1.

C-12:n havainnot (H-171–H-177), lopullinen [päätelty] (`ominaisuusvalitsimet.md` 13.3): **P2** H-171
(toistuva 158 tavun staattisen muistin ylikirjoitus jokaisessa ViestiWinin kilpailun avauksessa; ei P1,
koska ei laukea verkosta; samalla perusteella kuin H-149); **P3** H-134 (H-129:n kanssa, nostettu
keskitasoon), H-172, H-173, H-174; **P4** H-175; **P5** H-176, H-177 (omistajan päätös Q111).
Korjauskokonaisuudet **KK-C12-1** julkaisutiedostot (H-172, H-149, H-150, H-176:n HTML-pohja),
**KK-C12-2** asetusten hallinta (H-171, H-173, H-036, H-176, T-051), **KK-C12-3** valitsinhygienia
(T-050, T-052, H-177; ei poistopäätöksiä). / C-12 final: P2 H-171; P3 H-134 (with H-129), H-172, H-173,
H-174; P4 H-175; P5 H-176, H-177; fix packages KK-C12-1..3.

C-9:n havainnot (H-145–H-158), lopullinen [päätelty] (`virheenkasittely.md` 13.3): **P2** H-149
(ASanilla todennettu, tavallisessa konfiguraatiossa joka kierroksella; ei P1, koska ei laukea
verkosta; P2:ssa H-129:n edelle), H-150 (muutos P3 → P2: tiedostovirrat loppuvat konfiguraatiossa
XML-vienti + vaihe ≥ 2 + `AUTOFILE.LST` saman päivän aikana, MSVC noin 1,4–8,5 h ja C++Builder alle
7,5–45 min 10–60 s välillä; vakavuus keskitaso ennallaan); **P3** H-145 (kärkeen: kortti kuitataan
mutta puuttuu tuloksista, palautus käsin), H-146, H-151, H-147; **P4** H-148, H-152, H-153, H-154,
H-155; **P5** H-156, H-157, H-158. Korjauskokonaisuudet KK-C9-1–3 (13.4). / C-9 final: P2 H-149,
H-150 (changed from P3 on stream exhaustion); P3 H-145 (first), H-146, H-151, H-147; P4 H-148,
H-152–H-155; P5 H-156–H-158; fix packages KK-C9-1..3.

C-6:n havainnot (H-087–H-108) ovat lukkiutumisia, vapauttamattomia lukkoja, elinkaaren
kilpailutilanteita ja VCL-käyttöä työsäikeistä; verkosta laukeavia syötevirheitä ei ole.
Päivitetty sijoitus vaiheen 2 jälkeen [päätelty] (`saikeet-ja-lukot.md` 13.9): **P2** H-087,
H-091, H-090, H-093 (jäätyminen tai kaatuminen kilpailun aikana; H-090 ja H-093 todennettu
koeohjelmalla), H-107 (kaatuminen ensimmäisestä luennasta asetuksella); **P3** H-094, H-103,
H-104, H-105, H-106, H-088, H-089, H-092, H-097; **P4** H-095, H-096, H-098, H-100, H-101,
H-102, H-108; H-099 kumottu ja poistettu jonosta. / Updated after step 2 [inferred]: P2 H-087,
H-091, H-090, H-093, H-107; P3 H-094, H-103–H-106, H-088, H-089, H-092, H-097; P4 the rest;
H-099 refuted and removed.

C-7:n havainnot (H-109–H-127) laukeavat käyttäjän toiminnosta lomakkeessa, eivät verkosta.
Alustava sijoitus [päätelty] (`gui-liiketoimintalogiikka.md` luku 11): **P2** H-109 (tietojen
korruptio), H-110 (työsäikeet pysähtyvät); **P3** H-111–H-117; **P4** H-118–H-125; **P5** H-126,
H-127. / C-7 findings are triggered by operator actions in forms, not by the network;
provisional placement P2 H-109, H-110; P3 H-111–H-117; P4 H-118–H-125; P5 H-126, H-127.

C-5:n havainnoille (H-078–H-086) ei ole osoitettu verkkosaavutettavuutta; ne vaativat
paikallisen tiedoston tai operaattorin syötteen. Alustava sijoitus [päätelty]: P4 H-078,
H-079 (uudelleenarviointi kesken, ks. jatkotehtävät), H-082, H-085; P5 H-080, H-081, H-083,
H-084, H-086. H-042:n laajennus kuuluu H-042:n mukana P3:een. / C-5 findings have no shown
network reachability; provisional placement [inferred] P4 H-078, H-079 (re-assessment
pending), H-082, H-085; P5 the rest; H-042 stays P3.

## Moduulien dokumentointi / Module documentation

| Moduuli / Module | Arkkitehtuuri / Architecture | Koodikommentit / Code comments |
|---|---|---|
| `Hk/` | yleiskuva, päävirta / overview, main flow | – |
| `Juk/` | yleiskuva (päätelty HK:n pohjalta) / overview (inferred from HK) | – |
| `Tp/` | yleiskuva, säikeet, säie- ja lukkokartta (`saikeet-ja-lukot.md`) / overview, threads, thread and lock map | – |
| `com/` | kuljetus (`tiedonsiirto.md`) / transport | – |
| `cbHk/`, `ViestiWin/`, `cbTp/` | logiikkaa sisältävät tunnistettu; kahdeksan liiketoimintalomaketta kuvattu (`gui-liiketoimintalogiikka.md`) / forms with logic identified; eight business-logic forms described | – |
| `tputilv2/`, `include/`, `dbboxm/` | rajapinta mainittu / interface mentioned | – |
| `HkConsole/`, `JukConsole/` | käynnistys / startup | – |

## Avoimet kysymykset / Open questions

| # | Kysymys / Question | Missä / Where |
|---|---|---|
| ~~Q1~~ | **Ratkaistu / resolved 2026-11-24:** työsäie `tarkcom`; samanaikaisuus mahdollinen lukitsemattomien kutsujen kautta / worker thread `tarkcom`; concurrency possible via unlocked callers (`tallenna-samanaikaisuus.md`, H-007, H-010) | – |
| Q65–Q74 | Erän C-6 kysymykset (ei-omistajan `Leave`, lokisoketin virheen pysyvyys, `aloitus` kahdesti, `opencomfile(1)` yhdellä yhteydellä, emit-muistin lukijat vaihdon aikana, `TCPrcvTh` vapautuksen jälkeen, viestin `tark_aika` lopetuksessa, `LAJUNEN`, konsolin `CTRL_CLOSE`, syvemmät lukitusreunat) / batch C-6 questions. **Vaihe 2:** Q65, Q66, Q69, Q70, Q71, Q72, Q74 ratkaistu, Q67 osittain; Q68 ja Q73 avoinna / step 2: resolved except Q67 (partly), Q68, Q73 | `saikeet-ja-lukot.md`, luvut / sections 10, 13.5 |
| Q12 | Kumpi `add_sakko` linkitetään HkKisaWiniin? / Which `add_sakko` is linked? | `cbHk/UnitASakot.cpp:269`, `cbHk/UnitASuunn.cpp:289` |
| ~~Q13~~ | **Ratkaistu / resolved (C-2):** kyllä, kaikki perustiedot paitsi koon 0 kentät; ei vaiheita / yes, all base fields except size-0 fields; no stages (`tiedonsiirto.md`) | – |
| Q14 | Kutsuuko `tulostasarja` lukon aikana `SendMessage`-polkua? **Osittain ratkaistu (C-6):** GUI-polulta ei löytynyt; konsolin kehotteet vapauttavat lukon / Does `tulostasarja` reach a `SendMessage` while holding the lock? **Partly resolved (C-6):** none found on the GUI path | `Hk/HkTulostus.cpp:5850`–`:6419` |
| Q15 | Voiko VCL-poikkeus `tallenna`-polulla jättää lukon päälle? **Osittain ratkaistu (C-7):** kyllä lomakkeiden käsittelijöissä (H-115, H-125, T-036); `tallenna`-polkua itseään ei tutkittu / Can a VCL exception leave the lock held? **Partly resolved (C-7):** yes in form handlers | `tallenna-samanaikaisuus.md`, `gui-liiketoimintalogiikka.md` luku 10 |
| Q75 | `kuntosuunnmoodi`-arvojen (1, 2, 3, 98, 99), `esiluenta`n ja `kuntoLahto`n merkitys / semantics of the fitness modes | `cbHk/UnitEmit.cpp:199-320` |
| Q76 | Päivittääkö viestin ydin joukkuetietueen, kun ajanoton paneelista tallennetaan vain jonorivi? / Does the relay core update the team record after a queue-only save? | `ViestiWin/UnitAjanotto.cpp:657` |
| Q77 | Kumpi HK/viesti-haara on uudempi yksipuolisissa korjauksissa (versiohistoria, T-034)? / Which fork is newer per one-sided fix? | `gui-liiketoimintalogiikka.md` luku 4 |
| Q78 | Mikä `n_lisaa` asennuksissa on, ja vaihdetaanko vaihetta käynnissä olevassa prosessissa? / Real `n_lisaa` and stage-change practice (H-129) | `globaali-tila.md` luku 9 |
| Q79 | `SeurPos` case 0 välittää tietuepaikan numeroa odottavalle `getpos`ille? / record position passed as bib (H-128) | `cbHk/UnitArvonta.cpp:221-223`, `:421` |
| Q80 | Kaatuuko saman kilpailun uusintayritys `KILP.DAT`:n jakorikkomukseen? / Sharing violation on retry (H-133, Q67) | `Hk/HkInit.cpp:3486-3491` |
| Q81 | Useita lukijoita yhdellä HK-instanssilla; sammutetun TCP-lukijan Winsock-virhe? / Multiple readers in practice (H-136, H-138) | `globaali-tila.md` luku 9 |
| Q82 | Mihin `tmaali` kirjoitetaan? / Where is `tmaali` written? (H-143) | `Hk/HkTulostus.cpp:6562` |
| ~~Q83~~ | **Ratkaistu / resolved (C-9 vaihe 2):** bcc32-RTL:n `fputws` ja `fputs` palauttavat virheessä −1, eivät `WEOF`:ia (H-147) / both return -1 | `todisteet/c9/vaihe2/koeohjelmat/tulokset/rtl_bcc32.txt` |
| ~~Q84~~ | **Ratkaistu / resolved (C-9 vaihe 2):** bcc32-RTL:ssä 50 virtaa (47 + 3); GUI:n vakiotila Q95:ssä (H-150) / 50 streams | `todisteet/c9/vaihe2/koeohjelmat/tulokset/rtl_bcc32.txt` |
| ~~Q85~~ | **Ratkaistu MSVC:n osalta / resolved for MSVC (C-9 vaihe 2):** STATUS_HEAP_CORRUPTION (`0xC0000374`) ensimmäisellä kierroksella; bcc32-RTL:n `free` hiljainen; VCL Q91:ssä (H-149) / fail-fast in MSVC | `virheenkasittely.md` 12.1 |
| Q86 | Pitääkö lukija kuittausta lopullisena? **Kavennettu (C-10):** koodin puoli selvitetty (`+
` kun `tall_emit` palauttaa 0, myös hylätylle kaksoiskappaleelle, H-168); laitteen käytös Q100:ssa / code side answered, device side in Q100. **Kavennettu (C-9 vaihe 2):** kuittaus vain Emit-kellolla (`Tp/TpLaitteet.cpp:911-917`); laitteen tulkinta avoin / reader ACK semantics, Emit clock only (H-145) | `Tp/TpLaitteet.cpp:911-916` |
| Q87 | Verkkolevyn kahva katkon jälkeen? / SMB handle after a drop (H-146, H-015) | `virheenkasittely.md` luku 9 |
| ~~Q88~~ | **Ratkaistu / resolved (C-9 vaihe 2):** viestin GUI ajaa `autofile`:n `LukumaaratTh`-työsäikeessä (H-147, H-149) / worker thread | `ViestiWin/UnitMain.cpp:535-541` |
| Q89 | Konsolin ilmoituksen näppäin työsäikeessä? / console key routing to worker-thread prompts (H-157) | `tputilv2/Writeerr.cpp:66-94` |
| Q90 | Ajetaanko kilpailuja ilman levypuskuria? / memory-only outbuf in practice | `virheenkasittely.md` luku 6 |
| Q91 | Miten ViestiWinin VCL-muistinhallinta reagoi H-149:n kaksoisvapautukseen? **Osittain (C-10, staattisesti):** C ja VCL jakavat `borlndmm.dll`:n; toinen vapautus hiljainen, jos lohkoa ei ole varattu uudelleen, muuten hiljainen korruptio; ajokoe avoin (`riippuvuudet.md` luku 14) / partly: shared FastMM, run still open / VCL memory manager and the H-149 double free | `virheenkasittely.md` 12.1 |
| ~~Q92~~ | **Ratkaistu staattisesti / resolved statically (C-10):** kyllä, jakotila 0 kirjoitukselle, `FILE_SHARE_READ` lukemiselle (RTL `__open.c`), kuten MSVC / yes, exclusive for write, as in MSVC (H-147) | `tputilv2/TextFileClass.cpp:305` |
| Q93 | Rikkooko `sendtcpfile`n STX/tiedosto/ETX-virta vastapään jäsennyksen tavallisella tiedonsiirtoyhteydellä? / file stream on a peer data link (H-149, H-151) | `Juk/VTulostus.cpp:7116-7128` |
| Q94 | Käytetäänkö AJAT-riviä tulosten uudelleenrakentamiseen? / are AJAT rows used to rebuild results (H-146) | `virheenkasittely.md` 12.3 |
| Q95 | HkKisaWinin virtamäärä vakiotilassa ja NULL-`femitfile` vaiheen vaihdossa (H-150) / HkKisaWin streams at steady state | `Hk/HkEmit.cpp:3057-3058` |
| Q96 | Päättyykö Sirit/Zebra-lukijasäie uudelleenyhteyden jälkeen? (H-159) / reader thread after reconnect | `riippuvuudet.md` luku 17 |
| Q97 | Lopettaako SQL-virhe kirjoitussäikeessä ohjelman; jääkö `tall_CriticalSection` päälle viennissä? (H-160, H-162) / SQL error abort, export lock | `riippuvuudet.md` luku 17 |
| Q98 | GUI-ohjelmien käynnistys ja SQL julkaisu-ZIP:stä puhtaalla koneella (H-163, H-165) / clean-machine start | `riippuvuudet.md` luku 17 |
| Q99 | `get_stream.php`:n vastausmuoto; palvelimen `195.159.103.189:1379` omistaja (H-166) / server format and owner | `riippuvuudet.md` luku 17 |
| Q100 | Emit-kellon käytös ilman kuittausta; kehyksen tarkistussumma (Q86) / Emit clock behaviour without ACK | `riippuvuudet.md` luku 13 |
| Q101 | Blaise Computing- ja Microsoft-esimerkkikoodin ehdot (T-047) – omistajan päätös / owner decision (OM-C10-1) | `riippuvuudet.md` 19.5 |
| Q102 | Voiko `IndySystem`in poistaa linkityksestä? (T-005, T-044) / can IndySystem be removed | `riippuvuudet.md` luku 14 |
| Q103 | `windows-latest`-kuvan `v143` ja `upload/download-artifact@v7` (T-044) / CI image and action versions | `riippuvuudet.md` luku 15 |
| Q104 | Viestin `vert0` SQL-kierroksen jälkeen (H-161) / relay `vert0` after an SQL round trip | `riippuvuudet.md` luku 17 |
| Q105 | H-161: voiko SQL-lauseisiin liitettävää merkkijonoa ohjata ulkopuolelta (operaattori, tuontitiedosto, työasemien sanomat), mitkä ovat MySQL-käyttäjän oikeudet, ja salliiko ajuri useita lauseita? Injektio ei ole vahvistettu / can the spliced strings be controlled from outside; privileges; multi-statement support; injection not confirmed | `riippuvuudet.md` 19.3.1 |
| Q106 | H-169: leviävätkö `lue_SQL`:n korvaukset `tallenna`n kautta muille työasemille, ja miten tiedot palautetaan? / does the SQL import overwrite propagate to other workstations; recovery | `riippuvuudet.md` 19.3.2 |
| Q107 | **Siirretty ajonaikaiseen todennukseen (C-12:n sulkeminen) / moved to run-time verification:** H-171: mitä bcc32 sijoittaa ViestiWinissä `cfgfilename`n jälkeen, ja nollautuvatko `autostr`/`baudst` käytännössä? (`.map` tai vahtipiste) / what follows `cfgfilename` in ViestiWin | `ominaisuusvalitsimet.md` 11 |
| Q108 | **Siirretty ajonaikaiseen todennukseen (C-12:n sulkeminen) / moved to run-time verification:** H-172: mitä viestin konsolin X)ML-tiedosto sisältää ajossa, myös aiemman autofile-XML-ajon jälkeen? / content of the console X)ML file | `ominaisuusvalitsimet.md` 11 |
| Q109 | Tarkistaako `GETREC` rajat tiedoston lopun ylittävälle tietuenumerolle (`SQLjonoVarm.dat`, H-043)? / does GETREC bound-check | `ominaisuusvalitsimet.md` 11 |
| Q110 | **Siirretty ajonaikaiseen todennukseen (C-12:n sulkeminen) / moved to run-time verification:** H-134: mitä vaiheen vaihdon else-haara tekee vapautetulla ratamuistilla jäsennysvirheen jälkeen (ajo rikotulla `RADAT<n>.XML`:llä)? / stage change after a parse error | `ominaisuusvalitsimet.md` 11 |
| Q111 | **Omistajan päätös, ei ratkaista oletuksilla / owner decision, not resolved by assumption.** Omistajalle: onko `SEURATUL`/`ORAVA` (H-177) poistettu tarkoituksella, kuuluuko RFID (`LAJUNEN`) ViestiLuentaan (T-052), ja saako kuolleet valitsimet (T-050) poistaa? / owner questions | `ominaisuusvalitsimet.md` 11 |
| Q112 | **Avoin (C-12:n sulkeminen): käyttöohjeet, esimerkit ja Git-historia eivät kerro yksikköä (`ominaisuusvalitsimet.md` 13.4) / open: no unambiguous unit found.** `AIKAKORJAUSn`: HK kertoo `SEK / 100`, viesti `SEK`; kumpi yksikkö on tarkoitettu? / which unit is intended | `ominaisuusvalitsimet.md` 6.2 |
| ~~Q16~~ | **Ratkaistu / resolved (C-2):** viestin vastaanotto lukitsee ennen lukua, ei H-007:ää; omat viat H-024–H-026 / relay locks before reading, no H-007; own defects H-024–H-026 | – |
| ~~Q17~~, ~~Q18~~ | **Ratkaistu / resolved (C-3):** bcc32 noudattaa `-K`:ta kesken yksikön, tarkistussummat täsmäävät; koot mitattu kaikista käännöksistä / bcc32 honours `-K` mid-unit; sizes measured for all builds | `tiedostomuodot.md` |
| Q19–Q25 | Erän C-2 kysymykset (AIKAT-kaksoiskappaleet, TCP-virta, rengas, `tark_ylahto`, `Kysy`, kirjastot, MONITORI-portti) / batch C-2 questions **C-13:** Q24 ratkaistu (kirjastot tuotteittain: `riippuvuudet.md` luku 9; kaikki ohjelmat kääntyvät); Q25 on kirjattu T-014:ksi ja kuuluu omistajan päätökseen OP-11 / Q24 resolved; Q25 is owner decision OP-11. | `tiedonsiirto.md`, "Avoimet kysymykset" |
| Q26–Q34 | Erän C-3 kysymykset (Q30 ratkaistu C-6:ssa / resolved in C-6: `saikeet-ja-lukot.md` luku 5; HK:n XML-rajat, NTFS-katkos, `initindex`, viestin vienti, vaiheen vaihto, viestin 250-käännös, `t_time_l`, `mbtowc`, konstruktorijärjestys) / batch C-3 questions **C-13:** Q26 ratkaistu (ei rajata: `testaus.md` 18.1; H-041:n toinen reitti) / Q26 resolved: not clamped. | `tiedostomuodot.md`, "Avoimet kysymykset" |
| Q35–Q42 | Erän C-11 kysymykset (v143 ja ASan CI:ssä, `LockCount`, H-041 käynnistyksessä, H-001:n sisääntulo, `/we4789` ja LTCG, interaktiivinen CI-istunto, viestin `addtall`-säikeet, HK:n luokkapituudet) / batch C-11 questions | `testaus.md`, "Avoimet kysymykset" |
| Q43–Q56 | Erän C-4 kysymykset (`pkglen`, ARES-kehys, bcc32:n `atoi` ja ASLR, `mrivi`, `RADAT.LST`, GUI:n muotoilumerkkijonot, emiTag-TCP-palvelin konsolissa, FEIG viestissä, `ossijat`-valikot, `EMVALOKI`, `lahriviCPower`, XML-tuonnin paikat, emiTag-kortin vaikutus) / batch C-4 questions **C-13:** Q53 ratkaistu (`EMVALOKI` vain jäsentimissä, ei asetuseditoreissa: `rekisteri/asetusavaimet.csv`) / Q53 resolved. | `muistiturvallisuus.md`, "Avoimet kysymykset" |
| ~~Q2~~ | **Ratkaistu / resolved 2026-11-26 (C-12):** jokaisen testatun makron luokka ja ominaisuusvalitsimien merkitys `rekisteri/valitsimet.csv`:ssä ja `ominaisuusvalitsimet.md` luvut 3–4; konsolin ja GUI:n erot luku 5 (T-008) / every tested macro classified, meanings and console/GUI differences documented | `ominaisuusvalitsimet.md` |
| ~~Q3~~ | **Ratkaistu / resolved (C-2, C-3; merkitty C-13:ssa):** kehys, kuittaus ja yhteensopivuus kuvattu (C-2); tarkat koot mitattu C-3:ssa (Q18) / frame, ack and compatibility documented; exact sizes measured in C-3 | `tiedonsiirto.md` |
| ~~Q4~~ | **Ratkaistu / resolved (C-3):** muoto kuvattu tavutasolla; vanhaa muotoa ei tunnisteta automaattisesti, vain GUI-tuonti käyttäjän parametrein / layout documented; no automatic old-format detection, only GUI import with user parameters | `tiedostomuodot.md` |
| ~~Q5~~ | **Ratkaistu / resolved (C-10):** taulut `cbHk/kilpsql.txt` ja `ViestiWin/vkilpsql.txt`, valinnainen peili vain GUI:ssa, `KILP.DAT` ensisijainen; käyttöönotto `SQL*`-avaimilla tai valikosta / tables in the two schema files, optional GUI-only mirror, KILP.DAT is master | `riippuvuudet.md` luku 11 |
| Q6 | Ovatko kirjastoon käännetyt `main`-funktiot (`tputilv2/ctrlhndl.cpp` ym.) esikääntäjällä pois? / Are the `main` functions in the library excluded by the preprocessor? | `inventaario.md` |
| ~~Q7~~ | **Ratkaistu / resolved (C-5):** kyllä, kapeat ja leveät merkkijonot sekä merkkivakiot oikein myös sisällytetyissä otsikoissa; BOMiton UTF-8 on väärin molemmilla kääntäjillä / yes; UTF-8 without BOM is wrong on both compilers | `kaantajaerot.md` luku 5 |
| Q57–Q64 | Erän C-5 kysymykset; Q57–Q59, Q61 ja Q62 ratkaistu vaiheessa 2; avoinna Q60 (`KUNTOSUUNN`-eron tarkoitus, **omistajan päätös**), Q63 (bcc32:n negatiivinen `new`) ja Q64 (`UnitSakkoKierr`in kumoamispolun jäsennys) **avoimina teknisinä kysymyksinä** C-5:n sulkemisen jälkeen / batch C-5 questions; Q60 owner decision, Q63 and Q64 open technical questions after closing C-5 | `kaantajaerot.md`, "Avoimet kysymykset" |
| ~~Q8~~ | **Ratkaistu / resolved (C-5 vaihe 2):** bcc32-objektien OMF-vertailu `tyokalut/vertaa_omf.py` / OMF comparison of bcc32 objects | `kaantajaerot.md` luku 13.1, `merkistot.md` |
| Q9 | Sallitaanko C++11 tuotantokoodissa? / Is C++11 allowed in production code? | `riippuvuudet.md` |
| Q10 | Ovatko orvot tiedostot käytössä jossain (muut haarat, ulkoiset käännökset)? / Are orphan files used anywhere? | `inventaario.md` |
| Q11 | Mihin `Tp/TpComY32.cpp` (vanha kopio) ja `tputilv2/*-2.cpp`, `*-v.cpp` -versiot liittyvät? / What are the old copies for? | `inventaario.md` |

## Epäillyt viat / Suspected defects

Kirjattu issue-luonnoksiksi (paikallinen `issue-drafts/`, ei repossa).
Recorded as issue drafts (local `issue-drafts/`, not in the repo).

| # | Vika / Defect | Vakavuus / Severity | Tila / State |
|---|---|---|---|
| 01 | `memset(stsk, 0, sizeof(st))` ylivuoto / overrun, `Juk/VTulostus.cpp:2228`, `:3981` | muistin korruptio / memory corruption | luonnos / draft |
| 02 | `sscanf("%s")` `int`-kenttään / into `int`, `Juk/vSarjat.cpp:377`, `:400` | muistin korruptio / memory corruption | luonnos / draft |
| 03 | `printf`/`swprintf`-muotoiluvirheet, 16 kohtaa / format mismatches, 16 places | kaatuminen (2) tai väärä teksti / crash (2) or wrong text | luonnos / draft |
| 04 | U+FFFD käyttäjälle näkyvissä teksteissä / in UI strings (`Hk/HkAjat.cpp`, `Juk/vajat.cpp`); UTF-8-teksti konsolissa / UTF-8 text in console (`Tp/TpLaitteet.cpp:2581`) | väärä teksti / wrong text | luonnos / draft |
| 05 | `Tests/run.sh`: puuttuva `\` ja `-I Hk` (merge `6107c8f`); testejä ei ajeta CI:ssä / missing `\` and `-I Hk`; tests not run in CI. `TpTest.sln` itse toimii: 46/46 testiä läpi 2026-11-24 / `TpTest.sln` itself works: 46/46 pass | testit eivät aja automaattisesti / tests not run automatically | luonnos / draft |

## Vaiheen K ehdokkaat ja tarkistuskohteet / Phase K candidates and items to check

Vikakorttien kirjoituksessa ja niiden riippumattomassa tarkastuksessa (2026-11-27,
`doc/tietopohja/laatu/tarkastukset/k2-pilotti-opus.md`) huomattuja asioita. Vaihe K ei ole
auditointi: näitä ei ole tarkastettu, niille ei anneta H-tunnusta eikä riskiarviota muuteta tässä
vaiheessa. Käsittely D-vaiheen riskiseulonnassa E-1 (`korjaussuunnitelma.md` 9.1) tai korjauksen
yhteydessä. Kohdat säilyvät tunnuksineen ennallaan, kunnes niille tehdään erillinen tarkastus;
E-1:tä ei ole aloitettu. / Noticed while writing and reviewing the pilot cards; not reviewed, no ids,
no risk changes in phase K; to be handled in screening E-1 (not started) or with the fix.

| # | Kohde / Item | Löytötilanne / How found | Lähde / Source | Varmuus / Certainty | Ehdotettu käsittely / Proposed handling |
|---|---|---|---|---|---|
| KE-1 | H-033: `unpack`-haarassa väliajat kirjoitetaan alueen ulkopuolisen olion `va`-osoittimen kautta, joten kirjoitus voi osua mielivaltaiseen osoitteeseen (`Hk/HkDat.cpp:362-368`, `:189-196`) / split times written through the out-of-range object's pointer | Opus-tarkastaja luki H-033:n kortin teknisiä väitteitä koodista (K-2-pilotin tarkastus 2026-11-27) | Opus-tarkastaja | päätelty koodista; ei ajettu, ei riippumattomasti tarkastettu | H-033:n laajennus tarkastuksen jälkeen; KK-C13-1:n testiin |
| KE-2 | H-129: riskiarvion `tietoturva` = ei ja `saavutettavuus` = asetus; toisen työaseman vaiheen vaihtopyyntö käynnistää vaihdon ilman vahvistusta ja toistettuna kasvattaa hakemiston laskuria (`Hk/HkCom32.cpp:843-848`, `Tp/TpComY.cpp:636-638`, `cbHk/WinHk.cpp:2149-2165`) [päätelty] | kortin H-129 kirjoittaja huomasi verkkopyynnön, Opus-tarkastaja luki kutsuketjun (pilotti 2026-11-27) | Opus-tarkastaja, kirjoittaja | päätelty koodista; vaikutus muihin työasemiin ja toistuvan pyynnön seuraus ajamatta | riskiarvion uudelleenarvio E-1:ssä (mahdollisesti verkko, verkko_oletus) |
| KE-3 | H-145: riskiarvion saavutettavuus `normaalikaytto`, vaikka vika vaatii emit-tiedoston kirjoitusvirheen; asteikossa ei ole arvoa ympäristövirheelle | kortin H-145 kirjoittaja vertasi riskiarviota raportin laukaisuehtoon (pilotti 2026-11-27) | kirjoittaja, Opus-tarkastaja | dokumenttien välinen ristiriita; ei koodivika | saavutettavuusasteikon täydennys tai arvon korjaus |
| KE-4 | H-041: ylivuotava osa on viimeisen vaiheen väliaikatietoa (`Hk/HkDat.cpp:424-430`); voiko siihen päätyä verkosta tai laitteelta tullutta tietoa [päätelty] | Opus-tarkastaja arvioi H-041:n tietoturvariviä (pilotti 2026-11-27) | Opus-tarkastaja | päätelty; ylivuotavan osan alkuperää ei jäljitetty | riskiarvion `tietoturva` tarkistus E-1:ssä |
| KE-5 | H-109: HK:n `GETREC` antaa virheilmoituksen tietuenumerolla -1, mutta `deltall` jatkaa nollatulla tietueella | kortin H-109 kirjoittaja luki poistopolun koodista (pilotti 2026-11-27) | kirjoittaja | luettu koodista; ei tarkastettu; ei tiedossa, onko H-109 tai T-035 jo kattaa | tarkistus H-109:n korjauksen yhteydessä (KK) |
| KE-6 | H-093: lokisäikeen pino 40 kt, muiden 1 Mt; kirjoittaako lokisäie itse lokia | kortin H-093 kirjoittaja luki säikeiden pinokoot raportista (pilotti 2026-11-27) | kirjoittaja | avoin kysymys; ei selvitetty | H-093:n korjauksen yhteydessä |
| KE-7 | `initindex` ei nollaa `idindex->nrec`iä (mainittu H-133:n rivillä ja `todisteet/c8/osaE-tarkastus.md`:ssä) | kortin H-129 kirjoittaja löysi maininnan C-8:n tarkastusraportista (pilotti 2026-11-27) | kirjoittaja | päätelty (C-8:n tarkastus); mainittu H-133:n rivillä | varmistetaan, että KK-C13-3:n korjaus kattaa |
| KE-8 | H-171: alustamaton `baud`-arvo sarjayhteyden avauksessa, kun `baudst` on nollattu; käyttävätkö laitepolut `baudst`-taulua | Opus-tarkastaja luki H-171:n sarjaporttiasetusten käytön (pilotti 2026-11-27) | Opus-tarkastaja | ratkaisematta; koodi luettu osittain | Q107:n ajonaikaisen todennuksen yhteydessä |
| KE-9 | `cbHk/UnitPistelasku.cpp`: ehdossa `!yhdsarjat[srj-sarjaluku, kilp.Sarja()]` pilkkuoperaattori tekee lausekkeesta osoittimen, joten ehto on aina epätosi ja yhdistelmäsarjoihin ei ilmeisesti valikoidu ketään [päätelty]; tiedosto analyysitasolla 1 (auditoimaton) | agentin testivastaus Q23 kohdennetusta koodin lukemisesta auditoimattomassa tiedostossa; K-1:n Opus-arvioija vahvisti havainnon lukemalla (2026-11-27) | agentin testivastaus Q23, K-1:n Opus-arvioija (`doc/tietopohja/laatu/tarkastukset/k1-arvio.md`) | päätelty koodista kahden lukijan toimesta; ei ajettu, ei tarkastettu auditointimenettelyllä | tarkastus ja mahdollinen H-tunnus D-vaiheen valmistelussa |
| KE-10 | Emit-käsittelyssä (`pros_emit`) kilpailijan tulos tallennetaan ja kilpailijapaketit jonotetaan ennen emit-tietueen uudelleenkirjoitusta (`Hk/HkIV.cpp:1820-1822`, `:1845`); jos kirjoitus epäonnistuu, emit-tietuetta ei lähetetä mutta käsittelyosoitin etenee, jolloin vertaisilla on tulos ilman emit-tietuetta [päätelty]; sama järjestys viestissä (`Juk/VIv.cpp:1673-1679`) | K-3-aineiston Emit-selvitys (Opus, 2026-11-27) luki käsittelyketjun koodista | `doc/tietopohja/kasikirja/aineisto/emit-luenta.md` 12.3 | päätelty koodista; ei ajettu; läheinen H-145:lle (eri vaihe) | tarkastus E-1:n tai KK-C9-2:n yhteydessä |
| KE-11 | Uudelleenkäynnistyksen jälkeen käsittelemättömät paikalliset emit-tietueet, joita seuraa vertaiselta tullut käsitelty tietue, jäävät automaattikäsittelyn ulkopuolelle (`Hk/HkEmit.cpp:3103-3104`) [päätelty]; lähisukuinen H-137:lle | sama Emit-selvitys | `doc/tietopohja/kasikirja/aineisto/emit-luenta.md` 12.3 | päätelty koodista; ei ajettu; voi olla H-137:n ilmentymä | tarkastus: onko H-137:n laajennus vai erillinen |

## Tekninen velka / Technical debt

- Ydin riippuu GUI-otsikoista (`Hk/HkDeclare.h:23-28`, `Juk/VDeclare.h:26-27`) / core depends on GUI headers
- Kehäriippuvuudet `Hk ↔ Tp`, `Juk ↔ Tp`, `cbTp ↔ cbHk/ViestiWin` / cyclic dependencies
- Noin 850 `extern`-globaalia / about 850 extern globals
- Sekoitetut merkistöt; kaksi sekoitettua tiedostoa / mixed encodings; two mixed files
- Indy- ja ODBC-riippuvuudet ilman käyttöä / Indy and ODBC dependencies without use (`riippuvuudet.md`; tarkennettu C-10: T-005, T-044)
- C-10: suojaamattomat ulkoiset yhteydet (T-043), jakelun todennus (T-044), valitsimet projektien ulkopuolella (T-045, syöte C-12:lle), vanhentuvat alustan osat (T-046), kolmannen osapuolen koodin ehdot (T-047, omistajan päätös OM-C10-1), laitteiden elävyys (T-048), kuollut riippuvuuskoodi (T-049); kokonaisuudet / packages KK-C10-2–4 (`riippuvuudet.md` 19.4) / C-10 technical debt
- C-12: valitsinten hygienia (T-050: kuollut koodi, kirjoitusvirheet, käyttämättömät määritykset, `GDIPRINTER`-törmäys), kokoonpanojen ja jäsentimien ajautuminen (T-051), ViestiLuennan dokumentoimaton supistus (T-052) / switch hygiene, configuration drift, ViestiLuenta reduction
- JukLuenta linkittää vanhaan `vc10/`-hakemistoon / links against old `vc10/` (vain kokoonpanot `Debug250`, `Release150os`, C-10 / only two configurations)
- 32 orpoa tiedostoa / 32 orphan files (`inventaario.md`)
- Konsoli- ja GUI-versiot eri valitsimilla / console and GUI versions with different switches
- Tiedostoissa ei tunnistetta, versiota eikä tarkistussummaa; ei korjaustyökalua (T-019); pragma-hygienia (T-016); dbboxm:n kuollut indeksikerros (T-018) / no magic, version or checksum; pragma hygiene; dead index layer
- `makitp maki;` luokan ulkopuolella `#ifdef MAKI`-lohkossa (`Hk/HkDef.h`, `kilppvtp`:n jälkeen) – ei käännetä nykyisillä asetuksilla / outside a class in an `#ifdef MAKI` block – not compiled with current settings

- Vaiheriippuvan tilan nollaus ei ole keskitetty (T-038) ja johdetut rakenteet ylläpidetään käsin pareittain ilman invarianttitarkistusta (T-039): arkkitehtuurin kehityskohteita, huomioidaan vaiheen vaihdon ja hakemistojen korjauksissa (`globaali-tila.md` luku 7) / no central stage reset (T-038), hand-paired derived structures (T-039): architectural development items

## Päätökset / Decisions

| Päivä / Date | Päätös / Decision | Peruste / Rationale |
|---|---|---|
| 2026-11-24 | Arkkitehtuuridokumentit hakemistoon `doc/arkkitehtuuri/` / location | Kehittäjäanalyysien viereen; `docs-site/` on käyttäjille / next to developer analyses; `docs-site/` is for users |
| 2026-11-24 | Dokumentit kaksikielisiä samassa tiedostossa, suomi ensin / bilingual, Finnish first | Suomalaiset ylläpitäjät ja ulkopuoliset lukijat / Finnish maintainers and outside readers |
| 2026-11-24 | Uudet koodikommentit englanniksi ja ASCII:na / new code comments English, ASCII | Merkistöongelmat (`merkistot.md`) / encoding problems |
| 2026-11-24 | Pekan alkuperäiset kommentit säilytetään / Pekka's original comments are kept | Historiallinen tieto / historical knowledge |
| 2026-11-24 | Ensimmäisellä kierroksella ei muuteta `TPsource/`-hakemistoa / no `TPsource/` changes in round 1 | Ensin ymmärrys, sitten muutokset / understanding before changes |
| 2026-11-24 | C-11: testit haaralle `testit/vikojen-todentaminen`, ajuri Pythonilla, uusi todennustaso `toistettu testikehyksessä`; ei tuotantokoodin muutoksia, testikoukkuja eikä CI-muutoksia; testikoodi commitoidaan vain erillisellä hyväksynnällä / tests on their own branch, Python runner, new verification level; no production, hook or CI changes; test code committed only with approval | Käyttäjän ohje / user instruction |
| 2026-11-24 | Syväauditointi erinä riskijärjestyksessä, enintään 3 analyysiagenttia rinnakkain, jokainen erä riippumattomasti tarkastettu / deep audit in risk-ordered batches, max 3 parallel analysis agents, each batch independently reviewed | Käyttäjän ohje / user instruction |
| 2026-11-24 | C-11 vaihe 4: menetelmät A (toimitettu HkMaali.exe debuggerin alla) + C (komponenttitestit), ei B; rajapari 220/221, ei 250; WER vain debuggeritilalla, HKCU-muutos vain erillisellä luvalla; pudotuslokituksen ylläpitäjäkomennot ajaa käyttäjä; esiehdot erillisenä paikallisena committina, ajotuloksia ei commitoida ilman hyväksyntää; H-041 "vahvistettu ajamalla" vain yksiselitteisellä näytöllä / stage 4: methods A + C, no B; pair 220/221; WER via debugger mode only; user runs admin commands; prerequisites committed separately, run results only with approval | Käyttäjän ohje / user instruction |
| 2026-11-24 | C-4 vaihe 1: vain varoitusinventaario ja luokittelu; ei tuotantokoodin, projektiasetusten tai CI:n muutoksia; analyysikäännökset väliaikaishakemistoon MSBuildin `ForceImportAfterCppTargets`-ohjauksella; ReleaseVA200 ajettu `PlatformToolset=v145`-ohituksella; ei committia ilman hyväksyntää / warning inventory and triage only; no production, project or CI changes; analysis builds redirected to scratch; ReleaseVA200 with a toolset override; no commit without approval | Käyttäjän ohje / user instruction |
| 2026-11-25 | C-5 suljettu käyttäjän hyväksynnällä; ei uusia C-5-analyysejä tai -testejä; Q60 omistajalle, Q63–Q64 avoimina; 161 jäsennysehdokasta ja H-079:n uudelleenarviointi jatkotehtäviksi; C-6 valmistellaan mutta ei aloiteta. Työtapa C-6:sta alkaen: Sonnet oletusmallina, Opus vain perustelluissa vaativissa tarkastuksissa, ei päällekkäistä analyysiä, työerälle ennalta rajaus ja tokenbudjetti, olemassa olevat todisteet ennen uusia koeohjelmia, riippumaton tarkastus riskiperusteisesti / C-5 closed; follow-ups recorded; C-6 prepared, not started; working method from C-6: Sonnet by default, Opus only for justified demanding reviews, no overlapping analysis, scope and token budget fixed up front, existing evidence before new probes, risk-based independent review | Käyttäjän päätös; C-5:n agenttien kulutus noin 1,56 milj. tokenia / user decision; C-5 agents used about 1.56 M tokens |
| 2026-11-25 | C-6 vaihe 1 aloitettu ja tehty käyttäjän luvalla; tälle erälle ei keinotekoisia tokenrajoja (suunnitelman 380 k ei sitova), Sonnet inventointiin, Opus vakavien väitteiden riippumattomaan tarkastukseen; paikallinen commit sallittu, ei pushia / C-6 step 1 started and done on approval; no artificial token limits for this batch, Sonnet for inventory, Opus for reviewing severe claims; local commit allowed, no push | Käyttäjän ohje / user instruction |
| 2026-11-25 | C-6 vaihe 2 tehty käyttäjän luvalla: eristetyt koeohjelmat sallittu (ei tuotantotietoja, ei verkkoyhteyksiä, ei toimitettujen ohjelmien käynnistystä); koeohjelmat ja niiden projekti todisteina `todisteet/c6/vaihe2/`, ei muutoksia `TPsource/`-puuhun; ei tokenrajoja; paikallinen commit, ei pushia; C-7 vasta C-6:n sulkemispäätöksen jälkeen / C-6 step 2 done on approval; isolated probes allowed, kept as evidence outside TPsource | Käyttäjän ohje / user instruction |
| 2026-11-25 | C-6 suljettu käyttäjän hyväksynnällä; ei uusia C-6-analyysejä tai -testejä; ajoa vaativat avoimet asiat (bcc32, H-087/H-103 ViestiWinissä, Q67:n apusäikeet, Q68, Q73 ym.) jatkotehtäviksi; korjausjärjestys säilyy (P2 H-087, H-090, H-091, H-093, H-107), korjauksia ei aloiteta; C-7:n vaihe 1 rajataan mutta ei aloiteta / C-6 closed on approval; open items deferred; fix order kept; C-7 step 1 scoped, not started | Käyttäjän päätös / user decision |
| 2026-11-25 | C-7:n vaihe 1 aloitettu käyttäjän luvalla ja tehty rajauksen mukaan; poikkeama: täydentävä B-ajo kattavuusaukkojen vuoksi ja toinen Opus-tarkastuskierros; ei tokenrajaa (noin 1,0 milj.); paikallinen commit, ei pushia / C-7 step 1 started on approval and done as scoped; deviation: supplementary B pass and second review round; about 1.0 M tokens; local commit, no push | Käyttäjän lupa / user approval |
| 2026-11-25 | C-7 suljettu käyttäjän hyväksynnällä kattavuustarkistuksen jälkeen; ei uusia C-7-analyysejä; tekemättä jääneet osat jatkotyöjonoon J7-1–J7-7 (arvonta-algoritmi, 41 muuta lomaketta, 152 jäsennysehdokasta, ajonaikaiset vahvistukset, tarkastamattomat rivit, Q15/Q75–Q77, haarojen korjaussuunnittelu); C-8:n vaihe 1 rajataan mutta ei aloiteta / C-7 closed on approval after the coverage check; gaps to queue J7-1–J7-7; C-8 step 1 scoped, not started | Käyttäjän päätös / user decision |
| 2026-11-26 | C-8:n vaihe 1 aloitettu käyttäjän pyynnöstä ("C-8:n analyysi") rajauksen mukaisesti ilman tokenbudjettia; skripti-inventaario + Sonnet-analyysi (A–D) + Opus-tarkastus (E); valmis, odottaa tarkastusta / C-8 step 1 started at the user's request per the scope; done, awaiting review | Käyttäjän pyyntö / user request |
| 2026-11-26 | C-8 suljettu käyttäjän hyväksynnällä rajattujen tarkistusten jälkeen: lukumäärät täsmäytetty, H-128 ja H-129:n mekanismi toistettu eristetyllä koeohjelmalla (ei toimitetussa ohjelmassa), H-129 korjausjonoon P2; poikkeamat kirjattu; C-9:n vaihe 1 rajataan mutta ei aloiteta / C-8 closed on approval after bounded checks; C-9 step 1 scoped, not started | Käyttäjän päätös / user decision |
| 2026-11-26 | C-9:n vaihe 1 tehty käyttäjän pyynnöstä ("Otetaan sitten työstöön C9") rajauksen mukaisesti: osat A–F, Sonnet osissa A–D, Opus osassa E; osan A skripti korjattiin agentin otoksen perusteella (v2), ja H-149 (tarkastajan löytö) todennettiin pääistunnossa koodista; ei pushia / C-9 step 1 done on the user's request per the scope | Käyttäjän päätös / user decision |
| 2026-11-26 | C-9:n vaihe 2 tehty käyttäjän hyväksynnällä ("Hyväksyn C-9:n toisen työerän"): eristetyt koeohjelmat C9ProbeHk/C9ProbeJuk (Release, Asan) muuttamattomalla tuotantokoodilla, virheinjektio kuvaajan vaihdolla, bcc32-RTL-koe; Opus-tarkastus, jonka ehdottamat lisäkokeet (polku c TCP:llä, N1) ajettiin; H-149 → korkea; ei tuotantokoodin, projektien eikä CI:n muutoksia; ei pushia / C-9 step 2 done on approval | Käyttäjän päätös / user decision |
| 2026-11-26 | C-9 suljettu käyttäjän hyväksynnällä ("Hyväksyn C-9:n sulkemisen"): kattavuus tarkistettu rajausta vasten, todentamisen rajat ja kääntäjäerot kirjattu, korjausjärjestys viimeistelty (H-149 P2 kärkeen, ei P1; H-145 P3 kärkeen; H-150 P3 → P2 tiedostovirtojen loppumisen perusteella), korjauskokonaisuudet KK-C9-1–3; C-10:n vaihe 1 rajattu mutta ei aloitettu; ei pushia / C-9 closed on approval; C-10 step 1 scoped, not started | Käyttäjän päätös / user decision |
| 2026-11-26 | C-10:n vaihe 1 tehty käyttäjän luvalla suunnitelman rajauksella: B, C ja D rinnakkain, enintään kolme analyysiagenttia, Sonnet inventointiin ja analyysiin, Opus riippumattomaan tarkastukseen, ei keinotekoisia tokenbudjetteja; inventaarioskripti tarkistettava ennen agenttikäyttöä; staattinen analyysi: ei käännöksiä, ajoja, verkko-, tietokanta- tai laiteyhteyksiä, riippuvuuspäivityksiä, tuotantokoodin, projektiasetusten tai CI:n muutoksia, ei pushia; paikallinen commit sallittu. Suunnitelmaan lisätty uudet erät C-12 (ominaisuusvalitsimet, Q2, T-008) ja C-13 (synteesi ja korjaussuunnitelma), ei aloitettu / C-10 step 1 done on approval within the planned scope; new batches C-12 and C-13 added to the plan, not started | Käyttäjän päätös; alkuperäinen suunnitelma ja historia säilytetään / user decision; original plan and history kept |
| 2026-11-26 | C-10 suljettu käyttäjän hyväksynnällä ilman toista työerää ("Hyväksyn C-10:n sulkemisen ilman toista työerää"): kattavuus tarkistettu rajausta vasten; ajonaikaisesti todentamattomat havainnot merkitty; prioriteetit H-159 P2, H-160 P2 (P3 → P2 perusteltu, vakavuus ennallaan), muut P3–P5 perustein; H-161:n toiminnallinen vaikutus ja injektio eroteltu (injektio ei vahvistettu, Q105); H-169:n eheys, edellytykset ja palautettavuus (Q106); korjauskokonaisuudet KK-C10-1–4; lisenssit omistajan päätökseen OM-C10-1; tuoteinventaario täsmäytetty ja `/MT`-johtopäätös tarkennettu; C-12:n vaihe 1 rajattu mutta ei aloitettu; C-13 ei aloitettu; ei uusia analyysejä, koeohjelmia eikä pushia / C-10 closed on approval without a step 2; C-12 step 1 scoped, not started | Käyttäjän päätös / user decision |
| 2026-11-26 | C-12:n vaihe 1 tehty käyttäjän aloitusluvalla ("Aloitetaan C-12") suunnitelman rajauksella: A0 ja B0 skripteillä pääistunnossa ja tarkistettu ennen agenttikäyttöä (C-10:n tulosteet tavulleen ennallaan, yksi skriptivirhe korjattu); A, B ja C rinnakkain Sonnetilla, D Opusilla; ei keinotekoisia tokenbudjetteja; staattinen analyysi, ei käännöksiä, ajoja tai yhteyksiä, ei tuotantokoodin, projektien tai CI:n muutoksia, ei pushia; paikallinen commit / C-12 step 1 done on approval within the scope | Käyttäjän päätös / user decision |
| 2026-11-26 | **Vaihe C suljettu** käyttäjän päätöksellä ("Hyväksyn C-13:n ja koko C-vaiheen sulkemisen"): C-1–C-3 ja C-13 merkitty suljetuiksi; korjaus- ja testaussuunnitelma hyväksytty D-vaiheen lähtökohdaksi, mikä ei ole lupa aloittaa tuotantokoodin korjaamista; D-vaiheen ennakkotehtäviksi kirjattu rajattu riskiseulonta (H-033:n perusteella; ei uutta syväauditointia), haarojen `origin/88`, `90`, `92` ja `95` täsmäytys (ei yhdistetä eikä haeta verkosta vielä) ja auditointiaineiston säilytys: auditointihaaraa ei pushata julkiseen repositorioon ilman erillistä hyväksyntää, yksityinen varmuuskopio tarvitaan; kaikki 18 OP-kohtaa säilyvät avoimina, OP-1–OP-3 käsitellään ennen D-vaihetta; uusi vaihe K (ohjelmiston tietopohja ja käsikirja: asiantuntija-agentti, vikakortisto, käsikirja, paikallinen portaali) suunniteltu `k-vaihe-suunnitelma.md`:ään, toteutusta ei aloitettu; K-vaiheessa Sonnet ensisijaisesti, Opus vaikeisiin tulkintoihin ja laadunvarmistukseen, ei tokenbudjetteja / phase C closed by the user's decision; fix and test plans approved as the starting point of phase D without permission to start; preparatory tasks recorded; audit branch not to be pushed to a public repository without separate approval; phase K planned, not started | Käyttäjän päätös / user decision |
| 2026-11-27 | **Vaihe K käynnistetty** käyttäjän päätöksellä: K-vaiheen suunnitelma hyväksytty tarkennuksin (pilotin korttivalinta, koneluettavat käytännön vaikutukset, K-4:n lisävaatimukset, uusi K-5); tässä istunnossa K-0, K-1 ja K-2:n pilotti; pysähdytään pilotin jälkeen käyttäjän arvioon; ei tuotantokoodin muutoksia, ei pushia, ei haarojen yhdistämistä, ei omistajan päätöksiä / phase K started by the user; K-0, K-1 and the K-2 pilot in this session, then stop for review | Käyttäjän päätös / user decision |
| 2026-11-26 | C-13 tehty käyttäjän aloitusluvalla ("Aloita C-13 nyt") suunnitelman rajauksella ilman välihyväksyntöjä: pääistunto Fable, Sonnet inventointeihin ja mekaaniseen dokumentointiin, Opus juurisyihin, riskiperusteisiin, rajattuihin teknisiin kysymyksiin ja riippumattomaan kokonaisarvioon; enintään kolme analyysiagenttia rinnakkain, ei tokenbudjettia; `synteesi.py` tarkistettu ennen agenttikäyttöä; pääistunto muodosti itse korjauskokonaisuudet, järjestyksen ja kehittämissuunnitelman ja arvioi agenttien ehdotukset näyttöä vasten (kolme juurisyyanalyysin muutospyyntöä ja kaksi tarkastuksen kohtaa hyväksytty vain osin); ei tuotantokoodin, projektien tai CI:n muutoksia, ei käännöksiä tai ajoja, ei verkkoyhteyksiä, ei pushia; paikallinen commit; vaihetta C ei merkitty suljetuksi / C-13 done on the user's go-ahead within the scope; phase C not marked closed | Käyttäjän päätös / user decision |
| 2026-11-26 | C-12 suljettu käyttäjän hyväksynnällä ilman toista työerää ("Hyväksyn C-12:n sulkemisen ilman toista työerää"): kattavuus ja rekisterien johdonmukaisuus tarkistettu, testimatriisia ei ajettu, poistopäätöksiä ei tehty; H-171:n vakavuus tarkistettu rajatusti (puskurin koko, kirjoituspituus, sijoittelu olemassa olevasta bcc32-objektista, saavutettavuus, toistuvuus) → korkea, P2; H-134 kytketty vaiheen vaihdon, globaalin tilan, elinkaaren ja virheenkäsittelyn havaintoihin; H-172 julkaisutiedostojen kokonaisuuteen KK-C12-1; Q107, Q108, Q110 ajonaikaiseen todennukseen, Q111 omistajalle, Q112 avoin; C-13:n vaihe 1 rajattu, ei aloitettu; ei käännöksiä, ajoja eikä pushia / C-12 closed on approval; H-171 high, P2; C-13 scoped | Käyttäjän päätös / user decision |
| 2026-11-25 | C-5 vaihe 1: riippumaton tarkastus ja dokumentointi ilman välihyväksyntöjä, paikallinen commit sallittu, ei pushia; sen jälkeen C-5:n jäljellä olevat tavoitteet rajattuna staattisena työeränä; ei tuotantokoodin, projektiasetusten tai CI:n muutoksia, ei toimitettujen ohjelmien ajoja eikä verkkokokeita, ei uusia laajoja testikehyksiä; epäselvyydet avoimiksi kysymyksiksi / step 1 review and write-up without intermediate approvals, local commit allowed, no push; then the remaining C-5 goals as a bounded static step; no production, project or CI changes, no runs of shipped programs, no network tests, no new large harnesses | Käyttäjän ohje / user instruction |
| 2026-11-25 | C-4 suljettu käyttäjän hyväksynnällä; H-071, H-072, H-055 ja H-070 kiireellisesti korjattaviksi; väliaikaiset suojaukset vain suosituksina, verkko- tai tuotantoasetuksia ei muutettu; C-5 seuraavaksi aloitusluvan jälkeen / C-4 closed on approval; urgent fixes marked; mitigations as recommendations only; C-5 next on approval | Käyttäjän päätös / user decision |
| 2026-11-25 | C-4 vaihe 2 (viimeinen): vain analyysi; ei ASan-sarjoja, uusia testikehyksiä, GUI-ajoja, korjauksia eikä CI-muutoksia; vaihe 1 commitoitiin hyväksynnän ja uudelleentarkastuksen jälkeen (`81bcad9`), vaihetta 2 ei commitoida ilman hyväksyntää; uusi H vain, jos kyse ei ole rekisteröidyn vian toisesta ilmenemismuodosta / analysis only; step 1 committed after approval and re-review; step 2 not committed without approval | Käyttäjän ohje / user instruction |
| 2026-11-24 | H-041: ennalta kiinnitetty arvio "not confirmed" säilyy; käyttäjä hyväksyi dokumentoidun poikkeaman (prosessi 26236 = kontrolliajon jälkikoeohjelma, ei HkMaali.exe:n liikennettä) ja H-041 nostetaan tasolle "vahvistettu ajamalla"; raakatuloksia ja kriteerejä ei muuteta; uutta ajoa ei tehdä; C-11 suljetaan, H-009 ja H-014 jäävät avoimiksi; seuraavaksi C-4 / pre-fixed verdict kept; user-approved deviation (pid 26236); H-041 confirmed by running; C-11 closed; next C-4 | Käyttäjän päätös / user decision |

## Oletukset, joita ei ole todennettu / Unverified assumptions

- Viestin (`Juk/`) päävirta vastaa HK:n virtaa (`yleiskuva.md`) / relay main flow matches HK
- Säikeiden tehtävät on päätelty nimistä ja käynnistyspaikoista; `tallenna`-polut on jäljitetty erässä 1, muut tehtävät eivät / thread tasks inferred from names and start sites; `tallenna` paths traced in batch 1, other tasks not
- Tiedostojen roolit on päätelty nimistä (`riippuvuudet.md`) / file roles inferred from names
- ~~`bcc32` käsittelee kaikki UTF-8-tiedostot oikein~~ **kumottu C-5:ssä / refuted in C-5:** bcc32 lukee BOMittoman UTF-8:n ANSI-koodisivuna kuten MSVC; aiempi todiste oli väärin tulkittu (`kaantajaerot.md` luku 5) / reads UTF-8 without BOM as the ANSI code page like MSVC
