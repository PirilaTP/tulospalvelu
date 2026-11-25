<!-- Provenance: final report of architecture-analyst (Sonnet), C-9 step 1 part A, 2026-11-26; copied unmodified from the hand-back message by the main session. Counts refer to the first script run (osaA/yhteenveto-v1.txt). -->
# C-9 step 1, part A - classification of the resource and error-handling inventory

Tags: [verified: read] = read in source; [script] = taken from the CSVs only; [inferred] = reasoned, chain not fully traced. All paths are relative to TPsource/V52/ unless they start with doc/. "K/S/U/L" are the run phases defined in the task.

## 0. Summary

1. The script is usable as a worklist but not as a count of defects. The `use` column is wrong in several systematic ways (section 1.2). The four largest are:
   - Brace-less `if (c) call();` makes the call count as "checked". All 20 `fclose`/`_close` rows classed "checked" are false positives (the regex check below), and so are at least 3 of 4 sampled "checked" memory allocations.
   - The `Open`/`Close` rows are mostly VCL SQL datasets and forms, not `TextFl`.
   - `openfile`/`makefile` are classed "void" although they return int in the built code.
   - 211 rows (6.7 %) are in files that no project builds. A further 29 rows are function definitions, not calls.
2. The most important semantic finding is that the write-error check on result and auto-output files only works for UTF-16 files. `puts_f`/`writeline`/`writehtml` test `TextFl::WriteLine(...) == WEOF`. `WriteLine` returns the `fputs` result (EOF = -1) for the 8-bit types ('8','U','A','O'), and `openprfile` and `autofile` open files with type '8' or 'A'. So these write errors are never detected. `TextFl::Close` and the destructor ignore the `fclose` result. Details are in B-29 and B-30.
3. The `dbbox1.cpp` write path never reports a write failure to the caller. `putrec` shows a modal Yes/No dialog and returns -1, and every caller ignores the -1 (H-015 already covers this). New detail: `newrec` advances `numrec` before the record is written. For the AJAT queue, `writeajat` advances `fp1` before it writes, so a failed write marks the rows as persisted.
4. New low-severity or unregistered observations that I verified by reading:
   - Leaks on error paths: `sifile` (Tp/TpComY.cpp:1973-1976), `lahfile` (Tp/TpComY.cpp:2931-2936), and the `_wopen` handle in `openprfile` (tputilv2/Wrttext.cpp:249-262).
   - `fclose(NULL)` in `teeBackupkopio` (Tp/TpApu.cpp:163-165), on its normal path.
   - `autofile` writes the result file by truncating it in place, and runs `system(autokomento)` even if the write failed (Hk/HkTulostus.cpp:7102-7108, 7341-7346).
   - Per-series open failures in `autofile` are silent (Hk/HkTulostus.cpp:7217-7218, 7276-7278).
   - The SQL backup queue is disabled silently when its file cannot be opened (cbHk/UnitMySQL.cpp:691-692).
5. Phase totals: 3130 call sites, 235 functions with a persistent write or an unchecked non-memory acquire. By phase (row counts; rule-based, section 2): S 370, K 699, U 1546, L 29, library code used by several phases 127, U-or-K 117, unclassified 31, not built 211. The 78 functions in K are the real target.
6. Verified by reading, so no finding: the `tul_a` allocation in `tulostasarja` (freed at Hk/HkTulostus.cpp:6475 and 6531), the ring buffers in `add_bdg_t` (allocated once, Hk/HkIV.cpp:826-832), and `reconnectTCP` (closes the previous socket at com/WinUDP.cpp:874).

## 1. Validation of the script

### 1.1 Method

I read 105 sampled rows plus about 30 targeted ones, which comes to about 135 rows. The 105 were drawn with a seeded stratified random sample over every (group, kind, use) class. I also ran my own regex checks over all rows (the brace-less check, the definition check, the `Open`/`Close` object check, the `new TextFl` check, the project-membership check and the skipped-preprocessor-branch check). Helper scripts were kept in the scratchpad only.

### 1.2 Results per class

"n" is the number of rows I read in that class. TP means the class label was correct for what the code does.

| class (group/kind/use) | n | TP | FP / FN found |
|---|---|---|---|
| dbbox write / ignored | 5 | 5 | None. The `putrec` result really is ignored in `dbbox1.cpp:94,132,156,221,249,258,265` [verified: read]. 3 of the 5 are in the unbuilt `Dbbox.cpp`. |
| dbbox write / nested | 1 | 0 | `Juk/vkilp.cpp:239` is the definition of `kilptietue::putrec`, not a call. |
| dbbox acquire / void | 31 rows | - | Systematic FN (S2 below). |
| file acquire / assigned-checked | 3 | 3 | None (`Tp/TpApu.cpp:876`, `Hk/HkIV.cpp:890`, `ViestiWin/UnitMain.cpp:300`). |
| file acquire / assigned-unchecked | 4 | 2 | FP 2: `com/winCom.cpp:115` (the check `*hC == INVALID_HANDLE_VALUE` is at line 122, outside the 6-line window) and `Tp/TpComY.cpp:3013` (the member is tested at line 3021). TP 2: `cbHk/UnitKilpMaaritys.cpp:604` and `cbHk/HkArv2.cpp:897`, a debug file. |
| file acquire / checked | 4 | 4 | One is correct only by luck (`ViestiWin/UnitComfile.cpp:799`; the `if` is the dialog test, the NULL test is on the next line). |
| file read / assigned-unchecked | 4 | 4 | `cbHk/UnitMySQL.cpp:669,677` and `ViestiWin/UnitMySQL.cpp:616`: `er` is overwritten and never tested. |
| file read / ignored | 4 | 2 | TP 2: `Juk/VEmit.cpp:1411` and `Hk/HkSak.cpp:136`. FP 2: `ReadFile` in `dbboxm/dbbox1.cpp` `getrec` and `com/winCom.cpp:198`, where the result is tested through the byte-count output variable. |
| file read / checked (`_read(..) < sizeof(x)`) | 1 read | lexical TP | Semantically unreliable: `-1 < sizeof` is false for an unsigned compare (registry H-053). I read only `Juk/VEmit.cpp:5928`; the other 3 of the 4 sampled sites are [inferred] to have the same pattern. |
| file release / checked | 20 (all) | 0 | FP 20 of 20. The regex check (every row whose call starts the statement and whose previous line is a brace-less `if`/`else`/`for`) hits 20 of 20; I read `tputilv2/TextFileClass-2.cpp:168`, `cbHk/UnitComfile.cpp:823`, `JukConsole/lasknum5.cpp:219` and `tputilv2/UnitTest.cpp:332`. The `fclose` result is never used. |
| file release / ignored | 4 | 4 | None. |
| file seek / assigned-checked | 1 | 1 | None. |
| file seek / assigned-unchecked | 4 | 3 | FP 1: `tputilv2/TextFileClass.cpp:659` is returned on line 661. |
| file seek / ignored | 4 | 4 | None. |
| file write / assigned-checked | 1 | 0 | `Juk/vSarjat.cpp:524`: `er` is reassigned on line 527 before it is tested on line 528. |
| file write / assigned-unchecked | 3 | 0 | `TextFileClass.cpp:546,596` and `UnitTest.cpp:464`: the value is accumulated in `nwritten` and returned, so it is an indirect "returned". |
| file write / checked | 4 | 2 | FP 1: `Juk/VInit.cpp:315` is `for (..) fprintf(..)`, so the `for` keyword counts as a check. Brace-less pattern: 20 of 21 rows whose call starts the statement and are classed "checked". |
| file write / ignored | 5 | 5 | None (`Tp/TpComY.cpp:227`, `ViestiWin/UnitMySQL.cpp:658`, `Hk/HkAjat.cpp:467`, `Juk/VEmit.cpp:1518`, `HkSak.cpp:130`). |
| textfl method-open / ignored+checked | 5 | 1 | FP 4 of 5 sampled: `SQLDataSet1->Open()` is a VCL dataset. Of the 33 `Open` rows only 8 are `TextFl` (KierrFile 6, af_tied 1, sralstf 1). |
| textfl method-close (void) | 76 rows | - | 59 rows are forms, `SQLDataSet1`, `SQLConnection1`. Roughly 17 are `TextFl` (InFile 4, InitFile 2, KierrFile 6, OutFile 4, af_tied 1). |
| textfl method-write / ignored | 5 | 5 | None. |
| textfl method-write / checked | 8 | 5 | FP 3 of the 8: `cbHk/UnitAvausMaar.cpp:91`, `cbHk/UnitSakkoKierr.cpp:146`, `cbTp/UnitHtmlEdit.cpp:165` are brace-less ifs. All 24 "checked" rows minus the 5 genuine ones (`Hk/HkTulostus.cpp:1484,1526`; `Juk/VTulostus.cpp:1369,1392,1408`) are [inferred] FP. |
| textfl method-write / checked, the genuine ones | 5 | 5 lexical | Semantically ineffective except for UTF-16 (B-29). |
| mem acquire / assigned-checked | 1 | 1 | None. |
| mem acquire / assigned-unchecked | 5 | 5 | None (`Hk/HkEmit.cpp:2810`, `Hk/HkIx.cpp:1636`, `Juk/VEmit.cpp:3170`, `Juk/VInit.cpp:471`, `Tp/TpComY.cpp:498`). |
| mem acquire / checked | 5 | 2 | FP 3: `Hk/HkIV.cpp:260`, `com/WinUDP.cpp:280`, `Juk/VIx.cpp:1102`. Brace-less pattern in 18 of the 67 "checked" rows (regex). `com/gethttp.cpp:76` is correct by luck. |
| socket / assigned-unchecked | 1 | 1 | `com/WinUDP.cpp:877`. |
| socket release / ignored; gdi release / ignored; sendto assigned-checked | 3 | 3 | None. |
| error / report (writeerror.csv) | 8 | 8 lexical | The `next` token is correct, but the `stmt:` class (166 rows, 33 %) is uninformative. E.g. `Hk/HkAjat.cpp:217` is followed by `if (loki) ...; errret = 1; goto poistu;`. `Juk/VTulostus.cpp:6020` is a "restart" notice. 3 of the 8 turned out to be return or goto within 3 statements. |
| exit / exit | 7 | 7 | None. 2 of them are in the unbuilt `Boxtest.cpp` and `BOXTEST_F.cpp`. FN: the callers of `lopetus()` (about 25 call sites) are not rows, see 1.3. |

