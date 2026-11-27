<!-- Provenance: final report of independent-verifier (Opus), C-9 step 2 review, 2026-11-26; copied unmodified from the hand-back message by the main session. Its questions 4 and N1 were then run by the main session (cases "separate-files mode without a list, TCP link configured" and "C9 N1 ..." in koeohjelmat/tulokset/ajo5.txt). -->
# C-9 step 2: independent review of the probes and claims

Scope: I re-read the production code (cp1252 via iconv), the probe sources, ajo4.txt and the raw ASan reports. I did not edit, build or run anything. All paths below are relative to `C:\dev\tulospalvelu\TPsource\V52` unless they are absolute.

## Summary table

| # | Claim | Verdict | Proposed severity -> mine | Key evidence (my reading) |
|---|---|---|---|---|
| 1a | H-149 single-file UAF (:7200 then :7299) | VAHVISTETTU (confirmed) | korkea -> korkea | `Juk/VTulostus.cpp:7200` `closeprfile` with no NULL; `:7299` `htmlloppu`; `tputilv2/Wrttext.cpp:279-300` frees `pf`; the raw ASan stack is writehtml:1366 <- htmlloppu:7026 <- autofile:7299, freed at :7200 |
| 1b | H-149 kohde T double free at :7321 | VAHVISTETTU | (same row) | The ASan double-free stack: second free at :7321, first free at :7200, allocation at :7166. The same double free also happens for kohde H when `fileyht<0` (:7317-7322), after the UAF |
| 1c | H-149 `delete aflst` at :7164 and :7295 | VAHVISTETTU from code; the ASan evidence is valid but indirect | (same row) | `:7162-7164` deletes without NULL and `:7295-7296` deletes again. `~TextFl` (`tputilv2/TextFileClass.cpp:312-317`) runs `fclose(File)` on a freed object. File was NULL at construction (`:228`, open failed), so the AV in fclose at 0x96 can only come from reading freed memory. ASan did not report a double-free as such |
| 1d | H-149 kohde X leaves `lstf` dangling, read at :7150 | VAHVISTETTU | (same row) | `:7179-7185` returns after `closeprfile` with no NULL; ASan UAF at :7150, freed at :7182 |
| 1-reach | Reachability via startaf at startup | OSITTAIN (partly) | | startaf runs at startup only when `afvali0 != 0` (`Juk/VInit.cpp:2936-2938`). That is set only by `FILETCP` with no prior `HTML=`/`XML=` (`:946-954`). The normal `HTML=` config leaves `yksihtml=0` (`:2526`, `:919-929`), so the default path without AUTOFILE.LST is **(c)**, not (a). Path (a) comes from the console menu (`JukConsole/VConTul.cpp:463-465`, startaf(1)), from FILETCP, or from the GUI radio. The GUI default radio is "separate" (`ViestiWin/UnitAutoFile.cpp:49,68`), so it also gives (c) |
| 1-fileyht | fileyht >=0 on the first TCP link | VAHVISTETTU: any TCP data link, not only a file link | | `Tp/TpComY.cpp:1221-1222` is in `opencomTCP`, called from `tiedonsiirtoTCP` (`:1273`), which starts for every `comtype & comtpTCP` link (`:2226-2227`) |
| 1-sev | korkea | Agreed, with a different argument | korkea | Path (c) double-deletes `aflst` whatever `fileyht` is (:7164/:7295). So a console relay with `HTML=` and no list file should crash at the first cycle even with a TCP link. This is inferred: Q85 ran (c) only with `fileyht=-1` |
| 1-func | Single-file + fileyht>=0: no TCP transfer | VAHVISTETTU as an inference that depends on undefined behaviour | | `sendtcpfile` guard `:7110` reads freed `lstf`. `u.Fl` was NULLed before `free` (`Wrttext.cpp:285-286`), so it returns early. The result depends on the freed block not being reused |
| 2 | H-145 emit write failure | VAHVISTETTU, with two corrections | keskitaso -> keskitaso | `Hk/HkEmit.cpp:1666-1669`: `on_emit` registers before `putem`, and `res` stays 0. `Hk/HkIV.cpp:1145-1152`: the `ed_em` cache is updated before `addem`, the return is ignored, and the function always returns 0. `laheta_em` runs only on success (`HkEmit.cpp:1681`) |
| 3 | H-146 AJAT hole | VAHVISTETTU; the effect on results is inferred | keskitaso -> keskitaso | `Hk/HkAjat.cpp:274-280` lowers fp1 to the insert row, `:369` does `fp1++` before `:372` `_write`. `initajat(0,1)` matches startup in both settings (see notes) |
| 4 | H-147 WEOF dead / autofile sequence | OSITTAIN: WEOF part confirmed for both compilers; sharing violation confirmed for MSVC only | keskitaso -> keskitaso | `TextFileClass.cpp:546,575` return the `fputws`/`fputs` result (-1), compared with WEOF (0xFFFF) at `Hk/HkTulostus.cpp:1484,1503,1526` and `Juk/VTulostus.cpp:1370,1392,1408`. bcc32 RTL probe: `fputws` gives -1 and `sizeof(wint_t)=2`. Both compilers use the same `_wfopen_s` call (`TextFileClass.cpp:305`; `Tputil-XE.cbproj:504`). The bcc32 share mode was not measured |
| 5 | H-150 leak, bcc32 50 streams | VAHVISTETTU (leak); consequences in HkKisaWin EI TODENNETTAVISSA (unverifiable statically) | keskitaso -> keskitaso (korkea only after a bcc32 run) | `Hk/HkTulostus.cpp:7120-7147,7314-7316`: `new TextFl` inside the do-while, which runs twice when `k_pv && kohde=='X'`; only the last one is deleted |
| 6 | H-151 via unopened link | VAHVISTETTU; the setup is representative for the free/dangling part | keskitaso -> keskitaso (see new issue N1) | `Hk/HkTulostus.cpp:7060`: `closeprfile` runs whatever `wrt_st_TCP` returns; ASan stack shows the free in thread T1 at sendtcpfile:7060 and the read at :7093 |
| 7 | H-152 deferred | Agreed | | Needs the `cbHk/TulostusUnit2.cpp:1388-1397` handler plus a real printer PRFILE. A static reading is not enough to show the stale alias is later dereferenced on a live path |
| 8 | Inventory diff, nothing significant | OSITTAIN | | 15 of the 16 closeprfile rows are benign (`avaa_tiedosto_nm` NULLs on failure, `Juk/VTulostus.cpp:8391-8393`). `Juk/VTulostus.cpp:7131` is `sendtcpfile`, not list/htmlrunko/yhtveto, and it is the relay twin of H-151 (new issue N1) |
| 9 | Probe hygiene | Mostly sound; 3 caveats | | The H-149 list-mode control does nothing; the bcc32 dfree probe is not a VCL process; the H-147 probe header describes an unused injection method (details below) |

