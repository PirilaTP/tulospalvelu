# Bilaga 2. Ibruktagning av dataöverföringen

## Bilaga 2. Ibruktagning av dataöverföringen

### A2.1 Dataöverföring inom resultattjänstnätverket via TCP/IP-nätverk

Mer information om möjligheterna att använda dataöverföringen finns i dokumentet HkMaali.pdf.

I förbindelser mellan datorer som använder resultatprogrammen
används i allmänhet UDP-protokollet. TCP-protokollet är tillgängligt
när UDP inte fungerar, till exempel via en mobilförbindelse. Även dataöverföring via serieportar och
nollmodemkablar fungerar fortfarande i programmen.

En UDP-förbindelse förutsätter att
var och en av de två datorer som använder förbindelsen reserverar en UDP-port för att ta
emot meddelanden från motparten. Standardvärdet för denna port är 15900 + n,
där n är förbindelsens nummer, så portens nummer är känt när förbindelsens nummer
är känt. Det värde som används vid beräkningen av portnumret kan ändras med parametern

PORTBASE=pohja-arvo

vars motsvarande standard alltså är PORTBASE=15900
Portnumret kan också ändras per förbindelse, som beskrivs nedan. Om
inte heller förbindelsens nummer har angetts antar programmet att det gäller
förbindelse 1 och porten därmed är 15901. Förbindelsens nummer är datorspecifikt. När UDP-förbindelser används
är det vanligt att endast en dator, "servern", använder flera förbindelser, medan alla
andra datorer då kan använda endast en förbindelse. Om det också finns en reservserver
måste även de andra datorerna använda två förbindelser, till exempel så att förbindelse 1
används till servern och förbindelse 2 till reservservern.

I allmänhet är det mest praktiskt att definiera förbindelsen så
att ip-adressen och motpartens port definieras endast i den ena ändan av förbindelsen.
Då sänder denna part av förbindelsen sin egen ip-adress och port
till motparten, varefter förbindelsen fungerar i båda riktningarna. Inte heller motpartens
port behöver definieras separat, om det gäller motpartens förbindelse
1, eftersom denna förbindelses port 15901 är standard. (Parametern PORTBASE
påverkar också detta numeriska värde.)

UDP-dataöverföring mellan två datorer startas
genom att på båda datorerna ange parametern YHTEYS
i fullständig form:

`YHTEYSn=UDP:omaportti/vastaip:vastaportti`

eller i enklare former, där åtminstone en del av adressparametrarna utelämnas,
varav exempel följer nedan

`YHTEYSn=UDP
YHTEYSn=UDP:omaportti
YHTEYSn=UDP:0/vastaip`

Om uppgiften omaportti saknas helt eller är 0 eller består av bokstäver, använder programmet
värdet 15900 + n för den egna porten, där n är förbindelsens nummer. Om vastaip eller vastaportti
ska anges måste fältet omaportti innehålla 0 eller någon bokstav, dvs.
tecknen : och / kan inte följa direkt efter varandra.

Om parametern vastaip saknas eller dess värde är AUTO, väntar programmet i sin egen port på
ett meddelande från motparten, ur vilket det får reda på både motpartens ip-adress och motpartens port.
Det finns lite mer information om detta i slutet av detta kapitel. När motpartens ip-adress inte anges
behöver alltså inte heller vastaportti anges, eftersom programmet också får denna uppgift ur det meddelande det tar emot.

Standardvärdet för uppgiften vastaportti är 15901, dvs. standard är att det gäller motpartens förbindelse 1,
för vilken ingen avvikande port har definierats. Om man i fältet vastaportti skriver ordet
yhteys och ett nummer n, väljer programmet 15900 + n som motpartens port, dvs. antar att
det gäller standardporten för motpartens förbindelse n. (Här kan man i stället för ordet yhteys använda
enbart bokstaven Y.)

Direkt efter ordet UDP kan bokstaven i eller o stå för att ange att uppgifter
endast tas emot (incoming) eller endast sänds (outgoing).

Exempel som förtydligar saken:

Exempel 1: Förbindelser som använder standardportar

