# C-13 part F: test material inventory (osaF-testausaineisto)

Compiled 2026-11-26 for batch C-13 part F (regression test plan). Read-only inventory: no product, probe or
test program was built or run for this file; nothing outside this file was written. Repo root `C:/dev/tulospalvelu`.
Path prefixes: `T/` = `TPsource/V52/Tests/`, `VT/` = `TPsource/V52/VS/Tests/`, `E/` = `doc/arkkitehtuuri/todisteet/`,
`D/` = `doc/arkkitehtuuri/`. Labels: [verified] = read in a file during this compilation, [inferred] = deduced.

Run-state vocabulary: RUN(date; result; where the raw result is stored) or PLANNED (never run).
Test-type codes used in sections 2-6: **UM** automated unit/harness test (MSVC); **UB** automated bcc32 test;
**INT** integration (two-process loopback / multi-workstation network); **GUI** GUI manual or scripted; **DEV** device
(device named); **SQL** local MySQL; **REL** release package on a clean machine; **ACC** manual acceptance (shipped program run, field/owner data).

Sources read: `D/testaus.md` (C-11, ch. 1-20), `D/ominaisuusvalitsimet.md` 9-13, `D/seuranta.md` follow-up lists, the batch reports
(`saikeet-ja-lukot.md` 13.10-13.11, `globaali-tila.md` 11-12, `virheenkasittely.md` 12-13.5, `riippuvuudet.md` 17 and 19.7,
`muistiturvallisuus.md` 15, `kaantajaerot.md` 13.4-13.7, `gui-liiketoimintalogiikka.md` 10 and 13.2),
`E/c13/osaA0/{havainnot,kysymykset}.csv`, `T/`, `VT/`, `.github/workflows/{build,release}.yml`, `RADStudio10/*.ps1`.



## 0. Summary

- Assets: 10 committed harness/test items (A1-A10) and 7 evidence/probe groups (B1-B7), 47 manifest entries (24 defect, 19 control, 4 info), 46 TpTest cases, 46+39 Python self-tests.
- Regression-ready harness tests (detect a fix as is, XFAIL to XPASS): H-001, H-002, H-007, H-008, H-022, H-023, H-024, H-025, H-026, H-027, H-031, H-032 (if fixed in `addseur`), H-040, H-041 (b2, b4 if the buffer is enlarged), H-042.
  Probe-only (need migration into `T/core/`): H-090, H-091, H-093, H-094, H-128, H-129, H-145, H-146, H-147, H-149 (a, b, d), H-150, H-151. Not usable: Release Q85 crash cases, black-box H-041 run, C-5 probes (except H-079 after wrapping).
- Automation blockers: no test runs in CI; `run.sh` broken (missing backslash on line 14, missing `-I Hk` on line 11); test projects hard-code v145 while CI uses v143 (Q35); x86 ASan on `windows-latest` unknown (Q35);
  XPASS is never accepted by the runner (no regression mode); doctest does not compile on bcc32, so no automated bcc32 test; black-box layer needs admin firewall rules, debugger and interactive console (Q40); push ban.
- Open obligations: 80 rows (UM 23, UB 8, INT 10, GUI 15, DEV 5, SQL 3, REL 1, ACC 15).
- P1/P2 without a reproducing asset: 15 of 22 (UM 9, GUI 5, SQL 1); see section 6.
## 1. Test asset inventory

### 1.1 Committed test projects, harness, fixtures, runners (inside the repository)

All committed [verified: `git ls-files Tests VS/Tests`; `git status --short TPsource` clean]. Last test-tree commit `c6205d2`.
Production code unchanged since `fc03b67` (testaus.md 14-15, 20.7).


| # | Path | Contents | Compiler / configuration | Documented build and run | Build output location | State | Production sources linked unchanged |
|---|---|---|---|---|---|---|---|
| A1 | `VT/TpTest.sln`, `VT/TpTest.vcxproj`; `T/DoctestMain.cpp`, `T/HkEnnTavTest.cpp` (15 TEST_CASE), `T/VRinnakkaisOsuusTest.cpp` (31), `T/WideString.h`, `T/doctest.h` (2.4.11) | doctest unit tests of pure logic, 46 cases | MSVC v145 hard-coded (`VT/TpTest.vcxproj:23,29`); Debug (`:60`) and Release (`:77`); `WIN32;_CRT_SECURE_NO_DEPRECATE;_CONSOLE;_DEBUG or NDEBUG`; include dirs Hk;Juk;Tp;include;Tests; no libs, no `/J`, no feature defines | CLAUDE.md: build `TpTest.sln`, run `TpTest.exe`; `--reporters=junit` works (testaus.md:62) | `VT/Release/TpTest.exe`, `.pdb`, `VT/TpTest/` intermediates: inside the repo tree, ignored (root `.gitignore:4` `Release`) | works. RUN(2026-11-24; 46/46 cases, 94 assertions; no raw result file stored, only text in testaus.md:62 and seuranta.md:320). exe mtime 2026-11-24 01:47; not built or run since; not in CI | `Hk/HkEnnTav.cpp`, `Juk/VRinnakkaisOsuus.cpp`, `tputilv2/aikatowst_s.cpp`, `tputilv2/putint_r.cpp` (`VT/TpTest.vcxproj:87-93`) |
| A2 | `T/VRinnakkaisOsuusTestit.xlsx` | spreadsheet beside the relay parallel-leg tests; contents not read [unknown] | n/a | n/a | n/a | not a runnable asset | none |
| A3 | `T/run.sh` | g++ build of the same unit tests | g++ (`CXX` default g++, `-Wall`) | `./Tests/run.sh` from `TPsource/V52` | `./TpTest` in cwd | BROKEN (3.1); never run in the audit | would link the TpTest production units |
| A4 | `VT/TpCore.sln`, `VT/TpCoreHk.vcxproj`, `VT/TpCoreJuk.vcxproj`, `VT/.gitignore` | two doctest executables (HK core, relay core); manifest programs TpCoreHk, TpCoreJuk (Release), TpCoreHkAsan, TpCoreJukAsan | MSVC v145 (`VT/TpCoreHk.vcxproj:38,46`), Win32; `Release` and `Asan` (`VT/TpCore.sln:11-12`); product Release flags (`/J /GL /MT /O2 /GS`) and product defines (HkMaali Release `N_PV 5`, `VALUKU 250`; JukMaali Release `MAXOSUUSLUKU 30`, `VAIKALUKU 12`); Asan = Release + `EnableASAN`, `/Zi`, post-build copy of `clang_rt.asan_dynamic-i386.dll` from `$(VCToolsInstallDir)` (`VT/TpCoreHk.vcxproj:114-116`) | testaus.md 13.6: msbuild `VS/Libs/tputilv2.sln` Release/Win32 first, then msbuild `VS/Tests/TpCore.sln` with Configuration Release or Asan; link about 2 min (`/GL`) | `VT/Release/`, `VT/Asan/` (ignored) and the libs `TPexe/lib/V52/VS/Release/tputilv2.lib`, `dbboxm.lib` (`TPexe` ignored, `.gitignore:1`): inside the repo tree, never committed | works. RUN(2026-11-24 22:18; PASS 23, XFAIL 24, FAIL 0, XPASS 0; `E/c11/vaihe4/core_all/results.json`, `results.md`, 47 logs). Binaries present locally (Release Hk 22:02, Juk 19:36; Asan Hk 22:02, Juk 19:36) [verified mtimes]; not rebuilt since | HkMaali Release source list minus `HkConsole/HkMain.cpp`; JukMaali Release list minus `JukConsole/laskenta.cpp`; `Juk/VTulostus.cpp` and `Juk/vdat.cpp` by `#include` from test files; `tputilv2.lib`, `dbboxm.lib`; `WRITEERR.obj` and `perr.obj` members replaced by recording stubs (testaus.md 13.1) |
| A5 | `T/core/*.cpp` (12 files, 44 TEST_CASE incl. `tp_build_info`), `T/core/manifest.json` (47 entries: 24 defect, 19 control, 4 info) | tests and the test list (program, case, kind, finding, mechanism, expected, data, timeout, termination) | as A4 | run by A6 | as A4 | as A4 | as A4 |
| A6 | `T/run_core_tests.py` (runner), `T/test_run_core_tests.py` (46 self-tests) | one process and one temp dir per test; timeout and process-tree kill; PASS/XFAIL/XPASS/FAIL/SKIP; exit 0, 1 (FAIL) or 3 (XPASS without FAIL) (`run_core_tests.py:41,47`); writes `Tests/results/<time>/results.json` and `results.md` (ignored, `T/.gitignore`) | Python 3.13 stdlib | `python -I TPsource/V52/Tests/test_run_core_tests.py`; `python -I TPsource/V52/Tests/run_core_tests.py [--out DIR] [--filter H-040] [--keep] [--allow-missing]` | `Tests/results/` ignored | works. RUN(2026-11-24; 46 self-tests OK, testaus.md 15.2; raw not stored) | none |
| A7 | `T/fixtures/{TpCore,HkFixture,VFixture}.{h,cpp}`, `TpCrash.cpp`, `TpNet.cpp` | sandbox check, TPRESULT protocol, lock probe, thread runner, UI stubs; HK and relay start-up parts; crash evidence (vectored handler, DbgHelp, ASan callback); 127.0.0.1-only network helper | linked into A4 | n/a | n/a | works (used by the 2026-11-24 run) | uses production globals unchanged |
| A8 | `T/data/juk/h042_in_limit.xml`, `h042_over_limit.xml`, `h002_{six,seven,nine}_fields.srj`; `T/data/hk/run1_kilpsrj.xml`, `h041_v220.xml`, `h041_v221.xml` | synthetic ASCII fixtures (sha256 in `results.json`) | n/a | copied into the sandbox via manifest `data` | n/a | works | n/a |
| A9 | `T/blackbox/run_hkmaali.py`, `hkdebug.py`, `verify_log.py`, `isolation_probe.c`, `console_probe.c`, `crash_probe.c`, `build_probes.cmd`, `scripts/h041_v220.json`, `scripts/h041_v221.json`, `test_run_hkmaali.py` (39 tests) | black-box driver for the SHIPPED `HkMaali.exe`: firewall isolation, write-restricted token, debugger mode, key scripts, drop-log verifier | driver: Python ctypes; probes MSVC x86 (`cl /O2 /MT`; `crash_probe` also `/GS /Zi`) | `T/blackbox/build_probes.cmd OUTDIR` (outside repo); admin-created firewall rules `TP C11 HkMaali block in/out`; run commands testaus.md 19.7 | probes and the exe copy were outside the repo (since removed, testaus.md 20.6); binaries not archived (`E/c11/BINARIES.sha256`) | works, run once. RUN(2026-11-24: run 0, run 1, H-041 220 and 221; 39 offline self-tests passed per testaus.md 19.8-19.10). Firewall rules removal prepared, user action open (testaus.md 20.6) | none linked; drives `TPexe/Hk/V521/HkMaali.exe` (sha256 `61034e5d...7d297d`) |
| A10 | `T/.gitignore`, `VT/.gitignore` | ignore results, `__pycache__`, `Asan/` | n/a | n/a | n/a | `T/__pycache__` and `T/blackbox/__pycache__` exist locally, ignored | n/a |

