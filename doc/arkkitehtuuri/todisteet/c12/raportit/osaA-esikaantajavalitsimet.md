# C-12 osa A / part A: preprocessor switch classification

Agent `build-dependency-analyst` (Sonnet), 2026-11-26. Report saved by the main session from the
agent's hand-back. Claims, evidence and labels are unchanged; wording and layout are condensed.
Static reading only. Paths are relative to `TPsource/V52/`; `csv` = `doc/arkkitehtuuri/todisteet/c12/osaA0/`.
Labels N-1..N-14 are provisional. They are not register IDs and are a different series from the part C and part B labels.

Markers: [verified] = code or project file read; [inferred] = deduced from names or context;
"name-based guess" = the code alone does not tell. Values are given in the order
HkMaali, ViestiMaali, ViestiLuenta, HkKisaWin, ViestiWin; `-` = undefined, `1` = defined with no value,
`?` = unknown to the evaluator. "dead N" = sum of `body_lines` of dead and unreachable sites.
For `#ifndef X` / `!defined(X)` with X never defined, the branch is always taken.

## 1. Macro table (all macros except include-guard and compiler-platform)

### 1a. Variants

|macro|class|meaning|evidence|values|status|notes|
|---|---|---|---|---|---|---|
|_CONSOLE|variant (csv: include-guard, wrong)|console TUI vs VCL GUI build of shared source [verified]|`include/tputil.h:474`; 252 sites in 59 files, 226 varies|1,1,1,-,-|3 console products; tputilv2 also defines it, together with CONSOLELIB|HK: 124 sites / 3494 lines; relay: 130 / 4942 differ console vs GUI|
|_BORLAND_|variant|C++Builder project switch: `vcl.h`/`WinHk.h` includes and API names. The only behaviour block is the KUNTOSUUNN badge swap (H-081)|`Hk/HkAjat.cpp:17`; `Hk/HkCom32.cpp:34`; `Hk/HkIV.cpp:1663`; `Tp/TpApu.cpp:17,208,218`; `dbboxm/dbbox1.cpp:24`|-,-,-,1,-; Tputil-XE=1, DBboxm-XE=1|ViestiWin does **not** define it, although its libraries do|matters for `Tp/TpApu.cpp`, which is compiled into both GUIs|
|LUENTA|variant|card-reader-only relay program; removes timing and result code|`Juk/VDef.h:49,875`; `Juk/VDeclare.h:740`; `JukConsole/VConTul.cpp:379`; 83 sites|-,-,1,-,-|Luenta only|4144 lines differ from Maali|
|MAALI|variant (csv: feature)|finish/timing mode; forced on in all products; the `EIMAALI` and `!MAALI` opt-outs are never used|`Hk/HkDef.h:112`; `Juk/VDef.h:75`; 60 sites|1,1,1,1,1|always on; the `!MAALI` branches are dead| |
|MAKI|variant (dead)|ski-jumping or hill variant (name-based guess); forced off by `#undef`|`Hk/HkDef.h:33-35`; `Hk/HkInitVanha.cpp:55`; no `#define MAKI` anywhere [verified]|?,-,-,?,-; real: undefined|DEAD; the 68 "unknown" sites come only from the conditional `#undef`; 1232 dead lines|largest blocks: `Hk/HkTulostus.cpp:2215` (310), `Hk/HkXmlTulokset.cpp:562` (216), `Hk/HkTulostus.cpp:8098` (95)|
|HIIHTO|variant (dead)|retired skiing variant; the header turns any external `HIIHTO` into 1, but nothing defines it|`Hk/HkDef.h:118-120`; `Hk/HkTV.cpp:130`; `HkConsole/HkCons2.cpp:69`; `Juk/VInit.cpp:756` (a comment says "HIIHTO removed")|?,-,-,?,-; real: undefined|DEAD|`HIIHTO2`: 1 unreachable site|
|HIIHTOLAHTO|feature (forced on)|ski start-mode code|`Hk/HkDef.h:122`|1,-,-,1,-|always on in HK| |

### 1b. Features

