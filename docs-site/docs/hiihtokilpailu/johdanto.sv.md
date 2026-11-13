# Beskrivning av exempeltävlingarna

### Resultattjänst för skidtävlingar – exempel

Pekka Pirilä, 12.2.2015

Denna anvisning går igenom de åtgärder som hör till
arrangemanget av några olika slags skidtävlingar.

De tävlingstyper som behandlas är:

1. Ett litet evenemang som
   sköts med en dator

   - En typisk nationell tävling med datorer i målet,
     i resultatcentralen och hos speakern.

     - Jaktstartstävlingens ytterligare särdrag jämfört med föregående.

       - Sprint i längdskidåkning

Det sista kapitlet i dessa anvisningar är en detaljerad
förteckning över de steg som resultattjänsten vid en mycket liten tävling
kräver.

Innan dessa arbetsmoment påbörjas måste programvaran installeras, och det
antas ha skett enligt installationsanvisningen.

Programmet används i tre olika skeden:

- Tävlingen skapas och
  grundvalen görs innan man går vidare till övriga skeden

  - Klasserna definieras i förberedelseläget. I detta läge genomförs också inmatningen av tävlande och lottningen,
    om inte tävlande läggs till först under själva evenemanget.

    - I resultattjänstläget sköts själva resultattjänsten. I detta läge är det också möjligt att
      lägga till tävlande som anmäler sig under evenemanget.

Efter att grundvalen har gjorts lönar det sig att genast gå över till förberedelseläget. Från förberedelseläget kan du inte gå över
till resultattjänstläget på annat sätt än genom att däremellan avsluta programmet.

I fall 1 finns några tiotal deltagare,
som eventuellt anmäler sig på tävlingsplatsen. Tidtagningen sker med datorns klocka.

I fall 2 väntas den aktuella tävlingen samla ett par
hundra deltagare. Tidtagningen sker med en målklocka. De tävlandes nummer matas in
manuellt på datorn vid sidan av tiden från klockan. Även en
startgrind används. I själva resultattjänsten används också en andra
dator för registrering av olika ändringar samt för att sammanställa
utskrifter. En tredje dator finns hos speakern för att följa tävlingen.

För fall 3 och
4 granskas de ytterligare åtgärder som hör till förberedelsen och
skötseln av tävlingen. I övrigt motsvarar fallen fall 2.

Ett litet evenemang enligt fall 1 omfattar som minimum
följande
steg:

- [Skapa tävlingen
  och göra grundvalen](perusvalinnat.md)

  - [Definiera klasserna](sarajatiedot.md)
    i förberedelseläget

    - Sköta själva
      evenemanget
      i resultattjänstläget

      - [Mata in deltagarna](ilmoittautumisten_syotto.md) 
        och samtidigt ge starttider och
        nummer i funktionen *Deltagare / Anmälningar*

        - [Tidtagning](ajanotto.md)
           med datorns
          klocka i funktionen *Resultattjänst / Tidtagning*

          - [Sammanställa
            resultatlistor](tulosluettelot.md) 
            för skrivare och/eller
            fil

Även skötseln av ett litet evenemang kan innebära behov av att
registrera anmälningar i förväg, genomföra lottning samt upprätta
konfigurationsdefinitioner för resultattjänstskedet, till exempel för användning
av startklocka.
