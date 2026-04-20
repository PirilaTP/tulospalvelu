# Liite 1. Käynnistysparametrit

## Liite 1. Käynnistysparametrit

Ohjelmien toimintaa ohjataan käynnistysparametrien avulla.
Toimittaessa yhdellä tietokoneella ei moniin perustoimintoihin tarvita lainkaan
käynnistysparametreja mutta ne ovat välttämättömiä mm. liitettäessä tietokoneita
toimimaan yhdessä verkon välityksellä. Parametrien tunnukset voidaan
kirjoittaa isoilla tai pienillä kirjaimilla samoin parametreilla ilmoitettavat
arvot. Vain muutamissa tekstikentissä, kuten otsikkoa
määriteltäessä säilyttää ohjelma pienet kirjaimet, kaikissa muissa
tapaukissa muuttaa ohjelma kirjaimet isoiksi ennen käyttöä.

### A1.1 Perus- ja sekalaisia parametreja

|  |  |
| --- | --- |
| CFG=tied.nimi | Määrittelee tiedoston, josta muut parametrit luetaan. Oletuksena on LASKENTA.CFG. Tämä parametri annetaan yleensä ohjelman komentorivillä. |
| IKKUNAT IKKUNAT=xxxx | Pyytää lukemaan käytössä olevien kaavakkeiden sijainnit ja asetukset tiedostosta xxxx. Ellei tiedoston nimeä anneta, käytetään oletuksena nimeä ikkunat.xml |
| IKKUNAOTS=xxxx | Ohjelmaikkunan otsikkoriville kirjoitetaan teksti xxxx |
| ÄÄNI=x | missä x on 0, 1 tai 2. ÄÄNI=0 poistaa kaikki äänimerkit, ÄÄNI=1 ottaa käyttöön äänimerkin virhe- ja varoitustilanteissa ja ÄÄNI=2 ottaa käyttöön myös matalamman huomautusäänen sarjan ensimmäiseksi tai viimeiseksi sijoittuvalle kilpailijalle. Oletusarvo 1. |
| LISÄÄ=xxx | missä x suurin määrä kilpailijoita, joita voidaan lisätä yhdessä istunnossa. Oletusarvona on kilpailijoiden kokonaismäärä 5000 ja lisäys 1000, jos määrä ylittää 5000. |
| SULKUSALASANA | Antaa mahdollisuuden ohjelman etäsammuttamiseen |
| SÄILHYL | Pyytää sisällyttämään myös hylätyt ajan mukaiseen paikkaan tulosluettelossa |

#### Vain ohjelmia ViestiMaali ja ViestiLuenta koskevia parametreja

|  |  |
| --- | --- |
| BOOT | Ohjelma käynnistetään kysymättä vahvistuksia. |
| RIVIT=25 | Avaa ohjelman 25-rivisessä ikkunassa |
| KIRKAS | Näytöllä käytetään kirkasta intensiteettiä |
| VÄRIT=xxx | Vaihtaa konsoli-ikkunan väritystä (toteutus toistaiseksi puutteellinen) |
| TARKNO=n | Säätelee tarkistusnumeron käyttöä. n = 0: ei käytetä, n = 1: käytetään paitsi ajanottotoiminnossa, n = 2: käytetään aina (oletus 0). |

### A1.2 Tiedonsiirtoparametrit

