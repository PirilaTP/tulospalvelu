# Appendix 4. Relays with parallel legs

## Appendix 4. Relays with parallel legs

The standard version of the software can also be used
in relays with parallel legs, such as the Halikko relay and some regional relays.
To enable the required additional features,

- in the function *Competition creation and basic properties*
  - the maximum number of parallel runners is given
    (for example 3 in the Halikko relay)- space for legs is reserved according to the maximum team size
      (for example 15 in the Halikko relay)- the classes must be defined so that on the form
    handling one class
    - the number of legs is given as the number of consecutive
      legs, not as the number of runners in the team
      (i.e. 7 rather than 15 in the Halikko relay)- the maximum number of parallel runners for the class
        is given (in some classes this may be 1, in which case the class
        is a normal relay class)- the leg-specific data then gets the possibility
          to enter the number of parallel runners on each leg. The number of the team's
          runners is the sum of these values- all other leg-specific data is given so that parallel
            legs are not handled separately.- In a class containing parallel legs, changes to the numbers of legs
      must be made on the single-class form; the class table rejects such changes
      written there.

For parallel legs the program almost always uses codes consisting of
the leg's sequence number and a letter A, B, ... Thus, for example,
the legs of the Halikko relay are

- 1- 2A, 2B, 2C- 3A, 3B, 3C- 4A, 4B, 4C- 5A, 5B, 5C- 6- 7

Some files, however, use numbering
that corresponds to the runner's position in the competitor data, in which case, for example,
the legs of the Halikko relay are numbered 1-15. This applies e.g. to the CSV
competitor data transfer file, which the program writes and reads, interpreting the leg
according to the heading row. In multi-class relays with parallel legs, a plain
number is also used on the punch check display, because the relationship between the leg's
number form and the leg code is then usually not the same in different
classes.

When a class has parallel legs, this is automatically taken into account
in calculating results, on the monitoring displays and in the results lists. In the formatting
of results lists, however, field widths are not increased automatically, so
the user must change the formatting, e.g. for leg-specific outputs;
the code of a parallel leg (e.g. 105-3A) fits in the number field only if the field
is wide enough. Final results
containing all legs do not require a change to the formatting.

On the monitoring display and in the headings of results lists, *Open* means

- in a competition with parallel legs: the number of teams
  that have no time on that leg and no marking of disqualification,
  did not finish or did not start, and whose earlier leg marking has not
  closed the leg. Later legs of teams closed because of an earlier leg marking
  are therefore not open. On a parallel leg (e.g. 3A, 3B,
  3C) the team is open until the result of the leg has been decided.
- in an ordinary relay without parallel legs: the number of individual
  participants who have no time or marking of disqualification,
  did not finish or did not start. This also includes those legs that have been
  closed because of such a marking on an earlier leg, unless the marking has been
  extended to later legs.

## Outputs in a competition with parallel legs

*Results by leg* are output in a competition with parallel legs
as a runner-by-runner list: each runner is on their own
row in order of leg time, and on parallel legs
the number field contains the team number and the leg code (for example 105-3A).
Equal times get the same place. The numbers in the list heading (results, did not finish,
disqualified, open) are numbers of runners in this list.
In a team-by-team list (e.g. *Results*) the same numbers are numbers of
teams.

With the *Output restriction* selections *Disqualified*, *Did not finish*,
*Not started* and *Open*, the leg-specific list contains only runners
with the selected status; the status is each runner's own status, and runners of teams
closed because of an earlier leg marking are left out of the open ones. On a parallel leg
the status of the first runner to finish determines the status of the team's leg
(see below), but the runner-by-runner list also shows the disqualifications and
non-finishes of the other parallel runners, so its numbers may differ from
those of the team-by-team list. A list with no rows at all
is written to an HTML file with its heading and numbers; nothing is written
for an empty list to a text file or to the printer.

The selection *All legs* writes all leg times into one list
in time order. In a text file the place is the place achieved on the leg;
for the printer a running sequence number is written.

The same leg-specific restrictions also work in an ordinary relay: a leg-specific
*Disqualified*, *Did not finish*, *Not started* or *Open* list writes
those runners (previously an empty file was output).

## The first finisher of a parallel leg decides and starts the next leg

In a class containing parallel legs, two independent
rules can be selected. The settings are made on two rows of the table of the
*Add and edit class* form, which are shown automatically when
the class has at least one parallel leg.

