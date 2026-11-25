## C-7 step 1, part D round 2: independent review of N-14..N-20

(independent-verifier, Opus. Copied verbatim from the agent's hand-back message.)

All paths are under `C:\dev\tulospalvelu\TPsource\V52`. This was a static, read-only review; nothing was run.

### Verdict table

| Claim | Verdict | Severity I'd assign | Key evidence |
|---|---|---|---|
| N-14 Poista with dKilp -1 / 0 / already deleted | **VAHVISTETTU**, with corrections | **high** for HK -1 and for the double delete; medium for the 0 case | `cbHk/UnitKilpailijatiedot.cpp:1746-1752`, `:1518`, `:1736-1742`, `:1305-1315`, `:1157-1176`; `Hk/HkDat.cpp:597-621`, `:640-650`, `:741-766`; `Hk/HkTls.cpp:117-143`, `:1143-1144`; **`dbboxm/dbbox1.cpp:227-270`** (not Dbbox.cpp); `ViestiWin/UnitJoukkuetiedot.cpp:76-83`, `:1365-1371`; `Juk/vdat.cpp:164-176`, `:194-204`, `:907-939` |
| N-15 select3 X = button 2 | **VAHVISTETTU** | medium | `cbTp/cbTpApu.cpp:263-276`; `cbTp/UnitSelect3.cpp:39-59` (ModalResult 1/2/3); `UnitSelect3.dfm` has no BorderIcons (so the X is there), no Cancel button, no OnClose/OnCloseQuery and no KeyPreview |
| N-16 lueKilp3 overwrites | **OSITTAIN** | low-medium | `cbHk/UnitEmit.cpp:1663-1681`, `:1698-1745`, `:203-206` vs `:271-272`; the esiluenta save resets the fields anyway: `:1592-1598` |
| N-17 placeholder keeps the badge | **OSITTAIN** ([I], depends on mode) | low-medium | `Hk/HkIV.cpp:1577-1622` (GUI branch, no esiluenta guard); `Hk/HkDat.cpp:950`, `:1055-1064`; `cbHk/UnitEmit.cpp:1569-1578`, `:1586-1594`, `:722-727` |
| N-18 combo ItemIndex -1 | **VAHVISTETTU**; the effect is worse than claimed | medium (both programs) | dfm `SrjVal` has no Style, so it is csDropDown (HK dfm:472-486, relay dfm:400-413, `CBSelOsuus` dfm:736-749); HK `:724-735` → `naytaTiedot` → `sijat(-1)` `Hk/HkTulostus.cpp:1165-1200`; relay `:584-591` → `Juk/vdat.cpp:288-290` |
| N-19 PvGrid->Row out of range inside tall | **VAHVISTETTU** (the VCL raise is strong inference) | medium | `cbHk/UnitKilpailijatiedot.cpp:197-203`, `:236`, `:477`; locked callers `:1105-1153`, `:1157-1211`, `:1070-1087`; no `OnException`/`__finally` in cbHk/cbTp; `Hk/HkSrj.cpp:300-311` |
| N-20 relay tall_rivi argument shift | **VAHVISTETTU** | low-medium | `ViestiWin/UnitAjanotto.cpp:1221` vs `Juk/VDeclare.h:166` / `Juk/vajat.cpp:103-104`; `Juk/VCom.cpp:239-246`, `:400-420`; `Juk/vajat.cpp:~170`, `~225` (muutos=1); HK correct at `cbHk/UnitAjanotto.cpp:1226` |

### Per-claim reasoning

**N-14**
- **Reachable.** The "Poista kilpailija" menu item has no Enabled or Visible gating (dfm:1028-1031, relay dfm:821-824). `asetaMuokkaustila` (`:1257-1299`) never touches it, so it also works when `sallimuokkausvalinta` is false.
- **HK -1.** Open from the main menu (`WinHk.cpp:1896-1900`): new form, `dKilp=-1`. `FormShow` calls `naytaKilpailija(-1)`, which does nothing. Then Poista leads to `deltall(-1)`:
  - `GETREC(-1)` only shows an error.
  - `remjarjarr` decrements the series 0 counters.
  - `delfrec(-1)`: `deleterec(&datf2, 0xFFFFFFFF)`. `putrec` fails because of the seek. `firstfree` becomes 0xFFFFFFFF, which **drops the whole free list**; the original missed this.
  - `kilp.PUTREC(-1,0)`: `PUTREC` only checks `d > maxrec`, so `kilparr[-1] = *this`.
  - `operator=` (`HkTls.cpp:141-142`) writes `pv[i]` through the `pv` pointer of `kilparr[-1]`. That memory is the heap header / new[] cookie in front of `kilparr`, which has a non-trivial destructor. This is a wild write, and a crash is likely ([I] about the exact layout).
- **HK 0.** Lisää then Peruuta leaves `dKilp=0` while showing `Kilp1`; Lisää then Poista directly also does. `deleterec(...,0)` overwrites the file header with the free-link buffer and sets `firstfree=0`. The next `addrec` gets record 0, which `updatedescr`/`closefile` overwrite with the header, and loops start at `p=1`. So the next added competitor is lost or invisible. Medium.
- **Double delete.** Confirmed in the default mode "Sijoitus" (`RGSelaus ItemIndex=0`, dfm:943).
  - For an untimed competitor, `p_sija` returns 0 (`HkTls.cpp:1143`).
  - For a timed one, the record is already unlinked, so the walk ends with `p=-1`. The form keeps showing the deleted competitor with `dKilp>0`.
  - Setup mode is effectively safe: `naytaKilpailija` (`:143-146`, also run from `FormShow`) forces mode 2 when `ToimintaTila==1`. So the risk is in **competition mode (ToimintaTila 2)**.
  - Neither `deleterec` (dbbox1.cpp:262-270) nor `deltall` checks `kilpstatus`.
  - The second delete writes a self-link (record d points to itself) and sets `firstfree=d`.
  - Add #1 gets d and re-reads the self-link. Add #2 also gets d, so the two competitors share a record.
  - Add #3 takes `firstfree` from the low 16 bits of the packed record. `kilpstatus` is INT16 at bufpos 0 (`HkDat.cpp:80`), so that is probably 0, the header ([I]). The damage therefore continues past two adds.
  - The second delete also decrements the counters again.
- **Relay.** Confirmed as described.
  - `kilptietue` has no constructor (`Juk/VDef.h:734ff`), so the local record in `deltall` is stack garbage.
  - `GETREC` ignores d<0 (`vdat.cpp:164-169`).
  - `deleterec(-1)` drops the free list in the same way.
  - `PUTREC(-1)` is a no-op (`vdat.cpp:172`).
- **Not checked.** Whether `remjarjarr` on a record that is no longer in the chain corrupts the rank chains.

**N-15**
- Confirmed for every listed caller:
  - HK `TallennaMuutokset`: `Vast=2` default and only `<=1` cancels, `cbHk/UnitEmit.cpp:2586-2593`.
  - Relay: `ViestiWin/UnitEmit.cpp:2164-2170`.
  - HK `BtnLoppuun` (`:3664-3677`) and relay (`:1978-1990`): `default` calls `Nayta()`, so X behaves as "Hylkää".
  - HK mode 99 (`:607-627`): case 2 reassigns the badge and saves.
  - Relay Ajanotto (`:646-651`, `:1203-1210`): `piste = valuku+1`, so the number is saved without a result.
- **Extra caller the original missed:** `cbHk/UnitEmit.cpp:1511-1514`, `select3(...) != 1`, so X is read as "Maaliin tulija".
- Esc does nothing (no Cancel button). Alt+F4 behaves like the X.
- That X returns mrCancel=2 is standard VCL `TCustomForm.Close` behaviour for modal forms ([I] without a run, but well established).

**N-16**
- The copy and the `'P'`-only guard are real, and the trigger needs esiluenta or kuntoLahto (`:203-206`).
- In **esiluenta**, `tallennaKilpailija` itself forces `tark '-'`, clears the result and sets the start to now (`:1592-1598`) for whatever competitor is shown. So the "overwrite" there is the designed esiluenta semantics, not caused by `lueKilp3`.
- The `lueKilp3` copy really matters only in kuntoLahto. There the placeholder's (vacancy's) start, status and result are stored into R.
- The real defect is that a wrong bib gets no "already has a result/status" check. Downgrade to low-medium.

