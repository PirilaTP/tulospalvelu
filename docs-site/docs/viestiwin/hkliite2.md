# Liite 2. Tiedonsiirron käyttöönotto

## Liite 2. Tiedonsiirron käyttöönotto

### A2.1 Tulospalveluverkon sisäinen tiedonsiirto käyttäen TCP/IP-verkkoa

Lisätietoja tiedonsiirron käyttömahdollisuuksista on dokumentissa HkMaali.pdf.

Tulospalveluohjelmia käyttävien tietokoneiden välisissä
yhteyksissä käytetään yleensä UDP-protokollaa. TCP-protokolla on käytettävissä,
kun UDP ei toimi esimerkiksi kännykkäyhteyden kautta. Myös sarjaporttien ja
nollamodeemikaapeleiden kautta tapahtuva tiedonsiirto toimii ohjelmissa edelleen.

UDP-yhteys edellyttää, että
kutakin yhteyttä käyttävistä koneista kumpikin varaa yhden UDP-portin ottamaan
vastaan vastapuolelta tulevia sanomia. Tälle portille on oletusarvona 15900 + n,
missä n on yhteyden numero, joten portin numero on tiedossa, kun yhteyden numero
tiedetään. Portin numeron laskennassa käytetty arvo voidaan vaihtaa käyttäen parametria

PORTBASE=pohja-arvo

jota vastaava oletus on siis PORTBASE=15900
Portin numero voidaan vaihtaa myös yhteyskohtaisesti, kuten alempana kerrotaan. Jos
yhteyden numeroakaan ei ole ilmoitettu, olettaa ohjelma, että kyseessä on
yhteys 1 ja portti siten 15901. Yhteyden numero on konekohtainen. UDP-yhteyksiä käytettäessä
on yleistä, että vain yhdessä koneessa, "palvelimessa", käytetään useita yhteyksiä, kaikissa
muissa koneissa voidaan tällöin käyttää vain yhtä yhteyttä. Jos käytössä on myös varapalvelin,
on muissakin koneissa käytettävä kahta yhteyttä esimerkiksi siten, että palvelimeen käytetään
yhteyttä 1 ja varapalvelimeen yhteyttä 2.

Yleensä on käytännöllisintä määritellä yhteys siten,
että ip-osoite ja vastapuolen portti määritellään vain yhteyden toisessa päässä.
Tällöin tämä yhteyden osapuoli lähettää oman ip-osoitteensa ja porttinsa
vastapuolelle, minkä jälkeen yhteys toimii molempiin suuntiin. Vastapuolen
porttiakaan ei tarvitse erikseen määritellä, jos kyseessä on vastapuolen yhteys
1, koska tämän yhteyden portti 15901 on oletuksena. (Parametri PORTBASE
vaikuttaa myös tähän numeroarvoon.)

UDP-tiedonsiirto kahden koneen välillä käynnistetään
antamalla molemmissa koneissa parametri YHTEYS
täydellisessä muodossa:

`YHTEYSn=UDP:omaportti/vastaip:vastaportti`

tai yksinkertaisemmissa muodoissa, joissa ainakin osa osoiteparametreista jätetään pois,
mistä seuraavassa esimerkkejä

`YHTEYSn=UDP
YHTEYSn=UDP:omaportti
YHTEYSn=UDP:0/vastaip`

Jos tieto omaportti puuttuu kokonaan tai on 0 tai koostuu kirjainmerkeistä, käyttää ohjelma
omalle portille arvoa 15900 + n, missä n on yhteyden numero. Jos vastaip tai vastaportti
halutaan antaa, on kenttään omaportti merkittävä 0 tai joku kirjainmerkki, eli
merkit : ja / eivät voi seurata suoraan toisiaan.

Jos parametri vastaip puuttuu tai sen arvo on AUTO, odottaa ohjelma omaan porttiin
vastapuolen sanomaa, josta se saa selville sekä vastapuolen ip-osoitteen että vastaportin.
Asiasta on vähän lisätietoa tämän luvun lopussa. Kun vastapuolen ip-osoitetta ei anneta,
ei siis tarvitse antaa myöskään vastaporttia, sillä ohjelma saa myös tämän tiedon vastaanottamastaan sanomasta.

Tiedolle vastaportti on oletusarvona 15901 eli oletuksena on, että kyseessä on vastapuolen yhteys 1,
jolle ei ole määritelty poikkeavaa porttia. Jos kenttään vastaportti kirjoitetaan sana
yhteys ja numero n, valitsee ohjelma vastapuolen portiksi 15900 + n eli olettaa, että
kyseessä on vastapuolen yhteyden n oletusportti. (Tässä voi sanan yhteys sijasta käyttää
pelkkää kirjainta Y.)

Heti sanan UDP perässä voi olla kirjain i tai o ilmaisemassa, että tietoja
vain otetaan vastaan (incoming) tai vain lähetetään (outgoing).

Asiaa selventävät esimerkit:

Esimerkki 1: Vakioportteja käyttävät yhteydet

