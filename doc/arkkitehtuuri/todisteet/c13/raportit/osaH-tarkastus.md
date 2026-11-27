# C-13 part H: independent overall review

Reviewer: independent-verifier (Opus), 2026-11-26. Read-only; nothing built or run.
Scripts run: `synteesi.py . <scratch> --tarkista` -> OK (177 risk rows, 35 package rows, 228 primary
members, no cycles, waves consistent with `riippuu`); `tarkista_auditointi.py .` -> OK (2423 refs).
Production code tree of `fc03b67` and `f7fe553` is identical (`git diff` empty).
Paths are relative to `TPsource/V52/` unless they start with `doc/`.

Verdicts: VAHVISTETTU (confirmed) / OSITTAIN (partly) / KUMOTTU (refuted) / EI TODENNETTAVISSA
(unverifiable statically). Severity: must-fix before commit / should-fix / note.

## Findings

### K1. H-033 is a network-default heap write ranked P4, outside the P1 change - MUST-FIX
- Claim: H-033 severity "matala", P4, primary KK-C13-10 (wave 2); riskiarvio huomio: "Read or write
  nature not re-traced; if it writes outside pv[] it belongs with H-072 (P1)".
- Verdict: KUMOTTU (the rating). `Hk/HkCom32.cpp:274` and `:277` call
  `tkilp.pv[ibuf->d.pv.pv].unpack0(...)` / `.unpack(...)` with the wire field `INT16 pv`
  (`Hk/HkDef.h:1261`), never checked. `pv` is a heap array of `kilpparam.n_pv` objects
  (`Hk/HkDat.cpp:209`). `kilppvtp::unpack0` memcpys about 20 fields into the indexed object
  (`Hk/HkDat.cpp:311-358`); `unpack` also writes through `va[i]`, a pointer read from the out-of-range
  object (`Hk/HkDat.cpp:362-367`). Negative indices are possible (signed). Reached for every KILPPVT
  packet (`Hk/HkCom32.cpp:901-902`); no earlier check of `d.pv.pv` exists (grep). A legitimate peer
  with more stages (configuration mismatch) also triggers it. By 9.2 (network, default settings,
  memory write) this is P1; it is one of only two `tietoturva=verkko` rows below P2. Rule 3 (unknown
  effect on a default network path -> worse case) was applied to H-061 but not here. C-2's "matala"
  has no justification anywhere (`doc/arkkitehtuuri/tiedonsiirto.md:284`).
- Correction: H-033 -> P1, severity korkea, primary package KK-C13-1 (PR 1 or 2), first batch. Update
  counts: P1 5, P4 48; first batch 26; KK-C13-1 P1:5 P2:1; KK-C13-10 P2:3; 9.3, 10, 6.1, 7.1 texts.

### K2. H-116 extension verified; H-153 not re-rated with it - should-fix
- Claim: errorMsgHandler treats ShowModal result 2 as exit; X returns mrCancel = 2; recorded as an
  extension of H-116, P3 -> P2.
- Verdict: VAHVISTETTU for the code. `cbTp/cbTpApu.cpp:445-463` (`retval == 2` -> `suljesuoraan = 1;
  Close();`); `cbTp/UnitSelect3.cpp:38-59` sets ModalResult only in button handlers; the form has no
  OnClose/OnCloseQuery and no Cancel button (`cbTp/UnitSelect3.dfm:195-262`); the OK-only variant only
  hides Button2 (`UnitSelect3.cpp:73-78`), so X still gives 2. `suljesuoraan` skips the exit
  confirmation and runs `lopetus1()` (`cbHk/WinHk.cpp:793-803`). mrCancel on closing a modal VCL form
  is [paatelty] (standard VCL). Both GUIs link the unit (`RADStudio10/HkKisaWin.cbproj:927`,
  `RADStudio10/ViestiWin.cbproj:528`). Extension rather than new id is defensible (one dialog class,
  one fix point); P2 fits 9.2(c).
- Gap: H-153 (P4, "only when exiting") becomes reachable by a routine X click: a worker blocked in
  `writeerror_w` while holding `tall`/`outb`, and `FormCloseQuery -> talljarj` waiting for the same
  lock = GUI hang (`cbTp/cbTpApu.cpp:451-462`, `cbHk/WinHk.cpp:793-803`; the H-116 row itself names
  H-153).
