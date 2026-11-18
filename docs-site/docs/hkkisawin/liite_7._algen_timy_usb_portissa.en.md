# Appendix 7. Alge Timy on a USB port

## Appendix 7. Alge Timy on a USB port

The Alge Timy finish clock can be connected to a computer through either
a serial port or a USB port. Operation through a serial port is the most reliable
option, at least when the computer has a built-in serial port, and it also works well
through a USB/RS232 serial port converter. Problems have, however, been observed
with all USB solutions. They can occur both when using a serial port converter
and when connecting the Timy directly to the computer with a USB cable.
Because of the greater user experience, I recommend using a serial port converter
for the actual timing whenever possible.

#### Use through a serial port converter

Connecting a USB serial port converter to the computer normally
makes Windows install the necessary
driver software automatically. If this does not happen, the software usually has to be installed
from the disc supplied with the device. When the converter is connected to another USB port,
the installation has to be repeated, which again usually happens automatically. During the
installation the serial port number is determined, and it usually changes when switching
to another USB port. The computer's Device Manager shows
which serial ports are installed. A serial port installed through a USB port
typically gets a number that is at least 4. When the program is started, this
number must be given with the TIMY parameter, e.g.

TIMY=4

Using a serial port converter requires no other additional steps.
Because disturbances are possible, it is advisable to install the converter in advance on several
USB ports, if available, and write down the corresponding
serial port numbers. Then, after a disturbance, you can move the converter to another
port, change the number in the program's startup parameter and restart the program.

#### Use directly through a USB port

The results programs themselves do not support using the Timy
directly through a USB port, because this involves several problems. Instead,
a small utility *AlgeTimyUsbToTcp* is provided, which receives
the times coming from the USB port and immediately sends them over the TCP protocol
to the results program. The utility can also synchronise the Timy clock
to the computer time and request that the times in the Timy memory be resent.

Using *AlgeTimyUsbToTcp*
requires that the device drivers available from the Alge Timing website are first installed
on the computer. The device drivers must be sufficiently recent. Testing has been done
with version V2.70. Next, place the program *AlgeTimyUsbToTcp*
and the DLL file *AlgeTimyUsb.x86.dll*
or *AlgeTimyUsb.x64.dll* in the same directory as the program, and create
a shortcut that opens *AlgeTimyUsbToTcp.* Of the DLL files,
the first is for 32-bit Windows versions and the second for 64-bit versions. You can copy
both, because the program automatically selects the one it needs.

Also with this procedure it is advisable to connect
the Timy in advance
to all USB ports that may be used and make sure that the device drivers work
with them. This makes it easier to move the Timy to another USB port during the competition.
This should work while the programs are running without requiring additional steps from the user.

The program also requires the Microsoft .NET
framework version 4.0 or newer. This is very often already
available and can be downloaded from the Microsoft website if necessary.

When all this is ready, you can start the program. If
the Timy is connected, or is later connected, to the computer through a USB port,
the program connects to it immediately (a connection is made to all Timys connected to the computer
if there are several). The Timy sends a handshake message once a second,
and the display of these can be enabled or suppressed with a menu item.

On startup the program opens a TCP server on port
14701. The server can be closed and the port number changed while the program is
running. The program does not forward handshake messages, only
timing messages. The program allows a maximum of 20 results program
connections and sends the same messages to all programs that have connected.
The program normally shows the number of connected computers, but if
the Timy is not connected, the number must be requested with the *Test TCP.* button.

The results program (*HkKisaWin* or
*HkMaali*) is connected to the program
*AlgeTimyUsbToTcp* running on the same computer by giving it the parameter

TIMY=TCP:localhost:14701

If you want to use a port other than 14701, this
number must be changed. The Timy can also be connected to another computer, in which case
the IP address or name of that computer must be written in place of the word localhost.

If the TCP connection is broken, *AlgeTimyUsbToTcp* is immediately ready to accept a new
connection. The results program tries every few seconds to create a new
connection after detecting that the connection has been broken, but in some cases
the program does not notice the break, in which case restarting the results program
helps. If port 14701 cannot be opened because it has erroneously remained
reserved, the port must be changed or the computer restarted.

From *AlgeTimyUsbToTcp* you can synchronise the Timy to the computer
time by first enabling this function from the menu
and then clicking the button. I recommend keeping the function disabled during timing,
because the Timy clock is accurate and should not be
resynchronised in the middle of timing. Through the menu you can also request that the Timy memory be cleared.

The program reports the timing precision in use in the Timy and allows you to change it.

From the program you can also request resending of either all
times in the Timy memory or the rest starting from a selected row. Note
that the row number refers to the entire contents of the Timy memory, not to the sequence numbers in the messages, which are often
much smaller when the numbering has restarted from the beginning. It is advisable
to always clear the Timy memory before a competition so that resending is problem-free. The results program
skips times that are exactly the same as those received
earlier, so you can safely request times again when they are
all from the event in question. Times that you want included
are certainly not skipped when the Timy is set to report times with 1/1000
s or 1/10000 s precision, because times this precise and identical
cannot be taken from the same channel. The results program rounds the times as needed, so
it is good for the Timy to use one of the highest precisions (from the Timy's own keypad
you get times rounded to hundredths, but when working through the interfaces, the full selected precision.)
