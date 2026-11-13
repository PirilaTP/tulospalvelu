# Bilaga 1. Startparametrar

## Bilaga 1. Startparametrar

Programmens funktion styrs med hjälp av startparametrar.
När man arbetar på en enda dator behövs för många grundfunktioner inga
startparametrar alls, men de är nödvändiga bl.a. när datorer kopplas
samman för att arbeta tillsammans via nätverk. Parametrarnas namn kan
skrivas med stora eller små bokstäver, liksom de värden som anges med
parametrarna. Endast i några textfält, såsom när rubriken
definieras, behåller programmet små bokstäver; i alla andra
fall ändrar programmet bokstäverna till versaler före användning.

För redigering av konfigurationsfiler finns ett
[formulär](1.3_toimintatilojen_konfigurointi.md)
, som underlättar
de flesta definitionerna. I tabellen har de parametrar som kan definieras
med konfigurationseditorn markerats med asterisker. \*\*\* betyder
att parametern kan definieras fullständigt, \*\*
och \* att parametern kan definieras,
men att inte alla dess underdefinitioner kan göras.

### A1.1 Grundläggande och blandade parametrar

|  |  |  |
| --- | --- | --- |
| CFG=tied.nimi |  | Anger den fil från vilken övriga parametrar läses. Standard är LASKENTA.CFG . Denna parameter används oftast på kommandoraden eller i genvägens inställningar. |
| VAIHE=n PÄIVÄ=n | \*\*\* | Anger tävlingens etapp i tävlingar med flera etapper. När n är bokstaven V bestäms etappen utifrån filen vaihevalinta.cfg. |
| IKKUNAT IKKUNAT=xxxx | \*\*\* | Begär att positioner och inställningar för de formulär som används läses från filen xxxx. Om inget filnamn anges används som standard namnet ikkunat.xml |
| IKKUNAOTS=xxxx | \*\*\* | Texten xxxx skrivs i programfönstrets titelrad |
| ÄÄNI=x | \*\*\* | där x är 0, 1 eller 2. ÄÄNI=0 tar bort alla ljudsignaler, ÄÄNI=1 aktiverar ljudsignal vid fel- och varningssituationer och ÄÄNI=2 aktiverar även en lägre påminnelseton för den tävlande som placerar sig först eller sist i klassen. Standardvärde 1. |
| LISÄÄ=xxx | \*\*\* | där x är det största antal tävlande som kan läggas till under en session. Standardvärde är det totala antalet tävlande 5000 och tillägget 1000, om antalet överstiger 5000. |
| SAKOT=xxx |  | Enhet i sekunder som används vid hantering av straff, handikapp och tilläggstid. **Användning av parametern bör undvikas, och straffens storlek ska definieras i tävlingens egenskaper samt i uppgifterna för varje klass med straff.** |
| JUOKSUILM | \*\*\* | i korrigeringsfunktionen börjar man från namnfältet (annars från fältet för diskvalificeringsuppgift). I programmet HkKisaWin påverkar den anmälningsformulärets standardinställningar. |
| KUNTOSUUNNISTUS=x | \*\*\* | där x är 1, 2 eller 3, ställer in programmet WinKisaHk att arbeta i motionsorienteringsläge |
| VAKANTTIALKU=nnnn | \*\*\* | Anger det område av lediga nummer som används i samband med föregående. Samma startnummer används också vid omnumrering av EMIT-koder, om detta inte upphävs med parametern EMITALKU=nnn. |
| LISÄVAKANTTIALKU=nn | \*\*\* | Anger den nedre gränsen för nummerområdet för automatiskt skapade vakanser (som standard skapas de i nummerområdets övre ände). |
| VAKANTTILUKU=nnn | \*\*\* | Anger antalet vakansposter som programmet automatiskt skapar i en ny deltagarfil vid orientering (kräver användarens godkännande). Standard är 20. För motionsorientering passar ett väsentligt större antal (t.ex. 1000). Vakanserna skapas i den övre änden av det nummerområde som används (i regel slutande på 9999), om inte parametern LISÄVAKANTTIALKU har angetts när de skapas, varvid numren börjar från det angivna värdet. |
| SEURATIEDOT=x | \*\*\* | där x är H eller E. Anger omfattningen av hanteringen av klubbuppgifter. När x=H används sökning i klubbregistret som hjälp. När x=E visas klubbuppgifter inte alls, till exempel i stämplingskontrollfönstret. |
| HENKILÖTIETOKANTA=xx | \*\*\* | Anger persondatabasfilen. xx är det fullständiga namnet på filen som ska användas, inklusive katalogsökväg. Standard är henkilot.csv i tävlingens mapp. |
| SEURATIEDOSTO=xx | \*\*\* | Anger klubbregisterfilen. xx är det fullständiga namnet på filen som ska användas, inklusive katalogsökväg. Samtidigt aktiveras sökning i klubbregistret. Standard är seurat.csv i tävlingens mapp. |
| KUNTOLÄHTÖ | \*\*\* | Aktiverar ett förenklat formulär lämpligt för anmälan när funktionen *Stämplingskontroll* väljs. |
| KUNTOMAALI | \*\*\* | Väljer valet *Målets dator* på motionsorienteringens stämplingskontrollskärm. |
| VAHVISTAAINA=EI | \*\*\* | Anger att en person som hittats i databasen inte behöver bekräftas i motionsorienteringsläget. |
| SULKUSALASANA | \*\*\* | Gör det möjligt att stänga av programmet på distans |

