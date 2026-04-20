# Konfiguraatiotiedostot

### Konfiguraatiotiedostot

Tulospalveluun liittyvät perustoiminnot kisan aikana
ovat ajanotto ja kuulutuksen tuki. Lisäksi voi olla tarpeen tehdä
osanottajatieoihin joitain muuoksia. Näitä tehtäviä varten käytetään kolmea
tietokonetta:

- **Ajanoton kone**.
  Tähän koneeseen on liitetty maalikello, johon on puolestaan liitetty
  lähtöportti sekä valokenno (tai ajanottopainike). Tämä kone toimii myös
  tietokoneiden tiedonsiirron keskuskoneena, joka on ytheydessä molempiin muihin
  koneisiin. Ajanoton hoitamiseen soveltuu ainakin toistaiseksi parhaiten
  ohjelma *HkMaali*, koska kilpailijoiden numeroiden
  syöttäminen otettujen aikojen rinnalle on sujuvinta tässä ohjelmassa.

  - **Kuuluttajan
    kone.** Tätä konetta käytetään kuuluttajan toimintaa tukevien tietojen
    näyttämiseen. Käytössä ohjelma *HkKisaWin* .

    - **Toimiston
      kone.** Tällä koneella tehdään mahdollisesti tarvittavat muutokset
      osanottajatietoihin sekä laaditaan tulosteet sekä kirjoittimelle että html- ja
      muihin tiedostoihin. Käytössä ohjelma *HkKisaWin*
      .

Kunkin tietokoneen tehtävät edellyttävät niitä vastaavan
konfiguraatiotiedoston laatimisen ja sijoittamisen kilpailun hakemistoon.
Konfigraatiotiedoston oletusnimi on Laskenta.cfg, mitä
voidaan käyttää kaikilla koneilla, mutta toinen ja ehkä parempi vaihtoehto on
antaa tiedostoille koneen tehtävää vastaavat nimet maali.cfg, kuul.cfg ja toimisto.cfg.

Ajanoton koneen konfiguraatiotiedoston sisältö on
seuraavankaltainen (//-alkuiset rivit ovat seuraavaa parametria selittäviä kommentteja)

```
//Koneen tunnus
kone=MA
//käynnistä ohjelma kysymättä valintojen hyväksymistä
boot
//Määrittele näppäimistöltä ajanottoon käytettävälle näppäimelle epäkelpo koodi. 
//(Ohjelma ei tällöin kysy näppäintä eikä aikaa voi ottaa näppäimistöltä)
näppäin=99,99
//Maalikellon liitäntä (oletuksena kello Timy liitettynä sarjaporttiin COM5)
timy=5
//Kellon lähtöportilta saamat ajat tulkitaan ohjelmassa lähtöajoiksi ja valokennon ajat maaliajoiksi.
//Kaikille sarjoille käytetään lähtijän päättelyä otetun ajan perusteella
pisteet=mmmlm/kaikki
//Kaikki ajat, jotka eivät tule lähtöportilta ovat loppuaikoja (eivät siis väliaikoja)
pisteet=m/vain
//ajanottotiedot säilytetään uudelleenkäynnistyksessä
ajat=/s
//lähtöportin ajat tallennetaan eri tiedostoon
lajat
//yhteys toimiston koneeseen (koneen ip-numero 192.168.1.11)
yhteys1=udp:0/192.168.1.11
//mahdollisesti käyttöön otettava ajanottotaulukon siirto toimiston koneelle (ei välttämätön)
lähaika1
//yhteys kuuluttajan koneeseen (koneen ip-numero 192.168.1.12)
yhteys2=udp:0/192.168.1.12
```

Toimiston koneen konfiguraatiotiedoston sisältö on seuraavankaltainen

```
//Koneen tunnus
kone=TO
//ajanottotiedot säilytetään uudelleenkäynnistyksessä
ajat=/s
//lähtöportin ajat tallennetaan eri tiedostoon
lajat
//yhteys maalin koneeseen
yhteys1=udp
//mahdollisesti käyttöön otettava ajanottotaulukon siirto maalin koneelta (ei välttämätön)
lähaika1
```

Kuuluttajan koneen konfiguraatiotiedoston sisältö on seuraavankaltainen

```
//Koneen tunnus
kone=KU
//yhteys maalin koneeseen yksisuuntaisena (kuuluttajan koneelta ei voi lähettää tietoja muille)
yhteys1=udpi
//Seuraava parametri pyytää ohjelmaa avaamaan tallennetun kaavakeasettelun käynnistyksen yhteydessä
ikkunat
```

Maalin koneella on luotava ohjelmaan HkMaali viittaava
pikakuvake niin, että se viittaa kipailutietojen hakemistoon. Ohjelmaa HkKisaWin
käytettäessä ei vastaavaa menettelyä tarvita, vaan työhakemisto valitaan
ohjelmasta käsin.

Ohjelman HkKisaWin avausmääritykset ovat erityisen
hyödylliset kuuluttajan koneella, jolla kannattaa menetellä seuraavasti (Tämä on
mahdollista vain, kun käytettävissä on tiedostot KilpSrj.xml ja KILP.DAT ja
ohjelma on käynnistetty tulospalvelutilaan).

- luodaan sellainen ikkunoiden asettelu, jota
  kuuluttaja tulee kilpailussa käyttämään
- tallennetaan asettelu tiedostoon oletus.ikk ikkunaohjelman valinnassa *Tiedostot /
  Tallenna ikkunat*
- Valinnassa *Tiedostot / Avausmääritykset*
  valitaan avaaminen suoraan tulospalveluun sekä konfiguraatiotiedostoksi yllä
  kuvattu tiedosto.