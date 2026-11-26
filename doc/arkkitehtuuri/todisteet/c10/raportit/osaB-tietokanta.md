# C-10 osa B / part B: database dependency (MySQL via dbExpress), Q5

Agent `architecture-analyst` (Sonnet), 2026-11-26. Report saved by the main session from the
agent's hand-back (claims, evidence and labels unchanged; wording and layout condensed). Paths relative to `TPsource/V52/`
unless they start with `doc/`, `docs-site/`, `.github/` or `TPexe/`. Tags [verified],
[inferred], [unknown]. Nothing built or run, no database contacted; installed C++Builder 37.0
files read read-only, version resources via `Get-Item(...).VersionInfo`. Candidate labels
D1-D13 are the agent's; register IDs are assigned in `riippuvuudet.md`.

## Summary

- Live database code only in HkKisaWin and ViestiWin: three VCL forms with TSQLConnection/TSQLDataSet: `cbHk/UnitMySQL.*` (HK), `ViestiWin/UnitMySQL.*` (relay), `cbTp/UnitEmitSQL.*` (shared, emit reader). Console programs have no live DB code; the `mysql_*` block in `Juk/vdat.cpp` is dead. [verified]
- Role: optional parallel write target (mirror). KILP.DAT and the in-memory array stay master. SQL is written from the in-memory record by `kirjSQLrivi`. One explicit read-back: import case 6 (`lue_SQL`) overwrites KILP.DAT records from SQL, user-triggered. `UnitEmitSQL` is a read-only client of the external `tags` table (emiTag server).
- Table layout: no CREATE in code; layout in `cbHk/kilpsql.txt` (HK: `osanotot`, `osottopv`, `tulos`) and `ViestiWin/vkilpsql.txt` (relay: `vosanotot`, `osuudet`, `vtulos`); all InnoDB `DEFAULT CHARSET=latin1`; plain INSERT/UPDATE/SELECT/DELETE, never REPLACE.
- Connection parameters rebuilt in code on every open, not from an ini; `dbxkilp.ini` only in commented-out code. Password plain text in cfg/command line, shown unmasked, default `hkkisawin`. No user-configurable TLS. Charset not set; tables latin1.
- Packaging gap: GUI exes link `DBXMySQLDriver370.bpl` statically; the release collector copies only static imports, so `dbxmys.dll`, `libmysql.dll`, `dbxdrivers.ini` are not collected [inferred].
- Essentially no error handling around SQL; several UPDATE statements malformed (section 7).
- Versions: installed `dbxmys.dll` 37.0.60542.8024 "MySQL 5.x.xx driver"; manual: 32-bit MySQL 5.1 components, also work with 5.5; no client library in repo or Studio install → version unknown.

## 1. Inventory per product

| Unit | HkKisaWin | ViestiWin | Content |
|---|---|---|---|
| `cbHk/UnitMySQL.cpp/.h/.dfm` | yes | no | HK write/read form [verified] |
| `ViestiWin/UnitMySQL.cpp/.h/.dfm` | no | yes | relay write/read form [verified] |
| `cbTp/UnitEmitSQL.cpp/.h/.dfm` | yes | yes | read-only client of external `tags` (`cbTp/UnitEmitSQL.cpp:96-307`) [verified] |
| `Hk/HkGlobals.cpp:635-667` | HkKisaWin, HkMaali (data only) | – | `sqlparam`, `sqlEparam` defaults [verified] |
| `Juk/VGlobals.cpp:520-553` | ViestiWin, ViestiMaali (data, `#ifdef EMITHTTP`) | – | same structs [verified] |
| `Hk/HkInit.cpp:713-755`, `Juk/VInit.cpp:802-845` | SQL key parsing | | section 3 |
| `Hk/HkDat.cpp:590-593`, `Juk/vkilp.cpp:239-248` | call `kirjKilpSQL(d)` | | write hook (HK `write(d, 1)` via `PUTREC`, `Hk/HkDat.cpp:615-621`) |

