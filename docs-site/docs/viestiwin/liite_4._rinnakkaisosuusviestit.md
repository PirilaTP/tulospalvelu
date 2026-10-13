# Liite 4. Rinnakkaisosuusviestit

## Liite 4. Rinnakkaisosuusviestit

Ohjelmiston vakioversiota voidaan käyttää myös
rinnakkaisosuusviesteissä, kuten Halikkoviestissä ja eräissä maakuntaviesteissä.
Jotta tarvittavat lisäpiirteet saataisiin käyttöön on

- toiminnossa *Kilpailun luominen ja perusominaisuudet*
  - ilmoitetaan rinnakkaisjuoksijoiden maksimilukumäärä
    (esimerkiksi Halikkoviestissä 3)- osuuksien tilavaraus tehdään joukkueen maksimikoon mukaisesti
      (esimerkiksi Halikkoviestissä 15)- sarjat määriteltävä siten, että yhtä sarjaa
    käsittelevällä kaavakkeella
    - ilmoitetaan osuuksien lukumäärä peräkkäisten
      osuuksien määränä, ei joukkueeseen kuuluvien juoksijoiden lukumääränä
      (Halikkoviestissä siis 7 eikä 15)- ilmoitetaan sarjaa koskeva rinnakkaisten
        juoksijoiden maksimiluku (osassa sarjoista tämä voi olla 1, jolloin kyseinen
        sarja on normaali viestin sarja)- tällöin tulee osuuskohtaisiin tietoihin mahdollisuus
          merkitä rinnakkaisten juoksijoiden lukumäärä kullakin osuudella. Joukkueen
          juoksijoiden lukumäärä on näiden arvojen summa- kaikki muut osuuskohtaiset tiedot ilmoitetaan siten, että rinnakkaisia
            osuuksia ei käsitellä erikseen.- Rinnakkaisosuuksia sisältävässä sarjassa on osuuksien lukumääriä koskevat
      muutokset tehtävä yhden sarjan kaavakkeella, sarjataulukko hylkää sinne
      kirjoitettavat tätä koskevat muutokset.

Ohjelma käyttää lähes aina rinnakkaisosuuksille tunnuksia, jotka muodostuvat
osuuden järjestysnumerosta ja kirjaimesta A, B, ... Täten esimerkiksi
Halikkoviestin osuudet ovat

- 1- 2A, 2B, 2C- 3A, 3B, 3C- 4A, 4B, 4C- 5A, 5B, 5C- 6- 7

Joissain tiedostoissa käytetään kuitenkin numerointia,
joka vastaa juoksijan sijaintia kilpailijatiedoissa, jolloin esimerkiksi
Halikkoviestin osuudet numeroidaan 1-15. Tämä koskee mm. csv-muotoista
kilpailijatietojen siirtotiedostoa, jonka ohjelma kirjoittaa ja lukee tulkiten osuuden
otsikkorivin mukaan. Monisarjaisissa rinnakkaisosuusviesteissä käytetään pelkkää
numeroa myös leimantarkastusnäytöllä, koska osuuden numeromuotoisen ja
osuuskoodin välinen yhteys eivät tällöin ole yleensä samat eri
sarjoissa.

Kun sarjassa on rinnakkaisosuuksia, ottaa tämän automaattisesti huomioon
tulosten laskennassa, seurantanäytöillä ja tulosluetteloissa. Tulosluetteloiden
muotoilussa ei kenttien leveyksiä kuitenkaan kasvateta automaattisesti, joten
käyttäjän on muutettava muotoilua mm. osuuskohtaisten tulosteiden osalta;
rinnakkaisosuuden tunnus (esim. 105-3A) mahtuu numerokenttään vain, jos kenttä
on riittävän leveä. Kaikki
osuudet sisältävät lopputulokset eivät vaadi muutosta muotoiluun.

Seurantanäytöllä ja tulosluetteloiden otsikoissa *Avoinna* tarkoittaa

- kilpailussa, jossa on rinnakkaisosuuksia: niiden joukkueiden lukumäärää,
  joilla kyseisellä osuudella ei ole aikaa eikä merkintää hylkäyksestä,
  keskeyttämisestä tai ei-lähtemisestä ja joiden aiemman osuuden merkintä ei ole
  sulkenut osuutta. Aiemman osuuden merkinnän takia suljettujen joukkueiden
  myöhemmät osuudet eivät siis ole avoimia. Rinnakkaisosuudella (esim. 3A, 3B,
  3C) joukkue on avoinna, kunnes osuuden tulos on ratkennut.
- tavallisessa viestissä, jossa rinnakkaisosuuksia ei ole: yksittäisten
  osanottajien lukumäärää, joilla ei ole aikaa tai merkintää hylkäyksestä,
  keskeyttämisestä tai ei-lähtemisestä. Mukana ovat myös ne osuudet, jotka on
  suljettu aiemman osuuden tällaisen merkinnän takia, ellei merkintää ole
  ulotettu myöhemmille osuuksille.

