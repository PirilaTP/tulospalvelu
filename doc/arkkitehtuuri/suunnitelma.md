# Suunnitelma / Plan

Tämä on ehdotus jatkovaiheiksi. Mitään ei aloiteta ennen kuin ylläpitäjät ovat
hyväksyneet sen.
This is a proposal for the next phases. Nothing starts before the maintainers
have approved it.

## Moduulikohtainen arvio / Per-module assessment

Rivit ja kommenttiosuus: `inventaario.md`. "Todennus" kertoo, voidaanko
kommenttimuutoksen vaarattomuus todistaa objektivertailulla (`merkistot.md`).
Arviot ovat [päätelty], ellei toisin mainita.

Lines and comment share: `inventaario.md`. "Verification" tells whether the
harmlessness of a commenting change can be proved with the object comparison
(`merkistot.md`). Assessments are [inferred] unless stated otherwise.

| Moduuli / Module | Rivit / Lines | Komm. % | Monimutkaisuus / Complexity | Riskit / Risks | Syvyys / Depth | Todennus / Verification |
|---|--:|--:|---|---|---|---|
| `Hk/HkDef.h`, `Juk/VDef.h`, `*Declare.h` | ~5 000 | – | Keskeinen: rakenteet, globaalit / central: structures, globals | Levy- ja sanomamuoto riippuvat rakenteista / disk and message format depend on them | Jokainen luokka ja tärkeät kentät / every class and key fields | MSVC obj |
| `Hk/` | 46 032 | 4 | Korkea: tallennus, ajat, emit, tulosteet / high | `HkEmit.cpp` 6 400 riviä, `HkTulostus.cpp` 7 000+ / lines | Tiedosto-otsikot + julkiset ja monimutkaiset funktiot / file headers + public and complex functions | MSVC obj |
| `Juk/` | 38 605 | 4 | Korkea; rinnakkaisosuudet / high; parallel legs | `char`/`wchar_t`-sekoitus / mix | Kuten Hk / as Hk | MSVC obj |
| `Tp/` | 11 704 | 6 | Korkea: laitteet, tiedonsiirto, säikeet / high: devices, comms, threads | Käännetään kahdesti eri tyypeillä / compiled twice with different types | Säikeet ja protokollat tarkasti / threads and protocols in detail | MSVC obj (molemmat / both builds) |
| `com/` | 1 807 | 12 | Keskitaso / medium | Socket-säikeet / socket threads | Kaikki funktiot / all functions | MSVC obj |
| `tputilv2/`, `include/` | 17 396 | 22 | Matala, paljon pieniä tiedostoja / low, many small files | UTF-8 ilman BOMia / UTF-8 without BOM; `__LINE__` | Vain otsikot ja ei-ilmeiset / headers and non-obvious only | MSVC obj |
| `dbboxm/` | 2 375 | 6 | Keskitaso: tiedostomuoto / medium: file format | Levymuoto / disk format | Tiedostomuoto kokonaan / full file format | MSVC obj |
| `HkConsole/`, `JukConsole/` | 17 876 | 4 | Keskitaso; valikot / medium; menus | Vain konsoliversiossa / console only | Tiedosto-otsikot / file headers | MSVC obj |
| `cbHk/` (logiikkaa sisältävät / with logic) | ~15 000 | 8 | Korkea: `UnitEmit` 3 800, `UnitAjanotto`, `UnitArvonta`, `UnitKilpailijatiedot`, `HkArv2`, `UnitIlm2`, `UnitSprinthiihto` | Liiketoimintalogiikka tapahtumankäsittelijöissä / business logic in event handlers | Kuten ydin / as the core | Ei vielä / not yet |
| `cbHk/` (muut / others) | ~52 000 | 8 | Matala–keskitaso / low–medium | – | Tiedosto-otsikot / file headers | Ei vielä / not yet |
| `ViestiWin/` | 43 424 | 8 | Keskitaso; `UnitEmit` 3 100, `UnitImport` | Kuten cbHk / as cbHk | Kuten cbHk / as cbHk | Ei vielä / not yet |
| `cbTp/` | 15 115 | 12 | Keskitaso; kaksi käännöstä / medium; two builds | Riippuu molemmista ytimistä / depends on both cores | Tiedosto-otsikot + `UnitRadat`, `UnitEmitSQL` | Ei vielä / not yet |
| `Tests/` | 7 705 | – | doctest + 2 testitiedostoa / test files | `run.sh` rikki / broken (`issue-drafts/05`) | Ei kommentointia / no commenting | – |

