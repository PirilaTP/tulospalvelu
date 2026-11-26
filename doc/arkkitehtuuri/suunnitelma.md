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
| 5 | Kääntäjäkohtainen käytös: `#pragma -K -a1` (43 tiedostoa, vain bcc32) ja rakenteiden asettelu käännösyksiköiden välillä, `/J` (unsigned char), `wchar_t`, merkistöt / compiler-specific behaviour | korkea / high (jos asettelu eroaa / if layouts differ) | legacy-encoding-analyst, build-dependency-analyst | **vaihe 1 valmis / step 1 done 2026-11-25** (`kaantajaerot.md`, H-078–H-081, T-023–T-028; Q7 ratkaistu / resolved; `merkistot.md` korjattu / corrected); vaihe 2 valmis / step 2 done (`kaantajaerot.md` luku 13: Q8, H-082–H-086, T-029); **valmis ja suljettu / done and closed 2026-11-25** (`b17d536`, `55897eb`); avoinna / open Q60, Q63, Q64. Alkuperäinen tuotos / original output: `merkistot.md` laajennus; Q7. Osittain tehty erässä C-3: levylle ja verkkoon kirjoitettavien rakenteiden koot ja `-K`/`-a1` mitattu (`tiedostomuodot.md`) / partly done in C-3 |
| 6 | Säikeiden omistajuus ja synkronointi kokonaisuutena: kaikki säikeet, lukot, lukitusjärjestys, lopetus / thread ownership and synchronisation | keskitaso–korkea / medium–high | defect-concurrency-auditor, architecture-analyst | `tietorakenteet-ja-tila.md` laajennus — **vaihe 1 valmis / step 1 done 2026-11-25** (`saikeet-ja-lukot.md`, H-087–H-102, T-030–T-032, Q30 ratkaistu / resolved); **vaihe 2 valmis / step 2 done 2026-11-25** (luku 13: koeohjelmat, H-103–H-108, T-033, H-099 kumottu / refuted); **valmis ja suljettu / done and closed 2026-11-25** (`bf5b566`, `2767145`); avoimet jatkotehtävät `seuranta.md` / open follow-ups in `seuranta.md` |
| 7 | GUI:n liiketoimintalogiikka: `cbHk/UnitEmit.cpp`, `UnitAjanotto`, `UnitArvonta`, `UnitKilpailijatiedot`, `ViestiWin/UnitEmit.cpp` / GUI business logic | keskitaso–korkea | architecture-analyst, defect-concurrency-auditor | moduulikuvaukset / module descriptions — **vaihe 1 valmis ja suljettu / step 1 done and closed 2026-11-25** (`gui-liiketoimintalogiikka.md`, H-109–H-127, T-034–T-037; jatkotyöjono / follow-up queue J7-1–J7-7) |
| 8 | Globaali muuttuva tila: kuka kirjoittaa mitäkin, alustus, nollaus kilpailun vaihdossa / global mutable state | keskitaso / medium | architecture-analyst | `tietorakenteet-ja-tila.md` — **vaihe 1 valmis ja suljettu / step 1 done and closed 2026-11-26** (`globaali-tila.md`, H-128–H-144, T-038–T-039) |
| 9 | Virheenkäsittely ja resurssit: palautusarvojen tarkistus, tiedostokahvat, muistivuodot, `writeerror`-polut / error handling and resources | keskitaso / medium | defect-concurrency-auditor | H/T-numerot — **valmis ja suljettu / done and closed 2026-11-26** (`virheenkasittely.md`, H-145–H-158, T-040–T-042; vaiheet / steps `f1eb195`, `920930c`) |
| 10 | Ulkoiset riippuvuudet: MySQL/dbExpress, WinHTTP, laiteprotokollat, BPL-ajonaikaiset paketit / external dependencies | matala–keskitaso | build-dependency-analyst | `riippuvuudet.md` — **vaihe 1 rajattu, ei aloitettu / step 1 scoped, not started** (alla / below) |
| 11 | Käännös- ja testiautomaatio: testit CI:hin, `run.sh`, GUI-todennus kommenttimuutoksille / build and test automation | matala riski, suuri hyöty / low risk, high leverage | build-dependency-analyst, architecture-analyst, defect-concurrency-auditor, independent-verifier ×2 | `testaus.md` — **valmis / done 2026-11-24** (luku / section 20; 15 toistettu testikehyksessä, H-041 vahvistettu ajamalla / 15 reproduced, H-041 confirmed by running); CI-vienti avoin / CI integration open (Q35–Q42). GUI-kommenttien todennus (Q8) ei vielä / GUI comment verification not yet |

Erät 2–5 ovat riippumattomia toisistaan ja voidaan ajaa missä järjestyksessä
tahansa erän 1 jälkeen. Erä 11 on nopea ja hyödyttää kaikkia muita, joten sen
voi tehdä milloin tahansa. Arvio: 1 istunto per erä. [päätelty]

Batches 2–5 are independent and can run in any order after batch 1. Batch 11 is
quick and benefits all others, so it can be done at any time. Estimate: one
session per batch. [inferred]

Jatkotehtävä C-5:stä (ei numeroitu erä): syötteiden kokonaislukujäsennyksen läpikäynti,
161 ehdokasta (`seuranta.md`, "Avoimet jatkotehtävät C-5:stä"); voidaan yhdistää erään 7.
Follow-up from C-5 (not a numbered batch): integer-parse review, 161 candidates; may be
merged with batch 7.

