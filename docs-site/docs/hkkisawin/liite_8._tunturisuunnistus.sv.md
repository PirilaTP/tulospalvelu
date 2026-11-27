# Bilaga 8. Särdrag vid fjällorientering

## Bilaga 8. Särdrag vid fjällorientering

Fjällorientering skiljer sig från andra tävlingar i det
avseendet att man i de flesta klasser orienterar i par och båda
medlemmarna i paret måste stämpla vid alla kontroller.

I tävlingens grundegenskaper anges grenen
orientering och preciseringen parorientering. Då förväntar sig programmet att
båda brickorna läses in för alla som har fått två Emit-koder.
Den som sköter resultattjänsten måste se till att inga koder saknas, eftersom slutsatsen
dras för varje deltagare separat och inte utifrån klassen.

Det lönar sig att skriva den ena medlemmens namn i
förnamnsfältet och den andras i efternamnsfältet. I övrigt finns det skillnader endast i stämplingskontrollen.

Om online-tidtagning används bör du ange
parametern UUSINAIKA med en tillräckligt stor övre gräns, så att den senare
deltagarens tid gäller även när paret av någon anledning passerar
mållinjen vid klart olika tidpunkter.

Vid stämplingskontrollen godkänner programmet inte prestationen
utifrån den första brickan när två Emit-koder har angetts. Likaså
sker tidsberäkningen utifrån stämplingarna först när båda brickorna har lästs in,
och då väljs den sämre tiden oberoende av inläsningsordningen. Denna
funktion gäller endast om inga tider har registrerats före stämplingskontrollen.
