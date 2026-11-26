# C-13 part B: findings register unification, inventory

Scope: inventory half of part B. Work from `rekisteri/havainnot.md` (177 H rows, 52 T rows; H-099 refuted and excluded from classes) and the batch reports; no new source audit. Paths are relative to `TPsource/V52/`. Identifiers are only cross-referenced, none deleted or merged. Machine files: `osaB/luokat.csv` (228 rows), `osaB/suhteet.csv` (221 relations).

Confidence labels in this report: [todennettu] = stated in the cited row/report or lines overlap in the cited rows (CSV `varmuus=verified`); [paatelty] = inferred by me from row text, chain not traced in code (CSV `inferred`). I opened no source file; line evidence is taken from register rows. Macro values from the C-10/C-12 scripts were not used.

## 1. Summary

- 176 open H + 52 T = 228 rows, each with one primary class from a fixed vocabulary of 27 classes (section 2), 90 with a secondary class.
- 221 relations: hk_viesti_vastine 11, korjattava_yhdessa 35, korjaus_ratkaisee 60, ristiriita 12, sama_koodikohta 61, sama_vika 15, seuraus 27.
- Largest classes: RACE 17, GUI_LOGIC 16, DEAD_SWITCH 16, IO_ERR 14, BUF_INT 12, LOCK_LEAK 11.
- 61 open H rows (of which 12 korkea) have no priority; see section 6.
- Duplicates and same-mechanism recordings: C-12 C3 = H-063, C4 mostly H-043, B1 = H-036 (no own ids); H-061 is H-032 class; H-014 is extended by H-160; H-156 item = H-071 part; H-144 ~ H-019; H-170 N-11 ~ H-164; H-069 part moved to H-112; T-019/T-041 overlap on KILP.DAT checksum.

## 2. Class vocabulary and inventory

Primary class = the mechanical defect type of the main finding; secondary = a second mechanism worth checking when fixing. "Highest sev" is the register severity normalised (korkea > keskitaso > matala-keskitaso > matala; conditional severity counted at its highest). "Highest prio" is the best current priority among H members (T rows have none). KK = earlier fix packages (KK-C9-x, KK-C10-x, KK-C12-x; `*` partial) with number of members in the class.

| Class | Meaning | n (H/T) | Highest sev | Highest prio | KK coverage | Secondary uses |
|---|---|---|---|---|---|---|
| BUF_NET | missing bounds/length check on network packet input | 7 (6/1) | korkea | P1 | - | 5 |
| BUF_DEV | missing bounds/length check on device message (serial/TCP reader, emiTag, shooting) | 4 (4/0) | korkea | P2 | - | 2 |
| BUF_FILE | missing bounds check on local file, import or configuration input | 8 (8/0) | korkea | P3 | KK-C12-2:1 | 2 |
| BUF_GUI | unchecked index/length from operator or GUI control (incl. ItemIndex -1) | 7 (6/1) | keskitaso | P3 | - | 1 |
| BUF_INT | fixed buffer smaller than internally computed data, no external input needed (wrong size/stride/format) | 12 (12/0) | korkea | P2 | KK-C12-2:1 | 1 |
| FMT | format-string / argument mismatch | 2 (2/0) | korkea | P4 | - | 2 |
| LOCK_LEAK | lock left held, double Leave or unbalanced Enter/Leave on a path | 11 (10/1) | korkea | P2 | - | 3 |
| LOCK_ORDER | lock-order inversion, deadlock risk, blocking/UI call under a lock | 9 (7/2) | korkea | P2 | KK-C9-2:1 | 2 |
| RACE | unsynchronised shared state, missing lock, VCL access from worker thread | 17 (16/1) | korkea | P3 | - | 8 |
| THREAD_LIFE | thread lifecycle: start, stop, restart, shutdown | 7 (6/1) | keskitaso | P3 | KK-C10-2:1 | 2 |
| PTR_OWN | use after free, double free, dangling or stale pointer/handle ownership | 7 (6/1) | korkea | P2 | KK-C12-1:1, KK-C9-1:4 | 4 |
| UNINIT_NULL | uninitialised variable/pointer or NULL dereference / fclose(NULL) | 11 (9/2) | keskitaso | P2 | - | 3 |
| LEAK | resource leak in a repeated operation | 3 (3/0) | keskitaso | P2 | KK-C12-1:1, KK-C9-3:2 | 0 |
| IO_ERR | I/O error not propagated, acknowledgement before persistence, lossy file mode | 14 (12/2) | keskitaso | P3 | KK-C9-2:8 | 8 |
| STAGE_RESET | stage-change or restart state not reset / not re-runnable | 9 (8/1) | korkea | P3 | KK-C9-2:1 | 5 |
| DERIVED | derived structure (index, counter, link) out of sync with primary data | 9 (8/1) | korkea | P2 | - | 2 |
| GUI_LOGIC | GUI handler business-logic error | 16 (16/0) | korkea | P2 | - | 4 |
| FORK_DIV | HK/relay (or console/GUI) fork divergence | 6 (2/4) | keskitaso | P5 | KK-C12-3:1 | 9 |
| SQL | SQL integration (writer, statements, import, start-up) | 7 (7/0) | korkea | P2 | KK-C10-1:5, KK-C10-4*:1 | 4 |
| DEV_CONN | device/connection lifecycle: open, retry, reconnect, liveness | 8 (7/1) | korkea | P2 | KK-C10-1*:1, KK-C10-2:4, KK-C10-2*:1, KK-C10-4:1 | 4 |
| ENCODING | encoding / compiler / RTL dependence | 10 (4/6) | keskitaso | P4 | - | 3 |
| CONFIG | configuration parser and key handling | 5 (3/2) | keskitaso | P3 | KK-C12-1*:1, KK-C12-2:2, KK-C12-2*:1 | 8 |
| DEAD_SWITCH | dead code, undefined switch, switch hygiene | 16 (4/12) | keskitaso | P3 | KK-C10-3:1, KK-C12-1:1, KK-C12-3:2, KK-C9-1:1 | 4 |
| BUILD_DEPLOY | build, deployment, packaging, licensing | 8 (1/7) | keskitaso | P3 | KK-C10-3:4 | 0 |
| TEST_CI | test / CI / diagnostics gap | 2 (1/1) | keskitaso | - | - | 0 |
| PROTO | protocol or file-format design (no authentication, version, checksum, ordering) | 10 (8/2) | korkea | - | KK-C10-1*:1, KK-C10-4:1 | 4 |
| ARCH | architecture / layering / global state as design debt | 3 (0/3) | keskitaso | - | - | 0 |

Total primary: 228. Classes with no primary member: none.

### 2.1 Members per class

