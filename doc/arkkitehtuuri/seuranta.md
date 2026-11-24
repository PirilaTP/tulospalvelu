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
| B. Vikakorjaukset / bug fixes | ei aloitettu / not started | – |
| C, erä 1: `tallenna()`-samanaikaisuus / batch 1: `tallenna()` concurrency | valmis, tarkastettu / done, reviewed (`tallenna-samanaikaisuus.md`) | 2026-11-24 |
| C, erä 2: työasemien tiedonsiirto / batch 2: workstation communication | valmis, tarkastettu / done, reviewed (`tiedonsiirto.md`) | 2026-11-24 |
| C, erä 3: pysyvät tiedostot ja eheys / batch 3: persistent files and integrity | valmis, tarkastettu / done, reviewed (`tiedostomuodot.md`) | 2026-11-24 |
| C, erä 11: testiautomaatio, suunnittelu / batch 11: test automation, planning | valmis, tarkastettu, commitoitu `f5d793b` / done, reviewed, committed (`testaus.md`) | 2026-11-24 |
| C, erä 11, vaihe 1: H-025, H-040, H-042 / batch 11, stage 1 | valmis, hyväksytty, commitoitu `79d9957` / done, approved, committed (`testaus.md` luku 13) | 2026-11-24 |
| C, erä 11, vaihe 2: H-041, H-031, H-008, H-024, H-027 / batch 11, stage 2 | valmis, hyväksytty, commitoitu `cad2781` / done, approved, committed (`testaus.md` luku 14) | 2026-11-24 |
| C, erä 11, vaihe 3: H-026, H-032, H-022, H-023, H-007, H-001, H-002 / batch 11, stage 3 | valmis, hyväksytty, commitoitu / done, approved, committed (`testaus.md` luku 15) | 2026-11-24 |
| HkMaali.exe:n koeajot 0 ja 1 / black-box runs 0 and 1 | tehty palomuurieristyksellä, molemmat odotetusti; ensimmäinen AppContainer-yritys keskeytyi; ei H-vahvistuksia; done under firewall isolation, both as expected; first AppContainer attempt stopped; no H confirmed (`432ea3d`, `testaus.md` luku 17) | 2026-11-24 |
| C-11 vaihe 4: H-041 toimitetussa ohjelmassa / stage 4 | esiehdot `c6205d2`; ajot 220/221; ennalta kiinnitetty arvio "not confirmed" (kriteeri 7, loki-ikkuna); käyttäjän poikkeamapäätöksellä H-041 **vahvistettu ajamalla** / pre-fixed verdict not confirmed; H-041 confirmed by running by user-approved deviation (`testaus.md` 19.9–19.11) | 2026-11-24 |
| **C, erä 11 (C-11) kokonaisuutena / batch 11 as a whole** | **valmis / complete**; loppuyhteenveto / final summary `testaus.md` luku 20; raakatulokset / raw results `todisteet/c11/` | 2026-11-24 |
| C, erä 4 (C-4) muistiturvallisuus / memory safety | seuraava, ei aloitettu; aloitusvalmius / readiness `testaus.md` 20.8 / next, not started | – |
| C, erät 5–10 / batches 5–10 | ei aloitettu / not started (`suunnitelma.md`); erä 5 osittain / batch 5 partly | – |
| D. Kommentointi / commenting | ei aloitettu / not started | – |

**Seuraava askel / Next step:** C-4 muistiturvallisuus (`suunnitelma.md` erä 4) luvan
jälkeen; ensin varoitusinventaario molemmilla kääntäjillä. / C-4 memory safety after
approval, starting with a warning inventory on both compilers.

**Avoimet jatkotehtävät C-11:stä / open follow-ups from C-11** (eivät estäneet
sulkemista / did not block closing; `testaus.md` 20.4):
- H-009: käsitesti GUI:ssa (pakotettu ajoitus) / manual GUI test (forced timing)
- H-014: käsitesti HkKisaWin + paikallinen MySQL / manual test with local MySQL
- kahden prosessin loopback-kerros (H-022, H-032, H-027); H-041:n lukupolku (Q37) ja 250;
  testit CI:hin (Q35, Q36, Q40) / two-process loopback layer; H-041 read path; CI
- palomuurisääntöjen `TP C11 HkMaali block in/out` poisto (käyttäjä, ylläpitäjä) /
  removal of the C-11 firewall rules (user, admin)

