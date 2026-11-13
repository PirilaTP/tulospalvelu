# Nätverksinställningar

Datorerna som används i resultattjänsten är anslutna till varandra via ett lokalt
nätverk. Varje förbindelse går mellan två datorer, och förbindelsen öppnas
så att den ena datorn väntar på en anslutningsbegäran som den andra gör.
För att detta ska vara möjligt måste den dator som gör begäran känna till motpartens
identifierare, som i praktiken oftast är ett ip-nummer men också kan vara ett namn som
kan kopplas till ett visst ip-nummer. Det protokoll som normalt används
är UDP, vars användning förutsätter att man utöver ip-numret känner till ytterligare ett
numeriskt värde, som kallas port.

Innan du kan börja definiera de parametrar som förbindelsen
kräver måste du ta reda på vilka ip-nummer som används, eller åtminstone så
många av dem att den andra partens ip-nummer, eller ett namn utifrån vilket programmet kan
ta reda på ip-numret, är känt för varje förbindelse mellan två datorer. Detta är enkelt när TCP/IP-nätverket har definierats med fasta
ip-nummer, men mer problematiskt när så inte har gjorts. När resultatprogrammet används
i ett lokalt nätverk är det bäst att använda fasta ip-nummer för alla datorer. I vissa fall behövs
en förbindelse till exempel via mobilnätet, och då är bara den ena
datorns ip-nummer känt, men det räcker för att upprätta förbindelsen.

Kommandot

`ipconfig`

i Windows kommandofönster ger ett svar som
innehåller datorns ip-nummer på en rad som till exempel kan vara

`IPv4 Address . . . . : 192.168.1.10`

men detta nummer kan vara ett annat efter nästa
start, om numren inte har definierats som fasta.

UDP-portnumren kan ligga mellan 1 och 65535, och av dessa
är de flesta värden över 1024 i allmänhet lediga (undantag är oftast 8080 och 8008,
men även vissa andra nummer kan vara upptagna. Portar som används kan
visas med kommandot `netstat /a`, men även andra
portar kan vara upptagna vid någon annan tidpunkt). Programmen använder
som standard värden i området 15901–15964, vilka i praktiken inte har orsakat
problem. Programmens förbindelser är numrerade, och det högsta tillgängliga
numret är normalt 64. Standardportnumret för förbindelse N är
15900+N.

Du måste säkerställa att brandväggsprogram inte hindrar
dataöverföringen. Detta förutsätter typiskt att programmen HkMaali.exe och
HkKisaWin.exe tillåts använda alla förbindelser även som server. Om nätverket
inte är anslutet till internet kan alla brandväggsprogram vid behov stängas av
under
tävlingen.
