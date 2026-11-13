# Bilaga 9. Användning av EmiTag-enheter

## Bilaga 9. Användning av EmiTag-enheter

#### A9.1 Driftmodeller

De driftmodeller som mina program
stöder bygger på modeller som beskrivs i Emits anvisningar
"Instructions for use Emit Finish Station ECB1" och "emiTag brikkesystem –
Koblingsskema, reservedeler og ekstrautsryr", av vilka den senare främst
innehåller olika schematiska bilder över hur enheterna kopplas och allmänna
arrangemang.

För tidtagningens del motsvarar driftmodellen i allt väsentligt schemabilden
"emiTag langrenn, enkel" eller "emiTag langrenn, avansert".
För stämplingskontrollen känner jag inte till någon motsvarande dokumentation.

Till datorn ansluts typiskt följande enheter

- målstation ECB1

  - mellantidsstation ETS1

    - avläsare för stämplingskontroll ECU1 eller MTR5

Enheten ansluts till datorn med USB-kabel, via RS485-förbindelse
(strömslinga) med hjälp av en MOXA-USB-omvandlare eller via RS232-förbindelse.
Även USB-anslutningar syns för programmet som serieportar, men förbindelsens hastighet beror på
typen av anslutning:

- direkt USB-anslutning 115200
  b/s

  - RS485-baserad förbindelse 19200
    b/s

    - RS232-förbindelse 9600
      b/s.

Programmen stöder både direkt anslutning av de ovan
nämnda enheterna och hämtning av mellantidsdata från Emits
databasserver, som samlar in de mellantidsobservationer som ETS1-stationen
förmedlar via GPRS-förbindelse.

#### A9.2 Programmets parametrar

För användning av EmiTag-funktionerna finns startparametrarna:

```
EMITAGy=x
EMITAGy=x/U
EMITAGy=x/C
```

som anger serieportens nummer för den förbindelse som
används för att ansluta enheten. y är förbindelsens nummer och
x serieportens nummer.

Tillägget
/U i slutet av parametern anger att förbindelsen är en snabb USB-förbindelse och
/C en RS485-förbindelse. Annars är hastigheten 9600 b/s.

ECAIKA

anger att den tid som ECB1-, ETS1-,
ECU1- eller MTR5-enheten rapporterar registreras som händelsens tid, och inte datorns tid.

#### A9.3 Styrning av EmiTag-enheter och överföring av minnesdata

I stämplingskontrollskärmens val *Funktioner /
Emit-enheternas anslutningar, emiTag-styrning* kan du ändra enhetens inställningar och be enheten skicka de uppgifter som finns i dess minne.

Från denna skärm kan enhetens klocka synkroniseras med datorns
klocka. De flesta enheter måste synkroniseras efter varje omstart.

På skärmen kan du ställa in enhetens driftläge,
som alltid ska vara 0, samt en kod, vilket kan behövas
för att identifiera en mellantidspunkt, varvid en kod mellan 65 och 239 används. Koderna
240, 243, 250 och 253 begär att enheten arbetar i stämplingskontrolläge. Koderna
240 och 243 hämtar data för de 5 senaste dagarna till datorn, dvs. ofta onödig information.
Koderna 250 och 253 hämtar endast den senaste tävlingen och är därför i regel
bättre. Kod 248 används vid måltidtagning med ECB1-enheten.

Motsvarande funktioner finns till stor del även
i programmet *HkMaali*: via huvudmenyns val
*A)setukset / Emit* (inställningar) går du till enhetens styrskärm,
dit du också kommer
från stämplingskontrollskärmen med tangenten 'M' (syftar på ordet MTR).

Alla emiTag-avläsare sparar de uppgifter de läser
i minnet. Dessa uppgifter i minnet kan läsas i efterhand.
Att välja vilka uppgifter som ska läsas utifrån meddelandenummer kan vara nästan
omöjligt, eftersom numreringen inte direkt motsvarar antalet lästa stämplingar. Därför
kan det vara enklast att läsa den senaste dagens uppgifter eller alla uppgifter
till en separat fil och därifrån välja de rader som ska behandlas
i valet *Funktioner / Läs uppgifter från fil*
.

#### A9.4 Hämtning av tider från Emits tidsserver

Mellantidsstationen ETS1 kan skicka tider
via GPRS-förbindelse till en databasserver som Emit underhåller.

Parametern

```
ETGPRSy=xxxxx,uuuuu,vvvvv,zzzzz
```

eller

```
ETGPRSy=xxxxx/L
```

aktiverar hämtning av mellantider från Emits server baserad på http-protokollet.
xxxxx, uuuuu, vvvvv och zzzzz är serienumren för de högst fyra ETS1-enheter som
har producerat uppgifterna. Tillägget /L innebär att
starten av dataöverföringen skjuts upp tills den startas separat
från programmet. y
anger förbindelsens nummer för hämtningen av uppgifterna.

En annan möjlighet, som uppenbarligen är både krångligare och långsammare,
är att hämta uppgifterna med SQL-frågor direkt från databasservern. Då är parametern

ETSQLy=xxxxx,uuuuu,vvvvv,zzzzz