- Correction: re-assess H-153 with the same reasoning; give the error-dialog manifestation its own
  acceptance item and GUI step; record what X must mean per call site (osaB-juurisyyt.md 7 calls this
  an owner decision; omistajan-paatokset.md never mentions H-116). Consider H-116 for the first batch
  (one form, all GUI programs, routine trigger).

### K3. H-116 and the writeerror_w change touch the same handler without coordination - should-fix
- H-116 (KK-C13-14 PR 2) and the writeerror_w change (KK-C13-5 PR 3) both change
  `TFormMain::errorMsgHandler` and TFormSelect3, both in wave 2. Neither `riippuu` nor the
  "Ei rinnakkain" list (korjaussuunnitelma.md 4.4) links them; osaB 4.1 FT-1 says change them together.
- Correction: add the pair to "Ei rinnakkain" and order the PRs, or move H-116's error-dialog part
  into KK-C13-5 PR 3.

### K4. Dependencies stated only in prose; one violates the wave order - should-fix
- KK-C10-1 (wave 2) text: H-163 testing needs KK-C10-3 (H-165, wave 3), and H-014 waits for the
  KK-C13-8 flag decision. CSV `riippuu` = KK-C13-18 only, so the script cannot see it. 4.5 item 3 says
  "H-161 ennen H-160:ta tai sen kanssa" but the PR order is PR 1 H-160 + H-162, PR 2 H-161.
- KK-C13-4 (wave 1) PR 4 H-087 waits for KK-C13-5 (wave 2), which depends on KK-C13-4. Not a cycle
  under the "first series" rule, but H-087 (P2, korkea) is in practice wave 2+, while the wave table
  lists "KK-C13-4 (sarja a)" in wave 1 and sarja a is "korkean vakavuuden kohdat".
- KK-C9-3 (T-020 handler) must follow H-059 (KK-C13-13), H-078 (KK-C13-12), H-105 (KK-C13-3)
  (4.5 item 7); `riippuu` lists only KK-C13-18 (waves happen to fit).
- KK-C13-6: "H-047 korjataan yhdessa H-040:n ja H-148:n kanssa", but H-040 is wave 1 (KK-C13-4) and
  H-148 wave 3 (KK-C9-2).
- Correction: record member-level dependencies in a checked column (or note) and extend
  `synteesi.py --tarkista`; move H-163's release test to KK-C10-3 or H-165 to wave 2; swap KK-C10-1
  PR 1/2 or drop 4.5 item 3; state "H-087 = wave 2"; resolve the H-047 split.

### K5. "Safe in parallel" contradicts the H-008/H-060 coupling - should-fix
- `korjauskokonaisuudet.csv` lists KK-C13-2 and KK-C13-4 as parallel-safe, but H-008 (KK-C13-4 PR 1)
  and H-060 (KK-C13-2) share `Hk/HkCom32.cpp:647`, `Tp/TpLaitteet.cpp:1670` and `Hk/HkSak.cpp`
  (havainnot.md rows), and KK-C13-2's own text says they go in one series.
- Correction: drop the pair from `rinnakkain`; one PR (or a fixed order) for H-008 + H-060.

### K6. H-013's fix activates a never-run save-and-broadcast loop - should-fix
- Claim (KK-C13-3 compatibility): KILP.DAT unchanged, "uudelleenrakennus kirjoittaa muuttuneet
  tietueet samassa muodossa [paatelty]"; H-013 is in first-batch item 4 with H-129.
- Verdict: OSITTAIN. `Hk/HkIx.cpp:1834` `for (d = 1; i < nrec; i++)` calls `kilp.tallenna(d, 0, 0, 0,
  0)`. Fixed, every points-scored record is re-saved at each stage change, and `tallenna` with
  `kielto = 0` sends changed records to all peers (`Hk/HkDat.cpp:1285`, `:1293`, `:1300`, `:1307`) and
  may open dialogs (`kysyfl = !kielto`, `Hk/HkDat.cpp:952`). Peers may themselves be mid-stage-change
  (H-023, H-094, JS-5). This is a network-traffic and workflow change, and 7.3 lists no test for H-013.