Logiikkaa sisältävät GUI-lomakkeet tunnistettiin laskemalla ytimen kutsut
(`tallenna`, `laheta`, `PUTREC`, `GETREC`, `addtall`, `set_tulos`, `putem` jne.)
tiedostoittain. Eniten: `cbHk/UnitEmit.cpp` 59, `ViestiWin/UnitEmit.cpp` 27,
`cbHk/UnitAjanotto.cpp` 25, `cbHk/UnitSprinthiihto.cpp` 25, `cbHk/UnitArvonta.cpp` 21,
`cbHk/UnitKilpailijatiedot.cpp` 20. [todennettu: laskenta]

GUI forms with logic were identified by counting core calls per file (list above).
[verified: count]

## Riskit dokumentointityölle / Risks for the documentation effort

| Riski / Risk | Todennäköisyys / Likelihood | Vaikutus / Impact | Hallinta / Mitigation |
|---|---|---|---|
| Kommentointi muuttaa koodia (merkistö, rivinvaihto, vahinko) / commenting changes code | Keskitaso / medium (tapahtui keväällä 2026 / happened in spring 2026) | Korkea / high | Objektivertailu jokaiseen PR:ään; ASCII-kommentit / object comparison in every PR; ASCII comments |
| Väärät kommentit / wrong comments | Keskitaso / medium | Korkea / high | Vain todennettua; `Inferred:`-merkintä; ihmisen katselmointi / verified only; marker; human review |
| Ristiriidat avoimien haarojen kanssa / conflicts with open branches | Korkea / high | Keskitaso / medium | Pienet PR:t hakemisto kerrallaan; sovitaan ylläpitäjien kanssa / small PRs per directory; agree with maintainers |
| GUI-muutoksia ei voi todentaa objekteista / GUI changes cannot be verified via objects | Varma / certain | Keskitaso / medium | Kommentit poistava tekstivertailu (ks. alla) / comment-stripping text comparison |
| Dokumentit vanhenevat / documents go stale | Korkea / high | Keskitaso / medium | Viittaukset funktioihin rivinumeroiden lisäksi; inventaario skriptillä / reference functions, not only lines; scripted inventory |
| Työmäärä ylittyy / effort overruns | Keskitaso / medium | Matala / low | Syvyys moduulin mukaan; ensin arvokkaimmat / depth per module; most valuable first |

GUI-tiedostojen todennukseen ehdotetaan skriptiä, joka poistaa kommentit ja
vertaa jäljelle jäävää tekstiä tavu tavulta ennen ja jälkeen. Se ei todista
käännöstulosta, mutta osoittaa, ettei mikään kommenttien ulkopuolinen merkki
muuttunut. [ehdotus]

For verifying GUI files, a script is proposed that strips comments and compares
the remaining text byte by byte before and after. It does not prove the compiled
result, but shows that no character outside comments changed. [proposal]

## Vaiheet ja PR:t / Phases and PRs

Vikakorjaukset ja dokumentaatio pidetään erillään. Jokainen PR on pieni ja
yhden asian.

Bug fixes and documentation are kept separate. Every PR is small and about one
thing.

**Vaihe A – tämä selvitys / this audit** (dokumentit, ei koodia / docs, no code)
- PR: `doc/arkkitehtuuri/` + CLAUDE.md-korjaukset / CLAUDE.md fixes

**Vaihe B – erilliset vikakorjaukset / separate bug fixes** (`issue-drafts/01-05`)
- Yksi PR per issue; jokainen käännetään molemmilla työkaluketjuilla /
  one PR per issue; each built with both toolchains

**Vaihe C – syventävät dokumentit / deeper documents** (ei koodia / no code)
1. Tiedonsiirtoprotokolla: `combufrec`-paketit tavutasolla, kättely, uudelleenlähetys / comms protocol at byte level
2. Tiedostomuodot: `KILP.DAT`, `laskenta.cfg`, `KilpSrj.xml`, `radat.xml` / file formats
3. Ominaisuusvalitsimet: mitä kukin `#define` tekee / what each feature switch does
4. Säikeet: kuka käynnistää, mitä lukitsee, miten lopetetaan / threads: who starts, what they lock, how they stop

