<!-- Provenance: final report of defect-concurrency-auditor (Sonnet), C-8 step 1 part C, 2026-11-26; copied by the main session from the agent's second hand-back message (the agent declined to write repository files). -->

C-8 step 1 part C: integrity of derived structures (K3). Read-only. No repository file was modified or created. Evidence line numbers are from `sed`/`grep -n` on the working tree of 2026-11-26. I did not run any code. HK means `Hk/` plus `cbHk/` and `HkConsole/`. Relay means `Juk/` plus `ViestiWin/` and `JukConsole/`.

## 1. Conclusion

I found 15 new suspected defects (N-C1..N-C15) and 7 latent or dead-code notes (L-1..L-7).

The central problem is in the HK bib index `idindex`, maintained by `setpos` (Hk/HkIx.cpp:205-226):
- **N-C1.** The `delix` branch of `setpos` can never execute (:219-220), and `addix` is the only write. Every delete or bib change leaves stale entries. A stale `key -> 0` entry sits in front of the older `key -> d` entry. Every number-order walk (`getpos(x, true)` followed by `nextpos()`) stops at the first entry whose value is not above 0.
- **N-C2.** The add in `setpos` is skipped when the next-greater key already maps to the same record (:216). A bib change into a free number just below the old bib then leaves the competitor without an index entry.
- **N-C3.** `teejarj(true)` (stage change) zeroes `idindex->ix` but not `idindex->nrec` (Hk/HkIx.cpp:1610). After the rebuild `nrec` is old plus new, and the buffer holds `maxrec+2` records. With about 1500 or more competitors this overflows the heap at the first stage change.

The other findings are individually smaller:
- **Counters.** N-C4: `nvak` is incremented for V and P but decremented only for V. N-C5: `nyilmt` and `nytulost` are never reset. N-C10: in relay, a series change via `tallenna` never updates `nilm`/`nilmt`/`nilmtt`, because the update branch compares `kilp.sarja` with `ekilp->sarja` right after `kilp = *ekilp`. N-C12: relay combined-series double counting.
- **Badge and emit maps.** N-C6: with two badges, a bib change fails to remove the second badge from `bdg_kno`. N-C7: with `kaksibadge==1`, a change to the second badge is not removed from `bdg_kno`. N-C8: `emitjarr[..][d]` is not cleared on delete, and slot reuse hands the link to the next competitor. N-C15: a stale mapping stays when status changes to P.
- **Name index.** N-C9: the `nameindex` key contains the series, but the index is updated only on a name change (HK) or a club/team change (relay).
- **Relay rank chains.** N-C11: relay series change with a different number of legs leaves split-time chain links in the old series.
- **H-109 (double decrement).** The mechanism lives in `deltall` itself (no `kilpstatus` guard). Besides H-109's own callers it is reachable from the list-based delete in `cbHk/UnitOsanottajat.cpp:1386` and `ViestiWin/UnitJoukkueet.cpp:1144`. I did not check `UnitOsanottajat.cpp:958` or `JukConsole/vcons2.cpp:1851` for guards.

Results impact (wrong rank, missing competitor, wrong header counts, heap overflow) involves multi-step reasoning in N-C1, N-C2, N-C3, N-C10 and N-C6. Those are marked "OPUS CHECK" below.

Self-healing: every derived structure here lives only in memory. `teejarj(false)` at program start rebuilds all of them from the records. Only the HK `teejarj(true)` stage change is a mid-run rebuild, and it has the N-C3 defect. Nothing in this part writes wrong derived data to disk. The exposure is "until restart".

Known findings that already cover related ground, cited by ID only: H-013 (`teejarj` loop `for (d = 1; i < nrec; i++)`, Hk/HkIx.cpp:1834), H-076 (`ossijat` buffer size), H-085, H-102 (`teejarj(true)` zeroes `idindex->ix` without the lock), H-109, H-114, T-035.

## 2. C1: structures, invariants, maintainers, symmetry

HK maintainers. `A` = `addjarjarr` (Hk/HkIx.cpp:800-1171). `R` = `remjarjarr` (:1173-~1462). `U` = `uusijarj` (:1514-1552). `T` = `teejarj` (:1554-1832). `V` = `vaihdavaihe` (:1855-2004). `Ta` = `kilptietue::tallenna` (Hk/HkDat.cpp, rem loop :1024-1027, add loop :1222-1229). `At` = `addtall` (Hk/HkDat.cpp:~689-732). `Dt` = `deltall` (:741-766).

