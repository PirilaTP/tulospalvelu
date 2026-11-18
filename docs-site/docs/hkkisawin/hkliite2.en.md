# Appendix 2. Setting up data transfer

## Appendix 2. Setting up data transfer

### A2.1 Data transfer within the results service network using a TCP/IP network

Connections between computers running the results programs
normally use the UDP protocol. The TCP protocol
is available when UDP
does not work, for example over a mobile phone connection. Data transfer through serial ports
and null modem cables also still works in the programs. The TCP protocol can
also be used for sending data as XML records to an external server.

A UDP connection requires that
each of the computers using the connection reserves one UDP port for receiving
messages from the other party. The default for this port is 15900 + n,
where n is the connection number, so the port number is known when the connection number
is known. The value used in calculating the port number can be changed with the parameter

PORTBASE=pohja-arvo

for which the corresponding default is thus PORTBASE=15900
The port number can also be changed per connection, as described below. If
not even the connection number has been given, the program assumes that this is
connection 1 and the port therefore 15901. The connection number is specific to each computer. When using UDP connections
it is common that only one computer, the "server", uses several connections; all
other computers can then use only one connection. If a backup server is also used,
the other computers must also use two connections, for example so that connection 1 is used for the server
and connection 2 for the backup server.

It is usually most practical to define a connection so
that the IP address and the other party's port are defined only at one end of the connection.
This party of the connection then sends its own IP address and port
to the other party, after which the connection works in both directions. Not even the other party's
port needs to be defined separately if it is the other party's connection
1, because the port 15901 of this connection is the default. (The PORTBASE parameter
also affects this number.)

UDP data transfer between two computers is started
by giving on both computers the parameter YHTEYS
in its full form:

`YHTEYSn=UDP:omaportti/vastaip:vastaportti`

or in simpler forms in which at least some of the address parameters are left out,
for example

`YHTEYSn=UDP
YHTEYSn=UDP:omaportti
YHTEYSn=UDP:0/vastaip`

If omaportti (own port) is missing entirely, is 0 or consists of letters, the program uses
the value 15900 + n for its own port, where n is the connection number. If vastaip (remote IP) or vastaportti (remote port)
is to be given, the field omaportti must contain 0 or some letter, i.e.
the characters : and / cannot directly follow each other.

If the parameter vastaip is missing or its value is AUTO, the program waits on its own port
for a message from the other party, from which it finds out both the other party's IP address and the remote port.
There is a little more information on this at the end of this chapter. So when the other party's IP address is not given,
you do not need to give the remote port either, because the program also gets this information from the message it receives.

The default for vastaportti is 15901, i.e. the default assumption is that it is the other party's connection 1,
for which no different port has been defined. If the word
yhteys and a number n are written in the field vastaportti, the program selects 15900 + n as the other party's port, i.e. it assumes that
it is the default port of the other party's connection n. (Instead of the word yhteys you can use
just the letter Y here.)

Immediately after the word UDP there can be the letter i or o to indicate that data
are only received (incoming) or only sent (outgoing).

Examples to clarify this:

Example 1: Connections using standard ports

In this example one "server" is connected to three other computers. On the server
the data transfers are defined as follows

`yhteys1=udp:0/192.168.1.12
yhteys2=udp:0/192.168.1.13
yhteys3=udpo:0/192.168.1.14:yhteys2`

At the other end of the first connection the parameter is

`yhteys1=udp`

The parameter of the other party of the second connection is

`yhteys1=udp`

The parameter of the other party of the third connection indicates that this is one-way
incoming data and that this is connection 2 of this computer:

`yhteys2=udpi`

Every connection requires that one of its ends has been told
the other party's IP address and that this end also correctly knows the other party's omaportti value.
This port does not, however, need to be written as a parameter if it is the other party's connection 1.

Example 2: A connection where all values are defined in the parameters:

UDP can be used for data transfer by giving the yhteys parameter on a computer
whose IP address is, for example, 192.168.1.11, in the form

`YHTEYS1=UDP:1250/192.168.1.12:1350`

when the IP address of the computer at the other end is 192.168.1.12. There, at the other end, the parameter must be

`YHTEYS1=UDP:1350/192.168.1.11:1250`

The first number is the port on which reception of UDP messages is opened on that computer,
and after the slash come the IP address and UDP port of the other computer.
Each connection thus uses UDP reception at both ends. Messages are always sent
to the receiving port, which listens continuously and replies to the sending port,
which the system assigns automatically for each message separately. Each
computer can be connected to a great many other computers, and a separate receiving port
must be reserved for each connection.

The ports reserved on a computer can be queried with the command

`NETSTAT /A`

and the UDP ports it reports must not be used. The port numbers used above,
1250 and 1350, were just two numbers I chose at random. Any
free ports will do just as well. As stated above, the defaults are
the ports starting from 15901.

