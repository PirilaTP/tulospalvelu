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

## Avoimet kysymykset / Open questions

| # | Kysymys / Question | Ratkaisu / How to settle |
|---|---|---|
| Q43 | Säilyykö `luepaketti`n `pkglen` kutsujen välillä toimitetuissa käännöksissä? (H-061) / Does `pkglen` survive between calls? | disassembly |
| Q44 | Voiko ARES-kehyksessä olla 0x00-tavuja; kuinka suureksi sarjajono kasvaa? (H-056) | protokollan kuvaus tai kaappaus / protocol or capture |
| Q45 | Miten bcc32:n `atoi` käyttäytyy ylivuodossa? (H-055 GUI) | RTL-lähde / RTL source |
| Q46 | Mikä globaali seuraa `mrivi`ä kussakin exessä? (H-067) | linkkerin kartta / linker map |
| Q47 | Käytetäänkö kentällä käsin muokattuja `RADAT.LST`-tiedostoja tai yli 50 rastin ratoja? (H-058) | ylläpitäjät / maintainers |
| Q48 | Miten GUI-tiedostojen muotoilumerkkijonot tarkistetaan, kun bcc32 ei sitä tee? (T-021) | esim. MSVC-syntaksitarkistus GUI-ytimelle / e.g. MSVC syntax-only pass |
