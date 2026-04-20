# Liite 3. Kuntosuunnistusmalli 1

## Liite 3. Kuntosuunnistusmalli 1

### A3.1 Toimintamoodin peruskuvaus

Kuntosuunnistuksen toimintamoodi 1 otetaan käyttöön antamalla
ohjelmalle käynnistysparametri

`KUNTOSUUNNISTUS=1`

Kuntosuunnistusmoodiin päästään sitten avaamalla ohjelma tulospalvelutilaan ja valitsemalla
päävalikosta "Tulospalvelu/Emit-luenta". Vaihtoehtoisesti voidaan määritellä ohjelma aukeamaan
suoraan tulospalvelutilaan ja avaamaan käynnistyessään kaavake Emit-luenta.

Toimintamoodissa 1 kilpailijatiedosto ei alussa sisällä tietoja osanottajista, vaan heitä
koskevat tiedot ovat erillisessä mahdollisten osanottajien luettelossa (henkilötietokannassa).
Kilpailijatiedoston on tällöin sisällettävä niin runsaasti vakanttipaikkoja, että kaikki
osanottajat varmasti mahtuvat mukaan.

Toimintamoodissa 1 lähtijäksi ilmoittautuminen ja leimantarkastus tapahtuvat eri lukemiskerroilla.
Kun kortti luetaan ensimmäisen kerran, merkitsee ohjelma kilpailijan lähteneeksi ja tallentaa
emit-koodin osanottajan tietoihin. Kun Emit-kortti luetaan toiseen kertaan, tallennetaan tulos
ja vaihdetaan Emit-koodille vapaa arvo pienten lukujen alueelta. Täten kolmas lukeminen
on taas uuden kilpailijan ilmoittautuminen aivan samaan tapaan kuin
ensimmäinen lukeminen. Ilmoittautuminen ja leimantarkastus voivat tapahtua
samalla tai eri koneilla edellyttäen, että koneiden välillä on tiedonsiirtoyhteys.

Tulos voidaan kuitenkin tallentaa tulos jo ensimmäisellä lukemiskerralla edellyttäen, että
leimat antavat hyväksytyn suorituksen jollekin radalla. Muussa tapauksessa tarvitaan toinen lukeminen
ja lukemisten välissä on luettava joku toinen kortti.

### A3.2 Esivalmistelut toimintamoodiin 1

Esivalmistelu sisältää seuraavat vaiheet:

