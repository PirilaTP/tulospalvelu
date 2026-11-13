# Bilaga 5. Användning av EmiTag-enheter

## Bilaga 5. Användning av EmiTag-enheter

#### A5.1 Arbetsmodeller

De arbetsmodeller som mina program
stöder bygger på modeller som beskrivs i Emits anvisningar
"Instructions for use Emit Finish Station ECB1" och "emiTag brikkesystem –
Koblingsskema, reservedeler og ekstrautsryr", av vilka den senare främst innehåller
olika schematiska bilder av hur enheterna kopplas samt allmänna
arrangemang.

När det gäller tidtagningen motsvarar arbetsmodellen i huvudsak schemabilden
"emiTag langrenn, enkel" eller "emiTag langrenn, avansert".
För stämplingskontrollen känner jag inte till någon motsvarande dokumentation.

Till datorn ansluts typiskt följande enheter

- målstationen ECB1

  - mellantidsstationen ETS1

    - stämplingskontrolläsaren ECU1 eller MTR5

Enheten ansluts till datorn med en USB-kabel, via en RS485-förbindelse
(strömslinga) med hjälp av en MOXA-USB-omvandlare eller via en RS232-förbindelse.
Även USB-anslutningar syns för programmet som serieportar, men förbindelsens hastighet beror på
anslutningens typ:

- direkt USB-anslutning 115200
  b/s

  - RS485-baserad förbindelse 19200
    b/s

    - RS232-förbindelse 9600
      b/s.

Programmen stöder både direkt anslutning av de ovan
uppräknade enheterna och hämtning av mellantidsuppgifter från Emits
databasserver, som samlar in de mellantidsobservationer som ETS1-stationen förmedlar via
GPRS-förbindelse.

#### A5.2 Programmets parametrar

För att EmiTag-funktionerna ska kunna
användas har nya parametrar lagts till i programmet MaalEmit32:

```
EMITAGy=x
EMITAGy=x/U
EMITAGy=x/C
```

anger serieportens nummer för den förbindelse som används
för enhetens anslutning. y är förbindelsens nummer och
x serieportens nummer.

/U i slutet av parametern
anger att förbindelsen är en snabb USB-förbindelse och
/C en RS485-förbindelse. Annars är hastigheten 9600 b/s.

ECAIKA

anger att som tid för händelsen registreras den tid som ECB1-, ETS1-,
ECU1- eller MTR5-enheten anger och inte datorns tid.

#### A5.3 Styrning av EmiTag-enheter och överföring av minnesuppgifter

I stämplingskontrollskärmens val *Funktioner /
Styrning av emiTag-avläsning* kan du ändra enhetens inställningar och begära att enheten sänder uppgifter som finns i dess minne.

Från denna skärm kan enhetens klocka synkroniseras med datorns
klocka. De flesta enheter måste synkroniseras efter varje omstart.

På skärmen kan du ställa in enhetens driftläge,
som alltid måste vara 0, samt koden, vilket kan behövas
för att identifiera mellantidspunkten, varvid en kod mellan 65 och 239 används. Koderna
240, 243, 250 och 253 begär att enheten arbetar i stämplingskontrolläge. Koderna
240 och 243 hämtar uppgifterna för de 5 senaste dagarna till datorn, dvs. ofta även onödiga uppgifter.
Koderna 250 och 253 hämtar endast den senaste tävlingen och är därför i allmänhet
bättre. Koden 248 används vid tidtagning i mål med ECB1-enheten.

Motsvarande funktioner finns till stor del tillgängliga
i programmet *HkMaali*: via huvudmenyns val
*A)setukset / Emit* går du till enhetens styrskärm,
dit du också kan komma
från stämplingskontrollskärmen med tangenten 'M' (syftar på ordet MTR).

Alla emiTag-läsarenheter sparar de uppgifter de läser
i minnet. Dessa uppgifter som sparats i minnet kan läsas i efterhand.
Att välja vilka uppgifter som ska läsas utifrån meddelandenumren kan vara nästan
omöjligt, eftersom numreringen inte direkt motsvarar antalet lästa stämplingar. Därför
kan det vara enklast att läsa den senaste dagens uppgifter eller alla uppgifter
till en separat fil och därifrån välja de rader som ska behandlas
i valet *Funktioner / Läs uppgifter från fil*
.

#### A5.4 Hämtning av tider från Emits tidsserver

Mellantidsstationen ETS1 kan sända tider
via GPRS-förbindelse till en databasserver som Emit underhåller.

Parametern

```
ETGPRSy=xxxxx,uuuuu,vvvvv,zzzzz
```

eller

```
ETGPRSy=xxxxx/L
```

aktiverar hämtning av mellantider från Emits server baserad på http-protokollet,
xxxxx, uuuuu, vvvvv och zzzzz är serienumren för de högst fyra ETS1-enheter som har
producerat uppgifterna. Tilläggsmarkeringen /L betyder att
starten av dataöverföringen skjuts upp tills den startas separat
från programmet. y
anger förbindelsens nummer för hämtningen av uppgifter.

En annan, men uppenbarligen både långsammare och krångligare, möjlighet är att hämta uppgifterna med SQL-frågor direkt från databasservern. Då är parametern

ETSQLy=xxxxx,uuuuu,vvvvv,zzzzz

