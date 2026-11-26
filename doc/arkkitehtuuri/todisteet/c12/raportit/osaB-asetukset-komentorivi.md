# C-12 osa B / part B: run-time settings (config keys and command line)

Agent `architecture-analyst` (Sonnet), 2026-11-26, about 245 k tokens. The main session saved the
report from the agent's hand-back. Claims, evidence and labels are unchanged; wording and layout are
condensed. Static reading only. Paths are relative to `TPsource/V52/`.

Labels N-1..N-12 are provisional, not register IDs, and are not the part A or part C series.
Tags: [verified] = code read; [inferred] = partly traced; [unknown] = not established.
Programs: H = HkMaali, G = HkKisaWin, M = ViestiMaali, L = ViestiLuenta, W = ViestiWin.
Compared length n: `=` full literal, `<` prefix only, `>` includes the terminator (exact match).
Raw input: `../osaB0/vertailut.csv`.

## Summary

- **Two run-time parsers.**
  - `Hk/HkInit.cpp` `lue_parametrit` is compiled into H and G.
  - `Juk/VInit.cpp` `lue_parametrit` is compiled into M, L and W [verified].
- **The two `UnitCfg.cpp` files are not run-time parsers.**
  - `cbHk/UnitCfg.cpp` and `ViestiWin/UnitCfg.cpp` (`TFormCfg::TulkitseTiedot`) map cfg lines to the widgets of a config editor form.
  - A recognised line is marked `Tulkitut[i]=1` and copied into a widget.
  - `PaivitaIkkuna` writes lines back between `//CfgEditorAlku` and `//CfgEditorLoppu`.
  - Unknown lines pass through unchanged, with no warning [verified: `cbHk/UnitCfg.cpp:196-222, 880-1000, 2325-2352`].
  - "Console vs GUI drift" therefore means two things: `#ifdef _CONSOLE` branches inside the shared parsers, and text the editor writes that the run-time parser does not accept.
- **Command line.**
  - Only the two console programs feed argv to the parser (`HkConsole/HkMain.cpp:178,201`; `JukConsole/laskenta.cpp:98,108`).
  - argv uses the same keys as the file and is applied last, so it wins.
  - The GUIs call `aloitus(0,0,cfgflnm)`, so they never pass argv to the parser [verified].
- **Main defect candidates:**
  - `KATKKIER` writes out of bounds in VInit (N-1).
  - The editors write keys that the run-time parser rejects (N-2).
  - VInit `UDPVIIVEET=` drops its first value (N-3).
  - The relay window title is upcased and cut (N-4).
  - `cfgfilename[41]` can be overrun (N-5).
- **Shadowing candidates.** Most of the 239 are false alarms. Real unreachable code: the TDC, S3 and S4 branches and the indexed `PISTEETn=` form (section 3).

## 1. Key inventory

### Common rules

**Matching.**

- Matching is case-insensitive: HkInit uses `wmemcmpU`, VInit uses `wmemcmp` on an upcased `fldn`.
- `fldn[90]` holds the key, cut at the first blank and upcased.
- "Pre-copy" keys keep the rest of the line as typed:
  - HkInit: HTML*, LISTA=, SQL*, LOKI=, TARRA=, KOMENTO, TAULUOT, SIRITAR, SIRITDE, IKKUNAO.
  - VInit: HTML*, LISTA=, SQL*, LOKI=, TARRA=, KOMENTO only.
  - [verified: `Hk/HkInit.cpp:612-629`, `Juk/VInit.cpp:673-691`]

**Line handling.**

- A line ends at `;`.
- A line over 77 characters is cut silently (`ReadLine(line,78)`, `Hk/HkInit.cpp:350`).
- At most 199 lines are read; beyond that the parser prints "Liikaa parametreja" (`Hk/HkInit.cpp:374`, `Juk/VInit.cpp:476`) [verified].

**Defaults.** Mostly the initial value of a file-scope global, usually 0; these were not traced one by one [inferred]. Explicit defaults:

