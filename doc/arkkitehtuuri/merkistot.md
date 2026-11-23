# Merkistöt ja turvallinen muokkaus / Encodings and safe editing

Polut ovat suhteessa hakemistoon `TPsource/V52/`.
Paths are relative to `TPsource/V52/`.

## Tilanne / Current state

631 lähdetiedostosta 275 on pelkkää ASCII:ta, 204 Windows-1252:ta (cp1252), 148
UTF-8:aa ilman BOMia, 2 UTF-8:aa BOMin kanssa ja 2 sekoitettuja
(`tputilv2/aakjarjws.cpp`, `tputilv2/aakjarjws2.cpp`). UTF-8-tiedostot ovat
pääosin `tputilv2/`- ja `dbboxm/`-hakemistoissa. Hakemistokohtainen jakauma:
`inventaario.md`. [todennettu: `tyokalut/inventaario.py`]

Of 631 source files, 275 are pure ASCII, 204 Windows-1252 (cp1252), 148 UTF-8
without BOM, 2 UTF-8 with BOM and 2 mixed (`tputilv2/aakjarjws.cpp`,
`tputilv2/aakjarjws2.cpp`). The UTF-8 files are mostly in `tputilv2/` and
`dbboxm/`. Distribution per directory: `inventaario.md`. [verified:
`tyokalut/inventaario.py`]

Sekoitetuissa tiedostoissa koodi ja merkkivakiot (`L'É'` jne.) ovat cp1252:ta ja
oikein, mutta kevään 2026 kommentoinnissa lisätyt kommentit ovat UTF-8:aa. Kääntäjä
ei välitä kommenteista, mutta editori näyttää aina osan tiedostosta väärin, ja
tallennus väärällä merkistöllä rikkoo koodin merkkivakiot. [todennettu: tavut]

In the mixed files the code and character constants (`L'É'` etc.) are cp1252 and
correct, but the comments added in the spring 2026 commenting pass are UTF-8. The
compiler ignores comments, but an editor always shows part of the file wrongly,
and saving with the wrong encoding breaks the code's character constants.
[verified: bytes]

## Miten kääntäjät lukevat tiedostot / How the compilers read the files

Järjestelmän ANSI-koodisivu tällä koneella on 1252. GUI-ohjelmat käännetään
**klassisella `bcc32`-kääntäjällä** (ei Clang-pohjaisella `bcc32c`:llä), mikä näkyy
käännöslokissa (`RADStudio10/build-logs/*.log`: "bcc32 command line").

The system ANSI code page on this machine is 1252. The GUI programs are built with
the **classic `bcc32` compiler** (not the Clang-based `bcc32c`), as the build log
shows (`RADStudio10/build-logs/*.log`: "bcc32 command line").

| Tiedoston merkistö / File encoding | MSVC (konsoli / console) | bcc32 (GUI) |
|---|---|---|
| cp1252 | oikein / correct | oikein / correct |
| UTF-8 ilman BOMia / without BOM | **väärin / wrong**: `ä` → `Ã¤` | oikein / correct |
| UTF-8 BOMilla / with BOM | oikein / correct | ei testattu / not tested |

Miten todennettu / How verified:

- **MSVC:** testiohjelma, jossa `L"äö"` ja `"äö"` tallennettuna kolmella
  merkistöllä, käännettynä `cl /J`:llä ja ajettuna. UTF-8 ilman BOMia tuotti
  merkit `00C3 00A4 00C3 00B6`, muut `00E4 00F6`. [todennettu]
- **bcc32:** Community Edition ei salli komentorivikäännöstä, joten tarkistus
  tehtiin valmiista ohjelmista. UTF-8-tiedoston `Tp/TpLaitteet.cpp:2581` teksti
  "Asema lähettää" on `HkKisaWin.exe`:ssä ja `ViestiWin.exe`:ssä oikein
  (`00E4`), mutta MSVC:llä käännetyssä `HkMaali.exe`:ssä väärin (`00C3 00A4`).
  cp1252-tiedoston `Hk/HkXml.cpp` teksti "rivillä" on oikein molemmissa.
  [todennettu]