Overall, among the non-void rows I read, the "checked" labels on release and write calls were wrong in most cases because of the brace-less pattern. The "assigned-unchecked" labels were about 70 % right (the misses are the 6-line window and indirect return). The "ignored" labels were about 90 % right.

### 1.3 Systematic script errors and how to fix them

S1. **Brace-less `if (c) call();` is "checked".** `classify()` returns "checked" if any `if/while/for/switch` token precedes the call inside the statement. Fix: apply the rule only when the call lies inside the parenthesised condition. Look at the depth of the nearest `(` that follows the keyword and check that the call index is before its matching `)`.

S2. **`openfile`/`makefile`/`openindex`/`makeindex` are registered with `rv=False`** (resurssit.py `_reg("dbbox","acquire",..., rv=False)`). In the built library they return int (dbboxm/dbbox1.cpp:104 `int makefile`, :161 `int openfile`; include/boxr.h:62-65), so they should be classed like file acquires. Callers that ignore the return value, which the script therefore calls "void" [verified: read]:
   - Hk/HkDat.cpp:1435
   - Hk/HkInit.cpp:3480 (checked by the following `openfile` at :3486)
   - Juk/vdat.cpp:188 (reopen in `addfrec`, K) and :948
   - Juk/VInit.cpp:2741
   - cbHk/UnitExport.cpp:466
   - cbHk/UnitImport.cpp:1437
   - ViestiWin/UnitImport.cpp:1117
   - HkConsole/HkMain.cpp:150

   `addrec0`, `openfile_w`, `makefile_w` and `putrec_i` are not in CALLS. The first is used at Hk/HkDat.cpp:1440 and cbHk/UnitExport.cpp:479, the last at Juk/vdat.cpp:155.

S3. **The assigned-checked window is measured from the call line, not from the end of the statement.** LOOKAHEAD is 6 lines. Multi-line calls and a check more than 6 lines later give false "assigned-unchecked" (`com/winCom.cpp:115`, `Tp/TpComY.cpp:3013`). A later reassignment of the same variable counts as a test (`Juk/vSarjat.cpp:524`). Fix: start the window at the `;` of the statement, and stop at the next assignment to the variable.

S4. **Indirect checking is not modelled.** `ReadFile`/`WriteFile` with a `&bytes` output variable, wrappers that accumulate and return (`TextFl::WriteLine`, `Rewind`), and `x = call(); return x;`. Fix: a small whitelist (`getrec`/`putrec` in dbboxm, `TextFl::*`), or classify them "indirect".

S5. **Name collisions.** `MEMBER_ONLY` matches `Open`/`Close` on any object. Fix: restrict to variables declared as `TextFl *`, using the declaration table from `scan_file`, or exclude `SQLDataSet`, `SQLConnection`, `Form*` and `Table`.

S6. **Definitions are counted as calls**, e.g. `void writeerror_w(...)` at cbTp/cbTpApu.cpp:467,480,493, HkConsole/HkCons0.cpp:102, JukConsole/laskent0.cpp:59, and `putrec`/`makefile`/`openfile`/`closefile`/`addrec`/`deleterec`/`addkey`/`deletekey` in `dbboxm/Dbbox.cpp` and `dbbox1.cpp`. At least 29 rows (my single-line regex; more if the return type is on its own line). Fix: skip a name token whose line, up to the name, matches a return type, or whose `)` is followed by `{` or `const`.

S7. **Files not built are included.** None of these files is in any `.vcxproj`/`.cbproj` outside the LibTest and TpCore test projects; I checked by basename against the project files. 211 rows, listed below. Fix: add a project-membership filter (read the `<ClCompile Include>` and `<CppCompile Include>` entries).

| file | rows |
|---|---|
| Tp/TpComY32.cpp | 55 |
| tputilv2/UnitTest.cpp | 46 |
| tputilv2/TextFileClass-2.cpp | 35 |
| dbboxm/Dbbox.cpp | 29 |
| dbboxm/BOXTEST_F.cpp | 21 |
| dbboxm/Boxtest.cpp | 17 |
| JukConsole/lasknum5u.cpp | 6 |
| tputilv2/testucode.cpp | 2 |

   Also `Hk/HkTV.cpp` (7 rows) is not in any project although it is called from `Hk/HkDat.cpp:1236` and `Hk/HkIV.cpp:3650` [inferred: likely under a macro; open question Q1].

