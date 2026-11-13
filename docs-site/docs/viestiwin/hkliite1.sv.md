# Bilaga 1. Startparametrar

## Bilaga 1. Startparametrar

Programmens funktion styrs med startparametrar.
När du arbetar på en enda dator behövs för många grundfunktioner inga
startparametrar alls, men de är nödvändiga bl.a. när datorer kopplas
ihop för att arbeta tillsammans via nätverket. Parametrarnas beteckningar kan
skrivas med stora eller små bokstäver, liksom de värden som anges med parametrarna.
Endast i några textfält, t.ex. när en rubrik
definieras, behåller programmet små bokstäver, i alla andra
fall ändrar programmet bokstäverna till versaler före användningen.

### A1.1 Grundläggande och diverse parametrar

|  |  |
| --- | --- |
| CFG=tied.nimi | Anger den fil från vilken övriga parametrar läses. Standard är LASKENTA.CFG. Denna parameter anges vanligen på programmets kommandorad. |
| IKKUNAT IKKUNAT=xxxx | Begär att positionerna och inställningarna för de formulär som används läses från filen xxxx. Om inget filnamn anges används som standard namnet ikkunat.xml |
| IKKUNAOTS=xxxx | Texten xxxx skrivs på programfönstrets titelrad |
| ÄÄNI=x | där x är 0, 1 eller 2. ÄÄNI=0 tar bort alla ljudsignaler, ÄÄNI=1 aktiverar en ljudsignal vid fel- och varningssituationer och ÄÄNI=2 aktiverar även en lägre påminnelseton för en tävlande som placerar sig först eller sist i klassen. Standardvärde 1. |
| LISÄÄ=xxx | där x är det största antalet tävlande som kan läggas till under en session. Standardvärdet är det totala antalet tävlande 5000 och tillägget 1000, om antalet överstiger 5000. |
| SULKUSALASANA | Gör det möjligt att stänga av programmet på distans |
| SÄILHYL | Begär att även diskvalificerade tas med på den plats i resultatlistan som motsvarar tiden |

#### Parametrar som endast gäller programmen ViestiMaali och ViestiLuenta

|  |  |
| --- | --- |
| BOOT | Programmet startas utan att bekräftelser efterfrågas. |
| RIVIT=25 | Öppnar programmet i ett fönster med 25 rader |
| KIRKAS | Skärmen använder klar intensitet |
| VÄRIT=xxx | Ändrar konsolfönstrets färger (genomförandet är tills vidare ofullständigt) |
| TARKNO=n | Styr användningen av kontrollsiffra. n = 0: används inte, n = 1: används utom i tidtagningsfunktionen, n = 2: används alltid (standard 0). |

### A1.2 Parametrar för dataöverföring

