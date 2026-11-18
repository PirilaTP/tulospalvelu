# Kapitel 12. Användning av MySQL-databas

## 12 Användning av MySQL-databas

### 12.1 Allmänt

En förbindelse till en MySQL-databas kan användas både så att alla uppgifter om de tävlande antingen
skrivs till MySQL-databasen eller läses från MySQL-databasen, och så att
programmet omedelbart sparar varje ändring i de tävlandes uppgifter även
på databasservern.

Databasstödet bygger på databasgränssnittet dbExpress. Med programleveransen följer
alla filer som behövs för att upprätta förbindelsen. Om dbExpress redan används på annat sätt
på datorn kan det uppstå kompatibilitetsproblem via filen dbxdrivers.ini.
Programmet installerar den medföljande filen i samma katalog som programmet HkKisaWin, men
en dbxdrivers.ini som installerats i samband med ett annat program kan förbigå denna fil. De
programkomponenter som används kommer från 32-bitarsversionen av MySQL 5.1, men de kan också ansluta till vissa
andra MySQL-versioner (åtminstone version 5.5).

Förbindelsen till databasen kan öppnas med menyvalet "Filer/MySQL" eller med parametrarna

|  |  |
| --- | --- |
| SQLHOST=host | Anger MySQL-serverns namn (standard 'localhost') |
| SQLDATABASE=database | Anger MySQL-databasens namn (standard 'kilp') |
| SQLUSER=user | Anger MySQL-databasens användare (standard 'kilp') |
| SQLPASSWORD=salasana | Anger lösenordet för MySQL-databasens användare (standard 'hkkisawin') |
| SQLSTART | Begär att MySQL-databasen öppnas automatiskt när programmet startar |
| SQLTALLENNUS | Begär att MySQL-databasen öppnas så att alla ändringar i de tävlandes uppgifter sparas automatiskt |

När omedelbar sparning av ändringar används säkerställer programmet som standard dataöverföringen med filen SQLjonoVarm.dat.
Utifrån denna fil drar programmet slutsatsen vilka uppgifter om tävlande som eventuellt inte har sänts när programmet stängdes.

Överföring av alla uppgifter om tävlande till databasen begärs i valet "Filer/Skriv till överföringsfil" och
läsning från databasen i valet "Deltagare/Hämta från fil". När uppgifter som gäller samma deltagande överförs på nytt
till databasen ersätter den nya uppgiften den gamla.

### 12.2 Databasens struktur

Uppgifterna om de tävlande sparas i tre databastabeller: vosanotot, osuudet
och vtulos. De SQL-kommandon som skapar dessa tabeller finns i
distributionsfilen vkilpsql.txt och de installeras i mappen för tävlingsuppgifterna.

Nycklarna för alla uppgifter innehåller tävlingens kod, som definieras
i tävlingens grundegenskaper, samt platsen för den tävlandes uppgift i filen
KILP.DAT, vilket är en oföränderlig uppgift, om inte ordningen ändras till exempel
genom att skriva till en csv-fil och läsa tillbaka därifrån i en annan ordning.
(Raderade poster kan ändra platserna när man gör så.)

Det är väsentligt att varje tävling som sparas i samma databas får en egen kod.
På så sätt kan man i en enda databas spara till exempel deltagarna och resultaten i alla
motionsorienteringsevenemang under säsongen.
