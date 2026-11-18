# Appendix 1. Startup parameters

## Appendix 1. Startup parameters

The operation of the programs is controlled with startup parameters.
When working on a single computer, many basic functions require no
startup parameters at all, but they are essential e.g. when connecting computers
to work together over a network. Parameter names can be
written in upper or lower case, as can the values given with the parameters.
Only in a few text fields, such as when defining a heading,
does the program keep lower-case letters; in all other
cases the program converts letters to upper case before use.

### A1.1 Basic and miscellaneous parameters

|  |  |
| --- | --- |
| CFG=tied.nimi | Defines the file from which the other parameters are read. The default is LASKENTA.CFG. This parameter is usually given on the program's command line. |
| IKKUNAT IKKUNAT=xxxx | Requests reading the positions and settings of the forms in use from the file xxxx. If no file name is given, the default name ikkunat.xml is used |
| IKKUNAOTS=xxxx | The text xxxx is written on the title bar of the program window |
| ÄÄNI=x | where x is 0, 1 or 2. ÄÄNI=0 removes all beeps, ÄÄNI=1 enables a beep in error and warning situations, and ÄÄNI=2 also enables a lower notification sound for a competitor placing first or last in the class. Default 1. |
| LISÄÄ=xxx | where x is the largest number of competitors that can be added in one session. The default is 5000 for the total number of competitors and 1000 for additions if the number exceeds 5000. |
| SULKUSALASANA | Makes it possible to shut down the program remotely |
| SÄILHYL | Requests including disqualified competitors too, at the place matching their time in the results list |

#### Parameters only for ViestiMaali and ViestiLuenta

|  |  |
| --- | --- |
| BOOT | The program is started without asking for confirmations. |
| RIVIT=25 | Opens the program in a 25-line window |
| KIRKAS | Bright intensity is used on the screen |
| VÄRIT=xxx | Changes the colours of the console window (implementation incomplete so far) |
| TARKNO=n | Controls the use of a check digit. n = 0: not used, n = 1: used except in the timing function, n = 2: always used (default 0). |

### A1.2 Data transfer parameters

|  |  |
| --- | --- |
| YHTEYSy=xxxx (or COMy=x) | enables data transfer connection number y between two PCs. If y is missing, the connection number is 1. xxxx defines the data transfer method |
| BAUDy=xxxx,p,b,s | Gives the properties of serial connection y. xxxx is the transmission speed of the serial connection, p the parity (n, e or o), b the number of data bits and s the number of stop bits. The default is 9600,n,8,1, and values matching the default can be omitted from the end. |
| PORTBASE=xxxx | Fixed part of the UDP port number, to which the connection number is added. The default is 15900. |
| VÄLITÄy=EI | Prevents the program from forwarding all or some incoming messages to another connection |
| VARASERVER | Indicates that the computer is started in a mode in which it does not send messages |
| KONE=xx | The computer's identifier is xx (two-character identifier), which is shown at the other end of the data transfer connection. |
| COMFILE COMFILE=S | Data transfer is buffered to a disk file. 'S' removes the question about deleting the file at startup. By default buffering with a question. |
| COMFILE=EI | Disables buffering of data transfer to a disk file. |
| MAXYHTEYS=n | Maximum number of connections. Affects only the structure of the file COMFILE.DAT. Usually ignored if COMFILE.DAT already contains messages. |
| FILETCPy=xxxx | Starts automatic sending of XML files at fixed intervals |
| XML=xxxx | Defines e.g. the length of the interval for the previous function |
| XMLSARJA=xxx | Selects the class for the previous function |
| TCPAUTOALKU | Requests starting automatic TCP transfer of files when the program has started |
| NAKVIIVE=xx UDPVIIVEET=xxx TCPVIIVEET=xxx | Parameters for tuning data transfer (not usually needed) |
| MONITORI MONITORI=xxxx | Requests the program to send messages regularly about the state of data transfer. With these messages the program InstanssiMonitori can monitor the state of the whole results service network. Without the specification xxxx, the messages are sent to the network broadcast address and the default port 14900. The specification xxxx contains the combination ip-address:port |
| MONITORVÄLI=xx | Requests changing the interval of monitoring messages to xx seconds (default 10 s). (Note the missing 'I'). |
| LÄHKELLOy=xx | Requests sending the time of day to connection y every xx seconds. (Intended for synchronising the clocks of certain devices.) The addition /VAIN can be appended to the parameter, in which case only times of day are sent to that connection and the connection is otherwise treated as one-way. |

