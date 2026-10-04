# Parallel-leg relays: printing analysis (branch 56)

Scope: ViestiWin result printing (`Tulosteet`, HTML output) for relays with parallel
runners on a leg (e.g. Nuorten Jukola 7 legs, leg 3 = 3A/3B/3C, first finish starts leg 4).
Code: `TPsource/V52/Juk/VTulostus.cpp`, `VIx.cpp`, `vkilp.cpp`.

Status of the changes described here: **uncommitted** in the 56 worktree.

## 1. How it was investigated

- Demo competition `C:\Temp\nj2026_demo` (generator: `make_demo.py`): 40 teams, 7 legs,
  leg 3 has 1-3 runners per team, DSQ/DNF/DNS cases per the first-finish rule, plus
  `nj2026_open.csv` (3 still-running teams) -> 43 teams.
- Prints made from the Tulosteet form (HTML), compared with the source csv.
- Temporary debug logging (reverted) showed the leg-time chain was populated
  (`n=39 ntulosix=39 josalku=15`), so the empty table was not a data/chain problem.
- Same empty table reproduced on a build of the repository's main checkout (branch 73),
  i.e. not caused by the branch 56 work.

## 2. Problems found and fixed

| # | Symptom | Root cause | Where | Fix |
|---|---------|------------|-------|-----|
| 1 | "Osuuskohtaiset tulokset" for one leg printed a header and no rows | Every output branch only wrote a row when `kilpparam.maxnosuus == 1` | `tulostasarja` (HTML, printer, text, console) | Per-runner list (`ixjarj = 10`) when `maxnosuus > 1`; rows printed per runner slot (`slotjrj`) |
| 2 | Possible heap overflow with parallel legs | `ossijat` result buffer and `osjarj` array sized `nilm+1`, but a parallel leg yields up to `nilm * maxnosuus` entries | `ossijat`, `tlsSeuraava` case 10 | Sized by `maxnosuus` (also fixes the existing `kaikkisijat` path) |
| 3 | Keskeytti/Hylatty counts in the per-runner header would be 0 | `ossijat` zeroes the counters it is given | `tulostasarja` | Counters saved and restored in per-runner mode |
| 4 | "Avoinna" count included empty 3B/3C slots (41 instead of 2) | `navoin/navoint` counted every slot with status "-" and no time | `addjarjarr`/`remjarjarr` in `VIx.cpp` | Slot counted only if it has a name, chip code or time (`onPaikkaKaytossa`) |
| 5 | Hylatyt list missed a DSQ team (3 teams in header, 2 printed); filters wrong for legs after the parallel leg | Filters used the status of one runner slot, and used the leg number as a slot number | `tulostasarja` filter checks | `osuusTila()` helper: team-level status on parallel legs, correct slot otherwise |
| 6 | Text output of leg results returned nothing | `textosuus` returned immediately when `maxnosuus > 1` | `textosuus` | Guard removed |

Verified on the demo (HTML, leg 3 unless noted). These prints ran on the build before the
last edit, which added `kilpparam.maxnosuus > 1` to the `slotjrj` condition; that edit
cannot change anything in a competition with parallel runners, and the newest exe was
built but not used for these prints:

| Print | Expected (from csv) | Printed |
|-------|--------------------|---------|
| Osuuskohtaiset, leg 3 | 65 runners with approved leg time, ordered by leg time, ties ranked | 65 rows, same runners and times (no difference over 1 s), tie ranks 1,1,3 |
| Hylatyt, leg 3 | teams 117, 111, 114 | 117, 111, 114 |
| Keskeyttaneet, leg 3 | 120, 123, 126 | 120, 123, 126 |
| Avoimet, leg 3 | 141 only | 141 only |
| Avoimet, leg 6 | 141, 142, 143 | 143, 142, 141 |
| Hylatyt, leg 6 | 135 | 135 |
| Keskeyttaneet, leg 7 | 136 | 136 |
| Header "Avoinna", leg 3 (before the open teams were added) | 2 | 2 |

## 3. Open problems and risks

Ordered by how likely they are to matter at a race.

1. **No list of all disqualified/retired runners per leg.** Hylatyt/Keskeyttaneet lists are
   per team. A DSQ of a runner who did not decide the leg (first-finish rule) appears in no
   printout (demo: Anni Karppinen team 105, Veeti Jokela and Santeri Moilanen team 129).
   This follows the rule, but the race office may want those runners listed.
2. **"Osuuskohtaiset tulokset" combined with Hylatyt/Keskeyttaneet/Avoimet on a parallel
   leg is still wrong.** Before the fixes it listed the *approved* runners of a DSQ team
   (team 111: Nea Siekkinen, Viljami Lampinen) and not the disqualified one, and the HTML
   had no series title or table opener (the header is written when the first row is
   printed, and the first rows were skipped). **Not re-tested after the last build.**
