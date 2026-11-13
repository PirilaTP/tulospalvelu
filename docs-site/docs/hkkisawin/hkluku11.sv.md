# 11.1 Beräkning av tävlandes poäng

### 11.1 Beräkning av poäng per tävlande

#### 11.1.1 Poäng för en etapp

Poängen beräknas först etapp för etapp och kan sedan slås samman till totalpoäng som en separat åtgärd.

Poängen kan bestämmas utifrån placering, resultat eller båda.

Placeringsbaserade poäng definieras på följande sätt. Beräkningen bygger på 1–5 placeringsområden. Poängen
antas inom varje område ändras med ett konstant antal när placeringen ändras med ett. Fem områden tillåter en snabbare förändring
bland de främsta placeringarna och en långsammare bland övriga placeringar.

När poängen är desamma för varje placering oberoende av
antalet deltagare anger du

- i fältet *Startplac.* områdets bästa placering,

  - i fältet *Slutplac.* områdets
    sämsta placering,

    - i fältet *Första poäng* poängen
      för den förstnämnda placeringen och

      - i fältet *Steg* det antal
        poängen ändras med när placeringen försämras. Observera att detta
        tal är negativt när poängen sjunker då placeringssiffran ökar.

Om du vill ha poäng som minskar med placeringen
för alla som fått ett resultat, så att den siste till exempel får 1 poäng, anger du
talet 9999 i fältet Startplac. och i fältet Slutplac. den bästa placering som denna regel
gäller. Uppgiften Första poäng gäller då den som placerat sig sist, och bättre
placeringar får sina poäng enligt steget.  Om poängen stiger snabbare
i toppen anger du på en andra rad den placering från vilken ökningen blir snabbare. Då måste
samma placering anges i fältet Slutplac. på den övre raden och i fältet Startplac. på
den följande raden. Programmet fortsätter att fastställa poängen från den poäng
som regeln på den övre raden leder till.

Om du använder placeringspoäng som ökar när placeringen försämras anger du ett positivt tal i fältet *Steg*. Då kan
alla placeringar tas med genom att ange talet 9999 i fältet *Slutplac.*.

Resultatbaserade poäng kan beräknas antingen så att poängen minskar
proportionellt mot tidsskillnaden till vinnaren, eller så att poängen är proportionella
mot förhållandet mellan vinnarens tid och deltagarens tid eller mot logaritmen av förhållandet.
I det förra fallet skulle en mycket stor förlust leda till negativa poäng,
men poängen kan ges en nedre gräns. I det senare fallet ger även en stor
förlust positiva poäng, så någon nedre gräns behövs inte. Tidsskillnaden kan
hanteras både som den är, som ett tal som anger skillnaden i hundradels sekunder, och
i förhållande till vinnartiden.

Poängen kan bestämmas som summan av placeringspoäng och
resultatbaserade poäng. Vikten för de olika faktorerna kan ändras med koefficienter. Om
man till exempel vill ge placeringarna primär vikt kan man använda ett mycket stort steg,
t.ex. en miljon.

Programmet hanterar poäng som heltal, så
användning av decimaler kräver att poängen beräknas till exempel 100-faldigt,
varvid de innehåller två decimaler. Vid utskriften är det möjligt
att lägga till decimalkommat på rätt ställe med menyvalet
*Formatering / Antal decimaler i poänglistor* i utskriftsformuläret.

#### 11.1.2 Beräkning av totalpoäng

Totalpoängen beräknas som summan av dagspoängen från alla etapper fram till den aktiva etappen. Antalet poäng som beaktas
kan begränsas till färre än antalet etapper, varvid de bästa poängen upp till denna gräns beaktas.

I vissa tävlingsserier är det möjligt att en tävlande deltar i olika klasser vid olika tillfällen. Då finns en möjlighet
att beräkna poängen separat för varje klass, men detta alternativ har tills vidare endast genomförts enligt Aktia Cupens regler.

