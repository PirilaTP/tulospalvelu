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
   `kilprecsize0` 360, `kilppvtpsize0` 154 ja `vatpsize` 8; sitten
   `CHECK(kilpparam.kilprecsize <= KILPBUFSIZE)`. (b1) `/GS`: `k.read(1, 0)` →
   `getrec` lukee tasan `recl` tavua (`dbboxm/dbbox1.cpp:74`). (b2) Asan: `write()`-
   tai `pack()`-polku, koska ASan ei todennäköisesti näe ytimen `ReadFile`-kirjoitusta
   [päätelty]. (b3) vaihtoehto: `pack()` kasapuskuriin, jonka koko on tasan
   `KILPBUFSIZE` + vartijatavut / canary.
6. **Oikea toiminta / correct:** `kilprecsize <= 10000`, tai konfiguraatio hylätään
   ennen tietueiden käsittelyä.
7. **Vika / defect:** (a) 360 + 5·(154 + 252·8) = **11 210**; deterministinen, ei
   kaadu. (b1) poistumiskoodi `0xC0000409`; (b2) ASan stack-buffer-overflow;
   (b3) vartija muuttunut. **Kumoaa / refutes:** (a) ≤ 10000 tai käynnistystarkistus
   hylkää n_pv 5 / valuku 250.
8. **Automatisointi:** (a), (b) CI hosted. Toimitetun `HkMaali.exe`:n musta laatikko
   (kerros d) myöhemmin; HkKisaWin (bcc32, ei `/GS`) käsin.
9. **Muutokset:** vain testit. **Käännösaikaista `static_assert`ia ei tehdä**: 360 ja
   154 lasketaan ajossa kenttätaulukoista, joten käännösaikainen kaava olisi käsin
   kopioitu, ja nykyarvoilla se **estäisi käännöksen** (11 210 > 10 000). Ajonaikainen
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
| 1.7 | H-041 (a) + (b1) `/GS` + (b3) vartija; asettelutarkistus 360/154/8 | uusi / new Tests/core/HkCoreTests.cpp, LayoutHk.cpp | 〃 |
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
