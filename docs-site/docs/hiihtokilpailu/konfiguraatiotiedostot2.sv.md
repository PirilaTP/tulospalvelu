# Konfigurationsfiler

### Konfigurationsfiler

De grundläggande funktionerna i resultattjänsten under tävlingen
är tidtagning och stöd till speakern. Dessutom kan det vara nödvändigt att göra
vissa ändringar i deltagaruppgifterna. För dessa uppgifter används tre
datorer:

- **Tidtagningsdatorn**.
  Till denna dator är målklockan ansluten, och till den är i sin tur
  startgrinden och fotocellen (eller en tidtagningsknapp) anslutna. Denna dator fungerar också
  som central dator för dataöverföringen mellan datorerna och är ansluten till båda de andra
  datorerna. För tidtagningen lämpar sig åtminstone tills vidare bäst
  programmet *HkMaali*, eftersom inmatningen av de tävlandes nummer
  vid sidan av de tagna tiderna går smidigast i detta program.

  - **Speakerns
    dator.** Denna dator används för att visa uppgifter som stöder speakerns
    arbete. Programmet *HkKisaWin* används.

    - **Kontorets
      dator.** På denna dator görs eventuella nödvändiga ändringar
      i deltagaruppgifterna och utskrifter sammanställs både för skrivare och till html- och
      andra filer. Programmet *HkKisaWin* används
      .

Varje dators uppgifter förutsätter att en motsvarande
konfigurationsfil upprättas och placeras i tävlingens katalog.
Konfigurationsfilens standardnamn är Laskenta.cfg, vilket
kan användas på alla datorer, men ett annat och kanske bättre alternativ är
att ge filerna namn som motsvarar datorns uppgift: maali.cfg, kuul.cfg och toimisto.cfg.

Konfigurationsfilen för tidtagningsdatorn har ett innehåll
av följande slag (rader som börjar med // är kommentarer som förklarar följande parameter)

```
//Datorns kod
kone=MA
//starta programmet utan att fråga om godkännande av valen
boot
//Definiera en ogiltig kod för den tangent som används för tidtagning från tangentbordet. 
//(Programmet frågar då inte efter tangenten och tid kan inte tas från tangentbordet)
näppäin=99,99
//Målklockans anslutning (som standard klockan Timy ansluten till serieporten COM5)
timy=5
//Tider från klockans startgrind tolkas i programmet som starttider och fotocellens tider som måltider.
//För alla klasser används härledning av den startande utifrån den tagna tiden
pisteet=mmmlm/kaikki
//Alla tider som inte kommer från startgrinden är sluttider (alltså inte mellantider)
pisteet=m/vain
//tidtagningsuppgifterna bevaras vid omstart
ajat=/s
//startgrindens tider sparas i en separat fil
lajat
//förbindelse till kontorets dator (datorns ip-nummer 192.168.1.11)
yhteys1=udp:0/192.168.1.11
//överföring av tidtagningstabellen till kontorets dator som kan tas i bruk vid behov (inte nödvändig)
lähaika1
//förbindelse till speakerns dator (datorns ip-nummer 192.168.1.12)
yhteys2=udp:0/192.168.1.12
```

Konfigurationsfilen för kontorets dator har ett innehåll av följande slag

```
//Datorns kod
kone=TO
//tidtagningsuppgifterna bevaras vid omstart
ajat=/s
//startgrindens tider sparas i en separat fil
lajat
//förbindelse till måldatorn
yhteys1=udp
//överföring av tidtagningstabellen från måldatorn som kan tas i bruk vid behov (inte nödvändig)
lähaika1
```

Konfigurationsfilen för speakerns dator har ett innehåll av följande slag

```
//Datorns kod
kone=KU
//enkelriktad förbindelse till måldatorn (från speakerns dator kan inga uppgifter skickas till andra)
yhteys1=udpi
//Följande parameter ber programmet att öppna den sparade formulärlayouten vid start
ikkunat
```

På måldatorn ska en genväg till programmet HkMaali
skapas så att den pekar på tävlingsdatakatalogen. När programmet HkKisaWin
används behövs inget motsvarande förfarande, utan arbetskatalogen väljs
inifrån programmet.

Startinställningarna för programmet HkKisaWin är särskilt
användbara på speakerns dator, där det lönar sig att gå till väga på följande sätt (detta är
möjligt endast när filerna KilpSrj.xml och KILP.DAT finns tillgängliga och
programmet har startats i resultattjänstläget).

- skapa den fönsterlayout som
  speakern kommer att använda under tävlingen
- spara layouten i filen oletus.ikk med fönsterprogrammets val *Filer /
  Spara fönster*
- I valet *Filer / Startinställningar*
  väljer du öppning direkt i resultattjänsten samt den ovan
  beskrivna filen som konfigurationsfil.
