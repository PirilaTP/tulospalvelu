# C-12 osa D / part D: independent review

Agent `independent-verifier` (Opus), 2026-11-26, about 114 k tokens. The main session saved this report
from the agent's hand-back. Claims, evidence and labels are unchanged; wording and layout are condensed.
Static review only. The agent read every cited line itself and made no repo edits. Paths are relative
to `TPsource/V52/`. Claim labels A1-A7, B1-B7 and C1-C5 are those of the review request (the main
session's selection from the part A, B and C reports).

Tally: 12 confirmed (B4 and A2 worse than claimed), 7 partly, 0 refuted. B4's concrete side effects
need a run.

The main session re-checked B4 (`Juk/VInit.cpp:74`, `:652`; `ViestiWin/UnitMain.cpp:220`): confirmed.

## Part A

**A1. Confirmed, with corrections.**

- `HkConsole/HkCons3.cpp:40-42` is a skipped group, so its `#undefine` is a legal non-directive.
- `SEURATUL` is defined only TU-locally (`Hk/HkTulostus.cpp:35`, `Juk/VTulostus.cpp:47`, `Juk/XMLtulos.cpp:43`).
- The code compiled out is `:600-627`, `:1539-1863` (about 325 lines, not about 393) and `:2642-2676`, `:2730`, `:2885`, `:2924`.
- Both `ORAVA` tests (`:1716`, `:2660`) are inside `SEURATUL` blocks, so the `ORAVA` define in `HkMaali.vcxproj:154` has no effect.
- New: the dead block contains a latent syntax error at `:606` (`wchar _t ch`), so the block has been dead for a long time.
- Impact: low. A console feature is missing (club and district results); nothing crashes.

**A2. Confirmed; the menu item is broken.**

- The `XML` macro is never defined in `JukConsole/VConTul.cpp`. As a result `xmlvalinnat` (`:1283-1324`) only opens the file.
- `xmlots(0, tulprm)` at `:1320` passes two arguments to a one-argument function (`Juk/XMLtulos.cpp:194`), with no declaration in the file.
- `xmlloppu` (`:2004-2013`) is skipped as well.
- The per-class body still runs through `Juk/VTulostus.cpp:5917`, `:6219` and `:6422`.
- `Juk/VTulostus.cpp:8960` is inside `list()`, which is under `#ifndef _CONSOLE` (`:8707`). The console has no other path.
- Effect: X)ML (`:1332`, `:1350`) writes a file without an XML declaration or root element. `tag_a` (`Juk/XMLtulos.cpp:192`) is zero unless an earlier autofile XML run called `xmlots`, and NULL tags are skipped (`tputilv2/TextFileClass.cpp:770`, `:677`), so the file is close to empty. Nothing crashes.
- `optiot=0` (`VConTul.cpp:1581`) rules out the IOF30 path.
- Severity: medium (silent wrong output). Reachable in ViestiMaali and ViestiLuenta with a normal configuration; one run settles it.

**A3. Confirmed; no behavioural difference.**

- `ViestiWin.cbproj` Defines (`:134`, `:195`, `:211`, `:223`) lack `_BORLAND_`, but no header tests it.
- In the ViestiWin translation units, `Tp/TpApu.cpp:17` and `cbTp/cbTpApu.cpp:17` only gate `#include <vcl.h>`.
- `Tp/TpApu.cpp:208` and `:218` are inside a `/* */` comment.
- The HK `_BORLAND_` sites (`HkIV.cpp:1663`, H-081) are not in ViestiWin.

**A4. Partly.** The conclusion holds: nothing defines either macro. The mechanisms differ:

- `MAKI`: `#undef` at `Hk/HkDef.h:33-35`.
- `HIIHTO`: `:118-120` has `#undef` followed by `#define HIIHTO 1`, so a defined `HIIHTO` would stay defined. It is undefined only because nothing defines it.

**A5. Confirmed; harmless.**

- `_BORLANDC_` at `Hk/HkTls.cpp:734`, `:825`, `:1170`, `:1190`, `Hk/HkTulostus.cpp:1697`, `Juk/VTulostus.cpp:1591`, `:1666` and `Tp/TpComY*.cpp` guards only `#pragma warn -par/+par`.
- "Typo" is inferred.

**A6. Partly.**

| Project | Configurations | Legs / classes / split points |
|---|---|---|
| ViestiLuenta | all four | 30 / 250 / 12 (`VS/V/JukLuenta520.vcxproj:95`, `:112`, `:131`, `:152`) |
| ViestiMaali | Debug250, Release150os | 250 / 25 / 8 (`JukMaali520.vcxproj:115`, `:157`) |
| ViestiWin | Debug Build (Cfg_1, `:223`), Debug150os (Cfg_5, inherits Cfg_1), Release150os (Cfg_4, `:343`) | 250 / 25 / 8 |
| ViestiWin | Release Build (`:285`), Demo (`:319`) | 30 / 250 / 12 |