Console evidence: `HkConsole/HkCons0.cpp:141-143` empty `kirjKilpSQL` stub; relay call `Juk/vkilp.cpp:246-247` under `#ifndef _CONSOLE`; `datatallfl[1]` = 0 (`Hk/HkGlobals.cpp:597`, `Juk/VGlobals.cpp:485`); `Hk/HkInit.cpp:713-755` is not under `#ifndef _CONSOLE` (the `#endif` at 705 closes the 693 block), so HkMaali parses SQL keys but nothing uses them; relay keys `Juk/VInit.cpp:801-845` under `#ifndef _CONSOLE`. `Juk/vdat.cpp:38-60` `#ifdef SQLDATA` (`mysql_init`, `mysql_real_connect`, `mysql_close(*sql)`, empty `putrecsql`): `SQLDATA` defined nowhere; `mysql_close(*sql)` would not compile; dead. `odbc32.lib` in `VS/V/JukLuenta520.vcxproj:103`, no ODBC calls. dbExpress headers only in GUI units (`cbHk/UnitMySQL.h:27`, `cbTp/UnitEmitSQL.h:27`, `ViestiWin/UnitMySQL.h:27`). `CreateForm` lines commented out (`HkKisaWin.cpp:148-232`, `ViestiWin.cpp:144-183`); forms created lazily: `cbHk/WinHk.cpp:372-374`, `:1579`, `cbHk/UnitExport.cpp:518`, `cbHk/UnitImport.cpp:1423`, `ViestiWin/UnitMain.cpp:273`, `:1944`, emit form `cbHk/UnitAjanotto.cpp:1476`, `ViestiWin/UnitAjanotto.cpp:1445`. [verified]

## 2. Q5: table layout and field mapping

Manual: the files are installed into the competition data folder (`docs-site/docs/hkkisawin/hkluku12.md:42-44`).

HK (`cbHk/kilpsql.txt`):

| Table | Columns | Primary key |
|---|---|---|
| `osanotot` (`:6-33`) | kilpailu varchar(15) NOT NULL; tietue int; id int NOT NULL; lisno, intid int; wrkoodi varchar(20); sukunimi, etunimi varchar(30); arvo varchar(15); seura varchar(30); seuralyh, yhdistys, joukkue varchar(15); maa varchar(3); piiri, ilmlista, piste1-3 int; sarja varchar(12); sukup char(1); ikasarja, synt, alisarja, arvryhma int | (kilpailu, tietue) |
| `osottopv` (`:39-66`) | kilpailu, tietue, vaihe int default 1; sarja varchar(15); era, bib, badge, badge2 int; laina, laina2 char(1); rata, selitys varchar(20); pvpiste1-2 int; tlahto int; tlahtotime time; keskhyl varchar(10); ampsakot varchar(8); tasoitus, sakko int; gps, qual char(1); yhtaika int; yhtaikatime time; ysija int | (kilpailu, tietue, vaihe) |
| `tulos` (`:72-81`) | kilpailu, tietue, vaihe, piste int default 0; aika int; aikatime time; sija int | (kilpailu, tietue, vaihe, piste) |

