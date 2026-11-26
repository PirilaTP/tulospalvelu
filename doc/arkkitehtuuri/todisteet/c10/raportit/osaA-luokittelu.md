# C-10 osa A / part A: classification, per-product dependency inventory

Agent `build-dependency-analyst` (Sonnet), 2026-11-26. Report saved by the main session from the
agent's hand-back (claims, evidence and labels unchanged; wording and layout condensed). Read-only; PE import tables of the
git-ignored `TPexe/` builds (2026-11-24, console 00:16, GUI 00:57; may differ from HEAD; marked
"PE") read with a parser, nothing run. Paths relative to `TPsource/V52/` unless they start with
`doc/`, `TPexe/` or `.github/`. HM = HkMaali, VM = ViestiMaali (`JukMaali520`), VL = ViestiLuenta
(`JukLuenta520`), HW = HkKisaWin, VW = ViestiWin. Labels B1..B13 are the agent's.

## 1.0 Key facts

| # | Fact | Evidence | Tag |
|---|---|---|---|
| F1 | Console exes import no `odbc32`, `advapi32`, `shell32`, `ole32`, `comdlg32`, `winmm`, `setupapi`, CRT DLL (`/MT`). HM, VM: `kernel32`, `user32`, `gdi32`, `winspool.drv`, `ws2_32`, `winhttp`; VL same without `winhttp`. HW, VW: `advapi32` (5 Reg*), `shell32` (`SHBrowseForFolderW`, `SHGetPathFromIDListW`), `user32`, `gdi32`, `winspool.drv`, `oleaut32` (4 ordinals), `ws2_32` (23), `winhttp` (8), `kernel32`, `borlndmm.dll`, `cc32370mt.dll`, BPLs `rtl370`, `vcl370`, `vclie370`, `vcledge370`, `dbexpress370`, `dbrtl370`, `DbxCommonDriver370`, `DBXMySQLDriver370`, `IndySystem370` | PE | [verified] for those builds |
| F2 | PE OS/subsystem version 6.0 (Vista) in all five; `v120_xp` configurations are not what is built | PE header | [verified] |
| F3 | `_WIN32_WINNT`/`WINVER` defined nowhere; both compilers fall back to 0x0A00 from `sdkddkver.h` (BDS `windows.h:124-126` 0x0500 fallback inert after `:33`) | Windows Kits `shared/sdkddkver.h:226-229`; BDS `include/windows/sdk/sdkddkver.h` | [verified] |
| F4 | classic bcc32 compiles against `Studio\37.0\include\windows\sdk`, not Windows Kits | `RADStudio10/build-logs/HkKisaWin.log` `-I` list | [verified] |
| F5 | GUI link `winhttp.lib import32.lib cp32mti.lib`; `winhttp.lib` found by name in `lib\Win32\release\psdk`; `..\cbHk\winhttp.lib` (`HkKisaWin.cbproj:755`) dead path; `import32.lib` also holds WS2_32 entries | ilink line; psdk listing | [verified]; [inferred] ws2_32 source |

## 1.1 Link-time libraries and system DLLs

Y = used; Y* = only when configured; . = not in product.