|  |  |
| --- | --- |
| YHTEYSy=xxxx (tai COMy=x) | ottaa käyttöön kahden PC:n välisen tiedonsiirtoyhteyden numero y. Jos y puuttuu, on yhteyden numero 1. xxxx määrittelee tiedonsiirtotavan |
| BAUDy=xxxx,p,b,s | Kertoo sarjaporttiyhteyden y ominaisuudet. xxxx on sarjaporttiyhteyden liikennöintinopeus, p pariteetti (n, e tai o), b databittien luku ja s stopbittien luku. Oletusarvo on 9600,n,8,1 ja lopusta voidaan jättää oletuksen mukaisia pois. |
| PORTBASE=xxxx | UDP-portin numeron vakio-osa, johon lisätään yhteyden numero. Oletusarvo on 15900. |
| VÄLITÄy=EI | Kieltää ohjelmaa välittämästä kaikkia tai osaa saapuvista viesteistä toiseen yhteyteen |
| VARASERVER | Ilmoittaa, että kone käynnistetään moodissa, jossa se ei lähetä sanomia |
| KONE=xx | Tietokoneen tunnus on xx (kaksimerkkinen tunnus), joka näytetään tiedonsiirto-yhteyden vastapuolella. |
| COMFILE COMFILE=S | Tiedonsiirto puskuroidaan levytiedostoon. 'S' poistaa tiedoston poistamista koskevan kysymyksen käynnistettäessä. Oletuksena puskurointi kysyen. |
| COMFILE=EI | Poistaa tiedonsiirron puskusoinnin levytiedostoon. |
| MAXYHTEYS=n | Yhteyksien maksimimäärä. Vaikuttaa vain tiedoston COMFILE.DAT rakenteeseen. Jätetään yleensä ottamatta huomioon, jos COMFILE.DAT sisältää jo sanomia. |
| FILETCPy=xxxx | Käynnistää automaattisen XML-tiedostojen lähettämisen määrävälein |
| XML=xxxx | Määrittää mm. edellisen toiminnon aikavälin suuruuden |
| XMLSARJA=xxx | Valitsee sarjan edelliseen toimintoon |
| TCPAUTOALKU | Pyytää käynnistämään tiedostojen automaattisen TCP-siirron ohjelman käynnistyttyä |
| NAKVIIVE=xx UDPVIIVEET=xxx TCPVIIVEET=xxx | Parametreja tiedonsiirron virittelyyn (ei yleensä tarpeen) |
| MONITORI MONITORI=xxxx | Pyytää ohjelmaa lähettämään säännöllisesti sanomia, jotka kertovat tiedonsiirron tilasta. Ohjelma InstanssiMonitori voi näiden sanomien avulla seurata koko tulospalveluverkon tilaa. Ellei täsmennystä xxxx ole, lähetetään sanomat verkon broadcast-osoitteeseen ja oletusporttiin 14900. Täsmennys xxxx sisältää yhdistelmän ip-numero:portti |
| MONITORVÄLI=xx | Pyytää vaihtamaan monitorointisanomien välin xx sekunniksi (oletuksena 10 s). (Huomaa puuttuva 'I'). |
| LÄHKELLOy=xx | Pyytää lähettämään kellonajan yhteyteen y xx sekunnin välein. (Tarkoitettu eräiden laitteiden kellon synkronointiin.) Parametriin voidaan liittää täydennys /VAIN, jolloin kyseiseen yhteyteen lähetään vain kellonajat ja yhteyttä käsitellään muuten yksisuuntaisena. |

### A1.3 Tulostuksiin liittyvät parametrit