Test-like code that is NOT a test asset: `dbboxm/Boxtest.cpp` (interactive, in no project), `Tp/mttest.cpp` (demo), `tputilv2/LibTest.cpp` (VCL, manual);
built-in switches `TESTAUS`, `COMTESTI`, relay Ctrl-T `TESTIAJAT` (testaus.md:69-70) [verified from the doc, not re-read in source].


### 1.2 Isolated probe programs kept as evidence (not part of any repo project)

C-6, C-8 and C-9 probes are generated by `tee_projektit.py` as a COPY of `VT/TpCoreHk.vcxproj` (or `TpCoreJuk.vcxproj`) with the probe file
substituted, are built OUTSIDE the repo (`rakenna.cmd OUTDIR`, hard-coded `C:/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/.../vcvars32.bat`)
and run by an own `aja.py` (sandbox plus timeout as in `run_core_tests.py`, but no manifest and no XFAIL/XPASS classification: it records exit code and output only).
They need prebuilt `tputilv2.lib` and `dbboxm.lib`. Sources and results are committed under `E/`; binaries are not.

| # | Path | Contents | Compiler / configuration | Documented build and run | State | Production sources linked |
|---|---|---|---|---|---|---|
| B1 | `E/c6/vaihe2/koeohjelmat/`: `c6_hk_probes.cpp` (17 TEST_CASE + `tp_build_info`), `C6ProbeHk.vcxproj`, `tapaukset.txt`, `aja.py`, `rakenna.cmd`, `q65_leave.c` | H-090 (4 return paths + control), H-093 (recursion, 2 lock edges, 2-thread deadlock, 2 controls), H-094 (+control), H-091 (2 edges, 2 controls); Q65 pure Win32 `LeaveCriticalSection` misuse | MSVC 19.52.37029 (v145) Release/Win32 (`.../tulokset/rakennus.txt`), HK console defines; Q65: `cl /nologo /W3 /O2 q65_leave.c` | `python -I tee_projektit.py`; `rakenna.cmd C:/temp/c6build`; `python -I aja.py C:/temp/c6build tulokset/uusi.txt` (`E/c6/vaihe2/README.md`) | RUN(2026-11-25, twice; 18 cases each, 11 TPRESULT observed=1, all exit 0, blocked-thread cases end by TPEXIT terminate; `tulokset/c6_hk_probes.txt`, `c6_hk_probes_ajo2.txt`, `q65_leave.txt`, `rakennus.txt`); probe sha256 `58a8ddc0...` | HkMaali source list as TpCoreHk, unchanged |
| B2 | `E/c8/koeohjelmat/`: `c8_hk_probes.cpp` (5 TEST_CASE + info), `C8ProbeHk.vcxproj`, `tapaukset.txt`, `aja.py`, `rakenna.cmd` | H-128a, H-128b, H-129 (+2 controls) | as B1 | as B1 (`E/c8/README.md`) | RUN(2026-11-26, twice; 6 cases, 3 observed=1; `tulokset/c8_hk_probes.txt`, `c8_hk_probes_ajo2.txt`, `rakennus.txt`) | HkMaali list, unchanged |
| B3 | `E/c9/vaihe2/koeohjelmat/`: `c9_hk_probes.cpp`, `c9_juk_probes.cpp`, `C9ProbeHk.vcxproj`, `C9ProbeJuk.vcxproj`, `tapaukset.txt`, `aja.py`, `rakenna.cmd` | HK Release: H-145, H-146, H-147, H-150; HK Asan: H-151; relay Asan: H-149 a-d; relay Release: Q85 repeat runs, N1 | MSVC v145 Release and Asan (`rakenna.cmd` builds both) | `aja.py <build dir> <out> <raw dir>` (`E/c9/vaihe2/README.md`) | RUN(2026-11-26; final `tulokset/ajo5.txt`: 27 cases; 13 observed=1, 3 observed=0 (controls and one Q85 case), 11 cases without TPRESULT (2 build-info, 4 HK controls, 5 relay Release cases that ended 0xC0000374, recorded as consequence not evidence); `ajo1-4.txt` earlier runs before fixture fixes; `tulokset/raw/` full output incl. ASan reports) | HK and relay lists (`Juk/VTulostus.cpp`, `Juk/vdat.cpp` included as in `T/core`), unchanged |
| B4 | `E/c9/vaihe2/koeohjelmat/rtl_bcc32.c`, `rtl_bcc32.cmd`, `tulokset/rtl_bcc32.txt` | bcc32 RTL behaviour (Q83 WEOF, Q84 stream limit, Q85 double free); no production code | bcc32 (Embarcadero C++ 7.80 for Win32) | `rtl_bcc32.cmd C:/temp/c9rtl` | RUN(2026-11-26; `tulokset/rtl_bcc32.txt`: `fputws` failure returns -1 which is not equal to WEOF 65535; `fopen` fails after 47 streams; second `free` of a block returns silently) | none |
| B5 | `E/c5/lea.tgz`, `ver.tgz`, `bda.tgz`; `E/c5/vaihe2/{lea,ver,dca,omf}.tgz`; `E/c5/skriptit/*.py`; `E/c5/msvc/*` | compiler-behaviour probe programs on BOTH compilers (encoding, printf, fp, `time_t`, `swprintf`, `wcstok`, `char` signedness, UTF-16BE `_wfopen_s` in `lea/be.cpp`, `TextFl::Open` mode-buffer overflow in `vaihe2/lea/q59/gs59.cpp`, `VersioInt` in `q57/vi57.cpp` and `vcl57.cpp`), bcc32 object comparison (`omf`), MSVC class-layout comparison (251 units, 0 errors, `msvc/msvc_run.txt`) | MSVC v145 and bcc32 classic; variants gl/nogl/nogs/libgl for `/GL` and `/GS` | per-probe `build-*.txt` inside the archives; no single build command documented | RUN(2026-11-25, `1457e26`/`b17d536`; outputs `*-bcc.out`, `*-msvc.out`, `vaihe2/*.out`); archives omit exe/obj; 4,327 file hashes in `vaihe2/raaka-manifest.sha256` | `gs59.cpp` includes `tputil.h` and uses production `TextFl` (variant `libgl` links `tputilv2.lib`) [verified]; the other probes are stand-alone programs |
| B6 | `E/c4/skriptit/`, `E/c4/commands.txt`, `E/c4/lokit/msvc-warn-logs.tgz`, `bcc32-outputs.json.gz`, `E/c4/warnings_unique.csv.gz`, triage CSVs | warning builds: MSVC `/W3` baseline and `/W4 /analyze` of HkMaali (Release; ReleaseVA200 = `VALUKU=200`), JukMaali (Release; Release150os = `MAXOSUUSLUKU=250`), JukLuenta, dbboxm, tputilv2; bcc32 `-w` re-run of commands recorded by `build-gui.ps1` logs | MSVC 14.52.37025 (v145); `base.props`/`w4analyze.props` redirect outputs; bcc32 7.80 | `E/c4/README.md` (`build.sh base, build.sh w4a`, `bcc_run.py`, `collect.py`) | RUN(2026-11-24, `f7fe553`; builds only, no test executed; logs archived); outputs went to a scratch dir outside the repo; project files unchanged | all product sources, unchanged |
| B7 | `E/c11/` (README, `SHA256SUMS` 111 entries, `BINARIES.sha256`) | raw results of A4 and A9 runs | n/a | `sha256sum -c SHA256SUMS` | stored | n/a |

Analysis scripts (read-only, not tests): `D/tyokalut/inventaario.py`, `riippuvuudet.py`, `valitsimet.py`, `asetusavaimet.py`, `resurssit.py`, `globaalit.py`,
`synteesi.py`, `tarkista_auditointi.py`, `vertaa_omf.py`, `utf8_literaalit.py`; C-13 base check `E/c13/osaA0/tarkistus_a0.py`.

### 1.3 Stored raw results (summary)

| Raw result | Run | Result | Path |
|---|---|---|---|
| TpTest 46/46 | 2026-11-24 | PASS (text only) | none stored; `D/testaus.md:62` |
| Core suite (A4, A6) | 2026-11-24 22:18, git_head `432ea3d`, tests_dirty true (later committed unchanged as `c6205d2`), production `fc03b67` clean | PASS 23, XFAIL 24 | `E/c11/vaihe4/core_all/results.json`, `results.md`, 47 `*.log` |
| Black-box runs 0 and 1 (firewall) | 2026-11-24 | as expected; no H finding confirmed or refuted | `E/c11/ajot-0-1/` |
| H-041 control 220 and boundary 221 on shipped `HkMaali.exe` | 2026-11-24 22.41-22.42 | pre-fixed evaluation "not confirmed" (criterion 7, log window); user-approved deviation then classed H-041 "confirmed by running" (testaus.md 19.11); 220 clean; 221: `0xC0000409` subcode 2 at `addfrec+0x117`, KILP.DAT 10 040 to 20 080 bytes | `E/c11/vaihe4/run_v220/`, `run_v221/`, `h041_evaluation.json`, `fwlog/` |
| Probe runs C-6, C-8, C-9, C-5, C-4 | 2026-11-24..26 | see 1.2 | `E/c6/vaihe2/koeohjelmat/tulokset/`, `E/c8/koeohjelmat/tulokset/`, `E/c9/vaihe2/koeohjelmat/tulokset/`, `E/c5/`, `E/c4/` |
| doctest on bcc32 does not compile (`nullptr`) | stage 1 planning | no stored artefact found [verified: search in `E/`] | claim only in `D/testaus.md:67` |
| This file | 2026-11-26 | nothing built or run | this file |


## 2. Findings with experiment evidence (27 H with `koe` != ei) and compiler probes

