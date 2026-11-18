# Network settings

The computers used in the results service are connected to each other through a local area
network. Each connection is between two computers, and a connection is opened
so that one of the computers waits for a connection request, which the other makes.
For this to be possible, the computer making the request must know the other party's
identifier, which in practice is usually an IP address, but can also be a name that
can be mapped to a particular IP address. The connection protocol normally used
is UDP, which requires that in addition to the IP address another
number, called the port, is known.

Before you can start defining the parameters a connection
requires, you must find out the IP addresses in use, or at least so
many of them that for each connection between two computers the IP address of one party, or a name from which the program can
determine the IP address, is known. This is simple when the TCP/IP local network has been set up using fixed
IP addresses, but more problematic when this has not been done. When using the results software
on a local network, it is best to use fixed IP addresses for all computers. In some cases
a connection is needed, for example, over a mobile network, in which case only one
computer's IP address is known, but this is enough to establish the connection.

The command

`ipconfig`

given in the Windows command window produces a response that
contains the computer's IP address on a line that may be, for example

`IPv4 Address . . . . : 192.168.1.10`

but this number may be different after the next
restart unless the addresses have been defined as fixed.

UDP port numbers can be between 1 and 65535, of which
most values above 1024 are usually free (most commonly excepting 8080 and 8008,
but some other numbers may also be in use. You can query the ports in use
with the command `netstat /a`, but other
ports may be in use at some other moment). By default the programs use
values in the range 15901-15964, which have not caused problems
in practice. The connections used by the programs are numbered, and the largest available
number is normally 64. The default port number for connection N is
15900+N.

Make sure that firewall programs do not block
data transfer. This typically requires that the programs HkMaali.exe and
HkKisaWin.exe are allowed to use all connections, also as a server. If the network
is disconnected from the internet, all firewall programs can be disabled if necessary
for the duration of the
competition.
