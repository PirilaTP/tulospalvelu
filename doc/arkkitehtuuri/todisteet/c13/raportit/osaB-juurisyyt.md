# C-13 part B: common root causes, dependency chains and package critique

Scope: root-cause half of part B. Inputs: `rekisteri/havainnot.md` (177 H, 52 T; H-099 refuted and excluded), `todisteet/c13/osaA0/*.csv`, the inventory `raportit/osaB-inventointi.md` with `osaB/luokat.csv` and `osaB/suhteet.csv`, the batch reports cited in the text, and the draft packages `osaD/luonnos1_kk.csv`. This is not a new source audit. Source was opened only to confirm that a claimed common cause is one mechanism; every such check is listed in section 1.2. Paths are relative to `TPsource/V52/`. Macro values recorded by the C-10/C-12 scripts were not used (`osaA0/makroartefaktit.csv`). No identifier was deleted, renumbered or created; package changes are written as move/split/merge requests.

Labels: [verified] = I read the register row or report passage, or the code lines, and the statement follows from them; [inferred] = my conclusion from rows/reports, the chain not traced in code. Severity/priority tags in member lists come from `osaA0/havainnot.csv`: `P1`/`P2` = current priority, `high` = severity korkea or keskitaso-korkea.

## 1. Summary

- 15 root causes (JS-1..JS-15). Every open H row has one primary root cause or is marked isolated (ISO, 23 rows); the full mapping is in the appendix (section 9). Ranking is by the number of directly caused rows that are P1/P2 or high; ties broken by made-worse risk rows.
- The top four explain 25 of the 37 risk rows (P1/P2/high) that have a root cause (3 more are isolated): JS-1 input boundary (10), JS-2 unscoped locks (7), JS-3 capacity vs run-time size (5), JS-4 side effects under global locks (3). All four P1 rows are JS-1.
- Ranking by P1/P2/high understates JS-11 (errors as dialogs, non-atomic writes): 10 direct rows, all keskitaso/matala at P3-P5, but it decides whether timing data survives a disk or network failure.
- Newly identified causes not stated by any T row: the single GUI error helper `writeerror_w` as common origin of lock, shutdown and persistence findings (JS-4/JS-11 bridge); `vaiheenvaihto` used both as pause and as thread-exit signal (JS-7); capacity vs run-time size including the `_CRT_NON_CONFORMING_SWPRINTFS` idiom (JS-3); exception-unsafe core called from throwing GUI code (JS-2); HK/relay copy-fork extends to the core, not only the GUI forms of T-034 (JS-12); no ownership convention for shared handles beyond `PRFILE` (JS-8); whole-record last-writer-wins also for local form edits (JS-5); verification gap as an enabling cause (JS-13).
- One unregistered candidate found while checking JS-4 (section 3.1): closing any GUI error dialog with X probably exits the program (H-116 mechanism inside `errorMsgHandler`). Not given an id; main session decides.
- 27 concrete package change requests (section 6); the main ones: split KK-C13-3, KK-C13-10 and KK-C13-15; move the `writeerror_w` rows into KK-C13-5; move the COMFILE layout rows (H-025, H-048, H-050) together into KK-C13-9; move T-019, T-039, T-035, T-021, T-026, T-008 to the packages whose fix they actually describe; dissolve the incoherent KK-C9-3 and KK-C12-1 by moves.

### 1.1 Ranked root causes

| Rank | Id | Root cause | Direct rows | Direct P1/P2/high | Made-worse P1/P2/high | T rows | Programs | Fix |
|---|---|---|--:|---|---|---|---|---|
| 1 | JS-1 | No validated-input boundary | 27 | 10: H-026(high) H-032(P1,high) H-055(P2,high) H-056(P2,high) H-060(P2) H-061(P1) H-070(P2,high) H-071(P1,high) H-072(P1,high) H-109(P2,high) | 4: H-002(P4,high) H-008(high) H-042(P3,high) H-073(P2) | T-022, T-037 | all | bounded now; validator layer later |
| 2 | JS-2 | Unscoped lock pairs, no exception boundary | 14 | 7: H-008(high) H-024(high) H-040(high) H-087(P2,high) H-090(P2,high) H-110(P2,high) H-160(P2,high) | 1: H-014(high) | T-036 (GUI only) | HK+relay, console+GUI | bounded + guard helper |
| 3 | JS-3 | Capacity not tied to run-time size; unbounded wide formatting | 14 | 5: H-025(high) H-041(P3,high) H-042(P3,high) H-073(P2) H-171(P2,high) | 1: H-129(P2,high) | none (T-026, T-024, T-051 parts) | all, per build | bounded; size from run time + string helper |
| 4 | JS-4 | Side effects under coarse global locks; synchronous `writeerror_w` | 9 | 3: H-009(high) H-091(P2,high) H-093(P2,high) | 1: H-007(high) | T-031, T-032, T-033, T-040 (part) | HK+relay; GUI and console | bounded per cycle; lock order + async errors |
| 5 | JS-5 | Whole-record last-writer-wins replication and editing | 8 | 3: H-007(high) H-022(high) H-023(high) | 0: - | none (H-016) | mostly HK | bounded checks; versioning = wire change |
| 6 | JS-6 | Start-up-only construction of in-memory state | 14 | 2: H-128(high) H-129(P2,high) | 3: H-023(high) H-031(high) H-109(P2,high) | T-038, T-039 | stage: HK; derived: both | bounded first; central reset/rebuild |
| 7 | JS-8 | No ownership convention for shared handles/pointers | 11 | 2: H-149(P2,high) H-150(P2) | 1: H-027(high) | T-042, T-020 | HK+relay | bounded; owner-by-reference API |
| 8 | JS-9 | Hand-rolled device connections, no liveness | 5 | 2: H-027(high) H-159(P2,high) | 1: H-056(P2,high) | T-048 | all with readers | bounded; device layer study |
| 9 | JS-7 | Flag+sleep thread lifecycle; `vaiheenvaihto` pause vs exit | 8 | 2: H-014(high) H-031(high) | 0: - | T-030 | HK (stage), both (shutdown) | bounded; thread registry |
| 10 | JS-10 | Shared globals/statics and VCL without owner thread or lock | 17 | 1: H-107(P2) | 0: - | T-003, T-033 | HK+relay; VCL GUI only | bounded; ownership map + queued UI |
| 11 | JS-12 | Copy-forked code HK/relay (GUI and core), console/GUI | 5 | 0: - | 5: H-007(high) H-024(high) H-025(high) H-040(high) H-111(P3,high) | T-034, T-008, T-035 | both domains | bounded + J7-7 rule; convergence later |
| 12 | JS-13 | Verification gap: warnings, tests, release check | 3 | 0: - | 2: H-001(P3,high) H-002(P4,high) | T-021, T-044 | all | process change |
| 13 | JS-11 | Errors as dialogs, not return values; non-atomic writes | 10 | 0: - | 1: H-009(high) | T-040, T-041 (T-019) | all | bounded per path; error policy |
| 14 | JS-14 | Configuration and switch drift | 4 | 0: - | 1: H-041(P3,high) | T-045, T-050, T-051, T-017, T-024, T-052 | all | bounded per key; one key/switch table |
| 15 | JS-15 | Two toolchains/RTLs, no portability or encoding policy | 4 | 0: - | 0: - | T-004, T-016, T-020, T-023, T-025..T-027 | MSVC vs bcc32 | bounded; encoding needs decision |
| - | ISO | isolated one-off defects | 23 | 3: H-001(P3,high) H-002(P4,high) H-111(P3,high) | - | - | - | per row |

Rank = direct P1/P2/high rows, then made-worse risk rows, then direct count. JS numbers are stable labels assigned before the final count; the rank column, not the number, gives the risk order (JS-7 ranks 9th, JS-11 13th).

### 1.2 Code checks made to confirm a mechanism

| # | What was checked | Lines read | Result |
|---|---|---|---|
| C1 | `writeerror_w` and `errorMsgHandler` | `cbTp/cbTpApu.cpp:445-490` | one global `perr_ptrs2`, synchronous `SendMessage`, busy wait `while (perr_ptrs2.vast == 0) Sleep(200)`, button 2 "Poistu ohjelmasta" sets `suljesuoraan` and calls `Close()` [verified] |
| C2 | `TFormSelect3` button results | `cbTp/UnitSelect3.cpp:38-60`, `:98-124` | buttons set `ModalResult` 1/2/3; X close not handled, so it yields VCL `mrCancel` = 2 [verified code; VCL value inferred as in H-116] |
| C3 | all uses of `vaiheenvaihto` | grep over `*.cpp`/`*.h` | thread loops `while (taustaon && !vaiheenvaihto && ...)` at `Hk/HkIV.cpp:796`, `cbHk/UnitMySQL.cpp:654`, `cbHk/UnitSakkoKierr.cpp:258`, `cbHk/TulosUnit.cpp:1893`, `cbHk/UnitEmit.cpp:838` exit; `cbHk/WinHk.cpp:1616-1849`, `cbHk/UnitAikavert.cpp:96`, `HkConsole/HkCons2.cpp:1581` only skip; relay GUI files shadow it with a file `static` under `MAXOSUUSLUKU` (`cbTp/UnitEmitSQL.cpp:29-31`, `cbTp/UnitNaytaTulos.cpp:30`) [verified] |
| C4 | HK vs relay record apply on receive | `Hk/HkCom32.cpp:242-334`, `Juk/VCom.cpp:570-584` | HK `kilp.GETREC(d)` at `:260` before `EnterCriticalSection` at `:326`; relay `GETREC` inside the lock at `Juk/VCom.cpp:579-580` [verified] |
| C5 | scoped-lock helpers and exception handling | grep `LeaveCriticalSection` in headers, `__finally`, `try`/`catch` per directory | no lock guard class, no `__finally`; `try` count 0 in `Hk/`, `Juk/`, `Tp/`, `dbboxm/` (word count, comments included) vs 141/103/17 `EnterCriticalSection` there; GUI dirs 4-8 `try` [verified] |
| C6 | swprintf idiom | `VS/Hk/HkMaali/HkMaali.vcxproj:118`, `:135`; `Hk/HkAjat.cpp:424-471`; `Hk/HkEmit.cpp:6165-6185` | `_CRT_NON_CONFORMING_SWPRINTFS` defined; two-argument `swprintf` into `wchar_t ln[120]` and `msg[400]` [verified]; configurations differ in `NREGNLY` (32 vs 4) and `VALUKU=200` [verified] |
| C7 | ownership idioms | `tputilv2/Wrttext.cpp:279-300`; `Hk/HkIV.cpp:740-751`, `:826-832`; `Hk/HkIx.cpp:1906-1912` | `closeprfile(PRFILE *pf)` frees and clears only the `prf[]` registry, never the caller field; `etls` freed by `close_bdg_t` and lazily re-created by the producer with no lock between NULL test and `InitializeCriticalSection`; `femitfile` nulled, sibling handle not [verified] |
| C8 | `teejarj` re-run branch and `setpos` | `Hk/HkIx.cpp:1598-1615`, `:205-226` | separate "uusinta" branch zeroes `idindex->ix` but not `nrec`; delete branch in `setpos` unreachable (`!ret && d == dd` inside `ret || d != dd`) [verified] |
| C9 | forked `autofile` | grep | `int autofile(int kaikki)` at `Hk/HkTulostus.cpp:7066` and `int autofile(void)` at `Juk/VTulostus.cpp:7137` (plus second variants at `:7356`, `:7341`) [verified] |

## 2. Root causes in detail

Format per cause: statement; mechanism with evidence; directly caused rows; made worse / made possible; T rows that state it; programs; bounded fix vs remove the cause; must not break.

### JS-1 No validated-input boundary

