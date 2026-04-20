# Luku 14. Ampumahiihto

## 14 Ampumahiihto

### 14.1  Sakkopaikkojen määrittely

Ampumahiihdon tulospalvelu eroaa hiihdosta sakkojen
käsittelyn osalta. Myös takaa-ajolähdön käsittelyssä on
pieniä eroja.

Kilpailun [perustiedoissa](2.1_uuden_kilpailun_luominen.md) valitaan lajiksi ampumahiihto
ja on hyvä määritellä myös ohilaukauksesta annettavan sakon oletussuuruus,
vaikka tämä arvo ei määrääkään sakkoa lopullisesti.

Ampumapaikkojen määrä sekä ohilaukauksesta annettava
sakko määritellään [sarjatiedoissa](2.2.1_sarjatietojen_muuttaminen.md)
kullekin sarjalle erikseen. Tämä tapahtuu
parhaiten yhden sarjan tietojen muokkauskaavakkeella, jossa on kunkin vaiheen
tiedoissa kentät näiden ilmoittamiseen.

[Takaa-ajolähtö](10.3_takaa-ajolahto.md)
 vaatii omat määrittelynsä sarjatiedoissa
ja aikojen käsittelytarkkuuden valinnoissa.

Kun ampumahiihdossa otetaan väliaikoja, on ohjelman
oletusvalintana, että 1. väliaikapiste sisältää 1. ampumapaikan, 2.
väliaikapiste kaksi ampumapaikkaa jne. Tätä oletustoimintaa voidaan muuttaa
ilmoittamalla väliaikapisteiden määrittelyssä kuhunkin
väliaikaan sisältyvien ampumapaikkojen lukumäärä

### 14.2  Sakkojen syöttö

Sakkojen syöttö voidaan tehdä sekä ohjelman HkMaali
korjausikkunassa että ohjelmassa HkKisaWin, jossa se sujuu parhaiten tähän
tarkoitukseen määritellyssä kaavakkeessa. Tämä kaavake voidaan avata
pääkaavakkeen valinnan *Tulospalvelu* kautta.

Sakkojen syöttökaavakkeella haetaan kilpailija antamalla
numero ja painamalla *Enter*, jolloin ohjelma siirtyy ensimmäisen
käsittelemättömän ampumapaikan kohdalle. Ohjelma käyttää tavuviivaan sen
merkkinä, että sakkojen määrää ei ole syötetty ja myös virheettömät suoritukset
on tallennettava antamalla arvo 0. Näppäin *Enter* vie sakkokentästä
syötön hyväksymiseen ja hyväksyminen uuden kilpailijanumeron syöttöön. Jos
kerralla halutaan syöttää useampia sakkoja, on kentästä toiseen siirtymiseen
käytettävä tabulaattoria (ja takaisinpäin yhdistelmää *Shift-Tab*).
Kun tieto tallennetaan, kirjautuu se myös syötettyjen tietojen luetteloon, joka
näkyy kaavakkeen yläosassa.

Ohjelma pystyy myös ottamaan sakot vastaan Kurvisen
laitteistosta. Tämä toimintamalli käynnistetään antamalla ohjelmalle
käynnistysparametri

```
SAKKO_YHT=n
```

sekä

```
SAKKO_LAJI=x
```

jos käytössä on Kurvisen laitteiston
vanhempi versio kuin 2. Vanhimmalle versioille x=0 ja seuraavalle
x=1. Oletuksena on uudempien laitteiden x=2, jota ei tarvitse kertoa
parametrilla. Kurvisen laiteversiot 1 ja 2 eroavat toisistaan vain
tiedonsiirtonopeuden osalta. Se on 9600 baud versiossa 1 ja 19200 baud versiossa
2. Ohjelma valitsee nopeuden yllä mainitun parametrin
perusteella.

Kun sakkotietoja tulee Kurvisen laitteistolta ja
sakkotietojen syöttöikkuna on avattuna kirjaa ohjelma saapuneet tiedot
syötettyjen tietojen luetteloon. Tiedot kirjautuvat kilpailijatietoihin vaikka
tämä ikkuna ei olisi avattuna ja toiminta on tällöin samanlainen ohjelmissa
HkKisaWin ja HkMaali. Ohjelma antaa virheilmoituksen, kun saapuvassa sanomassa
todetaan virhe.

Ohjelmat siirtävät ammunnan tuloksia koskevat Kurvisen laitteen
tiedot verkossa, joten sekä tämä toiminto, että kohdan 14.4 toiminto ovat
käytettävissä myös muilla koneilla kuin sillä, johon Kurvisen laitteet
on liitetty.

### 14.3  Sakkokierrosten seuranta

Ohjelman valinnassa *Sakkokierrosten seuranta*
voidaan syöttää kilpailijan numero aina sakkokierroksen täyttyessä. Tällöin
ohjelma näyttää toisessa ikkunassa niiden sakkokierrokset, joita ei ole kirjattu
suoritetuiksi.

Ohjelma
tallentaa tiedostoon SakkoKierr.txt syötetyt sakkokierrokset ja lukee ne,
kun sakkokierrosten seurantakaavake avataan. Ennen uutta kilpailua on
varmistettava, että tätä tiedostoa ei ole tai tiedosto on tyhjä. Virheelliesti
syötetty tieto voidaan poistaa siirtymällä sitä koskevalle riville ja
klikkaamalla *Peruuta valittu kirjaus*
.

Tiedosto SakkoKierr.txt on
käytettävissä vain konella, jolla syöttö on tehty. Tieto kirjattujen kierrosten
määrästä siirtyy muillekin tietkoneille, joten kirjaamattomien kierrosten
määriä voi katsella myös
muilta
koneilta.

### 14.4 Ammunnan laukaustasoinen seuranta

Ohjelman valinnassa *Seuranta / Ampumapaikat*
avautuu kaavake, jolla ohjelma näyttää jokaisen laukauksen tuloksen, jos
käytössä on Kurvisen laitteet, jotka tulevat laukausten seurantaa (versio 0 ei
tue).

---

 Copyright 2012, 2015 Pekka
Pirilä