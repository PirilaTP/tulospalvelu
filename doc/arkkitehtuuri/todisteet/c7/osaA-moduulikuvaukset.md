# C-7 step 1, part A: GUI form module descriptions (read-only)

Paths are relative to `TPsource/V52/`. Labels: **[V]** = verified, read in code. **[I]** = inferred, chain not fully traced.
Both GUI projects compile their own copy of every unit below: HK units come from `RADStudio10/HkKisaWin.cbproj`, relay units from `RADStudio10/ViestiWin.cbproj`. [V]
The units are not shared between programs; the pairs are copy-paste forks. The shared core is `Hk/` for HK and `Juk/` for relay.

## 0. Summary

- Project membership [V]:
  - HkKisaWin.cbproj compiles `cbHk/UnitAjanotto.cpp` (line 417), `UnitArvonta.cpp` (422), `UnitEmit.cpp` (461) and `UnitKilpailijatiedot.cpp` (539).
  - ViestiWin.cbproj compiles `OsuusTiedot.cpp` (667), `UnitAjanotto.cpp` (693), `UnitArvonta.cpp` (698), `UnitEmit.cpp` (730) and `UnitJoukkuetiedot.cpp` (796).
- Relay counterpart of `cbHk/UnitKilpailijatiedot`:
  - **`ViestiWin/UnitJoukkuetiedot.cpp` (`TFormJoukkuetiedot`) is the counterpart.** It has the same structure: `EdtKilpnoChanged`, `naytaTiedot`, `tallennaTiedot`, `paivitaMuutos`, `asetaMuokkaustila`, `SeurBtnClick`, `EdBtnClick`, `Liskilpailija1Click`, `Poistakilpailija1Click`. It has the same `Kilp`/`Kilp1`/`dKilp` edit-copy scheme and the same `addtall`/`Tallenna(...,&Kilp1)`/`deltall` write paths. It edits the whole team record and the per-leg `ostiet[]` grid in place of HK's per-day `pv[]` grid.
  - **`ViestiWin/OsuusTiedot.cpp` (`TFormOsuus`) is not a counterpart.** It is a read-only per-leg viewer. `naytaTiedot` reads the record with `GETREC` (OsuusTiedot.cpp:68ff), and the navigation buttons use `JosAlku`, `OsJarjSeur`, `entossija`, `JarjSeur` and `entsija` under `tall_CriticalSection` (OsuusTiedot.cpp:272-395). There is no `tallenna`, `addtall` or `setMaali` call in the file (grep gave 0 hits). Its only edit-like action is `eeanalyysi1Click`, which calls `uusiemitanalyysi(true)` (OsuusTiedot.cpp:470). The HK analogue of the leg view is the sub-forms `FormKilpailijaEmit` and `FormKilpailijaOnline`, which `UnitKilpailijatiedot` opens (cbHk/UnitKilpailijatiedot.cpp:1425-1444). Those units are outside this batch.
- Big structural difference in Emit: HK card processing happens in core `pros_emit` (`Hk/HkIV.cpp:1162`), which the form calls from `NewCardHandler` (cbHk/UnitEmit.cpp:936). In the relay GUI the same logic lives in the form itself: `ProsEmit`, `ProsEmitMaali`, `HaeKilpailija` and `KysyKilpailija` (ViestiWin/UnitEmit.cpp:732-945). [V]
- Locking pattern [V]: HK forms take `tall_CriticalSection` themselves around `kilptietue::tallenna`, because the HK `tallenna` does not lock (its `EnterCriticalSection` lines are commented out, `Hk/HkDat.cpp:680,737`). Relay `tallenna` locks internally (`Juk/vdat.cpp:496-850`), so the form-level lock is a recursive nested lock. Details belong to C-6 (saikeet-ja-lukot.md); not re-analysed.
- Unlocked HK write sites: `cbHk/UnitEmit.cpp:3449` and `:3483` call `tallenna` without the lock (H-010, H-017). `UnitEmit.cpp:1615` (`tallennaKilpailija`) takes the lock but only for an already-copied record.

---

## 1. cbHk/UnitEmit.cpp (`TFormEmit`, 3798 lines), HkKisaWin

### 1.1 Responsibility
Card (emit/badge) handling form for the HK program. It shows the newest card record (`emittp`), the competitor linked to it, and the checked punches in `EmitGrid`. It lets the user:
- link a card to a competitor (also by name search or from the person register, `FormTKanta`);
- fix punches and times in `FormEmitMuutokset` (UnitEmitMuutokset.cpp calls back into `FormEmit->TallennaMuutokset` etc., lines 36-320);
- calculate the finish time and result;
- print punch reports and intermediate results.

It also hosts several "fitness orienteering" (kuntosuunnistus) modes, selected by the globals `kuntosuunnmoodi` (1, 2, 3, 98, 99), `esiluenta` (1, 2), `kuntoLahto` and `kuntoMaali` (InitTila 199-320, FormShow 3316). [V] Exact semantics of each mode value: [tuntematon].

### 1.2 Main logic functions
| Function | File:line | Note |
|---|---|---|
| `NewCardHandler` | UnitEmit.cpp:880 | `WM_MYMSGNEWCARD` handler. `wparam>0` means print-only card; otherwise it calls core `pros_emit(&EKilp,msgtxt)` (:936), sets `UusiTietue`, then `HaeTietue(emithead2-1)`. Guards: `KasittelyKesken`, `EmitMuutosFlag`, `OnkoMuutoksia`. [V] |
| `emitLuettu` | :966 | Free function called from reader threads; `SendMessage(FormEmit->Handle,...)` without NULL check (H-107). [V] |
| `DisplTimerThread` | :834 | `_beginthread` (:891, :953) display-fade thread, sends `WM_MYDISPLTIMER`. Threading analysis is C-6. [V] |
| `HaeTietue` / `OtaTietue` | :973 / :1001 | Navigate emit records with `em_HaeSeuraava`/`em_HaeEdellinen`/`em.get(Tietue)`, then call `Paivita`. `OtaTietue` marks `eiLiitetty` when `getEmitJarr(em.kilpno)!=pos` (:1016). [V] |
| `Paivita(emittp*)` | :563 | Loads `Em`, finds the competitor (`getpos`, `Kilp.GETREC`). For a new card with `kuntosuunnmoodi==99` and an already-registered badge it asks (`select3`) and may re-issue the badge and save (:618-630). For a new unregistered card it queries the person database (`FormTKanta->NaytaHenkilot`, :650ff). In mode 99 it makes a vacant competitor (`setSarja(vakanttisarja)`, `tallenna` :702). [V] |
| `HaeTulostukseen` | :846 | Reprint path: `bdg2kno` (:851) then `getem` (:868) then `Tulosta(4)`. [V] |
| `tallennaKilpailija` | :1479 | Copies the form fields into `Kilp`, and saves with `Kilp.tallenna(DKilp,0,0,0,0)` (:1615) inside `tall_CriticalSection`. Contains the special mode logic: `kuntosuunnmoodi==1` start/finish interpretation (:1496-1518), badge rotation `haeSeurVapaaBadge` (:1579), `putem` (:1583,:1593,:1608), `setEmitJarr(-1,..)` (:1589). Returns 1 if no class chosen. [V] |
| `EdtKilpnoKeyDown` | :1684 | Enter: link the typed number to the card via `lueKilp3` then `tallennaKilpailija`; Ctrl+Enter: save with confirmation. [V] |
| `lueKilp3` | :1663 | Copies result and start times (`tall_tulos`, `tall_lajat_pv` :1672-1673) from the old to the new competitor record in memory (no write here). [V] |
| `LaskeTulos` | :2374 | If `Sarjat[].laskemtb[k_pv]` is set it calls `laskeEm_VA` (:2381), otherwise `e_maaliaika` (:2404, :2411) and `kilp.set_tulos` (:2429). The copy is modified in memory and only shown in the fields. Handles `kaksibadge==2`. [V] |
| `TallennaMuutokset` | :2557 | Re-link a card to another competitor (called from FormEmitMuutokset). Detailed in 1.3. [V] |
| `SiirraKilpailijalle` | :2513 | Prepares the in-memory move: `Kilp_0`/`Kilp` badge swap, `Emitvaihdottuleville` copies the badge to later days. No write. [V] |
| `MerkitseOhitetuksi` | :2722 | "Mark as skipped": clears the old competitor's badge (or turns them into a vacant) and `Em.kilpno=0`. [V] |
| `TallennaVaihto` | :2752 | Swap badges of two competitors via a vacant record. [V] |
| `TallennaEmMuutokset` | :2812 | Writes the edited punch grid back into `Em` and `Em.put(Tietue,true)` (:2826). [V] |
| `BtnOhitaClick` | :3478 | Clears `Em.kilpno/badge`, `Em.put` (:3482), `EKilp.tallenna` (:3483, **no lock**, H-017). [V] |
| `BtnPerKirjausClick` | :2995 | Cancel registration: badge=0, `set_tark('E')`, `tall_lajat_pv(TMAALI0)`, `Kilp.tallenna` (:3001). [V] |
| `Laskeuudelleen1Click` | :3372 | Bulk recompute for all emit records: `e_maaliaika`, `tarkista`, `set_tark`, `set_tulos`, `kilp.tallenna` (:3449, no lock). [V] |
| `Vaihdaluentaajat` | :3685 | Bulk rewrite of `em.time`/`em.maali` from competitor finish times: `em.put` (:3712). [V] |
| `Kirjoitakopioemittiedoista1Click` | :3622 | Raw copy of `emitfile` under `emit_CriticalSection` (reads `emitfile` directly). [V] |
| `Automaattinentulostus1Click` / `Automaattinenvliaikatuloste1Click` | :3094 / :3154 | Sets globals `autorap`, `comautorap` (1 = reject-report, -1 = intermediate print). [V] |
| `Tulosta` | :3021 | `emit_rap`, `tark_rap`, `emitlista`, `tulostaRastiVa` against `EmitTulPrm` (GDI printer). [V] |
| `Etsileimojavastaavarata1Click` | :3217 | `etsierata(&Em)` then offers class/course change. [V] |
| `BtnHaeSeur/HaeEd/HakuEhto` | :2063/:2123/:1978 | Search emit records by name/number/badge/index/"EmitErot" (diff list). [V] |
| `OnkoMuutoksia` | :1750 | Dirty check against `Val*` copies; toggles Save/Cancel buttons. [V] |