|  |  |
| --- | --- |
| LISTA LISTA=nimi | nimi on kirjoittimen tai tiedoston nimi. Ilman koko parametria on oletuksena NUL eli ei tulostusta. Parametri LISTA ilman annettua nimeä viittaa oletuskirjoittimeen |
| LOKI(=tnimi) | pyytää kirjoittamaan lokitiedoston levytiedostoon tai kirjoittimelle |
| MERKIT=xyy | vaihtaa listakirjoittimen merkistökoodit (x = G, I, R, A, W, P tai L), yy ilmoittaa mitä tiedostoja tai kirjoittimia koodit koskevat. Oletuksena G eli Windowsin kirjoitinajureiden käyttö. W soveltuu tekstitiedostoon ohjaukseen. Muita vaihtoehtoja ei tule käyttää, ellei poikkeuksellisesti haluta ohittaa Windowsin kirjoitinajureita. |
| MUOTOILU(=tied.nimi) | pyytää lukemaan tulosluettelon muotoilumääritykset levytiedostosta ohjelman käynnistysvaiheessa. Jos tiedoston nimeä ei ilmoiteta, nimeksi oletetaan TULLUET.FMT. |
| KIELI=E | Tulosluetteloiden otsikkoteksteissä käyettään englantia |
| OTSIKOT | Pyytää kirjoittamaan tulosluetteloihin sarakkeiden otsikot |
| AUTO(=xxxxx) | Käynnistää automaattitulostuksen heti ohjelman käynnistyttyä. xxxx sisältää tulostusta ohjaavia parametreja. |
| KOMENTO=xxxxx | Komento, jonka ohjelma toteuttaa tiedoston automaattisen tulostuksen jälkeen |
| HTML=tied/xx HTML=tied/xx/S | Kirjoita HTML-muotoinen tulosluettelo automaattisesti xx sekunnin välein. /S ilmaisee, että tulostus tapahtuu moneen sarjakohtaiseen tiedostoon. |
| HTMLTITLE=nimi | Laadittavan HTML-sivun nimi |
| HTMLOTS=otsikko | Laadittavan HTML-sivun otsikko |
| XML=tied/xx | Kirjoita XML-muotoinen tulosluettelo automaattisesti xx sekunnin välein. |

### A1.4 Ohjelman erilaisia ajanottotoimintoja ohjataan käyttämällä parametreja

