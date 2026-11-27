# C-13 part A: audit coverage and reliability

Scope: documents only; read-only apart from this file. Basis: osaA0/*.csv, rekisteri/*, the batch reports, seuranta.md, CHECKPOINT.md, suunnitelma.md. Revision analysed: f7fe553 (CHECKPOINT.md:11-12). Date of this check: 2026-11-26.

Marks: [verified] = I checked the statement in a file or script output in this session; [inferred] = my reading or judgement; [tuntematon] = not established.

## 0. Summary

- Starting figures: 177 H (H-099 refuted), 52 T, Q1..Q112, 2,395 validator references, 435 classified switches, 750 setting rows and the 18-row test matrix all match [verified]. The one real discrepancy is analysis level 3: tiedostot.csv has 22 level-3 files, CHECKPOINT.md:50 says 12 (stale C-4 figure). Levels 1 (474), 2 (135) and test level 4 (19; 0 production) agree everywhere. CHECKPOINT.md:53 (2,249 references) is also stale.
- 21 stale or inconsistent statements found (section 2); the most material: H-017 review state, review table limited to H-001..H-108, no C-7/C-8/C-9 rows in the batch table, rekisteri/README.md "no H finding confirmed by running", moduulit.md frozen at C-7, 34 high findings vs 22/26, seuranta.md rows for C-1..C-3 and C-11 not marked closed.
- Coverage: 631 production files / 270,050 lines: L1 474 files (97,956 lines incl. 7,106 of doctest.h), L2 135, L3 22 (25,871 lines), L4 none. Whole GUI form layer is mostly unread (284 level-1 form files, 73,248 lines). No directory is level 1 only.
- Verification strength of 177 H: 1 refuted, 1 confirmed in a shipped program, 26 reproduced in probe or harness, 89 fully reviewed, 43 partly reviewed, 3 reviewer finds, 13 never reviewed, 1 undecidable. Source reference validation is existence-only and says nothing about truth.
- 84 open Q: proposed classes (1) resolved 5, (2) run-time 39, (3) owner 15, (4) accepted open 25.

## 1. Reconciliation of the starting figures

Method: read osaA0/havainnot.csv, tekninen_velka.csv, kysymykset.csv, yhteenveto.txt, tarkistus_a0.out; recomputed levels from rekisteri/tiedostot.csv with a read-only script; ran `python -I doc/arkkitehtuuri/tyokalut/tarkista_auditointi.py .` (output: `findings: 229 (177 H, 52 T)`, `register rows: 650, levels: {'1': 474, '2': 135, '3': 22, '4': 19}`, `source references checked: 2395`, `agents: 6`, `OK`) [verified].

| # | Figure | Stated | Found | Verdict | Where a difference comes from |
|---|---|---|---|---|---|
| 1 | H findings | 177 (H-099 refuted) | 177 rows H-001..H-177: 176 open + 1 refuted (H-099) | matches [verified] | osaA0/havainnot.csv, yhteenveto.txt; validator line `findings: 229 (177 H, 52 T)` |
| 2 | T items | 52 | 52 rows T-001..T-052 | matches [verified] | Per-batch split in the A0 base puts T-001..T-010 under C-1 by ID range; their register sources are round-1 documents (inventaario.md, merkistot.md, riippuvuudet.md; rekisteri/havainnot.md:339-348), so round 1 has 10 T and C-1 has none [inferred] |
| 3 | Q ids | up to Q112 | Q1..Q112, 112 rows, none missing; 28 resolved, 72 open, 6 partly, 3 owner decision, 3 run-time | matches [verified] | yhteenveto.txt |
| 4 | Validated source references | 2,395 | Validator prints 2395 now | matches [verified] | It is a count of regex matches of backticked `path` or `path:line` strings that contain a slash, in doc/arkkitehtuuri/*.md and rekisteri/*.md (tarkista_auditointi.py:76-100): 1,969 with a line number, 426 file-only, 1,663 distinct strings (my recount with the same regex). The check is only that the file exists and the line number is not beyond the end of the file (tarkista_auditointi.py:9-10, 97-99). It does not check content. CHECKPOINT.md:53 says 2,249 (stale) |
| 5 | Classified switches | 435 | 435 rows in rekisteri/valitsimet.csv, all `laji` esikaantaja, all with a class (151 include-guard, 107 dead-never-defined, 60 feature, 39 compiler-platform, 28 size-constant, 15 tu-local, 12 other-constant/default-constant/function-macro, 5 variant, 5 debug-diagnostic, 5 feature-forced-on, 6 opt-out-never-defined, 2 variant-forced-off) | matches [verified] | These are macros tested in conditions (2,491 sites). 1,681 further #define names that no condition tests are listed but not explained (ominaisuusvalitsimet.md 13.1). Recorded macro values are lexical; 238 expression-defined macros appear as 1 (osaA0/makroartefaktit.csv) |
| 6 | Setting-key rows | 750 | 750 rows in rekisteri/asetusavaimet.csv: Hk/HkInit.cpp 291, Juk/VInit.cpp 206, cbHk/UnitCfg.cpp 135, ViestiWin/UnitCfg.cpp 118 | matches [verified] | Rows, not unique keys (the same key appears in several parsers; for example EMITAG at rows 245, 460, 617, 736). The plan estimated about 245, 169, 230 and 96 (suunnitelma.md Part B), so the editors are smaller and the parsers larger than planned |
| 7 | Test matrix | 18 rows | T1..T18 in ominaisuusvalitsimet.md:440-460 | matches [verified] | Not run (ominaisuusvalitsimet.md:433-434, 13.1) |
| 8 | Files and lines | 631 files, 270,050 lines (+19 test files) | 650 rows, 273,179 lines; 631 production rows with 270,050 lines; 19 L4 test files with 3,129 lines | matches [verified] | tiedostot.csv. The 631 include 5 older Tests/ files (doctest.h, DoctestMain.cpp, HkEnnTavTest.cpp, VRinnakkaisOsuusTest.cpp, WideString.h) |
| 9 | Level 1 / 2 / 3 / 4 | CHECKPOINT: 474 / 135 / 12 / 0 + 19 test | Production (631): L1 474, L2 135, L3 22, L4 0. Test files: 19 at L4. Validator `{1:474, 2:135, 3:22, 4:19}` counts all 650 rows | L1, L2, L4 match; **L3 differs: 22, not 12** [verified] | Only CHECKPOINT.md:50 is stale (it also makes the sum 474+135+12 = 621, not 631). The 12 is the C-4 state (rekisteri/moduulit.md:28-36); C-7 raised it to 20 (moduulit.md:40-45); Hk/HkIx.cpp and tputilv2/dbindex.cpp became L3 later (their notes say "C-8: ... luettu") [inferred for the batch]. Validator and CHECKPOINT do not conflict on L1, L2, L4 |

## 2. Stale and mutually inconsistent statements

All rows [verified] by reading the cited lines unless marked [inferred]. "Stale" = was true at some earlier batch.

| # | Where | Statement | What the register or another document shows | Kind |
|---|---|---|---|---|
| S1 | CHECKPOINT.md:50 | Level 3 = 12 | tiedostot.csv: 22 (list in section 4) | stale (C-4 figure) |
| S2 | CHECKPOINT.md:53 | 2,249 source references | Validator: 2,395 | stale (C-10 closing) |
| S3 | CHECKPOINT.md:109 | H-017 has no independent review | rekisteri/havainnot.md:26: "independent source review (C-7)"; csv `riippumaton`. Of H-001..H-108 this is the only difference between the CHECKPOINT table and the csv (73 + 26 + 8 + H-020 = 108 listed; csv gives 74 full incl. H-099, 26 partly, 7 none, 1 undecidable) | stale |
| S4 | CHECKPOINT.md:105-114 (Havainnot table) | Review states cover H-001..H-108 only | H-109..H-177 (69 rows) are absent; their states are 8 not reviewed, 3 reviewer finds, the rest reviewed (csv) | incomplete |
| S5 | CHECKPOINT.md:111-112 | Reproduced in harness lists only C-11 (and C-6 probes in the next row) | C-8 (H-128, H-129) and C-9 (H-145, H-146, H-147, H-149, H-150, H-151) probe results exist (csv `koe`: 26 harness/probe + H-041) | incomplete |
| S6 | CHECKPOINT.md:116 | High findings 22 (26 with C-6) | 34 high in the normalised base (validator "high severity = documented 34"). Additions after C-6: H-109, H-110, H-111, H-129, H-149, H-159, H-160, H-171 | stale |
| S7 | CHECKPOINT.md:16-45 | "Valmiit erat" table | Has no rows for C-7, C-8, C-9 (they appear only in the later prose, lines 211-293) | incomplete |
| S8 | CHECKPOINT.md:13, 150, 171-175, 299-301 | Commit list ends at c6205d2; open-question intro says Q2-Q77; first "Seuraava askel" and the English tail say C-11 is last and next is C-4 | Git log head 3f378e2; Q up to Q112; line 295 gives C-13 as next | stale, and internally inconsistent |
| S9 | CHECKPOINT.md:136-148 | Refuted claims listed for C-2, C-3, C-4 only; C-3 has 4 items | tiedostomuodot.md:410-415 lists 5 (adds the OEM/ANSI file-name issue); C-5..C-12 refutations are in the reports only | incomplete |
| S10 | CHECKPOINT.md:49 | L2 includes "C-10: dependency sites" | tiedostot.csv has no row whose notes mention C-10 or C-12 (0 of 650); C-10 and C-12 did not update file-level levels. cbHk/UnitCfg.cpp and ViestiWin/UnitCfg.cpp are still L1 although C-12 parsed their keys by script (135 + 118 rows) | inconsistent [inferred: C-10 may have counted files already L2] |
| S11 | rekisteri/moduulit.md:28-45, table | Summary stops at C-7 (477/134/20); table T2/T3 differ from csv for Hk (19/2 vs 18/3), Tp (4/1 vs 5/1), com (1/1 vs 2/1), tputilv2 (24/2 vs 23/3), include (4/3 vs 5/3), Tests (0/0 vs 4/0); Todennus column has nothing after C-7 | csv totals 474/135/22 | stale |
| S12 | rekisteri/README.md:59 | "At present no H finding is [confirmed by running]" | H-041 confirmed in the shipped HkMaali.exe (testaus.md 19.11; havainnot.md:50) | stale |
| S13 | rekisteri/README.md:48 | Severity scale kriittinen / korkea / keskitaso / matala | Register uses also matala-keskitaso and keskitaso-korkea (csv) and 23 conditional severities; no critical exists | incomplete |
| S14 | rekisteri/README.md Level 4 definition vs moduulit.md:36-38 | Level 4 = whole file read and checked for defects | The 19 L4 files are C-11 test files written by the audit, not reviewed for defects; the register's own wording says no production file is at L4 | label mismatch [inferred] |
| S15 | README.md reading-order table | riippuvuudet.md row has no C-10 (sections 7-19); testaus.md row says plan only (stages 1-4 done); virheenkasittely.md row H-145..H-157, T-040..T-041 | Final: H-145..H-158, T-040..T-042 after step 2 (csv: C-9 14 H, 3 T) | stale |
| S16 | suunnitelma.md:154-156 vs :160 | C-12 and C-13 "have not been started" | Same table: C-12 "done and closed"; C-13 has A0 material (todisteet/c13/osaA0) | stale |
| S17 | seuranta.md status table | Rows for C-1, C-2, C-3, C-11 say "valmis, tarkastettu"; closure wording ("suljettu") is only in the decisions log for C-4..C-10, C-12 (seuranta.md:352-366) | Closing condition 2 of C-13 requires every batch to read "suljettu" (suunnitelma.md C-13 conditions) | to fix before closing phase C |
| S18 | seuranta.md Q3 row | Struck-through but "partly resolved; exact sizes open (Q18)" | Q18 resolved in C-3, so the open part is closed; csv status `osittain` | stale |
| S19 | tallenna-samanaikaisuus.md H-021 row vs tiedonsiirto.md:272 | C-1 table: H-021 confirmed; C-2 table: partly | Register: partly. A0 base gives era C-1 (ID range) but the register source column says tiedonsiirto.md (osaA0/yhteenveto.txt "batch range vs source column") | inconsistent |
| S20 | tiedonsiirto.md H-038/H-039 rows | H-038 "not reviewed" | Register: partly reviewed in C-6 (T5, saikeet-ja-lukot.md:342) | report not updated (consistent with register) |
| S21 | A0 base `tarkastus` | Class "reviewer's own find" has only H-132, H-158, H-162 | tallenna-samanaikaisuus.md:193 also labels H-019 a reviewer's find; the register calls it "not reviewed" (H-017 was the other, reviewed in C-7) | classification choice, see section 5 |

## 3. Per batch: plan, criteria, coverage, method, results

Era of an H/T/Q is taken from the ID range in the A0 base (batch ranges in rekisteri/havainnot.md); where the register source differs (H-021, T-001..T-010) this is noted. Two tables follow, one row per batch, split only for width. Plan text from suunnitelma.md, criteria and outcomes from each report's coverage, closing and review sections. Counts in table B come from osaA0/*.csv [verified]; descriptions are my reading of the cited lines [verified] unless marked [inferred].

### 3.1 Table A: plan, criteria, deviations, gaps, follow-ups

| Batch | Goal and scope as planned | Completion criteria and status | Deviations from plan | Left out / not covered | Open follow-ups |
|---|---|---|---|---|---|
| Round 1 (K1) | Scripted inventory, overview, dependencies, structures, encodings, standard, plan; infrastructure (register, 6 project agents, validator). CHECKPOINT.md:19-21 | No formal criteria. Done: documents and register exist [verified: README.md reading-order table] | None recorded | Only inventory level for the five programs; no deep reading | Q6, Q9-Q12 (inventaario.md); CLAUDE.md fixes (T-010) |
| C-1 | Plan row 1: tallenna() concurrency; agents architecture-analyst, defect-concurrency-auditor x2, independent-verifier; output tallenna-samanaikaisuus.md, H-007..H-021 (suunnitelma.md:113-115). General rule: batch ends with independent review and register update (suunnitelma.md:111-114) | Met: report states every key claim reviewed independently (tallenna-samanaikaisuus.md:8-11); H-007..H-021 delivered [verified] | None recorded | Relay receive path and its locking; threads kirjSQLTh, SendCopyThread, vahaku, gazTh not fully traced (tallenna-samanaikaisuus.md:263-270) | Q12, Q14, Q15; 4 run-time checks proposed (tallenna-samanaikaisuus.md:241-261) |
| C-2 | Plan row 2: workstation sync protocol (receive, send, ack, resend, version compatibility); output tiedonsiirto.md, Q3 | Met: H-022..H-039, T-011..T-014; Q3 partly (exact sizes left to Q18, later resolved in C-3) [verified] | None recorded | Real multi-station behaviour (nothing run); 8 test scenarios written, not run (tiedonsiirto.md:298-327) | Q19-Q25 (7 open) |
| C-3 | Plan row 3: persistent files and integrity (KILP.DAT, emit, ajat, SQLjonoVarm.dat, old format); Q4, Q17, Q18; Q5 moved to batch 10 | Met: H-040..H-054, T-015..T-019; Q4, Q17, Q18 resolved [verified: seuranta.md Q4 and Q17-Q18 rows] | Q5 moved to C-10 as written in the plan table; compile-time probes added to the static method | Running programs, real data, power-cut behaviour (Q27) | Q26-Q34 (Q30 resolved in C-6) |
| C-4 | Plan row 4: warning walk-through on both compilers, MSVC /analyze, buffers and format strings. Step 2 scope fixed by the user: analysis only (muistiturvallisuus.md:341-346) | Met for the scope as limited: inventory, triage, H-055/H-056 deep analysis, 33 investigate rows, 522 conversion warnings, coverage, overall assessment (muistiturvallisuus.md:341-353, 677-700) [verified] | Step 2: no ASan series, no new harness, no GUI runs, no fixes (muistiturvallisuus.md:343-346) | About 93% of warnings (6,569 of 7,030) in untriaged groups; GUI form memory safety (bcc32 has no format or bounds analysis); Debug/Demo/Debug250/Win64 configurations; bcc32c/bcc64; 55 uncompiled files (muistiturvallisuus.md:96-118, 305-318, 478-499) | Q43-Q56 (14 open); device parsers and packet classes not read in full (muistiturvallisuus.md:665-675) |
| C-5 | Plan row 5: pragma -K -a1 (43 files), struct layout across units, /J, wchar_t, charsets; Q7 | Met in step 1; step 2 added Q8 (bcc32 object comparison), probes Q57-Q59, Q61, Q62; closed 2026-11-25 (kaantajaerot.md:341-363, 440-486) [verified] | Step 2 added after step 1; agent use about 1.56 M tokens led to the later working method (suunnitelma.md:692-704) | 161 unread integer-parse candidates (105 form units, 56 core); H-079 reachability; run-time confirmation of shipped programs (FPU word, H-078, H-079); code page measured only on 1252 (kaantajaerot.md:330-339, 460-486) | Q60 owner; Q63, Q64; 161 candidates; seuranta.md follow-ups |
| C-6 | Plan row 6, step 1 scope: thread map of all start sites, lock map of all (planned 24) locks, lock-order pairs, shutdown paths; Opus review of new high/medium claims; budget about 380 k tokens; step 2 on a separate decision (suunnitelma.md:649-690) | Met: 67 start lines (56 active), 22 locks, edge list (lower bound), 20 claims reviewed, validator passes (saikeet-ja-lukot.md:385-399); step 2: 5 findings, probes, VCL, 11 candidates (13.10) [verified] | No token budget by user instruction (about 950 k used); Opus review; starting figures corrected (56 vs 76 start sites, 22 vs 24 locks) (suunnitelma.md:651-654) | bcc32/GUI run (H-091 cycle), H-087 and H-103 in ViestiWin, Q67 helper threads, Q68, Q73, H-090 on real devices, H-106 cross-thread SetText, low candidates of section 9 (saikeet-ja-lukot.md:662-690) | seuranta.md "Avoimet jatkotehtavat C-6:sta" (run-time list) |
| C-7 | Plan row 7: GUI business logic: cbHk UnitEmit, UnitAjanotto, UnitArvonta, UnitKilpailijatiedot, ViestiWin UnitEmit; step 1 extended to relay counterparts | Mandatory goal met; 13.1: named units met, relay counterparts met, 9 parse candidates met, 16 claims reviewed met (5 low rows unreviewed), question 4 (draw correctness) partly (gui-liiketoimintalogiikka.md:406-424) [verified] | First B pass only sampled the competitor and team forms; supplementary B pass added (gui-liiketoimintalogiikka.md:78-85) | Draw algorithm cbHk/HkArv2.cpp (1,274 lines, level 1); 41 other form files that write competition data; 152 parse candidates; OsuusTiedot.cpp and UnitEmitMuutokset.cpp without own defect review | J7-1..J7-7 (seuranta.md): none started |
| C-8 | Plan row 8: global mutable state: who writes what, initialisation, reset at stage change | All 4 completion criteria met (globaali-tila.md:398-414), 6 closure rows met with 4 deviations (470-500) [verified] | 153 function-local statics outside Hk/ and Juk/ listed but not classified; six C-6 threads not covered by part D; part A writer lists are lower bounds; H-142..H-144 unreviewed (globaali-tila.md:476-495) | lue_parametrit beyond its start (2,200 lines), relay luesarjat, GUI Initialisoi, relay local statics, addem/lueemitfile internals (globaali-tila.md:414-421) | Q78-Q82; H-129 overflow itself and shipped confirmation; H-128 shipped confirmation |
| C-9 | Plan row 9: error handling and resources: return-value checks, file handles, leaks, writeerror paths; step 2 proposed at the end of step 1 | Met (virheenkasittely.md:493-503) incl. step 2 [verified] | Step 2 (isolated probes) added after step 1 on a separate decision | C-4, C-6, C-2, C-7, C-10 topics by design; H-152 reproduction; GUI/bcc32 run-time; low roll-ups H-156/H-157 one by one; GUI menu form handlers (new TForm); console prompts of 3 threads (virheenkasittely.md:248-257, 503-512) | Q87, Q89-Q95; seuranta.md C-9 list incl. H-153 deadlock in a GUI run |
| C-10 | C-10 step 1 scope (suunnitelma.md:409-475): per-product external dependencies, DB (Q5), HTTP and browser, devices, runtime packages and distribution, versions and modernisation risks | Criteria met (riippuvuudet.md:500-515 and 19.1); closed without step 2 on user approval [verified] | No step 2; closing section 19 re-ordered priorities and added KK-C10-1..4 | Real device and server behaviour; vulnerability databases; clean-machine start; PE read from 2026-11-24 builds; MAKI/HIIHTO/EMITFILE unknown; 31 files not compiled (riippuvuudet.md:518-532) | Q96-Q106; run tests for H-159, H-160, H-165 (Q96-Q98, Q102) only with permission |
| C-11 | Plan row 11 as written: tests into CI, run.sh, GUI verification for comment-only changes; later planned as test architecture plus harness (testaus.md) | Not as originally written: CI integration NOT done (Q35, Q36, Q40, push ban); run.sh still broken (H-006); GUI comment verification not in C-11 (done in C-5 step 2, Q8). Done instead: harness, 15 findings reproduced, H-041 run in the shipped program (testaus.md:1803-1866) [verified] | The pre-fixed verdict for the shipped run was not confirmed (criterion 7); H-041 recorded as confirmed by running through a user-approved deviation (testaus.md:1786-1790, 1886-1903) | Manual tests H-009, H-014; two-process loopback layer; H-041 read path (Q37); CI; WER positive control (testaus.md:1830-1845) | Q35-Q42; firewall rule removal prepared for the user (testaus.md:1866-1880) |
| C-12 | C-12 step 1 scope (suunnitelma.md:163-272): A0/B0 scripts, parts A, B, C, D, E; 7 completion criteria | All 7 criteria met (ominaisuusvalitsimet.md:80-111) [verified]; closed without step 2 | B0 added to the scope as input to part B (suunnitelma.md:165-167) | About 80 minor keys: see code; global initial values not walked; per-window GUI choices only for the Emit form; 1,681 #define constants listed but not explained; test matrix not run (ominaisuusvalitsimet.md:496-518, 520-560) | Q107, Q108, Q110 run-time; Q109, Q112 open; Q111 owner |

### 3.2 Table B: examined, method, results, review

| Batch | Examined (programs, configurations, areas) | Method | New findings (counts from osaA0 base) | Independent review outcome |
|---|---|---|---|---|
| Round 1 (K1) | All five programs and four libraries by inventory only | Scripted inventory (inventaario.py), compiler warnings, TpTest run 46/46, MSVC encoding probe (CHECKPOINT.md:80-84) | H 6 (2 full review, 2 harness/probe, 2 no review; high 2); T 0; Q 11 (6 ratkaistu, 1 osittain, 4 avoin); T-001..T-010 are listed under C-1 by A0 but come from round-1 documents [inferred] | None at the time; H-004, H-005 reviewed later (C-5) [verified: csv tarkastus] |
| C-1 | HkKisaWin, HkMaali (HK core Hk/, cbHk/, cbTp/, tputilv2/dbindex); relay only at the end (tallenna-samanaikaisuus.md:4-5) | Static reading; no run | H 15 (9 full review, 2 harness/probe, 2 partly, 1 no review, 1 undecidable; high 4); T 10 (by ID range; sources are round-1 documents [inferred]); Q 4 (1 avoin, 1 ratkaistu, 2 osittain) | Report table: 11 confirmed, 1 partly (H-010), 2 not reviewed (H-017, H-019), 1 undecidable (H-020) (tallenna-samanaikaisuus.md:182-194); 1 sub-claim refuted |
| C-2 | All five programs, HK and relay, console and GUI transport (com/, Tp/, Hk/HkCom32, Juk/VCom) | Static reading; no run | H 18 (5 full review, 8 harness/probe, 4 partly, 1 no review; high 8); T 4; Q 8 (1 ratkaistu, 7 avoin) | Of H-022..H-039: 12 confirmed, 4 partly (H-030, H-032, H-034, H-035), 2 not reviewed (H-038 later partly in C-6, H-039 never) (tiedonsiirto.md:268-290); 4 claims refuted |
| C-3 | KILP.DAT (dbboxm, HK and relay record, old format), COMFILE.DAT, emit, AJAT, SQLjonoVarm.dat, config; sizes in both compilers | Static reading plus compile-time sizeof/offsetof probes, MSVC 19.52 and bcc32 7.80, scratch directory (tiedostomuodot.md:3-8) | H 15 (7 full review, 2 harness/probe, 5 partly, 1 shipped run; high 3); T 5; Q 11 (3 ratkaistu, 8 avoin) | Of 15: 10 confirmed, 5 partly (H-044, H-047, H-049, H-051, H-052) (tiedostomuodot.md:366-420); 5 claims refuted |
| C-4 | MSVC /W3, /W3+LTCG, /W4 /analyze on 5 projects (7 configurations); bcc32 7.80 on 4 GUI projects; 210 MSVC and 364 bcc32 translation units (muistiturvallisuus.md:89-121) | Compiler builds and warning inventory (11,027 rows, 7,036 unique), triage of 275 rows, targeted reading; nothing run | H 23 (17 full review, 6 partly; high 5); T 3; Q 14 (14 avoin) | 5 independent reviews: K1-K9 (5 confirmed, 4 partly), step 2 reviews of new highs (muistiturvallisuus.md:353-359, 677-690); 1 claim refuted by review (UnitGAZ.cpp:997) |
| C-5 | MSVC layout report of 251 units; bcc32 pragma state of 405 units; encoding, types, formatting, RTL; 10 bcc32 object pairs; GUI-only types (kaantajaerot.md:61-72, 363-440) | Compiler runs (layout, preprocessor, -S), probe programs on both compilers, tdump of shipped GUI objects, string search in TPexe binaries; shipped programs not run | H 9 (7 full review, 2 partly; high 0); T 7; Q 8 (5 ratkaistu, 1 omistajan_paatos, 2 avoin) | Claims C1-C10 (kaantajaerot.md:68) plus 3 refinements in step 2 (455-458); 10 rows refuted or weakened in section 9 |
| C-6 | Core and GUI sources: 67 thread start lines, 22 locks; step 2: isolated MSVC console probes of production code for H-090, H-091, H-093, H-094 and a Win32 probe for Q65 (saikeet-ja-lukot.md:426-520) | grep scripts and reading; step 2 probes built outside the repository; no bcc32 GUI build, no shipped program | H 22 (10 full review, 4 harness/probe, 6 partly, 1 no review, 1 refuted; high 4); T 4; Q 10 (7 ratkaistu, 1 osittain, 2 avoin) | Step 1: 20 claims by two Opus reviewers (12 confirmed incl. L8 later refuted, 8 partly) (saikeet-ja-lukot.md:324-352); step 2: 11 static claims and all probes (13.10); H-099 refuted |
| C-7 | 8 GUI form units, about 16,000 lines (gui-liiketoimintalogiikka.md:19-31) | Static reading: 3 Sonnet parts, 2 Opus review rounds; no run | H 19 (8 full review, 6 partly, 5 no review; high 3); T 4; Q 3 (3 avoin) | 16 claims: 10 confirmed, 6 partly, 0 refuted; 9 suspicions refuted (gui-liiketoimintalogiikka.md:54-56, 334-341) |
| C-8 | Core directories Hk/, Juk/, Tp/, com/, cbTp/, tputilv2/: 1,200 global or file-static names, 370 function statics, about 45,500 references (globaali-tila.md:20-27) | Scripted inventory (globaalit.py), Sonnet classification, Opus review; isolated MSVC probe (C8ProbeHk) for H-128, H-129 mechanism; shipped programs not run | H 17 (6 full review, 2 harness/probe, 5 partly, 3 no review, 1 reviewer find; high 1); T 2; Q 5 (5 avoin) | 16 claims: 8 confirmed, 3 confirmed with changes, 5 partly, 0 refuted; new lead H-132 verified by the main session (globaali-tila.md:59-62) |
| C-9 | 11 directories, 417 .c/.cpp files (242 with no rows); write-error path of every persistent file; resource lifecycle; exit paths; step 2: MSVC Release+Asan probes (C9ProbeHk, C9ProbeJuk) and a bcc32 console RTL probe | Scripted inventory (resurssit.py), reading, isolated probes; no shipped program, no GUI run | H 14 (4 full review, 6 harness/probe, 3 partly, 1 reviewer find; high 1); T 3; Q 13 (5 ratkaistu, 2 osittain, 6 avoin) | Step 1: 19 claims in 21 rows: 16 confirmed, 5 partly, 1 sub-claim refuted, 2 duplicates; step 2: 9 items + N1 by Opus (virheenkasittely.md:187-192, 421-427, 505) |
| C-10 | All 5 products and 4 library projects, static; 27/27 device ids; DB, HTTP, browser code read in full; PE import tables of local builds of 2026-11-24 (riippuvuudet.md:194-232) | Scripted marker inventory (3,779 markers, 161 forms), reading, RTL and dbExpress sources of installed C++Builder 13; nothing built or run | H 12 (10 full review, 1 partly, 1 reviewer find; high 2); T 7; Q 11 (10 avoin, 1 omistajan_paatos) | 15 claims (K1-K15): 11 confirmed, 4 partly; N-9 refuted for HK; reviewer found E1-E3 (riippuvuudet.md:465-478) |
| C-11 | TpCoreHk, TpCoreJuk, TpTest (MSVC Release and Asan); shipped HkMaali.exe (SHA-256 61034e5d..) once under a debugger; black-box runs 0-1 under firewall isolation (testaus.md:1803-1830) | Harness runs (run_core_tests.py: 23 PASS, 24 XFAIL), one shipped-program run (stage 4), disassembly checks | H 0; T 0; Q 8 (8 avoin); no new H or T in C-11 [verified: no C-11 era rows] | Test-plan review V1/V2: 13 rows (4 confirmed, 6 partly, 2 refuted, 1 mixed) (testaus.md:499-522); stage reviews (testaus.md:691-695, 804, 900, 1259, 1635, 1747, 1886) |
| C-12 | 435 conditionally tested macros at 2,491 condition sites; 4 parsers (Hk/HkInit.cpp, Juk/VInit.cpp, cbHk/UnitCfg.cpp, ViestiWin/UnitCfg.cpp); other switches; 20-feature matrix and 18-row test matrix (ominaisuusvalitsimet.md:13-30, 404-464) | Lexical evaluator scripts (valitsimet.py, asetusavaimet.py), Sonnet analysis, Opus review; main session verified H-171 against an object file; nothing built or run | H 7 (4 full review, 3 partly; high 1); T 3; Q 6 (3 ajonaikainen_todennus, 2 avoin, 1 omistajan_paatos) | Part D: 19 claims: 12 confirmed, 7 partly, 0 refuted; 3 corrections, 2 claims made more severe (ominaisuusvalitsimet.md:70-85) |

Token use recorded in the documents [verified]: C-5 about 1.56 M, C-6 about 950 k, C-8 about 1.33 M, C-9 about 1.22 M, C-10 about 1.20 M, C-12 about 0.73 M. C-1..C-4, C-7 and C-11 do not state a total in the sections I read [tuntematon].

Open follow-ups that need explicit mention (counts from the reports):

- C-7: J7-1 draw algorithm (cbHk/HkArv2.cpp, 1,274 lines, level 1); J7-2 41 further form files that write competition data; J7-3 152 parse candidates (forms 96: cbHk 55, ViestiWin 30, cbTp 11; core 56: Hk 32, Juk 23, Tp 1); J7-4 run-time confirmation of H-109, H-110, H-116 (and H-111); J7-5 independent review of H-122, H-123, H-124, H-126, H-127; J7-6 Q15 remainder, Q75-Q77; J7-7 fix planning (T-034). None started (gui-liiketoimintalogiikka.md:426-449; seuranta.md J7 table).
- C-5: 161 parse candidates in total (105 form units, 56 core); 9 resolved in C-7 (7 safe, 1 = H-085, 1 partly unsafe = H-126), leaving 152 (seuranta.md "Avoimet jatkotehtavat C-5:sta").
- C-8: 153 function-local statics outside Hk/ and Juk/ unclassified (staattiset_funktiossa.csv); six C-6 threads outside part D (globaali-tila.md:476-492).
- C-6, C-9, C-10, C-12 run-time items: see seuranta.md follow-up lists; in the Q table of section 7 they are class 2.

## 4. Coverage of the code base (from rekisteri/tiedostot.csv)

Levels: 1 inventory by script only, code not read; 2 targeted (named functions or lines read); 3 structure read (all functions identified, key ones read); 4 whole file read and checked for defects (rekisteri/README.md:26-34). The register covers 650 .cpp/.h/.c/.H files; "All" below is that set. Table [verified], computed from tiedostot.csv. "Files with H/T ids" counts files whose `havainnot` column names at least one H or T.

| Directory | Files | Lines | L1 files / lines | L2 files / lines | L3 files / lines | L4 files / lines | Files with H/T ids | L1 share of lines |
|---|---|---|---|---|---|---|---|---|
| Hk | 27 | 46032 | 6 / 728 | 18 / 41418 | 3 / 3886 | 0 / 0 | 20 | 2% |
| Juk | 23 | 38605 | 3 / 331 | 20 / 38274 | 0 / 0 | 0 / 0 | 19 | 1% |
| Tp | 13 | 11704 | 7 / 498 | 5 / 8021 | 1 / 3185 | 0 / 0 | 5 | 4% |
| com | 3 | 1807 | 0 / 0 | 2 / 409 | 1 / 1398 | 0 / 0 | 3 | 0% |
| dbboxm | 13 | 2375 | 6 / 1333 | 5 / 603 | 2 / 439 | 0 / 0 | 2 | 56% |
| include | 21 | 1883 | 13 / 823 | 5 / 775 | 3 / 285 | 0 / 0 | 3 | 44% |
| tputilv2 | 169 | 15513 | 143 / 10549 | 23 / 4546 | 3 / 418 | 0 / 0 | 11 | 68% |
| HkConsole | 8 | 9605 | 4 / 1989 | 4 / 7616 | 0 / 0 | 0 / 0 | 3 | 21% |
| JukConsole | 10 | 8271 | 5 / 1121 | 5 / 7150 | 0 / 0 | 0 / 0 | 2 | 14% |
| cbHk | 149 | 67530 | 127 / 38996 | 18 / 19605 | 4 / 8929 | 0 / 0 | 22 | 58% |
| ViestiWin | 114 | 43424 | 94 / 23082 | 15 / 13011 | 5 / 7331 | 0 / 0 | 17 | 53% |
| cbTp | 73 | 15115 | 63 / 11170 | 10 / 3945 | 0 / 0 | 0 / 0 | 9 | 74% |
| RADStudio10 | 2 | 436 | 1 / 185 | 1 / 251 | 0 / 0 | 0 / 0 | 0 | 42% |
| VS | 1 | 45 | 1 / 45 | 0 / 0 | 0 / 0 | 0 / 0 | 0 | 100% |
| Tests | 5 | 7705 | 1 / 7106 | 4 / 599 | 0 / 0 | 0 / 0 | 0 | 92% |
| Tests (C-11 test files) | 19 | 3129 | 0 / 0 | 0 / 0 | 0 / 0 | 19 / 3129 | 0 | 0% |
| **All** | 650 | 273179 | 474 / 97956 | 135 / 146223 | 22 / 25871 | 19 / 3129 | 116 | 36% |

Reading the table [verified unless marked]:

- No directory is level 1 only. The nearest are VS (1 file, 45 lines), cbTp (63 of 73 files, 74% of its lines), tputilv2 (143 of 169 files, 68%), cbHk (127 of 149 files, 58%) and ViestiWin (94 of 114 files, 53%).
- Level 1 only (inventory, code not read): 474 files, 97,956 lines (36% of all lines). Without the third-party Tests/doctest.h (7,106 lines): 90,850 lines. Of these, the three GUI form directories cbHk, ViestiWin and cbTp hold 284 files and 73,248 lines (81% of the 90,850); tputilv2 holds 143 files and 10,549 lines.
- Core directories Hk, Juk, Tp, com are almost entirely level 2 or 3 (level 1 only 16 small files, 1,557 lines), but level 2 is "targeted": for example Juk/VTulostus.cpp (9,151 lines), Hk/HkTulostus.cpp (8,840), Hk/HkEmit.cpp (6,432) and Juk/VEmit.cpp (6,221) were read at the sites of findings, not through.
- Level 3 covers 25,871 lines (9.5%) in 22 files; level 4 has no production file. The 19 level-4 files are C-11 harness sources written by the audit (3,129 lines), not defect-reviewed code.
- 116 files carry H or T ids; all are at level 2 or 3. Files with findings per directory: cbHk 22, Hk 20, Juk 19, ViestiWin 17, tputilv2 11, cbTp 9, Tp 5, com 3, HkConsole 3, include 3, dbboxm 2, JukConsole 2.
- Register gaps: no row has a note mentioning C-10 or C-12; cbHk/UnitCfg.cpp (2,936 lines) and ViestiWin/UnitCfg.cpp (2,352) are level 1 although C-12 parsed their keys by script. Form files (.dfm) and project files have no level [verified: S10, L9].

### 4.1 Level 3 files (22)

| File | Lines |
|---|---|
| Hk/HkCom32.cpp | 1332 |
| Hk/HkInitVanha.cpp | 549 |
| Hk/HkIx.cpp | 2005 |
| Tp/TpComY.cpp | 3185 |
| ViestiWin/ConvDat.cpp | 236 |
| ViestiWin/UnitAjanotto.cpp | 1754 |
| ViestiWin/UnitArvonta.cpp | 611 |
| ViestiWin/UnitEmit.cpp | 3091 |
| ViestiWin/UnitJoukkuetiedot.cpp | 1639 |
| cbHk/UnitAjanotto.cpp | 1961 |
| cbHk/UnitArvonta.cpp | 1270 |
| cbHk/UnitEmit.cpp | 3798 |
| cbHk/UnitKilpailijatiedot.cpp | 1900 |
| com/WinUDP.cpp | 1398 |
| dbboxm/dbbox1.cpp | 284 |
| dbboxm/dbbox2.cpp | 155 |
| include/boxm.h | 90 |
| include/boxr.h | 79 |
| include/wincom.h | 116 |
| tputilv2/MbsWcs.cpp | 51 |
| tputilv2/dbindex.cpp | 245 |
| tputilv2/oem2ansi.cpp | 122 |

### 4.2 Level 4 files (19, all C-11 harness files)

| File | Lines |
|---|---|
| Tests/core/CoreMain.cpp | 124 |
| Tests/core/HkCoreTests.cpp | 229 |
| Tests/core/HkLockTests.cpp | 145 |
| Tests/core/HkRecvTests.cpp | 402 |
| Tests/core/TcpClientTests.cpp | 163 |
| Tests/core/VAddfrecTests.cpp | 100 |
| Tests/core/VComTests.cpp | 151 |
| Tests/core/VCoreTests.cpp | 233 |
| Tests/core/VRecvTests.cpp | 262 |
| Tests/core/VSrjTests.cpp | 105 |
| Tests/core/VTulostusTests.cpp | 88 |
| Tests/fixtures/HkFixture.cpp | 86 |
| Tests/fixtures/HkFixture.h | 45 |
| Tests/fixtures/TpCore.cpp | 321 |
| Tests/fixtures/TpCore.h | 159 |
| Tests/fixtures/TpCrash.cpp | 231 |
| Tests/fixtures/TpNet.cpp | 190 |
| Tests/fixtures/VFixture.cpp | 54 |
| Tests/fixtures/VFixture.h | 41 |

## 5. Verification strength of the 177 H findings

Keep two things apart. **Source reference validation** (the validator) only checks that a cited file exists and the line number is inside the file. **Technical verification** is what the classes below describe. The strongest evidence per finding is used; a class is assigned in this order: a refuted, b shipped program, c isolated probe or harness, then by review state. Data: osaA0/havainnot.csv columns `tila`, `koe`, `tarkastus` (derived from rekisteri/havainnot.md) [verified].

| Class | Meaning | Count | Findings |
|---|---|--:|---|
| a | refuted | 1 | H-099 (C-6 step 2) |
| b | confirmed in a shipped program | 1 | H-041 (C-11 stage 4, user-approved deviation) |
| c | reproduced with an isolated probe or the test harness (does not show the trigger in the running program) | 26 | C-11 (14): H-001, H-002, H-007, H-008, H-022, H-023, H-024, H-025, H-026, H-027, H-031, H-032, H-040, H-042. C-6 (4): H-090, H-091, H-093, H-094. C-8 (2): H-128, H-129. C-9 (6): H-145, H-146, H-147, H-149, H-150, H-151. Review of these: 23 full, 1 partly (H-032), 2 none (H-001, H-002) |
| d | independent source review, fully confirmed | 89 | all other findings with state `riippumaton` (list: csv) |
| e | independent source review, partly confirmed ("osittain vahvistettu") | 43 | H-010, H-021, H-030, H-034, H-035, H-038, H-044, H-047, H-049, H-051, H-052, H-057, H-060, H-063, H-065, H-074, H-077, H-080, H-085, H-089, H-095, H-096, H-100, H-101, H-102, H-111, H-112, H-113, H-119, H-120, H-125, H-130, H-133, H-136, H-138, H-141, H-148, H-156, H-157, H-170, H-173, H-174, H-176 (H-032 is the 44th "partly" and sits in class c) |
| f | reviewer's own find, checked by the main session | 3 | H-132 (C-8), H-158 (C-9), H-162 (C-10 E1) |
| g | single-agent static finding, never independently reviewed | 13 | H-003, H-006, H-019, H-039, H-108, H-122, H-123, H-124, H-126, H-127, H-142, H-143, H-144 |
| h | not statically decidable | 1 | H-020 (needs the link map, Q12) |

Notes [verified unless marked]:

- H-019 is a reviewer's find (tallenna-samanaikaisuus.md:193) but is counted in g because the register says "not reviewed". H-017 was the other C-1 reviewer find; C-7 reviewed it, so it is in d (stale CHECKPOINT row, S3).
- H-163 and H-170 were reviewer finds in C-10 (E2, E3); the A0 base puts H-163 in d and H-170 in e [inferred from riippuvuudet.md:473-478 and the csv].
- Class c findings are all reproduced under MSVC; the base counts H-041 only in b although it was also reproduced in the harness (testaus.md:1810-1812).
- The 23 conditional severities (A0 `vakavuus_ehto`) fall in d 14, c 6, e 3.

### 5.1 Class by normalised severity

| Class | korkea | keskitaso-korkea | keskitaso | matala-keskitaso | matala | kumottu | Total |
|---|---|---|---|---|---|---|---|
| a | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| b | 1 | 0 | 0 | 0 | 0 | 0 | 1 |
| c | 19 | 1 | 6 | 0 | 0 | 0 | 26 |
| d | 13 | 0 | 46 | 7 | 23 | 0 | 89 |
| e | 1 | 0 | 18 | 8 | 16 | 0 | 43 |
| f | 0 | 0 | 1 | 1 | 1 | 0 | 3 |
| g | 0 | 0 | 2 | 4 | 7 | 0 | 13 |
| h | 0 | 0 | 0 | 0 | 1 | 0 | 1 |
| Total | 34 | 1 | 73 | 20 | 48 | 1 | 177 |

Of the 34 high findings: 1 confirmed in a shipped program (H-041), 19 reproduced (c), 13 reviewed fully (d), 1 partly (e: H-060). None is class g. Two high findings in c (H-001, H-002) lack independent source review.

### 5.2 Class by batch (era from ID range; H-021 counted under C-1, see S19)

| Class | K1 | C-1 | C-2 | C-3 | C-4 | C-5 | C-6 | C-7 | C-8 | C-9 | C-10 | C-11 | C-12 | Total |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| a | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
| b | 0 | 0 | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
| c | 2 | 2 | 8 | 2 | 0 | 0 | 4 | 0 | 2 | 6 | 0 | 0 | 0 | 26 |
| d | 2 | 9 | 5 | 7 | 17 | 7 | 10 | 8 | 6 | 4 | 10 | 0 | 4 | 89 |
| e | 0 | 2 | 4 | 5 | 6 | 2 | 6 | 6 | 5 | 3 | 1 | 0 | 3 | 43 |
| f | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 1 | 1 | 0 | 0 | 3 |
| g | 2 | 1 | 1 | 0 | 0 | 0 | 1 | 5 | 3 | 0 | 0 | 0 | 0 | 13 |
| h | 0 | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 1 |
| Total | 6 | 15 | 18 | 15 | 23 | 9 | 22 | 19 | 17 | 14 | 12 | 0 | 7 | 177 |

Strength is concentrated where probes or the C-11 harness reproduced older findings (K1 2, C-1 2, C-2 8, C-3 2 plus H-041) and in the probe batches C-6 (4), C-8 (2), C-9 (6). C-4 (23 findings) and C-10 (12) and C-12 (7) have no probe or run evidence at all: 17 + 6, 10 + 1 + 1 and 4 + 3 are static reviews only. The weakest rows: C-7 (5 never reviewed of 19), C-8 (3 never reviewed, 1 reviewer find) and K1 (H-001, H-002, H-003 and H-006 never independently reviewed; the first two were reproduced in C-11).

## 6. Refuted and weakened claims per batch (false-positive picture)

Only one claim ever became a refuted H row: H-099 (C-6 step 2; `initcom` always returns 0, so the failing branch is dead code; rekisteri/havainnot.md:108). All other refutations happened before a claim was registered, or removed part of a registered one. Counts below are items listed in the reports; the A0 base does not hold them. All rows [verified] against the cited lines.

| Batch | Refuted | Weakened, corrected or partly | One-line content | Source |
|---|--:|--:|---|---|
| Round 1 | 3 | 0 | merkistot.md claims: bcc32 reads BOM-less UTF-8 correctly; 3 broken literals (real 36); encodings cannot be unified (all refuted in C-5) | kaantajaerot.md:302-312 |
| C-1 | 1 | 1 | LukumaaratTh -> sijat2 part of H-010 refuted; H-010 partly; H-020 undecidable | tallenna-samanaikaisuus.md:187 |
| C-2 | 4 | 5 | Refuted: forced-packet loop in HK rings; WinUDP array overflow; restart replays acked packets; signed pkgclass. Partly: H-021, H-030, H-032, H-034, H-035 | tiedonsiirto.md:288-292 |
| C-3 | 5 | 5 | Refuted: relay pack is a raw struct copy; HK Release VALUKU 200; COMFILE pointer race; two programs truncate COMFILE.DAT; OEM/ANSI file names (CHECKPOINT.md lists 4). Partly: H-044, H-047, H-049, H-051, H-052 | tiedostomuodot.md:410-415 |
| C-4 | 6 | 4+ | Refuted: copydfile (HK), fclose(NULL) crash on C++Builder, tark_partio on GUI, TauluJako (nikk <= 4), HK source-point file overflow, H-061 "no /J". K1-K9: 4 partly. Triage: 141 of 275 classified warnings were false positives (51%); 14 of 33 investigate rows (42%) | CHECKPOINT.md:136-148; muistiturvallisuus.md:353, 436-444 |
| C-5 | 7 | 3 | 7 refuted (3 in merkistot.md, TpLaitteet.cpp:1892 not dead, two time_t consumers, time_t_date 2038, _BORLAND_ block); 1 weakened (NREGNLY); 2 none found; reviewer refined 3 | kaantajaerot.md:302-312, 455-458 |
| C-6 | 1 H + 8 | 8 | H-099 refuted; refuted or weakened: T4 double free, L9 live-lock reinit, L3 GUI reach, T10 KILP.DAT corruption, T9 UnitEmitSQL, 3 cycle candidates. Step 1 review: 8 of 20 partly. The "emit-rastiva cycle refuted" was itself wrong (H-103) | saikeet-ja-lukot.md:344-352, 648-650 |
| C-7 | 9 | 6 | 9 suspicions refuted (getAktTime, lahtolaji, draw randomness, TallennaMuutokset, EmitGrid, keyn, naytaRivi, LLaji, Paivita); 0 of 16 reviewed claims refuted, 6 partly with corrected effects or severities | gui-liiketoimintalogiikka.md:54-56, 323-341, 350-358 |
| C-8 | 0 | 8 | 0 of 16 refuted; 3 confirmed with changes, 5 partly; part A corrections (teejarj does not recompute ixarr, suljekilpailu dead, tmaali has no writer) | globaali-tila.md:59-62, 359-369 |
| C-9 | 8 | 5 | Refuted: relay autofile is not built; D-2 tee_aika, D-3 add, D-17 reconnectTCP, D-18 wrt_ch, C-4 fonts, C-11 kirjSQLTh, teejarj leak. 5 of 19 claims partly; 2 duplicates | virheenkasittely.md:187-192, 219-233 |
| C-10 | 1 | 4 | N-9 refuted for HK; K6 consequence corrected; K2 lowered to medium; 4 of 15 partly | riippuvuudet.md:465-478 |
| C-11 | 2 | 6 | Test-plan review: V1.2 (compile-time check for H-041) and V2.4 (writeerror_w blocks) refuted; 6 partly, 1 mixed. Shipped-run verdict "not confirmed" overridden for H-041 by the user | testaus.md:499-522, 1786-1790 |
| C-12 | 2 keys | 7 | B2: TAKAA-AJO and NOLLAKARKI are accepted by HK (refuted); A1 row count 325 not 393; C2 not fully silent; 7 of 19 partly; 2 claims more severe than the agents said | ominaisuusvalitsimet.md:70-85 |

Reading the table [inferred]:

- At H-row level the refutation rate is 1 of 177 (0.6%) after review. That figure is low because refutation happens before registration: in the batches that report both sides (C-6 step 1: 20 claims, C-7: 28 candidates to 19 rows and 9 refuted suspicions, C-9: 19 claims) roughly a quarter to a half of reviewed claims (C-6 8 of 20, C-7 6 of 16, C-8 8 of 16, C-9 5 of 19, C-10 4 of 15, C-12 7 of 19) were narrowed or partly refuted, and 5 to 10 further suspicions per batch were dropped.
- The compiler-warning route had the highest raw false-positive rate (51% in the C-4 triage). Static findings that reached an H row were mostly mechanisms that held; the weakness is in impact and reachability, which reviewers corrected in 6 to 9 claims per batch.
- The 13 never-reviewed rows (class g, section 5) and the 3 single-agent reviewer finds are outside this picture: their false-positive rate is unknown [tuntematon].
- Counts of "refuted" differ in kind: C-5, C-7 and C-9 list suspicions never registered; C-2 and C-3 list claims from agent drafts; C-11 refutes parts of a test plan, not of the code.

## 7. Open questions: proposed closing class

Classes required for closing phase C (suunnitelma.md, C-13 closing condition 3): (1) resolved, (2) run-time verification in phase D, (3) owner decision, (4) accepted as open (static; nobody needs the answer before fixing). The classification is my proposal [inferred]; the basis column cites where the documents answer or defer the question. 84 Q are not "ratkaistu" in osaA0/kysymykset.csv (72 avoin, 6 osittain, 3 omistajan_paatos, 3 ajonaikainen_todennus) [verified].

Range rows checked in the batch reports (Q19-Q25 in tiedonsiirto.md:331-341, Q26-Q34 in tiedostomuodot.md:456-468, Q35-Q42 in testaus.md:541-552, Q43-Q56 in muistiturvallisuus.md:701-718) against later documents:

- C-11 stage reports did not answer Q37 or Q38: testaus.md:1846-1848 lists the H-041 read path (Q37), the H-001 public entry (Q38), Q39 and Q41 as follow-ups. Stage 4 ran only the add path (220/221) [verified].
- C-4 step 2 did not answer any of Q43-Q56: grep of the later reports and todisteet/c6..c12 reports finds no answer; Q43 is still "disassembly" (muistiturvallisuus.md:658) [verified]. C-12's registers give partial help for Q50, Q51, Q53 only.
- C-12 did not answer Q6, Q9, Q10, Q11 or Q12 (no mention outside seuranta.md and inventaario.md) [verified].
- Later documents answer: Q3 (via Q18, C-3), Q24 (C-10 library table), Q25 (T-014), Q26 (C-11 section 18.1), Q53 (C-12 register) [inferred; the question rows were not updated].

Counts of the proposal:

| Class | Meaning | Count | Q ids |
|---|---|---|---|
| 1 | resolved (documents show it was answered) | 5 | Q3, Q24, Q25, Q26, Q53 |
| 2 | run-time verification in phase D (build, run, device, network, CI) | 39 | Q12, Q21, Q28, Q33, Q34, Q35, Q36, Q37, Q39, Q40, Q43, Q44, Q45, Q46, Q48, Q52, Q56, Q63, Q67, Q68, Q73, Q80, Q86, Q87, Q89, Q91, Q93, Q95, Q96, Q97, Q98, Q100, Q102, Q103, Q105, Q106, Q107, Q108, Q110 |
| 3 | owner decision | 15 | Q9, Q10, Q11, Q29, Q47, Q60, Q75, Q77, Q78, Q81, Q90, Q99, Q101, Q111, Q112 |
| 4 | accepted as open (static, not needed before fixing) | 25 | Q6, Q14, Q15, Q19, Q20, Q22, Q23, Q27, Q31, Q32, Q38, Q41, Q42, Q49, Q50, Q51, Q54, Q55, Q64, Q76, Q79, Q82, Q94, Q104, Q109 |
|  | **Total** | 84 |  |

By status:

| Status in kysymykset.csv | (1) | (2) | (3) | (4) | Total |
|---|---|---|---|---|---|
| avoin | 4 | 33 | 12 | 23 | 72 |
| osittain | 1 | 3 | 0 | 2 | 6 |
| omistajan_paatos | 0 | 0 | 3 | 0 | 3 |
| ajonaikainen_todennus | 0 | 3 | 0 | 0 | 3 |

Per question (Q ids not listed are resolved):

| Q | Status in kysymykset.csv | Batch | Class | Basis |
|---|---|---|---|---|
| Q3 | osittain | K1 | 1 | Part left open was exact sizes (Q18); Q18 resolved in C-3 (tiedostomuodot.md "Mitatut koot", seuranta.md Q17/Q18 row); Q3 row not updated [inferred] |
| Q6 | avoin | K1 | 4 | static check of main() guards in tputilv2 (ctrlhndl.cpp etc.); riippuvuudet.md:770 lists them as "in library or no project"; no fix depends on it |
| Q9 | avoin | K1 | 3 | policy question: allow C++11 in production code (riippuvuudet.md:150-156); owner decides |
| Q10 | avoin | K1 | 3 | orphan files may be used by other branches or external builds (inventaario.md:143-150); only the owner knows |
| Q11 | avoin | K1 | 3 | purpose of old copies (TpComY32.cpp, *-2.cpp, *-v.cpp); owner knowledge |
| Q12 | avoin | C-1 | 2 | which add_sakko is linked needs a bcc32 link map (H-020 "not statically decidable") |
| Q14 | osittain | C-1 | 4 | partly resolved by C-6; the rest is a static read of tulostasarja and is covered by the H-009 fix (lock across SendMessage) |
| Q15 | osittain | C-1 | 4 | partly resolved by C-7 (form handlers); the tallenna path itself is J7-6 (static); fix H-115/H-125 with try/finally regardless |
| Q19 | avoin | C-2 | 4 | static trace of tall_rivi (HkAjat.cpp:161-266); affects only low H-034 |
| Q20 | avoin | C-2 | 4 | static trace of kirjaahyvStream (TpComY.cpp:1318,1359); relates to H-027/H-028 fixes |
| Q21 | avoin | C-2 | 2 | needs a 3-5 station ring (C-11 two-process loopback layer, testaus.md 20.4); H-016/H-039 |
| Q22 | avoin | C-2 | 4 | static trace of relay tark_ylahto; same family as never-reviewed H-039 |
| Q23 | avoin | C-2 | 4 | static read of Kysy(wait=0); affects only persistence of the H-021 candidate |
| Q24 | avoin | C-2 | 1 | riippuvuudet.md:76-77 (C-10) lists libraries per product and all six programs build locally (CHECKPOINT.md "Kaikki 6 ohjelmaa"); winhttp.lib comes from the toolchain [inferred] |
| Q25 | avoin | C-2 | 1 | recorded as T-014 (code 14770, manual 14900; rekisteri/havainnot.md:352); which side is right is a doc/owner choice |
| Q26 | avoin | C-3 | 1 | testaus.md:284 and 1298-1305 [todennettu there]: XML path reads n_pv/valuku unclamped, HkSrj.cpp:158 check only for the old text format; seuranta row not updated [inferred] |
| Q27 | avoin | C-3 | 4 | NTFS behaviour after a power cut cannot be settled by reading; no fix depends on it |
| Q28 | avoin | C-3 | 2 | link-time winner of initindex needs a link map (build) |
| Q29 | avoin | C-3 | 3 | how users use the relay export (as KILP.DAT?) is user practice (H-046); owner knows |
| Q31 | avoin | C-3 | 4 | relay 250 builds are not shipped (T-051, ominaisuusvalitsimet.md T17); H-042 covers the relay buffer analogue |
| Q32 | avoin | C-3 | 4 | static read of t_time_l unit and wrap-around |
| Q33 | avoin | C-3 | 2 | mbtowc above 127 in the "C" locale on both compilers: small probe run |
| Q34 | avoin | C-3 | 2 | constructor order vs valuku needs a run (C-11 fixture had to pre-set kilpparam, testaus.md:134) |
| Q35 | avoin | C-11 | 2 | needs a CI image run (v143, ASan x86); push ban |
| Q36 | avoin | C-11 | 2 | LockCount encoding on the CI image: needs a CI run |
| Q37 | avoin | C-11 | 2 | H-041 read path at startup: run follow-up in testaus.md 20.4; stage 4 ran only the add path |
| Q38 | avoin | C-11 | 4 | public entry to textjtulos (H-001); not needed for the fix; still open in testaus.md 20.4 |
| Q39 | avoin | C-11 | 2 | C-4 shows C4789 only at /GL link (muistiturvallisuus.md:55,297); /we4789 under LTCG untested; needs a build |
| Q40 | avoin | C-11 | 2 | self-hosted runner interactivity: CI infrastructure run |
| Q41 | avoin | C-11 | 4 | static: which relay threads call addtall/deltall (H-040) |
| Q42 | avoin | C-11 | 4 | C-3 sizes table (tiedostomuodot.md:259-275) shows combufsize covers the largest class; SEURAT/AIKAT not explicit; H-025/H-032 fixes cover it |
| Q43 | avoin | C-4 | 2 | verify-first item of C-4: needs disassembly of the shipped build (muistiturvallisuus.md:657-658) |
| Q44 | avoin | C-4 | 2 | verify-first item of C-4 (muistiturvallisuus.md:659): needs ARES protocol description or a capture (device) |
| Q45 | avoin | C-4 | 2 | bcc32 atoi overflow behaviour: RTL probe or RTL source |
| Q46 | avoin | C-4 | 2 | needs a linker map per exe |
| Q47 | avoin | C-4 | 3 | field use of hand-edited RADAT.LST or courses over 50 controls: maintainers know |
| Q48 | avoin | C-4 | 2 | MSVC syntax-only pass over the GUI core: a build task |
| Q49 | avoin | C-4 | 4 | C-4 says H-055 exploitability (frame layout, ASLR) does not affect the fix (muistiturvallisuus.md:658-659); PE header of the bcc32 GUI builds can be read later; only HkMaali value 0x8140 known (testaus.md:1327) |
| Q50 | avoin | C-4 | 4 | key EMITAG exists in both parsers (rekisteri/asetusavaimet.csv rows 245, 460); whether it selects a TCP server is a static read for H-055 |
| Q51 | avoin | C-4 | 4 | relay parser has a FEIG key (Juk/VInit.cpp:1596,1605; riippuvuudet.md:364); TCP transport detail unread |
| Q52 | avoin | C-4 | 2 | verify-first item of C-4 (muistiturvallisuus.md:659-660): console menu combination for H-076 needs a console run |
| Q53 | avoin | C-4 | 1 | rekisteri/asetusavaimet.csv has EMVALOKI only as parser rows (287, 477), no row in cbHk/UnitCfg.cpp or ViestiWin/UnitCfg.cpp: not set from the editors, only by hand-edited cfg [inferred] |
| Q54 | avoin | C-4 | 4 | static: bound of len in lahriviCPower (H-077, low) |
| Q55 | avoin | C-4 | 4 | XML import keep-positions loop: cbHk/UnitImport.cpp unread; fold into J7-2 |
| Q56 | avoin | C-4 | 2 | vendor protocol or device: can card contents steer index P (H-055) |
| Q60 | omistajan_paatos | C-5 | 3 | owner decision (is the H-081 KUNTOSUUNN difference intended?) |
| Q63 | avoin | C-5 | 2 | bcc32 new int[negative]: small probe run (H-085) |
| Q64 | avoin | C-5 | 4 | static: wkello() length vs SubString(15,2) (H-082) |
| Q67 | osittain | C-6 | 2 | helper threads started before the failure point in aloitus: run-time (seuranta.md C-6 follow-ups) |
| Q68 | avoin | C-6 | 2 | opencomfile(1) with one connection: run-time (C-6 follow-ups) |
| Q73 | avoin | C-6 | 2 | console CTRL_CLOSE and maaliajat: run-time (C-6 follow-ups) |
| Q75 | avoin | C-7 | 3 | meaning of fitness modes 1,2,3,98,99: domain knowledge |
| Q76 | avoin | C-7 | 4 | static: relay core update after queue-only save (J7-6) |
| Q77 | avoin | C-7 | 3 | which fork is newer: git history is a 2015 import only (ominaisuusvalitsimet.md 13.4); owner knowledge |
| Q78 | avoin | C-8 | 3 | real n_lisaa and stage-change practice: how events are run (H-129) |
| Q79 | avoin | C-8 | 4 | static: SeurPos case 0 passes a record position (H-128) |
| Q80 | avoin | C-8 | 2 | GUI run: sharing violation on retry (H-133) |
| Q81 | avoin | C-8 | 3 | whether events run several readers in one HK instance: user practice (H-136, H-138) |
| Q82 | avoin | C-8 | 4 | static: where tmaali is written (H-143, low) |
| Q86 | osittain | C-9 | 2 | reader ACK semantics: code side done in C-10, device side needs a device (Q100) |
| Q87 | avoin | C-9 | 2 | SMB handle after a drop: network share test |
| Q89 | avoin | C-9 | 2 | console key routing to worker-thread prompts: console run |
| Q90 | avoin | C-9 | 3 | whether events run without the disk buffer: user practice |
| Q91 | osittain | C-9 | 2 | VCL memory manager reaction to the H-149 double free: VCL probe (seuranta.md C-9 follow-ups) |
| Q93 | avoin | C-9 | 2 | sendtcpfile stream on a peer data link: two-station run |
| Q94 | avoin | C-9 | 4 | static: are AJAT rows used to rebuild results (H-146) |
| Q95 | avoin | C-9 | 2 | HkKisaWin stream count at steady state: GUI run (H-150) |
| Q96 | avoin | C-10 | 2 | Sirit/Zebra reader thread after reconnect: device; mechanism already read from code by the main session (H-159, riippuvuudet.md:468-480) |
| Q97 | avoin | C-10 | 2 | SQL error in writer thread: MySQL run (H-160) |
| Q98 | avoin | C-10 | 2 | clean-machine start of GUI from the release ZIP (H-163, H-165) |
| Q99 | avoin | C-10 | 3 | server format and owner of 195.159.103.189:1379 (H-166): external party |
| Q100 | avoin | C-10 | 2 | Emit clock behaviour without ACK: device |
| Q101 | omistajan_paatos | C-10 | 3 | owner decision OM-C10-1 (licences of Blaise/Microsoft sample code) |
| Q102 | avoin | C-10 | 2 | can IndySystem be removed from the link: build test (T-005, T-044) |
| Q103 | avoin | C-10 | 2 | windows-latest image v143 and artifact action versions: CI run |
| Q104 | avoin | C-10 | 4 | static: relay vert0 after an SQL round trip (H-161) |
| Q105 | avoin | C-10 | 2 | H-161 injection reachability, privileges, multi-statement: MySQL run; partly static |
| Q106 | avoin | C-10 | 2 | H-169 propagation of SQL import overwrite: multi-station plus SQL run |
| Q107 | ajonaikainen_todennus | C-12 | 2 | already moved to run-time verification (C-12 closing): H-171 layout after cfgfilename |
| Q108 | ajonaikainen_todennus | C-12 | 2 | already moved to run-time verification: H-172 X)ML content |
| Q109 | avoin | C-12 | 4 | static: GETREC bounds on SQLjonoVarm.dat (H-043) |
| Q110 | ajonaikainen_todennus | C-12 | 2 | already moved to run-time verification: H-134 after a parse error |
| Q111 | omistajan_paatos | C-12 | 3 | owner decision: SEURATUL/ORAVA removal, RFID in ViestiLuenta, dead switches |
| Q112 | avoin | C-12 | 3 | owner decision: unit of AIKAKORJAUS (SEK vs SEK/100); docs, examples and git history give no answer |

## 8. Test and analysis limitations that bound the conclusions

| # | Limitation | Evidence | Mark |
|---|---|---|---|
| L1 | Only H-041 is confirmed in a shipped program (HkMaali.exe, debugger, limit pair 220/221). Even that is by a user-approved deviation: the pre-fixed verdict was "not confirmed" (criterion 7, a stray probe process in the log window). The read path of H-041 and the 250-case were not run | testaus.md:1786-1790, 1830-1845, 1886-1903; rekisteri/havainnot.md:50 | [verified] |
| L2 | 26 more findings are "reproduced" only in isolated MSVC console builds of unchanged production code with hand-set state (C-11 14, C-6 4, C-8 2, C-9 6). That shows the mechanism, not that the trigger occurs in the running program. Probes are MSVC (/MT, v145); the GUI programs are bcc32 builds that were never run. Console-built results (for example the H-149 crash 0xC0000374) are explicitly not generalised to the bcc32 GUI (virheenkasittely.md:514-540) | rekisteri/README.md:50-59; virheenkasittely.md 13.2; globaali-tila.md:429-465 | [verified] |
| L3 | bcc32 evidence is limited to: struct size/offset probes (C-3), pragma-state trace and OMF comparison (C-5), RTL console probes (C-5, C-9), warnings (C-4) and a VCL-like FPU probe (Q57). No bcc32 GUI program, VCL form or package behaviour was run | kaantajaerot.md:330-339; virheenkasittely.md:514-540 | [verified] |
| L4 | No device, network, SQL or multi-station run exists. Device protocols, MySQL, emiTag HTTP and Emit server behaviour are inferred from code and documentation (Q96-Q100). The only socket test is a loopback TCP refusal in the harness. C-10 and C-12 nothing built or run | riippuvuudet.md:500-515; ominaisuusvalitsimet.md:496-518 | [verified] |
| L5 | GUI forms are mostly unread: 284 level-1 form files (cbHk, ViestiWin, cbTp) with 73,248 lines; only 8 form units were read for business logic (C-7); 41 other writing forms, 152 parse candidates and the draw algorithm remain (J7-1..J7-3). bcc32 GUI memory safety got only weak compiler warnings | section 4; seuranta.md J7 table; muistiturvallisuus.md:96-118 | [verified] |
| L6 | The macro evaluators in C-10 and C-12 are lexical. 238 expression-defined macros (176 names) are recorded with value 1; 22 of them are tested in conditions (13 DOCTEST_*). Relay MAX_PORTTI shows 1 in valitsimet.csv/makrot.csv; the real value is 132 (Juk/VDef.h:62). Recorded values of listed macros must not be used as facts | osaA0/makroartefaktit.csv; yhteenveto.txt; ominaisuusvalitsimet.md 13.1 | [verified] |
| L7 | "Independent review" means a second agent re-read the code and tried to refute the claim (same model family, same repository, no human); it is static (rekisteri/README.md:46-52). 13 findings were never reviewed (class g), 43 only partly, and 3 are reviewer finds. | section 5; osaA0/yhteenveto.txt | [verified] (priority list), [inferred] (independence caveat) |
| L8 | Source reference validation is not technical verification. The validator checks only that a cited file exists and that the line number is not past the end of the file; it never compares content. It also scans only doc/arkkitehtuuri/*.md and rekisteri/*.md, not todisteet/ | tarkista_auditointi.py:9-10, 76-100 | [verified] |
| L9 | The register tracks levels for 650 .cpp/.h/.c files only. Form files (.dfm), project files and CI definitions have no level; C-10 inventoried 161 forms separately. C-10 and C-12 reading is not recorded at file level | tiedostot.csv extension counts (436 cpp, 209 h, 3 c, 2 H); todisteet/c10/osaA/README.md | [verified] |
| L10 | Level 2 means "targeted": its 146,223 lines are whole-file sizes, not lines read. The largest L2 files (Juk/VTulostus.cpp 9,151, Hk/HkTulostus.cpp 8,840, Hk/HkEmit.cpp 6,432, Juk/VEmit.cpp 6,221) carry many findings but were read by site | tiedostot.csv | [verified] |
| L11 | Everything is pinned to revision f7fe553 (TPsource unchanged since) and to local builds of 2026-11-24 (PE tables, bcc32 logs); CI uses v143, the local build v145 (Q35) | CHECKPOINT.md:11-12; riippuvuudet.md:218-232 | [verified] |
| L12 | The C-4 warning route left about 93% of warnings (6,569 of 7,030) in untriaged groups and about 6,500 unclassified; known buffer defects H-041, H-042, H-052, H-055, H-060 produced no warning, so absence of warning is no evidence | muistiturvallisuus.md:305-318, 665-675 | [verified] |
| L13 | Fix priorities exist for 115 of 177 H; 62 have none, among them 12 of the 34 high findings (H-007, H-008, H-009, H-014, H-022, H-023, H-024, H-025, H-026, H-027, H-031, H-040) | osaA0/yhteenveto.txt; my count from havainnot.csv | [verified] |

## 9. Open questions for the synthesis (part A level)

1. Should the A0 base assign T-001..T-010 to round 1 and keep H-021 under C-2 (register source) or C-1 (ID range)? Counts per batch in sections 3 and 5.2 follow the ID range [inferred that round 1 is right].
2. Should tiedostot.csv be updated for C-8 follow-up, C-10 and C-12 before the phase C closing (S10)? Not done here (read-only task).
3. Class g (13 findings) and the 3 reviewer finds have no independent confirmation; the 12 high findings without a priority (L13) are all reviewed or reproduced but unprioritised: part C of C-13 must assign them.
4. Q24, Q25, Q26, Q53 and Q3 look answered by later documents (class 1, [inferred]); the question rows in seuranta.md have not been updated, so the owner of the register should confirm and mark them.
5. For Q42, Q31 and Q14 the C-3 and C-4 material may already answer the question; I could not trace the chain fully [tuntematon].
6. Whether the 19 harness files at level 4 should count as "reviewed" is a register convention question (S14).
