# C-13 part D: compatibility questions for the fix plan

Scope: Q-A..Q-G from the C-13 main session. Static source reading only; nothing built
or run. Paths relative to `TPsource/V52/`. Marks: [verified] read in code,
[inferred] reasoned from code, [unknown] not decidable here. "Unfixed peer" means
an unfixed program version on the same network or the same disk files.

## Summary

| Q | Finding | Wire change needed? | File change? | Main constraint for a bounded fix |
|---|---|---|---|---|
| A (H-025, H-048) | relay `combufsize` = record stride of COMFILE.DAT (and `__FCOMFILE__.TMP`, memory queue); not on the wire, **but** `class_len[FILESEND] = combufsize - 14` is | no, if FILESEND length is frozen | COMFILE record length +14 | keep `class_len[FILESEND]` at its old value; an old COMFILE is rejected by the existing header check (prompt), not misread |
| B (H-041) | KILPBUFSIZE is used only for stack buffers; never a record length, offset or packet length | no | no | stack cost about 1.2-2 KB per frame; reserve 1 MB (HkMaali) / 4 MB (HkKisaWin) |
| C (H-026) | packet already ACKed and stored in a fixed-stride slot before dispatch; HK skips unknown classes | no | no | fix = advance `inbens` (as HK); do not NAK |
| D (H-071) | FILESEND is a documented feature (LÄHTIEDOSTOT, /K); receiver accepts unconditionally | no | no | restricting to the working directory breaks documented /K and the undocumented parent-directory case |
| E | all six are receiver-side checks | no | no | H-033 can reject updates for stages the receiver does not have; H-060 `k == 0` unknown |
| F (H-129) | reset `idindex->nrec` (and `pos`) with the memset under `idindex_CriticalSection` | no | no | rebuild then equals the startup build; clears H-128 stale rows |
| G (H-171) | 41 is the outlier; HK has 201; the name is never written or sent | no | no | enlarge to 201 like HK |

## Q-A (H-025, H-048): what `combufsize` controls in relay

**Computation.** `Juk/VCom.cpp:1228-1240`: the first value (`sizeof(combufrec)` plus
EMITVA extension) is discarded at `:1236`; `combufsize = max(class_len[i])`, i.e.
payload only, without the 14-byte in-memory header (`lahetetty` 4, `portti` 2,
`so,id,iid,pkgclass` 4, `len` 2, `checksum` 2; `Juk/VDef.h:1165-1171`). HK adds 14
(`Hk/HkCom32.cpp:1095-1104`). [verified]
The value already varies per competition: `class_len[YHTEISLAHTOT] = 8 * n_os + 4`
(`Juk/VCom.cpp:1221`) and the `MAXEMVALAH` option (`Juk/VInit.cpp:2082-2083`,
`Juk/VCom.cpp:1230-1231`). [verified]

**(a) COMFILE.DAT record length: yes.** [verified]
- Write stride `Tp/TpComY.cpp:495-509` (`pos0 = ((cjseur+1)*maxyhteys+cn)*combufsize`,
  `_write(obfile, obuf, combufsize)`); read stride `:1467-1470`; sent-time patch
  `:1129`, `:1479`; restart scan `:2278-2324`.
- File header: block 0 is `maxyhteys` records; record 0 holds `so = maxyhteys` and
  `len = combufsize` (`Tp/TpComY.cpp:2364-2368`). So **yes, the file records its own
  record length** in `combufrec.len` of record 0.
- Per-stage files: HK stage change renames `obfilename[7]` and reopens
  (`Hk/HkIx.cpp:1888-1889`, `:1963-1970`); relay analogue not traced [inferred].