### A1.3 Output-related parameters

|  |  |
| --- | --- |
| LISTA LISTA=nimi | nimi is the name of the printer or file. Without the whole parameter the default is NUL, i.e. no output. The parameter LISTA without a name refers to the default printer |
| LOKI(=tnimi) | requests writing a log to a disk file or to a printer |
| MERKIT=xyy | changes the character set codes of the list printer (x = G, I, R, A, W, P or L), yy indicates which files or printers the codes apply to. The default is G, i.e. use of the Windows printer drivers. W is suitable for output to a text file. The other options should not be used unless you exceptionally want to bypass the Windows printer drivers. |
| MUOTOILU(=tied.nimi) | requests reading the results list formatting settings from a disk file when the program starts. If no file name is given, the name is assumed to be TULLUET.FMT. |
| KIELI=E | English is used in the heading texts of results lists |
| OTSIKOT | Requests writing column headings in results lists |
| AUTO(=xxxxx) | Starts automatic output immediately when the program has started. xxxx contains parameters controlling the output. |
| KOMENTO=xxxxx | Command that the program executes after automatic file output |
| HTML=tied/xx HTML=tied/xx/S | Write an HTML results list automatically every xx seconds. /S indicates that output goes to many class-specific files. |
| HTMLTITLE=nimi | Title of the HTML page to be produced |
| HTMLOTS=otsikko | Heading of the HTML page to be produced |
| XML=tied/xx | Write an XML results list automatically every xx seconds. |

### A1.4 The program's various timing functions are controlled with parameters

|  |  |
| --- | --- |
| REGNLYx=n/..... | Indicates that a Regnly finish clock is used for timing |
| ALGE=n/..... | Indicates that the Alge finish clock Timer S3 is used for timing |
| ALGE4=n/..... | Indicates that the Alge finish clock Timer S4 is used for timing |
| COMET=n/..... | Indicates that the Alge COMET finish clock is used for timing |
| TIMY=n/..... | Indicates that the Alge Timy finish clock is used for timing |
| AIKAERO=xx | Gives in seconds the time of day at which the timing device's clock shows zero. Skips the question at startup. |
| REGNLYNO COMETNO TIMYNO | Indicates that numbers coming from the clock are interpreted as the competitor's number. |
| REGNLYVIIVE=xx | Sets the interval between successive requests when an RTR finish clock is used |
| KELLOBAUD=xxxx | Gives the data transfer speed of the finish clock |
| KELLO\_ESTO=xx | Gives in hundredths of a second the time within which no second time from the finish clock is stored |
| AIKA\_COM=x AIKA\_ESTO=x  AIKA\_MASK=xxx | Parameters for timing via a serial interface. See the relevant chapter. |
| LÄHAIKAy | Indicates that connection y is also used for transferring timing data. y is the connection number as in the parameter YHTEYSy. |
| AJAT=/S AJAT=tied.nimi/S | Gives the name of the timing file. '/S' removes the question about keeping the file (keeps it without asking). |
| AIKAKORJAUSy=xx | Gives the correction by which the times of queue y are automatically changed |
| PISTEET=xxxx | Gives the purposes of the different timing methods |
| JONOT=xxxx | Defines that the recorded times are divided into two or more queues based on the timing method |
| MAXJONO=n | Gives the number of queues available, when it is not evident from other parameters. |
| PISTE=x PISTEy=x JONOPISTEy=x | Gives the nature of the times contained in the different timing queues. When y is missing, the parameter applies to all queues; otherwise y gives the queue. x can be M, L, A or the number of a split point |
| YHTAIKAJONOy=x | Times received from connection y are directed to queue x |
| LÄHDEPISTEET SJBOX | Rules concerning the timing points are read from the file lahdepisteet.lst |
| LÄHTÖRYHMÄ=n | Indicates that in a wave-type start the wave size is 6 teams |
| NÄPPÄIN=mmm/nnn | Gives the key used for timing. (Only in HkMaali). |
| UUSINAIKA | Of the times taken for the same point, the latest (not the first) remains in effect |
| VAINVÄLIAJAT | Prevents automatic inference from marking a time as the final result. |
| ENNAKOI | In the timing function several times can be entered in advance |
| ESTÄYLIM ESTÄYLIM=xx | Prevents saving a timing row when the competitor already has a corresponding result. xx is the interval within which times are compared, by default 2 s. |
| ESTÄNEG ESTÄHAAMUT ESTÄHAAMUT=xx | Prevents recording a time if the result is less than xx minutes or more than 20 hours, which corresponds to a negative time of less than 4 hours. If xx is not given, the lower limit is 1 s. |
| SAMAPISTE=xx | Gives in seconds the limit within which incoming times are interpreted as coming from the same point. |
| LISÄÄEDELLE | In the timing function, a row added with the key F2 is placed before the highlighted time, not after it. |
| SALLITOISTO | Requests saving successive timing messages that appear to be repeats |
| KAKSITUNN | The program recognises two chip codes for the same stage that produce the same result. |
| OLETUSOSUUS=n OSUUSRAJA=n | Give the same information on how legs are determined that can be changed in ViestiMaali with the keys Alt-O on the timing screen |
| YHTEYSAJAT VA-AJAT VAIKAVERT=xxx ILMTUNT(=EI) | Parameters related to handling timing data arriving via data transfer (e.g. from the program SendECodes). Usually only YHTEYSAJAT of these is needed. |
| LÄHECODEy LÄHECODEy=VAIN | Indicates that timing data received on the basis of the parameter YHTEYSAJAT is sent to connection y. y is the connection number as in the parameter YHTEYSy. The additional specification VAIN means that no other messages are sent to this connection. |

