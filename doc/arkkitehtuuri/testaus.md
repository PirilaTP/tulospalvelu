# Testausstrategia ja ajonaikainen todennus / Test strategy and runtime verification (C-11)

Erä C-11, 2026-11-24, lähdekoodin revisio `f7fe553` (`TPsource/` ei muuttunut sen
jälkeen). Polut suhteessa `TPsource/V52/`, ellei toisin mainita. Tämä on
**suunnitelma**: yhtään testiä ei ole toteutettu, eikä yhtäkään havaintoa ole tämän
erän perusteella vahvistettu ajamalla. Toteutus vaatii erillisen luvan, koska testit
ja testiprojektit sijoittuvat `TPsource/V52/`-hakemistoon.

Batch C-11, 2026-11-24, source revision `f7fe553` (`TPsource/` unchanged since).
Paths relative to `TPsource/V52/` unless stated. This is a **plan**: no test has
been implemented, and no finding is runtime-verified because of this batch.
Implementation needs separate approval, because the tests and test projects live
under `TPsource/V52/`.

Lähteet / sources: kolme analyysiagenttia (`build-dependency-analyst`,
`architecture-analyst`, `defect-concurrency-auditor`) ja kaksi riippumatonta
tarkastusta (`independent-verifier` V1, V2). Tarkastajien korjaukset on viety tähän
dokumenttiin; ne on lueteltu kohdassa "Riippumaton tarkastus". Aiempien erien
testattavuustaulukot (`tiedostomuodot.md`, `tiedonsiirto.md`,
`tallenna-samanaikaisuus.md`) ovat lähtötietoja; tämä dokumentti korvaa ne siltä osin
kuin ne eroavat.

Sources: three analysis agents and two independent reviews. The reviewers'
corrections are already applied here and listed under "Independent review". The
testability tables of earlier batches are inputs; this document supersedes them
where they differ.

## 1. Todennuksen tasot / Levels of verification

Rekisteri erottaa staattisen analyysin, riippumattoman lähdetarkastuksen ja
ajonaikaisen todennuksen (`CHECKPOINT.md`). Tämä suunnitelma lisää välitason:
vika voidaan toistaa **testikehyksessä**, joka linkittää tuotantokoodin
toimitetuilla käännösmäärityksillä, mutta ei aja varsinaista ohjelmaa. Se on
vahvempi kuin lähdetarkastus mutta heikompi kuin vian toistaminen ajossa olevassa
ohjelmassa, koska fixture asettaa tilan käsin.

The register separates static analysis, independent source review and runtime
verification. This plan adds an intermediate level: a defect can be reproduced
**in a test harness** that links production code with the shipped build defines,
without running the actual program. It is stronger than source review but weaker
than reproducing the defect in the running program, because the fixture sets the
state by hand.

| Taso / Level | Mitä ajetaan / What runs | Rekisterin arvo (ehdotus) / Register value (proposal) |
|---|---|---|
| Staattinen / static | ei mitään / nothing | nykyinen / current |
| Riippumaton lähdetarkastus / independent source review | ei mitään / nothing | nykyinen / current |
| Toistettu testikehyksessä / reproduced in test harness | tuotantokoodin funktiot, toimitetut määritykset, synteettinen tila / production functions, shipped defines, synthetic state | **uusi, vaatii hyväksynnän** / new, needs approval |
| Vahvistettu ajamalla / confirmed by running | toimitettu ohjelma (HkMaali, ViestiMaali, GUI) synteettisellä aineistolla / shipped program with synthetic data | nykyinen määritelmä / current definition |

Testi voi myös **kumota** havainnon: jokaiselle testille on alla kirjattu tulos, joka
kumoaa väitteen.

A test can also **refute** a finding: every test below states the result that
would refute the claim.

## 2. Nykytila / Current state