- **Statement.** Bytes from LAN packets, device frames, local files, configuration and GUI fields are parsed directly into fixed arrays; index and length fields taken from the input are used without a range check, and no shared bounded reader exists. [verified: T-022 row, `muistiturvallisuus.md` 15]
- **Mechanism.** One idiom, many parsers (no common helper): `class_len` 0 skips the length check and `pkgclass` is not bounded (H-032, `Tp/TpComY.cpp:1001`); FILESEND `d.fl.len` not compared with the class length (H-071, `Tp/TpComY.cpp:3004-3037`); EXTRA indices `eralahto[d2]` from the packet (H-072, `Hk/HkCom32.cpp:640-645`); `controls[i]`, `i = atoi(...)` (H-055, `Tp/TpLaitteet.cpp:382-403`); FEIG frame length into `FEIGbuf[200]` (H-070, `Tp/TpLaitteet.cpp:805-850`); `paikat[k-1]` (H-060, `Hk/HkSak.cpp:159-171`); count pass and fill pass disagree (H-058, H-074); XML `depth` -1 (H-057, `tputilv2/xmlparser.cpp:267-274`); GUI `ItemIndex` -1 or empty selection used as index (H-113, H-114, H-109, H-112; T-037). Reach: no authentication or keyed integrity (H-029, `com/WinUDP.cpp:1115-1121`), so one forged packet suffices for the P1 rows. [verified rows]
- **Directly caused (27; P1/P2 9, high 8):** H-026(high) H-029 H-032(P1,high) H-033 H-036 H-049(P4) H-052(P4) H-055(P2,high) H-056(P2,high) H-057(P4) H-058(P4) H-060(P2) H-061(P1) H-065(P4) H-070(P2,high) H-071(P1,high) H-072(P1,high) H-074(P4) H-082(P4) H-083(P5) H-084(P5) H-085(P4) H-109(P2,high) H-112(P3) H-113(P3) H-114(P3) H-124(P4)
- **Made worse / made possible:** H-002(P4,high) H-008(high) H-042(P3,high) H-073(P2) H-115(P3) H-125(P4) H-166(P3)
- **T rows stating it:** T-022 (C-4 parsers), T-037 (GUI combo boxes); T-043 (unauthenticated external links) widens reach.
- **Programs:** all. Packet receive in shared `Tp/` (both builds) plus `Hk/HkCom32.cpp` and `Juk/VCom.cpp`; device readers in `Tp/TpLaitteet.cpp` (console and GUI); GUI-field rows only HkKisaWin/ViestiWin.
- **Bounded fix enough?** Yes for every listed row: each is a range or length check (`muistiturvallisuus.md` 15 "fixable on current evidence"). Removing the cause: one validating reader per input kind (packet: length vs class table and index vs array bound; device frame; text field) that all parsers use, with reject-and-count instead of crash or stop; authentication is a separate protocol decision (H-029).
- **Must not break:** packet layouts and the class table between program versions on one network (`tiedonsiirto.md` "Käännös ja yhteensopivuus"); HK and relay packet structs differ (`Hk/HkDef.h:1301-1307` vs `Juk/VDef.h:1247-1253`), so limits are per build; a rejected packet must not stop the receive ring (H-026 shows that stopping is itself a defect); valid device frames (emiTag, FEIG, ARES, Kurvinen) must parse byte-identically; configuration and import files the old program accepted should still load (clamp and warn rather than refuse, unless the owner decides otherwise).

### JS-2 Unscoped lock pairs and no exception boundary

- **Statement.** Every lock is a manual `EnterCriticalSection`/`LeaveCriticalSection` pair. There is no guard object, no `__finally`, and the core has no `try` at all, while it is called from VCL handlers and DBX code that throw. Any early `return`, `break`, `goto` or exception between the pair leaks the lock; a `Leave` on a path that never entered corrupts it. [verified C5]
- **Mechanism.** Early-return leaks in the core: `tall_sak` (H-008, `Hk/HkSak.cpp:89`, `:99`), `vaihdakilp` (H-012), relay `tark_trk` (H-024, `Juk/VCom.cpp:690-706`), relay `addfrec` enters `kilpdat_CS` twice (H-040, `Juk/vdat.cpp:178-192`), `EmitHtmlOsuus` never leaves `print_CS` (H-087, `Juk/VEmitHtml.cpp:187-293`), `tall_regnly` returns past the `Leave` (H-090, `Hk/HkIV.cpp:2673-2782`), `break` skips re-entry then double `Leave` (H-089, `Hk/HkEmit.cpp:5191-5241`). Exception leaks in GUI handlers: grid exception inside `tall` (H-115), division by `pari` inside `tall` (H-125), SQL exception skips `Leave` (H-162, `cbHk/UnitExport.cpp:420-450`); uncaught `TDBXError` ends the program from a thread (H-160) or skips start-up (H-163).
- **Directly caused (14; P1/P2 4, high 7):** H-008(high) H-012 H-024(high) H-040(high) H-087(P2,high) H-088(P3) H-089(P3) H-090(P2,high) H-110(P2,high) H-115(P3) H-125(P4) H-160(P2,high) H-162(P3) H-163(P3)
- **Made worse:** H-014(high) H-148(P4)
- **T rows stating it:** T-036 (GUI handlers only). The core early-return part and the DBX exception part are not stated by any T row (new extension). H-099 (refuted) is not counted.
- **Programs:** HK and relay, console and GUI; exception paths only in the GUI (bcc32, VCL/DBX).
- **Bounded fix enough?** Each site is a 1-5 line fix, but without a shared guard helper the next early return re-creates the class. Removing the cause: a small hand-written scope-guard class (C++03; a standard `lock_guard` is not available in bcc32 classic [inferred]) used for all new and touched pairs, and an exception boundary at every thread entry and VCL handler that calls the core (catch, log, release, continue).
- **Must not break:** which lock is taken when (otherwise JS-4 cycles change); recursive `CRITICAL_SECTION` ownership (H-087 leaks one recursion level per team; a guard must release exactly once per enter); console builds have no VCL exceptions; MSVC and bcc32 must both compile the helper.

### JS-3 Capacity not tied to the run-time size

- **Statement.** Buffer and table capacities are compile-time constants (`KILPBUFSIZE`, `KILPRECSIZE`, `MAXOSUUSLUKU`, `MAX_LAHPORTTI`, `NREGNLY`, fixed `wchar_t` lines) or start-up allocations (`maxrec+2`), while the amount written is a run-time quantity (record size from `n_pv`/`valuku`, XML counts, index rows after deletes, legs x splits, path length). Writers do not check capacity, and the constants differ per build configuration (C6), so a size safe in one product is not in another. Wide strings are built with two-argument `swprintf` (`_CRT_NON_CONFORMING_SWPRINTFS`) or `wcsncpy` with the wrong count. [verified C6 and rows]
- **Mechanism.** H-041 `char buf[KILPBUFSIZE]` = 10000 vs records up to 11 200 bytes in shipped Release builds (confirmed by run, C-11); H-042 relay XML counts vs `KILPRECSIZE`/`MAXOSUUSLUKU`; H-025 relay `combufsize` without the 14-byte header; H-129 `addix` past `maxrec+2`; H-076 buffer sized from a counter that is not reset; H-122 `ix_jrj[1000]`; H-171 `wcsncpy(cfgfilename, cfgflname, 120)` into `[41]`; H-059 `ln[120]`, H-073 `msg[400]`, H-079 `mode[16]`, H-066 `hkey[22]` used as a 30/50-byte key. Unlike JS-1, no malformed input is needed: a large competition (H-041, H-076, H-129) or a long path (H-171) is enough. [verified rows; C6]
- **Directly caused (14; P1/P2 2, high 4):** H-025(high) H-035 H-041(P3,high) H-042(P3,high) H-048 H-059(P3) H-066(P5) H-067(P5) H-073(P2) H-076(P3) H-077(P5) H-079(P4) H-122(P4) H-171(P2,high)
- **Made worse:** H-030 H-044 H-050 H-129(P2,high)
- **T rows stating it:** none as a whole (new). Parts: T-026 (bcc32 `swprintf(buf, n, ...)` does not terminate), T-024 and T-051 (per-build sizes).
- **Programs:** all; which rows trigger depends on the build configuration.
- **Bounded fix enough?** Yes per site. Removing the cause: size buffers from the run-time record size and check counts once where they enter (XML and configuration load), plus one bounded string-format helper that behaves the same in both RTLs.
- **Must not break:** `KILP.DAT` record size and layout per build (`tiedostomuodot.md`): enlarge or heap-allocate buffers, never change the record; `combufsize` defines the `COMFILE.DAT` stride, so H-025 changes the disk-queue layout (owner decision); bcc32 `-a1`/`-a8` struct layout (T-023); thread stacks are small and fixed at `_beginthread` (e.g. 40960 bytes for `pros_bdg_t`, `Hk/HkIV.cpp:830`, seen in C7), so a larger stack buffer can move the overflow instead of removing it [inferred].

### JS-4 Side effects inside coarse global critical sections

- **Statement.** Data updates and their side effects run in one synchronous call chain under coarse global locks (`tall`, `print`, `emit`, `ajat`, `outb`, `tcpip`, `loki`). Side effects include the modal error dialog of `writeerror_w`, printing, logging, socket I/O and SQL. There is no documented lock order, so independent call chains form cycles. [verified: `saikeet-ja-lukot.md` 7.1-7.3; C1]
- **Mechanism.** `writeerror_w` (C1): the worker fills the global `perr_ptrs2`, calls `SendMessage` to the main window and spin-waits until `vast` changes, while it still holds `outb`, `ajat` or `emit` (T-032 sites `Tp/TpComY.cpp:1133`, `:1471`, `:1602`; `Hk/HkAjat.cpp:217`; `Hk/HkIV.cpp:2779`; `Juk/VEmit.cpp:1460`). Lock cycles: `tall`-`print` via `tallenna` -> `lisaToimet` -> `sivuTuloste` (H-091), `print`-`emit` (H-092), `loki`-`tcpip` (H-093), relay `rastiva`-`emit` (H-103). Blocking `connect`, DNS, `select` 10 s and `Sleep` under the single `tcpip_CS` (T-031, H-096). SQL query under `tall` (H-162).
- **Directly caused (9; P1/P2 2, high 3):** H-009(high) H-021 H-091(P2,high) H-092(P3) H-093(P2,high) H-096(P4) H-103(P3) H-153(P4) H-167(P4)
- **Made worse:** H-007(high) H-015 H-019 H-116(P3) H-144 H-162(P3)
- **T rows stating it:** T-031, T-032; T-033 (modal dialogs inside `vaihdavaihe` under locks); the dialog half of T-040. That one helper (`writeerror_w`) links these to the persistence findings of JS-11 is new.
- **Programs:** HK and relay; GUI via `SendMessage`, console via keyboard prompts under `autotul`/`print` (`Hk/HkTulostus.cpp:1029-1031`).
- **Bounded fix enough?** Per cycle yes (move the side effect out of the lock, or fix the order on one edge). Removing the cause: a documented lock hierarchy (the edge list of `saikeet-ja-lukot.md` 7.1 is acyclic apart from the 7.2 pairs) and worker-side error reporting that queues the message and returns an error code instead of waiting for an answer under a lock.
- **Must not break:** every error still reaches the operator; console prompts (`tputilv2/perr.cpp`) differ from the GUI helper and need their own path; handlers that assume the worker is paused while they run (T-033); printer output order.

### JS-5 Whole-record last-writer-wins replication and editing

- **Statement.** Records are replicated and edited as whole-record snapshots without version, timestamp or origin; the last copy to arrive or to be saved wins, both between workstations and between a long-open form and the core.
- **Mechanism.** No version/origin in the packet (H-016, `Hk/HkDef.h:1235-1313`, `Hk/HkCom32.cpp:195-233`); HK `tark_kilp` reads the record before `tall_CS` and saves the stale merge (H-007; relay reads inside the lock, C4); the form holds `Kilp_0` and rewrites it whole (H-017, `cbHk/UnitEmit.cpp:2469-2791`); concurrent adds get the same record number and stage data is applied without a bib check (H-022); `VAIN_TULOST` applied to the receiver's stage (H-023); resend of acknowledged snapshots (H-030); old state via another mesh route (H-039); in-flight packet applied twice (H-034). [verified rows, C4]
- **Directly caused (8; P1/P2 0, high 3):** H-007(high) H-016 H-017 H-022(high) H-023(high) H-030 H-034 H-039
- **Made worse:** H-029 H-168(P4)
- **T rows stating it:** none; H-016 is the only general statement and it is an H row (new root cause at T level).
- **Programs:** mostly HK (H-007, H-022, H-023); both for H-016, H-030, H-039.
- **Bounded fix enough?** Most risk goes with bounded checks that keep the wire format: read inside the lock (H-007, as relay does), check bib and stage before applying (H-022, H-023), compare-before-write in the form (H-017; relay does it inside the lock). Removing the cause needs a version or origin field and a merge rule: a wire-format change (owner decision; Q3, Q18).
- **Must not break:** workstation protocol between program versions; `KILP.DAT` layout if a version is stored per record; HK and relay packet layouts.

### JS-6 Start-up-only construction of in-memory state

