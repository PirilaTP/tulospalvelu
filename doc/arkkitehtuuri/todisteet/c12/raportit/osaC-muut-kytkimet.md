# C-12 osa C / part C: file-presence, GUI settings, env/process inputs and competition parameters as feature switches

Agent `architecture-analyst` (Sonnet), 2026-11-26, about 163 k tokens. The main session saved this
report from the agent's hand-back. Claims, evidence and labels are unchanged; the layout is condensed.
Static reading only. Paths are relative to `TPsource/V52/`. Tags: [verified] = the agent read the
code; [inferred] = the chain was not fully traced. Labels D1-D8 are provisional and not register IDs.

## Summary

- **File presence.** The switches that change real behaviour are:
  - `AUTOFILE.LST`: its presence selects multi-file autoprint.
  - `RADAT*.XML`: its absence falls back to `RADAT*.LST` + `LEIMAT*.LST`.
  - `KilpSrj.xml`: its absence, or a parse error, falls back to `KILP.SRJ`.
  - `HTMLALK?.TXT`: the template replaces the HTML head.
  - `TLSKERR.LST`, `SARJAYHD.LST`, `XMLPISTE.LST`, `TVPISTE.LST`, `vaihevalinta.cfg` and the `korostus` files.
- **GUI persistence.**
  - `ikkunat.xml` holds the window layout and per-window options. It is read only when the `IKKUNAT` key is set.
  - `Init.cfg` holds the start mode of both GUIs.
  - `SQLjonoVarm.dat` is a recovery queue.
  - The registry is used once: IE browser emulation, in HKCU.
  - There are no `TIniFile` or `TRegistry` uses.
- **Environment.** Product code has no `getenv` or `GetEnvironmentVariable` calls. The only such call is in the tests (`Tests/fixtures/TpCore.cpp:35`). Process inputs are argv, the working directory and the exe name.
- **Competition parameters.**
  - HK: `Discipline` (`kilplaji`), `Subtype` (`alalaji`), the badge type, `StartGate`, `VariableClass`, `VariableBib` and `IntIdLen` switch code paths.
  - Relay: `Subtype` sets `partio` and `rogaining`.
  - The badge type can turn emit reading on by itself.
- **Defect candidates.** D1-D8 (section 6).

## 1. File-presence features

All files are relative to the current directory unless noted.