**Vaihe D – kommentointi / commenting** (yksi PR per rivi / one PR per row)

| PR | Sisältö / Content | Rivit luettava / Lines to read |
|---|---|---|
| D1 | `Hk/HkDef.h`, `Hk/HkDeclare.h`, `Hk/HkGlobals.cpp` | ~3 000 |
| D2 | `Juk/VDef.h`, `Juk/VDeclare.h`, `Juk/VGlobals.cpp` | ~2 800 |
| D3 | `Hk/HkDat.cpp`, `Hk/HkIx.cpp`, `Hk/HkCom32.cpp` | ~8 000 |
| D4 | `Tp/TpComY.cpp`, `com/` | ~5 000 |
| D5 | `Tp/TpLaitteet.cpp`, RFID-lukijat / readers | ~5 000 |
| D6 | `Hk/HkInit.cpp`, `Hk/HkAjat.cpp`, `Hk/HkSrj.cpp`, `Hk/HkTls.cpp` | ~8 000 |
| D7 | `Hk/HkEmit.cpp` | ~6 400 |
| D8 | `Hk/HkTulostus.cpp`, `Hk/HkMuotoilu.cpp` | ~10 000 |
| D9 | `Hk/HkXml*.cpp`, `Hk/HkIV.cpp`, muut / others | ~10 000 |
| D10–D14 | `Juk/` samassa järjestyksessä / in the same order | ~38 000 |
| D15 | `dbboxm/` | ~2 400 |
| D16 | `tputilv2/`, `include/` (otsikot / headers) | ~17 000 (kevyesti / lightly) |
| D17–D18 | `HkConsole/`, `JukConsole/` | ~18 000 |
| D19–D22 | Logiikkaa sisältävät GUI-lomakkeet / GUI forms with logic | ~15 000 |
| D23–D25 | Muut GUI-lomakkeet, tiedosto-otsikot / other forms, file headers | ~95 000 (kevyesti / lightly) |

Työmäärä: vaihe C noin 2–3 istuntoa, vaihe D noin 10–15 istuntoa. Arvio on
karkea. [päätelty]
Effort: phase C about 2–3 sessions, phase D about 10–15 sessions. The estimate is
rough. [inferred]

## Syväauditoinnin erät / Deep audit batches

Vaihe C toteutetaan erinä. Järjestys perustuu riskiin: mitä vakavampi seuraus
(tietojen katoaminen tai korruptio kilpailun aikana) ja mitä todennäköisempi
vika, sitä aiemmin. Hakemiston koko ei ratkaise järjestystä. Jokainen erä
päättyy riippumattomaan tarkastukseen (`independent-verifier`) ja rekisterin
päivitykseen (`rekisteri/`). Rinnakkain enintään kolme analyysiagenttia.

Phase C is done in batches. The order is based on risk: the more serious the
consequence (data loss or corruption during a competition) and the more likely
the defect, the earlier. Directory size does not decide the order. Every batch
ends with an independent review (`independent-verifier`) and a register update
(`rekisteri/`). At most three analysis agents run in parallel.

