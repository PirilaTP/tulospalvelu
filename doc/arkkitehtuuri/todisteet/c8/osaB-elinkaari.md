<!-- Provenance: final report of architecture-analyst (Sonnet), C-8 step 1 part B (lifecycle of global state), 2026-11-26; written to file at the coordinator's request. Static reading and grep only; nothing was run. Paths relative to TPsource/V52/. -->

# C-8 step 1, part B: lifecycle of global state (state side)

## SUMMARY

- Startup order: HK and relay follow the same order: defaults, lue_parametrit, luesarjat, open KILP.DAT, teejarj(false), course and emit loading, inittimer, initcom, taustaon=1. No K2/K3 global is read before its initialisation. Zero is not a valid default for maaliajat (N-B5).
- Stage change: vaihdavaihe (Hk/HkIx.cpp:1855-2005) resets counters, rank chains, idindex, emit memory and time queues, and reloads Sarjat and pvparam from KilpSrj.xml. It skips most of the aloitus post-processing. Left from the previous stage: kilpparam.maxvaluku, lsakmax, mno/lsak clamps, eralahto, maaliajat, aikaflname template, lahdepiste latch, kaksibadge.
- Part A correction: teejarj does not recompute ixarr, sarjaindex, ljarjindex etc. Consumers rebuild them on demand with teeindeksi(..., true).
- Reopen: no reopen path inside a running HK process. Import only closes and reopens the same datf2. suljekilpailu has no callers. A second aloitus happens only after a failed start (Q67, partly answered in C-6).
- New findings: 14 suspected defects N-B1..N-B14 (4 medium, rest low, none high). None repeats H-013, H-031, H-094, H-097, H-102, H-104, H-105, H-047.

## B1. Initialisation order

### HK (shared core Hk/HkInit.cpp:3079-3716)

Line numbers are in Hk/HkInit.cpp unless stated. Console and GUI share the order; differences are noted. All [todennettu].

| # | Step | Evidence | Globals set |
|---|---|---|---|
| 0a | Console: wmain sets kilpparam defaults, calls aloitus | HkConsole/HkMain.cpp:176-203 | K1 |
| 0b | GUI: TFormMain constructor sets kilpparam, pvparam.pyor/laika*; Initialisoi calls aloitus only after the user picks a competition | cbHk/WinHk.cpp:185-230, :279 | K1; ToimintaTila set by caller (:1199, :1259, :1735) |
| 1 | InitializeCriticalSection x12 | :3099-3110; print_CS and autotul_CS at :3206-3207 | locks (Q67 territory) |
| 2 | Inline defaults: maxkilpno=9999, port arrays, kaikki_ajat[]=-1, aikaero, eralahto[i]=eralahto[0], pvparam laika and pyor | :3115-3162 | K1/K4 |
| 3 | ToimintaTila==2 sets k_pv=-1; kilpparam defaults | :3164-3175 | K2 |
| 4 | lue_parametrit (cfg and argv); sets k_pv from VAIHE= (:707); no reset of K1 | :3176, :552- | K1, k_pv |
| 5 | luevaihevalinta or k_pv==-1 prompt flag; arv_pv=k_pv | :3188-3193, :3211 | k_pv, arv_pv |
| 6 | ConsoleAloitus (console only), port/baud/parity parsing | :3208-3247 | K1 |
| 7 | luesarjat: lueEventXml -> loadEventData/loadRaceData/loadClassData, then alustaSarjatiedot | :3252; Hk/HkSrj.cpp:72-75; Hk/HkXml.cpp:449-1000 | Sarjat, pvparam, sarjaluku, kilpparam.* (kaksibadge, n_pv_akt, kilplaji) |
| 8 | Post-processing: clamp k_pv, tlsjakaja, emitfl default, maxvaluku, lsakmax, luetlskertoimet, luesarjayhdistelmat (sets nsarjayhd) | :3257-3355 | K2/K3 |
| 9 | File-name digit patching (obfilename, aikaflname, lokifname, emitfname) | :3357-3375 | K1 names (non-idempotent, N-B1) |
| 10 | sjfl, prior0, clamps of lsak and mno for k_pv only | :3377-3393 | K3 |
| 11 | Printers and log; init_muotoilut | :3398-3473 | K6 |
| 12 | Open or create KILP.DAT; maaliajat[unused mno]=TMAALI0 | :3478-3497 | datf2 |
| 13 | emitfl and kaikki_ajat normalisation | :3502-3580 | K4 |
| 14 | teejarj(false): allocates kilparr, yhdsija, nameindex, idindex, jarjtulos, jrjseur, bdg_kno, emitjarr; builds jalku, counters, badge map; sets kaynnistysvaihe=1 | :3582; Hk/HkIx.cpp:1554-1812 | K2/K3 |
| 15 | lue_radat_xml with fallback lue_leimat+lue_radat; lueemitfile, lue_lainat; vakanttisarja enspv/viimpv | :3593-3620 | rata, kuvio, Map, rastiva, rvalit, emithead*, emitptr, lainaemit |
| 16 | GUI only: lueKorRajat. kaynnistysvaihe=2; inittimer (readers, aikajono+initajat); initcom under inb_CS | :3622-3668 | |
| 17 | kaynnistysvaihe=3; gaz, autofile, autova; taustaon=1 | :3670-3695 | K4 |
| 18 | Console: initscreens, ProsEmitOn, taustaThr. GUI: LukumaaratTh/ProsEmitThread started in Initialisoi after aloitus returns | :3703-3708; cbHk/WinHk.cpp:~340-352 | |
| 19 | kaynnistysvaihe=4 | :3712 | |