## Notes and disagreements

### 1. H-149

- **Correct the reachability text.** "startaf sets yksihtml when AUTOFILE.LST is missing and is called at startup in console and GUI" is only true when FILETCP is configured. In that case afname becomes `__TMP__.XML` while kohde stays 'H' (`Juk/VInit.cpp:950-953`, `:2525`).
  - The common configurations are `HTML=name` in the console, or the GUI dialog with its default "separate" radio. Without a list file both give `yksihtml=0`, so they take path (c).
  - Path (c) combines all the defects: double delete of `aflst`, the UAF at :7299, and the double free at :7321 when `fileyht<0`.
  - Q85 "separate-files without a list" (exit 0xC0000374 in cycle 1) is therefore the measurement for the default configuration. It ran with `fileyht=-1`.
- **korkea is justified for MSVC**, but the argument should rest on path (c).
  - Path (c) crashes whatever `fileyht` is, because the double delete at :7164/:7295 does not depend on `fileyht`. This is inferred: no Q85 case ran (c) with `fileyht>=0`.
  - Path (a) with a TCP link does not crash. It never transfers the file and leaves `lstf` dangling.
- **ViestiWin (bcc32):**
  - "Silent double free every cycle" rests on `rtl_bcc32.c` dfree. That is a `-WCMR` console program with no VCL, and the free happens immediately with no allocations in between.
  - ViestiWin is a VCL program with runtime packages. Its allocator may be the shared memory manager (borlndmm/FastMM), not the bare RTL heap. This was not measured.
  - In ViestiWin, autofile runs on the `LukumaaratTh` worker thread (`ViestiWin/UnitMain.cpp:535-541`). The corruption is therefore concurrent with the VCL main thread, and its later effects are unknown. Use "double free, effect unmeasured" rather than "silent".
- **(c) ASan evidence.**
  - The report is an access violation inside `fclose` (`TextFileClass.cpp:315` <- `VTulostus.cpp:7295`), not an ASan "double-free". This is a borderline use of a crash as evidence under the TpCore protocol.
  - It is still conclusive in combination with the code. `File` stayed NULL for a list file that failed to open (`TextFileClass.cpp:272-279`), so only freed memory can explain a non-NULL `File` there.
  - Cleaner evidence: build tputilv2 with ASan, or record the `aflst` pointer before both deletes.
