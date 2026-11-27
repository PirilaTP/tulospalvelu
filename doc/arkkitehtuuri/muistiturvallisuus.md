# Muistiturvallisuus ja määrittelemätön käytös / Memory safety and undefined behaviour (C-4)

Erä C-4, ensimmäinen vaihe: koneellinen varoitusinventaario molemmilla kääntäjillä ja
sen luokittelu. Päivitetty 2026-11-24. Analysoitu revisio `f7fe553` (`TPsource/`
muuttumaton). Raakatulokset ja skriptit: `todisteet/c4/`. Havainnot:
`rekisteri/havainnot.md` (H-055–H-069, T-020, T-021).

Batch C-4, first step: a scripted warning inventory on both compilers and its triage.
Analysed revision `f7fe553`. Raw results and scripts: `todisteet/c4/`. Findings:
`rekisteri/havainnot.md` (H-055–H-069, T-020, T-021).

Kaikki tämän vaiheen tulokset ovat **staattisia**: koodia luettiin ja käännettiin, mutta
mitään ohjelmaa ei ajettu. Varoitusta ei ole luokiteltu viaksi ilman koodin lukemista.

All results of this step are **static**: code was read and compiled, no program was
run. No warning was classified as a defect without reading the code.

**Todennuksen tasot tässä dokumentissa / verification levels in this document.**
"Vahvistettu" (CONFIRMED, [todennettu]) tarkoittaa koko C-4:ssä **staattisesti
vahvistettua vikamekanismia**: koodi luettu, mekanismi osoitettu ja (rekisterin
havainnoissa) riippumattomasti tarkastettu. Se ei tarkoita, että vika olisi toistettu
testikehyksessä tai käynnissä olevassa ohjelmassa. C-4:n 23 uudesta havainnosta yksikään
ei ole toistettu tai ajossa vahvistettu; koko rekisterissä ajossa vahvistettu on vain
H-041 (C-11). Vaikutukset (kaatuminen, korruptio, hyödynnettävyys) ovat päätelmiä.

"Confirmed" throughout C-4 means a **statically confirmed defect mechanism** (code read,
mechanism shown, independently reviewed for register entries), not a defect reproduced in
a harness or in the running program. None of the 23 new C-4 findings has been reproduced
or confirmed by running; the only finding confirmed by running is still H-041 (C-11).

## 1. Kääntäjät ja käännökset / Compilers and builds

| Kääntäjä / Compiler | Versio / Version | Käytetty / Used for |
|---|---|---|
| MSVC | `cl.exe` 19.52.37029 (x86), VS 18 Build Tools, työkalut / tools 14.52.37025, toolset v145 | konsoliohjelmat ja kirjastot / console programs and libraries |
| bcc32 (classic) | Embarcadero C++ 7.80, C++Builder 13 CE (`Studio\37.0`) | GUI-ohjelmat ja niiden kirjastot / GUI programs and libraries |
| VS 2022 Build Tools | asennettu ilman VC-työkaluja / installed without VC tools | ei käytettävissä / not usable |

Ajetut kokoonpanot / configurations run:

| Kääntäjä / Compiler | Projekti / Project | Kokoonpano / Config | Tilat / Modes |
|---|---|---|---|
| MSVC | `VS/Libs/dbboxm/dbboxm.vcxproj`, `VS/Libs/tputilv2.vcxproj` | Release | W3, W4+analyze |
| MSVC | `VS/Hk/HkMaali/HkMaali.vcxproj` | Release | W3, W3+LTCG, W4+analyze |
| MSVC | `VS/Hk/HkMaali/HkMaali.vcxproj` | ReleaseVA200 (`VALUKU=200`) | W4+analyze (työkalupoikkeama, ks. alla / toolset deviation, see below) |
| MSVC | `VS/V/JukMaali520.vcxproj` | Release, Release150os (`MAXOSUUSLUKU=250`) | W3+LTCG, W4+analyze (Release myös W3 / also W3) |
| MSVC | `VS/V/JukLuenta520.vcxproj` | Release | W3, W3+LTCG, W4+analyze |
| bcc32 | `RADStudio10/DBboxm-XE.cbproj`, `Tputil-XE.cbproj`, `HkKisaWin.cbproj`, `ViestiWin.cbproj` | Release (projektin oletus / project default) | tallennettu komento / recorded command, + `-w -g0` |

Tilat / modes:

- **W3**: projektin omat asetukset, vain tulostepolut ohjattu väliaikaishakemistoon
  (`/Fo`, `/Fd`, `IntDir`, `OutDir`). / project settings, output paths redirected only.
- **W3+LTCG**: sama ja linkitys; toimitetut Release-projektit käyttävät `/GL`:ää, joten
  taustavaiheen varoitukset (C4789) syntyvät vasta linkityksessä. / same plus link; with
  `/GL`, back-end warnings (C4789) appear only at link time.
- **W4+analyze**: lisäksi `WarningLevel=Level4` ja `EnablePREfast=true` (`/W4 /analyze`)
  MSBuildin `ForceImportAfterCppTargets`-ominaisuudella. Projektitiedostoja ei muutettu.
  / adds `/W4 /analyze` via `ForceImportAfterCppTargets`; project files unchanged.
- **bcc32 tallennettu / recorded**: `build-gui.ps1`:n lokiin (`RADStudio10/build-logs/`)
  tallennettu komento sellaisenaan, vain `-o`, `-n` ja `-H=` ohjattu väliaikaishakemistoon.
  Tulos täsmäsi tallennettuun lokiin (2/4/18/2 varoitusta). / the recorded command with
  output paths redirected; matched the recorded logs exactly.
- **bcc32 `-w`**: sama ja `-w -g0` (kaikki varoitukset, ei enimmäismäärää). `-g0`
  tarvittiin, koska `Hk/HkMuotoilu.cpp` katkesi oletusrajaan (E2228). / all warnings, no
  limit; `-g0` was needed because one unit hit the default limit.

Alkuperäiset ja analyysikäännösten komennot: `todisteet/c4/commands.txt`.
Original and analysis compile commands: `todisteet/c4/commands.txt`.

### Poikkeamat ja ympäristöongelmat / Deviations and environment issues

- **ReleaseVA200** vaatii työkalut `v120_xp` (VS 2013 XP), joita ei ole asennettu
  (MSB8020). Ajettiin komentorivin `-p:PlatformToolset=v145`-ohituksella; pysyvää
  asetusta ei muutettu. Tulos oli samat 784 varoitusta kuin Releasella. / needs
  `v120_xp`, not installed; run with a command-line toolset override; same 784 warnings.
- **dbboxm-kokeilukäännös**: ensimmäinen kokeilu kirjoitti objektit projektin omaan
  `VS/Libs/dbboxm/Release/`-hakemistoon (git-ignoroitu), koska projekti määrittää
  `ObjectFileName`n erikseen. Käännösasetukset olivat projektin omat, joten tulos vastaa
  tavallista uudelleenkäännöstä; `TPexe/`-kirjastoa ei kirjoitettu. Sen jälkeen kaikki
  tulosteet ohjattiin. / The first trial wrote objects into the project's own ignored
  `Release/` folder (explicit `ObjectFileName`); same settings as a normal rebuild, the
  `TPexe/` library was not written. All later outputs were redirected.
- **Linkitys** luki kirjastot `TPexe/lib/V52/VS/release/`-hakemistosta (vain luku);
  exe-tiedostot syntyivät väliaikaishakemistoon. / linking read `TPexe` libraries only.
- C5298 (`ignoring spurious 'U+1a'`): 35 lähdetiedoston lopussa on DOS:n EOF-merkki;
  ei vaikutusta käännökseen. / 35 source files end with a DOS EOF byte; harmless.

## 2. Kattavuus / Coverage

| Mittari / Metric | Arvo / Value |
|---|---|
| `.cpp`/`.c`-tiedostoja versionhallinnassa (HEAD `72279b9`; analysoidussa revisiossa `f7fe553` 423, joista kääntämättä 35) / in the repository (at HEAD; 423 and 35 at `f7fe553`, the difference is 20 C-11 test files) | 443 |
| MSVC:n kääntämiä käännösyksiköitä / MSVC translation units | 210 |
| bcc32:n kääntämiä / bcc32 | 364 |
| molemmat / both | 186 |
| ei kumpikaan / neither | 55: 23 C-11-testitiedostoa (`Tests/`), 2 `tputilv2/LibTest.cbproj`-testiprojektin tiedostoa (`libtest.cpp`, `unittest.cpp`), 30 orpoa tiedostoa (`inventaario.md`, T-007) / 23 test files, 2 LibTest project files, 30 orphans |
| vain bcc32 / bcc32 only | `cbHk/` 76, `ViestiWin/` 57, `cbTp/` 36, muut / others 9 |

