# C-7 step 1, parts B and C: static audit (read-only)

Tags: [V] = verified by reading code, [I] = inferred. Line numbers are in `TPsource\V52`. I did not run or compile anything.

**Coverage limits:**
- I read the emit-card, manual-timing and draw units in full or near-full.
- I only sampled `cbHk/UnitKilpailijatiedot.cpp` (about lines 540-1010) and `ViestiWin/UnitJoukkuetiedot.cpp` (about lines 60-175, 440-560, 640-780).
- `ViestiWin/OsuusTiedot.cpp` was not read. The leg-level edit logic is in `UnitJoukkuetiedot.cpp` `paivitaMuutos`.
- I did not read `Paivita` or `NaytaEmit` in either UnitEmit, or the relay `UnitEmit` handlers beyond those cited.

## (1) Findings table

| ID | Title | Units | Sev | Evidence | Reach | Status | Extends |
|---|---|---|---|---|---|---|---|
| N-1 | "Tallenna muutos" turns an A/L (start-type) row into a finish row (`piste` = 0) | cbHk and ViestiWin `UnitAjanotto` | high where A/L rows exist, otherwise none | `cbHk/UnitAjanotto.cpp:630-639`; `ViestiWin/UnitAjanotto.cpp:634-641` | The edit panel shows A/L in `EdPiste`. Pressing "Tallenna muutos" on such a row re-parses it. | [V] | new |
| N-2 | Relay `TallennaVaihto` uses an unset `kno`, accepts `d` = -1 and `os` = -1 | ViestiWin `UnitEmit` and `UnitEmitMuutokset` | medium | `ViestiWin/UnitEmit.cpp:2284-2313`; `UnitEmitMuutokset.cpp:277-281`; `Juk/vdat.cpp:64-74`, `:164-169`, `:470-495` | Emit form, then Muutokset, then "Vaihda" button | [V] code, [I] effect | extends H-069 (never-assigned locals) |
| N-3 | Relay `BtnLahtoajatClick` leaks `tall` on every skipped record | ViestiWin `UnitArvonta` | high | `ViestiWin/UnitArvonta.cpp:544-547` | "Lähtöajat" button, with any unselected series or deleted record | [V] leak, [I] consequence | new (relies on C-6 lock semantics) |
| N-4 | Relay "Numeroiden muuttaminen" in `ToimintaTila==1` traverses with the wrong cursor | ViestiWin `UnitArvonta` | medium | `ViestiWin/UnitArvonta.cpp:375, 387, 440-448, 455` | `BtnNumeroiClick` with Toiminto = 4 in setup mode | [V] mismatch, [I] effect | new |
| N-5 | Relay renumber: precheck/runtime limit mismatch, partial write on abort, success message anyway | ViestiWin `UnitArvonta` | low-medium | `ViestiWin/UnitArvonta.cpp:81` vs `:353`; `:404-460`; `:480` | Numeroi, Toiminto 0-4 | [V] | new |
| N-6 | Editable ComboBox with ItemIndex -1 or "Ei valittu" used as an array index | ViestiWin `UnitArvonta` and `UnitEmitMuutokset`; cbHk `UnitArvonta` | medium (relay `CBLAOsuudet` writes into the record), low (others) | `ViestiWin/UnitArvonta.cpp:548-553` (+ `UnitArvonta.dfm:566-577`); `UnitEmitMuutokset.cpp:281, 289`, `UnitEmit.cpp:2101-2128`; `cbHk/UnitArvonta.cpp:526`, `:146`, `:1039`, `:1078` | Operator types text into a dropdown, or leaves "Ei valittu" | [V] code, [I] reach | new |
| N-7 | HK timing edit with `piste` above the series limit: old result cleared, new one dropped silently. `tall_rivi` return value ignored. | cbHk `UnitAjanotto` (+relay for the return value) | medium | `cbHk/UnitAjanotto.cpp:656-675`, `:1110-1135`, `:1163-1190`; `Hk/HkTls.cpp:829-851`; `ViestiWin/UnitAjanotto.cpp:645`, `:655`, `:1235-1255` | Typo in the Piste cell or field on an already-timed row | [V] | new |
| N-8 | `ToteutaJako` loops over grid rows but `msrj[]` holds only the valid rows; cell values are stale or recomputed | cbHk `UnitArvonta` | low-medium | `cbHk/UnitArvonta.cpp:914-917`, `:957-960`, `:1022-1034`, `:1073-1078` | Blank or invalid series name in a "muodostettavat" row | [I] | extends H-085 |
| N-9 | Division by `pari[i]` without a zero check, in the draw loop with `tall` held | cbHk `UnitArvonta` | low-medium | `cbHk/UnitArvonta.cpp:429-438`; `cbHk/TakaaAjoUnit.cpp:229`; `Hk/HkSrj.cpp:371-387` | Series with `pari` = 0 (typed in the TakaaAjo form or read from the series file) | [I] | new |
| N-10 | HK Emit-muutokset dialog holds `Kilp_0`/`DKilp_0` for the whole dialog and writes them back as whole records | cbHk `UnitEmit` | medium | `cbHk/UnitEmit.cpp:2461-2480`, `:2650-2670`, `:2722-2736`, `:2784-2806` | Muutokset dialog, then Tallenna/Siirrä/Vaihda | [V] pattern | extends H-017 (new manifestations) |
| N-11 | `uusintaTallennus` accepts a negative `osuus` and indexes `valuku[]` by `osuus` instead of `yosuus[osuus]` | ViestiWin `UnitAjanotto` | low | `ViestiWin/UnitAjanotto.cpp:947`; `Juk/vkilp.cpp:754-757` | Row with a typed or garbage `osuus` | [V] | new |
| N-12 | `BtnEiBadgeClick` does not check for "no vacant" (`kno` = 0) | cbHk `UnitEmit` | low | `cbHk/UnitEmit.cpp:1350-1352`; `Hk/HkDat.cpp:597-607` | "Ei badgea" button when no vacant slot exists | [V] | new |
| N-13 | HK Numeroi: "Numeroiden muuttaminen" and `SprLisa` have no range or duplicate check and no confirmation | cbHk `UnitArvonta` | low-medium | `cbHk/UnitArvonta.cpp:575, 677-678, 708-716` | `RGNumToiminto` = 7 (the code label reads "Numeroiden muuttaminen"); `EdMuutos` and `EdtSprLisa` | [I] | new |