- `cfgfilename` = "LASKENTA.CFG" (`Hk/HkInit.cpp:59`, `Juk/VInit.cpp:74`)
- `setySize` = 50 (`:52`, `Juk/VInit.cpp:56`)
- `merkit` = 'G' (`:57`)
- `baudst[0]` = "9600,n,8,1" (`:61`)
- UDP delays 10/20/20 (`Tp/TpComY.cpp:63-65`)
- `monitoriVali` = 10000 (`:83`)
- `af_xmlva` = 'M', `aflstnm` = "AUTOFILE.LST", `SiritEventPort` = 50008, `SiritCmdPort` = 50007 (`Hk/HkInit.cpp:3079-3176`)
- `maxasakko` = -99999, `analoolisa` = 20 (`:3168-3170`)
- HkMaali sets its `kilpparam` start values in `HkConsole/HkMain.cpp:185-201`.

### 1a. `Hk/HkInit.cpp` `lue_parametrit` (H, G), about 230 keys

| Key (line, n) | Syntax, variable | Prog | Meaning |
|---|---|---|---|
| CFG (631, 3) | any key starting with "CFG" sets `cfgparam=1` and the line is skipped | HG | a missing cfg then gives an error; `CFG=name` counts only as argv[1] (571-572) |
| IKKUNAO[TS]= (634, 7) | text -> `ikkunaots` | HG | window title |
| IKKUNAT[=file] (641, 7) | -> `alkulayout`, default "ikkunat.xml" | HG | layout, used by the GUI |
| SALLISULJE (648, 10) | `poistaSulje=false` | HG | |
| KIRKAS (652, 5), VÄRIT= (656, 6) | colours | HG | |
| RIVIT=25 / RIVIT=50 (675/679, 8) | `setySize` | HG | only these two values |
| PÄIVÄ= / VAIHE= (703, 6) | digit -> `k_pv`; V -> `vaihevalinta=1` | HG | V uses `vaihevalinta.cfg` (289-325) |
| SQLDRIVER/HOST/DATABASE/USER/PASSWORD (713-742, 7), SQLSTART, SQLTALLENNUS (748/752) | `sqlparam.*` | HG | MySQL |
| JUOKSUILM (765), HENKILÖTIETOKANTA (769), SEURATIEDOSTO (780) / SEURATIETO (787) | | HG | |
| KUNTOSUUNN[=n] (796), KUNTOMAALI (806), KUNTOLÄHTÖ (810) | fitness modes | HG | |
| RASTIVATULOSTE (814), VAKANTTILUKU= (823), VAKANTTIALKU= (829, clamped), LISÄVAKANTTIALKU (842), EMITALKU= (849) | | HG | vacant numbers |
| POISSAMUKAAN, HYVHAVAITUT, VAHVISTAAINA[=E], LISÄÄEDELLE (855-870) | flags | HG | |
| TARKNO=EI/0, ON/1, KA/2 (874-885, `>`) | exact -> `tarknofl` | HG | |
| MUOTOILU (889), VAADITARK (901), LÄHETÄCR=EI (905), KORKEAT[=STAR] (909/911) | | HG | |
| LOKI (938), LISTA (943), MERKIT= (961), DESEROTIN= (969), ESTÄMUUTOS (974) | | HG | |
| HTML=, MHTML=, XML= (979), HTMLTIT (1008), HTMLOTS (1014), KOMENTO (1020) | `#ifdef AUTOFILE` | HG | auto output (H-149/H-150) |
| TCPTULOS= (1029), FILETCPn (1034, TCPSIIRTO), XMLSARJA= (1047) | | HG | |
| BOOT (1052), AUTO (1061, guarded), PUSKUROI, YHTEISLÄHTÖ, KONE=, SOTILAS, HYVKAIKKI, MONITORVÄLI=, MONITORI (1066-1101) | | HG | |
| COMn / YHTEYSn (1105-1160) | `yhteys_no`; `comtype`, `ipparam`, `portparam`, `comdir` | HG | connections (`tiedonsiirto.md`) |
| TCPAUTOALKU (1150), PORTBASE=, MAXYHTEYS= (clamped), VÄLITÄn, SULKUSALASANA=, NAKVIIVE=, UDPJARRU=, UDPVIIVEET=, TCPVIIVEETn=, BAUDn=, COMFILE (1165-1256) | | HG | |
| VARASERVER (1193) | `#ifdef SERVER` | H | |
| VAIKAVERT=, ILMTUNT, VA-AJAT/YHTEYSAJAT, ÄÄNI=, LISÄÄ=, KUNTO/SUORASYÖTTÖ, RTNMn, CPOWERn (1289-1333) | | HG | |
| TESTAUS (1395), COMTESTI (1400) | `testi` | HG | |
| DBGTOFILE (1405) | `DBGFILE` | G | |
| AJAT, LAJAT, PAKOTALAIKA(RAJA), LÄHTÖRYHMÄ=, TAUSTA-AIKA, AIKAKORJAUSn, LÄHKELLOn, LÄHAIKAn, LÄHECODEn, LÄHTIEDOSTOT, SYÖTTÖ, AIKAERO[n]=, AIKA_COM=, AIKA_ESTO=, KELLO_ESTO=, ESTÄNEG, ESTÄHAAMUT, ESTÄYLIM, REGNLYVIIVE=, REGNLYNO/KELLONO/COMETNO/TIMYNO/TDCNO/ASCNO, REGNLY[2]= (1411-1623) | | HG | timing sources |
| VERTRAJAT= (1422) | `AIKAVERT` | H | |
| NÄPPÄIN= (1469) | `_CONSOLE` | H | |
| ENNAKOI (1542) | `ENNAKOI` | H | |
| SALLITOISTO (1702), SIRIT*/ZEBRA* (1711-1783) | `LAJUNEN` | HG | RFID |
| TUNNKOODIT (1791) | | HG | |
| ALGE, COMET, RADIOEMIT, TIMY, FEIG, SIRIT, ZEBRA, IMPINJ, ARES, SW2000, ASC (1799-1881) | `regnly[rno]`, `port_regnly`, `maxero` | HG | outer prefix test and inner chain (section 3) |
| KELLOBAUD=, KELLOBITS=, LÄHTÖIKKUNA=, KUOLLUTAIKA=, UUSINAIKA, MUUTOSKYSELY=, VAVUOROT (1907-1942) | | HG | |
| JONOT, JONOPISTEn, LÄHDEPISTEET, EMITJONO=, MAXJONO=, TUNNJONO, KAKSITUNN, TUNNRAJAT=, PISTEET=, KIERROSVAn, VAINVÄLIAJAT, VAINALARAJAT (1959-2109) | | HG | queues and points |
| RATATUNNISTUS=EI (2116), TALL-LÄHD (2121), RACPISTEET (2186) | | HG | |
| SAKKO_YHT/COM/BAUD/LAJI (2133-2178) | `SAKKOCOM` | HG | penalty device |
| LAINALUETTELO (2191) | | HG | loan cards |
| EMIT (2210, `>`), EMLOKI, ESITARK, ESILUENTA, LAINAT, LEIMAT=E, EILÄHT=AVOIN, EMITAJAT, ELINKLÄHTÖ, LÄHTÖVERT=, KARTTA=, EMITKELLO, LUKIJAn, EMITAGLISÄ, EMITAGn, EMITANAL (2356, effect only with `!_CONSOLE`), ECAIKA | `EMITLEIMAT && MAALI` | HG | emit |
| ETSQLn, ETGPRSn, ETHAKUVÄLI=, ETDATE=, ETTIME=, ETHOST= (2369-2446) | `EMITHTTP` | HG | emiTag |
| MTRn, LUKUTOISTO, EMITVASIVU, TARKRAPOHITA, MAALILEIMASIN=E, AUTOKILP, COMAUTORAP, AUTORAP, SIVUJAKO, HTMLSÄILNIMI, ORIGEMVABADGE, ESTÄ/SALLIEMITTOISTO, AIKALUKIJA, SJBOX, KUORMITUSTESTI, LUESUORAAN, AUTOSARJA, AUTORATA, ANNASEUR (2455-2690) | | HG | |
| VALONÄYTTÖ=, VALOT= (LAJUNEN), LÄHEMITn, LÄHEMVAn, EMVALOKI, BIBSIIRTOALUE=, RAPORTTIHAK=, DEVELOPER (2697-2761) | | HG | |
| not built | SIIMPORT, GATE, TV_*, TAULU_*, GAZ*, SPORTIDENT, FISKOODI, PISTEPV=, all MAKI keys (1645-1697) | none / ? | |