**(b) Wire: not directly.** Sent bytes are `obuf.len + 8` starting at `so`
(`Tp/TpComY.cpp:1513`); `len = class_len[class]`. Receivers copy `len + 14` into a
`sizeof(combufrec)` slot (`:648-654`, `:2399`). [verified]
**Indirect wire dependency:** `class_len[FILESEND] = combufsize - 14`
(`Juk/VCom.cpp:1240`), and the receiver rejects a packet whose `len` differs from its
own `class_len` (UDP `Tp/TpComY.cpp:1001`; stream `:750`, `:760`). [verified]
If a fix sets `combufsize = max + 14` and keeps this line, FILESEND packets grow by 14
bytes and are rejected by unfixed relay peers (and vice versa). Unacknowledged UDP
packets are retransmitted (`Tp/TpComY.cpp:1846-1849`), and file packets are taken
before the normal queue (`:1456-1461`), so the queue of the sending station on that
connection would stall [inferred].

**(c) Memory:** `outbuf` (no disk queue) stride `combufsize` with `memcpy(..., len+5)`
from `pkgclass` (`Tp/TpComY.cpp:529-530`, `:1494-1495`, `:2409`): 14 bytes overrun per
max-size packet. `__FCOMFILE__.TMP` stride `combufsize` (`:2884-2887`, `:2940`,
`:2965`); created per run (`:2934`), so no cross-version file issue. [verified]

**Restart mid-competition with mixed versions.** `opencomfile`
(`Tp/TpComY.cpp:2276-2302`): if record 0 has `len != combufsize`: when the file holds
only the header block it is reused silently; otherwise the message "Aiempi tiedosto
COMFILE.DAT ei yhteensopiva ..." and the choice delete or abort. Symmetric for a
fixed program reading an unfixed file and vice versa. [verified] Effect: no silent
misread, but the queue (sent and unsent records) is lost; unsent updates must be
resent with the existing resend functions (`uusintaTCP`, `Hk/HkCom32.cpp:1219`;
relay `Juk/VCom.cpp:1447`), which rebuild packets from KILP.DAT. [inferred]
The last 14 bytes of max-size records are already missing in an old file, so a
legacy-stride reader could not recover them anyway. [verified by arithmetic]
- `UnitComfile` viewers adopt the header length into the **global** `combufsize`
  (`ViestiWin/UnitComfile.cpp:135-147`, `cbHk/UnitComfile.cpp:145-148`, H-050): after
  a fix, viewing an old-format file in a running fixed program would switch the live
  queue to the old stride. H-050 should get a local stride in the same package.
  [verified code; inferred effect]
- Send-test replay reads a COMFILE with `RecLen = combufsize` (`Tp/TpComY.cpp:3054`,
  read at about `:3148-3149`) - a test tool; old files would be read with the wrong
  stride. [verified]

**H-048.** `lahetanumerotTCP` reads with `sizeof(combufrec)` (`Juk/VCom.cpp:1375-1384`;
HK `Hk/HkCom32.cpp:1202-1205`). Fix = use `combufsize`; local read only, the resend
rebuilds packets via `laheta(...)` from current data (`Juk/VCom.cpp:1398`), so no
file or wire change. Only compiled with `SERVER` (`Juk/VCom.cpp:1367`). [verified]

**Fix may:** change relay `combufsize` (record stride) and accept a one-time COMFILE
reset. **Must not:** change `class_len[FILESEND]` (freeze it at `max(class_len)` minus
14 as today) or any other `class_len`. Operational rule: no unsent messages before
upgrading a station (HK stage change already enforces this, `Hk/HkIx.cpp:1871-1875`).
Optional: accept `len == combufsize - 14` in record 0 as legacy and offer to keep the
file with the legacy stride.

## Q-B (H-041): KILPBUFSIZE stack buffers

