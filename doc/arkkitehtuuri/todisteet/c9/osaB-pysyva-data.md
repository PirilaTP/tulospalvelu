<!-- Provenance: final report of defect-concurrency-auditor (Sonnet), C-9 step 1 part B, 2026-11-26; copied unmodified from the hand-back message by the main session. -->
# C-9 step 1, part B - persistent competition data, write-failure paths

Tags: [verified: read] = read in source or headers; [inferred] = reasoned, not run; [unknown] = not established.
Paths are relative to TPsource/V52/ unless they start with doc/. Nothing was run and no file was written.

## 0. Conclusion

1. **Emit file write failure loses the card in a way the caller cannot see.**
   - HK live `addem` (Hk/HkEmit.cpp:1657-1693) returns 0 when `putem` fails. `res` is 1 only for the duplicate-suppression case. osaA W11 ("`addem` returns 1") is wrong for the built code: the `res = putem(...)` variant at :1333-1355 sits inside `#if defined(EMIT) && defined(TALLEMIT)` (:1275-1377), and no `.vcxproj`/`.cbproj`/header defines `TALLEMIT`.
   - Relay `addem` (Juk/VEmit.cpp:1505-1544) does return 1, but its caller ignores it.
   - Both `tall_emit` variants store the card in the per-reader duplicate cache `ed_em[r_no]` before `addem` (Hk/HkIV.cpp:1149-1150, Juk/VIv.cpp:1100-1110).
   - They then return 0, so `Tp/TpLaitteet.cpp:911-917` sends the ACK (`kuittaus`) to the reader. The MTR path advances `pkg_alku` (:1247-1255).
   - The operator sees only a modal dialog (N-B1).
2. **AJAT queue rows are marked persisted before the write.** `writeajat` advances `fp1` before `_write` in both domains. After a failed write that chunk (normally one row) is never retried and a gap stays in the file (N-B2).
3. **B-29/B-30 are confirmed with a correction (Q2).**
   - Every `autofile` output is opened with type 'A' or '8', so the `== WEOF` test in `puts_f`/`writeline`/`writehtml` can never fire there.
   - The event log gets '8' through `trlate == 0`. The `lsttulparam` path also gets '8': `trlate[0]` is 0, set at Hk/HkInit.cpp:143, so `merk = 0` and the type defaults to '8'.
   - Only the manual list/export paths that pass the global `merkit` can be 'W', and only if `merkit == 'W'` (N-B3).
4. **`autofile` truncates the published file at open and has no error detection.**
   - A reader of the shared file sees an empty or partial file during generation.
   - A failed per-series open is silent. With `vainMuutt` (the "only changed series" option) the update is also dropped, because `fprior[srj]` is cleared before the open (N-B3).
5. **KILP.DAT.** H-015 is confirmed. The restart consequences are traced in section 3. New detail: relay `addfrec` ignores the result of its `openfile`, and a disk-full failure of the append breaks the live handle and loses the new team in memory too (N-B4).
6. **Durability.**
   - KILP.DAT is opened with `FILE_FLAG_WRITE_THROUGH`.
   - The emit file, AJAT queue, COMFILE queue and event log use `_write`/stdio with no `_commit`.
   - The `kirjheti` flush bits therefore protect against a process crash but not against power loss (N-B5).
7. Nothing in part B reaches severity "high" by the task's definition, because every disk failure on KILP.DAT, the emit file and the AJAT queue raises a modal dialog. The worst cases are medium: detected, with the loss unrecoverable and the dialog not saying so.

## 1. Thread and call-site table (as far as it decides data loss)