- Correction: own PR, after deciding whether recalculation at a stage change should broadcast (the
  `kielto` value), with a two-station test; record the effect in KK-C13-3's compatibility field.

### K7. First batch: missing tests and environments - should-fix
- 7.3 names tests for 22 of 25 items; none for H-102 (race; needs a controlled-thread test), H-013
  (K6) or H-006 (infra), while 7.5(1) requires a green test for every item.
- H-159 (item 9) needs the TCP stub reader (regressiotestaussuunnitelma.md 5 item 9, OP-13), H-110
  and H-171 need GUI scripts (item 7); wave 0 creates only items 1-4. Wave-1 package acceptance also
  refers to loopback (item 6), multi-stage GUI runs (KK-C13-3) and real readers (KK-C10-2), none of
  which exist.
- KK-C13-3 acceptance (4) "Yksivaiheisen kilpailun toiminta ei muutu" contradicts fixing H-128 in the
  same series (single-stage walks change: skipped competitors reappear; the bib-lowering half of H-128
  has no fix principle at all). KK-C9-1 acceptance (2) "tavu tavulta sama" has no valid baseline for
  H-149 path (c) (MSVC Release aborts at the first cycle; ViestiWin effect unmeasured).
- Correction: add the stub reader and one GUI script to wave 0, or mark H-159/H-110/H-171 as manual
  verification with a written procedure; add tests or manual steps for H-102 and H-013; restate the two
  acceptance criteria; state that the 7.5(5) acceptance run uses HkKisaWin and ViestiWin (bcc32), since
  H-129 and H-149 are shared-core fixes whose bcc32 behaviour was never observed (Q91, Q95).

### K8. Owner-decision items of the root-cause report dropped without disposition - should-fix
- osaB-juurisyyt.md 7 lists fixes whose correct behaviour needs an owner decision: H-116 (meaning of
  X), H-022/H-023 (reject or merge, which stage), H-041/H-042 (grow buffers or refuse configurations),
  H-109 (deleting a competitor with a result), H-111/H-117, H-139/H-142, H-161 schema.
  omistajan-paatokset.md mentions none of H-116, H-109, H-022, H-023, H-041, H-042, H-117, H-139, H-161
  (grep count 0). First-batch criterion (4) "omistajan paatosta ei tarvita" is asserted for H-041; KK-
  C13-10 says OP-4 only concerns "hylkayksen nakyvyys".
- Correction: for each osaB 7 row add an OP item or one sentence why the plan's choice keeps current
  behaviour (e.g. H-041: buffers grow because the configuration is already accepted).

### K9. Priority consistency - K9a should-fix, K9b-K9d note
- K9a H-001 (P3) vs H-171 (P2): both deterministic memory overwrites with unknown effect on every use
  of an ordinary feature (biathlon relay text results with shooting positions,
  `Juk/VTulostus.cpp:2226-2228`; every ViestiWin competition open). H-001 is even ASan-reproduced.
  Raise H-001 to P2 or write the distinguishing criterion into 9.2.
- K9b Rule 1 (verification strength does not lower a verified mechanism) vs H-009, whose rationale
  ranks it below H-091 because H-091's edges were reproduced. Re-base the rationale on reachability.
- K9c H-064 (P3, deterministic NULL dereference at card reading with a setting) vs H-107 (P2,
  deterministic crash at the first card read with a setting): "console vs main GUI" is not a 9.2
  criterion.
- K9d P2 = 35 is defensible under 9.2(c), not inflated, but 14 P2 rows depend on a setting (asetus or
  verkko_asetus: H-008, H-023, H-024, H-027, H-028, H-055, H-073, H-091, H-093, H-107, H-129, H-150,
  H-151, H-160) while 9.2 P3 names "harvinainen asetusyhdistelma". Give an explicit within-P2 order
  (default configuration first).