### 1.3 Data read and written
- **Competitor record `kilptietue Kilp`/`Kilp_0`/`EKilp`**: read with `GETREC`/`getpos`, edited in memory, saved with `Kilp.tallenna(d, comtarfl, kielto, pakota_lah, entkilp)`.
  - `tallennaKilpailija`: `Kilp.tallenna(DKilp,0,0,0,0)` (:1615). Lock held. Note: saved only if `DKilp==getpos(Kilp.id())` (:1614). [V]
  - `TallennaMuutokset`: `Kilp.tallenna(DKilp,0,-1,0,0)` (:2628, :2569, :2697), `Kilp_0.tallenna(DKilp_0,-1,0,0,0)` (:2648), `vkilp.tallenna(d,0,-1,0,0)` (:2618, vacant target). The second argument (comtarfl) and third (kielto) are swapped for `Kilp_0` compared with `Kilp` (`-1,0` vs `0,-1`), and the relay twin shows the same pattern (ViestiWin/UnitEmit.cpp:2209 vs :2211). [V] that args differ; intent [tuntematon]. Lock held. [V]
  - `TallennaVaihto`: `vkilp.tallenna` (:2775, :2799), `kilp.tallenna` (:2784), `Kilp_0.tallenna` (:2793). Lock held. [V]
  - `MerkitseOhitetuksi`: `Kilp_0.tallenna(DKilp_0,-1,0,0,0)` (:2741), lock held. [V]
  - `Paivita` (mode 99/rebadge): `Kilp.tallenna` at :619 (no lock) and :702 (with lock). [V]
  - `BtnPerKirjausClick`: `Kilp.tallenna` (:3001), no lock. `BtnOhitaClick`: `EKilp.tallenna` (:3483), no lock. `Laskeuudelleen1Click`: `kilp.tallenna` (:3449), no lock. [V]
- **Emit records `emittp`**: `Em.put(Tietue,flag)` (:630, :707, :2570, :2625, :2745, :2826, :3482, :3712), `putem(&Em,Tietue,0)` (:1583, :1593, :1608, :2692), `em.get(Tietue)`, `getem` (:868, :2081-2267), `lueem` through the core, `setEmitJarr` (:605, :704, :1589, :2573), `addbadge` (:2572), `addemitanal` (:2705). `emithead`/`emithead2` read for the cursor. [V]
- **Time queues (`aikajono`/`aikatp`)**: not touched. [V] (grep: none in UnitEmit.cpp)
- **Draw data**: none. **Files**: `emitfile` read directly under `emit_CriticalSection` (:3632-3645). Print devices through `openprfile`. [V]

### 1.4 Calls into the core
| Caller -> callee | File:line |
|---|---|
| `NewCardHandler` -> `pros_emit` (Hk/HkIV.cpp:1162) | UnitEmit.cpp:936 |
| `LaskeTulos` -> `laskeEm_VA` (Hk/HkEmit.cpp:6198) / `e_maaliaika` (Hk/HkEmit.cpp:3618) | :2381 / :2404, :2411 |
| `TallennaMuutokset` -> `tarkista` (Hk/HkEmit.cpp:2029) | :2662, :2668 |
| `TallennaMuutokset` -> `etsiesarja` (Hk/HkEmit.cpp:2253) when `autosarja` | :2658 |
| `TallennaMuutokset` -> `emit_rap` (Hk/HkEmit.cpp:4556) / `tulostaRastiVa` (Hk/HkEmit.cpp:4333) (auto print) | :2701 / :2703 |
| `TallennaMuutokset` -> `haeSeurVakantti`, `haeSeurVapaaBadge` (Hk/HkApu.cpp:247, :228) | :2602 / :2690 |
| `TallennaMuutokset` -> `addbadge`, `setEmitJarr` (Hk/HkEmit.cpp:1034, :1016) | :2572-2573 |
| `TallennaMuutokset` -> `addemitanal` (Hk/HkEmit.cpp:2750) | :2705 |
| `Laskeuudelleen1Click` -> `e_maaliaika`, `tarkista`, `kilp.set_tulos`, `kilp.tallenna` | :3403, :3404, :3447, :3449 |
| `Tulosta` -> `emit_rap`, `tark_rap`, `emitlista`, `tulostaRastiVa` | :3042-3051 |
| `Etsileimojavastaavarata1Click` -> `etsierata` (Hk/HkEmit.cpp:2238) | :3221 |
| `HaeTulostukseen` -> `bdg2kno`, `getem` | :851, :868 |
| `Lueratatiedotuudelleen1Click` -> `lueRatatiedotUudelleen` (Hk/HkEmit.cpp:6403) | :3188-3190 |

---

## 2. ViestiWin/UnitEmit.cpp (`TFormEmit`, 3091 lines), ViestiWin

### 2.1 Responsibility
Relay-program card form with the same user purpose as HK (view the card, link it to a team and leg, fix punches, calculate the finish time, print). The relay version is keyed on (team number `kilpno`, leg `osuus`) pairs. It carries card processing in the form: `ProsEmit` (:893) does the card to team/leg match, writes the record and triggers printing. It has no kuntosuunnistus modes and no person-database integration (`FormTKanta`/`SiirraHTTiedot` are absent from the relay file). [V] Instead it has the modes `esitark` (-2, <0, 0), `jalkisyotto` (late entry), `val_korkein`, `huomaatark` and `joustoviesti` (the last in Ajanotto only).

