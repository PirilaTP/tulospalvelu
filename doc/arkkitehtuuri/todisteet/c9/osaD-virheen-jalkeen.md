<!-- Provenance: final report of defect-concurrency-auditor (Sonnet), C-9 step 1 part D, 2026-11-26; copied unmodified from the hand-back message by the main session. -->
# C-9 step 1, part D: program state after an error report, and exit paths

Read-only audit. Sources: `TPsource/V52`, osaA `writeerror.csv`/`kutsut.csv`, osaA-luokittelu section 4.3. Tags: [verified: read] = I read the code and the chain holds; [inferred] = reasoned from the code but not run; [unknown] = cannot be settled statically.

## 1. Conclusion

1. Most `writeerror` sites are harmless. Of the 368 rows in Hk/Juk/Tp/com/tputilv2, about 85-90 % are message followed by return/continue/break with nothing half-applied [inferred, from ~55 sites read]. 111 rows are `stmt:` and 89 are `}`; I read about 40 of these. The true inconsistent-state sites are few (about 10-15 rows, see section 4).
2. The realistic harm is not silent wrong results. It is blocking: `writeerror_w(..., 0)` blocks the calling thread until the operator acknowledges, usually under a lock. In the console build it waits for Enter in whichever worker thread called it (section 6).
3. New confirmed items (N-D1..N-D8, section 5). None is "kriittinen". Candidates for `keskitaso`:
   - N-D1: a COMFILE read failure makes the sender transmit a stale packet.
   - N-D2: the exit choice in an error dialog hangs the GUI instead of exiting.
   - N-D3: the HK console `_exit` truncates an in-progress auto result file.
   - N-D5: an ignored `opencomfile` failure at stage change.
4. Q4 is resolved (section 2). Which `lopetus1` is linked:
   - HkMaali links `Hk/HkInit.cpp:3730`. It ends in `_exit(0)`.
   - ViestiMaali and ViestiLuenta link `Juk/vApu.cpp:944`. It ends in `exit(0)`.
   - HkKisaWin and ViestiWin link the same two functions without `_CONSOLE`, so there is no process exit. The window closes through the VCL.
   - `tputilv2/globals.cpp:33` is not linked anywhere.
5. D-5 is resolved. It is H-134, confirmed (section 2).
6. Already known; do not re-report (all [verified: read]):
   - H-031: refused stage change leaves `vaiheenvaihto = true`. I re-read `Hk/HkIx.cpp:1868-1881`; it matches H-031.
   - H-104: `inLopetus` preset makes `lopetus()` a no-op (D-32, D-33).
   - H-044: `buflah` advances `cjseur` after a failed write (D-7).
   - H-027: the `opencomTCP` operator precedence. `err = (openportTCP(..) && comtype == TCP)` makes `err` 0 or 1, so the retry loop of D-10 never loops.
   - H-117, H-009, H-015, T-032, T-033, T-020, H-134, H-105, H-095, H-038, T-030: applicable as noted.
   - B-14/B-15 `writeajat` advances `fp1` before `_write`. This is already in osaA as B-14/B-15, so it belongs to part B.

## 2. Resolved osaA questions

### Q4: which `lopetus`/`lopetus1` is linked into each program

| Program | Project evidence | `lopetus` | `lopetus1` | Process end |
|---|---|---|---|---|
| HkMaali.exe | `VS/Hk/HkMaali/HkMaali.vcxproj:118,135,154` define `_CONSOLE;EMIT;MAALI;...`; links `tputilv2.lib` | `Hk/HkInit.cpp:3720-3728`: `_CONSOLE` calls `lopetus1()` | `Hk/HkInit.cpp:3730-3830`, `_exit(0)` at :3830 | `_exit`: no atexit, no CRT stream flush |
| ViestiMaali.exe, ViestiLuenta.exe | `VS/V/JukMaali520.vcxproj:97+`, `JukLuenta520.vcxproj:95+` define `_CONSOLE`; Luenta adds `LUENTA` | `Juk/vApu.cpp:933-940` | `Juk/vApu.cpp:944-1036`, `exit(0)` at :1035 | `exit`: CRT flush and atexit/static destructors run on the calling thread |
| HkKisaWin.exe | `RADStudio10/HkKisaWin.cbproj` (GUI, no `_CONSOLE`) | `lopetus()` calls `SuljeKaikki()` (`cbHk/WinHk.cpp:1781-1787`) | the same `HkInit.cpp:3730` body; the `_exit` is `#ifdef _CONSOLE` so it is absent. It is called only from `FormCloseQuery` (`cbHk/WinHk.cpp:802`) | VCL ends when the main form closes |
| ViestiWin.exe | same | `Juk/vApu.cpp:936-938` and `ViestiWin/UnitMain.cpp:1699-1706` (`talltapahtumat()` then SendMessage) | the same `vApu.cpp` body, no `exit`. Called from `FormCloseQuery` (`ViestiWin/UnitMain.cpp:986`) | VCL |

