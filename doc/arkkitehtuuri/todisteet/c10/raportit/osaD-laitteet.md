# C-10 osa D / part D: timing devices and device protocols

Agent `architecture-analyst` (Sonnet), 2026-11-26. Report saved by the main session from the
agent's hand-back (claims, evidence and labels unchanged; wording and layout condensed). Static reading only. Paths relative to
`TPsource/V52/` unless they start with `doc/` or `docs-site/`. Labels N-1..N-16 and the open
questions are the agent's; register IDs are assigned in `riippuvuudet.md`.

## Summary

- 27 `LID_*` ids (`include/TpLaitteet.h:17-47`); SPORTIDENT, SIIMPORT, TUOMARI, ALGETUTKA, SW2000, ASC3, RESERV_1 dead; RTNM/CPOWER readers are stubs.
- Transports: serial, TCP client/server, UDP stream, HTTP. Serial `\\.\COM%d`, no handshake, no device enumeration; no SetupDi/USB API (grep `SetupDi`/`CM_Get`/FTDI/`RegisterDeviceNotification`: only print units).
- Sirit/Zebra reconnect checks the return value with inverted polarity → reader thread most likely ends after the first reconnect (N-1).
- A vanished serial reader is never reopened or reported (N-2). Half-open TCP not detected unless `/P` or `SIRITPOLL` (N-3).
- Q86: ACK `"+\r"` is sent when `tall_emit` returns 0, also for dropped duplicates; only the Emit clock family has an ACK; device behaviour without ACK not decidable from the repo.
- `EMITAG` macro (ViestiMaali, ViestiWin) is never tested in any `#if`.

## 1. Device catalogue

Baud index map `com/winCom.cpp:24-26` (4=1200, 5=2400, 6=4800, 7=9600, 8=19200, 9=38400, 11=115200); `start_regnly` table `Tp/TpLaitteet.cpp:1963-2095`. Parsers `Hk/HkInit.cpp`, `Juk/VInit.cpp`, GUI `cbHk/UnitCfg.cpp`, `ViestiWin/UnitCfg.cpp`.