### C-10 vaihe 1: rajaus / Step 1: scope (**valmisteltu, ei aloitettu / prepared, not started**)

Suunnitelman erä 10 (taulukko yllä, ennallaan): ulkoiset riippuvuudet – MySQL/dbExpress, WinHTTP,
laiteprotokollat, BPL-ajonaikaiset paketit; riski matala–keskitaso; agentti
`build-dependency-analyst`; tuotos `riippuvuudet.md`. Erään 10 on siirretty Q5 (SQL-taulujen
rakenne ja milloin SQL on käytössä, C-3). Rajaus pitää tavoitteen ja tuotoksen ennallaan ja
painottaa sitä, mitä riippuvuuden puuttuminen, väärä versio tai vika tekee kilpailun aikana.

**Lähtötiedot, ei analysoida uudelleen / inputs, not re-analysed:** `riippuvuudet.md`
(hakemistoriippuvuudet, kirjastot ja linkitys, kolmannen osapuolen koodi, ajonaikaiset
tiedostot, kehitysympäristö; T-005 Indy ja ODBC ilman käyttöä); C-2 (työasemien protokolla,
H-022–H-039; ei laiteprotokollia); C-3 (`SQLjonoVarm.dat`, H-043); C-4 (laitesanomien jäsennyksen
muistiturvallisuus H-055, H-056, H-060, H-070; P1/P2-jono); C-5 (kääntäjät, RTL-erot, T-023–T-029);
C-6 (MySQL-jonon lukitus H-100, lukijasäikeet H-090, H-107); C-1 (H-014 MySQL-kirjoitus); C-9
(H-145 lukijan kuittaus, H-157 `vahaku`/`FormatMessage`, bcc32-RTL-koe Q83–Q85); C-11
(testikehys, CI-kysymykset Q35–Q42). Niihin viitataan, niitä ei toisteta.

**C-10:n aihepiiriin kuuluvat avoimet kysymykset / open questions in scope:** Q5 (SQL),
Q86 (laitteen kuittauksen tulkinta – staattisesti protokollan osalta), Q91 (ViestiWinin
muistinhallinta ajonaikaisten pakettien kanssa – staattisesti: mikä muistinhallinta linkittyy),
Q92 (bcc32-RTL:n `_wfopen_s`-jakotila – RTL-lähteistä, jos ne ovat asennuksessa), T-005.
Q2 (ominaisuusvalitsimet) vain siltä osin kuin valitsin kytkee ulkoisen riippuvuuden päälle
(esim. `EMITHTTP`, `TCPSIIRTO`, `EMITAG`, `TCPLUKIJA`); koko Q2 ei kuulu erään 10.

| Osa / Part | Sisältö / Content | Agentti, malli / Agent, model |
|---|---|---|
| A | Inventaario tuotteittain (HkMaali, ViestiMaali, ViestiLuenta, HkKisaWin, ViestiWin): linkitettävät kirjastot (`.vcxproj`, `.cbproj` `PackageImports`/`#pragma link`), ajonaikaiset paketit ja DLL:t (`collect-gui-runtime.ps1`, `release.yml`), dynaamiset lataukset (`LoadLibrary`), VCL-komponentit `.dfm`-tiedostoissa (`TSQLConnection`, `TEdgeBrowser`/`TWebBrowser` ym.), Windows-API-alueet, ulkoiset tiedostot ja rekisteri, verkkopäätepisteet (portit, URL:t), laitetyypit `LID_*`; versiot ja niiden kiinnityskohdat; lisenssit; mitkä ominaisuusvalitsimet kytkevät riippuvuuden; raakatulos `todisteet/c10/` / per-product inventory | skripti + `build-dependency-analyst`, Sonnet |
| B | Tietokanta: dbExpress/MySQL-ajurit ja asiakaskirjasto, `dbxkilp.ini`, Q5 (taulut, kentät, milloin SQL on käytössä), yhteyden avaus ja katkos, tunnistetietojen säilytys, ajurin tai palvelimen puuttumisen seuraus kilpailun aikana; H-014, H-043, H-100 vain viitteinä / database dependency | `architecture-analyst`, Sonnet |
| C | HTTP ja selain: `com/gethttp.cpp` (WinHTTP) ja sen käyttäjät (`EMITHTTP`-pollaus, muut haut), aikakatkaisut, TLS ja varmenteiden tarkistus, välityspalvelin, virheiden käsittely; `TEdgeBrowser`/WebView2-ajonaikaisympäristö ja `TWebBrowser`; mitä puuttuva ajonaikaisympäristö tai palvelin tekee / HTTP and browser | `defect-concurrency-auditor`, Sonnet |
| D | Laiteprotokollat: jokaisesta `LID_*`-laitteesta kuljetus (sarja, TCP, UDP, HTTP), kehystys, kuittaus ja uudelleenlähetys, aikakatkaisut, laiteohjelmistoon tai protokollaversioon sidotut oletukset ja mitä tapahtuu, kun laite katoaa tai lähettää odottamatonta; Q86 staattisesti; ei jäsennyksen muistiturvallisuutta (C-4) eikä säikeitä (C-6) / device protocols | `architecture-analyst`, Sonnet; `legacy-encoding-analyst`, Sonnet sanomarakenteiden tavutasolle tarvittaessa / for byte layouts if needed |
| E | Ajonaikaiset paketit ja jakelu: GUI-tuotteiden BPL/DLL-joukko ja sen täydellisyys julkaisu-ZIP:ssä, C++Builder 13- ja Windows SDK -versioriippuvuudet, `DynamicRTL`/`UsePackages`, muistinhallinta (Q91 staattisesti), RTL-erot (Q92), MSVC:n staattinen CRT (`/MT`), käyttämättömät riippuvuudet (T-005), CI-ympäristön riippuvuudet (`.github/workflows/`) / runtime packages and deployment | `build-dependency-analyst`, Sonnet |
| F | Riippumaton tarkastus uusille keskitason ja vakavammille väitteille sekä olemassa olevia havaintoja muuttaville väitteille / independent review | `independent-verifier`, Opus |
| G | Dokumentointi: `riippuvuudet.md` laajennetaan (kaksikielinen luku "Ulkoiset riippuvuudet" ja tuotekohtainen taulukko), rekisterit, `seuranta.md`, `CHECKPOINT.md`; raakatulokset `todisteet/c10/`; validaattori; paikallinen commit, ei pushia / documentation | pääistunto / main session |

