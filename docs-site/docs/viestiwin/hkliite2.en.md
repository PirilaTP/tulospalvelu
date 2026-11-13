# Appendix 2. Setting up data transfer

## Appendix 2. Setting up data transfer

### A2.1 Data transfer within the results service network using a TCP/IP network

More information on the possibilities of data transfer is in the document HkMaali.pdf.

Connections between computers running the results software
usually use the UDP protocol. The TCP protocol is available
when UDP does not work, for example over a mobile phone connection. Data transfer via serial ports and
null modem cables also still works in the programs.

A UDP connection requires that
each of the two computers using the connection reserves one UDP port for receiving
messages from the other side. The default for this port is 15900 + n,
where n is the connection number, so the port number is known when the connection number
is known. The value used in calculating the port number can be changed with the parameter

PORTBASE=pohja-arvo

whose default is therefore PORTBASE=15900
The port number can also be changed per connection, as described below. If
not even the connection number has been given, the program assumes that it is
connection 1 and the port is therefore 15901. The connection number is computer-specific. When UDP connections are used,
it is common that only one computer, the "server", uses several connections; all
other computers can then use only one connection. If a backup server is also in use,
the other computers must also use two connections, for example so that connection 1 is used
for the server and connection 2 for the backup server.

It is usually most practical to define a connection so
that the IP address and the remote port are defined only at one end of the connection.
This party of the connection then sends its own IP address and port to
the other side, after which the connection works in both directions. Not even the remote
port needs to be defined separately if it is the remote side's connection
1, because the port 15901 of this connection is the default. (The parameter PORTBASE
also affects this number.)

UDP data transfer between two computers is started
by giving on both computers the parameter YHTEYS
in its full form:

`YHTEYSn=UDP:omaportti/vastaip:vastaportti`

or in simpler forms in which at least some of the address parameters are omitted,
for example

`YHTEYSn=UDP
YHTEYSn=UDP:omaportti
YHTEYSn=UDP:0/vastaip`

If omaportti (own port) is missing entirely, is 0 or consists of letters, the program uses
the value 15900 + n for its own port, where n is the connection number. If you want to give vastaip (remote IP) or vastaportti (remote port),
the field omaportti must contain 0 or some letter, i.e.
the characters : and / cannot directly follow each other.

If the parameter vastaip is missing or its value is AUTO, the program waits in its own port
for a message from the other side, from which it finds out both the remote IP address and the remote port.
There is a little more information on this at the end of this chapter. So when the remote IP address is not given,
the remote port does not need to be given either, since the program also gets this information from the message it receives.

The default value of vastaportti is 15901, i.e. the default is that it is the remote side's connection 1,
for which no different port has been defined. If the word
yhteys and a number n are written in the field vastaportti, the program selects 15900 + n as the remote port, i.e. assumes that
it is the default port of the remote side's connection n. (Instead of the word yhteys, the
letter Y alone can be used here.)

Immediately after the word UDP there can be the letter i or o indicating that data
is only received (incoming) or only sent (outgoing).

Examples to clarify:

Example 1: Connections using standard ports

In this example one "server" is connected to three other computers. For the server
the data transfers are defined as follows

`yhteys1=udp:0/192.168.1.12
yhteys2=udp:0/192.168.1.13
yhteys3=udpo:0/192.168.1.14:yhteys2`

At the other end of the first connection the parameter is

`yhteys1=udp`

The parameter at the other end of the second connection is

`yhteys1=udp`

The parameter at the other end of the third connection says that it is one-way
incoming data and that it is connection 2 of this computer:

`yhteys2=udpi`

Each connection requires that one of its ends has been given
the remote IP address and that this end also knows the correct value of the remote side's omaportti.
This port does not, however, need to be written as a parameter if it is the remote side's connection 1.

Example 2: A connection in which all values are defined in the parameters:

The UDP protocol is used for data transfer by giving the connection parameter on the computer
whose IP address is, for example, 192.168.1.11, in the form

`YHTEYS1=UDP:1250/192.168.1.12:1350`

when the IP address of the computer at the other end is 192.168.1.12. There, at the other end, you must in turn give the parameter

`YHTEYS1=UDP:1350/192.168.1.11:1250`

The first number is the port in which UDP message reception is opened on that computer,
and after the slash come the other computer's IP address and UDP port.
Each connection therefore uses UDP reception at both ends. Messages are always sent
to the receiving port, which listens continuously and replies to the sending port,
which the system always assigns automatically for each message separately. Each
computer can be connected to a great many other computers, and for each connection
a separate receiving port must therefore be reserved.