### Relay (Juk/VInit.cpp:2538-2966)

- Differences from HK: explicit asetaOletusarvot (:2460-2535) also resets kaikki_ajat=-1, aikaero and every Sarjat[i].nollaa() (:2535); HK has no Sarjat reset. luesarjat comes before kilprecsize is computed (:2654-2662). teejarj() at :2846 precedes lue_radat_xml and lueemitfile (:2867-2886). taustaThr is started in both console and GUI (:2957-2958).
- No k_pv, arv_pv or vaihdavaihe in relay. [todennettu]

### Read-before-init check

- idindex, kilparr, maxrec, nrec are NULL/0 until teejarj(false) (Hk/HkIx.cpp:1568-1602; initindex :1592). No call of getpos/GETREC/PUTREC found between aloitus start and step 14 (read of aloitus and loaders, not a full call-graph trace). [paatelty]
- nsarjayhd stays 0 when ToimintaTila!=2 (:3313-3331); zero valid. [todennettu]
- arv_pv copied from k_pv at :3211 before clamp (:3257), prompt (:3322), second clamp (:3381): N-B13.
- maaliajat[used mno]: only unused entries get TMAALI0 (:3497); used rely on static zero, which is a legal clock value: N-B5.
- pros_bdg_t starts lazily on first badge (Hk/HkIV.cpp:831); taustaon=1 only at :3695 after all loaders and inittimer. No thread found that exits early because of this. [paatelty]
- Relay nbadge[0], nbadge[1] only incremented (Juk/VIx.cpp:1192, :1213); no assignment anywhere in Juk/ (grep). Matters on rerun (B4).

## B2. Stage change vaihdavaihe (Hk/HkIx.cpp:1855-2005) and callers

### Callers

| Caller | Evidence | Notes |
|---|---|---|
| Console menu | HkConsole/HkCons0.cpp:933 | sets vaiheenvaihto; flag stays on refusal (H-031) |
| Network request (console and GUI) | Hk/HkCom32.cpp:844-847 -> pyyda_vaihdavaihe; console HkConsole/HkCons0.cpp:869-871; GUI SendMessage cbHk/WinHk.cpp:2231-2235 | tark_alku gates on edvaihto (5 s) and var[cn] |
| GUI menu Vaiheenvaihto1Click | cbHk/WinHk.cpp:2049-2145 | also reread-data menu item = vaihe==k_pv+1 (:2066-2068) |
| GUI VaihdaVaiheHandler | cbHk/WinHk.cpp:2149-2229 | |

Both GUI handlers set vaiheenvaihto, lkmThreadOn=0, ProsEmitOn=false, delete FormOsanottajat, FormMaastossa, TulosteForm, FormSprinthiihto, FormEmit, FormAjanotot, call vaihdavaihe, then restart LukumaaratTh and ProsEmitThread. Thread and lock issues are C-6 (H-094, H-097, H-102, H-107, T-033) and not repeated.

### Reset table

Code in Hk/HkIx.cpp unless stated.