| Location | Function | Use of buffer | Caller threads |
|---|---|---|---|
| `Hk/HkDat.cpp:565` | `kilptietue::read` | `getrec(&datf2, d, buf)` | any |
| `Hk/HkDat.cpp:583` | `kilptietue::write` | `pack(buf)`, `putrec` | `PUTREC(d,1)` from tarkcom, emit and timing threads |
| `Hk/HkDat.cpp:629` | `kilptietue::addfrec` | `pack`, `addrec` | UI, tarkcom (`addtall`) |
| `Hk/HkDat.cpp:1433` | KILP.DAT copy writer | `pack`, `addrec0/addrec` into `makefile(..., kilprecsize)` | UI |
| `cbHk/UnitExport.cpp:464` | `TFormExport::kirjKilpDat` | same | UI |
| `Hk/HkDat.cpp:176-177` | size computation | `kilprecsize = kilprecsize0 + n_pv*kilppvtpsize` | not a buffer |
| `Hk/HkSrj.cpp:156` | configuration gate | `n_pv > N_PV or valuku > VALUKU` | not a buffer |

Definitions `Hk/HkDef.h:98-103` (2500 if `VALUKU < 31`, else 10000; `KILPRECSIZE`
has the same values and no other use). Test files also declare
`char buf[KILPBUFSIZE]` (`Tests/core/HkLockTests.cpp:96`, `Tests/core/HkRecvTests.cpp:73`).
[verified, grep]

**Format effect: none.** The disk record length is the runtime `kilpparam.kilprecsize`
(`makefile(...)`, `Hk/HkDat.cpp:1435`); packet lengths are `class_len[KILPT] =
kilprecsize0 + 6` and `class_len[KILPPVT] = kilppvtpsize + 8` (`Hk/HkCom32.cpp:1086-1088`);
wire arrays `ckilp[400]` and `cpv[KILPPVSIZE+(VALUKU+2)*VASIZE]` (HK `combufrec`,
`Hk/HkDef.h` about `:1255`, `:1263`). KILPBUFSIZE appears in no offset, length or
header. [verified]
Sizing: largest record 11 200 bytes (C-11 measurement at `N_PV` 5, `VALUKU` 250). A
compile-time bound needs an expression over field sizes because `kilprecsize0` is
computed at run time from the field table (`Hk/HkDat.cpp:160-177`); a heap buffer of
`kilprecsize` bytes avoids the bound. [inferred]

**Stack.** None of read/write/addfrec is recursive [verified]. One buffer per call
chain (GETREC uses `kilparr`, not `read`). Thread stacks: `tarkcom` 40960
(`Tp/TpComY.cpp:2415`), `comajanotto` 40960 (`Hk/HkIV.cpp:4278`), `ProsEmitThread`
20480 (`cbHk/WinHk.cpp:379`), comm threads 40960 (`Tp/TpComY.cpp:2213-2233`). The
`_beginthread` size is the initial commit; the reserve is the image default when
that is larger: HkMaali has no stack setting (grep of `VS/Hk`, linker default 1 MB),
HkKisaWin `ILINK_StackReserveSize 0x00400000` (`RADStudio10/HkKisaWin.cbproj:153`).
[verified settings; inferred CreateThread semantics] The current 10 000-byte frame
already exceeds one page and works, so an 11-12 KB frame adds about 1.2-2 KB per
chain. [inferred]

**Relay (Q31).** Buffers are `char buf[KILPRECSIZE]` with `KILPRECSIZE =
sizeof(kilptietue)` (`Juk/vdat.cpp:145`, `:153`, `:181`; `Juk/VDef.h:837`), and the disk
record uses the same constant (`Juk/VInit.cpp:2472`). Equal by construction in every
build; the only overflow route is H-042 (unclamped XML counts). Shipped relay
Release has `MAXOSUUSLUKU=30` (`VS/V/JukMaali520.vcxproj:135`); 250 builds are
`Debug250` and `Release150os` (`:115`, `:157`) where the large frame is a stack cost,
not an overflow. [verified]

**Fix may** enlarge or heap-allocate the buffers freely.

## Q-C (H-026): unknown packet class in relay

- Dispatch `Juk/VCom.cpp:913-981`: slot `ibuf = inbuf + inb` (fixed stride
  `sizeof(combufrec)`, `Tp/TpComY.cpp:2399`); `default:` sets `lisa = 0`, so `inbens`
  never advances. [verified]