| name | read at | effect at | programs | default when absent | meaning |
|---|---|---|---|---|---|
| `AUTOFILE.LST` (name `Hk/HkInit.cpp:3117`, `Juk/VInit.cpp:2531`) | `Hk/HkTulostus.cpp:7147`; `Juk/VTulostus.cpp:7038`, `:7161` | Hk: loop `aflst->IsOpen() && kohde H/M && !yksihtml` at `Hk/HkTulostus.cpp:7150`. Juk: `yksihtml = true` when the file cannot be opened, `Juk/VTulostus.cpp:7039-7040` | all four (shared code `#ifdef AUTOFILE`, `Hk/HkTulostus.cpp:6978-7351`, `Juk/VTulostus.cpp:7030-7336`; `TCPSIIRTO` forces `AUTOFILE` on, `Hk/HkDef.h:53-54`) | single combined output (Juk [verified]; Hk [inferred]) | Present: per-class HTML, MHTML and XML files from `HTML=`/`MHTML=` lines. Re-read on every cycle; needs `afvali` (key, `Hk/HkInit.cpp:988`) |
| `korostus<k_pv+1>.lst` (HK), `korostus.lst` (relay) | `Hk/HkInit.cpp:220`, `:235`; `Juk/VInit.cpp:2200`, `:2215` | `Hk/HkInit.cpp:3623-3624`, `Juk/VInit.cpp:2901-2902`, menus `cbHk/AsetusUnit.cpp:53`, `ViestiWin/VertAikaUnit.cpp:60`; fills `korRaja`, `vipLaji` and `korVari` (`Hk/HkInit.cpp:226-262`) | GUI only (`#ifndef _CONSOLE`, `Hk/HkInit.cpp:3622`, `Juk/VInit.cpp:2901`) | `korRaja` = {3,5}, no VIP, no colours | Highlight limits per class, VIP and colour lines; use of `korRaja` not traced [inferred] |
| `vaihevalinta.cfg` | read `Hk/HkInit.cpp:289-310`; written `:314-325` | `k_pv = luevaihevalinta()` `:3189`; written at `Hk/HkIx.cpp:1997-1998`; -1 asks the user (`:3190-3193`) | HK shared | stage 1 | Remembers the last stage; only with the `vaihevalinta` key |
| `peruscfg.cfg`, cfg (`LASKENTA.CFG` or `CFG=` in argv[1]) | `Hk/HkInit.cpp:332-345`, `:567-570`; `Juk/VInit.cpp:438`, `:655-658` | `lue_parametrit` | all four | `peruscfg.cfg` missing: silent; named cfg missing: message only if `cfgparam` (`Hk/HkInit.cpp:344-349`) | Layers: peruscfg, then cfg, then argv (`:572-583`) |
| `RADAT<k_pv+1>.XML`, relay `radat.xml` | `Hk/HkEmit.cpp:852-853`; `Juk/VEmit.cpp:893` | `ratatiedot = 2` `Hk/HkEmit.cpp:861`; -1 when the file is missing (`Hk/HkXml.cpp:1825`) | all four | `RADAT<n>.LST` + `LEIMAT.LST` | Course data |
| `RADAT<n>.LST`, `LEIMAT<n>.LST` | `Hk/HkEmit.cpp:2344-2349`, `:2878`; `Juk/VEmit.cpp:2773`, `:3444` | only if the XML read returned -1: `Hk/HkInit.cpp:3594-3601`, `Juk/VInit.cpp:2868-2877` | all four | `emitfl==-1` or `2` tolerates a missing file (`Hk/HkEmit.cpp:2882-2886`); otherwise an error and `emitfl=0` | Legacy course data; block runs only when `ToimintaTila==2 && emitfl==1 && !esitark && esiluenta!=1` (`Hk/HkInit.cpp:3592`) |
| `KilpSrj.xml`, then `KILP.SRJ` | `Hk/HkSrj.cpp:72`, `:81`; `Juk/vSarjat.cpp:315` | `if (!lueEventXml(...)) {...}` | all four | `KILP.SRJ`; if that is also missing the message names `KilpSrj.xml` (`Hk/HkSrj.cpp:83`) | A parse error also falls back (D2) |
| `HTMLALKY/M.TXT`, `HTMLLOPY/M.TXT` | `Hk/HkTulostus.cpp:6903-6915`, `:6959-6970`; `Juk/VTulostus.cpp:6977-6990`, `:7011` | `htmlalkufl=1` replaces the head (`Hk/HkTulostus.cpp:6921-6934`); closing tags are written only without a template (`:6973-6974`) | all four | built-in head and closing tags | HK ignores the templates for responsive pages (`:6908-6910`); Juk has no such test |
| `TLSKERR.LST` | `Hk/HkInit.cpp:2793` | `Sarjat[].tlskerroin/tlsjakaja` `:2781-2784`; skipped if `n_pv_akt<2` (`:2790`) | HK shared | 1/1 | Per-class time scaling across stages |
| `SARJAYHD.LST` | `Hk/HkSrj.cpp:510`; `Juk/vSarjat.cpp:245` | `Hk/HkInit.cpp:3330`; `Juk/VInit.cpp:2755` | all four | silent | Combined classes |
| `XMLPISTE.LST` | `Hk/HkXmlTulokset.cpp:196` | `xmlpiste[]` `:191-205` | HK shared | identity | Split points in XML output |
| `TVPISTE.LST`, `TVFILE.TXT` | `Hk/HkTV.cpp:54`, `:83` | `tv_val`; `TVFILE.TXT` written when `tv_com==999` | HK shared | `i-1` | TV feed; TV mode only |
| `LAHDEPISTEET*.LST`, `BOXOHJE.LST`, `LAHDEJONOT*.LST` | `Hk/HkIV.cpp:202`, `:382`; `Juk/VIv.cpp:165`, `:381` | `Hk/HkIV.cpp:4240-4241`, `Juk/VIv.cpp:2891-2892` | all four | none | Start points and start queues; only with `lahdepistehaku` (key) |
| `LAINAEMT.LST` / `LAINAEMIT.LST` | `Hk/HkEmit.cpp:1399`, `:1415-1418`; `Juk/VEmit.cpp:939-944` | `lainaemit` table | all four | none | Loaned emit cards; only with `lainatied` (key) |
| `EMITKDI.LST`, `TUNNKDI.LST`, `EMITREK.LST`, `PISTESRA.LST`, `numtaulu.txt`, `ERADAT.LST` | `Hk/HkEmit.cpp:1468-1486`; `JukConsole/laskemtn.cpp:337`; `JukConsole/VConTul.cpp:1168`; `JukConsole/lasknum5.cpp:48`; `Hk/HkInit.cpp:2838` | menus or dialogs | mostly console | error message | Operator imports, not switches |
| `pistesrj.lst` | `Hk/HkTulostus.cpp:7371`, `:7384-7386`; `Juk/VTulostus.cpp:7393` | falls back to the print-form value (`Hk/HkTulostus.cpp:7430-7433`) | GUI only (`#ifdef JOUKKUEPISTEET`, `#ifndef _CONSOLE`) | form value | Team-points limits |
| `PIIRIT.LST`, `SIJAPISTEET.LST` | `Hk/HkTulostus.cpp:7580`, `:7625`; `Juk/VTulostus.cpp:7647`, `:7695`, `:7947` | `luepiirit`: no caller found (declaration only, `cbHk/UnitPiirival.cpp:29`) | shared | not read | District and placement points [inferred] |
| `AIKARAJAT.TXT` | `Juk/VEmit.cpp:3389-3391` | only if `kilpparam.rogaining` (`:3575`) | relay | message, return | Rogaining time limits |
| `HAJONNAT.LST` | `Juk/VTulostus.cpp:8615`, `:8628-8629` | `tiedostosta` (`:8633-8650`); `#ifndef LUENTA` | relay shared | forking from the course list | D3 |
| `EMIT_IN.DAT` | `Hk/HkIV.cpp:890`; `Juk/VIv.cpp:823` | `file2emit` | `#ifdef EMITFILE` (`Hk/HkIV.cpp:877-937`) | returns 1 | Emit records from a file |
| `ESITARK.DAT` | `Hk/HkEmit.cpp:3037-3038`; `Juk/VEmit.cpp:3599` | when `esitark \|\| esiluenta==1` | all four | `EMITn.DAT` | Pre-check file |
| `ComTestParam.txt` | `Tp/TpComY.cpp:3038-3041` | `testi==2` (`Hk/HkInit.cpp:3713-3714`, `Juk/VInit.cpp:2962-2963`) | all four | defaults `:3042-3051` | Test sender |
| `yhtjonot.txt` | `Tp/TpApu.cpp:789-792`; `cbTp/TpApu.cpp:315` | no caller found | none | n/a | Dead code [inferred] |
| `__paikat__.txt` | `Hk/HkSak.cpp:121-123` | guard `er == 1`, but `er` is always 0 (`:114-121`) | HK | never opened | Dead code (D6) |
| `Init.cfg`, `KilpailuLuettelo.xml`, `SQLjonoVarm.dat` | section 2 | | GUI | | |
| `DBGFILE.TXT`, `sifile.txt`, `profdata.txt`, `__FCOMFILE__.TMP`, `ArvDebug.txt` | `Tp/TpLaitteet.cpp:966`, `:1312`; `Tp/TpComY.cpp:1966-1973`, `:206`, `:2934`; `cbHk/HkArv2.cpp:611` | outputs only | | | Debug logs |

