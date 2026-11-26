# C-10 osa E / part E: runtime packages, runtime libraries, deployment

Agent `build-dependency-analyst` (Sonnet), 2026-11-26. Report saved by the main session from the
agent's hand-back (claims, evidence and labels unchanged; wording and layout condensed). Files only read; nothing built or run, no
network. PE import tables read with a read-only Python parser (copies in `../osaE/`). Labels
N-1..N-11 are the agent's; register IDs are assigned in `riippuvuudet.md`.

Binaries read (gitignored, 2026-11-24 build, may not match the current source): `TPexe/Hk/V521/HkKisaWin.exe` (5.21.0.2), `TPexe/Juk/V521/ViestiWin.exe` (5.21.0.1), `TPexe/Hk/V521/HkMaali.exe`, `TPexe/Juk/V52/ViestiMaali.exe`, `TPexe/Juk/V52/ViestiLuenta.exe`; logs `RADStudio10/build-logs/{HkKisaWin,ViestiWin}.log` (full bcc32/ilink32 lines). Installed Studio 37.0 `radstudio_deploy_en.htm` "Delphi 13.1 and C++Builder 13.1"; `rtl370.bpl`, `vcl370.bpl`, `borlndmm.dll`, `dbxmys.dll` FileVersion 37.0.60542.8024 [verified].

## 1. GUI runtime set

Shipped config both products: `DynamicRTL`, `UsePackages`, `Multithreaded`, `UsingDelphiRTL` true; Release `LinkPackageImports` `HkKisaWin.cbproj:265`, `ViestiWin.cbproj:302`; link line `memmgr.lib sysinit.obj` (`build-logs/HkKisaWin.log:1556-1557`) [verified].

| Module | HkKisaWin | ViestiWin | Origin |
|---|---|---|---|
| `rtl370`, `vcl370`, `dbrtl370`, `dbexpress370`, `DbxCommonDriver370`, `DBXMySQLDriver370`, `vclie370`, `vcledge370`, `IndySystem370` (.bpl) | yes | yes | Embarcadero |
| `BORLNDMM.DLL` (`GetAllocMemCount`) | yes | yes | Embarcadero |
| `CC32370MT.DLL` | yes | yes | classic bcc32 dynamic RTL |
| `WINHTTP.DLL`, `WS2_32.DLL`, `WINSPOOL.DRV`, kernel32/user32/gdi32/advapi32/shell32/oleaut32, `API-MS-WIN-CORE-WINRT-STRING-L1-1-0.DLL` | yes | yes | OS |

Linked but not imported: `dsnap`, `bcbie`, `IndyCore`, `IndyProtocols` (HkKisaWin); `IndyCore`, `vclx` (ViestiWin) [verified].

Collector (re-implemented in Python, script not run): searches `bin`, `Redist\win32`, `SysWOW64` (`*.bpl`); result = exes + the 11 modules above, same for both, all in `Redist/win32.redistlist`; no Embarcadero module in the closure delay-loads anything non-OS; `CC32370MT.DLL` imports only kernel32/user32 (no VC++ redistributable); suffix `370` taken from imports [verified].

| Loaded at run time, not in ZIP | Loaded by | Needed for | Evidence |
|---|---|---|---|
| `dbxmys.dll` (Studio `bin`, `win32.redistlist:57`) | `LoadLibrary(LibraryName)` `Data.DBXDynalink.pas:933`; default `Data.DbxMySql.pas:98` | SQL forms (`DriverName='MySQL'` in `cbHk/UnitMySQL.dfm`, `cbTp/UnitEmitSQL.dfm`, `ViestiWin/UnitMySQL.dfm`) | [verified] source; [inferred] ZIP content |
| `libmysql.dll` 32-bit | DBX vendor lib `Data.DbxMySql.pas:100`; code sets only `vendorlibwin64` (`cbHk/UnitMySQL.cpp:120`, `cbTp/UnitEmitSQL.cpp:134`; `Hk/HkGlobals.cpp:638`, `Juk/VGlobals.cpp:523`) | same | not in Studio tree or repo [verified] |
| `midas.dll` | – | not needed (no TClientDataSet/TDataSetProvider) | [verified] |
| MSHTML/IE (`shdocvw`, `ieframe`, `mshtml`) | COM via `TWebBrowser` | browser/HTML edit forms | OS; names [inferred] |
| `hhctrl.ocx` | `#pragma link "Vcl.HtmlHelpViewer"` (`cbHk/WinHk.cpp:89`, `ViestiWin/UnitMain.cpp:72`) | help | OS [inferred] |
| `HkKisa.chm`, `ViestiWin.chm` | `Application->HelpFile` (`cbHk/WinHk.cpp:188`, `ViestiWin/UnitMain.cpp:167`) | F1 | not in repo or ZIP [verified] |
| `dbxdrivers.ini`, `dbxconnections.ini` | – | not required: exe dir → registry `\Software\Embarcadero\BDS\37.0\dbExpress` (`Data.DBXCommon.pas:214-216`, `:7118-7150`) → memory factory | [inferred] |