## (2) Per-finding detail

### N-1: A/L start rows become finish rows on re-save [V]
- HK `BtnTall2Click`, `cbHk/UnitAjanotto.cpp:630-639`: the `A` and `L` branches set `piste` = -2 or -1. Then `if (cp == L'M') ... else upt.piste = (char)_wtoi(EdPiste->Text)` runs. The second test is a separate `if`, so for `A` and `L` the `else` also runs. `_wtoi(L"A")` is 0, so `piste` ends as 0 (finish).
- `naytaAktAika` fills the field with `str_piste(pt.piste)` (`cbHk/UnitAjanotto.cpp:521`; `Hk/HkApu.cpp:148-153`), so start-type rows show a letter. Pressing "Tallenna muutos" (`BtnTall2`, dfm:314) on any such row therefore rewrites `piste` to 0.
- HK then clears the old competitor's value (`kilp.tall_tulos(va_piste(vpt.piste), 0)`, line 659). It then calls `set_tulos(va_piste(0), upt.t)` (line 672), which sets the competitor's finish time to the start-time value.
- Relay `Button2Click`, `ViestiWin/UnitAjanotto.cpp:634-641`: same missing `else`. Here `'A'` is also mapped to -1, whereas the display array `"?ALM"` makes -2 the code for A. There is no `'L'` branch at all. The relay version only changes the queue row, which is `tall_rivi(Jono,&upt,&vpt,...)` at line 656. It does not write to a competitor.
- Fix direction: use `else if` and handle `'L'` in the relay.
- To verify: build, add an L row in the time queue, press "Tallenna muutos", and look at the `Piste` column.

