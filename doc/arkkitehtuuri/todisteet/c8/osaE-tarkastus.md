<!-- Provenance: final report of independent-verifier (Opus), C-8 step 1 part E, 2026-11-26; copied by the main session from the hand-back message. -->

C-8 step 1, independent review of parts C, B and D (global mutable state). Read-only; I wrote no files and ran no code. Line numbers come from the working tree (cp1252 files read through iconv). Root is `C:\dev\tulospalvelu\TPsource\V52`, and all paths below are relative to it.

## Summary

- **Confirmed (8):** N-C1, N-C2, N-C3, N-C4, N-C6, N-C8, N-B3, N-B11.
- **Confirmed with changed wording or severity:** N-C10 (overflow only under a narrower condition), N-B12, N-D2.
- **Partly confirmed:** N-B1, N-B2, N-B4, N-D1, N-D3.
- **Refuted:** none outright. The counter-doubling part of N-B2 cannot happen when the retry uses the same competition.
- **Duplicates:** none of the claims repeats H-001..H-127. Related entries: H-102 (same line as N-C3, a different defect), H-094 (same subsystem as N-D1, different data), H-072 (`eralahto`, a different defect), H-108 (another static in reader opening). Q67 was "not registered"; N-B1 and N-B2 are its concrete consequences.

---

## Part C

### N-C1: `setpos` never deletes, so stale `key -> 0` entries cut short every walk in bib order
**Verdict: VAHVISTETTU (confirmed). Severity: medium-high (claimed: high).**
- **The delete branch is dead code.** `Hk/HkIx.cpp:217-221`: the inner test `!ret && d == dd` sits inside `if (ret || d != dd)`, so `delix` can never run. Every change calls `addix`.
- **The newest entry goes first.** `tputilv2/dbindex.cpp:170-173`: `bsrch` steps back to the first equal key. `addix` (`:59-68`) inserts at that point, so the new entry comes before older entries with the same key. `searchix` (`:113-117`) returns that first entry. `nextix` (`:126-135`) walks the array in order and returns the value without filtering.
- **How deletes and bib changes leave stale entries.**
  - Delete: `Hk/HkDat.cpp:760` adds `key -> 0` in front of `key -> d`.
  - Bib change: `:1157` adds the new bib, then `:1180` adds `old -> 0`.
  - Re-adding the same bib (`:720`) gives the order `new d'`, `0`, `old d`.
- **Effect on walks.** All the walks I checked stop when `d <= 0`: `Hk/HkIV.cpp:1914-1922` (`haeratakilp`), `Hk/HkTulostus.cpp:2548-2575` (meri3 points), `cbHk/UnitSakkoKierr.cpp:68-83`, `cbHk/UnitArvonta.cpp:223-232` (`SeurPos` case 0, used by the draw). The one-step uses also fail: `cbHk/UnitExport.cpp:346`, `cbHk/UnitKilpailijatiedot.cpp:1120-1121`, `cbHk/UnitKilpailijaOnline.cpp:617-618`. If the first entry at or above `x` is a stale 0, `getpos(x, true)` itself returns -1 (`:178`).
- **Self-healing.** `luo_posarr` resets the index (`Hk/HkIx.cpp:314-320`). Its only caller is the end of a draw operation (`cbHk/UnitArvonta.cpp:761`), so the draw also heals it.
- **Why medium-high rather than high.** Result lists use the rank chains, not `idindex`. The walks feed export, draw, the penalty list, course identification (`rataTunnistus`) and meri3 points.
- **Side lead, not checked.** In `SeurPos` case 0, `Ed` is a record position from the previous call (`:421`) but is used as a bib (`getpos(Ed+1, true)`, `:223`).
- **To settle it:** a doctest that links `Hk/HkIx.cpp` and `dbindex.cpp`: `setpos(105,7); setpos(106,8); setpos(105,0); d=getpos(1,true); nextpos()...` and check that the walk stops at 105.

