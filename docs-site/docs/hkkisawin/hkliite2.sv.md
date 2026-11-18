# Bilaga 2. Ibruktagning av dataöverföring

## Bilaga 2. Ibruktagning av dataöverföring

### A2.1 Intern dataöverföring i resultattjänstnätverket via TCP/IP-nätverk

För förbindelser mellan datorer som använder resultatprogrammen
används i regel UDP-protokollet. TCP-protokollet
kan användas när UDP
inte fungerar, till exempel via mobilförbindelse. Även dataöverföring via serieportar
och nollmodemkablar fungerar fortfarande i programmen. TCP-protokollet kan också användas
för att skicka uppgifter som XML-poster till en extern server.

En UDP-förbindelse förutsätter att
båda datorerna som använder förbindelsen reserverar var sin UDP-port för att ta
emot meddelanden från motparten. För denna port är standardvärdet 15900 + n,
där n är förbindelsens nummer, så portnumret är känt när förbindelsens nummer
är känt. Det värde som används vid beräkningen av portnumret kan ändras med parametern

PORTBASE=pohja-arvo

vars motsvarande standard alltså är PORTBASE=15900
Portnumret kan också ändras per förbindelse, som beskrivs nedan. Om
inte heller förbindelsens nummer har angetts antar programmet att det gäller
förbindelse 1 och därmed port 15901. Förbindelsens nummer är datorspecifikt. När UDP-förbindelser används
är det vanligt att endast en dator, "servern", använder flera förbindelser; alla
andra datorer kan då använda endast en förbindelse. Om även en reservserver används
måste de övriga datorerna också använda två förbindelser, till exempel så att förbindelse 1 används
till servern och förbindelse 2 till reservservern.

I regel är det mest praktiskt att definiera förbindelsen så
att ip-adressen och motpartens port definieras endast i den ena änden av förbindelsen.
Då skickar denna part sin egen ip-adress och port
till motparten, varefter förbindelsen fungerar i båda riktningarna. Inte heller
motpartens port behöver definieras separat om det gäller motpartens förbindelse
1, eftersom denna förbindelses port 15901 är standard. (Parametern PORTBASE
påverkar också detta nummervärde.)

UDP-dataöverföring mellan två datorer startas
genom att på båda datorerna ange parametern YHTEYS
i fullständig form:

`YHTEYSn=UDP:omaportti/vastaip:vastaportti`

eller i enklare former, där åtminstone en del av adressparametrarna utelämnas,
varav exempel följer här

`YHTEYSn=UDP
YHTEYSn=UDP:omaportti
YHTEYSn=UDP:0/vastaip`

Om uppgiften omaportti (egen port) saknas helt, är 0 eller består av bokstäver använder programmet
värdet 15900 + n för den egna porten, där n är förbindelsens nummer. Om vastaip eller vastaportti
ska anges måste 0 eller någon bokstav anges i fältet omaportti, dvs.
tecknen : och / kan inte följa direkt efter varandra.

Om parametern vastaip (motpartens ip) saknas eller dess värde är AUTO väntar programmet på den egna porten
på ett meddelande från motparten, av vilket det får reda på både motpartens ip-adress och motpartens port.
Lite mer information om detta finns i slutet av detta kapitel. När motpartens ip-adress inte anges
behöver alltså inte heller vastaportti (motpartens port) anges, eftersom programmet får även denna uppgift från det mottagna meddelandet.

För uppgiften vastaportti är standardvärdet 15901, dvs. standard är att det gäller motpartens förbindelse 1,
för vilken ingen avvikande port har definierats. Om man i fältet vastaportti skriver ordet
yhteys och ett nummer n väljer programmet 15900 + n som motpartens port, dvs. antar att
det gäller standardporten för motpartens förbindelse n. (Här kan man i stället för ordet yhteys använda
enbart bokstaven Y.)

Direkt efter ordet UDP kan bokstaven i eller o stå för att ange att uppgifter
endast tas emot (incoming) eller endast skickas (outgoing).

Exempel som förtydligar saken:

Exempel 1: Förbindelser som använder standardportar

I detta exempel kopplas en "server" till tre andra datorer. För servern
definieras dataöverföringarna på följande sätt

`yhteys1=udp:0/192.168.1.12
yhteys2=udp:0/192.168.1.13
yhteys3=udpo:0/192.168.1.14:yhteys2`