| writer | thread that runs it | caller treats result as | evidence |
|---|---|---|---|
| `putrec` (dbbox1) via `kilptietue::write` (HK) | any worker or UI thread, under `kilpdat_CriticalSection` | ignored; `write` returns 0 | dbbox1.cpp:84-100; Hk/HkDat.cpp:581-595, 609-624 [verified: read] |
| relay `putrec_i` | caller of `kilptietue::putrec(d, true)` | ignored | Juk/vdat.cpp:151-162; Juk/vkilp.cpp:239-250 [verified: read] |
| `addem`, HK | `tall_emit`: device read thread (Tp/TpLaitteet.cpp:911, :1248); `tark_emit`: `tarkcom` thread (Hk/HkCom32.cpp:796); GUI: UnitLisaTietue.cpp:133 (main thread) | ignored, except the `tark_emit` early return | Hk/HkIV.cpp:1150 [verified: read] |
| `addem`, relay | `tall_emit` (Juk/VIv.cpp:1110); `tarkcom` (Juk/VCom.cpp:828) | ignored | [verified: read] |
| `writeajat`, HK | `tall_rivi` (Hk/HkAjat.cpp:280, under `ajat_CriticalSection` unless `InCriticalSection`); `suljeajat` :389 | void | [verified: read] |
| `writeajat`, relay | `tall_rivi` (Juk/vajat.cpp:240); `timerfl` path (Juk/VIv.cpp:2507); `suljeajat` | void | [verified: read] |
| `writeaika` (both) | no caller in any scanned directory | - | grep in Hk, cbHk, HkConsole, Juk, ViestiWin, JukConsole, Tp: definitions only. It is dead code (B-13 refuted as a live path). |
| `autofile` | HK GUI: `tarkAutoTulostus` inside `LukumaaratTh`, a worker thread (cbHk/WinHk.cpp:1801-1852). HK console: main loop (Hk/HkIV.cpp:3678). Relay: ViestiWin/UnitMain.cpp:520; Juk/VIv.cpp:2443. Also Hk/HkCom32.cpp:981; Tp/TpComY.cpp:2702-2704; HkConsole/HkCons3.cpp:1986 | the return value only moves the retry time (+50 s on a non-zero return) | [verified: read] |
| `kirjSQLTh` | its own `_beginthread` thread | `fseek`/`fread` results unused; `fwrite` ignored | cbHk/UnitMySQL.cpp:645-728 [verified: read] |
| `tark_tiedosto` | `tarkcom` thread | `fwrite`/`fclose` results unused | Tp/TpComY.cpp:3000-3036 [verified: read] |

## 2. Per-file table: questions (1)-(5)

(1) detected, (2) operator told, (3) retry or later recovery, (4) silent loss or memory/disk divergence, (5) partial-write corruption.

### 2.1 KILP.DAT, both domains

| Q | Answer |
|---|---|
| 1 | Partly. `putrec` tests `SetFilePointer` and the byte count `ls < recl`, and `WriteFile` zeroes `ls` on failure (dbbox1.cpp:84-100). Every caller drops the -1 return (H-015). `closefile` and `CloseHandle` results are also ignored (:214-224). |
| 2 | Modal `MessageBoxA(NULL, ..., MB_YESNO)` "Continue execution?" per failing write (tputilv2/perr.cpp:28-60). It blocks the writing thread, here inside `kilpdat_CriticalSection`. "No" calls `lopetus()`. One dialog per failing write: a dropped share means one dialog per record update. |
| 3 | There is no retry and no periodic rewrite. A later `tallenna` of the same record rewrites that record, which heals only records that change again. A manual full rewrite from memory exists: export menu "DAT" `kirjKilpDat` (cbHk/UnitExport.cpp:458-485, Hk/HkDat.cpp:1427-1446), which reads `kilparr` through `GETREC`. For a dropped network handle it is likely that only a restart or reopen gives a working handle [inferred]. |
| 4 | Yes. Memory is authoritative while the program runs (`PUTREC` writes `kilparr[d]` first, Hk/HkDat.cpp:616). Disk is read only at startup (`kilp.read`, Hk/HkIx.cpp:1668; relay `getrec_i`, Juk/VIx.cpp:1134). A restart therefore loads the old record. A failed `deleterec` write leaves the deleted competitor undeleted on disk (resurrection after restart) [inferred from :262-270 and `delfrec`, Hk/HkDat.cpp:640-651]. |
| 5 | `newrec` does `numrec++` first (:227-232), then `putrec`, then `updatedescr` (:246-250). Case A, disk full with the header still writable: the file stays N records long but the header says N+1. `openfile` then rejects it ("otsikkoalue virheellinen", :195-200), so startup is refused (H-015, C-3). Case B, a torn last record: the same rejection, because `reclen * numrec == flen` fails. Case C, crash between the record write and the header write: tolerated by `int2 == numrec - 1` (:195-196). `deleterec` updates no header (:262-270). `closefile` rewrites the header (:214-224), so a failed header write at exit matters only after deletes or `addrec0`. There is no checksum, so a torn record in the middle of the file is not detected. |

### 2.2 Emit file (HK `femitfile`/`emitfile`, relay the same)