Kääntämättä jääneistä yksikään ei kuulu toimitettuihin ohjelmiin: ne eivät ole yhdenkään
tuotantoprojektin lähdelistassa eikä niitä oteta mukaan `#include`-direktiivillä, ja
toimitetut ohjelmat linkittyvät ilman niitä (konsolin HkMaali, ViestiMaali ja ViestiLuenta linkitettiin tässä analyysissä, GUI-ohjelmat tallennetussa käännöksessä `RADStudio10/build-logs/`). (Vaiheen 2 tarkistus korjasi MSVC:n
määrän 209 → 210: `tputilv2/esnimet-2.cpp` jäi alkuperäisestä laskennasta pois, koska
komentorivin viimeinen tiedosto päättyi lainausmerkkiin.) / None of the uncompiled files
belongs to a shipped program. Step 2 corrected the MSVC count from 209 to 210.

Puuttuvat tai rajatut / missing or limited:

- **GUI-koodi (178 käännösyksikköä) vain bcc32:lla.** Classic bcc32 ei tarkista
  muotoilumerkkijonoja, ei tee polkuanalyysiä eikä puskurirajoja. GUI-lomakkeiden
  muistivirheet jäävät tämän vaiheen ulkopuolelle. / GUI code only through bcc32, which
  does no format, path or bounds analysis.
- **Debug-, Demo-, Debug250-kokoonpanot** ja `JukLuenta520`:n muut kokoonpanot (T-017:
  samat määritykset) eivät mukana. / not run.
- **bcc32c/bcc64 (Clang)** saatavilla, mutta niillä tuotantokoodi ei ole käännetty
  koskaan; ei ajettu. / available but never used for production; not run.
- **Testit** (`Tests/`, TpCore Asan) eivät kuuluneet tähän vaiheeseen. / tests out of
  scope.
- Otsikkotiedostot analysoitiin vain niiden käännösyksiköiden kautta. / headers only via
  their translation units.

## 3. Varoitusten määrät / Warning counts

| | Rivejä / rows | Yksilöllisiä / unique (tiedosto, rivi, koodi / file, line, code) |
|---|--:|--:|
| Kaikki / all | 11 027 | 7 036 (järjestelmäotsikot / system headers 6) |
| MSVC W3 (perustaso / baseline) | 106 | 69 |
| MSVC W3+LTCG | 36 | 29 (C4789 vain linkityksessä / at link only: 2) |
| MSVC W4+analyze | 3 751 | 1 603 (136 tiedostoa / files, 47 koodia / codes) |
| bcc32 tallennettu / recorded | 26 | 22 |
| bcc32 `-w` | 7 108 | 5 425 (397 tiedostoa / files, 12 koodia / codes) |

MSVC W4+analyze ryhmittäin (yksilölliset) / by group (unique):

| Ryhmä / Group | Koodit / Codes | Määrä / Count | Käsitelty / Triaged |
|---|---|--:|---|
| Puskurit ja indeksit / buffers, indices | C4789, C6385, C6386, C6200, C6201, C6053, C6054 | 139 | kyllä / yes |
| Alustamaton muisti / uninitialised | C6001, C28199, C4701, C4703 | 73 | kyllä / yes |
| NULL | C6011, C6387, C28183 | 34 | kyllä / yes |
| Muotoilu / format | C4477, C4474, C4778, C6302, C6066, C6067, C6271, C6273, C6340 | 58 | verrattu H-003:een / compared with H-003 |
| Pino / stack | C6262 | 108 | arvioitu ryhmänä / assessed as a group |
| Logiikka / logic | C6290, C6313, C6292, C4065 | 6 | kyllä / yes (paitsi / except C4065) |
| Muunnokset / conversions | C4244, C4245, C4389, C4018 | 633 | ei / no |
| Paluuarvo / return value | C6031 | 50 | ei (C-3: H-053) / no |
| Muut / other | C4100, C4189, C4459, C6244, C6246, C5298, C4456 ym. | 504 | ei / no |

bcc32 `-w` (yksilölliset / unique): W8057 2 831, W8071 760, W8004 572, W8059 373,
W8061 349, W8080 277, W8012 215, W8079 21, W8027 21, W8013 4, W8008 1, W8066 1.
Käsitelty: W8013, W8079, W8008; ainoa W8066 (`ViestiWin/TulostusUnit2.cpp:976`, saavuttamaton koodi `if (0)`-lohkossa) luettiin käsin vaiheessa 2: väärä positiivinen. W8059 (pakkauksen muutos) kuuluu erään C-5.

Kokoonpanojen erot / configuration differences: Release150os eroaa Releasesta vain
pinokäytössä (C6262, 89 riviä); ReleaseVA200 ei eroa lainkaan. / Release150os differs
only in stack size warnings; ReleaseVA200 not at all.

## 4. Luokittelu / Triage

Kolme `defect-concurrency-auditor`-agenttia luokitteli 275 tärkeimmän ryhmän varoitusta
lukemalla koodin (`todisteet/c4/triage_*.csv` syötteinä). Uudet merkittävät havainnot
tarkastettiin kahdella `independent-verifier`-agentilla.

Three auditors triaged 275 priority warnings by reading the code; new significant
findings were reviewed by two independent verifiers.

| Erä / Set | Rivejä / rows | Vahvistettu / confirmed | Todennäköinen / likely | Tutkittava / investigate | Väärä positiivinen / false positive | Ympäristö / environment |
|---|--:|--:|--:|--:|--:|--:|
| A1 `Hk/`, `HkConsole/` | 96 | 20 | 12 | 5 | 59 | 0 |
| A2 `Tp/`, `tputilv2/`, `dbboxm/`, `com/`, `cbHk/` (W8079) | 80 | 5 | 22 | 0 | 53 | 0 |
| B `Juk/`, `JukConsole/`, `ViestiWin/` | 99 | 12 | 30 | 28 | 29 | 0 |
| **Yhteensä / total** | **275** | **37** | **64** | **33** | **141** | **0** |

Riveittäiset luokitukset: `todisteet/c4/triage_*_luokitus.csv`. Vaiheen 2 tarkistus
korjasi alkuperäiset yhteenvedon luvut (274 riviä; 38/64/31/141): A2-agentti jätti
pois rivin `tputilv2/QSORTINT.cpp:71` C6001 (sama perustelu kuin samalla rivillä
olevalla C4701:llä → väärä positiivinen), B-agentti jätti pois rivin
`Juk/VXml.cpp:1853` (pääistunto luokitteli: todennäköinen, matala; literaalin `L"RT"`
yliluku, kun `taslaji % 10` ≥ 3) ja laski omat summansa väärin. "Vahvistettu"
tarkoittaa koodista vahvistettua, ei ajettua. "Todennäköisistä" 9 laukeaa vain muistin
loppuessa. Tutkittavat rivit käsitellään vaiheessa 2.

Per-row verdicts: `todisteet/c4/triage_*_luokitus.csv`. Step 2 corrected the original
totals (274; 38/64/31/141): auditor A2 omitted `tputilv2/QSORTINT.cpp:71` C6001 (false
positive, same reason as C4701 there), auditor B omitted `Juk/VXml.cpp:1853`
(classified likely/low by the main session) and mis-added its totals. "Confirmed" means
confirmed from code, not run. The investigate rows are handled in step 2.

### Väärät positiiviset ryhmittäin / False positives by pattern

| Kuvio / Pattern | Esimerkit / Examples |
|---|---|
| Koko olion `memset`/`memcpy`/`memcmp` ensimmäisestä jäsenestä (ei vtablea) / whole-object idiom | `Hk/HkEmit.cpp:145`, `Hk/HkSrj.cpp:668`, `Juk/VEmit.cpp:176`, `Juk/vSarjat.cpp:51` |
| `extern int aikaero[NREGNLY]`, määrittely `[NREGNLY+1]`: muodollisesti virheellinen C++ (ei vaadi diagnostiikkaa), käytännössä harmiton / declaration smaller than definition: ill-formed (no diagnostic required), harmless in practice | `Hk/HkIV.cpp:3390`, `Hk/HkInit.cpp:3145` (`Hk/HkDeclare.h:765`, `Hk/HkGlobals.cpp:385`) |
| Tarkoituksellinen kirjoitus seuraavaan kenttään saman rakenteen sisällä: muodollisesti määrittelemätön käytös, käytännössä harmiton / deliberate write into the next member of the same struct: undefined behaviour formally, harmless in practice | `Hk/HkIV.cpp:983`, `:2335`, `Juk/VIv.cpp:973`, `:1902` |
| Kuollut koodi (dbboxm:n B-puu, `loadtree`) / dead code | `dbboxm/findkey.cpp:54`, `tputilv2/xmlparser.cpp:450` (huom. `Tp/TpApu.cpp:817` on todellinen vika kutsumattomassa funktiossa, luokiteltu todennäköiseksi / real defect in an uncalled function, classified likely) |
| Analysoija ei näe invarianttia / invariant not visible | `Hk/HkIx.cpp:1357`, `Juk/VIx.cpp:385` (H-042 voi rikkoa / H-042 can break it) |
| Nollattu puskuri ja rajattu kopio / zeroed buffer, bounded copy | `Hk/HkDat.cpp:558`, `Tp/TpApu.cpp:428`, `Juk/vkilp.cpp:976` |
| `wctomb`-oletus `MB_LEN_MAX` 5 / assumption | `tputilv2/wcr2oem.cpp:56` |

