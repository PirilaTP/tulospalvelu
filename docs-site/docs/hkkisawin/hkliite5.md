# Liite 5. Kuntosuunnistusmalli 3

## Liite 5. Kuntosuunnistusmalli 3

### A5.1 Toimintamoodin peruskuvaus

Kuntosuunnistuksen toimintamoodi 3 otetaan käyttöön antamalla
ohjelmalle käynnistysparametri

`KUNTOSUUNNISTUS=3`

Jos käytössä on myös esiluenta, on esiluennan ohjelmalle annettava parametri

`ESILUENTA=KUNTO`

Kuntosuunnistusmoodiin päästään sitten avaamalla ohjelma tulospalvelutilaan ja valitsemalla
päävalikosta *Tulospalvelu/Emit-luenta* tai *Osanottajat / Ilmoittautumiset* riippuen
kyseisessä pisteessä hoidettavasta tehtävästä. Vaihtoehtoisesti voidaan määritellä ohjelma aukeamaan
suoraan tulospalvelutilaan ja avaamaan käynnistyessään kaavake Emit-luenta.

Toimintamoodissa 3 muodostuu kokonaisuus seuraavista tehtävistä:

- Tiedostojen esivalmistelu- Osanottajatietojen syöttö kisakansliassa- Emitkoodien antaminen osanottajille ja osanottajien
      merkintä lähtijöiksi.- Leimantarkastus, jonka yhteydessä saadaan myös tulokset.

### A5.2 Esivalmistelut toimintamoodiin 3

Esivalmistelu sisältää seuraavat vaiheet:

