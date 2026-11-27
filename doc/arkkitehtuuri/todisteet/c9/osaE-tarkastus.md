<!-- Provenance: final report of independent-verifier (Opus), C-9 step 1 part E, 2026-11-26; copied unmodified from the hand-back message by the main session. -->
# C-9 step 1 part E: independent review of the N-B, N-C and N-D claims

All of this is static reading of the source plus one small Win32 API test (FormatMessage, run from the scratchpad). No repository file was changed. Paths are relative to `C:\dev\tulospalvelu\TPsource\V52\` unless noted.

## Summary table

| # | Claim | Verdict | Severity proposed -> mine | Built in | Duplicate of | Key evidence (my own reading) |
|---|---|---|---|---|---|---|
| 1 | N-B1 emit write failure, card ACKed | VAHVISTETTU | medium -> medium | all (EMIT) | new; related H-011, H-105 | HkEmit.cpp:1657-1693 (`res` is 1 only from `on_emit`; a failed `putem` returns 0); `TALLEMIT` has no definition anywhere (grep of .h/.cpp/.vcxproj/.cbproj), block :1275-1377; HkIV.cpp:1145-1151 (`ed_em` set before `addem`, always `return(0)`); TpLaitteet.cpp:911-916 (ACK when `er==0`), :1248-1254 |
| 2 | N-B2 `writeajat` fp1++ before `_write`; `writeaika` dead | VAHVISTETTU | medium -> medium | all | new (not H-054) | HkAjat.cpp:366-375; vajat.cpp:345-356; `writeaika` only defined/declared (HkAjat.cpp:326, vajat.cpp:307, two headers) |
| 3 | N-B3 autofile truncation, WEOF, per-series failure | OSITTAIN | medium -> low-medium | all | new | WEOF part confirmed and wider (see notes). "Silent per-series failure" is wrong: D-12 is right (Wrttext.cpp:171-176) |
| 4 | N-B4 relay `addfrec` reopen | OSITTAIN | medium -> medium | ViestiWin, ViestiMaali | extends H-015, H-040 | vdat.cpp:178-191; dbbox1.cpp:273-276 (`filelen` = `numrec`, so every append reopens); :186-208 header check. Disk-full outcome is [inferred]. Not "silent": `putrec` raises `perr` first (dbbox1.cpp:91-96) |
| 5 | N-B5 no `_commit` | VAHVISTETTU | medium -> low | all | new | No `_commit`/`FlushFileBuffers` except Juk/vdat.cpp:157; WRITE_THROUGH at dbbox1.cpp:118, :180 |
| 6 | N-B7 FILESEND partial forward | VAHVISTETTU | medium-low -> low | all | extends H-071 | TpComY.cpp:3004-3033 (`pos` stays > 0 after close; the -2 packet forwards) |
| 7 | N-C1 HK `aflst` leak | VAHVISTETTU (HK); the "relay autofile not compiled" part is KUMOTTU | medium -> medium | HkMaali, HkKisaWin | new | HkTulostus.cpp:7147 inside `do{}while` (:7314), single `delete` :7315-7316; HkDef.h:53-55. Relay: ViestiWin.cbproj:134 (Base `<Defines>AUTOFILE`), JukMaali520/JukLuenta520 define AUTOFILE |
| 8 | N-C2 dangling `aftulparam.lstf` after `sendtcpfile` | VAHVISTETTU | medium -> medium | HkMaali, HkKisaWin (TCPSIIRTO) | new | HkTulostus.cpp:7060 (frees, does not NULL); :7093 guard; :7171-7174, :7272-7275, :7317-7319 use the stale pointer |
| 9 | N-C3(a) `openportTCP` socket leak | VAHVISTETTU (mechanism); rate is [inferred] | medium -> low-medium | all | extends H-027 | WinUDP.cpp:812-817 frees without `closesocket` (sockets from :760 and :789) |
| 10 | N-C4 `vahaku` FormatMessage | OSITTAIN | low-medium -> low | HkMaali **and HkKisaWin** | new | EMITHTTP is in HkKisaWin.cbproj:181 (Base_Win32) and ViestiWin.cbproj:134. Test: FormatMessage fails and sets the pointer to NULL |
| 11 | N-C5 `autotulostusparam.lstf` stale alias | VAHVISTETTU | medium -> medium | HkKisaWin | new | HkInit.cpp:3474; UnitStatus.cpp:698-706; TulostusUnit2.cpp:1388; HkTulostus.cpp:982-984 |
| 12 | N-C6 printer DC lost after error | VAHVISTETTU (state); "message on first error" is wrong | medium -> low-medium | all (GDI mostly GUI) | new | Wrttext.cpp endpageGDI/enddocGDI/SendCharsToGDIPrinter (around :805-812, :824-829, :876-891) give no message; no code reopens when `hPrinter==0` |
| 13 | N-D1 COMFILE read failure sends stale packet | VAHVISTETTU | medium -> low-medium | all | **duplicate of H-044** (cites TpComY.cpp:1465-1513, "previous packet again with new id") | TpComY.cpp:1464-1477, 1478, 1497-1503; `obuf` is never cleared after the ack (kirjaahyv :1573-1610) |
| 14 | N-D2 "Poistu" or `perr` "No" deadlock | VAHVISTETTU (static) | medium -> medium | HkKisaWin, ViestiWin | extends H-009, T-032 (new trigger) | cbTpApu.cpp:480-490 (2-arg form = two buttons, :451-462); TpComY.cpp:1464/1471/1474 under `outb_CS`; WinHk.cpp:793-803; TpComY.cpp:1921 |
| 15 | N-D3 HK console `_exit` truncates auto file | VAHVISTETTU | low-medium -> low | HkMaali | relates T-030 | HkInit.cpp:3730-3830 (no `aftulparam.lstf` close, `_exit(0)`); autofile on `taustaThr` (HkIV.cpp:3673-3686, HkInit.cpp:3708) |
| 16 | N-D5 `opencomfile` ignored at stage change | OSITTAIN | medium-low -> low | all (levypusk) | new; relates T-033 | HkIx.cpp:1858, 1974-1979; TpComY.cpp:2268-2273, 513-515, 2404-2422. The note "only from the second port onward" is confirmed |
| 17 | H-134 extension: `putem` on closed fd | VAHVISTETTU | - | all | **duplicate of H-105** (already cites HkIx.cpp:1936-1946 and the MSVC invalid-parameter end) | HkIx.cpp:1907-1912, 1936-1946; HkEmit.cpp:3050-3060 (`femitfile` wraps `emitfile`); `tall_emit` has no `emitfl` test |
| 18 | D-4 `teejarj` writes before NULL test | VAHVISTETTU | low -> low | all | new (OOM only) | HkIx.cpp:1636-1651; VIx.cpp:1104-1112 (only the last `os` is tested) |
| 19a | `luepakettiTCPSI` dead | OSITTAIN | - | compiled, never called | T-012 | TpComY.cpp:1957-1961 is guarded only by `__linux__`, **not** by SIIMPORT. It has no caller. SIIMPORT has no definition (only `LID_SIIMPORT`) |
| 19b | `globals.cpp` not linked | VAHVISTETTU | - | none | - | not in tputilv2.vcxproj or Tputil-XE.cbproj |
| 19c | HkTV.cpp in no project; `lah_tv` | VAHVISTETTU | - | none | - | the call at HkDat.cpp:1231-1239 is under `#if defined(TV1)`. TV1 has no definition (only `TV1COM`, HkDef.h:94) |

