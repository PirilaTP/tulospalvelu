# C-8 step 1, part D: shared mutable state between threads, not covered by C-6

Read-only. No repository file was modified. Static analysis only; nothing was built or run. Paths are relative to `TPsource/V52/` unless they start with `doc/`. Windows-1252 sources were read through iconv.

## 1. Conclusion

I found 6 new suspected defects. Three are substantive and three are low.

| ID | One-line claim | Severity | Confidence |
|---|---|---|---|
| N-D1 | The 8-entry badge-time table `bdg_t[]` is read and modified by two threads with no lock (HK and relay). A torn or lost entry gives a wrong `em.time`, which shifts the derived split and finish times. | medium | [todennettu] no lock and two threads; [paatelty] consequence |
| N-D2 | Check-then-act on the emit read cursor `emithead2`: `addem` (comm thread) against `pros_emit` (consumer). One later card can be skipped by automatic processing. | medium-low | [paatelty] |
| N-D3 | `static int count` in `openlukijaTCP` is shared by all TCP readers. An offline reader inflates another reader's reconnect back-off to about 4 minutes. | medium | [todennettu] sharing; [paatelty] scenario |
| N-D4 | Static return buffers (`asakkoStr`, `osuuskoodi`, and similar) are used by a worker thread and the UI thread without a common lock. | low | [paatelty] |
| N-D5 | HkMaali GAZ output: `ingazThread` is set inside the new thread, so `gazTh` and `writegaz` can overlap on `gazline`/`gazline1`. | low | [paatelty] |
| N-D6 | In the GUI, `writeerror_w` and the related helpers use one global exchange struct `perr_ptrs2` for every thread. | low | [paatelty] |

D3 (flags without volatile or barrier) produced no finding with an observable consequence; see section 5.

Most candidates were rejected. Per-reader statics are indexed by `r_no` or `cn`. The print and format statics are serialised by `tall` inside `tulostasarja`. The remaining static-return helpers are only called from the main thread (section 6).

## 2. Thread and shared-state table (what was traced)

The thread entries are C-6's list. "Depth" is the number of call levels followed below the thread entry function.

| Thread (C-6) | Shared state examined | Depth | Result |
|---|---|---|---|
| `lue_regnlyThread` → `lue_regnly` → `lue_LUKIJA`/`lue_MTR`/`lue_EmitKello`/`lue_EMITAG` → `tall_emit`/`tall_regnly` → `hae_bdg_t`, `add_bdg_t`, `addem` | `static` arrays in the `lue_*` functions, `tall_emit` and `tall_regnly`; `bdg_t[]`; `emithead`/`emithead2`; `valo_on`; `pollaa` | 4-5 | N-D1. The other statics are indexed per reader (section 6). |
| `pros_bdg_t` → `tall_bdg_t` | `bdg_t[]`, `etls` ring | 2 | N-D1. `etls` is H-094. |
| `tarkcom` → `tark_emit` → `addem`; `tark_alku`, `tark_aika` statics | `emithead`/`emithead2`; per-`cn` statics | 3 | N-D2. The statics are indexed per `cn`. |
| main thread consumer (`pros_emit` via `NewCardHandler`; console `leimat`) | `emithead2`, `in_pros_emit` | 2 | N-D2 |
| `lue_regnlyThread` → `openlukijaTCP` (also `ZebraReader.cpp:934`, `openSirit2` `:2253`) | `static count` | 1 | N-D3 |
| `kirjSQLTh` → `kirjSQLrivi` → `addSQLpv`/`updateSQLpv` | `asakkoStr` static buffer | 3 | N-D4 |
| `taustaThr` → `tausta` → `writegaz`; `gazTh` | `gazline[]`, `gazline1`, `gazline0`, `ingazThread` | 2 | N-D5 |
| any worker → `writeerror_w` (cbTp) | `perr_ptrs2` | 1 | N-D6 |
| `LukumaaratTh` → `etHaku`, `autotulostus`, `prior` decay | `prior[]`, `httpbuf`, `eTParam` | 2 | Rejected or benign (section 6) |
| `vahaku`, `naytatulosTh`, `naytaluentaTh`, `file2emitThread` | `inhttpHaku`, `valo_on`, `bdg` | 1 | Rejected (section 6) |
| `tiedonsiirto*`, `luepakettiUDP`, `SendCopyThread`, `TCPrcvTh`, `UDPstreamrcvTh` | `buflah`, `lahetaMonitorille`, `luepaketti` statics | 1 | C-2 territory or indexed per `cn`; not re-analysed |

