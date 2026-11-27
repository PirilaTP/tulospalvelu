# Arkkitehtuuridokumentaatio / Architecture documentation

## Mikä tämä on / What this is

Tämä hakemisto kuvaa tulospalveluohjelmiston rakenteen: moduulit, riippuvuudet,
datavirrat, tiedostomuodot ja tunnetut riskit. Se on kehittäjille, ei käyttäjille;
käyttöohje on `docs-site/`-hakemistossa. Dokumentit ovat kaksikielisiä: jokaisessa
osiossa on ensin suomenkielinen ja sitten englanninkielinen teksti. Taulukot,
kaaviot ja koodiviitteet ovat vain kerran.

This directory describes the structure of the timekeeping suite: modules,
dependencies, data flows, file formats and known risks. It is for developers, not
end users; the user manual lives in `docs-site/`. Documents are bilingual: each
section has Finnish text first and English text after it. Tables, diagrams and
code references appear only once.

## Tila ja kattavuuden rajat / State and limits of coverage

**Vaihe C (syväauditointi, erät C-1–C-13) on suljettu 2026-11-26.** Yhtään vikaa ei ole korjattu.
Dokumentaatio kuvaa sen, mitä on tutkittu; se ei ole koko lähdekoodin katselmointi:

- 631 tuotantotiedostosta 474 (36 % riveistä) on vain inventoitu skriptillä, 135 luettu
  kohdennetusti ja 22 rakenteen tasolla; yhtäkään ei ole luettu kokonaan vikojen varalta.
- 177 havainnosta yksi on vahvistettu toimitetussa ohjelmassa ja 26 toistettu koeohjelmalla tai
  testikehyksessä; muut perustuvat koodin lukemiseen. GUI-ohjelmia ei ole ajettu.
- Havaintojen määrä on alaraja. Yksityiskohdat: `auditoinnin-yhteenveto.md` luvut 3, 4, 10 ja 12.

**Phase C (the deep audit, batches C-1..C-13) was closed on 2026-11-26.** No defect has been
fixed. The documentation describes what was examined; it is not a review of the whole code base:
474 of the 631 production files (36 % of the lines) are inventory only, no file was read in full,
one of the 177 findings is confirmed in a shipped program and 26 are reproduced with a probe or the
harness. The number of findings is a lower bound.

Aineisto sisältää yksityiskohtia korjaamattomista tietoturvavioista. Sitä ei julkaista eikä
pushata julkiseen repositorioon ilman erillistä hyväksyntää. / The material contains details of
unfixed security defects and must not be published or pushed to a public repository without
separate approval.

## Lukujärjestys / Reading order