- **(d)** is confirmed. Side observation (not in the claim): relay kohde 'X' writes only `xmlloppu` (`:7179-7181`; `writexml` is commented out), so the relay automatic XML file contains only the closing part. Probably a separate functional defect.

### 2. H-145

- **The ACK path applies to the Emit clock only.** `Tp/TpLaitteet.cpp:911-917` is in `lue_EmitKello` (the `regnly > 10` branch with 'A'..'L' message types).
  - The MTR path (`:1247-1248`) and the LUKIJA/SIRIT/SportIdent paths (`:1098`, `:1365`, `:1475`) ignore the return value. MTR advances by package number.
  - LID_MTR is a fair stand-in for the storage logic, because the code after parsing (`Hk/HkIV.cpp:1135-1152`) is shared by all readers. It does not exercise the ACK.
  - Rewording: "tall_emit never reports the failure. For the Emit clock this produces the ACK; for other readers the message is consumed anyway."
- **Injection fairness:** a read-only fd makes `_write` return -1 (EBADF). Disk-full gives -1 or a short write; both fail the `< sizeof(emittp)` check at `HkEmit.cpp:1612`. Fair.
- **pros_emit range:** pros_emit walks `emithead2..emithead`, not `emitptr` (`Hk/HkIV.cpp:1184-1187`).
  - `emithead` is not advanced, and the next card overwrites the same slot (`putem(em, emithead)`).
  - So the card never reaches a competitor and never reaches `laheta_em`.
- **Permanence:**
  - Within a session, yes with `estaemittoisto=1` (default, `Hk/HkGlobals.cpp:523`).
  - `emitvaraus` and `ed_em` are in-memory statics. After a restart, a re-read (or a reader-memory resend) would store the card. "Permanent until restart or manual entry" is more accurate.
  - The failure is not silent: `iowerr` shows a `wperr` dialog.
- **Code variant:** the compiled `addem` is `HkEmit.cpp:1657`. TALLEMIT, which guards the `:1333` copy, is not defined in `VS/Hk/HkMaali/HkMaali.vcxproj` or `RADStudio10/HkKisaWin.cbproj`.

### 3. H-146

- **Is `initajat(0,1)` the default start?**
  - `ajanottofl` defaults to -1 (`Hk/HkGlobals.cpp:431`), which gives `initajat(ino, 1)`.
  - It becomes 1 with timing devices (`REGNLY`, `VA-AJAT`, ...; `Hk/HkInit.cpp:1303,1624,1888`), which gives `initajat(ino,-1)` and skips the placeholder fill at `HkIV.cpp:4003-4014`.
  - The caller has already filled every row with the same placeholder (`HkIV.cpp:4207-4224`), so both start states are equivalent. The fixture is fair.
  - `sail_ajat=1` is equivalent to answering K at the prompt (`:4039-4076`).
- **Mechanism (confirmed):**
  - `HkAjat.cpp:274-275` lowers fp1 to the inserted row.
  - The loop at `:368-370` advances fp1 past it before the `_write` at `:372`.
  - The iowerr offset (66 = 3 x 22) in the raw output matches.
  - The hole is not strictly permanent: any later change at or before that row lowers fp1 again (`:208-209`, `:274-275`) and rewrites it. The register says this correctly.
- **Effect:**
  - The startup file pre-write (`HkIV.cpp:4113-4127`) leaves a placeholder (t = TMAALI0*AIKAJAK, kno 0) in the slot.
  - After a restart, `rwtime` trims only trailing placeholders (`:4102-4103`), so an empty row stays inside the list. It sorts first because its time is the minimum, and new times go after it (as measured).
  - The effect on results is not shown. Assigned competitor times live in KILP.DAT through a separate path, so I would expect a lost AJAT/audit row plus an empty list row. Results are hit only if the time was unassigned or results are rebuilt from AJAT. This is inference; label it [paatelty].

### 4. H-147

- **Injection is fair:**
  - The reduced-access duplicate is put in with `_dup2` after the open, so stdio fails at flush. That is the same path as a disk full during writing.
  - Caveat: a real disk-full usually leaves a *partial* file (earlier buffers flushed), not 0 bytes. "Stays 0 bytes" is specific to a failure at the first flush. "Truncated or partial, no message" is the general result.