| Class | Primary members | Secondary members |
|---|---|---|
| BUF_NET | H-032 H-033 H-035 H-061 H-071 H-072 T-022 | H-026 H-055 H-057 H-060 H-073 |
| BUF_DEV | H-055 H-056 H-060 H-070 | H-008 T-022 |
| BUF_FILE | H-036 H-042 H-049 H-050 H-052 H-057 H-058 H-074 | H-002 H-065 |
| BUF_GUI | H-082 H-083 H-084 H-085 H-113 H-114 T-037 | H-042 |
| BUF_INT | H-001 H-025 H-041 H-048 H-059 H-066 H-067 H-073 H-076 H-077 H-079 H-171 | H-129 |
| FMT | H-002 H-003 | T-021 T-029 |
| LOCK_LEAK | H-008 H-012 H-024 H-040 H-087 H-088 H-089 H-090 H-110 H-115 T-036 | H-028 H-125 H-162 |
| LOCK_ORDER | H-009 H-021 H-091 H-092 H-093 H-103 H-153 T-031 T-032 | H-167 T-033 |
| RACE | H-007 H-010 H-011 H-018 H-019 H-045 H-047 H-051 H-100 H-102 H-106 H-108 H-136 H-137 H-138 H-144 T-033 | H-017 H-031 H-050 H-094 H-096 H-101 H-107 T-003 |
| THREAD_LIFE | H-038 H-097 H-098 H-101 H-104 H-167 T-030 | H-095 H-160 |
| PTR_OWN | H-094 H-095 H-096 H-149 H-151 H-152 T-042 | H-047 H-105 H-154 H-158 |
| UNINIT_NULL | H-062 H-063 H-064 H-065 H-068 H-069 H-075 H-107 H-112 T-020 T-035 | H-027 H-085 H-163 |
| LEAK | H-150 H-155 H-157 | - |
| IO_ERR | H-015 H-044 H-053 H-054 H-086 H-145 H-146 H-147 H-148 H-154 H-156 H-168 T-040 T-041 | H-021 H-043 H-063 H-134 H-153 H-174 T-019 T-020 |
| STAGE_RESET | H-031 H-105 H-131 H-132 H-133 H-134 H-141 H-143 T-038 | H-014 H-023 H-097 H-102 H-139 |
| DERIVED | H-013 H-128 H-129 H-130 H-135 H-139 H-140 H-142 T-039 | H-022 H-109 |
| GUI_LOGIC | H-017 H-109 H-111 H-116 H-117 H-118 H-119 H-120 H-121 H-122 H-123 H-124 H-125 H-126 H-127 H-175 | H-086 H-112 H-113 H-115 |
| FORK_DIV | H-046 H-081 T-008 T-024 T-034 T-052 | H-069 H-075 H-106 H-117 H-121 H-123 H-130 T-035 T-051 |
| SQL | H-014 H-043 H-160 H-161 H-162 H-163 H-169 | H-100 H-165 H-170 T-043 |
| DEV_CONN | H-027 H-028 H-037 H-159 H-164 H-166 H-170 T-048 | H-056 H-108 H-138 H-155 |
| ENCODING | H-004 H-005 H-078 H-080 T-004 T-016 T-023 T-025 T-026 T-027 | H-079 H-081 H-147 |
| CONFIG | H-173 H-174 H-176 T-014 T-051 | H-036 H-132 H-171 H-172 H-175 T-017 T-024 T-045 |
| DEAD_SWITCH | H-020 H-158 H-172 H-177 T-007 T-009 T-011 T-012 T-013 T-015 T-018 T-028 T-029 T-045 T-049 T-050 | H-035 H-176 T-008 T-052 |
| BUILD_DEPLOY | H-165 T-005 T-006 T-010 T-017 T-044 T-046 T-047 | - |
| TEST_CI | H-006 T-021 | - |
| PROTO | H-016 H-022 H-023 H-026 H-029 H-030 H-034 H-039 T-019 T-043 | H-007 H-025 H-071 H-166 |
| ARCH | T-001 T-002 T-003 | - |

Per-row rationale: `osaB/luokat.csv`. Rows whose class is a judgement call: H-014 (SQL vs STAGE_RESET), H-025 and H-048 (BUF_INT, not a network overflow: sizing/stride defects), H-046 (FORK_DIV: relay export diverges from `pack()`), H-086 (IO_ERR: file-mode bug), H-168 (IO_ERR: ACK of dropped data), H-158 (DEAD_SWITCH, with PTR_OWN part under KK-C9-1), H-170/H-176/H-156/H-157/H-126/H-127/H-142/H-143/H-144 (roll-up rows, classed by their dominant item). [paatelty]

### 2.2 Fix-package coverage by class

Earlier packages (KK-C9-1..3, KK-C10-1..4, KK-C12-1..3) cover 43 distinct ids (from `osaA0/kk_aiemmat.csv`). Open ids in classes with no KK coverage: FMT (2), ENCODING (10), TEST_CI (2), RACE (17), LOCK_LEAK (11), DERIVED (9), GUI_LOGIC (16), BUF_NET (7), BUF_DEV (4), UNINIT_NULL (11), BUF_GUI (7), ARCH (3). The big uncovered areas are memory safety (BUF_*, UNINIT_NULL, FMT), concurrency (RACE, LOCK_*, THREAD_LIFE), stage-change/derived state, GUI logic, encoding and protocol design.

## 3. Relations

Columns as in `osaB/suhteet.csv`. Direction: for `seuraus` and `korjaus_ratkaisee` id_a is the cause/root; for `ristiriita` the pair is the two rows that disagree; other types are symmetric. `v` = verified [todennettu], `i` = inferred [paatelty].

Counts: hk_viesti_vastine: 7 verified + 4 inferred; korjattava_yhdessa: 26 verified + 9 inferred; korjaus_ratkaisee: 52 verified + 8 inferred; ristiriita: 10 verified + 2 inferred; sama_koodikohta: 51 verified + 10 inferred; sama_vika: 11 verified + 4 inferred; seuraus: 14 verified + 13 inferred.

### 3.1 sama_vika - same defect recorded twice or second manifestation

| A | B | Evidence | c |
|---|---|---|---|
| H-061 | H-032 | H-061 row: same defect class as H-032 on serial/binary TCP; Tp/TpComY.cpp:1001,:654 vs :750,:760 | v |
| H-014 | H-160 | H-160 row: extends H-014 "no exception handling"; cbHk/UnitMySQL.cpp:637-760 | v |
| H-014 | H-097 | H-097 row: same stage-change thread stop, writer thread is H-014; cbTp/UnitEmitSQL.cpp:29-31 | v |
| H-018 | H-106 | H-018 row lists ViestiWin worker-thread VCL sites; H-106 ViestiWin/UnitStatus.cpp:532-535 is a second site | v |
| H-018 | H-101 | H-101 row: BtnLahValClick VCL handler on worker thread, see H-018; cbHk/UnitComfile.cpp:1047-1066 | v |
| H-019 | H-144 | H-144 row: writeerror_w family shares one global swap struct perr_ptrs2; H-019 cbTp/cbTpApu.cpp:470-489 | i |
| H-071 | H-156 | H-156 row item N-B7 = FILESEND partial file forwarded, also in H-071 row (N-B7) | v |
| H-069 | H-112 | H-069 row: TallennaVaihto uninitialised kno moved to H-112 | v |
| H-058 | H-074 | same count-pass vs fill-pass mismatch in another loader: Hk/HkEmit.cpp:2896-2937 vs Hk/HkIV.cpp:231-310 | i |
| H-129 | H-133 | H-133 row: initindex does not reset idindex->nrec (H-129 pattern); Hk/HkIx.cpp:101-107 vs :1573-1610 | v |
| T-019 | T-041 | both rows state KILP.DAT has no checksum (dbboxm/dbbox1.cpp:104-211, :227-251) | v |
| T-012 | T-049 | T-049 row lists SIIMPORT dead code also described by T-012 (Tp/TpComY.cpp:1932-2076) | v |
| H-176 | T-052 | T-052 row: RFID keys silently become ALGE timing = H-176 item | v |
| H-170 | H-164 | H-170 item N-11 comajanotto spins when port vanishes vs H-164 vanished serial reader never reopened | i |
| T-017 | T-051 | T-051 row lists ViestiLuenta same-name configurations with shipped sizes = T-017 identical defines (VS/V/JukLuenta520.vcxproj:95-152) | i |