A ensin (B–E käyttävät sen luetteloa); B, C ja D rinnakkain (enintään kolme analyysiagenttia);
E A:n jälkeen joko rinnakkain B–D:n kanssa, jos kolmen agentin raja sallii, tai niiden jälkeen; F
lopuksi. Ei keinotekoista tokenbudjettia; päällekkäisyys estetään lähtötietoluettelolla ja jakamalla
kohteet (B tietokanta, C HTTP ja selain, D laitteet, E paketit ja jakelu). Rajattu pois:
jäsennyksen muistiturvallisuus (C-4), työasemien välinen protokolla (C-2), lukot ja säikeet (C-6),
korjaukset, kommentointi, koko Q2. Ajonaikaiset kokeet vain erillisellä päätöksellä ja vain
eristettyinä (ei MySQL-palvelinta, ei verkkoa, ei laitteita, ei toimitettuja ohjelmia); vaiheessa 1
ei ajeta mitään. Valmis, kun jokaisen tuotteen ulkoiset riippuvuudet on luetteloitu versioineen ja
kiinnityskohtineen, jokaisen riippuvuuden puuttumisen tai vian seuraus kilpailun aikana on kuvattu
(tai syy puuttumiseen kirjattu), Q5 on ratkaistu tai rajattu, uudet merkittävät havainnot on
tarkastettu riippumattomasti ja validaattori menee läpi. Vaihe 2 erillisellä päätöksellä. [ehdotus]

Batch 10, step 1: (A) scripted per-product inventory of external dependencies (libraries,
runtime packages and DLLs, dynamic loads, VCL components in forms, Windows API areas, external
files and registry, network endpoints, device types; versions and where they are pinned;
licences; which feature switches enable them); (B) database: dbExpress/MySQL drivers, Q5, connect
and loss, credentials; (C) HTTP (WinHTTP) and browser runtime; (D) device protocols per `LID_*`:
transport, framing, acknowledgement, timeouts, loss of device; Q86 statically; (E) runtime
packages and deployment: BPL/DLL set, C++Builder and SDK version coupling, memory manager (Q91
statically), RTL differences (Q92), static MSVC CRT, unused dependencies (T-005), CI; (F) Opus
review; (G) extend `riippuvuudet.md`, registers, evidence `todisteet/c10/`, local commit. Sonnet
for inventory and analysis, Opus for review; no token budget; earlier findings are inputs. Out of
scope: parser memory safety, workstation protocol, threads, fixes, commenting, the whole of Q2;
no runs in step 1. [proposal]

### C-9 vaihe 2 / Step 2 (**valmis / done 2026-11-26**, `virheenkasittely.md` luku 12)

Käyttäjän hyväksymä toteutus: H-149:n todentaminen (omistajuus, vapautukset, saavutettavuus,
Release-käytös, Q85), H-145–H-147 virheinjektiolla, H-150:n vuodon mittaus, H-151:n ja H-152:n
testattavuus, inventaarion v1/v2-erotus, palautumisen kokonaiskuva, Opus-tarkastus. Eristetyt
koeohjelmat C-6/C-8-mallilla (`todisteet/c9/vaihe2/koeohjelmat/`); ei tuotantokoodin,
projektiasetusten eikä CI:n muutoksia, ei toimitettuja ohjelmia, ei verkkoa. / Done on approval:
isolated probes, no production, project or CI changes.

### C-9 vaihe 1: rajaus / Step 1: scope (**valmis / done 2026-11-26**, `virheenkasittely.md`)

Toteutus: rajauksen mukaan; osan A skripti korjattiin agentin otoksen perusteella (v2), osan B
monivaiheinen päättely tehtiin Sonnetilla ja Opus vain osassa E; tarkastajan löytämä H-149
todennettiin pääistunnossa. Agentit noin 1,22 milj. tokenia. / Done per the scope; script fixed
(v2) after the agent's sample; Opus only in part E; about 1.22 M agent tokens.

Suunnitelman erä 9: virheenkäsittely ja resurssit – palautusarvojen tarkistus, tiedostokahvat,
muistivuodot, `writeerror`-polut; riski keskitaso; agentti `defect-concurrency-auditor`; tuotos
H- ja T-numerot. Rajaus pitää tavoitteen ja painottaa kilpailun aikaista riskiä: kirjoitus-
virhe, joka hukkaa tuloksen hiljaa, on vakavampi kuin käynnistyksessä kerran vuotava muisti.

