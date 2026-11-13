# Steg för steg: en mycket liten tävling

### Steg för steg: en mycket liten tävling

Detta kapitel innehåller en detaljerad förteckning över åtgärderna
för att sköta en helt liten tävling, med början från noll. Innan detta behöver du bara installera
programmet och ha dess startikon tillgänglig. Här antas
att ingen lottning genomförs, utan att deltagarna får starttider samtidigt som deras namn sparas.

1. **Skapa tävlingen**

1. Starta programmet HkKisaWin från ikonen
2. Klicka på *Skapa tävling och
   grundegenskaper*
3. Klicka på *Välj tävlingens katalog* och
   välj en ledig katalog eller skapa en ny katalog och välj den
4. Skriv på den översta raden tävlingens namn, som kommer med i resultatlistorna
5. Välj tävlingens gren och precisering (nu
   *Skidåkning* och *Normal* )
6. Kryssa i *Skapa nya klassdata* och ange
   antalet klasser som ska användas (detta kan utökas senare)
7. Klicka på *Godkänn gjorda val och avsluta*

2. **Namnge klasserna**

1. Välj *Förberedelse* och
   *Klassuppgifter* i menyn
2. Klicka på *Tillåt redigering
   i tabellen*
3. Ändra klassernas namn till önskad form
4. När lottning inte används behöver du inte göra
   något annat, men om du vill kan du också ändra första numret, som är det lägsta
   startnumret för varje klass, samt ange klassens startintervall
   i sekunder och den första startandes starttid. Om du vill ha distansen i resultaten
   kan den anges i kolumnen *Distans* i kilometer med eventuella decimaler.
5. Uppgifterna ska till sist sparas med knappen
   *Spara redigerade data*. Sedan kan du stänga formuläret.
   (Formulären har ofta en knapp Stäng eller Avsluta, liksom detta. Även
   menyn har Stäng. Om formuläret inte har någon av dessa kan du stänga det med krysset i övre hörnet.)

3. **Skapa
   tävlingsfilen**

1. Klicka på den stora knappen *Öppna tävlingen
   för förberedelse* och välj den föreslagna filen ilmoitt.cfg. Då skapar programmet en tom
   deltagarfil. (Det finns inte alltid en konfigurationsfil i katalogen.
   Då kan du godkänna valet av en fil som inte finns. Det viktiga här är att hamna i rätt katalog.)
2. När tävlingen inte har någon lottning behöver du
   inte nödvändigtvis göra något annat i förberedelseläget. Om
   anmälningar redan är kända kan du mata in dem här, men jag behandlar inmatningen av dem längre fram.
3. Stäng programmet från menyn eller med krysset i övre hörnet.

4. **Registrera deltagarna
   genom att knappa in dem**

1. Starta programmet och klicka på *Öppna tävlingen
   för resultattjänst*. Nu kan du godkänna den föreslagna
   konfigurationsfilen laskenta.cfg .
2. Välj *Deltagare* och
   *Anmälningar* .
3. Gå till undersidan *Val av funktioner och
   uppgifter att behandla* .
4. Välj *Lägg till nya
   poster* i övre högra hörnet.
5. Kryssa i *Numrera tillagda
   automatiskt,* om du inte uttryckligen vill ange numren själv.
6. Ett kryss vid Ge starttid automatiskt
   kan hjälpa vid tilldelningen av starttider, men det kan hända att programmet inte kan
   känna till tiderna, så det lönar sig kanske inte att kryssa i det.
7. Du kan ta bort kryssen vid *Id-kod* och
   *Identifieringskoder*. De används inte i den här tävlingen.
8. Gå till den undersida som ursprungligen
   visades, *Användning* .
9. Börja registreringen av en ny deltagare i fältet
   Efternamn om du har begärt automatisk numrering, annars börjar du med att ange numret.