Instead of a numeric address, the parameter vastaip can also be a text-form
address of the target computer. In order not to depend on the name server
in a competition situation, I recommend defining all text-form names used
in the hosts file, which is typically located in the directory \WINNT\system32\drivers\etc.

When you want to use several copies of the results programs on the same computer,
the address localhost, which always refers to
the IP address 127.0.0.1, can be used as the vastaip parameter. Using localhost works even if the computer has
no network card at all, provided that the TCP/IP functions are installed.
For connections within one computer the default port values cannot be used unless different
connection numbers are defined for different programs, because the same port cannot be reserved
for more than one program.

The word AUTO can also be given as the vastaip parameter at one end of the connection.
When this is done, that computer waits for a UDP message on the port omaportti and
from then on uses the IP address of the sender of the received message. This IP address
can also change while the program is running, because the program always uses the IP address of the sender of the last
UDP message received on omaportti. This
procedure is useful both for simplifying the parameters and in situations
where the IP address of the other party of the connection is not known in advance or may even vary.
This situation arises mainly when the connection is made over the internet
using a public service provider. At the other end, however, the other party's
IP address must always be given so that the connection can be established.

### A2.2 Configuring the local network for UDP data transfer

The program only requires that a connection between the computers is established, so
nothing different from ordinary configuration of a TCP/IP-based local network
is needed. If the computers used are not otherwise in the same local network, you
typically have to carry out the following steps:

- Build a physical network consisting of one or
  more switches or hubs and cables that
  connect the computers to the switches or hubs. A hub-based 10
  Mb/s Ethernet network is probably sufficient even for the largest results service systems.- All configuration changes are made in the TCP/IP settings
    of the network card (except in cases where the connection
    is made over a telephone line, but in these cases usually nothing
    needs to be changed). Write down the previous TCP/IP network configuration
    before you change it- Disable automatic retrieval of the IP address (if it is in use) and define
      the computers' IP addresses and subnet mask so that the computers can connect
      to each other. In most cases it is advisable to set up the network so that all computers in the local network
      are in the same subnet. This is done, for example, so that
      the IP addresses of all computers are of the form 192.168.1.xxx and the subnet mask is
      255.255.255.0. When working within the same local network, the IP addresses of the router and the name server
      usually do not matter.

Nothing prevents you from using separate local networks connected to each other by a router
if there are reasons for this. The connection can also be established over an external internet connection.
This makes it possible to connect to the network, for example, a competition office located away from the competition centre
or a computer providing internet services. An external network can also be used
if you want to connect computers to the results service network, for example over a GPRS phone connection
or a modem connection. The use of a GPRS network does, however, have restrictions that depend on
the service provider and the selected service. Usually a UDP connection works only
for transfer from the GPRS connection to the fixed network, if even that.

- It is usually simplest to use IP addresses to
  identify the computers, but if you wish, names recognised by the network can also be
  used. In order not to depend on a name server, the
  names can be added to the hosts file (if you do not know how to do this, forget it
  and use IP addresses).- Any firewall programs in use are disabled (or
    the network address range used is defined in them as a local network in which all
    traffic is allowed).

### A2.3 Special data transfer operations

From the main menu item *Files /
Data transfer settings* you can perform the following special operations

- **stop and restart
  data transfer**
  either completely or for outgoing or incoming messages only,
  and either for all connections or for an individually selected
  connection.

  - **request resending of previously sent
    messages**
    selecting in minutes the period the request applies to

    - **view the contents of the data transfer
      backup file COMFILE.DAT**,
      which shows what kind of messages the computer in question
      has sent to the different connections.

      - **send competitor data as XML records using the TCP protocol** to an external
        service that uses such data. This is done to a connection whose
        type is defined as TCPX. The program also sends to such a connection
        every new result as a separate message.- **send files** through the UDP connections of the results service network

File transfer is available only
when the program has been given the parameter

LÄHTIEDOSTOT

or

LÄHTIEDOSTOTx

where x is the number of one connection. The former enables file transfer on all
UDP connections, the latter on one connection only.

The parameter

LÄHTIEDOSTOTx=0

can be given after the parameter applying to all connections
to remove file transfer from connection x.

The suffix /K can be added to the end of the parameter
.

Unless the suffix /K
 has been given, the file is always transferred from either the program's
competition data folder or its
subfolder to the corresponding folder
of the receiving computer. The receiving subfolder must already exist;
the program does not create a new folder.

When the suffix /K
 has been given,
the program uses the full
directory path of the file as such.
In this case the transfer requires that the receiving computer has an identically
named directory under the same drive letter.

File transfer is also used when
course data are saved in results service mode on the form that handles them. The program then
asks whether the data should be sent through the results service network.