- The length is known and irrelevant at this point: the packet was already framed,
  length-checked and copied by `addseur` (`Tp/TpComY.cpp:648-654`), and **the ACK
  was already sent** after `addseur` succeeded (stream `:840-870`; UDP `:1040-1047`).
  Skipping needs only `lisa = 1`. [verified]
- HK: `default:` logs only when `testi` and keeps `lisa = 1` (`Hk/HkCom32.cpp:942-948`,
  `:952`). [verified]
- Transport filtering: stream (RS-232, binary TCP) requires `class_len[class] == len`
  (`Tp/TpComY.cpp:750`, `:760`), so a class with `class_len` 0 is refused before
  `addseur`; on TCP the peek path returns without consuming (`:750-752`); whether the
  stream recovers was not traced [unknown]. UDP skips the length check when
  `class_len` is 0 (`:1001`, H-032), so classes 7, 11, 12 (and >= 15, index out of
  range) reach dispatch. [verified]
- Classes: HK 0,1,2(KILPPVT),3,4,6,7(SEURAT),10,13,14 (`Hk/HkCom32.cpp:47-57`);
  relay 0,1,2(OSUUST),3,4,5(TARKT),6,8(VEMITT),9(YHTEISLAHTOT),10,13,14
  (`Juk/VCom.cpp:63-75`). Unknown to relay: 7, 11, 12. Unknown to HK: 5, 8, 9, 11, 12.
  Number 2 means different things. All relay products compile the same switch
  (case bodies under `#ifdef`, cases present), so no shipped relay-relay pair sends
  an unknown class. [verified]
- Real mixed sources: an HK station linked to a relay station. Relay `tark_alku`
  refuses an HK ALKUT (`vaihe != 0`, `Juk/VCom.cpp:859-865`) and stops its own sending
  (`com_keskeytt`, `Tp/TpComY.cpp:639`, `:1846`), but receiving continues; the HK
  SEURAT packet (`Hk/HkCom32.cpp:471`, sent from club edits `Hk/HkApu.cpp:780`) is
  class 7. Whether HK sends more than ALKUT without an ACK was not traced
  [inferred: unlikely]. Corrupted or hostile UDP datagrams with a valid checksum are
  the other source.
- Peer behaviour if a packet is dropped **without** ACK: the UDP sender resends until
  ACK (`Tp/TpComY.cpp:1846-1849`), no retry limit found; its queue for that connection
  stops [inferred]. Today the first unknown packet is ACKed, then `inbuf` fills
  (`INBUFL`) and `addseur` fails, so every later packet is unacknowledged and all peer
  queues towards this station stall. [inferred from code]

**Fix may:** skip the slot (`lisa = 1`) and log, exactly as HK; keep the ACK.
**Must not:** NAK or withhold the ACK (peers would retransmit forever).

## Q-D (H-071): FILESEND reception

Receiver `tark_tiedosto` (`Tp/TpComY.cpp:2998-3037`), state per connection
`FileRcv[cn]` (name `wchar_t[200]`): [verified]
- `pos < 0`: closes an open file.
- `pos == -1`: stores `koodi`; copies the name from `d.fl.buf` with
  `wcsncpy(..., 199)`; opens it with `_wfopen` in mode wb with no path check; `pos = 0`.
- `pos == -2` and `FileRcv.pos > 0`: forwards the file with `lah_tiedosto(name,
  cn+1, flags)` to the other peers (`lah_tiedosto` rewrites the name in place and
  refuses absolute names unless the local `/K` is set, `:2928-2930`).
- `pos >= 0`: if equal to the expected position, `fwrite(buf, d.fl.len, 1, ...)`;
  `d.fl.len` (INT32) is never compared with `class_len[FILESEND] - 16`; any mismatch
  or write error closes the file silently.
- No check of `lah_tiedostot` or any flag on the receiving side. Dispatched in every
  product: HK `Hk/HkCom32.cpp:939-940`, relay `Juk/VCom.cpp:968-969`; `Tp/TpComY.cpp`
  is in HkMaali, ViestiMaali, ViestiLuenta, HkKisaWin and ViestiWin (vcxproj and
  cbproj grep).