Mapping (`cbHk/UnitMySQL.cpp:408-435`, `:369-396`, `:350-358`): `kilpailu` = `kilpparam.kilpkoodi`; `tietue` = KILP.DAT record number; `vaihe` = `ipv+1`; `piste` = `piste-1`; `id` = `kilpno`; `lisno` = `lisno[0]`; `intid` = `lisno[1]`; `sarja` = series name text; `piste1-3` = `pisteet[0..2]`; `badge/badge2` = `badge[0..1]`; `laina*` 'L' or empty; `pvpiste1-2` = `pvpisteet[0..1]`; `tlahto` = `tlahto + t0*TUNTI`; `ampsakot` = `asakkoStr`; `yhtaika` = `ytulos(ipv)`; `ysija` = `y_sija(ipv)`; `aika` = `vatulos + tt0*TUNTI` (`tt0 = t0` only for `piste==0`); `sija` = `vasija`; rows with `vatulos==0`, `piste>0` skipped (`:346-347`); `*time` = formatted time or `00:00:00` for `TMAALI0`. Statements: SELECT count `:69-79`, `:156`; `tulos` `:172`; `osottopv` `:197-203`; `osanotot` `:260-266`; existence `:459-462`, `:511-515`; INSERT `:350`, `:369`, `:408`; UPDATE `:492`, `:525`, `:574`; DELETE `:610`, `:615`, `:620`.

Relay (`ViestiWin/vkilpsql.txt`):

| Table | Columns | Primary key |
|---|---|---|
| `vosanotot` (`:6-23`) | kilpailu, tietue, id, sarja varchar(12), seura varchar(50), joukkue int, joukkid int, joukkuenimi varchar(15), maa varchar(3), piiri, ilmlista, piste1-3, era int | (kilpailu, tietue) |
| `osuudet` (`:29-53`) | kilpailu, tietue, osuus, lisno, sukunimi/etunimi varchar(30), arvo, seura, badge, badge2, laina, laina2, rata, selitys, tlahto, tlahtotime, lahtolaji, keskhyl, ampsakot, sakko, gps, ossija | (kilpailu, tietue, osuus) |
| `vtulos` (`:59-68`) | kilpailu, tietue, osuus, piste default 0, aika, aikatime, sija | (kilpailu, tietue, osuus, piste) |

`ViestiWin/UnitMySQL.cpp`: INSERT `:317`, `:337`, `:377`; UPDATE `:449`, `:483`, `:531`; DELETE `:557`, `:562`, `:567`; SELECT `:69-79`, `:176`, `:200-206`, `:270-276`. Mapping: `joukkue` = team number; `joukkuenimi` = `joukknimi`; `seura` = `Seura()`; `osuus` = `os+1`; `rata` = `Hajonta()`; `tlahto` = `ylahto + t0*TUNTI`; `gps` = `seuranta`; `piste` 0-based incl. start; rows with `va->aika == TMAALI0` skipped (`:314`). Relay manual has no schema text. [verified]

UnitEmitSQL: reads table `tags` (`cbTp/UnitEmitSQL.cpp:96`, `:104-107`, `:194-198`, `:227-232`), columns Y (device), E (time string), N (badge), C (channel), M (message number), B (type `'M'`), `time`; schema of the external emiTag server, no DDL in repo; rows → `tall_etulos`/`tall_rivi` (`:282-294`) [inferred, not traced]; no writes. [verified]

## 3. Role and enabling

Mirror, not master [verified]: `kirjSQLrivi` loads with `kilp.GETREC(d)` (`cbHk/UnitMySQL.cpp:455`) from `kilparr` (`Hk/HkDat.cpp:596-605`), inserts or updates by diff against `readSQLrivi` (`:561-566`); `PUTREC` writes `kilparr` then `write(d, i)` for every `datatallfl[i]`, default `{1,0}` (`Hk/HkDat.cpp:615-621`, `Hk/HkGlobals.cpp:597`).

Read-back: import case 6 `lue_SQL` (`cbHk/UnitImport.cpp:1066-1088`, `:1422-1432`; relay `ViestiWin/UnitImport.cpp:759-761`, `:1102-1108`) reads rows 1..9998 for the current `kilpkoodi`, writes missing rows as `kilpstatus=0xffff` (`:1074-1077`), calls `kilp.tallenna` or `addtall`; SQL write-back from that [unknown]. `readSQLrivi` otherwise only in the update diff and a hidden test button (`cbHk/UnitMySQL.cpp:629-635`). Manual `hkluku12.md:34-37`. Bulk write: export option 3 (`cbHk/UnitExport.cpp:422-452`, `:516-527`; `ViestiWin/UnitExport.cpp:412-428`, `:466-475`).