### 1b. `Juk/VInit.cpp` `lue_parametrit` (M, L, W), about 200 keys

- **Shared with HkInit.** Most keys are shared with HkInit and set the same variables. The relay differences:
  - `ET*`, `TAULU_*`, `SIRIT*` and `AIKA_*` are only in M and W.
- **Relay-only keys:**
  - Removed keys: SAKOT, AMPUSAKOT, HIIHTO, PARTIO and ROGAI print "poistettu käytöstä" ("removed") and do nothing (748-778).
  - Others: MAALILUKIJA (760, sets `lisaa_edelle`), SISÄÄNLUENTA, LUENTALOKI, KIERROKSET, VUOROKAUDET, TARKNO=d (791), KIELI=, OTSIKOT, RATAVAL=, SÄILHYL (977, n=7), VÄLIAJAT, TCPHAJONTA, KATKKIERn (1137), CHECKST, NÄYTÄHAJONTA=EI, YHTAIKAJONOn=, SAMAPISTE, OSUUSOLETUS=, OSUUSRAJA=, PISTEET= (no index), PISTE[n]=/JONOPISTE, VUOROMAALI, AIKAERO= (1467, n=8), AIKA_MASK=, VAROITUSKYNNYS=, JÄLKISYÖTTÖ, MAXEMVALAH, EMITTO, VAPAAEMIT=, JOUSTOVIESTI, JOUSTOPAKOTA, LUENTATESTI, SALLIUUSINTALUKU, EMITLUKU.