**Lähtötiedot [todennettu: raaka grep, 2026-11-26; `Hk/`, `Juk/`, `Tp/`, `com/`, `cbTp/`,
`tputilv2/`, `dbboxm/`, `cbHk/`, `ViestiWin/`, `HkConsole/`, `JukConsole/`]:** `fopen`/`_wfopen`
108, `new TextFl(` 196, `_open`/`open` 18, `openfile` 14, `CreateFile` 6, `fclose`/`_close`/`close`
129; `malloc`/`calloc`/`realloc` 177, `free` 186, `new` 835, `delete` 558; `fwrite`/`_write`/`write`
71; `writeerror_w(` 396; `catch`/`__finally` 22. Luvut ovat raakoja osumia.

**Lähtötiedot, ei analysoida uudelleen / inputs, not re-analysed:** C-1 (`tallenna`, H-007–H-021);
C-2 (yhteysjonot ja lähetysvirheet, H-022–H-039, H-027); C-3 (tiedostomuodot, `_read`-tarkistukset,
H-040–H-054, H-054 AJAT-jonon `_close`); C-4 (muistiturvallisuus, H-055–H-077; T-020
varauksen tulosta ei tarkisteta, `fclose(NULL)` H-063); C-5 (H-078, H-079 `TextFl`-tilat; T-026);
C-6 (lukot ja säikeet, H-087–H-108; T-030 lopetus, T-032 `writeerror_w` lukon alla, H-009, H-019);
C-7 (lomakelogiikka H-109–H-127; T-036 VCL-poikkeus lukon alla); C-8 (H-133 `aloitus`-uusinnan
vuodot, `lopetus1` ei vapauta, H-105). Niihin viitataan, niitä ei toisteta.

| Osa / Part | Sisältö / Content | Agentti, malli / Agent, model |
|---|---|---|
| A | Inventaario: skripti (`tyokalut/`, raakatulos `todisteet/c9/`) luettelee resurssin hankinta- ja vapautuskohdat (tiedostot, `TextFl`, `openfile`, kahvat, pistokkeet, muistivaraukset) ja jokaisen kirjoitus- ja avauskutsun palautusarvon käytön (tarkistettu / sijoitettu tarkistamatta / ohitettu) sekä `writeerror_w`-kutsun jälkeisen toiminnan (paluu, jatko, `exit`); agentti luokittelee ja valitsee kilpailun aikana toistuvat polut osille B–D / scripted inventory of acquire/release sites, return-value use and what follows `writeerror_w`; classification | skripti + `architecture-analyst`, Sonnet |
| B | Pysyvän datan virhepolut: `KILP.DAT` (`dbboxm` `write`/`PUTREC`/`addrec`), emit-tiedosto, AJAT-jono, `COMFILE`-levyjono, SQL-varmistusjono, lokit ja tulosteet levylle: havaitaanko epäonnistunut kirjoitus (levy täynnä, verkkolevy katkeaa, tiedosto lukittu), ilmoitetaanko, yritetäänkö uudelleen, voiko tulos kadota hiljaa tai jäädä muistiin eri arvoon kuin levylle / persistent-data error paths: silent loss, memory vs disk divergence | `defect-concurrency-auditor`, Sonnet; Opus monivaiheiseen tietojen katoamisen päättelyyn / Opus for multi-step data-loss reasoning |
| C | Resurssien elinkaari kilpailun aikana: kahvat ja muisti, jotka vuotavat virhepolulla tai toistuvassa toiminnossa (tuloste, vienti, kortin luku, vaiheen vaihto, uudelleenyhteys), kasvu tapahtumaa kohden ja raja (kahvat, GDI, muisti); käynnistyksen kertaluonteiset vuodot vain mainintana / handle and memory leaks per repeated event | `defect-concurrency-auditor`, Sonnet |
| D | Virheilmoituksen jälkeinen tila: jatkaako ohjelma `writeerror_w`:n, epäonnistuneen avauksen tai varauksen jälkeen epäjohdonmukaisessa tilassa (puolikas päivitys, NULL-osoitin, jatkuva virhesilmukka), `exit`/`abort`-polut kesken kilpailun ja tallentamaton tieto niissä; ei lukko- eikä VCL-poikkeusnäkökulmaa (C-6, T-036) / state after an error report; exit paths during a competition | `defect-concurrency-auditor`, Sonnet |
| E | Riippumaton tarkastus uusille korkeille ja keskitason väitteille ja olemassa olevia havaintoja muuttaville väitteille / independent review | `independent-verifier`, Opus |
| F | Dokumentointi: uusi `virheenkasittely.md` (kaksikielinen), rekisterit, `seuranta.md`, `CHECKPOINT.md`; raakatulokset `todisteet/c9/`; validaattori; paikallinen commit, ei pushia / documentation, local commit | pääistunto / main session |