| Structure | Invariant (current stage `k_pv`, non-deleted, `onMukana`) | Maintainers (file:line) | Symmetry of add/remove |
|---|---|---|---|
| `nilm[s]`, `nilmt` | number of non-deleted competitors in series s that are "mukana" (not P, unless `poissamukaan`) | A :843, :850. R :1213, :1217. U zero :1519-1535. T via A :1761. V zero :1913-1929. `At` and `Dt` via A/R | Symmetric. Same gate `ToimintaTila==2 && !onMukana` at :833 and :1202 |
| `nyilm[s]`, `nyilmt` | number of registered competitors (all, before the P gate) | A :816-819. R :1180-1184 | Symmetric, but never reset (N-C5) |
| `ntulos[s][0]` / `ntulost` | competitors with an accepted result `tulos_pv(ik_pv,true,1)!=0` | A :~940. R :~1290 | Symmetric by record state, not by chain membership. Counter is unconditional, chain removal is conditional |
| `ntulos[s][p]` (p>0), `nva[p-1]` | accepted split times | A :~1031. R :~1345 | Add counts when `hyv() && tulos2(p,1)!=0`. Remove counts only when found in the chain (`sj<9999`), so remove is chain-driven. OK |
| `nhyl`, `nkesk`, `neil`, `neilaika`, `nesit` and `nhylt`..`nesitt` | status counters from `tark(ik_pv)` | A :~950-990. R :~1280-1310. Tila 1 block in A :853-906, R :1215-1268 | Symmetric by state |
| `nvak[pv][s]` | vacant competitors (V) in stage pv | A :822-824 (V or P). R :1192-1194 (V only) | **Asymmetric (N-C4)** |
| `nilm_pv[pv][s]` | competitors present in stage pv, uses `Sarja(ipv)` and `luonne[ipv]>=0` | A :818-824. R :1188-1194 | Symmetric if `luonne`, `poissamukaan` and `yhtpiston` are unchanged between add and remove (L-3) |
| `nytulost`, `nvakt` | yhteistulos and V totals | A :1040, :1077. R :1371, :1400. `nvakt` has no reader | `nytulost` never reset (N-C5) |
| `jalku[s][lj]`, `jrjseur`, `jarjtulos` (rank chains) | chain lj of series s contains exactly the records counted in the matching counter, in key order | A, R. T resets `jalku` to -1 at :1655-1657. U resets `jrjseur`/`jarjtulos` only (:1536-1546) | Remove walks the chain by record identity (`entsija`), so it does not depend on the key. It depends on the state-derived `tllj` matching the state at add time |
| `pkarki[s][p]` | key of the head of chain 0 (or `valuku+5`) | A :1023, :1160. R :1348-1357, :1441-1449 | Set on head change only. Not reset by `teejarj` or `vaihdavaihe` (stale value harmless when the chain is empty) |
| `yhdsija` | rank within combined series | `sijat2` (Hk/HkTulostus.cpp:1212), T memset :1648 | not analysed in depth |
| `idindex` (`getpos`, `setpos`) | bib -> record position, one live entry per bib | `setpos` :205-226 (callers: `At` :720, `Dt` :760, `Ta` :1157/:1180, T :1758, `luo_posarr` :319, :334). Allocated T :1600. Zeroed T :1610. Freed :1505 | **Broken (N-C1, N-C2, N-C3)** |
| `nameindex` | name+series key -> record | T :1593 (build, only `!uusinta`). `At` :723. `Dt` :763. `Ta` :1030-1038. Key `keyaak` :527-538 | Keys come from the same function, but the update trigger is name only (N-C9) |
| `ixarr` indexes (sarja, seura, lahto, ...) | rebuilt on demand | `teeindeksi` :243-290 rebuilds fully each call (`reindex=1` is set at :250 before the "already built" test :254-255, so the cache never hits) | Not incrementally maintained. GUI lists keep their own `DKilp[]` copies |
| `bdg_kno[]`, `nbadge` | sorted badge -> bib, one entry per (badge, competitor), P status excluded at build | T build :1764-1774, dedupe :1788-1810. `addbadge` (Hk/HkEmit.cpp:1034-1094). `rembadge` :1096-1156. Callers `At` :726/:728, `Dt` :752/:754, `Ta` :1124-1218, `vaihda_badge` :1158-1190, Hk/HkIV.cpp:1268 | Not symmetric: N-C6, N-C7, N-C15. Weak spot in `rembadge` (L-4) |
| `emitjarr[0/1][d]` | record position d -> emit record number | `setEmitJarr` :1016-1022. `addem` :1671. `lueemitfile` :3093. `Ta` :1134/:1136/:1167/:1171/:1187. `T` reset :1625-1642 | Never cleared on delete (N-C8) |

Relay maintainers. Files: Juk/VIx.cpp (`addjarjarr` :728, `remjarjarr` :876, `paivJoukkLkm` :654-659, `teejarj` :1062-1380), Juk/vdat.cpp (`tallenna` :470-855, `addtall` :858, `deltall` :907).

| Structure | Invariant | Maintainers | Symmetry |
|---|---|---|---|
| `nilm[s]`, `nilmt`, `nilmtt` | teams per series | `paivJoukkLkm` callers: vdat.cpp:742-743 (dead, N-C10), :898 (`addtall`), :927 (`deltall`), VIx.cpp:1294 (`teejarj`). Also VIx.cpp:1173 and `memset(nilm)` :1288 | Broken on series change (N-C10) |
| `ntulosix[s][os][p]`, `ntulos`, `nkesk`, `nhyl`, `neil`, `ntulost[os]` | per leg | A :~790-816. R :~920-950 | Symmetric by state. N-C12 for the `valuku+1` block |
| `navoin`, `navoint`, `nekoodi`, `nylahto`, `nesit` | piste 0 only | A :755-775. R :~890-905 | Symmetric |
| `jalku`, `jrjseur`, `jarjtulos/jarjostulos`, `josalku` | rank chains | A, R, `teejarj` VIx.cpp:1239-1240, `setJosAlku` vApu.cpp:300 | Rem/add loops in vdat.cpp:616 vs :759 (N-C11) |
| `kilpnoix` (bib -> record) | direct array | vdat.cpp:64-98 | Direct assignment, no stale-entry problem |
| `nameindex` | key = club + series + team number (`keyname` Juk/vApu.cpp:180-188) | `teejarj` VIx.cpp:1177, `addtall` vdat.cpp:900ff, `deltall` :929, `tallenna` vdat.cpp:731-735 | Trigger omits series (N-C9) |
| `bdg_kno[2]`, `nbadge[2]`, `emitjarr[os][d]` | per badge slot and leg | `addbadge` Juk/VEmit.cpp:1101, `rembadge` :1139, `setEmitJarr` :1440, `teejarj` VIx.cpp:1092-1122, :1190-1245, :1305-1358 | emitjarr indexed by record position d, same slot issue as HK (N-C8) |
| `nollaayhdlkm` (Juk/vApu.cpp:560-565) | resets combined-series counters | no caller found | dead (L-2) |