27 H: `koe_tai_testikehys` 26 + `toimitettu_ohjelma` 1 [verified: `E/c13/osaA0/yhteenveto.txt`]: C-11 harness 15 (H-001, 002, 007, 008, 022, 023, 024, 025,
026, 027, 031, 032, 040, 041, 042), C-6 probes 4 (H-090, 091, 093, 094), C-8 probes 2 (H-128, 129), C-9 probes 6 (H-145, 146, 147, 149, 150, 151).
No result of this section was produced in C-13; all RUN dates are earlier.

Column "AS IS" codes:
- **Y** = the test is in the repo harness; after a correct fix the observed mechanism disappears and the runner reports XPASS (mechanism not seen), exit code 3
  (`T/run_core_tests.py:41,47`: XPASS is never accepted). So it detects the fix as is; to use it as a green gate the manifest `kind` or the runner needs a regression mode
  (no such mode exists [verified]). Fix-shape dependence is noted per row.
- **A** = needs adaptation (what is stated).
- **N** = cannot serve (why).


### 2.1 C-11 harness (path `T/core/`, manifest `T/core/manifest.json`, result `E/c11/vaihe4/core_all/ID.log` and `results.json`, run 2026-11-24 22:18, all defect cases XFAIL, controls PASS)

| Finding | Test case (file:line) and control | What it demonstrates | Config | Recorded result | AS IS |
|---|---|---|---|---|---|
| H-001 | `H-001a relay textjtulos clears past stsk (ASan)` `VTulostusTests.cpp:55`; `H-001b relay textosuus clears past stsk (ASan)` `:71` (program TpCoreJukAsan) | mechanism: production functions called directly write 160 bytes into 40-byte `stsk` (ASan stack-buffer-overflow WRITE 160 at `Juk/VTulostus.cpp:2228` and `:3981`; disassembly-confirmed). Not shown: public entry (Q38), effect in the Release build (only the C4789 warning) | Asan (Release + `/fsanitize=address /Zi`), needs the ASan DLL | XFAIL x2 | Y if the fix removes the overflow (Asan config required; Q35 for CI) |
| H-002 | `H-002a relay legacy 6-field line writes through paikat[0] value` `VSrjTests.cpp:95`; `H-002b ... 9-field ...` `:100`; control `:68`; data `T/data/juk/h002_*.srj` | mechanism: `sscanf` writes "4242" / "TPH002" to address 0x30303030 (the int value of `paikat[0]`); the page is reserved by the test; effect with an unreserved address not shown | Release only (ASan shadow memory occupies 0x30000000-) | XFAIL x2 | Y (Release config only) |
| H-007 | `H-007 HK received packet overwrites a concurrent local change` `HkRecvTests.cpp:374`; control `:335` (waiter probe) | mechanism with forced ordering: receive thread reads before taking `tall_CS`; a local change made while it waits is lost. Propagation to peers and real frequency not shown | Release; relies on the undocumented `CRITICAL_SECTION.LockCount` encoding (Q36) | XFAIL | Y on this machine; CI image unverified (Q36) |
| H-008 | `H-008 HK tall_sak error path leaves tall_CriticalSection held` `HkLockTests.cpp:123`; controls `:70`, `:92` | mechanism: the -3 path returns with the lock owned by the ended worker thread | Release | XFAIL | Y |
| H-022 | `H-022 HK stage packet applied to a different competitor` `HkRecvTests.cpp:154`; control `:140` | mechanism: packet with bib 13 stored on the record of bib 12 (single process). Two-station allocation race not shown | Release | XFAIL | Y (two-station half needs INT, see 4.) |
| H-023 | `H-023a HK stage result applied to the receiver's own stage` `HkRecvTests.cpp:175`; `H-023b HK link accepted although the stage change was refused` `:269`; controls `:243`, `:255` | mechanism: result stored on the receiver stage; link accepted though the change was refused | Release | XFAIL x2 | Y |
| H-024 | `H-024 relay tark_trk announcer path leaves tall_CriticalSection held` `VComTests.cpp:128`; control `:111` | mechanism: lock stays owned by the ended `tarkcom` worker; packet injected into the queue, no network | Release | XFAIL | Y |
| H-025 | `H-025a relay combufsize vs header plus largest class` `VCoreTests.cpp:64`; `H-025b relay disk queue stores whole packet` `:176` | mechanism: `combufsize` 204 or 244 < 14 + largest class; slot stride truncates 14 bytes. A corrupted send on the wire is NOT reproduced; only EMITVA with LAHEMVA or YHTEISLAHTOT at n_os >= 24 are affected | Release (`n_os` 4 and 30) | XFAIL x2 | Y (a flips if `combufsize` includes the header, b if the slot stride is fixed) |
| H-026 | `H-026 relay unknown packet class stops receiving permanently` `VRecvTests.cpp:224`; control `:207` | mechanism: `inbens` stuck, loop keeps spinning after `comfl = 0`; process ended by TerminateProcess (`TPEXIT blocked_threads=1`) | Release | XFAIL | Y (a still-spinning thread after a fix is FAIL by runner rule, not XPASS) |
| H-027 | `H-027a TCP client does not retry after a failed connect` `TcpClientTests.cpp:95`; `H-027b TCPXML client dereferences a freed connection` `:136`; control `:64` | mechanism: one failed attempt, thread ends, no retry within 15 s against a 127.0.0.1 listener; TCPXML NULL read in `reconnectTCP` (0xC0000005, symbol resolved by the runner) | Release, loopback only | XFAIL x2 | Y (a needs about 15 s after the fix; b flips when no crash) |
| H-031 | `H-031 refused stage change blocks a later allowed change` `HkCoreTests.cpp:265`; control `:241` | mechanism: refused change leaves `vaiheenvaihto`, a later allowed change does nothing. Stop of background processing in the running program not shown | Release | XFAIL | Y |
| H-032 | `H-032a relay addseur copies an unchecked length past the receive block` `VRecvTests.cpp:128` (Release); `H-032b relay addseur heap-buffer-overflow (ASan)` `:151`; control `:115` | mechanism: `addseur` called directly with class 7 (`class_len` 0), len 1000, writes 498 bytes past the block (ASan WRITE 1014 at `Tp/TpComY.cpp:654`). The UDP predicate in `luepakettiUDP` is not exercised and no datagram is sent | Release and Asan | XFAIL x2 | Y if the fix is inside `addseur`; A if only the UDP length check (`Tp/TpComY.cpp:1001`) is fixed: add a test through the public receive path (INT datagram) |
| H-040 | `H-040 relay addfrec leaves kilpdat_CriticalSection held` `VAddfrecTests.cpp:74`; control `:47` | mechanism: lock held with RecursionCount 2 after `addfrec`. A real two-thread freeze is not shown (needs `tarkcom`, Q41) | Release (`vdat.cpp` included) | XFAIL | Y |
| H-042 | `H-042 relay XML counts above compile-time limits` `VCoreTests.cpp:212`; control `:190`; data `T/data/juk/h042_in_limit.xml`, `h042_over_limit.xml` | mechanism only: `luesarjat` returns 0, `n_os` 31, `valuku` 13, derived `kilprecsize` 7330 > `KILPRECSIZE` 6918. The buffer overflow itself and the ViestiWin XML button path are NOT run | Release | XFAIL | Y (reject or clamp both give observed=0) |
| H-041 | `H-041a HK record size vs KILPBUFSIZE at shipped limits` `HkCoreTests.cpp:66`; `H-041b3 HK pack writes past KILPBUFSIZE (heap guard)` `:80`; `H-041b2 HK write overflows its stack buffer (ASan)` `:128`; `H-041b4 HK addfrec overflows its stack buffer at valuku 221 (ASan)` `:193`, control `:175` (valuku 220); data checks `HkDataTests.cpp:96`, `:101` with `T/data/hk/h041_v220.xml`, `h041_v221.xml`; black-box scripts `T/blackbox/scripts/h041_v220.json`, `h041_v221.json` | harness: `kilprecsize` 11 200 > 10 000 at N_PV 5 and VALUKU 250; ASan stack-buffer-overflow in the `kilptietue::write` and `addfrec` paths. SHIPPED `HkMaali.exe`: 220 clean; 221 gives a /GS fast fail 0xC0000409 sub 2 at `addfrec+0x117`, saved EBP, return address and parameter overwritten, record written to KILP.DAT first. Not shown: read path (Q37), valuku 250 in the shipped exe, behaviour without /GS or without the debugger (WER), bcc32 builds | Asan (b2, b4), Release (a, b3, data) | XFAIL a, b3, b2, b4; shipped 221: pre-fixed verdict "not confirmed", user-approved "confirmed by running" (`E/c11/vaihe4/run_v220`, `run_v221`, `h041_evaluation.json`) | b2 and b4: Y if the fix enlarges or moves the buffer. A if the fix is a `valuku` limit in `luesarjat`: control `hk-data-h041-v221` and b4 data then fail on preconditions, while a and b3 (direct `luekentat`) would not flip, so add a limit test. Black-box run: N for CI (admin firewall rules, debugger, interactive console, shipped binary) |


### 2.2 Isolated probes outside the harness (C-6, C-8, C-9)

Common: built outside the repo, no manifest, `aja.py` records but does not judge. A probe checks preconditions (REQUIRE/CHECK) and prints one TPRESULT;
after a fix it would print observed=0 or fail a REQUIRE, but no tool in the repo classifies that. Common adaptation for "A": move the probe file to `T/core/`,
add it to `VT/TpCoreHk.vcxproj` (or the Juk project) and `T/core/manifest.json`, so that `run_core_tests.py` judges it. This has not been done; as repo tests all are PLANNED.

