# Konfiguraatiotiedostot

Tässä esitettävät knofiguraatiotiedostot perustuvat
seuraaviin valintoihin:

- Käytössä on edellä sivulla *Tietokoneiden
  tehtävät* kuvattu **Vaihtoehto
  2** .

  - Tietokoneiden ip-numerot ovat:

    - 192.168.1.10  Toimisto

      - 192.168.1.11  Leimantarkastus

        - 192.168.1.12  Kuulutus- Tulostukseen käytetään toimiston tietokoneen
      oletuskirjoitinta

      - Lukijaleimasin on liitetty leimantarkastuksen
        tietokoneeseen USB-muuntimella, joka on antanut portiksi COM5.

        - Itkumuuritoiminnot hoidetaan toimistossa

          - Kaikilla koneilla on kilpailun käytössä kansio C:\Kisat\Kisa1

            - Toimiston koneella on ohjelman HkKisaWin käytössä
              alikansio C:\Kisat\Kisa1\HkKisaWin ja ohjelman
              HkMaali käytössä C:\Kisat\Kisa1\HkMaali

              - Kuuluttaja ei seuraa leimantarkastuksesta saatavia
                Emit-väliaikoja

Seuraavissa konfiguraatiotiedoistoissa on merkeillä //
alkavia kommenttirivejä, jotka kertovat seuraavan rivin merkityksestä

**Toimiston koneen konfigurointi**

Kansio C:\Kisat\Kisa1\HkMaali

- kuvaus

  - käytössä ohjelma
    HkMaali

    - toimii tiedonsiirron
      solmupisteenä eli "serverinä"

      - hoitaa myös automaattisen tulostuksen

        - ohjelma käynnistetään työpöydällä olevasta
          pikakuvakkeesta, jonka käynnistyskansioksi on määritelty C:\Kisat\Kisa1\HkMaali ja
          käynnistettävän ohjelman nimen HkMaali.exe perään on kirjoitettu parametri
          CFG=SR.CFG- konfiguraatiotiedoston SR.CFG sisältö

```
// 2-merkkinen tunnus tiedonsiirtoyhteyksien tulkinnan   tueksi
kone=sr
// Avattavan konsoli-ikkunan otsikkotieto
ikkunaots=Server
// Yhteys leimantarkastuksen tietokoneeseen. Siirtää myös leimaustiedot
yhteys1=udp:0/192.168.1.11:y1
lähemit1
// Yksisuuntainen yhteys kuulutttajan tietokoneeseen. Ei siirrä leimaustietoja
yhteys2=udpo:0/192.168.1.12:y1
// Yhteys toimiston koneen ohjelmaan WinKisaHk. Siirtää myös leimaustiedot
yhteys3=udp:0/localhost::y4
lähemit3
// Ohjelma käyttää oletuskirjoitinta
lista
// Ohjelma tulostaa automaattisesti tulosluetteloita
auto=L/40/60
//  Ohjelma tulostaa automaattisesti raportin hylkäysesitykseen johtaneista leimoista
comautorap=h
```

Kansio C:\Kisat\Kisa1\HkKisaWin

- kuvaus

  - käytössä ohjelma HkKisaWin

    - toimii toimiston erilaisten tehtävien hoidossa
      ml. itkumuurin tarpeet

      - ohjelma käynnistetään työpöydällä olevasta
        pikakuvakkeesta, joka on peruskuvake ohjelman HkKisaWin käynnistämiseksi- käynnistyminen tähän kilpailuun on automatisoitu
          ohjelman HkKisaWin valinnassa *Tiedosto /
          Avausmääritykset* aiemman käytön yhteydessä (tieto tallentuu
          tiedostoon init.cfg, mutta tästä
          ei tarvitse itse huolehtia suoraan).- tällöin on konfiguraatiotiedostoksi valittu to.cfg- konfiguraatiotiedoston to.cfg sisältö

```
// 2-merkkinen tunnus tiedonsiirtoyhteyksien tulkinnan tueksi 
kone= to 
// Yhteys toimiston koneen ohjelmaan HkMaali. Siirtää myös leimaustiedot 
yhteys4= udp
lähemit4
```

**Leimantarkastuksen koneen konfigurointi**

- kuvaus

  - käytössä ohjelma
    HkKisaWin

    - hoitaa leimantarkastuksen, joka
      tuottaa myös loppuajat

      - ohjelma käynnistetään työpöydällä olevasta pikakuvakkeesta, joka on peruskuvake ohjelman HkKisaWin käynnistämiseksi- käynnistyminen tähän kilpailuun on automatisoitu ohjelman HkKisaWin valinnassa *Tiedosto / Avausmääritykset* aiemman käytön yhteydessä (tieto tallentuu tiedostoon init.cfg, mutta tästä ei tarvitse itse huolehtia suoraan).- tällöin on konfiguraatiotiedostoksi valittu em.cfg- aiemmalla käyttökerralla on tallennettu avattavien
              ikkunoiden tilanne valinnassa *Tiedosto / Tallenna ikkunat*, kun
              leimantarkastusikkuna on halutussa paikassa ruudulla (valinta tallentuu
              tiedostoon oletus.ikk)- konfiguraatiotiedoston em.cfg sisältö

```
// 2-merkkinen tunnus tiedonsiirtoyhteyksien tulkinnan tueksi
kone=em
//  Ohjelma avaa käynnistyksen yhteydessä aiemmin valitut ikkunat
ikkunat
// Yhteys toimiston koneen ohjelmaan HkMaali. Siirtää myös leimaustiedot
yhteys1=udp
lähemit1
//  Lukijaleimasin liitetty porttiin COM5 (UDP/sarjaporttimuuntimen avulla)
lukija=5
```

**Kuuluttajan koneen konfigurointi**

- kuvaus

  - käytössä ohjelma
    HkKisaWin

    - kuuluttaja
      seuraa kilpailun tilannetta viimeisten tapahtumien näytön sekä kahden
      valitun sarjan tilannetta
      näyttävän ikkunan avulla

      - ohjelma käynnistetään työpöydällä olevasta pikakuvakkeesta, joka on peruskuvake ohjelman HkKisaWin käynnistämiseksi- käynnistyminen tähän kilpailuun on automatisoitu ohjelman HkKisaWin valinnassa *Tiedosto / Avausmääritykset* aiemman käytön yhteydessä (tieto tallentuu tiedostoon init.cfg, mutta tästä ei tarvitse itse huolehtia suoraan).- tällöin on konfiguraatiotiedostoksi valittu ku.cfg- aiemmalla käyttökerralla on tallennettu avattavien
              ikkunoiden tilanne valinnassa *Tiedosto / Tallenna ikkunat*, kun
              ikkunat ovat halutuissa paikoissa ruudulla (valinta tallentuu
              tiedostoon oletus.ikk)- konfiguraatiotiedoston ku.cfg sisältö

```
// 2-merkkinen tunnus tiedonsiirtoyhteyksien tulkinnan tueksi
kone=ku 
// Ohjelma avaa käynnistyksen yhteydessä aiemminvalitut ikkunat
ikkunat  
// Yhteys toimiston koneen ohjelmaan HkMaali.
yhteys1=udp
```