**N-17**
- The placeholder *is* created in esiluenta and kuntoLahto: the GUI `pros_emit` branch `HkIV.cpp:1577` has no esiluenta guard.
- The refusal in `tallenna` (`bkno && kysyfl`, `kysyfl=!kielto`) is real for `tallenna(DKilp,0,0,0,0)`.
- The conflict does **not** happen in every non-99 mode:
  - kuntosuunnmoodi 2/3 (`badge<BADGEASKEL`) replace `badge[0]` with a fresh badge first (`:1569-1578`), so ibdg 0 does not conflict. Only `kaksibadge==2` would check `badge[1]=B`.
  - In mode 0, `VakTietue` auto-opens the Muutokset dialog (`:722-727`). That gives the proper transfer path, which releases the vacancy (`:2629-2648`).
- So the conflict occurs in mode 0 or mode 1 when the operator types the bib instead of using Muutokset.
- The error dialog is visible, so the failure is not silent. Low-medium, [I], needs a run.

**N-18**
- The combos are csDropDown. Typed text with no AutoComplete match, or an emptied field, gives `ItemIndex=-1`.
- **View mode:** depends on `nilm[-1]`, the global before the array. Layout-dependent and unverifiable statically. If it is ≤0, the else branch restores the index safely.
- **HK edit mode:** worse than claimed. `setSarja(-1)` → `naytaTiedot` → `sijat(-1)` → `new int[nilm[-1]]` plus a walk of `jalku[-1][..]` writing into `sjj`/`djj`. That is possible heap overflow or bad_alloc ([I]).
- **Relay edit mode:** the core does not refuse. `kilpTarkasta` silently sets `sarja=0` (`vdat.cpp:288-290`) inside `tallenna` (`:470ff`), so **the team is silently moved to series 0**. The original did not see this.
- `CBSelOsuus` -1 is confirmed. `jalku[Sarja][-1]` reads the previous row, a wrong team, not a crash.