S8. **Wrappers are invisible.** Not in the inventory:
   - `openprfile`/`closeprfile` (tputilv2/Wrttext.cpp:147/279), the main way result and auto files are opened.
   - `_wfopen_s` in `TextFl` (tputilv2/TextFileClass.cpp:305, 361), so the constructor open and `TextFl::Open` are not rows. `new TextFl(..)` is a row but with `use` n/a.
   - `_wfdopen`/`_fdopen` (6 sites): Hk/HkEmit.cpp:1371, 3057; Hk/HkIV.cpp:4134; Juk/VEmit.cpp:3607; Juk/VIv.cpp:2800; tputilv2/Wrttext.cpp:253.
   - `_chsize` (4): Hk/HkAjat.cpp:402; Hk/HkEmit.cpp:3075; Juk/vajat.cpp:392; Juk/VEmit.cpp:3625.
   - `_wrename` (Tp/TpApu.cpp:165), `CopyFile` (cbHk/UnitKisaLuett.cpp:428-434), `system` (Hk/HkTulostus.cpp:7345, tputilv2/lue_v_pv.cpp:48), `ofstream` (cbHk/UnitYv.cpp:96, ViestiWin/UnitYv.cpp:111).
   - `iowerr`/`perr` (the modal error dialog).
   - `lopetus()` (about 25 call sites). Fix: add these names to CALLS. Add `lopetus` to the "exit" group.

S9. **`new T` rows have no result classification.** `new` throws, so a `== NULL` test after it is meaningless. I checked all 186 `new TextFl`: 180 have an `IsOpen` test within 8 lines. The 6 that do not:
   - HkConsole/HkCons3.cpp:1523 and Juk/VTulostus.cpp:7748 test `== NULL`, which is never true, so no open check [verified: read]. In `HkCons3.cpp:1531`, `WriteLine` then runs on a `TextFl` whose `File` is NULL; `TextFl::WriteLine` has no `File` guard (tputilv2/TextFileClass.cpp:536-582). This is U-phase, and on the MSVC console build `fputws(..., NULL)` ends the program [inferred, same mechanism as H-063/T-020].
   - cbHk/UnitComfile.cpp:799, cbTp/UnitFIS.cpp:418, :490 and cbTp/UnitLisaTekstit.cpp:66 (U, GUI forms).

S10. **Preprocessor.** `drop_else_branches` keeps only the first branch of every `#if`. I counted calls in the skipped `#else`/`#elif` branches (comments excluded): 0 acquire, 0 write, 0 release calls, and 31 `writeerror`/`writeerror_w` (Juk/VCom.cpp 10, Hk/HkIV.cpp 6, HkConsole/HkCons3.cpp 3 and others). So the effect on the resource inventory is nil, and small for the error-report lists. Conversely, code under macros that no project defines (e.g. `TALLEMIT`, `Hk/HkEmit.cpp:1275-1377`; I grepped `.vcxproj`/`.cbproj`: no definition) and debug-only macros (`DBGFILE`, defined only in the HkMaali Debug configuration, VS/Hk/HkMaali/HkMaali.vcxproj:112-118) is counted as live. 16 `dbgarvfile` rows (cbHk/HkArv2.cpp) and the `dbgfile` rows in Tp/TpLaitteet.cpp are such cases [inferred].

S11. **Fallback attribution `~` (366 rows, 11.7 %)** was right in the cases I looked at (`~TFormComfile::Button8Click`, `~opencomfile`), but names such as `~kirjaahyv` are nearest column-0 heads. I found no wrong attribution in about 20 checks.

S12. **Dead code is not detected.** `Hk/HkSak.cpp:114,121`: `er` is initialised to 0 and never assigned before `if (!paikkafl && er == 1)`, so the two unchecked `_wfopen` rows (:123, :132) and the following `fwrite`/`fread` are unreachable [verified: read].

## 2. Phase classification (task 2)

### 2.1 Method

Rule-based by file and function name, with overrides for known call chains. The mapping script is in the scratchpad (not kept). Everything is [inferred] except where a caller was grepped. Callers I grepped:
- `kirjvaliajat`: Hk/HkTulostus.cpp:8569 etc.
- `tall_ampsakko`: Hk/HkSak.cpp:244; Tp/TpLaitteet.cpp:1670 (device message path).
- `lah_tv`: Hk/HkDat.cpp:1236 (inside `tallenna`).
- `kirjvaihevalinta`: Hk/HkIx.cpp:1998 (stage change).
- `initajat`: Hk/HkIV.cpp:4171, 4231; Hk/HkIx.cpp:1970.
- `AjatTiedostoon`: cbHk/UnitAjanotto.cpp:1389, HkConsole/HkCons2.cpp:1263 (menu export).
- `kirjKilpDat`: cbHk/UnitExport.cpp:532, HkConsole/HkCons0.cpp:358 (export).
- `teeBackupkopio`: cbHk/UnitTKanta.cpp:1397 only.
- `luepakettiTCPSI`: started per TCP-SI connection (Tp/TpComY.cpp:1958-2018).

Phase codes:
- K: recurring during a competition (result/split entry, emit read, communication, autosave/auto print/auto export, stage change, reconnect).
- S: startup or config load.
- L: shutdown.
- U: user-triggered one-off (menu, import/export, maintenance).
- "lib": library code (TextFl, XML writers, `Wrttext`) used by both K and U.
- "U/K": the XML export writers (HkXml*, VXml*), used from menus (U) and from `autofile` kohde 'X' (K).
- "X": file not built.
- "?": unclassified.

### 2.2 Totals (rows; the 3130 call sites)

| phase | acquire (file/sock/handle/gdi/dbbox) | acquire assigned-unchecked | release | persistent writes | writes ignored or assigned-unchecked | `new TextFl` | malloc-family | malloc assigned-unchecked | writeerror | exit | all rows |
|---|---|---|---|---|---|---|---|---|---|---|---|
| S | 31 | 3 | 14 | 7 | 4 | 29 | 54 | 18 | 96 | 0 | 370 |
| K | 34 | 8 | 65 | 114 | 52 | 21 | 55 | 17 | 188 | 0 | 699 |
| U | 65 | 23 | 111 | 175 | 132 | 117 | 44 | 23 | 157 | 2 | 1546 |
| L | 1 | 0 | 14 | 3 | 2 | 0 | 0 | 0 | 3 | 2 | 29 |
| lib | 1 | 0 | 4 | 37 | 29 | 2 | 7 | 5 | 10 | 1 | 127 |
| U/K | 0 | 0 | 0 | 12 | 12 | 15 | 0 | 0 | 13 | 0 | 117 |
| ? | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 12 | 0 | 31 |
| X (not built) | 28 | 4 | 31 | 58 | 26 | 2 | 9 | 7 | 21 | 2 | 211 |
| sum | 160 | 38 | 240 | 406 | 257 | 186 | 169 | 70 | 500 | 7 | 3130 |

("persistent writes" = `file:write` + `dbbox:write` + `textfl:method-write`; sockets excluded; `socket:acquire` is included in the acquire column.) The writeerror column counts definitions too (S6). The "ignored" counts are inflated by S1 (they are lower bounds for "ignored" in the checked classes).

Functions (file, function) containing a persistent write or an unchecked non-memory acquire: 235 in total. By phase: K 78, U 85, U/K 11, lib 19, S 6, L 2, X 34.

### 2.3 K-phase functions with a persistent write or unchecked acquire (78; grouped)

Check state is the lexical `use` class, corrected where I read the code.