| Erä / Batch | Aihe / Topic | Riski / Risk | Agentit / Agents | Tuotos / Output |
|---|---|---|---|---|
| 1 | `tallenna()`-samanaikaisuus / concurrency | korkea / high | architecture-analyst, defect-concurrency-auditor ×2, independent-verifier | `tallenna-samanaikaisuus.md`, H-007–H-021 — **valmis / done 2026-11-24** |
| 2 | Työasemien tiedonsiirto: vastaanotto, lähetys, kuittaus, uudelleenlähetys, versioyhteensopivuus / workstation sync protocol | korkea / high | architecture-analyst, legacy-encoding-analyst (sanomamuodot / message layouts), defect-concurrency-auditor | `tiedonsiirto.md`; Q3 — **valmis / done 2026-11-24** (H-022–H-039, T-011–T-014) |
| 3 | Pysyvät tiedostot ja eheys: `KILP.DAT`, emit-tiedot, `ajat.txt`, `SQLjonoVarm.dat`, vanha muoto / persistent files and integrity | korkea / high | legacy-encoding-analyst, defect-concurrency-auditor, build-dependency-analyst | `tiedostomuodot.md`; Q4, Q17, Q18 — **valmis / done 2026-11-24** (H-040–H-054, T-015–T-019); Q5 (SQL-taulut / tables) siirtyy erään 10 / moves to batch 10 |
| 4 | Muistiturvallisuus ja määrittelemätön käytös: kääntäjävaroitusten läpikäynti molemmilla kääntäjillä, MSVC `/analyze`, puskurit ja muotoilumerkkijonot / memory safety and UB | korkea / high | defect-concurrency-auditor, build-dependency-analyst | **valmis ja suljettu / done and closed 2026-11-25** (`muistiturvallisuus.md`, H-055–H-077, T-020–T-022); H-numerot, issue-luonnokset / H entries, issue drafts |
| 5 | Kääntäjäkohtainen käytös: `#pragma -K -a1` (43 tiedostoa, vain bcc32) ja rakenteiden asettelu käännösyksiköiden välillä, `/J` (unsigned char), `wchar_t`, merkistöt / compiler-specific behaviour | korkea / high (jos asettelu eroaa / if layouts differ) | legacy-encoding-analyst, build-dependency-analyst | **vaihe 1 valmis / step 1 done 2026-11-25** (`kaantajaerot.md`, H-078–H-081, T-023–T-028; Q7 ratkaistu / resolved; `merkistot.md` korjattu / corrected); vaihe 2 valmis / step 2 done (`kaantajaerot.md` luku 13: Q8, H-082–H-086, T-029); ehdotus: erä suljetaan / proposal: close the batch. Alkuperäinen tuotos / original output: `merkistot.md` laajennus; Q7. Osittain tehty erässä C-3: levylle ja verkkoon kirjoitettavien rakenteiden koot ja `-K`/`-a1` mitattu (`tiedostomuodot.md`) / partly done in C-3 |
| 6 | Säikeiden omistajuus ja synkronointi kokonaisuutena: kaikki säikeet, lukot, lukitusjärjestys, lopetus / thread ownership and synchronisation | keskitaso–korkea / medium–high | defect-concurrency-auditor, architecture-analyst | `tietorakenteet-ja-tila.md` laajennus |
| 7 | GUI:n liiketoimintalogiikka: `cbHk/UnitEmit.cpp`, `UnitAjanotto`, `UnitArvonta`, `UnitKilpailijatiedot`, `ViestiWin/UnitEmit.cpp` / GUI business logic | keskitaso–korkea | architecture-analyst, defect-concurrency-auditor | moduulikuvaukset / module descriptions |
| 8 | Globaali muuttuva tila: kuka kirjoittaa mitäkin, alustus, nollaus kilpailun vaihdossa / global mutable state | keskitaso / medium | architecture-analyst | `tietorakenteet-ja-tila.md` |
| 9 | Virheenkäsittely ja resurssit: palautusarvojen tarkistus, tiedostokahvat, muistivuodot, `writeerror`-polut / error handling and resources | keskitaso / medium | defect-concurrency-auditor | H/T-numerot |
| 10 | Ulkoiset riippuvuudet: MySQL/dbExpress, WinHTTP, laiteprotokollat, BPL-ajonaikaiset paketit / external dependencies | matala–keskitaso | build-dependency-analyst | `riippuvuudet.md` |
| 11 | Käännös- ja testiautomaatio: testit CI:hin, `run.sh`, GUI-todennus kommenttimuutoksille / build and test automation | matala riski, suuri hyöty / low risk, high leverage | build-dependency-analyst, architecture-analyst, defect-concurrency-auditor, independent-verifier ×2 | `testaus.md` — **valmis / done 2026-11-24** (luku / section 20; 15 toistettu testikehyksessä, H-041 vahvistettu ajamalla / 15 reproduced, H-041 confirmed by running); CI-vienti avoin / CI integration open (Q35–Q42). GUI-kommenttien todennus (Q8) ei vielä / GUI comment verification not yet |

Erät 2–5 ovat riippumattomia toisistaan ja voidaan ajaa missä järjestyksessä
tahansa erän 1 jälkeen. Erä 11 on nopea ja hyödyttää kaikkia muita, joten sen
voi tehdä milloin tahansa. Arvio: 1 istunto per erä. [päätelty]

Batches 2–5 are independent and can run in any order after batch 1. Batch 11 is
quick and benefits all others, so it can be done at any time. Estimate: one
session per batch. [inferred]