| Finding | Probe case (file:line) | What it demonstrates | Config | Recorded result | AS IS |
|---|---|---|---|---|---|
| H-090 | `E/c6/vaihe2/koeohjelmat/c6_hk_probes.cpp`: `C6 H-090 tall_regnly time queue not started returns at 2782 holding ajat` `:192`; `... clock difference over maxero returns at 2776 holding ajat` `:208`; `... KELLO_ESTO returns at 2771 holding ajat` `:224`; `... code G returns at 3074 holding ajat` `:244`; control `:177` | mechanism: 4 return paths of `tall_regnly` (`Hk/HkIV.cpp:1984`) leave `ajat_CriticalSection` owned by the ended thread. A real freeze and the frequency with real devices are not shown | MSVC Release, HK console defines | observed=1 x4 in both runs: `E/c6/vaihe2/koeohjelmat/tulokset/c6_hk_probes.txt`, `c6_hk_probes_ajo2.txt` | A (move into harness) |
| H-091 | `c6_hk_probes.cpp`: `C6 H-091 edge tall to print: tall_elahto holds tall and waits for print` `:817`; `C6 H-091 edge print to tall: autotulostus holds print and waits for tall` `:871`; controls `:798`, `:857` | mechanism: each lock-order edge blocks while the other lock is held by a test thread (TPEXIT terminate); the two edges are not combined into one 2-thread deadlock; probe sets GUI-only `lisaToimetOn` and `GDIPRINTER`; the bcc32 GUI cycle was not run | MSVC Release | observed=1 x2, same files | A (the real cycle also needs a bcc32 GUI run, see 4.) |
| H-093 | `c6_hk_probes.cpp`: `C6 H-093 wkirjloki TCP log reconnect recursion overflows the stack` `:531`; `C6 H-093 edge loki to tcpip: ...` `:574`; `C6 H-093 edge tcpip to loki: ...` `:596`; `C6 H-093 two-thread deadlock loki and tcpip` `:619`; controls `:508`, `:557` | mechanism: unbounded recursion until 0xC00000FD (501 nested frames), and a real 2-thread deadlock between `loki_CS` and `tcpip_CS` (both threads alive 2 s after release). Connection records use INVALID_SOCKET, no network | MSVC Release | observed=1 x4, same files | A (the recursion case ends in a caught stack overflow: needs a `termination: exception` entry like H-027b) |
| H-094 | `c6_hk_probes.cpp`: `C6 H-094 add_bdg_t during vaiheenvaihto: consumer exits and badge is never processed` `:704`; control `:689` | mechanism: consumer `pros_bdg_t` exits at once; `regnlyhetki[0]` stays at the sentinel after 1 s | MSVC Release | observed=1 | A |
| H-128 | `E/c8/koeohjelmat/c8_hk_probes.cpp`: `C8 H-128a setpos to zero leaves a stale entry that stops the walk`; `C8 H-128b lowering a bib into a free number drops it from the index`; control `C8 H-128 control: walk by bib visits every live entry` | mechanism in production `setpos`/`getpos`/`nextpos`: stale entry stops the walk (6 of 8 visited); a lowered bib is not findable. Shipped program not run | MSVC Release | observed=1 x2: `E/c8/koeohjelmat/tulokset/c8_hk_probes.txt`, `c8_hk_probes_ajo2.txt` | A |
| H-129 | `c8_hk_probes.cpp`: `C8 H-129 stage change keeps idindex->nrec`; control `C8 H-129 control: luo_posarr resets idindex->nrec` | mechanism: production `vaihdavaihe` and `teejarj(true)` leave `idindex->nrec` 5 instead of 0. The heap overflow itself (about 1 500 records) is NOT run | MSVC Release | observed=1 | A (add records up to capacity under ASan or PageHeap for the overflow) |
| H-145 | `E/c9/vaihe2/koeohjelmat/c9_hk_probes.cpp`: `C9 H-145 emit write failure with duplicate suppression (default)`; `C9 H-145 emit write failure without duplicate suppression`; control `C9 H-145 control: emit card is stored` | mechanism: a failed write returns the ACK value, card cached but not stored; reader ACK semantics (Q86) not shown | MSVC Release | observed=1 x2: `E/c9/vaihe2/koeohjelmat/tulokset/ajo5.txt`, `raw/Release_C9ProbeHk_C9_H-145_*.txt` | A |
| H-146 | `c9_hk_probes.cpp`: `C9 H-146 AJAT write failure leaves a hole that the restart reads`; control `C9 H-146 control: AJAT rows are written and read back` | mechanism: `fp1` advanced before the failed write, empty slot read on restart | MSVC Release | observed=1 | A |
| H-147 | `c9_hk_probes.cpp`: `C9 H-147 WriteLine return value on a failed flush vs WEOF`; `C9 H-147 autofile sequence truncates the published file and hides the failed flush`; `C9 H-147 autofile with the published file locked whole`; control `C9 H-147 control: autofile replaces the published file` | mechanism: WEOF check misses write errors (MSVC CRT value -1); published file truncated to 0 bytes without a message. bcc32 RTL differs (`rtl_bcc32.txt`); share mode (Q92) resolved statically only | MSVC Release | observed=1 x3 | A (the WEOF case documents MSVC CRT behaviour) |
| H-149 | `E/c9/vaihe2/koeohjelmat/c9_juk_probes.cpp`: `C9 H-149a relay autofile single-file mode writes through the freed PRFILE (ASan)`; `C9 H-149b relay autofile T mode frees the PRFILE twice (ASan)`; `C9 H-149c relay autofile separate-files mode without a list deletes aflst twice (ASan)`; `C9 H-149d relay autofile X mode leaves lstf dangling for the next run (ASan)`; control `C9 H-149 control: relay autofile list mode (ASan)`; Release cases `C9 Q85 Release ...` | mechanism: use-after-free and double free shown by ASan (a, b, d; c needs code reading as well). Release effect: 5 cases crash 0xC0000374 (consequence only), list mode and the TCP-link case survive. ViestiWin (bcc32, VCL memory manager) not run (Q91) | Asan (a-d), Release (Q85) | ASan observed=1 x4, control observed=0: `ajo5.txt`, `raw/Asan_C9ProbeJuk_*` | A for a, b, d (Asan project); N for the Release Q85 crash cases (a crash is a consequence, the runner never accepts it as evidence) |
| H-150 | `c9_hk_probes.cpp`: `C9 H-150 autofile XML stage 2 with AUTOFILE.LST`; `C9 H-150 autofile XML stage 2 without AUTOFILE.LST`; control `C9 H-150 control: autofile XML stage 1` | mechanism measured: 700 runs, heap +60 200 bytes (86 per run), streams and handles +508 with the list. bcc32 HkKisaWin steady state (Q95) not run | MSVC Release (heap walk, stream count) | observed=1 x2 | A |
| H-151 | `c9_hk_probes.cpp`: `C9 H-151 next autofile reads the PRFILE freed by sendtcpfile (ASan)` | mechanism: use-after-free READ 4 reported by ASan | Asan | observed=1 | A |


### 2.3 C-5 compiler probes and C-4 warning builds

These items have `koe` = ei in `havainnot.csv` (H-078, H-079, H-080, H-001/H-002 diagnostics) but carry measured evidence [verified: `nayton_laji` column; `D/kaantajaerot.md` 7 and 13.4].

| Finding / item | Probe or build (path) | What it demonstrates | Config | Recorded result | AS IS |
|---|---|---|---|---|---|
| H-078 (P4) | `E/c5/lea.tgz`: `lea/be.cpp`, `lea/be-msvc.out` (`_wfopen_s` with mode `wb, ccs=UTF-16BE`) | mechanism only: CRT invalid-parameter fast fail (rc -1073740791 = 0xC0000409) on MSVC `/MT`; bcc32 opens the file. Production `TextFl` is not called; a console abort on a UTF-16BE input file is [inferred] from the 68 read sites | MSVC v145 and bcc32 | `be-msvc.out` | N (stand-alone CRT call); a repo test would open a `FE FF` file with production `TextFl` in the core harness |
| H-079 (P4) | `E/c5/vaihe2/lea.tgz`: `lea/q59/gs59.cpp`, `run.out`, `run_bcc.out`, `run.py`, `runb.py`; `vaihe2/ver.tgz` `ver/q59/` | effect in production code: `TextFl::Open` `wchar_t mode[16]` overflow with `ccs=UTF-16LE`: MSVC variant `nogl` 0xC0000409 (/GS), `nogs` 0xC0000005, `gl` survives, `libgl` (links `tputilv2.lib`) 0xC0000409; bcc32 0xC0000005 (clobbered saved EBP) | MSVC v145 variants and bcc32 | `run.out`, `run_bcc.out` | A (hand-run scripts judge the exit code: wrap as a manifest entry with `termination: exception`; the bcc32 variant needs its own runner) |
| H-080 (P5) | `E/c5/ver.tgz`: `ver/p_tt/tt.cpp`, `tt-bcc.out` [role inferred from the name] | mechanism: bcc32 `mktime` for year 2040 returns -1, so `date_time_t` gives 0 | bcc32 | `tt-bcc.out` | N (stand-alone; the fix would be in `Tp/TpApu.cpp`) |
| T-025 (VersioInt) | `E/c5/vaihe2/lea.tgz`: `lea/q57/vi57.cpp`, `vcl57.cpp`, `q57.out` | VCL control word 0x037F gives `VersioInt` 520, MSVC 521 | bcc32 VCL probe | `q57.out` | N (no reader depends on the tag) |
| T-026 (swprintf, atoi, wcstok, iswalnum) | `E/c5/ver.tgz`: `ver/p_sw/`, `p_wt/`, `p_loc/`; `E/c5/vaihe2/lea.tgz`: `lea/q58/`, `lea/q61/` | run-time differences bcc32 vs MSVC measured; only five `swprintf` sites can truncate | both | `*.out` | N |
| H-001 (C4789), H-002 (C4477) | `E/c4/lokit/msvc-warn-logs.tgz` (baseline and `/W4 /analyze` logs); C4789 appears only at the `/GL` link (`D/muistiturvallisuus.md:55-56`) | compile-time diagnostic that the overflow is visible to the compiler | MSVC v145 `/GL` link | logs, `E/c4/warnings_unique.csv.gz` | Y at build level: the diagnostic at `Juk/VTulostus.cpp:2228`, `:3981` (C4789) and the `Juk/vSarjat.cpp` sscanf site (C4477) disappears after a fix; needs a full LTCG link of JukMaali |
| C-4 findings H-055..H-077 | `E/c4/` warning triage, `E/c4/commands.txt` | static only: no experiment; the Release150os (`MAXOSUUSLUKU=250`) and ReleaseVA200 (`VALUKU=200`) builds are compile evidence for T-051 | MSVC | logs | N |


## 3. State of automation

### 3.1 `T/run.sh` breakage [verified: file read with `cat -A`, `git log -p`]

