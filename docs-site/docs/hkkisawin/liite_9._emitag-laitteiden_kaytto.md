# Liite 9. EmiTag-laitteiden käyttö

## Liite 9. EmiTag-laitteiden käyttö

#### A9.1 Toimintamallit

Ohjelmieni tukemat
toimintamallit perustuvat malleihin, joita on kuvattu Emitin ohjeissa
"Instructions for use Emit Finish Station ECB1" ja "emiTag brikkesystem –
Koblingsskema, reservedeler og ekstrautsryr", joista jälkimmäisestä löytyy
lähinnä erilaisia kaaviokuvia laitteiden kytkennästä ja yleisistä
järjestelyistä.

Ajanoton osalta toimintamalli on olennaisesti kaaviokuvan
"emiTag langrenn, enkel" tai "emiTag langrenn, avansert" mukainen.
Leimantarkastuksesta ei vastaavaa dokumentaatiota ole tiedossani.

Tietokoneeseen liitetään tyypillisesti seuraavia laitteita

- maaliasema ECB1

  - väliaika-asema ETS1

    - leimantarkastuslukija ECU1 tai MTR5

Laite liitetään tietokoneella USB-kaapelilla, RS485
(virtasilmukka) –yhteydellä MOXA –USB-muuntimen avulla tai RS232 –yhteydellä.
Myös USB-liitännät näkyvät ohjelmalle sarjaportteina, mutta yhteyden nopeus riippuu liitännän
tyypistä:

- suora USB-liitäntä 115200
  b/s

  - RS485 perusteinen yhteys 19200
    b/s

    - RS232 –yhteys 9600
      b/s.

Ohjelmat tukevat sekä edellä lueteltujen
laitteiden suoraa liittämistä että väliaikatietojen noutamista Emitin
tietokantapalvelimelta, joka kerää ETS1-aseman GPRS-yhteyden kautta välittämiä
väliaikahavaintoja.

#### A9.2 Ohjelman parametrit

EmiTag-toimintojen käyttöä varten on tarjolla käynnistysparametrit:

```
EMITAGy=x
EMITAGy=x/U
EMITAGy=x/C
```

jotka määrittelevät sarjaportin numeron laitteen liitännässä
käytetylle yhteydelle. y on yhteyden numero ja
x sarjaportin numero.

Parametrin lopussa oleva
/U kertoo yhteyden olevan nopea USB-yhteys ja
/C RS485-yhteys. Muuten nopeus on 9600 b/s.

ECAIKA

kertoo, että tapahtuman ajaksi kirjataan ECB1-, ETS1-,
ECU1-tai MTR5-laitteen ilmoittama aika eikä tietokoneen aikaa.

#### A9.3 EmiTag laitteiden ohjaus ja muistitietojen siirto

Leimantarkastusnäytön valinnassa *Toiminnot /
emiTag-luennan ohjaus* voidaan muuttaa laitteen asetuksia sekä pyytää laitetta lähettämään muistissaan olevia tietoja.

Tältä näytöltä voidaan synkronoida laitteen kello tietokoneen
kelloon. Useimmat laitteet on synkronoitava aina uuden käynnistyksen jälkeen.

Näytöllä voidaan asettaa laitteen toimintamoodi,
jonka on aina oltava 0 sekä koodi, mikä voi olla tarpeen
väliaikapisteen tunnistamiseksi, jolloin käytetään koodia väliltä 65-239. Koodit
240, 243, 250 ja 253 pyytävät laitteen toimimaan leimantarkastusmoodissa. Koodit
240 ja 243 tuovat koneelle 5 viimeisen päivän tiedot eli usein turhaakin tietoa.
Koodit 250 ja 253 tuovat vain viimeisen kilpailun ja ovat siten yleensä
paremmat. Koodi 248 on käytössä maalin ajanotossa ECB1-laitteen avulla.

Vastaavat toiminnot ovat suurelta osin käytettävissä
ohjelman *HkMaali*  päävalikon valinnasta
*A)setukset / Emit* siirrytään laitteen ohjausnäytölle,
minne pääsee
myös leimantarkastusnäytöltä näppäimellä 'M' (viittaa sanaan MTR).

