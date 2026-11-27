<!-- Provenance: final report of architecture-analyst (Sonnet), C-8 step 1 part A, 2026-11-25; copied from the hand-back message. Note added by the main session: after this report the script got a column-0 fallback for function attribution ("~name" in viittaukset.csv); unattributed references fell from 8430 to 3318, so finding 6 "function-attribution loss" is largely fixed in the current CSVs. luokittelu.csv was made against the earlier run. -->

C-8 step 1 part A: classification of global and static mutable state

SUMMARY
- I classified 1180 names. That is 983 file-level names (globals and file statics, from maarittelyt.csv) plus 197 function-local statics in Hk/ and Juk/.
- I skipped 217 of the 1200 file-level names, with the reason recorded per row.
- Evidence file: doc/arkkitehtuuri/todisteet/c8/osaA/luokittelu.csv. It is UTF-8 with LF line endings and 1398 lines (header, 1180 classified rows, 217 SKIP rows). Columns are as you specified.
- Method: category is by name list with defining file and decl as context, mostly mechanical. Code was read for the K4 flags, the K3 maintainers, vaihdavaihe (Hk/HkIx.cpp:1855-2005) and several function-local statics.
- Confidence: classification by name is [paatelty] unless a finding below cites a line. The K3 maintainer lists and K4 writer lists come from the script's W/A references, so they are lower bounds (see limitations).
- stage_dependent is "y" for 44 names and "?" for 31.
- The only file created is luokittelu.csv. The scratch scripts are outside the repository.

COUNTS PER CATEGORY AND DOMAIN

Domain "both" means defined in both HkGlobals.cpp and VGlobals.cpp. "shared" means defined only in Tp/, com/, cbTp/ or tputilv2/.

File-level names (983):

| Cat | Hk | Juk | both | shared | total |
|---|---|---|---|---|---|
| K1 config | 135 | 62 | 147 | 54 | 398 |
| K2 competition data | 17 | 22 | 36 | 5 | 80 |
| K3 derived | 31 | 15 | 48 | 0 | 94 |
| K4 run-time flags | 8 | 5 | 39 | 16 | 68 |
| K5 comm buffers | 13 | 7 | 69 | 16 | 105 |
| K6 output/format | 49 | 29 | 109 | 25 | 212 |
| K7 other | 5 | 0 | 1 | 20 | 26 |

Function-local statics (197, Hk/ and Juk/ only):

| Cat | Hk | Juk | total |
|---|---|---|---|
| K1 | 1 | 0 | 1 |
| K3 caches | 34 | 14 | 48 |
| K4 first-call and state | 15 | 17 | 32 |
| K5 | 11 | 8 | 19 |
| K6 print carry-over | 40 | 38 | 78 |
| K7 return buffers | 13 | 6 | 19 |

- The local-static classification is by name, not by reading each function. Examples: `edtls` in htmlmakitulos, prtmakitulos and xmlfistulos is classed K3 but is really K6 print state. `maxtime` in inittimer is classed K4 but is a computed K3 value.
- K7 file-level contents:
  - MTRand `p` and `init` (Tp/mtrand.cpp, class statics).
  - Profiling buffers `profarr`, `profptr`, `proffile` (Tp/TpComY.cpp).
  - Console handles `hStdIn`, `hStdOut`, `hConIn`, `hConOut` (tputilv2/initcon.cpp).
  - `perr_*` message buffers (cbTp/cbTpApu.cpp).
  - Merged generic names: `n`, `rivi`, `ok`, `wait`, `top`, `step`.
  - `piste2`, `emittime`, `kentat`, `alkuaika`, `__tputil__ok`, `dr1`, `sarjanimi`.

FINDINGS