| Tiedosto / File | Sisältö / Content |
|---|---|
| [CHECKPOINT.md](CHECKPOINT.md) | Tilanne ja jatkamisohjeet / current state and resume instructions |
| [yleiskuva.md](yleiskuva.md) | Arkkitehtuuri, moduulit, päävirrat, kaaviot / Architecture, modules, main flows, diagrams |
| [inventaario.md](inventaario.md) | Projektit, binäärit, hakemistot, koot, merkistöt / Projects, binaries, directories, sizes, encodings |
| [auditoinnin-yhteenveto.md](auditoinnin-yhteenveto.md) | **Erä C-13: vaiheen C yhteenveto**: kattavuus, todennuksen vahvuus, juurisyyt JS-1–JS-15, yhtenäinen riskiarvio P1–P5, riippumattoman tarkastuksen käsittely / **batch C-13: summary of phase C**: coverage, strength of verification, root causes, uniform risk assessment, handling of the independent review |
| [korjaussuunnitelma.md](korjaussuunnitelma.md) | Erä C-13: 34 korjauskokonaisuutta, riippuvuudet ja toteutusjärjestys, yhteensopivuus, D-vaiheen kehittämissuunnitelma ja ensimmäinen toteutuserä / batch C-13: fix packages, order, compatibility, development plan for phase D and its first batch |
| [regressiotestaussuunnitelma.md](regressiotestaussuunnitelma.md) | Erä C-13: testilajit, testit kokonaisuuksittain, infrastruktuurityöt, julkaisun testaus / batch C-13: kinds of tests, tests per package, infrastructure work, release testing |
| [omistajan-paatokset.md](omistajan-paatokset.md) | Erä C-13: omistajan päätöstä vaativat asiat OP-1–OP-18 vaihtoehtoineen / batch C-13: owner decisions with options |
| [k-vaihe-suunnitelma.md](k-vaihe-suunnitelma.md) | Vaiheen K suunnitelma ja tila (K-0–K-5 toteutettu 2026-11-27; tuotokset `doc/tietopohja/`): asiantuntija-agentti, selkokielinen vikakortisto, toiminnallinen käsikirja, paikallinen portaali, tietopohjan ylläpito / plan and state of phase K (K-0, K-1 done; K-2 pilot under review): expert agent, plain-language defect cards, functional handbook, local portal, maintenance |
| [c14-suunnitelma.md](c14-suunnitelma.md) | C-14: koko tuotantolähdekoodin katselmoinnin lähtötaso (funktiotaso, `todisteet/c14/lahtotaso/`, `tyokalut/kattavuus.py`) ja toteutussuunnitelma; odottaa hyväksyntää / baseline of review coverage and plan for a full review; awaiting approval |
| [riippuvuudet.md](riippuvuudet.md) | Käännös- ja ajonaikaiset riippuvuudet, protokollat, tiedostot; luvut 7–19: erä C-10 (tuotekohtainen inventaario, tietokanta, HTTP, laitteet, jakelu; H-159–H-170, T-043–T-049) / Build and runtime dependencies, protocols, files; sections 7–19: batch C-10 |
| [tietorakenteet-ja-tila.md](tietorakenteet-ja-tila.md) | Keskeiset rakenteet, globaali tila, säikeet / Core structures, global state, threads |
| [merkistot.md](merkistot.md) | Merkistöt ja turvallinen muokkausmenettely / Encodings and the safe editing procedure |
| [tallenna-samanaikaisuus.md](tallenna-samanaikaisuus.md) | Syväauditointi 1: `tallenna()`, säikeet ja lukot / Deep audit 1: `tallenna()`, threads and locks |
| [tiedonsiirto.md](tiedonsiirto.md) | Syväauditointi 2: työasemien tiedonsiirto ja synkronointi / Deep audit 2: workstation communication and sync |
| [tiedostomuodot.md](tiedostomuodot.md) | Syväauditointi 3: pysyvät tiedostot, tavuasettelut, mitatut koot ja eheys / Deep audit 3: persistent files, byte layouts, measured sizes and integrity |
| [muistiturvallisuus.md](muistiturvallisuus.md) | Erä C-4: kääntäjävaroitukset (MSVC `/W4 /analyze`, bcc32 `-w`), luokittelu, uudet muistiviat / Batch C-4: compiler warnings, triage, new memory defects |
| [kaantajaerot.md](kaantajaerot.md) | Erä C-5: kääntäjäkohtainen käytös (asetukset, rakenteiden asettelu käännösyksiköiden välillä, merkistö, ajonaikaiset erot) / Batch C-5: compiler-specific behaviour |
| [gui-liiketoimintalogiikka.md](gui-liiketoimintalogiikka.md) | Erä C-7 vaihe 1: kahdeksan GUI-lomakeyksikön moduulikuvaukset, HK/viesti-erot, liiketoimintalogiikan viat H-109–H-127, C-5:n jäsennysehdokkaat / Batch C-7 step 1: module descriptions of eight GUI form units, HK/relay differences, business-logic defects, parse candidates |
| [globaali-tila.md](globaali-tila.md) | Erä C-8 vaihe 1: globaalien ja staattisten muuttujien inventaario ja luokittelu, elinkaari ja vaiheen vaihdon nollaustaulukko, johdettujen rakenteiden eheys, säikeiden yhteinen tila, viat H-128–H-144 / Batch C-8 step 1: inventory and classification of globals and statics, lifecycle and stage-change reset table, derived-structure integrity, shared state between threads |
| [virheenkasittely.md](virheenkasittely.md) | Erä C-9 vaihe 1: virheenkäsittely ja resurssit: resurssi- ja virhekutsujen inventaario, pysyvän datan kirjoitusvirheiden käsittely tiedostoittain, resurssien elinkaari kilpailun aikana, tila virheilmoituksen jälkeen ja lopetuspolut; vaihe 2: eristetyt koeohjelmat; H-145–H-158, T-040–T-042 / batch C-9 steps 1–2: error handling and resources |
| [ominaisuusvalitsimet.md](ominaisuusvalitsimet.md) | Erä C-12 vaihe 1: ominaisuusvalitsimet ja asetukset: esikääntäjän valitsinten luokittelu ja merkitys (Q2), konsoli vs. GUI (T-008), asetusavaimet ja komentorivi, muut kytkimet, ominaisuus- ja testimatriisi; H-171–H-177, T-050–T-052 / batch C-12 step 1: feature switches and settings |
| [saikeet-ja-lukot.md](saikeet-ja-lukot.md) | Erä C-6 vaiheet 1–2: säiekartta, elinkaari ja lopetus, lukkokartta, lukitusjärjestys, Q30; vaihe 2: polkujen todennus eristetyillä koeohjelmilla, VCL-säieturvallisuus / Batch C-6 steps 1–2: thread map, lifecycle, lock map, lock order; step 2: probe verification, VCL thread safety |
| [testaus.md](testaus.md) | Erä C-11: testausstrategia, testikehys (`TpCoreHk`, `TpCoreJuk`, ajuri) ja vaiheiden 1–4 tulokset: 15 havaintoa toistettu testikehyksessä, H-041 toimitetussa ohjelmassa / Batch C-11: test strategy, the harness and the results of stages 1–4 |
| [rekisteri/](rekisteri/README.md) | Auditointirekisteri: tiedostot, moduulit, havainnot, valitsimet, asetusavaimet, riskiarvio, korjauskokonaisuudet / Audit register: files, modules, findings, switches, config keys, risk assessment, fix packages |
| [dokumentointistandardi.md](dokumentointistandardi.md) | Kommentointikäytännöt / Commenting conventions |
| [suunnitelma.md](suunnitelma.md) | Moduulikohtainen arvio ja vaiheistus / Per-module assessment and phasing |
| [seuranta.md](seuranta.md) | Edistyminen, avoimet kysymykset, epäillyt viat / Progress, open questions, suspected defects |