- **Statement.** In-memory state (stage-dependent globals, derived indexes, counters and rank chains) is designed to be built once at start (`aloitus`, `teejarj(false)`). In-run transitions (stage change, delete, renumber, series change) patch it at each call site, through a separately written re-run branch, with no central reset table, no invariant check, and restart as the only repair.
- **Mechanism.** `teejarj` has its own "uusinta" branch that resets only part of what the first-time branch allocates: `idindex->ix` is zeroed, `nrec` is not (H-129); the `setpos` delete branch is unreachable (H-128) (C8). `vaihdavaihe` resets items case by case; `aikaflname`, `maxvaluku`, `eralahto`, `nemitvar`, `maaliajat` and `korRaja` stay stale (`globaali-tila.md` 4.2; H-131, H-141, H-143). `aloitus` edits file names in place (H-132, H-133). Counters are kept by hand-paired increments (H-130, H-135, H-139, H-140) and trusted by consumers that size buffers from them (H-076; `globaali-tila.md` 5).
- **Directly caused (14; P1/P2 1, high 2):** H-102(P4) H-128(high) H-129(P2,high) H-130 H-131 H-132 H-133 H-134(P3) H-135 H-139 H-140 H-141 H-142 H-143
- **Made worse:** H-013 H-023(high) H-031(high) H-076(P3) H-105(P3) H-109(P2,high) H-174(P3)
- **T rows stating it:** T-038 (stage reset) and T-039 (derived structures) state the two halves; the common assumption "restart is the repair" (`globaali-tila.md` 5: the only in-run rebuild is the HK stage change) is not stated as one cause [inferred].
- **Programs:** stage change HK only (relay has none, `saikeet-ja-lukot.md` 4.3); derived-structure rows HK and relay.
- **Bounded fix enough?** For H-129 and H-128 yes, and they must ship first (P2, reproduced in isolation). Removing the cause: one reset/rebuild routine driven by the C-8 reset table, or rebuild from records on each transition (the start-up path already does it), plus debug-build invariant checks.
- **Must not break:** derived structures are memory-only, so `KILP.DAT` is not at stake (`globaali-tila.md` 5); `vaihevalinta.cfg`; per-stage AJAT/COMFILE file names (H-132 may follow a documented naming rule); a restarted program must behave as today.

### JS-7 Flag-and-sleep thread lifecycle; `vaiheenvaihto` with two meanings

- **Statement.** Threads are stopped by polling global flags plus fixed `Sleep`s; no join, handles unused. The global `vaiheenvaihto` is a "pause" signal for some threads and an exit condition for others, and nothing restarts the threads that exit. [verified C3; `saikeet-ja-lukot.md` 4.1]
- **Mechanism.** Exit-on-flag loops (C3): `pros_bdg_t` (`Hk/HkIV.cpp:796`) exits and frees `etls` while the reader still writes (H-094); `kirjSQLTh` (`cbHk/UnitMySQL.cpp:654`, H-014); `HaeAvoimetTh` and `ScrollThread` (H-097); a refused console stage change leaves the flag set, so all of them stop for good (H-031). Shutdown: no wait for `tarkcom` (H-038), queues deleted without lock (H-095), `inLopetus` latch (H-104), main-thread wait loop without message pump (H-167), `_endthread` before resetting the handle (H-098).
- **Directly caused (8; P1/P2 0, high 2):** H-014(high) H-031(high) H-038 H-094(P3) H-095(P4) H-097(P3) H-098(P4) H-104(P3)
- **Made worse:** H-101(P4) H-167(P4)
- **T rows stating it:** T-030 (flag+sleep). The double meaning of `vaiheenvaihto` is new.
- **Programs:** HK GUI and console (relay GUI files shadow the name with a `static` false, `cbTp/UnitEmitSQL.cpp:29-31`); shutdown rows both domains.
- **Bounded fix enough?** Yes per row (restart after stage change or skip instead of exit; clear the flag on refusal). Removing the cause: a thread registry with start/stop/join and a pause state separate from stop.
- **Must not break:** a stage change must still quiesce every writer before `vaihdavaihe` frees memory (that is why some loops exit); shutdown order `closecom` before `closefile` (`saikeet-ja-lukot.md` 4.2); console `_exit`/`exit`.

### JS-8 No ownership convention for shared handles and pointers

- **Statement.** Shared resources (`PRFILE`, `TextFl`, `FILE`, sockets, queues, thread handles) are raw pointers or handles in globals or parameter structs. Whoever closes one frees it, the shared field is not cleared, struct assignment copies the pointer, and no resource has a defined owner. [verified C7]
- **Mechanism.** `closeprfile(PRFILE *pf)` frees and clears only the `prf[]` registry (T-042; H-149, H-151); `autotulostusparam = lsttulparam` copies `lstf` (H-152); `new TextFl` inside a loop, only the last deleted (H-150); the consumer thread frees `etls`, the producer re-creates it lazily (H-094); `fclose(femitfile)` also closes `emitfile`, which is not nulled (H-105); `openportTCP` error path frees the struct with sockets open (H-155); a second `accept` replaces the socket (H-028); `PRFILE` kept after its DC is deleted (H-154); `fclose(NULL)` (H-063, T-020).
- **Directly caused (11; P1/P2 2, high 1):** H-028 H-063(P4) H-105(P3) H-149(P2,high) H-150(P2) H-151(P3) H-152(P4) H-154(P4) H-155(P4) H-157(P5) H-158(P5)
- **Made worse:** H-027(high) H-037 H-054 H-094(P3) H-096(P4)
- **T rows stating it:** T-042 (`PRFILE` only); T-020 (`fclose(NULL)`). The general convention gap across C-2, C-6, C-8 and C-9 is new.
- **Programs:** HK and relay; autofile/PRFILE rows in both forks (C9).
- **Bounded fix enough?** Yes per site (null after close, delete in the loop, close sockets on error). Removing the cause: close functions take the owner field by reference (`PRFILE **`), one owner per resource, copies hold ids or non-owning references.
- **Must not break:** printer/GDI paths (`prno` -2, -1, 1, 2 in `tputilv2/Wrttext.cpp:279-300`), autofile publication timing, socket reconnect behaviour.

### JS-9 Hand-rolled device connections without liveness

- **Statement.** Each reader thread implements its own open, retry and reconnect loop; there is no shared connection state machine and no liveness monitoring. [verified: T-048 row, `riippuvuudet.md` 19.4]
- **Mechanism.** Inverted return test on reconnect (H-159, `Tp/TpLaitteet.cpp:1805-1847`); vanished serial reader never reopened (H-164); TCP client does not retry (H-027); HTTP without status check or timeouts (H-166); no keepalive, `lastmsg` written but never read (T-048).
- **Directly caused (5; P1/P2 1, high 2):** H-027(high) H-159(P2,high) H-164(P3) H-166(P3) H-170(P5)
- **Made worse:** H-034 H-056(P2,high) H-108(P4) H-138 H-155(P4)
- **T rows stating it:** T-048.
- **Programs:** all products with readers (`Tp/TpLaitteet.cpp`, `Tp/ZebraReader.cpp`, `com/`).
- **Bounded fix enough?** Yes for H-159 (P2, one condition) and H-164; the common layer is a later study item (KK-C10-2).
- **Must not break:** device protocols; reader numbering and `kanava` mapping (T-024); Emit ACK semantics (H-145).

### JS-10 Shared mutable globals and VCL access without an owning thread or lock

- **Statement.** About 850 externs (T-003), function-static return buffers and file statics are shared across threads without an assigned lock, and VCL objects are used from worker threads; the only marshalling is synchronous `SendMessage` (T-033). [verified rows; `globaali-tila.md` 6]
- **Mechanism.** Unlocked `tallenna` call sites (H-010); emit file and index without `emit_CS` (H-011, H-045, H-137); `bdg_t` (H-136); SQL queue indices (H-100); static return buffers and `perr_ptrs2` (H-019, H-144); static thread argument or counter (H-108, H-138); a viewer setting the global `combufsize` (H-050); shared static `tarecbuf` (H-051); VCL from workers (H-018, H-101, H-106, H-107; e.g. `kirjSQLTh` reads `FormMySQL->CBVarmista->Checked`, `cbHk/UnitMySQL.cpp:655`, seen in C3).
- **Directly caused (17; P1/P2 1, high 0):** H-010 H-011 H-018 H-019 H-045 H-047 H-050 H-051 H-100(P4) H-101(P4) H-106(P3) H-107(P2) H-108(P4) H-136 H-137 H-138 H-144
- **Made worse:** H-102(P4)
- **T rows stating it:** T-003, T-033.
- **Programs:** HK and relay; VCL rows GUI only.
- **Bounded fix enough?** Yes per site. Removing the cause: assign each shared global to one lock or one thread (`globaali-tila.md` 6 is the inventory) and queued UI marshalling. Widening `tall_CS` is not a fix (it worsens JS-4).
- **Must not break:** hot-path cost (`tall` has 263 live `Enter` sites, `saikeet-ja-lukot.md` 6); HK and relay global sets differ.

### JS-11 Errors as dialogs, not return values; non-atomic writes

- **Statement.** Write errors are shown to the operator and not returned to the caller; success is acknowledged regardless; files are rewritten in place without temp file, rename or commit. [verified: T-040, T-041 rows; `virheenkasittely.md` 12.7]
- **Mechanism.** `dbbox` functions are `void` or callers ignore -1 (H-015); `tall_emit` always returns 0, so the reader gets an ACK (H-145); `writeajat` advances before writing (H-146); `WriteLine(...) == WEOF` is never true (H-147); `addfrec` reopen unchecked (H-148); `cjseur` advances on a failed write (H-044); a parse error is treated as a missing file (H-174).
- **Directly caused (10; P1/P2 0, high 0):** H-015 H-044 H-053 H-054 H-145(P3) H-146(P3) H-147(P3) H-148(P4) H-156(P5) H-174(P3)
- **Made worse:** H-009(high) H-021 H-043 H-117(P3) H-134(P3) H-153(P4)
- **T rows stating it:** T-040, T-041; T-019 (no checksum, no version) makes torn writes undetectable.
- **Programs:** HK and relay, console and GUI.
- **Bounded fix enough?** Yes per path (`virheenkasittely.md` 13.4, KK-C9-2). Removing the cause: an error policy (return values plus one reporting channel), temp file plus rename for published files, `_commit` for queues; checksums belong to T-019 (format change).
- **Must not break:** `KILP.DAT` format (a checksum needs a version, T-019); reader ACK behaviour (withholding an ACK changes what the Emit reader does, Q86/Q100); `COMFILE.DAT` layout; published file names polled by consumers.

### JS-12 Copy-forked code: HK vs relay, console vs GUI

- **Statement.** HK and relay are parallel copies both in the GUI forms (T-034) and in the core (`Hk/` vs `Juk/`: C4 `tark_kilp`, C9 `autofile`), and console vs GUI differ by `#ifdef` blocks (T-008); fixes land on one side. [verified C4, C9; rows]
- **Mechanism.** HK reads before the lock, relay inside (H-007, C4); range guard only in relay (H-117); argument shift only in relay (H-121); flag set only in HK (H-069); comparison vs assignment in the name-search handler (H-127); GUI-only code swap in the shared core (H-081); relay record without constructor (T-035; H-109, H-112).
- **Directly caused (5; P1/P2 0, high 0):** H-069(P5) H-081(P5) H-117(P3) H-121(P4) H-127(P5)
- **Made worse:** H-007(high) H-017 H-020 H-024(high) H-025(high) H-040(high) H-046 H-075(P4) H-106(P3) H-111(P3,high) H-112(P3) H-126(P5)
- **T rows stating it:** T-034 (GUI forms only; the core extension is new), T-008, T-035.
- **Programs:** both domains; console vs GUI.
- **Bounded fix enough?** Yes per row, with the J7-7 rule (check the counterpart, `gui-liiketoimintalogiikka.md` 13.2). Removing the cause: shared implementation parameterised by build (KK-C13-22), only after tests exist (JS-13).
- **Must not break:** intended differences (relay processes the card in the form, HK in the core; `gui-liiketoimintalogiikka.md` 4); struct layouts per build.

### JS-13 Verification gap (process)

