# 11.1 Calculating competitor points

### 11.1 Calculating competitor-specific points

#### 11.1.1 Points for one stage

Points are first calculated one stage at a time and can then be combined into total points as a separate operation.

Points can be determined by placing, by result, or by both.

Placing-based points are defined as follows. The calculation is based on using 1–5 placing ranges. Within each range
the points are assumed to change by a constant amount when the placing changes by one. Five ranges allow a faster change
at the top places and a slower change at other places.

When the points for each placing are the same regardless
of the number of participants, enter

- in *From place* the best placing of the range,

  - in *To place* the
    worst placing of the range,

    - in *First points* the number of points
      corresponding to the first-mentioned placing, and

      - in *Step* the amount
        by which the points change as the placing gets worse. Note that this
        number is negative when the points decrease as the place number increases.

If you want points that decrease with the placing
for everyone who has a result, giving the last one, for example, 1 point, enter
9999 in From place and in To place the best placing to which this rule
applies. First points then applies to the competitor placed last, and the better
places receive their points according to the step. If the points rise faster
at the top, enter on the second row the placing from which the rise accelerates. In this case
the same placing must be entered in To place on the upper row and in From place on the next
row. The program continues assigning points from the number of points
that the rule on the upper row leads to.

If you use place-number points that increase as the placing gets worse, enter a positive number in *Step*. In this case
including all placings can be expressed with the number 9999 in *To place*.

Result-based points can be calculated either so that the points decrease
in proportion to the time difference to the winner, or so that the points are proportional
to the ratio of the winner's time to the participant's time or to the logarithm of that ratio.
In the former case a very large time loss would lead to negative points,
but the points can be given a lower limit. In the latter case even a large
time loss gives positive points, so no lower limit is needed. The time difference can be
handled either as such, as a number giving the difference in hundredths of a second, or
relative to the winning time.

Points can be determined as the sum of place-number points and result-based
points. The weights of the different factors can be changed with coefficients. If,
for example, you want placings to have primary weight, you can use a very large step,
e.g. one million.

The program handles points as integers, so
using decimals requires calculating the points, for example, multiplied by 100,
in which case they include two decimals. At the output stage it is possible
to insert the decimal comma in the right place using the menu item
*Formatting / Point decimals* on the output form.

#### 11.1.2 Calculating total points

Total points are calculated as the sum of the daily points from all stages up to the active one. The number of points taken into account
can be limited to fewer than the number of stages, in which case the best points up to that limit are taken into account.

In some competition series a competitor may take part in different classes on different occasions. For this there is an option
to calculate points separately for each class, but for the time being this option has been implemented only according to the Aktia Cup rules.

Points can be calculated both for ordinary classes
and for class combinations, but a competitor can be given
only one set of points at a time, so each class combination must be handled in one go,
from the stage calculation to producing the outputs, before calculating other points concerning the same
competitors.

#### 11.1.3 Calculating points directly from the result

On request, the program calculates the points at the same time as it
saves the result. To get correct points this way, the number of points must not
depend on anything other than the competitor's own result. The reference
time used in the calculation must be fixed before the competition.
If the final points depend on the winning result of the class, the points calculated during the competition
are approximately correct if the winning time can be estimated
reasonably accurately in advance.

In some cases the formula
used for calculating points may be more complex. Such a formula can
possibly be defined from the menu of the *Class data* form.
The points to be calculated are then determined by giving the result that gives zero
points and the coefficients that tell how fast the points increase
when the result is better than that limit. In most cases, however, the menu item *Results service
/ Calculate competitor points* is more appropriate. Continuous
calculation of points is enabled by checking *Always calculate stage points when saving result*
and *possibly Always calculate total points when saving
result.* In addition, you can ask the program to calculate
total points also for those who do not get a result in the stage in progress, either
because of disqualification or not starting. This option is
offered because in some cases the points from earlier stages may
entitle them to even a good placing.

Checking *Use total points as total result*
causes the total points and the points difference to the leader in total points
to be shown on the announcer's monitoring screens.

#### 11.1.4 Saving point calculation formulas

The formulas defined on the *Points calculation* form can
be saved in the class data using a menu item of the form. Saving
works as follows:

- Saving is done for all selected classes.

  - By changing the selected classes you can
    save different formulas for different classes

    - The formulas are saved per stage so that they contain
      both the stage-specific points of that stage and the calculation of total points
      up to that stage. For different stages the formulas must be saved in that stage's
      results service mode.

      - The formulas of the selected classes become
        as shown defined on the
        form. The formulas of other classes are saved in their earlier form. Writing to
        the file thus affects all classes, because all are saved in the same
        class file.

When the program is started after saving,
it reads the formulas automatically, and they can be used via the button *Calculate
all points by each class's saved rule*. The formulas
can be brought to the screen by selecting a class for processing and selecting
*Read classes from file* from the menu. The program then shows that class's
formulas on the screen and adds to the list of classes being processed all classes for which
these same formulas have been defined. After this you can change the formulas and choose
which classes the changes will apply to.

The formula definitions of the previous stage can also be fetched
from the class data. The fetch applies to all classes, but
the formulas of one class are shown on the screen, and the classes that
have the same formulas become selected. When the data are saved to the file, all classes are saved, including
those that have not been viewed.

#### 11.1.5 Viewing point calculation formulas

All calculation formulas defined for the different classes and different stages
can be viewed in a table that opens with a menu item
of the points calculation form. The meaning of the columns should become clear by comparing them
with the fields of the definition form. Formulas
cannot be edited in the table.
