# Liite 1. Käynnistysparametrit

## Liite 1. Käynnistysparametrit

Ohjelmien toimintaa ohjataan käynnistysparametrien avulla.
Toimittaessa yhdellä tietokoneella ei moniin perustoimintoihin tarvita lainkaan
käynnistysparametreja mutta ne ovat välttämättömiä mm. liitettäessä tietokoneita
toimimaan yhdessä verkon välityksellä. Parametrien tunnukset voidaan
kirjoittaa isoilla tai pienillä kirjaimilla samoin parametreilla ilmoitettavat
arvot. Vain muutamissa tekstikentissä, kuten otsikkoa
määriteltäessä, säilyttää ohjelma pienet kirjaimet, kaikissa muissa
tapaukissa muuttaa ohjelma kirjaimet isoiksi ennen käyttöä.

Konfiguraatiotiedostojen muokkaamiseen on käytettävissä
[kaavake](1.3_toimintatilojen_konfigurointi.md)
, joka helpottaa
useimpien määrittelyjen tekemistä. Taulukossa on merkitty
tähdillä parametrit, jotka voidaan määritellä konfiguraatioeditorilla. \*\*\* tarkoittaa
että parametri on täysin määriteltävissä, \*\*
ja \* että paramteri on määriteltävissä,
mutta sen kaikkia alamäärityksiä ei voi tehdä.

### A1.1 Perus- ja sekalaisia parametreja

|  |  |  |
| --- | --- | --- |
| CFG=tied.nimi |  | Määrittelee tiedoston, josta muut parametrit luetaan. Oletuksena on LASKENTA.CFG . Tätä parametria käytetään yleisimmin komentorivillä tai pikakuvakkeen määrityksissä. |
| VAIHE=n PÄIVÄ=n | \*\*\* | Ilmoittaa kilpailun vaiheen monivaiheissa kilpailuissa. Kun n on kirjain V, käytetään vaiheen määräytymistä tiedoston vaihevalinta.cfg perusteella. |
| IKKUNAT IKKUNAT=xxxx | \*\*\* | Pyytää lukemaan käytössä olevien kaavakkeiden sijainnit ja asetukset tiedostosta xxxx. Ellei tiedoston nimeä anneta, käytetään oletuksena nimeä ikkunat.xml |
| IKKUNAOTS=xxxx | \*\*\* | Ohjelmaikkunan otsikkoriville kirjoitetaan teksti xxxx |
| ÄÄNI=x | \*\*\* | missä x on 0, 1 tai 2. ÄÄNI=0 poistaa kaikki äänimerkit, ÄÄNI=1 ottaa käyttöön äänimerkin virhe- ja varoitustilanteissa ja ÄÄNI=2 ottaa käyttöön myös matalamman huomautusäänen sarjan ensimmäiseksi tai viimeiseksi sijoittuvalle kilpailijalle. Oletusarvo 1. |
| LISÄÄ=xxx | \*\*\* | missä x suurin määrä kilpailijoita, joita voidaan lisätä yhdessä istunnossa. Oletusarvona on kilpailijoiden kokonaismäärä 5000 ja lisäys 1000, jos määrä ylittää 5000. |
| SAKOT=xxx |  | Sakkojen, tasoitusten ja lisäajan käsittelyssä käytettävä yksikkö sekunteina. **Parametrin käyttöä on vältettävä ja sakkojen suuruus määriteltävä kilpailun ominaisuuksissa sekä jokaisen sakollisen sarjan tiedoissa.** |
| JUOKSUILM | \*\*\* | korjaustoiminnossa aloitetaan nimikentästä (muuten hylkäystiedon kentästä). Ohjemassa HkKisaWin vaikuttaa ilmoittautumiskaavakkeen oletusasetuksiin. |
| KUNTOSUUNNISTUS=x | \*\*\* | missä x on 1, 2 tai 3, määrittelee ohjelman WinKisaHk toimimaan kuntosuunnistusmoodissa |
| VAKANTTIALKU=nnnn | \*\*\* | Määrittelee edelliseen liittyen käytettävän vapaiden numeroiden alueen. Samaa alkunumeroa käytetään myös EMIT-koodien uudelleennumeroimisessa, ellei tätä kumota parametrilla EMITALKU=nnn. |
| LISÄVAKANTTIALKU=nn | \*\*\* | Kertoo automaattisesti luotavien vakanttien numeroalueen alarajan (oletuksena luominen numeroalueen yläpäähän). |
| VAKANTTILUKU=nnn | \*\*\* | Kertoo niiden vakanttitietueiden lukumäärän, jotka ohjelma muodostaa automaattisesti uuteen osanottajatiedostoon suunnistuksessa (vaatii käyttäjän hyväksynnän). Oletus on 20. Kuntosuunnistuksiin sopii olennaisesti suurempi lukumäärä (esim. 1000). Vakantit luodaan käytössä olevan numeroalueen yläpäähän (päättyen yleensä 9999), ellei luomisvaiheessa ole annettuna parametria LISÄVAKANTTIALKU, jolloin numerot aloitetaan näin annetusta arvosta. |
| SEURATIEDOT=x | \*\*\* | missä x on H tai E. Kertoo seuratietojen käsittelyn laajuuden. Kun x=H, käytetään seuraluettelosta hakua toiminnan apuna. Kun x=E, ei seuratietoa näytetä lainkaan esimerkiksi leimantarkastusikkunassa. |
| HENKILÖTIETOKANTA=xx | \*\*\* | Kertoo henkilötietokantatiedoston. xx on käytettävän tiedoston täysi nimi hakemistopolkuineen. Oletus on henkilot.csv kilpailun kansiossa. |
| SEURATIEDOSTO=xx | \*\*\* | Kertoo seuraluettelotiedoston. xx on käytettävän tiedoston täysi nimi hakemistopolkuineen. Samalla otetaan käyttöön haku seuraluettelosta. Oletus on seurat.csv kilpailun kansiossa. |
| KUNTOLÄHTÖ | \*\*\* | Ottaa käyttöön ilmoittautumiseen sopivan pelkistetyn kaavakkeen, kun valitaan toiminto *Leimatarkastus.* |
| KUNTOMAALI | \*\*\* | Valitsee kuntosuunnistuksen leimatarkastusnäytöllä valinnan *Maalin tietokone.* |
| VAHVISTAAINA=EI | \*\*\* | Kertoo, että kuntosuunnistusmoodissa ei tarvitse vahvistaa tietokannasta löytynyttä henkilöä. |
| SULKUSALASANA | \*\*\* | Antaa mahdollisuuden ohjelman etäsammuttamiseen |