| Q | Answer |
|---|---|
| 1 | `putem` tests `_lseek < 0` and `_write < sizeof` (Hk/HkEmit.cpp:1607-1615; Juk/VEmit.cpp:1459-1468). `lueem` ignores `_read` (H-053). |
| 2 | HK: `iowerr` -> `wperr` modal MessageBox under `emit_CriticalSection` (`addem` holds it, :1665). The code shown is the operation code, not the OS error (N-B6). Relay: `writeerror_w(..., 0)` blocks until acknowledged, inside `emit_CriticalSection` (T-032, H-009; in the GUI relay `writeerror_w` is a SendMessage). |
| 3 | There is no retry. HK `addem` leaves `emithead` and the card index unchanged and calls no `laheta_em`, so peers never receive the card either. The next successful `addem` reuses the same slot, so a torn write heals itself in the file. Re-reading the card at the same reader is suppressed by the `ed_em` cache until a different card has been read in between. With `estaemittoisto`, `on_emit` has already registered the card (HkEmit.cpp:1666, Juk/VEmit.cpp:1511) and a re-read returns res=1 (duplicate) (N-B1). |
| 4 | Yes, see N-B1. The reader is told the card was received. A restart does not see the card in the file. |
| 5 | A short write is not a problem at restart: `emithead = filelength / sizeof(emittp)` (HkEmit.cpp:3060) truncates the partial record, and the slot is overwritten by the next write. `putem` for an in-place update (`tallenna`, H-011) failing leaves memory and file different. |

### 2.3 AJAT queue (`AJATn.LST`)

| Q | Answer |
|---|---|
| 1 | HK `writeajat`: `_lseek` and `_write` checked (Hk/HkAjat.cpp:361-375). Relay: the same (Juk/vajat.cpp:338-352). |
| 2 | HK: `iowerr` modal, under `ajat_CriticalSection` when called from `tall_rivi`. Relay: `writeerror_w`. |
| 3 | After a seek failure `fp1` is untouched, so the next call retries (HK `return`s from the whole function, so the other queues are skipped too). After a `_write` failure there is no retry (N-B2). `fp1` is lowered only when an earlier row is inserted or changed (Hk/HkAjat.cpp:208-209, :274-275), which rewrites from that row onwards. `initajat` at a stage change rewrites the whole file from memory (Hk/HkIV.cpp:4111-4127). |
| 4 | Yes. Memory holds all rows. After a restart the missing rows are absent, and the gap rows may load as time-0 rows [inferred]. |
| 5 | The file is overwritten in place, and a failed `initajat` rewrite leaves the old content after the failing row. The gap zero-fill is created by later writes landing beyond EOF [inferred]. |

### 2.4 COMFILE disk queue (`COMFILE*.DAT`, replication to peers, not primary data)

| Q | Answer |
|---|---|
| 1 | `buflah` and `lahetapaketti` test `_write`/`_read` sizes (Tp/TpComY.cpp:509, 1470, 1484, 1601). The `calloc` result is unchecked (T-020). |
| 2 | `writeerror_w` message only. `lahetapaketti` and `kirjaahyv` show it under `outb_CriticalSection` (T-032). |
| 3 | There is no retry. The queue pointers advance (`cjseur[cn]++` :516, `cjens[cn]++` :1487, :1604). This is H-044. |
| 4 | Peers diverge. The origin station keeps the result in KILP.DAT and the AJAT queue. |
| 5 | A slot can be left half written. The restart gap is in H-044. |

Not new, apart from the mapping to the question.

### 2.5 SQL backup queue (`SQLjonoVarm.dat`) and SQL transfer

| Q | Answer |
|---|---|
| 1 | No. `er` from `fseek`/`fread` is never tested; `fwrite`/`fflush` are ignored (cbHk/UnitMySQL.cpp:665-719). |
| 2 | No message. An open failure sets `varmistaSQLsiirto = false` silently (:691-692). |
| 3 | The queue is in memory and holds only record numbers `d`; the data comes from `kilparr` at send time. If the backup file is unusable the queue is lost at restart. |
| 4 | The ring has 10000 entries. On overflow `kirjKilpSQL` advances `tailSQLjono` and drops the oldest entry silently (:747-749). `tailSQLjono` is advanced after `kirjSQLrivi` without a success result (:698-701, H-014). |
| 5 | H-043. |

SQL is a mirror, not the primary store. Relation: extends H-014 and H-043 only with the overflow drop (low).

### 2.6 Event log (`loki`), runtime error log (`DATA_ERR.TXT`), debug files