#### Parametrar som endast gäller programmet HkMaali

|  |  |  |
| --- | --- | --- |
| BOOT BOOT=Mn | \*\*\* | Programmet startas utan att fråga efter bekräftelser. Tillägget =Mn begär att tidtagningsskärmen öppnas omedelbart (valet *Maali*). Man måste separat se till, till exempel med parametrarna AIKAERO, NÄPPÄIN, AJAT=/S och COMFILE=S, att programmet hoppar över eventuella andra frågor. |
| RIVIT=25 | \*\*\* | Öppnar programmet i ett fönster med 25 rader |
| KIRKAS | \*\*\* | Skärmen använder ljus intensitet |
| VÄRIT=xxx | \*\*\* | Ändrar konsolfönstrets färger (implementeringen är tills vidare bristfällig) |
| TARKNO=n |  | Styr användningen av kontrollsiffra. n = 0: används inte, n = 1: används utom i tidtagningsfunktionen, n = 2: används alltid (standard 0). |
| KUNTO SUORASYÖTTÖ | \*\* | Avsedd för motionsorientering. Tillåter att resultat anges direkt när en tävlande läggs till i programmet HkMaali. |
| FISKOODI |  | Aktiverar hantering av FIS-kod och beräkning av FIS-poäng. Behövs inte i programmet HkKisaWin. |

### A1.2 Parametrar för dataöverföring