#### Vain ohjelmaa HkMaali koskevia parametreja

|  |  |  |
| --- | --- | --- |
| BOOT BOOT=Mn | \*\*\* | Ohjelma käynnistetään kysymättä vahvistuksia. Lisäys =Mn pyytää avaamaan välittömästi ajanottonäytön (valinta *Maali*). Erikseen on huolehdittava käyttäen esimerkiksi parametreja AIKAERO, NÄPPÄIN, AJAT=/S ja COMFILE=S, että ohjelma ohittaa mahdolliset muut kyselyt. |
| RIVIT=25 | \*\*\* | Avaa ohjelman 25-rivisessä ikkunassa |
| KIRKAS | \*\*\* | Näytöllä käytetään kirkasta intensiteettiä |
| VÄRIT=xxx | \*\*\* | Vaihtaa konsoli-ikkunan väritystä (toteutus toistaiseksi puutteellinen) |
| TARKNO=n |  | Säätelee tarkistusnumeron käyttöä. n = 0: ei käytetä, n = 1: käytetään paitsi ajanottotoiminnossa, n = 2: käytetään aina (oletus 0). |
| KUNTO SUORASYÖTTÖ | \*\* | Tarkoitettu kuntosuunnistuksiin. Sallii tulosten antamisen suoraan kilpailijan lisäämisen yhteydessä ohjelmassa HkMaali. |
| FISKOODI |  | Ottaa käyttöön FIS-koodin käsittelyn ja FIS-pisteiden laskennan. Ei tarvita ohjelmassa HkKisaWin. |

### A1.2 Tiedonsiirtoparametrit