A ensin (B, C ja D käyttävät sen luetteloa); B, C ja D rinnakkain; E niiden jälkeen. Ei
keinotekoista tokenbudjettia; päällekkäisyys estetään lähtötietoluettelolla ja jakamalla kohteet
osien välillä (B: pysyvä data, C: elinkaari ja vuodot, D: tila virheen jälkeen). Rajattu pois:
muistiturvallisuus ja puskurit (C-4), lukot ja säikeiden lopetus (C-6), yhteysprotokollan
virheet (C-2, paitsi resurssivuodot), lomakkeiden syötteet (C-7), ulkoiset kirjastot (C-10),
korjaukset. Ajonaikainen todennus vain eristetyllä koeohjelmalla (C-6/C-8-malli, esim.
levyvirheen simulointi tiedostokahvaa sulkemalla) ja vain erillisellä päätöksellä. Valmis, kun
osan A luettelo kattaa kohdehakemistot (tai syy puuttumiseen on kirjattu), jokaisen pysyvän
tiedoston kirjoitusvirheen käsittely on kuvattu, toistuvien toimintojen vuodot on käyty läpi,
uudet merkittävät havainnot on tarkastettu riippumattomasti ja validaattori menee läpi. Vaihe 2
erillisellä päätöksellä. [ehdotus]

Batch 9, step 1: (A) scripted inventory of resource acquire/release sites, return-value use and
what follows `writeerror_w`, classified by an agent; (B) error paths of persistent data (silent
loss, memory/disk divergence); (C) handle and memory leaks per repeated competition event; (D)
program state after an error report and exit paths; (E) Opus review of new high/medium claims;
(F) new `virheenkasittely.md`, registers, evidence `todisteet/c9/`, local commit. Sonnet for
inventory and analysis, Opus for review and multi-step data-loss reasoning; no token budget;
earlier findings are inputs. Out of scope: memory safety, locks, protocol errors, form input,
external libraries, fixes; isolated probes only on a separate decision. [proposal]

### C-8 vaihe 1: rajaus / Step 1: scope (**valmis ja suljettu / done and closed 2026-11-26**, `globaali-tila.md`)

Toteutus: rajauksen mukaan; osan A skriptiin lisättiin funktiokohdistuksen varamenetelmä
(sarake 0), ja osien B ja D agentit tallensivat raporttinsa itse, C:n ja E:n pääistunto.
Kulutus noin 1,33 milj. tokenia. / Done per the scope; about 1.33 M tokens.

Suunnitelman erä 8: globaali muuttuva tila – kuka kirjoittaa mitäkin, alustus, nollaus
kilpailun vaihdossa; riski keskitaso; agentti `architecture-analyst`; tuotos
`tietorakenteet-ja-tila.md`. Rajaus pitää tavoitteen ja lisää kaksi C-1–C-7:n perusteella
tarpeellista näkökulmaa: johdettujen tietorakenteiden eheys (vaikuttaa tulosten
oikeellisuuteen) ja säikeiden yhteinen tila niiltä osin kuin C-6 ei sitä kattanut.

**Lähtötiedot [todennettu: laskenta, 2026-11-25]:** määrittelyt `Hk/HkGlobals.cpp` (675 riviä,
noin 490 lausetta) ja `Juk/VGlobals.cpp` (554, noin 430); `extern`-esittelyjä `Hk/HkDeclare.h`
453 ja `Juk/VDeclare.h` 396 (`tietorakenteet-ja-tila.md`); tiedostotason `static`-määrittelyjä
(sarake 0) `Hk/` 102, `Juk/` 74, `Tp/` 21, `com/` 12, `cbTp/` 49, `tputilv2/` 27, `cbHk/` 131,
`ViestiWin/` 81; funktion sisäisiä `static`-muuttujia `Hk/` 108, `Juk/` 88, `Tp/` 90, `tputilv2/`
33, `cbHk/` 46, `ViestiWin/` 40, muut ≤ 10. Luvut ovat raakoja grep-osumia.

**Lähtötiedot, ei analysoida uudelleen / inputs, not re-analysed:** C-1 (`tallenna`, H-007–H-021);
C-2 (yhteysjonot, H-022–H-039); C-3 (tiedostot ja `datf2`, H-040–H-054, H-047); C-5
(rakenteiden asettelu); C-6 (säie- ja lukkokartta, vaiheen vaihto Q30, H-087–H-108, erityisesti
H-094, H-097, H-102, T-030, T-033; avoin Q67); C-7 (H-109:n laskurit, H-114, T-035).
Niihin viitataan, niitä ei toisteta.