När denna parameter används måste en SSH-tunnel öppnas
från resultattjänstens dator till databasservern. Med tunneln
syns servern för programmet precis som om den vore en lokal MySQL-server
på den port som anges i tunneldefinitionen (programmets standard är 53306). Denna
funktion styrs från ett formulär som öppnas med valet *Funktioner / Hämta från MySQL-databas*
i tidtagningsformuläret i programmet HkKisaWin. Programmet
öppnar för närvarande inte förbindelsen automatiskt, utan den måste öppnas
med knappen *Öppna anslutning* när parametrarna är rätt och tunneln har
öppnats till exempel med programmet plink. Att
förbindelsen har öppnats framgår av att programmet
rapporterar antalet poster på servern.

Användning av programmet plink
förutsätter en nyckelfil samt uppgifter om den adress och port som förbindelsen
ska tas till. Dessa uppgifter har fåtts från Emit och för att få använda dem krävs
tillstånd från Emit. Den exakta kommandoraden för programmet plink fås samtidigt som
nyckelfilen. plink finns fritt tillgängligt som en del av
programpaketet
*PuTTY*
.

När förbindelsen har öppnats kan uppgifter hämtas med knappen
*Hämta*. Normalt används dock
kontinuerlig hämtning, som startar när den väljs. Hämtningen sker
med det angivna hämtningsintervallet. Tidsgränsen bör sättas nära tävlingens
faktiska start, så att programmet inte hämtar tider som samma enhet
producerat någon gång tidigare. **Obs! Tidsgränsen ska anges i norsk tid.**

När programmet har hittat åtminstone en post som
kommer från den begärda enheten och är daterad senare än tidsgränsen, rapporterar programmet
löpnumret i databasen för den första och den sista posten som uppfyller villkoren
och börjar hämta uppgifter. Uppgifterna hämtas normalt i omgångar om högst 100
poster, men detta värde kan ändras. Efter hämtningen
uppdaterar programmet uppgiften om det högsta meddelandenumret för varje enhet. Programmet
riktar senare hämtningar endast
till uppgifter vars meddelandenummer är större än den senast hämtade post som visas på skärmen.
Om fältet av någon anledning innehåller ett för stort nummer hämtas inga meddelanden.
Nyligen mottagna meddelanden kan
hämtas på nytt genom att minska detta nummer.

Den kontinuerliga hämtningen måste avbrytas innan enhetsnumret eller
numret på det senaste meddelandet
redigeras, och sedan startas på nytt.

Båda sätten att hämta tider från Emits server kan styras med följande parametrar

ETHAKUVÄLI= xx

Väljer intervallet mellan på varandra följande mellantidshämtningar i tiondels sekunder

ETDATE= vvvvkkpp

Den första dagen från vilken uppgifter hämtas, i formatet
vvvvkkpp (t.ex. 20131124
). Om den inte anges hämtas endast uppgifterna för hämtningsdagen.

ETTIME= tt:mm:ss

Klockslag före vilket äldre uppgifter inte hämtas, i formatet
tt:mm:ss (t.ex. 13:10:00
). Om det inte anges hämtas alla klockslag.

```
ETHOST=ip:portti
```

anger ip-numret och porten för html-förbindelsen. Om det inte anges
används numren 195.159.103.189:1379

Av programmets övriga parametrar används ofta
parametrarna AIKALUKIJAy=VAINx, där y och x också kan saknas, samt
LÄHDEPISTEET (se [kapitel 5.2](5.2_tunnisteisiin_perustuva_ajanotto.md)).
Tillsammans med parametern LÄHDEPISTEET lönar det sig
oftast att ange parametern AIKALUKIJA=VAIN.

Det behövs alltså alltid antingen parametern EMITAG,   ETGPRS eller ETSQL
. Olika parametrar och/eller flera EMITAG
-parametrar kan användas samtidigt när förbindelserna har getts olika
nummer.

#### A9.5 Programmets tilläggsfunktioner

Uppgifter som kommer från emiTag-enheter behandlas på samma
sätt som uppgifter från andra Emit-stämplare. Om parametern AIKALUKIJAy=VAINx saknas antar programmet att enheten producerar
stämplingsuppgifter. Tidtagningsuppgifter behandlas inte utan parametern AIKALUKIJA eller ETGPRS
.

Från tidtagningsskärmen kan du med tangenterna *Alt-W* gå till
ett val där hämtningen av mellantider från Emits
server startas eller avbryts, när parametern ETGPRS
har angetts. I valet kan du också begära att alla
uppgifter skickas på nytt. Annars begär programmet att de 10 senaste uppgifterna upprepas
för att säkerställa att alla uppgifter fås utan stora fördröjningar även om
det har förekommit störningar i dataöverföringen.

#### A9.6 Användning i skidtävlingar

Av parametrarna lönar det sig i regel att ange EMITAG, ECAIKA samt ETGPRS, om GPRS-förbindelse används för överföring
av mellantider. I övrigt följs samma praxis som vid andra tagg-baserade
tidtagningssätt, till exempel när man stämplar med Emit-bricka på mållinjen.
(se [kapitel 5.2](5.2_tunnisteisiin_perustuva_ajanotto.md)).

Om startgrinden är ansluten via ECB1-enheten
lönar det sig att ange parametern

PISTEET= MMMLM

eller, om du vill att programmet automatiskt avgör vem som startar utifrån starttiden
inom 3 sekunder,

PISTEET=MMMLM/sarjannimi

Om avgörandet ska göras för flera klasser skrivs
KAIKKI i stället för klassnamnet.