|macro|class|meaning|evidence|values|status|notes|
|---|---|---|---|---|---|---|
|EMIT|feature|emit card reading and handling (about 70 sites)|`Hk/HkDef.h:112`|1,1,1,1,1|always on| |
|EMITLEIMAT|feature, derived|emit split ("leimat") handling, from `(MAALI or AUTOVA) and EMIT`, unless `EILEIMAT`|`Hk/HkDef.h:112`; `Juk/VDef.h:75`; `JukConsole/lasktxt.cpp:18`|1,1,1,1,1|always on|three derivation sites; the `EILEIMAT` opt-out is never defined|
|EMITFILE|feature|test-mode emit input from `EMIT_IN.DAT` (port > 900), `file2emit`|`Juk/VIv.cpp:810-892`; `Hk/HkIV.cpp:877,3081`; `Tp/TpLaitteet.cpp:1911`|1,1,-,1,-|HkMaali, ViestiMaali, HkKisaWin|`#undef` under LUENTA at `Juk/VIv.cpp:47` and `Hk/HkIV.cpp:48`; the HK one is dead|
|EMITHTTP|feature|WinHTTP for emiTag (`riippuvuudet.md` section 10)|`Juk/VGlobals.cpp:502`; `Juk/VInit.cpp:1852`|1,1,-,1,1|not in Luenta (223 lines)| |
|EMITAG|project define, never tested|"EMITAG" is a reader-type keyword (`Tp/TpLaitteet.cpp:64,90`)|`Hk/HkInit.cpp:2334`; `Juk/VInit.cpp:1826`|-,1,-,-,1|no effect [verified]|`riippuvuudet.md:286`|
|LAJUNEN|feature|Sirit/Zebra UHF RFID readers; the name is probably a customer (name-based guess)|`include/SiritReader.h:10,16`; `Tp/TpLaitteet.cpp:104,669-673,738,1535`; `HkConsole/HkCons0.cpp:904`; `Tp/ZebraReader.cpp:38`; 36 sites|1,1,-,1,1|not in Luenta (1666 lines)|`LAJUNENx` and `LAJUNENxx` (23 lines) are disabled variants|
|TCPSIIRTO|feature|TCP file send; forces `AUTOFILE` in HK|`Hk/HkDef.h:54`; `com/WinUDP.cpp:21-23` TU-local; 32 sites|1,1,1,1,1|always on|`TCPSIIRTOxx` and `TCPSIIRTO_2` are disabled copies|
|TCPLUKIJA|feature|TCP-fed card reader channel|`Hk/HkInit.cpp:2143,2297`; `Juk/VInit.cpp:1778`; `Tp/TpComY.cpp:2078`|1,1,1,1,1|always on| |
|AUTOFILE|feature, derived in HK|automatic result-file output|`Hk/HkDef.h:54`; `Juk/VTulostus.cpp:7030` (305 lines); `Hk/HkTulostus.cpp:6978` (372)|1,1,1,1,1|always on|the relay projects define it explicitly|
|KONETUNN|feature|neighbour machine name in the connection status instead of "On"|`Tp/TpComY.cpp:1742,1754,1766,1818`|1,1,1,1,1|always on (6 lines)| |
|SERVER|feature (console UI)|`lahetanumerotTCP`, `yhteysasetukset` menu, `VARASERVER` key|`Hk/HkCom32.cpp:1191`; `Juk/VCom.cpp:1367`; `Tp/TpComY.cpp:2429`; `Hk/HkInit.cpp:1192`|1,1,1,-,-|console only|549 (HK) / 571 (relay) lines; the GUI has its own forms [verified]|
|DBGFILE|debug-diagnostic|raw reader bytes to per-port files, runtime key `DBGTOFILE`; 37 sites|`Tp/TpLaitteet.cpp:1624`; `Hk/HkInit.cpp:1404`; `Hk/HkDeclare.h:422`|-,1,1,1,-; HkMaali Debug: 1|missing from HkMaali Release and ViestiWin|N-7|
|PROFILOI|debug-diagnostic|comm profiling `put_profile`, `profarr` (10000), config `PROFILOI[=2]`|`Tp/TpComY.cpp:185,886-969`; `Juk/VInit.cpp:1144`|-,1,1,-,1|relay only (only the relay parser reads the key)|28 sites in `TpComY32.cpp`, which is in no product|
|GAZ|feature|console HK serial scoreboard: `gaz_com`, `gaztap`, `opengaz`, keys `GAZ=`, `GAZVAIHE=`, `GAZRIVI`; the relay has no `#if` consumer|`Hk/HkIV.cpp:3102` (268 lines); `Hk/HkInit.cpp:109,1376`; `Hk/HkDat.cpp:1191`; GUI: `cbHk/UnitTaulu.cpp:213-226` (unconditional)|1,1,-,-,1|HK console only|`GAZ2` and `GAZTEST` are disabled (`ViestiWin/UnitGAZ.cpp`)|
|ALGETAULU|feature|relay ALGE scoreboard, `TAULU_COM=`, `paiv_taulu`, `JukConsole/lasknum5.cpp` (286 lines)|`Juk/VInit.cpp:557,1008`; `Juk/VIv.cpp:88`; `JukConsole/lasknum5.cpp:17`|-,1,-,-,1|Maali and Win|legacy `TAULU` (63 dead lines) is paired with it|
|ESITARK|feature|emit pre-check screen at the finish|`Juk/VEmit.cpp:3702` (37 lines), `4572`; `Juk/VInit.cpp:2111`; `Hk/HkEmit.cpp:3969`|-,1,-,-,-|ViestiMaali only|82 lines differ from ViestiWin|
|TESTIAJAT|feature|Ctrl-T `lahetatestiajat` sends test times|`JukConsole/vcons2.cpp:2299` (45 lines), `2744`|-,1,-,-,-|ViestiMaali only| |
|ENNAKOI|feature|anticipated bib entry, up to 5 (`jono[]`, `maxennjono=5`), highlighted on the time screen; key `ENNAKOI`|`HkConsole/HkCons2.cpp:1519,1594,2121-2154`; `JukConsole/vcons2.cpp:2402-2984`; `Hk/HkInit.cpp:1541`|1,1,-,-,-|console only|GUI twin `cbHk/UnitAjanotto.cpp:163,1436`|
|AIKAVERT|feature|console HK time-comparison screen (`vertajat`, `VERTRAJAT=`, Alt-S/Alt-V)|`HkConsole/HkCons2.cpp:2581` (536 lines); `Hk/HkInit.cpp:1421`; `Hk/HkAjat.cpp:311`|1,-,-,-,-|HkMaali only|GUI twin `cbHk/VertAikaUnit.cpp`|
|VALUKIJA|feature|split-time reader: `laheta_ec` forwards an emit read to peers|`Hk/HkCom32.cpp:497`; `Juk/VCom.cpp:1505`; `Juk/VDef.h:38`|1,1,-,1,1|not in Luenta|implies AUTOVA in the relay|
|AUTOVA|feature, derived in the relay|automatic split-time processing (`start_autova`, `varasti`, `t0_autova`)|`Hk/HkIV.cpp:3373` (124 lines); `Juk/VIv.cpp:2251` (116)|1,1,-,1,1|not in Luenta| |
|SAKKOCOM|feature (forced on)|penalty serial channel, `SAKKO_COM`/`SAKKO_YHT`|`Hk/HkInit.cpp:2130-2182`; `Hk/HkDef.h:124-126`|1,-,-,1,-|always on in HK; the HkMaali project define is redundant| |
|SEURAVAL|feature|club selection list `sraLuett`; `lah_seura` sends club info|`HkConsole/HkKilp.cpp:61` (262 lines); `Hk/HkCom32.cpp:460,927`|1,-,-,1,-|HK only|not the same as SEURATUL (N-1)|
|JOUKKUETUL|feature|team results `joukkuetul()` (`JKOKO 300`), via `rajaus=='J'` in GUI print|`Hk/HkTulostus.cpp:7601` (309 lines), `8749`; `Juk/VTulostus.cpp:7672` (482, dead)|-,-,-,1,-|HkKisaWin only|the relay copy never compiles (N-12)|
|JOUKKUEPISTEET|feature (forced on)|team points `Hk/HkTulostus.cpp:7366-7573`|`Hk/HkDef.h:41-43`; `HkConsole/HkCons3.cpp:1483,2008-2064`|1,-,-,1,-|always on in HK; the `!defined` branches are dead (258 lines)| |
|MERI5|feature (forced on)|three-stage points rule `meri3pisteet`: 1500 - (500*t + best/2)/best|`Hk/HkTulostus.cpp:2528-2575` (386 lines); `Hk/HkDef.h:37-39`|1,-,-,1,-|always on in HK|name origin unknown|
|MILLISEKAJAT|define-only|no `#if` tests it|`Hk/HkDef.h:26`|project: HkKisaWin|no effect [verified]| |
|KSEK|other|time unit `(10L*CSEK)`; the test `KSEK==10` is dead|`Tp/TpDef.h:81`; `Tp/TpApu.cpp:440`|1,1,1,1,1|constant| |
|NOMOUSE|feature (forced on)|disables mouse code in HK|`Hk/HkDef.h:79`; `Hk/HkInit.cpp:95`|1,-,-,1,-|always on (HK)| |
|NOCOM_AIKA|feature, derived|disables time-stamp comms in Luenta: `LUENTA && !NOCOM_AIMA`|`Juk/VDef.h:49-51`; `Tp/TpLaitteet.cpp:2402` (34 lines); `Juk/VInit.cpp:1472`|-,-,1,-,-|Luenta only|`NOCOM_AIMA` typo (N-3)|
|DEMO|feature (non-shipped configurations)|demo build: caption, message, `maxrec=30` (HK) or 20 (relay)|`Hk/HkInit.cpp:283`; `Hk/HkIx.cpp:1581-1582`; `Juk/VIx.cpp:1077-1078`; `cbHk/WinHk.cpp:190`; `ViestiWin/UnitMain.cpp:169`|- in shipped; Demo configurations of HkMaali, HkKisaWin and ViestiWin|dead in the shipped builds|6 sites|
|BIT32|compiler-platform (csv: feature)|legacy 16/32-bit; the 16-bit branch is dead|`JukConsole/laskemtn.cpp:29,245`|1,1,1,1,1|always on| |
|CONSOLELIB|variant|tputilv2 console marker; `tputil.h:474` skips GUI declarations|`include/tputil.h:474`|tputilv2 only| | |
|LANG, SV|other|`SU=0, EN=1, SV=2, LANG SU`; `#if LANG==SV` is dead|`Tp/TpDef.h:66-70`; 8 dead sites|constants|dead|Swedish texts use `LANGSV_K`|