| LID | Device / purpose | Config key | Products / switch | Transport defaults |
|---|---|---|---|---|
| 1 RTR, 2 RTR2 | Regnly finish clock (Emit clock) | `REGNLYx=n/..` HkInit.cpp:1625-1650; VInit.cpp:1514 | all except ViestiLuenta [inferred] | 1200, msg 19/24 (TpLaitteet.cpp:1964-1971) |
| 3 ALGE | ALGE Timer S3 (hkliite1.md:111) | `ALGE=` HkInit.cpp:1836; VInit.cpp:1594-1625 | HK, relay; `ALGE` defined HkInit.cpp:40, VInit.cpp:41 | 2400, 7 data, 2 stop (:1972-1977) |
| 5 ALGE4 | ALGE S4 (:112) | `ALGE4=`, `S4` HkInit.cpp:1833 | same | 4800 (:1982) |
| 4 COMET | ALGE Comet (:113) | `COMET` HkInit.cpp:1813 | same | 2400, msg 22 |
| 6 TIMY, 8 TDC | ALGE Timy; TDC8001 (HkIV.cpp:2109) | `TIMYn=` HkInit.cpp:1818; `TDC` :1830 | same | 9600, msg 27 |
| 9 ASC | ALGE ASC1 start clock | `ASC` HkInit.cpp:1826 | HK only | 2400, msg 19 |
| 22 ARES | ARES timer SOH..EOT (TpLaitteet.h:87-111) | `ARESn` HkInit.cpp:1823 | HK only | 9600 (:2042-2047) |
| 28 RADIOEMIT | radio-link Emit [unknown hardware] | `RADIOEMIT` HkInit.cpp:1841 | HK only | 9600, msg 20 |
| 11 EMITKELLO | Emit clock/reader, 462-byte frame | `EMITKELLO` HkInit.cpp:2271; VInit.cpp:1752 | HK, relay | 4800 (:2012-2015) |
| 12 LUKIJA | Emit reader, XOR 0xDF, 217 bytes | `LUKIJAn=com[:U]`, `TCP:ip:port`, `UDP`, `/P` HkInit.cpp:2292-2325; VInit.cpp:1768 | all five | 9600 or 115200 `/U`; 2 stop (:2016-2023) |
| 13 MTR | Emit MTR, binary pull, 234 bytes | `MTRn=com` HkInit.cpp:2459 | all five | 9600 |
| 14 EMITAG | emiTag ECB1/ETS1/ECU1 (liite_9..md:18-24) | `EMITAGn=com[/U/C]` HkInit.cpp:2327-2350; VInit.cpp:1826 | all; parsed regardless of macro | 9600, `/U` 115200, `/C` 19200 (:2029-2037) |
| 15 ETGPRS, 16 ETSQL | emiTag via Emit server (HTTP or SQL over SSH) (liite_9..md:100-135) | `ETGPRSy`, `ETSQL`, `ETHOST`, `ETHAKUVÄLI`, `ETDATE`, `ETTIME` HkInit.cpp:2373-2450; VInit.cpp:1853-1873 | `EMITHTTP`: all except ViestiLuenta | WinHTTP (part C) |
| 21 SPORTIDENT | SI reader | HkInit.cpp:2479 | never defined | dead |
| 23 CPOWER | C-Power LED board (write-only; stub reader TpLaitteet.cpp:1376-1382) | `CPOWERn=TCP:..` HkInit.cpp:1333; VInit.cpp:992 | sending only GUI (cbHk/UnitTaulu.cpp:166-171, ViestiWin/UnitGAZ.cpp:188) | 115200 or TCP 5200 relay (VInit.cpp:996-997) |
| 24 RTNM | UDP board (stub, :1385-1391) | `RTNMn=UDP..` HkInit.cpp:1325 | GUI | srvport = destport+10 (:2053-2054) |
| 29 VALOT | signal lights via RTS/DTR | `VALOT=` HkInit.cpp:2701 (`LAJUNEN`) | `asetavalo` JukConsole/laskemtn.cpp:248-275 | 9600 (:2077-2081) [inferred purpose] |
| 30 SIRIT | Sirit / Zebra FX9500 RFID, text over TCP | `SIRITn=TCP:..`, `SIRITREUNA`, `SIRITARRIVE/DEPART`, `SIRITEVENTPORT/CMDPORT/POLL/MASK/SYNC/LOKI` HkInit.cpp:1710-1760, 1847-1860; VInit.cpp:1544-1590 | `LAJUNEN`: all except ViestiLuenta | event 50008, command 50007 (TpLaitteet.cpp:105-106) |
| 34 ZEBRA | Zebra FX9600, LLRP (Tp/ZebraReader.cpp:1-20; doc/fx9600-parametrit.md) | `ZEBRAn=TCP:ip[:port]`, `ZEBRADEPART/CLEANUP/GPI` HkInit.cpp:1763-1790, 1863; VInit.cpp:1587, 1616 | `LAJUNEN` | 5084 (ZebraReader.cpp:41) |
| 31 FEIG | FEIG HF/UHF reader | `FEIG` HkInit.cpp:1847 | `LAJUNEN` | 38400 parity e (:2069-2073) |
| 33 IMPINJ | Impinj CSV "antenna,epc,time" (HkIV.cpp:2468-2495) | `IMPINJ` HkInit.cpp:1869 | `LAJUNEN` | baud 0 in table, TCP in practice [inferred] |
| 40 KURVINEN | shooting penalty serial | `SAKKO_YHT/COM` HkInit.cpp:2139 | `SAKKOCOM` (HkDef.h:125) | `sak_baud` (:2089-2094) |
| 19, 7, 36, 99, 10, 32 | judge, ALGE Tutka, SW2000, SI import, ASC3, reserved | `#ifdef MAKI/TUTKA/SWIM2000/SIIMPORT` | not compiled (`MAKI` `#undef` HkDef.h:33-34) | dead |
| (GAZ) | GAZ results board, serial write-only | `GAZ=n`, `GAZVAIHE`, `GAZRIVIy` HkInit.cpp:1376-1392 | `GAZ`: HkMaali, ViestiMaali, ViestiWin | 2400 8N1 (HkIV.cpp:3137; ViestiWin/UnitGAZ.cpp:233-236) |
| (ALGETAULU) | scoreboard serial/UDP | `TAULU_COM`, `TAULU_BAUD` VInit.cpp:1009-1035 | ViestiMaali, ViestiWin | UDP 51702 (VInit.cpp:566; HkInit.cpp:463) |
| (SjBox) | serial-over-IP for Emit reader (5.2_...md:15-30) | `LUKIJAn=TCP:`/`UDP` | `TCPLUKIJA` all five | |
| (AIKA_COM) | CTS pulse → PC-clock timestamp | `AIKA_COM`, `AIKA_ESTO` VInit.cpp:1473 | all except ViestiLuenta (`NOCOM_AIKA`) | 9600 8N1 (TpLaitteet.cpp:2415-2421) |

