# Esimerkkiaineisto

Asennusohjelma luo kansiot *HkKisaWinData* ja
*HkMaaliData* (sivu [Kansiorakenne](kansiorakenne.md)), jotka
sisältävät tutustumiseen tarkoitettua aineistoa. Aineisto vastaa pientä
suunnistuskilpailua siten, että

- *HkMaaliData* sisältää muuten ohjelmalla
  *HkMaali* totetutettavaa leimatarkastusta vastaavat tiedot ja
  määritykset paitsi, että yhteyttä lukijaleimasimeen ei ole määritelty.
  Tarkastelu voidaan käynnistää klikkaamalla tähän kansioon asennettua
  pikakuvaketta ohjelmaan *HkMaali.*

  - *HkKisaWinData* sisältää saman kilpailun
    aineiston tarkasteltavaksi ohjelmalla *HkKisaWin*. Tarkastelua varten
    on ohjelma *HkKisaWin* käynnistettävä omasta pikakuvakkeestaan, jonka
    asennusohjelma sijoittaa sekä työpöydälle että käynnistysmenuun. Tämän jälkeen
    on ohjelman painikekentästä *Avaa kilpailu tulospalveluun* haettava
    kansio *HkKisaWinData* ja avattava tiedosto
    *Laskenta.cfg.*

    - Kun molemmat ohjelmat on avattu, avautuu niiden
      välille myös tiedonsiirtoyhteys.

Syntyvä tilanne vastaa läheisesti sitä, mikä syntyy, kun
yhdellä tietokoneella suoritetaan leimantarkastusta ja toisella tehdään joitain
muita tehtäviä. Tässä esimerkkitilanteessa molemmat toiminnot on vain toteutettu samalla tietokoneella.

Kummassakin kansiossa on tiedostot

- KILP.DAT, joka sisältää kaikki osanottajatiedot ja tulokset

  - KilpSrj.xml, joka sisältää kilpailun ja sarjojen määritykset

    - EMIT.DAT, joka sisältää Emit-korteilta luetut tiedot

      - LEIMAT.LST, joka sisältää luettelon rasteilla
        olevien leimasimien koodeista

        - RADAT.LST, joka sisältää ratojen kuvaukset
          (rastiluettelot ja joitain muita tietoja)

          - seurat.csv, joka sisältää luettelon suunnistuksen
            seuroista ja seuralyhenteistä

            - COMFILE.DAT, joka syntyy aina, kun tiedonsiirto
              otetaan käyttöön ja jota käytetään tiedosiirron varmistamiseen

              - Laskenta.cfg on konfigurontitiedosto, jossa kuvataan
                kyseistä kansiota käyttävän ohjelman tehtäviä ja asetuksia

Kaksi viimeisintä tiedostoa ovat kansioissa poikkeavat,
muut ovat sisällöltään identtiset.

Lisäksi on kansiossa *HkMaaliData* pikakuvake
ohjelmaan *HkMaali* ja kansiossa *HkKisaWinData* esimerkinomainen
henkilöluettelo sekä esivalmisteluvaihetta varten tarkoitettu tyhjä
konfiguraatiotiedosto *ilmoitt.cfg* .

Kansion *HkMaaliData* tiedosto
*Laskenta.cfg* sisältää rivit

```
Kone=MA
Emit
yhteys1=udp:0/localhost:y2
lähemit1
```

Ensimmäinen rivi kertoo koneen kaksikirjaimisen tunnuksen,
joka näkyy tiedonsiirron vastapuolen koneella. Toinen rivi kertoo, että on
varauduttu käsittelemään Emit-leimantarkastuksen tietoja. Kolmas rivi kertoo,
että yhteys no 1 otetaan udp-protokollaan käyttäen samalla koneella toimivan
toisen ohjelmainstanssin yhteyteen no 2. Merkki '0' ennen kauttaviivaa kertoo,
että yhteys käyttää yhteyden 1 oletusporttia 15901. Neljäs rivi kertoo, että
myös leimaustiedot siirretään yhteyden 1 kautta.

Kansion *HkKisaWinData* tiedosto
*Laskenta.cfg* sisältää rivit

```
Kone=WI
Emit
yhteys2=udp
lähemit2
```

Sisältö on muuten sama, mutta yhteydestä ei sen toisessa päässä tarvitse kertoa muuta kuin sen numero, koska ohjelma voi päätellä muut tiedot toisen ohjelman ottaessa
siihen
yhteyttä.