|  |  |  |
| --- | --- | --- |
| YHTEYSy=xxxx (tai COMy=x) | \*\*\* | aktiverar dataöverföringsförbindelse nummer y mellan två PC. Om y saknas är förbindelsens nummer 1. xxxx anger överföringssättet |
| BAUDy=xxxx,p,b,s | \*\* | Anger egenskaperna för serieportsförbindelse y. xxxx är serieportsförbindelsens överföringshastighet, p paritet (n, e eller o), b antal databitar och s antal stoppbitar. Standardvärdet är 9600,n,8,1 och värden enligt standard kan utelämnas i slutet. |
| PORTBASE=xxxx | \*\*\* | Den konstanta delen av UDP-portens nummer, till vilken förbindelsens nummer adderas. Standardvärdet är 15900. |
| VÄLITÄy=EI | \*\*\* | Förbjuder programmet att vidarebefordra alla eller en del av de inkommande meddelandena till en annan förbindelse |
| VARASERVER | \*\*\* | Anger att datorn startas i ett läge där den inte skickar meddelanden |
| KONE=xx | \*\*\* | Datorns identifierare är xx (identifierare med två tecken), som visas i andra änden av dataöverföringsförbindelsen. |
| COMFILE COMFILE=S | \*\*\* | Dataöverföringen buffras i en diskfil. 'S' tar bort frågan om att radera filen vid start. Som standard buffring med fråga. |
| COMFILE=EI | \*\*\* | Tar bort buffringen av dataöverföringen i diskfil. |
| MAXYHTEYS=n | \*\*\* | Största antal förbindelser. Påverkar endast strukturen i filen COMFILE.DAT. Ignoreras i regel om COMFILE.DAT redan innehåller meddelanden. |
| TCPTULOS=x |  | Begär att totalresultatet i en tvådagarstävling skickas till TCP-förbindelsen utöver eller i stället för dagens resultat. |
| FILETCPy=xxxx |  | Startar automatisk sändning av XML-filer med jämna mellanrum |
| XMLSARJA=xxx |  | Väljer klass för föregående funktion |
| TCPAUTOALKU |  | Begär att automatisk TCP-överföring av filer startas när programmet har startat |
| NAKVIIVE=xx UDPJARRU=xx UDPVIIVEET=xxx |  | Parametrar för finjustering av dataöverföringen (behövs i regel inte) |
| MAALILEIMASIN=EI | \*\*\* | I sträcktidsposten tas tiden för den sista stämplingen med, även om den enligt banuppgifterna är målstämplarens tid, som man inte vill ha i online-resultattjänsten vid sidan av den egentliga måltiden. |
| MONITORI MONITORI=xxxx |  | Begär att programmet regelbundet skickar meddelanden som berättar om dataöverföringens status. Programmet InstanssiMonitori kan med hjälp av dessa meddelanden följa hela resultattjänstnätverkets status. Om preciseringen xxxx saknas skickas meddelandena till nätverkets broadcast-adress och standardporten 14900. Preciseringen xxxx innehåller kombinationen ip-nummer:port |
| MONITORVÄLI=xx |  | Begär att intervallet för övervakningsmeddelanden ändras till xx sekunder (standard 10 s). (Observera det saknade 'I'.) |
| LÄHKELLOy=xx | \*\*\* | Begär att klockslaget skickas till förbindelse y med xx sekunders mellanrum. (Avsett för synkronisering av klockan i vissa enheter.) Till parametern kan tillägget /VAIN fogas, varvid endast klockslag skickas till förbindelsen och förbindelsen i övrigt behandlas som enkelriktad. |
| LÄHTIEDOSTOT LÄHTIEDOSTOTy=x | \*\* | Aktiverar filöverföring via resultattjänstnätverket. Det första alternativet gäller alla UDP-förbindelser, det senare endast förbindelse y. Om x = 0 upphävs användningen för denna förbindelse. /K som lagts till i slutet av kommandot anger att filer kan överföras även från andra platser än under tävlingskatalogen. |

### A1.3 Parametrar för utskrifter