| group | functions (file:first line) | checked? |
|---|---|---|
| KILP.DAT layer | dbboxm/dbbox1.cpp putrec :94, addrec :249, addrec0 :258, deleterec :265, updatedescr :156; Hk/HkDat.cpp write :588, addfrec :633, delfrec :646; Juk/vdat.cpp putrec_i :155, addfrec :185, delfrec :200; Juk/vkilp.cpp putrec :239 | `putrec`, `write`, `putrec_i` ignored; `addrec`, `deleterec`, `addfrec` are void |
| emit file | Hk/HkEmit.cpp putem :1612, addem :1676; Juk/VEmit.cpp putem :1464, addem :1518; Hk/HkIV.cpp tall_emit :1140; Juk/VIv.cpp tall_emit :1090 | `putem` checked; `fflush` ignored; `tall_emit` ignored (`emittallfile`, GUI test tool) |
| AJAT queue | Hk/HkAjat.cpp writeaika :338, writeajat :372; Juk/vajat.cpp writeaika :317, writeajat :349; Hk/HkIV.cpp initajat :4122; Juk/VIv.cpp initajat :2787 | checked; see B-17/B-18 |
| COMFILE queue | Tp/TpComY.cpp buflah :500-509, lahetapaketti :1484, kirjaahyv :1601, merk_uusinta :1153, lah_tiedosto :2965, tark_tiedosto :3013 | partly checked |
| SQL backup | cbHk/UnitMySQL.cpp kirjSQLTh :687; ViestiWin/UnitMySQL.cpp kirjSQLTh :634-659; both kirjKilpSQL | ignored; `er` assigned and never tested |
| logs, debug | Tp/TpApu.cpp:880, Juk/VInit.cpp:154, cbTp/TpApu.cpp:397 `kirj_err_file`; Tp/TpComY.cpp put_profile :251; Hk/HkApu.cpp wkirjloki (via `sendwline`) | ignored |
| result and auto output | Hk/HkTulostus.cpp puts_f :1484, writeline :1526; Juk/VTulostus.cpp writehtml :1369, writeline :1392, puts_f :1408; tputilv2/Wrttext.cpp sendline/sendwline/sendchars/sendwchars | checked against WEOF (B-29); `Wrttext` returns the value |
| XML auto export | Hk/HkXml*.cpp, Juk/VXml*.cpp, `xml_put.cpp`/`wxml_put.cpp` (14 functions), `xmlparser.cpp` writexml | ignored (all) |
| device/comms | com/winCom.cpp wrt_ch :212, wrt_st :240; Tp/TpLaitteet.cpp lue_* (debug `dbgfile`, checked); com/WinUDP.cpp openport* :320, 381, 760, reconnectTCP :877 (socket unchecked) | see D list |
| stage change | Hk/HkInit.cpp kirjvaihevalinta :322 | ignored |
| operator window | ViestiWin/UnitGAZ.cpp lahriviCPower :921, taulu_tied :257; cbHk/UnitTaulu.cpp taulu_tied :246; ViestiWin/UnitLuenta.cpp NewCardHandler :437 | checked/ignored (display files, card-reading log) |

## 3. Persistent competition data: every writer (task 3)

"Checked" = the write's return value is tested. "Failure" = what the code does. Registry IDs are in the last column; I did not re-analyse them. In every case the data is held in memory and written second ("memory-first") unless noted.