Kaikki emiTag-lukijalaitteet tallentavat lukemansa
tiedot muistiin. Nämä muistiin tallennetut tiedot voidaan lukea jälkikäteen.
Luettavien tietojen valitseminen sanomanumeroiden perusteella voi olla lähes
mahdotonta, koska numerointi ei suoraan vastaa luettujen leimojen määrää. Täten
voi olla helpointa lukea viimeisimmän päivän tiedot tai kaikki tiedot
erilliseen tiedostoon ja valita sieltä rivit, jotka otetaan käsiteltäviksi
valinnassa *Toiminnot / Lue tietoja tiedostosta*
.

#### A9.4 Aikojen haku Emitin aikapalvelimelta

Väliaika-asema ETS1 pystyy lähettämään aikoja
GPRS-yhteyden kautta Emitin ylläpitämälle tietokantapalvelimelle.

Parametri

```
ETGPRSy=xxxxx,uuuuu,vvvvv,zzzzz
```

tai

```
ETGPRSy=xxxxx/L
```

ottaa käyttöön http-protokollaan perustuvan väliaikojen noudon Emitin palvelimelta,
xxxxx, uuuuu, vvvvv ja zzzzz ovat niiden korkeintaan neljän ETS1-laitteeen sarjanumerot, jotka ovat
tiedot tuottaneet. Lisämerkintä /L merkitsee, että
tiedonsiirron aloittamista lykätään, kunnes se käynnistetään ohjelmasta
erikseen. y
kertoo yhteyden numeron tietojen haulle.

Toinen, mutta ilmeisesti sekä hankalampi että hitaammin
toimiva mahdollisuus, on hakea tiedot SQL-kyselyillä suoraan tietokantapalvelimelta. Tällöin parametri on

ETSQLy=xxxxx,uuuuu,vvvvv,zzzzz

Tätä parametria käytettäessä on avattava SSH-tunneli
tulospalvelun tietokoneelta tietokantapalvelimelle. Tunnelin avulla saadaan
palvelin näkymään ohjelmalle aivan niin kuin se olisi paikallinen MySQL-palvelin
tunnelimäärittelyn mukaisessa portissa (ohjelman oletus on 53306). Tätä
toimintoa ohjataan kaavakkeelta, joka avataan ohjelman HkKisaWin
ajanottokaavakkeen valinnasta *Toiminnot / Hae MySQL-kannasta*. Ohjelma
ei tällä hetkellä avaa yhteyttä automaattisesti, vaan se on avattava
painikkeesta *Avaa yhteys*, kun parametrit ovat oikein ja tunneli on
avattu esimerkiksi ohjelman plink avulla. Yhteyden
avaamisen onnistuminen ilmenee siitä, että ohjelma
kertoo palvelimella olevien tietueiden lukumäärän.

Ohjelman plink käyttö
edellyttää avaintiedostoa sekä tiedot osoitteesta ja portista, joihin yhteys on
otettava. Nämä tiedot on saatu Emitiltä ja niiden saaminen käyttöön edellyttää,
että käytölle on lupa Emitiltä. Ohjelman plink tarkka komentorivi on saatavissa samalla kuin
avaintiedosto. plink on saatavana vapaasti osana
*PuTTY*
-ohjelmistoa.

Kun yhteys on avattu, voidaan tietoja hakea painikkeen
*Hae* avulla. Normaalisti käytetään kuitenkin
jatkuvaa hakua, joka käynnistyy, kun se valitaan käyttöön. Haku tapahtuu
määritellyn hakuvälin välein. Aikaraja kannattaa asettaa lähelle kilpailun
todellista käynnistymistä, jotta ohjelma ei hakisi saman laitteen joskus aiemmin
tuottamia aikoja. **Huom. Aikaraja on ilmoitettava Norjan aikana.**