- **Statement.** Compiler warnings are suppressed or not analysed, tests do not run in CI, and the release package is not verified. [verified: T-021, H-006, T-044 rows; `raportit/osaF-testausaineisto.md` 3]
- **Mechanism.** MSVC `/W3` without `/analyze`, bcc32 `-w-par -w-aus` (T-021); warnings pointed directly at 16 of the 23 C-4 defects (`muistiturvallisuus.md:569`); `Tests/run.sh` broken and tests not in CI (H-006); the collector follows only static imports (T-044 -> H-165 -> H-163).
- **Directly caused (3; P1/P2 0, high 0):** H-003(P5) H-006 H-165(P3)
- **Made possible:** H-001(P3,high) H-002(P4,high) H-062(P3) H-064(P4) H-068(P5) H-163(P3), and most ISO coding errors [inferred]. Whether missing tests caused the one-sided fixes of JS-12 is not shown (history not read, Q77) [inferred, unproven].
- **T rows stating it:** T-021, T-044.
- **Bounded fix?** No; a process change (CI job for the doctest suite and the C-11 harness, warning baseline). It is the precondition for safe work on JS-6, JS-12 and the T-centred packages.
- **Must not break:** CI runner constraints (self-hosted Windows, bcc32 SDK version; CLAUDE.md).

### JS-14 Configuration and switch drift

- **Statement.** Switches, setting keys and size macros are defined in several places (project files, headers, source; T-045); editors write keys the parser does not know; switches are never defined or defined only locally; configurations are identical where they should differ or differ silently. [verified rows]
- **Mechanism.** H-173 (editor keys unknown to the parser), H-172 (`#ifdef XML` never defined in `JukConsole/VConTul.cpp`), H-177 (`SEURATUL` defined only in `Hk/HkTulostus.cpp`), H-176 (roll-up), T-017, T-024, T-050, T-051, T-052.
- **Directly caused (4; P1/P2 0, high 0):** H-172(P3) H-173(P3) H-176(P5) H-177(P5)
- **Made worse:** H-020 H-036 H-041(P3,high) H-083(P5) H-158(P5) H-175(P4)
- **T rows stating it:** T-045, T-050, T-051, T-017, T-024, T-052.
- **Bounded fix enough?** Yes per key; removing the cause: one table of keys and switches (`rekisteri/asetusavaimet.csv`, `rekisteri/valitsimet.csv` are the spec).
- **Must not break:** existing configuration files (accept both spellings); `AIKAKORJAUS` units (HK `SEK/100`, relay `SEK`).

### JS-15 Two toolchains and RTLs without a portability or encoding policy

- **Statement.** The same source is compiled by MSVC and bcc32 with different RTL semantics and code-page handling, without compile-time checks or a shim. [verified rows; `kaantajaerot.md`]
- **Mechanism.** 32-bit `time_t` in bcc32 (H-080); MSVC invalid-parameter abort for `ccs=UTF-16BE` with no handler (H-078, T-020); BOM-less UTF-8 read with the ANSI code page (H-005, T-027); U+FFFD bytes in source (H-004); `swprintf` termination (T-026); `-a1` pragma leak (T-016, T-023); float version number (T-025).
- **Directly caused (4; P1/P2 0, high 0):** H-004 H-005 H-078(P4) H-080(P5)
- **Made worse:** H-059(P3) H-063(P4) H-079(P4)
- **T rows stating it:** T-004, T-016, T-020 (handler part), T-023, T-025, T-026, T-027.
- **Bounded fix enough?** Per site yes; the encoding part needs an owner decision (Q7; CLAUDE.md forbids encoding changes without it).
- **Must not break:** file encodings and BOMs (`merkistot.md`), struct layout on disk and wire (`kaantajaerot.md` 4.1), the version identifier written by HK console vs GUI (T-025).

### Isolated one-off defects (no shared root)

H-001(P3,high) H-002(P4,high) H-013 H-020 H-037 H-043 H-046 H-062(P3) H-064(P4) H-068(P5) H-075(P4) H-086(P5) H-111(P3,high) H-116(P3) H-118(P4) H-119(P4) H-120(P4) H-123(P4) H-126(P5) H-161(P3) H-168(P4) H-169(P3) H-175(P4)

These are local coding errors (wrong `sizeof` variable H-001, `%s` into `int` H-002, loop variable H-013, missing `else` H-111, operator precedence H-064, self-comparison H-168, file-mode misuse H-043 and H-086, uninitialised locals H-062 and H-068, pointer set only for two printer types H-075, handle-1 index H-037, cursor gets the bib H-118, check mismatch H-123, menu vs variable H-175, duplicate symbol H-020), single design choices that need an owner decision (H-046, H-119, H-120, H-169), or roll-ups (H-126). The only common factor is the enabling cause JS-13 (most are compiler-warning classes). H-116 is isolated as a defect, but its helper is shared (section 3.1). H-161 (SQL built by string concatenation) is one module's idiom, not a cross-batch cause.

## 3. Cross-batch causes the batch reports could not see (task 2)

| # | Hypothesis | Verdict | Evidence |
|---|---|---|---|
| N1 | Same copy-pasted function in `Hk/` and `Juk/` with a fix on one side only | **confirmed**; extends T-034 from GUI forms to the core | C4: `tark_kilp` reads before the lock in HK (`Hk/HkCom32.cpp:260` vs `:326`), inside it in relay (`Juk/VCom.cpp:579-580`); C9: `autofile` exists in both forks with different defects (H-149 relay, H-150/H-151 HK). [verified] -> JS-12 |
| N2 | Modal error dialog from worker threads is the common origin of lock and persistence findings | **confirmed**: one helper | C1: `writeerror_w` (`cbTp/cbTpApu.cpp:467-490`) is the mechanism of H-009 (lock across `SendMessage`), H-019/H-144 (global `perr_ptrs2` without lock), H-021 (dialog at 25/38 under `outb`/`tall`), H-153 ("Poistu ohjelmasta" while the worker holds `outb`), H-167 (shutdown waits for an HTTP thread stuck in it), H-015 (dialog under lock on a `KILP.DAT` write error), T-032, T-033 and the dialog half of T-040. [verified code; links from rows] -> JS-4/JS-11 bridge |
| N3 | Fixed compile-time array sizes vs run-time counts cause both file and network overflows | **partly confirmed** | True for size findings without hostile input (H-041, H-042, H-025, H-076, H-122, H-129, H-171) and per-build constants (C6) -> JS-3. **Rejected** for the P1 network rows H-032, H-071, H-072: there the buffer size is known and the input index or length is simply not checked (JS-1). [verified rows] |
| N4 | One global flag `vaiheenvaihto` used as a thread-stop signal | **confirmed** | C3: exit-on-flag loops in five threads, skip-on-flag in others; explains H-014, H-031, H-094, H-097 together. [verified] -> JS-7 |
| N5 | Pointer aliasing by struct copy | **confirmed, narrow** | Only H-152 (`autotulostusparam = lsttulparam` copies `lstf`) is true aliasing; H-017 is a stale value copy (JS-5), not aliasing. Folded into JS-8. [verified rows] |
| N6 | Absence of any ownership convention | **confirmed** | C7: `closeprfile` by value, `etls` freed by its consumer, sibling handle not nulled; rows from C-2 (H-028), C-6 (H-094), C-8/C-6 (H-105), C-9 (H-149..H-155). [verified] -> JS-8 |
| N7 | Absence of tests as the cause of unfixed one-sided fixes | **not confirmed** for one-sided fixes (no history read, Q77); **confirmed as enabling cause** for coding-error classes (warnings hit 16/23 C-4 defects directly; tests not in CI) | `muistiturvallisuus.md:569`; H-006; T-021. -> JS-13 |
| N8 | (own) Exception-unsafe core called from throwing GUI/DBX code | **confirmed** | C5: zero `try` in `Hk/`, `Juk/`, `Tp/`, `dbboxm/`; no `__finally` anywhere; H-115, H-125, H-160, H-162, H-163. -> JS-2 |
| N9 | (own) Last-writer-wins also for local edits, not only replication | **confirmed** | H-017 (form copy rewritten whole) and H-007 (stale merge) share the mechanism of H-016. -> JS-5 |
| N10 | (own) T-038 and T-039 are one design assumption (start-up-only construction) | **inferred** | C8 shows the separate re-run branch of `teejarj`; `globaali-tila.md` 5 states restart is the only repair. -> JS-6 |
| N11 | (own) "Widen `tall_CS`" as the race fix | **rejected** | Races are on structures covered by no lock (`globaali-tila.md` 6) and `tall` is already the hub of the JS-4 cycles; widening it adds blocking under the lock. [inferred from reports] |

### 3.1 Unregistered candidate found during check C1/C2 (no id given)

`errorMsgHandler` (`cbTp/cbTpApu.cpp:445-463`) shows `TFormSelect3` modally and treats `retval == 2` as "Poistu ohjelmasta": it sets `suljesuoraan = 1` and calls `Close()`. `TFormSelect3` sets `ModalResult` only from its buttons (`cbTp/UnitSelect3.cpp:38-60`); closing the window with X gives VCL `mrCancel`, which is 2 (the H-116 mechanism). So closing **any** GUI error dialog with X, including "OK only" dialogs (`vainok`), would end the program, possibly while a worker holds a lock and is mid-write (H-153 context). Code read [verified]; the `mrCancel` value and the end-to-end effect are [inferred] (same inference as H-116, not run). The main session should decide whether this is an extension of H-116 or H-153 or a new row. It belongs to the `writeerror_w` group (section 4.1, FT-1).

## 4. Dependency chains for fix order (task 3)

### 4.1 Must be fixed together (one change or one PR series)

| Group | Members | Why together | Draft packages today |
|---|---|---|---|
| FT-1 `writeerror_w` | H-009, H-019, H-153, H-167, dialog parts of H-015 and H-021, `perr_ptrs2` item of H-144, candidate 3.1, H-116 (if `TFormSelect3` results change) | One helper (C1). Making it non-blocking without per-call message storage overwrites messages (H-019); changing the X/button-2 meaning changes every error dialog; callers that act on the answer ("Ei" -> `lopetus`, H-153) need the new semantics at the same time. [verified C1] | KK-C13-5, KK-C13-6, KK-C9-2, KK-C10-2, KK-C13-14 |
| FT-2 bib index | H-128, H-129, H-102, H-133 (index part), H-013 | Same rows `Hk/HkIx.cpp:1573-1613`, `:205-226`, `tputilv2/dbindex.cpp:51-135`; H-128 deletions lower the H-129 threshold (H-129 row), so fixing one changes the reproduction of the other. [verified C8, rows] | KK-C13-3, KK-C13-16 |
| FT-3 stage-change emit and XML result | H-134, H-105, H-174, H-141 | H-105 is reached through the H-134 branch (relation `seuraus`); H-134 and H-174 handle the same `lueEventXml` result (`Hk/HkXml.cpp:1824-1837`). [verified rows; H-174 link inferred] | KK-C13-3, KK-C13-12 |
| FT-4 relay `addfrec` | H-040, H-148, `addfrec` part of H-047 | Same lines `Juk/vdat.cpp:178-192`: double lock, unlocked close/reopen, unchecked reopen. [verified rows] | KK-C13-4, KK-C9-2, KK-C13-6 |
| FT-5 shooting penalties | H-008, H-060 | Same call site `Hk/HkCom32.cpp:647`, `Tp/TpLaitteet.cpp:1670`, `Hk/HkSak.cpp`; H-008 is triggered by the malformed message H-060 accepts. [verified rows] | KK-C13-4, KK-C13-1 |
| FT-6 `COMFILE.DAT` layout | H-025, H-048, H-050; readers H-044, H-030 | `combufsize` defines record size and stride; correcting the relay header (H-025) changes what the resend (H-048), viewer (H-050) and resend tool (H-030) read. Needs an owner decision on old queue files. [inferred from rows] | KK-C13-1, KK-C13-9, KK-C13-12, KK-C9-2, KK-C13-10 |
| FT-7 timing form pair | H-117, H-121, H-111 | Opposite one-sided fixes of the same `tall_rivi` call (`cbHk/UnitAjanotto.cpp:1226` vs `ViestiWin/UnitAjanotto.cpp:1221`); H-111 is the shared A/L parse in the same handler. [verified rows] | KK-C13-14, KK-C13-15 |
| FT-8 SQL | H-160, H-161, H-162, H-014, then H-169 | H-161 statements trigger H-160; H-160/H-161 make the mirror stale, which makes H-169 destructive; H-162 needs the same exception boundary. [verified `riippuvuudet.md` 19.3] | KK-C10-1 |
| FT-9 delete path | H-109, H-140, H-139 | `deltall`/`deleterec` path (`Hk/HkDat.cpp:597-621`, `:740-770`; `Juk/vdat.cpp:907-939`): status check, `emitjarr` clear and counters change in the same function. [inferred from rows] | KK-C13-14, KK-C13-16 |
| FT-10 duplicate-card rule | H-034, H-141, H-168, ACK part of H-145 | One duplicate-detection rule (`Hk/HkEmit.cpp:1627-1666`, `Hk/HkIV.cpp:940-952`); fixing H-168 changes which resends H-034 lets through; ACK meaning is an owner question (Q86, Q100). [inferred] | KK-C13-9, KK-C13-3, KK-C10-2, KK-C9-2 |
| FT-11 HK `tark_kilp` | H-007, H-022, H-023, H-033 | Same function `Hk/HkCom32.cpp:242-334` (C4). [verified] | KK-C13-6, KK-C13-10, KK-C13-1 |
| FT-12 receive class handling | H-026, H-032, H-071 | Same lines `Tp/TpComY.cpp:1001`, `:2399`. [verified relations] | KK-C13-1 |
| FT-13 `TextFl::Open` | H-078, H-079, H-086 | One function `tputilv2/TextFileClass.cpp:322-359`. [verified relations] | KK-C13-12 |
| FT-14 autofile | H-149, H-150, H-151, H-152, H-158 | Same functions (C9) and the same `lstf`/`aflst` fields. [verified C9, rows] | KK-C9-1, KK-C9-3, KK-C12-1 |