| Item | Evidence |
|---|---|
| Cause 1: missing line continuation | `T/run.sh:14` (`Juk/VRinnakkaisOsuus.cpp`) ends without a backslash, so the following lines (`Tests/HkEnnTavTest.cpp`, `Hk/HkEnnTav.cpp`, `tputilv2/aikatowst_s.cpp`, `tputilv2/putint_r.cpp`) form a second command; with `set -e` the script stops |
| Cause 2: missing include path | `T/run.sh:11` has `-I include -I Tp -I Juk -I Tests` but no `-I Hk`, which `Tests/HkEnnTavTest.cpp` and `Hk/HkEnnTav.cpp` need; the MSVC project has Hk;Juk;Tp;include;Tests (`VT/TpTest.vcxproj:61`) |
| Origin | Two branches each created `run.sh`: `4b50eda` (relay version: `-I Juk`, sources DoctestMain, VRinnakkaisOsuusTest, VRinnakkaisOsuus) and `4282f22` (HK version: `-I Hk`, DoctestMain, HkEnnTavTest, HkEnnTav and 2 tputilv2 units); the merge `6107c8f` left the hybrid that exists now (last commit touching the file) |
| Run history | never run during the audit; whether g++ can build the production units at all is [unknown] (TpTest is MSVC-only so far) |
| Documentation | `CLAUDE.md:59`, `D/rekisteri/havainnot.md:15` (H-006), `D/seuranta.md:320` (issue draft 05); decision 6 of `testaus.md` 12 (drop or fix) still open |

### 3.2 Status table

| Item | State | Evidence |
|---|---|---|
| TpTest | works, 46/46 (15 + 31 TEST_CASE, 94 assertions), RUN 2026-11-24; covers 4 pure units (names suggest: HK personal-best/target columns, relay parallel-leg start logic, 2 tputilv2 time/int helpers) [inferred from file names and commit messages]; no core code, no globals, no I/O, no network | `D/testaus.md:61-63`; `VT/TpTest.vcxproj:87-93` |
| Core suite (TpCoreHk, TpCoreJuk, Release and Asan) | works locally; PASS 23, XFAIL 24 (47 manifest entries); 46 runner self-tests; MSVC v145 only; not run in CI; not built or run in C-13 | `E/c11/vaihe4/core_all/results.md` |
| Black-box layer (shipped `HkMaali.exe`) | works once on the owner machine; needs admin-created firewall rules, an interactive console session and debugger mode; H-041 only | `D/testaus.md` 17-19 |
| Layer (e) two-process loopback | PLANNED only; no runner, no fixture beyond `T/fixtures/TpNet.cpp` | `D/testaus.md:87`, 20.4 |
| Layer (f) GUI | PLANNED only (manual) | `D/testaus.md:88` |
| bcc32 tests | none exist; doctest 2.4.11 does not compile on classic bcc32 (`nullptr`); `static_assert` and the negative-array-typedef trick work on both compilers (probe result, not stored); `T/doctest.h` is 2.4.11 (`T/doctest.h:47-49`) | `D/testaus.md:67` |
| CI | builds only, no test is run (3.3) | `.github/workflows/build.yml`, `release.yml` |
| Python test runners | self-tests exist (`test_run_core_tests.py` 46, `blackbox/test_run_hkmaali.py` 39) but no workflow runs them | [verified] only `docs.yml` has a Python step (`actions/setup-python`, docs site) |
| Push and PR ban | pushing and PRs are banned for now (project memory), so no workflow change can be validated on the hosted runner | session memory index |


### 3.3 What CI builds and runs [verified: workflow files read]

| Workflow / job | Runner | Toolset | Builds | Tests | Artefacts |
|---|---|---|---|---|---|
| `build.yml` `console` (push and PR to main/master, dispatch) | `windows-latest` | `/p:PlatformToolset=v143` on all three solutions | `VS/Libs/tputilv2.sln`, `VS/Hk/HkMaali520.sln`, `VS/V/JukMaali520.sln`, Release/Win32 | none; `TpTest.sln` and `TpCore.sln` are NOT built | none |
| `build.yml` `gui` | self-hosted `[self-hosted, Windows, X64]` (runner TPbuilderi, C++Builder Community in `C:/Program Files (x86)/Embarcadero/Studio/37.0`); fork guard on pull requests (head repo must equal this repo); timeout 90 min | classic bcc32 through `bds.exe -b` (IDE driven; Community edition refuses direct command-line compilation; `-Tool msbuild` only for licensed editions) | `RADStudio10/build-gui.ps1`: DBboxm-XE, Tputil-XE, HkKisaWin, ViestiWin (default config of each `.cbproj`, Win32) | none | `HkKisaWin.exe`, `ViestiWin.exe` (30 d), build logs (7 d) |
| `release.yml` `console`, `gui`, `package` (tag `v*` or dispatch) | `windows-latest`; `gui` self-hosted; `package` windows-latest | v143 / bcc32 | same builds; `collect-gui-runtime.ps1` copies BPL and DLL runtime files found from the PE import table; ZIP with `SHA256SUMS.txt`; `gh release create` only for tags | none; no start of any program, no clean-machine check (Q98) | release ZIP |

### 3.4 Constraints and open questions

| Item | State |
|---|---|
| Self-hosted runner | same-repo runs only (fork guard); `bds.exe` build is GUI-session dependent, with dialog-closing logic and a 10-minute per-project timeout in `build-gui.ps1`; the Windows SDK must be one C++Builder 13 recognises (up to 10.0.26100.6584, `CLAUDE.md`); linker heap errors may need `bcdedit /set IncreaseUserVa 3072`; whether MSVC v145, Python or an interactive console session exist there is [unknown] (Q40) |
| Q35 | OPEN. Do the test projects build with v143, and does `windows-latest` have x86 ASan? Locally VS 18 (14.52) has `clang_rt.asan_dynamic-i386.dll`, local VS 2022 does not (`D/testaus.md:68`). `TpTest.vcxproj` and `TpCore*.vcxproj` hard-code v145; CI would pass `/p:PlatformToolset=v143` as it does for the product solutions. Not tested |
| Q36 | OPEN. The `LockCount` encoding used by the H-007 waiter probe (`(-1 - LockCount) >> 2`) was measured 200/200 on Windows 11 26100 only (`D/testaus.md:138`); undocumented; CI image not checked |
| Q39 | OPEN. Whether `/we4789` becomes an error under `/GL` link-time code generation (it would make the C4789 of H-001 a build gate); never tested |
| Q40 | OPEN. Can the self-hosted runner run console programs in an interactive session (layer d needs `AttachConsole` and `WriteConsoleInputW`)? Black-box runs also need admin for firewall rules |
| Q103 | OPEN, needs an online check that was not done: does `windows-latest` still offer v143, do `upload-artifact@v7` and `download-artifact@v7` exist |
| Open decisions | `D/testaus.md` 12.1: item 4 (known defects as expected failures = green CI, or red), item 6 (`run.sh`), item 7 (later production test hooks for H-007, H-009; extraction for H-027, H-032) |

### 3.5 Future work items to run tests in CI (list only; nothing was changed)

1. Decide the XPASS policy (decision 4) and add a regression mode to `run_core_tests.py` (or flip the manifest `kind` after each fix), so that a fixed defect turns the gate green instead of exit 3.
2. Make the test projects toolset-neutral, or pass `/p:PlatformToolset=v143` for `TpTest.sln` and `TpCore.sln` in the `console` job; settle Q35.
3. `console` job: build `TpTest.sln`, run `TpTest.exe` (46 cases, `--reporters=junit`), upload the JUnit file.
4. `console` job: after `tputilv2.sln`, build `TpCore.sln` Release (about 2 min link per project), run `python -I Tests/test_run_core_tests.py` and `python -I Tests/run_core_tests.py --out DIR`; upload `results.json` and `results.md`.
5. Add the `Asan` configuration once x86 ASan on the hosted image is confirmed (Q35); the DLL copy step uses `$(VCToolsInstallDir)`, so it is image-independent [inferred]; keeps H-001, H-032b, H-041b2 and H-041b4 testable.
6. Verify Q36 on the image, or provide a fallback for the H-007 waiter probe; verify Q39 before using `/we4789` as a gate.
7. Move the C-6, C-8 and C-9 probe files into `T/core/` with manifest entries (today they are generated project copies that build outside the repo).
8. Fix or delete `T/run.sh` (decision 6).
9. Add a test gate before `package` in `release.yml`, plus a clean-machine start check of the release ZIP (Q98) as a manual or VM step.
10. bcc32 side: no automated test is possible with doctest 2.4.11; options are a C-style harness compiled by bcc32 (`rtl_bcc32.c` is the only precedent) or an older doctest; the `gui` job could only add a post-build step on the self-hosted runner (fork guard stays).
11. Black-box layer (d) stays manual: not suitable for hosted CI (admin firewall rules, debugger, interactive console).
12. Layer (e): write the two-process loopback runner (127.0.0.1 only, separate directories; a firewall rule is needed because of the `INADDR_ANY` binding, H-029) before gating H-022, H-032 and H-027 real-path tests.
13. Lift the push ban before any workflow change can be validated.


## 4. Open verification obligations (run-time, build, device, network, GUI, SQL)

One row per open item from the "Avoimet jatkotehtavat" lists of `D/seuranta.md` (C-5, C-6, C-8, C-9, C-10, C-11 and the C-7 queue J7-4), the batch reports named in the task and the open
Q register (`kysymykset.csv`, status avoin / osittain / ajonaikainen_todennus). Questions that are purely static or owner decisions (Q3, Q6, Q9-Q12, Q14, Q15, Q24-Q26, Q29, Q47 apart from the field part,
Q50, Q51, Q54, Q60, Q68, Q75-Q77, Q79, Q82, Q94, Q101, Q111, the 161 parse candidates, J7-1..J7-3, J7-5..J7-7) are not test obligations and are omitted [inferred classification].
Exactly one test type per row (device named after DEV:). All rows are PLANNED: none of them has been run. "Existing asset" names a probe or fixture that covers part of the item.