### 3.2 sama_koodikohta - different defects in the same function/lines

| A | B | Evidence | c |
|---|---|---|---|
| H-032 | H-071 | both cite Tp/TpComY.cpp:1001 and :2399 (packet length vs class_len) | v |
| H-032 | H-026 | both cite Tp/TpComY.cpp:1001 (receive class handling) | v |
| H-026 | H-071 | both cite Tp/TpComY.cpp:1001 and relay Juk/VCom.cpp:968-981 | v |
| H-025 | H-021 | Tp/TpComY.cpp:529-530 (H-025) inside :519-545 (H-021) memory queue | v |
| H-025 | H-030 | both cite Tp/TpComY.cpp:509 and :1467-1470 | v |
| H-025 | H-044 | Tp/TpComY.cpp:509 and :1465-1513 in both | v |
| H-030 | H-044 | Tp/TpComY.cpp:1467-1470 in :1465-1513 | v |
| H-007 | H-033 | both in HK tark_kilp: Hk/HkCom32.cpp:260,:326-332 and :274-279 | v |
| H-007 | H-022 | HK tark_kilp neighbours: Hk/HkCom32.cpp:326-332 vs :324-325 | i |
| H-016 | H-022 | Hk/HkCom32.cpp:204-208,:220-222 inside H-016 :195-233 | v |
| H-022 | H-023 | Hk/HkCom32.cpp:296-305 vs :283-293, same receive handler; Hk/HkDef.h:1254-1261 | i |
| H-008 | H-060 | same call site Hk/HkCom32.cpp:647 and Tp/TpLaitteet.cpp:1670; Hk/HkSak.cpp | v |
| H-040 | H-148 | relay addfrec Juk/vdat.cpp:178-192 vs :178-191 | v |
| H-040 | H-047 | Juk/vdat.cpp:185-189 inside addfrec :178-192 | v |
| H-047 | H-148 | Juk/vdat.cpp:185-189 vs :178-191 | v |
| H-015 | H-047 | dbboxm/dbbox1.cpp:84-100 inside :64-100 (file open/close layer) | v |
| H-011 | H-045 | Hk/HkEmit.cpp:1600-1623 in both (emit file without emit_CS) | v |
| H-053 | H-105 | Hk/HkEmit.cpp:1581 (lueem) inside H-105 :1573-1583 | v |
| H-034 | H-141 | Hk/HkEmit.cpp:1627-1638 inside :1627-1666 duplicate-card prevention | v |
| H-102 | H-129 | same line Hk/HkIx.cpp:1610 teejarj(true); H-129 row says different defect | v |
| H-132 | H-133 | Hk/HkInit.cpp:3357-3360 wmemmove of file names in both | v |
| H-131 | H-132 | aikaflname handling Hk/HkInit.cpp:3357-3364 adjacent | i |
| H-105 | H-134 | Hk/HkIx.cpp:1907-1941 contains :1909-1912,:1936-1946 | v |
| H-134 | H-174 | XML parse error treated as missing/success: Hk/HkXml.cpp:1824-1826 vs :1832-1837 | i |
| H-031 | H-094 | both cite Hk/HkIV.cpp:796 | v |
| H-094 | H-136 | pros_bdg_t Hk/HkIV.cpp:790-851 vs :796-808,:753-781 | v |
| H-038 | H-095 | Hk/HkInit.cpp:3778-3790 inside :3749-3790 shutdown | v |
| H-038 | H-167 | Hk/HkInit.cpp:3781-3804 overlaps :3778-3790 | v |
| H-104 | H-095 | lopetus path Hk/HkInit.cpp:3739-3742 vs :3749-3790 | i |
| H-009 | H-019 | cbTp/cbTpApu.cpp:467-490 and :470-489 writeerror_w | v |
| H-019 | H-021 | cbTp/cbTpApu.cpp:470-489 and :480-490 | v |
| H-021 | T-026 | cbTp/cbTpApu.cpp:480-489/490 in both | v |
| H-019 | T-026 | cbTp/cbTpApu.cpp:480-489 in both | v |
| H-028 | H-096 | com/WinUDP.cpp:656-667 and :655-659 | v |
| H-027 | H-037 | com/WinUDP.cpp:845-847 and Tp/TpComY.cpp:1297-1301 TCP open | v |
| H-030 | H-037 | Hk/HkCom32.cpp:986-988 and :986-987 | v |
| T-031 | H-096 | com/WinUDP.cpp:343 in both (tcpip_CS held in close) | v |
| T-031 | H-093 | com/WinUDP.cpp:911 inside H-093 :907-936 (tcpip_CS) | v |
| H-078 | H-079 | tputilv2/TextFileClass.cpp:357-359 and :347-359 (TextFl::Open) | v |
| H-079 | H-086 | tputilv2/TextFileClass.cpp:347-359 inside :322-359 | v |
| H-078 | H-086 | tputilv2/TextFileClass.cpp:357-359 inside :322-359 | v |
| H-059 | H-068 | HkConsole/HkCons2.cpp:1263 inside :1244-1317 | v |
| H-059 | H-075 | relay console tulostamaali JukConsole/vcons2.cpp:2165-2178 and :2140,:2196-2200 | v |
| H-111 | H-117 | cbHk/UnitAjanotto.cpp:659,:672 inside :657-674 | v |
| H-109 | H-115 | cbHk/UnitKilpailijatiedot.cpp:1157-1176 inside :1157-1211 | v |
| H-069 | H-123 | ViestiWin/UnitArvonta.cpp:418,:464 inside :404-460/:353 | v |
| H-118 | H-123 | ViestiWin/UnitArvonta.cpp:442-455 inside :404-460 | v |
| H-110 | H-113 | ViestiWin/UnitArvonta.cpp:548-553 inside :544-576 handler | i |
| H-119 | H-120 | both in HK lueKilp3 + tallennaKilpailija: cbHk/UnitEmit.cpp:1663-1745 and :1615,:693-727 | i |
| H-087 | H-092 | Juk/VEmit.cpp:1510,:1409 and print_CS/emit path in both | v |
| H-087 | H-103 | Juk/VEmit.cpp:1409,:1510,:1524 in both | v |
| H-092 | H-103 | Juk/VEmit.cpp:1409,:1510,:1524 and Juk/VCom.cpp:828 in both | v |
| H-014 | H-100 | cbHk/UnitMySQL.cpp:699-701,:748-752 inside H-100 :697-701,:739-752 | v |
| H-100 | H-162 | cbHk/UnitExport.cpp:436 inside :420-450 | v |
| H-043 | H-160 | cbHk/UnitMySQL.cpp:703-719 inside :690-760 | v |
| H-056 | H-070 | Tp/TpLaitteet.cpp:1547,:1706-1711 in both (reader thread loop) | v |
| H-150 | H-151 | both in HK autofile Hk/HkTulostus.cpp:7060-7340 | i |
| H-149 | H-158 | relay autofile Juk/VTulostus.cpp:7036-7186; H-158 row cites H-149 d | v |
| H-036 | H-176 | Juk/VInit.cpp:1139 vs :1168-1171 same relay parameter parser | i |
| H-025 | H-048 | combufsize vs disk-queue stride: Tp/TpComY.cpp:495,:509; Juk/VCom.cpp:1376-1384 | i |
| T-016 | T-023 | both cite include/tputil.h:27-29 (-K -a1 without restore); viittaukset T-023->T-016 | v |