## Tulosteet rinnakkaisosuuksia sisältävässä kilpailussa

*Osuuskohtaiset tulokset* tulostetaan rinnakkaisosuuksia sisältävässä
kilpailussa juoksijakohtaisena listana: jokainen juoksija on omalla
rivillään osuusajan mukaisessa järjestyksessä, ja rinnakkaisilla osuuksilla
numerokentässä on joukkueen numero ja osuuden tunnus (esimerkiksi 105-3A).
Samat ajat saavat saman sijan. Listan otsikon luvut (tuloksia, keskeyttäneitä,
hylättyjä, avoinna) ovat tässä listassa juoksijoiden lukumääriä.
Joukkuekohtaisessa listassa (esim. *Tulokset*) samat luvut ovat joukkueiden
lukumääriä.

Tulostusten *Tulostettavien rajaus* -valinnoilla *Hylätyt*, *Keskeyttäneet*,
*Ei-lähteneet* ja *Avoimet* osuuskohtainen lista sisältää vain valitun tilan
juoksijat; tila on kunkin juoksijan oma tila, ja avoimista jätetään pois
aiemman osuuden merkinnän takia suljettujen joukkueiden juoksijat. Rinnakkaisosuudella
ensimmäisen maaliin tulleen juoksijan tila ratkaisee joukkueen osuuden tilan
(ks. alla), mutta juoksijakohtainen lista näyttää myös muiden rinnakkaisten
juoksijoiden omat hylkäykset ja keskeytykset, joten sen luvut voivat poiketa
joukkuekohtaisen listan luvuista. Lista, jossa ei ole yhtään riviä,
kirjoitetaan html-tiedostoon otsikkoineen ja lukuineen; tekstitiedostoon ja
kirjoittimelle tyhjää listaa ei kirjoiteta mitään.

Valinta *Kaikki osuudet* kirjoittaa kaikki osuusajat yhteen listaan
aikajärjestyksessä. Tekstitiedostossa sija on osuudella saavutettu sija,
kirjoittimelle kirjoitetaan juokseva järjestysnumero.

Samat osuuskohtaiset rajaukset toimivat myös tavallisessa viestissä: osuuskohtainen
*Hylätyt*-, *Keskeyttäneet*-, *Ei-lähteneet*- tai *Avoimet*-lista kirjoittaa
niiden juoksijat (aiemmin tulostui tyhjä tiedosto).

## Rinnakkaisosuuden ensimmäinen maaliin tullut ratkaisee ja käynnistää seuraavan osuuden

Rinnakkaisosuuksia sisältävässä sarjassa voidaan valita kaksi toisistaan
riippumatonta sääntöä. Asetukset tehdään *Sarjan lisäys ja muokkaus*
-kaavakkeen taulukon kahdella rivillä, jotka näkyvät automaattisesti, kun
sarjassa on vähintään yksi rinnakkaisosuus.

1. *Rinnakkaisosuuden tuloksen ratkaisee ensimmäisenä maaliin tullut, muita
   ei tarvita (1=kyllä)*. Arvo 1 kirjoitetaan rinnakkaisen osuuden (esim. 3)
   sarakkeeseen. Tällöin joukkueen osuustulosta ja -tilaa ei odoteta kaikilta
   rinnakkaisilta juoksijoilta (esim. 3A, 3B ja 3C), vaan ne määräytyvät heti,
   kun *ensimmäinen* heistä saapuu maaliin.
2. *Osuus lähtee, kun edellisen rinnakkaisosuuden ensimmäinen juoksija tulee
   maaliin (1=kyllä)*. Arvo 1 kirjoitetaan sen osuuden (esim. 4) sarakkeeseen,
   joka seuraa rinnakkaisosuutta. Osuuden lähtöaika on silloin ensimmäisen
   rinnakkaisen juoksijan maaliintuloaika. Jos osuus on itsekin rinnakkainen,
   kaikki sen paikat lähtevät yhdessä.

Säännöt voi valita erikseen:

| Ratkaisee ensimmäinen (osuus 3) | Lähtee ensimmäisestä (osuus 4) | Vaikutus |
|---|---|---|
| kyllä | kyllä | Nuorten Jukolan sääntö: ensimmäinen ratkaisee tuloksen ja käynnistää seuraavan osuuden |
| ei | kyllä | Seuraava osuus lähtee ensimmäisestä, mutta joukkueen osuustulos ja -tila vaativat kaikkien rinnakkaisten tulokset: osuusaika on hitaimman aika ja yhden juoksijan hylkäys tai keskeytys hylkää tai keskeyttää joukkueen |
| ei | ei | Normaali rinnakkaisosuus: tulos odottaa kaikkia, seuraava osuus lähtee joukkueen osuustuloksesta (hitain) |
| kyllä | ei | Ensimmäisen aika on joukkueen osuusaika; tavallinen (yhden juoksijan) seuraava osuus lähtee silti tästä ajasta. Jos seuraava osuus on itsekin rinnakkainen, sen ensimmäinen paikka (esim. 4A) lähtee ensimmäisestä, toinen (4B) toiseksi maaliin tulleesta ja kolmas (4C) kolmanneksi maaliin tulleesta |