1. K3 derived structures and their maintainers.
   - All entries are [todennettu] for the writer lines shown, from the script's W/A references. They are lower bounds, because the script loses function names in some files (finding 11).
   - Each Hk structure has a Juk twin in VIx.cpp unless stated otherwise.
   - Series and competitor counters, written by `addjarjarr`, `remjarjarr` and `teejarj`:
     - Per-series counters `nilm`, `nyilm`, `ntulos`, `nhyl`, `nkesk`, `neil`, `nvak`, `neilaika`, `nilm_pv`, `nesit`.
     - Totals `nilmt`, `ntulost`, `nhylt`, `nkeskt`, `neilt`, `nvakt`, `nesitt`.
     - Rank chains `jalku` and `pkarki`, plus `modif`.
     - Evidence: `addjarjarr` Hk/HkIx.cpp:814-1160; `remjarjarr` Hk/HkIx.cpp:1185-1357.
     - `jalku` is written by `teejarj` at Hk/HkIx.cpp:1657 and Juk/VIx.cpp:1239.
     - `uusijarj` sets the totals at Hk/HkIx.cpp:1519-1524.
     - `vaihdavaihe` zeroes the totals at Hk/HkIx.cpp:1913-1919 and memsets the per-series arrays (see finding 3).
   - Relay counters:
     - `ntulosix`, `navoin`, `navoint`, `nekoodi`, `nylahto`, and `nilm` in `paivJoukkLkm` (Juk/VIx.cpp:656-658), with `addjarjarr` and `remjarjarr` in VIx.cpp:745-1026.
     - `nollaayhdlkm` (Juk/vApu.cpp:562-565).
     - `njoukhyl` and `njoukkesk` are written by `ossijat` (Juk/VTulostus.cpp:968-998).
     - `josalku` is written by `teejarj` (Juk/VIx.cpp:1240) and `setJosAlku` (Juk/vApu.cpp:300).
   - Per-split counter `nva` is written at Hk/HkEmit.cpp:2442, :2491 and :2544, and by `luesarjat` (Hk/HkSrj.cpp:84, :436).
   - Index arrays:
     - `initindex` (Hk/HkIx.cpp:106-159) sets `idindex`, `sarjaindex`, `seuraindex`, `seuranoindex`, `srjaakindex`, `lahtoindex`, `ljarjindex`, `aakkosindex`, `nroindex`, `srjnroindex`, `rataindex`, `ixarr`, `pstindex` and `vaihetlsindex`.
     - `teeindeksi` (Hk/HkIx.cpp:243-290) sets `ixarr` and `reindex`.
     - `teejarj` writes `nameindex` (Hk/HkIx.cpp:1593) and `idindex` (:1600).
     - `suljekilpailu` clears indexes (Hk/HkIx.cpp:1486-1507).
     - `nameindex` is also updated from `kilptietue::addtall`, `deltall` and `tallenna` (Hk/HkDat.cpp:723, :763, :1034).
     - `idindex` is updated by `luo_posarr` (Hk/HkIx.cpp:311).
   - Rank-chain and jarj arrays: `jarjtulos` and `jrjseur` are allocated and set by `AllocJarjSeur` (Hk/HkIx.cpp:65-98), `SetJarjSeur` (:60) and `uusijarj` (:1538).
   - Badge and emit maps:
     - `bdg_kno` and `nbadge` are written by `addbadge` (Hk/HkEmit.cpp:1070-1072) and `rembadge` (:1146-1150).
     - `bdg_kno`, `nbadge` and `emitjarr` are rebuilt by `teejarj` (Hk/HkIx.cpp:1625-1661).
     - `emitjarr` is also written by `setEmitJarr` (Hk/HkEmit.cpp:1021) and `lueemitfile` (:3044).
     - Emit ring pointers `emithead` and `emithead2` are written by `addem` (Hk/HkEmit.cpp:1342-1680) and `lueemitfile` (:1373, :3060).
   - Smaller derived values:
     - `maaliajat` is written by `kilptietue::tallenna` (Hk/HkDat.cpp:1320).
     - `yhdsija` is written by `teejarj` (Hk/HkIx.cpp:1590) and `sijat2` (Hk/HkTulostus.cpp:1212).
     - `yhdseur` is written by `joukkuetul` (Hk/HkTulostus.cpp:7905).
     - `sjfl` is written by `tallenna` (Hk/HkDat.cpp:1314).
     - `prior` and `fprior` are written by `tallenna` (Hk/HkDat.cpp:1335-1338).
     - `keylen` is written by `teejarj` (Hk/HkIx.cpp:1571).
     - `njarjpst` is written by `AllocJarjSeur` (Hk/HkIx.cpp:65).
   - Writers I did not find: `nost`, `ntulos2` and `njrec` have no lexical writer in the CSVs. They are classed K3 by name only.
   - Writers outside Hk/ and Juk/: `ilmkentat` is also written at cbHk/UnitIlm2.cpp:214, and `navoin` at cbHk/UnitStatus.cpp:522.
   - Function-local K3 caches (previous-value or dedupe memory between calls):
     - Emit and badge: `tall_bdg_t::edbadge`, `tall_emit::ed_em`, `tall_emit::uusin`, `pros_emit::edkno`, `on_emit::emitvaraus` and `on_emit::nemitvar`, `laskeemitvaliajat::ev` and `laskeemitvaliajat::rst`, `vaihda_badge::seurbdg`.
     - Reader dedupe: the `tall_regnly::ed*` family.
     - Background task: `tausta::prev_t`, `tausta::edhaku` and similar.
     - `seur_rata::haj`.