### 1c. Size and default constants

|macro|meaning|evidence|values|notes|
|---|---|---|---|---|
|HAJONTA|length of the course-variation code field (`rata[HAJONTA+1]`)|`Hk/HkDef.h:172,460`; `Juk/VDef.h:193,530,668,843`|11,8,8,11,8|equal within a domain; `HAJONTA0`=6 in `ViestiWin/ConvDat.cpp:39`|
|LJK, LPSARJA, LSEURA, LSARJA, LARVO, LYHD, LLYH, LSAKKO, LSAKOT, LWRKOODI, MAXYHD, MAXNSEURA, OSNIMIL, MAXMAALI, MAXNOSUUS|string and array sizes (HK/relay): LJK 15/30, LPSARJA 31/40, LSEURA 31/50, LSARJA 10/10, LARVO 15/15, MAXYHD 40/10, MAXNSEURA 600/2000. HK only: LYHD 15, LLYH 15, LSAKKO 6, LWRKOODI 9. Relay only: LSAKOT 5, OSNIMIL 40, MAXMAALI 3, MAXNOSUUS 4|`Hk/HkDef.h:168-212`; `Juk/VDef.h:20,122,125,161,184-203`|equal within a domain, console = GUI|`Juk/VDef.h:125` MAXNSEURA is unguarded (N-13)|
|LENIMI, LSNIMI|HK 24; ViestiWin `(OSNIMIL-5)`=35 from `cbTp/UnitFIS.h:32-33` when MAXOSUUSLUKU is defined|`Hk/HkDef.h:204,208`|24,-,-,24,35 (csv shows 1 for ViestiWin: evaluator artifact)| |
|MAXSARJALUKU|class array size|`Hk/HkDef.h:64`; `Juk/VDef.h:107-117` (HALIKKO 1, MKV 3, otherwise 200)|200,250,250,200,250; 25 in ViestiMaali Debug250/Release150os and ViestiWin Debug/Debug150os/Release150os|N-5|
|MAXOSUUSLUKU|max legs; array sizes `Juk/VDef.h:383-392`; no default|`Juk/VDef.h:127`; 93 sites|-,30,30,-,30; 250 in the same non-shipped configurations; Luenta 30 everywhere|N-5|
|VAIKALUKU|relay split points per leg, default 12|`Juk/VDef.h:33`|-,12,12,-,12; 8 in the same non-shipped configurations|`VAIKALUKU0=12` in `ViestiWin/ConvDat.cpp:29`|
|VALUKU|HK split points per course, default 250|`Hk/HkDef.h:57-58,98`; `cbHk/TulosUnit.h:35-37`|250,-,-,250,-; 200 in HkMaali DebugV200/ReleaseVA200 and HkKisaWin 200Debug/200Release|`tiedostomuodot.md:266-269`: `combufrec` is 2196 vs 1796 bytes, so the 200 builds are wire- and file-incompatible|
|N_PV|max days, default 5|`Hk/HkDef.h:45-46`|5,-,-,5,-; HkKisaWin 200Release=2| |
|NREGNLY|max reader channels; header defaults 32 (HK and relay); `HkConsole/valinnat.h:25-27` (4 or 0) is shadowed|`Hk/HkDef.h:86`; `Juk/VDef.h:54-55`|32,8,8,32,32; HkMaali DebugV200/Demo/ReleaseVA200 = 4|T-024|
|MAX_LAHPORTTI|max send ports; header default HK 64, relay 32; every project defines 64|`Hk/HkDef.h:82`; `Juk/VDef.h:43`|64 everywhere|the relay header default is unreachable|
|NCOMEXTRA, MAXTUNNUS, MAXCOMDATA, IN_Q_EMPTY|default constants 4, 899, 250, 10|`Hk/HkDef.h:90`; `Juk/VDef.h:58`; `Hk/HkEmit.cpp:37`; `Tp/TpComY.cpp:108`|equal|MAXTUNNUS has five definitions (`cbTp/UnitPiikit.h:35`, `UnitRadat.h:40`, three guarded); the csv `-` for console = TU-local default not counted|
|DEBUG1, DEBUG2|com debug flags 1/0, TU-local in 4 comm TUs; `#if !DEBUG1` is dead|`Hk/HkCom32.cpp:41,44`; `Juk/VCom.cpp:55,58`; `Tp/TpComY.cpp:47,50`| | |
|max, min|function-like fallbacks|`include/butil.h:70,76`; `Juk/VDef.h:271-275`| | |

