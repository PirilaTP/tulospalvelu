# Bilaga 5. Motionsorienteringsmodell 3

## Bilaga 5. Motionsorienteringsmodell 3

### A5.1 Grundbeskrivning av driftläget

Driftläge 3 för motionsorientering aktiveras genom att ge
programmet startparametern

`KUNTOSUUNNISTUS=3`

Om även förinläsning används måste förinläsningens program få parametern

`ESILUENTA=KUNTO`

Till motionsorienteringsläget kommer du sedan genom att öppna programmet i resultattjänstläge och välja
*Resultattjänst/Emit-avläsning* eller *Deltagare / Anmälningar* i huvudmenyn, beroende
på vilken uppgift som sköts vid den aktuella platsen. Alternativt kan programmet ställas in att öppnas
direkt i resultattjänstläge och öppna formuläret Emit-avläsning vid start.

I driftläge 3 bildas helheten av följande uppgifter:

- Förberedelse av filerna- Inmatning av deltagaruppgifter i tävlingskansliet- Tilldelning av Emit-koder till deltagarna och markering av deltagarna
      som startande.- Stämplingskontroll, i samband med vilken även resultaten fås.

### A5.2 Förberedelser för driftläge 3

Förberedelserna omfattar följande steg:

- Definition av tävlingen
  - Den filmapp som ska användas väljs och skapas antingen utanför programmet eller i funktionen
    *Skapa tävling och grundegenskaper*- Tävlingens klasser skapas och namnges antingen i nämnda
      funktion eller genom att kopiera en tidigare lämplig fil KilpSrj.xml
      till tävlingens filmapp innan tävlingens grunduppgifter definieras. Bland klasserna måste
      en extra vakansklass ingå, där alla uppgifter placeras vid förberedelsen.
      Programmet försöker skapa en lämplig
      vakansklass automatiskt.- I grundegenskaperna bör du definiera rubrikuppgifterna och ge tävlingen en identifieringskod som
        skiljer sig från andra tävlingar.  
  - Definition av banor i programmets val *Förberedelse / Banuppgifter*. För varje klass måste
    en bana definieras vars namn är detsamma som klassens namn. Som nollställningsförskott
    anges *Nollställning vid starttid* och för målet ska
    *Stämplingsenhet på mållinjen (ej online*) väljas. Även för vakansklassen rekommenderas
    en bandefinition för att eliminera störande felmeddelanden.
    I vakansklassens bana tas minst en kontroll med som inte finns i skogen och som ges en
    kontrollkod som inte heller används, så att ingen kan få en godkänd
    prestation i vakansklassen.
      
      
    - Upprättande av konfigurationsfil. Enklast är att använda standardnamnet
      `Laskenta.cfg` som filnamn. Filen måste innehålla åtminstone följande rader:  
        
      - På kansliets dator
          
         `KUNTOSUUNNISTUS=3`
          
          
        - På förinläsningens dator
            
           `ESILUENTA=KUNTO
            
          LUKIJA=n`
            
            
          - På stämplingskontrollens dator
              
             `KUNTOSUUNNISTUS=3
              
            LUKIJA=n
              
            EMITALKU=aaa`  
      där nummervärdet i parametern `LUKIJA=n` måste vara numret på den serieport som används, alltså `LUKIJA=1`
      om `COM1` används. Parametern EMITALKU
      behövs när det finns flera stämplingskontrollplatser. Varje plats måste
      använda ett annat startvärde aaa. Värdena bör väljas mellan 1000
      - 30000 för att undvika värdet 200 för felaktiga brickor och de nummerområden som
      brickorna annars använder.
        
        
      Dessutom måste de parametrar som dataöverföringen kräver anges.
        
        
      - Poster skapas för varje deltagare enligt de nummerområden för möjliga nummerlappar
        som används, när programmet är i förberedelseläge, i valet *Deltagare / Lägg till vakanta*.
        Posterna markeras då som Ej startat. Dessutom skapas extra vakanser, av vilka ett litet
        antal behövs i grundverksamheten och som dessutom fungerar som reservplatser om sådana någon gång behövs.
        Dessa vakanser skapas utanför det nummerområde som används och markeras som vakanser.
        Starttiden bör lämnas öppen för alla.
          
          
        - Eventuellt förbereds för tävlingskansliets bruk en persondatabas utifrån tidigare deltagare.
          Filen ges namnet `henkilot.csv`.
          Filens första rad är
            
            
          `KilpId;Sukunimi;Etunimi;Seura;Maa;Joukkue;Sarja;Badge`  
            
          och på var och en av de övriga raderna, enligt denna rubrikrad och åtskilda med semikolon, åtminstone följande uppgifter
          (saknade uppgifter utelämnas, men antalet semikolon måste vara rätt):
            
          - Den tävlandes identifieringskod, som används för att identifiera
            personer och som sparas i licensfältet. Nödvändig
            för det kontinuerliga underhållet av filen.- Efternamn, högst 24 tecken- Förnamn, högst 24 tecken- Klubb eller sammanslutning, högst 31 tecken- Land. Används i regel inte, 3 tecken- Lag. Används i regel inte, högst 15
                      tecken- Klass. Inte nödvändig. Används som standard om
                        den är densamma som i klassuppgifterna. Underlättar arbetet något.- Emit-brickans kod. En väsentlig hjälp när brickan används av en eller högst några få personer.
                          Bör inte sparas om personen inte använder samma bricka upprepade gånger.  
          - En klubbdatabas förbereds, om en sådan ska användas. Databasen innehåller på varje rad följande
            uppgifter åtskilda med tabulator, semikolon eller mellanslag:
            - Distriktskod. Nummervärde, som anges som 0 om
              inget annat värde finns.- Namnförkortning. En förkortning på högst 15 tecken som
                inte innehåller mellanslag eller semikolon. Används i regel inte, men någon
                text måste finnas.- Klubbens namn. Högst 31 tecken.  
            - Filerna
              `KilpSrj.xml ja KILP.DAT` kopieras till tävlingens arbetskatalog på varje dator,
              där även en konfigurationsfil anpassad för respektive dator placeras.
              Till stämplingskontrollens dator kopieras också filen  `radat1.xml`.
              Filerna `henkilot.csv ja Seurat.csv` placeras
              på tävlingskansliets dator, om de har upprättats.
                
                
              - Om du vill att programmet startar automatiskt i användningsläge
                - starta programmet i önskat läge- spara fönstren i valet *Filer / Spara fönster*- i valet *Filer / Startinställningar*
                      - Välj konfigurationsfil- Ta med raden `ikkunat` i den, och lägg till filnamnet
                          om fönstren sparades i en annan fil än `ikkunat.xml`. Eventuella ändringar måste
                          sparas.- Välj *Öppna direkt i resultattjänst* och spara valen.- När du vill avsluta den automatiska starten väljer du att spara
                              *Öppna till huvudmenyn*