- Kilpailun määrittely
  - Käytettävä tiedostokansio valitaan ja luodaan joko ohjelman ulkopuolella tai toiminnossa
    *Kilpailun luominen ja perusominaisuudet*- Luodaan ja nimetään kilpailun sarjat joko em.
      toiminnossa tai kopioimalla aiempi soveltuva tiedosto KilpSrj.xml kilpailun
      tiedostokansioon ennen kilpailun perustietojen määrittelyä. Sarjoihin on
      sisällytettävä ylimääräinen sarja vakanttitiedoille. Ohjelma pyrkii
      luomaan tarkoitukseen sopivan vakanttisarjan automaattisesti.- Perusominaisuuksissa on syytä määritellä otsikkotiedot ja antaa kilpailulle tunnuskoodi, joka
        eroaa muista kilpailuista.  
  - Ratojen määrittely ohjelman valinnassa *Valmistelu / Ratatiedot*
    .
    Jokaiselle sarjalle on määriteltävä rata, jonka nimi on sama kuin sarjan nimi. Nollausennakoksi
    määritellään *Nollaus lähtöhetkellä* ja maalin
    osalta on valittava *Maaliviivalla leimasin (ei online*). Myös
    vakanttisarjalle on radan määrittely suositeltavaa häiritsevien
    virheilmoitusten eliminoimiseksi. Vakanttisarjan rataan
    sisällytetään ainakin yksi rasti, jota ei ole metsässä ja jolle annetaan
    leimasinkoodi, jota ei myöskään ole käytössä, jotta kukaan ei voisi saada hyväksyttyä
    suoritusta vakanttisarjaan.
      
      
    - Konfiguraatiotiedoston laatiminen. Yksinkertaisinta on käyttää tiedoston nimenä oletusnimeä
      `Laskenta.cfg`. Tiedostoon on sisällytettävä ainakin seuraavat rivit:
        
       `KUNTOSUUNNISTUS=1
        
      LUKIJA=n  
      VAKANTTIALKU=kkk`

        
      missä parametrin `LUKIJA=n` numeroarvon on oltava käytettävän sarjaportin numeron, siis `LUKIJA=1`,
      jos käytössä on `COM1`. Parametri VAKANTTIALKU tarvitaan, kun leimantarkastuspisteitä on
      useita.
        
        
      Lisäksi on annettava tiedonsiirron edellyttämät parametrit, jos käytetään useampia yhteen liitettyjä koneita.
      Tällöin on myös määriteltävä eri koneiden vakanttialueet parametrilla `VAKANTTIALKU=nnnn` niin, että
      jokaiselle koneelle varmasti riittää omaan alueeseen kuuluvia vakantteja.
      Yhdellä koneista valitaan nnnn vastaamaan
      käytössä olevan numeroalueen alarajaa ja muilla keskelle tätä numeroaluetta.
      Edelleen voidaan antaa parametri `SEURATIETO=E`, jos ei haluta käsitellä seuratietoja,
      ja parametri `VAHVISTAAINA=E`,
      jos halutaan, että ohjelma hyväksyy ilman vahvistusta osanottajan, kun
      vaihtoehtoja on tarjolla vain yksi.   
        
      Parametri KUNTOLÄHTÖ soveltuu käytettäväksi ilmoittautumispisteen
      koneella, jos on varma, että pisteeseen tulee vain ilmoittautujia. Tätä
      parametria käytettäessä kaavakkeelta on karsittu kaikki ylimääräinen tieto.
      Valikon kautta on mahdollista ottaa tämä moodi käyttöön ja palata normaaliin
      leimantarkastuksen näyttöön.  
        
      Parametri KUNTOMAALI johtaa siihen, että ohjelma varautuu
      mahdollisuuteen, että suunnistaja saapuu maaliin suorituksen tehtyään, vaikka
      korttia ei ole kirjattu lähdössä. Tällöin ohjelma kysyy, onko kyse lähdöstä
      vai maaliintulosta. Ilman tätä parametria ohjelma olettaa kaikkien niiden
      olevan lähtijöitä, joiden korttia ei ole vielä luettu ja joilla ei ole
      hyväksyttyä suoritusta jollekin radalle. Jos kortilta löytyy hyväksytty
      suoritus, kysyy ohjelma joka tapauksessa, onko kyse maaliintulijasta.
        
        
      Luvussa 6.6.4 esitettävä tapa käyttää parametreja LÄHEMITx=V ja LÄHEMITy=I
      saattaa helpottaa tapahtumassa kerättyjen tietojen viimeistelyä rastiväliaikatulosteita varten.
        
        
      - Luodaan vakanttipaikat ohjelman ollessa esivalmistelumoodissa valinnassa *Osanottajat / Lisää vakantteja*.
        Vakantteja kannattaa luoda runsaasti ja niin, että jokaisen parametrilla `VAKANTTIALKU=nnnn` perään
        tulee enemmän vakantteja, kuin yhdelle koneelle voi enimmillään tulla osanottajia ilmoittautumaan.
        Vakantit luodaan vakanteille varattuun sarjaan vakanteiksi ja jättäen sukunimi ja seura tyhjiksi.
          
          
        Samaa vakantit sisältävää tiedostoa voidaan käyttää
        useissa eri tapahtumissa, jos sarjojen lukumäärä ja vakanttisarjan
        järjestysnumero sarjaluettelossa on kaikissa sama.   
          
        Ohjelman
        automaattisesti luomien vakanttien lukumäärää säädellään parametrilla VAKANTTILUKU=nnn esivalmisteluvaiheen
        konfiguraatiotiedostossa ilmoitt.cfg tai tulospalveluvaiheen
        konfiguraatiottiedostossa. Tähän
        tiedostoon sisältyvä parametri LISÄVAKANTTIALKU=nn
        kertoo vakanttien numeroalueen alkupisteen. Ilman tätä
        parametria lisätään vakantit numeroalueen loppuun.
          
          
        - Valmistellaan henkilötietokanta aiempien osanottajien perusteella. Tiedoston nimeksi annetaan `henkilot.csv`.
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
            - Kaikki edellisissä vaiheissa luodut tiedostot
               `KilpSrj.xml, KILP.DAT, radat1.xml, henkilot.csv, Seurat.csv` kopioidaan jokaiselle tietokoneelle kilpailun
              työhakemistoon, jonne sijoitetaan myös konekohtaisesti sovitettu
              konfiguraatiotiedosto.
                
                
              - Haluttaessa, että ohjelma käynnistyy automaattisesti käyttötilaan
                - käynnistetään ohjelma haluttuun tilaan- tallennetaan ikkunat valinnassa *Tiedostot / Tallenna ikkunat*- valinnassa *Tiedostot / Avausmääritykset*
                      - Valitaan konfiguraatiotiedosto- Sisällytetään siihen rivi `ikkunat`, johon lisätään tiedoston nimi,
                          jos ikkunat tallennettiin muuhun tiedostoon kuin `ikkunat.xml`. Mahdolliset muutokset on
                          tallennettava.- Valitaan *Avaa suoraan tulospalveluun* ja tallennetaan valinnat.- Kun halutaan lopettaa automaattinen käynnistyminen, valitaan tallennettavaksi
                              *Avaa päävalikkoon*