No lock file, flag file or single-instance check exists: there is no `CreateMutex`, `FindFirstFile`, `remove`, `DeleteFile` or `_unlink` [verified by grep, `include/` not searched].

## 2. GUI persistent settings

| name | kind | read at | effect at | programs | default | meaning |
|---|---|---|---|---|---|---|
| `ikkunat.xml` (or `IKKUNAT=` file) | gui-xml | key `Hk/HkInit.cpp:641-646`, `Juk/VInit.cpp:694-699`; load `cbHk/WinHk.cpp:365-368`, `ViestiWin/UnitMain.cpp:266-269`, `:553-555`; parser `cbHk/WinParamXml.cpp:340-342`, `:511-`, `:536-` | form positions and per-window options `cbHk/WinParamXml.cpp:197-335`; written by `TallennaIkkunat` (`:1432-`) from the menu | both GUIs; `haeIkkunat` is `#ifndef _CONSOLE` (`Hk/HkIx.cpp:1999-2001`) | no layout | Window types `PTVENISARBCD` (`cbHk/WinParamXml.cpp:48-50`). Only open windows are saved (`:1440-1445`). Options include Rajaus, Seuratieto, Kokonaistulos, Smart, Yhteistulos, NaytaTulos, Ennakoi, VaadiVahvistus, Jono, OletusPiste; for Emit: VaadiToisto, LueVialliset, EmitvaihtoVaiheille, MerkLainat, HenkTietokanta, KuntoKone, VahvIlmAina |
| old layout file (e.g. `oletus.ikk`) | gui-xml | `cbHk/WinHk.cpp:908-925` (first line `<Ikkunat` decides the format) | | both GUIs | | Two formats |
| `Init.cfg` (in `InitDir`, `cbHk/WinHk.cpp:189`, `ViestiWin/UnitMain.cpp:168`) | gui text | `cbHk/WinHk.cpp:1645-1678`; `ViestiWin/UnitMain.cpp:1456-1491` | `Aloita` `cbHk/WinHk.cpp:1705-1737`, `ViestiWin/UnitMain.cpp:1535-1547`; written by `cbHk/UnitAvausMaar.cpp:83-107`, `ViestiWin/UnitAvausMaar.cpp:81` | both GUIs | mode 0, no auto-open | `ILM` mode 1, `TULOSP` mode 2, `KUNT` sets `kuntosuunnmoodi=-1` (`cbHk/WinHk.cpp:1707-1710`), `LUETT` competition chooser, `AVAA` open at start, `CFG=file`. ViestiWin has no `KUNT` or `LUETT` |
| argv[1] `INIT=<file>` | process | `ViestiWin/UnitMain.cpp:1527-1534` | `lueInit` | ViestiWin only | `Init.cfg` | HkKisaWin has no `_argc` use |
| `KilpailuLuettelo.xml` | file | `cbHk/UnitKisaLuett.cpp:284`, `cbHk/UnitValitseKilp.cpp:108` | chooser `cbHk/WinHk.cpp:1711-1721` | HkKisaWin | none | Only with `LUETT` and `AVAA` |
| `SQLjonoVarm.dat` | file + checkbox | `ViestiWin/UnitMySQL.cpp:597`, `:605-624`; `cbHk/UnitMySQL.cpp:640-700` | replays the queue tail into `dKilpSQLjono[]` | GUI | not used | Only when `CBVarmista->Checked` (`ViestiWin/UnitMySQL.cpp:602`); D4 |
| HKCU `...FEATURE_BROWSER_EMULATION\<exe>` | registry | version from HKLM (`cbTp/cbTpApu.cpp:591-606`), value `:612-630` | `setBrowserEmulation` from `cbHk/WinHk.cpp:381`, `ViestiWin/UnitMain.cpp:290`, `cbTp/UnitBrowser.cpp:71`; `EnvData.IEEmul < 9000` changes the UI (`cbHk/TulostusUnit2.cpp:239`, `:1171`) | GUI | 0 | Only registry use; `dbxkilp.ini` only as a dialog default (`cbHk/UnitMySQL.cpp:96`) |
| Emit form menu items | gui form | `cbHk/UnitEmit.cpp:113`, `:122`, `:159-196` | `lukutoisto` (`:3197`), `vahvistaaina` (`:2907`), `kuntoMaali`/`kuntoLahto` (`:187-190`) | HkKisaWin | `lukutoisto=2` (`Hk/HkGlobals.cpp:524`) | Persisted only via `ikkunat.xml`; D7 |