|  |  |  |
| --- | --- | --- |
| YHTEYSy=xxxx (tai COMy=x) | \*\*\* | ottaa käyttöön kahden PC:n välisen tiedonsiirtoyhteyden numero y. Jos y puuttuu, on yhteyden numero 1. xxxx määrittelee tiedonsiirtotavan |
| BAUDy=xxxx,p,b,s | \*\* | Kertoo sarjaporttiyhteyden y ominaisuudet. xxxx on sarjaporttiyhteyden liikennöintinopeus, p pariteetti (n, e tai o), b databittien luku ja s stopbittien luku. Oletusarvo on 9600,n,8,1 ja lopusta voidaan jättää oletuksen mukaisia pois. |
| PORTBASE=xxxx | \*\*\* | UDP-portin numeron vakio-osa, johon lisätään yhteyden numero. Oletusarvo on 15900. |
| VÄLITÄy=EI | \*\*\* | Kieltää ohjelmaa välittämästä kaikkia tai osaa saapuvista viesteistä toiseen yhteyteen |
| VARASERVER | \*\*\* | Ilmoittaa, että kone käynnistetään moodissa, jossa se ei lähetä sanomia |
| KONE=xx | \*\*\* | Tietokoneen tunnus on xx (kaksimerkkinen tunnus), joka näytetään tiedonsiirto-yhteyden vastapuolella. |
| COMFILE COMFILE=S | \*\*\* | Tiedonsiirto puskuroidaan levytiedostoon. 'S' poistaa tiedoston poistamista koskevan kysymyksen käynnistettäessä. Oletuksena puskurointi kysyen. |
| COMFILE=EI | \*\*\* | Poistaa tiedonsiirron puskusoinnin levytiedostoon. |
| MAXYHTEYS=n | \*\*\* | Yhteyksien maksimimäärä. Vaikuttaa vain tiedoston COMFILE.DAT rakenteeseen. Jätetään yleensä ottamatta huomioon, jos COMFILE.DAT sisältää jo sanomia. |
| TCPTULOS=x |  | Pyytää lähettämään TCP-yhteyteen kaksipäiväisessä kilpailussa yhteistuloksen päivän tuloksen ohella tai sijasta. |
| FILETCPy=xxxx |  | Käynnistää automaattisen XML-tiedostojen lähettämisen määrävälein |
| XMLSARJA=xxx |  | Valitsee sarjan edelliseen toimintoon |
| TCPAUTOALKU |  | Pyytää käynnistämään tiedostojen automaattisen TCP-siirron ohjelman käynnistyttyä |
| NAKVIIVE=xx UDPJARRU=xx UDPVIIVEET=xxx |  | Parametreja tiedonsiirron virittelyyn (ei yleensä tarpeen) |
| MAALILEIMASIN=EI | \*\*\* | Rastiväliaikatietueeseen sisällytetään viimeisen leimauksen aika, vaikka se on ratatietojen mukaan maalileimasimen aika, jota ei haluta online-tulospalveluun varsinaisen maaliajan rinnalle. |
| MONITORI MONITORI=xxxx |  | Pyytää ohjelmaa lähettämään säännöllisesti sanomia, jotka kertovat tiedonsiirron tilasta. Ohjelma InstanssiMonitori voi näiden sanomien avulla seurata koko tulospalveluverkon tilaa. Ellei täsmennystä xxxx ole, lähetetään sanomat verkon broadcast-osoitteeseen ja oletusporttiin 14900. Täsmennys xxxx sisältää yhdistelmän ip-numero:portti |
| MONITORVÄLI=xx |  | Pyytää vaihtamaan monitorointisanomien välin xx sekunniksi (oletuksena 10 s). (Huomaa puuttuva 'I'). |
| LÄHKELLOy=xx | \*\*\* | Pyytää lähettämään kellonajan yhteyteen y xx sekunnin välein. (Tarkoitettu eräiden laitteiden kellon synkronointiin.) Parametriin voidaan liittää täydennys /VAIN, jolloin kyseiseen yhteyteen lähetään vain kellonajat ja yhteyttä käsitellään muuten yksisuuntaisena. |
| LÄHTIEDOSTOT LÄHTIEDOSTOTy=x | \*\* | Ottaa käyttöön tiedostojen siirron tulospalveluverkon kautta. Ensimmäinen vaihtoehto koskee kaikkia UDP-yhteyksiä, jälkimmäinen vain yhteyttä y. Jos x = 0, kumotaan käyttö tämän yhteyden osalta. Komennon loppuun lisätty /K kertoo, että tiedostoja voidaan siirtää muualtakin kuin kilpailuhakemiston alta. |

### A1.3 Tulostuksiin liittyvät parametrit