|  |  |
| --- | --- |
| YHTEYSy=xxxx (eller COMy=x) | aktiverar dataöverföringsförbindelse nummer y mellan två datorer. Om y saknas är förbindelsens nummer 1. xxxx anger sättet för dataöverföring |
| BAUDy=xxxx,p,b,s | Anger egenskaperna för serieportsförbindelsen y. xxxx är serieportsförbindelsens överföringshastighet, p paritet (n, e eller o), b antal databitar och s antal stoppbitar. Standardvärdet är 9600,n,8,1 och värden enligt standard kan utelämnas i slutet. |
| PORTBASE=xxxx | Den fasta delen av UDP-portens nummer, till vilken förbindelsens nummer läggs. Standardvärdet är 15900. |
| VÄLITÄy=EI | Förbjuder programmet att vidarebefordra alla eller en del av de inkommande meddelandena till en annan förbindelse |
| VARASERVER | Anger att datorn startas i ett läge där den inte sänder meddelanden |
| KONE=xx | Datorns beteckning är xx (beteckning med två tecken), som visas i andra ändan av dataöverföringsförbindelsen. |
| COMFILE COMFILE=S | Dataöverföringen buffras i en diskfil. 'S' tar bort frågan om att radera filen vid start. Som standard buffring med fråga. |
| COMFILE=EI | Tar bort buffringen av dataöverföringen i en diskfil. |
| MAXYHTEYS=n | Maximalt antal förbindelser. Påverkar endast strukturen för filen COMFILE.DAT. Beaktas i allmänhet inte om COMFILE.DAT redan innehåller meddelanden. |
| FILETCPy=xxxx | Startar automatisk sändning av XML-filer med jämna mellanrum |
| XML=xxxx | Anger bl.a. storleken på tidsintervallet för föregående funktion |
| XMLSARJA=xxx | Väljer klass för föregående funktion |
| TCPAUTOALKU | Begär att den automatiska TCP-överföringen av filer startas när programmet har startat |
| NAKVIIVE=xx UDPVIIVEET=xxx TCPVIIVEET=xxx | Parametrar för finjustering av dataöverföringen (behövs i allmänhet inte) |
| MONITORI MONITORI=xxxx | Begär att programmet regelbundet sänder meddelanden som anger dataöverföringens status. Programmet InstanssiMonitori kan med hjälp av dessa meddelanden följa hela resultattjänstnätverkets status. Om preciseringen xxxx saknas sänds meddelandena till nätverkets broadcast-adress och standardporten 14900. Preciseringen xxxx innehåller kombinationen ip-nummer:port |
| MONITORVÄLI=xx | Begär att intervallet för övervakningsmeddelandena ändras till xx sekunder (standard 10 s). (Observera det saknade 'I'). |
| LÄHKELLOy=xx | Begär att klockslaget sänds till förbindelsen y med xx sekunders mellanrum. (Avsett för synkronisering av klockan i vissa enheter.) Till parametern kan tillägget /VAIN fogas, varvid endast klockslag sänds till förbindelsen i fråga och förbindelsen i övrigt behandlas som enkelriktad. |

### A1.3 Parametrar för utskrifter

|  |  |
| --- | --- |
| LISTA LISTA=nimi | nimi är skrivarens eller filens namn. Utan hela parametern är standard NUL, dvs. ingen utskrift. Parametern LISTA utan angivet namn hänvisar till standardskrivaren |
| LOKI(=tnimi) | begär att en loggfil skrivs till en diskfil eller till skrivaren |
| MERKIT=xyy | ändrar listskrivarens teckenkoder (x = G, I, R, A, W, P eller L), yy anger vilka filer eller skrivare koderna gäller. Standard är G, dvs. användning av Windows skrivardrivrutiner. W lämpar sig för styrning till textfil. Övriga alternativ ska inte användas, om man inte undantagsvis vill förbigå Windows skrivardrivrutiner. |
| MUOTOILU(=tied.nimi) | begär att resultatlistans formateringsdefinitioner läses från en diskfil när programmet startar. Om filnamnet inte anges antas namnet vara TULLUET.FMT. |
| KIELI=E | Engelska används i resultatlistornas rubriktexter |
| OTSIKOT | Begär att kolumnrubriker skrivs i resultatlistorna |
| AUTO(=xxxxx) | Startar automatisk utskrift genast när programmet har startat. xxxx innehåller parametrar som styr utskriften. |
| KOMENTO=xxxxx | Kommando som programmet utför efter automatisk utskrift av filen |
| HTML=tied/xx HTML=tied/xx/S | Skriv en resultatlista i HTML-format automatiskt med xx sekunders mellanrum. /S anger att utskriften görs till många klassvisa filer. |
| HTMLTITLE=nimi | Namnet på den HTML-sida som skapas |
| HTMLOTS=otsikko | Rubriken på den HTML-sida som skapas |
| XML=tied/xx | Skriv en resultatlista i XML-format automatiskt med xx sekunders mellanrum. |

### A1.4 Programmets olika tidtagningsfunktioner styrs med parametrar

