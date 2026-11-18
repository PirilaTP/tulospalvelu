# Bilaga 3. Motionsorienteringsmodell 1

## Bilaga 3. Motionsorienteringsmodell 1

### A3.1 Grundbeskrivning av driftläget

Driftläge 1 för motionsorientering aktiveras genom att ge
programmet startparametern

`KUNTOSUUNNISTUS=1`

Till motionsorienteringsläget kommer du sedan genom att öppna programmet i resultattjänstläge och välja
"Resultattjänst/Emit-avläsning" i huvudmenyn. Alternativt kan programmet ställas in att öppnas
direkt i resultattjänstläge och öppna formuläret Emit-avläsning vid start.

I driftläge 1 innehåller tävlandefilen i början inga uppgifter om deltagarna, utan uppgifterna
om dem finns i en separat förteckning över möjliga deltagare (persondatabas).
Tävlandefilen måste då innehålla så många vakansplatser att alla
deltagare säkert ryms med.

I driftläge 1 sker anmälan som startande och stämplingskontroll vid olika inläsningstillfällen.
När brickan läses första gången markerar programmet den tävlande som startad och sparar
Emit-koden i deltagarens uppgifter. När Emit-brickan läses en andra gång sparas resultatet
och Emit-koden ändras till ett ledigt värde i området för små tal. Därmed är den tredje inläsningen
åter en ny tävlandes anmälan, på exakt samma sätt som
den första inläsningen. Anmälan och stämplingskontroll kan ske
på samma eller olika datorer, förutsatt att det finns en dataöverföringsförbindelse mellan datorerna.

Resultatet kan dock sparas redan vid den första inläsningen, förutsatt att
stämplingarna ger en godkänd prestation för någon bana. I annat fall behövs en andra inläsning,
och mellan inläsningarna måste någon annan bricka läsas.

### A3.2 Förberedelser för driftläge 1

Förberedelserna omfattar följande steg:

- Definition av tävlingen
  - Den filmapp som ska användas väljs och skapas antingen utanför programmet eller i funktionen
    *Skapa tävling och grundegenskaper*- Tävlingens klasser skapas och namnges antingen i nämnda
      funktion eller genom att kopiera en tidigare lämplig fil KilpSrj.xml till tävlingens
      filmapp innan tävlingens grunduppgifter definieras. Bland klasserna måste
      en extra klass för vakansuppgifter ingå. Programmet försöker
      skapa en lämplig vakansklass automatiskt.- I grundegenskaperna bör du definiera rubrikuppgifterna och ge tävlingen en identifieringskod som
        skiljer sig från andra tävlingar.  
  - Definition av banor i programmets val *Förberedelse / Banuppgifter*
    .
    För varje klass måste en bana definieras vars namn är detsamma som klassens namn. Som nollställningsförskott
    anges *Nollställning vid starttid* och för målet
    ska *Stämplingsenhet på mållinjen (ej online*) väljas. Även för
    vakansklassen rekommenderas en bandefinition för att eliminera störande
    felmeddelanden. I vakansklassens bana
    tas minst en kontroll med som inte finns i skogen och som ges en
    kontrollkod som inte heller används, så att ingen kan få en godkänd
    prestation i vakansklassen.
      
      
    - Upprättande av konfigurationsfil. Enklast är att använda standardnamnet
      `Laskenta.cfg` som filnamn. Filen måste innehålla åtminstone följande rader:
        
       `KUNTOSUUNNISTUS=1
        
      LUKIJA=n  
      VAKANTTIALKU=kkk`

        
      där nummervärdet i parametern `LUKIJA=n` måste vara numret på den serieport som används, alltså `LUKIJA=1`
      om `COM1` används. Parametern VAKANTTIALKU behövs när det finns flera
      stämplingskontrollplatser.
        
        
      Dessutom måste de parametrar som dataöverföringen kräver anges om flera sammankopplade datorer används.
      Då måste också vakansområdena för de olika datorerna definieras med parametern `VAKANTTIALKU=nnnn` så att
      varje dator säkert har tillräckligt med vakanser i sitt eget område.
      På en av datorerna väljs nnnn så att det motsvarar den nedre gränsen för
      det nummerområde som används, och på de andra mitt i detta nummerområde.
      Vidare kan parametern `SEURATIETO=E` anges om du inte vill hantera klubbuppgifter,
      och parametern `VAHVISTAAINA=E`
      om du vill att programmet godkänner deltagaren utan bekräftelse när
      det bara finns ett alternativ.   
        
      Parametern KUNTOLÄHTÖ lämpar sig för användning på anmälningsplatsens
      dator, om man är säker på att endast anmälande kommer till platsen. När denna
      parameter används har all överflödig information rensats bort från formuläret.
      Via menyn kan detta läge aktiveras och man kan återgå till den normala
      stämplingskontrollskärmen.  
        
      Parametern KUNTOMAALI leder till att programmet förbereder sig på
      möjligheten att orienteraren kommer i mål efter sin prestation, även om
      brickan inte har registrerats vid starten. Då frågar programmet om det gäller start
      eller målgång. Utan denna parameter antar programmet att alla vars
      bricka ännu inte har lästs och som inte har en
      godkänd prestation för någon bana är startande. Om brickan innehåller en godkänd
      prestation frågar programmet i vilket fall som helst om det gäller en målgång.
        
        
      Sättet att använda parametrarna LÄHEMITx=V och LÄHEMITy=I som presenteras i kapitel 6.6.4
      kan underlätta slutbearbetningen av de uppgifter som samlats in under evenemanget för sträcktidsutskrifter.
        
        
      - Vakansplatser skapas när programmet är i förberedelseläge, i valet *Deltagare / Lägg till vakanta*.
        Det lönar sig att skapa många vakanser och så att det efter varje `VAKANTTIALKU=nnnn` som angetts med parametern
        finns fler vakanser än det maximala antalet deltagare som kan anmäla sig vid en dator.
        Vakanserna skapas i den klass som reserverats för vakanser, som vakanser och med efternamn och klubb tomma.
          
          
        Samma fil med vakanser kan användas
        vid flera olika evenemang, om antalet klasser och vakansklassens
        ordningsnummer i klasslistan är detsamma i alla.   
          
        Antalet vakanser som programmet
        skapar automatiskt styrs med parametern VAKANTTILUKU=nnn i förberedelseskedets
        konfigurationsfil ilmoitt.cfg eller i resultattjänstskedets
        konfigurationsfil. Parametern LISÄVAKANTTIALKU=nn
        i denna fil anger startpunkten för vakansernas nummerområde. Utan denna
        parameter läggs vakanserna till i slutet av nummerområdet.
          
          
        - En persondatabas förbereds utifrån tidigare deltagare. Filen ges namnet `henkilot.csv`.
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
            - Alla filer som skapats i de föregående stegen,
               `KilpSrj.xml, KILP.DAT, radat1.xml, henkilot.csv, Seurat.csv`, kopieras till tävlingens
              arbetskatalog på varje dator, där även en konfigurationsfil
              anpassad för respektive dator placeras.
                
                
              - Om du vill att programmet startar automatiskt i användningsläge
                - starta programmet i önskat läge- spara fönstren i valet *Filer / Spara fönster*- i valet *Filer / Startinställningar*
                      - Välj konfigurationsfil- Ta med raden `ikkunat` i den, och lägg till filnamnet
                          om fönstren sparades i en annan fil än `ikkunat.xml`. Eventuella ändringar måste
                          sparas.- Välj *Öppna direkt i resultattjänst* och spara valen.- När du vill avsluta den automatiska starten väljer du att spara
                              *Öppna till huvudmenyn*