## Varmuusasteet / Confidence levels

Väitteet on merkitty näin:

- **[todennettu]** – tarkistettu koodista, käännöksestä tai ajamalla
- **[päätelty]** – johdettu koodista, mutta ei ajettu tai jäljitetty loppuun
- **[tuntematon]** – ei selvitetty; ks. `seuranta.md`

Claims are marked as follows:

- **[todennettu] / verified** – checked against code, a build or a run
- **[päätelty] / inferred** – derived from code, but not run or fully traced
- **[tuntematon] / unknown** – not established; see `seuranta.md`

[todennettu] kertoo, että väite on tarkistettu koodista; se ei tarkoita, että vika
olisi toistettu käynnissä olevassa ohjelmassa. Staattisen analyysin,
riippumattoman lähdetarkastuksen ja ajonaikaisen todennuksen ero: `CHECKPOINT.md`.

[verified] means the claim was checked against the code; it does not mean the
defect was reproduced in the running program. Static analysis vs independent
source review vs runtime verification: `CHECKPOINT.md`.

## Jatkaminen myöhemmissä istunnoissa / Continuing in later sessions

Aloita tiedostosta `CHECKPOINT.md` ja sitten `seuranta.md`: niissä on tehdyt vaiheet, avoimet kysymykset ja
seuraava askel. Inventaarion luvut on tuotettu skriptillä, joka käy läpi hakemiston
`TPsource/V52` (ks. `inventaario.md`, kohta "Miten tuotettu"). Jos koodi muuttuu
paljon, luvut kannattaa tuottaa uudelleen ennen kuin niihin luotetaan.

Start from `CHECKPOINT.md`, then `seuranta.md`: they list completed steps, open questions and the next
step. Inventory figures were produced by a script that walks `TPsource/V52` (see
`inventaario.md`, "How it was produced"). If the code changes significantly,
regenerate the figures before relying on them.

Selvitys tehtiin 2026-11-24–26 commitin `f7fe553` päälle; vaiheen C synteesi (C-13) valmistui
2026-11-26, ja vaihe C suljettiin samana päivänä. Tuotantokoodia ei ole muutettu.
The audit was made on top of commit `f7fe553` on 2026-11-24..26; the synthesis of phase C (C-13)
was completed on 2026-11-26 and phase C was closed the same day. No production code has been changed.