| ID | H / Q ids | What must be shown | Test type | Prerequisites | Existing probe or fixture | Source |
|---|---|---|---|---|---|---|
| O01 | H-009 | lock held across SendMessage (add_sakko, writeerror_w) never deadlocks the GUI; forced timing | GUI | HkKisaWin (bcc32) on a test VM; debugger or production test hook (hook decision 7 open, separate permission) | none (plan only, testaus.md 6) | testaus.md 6, 20.4 |
| O02 | H-014 | MySQL write thread keeps writing after a stage change; failed rows and deletes are not lost | SQL | local MySQL on 127.0.0.1, HkKisaWin with SQL on, test machine | none (plan only, testaus.md 6) | testaus.md 6, 20.4 |
| O03 | H-022 | two stations adding at the same time get distinct record numbers (Hk/HkDat.cpp:626-636) | INT | two HkMaali processes, 127.0.0.1 only, separate directories, firewall rule (INADDR_ANY binding) | single-process H-022 test (HkRecvTests.cpp:154) and TpNet.cpp helper; no two-process runner | testaus.md 15.5, 20.4 |
| O04 | H-032 | a forged UDP datagram with class_len 0 is rejected by the public receive path (Tp/TpComY.cpp:1001) | INT | loopback datagram to a relay instance or harness through the public receive predicate | H-032a/b (VRecvTests.cpp:128,151) call addseur directly | testaus.md 15.3, 20.4 |
| O05 | H-027 | real TCP reconnect after the peer returns | INT | two processes on 127.0.0.1 | H-027a/b (TcpClientTests.cpp:95,136) and TpNet listener | testaus.md 14.3, 20.4 |
| O06 | H-041, Q37 | HK start-up reads every record with the 10 000-byte buffer (kilptietue::read): crash at start-up? | UM | /GS fast fail is the only detector for the kernel ReadFile write (ASan does not see it); KILP.DAT with 221-valuku records | write-path tests b2, b4 only | testaus.md 14.1, 18.1, 20.4 |
| O07 | H-041 | valuku 250 and other n_pv/valuku combinations | UM | data XML like h041_v221.xml, Asan build | T/data/hk/h041_v22[01].xml and test b4 as pattern | testaus.md 20.4 |
| O08 | H-041 | non-zero overflow bytes (split times of the last stage), WER behaviour without debugger | ACC | shipped HkMaali.exe, admin firewall rules, new data set | T/blackbox scripts v220/v221 (overflow bytes were zeros) | testaus.md 19.9, 19.10, 20.4 |
| O09 | H-041, H-042 on bcc32 | behaviour of HkKisaWin and ViestiWin builds (no /GS equivalent assumed, Q49) | UB | bcc32 build; doctest cannot be used, C-style harness needed | none (testaus.md 6 plans it as manual) | testaus.md 6 |
| O10 | H-001, Q38 | public entry to textjtulos and textosuus; effect in the Release build | UM | static search for the entry first, then a Release (non-ASan) test | H-001a/b are ASan-only direct calls | testaus.md 20.4 |
| O11 | H-001, Q39 | /we4789 under /GL turns C4789 into an error | UM | test build only (it would break the relay build) | none | testaus.md 6, 11 |
| O12 | H-002 | effect with an unreserved address 0x30303030 | UM | Release harness without the page reservation | H-002a/b reserve the page | testaus.md 15.3, 20.4 |
| O13 | H-040, Q41 | real freeze: which relay threads call addtall/deltall | INT | ViestiMaali with a network link (tarkcom thread); ViestiWin variant is GUI | H-040 single-thread test (VAddfrecTests.cpp:74) | testaus.md 13.4, 20.4 |
| O14 | (no Q id; H-032 neighbour) | read of class_len[pkgclass] for index 15..255 (Tp/TpComY.cpp:1001) | UM | Asan harness | none | testaus.md 15.5 |
| O15 | H-023, H-026, H-032 | GUI paths (cbHk/WinHk.cpp:2149-2160, :2231) | GUI | HkKisaWin plus a second station | none | testaus.md 15.5, 20.4 |
| O16 | Q35 | test projects build with v143; x86 ASan present on windows-latest | UM | a CI run (push ban must be lifted) | local v145 build only | testaus.md 11, 20.4 |
| O17 | Q36 | LockCount encoding identical on the CI image | UM | CI run | H-007 control test checks it locally | testaus.md 11, 20.4 |
| O18 | Q40 | self-hosted runner can run console programs interactively | ACC | access to the runner | none | testaus.md 11, 20.4 |
| O19 | WER indicators | positive control for the WER absence indicators | ACC | only if run-time crashes of shipped programs continue; admin | crash_probe.c and selftest5 crash control (without WER) | testaus.md 19.4, 20.4 |
| O20 | H-087, H-103 | EmitHtmlOsuus leaks print_CS/rastiva_CS; ViestiWin GUI-only code cannot be linked into a console probe | GUI | ViestiWin bcc32 build, debugger or Application Verifier (OwningThread, RecursionCount) | none (source evidence only) | saikeet-ja-lukot.md 13.10-13.11 |
| O21 | H-091 on bcc32 | lock cycle tall to print and print to tall in HkKisaWin (GDI printer path) | GUI | HkKisaWin bcc32 build | C6 H-091 edge probes (MSVC console only) | saikeet-ja-lukot.md 13.10, seuranta C-6 list |
| O22 | Q67 | helper threads of aloitus before the failure point; aloitus twice in one GUI process | GUI | GUI process | none | saikeet-ja-lukot.md 13.10 |
| O23 | Q73 | console CTRL_CLOSE time limit on the target system; maaliajat bytes reuse on restart | ACC | target OS | none | saikeet-ja-lukot.md 13.10 |
| O24 | H-090 | frequency of the four return paths with real devices | DEV:timing-reader (device not identified in the sources read [unknown]) feeding tall_regnly | device or recorded message stream | C6 H-090 probes (synthetic) | saikeet-ja-lukot.md 13.10 |
| O25 | H-093 | comfl condition and recursion with a real TCP log configuration | INT | loopback TCP LOKI peer | C6 H-093 probes (INVALID_SOCKET records) | saikeet-ja-lukot.md 13.10 |
| O26 | H-094 | freed-use race at add_bdg_t consumer | UM | ASan or timing harness | C6 H-094 probe (no ASan) | saikeet-ja-lukot.md 13.10 |
| O27 | H-106, H-107 | cross-thread TEdit::SetText; ToimintaTila==1 with a reader; SendMessage to a destroyed window | GUI | HkKisaWin on a test VM | none | saikeet-ja-lukot.md 13.10 |
| O28 | Q65 | extra or wrong-thread Leave on older Windows and with waiters | ACC | older Windows VM | q65_leave.c (run on this Windows only) | saikeet-ja-lukot.md 13.10 |
| O29 | H-129 | the heap overflow itself (about 1 500 rows, more than maxrec+2) | UM | ASan or PageHeap, records up to capacity | C8 H-129 probe shows only the unreset count | globaali-tila.md 11, seuranta C-8 list |
| O30 | H-128, H-129 | confirmation in a shipped program (delete in the middle, export in bib order; stage change) | ACC | shipped program with isolation, permission | C8 probes | globaali-tila.md 11, seuranta C-8 list |
| O31 | H-132, Q80 (H-133) | AJAT= name in a one-stage competition; retry in the same process hits a sharing violation on KILP.DAT | GUI | HkKisaWin | none | seuranta C-8 list |
| O32 | Q78, Q81, Q90, Q47 | field practice: real n_lisaa, several readers per HK instance, memory-only outbuf, hand-edited RADAT.LST | ACC | owner and maintainer information | none | globaali-tila.md 9, virheenkasittely.md 13.5, muistiturvallisuus.md 15 |
| O33 | H-109, H-110, H-111, H-115, H-120 | kilparr[-1] and double delete; thread stop after the Lahtoajat button; L row from the panel; VCL exception between Enter and Leave; conflicting modes | GUI | ViestiWin and HkKisaWin on a test VM, debugger | none | gui-liiketoimintalogiikka.md 10, 13.2 (J7-4) |
| O34 | H-112, H-113, H-114, H-116, H-125 | out-of-object writes in the bcc32 layout; nilm[-1] and Sarjat[-1] in the link layout; X to 2 in this C++Builder version; EDivByZero in bcc32 | UB | bcc32 probe programs and .map; precedent rtl_bcc32.c | none | gui-liiketoimintalogiikka.md 10, 13.2 (J7-4) |
| O35 | H-078, H-079, T-025 (Q57) | shipped-program confirmation: UTF-16BE file at start-up, UTF-16 file re-open, FPU control word and VersioInt | ACC | permission to run shipped programs | E/c5 probes (not shipped programs) | kaantajaerot.md 13.6-13.7, seuranta C-5 list |
| O36 | Q63 (H-085) | does bcc32 new int[negative n] throw or allocate | UB | bcc32 console probe | none | kaantajaerot.md 10 |
| O37 | Q64 (H-082) | is wkello() length such that SubString(15,2) in the undo path is correct | GUI | ViestiWin | none | kaantajaerot.md 10 |
| O38 | Q45 (H-055 GUI) | bcc32 atoi overflow behaviour | UB | bcc32 probe or RTL source | rtl_bcc32.c pattern | muistiturvallisuus.md open questions |
| O39 | Q46, Q49 (H-067, H-055) | which global follows mrivi in each exe (linker map); ASLR and stack protection in bcc32 GUI builds (PE header) | UB | bcc32 link with map file; PE header read | none | muistiturvallisuus.md open questions |
| O40 | Q48 (T-021) | format strings of GUI files checked by an MSVC syntax-only pass over the GUI core | UM | MSVC syntax-only pass | E/c4 MSVC warning builds cover console code only | muistiturvallisuus.md open questions |
| O41 | Q33 | mbtowc above 127 in the C locale on both compilers | UB | bcc32 and MSVC probe pair | E/c5 encoding probes (partial) | open questions |
| O42 | H-071, H-072 | one forged FILESEND packet (length) and one EXTRA packet (index) sent to an isolated HkMaali instance | INT | isolated HkMaali with firewall rules, loopback sender, admin | none (a harness UM variant calling the handlers under ASan is possible) | muistiturvallisuus.md 15 (proposed later runtime checks) |
| O43 | H-055, H-070 | tulk_emiTag P index and lue_FEIG frame length through the existing TpCore input path | UM | Asan harness; the planned InbufInjector is not written | TpCore fixtures; no test for these | muistiturvallisuus.md 15 |
| O44 | H-061, Q43 | pkglen survives between calls in the shipped builds (disassembly) and chunked binary packets | UM | Release harness plus disassembly | none | muistiturvallisuus.md 15 |
| O45 | H-056, Q44 | ARES frame bytes: 0x00 bytes in frames, growth of the serial queue | DEV:ARES | protocol description or capture | none | muistiturvallisuus.md open questions |
| O46 | H-055 (Q56), Q99 | can emiTag card contents influence the P index; get_stream.php response format and server owner | DEV:emiTag | vendor protocol, owner information; no network contact from the audit | none | muistiturvallisuus.md; riippuvuudet.md 17 |
| O47 | H-076, Q52 | menu reachability of XML + H + all legs in console and AutoFile menus | ACC | shipped console program | none | muistiturvallisuus.md 15 |
| O48 | H-073 (Q53), Q55 | EMVALOKI settable from the GUI; XML import keep-positions loop at maxrec | GUI | HkKisaWin, ViestiWin | none | muistiturvallisuus.md open questions |
| O49 | H-152 | reproduce with the GUI handler and printable results | GUI | HkKisaWin or ViestiWin | none | virheenkasittely.md 13.5, seuranta C-9 list |
| O50 | H-153 | deadlock in a GUI run | GUI | GUI | none | seuranta C-9 list |
| O51 | Q91 (H-149) | ViestiWin VCL memory manager reaction to the PRFILE double free | UB | bcc32 VCL probe program | rtl_bcc32.c covers the RTL only | virheenkasittely.md 13.5 |
| O52 | Q95 (H-150) | HkKisaWin stream count at steady state; NULL femitfile at stage change | GUI | HkKisaWin | C9 H-150 probe (MSVC) | virheenkasittely.md 13.5 |
| O53 | H-149 | confirmation in the shipped relay console program (permission) | ACC | shipped ViestiMaali, isolation | C9 H-149 ASan probes | seuranta C-9 list |
| O54 | Q86, Q100 (H-145) | reader ACK final or not; Emit clock and reader without ACK, late ACK, - reply; checksum in the 462-byte frame | DEV:Emit | Emit reader and clock, device documentation | C9 H-145 probes (synthetic write failure) | virheenkasittely.md 13.5; riippuvuudet.md 17 |
| O55 | Q87 (H-146, H-015) | network-share handle after a link drop: permanent write failure or SMB reconnect | INT | SMB share and a controlled disconnect | C9 H-146 probe (local write failure) | virheenkasittely.md 13.5 |
| O56 | Q89 (H-157) | console key routing from a worker thread | ACC | shipped console program | none | virheenkasittely.md 13.5 |
| O57 | Q93 (H-149, H-151) | does the sendtcpfile STX/file/ETX stream break the peer parsing on an ordinary data link | INT | two instances on loopback | none | virheenkasittely.md 13.5 |
| O58 | Q96 (H-159) | does the Sirit/Zebra reader thread end after a reconnect (openConnection result tested inverted) | UM | TCP stub on 127.0.0.1 | TpNet.cpp listener helper [inferred reusable] | riippuvuudet.md 17, 19.7 |
| O59 | H-159 (matrix T4) | reconnect with a real Sirit or Zebra reader | DEV:Sirit/Zebra | reader device | none | ominaisuusvalitsimet.md 10 (T4) |
| O60 | Q97 (H-160, H-162) | an SQL error in the write thread ends the program; export keeps tall_CriticalSection | SQL | isolated MySQL, no production server | none | riippuvuudet.md 17 |
| O61 | Q104, Q105, Q106 (H-161, H-169) | vert0 after an SQL round trip; reachability of SQL injection; propagation of the SQL import overwrite and recovery | SQL | local MySQL | none | riippuvuudet.md 19.7 |
| O62 | Q98 (H-163, H-165) | GUI programs start from the release ZIP on a clean machine; what is missing for the SQL connection | REL | clean VM, release ZIP | collect-gui-runtime.ps1 (build-time collection only) | riippuvuudet.md 17 |
| O63 | Q102 | IndySystem package can be removed from the link; Idwinsock2 start-up effect | UB | bcc32 link trial | none | riippuvuudet.md 17 |
| O64 | Q103 | windows-latest still offers v143; artifact actions v7 exist | UM | online check of the CI image (not done) | none | riippuvuudet.md 17 |
| O65 | Q107 (H-171) | what bcc32 places after cfgfilename; are autostr and baudst zeroed | GUI | ViestiWin bcc32 build, .map or debugger | object-file reading only (ominaisuusvalitsimet.md 13.2) | ominaisuusvalitsimet.md 13.4 |
| O66 | Q108 (H-172) | content of the relay console XML file, also after an earlier autofile XML run | ACC | shipped ViestiMaali console; the black-box driver is HK-specific | none | ominaisuusvalitsimet.md 13.4 |
| O67 | Q110 (H-134) | what the stage-change else-branch does on freed track memory after a parse error | UM | ASan, C-8 probe pattern | C8 probes use the same vaihdavaihe path | ominaisuusvalitsimet.md 13.4 |
| O68 | Q109 (H-043) | GETREC bounds check beyond end of file (SQLjonoVarm.dat) | UM | harness with a short file | none | ominaisuusvalitsimet.md 13.4 |
| O69 | Q112 | AIKAKORJAUS unit (HK SEK/100, relay SEK) | ACC | owner or a clock device | none | ominaisuusvalitsimet.md 13.4 |
| O70 | Q19 | a duplicate AIKAT is applied twice (tall_rivi) | UM | harness, Hk/HkAjat.cpp:161-266 | none | seuranta open questions |
| O71 | Q20 | TCP stream allows more than one unacknowledged packet (Tp/TpComY.cpp:1318,1359) | INT | two processes on loopback | none | seuranta open questions |
| O72 | Q21, Q22 | conflicting edits settle in a 3-5 station ring; relay tark_ylahto circulation without katkkierto | INT | 3-5 stations (multi-workstation network) | none | seuranta open questions |
| O73 | Q23 | Kysy(..., wait=0) waits for the user | GUI | GUI | none | seuranta open questions |
| O74 | Q27 | non-multiple file length after a power cut during a write-through append | ACC | power-cut test on a VM or disk | none | seuranta open questions |
| O75 | Q28 | dbboxm initindex wins at link time over Hk/HkIx.cpp or Juk/VIx.cpp | UM | link-map check of the MSVC link | core link succeeded (no duplicate symbol), not analysed | seuranta open questions |
| O76 | Q31 | H-041 analogue in relay builds with MAXOSUUSLUKU 250 | UM | build of the unshipped configuration (matrix T17) | E/c4 Release150os compile and link (no run) | seuranta open questions |
| O77 | Q32 | unit and wraparound of the t_time_l timestamp | UM | harness | none | seuranta open questions |
| O78 | Q34 | kilptietue constructor runs before valuku is read (global objects): heap overflow in unpack | UM | ASan harness | HkFixture sets kilpparam before the first kilptietue (testaus.md 3.3), so the production order is not exercised | seuranta open questions |
| O79 | Q42 | HK SEURAT/AIKAT packet classes exceed combufsize | UM | size test in TpCoreHk | H-025a pattern (relay) | testaus.md 11 |
| O80 | (housekeeping) | remove the firewall rules TP C11 HkMaali block in/out (admin, command prepared) | ACC | admin shell | command in testaus.md 20.6 | testaus.md 20.6, seuranta C-11 list |