- Checked consistent: the four P1 rows; H-026 and H-060 at P2 by 9.2(a); the two overrides are the
  only differences between `osaC/riskiarvio_luonnos.csv` and the final CSV (scripted diff: H-061
  P2 -> P1; H-116 P3 -> P2 and continuity ei -> pysayttaa); H-149/H-151 counterparts equal; no matala
  row above P4; every distribution in 9.1 and 9.3 recomputed and matching.
- Not P2/P1 on the evidence: H-014 (mirror only), H-041 (pre-event configuration), H-042 (justified
  exception), H-145 (needs I/O failure). H-009 is the closest to P2 (see K9b and K2).

### K10. JS-7 undercounts the stage-flag exit loops - should-fix
- Claim: `vaiheenvaihto` is the exit condition in five thread loops [todennettu].
- Verdict: OSITTAIN. At least seven while-loops exit on `!vaiheenvaihto`: `cbHk/TulosUnit.cpp:1893`,
  `cbHk/UnitEmit.cpp:838`, `cbHk/UnitMySQL.cpp:654`, `cbHk/UnitSakkoKierr.cpp:258`,
  `cbTp/UnitEmitSQL.cpp:62`, `cbTp/UnitNaytaTulos.cpp:100`, `Hk/HkIV.cpp:796` (plus a for-loop at
  `Hk/HkIV.cpp:3587`). KK-C13-8 PR 2 is "one decision for all threads", so the inventory matters.
- Correction: list the loops in KK-C13-8 and check which threads are restarted after a stage change
  (only H-014 and H-097 are registered).
- Other spot checks VAHVISTETTU: JS-2 zero `try` in `Hk/`, `Juk/`, `Tp/`, `dbboxm/` (grep); JS-4
  `writeerror_w` SendMessage + Sleep wait (`cbTp/cbTpApu.cpp:467-490`); JS-6 H-129 `uusinta` branch
  clears `idindex->ix` without resetting `nrec` (`Hk/HkIx.cpp:1600`; `tputilv2/dbindex.cpp:51-71`);
  JS-3 H-041 constant KILPBUFSIZE vs run-time kilprecsize, five stack buffers
  (`Hk/HkDat.cpp:565`, `:583`, `:629`, `:1433`; `cbHk/UnitExport.cpp:464`).
- Possible missing cross-cutting cause (note): no agreement of competition configuration between
  stations (n_pv, valuku, class lengths, version 520/521) behind H-023b, H-026, H-033, T-025 and part of
  H-016; JS-5 covers record versioning, not configuration agreement. "Isolated" is mostly right; H-086
  and H-043 share `TextFl`/file-mode handling with FT-13 and are not fully isolated.

### K11. Unexamined areas scheduled too late relative to JS-1 - should-fix
- J7-3's 152 parse candidates include 56 in the core (Hk 32, Juk 23, Tp 1;
  `doc/arkkitehtuuri/gui-liiketoimintalogiikka.md:442`), the same class as the P1 rows; the plan puts
  them in stage 3 (6.3). K1 shows a network-default index defect that stayed misrated through phase C.
  J7-5 (independent review of H-122..H-127 before fixing) is placed nowhere in the plan (grep).
- Correction: triage the 56 core candidates and all receive handlers (e.g. every EXTRA `d1` case,
  `Hk/HkCom32.cpp:626-660`) in stage 1 beside KK-C13-1; put J7-5 before KK-C13-15.

### K12. Summary and plan statements the registers do not support - should-fix
- auditoinnin-yhteenveto.md 10 item 2 ("every P2 has a bounded fix without format or workflow change,
  except H-025"): KUMOTTU for H-029 (KK-C10-4 is a decision package; "voi estaa nykyisen kayttotavan";
  acceptance "maaritellaan paatoksen jalkeen") and OSITTAIN for H-022/H-023 and H-116 (behaviour
  changes). The same applies to korjaussuunnitelma.md principle 2 and 6.1 "Valmis, kun ... jokaisella
  P1- ja P2-havainnolla on testi ... joka lapaisee" (H-029 and the H-071 path part need OP-4/OP-5).
- 6.4 lists KK-C10-4 in stage 4; the wave table, 6.1 and the CSV (`vaihe` 1, `aalto` 2) put it in
  stage 1. 6.2 calls KK-C13-12 a P3 package; the CSV says P4.