- `tputilv2/globals.cpp` is in neither `VS/Libs/tputilv2.vcxproj` nor `RADStudio10/Tputil-XE.cbproj`. grep for "globals" finds only `Label="Globals"` and `HkGlobals.cpp`. `tputilv2/perr.cpp:21` (`extern void lopetus(void)`) therefore resolves to the program's own definition. D-28 is dead code and never linked [verified: grep].
- `exit`/`abort`/`ExitProcess`/`TerminateProcess`/`Application->Terminate`: none in the scanned .cpp. The only `exit`/`_exit` are `HkInit.cpp:3830`, `vApu.cpp:1035` and `globals.cpp:35` (unlinked), plus the unlinked `dbboxm/Boxtest.cpp` and `BOXTEST_F.cpp`. `_endthread` appears only in the test tools `cbTp/UnitEmiTag.cpp:220` and `UnitMTR.cpp:405` [verified: grep].
- Implicit exits not in the scan:
  - MSVC invalid-parameter abort (`fclose(NULL)`: H-063/T-020; `suljeajat` does `fclose(fafhandle)` unchecked at `Hk/HkAjat.cpp:402`).
  - Uncaught exception in a `_beginthread` thread (C-6/T-036 scope).

### D-5: `vaihdavaihe` after rata/leimat load failure (`Hk/HkIx.cpp:1936-1947`)

= H-134, confirmed [verified: read]. Order of events:

1. At `:1909-1914`, with `emitfl > 0`, `vapautaemitmuisti(...)` runs and `fclose(femitfile)` runs.
2. After `teejarj(true)`, `lue_radat_xml(7) == -1` leads to `writeerror_w(..., 0)` and `emitfl = 0` (:1938-1939). No fallback to the text rata/leimat files is tried.
3. `lueemitfile()` failure leads to a message and `emitfl = 0` (:1943-1945).
4. `emitfl` is set to 1 again only by startup parameter parsing (`Hk/HkInit.cpp:817-2728`, `:3303-3305`), not at run time [inferred: not every menu path was read].

Addition to H-134: `emitfl = 0` does not stop card intake. `tall_emit` (`Hk/HkIV.cpp:955`) has no `emitfl` test. `addem` calls `putem(em, emithead, 1)` (`Hk/HkEmit.cpp:1657-1671`, :1236), which does `_lseek/_write(emitfile)`. In this branch `femitfile` was closed (:1912) and `emitfile` and `emithead` were not reset (H-105). The reader threads keep running, so every card goes to a closed or reused fd. Relation: H-105. Whether the fd is reused by the `initajat` `_wopen` that follows is [unknown].

## 3. D-list verdicts (D-1..D-36)