Sender `lah_tiedosto` (`Tp/TpComY.cpp:2898-2996`): strips `WorkingDir`; a file in an
**ancestor** of the working directory gets a relative name with `..` components
(`:2915-2926`); absolute names only with `/K` (`lahkaikkitied`, `:2928`). Chunk
`class_len[FILESEND] - 16` bytes, name at most `(class_len - 18)/2` characters. Sends
only to `comtpUDP` connections with `lah_tiedostot[cn] & flags` (`:2979-2981`).
Option `LÄHTIEDOSTOT[y][=x][/K]` (`Hk/HkInit.cpp:1516-1534`, `Juk/VInit.cpp:1275-1293`).
Callers (all GUI): the send-file menu item `Lhettiedosto1` in HK `cbHk/WinHk.cpp:2005-2011`
and relay `ViestiWin/UnitMain.cpp:1879`; offer after saving courses
`cbTp/UnitRadat.cpp:2404-2407`, series XML `cbHk/UnitSarjatiedot.cpp:841-844`, person
database `cbHk/UnitTKanta.cpp:1418-1421`. Console: only the `pos == -2` forward.
`FILETCP` and `sendtcpfile` (`Hk/HkTulostus.cpp:7031`, `:7331`) are a separate TCP
upload to an external service and do not use FILESEND. [verified]

User manual: `docs-site/docs/hkkisawin/hkliite2.md:213-252` and
`docs-site/docs/hkkisawin/hkliite1.md:81` document LÄHTIEDOSTOT: without `/K` the
file goes from the competition folder or a subfolder to the same relative folder on
the receiver (which must exist); with `/K` the full path is used as is and the
receiver must have the same drive and directory. The ViestiWin manual lists only
`FILETCP` (`docs-site/docs/viestiwin/hkliite1.md:49`). [verified]

**Fix may:** validate `0 <= d.fl.len <= class_len[FILESEND] - 16`; reject names with
a drive, a leading separator or `..` **unless** the receiver opts in (for example its
own `/K`); require a terminator inside the packet; optionally require LÄHTIEDOSTOT on
the receiver. **Breaks:** documented `/K` transfers if absolute paths are refused
without an opt-in; the undocumented ancestor-directory case (`..`); and, if a
receiver-side LÄHTIEDOSTOT requirement is added, installations where only the sender
is configured (the manual, `hkliite2.md:215-218`, does not say which end must have
the option) [inferred].

## Q-E: receiver-side only? Can legitimate values be rejected?

All six fixes are receiver-side checks or local buffer changes; none changes a packet
layout, a class length or a file format. [verified]