### Pinokäyttö / Stack usage (C6262)

108 funktiota käyttää yli 16 kt pinoa, viestissä 50–100 kt (kilpailijatietue
paikallisena muuttujana). `_beginthread`-kutsujen koko (8–160 kt) on vain alkuvaraus;
varauksen yläraja tulee exe-otsakkeesta: HkMaali 1 Mt, ViestiMaali 2 000 000 tavua (noin 1,9 MiB), HkKisaWin 4 Mt,
ViestiWin 8 Mt (mitattu toimitetuista binääreistä). Yksittäinen funktio ei ylitä rajaa;
syvät kutsuketjut viestissä (yli noin 20 tällaista kehystä) ylittäisivät. Ei kirjattu
viaksi. [päätelty]

108 functions use over 16 KB of stack, relay ones 50–100 KB. The `_beginthread` size is
only the initial commit; the reserve comes from the exe header (1/2/4/8 MB measured).
Not recorded as a defect. [inferred]

## 5. Uudet havainnot / New findings

Riippumattomasti tarkastetut / independently reviewed (`rekisteri/havainnot.md`):

| ID | Vakavuus / Severity | Lyhyesti / Summary | Varoitus / Warning | Ohjelmat / Programs |
|---|---|---|---|---|
| H-055 | korkea / high | emiTag-sanoman indeksi pinotaulukkoon `controls[500]` tarkistamatta (`Tp/TpLaitteet.cpp:382`) / device message index into stack array | ei (löytyi vierestä) / none | kaikki 5 / all |
| H-056 | korkea ARES:llä / high with ARES | `lue_ARES` jää ikuiseen silmukkaan; tavun kasaylitys (`Tp/TpLaitteet.cpp:711-733`) / infinite loop | C6386 | HK |
| H-057 | keskitaso / medium | XML-lukijat: `depth` −1, `</>` kaataa, syvyysrajaa ei ole (`tputilv2/xmlparser.cpp:267-274`, `:349-350`) | C6385 ×10 | kaikki / all |
| H-058 | keskitaso / medium | `lue_radat`: `rata[]`-kasaylitys; viestissä yli 50 rastia (`Hk/HkEmit.cpp:2896-2937`, `Juk/VEmit.cpp:3542-3561`) | C6385/C6386 | kaikki / all |
| H-059 | keskitaso / medium | `AjatTiedostoon`: `ln[120]`-pinoylivuoto ja `fclose(NULL)` (`Hk/HkAjat.cpp:424`, `:471`) | C6387, C6001 | HK |
| H-060 | keskitaso / medium | ampumapaikan indeksi laitteen/verkon sanomasta (`Hk/HkSak.cpp:159-160`) | ei / none | HK |
| H-061 | keskitaso / medium | `luepaketti`: alustamaton `pkglen`; `class_len`-indeksi (`Tp/TpComY.cpp:708`, `:750`) | C6001 | kaikki / all |
| H-062 | keskitaso / medium | `sendwline` kirjoittaa alustamatonta pinoa tiedostoon (`tputilv2/Wrttext.cpp:386`) | C6001, C6054 | kaikki / all |
| H-063 | keskitaso / medium | `seur_rata`: `fclose(NULL)` kaataa ViestiMaali.exe:n (`Juk/VTulostus.cpp:8631`) | C6387 | ViestiMaali |
| H-064 | keskitaso / medium | `tark_partio`: NULL-osoitin (`Juk/VEmit.cpp:2388`) | C6011 | ViestiMaali |
| H-065 | keskitaso / medium | `SARJAYHD.LST`: `KAIKKI` jättää indeksin alustamatta (`Juk/vSarjat.cpp:282-285`) | C6001 | viesti / relay |
| H-066 | matala / low | seuran avain 22 tavua, käytetään 30/50 (`Hk/HkApu.cpp:752`, `Juk/vApu.cpp:882`) | C6385 | kaikki / all |
| H-067 | matala / low | `mrivi[3]` indeksillä 3 (`Hk/HkTulostus.cpp:74`) | C6200 | konsolit / consoles |
| H-068 | matala / low | alustamaton osoitin `ss` (`HkConsole/HkCons2.cpp:1244`) | C6001 | HkMaali |
| H-069 | matala / low | `NumKaikki`, `luelaina` ei koskaan asetettu (`ViestiWin/UnitArvonta.cpp:373`) | W8013 | ViestiWin |

Laukaisijat / triggers: H-055, H-056, H-060 laitteen tai verkon sanoma; H-061 verkon
paketti; H-057, H-058, H-065 operaattorin tai tuonnin tiedosto; H-059, H-062, H-063,
H-064 tavallinen käyttö tietyissä asetuksissa. / device or network message; network
packet; operator or import file; normal use under certain settings.

Tarkastus korjasi / the review corrected: H-055:n negatiivinen indeksi ei ole
saavutettavissa MSVC:n `atoi`lla; H-061:n väite "ei `/J`" oli väärä (kaikki konsolin
Release-kokoonpanot käyttävät `/J`; vain HkKisaWin on etumerkillinen); `Tp/TpApu.cpp:164`
ja GUI:n `seur_rata` eivät kaadu (C++Builderin `fclose(NULL)` palauttaa EOF,
C++Builderin RTL-lähde / RTL source `fclose.c` rivi / line 45); H-064 ei koske GUI:ta; H-065:n `MAXYHD`-ylitys vain
erikoistapauksissa; H-057:n `txtbuf[-1]` käytännössä saavuttamaton. Tarkastajat löysivät
lisäksi `loadbranch`-syvyysrajan puuttumisen (H-057) ja viestin yli 50 rastin ylivuodon
(H-058); pääistunto tarkisti molemmat koodista.

### Muut löydökset, ei riippumattomasti tarkastettu / Other findings, not independently reviewed

Matala vakavuus tai kapea laukaisu; kirjattu tänne, ei rekisteriin. / Low severity or
narrow trigger; recorded here, not in the register.

- `Juk/VTulostus.cpp:8664-8682`: `HAJONNAT.LST`, data ennen `S`/`O`-riviä → kirjoitus
  alustamattoman `haj0`:n kautta; `O`-arvoa ei rajata ylhäältä.
- `JukConsole/laskent0.cpp:477-545`: kynnysarvoeditori, `srj` kasvaa yli `sarjaluku`n;
  tiedostoon kirjoitetaan vain puolet arvoista.
- `JukConsole/laskent0.cpp:357-395` `copydfile`: `CBUFL` 4096 < `KILPRECSIZE` →
  kasan yliluku ja ikuinen silmukka lukon kanssa; **ei kutsujia**. HK:n vastine ei käytä
  puskuria (`HkConsole/HkCons0.cpp:345`), joten epäily kumottiin HK:n osalta.
- `Hk/HkEmit.cpp:1556-1566` `aikaruutu`: `st[15]` ylivuotaa, kun `kno` ≥ 10000.
- `Hk/HkTls.cpp:878-905` `asakkoStr`: staattinen `st[20]` ylivuotaa kuudella
  moninumeroisella sakolla.
- `Tp/TpComY.cpp:2725`: `uusintalahetys` `LUENTA`-käännöksessä alustamaton aikaikkuna.
- `Tp/TpComY.cpp:874` (C6313): RS-232:n viive ei koskaan toteudu (`comtpRS` 0).
- `Hk/HkTulostus.cpp:5721` (C6290): operaattorien sidonta väärin.
- Alustamattomia arvoja tulosteissa tai päätöksissä: `Hk/HkAjat.cpp:465`,
  `Hk/HkCom32.cpp:957`, `Hk/HkIV.cpp:688`, `Hk/HkEmit.cpp:2310`,
  `Hk/HkXmlTulokset.cpp:874`, `:1249`, `Juk/VCom.cpp:989`, `:1358`, `Juk/VEmit.cpp:1319`,
  `:5965`, `tputilv2/DateNo.cpp:133`.