| Class | HM | VM | VL | HW | VW | Run-time need | Gate | Evidence |
|---|---|---|---|---|---|---|---|---|
| `tputilv2.lib`, `dbboxm.lib` | Y | Y | Y | . | . | static | – | `HkMaali.vcxproj:164`; `JukMaali520.vcxproj:145`; `JukLuenta520.vcxproj:141` |
| `Tputil-XE.lib`, `DBboxm-XE.lib` | . | . | . | Y | Y | static | – | ilink line; `tuotteet.csv` |
| `ws2_32` | Y | Y | Y | Y | Y | DLL imported at load in all five; sockets only when configured | `com/WinUDP.cpp:21` forces `TCPSIIRTO`, TCP block `:114-963` always compiled | `WinUDP.cpp:21-23`, `:259` |
| `winhttp` | Y | Y | . | Y | Y | DLL imported in 4; request only with `EMITHTTP`+`ETGPRS` | `EMITHTTP` | `gethttp.cpp:19-20` |
| `odbc32`, `odbccp32` | (default list) | Y | Y | . | . | none; no call, no import | – | VM `JukMaali520.vcxproj:105,123,145,167`; VL `JukLuenta520.vcxproj:103,120,141,162` (twice per line); HM via `%(AdditionalDependencies)` [inferred] |
| `kernel32`, `user32` | Y | Y | Y | Y | Y | mand | – | PE |
| `gdi32`, `winspool.drv` | Y | Y | Y | Y | Y | DLL mand; print jobs optional | `GDIPRINTER=7` | `tputilv2/Wrttext.cpp:132,200,210,740` |
| `advapi32` | . | . | . | Y | Y | mand in GUI; browser setup | – | `cbTp/cbTpApu.cpp:579-636` |
| `shell32` | . | . | . | Y | Y | folder picker | – | `cbTp/cbTpApu.cpp:351,353` |
| `comdlg32`, `ole32`, `oleaut32`, `shlwapi` | . | . | . | via VCL | via VCL | – | – | [inferred] |
| `winmm`, `setupapi`, `iphlpapi`, `crypt32`, `wininet` | . | . | . | . | . | none | – | grep; PE |
| `commode.obj` | Y | Y | Y | . | . | static | – | `HkMaali.vcxproj:164` |
| MSVC CRT `/MT` | static | static | static | . | . | none | – | `tuotteet.csv`; PE |
| `cc32370mt.dll` | . | . | . | Y | Y | mand | – | `cp32mti.lib`; PE |

## 1.2 VCL/RTL packages (summary; part E has deployment)

HW, VW only (`UsePackages`, `DynamicRTL`). `.pdi`: HW 13 (`vcledge`, `DBXMySQLDriver`, `dbexpress`, `rtl`, `IndyCore`, `IndyProtocols`, `IndySystem`, `bcbie`, `vclie`, `dsnap`, `DbxCommonDriver`, `dbrtl`, `vcl`), VW 11 (adds `vclx`; no `IndyProtocols`, `bcbie`, `dsnap`); VW exe imports 9 BPLs while its `.pdi` lists `IndyCore` [verified]. `Base_Win32` `PackageImports` (`HkKisaWin.cbproj:183`) names `frx16`, `Rave100VCL`, `TeeDB`, `CodeSiteExpressPkg`, `intrawebdb` — none in `.pdi` or source.

## 1.3 Database, browser, devices, HTTP

| Class | HM | VM | VL | HW | VW | Gate | Detail |
|---|---|---|---|---|---|---|---|
| MySQL/dbExpress | . | . | . | opt | opt | forms, `SQLSTART`, menu | part B; `Juk/vdat.cpp:38-60` dead |
| `TWebBrowser` | . | . | . | Y | Y | form creation | part C; correction 6.6 |
| WinHTTP emiTag | Y* | Y* | . | Y* | Y* | `EMITHTTP`+`ETGPRS` | part C |
| serial/RFID/readers | Y* | Y* | Y* | Y* | Y* | configuration | part D |
| UDP/TCP | Y* | Y* | Y* | Y* | Y* | configuration | `WinUDP.cpp:291,795` bind `INADDR_ANY` |

## 1.4 External files, registry, commands