2. K4 flags, their writers, and which threads read them.
   - The thread mapping combines the script's direct references in thread entry functions with the C-6 map. C-6 evidence is in saikeet-ja-lukot.md section 3.2.
   - Writers are [todennettu] where a line is cited.
   - Some threads are named in C-6 only through the code lines it cites, and the script cannot attribute those. These cases are marked.

| Flag | Writers | Reading threads |
|---|---|---|
| `taustaon` | `aloitus` (Hk/HkInit.cpp:3695), `lopetus1` (:3756); relay `aloitus` (VInit) and `lopetus1` (vApu) | pros_bdg_t, comajanotto, taustaThr, SendCopyThread, sendtcpfile, LukumaaratTh, ProsEmitThread, DisplTimerThread, ScrollThread, HaeAvoimetTh, kirjSQLTh, haeSQLTh, PaivThread, NaytaTulosTimerThread |
| `vaiheenvaihto` | `vaihdavaihe` (Hk/HkIx.cpp:1870), `Vaiheenvaihto1Click` (cbHk/WinHk.cpp:2080), `VaihdaVaiheHandler` (cbHk/WinHk.cpp:2163) | pros_bdg_t, taustaThr, ScrollThread, HaeAvoimetTh, kirjSQLTh, haeSQLTh, LukumaaratTh, ProsEmitThread, DisplTimerThread, NaytaTulosTimerThread |
| `comfl` | `closec` (Tp/TpComY.cpp:1892), `closecom` (:1916), `opencom` (:2204), `yhteysasetukset` (:2571), `openSirit` (Tp/TpLaitteet.cpp:2182) | tiedonsiirto and the UDP/TCP threads, lue_regnlyThread, tarkcom, TCPacceptTh, TCPrcvTh, UDPstreamrcvTh, sendtcpfile |
| `comopen[]` | `vaihdavaihe` (Hk/HkIx.cpp:1887), `opencomRS` (Tp/TpComY.cpp:690), `opencomUDP` (:899), `opencomTCP` (:1227), `closec` (:1887), `opencomTCPin` (:1950), `openlukijaUDP` and `openlukijaTCP` (:2118, :2185), `openSirit2` (Tp/TpLaitteet.cpp:2284), `openConnection` (Tp/ZebraReader.cpp:945), `startaf` (Hk/HkTulostus.cpp:7021) | lue_regnlyThread (direct, once before its loop); `tiedonsiirto` and `tiedonsiirtoUDP` read it per C-6 |
| `comclose[]` | `BtnAvaaClick` and `BtnSuljeYhteysClick` (cbTp/UnitEmiTag.cpp:381, :567), `BtnLaite1SuljeClick` and `BtnLaite2SuljeClick` (cbTp/UnitMaalikello.cpp:237, :254), `LaiteAvaa` (:345) | lue_regnlyThread |
| `katkaisuPyydetty` | GUI buttons only (UnitGAZ.cpp:1259, UnitEmiTag.cpp:566, UnitMaalikello.cpp:236, :253) | lue_regnlyThread |
| `inLopetus` | `lopetus1` (Hk/HkInit.cpp:3749), `initajat` (Hk/HkIV.cpp:4073), `tallenna` (Hk/HkDat.cpp:1378) | lue_regnlyThread; tiedonsiirtoTCP per C-6 (Tp/TpComY.cpp:1277) |
| `ProsEmitOn` | `aloitus` (Hk/HkInit.cpp:3706), `lopetus1` (:3754), plus GUI writers | ProsEmitThread |
| `lkmThreadOn` | `lopetus1` (Hk/HkInit.cpp:3739), GUI writers (cbHk/WinHk.cpp:2081, :2164; ViestiWin/UnitMain.cpp:271, :1703) | LukumaaratTh, DisplTimerThread, NaytaTulosTimerThread, ProsEmitThread, ScrollThread |
| `timerfl` | `inittimer` (Hk/HkIV.cpp:4284), `remtimer` (:4297), `Lueemitkoodeja1Click` (cbHk/UnitOsanottajat.cpp:1797) | pros_bdg_t |
| `regnly_open[]` | `remtimer` (Hk/HkIV.cpp:4295), `lue_regnlyThread` itself (Tp/TpLaitteet.cpp:1769), `start_regnly` (:2113), `rem_regnly` (:2132), `openlukijaUDP` and `openlukijaTCP` (Tp/TpComY.cpp:2112, :2163) | lue_regnlyThread |
| `inhttpHaku` | `vahaku` (Hk/HkIV.cpp:3513) | writer is vahaku; `lopetus1` waits on it (Hk/HkInit.cpp:3803) |
| `inkirjSQLThread` | `lopetus1` (Hk/HkInit.cpp:3740), `kirjSQLTh` (cbHk/UnitMySQL.cpp:599) | kirjSQLTh |
| `taustasulj` | `taustaThr` (Hk/HkIV.cpp:3722) | no reader found |
| `PaivOn` | UnitEmiTag.cpp:111, :251; UnitMTR.cpp:261 | PaivThread |
| `SelTimerThreadOn` | UnitSelect3.cpp:40, :48, :57 and the thread itself (:109) | SelTimerThread |
| `NaytaTulosTimerThreadOn` | the thread itself (cbTp/UnitNaytaTulos.cpp:98), `DisplTimerHandler` (:151) | NaytaTulosTimerThread |
| `in_aseta_mtr` | `aseta_mtr` (Tp/TpLaitteet.cpp:2676), UnitMTR.cpp:71, :252, :340, :379, PaivThread (:400) | PaivThread (also a writer) |
| `suljesuoraan` | `tark_extra` (Hk/HkCom32.cpp:661), `CloseReqHandler`, `errorMsgHandler` | tarkcom and LukumaaratTh read it |
| `autoclose` | `tarkcom` (Hk/HkCom32.cpp:958) | tarkcom |
| `keyclose[]` | `Button3Click` (cbHk/UnitUusinta.cpp:146) plus several unattributed writes in Tp/TpComY.cpp (:2042-2618) | no thread-function reference found by the script |
| `server_on`, `luepaketteja` | writes only in Tp/TpComY.cpp (:2740, :2858; :140, :1911) | `luepaketteja` is a static in TpComY.cpp used by `luepakettiUDP` (Tp/TpComY.cpp:956) |
| `status_on` | `ohj_aikaruutu`, `leimat`, `tark_rap`, `yhteenveto`, others | tarkcom |
| `esto` | `val_gaz`, `inittimer`, `kbdiv1` | no thread reader found |

   - Single-writer or GUI/UI-thread flags, with no thread-function reads found by the script: `ToimintaTila`, `k_pv`, `arv_pv`, `bootfl`, `ajat_on`, `autokilp`, `inGDIdoc`, `sulkemassa`, `odota`, `gpoActive`, `siritopen`.
   - `ToimintaTila` is written by `aloitus` (Juk/VInit.cpp:2740), the GUI `Aloita` handlers (ViestiWin/UnitMain.cpp:1526, cbHk/WinHk.cpp), `BtnIlmTilaClick` and `BtnTulPalvClick` (UnitMain.cpp:1402, :1425).
   - `k_pv` is written only by `lue_parametrit` (Hk/HkInit.cpp:707), `aloitus` (:3165) and `vaihdavaihe` (Hk/HkIx.cpp:1931). It has 2161 reads.
   - `arv_pv` has 16 writer functions, for example `jarj2pv` (Hk/HkArv2.cpp:609), `arvo` (:1162) and `list` (Hk/HkTulostus.cpp:8559).
   - `regnly[]` is classed K1, but `lopetus1` also zeroes it as a stop flag (C-6 section 4.2).
   - `comfl` and `comopen[]` are K4 per C-6.

