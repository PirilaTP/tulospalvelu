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
| [yleiskuva.md](yleiskuva.md) | Arkkitehtuuri, moduulit, päävirrat, kaaviot / Architecture, modules, main flows, diagrams |
| [inventaario.md](inventaario.md) | Projektit, binäärit, hakemistot, koot, merkistöt / Projects, binaries, directories, sizes, encodings |
| [riippuvuudet.md](riippuvuudet.md) | Käännös- ja ajonaikaiset riippuvuudet, protokollat, tiedostot / Build and runtime dependencies, protocols, files |
| [tietorakenteet-ja-tila.md](tietorakenteet-ja-tila.md) | Keskeiset rakenteet, globaali tila, säikeet / Core structures, global state, threads |
| [merkistot.md](merkistot.md) | Merkistöt ja turvallinen muokkausmenettely / Encodings and the safe editing procedure |
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

## Jatkaminen myöhemmissä istunnoissa / Continuing in later sessions

Aloita tiedostosta `seuranta.md`: siinä on tehdyt vaiheet, avoimet kysymykset ja
seuraava askel. Inventaarion luvut on tuotettu skriptillä, joka käy läpi hakemiston
`TPsource/V52` (ks. `inventaario.md`, kohta "Miten tuotettu"). Jos koodi muuttuu
paljon, luvut kannattaa tuottaa uudelleen ennen kuin niihin luotetaan.

Start from `seuranta.md`: it lists completed steps, open questions and the next
step. Inventory figures were produced by a script that walks `TPsource/V52` (see
`inventaario.md`, "How it was produced"). If the code changes significantly,
regenerate the figures before relying on them.

Selvitys tehtiin 2026-11-24 commitin `f7fe553` päälle.
The audit was made on top of commit `f7fe553` on 2026-11-24.