`viittaukset.csv` loses function attribution in many Tp/ and GUI files, as part A's finding 6 says. For the thread entries above I therefore read the source directly and used the CSV only to find candidate names.

## 3. Findings

### N-D1. `bdg_t[8]` badge-time table: unlocked read and modify from two threads (HK and relay)

**Claim.** `static bdg_t_tp bdg_t[N_BDG_T]` (N_BDG_T = 8) is a small table of `{badge, t}` entries. It is written by the consumer thread `pros_bdg_t` and read, and in HK also modified, by the reader threads. No lock covers it.

**Evidence.**
- Table: `Hk/HkIV.cpp:136-142` and `Juk/VIv.cpp:140-144`.
- Writer thread `pros_bdg_t`:
  - HK `tall_bdg_t` at `Hk/HkIV.cpp:753-781`. It reads the table at `:766-770`, does `memmove` at `:775` and writes entries at `:778-779`.
  - It is called at `:807`, after `LeaveCriticalSection(&etls_CriticalSection)` at `:805`.
  - Relay `tall_bdg_t` at `Juk/VIv.cpp:691-722`, with `memmove` at `:709`; called at `:741` after the `etls` lock is released.
- Reader thread (`lue_regnlyThread` → `lue_regnly` → `lue_LUKIJA`/`lue_EmitKello`/`lue_MTR` → `tall_emit`), in HK:
  - `tall_emit` is called at `Tp/TpLaitteet.cpp:911`, `:1098`, `:1248` and others.
  - It calls `hae_bdg_t` at `Hk/HkIV.cpp:1026` (LID_LUKIJA) and `:1078` (LID_EMITAG).
  - `hae_bdg_t` (`:855-875`) scans the table, does `memmove(bdg_t+k, bdg_t+k+1, ...)` at `:871` and sets `bdg_t[7].badge = 0` at `:872`, so it modifies the table.
- Reader thread, relay: `hae_bdg_t` at `Juk/VIv.cpp:794-808` is read-only; callers are `:1018` and `:1070`.
- The entries are produced by `add_bdg_t`, called from the reader thread (`Tp/TpLaitteet.cpp:494`, `:1055`). It queues to the `etls` ring, and `pros_bdg_t` polls every `Sleep(50)`.

**Why no C-6 lock covers it.**
- `etls_CriticalSection` protects only the ring head, tail and slots (`Hk/HkIV.cpp:796-805`, `:827-850`).
- `tall_bdg_t` and `hae_bdg_t` run outside it.
- H-094 concerns the creation and deletion of the queue, not this table.
- I found no mention of `bdg_t` or `hae_bdg_t` in `doc/arkkitehtuuri/*.md` or `rekisteri/`.

**Trigger.**
- A reader thread runs `hae_bdg_t` (card read) while the consumer thread inserts a different badge, for example when several runners finish close together.
- The interleaving is two overlapping `memmove` calls, or a stale index `i` in `tall_bdg_t` after `hae_bdg_t` has shifted the entries.
- The window is nanoseconds to microseconds per event, so the probability is low per card and rises with finish bursts.

**Consequence.**
- `em.time` is taken from the table. If an entry is lost, `em.time` silently falls back to `biostime`, the card read time.
- If an entry is overwritten or duplicated, `em.time` holds another badge time.
- Downstream:
  - `Hk/HkEmit.cpp:1758-1759` computes `em->maali = em->time - 10*(tlkja - ctrltime[i])`.
  - `Hk/HkEmit.cpp:3632` and `:3642` compute split times from `KSEK*em->time - SEK*ctrltime`.
  - A wrong `em.time` therefore shifts all derived times of that card.
  - The `on_emit` duplicate key `4000*em->time + badge` also uses it (`:1320`, `:1644`).