### N-2: Relay `TallennaVaihto`, three separate defects [V]
1. `kno` is declared (`UnitEmit.cpp:2286`) but its only assignment is commented out (`:2312`, `//kno = haeSeurVakantti(0);`). The `else` branch (`DKilp_0 != d`, swap between two different teams) then runs `vd = getpos(kno)` on an indeterminate value.
2. `getpos` returns -1 for out-of-range and unknown numbers (`Juk/vdat.cpp:66-73`). The guard at `:2288` is `d == 0`, so `d` = -1 passes. `BtnVaihdaClick` sets `d = getpos(typed)` (`UnitEmitMuutokset.cpp:279`), so a mistyped or empty team number gives -1.
3. `os = CBOsuus->ItemIndex-1` is -1 when "Ei valittu" (item 0, `UnitEmitMuutokset.cpp:38`) is selected. There is no check, so `ostiet[-1]` is read and written on `Kilp_0`.
- Effect with `d` = -1 [I]: relay `GETREC` and `PUTREC` ignore out-of-range `d` (`vdat.cpp:164-176`), so `kilp` and `vkilp` stay as constructed. `tallenna` with an invalid `d` only raises `parametrivirhe` and returns (`vdat.cpp:492-495`). Then `Kilp_0.ostiet[Osuus].badge` is overwritten from the empty `vkilp` (`UnitEmit.cpp:2301-2310`) and saved with a valid `DKilp_0` (`:2310`). The source team's card assignment is cleared by a typo.
- Effect with a valid second team: the swap still works, using `vkilp` as a temporary, with spurious error dialogs for the failed saves of `vkilp`. If the garbage `kno` happens to be a real number, an unrelated team is loaded and then marked `'V'` with its badges cleared (`:2320-2326`).
- The confirmation dialog is only "Jatka" (`UnitEmitMuutokset.cpp:277`).
- To verify: run "Vaihda" with a non-existent team number; also check whether `kilptietue`'s constructor zeroes the record.

### N-3: Relay `BtnLahtoajatClick` skips `Leave` [V]
- `ViestiWin/UnitArvonta.cpp:544` takes `tall_CriticalSection` at the top of every loop iteration. Line 546-547 is `if (kilp.kilpstatus || !SarjaValinnat[kilp.sarja]) continue;`. The `Leave` is at the end of the loop body (about line 575), so every skipped record leaves one extra recursion level held.
- Skipped records are any deleted record, any record in an unselected series, and any vacant slot.
- The main thread then owns `tall` for good (it is recursive for the owner). Worker threads that need `tall` (`tarkcom` and the emit/time readers) block until exit. This overlaps with C-6 lock semantics but is a plain code defect.
- Also: `SarjaValinnat[kilp.sarja]` is indexed with a possibly negative `sarja`. The `kirjheti` reset at the end is fine.
- To verify: select one series, press the button, then check that a second thread can enter `tall`.

### N-4: Relay Toiminto 4 cursor [V] mismatch / [I] effect
- `jarj[9] = {1,2,2,3,2}` (`:375`). `jarj[4]` stays 2 when `ToimintaTila == 1` (line 387 only sets it to 0 otherwise), so `JarjNo` = 2.
- `SeurPos` case 2 interprets `Ed` as a record index (`for d = Ed+1`, ~line 179). But for Toiminto 4 the code sets `d_in = kilp.kilpno` (`:448`), a bib number. In the other modes `d_in = d` (`:455`).
- After the first team, the next search starts at record index = old bib. When bibs are larger than the record count the loop ends after one team. When they are smaller it revisits records and shifts them twice.
- Setup mode (`ToimintaTila == 1`) is the normal numbering phase. In that mode `varmista_vapaa` skips the duplicate check (`:358`) and `tarkNumeroVapaus` is not called (`:386`), so nothing catches it.
- To verify: run with a handful of teams in setup mode and bibs ≥ 100.

### N-5: Relay Numeroi [V]
- Precheck `tarkNumeroVapaus` rejects `kno > maxkilpno` (`:81`). `varmista_vapaa` rejects `kno >= maxkilpno` (`:353`). `SeurPos` treats `maxkilpno` as inclusive (`:92`, `i <= maxkilpno`). A shift landing exactly on `maxkilpno` passes the precheck and aborts partway.
- The abort (`if (varmista_vapaa(...)) break;`, `:414`, `:449`, `:453`) leaves the teams already renumbered and saved (no rollback). `keskeyta` is never set, so the outer loop continues with the next series and "Toiminto toteutettu" is shown regardless (`:480`).
- `UINT16 nums[10000]` (`:430`): `arvonumerot(nums, nilm[srj])` and `nums[k++]` overrun it when a series has more than 10000 teams, and `nums[k++]` can run past `nilm[srj]` if `nilm` is stale. Both are unlikely.