- Correction: ViestiWin has no Debug250 configuration. Its configurations are Debug Build = Cfg_1, Release Build = Cfg_2, Demo = Cfg_3, Release150os = Cfg_4, Debug150os = Cfg_5 (`:1049-1067`).
- Additional: ViestiWin's ordinary Debug Build does not have the shipped limits.

**A7. Confirmed.** `LAJUNEN` does not appear in `VS/V/JukLuenta520.vcxproj` and no source defines it. The RFID headers (`include/SiritReader.h:16`, `ZebraReader.h:19`) are gated by it.

## Part B

**B1. Confirmed; duplicates H-036.**

- `Juk/VInit.cpp:1137-1141` compares 6 characters, and the bounds test at `:1139` is always true.
- The array is `katkkierto[MAX_LAHPORTTI]` (`Juk/VGlobals.cpp:319`), and `MAX_LAHPORTTI` is 64 in all relay products.
- `ny` is the last character minus `'1'`:
  - `'0'` gives -1, an out-of-bounds write.
  - `'O'` (a bare `KATKKIERTO`) gives 30 and `'='` gives 12: in bounds but the wrong ports.
  - Any character at or above `'r'` gives 64 or more, for example `'Ä'`.
- Only ports 1-9 can be addressed.
- Severity: low.

**B2. Partly.**

- **Confirmed editor writes:**
  - `cbHk/UnitCfg.cpp:501` writes `AIKACOM=`.
  - `ViestiWin/UnitCfg.cpp` writes `AIKAERO1=` and `AIKAERO2=` (`:397`, `:412`), `AIKACOM=` (`:429`), and further keys at `:599`, `:627`, `:632`/`:634` (`EMITAJAT=` and bare `EMITAJAT`), `:703` and `:732`.
- **Corrections:**
  - The HK `AUTOJAKELU=` write is at `cbHk/UnitCfg.cpp:801`; line `:1332` is the reader.
  - Refuted: HK does accept `TAKAA-AJO` (`Hk/HkInit.cpp:757`) and `NOLLAKÄRKI` (`:690`). The cp1252 bytes match.
- **Run-time handlers:**
  - HkInit has only `AIKA_COM=` (`:1558`) and no `AUTOJAKELU` handler.
  - VInit has `AIKAERO=` with n=8 (`:1467`) and `AIKA_COM=` (`:1473`), and nothing for EMITALKU, EMITJONO, EMITAJAT, SUORASYÖTTÖ or AUTOJAKELU.
- **Shorter-prefix collisions:** checked, none.
  - `AUTO` (4 characters) needs `fldn[4]` to be NUL or `=`.
  - `EMIT` (5 characters, including the NUL) is exact.
  - EMITANALYYSI (8), EMITAG (6), EMITLUKU (6), EMITTO (6) and AIKALUKIJA (6) do not match.
- **Round trip:** each editor reads `AIKA_COM=` (`cbHk/UnitCfg.cpp:1692`, `ViestiWin/UnitCfg.cpp:1480`) but writes `AIKACOM=`, so a round trip loses the setting.
- **Origin:** the ViestiWin keys look copied from HK (HkInit EMITALKU `:849`, EMITJONO `:1994`, EMITAJAT `:2246`, SUORASYÖTTÖ `:1316`).
- **Severity:** low to medium (settings silently ignored). Reachable in both GUIs.

**B3. Confirmed.**

- At `Juk/VInit.cpp:1168-1169` tokenising starts at `fldn+10`, which is the `'='` character. `'='` is not a delimiter, so `_wtoi("=n")` returns 0.
- `UDPviive_lah` therefore keeps its default of 10 (`Tp/TpComY.cpp:64`). The other three values are read correctly.
- HK is correct (`Hk/HkInit.cpp:1212`).
- Severity: low.

**B4. Confirmed, and worse than claimed.**

- `cfgfilename[41]` is declared at `Juk/VInit.cpp:74`; `wcsncpy(..., 120)` is at `:652`.
- `wcsncpy` zero-pads up to n, so every non-empty `cfgflname` writes 120 wchar_t: 79 characters (158 bytes) past the buffer, whatever the path length.
- ViestiWin always passes a full OpenDialog path (`ViestiWin/UnitMain.cpp:1393-1403`, `:1414-1426`, `:1542`). The path goes through `Initialisoi` to `aloitus(0, 0, cfgflnm)` at `:220`.
- The overrun therefore happens on every ViestiWin start that opens a competition.
- HK is safe (`cfgfilename[201]`, `Hk/HkInit.cpp:59`).
- What is overwritten depends on the bcc32 static layout. The source-order neighbours are `autostr`, `korkeat`, `star` and `baudst[]="9600,n,8,1"` [inferred].
- Severity: medium. A `.map` file or a watchpoint settles the side effects.

**B5. Confirmed.**

- `ALGE` is defined unconditionally (`Juk/VInit.cpp:41`).
- The outer test (`:1593-1599`) lists FEIG, IMPINJ, SIRIT and ZEBRA, but their branches are `#ifdef LAJUNEN` (`:1604-1628`).
- In ViestiLuenta these keys therefore fall to `regnly[0]=3` (5 if `fldn[4]=='4'`), with `port_regnly[0]=1` and `ajanottofl=1`.
- Severity: low.

