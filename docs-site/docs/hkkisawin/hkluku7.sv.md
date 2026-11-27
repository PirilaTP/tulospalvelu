# Kapitel 7. Granskning och redigering av deltagaruppgifter

## 7 Granskning och redigering av deltagaruppgifter

### 7.1 Allmänt

Deltagaruppgifterna kan granskas och redigeras både i tabellen *Deltagare* och i formuläret som visar
uppgifterna för en deltagare. Dessutom redigeras uppgifter i
inmatningsformuläret för anmälningar, som behandlas i [kapitel 3.1](3.1_ilmoittautumisten_syotto.md)
. Deltagartabellen lämpar sig särskilt för
att granska flera deltagare och göra ändringar som gäller dem, medan granskning och redigering av uppgifterna för en enskild deltagare görs tydligast
i formuläret *Tävlandeuppgifter*, som öppnas genom att välja *En deltagare* i menyn eller genom att klicka på en rad i deltagartabellen.

Menyvalet *Deltagare / Visa tävlande i terrängen* öppnar
en tabell över de tävlande som ännu inte har någon måltid och som inte har
markerats som frånvarande, ej startat, vakans, utgått eller
diskvalificerad. Tabellen visar startplatsen (klassens startnummer och banans
start), starttiden och alla mellantidspunkter som används. Klassen kan
väljas i listan överst; knappen *Hämta* uppdaterar listan.

I menyn *Fält* i tabellen *Deltagare* väljer du vilka kolumner
som visas i tabellen. För etappspecifika uppgifter (t.ex. *Starttid*,
*Resultat*, *Bana*) visas i en tävling med flera etapper en egen kolumn för varje
etapp när *Alla etapper* är valt.

Valen *Rekord* och *Målsättning* tar med den tävlandes rekord- och
måltid i tabellen. Dessa visas annars bara en tävlande i taget i formuläret
*Tävlandeuppgifter* (menyvalen *Visa eget rekord* och *Visa
riktresultat*). På så sätt kan du granska och jämföra rekord och mål för hela
deltagarskaran på en gång, till exempel inför en handikapp- eller
måltidstävling. Kolumnerna är dolda som standard.

- Tiden visas i formatet tt.mm.ss i hela sekunder, på samma sätt som
  i formuläret *Tävlandeuppgifter*. En tom cell betyder att ingen tid har
  angetts.
- När redigering är tillåten (*F9* eller *Redigering tillåten*) kan du
  ändra värdena direkt i tabellen. Tiden skrivs på samma sätt som vid
  inläsning från fil, till exempel `45.30` eller `45:30` (45 min 30 s) och `1.02.30`
  eller `1:02:30`. Programmet formaterar tiden när du lämnar cellen.
  Om cellen töms tas tiden bort.
- Ändringarna sparas tillsammans med övriga ändringar som gjorts i tabellen.
  Om den sparade tiden innehåller delar av sekunder behålls de, om inte cellen
  ändras.
- Tabellen kan sorteras efter rekord eller målsättning genom att välja
  *Rekord* eller *Målsättning* som ordning eller genom att klicka på kolumnens
  rubrik. Den snabbaste tiden kommer först och tävlande utan tid
  sist. Lika tider ordnas efter namn.

Rekord- och måltider kan också läsas från fil med fälten
`Ennätys-1`, `Tavoite-1` osv. (se [kapitel 3.2](3.2_ilmoittautumisten_lukeminen_tiedostosta.md)).

Både i tabellen och i formuläret kräver ändringar att de tillåts med knappen *Tillåt ändringar*. Mellantider visas inte
i detta formulär, utan de granskas och ändras i formuläret som öppnas med knappen *Online mellantider*. När uppgifter som påverkar resultatet
ändras i formuläret *Tävlandeuppgifter* ändras antingen måltiden eller
resultatet i motsvarande grad. Således påverkar starttid, måltid och straff
resultatet, och resultatet i sin tur måltiden. Placering och totalresultat bestäms
på nytt först när ändringarna sparas.

I formuläret *Tävlandeuppgifter* kan du välja att visa klubbregistret. När klubbregistret har valts att visas och registerfilen har öppnats
söker programmet automatiskt klubben i registret när klubbens namn eller förkortning redigeras, beroende på vilken av dem som har angetts som primär för tävlingen.
När rätt klubb är vald leder ett tryck på *Enter* till att klubbens uppgifter (långt namn, förkortning och distriktskod) förs över
till tävlandeformuläret. På så sätt kan du säkerställa att klubbens namn alltid registreras exakt likadant och hålla långa klubbnamn uppdaterade när man inom orientering
hanterar förkortningar.

Den tävlande som ska granskas kan väljas när formuläret
*Tävlandeuppgifter* är i sök- och visningsläge genom att skriva
den tävlandes nummer i motsvarande fält, genom att välja klass och
bläddra eller genom att använda namnsökning, som också kan kompletteras med bläddring.

När formuläret är i redigeringsläge är sökfunktionerna inte
tillgängliga. Knappen *Ångra ändringar* förkastar ändringarna och återgår till
sök- och visningsläget. Knappen *Spara* sparar uppgifterna och återgår till
sök- och visningsläget om sparandet lyckas. Om sparandet misslyckas
(till exempel upptaget nummer) förblir formuläret i redigeringsläge så att uppgifterna
kan rättas och sparas på nytt direkt.

Om formuläret stängs medan redigeringsläget är på (knappen *Stäng* eller
genom att stänga fönstret) frågar programmet alltid *Sparas ändringarna?*.
*Ja* sparar uppgifterna om de har ändrats eller om det gäller en ny
tävlande. Om sparandet misslyckas (till exempel upptaget nummer eller
uppgifter som ändrats på annat håll) förblir formuläret öppet i redigeringsläge. *Nej*
förkastar ändringarna; tillägget av en ny tävlande avbryts. Efter ett lyckat
sparande eller förkastande avslutas redigeringsläget, och formuläret
öppnas nästa gång i sök- och visningsläge.

För att underlätta ändringar kan du använda
tangenten *F9* både för att gå till redigeringsläge och för att spara
gjorda ändringar.

I formulärets meny kan du också välja ett läge som effektiviserar
etappspecifika ändringar. När detta läge är valt flyttar
programmet vid tryck på *F9* till tabellen med etappuppgifter,
till början av raden för den pågående etappen