|  |  |
| --- | --- |
| REGNLYx=n/..... | Anger att Regnlys målklocka används för tidtagningen |
| ALGE=n/..... | Anger att Alges målklocka Timer S3 används för tidtagningen |
| ALGE4=n/..... | Anger att Alges målklocka Timer S4 används för tidtagningen |
| COMET=n/..... | Anger att Alges målklocka COMET används för tidtagningen |
| TIMY=n/..... | Anger att Alges målklocka Timy används för tidtagningen |
| AIKAERO=xx | Anger i sekunder det klockslag då tidtagningsutrustningens klocka visar noll. Förbigår frågan vid start. |
| REGNLYNO COMETNO TIMYNO | Anger att nummer som kommer från klockan tolkas som den tävlandes nummer. |
| REGNLYVIIVE=xx | Anger intervallet mellan på varandra följande förfrågningar när RTR-målklockan används |
| KELLOBAUD=xxxx | Anger målklockans överföringshastighet |
| KELLO\_ESTO=xx | Anger i hundradels sekunder den tid inom vilken en andra tid från målklockan inte sparas |
| AIKA\_COM=x AIKA\_ESTO=x  AIKA\_MASK=xxx | Parametrar för tidtagning via seriegränssnitt. Se kapitlet om detta. |
| LÄHAIKAy | Anger att förbindelsen y också används för överföring av tidtagningsuppgifter. y är förbindelsens nummer som i parametern YHTEYSy. |
| AJAT=/S AJAT=tied.nimi/S | Anger tidtagningsfilens namn. '/S' tar bort frågan om att behålla filen (behåller utan att fråga). |
| AIKAKORJAUSy=xx | Anger den korrigering med vilken tiderna i kön y ändras automatiskt |
| PISTEET=xxxx | Anger användningsändamålen för olika tidtagningssätt |
| JONOT=xxxx | Anger att tagna tider fördelas på två eller flera köer baserat på tidtagningssättet |
| MAXJONO=n | Anger antalet tillgängliga köer, när det inte framgår av andra parametrar. |
| PISTE=x PISTEy=x JONOPISTEy=x | Anger arten av de tider som de olika tidtagningsköerna innehåller. När y saknas gäller parametern alla köer, annars anger y kön. x kan vara M, L, A eller mellantidspunktens nummer |
| YHTAIKAJONOy=x | Tider som fås från förbindelsen y styrs till kön x |
| LÄHDEPISTEET SJBOX | Reglerna för tidtagningspunkterna läses från filen lahdepisteet.lst |
| LÄHTÖRYHMÄ=n | Anger att storleken på en grupp vid gruppvis start är 6 lag |
| NÄPPÄIN=mmm/nnn | Anger den tangent som används för tidtagning. (Endast i programmet HkMaali). |
| UUSINAIKA | Av tider som tagits för samma punkt gäller den senaste (inte den första) |
| VAINVÄLIAJAT | Förhindrar automatisk slutledning från att markera tiden som slutresultat. |
| ENNAKOI | I tidtagningsfunktionen kan flera tider matas in i förväg |
| ESTÄYLIM ESTÄYLIM=xx | Förhindrar att en tidtagningsrad sparas när den tävlande redan har ett motsvarande resultat. xx är det tidsintervall inom vilket tiderna jämförs, standard 2 sek. |
| ESTÄNEG ESTÄHAAMUT ESTÄHAAMUT=xx | Förhindrar att tiden registreras om resultatet är under xx minuter eller över 20 timmar, vilket motsvarar en negativ tid på under 4 timmar. Om xx inte anges är den nedre gränsen 1 sek. |
| SAMAPISTE=xx | Anger i sekunder den gräns inom vilken inkomna tider tolkas som tider från samma punkt. |
| LISÄÄEDELLE | I tidtagningsfunktionen hamnar en rad som läggs till med tangenten F2 före den markerade tiden och inte efter. |
| SALLITOISTO | Begär att på varandra följande tidtagningsmeddelanden som ser ut som upprepningar sparas |
| KAKSITUNN | Programmet identifierar för samma etapp två brickkoder som ger samma resultat. |
| OLETUSOSUUS=n OSUUSRAJA=n | Anger beträffande hur sträckorna bestäms samma uppgifter som i programmet ViestiMaali kan ändras med tangenterna Alt-O på tidtagningsskärmen |
| YHTEYSAJAT VA-AJAT VAIKAVERT=xxx ILMTUNT(=EI) | Parametrar för hantering av tidtagningsuppgifter som kommer via dataöverföringen (bl.a. från programmet SendECodes). Oftast behövs av dessa endast YHTEYSAJAT. |
| LÄHECODEy LÄHECODEy=VAIN | Anger att tidtagningsuppgifter som tagits emot på grund av parametern YHTEYSAJAT sänds till förbindelsen y. y är förbindelsens nummer som i parametern YHTEYSy. Tilläggsdefinitionen VAIN betyder att inga andra meddelanden sänds till denna förbindelse. |