### N-C2: `setpos` skips the add when the next-greater key maps to the same record
**Verdict: VAHVISTETTU. Severity: medium (claimed: medium-high).**
- **Mechanism.** When the key is absent, `searchix` returns the next-greater entry and `ret = 0` (`bsrch` clears `ok` only when the key is above every key). If that entry's value equals `d`, then `ret || d != dd` is false and `addix` is never called.
- **Trigger.** In `tallenna`, `setpos(new, d)` (`Hk/HkDat.cpp:1157`) runs while `old -> d` is still the next-greater entry. That happens when the bib is lowered and there are no entries between the new and old number (61 to 60, or 65 to 60 when 60-64 have never been used).
- **Effect.**
  - `:1180` then hides the old entry as well, so `getpos(new) == -1`.
  - `setEmitJarr` (`Hk/HkEmit.cpp:1019`) and `addbadge` (`:1044`) silently do nothing.
- **Self-healing.** Restart, or `luo_posarr`.
- **To settle it:** the same doctest: `setpos(61,7); setpos(100,8); setpos(60,7); setpos(61,0); getpos(60)` should return -1.

### N-C3: `teejarj(true)` does not reset `idindex->nrec`
**Verdict: VAHVISTETTU. Severity: high (agree). Two thresholds corrected.**
- **The defect.**
  - `Hk/HkIx.cpp:1610` runs `memset` only; `nrec` is not reset.
  - `setpos` at `:1757` then searches over `nrec_old` zero keys. Every real key is greater, so `ret == 1` and `addix` appends at `base + nrec*recl`.
  - `addix` has no capacity check. The buffer holds `maxrec+2` records (`:1600`). Each append past that limit writes 8 bytes beyond the block.
- **How `maxrec` is set (Tila 2).** `filelen(&datf2) + n_lisaa` (`:1573`; `n_lisaa` defaults to 1000, `Hk/HkGlobals.cpp:394`, and can be set at `Hk/HkInit.cpp:1313`). It is raised to at least 3000 (`:1576`) and capped at `maxkilpno = MAXKILPNO = 9999` (`Hk/HkDef.h:28`).
- **Corrected thresholds** (default `n_lisaa`, no deletes, `nrec_old ≈ n`):
  - **First stage change:** overflow when 2n > maxrec + 2. With `filelen <= 2000` that is n >= ~1502. With `filelen > 2000` it is always true.
  - So "about 1500 or more competitors" is right. The analyst's second case ("n > ~1000 when filelen+1000 > 3000") only applies when n > 2000 anyway.
  - **Second stage change** (3-stage event): about n >= 1001.
  - Each delete adds one entry to `nrec_old` and each bib change adds two (N-C1), which lowers the thresholds.
- **Reachability.** The GUI calls `vaihdavaihe` at `cbHk/WinHk.cpp:2126` and `:2209`. A remote request reaches it through `Hk/HkCom32.cpp:847`. The console calls it at `HkConsole/HkCons0.cpp:871` and `:933`.
- **Relation to H-102.** H-102 is the missing lock on the same line; this is a different defect.
- **Side effect if `MINKILPNO=0` is configured:** after the stage change, the zero-key prefix makes `getpos(0, true)` return -1 ([paatelty]).
- **To settle it:** debugger watch on `idindex->nrec` before and after `vaihdavaihe`, or a 2000-record stage change under PageHeap.

### N-C4: `nvak` is incremented for V and P but decremented only for V
**Verdict: VAHVISTETTU. Severity: low-medium (agree).**
- Increment: `Hk/HkIx.cpp:822-823`. Decrement: `:1192-1193`. Both sit behind the same `onMukana` gate (`:47-51`).
- `vaihdavaihe` resets it (`:1927`).

### N-C6: with two badges, a bib change leaves badge 2 mapped to the old bib
**Verdict: VAHVISTETTU. Severity: medium-low (claimed: medium).** Confidence can be raised to [todennettu] for the code path.
- **Mechanism.**
  - The id-change block is inside the `ibdg` loop (`Hk/HkDat.cpp:1042`, `:1151-1181`).
  - Iteration 0 ends with `setpos(old, 0)` (`:1180`).
  - In iteration 1, `rembadge(ebadge[1], old)` returns at `getpos(old) <= 0` (`Hk/HkEmit.cpp:1102-1103`).
  - `addbadge(ubadge[1], new)` (`:1173`) then finds the badge under the old bib and returns 1 with "Kilpailijoilla … sama e-koodi" (`Hk/HkEmit.cpp:1056-1063`). The message is shown only if `kysyfl` is set. The final loop (`:1215-1218`) fails the same way.
