# Luku 12. MySQL-tietokannan käyttö

## 12 MySQL-tietokannan käyttö

### 12.1 Yleistä

Yhteys MySQL-tietokantaan on käytettävissä sekä niin, että kaikki kilpailijatiedot joko
kirjoitetaan MySQL-tietokantaan tai luetaan MySQL-tietokannasta, että niin, että
ohjelma tallentaa jokaisen kilpailijatietoihin tulevan muutoksen välittömästi myös
tietokantapalvelimelle.

Tietokantatuki perustuu dbExpress -tietokantaliittymään. Ohjelmatoimituksen mukana tulevat
kaikki tiedostot, jotka tarvitaan yhteyden luomiseksi. Jos tietokoneessa on jo muuten käytössä
dbExpress, on mahdollista, että syntyy yhteensopivuusongelmia tiedoston dbxdrivers.ini-tiedoston
kautta. Ohjelma asentaa mukana tulevan tiedoston samaan hakemistoon kuin ohjelman HkKisaWin, mutta
toisen ohjelman yhteydessä asennettu dbxdrivers.ini saattaa ohittaa tämän tiedoston. Käytettävät
ohjelmakomponentit ovat 32-bittisen MySQL 5.1 versiosta, mutta ne ottavat yhteyden myös joihinkin
muihin MySQL-versioihin (ainakin versioon 5.5).

Yhteys tietokantaan voidaan avata käyttäen valikon valintaa "Tiedostot/MySQL" tai käyttäen parametreja

|  |  |
| --- | --- |
| SQLHOST=host | Määrittelee MySQL-palvelimen nimen (oletuksena 'localhost') |
| SQLDATABASE=database | Määrittelee MySQL-tietokannan nimen (oletuksena 'kilp') |
| SQLUSER=user | Määrittelee MySQL-tietokannan käyttäjän (oletuksena 'kilp') |
| SQLPASSWORD=salasana | Määrittelee MySQL-tietokannan käyttäjän salasanan (oletuksena 'hkkisawin') |
| SQLSTART | Pyytää avaamaan MySQL-tietokannan automaattisesti ohjelman käynnistyessä |
| SQLTALLENNUS | Pyytää avaamaan MySQL-tietokannan siten, että kaikki kilpailijatietoihin tulevat muutokset talletetaan automaattisesti |

Kun muutosten välitön tallentaminen on käytössä, varmistaa ohjelma oletuksena tiedonsiirron tiedoston SQLjonoVarm.dat avulla.
Tästä tiedostosta ohjelma päättelee, mitkä kilpailijatiedot ovat mahdollisesti jääneet lähettämättä, kun ohjelma on suljettu.

Kaikkien kilpailijatietojen siirtäminen tietokantaan pyydetään valinnassa "Tiedostot/Kirjoita siirtotiedostoon" ja
lukeminen tietokannasta valinnassa "Osanottajat/Hae tiedostosta". Kun samaa osanottoa koskevat tieto siirretään uudelleen
tietokantaan, korvaa uusi tieto vanhan.

### 12.2 Tietokannan rakenne

Kilpailijatiedot tallennetaan kolmeen tietokantatauluun: osanotot, osottopv ja tulos. Näiden taulujen luomisen toteuttavat
SQL-komennot ovat mukana jakelutiedostossa kilpsql.txt ja ne asennetaan kilpailutietojen kansioon. Kaikkien tietojen
avaimiin sisältyy kilpailun koodi joka määritellään kilpailun perusominaisuuksissa sekä kilpailijatiedon sijainti
tiedostossa KILP.DAT, mikä on muuttumaton tieto, ellei järjestystä muuteta esimerkiksi kirjoittamalla csv-tiedostoon
ja lukemalla sieltä takaisin eri järjestyksessä. (Poistetut tietueet voivat muuttaa sijainteja näin toimittaessa.)

On olennaista, että jokaiselle kilpailulle, joka
tallennetaan samaan tietokantaan, annetaan eri koodi. Täten voidaan yhteen
tietokantaan tallentaa esimerkiksi kauden kaikkien kuntosuunnistustapahtumien
osanottajat ja tulokset. Tällä hetkellä ei ohjelma anna mahdollisuutta jättää
pois esimerkiksi vakantteja, joten ne on poistettava haluttaessa jälkikäteen
esimerkiksi sopivalla SQL-komennolla.

---

 Copyright 2012, 2015 Pekka
Pirilä