### 2.2 Main logic functions
| Function | File:line | Note |
|---|---|---|
| `NewCardHandler` | UnitEmit.cpp:947 | Same skeleton as HK, but it reads the record itself: `lueem(&em,emithead2)`, skips already-booked (`em.kirjaus != TMAALI0/KSEK`, :997-999), then calls local `ProsEmit()` (:1013). [V] |
| `ProsEmit` | :893 | `lueem`, stamps `Em.kirjaus` (:898-899), `HaeKilpailija` (:907) if not `jalkisyotto`, `KysyKilpailija` (:911) if no team/leg, then under `tall_CriticalSection`: `haeVapaaKoodi` (:919), `setEmitJarr` (:923), `ProsEmitMaali` (:924), `tark_badge` (:925), `tallenna` (:926). Then `putem` (:930), `laheta_em` (:931), `tark_partio` (:934, `#ifdef PARTIO`), auto `emit_rap` (:940). [V] |
| `HaeKilpailija` | :732 | `bdg2kno(Em.badge,0)` then leg search over `kilp.ostiet[os].badge[0]`; badge 200 = damaged card. [V] |
| `KysyKilpailija` | :767 | Opens `FormEmitMuutokset` modally for manual team/leg entry (:786-813). [V] |
| `ProsEmitMaali` | :817 | Computes the finish time for `rt->maalilaji==4` (`e_maaliaika` :829) and, when `!esitark`, pushes the card finish time into the time queue with `tall_rivi(ino,&itm,...)` (:852, :878) (package 10000000-11000000 = chip-time rows). Sets `Kilp.setMaali` (:888). [V] |
| `tallennaKilpailija` | :1211 | Saves class, penalty, check mark (`SetTark`; for `partio` all legs), `setMaali`, course, comment; `tallenna(&Kilp,DKilp,0,0,0,0)` (:1248) inside lock. Refuses if `DKilp != getpos(Kilp.kilpno)` (:1214). [V] |
| `EdtKilpnoKeyDown` / `lueKilp3` | :1286 / :1270 | Same as HK; `lueKilp3` copies check mark/finish/badge/new-flag of the leg. [V] |
| `LaskeTulos` | :2009 | `e_maaliaika(&Em,&kilp,Osuus)` (:2016), shows fields only. No `laskemtb`/`kaksibadge==2` branch. [V] |
| `TallennaMuutokset` | :2137 | Move card to another team/leg. `tallkvaihto` (:2207), `tallenna` (:2209, :2211, :2228), `tarkista` (:2223), `emit_rap` (:2231), `addemitanal` (:2235). The lock is taken at :2158 and released at :2230 or :2238 depending on `kilpVapaa`. [V] |
| `MerkitseOhitetuksi` | :2249 | `putem` (:2267, :2274), `tallkvaihto` (:2268), `tallenna` (:2269), `setEmitJarr(...,-1)` (:2275). **No lock here.** [V] |
| `TallennaVaihto(d,os)` | :2284 | Swap badges between legs. `tallenna` at :2309-2336. No lock taken in the function. [V] |
| `TallennaEmMuutokset` | :2349 | `putem(&Em,Tietue,true)` (:2363). [V] |
| `Mrluentaajatmallinajoista1Click` | :2950 | Bulk: per card sets `Em.time` from the leg finish time via `tarkista`/`haelukija`; `putem` (:2983). Relay-only (HK analogue is `Vaihdaluentaajat`). [V] |
| `Etsileimojavastaavarata1Click` | :2736 | Uses `etsiesarja` (HK uses `etsierata` then class search). [V] |
| `Tulosta` | :2538 | `emit_rap`, `tark_rap`, `emitlista`, `tulostaRastiVa(...,Osuus)`. [V] |
| `HylkRapClick` | :3072 | Opens `FormHylkRap->NaytaKilpailija(Kilp.kilpno,Osuus,Em.badge0)`. [V] |

### 2.3 Data read and written
- **Team record `kilptietue Kilp`/`Kilp_0`**: `getrec`, written through the free function `tallenna(&Kilp,d,comtarfl,kielto,pakota_os,entkilp)` (Juk/vdat.cpp:470) at :926, :935 (`TRUE` in the comtarfl slot), :1248, :2148, :2209-2228, :2269, :2309-2336, :2532 (`BtnPerKirjaus`). [V]
- **Emit records**: `putem` (:903, :930, :2267, :2274, :2363, :2983), `Em.put` (:2149), `lueem` (:898, :997, :1052, :1075, :2168, :2338, :2957), `setEmitJarr(kilpno,os,ep)` (:923, :2152, :2275), `addbadge` (:2151). [V]
- **Time queue**: `tall_rivi(ino,&itm,...)` (:852, :878) in `ProsEmitMaali`, with `aikajono[ino]->getAktTime` (:861-879). `seurkrivi`/`autokilp` select the next free queue row. Locking of `ajat_CriticalSection` inside `tall_rivi` is core; not analysed. [V] for the call, [I] for locking.
- **Communication**: `laheta_em(&Em,1,0)` (:931) sends the card to partner computers. [V]

### 2.4 Calls into the core
| Caller -> callee | File:line |
|---|---|
| `ProsEmit` -> `lueem` (Juk/VEmit.cpp:1402 / VIv.cpp:909), `haeVapaaKoodi` (VEmit.cpp:1074), `setEmitJarr` (:1440), `tark_badge` (:2659), `tallenna` (vdat.cpp:470), `putem` (:1452), `laheta_em` (VCom.cpp:792), `tark_partio` (:2264), `emit_rap` (:5196) | UnitEmit.cpp:898, 919, 923, 925, 926, 930, 931, 934, 940 |
| `ProsEmitMaali` -> `e_maaliaika` (VEmit.cpp:1298), `tall_rivi` (Juk/vajat.cpp:103) | :829, :852, :878 |
| `HaeKilpailija` -> `bdg2kno` (VEmit.cpp:1028), `EmitJarr` (:1427) | :735-760 |
| `TallennaMuutokset` -> `tallkvaihto` (VEmit.cpp:3999), `tarkista` (:2004), `addemitanal` (:3224) | :2207, :2223, :2235 |
| `Mrluentaajatmallinajoista1Click` -> `haelukija`, `tarkista` | :2961-2964 |

---

## 3. cbHk/UnitAjanotto.cpp (`TFormAjanotto`, 1961 lines), HkKisaWin

### 3.1 Responsibility
Manual and semi-automatic timekeeping form. It shows one or two time queues (`aikajono[IkkParam.Jono]`, grids `DGajat1` and `DGajat2`) and lets the user take a time with a key press (`lisaaAika`), insert, edit or delete rows, and attach a competitor number (`kno`), a split point (`piste`) and a badge to each time row. When a row is saved, the form propagates the time to the competitor record. It also handles start-time assignment for a whole class or heat from a time row (letter `S`/`R` in the number column), the Sirit RFID reader connection controls and import/export of the queue as text. [V]

### 3.2 Main logic functions
| Function | File:line | Note |
|---|---|---|
| `FormCreate` | :73 | If `aikajono[0]` is not open sets `ajanottofl=1` and calls `inittimer()` (:98). Builds point/queue lists; `IkkParam.Ennakoi=true` (:~121). [V] |
| `lisaaAika` | :558 | Key-press timestamp (`Nyt()`), `lisaa_aika(IkkParam.Jono,t,NappainPiste,...)` (:568). [V] |
| `BtnLuoUusiClick` | :578 | Create a row between neighbours (time = previous + `AikaLisays`), `lisaa_aika` (:606). [V] |
| `BtnTall2Click` | :616 | Save edit panel: `tall_rivi` (:652); then under `tall_CriticalSection` clears the old competitor's time (`tall_tulos`, :659) and sets the new (`set_tulos` :672, `tallenna` :661, :673). [V] |
| `BtnPoistaClick` | :688 | `poista_aika` (:697) after confirm. Does not touch the competitor record. [V] |
| `DGajat1KeyPress` | :1011 | The main grid editor (about 280 lines). Input modes by `IkkParam.SyottoTila` (1-6: number, point, time, anticipate, badge). `tall_rivi` (:1222, :1226), then under lock: reset the old competitor time if it still matches (tolerance `2*pyor[3]`, :1235), `set_tulos` + `tallenna(d,0,0,pakLah,0)` (:1250-1251), `lahetaeralahto` for ski mass-start heats (:1258). Asks "competitor already has a time" (:1127-1131) and confirmation (`VaadiVahvistus`). [V] |
| `SarjaLahto` | :888 | `S`: `FormSarjaVal` then `asetaSrjLahto(sarja,aika,false,true,false)` (:903) and a row with `kno=30001+sarja`, `piste=-1` (:905-906). `R`: `asetaEraLahto` (:916) with `kno=31000+era`. [V] |
| `uusintaTallennus` | :971 | Ctrl+U: re-send row `tall_rivi(...,pakota_lah=1)` (:980) and rewrite competitor time `set_tulos`/`tallenna(...,1,0)` (:985-986). [V] |
| `Aktivoi` | :994 | Ctrl+T/O: set row status (active/ignored) via `tall_rivi` (:1003). [V] |
| `DGajat1KeyDown` | :1299 | F11 = new row, Ctrl+U/T/O, timing key. [V] |
| `kasitteleSpecialKey` | :716 | Timing-key detection. `Key=0` is set before the `Key==L'E'` test, so the Ctrl+E branch is unreachable (:719-725). [V] |
| `Vaihdaer1Click`, `Seuraavaer1Click`, `Edellinener1Click` | :1363, :1882, :1890 | Change active heat `era_akt`. [V] |
| `Lueaikojatekstitiedostosta1Click`, `Kirjoitaajattekstitiedostoon1Click` | :1394, :1380 | `AjatTiedostosta` (Hk/HkAjat.cpp:474), `AjatTiedostoon` (:413). [V] |
| `Vaihdanytetynaikajonontiedosto1Click` | :1867 | `suljeajat` + `initajat(jono,1)` (:1875-1876): closes and reopens a queue file. [V] |
| `KorjaakelloClick` | :1499 | Opens `FormAikakorjaus` (clock correction). [V] |
| `SynkronoiSirit1Click`, `KynnistSirityhteysuudelleen1Click`, `Sirityhteydenpollaus1Click` | :1351, :1600, :1626 | Sirit reader controls: `SiritSync(true)`, `reconnectSirit[]`, `SiritPoll[]`. [V] |
| `BtnHakuClick` | :1448 | Row search by competitor number. [V] |