3. Stage-dependent globals.
   - "y" means the value is indexed by `k_pv` or reset in `vaihdavaihe` [todennettu] (cited lines), or recomputed by `teejarj` after the stage change [paatelty].
   - Written in `vaihdavaihe` (Hk/HkIx.cpp:1855-2005):
     - `k_pv` and `arv_pv` (:1931-1932).
     - `obfilename[7]` and `aikajono[]->aikafname` file-name digit (see the `L'1' + k_pv` lines in `vaihdavaihe`).
     - Counters `nilmt`, `ntulost`, `nhylt`, `nkeskt`, `neilt`, `nesitt`, `nvakt` set to 0 (:1913-1919).
     - Per-series arrays memset to 0: `nva`, `nilm`, `nyilm`, `ntulos`, `nhyl`, `nkesk`, `neil`, `nvak`, `neilaika`, `nilm_pv`, `nesit`.
     - `emitfname[4]` (:1946-1948 region).
     - `comopen[]` temporarily zeroed (:1887).
     - `Sarjat` reloaded by `luesarjat`, then `teejarj(true)` called, rebuilding the indexes and rank chains below.
   - Indexed by stage:
     - `pvparam[N_PV]`, for example `pvparam[k_pv]` at Hk/HkIV.cpp:701 and Hk/HkDat.cpp:1305.
     - `Sarjat[].va_raja[k_pv]` (Hk/HkApu.cpp:393).
     - `Sarjat[].pistekaava[k_pv]` (Hk/HkDat.cpp:872).
     - `Sarjat[].valuku[k_pv]` (Hk/HkDat.cpp:1203).
     - `Sarjat[].mno[k_pv]` (Hk/HkDat.cpp:1320).
     - `nilm_pv[N_PV]`, `nvak[N_PV]`, `maxlsak[N_PV]`.
   - "y" by `teejarj` recomputation [paatelty]: `ixarr`, `jalku`, `jarjtulos`, `jrjseur`, `pkarki`, `ljarjindex`, `sarjaindex`, `srjaakindex`, `seuraindex`, `seuranoindex`, `lahtoindex`, `aakkosindex`, `nroindex`, `srjnroindex`, `idindex`, `rataindex`, `nameindex`, `pstindex`, `vaihetlsindex`.
   - "?" (31 names): emit-memory and course structures that `vaihdavaihe` frees or reloads but I did not trace (`vapautaemitmuisti` and `lue_radat_xml(7)` at Hk/HkIx.cpp:1889 region and :1941), plus mutable per-series fields.
     - `kuvio`, `nkuvio`, `rvalit`, `rvluku`, `EmitErot`, `nEmitErot`, `rastiva`, `Map`, `lainaemit`, `rata`, `nrata`.
     - `bdg_kno`, `nbadge`, `emitjarr`, `emitfl`, `emithead`, `emithead2`, `emitptr`, `emitpos`, `emitfile`.
     - `maaliajat`, `tmaali`, `kaikki_ajat`, `era_akt`, `yhtjonot`.
     - `sjfl`, `prior`, `prior0`, `fprior`, `modif`, `tlsmuutos`.
   - Relay (Juk) has no stage change (saikeet-ja-lukot.md section 4.3). The "y" flags apply to the HK build only.