- Separate ordering issue: the consumer inserts asynchronously with up to 50 ms latency. If `hae_bdg_t` runs first, the lookup misses and uses `biostime`. This is a design race on a single table, not a corruption.
- Applies to HK builds with `timerfl` on (`add_bdg_t` returns when it is off) and to relay builds with the `lue_LUKIJA`/EMITAG readers.

**Severity:** medium. **Confidence:** [todennettu] for the missing lock and the two distinct threads; [paatelty] for the observable effect.

**To verify:**
- A two-thread stress probe in the style of the C-6 probes, hammering `add_bdg_t` (via the queue) and `hae_bdg_t` with distinct badges and checking the returned `t`.
- A trace of how `em.time` is used in LID_LUKIJA mode, to confirm which results it changes.

### N-D2. `emithead2` check-then-act between `addem` and the consumer

**Claim.** `addem` advances the consumer cursor with an unlocked `if (emithead2 == emithead-1) emithead2 = emithead;` for a record that is already processed. The consumer tests the guard `emithead2 >= emithead` without the emit lock, then increments `emithead2` without it. Interleaved, `emithead2` ends up greater than `emithead`, and one subsequent card is skipped by automatic processing.

**Evidence.**
- `addem`, HK: `Hk/HkEmit.cpp:1657-1688`. It holds `emit_CriticalSection` (`:1664`), does `emithead++` (`:1674`) and `fflush(femitfile)` (`:1675-1677`, active with the default `kirjheti = 0xffffffff` from `Hk/HkGlobals.cpp:350`), then the assignment at `:1679-1680`.
- `addem`, relay: `Juk/VEmit.cpp:1505-1525`, with an unconditional `fflush` between `emithead++` and the assignment (`:1519`).
- Consumer, HK: `pros_emit` (`Hk/HkIV.cpp:1184-1190`):
  - guard `in_pros_emit || emithead2 >= emithead || !ProsEmitOn` without a lock;
  - `lueem(&em, emithead2)`, whose lock is taken inside (`Hk/HkEmit.cpp:1579`) while the argument is evaluated before the call;
  - `emithead2++` at `:1189`, `:1332` and elsewhere.
- Consumer, relay: `Juk/VIv.cpp:1136-1141`.
- Callers of `addem` on a different thread from the consumer:
  - `tark_emit` at `Hk/HkCom32.cpp:790-798` and `Juk/VCom.cpp:821`, on `tarkcom`, with records forwarded by another station that already have `kirjaus` set (`com_emit[cn]==1`);
  - the reader thread at `Hk/HkIV.cpp:1150`, with `kirjaus` unset, so no cursor write.
- HK GUI consumer: `TFormEmit::NewCardHandler` calls `pros_emit` in the main thread (`cbHk/UnitEmit.cpp:936`).
- HK console consumer: the main loop at `Hk/HkEmit.cpp:3867` polls with `Sleep(100)`.

**Why no C-6 lock covers it.** The `emit` lock is held only inside `addem` and `lueem`. The cursor read, the guard and the increment in `pros_emit` are outside it. H-092 and H-105 concern other emit-state paths and do not mention `emithead2`.

**Trigger.**
1. The consumer is idle with `emithead2 == emithead == H`.
2. An `addem` call on `tarkcom` appends a processed record. `emithead++` makes `emithead == H+1`.
3. During the `fflush` window the consumer guard sees `H < H+1`, passes, and evaluates the `lueem` argument as `H`. It then blocks on the emit lock inside `lueem`.
4. `addem` sets `emithead2 = H+1`, leaves the lock, and the consumer reads record H (`kirjaus` set) and runs `emithead2++`, giving `H+2` with `emithead == H+1`.
5. The next local card is stored at index `H+1`, so `emithead == H+2`. The guard `emithead2 >= emithead` is true, so it is not processed.
6. The following card at `H+2` is processed, and `H+1` stays unprocessed.

**Consequence.** Automatic emit processing of one card is skipped (no result registered). The record stays in the emit file and can be processed manually. It needs a multi-station emit setup (`com_emit == 1`) and a coincidence with a consumer poll inside the `fflush` window.

**Severity:** medium-low (propose medium if multi-station emit forwarding is common at events). **Confidence:** [paatelty].

**To verify:** a two-thread probe against `addem` and `pros_emit` with an injected delay between `emithead++` and the assignment, or a test with two stations forwarding processed records.