| D | Site | Verdict | Evidence |
|---|---|---|---|
| D-1 | `tall_rivi` errret | Confirmed but mild. Both `errret = 1; goto poistu` paths run before any change (`HkAjat.cpp:190-198`, :230-238), so nothing is half-applied. Return value ignored: H-117. A third exit, `if (!aikajono[ino]->mxtime) return(1)` (:123), is silent and takes no lock | [verified: read] |
| D-2 | `tee_aika` | Refuted as inconsistent state. `tee_aika` pre-checks `rwtime - mxtime >= 0` (`HkIV.cpp:3755-3760`) before `tall_tulos`/`tallenna`, so the `tall_rivi` full-table branch cannot be reached (except by a race, [inferred]). `aika_tail = aika_head` (:3759) is a no-op: `aika_head`/`aika_tail` are used nowhere else. The dialog blocks the `comajanotto` thread (section 6) | [verified: read] |
| D-3 | `add` | Refuted. In both branches `exfl = 1; dataf = 0; break` and the function returns 0 (`HkEmit.cpp:1236-1256`). Caller: `HkIV.cpp:1460` `(d = add(&em)) == 0` leads to ESC | [verified: read] |
| D-4 | `teejarj` OOM | Real but OOM-only. HK `HkIx.cpp:1636-1650` writes `emitjarr[0][i] = -1` in the loop before the `if (!emitjarr[0])` test, so the test is ineffective: NULL deref before check. `emitjarr[1]` is never checked. Juk `VIx.cpp:1100-1112` has the same order. In the `er` branch (`HkIx.cpp:1612-1615`) `teejarj` returns 1 after `DeAllocJarjSeur`, and `vaihdavaihe` (:1930) ignores the result. Allocation sizes are 4*maxrec bytes, so the trigger is unrealistic. Severity matala | [verified: read] |
| D-5 | `vaihdavaihe` | = H-134, see section 2 | |
| D-6 | `lahetapaketti` | New finding N-D1 | |
| D-7 | `buflah` | = H-044. Also `buflah` is called under `outb_CS` by the callers, so the "koodi 1/2" dialogs (not guarded by `esto`) block under it (T-032) | |
| D-8 | `kirjaahyv` ack stamp | Low. A failed `_write` of the "sent" stamp (`TpComY.cpp:1592-1605`) is followed by `cjens++`. In memory the queue is consistent; on restart that record is sent again (duplicate, harmless). Matala | [verified: read] |
| D-9 | `luepaketti` every 10th error | Confirmed, medium-low: the dialog blocks one per-connection thread, see N-D7 / section 6 | `TpComY.cpp:853, 1074` |
| D-10 | open-failure loop | Mostly refuted. In `opencomTCP` the loop never repeats (H-027 precedence). The message is console-only (`#ifdef _CONSOLE`), so in the GUI the failure is silent. `opencomTCPin` shows one message and then sets `comopen = TRUE` regardless (H-027). Only `openlukijaTCP` repeats: N-D6 | `TpComY.cpp:1173-1236, 1933-1952, 2121-2189` |
| D-11 | Juk `putem` dialog | = B-11/T-032 | |
| D-12 | `autofile` per-series open | Refuted as "no message". `openprfile` itself calls `writeerror_w(L"Tiedoston avaaminen ei onnisunut", 0)` for the file branch (`tputilv2/Wrttext.cpp:171-176`). The `continue` (`HkTulostus.cpp:7217-7218, 7276-7278`) then skips that series, and its file stays at the old content. See N-D3 for the blocking consequence | [verified: read] |
| D-13 | `system(autokomento)` | Confirmed, low (details in Findings): `kirjoitettu` is set after `tulostasarja` regardless of success. The write-error test in `puts_f`/`writeline` only works for UTF-16 files, see D-14 | `HkTulostus.cpp:7341-7346` |
| D-14 | `puts_f` WEOF | Confirmed, and wider. `TextFl::WriteLine` returns `fputws()` for type `W` and `fputs()` (EOF = -1) for types 8/U/A/O (`TextFileClass.cpp:536-585`). The comparison `== WEOF` (65535) is therefore true only for UTF-16. Autofile uses `merkit` `A`/`8` (`HkTulostus.cpp:7096-7099`). So the "Virhe tiedostoon kirjoitettaessa" branches are effectively dead, and a failed write of a result file is never reported (part B overlap). The double-free in the `W` case is real code: `closeprfile(lstf)` (:1488) without clearing `lstf`, then `closeprfile(aftulparam.lstf)` (:7338/7344) frees it again. Reachable only with a UTF-16 target. matala | [verified: read] |
| D-15 | `kirjSQLTh` backup open | Part B/C. `varmistaSQLsiirto = false` without a message (`UnitMySQL.cpp:692`), so the queue then has no disk backup. GUI only | [verified: read] |
| D-16 | `malloc` then `memset` | Known as T-020 (`com/WinUDP.cpp:281-282`) | |
| D-17 | `reconnectTCP` | Refuted. `socket()` failure gives INVALID_SOCKET; `connect` returns SOCKET_ERROR; the message "epäonnistui" is shown and the next attempt comes after `RCWAIT` (`WinUDP.cpp:877-897`). Nothing is dereferenced. The NULL-deref at `:849` is H-027 | [verified: read] |
| D-18 | `wrt_ch`/`wrt_st` | Refuted as a state problem. The error value comes from `ClearCommError`. `dwWritten` is zeroed by `WriteFile`. Loss is handled by the ack/retry protocol (legacy RS-232 only) | `com/winCom.cpp:205-250` |
| D-19, D-20, D-21 | B-8, B-5, initajat | Part B. For D-21 (`HkIV.cpp:4122-4136`): `_wfdopen` is unchecked and followed by `setvbuf(fafhandle, ...)`, which is an invalid-parameter abort if it returns NULL (low) | |
| D-22 | `writeline` calloc | Low, OOM only: `calloc` NULL then `wcscpy` (`HkTulostus.cpp:1520-1521`) | |
| D-23, D-31 | `perr`/`wperr` "No" | N-D2 | |
| D-24 | GUI `writeerror_w` | See N-D2 and H-009/H-019/H-144 | |
| D-25 | console wait | Section 6 | |
| D-26 | HK `_exit` | Refined, see N-D3 and the exit table | |
| D-27 | relay `exit` | Confirmed, see the exit table | |
| D-28 | `globals.cpp` | Not linked | |
| D-29 | `_endthread` | U-phase test tools | |
| D-30 | dbbox2 | Dead (T-018) | |
| D-32, D-33 | `inLopetus = 1; lopetus()` | = H-104 | |
| D-34 | remote close | By design, with a 3 s warning. See the exit table | `HkCom32.cpp:652-664`, `VCom.cpp:1622-1634` |
| D-35, D-36 | startup | S-phase, not analysed | |

## 4. Examined sites (K-phase `stmt:` and `}` rows) and verdict

"Consistent" means the message is followed by return/continue/break without partial state change. Blocking dialogs are counted separately in section 6.