| Asia / Item | Tila / State | Näyttö / Evidence |
|---|---|---|
| `TpTest` | 7 lähdettä: doctest-main, 2 testitiedostoa, 4 puhdasta tuotantoyksikköä; ei kirjastoja, ei ominaisuusmäärityksiä, ei `/J`; v145 kiinteästi / 7 sources, 4 pure production units; no libs, no feature defines, no `/J`; v145 hard-coded | `VS/Tests/TpTest.vcxproj:23`, `:60`, `:77`, `:87-93` [todennettu] |
| Testit / tests | 46/46 läpi, 94 tarkistusta; `--reporters=junit` toimii / pass, junit works | ajettu 2026-11-24 / run [todennettu] |
| Testien malli / test pattern | puhdas logiikka irrotettu omaksi yksiköksi, tuotanto syöttää globaaleista; ei tynkiä / pure logic extracted, production feeds it; no stubs | `Juk/VRinnakkaisOsuus.h:17-23` [todennettu] |
| Ydinkoodin testit / core tests | ei yhtään: dbboxm, pack, tiedonsiirto, lukot testaamatta / none | [todennettu] |
| CI | `console`-työ (`windows-latest`, v143) kääntää, ei aja testejä; `gui` itse ylläpidetyllä koneella, fork-suoja / builds only; fork guard | `.github/workflows/build.yml:13`, `:22-28`, `:36` [todennettu] |
| `Tests/run.sh` (g++) | rikki: rivin 14 lopusta puuttuu `\`, `-I Hk` puuttuu / broken | `Tests/run.sh:14` [todennettu] |
| doctest ja bcc32 | doctest 2.4.11 ei käänny klassisella bcc32:lla (`nullptr`); `static_assert` ja negatiivisen taulukon typedef toimivat molemmilla / doctest does not compile on bcc32; both assert forms work on both | `Tests/doctest.h:49-51`; koeohjelma / probe [todennettu] |
| ASan x86 | paikallisessa VS 18:ssa (14.52) on `clang_rt.asan_dynamic-i386.dll`; ei paikallisessa VS 2022:ssa; `windows-latest` [tuntematon] / present in local VS 18, not local VS 2022 | [todennettu] paikallisesti / locally |
| Olemassa oleva testimäinen koodi / existing test-like code | `dbboxm/Boxtest.cpp` (interaktiivinen, ei projektissa), `Tp/mttest.cpp` (demo), `tputilv2/LibTest.cpp` (VCL, käsin); `testucode.cpp` on kirjastofunktio, ei testi / interactive, demo, manual; not a test | [todennettu] |
| Sisäänrakennetut testikytkimet / built-in test switches | `TESTAUS`, `COMTESTI` (`ComTestParam.txt`), viestin Ctrl-T (`TESTIAJAT`) | `Hk/HkInit.cpp:1395-1403`; `Tp/TpComY.cpp:3038-3100` [todennettu] olemassaolo / existence |

Testejä ajetaan nyt vain käsin. Mikään nykyinen testi ei kosketa yhtäkään
H-havaintoa.

Tests are run only by hand today. No current test touches any H finding.

## 3. Ehdotettu testausarkkitehtuuri / Proposed test architecture

### 3.1 Kerrokset / Layers

| Kerros / Layer | Mitä todentaa / Verifies | Työkalu / Tooling | Eristys / Isolation | Havainnot / Findings |
|---|---|---|---|---|
| (a) Asettelutarkistukset / layout checks | Mitatut `sizeof`/`offsetof` (C-3); `combufsize`- ja `kilprecsize`-kaavat **ajonaikaisesti** / measured sizes; size formulas **at run time** | doctest-tapaus kussakin ydinprojektissa; käännösaikainen `static_assert` vain kiinteille rakenteille / doctest case per core project; compile-time only for fixed structs | ei tilaa / none | H-041, H-025, rakenteiden muutokset / layout drift |
| (b) Yksikkö / unit (`TpTest`) | Puhtaat funktiot; dbboxm tyngillä; irrotetut tarkistusfunktiot / pure functions; dbboxm with stubs | nykyinen TpTest | prosessin sisällä / in-process | H-043, H-015; H-032, H-027 vasta irrotuksen jälkeen / only after extraction |
| (c) Komponentti / component (`TpCoreHk`, `TpCoreJuk`) | Ydinyksiköt fixtureilla: lukkokoettimet, pakettien syöttö, XML, pack, `COMFILE.DAT` / core units with fixtures | doctest; konfiguraatiot Release ja Asan / configs Release and Asan | **yksi aliprosessi per testitapaus**, oma väliaikaishakemisto / one subprocess per test case, own temp dir | H-040, H-041, H-042, H-025, H-024, H-026, H-008, H-031, H-022, H-023, H-007, H-001, H-002, H-032, H-027 |
| (d) Musta laatikko / black box | Toimitettu `HkMaali.exe`/`ViestiMaali.exe` synteettisellä aineistolla / shipped console exes with synthetic data | ajuri: `CREATE_NEW_CONSOLE`, `AttachConsole` + `WriteConsoleInputW`, `WM_CLOSE`, aikakatkaisu / driver | oma hakemisto per instanssi; vaatii interaktiivisen istunnon / own dir; needs interactive session | H-041 (todellinen ohjelma / real program), H-031, savutestit / smoke |
| (e) Kaksi prosessia / two processes (loopback) | Replikointi ja protokolla, väärennetyt datagrammit / replication, forged datagrams | `YHTEYS1=UDP:p1:127.0.0.1:p2` + peilikuva / mirror | vain 127.0.0.1, erilliset hakemistot / loopback only | H-007, H-016, H-022, H-023, H-026, H-032 |
| (f) GUI | VCL-dialogit, `SendMessage`-lukkiutuminen, MySQL | käsin, testikone / manual, test VM | erillinen kone / separate machine | H-009, H-014, Q15 |

Ensimmäinen toteutuserä kattaa kerrokset (a)–(c) ja TpTestin CI-ajon. Kerrokset
(d)–(f) suunnitellaan myöhemmin.

The first implementation batch covers layers (a)–(c) and running TpTest in CI.
Layers (d)–(f) come later.

### 3.2 Testiprojektit / Test projects

| Projekti / Project | Lähteet / Sources | Määritykset / Defines | Huomiot / Notes |
|---|---|---|---|
| `TpTest` (nykyinen / existing) | puhtaat yksiköt / pure units | `_CONSOLE` | ei muutu; saa myöhemmin irrotetut funktiot / unchanged; later gets extracted functions |
| `TpCoreHk` (uusi / new) | HkMaali Release -lähdelista **ilman** `HkConsole/HkMain.cpp` + testit + tyngät / HkMaali list **minus** `HkMain.cpp` | `VS/Hk/HkMaali/HkMaali.vcxproj:154` sellaisenaan (`N_PV` 5, `VALUKU` 250 oletuksina), `/J`, `UNICODE` | linkitys kokeiltu: 0 virhettä / link probed: 0 errors [todennettu] |
| `TpCoreJuk` (uusi / new) | JukMaali Release -lista **ilman** `JukConsole/laskenta.cpp` | `VS/V/JukMaali520.vcxproj:135` (`MAXOSUUSLUKU` 30, `VAIKALUKU` 12), `/J`, `UNICODE` | linkitys kokeiltu: 0 virhettä [todennettu] |

Perustelut / rationale:

- HK ja viesti eivät mahdu samaan ohjelmaan: molemmat määrittelevät luokan
  `kilptietue` ja globaalin `kilpparam` (`Hk/HkDef.h:496`, `Juk/VDef.h:734`,
  `Hk/HkGlobals.cpp:594`, `Juk/VGlobals.cpp:79`). [todennettu]
- Pääohjelmatiedosto jätetään pois. `HkMain.cpp` määrittelee vain `valikko`n ja
  `wmain`in (`HkConsole/HkMain.cpp:53`, `:178`), `laskenta.cpp` samoin
  (`JukConsole/laskenta.cpp:27`, `:98`). `/Dwmain=`-uudelleennimeäminen ei hyödytä,
  koska `wmain` kutsuu `initcon`ia ja `aloitus`ta. [todennettu]
- Pois jätetyn `wmain`in oletusarvot (`HkConsole/HkMain.cpp:185-201`: `maxasakko` 5,
  `valuku` 10, nimien pituudet …) toistetaan fixturessa. [todennettu]
- Kaikki tuotantokoodi käännetään `/J`:llä kuten tuotantoprojekteissa; TpTestissä sitä
  ei ole, joten tuotantolähteitä ei lisätä TpTestiin. [todennettu]
- Release-käännös, ei Debug: asettelumääritykset ovat samat, mutta Release vastaa
  toimitettua ohjelmaa ja on yhteensopiva ASanin kanssa. Asan-konfiguraatio:
  `/fsanitize=address`, ei `/RTC1`, ei `/ZI`, ei inkrementaalista linkitystä.

- HK and relay cannot share one executable (both define `kilptietue` and `kilpparam`).
- The main file is excluded; it only defines `valikko` and `wmain`. Renaming with
  `/Dwmain=` gains nothing.
- The excluded `wmain`'s defaults are reproduced in the fixture.
- Production code is compiled with `/J` as in the product projects; TpTest lacks it,
  so production sources are not added to TpTest.
- Release, not Debug; the Asan configuration avoids `/RTC1`, `/ZI` and incremental
  linking.

### 3.3 Testikomponentit / Test components

| Komponentti / Component | Tehtävä / Purpose | Keskeiset yksityiskohdat / Key details |
|---|---|---|
| `HkFixture` | HK-ytimen alustus ilman `aloitus`ta / HK core init without `aloitus` | `kilpparam` **ennen** ensimmäistä `kilptietue`-oliota (Q34; `Hk/HkDat.cpp:193-209`); `InitializeCriticalSection` kuten `Hk/HkInit.cpp:3099-3110`; `luekentat(0)`; `makefile`/`openfile` väliaikaishakemistossa; `aani = 0` |
| `VFixture` | Viestiytimen alustus / relay core init | `InitializeCriticalSection` kuten `Juk/VInit.cpp:2557-2566`; `asetaOletusarvot()` (`Juk/VInit.cpp:2460-2475`, ei otsikossa / not in a header); koot kuten `Juk/VInit.cpp:2661-2662`; `aani = 0` |
| `UiStubs` | Estää käyttöliittymän odotukset / blocks UI waits | korvaa `writeerror`, `writeerror_w` **ja** `writeerrorOn` yhdessä (muuten LNK2005); laskee viestit; `wselectopt`-tynkä tai polun välttäminen |
| `LockProbe` | Lukon tila **toisesta** säikeestä / lock state from another thread | `TryEnterCriticalSection`; saman säikeen koetin on hyödytön (rekursiivinen lukko) / same-thread probe useless (recursive) |
| `WaiterProbe` | H-007: odota, että `tall_CS`:llä on odottaja / wait for a waiter | `(-1 - LockCount) >> 2`; mitattu 200/200 Windows 11 26100:lla; ei dokumentoitu rajapinta → aikakatkaisu ja tarkistus CI-kuvassa / undocumented → timeout, recheck on CI image |
| `InbufInjector` | Vastaanottopaketin syöttö ilman soketteja / packet injection without sockets | `inbuf`, `inbseur`, `inbens`, `comfl` ja `tarkcom`-säie (`Juk/VCom.cpp:893-916`) |
| `TempDir` | Oma hakemisto per aliprosessi / per-subprocess directory | `SetCurrentDirectory` **ennen** mitään tiedosto-operaatiota: `dataf2name` ja `obfilename` ovat suhteellisia kiinteän kokoisia taulukoita (`Juk/VGlobals.cpp:108`, `:117`; `Hk/HkGlobals.cpp:121-128`) |
| `DataWriters` | Synteettiset `KILP.DAT`, `COMFILE.DAT`, `KilpSrj.xml`, `LASKENTA.CFG` | `KILP.DAT` tuotannon `pack` + dbboxm; XML **yksi elementti per rivi** (`tputilv2/xmlparser.cpp:216-294`); vain ASCII |
| `run_core_tests` (ajuri / runner) | Ajaa jokaisen testitapauksen omassa prosessissa / one process per test case | `TpCoreX.exe -tc="<nimi>"`; aikakatkaisu ja tappaminen; arvioi poistumiskoodin (`0xC0000409`, `0xC0000005`, ASan) ja tulosteen; JUnit-yhteenveto / timeout and kill; judges exit code; JUnit summary |

`writeerror_w(...,0)` ei jää odottamaan testiprosessissa, koska `hConIn` on NULL ilman
`initcon`ia (`tputilv2/initcon.cpp:25`, `:77`) ja `readkbd` palaa heti
(`tputilv2/Readkbd.cpp:40-52`); tarkastaja mittasi 0 ms. **Valintakehotteet jäävät
kuitenkin ikuiseen silmukkaan** (`tputilv2/wSelectxy.cpp:46-73`), joten testi ei saa
kulkea `wselectopt`-polun kautta (esim. `Hk/HkCom32.cpp:860-868`, `Tp/TpComY.cpp:2298`,
`:2338`). Tyngän korvaaminen onnistuu linkittämällä valmis `tputilv2.lib`, kunhan tynkä
määrittelee kaikki kolme `WRITEERR.obj`:n symbolia (koeohjelma). [todennettu]

`writeerror_w(...,0)` does not block in a test process: `hConIn` is NULL without
`initcon`, and `readkbd` returns at once (0 ms measured). **Selection prompts,
however, loop forever**, so tests must not pass through `wselectopt` paths. The
stub can replace the library member if it defines all three `WRITEERR.obj` symbols
(probed).

### 3.4 Hakemistorakenne (ehdotus) / Directory layout (proposal)

```
TPsource/V52/Tests/
  (nykyiset / existing: DoctestMain.cpp, HkEnnTavTest.cpp, VRinnakkaisOsuusTest.cpp, WideString.h, doctest.h)
  core/        HkCoreTests.cpp, VCoreTests.cpp, LayoutHk.cpp, LayoutV.cpp
  fixtures/    HkFixture.*, VFixture.*, UiStubs.cpp, LockProbe.h, InbufInjector.h, TempDir.h
  data/        DataWriters.*, xml/*.xml (ASCII, synteettinen / synthetic)
  run_core_tests.py   (tai / or .ps1)
TPsource/V52/VS/Tests/
  TpTest.sln (+ TpCoreHk.vcxproj, TpCoreJuk.vcxproj)
```

Uudet lähdetiedostot kirjoitetaan ASCII:na ja kommentit englanniksi
(`merkistot.md`). / New sources are ASCII with English comments.

### 3.5 Turvallisuus ja eristys / Safety and isolation

1. **Ei oikeaa kilpailuaineistoa, ei tuotantopalvelimia.** Kaikki aineisto syntyy
   testissä. / No real competition data or production servers.
2. **Verkko vain 127.0.0.1.** Vapaa portti `bind(127.0.0.1:0)` + `getsockname`. Ei
   palvelintilaa (`comtpTCPSRV` sitoo `INADDR_ANY`, `com/WinUDP.cpp:795`), ei BRO- eikä
   MONITORI-yhteyksiä. H-029:n vuoksi ohjelma vastaa viimeisen datagrammin
   lähettäjälle, joten kaikkien testisokettien on oltava loopbackissa. / Loopback only;
   no server mode, broadcast or monitor links.
3. **Tiedostot vain väliaikaishakemistossa.** `SetCurrentDirectory` ennen ensimmäistä
   tiedosto-operaatiota; muuten `openfile` voi avata ja `closefile` kirjoittaa
   nykyhakemiston `KILP.DAT`:iin. / Files only in a temp directory, set before any
   file operation.
4. **Kaatuvat ja jumiutuvat testit omassa prosessissa** aikakatkaisulla. doctestin
   `should_fail` ei kata kaatumisia, ja `timeout()` vain raportoi
   (`Tests/doctest.h:4686`, `:1929`); ajuri tuomitsee. / Crashing and hanging tests in
   their own process; the runner judges.
5. **Ei dialogeja.** `SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX)`,
   `_set_abort_behavior`, `_CrtSetReportMode`/`_RTC_SetErrorFunc`. Nämä eivät estä
   `perr`in `MessageBoxA`:ta (`tputilv2/perr.cpp:51`): `initcon`ia ei kutsuta, ja
   `KILP.DAT`-otsakkeen ja tietuepituuden on oltava oikeat (`dbboxm/dbbox1.cpp:74`,
   `:204-207`). / No dialogs; `perr` must be avoided by construction.
6. **Ikuisesti lukitut säikeet:** prosessi lopetetaan `ExitProcess`illa. / Threads
   blocked forever: end the process with `ExitProcess`.
7. **Itse ylläpidetty CI-kone:** vain samasta reposta tulevat ajot (nykyinen
   fork-suoja); MSVC-testit ajetaan `windows-latest`-koneella. / Self-hosted runner:
   same-repo runs only; MSVC tests on hosted runners.

## 4. Korkean vakavuuden havaintojen testattavuus / Testability of high-severity findings

Kaikki 17 korkean vakavuuden havaintoa. "Deterministinen" tarkoittaa, että tulos ei
riipu ajoituksesta. / All 17 high-severity findings. "Deterministic" means the
result does not depend on timing.

| ID | Ympäristö / Environment | Deterministinen / Deterministic | Kaatuu / Crashes | CI | Vaikeus / Difficulty | Erä / Batch |
|---|---|---|---|---|---|---|
| H-040 | TpCoreJuk | kyllä / yes | ei / no | hosted | M | 1 |
| H-041 | TpCoreHk (+ musta laatikko / black box) | kyllä | (b) kyllä / yes | hosted; musta laatikko itse ylläpidetty / black box self-hosted | M | 1 |
| H-042 | TpCoreJuk | kyllä | (b) kyllä | hosted | S (a), M (b) | 1 |
| H-025 | TpCoreJuk | kyllä | ei | hosted | S (a), M (b), L (c) | 1 |
| H-027 | TpCoreJuk tai / or TpCoreHk, loopback | kyllä (tila / state); aikaraja rajaa / time bound only | (b) kyllä | hosted | M | 1 |
| H-031 | TpCoreHk | kyllä | ei | hosted | S | 1 |
| H-024 | TpCoreJuk | kyllä | ei | hosted | S–M | 1 |
| H-008 | TpCoreHk | kyllä | ei | hosted | M | 1 |
| H-026 | TpCoreJuk | kyllä (pysyvä tila / permanent state) | jumittuu / hangs | hosted | M | 2 |
| H-032 | TpCoreJuk (+ loopback) | kyllä (vartija / sentinel) | (b) kyllä | hosted | M–L | 2 |
| H-022 | TpCoreHk | kyllä (yksi prosessi / single process) | ei | hosted | M | 2 |
| H-023 | TpCoreHk | kyllä, jos `ln[0]==0` / if | ei | hosted | M | 2 |
| H-007 | TpCoreHk | kyllä `WaiterProbe`lla; muuten tilastollinen / otherwise statistical | ei | hosted | M | 2 |
| H-001 | TpCoreJuk | kyllä (ASan/RTC) | kyllä | hosted | M | 2 |
| H-002 | TpCoreJuk | kyllä, jos osoite varataan / if address reserved | kyllä | hosted | S–M | 2 |
| H-009 | GUI, käsin / manual | vain debuggerilla tai koukulla / only with debugger or hook | jumittuu / hangs | ei / no | L | käsin / manual |
| H-014 | GUI + paikallinen MySQL / local MySQL | kyllä | ei | ei / no | L | käsin / manual |

15/17 voidaan testata ilman GUI:ta, verkkoa tai oikeaa aineistoa; H-009 ja H-014
vaativat GUI:n. Ensimmäisen erän testit eivät tarvitse tuotantokoodiin testikoukkuja.

15 of 17 can be tested without the GUI, a LAN or real data; H-009 and H-014 need
the GUI. The first-batch tests need no production test hooks.

## 5. Ensisijaisten havaintojen testisuunnitelmat / Test plans for priority findings

Jokaisessa: (1) toiminto ja viitteet, (2) tavoite, (3) ympäristö, (4) aineisto,
(5) vaiheet, (6) oikea toiminta, (7) vian osoittava tulos ja kumoava tulos,
(8) automatisointi, (9) koodimuutokset, (10) vaikeus.

Each has: (1) function and refs, (2) goal, (3) environment, (4) data, (5) steps,
(6) correct behaviour, (7) defect result and refuting result, (8) automation,
(9) code changes, (10) difficulty.

### 5.1 H-040: viestin `addfrec` ei vapauta lukkoa / relay `addfrec` never releases its lock

1. **Toiminto / function:** `static void addfrec`, `Juk/vdat.cpp:178`;
   `EnterCriticalSection(&kilpdat_CriticalSection)` rivillä `:183` ja uudelleen
   `:190`, ei `Leave`a. Kutsuja `addtall` (`:858-905`, `tall_CS` `:867`, kutsu `:894`).
   Lukon muut käyttäjät: `delfrec` (`:194-204`), käyttämättömät `getrec_1`/`putrec_1`.
   [todennettu, V1]
2. **Tavoite / goal:** osoittaa, että `addfrec`in palattua lukko on yhä kutsujan
   hallussa, joten `delfrec`/`addfrec` muussa säikeessä jumittuu. / Show the lock is
   still held after return.
3. **Ympäristö / environment:** TpCoreJuk, Release-määritykset. Testitiedosto
   `#include "../../Juk/vdat.cpp"`, ja `vdat.cpp` jätetään projektista pois; tiedoston
   staattiset muuttujat (`:33-34`) ovat harmittomia. `addtall` on huono sisääntulo
   (vaatii nimi-indeksin, sarjat, `tallenna`n). / Include `vdat.cpp` in the test file.
4. **Aineisto / data:** `VFixture`; `SetCurrentDirectory(temp)`;
   `makefile(&datf2, dataf2name, kilpparam.kilprecsize)`. `addfrec` sulkee ja avaa
   tiedoston uudelleen (`:185-189`) nimellä `dataf2name` = `"KILP.DAT"`
   (`Juk/VGlobals.cpp:108`). `kilparr`: vähintään (numrec+1)·kilprecsize tavua, koska
   `PUTREC` kirjoittaa sinne (`:171-176`). Yksi nollattu `kilptietue`.
5. **Vaiheet / steps:** pääsäie kutsuu `addfrec(&k, &d)`. Säie B kutsuu
   `TryEnterCriticalSection(&kilpdat_CriticalSection)` ja raportoi. Toissijaisesti
   `RecursionCount` (sisäinen kenttä).
6. **Oikea toiminta / correct:** B saa lukon (TRUE).
7. **Vika / defect:** B saa FALSE; `RecursionCount == 2`. Deterministinen.
   **Kumoaa / refutes:** TRUE. Tämä ei näytä varsinaista jumiutumista; valinnainen
   lisävaihe: B kutsuu `deltall` 5 s aikarajalla, prosessi päätetään `ExitProcess`illa.
   / Does not show the actual freeze; optional extra step.
8. **Automatisointi / automation:** CI (hosted), ei kaadu, ei soketteja.
9. **Muutokset / changes:** vain testitiedostot ja projekti. / Test-only.
10. **Vaikeus / difficulty:** M (testi pieni, TpCoreJuk-projekti on työ / project is
    the work).

### 5.2 H-041: HK:n tietue suurempi kuin pinopuskuri / HK record larger than stack buffer

1. **Toiminto / function:** `KILPBUFSIZE` 10000, kun `VALUKU >= 31`
   (`Hk/HkDef.h:98-103`); `N_PV` 5 ja `VALUKU` 250 oletuksina (`Hk/HkDef.h:45-46`,
   `:57-58`), HkMaali Release ei ohita (`VS/Hk/HkMaali/HkMaali.vcxproj:154`), HkKisaWin
   ohittaa vain 200Release-konfiguraatiossa. Koko: `Hk/HkDat.cpp:137`, `:176-177`.
   Pinopuskurit: `Hk/HkDat.cpp:565` (`read`), `:583` (`write`), `:629` (`addfrec`),
   `:1433`, **sekä `cbHk/UnitExport.cpp:464`**. Ainoa raja `Hk/HkSrj.cpp:158`, jonka
   XML-polku ohittaa (`Hk/HkSrj.cpp:72-75`, `Hk/HkXml.cpp:439-442`). [todennettu, V1]
2. **Tavoite / goal:** (a) `kilprecsize > KILPBUFSIZE` toimitetuilla määrityksillä;
   (b) pinon ylivuoto `kilptietue::read`/`write`.
3. **Ympäristö / environment:** TpCoreHk; (b) aliprosessissa, Release `/GS` ja Asan.
4. **Aineisto / data:** `kilpparam.n_pv = 5`, `valuku = 250` (tai `KilpSrj.xml`, jossa
   `<Races>5</Races>`, `<Intermediaries>250</Intermediaries>`, yksi elementti per
   rivi); asetetaan **ennen** ensimmäistä `kilptietue`-oliota. `KILP.DAT`, jossa
   `recl = kilprecsize` ja täysi tietue 1 (lyhyt tietue → `perr` → MessageBox).
5. **Vaiheet / steps:** (a) `luekentat(0)`; tarkista lisäksi, että se antaa
   `kilprecsize0` 360, `kilppvtpsize0` 152 ja `vatpsize` 8 (C-11: 152 mitattu ajossa; suunnitelmassa ollut 154 oli virhe / 152 measured, the planned 154 was wrong); sitten
   `CHECK(kilpparam.kilprecsize <= KILPBUFSIZE)`. (b1) `/GS`: `k.read(1, 0)` →
   `getrec` lukee tasan `recl` tavua (`dbboxm/dbbox1.cpp:74`). (b2) Asan: `write()`-
   tai `pack()`-polku, koska ASan ei todennäköisesti näe ytimen `ReadFile`-kirjoitusta
   [päätelty]. (b3) vaihtoehto: `pack()` kasapuskuriin, jonka koko on tasan
   `KILPBUFSIZE` + vartijatavut / canary.
6. **Oikea toiminta / correct:** `kilprecsize <= 10000`, tai konfiguraatio hylätään
   ennen tietueiden käsittelyä.
7. **Vika / defect:** (a) 360 + 5·(152 + 252·8) = **11 200** (mitattu / measured, luku 14); deterministinen, ei
   kaadu. (b1) poistumiskoodi `0xC0000409`; (b2) ASan stack-buffer-overflow;
   (b3) vartija muuttunut. **Kumoaa / refutes:** (a) ≤ 10000 tai käynnistystarkistus
   hylkää n_pv 5 / valuku 250.
8. **Automatisointi:** (a), (b) CI hosted. Toimitetun `HkMaali.exe`:n musta laatikko
   (kerros d) myöhemmin; HkKisaWin (bcc32, ei `/GS`) käsin.
9. **Muutokset:** vain testit. **Käännösaikaista `static_assert`ia ei tehdä**: 360 ja
   152 lasketaan ajossa kenttätaulukoista, joten käännösaikainen kaava olisi käsin
   kopioitu, ja nykyarvoilla se **estäisi käännöksen** (11 200 > 10 000). Ajonaikainen
   tarkistus merkitään odotetusti epäonnistuvaksi, kunnes vika korjataan;
   `static_assert` voi tulla vasta korjauksen mukana. Tämä korvaa
   `tiedostomuodot.md`:n ehdotuksen. / No compile-time assert: it would be a
   hand-copied formula and would break the build today. Supersedes the
   `tiedostomuodot.md` proposal.
10. **Vaikeus:** M.

### 5.3 H-042: viestin XML-määriä ei rajata / relay XML counts not clamped

1. **Toiminto / function:** `loadSoftwareData`, `Juk/VXml.cpp:430-450`: `Legs` →
   `kilpparam.n_os` (`:442`), `Intermediaries` → `valuku` (`:445`), `LegCount` →
   `n_os_akt` (`:482`). `lueEventXml(wchar_t*, bool lueSarjat, bool lueRadat, bool
   lueOsanottajat)` `:1521`, kutsuja `Juk/vSarjat.cpp:315`. Ainoat rajat GUI:ssa
   (`ViestiWin/UnitKilpMaaritys.cpp:271-277`, `:340`, `:365`). Ylivuodot:
   `Juk/vdat.cpp:91-101` (`pack`), puskurit `:145`, `:153`, `:181`;
   `Juk/vSarjat.cpp:60-70` (`nollaa`). [todennettu, V1]
2. **Tavoite:** (a) rajat ylittävät arvot hyväksytään; (b) ylivuoto.
3. **Ympäristö:** TpCoreJuk Release (`MAXOSUUSLUKU` 30, `VAIKALUKU` 12; ei
   Release150os/Debug250). (b) aliprosessissa.
4. **Aineisto / data:** jäsennin lukee yhden elementin riviltä
   (`tputilv2/xmlparser.cpp:216-294`); yhdelle riville kirjoitettu tiedosto ei testaisi
   vikaa lainkaan ja voisi jäädä odottamaan (`Juk/VXml.cpp:1687-1695`). Siksi:

   ```xml
   <Event>
   <Software>
   <VersionId>x</VersionId>
   <FileFormat>
   <Legs>31</Legs>
   <Intermediaries>13</Intermediaries>
   </FileFormat>
   </Software>
   <EventData>
   <LegCount>31</LegCount>
   <ClassCount>0</ClassCount>
   </EventData>
   </Event>
   ```

   Toinen tiedosto: yksi sarja ja `Legs` 200 (`nollaa`-ylivuoto).
5. **Vaiheet:** (a) `asetaOletusarvot()`, `lueEventXml(L"KilpSrj.xml", true, false,
   false)`, tarkista `n_os <= MAXOSUUSLUKU`, `valuku <= VAIKALUKU`,
   `n_os_akt <= MAXOSUUSLUKU` tai virhepaluu. (b) koot kuten `Juk/VInit.cpp:2661-2662`
   (KILPRECSIZE 6918, OSRECSIZE 226 mitattuna), `pack` → ASan tai vartija; `nollaa`:
   vertaa `Sarjat[1]` tavuittain ennen ja jälkeen (ASan ei näe olion sisäistä
   ylitystä).
6. **Oikea toiminta:** tiedosto hylätään tai arvot rajataan.
7. **Vika:** (a) paluu 0 ja `n_os == 31`; deterministinen. (b) ASan
   stack-buffer-overflow / vartija / `Sarjat[1]` muuttunut. **Kumoaa:** (a) hylätään
   tai rajataan.
8. **Automatisointi:** CI hosted.
9. **Muutokset:** vain testit ja ASCII-XML-aineisto.
10. **Vaikeus:** S (a), M (b).

### 5.4 H-025: viestin `combufsize` ilman otsaketta / relay `combufsize` lacks the header

1. **Toiminto:** `init_class_len`, `Juk/VCom.cpp:1236-1240` (`:1228` ja `:1233`
   ylikirjoitetaan); HK lisää 14 (`Hk/HkCom32.cpp:1095-1105`). Otsake 14 tavua,
   `sizeof(combufrec)` 258 (mitattu). Levyjono: kirjoitus `Tp/TpComY.cpp:509`, paikka
   `:495`, luku `:1470`, tarkistussumma lasketaan uudelleen `:1507-1508`. Viesti käyttää
   aina levyjonoa (`Juk/VGlobals.cpp:330`; nollaus kommentoitu `Juk/VInit.cpp:1218`).
   [todennettu, V1]
2. **Tavoite:** (a) `combufsize < 14 + max(class_len)`; (b) suurimman paketin 14
   viimeistä tavua katoavat levyltä; (c, myöhemmin) lähetetyssä datagrammissa on
   edellisen paketin loppu kelvollisella tarkistussummalla.
3. **Ympäristö:** TpCoreJuk; (c) loopback-UDP.
4. **Aineisto:** `kilpparam.n_os = 30` → luokkapituudet KILPT 144, OSUUST 160, EMITT
   188, EMITVA 204, **YHTEISLAHTOT 244 = suurin**. Kaksi YHTEISLAHTOT-pakettia,
   **pituus = `combufsize`** (lyhyempi paketti läpäisisi testin virheellisesti), rungot
   0xAA ja 0xBB. Esiehdot: `init_class_len()` (`kilprecsize0`, `osrecsize0`, `n_os`,
   `maxemvalah` asetettu), `SetCurrentDirectory(temp)`, `obfile = -1`,
   `maxyhteys = 1`, `cjseur`/`cjens` 0, `status_on = 0`, `opencomfile(0)` **tyhjään**
   hakemistoon (olemassa oleva tiedosto → `wselectopt`, `Tp/TpComY.cpp:2298`,
   `:2338`, jumittuu); otsake kirjoitettava ennen `buflah`ia (muuten
   `writeerror_w(...,0)`-polku `:506-515`).
5. **Vaiheet:** (a) `CHECK(combufsize >= 14 + class_len[i])` kaikille i. (b)
   `buflah(0, &P1)`, `buflah(0, &P2)`; lue `COMFILE.DAT` tavallisella tiedosto-I/O:lla
   kohdasta `((n+1)*maxyhteys + cn) * combufsize`, vertaa 258 tavua.
6. **Oikea toiminta:** (a) tosi; (b) jokaisessa paikassa koko paketti.
7. **Vika:** (a) 244 < 258; (b) jokaisen paketin 14 viimeistä tavua puuttuvat;
   deterministinen tavuvertailu. **Kumoaa:** (a) tosi kaikille luokille arvoilla n_os 4
   ja 30.
8. **Automatisointi:** (a), (b) CI hosted; (c) hosted loopback.
9. **Muutokset:** vain testit. Käynnistystarkistus tuotantoon kuuluu korjaukseen
   (erillinen lupa).
10. **Vaikeus:** S (a), M (b), L (c).

### 5.5 H-027: TCP-asiakas ei yhdistä uudelleen / TCP client never reconnects

1. **Toiminto:** `Tp/TpComY.cpp:1179`
   `err = openportTCP(...) && (comtype[cn] == comtpTCP)` → err on 0 tai 1, joten
   `WSAECONNREFUSED`-haara (`:1201-1203`) ei toteudu; `default` → `poistu` → paluu
   (`:1219-1220`), säie päättyy (`:1274-1275`). Muilla TCP-tyypeillä err = 0
   epäonnistumisesta huolimatta (myös `comtpTCPSRV`: `comopen` TRUE). TCPXML:
   `openportTCP` vapauttaa `TCPyhteys[yhtno]` ja nollaa sen (`com/WinUDP.cpp:812-817`),
   `*hC = yhtno+1` jää (`:737`), `TCPyht_on` palauttaa 0 (`:411-418`),
   `reconnectTCP` (kutsu `Tp/TpComY.cpp:1289`) viittaa NULLiin (`com/WinUDP.cpp:848`)
   ennen `RCWAIT`-viivettä. Oikeat sulut: `Tp/TpComY.cpp:1938`, `:2128`. [todennettu, V1]
2. **Tavoite:** (a) comtpTCP-asiakas: säie päättyy yhden yrityksen jälkeen eikä
   yhdistä palvelimen käynnistyttyä; (b) comtpTCPXML: muistinsuojausvirhe.
3. **Ympäristö:** TpCoreJuk tai TpCoreHk (`Tp/`-koodi yhteinen); vain asiakas,
   127.0.0.1.
4. **Aineisto:** `comtype[0] = comtpTCP` / `comtpTCPXML`, `ipparam[0]` = asiakas,
   `127.0.0.1:P`, P vapaa portti (`bind(0)` + `getsockname` + sulku); `comfl = 1`,
   `status_on = 0`.
5. **Vaiheet:** (a) säie `tiedonsiirtoTCP(&cn)`; odota sen päättymistä enintään 10 s;
   kuuntele sitten porttia P 15 s ja laske yhteydet. (b) aliprosessissa sama
   comtpTCPXML:llä, odota poistumista enintään 10 s.
6. **Oikea toiminta:** (a) säie jatkaa ja yrittää noin 6 s välein (2 s yhteysyritys +
   4 s `Sleep`, `:1217`) ja yhdistää; (b) ei kaadu.
7. **Vika:** (a) säie päättyy noin **2 s** kuluttua (mitattu: suljettuun
   loopback-porttiin `WSAECONNREFUSED` 2000 ms), `comopen[0] == FALSE`, 0 yhteyttä.
   (b) poistumiskoodi `0xC0000005` (`reconnectTCP`). **Kumoaa:** (a) säie elää ja
   yhdistää; (b) 30 s ilman kaatumista.
8. **Automatisointi:** CI hosted (loopback). 2 s:n aika on mitattu vain yhdellä
   koneella; aikarajat ovat väljiä.
9. **Muutokset:** vain testit. Palvelinvarianttia ei ajeta (`INADDR_ANY`,
   `com/WinUDP.cpp:795`).
10. **Vaikeus:** M (paljon globaalia tilaa: `lahcom`, `TCPviive_*`, lukot).

## 6. Muiden korkean vakavuuden havaintojen testit / Tests for the other high findings

Tiivis muoto; korjaukset tarkastuksesta (V2) on viety taulukkoon. / Compact;
review (V2) corrections applied.

| ID | Toiminto ja viitteet / Function and refs | Testi ja deterministinen tulos / Test and deterministic result | Kumoaa / Refuted if | Muutokset / Changes |
|---|---|---|---|---|
| H-008 | `tall_sak` `Hk/HkSak.cpp:59`; lukko `:89`, `return(-3)` `:99`; `tall_ampsakko` `:111` | `kilpparam.maxasakko >= 3` (muuten paluu -2 rivillä `:68` ennen lukkoa), sarja `lsak[k_pv] = 0`, numero 12 indeksissä (`Hk/HkIx.cpp:164-182`), `kilparr[d]`, `r_no = 999`; viesti `"  12 1 3"`; toisen säikeen `TryEnter(&tall_CS)` → FALSE | TRUE | testit / tests |
| H-024 | `tark_trk` (static) `Juk/VCom.cpp:680-708`; lukko `:690`, paluu `:701` | `kuuluttaja = 1`, `kilpstatus == 0`, `d.trk.kno == kilpno`, osuuden tark "KH":ssa (`:700`), TARKT `keskhyl = 'I'`; säie R ajaa `tark_trk`; pääsäikeen `TryEnter(&tall_CS)` → FALSE | TRUE | testit (`#include`) |
| H-031 | `vaihdavaihe` `Hk/HkIx.cpp:1855`; lippu `:1870`, paluu `:1872-1874`, nollaus vain `:1995` | `maxyhteys = 1`, `comopen[0] = 1`, `keyclose[0] = 0`, `cjseur[0] = 1`, `cjens[0] = 0`, `lah_tied_seur[0] = 0` (`Tp/TpComY.cpp:471-474`); paluu 1, `vaiheenvaihto == true`; jonon tyhjennyksen jälkeen toinen kutsu palauttaa 0 (`:1868-1869`) | lippu false | testit |
| H-026 | `Juk/VCom.cpp:971-973`, `:981`; luokat 7, 11, 12 vapaina (`:63-75`) | Luokan 7 paketti + kelvollinen paketti; `tarkcom` pyörii ikuisesti eikä noudata `comfl`ia → `inbens` ei etene 5 s; liitos aikarajalla ja prosessin lopetus | `inbens` etenee | testit |
| H-032 | `Tp/TpComY.cpp:1001` (ohitus, kun `class_len` 0); `:654` kopioi `len+14`; paikka `sizeof(combufrec)` (`:2399`); `LUEBUF` 3000 (`:61`) | (a) `inbseur == inbens` (`:649-650`), vartija paikassa +1, luokka 7, `len = 1000` → vartija muuttuu. (b) loopback-datagrammi, ASan | raja pitää / datagrammi hylätään | testit; tarkistuksen irrotus myöhemmin |
| H-022 | `Hk/HkDat.cpp:626-636`; `Hk/HkCom32.cpp:270-279` | `KILP.DAT` 3 tietueella; paikallinen `addtall` A (tietue 4); syötä KILPPVT d = 4 toisen kilpailijan tiedoilla → A:n `pv[0]` saa vieraan emit-koodin (yksi prosessi; kahden aseman kilpa kerroksessa e) | hylätään | testit |
| H-023 | `Hk/HkCom32.cpp:283-289` (`k_pv`), `:843-870` | (a) vastaanottaja `k_pv = 0`, VAIN_TULOST `d.v.k_pv = 1` → `pv[0]` muuttuu. (b) `tark_alku` vain kun `ln[0] == 0` (muuten `wselectopt` `:863` jumittuu); `pyyda_vaihdavaihe` (`:847`) tyngäksi | aika `pv[1]`:ssä tai hylätty | testit |
| H-007 | `Hk/HkCom32.cpp:260` (luku ilman lukkoa), `:326` (lukko) | T pitää `tall_CS`:ää; R ajaa `tark_kilp`; T odottaa `WaiterProbe`lla odottajaa, muuttaa tietueen seuran, tallentaa, vapauttaa → seura palautuu vanhaksi. Varalla 1000 toistoa satunnaisviiveellä (≤ 30 s), raportoidaan osuus | uusi seura säilyy | testit; valinnainen koukku (erillinen lupa) |
| H-001 | `Juk/VTulostus.cpp:2228`, `:3981` | C4789 todennettu koeohjelmalla ("stsk 40 bytes … 158 bytes"). Ajonaikaisesti ASan julkisen tulostusfunktion kautta; sisääntulo jäljittämättä [tuntematon]. `/we4789` vain testikäännökseen (rikkoisi viestin käännöksen; LTCG-vaikutus [tuntematon]) | ei raporttia | testit |
| H-002 | `Juk/vSarjat.cpp:376-377`, `:400` | `paikat[0] = 0x30303030`; laukeaa 9 kentällä tai tasan 6 numerokentällä. Deterministiseksi: `VirtualAlloc` osoitteeseen 0x30300000 ennen kutsua ja tarkista muuttuneet tavut | tavut ennallaan | testit |
| H-009 | `Hk/HkDat.cpp:944`, `:1250-1251`; `cbTp/cbTpApu.cpp:467-490` | GUI: pakotettu ajoitus debuggerilla tai tuotantokoukulla (lupa). Muuten rajattu rasitus ≤ 60 s vahtikoiran kanssa | ei jumiutumista | käsin |
| H-014 | `cbHk/UnitMySQL.cpp:637`, `:651-654`, `:699-701`; `Hk/HkInit.cpp:3740` | Paikallinen MySQL 127.0.0.1:ssä, HkKisaWin SQL päällä, vaiheen vaihto, muokkaus → rivejä ei kirjoiteta, säie päättynyt | rivejä tulee | käsin |

## 7. Ensimmäinen toteutuserä / First implementation batch (ehdotus / proposal)

Toteutetaan vasta luvalla. Kaikki muutokset testitiedostoihin, testiprojekteihin ja
(erillisellä luvalla) CI:hin; **ei tuotantokoodin muutoksia eikä testikoukkuja**.

Only with approval. Changes go to test files, test projects and (with separate
approval) CI; **no production code changes or test hooks**.

| # | Sisältö / Content | Tuotos / Output | Valmis, kun / Done when |
|---|---|---|---|
| 1.1 | TpTest CI:hin: käännös `/p:PlatformToolset=v143`, ajo `--reporters=junit`, artefakti `if: always()` `console`-työhön / TpTest into CI | `.github/workflows/build.yml` (erillinen lupa / separate approval) | CI ajaa 46/46 |
| 1.2 | `TpCoreJuk`-projekti, `VFixture`, `UiStubs`, `TempDir`, `LockProbe`, ajuri / project, fixtures, runner | uusi / new VS/Tests/TpCoreJuk.vcxproj, `Tests/fixtures/`, `Tests/run_core_tests.*` | tyhjä testi ajetaan aliprosessissa väliaikaishakemistossa / empty test runs in subprocess |
| 1.3 | H-025 (a)+(b) | uusi / new Tests/core/VCoreTests.cpp | tulos dokumentoitu / result documented |
| 1.4 | H-040 | sama / same | 〃 |
| 1.5 | H-042 (a) (+ (b) Asan-konfiguraatiossa / in Asan config) | sama + `Tests/data/xml/` | 〃 |
| 1.6 | `TpCoreHk`-projekti, `HkFixture` (sis. `wmain`in oletukset / incl. `wmain` defaults) | uusi / new VS/Tests/TpCoreHk.vcxproj, `Tests/fixtures/` | tyhjä testi ajetaan |
| 1.7 | H-041 (a) + (b1) `/GS` + (b3) vartija; asettelutarkistus 360/152/8 | uusi / new Tests/core/HkCoreTests.cpp, LayoutHk.cpp | 〃 |
| 1.8 | H-031, H-008 | sama | 〃 |
| 1.9 | H-024 | VCoreTests.cpp | 〃 |
| 1.10 | H-027 (a)+(b) loopback | VCoreTests.cpp | 〃 |
| 1.11 | Rekisterin ja tämän dokumentin päivitys tuloksilla (toistettu / kumottu) / register update with results | `rekisteri/havainnot.md`, `testaus.md` | jokaiselle testille tila / state per test |

Tunnetut viat merkitään testeissä odotetusti epäonnistuviksi (doctest
`should_fail` tai ajurin odotettu poistumiskoodi), jotta CI pysyy vihreänä, kunnes
korjaus tehdään; korjaus kääntää odotuksen. Kerrosten (c) ytimen testejä **ei
lisätä CI:hin** ensimmäisessä erässä, ennen kuin ne on ajettu paikallisesti
toistuvasti ja ASanin saatavuus `windows-latest`-koneella on todettu.

Known defects are marked expected-to-fail so CI stays green until fixed; a fix
flips the expectation. Core tests are **not added to CI** in the first batch until
run repeatedly locally and ASan availability on `windows-latest` is confirmed.

**Toinen erä / second batch:** H-026, H-032, H-022, H-023, H-007, H-001, H-002;
kerros (d) HkMaali-musta laatikko (H-041 toimitetussa ohjelmassa); TpCore-testit
CI:hin. **Käsin / manual:** H-009, H-014, H-041/H-042 bcc32:lla.

## 8. Muutokset ja riippuvuudet / Changes and dependencies

| Asia / Item | Tarve / Need | Tila / State |
|---|---|---|
| Kirjastot / libraries | `tputilv2.lib`, `dbboxm.lib` (`VS/Libs/tputilv2.sln`) ennen ydinprojekteja / before core projects | olemassa / exists |
| Järjestelmäkirjastot / system libs | `ws2_32 winhttp user32 gdi32 winspool advapi32 shell32 ole32 comdlg32`, `commode.obj` | linkitys kokeiltu / probed |
| Työkaluketju / toolset | v145 paikallisesti, v143 CI:ssä; TpTestin v143-käännös [tuntematon] | tarkistettava ensimmäisessä CI-ajossa / check in first CI run |
| ASan x86 | paikallisesti kyllä (VS 18); `windows-latest` + v143 [tuntematon] | tarkistettava / check |
| Python tai PowerShell ajurille / for the runner | Python 3.13 paikallisesti; `windows-latest`-kuvassa molemmat [päätelty] | – |
| Testikoukut tuotantoon / production hooks | ei ensimmäisessä erässä; myöhemmin ehkä H-007, H-009, irrotukset H-027/H-032 / none in batch 1 | erillinen lupa / separate approval |
| `Tests/run.sh` | poisto tai korjaus vain puhtaille yksiköille / drop or fix for pure units only | päätös / decision |
| bcc32 | ei ensimmäisessä erässä; myöhemmin käännösaikainen asettelu-TU itse ylläpidetyllä koneella / not in batch 1 | – |

## 9. Riippumaton tarkastus / Independent review

Kaksi `independent-verifier`-ajoa: V1 ensisijaiset testit (H-040, H-041, H-042, H-025,
H-027), V2 infrastruktuuri, muut testit ja turvallisuus. Kaikki viisi ensisijaista
vikaa pitivät staattisesti; jokaisen testisuunnitelman fixturea jouduttiin korjaamaan.

Two `independent-verifier` runs: V1 priority tests, V2 infrastructure, other tests
and safety. All five priority defects held statically; every test plan's fixture
needed corrections.

| # | Väite / Claim | Tulos / Verdict | Korjaus suunnitelmaan / Correction |
|---|---|---|---|
| V1.1 | H-040-testi / test | osittain / partly | `addfrec` avaa tiedoston uudelleen suhteellisella nimellä → `SetCurrentDirectory` ensin; `kilparr` koko; `addtall` huono sisääntulo |
| V1.2 | H-041 käännösaikainen tarkistus / compile-time check | **kumottu / refuted** | ajonaikainen tarkistus; `static_assert` estäisi käännöksen; ASan ei näe `ReadFile`-kirjoitusta → `/GS`- tai `pack`-polku; puuttuva viite `cbHk/UnitExport.cpp:464` |
| V1.3 | H-042 XML-aineisto / fixture | **osittain: aineisto väärä / fixture wrong** | yksi elementti per rivi; yksirivinen tiedosto ei testaisi vikaa |
| V1.4 | H-025-testi | osittain | paketin oltava suurinta luokkaa (244 @ n_os 30), muuten väärä läpäisy; `opencomfile` tyhjään hakemistoon |
| V1.5 | H-027-testi ja viitteet / refs | vahvistettu, ajoitus korjattu / confirmed, timing fixed | säie päättyy ~2 s (ei ~1 s); rivit `com/WinUDP.cpp:737`, `:848`, `Tp/TpComY.cpp:1289` |
| V2.1 | TpTest-anatomia / anatomy | vahvistettu | TpTestissä ei `/J`:tä → tuotantolähteet vain ydinprojekteihin |
| V2.2 | Linkitys ilman pääohjelmaa / link without main | osittain | `wmain`in oletukset fixtureen; `/Dwmain` hylätty |
| V2.4 | `writeerror_w(...,0)` jää odottamaan? / blocks? | **auditoijan väite kumottu / auditor's claim refuted** | ei jää (0 ms); `wselectopt` jää; `aani = 0`; tynkä kaikille kolmelle symbolille |
| V2.5 | Konsoliohjelmat vaativat konsolin / need a console | osittain | ensimmäinen virhe jo `GetConsoleScreenBufferInfo`: stdout ei saa olla uudelleenohjattu |
| V2.6 | bcc32 | vahvistettu | – |
| V2.7 | Muut testit / other tests | H-024, H-031, H-026, H-032, H-001 vahvistettu; H-008, H-022/H-023, H-002 osittain | ks. luku 6 (`maxasakko`, `ln[0]`, `VirtualAlloc`) |
| V2.8 | Eristys ja ASan / isolation and ASan | osittain | `should_fail`/`timeout` eivät kata kaatumisia → ajuri tuomitsee; dialogien esto ei koske `perr`iä |
| V2.9 | CI ja `run.sh` | vahvistettu | – |

## 10. Työmääräarvio / Effort estimate

| Osa / Part | Arvio / Estimate |
|---|---|
| 1.1 TpTest CI:hin / into CI | 0,5 istuntoa / session |
| 1.2 + 1.6 ydinprojektit, fixturet, tyngät, ajuri / core projects, fixtures, stubs, runner | 1–1,5 istuntoa |
| 1.3–1.10 kahdeksan havainnon testit / tests for eight findings | 1–1,5 istuntoa |
| 1.11 rekisteri ja dokumentit / register and docs | 0,25 istuntoa |
| **Ensimmäinen erä yhteensä / first batch total** | **noin 3 istuntoa / about 3 sessions** |
| Toinen erä (7 havaintoa, musta laatikko, CI) / second batch | 2–3 istuntoa |
| Käsin tehtävät GUI-testit / manual GUI tests | 1 istunto + testikone / test machine |

Arvio on karkea [päätelty]; suurin epävarmuus on, miten paljon globaalia tilaa
fixturet lopulta tarvitsevat. / Rough estimate; the main uncertainty is how much
global state the fixtures end up needing.

## 11. Avoimet kysymykset / Open questions

| # | Kysymys / Question |
|---|---|
| Q35 | Kääntyykö TpTest (ja ydinprojektit) v143:lla, ja onko `windows-latest`-kuvassa ASan x86? / Do the projects build with v143, and does `windows-latest` have x86 ASan? |
| Q36 | Pysyykö `LockCount`-koodaus samana CI-kuvassa (`WaiterProbe`, H-007)? / Is the `LockCount` encoding stable on the CI image? |
| Q37 | Kutsuuko HK:n käynnistys `kilptietue::read`ia jokaiselle tietueelle, jolloin H-041 kaataa toimitetun ohjelman jo käynnistyksessä? / Does HK startup read every record (H-041 crash at startup)? |
| Q38 | Mikä julkinen funktio johtaa `textjtulos`/`textosuus`-kutsuun (H-001)? / Public entry to `textjtulos` (H-001)? |
| Q39 | Muuttuuko `/we4789` virheeksi LTCG-linkityksessä (`/GL`)? / Does `/we4789` apply under LTCG? |
| Q40 | Voiko itse ylläpidetty CI-kone ajaa konsoliohjelmia interaktiivisessa istunnossa (kerros d)? / Can the self-hosted runner run console programs interactively? |
| Q41 | Mitkä ViestiWinin ja ViestiMaalin säikeet kutsuvat `addtall`/`deltall` (H-040:n todellinen jumiutuminen)? / Which relay threads call `addtall`/`deltall`? |
| Q42 | Voivatko HK:n SEURAT- tai AIKAT-luokat ylittää `combufsize`n (HK lisää 14 vain neljälle luokalle)? / Can HK SEURAT/AIKAT exceed `combufsize`? |

## 12. Hyväksyttävät päätökset / Decisions needing approval

1. Lupa toteuttaa ensimmäinen erä (luku 7): uudet tiedostot `TPsource/V52/Tests/`- ja
   `TPsource/V52/VS/Tests/`-hakemistoihin; tuotantokoodi ei muutu. / Approval to
   implement batch 1 (new files under `Tests/` and `VS/Tests/`; no production
   changes).
2. Lupa muuttaa CI:tä (1.1, TpTest `console`-työhön) erikseen. / Separate approval
   for the CI change.
3. Uusi rekisterin todennusarvo "toistettu testikehyksessä" (luku 1) vai vain
   "vahvistettu ajamalla"? / New register value "reproduced in test harness"?
4. Tunnetut viat odotetusti epäonnistuviksi CI:ssä (vihreä CI) vai punaisiksi? /
   Known defects as expected failures (green CI) or red?
5. Ajurin kieli: Python vai PowerShell? / Runner language: Python or PowerShell?
6. `Tests/run.sh`: poisto vai korjaus puhtaille yksiköille? / Drop or fix?
7. Myöhemmät tuotantokoodin testikoukut (H-007, H-009) ja tarkistusfunktioiden
   irrotus (H-027, H-032): sallitaanko, ja omassa haarassaan? / Later production test
   hooks and extractions: allowed, and in their own branch?
8. Haara: toteutetaanko testit tällä dokumentaatiohaaralla vai omalla haarallaan
   (`suunnitelma.md`: korjaukset ja dokumentit erillään)? / Branch for the tests.

### 12.1 Päätökset / Decisions (2026-11-24)

Käyttäjä hyväksyi suunnitelman rajauksin / the user approved the plan with limits:
testit omalle haaralle `testit/vikojen-todentaminen`; ajuri Pythonilla; uusi
todennustaso `toistettu testikehyksessä`; ei tuotantokoodin muutoksia, testikoukkuja
eikä CI-muutoksia; ensin vain H-025, H-040 ja H-042; testikoodia ei commitoida ilman
erillistä hyväksyntää. Kohdat 4 (CI:n odotetut epäonnistumiset), 6 (`run.sh`) ja 7
(myöhemmät koukut) ovat yhä auki.

Tests on their own branch; Python runner; new verification level; no production
changes, hooks or CI changes; H-025, H-040 and H-042 first; test code not committed
without separate approval. Items 4, 6 and 7 remain open.

## 13. Toteutus, vaihe 1 / Implementation, stage 1 (H-025, H-040, H-042)

### 13.1 Mitä tehtiin / What was built

Kaikki uudet tiedostot ovat hakemistoissa `TPsource/V52/Tests/` ja
`TPsource/V52/VS/Tests/`; tuotantokoodia ei muutettu (`production_dirty = false`
ajurin tuloksissa). Tiedostot ovat ASCII-muotoisia, LF-rivinvaihdoin.

All new files are under `Tests/` and `VS/Tests/`; no production code changed.
Files are ASCII with LF line endings.

| Tiedosto / File | Tehtävä / Purpose |
|---|---|
| `Tests/run_core_tests.py` | Ajuri: yksi prosessi ja oma väliaikaishakemisto per testi, aikakatkaisu ja prosessipuun lopetus, tarkka testinimi (`--list-test-cases`), luokittelu PASS/XFAIL/XPASS/FAIL/SKIP, `results.json` + `results.md` + lokit / runner |
| `Tests/test_run_core_tests.py` | Ajurin luokittelun itsetestit (20 kpl): kaatuminen, aikakatkaisu, väärä mekanismi, TPSKIP + virhe jne. eivät koskaan anna XFAIL:ia / self-tests: crash, timeout, wrong mechanism never give XFAIL |
| `Tests/core/manifest.json` | Testiluettelo: ohjelma, testitapaus, laji, havainto, mekanismi, odotettu tulos, aineisto, aikaraja / test list |
| `Tests/core/CoreMain.cpp` | doctest-main, virhedialogit pois, `tp_build_info` / main, dialogs off, build facts |
| `Tests/core/VCoreTests.cpp` | H-025a, H-025b, H-042 + kontrolli / control |
| `Tests/core/VAddfrecTests.cpp` | H-040 + kontrolli; sisällyttää `Juk/vdat.cpp`:n (staattinen `addfrec`) / includes `Juk/vdat.cpp` |
| `Tests/core/HkCoreTests.cpp` | TpCoreHk:n savutesti / smoke test |
| `Tests/fixtures/TpCore.{h,cpp}` | Hiekkalaatikkotarkistus, TPRESULT-protokolla, lukkokoetin, tyngät `writeerror`/`writeerror_w`/`writeerrorOn` ja `perr`/`wperr` / sandbox check, result protocol, lock probe, stubs |
| `Tests/fixtures/VFixture.{h,cpp}` | Viestin käynnistyksen osat: lukot, `asetaOletusarvot`, tietuekoot / relay start-up parts |
| `Tests/data/juk/h042_*.xml` | Synteettinen `KilpSrj.xml`, rajoissa ja yli / synthetic, in and over limits |
| `VS/Tests/TpCore.sln`, `TpCoreJuk.vcxproj`, `TpCoreHk.vcxproj` | Release/Win32, v145; tuotantolähteet ja -asetukset JukMaali/HkMaali Releasesta / production sources and settings |

Tahalliset erot toimitettuun ohjelmaan / deliberate differences from the shipped
program: pääohjelmatiedosto (`laskenta.cpp`, `HkMain.cpp`) pois; `Juk/vdat.cpp`
käännetään testitiedoston sisällä; `tputilv2`:n `WRITEERR.obj` ja `perr.obj`
korvataan tallentavilla tyngillä; fixture asettaa tilan käsin `aloitus`-funktion
sijaan. Riippumaton tarkastaja vertasi käännös- ja linkityskomennot tlog-tiedostoista:
tuotantotiedostojen liput ovat identtiset.

Huomio: `Juk/VDef.h:29` määrittelee `__fastcall`in tyhjäksi, mikä rikkoo sen jälkeen
sisällytetyt C++-standardiotsikot (C2953). Testitiedostoissa standardi- ja
apuotsikot sisällytetään siksi ennen tuotanto-otsikoita. / `Juk/VDef.h:29` defines
`__fastcall` away, breaking standard headers included after it; test files include
standard headers first.

### 13.2 Luokittelu / Classification

| Tila / Status | Merkitys / Meaning | Todennustaso / Verification level |
|---|---|---|
| PASS | Kontrolli: kaikki tarkistukset läpi / control passed | – |
| XFAIL | Vikatesti tulosti täsmälleen yhden TPRESULT-rivin oikealla havainnolla ja mekanismilla, `observed=1`, kaikki esiehdot läpi, poistumiskoodi 0 / exactly one result line, right finding and mechanism, preconditions passed, exit 0 | toistettu testikehyksessä / reproduced in test harness |
| XPASS | Kuten yllä, mutta `observed=0`: mekanismia ei näkynyt / mechanism not seen | ei toistunut; havainto tarkastettava / not reproduced; review the finding |
| FAIL | Aikakatkaisu, kaatuminen, epäonnistunut esiehto, puuttuva tai kahdennettu tulosrivi, väärä mekanismi, testiä ei löydy tarkalla nimellä, ohjelmaa ei ole käännetty / timeout, crash, failed precondition, missing or duplicate result, wrong mechanism, inexact name, not built | ei ratkennut / undecided |
| SKIP | Testi ilmoitti TPSKIP eikä mikään epäonnistunut, tai `--allow-missing` / reported TPSKIP and nothing failed | ei ratkennut / undecided |

Kaatumista tai aikakatkaisua ei koskaan lueta vian näytöksi. Ajurin paluuarvo on 1,
jos yksikin tulos on FAIL tai XPASS.

A crash or a timeout is never evidence of a defect. The runner exits 1 on any FAIL
or XPASS.

### 13.3 Tulokset / Results

| | |
|---|---|
| Git-revisio / revision | `f5d793b` (haara / branch `testit/vikojen-todentaminen`); testikoodi commitoimatta, tunnistettu sha256-tiivisteillä `results.json`:ssa (`harness_sha256`) / test code uncommitted, identified by sha256 |
| Tuotantokoodi / production code | muuttamaton / unchanged (`production_dirty = false`) |
| Käännösympäristö / build | VS 18 Build Tools, MSVC 19.52.37029 (v145), Release/Win32, `/J` (`char_unsigned=1`), `/GL`, `/MT`, `/O2`, `/GS`; JukMaali Release -määritykset (`MAXOSUUSLUKU=30`, `VAIKALUKU=12`, `MAXSARJALUKU=250`); `tputilv2.lib`, `dbboxm.lib` (Release, sha256 kirjattu / recorded) |
| doctest | 2.4.11 |
| Ympäristö / host | Windows 11 10.0.26100, Python 3.13 |
| Komento / command | `python -I TPsource/V52/Tests/run_core_tests.py`; testi: `VS\Tests\Release\TpCoreJuk.exe "--test-case=<nimi>" --no-colors=true --no-version=true` (cwd = hiekkalaatikko / sandbox) |
| Ajot / runs | 2026-11-24, 5 täyttä ajoa ennen korjauksia ja 4 korjausten jälkeen; kaikki PASS=5, XFAIL=4, poistumiskoodit 0x00000000 / 5 runs before and 4 after fixes, identical |

| ID | Aineisto / Data | Odotettu oikea / Expected correct | Todellinen / Actual | Tila / Status | Todennus / Verification |
|---|---|---|---|---|---|
| H-025a | ei tiedostoja; `n_os` 4 ja 30 / no files | `combufsize >= 14 + class_len[i]` kaikille luokille / for all classes | n_os 4: `combufsize` 204, suurin luokka 13 (EMITVA) 204 → 218 ei mahdu; n_os 30: 244, luokka 9 (YHTEISLAHTOT) 244 → 258 ei mahdu / does not fit | XFAIL | toistettu testikehyksessä |
| H-025b | testin luoma levyjono / queue file created by the test (`COMFILE4.DAT`, `COMFILE.DAT`) | jokaisessa paikassa koko paketti (otsake + len) / whole packet per slot | molemmissa: paikan pituus = `combufsize`, 14 tavua jää tallentamatta, tiedosto 3 paikkaa (612 / 732 tavua), paikan 1 lopun kohdalla alkaa paikka 2 / 14 bytes never stored | XFAIL | toistettu testikehyksessä |
| H-040-control | testin luoma `KILP.DAT` | koetin: vapaa/varattu/vapaa; tuotannon `delfrec` vapauttaa lukon / production `delfrec` releases | kuten odotettu / as expected | PASS | – |
| H-040 | testin luoma `KILP.DAT` | `addfrec`in jälkeen toinen säie saa `kilpdat_CriticalSection`in / another thread gets the lock | toinen säie ei saa lukkoa; omistaja = kutsuja; RecursionCount 2 / not free, owned by caller | XFAIL | toistettu testikehyksessä |
| H-042-control | `h042_in_limit.xml` (sha256 `d299275b…`) | `luesarjat` 0; 30/30/12 sellaisinaan | kuten odotettu | PASS | – |
| H-042 | `h042_over_limit.xml` (sha256 `da3e60a5…`): Legs 31, LegCount 31, Intermediaries 13 | hylätään tai rajataan / rejected or clamped | `luesarjat` 0, ei virheilmoitusta, `n_os` 31, `n_os_akt` 31, `valuku` 13; johdettu `kilprecsize` 7330 > `KILPRECSIZE` 6918 / derived record size exceeds buffer | XFAIL | toistettu testikehyksessä |
| juk/hk-build-info, hk-smoke | – | käännöstiedot, HK-ydin käynnistyy hiekkalaatikossa / build facts, HK core runs | kuten odotettu | PASS | – |

### 13.4 Mitä tulokset eivät osoita / What the results do not show

- **H-025:** lähetetyn paketin korruptoitunutta loppua (luku `Tp/TpComY.cpp:1470`,
  tarkistussumma `:1507-1508`) ei toistettu. Vika koskee vain paketteja, joiden pituus
  > `combufsize` − 14: oletuksin EMITVA, joka lähetetään vain LÄHEMVA-asetuksella
  (`Juk/VInit.cpp:2017-2020`), tai YHTEISLAHTOT, kun `n_os` ≥ 24. Häntä jää
  kirjoittamatta, ei ylikirjoitu. `maxyhteys` > 1 -tapausta ei testattu. Muistijonon
  ylivuoto ei ole viestissä saavutettavissa (`levypusk` on aina 1).
  / The corrupted send is not reproduced; only packets longer than `combufsize` − 14
  are affected (EMITVA with LÄHEMVA, or YHTEISLAHTOT at `n_os` ≥ 24); the tail is never
  written, not overwritten; `maxyhteys` > 1 not tested; the memory queue is
  unreachable in relay builds.
- **H-040:** ohjelman jumiutumista ei toistettu. Se vaatii lisäyksen tai poiston
  kahdesta eri säikeestä (pääsäie ja `tarkcom`); ilman verkkoa (`comfl` 0) kaikki
  lisäykset tehdään pääsäikeessä eikä mikään jumiudu, rekursiolaskuri vain kasvaa.
  `vdat.cpp` käännetään testitiedoston sisällä; liput ovat samat, koodin generointia
  ei verrattu. / The freeze is not reproduced; it needs adds/deletes from two
  threads. Code generation of `addfrec` in the test was not compared with the product.
- **H-042:** puskurien ylivuotoa (`Juk/vdat.cpp:145`, `:153`, `:181`;
  `Juk/vSarjat.cpp:60-70`) ei ajettu (suunnitelman vaihe b). ViestiWinin osittainen
  rajaus (`ViestiWin/UnitKilpMaaritys.cpp:271-276`) ja sen XML-painikkeen polku
  (`:316`) eivät ole testissä mukana. / The overflow itself is not run; ViestiWin's
  partial clamp and its XML button path are not covered.
- Mikään tulos ei ole "vahvistettu ajamalla": varsinaista ohjelmaa ei ajettu. / No
  result is "confirmed by running": the actual program was not run.

### 13.5 Tarkastukset ja korjaukset / Reviews and fixes

| Tarkastaja / Reviewer | Tulos / Result | Korjattu / Fixed |
|---|---|---|
| `build-dependency-analyst` (Sonnet) | Tuotantolähteet ja -asetukset koottiin, koelinkitys onnistui / sources and settings, probe link OK | – |
| `defect-concurrency-auditor` | Kaikki neljä vikatestiä päteviä, kolme rajauksin; ei vääriä positiivisia; oikea toteutus antaisi `observed=0` / all valid, three with caveats; no false positives | H-040-kontrolli käyttää tuotannon `delfrec`iä (`putrec_1` ei ole käytössä tuotannossa); H-025b myös n_os 4 / EMITVA ja `fsize == 3 × stride`; H-042 raportoi johdetun tietuekoon; koettimen odotus ilman aikarajaa (ei pinomuuttujan käyttöä vapautuksen jälkeen) / control uses production `delfrec`; H-025b two configs; H-042 reports derived size; probe wait fixed |
| `independent-verifier` | Projektit vastaavat tuotantoa; eristys, toistettavuus (3 ajoa) ja merkistö OK; kolme ajurivikaa / projects faithful; isolation, reproducibility, encoding OK; three runner defects | TPINFO-rivit katosivat (käännöstiedot tyhjiä) → korjattu; TPSKIP peitti epäonnistumisen → virhe tarkistetaan ensin; aikakatkaisu ei pitänyt aliprosessin kanssa → `taskkill /T /F` ja rajattu odotus; lisäksi: tarkka nimi `--list-test-cases`:lla, käännöksen puuttuminen = FAIL, yksittäinen virhe ei kaada ajoa, `perr`/`wperr`-tyngät, harness-tiedostojen ja kirjastojen sha256, doctest-versio, ajon yhteenveto, ohjelma vs. lähteiden aikaleima / TPINFO lost, SKIP masked FAIL, timeout with child process; plus exact names, missing build = FAIL, per-test error isolation, perr stubs, hashes, versions |

Avoimet / open: `SEM_NOGPFAULTERRORBOX` ja `/GS`-kaatumisen (0xC0000409) dialogi
testataan, kun ensimmäinen kaatuva testi (H-041 b) lisätään; verkkohiljaisuus on
todettu vain koodia lukemalla. / Fast-fail dialog behaviour is checked with the first
crashing test (H-041 b); network silence checked by reading code only.

### 13.6 Ajaminen / How to run

```
msbuild TPsource\V52\VS\Libs\tputilv2.sln /p:Configuration=Release /p:Platform=Win32
msbuild TPsource\V52\VS\Tests\TpCore.sln  /p:Configuration=Release /p:Platform=Win32
python -I TPsource\V52\Tests\test_run_core_tests.py
python -I TPsource\V52\Tests\run_core_tests.py [--out DIR] [--filter H-040] [--keep]
```

Tulokset oletuksena `TPsource/V52/Tests/results/<aika>/` (git ohittaa). Linkitys
kestää noin 2 minuuttia (`/GL` + LTCG). / Results go to `Tests/results/<time>/`
(ignored by git); linking takes about 2 minutes.

### 13.7 Seuraavat viisi testiä (odottavat lupaa) / Next five tests (awaiting approval)

Luvun 7 järjestyksessä / in the order of section 7: H-041 (a, b1, b3), H-031, H-008,
H-024, H-027 (a, b). Ne vaativat `HkFixture`n (`wmain`in oletukset, `HkConsole/HkMain.cpp:185-201`)
ja ensimmäisen kaatuvan testin käsittelyn. / They need `HkFixture` and the first
crashing test.

## 14. Toteutus, vaihe 2 / Implementation, stage 2 (H-041, H-031, H-008, H-024, H-027)

Vaihe 1 commitoitiin `79d9957` ja vaihe 2 sen jälkeen samalle haaralle
`testit/vikojen-todentaminen`. Tuotantokoodi ja CI ovat muuttumattomat (`git diff fc03b67` ilman
testihakemistoja: ei eroja).

Stage 1 was committed as `79d9957` and stage 2 after it on the same branch.
Production code and CI are unchanged.

### 14.1 Lisäykset / Additions

| Tiedosto / File | Sisältö / Content |
|---|---|
| `Tests/core/HkCoreTests.cpp` | H-041a, H-041b3, H-041b2 (vain ASan-käännöksessä / ASan build only), H-031 + kontrolli |
| `Tests/core/HkLockTests.cpp` | H-008 + kaksi kontrollia (varhainen virhe, kelvollinen sanoma) / two controls |
| `Tests/core/VComTests.cpp` | H-024 + kontrolli (julkinen `tarkcom`, ei `#include`a / public `tarkcom`, no include) |
| `Tests/core/TcpClientTests.cpp` | H-027 kontrolli, H-027a, H-027b (julkinen `tiedonsiirtoTCP`) |
| `Tests/fixtures/HkFixture.{h,cpp}` | HK-käynnistyksen osat: lukot, `wmain`in oletukset, `luekentat`, tyhjä `KILP.DAT` / HK start-up parts |
| `Tests/fixtures/TpCore.{h,cpp}` | `runOnThread` (aikaraja, kahvat pidetään auki → säietunnus ei kierrä), säietunnisteet, lukon omistaja, koetin hylkää omistajasäikeen / controlled threads, owner, probe guard |
| `Tests/fixtures/TpCrash.cpp` | `armCrash`/`disarmCrash`, ASan-raportin takaisinkutsu, vektoroitu poikkeuskäsittelijä + DbgHelp (symbolit vain exe-hakemistosta) / crash evidence |
| `Tests/fixtures/TpNet.cpp` | Vain 127.0.0.1: vapaa portti, varattu hylkäävä portti, yritykset laskeva kuuntelija (`SO_CONDITIONAL_ACCEPT`), testin oma `connect` / loopback only |
| `Tests/core/CoreMain.cpp` | doctestin SEH- ja signaalikäsittely pois (kaatuminen päättää prosessin oikealla koodilla), puskuroimaton tuloste, ASan-takaisinkutsu, `TerminateProcess`-loppu estyneille säikeille / doctest crash handling off |
| `Tests/run_core_tests.py` | `termination: asan / exception`: XFAIL vain, jos TPARM, käsittelijän TPRESULT, ajurin itse jäsentämä todiste (ASan-raportti stderr:stä; TPCRASH + symbolin ratkaisu prosessin ulkopuolella DbgHelpillä) ja odotettu poistumiskoodi täsmäävät; selvinnyt kaatumistesti vain järjestyksessä TPARM < TPDISARM < TPRESULT ja `observed=0` → XPASS; XPASS-huomio stdout/stderr/`results.md`, paluuarvo 3; ympäristöstä poistetaan `ASAN_*`, `*SAN_OPTIONS`, `_NT_SYMBOL_PATH`, `TP_*`; revisiot `production_rev`, `tests_rev`, likaisuusliput, PDB-, ASan-DLL- ja `doctest.h`-tiivisteet; vanhentuneisuus ohjelmakohtaisesti / runner extensions |
| `Tests/test_run_core_tests.py` | 43 itsetestiä (oli 20) / self-tests (was 20) |
| `VS/Tests/TpCore*.vcxproj`, `TpCore.sln`, `VS/Tests/.gitignore` | `Asan|Win32` = Release + `EnableASAN`, `/Zi`, ASan-DLL:n kopio; tulosteet git-ohitettu / Asan configuration |

AddressSanitizer (VS 18, MSVC 14.52, x86) toimii `/MT /GL /GS`:n kanssa ja
instrumentoimattomien `tputilv2.lib`/`dbboxm.lib`:n kanssa. Se näkee
instrumentoidun koodin `memset`/`memmove`-kirjoitukset, ei ytimen `ReadFile`-
kirjoitusta (`getrec`), jonka vain `/GS` havaitsee palatessa (0xC0000409, ei
täsmällinen). Siksi H-041:n ylitys näytetään kirjoituspolulla. VS 2022:n
v143-ASan-ajonaikaista ei ole tällä koneella [tuntematon CI:lle].

AddressSanitizer works with the shipped settings; it sees instrumented writes,
not the kernel `ReadFile` write, so H-041 is shown on the write path.

### 14.2 Tulokset / Results

| | |
|---|---|
| Revisiot / revisions | tuotanto / production `fc03b67` (muuttamaton / unchanged), testit / tests `79d9957` + vaihe 2 ajettuna ennen sen commitia, tunnistettu tiivisteillä `results.json`:ssa / stage 2 run before its commit, identified by hashes |
| Käännös / build | MSVC 19.52.37029 (v145), Release/Win32 `/J /GL /MT /O2 /GS`; TpCoreHk Asan: sama + `/fsanitize=address /Zi`; ASan-DLL 14.52.37025 |
| Komento / command | `python -I TPsource/V52/Tests/run_core_tests.py` |
| Ajot / runs | 2026-11-24, 3 + 1 virallinen ajo korjausten jälkeen: kaikki PASS=11, XFAIL=12, FAIL 0, XPASS 0, paluuarvo 0; kesto ~23 s |
| Aineisto / data | testien itse luomat tiedostot hiekkalaatikossa; H-042: `data/juk/h042_*.xml` (sha256 `d299275b…`, `da3e60a5…`) |

| ID | Tila / Status | Todellinen / Actual |
|---|---|---|
| H-041a | XFAIL | n_pv 5, valuku 250: `kilprecsize0` 360, `kilppvtpsize0` **152**, `vatpsize` 8 → `kilprecsize` **11 200** > `KILPBUFSIZE` 10 000 |
| H-041b3 | XFAIL | tuotannon `pack` kirjoitti tasan [0, 11 200), 1 200 tavua kohdasta 10 000 eteenpäin / wrote exactly [0, 11 200) |
| H-041b2 | XFAIL | ASan: `stack-buffer-overflow`, `WRITE of size 8`, pino `vatp::pack` (`Hk/HkDat.cpp:221`) ← `kilppvtp::pack` ← `kilptietue::pack` ← `kilptietue::write` (`:586`); muuttuja `buf` [16, 10016) eli 10 000 tavua; poistumiskoodi 77 |
| H-031-control | PASS | sallittu ensimmäinen vaihto: `k_pv` 1, lippu nollattu, lukot vapaat / allowed change proceeds |
| H-031 | XFAIL | evätty vaihto (r1 = 1) jättää lipun; myöhempi sallittu kutsu: r2 = 0, `k_pv` 0, `lah_alku` 0, ei viestejä, lippu yhä päällä |
| H-008-control, -control-valid | PASS | koetin vapaa/varattu/vapaa; tuntematon numero palaa ennen lukkoa; kelvollinen sanoma ottaa ja vapauttaa lukon / valid message releases the lock |
| H-008 | XFAIL | sanoma "  12 1 3", `lsak` 0: virhe -3 -viesti, työsäie palasi; toinen säie ei saa `tall_CS`:ää, omistaja = päättynyt työsäie |
| H-024-control | PASS | väärän numeron TARKT kulkee virhepolun kautta, kaksi viestiä, lukko vapaa |
| H-024 | XFAIL | TARKT 'I' K-osuudelle kuuluttajatilassa: paketti käsitelty, `tarkcom`-säie päättyi normaalisti, `tall_CS` jää sen omistukseen; osuuden tila yhä K |
| H-027-control | PASS | kuuntelija näki hylätyn (`WSAECONNREFUSED` 10061) ja hyväksytyn yrityksen |
| H-027a | XFAIL | (i) hylkäysvaiheessa SYN-paketteja 5 (yksi `connect`), `hComm[0]` 1, `TCP_on[0]` 127.0.0.1, `comopen` 0; (ii) säie päättyi ~2 000 ms:ssä, hyväksyvässä vaiheessa 15 s:ssa 0 yritystä |
| H-027b | XFAIL | lukuvirhe osoitteessa 0x0000000C asiakassäikeessä, käsittelijän symboli `reconnectTCP`; ajuri ratkaisi saman RVA:n prosessin ulkopuolella `reconnectTCP`:ksi; poistumiskoodi 0xC0000005 |

### 14.3 Mitä kukin testi todisti ja mitä ei / What each test proved and did not

| ID | Todisti / Proved | Ei todistanut / Did not prove |
|---|---|---|
| H-041 | Toimitetuilla määrityksillä hyväksytty kokoonpano (n_pv 5, valuku 250) tuottaa 11 200 tavun tietueen; tuotannon `kilptietue::write` kirjoittaa oman 10 000 tavun pinopuskurinsa yli (ASan, b2). b3 on välillinen näyttö (`pack`in kirjoitusmäärä). / accepted configuration gives 11 200 bytes; `write` overruns its own stack buffer (ASan) | Toimitetun HkMaali.exe:n tai HkKisaWinin (bcc32) kaatumista; luku-polun (`read`, `ReadFile`) ylitystä; käynnistyspolkua `luesarjat`in kautta (a kutsuu `luekentat`ia suoraan); onko valuku ≥ 221 käytössä oikeissa kilpailuissa / shipped program, read path, start-up path, real-world use |
| H-031 | Evätty vaihto jättää `vaiheenvaihto`-lipun, ja myöhempi sallittu vaihto ei tee mitään; kontrolli osoittaa, että sallittu vaihto toimii samassa fixturessa / refused change blocks a later allowed one | Emit-, merkki- ja taustakäsittelyn pysähtymistä (`Hk/HkIV.cpp:796`, `:3714`, `Hk/HkEmit.cpp:3873`); konsolivalikon polkua; GUI ei kuulu (vain `_CONSOLE`) / background stop, menu path |
| H-008 | Virhepolku -3 palaa `tall_CS` varattuna; lukko jää päättyneelle säikeelle; kelvollinen polku vapauttaa sen / -3 path leaves the lock held | Jumiutumista seuraavassa tallennuksessa; laitesanoman (`Tp/TpLaitteet.cpp:1670`) tai verkkopaketin reittiä; `sak_laji` ≠ 0 -polkuja / the freeze, device or network route |
| H-024 | Kuuluttajapolku palaa `tall_CS` varattuna `tarkcom`-säikeessä, joka sitten päättyy; kontrollin virhepolku vapauttaa / announcer path leaves the lock held | Ohjelman jumiutumista (toinen `tall_CS`:n käyttäjä); oikeaa verkkoyhteyttä (paketti syötettiin jonoon) / freeze, real network |
| H-027 | (i) yksi yritys tehtiin ja epäonnistui, (ii) säie päättyy eikä yritä uudelleen, vaikka kuuntelija hyväksyy 15 s; TCPXML: NULL-osoittimen luku `reconnectTCP`:ssä vapautetun yhteyden takia / one failed attempt and no retry; TCPXML NULL read | Palvelinpuolen avausvirheen ohitusta (palvelintila sitoo `INADDR_ANY`, ei ajettu); HK/viesti-ohjelman käyttäytymistä yhteyden katketessa käytössä; muita TCP-tyyppejä kuin comtpTCP ja comtpTCPXML / server-side open error, live behaviour, other types |

Yhteistä: mikään tulos ei ole "vahvistettu ajamalla"; testit ajavat muuttamatonta
tuotantokoodia toimitetuin käännösasetuksin, mutta tila asetetaan käsin. / No result
is "confirmed by running"; the state is set by hand.

### 14.4 Tarkastukset ja korjaukset / Reviews and fixes

| Agentti / Agent | Tehtävä / Task | Tulos ja korjaukset / Result and fixes |
|---|---|---|
| `build-dependency-analyst` (Sonnet) | ASanin soveltuvuus ja Asan-konfiguraatio / ASan feasibility | Toimii; havaitsee `memmove`n, ei `ReadFile`ä; raportin muoto, takaisinkutsu, poistumiskoodit, ei dialogeja |
| `defect-concurrency-auditor` | Fixture-reseptit viidelle testille / fixture recipes | Korjasi suunnitelman: `kilppvtpsize0` 152 (ei 154) → 11 200; `sak_laji`-oletus 2; H-024 ja H-027 julkisten funktioiden kautta ilman `#include`a; H-031:n kontrolli |
| `architecture-analyst` | Ajurin laajennus, prosessien hallinta / runner design | Kaatumistodisteiden protokolla, `/GS` ei kelpaa todisteeksi, `TerminateProcess`, XPASS-näkyvyys, revisiot, ympäristön siivous |
| `independent-verifier` | Toteutuksen tarkastus / implementation review | Ei estäviä ongelmia; tuotanto muuttamaton; H-027b:n vika varmistettu myös disassemblystä. Korjattu: säietunnuksen kierto (kahvat auki, koetin hylkää omistajan, omistaja REQUIRE), H-008:n viestin ja H-024:n tilan REQUIRE:t, H-008:n kelvollisen polun kontrolli, H-027a:n esiehdot REQUIRE:iksi ja `WSAECONNREFUSED`-tarkistus, hylkäävä portti pidetään varattuna, H-041b2 selviytyessä XPASS eikä FAIL, rivijärjestyksen tarkistus, symbolin ratkaisu prosessin ulkopuolella, ympäristön etuliitesiivous, `git status` -sarakkeen säilytys, PDB- ja `doctest.h`-tiivisteet, ASan-ohjelman käännöstiedot |

Ensimmäisessä ajossa löytyneet testivirheet (korjattu ennen tarkastusta): yksi
hylätty `connect` näkyy kuuntelijalle useana SYN-pakettina, joten hylkäysvaiheen
laskuria ei käytetä sovellustason yritysten määränä; doctestin oma kaatumiskäsittely
peitti H-027b:n poistumiskoodin. / First-run test bugs fixed before review: SYN
retransmissions; doctest's own crash handler masked the exit code.

### 14.5 Avoimet / Open

- ASanin saatavuus `windows-latest`-koneella (v143) [tuntematon] (Q35).
- `LockCount`-koettimen tarvetta ei ollut; H-007 on seuraavassa erässä.
- H-027a:n oikean toiminnan (XPASS) tapauksessa tuotannon `TCPrcvTh` voi jäädä
  käyntiin prosessin lopussa [päätelty]. / In the XPASS case a production receive
  thread may outlive the test.
- Seuraava erä (luku 7): H-026, H-032, H-022, H-023, H-007, H-001, H-002;
  kerros (d) toimitetulle `HkMaali.exe`:lle (H-041 todellisessa ohjelmassa). /
  Next batch per section 7.

## 15. Toteutus, vaihe 3 / Implementation, stage 3 (H-026, H-032, H-022, H-023, H-007, H-001, H-002)

Vaihe 2 commitoitiin `cad2781` ja vaihe 3 sen jälkeen samalle haaralle. Tuotantokoodi ja CI ovat muuttumattomat (`fc03b67`).

Stage 2 was committed as `cad2781` and stage 3 after it.

### 15.1 Lisäykset / Additions

| Tiedosto / File | Sisältö / Content |
|---|---|
| `Tests/core/VRecvTests.cpp` | H-032 kontrolli, H-032a (suojakuvio vastaanottolohkon jälkeen), H-032b (ASan); H-026 kontrolli ja vikatesti (silmukka jää pyörimään) |
| `Tests/core/HkRecvTests.cpp` | H-022 kontrolli ja vikatesti, H-023a, H-023b + kaksi kontrollia, H-007 kontrolli (`LockCount`-odottajakoetin) ja vikatesti (pakotettu järjestys) |
| `Tests/core/VTulostusTests.cpp` | sisällyttää `Juk/VTulostus.cpp`:n (staattiset funktiot), joka poistettiin TpCoreJukin lähdelistasta; H-001a/b vain ASan-käännöksessä |
| `Tests/core/VSrjTests.cpp` | H-002 kontrolli, H-002a/b vain Release-käännöksessä (ASanin varjomuisti on osoitteessa 0x30000000-) |
| `Tests/data/juk/h002_*.srj` | synteettiset vanhan muodon sarjatiedostot (6, 7 ja 9 kenttää) / synthetic legacy series files |
| `Tests/fixtures/TpCore.*` | korvaajien atominen kutsulaskuri ja hiljainen tila tietylle säietunnisteelle (tuotannon silmukka, joka raportoi virheen joka kierroksella) / stub call counter, quiet mode |
| `Tests/run_core_tests.py` | estynyt säie vain havaitun vian osana (muuten FAIL); vika puuttuu ja säie päättyi → XPASS; ASan-todisteisiin `size` ja `summary_contains` |
| `VS/Tests/TpCoreJuk.vcxproj` | `Juk\VTulostus.cpp` pois listasta (testitiedosto sisällyttää sen); Asan-konfiguraatio käännetty ensimmäistä kertaa |

### 15.2 Tulokset / Results

| | |
|---|---|
| Revisiot / revisions | tuotanto / production `fc03b67` (muuttamaton / unchanged, `production_dirty` false); testit / tests `cad2781` + vaihe 3 (tiivisteet `results.json`) |
| Käännös / build | MSVC 19.52.37029 (v145), Release/Win32 `/J /GL /MT /O2 /GS`; TpCoreJuk ja TpCoreHk Asan = Release + `/fsanitize=address /Zi`, ASan-DLL 14.52.37025 |
| Ajot / runs | 2026-11-24, kolme ajoa tarkastuskorjausten jälkeen: PASS=19, XFAIL=23, FAIL 0, XPASS 0, identtiset; itsetestit 46 OK |
| Aineisto / data | testien luomat tiedostot ja paketit; `data/juk/h002_{six,seven,nine}_fields.srj` (sha256 `results.json`:ssa) |

| ID | Tila | Todellinen / Actual |
|---|---|---|
| H-032-control | PASS | rajoissa oleva TARKT-paketti viimeiseen paikkaan: suojakuvio koskematon |
| H-032a | XFAIL | luokka 7 (`class_len` 0), len 1000, paikka 49/51 × 258 tavua: lohkon jälkeen tasan 498 tavua paketin dataa (odotettu 498) |
| H-032b | XFAIL | ASan heap-buffer-overflow WRITE of size 1014, `TpComY.cpp:654 in addseur`, "0 bytes after 13158-byte region" (51 × 258) |
| H-026-control | PASS | kaksi kelvollista pakettia käsitelty, silmukka päättyy `comfl = 0`:n jälkeen |
| H-026 | XFAIL | `inbens` jää tuntemattomaan pakettiin, seuraava kelvollinen paketti käsittelemättä, silmukka pyörii `comfl = 0`:n jälkeen (merkitsee paikan uudelleen, 6 virhekutsua 300 ms:ssa); prosessi päätetään `TerminateProcess`illa (`TPEXIT|blocked_threads=1`) |
| H-022-control | PASS | oikean numeron (12) VAIN_TULOST tallentuu tietueelle 1, lukko vapaa |
| H-022 | XFAIL | numeron 13 paketti tallentuu tietueelle 1 (numero 12) täsmälleen kuten kontrolli; ei viestejä |
| H-023a | XFAIL | vaiheen 2 paketti (`d.v.k_pv` 1), vastaanottaja `k_pv` 0: tallentuu vaiheelle 1, vaihe 2 koskematon |
| H-023b-control-refused / -allowed | PASS | vaihtopyynnöttä toisen vaiheen yhteys hylätään; sallittu vaihto tehdään (`k_pv` 1) |
| H-023b | XFAIL | vaihto evätään (`vaihdavaihe`n viesti "...estävät vaihdon", `k_pv` 0), mutta `tark_alku` palauttaa false eli hyväksyy yhteyden |
| H-007-control | PASS | odottajalaskuri 0 / 0 / 1 / 0 (vapaa, varattu, odottaja, vapautettu) |
| H-007 | XFAIL | vastaanottosäie odotti `tall_CS`:ää (odottaja nähty, `inbens` 0), paikallinen muutos UUSI tallennettiin, paketin jälkeen seura = VANHA, tasoitus 600 |
| H-001a / H-001b | XFAIL | ASan stack-buffer-overflow WRITE of size 160 kohdissa `VTulostus.cpp:2228 in textjtulos` ja `:3981 in textosuus` |
| H-002-control | PASS | 7 kentän rivi jäsennetään (sarja H21, 1 osuus), varattu sivu koskematon |
| H-002a / H-002b | XFAIL | osoitteessa 0x30303030 tasan "4242\0" (5 tavua) / "TPH002\0" (7 tavua) |

**H-001:n todisteen tarkkuus / evidence specificity:** MSVC:n x86-ASan listaa
`textjtulos`-kehyksen muuttujat päällekkäisillä siirtymillä (kehyskuvaaja on väärä ja
yhteinen molemmille funktioille), joten muuttujan nimeä ei käytetä todisteena.
Riippumaton tarkastaja disassembloi Asan-ohjelman: kummassakin funktiossa on tasan yksi
0xA0 (160) tavun `memset`, jonka kohde on `stsk` (heti perään `stsk[0] = L'('` samaan
osoitteeseen); todellinen asettelu st [16,176), stjk [208,328), stsk [360,400), ln
[432,1032), ja ASanin raportoima ensimmäinen virheellinen tavu on siirtymässä 400 eli
`stsk`:n lopussa. / The ASan frame listing is wrong on MSVC x86; disassembly confirms
the single 160-byte memset targets stsk and the first bad byte is at stsk's end.

### 15.3 Mitä kukin testi todisti ja mitä ei / What each test proved and did not

| ID | Todisti / Proved | Ei todistanut / Did not prove |
|---|---|---|
| H-032 | Luokan, jonka `class_len` on 0, paketti kopioidaan `len+14` tavun pituisena vastaanottopaikkaan ja yli tuotannon kokoisen lohkon (498 tavua; ASan: 1014 tavun kirjoitus) / unchecked length overruns the receive block | Itse UDP-tarkistuksen ohitusta `luepakettiUDP`:ssä (näytetään vain `class_len` 0:lla, testi kutsuu `addseur`ia suoraan); `class_len[pkgclass]`-lukua indeksillä ≥ 15; loopback-datagrammia; HK:ta / the receive predicate itself, OOB class_len read, loopback, HK |
| H-026 | Tuntematon luokka pysäyttää vastaanoton pysyvästi: jono ei etene, seuraava paketti jää käsittelemättä, silmukka ei noudata `comfl`ia / receiving stops permanently | Reittiä UDP:n kautta (luokka 7 pääsee jonoon H-032:n takia, vain koodin luvulla); vaikutusta käynnissä olevaan kilpailuun / UDP route, live effect |
| H-022 | VAIN_TULOST tallentuu tietueelle `dk` tarkistamatta paketin numeroa / no bib check | KILPPVT-versiota; kahden aseman saman tietuenumeron varausta (`Hk/HkDat.cpp:626-636`, vaatii kahden prosessin kerroksen) / KILPPVT, the two-station half |
| H-023 | (a) Tulos tallentuu vastaanottajan vaiheeseen, lähettäjän `d.v.k_pv` ohitetaan; (b) yhteys hyväksytään, vaikka vaiheen vaihto evättiin / (a) wrong stage, (b) link accepted | Seuraavien pakettien väärää soveltamista kahden aseman välillä; GUI-polkua (`cbHk/WinHk.cpp:2149-2160`, `:2231`) / multi-station sequence, GUI path |
| H-007 | Vastaanotto lukee tietueen ennen lukkoa ja kumoaa välissä tehdyn paikallisen muutoksen; järjestys pakotettu (odottaja nähty) / lost update with forced ordering | Muutoksen leviämistä vertaisille (`maxyhteys` 0); kuinka usein ikkuna esiintyy oikeassa ajossa; `LockCount`-koodaus on dokumentoimaton (kontrolli tarkistaa sen tällä koneella) / propagation, real-world frequency |
| H-001 | Kumpikin funktio kirjoittaa 160 tavua 40 tavun `stsk`-puskuriin (ASan + disassembly) / 160-byte write into 40-byte stsk | Release-käännöksen vaikutusta (vain C4789-varoitus; ei ajonaikaista havaintoa); julkista polkua `tulostasarja` (funktiot kutsuttiin suoraan) / Release effect, public path |
| H-002 | Vanhan muodon sarjatiedoston 6- ja 9-kenttäiset rivit kirjoittavat merkkijonon osoitteeseen 0x30303030 / write through the int value | Mitä tapahtuu, kun osoite ei ole varattu (kaatuminen tai muun muistin korruptio); ohjelman oma kirjoittaja tuottaa 8 kenttää eikä laukaise vikaa (`Juk/vSarjat.cpp:514`) / unreserved address effect; own writer does not trigger it |

Mikään tulos ei ole "vahvistettu ajamalla". / No result is "confirmed by running".

### 15.4 Tarkastukset / Reviews

| Agentti / Agent | Tehtävä / Task | Tulos / Result |
|---|---|---|
| `defect-concurrency-auditor` | Reseptit H-026, H-032, H-022, H-023, H-007 | Sisääntulot (`addseur`, `tark_alku`, julkinen `tarkcom`), paketit, fixturet, H-007:n pakotettu järjestys; kaksi kehikkomuutosta (hiljainen tila, estyneen säikeen XPASS) |
| `architecture-analyst` | H-001, H-002 ja HkMaali.exe:n ajosuunnitelma | `VTulostus.cpp`:n sisällytys; vanhan muodon rivimuodot; ASanin varjomuisti osoitteessa 0x30000000-; luku 16 |
| `build-dependency-analyst` (Sonnet) | TpCoreJuk Asan, `memset`-instrumentointi, 0x30300000 | Asan-käännös toimii; `memset` → `___asan_memset`; osoite vapaa Releasessa 10/10, ei ASanissa; kasan ylivuoto havaitaan, lohkon sisäinen ei. Agentin erillinen `cl`-ajo jätti `TPsource/V52/VS/V/vc140.pdb`:n, joka poistettiin |
| `independent-verifier` | Toteutuksen tarkastus / review | Kaikki testit osoittavat oikean mekanismin, ei vääriä XFAIL-tuloksia; H-001 vahvistettu disassemblystä. Korjattu: estynyt säie ilman havaittua vikaa on nyt FAIL (oli XPASS); H-026 vaatii elävän silmukan, jos säie jää käyntiin; H-032b:n todisteisiin koko 1014 ja `TpComY.cpp:654 in addseur` |

### 15.5 Avoimet / Open

- `class_len[pkgclass]`-luku indeksillä 15–255 (`Tp/TpComY.cpp:1001`) ilman testiä.
- H-022:n kahden aseman osuus ja KILPPVT-versio; H-023:n ja H-026/H-032:n GUI-polut.
- Kahden prosessin loopback-kerros (e) ja toimitetun ohjelman ajo (luku 16, odottaa
  hyväksyntää).

## 16. Toimitetun HkMaali.exe:n turvallinen ajo (suunnitelma, ei ajettu) / Safe run of the shipped HkMaali.exe (plan, not run)

Tätä ohjelmaa **ei ole ajettu**. Jokainen alla kuvattu ajo vaatii käyttäjän erillisen
hyväksynnän. Selvitys: `architecture-analyst` (koodin luku, ei ajoa), 2026-11-24,
revisio `f7fe553`.

This program has **not been run**. Each run below needs separate user approval.

### 16.1 Käynnistyksen sivuvaikutukset / Start-up side effects

| Asia / Item | Havainto / Finding | Näyttö / Evidence |
|---|---|---|
| Käynnistysjärjestys / start order | `initcon` (vaatii konsolin; epäonnistuessa `perr` → MessageBox; jos kursori ei ole 0,0: `FreeConsole` + `AllocConsole`) → koodisivu 850, Ctrl-käsittelijä, `WorkingDir = GetCurrentDirectory` → `aloitus` → `valikko` → `lopetus` | `HkConsole/HkMain.cpp:178-235`; `tputilv2/initcon.cpp:49-84`; `tputilv2/perr.cpp:51` [todennettu] |
| Luettavat tiedostot / files read | kaikki suhteellisia nimiä nykyhakemistossa: `peruscfg.cfg`, `LASKENTA.CFG` tai `CFG=` (voi olla absoluuttinen), argv; `KilpSrj.xml` → `KILP.SRJ`; `TLSKERR.LST`, `SARJAYHD.LST`; `KILP.DAT`; emit- ja ratatiedostot vain, jos `emitfl == 1` (ei oletuksena) / all relative to cwd | `Hk/HkInit.cpp:59`, `:332-345`, `:567-583`, `:2793`, `:3487-3496`, `:3590-3615`; `Hk/HkSrj.cpp:72-83`, `:510` [todennettu] |
| Ympäristö, rekisteri, moduulipolku / environment, registry, module path | ei konsolilähteissä (getenv, rekisteri, `GetModuleFileName`, `GetTempPath` vain GUI:ssa) / none in console sources | grep Hk, HkConsole, Tp, tputilv2, com, dbboxm [todennettu] |
| Kirjoitettavat tiedostot / files written | `KILP.DAT` (luodaan, jos puuttuu; otsake `closefile`issa), `DATA_ERR.TXT` vain datavirheissä, `LOKI1.LST` vain LOKI-asetuksella, `AJAT*.LST` vain ajanotossa, `COMFILE*.DAT` vain yhteyksillä; ei poistoja eikä uudelleennimeämisiä konsolipolulla | `Hk/HkInit.cpp:3487-3491`; `dbboxm/dbbox1.cpp:214-222`; `Tp/TpApu.cpp:876`, `:938` [todennettu; täydellisyys päätelty, vahvistettava Procmonilla] |
| Verkko / network | avataan vain asetuksilla: YHTEYS/COM, BRO, MONITORI, TCPLUKIJA/REGNLY, emiTag HTTP, FILETCP/TCPTULOS, AUTO+KOMENTO (`system()`); **ei mitään oletuksena**; käytössä UDP ja TCP-palvelin sitovat `INADDR_ANY` / none by default; when enabled, binds INADDR_ANY | `Hk/HkInit.cpp:1101-1160`, `:2135-2150`, `:3636-3666`; `com/WinUDP.cpp:291`, `:795`; `Hk/HkTulostus.cpp:7341-7345` [todennettu] |
| Tietokanta / database | ei konsolikäännöksessä (`kirjKilpSQL` tyhjä) / none in the console build | `HkConsole/HkCons0.cpp:141` [todennettu] |
| Laitteet ja tulostin / devices, printer | ei COM-porttia eikä lukijaa ilman asetuksia; `listfname` = "NUL" / no COM port or reader without config | `Hk/HkGlobals.cpp:282`; `Hk/HkInit.cpp:3409`, `:3565-3568` [todennettu] |
| Säikeet / threads | vain `taustaThr` (10 ms: kello, laskurit, `autotulostus` palaa, kun `lstf` NULL) / only the background clock thread | `Hk/HkInit.cpp:3708`; `Hk/HkIV.cpp:3711`; `Hk/HkTulostus.cpp:974-984` [todennettu] |
| Kehotteet / prompts | (a) aloitusvalinnat (ohitetaan argv `BOOT`), (b) `KilpSrj.xml` puuttuu → lopetus, (c) `KILP.DAT` puuttuu → luodaan, (d) vaihekysely vain, jos `n_pv_akt > 1`, (e) tuntematon parametri; kehotteet tyhjentävät näppäinpuskurin ensin; Enter hyväksyy oletuksen; `aani` 1 → äänimerkki / prompts | `Hk/HkInit.cpp:1052`, `:3060-3068`, `:3233-3236`, `:3311-3320`, `:3488`, `:2765-2768`; `Hk/HkSrj.cpp:79`; `tputilv2/wSelectxy.cpp:37`, `:70` [todennettu] |
| Lopetus / quitting | näppäimet `P`, `K` → `lopetus1` sulkee tiedostot, `cleanWSA`, `_exit(0)`; `CTRL_CLOSE_EVENT` → `lopetus`; poistumiskoodi 0 myös käynnistysvirheessä; ei lapsiprosesseja / keys P, K; exit 0 also on start-up failure | `HkConsole/HkMain.cpp:160-176`; `HkConsole/HkCons0.cpp:85-99`; `Hk/HkInit.cpp:3730-3832` [todennettu] |

### 16.2 Suoritusmenettely / Execution procedure

1. **Hyväksyntä / approval:** käyttäjä hyväksyy yksittäisen ajon (ajo 0 tai 1, alla).
2. **Ohjelma / binary:** `TPexe/Hk/V521/HkMaali.exe`; kirjataan sha256 ja PDB:n sha256
   sekä käännösaika ennen ajoa. Ei kopioida muualle kuin hiekkalaatikkoon. / record hashes.
3. **Hiekkalaatikko / sandbox:** uusi tyhjä hakemisto `%TEMP%`:n alla (ei repossa), cwd =
   hiekkalaatikko, vain suhteelliset nimet; ei `CFG=`-polkua. Synteettiset tiedostot
   luodaan testiohjelmalla (esim. HK:n `KilpSrj.xml` tuotannon kirjoittajalla
   `kirjEventXml`, `Hk/HkXml.cpp:2422`) – ei oikeaa aineistoa. / new empty dir, relative
   names, synthetic files only.
4. **Asetukset / configuration:** ei `peruscfg.cfg`- eikä `LASKENTA.CFG`-tiedostoa (ei
   yhteyksiä, laitteita, tulostusta, HTTP:tä eikä `KOMENTO`a); argv `BOOT`. /
   no config files, so no network, devices, printing or commands.
5. **Prosessinhallinta / process control:** Python (ctypes, ei kolmannen osapuolen
   kirjastoja): `CreateProcessW(CREATE_NEW_CONSOLE | CREATE_SUSPENDED)`, Job-objekti
   (`JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`, ei vaadi ylläpitäjää), sitten jatko. stdoutia ei
   ohjata (muuten `initcon` epäonnistuu). / job object, new console.
6. **Konsolin ohjaus / driving the console:** `AttachConsole(pid)`; odotetaan kehotteen
   tekstiä `ReadConsoleOutputCharacterW`:llä ennen jokaista näppäintä (kehote tyhjentää
   puskurin); `WriteConsoleInputW` (näppäin alas + ylös, oikea virtuaalikoodi);
   `FreeConsole`. / wait for prompt text, then send keys.
7. **Aikarajat / timeouts:** 10 s per kehote, 60 s yhteensä; ylityksessä `WM_CLOSE`
   konsoli-ikkunalle, 5 s jälkeen `TerminateJobObject`. / timeouts and kill.
8. **Verkon valvonta / network watch:** ajon aikana 100 ms välein
   `Get-NetTCPConnection`/`Get-NetUDPEndpoint -OwningProcess <pid>`: odotetaan tyhjää;
   mikä tahansa avoin päätepiste keskeyttää ajon. Valinnainen (vaatii ylläpitäjän):
   Windowsin palomuurin estosäännöt ohjelmapolulle sisään ja ulos (loopback sallittu
   oletuksena); **pakollinen** myöhemmille YHTEYS-ajoille `INADDR_ANY`-sidonnan vuoksi. /
   no endpoints expected; firewall rules mandatory for later link runs.
9. **Jälkitarkastus / afterwards:** hiekkalaatikon tiedostolista tiivisteineen;
   `git status --porcelain --ignored` muuttumaton; ajon aikana muuttuneet tiedostot
   `%TEMP%`:ssä, `%LOCALAPPDATA%\CrashDumps`:ssa ja WER-hakemistoissa; valinnaisesti
   Procmon (erillinen hyväksyntä). Käyttöjärjestelmä voi kirjoittaa prefetch-, WER- ja
   AppCompat-tietoja. / verify nothing outside the sandbox changed.
10. **Tulos / result:** kirjataan testaus.md:hen ja rekisteriin tasolla "vahvistettu
    ajamalla" vain, jos vika toistuu toimitetussa ohjelmassa havaintokohtaisella
    näytöllä. / "confirmed by running" only with finding-specific evidence.

### 16.3 Ehdotetut ensimmäiset ajot / Proposed first runs

| Ajo / Run | Sisältö / Content | Odotettu / Expected |
|---|---|---|
| 0 | Tyhjä hakemisto, ei argv:tä; Enter kehotteisiin (a), (b) / empty dir | ohjelma lopettaa (KilpSrj.xml puuttuu), poistumiskoodi 0, ei luotuja tiedostoja, ei verkkopäätepisteitä [päätelty] |
| 1 | argv `BOOT`, synteettinen `KilpSrj.xml` (1 sarja, 1 vaihe, vähän väliaikoja); (c) Enter; päävalikko; `P`, `K` / synthetic series file | vain `KILP.DAT` syntyy, siisti lopetus, ei verkkoa |
| 2 (myöhemmin / later) | H-041 toimitetussa ohjelmassa: n_pv 5, valuku 250, kilpailijan lisäys (`Hk/HkDat.cpp:583`) ja kontrolli valuku 10 / H-041 in the shipped binary | 0xC0000409 on vain yhdenmukainen vian kanssa (ei täsmällinen todiste, luku 14.1); WER voi näyttää dialogin tai kirjoittaa hiekkalaatikon ulkopuolelle → vaatii erillisen arvion |

H-031 toimitetussa ohjelmassa vaatii avoimen yhteyden lähettämättömine sanomineen eli
kahden prosessin loopback-kerroksen (e); ei ensimmäisissä ajoissa. / H-031 needs the
two-process loopback layer.

### 16.4 Avoimet / Open

- Konsolin toiminta AppContainerissa tai Windows Sandboxissa; sijoittuuko conhost
  Job-objektiin [tuntematon].
- WER:n käyttäytyminen toimitetun ohjelman pikakaatumisessa [tuntematon].
- Pienin kelvollinen HK:n `KilpSrj.xml` [tuntematon]; tuotetaan tuotannon kirjoittajalla.

## 17. HkMaali.exe:n koeajot 0 ja 1 / Black-box runs 0 and 1 (2026-11-24)

Käyttäjä hyväksyi ajot 0 ja 1 ehdolla, että verkkoyhteydet estetään ennakolta
todennettavalla menetelmällä. Ensimmäinen ajo 0 AppContainerissa keskeytyi
odottamattomaan tilaan (17.1–17.6); ajot 0 ja 1 tehtiin sen jälkeen palomuurieristyksellä
(17.7–17.10) ja tarkastettiin riippumattomasti (17.12). Yhtään H-havaintoa ei merkitty
"vahvistettu ajamalla". Työkalut: `TPsource/V52/Tests/blackbox/` (`run_hkmaali.py`,
`verify_log.py`, `isolation_probe.c`, `console_probe.c`), aineisto `Tests/data/hk/`,
aineiston tarkistus `Tests/core/HkDataTests.cpp`.

The user approved runs 0 and 1 provided network access is blocked beforehand in a
verifiable way. The first run 0 in an AppContainer stopped in an unexpected state
(17.1–17.6); runs 0 and 1 were then done under firewall isolation (17.7–17.10) and
reviewed independently (17.12). No H finding was marked "confirmed by running".

### 17.1 Ohjelma / Program

| | |
|---|---|
| Ohjelma / binary | `TPexe/Hk/V521/HkMaali.exe`, 1 156 096 tavua, SHA-256 `61034e5dbb18817985d6fe64343b5d1ee2aa9fbab91495bd43549eaa0d7d297d`; PDB `HkMaali.pdb` SHA-256 `68ec2cd7…`, RSDS-GUID {B0500861-3E83-4118-B0DA-E8E331F12614} täsmää / matches |
| Alkuperä / provenance | `build-dependency-analyst`: PE-aikaleima 2026-11-24 00:16:10, linker 14.52, x86 CUI, `/MT`; tuonnit vain järjestelmä-DLL:istä (ws2_32, winhttp, kernel32, user32, gdi32, winspool); ei advapi32:n rekisterikutsuja; uusi käännös samasta vcxproj:sta (Release) tuottaa identtiset otsakkeet ja tuonnit ja saman `.text`-koon. Päätelmä: uskollinen Release-käännös tuotantokoodista `fc03b67` [päätelty; määrityksiä ei ole tlogissa] |
| Revisio / revision | haara `testit/vikojen-todentaminen`, HEAD `c11231b`; tuotantokoodi muuttamaton |

### 17.2 Eristys / Isolation

Ylläpitäjän oikeuksia ei ole eikä Windows Sandboxia ole asennettu, joten palomuurisääntöä
tai virtuaalikonetta ei voitu käyttää. Käytettiin **AppContainer**ia ilman
kyvykkyyksiä (verkko estetty käyttöjärjestelmän WFP-kerroksessa) + Job-objekti
(`KILL_ON_JOB_CLOSE`, `DIE_ON_UNHANDLED_EXCEPTION`) + uusi hiekkalaatikkohakemisto
`%TEMP%\hkmaali_run_*\sandbox`, johon vain AppContainerin SID:llä on oikeudet. Ennen
ohjelmaa ajetaan samassa säiliössä `isolation_probe`; jos yksikin tarkistus epäonnistuu,
ohjelmaa ei käynnistetä (tapahtui ensimmäisellä yrityksellä konsolikahvojen vuoksi;
korjattu ennen ajoa 0).

No admin rights and no Windows Sandbox, so no firewall rule or VM. An AppContainer
without capabilities + job object + fresh sandbox was used; a probe in the same
container checks isolation first and blocks the run on any failure.

| Tarkistus (ajo 0:n alussa) / check | Tulos / result |
|---|---|
| Konsolisyöte / console input | näppäin vastaanotettu / key received |
| Kirjoitus hiekkalaatikkoon / write in sandbox | onnistuu / ok |
| Kirjoitus sen ulkopuolelle / write outside | estetty, virhe 5 / refused |
| TCP 127.0.0.1 (ohjaajan kuuntelija / driver listener) | epäonnistui 10060 / failed, 0 yhteyttä / 0 accepts |
| TCP 192.0.2.1:9 (TEST-NET-1, RFC 5737) | estetty heti, 10013 / refused at once (access denied) |
| Silmukkapoikkeus / loopback exemption | säiliön SID ei listalla (`CheckNetIsolation -s`); BFE ja palomuuri päällä / not exempt; firewall on |

Rajaus (riippumaton tarkastus): todennettu vain TCP:lle; UDP, kuuntelu, IPv6, DNS ja
järjestelmäpalvelujen välittämä liikenne (WinHTTP-välityspalvelimen haku) eivät olleet
mukana. / Limit: proven for TCP only; UDP, listening, IPv6, DNS and service-proxied
traffic were not tested.

### 17.3 Ajo 0 / Run 0

| | |
|---|---|
| Komento / command | `python -I Tests/blackbox/run_hkmaali.py --exe TPexe/Hk/V521/HkMaali.exe --probe isolation_probe.exe --script run0.json` |
| Ohjelman komento / program command line | `HkMaali.exe BOOT`, cwd = hiekkalaatikko / sandbox, ei asetustiedostoja / no config files |
| Aineisto / data | ei mitään (tyhjä hakemisto) / none |
| Odotettu / expected | kehote "Tiedosto KilpSrj.xml puuttuu" + Enter, "Sarjatietojen lukeminen ei onnistunut" + Enter, lopetus, ei tiedostoja |
| Toteutunut / actual | konsolipuskuri tyhjä koko ajan; ensimmäistä kehotetta ei näkynyt 10 s:ssa; ohjaaja lopetti Job-objektin (poistumiskoodi 0xDEAD0001); ei ikkunoita, ei verkkopäätepisteitä, ei uusia tiedostoja hiekkalaatikossa (vain `HkMaali.exe`, tiiviste ennallaan); muutokset hiekkalaatikon ulkopuolella vain MSRDC-jäljitystiedostoja, joita esiintyi myös koeohjelma-ajoissa (järjestelmän muu toiminta) |
| Prosessit / processes | ei jäänteitä (`tasklist`) / none left |

**Syy / cause (todennettu koeohjelmalla, ei HkMaalia ajamalla / verified with a probe,
not by running HkMaali):** samassa säiliössä ja konsolijärjestelyssä `console_probe`
osoitti, että `ReadConsoleOutputW` ja `ReadConsoleOutputA` epäonnistuvat virheellä 5
(pääsy estetty), kun taas kirjoitus, koko puskurin tyhjennys (9001 riviä 250 ms:ssa),
tila- ja otsikkokutsut onnistuvat. HkMaalin näyttörutiinit lukevat puskurin ennen
kirjoitusta ja ohittavat kirjoituksen hiljaa, jos luku epäonnistuu
(`tputilv2/viwrrectw.cpp`, `tputilv2/Viwrrect.cpp:61-64`, `tputilv2/virdrect.cpp:42-45`);
`writeerror_w` (`tputilv2/Writeerr.cpp:66-95`) jää silti odottamaan näppäintä. Tämä
selittää kaiken havaitun. Puskuri, jonka DACL salli säiliön SID:n, ei auttanut: luku
estyy silti (Windowsin rajoitus AppContainer-prosessille [päätelty]). / ReadConsoleOutput
is denied to the container process; the program's screen routines then skip every write
silently while the prompt still waits for a key. A buffer DACL granting the container
did not help.

**Tulkinta / interpretation:** kyse on eristysmenetelmän ja ohjelman näyttörutiinien
yhteisvaikutuksesta, ei HkMaalin virheestä normaalikäytössä; ajo ei todista eikä kumoa
mitään H-havaintoa. Havainto, että näyttörutiinit ohittavat virheet hiljaa, on
huomionarvoinen mutta ei tässä erässä uusi H-havainto (vain koetilanteessa). / An
interaction of the isolation method with the screen routines, not a defect in normal use;
the run proves or refutes no finding.

### 17.4 Mitä ajo 0 todisti ja mitä ei / What run 0 showed and did not

- Todisti / showed: ohjelma käynnistyy säiliössä ilman verkkoyhteyttä, ilman
  ikkunoita ja ilman tiedostomuutoksia, ja prosessi saadaan aina lopetettua; eristyksen
  esitarkastus toimii (esti kerran ajon, kun konsoli ei ollut kunnossa).
- Ei todistanut / did not show: puuttuvan sarjatiedoston käsittelyä, normaalia
  lopetusta eikä poistumiskoodia 0, koska kehotteita ei voitu nähdä.

### 17.5 Este ja vaihtoehdot / Blocker and options (päätös käyttäjälle / user decision)

AppContainer on ainoa tässä ympäristössä ilman ylläpitäjää todennettavissa oleva
verkkoeristys, mutta se estää konsolin luvun, jota HkMaali tarvitsee. Vaihtoehdot:

1. **Windowsin palomuurin estosäännöt** ohjelmapolulle (sisään ja ulos, kaikki
   protokollat) – vaatii ylläpitäjän; ohjelma ajetaan tavallisena prosessina
   hiekkalaatikkohakemistossa, ja sama koeohjelma todentaa eston saman polun alta ennen
   ajoa (kopioidaan koeohjelma samaan polkuun / sääntö koskee hakemistoa).
2. **Windows Sandbox** (`.wsb`, `<Networking>Disable</Networking>`) tai virtuaalikone
   ilman verkkokorttia – vaatii ominaisuuden käyttöönoton (ylläpitäjä).
3. **Ei verkkoeristystä** – ei hyväksyttävä käyttäjän ehtojen mukaan.

Ajo 1:tä ei tehdä ennen kuin jokin vaihtoehdoista 1–2 on hyväksytty ja ajo 0 toistettu
onnistuneesti.

### 17.6 Jäänteet / Leftovers

- AppContainer-profiili `tulospalvelu.c11.hkmaali` on rekisteröity (voidaan poistaa
  `DeleteAppContainerProfile`-kutsulla, kun sitä ei enää tarvita).
- `%TEMP%\hkmaali_run_*`-hakemistot (todisteet; sisältävät kopion ohjelmasta ja
  koeohjelmista) säilytetty tarkastusta varten.
- Ohjaajan ja koeohjelmien tulokset: `run.json`, `screens.log`, `probe_result.txt`,
  `console_probe.txt` (tilapäishakemisto, ei repossa).

### 17.7 Palomuurieristys (käyttäjän valitsema vaihtoehto 1) / Firewall isolation (option 1)

Käyttäjä loi järjestelmänvalvojana estosäännöt (sisään ja ulos, kaikki protokollat ja
profiilit) kiinteälle ohjelmapolulle
`%LOCALAPPDATA%\tulospalvelu-c11\hkmaali-fw\HkMaali.exe` (säännöt `TP C11 HkMaali block
in/out`). Ohjain tarkistaa ennen jokaista ajoa, että säännöt ovat käytössä, osoittavat
tähän polkuun ja että palomuuri on päällä kaikissa profiileissa.

The user created block rules (in and out, all protocols, all profiles) for a fixed
program path; the driver checks them before every run.

**Tiedostoeristys / file isolation:** matala eheystaso estää myös konsolin luvun
(`console_probe`: `ReadConsoleOutput` virhe 5), joten käytettiin **kirjoitusrajoitettua
tunnusta** (`CreateRestrictedToken`, `WRITE_RESTRICTED`, rajoittava SID `S-1-5-12`
RESTRICTED): kirjoitus vaatii lisäksi oikeuden tälle SID:lle, joka on vain
testihakemistolla; luku ei muutu, joten konsolin luku toimii. Ei vaadi ylläpitäjää.
Konsolin koko asetetaan valmiiksi 80×50 (ohjelman oma koonmuutos olisi muuten
epäonnistunut `perr`-ikkunaan). / Low integrity also denies console reads, so a
write-restricted token was used: writes need a grant to the restricting SID, which only
the sandbox has; reads (and the console) work.

**UDP-todennus / UDP verification:** UDP:n `sendto` palauttaa onnistumisen myös
estettynä (palomuuri pudottaa hiljaa), joten se ei ole todiste. Käyttäjä otti
järjestelmänvalvojana pudotettujen pakettien lokituksen käyttöön testin omaan
hakemistoon (`Set-NetFirewallProfile -LogBlocked True -LogFileName …`; palvelulle
`NT SERVICE\mpssvc` annettiin kirjoitusoikeus); palvelu suojaa lokin, joten käyttäjä
kopioi sen luettavaksi. Koeohjelma lähettää TCP- ja UDP-paketin satunnaisiin
kohdeportteihin osoitteeseen 192.0.2.1 (TEST-NET-1) ja kirjaa lähdeportit ja ajan;
`verify_log.py` hyväksyy vain DROP-merkinnät, joissa protokolla, kohdeosoite, kohde- ja
lähdeportti täsmäävät ja aika on ±3 min. Ohjelma käynnistyy vain, jos tällainen
todennus on hyväksytty enintään 2 h sitten samalla sääntöjoukolla; kunkin ajon oma
koepaketti todennetaan lisäksi jälkikäteen. Alkuperäiset lokitusasetukset (kaikissa
profiileissa `LogBlocked` False, `%systemroot%\system32\LogFiles\Firewall\pfirewall.log`)
palautettiin kokeen jälkeen ja tarkistettiin. / sendto success is no evidence; the drop
log, copied by the administrator, is matched on protocol, address, both ports and time;
the original logging settings were restored and checked.

| Tarkistus / check | Tulos / result |
|---|---|
| Säännöt / rules | 2 käytössä, Block, Any, ohjelma = kiinteä polku / enabled, Block, Any, program = fixed path |
| Koeohjelman tunnus / probe token | rajoitettu (restricted=1), eheys 0x2000 (keskitaso), ei ylläpitäjäjäsenyyttä / restricted, medium, not admin |
| Konsoli / console | näppäin vastaanotettu, luku ja kirjoitus toimivat / key received, read and write work |
| Kirjoitus ulkopuolelle / write outside | `%TEMP%`, profiili, Dokumentit, `LOCALAPPDATA`: kaikki estetty (virhe 5) / all refused |
| TCP 192.0.2.1 | `connect` estetty heti (10013); lokissa DROP TCP, portit täsmäävät / refused; DROP logged |
| UDP 192.0.2.1 | `sendto` palautti onnistumisen; lokissa DROP UDP, portit täsmäävät / DROP logged |
| Loopback 127.0.0.1 | yhteys syntyi: palomuuri ei suodata silmukkaliikennettä (tieto, ei vaatimus) / connected: loopback not filtered (information) |

Pudotusloki (koko sisältö, 6 merkintää; viimeinen sarake = prosessin pid) / the whole
drop log:

```
2026-11-24 21:06:30 DROP TCP 192.168.2.199 192.0.2.1 52840 46499 ... SEND 108528   (eristyskoe / isolation probe)
2026-11-24 21:06:30 DROP UDP 192.168.2.199 192.0.2.1 60933 48541 ... SEND 108528
2026-11-24 21:10:30 DROP TCP 192.168.2.199 192.0.2.1 57086 43519 ... SEND 70264    (ajo 0:n koe / run 0 probe)
2026-11-24 21:10:30 DROP UDP 192.168.2.199 192.0.2.1 52597 48663 ... SEND 70264
2026-11-24 21:11:30 DROP TCP 192.168.2.199 192.0.2.1 61919 46217 ... SEND 108212   (ajo 1:n koe / run 1 probe)
2026-11-24 21:11:30 DROP UDP 192.168.2.199 192.0.2.1 54033 41374 ... SEND 108212
```

Lokissa ei ole muita pudotusmerkintöjä: ajojen aikana HkMaali.exe:n polusta ei lähtenyt
palomuurin estämää ja lokiin kirjattua IP-liikennettä ulkoverkkoon. Se, että säännöt
koskivat myös HkMaali.exe:tä, on [päätelty]: säännöt kohdistuvat polkuun, ja ohjelma ajettiin
samasta polusta kuin koeohjelma. Loki **ei kata** DNS-kyselyjä (Dnscache-palvelu tekee
ne), silmukkaliikennettä (ei suodateta eikä kirjata), muiden prosessien ohjelman puolesta
tekemää liikennettä (tulostusjono, WinHTTP-palvelu, synkronoidut asemat, lapsiprosessit)
eikä kuuntelevia soketteja (netstat 2–3 näytettä). HkMaali.exe:n pid:tä ei tallennettu,
joten lokin pid-saraketta ei voitu käyttää. Palomuurin eston todennus (koeohjelma) ja
havainto ohjelman omista yhteysyrityksistä ovat siis eri asioita.

No other drop entries: no blocked, logged IP traffic to external addresses left the
HkMaali.exe path during the runs. That the rules also covered HkMaali.exe is inferred
(path rule, same path). The log does not cover DNS (Dnscache service), loopback,
traffic made by other processes on the program's behalf, or listening sockets (2–3
netstat samples); HkMaali's pid was not recorded. Verifying the block (probe) and
observing the program's own connection attempts are different things.

### 17.8 Ajo 0 uudelleen (palomuurieristys) / Run 0 again (firewall isolation)

| | |
|---|---|
| Ohjelma / program | `HkMaali.exe` SHA-256 `61034e5d…7d297d` (kopio testipolussa sama / copy identical) |
| Komento / command | `run_hkmaali.py --mode firewall --udp-verified udp_verified.json --exe TPexe/Hk/V521/HkMaali.exe --probe isolation_probe.exe --script run0.json`; ohjelma: `HkMaali.exe BOOT`, cwd = testihakemisto |
| Aineisto / data | ei mitään / none |
| Kulku / sequence | otsikkolaatikko (HkMaali 5.21, Build 5.22.002/2026-07-31); havaittu viimeistään 0,21 s "Tiedosto KilpSrj.xml puuttuu" → Enter; 0,73 s "Sarjatietojen lukeminen ei onnistunut" → Enter (ajat = havaintohetket: 0,2 s kysely + 0,3 s viive ennen näppäintä / detection times: 0.2 s polling + 0.3 s delay before each key) |
| Lopetus / end | ohjelma päättyi itse 1,44 s, poistumiskoodi 0. Poistumiskoodi on aina 0 (`HkConsole/HkMain.cpp:233`), myös virhepolulla; se osoittaa vain, ettei prosessi kaatunut eikä sitä lopetettu / exit code is always 0, also on the error path; it only shows no crash or kill |
| Tiedostot / files | lopputilassa vain `HkMaali.exe` (ennallaan) / final state: only the program |
| Muut / other | ei ikkunoita; netstat 2 näytettä ilman päätepisteitä (ei lopetusvaiheessa); ei muutoksia seuratuissa hakemistoissa (`%TEMP%`, CrashDumps, WER; syvyys ≤ 3, koko ja aikaleima) / no windows; 2 netstat samples without endpoints (none during shutdown); no changes in the watched directories |

### 17.9 Ajo 1 / Run 1

| | |
|---|---|
| Komento / command | kuten ajo 0, `--script run1.json --data Tests/data/hk/run1_kilpsrj.xml=KilpSrj.xml` |
| Aineisto / data | `Tests/data/hk/run1_kilpsrj.xml` (SHA-256 `23ebf0b8…015ab7e`): 1 vaihe, 2 väliaikaa, 1 sarja H21; tarkistettu ennen ajoa tuotannon lukijalla komponenttitestissä `hk-data-run1-kilpsrj` (PASS: `luesarjat` 0, ei viestejä, `kilprecsize` 544) |
| Kulku / sequence | havaittu viimeistään 0,21 s "Tiedostoa KILP.DAT ei ole. Luodaan uusi tiedosto." → Enter; 0,73 s PÄÄVALIKKO (rivi "P)oistu") → P; 1,25 s "Haluatko varmasti poistua ohjelmasta (K/E)" → K (havaintohetket, ks. ajo 0 / detection times) |
| Lopetus / end | ohjelma päättyi itse 2,16 s, poistumiskoodi 0 (aina 0, ks. ajo 0 / always 0) |
| Tiedostot / files | `KILP.DAT` 544 tavua (SHA-256 `f1d3fa28…af985`) + syötetiedostot ennallaan / input files unchanged |
| KILP.DAT | tasan yksi otsaketietue: `firstfree` FFFF, `numberfree` 0, `int1` 0, `int2` 1; `maaliajat[0]` 0, `[1..8]` −86 400 000 (TMAALI0), `[9]` 0; tavut 56–543 nollia. Rakenne vastaa `tiedostomuodot.md`:n otsakekuvausta; arvot selittyvät koodista (`dbboxm/dbbox1.cpp:34-44`, `:214-222`; `Hk/HkInit.cpp:3388-3392`, `:3497`; otsakkeen kirjoitti lopetuksen `closefile`, `Hk/HkInit.cpp:3790`). Arvoja ei ollut ennustettu kirjallisesti ennen ajoa. Tavut 56–543 ovat nollia, koska `tarecbuf` oli käyttämätön (ei taattu, H-051) / layout matches the header description; values follow from the code (closefile at exit); not predicted in writing before the run; zero tail not guaranteed (H-051) |
| Muut / other | ei ikkunoita; netstat 3 näytettä ilman päätepisteitä (ei lopetusvaiheessa); ei muutoksia seuratuissa hakemistoissa. Lopputilassa hiekkalaatikossa uutena vain KILP.DAT; tiedosto-operaatioita (luvut, poistetut väliaikaistiedostot, estetyt kirjoitusyritykset) ei jäljitetty – Procmon-todennus (luku 16) tekemättä / final state: only KILP.DAT new; file operations were not traced (no Procmon) |

### 17.10 Mitä koeajot osoittivat ja mitä eivät / What the runs showed and did not

Osoittivat / showed:
- Toimitettu HkMaali.exe (alkuperä varmistettu) käynnistyy ja toimii eristettynä.
  Palomuuri esti testipolusta lähtevän IPv4 TCP- ja UDP-liikenteen (todennettu lokista
  koeohjelmalla). NTFS-kirjoitukset rajattiin testihakemistoon kirjoitusrajoitetulla
  tunnuksella [päätelty mekanismista; testattu neljällä hakemistolla: `%TEMP%`, profiili,
  Dokumentit, `LOCALAPPDATA`]. Ei testattu: rekisteri (HKCU antaa RESTRICTED-SID:lle vain
  luvun), D:, G: (Google Drive, FAT32 – ACL-suojaus ei välttämättä päde), verkkojaot,
  nimetyt objektit. Job-objekti ja käynnistys keskeytettynä toimivat; aikarajan ylitys ja
  lopetus nähtiin vain AppContainer-ajossa (0xDEAD0001).
- Puuttuvan sarjatiedoston käsittely: kaksi ilmoitusta ja lopetus itsestään (poistumiskoodi
  0, joka ei erota virhepolkua onnistumisesta), ei tiedostoja.
- Synteettisen sarjatiedoston luku, KILP.DAT:n luonti (otsakekuvauksen mukainen) ja
  lopetus valikosta.
- Lokissa ei ole ohjelman polusta estettyä ja kirjattua ulkoverkkoliikennettä (rajaukset
  17.7).

Eivät osoittaneet / did not show:
- **Yhtään H-havaintoa ei vahvistettu eikä kumottu ajamalla**; vikamekanismeja ei
  yritetty toistaa (H-041 toimitetussa ohjelmassa vaatii erillisen ajon ja arvion).
- Ettei ohjelma yrittänyt verkkoyhteyksiä lainkaan: DNS, silmukkaliikenne, muiden
  prosessien välittämä liikenne ja kuuntelevat soketit jäivät lokin ulkopuolelle; netstat
  vain 2 (ajo 0) / 3 (ajo 1) näytettä, ei lopetusvaiheessa.
- Silmukkaliikenteen (127.0.0.1) estoa: palomuuri ei suodata sitä.
- IPv6:ta, DNS:ää ja kuuntelua ei testattu koeohjelmalla (säännöt koskevat kaikkia
  protokollia [päätelty]).
- Tiedosto-operaatioita (Procmon) eikä lapsiprosesseja (Job-objektin laskuri) seurattu.
- Kaatumisraporttien ohjautumista: kaatumisia ei tapahtunut (`DIE_ON_UNHANDLED_EXCEPTION`
  käytössä).

In English (17.10): shown – the shipped program starts and runs isolated; the firewall
blocked IPv4 TCP/UDP from the test path (log, probe); NTFS writes confined by the
write-restricted token (inferred, four directories tested; registry, D:, G: (FAT32,
Google Drive), shares and named objects not tested); timeout kill only seen in the
AppContainer run; missing series file → two messages and self-exit (exit 0 does not tell
the error path apart); synthetic series file read, KILP.DAT created per the header
description; no blocked, logged external traffic from the program path. Not shown – no H
finding confirmed or refuted; that the program made no network attempts at all (DNS,
loopback, other processes, listening sockets; 2–3 netstat samples); IPv6/DNS/listen not
probed; file operations and child processes not traced.

### 17.11 Jäänteet / Leftovers (päivitetty / updated)

- Palomuurisäännöt `TP C11 HkMaali block in/out` ovat yhä käytössä (poisto
  järjestelmänvalvojana: `Remove-NetFirewallRule -DisplayName 'TP C11 HkMaali block*'`).
- Lokitusasetukset palautettu ja tarkistettu.
- `%LOCALAPPDATA%\tulospalvelu-c11\` (testihakemisto, lokikopio; palvelun suojaama
  `pfirewall.log` voi vaatia järjestelmänvalvojan poistoon).
- AppContainer-profiili `tulospalvelu.c11.hkmaali` ja `%TEMP%\hkmaali_run_*` (luku 17.6).
- Tulokset: `run.json`, `screens.log`, `sandbox_copy/`, `udp_verified.json`,
  `fwrun0_log.json`, `fwrun1_log.json` (tilapäishakemisto, ei repossa).

### 17.12 Riippumaton tarkastus / Independent review (2026-11-24)

`independent-verifier` tarkasti ajot 0 ja 1, ajurin, eristyksen, lokin tulkinnan,
aineiston, KILP.DAT-tarkistuksen ja dokumentaation. **Raakaluvut vastasivat
tulostiedostoja** (tiivisteet, ajat, poistumiskoodit, tiedostoluettelot, lokirivit,
KILP.DAT-tavut, aineiston tarkistus ennen ajoa 1). Korjatut liioittelut: väite "ohjelma
ei yrittänyt verkkoliikennettä" (rajattu lokin kattavuuteen, 17.7), "kirjoitukset
rajattu testihakemistoon" (rajattu NTFS:ään ja testattuihin hakemistoihin), "ei
odottamattomia tiedosto-operaatioita" (lopputila, ei jäljitystä), "ei ulkopuolisia
muutoksia" (seuratut hakemistot), "täsmälleen ennusteen mukainen" (otsakekuvauksen
mukainen, arvot selitetty koodista), poistumiskoodin tulkinta, vaiheajat havaintohetkinä
ja luvun 17 vanhentunut johdanto.

Raw numbers matched the result files; the overstatements listed above were corrected.

Ajuriin ei tehty muutoksia ajojen jälkeen: commitoitu `run_hkmaali.py` on sama, jolla
ajot tehtiin. Tarkastuksen työkaluehdotukset ovat vaiheen 4 esiehtoja (kirjataan vaiheen 4 suunnitelmaan ennen H-041-ajoa):
HkMaali.exe:n pid `run.json`:iin ja lokin täsmäytykseen; netstat jokaisella kyselyllä
myös lopetusvaiheessa; Job-objektin prosessilaskuri (lapsiprosessit); koeohjelman
lopetus 30 s jälkeen; ulkoisen TCP:n hyväksyntä vain virheellä 10013 palomuuritilassa;
D:, G: ja HKCU-kirjoitus ulkopuolisiin tarkistuksiin (tai dokumentoidaan
testaamattomiksi); prosessikahvojen sulkeminen; moduulin kuvausteksti ajan tasalle.

The driver was not changed after the runs (committed = as run). The review's tool
proposals are prerequisites of stage 4 (to be recorded in the stage 4 plan before any H-041 run).

Avoimet kysymykset / open: valvooko G: (Google Drive, FAT32) ACL:iä rajoitettua tunnusta
vastaan; kattoiko 21.12.38 otettu lokikopio varmasti 21.11.31–33 pudotukset (päätelty
lokin muutosajasta 21.11.56); yrittikö ohjelma estettyjä kirjoituksia tai odottamattomia
lukuja (Procmon); DNS/IPv6/kuuntelu koeohjelmalla.

## 18. C-11 vaihe 4: H-041 toimitetussa HkMaali.exe:ssä – suunnitelma / Stage 4 plan (2026-11-24)

**Tila / status: hyväksytty päätöksin (luku 19.1); esiehdot toteutettu, ajot odottavat
(luku 19).** / Approved with decisions (19.1); prerequisites done, runs pending (19).
Lähteet: `architecture-analyst` (polku ja aineisto), oma staattinen
tarkistus toimitetusta binääristä ja PDB:stä (ohjelmaa ei käynnistetty), luvun 17.12
tarkastus.

### 18.1 Vikapolku toimitetussa ohjelmassa / Fault path in the shipped program

- **Aineisto / data:** `luekentat`: `kilprecsize = 360 + n_pv * (152 + (valuku + 2) * 8)`
  (`Hk/HkDat.cpp:105-180`, `:137`, `:177`). `n_pv` ja `valuku` luetaan XML:stä
  (`<FileFormat><Races>`, `<Intermediaries>`, `Hk/HkXml.cpp:438-443`) ilman rajatarkistusta;
  `luesarjat` palaa heti onnistuneen XML-luvun jälkeen (`Hk/HkSrj.cpp:72-75`), joten
  rajatarkistus `:158` koskee vain vanhaa tekstimuotoa (ja N_PV 5 / VALUKU 250 ovat
  sallittuja maksimeja muutenkin) [todennettu]. Raja `n_pv` 5: `valuku` 220 → 10 000
  (mahtuu tasan), 221 → 10 040 (ensimmäinen ylitys), 250 → 11 200.
- **Käynnistys ei vielä ylitä / startup does not overflow:** tyhjän KILP.DAT:n luonti ja
  avaus käyttävät kekopuskuria `tarecbuf` (`dbboxm/dbbox1.cpp:103-134`, `:161`), ja
  `teejarj`-lukusilmukka ohitetaan, kun tietueita ei ole (`Hk/HkIx.cpp:1663-1668`)
  [todennettu]. Lukupolku (`kilptietue::read` käynnistyksessä) vaatisi valmiin
  11 200-tavuisen kilpailijatietueen – rajataan pois vaiheesta 4.
- **Kirjoituspolku / write path (vähin syöte):** PÄÄVALIKKO `K` → `korjaukset`
  (`HkConsole/HkMain.cpp:138`) → `L)isää` → `add(0)` (`HkConsole/HkCons2.cpp:805`) →
  `kilp.inputkilp(1)` (sukunimi + `+` hyväksyy, `HkConsole/HkKilp.cpp:932`) →
  `addtall` (`Hk/HkEmit.cpp:1265`) → `addfrec` (`Hk/HkDat.cpp:629-633`): `pack(buf)`
  kirjoittaa `kilprecsize` tavua pinopuskuriin `char buf[KILPBUFSIZE]`. Näppäinjärjestys
  lomakkeessa on [päätelty] ja varmistetaan kontrolliajossa.
- **Toimitetun binäärin rakenne (staattinen, PDB) / shipped binary (static, PDB):**
  `kilptietue::addfrec` = 0x00423120 (285 tavua). Kehys: `mov eax,2718h; call
  __alloca_probe`; eväste `[ebp-4]` = `__security_cookie ^ ebp`; `buf` = `ebp-2714h`,
  joten puskurin loppu on **täsmälleen eväste** (`ebp-4`). Ylitys `kilprecsize − 10 000`
  tavua kirjoittaa evästeen, tallennetun EBP:n, paluuosoitteen, parametrin `d` ja kutsujan
  kehystä (11 200: 1 196 tavua `ebp`:n yli; 10 040: 36 tavua). `pack` → `addrec`
  (`WriteFile` KILP.DAT:iin) → `kilparr`-kopio → kumpikin paluupolku kutsuu
  `__security_check_cookie` (0x00423232 → paluu 0x00423237; virhehaara 0x004231FB →
  0x00423200) ennen `ret`-käskyä. Kuvassa on `/GS`-eväste (`dumpbin /loadconfig`:
  Security Cookie 0x004F8700), DEP ja ASLR (DLL characteristics 0x8140). `d` luetaan
  ylityksen jälkeen kehyksen alapuolelta (`ebp-2718h`), ei ylitetyltä alueelta
  [todennettu disassemblystä].
- **Odotettu tulos / expected:** tietue kirjoittuu KILP.DAT:iin (11 200 tavua) ja
  `addfrec`-paluussa evästetarkistus epäonnistuu → `__report_gsfailure` → `__fastfail(2)`
  → STATUS_STACK_BUFFER_OVERRUN 0xC0000409. Paluuosoitetta ei koskaan käytetä
  [päätelty disassemblystä; ei havaittu ajossa].

### 18.2 Vaihtoehdot / Options

| | Mitä todistaa / proves | Mitä ei / does not prove |
|---|---|---|
| **A. Toimitettu HkMaali.exe, ei instrumentointia; ajuri debuggerina + PDB** | Vika laukeaa toimitetussa ohjelmassa käyttöliittymän kautta: 0xC0000409 (alikoodi 2) `addfrec`-funktion evästetarkistuksessa, KILP.DAT:ssa 11 200-tavuinen tietue, rajapari 220/221 erottaa ylityksen. Taso: **vahvistettu ajamalla** (toimitettu ohjelma). | Ylityksen tavumäärää ei nähdä suoraan (vain eväste rikki + tiedoston koko); `/GS` kertoo, että eväste muuttui, ei kuka kirjoitti; lukupolkua ei. |
| **B. ASan-käännös samasta tuotantokoodista** (uusi projekti `VS/Tests/`-hakemistoon) | Tarkka WRITE-osoite, koko ja kehys koko ohjelman UI-polulla. | Ei toimitettu ohjelma: eri pinoasettelu (punaiset vyöhykkeet), ei `/GS`-käytöstä. Taso: toistettu testikehyksessä / uudelleenkäännetyssä ohjelmassa – **ei** vahvistettu ajamalla toimitetussa. Suuri työ (konsoliohjelman koko lähdejoukko). |
| **C. Komponenttitestien täydennys** (TpCoreHk Release + Asan) | Halpa: `addfrec`-polun ASan-ylitys (nyt vain `write`), lukupolku muokatulla KILP.DAT:lla (`getrec` → `read`), XML-rajapari 220/221 tuotannon lukijalla. | Ei ohjelma-ajoa; taso pysyy "toistettu testikehyksessä". |

**Suositus / recommendation:** A ensisijaisena; C sen esiehtona ja täydennyksenä
(aineistotarkistus + `addfrec`-ASan). B vain, jos A jää epäselväksi. Tuloksia ei
yhdistetä: A:n havainto merkitään toimitetulle ohjelmalle, B:n ja C:n testikehykselle /
uudelleenkäännetylle ohjelmalle erikseen.

### 18.3 Menetelmä A / Method A

1. **Esiehdot (ei ohjelma-ajoa) / prerequisites:**
   - Ajurikorjaukset luvusta 17.12: HkMaali.exe:n pid `run.json`:iin ja lokin
     täsmäytykseen; netstat jokaisella kyselyllä myös lopetusvaiheessa; Job-objektin
     prosessilaskuri (lapsiprosessit) ja `ACTIVE_PROCESS` 1; koeohjelman lopetus 30 s
     jälkeen; ulkoinen TCP hyväksytään vain virheellä 10013; prosessikahvat suljetaan;
     moduulin kuvaus ajan tasalle; D:, G: ja HKCU-kirjoitus ulkoisiin tarkistuksiin tai
     dokumentoidaan testaamattomiksi.
   - **Debuggeritila:** ajuri käynnistää ohjelman `DEBUG_ONLY_THIS_PROCESS`-lipulla (sama
     rajoitettu tunnus, Job-objekti, konsoli), käsittelee `WaitForDebugEvent`-silmukan,
     tallentaa jokaisen poikkeuksen (koodi, osoite, ensimmäinen/toinen mahdollisuus,
     `ExceptionInformation`), säikeen kontekstin ja pinon (EBP-ketju + DbgHelp, PDB:n
     GUID tarkistettu) ja lopettaa prosessin 0xC0000409:n jälkeen. Syy: Windowsin
     virheraportointi on käytössä (WerSvc, ei `Disabled`, ei LocalDumps) – ilman
     debuggeria pikakaatuminen käynnistäisi WerFaultin, joka toimii Job-objektin ja
     palomuurisäännön ulkopuolella, kirjoittaa `ProgramData\…\WER`-hakemistoon ja voi
     lähettää raportin verkkoon.
   - **Mekanismin kontrolli:** oma pieni testiohjelma, joka kutsuu `__fastfail(2)` ja
     tavallisen käsittelemättömän poikkeuksen; ajuri debuggerina näkee molemmat, WerFault-
     prosessia ei synny eikä WER-hakemistoihin tule raportteja. Jos tämä ei pidä,
     keskeytetään.
   - **Aineisto:** `Tests/data/hk/h041_v220.xml`, `h041_v221.xml`, (`h041_v250.xml`):
     ajon 1 tiedosto, jossa `Races` 5 ja `Intermediaries` 220/221/250. Tarkistetaan
     TpCoreHk:ssa: `luesarjat` 0, `kilprecsize` 10 000 / 10 040 / 11 200.
   - Palomuurin pudotuslokitus otetaan uudelleen tilapäisesti käyttöön (järjestelmänvalvoja)
     ja palautetaan lopuksi, kuten luvussa 17.7.
2. **Kontrolliajo / control run (valuku 220, `kilprecsize` 10 000):** BOOT → KILP.DAT
   luodaan → `K` → `L` → sukunimi → `+` → poistu valikkoon → `P` → `K`. Odotus: ei
   poikkeusta, KILP.DAT = 2 × 10 000 = 20 000 tavua (otsake + 1 tietue), itse lopetus.
   Ajo vahvistaa myös näppäinjärjestyksen ja sen, että kirjoituspolku todella kulkee
   (tiedoston kasvu yhdellä tietueella).
3. **Rajanylitysajo / boundary run (valuku 221, 10 040):** sama näppäinsarja. Valinnainen
   lisäajo valuku 250 (11 200).

### 18.4 Hyväksymiskriteerit / Acceptance criteria (H-041 "vahvistettu ajamalla")

Kaikki seuraavat / all of:
1. Eristys todennettu ennen ajoa (luku 17.7 + esiehtojen korjaukset), alkuperä
   (SHA-256 `61034e5d…7d297d`, PDB-GUID `{B0500861-3E83-4118-B0DA-E8E331F12614}`).
2. Kontrolliajo (220) PASS: ei poikkeusta, KILP.DAT 20 000 tavua, tietueen koko
   10 000, itse lopetus.
3. Ylitysajossa (221) näytöt etenevät samoin kuin kontrollissa hyväksyntään (`+`) asti.
4. Debuggeri saa poikkeuksen 0xC0000409, `ExceptionInformation[0]` = 2
   (FAST_FAIL_STACK_COOKIE_CHECK_FAILURE); pinossa paluuosoite 0x00423237 tai 0x00423200
   (`kilptietue::addfrec`, PDB:n mukaan) ja kutsuja `addtall`; ennen sitä ei muita
   käsittelemättömiä poikkeuksia.
5. Evästepaikka `[ebp-4]` poikkeaa arvosta `__security_cookie ^ ebp` (luetaan prosessin
   muistista poikkeushetkellä).
6. KILP.DAT = 2 × 10 040 = 20 080 tavua (otsake + tietue kirjoitettu ennen
   evästetarkistusta).
7. Ei WerFault-prosessia, ei uusia WER-raportteja, ei ohjelman pid:n pudotusmerkintöjä,
   ei muutoksia seuratuissa hakemistoissa.

Pelkkä kaatuminen, aikakatkaisu tai poistumiskoodi ei riitä. Jos kohdat 4–6 täyttyvät
mutta 2 ei, tulos on epäselvä (ei vahvistettu).

### 18.5 Keskeytyskriteerit / Abort criteria

Keskeytä ja raportoi ennen seuraavaa ajoa, jos / stop and report if:
- eristystä (verkko, tiedostot, konsoli, debuggeritila, WER-kontrolli) ei voida todentaa;
- kontrolliajo poikkeaa odotuksesta (poikkeus, eri tiedostokoko, eri näytöt);
- ylitysajossa tulee muu poikkeus tai poikkeus muualla kuin `addfrec`-paluussa
  (kirjataan havainnoksi, ei luokitella H-041:ksi);
- WerFault-prosessi, uusi WER-raportti tai ohjelman pid:n pudotusmerkintä havaitaan;
- aikaraja ylittyy (prosessi lopetetaan, tulos ei ole todiste);
- tiedostoja syntyy testihakemiston ulkopuolelle.

### 18.6 Riskit ja eristysvaatimukset / Risks and isolation

- **Muistikorruptio:** rajoittuu prosessin omaan pinoon (nollia ja oletusarvoja
  pakatusta tietueesta, ei ulkoista syötettä). Ylityksen ja evästetarkistuksen välissä
  ohjelma tekee vain `WriteFile`-kirjoituksen KILP.DAT:iin (testihakemisto),
  `kilparr`-kopion ja `LeaveCriticalSection`-kutsun; paluuosoitetta ei käytetä
  [päätelty disassemblystä]. Testihakemiston ulkopuolelle vaikutus voisi kulkea vain
  prosessin omien oikeuksien kautta (kirjoitusrajoitettu tunnus, palomuuri).
- **WER** on suurin eristysriski (ks. 18.3); debuggeritila tai käyttäjän erillinen lupa
  WER:n tilapäiseen poistoon (HKCU `Windows Error Reporting\Disabled` = 1, ei vaadi
  järjestelmänvalvojaa, mutta on asetusmuutos).
- **Jumiutuminen:** Job-objekti + aikaraja; debuggeritilassa prosessi ei jatka
  0xC0000409:n jälkeen.
- **Debuggerin vaikutus:** debuggerin alla Windows käyttää eri keon
  tarkistusasetuksia; pinon `/GS`-käytös ei muutu [päätelty].
- **Lukupolku** (käynnistyksessä, `teejarj` → `read`) jää todentamatta; se vaatisi
  käsin tehdyn KILP.DAT:n.

### 18.7 Päätökset ennen ajoa / decisions needed

1. Hyväksytäänkö menetelmä A (debuggeritila) ensisijaiseksi ja C sen esiehdoksi; tehdäänkö
   B lainkaan.
2. Ajetaanko ylitys rajalla 221, lisäksi 250, vai vain toinen.
3. WER: riittääkö debuggeritila (ja sen kontrolli), vai sallitaanko HKCU-tason WER:n
   tilapäinen poisto lisävarmistukseksi.
4. Palomuurin pudotuslokituksen tilapäinen käyttöönotto uudelleen (järjestelmänvalvoja).
5. Saako ajurin esiehtokorjaukset ja aineistot commitoida ennen ajoa erillisenä committina.

In English: stage 4 reproduces H-041 in the shipped HkMaali.exe through K → L → add →
`addfrec` with a synthetic series file (`Races` 5, `Intermediaries` 221; control 220).
Static disassembly with the PDB shows `buf` ends exactly at the `/GS` cookie and both
return paths check it, so the expected result is 0xC0000409 (fast-fail code 2) at the
`addfrec` return after the record is written. The driver runs the program as a debugger
so WER (enabled on this machine) never starts. Acceptance needs the control run, the
exception code and subcode, the return address in `addfrec`, the broken cookie, the
KILP.DAT size and clean isolation evidence; a crash alone is not enough. Option B (ASan
rebuild) and C (component tests) prove the overrun only in rebuilt code and are recorded
separately.

## 19. C-11 vaihe 4: esiehdot ja H-041-koeajo / Stage 4: prerequisites and the H-041 runs (2026-11-24)

### 19.1 Käyttäjän päätökset / User decisions

| | Päätös / decision |
|---|---|
| Menetelmä / method | A (toimitettu HkMaali.exe debuggerin hallinnassa) ja C (komponenttitestit ensin); B (ASan-uudelleenkäännös) ei tässä vaiheessa / A and C; no B |
| Rajapari / boundary pair | 5 vaihetta / 220 väliaikaa (10 000 tavua) ja 5 / 221 (10 040); 250 ei / not 250 |
| WER | debuggeritila; ei WER-asetusmuutoksia; jos debuggeri ei estä WER:iä luotettavasti → pysähdy ja pyydä lupa HKCU-muutokseen / debugger mode, no settings change |
| Palomuuri / firewall | pudotuslokituksen järjestelmänvalvojan komennot ajaa käyttäjä; alkuperäiset asetukset kirjataan ja palautetaan; säännöt voimassa koko ajan / the user runs the admin commands |
| Commit | esiehdot erillisenä paikallisena committina; koeajojen tuloksia ei commitoida ilman erillistä hyväksyntää; ei pushia / prerequisites in a separate local commit; run results only with approval |
| Rajaukset / limits | ei tuotantokoodin, CI:n eikä muiden havaintojen tasojen muutoksia; ei oikeita kilpailutietoja; ei muita vikatoistoja / no production, CI or other-finding changes |

### 19.2 Aineisto ja komponenttitestit (menetelmä C) / Data and component tests (method C)

| Tiedosto / file | Sisältö / content | SHA-256 |
|---|---|---|
| `Tests/data/hk/h041_v220.xml` | ajon 1 tiedosto, `Races` 5, `Intermediaries` 220, `RaceCount` 1 | `8cb8262c7338881ebcf5995510cacb865f8c8f48187a86216c8d46d41bf42de2` |
| `Tests/data/hk/h041_v221.xml` | sama, `Intermediaries` 221 | `2ed216acdfa9b3071541f1efad52c0c417e299a6e3ca96d6ae3b5b8e5c445a42` |

| Testi / test (`run_core_tests.py`) | Tulos / result |
|---|---|
| `hk-data-h041-v220` (TpCoreHk Release, tuotannon `luesarjat` + `luekentat`) | PASS: `luesarjat` 0, ei viestejä, n_pv 5, n_pv_akt 1, valuku 220, `kilprecsize` **10 000** (= KILPBUFSIZE) |
| `hk-data-h041-v221` | PASS: valuku 221, `kilprecsize` **10 040**; tulostettu havainto (ei tarkistus): `pack` kirjoittaa `nollaa()`-tyhjennetylle tietueelle tavuihin 10 000–10 039 nollia (käyttöliittymän tietue on eri tietue; kriteeri 6 vertaa tiedoston tavuihin) / printed observation for a cleared record, not an assertion |
| `H-041b4-control` (TpCoreHkAsan) | PASS: `kilptietue::addfrec` valuku 220:llä ilman ASan-raporttia, KILP.DAT 20 000 tavua |
| `H-041b4` (TpCoreHkAsan) | XFAIL: ASan `stack-buffer-overflow` WRITE muuttujaan `buf` (10 000) kehyksessä `kilptietue::addfrec` valuku 221:llä |
| Koko sarja / whole suite | PASS 23, XFAIL 24, ei FAIL/XPASS; `test_run_core_tests.py` 46/46 |

Seuraus nollista / consequence of the zeros: pinon ja tietueen tavujen vertailu yksin
olisi heikko todiste (nolla vastaa nollaa). Siksi debuggeri tallentaa kehyksen arvot
**ennen** ylitystä (keskeytyskohta funktion alussa) ja vertaa niitä poikkeushetken
arvoihin (19.4). / Comparing stack and record bytes alone would be weak (zeros), so the
frame values are recorded before the overflow and compared after.

`H-041b4` on uudelleenkäännettyä koodia (ASan), ei toimitettu ohjelma; taso pysyy
"toistettu testikehyksessä". / Rebuilt code; level stays "reproduced in test harness".

### 19.3 Ajurin korjaukset (luvut 17.12, 18.3) / Driver fixes

Työkalut / tools: `Tests/blackbox/run_hkmaali.py`, uusi `hkdebug.py`, `verify_log.py`,
`isolation_probe.c`, uusi `crash_probe.c`, `build_probes.cmd` (käännös repositorion
ulkopuolelle), `scripts/h041_v220.json`, `scripts/h041_v221.json`, offline-testit
`test_run_hkmaali.py` (39 testiä).

| Vaatimus / requirement | Toteutus / implementation | Todennus / verified by |
|---|---|---|
| PID tallennetaan / pid recorded | `run.pid`, `probe.pid`; `verify_log.py` hakee pudotukset ohjelman pid:llä ja ajoikkunassa (±60 s) | offline-testi `ProgramDrops` |
| Prosessit ja lapsiprosessit / processes and children | jokaiselle käynnistykselle oma Job-objekti, `ActiveProcessLimit` 1 (lapsiprosessi ei käynnisty), kirjanpito (`TotalProcesses`, `ActiveProcesses`); prosessilistan seuranta 0,1 s välein (lapset ja WER-prosessit) | crash control: lapsi estetty (virhe 1816, `TotalProcesses` 1); ilman rajaa lapsi nähtiin ja kuoli Job-objektin mukana; 300 ms:n prosessi havaittiin |
| Verkkoseuranta / network monitoring | iphlpapi: TCP (kaikki tilat) ja UDP, IPv4 ja IPv6, ohjelman pid; jokaisella kyselyllä, myös lopetusta odotettaessa ja lopun jälkeen | positiivinen kontrolli: ajurin omat TCP-, UDP- ja TCP6-pistokkeet löytyivät |
| Koeohjelman aikakatkaisu / probe timeout | 30 s kokonaisraja (myös READY-odotus), sitten `TerminateProcess` ja "ei todennettu" | itsetesti: jumiutuva koeohjelma lopetettiin 5 s:n rajalla (5,16 s), ei todennettu |
| Ulkoinen TCP vain 10013 / external TCP only 10013 | koeohjelma ja ajuri vaativat virheen 10013 (muu virhe, estetty soketti tai aikakatkaisu = ei todennettu) | koe: `error 10013 after 0 ms` |
| D:, G:, HKCU | ulkopuoliset kirjoituskohteet `D:\`, `G:\`, `G:\Oma Drive\`; HKCU: avaimen luonti ja `KEY_SET_VALUE`; jäljelle jäänyt avain hylkää koeohjelman tuloksen (`run_probe`) | koe: kaikki virhe 5; HKCU 5 ja 5 (19.5) |
| Lokin kattavuus / log coverage | toinen koeohjelma-ajo ohjelman **jälkeen** (`post_probe`); `verify_log.py` vaatii myös sen paketit lokikopiosta (`covers_run`), ei pudotuksia ohjelman pid:llä eikä muiden kuin koeohjelmien lähteviä (SEND) pudotuksia ajoikkunassa | offline-testit `ProgramDrops` |
| Kahvat / handles | `close_pi`, Job- ja token-kahvat suljetaan; debug-tapahtumien `hFile` suljetaan | koodi |

### 19.4 Debuggeritila ja WER-kontrolli / Debugger mode and WER control

**Toteutus / implementation (`hkdebug.Debugger`):** ajuri käynnistää ohjelman
`DEBUG_ONLY_THIS_PROCESS`-lipulla omassa säikeessään (sama kirjoitusrajoitettu tunnus,
Job-objekti ja konsoli) ja käsittelee jokaisen debug-tapahtuman. Ensimmäisen
mahdollisuuden poikkeukset annetaan ohjelmalle ja kirjataan. Kohtalokas poikkeus
(0xC0000409, toinen mahdollisuus tai jatkokelvoton) → konteksti (`Wow64GetThreadContext`),
EBP-ketju, pinon koodiosoitteet, valvotun funktion kehys → `TerminateProcess` (koodi
0xDEAD0409) **ennen** jatkamista, joten WER ei käynnisty. Valinnainen `--watch`:
kertakäyttöinen int3 funktion alkuun (PDB:stä, ASLR-kanta huomioiden); siinä tallennetaan
paluuosoite, kutsujan EBP, ensimmäinen parametri ja evästeen odotusarvo
(`__security_cookie ^ EBP`), ja asetetaan toinen int3 paluukohtaan. Paluukohdassa
(normaali paluu) luetaan kehyksen paikat uudelleen. Alkuperäinen tavu palautetaan ja EIP
asetetaan takaisin; binääriä levyllä ei muuteta. DbgHelp `SYMOPT_EXACT_SYMBOLS`; PDB:n
GUID verrataan EXE:n RSDS-tietoon.

The driver is the program's debugger; fatal exceptions are recorded and the process is
terminated before it is continued, so WER never starts. One-shot breakpoints at the
watched function's entry and return site record the frame before and after; the binary
on disk is not changed.

**Kontrolli / control (`crash_probe.c`, x86 `/GS /Oy-`, `victim()`: 64 tavun puskuri
päättyy evästeeseen kuten `addfrec`issa; "gs 104" ylittää 40 tavulla nollilla):**

| Skenaario / scenario | Tulos / result |
|---|---|
| `gs 64` (ei ylitystä) | poistumiskoodi 0; alun keskeytys: kutsuja `main+0x6C`; paluukohdassa eväste, tallennettu EBP ja paluuosoite ennallaan |
| `gs 104` | 0xC0000409, alikoodi 2, toinen mahdollisuus, `__report_gsfailure+0x5`; [ESP] = `victim+0x41`; EBP = valvottu kehys; eväste-, EBP-, paluu- ja parametripaikat = 0 (ennen: paluu `main+0x6C`, eväste odotusarvossaan); 40 ensimmäistä tavua nollia; prosessi lopetettu (0xDEAD0409) |
| `fastfail` | 0xC0000409, alikoodi 2, lopetettu |
| `av` | 0xC0000005 ensin 1. mahdollisuus, sitten 2. mahdollisuus → lopetettu |
| `child` + raja 1 | lapsen luonti estetty (virhe 1816), `TotalProcesses` 1; 0xC0000409 |
| `child` ilman rajaa | lapsi luotiin (havaittu prosessilistassa), kuoli Job-objektin sulkemisessa |
| WER (kaikki) | WerSvc STOPPED ennen ja jälkeen; ei WerFault/WerFaultSecure/wermgr-prosesseja; Application-lokissa 0 tapahtumaa 1000/1001/1002/1005 kontrollin ajalta; ei muutoksia WER- ja CrashDumps-hakemistoissa |

Havainto / finding: tämän CRT:n `__report_gsfailure` on kehyksetön: paluuosoite
epäonnistuneeseen funktioon on [ESP]:ssä ja EBP on yhä sen kehys (ei EBP+4). Luvun 18.4
kohta 4 tarkennettiin tämän mukaan. / The CRT's `__report_gsfailure` is frameless: the
return address is at [ESP]; criterion 4 adjusted.

Rajaus / limit: WER-ilmaisimille ei tehty positiivista kontrollia (se vaatisi oikean
kaatumisen ilman debuggeria, jolloin WER toimisi eristyksen ulkopuolella). Prosessilistan
seuranta havaitsi 300 ms:n prosessin; WerSvc-tila ja tapahtumaloki ovat pysyviä
ilmaisimia. / No positive control for the WER detectors (it would need WER to run outside
the isolation).

Päätelmä / conclusion: viidessä tarkoituksellisessa kaatumisessa (kuudes skenaario ei
kaadu) yksikään ilmaisin ei nähnyt WER-toimintaa; debuggeri lopetti prosessin ennen
jatkamista. Että WER *estyi*, on päätelmä mekanismista ja ilmaisimista, joilla ei ole
positiivista kontrollia. Tämä riittää käyttäjän ehdon mukaiseen jatkamiseen ilman
HKCU-muutosta. / In five deliberate crashes no detector saw WER activity; "prevented" is
inferred from the mechanism, the detectors have no positive control. No HKCU change.

### 19.5 Eristystarkistukset / Isolation checks (koeohjelma palomuurin alaisessa polussa / probe at the firewalled path)

| Tarkistus / check | Tulos / result |
|---|---|
| Tunnus / token | rajoitettu, keskitaso, ei ylläpitäjä / restricted, medium, not admin |
| `%TEMP%`, profiili, Dokumentit, `LOCALAPPDATA` | virhe 5 / error 5 |
| `D:\` (NTFS) | virhe 5 |
| `G:\` ja `G:\Oma Drive\` (Google Drive, ilmoittaa FAT32) | virhe 5 (testattu kirjoittamalla, ei oletettu ACL:stä) / error 5, tested by writing |
| HKCU: avaimen luonti, `KEY_SET_VALUE` | 5 ja 5; avainta ei jäänyt |
| TCP 192.0.2.1 | 10013 heti / at once |
| TCP6 [2001:db8::1] (tieto / info) | 10051 (ei IPv6-reittiä; palomuuria ei testattu IPv6:lla) / no IPv6 route |
| Silmukka / loopback (tieto) | yhteys syntyi (palomuuri ei suodata) |
| UDP | `sendto` onnistui; todiste vain pudotuslokista (19.7) |

Ei testattu / not tested: verkkojaot, nimetyt objektit, muut rekisterihaarat kuin HKCU,
siirrettävät asemat (ei kytkettynä). / Shares, named objects, other hives, removable drives.

### 19.6 Tarkennetut hyväksymiskriteerit / Refined acceptance criteria

Luvun 18.4 kriteerit koodina: `hkdebug.evaluate_h041(control, overflow, logs)`
(offline-testit: 31 tapausta, joissa yksikin poikkeama antaa "not confirmed"). Kriteerit
on kiinnitetty ennen ajoja; niitä ei muuteta tulosten nähtyä. Tarkennukset / refinements:
- (1) EXE:n ja kopion SHA-256 = `61034e5d…7d297d` ja PDB-GUID = `{B0500861-…}`
  (kiinnitetyt arvot, ei vain "kopio = lähde"); koeohjelma ennen ja jälkeen todennettu.
- (2) kontrolli: ei kohtalokasta poikkeusta, ei toisen mahdollisuuden poikkeuksia, ei
  odottamattomia int3-pysähdyksiä.
- (4) poikkeus 0xC0000409, alikoodi 2, RVA 0xA1295 (`__report_gsfailure+5`; HkMaalin
  `___report_gsfailure` 0x4A1290 on `mov ecx,2; int 29h`, kehyksetön); paluuosoite [ESP]:ssä
  (tai EBP-ketjussa) RVA 0x23237 tai 0x23200; EBP = `addfrec`in kehys; täsmälleen yksi
  kohtalokas poikkeus ja se on ensimmäinen toisen mahdollisuuden poikkeus (debuggeri
  säilyttää ensimmäisen); sama säie kuin alun keskeytyksessä; ei odottamattomia
  int3-pysähdyksiä; debuggeri lopetti prosessin (0xDEAD0409); **kutsuja `addtall`** luetaan
  alun keskeytyksestä (ylityksen jälkeen paluuosoite on ylikirjoitettu).
- (5) evästepaikka ≠ `__security_cookie ^ EBP` **ja** paluupaikka ≠ alussa tallennettu
  paluuosoite.
- (6) KILP.DAT 20 080 tavua **ja** pinon tavut [EBP−4, EBP+36) = tietueen 1 tavut
  [10 000, 10 040) (nollia; ks. 19.2).
- (7) kummassakin ajossa: ei WER-prosesseja (seurattu), ei WER-tapahtumia ohjelmasta,
  WerSvc pysähdyksissä lopussa, ei verkkopäätepisteitä, Job `TotalProcesses` 1, ei
  ulkopuolista tiedostoa eikä HKCU-avainta, ei muutoksia seuratuissa hakemistoissa paitsi
  ennalta kiinnitetty kohina (`hkdebug.outside_noise`: `%TEMP%\DiagOutputDir\RdClientAutoTrace\*.etl`,
  Etätyöpöytäasiakkaan jäljitys, nähty kaikissa C-11-ajoissa ohjelmasta riippumatta; WER-
  ja CrashDumps-muutokset eivät koskaan ole kohinaa); **ja** `verify_log.py`: koepaketit
  täsmäävät, ei pudotuksia ohjelman pid:llä, ei muiden kuin koeohjelmien lähteviä
  pudotuksia ajoikkunassa, jälkimmäisen koeohjelman paketit kopiossa (kopio kattaa ajon).
- Kontrolli (2): paluukohdan keskeytyksessä eväste ja paluuosoite ennallaan.

Keskeytyskohdat muuttavat prosessin koodia hetkellisesti (int3, palautetaan heti);
pinon asetteluun tai `/GS`-käytökseen ne eivät vaikuta [päätelty]. / Breakpoints touch
code bytes briefly; no effect on stack layout or /GS [inferred].

### 19.7 Ajomenettely ja järjestelmänvalvojan komennot / Run procedure and admin commands

Alkuperäiset lokitusasetukset (luettu 2026-11-24 ennen muutoksia) / original logging
settings: kaikissa profiileissa (Domain, Private, Public) `Enabled` True, `LogBlocked`
False, `LogAllowed` False, `LogFileName` `%systemroot%\system32\LogFiles\Firewall\pfirewall.log`,
`LogMaxSizeKilobytes` 4096. Säännöt `TP C11 HkMaali block in/out` käytössä.

1. Käyttäjä (ylläpitäjä): `run_hkmaali.LOGGING_ON_COMMAND` (lokitus testihakemistoon).
2. Ajuri: `--probe-only` → koepaketit.
3. Käyttäjä: `LOG_COPY_COMMAND` (lokikopio luettavaksi); ajuri: `verify_log.py` →
   `udp_verified.json` (portti-, aika- ja protokollatäsmäys).
4. Kontrolliajo 220: `--mode firewall --debug --watch kilptietue::addfrec --udp-verified …
   --script scripts/h041_v220.json --data Tests/data/hk/h041_v220.xml=KilpSrj.xml`.
5. Käyttäjä: lokikopio; ajuri: `verify_log.py` (koepaketit ennen ja jälkeen, ohjelman
   pid, kattavuus). Jos kontrolli tai eristys poikkeaa → keskeytys.
6. Ylitysajo 221 samoin (`h041_v221.json`, `h041_v221.xml`).
7. Käyttäjä: lokikopio ja `LOGGING_RESTORE_COMMAND`; ajuri: `verify_log.py` ja
   lokitusasetusten tarkistus alkuperäisiksi.

Ajojen tulokset kirjataan lukuun 19.9 (ei commitoida ilman hyväksyntää). / Results go to
19.9, not committed without approval.

### 19.8 Riippumaton tarkastus / Independent review (esiehdot / prerequisites)

`independent-verifier` (vain luku; ajoi offline-testit, dumpbin HkMaali.exe ja crash_probe):
C1 (aineisto), C2 (H-041b4), C3 (`addfrec`in kehys toimitetussa binäärissä; myös HkMaalin
`___report_gsfailure` kehyksetön) ja C5 (kaatumiskontrolli; `victim` uskollinen malli)
**vahvistettu**. Osittain: C4 (debuggeri), C6 (koeohjelma), C7 (arviointi heikompi kuin
dokumentti), C8 (lokin kattavuus), C9 (näppäinpolku vain staattisesti), C10 (liioittelut).
Korjattu ennen committia / fixed before the commit:

1. Lokin kattavuus: jälkimmäinen koeohjelma-ajo ja `covers_run`; `verify_log`-tulos osaksi
   kriteeriä 7 (`evaluate_h041(…, logs)`).
2. Kriteeri 1 kiinnitettyihin SHA-256- ja GUID-arvoihin.
3. Ensimmäinen kohtalokas poikkeus säilytetään, `fatal_count`; vaaditaan täsmälleen yksi;
   mahdoton offline-testi korvattu realistisella.
4. Kohinasääntö `outside_changes`ille kiinnitetty ennen ajoa (yllä).
5. Odottamattomat int3-pysähdykset (yli kaksi WOW64-käynnistyksen pysähdystä) kirjataan ja
   hylkäävät.
6. Kriteeri 4: poikkeuksen RVA, säie, `terminated_by_debugger`, 0xDEAD0409.
7. Puuttuvat negatiiviset testit lisätty (EBP, paluuosoite ennallaan, PDB, tiiviste,
   pistokkeet, ulkopuoliset muutokset, WerSvc, loki); 39 offline-testiä.
8. Koeohjelman aikakatkaisu kokonaisrajaksi (5,16 s itsetestissä); koeohjelmien tiivisteet
   itsetestin tulokseen; jäljelle jäänyt HKCU-avain hylätään `run_probe`ssa.
9. Dokumentti: 19.2 havainto vs. tarkistus, 19.3 aikaraja, 19.4 päätelmä (viisi kaatumista,
   "ei havaittu" eikä "estetty").

Lisäksi: jos debug-silmukka ei pääty, kahvoja ei suljeta (C4 d). Avoimet: `Sarjat[-1]`-luku
`inputkilp`in alussa (`Sarja()` = −1) ja `ProcessSpecialKey` Tab-näppäimelle – kontrolliajo
näyttää; WER-ilmaisimilla ei positiivista kontrollia. Tarkastajan mukaan esiehdot riittävät
kontrolliajoon; kohdat 1–4 piti korjata ennen 221-ajon luokittelua (korjattu).

Itsetesti korjausten jälkeen / self-test after the fixes: kaikki 6 osaa OK; crash control:
kussakin kaatumisessa `fatal_count` 1, 2 käynnistyspysähdystä, 0 odottamatonta.

### 19.9 Koeajot / Runs

Ei vielä ajettu. / Not run yet.