| Item | Disposition | Evidence | If stale: used afterwards? effect |
|---|---|---|---|
| k_pv, arv_pv | reset | :1931-1932 | arv_pv is shared scratch overwritten by many forms (cbHk/HkArv2.cpp:609, UnitArvonta.cpp:408) |
| obfilename[7] | reset | :1888-1889 | |
| aikajono[]->aikafname digit | reset | :1890-1894 | only if p[-1] equals old digit |
| aikaflname[2][] (queue-file template) | STALE | set only HkInit.cpp:3364-3367; used HkIV.cpp:4227 | yes: N-B3 |
| lokifname | stale | HkInit.cpp:3368 | name only, log handle already open; harmless |
| emitfname[4] | reset only on success path (:1942) | | stale after failure, but emitfl=0 then |
| comopen[] | reset then restored | :1886-1887, :1983 | |
| counters nilmt, ntulost, nhylt, nkeskt, neilt, nesitt, nvakt, nva, nilm, nyilm, ntulos, nhyl, nkesk, neil, nvak, neilaika, nilm_pv, nesit | reset, then recomputed | reset :1913-1930; recomputed by addjarjarr in teejarj(true) :1759-1761 (increments Hk/HkIx.cpp:846-848) | |
| jalku | reset | :1656-1658 | |
| jarjtulos, jrjseur, njarjpst | reallocated, recomputed | DeAllocJarjSeur :1612, AllocJarjSeur :1615, :63-85 | njarjpst depends on k_pv>0 (:65-68), recomputed |
| yhdsija | reset | :1604-1609 | zero until consumer recomputes (sijat2 yhdarr) |
| idindex | reset, recomputed by setpos | :1610, :1757-1758 | id() can vary per stage; lock gap = H-102 |
| nameindex | left, valid | built only when not uusinta (:1741-1745, :1785-1787) | keyed by name only; maintained by addtall/deltall/tallenna |
| ixarr indexes (sarjaindex, seuraindex, lahtoindex, ljarjindex, aakkosindex, nroindex, srjnroindex, rataindex, seuranoindex, srjaakindex), pstindex, vaihetlsindex | left; REBUILT ON DEMAND | teejarj(true) calls neither initindex nor teeindeksi (:1603-1613); teeindeksi always rebuilds (:240-286, reindex=1 at :241 makes eday cache dead); all callers pass uusi=true: cbHk/HkArv2.cpp:615, :1172; UnitArvonta.cpp:265, :309, :574, :845, :907; UnitPistelasku.cpp:242; HkTulostus.cpp:8561; UnitExport.cpp:314, :321; HkKisaLahtLuet.cpp:446, :506, :768, :2070 | stale buffer not used; corrects part A [paatelty] |
| Sarjat, pvparam (all stages) | reloaded from KilpSrj.xml | :1934 -> Hk/HkSrj.cpp:72-75 -> Hk/HkXml.cpp:1663-1852 | additive: valuku grows, never shrinks (HkXml.cpp:801-803); loadClassData does not clear Sarja; absent XML fields keep old value |
| kilpparam.* read from XML | reloaded | HkXml.cpp:449-520 | overwrites aloitus post-processing: N-B8 |
| kilpparam.maxvaluku, lsakmax | STALE | set HkInit.cpp:3324-3328, :3345-3347; XML path returns at HkSrj.cpp:72-75 before :479 | used HkIV.cpp:4211 (maxntime, first queue creation), HkIx.cpp:1494 (dead suljekilpailu), cbHk/UnitStatus.cpp:340-359, TapUnit2.cpp:181, :458: N-B6 |
| Sarjat[].mno, lsak clamps | LOST on reload | clamp for k_pv only HkInit.cpp:3383-3392; XML unclamped HkXml.cpp:676, :695 | maaliajat[mno-1] (HkDat.cpp:1320, HkTulostus.cpp:3729): N-B7 |
| prior0, fprior, prior, sjfl | left | prior0 const 1 (HkInit.cpp:3384); sjfl has NO reader (only commented HkTulostus.cpp:1182; writes HkDat.cpp:1314, HkTulostus.cpp:1226, HkInit.cpp:3377) | harmless; prior/fprior decayed/incremented elsewhere (HkIV.cpp:3702, HkDat.cpp:1335-1338) |
| modif | recomputed | set TRUE in addjarjarr (HkIx.cpp:845) during teejarj(true); cleared by autoprint (HkTulostus.cpp:5994-6019) | probably forces one autoprint round of all series [paatelty] |
| tlsmuutos | GUI-side | cbHk/UnitStatus.cpp:340, :299 | status form re-inits when k_pv differs from ed_pv |
| maaliajat[9] | STALE | HkDat.cpp:1320 | N-B5 |
| tmaali | no writer found | reads only: HkTulostus.cpp:6562, :6638; cbHk/HkKisaLahtLuet.cpp:1713; HkConsole/HkCons2.cpp:227 | [tuntematon] |
| kaikki_ajat[] | left | config; read :1952 | stage-independent |
| eralahto[], era_akt | STALE | vaihdavaihe has no reference | N-B4 |
| yhtjonot | n/a | only read (HkCom32.cpp:436); assignment commented (HkInit.cpp:1439) | constant 0 |
| emitpos | n/a | no references in Hk/, cbHk/, HkConsole/ | dead name |
| rata, nrata, kuvio, maxnkuvio, lainaemit, rvalit, rastiva | freed, reloaded | freed by vapautaemitmuisti(true,true,true,true) (:1908; Hk/HkEmit.cpp:6365-6396); reloaded by lue_radat_xml(7) (:1937; HkEmit.cpp:852-863), siirraXml* -> tarkkilpradat (HkEmit.cpp:626, :2797-2803) | failure: rata/kuvio stay NULL; esitark/esiluenta==1 skips block = H-105; missing XML = N-B12 |
| nkuvio | stale | vapautaemitmuisti zeroes maxnkuvio only (HkEmit.cpp:6376-6378) | matters only if reload fails; emitfl=0 then [paatelty] |
| rvluku | stale | not reset (HkEmit.cpp:6389); analradat += at :2531 | over-allocates rvint only |
| Map | overwritten | HkEmit.cpp:284-291 | |
| CourseData | reset | nollaa() HkXml.cpp:1193 | |
| EmitErot, nEmitErot | left; rebuilt on demand | HkEmit.cpp:1695-1707; cbHk/UnitEmit.cpp:2051 | static cursor kEmitErot (UnitEmit.cpp:1938) clamped :2052; harmless |
| emithead, emithead2, emitptr | reloaded | lueemitfile HkEmit.cpp:3059-3078 | stale on skip = H-105 |
| emitjarr | reset, then setEmitJarr | HkIx.cpp:1622-1628; HkEmit.cpp:3044 | [1] reset only when kaksibadge==2: N-B8 |
| bdg_kno, nbadge | recomputed | memset :1630, nbadge=0 :1661, rebuilt :1765-1809 | |
| emitfl | set 0 on failure | :1939, :1945 | GUI callers check emitfl before restarting ProsEmitThread (cbHk/WinHk.cpp:2135, :2217) |
| aikajono contents | reloaded | suljeajat(-1,false) :1904; initajat(ino,2) :1970 (in-memory clear Hk/HkIV.cpp:4003-4015, then reads new stage file) | not reloaded if aikajono[0] exists but ajanottofl<=0 and no regnly (:1962-1968): queue holds old entries, file closed [paatelty] |
| htime[] | recomputed | initajat | |
| korRaja, vipLaji (GUI) | STALE | HkInit.cpp:217-266 reads korostus<k_pv+1>.lst; called only :3624 and cbHk/AsetusUnit.cpp:53 | N-B10 |