- **L only:** TARKAVOIMET (2152), SALLIEMITENSIN (2156).
- **W only** (`#ifndef _CONSOLE`): IKKUNAT, KUULUTTAJA, VALONÄYTTÖ= (693-704), SQL* (802-841), EMITANALYYSI (1725), RASTIVATULOSTE (1732), KARTTA= (1738).
- **M only:** NÄPPÄIN= (1248), ENNAKOI (1462), ESITA (2112).
- **M and W:** TAULU_COM=/BAUD= (1009, 1027), AIKA_COM=/ESTO= (1473, 1483), SIRIT* (1545-1587), LAINALUETTELO (1703), ET* (1853-1926).
- **Not built:** PROFILOI (1145), JOUKKUEEMIT (1715), VARASERVER (1161).

### 1c, 1d. `cbHk/UnitCfg.cpp` (G) and `ViestiWin/UnitCfg.cpp` (W) `TulkitseTiedot`

- **Mappings.** The two editors have 179 and 151 comparisons, about 142 and 122 different literals. Each is a widget mapping: `if (tyhjenna) reset widget else if (match) {Tulkitut[i]=1; set widget; continue;}` [verified: `cbHk/UnitCfg.cpp:1005-1075`].
- **Commented-out comparisons.** `cbHk/UnitCfg.cpp:2364-2513` is a block comment with 150 old run-time style comparisons. Those keys remain plain text in the editor.

## 2. Console vs GUI drift

### 2.1 HK

**Product differences inside the shared parser** [verified]:

- `NÄPPÄIN=` is console only (`:1469`).
- `DBGTOFILE` is GUI only (`:1405`).
- `EMITANAL` has an effect only in the GUI (`:2356-2362`); the console accepts it silently.
- `VARASERVER` is H only.

**Keys the editor (G) writes that the run-time parser rejects** [verified]:

| Key | Editor | Run-time |
|---|---|---|
| AUTOJAKELU= | read and written (`cbHk/UnitCfg.cpp:801`, `:1332`) | no handler in HkInit, so the parser reports "Tuntematon parametri" ("unknown parameter") |
| AIKACOM= | written (`:501`); reads `AIKA_COM=` (`:1690`) | accepts `AIKA_COM=` (`Hk/HkInit.cpp:1558`), so the round trip renames the key to one it rejects |
| TAKAA-AJO, NOLLAKÄRKI | written by check boxes (`:809`, `:813`) | handlers commented out (`Hk/HkInit.cpp:684-690`, `:757`); the editor does not read them back |