|  |  |
| --- | --- |
| REGNLYx=n/..... | Ilmoittaa, että ajanottoon käytetään Regnlyn maalikelloa |
| ALGE=n/..... | Ilmoittaa, että ajanottoon käytetään Algen maalikelloa Timer S3 |
| ALGE4=n/..... | Ilmoittaa, että ajanottoon käytetään Algen maalikelloa Timer S4 |
| COMET=n/..... | Ilmoittaa, että ajanottoon käytetään Algen COMET-maalikelloa |
| TIMY=n/..... | Ilmoittaa, että ajanottoon käytetään Algen Timy-maalikelloa |
| AIKAERO=xx | Ilmoittaa sekunteina kellonajan, jolloin ajanottolaitteen kello näyttää nollaa. Ohittaa kyselyn käynnistettäessä. |
| REGNLYNO COMETNO TIMYNO | Ilmoittaa, että kellolta tulevat numerot tulkitaan kilpailijan numeroksi. |
| REGNLYVIIVE=xx | Määrää perättäisten pyyntöjen välin RTR-maalikelloa käytettäessä |
| KELLOBAUD=xxxx | Ilmoittaa maalikellon tiedonsiirtonopeuden |
| KELLO\_ESTO=xx | Ilmoittaa sekunnin sadasosina ajan, jonka sisällä ei tallenneta toista aikaa maalikellolta |
| AIKA\_COM=x AIKA\_ESTO=x  AIKA\_MASK=xxx | Sarjaliitännän avulla tapahtuvaa ajanottoa koskevia parametreja. Katso ao. lukua. |
| LÄHAIKAy | Ilmoittaa, että yhteyttä y käytetään myös ajanottotietojen siirtoon. y on yhteyden numero kuten parametrissa YHTEYSy. |
| AJAT=/S AJAT=tied.nimi/S | Ilmoittaa ajanottotiedoston nimen. '/S' poistaa tiedoston säilyttämistä koskevan kysymyksen (säilyttää kysymättä). |
| AIKAKORJAUSy=xx | Ilmoittaa korjauksen, jolla jonon y aikoja muutetaan automaattisesti |
| PISTEET=xxxx | Ilmoittaa eri ajanottotapojen käyttötarkoitukset |
| JONOT=xxxx | Määrittelee, että otetut ajat jaetaan kahteen tai useampaan jonoon ajanottotavan perusteella |
| MAXJONO=n | Kertoo käytettävissä olevien jonojen määrän, kun se ei ilmene muista parametreista. |
| PISTE=x PISTEy=x JONOPISTEy=x | Kertoo eri ajanottojonojen sisältämien aikojen luonteen. Kun y puuttuu koskee parametri kaikkia jonoja, muuten y kertoo jonon. x voi olla M, L, A tai väliaikapisteen numero |
| YHTAIKAJONOy=x | Yhteydestä y saatavat ajat ohjataan jonoon x |
| LÄHDEPISTEET SJBOX | Ajanottopisteitä koskevat säännöt luetaan tiedostosta lahdepisteet.lst |
| LÄHTÖRYHMÄ=n | Ilmoittaa, että erätyyppisessä lähdössä erän koko on 6 joukkuetta |
| NÄPPÄIN=mmm/nnn | Ilmoittaa ajanottoon käytettävän näppäimen. (Vain ohjelmassa HkMaali). |
| UUSINAIKA | Samalle pisteelle otetusta ajoista jää voimaan viimeisin (ei ensimmäinen) |
| VAINVÄLIAJAT | Estää automaattista päättelyä merkitsemästä aikaa lopputulokseksi. |
| ENNAKOI | Ajanottotoiminnassa voidaan syöttää useita aikoja ennalta |
| ESTÄYLIM ESTÄYLIM=xx | Estää ajanottorivin tallentamisen, kun kilpailijalla jo vastaava tulos. xx on aikaväli, jonka sisällä aikoja vertaillaan, oletuksena 2 sek. |
| ESTÄNEG ESTÄHAAMUT ESTÄHAAMUT=xx | Estää ajan kirjautumisen, jos tulos on alle xx minuuttia tai yli 20 tuntia, mikä vastaa alle 4 tunnin negatiivista aikaa. Jos xx ei anneta, on alaraja 1 sek. |
| SAMAPISTE=xx | Kertoo sekunteina rajan, jonka sisällä tulleet ajat tulkitaan samasta pisteistä tulleiksi. |
| LISÄÄEDELLE | Ajanottotoiminnossa näppäimellä F2 lisättävä rivi tulee korostetun ajan edelle eikä jälkeen. |
| SALLITOISTO | Pyytää tallentamaan peräkkäisiä ajanottosanomia, jotka vaikuttavat toistolta |
| KAKSITUNN | Ohjelma tunnistaa samaan vaiheeseen kaksi tunnistinkoodia, jotka tuottavat saman tuloksen. |
| OLETUSOSUUS=n OSUUSRAJA=n | Kertovat osuuksien määräytymisen osalta samoja tietoja, mitä ohjelmassa ViestiMaali voidaan muuttaa näppäimillä Alt-O ajanottonäytöllä |
| YHTEYSAJAT VA-AJAT VAIKAVERT=xxx ILMTUNT(=EI) | Tiedonsiirron kautta saapuvien ajanottotietojen (mm. ohjelmasta SendECodes) käsittelyyn liittyviä parametreja. Usimmiten tarvitaan mäistä vain YHTEYSAJAT. |
| LÄHECODEy LÄHECODEy=VAIN | Ilmoittaa, että yhteyteen y lähetetään parametrin YHTEYSAJAT perusteella vastaanotetut ajanottotiedot. y on yhteyden numero kuten parametrissa YHTEYSy. Lisämääritys VAIN merkitsee, että tähän yhteyteen ei lähetetä muita sanomia. |

### A1.5 Emit-toimintojen parametreja