När denna parameter används måste en SSH-tunnel öppnas
från resultattjänstens dator till databasservern. Med hjälp av tunneln syns
servern för programmet precis som om den vore en lokal MySQL-server
i den port som anges i tunneldefinitionen (programmets standard är 53306). Denna
funktion styrs från ett formulär som öppnas via valet *Funktioner / Hämta från MySQL-databas* på
tidtagningsformuläret i programmet HkKisaWin. Programmet
öppnar för närvarande inte förbindelsen automatiskt, utan den måste öppnas
med knappen *Öppna anslutning*, när parametrarna är korrekta och tunneln har
öppnats till exempel med programmet plink. Att förbindelsen
har öppnats framgår av att programmet
anger antalet poster som finns på servern.

Användningen av programmet plink
förutsätter en nyckelfil samt uppgifter om den adress och port som förbindelsen ska
upprättas till. Dessa uppgifter har erhållits från Emit och för att de ska kunna användas krävs
tillstånd från Emit. Den exakta kommandoraden för programmet plink kan fås samtidigt som
nyckelfilen. plink är fritt tillgängligt som en del av
*PuTTY*
-programvaran.

När förbindelsen har öppnats kan uppgifter hämtas med knappen
*Hämta*. Normalt används dock
kontinuerlig hämtning, som startar när den väljs. Hämtningen sker
med det definierade hämtintervallet. Tidsgränsen bör ställas in nära tävlingens
faktiska start, så att programmet inte hämtar tider som samma enhet
producerat någon gång tidigare. **Obs. Tidsgränsen ska anges i norsk tid.**

När programmet har hittat en enda post som kommer
från den begärda enheten och är daterad senare än tidsgränsen, anger programmet
ordningsnumret i databasen för den första och den sista post som uppfyller villkoren
och börjar hämta uppgifter. Uppgifterna hämtas normalt i omgångar om högst 100
poster, men detta värde kan ändras. Efter hämtningen
uppdaterar programmet uppgiften om det högsta meddelandenumret för varje enhet. Programmet
riktar senare hämtningar endast till
uppgifter vars meddelandenummer är större än den senast hämtade post som anges
på skärmen. Om fältet av någon anledning innehåller ett för stort nummer hämtas inga meddelanden.
Nyligen inkomna meddelanden kan
hämtas på nytt genom att minska detta nummer.

Den kontinuerliga hämtningen måste avbrytas innan enhetsnumret eller
numret på det senaste meddelandet
redigeras och sedan startas på nytt.

Båda sätten att hämta tider från Emits server kan styras med följande parametrar

ETHAKUVÄLI= xx

Väljer intervallet mellan på varandra följande mellantidshämtningar i tiondels sekunder

ETDATE= vvvvkkpp

Den första dag från vilken uppgifter hämtas, i formen
vvvvkkpp (t.ex. 20131124
). Om den inte anges hämtas endast hämtningsdagens uppgifter.

ETTIME= tt:mm:ss

Klockslag före vilket äldre uppgifter inte hämtas, i formen
tt:mm:ss (T.ex. 13:10:00
). Om det inte anges hämtas alla klockslag.

```
ETHOST=ip:portti
```

anger ip-numret och porten för html-förbindelsen. Om den inte anges
används numren 195.159.103.189:1379

Av programmets övriga parametrar används ofta
parametrarna AIKALUKIJAy=VAINx, där y och x också kan saknas, samt
LÄHDEPISTEET (se [kapitel 5.2](5.2_tunnisteisiin_perustuva_ajanotto.md)).
Tillsammans med parametern LÄHDEPISTEET lönar det sig
oftast att ange parametern AIKALUKIJA=VAIN.

Det behövs alltså alltid antingen parametern EMITAG,   ETGPRS eller ETSQL
. Olika parametrar och/eller flera EMITAG
-parametrar kan användas samtidigt, när förbindelserna har fått olika
nummer.

#### A5.5 Programmets tilläggsfunktioner

Uppgifter som kommer från emiTag-enheter behandlas på samma
sätt som uppgifter som kommer från andra emit-stämplare. Om parametern AIKALUKIJAy=VAINx saknas antar programmet att enheten producerar
stämplingsuppgifter. Tidtagningsuppgifter behandlas inte utan parametern AIKALUKIJA eller ETGPRS
.

Från tidtagningsskärmen kan du med tangenterna *Alt-W* gå
till ett val där hämtningen av mellantider från Emits
server startas eller avbryts, när parametern ETGPRS
har angetts. I valet kan du också begära att alla
uppgifter sänds på nytt. Annars begär programmet att de 10 senaste uppgifterna upprepas
för att säkerställa att alla uppgifter fås utan stora fördröjningar även om
det har förekommit en störning i dataöverföringen.

#### A5.6 Användning i skidtävlingar

Av parametrarna lönar det sig i allmänhet att ange EMITAG, ECAIKA samt ETGPRS, om en GPRS-förbindelse används för överföring av
mellantider. I övrigt följs samma praxis som i andra tidtagningssätt som baserar sig på
brickor, som vid stämpling med Emit-bricka på mållinjen.
(se [kapitel 5.2](5.2_tunnisteisiin_perustuva_ajanotto.md)).

Om startgrinden har anslutits via ECB1-enheten
lönar det sig att ange parametern

PISTEET= MMMLM

eller, om du vill att startande automatiskt härleds utifrån starttiden
inom 3 sekunder,

PISTEET=MMMLM/sarjannimi

Om härledningen önskas för flera klasser skrivs
KAIKKI i stället för sarjannimi.