|  |  |  |
| --- | --- | --- |
| LISTA LISTA=nimi | \*\*\* | nimi är skrivarens eller filens namn. Utan hela parametern är standard NUL, dvs. ingen utskrift. Parametern LISTA utan angivet namn avser standardskrivaren |
| LOKI(=tnimi) | \*\*\* | begär att en loggfil skrivs till en diskfil eller till skrivare |
| MERKIT=xyy |  | ändrar listskrivarens teckenkoder (x = G, I, R, A, W, P eller L), yy anger vilka filer eller skrivare koderna gäller. Standard är G, dvs. användning av Windows skrivardrivrutiner. W lämpar sig för utskrift till textfil. Övriga alternativ ska inte användas, om man inte undantagsvis vill förbigå Windows skrivardrivrutiner. |
| MUOTOILU(=tied.nimi) | \*\*\* | begär att formatinställningarna för resultatlistan läses från en diskfil när programmet startar. Om inget filnamn anges antas namnet TULLUET.FMT. |
| AUTO(=xxxxx) | \*\*\* | Startar automatisk utskrift direkt när programmet har startat. xxxx innehåller parametrar som styr utskriften. |
| VAADITARKASTUS | \*\*\* | En klass anses färdig först när alla har fått någon kontrollmarkering och alla godkända har ett resultat. När klassen är färdig producerar den automatiska utskriften en resultatlista, även om övriga regler inte genast skulle göra det. |
| NOLLAKÄRKI | \*\*\* | I resultaten skrivs tiden '00.00.00' i differenskolumnen för vinnaren. |
| TAKAA-AJO | \*\*\* | Mellantiderna dag 2 visas som totaltider som inkluderar tiden från dag 1 |
| HTML=tied/xx HTML=tied/xx/S MHTML=xx | \*\*\* | Skriv en resultatlista i HTML-format automatiskt med xx sekunders mellanrum. /S anger att utskriften görs till många klassvisa filer. M syftar på mobilformatering. |
| HTMLTITLE=nimi | \*\*\* | Namnet på HTML-sidan som skapas |
| HTMLOTS=otsikko | \*\*\* | Rubriken på HTML-sidan som skapas |
| HTMLSÄILNIMI | \*\*\* | Ordningen mellan förnamn och efternamn ändras inte i HTML-utskrifter |
| XML=tied/xx | \*\*\* | Skriv en resultatlista i XML-format automatiskt med xx sekunders mellanrum. |
| KOMENTO=xxxxx | \*\*\* | Kommando som programmet kör efter den automatiska filutskriften |
| DESEROTIN=x |  | Väljer tecknet 'x' (komma eller punkt) som decimaltecken i xml-utskrifter. |
| KILPKDI=xxx |  | Anger tävlingens kod som skrivs ut i vissa filutskrifter. Utgående, definiera som etappspecifik uppgift för tävlingen. |
| LÄHETÄCR=EI |  | Utelämnar radbrytningstecknet när sidbrytningstecken skickas till skrivaren |

### A1.4 Programmets olika tidtagningsfunktioner styrs med parametrar

