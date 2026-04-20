# Perusvalinnat

Ryhdyttäessä perustamaan kilpailua on päätettävä, mihin
kansioon kilpailua koskevat tiedot sijoitetaan. Suositukseni on että kilpailulle
luodaan uusi kansio asennusvaiheessa luodun kansion Kisa alikansioksi. Kansio
voidaan luoda ohjelmalla *HkKisaWin* samalla, kun määritellään muut
perusvalinnat. Toiminta aloitetaan käynnistämällä ohjelma *HkKisaWin* ja
klikkaamalla painikekenttää *Kilpailun luominen ja perusominaisuudet*.
Tällöin avautuu kaavake *Kilpailumääritykset.*

![](../images/KilpMaar.png)

Hiihtokilpailussa on usein vain muutama sarja, jolloin ne
voidaan luoda uusina valitsemalla haluttu määrä luotavaksi. Itse sarjojen
luominen tapahtuu, kun tämä kaavake suljetaan hyväksyen valinnat. Jos kyse on
kuitenkin kilpailusta, jossa sarjoja on runsaammin ja
samankaltainen kilpailu on järjestetty aiemminkin säästetään huomattava määrä vaivaa
käyttämällä pohjana jotain aikaisempaa kilpailua klikkaamalla
*Lue pohjaksi aiemmat kilpailu- ja sarjatiedot* ja avaamalla tiedosto
KilpSrj.xml kyseisen aiemman kilpailun
tiedoista, joiden pitää olla jossain
muussa kansiossa kuin nyt käsiteltävän kansio tai nimettynä
jollain muulla nimellä.

Seuraavaksi klikataan *Valitse kilpailun hakemisto* ja siirrytään kilpailulle varattuun kansioon tai luodaan tämä
kansio käyttäen dialogi-ikkunan kansion luomistoimintoa. Kun valinta hyväksytään
luo ohjelma tyhjän tiedoston ilmoitt.cfg ja tallentaa sen valittuun kansioon.

Kilpailulle on syytä antaa sopiva otsikko sekä
tunnuskoodi, joka voi olla esimerkiksi Hiihtoliiton kilpailukalenterin
mukainen numerokoodi. Kun kilpailulajiksi valitaan *Hiihto* ja
täsmennykseksi *Normaali* asettaa ohjelma muut
valinnat 1-vaiheiselle hiihtokilpailulle sopiviksi: sjoitukset
määräävä ja tulosluettelossa käytettävä ajanottotarkkuus on 0,1 s
ja seuranimistä käytetään pitkää muotoa. Sekä sarja että rintanumero ovat
aina samat eli samat kilpailijan perustiedoissa ja käytössä
olevassa ainoassa kilpailuvaiheessa.

Kun kaavakkeelta poistutaan hyväksyen tehdyt valinnat,
tallentuu määritykset sisältyvä tiedosto KilpSrj.xml
kilpailun kansioon. Samalla syntyy vielä tyhjä tiedosto laskenta.cfg.

Kun perusominaisuudet on näin valittu, voidaan kilpailu
avata esivalmisteluun klikkaamalla näin nimettyä painikekenttää ja avaamalla ilmoitt.cfg. Tällöin ohjelma ilmoittaa, että tiedostoa KILP.DAT
ei
ole ja luo
sellaisen.

**Takaa-ajokilpailu**

Takaa-ajokilpailu tarkoittaa kilpailua, jossa osat
hiihdetään erillisinä vaiheina. Suksenvaihdon sisältävä yhtenäinen kilpailu
käsitellään yksivaiheisena kilpailuna, jossa
suksenvaihtoon liittyy yksi
väliaika.

Takaa-ajokilpailussa on vaiheiden lukumäärä
normaalisti 2 (joskus
enemmän).

Takaa-ajokilpailussa on yleensä valittava
rintanumero vaihtuvaksi vaiheesta
toiseen.

Takaa-ajokilpailussa voi olla perusteltua käyttää
jälkimmäisessä vaiheessa suurempaa aikojen sijat määräävää tarkkuutta.
Takaa-ajovaheessa ei myöskään käytetä lähtöporttia. Näiltä osin poikkeavat
valinnat
voidaan tehdä vaihekohtaisissa
määrityksissä.

**Sprinthiihto**

Sprinthiihdossa vaiheiden lukumäärä voi olla 2, 3 tai 4.
Kaikkia aikoja käsitellään tyypillisesti 0,01 s tarkkuudella. Tallennustarkkuus
ja erävaiheiden sijat määräävä tarkkuus voivat kuitenkin olla myös 0,001 s,
jolloin sadasosalleen saman
ajan saaneillekin
voidaan saada eri
sijat.

Sprinthiihdossa on yleensä valittava sekä sarja että
rintanumero vaihtuvaksi vaiheesta
toiseen.

Sprinthiihdossa ei erien lähtöaika ole ole yleensä
täsmälleen ennalta määrätty. Tästä syystä on valittava,
että lähtöportti on
käytössä.

---

 Copyright 2012, 2015 Pekka
Pirilä