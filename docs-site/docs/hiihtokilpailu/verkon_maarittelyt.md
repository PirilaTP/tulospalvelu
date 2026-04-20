# Verkon määrittelyt

Tulospalvelussa käytettävät tietokoneet ovat yhteydessä toisiinsa lähiverkon
kautta. jokainen yhteys on kahden tietokoneen välinen ja yhteyden avaaminen
tapahtuu niin, että koneista toinen odottaa yhteyspyyntöä, jonka toinen tekee.
Jotta tämä olisi mahdollista on pyynnön tekevän koneen tiedettävä vastapuolen
tunnus, joka on käytännössä useimmiten ip-numero, mutta voi olla myös nimi, joka
on liitettävissä tiettyyn ip-numeroon. Normaalisti käytettävä yhteysprotokolla
on UDP, jonka käyttö edellyttää, että ip-numeron lisäksi tiedetään toinen
numeroarvo, jota nimitetään portiksi.

Ennen kuin on mahdollista ryhtyä määrittelemään yhteyden
vaatimia parametreja on selvitettävä käytössä olevat ip-numerot tai ainakin niin
suuri osa niistä, että jokaisen kahden koneen välisen yhteyden osalta tiedetään toisen osapuolen ip-numero tai nimi, jonka perusteella ohjelma voi
selvittää ip-numeron. Tämä on yksinkertaista, kun TCP/IP-lähiverkko on määritelty käyttäen kiinteitä
ip-numeroita, mutta ongelmallisempaa, kun näin ei ole tehty. Käytettäessä tulospalveluohjelmaa
lähiverkossa on parasta käyttää kaikille koneille kiinteitä ip-numeroita. Joissain tapauksissa tarvitaan
yhteys esimerkiksi kännykkäverkon kautta, jolloin vain toisen
koneen ip-numero on tiedossa, mutta tämä riittää yhteydenottoon.

Windowsin komentoikkunassa annettu komento

`ipconfig`

tuottaa vastauksen, joka
sisältää koneen ip-numeron rivillä, joka voi olla esimerkiksi

`IPv4 Address . . . . : 192.168.1.11`

mutta tämä numero voi olla seuraavan
käynnistyksen jälkeen toinen, ellei numeroita ole määritelty kiinteiksi.

UDP-portin numerot voivat olla välillä 1-65535, joista
useimmat yli 1024 arvot ovat yleensä vapaita (poikkeuksena usimmin 8080 ja 8008,
mutta myös jotkut muut numerot voivat olla käytössä. Käytössä olevat portit voi
kysyä komennolla `netstat /a`, mutta muutkin
portit voivat olla käytössä jollain toisella hetkellä). Ohjelmat käyttävät
oletuksena arvoja alueelta 15901-15964, jotka eivät ole johtaneet käytännössä
ongelmiin. Ohjelmien käyttämät yhteydet on numeroitu ja suurin käytettävissä
oleva numero on normaalisti 64. Yhteyttä N vastaava portin oletusnumero on
15900+N.

On varmistettava, että palomuuriohjelmat eivät estä
tiedonsiirtoa. Tämä edellyttää tyypillisesti, että ohjelmien HkMaali.exe ja
HkKisaWin.exe sallitaan käyttää kaikkia yhteyksiä myös palvelimena. Jos verkko
on irti internetistä, voidaa kaikki palomuuriohjelmat poistaa tarvittaessa
käytöstä kilpailun
ajaksi.

---

 Copyright 2012, 2015 Pekka
Pirilä