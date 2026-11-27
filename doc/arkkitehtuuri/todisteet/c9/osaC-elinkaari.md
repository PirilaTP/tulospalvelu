<!-- Provenance: final report of defect-concurrency-auditor (Sonnet), C-9 step 1 part C, 2026-11-26; copied unmodified from the hand-back message by the main session. -->
# C-9 step 1, part C: resource lifecycle during a competition (read-only audit)

Tags: [verified: read] = source read; [inferred] = reasoned, not run; [unknown] = not established. Paths are relative to TPsource/V52/ unless they start with doc/. The note on GDI font handles (Q5) is in section 1.

## 0. Conclusion

- **Net result:** I found no resource leak per normal competition event that exhausts a resource within one competition day. The K-phase code is mostly balanced.
- **Printing:** GDI fonts, printer DCs, `PRFILE` slots and `TextFl` objects are balanced. Fonts are created once per font struct and cached. The printer DC stays open for the whole run and is closed at shutdown.
- **Serious defects:** there are no per-event leaks of that size. The notable defects are three lifecycle bugs:
  - A dangling `aftulparam.lstf` after the `sendtcpfile` thread (N-C2).
  - A dangling alias `autotulostusparam.lstf` after the printer is changed from the menu (N-C5).
  - A printer DC that is deleted after one spooler error and never reopened (N-C6).
- **Leaks that can add up:**
  - `aflst` in HK `autofile` leaks one `TextFl` per run, plus a CRT `FILE` if `AUTOFILE.LST` exists, for XML export in stage 2 or later (N-C1).
  - Sockets leak on TCP/UDP open-failure paths, mainly when a reader is down for hours (N-C3).
