# Appendix 3. Alge Timy on a USB port

### Appendix 3. Alge Timy on a USB port

The Alge Timy finish clock can be connected to a computer either
via a serial port or via a USB port. Operation via a serial port is the most reliable
option, at least when the computer has a built-in serial port, and it also works well
via a USB/RS232 serial port converter. Problems have, however, been observed with all USB
solutions. They can occur both when using a serial port converter
and when connecting the Timy directly to the computer with a USB cable.
Because of the greater operating experience, I recommend using a serial port converter
for the actual timing when possible.

#### Use via a serial port converter

Connecting a USB serial port converter to the computer
normally makes Windows automatically install the required
drivers. If this does not happen, the drivers usually have to be installed
from the disc supplied with the device. When the converter is connected to another USB port,
the installation must be repeated, which again usually happens automatically. The installation
determines the serial port number, which usually changes when switching
to another USB port. The computer's Device Manager shows
which serial ports are installed. A serial port installed via a USB port
typically gets a number of at least 4. When the program is started, this
number must be given with the parameter TIMY, e.g.

TIMY=4

Using a serial port converter requires no other additional steps.
Because disturbances are possible, it is advisable to install the converter in advance on several
USB ports, if available, and to write down the corresponding
serial port numbers. That way, after a disturbance you can move the converter to another
port, change the number in the program's startup parameter and restart the program.

#### Use directly via a USB port

The results programs themselves do not support using the Timy
directly via a USB port, because this involves several problems. Instead,
a small utility *AlgeTimyUsbToTcp* is provided, which receives
the times coming from the USB port and immediately sends them over the TCP protocol
to the results program. The utility can also synchronise the Timy's clock
with the computer time and request retransmission of the times in the Timy's memory.

Using *AlgeTimyUsbToTcp*
requires first installing on the computer the device drivers available from the Alge Timing
website. The drivers must be recent enough. Testing has been done
with version V2.70. Next, place the program *AlgeTimyUsbToTcp*
and the DLL file *AlgeTimyUsb.x86.dll*
or *AlgeTimyUsb.x64.dll* in the same directory as the program and create
a shortcut to open *AlgeTimyUsbToTcp.* Of the DLL files,
the former is for 32-bit Windows versions and the latter for 64-bit ones. Both
can be copied, because the program automatically selects the one it needs.

With this method, too, it is advisable to connect the
Timy in advance to all the USB ports that may be used and make sure the drivers work
with them. This makes it easier to move the Timy to another USB port during the event.
This should work while the programs are running without requiring additional steps from the user.

The program also requires the Microsoft .NET
framework version 4.0 or newer. It is very often already
available and can be downloaded from Microsoft's website if needed.

When all this is ready, the program can be started. If
the Timy is connected, or is connected later, to the computer via a USB port,
the program connects to it immediately (a connection is made to all Timys connected to the computer,
if there are several). The Timy sends a handshake message once a second;
displaying these can be enabled or suppressed with a menu selection.

When it starts, the program opens a TCP server on port
14701. The server can be closed and the port number changed while the program is
running. The program does not forward handshake messages, only
timing messages. The program allows at most 20 results programs
to connect and sends the same messages to all connected programs.
The program normally shows the number of connected computers, but if
the Timy is not connected, the number must be requested with the button *Test TCP*.

The results program (*HkKisaWin* or
*HkMaali*) is connected to *AlgeTimyUsbToTcp* running on the same computer
by giving it the parameter

TIMY=TCP:localhost:14701

If you want to use a port other than 14701, this
number must be changed. The Timy can also be connected to another computer, in which case
the IP address or name of that computer must be written in place of the word localhost.

If the TCP connection breaks, *AlgeTimyUsbToTcp* is immediately ready to accept a new
connection. The results program tries to create a new connection every few seconds
after detecting that the connection has broken, but in some cases
the program does not notice the break, and then restarting the results program
helps. If port 14701 cannot be opened because it has incorrectly remained
reserved, the port must be changed or the computer restarted.

From *AlgeTimyUsbToTcp* the Timy can be synchronised with the computer
time by first enabling this function from the menu
and then clicking the button. I recommend keeping the function disabled during timing,
because the Timy's clock is accurate and should not be
resynchronised in the middle of timing. The menu can also be used to request clearing the Timy's memory.

The program shows the timing precision in use on the Timy and allows it to be changed.

From the program you can also request retransmission of either all times in the Timy's memory
or the remaining part starting from a selected row. Note
that the row number refers to the entire contents of the Timy's memory, not to the sequence numbers included in the messages, which are often
much smaller when numbering has restarted from the beginning. It is advisable to
clear the Timy's memory before every event so that retransmission is trouble-free. The results program
skips times that are exactly the same as previously
received ones, so times can be requested again without harm when they are
all from the event in question. Times that you want included
will certainly not be skipped when the Timy is set to report times with 1/1000
s or 1/10000 s precision, because identical times this precise
cannot be taken from the same channel. The results program rounds the times as needed, so
it is good for the Timy to use one of the highest precisions (times from the Timy's own keypad
are rounded to hundredths, but when working through the interfaces the full selected precision is used.)