| Item | Dir. | Products | Gate | Evidence |
|---|---|---|---|---|
| `HKLM\SOFTWARE\Microsoft\Internet Explorer\Version`, `svcVersion` | read | HW, VW (start-up) | – | `cbTp/cbTpApu.cpp:579-601`; `cbHk/WinHk.cpp:381`, `ViestiWin/UnitMain.cpp:290` |
| `HKCU\...\FEATURE_BROWSER_EMULATION\<exe>` | write every start, never removed | HW, VW | – | `cbTp/cbTpApu.cpp:611-628` |
| dbExpress registry key, `dbxdrivers.ini`, `dbxconnections.ini` | read (RTL) | HW, VW opt | MySQL | part B |
| `libmysql.dll` | load | HW, VW opt | MySQL | `Hk/HkGlobals.cpp:638,655`, `Juk/VGlobals.cpp:523` |
| `HkKisa.chm`, `ViestiWin.chm`, HTML Help | open | HW, VW opt | F1/help | `cbHk/WinHk.cpp:188`, `ViestiWin/UnitMain.cpp:167`; `#pragma link "Vcl.HtmlHelpViewer"` `cbHk/WinHk.cpp:89`, `ViestiWin/UnitMain.cpp:72` |
| `POSTSCRI.INI` | read | HM, HW opt | PostScript printing | `Hk/HkInit.cpp:139,197` |
| `system("<autokomento> >NUL")` | exec | HM, HW opt | `AUTOFILE`, `autokomentolaji==1` | `Hk/HkTulostus.cpp:7341-7345` |
| `_wsystem("start /min <autokomento> >NUL 2> cmderrors.txt")` | exec | VM, VL, VW opt | `AUTOFILE` | `Juk/VTulostus.cpp:7325-7329` |
| `system("DATE d.m.y")` | never executed (`as_v_pv` has no caller) | libraries | – | `tputilv2/lue_v_pv.cpp:35-48`; `include/tputil.h:320` |
| default printer, spooler | use | all opt | printing | `tputilv2/Wrttext.cpp:185,200`; `cbHk/TulostusUnit2.cpp:80,120` |

## 1.5 Network endpoints

| Endpoint | Dir. | Products | Gate | Evidence |
|---|---|---|---|---|
| `195.159.103.189:1379` GET `/emitphp/get_stream.php` | out | HM, VM, HW, VW | `EMITHTTP` | `Hk/HkGlobals.cpp:624-625`; `Juk/VGlobals.cpp:506` |
| `255.255.255.255` UDP broadcast (monitor) | out LAN | all | config | `Tp/TpComY.cpp:426` |
| UDP/TCP servers `INADDR_ANY`, configured ports | in | all | config | `com/WinUDP.cpp:291,795,802` |
| TCP client (`gethostbyname`, `inet_addr`) | out | all | config | `com/WinUDP.cpp:309-315,378,756` |
| MySQL server | out | HW, VW | MySQL | part B |
| IOF/XSI namespace strings | none | all | – | `Hk/HkXml_IOF30.cpp:964-965`; `Juk/VXml_IOF30.cpp:950-951` |

## 1.6 Third-party source

| Code | Files | Licence stated | In products | Evidence |
|---|---|---|---|---|
| doctest 2.4.11 | `Tests/doctest.h` | MIT (Viktor Kirilov 2016-2023) | TpTest | `Tests/doctest.h:49-51`; `Tests/doctest-LICENSE.txt:1-3` |
| Mersenne Twister MT19937 (port J. Bedaux 2003) | `Tp/mtrand.h`, `Tp/mtrand.cpp` | BSD 3-clause in header | HW, VW | `Tp/mtrand.h:1-30` |
| Blaise Computing Inc. "Version 6.00 (C) 1983-1989" | `include/bkeybrd.h`, `bscreens.h`, `bstrings.h`, `butil.h`; `tputilv2/STPCVT.cpp`, `STPEXPAN.cpp`, `STPJUST.cpp`, `STPTABFY.cpp`, `STPXLATE.cpp`, `STSCHIND.cpp`, `scclrmsg.cpp`, `sccurset.cpp`, `sccurst.cpp`, `scpclr.cpp`, `scwrite.cpp` (11 compiled), `scbox.cpp` (not built) | no licence text (grep) | all five via libraries | `include/bkeybrd.h:5`; `tputilv2/STPCVT.cpp:46` |
| Microsoft Source Code Samples | `include/Console.h` (1993), `include/size.h` (1993), `tputilv2/consolesize.cpp` (1993-1997) | "All rights reserved ... only intended as a supplement to Microsoft Development Tools" | console + libraries | `include/Console.h:2-9`; `tputilv2/consolesize.cpp:4-7` |
| own GPL header | `Tp/mtrand.cpp:1-14` | project's GPLv3 header | | |

## 2. Feature switches that turn on external dependencies