### N-6: ItemIndex -1 or placeholder used as index
- Relay `BtnLahtoajatClick` (`:548-553`): `CBLAOsuudet` has no `Style` in the dfm (`UnitArvonta.dfm:566-577`), so it is the editable default and typed text gives `ItemIndex` -1. `os_ala` is then -2, and `Sarjat[].ylahto[-2]` is read. `kilp.ostiet[-2].ylahto` and `.lahtolaji` are written, followed by `Tallenna` (`:560-575`). That corrupts the local record and saves it, for every record. The `FormShow` also builds labels `os-1` (`:54`, "-1. osuus" first), which is cosmetic.
- Relay `SiirraKilpailijalle(kno, osuus)` (`UnitEmit.cpp:2101-2128`) has no bounds check on `osuus`. The `BTnSiirraClick` call passes `CBOsuus->ItemIndex-1` (-1 for "Ei valittu"), whereas the other call at `UnitEmitMuutokset.cpp:59-60` is guarded by `ItemIndex > 0`. `Kilp.ostiet[-1].badge[0] = Em.badge` and `Osuus = -1` follow.
- HK `BtnNumeroiClick` (`:526`): `Pv = CBNumPaiva->ItemIndex` can be -1 (editable dropdown, no `Style` in dfm). `CBNumPaivaChange` only corrects it in `ToimintaTila == 2`. It is then used as `arv_pv` (`Sarjat[].bibalku[arv_pv]`, `luonne[ipv]` at `:146`, `nilm_pv[ipv]`). Reads only, with wrong numbering behaviour [I].
- HK `BtnJaaClick` (`:1039`) and `SGMuodostettavatExit` (`:1078`) index `nilm[jsrj]` before checking `jsrj < 0`. The first is read-only and is followed by the check; the second does not return after it.
- HK `CBPaiva` and `CBJakoVaiheet` at -1 are handled (they fall back to "all").

### N-7: HK timing edit, `piste` beyond series limit [V]
- `BtnTall2Click` clears the old competitor unconditionally (`:656-662`) and writes the new one only if `va_piste(upt.piste) <= valuku[k_pv]` (`:671`). `DGajat1KeyPress` (`:1163-1190`) has no range check at all. `tall_tulos` silently refuses `piste > valuku` (`Hk/HkTls.cpp:829-851`).
- A typo such as piste 9 on a row that was already applied clears the old competitor's time and does not write the new one. The queue row is saved with the invalid `piste`, and `uusintaTallennus` (`:971-990`) will not repair it.
- Relay handles this case better: `if (upt.piste > valuku[upt.osuus]) tallFl = 1` (`ViestiWin/UnitAjanotto.cpp:1183-1184`) keeps the competitor untouched.
- Both builds ignore the `tall_rivi` return value (HK `:652`, `:1124-1130`; relay `:656`, `:1228`). When `vtm` is not found, `tall_rivi` returns 1 silently with `kysy` = 0 (`Juk/vajat.cpp:150-160`, `:246`). The competitor record is then still changed. The not-found case needs a concurrent queue change [I].

### N-8: ToteutaJako rows vs `msrj[]` (extends H-085) [I]
- `BtnJaaClick` collects `msrj[n++]` only for rows with a valid series name (`:1043-1054`). `ValmisteleJako` writes `Cells[2][i+1]` for `i < n`, consecutively. `ToteutaJako` loops `rs < RowCount-1` over all grid rows and uses `msrj[rs]`, `nv[rs]`, `nu[rs]` (`:914-917`, `:957-960`).
- A cleared or invalid name leaves a row with a stale or `SGMuodostettavatExit`-computed value in `Cells[2]` (`:1073-1078`: `nilm[jsrj]` minus the other rows, which may be negative) and an uninitialised `msrj[rs]`.
- `Sarjat[msrj[rs]]` and `setSarjaAll(msrj[rs])` then run on garbage.
- This only needs one editable name cell (`goEditing` in dfm:880). Whether it was reachable was not run.

### N-9: `pari` = 0 [I]
- `kilp.tall_lajat_pv(... (kno - alkuno)/Sarjat[srj].pari[i] ...)` (`:434`, `:438`) runs inside `Enter(tall)` … `Leave(tall)` (`:415-441`). `pari` is read from the series file unvalidated (`Hk/HkSrj.cpp:377`, `:387`) or typed in `TakaaAjoUnit.cpp:229` (`_wtoi`, no minimum). `UnitSarja.cpp:183` does clamp to ≥ 1, which suggests 0 is not meant to occur.
- A zero would raise an integer divide exception, and `Leave` would be skipped. Whether bcc32 maps it to a catchable VCL exception was not tested.