### Function-local statics (K3/K4) after a stage change

| Static | Evidence | Disposition and effect |
|---|---|---|
| tall_emit::ed_em[r_no] | Hk/HkIV.cpp:961, used :1145-1149 | stale; last emit record per reader for repeat suppression; harmless [paatelty] |
| tall_bdg_t::edbadge, add_bdg_t::edbadge | HkIV.cpp:756, :822, used :762-764, :836-838 | stale; same badge as last read before change dropped once (when ohitatoisto is off); very low [paatelty] |
| pros_emit::edkno | HkIV.cpp:1164, :1411-1414 | stale; prompt prefill only |
| pros_emit::uusin, tall_emit::uusin | HkIV.cpp:957, :1164 | use not traced [tuntematon] |
| on_emit::emitvaraus, nemitvar | HkEmit.cpp:1303-1327 and :1627-1651 | table freed by vapautaemitmuisti (:6367), nemitvar not reset: N-B11 |
| tausta::prev_t, alku_pv | HkIV.cpp:3621-3622, :3664-3667 | init latch, written once, never read again in tausta; not stage-dependent (part A called prev_t K3) [todennettu] |
| tausta::ed_lkm_t, edlahkello, t_af | HkIV.cpp:3619-3625 | timing/polling caches, not stage-dependent |
| luelahdepisteet::luettu | HkIV.cpp:204-210 | latch: N-B9 |
| lue_radat::init | HkEmit.cpp:2863, :2868-2875 | latch for text-course file-name digit, once per process (N-B12) |
| initajat::chs | HkIV.cpp:3969- | caches last prompt answer; prompt itself is T-033 territory |
| teeindeksi::eday | HkIx.cpp:240, :248 | dead: reindex=1 set first |
| tark_alku::var[cn], edvaihto | HkCom32.cpp:823, :844 | var[cn] never reset: stage/KILP.DAT mismatch warning once per connection per process (:850-858); cosmetic |