| Switch | Enables | HM | VM | VL | HW | VW | Evidence |
|---|---|---|---|---|---|---|---|
| `EMITHTTP` | WinHTTP polling; `ETHOST`, `ETGPRS`, `ETSQL` | proj | proj | – | proj | proj | `Hk/HkIV.cpp:3500`; `Juk/VIv.cpp:2369`; `Tp/TpLaitteet.cpp:448`; `Juk/VGlobals.cpp:502` |
| `TCPSIIRTO` | result file over TCP; TCP code | proj | proj | proj | proj | proj | `Hk/HkInit.cpp:1033`; `Hk/HkTulostus.cpp:7328`; **`com/WinUDP.cpp:21-23` defines it itself** |
| `TCPLUKIJA` | TCP/UDP-stream reader connections | proj ×5 | | | | | `Hk/HkInit.cpp:2143,2297,2484`; `Juk/VInit.cpp:1778`; `Tp/TpComY.cpp:1169,2078` |
| `LAJUNEN` | Sirit, Zebra RFID | proj | proj | – | proj | proj | `Tp/TpLaitteet.cpp`; `Tp/SiritReader.cpp:25,79`; `Tp/ZebraReader.cpp:38` |
| `GAZ` | GAZ board | proj | proj | – | **–** | proj | `Hk/HkInit.cpp:109,1376,3675`; `Hk/HkIV.cpp:3102,3641,3918` |
| `AUTOFILE` | automatic output incl. `system()` | proj | proj | proj | **hdr** (`Hk/HkDef.h:53-55` from `TCPSIIRTO`) | proj | `Hk/HkTulostus.cpp:6978,7341`; `Juk/VTulostus.cpp:7030,7325` |
| `VALUKIJA` | intermediate reader, `laheta_ec`, `AUTOVA` (`Juk/VDef.h:36-38`) | proj | proj | – | proj | proj | `Hk/HkCom32.cpp:497,627`; `Juk/VCom.cpp:1505,1612` |
| `EMITAG` | **no `#if` uses it** | – | proj | – | – | proj | `Hk/HkInit.cpp:2334`; `Juk/VInit.cpp:1826`; inert [verified] |
| `EMITFILE` | emit-file reading | proj | proj | unknown | proj | – | part D |
| `SAKKOCOM` | penalty device | proj | – | – | hdr (`Hk/HkDef.h:124-126`) | – | `Hk/HkInit.cpp:120,2132` |
| `GDIPRINTER` | GDI/spooler printing | 7 | 7 | 7 | 7 | 7 | `tputilv2/Wrttext.cpp` |
| `SQLDATA` | direct `mysql_*` | – | – | – | – | – | `Juk/vdat.cpp:38,158`; dead; `:55` would not compile |
| `EI_OLE` | old non-OLE code (TCP-server prototypes `include/wincom.h:103-109`, `TGetHttpForm` `cbHk/WinHk.cpp:1167`, ~20 blocks) | – | – | – | – | – | dead; `openportTCPsrv`, `read_TCPsrv` undefined |
| `TAULU`, `TAULU_2` | scoreboard | – | – | – | – | – | `Hk/HkInit.cpp:105`; `Hk/HkIV.cpp:3623` [inferred] |

## 3. Windows API areas

