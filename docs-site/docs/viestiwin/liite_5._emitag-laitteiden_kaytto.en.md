# Appendix 5. Using emiTag devices

## Appendix 5. Using emiTag devices

#### A5.1 Operating models

The operating models supported by my
programs are based on the models described in Emit's instructions
"Instructions for use Emit Finish Station ECB1" and "emiTag brikkesystem –
Koblingsskema, reservedeler og ekstrautsryr", the latter of which mainly contains
various diagrams of how the devices are connected and of general
arrangements.

For timing, the operating model essentially follows the diagram
"emiTag langrenn, enkel" or "emiTag langrenn, avansert".
I am not aware of corresponding documentation for punch checking.

The following devices are typically connected to the computer

- finish station ECB1

  - split time station ETS1

    - punch check reader ECU1 or MTR5

A device is connected to the computer with a USB cable, an RS485
(current loop) connection via a MOXA USB converter, or an RS232 connection.
USB connections also appear to the program as serial ports, but the connection speed depends on the
type of connection:

- direct USB connection 115200
  b/s

  - RS485-based connection 19200
    b/s

    - RS232 connection 9600
      b/s.

The programs support both direct connection of the devices
listed above and retrieval of split time data from Emit's
database server, which collects the split time observations forwarded by the ETS1 station
over a GPRS connection.

#### A5.2 Program parameters

To use the emiTag functions,
new parameters have been added to the program MaalEmit32:

```
EMITAGy=x
EMITAGy=x/U
EMITAGy=x/C
```

define the serial port number for the connection used
to connect the device. y is the connection number and
x the serial port number.

The suffix
/U at the end of the parameter indicates that the connection is a fast USB connection and
/C an RS485 connection. Otherwise the speed is 9600 b/s.

ECAIKA

indicates that the time reported by the ECB1, ETS1,
ECU1 or MTR5 device is recorded as the time of the event, not the computer's time.

#### A5.3 Controlling emiTag devices and transferring memory data

With the punch check display selection *Functions /
emiTag reading control* you can change the device settings and request the device to send the data in its memory.

From this display the device's clock can be synchronised with the computer's
clock. Most devices must be synchronised every time they are restarted.

On the display you can set the device's operating mode,
which must always be 0, and the code, which may be needed
to identify a split point, in which case a code between 65-239 is used. The codes
240, 243, 250 and 253 request the device to work in punch check mode. The codes
240 and 243 bring the data of the last 5 days to the computer, i.e. often unnecessary data.
The codes 250 and 253 bring only the last competition and are therefore usually
better. Code 248 is used for finish timing with the ECB1 device.

The corresponding functions are largely available
in *HkMaali*: from the main menu selection
*A)setukset / Emit* (Settings / Emit) you go to the device control display,
which can
also be reached from the punch check display with the key 'M' (referring to the word MTR).

All emiTag reader devices store the data they read
in memory. This stored data can be read afterwards.
Selecting the data to read by message numbers can be almost
impossible, because the numbering does not directly correspond to the number of punches read. Therefore
it may be easiest to read the data of the most recent day or all data
into a separate file and select from there the rows to be processed
with the selection *Functions / Read data from file*
.

#### A5.4 Retrieving times from Emit's time server

The split time station ETS1 can send times
over a GPRS connection to a database server maintained by Emit.

The parameter

```
ETGPRSy=xxxxx,uuuuu,vvvvv,zzzzz
```

or

```
ETGPRSy=xxxxx/L
```

enables HTTP-based retrieval of split times from Emit's server;
xxxxx, uuuuu, vvvvv and zzzzz are the serial numbers of the at most four ETS1 devices that
produced the data. The additional marking /L means that
starting the data transfer is postponed until it is started separately from
the program. y
gives the connection number for retrieving the data.

Another possibility, though apparently both slower and more awkward, is to retrieve the data with SQL queries directly from the database server. The parameter is then

ETSQLy=xxxxx,uuuuu,vvvvv,zzzzz