|  |  |
| --- | --- |
| EMIT | Käynnistää Emit-tietojen käsittelyn ilman muita toimintoja. Ei tarpeen, kun esim. LUKIJA on määritelty. |
| SISÄÄNLUENTA SISÄÄNLUENTA=MYÖS | Ohjelma ViestiWin käynnistyy sisäänluennan sallivaan tilaa, jossa leimaustietoihin liittyvät toiminnot eivät ole käytettävissä. Kun parametri sisältää täydennyksen MYÖS, on mahdollista vuorotella sisäänluennan ja leimantarkastuksen kesken. |
| EMITANALYYSIT | Ohjelma ViestiWin kerää jatkuvasti analyysitietoja emitväliajoista. Tarpeen mm. väliaikatulosteiden laadinnassa. |
| LUKIJAx=n LUKIJAx=n/i/a | Tiedot luetaan suoraan lukijalaitteesta sarjaporttiin COMn. x tarpeen vain, jos useita lukijoita tai MTR-laitteita. |
| EMITAGx=n EMITAGx=n/u | Tiedot luetaan emiTag-laitteelta. /u kertoo, että USB-sarjaportin nopeus on 115200 b/s |
| MTRx=n MTRx=n/i/a | Tiedot luetaan MTR-laitteesta sarjaporttiin COMn. x tarpeen vain, jos useita lukijoita tai MTR-laitteita. Kun siirto tapahtuu tiedostosta EMIT\_IN.DAT n on 'T'. |
| EMITKELLO=n EMITKELLO=n/w/i/a | Tiedot luetaan kellosta RTR2 sarjaporttiin COMn |
| TARKAVOIMET | Ohjelma valvoo avoimia osuuksia sisäänluennassa |
| SALLIEMITENSIN | Ohjelma ViestiLuenta sallii Emit-kortin lukemisen ennen kilpailijan tunnistamista |
| EMITLUKU | Pyytää valitsemaan aina ylimmän osuuden, jolla oikea emit-koodi |
| LUENTALOKI | Pyytää kirjoittamaan erilliseen lokitiedostoon tiedot sekä onnistuneista että kesken jääneistä sisäänluennan tapahtumista. |
|  | **Leimantarkastuksen parametreja** |
| VAADIAIKA | Ohjelma huomauttaa leimoja tarkastettaessa, jos kilpailijalla ei vielä ole tai saada automaattisesti aikaa (käytetään lähinnä, kun aika määräytyy leimauksen perusteella) |
| JÄLKISYÖTTÖ | Ohjelman ViestiMaali leimantarkstuksessa voidaan liittää kilpailija korttiin. |
| VAPAAEMIT=xxx | Numero, josta alkaen annetaan Emit-koodeja, kun ohjelma joutuu sellaisen antamaan. |
| LAINAT | Huomauta kuitattavalla ilmoituksella lainakortista |
| LAINALUETTELO LAINALUETTELO=VAIN | Lainakorttien numerot saadaan tiedostosta. Jos VAIN on annettu, ei kilpailijatietojen lainamerkintöjä käytetä |
| LÄHEMITn[=I/O/V] | Kortilta luetut Emit-tiedot siirretään yhteyden n kautta. Jos lopussa on =I tai =O, on Emit-tietojen siirto yksisuuntainen, = V kertoo, että vain yhden tietueen erikseen pyydetty lähettäminen sallitaan. |
| LÄHEMVAn[=I/O] | Normaalien Emit-tietojen sijaan lähetetään niistä lasketut väliajat. |
| AUTORAP | Pyytää tulostamaan automaattisesti jokaisen luetun kortin tiedot |
| AUTORAP=x | x=1, H tai S. Tulostaa osan em. raporteista (ei hyväksytyt) |
| COMAUTORAP COMAUTORAP=H | AUTORAP-toiminta voimassa vastaanottavalla koneella. Jos H annettu, koskee vain hylkäysesityksiä. Parametri annetaan koneella, jolla tulostus tapahtuu. |
| RASTIVATULOSTE | Käynnistää ohjelman tilaan, missä kortin lukeminen tuottaa väliaikatulosteen (ei vielä toteutettuna) ([luku 6.4](6.8_rastivaliaikatulosteet.md) ) |
| KARTTA=xxxx | Karttatiedosto, jota käytetään itkumuurin tukena ([luku 6.8](6.8_kartan_kaytto_itkumuurilla.md) ) |
| RAPORTTIHAK=xxxx | Hylkäysten käsittelyä koskevat raportit kirjoitetaan hakemistoon xxxx ([luku 6.12](6.12_hylkaysesitysten_kasittely.md)) |
| SIVUJAKO=n | Samalle arkille tulostetaan tiiviisti n kilpailijan Emit-väliajat. |
| VALONÄYTTÖ=yy | Ottaa käyttöön Emit-lukijan "liikennevalot" lukijalle, joka on määritelty parametrilla LUKIJA=n (ilman numeroa x). yy on valosignaalin kesto sekunnin kymmenyksinä. |
| VALOT=n | Ohjaa liikennevalo-ohjauksen eri sarjaporttiin, joka annetaan parametrilla n. |
| ESITARK | Ohjelmaa ViestiMaali käytetään esitarkastukseen, jolloin ohjelma toimii muuten kuten leimantarkastuksessa, mutta ratatietoja ei käytetä eikä mitään tietoja kirjata kilpailijatietoihin |
|  | **Emit-tunnisteeseen perustuvan ajanoton parametreja** |
| AIKALUKIJA AIKALUKIJA=VAIN AIKALUKIJA=VAINx AIKALUKIJAy=VAINx AIKALUKIJAy=LÄHDEz | Lukijarastin lukemishetki tallennetaan, vaikka kortilta ei saada muita tietoja kuin sen numero. VAIN: aina vain lukemishetki. x ilmaisee pisteen ja y lukijan. z kertoo lukijasta y saapuville sanomille annettavan lähdekoodin. |
| LÄHDEPISTEET | Väliaikapisteen päättelyyn käytetään tiedostoa lahdepisteet.lst. |
| AUTOKILP | Kirjaa luetut kilpailijat automaattisesti ajanottotietoihin liittäen maalikellon antamiin aikoihin. |
| LEIMAT=E | Tiedostoa LEIMAT.LST ei lueta, vaikka se on käynnistyshakemistossa. Saa ajanoton joissain tapauksissa toimimaan loogisemmin. |
| LISÄÄEDELLE | Ajanottonäytöllä lisättävä aika on 0,1 s parempi kuin aiempi korostettu aika. Käytetään, kun online-ajanottoa täydennetään maalituomarin määräämällä järjestystiedolla. |
| ESTÄEMITTOISTO=EI SALLIEMITOISTO | Saman Emit-kortin lähes peräkkäiset lukemiset kirjataan (normaalisti estetty). |
| JOUSTOVIESTI | Ohjelma tunnistaa joukkueen ja juoksijan emit-koodin perusteella, vaikka vuorossa olevalla osuudella ei ole tietoja |
| ECAIKA ETGPRS ETHAKUVÄLI ETDATE ETTIME ETHOST | emiTagin käyttöön liittyvän väliaikapalvelimen käytön ohjausparametreja |