| # | data | file:line | function (phase) | checked? | on failure | registry |
|---|---|---|---|---|---|---|
| W1 | KILP.DAT | dbboxm/dbbox1.cpp:84-100 | putrec (K) | bytes written tested, `WriteFile` return not; returns -1 | `perr` modal dialog "Continue execution?" Yes/No, "No" calls `lopetus()` (tputilv2/perr.cpp:51-58, 92-94 wperr) | H-015 |
| W2 | KILP.DAT | dbboxm/dbbox1.cpp:227-251 | newrec/addrec (K) | `putrec` and `updatedescr` results ignored | `numrec++` happens first (`*r = datf->numrec++` :232); the header is written after, also ignored | H-015 (range) [verified: read] |
| W3 | KILP.DAT | dbboxm/dbbox1.cpp:262-270 | deleterec (K) | ignored | free-list pointers change in memory regardless | H-015 |
| W4 | KILP.DAT | dbboxm/dbbox1.cpp:214-222 | closefile (L) | `putrec` and `CloseHandle` ignored | header lost | - |
| W5 | KILP.DAT, HK | Hk/HkDat.cpp:581-595, 609-624 | kilptietue::write, PUTREC (K) | `putrec` ignored (:588), `write` always returns 0 (:594) | memory is already updated (`kilparr[d] = *this` :616) | H-015 |
| W6 | KILP.DAT, HK | Hk/HkDat.cpp:626-651 | addfrec, delfrec (K) | `addrec`/`deleterec` are void | as W2/W3 | H-015 |
| W7 | KILP.DAT, relay | Juk/vdat.cpp:151-162 | putrec_i (K) | `putrec` ignored (:155); `FlushFileBuffers` (:157) ignored, only if `kirjheti & 1` | as W1 | H-015, H-047 |
| W8 | KILP.DAT, relay | Juk/vdat.cpp:178-192 | addfrec (K) | `addrec` void; the reopen `openfile` (:188) is not checked; `EnterCriticalSection` twice (:183, :190) | if the reopen fails, `datf2.hDatf` is closed (`closefile` :187) and later `putrec` calls fail with dialogs [inferred] | H-040, H-047; the unchecked `openfile` is new detail |
| W9 | KILP.DAT, relay | Juk/vkilp.cpp:239-250 | kilptietue::putrec (K) | `PUTREC` writes memory; `putrec_i` only if `tiedostoon` | memory only otherwise | C-1 `tallenna` |
| W10 | KILP.DAT copy | Hk/HkDat.cpp:1427-1446; Juk/vdat.cpp:948; cbHk/UnitExport.cpp:466-483 | kirjKilpDat (U) | `makefile` not checked; `addrec0` void | dialogs per record; shared `tarecbuf` (H-051) | H-051, H-046 |
| W11 | emit file, HK | Hk/HkEmit.cpp:1600-1625, 1657-1691 | putem, addem (K) | `_lseek`/`_write` checked; `iowerr` -> wperr modal; returns 1 | `addem` returns 1; `emithead` is not advanced; the card is not indexed; no retry in these functions. The caller of `addem` is not traced here: [tuntematon] | H-011, H-045, H-053 |
| W12 | emit file, relay | Juk/VEmit.cpp:1452-1473, 1518 | putem, addem (K) | checked (`err = 1`) | `writeerror_w(..., 0)` blocks until acknowledged, inside `emit_CriticalSection` (entered :1458 unless `inCrSe`) | H-011, T-032 |
| W13 | emit file open | Hk/HkEmit.cpp:3050-3060, 1371; Juk/VEmit.cpp:3607 | lueemitfile (S) | `_wopen` checked; `_wfdopen` and `setvbuf` not checked | `setvbuf(NULL, ...)` if `_wfdopen` fails [inferred] | H-105 (reopen part) |
| W14 | AJAT queue, HK | Hk/HkAjat.cpp:326-344 | writeaika (K) | `_lseek`/`_write` checked; `iowerr` modal; `return` | no retry; the row stays only in memory until a later `writeajat` | H-054 (relay), H-117 |
| W15 | AJAT queue, HK | Hk/HkAjat.cpp:348-382 | writeajat (K) | `_write` checked (:372); on failure `break` | `gettm(fp1++)` at :369 advances `fp1` before the write, so rows after a failed write are marked as persisted | new candidate |
| W16 | AJAT queue, relay | Juk/vajat.cpp:327-358 | writeajat (K) | `_write` checked; `break` | `fp = fp1` is set after the loop (:356) regardless | H-054; new detail |
| W17 | AJAT queue | Hk/HkAjat.cpp:384-411; Hk/HkIV.cpp:4100-4137; Juk/VIv.cpp:2787-2800 | suljeajat (L), initajat (S/K) | `_chsize` ignored (:402); `_wfdopen` unchecked (:4134); `fclose(fafhandle)` :403 | `fclose(NULL)` if `_wfdopen` failed [inferred] | H-054 |
| W18 | COMFILE.DAT disk queue | Tp/TpComY.cpp:491-516 | buflah (K) | `calloc` unchecked (:498); first `_write` stored in `pos`, tested `> 0`; the slot `_write` checked | message, then `cjseur[cn]++` regardless (:516) | H-044 |
| W19 | COMFILE.DAT read/ack | Tp/TpComY.cpp:1464-1491, 1593-1605 | lahetapaketti, kirjaahyv (K) | checked, message only | `aktpak[cn] = cjens[cn]` and `cjens[cn]++` regardless; the dialog is shown while holding `outb_CriticalSection` | H-044, T-032 |
| W20 | COMFILE file send | Tp/TpComY.cpp:2931-2989 | lah_tiedosto (K/U) | `fwrite` ignored (:2965) | leak of `lahfile` (C-2) | - |
| W21 | received files | Tp/TpComY.cpp:3003-3035 | tark_tiedosto (K) | `fopen` result tested at :3021, `fwrite == 1` tested | `fclose` and NULL on a short write; no message [verified: read, :3023-3031] | H-071 |
| W22 | SQL backup queue | cbHk/UnitMySQL.cpp:645-720; ViestiWin/UnitMySQL.cpp:610-659 | kirjSQLTh (K) | `fseek`/`fread` results assigned to `er`, never tested; `fwrite`/`fflush` ignored | open failure sets `varmistaSQLsiirto = false` silently (:691-692) | H-014, H-043, H-100 |
| W23 | runtime error log | Tp/TpApu.cpp:852-883 | kirj_err_file (K) | `_wfopen` result tested; `fputws` ignored | log lost | - |
| W24 | event log (loki) | Hk/HkApu.cpp:546-590; Juk/vApu.cpp:36,78 | wkirjloki (K) | `sendwline` result (:579, 586) and `wrt_st_TCP` `nsent` (:576, 583) unused | log line lost | - |
| W25 | result and auto files | Hk/HkTulostus.cpp:1477-1533; Juk/VTulostus.cpp:1369-1411 | puts_f, writeline, writehtml (K) | `WriteLine(..) == WEOF` | effective only for UTF-16 (B-29) | - |
| W26 | auto output files | Hk/HkTulostus.cpp:7066-7349; Juk/VTulostus.cpp:7137-7341 | autofile (K) | open checked at :7110 (message, `return 1`); per-series opens at :7217, :7276 only `continue` | truncates in place (`openprfile` mode "w+t", Wrttext.cpp:166 `mode[2]`); `system(autokomento)` runs after the write at :7345 | - |
| W27 | TextFl close | tputilv2/TextFileClass.cpp:312-317, 367-372 | ~TextFl, Close (lib) | `fclose` result ignored | buffered write errors are invisible | - |
| W28 | XML exports | tputilv2/xml_put.cpp, wxml_put.cpp, xmlparser.cpp:494-536; Hk/HkXml*.cpp; Juk/VXml*.cpp | writers (U/K) | all `WriteLine` ignored | silent | - |
| W29 | config/stage | Hk/HkInit.cpp:310-325 (:322) | kirjvaihevalinta (K) | `WriteLine` ignored; the `TextFl` is created and `IsOpen`-tested before | silent | - |
| W30 | series/params | Hk/HkSrj.cpp:609-638; Juk/vSarjat.cpp:494-527 | kirjsarjat (U) | HK ignored; relay `er` assigned but overwritten (:524/527) | silent | C-3 |
| W31 | TV and display files | Hk/HkTV.cpp:154 (`fputws`); ViestiWin/UnitGAZ.cpp:257; cbHk/UnitTaulu.cpp:246 | lah_tv, taulu_tied (K) | brace-less `if (tv_test)` (:153) | silent | - |
| W32 | test/diagnostic | Hk/HkIV.cpp:1139-1143; cbTp/UnitEmiTag.cpp:233, UnitMTR.cpp:228 | emittallfile (U) | `_wfopen` unchecked; `fwrite` ignored | silent | - |
| W33 | backup copy | Tp/TpApu.cpp:156-170 | teeBackupkopio (U) | `_wfopen` NULL branch calls `fclose(fl)` (:163-164) | `fclose(NULL)` on every normal path; MSVC ends the program, C++Builder returns EOF (only called from cbHk/UnitTKanta.cpp:1397, a C++Builder build) | new (T-020 pattern) |
| W34 | exports to file | cbHk/UnitKisaLuett.cpp:428-434 | CopyFile x3 (U) | not checked | silent | - |

Notes:
- No `_commit` exists anywhere in the scanned code. `FlushFileBuffers` appears once (Juk/vdat.cpp:157). The emit, AJAT and COMFILE writers use `_write` on a file descriptor; the `fflush(FILE*)` calls after them (HkEmit.cpp:1676 etc.) are no-ops on streams set unbuffered with `setvbuf(..., _IONBF, 0)` (HkEmit.cpp:3058). KILP.DAT is opened with `FILE_FLAG_WRITE_THROUGH` (dbboxm/dbbox1.cpp:101 `makefile`, `openfile`) [verified: read]. So a failed OS-cache flush is not visible in any of them [inferred].
- `kirjheti` default is 0xffffffff (Hk/HkGlobals.cpp:350, Juk/VGlobals.cpp:310), so all flush bits are on by default.

## 4. Target lists

### 4.1 List B - persistent-data error paths to analyse (30 rows)

