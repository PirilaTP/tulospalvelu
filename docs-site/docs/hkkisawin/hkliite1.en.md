# Appendix 1. Startup parameters

## Appendix 1. Startup parameters

The operation of the programs is controlled with startup parameters.
When working on a single computer, many basic functions need no
startup parameters at all, but they are essential, for example, when connecting computers
to work together over a network. Parameter names can be
written in upper or lower case, as can the values given
with the parameters. Only in a few text fields, such as when defining
the title, does the program keep lower case letters; in all other
cases the program converts the letters to upper case before use.

A [form](1.3_toimintatilojen_konfigurointi.md)
is available for editing configuration files, which makes
most definitions easier. In the table,
the parameters that can be defined with the configuration editor are marked with asterisks. \*\*\* means
that the parameter can be fully defined, \*\*
and \* that the parameter can be defined,
but not all of its sub-settings can be made.

### A1.1 Basic and miscellaneous parameters

|  |  |  |
| --- | --- | --- |
| CFG=tied.nimi |  | Defines the file from which the other parameters are read. The default is LASKENTA.CFG . This parameter is most commonly used on the command line or in shortcut settings. |
| VAIHE=n PÄIVÄ=n | \*\*\* | Gives the stage of the competition in multi-stage competitions. When n is the letter V, the stage is determined from the file vaihevalinta.cfg. |
| IKKUNAT IKKUNAT=xxxx | \*\*\* | Requests that the positions and settings of the forms in use be read from the file xxxx. If no file name is given, the default name ikkunat.xml is used |
| IKKUNAOTS=xxxx | \*\*\* | The text xxxx is written on the title bar of the program window |
| ÄÄNI=x | \*\*\* | where x is 0, 1 or 2. ÄÄNI=0 removes all beeps, ÄÄNI=1 enables a beep in error and warning situations and ÄÄNI=2 also enables a lower notification sound for a competitor placed first or last in the class. Default 1. |
| LISÄÄ=xxx | \*\*\* | where x is the maximum number of competitors that can be added in one session. The default is the total number of competitors 5000 and an addition of 1000 if the number exceeds 5000. |
| SAKOT=xxx |  | The unit in seconds used in handling penalties, handicaps and additional time. **Avoid using this parameter and define the size of the penalties in the competition properties and in the data of each class with penalties.** |
| JUOKSUILM | \*\*\* | in the correction function, start from the name field (otherwise from the disqualification field). In HkKisaWin it affects the default settings of the entry form. |
| KUNTOSUUNNISTUS=x | \*\*\* | where x is 1, 2 or 3, makes the program WinKisaHk operate in fitness orienteering mode |
| VAKANTTIALKU=nnnn | \*\*\* | Defines, in connection with the previous one, the range of free numbers to be used. The same starting number is also used in renumbering EMIT codes unless this is overridden with the parameter EMITALKU=nnn. |
| LISÄVAKANTTIALKU=nn | \*\*\* | Gives the lower limit of the number range of automatically created vacancies (by default they are created at the top of the number range). |
| VAKANTTILUKU=nnn | \*\*\* | Gives the number of vacancy records that the program creates automatically in a new entry file in orienteering (requires user approval). The default is 20. A considerably larger number (e.g. 1000) is suitable for fitness orienteering events. The vacancies are created at the top of the number range in use (usually ending at 9999), unless the parameter LISÄVAKANTTIALKU has been given at the creation stage, in which case the numbers start from the value given there. |
| SEURATIEDOT=x | \*\*\* | where x is H or E. Gives the extent of club data handling. When x=H, searching the club list is used as an aid. When x=E, club data are not shown at all, for example in the punch check window. |
| HENKILÖTIETOKANTA=xx | \*\*\* | Gives the person database file. xx is the full name of the file to be used, including the directory path. The default is henkilot.csv in the competition folder. |
| SEURATIEDOSTO=xx | \*\*\* | Gives the club list file. xx is the full name of the file to be used, including the directory path. At the same time, searching the club list is enabled. The default is seurat.csv in the competition folder. |
| KUNTOLÄHTÖ | \*\*\* | Enables a simplified form suitable for registration when the function *Punch check.* is selected. |
| KUNTOMAALI | \*\*\* | Selects the option *Finish computer.* on the fitness orienteering punch check screen. |
| VAHVISTAAINA=EI | \*\*\* | Indicates that in fitness orienteering mode a person found in the database need not be confirmed. |
| SULKUSALASANA | \*\*\* | Makes it possible to shut down the program remotely |