|  |  |  |
| --- | --- | --- |
| LISTA LISTA=nimi | \*\*\* | nimi on kirjoittimen tai tiedoston nimi. Ilman koko parametria on oletuksena NUL eli ei tulostusta. Parametri LISTA ilman annettua nimeä viittaa oletuskirjoittimeen |
| LOKI(=tnimi) | \*\*\* | pyytää kirjoittamaan lokitiedoston levytiedostoon tai kirjoittimelle |
| MERKIT=xyy |  | vaihtaa listakirjoittimen merkistökoodit (x = G, I, R, A, W, P tai L), yy ilmoittaa mitä tiedostoja tai kirjoittimia koodit koskevat. Oletuksena G eli Windowsin kirjoitinajureiden käyttö. W soveltuu tekstitiedostoon ohjaukseen. Muita vaihtoehtoja ei tule käyttää, ellei poikkeuksellisesti haluta ohittaa Windowsin kirjoitinajureita. |
| MUOTOILU(=tied.nimi) | \*\*\* | pyytää lukemaan tulosluettelon muotoilumääritykset levytiedostosta ohjelman käynnistysvaiheessa. Jos tiedoston nimeä ei ilmoiteta, nimeksi oletetaan TULLUET.FMT. |
| AUTO(=xxxxx) | \*\*\* | Käynnistää automaattitulostuksen heti ohjelman käynnistyttyä. xxxx sisältää tulostusta ohjaavia parametreja. |
| VAADITARKASTUS | \*\*\* | Sarja katsotaan valmiiksi vasta, kun kaikille on annettu joku tarkastusmerkintä ja kaikilla hyväksytyillä on tulos. Sarjan valmistuttua tuottaa automaattinen tulostus tulosluettelon, vaikka muut säännöt eivät sitä heti tuottaisi. |
| NOLLAKÄRKI | \*\*\* | Tuloksissa erosarakkeeseen kirjoitetaan voittajalle aika '00.00.00'. |
| TAKAA-AJO | \*\*\* | 2. päivän väliajat esitetään kokonaisaikoina joihin sisältyy 1. päivän aika |
| HTML=tied/xx HTML=tied/xx/S MHTML=xx | \*\*\* | Kirjoita HTML-muotoinen tulosluettelo automaattisesti xx sekunnin välein. /S ilmaisee, että tulostus tapahtuu moneen sarjakohtaiseen tiedostoon. M viittaa mobiilimuotoiluun. |
| HTMLTITLE=nimi | \*\*\* | Laadittavan HTML-sivun nimi |
| HTMLOTS=otsikko | \*\*\* | Laadittavan HTML-sivun otsikko |
| HTMLSÄILNIMI | \*\*\* | Etu- ja sukunimen järjestystä ei vaihdeta HTML-tulosteissa |
| XML=tied/xx | \*\*\* | Kirjoita XML-muotoinen tulosluettelo automaattisesti xx sekunnin välein. |
| KOMENTO=xxxxx | \*\*\* | Komento, jonka ohjelma toteuttaa tiedoston automaattisen tulostuksen jälkeen |
| AUTOJAKELU=xxxxx | \*\*\* | Kertoo, että tiedosto siirretään palvelimelle automaattisen tulostuksen jälkeen. =xxxx kertoo palvelinyhteyden määrittelevän tiedoston nimen, jos se ei ole AutoJakelu.xml |
| DESEROTIN=x |  | Valitsee desimaalierottimeksi xml-tulosteissa merkin 'x' (pilkku tai piste). |
| KILPKDI=xxx |  | Määrittelee kilpailun koodin tulostettavaksi eräisiin tiedostotulosteisiin. Poistuva, määrittele kilpailun vaihekohtaisena tietona. |
| LÄHETÄCR=EI |  | Jättää rivinvaihtomerkin pois lähettäessään sivunvaihtomerkin kirjoittimelle |

### A1.4 Ohjelman erilaisia ajanottotoimintoja ohjataan käyttämällä parametreja