Tässä esimerkissä liitetään yksi "palvelin" kolmeen muuhun koneeseen. Palvelimelle
määritellään tiedonsiirrot seuraavasti

`yhteys1=udp:0/192.168.1.12
yhteys2=udp:0/192.168.1.13
yhteys3=udpo:0/192.168.1.14:yhteys2`

Ensimmäisen yhteyden toisessa päässä on parametrina

`yhteys1=udp`

Toisen yhteyden vastapuolen parametri on

`yhteys1=udp`

Kolmannen yhteyden vastapuolen parametri kertoo, että kyse on yksisuuntaisesta
saapuvasta tiedosta sekä että kyseessä on tämän koneen yhteys 2:

`yhteys2=udpi`

Jokainen yhteys edellyttää, että yhteyden jommallekummalle päälle on kerrottu
vastapuolen ip-osoite ja että tämä pää tietää oikein myös vastapuolen arvon omaportti.
Tätä porttia ei tarvitse kuitenkaan kirjoittaa parametriksi, jos kyseessä on vastapuolen yhteys 1.

Esimerkki 2: Yhteys, jossa kaikki arvot on määritelty parametreihin:

UDP-protokollan saa tiedonsiirtoon antamalla yhteys-parametrin koneella,
jonka ip-numero on esimerkiksi 192.168.1.11 muodossa

`YHTEYS1=UDP:1250/192.168.1.12:1350`

kun toisessa päässä olevan koneen ip-numero on 192.168.1.12 Siellä toisessa päässä on taas annettava parametri

`YHTEYS1=UDP:1350/192.168.1.11:1250`

Ensimmäinen numeroarvo on portti, johon avataan kyseisellä koneella UDP-sanomien
vastaanotto ja kauttaviivan jälkeen tulevat toisen koneen ip-numero ja UDP-portti.
Jokainen yhteys käyttää siis kummassakin päässä UDP-vastaanottoa. Sanomat lähetetään
aina vastaanottavaan porttiin, joka kuuntelee jatkuvasti ja vastaa lähettävään porttiin,
jonka järjestelmä antaa automaattisesti aina jokaista sanomaa varten erikseen. Jokainen
kone voi olla yhteydessä hyvin moneen muuhun koneeseen ja jokaiselle yhteydelle on
siis varattava oma vastaanottava portti.

Koneessa varattuna olevat portit voi kysyä komennolla

`NETSTAT /A`

eikä sen ilmoittamia UDP-portteja pidä käyttää. Edellä käytetyt porttien numerot
1250 ja 1350 olivat vain kaksi satunnaisesti valitsemaani numeroa. Mitkä tahansa
vapaat portit käyvät yhtä hyvin. Kuten edellä on sanottu, odotusarvona ovat
portit alkaen numerosta 15901.

Numeroesityksen sijasta voidaan käyttää parametrina vastaip myös tekstimuotoista
osoitetta kohdekoneelle. Jotta kilpailutilanteessa ei oltaisi riippuvaisia nimipalvelimen
toiminnasta, suosittelen, että kaikki käytettävät tekstimuotoiset nimet määritellään
tiedostossa hosts, joka sijaitsee tyypillisesti hakemistossa \WINNT\system32\drivers\etc.

Kun samalla koneella halutaan käyttää useita kopioita tulospalveluohjelmista,
voidaan parametrina vastaip käyttää osoitetta localhost, joka viittaa aina
ip-numeroon 127.0.0.1. Osoitteen localhost käyttö onnistuu, vaikka koneessa ei
olisi lainkaan verkkokorttia, edellyttäen, että tcp/ip-toiminnot on asennettu.
Koneen sisäisissä yhteyksissä ei voida käyttää porttien oletusarvoja, ellei eri
ohjelmille määritellä eri yhteysnumeroita, koska samaa porttia ei voida varata
useammalle kuin yhdelle ohjelmalle.

Parametrina vastaip voidaan yhteyden toisessa päässä
antaa myös sana AUTO. Kun näin menetellään odottaa kyseinen kone UDP-sanomaa
porttiin omaportti ja käyttää jatkossa vastaanotetun sanoman lähettäjän
ip-numeroa. Tämä ip-numero voi lisäksi muuttua ohjelman toimiessa,
sillä ohjelma käyttää aina viimeisen porttiin omaportti vastaanottamansa
UDP-sanoman lähettäjän ip-numeroa. Tämä menettely on hyödyllinen sekä parametrien
yksinkertaistamiseksi että tilanteessa, jossa yhteyden toisen osapuolen ip-numeroa ei
tiedetä ennalta tai se voi jopa vaihdella. Tällaiseen
tilanteeseen päädytään lähinnä silloin, kun yhteys otetaan internetin kautta käyttäen
julkista palveluntarjoajaa. Toisessa päässä on vastapuolen ip-numero kuitenkin aina ilmoitettava,
jotta yhteys voisi muodostua.

Ohjelma voi toimia TCP-yhteydessä sekä
palvelimena että asiakkaana. Parametri on tyyppiä

