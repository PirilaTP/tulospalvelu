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
| 12 | text rows without a rank started with ". " | `"%s. %s"` with empty rank | omitted in the per-runner list, also on single-runner legs of a parallel competition (found when testing leg 6) |
| 13 | "Kaikki osuudet" as text printed every runner once per leg (1960 lines instead of 280) | text branch looped over the legs and called `textosuus` each time, but the per-runner list already has one row per runner | one call per runner when the list is per-runner (found testing item 3; 280 lines, no duplicates after the fix) |
| 14 | `Tulokset` + `Kaikki osuudet`: all legs after a parallel leg missing for a team whose non-deciding runner is DSQ (team 105 stopped after 3C) | the "stop listing" flag `khfl` was set by any runner that is not approved, including a DSQ of a runner who does not decide the team | on a parallel leg the flag is set only when the team itself is not approved (`tHyv`); HTML and printer |
| 15 | team leg time and place missing on a parallel leg when the team has fewer than three runners | printed only on the last slot row (3C), which is empty | new `tulosPaikka()`: the deciding runner's row (first-finish rule) or the last used slot; HTML and printer |
| 16 | garbage characters in empty time cells of the all-legs HTML/text rows | `oas` filled with spaces but never terminated | terminator added |
| 17 | `Tulosta yhteenveto` HTML showed `K&Atilde;&curren;rkiaika`-style text | the summary file had no `<head>`, so no charset; viewers guess ANSI | proper header with the charset of the chosen encoding |
| 18 | clearing a finish time in Joukkuetiedot (Maali column, split table) stored 00:00:00, so the runner got a leg time of 24h minus the start (127-3A/3B: 11.50.01) | `wstrtoaika_vap` returns 0 for an empty string when `t0 == 0` | an empty cell stores "no time" (`TMAALI0`). Old bad values must be cleared again. Other places that clear times were not checked |
| 19 | `105-3A` could wrap at the hyphen in the HTML number column | normal line breaking | wrapped in `white-space:nowrap` (HTML only) |
| 20 | team list of a parallel leg: last column printed every runner's time joined with "/" (`---/---/`), cut off by the column width so the real time was lost | `osuustlsst` listed all parallel runners | with the first-finish rule only the deciding runner's leg time is printed; legs without it still list all times. Checked in printer (PDF) output only |
| 21 | HTML team list of a parallel leg showed one runner's time only | single time column | all parallel runners' results on one line in a nowrap cell: time, `H` DSQ, `K` DNF, `E` DNS, `-` none/empty slot (`10.51/H/-`) |
| 22 | PDF/paper/text team list of a parallel leg (first-finish rule) | all times do not fit the column | only the deciding runner's leg time (row 20 behaviour); the HTML keeps all times. When all three times exceed one hour a full list would not fit the PDF (not built, not tested) |
| 23 | PDF: no runner names in a competition with parallel legs (any leg) | the name was built (`osuusnimist`) but never written | one name: the leg's runner, the deciding runner on a first-finish leg, or when nobody has a time the runner whose mark (E, K, H) decides, else the first runner |
| 24 | PDF: leg time of single-runner legs in a parallel competition printed with colons (`28:15`) | `aikatowstr_cols_n` | dots like other times |
| 25 | PDF: long parallel time lists were cut at the page edge | fixed field width | the text starts further left by its excess length (position unit calibrated by eye for the default layout) |

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

1. **Console build compiles and runs, per-runner list not run.** `JukMaali520.exe` (built
   12:11, before fixes 12-13) starts on the demo; the team-level Avoimet list for leg 3
   prints team 141 correctly. The per-runner screen (`naytaosuus`) was not reached. Its main
   menu shows Avoinna 35 (`navoint`, closed teams included), the Status window now 10.
2. **Coverage.** Text, HTML and printer (PDF) were run for leg 3; other legs: Hylatyt leg 6
   (text, PDF), Keskeyttaneet legs 1 and 7 (text), Avoimet legs 2, 4, 5 (text; leg 4 and 5
   also PDF/HTML), Ei-lahteneet leg 7 (HTML: empty, correct). "Kaikki osuudet": PDF 6 pages,
   ranks 1-280, and text 280 lines (after fix 13); the normal relay's all-legs PDF is complete.
   One earlier all-legs PDF was a single page ending at rank 31 and could not be reproduced
   (two later runs: 6 pages). Rank in the all-legs list is the leg rank in text and a running
   number in PDF (same in a normal relay, left as is). The empty-list title is printed in HTML
   only; in text and printer an empty list prints nothing, as in normal relays.
3. **Status window "Avoinna" is aligned with the printouts (verified on the demo).**
   `UnitStatus.cpp` uses `avoinLkm(srj, os)` (`VIx.cpp`); on a parallel leg it counts open
   teams still in play, like the header. Leg 3 shows 1 and the total row is the sum of the legs.
3a. **Status vs list "Avoinna" on single-runner legs: aligned for parallel-leg competitions.**
   Before: leg 5 showed 6 in the Status window (`navoin`, includes teams closed by an earlier
   K/E) and 2 in the Avoimet list. Now `avoinLkm` (also used by the team-level list headers
   through `otsikkoLuvut`) counts, whenever `maxnosuus > 1`, teams whose leg has no result and
   whose team is not closed (`tSulj`). Verified on the demo: Status Avoinna per leg 0/0/1/1/2/3/3,
   total 10 (was 27), equal to the printed headers (leg 4: 1, leg 5: 2, leg 6: 3, leg 7: 3).
   Normal relays (`maxnosuus == 1`) still use `navoin` (closed teams included), unchanged. The
   console main menu total (`navoint`, 35 on the demo) still counts runners of closed teams;
   the console was not rebuilt.
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