| Site | What follows | Verdict |
|---|---|---|
| `HkAjat.cpp:217,234` | goto poistu, nothing changed | consistent (D-1) |
| `HkIV.cpp:3748-3760` | return | consistent (D-2) |
| `HkIV.cpp:2770-2781` `tall_regnly` | return(1): the time point from the reader is dropped; `edaika[r_no][ino] = t` was already set. Dialog (0) in the reader thread | consistent, loud; H-090 covers the lock path |
| `HkEmit.cpp:1236-1256` `add` | exfl/dataf=0 | consistent |
| `HkEmit.cpp:2882-2892` `lue_radat` | `emitfl = 0; return(1)` (a flag, so a deliberate disable) | consistent, fail-stop with message |
| `HkEmit.cpp:2908, 2999` `lue_radat` | `goto esc1` on calloc NULL | OOM only; the NULL test at :2998 is placed after `memset` of an array member and is dead |
| `HkIV.cpp:3140`, `HkTulostus.cpp:4037` (`gaz_com`/`taulu_com = 0`) | return | consistent, deliberate disable |
| `HkIV.cpp:3084-3096` `seur_mtr`, `HkTulostus.cpp:6429` | return/goto | consistent |
| `HkIV.cpp:1578-1596` pros_emit unknown card (GUI `#ifndef _CONSOLE`) | `kno = haeseurvapaa()` can be 0 ("Ei vapaita kilpailijanumeroita"); the code still does `kilp.setId(0)` and `kilp.addtall(&d, 0)` before the `if (!kno)` test at :1611. `addtall` returns without setting `*dataf` when `nrec >= maxrec` (`HkDat.cpp:679-682`), leaving `d` as it was | latent, only when all numbers are taken; `tark_kilp` presets `dd = 0xffffffff` (`HkCom32.cpp:292-300`) and detects it, `add` pre-checks |
| `TpLaitteet.cpp:1546-1550` `lue_regnly` | `malloc` NULL, `return(-1)`, retried every 20-50 ms from the thread loop, dialog (0) each time | error loop, OOM only; low |
| `TpLaitteet.cpp:1874` (end of `lue_regnlyThread`) | message "...vaatii uudelleenkäynnistyksen" (0); the thread ends. `regnlystarted` stays 1 here, so `start_regnly` (`:1898`) refuses while `ToimintaTila == 2 \|\| regnly_open` | fail-stop with message; consistent |
| `TpComY.cpp:2273, 2354, 2369` `opencomfile` | N-D4, N-D5 | |
| `TpComY.cpp:2422` `initcom` | N-D5 | |
| `HkSak.cpp:224` | message and `inrec = 0` | consistent |
| `HkDat.cpp:779` `haeseurvapaa` | returns 0 and the caller proceeds (see the pros_emit row) | latent |
| `HkTulostus.cpp:7109-7114` | unlock and return(1) | consistent; the retry (50 s) repeats the dialog, see N-D3 |
| `HkTulostus.cpp:7217, 7276` | `continue` | stale file |
| `HkTulostus.cpp:7341` | system() runs anyway | low (D-13) |
| `TpComY.cpp:1133, 1154` `merk_uusinta` | message under `outb_CS` (T-032) | consistent |
| `Juk/vajat.cpp:318-350` | `writeaika` return; `writeajat` `continue` at a failed seek (fp1 not advanced = retried), `break` at a failed write (fp1 already advanced) | the write case is B-15 |
| `Juk/VEmit.cpp:1460-1466` | B-11 | |
| `Juk/vdat.cpp:632, 704, 885` | B (persistent) | |

Estimate: about 10-15 of the 368 rows leave state half-applied or run on stale state. These are `lahetapaketti` x3, `writeajat` (HK+Juk, part B), `pros_emit`/`haeseurvapaa` (1), `emitfl = 0` in vaihdavaihe and `lue_radat` (x4, deliberate), `opencomfile` (3), the `autofile` hook (1), `teejarj` (2). They are not evenly spread across the `stmt:` and `}` classes. The 111 `stmt:` rows mostly pair with a `delete`/`free`/`fclose` or a status-line draw. [inferred, sampled]

## 5. Candidate findings

### N-D1: COMFILE disk-queue read failure sends a stale packet and marks the record sent
- Mechanism: in `lahetapaketti` with `levypusk`, a failed `_lseeki64` or `_read` of the COMFILE slot only calls `writeerror_w`. `aktpak[cn] = cjens[cn]` is then set (`TpComY.cpp:1465-1477`), so the read is not retried. The static `obuf[cn]` still holds the previous packet, or a partly read new one. If `obuf[cn].pkgclass != 0`, the code falls through to the send at `:1490-1500`: `checksum = chksum(...)` is recomputed over the stale content, and `wrt_st_x` sends it with a new id as a valid packet. After the peer's ack, `kirjaahyv` stamps the record as sent and does `cjens++` (:1587-1599), so the original record is never sent.
- Evidence: `Tp/TpComY.cpp:1467-1477` (read), `:1481-1490` (empty-slot skip only if `pkgclass == 0`), `:1497-1503` (send), `:1575-1605`.
- Preconditions: levypusk (disk queue) enabled and a seek/read error on COMFILE.DAT. Realistic when the disk is full: H-044 shows `buflah` then advances `cjseur` although `_write`/extension failed, so the sender later reads past EOF, `_read` returns a short count, and "koodi 1" appears.
- Impact: the peer machine receives a duplicate of the previous record instead of the intended one (silent divergence between machines), plus one modal dialog under `outb_CS` (T-032).
- Severity: keskitaso (medium). Confidence: [paatelty] (chain read; no run).
- Relation: H-044 (write half); T-032. Verification: isolated probe with a truncated COMFILE.DAT (C-6/C-8 style) and a sniffed receiver.