### 4.2 Ordering: unblocks, unsafe alone, interactions

| # | Relation | Consequence for order | Label |
|---|---|---|---|
| O1 | A scoped lock guard (JS-2) before the lock-leak site fixes | Without it each fix is a one-off and new early returns re-leak; introduce the helper in the first KK-C13-4 change. | [inferred] |
| O2 | Lock-leak fixes change lock-order reproduction | Today relay `print_CS` stays held by the main thread after the first HTML team (H-087), so other threads block instead of entering the H-092/H-103 cycles; releasing it makes those cycles reachable. Fix KK-C13-4 and re-check KK-C13-5 in the same series. | [inferred] |
| O3 | `_set_invalid_parameter_handler` (T-020) changes MSVC behaviour | H-059, H-063, H-078 and H-105 abort today on MSVC; with a handler they continue with bad state, and the C-11 harness XFAIL expectations change. Add the handler only after those fixes or update the harness in the same change. | [inferred] |
| O4 | H-031 before H-094 lowers exposure only in the console | H-094 still occurs at every GUI stage change; both are needed. | [verified rows] |
| O5 | Changing exit-on-flag to pause (H-097, H-014) interacts with H-094 | `pros_bdg_t` exiting is what frees `etls`; a paused thread no longer frees it, which removes the free but requires that the producer never writes during `vaihdavaihe`. Decide pause vs exit once for all JS-7 threads. | [verified C3; consequence inferred] |
| O6 | H-161 before or with H-160 | Parameterised statements remove the most common trigger of H-160 but not server outages; H-160 is still required. H-169 after both. | [verified `riippuvuudet.md` 19.3] |
| O7 | H-165 before verifying H-163 | Missing DLLs in the release ZIP make H-163 the normal case; fix packaging first or the start-up fix cannot be tested from a release. | [verified rows] |
| O8 | Bounded checks before protocol versioning (JS-5) | H-007, H-022, H-023 keep the wire format; versioning (H-016) would change the meaning of any resend or duplicate logic built for H-030, H-034, H-039, so do not build that logic first. | [inferred] |
| O9 | H-029 authentication must not gate H-071/H-072/H-032 | Bounds checks are independent of the protocol decision; authentication only reduces reach. | [verified `muistiturvallisuus.md` 15] |
| O10 | JS-13 CI before KK-C13-20, KK-C13-21, KK-C13-22 | Refactoring forks, globals or stage state without the harness in CI repeats JS-12. | [inferred] |
| O11 | KK-C9-2 return values make KK-C13-5 easier | Once write errors are returned, the dialog can be shown after locks are released instead of inside `writeerror_w` under the lock. The bounded FT-1 change can go first; propagation follows. | [inferred] |
| O12 | H-041 via heap, not a larger stack buffer | Small fixed thread stacks (C7, `_beginthread(..., 40960, ...)`) mean a bigger local buffer can overflow the stack in a worker. | [inferred] |

### 4.3 Bounded immediate fix and later architectural fix, both needed

| Immediate (bounded, keeps formats) | Later (removes the cause) | Root |
|---|---|---|
| H-071, H-072, H-032, H-061 bounds and length checks | validated reader layer (T-022, KK-C13-19); authentication decision (H-029, KK-C10-4) | JS-1 |
| H-008, H-024, H-040, H-087, H-090, H-110 release on every path | scoped guard + exception boundaries (T-036 generalised) | JS-2 |
| H-041, H-042, H-171, H-073 size checks | sizes from run-time parameters; bounded string helper (T-026) | JS-3 |
| FT-1 `writeerror_w`: no exit from a worker under a lock, per-call buffer | non-blocking error channel and lock hierarchy (T-032, T-033, T-040) | JS-4 |
| H-007 read inside the lock, H-022/H-023 checks | version/origin in replication (H-016) | JS-5 |
| H-129 reset `nrec`, H-128 reachable delete | central reset/rebuild (T-038, T-039; KK-C13-20) | JS-6 |
| H-097/H-014 restart or pause, H-031 flag reset | thread registry with join (T-030) | JS-7 |
| H-149, H-151, H-152 null after close | `closeprfile(PRFILE **)` and one owner (T-042) | JS-8 |
| H-159 condition, H-164 reopen | device connection layer (T-048, KK-C10-2 study) | JS-9 |
| H-107 NULL checks, H-018 sites | queued UI marshalling (T-033) | JS-10 |
| H-145 no ACK before persistence, H-146, H-147 | error policy, atomic writes (T-040, T-041), checksum (T-019) | JS-11 |
| H-117, H-121, H-069 per-fork fixes with J7-7 check | fork convergence (T-034, KK-C13-22) | JS-12 |
| H-160 catch in the writer thread | optional-mirror principle (KK-C10-1 text) | JS-2 |

## 5. Critique of the draft packages (task 4)

Verdict scale: **keep** (one cause, one code area), **adjust** (keep with moves), **split**, **dissolve by moves** (id kept as alias, members moved). Root-cause column uses the primary JS of the members (appendix). "R" lists members that are P1/P2/high.