| Area | Products | Representative | DLL | Notes |
|---|---|---|---|---|
| Winsock | all | `com/WinUDP.cpp:288,291-293,657,802,911,918,1105` | ws2_32 | `WSAStartup(2,...)` (`:259,367,717`); IPv4 only |
| WinHTTP | HM, VM, HW, VW | `com/gethttp.cpp:43,47,54,61,90,134` | winhttp | no newer flags [inferred] |
| Serial | all | `com/winCom.cpp:53,78,96,114-115`; `Tp/TpLaitteet.cpp:2415-2426` | kernel32 | `\\.\COMn` |
| Console buffer API | HM, VM, VL | `tputilv2/initcon.cpp:48-49`; `HkConsole/HkMain.cpp:183`; `JukConsole/laskenta.cpp:106` | kernel32 | legacy console model |
| GDI printing | all | `tputilv2/Wrttext.cpp:210,740,588` | gdi32 | |
| Spooler | all | `tputilv2/Wrttext.cpp:132,200,185`; `cbHk/TulostusUnit2.cpp:80,120` | winspool.drv | `EnumPrinters` only GUI |
| User interface | all | `tputilv2/perr.cpp:50,86` | user32 | |
| Registry | HW, VW | `cbTp/cbTpApu.cpp:591,612,618,626` | advapi32 | |
| Shell | HW, VW | `cbTp/cbTpApu.cpp:351,353` | shell32 | legacy branch |
| Common item dialog | HW, VW | `cbTp/cbTpApu.cpp:322` | via VCL | Vista+ |
| OLE/ActiveX | HW, VW | `cbTp/UnitBrowser.cpp:57-65` | oleaut32, MSHTML (OS) | no `CoInitialize` in source |
| Threads | all | `Hk/HkIV.cpp:831,3351,3582` | CRT | |
| Time | all | `tputilv2/msdaytime.cpp:44`; `Tp/ZebraReader.cpp:883` | kernel32 | |
| Process | all | `Hk/HkTulostus.cpp:7345`; `Juk/VTulostus.cpp:7329` | CRT, cmd.exe | |
| `GetVersionEx` | HW, VW | `cbTp/cbTpApu.cpp:314` | kernel32 | deprecated; manifest declares Win 10 |
| Sound | all | `cbHk/TulosUnit.cpp:671`; `tputilv2/perr.cpp:50` | kernel32, user32 | no winmm |
| HTML Help | HW, VW | `cbHk/WinHk.cpp:89,188` | hhctrl.ocx | [inferred] |

Version binding: only explicit test `cbTpApu_IsOsVistaOrLater()` (`cbTp/cbTpApu.cpp:309-317`), pre-Vista branch `:337-358` legacy; GUI manifest from `$(BDS)\bin\default_app.manifest` declares Vista–10; PE OS 6.0; VCL 13 items and the `api-ms-win-core-winrt-string-l1-1-0.dll` delay import belong to Windows 8+ [inferred]; C++Builder 13 runtime minimum OS [unknown].

## 4. Versions and pins

| Item | Value | Evidence |
|---|---|---|
| MSVC toolset | `v145` all vcxproj | `HkMaali.vcxproj:36,42,49`; `JukMaali520.vcxproj:31-50`; `JukLuenta520.vcxproj:31-50`; `tputilv2.vcxproj:23,29`; `dbboxm.vcxproj:23,29` |
| `v120_xp` | `ReleaseVA200` (`HkMaali.vcxproj:56`), `Demo` (`:63`) | |
| CI | `/p:PlatformToolset=v143` (`build.yml:22,25,28`; `release.yml:30,33,36`), `windows-latest` (`build.yml:13`); TpTest not in CI | |
| local linker | 14.52 | PE |
| `WindowsTargetPlatformVersion` | 10.0 | vcxproj |
| SDK installed | `Windows Kits\10\Include\10.0.26100.0`; CLAUDE.md 10.0.26100.4188 | folder |
| C++Builder | `ProjectVersion 20.1` (`HkKisaWin.cbproj:4`); `bds.exe` 37.0.60542.8024; `bcc32.exe` 7.80.9351.38661; `ilink32.exe` 7.0.9278.38613; `rtl370.bpl`, `vcl370.bpl`, `borlndmm.dll` 37.0.60542.8024 | `VersionInfo` |
| BDS path | `build-gui.ps1:36,72` | |
| program | `Hk/BuildNo.h:17-18` `VERSIOKDI L"5.21"`, `BUILD L"5.22.002/2026-07-31"`; `HkKisaWin.cbproj:255` 5.21.0.2; `ViestiWin.cbproj:307` 5.21.0.1 (another config `5.20.0.25` `:358`); console no version resource [unknown] | |
| doctest | 2.4.11 | `Tests/doctest.h:49-51` |
| Mersenne Twister | no number; 2002/1/26, port 2003/1/1 | `Tp/mtrand.h:1-6` |
| Blaise | "Version 6.00" | `include/bkeybrd.h:5` |

