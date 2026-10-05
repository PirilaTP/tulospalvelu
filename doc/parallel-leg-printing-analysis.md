# Parallel-leg relays: printing analysis (branch 56)

Scope: ViestiWin result printing (`Tulosteet`) for relays with parallel runners on a leg
(e.g. Nuorten Jukola, 7 legs, leg 3 = 3A/3B/3C, first finish starts leg 4).
Code: `TPsource/V52/Juk/VTulostus.cpp`, `VIx.cpp`, `vkilp.cpp`, `VDeclare.h`.

## 1. How it was investigated

- Demo competition `C:\Temp\nj2026_demo` (generator `make_demo.py`): 40 teams, 7 legs, leg 3
  has 1-3 runners per team, DSQ/DNF/DNS cases from the first-finish rule, plus
  `nj2026_open.csv` (3 still-running teams) = 43 teams.
- Prints from the Tulosteet form (HTML, text, printer to PDF), compared with the source csv.
- Temporary debug logging (reverted) showed the leg-time chain is populated
  (`n=39 ntulosix=39 josalku=15`), so the first empty table was not a data/chain problem.
- A normal 4-leg relay (`C:\Temp\normal_relay`, 24 teams) was printed with the main-checkout
  exe and with the new exe and the outputs compared (section 4).

## 2. Problems found and fixed

| # | Symptom | Root cause | Fix |
|---|---------|------------|-----|
| 1 | "Osuuskohtaiset tulokset" for one leg printed no rows | every output branch wrote a row only when `kilpparam.maxnosuus == 1` | per-runner list (`ixjarj = 10`, `ossijatRinn`) when `maxnosuus > 1`; rows per runner slot in HTML, printer, text and console code |
| 2 | possible heap overflow | `ossijat` buffer and `osjarj` array sized `nilm+1`, parallel leg needs `nilm*maxnosuus` | sized by `maxnosuus` |
| 3 | counts line wrong in the per-runner list | `ossijat` zeroes the Kesk/Hyl counters | per-runner counts computed in `ossijatRinn` |
| 4 | "Avoinna" counted empty 3B/3C slots (41 instead of 2) | `navoin/navoint` counted every slot with status "-" and no time | slot counted only with a name, chip code or time (`onPaikkaKaytossa`) |
| 5 | Hylatyt/Keskeyttaneet/Avoimet lists missed a DSQ team | filters read one runner slot, and used the leg number as a slot number | `osuusTila()` gives the team-level status on parallel legs, the right slot otherwise |
| 6 | no text output of leg results | `textosuus` returned immediately when `maxnosuus > 1` | guard removed |
| 7 | "Osuuskohtaiset" + Hylatyt/Keskeyttaneet/Ei-lahteneet/Avoimet listed the approved runners of a DSQ team, no heading | team chain used, rows then filtered per team | per-runner collector applies the filter to each runner; status text from the runner's own status; heading printed on the first row |
| 8 | header counts mixed teams and runners | team counters next to the runner counter `navoin` | per-runner list: all counts are runners; team-level list on a parallel leg: "Avoinna" counts teams (`otsikkoLuvut`) |
| 9 | runners of a parallel leg indistinguishable | team number only | number column shows leg code (`105-3A`) in HTML, printer and text |
| 10 | empty list printed no title (looked like a failure) | series skipped when the list is empty | title, counts line and empty table are printed (HTML, parallel competitions only) |
| 11 | "Kaikki osuudet" printed an empty file | all-legs mode set `os = osuusluku - 1` (runner slots, 8 for NJ) instead of the last leg (6); the row count was read from a slot that is never filled, `n = 0`, whole list skipped | `os = ntosuus - 1` (identical when each leg has one runner) |
| 12 | text rows without a rank started with ". " | `"%s. %s"` with empty rank | omitted in the per-runner list |

Verified on the demo (HTML unless noted):

| Print | Expected (from the csv files) | Printed |
|-------|--------------------|---------|
| Osuuskohtaiset, leg 3 | 67 approved runners, leg-time order, tie ranks, header 67/6/7/7 | same, rows `105-3A ...` |
| Osuuskohtaiset + Hylatyt, leg 3 | 7 disqualified runners | the 7, status "Hyl." |
| ... + Keskeyttaneet | 6 | the 6 |
| ... + Avoimet | 7 (142-3A/3C and 143-3B/3C included, closed team 132 not) | the 7 |
| ... + Ei-lahteneet | 1 (102-3C) | 102-3C |
| "Kaikki osuudet" (Lopputulos) | 280 rows: 42/42/34+17+13/35/34/32/31 per leg | exactly that, time order, tie ranks |
| Same lists as continuous text | 67 lines; 7 DSQ runners | OK |
| Hylatyt as printer output (Microsoft Print to PDF) | heading, counts, 7 runners with leg codes | OK, layout checked |
| Keskeyttaneet / Avoimet / Ei-lahteneet, leg 3, continuous text | 6 / 7 / 1 runners | the 6 (108-3C, 120-3A/3B/3C, 123-3A, 126-3A), the 7 (141-3A/3B/3C, 142-3A/3C, 143-3B/3C), 102-3C; no ". " prefix |
| Same three lists as printer output (PDF) | same runners with leg codes | OK; header is the leg's runner counts 67/6/7/7 for every list |
| Status window (Seuranta > Status), demo | leg 3 row = header; totals = sum of legs | leg 3: 43 teams, 35 finished, 3 Kesk, 3 Hyl, 1 Ei laht., 1 Avoinna (adds to 43); totals 251/5/4/31/27 equal the leg sums |
| Team level Hylatyt, leg 3 | 117, 111, 114; header 35/3/3/1 | OK |
| Team level Avoimet, leg 3 / leg 6 | 141 / 141, 142, 143 | OK |
| Team level empty list (Hylatyt, leg 1) | title and counts | "NJ - 1. osuus", "Lahti : 43 Keskeytti : 1 Hylatty : 0", empty table |
| Per-runner empty list (leg 1) | title and counts | OK |