## 3. Call-site tables

### 3.1 Who calls the rank-chain / counter pair (HK)

| Event | Path | rem before change? | add after change? | Notes |
|---|---|---|---|---|
| Startup | `teejarj(false)` → A, Hk/HkIx.cpp:1761 | n/a | yes, piste 0..`valuku[k_pv]` where `p_aika(piste,1)` | `keskeyta` ignored (L-7) |
| Stage change | `vaihdavaihe` → counters zeroed :1913-1929 → `teejarj(true)` :1935 | n/a | yes | N-C3, N-C5 |
| Series edit (Tila 1 only) | cbHk/UnitSarja.cpp:348, :402, :421, :428 → `vaihdakilpsrj(..., true)` → `uusijarj` (Hk/HkSrj.cpp:897-915) | no (counters re-zeroed) | yes | N-C14 |
| Add competitor | `At` → A piste 0 only (Hk/HkDat.cpp:724) | n/a | piste 0 only | N-C13 |
| Delete competitor | `Dt` → R for piste 0 and for p with `p_aika(piste)` (:757-758) | yes | n/a | no `kilpstatus` guard (H-109) |
| Edit | `Ta` rem loop :1024-1027, add loop :1222-1229, same condition `pak_ix \|\| kcmp>3 \|\| m_ajat[p+1] \|\| (!p && (m_ajat[valuku+2] \|\| m_ajat[valuku+3]))` | uses stored record (`GETREC(d)` at :~965) | uses `this` | Symmetric trigger. Only the series/valuku bounds differ (old series for remove, new for add), which is correct |
| Draw | cbHk/HkArv2.cpp:178-179 (`change`), :393, :690, :733: `tallenna` in Tila 2, `PUTREC(…,1)` in Tila 1 | n/a | n/a | In Tila 1 chains are unused, so fine |
| Series number shift | Hk/HkSrj.cpp:908 `PUTREC` of all competitors | no | `uusijarj` after (if `indeksoi`) | Tila 1 only (buttons disabled in Tila 2: cbHk/UnitSarja.cpp:308-315) |

### 3.2 Who writes `kilparr` directly (HK), without `tallenna`

| Site | What changes | Derived structures affected | Verdict |
|---|---|---|---|
| Hk/HkEmit.cpp:1181 (`vaihda_badge`) | `badge` | `bdg_kno` via `rembadge` (:1173/:1175) then `addbadge` (:1184) | Paired, OK (see L-4) |
| Hk/HkIV.cpp:1265 | `badge[0]` set to a free badge | `addbadge` :1268 | Paired, OK |
| Hk/HkSrj.cpp:908 | series | `uusijarj` if `indeksoi` | Tila 1 only, N-C14 |
| Hk/HkTulostus.cpp:1220 (`sijat2`), :6118 | `sija` | none (derived value itself) | OK |
| Hk/HkXmlTulokset.cpp:1251 | `pisteet[0]` | not in chain keys unless `yhtpiston` | not analysed |
| cbHk/HkArv2.cpp:178, :393, :690, :733 | `tlahto`, bib | Tila 1: chains unused. Tila 2: `tallenna` used | OK |

### 3.3 Consumers that trust a derived value (C4)

| Consumer (file:line) | Trusted value | Failure when value is wrong |
|---|---|---|
| Hk/HkTulostus.cpp:1172-1173 `sijat2`: `new int[nilm[srj]]`, filled by walking the chains with no bound | `nilm[srj]` | Heap overflow when `nilm` is too small (e.g. after H-109 double decrement). H-109's consequence chain, not a new defect |
| Juk/VTulostus.cpp:960-965 `ossijat`: `malloc((nilm+1)*...)`, chain-walk fill, no bound (H-076 same buffer) | relay `nilm[srj]` | Overflow after N-C10 when 2 or more teams moved into a series |
| Juk/vApu.cpp:648-660 `merk_yht_lahdot`: `malloc(nilm*4)`, loop `i<nilm`, `d[i]` left uninitialised when the chain ends early | relay `nilm[srj]` too high (old series after N-C10) | `d[i]` is heap garbage, then `GETREC(&kilp, d[i])`; relay `GETREC` ignores a bad index and leaves the stack record unset (T-035). Low |
| cbHk/HkArv2.cpp:706 `arvonumerot`: `nn = nilm_pv - nvak - 1`, `malloc(nn*4)`, loop `while (ok && !memcmp(key, srjswab, 2)) jsatn[i++]` over the index, no bound | `nilm_pv`, `nvak` | Under-count (N-C4 inflates `nvak`) lets the loop read beyond `jsatn`. The `-1` may itself be off by one; not traced [tuntematon] |
| Hk/HkTulostus.cpp:6587, :6677 and cbHk/HkKisaLahtLuet.cpp:1687, :1769: `jarjtulos[0][0][jalku[srj][0]][0]` when `ntulos[srj][0] != 0` | `ntulos` ≠ 0 implies `jalku[srj][0] >= 0` | In Tila 1 `ntulos[srj][0]` is incremented by A without a chain. If these summaries can run in Tila 1, the index is -1 (heap underread). Reachability [tuntematon] |
| cbHk/TulosUnit.cpp:1116, :1130, :1559; cbHk/UnitArvonta.cpp:153, :170 | `nilm_pv - nvak > 0` decides whether a series is listed or drawn | N-C4 inflation hides or mis-sizes a series |
| Hk/HkTulostus.cpp:4847 `n = nameindex.nrec`, name-order walks via `nextix(&nameindex)` (:4880) | `nameindex` entries live | Dangling entries (N-C9) point at deleted records. Whether the printing code checks `kilpstatus`: [tuntematon] |
| Walks by number: Hk/HkIV.cpp:1914; Hk/HkTulostus.cpp:2548-2574; cbHk/UnitArvonta.cpp:223-231; cbHk/UnitExport.cpp:346; cbHk/UnitSakkoKierr.cpp:68-82; cbHk/UnitKilpailijatiedot.cpp:1121, :1180; cbHk/UnitKilpailijaOnline.cpp:618, :676 | `getpos(x, true)` / `nextpos()` yield all live competitors in bib order, loop ends at `d <= 0` | Truncated after any stale `key -> 0` entry (N-C1) |
| Hk/HkEmit.cpp:1089, :1141 | `getEmitJarr(kno,&ibdg)` truthy means "has emit" | Returns -1 for "none", and -1 is truthy; position 0 is falsy. Inverted test, not integrity (L-5) |

