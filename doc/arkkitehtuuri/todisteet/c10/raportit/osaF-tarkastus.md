# C-10 osa F / part F: independent review of K1-K15

Agent `independent-verifier` (Opus), 2026-11-26. Report saved by the main session from the
agent's hand-back (claims, evidence and labels unchanged; wording and layout condensed). Static reading only; one scratch PE
parser outside the repository. Claims K1-K15 are defined in the main session's request (summary
in `../../riippuvuudet.md`, C-10 section on review); E1-E3 are the reviewer's own findings.

Main corrections: K6 consequence is wrong (a failed SQL statement in the writer thread ends the
program, per RTL source); N-9 is wrong for HK; three new defects E1-E3.

## Summary

| Claim | Verdict | Products | Reachability | Severity | Overlap |
|---|---|---|---|---|---|
| K1 Sirit/Zebra reconnect | **confirmed**; for Sirit the thread ends whether the reconnect succeeds or fails | `LAJUNEN`: HkMaali, ViestiMaali, HkKisaWin, ViestiWin | `SIRITn=`/`ZEBRAn=` + TCP drop, reader reboot or manual reconnect | **high** | new (H-138 different) |
| K2 vanished serial reader | confirmed | all five | USB-serial unplugged | medium | new |
| K3 `i_pv` uninitialised | partly | all five | malformed E/W/P field without `:` | low | same function as H-055 |
| K4 `samatleimat` self-compare; ACK after dropped duplicate | confirmed, also relay | HK: HkMaali, HkKisaWin; relay: all three | same badge re-read on same reader, same codes, no card between | low | H-145 |
| K5 malformed UPDATEs | confirmed; `UnicodeString(char)` gives a character | HkKisaWin, ViestiWin | `SQLTALLENNUS` or export option 3 | medium; high when SQL used (K6) | H-014 |
| K6 mirror stops after one error | **refuted for the writer thread (program ends)**; partly true on the main thread | HkKisaWin, ViestiWin | any SQL error incl. server drop | **high when SQL is used** | H-014 |
| K7 relay ctor with `FormMySQL == NULL` | confirmed (ViestiWin only) | ViestiWin | `SQLSTART`+`SQLTALLENNUS`+connect | medium (see E2) | new |
| K8 `dbxmys.dll`/`libmysql.dll` not in ZIP | confirmed | release ZIP | any SQL use | medium | new |
| K9 shutdown deadlock | confirmed mechanism | HkKisaWin, ViestiWin | `ETGPRS` + fetch error during shutdown | low (KILP.DAT already closed) | new |
| K10 `vcledge`/`IndySystem` imports | confirmed | GUI | load time | low | T-005 |
| K11a shared FastMM, double free silent | partly | GUI | as H-149 | no change to H-149 | H-149 |
| K11b `_wfopen_s` exclusive | confirmed | GUI | always | info | Q92 |
| K12 switches outside project files | confirmed | all | compile time | low | – |
| K13 Q5 answer | partly: correct but incomplete | GUI | – | – | H-014 |
| K14 `gethttp` | confirmed with nuances | 4 (not ViestiLuenta) | `ETGPRS`/`ETSQL` | medium | H-157 |
| K15 MTR `pkg_alku` / Sirit ports | MTR confirmed, no practical effect; N-9 refuted for HK | MTR all; Sirit `LAJUNEN` | – | low/info | H-145 |

## Evidence per claim

**K1.** `ZebraReader::openConnection` returns 1 on failure (`Tp/ZebraReader.cpp:939-940`, `:946-947`), 0 on success (`:1001`). `openSirit2` returns 0 on success and most failures (`Tp/TpLaitteet.cpp:2242`, `:2262`, `:2329`), 1 only when `openlukijaTCP` fails in the non-reconnect branch (`:2253-2254`) → with `reconnect=true` always 0. Reconnect code `if (!...openConnection(r_no,true)) regnly_open[r_no]=0;` (`:1816`, `:1845`) ends the loop → "Häiriö..." (`:1872-1874`). Sirit: every reconnect attempt ends the thread (original 2015 source `50db2f8`, `git blame`). Zebra: retries every 5 s while unreachable, stops on the first successful reconnect. Reconnect runs on `!TCPyht_on` (`noreconnect=1` forced `:2220-2221`, `ZebraReader.cpp:925`) or user `reconnectSirit` (`cbHk/UnitAjanotto.cpp:1622`). No restart path: `start_regnly` returns early when `regnlystarted && ToimintaTila==2` (`:1898`); only `rem_regnly` resets it (`:2140`); Maalikello close calls it only if `regnly_open` (`cbTp/UnitMaalikello.cpp:235`), already 0. Logic [verified]; run-time effect needs a reader or TCP stub.

