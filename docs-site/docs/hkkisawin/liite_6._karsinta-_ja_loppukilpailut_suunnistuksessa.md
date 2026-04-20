# Liite 6. Karsinta- ja loppukilpailut suunnistuksessa

## Liite 6. Karsinta- ja loppukilpailut suunnistuksessa

Karsinnan ja loppukilpailun sisältävissä
suunnistuskilpailuissa on tyypillisesti vain ensimmäiseen vaiheessa mukana
olevia karsintasarjoja sekä vain toisessa vaiheessa kilpailtavia
loppukilpailuja. Koska kilpailijat kilpailevat eri vaiheissa eri sarjoissa on
kilpailun määrityksissä valittava, että sarja muuttuu vaiheesta toiseen, ja
useimmiten myös, että rintanumero vaihtuu vaiheesta toiseen.

Joissain tapauksissa käydään lähtöjärjestyksen määräävä
alkukilpailu ilman karsintaa ensimmäisessä vaiheessa ja
sitten saman sarjan loppukilpailu toisessa vaiheessa, jolloin sarja on
mukana molemmissa vaiheissa. Ilmoittautuneet kirjataan tyypillisesti ensin
sarjaan, joka on myös A-finaali, ja siirretään siitä karsintasarjoihin.
Ilmoittautumisten kirjausvaiheessa on parempi, että tähän käytettävät sarjat on
määritelty kaikki vaiheet sisältäviksi, mutta sen jälkeen, kun siirto
karsintasarjoihin on tapahtunut, voidaan vaiheet merkitä todellisia
kilpailuvaiheita vastaaviksi.

Ilmoittautumisten tallentamisen jälkeen on tyypillisesti
merkittävä rankipisteet osanottajatietoihin rankiin perustuvaa lähtöjärjestyksen
määräämistä varten. Tämä tapahtuu rankipisteet sisältävästä csv-tyyppisestä
tiedostosta ohjelman toiminnossa *Osanottajat / Siirrä lisätietoja*
perustuen lisenssinumeroon kilpailijan tunnistavana kenttänä.

Siirto karsintasarjoihin tehdään normaalisti toiminnassa
*Valmistelu / Arvonta ja numerointi* välilehdellä *Sarjojen
jakaminen*. Jakoperuste on useimmiten
*Rankin pohjalta tasapuolistettu* . Kun
jako on tehty voidaan lähtöjärjestys määrätä välilehdellä *Arvonta* ja
lopuksi numerointi välilehdellä *Numeroiden antaminen*. Karsintasarjoihin
jakaminen kannattaa usein kohdistaa vain 1. vaiheeseen, jolloin kilpailijan
perussarjaksi jää sarja, johon ilmoittautumiset on kirjattu. Tällöin on
kuitenkin huolehdittava, että kilpailijat ovat toisen vaiheen osalta jossain
sellaisessa sarjassa, johon osanottajia ei poimita karsintakilpailun
perusteella. Tällaiseksi sarjaksi käy joko yksi karsintasarjoista tai
seuraavassa käsiteltävässä tapauksessa B-finaali. Siirtäminen yhteen
karsintasarjaan onnistuu, vaikka sarja ei osallistu 2. vaiheeseen. Joissain
tapauksissa on syytä määritellä ylimääräinen sarja tähän tarkoitukseen, jotta
vältettäisiin ongelmat online-tulospalvelussa. Näistä valinnoista lisää
alempana.

Osassa kilpailuista järjestetään B-finaalit käyttäen
ennalta arvottuja lähtöaikoja. Tässä tapauksessa on kaikki kilpailijat
merkittävä asianomaisiin B-finaaleihin ennen arvontaa. Merkintä tapahtuu
toiminnossa *Osanottajat / Seuranimien ja sarjojen kopioinnit*
valitsemalla siellä *Siirrä kilpailijoita sarjasta toiseen*. Tämän
jälkeen valitaan kopioitavaksi tiedoksi *Perussarja* ja kohteeksi *2.
vaiheen sarja*, sarjavaihdon valinnoissa ilmoitetaan sitten kyseisten
kilpailijoiden perussarja ja uudeksi sarjaksi B-finaalin sarja (perussarjan
sijaan voi joskus olla parempi käyttää tunnistukseen 1. vaiheen sarjaa).

Edellisen kappaleen mukainen menettely sopii myös
tapauksiin, joissa B-finaali muodostetaan yhdessä A-finaalin kanssa, siten, että
kilpailijat siirretään johonkin sellaiseen sarjaan, joka ei osallistu 2.
vaiheeseen. Jos toisessa vaiheessa käytetään B-finaalin jakamista rinnakkaisiin
sarjoihin karsinnan jälkeen, on parempi määritellä ylimääräinen työsarja vain
tätä tarkoitusta varten niin, että B-finaalien muodostamisen jälkeen ei tähän
työsarjaan jää osanottajia minkään vaiheen osalta.

Karsinnan jälkeen muodostetaan finaalit käyttäen
menettelyä, jota kuvataan luvussa 10.4. Täten
muodostetaan kerralla kaikki ne finaalin tasot, jotka määräytyvät karsinnan sijoitusten
mukaan. Jos kaikki A-finaalin ulkopuolelle jäävät osallistuvat yhteen B-finaalisarjaan
ilman, että heidän lähtöjärjestyksensä määräytyisi karsinnan tuloksista, voidaan
heidät sijoittaa sinne jo ennalta. Jos lähtöjärjestys
määräytyy karsinnasta, sijoitetaan heidät ennalta muuhun sarjaan ja
siirretään B-finaaliin samalla kuin A-finaali muodostetaan. Jos B-finaali jaetaan rinnakkaisiin
sarjoihin, on kilpailijat sijoitettava johonkin muuhun sarjaan, mieluimmin
ylimääräiseen apusarjaan, joka on merkitty 2. vaiheeseen
osallistuvaksi, jotta jako B-finaaleihin onnistuisi.

Kaikki finaalien muodostamiseen liittyvät toimet voidaan
tehdä karsintavaiheen tulospalvelutilassa. Kaavakkeen *Loppukilpailun
muodostaminen* käyttö onnistuu vain
tässä tilassa. Kaavakkeella *Arvonta ja numerointi* tehtävät vaiheet voi
tehdä sekä 1. vaiheen tulospalvelutilassa että esivalmistelutilassa. Jos
toimitaan tulospalvelutilassa, toimii myös tulospalveluverkon tiedonsiirto ja
kaikkien verkossa olevien koneiden tiedostot päivittyvät samalla sen mukaan kuin
yhteyksiä on käytössä. Ellei aikataulu ole tiukka, voi kuitenkin olla parempi
tehdä toimet erikseen yhdellä koneella ja kopioida tiedostot lopuksi kaikille 2.
vaiheessa käytettäville koneille, koska täten saadaan paras varmuus siitä, että
kaikkien koneiden tiedostot ovat todella samat 2. vaiheen alkaessa.

Koska loppukilpailujen muodostaminen on suhteellisen
monimutkainen toiminto, on aina syytä tehdä kilpailijatiedostosta kopio ennen
homman aloittamista. Kopioita toi tehdä myös eri välivaiheissa käyttäen ohjelman valinnasta