- **Refuted osaA claims:** C-1 (dead code), C-4 as a leak per print job, C-11 as repeated `InitializeCriticalSection`, and C-13 as "closed or not at all".
- **Three items for the registry owner.**
  - C-9 `teejarj` (the osaA wording): the stage-change leak is refuted. `vaihdavaihe` calls `teejarj(true)`, which reuses all arrays (HkIx.cpp:1563-1617, 1970). Only the `aloitus` rerun leaks (H-133).
  - Relay `autofile`: `Juk/VTulostus.cpp` `autofile` is not compiled. `AUTOFILE` is auto-defined only in `Hk/HkDef.h:53` (via `TCPSIIRTO`). It is defined in no Juk or ViestiWin project or header [verified: grep of .vcxproj, .cbproj and Juk/*.h]. So the 2016 double `closeprfile` and double `delete aflst` in that function (VTulostus.cpp:7152, 7200, 7298-7321) is dead code in every shipped relay program.
  - Numbering: candidate IDs below use the prefix N-C. The C-8 audit already used N-C1..N-C15 for other findings, so renumber them when registering.

## 1. Answers to the open questions

**Q5, do the `Fnt` `hfont` handles get recreated per print job? No.**
- `selectfontGDI` reuses a cached `fnt->hfont` (tputilv2/Wrttext.cpp:556-558). It creates a font only when the handle is NULL (:588).
- The handle is cleared only in these places:
  - `lueGDIfont`, when the six font values changed (Hk/HkMuotoilu.cpp:1371, Juk/VMuotoilu.cpp:771).
  - XML format load, unconditionally (Hk/HkMuotoilu.cpp:2853, Juk/VMuotoilu.cpp:1906).
  - The GUI settings dialog, when values changed (cbHk/MuotoiluUnit.cpp:444-468).
  - HK `bold_on` (Hk/HkTulostus.cpp:1580-1587), when `CurrFnt` differs from `BoldFnt` in family, pitch or size.
- `bold_on` has only two HK callers: Hk/HkEmit.cpp:4494 (always with `CurrFnt` = `RastiVaFnt`, Arial 10) and the `Bold_on` member. After the first call `BoldFnt` matches, so no recreation [verified: read].
- Relay `bold_on` (Juk/VTulostus.cpp:1475-1479) just selects the preset `BoldFnt`.
- `DeleteObject` does not occur anywhere in the code base, and neither do `CreatePen`, `CreateSolidBrush` or `GetDC` [verified: grep over *.cpp and *.h in TPsource/V52].
- GDI growth is therefore at most about 8-10 HFONTs per process, plus 6-7 per XML format load or settings change. The 10 000 limit is not reachable. Verdict: bounded.
- Side effect, not a leak: the cached font height is computed from the first DC's DPI (Wrttext.cpp:563-585). A second printer with a different DPI gets wrongly sized text [inferred].

**C-13, who closes `aftulparam.lstf` after the `sendtcpfile` thread?**
- The thread does, at Hk/HkTulostus.cpp:7060 (`closeprfile(tulprm->lstf)`, where `tulprm == &aftulparam`).
- It never nulls `aftulparam.lstf` (`closeprfile` takes the pointer by value). `autofile` leaves the pointer set (:7329-7332; the `= NULL` at :7338 is only in the `else` branch). See N-C2.

**C-10, do callers of `laskeemitvaliajat` free `rst`/`ev`?**
- Balanced in HK: Hk/HkEmit.cpp:2771-2794 (`ev` initialised NULL, freed, `rstva` is 0) and :4388 with frees at :4471-4472. No `return` between them.
- Balanced in relay: Juk/VEmit.cpp:3244/3271 and :5063 with frees at :5129-5130.
- Leaks: Juk/VEmitHtml.cpp:224 (no free of `eva` or `rstva` afterwards) and the error path at Juk/VEmit.cpp:3195-3199. See N-C7.
- The remaining callers (cbHk/UnitKilpailijaEmit.cpp:162/260, ViestiWin/OsuusTiedot.cpp:156/254, cbHk and ViestiWin UnitVaGr) free, and they are U-phase.

**C-11, `kirjSQLTh`?**
- `InitializeCriticalSection` runs once per process. It is guarded by `if (!sqlVarmFl)` (cbHk/UnitMySQL.cpp:657-661) and `sqlVarmFl` stays non-NULL.
- It repeats about every 300 ms only while `_wfopen("SQLjonoVarm.dat","a+b")` keeps failing. Each repeat leaks a small CS debug block (`SQL_CriticalSection` is never deleted) and reinitialises a CS that `kirjKilpSQL` may hold (:740-746).
- `sqlVarmFl` is never closed (grep: no `fclose`). One handle per process, bounded, closed by the OS at exit.

## 2. Pairs examined (verdict)

| # | Pair, file:line | Verdict | Note |
|---|---|---|---|
| C-1 | `luepakettiTCPSI` Tp/TpComY.cpp:1973-2017 | refuted (dead) | `SIIMPORT` defined in no project (grep of .vcxproj/.cbproj); T-012 |
| C-2 | `lah_tiedosto` Tp/TpComY.cpp:2931-2936 | leak on error path | `lahfile` leaked if `__FCOMFILE__.TMP` cannot be opened; N-C8 |
| C-3 | `openprfile` raw path Wrttext.cpp:249-262 | leak on error path | fd leaked if `_wfdopen` fails, then `setvbuf(NULL)` crashes; N-C8 |
| C-4 | `selectfontGDI` Wrttext.cpp:588 | bounded | see Q5 |
| C-5 | `buflah` Tp/TpComY.cpp:498-501 | balanced | transient `calloc(maxyhteys*combufsize,20)`, `free` at :502; NULL unchecked |
| C-6 | `writeline`/`putfld` HkTulostus.cpp:1520-1531 | balanced | read |
| C-7 | WinUDP.cpp:280, 740, 760 | not a leak by itself | unchecked allocations (C-4 scope); socket paths in N-C3 |
| C-8 | `reconnectTCP` WinUDP.cpp:874-881 | bounded | closes old socket, one open socket after a failed connect |
| C-9 | `teejarj` | stage change balanced; `aloitus` rerun is H-133 | HkIx.cpp:1563-1617 |
| C-10 | `laskeemitvaliajat` | HK balanced; relay leaks | N-C7 |
| C-11 | `kirjSQLTh` | bounded | see section 1 |
| C-12 | `autofile` `sarjalista` HkTulostus.cpp:7186 | one-time | |
| C-13 | `sendtcpfile` | closed by thread, pointer left dangling | N-C2 |
| C-14 | `StartPrtPage`/`closeprfile`/GDI path | balanced | `DeleteDC` after errors is safe because `hPrinter` is zeroed; the state loss is N-C6 |
| C-15 | `TCPrcvTh` `rcvbuf` | balanced | freed in closeportTCP :346 and in `TCPacceptTh`; `reconnectTCP` keeps the buffer; H-096 covers races |
| C-16 | `add_bdg_t` | one-time | |
| C-17 | `opendbgfile`, `initcom` | S, debug only | |
| C-18 | `emittallfile` | balanced | closed at cbTp/UnitEmiTag.cpp:114 and UnitMTR.cpp:335 |

Pairs I examined beyond the osaA list:

| Pair | Verdict |
|---|---|
| `openprfile`/`closeprfile` slot table `PRLKM=16` (Wrttext.cpp:38, 147-279) | balanced. Steady use is 3-5 slots: log, list, auto print, and `EmitTulPrm` open and close per print (cbHk/UnitEmit.cpp:3032/3054). Callers checked: cbHk/UnitEmit.cpp:3032-3054, TulostusUnit2.cpp:1093/1102, UnitHylkRap.cpp:277/286, UnitStatus.cpp:700-702. Two exceptions: HkTulostus.cpp:8449 (early `return(0)` after the printer is opened at :8293, user print, low) and N-C5. |
| `autotulostus()` HkTulostus.cpp:969-1030 and per-card `emit_rap` | balanced. DC and `PRFILE` persistent; per job only `initpage`/`StartDoc`/`EndDoc` (HkEmit.cpp:4544-4545). |
| `closeprfile` on `prno==1` (RAW printer, Wrttext.cpp:288) | no `EndPage`/`EndDoc`; only `ClosePrinter`. Not a handle leak. |
| `TextFl` class (tputilv2/TextFileClass.cpp:224-375) | balanced. Destructor closes the file and frees `FileName`; `ReadLine`/`WriteLine` temporary arrays are freed. |
| Per-function `new TextFl` scan, Hk/Juk/Tp/tputilv2 (my script) | the `return`s between `new TextFl` and `delete` in K code (AjatTiedostosta Hk/HkAjat.cpp:480 and Juk/vajat.cpp:457, `lueSeurat` HkApu.cpp:669, `luepiirit` HkTulostus.cpp:7583) were all `break`/`continue`. The 22 hits, mostly XML import readers, were not read function by function. |
| `vaihdavaihe` HkIx.cpp:1855-2005 | `suljeajat` then `initajat`, `femitfile` closed at :1909, `vapautaemitmuisti`, `teejarj(true)`, `opencomfile` (`_close` of the previous handle at TpComY.cpp:2268). All balanced. `AllocJarjSeur`/`DeAllocJarjSeur` HkIx.cpp:63-100 balanced. |
| `initajat` HkIV.cpp:3969-4140 | opens the fd with `_wopen`, closes and reopens it, then `_wfdopen`. Balanced; the `_close(fl)` bug is H-054. |
| `lueemitfile` HkEmit.cpp:3029-3060 | one-time open, `femitfile` reused on rerun. Balanced. |
| `httphaku` com/gethttp.cpp | WinHTTP session, connect and request handles closed on every path; buffers freed. Balanced. See N-C4 for its caller. |
| `openport`/`closeport` com/winCom.cpp:50-125, `opencomRS` TpComY.cpp:671-699 | `opencomRS` calls `closec` first, so one port handle at most. Bounded. |
| `TTilanneForm` refresh (cbHk/TulosUnit.cpp:591-597, :270-271, :430-431) | `Cells`, `sjjarj`, `ypJrj` are freed and reallocated. Balanced. |
| K-phase VCL handlers `NewCardHandler`, `DisplTimerHandler`, `LukumaaraReqHandler` | no `new`/`malloc` in the first two [verified: grep]. `LukumaaraReqHandler` was only grepped (no `new`/`malloc` in `Lukumaarat`). There are no `TTimer` objects at all. `loadIkkuna` forms are guarded by `if (X == NULL)` (cbHk/WinParamXml.cpp:340-420). |
| Menu handlers `Form = new T..Form(this)` without `delete` (about 150 hits from my regex over cbHk/ViestiWin/cbTp) | U-phase, not analysed per handler [unknown]. The single-instance ones are cached. |

## 3. Candidate findings

### N-C1 (HK, medium): `autofile` leaks one `TextFl` and one CRT FILE per run for XML export in stage 2 or later
- **Mechanism.** `do { ... aflst = new TextFl(autofileparam.aflstnm, L"rt"); ... } while (k_pv && aftulparam.kohde == L'X' && !yht++);` (Hk/HkTulostus.cpp:7120, 7147, 7314). `delete aflst` is only after the loop (:7315-7316), so on the second pass the first object is overwritten. `TextFl` opens a `FILE` in its constructor if `AUTOFILE.LST` exists. It is created for every `kohde`, including X, which does not use the list.
- **Evidence.** HkTulostus.cpp:7120-7147, 7314-7316 [verified: read]. `AUTOFILE` is auto-defined for all HK builds (Hk/HkDef.h:53). The relay `autofile` is not compiled (see section 0).
- **Growth.**
  - Always: one `TextFl` plus its `FileName` array, about 100 bytes per run.
  - Plus one FILE per run if `AUTOFILE.LST` exists in the working directory. That file is a leftover from earlier HTML per-series configuration.
- **Run rate.** `LukumaaratTh` runs `tarkAutoTulostus` (cbHk/WinHk.cpp:1799-1826). The interval is `afvali`, default 60 s (Hk/HkInit.cpp:994, 1042). The interval is configurable.
- **Limit and time to failure.**
  - UCRT stdio limit is 512 streams, so about 500 runs. That is about 8.5 h at 60 s and about 43 h at 5 min [inferred].
  - The C++Builder `bcc32` RTL's own limit is [unknown] and may be lower.
  - `dbboxm` uses `CreateFile` handles (dbboxm/dbbox1.cpp:178) and is not affected.
- **What fails when the limit is hit.** `_wfopen` returns NULL, so `TextFl::IsOpen` is false. `openprfile` then shows "Tiedoston avaaminen ei onnistunut" and returns NULL. The auto export fails every run. Other TextFl-based outputs (log and result files) fail too. The error message is shown on every run and the export itself stops.
- **Impact.** Auto XML export stops. The competition database is intact.
- **Severity.** Medium. It needs XML auto export, stage 2 or later, and the leftover `AUTOFILE.LST`. If the file does not exist, only the small object leaks.
- **Confidence.** [verified: read] for the mechanism; [inferred] for the 512 limit and time estimate.
- **To verify.** Run `autofile` with `kohde='X'`, `k_pv=1` and an `AUTOFILE.LST` present, 600 times, in an isolated probe, and count open streams.
- **Relation.** New. H-133 is a different rerun.

### N-C2 (HK, medium): `aftulparam.lstf` dangles after the `sendtcpfile` thread
- **Mechanism.** `autofile` starts `_beginthread(sendtcpfile, ..., &aftulparam)` when `fileyht >= 0` (HkTulostus.cpp:7329-7331). The thread ends with `closeprfile(tulprm->lstf)` (:7060), which frees the `PRFILE` and nulls only `pf->u.Fl`. `aftulparam.lstf` is never set to NULL, so the next `autofile` run sees a freed pointer.
- **Evidence.** HkTulostus.cpp:7060, 7329-7340. `closeprfile` is at Wrttext.cpp:279-300.
- **Effect on the next run.**
  - The entry guard (:7101) reads freed memory.
  - For `kohde` H or M with `!yksihtml`, where `lstf` was not reopened at :7108, `htmlloppu` and `closeprfile(aftulparam.lstf)` run on the stale pointer (:7172 and :7273-7274). That is a double free of the `PRFILE`.
  - For other cases `openprfile` overwrites the pointer (:7108) and nothing happens.
- **Limit.** One double free per run in the affected configuration.
- **What fails.** A double free of a `PRFILE` can corrupt the heap or crash the process (modern Windows heaps often abort on a detected double free) [inferred]. The effect depends on whether the block was reused.
- **Second state issue.** If `_beginthread` fails (returns -1), `lstf` is never closed and the guard at :7101 makes every later `autofile` return 1 silently for the rest of the process. A thread stuck in `wrt_st_TCP` has the same effect for as long as it is stuck [inferred].
- **Conditions.** `fileyht >= 0`, i.e. the TCP file transfer is configured (Hk/HkInit.cpp:1036-1038).
- **Severity.** Medium. Heap corruption is possible but it needs the TCP file transfer and the HTML per-series mode.
- **Confidence.** [verified: read] for the dangling pointer; [inferred] for the crash.
- **To verify.** Probe with `fileyht >= 0`, `kohde='H'`, `yksihtml=false` and a stub `wrt_st_TCP`; run `autofile` twice.
- **Relation.** New. H-096 is the accept thread. T-032 is the lock held across `writeerror_w`.

### N-C3 (HK and relay, medium by rubric, small practical impact): sockets leak on open-failure paths
Four separate mechanisms in `com/WinUDP.cpp`, all [verified: read].
- **(a) `openportTCP` client or server open failure.** The `if (retcode && TCPyhteys[yhtno])` block (WinUDP.cpp:812-817) frees `rcvbuf` and the struct without `closesocket` of `sockdata` (created at :760) or `sockmaster` (created at :789). One socket per failed attempt.
  - Failed-attempt rate by caller, assuming connect fails: Sirit and Zebra reader reconnect, once per about 5 s plus connect time (TpLaitteet.cpp:1810-1850, 2228-2233, Zebra at :932). A Sirit reconnect can leak two sockets per attempt (data and command). Reader first open and `openlukijaTCP` (Tp/TpComY.cpp:2128-2175) retry on `WSAECONNREFUSED`/`WSAEHOSTDOWN`, every 5 s for the first 5 failures, then roughly every 60 s, and after the 10th about 4 min. Timeouts and unreachable networks mostly end the thread (poistu) [inferred].
  - A refused connection (host up, port closed) leaks the fastest. Estimated worst case is about 1 400 sockets per hour per Sirit reader, about 700 per hour for a single connection. A timing-out connection leaks about 140 per hour per connection.
- **(b) `closeportUDP` (:191-205) closes only `socksrv`.** The client socket `sockcli` created at :320 is never closed (the only `closesocket` calls are at :197, 340, 342, 874). One socket per UDP open and close cycle, with cycles at shutdown and when a reader is closed.
- **(c) UDP half-open state on bind failure.** If `bind` fails (:293), `openportUDP` returns an error with the socket open and `UDPyhteys[yhtno]` allocated. A later `openportUDP` with the same `*hC` skips the whole initialisation block (:266-269) and returns 0, so the caller believes the port is open [verified: read].
- **(d) Server side TCP: accepted sockets are never closed on peer disconnect.** `TCPacceptTh` stores `sockdata = sock` (:659) and the next accept overwrites it. `TCPrcvTh` (:546-631) ends on `recv` <= 0 and never closes `sockdata` in the server case, only `closeportTCP` does. One leaked kernel socket (in CLOSE_WAIT) per peer reconnect.
- **Limit.**
  - (a) The client ephemeral port range of about 16 000 ports. Reaching it needs roughly 11 h of continuous refused attempts at the worst rate, or more than 4 days at the timeout rate [inferred].
  - (b), (c) and (d): one per cycle or reconnect, so hundreds at most. No practical limit.
- **What fails when the limit is reached.** `connect` fails with WSAEADDRINUSE/WSAENOBUFS for every new outgoing connection, including reconnects of working links, with log or `writeerror_w` messages. Silent in between.
- **Severity.** Medium for (a) (realistic error path, reader outage; no day-one exhaustion). Low for (b), (c), (d).
- **Confidence.** [verified: read] for the mechanisms; [inferred] for rates and limits.
- **To verify.** Loop `openportTCP` against a closed local port and count handles and ports with a probe (the TcpClientTests fixture approach, Tests/core/TcpClientTests.cpp, is suitable).
- **Relation.** New. H-027 covers the retry logic and the null dereference after a failed open; this finding is the socket leak in the same path. H-096 covers the close race.

### N-C4 (HK console, low-medium): `vahaku` HTTP poll error path uses `FormatMessage` without `LocalFree` and possibly an uninitialised pointer
- **Mechanism.** On a failed poll `vahaku` calls `FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, er, 0, (LPWSTR) &pMsg, ...)` and formats `%s` with `pMsg` (Hk/HkIV.cpp:3503, 3545-3547). There is no `LocalFree` anywhere in the HK or Tp code except in tputilv2/perr.cpp.
  - If a message exists, the buffer leaks, a small block per failed poll.
  - If `FormatMessage` fails, `pMsg` is uninitialised and `swprintf` reads through a garbage pointer. WinHTTP error codes 12xxx may have no system message [unknown].
- **Reach.** `EMITHTTP` is defined only in `VS/Hk/HkMaali/HkMaali.vcxproj`, not in the GUI `.cbproj` files [verified: grep]. The relay `vahaku` (Juk/VIv.cpp:2397-2402) prints only the error number and is fine.
- **Growth and limit.** Under 200 bytes per failed poll (poll interval `eThakuvali`). No limit is reachable; only the crash path matters.
- **Severity.** Medium for the crash path while the poll server is unreachable, which is realistic. Low for the leak.
- **Confidence.** [verified: read] for the missing free and the uninitialised read path; [unknown] whether `FormatMessage` fails for WinHTTP codes.
- **To verify.** Call `vahaku` against a closed port and watch for an access violation, or check `FormatMessage` for code 12029.

### N-C5 (HK, medium): `autotulostusparam.lstf` is a stale alias after "Tuloskirjoitin" is used
- **Mechanism.** Startup copies the whole struct: `autotulostusparam = lsttulparam;` (Hk/HkInit.cpp:3474), so `autotulostusparam.lstf` and `lsttulparam.lstf` point to the same `PRFILE`. The menu handler `TFormStatus::Tuloskirjoitin1Click` does `closeprfile(lsttulparam.lstf)` and opens a new one (cbHk/UnitStatus.cpp:698-706). Nothing updates `autotulostusparam.lstf`.
- **Effect.** The GUI auto-print activation (cbHk/TulostusUnit2.cpp:1389-1397) opens a new `PRFILE` only if `autotulostusparam.lstf == NULL`, which is false. `autotulostus()` (HkTulostus.cpp:969-1015) then copies the stale pointer (:982) and prints through freed memory.
- **Growth and limit.** One use after free per auto print.
- **What fails.** Use after free of `PRFILE` and its GDI parameters, so wrong output or a crash; silent until it happens [inferred].
- **Conditions.** Auto print enabled, and the result printer changed from the menu during the run. This only applies when `listfname` was not "NUL" at startup so that the startup copy holds a live pointer.
- **Severity.** Medium.
- **Confidence.** [verified: read] for the alias and the missing update; [inferred] for the symptom.
- **To verify.** Probe: copy struct, `closeprfile`, then `autotulostus()`.
- **Relation.** New. Related to H-133 style aliasing, but different.

### N-C6 (HK and relay, medium): a printer DC deleted after one error is never reopened
- **Mechanism.** On `EndDoc`, `EndPage` or write failure the code deletes the DC and sets `pf->u.wp.hPrinter = 0` (Wrttext.cpp:786-800, 805-812, 879-891, 927-945). The `PRFILE` stays in `lsttulparam.lstf` (or `EmitTulPrm`, `autotulostusparam`) with `prno == 2`. Every later call returns early without a message: `selectfontGDI` returns 1 (:545), `startdocGDI`/`startpageGDI` return 0 and `SendCharsToGDIPrinter` returns -100 (:895 area), all on `!hPrinter`. Callers only reopen when `lstf == NULL`.
- **Reach.** The result printer is opened once at startup (HkInit.cpp:3418, Juk/VInit.cpp:2692) and closed at shutdown. Emit auto print uses it as well (`emittulparam = &lsttulparam`).
- **Growth and limit.** None; it is a permanent loss until restart or the menu "Tuloskirjoitin" (cbHk/UnitStatus.cpp:698) re-opens.
- **What fails.** Auto result prints and per-card emit prints stop silently after one spooler error (printer offline, paper jam reported as `EndDoc` failure). No message after the first.
- **Severity.** Medium for competitions using auto print. This overlaps part D (state after error).
- **Confidence.** [verified: read] for the state; [inferred] for how often `EndDoc` fails in practice.
- **To verify.** Probe with a DC that fails `EndDoc`; check later `sendline` return values.

### N-C7 (relay, low): `laskeemitvaliajat` results leaked in relay HTML emit report; `rst` leaked on its error path
- **Mechanism.** `Juk/VEmitHtml.cpp:224` calls it for every runner row; `eva` and `rstva` are never freed afterwards (:195-295, [verified: read]). In Juk/VEmit.cpp:3171-3199 the error branch frees `ev` and sets `*eva = 0` but not the `rst` allocated at :3171-3174, and the callers test only `eva`, so `rst` leaks.
- **Growth.** About (`rastiluku`+1) records of a few tens of bytes per row. In total under 2 MB for 1 500 team-legs.
- **Severity.** Low. The function is the one where H-087 leaks `print_CS` and `rastiva_CS`, so the threads stop printing after the first call anyway.
- **Confidence.** [verified: read].
- **Relation.** Same function as H-087 (the CS leak); no overlap with its leak.

### N-C8 (low): rare error-path handle leaks
- `openprfile` raw path: if `_wopen` succeeds and `_wfdopen` returns NULL, the fd leaks and `setvbuf(NULL)` is then called on NULL (Wrttext.cpp:249-262). Needs fd or memory exhaustion.
- `lah_tiedosto`: `lahfile` is not closed when `__FCOMFILE__.TMP` cannot be created (Tp/TpComY.cpp:2931-2936). One FILE per attempt; the attempt is a user file send or a received file (callers cbHk/WinHk.cpp:2009 and others).
- `openprfile` for `prno = -2` is fine. HkTulostus.cpp:8449 `case L'Y'` empty `PisteLista` returns with the GDI printer open (U-phase) [inferred].
- Severity low; confidence [verified: read].

## 4. One-time startup leaks (mention only)
- `aftulparam.sarjalista = new INT16[...]` (HkTulostus.cpp:7186).
- `lueSeurat` allocates `Seura = new seuraTieto[...]` without freeing the previous array (Hk/HkApu.cpp:660-690). Also callable from the GUI.
- `analradat`, `merkisto`, `lue_leimat`, the `etls` ring buffer (HkIV.cpp:826-832), `emitjarr`/`bdg_kno` (H-133 on rerun).
- `InitializeCriticalSection(&tcp_CriticalSection[fileyht])` is called again in `startaf` (HkTulostus.cpp:7016) and in `opencomTCP` (TpComY.cpp:1247).
- `__FCOMFILE__.TMP` is never truncated or removed (lahtiedbuf, TpComY.cpp:2863, 2934); only disk growth per file transfer.

## 5. Unresolved questions
1. What is the stdio stream limit in the `bcc32` RTL used by `HkKisaWin.exe` and `ViestiWin.exe`? It sets the real events-to-failure for N-C1 and any FILE leak [unknown].
2. Is `AUTOFILE.LST` present in a typical competition directory with kohde X? Without it N-C1 costs only a small object per run [unknown].
3. How often does a Sirit or Zebra reader's TCP reconnect get "connection refused" instead of a timeout? It sets the rate for N-C3(a) [unknown].
4. Does `FormatMessage` with `FROM_SYSTEM` return text for WinHTTP codes 12029 and 12002? It decides whether N-C4 is a leak or a crash [unknown].
5. Does `vaihdavaihe` with exactly one open connection and `levypusk` open the new COMFILE for the new stage? The loop at HkIx.cpp:1976-1984 calls `opencomfile(1)` only from the second open port onward. That is persistent data (part B), mentioned only [unknown].
6. The `new T..Form` menu handlers without `delete`: whether any is used repeatedly in a K phase was not established.
7. Did all the "left out of C" GUI `calloc` rows stay U-phase? Not rechecked.

No files were written or modified.