- **Trigger.** `kilpparam.kaksibadge` is 1 or 2 and badge 2 is non-zero.
- **Extra consequence, kaksibadge == 2 ([paatelty]).** `getem(&em, old, 1)` (`:1166`) also fails, so `em.kilpno` in the emit file keeps the old bib. After a restart, `lueemitfile` links that card to the old bib.
- **To settle it:** a GUI test with `kaksibadge=2`, then `bdg2kno(badge2)`.

### N-C8: `emitjarr[..][d]` is not cleared on delete
**Verdict: VAHVISTETTU. Severity: low-medium (agree).**
- `deltall` (`Hk/HkDat.cpp:740-770`) and `rembadge` (`Hk/HkEmit.cpp:1096-1155`) never call `setEmitJarr(-1, ...)`.
- `delfrec` → `deleterec` pushes the slot on the free list (`dbboxm/Dbbox.cpp:200-207`). `addfrec` → `addrec` → `newrec` reuses it; I did not read `newrec`, so LIFO reuse is [paatelty].
- `getem` does not check `kilpno` (`Hk/HkEmit.cpp:1587-1596`).

### N-C10: relay `tallenna` series-change update of the counts is dead code
**Verdict: dead code VAHVISTETTU; overflow OSITTAIN (partly). Severity: medium (agree).**
- **Dead code.** `Juk/vdat.cpp:738` `kilp = *ekilp;` makes the test at `:741` always false.
- **No other writer.** `nilm` is written only at `:742-743`, `:898`, `:927`, `Juk/VIx.cpp:658` (via `paivJoukkLkm`), `:1173` and `:1243`.
- **Reachable from the GUI.** The series combo has no Tila guard (`ViestiWin/UnitJoukkuetiedot.cpp:584-589`).
- **Overflow, the part that holds.**
  - `ossijat` allocates `(nilm[srj]+1) × (osuusluku or maxnosuus)` entries (`Juk/VTulostus.cpp:960-965`).
  - It fills one entry per (team, parallel leg) only for teams with a non-zero `osuustulos` (`:1015-1023`). `nt` is reset per (os, va) unless `monios` is set.
- **Narrower trigger than claimed.** Two or more teams moved into the series is not enough on its own. The number of teams with a result on that leg (including the moved-in teams) must exceed `nilm[srj]+1`. In practice that means final results of a series in which almost every team has finished.

### N-C1 to N-C10: how to register

| Proposed entry | Claims | Note |
|---|---|---|
| One entry | N-C1 + N-C2 | Same function (`setpos`), same fix |
| Separate entry | N-C3 | Heap overflow, different fix (reset `nrec`). Cross-reference H-102 |
| Separate entry | N-C6 | Optionally merged with N-C7 and N-C15 as "`bdg_kno` maintenance in `tallenna`" |
| Small entries | N-C4, N-C8 | N-C4 could merge with N-C5 as "HK counter asymmetry" |
| Separate entry | N-C10 | |

---

## Part B

### N-B1: `aloitus` patches file names in place and is not idempotent
**Verdict: OSITTAIN. Severity: low-medium (claimed: medium).**
- **What holds.**
  - Defaults: `Hk/HkGlobals.cpp:127`, `:132`. Patching: `Hk/HkInit.cpp:3357-3360`, `:3371-3372`.
  - A retry gives `COMFILE.DAT` → `COMFILEDAT`, then `COMFILEAT` and so on. `AJAT.LST` → `AJATLST` likewise.
  - Nothing reassigns `obfilename`. `aikaflname[0]` is reassigned only by `AJAT=name` (`:1418`).
  - The GUI retry is reachable: `BtnTulPalvClick` and `BtnIlmTilaClick` (`cbHk/WinHk.cpp:1176-1262`) have no guard, and a failed `Initialisoi` only sets `ToimintaTila = 0` (`:279-283`).
- **Correction to the trigger.**
  - `datf2` is never closed on the failure path.
  - `openfile` (`dbboxm/dbbox1.cpp:176-178`, the file compiled into the GUI through `RADStudio10/DBboxm-XE.cbproj:147`) opens with `GENERIC_WRITE` and `FILE_SHARE_READ`.
  - So a same-competition retry after any failure later than `:3486` fails again at `:3491` with a sharing violation (Win32 semantics, [paatelty]). The names are corrupted, but that run never gets going.
  - Corruption that actually takes effect therefore needs one of:
    - (a) a first failure at `:3402` (opening the LOKI file) or `:3491` (KILP.DAT locked by another instance), followed by a successful retry;
    - (b) a retry with a different competition or folder.