| Q | Answer |
|---|---|
| 1 | No. `wkirjloki` ignores the `sendwline` result and the `wrt_st_TCP` `nsent` (Hk/HkApu.cpp:576-586). `kirj_err_file` ignores `fputws` (Tp/TpApu.cpp:878-882). |
| 2 | No. |
| 3 | No. |
| 4 | Log lines are lost on write failure. They are also unflushed: the log is a `TextFl` (`openprfile(lokifname, -1, FALSE, 1, 0, immed)`, Hk/HkInit.cpp:3399), no `Flush()` is called for it, and the `immed` flag affects only the raw-FILE branch (Wrttext.cpp:364, 393, 422; the comment at Juk/VInit.cpp:971 says "(ei Textfl)"). A crash loses up to about 4 KB of log. `DATA_ERR.TXT` is closed only in `close_err_file`. |
| 5 | No. |

Diagnostics only. Severity low (N-B8).

### 2.7 Result files, auto output files, XML output

| Q | Answer |
|---|---|
| 1 | The `== WEOF` test in `puts_f`/`writeline` (Hk/HkTulostus.cpp:1484, 1526) and the relay equivalents does not detect failures on the auto path (N-B3). XML writers ignore the `WriteLine` return (W28). `sendline`/`sendwline` on `prno == -2` return the `WriteLine` value, and `wkirjloki` and others ignore it (Wrttext.cpp:353-355, 386-388). `~TextFl`/`Close` ignore `fclose` (TextFileClass.cpp:312-317, 367-372). |
| 2 | Only if the 'W' type is in use. `autofile` shows a message when the main open fails (:7110-7116, `writeerror_w(msg, 2000)`) and is silent for per-series opens (:7217-7218, 7276-7278). |
| 3 | The next `autofile` cycle (interval `afvali`) regenerates the file. The exception is `vainMuutt`, see N-B3. |
| 4 | Yes. A truncated or partial published file stays until the next successful run. |
| 5 | Truncation at open: `openprfile(..., append=2)` selects mode "w+t" (Wrttext.cpp:166). In `TextFl::TextFl` the `+` sets `NewFile` when `ReadFl` is false, giving `w+b` (TextFileClass.cpp:252-256, 291-292). The file is emptied at once. The content is written through a 4 KB stdio buffer with no flush and no temporary-file rename. |

### 2.8 Stage-selection file `vaihevalinta.cfg`

| Q | Answer |
|---|---|
| 1-2 | No (Hk/HkInit.cpp:314-325). |
| 3 | Rewritten at the next stage change. |
| 4 | See N-B9. |
| 5 | Truncated with "wt" before the `WriteLine`. An empty file reads back as stage 1 (`i_pv = 0`, :293-311). Only with the `vaihevalinta` option (Hk/HkInit.cpp:705, :3188-3189, Hk/HkIx.cpp:1997-1998). |

## 3. Candidate findings

### N-B1. Emit write failure: card ACKed to the reader, cached as read, not retried (HK and relay)

- **Mechanism.**
  1. `tall_emit` stores `em` in `ed_em[r_no]`, then calls `addem` and returns 0 whatever `addem` did (Hk/HkIV.cpp:1148-1152; Juk/VIv.cpp:1099-1113).
  2. HK `addem` returns 0 when `putem` fails (Hk/HkEmit.cpp:1669-1693). Relay returns 1 (Juk/VEmit.cpp:1514), but nobody uses it.
  3. On failure `emithead++`, `setEmitJarr`, `laheta_em` and the analysis are skipped.
  4. `Tp/TpLaitteet.cpp:911-917` sends `kuittaus` to the reader because `er == 0`. The MTR path advances `pkg_alku` (:1247-1255).
  5. A second read of the same card hits `samatleimat` and only sets `vahvistus` (Hk/HkIV.cpp:1145-1147). `on_emit` also records the card before the write (Hk/HkEmit.cpp:1666).
- **Preconditions.** The emit file write fails: disk full, a dropped share (emit file on a network path), or a locked file. EMIT is enabled.
- **Impact.** One or more cards are not in the emit file or the index, and no peer receives them. The competitor's emit result is missing. The operator saw one modal dialog per failed write, and none says that the card was lost. Physical recovery exists: re-dip the card after another card was read [inferred], or `LisaTietue` (cbHk/UnitLisaTietue.cpp:133) for a manual entry.
- **Severity.** Medium (upper end). Raise to high if the unattended case (dialog left open on a reader thread) is judged realistic: the reader thread is blocked in the MessageBox, and incoming data waits in the port buffer [inferred].
- **Confidence.** [verified: read] for the code path. The runtime effect of ACK on the reader is [inferred] and depends on the reader type.
- **Verify.** An isolated probe: call `tall_emit` with `emitfile` closed, or `_chsize`/quota to force `_write` failure, and observe the return and the ACK.
- **Relation.** New. Refutes the osaA W11 claim. Related: H-011, H-045, H-105, H-053, T-032.

