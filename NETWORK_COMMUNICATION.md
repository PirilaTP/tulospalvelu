# Network Communication Protocol Documentation

This document describes the network communication protocol used between Tulospalvelu instances. The protocol was designed by Pekka Pirilä for real-time sports timing data synchronization between multiple computers at competition venues.

## Table of Contents

- [Architecture Overview](#architecture-overview)
- [Transport Layers](#transport-layers)
  - [UDP (Primary)](#udp-primary)
  - [TCP (Alternative)](#tcp-alternative)
  - [UDP Broadcast](#udp-broadcast)
  - [Serial RS-232 (Legacy)](#serial-rs-232-legacy)
- [Frame Format](#frame-format)
  - [UDP Wrapper](#udp-wrapper)
  - [Protocol Header](#protocol-header)
  - [Checksum](#checksum)
- [ACK/NAK Protocol](#acknak-protocol)
- [Message Types](#message-types)
  - [ALKUT (0) - Handshake](#alkut-0---handshake)
  - [KILPT (1) - Full Competitor Record](#kilpt-1---full-competitor-record)
  - [KILPPVT (2) - Competitor Stage Data](#kilppvt-2---competitor-stage-data)
  - [VAIN_TULOST (3) - Time Result Only](#vain_tulost-3---time-result-only)
  - [AIKAT (4) - Timer Data](#aikat-4---timer-data)
  - [EMITT (6) - EMIT Punch Data](#emitt-6---emit-punch-data)
  - [SEURAT (7) - Club Data](#seurat-7---club-data)
  - [FILESEND (10) - File Transfer](#filesend-10---file-transfer)
  - [EMITVA (13) - EMIT Extended Data](#emitva-13---emit-extended-data)
  - [EXTRA (14) - Control Commands](#extra-14---control-commands)
- [Retry and Timeout Mechanisms](#retry-and-timeout-mechanisms)
- [Message Queuing](#message-queuing)
- [Connection Lifecycle](#connection-lifecycle)
- [TCP vs UDP Analysis](#tcp-vs-udp-analysis)
- [Java Implementation Status](#java-implementation-status)
- [Configuration (laskenta.cfg)](#configuration-laskentacfg)

---

## Architecture Overview

Tulospalvelu uses a peer-to-peer network where multiple competition computers synchronize data. Each machine has a 2-character identifier (e.g., "J1", "M1") and can both send and receive messages. The primary use case is synchronizing competitor data, timing results, and EMIT electronic punch card readings between:

- **Maali** (finish line) computers
- **Lähtö** (start) computers
- **Väliaikakone** (split time) computers
- **Tulospalvelin** (results server)

Communication types are defined in `TPsource/V52/include/wincom.h:24-40`:

```
ipNONE       = 0    No connection
ipUDPBOTH    = 1    Bidirectional UDP
ipUDPSTREAM  = 2    Streaming UDP (no ACK)
ipUDPCLIENT  = 3    UDP client only
ipUDPSERVER  = 4    UDP server only
ipTCPCLIENT  = 11   TCP client
ipTCPSERVER  = 12   TCP server
```

Higher-level communication type constants (`TPsource/V52/include/wincom.h:32-40`):

```
comtpRS        = 0    Serial RS-232
comtpUDP       = 1    Standard UDP with ACK/NAK
comtpUDPSTREAM = 5    Streaming UDP (fire-and-forget)
comtpTCP       = 2    TCP binary (same frame format as UDP)
comtpTCPSRV    = 6    TCP server (binary)
comtpTCPXML    = 10   TCP with XML messages
comtpTCPSRVXML = 22   TCP server with XML messages
comtpTCPLOKI   = 14   TCP for time/clock service
comtpTCPIMPORT = 18   TCP for data import
```

---

## Transport Layers

### UDP (Primary)

The main transport. Implements reliability on top of UDP with ACK/NAK, checksums, packet IDs, and retransmission.

**Implementation**: `TPsource/V52/com/WinUDP.cpp` (lines 289-304 for socket creation, lines 1095-1400 for read/write)

Key functions:
| Function | File:Line | Purpose |
|----------|-----------|---------|
| `openportUDP()` | `WinUDP.cpp:289` | Create UDP socket pair (server + client) |
| `read_UDP()` | `WinUDP.cpp:1129` | Read with STX frame unwrapping |
| `read_UDPcli()` | `WinUDP.cpp:1216` | Client-side read with fd_set polling |
| `wrt_st_UDP()` | `WinUDP.cpp:1286` | Write with STX header (client) |
| `wrt_st_UDPsrv()` | `WinUDP.cpp:1339` | Write with header (server) |

**Why UDP?** The system originates from the 1980s when it ran over RS-232 serial links. The UDP transport was added later as a network alternative, reusing the same reliable-delivery protocol (SOH framing, ACK/NAK, checksums) that was already proven over serial. Since the application layer already handles reliability, raw UDP was sufficient — and simpler to implement in the peer-to-peer topology where machines both send and receive.

**"Unconfirmed" UDP mode**: The `UDPvarmistamaton` flag (`TpComY32.cpp:75`) supports fire-and-forget UDP where ACK is auto-generated locally without waiting for the remote side. Bit 1 = skip send-side ACK wait, Bit 2 = skip receive-side ACK requirement. Configured via `laskenta.cfg`.

### TCP (Alternative)

A fully implemented but less-documented TCP transport exists alongside UDP. It shares the same codebase file (`WinUDP.cpp`).

**Implementation**: `TPsource/V52/com/WinUDP.cpp` (lines 334-963)

Key functions:
| Function | File:Line | Purpose |
|----------|-----------|---------|
| `openportTCP()` | `WinUDP.cpp:707` | Open TCP connection (client or server) |
| `reconnectTCP()` | `WinUDP.cpp:841` | Reconnect broken TCP connections |
| `closeportTCP()` | `WinUDP.cpp:334` | Close TCP connection |
| `TCPyht_on()` | `WinUDP.cpp:413` | Check if connection is active |
| `read_TCP()` | `WinUDP.cpp:453` | Read from TCP buffer (destructive) |
| `peek_TCP()` | `WinUDP.cpp:496` | Peek at TCP data without removing |
| `wrt_st_TCP()` | `WinUDP.cpp:898` | Write buffer to TCP |
| `wrt_ch_TCP()` | `WinUDP.cpp:958` | Write single character to TCP |
| `TCPrcvTh()` | `WinUDP.cpp:548` | Background receive thread |
| `TCPacceptTh()` | `WinUDP.cpp:647` | Server accept thread |

**Two TCP modes**:

1. **Binary TCP** (`comtpTCP`/`comtpTCPSRV`): Uses the **same SOH frame format** as UDP. The same `lahetapaketti()` function in `TpComY32.cpp:1459` handles both — it writes the SOH+id+iid+class+len+checksum+data frame, then for TCP adds a `Sleep(TCPviive_lah[cn])` delay instead of ACK polling (`TpComY32.cpp:1467-1468`). TCP's stream reliability replaces the need for application-level ACK/NAK.

2. **XML TCP** (`comtpTCPXML`/`comtpTCPSRVXML`): Sends competition data as XML messages framed with STX (0x02) and ETX (0x03). Implemented in `HkCom32.cpp:972-1078` (`lahetaXML_TCP()`). This mode converts KILPT and VAIN_TULOST messages to XML format and is used for integration with external systems. Conditional on `#ifdef TCPSIIRTO`.

**Buffer sizes** (`WinUDP.cpp`):
- `TCPBUFSIZE`: 2040 bytes (send buffer)
- `TCPRCVBUFSIZE`: 40000 bytes (receive buffer)
- `TCP_RD_TIMEOUT`: 5ms
- `TCP_SND_TIMEOUT`: 10ms

### UDP Broadcast

One-way broadcast for real-time results distribution. No ACK/NAK.

**Implementation**: `WinUDP.cpp:358` (`openportBroadcast()`), `WinUDP.cpp:1387` (`wrt_st_broadcast()`)

Used by `broadcasttulos()` in `HkCom32.cpp:143-153` to send a simple text line:
```
KT:MSGNO:KILPNO:VAIHE:PISTE:AIKA\r\n
```
Example: `J1:00042:0123:1:3:01:23:45\r\n`

### Serial RS-232 (Legacy)

The original transport from 1986. Still supported via `comtpRS`. Uses the same SOH framing and ACK/NAK protocol.

---

## Frame Format

### UDP Wrapper

Added on top of the protocol message for UDP transport (`WinUDP.cpp:1286-1305`).

**Client-side send (wrt_st_UDP)**:
```
Offset  Size  Field         Description
0       1     STX (0x02)    Start marker
1       2     port (LE)     Sender's server port (for replies)
3       2     machineId     2-char machine identifier (konetunn)
5       N     payload       Protocol message (SOH frame)
```

**Server-side send (wrt_st_UDPsrv)** — 4-byte wrapper without STX:
```
Offset  Size  Field         Description
0       2     port          Reserved/port field
2       2     machineId     2-char machine identifier
4       N     payload       Protocol message or ACK/NAK
```

### Protocol Header

The inner message frame, shared across all transports (`TpComY32.cpp:1450-1459`, `HkDef.h:1240-1318`).

```
Offset  Size  Field       Description
0       1     SOH (0x01)  Start of heading
1       1     id          Packet ID (1-255, wraps 255→1, skipping 0)
2       1     iid         Inverted ID (255 - id), for validation
3       1     pkgclass    Message type (0-14, see Message Types)
4       2     len (LE)    Payload data length
6       2     checksum    16-bit LE checksum of payload
8       N     data        Payload (structure depends on pkgclass)
```

Full C struct definition: `HkDef.h:1240-1318` (`combufrec`)

### Checksum

Simple 16-bit sum of all payload bytes, treating data as little-endian 16-bit words. Odd trailing byte added as-is.

**C++ implementation** (`TpComY32.cpp:604-618`):
```c
unsigned short chksum(char *bufptr, int len) {
    unsigned short *bptr = (unsigned short *) bufptr;
    unsigned short sum = 0;
    for (int i = 0; i < len/2; i++) sum += *(bptr++);
    if (len % 2) sum += *((char *) bptr);
    return sum;
}
```

Applied at send time: `obuf[cn].checksum = chksum((char *)&obuf[cn].d, class_len[obuf[cn].pkgclass])` (`TpComY32.cpp:1453-1454`)

Validated at receive time: `chksum((char *)&cbuf->d, cbuf->len) - cbuf->checksum` must equal 0 (`TpComY32.cpp:1019`)

---

## ACK/NAK Protocol

### ACK Response

Sent after successful packet reception (`TpComY32.cpp:855-859`):
```
Offset  Size  Field       Description
0       1     ACK (0x06)  Acknowledgement marker
1       1     id          Original packet's ID
2       1     255 - id    Inverted ID (validation)
3       1     id          Repeated packet ID
```

### NAK Response

Sent when packet validation fails (`TpComY32.cpp:862`):
```
Offset  Size  Field       Description
0       1     NAK (0x15)  Negative acknowledgement
```

### Validation Rules

1. **Packet ID check**: `id[0] + id[1] == 255` AND `id[0] == id[2]` (`TpComY32.cpp:1526-1527`)
2. **Checksum check**: computed checksum must match header checksum (`TpComY32.cpp:1019`)
3. **Duplicate detection**: packet ID matches last received → ACK but don't re-process

---

## Message Types

All message type constants defined in `HkCom32.cpp:47-57`. Payload sizes initialized in `init_class_len()` (`HkCom32.cpp:1082-1106`).

```c
#define ALKUT       0    // Handshake
#define KILPT       1    // Full competitor record
#define KILPPVT     2    // Competitor stage data
#define VAIN_TULOST 3    // Time result only
#define AIKAT       4    // Timer departure/finish times
#define EMITT       6    // EMIT punch card data
#define SEURAT      7    // Club/organization data
#define FILESEND   10    // File transfer segments
#define EMITVA     13    // EMIT extended validation data
#define EXTRA      14    // Control/command messages
#define N_CLASS    15    // Total message type count
```

Note: types 5, 8, 9, 11, 12 are unused/reserved.

### ALKUT (0) — Handshake

Sent when establishing communication between two machines. Must be acknowledged before data exchange begins.

**Payload** (`HkDef.h:1248-1254`): 10 bytes
```
Offset  Size  Type    Field       Description
0       1     CHAR    tunn        Marker (always ASCII 1)
1       2     CHAR[2] konetunn    Sender's machine ID (e.g., "J1")
3       1     UINT8   vaihe       Competition stage/leg (k_pv + 1)
4       2     UINT16  nrec        Record count in KILP.DAT (must match receiver)
6       4     UINT32  flags       Bit 0: stage change request (lah_vaihto)
```

**Send**: `alkusanoma()` in `HkCom32.cpp:811-819`
**Receive validation**: `tark_alku()` in `HkCom32.cpp:821-871`

Validation checks:
- `tunn` must be 1 (`HkCom32.cpp:826`)
- `nrec` must match local KILP.DAT record count (`HkCom32.cpp:830`)
- `vaihe` must match current competition stage (`HkCom32.cpp:850`)
- Mismatched stage with flag bit 0 triggers stage change (`HkCom32.cpp:843-848`)

### KILPT (1) — Full Competitor Record

Sends the entire base record for a competitor. Used for initial sync and full data updates.

**Payload** (`HkDef.h:1255-1261`): `kilprecsize0 + 6` bytes
```
Offset  Size          Type    Field     Description
0       1             CHAR    tarf      Target flag (for selective relay)
1       1             CHAR    pakota    Force save and forward (override)
2       2             INT16   dk        Database address (record index in KILP.DAT)
4       2             INT16   entno     Entry number (0 = new entry)
6       kilprecsize0  CHAR[]  ckilp     Packed competitor base data
```

**Send**: `laheta()` in `HkCom32.cpp:167` (when `piste == 0`)
**Receive**: `tark_kilp(cn, 1)` in `HkCom32.cpp:898-899`

The `ckilp` data contains competitor name, club, class, and all base fields packed by `kilp.pack0()`.

### KILPPVT (2) — Competitor Stage Data

Sends stage-specific data for a competitor: results, split times, EMIT badge number, etc. This is the most commonly sent message during competition.

**Payload** (`HkDef.h:1262-1269`): `kilppvtpsize + 8` bytes
```
Offset  Size           Type    Field    Description
0       1              CHAR    tarf     Target flag
1       1              CHAR    pakota   Force save and forward
2       2              INT16   dk       Database address (record index)
4       2              INT16   pv       Stage number (0-based, -1 for special)
6       2              INT16   valuku   Number of split time slots
8       kilppvtpsize   CHAR[]  cpv      Packed stage data (see below)
```

**CPV (stage data) structure** — key field offsets within `cpv`:
```
Offset  Size  Type      Field       Description
0       20    WTEXT     txt         Display text
20      1     CHAR      uusi        New/modified flag
22      8     INT32x2   flags       Status flags
30      4     INT32     tav         Target time
34      4     INT32     enn         Predicted time
38      24    WTEXT     rata        Course/route
62      2     INT16     sarja       Class/category
64      2     INT16     era         Heat/era
66      2     INT16     bib         Bib number
68      8     INT32x2   badge       EMIT card number + backup
76      4     WCHAR×2   laina       Loan flag
80      36    WTEXT     selitys     Description/notes
116     8     INT32×2   pvpisteet   Stage points
124     4     INT32     tlahto      Start time
128     2     WCHAR     keskhyl     DNS/DNF/DSQ code
130     12    WTEXT     ampsakot    Shooting penalties (biathlon)
142     4     INT32     tasoitus    Handicap
146     4     INT32     sakko       Penalty time
150     2     INT16     ysija       Overnight position
152+    N×8   vatp[]    va          Split times (8 bytes each)
```

Total: `kilppvtpsize = 152 + (valuku × 8)` bytes. Defined via `kilpparam.kilppvtpsize`.

**Send**: `laheta()` in `HkCom32.cpp:203-209` (when `piste < 0`)
**Receive**: `tark_kilp(cn, 2)` in `HkCom32.cpp:901-902`

### VAIN_TULOST (3) — Time Result Only

Lightweight message for sending a single time result (one split or finish time) without full competitor data.

**Payload** (`HkDef.h:1270-1278`): 20 bytes
```
Offset  Size  Type    Field    Description
0       1     CHAR    tarf     Target flag
1       1     CHAR    pakota   Force save and forward
2       2     INT16   dk       Database address
4       2     INT16   bib      Bib number
6       2     INT16   k_pv     Current stage
8       2     INT16   vali     Split point index (-1=start, 0=finish, >0=split)
10      4     INT32   aika     Time value (in 1/100s from t0)
```

**Send**: `laheta()` in `HkCom32.cpp:167` (when `piste >= 1`)
**Receive**: `tark_kilp(cn, 0)` in `HkCom32.cpp:913-914`

### AIKAT (4) — Timer Data

Sends departure and finish time data from a timing device.

**Payload** (`HkDef.h:1279-1283`): `2 × sizeof(aikatp) + 1` bytes
```
Offset  Size          Type    Field    Description
0       1             CHAR    pakota   Force flag
1       sizeof(aikatp) aikatp  daika    Departure time record
1+N     sizeof(aikatp) aikatp  iaika    Finish time record
```

**Send**: `lahetaaika()` (referenced in competition logic)
**Receive**: `tark_aika(cn)` in `HkCom32.cpp:904-905`

### EMITT (6) — EMIT Punch Data

Sends raw EMIT electronic punch card reading. Only compiled when `EMIT` preprocessor flag is defined.

**Payload** (`HkDef.h:1197-1220`): `sizeof(emittp)` bytes

```
Offset  Size           Type     Field       Description
0       4              UINT32   package     Package/session number
4       4              INT32    badge       EMIT card number
8       2              INT16    badgeyear   Card year
10      2              INT16    badgeweek   Card week
12      4              INT32    maali       Finish time (from RTR2 clock)
16      4              INT32    time        Card read timestamp
20      MAXNLEIMA      CHAR[]   ctrlcode    Control point codes
20+N    MAXNLEIMA×2    UINT16[] ctrltime    Control point punch times
...     2              INT16    sc          Start control count
...     2              INT16    pc          Pre-control count
...     2              INT16    lc          Finish control count
...     2              INT16    kilpno      Competitor number
...     2              INT16    osuus       Leg (for relay)
...     4              INT32    badge0      Original badge (before change)
...     4              INT32    kirjaus     Registration timestamp
```

**Receive**: `tark_emit(cn)` in `HkCom32.cpp:917-919` (conditional `#ifdef EMIT`)

### SEURAT (7) — Club Data

Sends club/organization information. Only compiled when `SEURAVAL` is defined.

**Payload** (`HkDef.h:1289-1294`): `sizeof(sra)` bytes
```
Offset  Size           Type     Field     Description
0       (LSEURA+1)×2   WCHAR[]  snimi     Full club name
...     (LLYH+1)×2     WCHAR[]  lyhenne   Abbreviation
...     4×2            WCHAR[]  maa       Country code
...     2              INT16    piiri     District number
```

**Receive**: `tark_seura(cn)` in `HkCom32.cpp:927-929`

### FILESEND (10) — File Transfer

Transfers files between machines in segments. Used for configuration sync, course data, etc.

**Payload** (`HkDef.h:1306-1312`): `combufsize - 14` bytes (fills maximum packet)
```
Offset  Size  Type    Field   Description
0       4     UINT32  flags   Control flags
4       4     UINT32  koodi   File type/code identifier
8       4     INT32   pos     Position (-1=start with filename, ≥0=data offset, -2=end)
12      4     INT32   len     Content length in this segment
16      N     CHAR[]  buf     Filename (at start) or file content (during transfer)
```

**Protocol**:
1. First segment: `pos = -1`, `buf` contains relative filename
2. Data segments: `pos = offset`, `buf` contains file data
3. Final segment: `pos = -2`, signals transfer complete

**Receive**: `tark_tiedosto(cn)` in `HkCom32.cpp:939-940`

### EMITVA (13) — EMIT Extended Data

Sends extended EMIT validation data with control point details. Only compiled when `EMIT` is defined.

**Payload** (`HkDef.h:1222-1231`): `sizeof(emitvatp)` bytes
```
Offset  Size             Type     Field      Description
0       2                INT16    kilpno     Competitor number
2       2                INT16    osuus      Leg (relay)
4       4                INT32    badge      EMIT card number
8       4                INT32    tulos      Result/time
12      2                INT16    rastiluku  Control count
14      12×2             WCHAR[]  rata       Course identifier
...     2                INT16    ok         Validation status
...     (MAXNLEIMA-1)×4  UINT16[][] rastit   Control point data (code + time pairs)
```

**Send**: `laheta_emva_yht()` in `HkCom32.cpp:715-719`
**Receive**: `tark_emitva(cn)` in `HkCom32.cpp:921-923`
**TCP XML**: Also sent as XML via `xmlemitvasanoma()` in `HkCom32.cpp:1039-1041`

### EXTRA (14) — Control Commands

Generic command/control message with 4 INT32 fields. Sub-type determined by `d1 & 0x0F`.

**Payload** (`HkDef.h:1300-1305`): 20 bytes (includes padding)
```
Offset  Size  Type    Field  Description
0       4     INT32   d1     Sub-type in low nibble + data in upper bits
4       4     INT32   d2     Parameter 2
8       4     INT32   d3     Parameter 3
12      4     INT32   d4     Parameter 4
```

**Sub-types** (handled in `tark_extra()`, `HkCom32.cpp:619-671`):

| d1 & 0x0F | Name | Description | Conditional |
|------------|------|-------------|-------------|
| 1 | Electronic checkpoint | `tall_ec(d3, d4, d2, cn+1)` — logs badge/time from electronic control | `#ifdef VALUKIJA` |
| 2 | Activation | `aktivointi(d2, d3, cn+1)` — activate competitor in MAKI mode | `#ifdef MAKIx` |
| 3 | Round change | `vaihdakierros(d2, cn+1)` — change round in MAKI mode | `#ifdef MAKIx` |
| 5 | Era start time | Set era start time: `eralahto[d2] = d3`, forward to others | Always |
| 6 | Curvinen data | `tulkKurvinen(...)` — process Curvinen timing data, forward | Always |
| 7 | Test initiation | `kaynnistatesti(cn+1)` — trigger test mode | Always |
| 9 | Shutdown | Remote shutdown command. If `d2` matches machine ID or is empty, shutdown. Otherwise forward. | Always |

**Send examples**:
- Test: `lahetatestikaynnistys()` in `HkCom32.cpp:601-617`
- Activation: `lahetaaktivointi()` in `HkCom32.cpp:674-692`
- Round change: `lahetakierros()` in `HkCom32.cpp:694-711`

---

## Retry and Timeout Mechanisms

### RTT Parameters (`WinUDP.cpp:42-46`)

```
UDP_RD_TIMEOUT  = 1000ms    UDP read timeout
RTT_RXTMIN      = 200ms     Minimum retransmit timeout
RTT_RXTMAX      = 5000ms    Maximum retransmit timeout
TCP_RD_TIMEOUT  = 5ms       TCP read timeout
TCP_SND_TIMEOUT = 10ms      TCP send timeout
```

### UDP Retransmission (`TpComY32.cpp:1470-1514`)

1. After sending, poll for ACK with increasing delays: `Sleep(UDPviive_lah + n*50)`
2. Track retry count per connection: `toisto[cn]` (up to 3)
3. Each attempt polls up to `3 × toisto[cn]` times
4. ACK validation: `buf[0] == ACK && nch >= 4`
5. If no ACK after all retries, message stays in outbound queue for next cycle

### TCP Handling (`TpComY32.cpp:1467-1468`)

TCP uses a simple delay after sending (`Sleep(TCPviive_lah[cn])`) instead of ACK polling. TCP's built-in reliability replaces application-level retransmission.

### RTT Adaptation (`WinUDP.cpp:967-974`)

RTO (Retransmission Timeout) adapts to network conditions:
- Initial values: `rttvar = 750`, `rto = 3000` (`WinUDP.cpp:284-285`)
- Clamped to `[200ms, 5000ms]` via `rtt_minmax()`

---

## Message Queuing

### Outbound Queue (`TpComY32.cpp:469-538`)

- **Per-connection** circular buffer: `outbuf[cn]`, length `OUTBUFL`
- Managed by head `cjseur[cn]` and tail `cjens[cn]`
- `buflah()` enqueues messages
- Capacity warnings at 50% and 75% full
- Optional disk-backed persistence (`levypusk`) for crash recovery

### Inbound Queue (`TpComY32.cpp:620-652`)

- **Single shared** circular buffer: `inbuf[]`, length `INBUFL`
- Managed by `inbseur` (head) and `inbens` (tail)
- `addseur()` enqueues incoming messages
- Important constraint: for UDP, the queue accepts only when `inbseur == inbens` (single-slot for UDP), unlike RS-232 which allows a deeper queue. This is why the Java implementation uses 500ms pacing between sends.

### Processing Thread (`HkCom32.cpp:875-964`)

`tarkcom()` runs as a background thread, dispatching incoming messages by `pkgclass`:
```c
switch (inbuf[inb].pkgclass) {
    case ALKUT:    break;  // handled during receive
    case KILPT:    tark_kilp(cn, 1);    break;
    case KILPPVT:  tark_kilp(cn, 2);    break;
    case AIKAT:    tark_aika(cn);       break;
    case VAIN_TULOST: tark_kilp(cn, 0); break;
    case EMITT:    tark_emit(cn);       break;
    case EMITVA:   tark_emitva(cn);     break;
    case SEURAT:   tark_seura(cn);      break;
    case EXTRA:    tark_extra(cn);      break;
    case FILESEND: tark_tiedosto(cn);   break;
}
```

---

## Connection Lifecycle

1. **Configuration**: Read `laskenta.cfg` to determine connections, ports, machine ID
2. **Port opening**: `opencomUDP()` or `openportTCP()` based on `comtype[cn]`
3. **Handshake**: Send ALKUT (type 0) with phase and record count
4. **Validation**: `tark_alku()` checks KILP.DAT record count and competition stage match
5. **Data exchange**: Messages queued via `laheta()` family, sent by `lahetapaketti()`, processed by `tarkcom()`
6. **Retry loop**: Failed UDP sends retry up to 3× with 200-5000ms backoff
7. **Shutdown**: EXTRA sub-type 9 sends remote shutdown command

---

## TCP vs UDP Analysis

### Feature Comparison

| Aspect | UDP | TCP Binary | TCP XML |
|--------|-----|-----------|---------|
| Frame format | SOH + payload + checksum | Same SOH frame | STX + XML + ETX |
| Reliability | Application-level ACK/NAK | TCP built-in | TCP built-in |
| Retransmission | App-level, up to 3 retries | TCP built-in | TCP built-in |
| Reconnection | N/A (stateless) | `reconnectTCP()` auto-reconnect | Same |
| Multi-message | One at a time (addseur constraint) | Stream, sleep-paced | Stream, sleep-paced |
| Firewall | Requires inbound UDP ports on both sides | Single outbound TCP connection | Same |
| NAT traversal | Difficult (needs port forwarding both ways) | Client-initiated, NAT-friendly | Same |
| Latency | Lower (no handshake) | Higher (TCP handshake + Nagle) | Higher (XML overhead) |
| Message types | All 10 types | All 10 types (same format) | KILPT, VAIN_TULOST, EMITVA as XML |

### TCP for Cloud/Remote Integration

TCP is significantly better suited for cloud deployments:

1. **Firewall-friendly**: Only requires a single outbound TCP connection from the venue. UDP requires symmetric port forwarding.
2. **NAT-friendly**: TCP client connections traverse NAT naturally. UDP's peer-to-peer model requires both sides to be reachable.
3. **Reconnection**: `reconnectTCP()` (`WinUDP.cpp:841`) handles connection drops with automatic retry, essential for WAN links.
4. **TCP XML mode** is particularly interesting for cloud integration: it outputs standard XML that could be consumed by web services without understanding the binary protocol.

### Current TCP Limitations

1. **Compile-time gating**: TCP features require `#ifdef TCPSIIRTO` or `#ifdef TCPLUKIJA` (`HkCom32.cpp:966`, `HkInit.cpp:464`). Not all build targets include these.
2. **Windows-centric**: TCP code uses `WinHTTP.h`, Winsock2, and Windows threading. The Linux compatibility layer may not cover TCP fully.
3. **XML mode is partial**: Only KILPT, VAIN_TULOST, and EMITVA are converted to XML. Other message types (AIKAT, SEURAT, EXTRA, etc.) are either skipped or not handled.
4. **No encryption**: Neither TCP nor UDP mode includes encryption. For cloud use, a VPN or SSH tunnel would be needed.

### Recommendation for pirila-udp Extension

For extending the Java `pirila-udp` library to support TCP:

1. **Binary TCP**: Minimal effort — the same frame format is used. Just replace `DatagramSocket` with `Socket`/`ServerSocket`. Remove ACK/NAK logic (TCP handles reliability). Keep the 500ms pacing since the C++ server's `addseur()` still has a single-slot input buffer.

2. **TCP XML**: Would require implementing XML serialization matching `xmlsanoma()` and `xmlemitvasanoma()` output formats. More work but more interoperable.

3. **For cloud**: TCP client mode (`comtpTCP` / `ipTCPCLIENT`) is the path of least resistance. The venue's C++ system acts as TCP server, the cloud Java service connects as TCP client. Only the C++ side needs `TCPSIIRTO` compiled in.

---

## Java Implementation Status

The `pirila-udp` library currently implements only a subset of the protocol:

| Message Type | Java Status | File |
|--------------|-------------|------|
| ALKUT (0) | Implemented | `TulospalveluConnection.java:348-359` |
| KILPT (1) | Not implemented | — |
| KILPPVT (2) | Implemented (send + receive) | `TulospalveluConnection.java:136-165` |
| VAIN_TULOST (3) | Not implemented | — |
| AIKAT (4) | Not implemented | — |
| EMITT (6) | Not implemented | — |
| SEURAT (7) | Not implemented | — |
| FILESEND (10) | Not implemented | — |
| EMITVA (13) | Not implemented | — |
| EXTRA (14) | Not implemented | — |

### Key Java constants

```java
SOH = 0x01, STX = 0x02, ETX = 0x03, ACK = 0x06, NAK = 0x15
PKGCLASS_ALKUT = 0x00, PKGCLASS_KILPPVT = 0x02
ALKUT_DATA_SIZE = 10
KILPPVTPSIZE0 = 152  // base stage size without split times
PV_OFF_BADGE = 68    // offset to badge INT32 within cpv
PORTBASE = 15900
```

### Priorities for Extension

1. **KILPT (1)** — receive full competitor records for initial sync
2. **VAIN_TULOST (3)** — receive real-time timing results
3. **EMITT (6)** — receive EMIT punch card readings
4. **EXTRA (14)** — at minimum handle sub-type 9 (shutdown) gracefully
5. **SEURAT (7)** — receive club data for display
6. **TCP transport** — for cloud connectivity

---

## Configuration (laskenta.cfg)

Connection configuration parsed in `HkInit.cpp`. Example:

```
Kone=J1                         # Machine ID (2 chars)
Emit                            # Enable EMIT support
yhteys1=udp:15901/localhost:15902  # UDP: local_port/remote_host:remote_port
yhteys2=tcp:/192.168.1.10:15901    # TCP client to remote host
yhteys3=tcp:+15903                 # TCP server on port 15903
```

**TCP configuration** (`HkInit.cpp:465-504`):

- TCP client: `tcp:/hostname:port` → `comtpTCP`, `ipTCPCLIENT`
- TCP server: `tcp:+port` → `comtpTCPSRV`, `ipTCPSERVER`
- Server parameter `-1`: XML mode → `comtpTCPXML`
- Server parameter `-2`: Clock service → `comtpTCPLOKI`

---

## References

- Main protocol implementation: `TPsource/V52/Tp/TpComY32.cpp`
- Socket layer: `TPsource/V52/com/WinUDP.cpp`
- Message definitions: `TPsource/V52/Hk/HkDef.h` (lines 1240-1318)
- Message type constants: `TPsource/V52/Hk/HkCom32.cpp` (lines 47-57)
- Message handlers: `TPsource/V52/Hk/HkCom32.cpp` (lines 875-964)
- TCP/XML transmission: `TPsource/V52/Hk/HkCom32.cpp` (lines 972-1078)
- Communication API: `TPsource/V52/include/wincom.h`
- Configuration parsing: `TPsource/V52/Hk/HkInit.cpp` (lines 464-504)
- Java implementation: `pirila-udp/src/main/java/fi/pirila/tulospalvelu/TulospalveluConnection.java`
- HTTP client (results upload): `TPsource/V52/com/gethttp.cpp` (WinHTTP-based, unrelated to inter-machine protocol)