- **New side lead, stronger than N-B1 ([paatelty], new).**
  - `AJAT=tied.nimi` is documented as a free file name (`docs-site/docs/hkkisawin/hkliite1.md`).
  - It is copied at `:1418` (inside `lue_parametrit`, called at `:3176`).
  - With `n_pv_akt == 1`, `:3359` deletes its 5th character on the first run: `AJAT.LST` → `AJATLST`, `C:\data\ajat.lst` → `C:\dta\ajjat.lst`.
  - To settle it: start a one-stage GUI competition with `AJAT=AJAT.LST` and look at the file that gets created.

### N-B2: a `teejarj(false)` rerun does not zero counters
**Verdict: OSITTAIN; the same-competition scenario is refuted. Severity: low (claimed: medium).**
- **True statically.** No counter reset in `teejarj(false)` (`Hk/HkIx.cpp:1568-1602`; the only resets are at `:1519-1535` and `:1913-1929`). The allocations at `:1584`, `:1596` and `:1600` leak.
- **Refuted for a same-competition retry.** It cannot reach `teejarj` because `openfile` fails at `Hk/HkInit.cpp:3486-3491` (sharing, see N-B1).
- **Still reachable** only when the retry uses a different KILP.DAT path after a first failure at `:3588-3661`.
- **Additional effect ([paatelty]).** `initindex` does not reset `idindex->nrec` (`Hk/HkIx.cpp:101-107`). The fresh `calloc` at `:1600` therefore gets the same append-after-`nrec_old` pattern as N-C3, and can overflow for large competitions.

### N-B3: `vaihdavaihe` does not patch the template `aikaflname[]`
**Verdict: VAHVISTETTU. Severity: medium (agree).**
- `Hk/HkIx.cpp:1891-1894` patches only existing `aikajono[]` entries. `inittimer` copies the template (`Hk/HkIV.cpp:4227`), which was set only at `Hk/HkInit.cpp:3364-3367`.
- New queues are created from `cbHk/UnitAjanotto.cpp:96-98`, `cbTp/UnitEmiTag.cpp:382` and `cbHk/UnitTaulu.cpp:1152`.
- Queues that already exist survive: `suljeajat(-1, false)` keeps them, and `initajat(ino, 2)` uses the patched name (`:1957-1970`).

### N-B4: `eralahto[]` and `era_akt` are not reset at stage change
**Verdict: OSITTAIN. Severity: low (claimed: medium).**
- The missing reset is true: `vaihdavaihe` has no reference to either.
- **Affected paths.** Only the paths that store a value when it is still the sentinel:
  - ARES, `Hk/HkIV.cpp:2407-2411`;
  - `tallenna` for kilplaji N/P/U, `Hk/HkDat.cpp:1254-1257`.
- **Unaffected paths.** The SW2000 START message overwrites the value unconditionally (`:2529`, `:2563`). A manual start entry in the GUI also updates it (`cbHk/UnitAjanotto.cpp:1256-1258`).
- **Scope.** Multi-stage N/P/U events with heats and ARES timing, changing stage without a restart. Memory only; a restart fixes it.

### N-B12: stage change without `RADAT<k>.XML` turns emit checking off with no text fallback
**Verdict: VAHVISTETTU. Severity: low-medium (claimed: medium).**
- `lueEventXml` returns -1 when the file does not open (`Hk/HkXml.cpp:~1824-1826`).
- On stage change, `Hk/HkIx.cpp:1936-1939` sets `emitfl = 0` after `vapautaemitmuisti` (`:1907`) has already freed the course data.
- `aloitus` falls back to `lue_leimat` and `lue_radat` instead (`Hk/HkInit.cpp:3593-3600`).
- The user sees an error message, which is why I lower the severity.

### N-B11: `on_emit` frees `emitvaraus` but does not reset `nemitvar`
**Verdict: VAHVISTETTU. Severity: low (agree).**
- `Hk/HkEmit.cpp:1303-1314` and `:1627-1638` free the table but leave `nemitvar`. The only free comes from `:6367`.
- The phantom zeros keep the table sorted. Capacity is `maxrec+1000`.
- **When suppression stops.** With about 1500 cards per stage and `maxrec = 3000`, duplicate suppression stops around the third stage change ([paatelty]).