### A1.5 Emit function parameters

|  |  |
| --- | --- |
| EMIT | Starts the handling of Emit data without other functions. Not needed when e.g. LUKIJA has been defined. |
| SISÄÄNLUENTA SISÄÄNLUENTA=MYÖS | ViestiWin starts in a mode allowing card readout, in which functions related to punch data are not available. When the parameter includes the addition MYÖS, it is possible to alternate between card readout and punch checking. |
| EMITANALYYSIT | ViestiWin continuously collects analysis data on Emit split times. Needed e.g. for producing split time printouts. |
| LUKIJAx=n LUKIJAx=n/i/a | Data is read directly from the reader device to serial port COMn. x is needed only if there are several readers or MTR devices. |
| EMITAGx=n EMITAGx=n/u | Data is read from an emiTag device. /u indicates that the speed of the USB serial port is 115200 b/s |
| MTRx=n MTRx=n/i/a | Data is read from an MTR device to serial port COMn. x is needed only if there are several readers or MTR devices. When the transfer is from the file EMIT\_IN.DAT, n is 'T'. |
| EMITKELLO=n EMITKELLO=n/w/i/a | Data is read from the RTR2 clock to serial port COMn |
| TARKAVOIMET | The program monitors open legs during card readout |
| SALLIEMITENSIN | ViestiLuenta allows the Emit card to be read before the competitor is identified |
| EMITLUKU | Requests always selecting the highest leg with the correct Emit code |
| LUENTALOKI | Requests writing data on both successful and incomplete card readout events to a separate log file. |
|  | **Punch check parameters** |
| VAADIAIKA | When checking punches, the program points out if the competitor does not yet have a time or cannot get one automatically (used mainly when the time is determined from the punches) |
| JÄLKISYÖTTÖ | In ViestiMaali's punch check a competitor can be linked to the card. |
| VAPAAEMIT=xxx | Number from which Emit codes are assigned when the program has to assign one. |
| LAINAT | Give a notice requiring acknowledgement about a rental card |
| LAINALUETTELO LAINALUETTELO=VAIN | The numbers of rental cards are taken from a file. If VAIN is given, the rental markings in the competitor data are not used |
| LÄHEMITn[=I/O/V] | Emit data read from the card is transferred via connection n. If there is =I or =O at the end, the Emit data transfer is one-way; = V indicates that only separately requested sending of a single record is allowed. |
| LÄHEMVAn[=I/O] | Instead of the normal Emit data, the split times calculated from them are sent. |
| AUTORAP | Requests automatically printing the data of every card read |
| AUTORAP=x | x=1, H or S. Prints some of the above reports (not approved ones) |
| COMAUTORAP COMAUTORAP=H | AUTORAP function in effect on the receiving computer. If H is given, applies only to disqualification proposals. The parameter is given on the computer where the printing takes place. |
| RASTIVATULOSTE | Starts the program in a mode in which reading a card produces a split time printout (not yet implemented) ([chapter 6.4](6.8_rastivaliaikatulosteet.md) ) |
| KARTTA=xxxx | Map file used to support the complaints desk ([chapter 6.8](6.8_kartan_kaytto_itkumuurilla.md) ) |
| RAPORTTIHAK=xxxx | Reports on handling disqualifications are written to the directory xxxx ([chapter 6.12](6.12_hylkaysesitysten_kasittely.md)) |
| SIVUJAKO=n | The Emit split times of n competitors are printed compactly on the same sheet. |
| VALONÄYTTÖ=yy | Enables the Emit reader "traffic lights" for the reader defined with the parameter LUKIJA=n (without the number x). yy is the duration of the light signal in tenths of a second. |
| VALOT=n | Directs the traffic light control to a different serial port, given with the parameter n. |
| ESITARK | ViestiMaali is used for pre-checking, in which case the program otherwise works as in punch checking, but course data is not used and nothing is recorded in the competitor data |
|  | **Parameters for timing based on Emit chips** |
| AIKALUKIJA AIKALUKIJA=VAIN AIKALUKIJA=VAINx AIKALUKIJAy=VAINx AIKALUKIJAy=LÄHDEz | The reading moment of a reader control is stored even if no data other than its number is obtained from the card. VAIN: always only the reading moment. x indicates the point and y the reader. z gives the source code assigned to messages arriving from reader y. |
| LÄHDEPISTEET | The file lahdepisteet.lst is used to infer the split point. |
| AUTOKILP | Records the read competitors automatically in the timing data, linking them to the times given by the finish clock. |
| LEIMAT=E | The file LEIMAT.LST is not read even if it is in the startup directory. Makes timing work more logically in some cases. |
| LISÄÄEDELLE | A time added on the timing screen is 0.1 s better than the earlier highlighted time. Used when online timing is supplemented with order information determined by the finish judge. |
| ESTÄEMITTOISTO=EI SALLIEMITOISTO | Nearly successive readings of the same Emit card are recorded (normally prevented). |
| JOUSTOVIESTI | The program identifies the team and runner by the Emit code, even if the leg in turn has no data |
| ECAIKA ETGPRS ETHAKUVÄLI ETDATE ETTIME ETHOST | Control parameters for using the split time server related to emiTag |

### A1.6 Parameters related to using an SQL database

|  |  |
| --- | --- |
| SQLHOST=host | Defines the name of the MySQL server (default 'localhost') |
| SQLDATABASE=database | Defines the name of the MySQL database (default 'kilp') |
| SQLUSER=user | Defines the MySQL database user (default 'kilp') |
| SQLPASSWORD=salasana | Defines the password of the MySQL database user (default 'kilp') |
| SQLSTART | Requests opening the MySQL database automatically when the program starts |
| SQLTALLENNUS | Requests opening the MySQL database so that all changes to the competitor data are saved automatically |

### A1.7 Miscellaneous parameters that do not work in all program versions

|  |  |
| --- | --- |
| TAULU\_COM=x TAULU\_BAUD= TAULUVIIVE=xx GAZ=x GAZVAIHE=x GAZRIVIy= | Parameters related to controlling various results boards |
| SIRIT SIRITREUNA SIRITARRIVE SIRITDEPART IMPINJ  Parameters related to using RFID tags for timing | |
| SW2000=xx | SW2000 swimming clock device in use |