No source `LoadLibrary`/`GetProcAddress`; `osumat.csv` 0 `dynload` rows [verified]. On a developer machine `dbxmys.dll` is found through Studio `bin` on PATH; clean-machine failure [unknown].

## 2. Package list hygiene

| Package | HkKisaWin | ViestiWin | Used? | Evidence |
|---|---|---|---|---|
| rtl, vcl, dbrtl, dbexpress, DbxCommonDriver, DBXMySQLDriver, vclie | link+import | link+import | yes | includes, dfm, imports |
| vcledge | not listed | not listed | pulled in by linker (Vcl.Edge init/fini), no source use | [verified] imports; reason [unknown] |
| IndySystem | link+import | link+import | no Indy code; exes import it (`@Idglobal@initialization`, `@Idwinsock2@initialization`) | [verified]; reason [unknown] |
| IndyCore | linked, not imported | same | no (`ViestiWin/UnitCfg.h:32-37` includes `IdFTP`, unused) | [verified] |
| IndyProtocols | LinkPackageImports | `LinkPackageStatics IndyProtocols.lib` (`ViestiWin.cbproj:293`) | no | [verified] |
| dsnap, bcbie | link | PackageImports only | no | not imported |
| vclx | – | link (`:302`) | no | not imported |
| bcboffice2k, bdertl, inetdbbde, adortl, tee*, ibxpress | – | `ViestiWin.cbproj:163` PackageImports only | no | never on link line |

Real link lists = `LinkPackageImports` exactly; `PackageImports` unused at link [verified].