Jos osuudelle on annettu oma yhteislähtöaika (*MassStart*), se on ensisijainen
eikä kumpikaan sääntö muuta lähtöaikaa.

Vanhoissa sarjatiedostoissa ollut yhteinen asetus (*FirstFinishStarts*) luetaan
automaattisesti kummaksikin säännöksi (osuudella ratkaisee ensimmäinen ja
seuraava osuus lähtee siitä), joten aiemmat kilpailut toimivat entiseen tapaan.
Tallennettaessa asetukset kirjoitetaan *KilpSrj.xml*-tiedostoon osuuskohtaisilla
tunnisteilla *OnlyFirstFinishCounts* ja *StartsAtPreviousFirstFinish*.

Kun osuudella ratkaisee ensimmäinen maaliin tullut:

- Joukkueen osuustulokseksi ja sijoitukseksi kirjataan sen rinnakkaisen
  juoksijan aika, joka ensimmäisenä saapuu maaliin — muiden rinnakkaisten
  juoksijoiden aikoja ei enää odoteta.
- Jos seuraavalla osuudella on käytössä lähtö edellisen ensimmäisestä, sama
  maaliintuloaika käynnistää myös joukkueen seuraavan osuuden lähtöajan, eli
  vaihto tapahtuu heti ensimmäisen rinnakkaisen juoksijan saapuessa, ei vasta
  kun kaikki kolme ovat vaihtaneet.
- Jos ensimmäisenä maaliin tullut juoksija myöhemmin hylätään, joukkueen
  osuustulos perii tämän hylkäyksen — sitä ei korvata jonkun toisen
  rinnakkaisen juoksijan (esim. myöhemmin maaliin tulleen) hyväksytyllä
  ajalla.
- Ensimmäisenä maaliin tulleen juoksijan hylkäys tai keskeytys on joukkueen
  osuuden tila, vaikka muut rinnakkaiset juoksijat olisivat hyväksyttyjä.
  Muiden rinnakkaisten juoksijoiden hylkäys tai keskeytys ei sen sijaan
  vaikuta joukkueen tulokseen, kun ensimmäisenä maaliin tullut on hyväksytty.
- Ratkaisevaa on ensimmäinen tapahtuma, jolla on aika. Hylkäys tai
  keskeytys ratkaisee joukkueen osuuden vain, jos sille on kirjattu aika
  (maaliaika-kenttään): silloin se pysyy ratkaisevana, vaikka joku muu
  tulisi myöhemmin hyväksyttynä maaliin. Ilman aikaa olevaa hylkäystä tai
  keskeytystä ei käsitellä joukkueen tuloksena, kun joku muu on vielä
  matkalla — osuus pysyy avoimena. Jos kaikki rinnakkaiset juoksijat ovat
  ulkona eikä kenelläkään ole aikaa, joukkue saa huonoimman merkinnän.
- Rinnakkainen juoksija, jolle ei ole kirjattu nimeä, otetaan mukaan, jos
  hänellä on Emit-koodi tai aika.
- Kun ensimmäinen maaliaika on kerran määrännyt seuraavan osuuden lähdön,
  se pysyy voimassa, vaikka ohjelma myöhemmin laskisi sarjalle uuden
  yhteislähtöajan — yhteislähtö ei siis avaa jo lukittua lähtöä uudelleen.

Ominaisuus näkyy myös seuranta- ja tulosnäytöillä:

- *Tilanne pisteessä* -näytöllä ratkaisevan (ensimmäisenä maaliin tulleen)
  juoksijan nimi lihavoidaan rinnakkaisosuuden rivillä; muiden rinnakkaisten
  juoksijoiden nimet näkyvät tavallisella tyylillä. Suodattimet *1 puuttuu*
  ja *2 puuttuu* piilotetaan automaattisesti, kun tarkasteltava osuus
  käyttää tätä asetusta, koska "puuttuvien" määrä ei enää ole merkityksellinen
  tieto sen jälkeen kun ensimmäinen aika on ratkaissut tuloksen; suodatin
  *Täysi osuus* toimii edelleen.
- *Joukkuetiedot*-näytön väliaikataulukossa (*Näytä väliajoista* → *Tulos*)
  näytetään joukkueen yhteinen tulos vain ratkaisevan juoksijan sarakkeessa;
  muiden rinnakkaisten juoksijoiden sarakkeissa näkyy 00:00:00. Jos halutaan
  nähdä jokaisen rinnakkaisen juoksijan oma henkilökohtainen väliaika,
  valitaan *Näytä väliajoista* → *OsTls*; tässä tilassa taulukko on
  kuitenkin vain luettavissa, ei muokattavissa.