#### Parameters for HkMaali only

|  |  |  |
| --- | --- | --- |
| BOOT BOOT=Mn | \*\*\* | The program is started without asking for confirmations. The addition =Mn requests that the timing screen (option *Maali*) be opened immediately. You must separately make sure, using for example the parameters AIKAERO, NÄPPÄIN, AJAT=/S and COMFILE=S, that the program skips any other questions. |
| RIVIT=25 | \*\*\* | Opens the program in a 25-line window |
| KIRKAS | \*\*\* | Bright intensity is used on the screen |
| VÄRIT=xxx | \*\*\* | Changes the colours of the console window (implementation incomplete for now) |
| TARKNO=n |  | Controls the use of the check digit. n = 0: not used, n = 1: used except in the timing function, n = 2: always used (default 0). |
| KUNTO SUORASYÖTTÖ | \*\* | Intended for fitness orienteering events. Allows results to be given directly when adding a competitor in HkMaali. |
| FISKOODI |  | Enables handling of FIS codes and calculation of FIS points. Not needed in HkKisaWin. |

### A1.2 Data transfer parameters

|  |  |  |
| --- | --- | --- |
| YHTEYSy=xxxx (tai COMy=x) | \*\*\* | enables data transfer connection number y between two PCs. If y is missing, the connection number is 1. xxxx defines the data transfer method |
| BAUDy=xxxx,p,b,s | \*\* | Gives the properties of serial port connection y. xxxx is the transmission speed of the serial port connection, p the parity (n, e or o), b the number of data bits and s the number of stop bits. The default is 9600,n,8,1, and values equal to the default can be left out from the end. |
| PORTBASE=xxxx | \*\*\* | The constant part of the UDP port number, to which the connection number is added. The default is 15900. |
| VÄLITÄy=EI | \*\*\* | Forbids the program from forwarding all or some of the incoming messages to another connection |
| VARASERVER | \*\*\* | Indicates that the computer is started in a mode in which it does not send messages |
| KONE=xx | \*\*\* | The computer's identifier is xx (two-character identifier), which is shown at the other end of the data transfer connection. |
| COMFILE COMFILE=S | \*\*\* | Data transfer is buffered in a disk file. 'S' removes the question about deleting the file at startup. By default buffering with the question. |
| COMFILE=EI | \*\*\* | Disables buffering of data transfer in a disk file. |
| MAXYHTEYS=n | \*\*\* | Maximum number of connections. Affects only the structure of the file COMFILE.DAT. Usually ignored if COMFILE.DAT already contains messages. |
| TCPTULOS=x |  | Requests that in a two-day competition the overall result be sent to the TCP connection in addition to or instead of the day's result. |
| FILETCPy=xxxx |  | Starts automatic sending of XML files at set intervals |
| XMLSARJA=xxx |  | Selects the class for the previous function |
| TCPAUTOALKU |  | Requests that automatic TCP transfer of files be started when the program has started |
| NAKVIIVE=xx UDPJARRU=xx UDPVIIVEET=xxx |  | Parameters for fine-tuning data transfer (not usually needed) |
| MAALILEIMASIN=EI | \*\*\* | The time of the last punch is included in the control split time record even though, according to the course data, it is the time of the finish unit, which is not wanted in the online results service alongside the actual finish time. |
| MONITORI MONITORI=xxxx |  | Requests the program to send regular messages reporting the status of the data transfer. The program InstanssiMonitori can use these messages to monitor the status of the whole results service network. Without the specification xxxx, the messages are sent to the network broadcast address and the default port 14900. The specification xxxx contains the combination ip-address:port |
| MONITORVÄLI=xx |  | Requests that the interval of monitoring messages be changed to xx seconds (default 10 s). (Note the missing 'I'.) |
| LÄHKELLOy=xx | \*\*\* | Requests that the time of day be sent to connection y every xx seconds. (Intended for synchronising the clock of certain devices.) The suffix /VAIN can be added to the parameter, in which case only times of day are sent to that connection and the connection is otherwise treated as one-way. |
| LÄHTIEDOSTOT LÄHTIEDOSTOTy=x | \*\* | Enables file transfer through the results service network. The first alternative applies to all UDP connections, the latter only to connection y. If x = 0, use is cancelled for this connection. /K added to the end of the command indicates that files can be transferred from elsewhere than under the competition directory. |

