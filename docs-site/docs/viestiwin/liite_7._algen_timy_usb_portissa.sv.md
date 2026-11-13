# Bilaga 3. Alge Timy i USB-port

### Bilaga 3. Alge Timy i USB-port

Målklockan Alge Timy kan anslutas till datorn både
via serieport och via USB-port. Användning via serieport är det mest tillförlitliga
alternativet, åtminstone när datorn har en fast serieport, och fungerar också bra
via en USB/RS232-serieportsomvandlare. I alla USB-lösningar har
dock problem observerats. Dessa kan förekomma både när en serieportsomvandlare
används och när Timy ansluts direkt till datorn med en USB-kabel.
På grund av den större användarerfarenheten rekommenderar jag att en serieportsomvandlare används
vid den egentliga tidtagningen, när det är möjligt.

#### Användning via serieportsomvandlare

När en USB-serieportsomvandlare ansluts till datorn installerar
Windows i normalfallet automatiskt de drivrutiner (driver)
som behövs. Om detta inte sker måste programmen i allmänhet installeras
från den skiva som följde med enheten. När omvandlaren ansluts till en annan USB-port
måste installationen göras om, vilket i allmänhet åter sker automatiskt. Vid installationen
bestäms serieportens nummer, som oftast ändras när man byter
till en annan USB-port. I datorns enhetshanterare (Device Manager) framgår
vilka serieportar som är installerade. En serieport som installerats via en USB-port får
typiskt ett nummer som är minst 4. När programmet startas måste detta
nummer anges med parametern TIMY, alltså t.ex.

TIMY=4

Användning av en serieportsomvandlare kräver inga andra åtgärder.
Eftersom störningar är möjliga lönar det sig att i förväg installera omvandlaren i flera
USB-portar, om sådana finns tillgängliga, och anteckna motsvarande
serieportsnummer. På så sätt kan du efter en störning flytta omvandlaren till en annan
port, ändra numret i programmets startparameter och starta om programmet.

#### Användning direkt via USB-port

Resultatprogrammen själva stöder inte användning av Timy
direkt via USB-port, eftersom detta är förknippat med flera problem. I stället
finns ett litet hjälpprogram *AlgeTimyUsbToTcp*, som tar emot
de tider som kommer från USB-porten och omedelbart sänder dem med TCP-protokollet
till resultatprogrammet. Hjälpprogrammet kan också synkronisera Timys klocka
med datorns tid samt begära att tiderna i Timys minne sänds på nytt.

Användningen av programmet *AlgeTimyUsbToTcp*
förutsätter att du först installerar de drivrutiner som finns på Alge Timings webbplats
på datorn. Drivrutinerna måste vara tillräckligt nya. Testningen har gjorts
med version V2.70. Därefter ska programmet *AlgeTimyUsbToTcp*
samt dll-filen *AlgeTimyUsb.x86.dll*
eller *AlgeTimyUsb.x64.dll* placeras i samma katalog som programmet och en
genväg skapas för att öppna programmet *AlgeTimyUsbToTcp.* Av dll-filerna
är den förstnämnda för 32-bitars Windows-versioner och den senare för 64-bitars. Båda
kan kopieras med, eftersom programmet automatiskt väljer den det behöver.

Även när detta förfarande används lönar det sig att i förväg ansluta
Timy till alla USB-portar som kan komma i fråga och säkerställa att drivrutinerna fungerar
med dem. På så sätt är det lättare att flytta Timy till en annan USB-port under tävlingen.
Detta torde lyckas medan programmen körs utan att användaren behöver vidta ytterligare åtgärder.

Användningen av programmet förutsätter också Microsofts .NET-programmiljö
i version 4.0 eller nyare. Den finns mycket ofta redan
tillgänglig och kan vid behov hämtas från Microsofts webbplats.

När allt detta är klart kan du starta programmet. Om
Timy är ansluten eller senare ansluts till datorn via USB-porten, upprättar
programmet genast förbindelse med den (förbindelse upprättas med alla Timy-enheter som är anslutna
till datorn, om det finns flera). Timy sänder en gång i sekunden ett handskakningsmeddelande,
vars visning kan tas med eller förhindras med ett menyval.

När programmet startar öppnar det en TCP-server i port
14701. Servern kan stängas och portnumret ändras medan programmet
körs. Programmet vidarebefordrar inte handskakningsmeddelandena, utan endast
tidtagningsmeddelandena. Programmet tillåter högst 20 resultatprogram
att ansluta och sänder samma meddelanden till alla program som har anslutit.
Programmet visar normalt antalet datorer som har anslutit, men om
Timy inte är ansluten måste antalet efterfrågas med knappen *Test TCP*.

Resultatprogrammet (*HkKisaWin* eller
*HkMaali*) kopplas till programmet
*AlgeTimyUsbToTcp* som körs på samma dator genom att ge det parametern

TIMY=TCP:localhost:14701

Om du vill använda en annan port än 14701 måste detta
nummer ändras. Timy kan också vara ansluten till en annan dator, varvid
ordet localhost ska ersättas med den datorns ip-nummer eller namn.

Om TCP-förbindelsen bryts är programmet *AlgeTimyUsbToTcp* genast redo att godkänna en ny
förbindelse. Resultatprogrammet försöker med några sekunders mellanrum upprätta en ny
förbindelse när det har upptäckt att förbindelsen har brutits, men i vissa fall
märker programmet inte avbrottet, och då hjälper det att starta om
resultatprogrammet. Om det inte går att öppna port 14701 eftersom den felaktigt
har förblivit reserverad, måste porten bytas eller datorn startas om.

Från programmet *AlgeTimyUsbToTcp* kan Timy synkroniseras med datorns
tid genom att först tillåta denna funktion i menyn
och sedan klicka på knappen. Jag rekommenderar att funktionen hålls förbjuden under tidtagningen,
eftersom Timys klocka är noggrann och inte ska
synkroniseras på nytt mitt under tidtagningen. Via menyn kan du också begära att Timys minne töms.

Programmet anger den tidtagningsnoggrannhet som Timy använder och tillåter att den ändras.

Från programmet kan du också begära att antingen alla tider i Timys minne sänds på nytt
eller den senare delen från och med en vald rad. Observera
att radnumret hänvisar till hela innehållet i Timys minne, inte till de ordningsnummer som ingår i meddelandena, vilka ofta är
mycket mindre när numreringen har börjat om från början. Det lönar sig att
alltid tömma Timys minne före tävlingen, så att det inte blir problem med att sända på nytt. Resultatprogrammet
hoppar över tider som är exakt desamma som tidigare
mottagna, så tider kan utan olägenhet begäras på nytt när de
alla gäller evenemanget i fråga. Tider som man vill ha med
hoppas säkert inte över när Timy är inställd att ange tiderna med 1/1000
s eller 1/10000 s noggrannhet, eftersom så exakt samma tider
inte kan tas från samma kanal. Resultatprogrammet avrundar tiderna vid behov, så
det är bra att Timy använder någon av de största noggrannheterna (från Timys eget tangentbord
kommer tider som avrundats till hundradelar, men vid användning via gränssnitten den fulla valda noggrannheten.)