### N-D2: the exit choice in an error dialog hangs the GUI instead of exiting; console exits mid-write
- Mechanism, GUI (HkKisaWin, ViestiWin):
  1. The two-argument `writeerror_w` (GUI) sends the dialog to the main thread and offers OK / "Poistu ohjelmasta" (`cbTp/cbTpApu.cpp:445-464`, `:480-490`; `FS->Kysy(vainok ? 1 : 2, ...)`). The `vainok = true` variants have no exit button.
  2. On "Poistu", the handler runs `suljesuoraan = 1; Close();` in the main thread (:460-461), while the caller thread is still blocked inside `SendMessage`.
  3. `Close()` leads to `FormCloseQuery` (`cbHk/WinHk.cpp:792-804`), which calls `talljarj()` (`Hk/HkApu.cpp:298-311`, `EnterCriticalSection(tall_CS)`) when `ToimintaTila == 2`, then `lopetus1()`.
  4. HK `lopetus1` calls `closecom()`, which ends with `EnterCriticalSection(&outb_CriticalSection)` (`TpComY.cpp:1921`); relay `lopetus1` enters `tall_CS` (`Juk/vApu.cpp:1000-1004`).
  5. If the thread that raised the dialog holds `outb_CS` (the `lahetapaketti` and `kirjaahyv` dialogs, `TpComY.cpp:1471-1485, 1602`) or `tall_CS`, the main thread waits for a lock that the worker holds while the worker waits for the SendMessage return: deadlock. The program neither closes nor continues.
- Mechanism, `perr`/`wperr` "No" (`tputilv2/perr.cpp:56-58, 92-94`): `MessageBox(NULL, ...)` runs in the calling thread (K-phase callers: `iowerr` in `HkAjat.cpp` x4, `HkEmit.cpp` x4, `HkIV.cpp` x5, `HkMuotoilu.cpp`; `dbbox1.cpp` x7 under `tall_CS`, H-015). "No" calls `lopetus()`.
  - GUI: `SuljeKaikki` = `SendMessage(FormMain, WM_MYMSGCLOSEAPPL)` (`WinHk.cpp:1781-1787`) leads to `CloseReqHandler` (:1741-1750) and the same `FormCloseQuery`/`talljarj`/`closecom` chain, so the same deadlock when the worker holds `tall_CS` (the KILP.DAT write dialog, H-015).
  - Console: `lopetus1` runs in the failing worker thread (the same thread continues after the dialog). In HK it ends with `_exit(0)`. The other threads keep running during the 2x `Sleep(200)` and the `closecom` sleeps (`Hk/HkInit.cpp:3730-3830`); `closefile(&datf2)` takes no lock (H-038).
- Impact: the operator's choice to leave after a write error does not leave. The data already written stays; the in-memory change that triggered the error is not saved (H-015). Mid-write: the failing call never returns in the console case; in the GUI the call is suspended forever.
- Severity: keskitaso (medium). Confidence: [paatelty]: each call link read, deadlock not run. Needs a probe: a thread holding `tall_CS`/`outb_CS` calling `writeerror_w`, with the main form answering "Poistu".
- Relation: H-009, H-015, H-104, H-038, T-030, T-032, T-033. Not a new lock analysis; it adds the exit button as a trigger.

### N-D3: HK console `_exit(0)` skips CRT flush; an in-progress `autofile` result is truncated, and the 2000 ms / Enter dialog stalls the background thread
- Mechanism (exit): `lopetus1` sets `taustaon = 0` and sleeps 200 ms (`HkInit.cpp:3743-3752`). It does not wait for `taustaThr`, does not take `autotul_CS` and does not close `aftulparam.lstf`. `_exit(0)` (:3830) then skips the flush of any open `FILE`. If `autofile` is mid-cycle (the file was opened with "w" and rewritten), the result file on disk is cut off. `err_file`, `lokitulparam`, `lsttulparam`, `femitfile` and `datf2` are closed earlier in the function.
- Relay `exit(0)` flushes all FILE streams, so the same race leaves a complete but possibly older state.
- Mechanism (stall): in the console build, `autofile` is called from `taustaThr` through `tausta` (`HkIV.cpp:3678`). When `openprfile` fails (locked file, bad path), `Wrttext.cpp:173` calls `writeerror_w(..., 0)` and `autofile` also calls `writeerror_w(msg, 2000)` (:7109-7114). The console version waits for Enter on the `taustaThr` thread, under `autotul_CS` (T-032). `tausta` stops: clock, `lahetakello`, `autotulostus`, `etHaku`, `lukumaarat`.
- Retry: `t_af = t_ta + 50*KSEK` (`HkIV.cpp:3678-3681`): the dialog comes back every 50 s.
- Impact: stale published results, or a truncated result file after Esc/`_exit`. The next autofile cycle after restart rewrites it. No loss of KILP.DAT data.
- Severity: matala-keskitaso. Confidence: [paatelty].
- Relation: H-038/T-030 (no join), T-032 (prompt under `autotul`), B-26. Needs: run `autofile` in a loop and kill via Esc.