|  |  |  |
| --- | --- | --- |
| SYÖTTÖ |  | Anger att funktionen M)aali i programmet HkMaali används för att ange tider genom inmatning i stället för tidtagning. |
| REGNLYx=n/..... | \*\*\* | Anger att Regnlys målklocka används för tidtagning |
| ALGE=n/..... | \*\*\* | Anger att Alges målklocka Timer S3 används för tidtagning |
| ALGE4=n/..... | \*\*\* | Anger att Alges målklocka Timer S4 används för tidtagning |
| COMET=n/..... | \*\*\* | Anger att Alges målklocka COMET används för tidtagning |
| TIMYx=n/..... | \*\*\* | Anger att Alges målklocka Timy används för tidtagning. x anger enhetens nummer (standard: 1). |
| ASC=n | \*\*\* | Anger att Alges startklocka ASC1 används för tidtagning |
| AIKAERO=xxxx | \*\*\* | Anger i sekunder det klockslag då tidtagningsenhetens tid är 0. Parametern gör att frågan om tidsskillnad hoppas över. |
| KELLONO | \*\*\* | Anger att nummer som kommer från klockan tolkas som den tävlandes nummer. (REGNLYNO, COMETNO och TIMYNO fungerar på samma sätt) |
| TUNNKOODIT(=E) |  | Aktiverar eller avaktiverar användningen av identifieringskod |
| REGNLYVIIVE=xx |  | Bestämmer intervallet mellan på varandra följande förfrågningar när RTR-målklocka används |
| KELLOBAUD=xxxx | \*\*\* | Anger målklockans överföringshastighet |
| KELLO\_ESTO=xx |  | Anger i hundradels sekunder den tid inom vilken ingen annan tid från målklockan sparas |
| AIKA\_COM=x AIKA\_ESTO=x | \* | Parametrar för tidtagning via seriegränssnitt. Se motsvarande kapitel. |
| LÄHAIKAy | \*\*\* | Anger att förbindelse y också används för överföring av tidtagningsdata. y är förbindelsens nummer som i parametern YHTEYSy. |
| AJAT=/S AJAT=tied.nimi/S | \*\* | Anger tidtagningsfilens namn. '/S' tar bort frågan om att behålla filen (behåller utan att fråga). |
| PAKOTALAIKA |  | Förhindrar användning av längdskidåkningens 3-sekundersregel vid hantering av starttider. Tillåter också att starttiden sparas från en tidtagningspunkt som i filen lahdepisteet(x).lst har markerats med bokstaven L som punkt (se LÄHDEPISTEET). Används t.ex. när starttiden läses från en RFID-läsare. |
| PAKOTALAIKARAJA=n |  | Används endast tillsammans med parametern PAKOTALAIKA. Anger i sekunder den största tillåtna skillnaden mellan pakotalaika-registreringen och en redan sparad starttid. Om skillnaden är större än n registreras pakotalaika inte på nytt. Standardvärde 0, dvs. ingen begränsning. |
| LÄHTÖIKKUNA=x |  | Ändrar tidsgränsen för "3-sekundersregeln" till x s. |
| LAJAT LAJAT=tied.nimi | \*\* | Startgrindens tider sparas i en annan fil än övriga tider |
| TAUSTA-AIKA |  | Gör det möjligt att öppna en andra tidskö i HkMaalis lilla fönster |
| AIKAKORJAUSy=xx |  | Anger en korrigering med vilken tiderna i kö y ändras automatiskt |
| LÄHTÖRYHMÄ=n |  | Anger storleken på en grupp som startar samtidigt, för att underlätta registreringen av starttiden från klockan för hela gruppen |
| PISTEET=xxxx | \*\* | Anger användningsändamålen för olika tidtagningssätt |
| JONOT=xxxx | \*\*\* | Anger att tagna tider delas upp i två eller flera köer |
| MAXJONO=n | \*\*\* | Anger antalet tidsköer som hanteras. (Behövs inte när antalet framgår av parametern JONOT.) |
| JONOPISTEy=x |  | Anger vilka resultat som fås från de olika tidtagningsköerna |
| VERTRAJAT=xxx |  | Ställer in jämförelsegränser för jämförelse av två tidtagningsköer |
| NÄPPÄIN=mmm,nnn | \*\*\* | Anger den tangent som används för tidtagning. (Endast i programmet HkMaali). |
| VAVUOROT VAVUOROT=sss/a/y | \*\*\* | Använder vid fastställandet av mellantidspunkt enbart tidens ordningsnummer eller den tid som förflutit sedan föregående registrerade mellantid (alltså inte tiden från start). sss anger i sekunder den period under vilken inkomna tider behandlas som en, a och y är nedre och övre gränser för de punkter som tillåts i växlingen. |
| VAINALARAJAT VAINALARAJAT=EI |  | Ändrar sättet på vilket parametern VAVUOROT använder mellantidernas nedre gränser |
| UUSINAIKA |  | Av tider som tagits för samma punkt gäller den senaste (inte den första) |
| VAINVÄLIAJAT | \*\*\* | Förhindrar att den automatiska slutledningen markerar tiden som slutresultat. |
| ENNAKOI | \*\*\* | I HkMaalis tidtagningsfunktion kan flera tider matas in i förväg (menyval i programmet HkKisaWin) |
| ESTÄMUUTOS |  | Förhindrar ändringar på tidtagningsskärmen |
| ESTÄYLIM |  | Förhindrar att en tidtagningsrad sparas när den tävlande redan har motsvarande resultat |
| ESTÄHAAMUT=xx |  | Förhindrar att tiden registreras om resultatet är under xx minuter eller över 20 timmar, vilket motsvarar en negativ tid på under 4 timmar |
| LISÄÄEDELLE |  | I HkMaalis tidtagningsfunktion hamnar en rad som läggs till med tangenten F2 före den markerade tiden och inte efter. |
| SALLITOISTO |  | Begär att på varandra följande tidtagningsmeddelanden som verkar vara upprepningar sparas |
| TUNNJONO TUNNJONO=y |  | Anger att den taggbaserade tidtagningskön y används endast för att identifiera den tävlande. Används när tiden tas till exempel med fotocell. Köns standardvärde är 2. När detta används räcker den kortare formen. |
| TUNNRAJAT=a/y |  | Tidsgränser i sekunder som hör till föregående. a är ofta negativ och anger då att tiden kan komma något före identifieringsuppgiften. y anger den maximala tiden från identifieringsuppgiften till den tagna tiden. |
| KAKSITUNN | \*\*\* | Programmet identifierar två taggkoder för samma etapp, vilka ger samma resultat. |
| YHTEYSAJAT VA-AJAT VAIKAVERT=xxx ILMTUNT(=EI) | \* | Parametrar för hantering av tidtagningsdata som kommer via dataöverföringsförbindelse. Används bl.a. när programmet SendECodes används. Parametern YHTEYSAJAT (eller VA-AJAT) aktiverar denna funktionalitet. |
| LÄHECODEy LÄHECODEy=VAIN | \*\*\* | Anger att tidtagningsdata som tagits emot på grund av parametern YHTEYSAJAT skickas till förbindelse y. y är förbindelsens nummer som i parametern YHTEYSy. Tilläggsdefinitionen VAIN innebär att inga andra meddelanden skickas till denna förbindelse. |

