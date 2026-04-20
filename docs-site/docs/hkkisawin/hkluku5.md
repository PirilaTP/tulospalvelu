# Luku 5. Ajanottotoiminnot

## 5 Ajanottotoiminnot

Ohjelma HkKisaWin sisältää mahdollisuuden käyttää
maalikelloa tai ajanottoon kykeneviä tunnistinjärjestelmiä,
kuten Emit-leimasimia, emiTag-järjestelmää ja joitain
RFID-laitteita, mutta toistaiseksi suosittelen käyttämään ohjelmaa
lähinnä seurantaan ja hoitamaan sujuvuussyistä varsinaisen
ajanoton ohjelmalla HkMaali. Tässä luvussa esitettävät ohjeet koskevat
suurelta osin myös ohjelmaa HkMaali.

Ohjelman HkKisaWin ajanottotoiminnoissa voidaan helposti liittää aikoja kilpailijoihin, mutta
ohjelmaan HkMaali sisältyy runsaasti kirjaukseen liittyviä erityistoimintoja, joita ei ole ohjelmassa HkKisaWin.

Ajanottotoiminoja säädellään monilla
käynnistysparametreilla, jotka on lueteltu [liitteessä
1](hkliite1.md). Tässä
luvussa kuvataan tärkeimpien parametrien käyttöä.

Algen Timy-maalikelloa koskevia lisäohjeita on [liitteessä 7](liite_7._algen_timy_usb_portissa.md) ja
EmiTag-järjestelmän käytöstä [liitteessä 9](liite_9._emitag-laitteiden_kaytto.md) .

Ohjelman ajanottotoiminnot käyttävät erillista
tiedostoa, jonka nimi on tyypillisesti AJAT1.LST missä
1 viittaa ensimmäiseen kilpailvaiheeseen. Myös viimeinen merkki voi vaihdella,
kun aikoja otetaan useampaan jonoon.

Kaikille ajanottotavoille yhteisiä parametreja ovat
mm.

```
      AJAT=/S  
LÄHAIKA1
```

joista ensimmäinen pyytää säilyttämään ajanottotiedot
ilman varmistuskysymystä ja jälkimmäinen lähettämään ajanottotiedot yhteyteen 1.
Ajanottotiedot ei tarkoita kilpailijan saamaa tulosta, joka siirretään aina, kun
yhteys on avoinna lähetykseen, vaan erillistä taulukkoa, jota käytetään
esimerkiksi maalikellolta tulleiden aikojen seurantaan ja
käsittelyyn.

---

 Copyright 2012, 2015 Pekka
Pirilä