I detta exempel kopplas en "server" till tre andra datorer. För servern
definieras dataöverföringarna enligt följande

`yhteys1=udp:0/192.168.1.12
yhteys2=udp:0/192.168.1.13
yhteys3=udpo:0/192.168.1.14:yhteys2`

I andra ändan av den första förbindelsen är parametern

`yhteys1=udp`

Parametern för motparten till den andra förbindelsen är

`yhteys1=udp`

Parametern för motparten till den tredje förbindelsen anger att det gäller enkelriktade
inkommande uppgifter samt att det gäller denna dators förbindelse 2:

`yhteys2=udpi`

Varje förbindelse förutsätter att den ena av förbindelsens ändar har fått
motpartens ip-adress och att denna ända också känner till motpartens värde för omaportti korrekt.
Denna port behöver dock inte skrivas som parameter, om det gäller motpartens förbindelse 1.

Exempel 2: En förbindelse där alla värden har definierats i parametrarna:

UDP-protokollet tas i bruk för dataöverföringen genom att ange förbindelseparametern på den dator
vars ip-nummer till exempel är 192.168.1.11 i formen

`YHTEYS1=UDP:1250/192.168.1.12:1350`

när datorn i andra ändan har ip-numret 192.168.1.12. Där i andra ändan ska i sin tur parametern

`YHTEYS1=UDP:1350/192.168.1.11:1250`

anges. Det första numeriska värdet är den port där mottagningen av UDP-meddelanden öppnas på datorn i fråga
och efter snedstrecket kommer den andra datorns ip-nummer och UDP-port.
Varje förbindelse använder alltså UDP-mottagning i båda ändarna. Meddelandena sänds
alltid till den mottagande porten, som lyssnar kontinuerligt och svarar till den sändande porten,
som systemet automatiskt tilldelar separat för varje meddelande. Varje
dator kan ha förbindelse med mycket många andra datorer och för varje förbindelse måste
alltså en egen mottagande port reserveras.

De portar som är reserverade på datorn kan du ta reda på med kommandot

`NETSTAT /A`

och de UDP-portar som det anger ska inte användas. De portnummer som användes ovan,
1250 och 1350, var bara två nummer som jag valde slumpmässigt. Vilka lediga
portar som helst går lika bra. Som sagts ovan är standardvärdena
portar från och med nummer 15901.

I stället för en numerisk framställning kan man som parameter vastaip också använda en adress i textform
för måldatorn. För att man i en tävlingssituation inte ska vara beroende av att namnservern
fungerar rekommenderar jag att alla namn i textform som används definieras
i filen hosts, som typiskt finns i katalogen \WINNT\system32\drivers\etc.

När man vill använda flera kopior av resultatprogrammen på samma dator
kan man som parameter vastaip använda adressen localhost, som alltid hänvisar till
ip-numret 127.0.0.1. Adressen localhost kan användas även om datorn inte
har något nätverkskort alls, förutsatt att tcp/ip-funktionerna har installerats.
I förbindelser inom datorn kan portarnas standardvärden inte användas, om inte olika
program tilldelas olika förbindelsenummer, eftersom samma port inte kan reserveras
för mer än ett program.

Som parameter vastaip kan man i den ena ändan av förbindelsen
också ange ordet AUTO. När man gör så väntar datorn i fråga på ett UDP-meddelande
i porten omaportti och använder därefter ip-numret för avsändaren av det mottagna meddelandet.
Detta ip-nummer kan dessutom ändras medan programmet körs,
eftersom programmet alltid använder ip-numret för avsändaren av det senaste UDP-meddelande
som tagits emot i porten omaportti. Detta förfarande är användbart både för att
förenkla parametrarna och i en situation där ip-numret för förbindelsens andra part inte
är känt i förväg eller till och med kan variera. En sådan
situation uppstår främst när förbindelsen upprättas via internet med en
offentlig tjänsteleverantör. I den andra ändan måste motpartens ip-nummer dock alltid anges
för att förbindelsen ska kunna upprättas.

