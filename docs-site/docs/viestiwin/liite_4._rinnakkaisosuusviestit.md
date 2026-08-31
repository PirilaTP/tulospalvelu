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
käyttäjän on muutettava muotoilua mm. osuuskohtaisten tulosteiden osalta. Kaikki
osuudet sisältävät lopputulokset eivät vaadi muutosta muotoiluun.

Ohjelma näyttää seurantanäytöllä avoinna olevien
lukumäärän niiden yksittäisten osanottajien määränä, joilla ei ole aikaa tai
merkintää hylkäyksestä, keskeyttämisestä tai ei-lähtemisestä. Mukana ovat myös
ne osuudet,
jotka on suljettu aiemman osuuden tällaisen merkinnän
takia, ellei merkintää ole ulotettu myöhemmille osuuksille.

## Rinnakkaisen osuuden ensimmäinen maaliaika käynnistää seuraavan osuuden lähdön

Rinnakkaisosuuksia sisältävälle sarjalle voidaan osuuskohtaisesti valita, että
joukkueen tulosta ei lasketa vasta kun kaikki kyseisen osuuden rinnakkaiset
juoksijat (esim. 3A, 3B ja 3C) ovat maalissa, vaan heti kun *ensimmäinen*
heistä saapuu maaliin. Asetus tehdään *Sarjan lisäys ja muokkaus*
-kaavakkeen taulukossa rivillä *Rinnakkaisen osuuden ensimmäinen maaliaika
käynnistää seuraavan osuuden lähdön (1=kyllä)*. Rivi näkyy taulukossa
automaattisesti, kun sarjassa on vähintään yksi rinnakkaisosuus, ja arvon 1
voi kirjoittaa vain niiden osuuksien sarakkeisiin, joilla on useampi kuin
yksi rinnakkainen juoksija.

Kun asetus on käytössä osuudelle:

- Joukkueen osuustulokseksi ja sijoitukseksi kirjataan sen rinnakkaisen
  juoksijan aika, joka ensimmäisenä saapuu maaliin — muiden rinnakkaisten
  juoksijoiden aikoja ei enää odoteta.
- Sama ensimmäinen maaliintuloaika käynnistää myös joukkueen seuraavan
  osuuden lähtöajan, eli vaihto tapahtuu heti ensimmäisen rinnakkaisen
  juoksijan saapuessa, ei vasta kun kaikki kolme ovat vaihtaneet.
- Jos ensimmäisenä maaliin tullut juoksija myöhemmin hylätään, joukkueen
  osuustulos perii tämän hylkäyksen — sitä ei korvata jonkun toisen
  rinnakkaisen juoksijan (esim. myöhemmin maaliin tulleen) hyväksytyllä
  ajalla.
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