### N-D4: "K)eskeytä ohjelman käynnistys" in `opencomfile` returns success
- Mechanism: `if (ch == L'K') return(1);` (console, `TpComY.cpp:2321-2322`) and `select3(...) == 2 -> return(1)` (GUI, :2325-2326). The function's success value is also 1 (`f_ok = 1` at :2371), and `initcom` (`:2404-2417`) uses `f_ok` to start `opencom`/`tarkcom`. The operator's abort is read as OK. `obfile` stays open on an incompatible COMFILE.DAT at an arbitrary file position, with `cjseur/cjens` zero.
- Impact: startup continues using the old incompatible file, and `buflah` overwrites records in it. S-phase.
- Severity: matala. Confidence: [todennettu] for the code; the effect is [inferred].

### N-D5: `opencomfile` failure is ignored at stage change; `initcom` misreports it
- Mechanism: `vaihdavaihe` calls `opencomfile(1)` (`Hk/HkIx.cpp:1975-1978`) and ignores the result. `opencomfile` first does `_close(obfile)` (`TpComY.cpp:2268-2269`) and `_wopen` may fail (file locked, read-only): `obfile` stays -1. After that every `buflah` takes the seek-fail branch (`pos = -1`) and shows the "koodi 2" dialog (kesto 0) per outgoing message (`TpComY.cpp:513-515`), then still does `cjseur[cn]++`. This is a dialog loop for as long as messages are produced. The call is also made under `tall`/`ajat`/`emit` (T-033) without `outb_CS`, so the close races the sender threads (C-6).
- Startup: `initcom` stores `f_ok` in a `static` set only on the first call (`:2404`). If it fails, `ok = 0` and the message is "Muisti ei riitä tiedonsiirtopuskurille" (`:2422`), which is misleading. All later connections also fail silently because `f_ok` stays 0.
- Severity: keskitaso-matala. Confidence: [paatelty].

### N-D6: `openlukijaTCP` shows a modal dialog on each retry, and with an uninitialised text
- Mechanism: for the cases 99, `WSAESHUTDOWN`, `WSAEALREADY`, `WSAEINPROGRESS` and `WSAEWOULDBLOCK`, `msg` is not assigned, yet `writeerror_w(msg, 2000)` is called (`TpComY.cpp:2121-2161`). The stack array `wchar_t msg[80]` is read for up to 200-512 wchars by `writeerror_w` (GUI `wcsncpy(..., MAX_MSG_BUF_SIZE)`). The dialog shows garbage text.
- Repeat: for the retryable errors the loop sleeps 5 s, then 55 s after 5 and 180 s after 10 tries (:2174-2185). So a disconnected reader shows a modal dialog on the main thread every few seconds at first. In the console, `writeerror_w(msg, 2000)` blocks the reader thread for 2 s per retry.
- Severity: matala (cosmetic/visible). Confidence: [todennettu] for the uninitialised read.
- Relation: memory-safety C-4 (read beyond the buffer) is out of scope; reported here only as the error-path text.

### N-D7: connection thread blocks in console on the "10 errors" and "queue full" dialogs
- Mechanism: the same thread both receives (`luepaketti`) and sends (`lahetapaketti`) for a connection (`tiedonsiirto` at `TpComY.cpp:1658-1790`, `tiedonsiirtoTCP` at :1257-1400). Every 10th bad packet calls `writeerror_w(tenerror, 0)` (:853, :1074). The console waits for Enter in that thread, so the connection stops receiving, sending and acknowledging until a key is pressed. In the GUI the thread waits on `perr_ptrs2.vast` (H-144).
- Impact: an unattended or noisy link stalls one connection. Peers see no ack and retry (`toisto`). No data change.
- Severity: matala-keskitaso. Confidence: [todennettu] for the call; the input contention with the main loop is [unknown].

### N-D8: `puts_f` dangling `lstf` after a UTF-16 write failure (D-14, narrowed)
- Narrowing of D-14: the failure branch is reachable only for `TextType == 'W'`, because the other encodings return `fputs()` results that never equal `WEOF`. So for the encodings autofile uses, a write failure to the result file is never reported at all (silent, part B), and the double-free exists only for UTF-16 targets.
- Severity: matala. Confidence: [todennettu].