Programmet kan i en TCP-förbindelse fungera både
som server och som klient. Parametern är av typen

`YHTEYS5=TCP:192.168.1.12:1350`

där ip-adressen och porten hänvisar till den server
som används, när programmet är klient, och

`YHTEYS5=TCPS:1355`

där det numeriska värdet är anslutningsporten, när programmet fungerar som server.

När man vill sända uppgifter i textform
via TCP-förbindelsen läggs bokstaven X till (TCPX eller TCPSX). För närvarande är formatet enkelt, alltså inte xml-format som i programmen för individuella tävlingar.

### A2.2 Konfigurering av det lokala nätverket för UDP-dataöverföring

Programmets funktion förutsätter endast att en förbindelse mellan datorerna uppstår, så
ingenting som avviker från konfigureringen av ett vanligt TCP/IP-baserat lokalt nätverk
behövs. Om de datorer som används inte annars finns i samma lokala nätverk måste
typiskt följande åtgärder vidtas:

- Bygg ett fysiskt nätverk som består av en eller
  flera switchar (switch) eller hubbar (hub) samt kablar som
  kopplar datorerna till switcharna eller hubbarna. Ett ethernet-nätverk på 10
  Mb/s baserat på hubbar torde räcka även för de största resultatsystemen.- Alla konfigurationsändringar görs i de tcp/ip-inställningar
    som hör till nätverkskortet (med undantag för de fall där förbindelsen
    upprättas via telefonlinje, men i dessa fall behöver i allmänhet ingenting
    ändras). Anteckna de tidigare konfigurationsuppgifterna för tcp/ip-nätverket
    innan du ändrar dem- Inaktivera automatisk hämtning av ip-nummer (om den används) och definiera
      datorernas ip-nummer och nätmask så att datorerna får förbindelse
      med varandra. Oftast lönar det sig att bygga nätverket så att alla datorer i det lokala nätverket
      finns i samma subnät. Detta sker till exempel så att
      alla datorers ip-nummer är av typen 192.168.1.xxx och nätmasken är
      255.255.255.0. Om man arbetar i samma lokala nätverk har routerns och namnserverns
      ip-nummer i allmänhet ingen betydelse.

Ingenting hindrar att man använder separata lokala nätverk som kopplats ihop med en router,
om det finns skäl för det. Förbindelsen kan också upprättas via en extern internetförbindelse.
På så sätt är det möjligt att till nätverket koppla till exempel ett tävlingskansli som ligger
avskilt från tävlingscentrum eller en dator som tillhandahåller internettjänster. Ett externt nätverk kan också användas
om man vill koppla datorer till resultattjänstnätverket till exempel via en gprs-telefonförbindelse
eller en modemförbindelse. Användningen av gprs-nätet har dock begränsningar som beror
på tjänsteleverantören och den valda tjänsten. I allmänhet fungerar en UDP-förbindelse endast
för överföring från en gprs-förbindelse till det fasta nätet, om ens det.

- I allmänhet är det enklast att använda ip-nummer för att
  identifiera datorerna, men om man vill kan man också använda namn som nätverket
  känner igen. För att man inte ska vara beroende av namnservern kan
  namnen läggas till i filen hosts (om du inte vet hur detta görs, glöm saken
  och använd ip-nummer).- Eventuella brandväggsprogram inaktiveras (eller
    så definieras i dem det nätverksadressområde som används som lokalt nätverk, där all
    trafik är tillåten).

### A2.3 Specialåtgärder för dataöverföringen

Via huvudmenyns val *Filer /
Inställningar för dataöverföring* kan du utföra följande specialåtgärder

- avbryta och starta om
  dataöverföringen antingen helt eller för utgående eller inkommande meddelanden
  och antingen för alla förbindelser eller för en enskilt vald förbindelse.

  - begära att tidigare sända meddelanden
    sänds på nytt, varvid du i minuter väljer den period som begäran gäller

    - granska innehållet i dataöverföringens säkerhetskopieringsfil
      COMFILE.DAT, som visar vilka slags meddelanden den aktuella
      datorn har sänt till de olika förbindelserna.
