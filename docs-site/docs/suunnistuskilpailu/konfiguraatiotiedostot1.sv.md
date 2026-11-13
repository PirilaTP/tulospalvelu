# Konfigurationsfiler

De konfigurationsfiler som visas här bygger på
följande val:

- Det **alternativ
  2** som beskrivs ovan på sidan *Datorernas
  uppgifter* används .

  - Datorernas ip-nummer är:

    - 192.168.1.10  Kontoret

      - 192.168.1.11  Stämplingskontrollen

        - 192.168.1.12  Speakern- För utskrifter används kontorsdatorns
      standardskrivare

      - Läsenheten är ansluten till stämplingskontrollens
        dator med en USB-omvandlare, som har fått porten COM5.

        - Klagomurens funktioner sköts på kontoret

          - Alla datorer har mappen C:\Kisat\Kisa1 för tävlingens bruk

            - På kontorets dator används
              undermappen C:\Kisat\Kisa1\HkKisaWin av programmet HkKisaWin och
              C:\Kisat\Kisa1\HkMaali av programmet HkMaali

              - Speakern följer inte de Emit-mellantider som fås
                från stämplingskontrollen

I följande konfigurationsfiler finns kommentarrader som börjar med
tecknen // och som förklarar vad följande rad betyder

**Konfigurering av kontorets dator**

Mappen C:\Kisat\Kisa1\HkMaali

- beskrivning

  - programmet
    HkMaali används

    - fungerar som nod
      för dataöverföringen, dvs. som "server"

      - sköter även den automatiska utskriften

        - programmet startas från en genväg på
          skrivbordet, vars startmapp har definierats som C:\Kisat\Kisa1\HkMaali och
          där parametern CFG=SR.CFG har skrivits efter namnet på programmet
          HkMaali.exe som ska startas- innehållet i konfigurationsfilen SR.CFG

```
// 2-teckens kod som stöd för tolkningen av dataöverföringsförbindelserna
kone=sr
// Rubrik för konsolfönstret som öppnas
ikkunaots=Server
// Förbindelse till stämplingskontrollens dator. Överför även stämplingsuppgifterna
yhteys1=udp:0/192.168.1.11:y1
lähemit1
// Enkelriktad förbindelse till speakerns dator. Överför inte stämplingsuppgifterna
yhteys2=udpo:0/192.168.1.12:y1
// Förbindelse till programmet WinKisaHk på kontorets dator. Överför även stämplingsuppgifterna
yhteys3=udp:0/localhost::y4
lähemit3
// Programmet använder standardskrivaren
lista
// Programmet skriver automatiskt ut resultatlistor
auto=L/40/60
//  Programmet skriver automatiskt ut en rapport över stämplingar som lett till förslag om diskvalificering
comautorap=h
```

Mappen C:\Kisat\Kisa1\HkKisaWin

- beskrivning

  - programmet HkKisaWin används

    - används för kontorets olika uppgifter,
      inkl. klagomurens behov

      - programmet startas från en genväg på
        skrivbordet, som är grundgenvägen för att starta programmet HkKisaWin- starten till denna tävling har automatiserats
          i programmet HkKisaWin med valet *Filer /
          Startinställningar* vid en tidigare användning (uppgiften sparas
          i filen init.cfg, men du behöver inte själv
          sköta detta direkt).- då har to.cfg valts som konfigurationsfil- innehållet i konfigurationsfilen to.cfg

```
// 2-teckens kod som stöd för tolkningen av dataöverföringsförbindelserna 
kone= to 
// Förbindelse till programmet HkMaali på kontorets dator. Överför även stämplingsuppgifterna 
yhteys4= udp
lähemit4
```

**Konfigurering av stämplingskontrollens dator**

- beskrivning

  - programmet
    HkKisaWin används

    - sköter stämplingskontrollen, som
      även ger sluttiderna

      - programmet startas från en genväg på skrivbordet, som är grundgenvägen för att starta programmet HkKisaWin- starten till denna tävling har automatiserats i programmet HkKisaWin med valet *Filer / Startinställningar* vid en tidigare användning (uppgiften sparas i filen init.cfg, men du behöver inte själv sköta detta direkt).- då har em.cfg valts som konfigurationsfil- vid ett tidigare användningstillfälle har läget för de fönster
              som ska öppnas sparats med valet *Filer / Spara fönster*, när
              stämplingskontrollfönstret var på önskad plats på skärmen (valet sparas
              i filen oletus.ikk)- innehållet i konfigurationsfilen em.cfg

```
// 2-teckens kod som stöd för tolkningen av dataöverföringsförbindelserna
kone=em
//  Programmet öppnar de tidigare valda fönstren vid start
ikkunat
// Förbindelse till programmet HkMaali på kontorets dator. Överför även stämplingsuppgifterna
yhteys1=udp
lähemit1
//  Läsenheten ansluten till porten COM5 (med hjälp av en UDP-/serieportsomvandlare)
lukija=5
```

**Konfigurering av speakerns dator**

- beskrivning

  - programmet
    HkKisaWin används

    - speakern
      följer tävlingsläget med hjälp av visningen av de senaste händelserna samt ett
      fönster som visar ställningen i två
      valda klasser

      - programmet startas från en genväg på skrivbordet, som är grundgenvägen för att starta programmet HkKisaWin- starten till denna tävling har automatiserats i programmet HkKisaWin med valet *Filer / Startinställningar* vid en tidigare användning (uppgiften sparas i filen init.cfg, men du behöver inte själv sköta detta direkt).- då har ku.cfg valts som konfigurationsfil- vid ett tidigare användningstillfälle har läget för de fönster
              som ska öppnas sparats med valet *Filer / Spara fönster*, när
              fönstren var på önskade platser på skärmen (valet sparas
              i filen oletus.ikk)- innehållet i konfigurationsfilen ku.cfg

```
// 2-teckens kod som stöd för tolkningen av dataöverföringsförbindelserna
kone=ku 
// Programmet öppnar de tidigare valda fönstren vid start
ikkunat  
// Förbindelse till programmet HkMaali på kontorets dator.
yhteys1=udp
```