- Yliluvut / over-reads: `HkConsole/HkCons2.cpp:333`, `HkConsole/HkCons0.cpp:854`,
  `Hk/HkTulostus.cpp:823`; `tputilv2/QSORTINT.cpp:117`, `tputilv2/dbindex.cpp:237`
  (lajittelun lopussa, ei vaikutusta / at end of sort, harmless).
- `cbTp/UnitAmpPaikat.cpp:155-191`: GUI-ruudukon indeksi (osa H-060:tä).
- `tputilv2/Inputwstr.cpp:59`: tiedostokohtainen `monirivi` varjostaa globaalin
  (toiminnallinen).

Tutkittavat (33 riviä, `todisteet/c4/triage_*.csv` + agenttiraportit tässä luvussa):
mm. `Hk/HkEmit.cpp:723` (`Point[MAXNPISTE]`, `tmpkdi[2000]` rajat), `Juk/VTulostus.cpp:1019`
(`ossijat`), `Juk/VIv.cpp:786`, `:1690`, `:2226`, `ViestiWin/UnitGAZ.cpp` (W8079 ×8),
`JukConsole/vcons2.cpp:2064`, `:2200`, `Juk/XMLtulos.cpp:326`, `:446`.
Investigate rows (33) include the above.

## 6. Kääntäjäkohtaiset erot / Compiler-specific differences

- `fclose(NULL)`: MSVC:n Release-CRT kutsuu oletusarvoista virheellisen parametrin
  käsittelijää, joka päättää prosessin; lähdepuussa ei ole
  `_set_invalid_parameter_handler`ia. C++Builder palauttaa EOF. → H-059 ja H-063
  kaatavat vain konsoliohjelmat. [todennettu] koodista ja RTL-lähteestä.
- `char`-etumerkki: konsolin Release-kokoonpanot ja `Tputil-XE`, `ViestiWin` käyttävät
  etumerkitöntä `char`ia (`/J`, `-K`); samoin `DBboxm-XE`; **HkKisaWin ei**. H-061:n
  luokkaindeksi voi siksi olla negatiivinen vain HkKisaWinissä.
- /GS-pinosuoja on MSVC:ssä oletuksena päällä, bcc32:ssa ei vastaavaa. /GS havaitsee
  vain yhtenäisen ylivuodon, joka ulottuu evästeeseen ennen funktion paluuta (esim.
  H-059, H-041: C-11-ajon 0xC0000409/2 `addfrec`in evästetarkistuksessa). H-055:n
  yksittäinen indeksoitu kirjoitus voi ohittaa evästeen kokonaan. GUI:ssa vastaavat
  ylivuodot voivat korruptoida muistia hiljaa. [päätelty]
- `atoi`-ylivuoto: MSVC rajaa arvoon `INT_MAX`; bcc32:n käytös [tuntematon] (H-055).
- C4789 syntyy vain `/GL`-linkityksessä; `/c`-käännös ilman linkitystä ei sitä näytä.