### N-B2. AJAT `writeajat` advances `fp1` before the write, so a failed row is never retried

- **Mechanism.** `gettm(aikajono[ino]->fp1++, abuf+r, ino)` runs before `_write`, and the loop breaks on failure (Hk/HkAjat.cpp:365-376). Relay: `gettm(...fp1++...)` at Juk/vajat.cpp:347, `_write` at :349-352, then `fp = fp1` regardless (:356). The next call starts at the new `fp1`, and with n=1 per timing event the failed row is simply skipped. Later successful writes seek past the gap.
- **Preconditions.** A `_write` failure on the AJAT file. A dropped share affects every subsequent row until the handle is repaired.
- **Impact.** Timing rows exist in memory only. A restart loses them, and the gap can load as zero rows [inferred]. Recovery routes exist only if a row earlier in the queue is edited (`fp1` lowered) or the stage changes. One dialog per failing write, under the lock in `tall_rivi`.
- **Severity.** Medium. The AJAT queue holds the raw split times. Detected with a message, but not recoverable without manual action.
- **Confidence.** [verified: read] for the order of operations. The restart behaviour is [inferred] (the read loop is at Hk/HkIV.cpp:4077-4101).
- **Verify.** A unit probe against an `aikajono` with an `afhandle` that fails, then call `writeajat` twice with the handle fixed.
- **Relation.** Extends H-054 (osaA W15/W16 candidate confirmed with the correction that `writeaika` is dead). The relay has `fl = rwtime - fp1` and HK `rwtime - fp1 + 1` (Hk/HkAjat.cpp:356, Juk/vajat.cpp:336); the difference was not analysed.

### N-B3. `autofile`: truncate in place, WEOF test ineffective, silent per-series failures

- **Mechanism.**
  1. The main file is opened with "w+t" (Hk/HkTulostus.cpp:7108, Juk/VTulostus.cpp:7166), which truncates immediately. `_wfopen_s` uses the default share mode, so concurrent readers open the file and see the empty or partial content for the whole generation time.
  2. The write check is `WriteLine(...) == WEOF` (Hk/HkTulostus.cpp:1484, 1526). `TextFl::WriteLine` returns the `fputs` result for types '8','U','A','O' (TextFileClass.cpp:575): EOF = -1 on error. Only the 'W' branch returns the `fputws` result (:546). `WEOF` is `(wint_t)0xFFFF`, with `wint_t` an `unsigned short` in the UCRT (Windows Kits 10.0.26100.0, ucrt/corecrt_wctype.h:21, corecrt.h:605), and in C++Builder 37.0 `WEOF` is `(_WINT_T)(0xFFFF)` with `wint_t` a `wchar_t` typedef (include/windows/crtl/stdio.h:88, `_stddef.h:140`). It cannot equal -1 in either. The actual `fputs` return value on error in the bcc32 RTL was not run [inferred, EOF = -1 per stdio.h:86].
  3. Callers of the file path and the type they give (grep of all `openprfile` calls): `autofile` passes `aftulparam.merkit`, 'A' or '8' (Hk/HkTulostus.cpp:7102-7105, :7183). Console list/export (HkCons3.cpp:154-161) gives 'A' or '8'; JukConsole/VConTul.cpp:334-341 'A' or 'U'. `lsttulparam`/`lokitulparam` give `trlate`/0, which becomes '8'. `cbHk/HkKisaLahtLuet.cpp:599`, `avaa_tiedosto_nm` (Hk/HkTulostus.cpp:7915) and Juk/VTulostus.cpp:8391 pass the global `merkit`. That global becomes 'W' at Hk/HkInit.cpp:3433-3434 and Juk/VInit.cpp:2707-2708 when the list file is not a printer, and can be set to 'W' by `MERKIT=` or the menu (Hk/HkInit.cpp:961-962, :2966-2967). So only those manual paths can be UTF-16 and have working detection [verified: read for the call sites; [inferred] for what value the global holds at run time].
  4. `~TextFl` ignores `fclose`, so a flush failure at close is never seen (TextFileClass.cpp:312-317).
  5. A per-series `openprfile` failure just `continue`s (Hk/HkTulostus.cpp:7217-7218, 7276-7278). `fprior[srj] = 0` was already executed at :7266, so with `vainMuutt` the series is skipped from then on (:7264) until it changes again. The old file stays published.
  6. `system(autokomento)` runs when `kirjoitettu` is true (:7341-7346), even if a write failed undetected. It does not run if all series failed to open (`kirjoitettu` false).
  7. If an earlier `aftulparam.lstf` is still non-NULL, `autofile` returns 1 at once (:7093-7094) and nothing is written.