Notes: `LID_SWIM2000` (HkIV.cpp:2129) not in the header (`LID_SW2000`), dead; `lue_SI` (TpLaitteet.cpp:1396-1486) uses undeclared `msg`, `od`; `lue_regnly` list :1584 omits 36 while :1620, :1658 include it; model names only in the manual.

## 2. Transport, framing, ACK, timeouts

Generic: one `lue_regnlyThread` per reader (TpLaitteet.cpp:1747-1875); serial via `opencomRS` (TpComY.cpp:671-695), UDP `openlukijaUDP`, TCP `openlukijaTCP`; loop 20 ms (:1867-1869), Sirit 50 ms (:1832), Zebra 20 ms (:1850). Serial: `CreateFileW("\\.\COM%d")` (winCom.cpp:114), `BuildCommDCBW` (:73), no CTS/DSR (:83-84), RTS/DTR asserted (:85-86), no XON/XOFF (:87), non-blocking timeouts (:98-103); open failure → error, no retry (winCom.cpp:122-126; TpComY.cpp:675-681). `quesize` error on `ClearCommError` failure or flags (:151-154) → `lue_*` clears buffer (LUKIJA :1011-1017, MTR :1175-1180, EMITAG :1342-1347). TCP: blocking `connect()` without timeout (WinUDP.cpp:775-776); blocking `recv()`, thread ends on `recv<=0` unless `WSAETIMEDOUT` (:575-581); no keepalive/receive timeout/nodelay (only UDP broadcast `setsockopt` :982, :996); reconnect once per 15 s (`RCWAIT` :836-846) if `reconnect` (:735; forced off for Sirit/Zebra TpLaitteet.cpp:2220-2221, ZebraReader.cpp:925); send timeout 10 s (WinUDP.cpp:46, :900); `lastmsg` written, never read (:585, :927, :1082, :1118, :1209); initial open retry 5 s / 55 s / 180 s with shared counter (TpComY.cpp:2121-2186; H-138). Console "down" colour only for TCP readers (HkCons0.cpp:314-316).