### A1.3 Output-related parameters

|  |  |  |
| --- | --- | --- |
| LISTA LISTA=nimi | \*\*\* | nimi is the name of the printer or file. Without the whole parameter the default is NUL, i.e. no output. The parameter LISTA without a name refers to the default printer |
| LOKI(=tnimi) | \*\*\* | requests that a log file be written to a disk file or to a printer |
| MERKIT=xyy |  | changes the character set codes of the list printer (x = G, I, R, A, W, P or L), yy indicates which files or printers the codes apply to. The default is G, i.e. use of Windows printer drivers. W is suitable for output to a text file. Other alternatives should not be used unless you exceptionally want to bypass the Windows printer drivers. |
| MUOTOILU(=tied.nimi) | \*\*\* | requests that the results list formatting settings be read from a disk file at program startup. If no file name is given, the name TULLUET.FMT is assumed. |
| AUTO(=xxxxx) | \*\*\* | Starts automatic output immediately when the program has started. xxxx contains parameters controlling the output. |
| VAADITARKASTUS | \*\*\* | A class is considered complete only when everyone has been given some check mark and all accepted competitors have a result. When the class is complete, automatic output produces a results list even if the other rules would not produce one immediately. |
| NOLLAKÄRKI | \*\*\* | In the results, the time '00.00.00' is written in the difference column for the winner. |
| TAKAA-AJO | \*\*\* | The split times of the 2nd day are shown as total times including the time of the 1st day |
| HTML=tied/xx HTML=tied/xx/S MHTML=xx | \*\*\* | Write an HTML results list automatically every xx seconds. /S indicates that the output goes to many class-specific files. M refers to mobile formatting. |
| HTMLTITLE=nimi | \*\*\* | Name of the HTML page to be produced |
| HTMLOTS=otsikko | \*\*\* | Heading of the HTML page to be produced |
| HTMLSÄILNIMI | \*\*\* | The order of first and last name is not swapped in HTML outputs |
| XML=tied/xx | \*\*\* | Write an XML results list automatically every xx seconds. |
| KOMENTO=xxxxx | \*\*\* | A command that the program executes after automatic file output |
| DESEROTIN=x |  | Selects the character 'x' (comma or point) as the decimal separator in XML outputs. |
| KILPKDI=xxx |  | Defines the competition code to be output in certain file outputs. Being phased out; define it as stage-specific competition data. |
| LÄHETÄCR=EI |  | Leaves out the carriage return character when sending a form feed character to the printer |

### A1.4 The program's various timing functions are controlled with parameters