3. **"Kaikki osuudet" (monios) on a parallel competition was empty before; not
   re-tested.** It now goes through the same per-runner path, so it may print, but this has
   not been checked.
4. **Header counts mix units on a parallel leg.** In the team-level lists "Tuloksia",
   "Kesk" and "Hyl" count teams (leg 3: 35/3/3), but "Avoinna" counts individual runners
   (9) and includes runners of teams closed by an earlier DNF (the docs describe this),
   while the Avoimet list is per team and excludes closed teams. In the per-runner
   printout "Tuloksia" counts runners (65). These are existing semantics, but a reader
   sees "Avoinna :9" above a list of one team.
5. **Printer (GDI), plain-text and console branches were changed in the same way but not
   exercised.** The console branch is only compiled in the Visual Studio build and was
   not compiled at all.
6. **Per-runner rows are not labelled with the leg code.** Runners of the same team
   appear on separate rows with the same team number; 3A/3B/3C is not shown (`htmlosuus`
   only adds the leg code in monios mode).
7. **Empty lists print nothing, not even the series title** (n = 0 skips the series).
   Existing behaviour, but it looks like a failure for Avoimet.
8. **Buffers sized by `nilm[]` elsewhere were not audited.** Problem 2 above was found by
   reading; other arrays may carry the same one-runner-per-team assumption.

Test-data caveat: in the demo csv the "Sukunimi" column holds first names and "Etunimi"
holds surnames (generator bug). ViestiWin prints them as "Lastname Firstname", which looks
right, so it does not affect any result above.

## 4. Effect on normal relays (no parallel legs)

Normal relay = every series has `nosuus[leg] == 1`, so `kilpparam.maxnosuus == 1`.
Verified preconditions in the code:

- `nosuus[i] = 1` by default (`vSarjat.cpp:61`); `NumberOfCompetitors` is written to XML
  only when > 1.
- `aosuus[0] = -1` and `aosuus[os+1] = aosuus[os] + nosuus[os]` (`vSarjat.cpp:182-185`,
  `VXml.cpp:702-703`), so with `nosuus == 1` the first slot of leg `os` is slot `os`.
- `kilpparam.maxnosuus` is rebuilt from the series in `maaraaOsuusluku` (starts at 1).

Every changed hunk, evaluated for a normal relay:

| Hunk | Normal relay |
|------|--------------|
| `ossijat`: `else if (maxnosuus > 1) i *= maxnosuus` | condition false, unchanged |
| `tlsSeuraava`: `n_osjarj = nilm*maxnosuus+1` | `maxnosuus == 1` -> `nilm+1`, identical |
| `list()`: `ixjarj = (monios \|\| maxnosuus > 1) ? 10 : 0` | identical to `monios ? 10 : 0` |
| `slotjrj` (requires `maxnosuus > 1`) | always false; karkiaika, rank, print calls and counter restore take the original branch |
| `maxnosuus == 1 \|\| slotjrj` in the 4 output branches | true exactly as before, same arguments (`slotjrj ? osd : os` -> `os`) |
| `textosuus`: guard `if (maxnosuus > 1) return` removed | guard was false for normal relays |
| `osuusTila()` in the Hylatyt/Keskeyttaneet/Avoimet checks | `nosuus[os] == 1` -> `Tark(aosuus[os]+1, n)` = `Tark(os, n)`; out-of-range/negative `os` falls back to the old call; `wTark(x,2)` == `ansitowchar(Tark(x,2))` |
| `addjarjarr`/`remjarjarr`: `nosuus[osuus] == 1 \|\| onPaikkaKaytossa()` | short-circuits to true, counters unchanged |
| `onPaikkaKaytossa`, `nkesk0/nhyl0` | new function unused; plain copies |

Conclusion from the code: a competition with no parallel leg takes exactly the same
paths as before. A competition that has parallel legs in *some* series is affected for all
its series: before, "Osuuskohtaiset tulokset" printed nothing for every series; now it
prints.

**Not verified by running it.** A before/after print comparison was prepared but could
not be run (permission to drive the GUI was declined):

- Data: `C:\Temp\normal_relay` - 4-leg relay, 24 teams (DSQ legs 2 and 3, DNF legs 2 and
  4, DNS leg 3, running teams on legs 2/3/4, one tied pair).
  Generator `make_normal.py`, series file `KilpSrj.xml`, entries `normal_relay.csv`.
- To run: create the competition from `KilpSrj.xml`, import the csv, then print the same
  lists with the main-checkout exe and the 56 exe and diff the HTML:
  Tulokset (Lopettaneet, Lopputulos and legs 2-3), Hylatyt leg 2, Keskeyttaneet leg 3,
  Avoimet leg 2, Osuuskohtaiset tulokset legs 2-3, and "Kaikki osuudet".
  The monios case matters most because it also uses `ixjarj = 10`.