### A3.3 Arbetet under evenemanget

Programmet startas. Gå till resultattjänstläget och välj den katalog
och konfigurationsfil som används, om dessa funktioner inte har automatiserats på det sätt
som beskrivs ovan. Välj vidare *Resultattjänst / Emit-avläsning* i menyn, om
inte heller detta sker automatiskt.

När deltagaren kommer till starten läses brickan. Då kan följande hända:

- Om Emit-koden hittas i persondatabasen för en deltagare frågar programmet
  om personen är rätt och sparar personen som deltagare om du svarar *Ja*.
  Svaret *Nej* leder till samma gren som en bricka som inte finns i databasen.
  Med en parameter eller via menyn kan denna begäran om bekräftelse tas bort.  
  - Om Emit-koden hittas i persondatabasen för flera deltagare ber programmet
    dig välja rätt person med tangenterna *pil upp / pil ned*. När
    rätt person är vald trycker du på *Enter* och sedan *Ctrl-Enter* eller
    bekräftar med knappen *Spara*.  
    - Om Emit-koden inte hittas i databasen flyttar programmet till efternamnsfältet, där
      du börjar skriva namnet. Om namnet finns i databasen kan du välja det när
      alternativen har minskat tillräckligt, med tangenten *Enter* eller genom att bläddra
      dessförinnan med tangenterna *pil upp / pil ned*. När namnet är rätt kontrollerar du
      vid behov klubb och klass och godkänner med tangenterna *Ctrl-Enter* eller
      knappen *Spara*.- Om deltagarens bricka innehåller uppgifter som motsvarar
        en godkänd prestation på någon bana som används frågar programmet om
        det gäller en anmälan eller en målgång.- Om en tävlande kommer i mål utan föregående anmälan och så att stämplingarna
          innehåller ett fel, måste brickan läsas på nytt efter att någon annan bricka har lästs emellan.
          För att brickan inte ska stängas av före den nya inläsningen måste man stämpla strax efter den första inläsningen
          med någon stämplingsenhet som inte är målets stämplingsenhet.

När en tidigare anmäld deltagare kommer i mål läses brickan. Programmet fastställer
genast resultatet, visar stämplingarna och avgör banan utifrån stämplingarna. I regel behöver man inte göra
något annat, men uppgifterna kan redigeras om de inte är rätt. När man redigerar
visas knappen *Spara* som tecken på att ändringarna måste sparas,
antingen med knappen eller med tangenterna *Ctrl-Enter*.

Tidigare inlästa resultat kan visas genom att bläddra med bläddringslisten eller genom att använda
knappen *Sök deltagare* och de sökmöjligheter som då visas.
Även uppgifter som visats på skärmen på detta sätt kan ändras och ändringarna sparas.

### A3.3.1 Granskning av resultat

Resultaten kan visas i huvudmenyns val *Uppföljning / Ställning*.

Deltagaruppgifterna kan också visas och redigeras i valet *Deltagare / Deltagare*,
som öppnar en tabell.

I tabellen i båda de ovan nämnda valen kan du klicka på en deltagare för att få mer information.
Uppgifterna som visas är inte identiska, utan kompletterar delvis varandra.

### A3.4 Arbetet efter evenemanget

### A3.4.1 Resultatlistor

Resultatlistor till html-fil eller papper kan göras i valet *Utskrifter*, där
du ska välja de klasser som ska skrivas ut samt typen av resultatlista.

### A3.4.2 Underhåll av databaserna

Persondatabasfilen kan kompletteras genom att öppna den funktion där den visas och sedan välja
*Redigering / Lägg till tävlande i databasen*.

Om en MySQL-databas används kan evenemangets deltagaruppgifter föras över dit i valet
*Filer / Skriv till överföringsfil*. Fler anvisningar i kapitel 11.