- **Preconditions.** Disk full, network share dropped, or the target locked by another process (an exclusive-share reader, an FTP client). A live-results web page or a shared folder reading the file.
- **Impact.** The published result file can be empty, partial or stale. The operator sees nothing, or one message for a failed main open. The next cycle fixes it, except the `vainMuutt` case.
- **Severity.** Medium for the stale/partial published results (no data in KILP.DAT is lost). The truncation window itself is low to medium, depending on the generation time (Q6 in osaA is open).
- **Confidence.** [verified: read] for the code, truncation, and header definitions. [inferred] for the reader seeing a truncated file (share mode default) and for the bcc32 `fputs` return.
- **Verify.** Run `autofile` on a large result set while a second process polls the file size. Force a failing `fputs` (full volume) and check the `WriteLine` return.
- **Relation.** New (B-26, B-29, B-30 confirmed and refined). D-14 (`closeprfile(lstf)` in `puts_f` leaving `this->lstf` dangling) is reachable only with 'W' (list path), not in `autofile`.

### N-B4. Relay `addfrec`: unchecked reopen; disk-full append breaks the live file handle and drops the new team

- **Mechanism.** After `addrec`, `filelen != l` holds for every appended record. Then `closefile(&datf2)` (header write, `CloseHandle`) and `openfile(&datf2, ...)` run with the result unchecked (Juk/vdat.cpp:185-189), and the lock is taken twice (:183, :190, H-040).
  - Disk full: the record write (:185 -> dbbox1.cpp:246-250) writes 0 bytes while `numrec` is N+1 and the header (an overwrite of an existing sector) may succeed. `openfile` then reads `numrec = flen/reclen = N`, `int2 = N+1`, fails the header check (dbbox1.cpp:195-200), runs `CloseHandle`, shows "otsikkoalue virheellinen" and returns 1. `datf2.hDatf` is a closed handle, `datf2.numrec` is N, and `PUTREC` requires `d < datf2.numrec` (Juk/vdat.cpp:173), so `PUTREC(kilp, *d)` (:191) does nothing: the new team is not in memory either. Every later KILP.DAT write fails with dialogs.
  - Transient lock or a dropped share at reopen: `CreateFile` fails, `hDatf == INVALID_HANDLE_VALUE`, `numrec` stays N+1, memory is fine, and every later write fails with a dialog until restart.
- **Preconditions.** The append fails (disk full) or the reopen fails (share drop, sharing violation in the window between close and open). Adds happen mostly at registration but also for late entries.
- **Impact.** In the disk-full case the entry is lost silently apart from a misleading message. In both cases KILP.DAT is dead for the rest of the session (dialog per write), and memory/disk diverge as in 2.1.
- **Severity.** Medium. Confidence [inferred]: the chain is read in source, but the disk-full behaviour of `WriteFile` (0 bytes, header still writable) is assumed.
- **Verify.** A probe with a quota-limited volume or a handle closed mid-call: call `addfrec` equivalent (`addrec` + `closefile` + `openfile`) on a datafile.
- **Relation.** Extends H-015, H-040, H-047. The unchecked `openfile` is also at Hk/HkDat.cpp:1435, Hk/HkInit.cpp:3480, Juk/VInit.cpp:2741 and in import/export (osaA S2).

### N-B5. No synchronous flush for the emit file, AJAT queue, COMFILE queue and event log