## 4. Findings

Severity is high/medium/low. Confidence tags: [todennettu] (code logic read and checked), [paatelty] (inferred), [tuntematon] (not traced).

### N-C1 `setpos` never replaces or deletes an entry, so stale entries break every walk by number
- **Claim.** In `setpos` the condition around `delix` requires `!ret && d == dd`, but the enclosing `if (ret || d != dd)` allows `!ret` only when `d != dd`. So `delix` is unreachable and every change calls `addix(idindex, key1, d)`. `addix` inserts the new entry before older entries with an equal key. The old entry stays.
- **Evidence.** Hk/HkIx.cpp:216-221; tputilv2/dbindex.cpp:51-71 (`addix` inserts at the first-equal position returned by `bsrch`, :168-175); `getpos` reads the first match (:163-181). `Dt` calls `setpos(kilp.id(), 0)` at Hk/HkDat.cpp:760; `Ta` at :1180.
- **Trigger.** Delete a competitor, or change a bib, in Tila 2. The index then holds `key -> 0` followed by `key -> oldd`.
- **Consequence.** `getpos(key)` correctly returns -1, but `nextpos()` returns the entry value unfiltered (Hk/HkIx.cpp:186-193). A walk `d = getpos(minkilpno, true); while (d > 0) {...; d = nextpos();}` stops at the first stale `key -> 0` entry, skipping competitors with higher bibs. Affected walks are in section 3.3 (export by number, draw, penalty loops, meri3 points, number browse). `idindex->nrec` grows by 1 per delete and 2 per bib change (capacity: N-C3).
- **Self-healing.** Restart.
- **Severity/confidence.** high. [todennettu] for the unreachable `delix` and entry order; [paatelty] for the end-to-end result. OPUS CHECK: confirm `bsrch` returns the first equal element and `addix` inserts there, so the new `key -> 0` precedes the old entry; confirm `nextix` walks in array order.
- **Verification.** Harness: `setpos(105,7); setpos(106,8); setpos(105,0);` then `getpos(1,true)` and `nextpos()` ×3. Manual: delete a mid-range competitor in Tila 2, then export by number.

### N-C2 `setpos` skips the add when the successor entry has the same record number
- **Claim.** `if (ret || d != dd)`: when the new key is absent and not above all keys, `searchix` returns the next-greater entry's value in `dd`. If that entry is the old bib of the same record, `d == dd` and `addix` is not called.
- **Evidence.** Hk/HkIx.cpp:216 with `searchix` semantics (tputilv2/dbindex.cpp:108-118: `ok` cleared only when the key exceeds all keys; the key buffer is overwritten with the found or next-greater key, hence the `key`/`key1` comparison). Call in `Ta` at Hk/HkDat.cpp:1157, then `setpos(kilp.id(), 0)` at :1180.
- **Trigger.** Change bib 61 to 60 when 60 is free and no live or stale key lies between 60 and 61.
- **Consequence.** `getpos(new bib)` returns -1. In Tila 2, `tee_aika` and other bib lookups fail. `setEmitJarr(ep, id(), ibdg)` (Hk/HkDat.cpp:1136, :1171) silently does nothing (Hk/HkEmit.cpp:1019). `addbadge(ubadge, id())` (:1173, :1218) returns 1 because `getpos(kno) <= 0` (Hk/HkEmit.cpp:1041). The record on disk has the new bib, so a restart is consistent.
- **Self-healing.** Restart, or another bib change that avoids the condition.
- **Severity/confidence.** medium-high. [todennettu] for the code logic; trigger frequency [tuntematon]. OPUS CHECK: trace `bsrch` for a key between two existing keys.
- **Verification.** `setpos(61,7); setpos(100,8); setpos(60,7);` then `getpos(60)`.

