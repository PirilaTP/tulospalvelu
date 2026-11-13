# Appendix 9. Using emiTag devices

## Appendix 9. Using emiTag devices

#### A9.1 Operating models

The operating models supported by my
programs are based on the models described in the Emit instructions
"Instructions for use Emit Finish Station ECB1" and "emiTag brikkesystem –
Koblingsskema, reservedeler og ekstrautsryr", of which the latter mainly contains
various diagrams of device connections and general
arrangements.

For timing, the operating model essentially corresponds to the diagram
"emiTag langrenn, enkel" or "emiTag langrenn, avansert".
I am not aware of corresponding documentation for punch checking.

The following devices are typically connected to the computer

- finish station ECB1

  - split time station ETS1

    - punch check reader ECU1 or MTR5

A device is connected to the computer with a USB cable, an RS485
(current loop) connection using a MOXA USB converter, or an RS232 connection.
USB connections are also seen by the program as serial ports, but the connection speed depends on the
type of connection:

- direct USB connection 115200
  b/s

  - RS485-based connection 19200
    b/s

    - RS232 connection 9600
      b/s.

The programs support both direct connection of the devices listed above
and fetching split time data from Emit's
database server, which collects split time observations sent by the ETS1 station
over a GPRS connection.

#### A9.2 Program parameters

The following startup parameters are provided for using the emiTag functions:

```
EMITAGy=x
EMITAGy=x/U
EMITAGy=x/C
```

which define the serial port number of the connection used
for the device. y is the connection number and
x the serial port number.

The suffix
/U at the end of the parameter indicates that the connection is a fast USB connection and
/C an RS485 connection. Otherwise the speed is 9600 b/s.

ECAIKA

indicates that the time reported by the ECB1, ETS1,
ECU1 or MTR5 device, and not the computer time, is recorded as the time of the event.

#### A9.3 Controlling emiTag devices and transferring memory data

In the punch check screen's menu item *Functions /
emiTag readout control* you can change the device settings and request the device to send data stored in its memory.

From this screen you can synchronise the device clock to the computer
clock. Most devices must be synchronised after every restart.

On the screen you can set the device's operating mode,
which must always be 0, and the code, which may be needed
to identify a split point, in which case a code in the range 65–239 is used. The codes
240, 243, 250 and 253 ask the device to operate in punch check mode. The codes
240 and 243 bring the data of the last 5 days to the computer, i.e. often unnecessary data.
The codes 250 and 253 bring only the last competition and are thus usually
better. The code 248 is used for finish timing with the ECB1 device.

The corresponding functions are largely available
in *HkMaali*: from the main menu item
*A)setukset / Emit* (Settings / Emit) you go to the device control screen,
which can
also be reached from the punch check screen with the key 'M' (referring to the word MTR).

All emiTag reader devices store the data they read
in memory. This stored data can be read afterwards.
Selecting the data to be read on the basis of message numbers can be almost
impossible, because the numbering does not directly correspond to the number of punches read. Thus
it may be easiest to read the data of the last day or all data
into a separate file and select from there the rows to be processed
with *Functions / Read data from file*
.

#### A9.4 Fetching times from Emit's time server

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

enables fetching split times from Emit's server over the HTTP protocol;
xxxxx, uuuuu, vvvvv and zzzzz are the serial numbers of the at most four ETS1 devices that have
produced the data. The additional suffix /L means that
starting the data transfer is postponed until it is started separately from the
program. y
gives the connection number for fetching the data.

Another, but apparently both more cumbersome and slower,
option is to fetch the data with SQL queries directly from the database server. In this case the parameter is

ETSQLy=xxxxx,uuuuu,vvvvv,zzzzz

