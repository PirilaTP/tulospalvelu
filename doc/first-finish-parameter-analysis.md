# First finish rule: should "no other result needed" be its own parameter? (branch 56)

Question: today one flag, "first finish starts the next leg" (`FirstFinishStarts`), also makes the
other parallel runners' results unnecessary. Could or should there be a separate parameter for
"no other result needed"?

Short answer: yes, it is possible and it is worth doing if you need the combination
"next leg starts at the first finisher, but every runner must still finish validly". It is not
worth doing if the only rule you need is today's. The cheapest alternative is to keep one flag and
document that it means both things.

## 1. What the one flag does today

`sarjatietue::ekaMaaliLahettaa[leg]` (`VDef.h:385`), saved as `<FirstFinishStarts>1` in `KilpSrj.xml`
(`VXml.cpp:600, 1744`), edited in the series form row "Rinnakkaisen osuuden ensimmainen maaliaika
kaynnistaa seuraavan osuuden lahdon" (`UnitSarja.cpp:195, 255`, only when the leg has parallel
runners). It bundles five different behaviours:

| # | Behaviour | Code |
|---|-----------|------|
| a | Start time of the next leg = the first finisher's time (all slots of the next leg start together) | `vkilp.cpp:365` `LahtoEdellisenTuloksesta` |
| b | Team time of the leg = the first finisher's time, not the slowest | `vkilp.cpp:496` `aTulos` |
| c | Team status of the leg (OK / DSQ / DNF) = the first finisher's status; DSQ or DNF of the others is ignored | `vkilp.cpp:1050` `tTark`, `1418, 1438` `tHyv`/`Hyv`, `VRinnakkaisOsuus.cpp:72` `EkaMaaliOsuudenTila` |
| d | Waiting for the others is dropped: the missing-time surcharge is cleared when one runner has finished and no "partial time" is returned | `vkilp.cpp:449, 456`, `VRinnakkaisOsuus.cpp:40` `PuutelisaNollataan` |
| e | Display and printing follow the deciding runner: "1/2 missing" filters hidden, only the deciding runner's column shows the team time, only that runner's time and name in the PDF | `TulosUnit.cpp:276-279, 592-607`, `UnitJoukkuetiedot.cpp:351, 433`, `VTulostus.cpp:1893, 2710, 3133` |

Only (a) is about the start of the next leg. (b)-(e) are about "which result counts".

## 2. What "no other result needed" can mean

1. **The others' status does not matter** (DSQ/DNF of a non-deciding runner): already (c).
2. **The others' times are not needed to complete the leg** (no waiting, no surcharge): already (d).
3. **The others' results are not tracked at all**: they are not counted as open ("Avoinna"),
   not listed as missing, not required for checking. This is only partly done today. The Status
   window and headers count teams (my `avoinLkm`), but the per-runner lists still list the other
   runners as "Avoinna" until they finish.

So the existing flag already contains meanings 1 and 2. A new parameter would only add value if it
is *independent* of the start rule.

## 3. Combinations

| Starts at first finisher (a) | Only the first finisher counts (b-e) | Meaning | Status |
|---|---|---|---|
| yes | yes | today's `FirstFinishStarts` (the Nuorten Jukola demo) | works |
| yes | no | the next leg is sent by the first finisher, but the team result is the slowest finisher, and a DSQ or DNF of any runner is a team DSQ or DNF | not possible today |
| no | yes | best of N counts; the next leg starts by mass start / fixed start (`ylahto`), not from the leg result | almost possible: set the mass start of the next leg (`vkilp.cpp:356` takes the fixed start first); the flag turns (a) on though |
| no | no | all must finish, the next leg starts at the team result (the slowest) | the default for parallel legs |

Note on (yes, no): the next leg start is calculated from `tTulos(previous leg)`. With "slowest
counts" that time is the slowest runner's, so a separate "first finisher's time" function is
needed for the start. This is the main part of the work.

## 4. Proposal

Keep the existing flag for (a) and add one for (b)-(e):