### A1.5 Parametrar för Emit-funktionerna

|  |  |
| --- | --- |
| EMIT | Startar hanteringen av Emit-uppgifter utan andra funktioner. Behövs inte när t.ex. LUKIJA har definierats. |
| SISÄÄNLUENTA SISÄÄNLUENTA=MYÖS | Programmet ViestiWin startar i ett läge som tillåter inläsning, där funktionerna för stämplingsuppgifter inte är tillgängliga. När parametern innehåller tillägget MYÖS är det möjligt att växla mellan inläsning och stämplingskontroll. |
| EMITANALYYSIT | Programmet ViestiWin samlar kontinuerligt analysuppgifter om emit-mellantiderna. Behövs bl.a. för att skapa mellantidsutskrifter. |
| LUKIJAx=n LUKIJAx=n/i/a | Uppgifterna läses direkt från läsarenheten till serieporten COMn. x behövs endast om det finns flera läsare eller MTR-enheter. |
| EMITAGx=n EMITAGx=n/u | Uppgifterna läses från en emiTag-enhet. /u anger att USB-serieportens hastighet är 115200 b/s |
| MTRx=n MTRx=n/i/a | Uppgifterna läses från en MTR-enhet till serieporten COMn. x behövs endast om det finns flera läsare eller MTR-enheter. När överföringen sker från filen EMIT\_IN.DAT är n 'T'. |
| EMITKELLO=n EMITKELLO=n/w/i/a | Uppgifterna läses från klockan RTR2 till serieporten COMn |
| TARKAVOIMET | Programmet övervakar öppna sträckor vid inläsningen |
| SALLIEMITENSIN | Programmet ViestiLuenta tillåter att Emit-brickan läses innan den tävlande har identifierats |
| EMITLUKU | Begär att alltid den översta sträckan med rätt emit-kod väljs |
| LUENTALOKI | Begär att uppgifter om både lyckade och avbrutna inläsningshändelser skrivs i en separat loggfil. |
|  | **Parametrar för stämplingskontrollen** |
| VAADIAIKA | Programmet påpekar vid kontrollen av stämplingar om den tävlande ännu inte har eller automatiskt får en tid (används främst när tiden bestäms utifrån stämplingen) |
| JÄLKISYÖTTÖ | I stämplingskontrollen i programmet ViestiMaali kan en tävlande kopplas till brickan. |
| VAPAAEMIT=xxx | Nummer från vilket Emit-koder ges när programmet måste ge en sådan. |
| LAINAT | Påpekar med ett meddelande som ska kvitteras att det är en lånebricka |
| LAINALUETTELO LAINALUETTELO=VAIN | Lånebrickornas nummer fås från en fil. Om VAIN har angetts används inte lånemarkeringarna i de tävlandes uppgifter |
| LÄHEMITn[=I/O/V] | Emit-uppgifter som lästs från brickan överförs via förbindelsen n. Om det i slutet står =I eller =O är överföringen av Emit-uppgifter enkelriktad, = V anger att endast separat begärd sändning av en enskild post tillåts. |
| LÄHEMVAn[=I/O] | I stället för normala Emit-uppgifter sänds de mellantider som beräknats ur dem. |
| AUTORAP | Begär att uppgifterna för varje avläst bricka skrivs ut automatiskt |
| AUTORAP=x | x=1, H eller S. Skriver ut en del av ovan nämnda rapporter (inte godkända) |
| COMAUTORAP COMAUTORAP=H | AUTORAP-funktionen gäller på den mottagande datorn. Om H har angetts gäller det endast diskvalifikationsförslag. Parametern anges på den dator där utskriften sker. |
| RASTIVATULOSTE | Startar programmet i ett läge där avläsning av brickan ger en mellantidsutskrift (ännu inte genomfört) ([kapitel 6.4](6.8_rastivaliaikatulosteet.md) ) |
| KARTTA=xxxx | Kartfil som används som stöd vid klagomuren ([kapitel 6.8](6.8_kartan_kaytto_itkumuurilla.md) ) |
| RAPORTTIHAK=xxxx | Rapporter om hanteringen av diskvalifikationer skrivs i katalogen xxxx ([kapitel 6.12](6.12_hylkaysesitysten_kasittely.md)) |
| SIVUJAKO=n | På samma ark skrivs Emit-mellantiderna för n tävlande tätt. |
| VALONÄYTTÖ=yy | Aktiverar Emit-läsarens "trafikljus" för den läsare som har definierats med parametern LUKIJA=n (utan numret x). yy är ljussignalens längd i tiondels sekunder. |
| VALOT=n | Styr trafikljusstyrningen till en annan serieport, som anges med parametern n. |
| ESITARK | Programmet ViestiMaali används för förhandskontroll, varvid programmet i övrigt fungerar som vid stämplingskontrollen, men banuppgifterna används inte och inga uppgifter registreras i de tävlandes uppgifter |
|  | **Parametrar för tidtagning baserad på Emit-bricka** |
| AIKALUKIJA AIKALUKIJA=VAIN AIKALUKIJA=VAINx AIKALUKIJAy=VAINx AIKALUKIJAy=LÄHDEz | Läsarkontrollens avläsningstidpunkt sparas, även om inga andra uppgifter än numret fås från brickan. VAIN: alltid endast avläsningstidpunkten. x anger punkten och y läsaren. z anger den källkod som ges till meddelanden som kommer från läsaren y. |
| LÄHDEPISTEET | För att sluta sig till mellantidspunkten används filen lahdepisteet.lst. |
| AUTOKILP | Registrerar avlästa tävlande automatiskt i tidtagningsuppgifterna och kopplar dem till de tider som målklockan ger. |
| LEIMAT=E | Filen LEIMAT.LST läses inte, även om den finns i startkatalogen. Får i vissa fall tidtagningen att fungera mer logiskt. |
| LISÄÄEDELLE | En tid som läggs till på tidtagningsskärmen är 0,1 s bättre än den tidigare markerade tiden. Används när online-tidtagningen kompletteras med ordningsuppgifter som fastställts av måldomaren. |
| ESTÄEMITTOISTO=EI SALLIEMITOISTO | Nästan på varandra följande avläsningar av samma Emit-bricka registreras (normalt förhindrat). |
| JOUSTOVIESTI | Programmet identifierar laget och löparen utifrån emit-koden, även om den sträcka som står i tur saknar uppgifter |
| ECAIKA ETGPRS ETHAKUVÄLI ETDATE ETTIME ETHOST | Styrparametrar för användningen av den mellantidsserver som hör till användningen av emiTag |

### A1.6 Parametrar för användning av SQL-databas

|  |  |
| --- | --- |
| SQLHOST=host | Anger MySQL-serverns namn (standard 'localhost') |
| SQLDATABASE=database | Anger MySQL-databasens namn (standard 'kilp') |
| SQLUSER=user | Anger MySQL-databasens användare (standard 'kilp') |
| SQLPASSWORD=salasana | Anger lösenordet för MySQL-databasens användare (standard 'kilp') |
| SQLSTART | Begär att MySQL-databasen öppnas automatiskt när programmet startar |
| SQLTALLENNUS | Begär att MySQL-databasen öppnas så att alla ändringar i de tävlandes uppgifter sparas automatiskt |

### A1.7 Diverse parametrar som inte fungerar i alla programversioner

|  |  |
| --- | --- |
| TAULU\_COM=x TAULU\_BAUD= TAULUVIIVE=xx GAZ=x GAZVAIHE=x GAZRIVIy= | Parametrar för styrning av olika resultattavlor |
| SIRIT SIRITREUNA SIRITARRIVE SIRITDEPART IMPINJ  Parametrar för användning av RFID-taggar vid tidtagning | |
| SW2000=xx | Simningens klockenhet SW2000 används |