| Osa / Part | Sisältö / Content | Agentti, malli / Agent, model |
|---|---|---|
| A | Inventaario: koneellinen luettelo ytimen (`Hk/`, `Juk/`, `Tp/`, `com/`, `cbTp/`, `tputilv2/`) globaaleista ja staattisista muuttujista; kirjoittajat ja lukijat funktioittain (skripti, raakatulos `todisteet/c8/`); agentti luokittelee: asetus (kirjoitetaan alustuksessa), kilpailudata (`kilparr`, `Sarjat`, indeksit), johdettu tila (laskurit, järjestysketjut, hakemistot), ajonaikaiset liput ja tilat, yhteyspuskurit, lomakkeiden yhteiset muuttujat / inventory and classification | skripti + `architecture-analyst`, Sonnet |
| B | Elinkaari: alustusjärjestys (`aloitus`, `luesarjat`, `teejarj`, `initajat`), HK:n vaiheen vaihto (`vaihdavaihe`: mitkä globaalit nollataan tai lasketaan uudelleen ja mitkä jäävät edellisen vaiheen arvoon, esim. `k_pv`:stä riippuvat taulukot ja välimuistit), kilpailun uudelleenavaus tuonnin kautta (H-047) ja kahdesti ajettu `aloitus` (Q67), lopetus vain H-038:n ulkopuolisilta osin / lifecycle | `architecture-analyst`, Sonnet |
| C | Johdettujen rakenteiden eheys: `nilm`/`nyilm`-laskurit, järjestysketjut (`jalku`, `remjarjarr`), `sarjaindex`, `idindex`/`getpos`, `bdg_kno`/`EmitJarr`, `pkarki`, `ntulos`: invariantit, ylläpitäjät ja polut, jotka voivat jättää ne ristiriitaan `kilparr`in kanssa (esim. H-109:n kaksoisvähennys), ja miten ristiriita näkyy tuloksissa / derived-structure integrity | `defect-concurrency-auditor`, Sonnet; Opus monivaiheiseen tulosvaikutuksen päättelyyn / Opus for multi-step result-impact reasoning |
| D | Jaettu muuttuva tila säikeiden välillä: osan A muuttujat, joita kirjoittaa yksi säie ja lukee toinen (C-6:n säiekartta), ilman C-6:n lukkokartan suojaa; vain uudet tapaukset, ei C-6:n havaintoja; liput ilman `volatile`a tai muistiestettä vain, jos niillä on havaittava seuraus / shared state not covered by C-6 | `defect-concurrency-auditor`, Sonnet |
| E | Riippumaton tarkastus uusille korkeille ja keskitason väitteille ja olemassa olevia havaintoja muuttaville väitteille / independent review | `independent-verifier`, Opus |
| F | Dokumentointi: uusi `globaali-tila.md` (kaksikielinen) ja tiivistelmä `tietorakenteet-ja-tila.md`:n lukuun "Globaali tila"; rekisterit; `seuranta.md`, `CHECKPOINT.md`; raakatulokset `todisteet/c8/`; validaattori; paikallinen commit, ei pushia / documentation, local commit | pääistunto / main session |

A ensin (B, C ja D käyttävät sen luetteloa); B, C ja D rinnakkain; E niiden jälkeen. Ei
keinotekoista tokenbudjettia; päällekkäisyys estetään lähtötietoluettelolla ja jakamalla
muuttujat osien välillä (B: alustus ja nollaus, C: johdetut rakenteet, D: säikeiden yhteiset).
Rajattu pois: lomakeyksiköiden sisäinen tila (C-7, J7-2), lukitusjärjestys ja säikeiden
elinkaari (C-6), tiedostomuodot (C-3), kääntäjäerot (C-5), konsoliohjelmien käyttöliittymätila,
korjaukset, ajonaikaiset kokeet (puhtaan laskentafunktion eristetty koeohjelma vain erillisellä
päätöksellä). Valmis, kun osan A luettelo kattaa ytimen globaalit ja staattiset (tai syy
puuttumiseen on kirjattu), vaiheen vaihdon nollaustaulukko on tehty, johdettujen rakenteiden
invariantit ja ylläpitäjät on listattu, uudet merkittävät havainnot on tarkastettu
riippumattomasti ja validaattori menee läpi. Vaihe 2 erillisellä päätöksellä. [ehdotus]

Batch 8, step 1: (A) scripted inventory of core globals and statics with writers and readers,
classified by an agent; (B) lifecycle: initialisation order, which globals the HK stage change
resets or leaves stale, reopen via import, `aloitus` twice; (C) integrity of derived structures
(counters, rank chains, indexes, badge maps) and how inconsistency reaches results; (D) shared
state between threads not covered by C-6; (E) Opus review of new high/medium claims; (F) new
`globaali-tila.md`, summary in `tietorakenteet-ja-tila.md`, registers, evidence `todisteet/c8/`,
local commit. Sonnet for inventory and analysis, Opus for review and multi-step result-impact
reasoning; no token budget; earlier findings are inputs. Out of scope: form-internal state,
lock order and thread lifecycle (C-6), file formats, compiler differences, fixes, run-time
probes. [proposal]

### C-7 vaihe 1: rajaus / Step 1: scope (**valmis / done 2026-11-25**, `gui-liiketoimintalogiikka.md`)

Toteutus poikkesi rajauksesta: ensimmäinen B-ajo jätti kilpailija- ja joukkuelomakkeen sekä
`Paivita`-funktiot osittain lukematta, joten tehtiin täydentävä B-ajo ja toinen riippumaton
tarkastuskierros (Opus); kulutus noin 1,0 milj. tokenia. / Deviation: a supplementary B pass
and a second Opus review round; about 1.0 M tokens.

Suunnitelman erä 7: GUI:n liiketoimintalogiikka. Alkuperäinen tavoite (taulukko yllä): lomake-
yksiköiden `cbHk/UnitEmit.cpp`, `UnitAjanotto`, `UnitArvonta`, `UnitKilpailijatiedot` ja
`ViestiWin/UnitEmit.cpp` liiketoimintalogiikka; riski keskitaso–korkea; agentit
`architecture-analyst` ja `defect-concurrency-auditor`; tuotos moduulikuvaukset. Rajaus pitää
tavoitteen ennallaan; viestin vastinyksiköt otetaan mukaan vertailuun, koska ne jakavat saman
logiikan.

**Kohde / target (8 tiedostoa, noin 16 300 riviä, kaikki nyt tasolla 2):** `cbHk/UnitEmit.cpp`
(3 798), `cbHk/UnitAjanotto.cpp` (1 961), `cbHk/UnitArvonta.cpp` (1 270),
`cbHk/UnitKilpailijatiedot.cpp` (1 900), `ViestiWin/UnitEmit.cpp` (3 091),
`ViestiWin/UnitAjanotto.cpp` (1 754), `ViestiWin/UnitArvonta.cpp` (611) ja viestin
kilpailijatietolomakkeen vastine (agentti tunnistaa; `UnitKilpailijatiedot` puuttuu
viestistä). Otsakkeet (`.h`) vain tarpeen mukaan.

