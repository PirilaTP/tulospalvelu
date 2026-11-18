# Bilaga 7. Alge Timy i USB-port

## Bilaga 7. Alge Timy i USB-port

Målklockan Alge Timy kan anslutas till datorn både
via serieport och via USB-port. Användning via serieport är det mest tillförlitliga
alternativet, åtminstone när datorn har en fast serieport, och fungerar bra
även via en USB/RS232-serieportsomvandlare. I alla USB-lösningar har
dock problem observerats. Dessa kan förekomma både när en serieportsomvandlare
används och när Timy ansluts direkt till datorn med en USB-kabel.
På grund av större användarerfarenhet rekommenderar jag att serieportsomvandlare används
vid den egentliga tidtagningen när det är möjligt.

#### Användning via serieportsomvandlare

När en USB-serieportsomvandlare ansluts till datorn
installerar Windows i normala fall automatiskt de nödvändiga
drivrutinerna (driver). Om så inte sker måste programmen i regel installeras
från skivan som följde med enheten. När omvandlaren ansluts till en annan USB-port
måste installationen göras om, vilket återigen i regel sker automatiskt. Vid installationen
bestäms serieportens nummer, som oftast ändras när man byter
till en annan USB-port. I datorns Enhetshanteraren (Device Manager) ser du
vilka serieportar som är installerade. En serieport som installerats via en USB-port får
typiskt ett nummer som är minst 4. När programmet startas måste detta
nummer anges med parametern TIMY, alltså t.ex.

TIMY=4

Användning av serieportsomvandlare kräver inga andra åtgärder.
Eftersom störningar är möjliga lönar det sig att i förväg installera omvandlaren i flera
USB-portar, om sådana finns tillgängliga, och anteckna motsvarande
serieportsnummer. På så sätt kan du efter en störning flytta omvandlaren till en annan
port, ändra numret i programmets startparameter och starta om programmet.

#### Användning direkt via USB-port

Resultatprogrammen själva stöder inte användning av Timy
direkt via USB-port, eftersom det är förknippat med flera problem. I stället
finns ett litet hjälpprogram *AlgeTimyUsbToTcp*, som tar emot
tiderna som kommer från USB-porten och omedelbart skickar dem med TCP-protokollet
till resultatprogrammet. Hjälpprogrammet kan också synkronisera Timys klocka
med datorns tid samt begära att tiderna i Timys minne skickas på nytt.

Användning av programmet *AlgeTimyUsbToTcp*
förutsätter att du först installerar drivrutinerna från Alge Timings webbplats
på datorn. Drivrutinerna måste vara tillräckligt nya. Testningen har gjorts
med version V2.70. Därefter ska programmet *AlgeTimyUsbToTcp*
och dll-filen *AlgeTimyUsb.x86.dll*
eller *AlgeTimyUsb.x64.dll* placeras i samma katalog som programmet, och
en genväg skapas för att öppna programmet *AlgeTimyUsbToTcp.* Av dll-filerna
är den förstnämnda för 32-bitars Windows-versioner och den senare för 64-bitars. Båda
kan kopieras med, eftersom programmet automatiskt väljer den det behöver.

Även när detta förfarande används lönar det sig att
i förväg
ansluta Timy till alla USB-portar som kan komma i fråga och kontrollera att drivrutinerna fungerar
med dem. På så sätt är det lättare att flytta Timy till en annan USB-port under tävlingen.
Detta torde lyckas medan programmen är igång utan att användaren behöver vidta ytterligare åtgärder.

Användning av programmet förutsätter också Microsofts .NET-
programmiljö i version 4.0 eller nyare. Den finns mycket ofta redan
tillgänglig och kan vid behov hämtas från Microsofts webbplats.

När allt detta är klart kan du starta programmet. Om
Timy är ansluten eller ansluts senare till datorn via USB-port, tar
programmet genast kontakt med den (kontakt tas med alla Timy-enheter som är anslutna till datorn,
om det finns flera). Timy skickar en gång i sekunden ett handskakningsmeddelande,
vars visning kan slås på eller av med ett menyval.

Vid start öppnar programmet en TCP-server på port
14701. Servern kan stängas och portnumret ändras medan programmet
är igång. Programmet vidarebefordrar inte handskakningsmeddelanden, utan endast
tidtagningsmeddelanden. Programmet tillåter högst 20 resultatprograms
anslutningar och skickar samma meddelanden till alla anslutna program.
Programmet visar normalt antalet anslutna datorer, men om
Timy inte är ansluten måste antalet frågas med knappen *Test TCP.*

Resultatprogrammet (*HkKisaWin* eller
*HkMaali*) ansluts till programmet *AlgeTimyUsbToTcp*
som körs på samma dator genom att ge det parametern

TIMY=TCP:localhost:14701

Om du vill använda en annan port än 14701 måste detta
nummer ändras. Timy kan också vara ansluten till en annan dator, varvid
ordet localhost måste ersättas med den datorns ip-nummer eller namn.

Om TCP-förbindelsen bryts är programmet *AlgeTimyUsbToTcp* genast redo att godkänna en ny
förbindelse. Resultatprogrammet försöker med några sekunders mellanrum skapa en ny
förbindelse när det upptäckt att förbindelsen har brutits, men i vissa fall
märker programmet inte avbrottet, och då hjälper det att starta om
resultatprogrammet. Om det inte går att öppna port 14701 eftersom den felaktigt
förblivit reserverad, måste porten bytas eller datorn startas om.

Från programmet *AlgeTimyUsbToTcp* kan Timy synkroniseras med datorns
tid genom att först tillåta denna funktion i menyn
och sedan klicka på knappen. Jag rekommenderar att funktionen hålls förbjuden under tidtagningen,
eftersom Timys klocka är noggrann och inte ska
synkroniseras på nytt mitt under tidtagningen. Via menyn kan du också begära att Timys minne töms.

Programmet visar vilken tidtagningsnoggrannhet Timy använder och tillåter att den ändras.

Från programmet kan du också begära att antingen alla
tider i Timys minne skickas på nytt, eller resten från en vald rad framåt. Observera
att radnumret avser hela innehållet i Timys minne, inte de löpnummer som ingår i meddelandena, vilka ofta är
mycket mindre när numreringen har börjat om från början. Det lönar sig
att alltid tömma Timys minne före tävlingen, så att det blir problemfritt att skicka tiderna på nytt. Resultatprogrammet
hoppar över tider som är exakt desamma som tidigare
mottagna, så tider kan utan olägenhet begäras på nytt när de
alla gäller samma evenemang. Tider som ska tas med
hoppas säkert inte över när Timy är inställd att rapportera tiderna med 1/1000
s eller 1/10000 s noggrannhet, eftersom så exakt samma tider
inte kan tas från samma kanal. Resultatprogrammet avrundar tiderna efter behov, så
det är bra att Timy använder någon av de största noggrannheterna (från Timys eget tangentbord
kommer tider avrundade till hundradelar, men via anslutningarna används den fulla valda noggrannheten.)