### 3.3 hk_viesti_vastine - HK/relay counterparts or one-fork-only defects

| A | B | Evidence | c |
|---|---|---|---|
| H-149 | H-151 | dangling PRFILE after closeprfile in relay autofile (Juk/VTulostus.cpp:7036-7043) vs HK autofile (Hk/HkTulostus.cpp:7060,:7171-7174); both KK-C9-1 | i |
| T-034 | H-017 | T-034 row: check inside lock only in relay; viittaukset T-034->H-017 | v |
| T-034 | H-117 | T-034 row: range guard only in relay; HK defect cbHk/UnitAjanotto.cpp:657-674 | v |
| T-034 | H-069 | T-034 row: relay-only defect (ViestiWin/UnitArvonta.cpp:373) | v |
| T-034 | H-121 | T-034 row: relay-only argument shift; ViestiWin/UnitAjanotto.cpp:1221 vs cbHk/UnitAjanotto.cpp:1226 | v |
| H-117 | H-121 | opposite one-sided fixes in the same timing form tall_rivi call: cbHk/UnitAjanotto.cpp:1226,:1234-1250 vs ViestiWin/UnitAjanotto.cpp:1170,:1221 | v |
| H-126 | H-127 | both roll-ups carry the N-24 naytaRivi defect, relay ViestiWin/UnitAjanotto.cpp:291-332 and HK cbHk/UnitEmit.cpp | v |
| H-123 | H-124 | renumber checks: relay ViestiWin/UnitArvonta.cpp:81,:353,:404-460 vs HK cbHk/UnitArvonta.cpp:575,:677-716 | i |
| H-130 | H-139 | relay series-change counts (Juk/vdat.cpp:738-743) vs HK counters (Hk/HkIx.cpp:47-51,:822-823); both under T-039 | i |
| H-041 | H-042 | HK KILPBUFSIZE 10000 (Hk/HkDef.h:45-58) vs relay KILPRECSIZE buffer (Juk/vdat.cpp:145,:153,:181); Q31 asks for 250-build analogue | i |
| H-106 | H-018 | H-106 row: HK writes only flags, ViestiWin writes TEdit from threads (cbHk/UnitStatus.cpp:586-596 vs ViestiWin/UnitStatus.cpp:532-535) | v |

### 3.4 seuraus - A makes B worse or B reachable only through A

| A | B | Evidence | c |
|---|---|---|---|
| H-161 | H-160 | H-160 row lists H-161 invalid statements as trigger of SQL exception termination; riippuvuudet.md 19.3.1 | v |
| H-161 | H-169 | riippuvuudet.md 19.3.2: stale mirror after H-160/H-161 makes lue_SQL import destructive | v |
| H-160 | H-169 | riippuvuudet.md 19.3.2: mirror incomplete after writer crash makes import destructive | v |
| H-165 | H-163 | riippuvuudet.md 19.3: missing DLLs in release ZIP make H-163 start-up failure likely | v |
| H-031 | H-094 | H-031 row C-6: every badge creates/destroys etls queue and exposes H-094 race; Hk/HkIV.cpp:796 | v |
| H-128 | H-129 | H-129 row: deletions and renumbers (H-128) lower the overflow threshold | v |
| H-134 | H-105 | H-105 row: cards written to closed handle after the H-134 branch; Hk/HkIx.cpp:1907-1941 | v |
| H-016 | H-039 | no version/origin lets old state win via another route (H-039 Hk/HkCom32.cpp:186-189) | i |
| H-016 | H-030 | no version lets resend of old snapshots undo newer changes (Tp/TpComY.cpp:1111-1167) | i |
| H-016 | H-007 | H-007 row: local change reverted and propagated to peers; no versioning in Hk/HkCom32.cpp:195-233 | i |
| H-018 | H-107 | viittaukset H-018->H-107; H-107 GUI hooks run on reader/pros_bdg_t threads (cbHk/UnitEmit.cpp:966-970) | v |
| H-050 | H-025 | viewer sets global combufsize (cbHk/UnitComfile.cpp:145-176) used by queue code (Tp/TpComY.cpp:509) | i |
| H-155 | H-027 | viittaukset H-155->H-027; both openportTCP failure paths com/WinUDP.cpp:266-293 vs :812-817 | i |
| H-029 | H-071 | no authentication lets any datagram reach the unvalidated handler (muistiturvallisuus.md luku 15) | i |
| H-029 | H-072 | no authentication lets any datagram reach EXTRA handler (muistiturvallisuus.md luku 15) | i |
| T-020 | H-063 | fclose(NULL) aborts because no invalid-parameter handler; C-12 C3 = H-063 (ominaisuusvalitsimet.md:485) | v |
| T-020 | H-078 | H-078 row: abort because _set_invalid_parameter_handler absent; tputilv2/TextFileClass.cpp:166 | v |
| T-020 | H-105 | H-105 row: MSVC invalid-parameter abort on stale handle | v |
| T-020 | H-059 | H-059 row: fclose(NULL) ends MSVC program; Hk/HkAjat.cpp:463-471 | v |
| T-001 | T-002 | core includes GUI headers (Hk/HkDeclare.h:23-28) create the cycles of T-002 | i |
| T-050 | H-172 | viittaukset T-050->H-172; undefined XML macro, JukConsole/VConTul.cpp:1283-1324 | v |
| T-050 | H-177 | viittaukset T-050->H-177; SEURATUL local define, HkConsole/HkCons3.cpp:40-42 | v |
| T-008 | H-081 | H-081 is an instance of console vs GUI switch difference (_BORLAND_ block Hk/HkIV.cpp:1663-1673) | i |
| T-021 | H-003 | T-021: bcc32 does not check format strings, so GUI-file printf errors stay unseen (same class as H-003) | i |
| T-019 | H-148 | no header length/checksum lets N+1 header vs N length mismatch pass undetected (Juk/vdat.cpp:178-191) | i |
| T-003 | H-144 | global state without common lock (about 850 externs, Hk/HkDeclare.h) underlies static buffers of H-144 | i |
| T-003 | H-051 | shared static tarecbuf is one of the globals of T-003 (dbboxm/dbbox1.cpp:20) | i |

### 3.5 korjaus_ratkaisee - fixing A (root T) removes B