- **RTR1/RTR2/EMITKELLO** (`lue_EmitKello` TpLaitteet.cpp:856-939): request/ACK (section 4); RTR1 check `s1/s2/cr` (HkIV.cpp:2072-2076), RTR2 7-bit sum (:2085-2090), EMITKELLO only `tunnus` K/L and `cr==13` (:965-969), length 462 (TpLaitteet.cpp:2014); PC re-sends `"?\r"` at most every `pyyntoviive` 200 ms (:933); `MAX_R_OD` 10 (:924).
- **ALGE/COMET/TIMY/TDC/ASC/RADIOEMIT** (generic line reader :1584-1695): CR/LF lines, partial lines > 10 ticks discarded (:1570-1575, :1689-1694), NUL skipped (:1620), > 28 (TIMY 33) bytes dropped (:1644-1647); length minimums (:1672-1676); no checksum; time = device time + `t0_regn` (:1940-1958, `AIKAERO`); no ACK.
- **ARES** (`lue_ARES` :696-736): SOH, 41 bytes, EOT at 19/40 (:711-715), content checks (HkIV.cpp:2378-2381), no CRC; H-056.
- **LUKIJA** (:948-1122): 217 bytes, header 0x2020, two checksums `r_tarksum` (:1029-1031, :1061-1062), XOR 0xDF; no ACK; duplicates by `vahvistus` and 6-tick gate (:1054, :1061); `lukutoisto = 2` (HkGlobals.cpp:524; :1085-1091); TCP `/P<s>` sends NAK 0x15 as poll (:969-979).
- **MTR** (:1126-1295): 234 bytes, header 0xFFFFFFFF, length 230, ID `M` (:1186-1190), checksum `r13.chksum2` (:1192-1194); `MAXNLEIMA` 50 (HkDef.h:1481-1507, :136); pull `"/GB"`+4-byte package (:1282-1292); `"/ST"` 59 bytes (:49; :2666-2686).
- **EMITAG** (:1299-1372, `tulk_emiTag` :290-500): STX..ETX, tab-delimited letter fields, no checksum, > 15 bytes (:1363); commands `/SC`, `/ST`, `/PP`, `/CD`, `/QF`, `/QC`, `/QM`, `/QD`, `/CL` (:2549-2625).
- **ETGPRS/ETSQL**: see part C.
- **SIRIT** (:741-801, :2213-2330): two TCP connections; wait 2.5 s; banner `event.connection id = N`, 10<N<99 (:2277-2283); `tag.reporting.arrive_fields`, `reader.register_event` (:2296-2308); fixed offsets (:617-644); line > 200 dropped (:1645); `SiritPoll` → `sync(false)` `info.time`, read after 2 s (:1821-1829); "Sirit ei vastaa" only via `lue_SiritCmd` (:2147-2174), generic path off with `LAJUNENxx` (:1586); `SIRITSYNC` sets reader clock (:2200-2204).
- **ZEBRA** (ZebraReader.cpp:921-1073): LLRP v1 port 5084, 10-byte header, `mlen<10 || mlen>RXSZ` clears buffer (:1019-1024); DELETE_ROSPEC, SET_READER_CONFIG, ADD_ROSPEC, ENABLE_ROSPEC, GPI query (:310-420); ACKs KEEPALIVE only (:1030-1033); GPO1 500 ms (:96); tag time reader UTC (:18-20); `sync`, `readCmd` empty (:1055-1064).
- **FEIG** (:805-851): flush, fixed request, `Sleep(40)`, read; CRC-16 0x8408 verified (:674-691, :832); clear frame after a good read (:845).
- **GAZ/boards/C-Power**: write-only; GAZ 23 chars + CR (HkIV.cpp:3260-3275); C-Power 16-bit sum (ViestiWin/UnitGAZ.cpp:996-1016), serial wrapper 0xA5..0xAE (:861-872); write errors ignored (HkIV.cpp:3205).
- **AIKA_COM**: `WaitCommEvent(EV_CTS)` → `tee_aika(1)`, debounce `50*aika_esto` ms (TpLaitteet.cpp:2424-2431).

## 3. Program differences

| Topic | HkMaali | ViestiMaali | ViestiLuenta | HkKisaWin | ViestiWin |
|---|---|---|---|---|---|
| `LAJUNEN` | yes | yes | no | yes | yes |
| `TCPLUKIJA` | yes | yes | yes | yes | yes |
| `EMITHTTP` | yes | yes | no | yes | yes |
| `EMITAG` macro | no | yes | no | no | yes |
| `GAZ` | yes | yes | no | no (`GAZCOM` only) | yes |
| `ALGETAULU` | no | yes | no | no | yes |
| `SAKKOCOM` | yes | no | no | yes | no |
| `NREGNLY` | 32 | 8 | 8 | 32 | 32 |