### N-D3. `openlukijaTCP`: one back-off counter for all TCP readers

**Claim.** `static int count` (`Tp/TpComY.cpp:2123`) is shared by every reader thread.

**Evidence.**
- `openlukijaTCP` is called per reader from `lue_regnlyThread` (`Tp/TpLaitteet.cpp:1785`, one thread per reader), from `ZebraReader.cpp:934` and from `openSirit2` (`Tp/TpLaitteet.cpp:2253`).
- On the retry error codes (WSAEHOSTDOWN, WSAECONNREFUSED, WSAEINPROGRESS, WSAEWOULDBLOCK) it does `Sleep(5000)`, then `count++`.
- With `count > 5` it adds `Sleep(55000)`, and with `count > 10` a further `Sleep(180000)` (`:2171-2184`).
- Only a successful open resets it (`count = 0`, `:2188`).
- The `EI_OLE` block that writes `virhesanoma` is dead code (`#ifdef EI_OLE`, `:2167`).

**Why no C-6 lock covers it.** It is a local static, not a lock-protected structure. H-108 covers the static `acn` in `openSirit`, which is a different variable.

**Trigger.**
1. Two or more TCP readers (for example start and finish) are configured.
2. One stays unreachable (switched off for the day). Its thread drives `count` above 10 in a few minutes.
3. The other reader fails once, for example because the program started before the reader booted or after a network drop.
4. The very first failed attempt of that reader now waits 5 + 55 + 180 s = about 4 minutes before the loop retries.

**Consequence.** A healthy reader is not read for about 4 minutes (reads may be lost unless the device buffers them). A success on any reader resets the back-off for every other reader.

**Severity:** medium. **Confidence:** [todennettu] for the shared static; [paatelty] for the scenario.

**To verify:** a probe with two simulated readers that refuse connections, logging the delay each thread sees.

### N-D4. Static return buffers shared by a worker thread and the UI thread

**Claim.** Several formatters return a pointer to a function-local static buffer. The UI thread (and `kirjSQLTh`) call them without a common lock against the print path.

**Evidence.**
- `kilptietue::asakkoStr` uses `static wchar_t st[20]` and does `memset` then `wcscat`/`_itow` (`Hk/HkTls.cpp:877-895`).
- Worker callers:
  - `kirjSQLTh` → `kirjSQLrivi` → `addSQLpv` at `cbHk/UnitMySQL.cpp:389` and `updateSQLpv` at `:541`. It holds only `SQL_CS`, not `tall`; `kirjSQLrivi` is at `:444` and `kirjSQLTh` at `:645-722`.
  - The print path (`Hk/HkTulostus.cpp:784`, `:3116`, `:3539`, `:3867`, `:3938`, `Hk/HkXmlTulokset.cpp:426`, `:891`), where `tall` is held inside `tulostasarja` (`Hk/HkTulostus.cpp:5850-6518`).
- UI callers in the main thread: `cbHk/TulosUnit.cpp:940`, `cbHk/UnitASakot.cpp:263`, `cbHk/UnitIlm2.cpp:720`.
- The same pattern exists for `str_piste` (`Hk/HkApu.cpp:147`), `osuuskoodi`, `wosuuskoodi`, `str_piste` and `keyname` (`Juk/vApu.cpp:182-280`) and `kilptietue::yhteisotieto` (`Hk/HkTls.cpp:608`).

**Why no C-6 lock covers it.** The UI callers hold no lock, and `kirjSQLTh` holds a lock (`SQL`) that the other threads do not take. C-6 VCL thread safety (section 13.7) covers VCL components, not these buffers.

**Trigger.** The window is about a microsecond per call. The UI grid refresh must run while `kirjSQLTh` or the auto-print thread is inside `asakkoStr`.

**Consequence.** One wrong penalty string, in the MySQL mirror (`ampsakot`, persistent until the next update) or in one printed or exported result line. Rare.

**Severity:** low. **Confidence:** [paatelty].

**To verify:** a concurrency probe on `asakkoStr` with two threads and different `i_pv`.

### N-D5. HkMaali GAZ output: `ingazThread` is set late