| Product | Keys | Defaults | Activation |
|---|---|---|---|
| HkKisaWin | `SQLDRIVER` (undocumented), `SQLHOST`, `SQLDATABASE`, `SQLUSER`, `SQLPASSWORD`, `SQLSTART`, `SQLTALLENNUS` (`Hk/HkInit.cpp:713-755`), first 7 chars matched, not upcased (`:614-619`); `LASKENTA.CFG` (`:59`) then command line (`:577`) | `Hk/HkGlobals.cpp:635-650`: driver MYSQL, localhost, db kilp, user kilp, password hkkisawin, port 3306, compressed/encrypted/autostart/sqltallennus false; manual `hkliite1.md:216-219` | `SQLSTART` connects at start (`cbHk/WinHk.cpp:372-374`); menu Tiedostot/MySQL (`:1579-1584`); "Jatkuva tiedonsiirto" sets `sqltallennus` (`cbHk/UnitMySQL.cpp:757-761`) |
| ViestiWin | same keys (`Juk/VInit.cpp:802-845`) | `Juk/VGlobals.cpp:520-535` | `ViestiWin/UnitMain.cpp:273-274`, `:1944` |
| UnitEmitSQL | `ETSQL...` under `EMITHTTP` (`Hk/HkInit.cpp:2369`; `Juk/VInit.cpp:1853`) | `sqlEparam` (`Hk/HkGlobals.cpp:652-663`): db ecb, user EmitRO, password emitag, port 53306; `autostart` never set | from timing form (`cbHk/UnitAjanotto.cpp:1476`); manual: not automatic (`liite_9...md:132-134`), SSH tunnel needed (`:127-130`) |
| console | no effect | | |

`cbHk/UnitCfg.cpp:2375-2381` lists the SQL keys [unknown purpose].

## 4. Connection