### 3.3 Data read and written
- **Time queue `aikajono[]`/`aikatp`**: `getAktTime` (many), writes through core `lisaa_aika` (:568, :606), `tall_rivi` (:255, :652, :906, :919, :980, :1003, :1222, :1226), `poista_aika` (:697), `initajat`/`suljeajat` (:1875-1876). [V]
- **Competitor record**: `getpos(kno)`, `kilp.GETREC`, `kilp.set_tulos(va_piste(piste), time)` and `kilp.tall_tulos(va_piste,0)`, then `kilp.tallenna(...)` at :263 (`naytaRivi`!), :661, :673, :986, :1237, :1251, :1269 (that last one is inside a commented block). All hold `tall_CriticalSection` except where noted. [V]
  - `naytaRivi` (:210-264) is a display function that also writes: at :255 it calls `tall_rivi(jono,&upt,&pt,...)` and at :262-263 `set_tulos` + `tallenna` inside the lock. This is a write path in a view function. Trigger condition not traced. [I]
- **Start data**: `asetaSrjLahto`/`asetaEraLahto` (Hk/HkApu.cpp:411/447, they take `tall_CriticalSection` internally at :421, :454) and `eralahto[]`. [V]
- **Files**: text import/export via core; queue file switching via `aikafname`. [V]

### 3.4 Calls into the core
| Caller -> callee | File:line |
|---|---|
| `lisaaAika` / `BtnLuoUusiClick` -> `lisaa_aika` (Hk/HkAjat.cpp:298) | UnitAjanotto.cpp:568 / :606 |
| `BtnTall2Click`, `DGajat1KeyPress`, `SarjaLahto`, `uusintaTallennus`, `Aktivoi` -> `tall_rivi` (Hk/HkAjat.cpp:114, 8 args incl. `pakota_lah`, `InCriticalSection`) | :652, :1222-1226, :906/:919, :980, :1003 |
| `BtnPoistaClick` -> `poista_aika` (HkAjat.cpp:318) | :697 |
| `SarjaLahto` -> `asetaSrjLahto` / `asetaEraLahto` | :903 / :916 |
| `DGajat1KeyPress` -> `lahetaeralahto` (Hk/HkCom32.cpp:558) | :1258 |
| `Kirjoita/Lue...` -> `AjatTiedostoon`/`AjatTiedostosta` | :1389 / :1405 |
| `Vaihdanytetynaikajonontiedosto1Click` -> `suljeajat`, `initajat` | :1875-1876 |
| `BtnTall2Click` etc. -> `va_piste` (Hk/HkApu.cpp:136) | :659, :672 |

---

## 4. ViestiWin/UnitAjanotto.cpp (`TFormAjanotto`, 1754 lines), ViestiWin

### 4.1 Responsibility
Same purpose as HK (time rows, competitor number, point), but each row also carries a **leg** (`upt.osuus`, grid column `cOs`). The leg is derived from the card badge or from the team's next unfinished leg (`haeseurosuus`, `haeseurpiste`, `haeKnoOs`). Saving writes into the team record with `setMaali(osuus,piste,time)`. It has no Sirit controls, no heat stepping and no start-queue (`kuntosuunn`) options; the input modes are 1-3 (`RGSyotto`) against HK's 1-6. [V]