Version identification: VCL/RTL by BDS 37.0 (suffix 370, 37.0.60542.8024); MSVC CRT by toolset; OS DLLs by the OS (no pin); `libmysql.dll`, MySQL server, emiTag server [unknown]; third-party code by file headers.

## 5. Dead code and stale references

Files in no product (from `tiedostot.csv`): `Tp/TpComY32.cpp`, `TpMuot.cpp`, `TpMuot.h`, `TpApuC.c`, `mttest.cpp`; `dbboxm/Dbbox.cpp`, `BOXTEST_F.cpp`, `Boxtest.cpp`, `DBINTF.H`; `tputilv2/UnitTest.cpp`, `UnitTest.h`, `LibTest.cpp`, `TextFileClass-2.cpp`, `Utf8Conv.cpp`, `aakjarjs*.cpp` (8), `globals.cpp`, `haesarja.cpp`, `haesarja_w.cpp`, `kbflush.cpp`, `scbox.cpp`, `testucode.cpp`; `Hk/HkApuC.c`, `HkTV.cpp`; `HkConsole/GetHttp.h` (listed `HkMaali.vcxproj:213`, refers to `ADODB.hpp`); `JukConsole/lasknum5u.cpp`; `ViestiWin/VWinApu.cpp`; `include/box.h`, `boxf.h`, `sitypes.h` (31 files) [inferred].

Dead blocks: `SQLDATA` (`Juk/vdat.cpp:38-60`, `:158-160`); `EI_OLE` (~20); `as_v_pv()` (`tputilv2/lue_v_pv.cpp:35-48`); `TGetHttpForm`/`GetHttp.h`; Linux stubs `com/WinUDP.cpp:149-171` (`__linux__`, 10 more files); pre-Vista picker `cbTp/cbTpApu.cpp:337-358`; commented `GetVersionEx` (`cbHk/TulostusUnit2.cpp:103`, `ViestiWin/TulostusUnit2.cpp:130`, `Hk/HkInit.cpp:3095`, `Juk/VInit.cpp:2555`); `EMITAG` inert.

Stale: ODBC libs (VM, VL); `vc10/lib` (`JukLuenta520.vcxproj:120`, `:162`, not shipped `Release` `:141`); `..\cbHk\winhttp.lib` (`HkKisaWin.cbproj:755`); missing-slash path (`:158,161`); Indy packages; `v120_xp` (`HkMaali.vcxproj:56,63`).

## 6. Corrections to `riippuvuudet.md`

| # | Line | Correction | Evidence |
|---|---|---|---|
| 6.1 | 70 | JukLuenta also links `odbc32`/`odbccp32` and no `winhttp.lib`; exact lines VM `JukMaali520.vcxproj:105,123,145,167`, VL `JukLuenta520.vcxproj:103,120,141,162` (twice per line, plus kernel32…uuid) | [verified] |
| 6.2 | 71 | `..\cbHk\winhttp.lib` dead path; found by name in psdk | build log |
| 6.3 | 75 | `vc10/lib` only in `Debug250` (`:120`) and `Release150os` (`:162`) | [verified] |
| 6.4 | 76 | Indy: `IndySystem370.bpl` is a run-time import of both exes; `HkKisaWin.pdi` lists `IndyCore`, `IndyProtocols`; no `TId*` | PE, `.pdi` |
| 6.5 | 77 | no ODBC import; also VL | PE |
| 6.6 | 126 | no `TEdgeBrowser`; C++Builder 13 `SHDocVw.hpp:32` includes `Vcl.Edge.hpp`; `TWebBrowser.SelectedEngine` default 0 = `IEOnly` (`SHDocVw.hpp:686,927`), not set anywhere; `vcledge370.bpl` is a start-up dependency | headers, DFM, PE |
| 6.7 | 133 | linker 14.52; `v120_xp` in two configs | |
| 6.8 | 135 | bcc32 uses bundled `include\windows\sdk`; installed folder `10.0.26100.0`; how the SDK mismatch arises [unknown] | log |
| 6.9 | 85-90 | add Blaise and Microsoft samples; `mtrand.cpp` GPL header is the project's own | 1.6 |
| 6.10 | 124 | HTTP gated by `EMITHTTP`, none in VL | |
| 6.11 | 118 | `dbxkilp.ini` only in commented code | part B |
| 6.12 | 66-83 | JukMaali/JukLuenta are project names; programs ViestiMaali/ViestiLuenta | |