## Notes per claim (only where I disagree or add something)

**1 (N-B1).**
- The ACK does not depend on `addem` at all: both `tall_emit` variants return 0 on every path that reaches `addem` (HkIV.cpp:1151; VIv.cpp:1112).
- The operator does see a `wperr` MessageBox in the reader thread under `emit_CS`. Its error code is wrong (N-B6).
- Its "No" button calls `lopetus()` from the reader thread (perr.cpp:92-94). That feeds item 14.
- Medium is justified.

**2 (N-B2).**
- "Never retried" is slightly too strong. HkAjat.cpp:208-209 and :274-275 lower `fp1` when a row is deleted or inserted at an earlier position. A later out-of-order time therefore rewrites the gap.
- In the HK version a seek failure `return`s and skips the other queues. In the relay it `continue`s.

**3 (N-B3).**
- **WEOF:**
  - UCRT `fputws` returns **EOF, not WEOF** ("intentional", `C:\Program Files (x86)\Windows Kits\10\Source\10.0.26100.0\ucrt\stdio\fputws.cpp:15-31`).
  - `WEOF` = `(wint_t)0xFFFF`, with `wint_t` an unsigned short (ucrt\corecrt_wctype.h:21, corecrt.h:605).
  - So in the MSVC builds (HkMaali, ViestiMaali, ViestiLuenta) the `== WEOF` test at HkTulostus.cpp:1484 and :1526 cannot fire for **any** TextType, 'W' included.
  - C++Builder 37.0: `WEOF (_WINT_T)(0xFFFF)` with `wint_t` a typedef of `wchar_t` (crtl\stdio.h:88, _stddef.h:139-144). Whether its RTL `fputws` returns WEOF is unknown.
  - Consequence: part B's "only 'W' gets working detection" and D-14/N-D8 ("double free reachable for UTF-16") hold at most for the GUI builds.