Not read: tall_regnly, writegaz, etHaku, tall_ampsakko statics; relay twins in Juk/.

### Resolution of the part A question-mark list (31 names)

- kuvio, nkuvio, rvalit, rvluku, rastiva, Map, lainaemit, rata, nrata, EmitErot, nEmitErot: freed/reloaded as above. Stage-dependent: yes, per-stage RADAT<k_pv+1>.XML (HkEmit.cpp:849).
- bdg_kno, nbadge, emitjarr, emitfl, emithead, emithead2, emitptr, emitfile: reset/recomputed. emitpos does not exist.
- maaliajat stale (N-B5); tmaali no writer; kaikki_ajat config; era_akt stale (N-B4); yhtjonot constant.
- sjfl no reader; prior, prior0, fprior not stage-keyed; modif recomputed; tlsmuutos GUI-handled.

## B3. Reopening a competition without a restart

1. Import (in-memory side of H-047). TFormImport::Button1Click ends with closefile(datf2) then openfile(datf2, dataf2name, kilprecsize) (cbHk/UnitImport.cpp:1434-1437). Records are added with addtall (:1405-1409; TallEhdoin :99-110). addtall maintains kilparr, idindex, nameindex, rank arrays, bdg_kno (Hk/HkDat.cpp:674-722; addfrec :630-640). Not re-initialised: maxrec (addtall refuses at nrec >= maxrec, :681), kilparr size, jarjtulos allocation, kilpparam, Sarjat (the XML participant import at :1172 also keeps them). nrec stays correct (addfrec sets it, :635). So import is same competition, new rows, not a reopen. [todennettu]
2. TFormValitseKilp. BtnValittuClick writes the globals kilpailu and pvparam[0].Date at click time (cbHk/UnitValitseKilp.cpp:59-62), before aloitus; if aloitus fails or the user cancels, the globals hold the new competition title and date. Initialisoi: SetCurrentDir(path) then aloitus (cbHk/WinHk.cpp:252-279). After a successful aloitus BtnIlmTila and BtnTulPalv are disabled (cbHk/WinHk.cpp:~285-289); a second selection is possible only after a failed start (ToimintaTila=0, :279-283). [todennettu]
3. suljekilpailu (Hk/HkIx.cpp:1481-1512) is dead code: grep -rn suljekilpailu TPsource/V52 finds only the definition. [todennettu] If called it would clear bdg_kno, jrjseur, jarjtulos, nameindex, idindex, kilparr. It would leave emitjarr[0/1], yhdsija, jalku, nrec, maxrec, the nilm counters, ixarr buffers and nbadge untouched. It would not free jarjtulos[yhd][valuku+3] (loop to valuku+3, while AllocJarjSeur allocates to valuku+4, :78), and it skips maxvaluku < i <= valuku which AllocJarjSeur did allocate (leak). Not a complete teardown, and nothing rebuilds the cleared state.
4. GUI Nollaa (TFormNollaus::Button1Click, cbHk/UnitNollaus.cpp:56-117): with Emit ticked it sets badge[0/1]=0 in the record, then tallenna; bdg_kno is written only by addbadge, rembadge, teejarj -> N-B14. [paatelty]

## B4. aloitus run twice (Q67): state consequences

C-6 already answered (saikeet-ja-lukot.md:577): aloitus can run again after an early failure because the buttons stay enabled (cbHk/WinHk.cpp:279-290, :1176-1262; ViestiWin/UnitMain.cpp:220-228); InitializeCriticalSection then runs on locks not held; initcom always returns 0 (H-099 refuted); helper threads started before the failure point were not checked. Not repeated.

State consequences not yet recorded:

| Consequence | Evidence |
|---|---|
| In-place file-name patching is non-idempotent for n_pv_akt==1: obfilename COMFILE1.DAT -> COMFILE.DAT -> COMFILEDAT; aikaflname[0] AJAT1.LST -> AJAT.LST -> AJATLST; [1] and emitfname likewise | HkInit.cpp:3357-3360, :3371-3372; defaults HkGlobals.cpp:127, :132, :519. N-B1 |
| teejarj(false) does not zero counters; nilm, nilmt, ntulos etc. are doubled when the first run passed teejarj and then failed | HkIx.cpp:1568-1602 no reset; increments :846-848; resets only in uusijarj (:1519-1535) and vaihdavaihe (:1913-1930); failure returns after teejarj HkInit.cpp:3598, :3604, :3610, :3661. N-B2 |
| Earlier allocations leaked by pointer overwrite: kilparr new[] (HkIx.cpp:1584), yhdsija (:1590), nameindex.ix (:1596), idindex ix (:1600), jrjseur/jarjtulos (:74, :79), bdg_kno/emitjarr (:1633-1636) | |
| datf2 opened again, old handle leaked | HkInit.cpp:3486; lopetus1 closes datf2 only when kaynnistysvaihe>1 (:3774-3790), teejarj sets it to 1 |
| K1 from the previous cfg survives; lue_parametrit assigns only keys present | HkInit.cpp:552-; only the head of the function read [paatelty] |
| Sarjat not reset before reload (relay does: Juk/VInit.cpp:2535) | HkInit.cpp:3252; HkXml.cpp:841- |
| Relay: nbadge[] not reset (Juk/VIx.cpp:1192, :1213); nilmtt never reset (Juk/VIx.cpp:657; display only: ViestiWin/UnitStatus.cpp:378, JukConsole/laskent0.cpp:262); bdg_kno capacity maxrec*n_os_akt (Juk/VIx.cpp:1105), a rerun would append after the old nbadge; overflow not proven | [paatelty] |
| k_pv carry-over on a rerun in ToimintaTila==1: the -1 reset is only for state 2 | HkInit.cpp:3164-3165; low |

## B5. Shutdown (state aspects outside H-038)

- lopetus1 frees no memory; the critical-section deletions are commented out (Hk/HkInit.cpp:3792-3818); process exit frees everything. [todennettu]
- No final state write in lopetus1. Persistence relies on write-through PUTREC(d,1) -> write() (HkDat.cpp:614-621; default datatallfl={1,0}, HkGlobals.cpp:597). PUTREC(d,0) is not written to file. Noted only; C-1 owns tallenna. [paatelty]
- Only persisted stage state: vaihevalinta.cfg, written at the end of vaihdavaihe when vaihevalinta is set (HkIx.cpp:1997-1998; HkInit.cpp:314-325). Restart returns to the last chosen stage. [todennettu]
- closefile(datf2) and fclose(femitfile) only when kaynnistysvaihe>1; after a teejarj failure the file stays open until process exit. [todennettu]
- No new shutdown defect beyond H-038, H-095, H-104.

## NEW SUSPECTED DEFECTS (N-B*)

### N-B1 (medium; arithmetic [todennettu], trigger [paatelty])
- Claim: aloitus patches file names in place (drops the stage digit) with wmemmove; not idempotent.
- Evidence: Hk/HkInit.cpp:3357-3360, :3371-3372 (EMITLEIMAT defined if (MAALI or AUTOVA) and EMIT, HkDef.h:110-112); defaults HkGlobals.cpp:127, :132, :519.
- Trigger: GUI, n_pv_akt==1, first aloitus fails after :3357, retry without restart (Q67 path).
- Consequence: COMFILEDAT, AJATLST and a corrupted emit name: a new empty comms-queue and timing file; old history not found; no error shown.

### N-B2 (medium; [paatelty])
- Claim: teejarj(false) rerun leaves counters doubled, arrays leaked, datf2 opened twice.
- Evidence: HkIx.cpp:1568-1602, :846-848; HkInit.cpp:3486, :3598-3661.
- Trigger: first run passes teejarj and fails at :3598, :3604, :3610 (course file error, emit file prompt answered K) or :3661, then retry.
- Consequence: per-series and total counts (nilm, ntulos, nhyl...) shown doubled; rank chains rebuilt correctly.

### N-B3 (medium; path [todennettu], trigger [paatelty])
- Claim: vaihdavaihe updates the digit of existing aikajono[]->aikafname but not the template aikaflname[] that inittimer uses for new queues.
- Evidence: HkIx.cpp:1888-1894 vs HkIV.cpp:4227, HkInit.cpp:3364-3367; GUI creation points cbHk/UnitAjanotto.cpp:96-98, cbTp/UnitEmiTag.cpp:382.
- Trigger: GUI, aikajono[0]==NULL at the stage change (no timing opened yet), then the time-keeping form or a reader is opened.
- Consequence: the stage-1 queue file AJAT1.LST is opened; initajat offers to read stage-1 times into the new stage; new times are written to the old stage file.

