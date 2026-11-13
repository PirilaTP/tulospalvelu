# Configuration files

### Configuration files

The basic results-service functions during the competition
are timing and support for the announcer. In addition it may be necessary to make
some changes to the competitor data. Three
computers are used for these tasks:

- **Timing computer**.
  A finish clock is connected to this computer, and the
  start gate and a photocell (or timing button) are in turn connected to the clock. This computer also acts
  as the central computer of the data transfer between computers, connected to both other
  computers. At least for now, the program best suited for timing is
  *HkMaali*, because entering competitors' numbers
  alongside the times taken is smoothest in this program.

  - **Announcer's
    computer.** This computer is used to display information that supports
    the announcer. It uses the program *HkKisaWin* .

    - **Office
      computer.** This computer is used to make any necessary changes
      to the competitor data and to produce outputs both for the printer and as html and
      other files. It uses the program *HkKisaWin*
      .

The tasks of each computer require a corresponding
configuration file to be prepared and placed in the competition folder.
The default name of the configuration file is Laskenta.cfg, which
can be used on all computers, but another and perhaps better option is
to name the files according to the computer's task: maali.cfg, kuul.cfg and toimisto.cfg.

The configuration file of the timing computer looks
something like this (lines starting with // are comments explaining the following parameter)

```
//Machine identifier
kone=MA
//start the program without asking to confirm the selections
boot
//Define an invalid code for the keyboard timing key.
//(The program then does not ask for a key and times cannot be taken from the keyboard)
näppäin=99,99
//Finish clock connection (by default a Timy clock connected to serial port COM5)
timy=5
//Times the clock receives from the start gate are interpreted as start times and photocell times as finish times.
//For all classes the starter is deduced from the time taken
pisteet=mmmlm/kaikki
//All times that do not come from the start gate are finish times (i.e. not split times)
pisteet=m/vain
//timing data is kept on restart
ajat=/s
//start gate times are saved to a separate file
lajat
//connection to the office computer (IP address 192.168.1.11)
yhteys1=udp:0/192.168.1.11
//optional transfer of the timing table to the office computer (not required)
lähaika1
//connection to the announcer's computer (IP address 192.168.1.12)
yhteys2=udp:0/192.168.1.12
```

The configuration file of the office computer looks something like this

```
//Machine identifier
kone=TO
//timing data is kept on restart
ajat=/s
//start gate times are saved to a separate file
lajat
//connection to the finish computer
yhteys1=udp
//optional transfer of the timing table from the finish computer (not required)
lähaika1
```

The configuration file of the announcer's computer looks something like this

```
//Machine identifier
kone=KU
//one-way connection to the finish computer (the announcer's computer cannot send data to others)
yhteys1=udpi
//The following parameter asks the program to open the saved form layout at startup
ikkunat
```

On the finish computer you must create a shortcut to the program HkMaali
so that it points to the competition data folder. When using HkKisaWin
no corresponding procedure is needed; instead, the working folder is selected
from within the program.

The startup settings of HkKisaWin are particularly
useful on the announcer's computer, where you should proceed as follows (this is
only possible when the files KilpSrj.xml and KILP.DAT are available and
the program has been started in results service mode).

- create the window layout that
  the announcer will use in the competition
- save the layout to the file oletus.ikk with the menu item *Files /
  Save windows*
- In the menu item *Files / Startup settings*,
  select opening directly in results service mode and, as the configuration file, the file
  described above.