- Kilpailun määrittely
  - Käytettävä tiedostokansio valitaan ja luodaan joko ohjelman ulkopuolella tai toiminnossa
    *Kilpailun luominen ja perusominaisuudet*- Luodaan ja nimetään kilpailun sarjat joko em.
      toiminnossa tai kopioimalla aiempi soveltuva tiedosto KilpSrj.xml
      kilpailun tiedostokansioon ennen kilpailun perustietojen määrittelyä. Sarjoihin on
      sisällytettävä ylimääräinen vakanttisarja, johon sijoittaan esivalmistelussa kaikki
      tiedot. Ohjelma pyrkii luomaan tarkoitukseen sopivan
      vakanttisarjan automaattisesti.- Perusominaisuuksissa on syytä määritellä otsikkotiedot ja antaa kilpailulle tunnuskoodi, joka
        eroaa muista kilpailuista.  
  - Ratojen määrittely ohjelman valinnassa *Valmistelu / Ratatiedot*. Jokaiselle sarjalle on
    määriteltävä rata, jonka nimi on sama kuin sarjan nimi. Nollausennakoksi
    määritellään *Nollaus lähtöhetkellä* ja maalin osalta on valittava
    *Maaliviivalla leimasin (ei online*). Myös vakanttisarjalle on radan
    määrittely suositeltavaa häiritsevien virheilmoitusten eliminoimiseksi.
    Vakanttisarjan rataan sisällytetään ainakin yksi rasti, jota ei ole metsässä ja jolle annetaan
    leimasinkoodi, jota ei myöskään ole käytössä, jotta kukaan ei voisi saada hyväksyttyä
    suoritusta vakanttisarjaan.
      
      
    - Konfiguraatiotiedoston laatiminen. Yksinkertaisinta on käyttää tiedoston nimenä oletusnimeä
      `Laskenta.cfg`. Tiedostoon on sisällytettävä ainakin seuraavat rivit:  
        
      - Kanslian koneella
          
         `KUNTOSUUNNISTUS=3`
          
          
        - Esiluennan koneella
            
           `ESILUENTA=KUNTO
            
          LUKIJA=n`
            
            
          - Leimantarkastuksen koneella
              
             `KUNTOSUUNNISTUS=3
              
            LUKIJA=n
              
            EMITALKU=aaa`  
      missä parametrin `LUKIJA=n` numeroarvon on oltava käytettävän sarjaportin numeron, siis `LUKIJA=1`,
      jos käytössä on `COM1`. Parametri EMITALKU
      tarvitaan, kun leimantarkastuspisteitä on useita. Jokaisessa pisteessä on
      käytettävä eri alkuarvoa aaa. Käytettävät arvot kannattaa valita väliltä 1000
      - 30000, jotta vältettäisiin viallisten korttien arvo 200 ja korteilla muuten
      käytettävät numeroalueet.
        
        
      Lisäksi on annettava tiedonsiirron edellyttämät parametrit.
        
        
      - Luodaan tietueet jokaiselle osanottajalle noudattaen käytössä olevia mahdollisten rintanumeroiden mukaisia
        numeroalueita ohjelman ollessa esivalmistelumoodissa valinnassa *Osanottajat / Lisää vakantteja*.
        Tietueet merkitään tällöin Ei-lähteneiksi. Lisäksi luodaan ylimääräisiä vakantteja, joita tarvitaan pieni
        määrä perustoiminnassa ja jotka toimivat lisäksi varapaikkoina, jos sellaisia joskus tarvitaan.
        Nämä vakantit luodaan käytössä olevan numeroalueen ulkopuolelle ja merkitään vakanteiksi.
        Lähtöaika kannattaa jättää avoimeksi kaikilta.
          
          
        - Valmistellaan mahdollisesti kisakanslian käyttöön henkilötietokanta aiempien osanottajien perusteella.
          Tiedoston nimeksi annetaan `henkilot.csv`.
          Tiedoston ensimmäinen rivi on
            
            
          `KilpId;Sukunimi;Etunimi;Seura;Maa;Joukkue;Sarja;Badge`  
            
          ja kullakin muulla rivillä tuon otsikkorivin mukaisesti puolipisteellä erotettuina ainakin seuraavia tietoja
          (puuttuvat tiedot jätetään pois, mutta puolipisteitä on oltava oikea määrä):
            
          - Kilpailijan tunnuskoodi, jota käytetään henkilöiden
            tunnistamiseen ja joka tallentuu lisenssitietokenttään. Välttämätön
            tiedoston jatkuvan ylläpidon kannalta.- Sukunimi, korkeintaan 24 merkkiä- Etunimi, korkeintaan 24 merkkiä- Seura tai yhteisö, korkeintaan 31 merkkiä- Maa. Ei yleensä käytössä, 3 merkkiä- Joukkue. Ei yleensä käytössä, korkeintaan 15
                      merkkiä- Sarja. Ei välttämätön. Käytetään oletuksena, jos
                        sama kuin sarjatiedoissa. Helpottaa hieman työskentelyä.- Emit-kortin koodi. Olennaisena apuna, kun korttia käyttää yksi tai korkeintaan muutama henkilö.
                          Ei pidä tallentaa, ellei henkilö käytä samaa korttia toistuvasti.  
          - Valmistellaan seuratietokanta, jos sellaista halutaan käyttää. Tietokannassa on kullakin rivillä seuraavat
            tiedot tabulaattorin, puolipisteen tai välilyöntien erottamina:
            - Piirikoodi. Numeroarvo, joksi merkitään 0, ellei
              muuta arvoa ole.- Nimilyhenne. Korkeintaan 15-merkkinen lyhenne, joka
                ei sisällä välilyöntejä tai puolipistettä. Ei yleensä käytössä, mutta jotain
                tekstiä on oltava.- Seuran nimi. Korkeintaan 31-merkkiä.  
            - Tiedostot
              `KilpSrj.xml ja KILP.DAT` kopioidaan jokaiselle tietokoneelle kilpailun
              työhakemistoon, jonne sijoitetaan myös konekohtaisesti sovitettu konfiguraatiotiedosto.
              Leimatarkastuksen koneelle kopioidaan myös tiedosto  `radat1.xml`.
              Tiedostot `henkilot.csv ja Seurat.csv` sijoitetaan
              kisakanslian koneelle, jos ne on laadittu.
                
                
              - Haluttaessa, että ohjelma käynnistyy automaattisesti käyttötilaan
                - käynnistetään ohjelma haluttuun tilaan- tallennetaan ikkunat valinnassa *Tiedostot / Tallenna ikkunat*- valinnassa *Tiedostot / Avausmääritykset*
                      - Valitaan konfiguraatiotiedosto- Sisällytetään siihen rivi `ikkunat`, johon lisätään tiedoston nimi,
                          jos ikkunat tallennettiin muuhun tiedostoon kuin `ikkunat.xml`. Mahdolliset muutokset on
                          tallennettava.- Valitaan *Avaa suoraan tulospalveluun* ja tallennetaan valinnat.- Kun halutaan lopettaa automaattinen käynnistyminen, valitaan tallennettavaksi
                              *Avaa päävalikkoon*