- **Mechanism.** KILP.DAT is opened with `FILE_FLAG_WRITE_THROUGH` (dbbox1.cpp:104-130 and `openfile`). The others use `_write` on descriptors with no `_commit` (grep: none in the scanned code), and `fflush(femitfile)` on the unbuffered stream (`setvbuf(..., _IONBF, 0)`, Hk/HkEmit.cpp:3058) writes nothing and does not commit to disk. `kirjheti & 4/8` and `FlushFileBuffers` (only Juk/vdat.cpp:157, KILP.DAT) therefore give no durability guarantee for these files.
- **Preconditions.** Power loss or an OS crash, for example a generator-powered event.
- **Impact.** The last seconds of emit and AJAT records can be missing after a restart while KILP.DAT is current. A process crash is not affected.
- **Severity.** Medium. Confidence [inferred] (the OS cache behaviour is standard Windows behaviour, not run).
- **Relation.** New. Not in H-IDs.

### N-B6. `iowerr`/`ioerr` show the operation code as the Windows error

- **Mechanism.** `iowerr(fname, oper, iores)` calls `wperr(fname, iores, L"iowerr", oper)` (tputilv2/ioerr.cpp). `wperr` formats `FormatMessage(dwError)` where `dwError = oper` (tputilv2/perr.cpp:64-95). The dialog for a failed `_write` in `putem` (oper 3) says "The system cannot find the path specified"; for an AJAT seek failure (oper 0) "The operation completed successfully". The real `errno`/`GetLastError` is not captured. `perr(..., GetLastError())` in dbbox1.cpp is correct.
- **Impact.** The operator cannot tell disk full from a dropped share. Severity low. [verified: read] for the argument order; the message text for specific codes is [inferred].
- **Relation.** New, diagnostic.

### N-B7. FILESEND receiver truncates the target at the first packet and forwards a partial file

- **Mechanism.** Packet `pos == -1` opens the target with "wb" (Tp/TpComY.cpp:3013). A failed or out-of-order block just closes the file and sets `tiedvast = NULL` (:3022-3033), with no message and no completeness check. The end packet `pos == -2` forwards the file with `lah_tiedosto` whenever `FileRcv[cn].pos > 0` (:3016-3018), including after a failed receive. The sender's `fwrite` into `__FCOMFILE__.TMP` is unchecked (:2965).
- **Files affected.** The files sent through `lah_tiedosto`: series XML, course XML and the person database (cbHk/UnitSarjatiedot.cpp:844, cbTp/UnitRadat.cpp:2407, cbHk/UnitTKanta.cpp:1421, and the dialog sends at cbHk/WinHk.cpp:2009, ViestiWin/UnitMain.cpp:1879).
- **Impact.** Configuration or reference data can be silently replaced by a truncated copy and then propagated to the other workstations. Competition results (KILP.DAT, emit, AJAT) are not sent this way.
- **Severity.** Medium to low. Confidence [inferred]: whether a lost packet reaches this code depends on the reliability layer.
- **Relation.** Extends H-071 (same function, different aspect: completeness).

### N-B8. Event log and runtime error log: unflushed, failures ignored

- See 2.6. The log is a buffered `TextFl`; only the raw-FILE branch honours `immed`. Severity low. [verified: read].
- **Relation.** New, diagnostic.

### N-B9. `vaihevalinta.cfg` truncated before write, write unchecked

- The file is opened "wt" and written once (Hk/HkInit.cpp:319-324). A failed or interrupted write leaves an empty or old file. With the `vaihevalinta` option the next start reads an empty file as stage 1 (:296-311) and silently starts in the wrong stage.
- **Severity.** Low to medium (the window is small, but the effect, a wrong stage after a restart, is visible only after it has happened). Confidence [inferred] for the failure case.
- **Relation.** New (B-28).

### N-B10. Export success messages after failed writes

- Several user-triggered exports write unchecked and then report success: `kirjKilpDat` (cbHk/UnitExport.cpp:466-483 and Hk/HkDat.cpp:1435-1445) ignores `makefile`, so on failure `datf_tmp.recl` is uninitialised and `addrec0` raises a `perr` dialog per record (and "No" exits the program through `lopetus()`); the caller then shows "Tiedosto kirjoitettu" (UnitExport.cpp:533, also :503, :515). `UnitTKanta.cpp:1397-1416` truncates the person database with "wt", ignores `WriteLine`/`Close`, shows "kirjoitettu" and re-reads the file into memory.
- The `teeBackupkopio` `fclose(NULL)` (Tp/TpApu.cpp:163-164) runs only in the C++Builder build (UnitTKanta.cpp:1397), where H-063 says the RTL returns EOF.
- **Severity.** Low to medium (user-triggered; the export is the manual backup route for KILP.DAT in 2.1, so a false "written" message matters). Confidence [verified: read].
- **Relation.** Extends H-015 (the export detail), duplicates H-063 for `teeBackupkopio`.

