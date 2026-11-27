# C-7 step 1, part B supplement: gaps G1-G4 (static, read-only)

(defect-concurrency-auditor, Sonnet. Copied verbatim from the agent's hand-back message.)

Paths are under `C:\dev\tulospalvelu\TPsource\V52`. Tags: [V] read in code, [I] inferred. Nothing was run or compiled. N-1..N-13 are not repeated.

**Summary**
- 15 new findings, N-14..N-28.
- The four most important: N-14, N-15, N-16 and N-17 (the first three are [V] in code, N-17 is [I]).
  - N-14: "Poista kilpailija" has no guard on `dKilp` or on an already-deleted record, in both forms.
  - N-15: closing a `select3` dialog with the window X acts as "button 2". This is destructive in several Emit dialogs.
  - N-16: `lueKilp3` overwrites an existing competitor's status, result and start time with the placeholder's blank values.
  - N-17: the placeholder record is never removed.
- G4: 5 of the 7 observations are defects; 2 are not.
- G1(b) is not a defect.

## (1) New findings

| ID | Title | Units | Sev | Evidence | Reach | Status | Extends |
|---|---|---|---|---|---|---|---|
| N-14 | "Poista kilpailija" calls `deltall(dKilp,0)` with no check for `dKilp<=0` or an already-deleted record. This can crash, or put a record on the free list twice. | cbHk and ViestiWin `UnitKilpailijatiedot` | medium (HK `-1` variant is a likely crash) | HK `cbHk/UnitKilpailijatiedot.cpp:1746-1752`, `:1518` (`dKilp=-1`), `:1738` (`dKilp=0`), `:1162`, `:1166-1176`; relay `ViestiWin/UnitJoukkuetiedot.cpp:1365-1371`, `:1241`, `:82`, `:1356`; `Hk/HkDat.cpp:597-601`, `:612`, `:640-649`, `:741-766`; `dbboxm/Dbbox.cpp:200-207`; `Juk/vdat.cpp:164-169`, `:907-939`; no constructor in `Juk/VDef.h:734` | See detail. | [V] code, [I] consequence | new |
| N-15 | `select3` returns `mrCancel` (2) when closed with the X, the same value as button 2. Several Emit dialogs treat 2 as the destructive choice. | cbTp `select3` with cbHk and ViestiWin `UnitEmit` | medium | `cbTp/cbTpApu.cpp:263-276`; `cbTp/UnitSelect3.cpp:42-59`; `cbTp/UnitSelect3.dfm` (no Cancel button, no `OnClose`); callers in detail | See detail. | [V] callers, [I] VCL behaviour | new (Q5) |
| N-16 | `lueKilp3` copies status, result, start time and badge from the placeholder into an existing competitor, guarding only against `'P'`. No confirmation. | cbHk `UnitEmit` | medium | `cbHk/UnitEmit.cpp:1663-1681`, callers `:1698-1712`, `:1727-1745`; `:203-206` vs `:272` | Emit form in `esiluenta` or `kuntoLahto` mode (the only modes where `EdtKilpno` is editable): read a card, type the wrong bib, Enter. | [V] | extends H-017 pattern (Q2) |
| N-17 | The placeholder (a vacant-slot competitor that owns the card) is never removed or released by `lueKilp3` or `tallennaKilpailija`. The core then refuses to move the badge. The exception is mode 99. | cbHk `UnitEmit`, `HkIV`, `HkDat` | medium | `cbHk/UnitEmit.cpp:1663-1681`, `:1615`, `:693-703`; `Hk/HkIV.cpp:1575-1612`; `Hk/HkDat.cpp:726`, `:947-952`, `:1056-1067` | See detail. | [I], needs a run | new (Q2) |
| N-18 | Series combo `ItemIndex` -1 is used as an index in both person and team forms. | cbHk `UnitKilpailijatiedot`, ViestiWin `UnitJoukkuetiedot` | medium (relay), low-medium (HK) | HK `:724-735`, `:1224-1235`, `:183-203`, `:254`; relay `:584-591`, `:977-1003`, `:119-154`, `:222-242`, `:823-830`; dfm `UnitKilpailijatiedot.dfm:470-486` | See detail. | [V] code, [I] effect | extends N-6 |
| N-19 | HK `naytaTiedot` ends with `PvGrid->Row = k_pv-epv+1`, which is out of range when `k_pv` lies outside the series stage span. The resulting exception skips `LeaveCriticalSection(tall)`. | cbHk `UnitKilpailijatiedot` | medium | `:477`, `:1550`, `:236`, `:196-203`; callers hold `tall`: `:1113-1153`, `:1164-1212`, `:1074-1087`; `Hk/HkSrj.cpp:306-311` | See detail. | [I] (VCL `InvalidOp` on `Row`) | new |
| N-20 | Relay `tall_rivi` is called with the arguments shifted: `pakLah` lands in `kielto` and `pakota_lah` gets `false`. | ViestiWin `UnitAjanotto` | low-medium | `ViestiWin/UnitAjanotto.cpp:1221`, `:1151-1153`, `:1288-1294`; `Juk/vajat.cpp:103-104`, `:247`; `Juk/VCom.cpp:239-243`, `:400-420` | Ctrl+Enter in the time grid. | [V] | new |
| N-21 | `SrjKarkiBtn` has no `break` outside tulospalvelu mode. It jumps to the last record of the series and refreshes the whole form for every record. Also, after "Lisää" and Tallenna the form moves away (low). | HK and relay | low | HK `:1089-1101`, `:1308-1318`; relay `:843-856`, `:1068-1080` | Series combo or "Sarja" button in setup mode. | [V] | new |
| N-22 | `Kilp.tall_tulos(k_pv, 0)` has `piste` and `tulos` in the wrong order. It is meant to clear the finish (`piste` = 0). | cbHk `UnitEmit` | low | `cbHk/UnitEmit.cpp:1588`, `:1597` (compare `:699`, `:1568` `(0, x)`); `Hk/HkTls.cpp:829-851` | `kuntosuunnmoodi==1` with `uusi=='U'`, or `esiluenta`, on stage `k_pv>0`. | [I] | new |
| N-23 | Relay `NewCardHandler` shows `MessageBoxW(msgtxt)` with an uninitialised buffer. It then returns without advancing `emithead2`. | ViestiWin `UnitEmit` | low (reach unproven) | `ViestiWin/UnitEmit.cpp:949-950`, `:1002-1006` (HK initialises `msgtxt[0]=0`, `cbHk/UnitEmit.cpp:~922`) | An unprocessed emit record with `badge<=0`. | [V] bug, [I] stall | new |
| N-24 | `naytaRivi` auto-assign (SyottoTila 1) writes the competitor from the display path. HK has no overwrite confirmation and reads uninitialised `uptr`. Relay consumes the memo number even when nothing is written. | cbHk and ViestiWin `UnitAjanotto` | low | HK `cbHk/UnitAjanotto.cpp:239-272`; relay `ViestiWin/UnitAjanotto.cpp:291-332` | See G4 table. | [V] | new (Q3) |
| N-25 | `teeRankiJarj` truncates to 1000 teams per class without a message. Teams beyond the first 1000 are not numbered. | ViestiWin `UnitArvonta` | low-medium | `ViestiWin/UnitArvonta.cpp:35`, `:154-181`, `:230-240` | Toiminto 3 (ranking numbering), series with more than 1000 teams. | [V] cap, [I] reach | new |
| N-26 | `CBsalanimiClick` reads `salausVal[2]` in a 2-byte array. It also edits `Kilp` outside edit mode. | cbHk `UnitKilpailijatiedot` | low | `:81`, `:234`, `:1895-1898`; `tputilv2/STSCHIND.cpp:32-41` | Pick "Ei salanimeä" in the combo. | [V] OOB read | new |
| N-27 | `BtnJjVaihtoClick` case 103: `wst[wcslen(wst-1)]` was meant to be `wst[wcslen(wst)-1]`. A trailing `'\|'` is not stripped. | ViestiWin `UnitJoukkuetiedot` | low | `ViestiWin/UnitJoukkuetiedot.cpp:1459-1462` | Edit mode, "Juoksujärjestysvaihdot", "Jaa nimet etu- ja sukunimiin" (`UnitJjVaihdot.cpp:57`). | [V] | new |
| N-28 | `EdtNimihakuChange` in HK has `RGSelaus->ItemIndex == 2;`, a comparison with no effect. | cbHk `UnitKilpailijatiedot` | low | `:1358` vs relay `ViestiWin/UnitJoukkuetiedot.cpp:1123` (assignment) | Name search in view mode. | [V] | new |

## (2) Detail

### N-14
- **Normal-use trigger.** The form is created with `dKilp = -1` (HK `:1518`, relay `:1241`), and "Lisää kilpailija" sets `dKilp = 0`. The "Tiedosto > Poista kilpailija" menu item is always enabled (`UnitKilpailijatiedot.dfm:1028-1031`). After one confirmation dialog it calls `deltall(dKilp, 0)` with no check of `dKilp` or of the record state.
- **How `dKilp` reaches -1 or 0.**
  - Open "Yksi osanottaja" and pick Poista before choosing anyone: `dKilp = -1`. `FormShow` does `if (dKilp) naytaKilpailija(-1)`, and -1 is truthy, but `naytaKilpailija` only acts when `d > 0`.
  - "Lisää", then "Peruuta": `BtnPeruutaClick` sets `Lisays = false` and calls `EdBtnClick`. `EdBtnClick` returns at once because `dKilp <= 0` (`:1162`). The form then shows `Kilp = Kilp1` (the previously shown competitor) while `dKilp` stays 0.
  - Relay only: typing an unknown team number in view mode sets `dKilp = 0` and `Kilp.nollaa()` (`UnitJoukkuetiedot.cpp:82-83`).
- **HK, `dKilp = -1`.**
  - `GETREC(-1)` only shows an error (`HkDat.cpp:597-601`) and the `kilptietue` constructor has zeroed the record.
  - Then `remjarjarr` decrements `nyilm` for series 0.
  - Then `delfrec(-1)` runs `deleterec(&datf2,-1)` and `kilp.PUTREC(-1,0)`. `PUTREC` only checks `d > maxrec` (`:612`), so it writes `kilparr[-1]`, a heap underflow with a garbage `pv` pointer. A crash is likely.
- **HK, `dKilp = 0`.** `deltall(0)` is valid input. `deleterec(&datf2,0)` puts the header record on the free list (`Dbbox.cpp:200-207`) and `laheta` sends a delete for kilpno 0.
- **Double delete.** After a successful delete, `EdBtnClick` is meant to move away. In the default mode ("Sijoitus", `ItemIndex=0`, dfm:943) it cannot, for two reasons:
  - An untimed competitor has `p_sija` = 0 (`HkTls.cpp:1134-1146`).
  - For a timed one, `deltall` has already removed the record from the rank chain, so the walk never meets `dKilp` and `p` becomes -1 (`:1171-1174`).
  - The form keeps showing the deleted competitor with `dKilp > 0`. The user sees nothing happen and clicks Poista again.
  - The second `deltall` runs `remjarjarr` and `nyilm--` again. `delfrec` then makes `firstfree = d` while `d` points to itself, so the next two `addtall` calls get the same record, and one competitor overwrites the other.
  - Modes "KilpNo" and "Nimi" do move away, and `OoGridDblClick` forces mode 2 only in setup mode.
- **Relay.**
  - `deltall` takes `tall` itself (`vdat.cpp:913`).
  - Relay `kilptietue` has no constructor, so the local `kilp` is uninitialised.
  - `GETREC` ignores `d<0` (`vdat.cpp:164-169`).
  - `Sarjat[kilp.sarja]` and `paivJoukkLkm(kilp.sarja,-1)` therefore run on stack garbage.
- **Verify.** Run it: open the form, Poista, Yes. And: delete an untimed competitor in mode 0, then Poista again.
- **Fix direction.** Guard `dKilp > 0 && Kilp.kilpstatus==0`. After a delete, reset `dKilp` and `Kilp`. Make `PUTREC` reject `d<0`.

### N-15
- `select3` returns the result of `ShowModal`. The buttons set `ModalResult` to 1, 2 and 3. There is no Cancel button and no `OnClose` handler. Closing a VCL modal form with the X sets `mrCancel` = 2 [I], which equals button 2.
- **Dialogs where 2 is the destructive choice:**

| Dialog | Evidence | Option 2 | Effect of X |
|---|---|---|---|
| HK `SiirraKilpailijalle` | `cbHk/UnitEmit.cpp:2586-2590` | "Siirrä menettäen yhteys aiempiin leimaustietoihin" (code cancels only on `Vast<=1`) | Transfer is executed. |
| Relay `SiirraKilpailijalle` | `ViestiWin/UnitEmit.cpp:2164-2170` | Same | Same. |
| `BtnLoppuunClick`, HK and relay | `cbHk/UnitEmit.cpp:3664-3677`, `ViestiWin/UnitEmit.cpp:1978-1990` | "Hylkää muutokset" (`default:` branch calls `Nayta()`) | Unsaved edits are discarded. |
| HK `Paivita`, mode 99 | `cbHk/UnitEmit.cpp:607-627` | "Ota kortti uuteen käyttöön" | The old holder's badge is reassigned and saved (`:617-619`). |
| Relay `Ajanotto` | `ViestiWin/UnitAjanotto.cpp:646-651`, `:1203-1210` | "Tallenna numero, mutta ei tulosta" | Saves; the user meant cancel. |

- **Verify.** Run it and close the dialog with the X.

### N-16
- `lueKilp3` reads the target competitor (`kilp1`) and rejects it only if `tark == 'P'`. It then copies the placeholder's blank values into it:
  - `set_tark(Kilp.tark)`.
  - `tall_tulos(0, Kilp.p_aika(0))`.
  - `tall_lajat_pv(Kilp.p_aika(-1))`.
  - `badge[0] = placeholder badge`.
  - `vac = 0`.
- It then does `Kilp = kilp1`. `tallennaKilpailija(false)` follows at once for plain Enter (`:1744`). Ctrl+Enter saves twice, with `kysy = (esiluenta==0)` the second time.
- Typing the bib of a competitor who already has a result, a DNF status (`'H'`/`'K'`) or another badge overwrites those fields without any warning. `Em.time` or `ctrl` data are not consulted.
- Fix direction: refuse or ask when `kilp1.p_aika(0) != 0` or `tark` is not in `"-IV"`.

### N-17 (answers the HK `lueKilp3` question)
- **What the placeholder is.** In the GUI the placeholder is not the console `add()` record (`HkEmit.cpp:1195-1266`, console path at `HkIV.cpp:1460`).
  - It is a vacant-slot competitor chosen by `haeSeurVakantti`, or a new vacant one when no slot is free.
  - It is given the badge, `uusi='U'` and `vac='V'`, and saved with `tallenna(d,0,0,0,NULL)` (`HkIV.cpp:1580-1612`).
  - `addtall` and `tallenna` register the badge as `bdg_kno[B]=P` (`HkDat.cpp:726`).
- **What `lueKilp3` does with it.** It never deletes or releases the placeholder. `DKilp` simply moves to the real competitor `R`.
- **Refusal in the core, in every mode except 99.**
  - `tallennaKilpailija` calls `tallenna(DKilp,0,0,0,0)`, so `kielto=0` and `kysyfl=1` (`HkDat.cpp:950`).
  - The core sees `bkno = bdg2kno(B) = P` (`:1056-1067`).
  - It prints "Kilpailijalla P on jo e-koodi B - poista ennen muutosta" and reverts `R`'s badge.
  - `R` is saved with the copied status and time but without the card.
  - `P` keeps card B, so a later read of B is matched to `P`, a ghost vacant record.
- **Mode 99 is the exception.** `Paivita` branch `:693-703` first turns `P` back into a vacant slot (`badge[0]=0`, `tark 'V'`, `setSarja(vakanttisarja)`, `tallenna(d...)`), which releases B.
- **Not proven.** Whether `bkno != 0` really occurs in modes 1, 2 and 3 and in plain `esiluenta`, and whether the error dialog blocks. Needs a run with an unknown card, then typing the bib.

### N-18
- **Both series combos are editable** (no `Style` in the dfm). The relay one is `UnitJoukkuetiedot.dfm:~405-413`; the HK one is `UnitKilpailijatiedot.dfm:470-486`. Typing text that matches no item gives `ItemIndex=-1`.
- **View mode.** `SrjValChange` runs `nilm[ItemIndex]` (HK `:1227`, relay `:982`). If `nilm[-1]` happens to be positive it sets `Sarja = -1` and calls `SrjKarkiBtn`, which uses `nilm[Sarja]` and `jalku[-1][...]` (HK `:1077`, relay `:830`). Typing a single letter is enough.
- **Edit mode.**
  - `SrjValExit` stores -1 in the record: relay `Kilp.sarja = -1` (`:587`), HK `setSarja(-1)` (`:727`).
  - It then runs `GridDims()` and `naytaTiedot()`, which read `Sarjat[-1].osuusluku` and `.yosuus[]` (relay `:126-145`, `:242`) or `enspv`/`viimpv` (HK `:201-202`, `:254`).
  - These reads are garbage, and relay `vOs` and the grid row counts are garbage.
  - The core `tallenna` does refuse a record with no series (`HkDat.cpp:932-939`), but the display has already run.
- **Relay `CBSelOsuus`** is also editable and its `OnChange` is `SrjKarkiBtn`. Typing text gives `SelOsuus = -1` (`:823`), which is then used in `jalku[Sarja][-1]`, `ntulos[Sarja][-1]` and `JarjSeur(0,-1,...)` (`:830`, `:874`, `:923-927`). `SrjValChange` also clamps with `SelOsuus = ntosuus` (`:999-1001`), one past the last leg, though that only matters if `ntosuus` shrinks.

### N-19
- `PvGrid->RowCount = vpv-epv+2`, so the valid rows are 0..`vpv-epv+1`. The last statement of `naytaTiedot` sets `Row = (k_pv-epv+1)==0 ? 1 : k_pv-epv+1`. Only the exact value 0 is handled.
- `epv`/`viimpv` come from the series file (`HkSrj.cpp:306-311`, per series). With `sarjavaihtelee` false, `epv=enspv` and `vpv=viimpv`.
- A competitor in a series that ends before `k_pv`, or starts later (`enspv >= k_pv+2`), gives `Row` outside 0..`vpv-epv+1`. VCL `MoveCurrent` raises `EInvalidGridOperation` "Grid index out of range" [I].
- Consequences:
  - The exception aborts the form update after the fields have been filled.
  - `SeurBtnClick`, `EdBtnClick` and `SrjKarkiBtn` call `naytaTiedot` between `EnterCriticalSection(tall)` and `LeaveCriticalSection` with no `try`/`finally`. The exception skips the `Leave`, and the main thread keeps `tall` for good (recursive, so it is not seen by itself). Worker threads that need `tall` then block (see N-3 for the same consequence).
- Reach: multi-day competitions where some series race only some days.
- **Verify.** A run with a two-day event and a one-day series, selecting that series on day 2.

### N-20 (G4 item 1)
- Relay signature: `tall_rivi(ino, utm, vtm, uptr, kysy, kielto, pakota_lah)`.
- The call `tall_rivi(Jono,&upt,&pt,&uptr,0,pakLah,false)` therefore gives `kysy=0`, `kielto=pakLah`, `pakota_lah=false`. `pakLah` is the Ctrl+Enter flag (`:1151-1153`, set only when `memcmp(&pt,&upt)==0`).
- With `kielto=1`, `tarkkielto(cn,1)` returns 1 for `cn == kielto-1`, so `laheta_aika` skips connection 0 (`VCom.cpp:239-243`, `:400-420`).
- Result: the forced resend of the time row neither sets the `pakota` flag nor reaches connection 1. With a single peer on connection 1, nothing is sent.
- The delete-and-insert in `tall_rivi` still sets `muutos=1`, so a normal send happens, but not to connection 1.
- The competitor record is still sent forced: `Tallenna(...,pakLah,...)` at `:1244` has the correct slot.
- HK is correct: `cbHk/UnitAjanotto.cpp:1226` passes `(…,0,0,pakLah,false)`, and its signature has the extra `InCriticalSection`.
- The relay call looks like a copy of the HK call with one argument dropped.

### N-21
- `SrjKarkiBtn` (HK `:1091-1101`, relay `:845-855`) loops over all records. It never breaks, and calls `naytaTiedot` (relay also `GridDims`) for every record in the series. It ends on the last record of the series. tulospalvelu mode (`ToimintaTila==2`) correctly takes the first by rank.
- With hundreds of records this is a slow refresh.
- Also, after a successful "Lisää" and Tallenna, `BtnPeruutaClick` calls `EdBtnClick` (HK `:1308-1312`, relay `:1068-1075`). In KilpNo or Nimi mode the form then jumps to the previous competitor instead of staying on the new one. This is cosmetic.

### N-22
- `tall_tulos(INT piste, INT32 tls)`. The calls `tall_tulos(k_pv, 0)` pass `piste=k_pv` and `tls=0`.
- On stage 0 this equals `(0,0)` and is correct. On stage ≥1 the finish time is not cleared. With `valuku[k_pv] >= k_pv` an intermediate split is cleared instead.
- Evidence: `:1588` (mode 1, `uusi=='U'`) and `:1597` (`esiluenta`). The other calls use `(0, x)`.

### N-23
- `wchar_t msgtxt[200]` is declared at `:950` and never assigned in `NewCardHandler`. The `bdg<=0` branch shows it in a `MessageBoxW` and returns before `emithead2++` or `ProsEmit`, so the next card event hits the same record again.
- How a record with `badge<=0` reaches this path is unknown. `ProsEmit` handles `Em.badge==0` at `:902`, but it cannot be reached from here for such a record.

### N-24
- HK:
  - The trigger is `IkkParam.SyottoTila==1`, the last queue row (`rw==aktrows-1`), `pt.kno==0` and a valid time. `naytaRivi` takes the next valid bib from `Memo1`, then writes the row with `tall_rivi` and the competitor with `set_tulos` and `tallenna` (`:253-264`).
  - There is no overwrite confirmation, unlike the manual path (`:1204-1208`).
  - `uptr` is uninitialised (`:254`) but is read inside `tall_rivi` (`vajat.cpp:152`, `:225`, same pattern in HK). This is H-069 style.
  - `piste > valuku` is silently dropped (same cause as N-7).
- Relay:
  - If `os` is out of range for this team's series (`CBOsVal->ItemIndex-2` ≥ `osuusluku`), `setMaali` is a no-op but `aktRivi1++` runs and the memo line is already deleted (`:305`).
  - If `haeseurosuus` returns -1, the row is assigned to leg 0 with no write to the competitor (`:313-332`).

### N-25
- `ix_jrj[1000]` (`:35`) and `if (n > 1000) n = 1000` (`:171`): sorting first, then truncating, so the lowest-ranked teams are dropped.
- In `SeurPos` case 3, `kk < n_ix_jrj` ends the loop and `d=-1` follows. The caller treats this as "series done", so the rest are not renumbered. There is no message.
- Series with more than 1000 teams (Jukola-sized classes) are realistic.

### N-26 and N-28
- N-26: `salausVal[2]="YJ"` has no terminator. `(ItemIndex+2)%3` is 2 for "Ei salanimeä", so one byte past the array is read and stored in `pv[0].salaa`. `nimi()` only tests `'Y'` and `'J'`, so garbage mostly behaves as no alias. The click handler also works in view mode, so a changed `Kilp` is saved later with the next edit.
- N-28: in HK `EdtNimihakuChange`, `RGSelaus->ItemIndex == 2;` is a comparison with no effect. The relay form has `RGSelaus->ItemIndex = 2` (`UnitJoukkuetiedot.cpp:1123`), so the intent was to switch to name order after a name search.
  - Effect: after a name search the Seur/Ed buttons keep their previous order. The default is rank order, which continues from the found competitor, so it is consistent but not the order expected. Selecting "Nimi" by hand works through `nameindex.pos`.

### N-27
- `wst[wcslen(wst-1)]` reads `wst[-1]`, the wchar before the array on the stack.
  - If that wchar is non-zero, the index is `len+1`: usually inside the array (reading after the terminator). If `len == OSNIMIL` it is one past the array.
  - If it is zero, the index is 0.
- The wanted last-char check effectively never matches, so a trailing `'\|'` is not removed. The following loop turns it into a space, which `wesnimet` trims.
- A write happens only when the stray read equals `'\|'`, and then only writes 0. No crash expected.
- Reach: edit mode, `BtnJjVaihto`, `ModalResult` 103 on the "Jaa nimet" button (`UnitJjVaihdot.cpp:57`).
- Severity: low.

## (3) G4 verdicts

| Observation | Verdict | Severity | Notes |
|---|---|---|---|
| Relay `UnitAjanotto.cpp:1221` argument shift | Defect (N-20) | low-medium | `kielto=1` skips connection 0; no force flag. HK is correct. Competitor resend still works. |
| HK `UnitAjanotto.cpp:719-725` `Key=0` then `Key==L'E'` | Dead code, defect | low | `Vaihdaer1Click` ("Vaihda erä", prompt for heat number, sets `era_akt`) is still reachable from the menu item "&Vaihda erä" (dfm:931, no ShortCut). Ctrl+E was meant to open it. The Ctrl+E clause is also commented out at `:719`. |
| `naytaRivi` writes the queue row and the competitor | By design, with weaknesses (N-24). Not an "old row re-applied" defect | low | Trigger: SyottoTila 1, last row, `kno==0`. After the first assignment `pt.kno != 0`, so a refresh does not re-apply. HK has no overwrite confirmation and an uninitialised `uptr`. Relay can consume the memo number without writing. |
| Relay `NewCardHandler` `msgtxt` | Defect (N-23) | low | Uninitialised in relay; HK initialises it. |
| `teeRankiJarj` `ix_jrj[1000]` | Defect, realistic only for Jukola-sized classes (N-25) | low-medium | Silent truncation after sorting. |
| Relay `kilptietue` constructor | There is none (`Juk/VDef.h:734`, no `kilptietue(` in `Juk/*`) | n/a | Local records are uninitialised stack data; VCL form members are zeroed by `NewInstance`. This changes N-2: `vkilp` in `TallennaVaihto` is a local (`UnitEmit.cpp:2288`). With `vd=-1`, `vkilp.getrec(vd)` does nothing, so `Kilp_0.ostiet[Osuus].badge/laina` are overwritten from stack garbage (`:2326-2329`) and saved (`:2330`), not zeros as N-2 assumed. It also affects `HaeKilpailija` (`:734-740`) and `deltall`. HK has a zeroing constructor (`HkDat.cpp:204-209`). |
| HK `lueKilp3` and `tallennaKilpailija`: placeholder removed? | Not removed (N-17) | medium [I] | The relevant placeholder is the GUI vacant slot, not the console `add()` copy at `HkEmit.cpp:1240-1266`. `lueKilp3` never deletes it. Only mode 99 `Paivita` releases it. |
| G1(a) `RGSelaus->ItemIndex == 2;` | Defect, no effect (N-28) | low | See N-28. |
| G1(b) `keyn` uninitialised in case 2 | Not a defect | n/a | `nextix` and `previx` only write `key` as output (`tputilv2/dbindex.cpp:121-153`) and use `idx->pos` for the cursor, which `EdtNimihakuChange` has set via `searchix`. The first call in HK `SeurBtnClick` case 2 (`keyn` uninitialised) therefore does no harm. |
| G2 case 103 `wst[wcslen(wst-1)]` | Defect (N-27) | low | Out-of-bounds read of one wchar; no meaningful write. |

## (4) Coverage lines

**G1 `cbHk/UnitKilpailijatiedot.cpp`** (now read in full except 540-585 comments)
- Q1:
  - `EdtKilpnoChanged` has a range check (`:122`) and a duplicate check only in `ToimintaTila==2` (`:126`). Setup mode allows duplicates and 0, but `tallennaTiedot` refuses duplicates at save (`:618-622`).
  - `naytaTiedot`: Sarja -1 is N-18, Row is N-19.
  - `EdtSynt` and `EdtIkasrj`: no range checks (a 3-digit year is accepted). Trivial.
  - `EdtSp`: handles empty text.
  - `Lisenssinumerohaku1Click` and `Perusnumerohaku1Click`: the `d<nrec` loops are fine.
  - Other defects: N-14, N-18, N-19, N-21, N-26, N-28.
- Q5:
  - `BtnPeruuta` with `Lisays` leaves `dKilp=0` (N-14).
  - `FormClose` handles `Lisays` with `dKilp>0` correctly; the "Yes" branch saves, "No" restores.
  - `SrjKarkiBtn` has no `tall` in the non-`ToimintaTila==2` branch and no break (N-21).
  - The form uses `Kilp1`/`Kilp2` compare at save: fine (see N-10).

**G2 `ViestiWin/UnitJoukkuetiedot.cpp`**
- Q1:
  - `EdtKilpnoChanged` view mode: unknown number gives `dKilp=0`, a cleared form (`tyhjennaKaavake`) and `salliNumeromuutos=false`.
  - Edit mode with `salliNumeromuutos`: `haeSeurVapaaKno(KilpNo)` picks the next free number, no range check; fine, since `tallennaTiedot` checks duplicates and zero (`:506-516`).
  - `paivitaVaMuutos`: indices `os=col-1`, `va=row` are inside the grid, and `setMaali` rejects out-of-range.
  - `naytaTiedot` col 9 `LLaji[lahtolaji+1]`: refuted (`lahtolaji` is UINT8 and normalised at load, `vdat.cpp:297`).
  - `EdtNimihakuChange` has no `d>0` guard on `searchix` (HK has one). With an empty index, `d` is read before the array. Rare.
  - Other defects: N-14, N-18, N-21, N-27.
- Q5: `tallennaTiedot` conflict check is inside `tall` (`:518-525`); `Lisays` cancel leaves `dKilp=0` (N-14); `BtnJjVaihto` works on `Kilp` and keeps the dialog modal, so no stale copy beyond `Kilp1`.

**G3 Emit `Paivita`/`NaytaEmit` (HK) and `ProsEmit`/`HaeKilpailija`/`KysyKilpailija` (relay)**
- Q1: `NaytaEmit` bounds are fine (`nc ≤ MAXNLEIMA`, `RowCount=MAXNLEIMA+1`, `tulkinta[MAXNLEIMA+1]`). `rt->rastikoodi[abs(tulkinta)-1]` relies on `tarkista` giving the same course as `haerata`. Not checked further.
- Q2:
  - HK mode 99: branch A (`:603-635`) and branch B (`:693-717`) only run when `Kilp.kilpno` is set, so `d` (`:573`) is valid. Branch B turns the placeholder into a vacant slot and releases the card, as in N-17.
  - After the mode-99 dialog the code always runs `Em.kilpno=0; Em.badge=0; Em.put(...)` (`:628-630`). Because of N-15, closing the dialog with the X becomes option 2.
  - Relay `HaeKilpailija`: highest-leg search with `val_korkein`. With the same card on two legs and `val_korkein`, the second read tests the highest leg again (`:745`), finds it linked, and falls to a manual query with the misleading text "Tuntematon koodi" (`:778`). Low, probably by design.
  - Relay `ProsEmit`: `Em.osuus` has no upper bound check, but values from `HaeKilpailija` and the Muutokset dialog are valid. `emithead2++` is done before processing; `KysyKilpailija` leaves the record unwritten in the file (unprocessed), which is the design.
  - Relay `Paivita` is plain loading; no defect.
- Q1/Q5: `NaytaEmit` and relay `Paivita` are display-only; no writes.

## (5) Refuted suspicions
- Uninitialised `keyn` into `nextix` (G1b): `key` is an output only.
- Relay `LLaji[lahtolaji+1]` out of bounds: `lahtolaji` is UINT8 and clamped to 0 when >2 at load (`vdat.cpp:297`) and in `paivitaMuutos`.
- `naytaRivi` re-applying an old row over a corrected result: not possible, `kno` is set after the first assignment.
- Relay Ctrl+Enter resend failing for competitor data: the competitor `Tallenna` call has `pakLah` in the right slot (`:1244`). Only the time row is affected (N-20).
- Relay `ProsEmitMaali` `TMAALI0/KSEK` vs `TMAALI0*10/SEK`: equal (`KSEK=10*CSEK`, `SEK=10*KSEK`, `TpDef.h:81-82`).
- HK `Paivita` using `d` unset at `:702`: refuted, `Kilp.kilpno` non-zero implies `d` was set at `:573`.
- Relay `emitLuettu`/`HaeTulostukseen` with `Osuus=-1`: there are no callers in the relay build (`HkIV.cpp:1091` is HK), so not reachable.

## (6) Open questions
1. N-17: does the badge conflict message (`HkDat.cpp:1056-1067`) fire in the esiluenta and kuntoLahto flows without mode 99? Needs a run: unknown card, type bib, Enter.
2. N-15: confirm that the X on `TFormSelect3` returns 2 in this C++Builder version (standard VCL `mrCancel`). Needs a run.
3. N-19: confirm the VCL grid raises on an out-of-range `Row`, and that a series with `viimpv<k_pv` can be shown (series file with different stage spans).
4. N-14: the HK `-1` crash and the double-delete free list need a run.
5. N-23: what produces an unprocessed relay emit record with `badge<=0`?
6. N-25: whether ranking numbering (Toiminto 3) is used for classes with more than 1000 teams in practice.
7. Mode semantics of `kuntosuunnmoodi` 1/2/3 remain undocumented; N-22 and N-17 depend on them.
