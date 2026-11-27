# Dokumentointistandardi / Documentation standard

Tämä on ehdotus, jota ei ole vielä otettu käyttöön. Se koskee uusia kommentteja
lähdekoodissa. Arkkitehtuuridokumentit noudattavat `README.md`:n kaksikielistä
muotoa.

This is a proposal that has not been adopted yet. It applies to new comments in
source code. The architecture documents follow the bilingual format in
`README.md`.

## Perusperiaatteet / Principles

1. **Englanti ja ASCII.** Uudet kommentit kirjoitetaan englanniksi ilman ääkkösiä
   (`merkistot.md`). Suomenkieliset tunnisteet (`kilptietue`, `tallenna`) mainitaan
   sellaisinaan.
2. **Pekan kommentit säilytetään.** Alkuperäisiä suomenkielisiä kommentteja ei
   käännetä, siirretä eikä muotoilla uudelleen. Jos alkuperäinen kommentti on
   väärä, lisätään uusi kommentti sen viereen ja kerrotaan miksi.
3. **Kerro miksi, älä mitä.** Kommentti, joka toistaa koodin (`// increment i`),
   on haitallinen. Kirjoita tarkoitus, rajoitteet, sivuvaikutukset ja historia.
4. **Vain todennettua.** Jos kommentti perustuu päättelyyn, se merkitään:
   `// Inferred: ...`. Väärä kommentti on pahempi kuin puuttuva.
5. **Ei koodimuutoksia samassa muutoksessa.** Kommentointi-PR ei saa muuttaa
   käännettyä koodia; se todennetaan (`merkistot.md`, "Todennus").

1. **English and ASCII.** New comments are written in English without non-ASCII
   letters (`merkistot.md`). Finnish identifiers (`kilptietue`, `tallenna`) are
   quoted as they are.
2. **Pekka's comments are kept.** Original Finnish comments are not translated,
   moved or reformatted. If an original comment is wrong, add a new comment next
   to it and explain why.
3. **Say why, not what.** A comment that repeats the code (`// increment i`) is
   harmful. Write purpose, constraints, side effects and history.
4. **Only what is verified.** If a comment rests on inference, mark it:
   `// Inferred: ...`. A wrong comment is worse than a missing one.
5. **No code changes in the same change.** A commenting PR must not change the
   compiled code; this is verified (`merkistot.md`, "Verification").

## Tiedoston otsikko / File header

Lisensseihin ei kosketa. Otsikkokommentti lisätään GPL-tekstin jälkeen:
License text is not touched. The header comment goes after the GPL text:

```cpp
// ---------------------------------------------------------------------------
// HkDat.cpp - competitor record storage for the individual competition (HK).
//
// Owns: kilparr[] access (GETREC/PUTREC), KILP.DAT writes, record add/delete,
//       kilptietue::tallenna() - the single entry point for record changes.
// Used by: Hk/*, cbHk/*, HkConsole/*, Tp/* (when built for HK).
// Threads: GETREC/PUTREC lock kilpdat_CriticalSection internally;
//          tallenna() does not lock tall_CriticalSection (see its comment).
// Built into: HkKisaWin, HkMaali.
// See: doc/arkkitehtuuri/yleiskuva.md "Main flow".
// ---------------------------------------------------------------------------
```

## Funktio / Function

Vain julkisille ja epäselville funktioille. Kentät, joilla ei ole sisältöä,
jätetään pois.

Only for public and non-obvious functions. Fields with nothing to say are left
out.

```cpp
// Saves changes of this record (record number d) and propagates them.
// Compares with the stored record (kilpcmp) and returns early if nothing changed.
// Side effects: updates sort arrays and name index, emit card links, writes
//   KILP.DAT (and SQL if enabled), queues the change to all open connections
//   except the sender (kielto = connection number + 1).
// Preconditions: 1 <= d < nrec; Sarja() valid unless kielto != 0.
// Threads: does not lock tall_CriticalSection itself (see HkDat.cpp:941).
// Legacy: comtarfl == -1 forces index rebuild (pak_ix).
void kilptietue::tallenna(INT d, INT comtarfl, INT kielto, INT pakota_lah, kilptietue *entkilp)
```

Tarkistuslista / Checklist:

- Tarkoitus yhdellä lauseella / purpose in one sentence
- Parametrit, joiden merkitys ei näy nimestä (`kielto`, `piste`, `mista`) /
  parameters whose meaning is not obvious from the name
- Paluuarvo, erityisesti virhekoodit / return value, especially error codes
- Sivuvaikutukset: globaalit, tiedostot, verkko, käyttöliittymä / side effects:
  globals, files, network, UI
- Esiehdot ja jälkiehdot / preconditions and postconditions
- Säikeet ja lukot / threads and locks
- Omistajuus: kuka vapauttaa muistin, staattiset puskurit / ownership: who frees
  memory, static buffers
- Yhteensopivuus: levymuoto, sanomamuoto, vanhat versiot / compatibility: disk
  format, message format, old versions

## Globaali muuttuja / Global variable

Esittelyn viereen (`*Declare.h`) tai määrittelyn viereen (`*Globals.cpp`), ei
molempiin:

Next to the declaration (`*Declare.h`) or the definition (`*Globals.cpp`), not
both:

```cpp
extern int ToimintaTila;  // 0 = no competition open, 1 = preparation, 2 = results service.
                          // Written by aloitus() and the main form; read everywhere.
```

## Monimutkaiset algoritmit / Complex algorithms

Lohkokommentti ennen algoritmia: mitä se ratkaisee, syöte ja tulos, tärkeät
erikoistapaukset sekä viittaus `doc/`-dokumenttiin, jos sellainen on (esim.
`doc/first-finish-parameter-analysis.md`).

A block comment before the algorithm: what it solves, input and output, important
special cases, and a reference to a `doc/` document if there is one (e.g.
`doc/first-finish-parameter-analysis.md`).

## GUI-lomakkeet / GUI forms

Lomakkeen tiedosto-otsikkoon: mitä lomake tekee, mistä se avataan, mitä
ytimen funktioita se kutsuu ja muuttaako se tietoja. Tapahtumankäsittelijöihin,
joissa on liiketoimintalogiikkaa (esim. `cbHk/UnitEmit.cpp`), kommentoidaan samoin
kuin ytimen funktioihin. Generoituun koodiin (`.dfm`, `__published`-osio) ei
kommentoida.

In the form file header: what the form does, where it is opened from, which core
functions it calls and whether it changes data. Event handlers containing
business logic (e.g. `cbHk/UnitEmit.cpp`) are commented like core functions.
Generated code (`.dfm`, `__published` section) is not commented.

## Mitä ei tehdä / What not to do

- Ei rivikohtaisia kommentteja ilmeiseen koodiin / no per-line comments on obvious code
- Ei kommentoidun koodin poistoa samassa muutoksessa / no removal of commented-out code in the same change
- Ei muotoilu-, sisennys- tai nimimuutoksia / no formatting, indentation or renaming
- Ei Doxygen-merkintöjä (`@param`) ilman erillistä päätöstä / no Doxygen tags without a separate decision