## 7. Candidates (agent labels)

| ID | Sev. | Mechanism | Evidence | Products | Consequence | Conf. |
|---|---|---|---|---|---|---|
| B1 | medium | provenance/licence of Blaise (16 files) and Microsoft sample (3 files) code in a GPLv3 repo, no licence text | `include/bkeybrd.h:5`; `tputilv2/STPCVT.cpp:46`; `include/Console.h:2-9`; `tputilv2/consolesize.cpp:4-7` | all | redistribution terms unclear; owner question | [verified] notices |
| B2 | medium | project defines do not control some switches: `com/WinUDP.cpp:21-23` defines `TCPSIIRTO`; `Hk/HkDef.h:53-55` defines `AUTOFILE`; `EMITAG` inert; `EI_OLE` misnamed | as listed | all | removing a define does not drop the dependency | [verified] |
| B3 | low | `vcledge370.bpl` start-up dependency without Edge use (`SHDocVw.hpp:32`) | `TPexe/Hk/V521/HkKisaWin.pdi:1`; PE | HW, VW | missing BPL blocks start-up | [verified] |
| B4 | low | unused ODBC libs, Indy packages, long PackageImports | 1.1, 1.2 | VM, VL, HW, VW | larger build surface | [verified] |
| B5 | low | HK `AUTOFILE` command via synchronous `system()` while the print thread holds `autotul_CriticalSection`; relay `start /min`; command from config (201 chars) | `Hk/HkTulostus.cpp:7341-7346,7349`; `Juk/VTulostus.cpp:7325-7329`; `Hk/HkInit.cpp:1023`; `Juk/VInit.cpp:940` | HM, HW blocking | hanging command stalls automatic output; by design anyone editing config can run commands | [inferred] |
| B6 | low | dead `as_v_pv()` `system("DATE ...")` | `tputilv2/lue_v_pv.cpp:35-48` | libraries | dormant command path | [verified] |
| B7 | low | `GetVersionEx` + XP-era picker | `cbTp/cbTpApu.cpp:309-358` | GUI | unreachable | [verified] |
| B8 | low | deprecated Winsock, IPv4 only (`gethostbyname`, `inet_addr`) | `com/WinUDP.cpp:309,315,378,756` | all | no IPv6; blocking resolver | [verified] |
| B9 | low | Win32-only; `HANDLE` cast to `int` | `com/WinUDP.cpp` ~`:187-189` | all | 64-bit port needs work | [inferred] |
| B10 | low | stale project paths | 5 | HW, VL, HM | confusion | [verified] |
| B11 | low | `.chm` help files absent from repo, release and collector; HelpContext in 8 DFM lines | `cbHk/WinHk.cpp:188`; `ViestiWin/UnitMain.cpp:167` | GUI | F1 does nothing or errors | [verified] absence |
| B12 | low | `osaA/README.md` lists only 4 of 31 unbuilt files | 5 | – | under-reports dead code | [inferred] |
| B13 | low | UDP/TCP servers bind `INADDR_ANY` without filter | `com/WinUDP.cpp:291,795` | all | any reachable host can feed input | [verified] mechanism |

Open questions: (1) is VW `.pdi` (`IndyCore`) link-time or design-time; (2) does a missing package in `PackageImports` break `bds.exe -b`; (3) how ilink gets `ws2_32` (import32 vs psdk); (4) is `EMITFILE` defined in VL (likely not); (5) minimum Windows for C++Builder 13 runtime; (6) licence terms of Blaise/Microsoft files (owner); (7) console TUI under Windows Terminal.
