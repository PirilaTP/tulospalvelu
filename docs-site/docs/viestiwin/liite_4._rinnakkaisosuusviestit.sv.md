# Bilaga 4. Stafetter med parallellsträckor

## Bilaga 4. Stafetter med parallellsträckor

Programvarans standardversion kan också användas
i stafetter med parallellsträckor, som Halikkokavlen och vissa landskapsstafetter.
För att de nödvändiga tilläggsegenskaperna ska tas i bruk ska

- i funktionen *Skapa tävling och grundegenskaper*
  - det maximala antalet parallella löpare anges
    (till exempel 3 i Halikkokavlen)- utrymmet för sträckorna reserveras enligt lagets maximala storlek
      (till exempel 15 i Halikkokavlen)- klasserna definieras så att man på formuläret
    som behandlar en klass
    - anger antalet sträckor som antalet på varandra följande
      sträckor, inte som antalet löpare som hör till laget
      (i Halikkokavlen alltså 7 och inte 15)- anger klassens maximala antal parallella
        löpare (i en del av klasserna kan detta vara 1, varvid klassen i fråga
        är en normal stafettklass)- då blir det möjligt att i de sträckvisa uppgifterna
          ange antalet parallella löpare på varje sträcka. Lagets
          antal löpare är summan av dessa värden- alla andra sträckvisa uppgifter anges så att parallella
            sträckor inte behandlas separat.- I en klass med parallellsträckor måste ändringar som gäller antalet sträckor
      göras på formuläret för en klass, klasstabellen förkastar sådana ändringar
      som skrivs in där.

Programmet använder nästan alltid beteckningar för parallellsträckorna som består av
sträckans ordningsnummer och bokstaven A, B, ... Till exempel är alltså
sträckorna i Halikkokavlen

- 1- 2A, 2B, 2C- 3A, 3B, 3C- 4A, 4B, 4C- 5A, 5B, 5C- 6- 7

I vissa filer används dock en numrering
som motsvarar löparens plats i uppgifterna om de tävlande, varvid till exempel
sträckorna i Halikkokavlen numreras 1–15. Detta gäller bl.a. överföringsfilen
i csv-format för uppgifter om de tävlande, som programmet skriver och läser och då tolkar sträckan
enligt rubrikraden. I stafetter med parallellsträckor och flera klasser används enbart
nummer även på stämplingskontrollskärmen, eftersom sambandet mellan sträckans nummerform och
sträckkoden då i allmänhet inte är detsamma i olika
klasser.

När en klass har parallellsträckor beaktas detta automatiskt
vid beräkningen av resultat, i uppföljningsvyerna och i resultatlistorna. I resultatlistornas
formatering ökas fältens bredder dock inte automatiskt, så
användaren måste ändra formateringen bl.a. för sträckvisa utskrifter;
parallellsträckans beteckning (t.ex. 105-3A) ryms i nummerfältet endast om fältet
är tillräckligt brett. Slutresultat som innehåller alla
sträckor kräver ingen ändring av formateringen.

I uppföljningsvyn och i resultatlistornas rubriker betyder *Öppna*

- i en tävling med parallellsträckor: antalet lag
  som på sträckan i fråga inte har någon tid eller någon markering om diskvalifikation,
  avbrott eller ej start och vars markering på en tidigare sträcka inte har
  stängt sträckan. Senare sträckor för lag som stängts på grund av en markering på en tidigare sträcka
  är alltså inte öppna. På en parallellsträcka (t.ex. 3A, 3B,
  3C) är laget öppet tills sträckans resultat har avgjorts.
- i en vanlig stafett utan parallellsträckor: antalet enskilda
  deltagare som inte har någon tid eller markering om diskvalifikation,
  avbrott eller ej start. Med räknas också de sträckor som har
  stängts på grund av en sådan markering på en tidigare sträcka, om markeringen inte har
  utsträckts till senare sträckor.

## Utskrifter i en tävling med parallellsträckor

*Sträckvisa resultat* skrivs i en tävling med parallellsträckor
ut som en lista per löpare: varje löpare står på en egen
rad i ordning efter sträcktiden, och på parallella sträckor
innehåller nummerfältet lagets nummer och sträckans beteckning (till exempel 105-3A).
Samma tider får samma placering. Talen i listans rubrik (resultat, utgångna,
diskvalificerade, öppna) är i denna lista antal löpare.
I listan per lag (t.ex. *Resultat*) är samma tal antal
lag.

Med valen *Diskade*, *Utgångna*,
*Ej startade* och *Öppna* under *Begränsning av utskrift* innehåller den sträckvisa listan endast löpare med det valda
läget; läget är varje löpares eget läge, och bland de öppna utelämnas
löpare i lag som stängts på grund av en markering på en tidigare sträcka. På en parallellsträcka
avgör läget för den löpare som först gick i mål lagets läge på sträckan
(se nedan), men listan per löpare visar också de andra parallella
löparnas egna diskvalifikationer och avbrott, så dess tal kan avvika från
talen i listan per lag. En lista utan en enda rad
skrivs till html-filen med rubrik och tal; till en textfil och
till skrivaren skrivs ingenting för en tom lista.

Valet *Alla sträckor* skriver alla sträcktider i en lista
i tidsordning. I textfilen är placeringen den placering som uppnåtts på sträckan,
till skrivaren skrivs ett löpande ordningsnummer.

Samma sträckvisa begränsningar fungerar också i en vanlig stafett: en sträckvis
lista *Diskade*, *Utgångna*, *Ej startade* eller *Öppna* skriver
dessa löpare (tidigare skrevs en tom fil ut).

## Den som först går i mål på en parallellsträcka avgör och startar nästa sträcka