**Claim.** `gazTh` sets `ingazThread = 1` as its first statement, inside the new thread, but `writegaz` starts it with `_beginthread` and keeps looping.

**Evidence.**
- `Hk/HkIV.cpp:3195-3227` (`gazTh`, with `ingazThread = 1` at `:3204`) and `Hk/HkIV.cpp:3230-3360` (`writegaz`).
- `writegaz` tests `if (ingazThread) return;` only at its entry. Inside the row loop it starts a thread with `if (!ingazThread) hgazThread = _beginthread(gazTh, ...)`, and the loop then continues.
- `gazTh` and `writegaz` both write `gazline[0]`, `gazline1` and `gazline0[0]`. `gazTh` always uses row 0 and ignores its argument; `gazluku` greater than 1 makes `writegaz` overwrite `gazline1` for the next row.
- `GAZ` is defined in HkMaali (`VS/Hk/HkMaali/HkMaali.vcxproj:118`); `N_PV = 5` (`Hk/HkDef.h:45-46`).
- The thread runs on `taustaThr` → `tausta` → `writegaz` (`Hk/HkIV.cpp:3642`).

**Why no C-6 lock covers it.** There is no lock; `gazTh` appears in the C-6 thread table only as "returns by itself".

**Trigger.** `gaz_com` set, `gazluku` greater than 1, `k_pv > 0` and `gazvaihe > 0`, with a GAZ display connected.

**Consequence.** A second `gazTh` can start, or the display line can be overwritten, so wrong or interleaved text goes to the GAZ display. No result data is affected.

**Severity:** low. **Confidence:** [paatelty].

**To verify:** a probe with a slow thread start, or a stub of `wrt_st_x` that logs the sequence.

### N-D6. GUI `writeerror_w`: one shared exchange struct

**Claim.** `writeerror_w`, `writewarning_w` and `Win_wselectopt` all use the single global `perr_ptrs2` (`cbTp/cbTpApu.cpp:394-397`).

**Evidence.**
- Each caller (`:467-490`, `:526-540`) writes `ots`, `msg` and `vast = 0`, then `SendMessage`s, then polls `while (perr_ptrs2.vast == 0) Sleep(200)`.
- The main-thread handler (`errorMsgHandler`, `:441-460`) reads `msg`/`ots`/`lparam` and writes `vast` after `ShowModal`.
- Worker callers are many (`tarkcom`, the final message of the reader thread at `Tp/TpLaitteet.cpp:1874`, `LukumaaratTh`, `kirjSQLTh`).

**Why no C-6 lock covers it.** No lock covers the exchange struct. C-6 section 13.7 notes that the modal loop processes other workers `SendMessage` calls; it does not examine this struct.

**Trigger.** A second worker calls `writeerror_w` while a dialog is open or a message is queued.

**Consequence.** The dialog text can be replaced before the handler reads it. A worker can spin in its `Sleep(200)` loop until another worker dialog is answered, because the other worker zeroed `vast` after the handler had set it. No data is affected.

**Severity:** low. **Confidence:** [paatelty].

**To verify:** a GUI test where two workers call `writeerror_w` within the same 100 ms.

## 4. Writer and reader thread summary for the new findings

| ID | Writer thread (file:line) | Reader or other writer thread (file:line) | Protecting lock today |
|---|---|---|---|
| N-D1 | `pros_bdg_t` → `tall_bdg_t` `Hk/HkIV.cpp:775-779`; relay `Juk/VIv.cpp:709-713` | `lue_regnlyThread` → `tall_emit` → `hae_bdg_t` `Hk/HkIV.cpp:862-872` (R and W); relay `Juk/VIv.cpp:799-805` (R) | none (the `etls` lock covers only the ring) |
| N-D2 | `tarkcom` → `tark_emit` → `addem` `Hk/HkEmit.cpp:1679-1680` | main thread `pros_emit` `Hk/HkIV.cpp:1184-1189` (guard and increment) | `emit` inside `addem`/`lueem` only; the guard and increment are unlocked |
| N-D3 | each `lue_regnlyThread` → `openlukijaTCP` `Tp/TpComY.cpp:2171-2188` | other `lue_regnlyThread` instances, same function | none |
| N-D4 | `kirjSQLTh` → `asakkoStr` `cbHk/UnitMySQL.cpp:389`, `:541` | main thread `cbHk/TulosUnit.cpp:940`, `cbHk/UnitASakot.cpp:263`, `cbHk/UnitIlm2.cpp:720` | `SQL` (writer only) or `tall` (print only); no common lock |
| N-D5 | `gazTh` `Hk/HkIV.cpp:3204-3225` | `taustaThr` → `writegaz` `Hk/HkIV.cpp:3240-3360` | none |
| N-D6 | any worker `writeerror_w` `cbTp/cbTpApu.cpp:467-490` | main thread `errorMsgHandler` `:441-460`, other workers | none |