The ports reserved on a computer can be listed with the command

`NETSTAT /A`

and the UDP ports it reports should not be used. The port numbers used above,
1250 and 1350, were just two numbers I chose at random. Any
free ports will do just as well. As stated above, the default is
ports starting from number 15901.

Instead of a numeric address, a text-form address of the target computer can also be used as the parameter vastaip.
To avoid depending on a name server during the competition,
I recommend that all text-form names used are defined
in the file hosts, which is typically located in the directory \WINNT\system32\drivers\etc.

When you want to use several copies of the results software on the same computer,
the address localhost can be used as the parameter vastaip; it always refers to
the IP address 127.0.0.1. Using the address localhost works even if the computer has
no network card at all, provided that the TCP/IP functions are installed.
Default port values cannot be used for connections within one computer unless different
connection numbers are defined for the different programs, because the same port cannot be reserved
for more than one program.

At one end of the connection the word AUTO can also be given
as the parameter vastaip. When this is done, that computer waits for a UDP message
in the port omaportti and from then on uses the IP address of the sender of the received
message. This IP address can also change while the program is running,
since the program always uses the IP address of the sender of the last UDP message
received in the port omaportti. This procedure is useful both for simplifying
the parameters and in a situation where the IP address of the other party of the connection is not
known in advance or may even vary. This kind of
situation arises mainly when the connection is made over the internet using
a public service provider. At the other end, however, the remote IP address must always be given
so that the connection can be established.

The program can act in a TCP connection both as
a server and as a client. The parameter is of the type

`YHTEYS5=TCP:192.168.1.12:1350`

where the IP address and port refer to the server
used, when the program is a client, and

`YHTEYS5=TCPS:1355`

where the number is the connection port, when the program acts as a server.

When you want to send data in text form
over a TCP connection, add the letter X (TCPX or TCPSX). At present the format is simple, i.e. not XML as in the individual competition programs.

### A2.2 Configuring a local network for UDP data transfer

The program only requires that a connection between the computers can be established, so
nothing different from the usual configuration of a TCP/IP-based local network
is needed. If the computers used are not otherwise in the same local network,
the following steps typically have to be taken:

- Build a physical network consisting of one or
  more switches or hubs and cables
  connecting the computers to the switches or hubs. A hub-based 10
  Mb/s Ethernet network is probably sufficient even for the largest results service systems.- All configuration changes are made in the TCP/IP settings
    of the network card (except in cases where the connection
    is made over a telephone line, but in those cases nothing usually
    needs to be changed). Write down the earlier TCP/IP network configuration
    before you change it- Disable automatic retrieval of the IP address (if it is in use) and define
      the computers' IP addresses and subnet mask so that the computers can connect
      to each other. In most cases it is best to set up the network so that all computers in the local
      network are in the same subnet. This is done, for example, so that
      the IP addresses of all computers are of the type 192.168.1.xxx and the subnet mask is
      255.255.255.0. When working in the same local network, the IP addresses of the router and the name server
      usually do not matter.

Nothing prevents using separate local networks connected to each other by a router,
if there is some reason for it. The connection can also be made over an external internet connection.
This makes it possible to connect to the network, for example, a competition office located away from the
competition centre, or a computer providing internet services. The external network can also be used
if you want to connect computers to the results service network, for example over a GPRS phone connection
or a modem connection. Using a GPRS network has limitations, however, which depend
on the service provider and the service chosen. Usually a UDP connection works only
for transfer from a GPRS connection to the fixed network, if even then.

- It is usually simplest to use IP addresses to
  identify the computers, but names recognised by the network can also be used
  if desired. To avoid depending on a name server, the
  names can be added to the file hosts (if you do not know how to do this, forget it
  and use IP addresses).- Any firewall programs in use are disabled (or
    the network address range used is defined in them as a local network in which all
    traffic is allowed).

### A2.3 Special data transfer operations

From the main menu selection *Files /
Data transfer settings* you can perform the following special operations

- suspend and restart
  data transfer either entirely or only for outgoing or incoming messages,
  and either for all connections or for an individually selected
  connection.

  - request retransmission of previously sent messages,
    choosing in minutes the period the request covers

    - view the content of the data transfer backup file
      COMFILE.DAT, which shows what kind of messages that
      computer has sent to the different connections.