- **Conflict B vs D-12:** D is right.
  - `openprfile`'s TextFl branch calls `writeerror_w(L"Tiedoston avaaminen ei onnisunut", 0)` (Wrttext.cpp:171-176). The message has no file name.
  - GUI: the 2-arg form blocks the caller in a `Sleep(200)` loop until the operator answers (cbTpApu.cpp:480-490). Console: it waits for a key.
- **Blocked threads** (all hold `autotul_CS`, taken at HkTulostus.cpp:7106):
  - HkKisaWin: `LukumaaratTh` (WinHk.cpp:1829-1852 -> `tarkAutoTulostus` :1801-1825). Counts refresh, `autotulostus` and `etHaku` stop until the dialog is answered.
  - HkMaali: `taustaThr` (HkIV.cpp:3673-3686).
  - Relay GUI: called from ViestiWin/UnitMain.cpp:520. Which thread runs it was not checked.
- A failed main open shows two dialogs: the one from `openprfile` and the one at :7111-7113.
- The `vainMuutt` drop is confirmed: `fprior[srj] = 0` at :7266 runs before the open at :7276.

**4 (N-B4).** The mechanism holds as written. "Lost silently apart from a misleading message" is wrong. Before the header dialog, the operator sees the `perr` "Continue execution?" box from the failed `putrec`. Pressing "No" there leads to item 14. Whether NTFS rejects the append depends on cluster slack [inferred].

**5 (N-B5).** I lowered it to low. Power loss only. The Windows lazy writer flushes within seconds, and peers hold replicated copies. `FILE_FLAG_WRITE_THROUGH` does not flush the drive cache either.

**7 (N-C1).**
- C++Builder `_nfile.h:20-24` sets `_NFILE_` to **50** for Win32 (512 for Win64). If the bcc32 RTL enforces that limit, HkKisaWin runs out of FILE streams after about 40-45 runs (about 45 min at `afvali` 60 s), not about 500. Not run.
- What fails then: every stdio/TextFl open (exports, auto files, the `_wfdopen` of the emit file at the next stage change). KILP.DAT is unaffected (CreateFile).
- Medium is right for the narrow precondition (kohde X, `k_pv > 0`, `AUTOFILE.LST` present).

**8 (N-C2).** One more path: with kohde H/M and no series written, the unconditional `htmlloppu(&aftulparam)` (:7317-7319) and a new `sendtcpfile` thread both run on the freed `PRFILE`. The thread then calls `closeprfile` on it again.

**10 (N-C4).**
- Test result (Windows 11, `FORMAT_MESSAGE_ALLOCATE_BUFFER|FROM_SYSTEM`): for 12029, 12002 and 12007 it returns 0 with last error 317 and **writes NULL** into the output pointer. Codes 2 and 10061 get text.
- `swprintf("%s", NULL)` gives "(null)" in the UCRT; bcc32 unknown. So a crash through a garbage pointer is unlikely.
- The leak happens only for non-WinHTTP codes (`er` = `GetLastError()` from httphaku, gethttp.cpp:148-151).

**11 (N-C5).** Confirmed. The `closeprfile` + `openprfile` pair often reuses the same malloc block, so the symptom may be masked and unpredictable. Note: the "Tuloskirjoitin" menu, which is the only recovery for N-C6, is also the trigger for N-C5.

**12 (N-C6).** The first EndDoc/EndPage failure is also silent. Only the StartDoc/StartPage failures have messages. With a spooler, EndDoc rarely fails for an offline printer [inferred], so I lowered the severity.

**13 (N-D1).** H-044 already covers the same function and the same effect. Record the read-failure trigger as an addition to H-044. In the disk-full chain the resent packet is the immediately preceding one, usually an idempotent record update.

**14 (N-D2).** The question asked is answered: the two-button variant is called from lock-holding sites.
- **`outb_CS` path:** `lahetapaketti` "koodi 1/2" (TpComY.cpp:1471, 1474) and `kirjaahyv` "koodi 1" (:1602) are 2-arg calls inside `outb_CS` (:1464, :1581).
  1. The worker thread waits inside SendMessage.
  2. On "Poistu", the main thread runs `Close()` -> `FormCloseQuery` -> `lopetus1` -> `closecom`, which ends in `EnterCriticalSection(&outb_CriticalSection)` (:1921).
  3. That lock is held by the worker, so both threads wait: deadlock.