- Principle 2 ("the structural solution is not a prerequisite of any P1-P3 fix") vs KK-C13-23, which
  holds P3 H-016, H-030, H-039 with no bounded fix.
- Correction: qualify these sentences (H-029 and the H-071 path wait for decisions; H-016/H-030/H-039
  wait for KK-C13-23); fix 6.4 and 6.2.

### K13. Numeric and reference inconsistencies - should-fix
- auditoinnin-yhteenveto.md completion table, row 9, points to "luku 11", which does not exist
  (headings end at 10).
- omistajan-paatokset.md: "84 ratkaisemattomasta 16 on luokiteltu omistajan paatoksiksi" and maps 16
  Qs including Q25; auditoinnin-yhteenveto.md 6 has 15 in class 3 and Q25 in class 1 (resolved).
- regressiotestaussuunnitelma.md 5 item 7 and 4: "7 P2-havaintoa on vain GUI:ssa" omits GUI-only P2
  rows H-091 (`lisaToimetOn` set only in the GUI) and H-160 (SQL mirror, GUI only).
- korjaussuunnitelma.md 7.4 "noin 15 PR:aa": items 1-9 as written give about 19 (3+3+4+1+2+1+1+3+1).
- OP-11 "Viimeistaan: aalto 1 sarja b" vs the wave table, which has only "KK-C12-2 (sarja a)".
- auditoinnin-yhteenveto.md 1 "TPsource/ on ollut siita lahtien ennallaan": production code yes, but 46
  test files were added under `Tests/` and `VS/Tests/` (`git diff --name-only f7fe553 HEAD`). Say
  "production code".
- seuranta.md:38 still says C-13 "rajattu, ei aloitettu" (part I item).
- Verified consistent: 34 packages + ULKOPUOLELLA; each package's `havainnot` equals the set of
  riskiarvio rows with that `kk`; every `prioriteettijakauma` and package `prioriteetti` recomputed;
  the 3.2 table (waves, H/T counts, P) matches the CSV; first batch = 25 (4 P1, 17 P2, H-102, H-013,
  H-041, H-006); stage 1 = 39 findings in 17 packages; 19 of 39 P1/P2 reproduced (H-032 + 18 P2) and 7
  P3 reproduced; PASS 23 / XFAIL 24 matches `doc/arkkitehtuuri/todisteet/c11/vaihe4/core_all/results.json`
  (24 XFAIL ids = 15 findings) and `testaus.md:1481`; manifest 47 rows (24 defect, 19 control, 4 info);
  open run-time obligations sum to 80; no C-13 conclusion uses a value from `makroartefaktit.csv`
  (scripted name search over the four documents and both CSVs: only the caveat itself).

### K14. Compatibility claims checked against code - confirmed, two caveats - note
- H-025 VAHVISTETTU: relay `combufsize` = max class_len without the 14-byte header
  (`Juk/VCom.cpp:1236-1240`), HK adds 14 (`Hk/HkCom32.cpp:1095-1097`); FILESEND class_len =
  combufsize - 14 (`Juk/VCom.cpp:1240`); the queue header check compares `ob->len` with combufsize and
  offers delete or abort (`Tp/TpComY.cpp:2282-2293`). Correctly format-changing, relay only.
- H-041 VAHVISTETTU: KILPBUFSIZE (`Hk/HkDef.h:98-104`) is used only for five stack buffers (K10); not a
  record length, file offset or packet length. Caveat: osaB 4.2 O12 says "via heap, not a larger stack
  buffer" (40960 bytes is the most common `_beginthread` stack argument, 29 sites; 12 sites use 10240),
  while KK-C13-11 says thread stacks are "1 Mt tai enemman [paatelty]". With CreateThread the argument
  is a commit size, so the plan is probably right; resolve the contradiction in KK-C13-11.
- H-071 VAHVISTETTU, with a precision: `/K` is a sender-side flag (`lahkaikkitied`,
  `Tp/TpComY.cpp:2928`); the sender also produces parent-relative names (`:2912-2925`); the receiver
  opens whatever name arrives (`:3011-3013`); documented in `docs-site/docs/hkkisawin/hkliite1.md:81`.
  A receiver path restriction breaks both uses, as the plan says.