### A3.3 Toiminta tapahtuman aikana

Ohjelma käynnistetään. Siirrytään tulospalvelutilaan ja valitaan käytössä oleva
hakemisto sekä konfiguraatiotiedosto, ellei näitä toimintoja ole automatisoitu edellä
kuvatulla tavalla. Edelleen valitaan valikosta *Tulospalvelu / Emit-luenta*, ellei
tämäkin tapahdu automaattisesti.

Osanottajan saapuessa lähtöön, luetaan kortti. Tällöin voi tapahtua seuraavia asioita:

- Jos emit-koodi löytyy henkilötietokannasta yhdeltä osanottajalta, kysyy ohjelma,
  onko henkilö oikea ja tallentaa hänet osanottajaksi, jos vastataan *Kyllä*.
  Vastaus *Ei* johtaa samaan haaraan kuin kortti, jota ei ole tietokannassa.
  Parametrilla tai talikosta voidaan poistaa tämän vahvistuksen pyyntö.  
  - Jos emit-koodi löytyy henkilötietokannasta useammalta osanottajalta, pyytää ohjelma
    valitsemaan oikean henkilön käyttäen näppäimiä *nuoli ylös / nuoli alas*. Kun
    oikea henkilö on valittuna, painetaan *Enter* ja sitten *Ctrl-Enter* tai
    vahvistetaan painikkeesta *Tallenna*.  
    - Jos emit-koodia ei löydy tietokannasta, siirtyy ohjelma sukunimi-kenttään, missä
      aloitetaan nimen kirjoittaminen. Jos nimi löytyy tietokannasta, voi sen valita, kun
      vaihtoehdot ovat vähentyneet riittävästi, käyttäen näppäintä *Enter* tai selaten
      ennen sitä näppäimillä *nuoli ylös / nuoli alas*. Kun nimi on oikein, tarkastetaan
      haluttaessa seura ja sarja ja hyväksytään näppäimillä *Ctrl-Enter* tai
      painikkeesta *Tallenna*.- Jos osanottajan kortilla on tiedot, jotka vastaavat
        hyväksyttyä suoritusta jollain käytössä olevalla radalla, kysyy ohjelma, onko
        kyse ilmoittautumisesta vai maaliintulosta.- Jos kilpailija tulee maaliin ilman edeltävää ilmoittautumista ja niin, että leimoissa
          on virhe, on kortti luettava uudelleen sen jälkeen, kun joku muu kortti on luettu väliin.
          Jotta kortti ei sammuisi ennen uutta lukemista, on leimattava pian ensimmäisen lukemisen
          jälkeen jollain leimasimella, mikä ei ole maalin leimasin.

Kun aiemmin ilmoittautunut osanottaja tulee maaliin, luetaan kortti. Ohjelma toteaa
heti tuloksen, näyttää leimat ja päättelee radan leimoista. Yleensä ei tarvitse tehdä
mitään muuta, mutta tietoja voi muokata, jos ne eivät ole oikein. Muokkausta tehtäessä
tulee näkyviin painike *Tallenna* merkiksi siitä, että muutokset on tallennettava
joka painikkeesta tai näppäimillä *Ctrl-Enter*.

Aiemmin luetut tulokset saa näkyville selailemalla selailupalkista tai käyttäen
painiketta *Hae osanottajaa* ja käyttämällä esille tulevia hakumahdollisuuksia.
Myös näin näytölle tulleita tietoja voi muuttaa ja muutokset tallentaa.

### A3.3.1 Tuloksien tarkastelu

Tuloksia voi katsella päävalikon valinnassa *Seuranta / Tilanne*.

Osanottajatietoja voi katsella ja muokata myös valinnassa *Osanottajat / Osanottajat*
jonka kautta aukeaa taulukko.

Kummankin edellä mainitun valinnan taulukossa voi klikata osanottajaa saadakseen lisätietoja.
Esille tulevat tiedot eivät ole identtiset, vaan joiltain osin toisiaan täydentäviä.

### A3.4 Toiminta tapahtuman jälkeen

### A3.4.1 Tulosluettelot

Tulosluettelot html-tiedostoon tai paperille voi laatia valinnassa *Tulosteet*, missä
on valittava tulostettavat sarjat sekä halutun tulosluettelon tyyppi.

### A3.4.2 Tietokantojen ylläpito

Henkilötietokantatiedostoa voi täydentää avaamalla toiminnon, jossa se näkyy ja valitsemalla sitten
*Muokkaus / Lisää kilpailijat tietokantaan*.

Jos käytössä on MySQL-tietokanta, voi tapahtuman osanottotiedot siirtää sinne valinnassa
*Tiedostot / Kirjoita siirtotiedostoon*. Lisäohjeita luvussa 11.

---

 Copyright 2012, 2015 Pekka
Pirilä