## 3. Environment variables and process inputs

- **Environment variables.** None in product code. The only call is `Tests/fixtures/TpCore.cpp:35` (`TP_SANDBOX_TOKEN`) [verified by grep].
- **Working directory.**
  - It is read at `HkConsole/HkMain.cpp:182`, `JukConsole/laskenta.cpp:104` and `cbHk/WinHk.cpp:189`, and all of the files above are relative to it.
  - The GUI restores it after file dialogs (`SetCurrentDir(FormMain->CurrentDir)`).
  - `cbHk/UnitKilpMaaritys.cpp:222` sets it.
  - HkKisaWin passes `alkulayout` without a directory prefix (`cbHk/WinHk.cpp:368`); ViestiWin adds `GetCurrentDir()` (`ViestiWin/UnitMain.cpp:269`).
- **argv.**
  - It is read at `Hk/HkInit.cpp:567-583` and `Juk/VInit.cpp:655-670`, with `wmain` at `HkConsole/HkMain.cpp:178` and `JukConsole/laskenta.cpp:98`.
  - `CFG=` in argv[1] selects the cfg file; the rest are keys.
- **Exe name.** It is the name of the registry emulation value (`cbHk/WinHk.cpp:1696-1703`, `ViestiWin/UnitMain.cpp:1517-1523`).