| Package | n | Causes of members | Code area | Verdict | Issues |
|---|---|---|---|---|---|
| KK-C9-1 PRFILE lifecycle | 5 | JS-8 | `tputilv2/Wrttext.cpp`, both `autofile` | **adjust** | Coherent. Missing H-150 (same HK `autofile`, `aflst` leak, now in KK-C9-3) and H-158 (same relay `autofile` and `lstf` as H-149, now in KK-C12-1). R: H-149. |
| KK-C9-2 write-error propagation | 12 | JS-11 (+ H-153 JS-4) | `dbboxm`, emit, AJAT, result files, COMFILE | **adjust** | Cause coherent, code area wide: review as series per file (KILP.DAT H-015/H-148; emit H-145/H-053; AJAT H-146; results H-147/H-156; COMFILE H-044/H-054). H-153 is the `writeerror_w` helper (FT-1). H-148 must go with H-040 (FT-4). T-040/T-041 are redesigns: keep as guiding text, do not gate the H fixes. No P1/P2/high member. |
| KK-C9-3 resources in repeated operations | 4 | JS-8, T-020 | unrelated files | **dissolve by moves** | H-150 (HK autofile) and H-155 (`com/WinUDP.cpp` `openportTCP`, same function as H-027) share nothing in code. What remains coherent is H-063 + T-020 (`fclose(NULL)`, unchecked allocations). R: H-150. |
| KK-C10-1 SQL integration | 8 | JS-2, JS-7, ISO | `cbHk/UnitMySQL.cpp`, `ViestiWin/UnitMySQL.cpp`, `UnitExport`, `UnitImport` | **keep** | One code area, one PR series (FT-8). H-014's stage-change half depends on the JS-7 pause/exit decision (O5). H-169 needs the owner principle; it must not block H-160 (P2). R: H-014, H-160. |
| KK-C10-2 device connections | 9 | JS-9, JS-10, JS-4, ISO | `Tp/TpLaitteet.cpp`, `Tp/ZebraReader.cpp`, `com/` | **adjust** | H-167 is shutdown/`writeerror_w` (JS-7/JS-4), not a device connection; H-168 is the duplicate-card rule (FT-10), not connection management. R: H-159. |
| KK-C10-3 deployment | 8 | JS-13, JS-14 | project files, release scripts | **adjust** | T-017 (JukLuenta identical defines) is configuration drift (KK-C12-2, `sama_vika` with T-051). T-047 is a licence decision; must not block H-165. |
| KK-C10-4 external link security | 2 | JS-1 | protocol/decisions | **keep** | Decision package; correct that H-071/H-072 are not in it (O9). |
| KK-C12-1 XML/HTML/auto files | 2 | JS-8, JS-14 | relay `autofile`, relay console | **dissolve by moves** | Two different mechanisms; H-158 belongs with H-149 (FT-14), H-172 is a T-050 child like H-177. Too small to keep an own id. |
| KK-C12-2 settings | 8 | JS-1, JS-3, JS-14 | `Juk/VInit.cpp`, `Hk/HkInit.cpp`, `UnitCfg` | **adjust** | Run bounded fixes first (H-171 P2 high, H-036, H-052, H-176 `fldn`) and decision items after (H-173 key spelling, T-014, T-024, T-051). Add T-017. R: H-171. |
| KK-C12-3 switch hygiene | 14 | JS-14 | many | **adjust** | All owner-decision items (Q111), fine as one batch. T-008 is not hygiene but console/GUI divergence (FORK_DIV; H-081 is its instance) and belongs to KK-C13-22. T-028 (`#ifdef CONSOLE` typo) is a bounded bug fix that needs no deletion decision; ship it separately. Add H-172, H-020. |
| KK-C13-1 network receive memory safety | 9 | JS-1, JS-3 | `Tp/TpComY.cpp`, `Hk/HkCom32.cpp`, `Juk/VCom.cpp` | **adjust** | All four P1 rows: must be the first series. H-025 is queue sizing (FT-6), H-060 is the shooting device path (FT-5), H-073 is a log string builder (`Hk/HkEmit.cpp:6165-6185`), none in the receive code. R: H-026, H-032, H-061, H-071, H-072 (+ moved H-025, H-060, H-073). |
| KK-C13-2 device input parsers | 4 | JS-1, JS-3 | `Tp/TpLaitteet.cpp` | **adjust** | H-077 writes an output buffer for a display board (`ViestiWin/UnitGAZ.cpp`, `cbHk/UnitTaulu.cpp`), not a device input parser. Add H-060 (+ FT-5 coordination with H-008). R: H-055, H-056, H-070. |
| KK-C13-3 stage change and start-up state | 12 | JS-6, JS-7, JS-8 | `Hk/HkIx.cpp`, `Hk/HkInit.cpp`, `Hk/HkIV.cpp`, `Hk/HkEmit.cpp`, `cbHk` threads | **split** | Too big and mixes three mechanisms and P2 with owner-decision items. Thread rows H-031, H-094, H-097 are JS-7 (`vaiheenvaihto` semantics, O5) and belong with thread lifecycle. Add H-128 and H-013 (FT-2), H-174 (FT-3). Proposed series: (a) bib index H-128, H-129, H-102, H-013; (b) emit memory on stage change H-105, H-134, H-174, H-141; (c) file names and re-run H-131, H-132, H-133 (decisions Q67, H-132 naming); (d) roll-up H-143. R: H-129; H-031 moves out, H-128 moves in. |
| KK-C13-4 lock release on error/return paths | 11 | JS-2 | many (HK core, relay core, GUI) | **adjust** | One idiom, many sites: fine if the first change adds the guard helper (O1) and each site is its own small PR. Add H-115 (T-036 lists it). Coordinate FT-4 (H-040 with H-148, H-047) and FT-5 (H-008 with H-060); re-check H-092/H-103 after H-087 (O2). R: H-008, H-024, H-040, H-087, H-090, H-110. |
| KK-C13-5 lock order and blocking under lock | 7 | JS-4 | `cbTp/cbTpApu.cpp`, `com/WinUDP.cpp`, `Hk/HkTulostus.cpp`, `Juk/VEmit.cpp` | **adjust** | Should own FT-1 (`writeerror_w`): add H-019 and H-153 (and candidate 3.1). T-031 is a redesign (per-socket locks); H-093 has a bounded fix (no logging under `tcpip_CS`) that must not wait for it. R: H-009, H-091, H-093. |
| KK-C13-6 synchronisation of saving and shared state | 11 | JS-10, JS-5 | `Hk/HkDat.cpp`, `Hk/HkEmit.cpp`, `Hk/HkIV.cpp`, `dbboxm`, `cbHk` | **adjust** | Too broad a theme. H-007 is the `tark_kilp` function (FT-11, JS-5); H-101 is VCL from a worker (H-018 family, `sama_vika`); H-019 is FT-1. H-136 (`bdg_t`) shares code with H-094 (`pros_bdg_t`) and should be fixed in the same change. H-010 (about 35 unlocked call sites) is a lock-policy decision rather than a bounded fix. R: H-007 (moves). |
| KK-C13-7 VCL from worker threads | 4 | JS-10 | `cbHk`, `ViestiWin`, `cbTp` | **adjust** | Add H-101. H-107 (P2) has a bounded fix (NULL checks) that must not wait for the T-033 marshalling redesign. R: H-107. |
| KK-C13-8 thread lifecycle and shutdown | 7 | JS-7, JS-4, JS-8 | `Hk/HkInit.cpp`, `Juk/vApu.cpp`, `Tp/TpComY.cpp`, `cbTp` | **adjust** | H-096 is the accept loop in `com/WinUDP.cpp:645-684` (same lines as H-028) and belongs with sockets. Add H-031, H-094, H-097 (JS-7, from KK-C13-3) and H-167 (shutdown wait, from KK-C10-2). H-157 is a roll-up whose items span JS-8 and JS-11. |
| KK-C13-9 connections and queues | 7 | JS-9, JS-5, JS-3, JS-4 | `com/WinUDP.cpp`, `Tp/TpComY.cpp` | **adjust, then split** | H-034 is replication idempotency (FT-10, JS-5). Add H-025, H-050 (FT-6), H-096 (with H-028), H-155 (with H-027). Then two series: sockets (H-027, H-028, H-037, H-096, H-155) and queue layout (H-021, H-025, H-035, H-048, H-050; reader H-044 in KK-C9-2). R: H-027 (+ H-025 moved). |
| KK-C13-10 replication consistency | 6 | JS-5, JS-12 | `Hk/HkCom32.cpp`, `Tp/TpComY.cpp`, `cbTp/UnitUusinta.cpp`, `cbHk/UnitEmit.cpp` | **split** | Mixes bounded checks with a wire-format decision; per the draft's own design rule the decision must not block the fixes. (a) bounded, no format change: H-007 (from KK-C13-6), H-022, H-023, H-033 (FT-11); (b) protocol/owner decision: H-016, H-030, H-039, plus H-034 and H-168 (FT-10). H-017 is a GUI form fix (relay already compares inside the lock) and fits KK-C13-14. R: H-022, H-023. |
| KK-C13-11 record format and size limits | 5 | JS-3, ISO, T | `Hk/HkDef.h`, `Hk/HkDat.cpp`, `Juk/VXml.cpp`, `Juk/vdat.cpp` | **adjust** | T-019 (magic/version/checksum for `KILP.DAT`) is a file-format redesign that must not gate H-041 (confirmed by run) and H-042; T-035 (relay record without constructor) is the root of H-109/H-112 in KK-C13-14. H-046 needs Q29. R: H-041, H-042. |
| KK-C13-12 local file and import parsers | 11 | JS-1, JS-3, JS-15, ISO | `tputilv2`, `Hk/HkInitVanha.cpp`, `Juk/vSarjat.cpp`, readers | **adjust** | H-050 is the COMFILE viewer (FT-6); H-174 is the `lueEventXml` result (FT-3). `TextFl::Open` trio (FT-13) is one PR. R: H-002. |
| KK-C13-13 output path memory defects | 9 | JS-3, ISO, JS-13 | `Juk/VTulostus.cpp`, `Hk/HkAjat.cpp`, `tputilv2/Wrttext.cpp`, consoles | **adjust** | Coherent enough. Add H-073 and H-077 (same fixed-buffer formatting idiom). Should use the bounded string helper if one is introduced (KK-C13-19). R: H-001. |
| KK-C13-14 GUI actions that corrupt data | 9 | JS-1, JS-2, JS-12, ISO | `cbHk`/`ViestiWin` forms, `cbTp/UnitSelect3.cpp` | **adjust** | H-115 is a T-036 lock-leak child (KK-C13-4). Add T-035 (its children H-109, H-112 are here), H-017 (form-copy overwrite), H-121 (FT-7 with H-117). H-109's fix is mostly in the core `deltall` (FT-9 with H-140). H-116 changes a shared dialog used by `writeerror_w` (FT-1). R: H-109, H-111. |
| KK-C13-15 GUI input limits and small logic | 16 | JS-1, JS-12, JS-3, ISO | eight different forms | **split** | Too big and heterogeneous to review as one. (a) numbering and draw forms (`UnitArvonta` pair): H-069, H-085, H-118, H-122, H-123, H-124; (b) field bounds in small forms: H-082, H-083, H-084; (c) fitness-orienteering emit behaviour needing Q75: H-119, H-120, H-175; roll-ups H-126, H-127 item by item. H-121 to KK-C13-14 (FT-7); H-020 (duplicate `add_sakko`, needs a link-map check) to KK-C12-3. No P1/P2/high member. |
| KK-C13-16 derived-structure integrity | 9 | JS-6, JS-3, ISO | `Hk/HkIx.cpp`, `Hk/HkDat.cpp`, `Juk/vdat.cpp`, `Juk/VIx.cpp` | **adjust** | H-128 belongs with H-129 (FT-2) and H-013 is in `teejarj` (FT-2). T-039 is the architectural half of JS-6 and should sit with T-038 (KK-C13-20). Remaining H-066, H-130, H-135, H-139, H-140, H-142 are bounded; H-140 goes with H-109 (FT-9). R: H-128 (moves out). |
| KK-C13-17 encodings and compiler differences | 11 | JS-15, JS-13, JS-12 | build settings, many sources | **adjust** | H-081 is a console/GUI fork divergence (T-008 instance), not encoding. T-021 is the warning-hygiene half of JS-13 and makes KK-C13-18 worth an id. T-026 is the RTL half of the bounded string helper (JS-3) and belongs in KK-C13-19. Encoding items need Q7. |
| KK-C13-18 test and CI basis | 1 | JS-13 | `Tests/`, workflows | **too small; adjust** | Add T-021 (warning baseline) so the package covers JS-13; it is a precondition for KK-C13-20..22 (O10). |
| KK-C13-19 shared safe helper layer (input validation) | 1 | JS-1 | new helper | **adjust** | Single-T package is fine by the draft's design. Widen to "bounded parse and format helpers": add T-026 (JS-3 string part). JS-3 has no T row of its own; record that it is served here. |
| KK-C13-20 central stage-state management | 1 | JS-6 | `Hk/HkIx.cpp`, `Hk/HkInit.cpp` | **adjust** | Add T-039; the reset table and the derived-structure rebuild are one design (JS-6). |
| KK-C13-21 core/GUI separation and globals | 3 | JS-10, layering | headers | **keep** | Depends on KK-C13-18 (O10). T-033 (marshalling) could later join it; not needed now. |
| KK-C13-22 HK/relay fork convergence | 1 | JS-12 | forks | **adjust** | Scope must include core forks (`Hk/` vs `Juk/`, C4, C9), not only the GUI forms named by T-034. Add T-008 and H-081 (console/GUI divergence). |
| ULKOPUOLELLA | 1 | - | - | **keep** | T-010 already fixed. |

### 5.1 Root causes without a package or with only partial cover

| Root | Status | Proposal |
|---|---|---|
| JS-3 capacity vs run-time size | no T row and no architectural package; members spread over KK-C13-1/-11/-12/-13, KK-C12-2 | serve it through KK-C13-19 (bounded helpers, T-026) and record the rule "buffer size from run-time record size" in KK-C13-11 |
| JS-5 versioning of replication and files | H-016 in KK-C13-10; T-019 in KK-C13-11; COMFILE layout (FT-6) spread | the decision half of the KK-C13-10 split should also carry T-019 (format versioning) so that wire and file format changes are decided together |
| JS-2 guard helper | implicit only (T-036 is GUI-only) | first change of KK-C13-4 introduces the helper; mention it in the package text |
| JS-8 ownership beyond PRFILE | only T-042 | KK-C9-1 text should state the general rule (owner field by reference) and KK-C13-8/-9 apply it to queues and sockets |
| JS-13 verification gap | KK-C13-18 with one row | add T-021 (above) |

### 5.2 T rows inside H-centred packages

The draft's rule keeps refactorings out of fix packages, yet 14 root-cause T rows sit in H-centred packages (not counting the hygiene and configuration T rows of KK-C10-3, KK-C12-2, KK-C12-3, KK-C13-17). Pattern-level T rows describe how each site is fixed and are fine there: T-020 (KK-C9-3), T-035, T-036 (KK-C13-4), T-037 (KK-C13-14), T-042 (KK-C9-1), T-032 (KK-C13-5). Redesign-level T rows must be marked "later, does not gate the H members" or moved: T-019 (KK-C13-11), T-030 (KK-C13-8), T-031 (KK-C13-5), T-033 (KK-C13-7), T-039 (KK-C13-16, move to KK-C13-20), T-040 and T-041 (KK-C9-2), T-048 (KK-C10-2). [inferred from the row texts]

## 6. Change requests to the draft packages

No new ids; "split" means the main session assigns ids or keeps the parts as ordered series under one id.

1. **Move H-150** KK-C9-3 -> KK-C9-1 (same HK `autofile` as H-151; FT-14).
2. **Move H-155** KK-C9-3 -> KK-C13-9 (same `openportTCP` failure path as H-027, `com/WinUDP.cpp:266-293`, `:812-817`). KK-C9-3 keeps H-063 + T-020 (unchecked allocation, `fclose(NULL)`), which is coherent.
3. **Dissolve KK-C12-1 by moves:** H-158 -> KK-C9-1 (same relay `autofile` and `lstf` as H-149), H-172 -> KK-C12-3 (T-050 child like H-177); keep KK-C12-1 as an alias.
4. **Move H-167** KK-C10-2 -> KK-C13-8 (shutdown wait loop; coordinate with FT-1).
5. **Move H-168** KK-C10-2 -> decision part of KK-C13-10 (duplicate-card rule, FT-10).
6. **Move T-017** KK-C10-3 -> KK-C12-2 (configuration drift, same as T-051).
7. **Move T-008** KK-C12-3 -> KK-C13-22 and **H-081** KK-C13-17 -> KK-C13-22 (console/GUI divergence is fork divergence).
8. **Move H-025** KK-C13-1 -> KK-C13-9 and **H-050** KK-C13-12 -> KK-C13-9 (FT-6 `COMFILE.DAT` layout with H-048).
9. **Move H-060** KK-C13-1 -> KK-C13-2 and fix it in one change with H-008 (FT-5).
10. **Move H-073** KK-C13-1 -> KK-C13-13 and **H-077** KK-C13-2 -> KK-C13-13 (fixed-buffer output formatting).
11. **Move H-031, H-094, H-097** KK-C13-3 -> KK-C13-8 (JS-7 `vaiheenvaihto` semantics; decide pause vs exit once, O5). Fix H-136 together with H-094.
12. **Move H-128 and H-013** KK-C13-16 -> KK-C13-3, **H-174** KK-C13-12 -> KK-C13-3 (FT-2, FT-3).
13. **Split KK-C13-3** into ordered series: (a) bib index H-128, H-129, H-102, H-013 first (P2); (b) H-105, H-134, H-174, H-141; (c) H-131, H-132, H-133 after Q67/H-132 decisions; (d) H-143.
14. **Move H-115** KK-C13-14 -> KK-C13-4 (T-036 child). State in KK-C13-4 that its first change introduces the scope-guard helper (O1).
15. **Move H-019** KK-C13-6 -> KK-C13-5 and **H-153** KK-C9-2 -> KK-C13-5, so KK-C13-5 owns the `writeerror_w` change (FT-1); KK-C9-2 keeps return-value propagation.
16. **Move H-101** KK-C13-6 -> KK-C13-7 (VCL handler on a worker thread, `sama_vika` with H-018).
17. **Split KK-C13-10:** (a) bounded receive-apply checks with no wire change: H-007 (from KK-C13-6), H-033 (from KK-C13-1), H-022, H-023 (FT-11); (b) protocol/owner decision: H-016, H-030, H-039, H-034 (from KK-C13-9), H-168, T-019 (from KK-C13-11).
18. **Move H-017** KK-C13-10 -> KK-C13-14 (GUI form copy; relay already compares inside the lock).
19. **Move T-035** KK-C13-11 -> KK-C13-14 (its children H-109, H-112 are there).
20. **Move H-121** KK-C13-15 -> KK-C13-14 (FT-7 with H-117 and H-111).
21. **Move H-020** KK-C13-15 -> KK-C12-3 (duplicate symbol; needs a link-map check, not a GUI logic fix).
22. **Split KK-C13-15** into (a) numbering/draw forms H-069, H-085, H-118, H-122, H-123, H-124; (b) field bounds H-082, H-083, H-084; (c) Q75-dependent H-119, H-120, H-175; roll-ups H-126, H-127 handled item by item in (a)/(b) or KK-C13-14.
23. **Move T-039** KK-C13-16 -> KK-C13-20 (JS-6 architectural half).
24. **Move T-021** KK-C13-17 -> KK-C13-18 and **T-026** KK-C13-17 -> KK-C13-19 (verification baseline; bounded string helper).
25. **Move H-096** KK-C13-8 -> KK-C13-9 and run KK-C13-9 as two series: sockets (H-027, H-028, H-037, H-096, H-155) and queue layout (H-021, H-025, H-035, H-048, H-050).
26. **Cross-package fix-together marks (no move):** FT-4 H-040 (KK-C13-4) + H-148 (KK-C9-2) + H-047 (KK-C13-6); FT-9 H-109 (KK-C13-14) + H-140, H-139 (KK-C13-16); FT-8 H-014 (KK-C10-1) waits for the O5 decision in KK-C13-8; H-044 (KK-C9-2) follows FT-6.
27. **Package texts:** KK-C13-22 scope includes core forks (C4, C9); redesign T rows listed in 5.2 are marked 'does not gate the H members'; T-028 in KK-C12-3 is shipped as a bounded fix without waiting for Q111.