I den andra änden av den första förbindelsen är parametern

`yhteys1=udp`

Motpartens parameter för den andra förbindelsen är

`yhteys1=udp`

Motpartens parameter för den tredje förbindelsen anger att det gäller enkelriktade
inkommande data och att det är denna dators förbindelse 2:

`yhteys2=udpi`

Varje förbindelse förutsätter att motpartens ip-adress har angetts för någondera
änden av förbindelsen och att denna ände också känner till motpartens värde för omaportti korrekt.
Denna port behöver dock inte skrivas som parameter om det gäller motpartens förbindelse 1.

Exempel 2: Förbindelse där alla värden har definierats i parametrarna:

UDP-protokollet fås för dataöverföringen genom att ange yhteys-parametern på den dator
vars ip-nummer är till exempel 192.168.1.11 i formen

`YHTEYS1=UDP:1250/192.168.1.12:1350`

när ip-numret för datorn i andra änden är 192.168.1.12. Där i andra änden måste i sin tur parametern

`YHTEYS1=UDP:1350/192.168.1.11:1250`

anges. Det första nummervärdet är den port på vilken mottagning av UDP-meddelanden öppnas
på datorn i fråga, och efter snedstrecket kommer den andra datorns ip-nummer och UDP-port.
Varje förbindelse använder alltså UDP-mottagning i båda ändarna. Meddelandena skickas
alltid till den mottagande porten, som lyssnar kontinuerligt och svarar till den sändande porten,
som systemet automatiskt tilldelar separat för varje meddelande. Varje
dator kan ha förbindelse med mycket många andra datorer och för varje förbindelse
måste alltså en egen mottagande port reserveras.

De portar som är reserverade på datorn kan visas med kommandot

`NETSTAT /A`

och de UDP-portar som det visar ska inte användas. Portnumren som användes ovan,
1250 och 1350, var bara två nummer som jag valde slumpmässigt. Vilka
lediga portar som helst fungerar lika bra. Som sagts ovan är standardvärdet
portar från och med nummer 15901.

I stället för numerisk form kan man som parameter vastaip också använda en textadress
för måldatorn. För att man i tävlingssituationen inte ska vara beroende av att namnservern
fungerar rekommenderar jag att alla textnamn som används definieras
i filen hosts, som typiskt finns i katalogen \WINNT\system32\drivers\etc.

När man vill använda flera kopior av resultatprogrammen på samma dator
kan man som parameter vastaip använda adressen localhost, som alltid avser
ip-numret 127.0.0.1. Adressen localhost kan användas även om datorn inte
har något nätverkskort alls, förutsatt att tcp/ip-funktionerna är installerade.
Vid förbindelser inom datorn kan portarnas standardvärden inte användas, om inte olika
förbindelsenummer definieras för olika program, eftersom samma port inte kan reserveras
för mer än ett program.

Som parameter vastaip kan man i den ena änden av förbindelsen också ange ordet AUTO.
När man gör så väntar datorn i fråga på ett UDP-meddelande till porten omaportti och
använder därefter ip-numret för avsändaren av det mottagna meddelandet. Detta ip-nummer
kan dessutom ändras medan programmet körs, eftersom programmet alltid använder ip-numret för avsändaren
av det senaste UDP-meddelande som tagits emot på porten omaportti. Detta
förfarande är användbart både för att förenkla parametrarna och i situationer
där ip-numret för förbindelsens andra part inte är känt i förväg eller till och med kan variera.
En sådan situation uppstår främst när förbindelsen tas via internet
med hjälp av en offentlig tjänsteleverantör. I den andra änden måste motpartens
ip-nummer dock alltid anges för att förbindelsen ska kunna upprättas.

### A2.2 Konfiguration av lokalt nätverk för UDP-dataöverföring

Programmets funktion förutsätter endast att en förbindelse mellan datorerna uppstår, så
ingenting som avviker från en vanlig konfiguration av ett TCP/IP-baserat lokalt nätverk
behövs. Om de datorer som används inte annars finns i samma lokala nätverk måste
typiskt följande åtgärder vidtas:

- Bygg ett fysiskt nätverk som består av en eller
  flera switchar (switch) eller hubbar (hub) samt kablar som
  ansluter datorerna till switcharna eller hubbarna. Ett hubbaserat 10
  Mb/s ethernet-nätverk torde räcka även för de största resultattjänstsystemen.- Alla konfigurationsändringar görs i nätverkskortets
    tcp/ip-inställningar (med undantag för de fall där förbindelsen
    tas via telefonlinje, men i dessa fall behöver man i regel inte ändra
    något). Anteckna de tidigare konfigurationsuppgifterna för tcp/ip-nätverket
    innan du ändrar dem- Inaktivera automatisk hämtning av ip-nummer (om den används) och definiera
      datorernas ip-nummer och nätmask så att datorerna får förbindelse
      med varandra. Oftast lönar det sig att bygga nätverket så att alla datorer i det lokala
      nätverket finns i samma subnät. Detta görs till exempel så att
      alla datorers ip-nummer är av typen 192.168.1.xxx och nätmasken är
      255.255.255.0. Om man arbetar i samma lokala nätverk har routerns och namnserverns
      ip-nummer i regel ingen betydelse.

Ingenting hindrar att man använder separata lokala nätverk som kopplats samman med en router,
om det finns något skäl till det. Förbindelsen kan också upprättas via en extern internetförbindelse.
På så sätt är det möjligt att koppla till nätverket till exempel ett tävlingskansli som ligger
avskilt från tävlingscentrum, eller en dator som tillhandahåller internettjänster. Ett externt nätverk kan också användas
om man vill ansluta datorer till resultattjänstnätverket till exempel via gprs-telefonförbindelse
eller modemförbindelse. Användningen av gprs-nätet har dock begränsningar som beror på
tjänsteleverantören och den valda tjänsten. I regel fungerar en UDP-förbindelse endast
för överföring från gprs-förbindelse till fast nät, om ens då.

- I regel är det enklast att använda ip-nummer för att
  identifiera datorerna, men om man vill kan man också använda namn som
  nätverket känner igen. För att inte vara beroende av namnservern kan
  namnen läggas till i filen hosts (om du inte vet hur detta görs, glöm saken
  och använd ip-nummer).- Eventuella brandväggsprogram inaktiveras (eller
    så definieras i dem det nätverksadressområde som används som lokalt nätverk, där all
    trafik är tillåten).

### A2.3 Specialåtgärder för dataöverföring

Via huvudmenyns val *Filer /
Inställningar för dataöverföring* kan du göra följande specialåtgärder

- **avbryt och starta om
  dataöverföringen**
  antingen helt eller endast för utgående eller inkommande meddelanden,
  och antingen för alla förbindelser eller för en förbindelse som väljs
  separat.

  - **begär omsändning av tidigare
    skickade meddelanden**
    genom att i minuter välja den period som begäran gäller

    - **granska innehållet i dataöverföringens
      säkerhetsfil COMFILE.DAT**,
      av vilket framgår vilka meddelanden den aktuella
      datorn har skickat till olika förbindelser.

      - **skicka tävlandeuppgifter som XML-poster med TCP-protokollet** till en extern
        tjänst som använder sådana uppgifter. Detta sker till en förbindelse vars
        typ har definierats som TCPX. Programmet skickar till en sådan förbindelse också
        varje ny resultatuppgift som ett eget meddelande.- **skicka filer** via resultattjänstnätverkets UDP-förbindelser

Filöverföring är tillgänglig endast
när programmet har fått parametern

LÄHTIEDOSTOT

eller

LÄHTIEDOSTOTx

där x är numret på en förbindelse. Den förra formen aktiverar filöverföring i alla
UDP-förbindelser, den senare endast i en förbindelse.

Parametern

LÄHTIEDOSTOTx=0

kan anges efter parametern som gäller alla förbindelser
för att ta bort filöverföringen till förbindelse x.

Till slutet av parametern kan preciseringen /K
läggas till.

Om tilläggspreciseringen /K
inte har angetts överförs filen alltid antingen från programmets
tävlingsdatamapp eller från en
undermapp till den till motsvarande mapp på den mottagande
datorn. Den mottagande undermappen måste finnas i förväg;
programmet skapar ingen ny mapp.

När tilläggspreciseringen /K
 har angetts
använder programmet filens
fullständiga katalogsökväg som sådan.
Då förutsätter överföringen att den mottagande datorn har en likadant
namngiven katalog under samma enhetsbeteckning.

Filöverföring används också när
banuppgifter sparas i resultattjänstläget i det formulär som hanterar dem. Då frågar programmet
om uppgifterna ska skickas via resultattjänstnätverket.