Havainnot (H/T) ja niiden todennustila: `rekisteri/havainnot.md`.
Findings (H/T) and their verification status: `rekisteri/havainnot.md`.

## Moduulien dokumentointi / Module documentation

| Moduuli / Module | Arkkitehtuuri / Architecture | Koodikommentit / Code comments |
|---|---|---|
| `Hk/` | yleiskuva, päävirta / overview, main flow | – |
| `Juk/` | yleiskuva (päätelty HK:n pohjalta) / overview (inferred from HK) | – |
| `Tp/` | yleiskuva, säikeet / overview, threads | – |
| `com/` | kuljetus (`tiedonsiirto.md`) / transport | – |
| `cbHk/`, `ViestiWin/`, `cbTp/` | logiikkaa sisältävät tunnistettu / forms with logic identified | – |
| `tputilv2/`, `include/`, `dbboxm/` | rajapinta mainittu / interface mentioned | – |
| `HkConsole/`, `JukConsole/` | käynnistys / startup | – |

## Avoimet kysymykset / Open questions

| # | Kysymys / Question | Missä / Where |
|---|---|---|
| ~~Q1~~ | **Ratkaistu / resolved 2026-11-24:** työsäie `tarkcom`; samanaikaisuus mahdollinen lukitsemattomien kutsujen kautta / worker thread `tarkcom`; concurrency possible via unlocked callers (`tallenna-samanaikaisuus.md`, H-007, H-010) | – |
| Q12 | Kumpi `add_sakko` linkitetään HkKisaWiniin? / Which `add_sakko` is linked? | `cbHk/UnitASakot.cpp:269`, `cbHk/UnitASuunn.cpp:289` |
| ~~Q13~~ | **Ratkaistu / resolved (C-2):** kyllä, kaikki perustiedot paitsi koon 0 kentät; ei vaiheita / yes, all base fields except size-0 fields; no stages (`tiedonsiirto.md`) | – |
| Q14 | Kutsuuko `tulostasarja` lukon aikana `SendMessage`-polkua? / Does `tulostasarja` reach a `SendMessage` while holding the lock? | `Hk/HkTulostus.cpp:5850`–`:6419` |
| Q15 | Voiko VCL-poikkeus `tallenna`-polulla jättää lukon päälle? / Can a VCL exception leave the lock held? | `tallenna-samanaikaisuus.md` |
| ~~Q16~~ | **Ratkaistu / resolved (C-2):** viestin vastaanotto lukitsee ennen lukua, ei H-007:ää; omat viat H-024–H-026 / relay locks before reading, no H-007; own defects H-024–H-026 | – |
| ~~Q17~~, ~~Q18~~ | **Ratkaistu / resolved (C-3):** bcc32 noudattaa `-K`:ta kesken yksikön, tarkistussummat täsmäävät; koot mitattu kaikista käännöksistä / bcc32 honours `-K` mid-unit; sizes measured for all builds | `tiedostomuodot.md` |
| Q19–Q25 | Erän C-2 kysymykset (AIKAT-kaksoiskappaleet, TCP-virta, rengas, `tark_ylahto`, `Kysy`, kirjastot, MONITORI-portti) / batch C-2 questions | `tiedonsiirto.md`, "Avoimet kysymykset" |
| Q26–Q34 | Erän C-3 kysymykset (HK:n XML-rajat, NTFS-katkos, `initindex`, viestin vienti, vaiheen vaihto, viestin 250-käännös, `t_time_l`, `mbtowc`, konstruktorijärjestys) / batch C-3 questions | `tiedostomuodot.md`, "Avoimet kysymykset" |
| Q35–Q42 | Erän C-11 kysymykset (v143 ja ASan CI:ssä, `LockCount`, H-041 käynnistyksessä, H-001:n sisääntulo, `/we4789` ja LTCG, interaktiivinen CI-istunto, viestin `addtall`-säikeet, HK:n luokkapituudet) / batch C-11 questions | `testaus.md`, "Avoimet kysymykset" |
| Q2 | Mitä kukin ominaisuusvalitsin tekee (`GAZ`, `ORAVA`, `VERTAA`, `MERI5`, `LAJUNEN`, `PARTIO` …)? Miksi konsoli- ja GUI-HK eroavat? / What does each feature switch do? Why do console and GUI HK differ? | `inventaario.md` |
| ~~Q3~~ | **Osittain ratkaistu / partly resolved (C-2):** kehys, kuittaus ja yhteensopivuus kuvattu; tarkat koot avoinna (Q18) / frame, ack and compatibility documented; exact sizes open | `tiedonsiirto.md` |
| ~~Q4~~ | **Ratkaistu / resolved (C-3):** muoto kuvattu tavutasolla; vanhaa muotoa ei tunnisteta automaattisesti, vain GUI-tuonti käyttäjän parametrein / layout documented; no automatic old-format detection, only GUI import with user parameters | `tiedostomuodot.md` |
| Q5 | MySQL-taulujen rakenne ja milloin SQL on käytössä / SQL table layout and when SQL is used | `cbHk/UnitMySQL.cpp` |
| Q6 | Ovatko kirjastoon käännetyt `main`-funktiot (`tputilv2/ctrlhndl.cpp` ym.) esikääntäjällä pois? / Are the `main` functions in the library excluded by the preprocessor? | `inventaario.md` |
| Q7 | Kääntääkö `bcc32` UTF-8 BOMilla -tiedostot oikein? / Does `bcc32` compile UTF-8 with BOM correctly? | `merkistot.md` |
| Q8 | Miten GUI-tiedostojen kommenttimuutokset todennetaan? (ehdotus: kommentit poistava vertailu) / How to verify comment changes in GUI files? | `suunnitelma.md` |
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