Count per test type (rows O01-O80):

| Type | Rows |
|---|---|
| UM automated unit/harness (MSVC) | 23 |
| UB automated bcc32 test | 8 |
| INT integration (two-process loopback / multi-workstation) | 10 |
| GUI GUI manual or scripted | 15 |
| DEV device | 5 (ARES 1, Emit 1, Sirit/Zebra 1, emiTag 1, timing-reader 1) |
| SQL SQL (local MySQL) | 3 |
| REL release package on a clean machine | 1 |
| ACC manual acceptance | 15 |
| total | 80 |

Matrix rows T1..T18 (section 5) are not repeated here; their open run-time items are the same as O58-O59 (T4), O60-O61 (T7), O65 (T10), O66 (T11), O76 (T17).

## 5. C-12 matrix T1..T18 restated (`D/ominaisuusvalitsimet.md` section 10; labels are matrix rows, not register T-xxx debt ids)

The matrix itself was never run (`ominaisuusvalitsimet.md:436`). "Covered today" = an existing asset exercises the row; assets are those of section 1.

| Row | Configuration (product, switches/settings) | Exercises | Related H / T | Test type | Covered today? |
|---|---|---|---|---|---|
| T1 | single station, no setting file; all products | defaults, silent missing cfg | (T-008) | UM (harness); GUI shipped programs secondary | partial: HkMaali run 0 and run 1 without cfg (RUN 2026-11-24, `E/c11/ajot-0-1/`, shipped program, firewall isolation) and HK fixture start-up parts (`T/fixtures/HkFixture.cpp`); relay start-up parts `VFixture.cpp`; no ViestiMaali, ViestiLuenta or GUI run |
| T2 | network: two stations UDP/TCP (`YHTEYSn=`), `KONETUNN`; all products | replication and protocol (C-2) | H-022, H-023, H-026, H-027, H-032, H-007, H-016 | INT | partial: single-process packet-injection tests (`T/core/HkRecvTests.cpp`, `VRecvTests.cpp`, `TcpClientTests.cpp`); no two-process run |
| T3 | emit read with `EMITFILE` test input (`EMIT_IN.DAT`); HkMaali, ViestiMaali, HkKisaWin | emit path without a device | H-145 (write side) | UM | no (the C-9 H-145 probe covers the card-store write failure, not the `EMITFILE` input) |
| T4 | RFID `SIRITn=TCP:` or Zebra, reconnect; HkMaali, ViestiMaali, GUIs | `LAJUNEN` code | H-159 | UM with TCP stub; DEV:Sirit/Zebra for the real reader | no; `TpNet.cpp` listener helper exists |
| T5 | RFID key in ViestiLuenta (`regnly[0]=3`) | reduced LUENTA build | H-176, T-052 | ACC (shipped program; harness has no LUENTA defines) | no |
| T6 | autofile with and without `AUTOFILE.LST`; XML export from stage 2 | `AUTOFILE`, `HTML=`, `XML=` | H-149, H-150, H-151, (H-147) | UM | yes, as probes outside the repo: C-9 `c9_juk_probes.cpp` (H-149 a-d Asan), `c9_hk_probes.cpp` (H-150 leak, H-151 Asan); RUN 2026-11-26; not in the repo suite; ViestiWin/HkKisaWin (bcc32) not covered |
| T7 | SQL mirror, `SQLjonoVarm.dat`; HkKisaWin, ViestiWin | MySQL writer thread | H-160, H-161, H-043, H-014, H-162, H-169 | SQL | no |
| T8 | HK stage change; `RADAT2.XML` missing or broken; HkMaali, HkKisaWin | stage-change path, `.LST` fallback | H-129, H-134, H-031 | UM (GUI secondary) | partial: C-8 H-129 probe (index count), `T/core` H-031 and H-023b tests (stage-change flags); H-134 parse-error branch not covered (Q110) |
| T9 | emiTag `TCPS` / HTTP, `ETGPRS`; not ViestiLuenta | emiTag input handling | H-055, H-166 | UM (input buffers); DEV:emiTag real | no |
| T10 | ViestiWin opens a competition with a long path | `cfgfilename[41]` overrun | H-171 | GUI | no (object-file layout read only) |
| T11 | relay console `X)ML` | `xmlvalinnat` | H-172 | ACC | no |
| T12 | settings editor round trip; HkKisaWin, ViestiWin | `AIKACOM=`, `AUTOJAKELU=` | H-173 | GUI | no (parser key list could be unit tested, not done) |
| T13 | broken `KilpSrj.xml` and old `KILP.SRJ`; all | series-file fallback | H-174 | UM | adjacent only: H-042 XML fixtures and H-002 legacy `.srj` fixtures (`T/data/juk`) read series files; the silent-fallback path is not tested |
| T14 | course dispersion print without `HAJONNAT.LST`; ViestiMaali, ViestiWin | `fclose(NULL)` | H-063 (T-020) | UM (`TpCoreJuk`) | no |
| T15 | `NREGNLY` 8 vs 32, `LUKIJA9`..`32`; relay | device numbering | T-024 | UM (relay 8) + GUI (ViestiWin 32) | no (TpCoreJuk is the 8-reader build; `tp_build_info` does not report `NREGNLY` [inferred]) |
| T16 | `KUNTOSUUNN`, MSVC vs bcc32 | `_BORLAND_` block in the shared core | H-081, Q60 | UB (bcc32 side), UM (MSVC side) | no |
| T17 | unshipped configurations `VALUKU=200`, `MAXOSUUSLUKU=250` | file and message sizes | T-051, Q31 | UM (needs the configuration built) | compile and link only (C-4: HkMaali `ReleaseVA200`, JukMaali `Release150os`, `E/c4/commands.txt`); no run; H-041a computes sizes at the shipped VALUKU 250 only |
| T18 | `KATKKIER0` | index -1 | H-036 | UM (`TpCoreJuk` + ASan) | no (Asan configuration exists and is built) |