I en klass med parallellsträckor kan två av varandra
oberoende regler väljas. Inställningarna görs på två rader i tabellen på formuläret
*Lägg till och redigera klass*, som visas automatiskt när
klassen har minst en parallellsträcka.

1. *Parallellsträckans resultat avgörs av den som först går i mål, de andra
   behövs inte (1=ja)*. Värdet 1 skrivs i kolumnen för den parallella sträckan (t.ex. 3).
   Då väntar man inte på lagets sträckresultat och läge från alla
   parallella löpare (t.ex. 3A, 3B och 3C), utan de bestäms genast
   när den *första* av dem kommer i mål.
2. *Sträckan startar när den första löparen på föregående parallellsträcka går
   i mål (1=ja)*. Värdet 1 skrivs i kolumnen för den sträcka (t.ex. 4)
   som följer efter parallellsträckan. Sträckans starttid är då den första
   parallella löparens måltid. Om sträckan själv också är parallell
   startar alla dess platser tillsammans.

Reglerna kan väljas separat:

| Den första avgör (sträcka 3) | Startar från den första (sträcka 4) | Effekt |
|---|---|---|
| ja | ja | Ungdomsjukolas regel: den första avgör resultatet och startar nästa sträcka |
| nej | ja | Nästa sträcka startar från den första, men lagets sträckresultat och läge kräver resultat från alla parallella: sträcktiden är den långsammastes tid och en löpares diskvalifikation eller avbrott diskvalificerar laget eller gör att det utgår |
| nej | nej | Normal parallellsträcka: resultatet väntar på alla, nästa sträcka startar från lagets sträckresultat (den långsammaste) |
| ja | nej | Den förstas tid är lagets sträcktid; en vanlig nästa sträcka (med en löpare) startar ändå från denna tid. Om nästa sträcka själv är parallell startar dess första plats (t.ex. 4A) från den första, den andra (4B) från den som kom i mål som andra och den tredje (4C) från den som kom i mål som tredje |

Om sträckan har fått en egen masstarttid (*MassStart*) har den företräde
och ingen av reglerna ändrar starttiden.

Den gemensamma inställning (*FirstFinishStarts*) som fanns i gamla klassfiler läses
automatiskt in som båda reglerna (på sträckan avgör den första och
nästa sträcka startar från den), så tidigare tävlingar fungerar som förut.
Vid sparning skrivs inställningarna till filen *KilpSrj.xml* med de sträckvisa
taggarna *OnlyFirstFinishCounts* och *StartsAtPreviousFirstFinish*.

När den som först går i mål avgör på sträckan:

- Som lagets sträckresultat och placering registreras tiden för den parallella
  löpare som först kommer i mål — man väntar inte längre på de andra parallella
  löparnas tider.
- Om nästa sträcka använder start från den förstas målgång startar samma
  måltid också lagets starttid på nästa sträcka, dvs.
  växlingen sker genast när den första parallella löparen kommer in, inte först
  när alla tre har växlat.
- Om den löpare som först gick i mål senare diskvalificeras ärver lagets
  sträckresultat denna diskvalifikation — den ersätts inte med en annan
  parallell löpares (t.ex. en som gick i mål senare) godkända
  tid.
- Diskvalifikation eller avbrott för den löpare som först gick i mål är lagets
  läge på sträckan, även om de andra parallella löparna är godkända.
  Diskvalifikation eller avbrott för de andra parallella löparna påverkar däremot inte
  lagets resultat när den som först gick i mål är godkänd.
- På formuläret *Emit-uppgifter* visas anmärkningen "FÖRST I MÅL. Diskvalifikation
  (DSQ) eller avbrott (DNF) avgör lagets resultat." när det på sträckan
  är den som först går i mål som avgör, löparen är först i mål och
  valet under Kontroll är *Förslag*. Anmärkningen visas inte för andra parallella
  löpare och inte vid andra lägen.
- Avgörande är den första händelse som har en tid. Diskvalifikation eller
  avbrott avgör lagets sträcka endast om en tid har registrerats för den
  (i måltidsfältet): då förblir den avgörande, även om någon annan
  senare går i mål som godkänd. En diskvalifikation eller ett avbrott utan tid
  behandlas inte som lagets resultat när någon annan fortfarande är
  ute på banan — sträckan förblir öppen. Om alla parallella löpare är
  ute ur tävlingen och ingen har en tid får laget den sämsta markeringen.
- En parallell löpare för vilken inget namn har registrerats tas med om
  han eller hon har en Emit-kod eller en tid.
- När den första måltiden en gång har bestämt starten för nästa sträcka
  förblir den i kraft, även om programmet senare skulle beräkna en ny
  masstarttid för klassen — masstarten öppnar alltså inte en redan låst start på nytt.

Funktionen syns också i uppföljnings- och resultatvyerna:

- I vyn *Ställning vid punkt* visas namnet på den avgörande (den som först gick i mål)
  löparen i fetstil på parallellsträckans rad; de andra parallella
  löparnas namn visas i normal stil. Filtren *1 saknas*
  och *2 saknas* döljs automatiskt när den sträcka som granskas
  använder denna inställning, eftersom antalet "saknade" inte längre är en betydelsefull
  uppgift efter att den första tiden har avgjort resultatet; filtret
  *Hel sträcka* fungerar fortfarande.
- I mellantidstabellen i vyn *Lagdata* (*Visa av mellantider* → *Resultat*)
  visas lagets gemensamma resultat endast i kolumnen för den avgörande löparen;
  i kolumnerna för de andra parallella löparna visas 00:00:00. Om du vill
  se varje parallell löpares egen personliga mellantid
  väljer du *Visa av mellantider* → *Str.res*; i detta läge går tabellen
  dock endast att läsa, inte att redigera.