- **Readers:**
  - UCRT `_wfopen_s` uses `_SH_SECURE`, which gives share 0 for write access. Error 32 during generation is valid for MSVC builds. The same mechanism makes autofile's open fail whenever any reader holds the file open, whatever share flags the reader used.
  - bcc32 compiles the same `_wfopen_s` call (`TextFileClass.cpp:305`, no `#ifdef`). Whether the Embarcadero RTL opens it exclusively (C11 Annex K only says "to the extent supported") was **not** tested, so for HkKisaWin/ViestiWin the statement is unverified.
  - Also, after a failed run readers do see the truncated/partial file until the next good run. "Not partial content" holds only during generation.
- **WEOF:**
  - Type 'V' (`TextFileClass.cpp:548-556`) returns a character count and is never -1, so it is also undetected.
  - `tputilv2/Wrttext.cpp:447` has another WEOF comparison of the same kind.
- **Probe hygiene:** the header of `c9_hk_probes.cpp` says result files are injected with `LockFileEx` (`RangeLock`), but the code uses `makeWritesFail` (DuplicateHandle + `_dup2`), and `RangeLock` is unused. Fix the comment.

### 5. H-150

- **Leak confirmed.**
  - With AUTOFILE.LST present, each run leaks one FILE stream and handle. Without it, each run still leaks a TextFl object and its FileName (86 B), with no stream.
  - In the measurement, MSVC streams reached 511 after about 508 runs (about 8.5 h at 60 s).
- **bcc32:**
  - 47 + 3 streams were measured in a console bcc32 program. HkKisaWin already holds some streams at steady state (`femitfile` `_wfdopen` `Hk/HkEmit.cpp:1371/3057`, `fafhandle` `HkIV.cpp:4134`, log files), so exhaustion comes in fewer than about 45 runs.
- **What fails afterwards** (all inferred from code, none measured in a GUI):
  - autofile's own open fails, giving a `writeerror_w(...,2000)` message on every retry (`HkTulostus.cpp:7108-7115`).
  - Every TextFl/`openprfile` open (exports, print-to-file, lists) fails with messages.
  - AJAT `_wfdopen` returning NULL is tolerated (`HkAjat.cpp:340,377`, though `HkIV.cpp:4135` calls `setvbuf(NULL)`).
  - The emit reopen at `HkEmit.cpp:3057-3058` would call `setvbuf(NULL)` and later use a NULL `femitfile`. This needs one more stage change in the same session, because the leak starts only at stage 2 or later.
- **Severity:** I would not raise it to korkea yet. The configuration is narrow (XML + stage 2 or later + AUTOFILE.LST present in the directory), the failures are mostly reported rather than silent, and the most dangerous consequence needs another stage change. Raise it if a bcc32/VCL run shows the emit/AJAT paths crashing or losing data.

### 6. H-151

- **Representative.**
  - The free at `HkTulostus.cpp:7060` happens whatever the send result. A real link only makes the thread take longer: while it is still sending, the guard at :7093 sees a valid `lstf` and returns 1 (retry); after it finishes, the read is dangling as in the probe.
  - With a real link there is also an unsynchronised race between the thread's free (under the `tcp` lock) and the guard read (under the `autotul` lock).
  - `fileyht >= 0` arises from *any* TCP peer link (see 1-fileyht), so the scenario is "HK with any TCP link plus autofile".
  - The double free in separate-files mode (:7171-7174 / :7272-7275) is static only: ASan stops at the first read. It is labelled correctly.

### 8. Inventory spot-check

- Checked: `Hk/HkTulostus.cpp:8740,8826,8831`; `Juk/VTulostus.cpp:6672,6677,8450-8478,9068,9139,9144`.
  - All close a local `tulprm`, or reopen through `avaa_tiedosto_nm`, which leaves `lstf=NULL` on failure.
  - The 'P' branch is excluded from the `"IREHSXZ"` close, so there is no double close.
- `Juk/VTulostus.cpp:7933` (`joukkuetul`) is inside `#if defined(JOUKKUETUL) && !defined(AMPUSUUNN)` (`:7672`). I found no callers, so it is effectively dead.
- Startup `lopetus`:
  - `Hk/HkCom32.cpp:863-866` is a console J/P prompt that calls `lopetus()` on the communication thread. That matches the main session's description and the H-153 family.
  - I did not check whether the relay row (`Juk/VCom.cpp:881`) has the same shape.
- putrec_i rows `Juk/VEmit.cpp:1263,1700`: `PUTREC` then `putrec_i` with the result ignored, the H-015 pattern. Agreed.
- **Exception: `Juk/VTulostus.cpp:7131` was misgrouped.** It is the relay `sendtcpfile` and leads to N1.

### 9. Probe hygiene