### A5.3 Toiminta tapahtuman aikana

### A5.3.1 Kisakanslia

Ohjelma käynnistetään. Siirrytään tulospalvelutilaan ja valitaan käytössä oleva
hakemisto sekä konfiguraatiotiedosto. Edelleen valitaan valikosta
*Osanottajat / Ilmoittautumiset*.

Kilpailijoiden tiedot syötetään valitsemalla käsiteltävä tietue kilpailijanumerolla ja
kirjaamalla tietoihin ainakin nimi sekä haluttaessa seura. Myös sarja voidaan merkitä, mutta
se ei ole välttämätöntä. Kilpailija jätetään *Ei-lähteneeksi*. Jos on mahdollista käyttää
henkilötietokanta sekä ehkä myös seuraluetteloa, nopeuttaa tämä työskentelyä olennaisesti.

### A5.3.2 Esiluenta

Ohjelma käynnistetään. Siirrytään tulospalvelutilaan ja valitaan käytössä oleva
hakemisto sekä konfiguraatiotiedosto, ellei näitä toimintoja ole automatisoitu edellä
kuvatulla tavalla. Edelleen valitaan valikosta *Tulospalvelu / Emit-luenta*,
ellei tämäkin tapahdu automaattisesti. Osanottajan saapuessa paikalle, luetaan kortti
ja syötetään kilpailijan numero, mikä voi tapahtua myös lukemalla viivakoodi. Kun numerokentässä
painetaan *Enter*, merkitään kilpailija avoimeksi ja lähtöaikakenttään kirjautuu tapahtuman
kellonaika. Lähtöajan merkitys on vain tietona metsässä olon seurantaa varten.

Kirjaus voidaan peruuttaa, jos se on jostain syystä tarpeen.

Jos kortti on jo käytössä, huomauttaa ohjelma asiasta ja antaa mahdollisuuden joko
jättää kortti sidotuksi aiemmin kirjatulle osanottajalle tai vapauttaa se uuteen käyttöön.

### A5.3.3 Leimantarkastus

Kun normaalisti käsitelty kilpailija saapuu leimatarkastukseen, laskee ohjelma hänelle ajan
ja näyttää leimat. Jos leimat vastaavat jotain rataa, määrittää ohjelma sarjan radan perusteella.

Jos kilpailijan korttia ei ole luettu esiluennassa, kirjautuu tieto vakanttipaikalle, jolta se on
siirrettävä ao. osanottajalle käyttäen painiketta *Vaihda kilpailija tai muuta leimauksen tietoja*,
syöttäen sitten numero tai hakien nimellä ja vahvistaen kirjaus lopuksi painikkeilla *Siirrä leimaukset
valitulle kilpailijalle* ja *Tallenna tehty siirto*. Ensin mainitun painikkeen käytön
jälkeen näkyvät kilpailijan tiedot leimantarkastuskaavakkeella, mutta eivät vielä ole tallentuneet.

### A5.3.4 Tuloksien tarkastelu

Tuloksia voi katsella päävalikon valinnassa *Seuranta / Tilanne*.

Osanottajatietoja voi katsella ja muokata myös valinnassa *Osanottajat / Osanottajat*
jonka kautta aukeaa taulukko.

Kummankin edellä mainitun valinnan taulukossa voi klikata osanottajaa saadakseen lisätietoja.
Esille tulevat tiedot eivät ole identtiset, vaan joiltain osin toisiaan täydentäviä.

### A5.4 Toiminta tapahtuman jälkeen

### A5.4.1 Tulosluettelot

Tulosluettelot html-tiedostoon tai paperille voi laatia valinnassa *Tulosteet*, missä
on valittava tulostettavat sarjat sekä halutun tulosluettelon tyyppi.

### A5.4.2 Tietokantojen ylläpito

Henkilötietokantatiedostoa voi täydentää kanslian koneella avaamalla toiminnon, jossa se näkyy ja valitsemalla sitten
*Muokkaus / Lisää kilpailijat tietokantaan*.

Jos käytössä on MySQL-tietokanta, voi tapahtuman osanottotiedot siirtää sinne valinnassa
*Tiedostot / Kirjoita siirtotiedostoon*. Lisäohjeita luvussa 11.

---

 Copyright 2012, 2015 Pekka
Pirilä