### N-B11. SQL queue ring overflow (note)

- See 2.5. Low. Extends H-014.

## 4. Refuted or downgraded osaA claims

| osaA item | Result |
|---|---|
| W11 / Q3: HK `addem` "returns 1; the caller..." | Wrong for the built code. The live `addem` returns 0 on a write failure (res=1 is only the duplicate case). The `res = putem` variant is in the undefined `TALLEMIT` block. All callers ignore the result anyway (N-B1). |
| W14 / B-13: HK `writeaika` "row stays only in memory" | `writeaika` has no callers in either domain. Dead code. |
| W15 / B-14 and W16 / B-15 | Confirmed. The order `fp1++` before `_write` is the cause (N-B2). |
| B-8 "later putrec calls fail with dialogs if the reopen fails" | Confirmed, and the disk-full path is worse than stated (N-B4). |
| W4 / B-4 `closefile` header write ignored | Low impact: the header is rewritten by every `addrec`. It matters only after deletes or `addrec0` (export). |
| B-26 "`system(autokomento)` runs after the write failed" | True only for failures that do not stop `kirjoitettu`; it does not run when nothing was generated. In practice write failures are undetected anyway (N-B3). |
| D-14 `closeprfile(lstf)` after WEOF | Reachable only with a 'W' file (list path), not in `autofile`. |
| B-29 / B-30 | Confirmed (no `autofile` type is 'W'). Corrected: global `merkit` can be 'W' for manual list/export paths. |
| B-13, B-16, B-17 | `suljeajat` `_chsize` and `fclose` are shutdown paths; not analysed further here (part D). |
| B-22, B-23 | Confirmed low, see N-B8. |
| B-9 relay `kilptietue::putrec` | Default `tiedostoon = false` writes memory only (Juk/vkilp.cpp:239-250); which callers pass true is C-1 scope. |

## 5. Open questions

1. Does the bcc32/C++Builder 37.0 RTL `fputs` return -1 (EOF) on a flush failure, and does `fputws` return `WEOF`? Only the headers were read ([unknown] for run-time). A probe on a full volume would settle it.
2. Which value does the global `merkit` hold in practice on the GUI list/export path ('G', 'W', 'A' or '8')? Determines whether any result file is UTF-16 and gets the WEOF check.
3. Does a SportIdent or other reader treat the ACK as final (N-B1), or can the operator re-dip the card? Hardware-dependent.
4. When a network handle drops mid-write, do `SetFilePointer`/`WriteFile` fail permanently, or does SMB reconnect the handle? Decides how many writes are lost in N-B2 and 2.1.
5. How do zero rows in an AJAT gap load at restart (N-B2)? The read loop at Hk/HkIV.cpp:4077-4101 was read, but not the post-load handling in `tall_rivi`.
6. Is there a full "resend all records" route to another workstation (osaA open question in 2.1 item 3)? Hk/HkCom32.cpp:170 `lah_kaikkiajat` exists for time rows only; records are not traced.
7. How long does `autofile` take for a real result set (osaA Q6)? That sets the truncation window in N-B3.
8. Hk/HkIV.cpp:1139-1144 `emittallfile` (`fwrite` unchecked, test tool) and the sendwline raw-file branch (Wrttext.cpp:370-396 uses uninitialised `line` for `prno < 0`) were noticed but are outside the persistent-data scope.

Key files read: dboxm/dbbox1.cpp, tputilv2/perr.cpp, tputilv2/ioerr.cpp, tputilv2/Wrttext.cpp, tputilv2/TextFileClass.cpp, Hk/HkDat.cpp, Hk/HkEmit.cpp, Hk/HkIV.cpp, Hk/HkAjat.cpp, Hk/HkTulostus.cpp, Hk/HkInit.cpp, Hk/HkApu.cpp, Juk/vdat.cpp, Juk/vkilp.cpp, Juk/VEmit.cpp, Juk/vajat.cpp, Juk/VIv.cpp, Tp/TpLaitteet.cpp, Tp/TpComY.cpp, Tp/TpApu.cpp, cbHk/UnitMySQL.cpp, cbHk/UnitExport.cpp, cbHk/UnitTKanta.cpp, cbHk/WinHk.cpp, cbHk/HkKisaLahtLuet.cpp (all under C:\dev\tulospalvelu\TPsource\V52).