### 4.2 Main logic functions
| Function | File:line | Note |
|---|---|---|
| `lisaaAika`, `BtnLuoUusiClick` | :560, :579 | `lisaa_aika(Jono,...)` (:569, :607). Point argument is `0`, not `IkkParam.NappainPiste` as in HK. [V] |
| `Button2Click` (save edit panel) | :617 | Reads leg from `EdOsuus`; asks "already has time" via 3-way `select3` (:639-646, option 2 sets `upt.piste = valuku+1` = "store number but do not print"); `tall_rivi` (:657). **No competitor `tallenna` here**, unlike HK `BtnTall2Click`. [V] |
| `Button3Click` | :670 | Delete: `poista_aika` (:681). [V] |
| `SarjaLahto` | :841 | `S`: user chooses class start time or individual first-leg start times. `asetaSrjLahto(sarja,t,false)` (:865) or `Sarjat[].lahto=...; kirjEventXml(sarjaxmlfname,...); laheta_yl(...)` (:867-870); `R`: `asetaEraLahto` (:883). [V] |
| `haeKnoOs` (static) | ~:953-992 | Badge to (team, leg), with `val_korkein`, `kaksibadge`, `joustoviesti`, `EmitJarr`. [V] |
| `DGajat1KeyPress` | :995 | Main grid editor. Uses member `Pakota` (Ctrl+Enter, set in `DGajat1KeyDown` :1291-1293). `tall_rivi(Jono,&upt,&pt,&uptr,0,pakLah,false)` (:1221) **passes `pakLah` in the `kielto` slot** (see Noticed #1). Under lock: resets the old time (`setMaali(...TMAALI0)`, `Tallenna` :1231-1232) and sets the new (`setMaali`, `Tallenna(d,0,0,pakLah,0)` :1243-1244). [V] |
| `uusintaTallennus` | :934 | `setMaali` + `tallenna(&kilp,d,0,0,1,0)` (:947-948). Does not re-send the queue row (HK does, :980). [V] |
| `Aktivoi` | :698 | `tall_rivi` (:707). [V] |
| `DGajat1KeyDown` | :1288 | `Pakota`, timing key, F11, Ctrl+U/T/O. [V] |
| `RGSyottoClick`, `CBPisteValClick`, `Kaavakkeenoletuspiste1Click`, `Nytosuusvalinta1Click` | :903, :1743, :1347, :1731 | Mode/point/leg defaults (`va_mpiste[Jono]`). [V] |
| `Maalikello1Click`, `Vaihdanytetynjonontiedosto1Click` | :1705, :1717 | Finish clock and queue file switch (`initajat(Jono,1)` :1725). [V] |

### 4.3 Data read and written
- **Time queue**: `lisaa_aika` (:569, :607), `tall_rivi` (:332, :657, :707, :874, :886, :1221), `poista_aika` (:681), `initajat` (:1725). [V]
- **Team record**: `kilp.getrec(d)`, `setMaali(os,piste,time)` and write through `tallenna(&kilp,d,...)` (free function: :320, :948) or `kilp.Tallenna(...)` (method wrapper, :1232, :1244, :1262 in a comment). Locks held. [V] `naytaRivi`-adjacent write at :311-332 (inside the lock :311-327, `setMaali`+`tallenna` :319-320, then `tall_rivi` :332): same pattern as HK :255-264, with `tall_rivi` after the lock. [V]
- **Start data**: `Sarjat[].lahto`, XML event file `sarjaxmlfname` via `kirjEventXml`, broadcast `laheta_yl` (:869-870). [V]

### 4.4 Calls into the core
| Caller -> callee | File:line |
|---|---|
| `lisaa_aika` (Juk/vajat.cpp:262), `tall_rivi` (vajat.cpp:103, **7 args**), `poista_aika` (vajat.cpp:278) | UnitAjanotto.cpp:569/607, :332/657/707/874/886/1221, :681 |
| `asetaSrjLahto` (Juk/vApu.cpp:304, 3 args), `asetaEraLahto` (vApu.cpp:329) | :865, :883 |
| `kirjEventXml`, `laheta_yl` | :869, :870 |
| `haeseurosuus` (Juk/VIv.cpp:1709), `haeseurpiste` (VIv.cpp:1719) | :1107, :1117-ish |
| `bdg2kno` (VEmit.cpp:1028), `EmitJarr` | haeKnoOs ~:953-992 |
| `tallenna` (Juk/vdat.cpp:470) / `kilptietue::Tallenna` (Juk/vkilp.cpp:1695) | :320, :948 / :1232, :1244 |
| `AjatTiedostoon` (vajat.cpp:404) | :1400 |

---

## 5. cbHk/UnitArvonta.cpp (`TFormArvonta`, 1270 lines), HkKisaWin

### 5.1 Responsibility
Start-draw and numbering form for HK. It does four things:
1. **Draw** (`arvo(Toiminto)`): random or ranked draw of start order, per class, per day, with style options (`CB2pv`, `RGRajaustapa`, `RGPoissa`).
2. **Start-time assignment** by class parameters (`enslahto`, `lahtovali`, `pari`).
3. **Bib numbering** by order, alphabet, save order, rank, copy of base numbers and number offset.
4. **Splitting a class into several classes** (`BtnJaa`, `ToteutaJako`): `RPAYT` methods.

The draw algorithm itself is in `cbHk/HkArv2.cpp:1129` (`arvo`), `:696` (`arvonumerot`), `:740` (`arvovaihe`), outside this batch. [V]

### 5.2 Main logic functions
| Function | File:line | Note |
|---|---|---|
| `BtnToteutaClick` | :338 | Toiminto 0/1/3 call core `arvo(Toiminto)` (:398). 2/4/5 are done here: for each selected class iterate competitors with `SeurPos` and set start times `tall_lajat_pv(enslahto + (n/pari)*lahtovali*SEK)`, saving each with `kilp.tallenna(d,0,0,0,NULL)` (:442) inside `tall_CriticalSection` (:419-445). Restrictions: only later stages in results-service mode (`ToimintaTila==2`, :369-371), classes must participate in the chosen stage. [V] |
| `BtnNumeroiClick` | :515 | Numbering variants 0-7 over `SeurPos(&kilp,jarj[Toiminto],srj,d,arv_pv)`, bib set by `setId`/`setIdAll` (with `SprLisa` offset and `set_tark('V')` for later stages), `kilp.tallenna(d,0,0,0,NULL)` at :597, :613, :639, :656, :689, :706, :719, :736 under lock. Toiminto 4 = random bib draw `arvonumerot(srj,false,NumKaikkiPaivat,sarjaindex)` (:750). Ends with `luo_posarr()` (:761). [V] |
| `SeurPos` | :213 | Iterator over competitors (id index `getpos/nextpos` under `idindex_CriticalSection` :222-233; name index; save order; start-order index `ljarjindex`; rank index `pstindex`). [V] |
| `TarkVakantit`, `tarkNumerotila` | :88, :130 | Check vacant block structure and numbering state. [V] |
| `BtnMuodostettavat`, `ValmisteleJako`, `JaaSeuratKoossa`, `ToteutaJako`, `BtnJaa` | :785, :806, :833, :901, :1019 | Class splitting. `ValmisteleJako` computes target counts (H-085 sits here). `ToteutaJako` sets class by `setSarjaAll`/`setSarja` (and optionally bib + start time with `CBoxLajat`) and `tallenna` (:944, :1011) under lock; method `A`/`T` call `arvovaihe` (:909). `JaaSeuratKoossa` keeps clubs together (`tallenna` :885). End: `reindex=1` (:1016). [V] |

### 5.3 Data read and written
- **Competitor record**: bib (`setId`, `setIdAll`), class (`setSarja`, `setSarjaAll`), start time (`tall_lajat_pv`), check mark (`set_tark('V')`); all saved with `kilp.tallenna(d,0,0,0,NULL)` (lines in 5.2). [V]
- **Indexes**: `sarjaindex`, `ljarjindex`, `pstindex`, `nameindex` rebuilt with `teeindeksi` (:265, :309, :574, :845, :907) and `luo_posarr()` (:761). `reindex` set. [V]
- **Draw state**: globals `arv_pv`, `RYlimmasta`, `RPuuttuu`, `RPoissa`, `RPistekentta`, `SprLisa`, `OhitaVakantit`, `nilm`/`nilm_pv`/`nvak` (set from the form and read by core `arvo`). [V]
- **Files**: none directly. [V]

### 5.4 Calls into the core
| Caller -> callee | File:line |
|---|---|
| `BtnToteutaClick` -> `arvo` (cbHk/HkArv2.cpp:1129) | UnitArvonta.cpp:398 |
| `BtnNumeroiClick` -> `arvonumerot` (HkArv2.cpp:696) | :750 |
| `ToteutaJako` -> `arvovaihe` (HkArv2.cpp:740) | :909 |
| various -> `kilptietue::tallenna` (Hk/HkDat.cpp:897) | :442, :597-736, :885, :944, :1011 |
| `SeurPos` -> `getpos`, `nextpos`, `searchix`/`nextix`, `teeindeksi`, `keyfromwname` | :222-233, :249-309 |
| `BtnNumeroiClick` -> `luo_posarr`, `haeseurvapaa` | :761, :608 |
| `BtnNumeroiClick`/`FormShow` -> `tarkNumerotila` (local) | :463, :554 |

---

## 6. ViestiWin/UnitArvonta.cpp (`TFormArvonta`, 611 lines), ViestiWin

### 6.1 Responsibility
Much smaller than HK. It renumbers teams (`kilpno`) per class and assigns leg start times (`ostiet[os].ylahto`). It has **no start-order draw and no class split** in the unit: `arvo(int)` is declared at :33 but there is no definition or call anywhere in `ViestiWin/`, `Juk/` or `Tp/` (grep `\barvo(` gave only the declaration). The only random function is `arvonumerot(UINT16*,int)` (:328), a shuffle of the numbers `0..lkm-1` (Mersenne twister `MTRand_int32`, :330-340) used for random team numbers. [V]

### 6.2 Main logic functions
| Function | File:line | Note |
|---|---|---|
| `BtnNumeroiClick` | :371 | Toiminto 0 = alphabetic, 1 = save order, 2 = random (`arvonumerot` :432), 3 = rank-sum order (`teeRankiJarj`, :154), 4 = shift all numbers by `muutos`. `varmista_vapaa(kno,d)` (:351) checks range and duplicate numbers (not in `ToimintaTila==1`). Writes `tallenna(&kilp,d,0,0,0,NULL)` (:417, :452, :459) under lock. Sets `kirjheti=0` during the run and restores it (:379-380, :478). [V] |
| `BtnLahtoajatClick` | :523 | Per team and leg: toiminto 0 sets `ostiet[os].ylahto = Sarjat[].ylahto[os] + SEK*lahtovali*(kilpno-sarjaalku)` and `lahtolaji=2`; toiminto 1 shifts all by `EdtAikasiirto`. `kilp.Tallenna(d,0,0,0,0)` (:575) under lock (:544-576). [V] |
| `SeurPos` | :181 | Iterators: id range, name index, save order, rank order (`ix_jrj`), descending id. [V] |
| `tarkNumeroVapaus` | :71 | Check that a shift does not collide. [V] |
| `BtnFISpisteetClick` | :595 | Opens `FormFIS` (FIS points). [V] |

### 6.3 Data read and written
Team record: `kilpno`, `ostiet[os].ylahto`, `lahtolaji`, written with `tallenna`/`Tallenna` only. Index: `nameindex` (SeurPos). Globals `kirjheti` (write-flag mask, `Juk/VGlobals.cpp:310`, tested in `Juk/vajat.cpp:320`) saved and zeroed around bulk edits. No file I/O. [V]

### 6.4 Calls into the core
| Caller -> callee | File:line |
|---|---|
| `BtnNumeroiClick` -> `tallenna` (Juk/vdat.cpp:470), `arvonumerot` (local) | UnitArvonta.cpp:417, :432, :452, :459 |
| `BtnLahtoajatClick` -> `kilptietue::Tallenna` (Juk/vkilp.cpp:1695) | :575 |
| `SeurPos` -> `getpos`, `searchix`/`nextix`, local `teeRankiJarj` | :181-262 |

---

## 7. cbHk/UnitKilpailijatiedot.cpp (`TFormKilpailijatiedot`, 1900 lines), HkKisaWin

### 7.1 Responsibility
Browse/edit form for one HK competitor record. A read-only "search and browse" mode and an explicit edit mode (`asetaMuokkaustila`, :1257). Field edits go into the in-memory copy `Kilp`; `Kilp1` is the original; "Save" writes if the record has not changed elsewhere. It has a per-day grid `PvGrid` for the stage-specific fields (bib, class, badge, loan flag, start, result, check mark, penalty, course, heat, target time, points). Add (`Liskilpailija1Click`), delete (`Poistakilpailija1Click`), browse by number, name or start order, club-list integration (`FormSeurat`) and person register lookups (badge, licence, base number). [V]

### 7.2 Main logic functions
| Function | File:line | Note |
|---|---|---|
| `EdtKilpnoChanged` | :96 | In browse mode loads record `getpos(KilpNo,true)` then `GETREC`. In edit mode asks to change the number (`setId`, with `ToimintaTila==1` using base number -1); in add mode picks `haeSeurVapaaKno` (:116, :119). [V] |
| `naytaKilpailija(d)` | :142 | Load record `d`; sets `Kilp1=Kilp`. [V] |
| `tallennaTiedot` | :588 | **The main write function.** Add: `Kilp.addtall(&dKilp,0)` (:601) under lock. Change: aborts if `Kilp1==Kilp` (no change) or if `Kilp2.GETREC(dKilp)` differs from `Kilp1` (changed elsewhere, :613-616) or the number is taken (:620); then `Kilp.tallenna(dKilp,0,0,0,&Kilp1)` (:627) under lock. The "changed elsewhere" comparison and `Kilp2.GETREC` happen **before** the lock (:613 vs :626). [V] |
| `paivitaMuutos(col,row)` | :778 | Applies one `PvGrid` cell to `Kilp.pv[ipv]` by column type: bib, class, badge (offers to copy to later days), loan, start time, finish, result, check mark, penalties (`tall_sakko`), course, heat, target time, points. [V] |
| `SeurBtnClick`, `EdBtnClick` | :1106, :1157 | Next/previous record by start order (`JarjSeur`), id (`nextpos`), or name index; under `tall_CriticalSection`. [V] |
| `FormClose` | :1366 | Prompt to save; calls `tallennaTiedot` and may cancel the close. [V] |
| `Poistakilpailija1Click` | :1746 | `deltall(dKilp,0)` (Hk/HkDat.cpp:741) after confirm (:1749). [V] |
| `SiirraSeuraTiedot`, `Emitkoodihaku1Click`, `Lisenssinumerohaku1Click`, `Perusnumerohaku1Click` | :1588, :1669, :1686, :1711 | Look-ups into the club list and person register (`FormSeurat`, `naytaKilpailija(getpos(kno))` :1679). [V] |
| `BtnOnlineClick`, `BtnEmitClick` | :1423, :1436 | Open `FormKilpailijaOnline`/`FormKilpailijaEmit` for `dKilp`. [V] |
| `haeTiedot` | :482-585 | Inside a `/* ... */` block (:481, :585); dead code. [V] |

### 7.3 Data read and written
Competitor record (`Kilp`, `Kilp1`, `Kilp2`; `dKilp` = record position). Writes: `addtall` (:601), `tallenna(dKilp,0,0,0,&Kilp1)` (:627), `deltall` (:1749). All reads and writes go through core record functions; no direct file access; no emit/time-queue/draw access. The `entkilp` argument (`&Kilp1`) lets the core detect which fields changed (index updates, send to MySQL/network). [I] Details of core use of `entkilp`: not traced. [V] for the call, [I] for purpose.

### 7.4 Calls into the core
| Caller -> callee | File:line |
|---|---|
| `tallennaTiedot` -> `kilptietue::addtall` (Hk/HkDat.cpp:674) | UnitKilpailijatiedot.cpp:601 |
| `tallennaTiedot` -> `kilptietue::tallenna` (HkDat.cpp:897) | :627 |
| `Poistakilpailija1Click` -> `deltall` (HkDat.cpp:741) | :1749 |
| `EdtKilpnoChanged` -> `haeSeurVapaaKno(ipv,alku)` (Hk/HkApu.cpp:198), `getpos`, `setId`/`setIdAll` | :104, :116, :119 |
| `SeurBtnClick`/`EdBtnClick` -> `JarjSeur`, `nextpos`, `searchix`/`nextix`/`previx`, `keyaak` | :1113-1153, :1164-1212 |

---

## 8. ViestiWin/UnitJoukkuetiedot.cpp (`TFormJoukkuetiedot`, 1639 lines), ViestiWin

Counterpart of section 7 (see Summary for why `OsuusTiedot.cpp` is not).

### 8.1 Responsibility
Browse/edit form for one team record: team name/club/class/number/points, plus a per-leg grid `OsGrid` (leg runner name, club, licence, badge, loan flag, check mark, start time and type, finish time, shooting, penalty, course) and a split-time grid `TlsGrid` (split times or leg times, selected with `RGVaTieto`). Has edit mode, add/delete, club and person look-ups, "late-entry add mode" (`oistuvienlisystentila1`: after save of a team it opens the next leg row) and `BtnJjVaihto` (reorder/rewrite leg runner names, licences, badges and courses within the team through `FormJjVaihdot`). [V]

### 8.2 Main logic functions
| Function | File:line | Note |
|---|---|---|
| `EdtKilpnoChanged` | :69 | Browse mode: `getpos` + `getrec`. Edit mode: 3-way prompt before changing the team number, then `Kilp.kilpno = haeSeurVapaaKno(KilpNo)` (:113). [V] |
| `tallennaTiedot` | :476 | Add: `addtall(&Kilp,&dKilp,0)` (:488), then opens next-leg entry if `oistuvienlisystentila1`. Change: checks (no change, number free, `ToimintaTila==2` needs a number), **then lock (:518), compare `Kilp2.getrec(dKilp)==Kilp1` inside the lock (:519-521), `Kilp.Tallenna(dKilp,0,0,0,&Kilp1)` (:527), unlock (:528)**. [V] |
| `paivitaMuutos(col,row)` | :634 | Applies an `OsGrid` cell to `Kilp.ostiet[ipv]`: names (`setNimi`), club, licence, badge, loan, check mark, start (`ylahto`, `lahtolaji`), finish (`setMaali(ipv,0,...)`; an empty cell now deletes the time, :730-741), shooting (`setAmpsakot`), penalty, course. [V] |
| `paivitaVaMuutos` | :762 | Edits split/leg times in `TlsGrid` through `setMaali(os,va,...)`. [V] |
| `BtnJjVaihtoClick` | :1402 | Rewrites leg runner data of the team in memory (cases 101-104); written only when the user later saves. [V] |
| `SeurBtnClick`, `EdBtnClick` | :863, :912 | Navigation under `tall_CriticalSection`. [V] |
| `Poistakilpailija1Click` | :1365 | `deltall(dKilp,0)` (:1368). [V] |

### 8.3 Data read and written
Team record `Kilp`, `Kilp1`, `Kilp2` (`getrec`), `dKilp`. Writes: `addtall(&Kilp,&dKilp,0)` (Juk/vdat.cpp:858), `Kilp.Tallenna(dKilp,0,0,0,&Kilp1)` (Juk/vkilp.cpp:1695 -> vdat.cpp:470), `deltall` (vdat.cpp:907). No emit, time queue, draw or file access. [V]

### 8.4 Calls into the core
| Caller -> callee | File:line |
|---|---|
| `addtall` | UnitJoukkuetiedot.cpp:488 |
| `Tallenna` (wrapper for `tallenna`) | :527 |
| `deltall` | :1368 |
| `haeSeurVapaaKno(alku)` (Juk/vApu.cpp:148) | :113 |
| `setNimi`, `SetTark`, `setMaali`, `setAmpsakot`, `setHajonta` (record setters, Juk/vkilp.cpp) | :634-760 |

### 8.5 ViestiWin/OsuusTiedot.cpp (`TFormOsuus`, 476 lines): reference only
Read-only per-leg result viewer; includes an `EmitGrid` for card data when `emitfl`. Reads with `GETREC` and the navigation helpers listed in the Summary. No write call. [V]

---

## 9. HK vs relay comparison (paired units)

### 9.1 UnitEmit
Shared (copy-paste ancestry [I], evidenced by identical identifiers and structure):
- The same handler set: `NewCardHandler`, `HaeTietue`, `OtaTietue`, `Paivita`, `TallennaMuutokset`, `SiirraKilpailijalle`, `MerkitseOhitetuksi`, `TallennaVaihto`, `TallennaEmMuutokset`, `LaskeTulos`, `Tulosta`, `OnkoMuutoksia`, `BtnLoppuunClick`.
- The same "old record `Kilp_0`/`DKilp_0` versus new record" scheme.
- The same `DisplTimerThread` (HK :834, relay :687), the same `emitLuettu` (HK :966, relay :1024), and the same `_beginthread` calls.
- The same `tallenna(...,-1,...)` argument pattern for the old record (HK :2648, relay :2209).

Diverged:
1. **Card processing location.** HK calls core `pros_emit` (UnitEmit.cpp:936; core at Hk/HkIV.cpp:1162, which also contains console UI code under `#ifdef _CONSOLE`). Relay does it in the form (`ProsEmit`, UnitEmit.cpp:893). Therefore HK GUI and HK console share one card-processing function; the relay GUI and relay console do not [I], since a relay console equivalent was not looked for.
2. **Key**: HK `getEmitJarr(kilpno,&osuus)`, `bdg2kno(badge)`, `badge[0..1]` meaning main/second card; relay `EmitJarr(kilpno,os)`, `bdg2kno(badge,&toinen)`, `ostiet[os].badge[0..1]`.
3. **Modes**: HK has `esiluenta`, `kuntoLahto`, `kuntoMaali`, `kuntosuunnmoodi` 1/2/3/98/99, person register (`FormTKanta`), club list (`FormSeurat`), vacant handling with `vakanttisarja`. Relay has none of these but adds `jalkisyotto`, `huomaatark`, `val_korkein`, `esitark`, `FormHylkRap(kilpno,osuus,badge)`, `Sail99varoitus`.
4. **Bulk operations**: HK `Laskeuudelleen1Click`, `Vaihdaluentaajat`; relay `Mrluentaajatmallinajoista1Click` only.
5. **Time-queue side effect**: relay `ProsEmitMaali` puts the card's finish time into `aikajono` (:852, :878); HK `UnitEmit.cpp` has no `aikajono` access. [V]
6. **Locking**: relay `ProsEmit` and `TallennaMuutokset` hold `tall_CriticalSection` across `tark_badge`/`tarkista`/print calls (ProsEmit :917-928; TallennaMuutokset :2158-2230). HK `TallennaMuutokset` releases the lock right after the first writes (:2648-2649) and calls `tarkista`, `emit_rap` and `tallenna` outside or in separate short locks (:2662-2703). [V] Consequences for H-092 (emit to print lock order): not analysed here.
7. **Fix apparently applied to one side** [I]: HK `Laskeuudelleen1Click` (:3449) and `BtnPerKirjausClick` (:3001) save without the lock, while the relay saves in bulk functions inside the lock. Not enough evidence which side is newer.

### 9.2 UnitAjanotto
Shared: identical function set for rows, `lisaaAika`, `BtnLuoUusiClick`, `SarjaLahto`, `uusintaTallennus`, `Aktivoi`, `naytaRivi`, `tallVertRivi`/`vertRiviMuutt`, `DGajat1KeyPress` skeleton, the 30001+class / 31000+heat pseudo-numbers for start rows (HK :905/:918, relay :872/:885), and the same `Button*`/panel structure. [V]

Diverged:
1. **Leg dimension** in relay (`upt.osuus`, `cOs`, `EdOsuus`, `CBOsVal`, `haeKnoOs`, `haeseurosuus`). HK has none. [V]
2. **Input modes**: HK 0-6 (number, point, time, anticipate, badge); relay 0-3. [V]
3. **Queue selector**: HK `IkkParam.Jono`; relay `Jono` member plus `IkkParam.Jono`. [V]
4. **Save-panel (`BtnTall2Click` vs `Button2Click`)**: HK writes the competitor record after `tall_rivi` (:655-675); relay does not (only queue, :657) [V]. Whether the relay result is updated later from the queue by core code: [tuntematon].
5. **Forced resend**: HK `tall_rivi(..., 0,0,pakLah,false)` (:1226). Relay `tall_rivi(..., 0,pakLah,false)` (:1221) puts `pakLah` into `kielto`. See Noticed #1. [V] on arguments.
6. **Sirit controls, heat stepping (`Vaihdaer1Click` etc.), text-import and `VaadiVahvistus` menu items** exist only in HK (section 3). The relay has `Vaaditallennuksillevahvistus1`, `Vaadivahvistus2` as local menu checks (:1333-1345) and reads them in `DGajat1KeyPress` (:1015-1018). [V]
7. **Same defect in both**: `BtnTall2Click` (HK :641-649) and `Button2Click` (relay :640-649) assign `upt.piste` with `if (cp=='A') ...; if (cp=='M') ... else upt.piste=_wtoi(...)`, so a typed `A` (and in HK also `L`) is overwritten by the `else` branch. [V] (read; the effect depends on `_wtoi` of the text; not run.) Shared bug, not a one-sided fix.
8. **Date handling**: both call `date_time_t(EdDate->Text)`; H-080 applies to both. [V]
9. **`uusintaTallennus`**: HK re-sends the queue row (`tall_rivi(...,pakota_lah=1)`, :980) and writes the competitor with `tallenna(d,0,0,1,0)`; relay only does the second (`tallenna(...,0,0,1,0)` with `pakota_os=1`, :948). [V] Looks like HK-only extension [I].

### 9.3 UnitArvonta
Little shared code. The shared names are `SeurPos`, `SarjaValittu`, `BtnNumeroiClick`, `NaytaSarjavalinnat`, `Button1Click` and the `LBSarjat`/`LBSarjaVal` list handling. [V] HK is the large draw/numbering/split tool (1270 lines, `arvo`/`arvovaihe`/`arvonumerot` in `cbHk/HkArv2.cpp`); relay is a numbering plus leg-start-time tool (611 lines) with a local random number shuffle. HK has day-aware numbering (`bibvaihtelee`, `SprLisa`, `NumKaikkiPaivat`) and index-based ordering; relay has single-number-per-team logic with `varmista_vapaa` and `tarkNumeroVapaus` collision checks that HK lacks (HK `tarkNumerotila` checks the numbering state instead). [V]
- H-069 covers relay `NumKaikki` (declared :373, read :418, :464, never assigned). In HK the equivalent variable is assigned (`NumKaikki = RGRajaus->ItemIndex == 0`, :519). [V] This is the one-sided difference: HK initialises it, relay does not [I] (relay copy probably predates the `RGRajaus` radio group).
- Relay `BtnLahtoajatClick` has a lock leak, see Noticed #2.

### 9.4 Competitor/team data form
Shared structure: `EdtKilpnoChanged`, `naytaTiedot`, `tallennaTiedot`, `paivitaMuutos`, `asetaMuokkaustila`, `Kilp`/`Kilp1`/`dKilp`/`Sarja`/`nimihaku`/`sallimuokkaus`/`Lisays`/`sallimuokkausvalinta`, `SeurBtnClick`/`EdBtnClick`, add/delete menu items, `FormClose` save prompt. [V]

Diverged:
1. **Lock scope in `tallennaTiedot`.** Relay takes `tall_CriticalSection` before re-reading `Kilp2` and compares inside the lock (:518-528). HK reads `Kilp2` and compares at :613-616 and takes the lock only for the write (:626-628), leaving a window between the "changed elsewhere" check and the write. [V] That window is the H-017 mechanism for this form (HK cites :613-626). Looks like the relay side received a fix HK did not [I]; the commit history was not checked.
2. **Grid**: HK `PvGrid` per day (`pv[]`), relay `OsGrid` per leg (`ostiet[]`) plus `TlsGrid` for split and leg times (`paivitaVaMuutos`). Relay has `BtnJjVaihto` and late-entry mode; HK has sub-forms `FormKilpailijaOnline`/`FormKilpailijaEmit` and base-number versus day-number handling (`ToimintaTila==1` uses number -1). [V]
3. **Empty-cell finish time**: relay deletes the finish time on an empty cell (:730-741, comment `tyhja solu poistaa ajan`); HK `paivitaMuutos` has no finish-time column of this kind (it has `TulosCol` handling in other cases; the 'result' column was not examined in detail). [I]
4. **Add/delete**: HK `Kilp.addtall(&dKilp,0)` under lock (:600-602); relay `addtall(&Kilp,&dKilp,0)` without form lock (:488; core `addtall` locks, vdat.cpp:867-904). [V]
5. **HK dead code**: `haeTiedot` (:481-585) inside a comment; no such block in the relay file. [V]

---

## 10. Noticed, not analysed (for the defect-hunting agent)

| # | Where | Observation | Label |
|---|---|---|---|
| 1 | ViestiWin/UnitAjanotto.cpp:1221 vs Juk/vajat.cpp:103-104, 247 | `tall_rivi(Jono,&upt,&pt,&uptr,0,pakLah,false)`: parameter order is `(ino,utm,vtm,uptr,kysy,kielto,pakota_lah)`, so `pakLah` goes to `kielto` and `pakota_lah` is `false`. HK passes it in the `pakota_lah` position (cbHk/UnitAjanotto.cpp:1226). Forced resend by Ctrl+Enter probably does not work in the relay form, and `kielto` (connection number) gets the value 1. | [V] args, [I] effect |
| 2 | ViestiWin/UnitArvonta.cpp:544-549 | In `BtnLahtoajatClick` `EnterCriticalSection` is followed by `continue` when `kilp.kilpstatus` or `!SarjaValinnat[...]` (:547-549) without `LeaveCriticalSection`; the lock count grows for the GUI thread (recursive, so no immediate hang; other threads blocked until the thread exits or unlocks). Also `LbAjatValmis` shows before/after state mismatch. | [V] |
| 3 | ViestiWin/UnitEmit.cpp:2284-2304 | `TallennaVaihto`: `int kno` is never assigned before `vd = getpos(kno)` (:2312); in the `DKilp_0==d` branch `kilp.ostiet[os]...` is read before `kilp` is loaded (:2290-2293, `kilp` is a default-constructed record). | [V] |
| 4 | cbHk/UnitAjanotto.cpp:719-724 | `Key = 0;` precedes `if (Key == L'E' && ...)` so the Ctrl+E branch is dead. | [V] |
| 5 | cbHk/UnitAjanotto.cpp:641-649, ViestiWin/UnitAjanotto.cpp:640-649 | `if (cp=='A') ...; if (cp=='L') ...; if (cp=='M') piste=0; else piste=_wtoi(text)`: `A`/`L` assignments are overwritten by the `else` unless `_wtoi` happens to match. (In relay only `A` and `M` exist.) | [V] |
| 6 | cbHk/UnitAjanotto.cpp:255-264 and ViestiWin/UnitAjanotto.cpp:311-332 | `naytaRivi` (a display function) writes: queue row and competitor `set_tulos`/`setMaali` + `tallenna`. Trigger condition not traced. | [V] write, trigger [tuntematon] |
| 7 | cbHk/UnitKilpailijatiedot.cpp:1341-1364 | `RGSelaus->ItemIndex == 2;` is a comparison with no effect (intended assignment). | [V] |
| 8 | cbHk/UnitKilpailijatiedot.cpp:1140-1148 (`SeurBtnClick`, case 2) | When `nimihaku` is already true, `keyn` is passed to `nextix` uninitialised in this call (the buffer is a local `char keyn[MAXINDL+1]` declared inside the case). Not traced whether `nextix` ignores the incoming value. | [I] |
| 9 | ViestiWin/UnitJoukkuetiedot.cpp:~1470 (`BtnJjVaihtoClick`, case 103) | `wst[wcslen(wst-1)]` uses `wst-1` (pointer before the array) instead of `wcslen(wst)-1`; out-of-bounds read and write. | [V] expression, effect [I] |
| 10 | cbHk/UnitEmit.cpp:3449, :3483, :3001, :619 | `tallenna` without `tall_CriticalSection` (H-010/H-017 family); `:3483` is already H-017. | [V] |
| 11 | ViestiWin/UnitEmit.cpp:2267-2275, :2309-2336 | `MerkitseOhitetuksi` and `TallennaVaihto` take no `tall_CriticalSection` (only the inner `tallenna` locks per call); the multi-record swap is not atomic. | [V] |
| 12 | ViestiWin/UnitEmit.cpp:1013 (`NewCardHandler`) / :1004-1007 | `bdg<=0` path calls `MessageBoxW(msgtxt,...)` with `msgtxt` uninitialised (never filled in the relay copy; HK fills it via `pros_emit`). | [V] |
| 13 | ViestiWin/UnitArvonta.cpp:371-373 | `NumKaikki` uninitialised (H-069, already known). | [V] |
| 14 | ViestiWin/UnitArvonta.cpp:~340-350 vs cbHk | `static kk;` (implicit int) in `SeurPos` and `teeRankiJarj` caps at 1000 teams per class (`ix_jrj[1000]`, `n>1000 -> n=1000`, :154-175) so a class with more than 1000 teams is partly skipped silently in rank-order numbering. | [V] |
| 15 | cbHk/UnitEmit.cpp:2648, :2741 vs :2628 | `Kilp_0.tallenna(DKilp_0,-1,0,0,0)` vs `Kilp.tallenna(DKilp,0,-1,0,0)`: `comtarfl`/`kielto` swapped between the two calls; the same in the relay (ViestiWin/UnitEmit.cpp:2209 vs :2211). Possibly intentional (old competitor: send to network, new one: suppress). | [I] |

## 11. Open questions
- `kuntosuunnmoodi` values 1/2/3/98/99, `esiluenta` and `kuntoLahto` semantics: only observed through branches in `UnitEmit.cpp`; defining code in the console or init files not traced.
- Whether relay `Button2Click` (queue save) relies on a core step in `Juk/vajat.cpp:tall_rivi` to update the team record (HK form does it itself, :655-675). Not traced.
- Whether the HK `naytaRivi` write (UnitAjanotto.cpp:255-264) and relay twin (:311-332) is a deliberate "re-apply queue row to competitor when displayed" step: trigger not traced.
- Which fork is newer for items marked "fix on one side": commit history was not examined (`git log`/`blame` not run).
- H-017's form-held copy mechanism for relay `UnitJoukkuetiedot` was not rechecked beyond the locked compare at :518-528.

Key files:
- C:\dev\tulospalvelu\TPsource\V52\cbHk\UnitEmit.cpp
- C:\dev\tulospalvelu\TPsource\V52\ViestiWin\UnitEmit.cpp
- C:\dev\tulospalvelu\TPsource\V52\cbHk\UnitAjanotto.cpp
- C:\dev\tulospalvelu\TPsource\V52\ViestiWin\UnitAjanotto.cpp
- C:\dev\tulospalvelu\TPsource\V52\cbHk\UnitArvonta.cpp
- C:\dev\tulospalvelu\TPsource\V52\ViestiWin\UnitArvonta.cpp
- C:\dev\tulospalvelu\TPsource\V52\cbHk\UnitKilpailijatiedot.cpp
- C:\dev\tulospalvelu\TPsource\V52\ViestiWin\UnitJoukkuetiedot.cpp
- C:\dev\tulospalvelu\TPsource\V52\ViestiWin\OsuusTiedot.cpp
- C:\dev\tulospalvelu\TPsource\V52\cbHk\HkArv2.cpp
- C:\dev\tulospalvelu\TPsource\V52\Hk\HkIV.cpp