| # | file:line | function | phase | reason |
|---|---|---|---|---|
| B-1 | dbboxm/dbbox1.cpp:84-100 | putrec | K | `WriteFile` result ignored; dialog and -1; all callers ignore -1 (W1) |
| B-2 | dbboxm/dbbox1.cpp:227-251 | newrec, addrec | K | `numrec++` before the write; header write ignored (W2) |
| B-3 | dbboxm/dbbox1.cpp:262-270 | deleterec | K | free list changes in memory regardless (W3) |
| B-4 | dbboxm/dbbox1.cpp:214-222 | closefile | L | final header write and `CloseHandle` ignored (W4) |
| B-5 | Hk/HkDat.cpp:581-624 | kilptietue::write, PUTREC | K | memory-first; write failure invisible to caller (W5) |
| B-6 | Hk/HkDat.cpp:626-651 | addfrec, delfrec | K | as B-2/B-3 (W6) |
| B-7 | Juk/vdat.cpp:151-162 | putrec_i | K | `putrec` and `FlushFileBuffers` ignored (W7) |
| B-8 | Juk/vdat.cpp:178-192 | addfrec | K | unchecked reopen `openfile` (:188), double lock entry (W8) |
| B-9 | Juk/vkilp.cpp:239-250 | kilptietue::putrec | K | what actually writes to disk in relay; memory-first (W9) |
| B-10 | Hk/HkEmit.cpp:1600-1625, 1657-1691 | putem, addem | K | what the caller does with `res = 1` and the lost card (W11) |
| B-11 | Juk/VEmit.cpp:1452-1473 | putem | K | dialog inside `emit_CriticalSection` (W12) |
| B-12 | Hk/HkEmit.cpp:3050-3060 | lueemitfile | S/U | `_wfdopen`/`setvbuf` unchecked (W13) |
| B-13 | Hk/HkAjat.cpp:326-344 | writeaika | K | no retry; row only in memory (W14) |
| B-14 | Hk/HkAjat.cpp:348-382 | writeajat | K | `fp1` advanced before the write (W15) |
| B-15 | Juk/vajat.cpp:327-358 | writeajat | K | `fp = fp1` after a failed write (W16) |
| B-16 | Hk/HkAjat.cpp:384-411; Juk/vajat.cpp:392 | suljeajat | L | `_chsize` ignored; `fclose(fafhandle)` (W17) |
| B-17 | Hk/HkIV.cpp:4100-4137; Juk/VIv.cpp:2787-2800 | initajat | S/K | whole-file rewrite at stage start, `_wfdopen` unchecked (W17) |
| B-18 | Tp/TpComY.cpp:491-516 | buflah | K | `calloc` unchecked; queue pointer advances (W18) |
| B-19 | Tp/TpComY.cpp:1464-1491, 1593-1605 | lahetapaketti, kirjaahyv | K | read and ack failures only message (W19) |
| B-20 | Tp/TpComY.cpp:2931-2989 | lah_tiedosto | K/U | `fwrite` ignored (W20) |
| B-21 | cbHk/UnitMySQL.cpp:645-720; ViestiWin/UnitMySQL.cpp:610-659 | kirjSQLTh | K | `er` never tested; silent disable (W22) |
| B-22 | Tp/TpApu.cpp:852-883 | kirj_err_file | K | log write ignored (W23) |
| B-23 | Hk/HkApu.cpp:546-590; Juk/vApu.cpp:36,78 | wkirjloki | K | log lost silently (W24) |
| B-24 | Hk/HkTulostus.cpp:1477-1533; Juk/VTulostus.cpp:1369-1411 | puts_f, writeline, writehtml | K | WEOF check ineffective for 8-bit types |
| B-25 | tputilv2/TextFileClass.cpp:312-317, 367-372 | ~TextFl, Close | lib | `fclose` result ignored (W27) |
| B-26 | Hk/HkTulostus.cpp:7066-7349; Juk/VTulostus.cpp:7137-7341 | autofile | K | in-place truncation, silent per-series open failure, `system` after failure (W26) |
| B-27 | tputilv2/wxml_put.cpp, xml_put.cpp, xmlparser.cpp:494-536 | XML writers | U/K | all writes ignored (W28) |
| B-28 | Hk/HkInit.cpp:310-325 | kirjvaihevalinta | K | stage selection file, silent (W29) |
| B-29 | tputilv2/TextFileClass.cpp:546, 575 and :536-582 | TextFl::WriteLine | lib | return value semantic: see below |
| B-30 | tputilv2/Wrttext.cpp:147-277 (:166, :170) | openprfile | K | default type '8'; which `trlate` value callers pass decides whether the file type is 'W' |

B-29 detail, the central semantic finding [verified: read]: `WriteLine` returns `nwritten`. For type 'W' (UTF-16) it is the `fputws` result (WEOF = 0xFFFF on error, TextFileClass.cpp:546). For 'V' it counts characters (:551). For '8','U','A','O' it is the `fputs` result (0 or EOF = -1, :575). The callers compare `== WEOF`. `WEOF` is `((wint_t)(0xFFFF))` with `wint_t` an unsigned short (UCRT corecrt_wctype.h:21, corecrt.h:605), so it never equals -1. `openprfile` creates `TextFl(prtfname, mode[append], merk)` with `merk = L'8'` unless `trlate` is passed (Wrttext.cpp:166-170). `autofile` passes `aftulparam.merkit`, which is 'A' or '8' (Hk/HkTulostus.cpp:7102-7105, 7183). So the auto/result file write errors in `puts_f`/`writeline`/`writehtml` are not detected. Whether other callers pass 'W' is [tuntematon].

Left out of B: the GUI form writers (about 100 U-phase functions such as UnitExport, UnitHylkRap, UnitSarjatiedot; all `WriteLine` ignored, user-triggered); `kirjvaliajat`, `writevafile` (U); `tall_ampsakko` (dead branch, S12); the XML export call chains above the writers; the debug `dbgfile` rows (Tp/TpLaitteet.cpp; they already null the handle on a write error but do not close it); the unbuilt files.

### 4.2 List C - acquire/release pairs that may leak (K-phase; 18 rows)

Few leaks were confirmed. The K code mostly allocates and frees in balanced pairs (I read `putfld`, `writeline`, `autofile`, `tulostasarja`, `reconnectTCP`, `TCPrcvTh`).

| # | file:line | function | phase | reason |
|---|---|---|---|---|
| C-1 | Tp/TpComY.cpp:1973-1976, 2017 | luepakettiTCPSI | K | `sifile` `FILE` is not closed on the early `return` at :1976 (if `!comopen[cn]`); `fclose(sifile)` at :2017 with NULL if `fopen` failed. One handle per connection thread start [verified: read] |
| C-2 | Tp/TpComY.cpp:2931-2936 | lah_tiedosto | K/U | `lahfile` is not closed when `__FCOMFILE__.TMP` cannot be opened (`return 1` at :2936) [verified: read] |
| C-3 | tputilv2/Wrttext.cpp:249-262 | openprfile | K | raw-file path: `_wopen` handle leaks when `_wfdopen` returns NULL (`free(prf[nprf])` only) [verified: read] |
| C-4 | tputilv2/Wrttext.cpp:588 (and :543-544) | selectfontGDI | K | `CreateFontIndirectW` and no `DeleteObject` anywhere in the code base (grep: 0). The handle is cached in `fnt->hfont`, so growth is bounded unless the font structures are re-initialised per job: [tuntematon] |
| C-5 | Tp/TpComY.cpp:498-501 | buflah | K | `calloc(maxyhteys*combufsize, 20)` per queue extension, unchecked; transient 20 x the bytes written; NULL passed to `_write` |
| C-6 | Hk/HkTulostus.cpp:1520-1531, 1294-1301 | writeline, putfld | K | one allocation per printed line or field, balanced by `free`; `calloc` result unchecked (NULL then `wcscpy`) |
| C-7 | com/WinUDP.cpp:280-281, 740-743 | openportUDP, openportTCP | K | `malloc` result `memset` immediately without a test; `socket()` unchecked at :320, 381, 760, 877 (reconnect) |
| C-8 | com/WinUDP.cpp:874-877 | reconnectTCP | K | closes the previous socket at :874 and opens a new one; a failed `connect` leaves one open socket until the next attempt: bounded, not a leak |
| C-9 | Hk/HkIx.cpp:1586, 1636; Juk/VIx.cpp:1102-1105 | teejarj | K (stage change) | unchecked `calloc`/`malloc`; `emitjarr` and `bdg_kno` arrays are leaked on each re-run (H-133) |
| C-10 | Hk/HkEmit.cpp:2686-2689, 3170 (Juk) | laskeemitvaliajat | K | `rst`/`ev` allocated for the caller to free; unchecked; the callers (HkEmit.cpp:2771, 4388; cbHk/UnitKilpailijaEmit.cpp:162) were not traced for `free` |
| C-11 | cbHk/UnitMySQL.cpp:661-663 | kirjSQLTh | K | `InitializeCriticalSection` on every re-open; `sqlVarmFl` never closed (grep: no `fclose(sqlVarmFl)`) |
| C-12 | Hk/HkTulostus.cpp:7147-7316 | autofile | K | `aflst = new TextFl` freed at :7316; `aftulparam.sarjalista = new INT16[...]` allocated once (:7186), never freed [verified: read]: once only |
| C-13 | Hk/HkTulostus.cpp:7331 | autofile | K | `_beginthread(sendtcpfile, ..., &aftulparam)`; `lstf` is closed by the thread or not at all: [tuntematon] |
| C-14 | tputilv2/Wrttext.cpp:132-143, 295 | StartPrtPage, closeprfile | K | `ClosePrinter` and `DeleteDC` paths; `DeleteDC` results ignored (8 rows); no leak found |
| C-15 | com/WinUDP.cpp:560, 1075, 680-681 | TCPrcvTh, UDPstreamrcvTh | K | thread-lifetime `rcvbuf`; freed at :680 (H-096 covers the race) |
| C-16 | Hk/HkIV.cpp:826-832 | add_bdg_t | K | `etls` ring buffer allocated once (unchecked); not per event |
| C-17 | Tp/TpComY.cpp:2392; Tp/TpLaitteet.cpp:142 | initcom, opendbgfile | S | debug-only handles (`DBGXX`, `DBGFILE`) |
| C-18 | cbTp/UnitEmiTag.cpp:233; cbTp/UnitMTR.cpp:228 | emittallfile | U | unchecked open; the handle is closed at :114, :335 |