|  |  |  |
| --- | --- | --- |
| SYÖTTÖ |  | Ilmoittaa, että ohjelman HkMaali toimintoa M)aali käytetään ajanoton sijasta aikojen antamiseen näppäilemällä. |
| REGNLYx=n/..... | \*\*\* | Ilmoittaa, että ajanottoon käytetään Regnlyn maalikelloa |
| ALGE=n/..... | \*\*\* | Ilmoittaa, että ajanottoon käytetään Algen maalikelloa Timer S3 |
| ALGE4=n/..... | \*\*\* | Ilmoittaa, että ajanottoon käytetään Algen maalikelloa Timer S4 |
| COMET=n/..... | \*\*\* | Ilmoittaa, että ajanottoon käytetään Algen COMET-maalikelloa |
| TIMYx=n/..... | \*\*\* | Ilmoittaa, että ajanottoon käytetään Algen Timy-maalikelloa. x ilmaisee laitenumeron (oletus: 1). |
| ASC=n | \*\*\* | Ilmoittaa, että ajanottoon käytetään Algen ASC1-lähtökelloa |
| AIKAERO=xxxx | \*\*\* | Ilmoittaa sekunteina kellonajan, jolloin ajanottolaitteen aika on 0. Parametri johtaa aikaeroa koskevan kyselyn ohittamiseen. |
| KELLONO | \*\*\* | Ilmoittaa, että kellolta tulevat numerot tulkitaan kilpailijan numeroksi. (REGNLYNO, COMETNO ja TIMYNO toimivat samoin) |
| TUNNKOODIT(=E) |  | Ottaa käyttöön tai poistaa käytöstä tunnistuskoodin käytön |
| REGNLYVIIVE=xx |  | Määrää perättäisten pyyntöjen välin RTR-maalikelloa käytettäessä |
| KELLOBAUD=xxxx | \*\*\* | Ilmoittaa maalikellon tiedonsiirtonopeuden |
| KELLO\_ESTO=xx |  | Ilmoittaa sekunnin sadasosina ajan, jonka sisällä ei tallenneta toista aikaa maalikellolta |
| AIKA\_COM=x AIKA\_ESTO=x | \* | Sarjaliitännän avulla tapahtuvaa ajanottoa koskevia parametreja. Katso ao. lukua. |
| LÄHAIKAy | \*\*\* | Ilmoittaa, että yhteyttä y käytetään myös ajanottotietojen siirtoon. y on yhteyden numero kuten parametrissa YHTEYSy. |
| AJAT=/S AJAT=tied.nimi/S | \*\* | Ilmoittaa ajanottotiedoston nimen. '/S' poistaa tiedoston säilyttämistä koskevan kysymyksen (säilyttää kysymättä). |
| PAKOTALAIKA |  | Estää hiihdon 3 sekunnin säännön käytön lähtöaikoja käsiteltäessä |
| LÄHTÖIKKUNA=x |  | Vaihtaa "3 sek säännön" aikarajaksi x sek. |
| LAJAT LAJAT=tied.nimi | \*\* | Lähtöportin ajat tallennetaan eri tiedostoon kuin muut ajat |
| TAUSTA-AIKA |  | Antaa mahdollisuuden avata toinen aikajono ohjelman HkMaali pikkuikkunaan |
| AIKAKORJAUSy=xx |  | Ilmoittaa korjauksen, jolla jonon y aikoja muutetaan automaattisesti |
| LÄHTÖRYHMÄ=n |  | Kertoo samalla hetkellä lähtevän ryhmän koon helpottamaan kellolta saadun lähtöajan kirjaamista koko ryhmälle |
| PISTEET=xxxx | \*\* | Ilmoittaa eri ajanottotapojen käyttötarkoitukset |
| JONOT=xxxx | \*\*\* | Määrittelee, että otetut ajat jaetaan kahteen tai useampaan jonoon |
| MAXJONO=n | \*\*\* | Ilmoittaa käsiteltävien aikajonojen määrän. (Ei tarpeen, kun määrä ilmenee parametrista JONOT.) |
| JONOPISTEy=x |  | Ilmoittaa, mitä tuloksia eri ajanottojonoista saadaan |
| VERTRAJAT=xxx |  | Asettaa vertailurajat kahden ajanottojonon vertailuun |
| NÄPPÄIN=mmm,nnn | \*\*\* | Ilmoittaa ajanottoon käytettävän näppäimen. (Vain ohjelmassa HkMaali). |
| VAVUOROT VAVUOROT=sss/a/y | \*\*\* | Käyttää väliaikapisteen päättelyssä pelkkää ajan järjestysnumeroa tai aikaa, joka on kulunut edellisestä kirjatusta väliajasta (eikä siis aikaa lähdöstä). sss kertoo sekunteina jakson, jonka aikana saapuneita aikoja käsitellään yhtenä, a ja y ovat vuorottelussa sallittujen pisteiden ala- aja ylärajat. |
| VAINALARAJAT VAINALARAJAT=EI |  | Muuttaa tapaa, jolla parametri VAVUOROT käyttää väliaikojen alarajoja |
| UUSINAIKA |  | Samalle pisteelle otetusta ajoista jää voimaan viimeisin (ei ensimmäinen) |
| VAINVÄLIAJAT | \*\*\* | Estää automaattista päättelyä merkitsemästä aikaa lopputulokseksi. |
| ENNAKOI | \*\*\* | Ohjelman HkMaali ajanottotoiminnassa voidaan syöttää useita aikoja ennalta (Valikkovalinta ohjelmassa HkKisaWin) |
| ESTÄMUUTOS |  | Estää muutosten tekemisen ajanottonäytöllä |
| ESTÄYLIM |  | Estää ajanottorivin tallentamisen, kun kilpailijalla jo vastaava tulos |
| ESTÄHAAMUT=xx |  | Estää ajan kirjautumisen, jos tulos on alle xx minuuttia tai yli 20 tuntia, mikä vastaa alle 4 tunnin negatiivista aikaa |
| LISÄÄEDELLE |  | Ohjelman HkMaali ajanottotoiminnossa näppäimellä F2 lisättävä rivi tulee korostetun ajan edelle eikä jälkeen. |
| SALLITOISTO |  | Pyytää tallentamaan peräkkäisiä ajanottosanomia, jotka vaikuttavat toistolta |
| TUNNJONO TUNNJONO=y |  | Kertoo, että tunnistimeen perustuvaa ajanottojonoa y käytetään vain kilpailijan tunnistamiseen. Käytetään, kun aika otetaan esimerkiksi valokennolla. Jonon oletusarvo on 2. Tätä käytettäessä riittää lyhempi muoto. |
| TUNNRAJAT=a/y |  | Edelliseen liittyvät aikarajat sekunteina. a on usein negatiivinen ja ilmoittaa tällöin että aika voi tulla hieman ennen tunnistustietoa. y ilmoittaa maksimiajan tunnistustiedosta otettuun aikaan. |
| KAKSITUNN | \*\*\* | Ohjelma tunnistaa samaan vaiheeseen kaksi tunnistinkoodia, jotka tuottavat saman tuloksen. |
| YHTEYSAJAT VA-AJAT VAIKAVERT=xxx ILMTUNT(=EI) | \* | Tiedonsiirtoyhteyden kautta saapuvien ajanottotietojen käsittelyyn liittyviä parametreja. Käytetään mm ohjelman SendECodes ollessa käytössä. Parametri YHTEYSAJAT (tai VA-AJAT) ottaa tämän toiminnollisuuden käyttöön. |
| LÄHECODEy LÄHECODEy=VAIN | \*\*\* | Ilmoittaa, että yhteyteen y lähetetään parametrin YHTEYSAJAT perusteella vastaanotetut ajanottotiedot. y on yhteyden numero kuten parametrissa YHTEYSy. Lisämääritys VAIN merkitsee, että tähän yhteyteen ei lähetetä muita sanomia. |

