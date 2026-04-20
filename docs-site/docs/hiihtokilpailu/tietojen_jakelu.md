# Tietojen jakelu

Kilpailun esivalmistelu tapahtuu tyypillisesti yhdellä
tietokoneella. Kilpailun tarvitsemia tietoja sisältävät tiedostot

- KilpSrj.xml syntyy kilpailua luotaessa. Se sisältää
  kilpailun yleismääritykset sekä sarjoja koskevat tiedot.

  - KILP.DAT sisältää kaikki osanottajia koskevat tiedot

Näiden tiedostojen kuuluu olla identtiset kaikilla
tulospalveluun osallistuvilla tietokoneilla ja ne tarvitaan kahdessa eri
kansiossa, jos samalla tietokoneella on käynnissä kaksi tulospalveluun liittyvää
ohjelmaa tai sama ohjelma kahtena kopiona.

Lisäksi tarvitaan edellisen luvun mukaiset konfiguraatiotiedostot,
jotka määrittelevät eri koneiden tehtävät sekä keskinäiset
yhteydet. Konfiguraatiotiedostot ovat eri tietokoneilla erilaiset.

Ohjelmia käytettäessä syntyy muitakin tiedostoja

- COMFILE.DAT syntyy varmistamaan
  tiedonsiirtoa

  - ajanottotiedostoja, kuten AJAT1.LST ja LAJAT.LST syntyy, jos
    käytetään ajanottotoimintoja

    - lokitiedostoja syntyy, jos sellaisia on pyydetty.

Kilpailun alkaessa on jokaisessa käytössä olevassa
kansiossa oltava identtiset kopiot tiedostoista KilpSrj.xml ja KILP.DAT
. Tiedosto COMFILE.DAT on poistettava, kun tiedosto
KILP.DAT jaellaan eri tietokoneille, koska COMFILE.DAT
saattaa tällöin sisältää lähettämättä jääneitä sanomia, jotka aiheuttavat virhetilanteita ohjelmien käytössä.

Hyvä menettely on tehdä komentotiedostot, jotka poistavat ensin potentiaalisesti ongelmalliset tiedostot ja kopioivat sitten
uudet tiedostot KilpSrj.xml ja KILP.DAT käytössä oleviin
kansioihin verkon kautta. Tämä edellyttää, että tiedonsiirron käyttämät koneiden väliset yhteydet on sallittu määrittelemällä kyseiset hakemistopolut
jaetuiksi verkossa. Komentotiedoston sisältö voi olla esimerkiksi

```
del \\192.168.1.13\Kisat\kisa1\*.dat
del \\192.168.1.13\Kisat\kisa1\*.lst
copy c:\kisat\kisa1\KilpSrj.xml \\192.168.1.13\Kisat\kisa1\
copy c:\kisat\kisa1\KILP.DAT \\192.168.1.13\Kisat\kisa1\
del \\192.168.1.12\Kisat\kisa1\*.dat
copy c:\kisat\kisa1\KilpSrj.xml \\192.168.1.12\Kisat\kisa1\
copy c:\kisat\kisa1\KILP.DAT \\192.168.1.12\Kisat\kisa1\
```

Tässä on oletettu, että leimantarkastuksen koneilla
192.168.1.13 (maalin kone) sekä 192.168.1.12 (kuuluttajan kone) on verkkoon jaettu kansio
C:\kisat nimellä Kisat sallien kirjoittaminen kansioon ja sen alikansioihin
ja että kilpailun tiedot ovat tämän kansion alakansiossa kisa1 sekä tiedot
jakelevalla että ne vastaanottavalla koneella. Samoin on oletettu, ettei yksikään \*.dat -päätteinen tiedosto olisi mahdollisesti säilytettävä.

Edellä luetellut
komennot soveltuvat käytettäviksi toimiston koneelta. Vaihtoehtoisesti voidaan vastaavat
komennot toteuttaa niin että vain toimiston koneella jaetaan kansio Kisat ja muilla koneilla toteutetaan
tietojen haku vastaavalla komennolla samalla, kun koneita käynnistetään kilpailua varten.

Erikseen on huolehdittava, että tarvittavat
konfiguraatiotiedostot ovat käytettävissä sekä siitä, että ohjelmasta HkKisaWin
on tarpeen mukaan tallennettu avausmääritykset sekä avattavat ikkunat ennen kilpailun alkua.