### A1.6 SQL-tietokannan käyttöön liittyvät parametrit

|  |  |
| --- | --- |
| SQLHOST=host | Määrittelee MySQL-palvelimen nimen (oletuksena 'localhost') |
| SQLDATABASE=database | Määrittelee MySQL-tietokannan nimen (oletuksena 'kilp') |
| SQLUSER=user | Määrittelee MySQL-tietokannan käyttäjän (oletuksena 'kilp') |
| SQLPASSWORD=salasana | Määrittelee MySQL-tietokannan käyttäjän salasanan (oletuksena 'kilp') |
| SQLSTART | Pyytää avaamaan MySQL-tietokannan automaattisesti ohjelman käynnistyessä |
| SQLTALLENNUS | Pyytää avaamaan MySQL-tietokannan siten, että kaikki kilpailijatietoihin tulevat muutokset talletetaan automaattisesti |

### A1.7 Sekalaisia parametreja, jotka eivät toimi kaikissa ohjelmaversioissa

|  |  |
| --- | --- |
| TAULU\_COM=x TAULU\_BAUD= TAULUVIIVE=xx GAZ=x GAZVAIHE=x GAZRIVIy= | Erilaisten tulostaulujen ohjaukseen liittyviä parametreja |
| SIRIT SIRITREUNA SIRITARRIVE SIRITDEPART IMPINJ  RFID-tunnisteiden ajanottokäyttöön liittyviä parametreja | |
| SW2000=xx | Uinnin SW2000 kellolaite käytössä |

---

 Copyright 2012, 2015 Pekka
Pirilä