- **The H-149 list-mode control does nothing.**
  - It ran with `sarjaluku=0` and a list containing only "\r\n". The list loop finds no valid line (`:7212-7216`) and exits without opening any file (`lstf=00000000` in ajo4). The Release Q85 list-mode control is the same.
  - So "no ASan report in list mode" does not show that list mode is clean, and it never exercises `:7260-7262` (see N1).
  - It does not affect (a)-(d), which happen before or independently of any series output.
- `sarjaluku=0` elsewhere: harmless for the free/UAF mechanisms. Statements about file content (e.g. "931 bytes, </HTML> present") cover only an empty result set.
- `maxntime 20`, `k_pv` (0 for H-145/H-147, 1 for H-150), `regnly LID_MTR`, both `estaemittoisto` values, and `fileyht` (-1, or 5 with `hComm` 0) do not change the conclusions, apart from the ACK nuance in note 2.
- **TPRESULT observed=1 decisions:**
  - Sound for H-145, H-146, H-147, H-150 (state checks with REQUIREd preconditions).
  - Sound for H-149a/b/d and H-151 (ASan report type plus file:line).
  - H-149c relies on an ASan access violation rather than a sanitizer classification; acceptable only together with the code reading (note 1).
  - The Q85 Release crashes emit no TPRESULT. They are correctly used only as consequences, not as the mechanism.
  - Q85 cannot say which free in cycle 1 triggered 0xC0000374 (there is no per-step logging), but every candidate is one of the confirmed defects.
- **Release probe:** `/MT`, the same setting as the production `VS/V/JukMaali520.vcxproj:137,159`, so the 0xC0000374 behaviour should carry over to the production console programs.

## New issues found

**N1 (proposed extension of H-149 or H-151; I estimate korkea for networked relays): relay list mode plus any TCP link gives a dangling `lstf` and a double free from the 2nd cycle.**
- This is static only, not run; label it [paatelty].
- Sequence:
  1. With `fileyht >= 0` (any TCP data link, `Tp/TpComY.cpp:1221-1222`), list mode ends with `_beginthread(sendtcpfile, ..., &aftulparam)` (`Juk/VTulostus.cpp:7308-7316`).
  2. `sendtcpfile` frees `tulprm->lstf` at `:7131` and does not NULL `aftulparam.lstf`.
  3. Next cycle: the guard at `:7150` reads freed memory (`u.Fl` was NULLed before the free, so it passes). Then the first series with results reaches `:7260-7262`: `htmlloppu` (no-op on freed memory) followed by `closeprfile` on the freed PRFILE, which is a double free.
- Expected effect: under MSVC Release, 0xC0000374 at the second automatic output of a networked ViestiMaali/ViestiLuenta using AUTOFILE.LST, i.e. the configuration the current list-mode control was meant to show as safe.
- Also inferred: `sendtcpfile` sends only the last series file, and when `fileyht` is a peer data link rather than FILETCP it writes STX + file bytes + ETX into that link's protocol stream (`:7116-7128`).
- How to settle it: a probe with at least one series with `ntulos>0`, plus a list line naming an explicit file. Alternatively, set `aftulparam.lstf` to a fresh `openprfile` result, call `sendtcpfile(&aftulparam)` directly with `hComm[yht]==0`, then call `autofile()` in list mode under Release and check for 0xC0000374 before `tulostasarja` is reached. ASan would stop at the read at :7150 first.

## Unresolved questions

1. Does the Embarcadero RTL `_wfopen_s` open write streams exclusively (share 0)? Settle with a bcc32 `-WM` VCL probe that opens with `_wfopen_s("w+b")` and tries a second `CreateFile`.
2. Is ViestiWin's double free really silent under the VCL/runtime-package memory manager, and does the corruption show up later (duplicate block handed out, EInvalidPointer)? Settle with a VCL test program built with ViestiWin's packages, doing repeated autofile-like free/alloc on two threads.
3. Does N1 crash as predicted (see the probe recipe above)?
4. Does path (c) with `fileyht >= 0` crash in Release? Q85 ran (c) only with `fileyht=-1`.
5. H-146: is a lost AJAT row ever used to rebuild results (re-assigning times from AJAT after a restart), or is the effect limited to the time list?
6. H-150 in HkKisaWin: steady-state stream count and the actual behaviour of `setvbuf(NULL)` / a NULL `femitfile` at the next stage change under bcc32.
7. When `fileyht` points to a peer data link, does `sendtcpfile`'s STX/ETX file stream break the peer's packet parser? (`Tp/TpComY.cpp` receive side, not reviewed.)
