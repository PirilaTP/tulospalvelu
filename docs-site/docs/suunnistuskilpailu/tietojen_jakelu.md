# Tietojen jakelu

Kilpailun esivalmistelu tapahtuu tyypillisesti yhdellä
tietokoneella. Kilpailun tarvitsemia tietoja sisältävät tiedostot

- KilpSrj.xml syntyy kilpailua luotaessa. Se sisältää
  kilpailun yleismääritykset sekä sarjoja koskevat tiedot.

  - KILP.DAT sisältää kaikki osanottajia koskevat tiedot

    - radat1.xml sisältää ratojen
      kuvaukset ja rastien leimasinkoodit (nimen numero 1 viittaa ensimmäiseen tai
      ainoaan vaiheeseen, toisen vaiheen tiedosto on rada2.xml )
    - väistyvänä vaihtoehtona RADAT.LST on aiemmin
      käytetty tiedostomuoto, joka sisältää ratojen kuvaukset

      - väistyvänä vaihtoehtona LEIMAT.LST on aiemmin
        käytetty tiedostomuoto, joka sisältää luettelon eri rasteilla olevien
        leimasinten koodeista- EMIT.DAT sisältää tarpeellista tietoa vasta, kun
      emit-kortteja luetaan leimantarkastuksessa

Näiden tiedostojen kuuluu olla identtiset kaikilla
tulospalveluun osallistuvilla tietokoneilla ja ne tarvitaan kahdessa eri
kansiossa, jos samalla tietokoneella on käynnissä kaksi tulospalveluun liittyvää
ohjelmaa tai sama ohjelma kahtena kopiona.

Lisäksi tarvitaan konfiguraatiotiedostoja, jotka
määrittelevät eri koneiden tehtäviä sekä keskinäsisiä yhteyksiä.
Konfiguraatiotiedostot ovat yleensä eri tietokoneilla erilaiset.

Ohjelmia käytettäessä syntyy muitakin tiedostoja

- COMFILE.DAT syntyy varmistamaan
  tiedonsiirtoa (monivaiheisessa kilpailussa COMFILE1.DAT, COMFILE2.DAT
  jane.)

  - ajanottotiedostoja, kuten AJAT.LST syntyy, jos käytetään ajanottotoimintoja
    (myös LAJAT.LST, AJAT1.LST, AJAT.LS2
    yms.)

    - lokitiedostoja syntyy, jos sellaisia on pyydetty.

Kilpailun alkaessa on jokaisessa käytössä olevassa
kansiossa oltava identtiset kopiot tiedostoista KilpSrj.xml ja KILP.DAT. Samoin on ratatiedoston (radat1
.xml tai vast.) oltava kaikissa koneissa, jotka käsittelevät
tavalla tai toisella leimaustietoja. Tyhjä tiedosto EMIT.DAT saa olla mukana, mutta se syntyy aina tarvittaessa.
Tiedosto COMFILE.DAT on poistettava, kun tiedosto
KILP.DAT jaellaan eri tietokoneille, koska COMFILE.DAT
saattaa tällöin sisältää lähettämättä jääneitä sanomia, jotka aiheuttavat
virhetilanteita ohjelmien käytössä.

Hyvä menettely on tehdä komentotiedostot, jotka
poistavat ensin potentiaalisesti ongelmalliset tiedostot ja kopioivat sitten
uudet tiedostot KilpSrj.xml, KILP.DAT ja radat1.xml
käytössä oleviin kansioihin verkon kautta. Tämä edellyttää, että tiedonsiirron käyttämät
koneiden väliset yhteydet on sallittu määrittelemällä kyseiset hakemistopolut
jaetuiksi verkossa. Komentotiedoston sisältö voi olla esimerkiksi

```
del \\192.168.1.11\Kisat\kisa1\*.dat
del \\192.168.1.11\Kisat\kisa1\*.lst
copy c:\kisat\kisa1\KilpSrj.xml \\192.168.1.11\Kisat\kisa1\
copy c:\kisat\kisa1\KILP.DAT \\192.168.1.11\Kisat\kisa1\
copy c:\kisat\kisa1\radat1.xml \\192.168.1.11\Kisat\kisa1\
```

Tässä on oletettu, että leimantarkastuksen koneella 192.168.1.11 on jaettu kansio C:\kisat
nimellä Kisat sallien kirjoittaminen kansioon ja sen alikansioihin
ja että kilpailun tiedot ovat tämän kansion alakansiossakisa1 sekä tiedot jakelevalla että
ne vastaanottavalla koneella. Samoin on oletettu, että yksikään \*.dat tai \*.lst -päätteinen tiedosto olisi mahdollisesti säilytettävä.

Edellä luetellut komennot soveltuvat käytettäviksi palvelimelta käsin, jolloin samaan tiedostoon voidaan sisällyttää vastaavat
rivit kaikkien muiden kisakansioiden osalta. Vaihtoehtoisesti voidaan vastaavat komennot toteuttaa niin että vain palvelimella
jaetaan kansio Kisat ja muilla koneilla toteutetaan tietojen haku tällaisella
komennolla samalla, kun koneita käynnistetään kilpailua varten.

Erikseen on huolehdittava, että tarvittavat konfiguraatiotiedostot ovat käytettävissä sekä siitä, että ohjelmasta HkKisaWin
on tarpeen mukaan tallennettu avausmääritykset sekä avattavat ikkunat ennen kilpailun alkua.

---

 Copyright 2012 Pekka
Pirilä