Kun ohjelma on löytänyt yhdenkin tietueen, joka on
pyydetyltä laitteelta ja päivätty aikarajaa myöhäisemmäksi, ilmoittaa ohjelma
ensimmäisen ja viimeisen ehdot täyttävän tietueen järjestysnumeron tietokannassa
ja ryhtyy hakemaan tietoja. Tiedot haetaan normaalisti korkeintaan 100
tietueen erissä, mutta tätä arvoa voi muuttaa. Haun tapahduttua, ohjelma
päivittää tiedon kunkin laitteen ylimmästä sanomanumerosta. Ohjelma
kohdistaa myöhemmät haut vain
tietoihin, joiden sanomanumero on suurempi, kuin ruudulla ilmoitettu viimeinen haettu
tietue. Jos kentässä on jostain syystä liian suuri numero, ei sanomia haeta.
Äskettäisiä sanomia voidaan
hakea uudelleen pienentämällä tätä numeroa.

Jatkuva haku on keskeytettävä ennen laitenumeron tai
viimeisen sanoman numeron
muokkaamista ja käynnistettävä sitten uudelleen.

Kumpaankin tapaa hakea aikoja Emitin palvelimelta voidaan ohjata seuraavilla parametreilla

ETHAKUVÄLI= xx

Valitsee peräkkäisten väliaikahakujen välin sekunnin kymmenyksinä

ETDATE= vvvvkkpp

Ensimmäinen päivä, josta alkaen tietoja haetaan muodossa
vvvvkkpp (esim. 20131124
). Ellei haettu, haetaan vain hakupäivän tiedot.

ETTIME= tt:mm:ss

Kellonaika, jota vanhempia tietoja ei haeta muodossa
tt:mm:ss (Esim. 13:10:00
). Ellei annettu haetaan kaikki kellonajat.

```
ETHOST=ip:portti
```

kertoo yhteyden html-yhteyden ip-numeron ja portin. Ellei annettu,
käytetään numeroita 195.159.103.189:1379

Ohjelman muista parametreista käytetään usein
parametreja AIKALUKIJAy=VAINx, missä y ja x voivat myös puuttua, sekä
LÄHDEPISTEET (kts [luku 5.2](5.2_tunnisteisiin_perustuva_ajanotto.md)).
Parametrin LÄHDEPISTEET kanssa kannattaa
useimmiten antaa parametri AIKALUKIJA=VAIN.

Aina tarvitaan siis joko parametri EMITAG,   ETGPRS tai ETSQL
. Eri parametreja ja/tai useampia EMITAG
-parametreja voi olla käytössä samanaikaisesti, kun yhteyksille on annettu eri
numero.

#### A9.5 Ohjelman lisätoiminnot

emiTag-laitteilta tulevia tietoja käsitellään samaan
tapaan kuin muiltakin emit-leimasimilta tulevia tietoja. Ellei parametria AIKALUKIJAy=VAINx ole, olettaa ohjelma laitteen tuottavan
leimaustietoja. Ajanottotietoja ei käsitellä ilman parametria AIKALUKIJA tai ETGPRS
.

Ajanottonäytöltä voi näppäimillä *Alt-W* siirtyä
valintaan, jossa käynnistetään tai keskeytetään väliaikojen haku Emitin
palvelimelta, kun parametri ETGPRS
on annettu. Valinnassa voidaan pyytää myös lähettämään kaikki
tiedot uudelleen. Muuten ohjelma pyytää toistamaan 10 viimeistä tietoa
varmistaakseen, että kaikki tiedot saadaan ilman suuria viiveitä vaikka
tiedonsiirrossa olisi ollut häiriö.

#### A9.6 Käyttö hiihtokilpailussa

Parametreista kannattaa yleensä antaa EMITAG, ECAIKA sekä ETGPRS, jos käytössä on GPRS-yhteys väliaikojen
siirtämisessä. Muuten noudatetaan sanoja käytäntöjä kuin muissakin tunnisteisiin
perustuvissa ajanottotavoissa, kuten leimattaessa Emit-kortilla maaliviivalla.
(kts [luku 5.2](5.2_tunnisteisiin_perustuva_ajanotto.md)).

Jos lähtöportti on liitetty ECB1-laitteen kautta
kannattaa antaa parametri

PISTEET= MMMLM

tai haluttaessa lähtijän automaattinen päättely lähtöajan
perusteella 3 sek rajoissa

PISTEET=MMMLM/sarjannimi

Jos päättely halutaan useammalle sarjalle, kirjoitetaan
sarjannimen paikalle KAIKKI.