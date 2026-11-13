# Chapter 14. Biathlon

## 14 Biathlon

### 14.1  Defining shooting stages

The results service for biathlon differs from cross-country skiing in the
handling of penalties. There are also small differences in the handling of the
pursuit start.

In the competition's [basic data](2.1_uuden_kilpailun_luominen.md), select biathlon as the sport,
and it is a good idea to also define the default penalty given for a missed shot,
even though this value does not determine the penalty definitively.

The number of shooting stages and the penalty given for a missed
shot are defined in the [class data](2.2.1_sarjatietojen_muuttaminen.md)
separately for each class. This is
best done on the editing form for a single class's data, which has fields for entering these
in the data of each stage.

The [pursuit start](10.3_takaa-ajolahto.md)
 requires its own definitions in the class data
and in the choices of time handling precision.

When split times are taken in biathlon, the program's
default is that the 1st split point includes the 1st shooting stage, the 2nd
split point two shooting stages, and so on. This default behaviour can be changed
by giving, in the split point definitions, the number of shooting stages included in each
split time

### 14.2  Entering penalties

Penalties can be entered both in the correction window of HkMaali
and in HkKisaWin, where it works best on the form designed for this
purpose. This form can be opened
via the *Results service* menu of the main form.

On the penalty entry form you fetch a competitor by entering
the number and pressing *Enter*, whereupon the program moves to the first
unprocessed shooting stage. The program uses a hyphen as
a sign that the number of penalties has not been entered, and also faultless performances
must be saved by entering the value 0. The *Enter* key takes you from the penalty field
to accepting the entry, and accepting takes you to entering a new bib number. If
you want to enter several penalties at once, you must use the Tab key to move
from one field to another (and the combination *Shift-Tab* to move backwards).
When the data are saved, they are also recorded in the list of entered data, which
is shown at the top of the form.

The program can also receive penalties from the Kurvinen
equipment. This mode of operation is started by giving the program
the startup parameter

```
SAKKO_YHT=n
```

and

```
SAKKO_LAJI=x
```

if the Kurvinen equipment in use is
older than version 2. For the oldest version x=0 and for the next
x=1. The default is x=2 for newer equipment, which need not be given
as a parameter. Kurvinen equipment versions 1 and 2 differ from each other only in
data transfer speed. It is 9600 baud in version 1 and 19200 baud in version
2. The program selects the speed on the basis of the parameter
mentioned above.

When penalty data arrive from the Kurvinen equipment and
the penalty entry window is open, the program records the received data
in the list of entered data. The data are recorded in the competitor data even if
this window is not open, and the operation is then the same in
HkKisaWin and HkMaali. The program gives an error message when an error
is detected in an incoming message.

The programs transfer the Kurvinen equipment data on shooting results
over the network, so both this function and the function in section 14.4 are
available also on computers other than the one to which the Kurvinen equipment
is connected.

### 14.3  Penalty loop monitoring

In the program's menu item *Penalty loop monitoring*
you can enter the competitor's number each time a penalty loop is completed. The
program then shows in another window the penalty loops of those that have not been recorded
as completed.

The program
saves the entered penalty loops in the file SakkoKierr.txt and reads them
when the penalty loop monitoring form is opened. Before a new competition you must
make sure that this file does not exist or is empty. Incorrectly
entered data can be removed by moving to the row concerned and
clicking *Undo selected entry*
.

The file SakkoKierr.txt is
available only on the computer on which the entries were made. Information on the number of recorded loops
is transferred to other computers as well, so the numbers of unrecorded loops
can also be viewed
from other
computers.

### 14.4 Shot-level shooting monitoring

The program's menu item *Monitoring / Shooting ranges*
opens a form on which the program shows the result of every shot if
Kurvinen equipment that supports shot monitoring is in use (version 0 does not
support it).