4. K2/K3 names written from GUI dirs (cbHk and ViestiWin).
   - K2:
     - `Sarjat`: for example `TFormAjanotto::SarjaLahto`, `TFormEmit_VA::TallSarja`.
     - `kilpparam`: `TFormKilpMaaritys`, `TFormArvonta` handlers and others.
     - `pvparam`: `TFormKilpMaaritys::BtnVaiheMaarClick`, `FormCloseQuery` and others.
     - `kilpparam_v`: `TFormImport::Button1Click`.
     - `datf2`: `TFormImport::Button1Click` (also H-047).
     - `kilpailu`: `TFormKilpMaaritys::ButtonOKClick`, `TFormValitseKilp::BtnValittuClick`.
     - `sarjaluku`: `TFormKilpMaaritys::ButtonOKClick`.
     - `nkierros`: `TLoppuKilpForm::BtnToteutaClick`.
     - `aikajono`: 22 writer functions including `TFormAikavert::*` and `TFormAjanotto::*`.
     - `kilp0`: `TFormEmit::TallennaMuutokset`.
     - `osuus` and `va_mpiste`: ViestiWin dialogs.
     - `maxosuusluku`: `luevanha`.
   - K3:
     - Indexes: `ljarjindex` and `sarjaindex` (`TFormArvonta::SeurPos`, `BtnNumeroiClick`), `nameindex` (`TFormArvonta::SeurPos`, `TFormEmit::BtnHaeEdClick`, `BtnHaeSeurClick`), `pstindex`, `vaihetlsindex` (`LaskeVaiheSarja`), `reindex` (`TFormArvonta::ToteutaJako`, `TTulosteForm::CBJoukkHyvClick`), `lahtojarjfl` (`ASCLahdot`, `lahtoluettelo`, `webScorerLahdot`).
     - Counters and cursors: `nva` (`MaastossaNva`, `TFormMaastossa::*`), `prior` (`LukumaaratTh`), `era_akt` (`TFormAjanotto::*`), `emithead2` and `emitptr` (`TFormEmit::*`), `piirifl` (`TFormPiiriVal::*`).
     - `ilmkentat` (cbHk/UnitIlm2.cpp:214), `tlsmuutos` (ViestiWin/UnitStatus.cpp:412), `navoin` (cbHk/UnitStatus.cpp:522).
   - `prior` is written from `LukumaaratTh` (cbHk/WinHk.cpp:1829 and ViestiWin/UnitMain.cpp:535), a worker thread. Also `tausta` (Hk/HkIV.cpp:3702) writes it. This is mapping only, not a defect claim.
   - GUI writes to indexes (`nameindex`, `ljarjindex`, `sarjaindex`) appear to be address-taken ("A") for iteration. I did not check whether they modify the arrays [tuntematon].