**K2.** `quesize` returns 32 on `ClearCommError` failure (`com/winCom.cpp:151-152`); `lue_*` clear the buffer (`TpLaitteet.cpp:1011-1017`, `:1175-1180`, `:1342-1347`, `:1644-1647`); loop `:1856-1870` polls forever, no reopen, no message; only the GUI progress bar stops (`cbHk/WinHk.cpp:1622-1624`, `ViestiWin/UnitMain.cpp:1324-1326`). Medium: Emit data stays on cards and can be re-read after restart.

**K3.** `ec_strtoaika` returns before writing `*i_pv` without `:` (`TpLaitteet.cpp:263-268`); callers add `24*TUNTI*i_pv` (`:367-368`, `:385-386`); the malformed field already gives time 0 with `i_pv` 0, so the defect is missing validation; garbage enlarges the error. Low.

**K4.** `em1->ctrltime[i] != em1->ctrltime[i]` at `Hk/HkIV.cpp:947-948` and `Juk/VIv.cpp:940-941`; duplicate path (`HkIV.cpp:1145-1147`, `VIv.cpp:1095-1098`) sets `vahvistus`, does not store. Same badge re-read on the same reader with same codes but different times and no other card between (e.g. loaned card on the same course) → second read silently dropped and ACKed. Side note: `ed_em[r_no]` `malloc`ed, never initialised (`HkIV.cpp:967-968`).

**K5.** [verified]: relay `vosanotot` UPDATE trailing comma before `WHERE` (`ViestiWin/UnitMySQL.cpp:531-545`), team name into int `joukkue` (`:538`; read uses `joukkuenimi` `:286`); relay `osuudet` `badge=` for `lisno` (`:484`), `gps=` unquoted (`:501`); HK `sukup=` unquoted (`cbHk/UnitMySQL.cpp:593`), `salisarja` (`:596`; schema `kilpsql.txt:30` `alisarja`), `gps=`/`qual=` unquoted (`:544-545`). `ustring.h:115` `UnicodeString(char src) { sprintf(L"%c", src); }`, `:117` WideChar → character: `sukup=M` → unknown column; NUL → `gps=,`. Relay `vert0` is a `memcmp` over the whole header (`Juk/vkilp.cpp:215-220`), incl. bytes after terminators; `joukknimi[31]` vs `varchar(15)` → for many teams every rewrite runs the malformed UPDATE [inferred].

**K6.** `ExecSQL` → `TDBXDynalinkCommand.CheckResult` → `RaiseError` → `TDBXContext.Error` → `raise TDBXError` (`source/data/dbx/Data.DBXDynalink.pas:1110-1113`, `Data.DBXCommon.pas:11062-11074`). No `try`/`catch` in either `UnitMySQL.cpp`, `kirjSQLTh`, `kirjKilpSQL`. `_beginthread` thread outermost handler RTL `ExcHandler` (`cpprtl/Source/threads/thread.c:97-105`); for 0x0EEDFADE global unwind then `__call_terminate()` (`cpprtl/Source/except/except.c:113-126`) → terminate/abort. One error in the writer thread ends HkKisaWin/ViestiWin without flushing stdio buffers. Triggers: apostrophe in a name (D4), K5 statements, dropped server. Report section 5 contradicts its D5. D5 holds on the main thread (export `cbHk/UnitExport.cpp:439`, `Button2Click`): VCL catches, `inSQLquery` stays incremented (E1). [verified] from RTL source; one run would confirm.