Sources: makrot.csv; HkMaali.vcxproj:154, JukMaali520.vcxproj:135, JukLuenta520.vcxproj:95-131, HkKisaWin.cbproj:180, ViestiWin.cbproj:195; HkDef.h:85-86, VDef.h:54-55 (T-024). `EMITAG` not tested by any `#if` [verified]. Relay device keys not numbered (`regnly[0]`, VInit.cpp:1594-1625); no ARES/ASC/TDC/RADIOEMIT/SW2000 keys; relay Sirit ports only element 0 (VInit.cpp:1575-1586) vs HK `SIRIT<n>` (HkInit.cpp:1741-1754). `siritaika` writes `ut->lahde` in relay, `kanava/tapahtuma` in HK (TpLaitteet.cpp:631-661). ViestiLuenta: no Sirit/Zebra/FEIG/ETGPRS, `NOCOM_AIKA`, `siritaika` compiled out by `LUENTA` (:574). Console menus `aseta_emitag`/`aseta_mtr` `#ifdef _CONSOLE` (TpLaitteet.cpp:2506+); GUI cbTp/UnitEmiTag.cpp, UnitMTR.cpp, UnitMaalikello.cpp. Code map: readers TpLaitteet.cpp :1515-1743, thread :1747-1875, start :1879-2126, Sirit :2147-2330, `Tp/ZebraReader.cpp`, `Tp/SiritReader.cpp:30-92`, `com/winCom.cpp`, `com/WinUDP.cpp`, `com/gethttp.cpp`, `Hk/HkIV.cpp` (`tall_emit` :955-1150, `tall_regnly` :1984+), `Juk/VIv.cpp` (:947, :1762), `ViestiWin/UnitGAZ.cpp`, `cbHk/UnitTaulu.cpp`, `JukConsole/lasknum5.cpp`.

## 4. Q86 (Emit clock ACK)

`lue_EmitKello` (TpLaitteet.cpp:856-939): request `"?\r"`, ACK `"+\r"` (:858); `"T\r"` when `kerro_regnly` (:926-927). Read up to `r_msg_len - nmsg` (:864); nothing/error → `?` (:873-876); one byte → wait (:877-879); `'-'`+CR → `+` (:882-885); `tunnus` in A–G, J, K, L: short frame with CR → `?`, else `tall_emit`/`tall_regnly`, 0 → `+`, other → `?` (:890-918); other first byte → `?` (:920-922). Before a send `nmsg`, `od` zeroed and input flushed (:928-932); write only if ACK or 200 ms passed (:933-936).

`tall_emit` (HkIV.cpp:955-1150) returns 0 for: (1) normal store `addem(&em,0)` (:1146-1149) → `putem`, flush only with `kirjheti&4` (HkEmit.cpp:1657-1690) (H-145); (2) duplicate by `samatleimat()` (HkIV.cpp:940-952, :1145) — `samatleimat` compares `em1->ctrltime[i] != em1->ctrltime[i]` (always false), so times are never compared (:947-948) [verified]; (3) `addem` refusing a duplicate (`estaemittoisto && on_emit`, HkEmit.cpp:1666-1667), result ignored (:1149); (4) `emitTunnistus` mode without storing (:1130-1133); (5) EMITAG `kaikki_ajat==2` missing end code (:1067-1081). Returns 1 only for a non-K/L frame (:965-969) or `malloc` failure (:960-963).

Implied [inferred]: the device keeps or resends the card until `+`; no resend counter, no "already read" flag. LUKIJA streams without ACK (duplicates by `lukutoisto`/`vahvistus`, :1085-1109). MTR: `pkg_alku` advances after `tall_emit` regardless of return (:1247-1256); manual re-request (6.10_...md:11-22); MTR keeps memory (liite_9..md:68-80). Not decidable without device docs: resend on `?`, meaning of `-`, checksum in the 462-byte frame (`r11.loppu[4]`, HkDef.h:1479, unused), whether re-reading restores a lost card.

## 5. Firmware/protocol assumptions