## 5. D3: flags without volatile or barrier

I found no case with an observable consequence.

- **Loop shape.** Every loop that waits for another thread flag contains an external call: `Sleep`, `SendMessage`, `WaitCommEvent`, `recv`/`accept` or `httphaku`. This covers `LukumaaratTh`, `ProsEmitThread`, `taustaThr`, `lue_regnlyThread`, `DisplTimerThread`, `ScrollThread`, `PaivThread`, `SelTimerThread`, `writeerror_w` (`while (perr_ptrs2.vast == 0) Sleep(200)`) and `writewarning_w` (`while (FSwarning) Sleep(200)`). Within the compilers used here an opaque external call forces a reload of the flag.
- **Busy loops.** A grep for `while (...) ;` loops over `Hk/ Juk/ Tp/ com/ cbHk/ ViestiWin/ cbTp/ HkConsole/ JukConsole/ tputilv2/ dbboxm/` found no spin loop on a shared flag without a call in the body.
- **Torn multi-field state.** The only observable cases are the ones in N-D1 and N-D2. The `inhttpHaku` handshake (below) is a gap, not a memory-ordering problem.
- **`inhttpHaku` gap.** `vahaku` sets `inhttpHaku = 1` inside the thread (`Hk/HkIV.cpp:3513`, `Juk/VIv.cpp:2376`), not by the starter. The `while (inhttpHaku) Sleep(200)` wait in `lopetus1` (`Hk/HkInit.cpp:3803`) can therefore miss a just-started search. This is narrow and belongs to the H-038 class; not registered.

## 6. Rejected candidates