### A1.6 Specialparametrar för skidskytte

|  |  |  |
| --- | --- | --- |
| RACPISTEET | \*\*\* | När Regnlys klocka RTR2 och RAC-lådan används identifieras skjutplatsen enligt RAC-lådans ingång |
| SAKKO\_COM=n SAKKO\_YHT=n | \*\*\* | Programmet tar emot straffuppgifter från Kurvinens utrustning eller från en inmatningsplats som emulerar den via porten COMn. |
| SAKKO\_BAUD=nnnn |  | Parametrar för definition av dataöverföringen när parametern SAKKO\_COM används. |
| SAKKO\_LAJI=n | \*\*\* | Anger versionen av Kurvinens utrustning som används. n=0 för den äldsta utrustningen, standard: n=2. |

### A1.7 Parametrar för Emit-funktioner

|  |  |  |
| --- | --- | --- |
| EMIT | \*\*\* | Startar hanteringen av Emit-data utan andra funktioner. Behövs inte när t.ex. LUKIJA har definierats. |
| EMITANALYYSI | \*\*\* | Startar kontinuerlig analys av sträcktider. |
| LUKIJAx=n LUKIJAx=n/i/a | \*\* | Uppgifterna läses direkt från avläsaren till serieporten COMn. x behövs endast om det finns flera avläsare eller MTR-enheter. |
| EMITAGx=n EMITAGx=n/u | \*\*\* | Uppgifterna läses från en emiTag-enhet. /u anger att USB-serieportens hastighet är 115200 b/s |
| MTRx=n MTRx=n/i/a | \*\* | Uppgifterna läses från MTR-enheten till serieporten COMn. x behövs endast om det finns flera avläsare eller MTR-enheter. När överföringen sker från filen EMIT\_IN.DAT är n 'T'. |
| EMITKELLO=n EMITKELLO=n/w/i/a |  | Uppgifterna läses från klockan RTR2 till serieporten COMn |
|  |  | **Parametrar för stämplingskontroll** |
| VAADIAIKA | \*\*\* | Programmet påpekar vid kontroll av stämplingar om den tävlande ännu inte har eller automatiskt får en tid (används främst när tiden bestäms utifrån stämplingen) |
| LAINAT | \*\*\* | Påpeka om hyrbricka med ett meddelande som ska kvitteras |
| LAINALUETTELO LAINALUETTELO=VAIN | \*\*\* | Hyrbrickornas nummer fås från en fil. Om VAIN har angetts används inte hyrmarkeringarna i tävlandeuppgifterna |
| LÄHEMITn[=I/O/V] | \*\*\* | Emit-data som lästs från brickan överförs via förbindelse n. Om =I eller =O finns i slutet är överföringen av Emit-data enkelriktad, = V anger att endast separat begärd sändning av en enskild post tillåts. |
| LÄHEMVAn | \*\*\* | I stället för normala Emit-data skickas de mellantider som beräknats utifrån dem. |
| AUTORAP | \*\*\* | Begär att uppgifterna för varje inläst bricka skrivs ut automatiskt |
| AUTORAP=x | \*\* | x=1, H eller S. Skriver ut en del av ovan nämnda rapporter (inte godkända) |
| COMAUTORAP COMAUTORAP=H | \*\*\* | AUTORAP-funktionen gäller på den mottagande datorn. Om H har angetts gäller den endast förslag till diskvalificering. Parametern anges på den dator där utskriften sker. |
| RAPORTTIHAK=xxxx |  | Rapporter om hanteringen av diskvalificeringar skrivs till katalogen xxxx ([kapitel 6.16](6.16_hylkaysesitysten_kasittely.md)) |
| ANNASEUR |  | När frågan om nytt startnummer upprepas erbjuds alltid nästa nummer. |
| AUTOSARJA | \*\*\* | Byt klass vid inläsning av Emit-bricka till den som motsvarar stämplingarna |
| AUTORATA | \*\*\* | Använder en funktion liknande AUTOSARJA för banor som inte är klasspecifika. |
| RASTIVATULOSTE | \*\*\* | Startar programmet i ett läge där inläsning av brickan ger en sträcktidsutskrift ([kapitel 6.8](6.8_rastivaliaikatulosteet.md) ) |
| KARTTA=xxxx | \*\*\* | Kartfil som används som stöd vid klagomuren ([kapitel 6.12](6.12_kartan_kaytto_itkumuurilla.md) ) |
| MAALILEIMASIN=EI | \*\*\* | Begär att både tiden för den sista stämplingen och slutresultatet tas med i sträcktidslistorna, även om båda ofta avser tiden vid mållinjen. Påverkar sträcktidsutskrifterna och de uppgifter som skickas till onlinetjänsten. |
| SIVUJAKO=n |  | Emit-mellantiderna för n tävlande skrivs ut tätt på samma ark. |
| EMITVASIVU=l/m/n |  | Mellantidsutskriftens radlängd=l, antal rader m och av dessa lämnas de n översta tomma. |
| TARKRAPOHITA=n |  | Begär att n tomma rader lämnas överst i stämplingskontrollrapporten, till exempel för arrangörens logotyp. |
| LÄHTÖVERT=xxx |  | Programmet påpekar om Emit-brickans nollställningstidpunkt avviker från den lottade starttiden med mer än xxx s. |
| LUESUORAAN |  | Begär att MTR:s data töms utan avbrott med automatiskt tillägg av poster |
| EMITALKU=nnnn | \*\*\* | Anger det nummervärde från vilket en ledig Emit-kod söks när programmet byter koden automatiskt. |
| VALONÄYTTÖ=yy |  | Aktiverar Emit-avläsarens "trafikljus" för den avläsare som definierats med parametern LUKIJA=n (utan numret x). yy är ljussignalens längd i tiondels sekunder. |
| VALOT=n |  | Styr trafikljusstyrningen till en annan serieport, som anges med parametern n. |
| ESILUENTA | \*\*\* | Programmet används för förinläsning av startande, varvid de tävlande markeras som öppna (tidigare som ej startat) |
| ESILUENTA=KUNTO | \*\*\* | Programmet HkKisaWin används för förinläsning av startande, varvid Emit-koderna kopplas till tävlandeuppgifterna först i detta skede. I övrigt samma som föregående. |
| ESITARK |  | Programmet HkMaali används för förkontroll, varvid programmet i övrigt fungerar som vid stämplingskontroll, men banuppgifterna används inte och inga uppgifter registreras i tävlandeuppgifterna |
| EILÄHT=AVOIN |  | En parameter som är användbar i vissa arrangemangsmodeller för motionstävlingar, och som behövs när deltagarna markeras som ej startat tills de lämnar in sin bricka för inläsning. Utan denna parameter ger varje inläsning ett extra larm. |
| EMLOKI |  | Emit-händelser registreras i en loggfil. |
|  |  | **Parametrar för tidtagning baserad på Emit-tagg** |
| AIKALUKIJA AIKALUKIJA=VAIN AIKALUKIJA=VAINx AIKALUKIJAy=VAINx AIKALUKIJAy=LÄHDEz | \*\*\* | Avläsarkontrollens avläsningstidpunkt sparas, även om inga andra uppgifter än numret fås från brickan. VAIN: alltid endast avläsningstidpunkten. x anger punkten, y avläsaren och z den källkod som ges för tidtagningsdata från avläsare y. |
| LÄHDEPISTEET | \*\*\* | Vid fastställandet av tidtagningspunkt används filen lahdepisteetx.lst, där x är etappens nummer. Denna parameter behövs bl.a. ofta när SjBox används. Som punkt kan i filen också anges bokstaven L, varvid koden tolkas som starttid i stället för mål (M); kräver dessutom parametern PAKOTALAIKA. |
| AUTOKILP |  | Registrerar inlästa tävlande automatiskt i tidtagningsdata och kopplar dem till tiderna från målklockan. |
| LEIMAT=E |  | Filen LEIMAT.LST läses inte, även om den finns i startkatalogen. Får i vissa fall tidtagningen att fungera mer logiskt. |
| EMITAJAT EMITAJAT=n | \*\*\* | Tider som bestämts vid stämplingskontrollen skrivs till tidtagningsfilen. Parametern n anger tidtagningskön, om flera används. |
| LISÄÄEDELLE |  | En tid som läggs till på tidtagningsskärmen är 0,1 s bättre än den tidigare markerade tiden. Används när online-tidtagningen kompletteras med ordningsuppgifter som fastställts av måldomaren. |
| ESTÄEMITTOISTO=EI SALLIEMITOISTO | \*\*\* | Nästan på varandra följande inläsningar av samma Emit-bricka registreras (normalt förhindrat). |
| ECAIKA ETGPRS ETSQL ETHAKUVÄLI ETDATE ETTIME ETHOST |  | Styrparametrar för användning av den mellantidsserver som hör till användningen av emiTag |