When this parameter is used, an SSH tunnel must be opened
from the results service computer to the database server. The tunnel makes
the server appear to the program just as if it were a local MySQL server
in the port given in the tunnel definition (the program's default is 53306). This
function is controlled from a form opened from the HkKisaWin
timing form selection *Functions / Fetch from MySQL database*. The program
does not currently open the connection automatically; it must be opened
with the button *Open connection* when the parameters are correct and the tunnel has been
opened, for example with the program plink. Successful
opening of the connection is shown by the program
reporting the number of records on the server.

Using plink
requires a key file and information on the address and port to which the connection
is made. This information has been obtained from Emit, and using it requires
permission from Emit. The exact plink command line is available together with
the key file. plink is freely available as part of the
*PuTTY*
software.

When the connection is open, data can be retrieved with the button
*Fetch*. Normally, however,
continuous retrieval is used, which starts when it is enabled. Retrieval takes place
at the defined retrieval interval. The time limit should be set close to the
actual start of the competition, so that the program does not retrieve times produced
earlier by the same device. **Note: The time limit must be given in Norwegian time.**

As soon as the program has found even one record that is
from the requested device and dated later than the time limit, the program reports
the sequence numbers in the database of the first and last records meeting the conditions
and starts retrieving data. Data is normally retrieved in batches of at most 100
records, but this value can be changed. After the retrieval, the program
updates the information on the highest message number of each device. The program
directs later retrievals only
to data whose message number is greater than the last retrieved record shown on the
screen. If for some reason the field contains a number that is too large, no messages are retrieved.
Recent messages can be
retrieved again by decreasing this number.

Continuous retrieval must be stopped before editing the device number or
the number of the last
message, and then restarted.

Both ways of retrieving times from Emit's server can be controlled with the following parameters

ETHAKUVÄLI= xx

Selects the interval between successive split time retrievals in tenths of a second

ETDATE= vvvvkkpp

The first day from which data is retrieved, in the form
vvvvkkpp (yyyymmdd, e.g. 20131124
). If not given, only the data of the retrieval day is retrieved.

ETTIME= tt:mm:ss

Time of day older than which data is not retrieved, in the form
tt:mm:ss (hh:mm:ss, e.g. 13:10:00
). If not given, all times of day are retrieved.

```
ETHOST=ip:portti
```

gives the IP address and port of the HTML connection. If not given,
the numbers 195.159.103.189:1379 are used

Of the program's other parameters, the parameters
AIKALUKIJAy=VAINx, where y and x may also be missing, and
LÄHDEPISTEET (see [chapter 5.2](5.2_tunnisteisiin_perustuva_ajanotto.md)) are often used.
With the parameter LÄHDEPISTEET it is
usually advisable to give the parameter AIKALUKIJA=VAIN.

So either the parameter EMITAG,   ETGPRS or ETSQL is always needed
. Different parameters and/or several EMITAG
parameters can be in use at the same time when the connections have been given different
numbers.

#### A5.5 Additional program functions

Data coming from emiTag devices is handled in the same
way as data coming from other Emit punch units. Unless the parameter AIKALUKIJAy=VAINx is given, the program assumes that the device produces
punch data. Timing data is not handled without the parameter AIKALUKIJA or ETGPRS
.

From the timing screen you can use the keys *Alt-W* to go to
a selection in which retrieval of split times from Emit's
server is started or stopped, when the parameter ETGPRS
has been given. In this selection you can also request resending of all
data. Otherwise the program requests repetition of the last 10 items
to make sure that all data is received without long delays even if
there has been a disturbance in the data transfer.

#### A5.6 Use in a skiing competition

Of the parameters, you should usually give EMITAG, ECAIKA and ETGPRS if a GPRS connection is used for transferring
split times. Otherwise the same practices are followed as in other chip-based
timing methods, such as punching with an Emit card at the finish line.
(see [chapter 5.2](5.2_tunnisteisiin_perustuva_ajanotto.md)).

If the start gate is connected via the ECB1 device,
you should give the parameter

PISTEET= MMMLM

or, if you want automatic inference of the starter based on the start
time within 3 s,

PISTEET=MMMLM/sarjannimi

If inference is wanted for several classes, write
KAIKKI in place of sarjannimi (class name).