### 1d. Never-defined groups (meanings mostly [inferred])

|group|meaning|evidence|dead lines|
|---|---|---|---|
|AMPUSUUNN|shooting orienteering (map reading, shooting and orienteering); removes parts of normal HK/relay. `AMPU` itself (`Juk/VDef.h:53`; ViestiWin project) is never tested [verified]|`HkConsole/HkKilp.cpp:337`; `Juk/VTulostus.cpp:8157` (228); `Juk/VEmit.cpp:392`|967 dead, 676 always; 38 sites|
|EI_OLE|"does not exist": code commented out with `#ifdef`|`Hk/HkEmit.cpp:3575`; `Juk/VTulostus.cpp`|1774, the largest dead block; 28 sites|
|SEURATUL (tu-local), ORAVA|console club result screens; ORAVA is a variant inside them|`Hk/HkTulostus.cpp:35`; `HkConsole/HkCons3.cpp:40-42,600,1539,2642-2924`|387 (N-1)|
|SIIMPORT, SPORTIDENT, SWIM2000, FIS*, XMLFIS, IOF, WRKOODI, MCPIST, MATRTABS, MPA, TUTKA, EBA, TUNTURI*, UINTI*, JAAMA, MAASTO|retired or customer features: SI gate import (`Hk/HkIV.cpp:1927`), SI card reading (`Tp/TpLaitteet.cpp:1624`), FIS World Cup points (`Hk/HkTulostus.cpp:2356`), MCPIST points (`:148`), `eba_ajat` (`Hk/HkEmit.cpp:1732`)| |SIIMPORT 307, SPORTIDENT 243, IOF 250, SWIM2000 148, FIS* ~60, XMLFIS 12, WRKOODI 66, MCPIST 13, MATRTABS 22, TUTKA 43, EBA 71, UINTIxx 39|
|TAULU, TAULU_2, TV1, TXTTV, COMETNO, GAZ2, GAZTEST, TALLEMIT, TARK_LEIMAT, PING, PRACK, VERTKELLO, KUUL*, SAKOT, AJANOTTO, JOUKKUEEMIT, JUKOLAVANHA, HALIKKO, MKV*, SQLDATA, SKMKV, KEHITYSLISTA, ILMOITT, ECROSS, EI_LIENE, SHOWDEMO, TEST, TESTBDG, DBG*, MAKIx*, HIIHTO2, LVAYLA*, LANGSV_K|old boards (TAULU_2 209, TV1 49, TXTTV 35), debug traces (DBG* ~130), customer variants (HALIKKO 153; it also selects `MAXSARJALUKU 1`, `Juk/VDef.h:107`), Swedish UI (LANGSV_K: 60 sites, 24 dead, 15 unreachable, 109 always), `KEHITYSLISTA` = a to-do list in `BuildNo.h:20` (118)| | |
|opt-outs EIMAALI (2027), EILEIMAT (99), NOREGNLY (102), TESTI (45), COMOFF (43), UINTI|defining them would remove code; they are undefined, so the code is on|`HkConsole/HkCons2.cpp:753`; `Hk/HkInit.cpp:3014`; `Hk/HkDef.h:112`|0 dead (lines always on)|
|_BORLANDC_, _CHAR_UNSIGNED, _CONSOLExx(x), _LIBCPP_VERSION, CONSOLE, HAVE_UNISTD_H, SADASOSA_XX, VIESTI, L, NOCOM_AIMA, COMOFF|typos and placeholders; `_BORLANDC_` is a typo of `__BORLANDC__` (N-2)|`Hk/HkTls.cpp:734,825,1170,1190`; `Hk/HkTulostus.cpp:1697,1775,2934,3454`; `Juk/VTulostus.cpp:1591,1666`; `Tp/TpComY.cpp:550,608`|12 (pragmas)|
|DOCTEST_* (~60), NOMINMAX, STDOUT_FILENO, WIN32_LEAN_AND_MEAN, TP_CORE_NAME|third-party and test configuration|`Tests/doctest.h`; `Tests/DoctestMain.cpp:21`; `Tests/core/CoreMain.cpp:28,45`|not in product|
|ALGE, XML, EMITXML, XMLSARJAT, TARKNO, UDPYHTEYS, HAJONTA0, MAXOSUUSLUKU0, VAIKALUKU0, MAXCOMDATA, JKOKO, DEBUG1, DEBUG2|TU-local|`Hk/HkInit.cpp:40`; `HkConsole/HkCons3.cpp:35-36`; `Juk/VXml.cpp:42`; `tputilv2/Inpknoot.cpp:31`; `Hk/HkCom32.cpp:38`; `ViestiWin/ConvDat.cpp:29-42`| |
|`Tp/TpComY32.cpp`|32-bit copy of `TpComY.cpp`, in no product (124 sites)| | |