|  |  |  |
| --- | --- | --- |
| SYÖTTÖ |  | Indicates that the HkMaali function M)aali (finish) is used for entering times by typing instead of timing. |
| REGNLYx=n/..... | \*\*\* | Indicates that a Regnly finish clock is used for timing |
| ALGE=n/..... | \*\*\* | Indicates that the Alge finish clock Timer S3 is used for timing |
| ALGE4=n/..... | \*\*\* | Indicates that the Alge finish clock Timer S4 is used for timing |
| COMET=n/..... | \*\*\* | Indicates that the Alge COMET finish clock is used for timing |
| TIMYx=n/..... | \*\*\* | Indicates that the Alge Timy finish clock is used for timing. x gives the device number (default: 1). |
| ASC=n | \*\*\* | Indicates that the Alge ASC1 start clock is used for timing |
| AIKAERO=xxxx | \*\*\* | Gives in seconds the time of day at which the timing device's time is 0. The parameter causes the question about the time difference to be skipped. |
| KELLONO | \*\*\* | Indicates that numbers coming from the clock are interpreted as the competitor's number. (REGNLYNO, COMETNO and TIMYNO work in the same way) |
| TUNNKOODIT(=E) |  | Enables or disables the use of the identification code |
| REGNLYVIIVE=xx |  | Sets the interval between successive requests when using the RTR finish clock |
| KELLOBAUD=xxxx | \*\*\* | Gives the data transfer speed of the finish clock |
| KELLO\_ESTO=xx |  | Gives in hundredths of a second the time within which a second time from the finish clock is not saved |
| AIKA\_COM=x AIKA\_ESTO=x | \* | Parameters for timing through a serial interface. See the relevant chapter. |
| LÄHAIKAy | \*\*\* | Indicates that connection y is also used for transferring timing data. y is the connection number as in the parameter YHTEYSy. |
| AJAT=/S AJAT=tied.nimi/S | \*\* | Gives the name of the timing file. '/S' removes the question about keeping the file (keeps it without asking). |
| PAKOTALAIKA |  | Prevents the use of the skiing 3-second rule when handling start times. Also allows the start time to be saved from a timing point that is marked with the letter L as the point in the file lahdepisteet(x).lst (see LÄHDEPISTEET). Used, for example, when the start time is read from an RFID reader. |
| PAKOTALAIKARAJA=n |  | Used only with the PAKOTALAIKA parameter. Gives in seconds the largest allowed difference between the forced start time registration and the start time already saved. If the difference is greater than n, the forced start time is not recorded again. Default 0, i.e. no limit. |
| LÄHTÖIKKUNA=x |  | Changes the time limit of the "3-second rule" to x s. |
| LAJAT LAJAT=tied.nimi | \*\* | Start gate times are saved in a different file from other times |
| TAUSTA-AIKA |  | Makes it possible to open a second time queue in the small window of HkMaali |
| AIKAKORJAUSy=xx |  | Gives the correction by which the times of queue y are changed automatically |
| LÄHTÖRYHMÄ=n |  | Gives the size of a group starting at the same moment to make it easier to record the start time obtained from the clock for the whole group |
| PISTEET=xxxx | \*\* | Gives the purposes of the different timing methods |
| JONOT=xxxx | \*\*\* | Defines that the times taken are divided into two or more queues |
| MAXJONO=n | \*\*\* | Gives the number of time queues to be handled. (Not needed when the number is evident from the JONOT parameter.) |
| JONOPISTEy=x |  | Indicates which results are obtained from the different timing queues |
| VERTRAJAT=xxx |  | Sets the comparison limits for comparing two timing queues |
| NÄPPÄIN=mmm,nnn | \*\*\* | Gives the key used for timing. (Only in HkMaali.) |
| VAVUOROT VAVUOROT=sss/a/y | \*\*\* | Uses, in determining the split point, only the sequence number of the time or the time elapsed since the previous recorded split time (and thus not the time from the start). sss gives in seconds the period within which arriving times are treated as one; a and y are the lower and upper limits of the points allowed in the rotation. |
| VAINALARAJAT VAINALARAJAT=EI |  | Changes the way the VAVUOROT parameter uses the lower limits of split times |
| UUSINAIKA |  | Of the times taken for the same point, the latest (not the first) remains in effect |
| VAINVÄLIAJAT | \*\*\* | Prevents automatic determination from marking a time as the final result. |
| ENNAKOI | \*\*\* | Several times can be entered in advance in the timing function of HkMaali (a menu item in HkKisaWin) |
| ESTÄMUUTOS |  | Prevents making changes on the timing screen |
| ESTÄYLIM |  | Prevents saving a timing row when the competitor already has a corresponding result |
| ESTÄHAAMUT=xx |  | Prevents a time from being recorded if the result is less than xx minutes or more than 20 hours, which corresponds to a negative time of less than 4 hours |
| LISÄÄEDELLE |  | In the timing function of HkMaali, a row added with the F2 key goes before the highlighted time and not after it. |
| SALLITOISTO |  | Requests that successive timing messages that appear to be repeats be saved |
| TUNNJONO TUNNJONO=y |  | Indicates that the chip-based timing queue y is used only for identifying the competitor. Used when the time is taken, for example, with a photocell. The default queue is 2. When using it, the shorter form is sufficient. |
| TUNNRAJAT=a/y |  | Time limits in seconds related to the previous one. a is often negative and then indicates that the time may come slightly before the identification data. y gives the maximum time from the identification data to the time taken. |
| KAKSITUNN | \*\*\* | The program recognises two chip codes in the same stage that produce the same result. |
| YHTEYSAJAT VA-AJAT VAIKAVERT=xxx ILMTUNT(=EI) | \* | Parameters related to handling timing data arriving through a data transfer connection. Used, among other things, when the program SendECodes is in use. The parameter YHTEYSAJAT (or VA-AJAT) enables this functionality. |
| LÄHECODEy LÄHECODEy=VAIN | \*\*\* | Indicates that the timing data received on the basis of the YHTEYSAJAT parameter are sent to connection y. y is the connection number as in the parameter YHTEYSy. The additional setting VAIN means that no other messages are sent to this connection. |