Left out of C: all unchecked `calloc` in GUI form handlers (UnitSeurGr, UnitVaGr, HkArv2, about 25 rows; U, once per click); one-time startup allocations (readcfg, `merkisto`, `lue_leimat`, `analradat`; mentions only); the 500+ `new`/`delete` rows (`new` throws, so only leaks on early return matter; none found in K by the per-function new/delete balance except `kirjSeurat`, U).

### 4.3 List D - state after an error report; exit/abort sites (36 rows)

| # | file:line | function | phase | reason |
|---|---|---|---|---|
| D-1 | Hk/HkAjat.cpp:216-222, 233-238; Juk/vajat.cpp:177-191 | tall_rivi | K | `errret = 1; goto poistu` after a modal message; callers do not check the return value (H-117) |
| D-2 | Hk/HkIV.cpp:3754-3760, 3784-3788 | tee_aika | K | "AIKATAULUKKO TAYSI" returns, but `tall_rivi` result is ignored and `kilp.tallenna` runs after it |
| D-3 | Hk/HkEmit.cpp:1236-1256 | add | K | "Kilpailijataulukko taynna - lisaysta ei tallennettu"; `exfl = 1; dataf = 0`; the console branch sets state, the `#else` branch differs (the script scans only the first branch) |
| D-4 | Hk/HkIx.cpp:1650-1651; Juk/VIx.cpp:1115 | teejarj | K (stage change) | OOM: message, `emitfl = 0`, stage change continues with emit checking silently off |
| D-5 | Hk/HkIx.cpp:1938-1944 | vaihdavaihe | K | rata/leimat load failure: message then `emitfl` stays on or off [tuntematon] |
| D-6 | Tp/TpComY.cpp:1470-1476 | lahetapaketti | K | disk-queue read failure: message; `aktpak[cn] = cjens[cn]` set; stale `obuf[cn]` may be sent; modal under `outb_CriticalSection` [verified: read] |
| D-7 | Tp/TpComY.cpp:500-516 | buflah | K | write failure: message then `cjseur[cn]++` (H-044) |
| D-8 | Tp/TpComY.cpp:1593-1605 | kirjaahyv | K | ack write failure: message; `cjens[cn]++` regardless |
| D-9 | Tp/TpComY.cpp:853, 1074 | luepaketti, luepakettiUDP | K | every 10th error a modal `writeerror_w(.., 0)` blocks the receive thread (error loop) |
| D-10 | Tp/TpComY.cpp:1215, 1941, 2161, 2249 | opencomTCP, opencomTCPin, openlukijaTCP, openTCPloki | K | open failure: `writeerror_w(msg, 2000)` then `Sleep(4000)` or retry: continuous message loop |
| D-11 | Juk/VEmit.cpp:1460-1466 | putem | K | blocking dialog inside the emit lock (B-11) |
| D-12 | Hk/HkTulostus.cpp:7217-7218, 7276-7278 | autofile | K | per-series `openprfile` failure: `continue` with `lstf == NULL`; `puts_f`/`writeline` return silently (:1481, 1517); no message |
| D-13 | Hk/HkTulostus.cpp:7341-7346 | autofile | K | `system(autokomento)` runs when `kirjoitettu` is true even if a write failed |
| D-14 | Hk/HkTulostus.cpp:1484-1489 | puts_f | K | on WEOF: `closeprfile(lstf)` without clearing `this->lstf`; the later `closeprfile(aftulparam.lstf)` (:7338) would double free [inferred; reachable only with a UTF-16 file] |
| D-15 | cbHk/UnitMySQL.cpp:691-692 | kirjSQLTh | K | backup file open failure: `varmistaSQLsiirto = false`, no message |
| D-16 | com/WinUDP.cpp:280-281, 740-743 | openportUDP, openportTCP | K | `malloc` unchecked, `memset` follows |
| D-17 | com/WinUDP.cpp:877-878 | reconnectTCP | K | `socket` result unchecked before `connect` |
| D-18 | com/winCom.cpp:212, 240 | wrt_ch, wrt_st | K | `WriteFile` result ignored (serial output) |
| D-19 | Juk/vdat.cpp:186-189 | addfrec | K | reopen of `datf2` not checked (B-8) |
| D-20 | Hk/HkDat.cpp:581-595 | write | K | returns 0 on write error (B-5) |
| D-21 | Hk/HkIV.cpp:4122-4136; Juk/VIv.cpp:2787-2800 | initajat | K (stage change) | `_write` failure `break`, then `_close` and reopen; `setvbuf(NULL)` possible |
| D-22 | Hk/HkTulostus.cpp:1520-1521; 1294-1295 | writeline, putfld | K | `calloc` NULL then `wcscpy` |
| D-23 | tputilv2/perr.cpp:51-58, 92-94; wperr | perr, wperr | K | modal `MessageBox` Yes/No; "No" -> `lopetus()`; blocks the calling thread |
| D-24 | cbTp/cbTpApu.cpp:467-490 | writeerror_w (GUI) | K | `SendMessage` then spins `Sleep(200)` until `perr_ptrs2.vast != 0` (blocks the caller; H-009, H-019) |
| D-25 | tputilv2/Writeerr.cpp:66-94 | writeerror_w (console) | K | `kesto == 0` waits for Enter (`readkbd`), so an unattended console blocks |

Exit and abort sites (all that exist in the scan, plus `lopetus()` callers):