**N-19**
- `RowCount=vpv-epv+2` and `FixedRows=1` (default). `Row=k_pv-epv+1` maps only 0 to 1. Values >`vpv-epv+1` or <0 raise in VCL: `TCustomGrid.MoveCurrent` → `InvalidOp(SIndexOutOfRange)` ([I], standard VCL).
- Reach:
  - ToimintaTila 2, `sarjavaihtelee` false, a multi-day event where a series' `enspv`/`viimpv` excludes `k_pv`.
  - KilpNo or Nimi browsing reaches such competitors.
  - In setup mode, only when `enspv>=k_pv+2`.
- No try/finally and no `Application->OnException`. The default handler shows a message and the loop continues, but the `tall` recursion count stays +1 on the main thread.
- The same exception through `EdtKilpno` (`:110/138/154`) is unlocked and harmless.

**N-20**
- Exactly as described: `kielto=pakLah(1)` skips cn 0 (the first connection) and any cn whose `estavalitys` bit 0 is set. `pakota=false`.
- `muutos=1` still comes from the delete and re-insert, so the other connections do receive the row.

### What the original got wrong
- N-14:
  - It cites `dbboxm/Dbbox.cpp`, which is **not built** in either build (`RADStudio10/DBboxm-XE.cbproj:144-166`, `VS/Libs/dbboxm/dbboxm.vcxproj:112-118` use `dbbox1.cpp`). The logic there is the same.
  - It misses that `d=-1` drops the free list.
  - "Next two addtall" understates it: the third add probably gets the header.
  - The severity should be high, not medium.
- N-16: it does not account for the esiluenta save path resetting status, result and start by design.
- N-17: it overstates "every mode except 99". Modes 2/3 avoid the `badge[0]` conflict, and mode 0 auto-opens Muutokset.
- N-18: it misses the relay silent re-series to 0 and HK `sijat(-1)`. "Core refuses" applies to HK only.
- N-15: it misses the `:1511` caller.
- N-20: its "connection 0 … connection 1" wording means the same link; it also omits the `estavalitys` side effect.

### Unresolved questions (need runs)
1. Does HK `kilparr[-1]` really crash? Heap layout, debugger.
2. The third add after a double delete: does it hit the header? Logging run.
3. Does `remjarjarr` on a record already unlinked corrupt the rank chains?
4. What is `nilm[-1]` / `Sarjat[-1]` in the actual bcc32 link layout? It decides the view-mode effect of N-18.
5. N-17: does the conflict appear in modes 0 and 1 when the bib is typed? Is the Muutokset dialog modal?
6. N-19: confirm the VCL exception and the lingering lock count with a two-day event that has a one-day series.
7. N-15: confirm X returns 2 in this C++Builder version.