### A1.6 Special parameters for biathlon

|  |  |  |
| --- | --- | --- |
| RACPISTEET | \*\*\* | When using the Regnly clock RTR2 and the RAC box, the shooting stage is identified by the input of the RAC box |
| SAKKO\_COM=n SAKKO\_YHT=n | \*\*\* | The program receives penalty data from Kurvinen equipment or from an input point emulating it through port COMn. |
| SAKKO\_BAUD=nnnn |  | Data transfer definition parameters when using the SAKKO\_COM parameter. |
| SAKKO\_LAJI=n | \*\*\* | Gives the version of the Kurvinen equipment in use. n=0 for the oldest equipment, default: n=2. |

### A1.7 Parameters for Emit functions

|  |  |  |
| --- | --- | --- |
| EMIT | \*\*\* | Starts handling of Emit data without other functions. Not needed when, for example, LUKIJA has been defined. |
| EMITANALYYSI | \*\*\* | Starts continuous analysis of control split times. |
| LUKIJAx=n LUKIJAx=n/i/a | \*\* | Data are read directly from the reader device to serial port COMn. x is needed only if there are several readers or MTR devices. |
| EMITAGx=n EMITAGx=n/u | \*\*\* | Data are read from an emiTag device. /u indicates that the speed of the USB serial port is 115200 b/s |
| MTRx=n MTRx=n/i/a | \*\* | Data are read from an MTR device to serial port COMn. x is needed only if there are several readers or MTR devices. When the transfer is from the file EMIT\_IN.DAT, n is 'T'. |
| EMITKELLO=n EMITKELLO=n/w/i/a |  | Data are read from the RTR2 clock to serial port COMn |
|  |  | **Punch check parameters** |
| VAADIAIKA | \*\*\* | When checking punches, the program warns if the competitor does not yet have a time or does not get one automatically (used mainly when the time is determined from punching) |
| LAINAT | \*\*\* | Warn about a rental card with a message that must be acknowledged |
| LAINALUETTELO LAINALUETTELO=VAIN | \*\*\* | The numbers of rental cards are obtained from a file. If VAIN is given, the rental markings in the competitor data are not used |
| LÄHEMITn[=I/O/V] | \*\*\* | Emit data read from the card are transferred through connection n. If =I or =O is at the end, the transfer of Emit data is one-way; = V indicates that only the sending of a single, separately requested record is allowed. |
| LÄHEMVAn | \*\*\* | Instead of the normal Emit data, the split times calculated from them are sent. |
| AUTORAP | \*\*\* | Requests that the data of every card read be printed automatically |
| AUTORAP=x | \*\* | x=1, H or S. Prints some of the above reports (not accepted ones) |
| COMAUTORAP COMAUTORAP=H | \*\*\* | AUTORAP operation in effect on the receiving computer. If H is given, applies only to proposed disqualifications. The parameter is given on the computer where the printing takes place. |
| RAPORTTIHAK=xxxx |  | Reports on the handling of disqualifications are written to the directory xxxx ([chapter 6.16](6.16_hylkaysesitysten_kasittely.md)) |
| ANNASEUR |  | When the question for a new bib number is repeated, always offers the next number. |
| AUTOSARJA | \*\*\* | When reading an Emit card, change the class to the one matching the punches |
| AUTORATA | \*\*\* | Uses operation similar to AUTOSARJA for courses that are not class-specific. |
| RASTIVATULOSTE | \*\*\* | Starts the program in a mode where reading a card produces a split time printout ([chapter 6.8](6.8_rastivaliaikatulosteet.md) ) |
| KARTTA=xxxx | \*\*\* | Map file used to support the complaints desk ([chapter 6.12](6.12_kartan_kaytto_itkumuurilla.md) ) |
| MAALILEIMASIN=EI | \*\*\* | Requests that both the time of the last punch and the final result be included in the control split time lists, even though both often refer to the finish line time. Affects control split time printouts and the data sent to the online service. |
| SIVUJAKO=n |  | The Emit split times of n competitors are printed compactly on the same sheet. |
| EMITVASIVU=l/m/n |  | Split time printout line length=l, number of lines m, and of these the top n are left empty. |
| TARKRAPOHITA=n |  | Requests that n empty lines be left at the top of the punch check report, for example for the organiser's logo. |
| LÄHTÖVERT=xxx |  | The program warns if the reset time of the Emit card differs from the drawn start time by more than xxx s. |
| LUESUORAAN |  | Requests that the MTR data be unloaded without interruption, adding records automatically |
| EMITALKU=nnnn | \*\*\* | Gives the number from which a free Emit code is searched for when the program changes the code automatically. |
| VALONÄYTTÖ=yy |  | Enables the Emit reader "traffic lights" for the reader defined with the parameter LUKIJA=n (without the number x). yy is the duration of the light signal in tenths of a second. |
| VALOT=n |  | Directs the traffic light control to a different serial port, given with the parameter n. |
| ESILUENTA | \*\*\* | The program is used for pre-reading of starters, in which case the competitors are marked as open (previously as not started) |
| ESILUENTA=KUNTO | \*\*\* | HkKisaWin is used for pre-reading of starters, attaching the Emit codes to the competitor data only at this stage. Otherwise the same as the previous one. |
| ESITARK |  | HkMaali is used for pre-checking, in which case the program otherwise works as in the punch check, but course data are not used and no data are recorded in the competitor data |
| EILÄHT=AVOIN |  | A parameter useful in some fitness event organisation models, needed when participants are marked as not started until they bring their card to be read. Without this parameter every readout generates an extra alarm. |
| EMLOKI |  | Emit events are recorded in a log file. |
|  |  | **Parameters for Emit chip-based timing** |
| AIKALUKIJA AIKALUKIJA=VAIN AIKALUKIJA=VAINx AIKALUKIJAy=VAINx AIKALUKIJAy=LÄHDEz | \*\*\* | The reading time of a reader control is saved even if no other data than its number are obtained from the card. VAIN: always only the reading time. x gives the point, y the reader and z the source code given to timing data coming from reader y. |
| LÄHDEPISTEET | \*\*\* | The file lahdepisteetx.lst, where x is the stage number, is used to determine the timing point. This parameter is often needed, for example, when SjBox is in use. The letter L can also be marked as the point in the file, in which case the code is interpreted as a start time instead of finish (M); this additionally requires the PAKOTALAIKA parameter. |
| AUTOKILP |  | Records the read competitors automatically in the timing data, attaching them to the times given by the finish clock. |
| LEIMAT=E |  | The file LEIMAT.LST is not read even if it is in the startup directory. In some cases makes timing work more logically. |
| EMITAJAT EMITAJAT=n | \*\*\* | Times determined in the punch check are written to the timing file. The parameter n gives the timing queue if several are in use. |
| LISÄÄEDELLE |  | A time added on the timing screen is 0.1 s better than the previous highlighted time. Used when online timing is supplemented with order information determined by the finish judge. |
| ESTÄEMITTOISTO=EI SALLIEMITOISTO | \*\*\* | Almost consecutive readouts of the same Emit card are recorded (normally prevented). |
| ECAIKA ETGPRS ETSQL ETHAKUVÄLI ETDATE ETTIME ETHOST |  | Control parameters for using the split time server related to emiTag |

### A1.7 Parameters related to using an SQL database (HkKisaWin only)

|  |  |  |
| --- | --- | --- |
| SQLHOST=host |  | Defines the name of the MySQL server (default 'localhost') |
| SQLDATABASE=database |  | Defines the name of the MySQL database (default 'kilp') |
| SQLUSER=user |  | Defines the MySQL database user (default 'kilp') |
| SQLPASSWORD=salasana |  | Defines the password of the MySQL database user (default 'hkkisawin') |
| SQLSTART |  | Requests that the MySQL database be opened automatically when the program starts |
| SQLTALLENNUS |  | Requests that the MySQL database be opened so that all changes to the competitor data are saved automatically |

### A1.8 Miscellaneous parameters that do not work in all program versions

|  |  |  |
| --- | --- | --- |
| TAULU\_COM=x TAULUVIIVE=xx GAZ=x GAZVAIHE=x GAZRIVIy= |  | Parameters related to controlling various scoreboards |
| SIRIT SIRITREUNA SIRITARRIVE SIRITDEPART FEIG IMPINJ |  | Parameters related to using RFID tags for timing |
| SW2000=xx |  | The SW2000 swimming timing device is in use |
| BIBSIIRTOALUE |  | Range for the numbers of inactive records |