## 3. Open problems and risks

1. **Console build compiles, but was not run.** `JukMaali520.exe` was built (Debug, 5 Oct
   12:11, after the last source edit) and its `.pdb` contains `rinnLista`, `ossijatRinn`,
   `otsikkoLuvut`, `avoinLkm`, `onPaikkaKaytossa`, so the `#ifdef _CONSOLE` code compiles.
   Console output of the parallel-leg lists has not been tested.
2. **Coverage.** All status lists were run in HTML, text and printer (PDF) for leg 3 of the
   demo. Not run: other legs in text/printer. The empty-list title is printed in HTML only; in
   text and printer an empty list prints nothing, as in normal relays (left as is, because
   changing it would alter normal-relay output).
3. **Status window "Avoinna" is aligned with the printouts (verified on the demo).**
   `UnitStatus.cpp` uses `avoinLkm(srj, os)` (`VIx.cpp`); on a parallel leg it counts open
   teams still in play, like the header. Leg 3 shows 1 and the total row is the sum of the legs.
4. **Per-runner lists show a DSQ of a non-deciding runner; team-level lists do not.** This is
   by design (first-finish rule: the list of a leg shows each runner's own status, the team
   list shows the team's), so it is not changed.
5. **Normal relay, "Osuuskohtaiset" + Hylatyt/Keskeyttaneet/Avoimet/Ei-lahteneet now lists runners**
   (before: empty file). Verified on `C:\Temp\normal_relay` (HTML): Hylatyt leg 2 -> 105,
   Keskeyttaneet leg 2 -> 107, Avoimet leg 2 -> 118 "Avoinna", Ei-lahteneet leg 3 -> 115
   "Ei laht.". An empty list still prints no title there, and the all-legs list is skipped if
   nobody has finished the last leg (unchanged).
6. **Buffers (audited).** Every allocation in `Juk/` and `ViestiWin/` sized by a team or
   record count was read: `vApu.cpp:648` (`d[nilm]`) and `UnitArvonta.cpp:161` (`rjrj[nilm]`) are
   per-team arrays indexed per team, `UnitSeuraval/UnitSuodatus` are per club, `UnitSakkoKierr`
   uses `maxrec` with a fixed slot dimension. Only `ossijat` and `osjarj` were per-runner and
   are fixed (problem 2).

## 4. Effect on normal relays (no parallel legs)

Normal relay = every series has `nosuus[leg] == 1`, so `kilpparam.maxnosuus == 1`.
Preconditions checked in the code: `nosuus[i] = 1` by default (`vSarjat.cpp:61`);
`aosuus[0] = -1` and `aosuus[os+1] = aosuus[os] + nosuus[os]`, so the first slot of leg `os`
is slot `os`; `kilpparam.maxnosuus` is rebuilt from the series (`maaraaOsuusluku`).

| Change | Normal relay |
|--------|--------------|
| `ossijat` buffer, `n_osjarj`, `ixjarj` choice | `maxnosuus == 1`: same values/branches |
| `rinnLista`, `slotjrj`, `ossijatRinn`, row/label/status changes in `html/prt/textosuus` | require `maxnosuus > 1`, never run |
| `osuusTila()` in the Hylatyt/Keskeyttaneet/Avoimet checks | `nosuus[os] == 1` -> `Tark(aosuus[os]+1, n)` = `Tark(os, n)` |
| `navoin/navoint` | `nosuus[osuus] == 1` short-circuits to true |
| `otsikkoLuvut` (header) | returns the original array values unless the leg has parallel runners |
| empty-list title | requires `maxnosuus > 1` |
| all-legs `os = ntosuus - 1` | `ntosuus == osuusluku` |

**Verified by running both builds.** `C:\Temp\normal_relay` (4 legs, 24 teams: DSQ legs 2
and 3, DNF legs 2 and 4, DNS leg 3, running teams on legs 2/3/4, a tied pair) was opened with
the main-checkout exe (branch 73, built 10-02) and with the new exe, and the same lists were
printed: Tulokset (Lopettaneet; Lopputulos and leg 2), Hylatyt leg 2, Keskeyttaneet legs
2 and 3, Avoimet leg 2, Osuuskohtaiset (Lopettaneet legs 2 and 3, Hylatyt leg 2,
Lopputulos + Kaikki osuudet), Tulokset with Lopputulos, the leg 2 list as continuous text,
and the leg 2 list printed to PDF. **All 10 HTML files and the text file are byte-identical;
the PDF text is identical** (print time ignored). The main-checkout exe is the branch 73
build, not a pure `main` build.