**Lähtötiedot, ei analysoida uudelleen / inputs, not re-analysed:** näissä tiedostoissa jo
kirjatut H-010 (`tallenna` ilman lukkoa, C-1), H-017, H-069, H-080, H-085, H-089, H-092, H-107;
C-6:n säie-, lukko- ja VCL-havainnot (`saikeet-ja-lukot.md`); C-5:n kääntäjäerot
(`kaantajaerot.md`). Niihin viitataan, niitä ei toisteta.

| Osa / Part | Sisältö / Content | Agentti, malli / Agent, model |
|---|---|---|
| A | Moduulikuvaus jokaisesta kohdeyksiköstä: vastuut, tärkeimmät tapahtumankäsittelijät, mitä kilpailutietoja ne lukevat ja muuttavat (`kilparr`, `tallenna`, emit-tietueet, aikajonot, arvonta), kutsut ytimeen, HK- ja viestiversioiden erot / module description per unit | `architecture-analyst`, Sonnet |
| B | Liiketoimintalogiikan virheet: syötteiden tarkistus ja rajat (käyttäjän syöttämät numerot, ajat, rivi- ja sarjaindeksit), emit-kortin kohdistus kilpailijaan ja korjaus, käsiajanoton tallennus ja korjaus, arvonnan oikeellisuus (kaksoisnumerot, rajat, satunnaisuus, uudelleenarvonta), tilan johdonmukaisuus peruutuksissa ja keskeytyksissä; HK ja viesti rinnakkain / logic defects | `defect-concurrency-auditor`, Sonnet; Opus vain monivaiheiseen korkean riskin päättelyyn / Opus only for multi-step high-risk reasoning |
| C | C-5:n jäsennysehdokkaista ne 9, jotka ovat kohdetiedostoissa (`cbHk/UnitEmit.cpp:2819`, `:2820`, `:2974`; `cbHk/UnitArvonta.cpp:916`; `cbHk/UnitKilpailijatiedot.cpp:757`; `ViestiWin/UnitEmit.cpp:2356`, `:2357`, `:2504`; `ViestiWin/UnitAjanotto.cpp:633`), osana B:tä. Muut 152 ehdokasta jäävät erilliseksi jatkotehtäväksi (ei laajenneta erää) / the 9 C-5 parse candidates in the target files, as part of B | B:n agentti / B's agent |
| D | Riippumaton tarkastus kaikille uusille korkean ja keskitason havainnoille sekä väitteille, jotka muuttavat olemassa olevan havainnon arviota; tarkastaja yrittää kumota (saavutettavuus GUI:ssa, käyttäjän toiminto, asetukset, ytimen tarkistukset kutsujassa) / independent review | `independent-verifier`, Opus |
| E | Dokumentointi: uusi `gui-liiketoimintalogiikka.md` (kaksikielinen), rekisterit (`havainnot.md`, `tiedostot.csv`, `moduulit.md`), `seuranta.md`, `CHECKPOINT.md`; raakatulokset `todisteet/c7/`; validaattori; paikallinen commit, ei pushia / documentation, local commit | pääistunto / main session |

A ja B voivat edetä rinnakkain (B käyttää A:n tuloksia vain lopussa); D A:n ja B:n jälkeen.
Ei keinotekoista tokenbudjettia (käyttäjän linja C-6:sta); päällekkäisyys estetään antamalla
agenteille lähtötietojen luettelo. Rajattu pois: säikeet, lukot ja VCL-säieturvallisuus
(C-6), kääntäjäerot (C-5), muut lomakeyksiköt, ajonaikaiset kokeet (GUI-koodia ei voi
linkittää konsolikoeohjelmaan; mahdolliset puhtaat laskentafunktiot ytimessä voidaan
todentaa eristetyllä koeohjelmalla vain erillisellä päätöksellä), korjaukset. Valmis, kun
jokaisesta kohdeyksiköstä on moduulikuvaus, B:n kysymykset on käyty läpi kaikissa
yksiköissä, 9 jäsennysehdokasta on ratkaistu, uudet merkittävät havainnot on tarkastettu
riippumattomasti ja validaattori menee läpi. Vaihe 2 (esim. muut lomakeyksiköt tai jäljelle
jäävät 152 jäsennysehdokasta) erillisellä päätöksellä. [ehdotus]

Batch 7 (GUI business logic), step 1: module descriptions (A) and logic defects (B) for the
five planned units plus their relay counterparts; the 9 C-5 parse candidates in these files
(C); Opus review of new high/medium findings (D); new document `gui-liiketoimintalogiikka.md`,
register updates, evidence `todisteet/c7/`, local commit (E). No token budget; earlier findings
are inputs. Out of scope: threads/locks/VCL (C-6), compiler differences (C-5), other forms,
run-time probes, fixes; the other 152 parse candidates stay a separate follow-up. [proposal]

### C-6 vaihe 1: rajaus / Step 1: scope (**valmis / done 2026-11-25**, `saikeet-ja-lukot.md`)