Matrix priority stated in the source: T6, T8, T14, T18 first (harness rows), then T10, T11, T12 (shipped program) (`ominaisuusvalitsimet.md:462-463`).
Coverage count: yes 1 (T6, outside the repo suite), partial 4 (T1, T2, T8, T17), adjacent 1 (T13), no 12 (T3, T4, T5, T7, T9, T10, T11, T12, T14, T15, T16, T18).


## 6. Gaps: P1 and P2 findings without a reproducing asset

Source: `prioriteetti_nykyinen` in `E/c13/osaA0/havainnot.csv`: P1 4, P2 18 (22 findings); 62 findings have no priority yet (the 62 include the 11 with a harness or probe asset
listed below and the two high findings H-009 and H-014 with none). "Reproducing asset" = a test or probe in section 2 that shows the mechanism in production code.

Of the 22 P1/P2 findings, 7 have an asset (H-032 P1; H-090, H-091, H-093, H-129, H-149, H-150 P2: all except H-032 only as probes outside the repo) and 15 have none.

### 6.1 The 15 P1/P2 findings with NO reproducing asset, by test type needed

| Needed type | Finding (priority) | What the test would show | Prerequisite / existing helper |
|---|---|---|---|
| UM (MSVC harness, Asan) | H-061 (P1) | `luepaketti` uninitialised `pkglen` after a chunked binary packet (Q43) | packet fed in pieces through a loopback TCP or serial stub; `TpNet.cpp` helper; no `luepaketti` fixture |
| UM (Asan) | H-071 (P1) | FILESEND inner length `d.fl.len` not compared with the class length: `fwrite` reads past `inbuf` | receive-path injection as in H-032 tests (`addseur` / handler call); handler `tark_tiedosto` (`Tp/TpComY.cpp:3004-3037`); INT variant proposed in `muistiturvallisuus.md` 15 |
| UM (Asan) | H-072 (P1) | EXTRA packet `eralahto[d2] = d3` with network-controlled `d2` | same; HK handler `Hk/HkCom32.cpp:640-645` in TpCoreHk, relay `Juk/VCom.cpp:1617-1618` in TpCoreJuk |
| UM (Asan) | H-055 (P2) | `tulk_emiTag` index `P` unbounded into `controls[500]` | feed `tulk_emiTag` an `i >= 500` message; bcc32 GUI side needs UB (Q45, Q49); DEV:emiTag for real frames (Q56) |
| UM (Asan) | H-070 (P2) | `lue_FEIG` frame length -32896..32639 against `FEIGbuf[200]` | feed the reader buffer; DEV:FEIG for real frames |
| UM (Asan) | H-060 (P2) | `tall_ampsakko` case '1' index `paikat[k-1] = 0` without bound | message injection into `tall_ampsakko` (`Hk/HkSak.cpp:111`), `sak_laji` 2; DEV:Kurvinen for real frames |
| UM (Asan) | H-073 (P2) | `kirjemvaloki` unbounded `swprintf` into `wchar_t msg[400]` with ordinary data when `EMVALOKI` is on | a received EMITVA packet and the setting `EMVALOKI` |
| UM (+ DEV:ARES) | H-056 (P2) | `lue_ARES` loop does not advance on 41+ bytes without frame or NUL (100 % CPU, timing input stops) | reader-thread buffer fixture; frame questions need an ARES capture (Q44) |
| UM (TCP stub; DEV:Sirit/Zebra real) | H-159 (P2) | reconnect tests the `openConnection` result inverted | loopback stub server (Q96); `TpNet.cpp` |
| GUI | H-087 (P2) | `EmitHtmlOsuus` never leaves `print_CS` / `rastiva_CS` (GUI-only code, cannot be linked into the console harness) | ViestiWin bcc32 build, debugger or Application Verifier |
| GUI | H-107 (P2) | worker-thread GUI hooks dereference form pointers without NULL check (HkKisaWin) | GUI test VM |
| GUI | H-109 (P2) | Delete competitor with `dKilp = -1`: writes `kilparr[-1]` (HK and relay) | GUI (J7-4); a UM variant is conceivable for the core `PUTREC(-1)` but the check is missing in the form handler |
| GUI | H-110 (P2) | ViestiWin Lahtoajat button: `EnterCriticalSection(tall)` then `continue` without leave | GUI (J7-4) |
| GUI | H-171 (P2) | `wcsncpy(cfgfilename, ..., 120)` into 41 wchar_t at every ViestiWin competition open | ViestiWin bcc32, `.map` or debugger (Q107) |
| SQL | H-160 (P2) | SQL error in the MySQL write thread ends the whole program | local MySQL and HkKisaWin or ViestiWin (Q97) |

By type: UM 9 (H-061, H-071, H-072, H-055, H-070, H-060, H-073, H-056, H-159), GUI 5 (H-087, H-107, H-109, H-110, H-171), SQL 1 (H-160); UB 0 primary
(bcc32 is secondary for H-055, H-171, H-109); INT, DEV, REL, ACC are secondary variants only (H-071 and H-072 INT, DEV rows of section 4).

### 6.2 Other high findings without priority and without asset

H-009 (GUI), H-014 (SQL). Both are named as manual tests in `testaus.md` 6 and 20.4; rows O01 and O02.


<!-- machine check: P1/P2 22 (asset 7: H-032 H-090 H-091 H-093 H-129 H-149 H-150); no priority 62 (asset 11: H-007 H-008 H-022 H-023 H-024 H-025 H-026 H-027 H-031 H-040 H-128); koe != ei 27 -->

## 7. Notes and open questions

Notes
1. "Regression-ready AS IS" for harness tests means detection, not a green gate: after a correct fix the runner prints XPASS and exits 3, and its text says XPASS is "not an accepted result"
   (`T/run_core_tests.py:41`, `:668`). No regression mode exists. `testaus.md` 20.5 speaks of "XFAIL to PASS"; the code as read does not do that without a manifest or runner change.
2. The 24 defect cases mix three evidence strengths (`testaus.md` 14.3, 15.3): mechanism in production code with hand-set state (most), ASan-confirmed write (H-001, H-032b, H-041b2, H-041b4, H-149, H-151),
   shipped program (H-041 only). Mechanism-only rows (no effect in a shipped program): all except H-041 shipped 221.
3. Probe outputs outside the harness are not judged by any repo tool: `aja.py` copies print the TPRESULT lines; nobody records XFAIL or XPASS for them. Their runs are a one-off (C-6 and C-8 twice, C-9 five times).
4. Machine-base discrepancies to be aware of: (a) `havainnot.csv` `nayton_laji` for H-145, H-146, H-147, H-149, H-150, H-151 still says "static, source" although the C-9 step 2 probes exist; the `koe` column is correct.
   (b) H-078 and H-079 have `koe` = ei although C-5 probes exist (H-079's probe uses production `TextFl`; H-078's does not): the `koe` definition seems to count only harness-style probes. (c) `testaus.md:67` cites `Tests/doctest.h:49-51` for the
   bcc32 incompatibility, but those lines hold the version defines (47-49 in the file); the incompatibility probe itself has no stored result.
5. Macro values recorded by the C-10 and C-12 scripts for expression-defined macros (`E/c13/osaA0/makroartefaktit.csv`) were not used; sizes quoted here come from the C-11 results (`kilprecsize` 11 200, `combufsize` 204/244, derived 7330 vs 6918).
6. Nothing here was built or run; the "State" columns repeat earlier recorded results. File mtimes of the local binaries were read with `ls` only.
7. Project memory: pushing and PRs are banned for now; CI changes cannot be validated before that is lifted.

Open questions for the main session
- Q35 (v143 and x86 ASan on `windows-latest`), Q36, Q39, Q40, Q103: need a CI run or an online check; all open in `kysymykset.csv`.
- Decision 4 (XPASS = green or red CI) and decision 6 (`run.sh`) of `testaus.md` 12.1 are still open; the plan needs them.
- Is a probe-to-harness migration (C-6, C-8, C-9 probe files into `T/core/`) allowed, and is it a prerequisite for fixes of H-090, H-091, H-093, H-094, H-128, H-129, H-145..H-151?
- Which fix shape will be chosen for H-041 (buffer vs `valuku` limit) and H-032 (in `addseur` vs the UDP check)? The existing tests flip differently (section 2.1).
- H-009, H-014 and all GUI rows need a test VM and, for H-009, a production hook (decision 7); the rows of section 4 marked GUI need that decision.
- Who owns device protocol captures (ARES, Emit, emiTag, FEIG, Kurvinen, Sirit/Zebra)? No capture or simulator exists in the repo.