## 6. K-phase threads that can wait for Enter (console `kesto == 0`)

Console `writeerror_w` calls `readkbd(&che, 1, 0)` after drawing on the bottom line (`tputilv2/Writeerr.cpp:80-86`). `writeerrorOn` and the saved screen line are globals and are not locked (two simultaneous messages leave stale text on the status line; cosmetic [inferred]). GUI: the calling thread polls `perr_ptrs2.vast` (H-009/H-019/H-144), with the same effect but a clickable dialog.

| Thread (console) | Function | `kesto == 0` call sites | Locks held at the call | Existing ref |
|---|---|---|---|---|
| main (keyboard loop) | `kbdiv1`, `pros_emit` (`HkEmit.cpp:3867`), `vaihdavaihe` | `HkIV.cpp:1208-1650, 1857-1863`, `HkIx.cpp:1938-1944` | `emit_CS`/`tall`/`ajat` in `vaihdavaihe` | T-033; interactive, intended |
| per-connection (`tiedonsiirto*`) | `luepaketti`, `lahetapaketti`, `kirjaahyv`, `merk_uusinta` | `TpComY.cpp:776, 853, 1074, 1133, 1154, 1471-1485, 1602` | `outb_CS` for 1133-1154, 1471-1485, 1602 | T-032, D-9 |
| any producer calling `buflah` (main, `tarkcom`, readers, `comajanotto`) | `buflah` | `TpComY.cpp:510, 514` (every call on failure), `:543` (queue full, guarded by `esto`) | `outb_CS` (callers) | H-044, T-032 |
| `tarkcom` | `tark_kilp`, `tark_alku`, `tark_*` | `HkCom32.cpp:255, 342-344, 838, 854, 961`; `Juk/VCom.cpp:593-749, 863-871` | `tall_CS` (`tark_kilp`) | H-016 note in osaA; T-032 |
| reader threads (`lue_regnlyThread`) | `lue_regnly`, `lue_MTR`, `tall_regnly`, `tall_emit` | `TpLaitteet.cpp:1201-1280, 1548, 1874, 2706, 2909`; `HkIV.cpp:970, 2649, 2779-2781` | `ajat_CS` conditionally (H-090) | H-090, T-032 |
| `comajanotto` | `tee_aika` | `HkIV.cpp:3748, 3756, 3758` | none (`tall_CS` only after) | D-2 |
| `taustaThr` | `tausta` then `autofile` | `Wrttext.cpp:173` via `HkTulostus.cpp:7217, 7276`, `:7113` (2000 ms) | `autotul_CS` | N-D3, T-032 |
| `pros_bdg_t`, `gazTh`, `vahaku` | emit/GAZ/HTTP | not examined | | [unknown] |

Reference: H-009 (GUI lock held across SendMessage), H-019 (global buffer), T-032 (blocking calls under locks). Whether a keystroke meant for the dialog in a worker thread is swallowed by the main loop's `readkbd` is [unknown].

## 7. Exit paths reachable during a competition

| Site | Reachable when | Closed/flushed before the exit | Lost | Recovered at restart? |
|---|---|---|---|---|
| HK console `lopetus1` + `_exit(0)` (`Hk/HkInit.cpp:3730-3830`, called from `lopetus()` :3720) | Esc in the main loop (`HkIV.cpp:4072`, but `inLopetus` set first = no-op, H-104); `tark_extra` case 9 on the `tarkcom` thread (`HkCom32.cpp:652-662`); `perr`/`wperr` "No" from any thread; D-35/D-36 at startup | `taustaon=0`, `Sleep(200)`; `lsttulparam.lstf`; `close_sakot` (stage > 2); `suljeajat(-1,true)` = `writeajat`, `_chsize`, `fclose(fafhandle)`, delete queues; `closecom` (up to 3.5 s sleeps, `stopUDP/TCP`, `closec`); `remtimer`; `femitfile` fclose; `closefile(&datf2)` without `tall_CS` (H-038); `lokitulparam.lstf`; loki TCP; `cleanWSA`; `Sleep(200)` | `autofile` in progress (N-D3); `autotul` held; threads not joined (T-030); memory outbuf (when `levypusk` off) = all unsent messages; `lahettamatta` is checked only at stage change, not at exit | KILP.DAT yes; AJAT yes (re-read, "K luetaanko aiemmat"); COMFILE yes if `levypusk` and the restart question is answered "S" (H-038); memory outbuf no |
| Relay console `lopetus1` + `exit(0)` (`Juk/vApu.cpp:944-1036`) | `tark_extra` case 9; `perr` "No"; menu | `scpclr`; `closetaulu`; `suljeajat` (not in `LUENTA`, :978-980); `regnly[i]=0`; `remtimer`; `closecom`; `close_profile`; `kirjEventXml("KilpSrj.xml")` (its failure message waits for Enter at exit); `closefile(&datf2)` under `tall_CS`; log; `cleanWSA`; CRT `exit` flushes every FILE | threads not joined; `exit` from a non-main thread runs CRT shutdown while other threads still run [inferred] | as above |
| HkKisaWin: `FormCloseQuery` (`cbHk/WinHk.cpp:792-804`) | window close; `CloseReqHandler` (network command, `writeerror_w` "Poistu", `perr` "No", D-32) | `talljarj()` (if `ToimintaTila == 2`, `!inLopetus`), then HK `lopetus1` (same body, no `_exit`); the process ends when VCL closes the main form | worker threads killed at process end; `closecom` etc. as above; deadlock cases in N-D2 | as above; `sqlVarmFl` is not closed (`UnitMySQL.cpp`), the CRT flushes it at process end [inferred] |
| ViestiWin: `FormCloseQuery` (`UnitMain.cpp:980-988`), `SuljeKaikki` (:1699-1706) | same | `talltapahtumat()` (writes the events file with "wt"); relay `lopetus1`; `Sleep(500)` | same | same |
| `tark_extra` case 9 (`HkCom32.cpp:652-662`, `VCom.cpp:1622-1634`) | any peer sends class x with `d1 == 9` and an empty or matching machine id; `lahetasulku` forwards it to the other machines first | 3 s warning (`writeerror_w(..., 3000)` console, `writewarning_w` GUI), then the exits above | the remote command closes the program with no confirmation (by design) | as above |
| `dbbox2.cpp:27-62` (D-30) | unreachable (T-018, dead) | | | |
| `PaivThread` `_endthread` (D-29) | U-phase test tools | | | |