## 4. Competition parameters that switch features

| name | read at | effect at | programs | default | meaning |
|---|---|---|---|---|---|
| `Discipline` -> `kilpparam.kilplaji` (HK `SHJUAMNPB`, `Hk/HkXml.cpp:46-47`); per stage `pvparam[].kilplaji` (`:550`) | `Hk/HkXml.cpp:466-471`; `Hk/HkSrj.cpp:119`, `:146` | `N`/`P`: time scaling and point formulas (`Hk/HkInit.cpp:3260`, `:3286-3292`, `:3440-3466`; `Hk/HkDat.cpp:872`, `:1254`; `Hk/HkSrj.cpp:706`); `A`: `maxasakko=5`, `asakkolaji=1` (`Hk/HkInit.cpp:3344`, `:3348-3353`); `S`/`B`: `vainalarajat` (`:3310`); `B`: `Hk/HkSrj.cpp:681`, `Hk/HkXml.cpp:1973`, `cbHk/TulosUnit.cpp:918` | HK | console `S` (`HkConsole/HkMain.cpp:184`); GUI not traced | Sport; about 221 references |
| `Subtype` -> `kilpparam.alalaji` (`RSPMAEDNIOLUFCHWT`, `Hk/HkXml.cpp:48-51`) | `Hk/HkXml.cpp:472-475` | `S`+`A` pair: `kaksibadge=2` (`Hk/HkInit.cpp:3293-3295`); `H` shooting: `cbHk/HkKisaLahtLuet.cpp:1377`, `cbHk/UnitArvonta.cpp:68`; `D` rounds: `cbHk/TulostusUnit2.cpp:888`, `cbHk/TulosUnit.cpp:950`; per stage `P`/`H` -> `asakkolaji=3` (`Hk/HkInit.cpp:3352-3353`) | HK | blank | Subtype |
| relay `Subtype` -> `partio`, `rogaining` (`NSTPRD`, `Juk/VXml.cpp:51-52`) | `Juk/VXml.cpp:549-554` | `:563-574`: `P` -> `partio=2`; `R` -> `partio=2`, `rogaining=true`; otherwise both 0. Used at `Juk/VEmit.cpp:1531`, `:2197`, `:712`, `:3575`; `Juk/VIx.cpp:737`, `:885`, `:767`; `Juk/VTulostus.cpp:3074`, `:2000` | relay | 0/false | Patrol and rogaining; assigned only here [verified] |
| `Badge` -> `badgelaji`, `kaksibadge` | `Hk/HkXml.cpp:501-509`; `Juk/VXml.cpp:488-494` | HK: `ToimintaTila==2 && badgelaji>' ' && emitfl==0` -> `emitfl=1` (or -1 with `vaajat`) (`Hk/HkInit.cpp:3301-3307`); relay `emitfl=-1` (`Juk/VInit.cpp:2761-2762`) | all four | `E` (`HkConsole/HkMain.cpp:187`, `cbHk/WinHk.cpp:201`) | Turns emit reading on without the key; relay -1 meaning unknown |
| `StartGate` -> `pvparam[].hiihtolahto` | `Hk/HkXml.cpp:558-561` | about 76 refs, e.g. `Hk/HkInit.cpp:3626` | HK | 0 | Ski start gate |
| `VariableClass` / `VariableBib` / `IntIdLen` | `Hk/HkXml.cpp:485-496` | 43 / 56 / 9 refs | HK | false/false/0 | Per-stage class and bib; ID format |
| `CombinedResult` -> `yhttlslaji` (`SLPV`) | `Hk/HkXml.cpp:511-515` | 17 refs | HK | blank | Combined result mode |
| `RaceCount` -> `n_pv_akt` | `Hk/HkXml.cpp:477-479` | `Hk/HkInit.cpp:2790`, `:3314-3322`, `:3257`, `:3355` | HK | 1 | Multi-stage |
| `Precision` -> `tarkkuus`, `laika*` | `Hk/HkSrj.cpp:140`, `:155-186`; `Hk/HkXml.cpp:581-616`; `Juk/VXml.cpp:511-538` | field widths | all four | 0 | Time resolution |

