# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Tulospalvelu is a sports time-keeping and results system originally developed by Pekka Pirilä (1945-2015) starting in 1986. Specialized for orienteering but supports other sports. Released under GPLv3. UI and comments are in Finnish.

## Architecture

Three distinct components communicate via a custom UDP protocol:

1. **Legacy C++ System** (TPsource/V52) - The original console and Windows GUI applications
2. **Pirila-Comm** (pirila-comm/) - Multi-module Java protocol library (common + UDP + TCP transports)
3. **Java POC** (javapoc/) - CLI tool demonstrating protocol integration (emit card changes)
4. **Web Admin** (webadmin/) - Modern Spring Boot + Vaadin web interface (early stage, stub services)

Data flow: C++ system ↔ UDP/TCP protocol (port 15900+) ↔ Java components

Key data files in `kisat/`:
- `KILP.DAT` - Binary competitor database (dbbox format, little-endian)
- `laskenta.cfg` - Configuration (UTF-8, defines machine ID and UDP connections)
- `KilpSrj.xml` - Competition structure

## Build Commands

### Webadmin (Spring Boot + Vaadin)
```bash
cd webadmin
mvn                              # Run dev server on http://localhost:8080
mvn spring-boot:run              # Same as above
mvn clean package -Pproduction   # Build production JAR
mvn test                         # Run tests
```

### Pirila-Comm (Protocol Libraries)
```bash
cd pirila-comm
mvn clean install                # Build all modules (common, udp, tcp) and install to local repo
```

### Javapoc (UDP Protocol CLI)
```bash
cd javapoc
mvn clean package                # Build fat JAR (requires pirila-comm installed first)
java -jar target/tulospalvelu-java-1.0.0-SNAPSHOT.jar <competitor_no> <emit_card> [host] [port]
```

### C++ Console (Linux)
```bash
cd TPsource/V52
make                             # Build HkMaali executable
make clean                       # Clean build artifacts
make first-try                   # Quick compilation syntax check
```

### C++ Console (Windows - Visual Studio)
1. Open and build `TPsource/V52/VS/Libs/tputilv2.sln`
2. Open and build `TPsource/V52/VS/Hk/HkMaali520.sln` (individual competition)
3. For relay: `TPsource/V52/VS/V/JukMaali520.sln`

### C++ Windows GUI (RAD Studio 10.1 Berlin)
Requires SecureBridge 7.1 add-on. Build libraries first (DBboxm-XE, Tputil-XE), then HkKisaWin or ViestiWin.

## Key Files

### Pirila-Comm Libraries (pirila-comm/)
- `pirila-comm/pirila-comm-common/` - Shared protocol definitions, binary utilities, KILP.DAT access
  - `TulospalveluProtocol.java` - Protocol constants, checksum, payload builders/parsers for all 10 message types
  - `MessageListener.java` - Callback interface for incoming messages (KILPPVT, KILPT, VAIN_TULOST, EXTRA)
  - `KilpReader.java` - KILP.DAT binary parser and writer
  - `ConfigReader.java` - laskenta.cfg parser
- `pirila-comm/pirila-udp/` - UDP transport
  - `TulospalveluConnection.java` - Netty UDP handler with ACK/NAK, handshake, all message types
- `pirila-comm/pirila-tcp/` - TCP transport (NAT/firewall-friendly for cloud)
  - `TulospalveluTcpConnection.java` - Netty TCP handler with reconnection
  - `TulospalveluTcpFrameDecoder.java` - TCP stream framing decoder

### Javapoc CLI Application
- `javapoc/src/main/java/fi/pirila/tulospalvelu/InteractiveEmitChanger.java` - Interactive CLI for emit card changes

### Webadmin Views
- `webadmin/src/main/java/in/virit/pirila/views/KortinVaihtoView.java` - Emit card change UI (needs real backend integration)
- Services in `webadmin/src/main/java/in/virit/pirila/service/` are currently stubs

### C++ Core
- `TPsource/V52/Hk/` - Individual competition logic
- `TPsource/V52/Juk/` - Relay competition logic
- `TPsource/V52/com/WinUDP.cpp` - UDP implementation
- `TPsource/V52/include/linux_compat.h` - Linux portability layer

## UDP Protocol Notes

- Frame: `STX(0x02) | port_LE(2) | machineID(2) | payload`
- Message types: ALKUT (0x00 - handshake), KILPPVT (0x02 - competitor update)
- Checksum: 16-bit little-endian sum
- ACK (0x06) / NAK (0x15) responses
- See `TulospalveluConnection.java` for complete protocol implementation

## Tech Stack

- **Webadmin**: Java 25, Spring Boot 4.0.4, Vaadin 25.1.1, Viritin 3.4.0-SNAPSHOT
- **Javapoc**: Java 11, Netty 4.2.10.Final
- **C++ Console**: C++11 (g++) / C++03 (Visual Studio)
- **C++ GUI**: RAD Studio 10.1 Berlin with SecureBridge

## Notes

- Character encoding: ISO-8859-1 for C++ (Finnish language), UTF-8 for Java
- Competition data uses little-endian binary format
- Many webadmin services are stubs awaiting real implementation
