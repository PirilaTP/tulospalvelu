# Distribution av uppgifter

Förberedelsen av tävlingen sker typiskt på en
dator. De filer som innehåller de uppgifter som tävlingen behöver är

- KilpSrj.xml skapas när tävlingen skapas. Den innehåller
  tävlingens allmänna inställningar samt uppgifterna om klasserna.

  - KILP.DAT innehåller alla uppgifter om deltagarna

Dessa filer ska vara identiska på alla
datorer som deltar i resultattjänsten, och de behövs i två olika
mappar om två program som hör till resultattjänsten körs på samma dator
eller samma program körs i två exemplar.

Dessutom behövs konfigurationsfilerna enligt föregående kapitel,
som definierar de olika datorernas uppgifter samt förbindelserna
mellan dem. Konfigurationsfilerna är olika på olika datorer.

När programmen används uppstår även andra filer

- COMFILE.DAT skapas för att säkerställa
  dataöverföringen

  - tidtagningsfiler, såsom AJAT1.LST och LAJAT.LST, skapas om
    tidtagningsfunktionerna används

    - loggfiler skapas om sådana har begärts.

När tävlingen börjar måste varje mapp som används
innehålla identiska kopior av filerna KilpSrj.xml och KILP.DAT
. Filen COMFILE.DAT måste raderas när filen
KILP.DAT distribueras till olika datorer, eftersom COMFILE.DAT
då kan innehålla oskickade meddelanden som orsakar felsituationer när programmen används.

Ett bra förfarande är att göra kommandofiler som först raderar potentiellt problematiska filer och sedan kopierar
de nya filerna KilpSrj.xml och KILP.DAT till de mappar som används
via nätverket. Detta förutsätter att förbindelserna mellan datorerna som dataöverföringen använder har tillåtits genom att de aktuella sökvägarna definierats
som delade i nätverket. Kommandofilens innehåll kan till exempel vara

```
del \\192.168.1.13\Kisat\kisa1\*.dat
del \\192.168.1.13\Kisat\kisa1\*.lst
copy c:\kisat\kisa1\KilpSrj.xml \\192.168.1.13\Kisat\kisa1\
copy c:\kisat\kisa1\KILP.DAT \\192.168.1.13\Kisat\kisa1\
del \\192.168.1.12\Kisat\kisa1\*.dat
copy c:\kisat\kisa1\KilpSrj.xml \\192.168.1.12\Kisat\kisa1\
copy c:\kisat\kisa1\KILP.DAT \\192.168.1.12\Kisat\kisa1\
```

Här har det antagits att stämplingskontrollens datorer
192.168.1.13 (måldatorn) och 192.168.1.12 (speakerns dator) har delat mappen
C:\kisat i nätverket under namnet Kisat med tillåtelse att skriva i mappen och dess undermappar,
och att tävlingens uppgifter finns i undermappen kisa1 till denna mapp både på den dator
som distribuerar uppgifterna och på den som tar emot dem. Likaså har det antagits att ingen fil med ändelsen \*.dat eventuellt skulle behöva bevaras.

De ovan uppräknade
kommandona lämpar sig för användning från kontorets dator. Alternativt kan motsvarande
kommandon genomföras så att endast kontorets dator delar mappen Kisat och de andra datorerna
hämtar uppgifterna med ett motsvarande kommando samtidigt som datorerna startas för tävlingen.

Separat måste du se till att de nödvändiga
konfigurationsfilerna finns tillgängliga och att startinställningarna samt de fönster som ska öppnas
vid behov har sparats från programmet HkKisaWin före tävlingens början.