### A1.6 Ampumahiihdon erityisparametrit

|  |  |  |
| --- | --- | --- |
| RACPISTEET | \*\*\* | Ampumapaikka tunnistetaan Regnlyn kelloa RTR2 ja RAC-laatikkoa käytettäessä RAC-laatikon sisäänmenon mukaisesti |
| SAKKO\_COM=n SAKKO\_YHT=n | \*\*\* | Ohjelma ottaa vastaan sakkotietoja Kurvisen laitteilta tai niitä emuloivasta syöttöpisteestä portin COMn kautta. |
| SAKKO\_BAUD=nnnn |  | Tiedonsiirron määrittelyparametreja käytettäessä parametria SAKKO\_COM. |
| SAKKO\_LAJI=n | \*\*\* | Kertoo käytössä olevan Kurvisen laitteiston version. n=0 vanhimmille laitteille, oletus: n=2. |

### A1.7 Emit-toimintojen parametreja

|  |  |  |
| --- | --- | --- |
| EMIT | \*\*\* | Käynnistää Emit-tietojen käsittelyn ilman muita toimintoja. Ei tarpeen, kun esim. LUKIJA on määritelty. |
| EMITANALYYSI | \*\*\* | Käynnistää rastiväliaikojen jatkuvan analysoinnin. |
| LUKIJAx=n LUKIJAx=n/i/a | \*\* | Tiedot luetaan suoraan lukijalaitteesta sarjaporttiin COMn. x tarpeen vain, jos useita lukijoita tai MTR-laitteita. |
| EMITAGx=n EMITAGx=n/u | \*\*\* | Tiedot luetaan emiTag-laitteelta. /u kertoo, että USB-sarjaportin nopeus on 115200 b/s |
| MTRx=n MTRx=n/i/a | \*\* | Tiedot luetaan MTR-laitteesta sarjaporttiin COMn. x tarpeen vain, jos useita lukijoita tai MTR-laitteita. Kun siirto tapahtuu tiedostosta EMIT\_IN.DAT, n on 'T'. |
| EMITKELLO=n EMITKELLO=n/w/i/a |  | Tiedot luetaan kellosta RTR2 sarjaporttiin COMn |
|  |  | **Leimantarkastuksen parametreja** |
| VAADIAIKA | \*\*\* | Ohjelma huomauttaa leimoja tarkastettaessa, jos kilpailijalla ei vielä ole tai saada automaattisesti aikaa (käytetään lähinnä, kun aika määräytyy leimauksen perusteella) |
| LAINAT | \*\*\* | Huomauta kuitattavalla ilmoituksella lainakortista |
| LAINALUETTELO LAINALUETTELO=VAIN | \*\*\* | Lainakorttien numerot saadaan tiedostosta. Jos VAIN on annettu, ei kilpailijatietojen lainamerkintöjä käytetä |
| LÄHEMITn[=I/O/V] | \*\*\* | Kortilta luetut Emit-tiedot siirretään yhteyden n kautta. Jos lopussa on =I tai =O, on Emit-tietojen siirto yksisuuntainen, = V kertoo, että vain yhden tietueen erikseen pyydetty lähettäminen sallitaan. |
| LÄHEMVAn | \*\*\* | Normaalien Emit-tietojen sijaan lähetetään niistä lasketut väliajat. |
| AUTORAP | \*\*\* | Pyytää tulostamaan automaattisesti jokaisen luetun kortin tiedot |
| AUTORAP=x | \*\* | x=1, H tai S. Tulostaa osan em. raporteista (ei hyväksytyt) |
| COMAUTORAP COMAUTORAP=H | \*\*\* | AUTORAP-toiminta voimassa vastaanottavalla koneella. Jos H annettu, koskee vain hylkäysesityksiä. Parametri annetaan koneella, jolla tulostus tapahtuu. |
| RAPORTTIHAK=xxxx |  | Hylkäysten käsittelyä koskevat raportit kirjoitetaan hakemistoon xxxx ([luku 6.16](6.16_hylkaysesitysten_kasittely.md)) |
| ANNASEUR |  | Uuden kilpailijanumeron kyselyn toistuessa tarjoaa aina seuraavaa numeroa. |
| AUTOSARJA | \*\*\* | Vaihda Emit-korttia luettaessa sarja leimoja vastaavaksi |
| AUTORATA | \*\*\* | Käyttää AUTOSARJA:n kaltaista toimintaa radoille, jotka eivät ole sarjakohtaisia. |
| RASTIVATULOSTE | \*\*\* | Käynnistää ohjelman tilaan, missä kortin lukeminen tuottaa väliaikatulosteen ([luku 6.8](6.8_rastivaliaikatulosteet.md) ) |
| KARTTA=xxxx | \*\*\* | Karttatiedosto, jota käytetään itkumuurin tukena ([luku 6.12](6.12_kartan_kaytto_itkumuurilla.md) ) |
| MAALILEIMASIN=EI | \*\*\* | Pyytää ottamaan rastiväliaikalistoille sekä viimeisen leimauksen ajan että lopputuloksen, vaikka molemmat viittaavat usein maaliviivan aikaan. Vaikuttaa rastiväliaikatulosteisiin ja online-palveluun lähetettäviin tietoihin. |
| SIVUJAKO=n |  | Samalle arkille tulostetaan tiiviisti n kilpailijan Emit-väliajat. |
| EMITVASIVU=l/m/n |  | Väliaikatulosteen rivinpituus=l, rivien lukumäärä m ja näistä n ylintä jätetään tyhjiksi. |
| TARKRAPOHITA=n |  | Pyytää jättämään n tyhjää riviä leimantarkastusraportin yläreunaa esimerkiksi järjestäjän logoa varten. |
| LÄHTÖVERT=xxx |  | Ohjelma huomauttaa, jos Emit-kortin nollaushetki poikkeaa arvonnan lähtöajasta yli xxx sek. |
| LUESUORAAN |  | Pyytää purkamaan MTR:n tiedot keskeytyksettä lisäten automaattisesti tietueita |
| EMITALKU=nnnn | \*\*\* | Ilmoittaa numeroarvon, josta alkaen haetaan vapaata Emit-koodia, kun ohjelma vaihtaa koodin automaattisesti. |
| VALONÄYTTÖ=yy |  | Ottaa käyttöön Emit-lukijan "liikennevalot" lukijalle, joka on määritelty parametrilla LUKIJA=n (ilman numeroa x). yy on valosignaalin kesto sekunnin kymmenyksinä. |
| VALOT=n |  | Ohjaa liikennevalo-ohjauksen eri sarjaporttiin, joka annetaan parametrilla n. |
| ESILUENTA | \*\*\* | Ohjelmaa käytetään lähtijöiden esiluentaan, jolloin kilpailijat merkitään avoimiksi (aiemmin ei-lähteneinä) |
| ESILUENTA=KUNTO | \*\*\* | Ohjelmaa HkKisaWin käytetään lähtijöiden esiluentaan liittäen Emit-koodit vasta tässä vaiheessa kilpailijan tietoihin. Muuten sama kuin edellinen. |
| ESITARK |  | Ohjelmaa HkMaali käytetään esitarkastukseen, jolloin ohjelma toimii muuten kuten leimantarkastuksessa, mutta ratatietoja ei käytetä eikä mitään tietoja kirjata kilpailijatietoihin |
| EILÄHT=AVOIN |  | Eräissä kuntokisojen järjestelymalleissa hyödyllinen parametri, jota tarvitaan, kun osanottajat merkitään ei-lähteneiksi, kunnes he tuovat korttinsa luettavaksi. Ilman tätä parametria synnyttää jokainen luenta ylimääräisen hälytyksen. |
| EMLOKI |  | Emit-tapahtumat kirjataan lokitiedostoon. |
|  |  | **Emit-tunnisteeseen perustuvan ajanoton parametreja** |
| AIKALUKIJA AIKALUKIJA=VAIN AIKALUKIJA=VAINx AIKALUKIJAy=VAINx AIKALUKIJAy=LÄHDEz | \*\*\* | Lukijarastin lukemishetki tallennetaan, vaikka kortilta ei saada muita tietoja kuin sen numero. VAIN: aina vain lukemishetki. x ilmaisee pisteen, y lukijan ja z lähdekoodin, joka annetaan lukijasta y tuleville ajanottotiedoille. |
| LÄHDEPISTEET | \*\*\* | Ajanottopisteen päättelyssä käytetään tiedostoa lahdepisteetx.lst, missä x on vaiheen numero. Tämä parametri tarvitaan mm. usein, kun käytössä on SjBox. |
| AUTOKILP |  | Kirjaa luetut kilpailijat automaattisesti ajanottotietoihin liittäen maalikellon antamiin aikoihin. |
| LEIMAT=E |  | Tiedostoa LEIMAT.LST ei lueta, vaikka se on käynnistyshakemistossa. Saa ajanoton joissain tapauksissa toimimaan loogisemmin. |
| EMITAJAT EMITAJAT=n | \*\*\* | Leimantarkastuksessa määrätyt ajat kirjoitetaan ajanottotiedostoon. Parametri n kertoo ajanottojonon, jos useita on käytössä. |
| LISÄÄEDELLE |  | Ajanottonäytöllä lisättävä aika on 0,1 s parempi kuin aiempi korostettu aika. Käytetään, kun online-ajanottoa täydennetään maalituomarin määräämällä järjestystiedolla. |
| ESTÄEMITTOISTO=EI SALLIEMITOISTO | \*\*\* | Saman Emit-kortin lähes peräkkäiset lukemiset kirjataan (normaalisti estetty). |
| ECAIKA ETGPRS ETSQL ETHAKUVÄLI ETDATE ETTIME ETHOST |  | emiTagin käyttöön liittyvän väliaikapalvelimen käytön ohjausparametreja |