### N-B4 (medium; [paatelty])
- Claim: eralahto[] and era_akt are not reset at stage change.
- Evidence: vaihdavaihe has no reference; use HkIV.cpp:2407-2410, :2529-2543; HkDat.cpp:1255-1257.
- Trigger: multi-stage kilplaji N/P/U or ARES reader with batch (era) start times.
- Consequence: a stage-1 batch start remains (first start is recorded only when the value is the sentinel); stage-2 readings get the stale offset (HkIV.cpp:2407-2408).

### N-B5 (low; [paatelty])
- Claim: maaliajat[] not reset at stage change; at start only unused entries are reset.
- Evidence: HkDat.cpp:1320; HkTulostus.cpp:3729, :3860; HkInit.cpp:3497; tulos1 HkTls.cpp:737-775 (subtracts TLahto then normalises).
- Trigger: list output in stage 2 before the first finish at that gate.
- Consequence: the parenthesised provisional time of runners still on course is computed against the previous stage last finish; the start-up value 0 is also a legal clock time.

### N-B6 (low; [paatelty])
- Claim: kilpparam.maxvaluku and lsakmax are computed only in aloitus for the start stage, not at stage change (XML path returns before HkSrj.cpp:479).
- Evidence: HkInit.cpp:3324-3328, :3345-3347; HkSrj.cpp:72-75; consumers HkIV.cpp:4211, cbHk/UnitStatus.cpp:340-359, TapUnit2.cpp:181, :458.
- Trigger: stages with different numbers of splits or penalty counts.
- Consequence: wrong GUI grid columns and button counts; maxntime fixed at first queue creation.

### N-B7 (low; [paatelty])
- Claim: mno and lsak are clamped in aloitus only for the start stage; a reload re-reads them from XML unclamped.
- Evidence: HkInit.cpp:3383-3392 vs HkXml.cpp:676, :695.
- Trigger: hand-edited XML (mno 0 or above 9) in the new stage.
- Consequence: maaliajat[mno-1] out of range (HkDat.cpp:1320). Program-written XML stays in 1..9.

### N-B8 (low; [paatelty])
- Claim: a reload can lower kilpparam.kaksibadge from 2 to 1.
- Evidence: HkInit.cpp:3293-3295 sets 2 for kilplaji S with alalaji A; HkXml.cpp:503-506 sets 1 if the Badge text has the suffix *2; luesarjat at HkIx.cpp:1934.
- Trigger: stage change in such a competition.
- Consequence: emitjarr[1] is neither reset (HkIx.cpp:1626, :1637, :1641 test ==2) nor looked up (HkEmit.cpp:1087, :2284, :2296): stale second-badge entries from the previous stage emit file.

### N-B9 (low; code [todennettu], effect conditional)
- Claim: the first-call latch luelahdepisteet::luettu stops the per-stage LAHDEPISTEET<k_pv+1>.LST being read again.
- Evidence: HkIV.cpp:204-210; called at the first inittimer (HkIV.cpp:4258-4262).
- Trigger: lahdepistehaku active; per-stage files differ.
- Consequence: stage-1 start points stay in force in stage 2.

### N-B10 (low; [todennettu])
- Claim: lueKorRajat (korostus<k_pv+1>.lst) is not re-run at stage change (GUI).
- Evidence: HkInit.cpp:217-266; callers :3624, cbHk/AsetusUnit.cpp:53.
- Trigger: GUI, per-stage highlight files.
- Consequence: highlight thresholds and VIP marks of the previous stage.

### N-B11 (low; code [todennettu], capacity effect [paatelty])
- Claim: on_emit frees emitvaraus but does not reset nemitvar.
- Evidence: HkEmit.cpp:1303-1327 / :1627-1651; vapautaemitmuisti :6367; default estaemittoisto=1 (HkGlobals.cpp:523).
- Trigger: every stage change, lueRatatiedotUudelleen, uusiemitanalyysi.
- Consequence: the table is re-allocated as zeros and nemitvar keeps the old count; the array stays sorted but capacity (maxrec+1000) is consumed by phantom zeros; after a few changes the table is full and duplicate-card suppression stops silently. The free also empties the table.

### N-B12 (medium; [todennettu])
- Claim: a stage change with no RADAT<k_pv+1>.XML (lue_radat_xml returns -1) sets emitfl=0; aloitus falls back to the text course files (lue_leimat and lue_radat) in that case.
- Evidence: HkIx.cpp:1936-1941 vs HkInit.cpp:3593-3607; rataxmlfname index 5 set to k_pv+1 at HkEmit.cpp:849.
- Trigger: installation with legacy text course files, or one course XML for all stages.
- Consequence: rata and kuvio already freed, emit checking off until restart (an error message is shown). The text path file-name digit is also set once only (lue_radat::init, HkEmit.cpp:2863-2875).