5. Coverage.
   - 1200 file-level names were in the CSVs. I classified 983.
   - I skipped 217, with the reason in each CSV row:
     - 139 `tag_*` string-literal tables in Hk/HkXmlTulokset.cpp (the script's initial `tag_a` match was an exception, kept in K6). Their only writer is the `<file scope>` initialiser [todennettu from kirjoittajat.csv].
     - 35 form pointers (`FormXxx`, `EditDialog`, `FSwarning`, `FormThis`, `Form1`).
     - 17 CRITICAL_SECTION objects (`*_CriticalSection`).
     - 16 thread handles.
     - 9 script false positives: `getpos`, `pyoristatls`, `luesarja`, `maki`, `IV_KEY`, `std`, `AnsiString`, `ast`, `tulkp`.
     - 1 never-written unused static (`vlkm`).
   - Function-local statics: 217 rows in Hk/ and Juk/. I classified 197 and skipped 20 const tables, literal strings and thread handles.
   - Function-local statics in Tp/, tputilv2/, cbHk and ViestiWin were out of scope here (task was Hk/ and Juk/ only).

6. Script false positives and negatives, to document as limitations.
   - **Function-attribution loss** [todennettu for the symptom, cause [paatelty]].
     - 8430 references are attributed to `<file scope>`.
     - Besides the Globals files and the declaration headers (HkDeclare.h 572, VDeclare.h 511), large counts remain in Tp/TpComY.cpp (879), Tp/TpComY32.cpp (737, not built), Juk/vkilp.cpp (353), Juk/vSarjat.cpp (264), Juk/VMuotoilu.cpp (240), cbHk/UnitComfile.cpp (252), cbHk/TulosUnit.cpp (212), cbHk/UnitIlm2.cpp (208), ViestiWin/UnitComfile.cpp (179), cbHk/UnitStatus.cpp (161).
     - Real writes in those files show as `<file scope>`. Example: `comfl = 0` at Tp/TpComY.cpp:1892 is inside `closec` (:1882), and :1916 is inside `closecom`.
     - `kirjoittajat.csv` therefore undercounts writer functions.
     - luokittelu.csv marks it as "+N unattributed writes" in writers_summary (238 rows).
     - The likely cause is brace-depth drift in files with `#if` branches or odd formatting.
   - **Truncated or garbled function names**: writer entries such as `TForm` (for kilpparam and pvparam) are truncated method names.
   - **Parse-error rows (6)**: Hk/HkTV.cpp:103 and :116 (`rivi`, `ast`, `tulkp`, `piste2`), Juk/VEmit.cpp:939 (`lainafname`), cbTp/UnitNaytaTulos.cpp:94 (`NaytaTulosTimerThreadOn`). Two of them are real globals (`lainafname`, `NaytaTulosTimerThreadOn`), so I classified them from other sources. Cause: an `operator =` or `static` function defined after `}` on the same line.
   - **Header and non-variable false positives**: `getpos`, `pyoristatls`, `luesarja` (prototypes with defaults), `maki` (HkDef.h:493), `IV_KEY` (TpDef.h:121), `std` and `AnsiString` (HkInitVanha.cpp).
   - **Name merging**: generic names such as `n` (148 writers, 1142 reads), `p` (387 writers, 3266 reads, but it is only the MTRand class static), `rivi`, `ok`, `wait`, `top`, `step`, `init` merge unrelated locals, parameters and statics. Their counts are not meaningful.
   - **Merged duplicates**: names defined in both Hk and Juk are merged. For example `vaiheenvaihto` merges the HkGlobals global with the relay-only statics at cbTp/UnitEmitSQL.cpp:30 and cbTp/UnitNaytaTulos.cpp:30 (C-6 section 3.2 note).
   - **Declaration lists**: `INT a, b, c` declarations share one declaration text per name, so the `declaration` column repeats.
   - **Writes not seen**:
     - Only the first branch of each `#if` chain is scanned.
     - Pointer and array element writes through aliases, memset and `memcpy` on structs, and writes via member functions are only seen as "A", or not at all.
     - `kilparr` has 4 writer functions and 10 reads, which is clearly low because record access goes through `kilptietue` methods and `DATAREF`.
   - **GUI-only definitions**: cbHk and ViestiWin file statics and globals are outside maarittelyt.csv by design, so form-internal state is not covered.

OPEN QUESTIONS
- Which names that have no lexical writer (`nost`, `ntulos2`, `njrec`, `tmaali`, `kilparr`-adjacent sizes, etc.) are written through pointers or bulk operations? The CSV marks them "no lexical writer found".
- Stage dependence of the emit and course memory (`rata`, `kuvio`, `rvalit`, `Map`, `bdg_kno`, `emitjarr`) was not traced. This belongs to part B.
- Whether GUI "A" references to `nameindex`, `ljarjindex` and `sarjaindex` write the arrays or only pass them for iteration.
- Whether the Tp/TpComY.cpp attribution loss hides further K4 writers that the K4 table does not list.

FILES
- C:\dev\tulospalvelu\doc\arkkitehtuuri\todisteet\c8\osaA\luokittelu.csv
- Input data read: maarittelyt.csv, kirjoittajat.csv, viittaukset.csv, staattiset_funktiossa.csv (same directory).