| # | file:line | function | phase | what it loses |
|---|---|---|---|---|
| D-26 | Hk/HkInit.cpp:3830 | lopetus1 | L | `_exit(0)` (console only) after `suljeajat`, `closefile(&datf2)` (:3790), `closeprfile` of `lsttulparam` and `lokitulparam` (:3757, 3793). `_exit` skips CRT flushing of open `FILE`s not closed: `sqlVarmFl` (never closed), `err_file` is closed at :3711, `autotulostusparam.lstf` and `aftulparam.lstf` are not closed here [inferred] |
| D-27 | Juk/vApu.cpp:1035 | lopetus1 | L | `exit(0)` (console only); similar. `suljeajat` is under `#ifndef LUENTA` (:978) |
| D-28 | tputilv2/globals.cpp:35 | lopetus | L | `exit(0)`; not linked when HK/relay define their own `lopetus` [inferred] |
| D-29 | cbTp/UnitEmiTag.cpp:220; cbTp/UnitMTR.cpp:405 | PaivThread | U | `_endthread` (test tools) |
| D-30 | dbboxm/dbbox2.cpp:27-31, 59-62 | makedarr, newdrec | K | `malloc` fails: `writeerror` then `lopetus()` (index layer, T-018 dead code) |
| D-31 | tputilv2/perr.cpp:92-94 | perr | K | "No" in the dialog -> `lopetus()`; the KILP.DAT write that triggered it was not saved |
| D-32 | Hk/HkDat.cpp:1376-1380 | tallenna | K | "Indeksointivirhe, Ohjelma keskeytetaan": `inLopetus = 1; lopetus();` mid-update |
| D-33 | Hk/HkIV.cpp:4072-4075 | main loop | K | Esc -> `inLopetus = 1; lopetus()` |
| D-34 | Hk/HkCom32.cpp:660-662; Juk/VCom.cpp:1630-1632 | tark_extra | K | network command closes the program (`suljesuoraan = true; lopetus()`), console only |
| D-35 | Hk/HkInit.cpp:3421 | startup | S | `openprfile` failure -> `lopetus()` |
| D-36 | Juk/vSarjat.cpp:231 | startup | S | user chooses "E" -> `lopetus()` |

Left out of D: the roughly 100 K-phase `writeerror` calls that are plain operator messages followed by `return`/`continue` (e.g. "Tuntematon kilpailijanumero", Hk/HkIV.cpp:1419; Juk/VIv.cpp:1350-1381); about 40 `tark_*` "Tiedostot eri koneilla ei ole yhtapitavia" messages (Juk/VCom.cpp:593-749, Hk/HkCom32.cpp:342; H-016); the 12 lines of `lue_MTR`/`aseta_mtr` device messages; all U-phase handlers.

## 5. Coverage (task 5)

The scan covers 417 `.cpp/.c` files in SCAN_DIRS; 242 files (58 %) have no row. Per directory (files / files without rows):

| dir | files | no rows | expected? |
|---|---|---|---|
| Hk | 21 | 4 | `HkApuC.c`, `HkEnnTav.cpp`, `HkGlobals.cpp`, `HkTls.cpp`: no I/O calls in them [script] |
| Juk | 18 | 3 | `VEmitHtml.cpp`, `VGlobals.cpp`, `VRinnakkaisOsuus.cpp`: HTML goes through `tulprm->writehtml` |
| Tp | 10 | 5 | `mtrand.cpp`, `mttest.cpp`, `TpApuC.c`, `TpMuot.cpp`, `ZebraReader.cpp` (the latter uses buffers, no file calls: 0 `fopen`/`WriteFile`) |
| com | 3 | 0 | |
| cbTp | 37 | 22 | form files without file I/O; expected |
| tputilv2 | 168 | 147 | string/time/console helpers; `ioerr.cpp` and `perr.cpp` call `perr`/`wperr` (modal dialog) with no resource calls; expected |
| dbboxm | 11 | 4 | `DBboxm.cpp`, `findkey.cpp`, `nextkey.cpp`, `prevkey.cpp` (dead index layer, T-018); expected |
| cbHk | 76 | 33 | forms without I/O; expected |
| ViestiWin | 58 | 21 | forms without I/O; expected |
| HkConsole | 6 | 1 | `HkConsHelp.cpp`; expected |
| JukConsole | 9 | 2 | `laskhelp.cpp`, `lasktxt.cpp`; expected |

Not scanned at all: `Tests/` (fixtures redefine `perr`, `writeerror`), `RADStudio10/`, `VS/`, `include/` (headers). Expected.

What the script misses (grep counts over the 11 SCAN_DIRS `.cpp/.c`, comment-only lines excluded):

| pattern | count | note |
|---|---|---|
| `TFileStream`, `TStringList` (incl. `SaveToFile`), `TMemoryStream`, `TIniFile`, `TRegistry`, `GlobalAlloc`/`HeapAlloc`/`LocalAlloc`/`VirtualAlloc` | 0 | |
| `LoadFromFile` | 3 | cbTp/UnitKartta.cpp:65, 182, 225 (image loading, U) |
| `ofstream`/`ifstream`/`fstream` | 6 | cbHk/UnitYv.cpp:96, ViestiWin/UnitYv.cpp:111 and 3 `#include <fstream.h>`; U |
| `CopyFile`/`MoveFile`/`DeleteFile` | 3 | cbHk/UnitKisaLuett.cpp:428-434 (U), results unchecked |
| `_wremove`/`remove`/`_wrename`/`_unlink` | 1 | Tp/TpApu.cpp:165 (`teeBackupkopio`) |
| `fopen_s`/`_wfopen_s`/`_wsopen_s` | 7 | 5 in `TextFl` (2 in the built `TextFileClass.cpp`); `TextFl::Open` and the constructor are invisible |
| `_wfdopen`/`_fdopen` | 6 | 5 live (Hk/HkEmit.cpp:1371, 3057; Hk/HkIV.cpp:4134; Juk/VEmit.cpp:3607; Juk/VIv.cpp:2800; Wrttext.cpp:253), results unchecked at all but the last |
| `_chsize`/`SetEndOfFile` | 4 | results ignored |
| `system(`/`ShellExecute`/`CreateProcess` | 3 | Hk/HkTulostus.cpp:7345; tputilv2/lue_v_pv.cpp:48; 1 other |
| `WritePrinter`/`StartDocPrinter`/`OpenPrinter` | 3 | in `Wrttext.cpp`; results tested there |
| `_beginthread`/`CreateThread` | 65 | thread handles; no `CloseHandle` pairs (the `_beginthread` handle is closed by the CRT) |
| `FlushFileBuffers`/`_commit`/`fflush` | 20 | `FlushFileBuffers` only Juk/vdat.cpp:157; no `_commit` |
| `setvbuf` | 6 | |
| `fgetws`/`fgets`/`fscanf`/`fgetc` reads | 46 | not tracked; read paths were C-3 |
| SQL (`TSQLConnection`/`TSQLDataSet`/`ExecSQL`) | 18 | VCL calls; failures are exceptions |
| `__try`/`__finally`/`catch (`/`try {` | 35 | exception handling is sparse relative to the 500 write sites; C-7/T-036 cover VCL exceptions |
| `SetErrorMode`, `SetUnhandledExceptionFilter`, `_set_invalid_parameter_handler`, `signal(` | 0 | no process-level error handler (as T-020 states) |

## 6. Open questions

Q1. Is `Hk/HkTV.cpp` built (it is referenced from `HkDat.cpp:1236` and `HkIV.cpp:3650`, but is not in any project)? Possibly it is under a macro such as `TV` that no project sets.
Q2. Which `trlate`/type values do all callers pass to `openprfile`? That decides whether any result or auto file is 'W' and so has working write-error detection (B-29/B-30). The `lokitulparam` (log) caller is at Hk/HkInit.cpp:3418 (`trlst ? trlate : 0`).
Q3. What does the caller of `addem` (HK) do with `res == 1` (W11)? Is the card data retried or lost?
Q4. Is `HkTulostus.cpp:1470` `lopetus1` the one linked into the console builds, or the one in `tputilv2/globals.cpp:33`? I assume the program's own definition wins (D-28).
Q5. Do Fnt structures (`DefFnt`, `BoldFnt`, ...) get their `hfont` cleared per print job (C-4)? If yes, `CreateFontIndirectW` grows the GDI object count (limit about 10 000 per process) by one per font per job.
Q6. How many times per competition does `autofile` run and how large is the result? Needed to judge the in-place truncation window (B-26).
Q7. The phase classification of about 31 rows is open; the other classifications follow function names, not full call chains (2.1).