Stale: XE-era `PackageImports` (FastReport, Rave, IntraWeb, CodeSite, FMX) `HkKisaWin.cbproj:154,183,192`; malformed `..\..\..TPexe\Lib\V521\debug_build\` (`HkKisaWin.cbproj:158,161`; `HkKisaWin.log:1547`); `debug_build` libs before `release_build` on the Release link line (`HkKisaWin.cbproj:344-355`; `ViestiWin.cbproj:367-376`; folder absent now); `winhttp.lib`: `HkKisaWin.cbproj:755` `..\cbHk\winhttp.lib` (`IgnorePath=true`, file absent), `ViestiWin.cbproj:634` bare; logs pass bare `winhttp.lib import32.lib cp32mti.lib`, resolved from `Studio\37.0\lib\win32\release\psdk\winhttp.lib` via `-L` [verified]; `..\COMM`, `\BUILDER\libXE5\release_build` (Demo), `TURBOC`, CBuilder6 history (`ViestiWin.cbproj:152-186,203,328,1129-1202`); generic DeployFile lists, `Debugger_CWD` (`HkKisaWin.cbproj:1280-1301`); `$(BDS)\source\data\dbx` on include/lib paths.

## 3. Memory manager (Q91, static)

- Both GUI exes import `BORLNDMM.DLL` and link `memmgr.lib` [verified].
- `source/cpprtl/Source/memory/hrdir_b.c:122-135`: `__CRTL_MEM_CheckBorMem` (RTL DLL build skips the UseBorMM test) points the heap redirector at `borlndmm.dll` `SysGetMem`/`SysFreeMem`/`SysReallocMem`/`SysAllocMem` when the module is loaded [verified source; inferred `cc32370mt.dll` built that way].
- `source/cpprtl/Source/vcl/crtlvcl.cpp:200` `__CRTL_VCL_Sharemem()` hooks the Pascal memory manager to `borlndmm.dll` (`vclshmem.cpp:44-65`) [verified source].
- `del.cpp:19-22` `operator delete` = `free()` via `_heap_redirector` [verified].
- → `new`/`delete`/`malloc`/`free`/VCL share `borlndmm.dll` (FastMM Embarcadero Edition, `source/rtl/sys/getmem.inc:234`) [inferred, strong].
- Double free (H-149): `_bormm_stub_FreeMem` is void, return discarded (`hrdir_bh.c:27-31`) [verified]; `SysFreeMem` Pascal variant returns −1 for an already freed small block without writing (`getmem.inc:2954-2960`); asm variant not read [inferred same]. Prediction: second `free` silent in ViestiWin (no `0xC0000374`). The use-after-free (`htmlloppu` writes through freed `PRFILE`; block may be reallocated meanwhile) can still corrupt the heap [inferred].
- Unknown without a run: shipped `borlndmm.dll` = source, asm path, `rtl370.bpl` hook, PRFILE size class and reuse.

## 4. Q92: `_wfopen_s` share mode (bcc32 RTL)

`source/cpprtl/Source/io/fopen_s.c:27-41` → `__tfopen_helper(..., _O_SECURE)`; `fopen.c:301-308`, `:178` → `___topen(filename, oflag|shflag, mode)`; `__open.c:238-262` (`if (oflag & _O_SECURE)` `:255-262`; `_O_SECURE` = 0x04 `include/windows/crtl/fcntl.h:87`):

| Mode | Access | dwShareMode |
|---|---|---|
| write/update (`"wb"`, `"ab"`, `"r+"`) | GENERIC_WRITE or RW | 0 (exclusive) |
| read (`"r"`) | GENERIC_READ | FILE_SHARE_READ |

`cc32370mt.dll` exports `__wfopen_s`; both exes contain `_wfopen_s` [verified]. → `tputilv2/TextFileClass.cpp:305` opens write streams exclusively in bcc32 as in MSVC (Q92); shipped DLL = source [inferred].

## 5. MSVC programs

| Item | Result |
|---|---|
| CRT | `/MT` all five projects; exes import only `WS2_32`, `WINHTTP` (HkMaali, ViestiMaali), `KERNEL32`, `USER32`, `GDI32`, `WINSPOOL.DRV` [verified] |
| ODBC | `odbc32`/`odbccp32` on the line for ViestiMaali and ViestiLuenta; no ODBC import [verified] |
| Toolset | all `v145`; exes linked by 14.52; installed MSVC 14.52.37025 + v145 only (BuildTools 18); CI `/p:PlatformToolset=v143` on `windows-latest` (availability [unknown]) |
| v120_xp | `ReleaseVA200`, `Demo` (`HkMaali.vcxproj:51-63`; `HkMaali520.sln:21-26`); not installed, not in CI |
| `commode.obj` | all three console programs; present in 14.52 |
| JukLuenta `vc10` | `Debug250`, `Release150os` link `..\..\..\..\vc10\lib\...` (`JukLuenta520.vcxproj:120`, `:162`); `vc10/lib` absent; not in CI |
| Target | no `_WIN32_WINNT`/`WINVER`/`NTDDI_VERSION` anywhere; `WindowsTargetPlatformVersion=10.0` (`HkMaali.vcxproj:29`); exes OS/subsystem 6.0, console; ASLR, NX, TS-aware; no `LARGE_ADDRESS_AWARE` [verified] |
| TpTest | v145, /MT, not in CI |

## 6. Versions and pins

| Pin | Where | Upgrade requires |
|---|---|---|
| C++Builder 13 = Studio 37.0 | `collect-gui-runtime.ps1:43`, `build-gui.ps1:72`; cbproj `$(BDS)`; registry key `BDS\37.0\dbExpress` compiled into `Data.DBXCommon.pas:214` | new BPL suffix (scripts adapt via imports); runner with the new Studio; re-collect all Embarcadero files |
| `ProjectVersion` 20.1 | all four cbproj; Studio templates 20.0 | IDE upgrades in memory; `build-gui.ps1` closes without saving |
| classic bcc32 | `bcc32.exe ... -tR -tM -tU -tW -K -a8 -bi-` (`ViestiWin.log:12-13`) → `cc32370mt.dll` | Clang compiler → `cc32c370mt.dll`, `libc++-370.dll`, `libunwind-370.dll`; behaviour differences of `kaantajaerot.md` 3, 7 |
| Windows SDK limit | CLAUDE.md; Win32 bcc32/ilink32 uses only Studio include/lib and `psdk` (logs) → the limit comes from the IDE platform-SDK setup [verified paths; cause unknown] | keep SDK pinned on the runner |
| static linking | `ViestiWin.cbproj:253` Debug `DynamicRTL=false` | no BPL collection but memory manager/RTL behaviour changes; re-measure findings |
| MSVC | v145 local, v143 CI | `windows-latest` must provide v143 (not checked) |
| Runners | console/package `windows-latest`; gui `[self-hosted, Windows, X64]` Community Edition, forks excluded, 90 min; docs `ubuntu-latest`, Python 3.12, `mkdocs>=1.6`, `mkdocs-material>=9.5` unpinned | single self-hosted machine carries the licence |
| Actions | `checkout@v4`, `setup-msbuild@v2`, `upload-artifact@v7`, `download-artifact@v7`, `setup-python@v5`, `upload-pages-artifact@v3`, `deploy-pages@v4` | existence of `@v7`, tag vs SHA pinning [unknown, no network] |

## 7. Deployment prerequisites

| Product | Mandatory | Optional |
|---|---|---|
| HkKisaWin, ViestiWin | exe + 11 collected BPL/DLL in the same folder; Windows WinHTTP, Winsock, spooler | `dbxmys.dll` + 32-bit `libmysql.dll` (SQL); IE engine (browser forms); `hhctrl.ocx` + `.chm` (help); printer driver. `FEATURE_BROWSER_EMULATION` written to HKCU by the program, never removed (`cbTp/cbTpApu.cpp:579-636`) |
| HkMaali, ViestiMaali, ViestiLuenta | exe only (`/MT`) | WinHTTP for HkMaali, ViestiMaali (`EMITHTTP`) |

Config/data files are created by the user; none shipped. `release.yml` README says the BPL/DLL files are "the runtime files the GUI programs need" — true for static imports only.

## 8. T-005

ODBC: JukMaali520 and JukLuenta520 link `odbc32.lib;odbccp32.lib`; no ODBC calls; no ODBC imports → removal safe. Indy: no `TId*`; only unused `IdFTP` includes `ViestiWin/UnitCfg.h:32-37`. `IndyProtocols`/`IndyCore` very likely safe to drop (not imported). `IndySystem` imported by both exes with unit initialisations (`Idglobal`, `Idwinsock2`, ...), reason unknown → needs a link trial.

## 9. Candidates (agent labels)

| # | Sev | Mechanism | Evidence | Products | Consequence | Conf. |
|---|---|---|---|---|---|---|
| N-1 | medium | `dbxmys.dll` loaded by LoadLibrary, not collected | `Data.DBXDynalink.pas:933`; `Data.DbxMySql.pas:98`; `win32.redistlist:57` | GUI | SQL fails on a machine without Studio | [verified] mechanism; [inferred] failure |
| N-2 | medium | 32-bit `libmysql.dll` neither shipped nor documented as prerequisite (README) | `Data.DbxMySql.pas:100` | GUI | SQL fails even with N-1 fixed | [verified] absence |
| N-3 | low | `.chm` help files absent; `HelpKeyword` called (`cbHk/WinHk.cpp:1417,1423`) | `git ls-files` | GUI | F1 does nothing or errors | [verified] absence |
| N-4 | low-medium | `debug_build` libs before `release_build`; malformed path | logs; cbproj lines | GUI | Release exe could link a stale debug lib | [inferred] hazard |
| N-5 | medium (process) | collect script and CI never run the packaged exe; missing-module check limited | `collect-gui-runtime.ps1`; `release.yml` | GUI | gaps found by users | [verified] |
| N-6 | low | `IndySystem370.bpl` imported without Indy use | imports | GUI | extra BPL, unused start-up code | [verified] fact |
| N-7 | low | stale PackageImports and paths | section 2 | GUI, JukLuenta | confusion, IDE prompts | [verified] |
| N-8 | low | `Debug250`/`Release150os` need `vc10\lib`; `ReleaseVA200`/`Demo` need v120_xp | sln, vcxproj | ViestiLuenta, HkMaali | unbuildable configurations | [verified] |
| N-9 | low | `TWebBrowser`/emulation key depend on OS IE engine; `vcledge370.bpl` already linked | dfm; `cbTp/cbTpApu.cpp:579-636` | GUI | legacy component | [unknown] |
| N-10 | low | no `LARGE_ADDRESS_AWARE` (characteristics 0x30e GUI, 0x102 console) | PE headers | all | 2 GB address space | [verified] |
| N-11 | info | shared `borlndmm.dll` → H-149 double free silent in ViestiWin | `hrdir_bh.c`, `getmem.inc:2954-2960` | ViestiWin | Q91 predicted; use-after-free remains | [inferred] |

Open question candidates: (1) Q91 run; (2) Q92 run against shipped `cc32370mt.dll` (optional); (3) does SQL connect fail from a clean folder, which `libmysql.dll` is needed; (4) can `IndySystem` be removed; what does `Idwinsock2` init do; (5) do the GUI exes start from the collected folder on a machine without Studio; (6) does `windows-latest` provide v143, do `upload/download-artifact@v7` exist.