| A | B | Evidence | c |
|---|---|---|---|
| T-022 | H-032 | T-022 row: common root cause listing H-032 | v |
| T-022 | H-055 | T-022 row: common root cause listing H-055 | v |
| T-022 | H-060 | T-022 row: common root cause listing H-060 | v |
| T-022 | H-061 | T-022 row: common root cause listing H-061 | v |
| T-022 | H-070 | T-022 row: common root cause listing H-070 | v |
| T-022 | H-071 | T-022 row: common root cause listing H-071 | v |
| T-022 | H-072 | T-022 row: common root cause listing H-072 | v |
| T-022 | H-029 | T-022 row: no authentication / keyed integrity (H-029); fix is a protocol decision, see owner list | v |
| T-038 | H-131 | T-038 row lists H-131 | v |
| T-038 | H-132 | T-038 row lists H-132 | v |
| T-038 | H-133 | T-038 row lists H-133 | v |
| T-038 | H-141 | T-038 row lists H-141 | v |
| T-038 | H-143 | T-038 row lists H-143 | v |
| T-038 | H-134 | H-134 row links T-038 (stage-change state) | v |
| T-039 | H-128 | T-039 row lists H-128 | v |
| T-039 | H-130 | T-039 row lists H-130 | v |
| T-039 | H-135 | T-039 row lists H-135 | v |
| T-039 | H-139 | T-039 row lists H-139 | v |
| T-039 | H-140 | T-039 row lists H-140 | v |
| T-039 | H-129 | idindex nrec not reset is a derived-structure invariant (Hk/HkIx.cpp:1573-1610); H-129 row refs T-039 via H-134 | i |
| T-039 | H-142 | H-142 roll-up of derived-structure defects (nameindex, ntulosix) | i |
| T-040 | H-015 | T-040 row lists H-015; KK-C9-2 | v |
| T-040 | H-145 | T-040 row lists H-145; KK-C9-2 | v |
| T-040 | H-146 | T-040 row lists H-146; KK-C9-2 | v |
| T-040 | H-153 | T-040 row lists H-153; KK-C9-2 | v |
| T-040 | H-147 | viittaukset H-147->T-040; KK-C9-2 | v |
| T-040 | H-148 | viittaukset H-148->T-040; KK-C9-2 | v |
| T-040 | H-156 | viittaukset H-156->T-040; KK-C9-2 | v |
| T-041 | H-147 | T-041 row lists H-147 | v |
| T-041 | H-156 | T-041 row lists H-156 | v |
| T-041 | H-015 | T-041 row lists H-015 | v |
| T-041 | H-146 | viittaukset H-146->T-041 | v |
| T-042 | H-149 | T-042 row lists H-149; KK-C9-1 | v |
| T-042 | H-151 | T-042 row lists H-151; KK-C9-1 | v |
| T-042 | H-152 | T-042 row lists H-152; KK-C9-1 | v |
| T-042 | H-158 | H-158 row: KK-C9-1 root cause T-042 (lstf dangling part) | v |
| T-035 | H-109 | T-035 row lists H-109 (relay record without constructor) | v |
| T-035 | H-112 | T-035 row lists H-112 | v |
| T-036 | H-115 | T-036 row lists H-115 | v |
| T-036 | H-125 | T-036 row lists H-125 | v |
| T-037 | H-113 | T-037 row lists H-113 | v |
| T-037 | H-114 | T-037 row lists H-114 | v |
| T-030 | H-038 | H-038 row refs T-030; T-030 row refs H-038 | v |
| T-030 | H-095 | H-038 row: remaining writer is comajanotto (H-095, T-030) | v |
| T-030 | H-104 | flag-plus-sleep shutdown lifecycle; Hk/HkInit.cpp:3720-3742 | i |
| T-030 | H-167 | shutdown waits by flag and Sleep (Hk/HkInit.cpp:3781-3804) | i |
| T-032 | H-009 | T-032 row: H-009 pattern; H-009 row cbTp/cbTpApu.cpp:467-490 | v |
| T-032 | H-153 | viittaukset H-153->T-032 | v |
| T-032 | H-162 | T-032 cbHk/UnitExport.cpp:426-450 inside H-162 :420-450 | v |
| T-032 | H-021 | queue-full dialog under outb/tall locks (Tp/TpComY.cpp:519-545; T-032 :1133,:1471) | i |
| T-033 | H-018 | viittaukset H-018->T-033 | v |
| T-033 | H-101 | H-101 VCL handler on worker thread (H-018 family) | i |
| T-044 | T-005 | viittaukset T-044->T-005; KK-C10-3 | v |
| T-044 | H-165 | collector/packaging not verified (T-044) is the cause of H-165; KK-C10-3 | v |
| T-048 | H-164 | viittaukset T-048->H-164; KK-C10-2 | v |
| T-048 | H-170 | H-170 item N-11 comajanotto spins if port lost = liveness gap | i |
| T-051 | H-173 | viittaukset T-051->H-173; KK-C12-2 | v |
| T-051 | H-176 | H-176 settings parts in KK-C12-2 with T-051 | v |
| T-052 | H-176 | viittaukset T-052->H-176; ViestiLuenta key selects ALGE | v |
| T-027 | H-005 | BOM-less files read with host ANSI code page (T-027) is the mechanism of H-005; Tp/TpLaitteet.cpp:1548 | i |

### 3.6 korjattava_yhdessa - fix in one change

| A | B | Evidence | c |
|---|---|---|---|
| T-043 | H-166 | both in KK-C10-4 (riippuvuudet.md 19.4) | v |
| T-043 | H-161 | KK-C10-4 includes H-161 injection risk (Q105); KK-C10-1 includes T-043 identifiers | v |
| T-011 | T-049 | T-049 row: relates to T-011; same dead-code owner decision (Q11, Q111) | v |
| T-011 | T-012 | T-012 dead SIIMPORT code lives in Tp/TpComY.cpp; T-011 is the old copy Tp/TpComY32.cpp (Q11) | i |
| H-129 | H-134 | H-134 row and ominaisuusvalitsimet.md 13.3: fixed in the stage-change whole together with H-129; Hk/HkIx.cpp:1600-1610,:1907-1941 | v |
| H-129 | H-102 | same line Hk/HkIx.cpp:1610; one teejarj(true) change touches both | v |
| H-129 | H-128 | dbindex.cpp:51-135 setpos fix and nrec reset change the same index rows (H-129 row) | v |
| H-129 | H-133 | initindex/teejarj nrec reset pattern in both (Hk/HkIx.cpp:101-107,:1573-1610) | v |
| H-134 | H-131 | H-134 row links H-131..H-133 in one stage-change package | v |
| H-134 | H-132 | H-134 row links H-132 (aikaflname) with H-133 | v |
| H-134 | H-133 | H-134 row links H-133 | v |
| H-091 | H-092 | H-092 row: tall->emit third edge closes chain print->tall->emit->print with H-091; one lock hierarchy | v |
| H-092 | H-103 | both are emit-lock ordering in relay (Juk/VEmit.cpp:1409,:1510,:1524) | i |
| H-093 | H-091 | same lock-ordering policy (loki, tcpip, print, tall) | i |
| H-160 | H-161 | riippuvuudet.md 19.3: H-161 fixed with H-160 (KK-C10-1) | v |
| H-160 | H-162 | KK-C10-1: exceptions caught in writer thread and export, lock released on all paths | v |
| H-160 | H-163 | KK-C10-1 | v |
| H-160 | H-169 | KK-C10-1 | v |
| H-160 | H-014 | KK-C10-1 dependency H-014 (riippuvuudet.md 19.4) | v |
| H-160 | H-043 | KK-C10-1 dependency H-043; cbHk/UnitMySQL.cpp:663-719 | v |
| H-160 | H-100 | KK-C10-1 dependency H-100; cbHk/UnitMySQL.cpp:739-752 | v |
| H-159 | H-164 | KK-C10-2 device connection layer | v |
| H-164 | T-048 | KK-C10-2 | v |
| H-138 | H-108 | H-138 row: compare H-108 (shared static thread argument/counter); KK-C10-2 reader threads | v |
| H-159 | H-138 | reconnect logic Tp/TpLaitteet.cpp:1785-1847 and :2155-2188 (reader reopen) | i |
| H-149 | H-150 | ominaisuusvalitsimet.md 13.3 KK-C12-1 groups both; also KK-C9-1/-3 | v |
| H-149 | H-172 | KK-C12-1 XML/HTML/auto files package | v |
| H-034 | H-168 | duplicate-card prevention (Hk/HkEmit.cpp:1627-1666 vs Hk/HkIV.cpp:940-952) | i |
| H-141 | H-168 | duplicate-card prevention state (nemitvar, samatleimat) | i |
| H-025 | H-050 | combufsize stride/header: Tp/TpComY.cpp:509 and cbHk/UnitComfile.cpp:145-176 | i |
| H-011 | H-137 | emit-file/index lock policy (emit_CS) in Hk/HkEmit.cpp:1579,:1600-1688 | i |
| H-078 | H-079 | one TextFl::Open mode-string change (tputilv2/TextFileClass.cpp:347-359) | v |
| H-036 | H-171 | KK-C12-2 settings parsers (Juk/VInit.cpp:1139,:74-78,:652) | v |
| H-171 | H-173 | KK-C12-2 | v |
| H-065 | H-002 | both parse relay class files Juk/vSarjat.cpp:247-285 and :377-400 | i |