### N-10: H-017 new manifestations
- `BtnMuutoksetClick` (`:2461-2480`) snapshots `Kilp_0 = Kilp`, `DKilp_0`, `Em_0` and keeps the modeless `FormEmitMuutokset` open.
- `TallennaMuutokset` later modifies and writes the snapshot: `Kilp_0.tallenna(DKilp_0, ...)` (`:2670`).
- `MerkitseOhitetuksi` (`:2722-2736`) does the same.
- `TallennaVaihto` (`:2784-2806`) re-reads `kilp` and `vkilp` fresh but writes the stale `Kilp_0`.
- Core `tallenna` compares against the on-disk copy (`entkilp`) and writes ours, so a result or `tark` that arrived for the old competitor meanwhile is overwritten.
- Relay counterpart: `ViestiWin/UnitEmit.cpp:2192-2200` (`Kilp_0`) and `:2252-2275`. Relay `TallennaMuutokset` does the first read under `tall` (`:2171-2173`).
- Contrary to H-017's mention of `cbHk/UnitKilpailijatiedot.cpp:613-626`: this form has a conflict check. `tallennaTiedot` re-reads `Kilp2` and refuses to save if `Kilp2 != Kilp1`. In HK the check is outside the lock (`:618` vs `:622`), a small TOCTOU window. The relay form checks inside the lock (`UnitJoukkuetiedot.cpp:498-506`). The H-017 text for that form should be narrowed.

### N-11, N-12, N-13 (brief)
- N-11: `uusintaTallennus` guard `pt.osuus < osuusluku` lacks `>= 0`, and indexes `valuku[pt.osuus]` while `kilptietue::Maali`/`setMaali` use `valuku[yosuus[osuus]]` (`Juk/vkilp.cpp:756`). Different when `yosuus != osuus` (parallel or sub-legs). The OOB read is bounded by the struct. `setMaali` still rejects negatives.
- N-12: `haeSeurVakantti(0)` returns 0 when no vacant exists (`Hk/HkApu.cpp:247-275`; the callers at `UnitEmit.cpp:2602` and `:2762` check for it). `BtnEiBadgeClick` then runs `GETREC(getpos(0))` with whatever `getpos(0)` returns, shows an empty form, and a later save is blocked by the `tallenna` `d < 1` check. `GETREC` itself only accepts `d` in `0..maxrec`.
- N-13: HK Toiminto 7 shifts `kilp.id() + muutos` (`:708-716`) with no range or duplicate check, in either build mode. Only `tarkNumerotila` runs for Toiminto ≤ 2. `bibvaihtelee` confirmation does not apply otherwise.

## (3) Part C verdicts

| Candidate | Statement | Verdict | Guard / missing guard |
|---|---|---|---|
| cbHk/UnitEmit.cpp:2819 | `Em.ctrlcode[r-1] = _wtoi(Cells[1][r])` | **safe for index/alloc/loop**; partly for value (low) | Index guard: `EmitGrid->RowCount` is set once to `MAXNLEIMA+1` = 51 (`:445`; the shrink at `:483` is commented out), loop `r < RowCount`, so max index 49 = `MAXNLEIMA-1` (`HkDef.h:1200`). The value goes into `char`, so 128-255 are stored negative and >255 wraps mod 256. No range check. [V] |
| cbHk/UnitEmit.cpp:2820 | `Em.ctrltime[r-1] = _wtoi(Cells[3][r])` | **safe** (index); value wraps in UINT16 (negative becomes large) | Same index bound. [V] |
| cbHk/UnitEmit.cpp:2974 | `Em.ctrlcode/ctrltime[ARow-1] = (unsigned char)_wtoi(Value)` in `SetEditText` | **safe** (index, `ARow < RowCount`); partly (value) | Casting `ctrltime` (UINT16) to `unsigned char` truncates times above 255 in the live `Em` until `TallennaEmMuutokset` re-reads full values from `Cells`. `LaskeTulos` before save sees truncated values [I]. |
| cbHk/UnitArvonta.cpp:916 | `n1 = _wtoi(Cells[2][rs+1])` then `new int[n1+1]`, `d[n1] = -1` | **same defect as H-085 plus an additional path** | `BtnJaaClick` always runs `ValmisteleJako` first (`:1063`) which overwrites `Cells[2]` for valid rows, so the -1 comes from the computation, as H-085 says; typed values cannot reach valid rows. Additional path: rows with a blank or invalid name (N-8). `n1` < -1 gives a huge `new` (exception before `Enter`), `n1` = -1 gives the `d[-1]` heap write (H-085). No guard `n1 >= 0`. [V] code, [I] reach |
| cbHk/UnitKilpailijatiedot.cpp:757 | `Kilp.pisteet[ValPist] = _wtoi(...)` | **safe** | `_wtoi` is the value, not the index. `ValPist` is 0 or 1 (`:92`, `:1876`), `pisteet[3]` (`HkDef.h:504`). Value is INT32, no checks needed. [V] |
| ViestiWin/UnitEmit.cpp:2356, :2357 | `Em.ctrlcode/ctrltime[r-1] = _wtoi(...)` | **safe** (index) | `EmitGrid->RowCount` = 51 (`:446`), arrays are 50 (`VDef.h:1102-1103`), `r < RowCount`. Second loop `r = RowCount; r < 50` never runs. Value wraps as in HK. [V] |
| ViestiWin/UnitEmit.cpp:2504 | `(unsigned char)_wtoi(Value)` in `SetEditText` | **safe** (index, `ARow-1` ≤ 49); partly (value, as 2974) | [V] |
| ViestiWin/UnitAjanotto.cpp:633 | `upt.osuus = _wtoi(EdOsuus) - 1` | **partly** (low) | No range check before storage (INT16, so also truncated). Reads are covered: `kilptietue::Maali`/`setMaali` return/ignore out-of-range (`Juk/vkilp.cpp:754-780`). Gaps: (a) with a known team, `valuku[upt.osuus]` at `:635`-`:641` (the `vast == 2` branch) is only reached when `Maali()` != `TMAALI0`, which implies a valid `osuus`; (b) with an unknown `kno`, or `kno` = 0, no check at all and a row with `osuus` -1 or 999 is stored in the queue; (c) `uusintaTallennus` (`:947`) lacks `>= 0`. No memory write outside the queue was found. [V] |