| Row | Check | Legitimate value the check could reject |
|---|---|---|
| H-072 HK `d1 == 5` | `0 <= d2 < MAXERA` (200) before `eralahto[d2]` and `era_akt` (`Hk/HkCom32.cpp:640-645`); do not forward rejected values | none in current source: every sender indexes its own `eralahto[era]` first (`Hk/HkCom32.cpp:558-572`; callers `cbHk/UnitAjanotto.cpp:1258`, `Hk/HkIV.cpp:2411`, `Hk/HkDat.cpp:1257`); guards `era_akt < MAXERA` at `cbHk/UnitAjanotto.cpp:1884`, `Hk/HkIV.cpp:3897`; an out-of-range era already corrupts the sender [verified]. Only if versions differ in `MAXERA` [unknown] |
| H-072 relay `d1 == 3` | `0 <= sarja < MAXSARJALUKU+MAXYHD` (`Juk/VGlobals.cpp:81`) or `< sarjaluku` | `lahetaKnoSiirto` has no originator in current source, only the forward in `tallKnoSiirto` (`Juk/vdat.cpp:966-969`) [verified grep]; a `sarjaluku` bound could drop a packet from a peer with more series [inferred]; the array bound cannot |
| H-032 UDP | refuse `pkgclass >= N_CLASS` and `class_len == 0` (keep the ALKUT len 3/10 exception), bound `len <= sizeof(d)` (`Tp/TpComY.cpp:1001`) | no legitimate class has `class_len` 0 in the receiving program (Q-C). Check that relay EMITVA with `MAXEMVALAH` (+16 bytes, `Juk/VCom.cpp:1230-1231`) still fits `sizeof(d)` in shipped builds [inferred yes: the union holds `y` = 244 bytes at n_os 30 and C-11 measured the max class at 244]. Rejected UDP packets are not ACKed, so a wrong bound stalls the peer (Q-C) |
| H-061 | keep `pkglen` per connection (static like `pkgclass[cn]`), check `id[cn][3] < N_CLASS` (`Tp/TpComY.cpp:707`, `:750`, `:760`) | none; all classes are below 15 |
| H-033 | `0 <= d.pv.pv < kilpparam.n_pv` (`Hk/HkCom32.cpp:272-279`; `pv = new kilppvtp[n_pv]`, `Hk/HkDat.cpp:206`) | **yes, in mixed configurations**: `class_len[KILPPVT]` depends on `kilppvtpsize`, not on `n_pv` (`Hk/HkCom32.cpp:1088`), and `tark_alku` does not compare `n_pv` (`Hk/HkCom32.cpp:821-860`), so a peer with more stages passes the length check; today an out-of-bounds write, after the fix a dropped update the receiver cannot store anyway [inferred] |
| H-060 | `1 <= k <= MAXPAIKKA` in case 1, as case 2 already does (`Hk/HkSak.cpp:159-171`) | `k == 0` (lane 00) from the Kurvinen device: meaning not known [unknown]; the network path via EXTRA `d1 == 6` (`Hk/HkCom32.cpp:647`, `Hk/HkSak.cpp:239-244`) carries the same bytes |
| H-073 | enlarge `msg` or bound the append (`Hk/HkEmit.cpp:6168-6185`, `Juk/VEmit.cpp:6065-6080`); bound `rastiluku` to the array (`Hk/HkDef.h:1225`, 49 rows, loop to `MAXNLEIMA` 50) | nothing rejected; only the log line changes. Each pair is up to 12 characters, so 49 or 45 pairs need about 590 or 540 characters: truncation would shorten lines that are today written in full after overflowing. Prefer enlarging. Relay pairs beyond 45 with `MAXEMVALAH` are already not logged [verified] |

## Q-F (H-129, H-128, H-102, H-134): stage change

- `teejarj(true)`: `memset(idindex->ix, 0, (maxrec+2) * idindex->recl)`
  (`Hk/HkIx.cpp:1610`) leaves `idindex->nrec` (and `pos`) unchanged; `setpos`
  (`Hk/HkIx.cpp:205-221`) then `addix` (`tputilv2/dbindex.cpp:51-71`) inserts at
  `nrec` and increments without a capacity check. [verified]
- Must reset: `idindex->nrec = 0` and `idindex->pos = -1` (fields
  `include/tputil.h:99-107`; `nextix` uses `pos`, `tputilv2/dbindex.cpp:126`),
  together with the memset, inside `EnterCriticalSection(&idindex_CriticalSection)`
  (readers `getpos` and `nextpos` take only that lock, `Hk/HkIx.cpp:171`, `:188`).
  [verified] Between the reset and the end of the rebuild loop `getpos` returns -1
  for bibs not yet reinserted (H-102). Holding `idindex_CS` across the whole loop
  would close that window, but the loop also takes `kilpdat_CriticalSection` via
  `PUTREC` (`Hk/HkDat.cpp:615`); lock order not traced [unknown]. `vaihdavaihe`
  already holds the tall, ajat and emit locks around `teejarj(true)`
  (`Hk/HkIx.cpp:1897-1899`, `:1934`); `setpos` re-enters `idindex_CS` on the same
  thread, which a CRITICAL_SECTION allows. [verified]