`perr` "No" from a non-main thread: yes, reachable from any thread that calls `iowerr`/`ioerr` (K-phase: `writeajat`, `writeaika`, `putem`, `tall_rivi` callers, `dbbox1` write). Effect mid-write: see N-D2.

## 8. Refuted or narrowed osaA claims

- D-2: the "AIKATAULUKKO TAYSI" return does not skip `tall_rivi` in a state-changing way; `tee_aika` checks before any state change, and `aika_tail = aika_head` is a no-op.
- D-3: no state is lost; the return value is 0 and the caller handles it.
- D-10: not a continuous message loop for `opencomTCP` (precedence, H-027); `opencomTCPin` shows one message and goes on; only `openlukijaTCP` repeats.
- D-12: "no message" is wrong; `openprfile` reports through `writeerror_w(..., 0)`.
- D-17: no crash path in `reconnectTCP` from the unchecked `socket()`.
- D-18: not an inconsistent-state site.
- D-28: `globals.cpp` is not linked; `lopetus` cannot come from there.
- D-26: partly right (no flush), but the claim that `sqlVarmFl` matters for `_exit` is moot (GUI only); the real exposure is `aftulparam.lstf` (N-D3).
- osaA Q3 (what the caller of `addem` does with `res == 1`): all callers ignore the result (`HkCom32.cpp:796`, `HkIV.cpp:1150`, `cbHk/UnitLisaTietue.cpp:133`). A failed `putem` returns 0, the same as success. Before calling `addem`, `tall_emit` stores the card in `ed_em[r_no]` (`HkIV.cpp:1148-1150`), so a repeated read of the same card is treated as a confirmation (`vahvistus`) and is not re-added. Part B detail: the failed write is therefore not retried by a second read.

## 9. Open questions

1. Does a keystroke meant for a worker-thread `writeerror_w` dialog reach it, or does the main loop's `readkbd` consume it? [unknown]
2. In N-D2, which `writeerror_w` calls reach the GUI dialog with two buttons (non-`vainok`) while holding `outb_CS` or `tall_CS`? I verified `TpComY.cpp:1471-1485, 1602`; others not walked.
3. Whether `_wfdopen`/`_wopen` reuse the fd number of the closed `emitfile` in the H-134/H-105 branch [unknown].
4. Does a competition ever run with `levypusk == 0` (memory outbuf), so that unsent messages are lost at every exit? Config-dependent.
5. `pros_bdg_t`, `gazTh`, `vahaku` threads' `kesto == 0` calls were not examined.
6. Whether any menu re-enables `emitfl` at run time after the H-134 branch. I read only the parameter parsing sites.
7. Q1 stays open (`HkTV.cpp` not built; `opentv` rows are dead).

Key files: `C:\dev\tulospalvelu\TPsource\V52\Hk\HkInit.cpp`, `...\Juk\vApu.cpp`, `...\Hk\HkIx.cpp`, `...\Tp\TpComY.cpp`, `...\cbTp\cbTpApu.cpp`, `...\tputilv2\perr.cpp`, `...\tputilv2\Writeerr.cpp`, `...\tputilv2\Wrttext.cpp`, `...\cbHk\WinHk.cpp`, `...\ViestiWin\UnitMain.cpp`, `...\Hk\HkTulostus.cpp`, `...\Hk\HkIV.cpp`.