`YHTEYS5=TCP:192.168.1.12:1350`

missä ip-osoite ja portti viittaavat käytettävään
palvelimeen, kun ohjelma on asiakkaana ja

`YHTEYS5=TCPS:1355`

missä numeroarvo on yhteydenottoportti, kun ohjelma toimii palvelimena.

Kun tietoja halutaan lähettä tekstimuotoisena
TCP-yhteyden kautta. listään kirjain X (TCPX tai TCPSX). Tällä hetkellä muoto on yksinkertainen, ei siis xml-muotoinen, kuten henkilökohtaisissa ohjelmissa.

### A2.2 Lähiverkon konfigurointi UDP-tiedonsiirtoa varten

Ohjelman toiminta edellyttää vain, että yhteys koneiden välillä syntyy, joten
mitään tavanomaisesta TCP/IP-pohjaisen lähiverkon konfiguroinnista poikkeavaa
ei tarvita. Jos käytettävät koneet eivät ole muuten samassa lähiverkossa on
tyypillisesti toteutettava seuraavat toimenpiteet:

- Rakenna fyysinen verkko, joka koostuu yhdestä tai
  useammasta kytkimestä (switch) tai keskittimestä (hub) sekä kaapeleista, jotka
  liittävät tietokoneet kytkimiin tai keskittimiin. Keskittimiin perustuva 10
  Mb/s ethernet-verkko lienee riittävä suurimpiinkin tulospalvelujärjestelmiin.- Kaikki konfigurointimuutokset tehdään verkkokorttiin
    liittyviin tcp/ip-asetuksiin (lukuunottamatta niitä tapauksia, joissa yhteys
    otetaan puhelinlinjan kautta, mutta näissä tapauksissa ei yleensä mitään
    tarvitse muuttaa). Kirjoita muistiin aiemmat tcp/ip-verkon konfigurointitiedot
    ennen kuin muutat niitä- Poista käytöstä ip-numeron automaattinen haku (jos se on käytössä) ja määrittele
      tietokoneiden ip-numerot ja aliverkon peite siten, että tietokoneet saavat yhteyden
      toisiinsa. Useimmiten kannattaa toteuttaa verkko niin, että kaikki paikallisverkossa
      olevat tietokoneet ovat samassa aliverkossa. Tämä tapahtuu esimerkiksi niin, että
      kaikkien koneiden ip-numerot ovat tyyppiä 192.168.1.xxx ja aliverkon peite on
      255.255.255.0. Jos toimitaan samassa lähiverkossa, ei reitittimen eikä nimipalvelimen
      ip-numeroilla ole yleensä merkitystä.

Mikään ei estä käyttämästä reitittimen toisiinsa liittämiä erillisiä lähiverkkoja,
jos tälle on joitain perusteita. Yhteys voidaan muodostaa myös ulkoisen internet-yhteyden kautta.
Täten on mahdollista liittää verkkoon esimerkiksi kilpailukeskuksesta erillään oleva
kisakanslia tai internet-palveluja tarjoava tietokone. Ulkoista verkkoa voidaan käyttää myös,
jos halutaan liittää tulospalveluverkkoon tietokoneita esimerkiksi gprs-puhelinyhteyden
tai modeemiyhteyden kautta. Gprs-verkon käytössä on kuitenkin rajoituksia, jotka riippuvat
palveluntarjoajasta sekä käyttöön valitusta palvelusta. Yleensä UDP yhteys toimii vain
gprs-yhteydestä kiinteään verkkoon tapahtuvalle siirrolle, jos sillekään.

- Yleensä on yksinkertaisinta käyttää koneiden
  tunnistamiseen ip-numeroita, mutta haluttaessa voidaan käyttää myös verkon
  tunnistamia nimiä. Jotta ei oltaisi riippuvaisia nimipalvelimesta, voidaan
  nimet lisätä tiedostoon hosts (ellet tiedä, kuinka tämä tehdään, unohda asia
  ja käytä ip-numeroita).- Mahdollisesti käytössä olevat palomuuri-ohjelmat poistetaan käytöstä (tai
    niissä määritellään käytettävä verkko-osoitealue paikallisverkoksi, jossa kaikki
    liikennöinti on sallittua).

### A2.3 Tiedonsiirron erikoistoimet

Päävalikon valinnasta *Tiedostot /
Tiedonsiirtoasetukset* pääsee tekemään seuraavia erikoistoimia

- keskeyttämään ja uudelleen käynnistämään
  tiedonsiirron joko kokonaan tai joko lähtevien tai saapuvien sanomien osalta
  ja joko kaikkien yhteyksien tai yksittäin valittavan yhteyden
  osalta.

  - pyytämään aiemmin lähetettyjen sanomien
    uudelleenlähetyksen valiten minuutteina jakson, jota pyyntö koskee

    - tarkastelemaan tiedonsiirron varmistustiedoston
      COMFILE.DAT sisältöä, josta selviää, millaisia sanomia kyseessä oleva
      tietokone on eri yhteyksiin lähettänyt.