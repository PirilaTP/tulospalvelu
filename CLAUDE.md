# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project is

Pekka Pirilä's sports timekeeping suite ("tulospalvelu"), originally developed around 1986, released under GPLv3. It handles competitor timing, split recording, results calculation and publishing — primarily for orienteering, with extensions for skiing, biathlon, and relay races. The UI and most variable names/comments are in Finnish.

## MOST IMPORTANT RULE: do not break the working FX9500/SIRIT code

**Never modify the working FX9500/SIRIT code without being asked to do so explicitly.** It is in production use and must not be broken. This is the single most important rule in this project — when in doubt, leave the existing FX9500/SIRIT path untouched and ask first.

## Current work: FX9600 RFID reader support

The active task is adding support for the Zebra **FX9600** RFID reader alongside the existing, working **FX9500** reader.

- The older **FX9500** reader appears in code and configuration under the name **SIRIT** (FX9500 is based on Sirit technology) and is configured with the `SIRIT=` parameter (e.g. `SIRIT=TCP:192.168.0.31`).
- The new **FX9600** reader is configured with the parallel `ZEBRA=` parameter (e.g. `ZEBRA=TCP:192.168.0.31`). The program reads the configuration to decide which reader is active.
- Both readers live behind the common `IRfidReader` interface (`TPsource/V52/include/IRfidReader.h`). `SiritReader` (`TPsource/V52/Tp/SiritReader.cpp`) and `ZebraReader` (`TPsource/V52/Tp/ZebraReader.cpp`) are the two implementations, so application logic does not need to know which reader is in use.
- Do FX9600 work on its own git branch (e.g. `feature/fx9600`), not directly on the main branch.

## Working practices

- One thing at a time, in small, clear steps.
- Before larger changes: describe the plan first and wait for approval.
- If something is unclear, ask before assuming.
- Do not add new dependencies (libraries) without asking.

## Language conventions

- Write all **code comments and explanations in Finnish**.
- Variable, function and class names in **English** (established convention).
- **Git commit messages in Finnish.**

## CRITICAL: Source file character encoding

The source files (`.cpp`, `.h`) are encoded in **Latin1 / Windows-1252, NOT UTF-8**. They contain Finnish characters (ä, ö, Ä, Ö) both in comments and in code (e.g. `ESTÄNEG`).

- **NEVER convert these files to UTF-8.** When you edit them, preserve the Latin1 encoding. If you must write a file's bytes explicitly, use PowerShell Latin1: `[System.Text.Encoding]::GetEncoding(28591)`.
- **After every edit, verify that ä/ö characters were not corrupted** (corruption shows up as `�` or as mojibake like `Ã¤`). If you see corruption, fix it before continuing.

A `.gitattributes` file in the repo root marks the encoding of `.cpp`/`.h` files so encoding changes do not slip into commits unnoticed.

## Build instructions

### Console programs (Visual Studio 2022)

Build order matters — the utility library must be built first:

1. Open `TPsource\V52\VS\Libs\tputilv2.sln` → F7 (creates `vc10/` and `TPexe/` directories)
2. Open `TPsource\V52\VS\Hk\HkMaali520.sln` → F7 → produces `TPexe\Hk\V521\HkMaali.exe`
3. For relay version: `TPsource\V52\VS\V\JukMaali520.sln` → F7 → produces `TPexe\V\JukMaali.exe`

Language standard: C++03.

### Windows GUI programs (Embarcadero C++ Builder Community Edition)

No third-party add-ons are required.

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

## Tunnetut asiat (erikseen korjattavat)

### Merkistövioittuma: HkAjat.cpp ja vajat.cpp (U+FFFD)

`TPsource/V52/Hk/HkAjat.cpp` (48 kpl) ja `TPsource/V52/Juk/vajat.cpp` (39 kpl) sisältävät UTF-8:n korvausmerkkejä (U+FFFD, tavut `EF BF BD`) siellä missä pitäisi olla suomalaisia merkkejä (ä/ö ym.) — eli skanditieto on menetetty.

- **Alkuperä:** vioittuma on peräisin repon **juuricommitista `11b35b4`** ("Lähtötilanne ennen FX9600-työtä"), EI FX9600-työn aiheuttama. Molemmat tiedostot ovat olleet vioittuneita repon alusta asti.
- **Ehjää versiota EI ole gitissä:** ei työpuussa, ei haarahistoriassa, ei `master`-haarassa eikä reflogissa (tarkistettu kaikki refit). HkInit.cpp:n palautustapa (palauta vioittumista edeltävästä commitista) ei siis sovi näihin.
- **Sisältää myös käyttäjälle näkyvää tekstiä**, ei vain kommentteja — esim. `HkAjat.cpp`: `wselectopt(L"Aika ei vastaa riviä - vahvista tallennus (K/E)")`. Vioittuma vaikuttaa siis myös näyttöön.
- **Korjataan erikseen myöhemmin** joko (a) alkuperäisestä vioittumattomasta lähteestä (verrataan ja palautetaan tavutarkasti) tai (b) kontekstipohjaisella rekonstruktiolla (ympäröivä suomi on ehjää; muutokset katselmoitava ennen committia). Säilytä Latin1-koodaus korjauksessa (ks. *CRITICAL: Source file character encoding*).