**Other differences:**

- GUI-only literals: AUTOJAKELU, JONOT=, VAADIAIKA. VAADIAIKA still matches the run-time `VAADIA` (n=6).
- 157 run-time keys are not interpreted by the editor (SQL*, SIRIT*, ZEBRA*, ET*, ...). They pass through unchanged, which is intended.
- Compared lengths differ: KAKSITUNN 9 vs 7, LÄHTIEDOSTOT 8 vs 12, SAKKO_LAJI= 11 vs 10. No behaviour difference [verified].

### 2.2 Relay

**Keys active only in the GUI inside VInit.**

- IKKUNAT, KUULUTTAJA, VALONÄYTTÖ=, SQL*, EMITANALYYSI, RASTIVATULOSTE and KARTTA= (`Juk/VInit.cpp:693-704, 802-841, 1725-1738`) are W only.
- M and L report them as unknown, while the HK console accepts IKKUNAT, SQL*, KARTTA= and VALONÄYTTÖ=. This is an inconsistency between HK and relay [verified].

**Keys the W editor writes that VInit rejects** [verified]:

| Editor key | Editor line | VInit |
|---|---|---|
| AIKAERO1=, AIKAERO2= | `ViestiWin/UnitCfg.cpp:397, 412` | only `AIKAERO=` (n=8, `Juk/VInit.cpp:1467`) |
| EMITALKU= | `:599` | none |
| EMITJONO= | `:627` | none |
| EMITAJAT[=n] | `:632-634` | none |
| SUORASYÖTTÖ | `:732` | none |
| AUTOJAKELU= | `:703` | none |
| AIKACOM= | `:429` | accepts `AIKA_COM=` |

**Other editor issues:**

- **Device types.** The editor maps ARES, ASC, TDC, RADIOEMIT, SW2000, ALGET, S3, S4, ALGE4 and ALGE= (`:1515-1561`). VInit knows only ALGE, COMET, TIMY, FEIG, IMPINJ, SIRIT and ZEBRA. Whether the form can write the extra names is [unknown].
- **KILPKDI.** It appears only in the editor's pre-copy list (`:832`); no handler exists [verified].
- **SÄILHYL.** The editor matches it exactly (`>`, `:948`), but VInit uses n=7 (`:977`). The editor therefore ignores "SÄILHYL=E".
- **Unmapped keys.** 113 VInit keys are not mapped by the editor; they pass through, which is intended.

### 2.3 Same key, different behaviour in HK and relay [verified]

| Key | HkInit | VInit |
|---|---|---|
| ILMTUNT | n=8 `>` exact (`:1297`) | n=6 prefix (`:1676`) |
| UDPVIIVEET= | n=11, value at `fldn+11` (`:1212`) | n=10, value at `fldn+10` (`:1168`), N-3 |
| AIKAKORJAUSn | `* SEK / 100` (`:1464`) | `* SEK` (`:1693`); unit differs, intent [unknown] |
| TESTAUS | n=7 | n=5 |
| REGNLY | n=5 | n=6 |
| TARKNO | EI/ON/KA or 0/1/2, exact | `tarkfl = fldn[7]-'0'` (`:791`) |
| JONOT | index allowed | `JONOT=` only, index 0 |
| SIRITEVENTPORT/CMDPORT/POLL | index suffix | n includes `=`, index 0 only (`:1575-1587`) |
| ESTÄYLIM | optional `K` | no `K` |
| AIKAERO | `AIKAERO=` and `AIKAEROn=` | `AIKAERO=` only |
| ALGE | plain `ALGE` sets nothing | plain `ALGE` -> `regnly[0]=3` (`:1633-1634`) |

## 3. Prefix shadowing and lengths

**Refuted candidates.**