| # | Assumption | Evidence |
|---|---|---|
| A1 | Sirit text offsets `p+19`/`p+8` (tag), `p+17` time `HH:MM:SS.mmm` | TpLaitteet.cpp:617-644, 112-113; HkInit.cpp:1735-1744 |
| A2 | Sirit banner 10<N<99 | TpLaitteet.cpp:2277-2283 |
| A3 | Zebra LLRP v1, fixed type numbers, "llrp-1x0-def.xml"; reader clock NTP-synced | ZebraReader.cpp:36-82, 126, :18-20; doc/fx9600-parametrit.md §4 |
| A4 | Sirit = FX9500 naming | IRfidReader.h:3-5 |
| A5 | fixed frame sizes RTR 19, RTR2 24, COMET 22, TIMY/TDC 27, ASC 19, EMITKELLO 462, LUKIJA 217, MTR 234, ARES 41, FEIG 29, IMPINJ 30+ | TpLaitteet.cpp:1964-2076; HkDef.h:1449-1507 |
| A6 | card layout bound to `MAXNLEIMA=50` | HkDef.h:136, 1481-1507; TpLaitteet.cpp:2022, 2026 |
| A7 | emiTag firmware mode 0; reader codes 240, 243, 244, 250-253 | TpLaitteet.cpp:2515-2516, :481; HkIV.cpp:1073-1081 |
| A8 | ARES SOH/EOT layout | TpLaitteet.cpp:711-715; TpLaitteet.h:87-111 |
| A9 | FEIG fixed command bytes, CRC 0x8408; model [unknown] | TpLaitteet.cpp:808-809, 670-671 |
| A10 | ALGE S3 20-byte frames shifted 14 | TpLaitteet.cpp:1681-1682 |
| A11 | model names only in manual | hkliite1.md:110-115; liite_9..md:18-24; 5.2_...md:15-30 |
| A12 | day offset manual, device restart not detected [inferred] | TpLaitteet.cpp:1940-1961 |
| A13 | IMPINJ text from external bridge [unknown] | HkIV.cpp:2468-2495 |

## 6. Modernisation risks

- COM: `\\.\COM%d` works above COM9 (winCom.cpp:114); `GAZ=<one digit>` (HkInit.cpp:1378), `gaz_com > 8` return (HkIV.cpp:3134); relay scoreboard `taulu_com` 9..64 opened as COM but written/closed as file (JukConsole/lasknum5.cpp:55-63, :102, :215).
- Baud: fixed per device; `KELLOBAUD` indexes 2..9 max 38400 (TpLaitteet.cpp:2096-2098; HkInit.cpp:1905-1914); `/U`, `/C` only for LUKIJA, EMITAG; `TAULU_BAUD`, GAZ forced 2400 (lasknum5.cpp:59; UnitGAZ.cpp:233; HkIV.cpp:3137).
- No handshake; RTS/DTR asserted (winCom.cpp:85-86); signal lights tied to it.
- USB-serial: no arrival/removal events, re-plug with new COM number needs manual re-open (6.10_...md:18-25).
- IP/ports: only device-side literal `195.159.103.189:1379`; `255.255.255.255` (TpComY.cpp:426); placeholders; `127.0.0.1` tests only; MySQL localhost; ports 5084, 50008/50007, 5200, 51702, 53306, RTNM destport+10.
- Unencrypted links (HkIV.cpp:3540).
- Firmware changes affect A1, A2, frame sizes and emiTag letters; FX9500→FX9600 needed a parallel class; serial→TCP gateway loses serial error reporting and needs `/P`.

## 7. Candidates (agent labels)

