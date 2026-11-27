## C-7 step 1 part D: independent review of osaBC-logiikkavirheet.md

(independent-verifier, Opus. Copied verbatim from the agent's hand-back message.)

I re-read all cited code myself. Everything here is static; nothing was built or run. Line numbers are under `C:\dev\tulospalvelu\TPsource\V52`.

### Verdict table

| Claim | Verdict | My severity | Key evidence |
|---|---|---|---|
| N-1 A/L row becomes finish | **OSITTAIN** (partly) | High in HK only where L (start) rows exist (ski start recording, LAJAT queue). Low for A rows and for relay. | `cbHk/UnitAjanotto.cpp:632-640` (missing `else`). `:659` and `:672` apply the change. The grid path parses A/L correctly (`:1150-1152`, `"0ALM"`). `ol_piste` = -2 (`Hk/HkGlobals.cpp:298`). The A row is resolved by `maaraa_piste` (`Hk/HkIV.cpp:2811-2813`). |
| N-3 `tall` leak in relay `BtnLahtoajatClick` | **VAHVISTETTU** (confirmed) | High | `ViestiWin/UnitArvonta.cpp:544` Enter, `:546-547` `continue`, `:576` the only Leave. Not recorded in `saikeet-ja-lukot.md` or `havainnot.md`. |
| N-2 relay `TallennaVaihto` | **VAHVISTETTU**, effect corrected | Medium | `ViestiWin/UnitEmit.cpp:2286`, `:2288`, `:2312` (commented out), `:2310`, `:2325`. `Juk/vdat.cpp:63-73`, `:492-495`. The relay `kilptietue` has no constructor (`Juk/VDef.h:734-755`). |
| N-4 Toiminto 4 cursor | **VAHVISTETTU** (mismatch); effect [I] | Low-medium (setup mode only) | `ViestiWin/UnitArvonta.cpp:375`, `:387`, `:442`, `:448`, `:455`. `SeurPos` case 2 is at `:219-228`. |
| N-6 ComboBox -1 / "Ei valittu" | **OSITTAIN** | Medium for relay `CBLAOsuudet`, but for a different reason (labels shifted by two). Medium for relay Siirrä. Low for HK. | `ViestiWin/UnitArvonta.cpp:54`, `:548-553`; `UnitArvonta.dfm:566-577`. `UnitEmitMuutokset.cpp:58-60`, `:288`, `:154`. `cbHk/UnitArvonta.cpp:526`, `:550-553`. |
| N-7 piste above series limit | **VAHVISTETTU** | Medium (low-medium) | `cbHk/UnitAjanotto.cpp:657-662`, `:671-674`, `:1234-1250`. `Hk/HkTls.cpp:829-851`. |
| N-10 and H-017 narrowing | **VAHVISTETTU** | Medium | `cbHk/UnitEmit.cpp:2469-2474`, `:2694`, `:2722-2736`, `:2791`. `Hk/HkDat.cpp:964-979`. HK form check `cbHk/UnitKilpailijatiedot.cpp:613-626`; relay form `ViestiWin/UnitJoukkuetiedot.cpp:518-528`. |
| Part C `:916` / N-8 | **OSITTAIN** | Leave H-085 at medium; add a second trigger | The extra path exists only for split mode 'P' (`cbHk/UnitArvonta.cpp:949`). Its main harm is an uninitialised `msrj[rs]`, not only `n1`. |
| N-9 `pari` = 0 | **OSITTAIN** | Low | XML load, UnitSarja and the series grid all clamp `pari` to at least 1. `TakaaAjoUnit.cpp:229`, CSV import `UnitSarjatiedot.cpp:1432` and the legacy text file `Hk/HkSrj.cpp:377/387` do not. |

### Per-claim reasoning

**N-1.** The code defect is real in both builds. The panel shows letters: `naytaAktAika` uses `str_piste` (`ch_piste` = "ALM..."), so -2 shows as A, -1 as L, 0 as M. The edit panel (Panel2) is visible, and `EdPiste` and `BtnTall2` are enabled.
- **A rows (-2) are "decide the point automatically".** Keyboard and device times start as A (`ol_piste` = -2, `NappainPiste` = -2 at `:141`). Once a bib is known, the reader resolves A to a real point. So rows still showing A almost always have bib 0.
- **A rows are mostly harmless.** The grid path also turns A into the default point when a bib is typed (`OletusPiste` = 0, `:1137-1139`). The panel bug gives the same result as that default (piste 0). It only differs if `OletusPiste` was changed, or if automatic resolution would have chosen L or an intermediate point. Low.
- **L rows (-1) are start times with a bib.** They come from ski start recording (`hiihtolahto`, `maaraa_piste` setting -1 at `Hk/HkApu.cpp:321-327`), the LAJAT start queue (`Hk/HkInit.cpp:1442-1444`) and wave starts. `uusintaTallennus` handles them deliberately (`:982-983`).
- **The L-row effect is real.** "Tallenna muutos" on an L row (for example, correcting a bib) clears the start time if it matches (`tall_tulos(-1,0)`). It then sets the finish to the start-clock value (`set_tulos(0,…)`). The only warning is "Kilpailijalla jo aika" (competitor already has a time) when a finish already exists (`:645-647`). So: high in those setups, none in standard orienteering.
- **Relay.** 'A' maps to -1 while the display uses -2 for A, and there is no 'L' branch. The result is piste 0 for both letters, written to the queue only: `Juk/vajat.cpp:103-260` never touches the team record. Low.

**N-3.** Confirmed exactly at `:544` / `:546-547` / `:576`; there is no other Leave. A leak happens whenever a record is skipped. The loop starts at `d` = 1 over all records, and every deleted record or record in an unselected series is skipped, so in practice a leak is almost certain.
- The CS is a normal recursive Win32 critical section (`Juk/VInit.cpp:2557`, never deleted). The main thread keeps running, but every worker thread that enters `tall` blocks until exit.
- Through C-6 edges (relay tall→emit, rastiva→emit), a later main-thread wait on a lock that a blocked worker holds would freeze the GUI. That part is [I].
- Toiminto 1, "shift start times", is a plausible race-day action (delayed start), so high is right.
- Not recorded in C-6 (`saikeet-ja-lukot.md` has no hit for `UnitArvonta`/`BtnLahtoajat`) or in `havainnot.md`. Treat it as new.
- Additional: `SarjaValinnat[kilp.sarja]` is indexed before any status check.

**N-2.** All three defects are confirmed, with corrections:
- `getpos` returns 0, not -1, for an in-range unused number, because `kilpnoix` is calloc'd (`Juk/VIx.cpp:1053`, `:1091`). The `d == 0` guard catches that. `d` = -1 only comes from an empty or out-of-range number. An empty field is the realistic case: `_wtoi("")` = 0, which is below `minkilpno`.
- The relay `kilptietue` has no constructor, so `vkilp` is uninitialised stack memory. The source team's `ostiet[Osuus].badge` is overwritten with indeterminate bytes, not necessarily cleared, and saved (`:2325-2329`). That is worse than "cleared".
- `os` = -1 (`ostiet[-1]`) lands outside the object. `sizeof(osrec)` is roughly 250+ bytes and the `kilptietue` header is roughly 140, so the write goes into neighbouring stack or form members, not "on `Kilp_0`" as a record field.
- `os` = -1 is reachable: `Nayta` sets `ItemIndex = osuus+1`, so an unlinked card shows "Ei valittu" (not selected).
- The `kno` defect was already flagged by bcc32 W8013 in C-4 (`todisteet/c4/triage_B_luokitus.csv:90`, `UnitEmit.cpp:2313`, LIKELY) but never added to H-069.

**N-4.** Confirmed. In `ToimintaTila`==1 `jarj[4]` stays 2 (record cursor), while `d_in = kilp.kilpno` is a bib. Outside setup mode `JarjNo` is 0 or 4, which are bib-ordered, so there it is consistent.
- An extra effect [I]: with a negative shift and bibs below the record count, the cursor keeps finding the same record. It shifts it repeatedly until `varmista_vapaa` aborts at `minkilpno`.
- Setup only and recoverable by renumbering, so low-medium.

**N-6.**
- **`CBLAOsuudet`.** The dfm has no Style, so typed text gives index -1 and `os_ala` = -2. But `ostiet[-2]` is about 380 bytes before `ostiet`, outside the local `kilp`. It is a stack write, not "written into the record and saved". The effect depends on the frame layout (EI TODENNETTAVISSA, unverifiable statically).
- **The labels are not cosmetic, contrary to the original.** `UnitArvonta.cpp:54` labels item k as "(k-2). osuus", but item k selects leg index k-1. So picking "1. osuus" changes leg 3. That reaches a wrong-leg start-time overwrite through normal use.
- **Relay Siirrä.** `BTnSiirraClick` passes -1 when "Ei valittu" is selected, which is the default for an unlinked card. The claim that the `BtnTallennaClick` call (`UnitEmitMuutokset.cpp:58-60`) is guarded is **wrong**. The condition is `kno changed || (ItemIndex>0 && …)`, so a changed bib with "Ei valittu" also calls `SiirraKilpailijalle(kno,-1)`. `Kilp.ostiet[-1].badge[0]` is a write into the form object (out of bounds).
- **HK `CBNumPaiva` -1.** Typing only. In `ToimintaTila` 2 it is caught by `cbHk/UnitArvonta.cpp:550-553`. Setup-mode reads are out of bounds. Low.

**N-7.** Confirmed. The old point is cleared unconditionally when the time matches, and the new point is written only if it is within `valuku`. The grid path calls `set_tulos` without a range check, and `tall_tulos` silently returns 0. The first confirmation dialog only appears if `p_aika(out-of-range)` is non-zero, which it is not. So the user sees only the generic "Vahvista tallennus" (confirm save). The relay guard is at `ViestiWin/UnitAjanotto.cpp:1170`, not `:1183-1184`.

**N-10 / H-017.** HK `BtnMuutoksetClick` snapshots `Kilp_0` and keeps the dialog modeless (`:2481`). The later saves write `Kilp_0` without re-reading under `tall`. HK `tallenna` writes `*this` whole, after diffing against the on-disk record when `entkilp` = 0. The relay saves `Kilp_0` the same way (`ViestiWin/UnitEmit.cpp:2209`, `:2269`). Confirmed.

Suggested H-017 narrowing:
- `cbHk/UnitKilpailijatiedot.cpp:613-626` has a "changed elsewhere" check (`Kilp2` vs `Kilp1`). It runs outside `tall` (`:613` vs `:626`), so only a short TOCTOU window with no UI in between remains. The relay form does the same check inside `tall` (`ViestiWin/UnitJoukkuetiedot.cpp:518-528`).
- Replace the Kilpailijatiedot citation with the Emit-muutokset paths (HK `cbHk/UnitEmit.cpp:2469`, `:2694`, `:2722-2736`, `:2791`; relay `ViestiWin/UnitEmit.cpp:2054-2056`, `:2209`, `:2269`). Note Kilpailijatiedot as "residual TOCTOU only (low)".

**Part C / N-8.**
- `msrj[]` holds only rows with a valid name (`:1048-1060`), and `ValmisteleJako` writes `Cells[2]` only for rows 1..n.
- Only the 'P' branch loops `rs < RowCount-1` and uses `msrj[rs]` (`:949-984`). The R/A/T branch keeps `rs` within 0..n-1. It only reads `nv[rs]` from grid rows, which is misaligned when a middle row is blank: wrong bib and start offsets, low.
- In 'P' with one blank or invalid row, the last grid row keeps the value written by `SGMuodostettavatExit`. That handler also runs on `OnExit` when the Jaa button is clicked. Its value is `nilm[jsrj]` minus the other cells, so it can be large and positive. `msrj[n]` is uninitialised, so `Sarjat[garbage]` is read and `setSarjaAll(garbage)` is written. This happens only if `jsrj` still has members, for example when the source series is also a target [I].
- So the path is real but narrower ('P' only). Its consequence is a garbage series index rather than the `d[-1]` write. It adds a trigger to H-085 without changing H-085's medium rating.

**N-9.**
- `pari` is clamped to at least 1 in XML load (`Hk/HkXml.cpp:690-692`), `UnitSarja.cpp:183-184` and `UnitSarjatiedot.cpp:765-767`.
- It is not clamped in `TakaaAjoUnit.cpp:229`: an emptied "aalto" field stores 0 in `Sarjat`, while the form itself clamps only a local at `:300-302`. CSV import (`UnitSarjatiedot.cpp:1432`) and the legacy text file are also unclamped.
- Also in the TakaaAjo case: in-session use only, because saving and reloading through XML restores 1.
- If 0 is reached, the divide is inside `Enter(tall)` (`:419-442`), so an exception would skip `Leave`, the same class as N-3 [I].
- Other `/pari` divisions (`UnitSarjatiedot.cpp:504`, `:1086`, `:1542`) may fault first. Low.

### What the original analysis got wrong
1. N-1 calls the A/L case uniformly "high". The A case matches the default grid behaviour, so the real damage is L rows in start-recording setups.
2. N-2's effect: the badge is overwritten with uninitialised stack data, not cleared. `d` = -1 only for empty or out-of-range numbers. `ostiet[-1]` writes outside `Kilp_0`. The `kno` defect was already a C-4 W8013 (LIKELY) hit.
3. N-6: `ostiet[-2]` is a stack write outside the record, not a record write. The "os-1" labels are not cosmetic; they cause wrong-leg edits. The `UnitEmitMuutokset.cpp:59-60` call is not fully guarded.
4. N-8 is limited to split mode 'P', and the key garbage value is `msrj[rs]`.
5. Line drift: relay guard at `:1170`, relay `tall_rivi` at `:657`.

### Unresolved questions
- N-3: run time. Select one series, press "Lähtöajat", then check whether the emit reader and `tarkcom` stall and whether the GUI later deadlocks.
- N-1: how often operators use "Tallenna muutos" on L rows in ski events. Settle with a run: add an L row with a bib, then save from the panel.
- N-2 and N-6: where the out-of-object writes land in the bcc32 frame or form layout. Settle with a debugger or a probe that prints `offsetof` and `sizeof(osrec)`.
- N-8: what SeurPos mode 7 picks when the source series is also a target. Needs a run.
- N-9: whether bcc32 raises `EDivByZero` there and whether `Leave` is skipped. Needs a run.
- N-5, N-11, N-12 and N-13 were not reviewed (not requested).