### A1.7 Parametrar för användning av SQL-databas (endast HkKisaWin)

|  |  |  |
| --- | --- | --- |
| SQLHOST=host |  | Anger MySQL-serverns namn (som standard 'localhost') |
| SQLDATABASE=database |  | Anger MySQL-databasens namn (som standard 'kilp') |
| SQLUSER=user |  | Anger MySQL-databasens användare (som standard 'kilp') |
| SQLPASSWORD=salasana |  | Anger lösenordet för MySQL-databasens användare (som standard 'hkkisawin') |
| SQLSTART |  | Begär att MySQL-databasen öppnas automatiskt när programmet startar |
| SQLTALLENNUS |  | Begär att MySQL-databasen öppnas så att alla ändringar i tävlandeuppgifterna sparas automatiskt |

### A1.8 Blandade parametrar som inte fungerar i alla programversioner

|  |  |  |
| --- | --- | --- |
| TAULU\_COM=x TAULUVIIVE=xx GAZ=x GAZVAIHE=x GAZRIVIy= |  | Parametrar för styrning av olika resultattavlor |
| SIRIT SIRITREUNA SIRITARRIVE SIRITDEPART FEIG IMPINJ |  | Parametrar för användning av RFID-taggar vid tidtagning |
| SW2000=xx |  | Simningens klockenhet SW2000 används |
| BIBSIIRTOALUE |  | Område för nummer på icke-aktiva poster |
