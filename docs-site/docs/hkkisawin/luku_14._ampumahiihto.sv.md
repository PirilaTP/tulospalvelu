# Kapitel 14. Skidskytte

## 14 Skidskytte

### 14.1  Definition av skjutplatser

Resultattjänsten för skidskytte skiljer sig från längdskidåkning när det gäller
hanteringen av straff. Även i hanteringen av jaktstart finns
små skillnader.

I tävlingens [grunduppgifter](2.1_uuden_kilpailun_luominen.md) väljs skidskytte som gren,
och det är bra att även ange standardstorleken på straffet för en bom,
även om detta värde inte slutgiltigt bestämmer straffet.

Antalet skjutplatser samt straffet för en bom
definieras i [klassuppgifterna](2.2.1_sarjatietojen_muuttaminen.md)
separat för varje klass. Detta görs
bäst i redigeringsformuläret för en klass, som för varje etapp
har fält för att ange dessa.

[Jaktstart](10.3_takaa-ajolahto.md)
 kräver egna definitioner i klassuppgifterna
och i valen för noggrannheten i tidshanteringen.

När mellantider tas i skidskytte är programmets
standardval att den 1:a mellantidspunkten omfattar den 1:a skjutplatsen, den 2:a
mellantidspunkten två skjutplatser osv. Detta standardbeteende kan ändras
genom att i definitionen av mellantidspunkter ange antalet
skjutplatser som ingår i varje mellantid

### 14.2  Inmatning av straff

Straff kan matas in både i programmet HkMaalis
korrigeringsfönster och i programmet HkKisaWin, där det går bäst i det
formulär som definierats för detta ändamål. Formuläret kan öppnas
via valet *Resultattjänst* i huvudformuläret.

I formuläret för inmatning av straff hämtar du en tävlande genom att ange
numret och trycka på *Enter*, varvid programmet flyttar till den första
obehandlade skjutplatsen. Programmet använder bindestreck som
tecken på att antalet straff inte har matats in, och även felfria prestationer
måste sparas genom att ange värdet 0. Tangenten *Enter* för från straffältet
till godkännande av inmatningen och från godkännandet till inmatning av ett nytt startnummer. Om
du vill mata in flera straff på en gång måste du använda tabulatorn för att flytta
från ett fält till ett annat (och kombinationen *Shift-Tab* bakåt).
När uppgiften sparas registreras den också i listan över inmatade uppgifter, som
visas i formulärets övre del.

Programmet kan också ta emot straffen från Kurvinens
utrustning. Detta driftläge startas genom att ge programmet
startparametern

```
SAKKO_YHT=n
```

samt

```
SAKKO_LAJI=x
```

om en äldre version än 2 av Kurvinens utrustning
används. För den äldsta versionen är x=0 och för nästa
x=1. Standard är x=2 för nyare utrustning, vilket inte behöver anges
med parametern. Kurvinens utrustningsversioner 1 och 2 skiljer sig från varandra endast
i fråga om överföringshastigheten. Den är 9600 baud i version 1 och 19200 baud i version
2. Programmet väljer hastigheten utifrån ovan nämnda
parameter.

När straffuppgifter kommer från Kurvinens utrustning och
inmatningsfönstret för straff är öppet registrerar programmet de inkomna uppgifterna
i listan över inmatade uppgifter. Uppgifterna registreras i tävlandeuppgifterna även om
fönstret inte är öppet, och funktionen är då likadan i programmen
HkKisaWin och HkMaali. Programmet ger ett felmeddelande när ett fel
upptäcks i ett inkommande meddelande.

Programmen överför uppgifterna från Kurvinens utrustning om skjutresultaten
i nätverket, så både denna funktion och funktionen i avsnitt 14.4 är
tillgängliga även på andra datorer än den som Kurvinens utrustning
är ansluten till.

### 14.3  Uppföljning av straffrundor

I programmets val *Uppföljning av straffrundor*
kan du mata in den tävlandes nummer varje gång en straffrunda fullbordas. Då
visar programmet i ett annat fönster de straffrundor som inte har registrerats
som genomförda.

Programmet
sparar de inmatade straffrundorna i filen SakkoKierr.txt och läser in dem
när formuläret för uppföljning av straffrundor öppnas. Före en ny tävling måste du
kontrollera att denna fil inte finns eller att filen är tom. En felaktigt
inmatad uppgift kan tas bort genom att gå till raden för den och
klicka på *Ångra vald registrering*
.

Filen SakkoKierr.txt är
tillgänglig endast på den dator där inmatningen har gjorts. Uppgiften om antalet registrerade rundor
överförs även till andra datorer, så antalet oregistrerade rundor
kan också visas
på andra
datorer.

### 14.4 Uppföljning av skyttet skott för skott

Programmets val *Uppföljning / Skjutplatser*
öppnar ett formulär där programmet visar resultatet för varje skott, om
Kurvinens utrustning som stöder uppföljning av skott används (version 0
stöder det inte).