- Interaction with H-128: after a full reset the rebuild inserts each record once,
  in the same loop as at startup (`Hk/HkIx.cpp:1757-1758`; the startup branch uses a
  fresh `calloc`, `:1600`), so stale `numero -> 0` rows from earlier deletions or bib
  changes disappear. Fixing H-129 alone creates no new failure; the H-128 defects
  return when deletions or bib changes happen in the new stage. Duplicate bibs
  behave as at startup. [inferred]
- Files written: the rebuild loop writes changed records with `kilp.PUTREC(d, 1)`
  (`Hk/HkIx.cpp:1755-1756`: status `-`, start time stored as result in mode 2,
  trimmed club name) through `kilptietue::write` (KILP.DAT, unchanged record format)
  and SQL when enabled (`Hk/HkDat.cpp:617-621`). The stage change also switches the
  COMFILE, time and emit file names to the new stage (`Hk/HkIx.cpp:1888-1894`,
  `:1936`, `:1963-1970`). The `nrec` reset changes none of these. [verified]
- H-134 minimum invariant: after `vapautaemitmuisti` (`Hk/HkIx.cpp:1905-1909`) no
  path may read course or emit memory unless `lue_radat_xml` returned 0. Any non-zero
  return (-1 and parse error 1; `Hk/HkEmit.cpp:853-854`, `Hk/HkIx.cpp:1937-1941`) must
  leave a consistent state before the locks are released (`Hk/HkIx.cpp:1976-1978`):
  either emit off (`emitfl = 0`) with every freed pointer null and every count zero,
  so that `pros_emit`, `ProsEmitThread` and tarkcom EMITT see empty data, or the
  text-file fallback that `aloitus` uses (`Hk/HkInit.cpp:3595-3604`). [inferred]

## Q-G (H-171): `cfgfilename` in relay

- `Juk/VInit.cpp:74` declares `static wchar_t cfgfilename[41] = L"LASKENTA.CFG"`; HK
  `Hk/HkInit.cpp:59` has `[201]` with the same two copies (`:565`
  `wcsncpy(..., 120)`, `:572` `wcsncpy(..., fldn+4, 40)`). The copy length 120 and the
  HK size 201 agree; 41 is the outlier and matches only the `CFG=` copy (40). Both
  sizes exist since the first commit in history (50db2f8). [verified; intent inferred]
- All uses in Juk/ and ViestiWin/ (grep): `:74` definition; `:440` passed to
  `new TextFl(cfgflnm, "rt")` (read only; `TextFl` keeps a pointer,
  `include/tputil.h:134`); `:652` `wcsncpy(cfgfilename, cfgflname, 120)`, which does
  not terminate when the source has 120 or more characters, so at least 121 elements
  are needed (HK relies on static zero fill); `:659` `wcsncpy(..., fldn+4, 40)` from
  `fldn[90]` filled with at most 44 characters (`:656`); `:2188` `%s` into `msg[201]`
  with about 45 fixed characters, which fits 120 characters. No uses in `ViestiWin/`.
  [verified]
- Not written to any file, not sent, not part of any record. Enlarging changes no
  format. [verified]

**Fix:** `cfgfilename[201]` as in HK (121 minimum). This removes the overwrite of
`autostr`, `korkeat`, `star` and `baudst[0..2]` described in H-171.

## Open points

1. TCP stream behaviour after a refused header (peek without consume,
   `Tp/TpComY.cpp:750-752`) [unknown].
2. Relay COMFILE handling at stage change not traced (HK renames per stage).
3. Meaning of Kurvinen lane 00 in case 1 [unknown].
4. Lock order if `idindex_CS` is held across the whole rebuild [unknown].
5. Exact `sizeof(combufrec::d)` per relay build against extended EMITVA; compute in
   the test harness before fixing H-032 [inferred: fits].
6. Written to the scratchpad, not to `doc/arkkitehtuuri/todisteet/c13/raportit/`:
   this agent runs under a read-only rule for the repository.