Net effect [inferred]: KK-C13-1 becomes the pure P1 receive series (H-026, H-032, H-061, H-071, H-072); KK-C13-5 owns the `writeerror_w` change; KK-C13-9 owns sockets and the `COMFILE.DAT` layout; KK-C13-3 shrinks to state reset with the bib index first; the bounded part of KK-C13-10 can ship without any protocol decision.

## 7. Fixes whose correct behaviour is unclear (task 5)

| Rows | What is unknown | Needed | Source |
|---|---|---|---|
| H-116, candidate 3.1 | Should closing a `TFormSelect3` dialog (X, Alt+F4) mean cancel, button 1, or "exit"? Today it is button 2, which is "Poistu ohjelmasta" in error dialogs | owner decision; GUI run to confirm `mrCancel` | C1, C2; H-116 row |
| FT-1 (H-009, H-153, H-167) | May a worker-thread error offer program exit at all, and what does the console equivalent do | owner decision | T-040, T-032; `virheenkasittely.md` 12.7 |
| H-014, H-097, H-031 (O5) | During and after a stage change, should the SQL writer, scroll, open-card and SQL emit threads pause, stop and restart, or stop for good; what a refused change must leave behind | owner decision | C3; `saikeet-ja-lukot.md` 5 |
| H-022, H-023 | Which stage `VAIN_TULOST` should apply to; whether a bib mismatch on receive must be rejected or merged | owner decision plus two-station run | rows; Q3, Q18 |
| H-016, H-030, H-039 | Version/origin in packets, purpose of the resend tool, mesh forwarding rules | owner decision (wire format) | `tiedonsiirto.md`; Q3, Q18 |
| H-034, H-141, H-168, H-145 | Duplicate-card rule and whether an ACK means "stored" | owner decision; reader behaviour run | Q86, Q100 |
| H-025, H-048, H-050, H-044 | Whether old `COMFILE.DAT` files must stay readable after the header fix | owner decision | `tiedostomuodot.md` |
| H-041, H-042 | Supported `VALUKU`/`N_PV`/leg counts: grow buffers or refuse configurations above the limit | owner decision | rows; `ominaisuusvalitsimet.md` 3.3 |
| H-109 | May a competitor with a result be deleted; what the delete guard must refuse | owner decision | row; `globaali-tila.md` 5 |
| H-111, H-117 | Intended handling of A/L rows and of a point above the class split count (reject with message or clamp) | owner decision | rows; `gui-liiketoimintalogiikka.md` 4 |
| H-139, H-142 | Whether `nvak` should count P when `poissamukaan`; whether the name index key should include the class | owner decision | rows |
| H-119, H-120, H-175, H-081 | Meaning of `kuntosuunnmoodi` values and the GUI-only emit-code swap | owner decision | Q75, Q60 |
| H-132, H-133, H-134 | AJAT file naming in one-stage races; whether in-run stage change and re-run of `aloitus` are supported | owner decision | Q67, Q78, Q80, Q110 |
| H-161, H-169 | Intended SQL schema (`salisarja`, `badge` vs `lisno`); mirror-only principle for import | owner decision | `riippuvuudet.md` 19.3-19.4; Q104-Q106 |
| H-046 | Use of the relay export `.dat` downstream | owner decision | Q29 |
| H-071 | Whether FILESEND remote file write stays a feature | owner decision | H-071 row |
| H-172, H-177, T-052 | Were console XML, club results and RFID in ViestiLuenta removed on purpose | owner decision | Q111 |
| H-173, H-176, T-014, T-024, T-051 | Canonical key spellings, `AIKAKORJAUS` units, port 14770 vs 14900, relay `NREGNLY` per product | owner decision | Q112, Q25 |
| H-004, H-005, T-027 | Encoding policy (BOM or `/utf-8`) | owner decision | Q7; `merkistot.md` |
| H-061, H-056, H-076 | Run-time value of `pkglen`; frame assumption; console menu combination | run (disassembly, device or harness) | Q43, Q44, Q52 |
| H-020 | Which `add_sakko` is linked | build check (link map) | row |
| O2, O3 | Effect of H-087 fix on H-092/H-103; effect of an invalid-parameter handler on the harness | run after the fix | section 4.2 |

## 8. Open questions and limits
- Inferred links not traced in code: FT-6, FT-9, FT-10, O2, O3, O5 (consequence), O12, N10, N11; the inferred relations of `osaB/suhteet.csv` were not re-checked except where named in section 1.2.
- C5 counts the word `try` with comments included; it is an upper bound and does not change the conclusion for `Hk/`, `Juk/`, `Tp/`, `dbboxm/` (zero).
- That bcc32 classic lacks a usable standard lock guard is [inferred]; a hand-written guard works in both compilers either way.
- Primary/secondary assignment of borderline rows is a judgement: H-109 (JS-1 vs JS-6), H-019 (JS-10 vs JS-4), H-076 (JS-3 vs JS-6), H-159 (a one-off bug inside JS-9), H-007 (JS-5 vs JS-12). Counts in 1.1 move by at most one per cause if these flip. [inferred]
- Ranking uses current priorities; 61 open rows have no priority (inventory section 6), among them high rows H-007, H-022, H-023, H-024, H-031, H-040 that would raise JS-2, JS-5 and JS-7 further if prioritised.
- Candidate 3.1 needs a register decision and a GUI run.

## 9. Appendix: row -> root cause and draft package