- **Pre-copy list.** HTML, SQL, TAULUOT, SIRITAR and SIRITDE are only in the pre-copy list (`Hk/HkInit.cpp:614-619`; `Juk/VInit.cpp:677-680`) [verified].
- **Guards.**
  - `AUTO` is guarded by `(!fldn[4] || fldn[4]=='=')` (`Hk/HkInit.cpp:1061`; `cbHk/UnitCfg.cpp:1213`; `ViestiWin/UnitCfg.cpp:1002`).
  - `COM` and `YHTEYS` require `fldn[pos] <= '='` and `yhteys_no` (`:1105-1107`; `Juk/VInit.cpp:1051-1053`). COMFILE, COMTESTI, COMETNO, COMET, COMAUTORAP and YHTEYSAJAT therefore remain reachable.
- **Order or nesting.**
  - TUOMARI / TUOMARILUKU= (MAKI, not built).
  - LUESUORAAN (`:2673`) / LUESUORAAN= (`:2676`).
  - "=", "=E" and "=K" are `wcsstr` calls, not key comparisons.
  - EMIT (n=5 `>`) is an exact match.
  - Longer keys come first: SQL*, KUNTOSUUNN, SEURATIEDOSTO/SEURATIETO, EMITAGLISÄ=0 before EMITAGLISÄ before EMITAG, LAINALUETTELO before LAINAT.

**Confirmed unreachable code** [verified]:

- **TDC, S3, S4.**
  - TDC (`Hk/HkInit.cpp:1828`) and S3/S4 (`:1832-1837`) are not covered by the outer condition (`:1799-1809`), so "TDC", "S3" and "S4" are reported unknown.
  - The editors have the same dead branches (`ViestiWin/UnitCfg.cpp:1515-1547`, `cbHk/UnitCfg.cpp:1732-1772`).
- **`PISTEETn=`.** The comparison is `PISTEET=` with n=8 (`:2027`), and the index code starts at the `=`. The `else na = ny` branch is therefore dead.
- **`LÄHDEPISTEET`** appears twice (`:1990`, `:2660`); the second copy is never reached.
- **`ILMTUNT` (HK)** is an exact match: `ILMTUNT=KYLLÄ` is ignored silently and `ILMTUNT=EI` sets 0.

**Short compared lengths** [verified]. In each case n is shorter than the literal, so a longer key with that prefix also matches.

- **HkInit:** KIRKAS 5/6, SQL* 7, KUNTO 5, UUSINAIKA 6/9, VAVUOROT 5/8, RACPISTEET 5, RATATUNNISTUS=EI 15/16, LAINAT, EMLOKI, ESITARK, ESILUENTA and EMITKELLO all 5, EMITANAL 7, TARKRAPOHITA 10/12, SIRITARRIVE and SIRITDEPART 7. No other key is hidden, because longer keys are tested first; unknown keys with the same prefix are accepted silently.
- **VInit:** TESTAUS 5/7, ILMTUNT 6/7, AIKAKORJAUS 10/11, UDPVIIVEET= 10/11 (a real bug, N-3), KATKKIER 6/8.

**`>` cases** are deliberate exact matches: TARKNO=* (`:874-885`), ILMTUNT (`:1297`), EMIT (`:2210`, `Juk/VInit.cpp:1720`, both editors), SÄILHYL (`ViestiWin/UnitCfg.cpp:948`), NUL (`:2900`, not built).

## 4. Command line

| Program | argv read | Accepted | Same keys | Precedence | Default file |
|---|---|---|---|---|---|
| H | `wmain` -> `aloitus(argc,argv,NULL)` (`HkConsole/HkMain.cpp:178,201`) -> `lue_parametrit` (`Hk/HkInit.cpp:3176`) | any key; `CFG=name` only as argv[1] (`:566-572`, cut to 40 characters and upcased) | yes | 3 passes: `peruscfg.cfg`, then `cfgfilename`, then argv; the last one wins | `LASKENTA.CFG` and `peruscfg.cfg` in the current directory (`:59`, `:336-338`) |
| M, L | `wmain` -> `aloitus(argc,argv,0)` (`JukConsole/laskenta.cpp:98,108`) -> `Juk/VInit.cpp:2570` | same | yes | same | same (`Juk/VInit.cpp:74`, `:436-440`) |
| G | no argv use (no ParamStr/_argc/GetCommandLine; `RADStudio10/HkKisaWin.cpp:131`) | none | n/a | n/a | `Init.cfg` in `InitDir` (`cbHk/WinHk.cpp:189, 1645`); `CFG=` or the competition list gives `cfgflnm`; `aloitus(0,0,cfgflnm)` (`:279`); the directory comes from `SetCurrentDir` (`:272`) |
| W | only `_wargv[1]` as `INIT=file` (`ViestiWin/UnitMain.cpp:1527-1533`) | INIT= | no (`Init.cfg` keys ILM, TULOSP, AVAA, CFG=) (`:1456-1490`) | `lueInit`, then `aloitus(0,0,cfgflnm)` (`:220`) | `InitDir\Init.cfg` |