### 3.7 ristiriita - rows or priorities that disagree for one mechanism

| A | B | Evidence | c |
|---|---|---|---|
| H-008 | H-110 | same mechanism (lock left held blocks worker threads): H-110 korkea P2, H-008 korkea reproduced in harness but no priority | v |
| H-024 | H-110 | same mechanism, H-024 korkea reproduced, no priority; H-110 P2 | v |
| H-040 | H-087 | relay lock never released: H-087 korkea P2, H-040 korkea reproduced, no priority | v |
| H-014 | H-160 | SQL writer failure: H-160 korkea P2, H-014 korkea (SQL use) no priority | v |
| H-015 | H-145 | write error ignored: H-145 P3, H-015 (KK-C9-2) no priority | v |
| H-044 | H-145 | ack/advance before persistence: H-145 P3, H-044 no priority and not a KK-C9-2 member (only "liittyy") | v |
| H-061 | H-032 | same mechanism: H-032 korkea P1, H-061 keskitaso P1; severity text vs priority | v |
| H-029 | H-071 | no-authentication root (T-022) with network reach: H-071 P1, H-029 keskitaso no priority | i |
| H-055 | H-032 | "urgent fix" list names H-055/H-070 (P2) but not H-032/H-061 (P1); two axes disagree (muistiturvallisuus.md luku 15) | v |
| H-134 | H-129 | "fix together" yet H-134 P3 (C-12) vs H-129 P2 (provisional, C-8) | v |
| H-170 | H-164 | same item (vanished port): H-164 P3, inside roll-up H-170 P5 | i |
| H-156 | H-071 | same FILESEND partial-file item: H-071 P1, inside roll-up H-156 P5 | v |

### 3.8 Duplicates found in reports without an own identifier

| Report item | Register row | Evidence | c |
|---|---|---|---|
| C-12 C3 | H-063 (root T-020) | `ominaisuusvalitsimet.md:485` "C3 on H-063 (T-020)" | [todennettu] |
| C-12 C4 | H-043 (mostly) | `ominaisuusvalitsimet.md:485`; register closure text "C4 ~ H-043" | [todennettu] |
| C-12 B1 | H-036 | task statement and H-036 row (C-12 extension, `Juk/VInit.cpp:1139`) | [todennettu] |
| C-12 H-134 extension | H-134 | `ominaisuusvalitsimet.md` 13.3: part of the H-129 stage-change whole | [todennettu] |

## 4. Candidate chains: one fix resolves many

### 4.1 Root technical debt (T) and the H findings it explains

Taken from relations of type `korjaus_ratkaisee`/`seuraus` with a T as id_a. "Stated" = the T or H row names the link; "inferred" otherwise. Fixing the T means the common mechanism (policy, helper, central reset), not removing every symptom by itself.