- **`tall_CS` path:** `tark_kilp` calls `tallenna` under `tall_CS` (HkCom32.cpp:326-332) -> `PUTREC` -> `putrec` -> `perr`; "No" -> `SuljeKaikki` (SendMessage) -> `FormCloseQuery` -> `talljarj` -> `EnterCriticalSection(tall)` (HkApu.cpp:306). Deadlock.
- Every GUI 2-arg `writeerror_w` has the "Poistu ohjelmasta" button, so any 2-arg site under `tall`, `ajat` or `outb` is a candidate.

**15 (N-D3).** The file is truncated at open (mode "w+") anyway, so a cycle interrupted for any reason leaves it partial. The missing flush adds little. Low.

**16 (N-D5).**
- `comfileopened` is a local initialised to false (HkIx.cpp:1858). With one open connection, `opencomfile(1)` is never called at a stage change. With n connections it is called n-1 times.
- `obfilename` gets the stage number only at startup (HkInit.cpp:3358-3363), so the reopen opens the same file. Reopen failure is unlikely.
- `initcom`'s "Muisti ei riitä" comes after `opencomfile`'s own correct message. It misleads, but it is cosmetic.

**19a.** The conclusion (dead) is right, but for a different reason: the function is compiled into every product and simply has no caller.

## New issues found (not in N-B/N-C/N-D, not found in the registry)

1. **NEW: relay `autofile` frees the same `PRFILE` twice on every run** (Juk/VTulostus.cpp). It is compiled: ViestiWin.cbproj:134 and JukMaali520/JukLuenta520 define AUTOFILE. Callers are ViestiWin/UnitMain.cpp:520 and VIv.cpp:2443.
   - In the no-list branch: `closeprfile(aftulparam.lstf)` at :7200 does not NULL the pointer. Then `htmlloppu(&aftulparam)` at :7297-7299 uses the freed block, and `closeprfile(aftulparam.lstf)` at :7320-7321 (when `fileyht < 0`) frees it again.
   - `startaf` (:7036-7043) sets `yksihtml = true` when `AUTOFILE.LST` is missing. That makes this branch the **default** relay H mode.
   - Also: `delete aflst` at :7164 without NULL, then `delete aflst` at :7295-7296. That is a double delete when `yksihtml` is false and the list does not open (kohde T, or the list removed after `startaf`).
   - kohde X returns at :7182-7186 with `lstf` left dangling.
   - Part C judged this code dead from the wrong premise. Severity medium (heap corruption every `afvali` cycle). The real effect depends on the RTL heap (bcc32 or MSVC); a test run would settle it.
2. **NEW (low): `samatleimat` compares `em1->ctrltime[i]` with itself** (Hk/HkIV.cpp:949, Juk/VIv.cpp:941). The duplicate-read check therefore compares control codes only.
   - Two different readings of the same card with the same codes at the same reader, with no other card read in between, are treated as one re-read and not stored. Example: a loan card reused on the same course.
   - Related to N-B1 (the `ed_em` suppression).
3. **Note (part of N-B3):** in the MSVC builds the "Virhe tiedostoon ... kirjoitettaessa" branches (HkTulostus.cpp:1484-1489, 1526-1530, and the relay equivalents) are dead for every encoding, because UCRT `fputws` returns EOF.

## Unresolved questions

1. What do the bcc32 RTL `fputws`/`fputs` return on error (WEOF or EOF)? This decides whether the 'W' check works in HkKisaWin and ViestiWin. A probe on a full volume would settle it.
2. Does the bcc32 RTL enforce `_NFILE_` = 50 at run time? Count streams after N runs of the N-C1 path.
3. How do the bcc32 and MSVC heaps react to the relay `autofile` double free (silent, exception, or fail-fast)? Run relay `autofile` in H single-file mode a few times.
4. Which thread runs the relay GUI `autofile` (ViestiWin/UnitMain.cpp:520)? This decides what the per-series dialog blocks.
5. Does a reader treat the ACK as final, and can a re-dip recover a card lost under N-B1? This is hardware-dependent.
6. Does a refused TCP connect keep its ephemeral port until `closesocket` (the N-C3 limit)? A handle and port count in a loop test would settle it.
7. Relay `lopetus1` taking `tall_CS` (N-D2 relay variant) was not re-read by me.