- H-171 VAHVISTETTU: relay `cfgfilename[41]` receives `wcsncpy(..., 120)` (`Juk/VInit.cpp:74`, `:652`)
  only from ViestiWin (`ViestiWin/UnitMain.cpp:220`; the console passes 0,
  `JukConsole/laskenta.cpp:108`); the name is only read and shown. Caveat: the name is formatted into
  `wchar_t line[80]` ("Konfiguraatiotiedostoa %s ei loydy", `Juk/VInit.cpp:433-446`,
  `Hk/HkInit.cpp:331-345`); a missing config file with a GUI path longer than about 50 characters and a
  CFG key overflows that stack buffer in both GUIs, before and after the H-171 fix. Unregistered
  candidate [paatelty, not run]; handle in the H-171 PR.
- H-129 VAHVISTETTU: in-memory index only (`Hk/HkIx.cpp:1600`), no file effect. The fix takes
  `idindex_CS` in `teejarj` in wave 1, before KK-C13-5 documents the lock order (note).
- HK/relay and MSVC/bcc32 scoping of H-025 (relay), H-171 (ViestiWin), H-041 and H-129 (HK) is right.

### K15. Technology and owner decisions - confirmed - note
- korjaussuunnitelma.md 6.5 lists options with "pysya" in every row and decides nothing; principle 7
  keeps C++03 until OP-3 (matches CLAUDE.md). Every "Suositus" in omistajan-paatokset.md is labelled
  [ehdotus] or "Ei suositusta".
- Minor: KK-C9-2's atomic write (temporary file + rename) is a design choice that changes how
  published files are replaced (the CSV says so) without an OP item; OP-6's "tauko" recommendation
  rests only on osaB O5. Mark both as proposals in the package text.

### K16. Upstream branches with production changes are not considered - should-fix
- `origin/88-...`, `origin/90-...`, `origin/92-...`, `origin/95-...` change production code after
  `f7fe553`: `cbTp/cbTpApu.cpp` (H-116/H-153 site), `cbHk/UnitExport.cpp` (H-041 buffer site),
  `cbHk/UnitEmit.cpp` and `ViestiWin/UnitEmit.cpp` (H-116 sites), `Hk/HkEmit.cpp`, `Juk/VEmit.cpp`,
  `include/TpLaitteet.h` (`git diff --stat f7fe553 origin/<branch>`). Neither the plan nor 7.2
  mentions them.
- Correction: 7.2 should add "re-base on current `main`, re-check the cited lines of every finding
  whose file changed", and list these branches as merge-order risks.

### K17. Smaller notes
- H-030 (P3, silent undo of newer data; automatic resend after TCP start, `Hk/HkCom32.cpp:985-987`,
  `Tp/TpComY.cpp:1111-1167`) waits for KK-C13-23 (XL, wave 5); acceptable only if OP-14 shows XML-TCP
  is rare - say so in the package.
- KK-C13-13 contains H-064 (card-reading NULL dereference in `Juk/VEmit.cpp`) and H-075
  (uninitialised pointer); not fixed-buffer defects. Harmless, but the title misleads.
- The stored C-11 run has `tests_dirty True` (results.md header); 7.2 item 4 must re-run the
  baseline from a commit.
- H-072 walk-through: the plan names `d1 = 5` (eralahto) and relay `d1 = 3`; HK EXTRA also dispatches
  cases 1, 2, 3, 6, 7, 9 with wire values (`Hk/HkCom32.cpp:626-660`). The PR must audit all cases and
  state whether a rejected packet is still acknowledged (H-026 resend risk) and forwarded.
- Roll-ups: plan 8 requires splitting before the package starts; H-127 N-22 (`tall_tulos(k_pv, 0)`
  with swapped arguments, `cbHk/UnitEmit.cpp:1588`, `:1597`) clears split k_pv instead of the finish on
  stages > 0 - a possible silent result error; split it out before wave 4, not during it.