| Candidate | Why rejected |
|---|---|
| `tall_regnly` statics (`yritys`, `et`, `edaika`, `edtunn`, `ed_bdg`, `ed_t`, `edrivit`) | Indexed by `r_no` (`Hk/HkIV.cpp:1991-2009`), one thread per reader. The `r_no = -1` file path is SIIMPORT code that no project defines. |
| `lue_*` statics (`tviim`, `vahvistus`, `viimaika`, `viimTCPaika`, `in_lue_regnly`, `od`, `nmsg`, `vastaus`, `t_raja`, ...) | All arrays indexed by `r_no` (`Tp/TpLaitteet.cpp:861-956`, `:1131`, `:1304`, `:1518-1530`). |
| `tall_emit` statics (`uusin`, `ed_em`) | `uusin` is never written after its initialiser; `ed_em[r_no]` is per reader. |
| `tall_bdg_t::edbadge`, `add_bdg_t::edbadge` | Indexed by `r_no`; one consumer thread. |
| `pros_emit::uusin`, `edkno`, `in_pros_emit` | `pros_emit` runs in one thread at a time (main thread via `NewCardHandler`, or console main). The race is on `emithead2` (N-D2), not the statics. |
| `tausta` statics (`saika`, `st`, `ed_lkm_t`, `edlahkello`, `prev_t`, `edhaku`, ...) | `tausta` is called only from `taustaThr` (`Hk/HkIV.cpp:3715`, `Juk/VIv.cpp:2542`); in the HK GUI `taustaThr` does not run (console only). |
| `prior[]` writers (`LukumaaratTh` decay `cbHk/WinHk.cpp:1857`, `tausta` `Hk/HkIV.cpp:3702`, `tallenna` under `tall` `Hk/HkDat.cpp:1336-1341`, `autotulostus` under `autotul`) | The read-modify-write can lose one increment, which only delays an auto-print threshold. Aligned 32-bit `INT`, no torn value. Not worth a finding. |
| `gettm::caika`, `aikajonotp::getTime`/`getAktTime` static `caika` | Null-buffer calls are under `ajat` (`Hk/HkAjat.cpp:180`, `:185`) or in `initajat` (`Hk/HkIV.cpp:4102-4106`). The unlocked null-buffer callers are `Hk/HkCom32.cpp:452` (`od_fl`) and `Hk/HkIV.cpp:1781-1788` (`autokilp`), both rare options; I did not register them. |
| `str_piste` | The `HkAjat.cpp:136-148` callers feed only the diagnostic log (`wkirjloki`) and are under `ajat`; the UI callers (`cbHk/UnitAjanotto.cpp:283`, `:525`) only display. |
| `wkello()`, `wpaiva()`, `ISOdatetime()`, `stDateNo(NULL, ...)`, `wconcat()` | Concurrent writers write the same format and nearly the same content, so a torn read still looks like a valid time or date. Export paths are serialised by `tall` inside `tulostasarja`. Benign. |
| `aikatostr`, `aikatostr_l/_h/_t/_l2`, `aikatowstr_cols_n(NULL)` | Static-returning variants are called only from the main thread or console (`ViestiWin/UnitStatus.cpp:271`, `HkConsole/HkCons0.cpp:250`, `JukConsole/lasknum5u.cpp:225`, `:319`, `cbHk/UnitPistelasku.cpp:133`, `:179`). |
| `wcstokch` static `p2` | All callers are Form units on the main thread (`cbTp/UnitFIS.cpp`, `cbHk/LoppuKilpUnit.cpp`, `cbHk/UnitImport.cpp`, `ViestiWin/UnitMain.cpp`, `ViestiWin/UnitLisatiedot.cpp`). |
| `wretulos::edsija`, `htmlmakitulos::edtls`, `prtmakitulos::edtls`, `xmlfistulos::edtls` | Called only from `tulostasarja`, which holds `tall` from `Hk/HkTulostus.cpp:5850` to `:6518`; carry-over state is part B or C material. |
| `virhesanoma` | The writer in `Tp/TpComY.cpp:2169-2173` is under `#ifdef EI_OLE`; the consumer is in `LukumaaratTh` (`cbHk/WinHk.cpp:1838-1841`). Dead code. |
| `autoclose` | Never set non-zero anywhere (only cleared at `Hk/HkCom32.cpp:958`, `Juk/VCom.cpp:990`). Dead. |
| `on_emit::nemitvar` not reset by `on_emit(0,1)` | Not a thread issue and benign: leftover count over zero-filled entries only wastes capacity. Not registered. |
| `lahetaMonitorille` statics | Called only from the main thread (`cbHk/UnitStatus.cpp:268`, `ViestiWin/UnitStatus.cpp:207`) or console main. |
| `buflah` statics (`tmsg`, `esto`, `nj0`) | C-2 territory (`outb` lock commented out there). Not re-analysed. |
| `luepaketti`/`lahetapaketti`/`opencomTCP` statics | Arrays indexed per `cn`, or init-once; C-2 territory. |
| `valo_on`/`naytatulos` check-then-set | By design a drop of the second light signal; only a cosmetic result. |
| `naytaluenta` static `bdg` | Only the thread start reads it; console display. |
| `pollaa` flag | Console Alt-T flag consumed by whichever reader thread sees it first; cosmetic. |
| `in_aseta_mtr` | Single `int` protocol for all readers and `PaivThread` zeroing it; extends H-098. The MTR reader can be mid-read, and `aseta_mtr` does not wait for it. Operator-driven diagnostics only. |
| `inGDIdoc` | Check-then-set in `initdoc` (`Hk/HkTulostus.cpp:1689-1691`) is serialised by `print` in all callers. |
| `TFormSelect3` timer thread and `FormThis` | Already in C-6 section 13.6-13.7 (form lifetime). |
| `etHaku` statics (`inhttpVa`, `edhaku`, `keTserial`, `httpbuf`) | Called from `LukumaaratTh` (GUI) or `tausta` (console), each effectively one thread. The two simultaneous `LukumaaratTh` instances are the C-6 section 9 candidate. |
| `strtok` in `tulk_emiTag` (`Tp/TpLaitteet.cpp:326`) | MSVC CRT strtok is per-thread; thread-safety in the bcc32 RTL is unknown (open question 3). |
| `reconnectTCP` after `closeportTCP` | `closeportTCP` zeroes `*hC` (`com/WinUDP.cpp:349`), so `reconnectTCP` returns early (`yhtno = -1`); no crash path. |
| K1/K6 config structs (`autotulostusparam`, `autofileparam`, `lsttulparam`, `eTParam`) | Outside categories K2-K7 for this part; copying while the GUI edits is a config race, not tracked here. |