When using this parameter you must open an SSH tunnel
from the results service computer to the database server. The tunnel makes
the server appear to the program just as if it were a local MySQL server
on the port given in the tunnel definition (the program default is 53306). This
function is controlled from a form opened from the HkKisaWin
timing form's menu item *Functions / Fetch from MySQL database*. The program
does not currently open the connection automatically; it must be opened
with the *Open connection* button when the parameters are correct and the tunnel has been
opened, for example with the program plink. A successful
connection is indicated by the program
reporting the number of records on the server.

Using plink
requires a key file and information on the address and port to which the connection
must be made. This information has been obtained from Emit, and getting it requires
permission from Emit for its use. The exact command line for plink is available together with
the key file. plink is freely available as part of the
*PuTTY*
software.

Once the connection is open, data can be fetched with the
*Fetch* button. Normally, however,
continuous fetching is used, which starts when it is enabled. Fetching takes place
at the defined fetch interval. The time limit should be set close to the
actual start of the competition so that the program does not fetch times produced by the same device
at some earlier time. **Note: the time limit must be given in Norwegian time.**

When the program has found even one record that is
from the requested device and dated later than the time limit, the program reports
the sequence numbers in the database of the first and last records meeting the conditions
and starts fetching data. Data are normally fetched in batches of at most 100
records, but this value can be changed. After the fetch, the program
updates the information on the highest message number of each device. The program
directs later fetches only
to data whose message number is greater than the last fetched record shown on the screen.
If the field for some reason contains too large a number, no messages are fetched.
Recent messages can be
fetched again by decreasing this number.

Continuous fetching must be stopped before editing the device number or
the number of the last message
and then restarted.

Both ways of fetching times from Emit's server can be controlled with the following parameters

ETHAKUVÄLI= xx

Selects the interval between successive split time fetches in tenths of a second

ETDATE= vvvvkkpp

The first day from which data are fetched, in the format
vvvvkkpp (yyyymmdd, e.g. 20131124
). If not given, only the data of the fetch day are fetched.

ETTIME= tt:mm:ss

The time of day before which data are not fetched, in the format
tt:mm:ss (hh:mm:ss, e.g. 13:10:00
). If not given, all times of day are fetched.

```
ETHOST=ip:portti
```

gives the IP address and port of the HTTP connection. If not given,
195.159.103.189:1379 is used

Of the program's other parameters, the parameters
AIKALUKIJAy=VAINx, where y and x may also be omitted, and
LÄHDEPISTEET (see [chapter 5.2](5.2_tunnisteisiin_perustuva_ajanotto.md)) are often used.
With the parameter LÄHDEPISTEET you should
in most cases give the parameter AIKALUKIJA=VAIN.

So you always need either the parameter EMITAG,   ETGPRS or ETSQL
. Different parameters and/or several EMITAG
parameters can be in use at the same time when the connections have been given different
numbers.

#### A9.5 Additional program functions

Data from emiTag devices are handled in the same
way as data from other Emit punches. Unless the parameter AIKALUKIJAy=VAINx is given, the program assumes that the device produces
punch data. Timing data are not processed without the parameter AIKALUKIJA or ETGPRS
.

From the timing screen, the keys *Alt-W* take you
to an option where fetching split times from Emit's
server is started or stopped when the parameter ETGPRS
has been given. In this option you can also request that all
data be sent again. Otherwise the program asks for the last 10 data items to be repeated
to make sure that all data are obtained without long delays even if
there has been a disturbance in the data transfer.

#### A9.6 Use in a skiing competition

Of the parameters you should usually give EMITAG, ECAIKA and ETGPRS if a GPRS connection is used for transferring
split times. Otherwise the same practices are followed as in other chip-based
timing methods, such as punching with an Emit card at the finish line.
(see [chapter 5.2](5.2_tunnisteisiin_perustuva_ajanotto.md)).

If the start gate is connected through an ECB1 device,
you should give the parameter

PISTEET= MMMLM

or, if you want the starter to be determined automatically from the start time
within 3 s,

PISTEET=MMMLM/sarjannimi

If you want this determination for several classes, write
KAIKKI in place of the class name.
