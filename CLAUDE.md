# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project is

Pekka Pirilä's sports timekeeping suite ("tulospalvelu"), originally developed around 1986, released under GPLv3. It handles competitor timing, split recording, results calculation and publishing — primarily for orienteering, with extensions for skiing, biathlon, and relay races. The UI and most variable names/comments are in Finnish.

## Build instructions

### Console programs (Visual Studio 2013+)

Build order matters — the utility library must be built first:

1. Open `TPsource\V52\VS\Libs\tputilv2.sln` → F7 (creates `vc10/` and `TPexe/` directories)
2. Open `TPsource\V52\VS\Hk\HkMaali520.sln` → F7 → produces `TPexe\Hk\V521\HkMaali.exe`
3. For relay version: `TPsource\V52\VS\V\JukMaali520.sln` → F7 → produces `TPexe\V\JukMaali.exe`

Language standard: C++03.

### Windows GUI programs (Embarcadero C++ Builder 10.1 Berlin)

Requires **SecureBridge 7.1 for RAD Studio 10.1 Berlin**. Before compiling, edit:
`Program Files (x86)\Devart\SecureBridge for RAD Studio 10\Include\Win32\ScSSHSocket.hpp`
and change `Winapi::Winsock::PSockAddrIn` → `Winapi::Winsock2::PSockAddrIn`.

Build order:
1. `DBboxm-XE.cbproj` → right-click → Make
2. `Tputil-XE.cbproj` → right-click → Make
3. `HkKisaWin.cbproj` → Run (individual competition GUI)
4. `ViestiWin.cbproj` → Run (relay GUI)

All `.cbproj` files are in `TPsource\V52\RADStudio10\`.

If linker heap errors occur: run `bcdedit /set IncreaseUserVa 3072` as Administrator and reboot.

There are no automated tests.

## Architecture

### Two competition domains

**HK (henkilökilpailu = individual competition)**
- Core logic: `TPsource/V52/Hk/` — `HkDef.h` (data structures), `HkDeclare.h` (function declarations), `HkInit.cpp`, `HkEmit.cpp` (chipcard/RFID), `HkAjat.cpp` (time calculation), `HkIV.cpp` (screen output), `HkMuotoilu.cpp` (output formatting), `HkCom32.cpp` (communications)
- Windows GUI: `TPsource/V52/cbHk/` — ~76 VCL form units

**Juk/Viesti (relay/team competition)**
- Core logic: `TPsource/V52/Juk/` — `VDef.h`, `VDeclare.h`, `VEmit.cpp`, `VInit.cpp`, `VMuotoilu.cpp`, `VTulostus.cpp`, `VXml.cpp`, `VXml_IOF30.cpp`
- Windows GUI: `TPsource/V52/ViestiWin/` — ~60 VCL form units

Both domains share the same utility and database layers, and compile to both a console TUI and a Windows VCL GUI from the same source via `#ifdef _CONSOLE`.

### Shared layers

**Utilities** (`TPsource/V52/tputilv2/`, `TPsource/V52/include/`):
- `tputil.h` — master header: `TextFl` (file I/O), `PRFILE` (printer/GDI), time conversion (`aikatos`/`aikatostr`), string utilities, XML/HTML output
- `tptype.h` — cross-platform type definitions
- `TpDef.h` — global constants and keyboard codes
- `wincom.h` — network abstraction (RS-232, UDP, TCP, XML)

**Database** (`TPsource/V52/dbboxm/`): Custom binary record files with fixed-size records, block compression, and in-memory indexing (`kilpindex`). No SQL — records accessed by numeric `DATAREF`.

### Key data structures

- `kilptietue` — competitor record (name, club, category, bib, split time pointers)
- `kilppvtp` — per-stage data (split times array `vatp[]`, status, penalties)
- `tulosmuottp` — output formatting configuration (60+ flags for HTML/print styling)
- `kaavatp` / `pistekaavatp` — points calculation formulas

### Communications

Supports RS-232 (legacy), UDP, TCP, and XML message transport. The `comtp` and `iptype` enums in `wincom.h` define the protocol variants. Used to sync split times between distributed timing stations.

### Output formats

Results can be emitted as HTML, plain text, IOF 3.0 XML, and GDI print output. The `tulosmuottp` struct controls all formatting decisions and is passed to the formatter functions in `HkMuotoilu.cpp` / `VMuotoilu.cpp`.