| Root T | Explained H (v = stated, i = inferred) | Earlier KK |
|---|---|---|
| T-003 Noin 850 `extern`-globaalia / about 850 extern globals | H-051(i) H-144(i) | - |
| T-008 Konsoli- ja GUI-versioissa eri ominaisuusvalitsimet / differ | H-081(i) | - |
| T-019 Tiedostoissa ei tunnistetta, versiota, tietuepituutta eikä t | H-148(i) | - |
| T-020 Muistinvarauksen tulosta ei tarkisteta (noin 15 `malloc`/`ca | H-059 H-063 H-078 H-105 | - |
| T-021 Varoitushygienia: MSVC `/W3` ilman `/analyze`, bcc32 `-w-par | H-003(i) | - |
| T-022 Ulkoisen syötteen validointi puuttuu järjestelmällisenä käyt | H-029 H-032 H-055 H-060 H-061 H-070 H-071 H-072 | - |
| T-027 Merkistö riippuu käännöskoneen ANSI-koodisivusta: BOMittomat | H-005(i) | - |
| T-030 Säikeiden elinkaari: ei odotusta (join), kahvoja ei käytetä, | H-038 H-095 H-104(i) H-167(i) | - |
| T-032 Estävät ja käyttöliittymää odottavat kutsut lukon hallussa:  | H-009 H-021(i) H-153 H-162 | - |
| T-033 Käyttöliittymäsiirto vain synkronisella `SendMessage`lla; kä | H-018 H-101(i) | - |
| T-035 Viestin `kilptietue`lla ei ole konstruktoria: paikalliset ti | H-109 H-112 | - |
| T-036 Lomakkeiden käsittelijät ottavat `tall_CriticalSection`in il | H-115 H-125 | - |
| T-037 Muokattavia pudotusvalikkoja (`csDropDown`, ei `Style`a dfm: | H-113 H-114 | - |
| T-038 Vaiheriippuvaa tilaa ei nollata keskitetysti: `vaihdavaihe`  | H-131 H-132 H-133 H-134 H-141 H-143 | - |
| T-039 Johdetut rakenteet (laskurit, sijoitusketjut, `idindex`, `na | H-128 H-129(i) H-130 H-135 H-139 H-140 H-142(i) | - |
| T-040 Virheet raportoidaan modaali-ikkunoilla suoraan työsäikeistä | H-015 H-145 H-146 H-147 H-148 H-153 H-156 | KK-C9-2 |
| T-041 Tiedostoja ei kirjoiteta atomisesti eikä kestävästi: julkais | H-015 H-146 H-147 H-156 | KK-C9-2 |
| T-042 `closeprfile` ottaa `PRFILE`-osoittimen arvona eikä voi noll | H-149 H-151 H-152 H-158 | KK-C9-1 |
| T-044 Jakelua ja käännöstä ei todenneta: kerääjä seuraa vain staat | H-165 | KK-C10-3 |
| T-048 Laitteiden elävyyttä ei valvota: TCP-lukijoilla ei keepalive | H-164 H-170(i) | KK-C10-2 |
| T-050 Valitsinten hygienia: 107 koskaan määrittelemätöntä makroa j | H-172 H-177 | KK-C12-3 |
| T-051 Kokoonpanojen ja jäsentimien ajautuminen: `DBGFILE` vain osa | H-173 H-176 | KK-C12-2 |
| T-052 ViestiLuenta on supistettu käännös (`LUENTA`): ilman `LAJUNE | H-176 | KK-C12-3 |

T rows with no explained H found in the register: T-001, T-002, T-004, T-005, T-006, T-007, T-009, T-010, T-011, T-012, T-013, T-014, T-015, T-016, T-017, T-018, T-023, T-024, T-025, T-026, T-028, T-029, T-031, T-034, T-043, T-045, T-046, T-047, T-049. Of these T-005, T-006, T-007, T-009..T-018, T-024 (partly), T-028, T-029, T-045, T-046, T-047, T-049 are hygiene, dead code or owner questions and have no defect child. T-031, T-034 and T-043 have relations of other types (3.2, 3.3, 3.6). T-012/T-049/T-011 belong together (dead-code removal) and T-016/T-023 share `include/tputil.h:27-29`.

### 4.2 H findings that other H findings depend on

| Cause H | Dependent H | Why |
|---|---|---|
| H-016 | H-039(i) H-030(i) H-007(i) | no version/origin lets old state win via another route (H-039 Hk/HkCom32.cpp:186-189) |
| H-018 | H-107 | viittaukset H-018->H-107; H-107 GUI hooks run on reader/pros_bdg_t threads (cbHk/UnitEmit.cpp:966-970) |
| H-029 | H-071(i) H-072(i) | no authentication lets any datagram reach the unvalidated handler (muistiturvallisuus.md luku 15) |
| H-031 | H-094 | H-031 row C-6: every badge creates/destroys etls queue and exposes H-094 race; Hk/HkIV.cpp:796 |
| H-050 | H-025(i) | viewer sets global combufsize (cbHk/UnitComfile.cpp:145-176) used by queue code (Tp/TpComY.cpp:509) |
| H-128 | H-129 | H-129 row: deletions and renumbers (H-128) lower the overflow threshold |
| H-134 | H-105 | H-105 row: cards written to closed handle after the H-134 branch; Hk/HkIx.cpp:1907-1941 |
| H-155 | H-027(i) | viittaukset H-155->H-027; both openportTCP failure paths com/WinUDP.cpp:266-293 vs :812-817 |
| H-160 | H-169 | riippuvuudet.md 19.3.2: mirror incomplete after writer crash makes import destructive |
| H-161 | H-160 H-169 | H-160 row lists H-161 invalid statements as trigger of SQL exception termination; riippuvuudet.md 19.3.1 |
| H-165 | H-163 | riippuvuudet.md 19.3: missing DLLs in release ZIP make H-163 start-up failure likely |

### 4.3 Whole-change groups (fix together)

| Group | Members | Reason |
|---|---|---|
| G1 stage change in HK | H-129, H-134, H-102, H-128, H-131, H-132, H-133, H-141, H-143, T-038, T-039 | one `vaihdavaihe`/`teejarj`/`aloitus` reset design; `ominaisuusvalitsimet.md` 13.3, T-038, T-039 |
| G2 SQL integration | H-160, H-161, H-162, H-163, H-169, H-014, H-043, H-100, H-097 | KK-C10-1 plus dependencies |
| G3 lock hierarchy | H-091, H-092, H-093, H-103, H-087, T-031, T-032, H-009, H-021, H-153 | one documented lock order and no UI/blocking calls under lock |
| G4 PRFILE lifecycle and autofile | H-149, H-150, H-151, H-152, H-158, H-172, T-042 | KK-C9-1/-3 and KK-C12-1 overlap on H-149, H-150 |
| G5 write errors | H-015, H-044, H-145, H-146, H-147, H-148, H-153, H-156, T-040, T-041, T-019 | KK-C9-2; H-044 not a member yet identical mechanism |
| G6 external input validation | H-032, H-061, H-071, H-072, H-055, H-060, H-070, H-029, T-022 | T-022 root; H-029 needs a protocol decision |
| G7 device connection layer | H-159, H-164, H-138, H-108, H-027, H-155, H-056, H-166, T-048 | KK-C10-2 and `riippuvuudet.md` 19.4 |
| G8 TextFl::Open | H-078, H-079, H-086 | same function `tputilv2/TextFileClass.cpp:322-359` |
| G9 settings parsers | H-036, H-171, H-173, H-176, T-051 | KK-C12-2 |
| G10 duplicate-card prevention | H-034, H-141, H-168 | `Hk/HkEmit.cpp:1627-1666`, `Hk/HkIV.cpp:940-952` [paatelty] |
| G11 combufsize / comfile | H-025, H-048, H-050, H-021, H-030, H-044 | `Tp/TpComY.cpp:509` and queue stride [paatelty] |
| G12 fork divergence | T-034, H-017, H-117, H-121, H-069, H-123, H-124, H-126, H-127, H-106 | each fix checked in the counterpart fork (T-034) |

## 5. Findings whose fix needs an owner decision

Source column: where the need is stated. "[paatelty]" = my inference from the row (not stated by an audit report).

| Id | What must be decided | Source | c |
|---|---|---|---|
| H-081 | Is the GUI-only emit-code swap in fitness-orienteering mode (`KUNTOSUUNN`, `_BORLAND_` block) intended, or must HkMaali match? | H-081 row; Q60 (omistajan_paatos) | [todennettu] |
| H-119, H-120 | Meaning of `kuntosuunnmoodi` values 1,2,3,98,99, `esiluenta`, `kuntoLahto` before changing overwrite/placeholder behaviour | Q75 (open) | [todennettu] |
| H-177, T-052, T-050, T-049, T-012, T-013, T-018, T-007, T-011 | Is removal of console club results (`SEURATUL`/`ORAVA`) intentional; does ViestiLuenta need RFID (`LAJUNEN`); may dead switches, dead code and orphan files be deleted (feature removal) | Q111 (omistajan_paatos); `ominaisuusvalitsimet.md` 13.4; Q10, Q11 | [todennettu] |
| T-047 | Licence and provenance of bundled Blaise Computing and Microsoft sample code in a GPLv3 repo | Q101, OM-C10-1; `riippuvuudet.md` 19.5 | [todennettu] |
| H-165, T-044, T-046 | Licence and redistribution rights of `libmysql.dll`/`dbxmys.dll` before they enter the release ZIP; which obsolete packages may be dropped | `riippuvuudet.md` 19.4 KK-C10-3 item 1 | [todennettu] |
| T-043, H-166, H-161 | Security model for external links: TLS/credentials for MySQL, authentication of the hard-coded emiTag server `195.159.103.189:1379`, listener filtering; server maintainer unknown | KK-C10-4; Q99, Q105 | [todennettu] |
| H-029, T-022, H-071, H-072, H-032 | Adding authentication or keyed integrity to the station protocol is a protocol compatibility change; also whether the FILESEND remote file write (`pos == -1`, `-2`) stays as a feature | `muistiturvallisuus.md` luku 15; H-071 row; T-022 row | [paatelty] |
| H-016, H-039, H-030, H-034, H-022, H-023 | Adding version/timestamp/origin to replication packets or ordering rules changes the wire format and mixed-version behaviour | H-016 row; `tiedonsiirto.md` compatibility; Q3, Q18 | [paatelty] |
| H-025, H-048, H-050, H-044 | Fixing the `combufsize` 14-byte header or queue stride changes the disk-queue file layout (existing `COMFILE.DAT` files) | H-025 row; `tiedostomuodot.md` | [paatelty] |
| T-019 | Adding magic, version, record length and checksum to `KILP.DAT` and queue files is a file-format compatibility change (old files, old programs) | T-019 row; Q4, Q27 | [todennettu] |
| H-046 | Relay export writes the raw object; correcting it changes the `.dat` produced for downstream users; how it is used is unknown | H-046 row; Q29 (open) | [todennettu] |
| H-005, H-004, T-027, T-004 | Adding BOM or `/utf-8`, or re-encoding files, changes file encoding which the project rules forbid without decision | H-005 row (Q7); `merkistot.md`; CLAUDE.md editing rules | [todennettu] |
| H-173, H-176, T-051, T-014 | Which key spelling is canonical (`AIKA_COM` vs `AIKACOM`), whether the `UDPVIIVEET` first-value fix and the unit of `AIKAKORJAUS` (HK `SEK/100`, relay `SEK`) may change existing configs; port 14770 vs manual 14900 | H-173, H-176 rows; Q112 (open), Q25 | [todennettu] |
| T-024, T-017, T-051 | Which relay configurations are supported (`NREGNLY` 8 vs 32, `VALUKU` 200/250, non-shipped configurations); changing device-number mapping alters saved/forwarded `kanava` | T-024, T-051 rows; `ominaisuusvalitsimet.md` 5 | [todennettu] |
| H-132 | Whether dropping the 5th character of the AJAT file name in one-stage races is a documented naming convention (docs-site reference in row) | H-132 row evidence `docs-site/...` | [paatelty] |
| H-145, H-168 | Reader acknowledgement semantics (is ACK final; may a duplicate be re-accepted): depends on reader behaviour, not on code alone | Q86 (partly), Q100 | [todennettu] |
| H-160, H-169, H-014 | Architecture principle that MySQL is an optional mirror (failure must not stop service) and that import must not make the mirror master of `KILP.DAT` | KK-C10-1 text in `riippuvuudet.md` 19.4 (proposal) | [todennettu] |
| H-133, H-134 | Whether stage change in a running process (vs restart) is a supported use; decides how much of the reset must be idempotent | Q67, Q78, Q80, Q110 | [todennettu] |
| T-034 | Which fork is authoritative per one-sided fix (history decides, or owner) | Q77 (open) | [todennettu] |

## 6. Inconsistencies in the register

1. **Findings without priority.** 61 open H rows have no priority (`osaA0/yhteenveto.txt` lists 62 ids including refuted H-099): H-004 H-005 H-006 H-007 H-008 H-009 H-010 H-011 H-012 H-013 H-014 H-015 H-016 H-017 H-018 H-019 H-020 H-021 H-022 H-023 H-024 H-025 H-026 H-027 H-028 H-029 H-030 H-031 H-033 H-034 H-035 H-036 H-037 H-038 H-039 H-040 H-043 H-044 H-045 H-046 H-047 H-048 H-050 H-051 H-053 H-054 H-128 H-130 H-131 H-132 H-133 H-135 H-136 H-137 H-138 H-139 H-140 H-141 H-142 H-143 H-144. Pattern: nearly all rows from K1, C-1, C-2, C-3 (except H-032, H-041, H-042, H-049, H-052 and K1 H-001..H-003) and from C-8 (except H-129, H-134) have none. Among them 12 are korkea: H-007 H-008 H-009 H-014 H-022 H-023 H-024 H-025 H-026 H-027 H-031 H-040. Of these, H-007, H-008, H-022..H-027, H-031, H-040 were reproduced in the harness (C-11), while low-severity rows such as H-066 carry P5. [todennettu]
2. **Severity versus priority.** Priority follows reachability x severity (`muistiturvallisuus.md` luku 15), so some mismatches are by design, but they are not explained per row: H-002 korkea P4; H-003 keskitaso P5; H-060 keskitaso P2; H-061 keskitaso P1; H-073 keskitaso P2; H-107 keskitaso P2; H-150 keskitaso P2. H-061 is "keskitaso" but P1 (same class as H-032, korkea, P1). [todennettu]
3. **"Urgent" flag versus P-level.** C-4 marks H-071, H-072, H-055, H-070 "urgent" but H-032 and H-061 are P1 and not urgent; H-055 and H-070 are P2 (`muistiturvallisuus.md` luku 15, register C-4 section). [todennettu]
4. **Fix-package membership.** (a) H-134 row cites KK-C9-2, but the package table `virheenkasittely.md` 13.4 does not list H-134 (lists H-015, H-145..H-148, H-153, H-156; "liittyy H-044"). (b) KK-C10-1 lists T-043 and KK-C10-2 lists H-170 only in `riippuvuudet.md` 19.4, not on the rows (`osaA0/yhteenveto.txt`). (c) Overlapping membership: H-149 (KK-C9-1, KK-C12-1), H-150 (KK-C9-3, KK-C12-1), H-161 (KK-C10-1, KK-C10-4), H-166 (KK-C10-2, KK-C10-4), T-043 (KK-C10-1, KK-C10-4), H-176 (KK-C12-1, KK-C12-2), H-170 (KK-C10-1, KK-C10-2, partial). (d) H-044 is "liittyy" to KK-C9-2 but not a member and has no priority. [todennettu]
5. **Fix together but different priority.** H-134 (P3, C-12) and H-129 (P2 provisional, C-8) are to be fixed in one change; inside KK-C10-1 H-160 is P2, H-161/H-162/H-163/H-169 P3; H-150 (keskitaso) is P2 above H-145/H-146 (keskitaso) P3, with rationale in `virheenkasittely.md` 13.3. [todennettu]
6. **Duplicate recordings.** See sections 3.1 and 3.8: C-12 C3/C4/B1 have no ids; roll-up rows (H-144, H-156, H-170, H-176) repeat items that have their own rows (H-019, H-071, H-164, T-052) at lower priority (P5) than the main row (H-071 P1, H-164 P3). [todennettu]
7. **Roll-up rows hide material items.** H-170 (P5) contains N-13 `sallisummavirhe` (checksum skipped for all readers), D9/D10 (SQL columns) and the comajanotto spin; H-176 (P5) contains `fldn` over-read and `TARKNO=ON` items; H-156/H-157/H-126/H-127/H-142/H-143/H-144 mix severities under one value. [paatelty]
8. **Batch attribution.** H-021: batch range says C-1 (`tallenna-samanaikaisuus.md`), source column says `tiedonsiirto.md` (`osaA0/yhteenveto.txt`). [todennettu]
9. **Provisional or inferred priorities.** H-129 P2 is "alustava"; C-10 and C-12 priorities are labelled [paatelty]; C-12 gave H-173/H-174 (matala-keskitaso) P3 while keskitaso rows such as H-049 and H-063 are P4 (different criteria per batch). [todennettu]
10. **Refuted row.** H-099 stays as a row (kumottu) and is counted among the 62 without priority; excluded from the class inventory. [todennettu]
11. **Conditional severity.** H-032 "korkea viestissa, matala HK:ssa", H-016, H-095, H-014 and others: 23 rows carry a conditional severity (`vakavuus_ehto`), so "highest severity" per class is an upper bound. [todennettu]
12. **Roll-up items not addressable.** H-170 items (K3, N-5, N-7, ...) and H-176 items have no own ids, so a fix package cannot reference a single item. [paatelty]

## 7. Open questions

- The 50 `inferred` relations of 221 need a code check of the named lines; none was opened.
- Class boundaries (BUF_INT vs BUF_* by source for H-025, H-048, H-171; RACE vs LOCK_ORDER for T-033) are judgement calls. [tuntematon]
- Which of the 61 unprioritised H rows (esp. the 12 korkea) get P-levels before fix planning is a planning decision, not made here.
- Reachability of network paths beyond the analysed fields (`muistiturvallisuus.md` luku 15) stays [tuntematon].