### 1e. Project defines that no `#if` tests [verified by grep]

AMPU (ViestiWin; also `Juk/VDef.h:53`), ISODATA, JUKOHJ, JUKOLA and VAMAALI (ViestiWin); VERTAA, UUSI, MAAKNT
and PISTEET (HkMaali); EMITAG (ViestiMaali, ViestiWin); MILLISEKAJAT (HkKisaWin, also `Hk/HkDef.h:26`);
VERSIONO=510; STRICT (Windows SDK macro). "PISTEET", "UUSI" and "EMITAG" appear only as string literals or config keys.

### 1f. Corrections to the suggested classes

- `_CONSOLE` is a variant, not an include guard.
- `_DEBUG` is debug-diagnostic, not compiler-platform: all 10 of its sites are dead in the shipped builds.
- `BIT32` is compiler-platform.
- `MAALI` and `CONSOLELIB` are variants.
- `KSEK`, `LANG` and `SV` are other constants.
- `HIIHTO` and `MAKI` are forced-off variants.
- `NOMOUSE`, `MERI5`, `SAKKOCOM`, `JOUKKUEPISTEET` and `HIIHTOLAHTO` are forced-on features.
- The remaining 152 include-guard rows and 39 compiler-platform rows are confirmed in bulk.

## 3. Q2 answers ([verified] unless marked)