## Tekninen velka / Technical debt

- Ydin riippuu GUI-otsikoista (`Hk/HkDeclare.h:23-28`, `Juk/VDeclare.h:26-27`) / core depends on GUI headers
- Kehäriippuvuudet `Hk ↔ Tp`, `Juk ↔ Tp`, `cbTp ↔ cbHk/ViestiWin` / cyclic dependencies
- Noin 850 `extern`-globaalia / about 850 extern globals
- Sekoitetut merkistöt; kaksi sekoitettua tiedostoa / mixed encodings; two mixed files
- Indy- ja ODBC-riippuvuudet ilman käyttöä / Indy and ODBC dependencies without use (`riippuvuudet.md`)
- JukLuenta linkittää vanhaan `vc10/`-hakemistoon / links against old `vc10/`
- 32 orpoa tiedostoa / 32 orphan files (`inventaario.md`)
- Konsoli- ja GUI-versiot eri valitsimilla / console and GUI versions with different switches
- Tiedostoissa ei tunnistetta, versiota eikä tarkistussummaa; ei korjaustyökalua (T-019); pragma-hygienia (T-016); dbboxm:n kuollut indeksikerros (T-018) / no magic, version or checksum; pragma hygiene; dead index layer
- `makitp maki;` luokan ulkopuolella `#ifdef MAKI`-lohkossa (`Hk/HkDef.h`, `kilppvtp`:n jälkeen) – ei käännetä nykyisillä asetuksilla / outside a class in an `#ifdef MAKI` block – not compiled with current settings

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
| 2026-11-24 | H-041: ennalta kiinnitetty arvio "not confirmed" säilyy; käyttäjä hyväksyi dokumentoidun poikkeaman (prosessi 26236 = kontrolliajon jälkikoeohjelma, ei HkMaali.exe:n liikennettä) ja H-041 nostetaan tasolle "vahvistettu ajamalla"; raakatuloksia ja kriteerejä ei muuteta; uutta ajoa ei tehdä; C-11 suljetaan, H-009 ja H-014 jäävät avoimiksi; seuraavaksi C-4 / pre-fixed verdict kept; user-approved deviation (pid 26236); H-041 confirmed by running; C-11 closed; next C-4 | Käyttäjän päätös / user decision |

## Oletukset, joita ei ole todennettu / Unverified assumptions

- Viestin (`Juk/`) päävirta vastaa HK:n virtaa (`yleiskuva.md`) / relay main flow matches HK
- Säikeiden tehtävät on päätelty nimistä ja käynnistyspaikoista; `tallenna`-polut on jäljitetty erässä 1, muut tehtävät eivät / thread tasks inferred from names and start sites; `tallenna` paths traced in batch 1, other tasks not
- Tiedostojen roolit on päätelty nimistä (`riippuvuudet.md`) / file roles inferred from names
- `bcc32` käsittelee kaikki UTF-8-tiedostot oikein (todennettu yhdellä merkkijonolla) / handles all UTF-8 files correctly (verified with one string)