## Answers to the nine questions
1. Membership is complete (script OK; 30 rows sampled across batches fit their packages except H-033,
   which belongs in KK-C13-1). Significant risks outside the plan: H-033 (K1) and the H-153 consequence
   of the H-116 extension (K2). Roll-ups are flagged in `huomio` and in plan 8 (K17 for N-22). J7-5 and
   the 56 core parse candidates are under-scheduled (K11); the 153 statics are reasonably in stage 3.
   The H-116 extension is verified in code; recording it as an extension at P2 is acceptable (K2).
2. P1 set right except H-033; only two overrides exist and both are documented; inconsistencies K9.
   P2 = 35 is defensible but needs an internal order.
3. Root causes are real (JS-2, JS-3, JS-4, JS-6 confirmed in code; JS-7 partly, K10). Possible missing
   cause: configuration agreement between stations.
4. Sizes are workable given the series inside KK-C13-3, KK-C13-15 and KK-C12-3; single-T and two-row
   packages are fine. The three rejected change requests are justified in substance, but the
   fix-together coordination they carried (FT-1, FT-4, FT-5) was partly lost (K3-K5). CSV and text
   agree except K12-K13.
5. No cycle in `riippuu`; prose-only dependencies break the wave order once (KK-C10-1 -> KK-C10-3) and
   blur H-087 (K4); H-013 is unsafe in the first series (K6); wave 1 needs environments that wave 0
   does not create (K7). No large refactoring delays a P1/P2 fix, except that H-029's fix is a design
   package (K12).
6. Every P1-P3 package has a row in regressiotestaussuunnitelma.md 4; KK-C13-16 has no pre-release
   test and KK-C13-19, -20, -23 are deferred, so criterion 6 is partly met. No planned test is shown as
   executed; PASS 23 / XFAIL 24 verified. The bcc32 GUI weakness is stated honestly (9), but the first
   batch's manual acceptance run does not require the GUI programs (K7).
7. Compatibility claims hold (K14), with the KK-C13-11 stack/heap contradiction and the `line[80]`
   candidate as caveats; H-013 is the one case described as format-neutral that changes behaviour (K6).
8. No technology lock-in (K15).
9. H-072, H-129 + H-128 and H-149 can be located and their mechanisms understood from plan + register.
   Missing: the full EXTRA case inventory and reject/ack/forward semantics (H-072); the bib-lowering
   half of H-128 and the single-stage acceptance contradiction; H-013's broadcast effect; a valid
   baseline for H-149's byte-identity criterion; tests for H-102.

## C-13 completion criteria 1-9
1 met. 2 met. 3 met. 4 met formally (content: K1, K2). 5 met (script). 6 partly (KK-C13-16 pre-release
missing; KK-C13-19/-20/-23 deferred to design). 7 met. 8 met except K8. 9 open: this review; the
summary's "luku 11" does not exist yet.

## Fitness for phase D
Fit after one correction round. The registers are internally consistent (all package, priority and
count cross-checks pass), the package model and wave order are sound, the compatibility analysis is
accurate and the test plan is honest about what was never run. Three weakest points: (1) the ranking
still inherits an unexamined batch severity, so a network-default heap write (H-033) sits at P4 outside
the P1 change and the first batch; (2) ordering and fix-together constraints live in prose, not in the
checked `riippuu`/`rinnakkain` columns, so the script's "no wave violation" is weaker than stated (K3-K5);
(3) the first batch assumes test environments (TCP stub, GUI scripts, loopback, bcc32 runs) that wave 0
does not create, and one first-batch fix (H-013) changes network behaviour without analysis.

## Unresolved questions
- Does VCL in this C++Builder version return mrCancel (2) for X on TFormSelect3 (GUI run, J7-4)?
- Run-time effect of an out-of-range KILPPVT `pv` index (an ASan harness test through `addseur`
  would settle K1).
- Which of the seven stage-flag loops are restarted after a stage change (code trace or GUI run).
- Whether 40960-byte `_beginthread` stacks grow to the H-041 buffer size under bcc32 (probe).
- Whether upstream branches 88/90/92/95 merge before phase D (owner).
- Whether `swprintf(line, ...)` into `line[80]` is bounded in either RTL (K14; probe).