**`Init.cfg` keys.** HkKisaWin reads ILM, TULOSP, KUNT, LUETT, AVAA and CFG= (`cbHk/WinHk.cpp:1641-1680`). ViestiWin reads ILM, TULOSP, AVAA and CFG=. Both match on a prefix of 3-6 characters.

**Other cfg files.** `vaihevalinta.cfg` (`Hk/HkInit.cpp:293, 317`) and `ilmoitt.cfg` (`cbHk/UnitAvausMaar.cpp:70`, `UnitKilpMaaritys.cpp:599`). The editor's file dialogs default to "laskenta.cfg" (`:117`).

**Messages and quirks.**

- For an unknown key on the command line, the HkInit message still says "tiedostossa" ("in file") (`Hk/HkInit.cpp:2765-2767`). VInit says "komentorivillä" ("on the command line") (`Juk/VInit.cpp:2186-2188`).
- A `CFG=` argument after argv[1] is ignored, except that `cfgparam=1` is set [verified].

## 5. Mandatory vs optional, invalid values

**Missing files.**

- No key is mandatory [verified].
- A missing `peruscfg.cfg` is silent. A missing `LASKENTA.CFG` is also silent, unless a key starting with "CFG" was seen: then "Konfiguraatiotiedostoa X ei löydy" ("configuration file X not found") is printed and start-up continues (`Hk/HkInit.cpp:340-345`, `Juk/VInit.cpp:441-446`).
- The data files themselves are covered in part C and in `riippuvuudet.md` 19.6.

**Invalid values.**

- An unknown key gives `writeerror_w(msg,0)` and parsing continues (`Hk/HkInit.cpp:2765-2767`, `Juk/VInit.cpp:2186-2190`). The GUI version waits for the user (`cbTp/cbTpApu.cpp:467-478`); the console version was [not traced].
- Numbers are read with `_wtoi`, so garbage becomes 0 with no error.
- Values are clamped for MAXYHTEYS, VAKANTTIALKU, ZEBRAGPI, ZEBRADEPART, UDPVIIVEET (only values over 3 are accepted) and REGNLY (`od_regnly`, `maxero`).
- `aloitus` aborts when the VAKANTTILUKU value is too large (`Hk/HkInit.cpp:3203-3205`) or when reading the classes fails (`:3254-3255`); the program then exits [verified].
- The relay prints a notice for removed keys (`Juk/VInit.cpp:748-778`).
- Over 199 lines: "Liikaa parametreja" ("too many parameters").

## 6. Defect candidates

- **N-1 (medium): `KATKKIER` writes outside the array** [verified]. `Juk/VInit.cpp:1137-1141`:
  - `ny = fldn[wcslen(fldn)-1] - L'1'` is followed by `if (ny >= 0 || ny < MAX_LAHPORTTI)`, which is always true.
  - `katkkierto[MAX_LAHPORTTI=32]` (`Juk/VDeclare.h:582`, `VDef.h:43`) is written at index 33 for a key without a digit and at -1 for a trailing 0. Index validity beyond that is [inferred].
- **N-2 (medium, drift): the editors write keys the run-time parser rejects** (2.1, 2.2) [verified].
  - HK: AIKACOM=, AUTOJAKELU=, TAKAA-AJO, NOLLAKÄRKI.
  - Relay: AIKACOM=, AUTOJAKELU=, AIKAERO1/2=, EMITALKU=, EMITJONO=, EMITAJAT, SUORASYÖTTÖ.
  - After the form is used, these settings do nothing and a warning is shown at start-up.