- `fclose(NULL)`: MSVC Release ends the process; C++Builder returns EOF. `char`
  signedness: all console Release configs, Tputil-XE and ViestiWin unsigned; HkKisaWin
  signed. /GS in MSVC only and only for contiguous overflows that reach the cookie
  (not H-055's single indexed write); the GUI may corrupt silently. [inferred] `atoi` overflow on bcc32 unknown. C4789 only with `/GL` at
  link time.

## 7. Rajoitukset / Limitations

- Kaikki luokittelu on staattista; yhtäkään uutta havaintoa ei ole toistettu
  testikehyksessä eikä ajettu. / all static; nothing reproduced or run.
- Noin 93 % varoituksista (6 569 / 7 030) on luokissa, joita ei käyty läpi; erityisesti
  kokonaislukumuunnokset (C4244 612, W8071 760) ja etumerkkivertailut (W8012 215) voivat
  kätkeä ylivuotoja. / 93 % of warnings in untriaged groups; conversions may hide
  overflows.
- GUI-lomakkeiden logiikka analysoitiin vain bcc32:n heikoilla varoituksilla.
- Tunnetut puskuriviat H-041, H-042, H-052, H-055 ja H-060 eivät näkyneet yhdessäkään
  varoituksessa: varoitusinventaario ei korvaa kohdennettua lukemista. / known buffer
  defects produced no warning.
- 33 tutkittavaa riviä (A1 5, B 28) jäi vaiheessa 1 ratkaisematta.

## 8. Ehdotus seuraavaksi vaiheeksi / Proposed next step

*Vaiheen 1 ehdotus. Käyttäjä rajasi vaiheen 2 toisin (ei ASan-toistoja); ks. luku 9. / Step 1 proposal; step 2 was scoped differently, see section 9.*

**C-4 vaihe 2: korkean ja keskitason uusien havaintojen toisto testikehyksessä ja
kohdennettu luenta.** Rajaus:

1. Toista TpCore-testikehyksessä (ASan, ei tuotantokoodin muutoksia) H-055
   (`tulk_emiTag`-syöte `P600-…`), H-056 (`lue_ARES` 41 tavua ilman EOT:ta ja NUL:ia),
   H-057 (`</>` ja sulkeva tagi ylimmällä tasolla), H-058 (käsin tehty `RADAT.LST`),
   H-059 (pitkä nimi ja seura), H-060 (`tall_ampsakko`-sanoma). Testikehys on olemassa;
   uusia kehyksiä ei rakenneta.
2. Ratkaise H-061 disassemblyllä (`luepaketti`, `pkglen`:n sijainti toimitetussa
   HkMaali.exe:ssä), ei ajoa.
3. Lue 33 tutkittavaa riviä ja kokonaislukumuunnosvaroitukset (C4244/W8071) niissä
   funktioissa, jotka käsittelevät verkon, laitteiden tai tiedostojen dataa.
4. Ei GUI-ajoja, ei CI-muutoksia, ei korjauksia.

**C-4 step 2:** reproduce H-055–H-060 in the existing TpCore harness under ASan,
settle H-061 by disassembly, read the 33 investigate rows and conversion warnings in
input-handling functions. No GUI runs, CI changes or fixes.

## 9. Vaihe 2: rajaus ja menetelmä / Step 2: scope and method

Käyttäjä hyväksyi vaiheen 1 ja rajasi vaiheen 2 analyysiksi: ei ASan-testisarjoja, ei
uusia testikehyksiä, ei GUI-ajoja, ei korjauksia. Luvun 8 ehdotuksesta toteutettiin
kohdat 2–3 (lukemalla); toistoja ei tehty. Kaikki vaiheen 2 tulokset ovat staattisia.

The user approved step 1 and limited step 2 to analysis: no ASan series, no new
harnesses, no GUI runs, no fixes. Items 2–3 of section 8 were done by reading; no
reproduction. All step 2 results are static.

| Työ / Work | Agentti / Agent | Tulos / Result |
|---|---|---|
| Vaiheen 1 johtopäätösten uudelleentarkastus K1–K9 / re-review of step 1 | `independent-verifier` | 5 vahvistettu, 4 osittain; korjaukset tehty (luku 2, 4, 6) / 5 confirmed, 4 partly; corrected |
| ±1-poikkeama / count discrepancy | pääistunto / main session | ks. luku 4 / see section 4 |
| H-055 ja H-056 syvemmin / in depth | `architecture-analyst` | luku 10 / section 10 |
| 33 tutkittavaa riviä / investigate rows | `defect-concurrency-auditor` | luku 11, `todisteet/c4/tutkittavat_vaihe2.csv` |
| Muunnosvaroitukset riskitiedostoissa (522 riviä) / conversion warnings | `defect-concurrency-auditor` | luku 12, `todisteet/c4/muunnos_ehdokkaat.csv` |
| Uusien korkeiden tarkastus / review of new high findings | `independent-verifier` ×2 | luku 14 |
| Kattavuuden arviointi / coverage assessment | pääistunto / main session | luku 13 |

Erän 1 kokonaisuus commitoitiin paikallisesti ennen vaihetta 2 (`81bcad9`). / Step 1
was committed locally before step 2.

## 10. H-055 ja H-056 syvemmin / in depth

### H-055 `tulk_emiTag`

| Ohjelma / Program | Vikapolku / Defect path | Syöte / Input |
|---|---|---|
| HkMaali (kaikki kokoonpanot / all configs) | kyllä / yes | sarja/USB-lukija (`EMITAGn=`), HTTP-pollaus (`ETGPRS`/`ETSQL`) |
| ViestiMaali (kaikki / all) | kyllä / yes | sarja/USB, HTTP |
| ViestiLuenta | kyllä / yes | sarja/USB (`EMITHTTP` ei määritelty / not defined) |
| HkKisaWin | kyllä / yes | sarja/USB, **TCP-palvelin `TCPS`** (lomake / form `cbTp/UnitEmiTag.cpp:331-334`), HTTP |
| ViestiWin | kyllä / yes | sarja/USB, **TCP-palvelin `TCPS`**, HTTP |

- **Kutsuketju / call chain:** `lue_regnlyThread` (säie / thread, `Tp/TpComY.cpp:2233`) →
  `lue_regnly` → `lue_EMITAG` (`Tp/TpLaitteet.cpp:1299-1373`) → `tall_emit`
  (`Hk/HkIV.cpp:1074`, `Juk/VIv.cpp:1065`) → `tulk_emiTag`. HTTP: `etHaku`/`vahaku` →
  `httphaku(..., secure = 0, ...)` (`Hk/HkIV.cpp:3540`) → rivit yli 30 merkkiä →
  `tulk_emiTag` (`Hk/HkIV.cpp:3591`, `Juk/VIv.cpp:2485`).
- **Tarkistukset / checks:** puskurin koko rajataan (`r_buflen` 15 000); kehyksessä
  vaaditaan ETX yli 15 tavun kohdalla, STX:ää ei vaadita alussa, **tarkistussummaa ei ole**;
  `P`-kentässä vaaditaan kaksi `-`-merkkiä; indeksiä `i` ei rajata (`j > 255`
  -tarkistus on kommentoitu pois). Mikään tarkistus ei estä vikaa. [todennettu]
- **Syötteen luotettavuus / trust:** laiteperäinen (lukijan sarja- tai USB-väylä);
  **verkko**: GUI:n TCP-palvelin kuuntelee kaikissa osoitteissa ilman vertaisen
  suodatusta (`com/WinUDP.cpp:645-676`, `:795`), joten kuka tahansa samassa verkossa voi
  lähettää kehyksen; HTTP-vastauksen voi väärentää kuka tahansa yhteyden reitillä
  (salaamaton HTTP, oletuspalvelin `Hk/HkGlobals.cpp:624`). Voiko kilpailijan emiTag-kortti
  vaikuttaa `P`-indeksiin, on [tuntematon].
- **Ehdot / conditions:** emiTag-lukija tai HTTP-pollaus käytössä; yksi kehys, jossa
  `P<i>-<j>-...` ja `i` ≥ 500.
- **Pahin perusteltu vaikutus / worst justified impact:** 2 + 4 tavun kirjoitus
  lähettäjän valitsemalla siirtymällä ja osin valitulla arvolla. 32-bittisessä
  aritmetiikassa `i` ≥ 2^29 kiertää osoitteen myös taulukon alapuolelle, joten
  "negatiivinen indeksi ei mahdollinen" ei rajaa ulottuvuutta. Yksittäinen indeksoitu
  kirjoitus voi ohittaa /GS-evästeen (MSVC); bcc32-GUI:ssa evästettä ei ole.
  **Perusteltu:** tietojen korruptio ja koko ajanotto-ohjelman kaatuminen. **Spekulatiivinen:**
  koodin suoritus (vaatii kehysasettelun ja ASLR/DEP-ohituksen) [tuntematon]. [päätelty]
- **Altistavat tilanteet / exposure:** suunnistuksen tai hiihdon maali emiTag-lukijalla
  (häiriintynyt linja, lukijan laiteohjelmistovika); GUI:n `TCPS`-tila tapahtuman lähiverkossa;
  GPRS/online-väliajat julkisen tai tapahtumapaikan Wi-Fin yli.
- **Vakavuus / severity:** korkea (säilyy) / high (kept).

### H-056 `lue_ARES` (ja `lue_Sirit`, `lue_EMITAG`)

| Ohjelma / Program | ARES | Sirit (`LAJUNEN`) | emiTag |
|---|---|---|---|
| HkMaali, HkKisaWin | kyllä (`ARESn=`, sarja tai TCP-asiakas) / yes | kyllä / yes | kyllä / yes |
| ViestiMaali, ViestiWin | ei avainsanaa / no keyword | kyllä / yes | kyllä / yes |
| ViestiLuenta | ei / no | ei (`LAJUNEN` puuttuu) / no | kyllä / yes |

- **Kutsuketju / chain:** `lue_regnlyThread` → `lue_regnly` → `lue_ARES`
  (`Tp/TpLaitteet.cpp:696-736`) / `lue_Sirit` (`:741-797`) / `lue_EMITAG`.
- **Mekanismi / mechanism:** `lue_ARES` synkronoi SOH:hon ja silmukoi `while (*nmsg >= 41)`;
  jos EOT ei ole kohdissa 19 ja 40 eikä puskurissa ole NUL-tavua, `strstr` palauttaa
  puskurin alun (tai NULL) eikä mikään haara etene → ikuinen silmukka. ARES-kehys on
  ASCII-muotoinen, joten yksi kadonnut tai ylimääräinen tavu riittää. [todennettu]
  silmukka, [päätelty] kehyksen muoto (Q44). `lue_Sirit`: osittainen rivi ilman `\r`:ää
  riittää samaan. `lue_EMITAG`: 15 000 tavua ilman ETX:ää pysäyttää lukijan pysyvästi.
- **Vaikutus / impact:** lukijasäie pyörii 100 % yhdellä ytimellä, kyseisen laitteen
  ajanottosyöte pysähtyy eikä ohjelma voi lopettaa säiettä. Lukkoa ei pidetä, joten muu
  ohjelma jatkaa. Yhden tavun kasakirjoitus vaatii noin 15 kt:n purskeen ja osuu
  todennäköisesti varauksen täytetilaan. Koodin suoritusta ei voi perustella.
- **Syöte / input:** laiteperäinen (sarjaportti) tai ARES-TCP-yhdyskäytävä (lähtevä
  yhteys).
- **Altistavat tilanteet / exposure:** hiihdon tai ampumahiihdon maali ARES-laitteella
  RS-232:n yli: linjahäiriö, löysä kaapeli, väärä pariteetti, laitteen uudelleenkäynnistys
  kesken kehyksen.
- **Vakavuus / severity:** korkea ARES-, Sirit- tai emiTag-käytössä (ajanoton
  menetys), muuten ei vaikutusta / high where the device is used.

## 11. Tutkittavat rivit / Investigate rows (33)

| Luokka / Category | Määrä / Count | Rivit / Rows |
|---|--:|---|
| Vahvistettu / confirmed | 11 | 9 uutta (H-069:n lisäkohta `ViestiWin/UnitAjanotto.cpp:1096`, H-075 `JukConsole/vcons2.cpp:2200`, H-077 `ViestiWin/UnitGAZ.cpp:1047`, `:1072`, matalat näyttö- ja tulostearvot) + 2 = H-058 (`Juk/VEmit.cpp:3500`, `:3553`) |
| Todennäköinen / likely | 6 | H-076 (`Juk/VTulostus.cpp:1019`, `:1021`), `Hk/HkEmit.cpp:723`, `Juk/VTulostus.cpp:1868`, `Juk/VIv.cpp:1690`, `Juk/VIv.cpp:786` (T-020) |
| Jää auki / remains open | 2 | `Hk/HkXmlTulokset.cpp:1211`, `Juk/VIv.cpp:2226` (vain arvo tai näyttö / value or display only) |
| Väärä positiivinen / false positive | 14 | mm. kuollut `seur_kno`, W8079-tyyppisekoitukset / e.g. dead code, type mixes |

Tarkastus kumosi `ViestiWin/UnitGAZ.cpp:997`:n (`TauluJako`): kutsujat rajaavat
`nikk` ≤ 4. Rivikohtaiset tiedot: `todisteet/c4/tutkittavat_vaihe2.csv`. / The review
refuted `UnitGAZ.cpp:997`; per-row data in the CSV.

## 12. Muunnosvaroitukset / Conversion warnings

522 yksilöllistä C4244/C4245/C4389/C4018/W8071/W8012-riviä tiedostoissa, jotka käsittelevät
verkkopaketteja, laitesanomia, kilpailijatietueita, emit-dataa ja tiedostoja, ryhmiteltiin
juurisyyn mukaan ja ryhmien edustajat jäljitettiin käyttöön asti.

522 unique conversion warnings in input-handling files were grouped by root cause and
representatives were traced to their use.

| Kuvio / Pattern | ~Määrä / count | Muistin kannalta / Memory-relevant | Tulos / Verdict |
|---|--:|---|---|
| Merkki- ja taulukkomuunnokset (int → char/wchar_t) / character conversions | 108 | ei / no | harmiton / benign |
| `class_len`/`combufsize` → pakettipituus / packet length | 33 | pituus / length | harmiton; tunnetut H-025, H-032, H-061 |
| Lähetyspuolen INT16-kentät / send-side INT16 fields | 23 | vastaanottajan indeksi / receiver index | harmiton (vastaanottaja tarkistaa; H-033) |
| `aikatp`/`emittp`-kentät / fields | 85 | osin / partly | harmiton paitsi emiTag `F` (H-055) |
| Konfiguraatiotiedostojen INT16-indeksit / config INT16 indices | 55 | kyllä / yes | **H-074** |
| Emit- ja ratarakenteet / emit and course structures | 83 | osin / partly | harmiton; negatiivinen väliaika kiertyy 65xxx:ksi ja pidentää H-073:n tekstiä |
| `sizeof`-vertailut / comparisons | 34 | rajatarkistukset / bounds checks | suojaavia / protective |
| I/O-koot / sizes | 9 | kyllä / yes | H-053, H-049; `Tp/TpComY.cpp:3022` → **H-071** |
| XML/jäsennin / parser | 44 | syvyys / depth | H-057; muut harmittomia / others benign |
| Portit / ports | 12 | ei / no | harmiton / benign |
| Tietueen kenttätaulukko / record field table | 10 | `memcpy`-koko / size | harmiton (käännösaikaiset vakiot); tietueen koko on H-041 |
| Muut / other | ~16 | tietuelinkit / record links | harmiton (tietueita ≤ 9 999) |

**Tulos / result:** yksikään muunnosvaroitus ei sellaisenaan ole muistivian syy;
numerot ja tietuenumerot on rajattu (`MAXKILPNO` 9999). Käyttöpaikkojen jäljitys löysi
kuitenkin neljä uutta vikaa: H-071, H-072, H-073, H-074; kahdessa kavennus on osa
mekanismia (viestin INT16-`os`, UINT16-väliaika). / No conversion warning by itself
causes a memory defect; tracing the uses found H-071–H-074. Noin 300 riviä luokiteltiin
harmittomiksi kuvion perusteella avaamatta. / About 300 lines were classed benign by
pattern without opening them.

## 13. Kattavuuden arviointi / Coverage assessment

**Kääntämättä jääneet 55 tiedostoa** (HEAD): 23 C-11-testitiedostoa, 2 `LibTest.cbproj`-
testiohjelman tiedostoa ja 30 orpoa tiedostoa (`inventaario.md`). Yksikään ei ole
toimitetun ohjelman lähdelistassa eikä `#include`-direktiivin kautta mukana; konsoliohjelmat
linkittyivät tässä analyysissä ja GUI-ohjelmat tallennetussa käännöksessä ilman niitä.
Orpojen joukossa on vanhoja kopioita (`Tp/TpComY32.cpp`, `tputilv2/*-2.cpp`, `*-v.cpp`),
joiden vikoja ei ole tarkoituksenmukaista analysoida. / None is production code.

**Analyysin ulkopuolelle jääneet kokoonpanot / configurations not analysed:**

| Projekti / Project | Kokoonpanot / Configs | Ero analysoituun / Difference |
|---|---|---|
| HkMaali | Debug, DebugV200, Demo | `_DEBUG`, `NREGNLY`, `VALUKU=200` (katettu ReleaseVA200:lla / covered), `DEMO` |
| JukMaali520 | Debug, Debug250 | samat rajat kuin Release / Release150os / same limits |
| JukLuenta520 | Debug, Debug250, Release150os | samat määritykset (T-017) / identical defines |
| HkKisaWin, ViestiWin | Debug, Win64, Win64x | ei koskaan toimitettu / never shipped [päätelty] |
| TpTest, TpCoreHk, TpCoreJuk, LibTest | kaikki / all | testiohjelmia / test programs |
| bcc32c, bcc64 (Clang) | – | tuotantoa ei käännetä niillä / not used for production |

**Kääntäjävaroitusten ulkopuolelle jäävät vikaluokat / defect classes outside compiler
warnings** (havaittu tässä erässä / observed in this batch):

1. Ajonaikaisesti lasketut koot: tietueen koko konfiguraatiosta (H-041, H-042) — ei
   yhtään varoitusta.
2. Ulkoisesta syötteestä tulevat indeksit ja pituudet ilman rajatarkistusta (H-055, H-060,
   H-070, H-071, H-072) — kääntäjä ei tiedä arvon alkuperää.
3. Laskenta- ja täyttökierroksen erot (H-058, H-074) — vaativat kahden silmukan vertailun.
4. Etenemättömät silmukat ja palvelunesto (H-056) — ei muistivirhe kääntäjälle.
5. Samanaikaisuus, lukot ja elinkaari (H-007–H-021, H-040) — erät C-1/C-2, ei varoituksia.
6. Tiedostopolut ja oikeudet (H-071:n `"wb"`-avaus) — ei muistikysymys kääntäjälle.
7. GUI:n muotoilumerkkijonot ja puskurit: bcc32 ei tarkista (T-021).
8. Rakenteiden asettelu kääntäjien välillä (`-a1`, `-K`, `/J`) — erä C-5.

**Riskialueet, joita varoitusinventaario ei kata / risky areas not covered by the
inventory:**

- **Laitejäsentimet** `Tp/TpLaitteet.cpp` (`tulk_*`, `lue_*`): neljä uutta vikaa
  (H-055, H-056, H-070, H-055:n `F`-kenttä) löytyi lukemalla, ei varoituksista.
  Kaikkia jäsentimiä ei ole luettu (mm. `lue_regnly`n yleinen haara `:1611-1640`,
  RTR-, Alge-, Kurvinen-jäsentimet). [tuntematon]
- **Verkkopaketin hyötykuorman käsittelijät** (`tark_*`, `Hk/HkCom32.cpp`,
  `Juk/VCom.cpp`): H-071 ja H-072 löytyivät muunnosten jäljityksestä; kaikkia
  pakettiluokkia ei ole käyty läpi kenttä kentältä.
- **GUI-lomakkeet** (`cbHk/`, `ViestiWin/`, `cbTp/`, 178 käännösyksikköä vain bcc32:lla):
  tuonti (`UnitImport`), MySQL, näyttötaulut (`UnitGAZ`, `UnitTaulu`), COMFILE-katselin
  (H-050).
- **XML-tuonnin "säilytä paikat" -tila** (`cbHk/UnitImport.cpp:99-110`): `addtall`-silmukka
  XML:n avaimeen asti, käytös `maxrec`-rajalla [tuntematon].

## 14. Uudet havainnot vaiheessa 2 / New findings in step 2

| ID | Vakavuus / Severity | Lyhyesti / Summary | Syöte / Input | Ohjelmat / Programs | Todennus / Verification |
|---|---|---|---|---|---|
| H-070 | korkea FEIG-lukijalla / high with a FEIG reader | `lue_FEIG`: laitteen pituus → `memcpy` 15 001 tavun kasalohkoon | laite / device (sarja, TCP-asiakas) | HkMaali, HkKisaWin, ViestiMaali, ViestiWin | riippumaton / independent |
| H-071 | korkea / high | FILESEND: pituus tarkistamatta, verkon valitsema tiedostopolku `"wb"` | **verkko / network** | kaikki 5 / all | riippumaton / independent |
| H-072 | korkea / high | `EXTRA`: HK `eralahto[d2]`, viesti `Sarjat[sarja]` verkon indeksillä | **verkko / network** | kaikki 5 / all | riippumaton / independent |
| H-073 | keskitaso / medium | `kirjemvaloki`: `msg[400]`-pinoylivuoto | verkko (EMITVA), `EMVALOKI` | HkMaali, HkKisaWin, ViestiMaali, ViestiWin | riippumaton / independent |
| H-074 | keskitaso / medium | lähdepiste- ja lähdejonotiedostot: laskenta/täyttö, negatiivinen osuus | paikallinen tiedosto / local file | HK ja viesti / HK and relay | riippumaton, osittain / partly |
| H-075 | keskitaso / medium | viestin konsoli: alustamaton kirjasinosoitin | operaattori / operator | ViestiMaali | riippumaton / independent |
| H-076 | keskitaso / medium | `ossijat`: kasan ylivuoto XML-osuustuloksissa | operaattori / operator | viesti / relay | riippumaton / independent |
| H-077 | matala / low | C-Power-taulun loppumerkki pinopuskurin yli | operaattori / operator | HkKisaWin, ViestiWin | riippumaton, osittain / partly |

Laajennetut / extended: **H-055** (verkkosaavutettavuus, osoitteen kierto, `F`-kenttä),
**H-056** (`lue_Sirit`, `lue_EMITAG`), **H-059** (viestin konsolin `ln[120]`),
**H-069** (`srj` ajanottoruudukossa). Uutta H:ta ei luotu, kun kyse oli rekisteröidyn
vian toisesta ilmenemismuodosta. / No new H for another manifestation of a registered one.

Muita, rekisteriin kirjaamattomia matalia / other low, not registered:
`Juk/VTulostus.cpp:3076` (alustamaton `st` HTML:ään), `:1868` (päättämätön kopio),
`Juk/XMLtulos.cpp:446` (alustamaton `tag_cn`), `Juk/VIv.cpp:1690` (`ESITARK=-1`),
`Hk/HkEmit.cpp:723` (ratavienti, `Point[-1]`), `HkConsole/HkCons2.cpp:2663` ja
`JukConsole/vcons2.cpp:2064` (näyttö), `Tp/TpLaitteet.cpp:257-288` (`ec_strtoaika`
etumerkillinen ylivuoto), `:401-404` (`F`-kentän yliluku), `Juk/VTulostus.cpp:3061`
(`buf[-1]`).

## 15. Muistiturvallisuuden kokonaisarvio / Overall assessment

**Suomeksi.** Koodipohjassa ei ole järjestelmällistä syötteen validointia: laitteiden
sanomat, verkkopakettien hyötykuorma ja asetustiedostot jäsennetään kiinteän kokoisiin
puskureihin ja taulukoihin, ja indeksit ja pituudet otetaan syötteestä sellaisenaan.
Verkkoprotokollassa ei ole todennusta (H-029) eikä avaimellista eheystarkistusta, joten
tarkistamaton hyötykuorman kenttä on **mahdollisesti** lähiverkosta saavutettava.
Saavutettavuus on osoitettu havaintokohtaisella kutsu- ja verkkopolkuanalyysillä vain
alla olevan taulukon havainnoille; muiden pakettikenttien saavutettavuutta ei ole
analysoitu, eikä sitä voi päätellä yleisestä protokollan ominaisuudesta. Vakavimmat viat
ovat verkosta tai laitteelta tulevat kirjoitukset (H-071, H-072, H-055, H-070, H-032),
joista H-071 sallii myös tiedostojen korvaamisen ilman muistivirhettä. Pinon
ylivuodot syntyvät rajaamattomasta `swprintf`ista (`_CRT_NON_CONFORMING_SWPRINTFS`) ja
kiinteistä rivipuskureista (H-059, H-073). Tietueen ja taulukoiden koot riippuvat
käännösvalitsimista ja ajonaikaisista asetuksista (H-041, H-042), mitä kääntäjä ei
näe. Kääntäjävaroitus osui suoraan vikakohtaan 16:ssa C-4:n 23 uudesta havainnosta; 7
(H-055, H-060, H-070–H-074) löytyi lukemalla tai jäljittämällä varoitusten ympäristöä. Mikään C-4:n uusista havainnoista ei ole ajossa
vahvistettu; ainoa ajossa vahvistettu muistivika on edelleen H-041.

**In English.** There is no systematic input validation: device messages, network
payloads and configuration files are parsed into fixed buffers and arrays, with indices
and lengths taken from the input. The protocol has no authentication (H-029) and no keyed
integrity check, so an unchecked payload field is **potentially** reachable from the LAN.
Reachability has been shown by per-finding call and network path analysis only for the
findings in the table below; other packet fields have not been analysed and their
reachability cannot be inferred from the protocol property alone. The most severe
defects are network- or device-originated writes (H-071, H-072, H-055, H-070, H-032);
H-071 also allows overwriting files. Stack overflows come from unbounded
`swprintf` and fixed line buffers. Record and array sizes depend on build switches and
run-time settings that the compiler cannot see. Warnings found about half of this
batch's defects directly; the rest were found by reading around them. None of the new
C-4 findings is confirmed by running; H-041 remains the only memory defect confirmed by
running.

### Priorisointi / Prioritisation (vakavuus × saavutettavuus / severity × reachability)

| Prioriteetti / Priority | Havainnot / Findings | Peruste / Rationale |
|---|---|---|
| **P1: verkosta, ilman erityisasetuksia / network, default settings** | H-071, H-072, H-032, H-061 (sarja/TCP) | Yksi väärennetty paketti; ei todennusta. / One forged packet. |
| **P2: verkosta tai laitteelta tietyillä asetuksilla / network or device with a setting** | H-055 (GUI `TCPS`, HTTP, lukija), H-070 (FEIG), H-056 (ARES, Sirit, emiTag: palvelunesto), H-060 (ampumapaikka, myös verkon sanomatyyppi 6), H-073 (`EMVALOKI`) | Tavallinen laitekäyttö tai lähiverkko. / Normal device use or LAN. |
| **P3: toimitetussa käännöksessä tavallisella datalla / shipped build, ordinary data** | H-041 (ajossa vahvistettu / confirmed by running), H-042, H-001, H-059, H-076, H-062 | Ei hyökkääjää; riittää iso kilpailu tai tietty tulostevalinta. / No attacker needed. |
| **P4: paikallinen tiedosto tai operaattori / local file or operator** | H-002, H-057, H-058, H-074, H-065, H-063, H-064, H-075, H-052, H-049 | Vaatii virheellisen tai muokatun tiedoston tai tietyn valinnan. / Needs a bad file or choice. |
| **P5: matala / low** | H-066, H-067, H-068, H-069, H-077, H-003 | Väärä arvo tai näyttö, pieni yliluku. / Wrong value or display. |

### Havaintokohtainen saavutettavuus / Per-finding reachability

| ID | Syöte ja polku / Input and path | Portti tai laite / Gate | Analyysi / Analysis |
|---|---|---|---|
| H-071 | UDP, sarja, binääri-TCP → `addseur` → pakettiluokan käsittely (`Hk/HkCom32.cpp:939-940`, `Juk/VCom.cpp:968`) → `tark_tiedosto` (`Tp/TpComY.cpp:3004-3037`) | ainoa ehto: vastaanotto käytössä (`vastcom`), yhteys auki; ei asetuslippua / only receiving enabled | C-4 vaihe 2, riippumaton tarkastus F1 |
| H-072 | sama verkkopolku → `EXTRA` (`Hk/HkCom32.cpp:640-645`, `Juk/VCom.cpp:1617-1618`) | `comopen`; ei asetuslippua / no setting | F2, F3 |
| H-032 | UDP-vastaanotto (`Tp/TpComY.cpp:1001`) | UDP-yhteys / UDP link | C-2, C-11 testikehys / harness |
| H-061 | sarja ja binääri-TCP, pala-ajoin saapuva paketti (`Tp/TpComY.cpp:805-818`) | sarja- tai TCP-yhteys / link | D1 (ajonaikainen arvo [tuntematon] / runtime value unknown) |
| H-055 | emiTag-lukija (sarja/USB), GUI:n `TCPS`-palvelin, HTTP-pollaus (`Hk/HkIV.cpp:3540`) | emiTag-lukija, `TCPS` tai `ETGPRS`/`ETSQL` / reader, TCPS or polling configured | luku 10, E2 |
| H-070 | FEIG-lukija (sarja tai lähtevä TCP) → `lue_FEIG` | FEIG-avainsana / keyword | E1 |
| H-060 | Kurvinen-laite tai verkon sanomatyyppi 6 (`Hk/HkCom32.cpp:647`) | `sak_laji` 2 (oletus / default) | vaihe 1, C2 |
| H-073 | vastaanotettu EMITVA-paketti → `kirjemvaloki` | `EMVALOKI` | F4 |
| H-056 | ARES/Sirit/emiTag-laite → lukijasäie | laite konfiguroitu / device configured | luku 10, E3 |

### Kiireellisesti korjattavat / Urgent fixes

**H-071, H-072, H-055 ja H-070** merkitään kiireellisesti korjattaviksi: kolme ensimmäistä
on osoitettu saavutettaviksi verkosta (H-055 tietyillä asetuksilla), H-070 laitteelta, ja
kaikki antavat kirjoituksen syötteen valitsemaan paikkaan. Korjaukset ovat
rajatarkistuksia ja polun rajaus; ne tehdään omissa haaroissaan erillisellä päätöksellä.
Muiden havaintojen prioriteetit P1–P5 säilyvät ennallaan (P1: H-032, H-061; P2: H-056,
H-060, H-073).

**H-071, H-072, H-055 and H-070** are marked as urgent fixes. Other priorities P1–P5 are
unchanged (P1: H-032, H-061; P2: H-056, H-060, H-073).

### Väliaikaiset suojaustoimenpiteet (suositus) / Interim mitigations (recommendation)

Suosituksia ylläpitäjille ja käyttäjille ennen korjauksia. Tämä auditointi ei muuttanut
verkko-, palomuuri- eikä ohjelma-asetuksia. / Recommendations only; this audit changed no
network, firewall or program settings.

1. **Erillinen ajanottoverkko** (H-071, H-072, H-032, H-061): työasemat omaan suljettuun
   verkkoonsa tai VLANiin; ei yleisö-, vieras- tai julkista Wi-Fiä. / Dedicated closed
   timing network.
2. **Palomuurin sallittujen lista** tiedonsiirtoporttien UDP/TCP-liikenteelle vain tunnettujen
   työasemien osoitteista (Windowsin palomuurin saapuvan liikenteen säännöt). / Firewall
   allow-list of known peers.
3. **Vähimmät oikeudet ja oma työhakemisto** (H-071): ohjelma tavallisena käyttäjänä
   kilpailukohtaisessa hakemistossa, ei järjestelmänvalvojana; varmuuskopiot
   kilpailutiedostoista erilliseen paikkaan. / Least privilege, dedicated folder, backups.
4. **emiTag** (H-055): GUI:n `TCPS`-tilaa ei käytetä jaetussa verkossa tai se rajataan
   palomuurilla lukijan osoitteeseen; HTTP-pollaus (`ETGPRS`/`ETSQL`) vain luotetun
   yhteyden tai VPN:n yli. / Avoid TCPS on shared networks; polling over trusted links.
5. **Laitelukijat** (H-056, H-070): luotettavat kaapelit ja oikeat sarjaportin asetukset;
   seurataan, että ajanottosyöte etenee, ja käynnistetään ohjelma uudelleen, jos lukija
   pysähtyy; FEIG-, ARES- ja Sirit-lukijat vain suoraan kytkettyinä. / Reliable cabling;
   watch input; restart procedure.
6. **`EMVALOKI` pois päältä** (H-073), ellei sitä tarvita. / Keep `EMVALOKI` off.
7. **Asetus- ja tuontitiedostot** (H-057, H-058, H-065, H-074): käytetään ohjelman itse
   tuottamia tai tarkistettuja tiedostoja; ei käsin muokattuja tiedostoja tuotannossa ilman
   koeajoa. / Use program-generated or checked files.

### Korjattavat ja myöhemmin todennettavat / To fix vs to verify later

- **Korjattavissa nykyisellä näytöllä** (mekanismi vahvistettu koodista ja
  riippumattomasti, korjaus on rajatarkistus tai alustus; ei vaadi ajoa ennen korjausta) /
  **fixable on current evidence**: H-071, H-072, H-055, H-070, H-056, H-060, H-073,
  H-059, H-074, H-058, H-057, H-076, H-075, H-062, H-063, H-064, H-065, H-066–H-069, H-077.
- **Todennettava ennen korjausta tai vaikutuksen arvioimiseksi** / **verify first**:
  H-061 (`pkglen`-arvo: disassembly, Q43), H-055:n hyödynnettävyys (kehysasettelu, ASLR; ei
  vaikuta korjaukseen), H-056:n kehysoletus (Q44), H-076:n valikkoyhdistelmä konsolissa
  (Q52), `Hk/HkXmlTulokset.cpp:1211`, `Juk/VIv.cpp:2226`.
- **Ehdotettu myöhempi ajonaikainen todennus** (erillisellä luvalla, ei tässä erässä) /
  **proposed later runtime checks**: H-071 ja H-072 yhdellä paketilla eristetyssä
  HkMaali-instanssissa; H-055 ja H-070 TpCore-kehyksen olemassa olevalla syötepolulla.

## 16. Avoimet riskit / Open risks

- Laitejäsentimiä ja pakettiluokkia ei ole luettu kokonaan (luku 13); samaa kuviota on
  todennäköisesti lisää. [päätelty]
- GUI-lomakkeiden muistiturvallisuus on analysoitu vain bcc32:n heikoilla varoituksilla.
- Kaikki C-4:n havainnot ovat staattisia; vaikutukset (kaatuminen, korruptio,
  hyödynnettävyys) ovat päätelmiä.
- Noin 6 500 varoitusta (kokonaislukumuunnokset riskitiedostojen ulkopuolella,
  käyttämättömät, varjostus, pakkaus) jäi luokittelematta.
- H-071 ja H-072 tekevät lähiverkosta luotetun alueen; tapahtumaverkkoa, jossa on
  muita laitteita tai yleisö, ei voi pitää turvallisena ennen korjauksia.

## 17. C-4:n loppuyhteenveto / Final summary of C-4

| Mittari / Metric | Arvo / Value |
|---|---|
| Kääntäjät / compilers | MSVC 19.52 (v145), bcc32 7.80 |
| Kokoonpanot / configurations | 7 MSVC (5 projektia / projects), 4 bcc32 |
| Varoitusrivejä / warning rows | 11 027 |
| Yksilöllisiä / unique | 7 036 (MSVC 1 605, bcc32 5 431) |
| Luokiteltu riveittäin / triaged per row | 275 + 33 uudelleen / re-examined |
| Muunnosvaroituksia ryhmittäin / conversion warnings by group | 522 |
| Uusia H-havaintoja / new H findings | 23 (H-055–H-077): korkea / high 5, keskitaso / medium 13, matala / low 5 |
| Uusia T-havaintoja / new T findings | 2 (T-020, T-021) |
| Laajennettuja / extended | H-055, H-056, H-059, H-069 |
| Riippumattomia tarkastuksia / independent reviews | 5 |
| Ajossa vahvistettu / confirmed by running | 0 uutta / new |

Korkeat / high: H-055, H-056 (laitekäytössä / with the device), H-070, H-071, H-072;
keskitaso / medium: H-057–H-065, H-073–H-076; matala / low: H-066–H-069, H-077.

**C-4 on valmis ja suljettu (käyttäjän hyväksyntä 2026-11-25).** Seuraava työvaihe
alkuperäisen suunnitelman mukaan on erä C-5 (kääntäjäkohtainen käytös); se aloitetaan vain
erillisellä luvalla. / **C-4 is complete and closed (approved 2026-11-25).** Next per the
original plan: batch C-5 (compiler-specific behaviour), on separate approval.

## Avoimet kysymykset / Open questions

| # | Kysymys / Question | Ratkaisu / How to settle |
|---|---|---|
| Q43 | Säilyykö `luepaketti`n `pkglen` kutsujen välillä toimitetuissa käännöksissä? (H-061) / Does `pkglen` survive between calls? | disassembly |
| Q44 | Voiko ARES-kehyksessä olla 0x00-tavuja; kuinka suureksi sarjajono kasvaa? (H-056) | protokollan kuvaus tai kaappaus / protocol or capture |
| Q45 | Miten bcc32:n `atoi` käyttäytyy ylivuodossa? (H-055 GUI) | RTL-lähde / RTL source |
| Q46 | Mikä globaali seuraa `mrivi`ä kussakin exessä? (H-067) | linkkerin kartta / linker map |
| Q47 | Käytetäänkö kentällä käsin muokattuja `RADAT.LST`-tiedostoja tai yli 50 rastin ratoja? (H-058) | ylläpitäjät / maintainers |
| Q48 | Miten GUI-tiedostojen muotoilumerkkijonot tarkistetaan, kun bcc32 ei sitä tee? (T-021) | esim. MSVC-syntaksitarkistus GUI-ytimelle / e.g. MSVC syntax-only pass |
| Q49 | Onko bcc32-GUI-ohjelmissa ASLR ja jokin pinosuoja? (H-055) / ASLR and stack protection in bcc32 GUI builds? | linkkerin asetukset, PE-otsake / linker settings, PE header |
| Q50 | Voiko konsolin asetusavainsana asettaa emiTag-lukijan TCP-palvelimeksi? (H-055) / Can a console keyword make the emiTag reader a TCP server? | `Hk/HkInit.cpp`, `Juk/VInit.cpp` |
| Q51 | Tukeeko viesti FEIG-lukijaa TCP:n yli? (H-070) / FEIG over TCP in relay? | `Juk/VInit.cpp` |
| Q52 | Voiko konsolin tai AutoFilen valikoista valita XML + `H` + kaikki osuudet? (H-076) / menu reachability | valikot / menus |
| Q53 | Voiko `EMVALOKI`n asettaa GUI:sta? (H-073) / `EMVALOKI` from the GUI? | `cbHk/`, `ViestiWin/` |
| Q54 | Mikä rajaa `len`in `lahriviCPower`issa? (H-077) / bound of `len` | `ViestiWin/UnitGAZ.cpp:918`, `cbHk/UnitTaulu.cpp:858` |
| Q55 | XML-tuonnin "säilytä paikat": `addtall`-silmukka `maxrec`-rajalla / keep-positions loop at `maxrec` | `cbHk/UnitImport.cpp:99-110` |
| Q56 | Voiko emiTag-kortin sisältö vaikuttaa `P`-indeksiin vai tuottaako sen vain lukijan laiteohjelmisto? (H-055) / can card contents influence the `P` index? | valmistajan protokolla / vendor protocol |