Toteutus poikkesi rajauksesta: käyttäjän ohjeella ei tokenbudjettia (kulutus noin 950 k), ja
osan E tarkastus tehtiin Opuksella. Lähtöluvut korjattu: 56 aktiivista käynnistyskohtaa,
22 lukkoa (`saikeet-ja-lukot.md` luvut 3.3, 6). / Deviations: no token budget by the user's
instruction (about 950 k used), part E reviewed with Opus; starting figures corrected.

Suunnitelman erän 6 ensimmäinen, rajattu osa: kaikki säikeet ja lukot luetteloituina,
lukitusjärjestysehdokkaat ja lopetuspolut. Aiempia eriä ei analysoida uudelleen: C-1
(`tall_CriticalSection` ja `tallenna()`, H-007–H-021), C-2 (tiedonsiirtosäikeet, H-022–H-039, erityisesti H-027 ja H-028)
ja C-3 (H-040, H-045, H-047, H-051) otetaan lähtötiedoiksi viittauksin.

Lähtötiedot (raakaosumat `TPsource/V52`, sis. kommentit, 2026-11-25) [todennettu: laskenta]:
`_beginthread` 76 osumaa 35 tiedostossa, `CreateThread` 2/2, `TThread` 0;
`EnterCriticalSection` 487/95, 24 nimettyä lukkoa, joista `tall_CriticalSection` 272 (C-1),
`outb` 48, `print` 33, `emit` 18, `seura` 15, `kilpdat` 12, muut ≤ 11;
`InitializeCriticalSection` 71/21, `DeleteCriticalSection` 37/11; `WaitFor*` 15/6;
`_endthread`/`ExitThread`/`TerminateThread` 2/2.

| Osa / Part | Sisältö / Content | Agentti, malli, budjetti / Agent, model, budget |
|---|---|---|
| A | Säiekartta: jokainen käynnistyskohta → aloitusfunktio, käynnistäjä, ohjelmat, elinikä, lopetus (lippu, odotus, ei mitään); täydentää `tietorakenteet-ja-tila.md` "Säikeet" ([tuntematon: elinikä ja lopetus]) / thread map | `architecture-analyst`, Sonnet, 150 k |
| B | Lukkokartta: 24 lukkoa – alustus, poisto, suojattu tila, ottavat säikeet; `tall_CS`:n kutsupaikat vain viittauksena / lock map | `defect-concurrency-auditor`, Sonnet, 150 k (B + C) |
| C | Lukitusjärjestys: sisäkkäiset `Enter`-parit funktion sisällä ja yhden kutsutason yli koneellisesti; ristiriitaiset parit lukkiutumisehdokkaiksi; jo kirjatut H-008, H-009, H-012, H-021, H-024, H-040 vain viittauksena / lock order candidates | sama / same |
| D | Lopetus: ohjelman sulku ja vaiheen tai kilpailun vaihto säikeiden ollessa käynnissä (liittyy Q30) / shutdown and stage change | A:n agentti / A's agent |
| E | Riippumaton tarkastus vain uusille korkea- ja keskitasoväitteille ja lukkiutumisehdokkaille; Sonnet, Opus vain monitasoiselle korkealle väitteelle / risk-based review | `independent-verifier`, 80 k |

Yhteensä enintään noin 380 k tokenia (C-5: noin 1,56 milj.). Jos budjetti uhkaa ylittyä,
agentti palauttaa keskeneräisen listan, ja loput kirjataan avoimiksi. Rajattu pois:
ajonaikaiset kokeet, uudet koeohjelmat ja testikehykset, VCL-säieturvallisuus muuten kuin
H-018:n laajennuksena, korjaukset. Valmis, kun säiekartta kattaa kaikki käynnistyskohdat
(tai syy puuttumiseen on kirjattu), lukkokartta kaikki 24 lukkoa, järjestysparit on listattu,
uudet havainnot on tarkastettu riskiperusteisesti ja validaattori menee läpi. Vaihe 2
(ehdokkaiden syväanalyysi) erillisellä päätöksellä. [ehdotus]

First bounded part of batch 6: thread map (A), lock map (B), lock-order candidates (C) and
shutdown paths (D); earlier batches are inputs, not re-analysed. Starting data above.
Agents run on Sonnet; review (E) only for new high/medium claims and deadlock candidates,
Opus only for multi-level high claims. Total budget about 380 k tokens; an agent that would
exceed it returns a partial list and the rest is recorded as open. Out of scope: run-time
experiments, new probes or harnesses, VCL thread safety beyond extending H-018, fixes.
Step 2 (deep analysis of candidates) on a separate decision. [proposal]

### Työtapa erästä C-6 alkaen / Working method from batch C-6 on

Käyttäjän päätös 2026-11-25 (C-5:n agenttikulutus noin 1,56 milj. tokenia): Sonnet on
oletusmalli (agenttikutsun `model: sonnet`, agenttimäärittelyjä ei muuteta); Opus vain
perustelluissa vaativissa tarkastuksissa; ei päällekkäistä analyysiä; jokaiselle työerälle
rajaus ja tokenbudjetti etukäteen; olemassa olevat todisteet ennen uusia koeohjelmia;
riippumaton tarkastus riskiperusteisesti. Tämä korvaa yllä olevan säännön, että jokainen
erä päättyy täyteen riippumattomaan tarkastukseen.

User decision: Sonnet by default (per-call model override, agent definitions unchanged),
Opus only for justified demanding reviews, no overlapping analysis, scope and token budget
up front, existing evidence before new probes, risk-based independent review. This replaces
the rule above that every batch ends with a full independent review.