### N-C3 `teejarj(true)` does not reset `idindex->nrec`
- **Claim.** On stage change `teejarj(true)` zeroes the buffer but leaves `nrec` (Hk/HkIx.cpp:1610). `luo_posarr` resets it (:314). The following `setpos` loop (:1758) adds n entries after `nrec_old` zero-key entries.
- **Evidence.** Hk/HkIx.cpp:1608-1611, :1600 (allocation `maxrec+2` records of 8 bytes), :1758; `addix` has no capacity test. `maxrec` is computed only in the `!uusinta` branch (:1571-1577): `max(filelen + n_lisaa, 3000)` capped by `maxkilpno` (`n_lisaa` default 1000, Hk/HkGlobals.cpp:394; `maxkilpno` 9999, Hk/HkDef.h:28).
- **Trigger.** HK Tila 2 `vaihdavaihe` (cbHk/WinHk.cpp:2080/:2163 or remote request). Overflow when `nrec_old + n > maxrec + 2`; with `nrec_old ≈ n`: n > ~1500 (maxrec = 3000) or n > ~1000 (when `filelen + 1000 > 3000`).
- **Consequence.** `addix` writes past the `calloc` block by `(nrec_old + n - maxrec - 2) * 8` bytes (about 8 KB for n = 2000). Heap corruption or crash during the stage change. Below the threshold the index works but `nrec` grows each stage change.
- **Self-healing.** Restart cures the index; memory damage does not heal.
- **Severity/confidence.** high. [todennettu] that `nrec` is not reset and capacity is `maxrec+2`; [paatelty] that real competitions exceed it. OPUS CHECK the thresholds. H-102 covers the missing lock at the same line, not this.
- **Verification.** Stage change on a 2000-record test competition under a heap checker, or read `idindex->nrec` before and after in a debugger.

### N-C4 `nvak` incremented for V and P, decremented only for V
- **Claim.** Hk/HkIx.cpp:822-824 `nvak++` for `'V' || 'P'`; :1192-1194 `nvak--` only for `'V'`.
- **Trigger.** A P competitor who counts as present (`onMukana` true: `poissamukaan >= 4`, or Tila 2 with `poissamukaan > 0` and `(yhtpiston & 6) == 6`, :47-51) is removed, changed or deleted.
- **Consequence.** `nvak` stays high; `nilm_pv - nvak` too small. Used by the draw (cbHk/HkArv2.cpp:706, 1179; cbHk/UnitArvonta.cpp:153, 170) and list filters (cbHk/TulosUnit.cpp:1116, 1130, 1559). A series can drop out of lists.
- **Self-healing.** Restart.
- **Severity/confidence.** low-medium. [todennettu] asymmetry; frequency [tuntematon].
- **Verification.** Unit test with `poissamukaan = 4`: add P, remove P, assert `nvak == 0`.

### N-C5 `nyilmt`, `nytulost` (and `nvakt` in `uusijarj`) never reset
- **Claim.** `vaihdavaihe` zeroes `nilmt`..`nvakt` (Hk/HkIx.cpp:1913-1919) but not `nyilmt`/`nytulost`. `uusijarj` zeroes only `nilmt`, `ntulost`, `nhylt`, `nkeskt`, `neilt`, `nesitt` (:1519-1524). Only `++`/`--` writers exist (HkIx.cpp:819, 1040, 1077, 1184, 1371, 1400).
- **Consequence.** After a stage change `nyilmt` = old + new, `nytulost` likewise. `nyilmt` shown in cbHk/HkKisaLahtLuet.cpp:1846, :1895 and cbHk/UnitKilpSeurat.cpp:301; `nytulost` in Hk/HkTulostus.cpp:6704, cbHk/HkKisaLahtLuet.cpp:1795, and gates cbHk/UnitPistelasku.cpp:102. After a series edit `nyilmt` doubles. `nvakt` is never read.
- **Self-healing.** Restart. **Severity/confidence.** low. [todennettu] missing reset; display consequence [paatelty].

### N-C6 HK `tallenna`: with two badges, a bib change fails to remove the second badge
- **Claim.** The id-change block is inside `for (ibdg...)`. Iteration 0 ends with `setpos(kilp.id(), 0)` (Hk/HkDat.cpp:1180). In iteration 1 `rembadge(ebadge[1], kilp.id(), ...)` (:1165) starts with `getpos(kno) <= 0 -> return 1` (Hk/HkEmit.cpp:1101-1103) and does nothing. `addbadge(ubadge[1], id())` (:1173) then finds the old entry under another bib and returns 1.
- **Trigger.** `emitfl`, `kaksibadge != 0`, competitor with a second badge, bib change.
- **Consequence.** `bdg_kno` maps badge 2 to the OLD bib; a read of badge 2 resolves via `bdg2kno` to a bib `getpos` no longer knows (or that is reused).
- **Self-healing.** Restart. **Severity/confidence.** medium. [paatelty]. OPUS CHECK the interplay with `kilp.id()` after the first iteration.
- **Verification.** `kaksibadge=2`, two badges, change bib, call `bdg2kno(badge2)`.

### N-C7 HK `tallenna`: with `kaksibadge == 1` a change of the second badge never removes the old entry
- **Claim.** The remove step is guarded by `(ibdg == 0 || kilpparam.kaksibadge == 2)` (Hk/HkDat.cpp:~1051-1052); the final add loop (:1214-1219) has no such guard. `kaksibadge == 1` is set by `KAKSITUNN` (Hk/HkInit.cpp:2016-2017) or `*2` in XML (Hk/HkXml.cpp:505).
- **Consequence.** New second badge added, old one stays mapped. Clearing the second badge leaves its old entry. If the tag is reused, `addbadge` reports "same e-code" (record already saved with the duplicate).
- **Self-healing.** Restart. **Severity/confidence.** low-medium. [paatelty]; may be a deliberate limitation.

