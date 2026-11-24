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
| C, erä 11, vaihe 1: H-025, H-040, H-042 / batch 11, stage 1 | valmis, tarkastettu, commitoimatta haaralla `testit/vikojen-todentaminen`; odottaa käyttäjän tarkastusta / done, reviewed, uncommitted; awaiting user review (`testaus.md` luku 13) | 2026-11-24 |
| C, erät 4–10 / batches 4–10 | ei aloitettu / not started (`suunnitelma.md`); erä 5 osittain / batch 5 partly | – |
| D. Kommentointi / commenting | ei aloitettu / not started | – |

**Seuraava askel / Next step:** käyttäjä tarkastaa C-11:n vaiheen 1 (`testaus.md`,
luku 13) ja päättää testikoodin commitoinnista sekä seuraavien viiden testin
(H-041, H-031, H-008, H-024, H-027) aloittamisesta. / The user reviews C-11 stage 1,
decides on committing the test code and on starting the next five tests.

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

## Oletukset, joita ei ole todennettu / Unverified assumptions

- Viestin (`Juk/`) päävirta vastaa HK:n virtaa (`yleiskuva.md`) / relay main flow matches HK
- Säikeiden tehtävät on päätelty nimistä ja käynnistyspaikoista; `tallenna`-polut on jäljitetty erässä 1, muut tehtävät eivät / thread tasks inferred from names and start sites; `tallenna` paths traced in batch 1, other tasks not
- Tiedostojen roolit on päätelty nimistä (`riippuvuudet.md`) / file roles inferred from names
- `bcc32` käsittelee kaikki UTF-8-tiedostot oikein (todennettu yhdellä merkkijonolla) / handles all UTF-8 files correctly (verified with one string)