1. *Result of a parallel leg is decided by the first finisher, others
   not needed (1=yes)*. The value 1 is written in the column of the parallel leg (e.g. 3).
   The team's leg result and status are then not awaited from all
   parallel runners (e.g. 3A, 3B and 3C) but are determined as soon as
   the *first* of them finishes.
2. *Leg starts when the first runner of the previous parallel leg
   finishes (1=yes)*. The value 1 is written in the column of the leg (e.g. 4)
   that follows the parallel leg. The start time of the leg is then the finish time of the first
   parallel runner. If the leg is itself parallel,
   all its positions start together.

The rules can be selected separately:

| First decides (leg 3) | Starts from first (leg 4) | Effect |
|---|---|---|
| yes | yes | Youth Jukola rule: the first decides the result and starts the next leg |
| no | yes | The next leg starts from the first, but the team's leg result and status require the results of all parallel runners: the leg time is the time of the slowest, and one runner's disqualification or non-finish disqualifies the team or makes it a non-finisher |
| no | no | Normal parallel leg: the result waits for everyone, the next leg starts from the team's leg result (slowest) |
| yes | no | The first runner's time is the team's leg time; an ordinary (single-runner) next leg still starts from this time. If the next leg is itself parallel, its first position (e.g. 4A) starts from the first finisher, the second (4B) from the second finisher and the third (4C) from the third finisher |

If the leg has been given its own mass start time (*MassStart*), it takes precedence
and neither rule changes the start time.

The shared setting in old class files (*FirstFinishStarts*) is read
automatically as both rules (the first decides the leg and
the next leg starts from it), so earlier competitions work as before.
When saving, the settings are written to the *KilpSrj.xml* file with the leg-specific
tags *OnlyFirstFinishCounts* and *StartsAtPreviousFirstFinish*.

When the first finisher decides the leg:

- The time of the parallel runner who finishes first is recorded as the team's leg result
  and place — the times of the other parallel
  runners are no longer awaited.
- If the next leg uses start from the previous first finisher, the same
  finish time also starts the team's next leg start time, i.e.
  the changeover happens as soon as the first parallel runner arrives, not only
  when all three have handed over.
- If the runner who finished first is later disqualified, the team's
  leg result inherits this disqualification — it is not replaced by the approved
  time of another parallel runner (e.g. one who finished
  later).
- The disqualification or non-finish of the runner who finished first is the status of the team's
  leg, even if the other parallel runners are approved.
  The disqualification or non-finish of the other parallel runners, on the other hand, does not
  affect the team's result when the first finisher is approved.
- The *Emit data* form shows the notice "FIRST AT FINISH. Disqualification
  (DSQ) or non-finish (DNF) decides the team's result." when the leg
  is decided by the first finisher, the runner is the first at the finish and
  the Check selection is *Proposal*. The notice is not shown for the other parallel
  runners or with other statuses.
- What counts is the first event that has a time. A disqualification or
  non-finish decides the team's leg only if a time has been recorded for it
  (in the finish time field): it then remains decisive even if someone else
  later finishes as approved. A disqualification or non-finish without a time
  is not treated as the team's result while someone else is still
  on the course — the leg remains open. If all parallel runners are
  out and nobody has a time, the team gets the worst marking.
- A parallel runner with no name recorded is included if
  they have an Emit code or a time.
- Once the first finish time has determined the start of the next leg,
  it remains in effect even if the program later calculates a new
  mass start time for the class — the mass start therefore does not reopen a start that is already locked.

The feature is also visible on the monitoring and results displays:

- On the *Standings at point* display, the name of the deciding (first finishing)
  runner is shown in bold on the parallel leg row; the names of the other parallel
  runners appear in normal style. The filters *1 missing*
  and *2 missing* are hidden automatically when the leg being viewed
  uses this setting, because the number of "missing" is no longer meaningful
  information once the first time has decided the result; the filter
  *Full leg* still works.
- In the split time table of the *Team data* display (*Show from splits* → *Result*)
  the team's shared result is shown only in the deciding runner's column;
  the columns of the other parallel runners show 00:00:00. If you want to
  see each parallel runner's own personal split time,
  select *Show from splits* → *Leg res*; in this mode, however, the table is
  read-only and cannot be edited.