## 5. Interactions and console-vs-GUI differences

1. **`AUTOFILE.LST` reading.**
   - The code that reads `AUTOFILE.LST` is inside `#ifdef AUTOFILE`; HK defines `AUTOFILE` through `TCPSIIRTO` (`Hk/HkDef.h:53-54`).
   - No GUI field changes `aflstnm` [verified].
2. **`EMIT_IN.DAT`.** `#ifdef EMITFILE` gates the reader [verified].
3. **GUI-only file features.**
   - `korostus*.lst`, `pistesrj.lst`, `ikkunat.xml`, `Init.cfg` and `KilpailuLuettelo.xml`.
   - The console parses the `IKKUNAT` key (`Hk/HkInit.cpp:641`) but never loads the file.
   - The console asks for `PISTESRJ.LST` interactively (`HkConsole/HkCons3.cpp:1520-1526`) [verified].
4. **HK and relay differ** [verified]:
   - the `korostus` file naming;
   - `vaihevalinta.cfg` and `TLSKERR.LST`, which exist only in HK;
   - the responsive `HTMLALK` test, which only HK has;
   - the badge-driven `emitfl`: 1 in HK, -1 in the relay.
5. **`radat` reading at stage change.**
   - At start, -1 means "try the LST files" (`Hk/HkInit.cpp:3594`).
   - At a stage change it is an error (`Hk/HkIx.cpp:1937`). See D1 [verified].
6. **Enabling keys.** The `vaihevalinta` and `IKKUNAT` keys are what enable the two files (`Hk/HkInit.cpp:3188-3189`; `cbHk/WinHk.cpp:367`) [verified].