**B6. Confirmed.**

- The outer list (`Hk/HkInit.cpp:1799-1809`) lacks TDC, S3 and S4, so the branches at `:1828`, `:1832` and `:1835` are unreachable.
- No other code assigns `LID_TDC` (`include/TpLaitteet.h:24`). The TDC8001 handling (`Hk/HkIV.cpp:2109`, `:2301`, `:2693`) therefore cannot be enabled from the configuration.
- `cbHk/UnitCfg.cpp:1766` has the same pattern.
- Severity: low.

**B7. Confirmed.**

- `cbHk/WinHk.cpp:279` and `ViestiWin/UnitMain.cpp:220` both call `aloitus(0, 0, cfgflnm)`.
- The only `aloitus(_argc, _argv)` (`ViestiWin/UnitMain.cpp:553`) is commented out.
- ViestiWin reads `_argc` only for `INIT=` (`:1527-1536`).
- The console programs pass argv (`HkConsole/HkMain.cpp:203`, `JukConsole/laskenta.cpp:108`).

## Part C

**C1. Confirmed; duplicates H-134.**

- New: at a stage change only -1 is handled.
- A parse error (`er=1`, `Hk/HkXml.cpp:1828-1851`; `lue_radat_xml` returns before `siirraXml*`, `Hk/HkEmit.cpp:853-854`) takes the else branch at `HkIx.cpp:1941` as if the read succeeded. That branch works on course memory already freed at `:1908`.
- At start-up the same error correctly returns 1 (`HkInit.cpp:3602-3604`).
- Extend H-134.

**C2. Partly.**

- The fallback is real (`Hk/HkSrj.cpp:72-75`, `Juk/vSarjat.cpp:315-318`).
- It is not silent: `lueEventXml` shows the parse error (`Hk/HkXml.cpp:1832-1837`, `Juk/VXml.cpp:1695`). Only the fallback to an old KILP.SRJ is silent.
- Side defects:
  - The fallback message at `Hk/HkXml.cpp:1834` passes `filename` to a `%d`.
  - `HkSrj.cpp:79` reports `sarjaxmlfname` as missing although KILP.SRJ is the file that is missing.
- Severity: low to medium.

**C3. Confirmed; duplicates H-063 (T-020).**

- The defect is at `Juk/VTulostus.cpp:8629-8631`. `init` is set only in the file branch, so `fclose(NULL)` repeats on every call.
- The code is under `#ifndef LUENTA` (`:8620`).
- It is reached from ViestiMaali (`JukConsole/VConTul.cpp:1961`) and ViestiWin (`Juk/VTulostus.cpp:9080`) when course-variation results are printed.
- MSVC Release ends the process: there is no `_set_invalid_parameter_handler` (`muistiturvallisuus.md:284-287`). bcc32 returns EOF.
- Severity: medium.

**C4. Partly; mostly covered by H-043.**

- The replay itself is unchecked (`ViestiWin/UnitMySQL.cpp:597-624`).
- However, the file name is relative, and `Initialisoi` sets the current directory to the competition directory (`ViestiWin/UnitMain.cpp:211`).
- The replay only resends record numbers: `kirjSQLrivi` re-reads the current record (`ViestiWin/UnitMySQL.cpp:414`) under the current `kilpkoodi`. A "foreign" file therefore causes only extra writes of current records.
- New aspects: directory reuse, and record numbers out of range (`GETREC` bounds not checked). The harmful case is already H-043 (with H-014).
- Severity: low.

**C5. Confirmed, slightly wider.**

- `cbHk/UnitEmit.cpp:186` and `:193` restore only the menu ticks. The variables change only in the click handlers (`:2907`, `:3197`).
- `:113` sets `Checked = lukutoisto`, which is 1 or 2 and so always true: the menu shows double reading as required even when `lukutoisto=1` (`Hk/HkInit.cpp:820`, `:2500`).
- ViestiWin has the same pattern (`ViestiWin/UnitEmit.cpp:113` vs `:2716`).
- `kuntoMaali` and `kuntoLahto` are kept in sync (`:187-190`).
- Severity: low to medium.

## Register overlap

- **H-036:** B1 duplicates it. New detail: the index map.
- **H-134:** C1 duplicates it. Extend it with the parse-error branch.
- **H-063 / T-020:** C3 duplicates them.
- **H-043 (with H-014):** covers the harmful case of C4.
- **H-081:** related to A3.
- **T-045:** related to A1, A4 and B5.
- **T-034:** fits C5's HK/ViestiWin copy pattern.
- **No existing entry:** A1, A2, A5, A6, A7, B2-B7, C2 and C5.

## Open questions

1. **B4:** what bcc32 places after `cfgfilename` in ViestiWin (a `.map` file or a debugger would show it).
2. **A2:** what a console X)ML file contains after an earlier autofile XML run (needs a run).
3. **C4:** whether `GETREC` checks bounds for a record number beyond the end of the file.
4. **C1:** what the stage-change else branch does with the freed `rata` after a parse error (needs a run).
5. **C2:** whether users notice the class-data fallback (owner question).
