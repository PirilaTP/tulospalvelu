# Distribution av uppgifter

Förberedelsen av tävlingen sker typiskt på en
dator. De filer som innehåller de uppgifter som tävlingen behöver är

- KilpSrj.xml skapas när tävlingen skapas. Den innehåller
  tävlingens allmänna inställningar samt uppgifterna om klasserna.

  - KILP.DAT innehåller alla uppgifter om deltagarna

    - radat1.xml innehåller banornas
      beskrivningar och kontrollernas stämplingskoder (siffran 1 i namnet avser den första eller
      enda etappen, den andra etappens fil är rada2.xml )
    - som ett alternativ som håller på att fasas ut är RADAT.LST ett tidigare
      använt filformat som innehåller banornas beskrivningar

      - som ett alternativ som håller på att fasas ut är LEIMAT.LST ett tidigare
        använt filformat som innehåller en förteckning över koderna för stämplingsenheterna
        vid de olika kontrollerna- EMIT.DAT innehåller nödvändiga uppgifter först när
      Emit-brickor läses i stämplingskontrollen

Dessa filer ska vara identiska på alla
datorer som deltar i resultattjänsten, och de behövs i två olika
mappar om två program som hör till resultattjänsten körs på samma dator
eller samma program körs i två exemplar.

Dessutom behövs konfigurationsfiler, som
definierar de olika datorernas uppgifter samt förbindelserna mellan dem.
Konfigurationsfilerna är i allmänhet olika på olika datorer.

När programmen används uppstår även andra filer

- COMFILE.DAT skapas för att säkerställa
  dataöverföringen (i en tävling med flera etapper COMFILE1.DAT, COMFILE2.DAT
  osv.)

  - tidtagningsfiler, såsom AJAT.LST, skapas om tidtagningsfunktionerna används
    (även LAJAT.LST, AJAT1.LST, AJAT.LS2
    m.fl.)

    - loggfiler skapas om sådana har begärts.

När tävlingen börjar måste varje mapp som används
innehålla identiska kopior av filerna KilpSrj.xml och KILP.DAT. Likaså måste banfilen (radat1
.xml el. motsv.) finnas på alla datorer som på ett eller annat sätt
behandlar stämplingsuppgifter. En tom fil EMIT.DAT får finnas med, men den skapas alltid vid behov.
Filen COMFILE.DAT måste raderas när filen
KILP.DAT distribueras till olika datorer, eftersom COMFILE.DAT
då kan innehålla oskickade meddelanden som orsakar
felsituationer när programmen används.

Ett bra förfarande är att göra kommandofiler som
först raderar potentiellt problematiska filer och sedan kopierar
de nya filerna KilpSrj.xml, KILP.DAT och radat1.xml
till de mappar som används via nätverket. Detta förutsätter att förbindelserna mellan datorerna
som dataöverföringen använder har tillåtits genom att de aktuella sökvägarna definierats
som delade i nätverket. Kommandofilens innehåll kan till exempel vara

```
del \\192.168.1.11\Kisat\kisa1\*.dat
del \\192.168.1.11\Kisat\kisa1\*.lst
copy c:\kisat\kisa1\KilpSrj.xml \\192.168.1.11\Kisat\kisa1\
copy c:\kisat\kisa1\KILP.DAT \\192.168.1.11\Kisat\kisa1\
copy c:\kisat\kisa1\radat1.xml \\192.168.1.11\Kisat\kisa1\
```

Här har det antagits att stämplingskontrollens dator 192.168.1.11 har delat mappen C:\kisat
under namnet Kisat med tillåtelse att skriva i mappen och dess undermappar,
och att tävlingens uppgifter finns i undermappen kisa1 till denna mapp både på den dator som distribuerar uppgifterna och
på den som tar emot dem. Likaså har det antagits att ingen fil med ändelsen \*.dat eller \*.lst eventuellt skulle behöva bevaras.

De ovan uppräknade kommandona lämpar sig för användning från servern, och då kan motsvarande
rader för alla övriga tävlingsmappar tas med i samma fil. Alternativt kan motsvarande kommandon genomföras så att endast servern
delar mappen Kisat och de andra datorerna hämtar uppgifterna med ett sådant
kommando samtidigt som datorerna startas för tävlingen.

Separat måste du se till att de nödvändiga konfigurationsfilerna finns tillgängliga och att startinställningarna samt de fönster som ska öppnas
vid behov har sparats från programmet HkKisaWin före tävlingens början.
