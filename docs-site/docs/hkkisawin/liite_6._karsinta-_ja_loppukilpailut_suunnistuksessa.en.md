# Appendix 6. Qualification and final competitions in orienteering

## Appendix 6. Qualification and final competitions in orienteering

Orienteering competitions with a qualification
and a final typically have qualification classes that take part only in the first
stage, and finals that are contested only in the second
stage. Because competitors compete in different classes in different stages,
you must select in the competition settings that the class changes from one stage to the next, and
in most cases also that the bib number changes from one stage to the next.

In some cases a preliminary competition that determines the start order
is held without qualification in the first stage, and
then a final of the same class in the second stage, in which case the class
takes part in both stages. Entries are typically recorded first
in the class that is also the A final, and moved from there to the qualification classes.
During the recording of entries it is better that the classes used for this are
defined as including all stages, but after the move
to the qualification classes has taken place, the stages can be marked to correspond to the actual
competition stages.

After the entries have been saved, you typically
have to record ranking points in the entry data for determining the start order
based on ranking. This is done from a CSV-type file containing the ranking points
with the program function *Entries / Transfer additional data*,
using the licence number as the field identifying the competitor.

The move to the qualification classes is normally made with
*Preparation / Draw and numbering* on the *Splitting
classes* tab. The basis for the split is in most cases
*Balanced by ranking* . Once
the split has been made, the start order can be determined on the *Draw* tab and
finally the numbering on the *Assigning numbers* tab. It is often advisable to apply the split into qualification classes
only to the 1st stage, in which case the competitor's
base class remains the class in which the entries were recorded. In this case
you must, however, make sure that for the second stage the competitors are in some
class into which participants are not picked on the basis of the qualification
competition. Such a class can be either one of the qualification classes or,
in the case discussed below, the B final. Moving to one
qualification class works even if the class does not take part in the 2nd stage. In some
cases you should define an extra class for this purpose in order to
avoid problems in the online results service. More on these choices
below.

In some competitions B finals are held using
start times drawn in advance. In this case all competitors must be
marked in the relevant B finals before the draw. This is done
with *Entries / Copying club names and classes*
by selecting *Move competitors from class to class* there. After
this, select *Base class* as the data to copy and *Stage 2
class* as the target; in the class change options you then give the
base class of those competitors and as the new class the B final class (instead of the base
class it may sometimes be better to use the 1st stage class for identification).

The procedure in the previous paragraph is also suitable
for cases in which the B final is formed together with the A final, so that
the competitors are moved to some class that does not take part in the 2nd
stage. If in the second stage the B final is split into parallel
classes after the qualification, it is better to define an extra working class just
for this purpose, so that after the B finals have been formed no participants
remain in this working class for any stage.

After the qualification, the finals are formed using the
procedure described in chapter 10.4. In this way
all those final levels that are determined by the qualification placings
are formed at once. If everyone left outside the A final takes part in one B final class
without their start order being determined by the qualification results, they can
be placed there in advance. If the start order
is determined by the qualification, they are placed in advance in another class and
moved to the B final at the same time as the A final is formed. If the B final is split into parallel
classes, the competitors must be placed in some other class, preferably
an extra auxiliary class that is marked as taking part in the 2nd stage,
so that the split into B finals succeeds.

All operations related to forming the finals can
be done in the results service mode of the qualification stage. The *Form final
from qualification* form can be used only
in this mode. The steps done on the *Draw and numbering* form can
be done both in the results service mode of the 1st stage and in preparation mode. If
you work in results service mode, the data transfer of the results service network also works, and
the files of all computers on the network are updated at the same time to the extent that
connections are in use. Unless the schedule is tight, it may, however, be better to
do the operations separately on one computer and finally copy the files to all computers used in the 2nd
stage, because this gives the best certainty that
the files of all computers are really the same when the 2nd stage begins.

Because forming the finals is a relatively
complex operation, you should always make a copy of the competitor file before
starting the job. Copies can also be made at various intermediate stages using the program's menu item
