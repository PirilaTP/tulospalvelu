# Chapter 7. Viewing and editing entry data

## 7 Viewing and editing entry data

### 7.1 General

You can view and edit entry data both in the *Entries* table and on the form
showing the data of a single entry. Data are also edited on the entry
input form, which is covered in [chapter 3.1](3.1_ilmoittautumisten_syotto.md)
of the instructions. The entries table is particularly suited to
viewing several entries and making changes concerning them; viewing and editing the data of a single entry is clearest
on the *Competitor data* form, which is opened by selecting *One entry* from the menu or by clicking a row in the entries table.

The menu item *Entries / Show competitors on course* opens
a table of those competitors who do not yet have a finish time and who have not been
marked as absent, did not start, vacant, did not finish or
disqualified. The table shows the start location (the class's start number and the course's
start), the start time and all split points in use. The class can be
selected from the list at the top; the *Fetch* button updates the list.

In the *Fields* menu of the *Entries* table you choose which columns
are shown in the table. For stage-specific data (e.g. *Start time*,
*Result*, *Course*), a multi-stage competition shows a separate column for each
stage when *All stages* is selected.

The options *Record* and *Target* add to the table the competitor's record and
target time, which are otherwise shown only one competitor at a time on the
*Competitor data* form (menu items *Show record* and *Show
target result*). This way you can view and compare the records and targets of the whole
field at once, for example before a handicap or
target time competition. The columns are hidden by default.

- The time is shown in the format hh.mm.ss in whole seconds, just as
  on the *Competitor data* form. An empty cell means that no time has been
  given.
- When editing is allowed (*F9* or *Editing allowed*), the values can be
  changed directly in the table. The time is written in the same way as when
  reading from a file, for example `45.30` or `45:30` (45 min 30 s) and `1.02.30`
  or `1:02:30`. The program formats the time when you leave the cell.
  Clearing the cell removes the time.
- The changes are saved together with other changes made in the table.
  If the saved time contains fractions of a second, they are kept unless the cell
  is changed.
- You can sort the table by record or target by selecting
  *Record* or *Target* as the order or by clicking the column
  heading. The fastest time comes first and competitors without a time
  last. Equal times are ordered by name.

Record and target times can also be read from a file with the fields
`Ennätys-1`, `Tavoite-1` etc. (see [chapter 3.2](3.2_ilmoittautumisten_lukeminen_tiedostosta.md)).

Both in the table and on the form, making changes requires that they are allowed with the *Allow changes* button. Split times are not shown
on this form; they are viewed and changed on a form opened with the *Online split times* button. When data affecting the result
are changed on the *Competitor data* form, either the finish time or the
result changes according to the change made. Thus the start time, finish time and penalty
affect the result, and the result in turn affects the finish time. The placing and overall result are determined
again only when the changes are saved.

On the *Competitor data* form you can choose to show a club list. When the club list has been selected for display and the list file has been opened,
the program automatically searches the list for the club when the club name or abbreviation is edited, depending on which one has been defined as primary for the competition.
When the correct club is selected, pressing *Enter* transfers the club data (long name, abbreviation and district code)
to the competitor form. This ensures that the club name is always recorded in exactly the same way, and long club names are maintained while orienteering
handles abbreviations.

The competitor to be viewed can be selected while the
*Competitor data* form is in search and view mode by typing
the competitor's number in the relevant field, by selecting a class and
browsing, or by using the name search, which can also be complemented by browsing.

When the form is in edit mode, the search functions are not
available. The *Cancel changes* button discards the changes and returns to
search and view mode. The *Save* button saves the data and returns to
search and view mode if saving succeeds. If saving fails
(for example, a number already in use), the form stays in edit mode so that you
can correct the data and save again immediately.

If the form is closed while edit mode is on (the *Close* button or
closing the window), the program always asks *Save changes?*.
*Yes* saves the data if they have been changed or if this is a new
competitor. If saving fails (for example, a number already in use or
data changed elsewhere), the form stays open in edit mode. *No*
discards the changes; adding a new competitor is cancelled. After a successful
save or discard, edit mode ends, and the form
opens in search and view mode the next time.

To make changes easier, you can use
the *F9* key both for switching to edit mode and for saving the changes
made.

From the form's menu you can also select a mode that speeds up stage-specific
changes. When this mode is selected, pressing *F9* makes the
program move to the stage data table, to the start of the row of the
stage in progress