- **MSVC:** a test program with `L"äö"` and `"äö"` saved in three encodings,
  compiled with `cl /J` and run. UTF-8 without BOM produced `00C3 00A4 00C3 00B6`,
  the others `00E4 00F6`. [verified]
- **bcc32:** the Community Edition does not allow command-line compiling, so the
  check was made on the built programs. The text "Asema lähettää" from the UTF-8
  file `Tp/TpLaitteet.cpp:2581` is correct in `HkKisaWin.exe` and `ViestiWin.exe`
  (`00E4`) but wrong in the MSVC-built `HkMaali.exe` (`00C3 00A4`). The text
  "rivillä" from the cp1252 file `Hk/HkXml.cpp` is correct in both. [verified]

Johtopäätös: koodipohjaa **ei voi muuntaa** yhteen merkistöön muuttamatta
jommankumman ohjelmaryhmän tekstejä, ellei samalla muuteta käännösasetuksia
(esim. MSVC:n `/utf-8` tai BOM kaikkiin UTF-8-tiedostoihin). Sellainen muutos
on oma, erikseen testattava työnsä.

Conclusion: the code base **cannot be converted** to a single encoding without
changing the text of one group of programs, unless build settings change too
(e.g. MSVC `/utf-8` or a BOM in every UTF-8 file). Such a change is a separate
piece of work that must be tested on its own.

## Ohjelman tekstejä koskevat viat / Defects in program text

Skripti `tyokalut/utf8_literaalit.py` listaa UTF-8-tiedostojen ei-ASCII-merkkijonot,
jotka eivät ole kommenteissa. Tulos 2026-11-24:

The script `tyokalut/utf8_literaalit.py` lists non-ASCII string literals outside
comments in UTF-8 files. Result on 2026-11-24:

| Paikka / Location | Teksti / Text | Vaikutus / Effect |
|---|---|---|
| `Hk/HkAjat.cpp:234` | `L"AIKATAULUKKO T�YSI"` (U+FFFD) | Rikki kaikissa HK-ohjelmissa / broken in all HK programs |
| `Juk/vajat.cpp:191` | `"AIKATAULUKKO T�YSI"` (U+FFFD) | Rikki kaikissa viestiohjelmissa / broken in all relay programs |
| `Tp/TpLaitteet.cpp:2581` | `L"Koodi %d - Asema lähettää tietoja…"` | Rikki konsoliohjelmissa / broken in console programs |

U+FFFD-merkit syntyivät commitissa `5a65c64`, kun tiedosto muunnettiin UTF-8:ksi
ja alkuperäiset cp1252-merkit korvautuivat. Alkuperäiset tekstit ovat commitissa
`50db2f8`. Samoissa tiedostoissa on myös kommentoituja tekstejä, joissa on U+FFFD
(yhteensä noin 37 riviä). Vika on kirjattu erikseen: `issue-drafts/04`.

The U+FFFD characters were introduced in commit `5a65c64`, when the file was
converted to UTF-8 and the original cp1252 characters were replaced. The original
texts are in commit `50db2f8`. The same files also have commented-out text with
U+FFFD (about 37 lines in total). The defect is recorded separately:
`issue-drafts/04`.

## Turvallinen muokkausmenettely / Safe editing procedure

Säännöt kaikille lähdekoodimuutoksille:

1. **Uudet kommentit vain ASCII-merkein ja englanniksi.** Silloin kommentti on
   oikein kaikissa merkistöissä ja editoreissa.
2. **Älä muuta tiedoston merkistöä.** Muokkaa tavuina: lue ja kirjoita samalla
   merkistöllä, jolla tiedosto on (tarkista `inventaario.py`:n tuloksesta).
   PowerShell 5.1:n `Set-Content` ja `Out-File` vaihtavat merkistön; älä käytä
   niitä lähdekoodiin.
3. **Älä lisää BOMia äläkä poista sitä.**
4. **Älä muuta rivinvaihtoja.** Tiedostoissa on sekaisin CRLF:ää ja LF:ää.
5. **Älä koske merkkijonoihin ja merkkivakioihin** kommentointimuutoksissa.
6. **Editorit:** VS Codessa aseta `"files.encoding": "windows1252"` ja
   `"files.autoGuessEncoding": true` (paikallisesti, ei repoon). C++Builderissa
   rivinvaihtoasetus "Preserve".