| Id | Severity, priority | Primary | Made worse | Draft KK |
|---|---|---|---|---|
| H-001 | korkea P3 | ISO | JS-13 | KK-C13-13 |
| H-002 | korkea P4 | ISO | JS-1, JS-13 | KK-C13-12 |
| H-003 | keskitaso P5 | JS-13 | - | KK-C13-13 |
| H-004 | matala - | JS-15 | - | KK-C13-17 |
| H-005 | matala - | JS-15 | - | KK-C13-17 |
| H-006 | matala - | JS-13 | - | KK-C13-18 |
| H-007 | korkea - | JS-5 | JS-12, JS-4 | KK-C13-6 |
| H-008 | korkea - | JS-2 | JS-1 | KK-C13-4 |
| H-009 | korkea - | JS-4 | JS-11 | KK-C13-5 |
| H-010 | keskitaso - | JS-10 | - | KK-C13-6 |
| H-011 | keskitaso - | JS-10 | - | KK-C13-6 |
| H-012 | keskitaso - | JS-2 | - | KK-C13-4 |
| H-013 | keskitaso - | ISO | JS-6 | KK-C13-16 |
| H-014 | korkea - | JS-7 | JS-2 | KK-C10-1 |
| H-015 | keskitaso - | JS-11 | JS-4 | KK-C9-2 |
| H-016 | keskitaso - | JS-5 | - | KK-C13-10 |
| H-017 | keskitaso - | JS-5 | JS-12 | KK-C13-10 |
| H-018 | keskitaso - | JS-10 | - | KK-C13-7 |
| H-019 | matala - | JS-10 | JS-4 | KK-C13-6 |
| H-020 | matala - | ISO | JS-12, JS-14 | KK-C13-15 |
| H-021 | keskitaso - | JS-4 | JS-11 | KK-C13-9 |
| H-022 | korkea - | JS-5 | - | KK-C13-10 |
| H-023 | korkea - | JS-5 | JS-6 | KK-C13-10 |
| H-024 | korkea - | JS-2 | JS-12 | KK-C13-4 |
| H-025 | korkea - | JS-3 | JS-12 | KK-C13-1 |
| H-026 | korkea - | JS-1 | - | KK-C13-1 |
| H-027 | korkea - | JS-9 | JS-8 | KK-C13-9 |
| H-028 | keskitaso - | JS-8 | - | KK-C13-9 |
| H-029 | keskitaso - | JS-1 | JS-5 | KK-C10-4 |
| H-030 | keskitaso - | JS-5 | JS-3 | KK-C13-10 |
| H-031 | korkea - | JS-7 | JS-6 | KK-C13-3 |
| H-032 | korkea P1 | JS-1 | - | KK-C13-1 |
| H-033 | matala - | JS-1 | - | KK-C13-1 |
| H-034 | matala - | JS-5 | JS-9 | KK-C13-9 |
| H-035 | matala - | JS-3 | - | KK-C13-9 |
| H-036 | matala - | JS-1 | JS-14 | KK-C12-2 |
| H-037 | matala - | ISO | JS-8 | KK-C13-9 |
| H-038 | keskitaso - | JS-7 | - | KK-C13-8 |
| H-039 | keskitaso - | JS-5 | - | KK-C13-10 |
| H-040 | korkea - | JS-2 | JS-12 | KK-C13-4 |
| H-041 | korkea P3 | JS-3 | JS-14 | KK-C13-11 |
| H-042 | korkea P3 | JS-3 | JS-1 | KK-C13-11 |
| H-043 | keskitaso - | ISO | JS-11 | KK-C10-1 |
| H-044 | keskitaso - | JS-11 | JS-3 | KK-C9-2 |
| H-045 | keskitaso - | JS-10 | - | KK-C13-6 |
| H-046 | keskitaso - | ISO | JS-12 | KK-C13-11 |
| H-047 | keskitaso - | JS-10 | - | KK-C13-6 |
| H-048 | matala - | JS-3 | - | KK-C13-9 |
| H-049 | keskitaso P4 | JS-1 | - | KK-C13-12 |
| H-050 | keskitaso - | JS-10 | JS-3 | KK-C13-12 |
| H-051 | matala - | JS-10 | - | KK-C13-6 |
| H-052 | matala P4 | JS-1 | - | KK-C12-2 |
| H-053 | matala - | JS-11 | - | KK-C9-2 |
| H-054 | matala - | JS-11 | JS-8 | KK-C9-2 |
| H-055 | korkea P2 | JS-1 | - | KK-C13-2 |
| H-056 | korkea P2 | JS-1 | JS-9 | KK-C13-2 |
| H-057 | keskitaso P4 | JS-1 | - | KK-C13-12 |
| H-058 | keskitaso P4 | JS-1 | - | KK-C13-12 |
| H-059 | keskitaso P3 | JS-3 | JS-15 | KK-C13-13 |
| H-060 | keskitaso P2 | JS-1 | - | KK-C13-1 |
| H-061 | keskitaso P1 | JS-1 | - | KK-C13-1 |
| H-062 | keskitaso P3 | ISO | JS-13 | KK-C13-13 |
| H-063 | keskitaso P4 | JS-8 | JS-15 | KK-C9-3 |
| H-064 | keskitaso P4 | ISO | JS-13 | KK-C13-13 |
| H-065 | keskitaso P4 | JS-1 | - | KK-C13-12 |
| H-066 | matala P5 | JS-3 | - | KK-C13-16 |
| H-067 | matala P5 | JS-3 | - | KK-C13-13 |
| H-068 | matala P5 | ISO | JS-13 | KK-C13-13 |
| H-069 | matala P5 | JS-12 | - | KK-C13-15 |
| H-070 | korkea P2 | JS-1 | - | KK-C13-2 |
| H-071 | korkea P1 | JS-1 | - | KK-C13-1 |
| H-072 | korkea P1 | JS-1 | - | KK-C13-1 |
| H-073 | keskitaso P2 | JS-3 | JS-1 | KK-C13-1 |
| H-074 | keskitaso P4 | JS-1 | - | KK-C13-12 |
| H-075 | keskitaso P4 | ISO | JS-12 | KK-C13-13 |
| H-076 | keskitaso P3 | JS-3 | JS-6 | KK-C13-13 |
| H-077 | matala P5 | JS-3 | - | KK-C13-2 |
| H-078 | keskitaso P4 | JS-15 | - | KK-C13-12 |
| H-079 | matala P4 | JS-3 | JS-15 | KK-C13-12 |
| H-080 | matala P5 | JS-15 | - | KK-C13-17 |
| H-081 | matala P5 | JS-12 | - | KK-C13-17 |
| H-082 | keskitaso P4 | JS-1 | - | KK-C13-15 |
| H-083 | matala P5 | JS-1 | JS-14 | KK-C13-15 |
| H-084 | matala P5 | JS-1 | - | KK-C13-15 |
| H-085 | keskitaso P4 | JS-1 | - | KK-C13-15 |
| H-086 | matala P5 | ISO | - | KK-C13-12 |
| H-087 | korkea P2 | JS-2 | - | KK-C13-4 |
| H-088 | keskitaso P3 | JS-2 | - | KK-C13-4 |
| H-089 | keskitaso P3 | JS-2 | - | KK-C13-4 |
| H-090 | korkea P2 | JS-2 | - | KK-C13-4 |
| H-091 | korkea P2 | JS-4 | - | KK-C13-5 |
| H-092 | keskitaso P3 | JS-4 | - | KK-C13-5 |
| H-093 | korkea P2 | JS-4 | - | KK-C13-5 |
| H-094 | keskitaso P3 | JS-7 | JS-8 | KK-C13-3 |
| H-095 | matala-keskitaso P4 | JS-7 | - | KK-C13-8 |
| H-096 | matala P4 | JS-4 | JS-8 | KK-C13-8 |
| H-097 | keskitaso P3 | JS-7 | - | KK-C13-3 |
| H-098 | matala P4 | JS-7 | - | KK-C13-8 |
| H-100 | matala P4 | JS-10 | - | KK-C10-1 |
| H-101 | matala P4 | JS-10 | JS-7 | KK-C13-6 |
| H-102 | matala P4 | JS-6 | JS-10 | KK-C13-3 |
| H-103 | keskitaso P3 | JS-4 | - | KK-C13-5 |
| H-104 | keskitaso P3 | JS-7 | - | KK-C13-8 |
| H-105 | keskitaso P3 | JS-8 | JS-6 | KK-C13-3 |
| H-106 | keskitaso P3 | JS-10 | JS-12 | KK-C13-7 |
| H-107 | keskitaso P2 | JS-10 | - | KK-C13-7 |
| H-108 | matala-keskitaso P4 | JS-10 | JS-9 | KK-C10-2 |
| H-109 | korkea P2 | JS-1 | JS-6 | KK-C13-14 |
| H-110 | korkea P2 | JS-2 | - | KK-C13-4 |
| H-111 | korkea P3 | ISO | JS-12 | KK-C13-14 |
| H-112 | keskitaso P3 | JS-1 | JS-12 | KK-C13-14 |
| H-113 | keskitaso P3 | JS-1 | - | KK-C13-14 |
| H-114 | keskitaso P3 | JS-1 | - | KK-C13-14 |
| H-115 | keskitaso P3 | JS-2 | JS-1 | KK-C13-14 |
| H-116 | keskitaso P3 | ISO | JS-4 | KK-C13-14 |
| H-117 | keskitaso P3 | JS-12 | JS-11 | KK-C13-14 |
| H-118 | matala-keskitaso P4 | ISO | - | KK-C13-15 |
| H-119 | matala-keskitaso P4 | ISO | - | KK-C13-15 |
| H-120 | matala-keskitaso P4 | ISO | - | KK-C13-15 |
| H-121 | matala-keskitaso P4 | JS-12 | - | KK-C13-15 |
| H-122 | matala-keskitaso P4 | JS-3 | - | KK-C13-15 |
| H-123 | matala-keskitaso P4 | ISO | - | KK-C13-15 |
| H-124 | matala-keskitaso P4 | JS-1 | - | KK-C13-15 |
| H-125 | matala P4 | JS-2 | JS-1 | KK-C13-4 |
| H-126 | matala P5 | ISO | JS-12 | KK-C13-15 |
| H-127 | matala P5 | JS-12 | - | KK-C13-15 |
| H-128 | keskitaso-korkea - | JS-6 | - | KK-C13-16 |
| H-129 | korkea P2 | JS-6 | JS-3 | KK-C13-3 |
| H-130 | keskitaso - | JS-6 | - | KK-C13-16 |
| H-131 | keskitaso - | JS-6 | - | KK-C13-3 |
| H-132 | matala-keskitaso - | JS-6 | - | KK-C13-3 |
| H-133 | matala-keskitaso - | JS-6 | - | KK-C13-3 |
| H-134 | keskitaso P3 | JS-6 | JS-11 | KK-C13-3 |
| H-135 | matala-keskitaso - | JS-6 | - | KK-C13-16 |
| H-136 | matala-keskitaso - | JS-10 | - | KK-C13-6 |
| H-137 | matala-keskitaso - | JS-10 | - | KK-C13-6 |
| H-138 | matala-keskitaso - | JS-10 | JS-9 | KK-C10-2 |
| H-139 | matala-keskitaso - | JS-6 | - | KK-C13-16 |
| H-140 | matala-keskitaso - | JS-6 | - | KK-C13-16 |
| H-141 | matala - | JS-6 | - | KK-C13-3 |
| H-142 | matala - | JS-6 | - | KK-C13-16 |
| H-143 | matala - | JS-6 | - | KK-C13-3 |
| H-144 | matala - | JS-10 | JS-4 | KK-C13-6 |
| H-145 | keskitaso P3 | JS-11 | - | KK-C9-2 |
| H-146 | keskitaso P3 | JS-11 | - | KK-C9-2 |
| H-147 | keskitaso P3 | JS-11 | - | KK-C9-2 |
| H-148 | keskitaso P4 | JS-11 | JS-2 | KK-C9-2 |
| H-149 | korkea P2 | JS-8 | - | KK-C9-1 |
| H-150 | keskitaso P2 | JS-8 | - | KK-C9-3 |
| H-151 | keskitaso P3 | JS-8 | - | KK-C9-1 |
| H-152 | keskitaso P4 | JS-8 | - | KK-C9-1 |
| H-153 | keskitaso P4 | JS-4 | JS-11 | KK-C9-2 |
| H-154 | matala P4 | JS-8 | - | KK-C9-1 |
| H-155 | matala P4 | JS-8 | JS-9 | KK-C9-3 |
| H-156 | matala P5 | JS-11 | - | KK-C9-2 |
| H-157 | matala P5 | JS-8 | - | KK-C13-8 |
| H-158 | matala P5 | JS-8 | JS-14 | KK-C12-1 |
| H-159 | korkea P2 | JS-9 | - | KK-C10-2 |
| H-160 | korkea P2 | JS-2 | - | KK-C10-1 |
| H-161 | keskitaso P3 | ISO | - | KK-C10-1 |
| H-162 | keskitaso P3 | JS-2 | JS-4 | KK-C10-1 |
| H-163 | keskitaso P3 | JS-2 | JS-13 | KK-C10-1 |
| H-164 | keskitaso P3 | JS-9 | - | KK-C10-2 |
| H-165 | keskitaso P3 | JS-13 | - | KK-C10-3 |
| H-166 | keskitaso P3 | JS-9 | JS-1 | KK-C10-2 |
| H-167 | matala P4 | JS-4 | JS-7 | KK-C10-2 |
| H-168 | matala P4 | ISO | JS-5 | KK-C10-2 |
| H-169 | keskitaso P3 | ISO | - | KK-C10-1 |
| H-170 | matala P5 | JS-9 | - | KK-C10-2 |
| H-171 | korkea P2 | JS-3 | - | KK-C12-2 |
| H-172 | keskitaso P3 | JS-14 | - | KK-C12-1 |
| H-173 | matala-keskitaso P3 | JS-14 | - | KK-C12-2 |
| H-174 | matala-keskitaso P3 | JS-11 | JS-6 | KK-C13-12 |
| H-175 | matala-keskitaso P4 | ISO | JS-14 | KK-C13-15 |
| H-176 | matala P5 | JS-14 | - | KK-C12-2 |
| H-177 | matala P5 | JS-14 | - | KK-C12-3 |

T rows and the root cause they state or belong to (hygiene and dead-code rows are filed under JS-14 as switch and dead-code drift; T-001/T-002 layering under JS-10 as the global-state architecture):

| T | Root cause | Draft KK |
|---|---|---|
| T-001 | JS-10 | KK-C13-21 |
| T-002 | JS-10 | KK-C13-21 |
| T-003 | JS-10 | KK-C13-21 |
| T-004 | JS-15 | KK-C13-17 |
| T-005 | JS-13 | KK-C10-3 |
| T-006 | JS-14 | KK-C10-3 |
| T-007 | JS-14 | KK-C12-3 |
| T-008 | JS-12 | KK-C12-3 |
| T-009 | JS-14 | KK-C12-3 |
| T-010 | - | ULKOPUOLELLA |
| T-011 | JS-14 | KK-C12-3 |
| T-012 | JS-14 | KK-C12-3 |
| T-013 | JS-14 | KK-C12-3 |
| T-014 | JS-14 | KK-C12-2 |
| T-015 | JS-14 | KK-C12-3 |
| T-016 | JS-15 | KK-C13-17 |
| T-017 | JS-14 | KK-C10-3 |
| T-018 | JS-14 | KK-C12-3 |
| T-019 | JS-11, JS-5 | KK-C13-11 |
| T-020 | JS-8, JS-15 | KK-C9-3 |
| T-021 | JS-13 | KK-C13-17 |
| T-022 | JS-1 | KK-C13-19 |
| T-023 | JS-15 | KK-C13-17 |
| T-024 | JS-14 | KK-C12-2 |
| T-025 | JS-15 | KK-C13-17 |
| T-026 | JS-15, JS-3 | KK-C13-17 |
| T-027 | JS-15 | KK-C13-17 |
| T-028 | JS-14 | KK-C12-3 |
| T-029 | JS-14 | KK-C12-3 |
| T-030 | JS-7 | KK-C13-8 |
| T-031 | JS-4 | KK-C13-5 |
| T-032 | JS-4 | KK-C13-5 |
| T-033 | JS-10, JS-4 | KK-C13-7 |
| T-034 | JS-12 | KK-C13-22 |
| T-035 | JS-12 | KK-C13-11 |
| T-036 | JS-2 | KK-C13-4 |
| T-037 | JS-1 | KK-C13-14 |
| T-038 | JS-6 | KK-C13-20 |
| T-039 | JS-6 | KK-C13-16 |
| T-040 | JS-11, JS-4 | KK-C9-2 |
| T-041 | JS-11 | KK-C9-2 |
| T-042 | JS-8 | KK-C9-1 |
| T-043 | JS-1 | KK-C10-4 |
| T-044 | JS-13 | KK-C10-3 |
| T-045 | JS-14 | KK-C12-3 |
| T-046 | JS-13 | KK-C10-3 |
| T-047 | - (licence decision) | KK-C10-3 |
| T-048 | JS-9 | KK-C10-2 |
| T-049 | JS-14 | KK-C10-3 |
| T-050 | JS-14 | KK-C12-3 |
| T-051 | JS-14 | KK-C12-2 |
| T-052 | JS-14 | KK-C12-3 |