Poäng kan beräknas både för vanliga klasser
och för klasskombinationer, men samma tävlande kan vid varje tillfälle ges
endast en uppsättning poäng, så varje klasskombination måste behandlas i ett svep
från beräkningen av etapperna till framtagningen av utskrifterna innan poäng som gäller andra
klasser med samma tävlande beräknas.

#### 11.1.3 Beräkning av poäng direkt från resultatet

Programmet beräknar på begäran poängen samtidigt som det
sparar resultatet. För att få rätt poäng på detta sätt får poängen inte
bero på något annat än den tävlandes eget resultat. Den jämförelsetid som används i beräkningen
måste låsas före tävlingen.
Om de slutliga poängen beror på klassens segrartid är poängen som beräknas
under tävlingen ungefär rätt, om segrartiden kan uppskattas
i förväg med rimlig noggrannhet.

I vissa fall kan
formeln för poängberäkningen vara mer komplicerad. En sådan formel kan
eventuellt definieras i menyn i formuläret *Klassuppgifter*.
Poängen som beräknas på detta sätt bestäms genom att ange det resultat som ger noll
poäng samt koefficienter som anger hur snabbt poängen ökar
när resultatet är bättre än denna gräns. Oftast är dock valet *Resultattjänst
/ Beräkna tävlandes poäng* mer ändamålsenligt. Kontinuerlig
poängberäkning tas i bruk genom att markera *Beräkna etappens poäng alltid när resultat
sparas* samt *eventuellt Beräkna totalpoäng alltid när resultat
sparas.* Dessutom kan du be programmet beräkna
totalpoäng även för dem som inte får något resultat på den pågående etappen, antingen
på grund av diskvalificering eller för att de inte startat. Denna möjlighet
finns eftersom poängen från tidigare etapper i vissa fall kan
berättiga till en till och med bra placering.

Markeringen *Använd totalpoäng som totalresultat*
leder till att totalpoängen och poängskillnaden till totalpoängsledaren
visas på speakerns uppföljningsskärmar.

#### 11.1.4 Lagring av formler för poängberäkning

Formler som definierats i formuläret *Poängberäkning* kan
sparas i klassuppgifterna med ett val i formulärets meny. Lagringen
sker på följande sätt:

- Lagringen görs för alla valda klasser.

  - Genom att byta valda klasser kan olika formler
    sparas för olika klasser

    - Formlerna sparas per etapp och innehåller
      både etappens poäng och beräkningen av totalpoäng
      fram till den etappen. För olika etapper måste formlerna sparas i resultattjänstläget
      för respektive etapp.

      - Formlerna för de valda klasserna blir
        sådana som de är definierade
        i formuläret. Formlerna för övriga klasser sparas i sin tidigare form. Skrivningen
        till filen gäller på så sätt alla klasser, eftersom allt sparas i samma
        klassfil.

När programmet startas efter lagringen läser
programmet in formlerna automatiskt och de kan användas via knappen *Beräkna
alla poäng enligt varje klass sparade regel*. Formlerna
visas på skärmen genom att du väljer någon klass för behandling och väljer
*Läs regler från fil* i menyn. Då visar programmet den aktuella klassens
formler på skärmen och lägger till alla klasser som har
samma formler i listan över klasser som behandlas. Därefter kan du ändra formlerna och välja
vilka klasser ändringarna ska gälla.

Från klassuppgifterna kan du också hämta formeldefinitionerna för den föregående
etappen. Hämtningen gäller alla klasser, men
på skärmen visas formlerna för en klass och de klasser som har
samma formler blir valda. När uppgifterna sparas i filen sparas alla klasser, även
de som inte har granskats.

#### 11.1.5 Visning av formler för poängberäkning

Alla beräkningsformler som definierats för olika klasser och olika etapper
kan granskas i en tabell som öppnas med ett menyval i formuläret
för poängberäkning. Kolumnernas betydelse torde framgå genom jämförelse med
fälten i definitionsformuläret. I tabellen
kan formlerna inte redigeras.