Rules for all source changes:

1. **New comments in ASCII and English only.** Such a comment is correct in every
   encoding and editor.
2. **Do not change a file's encoding.** Edit as bytes: read and write in the
   file's own encoding (check the output of `inventaario.py`). PowerShell 5.1
   `Set-Content` and `Out-File` change the encoding; do not use them on source.
3. **Do not add or remove a BOM.**
4. **Do not change line endings.** Files mix CRLF and LF.
5. **Do not touch string literals or character constants** in commenting changes.
6. **Editors:** in VS Code set `"files.encoding": "windows1252"` and
   `"files.autoGuessEncoding": true` (locally, not in the repo). In C++Builder set
   line endings to "Preserve".

## Todennus: muuttiko muutos koodia? / Verification: did a change alter code?

Kommentointimuutoksen pitää jättää käännetty koodi ennalleen. Se voidaan todistaa
MSVC:llä:

1. Käännä muutosta edeltävä versio: ympäristömuuttuja `CL=/Brepro` ja MSBuild-
   parametri `/p:DebugInformationFormat=None`. Kopioi `.obj`-tiedostot talteen.
2. Tee muutos ja käännä samoin. Kopioi `.obj`-tiedostot.
3. Vertaa: `python -I tyokalut/vertaa_objektit.py <dumpbin.exe> <ennen> <jälkeen>`.
   Skripti ohittaa aikaleiman sekä osiot `.debug$*` ja `.chks64`, joissa on
   lähdetiedoston tarkistussumma ja polkuja. Koodi-, data-, relokaatio- ja
   symbolitietojen pitää olla identtiset.

A commenting change must leave the compiled code unchanged. This can be proved
with MSVC:

1. Build the version before the change with the environment variable `CL=/Brepro`
   and the MSBuild parameter `/p:DebugInformationFormat=None`. Save the `.obj`
   files.
2. Make the change and build the same way. Save the `.obj` files.
3. Compare: `python -I tyokalut/vertaa_objektit.py <dumpbin.exe> <before> <after>`.
   The script ignores the time stamp and the `.debug$*` and `.chks64` sections,
   which hold the source file checksum and paths. Code, data, relocation and
   symbol information must be identical.

Menettely testattiin 2026-11-24 `tputilv2`-kirjastolla (146 objektia) erillisessä
työpuussa: [todennettu]

The procedure was tested on 2026-11-24 with the `tputilv2` library (146 objects)
in a separate worktree: [verified]

| Muutos / Change | Tulos / Result |
|---|---|
| Ei muutosta, uudelleenkäännös / no change, rebuild | 0 / 146 eroaa / differ |
| 2 kommenttiriviä tiedoston alkuun + kommentti rivin perään / 2 comment lines at top + trailing comment | 0 / 146 eroaa / differ |
| `ct[8] = 0;` → `ct[9] = 0;` (`tputilv2/kello.cpp`) | 1 / 146 eroaa (`kello.obj`) / differs |

Rajoitukset / Limitations:

- `__LINE__`-makroa käyttävissä tiedostoissa rivien lisääminen muuttaa koodia
  oikeasti: `tputilv2/perr.cpp`, `tputilv2/consolesize.cpp`, `tputilv2/initcon.cpp`
  (`PERR`-makro, `include/Console.h:18`). Niihin kommentit vain rivien perään tai
  hyväksytään ero tietoisesti.
- Menettely kattaa vain MSVC:llä käännettävät tiedostot. GUI-hakemistot
  (`cbHk/`, `ViestiWin/`, `cbTp/`) käännetään vain C++Builderilla, eikä niille ole
  vielä vastaavaa todennusta. [tuntematon → `seuranta.md`]

- In files using the `__LINE__` macro, adding lines really changes the code:
  `tputilv2/perr.cpp`, `tputilv2/consolesize.cpp`, `tputilv2/initcon.cpp`
  (`PERR` macro, `include/Console.h:18`). Use trailing comments there, or accept
  the difference knowingly.
- The procedure covers only files built with MSVC. The GUI directories (`cbHk/`,
  `ViestiWin/`, `cbTp/`) are built only with C++Builder and have no equivalent
  check yet. [unknown → `seuranta.md`]