* `ekaMaaliLahettaa[leg]` (XML `FirstFinishStarts`): the first finisher starts the next leg. Unchanged.
* new `ekaMaaliRatkaisee[leg]` (XML `FirstFinishDecides`; form row "Ensimmainen maaliin ratkaisee
  osuuden tuloksen ja tilan, muita ei tarvita (1=kyllä)"): result, status, waiting rules and the
  display of (b)-(e).
* **Compatibility:** a file that has `FirstFinishStarts=1` and no `FirstFinishDecides` is read with
  Decides = 1, so existing competitions behave exactly as before. The new tag is written always.
* Both only editable on legs with more than one runner (as now).

### Code touch points

| File | Change |
|------|--------|
| `VDef.h`, `vSarjat.cpp` | new array, default false, copy/clear with the series |
| `VXml.cpp` | read/write the tag; default Decides = Starts when the tag is missing |
| `UnitSarja.cpp` | second row (`lisarivit` handling), validation: parallel legs only |
| `vkilp.cpp` | `aTulos`, `tTark`, `Hyv`/`tHyv`, puutelisa code use Decides; the start of the next leg (line 365) uses Starts and a new `ekaTulos(leg)` (first finisher's time) when Decides is off |
| `VRinnakkaisOsuus.*` | `LahtoEdellisenTuloksesta` and `PuutelisaNollataan` take both flags; unit tests |
| `TulosUnit.cpp`, `UnitJoukkuetiedot.cpp` | the deciding-runner display follows Decides |
| `VTulostus.cpp` | the three uses (best time, names, result row) follow Decides |
| `VIx.cpp`, `VTulostus.cpp` (`ossijatRinn`) | meaning 3: do not list/count the others as open when Decides is on (optional) |
| help `liite_4._rinnakkaisosuusviestit.md` | describe both parameters and the four combinations |

### Tests

`Tests/VRinnakkaisOsuusTest.cpp` already has doctest cases for first finisher / status / start.
Add a truth table for the four combinations (start time, team time, team status, "waiting"). Add
demo variants for leg 3: (yes, no) and (no, yes). The branch 56 print checks (HTML, PDF, status
window) have to be repeated for the new combinations.

### Risks

* The start calculation for (yes, no) is new logic in `vkilp.cpp`, the most sensitive file for the results.
* Ranking and the status window use `tTulos`/`tTark`: they must see the same flag as the printing.
* A data model change touches saved files; forward compatibility is solved by the default rule,
  backward compatibility (an older program reading a new file) ignores the unknown tag, which then
  means "Decides = Starts".

### Effort

Medium: about eight source files, one new unit test table and a help page. Roughly a day including
the print and Status window checks.

## 5. Recommendation

* If the series only ever use the Nuorten Jukola rule (first finisher starts the next leg and
  decides the result): **do not add a parameter**. Rename the form row to say that the other
  runners' results are not needed, and document it in the help.
* If any relay needs "next leg starts at the first finisher but all must finish validly", or "best
  of N counts with a fixed start": **add `FirstFinishDecides`** as above, with the compatibility
  default.

Questions to settle before building:

1. Is the (yes, no) combination needed in a real competition, and what should the team time be then
   (slowest finisher, or the first finisher's time for ranking)?
2. Is the (no, yes) combination needed (best of N, fixed or mass start of the next leg)?
3. With "only the first counts", should the other runners still appear as open ("Avoinna") in
   per-runner lists until they finish (today) or not at all (meaning 3)?

## 6. Implemented (branch 56)

The proposal was implemented with the start rule on the leg that is started:

* `sarjatietue::ekaMaaliRatkaisee[leg]` (XML `OnlyFirstFinishCounts`, on the parallel leg): the first
  finisher's time and status count, the others are not needed. This is the old `ekaMaaliLahettaa`
  flag renamed; all result, status, waiting and display code uses it.
* `sarjatietue::lahtoEdEkaMaalista[leg]` (XML `StartsAtPreviousFirstFinish`, on the leg that follows
  a parallel leg): the leg starts when the first runner of the previous parallel leg finishes.
  `vkilp.cpp` `Lahto()` uses it; when the previous leg does not have `OnlyFirstFinishCounts`, the
  start uses the new `kilptietue::ekaMaaliTulos()` (the first finisher's time) and not the team result.
* Old files: `FirstFinishStarts` on leg n is read as `OnlyFirstFinishCounts` on leg n and
  `StartsAtPreviousFirstFinish` on leg n+1, so existing competitions behave as before. Only the new
  tags are written.
* Form `Sarjan lisäys ja muokkaus`: two rows with the labels from section 4; the first can be edited
  only on parallel legs, the second only on a leg whose previous leg is parallel; the label column is wider.
* Help: `liite_4._rinnakkaisosuusviestit.md` describes both rules and the four combinations.

Verified on a copy of the demo with the same data and four XML variants (HTML prints of the leg 3
team list and the leg 4 runner list):

| Variant | Leg 3 team 139 (runners 12.01 / 10.51 / 11.11) | Leg 4 time of team 139 | Others |
|---|---|---|---|
| old `FirstFinishStarts` | 1.03.02 | 13.53 | reference |
| new tags, both on | identical to the reference (both files byte-identical) | identical | |
| start only (everyone needed) | 1.04.12 (slowest) | 13.53 (starts at the first finisher) | team 105 (3B DSQ) Hyl., team 108 (3C DNF) Kesk. and neither is in the leg 4 list |
| neither | not printed | 12.43 (starts at the team result, the slowest) | |

The form round trip was checked too: saving writes both tags, clearing both cells removes them.

Parallel to parallel (Halikko-viesti copy, leg 3 with `OnlyFirstFinishCounts`, leg 4 without its own
mass start, leg 4 runner list for slots 4A/4B/4C):

| Start flag on leg 4 | 4A | 4B / 4C |
|---|---|---|
| on | starts at the first finisher of leg 3 | same start, so longer times (team 1: 31.08 / 30.17) |
| off | same as on | start at the 2nd / 3rd finisher of leg 3, so shorter times (team 1: 30.16 / 28.47) |

Note: a `MassStart` on the leg wins over both rules, so the original Halikko data (leg 4 mass start 18:38:27)
gives identical output for both settings.

Not verified: the console build and the doctest suite (`Tests/`).