10. Gå till nästa fält med Enter-tangenten och fyll i de uppgifter som ska sparas
11. Välj åtminstone klass, antingen med musen,
    med piltangenterna upp och ner eller genom att skriva klassens namn tills rätt klass visas.
12. Registrera även starttiden (eller korrigera vid behov den starttid som programmet föreslår.
13. När Enter har flyttat fokus till knappen
    *Spara*, leder ytterligare ett Enter till att deltagaren sparas.
    Den automatiska numreringen sker i detta skede.
14. Uppgifterna visas i det övre tabellområdet,
    där du kan kontrollera att de blev rätt.
15. Om du upptäcker ett fel i uppgifterna måste du gå
    från *Tilläggsläget* tillbaka till *Redigeringsläget*. Då lönar det sig
    i allmänhet att kryssa i *Använd startnummer för att söka den
    som redigeras* .
16. Den tävlande hämtas för korrigering antingen genom att klicka på
    motsvarande rad i tabellen eller genom att mata in startnumret och trycka Enter.
17. När korrigeringarna har gjorts och sparats kräver
    tillägg av nya deltagare återigen att du går över till
    *Tilläggsläget* .
18. Formuläret kan stängas när deltagarna har
    registrerats, men det kan också lämnas öppet om fler anmälningar väntas.

5. **Tidtagning** (här tas endast
   sluttider under antagande att starttiderna har matats in rätt) Tiderna tas
   med en tangent på tangentbordet och enligt datorns klocka.

1. Välj *Resultattjänst, Tidtagning* och
   *Tidtagning - 1* .
2. Välj alternativet *Endast nummer* under *Registrering av ändringar
   i bläddringstabellen* .
3. Välj *Tidtagningsenheter* och
   *Tidtagningstangent och punkt*
4. Tryck på den tangent som ska användas för tidtagningen. Tangenten
   ½/§ i övre vänstra hörnet är ett bra val.
5. Välj *Mål* under *Punkt för vilken
   tider tas* och stäng formuläret.
6. Klicka i tabellens kolumn TävlNr på
   rutan på den första lediga raden, där det i förväg står 0.
7. Nu är programmet klart att ta tider och koppla dem till tävlande.
8. När den tävlande går i mål trycker du på den tangent som
   reserverats för tidtagningen, och tiden ska
   visas i kolumnen *Tid* .
9. Kontrollera att markören står i fältet TävlNr
   på den rad som motsvarar tiden, knappa in den tävlandes nummer och Enter.
10. När du har bekräftat sparandet ska
    den tävlandes namn och slutresultat visas för att tala om att resultatet har registrerats.
11. Du kan ta flera tider innan du börjar
    mata in nummer. Kontrollera alltid att numret hamnar på rätt rad.
12. Tidtagningsformuläret behöver inte stängas förrän
    tävlingen är över, även om andra formulär också används.

6. **Granska resultaten**

1. För att granska resultaten kan du välja
   *Uppföljning, Ställning* och *Ny* .
2. På detta formulär kan du se olika klassers
   resultat genom att byta klass. Flera liknande formulär kan vara öppna
   samtidigt, om de ryms på skärmen. Formuläret uppdateras normalt varje gång ett nytt resultat kommer.
3. På formuläret kan du också se starttider.

7. **Markera diskvalifikationer och
   avbrott**

1. Dessa markeringar kan göras antingen genom att öppna
   formuläret för den tävlandes uppgifter med valet *Deltagare / En
   deltagare,* där du även kan ändra andra uppgifter om deltagaren, eller
   med det särskilda formuläret *Deltagare / Markera kontrollkoder* .

8. **Sammanställa
   resultatlistor**

1. Välj *Utskrifter* och *Skrivare och
   filutskrifter* .
2. Välj antingen

1. Utskrift på skrivare och skrivaren som ska användas eller
2. Utskrift till fil, filtyp och filens namn

3. Välj klasserna som ska skrivas ut
4. Klicka på Skriv ut