**K7.** `FormMySQL = new TFormMySQL(this)` (`ViestiWin/UnitMain.cpp:273-274`); ctor calls `Button1Click` (`ViestiWin/UnitMySQL.cpp:36-37`) before assignment; `Button2Click` → `kirjKilpSQL(0)` (`:63`) → `FormMySQL->CBVarmista` (`:676`) NULL → access violation inside `Initialisoi`; `CreateForm` commented out (`RADStudio10/ViestiWin.cpp:144-183`); VCL catches; rest of `Initialisoi` skipped incl. `ProsEmitThread` (`UnitMain.cpp:276-279`). HK assigns first, then `StartMySQL()` (`cbHk/WinHk.cpp:372-374`) — not affected by K7, but by E2.

**K8.** `collect-gui-runtime.ps1:101-140` static imports only; `release.yml:125-133` copies exes, `staging\gui`, LICENSE, README; `dbxmys.dll` via `LoadLibrary` (`Data.DBXDynalink.pas:933`; `Data.DbxMySql.pas:98`, `libmysql.dll` `:100`); none tracked. Manual: all files come with the delivery (`hkluku12.md:12-13`) and `kilpsql.txt` is in the distribution (`:41`) — both false for the ZIP. `libmysql.dll` is third party, not in Studio. Clean-machine failure [inferred].

**K9.** `FormCloseQuery` main thread → `lopetus1()` (`cbHk/WinHk.cpp:793-803`, `ViestiWin/UnitMain.cpp:986`) → `while(inhttpHaku) Sleep(200)` (`Hk/HkInit.cpp:3803-3804`, `Juk/vApu.cpp:1009-1010`); `vahaku` `writeerror_w(msg,2000,true)` (`HkIV.cpp:3550`, `VIv.cpp:2399`) → `SendMessage(FormMain->Handle, ...)` (`cbTp/cbTpApu.cpp:474`) → hang. `datf2` and emit file closed before the wait (`HkInit.cpp:3781-3790`) → hang at exit, no data loss: low.

**K10.** Own parser: both exes import `vcledge370.bpl` and `IndySystem370.bpl`; delay-load only WinRT string API set. `include/windows/vcl/SHDocVw.hpp:32-33` includes `Vcl.Edge.hpp`, `Winapi.WebView2.hpp`; no Edge/WebView2 use; Indy only unused includes `ViestiWin/UnitCfg.h:32-37`. `vcledge370.bpl` imports only OS + rtl/vcl → WebView2 not needed at load.

**K11a.** Exes import `CC32370MT.DLL`, `BORLNDMM.DLL` (`ViestiWin.cbproj:153`); RTL DLL build hook unconditional (`hrdir_b.c:120-135`); `_bormm_stub_FreeMem` discards return (`hrdir_bh.c:27-31`); freed small block flagged `IsFreeBlockFlag` → second free returns −1 (`getmem.inc:2771ff`, `:2954-2960`). Silent only if the block was not reused between the frees; H-149 writes after free and other threads allocate → second free may release a live block (silent corruption) [inferred].

**K11b.** `fopen_s.c:40` `_O_SECURE`; `__open.c:255-262` share 0 for write, `FILE_SHARE_READ` for read; `wfopen_s.c` = `#define _UNICODE` + `#include "fopen_s.c"`. Not refuted; shipped DLL = source [inferred].

**K12.** `com/WinUDP.cpp:21-23` defines `TCPSIIRTO`; `Hk/HkDef.h:53-55` defines `AUTOFILE` from it; `HkKisaWin.cbproj:180` lacks `AUTOFILE`; `makrot.csv` HkKisaWin (and ViestiLuenta) `AUTOFILE=1`; no `#if` on `EMITAG`. WinUDP self-define redundant in practice (`TCPLUKIJA` everywhere; `WinUDP.cpp:114`, `:331`). Low.

**K13.** Missing from the answer: (1) `lue_SQL` writes a deleted record (`kilpstatus=0xffff`) over every local record below `nrec` that has no SQL row and overwrites existing records with only the mirrored fields (`cbHk/UnitImport.cpp:1069-1080`) — SQL becomes the master destructively; (2) SQL rows feed the update diff (`readSQLrivi` → `vert0`/UPDATE); (3) schema `.txt` files not shipped (K8); (4) `UnitEmitSQL` separate read-only dependency on the emiTag server. Console part correct (`HkCons0.cpp:141-143`; `SQLDATA` nowhere).