### Part B: how to register
- N-B1 + N-B2: one entry, "`aloitus` is not safe to re-run (Q67)".
- The `AJAT=` lead: its own entry once confirmed by a run.
- N-B3: its own entry.
- N-B12: its own entry.
- N-B4 + N-B11: one low entry, "state not reset at stage change", or append to the B family.

---

## Part D

### N-D1: `bdg_t[8]` is shared without a lock
**Verdict: OSITTAIN. Severity: medium-low (claimed: medium).**
- **No lock: confirmed.**
  - HK: `pros_bdg_t` calls `tall_bdg_t` after `LeaveCriticalSection` (`Hk/HkIV.cpp:796-808`). `tall_bdg_t` does `memmove` at `:775` and writes at `:778-779`.
  - The reader threads call `tall_emit` → `hae_bdg_t` (`:1026`, `:1078`), which does `memmove` at `:871` and writes at `:872`.
  - `tall_bdg_t` takes no lock, so any lock held by a `tall_emit` caller would not help.
- **Effect on results: confirmed.** `em->maali = em->time - 10*(tlkja - ctrltime)` (`Hk/HkEmit.cpp:1758-1759`); start and split times at `:3631-3642`.
- **Corrections.**
  - Relay `hae_bdg_t` (`Juk/VIv.cpp:794-808`) only reads; it does not modify the table.
  - With one serial reader, the badge message and the card data are handled in order on the same thread. Transferring the card data (about 200 bytes) takes longer than the 50 ms poll, so the two paths normally do not overlap.
  - The realistic overlap needs two or more reader threads.
  - The most likely result is a lost entry. `em.time` then falls back to `biostime` (the read moment), off by the read delay of sub-second to seconds. A torn entry carrying an old `t` from a freed slot needs a window of a few instructions.
- **To settle it:** a two-thread probe in the style of C-6 (`add_bdg_t` plus `hae_bdg_t` with distinct badges).
- **Register:** a separate H entry, cross-referenced to H-094.

### N-D2: `emithead2` check-then-act between `addem` and the consumer
**Verdict: VAHVISTETTU as a mechanism. Severity: medium-low (agree).**
- I reconstructed the interleaving:
  1. `addem` holds `emit_CS` and has done `emithead++` (`Hk/HkEmit.cpp:1673`), then `fflush` (`:1675-1677`).
  2. The consumer passes the unlocked guard (`Hk/HkIV.cpp:1184`), evaluates `emithead2` as H, and blocks inside `lueem` on `emit_CS` (`Hk/HkEmit.cpp:1579`).
  3. `addem` sets `emithead2 = H+1` (`:1679-1680`).
  4. `emithead2++` at `HkIV.cpp:1189` reloads the global and stores H+2.
  5. The next card at H+1 is skipped (guard `>=`). `HkIV.cpp:1322` only handles `<`, so nothing recovers it.
- The relay code is the same (`Juk/VEmit.cpp:1514-1520`, `Juk/VIv.cpp:1136-1141`).
- Probability ([paatelty]): it needs another station forwarding processed cards and a consumer pass that falls inside the `fflush` window.
- **Register:** a separate H entry.

### N-D3: `static int count` in `openlukijaTCP` is shared by all TCP readers
**Verdict: OSITTAIN. Severity: low-medium (claimed: medium).**
- **What holds.**
  - The static is at `Tp/TpComY.cpp:2123`.
  - The function is compiled under `#if defined(TCPLUKIJA) || defined(LAJUNEN)` (`:2078`). Both are defined in `RADStudio10/HkKisaWin.cbproj`, `ViestiWin.cbproj`, `VS/Hk/HkMaali/*.vcxproj` and `VS/V/JukMaali520.vcxproj`.
  - There is one thread per reader, which loops (`Tp/TpLaitteet.cpp:1785-1797`). There are also callers at `:2253` and `Tp/ZebraReader.cpp:934`.
  - The back-off is 5 + 55 + 180 s.
- **Correction to the trigger.**
  - `openportTCP` uses a blocking `connect` (`com/WinUDP.cpp:772-775`).
  - A switched-off or unreachable reader typically returns WSAETIMEDOUT or WSAEHOSTUNREACH. Both go to `default` → `poistu = 1` → that thread stops without incrementing `count` (`:2155-2163`).
  - `count` grows only on WSAECONNREFUSED or WSAEHOSTDOWN: a reachable host whose port refuses connections.