| ID | Sev. | Mechanism | Evidence | Products | Consequence | Conf. |
|---|---|---|---|---|---|---|
| N-1 | high | Sirit/Zebra reconnect: `if (!openConnection(r_no, true)) regnly_open[r_no] = 0;` while 0 = success (initial open `if (openConnection(...))` :1768, :1776; `openSirit2` returns 0 on success :2242, :2262, :2329; `ZebraReader::openConnection` 0 on success ZebraReader.cpp:1001; `IRfidReader.h:49`); loop ends on `regnly_open==0` with "Häiriö...jatko vaatii uudelleenkäynnistyksen" (:1872-1874); original 2015 (`50db2f8`), kept by `3238024`; manual reconnect uses same branch (HkCons0.cpp:923; JukConsole/laskent0.cpp:750; cbHk/UnitAjanotto.cpp:1622) | TpLaitteet.cpp:1805-1819, :1835-1847, :1872-1874 | HkMaali, ViestiMaali, HkKisaWin, ViestiWin | after reader power cycle, cable pull or manual reconnect the thread ends; no tags until restart | [inferred] high |
| N-2 | high | vanished serial reader never reopened or reported (TpComY.cpp:675-681; TpLaitteet.cpp:1011-1017, :1175-1180, :1342-1347, :1866-1869; HkCons0.cpp:314) | winCom.cpp:144-157; TpLaitteet.cpp:1856-1870 | all five | unplugged USB-serial silently stops reading | [inferred] medium-high |
| N-3 | medium | no half-open TCP detection; `lastmsg` unread; Zebra KEEPALIVE not tracked (ZebraReader.cpp:1030-1033); only `SIRITPOLL` (:1825-1829) or `/P` (:969-979) | WinUDP.cpp:546-640 | TCP readers | silent "connected" reader | [inferred] |
| N-4 | medium | `tulk_emiTag` `i_pv` uninitialised (TpLaitteet.cpp:295); `ec_strtoaika` may not write it (:264-277); added as days (:384-387, :402-404); no checksum | TpLaitteet.cpp:257-285, :383-404 | all | garbled time field → punch shifted by random days | [inferred] |
| N-5 | medium | `lue_MTR` waits for a stable queue without timeout | TpLaitteet.cpp:1284-1292 | MTR | thread stalls while device streams | [inferred] low-medium |
| N-6 | medium | `pkg_alku` advanced regardless of `tall_emit` result | TpLaitteet.cpp:1247-1256 | MTR | failed card not re-requested (with H-145) | [verified] code |
| N-7 | low | `static INT pyyntoviive = 200` (TpLaitteet.cpp:117) hides the global (HkDeclare.h:836, HkGlobals.cpp:466); `REGNLYVIIVE=` (HkInit.cpp:1595) has no effect | TpLaitteet.cpp:117, :933 | HK | request interval not tunable | [inferred] |
| N-8 | low | `regnly >= LID_LUKIJA || regnly <= LID_EMITAG` always true (:1910); `od_regnly` never read | TpLaitteet.cpp:1910-1921 | all | GUI "Lue" checked for time-only devices; config value ignored | [verified] |
| N-9 | low | `SiritEventPort/CmdPort` init element 0 only (:105-106), copied over configured port (:2065-2067) | TpLaitteet.cpp:2063-2068; HkInit.cpp:1741-1754 | HK LAJUNEN | second Sirit reader cannot connect without extra keys | [verified] |
| N-10 | low | HTTP fetch issues (= part C) | gethttp.cpp:28-120 | 4 | truncated lines | [inferred] |
| N-11 | low | `comajanotto` `WaitCommEvent` without sleep/error handling, spins if port vanishes | TpLaitteet.cpp:2423-2433 | AIKA_COM | CPU spin, no time source | [inferred] |
| N-12 | low | RTR2 checksum uses `r_msg_len[0]` (HkIV.cpp:2085) | HkIV.cpp:2085 | HK | latent | [verified] |
| N-13 | low | `sallisummavirhe` disables `r_tarksum` for every reader (TpLaitteet.cpp:246-248; cbHk/UnitEmit.cpp:3553; ViestiWin/UnitEmit.cpp:3018) | TpLaitteet.cpp:246-248 | GUI | corrupt frames accepted if left on | [verified] code |
| N-14 | low | relay console scoreboard open/write/close inconsistency; GAZ ≥ COM9 impossible | lasknum5.cpp:55-63, :102, :215; HkInit.cpp:1378; HkIV.cpp:3134 | ViestiMaali, HkMaali | display gets nothing | [verified] |
| N-15 | low | FEIG fixed 40 ms wait, flush before request | TpLaitteet.cpp:812-847 | LAJUNEN | missed/repeated tags | [inferred] |
| N-16 | info | dead device code (`lue_SI`, `LID_SWIM2000`, stubs, `LAJUNENxx`) | as cited | all | none | [verified] |

Open question candidates: (1) does a successful Sirit/Zebra reconnect end the thread (N-1); (2) Emit clock behaviour on `?`, no ACK, late `+`, checksum in 462-byte frame; (3) does MTR/emiTag firmware mark a card sent on `/GB`; (4) IMPINJ CSV origin, FEIG model and command 0x32; (5) does the compiler accept `extern` + `static pyyntoviive` (N-7); (6) owner and availability of 195.159.103.189:1379; (7) terminal-server answer to the `/P` NAK poll; (8) replacement for Sirit no-response detection.