## 6. Defect candidates

- **D1 (medium): no LST fallback at stage change.**
  - At start, a missing `RADAT1.XML` falls back to the LST files (`Hk/HkInit.cpp:3594-3601`).
  - At a stage change, -1 prints an error and sets `emitfl=0` (`Hk/HkIx.cpp:1937-1940`).
  - An LST-only competition therefore loses emit reading after the stage switch [verified].
- **D2 (medium): parse error falls back to `KILP.SRJ`.**
  - Any non-zero result from `lueEventXml`, including a parse error, falls back to `KILP.SRJ` (`Hk/HkSrj.cpp:72`, `Juk/vSarjat.cpp:315`).
  - A stale `KILP.SRJ` is then read silently.
  - The "missing" message names the wrong file (`Hk/HkSrj.cpp:81-83`) [verified].
- **D3 (medium MSVC / low others): `fclose(NULL)`.**
  - `Juk/VTulostus.cpp:8628-8631` calls `fclose(hajfile)` when `fopen("HAJONNAT.LST")` returned NULL, which is the normal case.
  - Under MSVC the invalid-parameter handler may end ViestiMaali [inferred]. The path is `!init && !tulprm->monios` and `#ifndef LUENTA` [verified].
- **D4 (medium, GUI): stale `SQLjonoVarm.dat`.** It is replayed with no check that it belongs to the current competition (`ViestiWin/UnitMySQL.cpp:605-623`) [verified].
- **D5 (low): missing closing tags.** With `HTMLALKY.TXT` present and `HTMLLOPY.TXT` absent, no closing tags are written (`Hk/HkTulostus.cpp:6959-6975`) [verified].
- **D6 (low): dead code at `Hk/HkSak.cpp:121`.**
  - `er == 1` never holds, so `paikkafl` stays NULL.
  - If the block were enabled, `fseek(NULL)` would follow (`:123-125`) [verified].
- **D7 (low-medium): Emit menu items out of step with their variables.**
  - `Checked = lukutoisto` turns the int into a bool (`cbHk/UnitEmit.cpp:113`, `:3196-3197`).
  - `VaadiToisto` and `VahvIlmAina` from `ikkunat.xml` change only the menu item, not `lukutoisto` or `vahvistaaina` (`:186`, `:193`); `kuntoMaali` and `kuntoLahto` are set (`:187-190`) [verified].
- **D8 (low): small items** [verified]:
  - the `lueEventXml` message passes `filename` for `%d` (`Hk/HkXml.cpp:1835`);
  - `infile` is not deleted when the file is not open (`Hk/HkXml.cpp:1824-1825`, `cbHk/WinParamXml.cpp:629-630`);
  - `luevaihevalinta` accepts `VAIHE=-5` (`Hk/HkInit.cpp:289-310`, `:3190-3193`);
  - the IE version is taken from its first character, so IE 10 and 11 read as 1 (`cbTp/cbTpApu.cpp:597`);
  - `lue_lainat` ignores the `swscanf` result and leaves `ekdi` uninitialised (`Hk/HkEmit.cpp:1421-1425`).

## 7. Not covered

- **Runs.** Nothing was built or run; the run-time claims are [inferred].
- **Code not traced:**
  - the HK `AUTOFILE` else branch after `Hk/HkTulostus.cpp:7200`;
  - where `korRaja`, `vipLaji` and `korVari` are used;
  - the `PIIRIT.LST` output.
- **Not checked:**
  - which per-window options are logic and which are cosmetic (only the Emit form was checked);
  - `ViestiWin/WinParamXml.cpp`, which was assumed equivalent;
  - the relay `emitfl=-1` and the GUI `kilplaji` default.
- **Not searched:** `include/`, `Tests/` and `RADStudio10/`.
- **Left to part B:** the cfg-writing form check boxes.