### N-B13 (low; [paatelty])
- Claim: arv_pv=k_pv is set at HkInit.cpp:3211, before the clamp (:3257), the prompt (:3322) and the second clamp (:3381).
- Evidence: HkInit.cpp:3190-3193, :3211, :3257-3258, :3322, :3381; consumers HkIx.cpp:248-278, :347, :420-492; VAIHE= has no upper clamp (:706-707).
- Trigger: cfg with VAIHE= beyond n_pv_akt, or the interactive stage prompt.
- Consequence: arv_pv is 0 or out of range until another form overwrites it; index arguments such as Sarja(arv_pv).

### N-B14 (low; [paatelty])
- Claim: GUI Nollaa with Emit ticked clears badge[] directly, so bdg_kno is not updated.
- Evidence: cbHk/UnitNollaus.cpp:106-111; bdg_kno writers are only addbadge, rembadge, teejarj (part A). Whether tallenna also touches bdg_kno was not traced.
- Trigger: reset with Emit ticked, then a read of a cleared badge.
- Consequence: the badge still maps to the old competitor number until the next teejarj.

Existing items touching the same area, no new claim: H-013 (HkIx.cpp:1834 loop), H-031, H-094, H-097, H-102, H-104, H-105, H-047.

## PART A CORRECTIONS

- teejarj does not recompute ixarr, sarjaindex, ljarjindex, aakkosindex, nroindex etc. (HkIx.cpp:1603-1613); consumers rebuild with teeindeksi(..., true) (reindex=1 at HkIx.cpp:241).
- suljekilpailu is dead code (no callers in V52) and incomplete as a teardown.
- sjfl has no reader; emitpos has no references; yhtjonot is constant; tmaali has no writer found; kaikki_ajat is stage-independent config.
- tausta::prev_t and alku_pv are init latches, not stage-dependent caches.
- teeindeksi::eday is dead.

## OPEN QUESTIONS

1. N-B2: does addjarjarr guard against inserting the same record twice? Only the counter increment (HkIx.cpp:846-848) was read, not the whole function (814-1160).
2. Which first-run failures leave the buttons enabled in the shipped GUI? Part of this is the Q67 helper-thread question left open in C-6.
3. N-B4: which readers and kilplaji really use eralahto offsets (HkIV.cpp:2407)? Reader type not traced.
4. Where is tmaali written (by reference via maaliaika, or in bulk)? [tuntematon]
5. pros_emit::uusin and tall_emit::uusin use not traced; relay twins of the statics and relay vaihda_badge::seurbdg not examined.
6. lue_parametrit is 2200 lines; whether any key resets rather than assigns is unchecked beyond about 40 lines.
7. Is taydenna_leimat (HkEmit.cpp:806) always followed by tarkkilpradat? Inferred from siirraXmlRadat (:626) only.
8. Does rastivatp free its ratava arrays when analradat does delete[] rastiva (HkEmit.cpp:2527-2531)? Possible leak per stage change. [tuntematon]

## COVERAGE

- Read in full: Hk/HkInit.cpp aloitus (3079-3716) and lopetus1 (3730-3830); Hk/HkIx.cpp 40-340 and 1481-2005; Juk/VInit.cpp aloitus (2538-2966); cbHk/WinHk.cpp Initialisoi (partly) and stage-change handlers (1150-1260, 1690-1760, 2040-2290).
- Read in part: Hk/HkXml.cpp (lueEventXml fully; loadEventData; loadRaceData, loadClassData, loadCourseData by keyword search); Hk/HkSrj.cpp 30-200, 395-520; Hk/HkEmit.cpp (vapautaemitmuisti, lueemitfile, lue_radat_xml, on_emit, analradat, haeEmiterot); Hk/HkIV.cpp 750-1010 (partly), 3605-3720, 3969-4130, 4170-4330; Hk/HkDat.cpp 590-780; Juk/VIx.cpp teejarj (1062-1345); cbHk/UnitImport.cpp, UnitValitseKilp.cpp, UnitNollaus.cpp (relevant segments); Hk/HkCom32.cpp 815-870.
- Not read: lue_parametrit beyond about 40 lines; relay luesarjat; relay GUI Initialisoi beyond its call site; relay twins of the function-local statics; addjarjarr/remjarjarr bodies (cited from part A); tall_regnly, writegaz, etHaku, tall_ampsakko statics; all lock and thread aspects (C-6).
- Static reading and grep only; nothing was run.