- Chain: `TSQLConnection` `DriverName 'MySQL'` (`cbHk/UnitMySQL.dfm:240-258`) → `DBXMySQLDriver370.bpl` (LinkPackageImports; `TPexe/Hk/V521/HkKisaWin.pdi:2`, `TPexe/Juk/V521/ViestiWin.pdi:1`) → `dbxmys.dll` loaded by name (`Data.DbxMySql.pas:98`; installed `dbxdrivers.ini` `[MYSQL]` 118-139) → `VendorLib=LIBMYSQL.dll` (Win32; `dbxdrivers.ini:127-128` under `C:\Users\Public\Documents\Embarcadero\Studio\dbExpress\37.0\`; `Data.DbxMySql.pas:100-102`). [verified]
- Code `Params->Clear()` then adds drivername, librarynameosx, vendorlibwin64, vendorlibosx, blobsize, database, hostname, localecode, Password, User_Name, compressed, encrypted (`cbHk/UnitMySQL.cpp:117-129`, relay same, `cbTp/UnitEmitSQL.cpp:131-144`). Dropped DFM values: ServerCharSet, ConnectTimeout (installed `dbxconnections.ini` 60), ErrorResourceFile. Win32 `vendorlib` not set → driver default `LIBMYSQL.dll` [inferred]. Port passed only by UnitEmitSQL (`:139`); `sqlparam.port` never used [verified]; non-default server port unreachable [unknown whether host:port works].
- Lookup (`Data.DBXCommon.pas:7119-7176`): exe directory ini files, then `HKCU`/`HKLM` `\Software\Embarcadero\BDS\37.0\dbExpress` (`:214-216`), then built-in defaults [verified]; manual warns another program's `dbxdrivers.ini` may override (`hkluku12.md:12-16`).
- `dbxkilp.ini`/`LoadParamsFromIniFile` only in commented code (`cbHk/UnitMySQL.cpp:95-113`); `LoadParamsOnConnect` not set (`Data.SqlExpr.pas:2453-2495`) [verified].
- Credentials plain text in cfg/command line (manual `hkliite1.md:218`), `wchar_t[50]` (`Hk/HkDef.h:1010-1011`); no `PasswordChar` in the three DFMs; defaults `Hk/HkGlobals.cpp:644`, `:659` [verified].
- `encrypted`/`compressed` always false, no keys; effect in `dbxmys.dll` [unknown] (strings show `mysql_options`, `mysql_real_connect`, no `mysql_ssl_set`).
- Charset: ServerCharSet not set; `SET NAMES` commented out (`ViestiWin/UnitMySQL.cpp:46-49`); tables latin1; characters outside latin1 may be lost [inferred].

## 5. Loss of service

- No `try`/`catch`/`OnError` in `cbHk/UnitMySQL.cpp`, `ViestiWin/UnitMySQL.cpp` [verified]. GUI thread → VCL exception dialog [inferred]; writer thread (`_beginthread(kirjSQLTh...)`, `cbHk/UnitMySQL.cpp:752`) uncaught → probably process end [inferred].
- `UnitEmitSQL` `catch(...)` only around field reads (`:111-119`, `:203-211`, `:253-299`); `Open()`, `ExecSQL` outside.
- Missing driver BPL → program does not start even without SQL use [inferred].
- Missing `dbxmys.dll`/client lib or unreachable server → `Open()` throws: HK `cbHk/UnitMySQL.cpp:46` after `Sleep(1000)` on the GUI thread (`:47`); relay `ViestiWin/UnitMySQL.cpp:43`; emit `cbTp/UnitEmitSQL.cpp:71`; with `SQLSTART` during start-up (`cbHk/WinHk.cpp:374`; relay ctor `ViestiWin/UnitMySQL.cpp:36-37`) [inferred].
- Release: collector reads static imports only (`collect-gui-runtime.ps1:101-140`); `dbxmys.dll`, `libmysql.dll`, `dbxdrivers.ini` not collected, none in the repo, though the manual says they come with the program (`hkluku12.md:12-14`) [inferred].
- Server drop not detected; `Connected` checked before each write (`cbHk/UnitMySQL.cpp:696`) [inferred].
- Queue/backup: H-014, H-043, H-100 (C-1, C-3). Relay thread lacks the `!vaiheenvaihto` test (`ViestiWin/UnitMySQL.cpp:601`) [verified].

## 6. Versions

- Manual: 32-bit MySQL 5.1 components, also at least 5.5 (`docs-site/docs/hkkisawin/hkluku12.md:17-18`; same in viestiwin). Dialect: `int(11)`, `time`, `ENGINE=InnoDB DEFAULT CHARSET=latin1`, `SELECT count(*)` (`cbHk/UnitMySQL.cpp:72`); no REPLACE. Newer servers (strict mode, default charset, auth plugin) [unknown].
- `bin\dbxmys.dll`, `Redist\win32\dbxmys.dll`: FileVersion 37.0.60542.8024, "Embarcadero dbExpress MySQL 5.x.xx driver", 265288 bytes; `DBXMySQLDriver370.bpl`, `dbexpress370.bpl` same FileVersion [verified].
- No `libmysql*`/`libmariadb*` in Studio tree, repo or `TPexe` → [unknown].
- `dbxmys.dll` strings include deprecated `mysql_escape_string` [verified: strings].
- Upgrade/replace [inferred]: user-supplied client library; FireDAC (`FireDACMySQLDriver370.bpl` installed) → replace TSQLConnection/TSQLDataSet in three forms and DFMs, package lists; `dbxdrivers.ini` lookup is a separate influence.

## 7. Candidates (agent labels)

| # | Sev | Mechanism and evidence | Products | Consequence | Certainty |
|---|---|---|---|---|---|
| D1 | high | Relay `vosanotot` UPDATE: every clause appends `",\r\n"`; last `era=...,` (`ViestiWin/UnitMySQL.cpp:544`) followed by `WHERE` (`:545`) → syntax error whenever the header differs (`kilp->vert0(kilp1)`, `:529`) | ViestiWin | team-header changes never reach SQL; exception (D5) | text [verified]; effect [inferred] |
| D2 | high | HK UPDATE: `sukup=`, `gps=`, `qual=` unquoted (`cbHk/UnitMySQL.cpp:593`, `:544`, `:545`); `salisarja=` nonexistent column (`:596`) | HkKisaWin | UPDATE fails when these fields change | text [verified]; effect [inferred] |
| D3 | medium | Relay `osuudet` UPDATE writes `badge=` for `lisno` (`ViestiWin/UnitMySQL.cpp:484`); `gps=` unquoted (`:501`); team name into int `joukkue` (`:538`) | ViestiWin | wrong data or failing statement | [verified] text |
| D4 | medium | strings pasted unescaped (`cbHk/UnitMySQL.cpp:417-424`, `ViestiWin/UnitMySQL.cpp:342-345`, `:382`, `cbTp/UnitEmitSQL.cpp:107`, `:232`) | GUI | `'` in a name breaks the statement → D5 | [verified]; effect [inferred] |
| D5 | medium | `kirjSQLrivi` increments `inSQLquery`, opens a transaction, commits/decrements only on success (`cbHk/UnitMySQL.cpp:463-476`; relay `:403-420`); writer loop requires `!inSQLquery` (`:696-697`), others wait 10 s and give up (`:450-453`) | GUI | after one failed statement the mirror stops until restart | [inferred] |
| D6 | medium | Relay `SQLSTART`+`SQLTALLENNUS`: ctor calls `Button1Click(FormMySQL)` (`ViestiWin/UnitMySQL.cpp:36-37`) while `FormMySQL` is NULL (`ViestiWin/UnitMain.cpp:273-274`); `kirjKilpSQL(0)` (`:63`) dereferences `FormMySQL->CBVarmista` (`:676`) | ViestiWin | access violation at start-up | [inferred] |
| D7 | medium | `deleteSQLrivi` has no caller; `kirjSQLrivi` returns early for deleted records (`cbHk/UnitMySQL.cpp:456-457`) | GUI | deleted competitors remain in SQL (overlaps H-014) | [verified] |
| D8 | medium | `Sleep(1000)` and dialogs on the GUI thread; no connect timeout (DFM 60 dropped) | GUI | freeze while server unreachable | [inferred] |
| D9 | low | `kilpkoodi` `wchar_t[25]` (`Hk/HkDef.h:811`, `Juk/VDef.h:358`) vs `varchar(15)` (`kilpsql.txt:7`); other narrow columns | GUI | truncation or error by SQL mode | [inferred] |
| D10 | low | `sqlparam.port` ignored; no keys for encrypted/compressed | GUI | non-default port / TLS-only server unusable | [verified]; effect [inferred] |
| D11 | low | `lue_SQL` capped `i < 9999` (`cbHk/UnitImport.cpp:1070`) | HkKisaWin | large competitions partly read back | [verified] |
| D12 | low | password unmasked, documented default, plain in cfg/command line | GUI | credentials visible | [verified] |
| D13 | low | driver files not collected by release script | release ZIP | SQL not usable out of the box | [inferred] |

Open question candidates: (1) do `Encrypted`/`Compressed`/`Port` have effect in `dbxmys.dll`; (2) which client library and server versions are used in practice; (3) does an uncaught exception in `kirjSQLTh` end the process (manual test, H-014); (4) what `vert0` compares (D1); (5) does `tallenna` in `lue_SQL` write back to SQL; (6) `UnitCfg.cpp:2375-2381` handling of SQL keys; (7) add the SQL schema to `tiedostomuodot.md` as the answer to Q5.