### A1.7 SQL-tietokannan käyttöön liittyvät parametrit (vain HkKisaWin)

|  |  |  |
| --- | --- | --- |
| SQLHOST=host |  | Määrittelee MySQL-palvelimen nimen (oletuksena 'localhost') |
| SQLDATABASE=database |  | Määrittelee MySQL-tietokannan nimen (oletuksena 'kilp') |
| SQLUSER=user |  | Määrittelee MySQL-tietokannan käyttäjän (oletuksena 'kilp') |
| SQLPASSWORD=salasana |  | Määrittelee MySQL-tietokannan käyttäjän salasanan (oletuksena 'hkkisawin') |
| SQLSTART |  | Pyytää avaamaan MySQL-tietokannan automaattisesti ohjelman käynnistyessä |
| SQLTALLENNUS |  | Pyytää avaamaan MySQL-tietokannan siten, että kaikki kilpailijatietoihin tulevat muutokset talletetaan automaattisesti |

### A1.8 Sekalaisia parametreja, jotka eivät toimi kaikissa ohjelmaversioissa

|  |  |  |
| --- | --- | --- |
| TAULU\_COM=x TAULUVIIVE=xx GAZ=x GAZVAIHE=x GAZRIVIy= |  | Erilaisten tulostaulujen ohjaukseen liittyviä parametreja |
| SIRIT SIRITREUNA SIRITARRIVE SIRITDEPART FEIG IMPINJ |  | RFID-tunnisteiden ajanottokäyttöön liittyviä parametreja |
| SW2000=xx |  | Uinnin SW2000 kellolaite käytössä |
| BIBSIIRTOALUE |  | Alue ei-aktiivisten tietueiden numeroille |

---

 Copyright 2012, 2015 Pekka
Pirilä