## (4) Per-unit coverage (questions 1-5)

- **cbHk/UnitEmit.cpp**
  - Q1: grid and cell parsing covered; `EdtKilpnoKeyDown` `_wtoi` + `getpos` are guarded; `EdtTietueExit` is guarded by `lueem` bounds (`HkEmit.cpp:1572-1577`). No defect found beyond N-12.
  - Q2: `TallennaMuutokset`, `SiirraKilpailijalle`, `TallennaVaihto`, `lueKilp3`, `tallennaKilpailija`. The core `tallenna` moves the emit link (`HkDat.cpp:1056-1141`). One open question: whether `lueKilp3` leaves the auto-created placeholder competitor behind (see section 6).
  - Q3: not applicable.
  - Q4: not applicable.
  - Q5: N-10; `BtnPeruutaClick` just reloads, which is fine. `tallennaKilpailija` silently skips the write if `DKilp != getpos(Kilp.id())` (`:1613`) but still clears the pending flag; low.
- **cbHk/UnitAjanotto.cpp**
  - Q1: N-1, N-7. Q3: all of `BtnTall2Click`, `BtnPoistaClick`, `DGajat1KeyPress`, `uusintaTallennus`; `getAktTime` is index-safe (`Tp/TpLaitteet.cpp:225`). Delete: `aktRivi1 = aktrows` reaching 0 is safe because of that.
  - Q5: delete-then-insert in `tall_rivi` (`vajat.cpp:150-160`) can fail between the two, see N-7.
  - Q2, Q4: not applicable.
- **cbHk/UnitArvonta.cpp**
  - Q1: N-6, N-8, N-13. Q4: draw is in `arvo()` in `HkArv2.cpp` (not in scope). This unit covers the numbering and the split-into-series draw. For randomness see the refuted list below.
  - Q5: `ToteutaJako` writes record by record with no rollback; the confirmation is a single "Toteuta jako" dialog.
- **cbHk/UnitKilpailijatiedot.cpp** (sampled)
  - Q1: part C 757 safe; `SrjValExit` can pass `ItemIndex` -1 to `setSarja`, but `tallenna` refuses a record without series (`HkDat.cpp:932-939`). Q2: not applicable.
  - Q5: `tallennaTiedot` checks for a concurrent change; see N-10. No bib duplicate check for stages other than `k_pv`.
- **ViestiWin/UnitEmit.cpp**
  - Q1: N-2, N-6. Q2: `TallennaMuutokset` / `SiirraKilpailijalle` / `TallennaVaihto`; duplicate target-with-data case is asked (`:2161-2165`). N-10 relay variant.
  - Q5: `TallennaMuutokset` holds `tall` over the whole operation; paths `Leave` in both branches (`:2237`, `:2244`).