## 7. Unresolved questions

1. N-D1: does the LID_LUKIJA flow normally find the entry in `bdg_t` or fall back to `biostime` because of the 50 ms consumer latency? A timing trace with a real or simulated reader would answer this and tell how often `em.time` is table-derived.
2. N-D2: is `com_emit[cn]==1` (receive emit records from another station) used in real multi-station setups? I could not tell from the code.
3. Is the bcc32 RTL `strtok` per-thread? This affects `tulk_emiTag`, which is called from several reader threads and from the `etHaku` thread.
4. N-D3: do events normally run two or more TCP readers at once, and are both always connected?
5. The `aikajono` data (`rwtime`, `aktrows`, `aktrow`, `atime`) is read by GUI forms without `ajat` while `tall_rivi` inserts and deletes under `ajat`. I did not register this; the display-only read race is real but I found no persistent consequence, because edits look rows up by content (`tall_rivi(..., &pt, ...)` in `cbHk/UnitAjanotto.cpp:888-920`). A deeper review of the form edit paths is needed.
6. `kirjSQLrivi` does `kilp.GETREC(dKilp)` (`cbHk/UnitMySQL.cpp:455`) with no `kilpdat` or `tall` lock; this looks like a C-1 or H-018 item and I did not trace it.
7. The `pyyda_vaihdavaihe`/stage-change reads of `k_pv` by workers without `tall` are covered by C-6 Q30/H-102 and were not re-analysed.

## 8. Coverage statement

- **Threads covered.** `tiedonsiirto*`, `luepakettiUDP`, `lue_regnlyThread`, `tarkcom`, `pros_bdg_t`, `vahaku`, `taustaThr`, `gazTh`, `naytatulosTh`, `naytaluentaTh`, `file2emitThread`, `kirjSQLTh`, `LukumaaratTh`, `ProsEmitThread`, `SelTimerThread`, `PaivThread`. Not covered: `HaeAvoimetTh`, `haeSQLTh`, `ScrollThread`, `DisplTimerThread`, `NaytaTulosTimerThread`, `TCPrcvTh`/`TCPacceptTh`/`UDPstreamrcvTh`, `sendtcpfile` (C-6, C-2, C-7 or form-internal), and `comajanotto` (only the C-6 H-102 aspects).
- **Call depth.** Reader chain `lue_regnlyThread` → `lue_regnly` → `lue_*` → `tall_emit`/`tall_regnly` → `hae_bdg_t`/`addem`: 4-5 levels. `tarkcom` → `tark_emit` → `addem`: 3 levels. `pros_bdg_t` → `tall_bdg_t`: 2 levels. `kirjSQLTh` → `kirjSQLrivi` → `addSQLpv`/`updateSQLpv` → `asakkoStr`: 3 levels. Other chains: 1-2 levels.
- **Method.** The CSVs of part A (`luokittelu.csv`, `staattiset_funktiossa.csv`, `viittaukset.csv`) were used to enumerate K3/K7 statics and K4 flags, plus direct source reading for thread bodies; `viittaukset.csv` attribution is lossy in `Tp/` and GUI files, so the thread-entry references were confirmed by grep and reading. Function-local statics in `Hk/ Juk/ Tp/ com/ tputilv2/` were checked (the staattiset list), including the `lahetapaketti`/`luepaketti` cluster only for per-`cn` indexing; function-local statics in `cbHk/`, `ViestiWin/` and `cbTp/` were not enumerated (form-internal state is C-7), apart from `cbTp/cbTpApu.cpp`.
- **Not done.** No compilation, execution or probe. All severities and the N-D1/N-D2 consequence chains rest on static reading and should go to part E for independent review.