- **Register:** a separate low entry, alongside H-108.

---

## Summary table

| Claim | Verdict | Severity (claimed → mine) | Register |
|---|---|---|---|
| N-C1 | VAHVISTETTU | high → medium-high | merge with N-C2 |
| N-C2 | VAHVISTETTU | medium-high → medium | merge with N-C1 |
| N-C3 | VAHVISTETTU (thresholds corrected) | high → high | own entry; see H-102 |
| N-C4 | VAHVISTETTU | low-medium | own entry, or with N-C5 |
| N-C6 | VAHVISTETTU | medium → medium-low | own entry, or with N-C7 and N-C15 |
| N-C8 | VAHVISTETTU (slot reuse [paatelty]) | low-medium | own entry |
| N-C10 | dead code VAHVISTETTU, overflow OSITTAIN | medium | own entry |
| N-B1 | OSITTAIN (reachability narrower) | medium → low-medium | with N-B2 |
| N-B2 | OSITTAIN (same-competition retry refuted) | medium → low | with N-B1 |
| N-B3 | VAHVISTETTU | medium | own entry |
| N-B4 | OSITTAIN | medium → low | with N-B11 |
| N-B11 | VAHVISTETTU | low | with N-B4 |
| N-B12 | VAHVISTETTU | medium → low-medium | own entry |
| N-D1 | OSITTAIN | medium → medium-low | own entry; see H-094 |
| N-D2 | VAHVISTETTU (mechanism) | medium-low | own entry |
| N-D3 | OSITTAIN (trigger narrower) | medium → low-medium | own entry; see H-108 |
| New: `AJAT=` name mangled in one-stage events | lead [paatelty] | medium if confirmed | candidate entry |

## Coverage
- **Read in full:** `setpos`/`getpos`/`nextpos`/`luo_posarr` (`Hk/HkIx.cpp:155-335`), `tputilv2/dbindex.cpp:51-180`, the start of `teejarj` and its `setpos` loop, `vaihdavaihe` (`Hk/HkIx.cpp:1855-2004`), the `tallenna` badge/id block (`Hk/HkDat.cpp:1040-1225`), `addtall`/`deltall`/`addfrec`/`delfrec`, `addbadge`/`rembadge`/`getEmitJarr`/`setEmitJarr`, `on_emit`, `addem`, the head of `pros_emit`, `tall_emit`, `bdg_t` with `tall_bdg_t`/`pros_bdg_t`/`add_bdg_t`/`hae_bdg_t`, relay `hae_bdg_t`/`addem`/consumer head, relay `tallenna:720-760`, `ossijat` (`Juk/VTulostus.cpp:949-1060`), `openlukijaTCP`, the `connect` path of `openportTCP`, the `aloitus` sections cited, the GUI start handlers and `ProsEmitThread`.
- **Grepped only:** callers of `nextpos`, `setpos`, `vaihdavaihe`, `inittimer`, `eralahto`, `emithead2`, `paivJoukkLkm`; the build defines.
- **Not checked:** `newrec` (LIFO reuse), relay `tallenna` rem/add series handling beyond the lines cited, `lueemitfile` relinking, part D's other claims (N-D4 onward), part C's N-C5, N-C7, N-C9, N-C11..N-C15, and part B's N-B5..N-B10, N-B13 and N-B14.

## Open questions
1. N-C1/N-C2/N-C3: no doctest was run. `Tests/` could link `HkIx.cpp` and `dbindex.cpp` for a direct check.
2. N-C3: what `n_lisaa` real installations use, and whether operators use the in-process stage change or restart between stages.
3. N-B1/N-B2: confirm the sharing violation on a same-process re-open of `KILP.DAT` (a GUI retry test).
4. `AJAT=` mangling: confirm with a one-stage run; check whether the `docs-site` examples always use names where losing the 5th character goes unnoticed.
5. N-D1: whether events in practice run two or more LID_LUKIJA or EMITAG readers on one HK instance; the probe described above.
6. N-D2: how often `pros_emit` is called in the GUI (what posts `WM_MYMSGNEWCARD`), which sets the probability.
7. N-D3: which Winsock error a powered-off reader actually returns to a blocking `connect` on the local subnet.
8. Side lead: `SeurPos` case 0 passes a record position where `getpos` expects a bib (`cbHk/UnitArvonta.cpp:221-223` vs `:421`); not analysed.