### N-C8 `emitjarr[..][d]` not cleared on delete; slot reuse gives the new competitor the old emit link
- **Claim.** `emitjarr` is indexed by record position `d` (Hk/HkEmit.cpp:996-1022; Hk/BuildNo.h:77). `Dt` and `rembadge` do not call `setEmitJarr(-1, ...)`. `addrec` reuses the freed record (free list per H-109 text); `At` does not clear `emitjarr`. Relay is the same: `emitjarr[os][d]` (Juk/VEmit.cpp:1427-1447), `deltall` (Juk/vdat.cpp:907-939).
- **Trigger.** Delete a competitor with emit data, then add one that lands on the same slot.
- **Consequence.** `getEmitJarr(newkno)` returns the old emit record; `getem` reads it with no `kilpno` check (:1587-1598). The new competitor appears to have an emit card (`vaihdakilp` refuses; HkIV.cpp:1224, 1285, 1421, 1550).
- **Self-healing.** Restart (`lueemitfile` re-links by the emit record's `kilpno`). **Severity/confidence.** low-medium. [paatelty].

### N-C9 `nameindex` key contains the series but the update trigger is name-only (HK) or club/team-only (relay)
- **Claim.** HK: `keyaak` = name (30 bytes) + `Sarja(arv_pv)` (2 bytes) (Hk/HkIx.cpp:527-538); `Ta` re-keys only on `sukunimi`/`etunimi` change (Hk/HkDat.cpp:1030-1038). Relay: `keyname` = club + series + team number (Juk/vApu.cpp:180-188); `tallenna` re-keys only on `seura`/`joukkue` change (Juk/vdat.cpp:731-735).
- **Trigger.** Series change, then later a name/club change or delete. `delix` computes the key with the new series, fails silently (tputilv2/dbindex.cpp:73-106). HK extra: the key depends on global `arv_pv`, changed by draw code (cbHk/HkArv2.cpp:609, :692, :1041, :1219); matters only with `sarjavaihtelee`.
- **Consequence.** Duplicate or dangling `nameindex` entries. Name-order lists and name browse (HkTulostus.cpp:4847-4880, cbHk/UnitEmit.cpp:2073-2206, cbHk/UnitArvonta.cpp:244-254) can show a competitor twice or a deleted one. `nrec` can in theory exceed the `maxrec+1` allocation.
- **Self-healing.** Restart. **Severity/confidence.** low-medium. [todennettu] key composition and trigger; [tuntematon] whether list code skips `kilpstatus != 0`.

### N-C10 Relay `tallenna`: series-change update of `nilm`/`nilmt`/`nilmtt` is dead code
- **Claim.** Juk/vdat.cpp:738 `kilp = *ekilp;` then :742 `if (kilp.sarja != ekilp->sarja)` can never be true. Even if it were, the arguments are inverted. `paivJoukkLkm` has no other caller on the edit path (vdat.cpp:742-743, :898, :927; VIx.cpp:1294).
- **Trigger.** Series change of an existing team (ViestiWin/UnitJoukkuetiedot.cpp:584-591, saved via `Kilp.Tallenna` :527) or H-114's silent `sarja = 0`.
- **Consequence.** `nilm[old]` too high, `nilm[new]` too low until restart; `nilmtt` also not updated. Used in headers and open counts: Juk/VTulostus.cpp:4522, 6485-6579; `avoinLkm` Juk/VIx.cpp:707. Buffers sized from `nilm`: Juk/VTulostus.cpp:960-965 (`ossijat`, H-076 buffer), also :818, :4704-4706. The chain walk includes the moved team, so the chain is longer than `nilm`: one moved team fits the `+1` slack; two or more overflow. `merk_yht_lahdot` (Juk/vApu.cpp:648-660) with an over-counted old series leaves `d[i]` uninitialised.
- **Self-healing.** Restart. **Severity/confidence.** medium. [todennettu] dead branch; [paatelty] overflow. OPUS CHECK allocation sizes vs fill for `monios` and `maxnosuus>1`.
- **Verification.** Move a team to another series in a Tila 2 relay competition, print the new series result list under a heap checker, or inspect `nilm[]` before and after.

### N-C11 Relay `tallenna`: series change with different leg count leaves split-time chain links in the old series
- **Claim.** `kilpcomp(ekilp, entkilp)` sets `erot.os[os]` flags only for `os < Sarjat[ekilp->sarja].osuusluku` (new series, Juk/vdat.cpp:311-326). The rem loop iterates `os < Sarjat[kilp.sarja].osuusluku` (stored, old series, :585-619) and removes `va > 0` only when `tulosvaik`/`aika[va]` is set (:611-616). Legs beyond the new count have no flags.
- **Trigger.** Team moves from a series with more legs to one with fewer.
- **Consequence.** The team stays linked in `jalku[old][os][4+va]` for the removed legs and `ntulosix`/`ntulos[old][os][va]` keep counting it. `va==0` entries are removed unconditionally.
- **Self-healing.** Restart. **Severity/confidence.** low-medium. [paatelty]. OPUS CHECK whether `aika[va]` can be set beyond the new count and whether the GUI allows the move in Tila 2.

### N-C12 Relay combined series: `ntulosix`/`ntulos[own][os][valuku+1]` counted once per `yhd` iteration
- **Claim.** The `ToimintaTila == 2 && piste == 0` block in `addjarjarr` (Juk/VIx.cpp:~832-872) is inside `for (yhd...)` and uses `kilp->sarja` rather than `srj` (:841-842). With `nsarjayhd > 0` and the series in a combined series the count is incremented repeatedly. `remjarjarr` mirrors it (:996-997).
- **Consequence.** While the team is present, "x of n" figures are too high (ViestiWin/OsuusTiedot.cpp:162, ViestiWin/TulosUnit.cpp:320, `n` bound Juk/VTulostus.cpp:5868).
- **Severity/confidence.** low. [paatelty]; display only.

### N-C13 HK `addtall` adds piste 0 only; `teejarj` and `deltall` handle all points
- **Claim.** `At` calls `addjarjarr(this, *dataf, 0, ...)` once (Hk/HkDat.cpp:724). `teejarj` loops `piste <= valuku[k_pv]` (Hk/HkIx.cpp:1759-1761), `Dt` too (:757-758).
- **Consequence.** A competitor added with split times already set (comm receive `kaikki==1`, Hk/HkCom32.cpp:296-303; import) is missing from split-time rankings and `ntulos[s][p]` until a later `tallenna` flags those points or a restart. Removal is safe (`sj==9999`).
- **Severity/confidence.** low. [paatelty]; whether any add path carries split times [tuntematon].

### N-C14 `uusijarj` does not reset `jalku`/`pkarki`; `vaihdakilpsrj` can set series -1
- **Claim.** `uusijarj` resets `jrjseur` and `jarjtulos` but not `jalku` (reset only in `teejarj` :1655-1657) or `pkarki` (Hk/HkIx.cpp:1514-1552). Separately, series delete in cbHk/UnitSarja.cpp:346-348 shifts later series down and calls `vaihdakilpsrj(SarjaNo, sarjaluku-1, -1, true)`: competitors of the deleted series move to the previous series, or to -1 if `SarjaNo == 0`. `uusijarj` then reads `Sarjat[kilp.Sarja()].valuku[k_pv]` (:1548) before `addjarjarr`'s `Sarja() < 0` return.
- **Consequence.** Only reachable in Tila 1 (series buttons disabled in Tila 2, cbHk/UnitSarja.cpp:308-315), where chains are not built (`addjarjarr` returns for `piste != 0`, :808-809). So the `jalku` omission is latent; the -1 case is an out-of-range read of `Sarjat[-1]`.
- **Severity/confidence.** low. [paatelty] latency; [tuntematon] for the -1 case (did not confirm `setSarjaAll(-1)` stores -1). Overlaps H-114's area.

### N-C15 HK badge map keeps a competitor's badge after status changes to P
- **Claim.** `teejarj` excludes P competitors from `bdg_kno` (Hk/HkIx.cpp:1764-1774). A runtime change to P in `tallenna` triggers no `rembadge` (badge block only on badge difference, Hk/HkDat.cpp:1051). `addbadge` returns for P (Hk/HkEmit.cpp:1044-1045) but only when called.
- **Consequence.** An absent competitor's badge still resolves until restart; reuse of the tag conflicts. After restart it is free.
- **Severity/confidence.** low. [paatelty].

## 5. Latent and dead code

| ID | Item | Evidence | Note |
|---|---|---|---|
| L-1 | `uusijarj` omits `jalku`/`pkarki` reset | N-C14 | latent, Tila 1 only |
| L-2 | `nollaayhdlkm` never called | Juk/vApu.cpp:560-565 (no caller by grep) | dead |
| L-3 | Add/remove predicates read configuration that can change in between: `Sarjat[].luonne[ipv]`, `poissamukaan`, `yhtpiston & 6` in `onMukana` (Hk/HkIx.cpp:47-51) | A :820, R :1190 | If a series dialog changes these while competitors exist, `nilm_pv`/`nilm`/`nvak` drift. Whether `luonne`/`yhtpiston` are editable in Tila 2: [tuntematon] |
| L-4 | `rembadge` default `kbdg = 0`: if the competitor holds neither badge (or both badges are equal) the first array entry is removed when it belongs to that bib | Hk/HkEmit.cpp:1099-1100, :1114-1129 | Reachable only with an inconsistent call; callers pass the competitor's own badge |
| L-5 | `getEmitJarr()` returns -1 for "none", used as a boolean | Hk/HkEmit.cpp:1089, :1141 | Not integrity; `laheta_emva` sent on the wrong condition |
| L-6 | `addjarjarr` `tllj = 999` (`!onMukana`) followed by `jalku[srj][tllj]` | Hk/HkIx.cpp:975-978, :1018-1019 | Unreachable today (Tila 2 returns at :833, Tila 1 at :906). If the gate changes, `jalku[srj][999]` is out of range |
| L-7 | `teejarj` and `uusijarj` ignore `keskeyta` | Hk/HkIx.cpp:1547, :1761 | A counter incremented without chain insertion goes unreported at startup. In `tallenna`, `keskeyta > 2` ends the program (Hk/HkDat.cpp:1368-1371); bits 1 and 2 do not |

Persistence note: none of these structures is written to disk. N-C2 can leave the in-memory index without the competitor while the record on disk is correct. The reverse (record changed on disk, derived structure not updated) is covered by the table in 3.2 (no live case found).

## 6. H-109's mechanism elsewhere
- The cause is that `deltall` (HK Hk/HkDat.cpp:741-766, relay Juk/vdat.cpp:907-939) does no `kilpstatus` check. A second call re-runs `remjarjarr` against the stale kilparr content (`delfrec` keeps the fields and only changes `kilpstatus`, Hk/HkDat.cpp:640-648), so counters drop twice, `setpos(id,0)` adds another stale entry (N-C1), `delix` fails silently and `rembadge` runs again.
- Callers without an upstream status check: cbHk/UnitOsanottajat.cpp:1386, ViestiWin/UnitJoukkueet.cpp:1144 (list rows via `DKilp[aktrow]` after a modal message box). Guarded callers: Hk/HkCom32.cpp:306-312 (`kilp.kilpstatus == 0 && kilp.kilpno == entno`), Juk/VCom.cpp:570-572 (`getpos(entno) == d`).
- Beyond H-109's own consequences: any undercount of `nilm` becomes a heap overflow in `sijat2` (HK) and `ossijat` (relay), per 3.3.

## 7. C3 summary: how each break shows up

| ID | Result impact | Self-healing |
|---|---|---|
| N-C1 | Walks by number stop early: export by number, draw, penalty loops, number browse | Restart |
| N-C2 | One competitor unfindable by bib; emit and badge links missing | Restart |
| N-C3 | Heap overflow at stage change, large events | No (memory damage) |
| N-C4 | Series hidden from lists or draw miscounted | Restart |
| N-C5 | Doubled totals in summary pages | Restart |
| N-C6, N-C7, N-C15 | Wrong badge-to-competitor mapping | Restart |
| N-C8 | New competitor appears to have emit data | Restart |
| N-C9 | Duplicate or dangling name entries | Restart |
| N-C10 | Header counts wrong; buffer overflow with 2+ moved teams | Restart |
| N-C11 | Old series split-time ranking lists a moved team | Restart |
| N-C12, N-C13 | Display figures; missing split-time rank | Restart |

## 8. Unresolved questions
1. N-C1/N-C2 need a small test of `setpos`/`getpos`/`nextpos` to confirm the `bsrch`/`addix` ordering. No code was run.
2. N-C3 thresholds depend on runtime `maxrec` (`n_lisaa` set by parameter, Hk/HkInit.cpp:1313) and whether `maxkilpno` is lowered. Needs a run or debugger.
3. Whether the summary routines at Hk/HkTulostus.cpp:6587 and cbHk/HkKisaLahtLuet.cpp:1687 run in Tila 1.
4. Whether name lists skip records with `kilpstatus != 0` (N-C9).
5. Relay N-C10: fill versus allocation for `monios` and `maxnosuus > 1` in `ossijat`; whether the GUI series combo can change a team's series in Tila 2 in practice.
6. N-C11: leg-count differences between series and how `aika[va]` is flagged beyond the new count.
7. `arvonumerot` (cbHk/HkArv2.cpp:700-733): `nn = nilm_pv - nvak - 1` against the walk length; suspected existing off-by-one, not traced.
8. The yhteistulos part of A/R (`valuku+5..+9` chains for `k_pv > 0`, Hk/HkIx.cpp:1085-1170, :1380-1460) was read once for add/remove symmetry only; chain-key (`tl[]`) construction not checked.
9. `ixarr` secondary indexes are rebuilt by GUI-side `teeindeksi` calls; stale-use cases not enumerated. Part D or the GUI audit should check GUI "A" references to `nameindex`/`ljarjindex`/`sarjaindex`.
10. `modif`, `tlsmuutos`, `sjfl`, `prior`, `fprior`, `maaliajat` (flags/dirty bits) not analysed for stage reset (part B scope).

## 9. Coverage statement
- **Read in full or in the relevant part.** HK: Hk/HkIx.cpp :40-330, :527-538, :670-1830, :1855-2004; Hk/HkDat.cpp :585-1380 (incl. `kilpcmp` :809-880); Hk/HkEmit.cpp :955-1190 and :1580-1700; Hk/HkTls.cpp :150-300 (`vert0`) and :1111-1180; Hk/HkSrj.cpp :895-915; tputilv2/dbindex.cpp (whole). Relay: Juk/VIx.cpp :630-1030 and :1062-1380; Juk/vdat.cpp :60-130, :280-470, :470-940; Juk/vApu.cpp :175-215, :540-680; Juk/VEmit.cpp :1101-1215, :1420-1450. Consumers: Hk/HkTulostus.cpp (1160-1235, 2540-2580, 4838-4890, 6560-6590), cbHk/HkArv2.cpp (:140-185, :360-400, :670-745), cbHk/UnitExport.cpp :330-375, cbHk/UnitArvonta.cpp :215-240, cbHk/UnitKilpailijatiedot.cpp :590-630 and :1110-1135, cbHk/UnitSarja.cpp :296-432, Juk/VTulostus.cpp (950-1100, 4515-4530, 5855-5885), selected ViestiWin lines.
- **Not read.** Relay `VIx.cpp` yhteistulos code after :1030 other than `teejarj`; Juk/VTulostus.cpp as a whole; HK `HkTulostus.cpp` rank walkers (`SeuraavaSrj`, `askellasarja` callers) beyond cited lines; `addem`/`lueemitfile` internals beyond the `emitjarr` writes; part D (locking) and C-6 matters.
- **Only partly checked.** `ixarr` secondary indexes; `modif`/`tlsmuutos`/`prior`; `yhdsija`; `pkarki` consumers.
- **Method limits.** All findings are from static reading under the single-threaded assumption. Nothing was executed or measured.

Files referenced (root `C:\dev\tulospalvelu\TPsource\V52`): `Hk\HkIx.cpp`, `Hk\HkDat.cpp`, `Hk\HkEmit.cpp`, `Hk\HkTls.cpp`, `Hk\HkSrj.cpp`, `Hk\HkTulostus.cpp`, `Hk\HkCom32.cpp`, `Hk\HkGlobals.cpp`, `Juk\VIx.cpp`, `Juk\vdat.cpp`, `Juk\vApu.cpp`, `Juk\VEmit.cpp`, `Juk\VTulostus.cpp`, `tputilv2\dbindex.cpp`, `cbHk\UnitSarja.cpp`, `cbHk\HkArv2.cpp`, `cbHk\UnitExport.cpp`, `cbHk\UnitOsanottajat.cpp`, `ViestiWin\UnitJoukkueet.cpp`, `ViestiWin\UnitJoukkuetiedot.cpp`.