- **N-3 (low-medium): `UDPVIIVEET=` first value dropped in the relay.** `Juk/VInit.cpp:1168-1171`: n=10 and `wcstok(fldn+10, ...)`, so the first token is "=40" and `_wtoi("=40")` is 0. `UDPviive_lah` is not set, because a value over 3 is required. HkInit starts at `fldn+11` (`:1212-1216`) [verified; the `_wtoi` behaviour is standard, inferred].
- **N-4 (low): relay title upcased and cut** [verified code].
  - `Juk/VInit.cpp:677-691`, `706-710`: IKKUNAO is missing from the pre-copy list, so "IKKUNAOTS=Viesti 2026" becomes "VIESTI". HkInit includes it (`:617-619`); the editor also keeps it (`ViestiWin/UnitCfg.cpp:833`).
  - In both parsers the same upcasing and cutting applies to text values of keys not in the list: HENKILÖTIETOKANTA=, SEURATIEDOSTO=, KARTTA=, MUOTOILU=, RAPORTTIHAK=, SULKUSALASANA= (`Hk/HkInit.cpp:621-628`).
  - How the upcased password is compared is [unknown].
- **N-5 (low-medium): `cfgfilename[41]` overrun** [verified]. `Juk/VInit.cpp:74`, `652`: `wcsncpy(cfgfilename, cfgflname, 120)` copies into 41 wide characters. A cfg name over 40 characters from the GUI (`Init.cfg` or the competition list) overwrites the neighbouring statics. HkInit has 201 characters (`Hk/HkInit.cpp:59`).
- **N-6 (low-medium): SIRIT, ZEBRA, FEIG and IMPINJ in ViestiLuenta select ALGE3** [verified]. `Juk/VInit.cpp:1594-1634`: LAJUNEN is not defined for L, so the default `regnly[0]=3` applies.
- **N-7 (low): dead branches** TDC, S3, S4 and `PISTEETn=` (section 3).
- **N-8 (low): over-reads** [verified code; effects inferred].
  - `Juk/VInit.cpp:673` `wmemcpy(fldn, fld[ncl], 7)` reads 7 characters from short strings.
  - `wcsncpy(fldn, argv[1], 44)` (`Hk/HkInit.cpp:566-568`, `Juk/VInit.cpp:656-658`) leaves `fldn` unterminated for an argument of 44 or more characters, and then `upcasewstr(fldn)` runs on it.
- **N-9 (low): lower-case HTMLTIT, HTMLOTS and SQL* fail in VInit** [verified]. `Juk/VInit.cpp:677-691` copies characters 5-6 raw and compares case-sensitively. HTMLTIT also requires the full "HTMLTITLE=" (`:931-933`).
- **N-10 (low): relay `TARKNO=ON` gives 31.** `Juk/VInit.cpp:791-793` [verified].
- **N-11 (low): the editor ignores "SÄILHYL=E"** (`ViestiWin/UnitCfg.cpp:948` vs `:977-980`) [verified].
- **N-12 (cosmetic or not built).**
  - The HkInit message says "tiedostossa" for argv (`:2765-2767`).
  - `strstr` on a wide string at `:1664` (MAKI) and `_wtoi(fldn+3)` instead of +10 at `:2475-2480` (SPORTIDENT). Neither is compiled in the five products.

## 7. Not covered

- **Meanings and defaults.** Not traced for every key; about 80 minor keys are marked "see code".
- **Partial read of HkInit.** HkInit 760-1000 and 2100-2760 were read in filtered form, so some `else` chains were inferred from the CSV.
- **Relay device list.** The relay editor's device list (`ViestiWin/UnitCfg.cpp:387`) was not read, so whether it can write ARES/ASC/TDC is [unknown].
- **Console start-up.** The console `writeerror_w` wait behaviour and cfg encoding detection were not traced.
- **Parser placement.** Which parser each GUI product compiles was taken from the CSV `act_` columns; the `.cbproj` files were not opened.
- **Ä/Ö in the CSV.** These show as replacement characters, so key identity was checked from the source with `iconv` where it mattered.
