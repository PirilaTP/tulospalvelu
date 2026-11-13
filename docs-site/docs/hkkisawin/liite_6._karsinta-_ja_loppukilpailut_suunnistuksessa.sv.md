# Bilaga 6. Kval- och finaltävlingar i orientering

## Bilaga 6. Kval- och finaltävlingar i orientering

I orienteringstävlingar som omfattar kval och
final finns typiskt kvalklasser som bara deltar i den första etappen
samt finaler som bara avgörs i den andra etappen. Eftersom de tävlande
tävlar i olika klasser i olika etapper måste
man i tävlingsinställningarna välja att klassen ändras från en etapp till nästa, och
oftast också att nummerlappen byts från en etapp till nästa.

I vissa fall avgörs en inledande tävling som bestämmer startordningen,
utan kval, i den första etappen och
därefter en final i samma klass i den andra etappen, varvid klassen är
med i båda etapperna. De anmälda registreras typiskt först
i den klass som också är A-finalen, och flyttas därifrån till kvalklasserna.
När anmälningarna registreras är det bättre att de klasser som används för detta är
definierade att omfatta alla etapper, men efter att flyttningen
till kvalklasserna har gjorts kan etapperna markeras så att de motsvarar de faktiska
tävlingsetapperna.

Efter att anmälningarna har sparats måste man typiskt
föra in rankingpoäng i deltagaruppgifterna för att kunna bestämma
startordningen utifrån rankingen. Detta görs från en csv-fil
som innehåller rankingpoängen, med programmets funktion *Deltagare / Flytta tilläggsdata*,
med licensnumret som det fält som identifierar den tävlande.

Flyttningen till kvalklasserna görs normalt i funktionen
*Förberedelse / Lottning och numrering* på fliken *Delning av klasser*.
Indelningsgrunden är oftast
*Utjämnad på basis av ranking* . När
indelningen är gjord kan startordningen bestämmas på fliken *Lottning* och
slutligen numreringen på fliken *Tilldelning av nummer*. Indelningen i kvalklasser
lönar sig ofta att rikta enbart till etapp 1, varvid den tävlandes
grundklass förblir den klass där anmälningarna registrerades. Då måste man
dock se till att de tävlande för den andra etappens del finns i någon
klass dit deltagare inte plockas utifrån kvaltävlingen.
En sådan klass kan vara antingen en av kvalklasserna eller,
i det fall som behandlas nedan, B-finalen. Flyttning till en
kvalklass fungerar även om klassen inte deltar i etapp 2. I vissa
fall är det skäl att definiera en extra klass för detta ändamål för att
undvika problem i online-resultattjänsten. Mer om dessa val
nedan.

I en del tävlingar ordnas B-finaler med
i förväg lottade starttider. I detta fall måste alla tävlande
markeras i respektive B-final före lottningen. Markeringen görs
i funktionen *Deltagare / Kopiering av klubbnamn och klasser*
genom att där välja *Flytta tävlande från klass till klass*. Därefter
väljer du *Grundklass* som uppgift som ska kopieras och *Etapp 2 klass* som mål.
I valen för klassbyte anger du sedan dessa
tävlandes grundklass och som ny klass B-finalens klass (i stället för grundklassen
kan det ibland vara bättre att använda etapp 1-klassen för identifieringen).

Förfarandet i föregående stycke passar också
för fall där B-finalen bildas tillsammans med A-finalen, så att
de tävlande flyttas till någon klass som inte deltar i etapp
2. Om B-finalen i den andra etappen delas upp i parallella
klasser efter kvalet är det bättre att definiera en extra arbetsklass enbart
för detta ändamål, så att det efter att B-finalerna har bildats inte
finns några deltagare kvar i denna arbetsklass för någon etapp.

Efter kvalet bildas finalerna med det
förfarande som beskrivs i kapitel 10.4. På så sätt
bildas på en gång alla de finalnivåer som bestäms av placeringarna
i kvalet. Om alla som hamnar utanför A-finalen deltar i en gemensam B-finalklass
utan att deras startordning bestäms av kvalresultaten, kan
de placeras där redan i förväg. Om startordningen
bestäms av kvalet placeras de i förväg i en annan klass och
flyttas till B-finalen samtidigt som A-finalen bildas. Om B-finalen delas upp i parallella
klasser måste de tävlande placeras i någon annan klass, helst
en extra hjälpklass som markerats att delta i etapp 2,
för att indelningen i B-finaler ska lyckas.

Alla åtgärder som hör till bildandet av finalerna kan
göras i kvaletappens resultattjänstläge. Formuläret *Skapa
final* kan användas endast
i detta läge. De steg som görs i formuläret *Lottning och numrering* kan
göras både i etapp 1:s resultattjänstläge och i förberedelseläget. Om
man arbetar i resultattjänstläget fungerar också dataöverföringen i resultattjänstnätverket och
filerna på alla datorer i nätverket uppdateras samtidigt i den mån
förbindelser används. Om tidtabellen inte är snäv kan det ändå vara bättre
att göra åtgärderna separat på en dator och till sist kopiera filerna till alla datorer som används i etapp
2, eftersom man på så sätt får den bästa säkerheten för att
filerna på alla datorer verkligen är desamma när etapp 2 börjar.

Eftersom bildandet av finaler är en relativt
komplicerad funktion är det alltid skäl att göra en kopia av tävlandefilen innan
arbetet påbörjas. Kopior kan också göras i olika mellanskeden med programmets val