**K14.** No status/header query in `gethttp.cpp`. Timeouts not set; WinHTTP defaults apply (connect 60 s, send/receive 30 s, resolve unlimited). Chunk check `*dwTot+dwSize < buflen` (`:97-101`) can skip a large chunk and append a later small one → **spliced line** (badge from one message, tail and time of another) parsed by `tulk_emiTag` → wrong split time possible; needs > 100 kB response (first poll from `ETTIME`, or after `msgno` reset). Any non-empty line ≤ 30 characters stalls polling for good (next request starts 10 before the last processed, `HkIV.cpp:3594-3602`); server behaviour unknown. Medium.

**K15.** MTR `pkg_alku` advanced regardless (`TpLaitteet.cpp:1247-1256`) [verified], but `tall_emit` returns 0 even on failed store (H-145) → nothing new. N-9 refuted for HK: `aloitus` sets all elements to 50008/50007 (`Hk/HkInit.cpp:3132-3140`) before `lue_parametrit` (`:3176`; GUI `cbHk/WinHk.cpp:279`); relay only element 0 (`Juk/VInit.cpp:1576-1580`) and only `regnly[0]` (`:1594-1622`) → no effect. Still true: port in `SIRITn=TCP:ip:port` overwritten by `SiritEventPort` (`TpLaitteet.cpp:2065`). Low.

## Extra findings

- **E1 (new, medium).** Export option 3 with `sqltallennus` off calls `kirjSQLrivi` on the main thread holding `tall_CriticalSection` (`cbHk/UnitExport.cpp:426-449`, `ViestiWin/UnitExport.cpp:411-430`); any SQL error throws; lock never released, `inSQLquery` stays 1; worker threads needing `tall_CriticalSection` block for good (incoming network records, emit processing); main thread keeps working (re-entrant lock) so the UI looks normal. [verified] lock flow; [inferred] which threads stall.
- **E2 (new, medium; high consequence with `SQLSTART`).** Server or `dbxmys.dll` missing at start → `SQLConnection1->Open()` throws inside `Initialisoi` (`cbHk/WinHk.cpp:372-374` → `cbHk/UnitMySQL.cpp:46`; relay via ctor); rest of `Initialisoi` skipped incl. `ProsEmitThread` (`WinHk.cpp:376-379`, `UnitMain.cpp:276-279`) and `setBrowserEmulation`. With K8, a release-ZIP user with `SQLSTART` gets no emit post-processing.
- **E3 (new, low).** `getwebPage` does not reset `pszOutBuffer` after `free` (`gethttp.cpp:76-109`); harmless only because `dwSize==0` ends the loop.
- `kirjSQLTh` uses the VCL from the worker thread (`FormMySQL->CBVarmista->Checked`, `EdtJono->Text`, `cbHk/UnitMySQL.cpp:655`, `:722-723`) — C-6 area.
- Spot checks correct: D N-8 condition always true; D list `:1584` omits 36; C `outfile` unreachable; C `GetLastError` after close; E 11-module set; B relay `kirjSQLTh` lacks `!vaiheenvaihto` (`ViestiWin/UnitMySQL.cpp:601`); A K12. Wrong detail: B "`#endif` at 705 closes the 693 block" — no `#if`/`#endif` between `HkInit.cpp:550` and `:977`; conclusion holds. D N-1 mechanism is statically [verified], only the run-time effect is inferred.

Open questions: (1) K1 test with a TCP stub; (2) K6/E1 test (drop MySQL server or `'` in a name with the mirror on): does the process abort, does `tall_CriticalSection` stay held; (3) K5 relay `vert0` after an SQL round trip; (4) K14 `get_stream.php` format, lines ≤ 30 characters; (5) K8/E2 clean-VM start-up from the release ZIP with `SQLSTART`; (6) K11a shipped `borlndmm.dll` asm `SysFreeMem` and reuse behaviour.