- **GAZ.** Console HK serial scoreboard (`Hk/HkIV.cpp:3102-3141,3205`; keys `Hk/HkInit.cpp:1376-1392`). The relay has no `#if` consumer for it. The GUIs have their own units: `ViestiWin/UnitGAZ.cpp` and `cbHk/UnitTaulu.cpp:213-226`.
- **ORAVA.** Dead. Its two tests (`HkConsole/HkCons3.cpp:1716`, `2660`, 25 lines) are inside `#ifdef SEURATUL`, which is compiled out (N-1). The meaning is guessed from the name: a variant of the series prompt.
- **VERTAA, PISTEET, UUSI, MAAKNT.** Nothing tests them; they are dead project defines. `PISTEET=` is only a config key (`Hk/HkInit.cpp:2027`; `Juk/VInit.cpp:1383`).
- **MERI5.** Three-stage points rule (`Hk/HkTulostus.cpp:2528-2575`), forced on by `Hk/HkDef.h:37-39`.
- **LAJUNEN.** Sirit/Zebra RFID reader support (`include/SiritReader.h:10`). The name is probably a customer or person (name-based guess).
- **PARTIO.** Relay patrol teams, together with the run-time flag `kilpparam.partio` (`Juk/VEmit.cpp:1530,2235`, where `tark_partio` is 421 lines; `JukConsole/laskent0.cpp:224`; `ViestiWin/UnitEmit.cpp:932`). Active in all three relay programs.
- **AIKAVERT.** Console time-comparison screen (`HkConsole/HkCons2.cpp:2581-2800`; `Hk/HkInit.cpp:1421`). The GUI twin is `cbHk/VertAikaUnit.cpp`.
- **ENNAKOI.** Anticipated bib entry (`HkConsole/HkCons2.cpp:2154-2170`, `JukConsole/vcons2.cpp:2985`). The GUI twin is `cbHk/UnitAjanotto.cpp:163`.
- **JOUKKUETUL.** HK team results (`Hk/HkTulostus.cpp:7615`), HkKisaWin only. The relay copy is never compiled.
- **SEURAVAL.** Club selection list and `lah_seura`.
- **KONETUNN.** Neighbour machine name in the connection status.
- **AUTOVA.** Automatic split-time processing.
- **VALUKIJA.** Forwards split-time reads to peers; implies AUTOVA in the relay.
- **SAKKOCOM.** Penalty serial channel, forced on in HK.
- **ESITARK.** Emit card pre-check screen; ViestiMaali only.
- **TESTIAJAT.** Ctrl-T test-time sender; ViestiMaali only.
- **PROFILOI.** Communication profiling (`Tp/TpComY.cpp:185`); the key exists in the relay only.
- **DBGFILE.** Raw-byte debug files per reader port, enabled by the `DBGTOFILE` key.
- **ALGETAULU.** Relay ALGE scoreboard.
- **AMPU.** Never tested. The real switch is `AMPUSUUNN`.
- **MILLISEKAJAT.** Defined but never tested.
- **MAKI, HIIHTO.** See 1a. The `?` comes from the conditional `#undef` (`Hk/HkDef.h:33-35,118-120`, `Hk/HkInitVanha.cpp:55`). The real value is undefined in both HK products.
- **EMITFILE.** It is "unknown" not in ViestiLuenta itself but at the `Juk/VIv.cpp:810` site, where ViestiLuenta's reach is `n` and ViestiWin's take is `?`. The cause is the conditional `#undef` at `Juk/VIv.cpp:47`. Real values: ViestiMaali defined, ViestiLuenta undefined, ViestiWin undefined.

## 4. T-008: console vs GUI

HkMaali vs HkKisaWin:

|switch|where|what differs|intent|
|---|---|---|---|
|_CONSOLE|124 sites, 3494 lines|TUI vs VCL|intentional|
|SERVER|3 sites, 549 lines|console TCP-server helpers, `VARASERVER`|looks intentional|
|GAZ|7 sites, 298 lines|console scoreboard|intentional (the GUI has `UnitTaulu`)|
|JOUKKUETUL|2 sites, 312 lines|team results only in the GUI|looks intentional|
|AIKAVERT, ENNAKOI|5 and 4 lines of shared code|console UI|intentional (GUI twins)|
|DBGFILE|13 sites, 64 lines|HkMaali Release lacks it|**drift** (N-7)|
|_BORLAND_, __BORLANDC__|11 + 46 sites, ~87 lines|includes and API names; the KUNTOSUUNN badge swap is GUI-only (H-081)|build difference, except H-081|
|ESITARK|`Hk/HkEmit.cpp:3969`, 3 lines|not defined in HK|none|
|EMIT/_CONSOLE `Hk/HkEmit.cpp:6241`, COMOFF, AUTOFILE/TAULU_2 `Hk/HkIV.cpp:3673`|119, 43, 13 lines|console UI|intentional|
|NREGNLY, MAXTUNNUS, SAKKOCOM, MERI5|header defaults|same effective values|none|

ViestiMaali/ViestiLuenta vs ViestiWin:

|switch|where|what differs|intent|
|---|---|---|---|
|_CONSOLE|130 sites, 4942 lines|UI|intentional|
|SERVER|571 lines|console only|as in HK|
|ESITARK|82 lines|the pre-check screen has no GUI equivalent found|probably a GUI **feature gap** [inferred]|
|DBGFILE|81 lines|missing from the GUI|drift (N-7)|
|TESTIAJAT, ENNAKOI|50 and 4+ lines|console only|intentional (ViestiWin has no ENNAKOI twin)|
|EMITFILE|8 lines|`EMIT_IN.DAT` input missing from ViestiWin (HkKisaWin has it)|drift (low)|
|PARTIO|17 lines (`Juk/VEmit.cpp:5551,5568`)|partly console only|intentional|
|NREGNLY|8 vs 32|T-024|known|
|_BORLAND_|ViestiWin lacks it|includes, `Tp/TpApu.cpp:17`|no behaviour change found|
|LUENTA vs Maali|4144 lines|Luenta has no LAJUNEN (1666), EMITHTTP (223), VALUKIJA/AUTOVA (116+24), ALGETAULU, ESITARK, ENNAKOI, TESTIAJAT or EMITFILE|reduced build; no RFID in a card-reader program is surprising, a question for the owner [inferred]|

## 5. Dependencies and conflicts

**Derived switches** [verified]:

- `AUTOFILE` comes from `TCPSIIRTO` (`Hk/HkDef.h:54`); the relay has no equivalent.
- `AUTOVA` comes from `VALUKIJA` (`Juk/VDef.h:38-40`).
- `EMITLEIMAT` comes from `(MAALI || AUTOVA) && EMIT && !EILEIMAT`.
- `NOCOM_AIKA` comes from `LUENTA && !NOCOM_AIMA`.
- `TCPSIIRTO` is forced locally in `com/WinUDP.cpp:21-23`.
- `TXTTVCOM` and `GAZCOM` are derived from `NREGNLY` and `MAX_LAHPORTTI` (`Hk/HkDef.h:91-93`, `Juk/VDef.h:59-60`).
- Forced on in HK: `MILLISEKAJAT`, `MERI5`, `JOUKKUEPISTEET`, `SAKKOCOM`, `NOMOUSE`, `HIIHTOLAHTO`.
- Forced off in HK: `MAKI`, `HIIHTO`.

**Exclusion and preference:**

- `LUENTA` excludes ALGETAULU, EMITFILE, ESITARK, TESTIAJAT and ENNAKOI (`Juk/VIv.cpp:47`; `JukConsole/vcons2.cpp:2402`).
- `JOUKKUETUL` and `SEURATUL` exclude `AMPUSUUNN` (`HkConsole/HkCons3.cpp:40,2037`).
- `HALIKKO` takes precedence over `MKV` in `MAXSARJALUKU`.
- `TAULU` is treated as equivalent to `ALGETAULU`.

**Name collisions:**

- **GDIPRINTER** [verified] has three meanings:
  - a tputilv2 project flag;
  - an empty fallback in `tputilv2/Wrttext.cpp:33-35`;
  - the printer-type constant 7 in `Hk/HkMuotoilu.h:33`, `Juk/VDef.h:171` and `Juk/VMuotoilu.h:33`.

  They coexist only because `Wrttext.cpp` includes none of those headers. Tputil-XE does not define it.
- **Short constants.** `SU`, `EN`, `SV` and `LANG` (`Tp/TpDef.h:66-70`) and `PROPRIEMU`, `TXTTAB` and `CANON` (`Juk/VDef.h:168-171`) are short macro names. No collision was found for the names grepped, but the 25 non-directive hits for `EN` were not examined.
- **ALGE** is an empty TU-local define in 4 files and also the device id string "ALGE".

**Tested in one TU, defined TU-locally in another:**

|macro|defined|tested|effect|
|---|---|---|---|
|SEURATUL|`Hk/HkTulostus.cpp:35`; `Juk/VTulostus.cpp:47` (under EI_OLE); `Juk/XMLtulos.cpp:43`|`HkConsole/HkCons3.cpp:600,1539,2642,2652,2730,2885,2924`; `JukConsole/VConTul.cpp:2017`; `Juk/VTulostus.cpp:7595`|~393 dead lines in HkMaali (N-1)|
|XML|`HkConsole/HkCons3.cpp:36`|`JukConsole/VConTul.cpp:1308,2004` (21 lines)|dead in the relay console (N-8)|
|EMITXML|`HkConsole/HkCons3.cpp:35`; `JukConsole/VConTul.cpp:35`|both|ok|
|TARKNO|`tputilv2/Inpknoot.cpp:31`|`tputilv2/Inpknoos.cpp` (23 dead); `HkConsole/HkCons2.cpp:95`|alternative file variant|
|EMITFILE|`#undef` at `Hk/HkIV.cpp:48`, `Juk/VIv.cpp:47`|`Tp/TpLaitteet.cpp:1911`; `Juk/VEmit.cpp:4726`|section 3|

**Non-shipped configurations** (from `kokoonpanot.csv`):