- **ViestiWin/UnitAjanotto.cpp**
  - Q1: N-1, N-11, part C 633. Q3: `Button2Click`, `Button3Click`, `DGajat1KeyPress`; relay guards `piste` (`:1183`). `srj` uninitialised in `DGajat1KeyPress` (`:998`, `:1096`) is H-069, not repeated.
- **ViestiWin/UnitArvonta.cpp**
  - Q1: N-5, N-6. Q4: `arvonumerot` yields a permutation and the seed is set via the shared static MT state, so no defect. N-3, N-4, N-5. `NumKaikki` unset is H-069.
- **ViestiWin/UnitJoukkuetiedot.cpp** (sampled)
  - Q1: `paivitaMuutos` cells `ipv = row-1` are within the grid. `lahtolaji` (UINT8) cell 9 is refuted below. Q2: not applicable.
  - Q5: `tallennaTiedot` re-checks `Kilp2 == Kilp1` inside `tall` (`:498-506`), better than HK. `addtall` in the `Lisays` branch is called without `tall` (`:481-487`); not analysed further (lock topic).
- **ViestiWin/OsuusTiedot.cpp**: not read.

## (5) Refuted suspicions

- Time-queue delete leaving `aktRivi1` = 0 and indexing -1: refuted, `getAktTime` takes a UINT32 and returns an empty time when out of range (`Tp/TpLaitteet.cpp:225-237`).
- `lahtolaji = wcswcind(...)` = -1 in relay `paivitaMuutos` case 9: refuted, the field is `UINT8` (`VDef.h:690`), so -1 becomes 255 and the very next line `if (lahtolaji > 2) lahtolaji = 0` catches it.
- Draw randomness: `FormCreate`'s local `MTRand_int32 irand(sd)` looks pointless but the generator state is static (`Tp/mtrand.h:66-71`, `:51-53`); the later default-constructed `irand` in `arvonumerot` does not reseed. Not a defect. `irand()/131072` gives a 15-bit key with ties; the permutation is still valid.
- HK `TallennaMuutokset` discarding `Em.kilpno` set in `SiirraKilpailijalle` via `Em.get(Tietue)`: refuted, the core `tallenna` badge move updates `em.kilpno` and links (`HkDat.cpp:1104-1141`, with `kielto` = -1 so `kysyfl` = 0).
- `EmitGrid->RowCount` growing past the arrays: refuted (fixed at 51; nothing adds rows).
- H-017 for `cbHk/UnitKilpailijatiedot.cpp:613-626`: narrower than registered (see N-10).

## (6) Unresolved questions

1. HK `lueKilp3` + `tallennaKilpailija` (card assigned to a competitor by typing the bib): is the auto-created placeholder record (`HkEmit.cpp:1240-1266`, first name = badge) removed, or does it keep a copy of the result/status? Needs a run with an unknown card.
2. Does `kilptietue` in the relay build zero its record in the constructor? It decides what N-2 writes when `vkilp` is never loaded.
3. N-9: is integer division by zero a catchable exception in bcc32 (and does `Leave` get skipped)? Needs a run.
4. N-6: confirm the dropdowns are `csDropDown` at run time (no `Style` line in the dfm); `ItemIndex` = -1 on typed non-matching text is standard VCL behaviour [I].
5. N-3 and N-4 need a run to check the consequences stated.
6. `cbHk/UnitEmit.cpp` `Paivita`/`NaytaEmit` (lines 434-740) and the relay equivalents were not read.

Files (absolute paths under `C:\dev\tulospalvelu\TPsource\V52`): `cbHk\UnitEmit.cpp`, `cbHk\UnitAjanotto.cpp`, `cbHk\UnitArvonta.cpp`, `cbHk\UnitKilpailijatiedot.cpp`, `ViestiWin\UnitEmit.cpp`, `ViestiWin\UnitEmitMuutokset.cpp`, `ViestiWin\UnitAjanotto.cpp`, `ViestiWin\UnitArvonta.cpp`, `ViestiWin\UnitJoukkuetiedot.cpp`, `Juk\vajat.cpp`, `Juk\vdat.cpp`, `Juk\vkilp.cpp`, `Hk\HkDat.cpp`, `Hk\HkTls.cpp`, `Hk\HkApu.cpp`.
