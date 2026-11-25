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

## Lukujärjestys / Reading order

| Tiedosto / File | Sisältö / Content |
|---|---|
| [CHECKPOINT.md](CHECKPOINT.md) | Tilanne ja jatkamisohjeet / current state and resume instructions |
| [yleiskuva.md](yleiskuva.md) | Arkkitehtuuri, moduulit, päävirrat, kaaviot / Architecture, modules, main flows, diagrams |
| [inventaario.md](inventaario.md) | Projektit, binäärit, hakemistot, koot, merkistöt / Projects, binaries, directories, sizes, encodings |
| [riippuvuudet.md](riippuvuudet.md) | Käännös- ja ajonaikaiset riippuvuudet, protokollat, tiedostot / Build and runtime dependencies, protocols, files |
| [tietorakenteet-ja-tila.md](tietorakenteet-ja-tila.md) | Keskeiset rakenteet, globaali tila, säikeet / Core structures, global state, threads |
| [merkistot.md](merkistot.md) | Merkistöt ja turvallinen muokkausmenettely / Encodings and the safe editing procedure |
| [tallenna-samanaikaisuus.md](tallenna-samanaikaisuus.md) | Syväauditointi 1: `tallenna()`, säikeet ja lukot / Deep audit 1: `tallenna()`, threads and locks |
| [tiedonsiirto.md](tiedonsiirto.md) | Syväauditointi 2: työasemien tiedonsiirto ja synkronointi / Deep audit 2: workstation communication and sync |
| [tiedostomuodot.md](tiedostomuodot.md) | Syväauditointi 3: pysyvät tiedostot, tavuasettelut, mitatut koot ja eheys / Deep audit 3: persistent files, byte layouts, measured sizes and integrity |
| [muistiturvallisuus.md](muistiturvallisuus.md) | Erä C-4: kääntäjävaroitukset (MSVC `/W4 /analyze`, bcc32 `-w`), luokittelu, uudet muistiviat / Batch C-4: compiler warnings, triage, new memory defects |
| [kaantajaerot.md](kaantajaerot.md) | Erä C-5: kääntäjäkohtainen käytös (asetukset, rakenteiden asettelu käännösyksiköiden välillä, merkistö, ajonaikaiset erot) / Batch C-5: compiler-specific behaviour |
| [gui-liiketoimintalogiikka.md](gui-liiketoimintalogiikka.md) | Erä C-7 vaihe 1: kahdeksan GUI-lomakeyksikön moduulikuvaukset, HK/viesti-erot, liiketoimintalogiikan viat H-109–H-127, C-5:n jäsennysehdokkaat / Batch C-7 step 1: module descriptions of eight GUI form units, HK/relay differences, business-logic defects, parse candidates |
| [globaali-tila.md](globaali-tila.md) | Erä C-8 vaihe 1: globaalien ja staattisten muuttujien inventaario ja luokittelu, elinkaari ja vaiheen vaihdon nollaustaulukko, johdettujen rakenteiden eheys, säikeiden yhteinen tila, viat H-128–H-144 / Batch C-8 step 1: inventory and classification of globals and statics, lifecycle and stage-change reset table, derived-structure integrity, shared state between threads |
| [saikeet-ja-lukot.md](saikeet-ja-lukot.md) | Erä C-6 vaiheet 1–2: säiekartta, elinkaari ja lopetus, lukkokartta, lukitusjärjestys, Q30; vaihe 2: polkujen todennus eristetyillä koeohjelmilla, VCL-säieturvallisuus / Batch C-6 steps 1–2: thread map, lifecycle, lock map, lock order; step 2: probe verification, VCL thread safety |
| [testaus.md](testaus.md) | Erä C-11: testausstrategia ja ajonaikaisen todennuksen suunnitelma / Batch C-11: test strategy and runtime verification plan |
| [rekisteri/](rekisteri/README.md) | Auditointirekisteri: tiedostot, moduulit, havainnot / Audit register: files, modules, findings |
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

Selvitys tehtiin 2026-11-24 commitin `f7fe553` päälle.
The audit was made on top of commit `f7fe553` on 2026-11-24.