| product | configuration | differs from Release |
|---|---|---|
| HkMaali | DebugV200 | `NREGNLY=4`, `VALUKU=200`, `VERSIONO=510`, `_DEBUG` |
| HkMaali | ReleaseVA200 | as DebugV200, without `_DEBUG` |
| HkMaali | Demo | `DEMO`, `NREGNLY=4` |
| HkMaali | Debug | `DBGFILE`, `_DEBUG` |
| ViestiMaali | Debug250, Release150os | 25 classes, 250 legs, 8 split points |
| ViestiLuenta | Debug250, Release150os | no change (30 legs, 250 classes, 12 split points) |
| ViestiWin | Debug, Debug150os, Release150os | 25 classes, 250 legs, 8 split points |
| ViestiWin | Demo | adds `DEMO` |
| HkKisaWin | 200Debug | `VALUKU=200`, `_DEBUG` |
| HkKisaWin | 200Release | `N_PV=2`, `VALUKU=200` |
| HkKisaWin | Demo | `DEMO` |

Effect:

- Builds with different `VALUKU`, `MAXOSUUSLUKU` or `VAIKALUKU` produce incompatible data files and wire records (`tiedostomuodot.md:266-269`).
- `DEMO` caps the number of competitors at 30 (HK) or 20 (relay).

## 6. Defect candidates (severity is a guess)

- **N-1 (medium for function, low for the typo)** [verified]:
  - `HkConsole/HkCons3.cpp:40-42` contains `#undefine SEURATUL`, which is not a valid directive. It is hidden in a branch that is never taken.
  - `SEURATUL` is defined only in `Hk/HkTulostus.cpp:35`. The console club-results menu `seuralista` and the ORAVA prompts (~393 lines) are therefore compiled out of HkMaali, and the ORAVA project define has no effect.
  - Whether this is intended is unclear; the code dates from the 2015-07-18 import.
- **N-2 (low)** [verified]: `_BORLANDC_` with single underscores is used at 12 product sites around `#pragma warn -par/+par`, so those pragmas never apply in bcc32.
- **N-3 (low)** [inferred]: `Juk/VDef.h:49` tests `NOCOM_AIMA`, probably a typo of `NOCOM_AIKA`.
- **N-4 (low, latent)** [verified]:
  - `Juk/VDef.h:57-61` defines `TXTTVCOM` and `GAZCOM` inside `#ifndef NCOMEXTRA`, so a project define of `NCOMEXTRA` would leave them undefined.
  - The two domains number the same channel differently: in HK `GAZCOM == TXTTVCOM == TV1COM`, in the relay `GAZCOM = TXTTVCOM-1`.
- **N-5 (medium-low, non-shipped)** [verified]:
  - ViestiLuenta Debug250 and Release150os keep the Release sizes.
  - ViestiMaali and ViestiWin configurations of the same names use 250 legs, 25 classes and 8 split points.
  - Mixing the two builds changes record sizes.
- **N-6 (low)** [verified]: 12 project defines have no consumer (1e).
- **N-7 (low)** [verified]: `DBGFILE` is defined in HkKisaWin Release, ViestiMaali and ViestiLuenta, but not in HkMaali Release or ViestiWin.
- **N-8 (medium-low)** [verified]:
  - `JukConsole/VConTul.cpp:1308-1322,2004-2013` is gated by `XML`, which that TU never defines, yet the menu still offers X)ML (`:1332,1350`).
  - The gated block calls `xmlots(0, tulprm)` with two arguments, while the function is declared with one (`Juk/VTulostus.cpp:64`, `Juk/XMLtulos.cpp:194`). The block is therefore stale and would not compile.
  - Another relay console XML path (`Juk/VTulostus.cpp:8960`) was not checked.
- **N-9 (info):** `tputilv2/Inpknoos.cpp` tests `TARKNO`, which only `Inpknoot.cpp` defines.
- **N-10 (low)** [verified]: the GDIPRINTER collision. It would cause a redefinition and a different value if `Wrttext.cpp` ever included those headers.
- **N-11 (info):** the `#undef EMITFILE` under LUENTA at `Hk/HkIV.cpp:48` is dead copy-paste.
- **N-12 (info)** [verified]: the relay `JOUKKUETUL` code (`Juk/VTulostus.cpp:7672`, 482 lines) is dead.
- **N-13 (low)** [verified]:
  - `Juk/VDef.h:117` reads `#endif !defined(MAXSARJALUKU)`, with tokens after `#endif`.
  - `Juk/VDef.h:125` `#define MAXNSEURA 2000` is unguarded, unlike `Hk/HkDef.h:131`.
- **N-14 (info):** dead line counts: MAKI 1232, HIIHTO ~8, AMPUSUUNN 967, EI_OLE 1774.

## 7. Not finished

- **Meanings of the never-defined groups** in 1d are guesses from names and samples [inferred]. At most 3 lines of each block were read.
- **Relay console XML export** was not traced (N-8).
- **The 25 `EN` hits** were not examined.
- **Evaluator artifacts.** The csv comes from the lexical evaluator, so the values for `MAXTUNNUS`, `LENIMI`, `LSNIMI` and the TU-local `#ifndef` defaults are artifacts, as noted per row.
- **Build output.** No build was run, so no value was confirmed from preprocessor output.