### A5.3 Arbetet under evenemanget

### A5.3.1 Tävlingskansliet

Programmet startas. Gå till resultattjänstläget och välj den katalog
och konfigurationsfil som används. Välj vidare
*Deltagare / Anmälningar* i menyn.

De tävlandes uppgifter matas in genom att välja den post som ska behandlas med startnumret och
registrera åtminstone namnet samt vid behov klubben. Även klassen kan anges, men
det är inte nödvändigt. Den tävlande lämnas som *Ej startat*. Om det är möjligt att använda
persondatabasen och kanske även klubbregistret går arbetet väsentligt snabbare.

### A5.3.2 Förinläsning

Programmet startas. Gå till resultattjänstläget och välj den katalog
och konfigurationsfil som används, om dessa funktioner inte har automatiserats på det sätt
som beskrivs ovan. Välj vidare *Resultattjänst / Emit-avläsning* i menyn,
om inte heller detta sker automatiskt. När deltagaren kommer till platsen läses brickan
och den tävlandes nummer matas in, vilket också kan ske genom att läsa en streckkod. När du trycker på
*Enter* i nummerfältet markeras den tävlande som öppen och händelsens klockslag registreras
i starttidsfältet. Starttiden har betydelse endast som information för uppföljningen av vilka som är i skogen.

Registreringen kan ångras om det av någon anledning behövs.

Om brickan redan används påpekar programmet detta och ger möjlighet att antingen
låta brickan förbli kopplad till den tidigare registrerade deltagaren eller frigöra den för ny användning.

### A5.3.3 Stämplingskontroll

När en tävlande som behandlats normalt kommer till stämplingskontrollen beräknar programmet den tävlandes tid
och visar stämplingarna. Om stämplingarna motsvarar någon bana bestämmer programmet klassen utifrån banan.

Om den tävlandes bricka inte har lästs vid förinläsningen registreras uppgiften på en vakansplats, från vilken den
måste flyttas till rätt deltagare med knappen *Byt tävlande eller ändra stämplingens data*,
genom att sedan mata in numret eller söka med namn och slutligen bekräfta registreringen med knapparna *Flytta stämplingarna
till vald tävlande* och *Spara gjord flyttning*. Efter att den förstnämnda knappen
har använts visas den tävlandes uppgifter i stämplingskontrollformuläret, men de har ännu inte sparats.

### A5.3.4 Granskning av resultat

Resultaten kan visas i huvudmenyns val *Uppföljning / Ställning*.

Deltagaruppgifterna kan också visas och redigeras i valet *Deltagare / Deltagare*,
som öppnar en tabell.

I tabellen i båda de ovan nämnda valen kan du klicka på en deltagare för att få mer information.
Uppgifterna som visas är inte identiska, utan kompletterar delvis varandra.

### A5.4 Arbetet efter evenemanget

### A5.4.1 Resultatlistor

Resultatlistor till html-fil eller papper kan göras i valet *Utskrifter*, där
du ska välja de klasser som ska skrivas ut samt typen av resultatlista.

### A5.4.2 Underhåll av databaserna

Persondatabasfilen kan kompletteras på kansliets dator genom att öppna den funktion där den visas och sedan välja
*Redigering / Lägg till tävlande i databasen*.

Om en MySQL-databas används kan evenemangets deltagaruppgifter föras över dit i valet
*Filer / Skriv till överföringsfil*. Fler anvisningar i kapitel 11.
