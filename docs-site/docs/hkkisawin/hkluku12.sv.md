# Kapitel 12. Användning av MySQL-databas

## 12 Användning av MySQL-databas

### 12.1 Allmänt

Förbindelsen till en MySQL-databas kan användas både så att alla tävlandeuppgifter antingen
skrivs till MySQL-databasen eller läses från MySQL-databasen, och så att
programmet omedelbart sparar varje ändring i tävlandeuppgifterna även
på databasservern.

Databasstödet bygger på databasgränssnittet dbExpress. Med programleveransen följer
alla filer som behövs för att skapa förbindelsen. Om dbExpress redan används för annat
på datorn kan kompatibilitetsproblem uppstå via filen dbxdrivers.ini.
Programmet installerar den medföljande filen i samma katalog som programmet HkKisaWin, men
en dbxdrivers.ini som installerats tillsammans med ett annat program kan åsidosätta denna fil. De programkomponenter
som används kommer från 32-bitarsversionen av MySQL 5.1, men de ansluter även till vissa
andra MySQL-versioner (åtminstone version 5.5).

Förbindelsen till databasen kan öppnas med menyvalet "Filer/MySQL" eller med parametrarna

|  |  |
| --- | --- |
| SQLHOST=host | Anger MySQL-serverns namn (som standard 'localhost') |
| SQLDATABASE=database | Anger MySQL-databasens namn (som standard 'kilp') |
| SQLUSER=user | Anger MySQL-databasens användare (som standard 'kilp') |
| SQLPASSWORD=salasana | Anger lösenordet för MySQL-databasens användare (som standard 'hkkisawin') |
| SQLSTART | Begär att MySQL-databasen öppnas automatiskt när programmet startar |
| SQLTALLENNUS | Begär att MySQL-databasen öppnas så att alla ändringar i tävlandeuppgifterna sparas automatiskt |

När omedelbar lagring av ändringar används säkrar programmet som standard dataöverföringen med hjälp av filen SQLjonoVarm.dat.
Utifrån denna fil avgör programmet vilka tävlandeuppgifter som eventuellt inte har skickats när programmet stängdes.

Överföring av alla tävlandeuppgifter till databasen begärs i valet "Filer/Skriv till överföringsfil" och
läsning från databasen i valet "Deltagare/Hämta från fil". När uppgifter om samma deltagande överförs på nytt
till databasen ersätter de nya uppgifterna de gamla.

### 12.2 Databasens struktur

Tävlandeuppgifterna sparas i tre databastabeller: osanotot, osottopv och tulos. SQL-kommandona som skapar dessa tabeller
finns i distributionsfilen kilpsql.txt och de installeras i mappen för tävlingsdata. Nycklarna för alla uppgifter
innehåller tävlingens kod, som anges i tävlingens grundegenskaper, samt tävlandeuppgiftens position
i filen KILP.DAT, vilket är en oföränderlig uppgift om inte ordningen ändras till exempel genom att skriva till en csv-fil
och läsa tillbaka därifrån i en annan ordning. (Raderade poster kan ändra positionerna när man gör så.)

Det är väsentligt att varje tävling som
sparas i samma databas får en egen kod. På så sätt kan man i en
databas spara till exempel deltagarna och resultaten från alla motionsorienteringsevenemang
under säsongen. För närvarande går det inte i programmet att utelämna
till exempel vakanser, så de måste vid behov tas bort i efterhand,
till exempel med ett lämpligt SQL-kommando.
