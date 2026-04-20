# 11.1 Kilpailijoiden pisteiden laskenta

### 11.1 Kilpailijakohtaisten pisteiden laskenta

#### 11.1.1 Yhden vaiheen pisteet

Pisteet lasketaan ensin vaihe kerrallaan ja voidaan sitten yhdistää yhteispisteiksi erillisenä toimenpiteenä.

Pisteet voivat määräytyä sijoituksen, tuloksen tai molempien perusteella.

Sijoitukseen perustuvat pisteet määritellään seuraavasti. Laskenta perustuu 1-5 sijoitusalueen käyttöön. Pisteiden
oletetaan muuttuvan kullakin alueella vakiomäärällä sijoituksen muuttuessa yhdellä. Viisi aluetta sallii nopeamman muutoksen
kärkisijoilla ja hitaamman muilla sijoilla.

Kun pisteet ovat osanottajamäärästä riippumatta samat
kullekin sijoitukselle, merkitään

- kohtaan *Sijasta* alueen paras sijoitus,

  - kohtaan *Sijaan* alueen
    huonoin sijoitus,

    - kohtaan *Ens. piste* ensin
      mainittua sijoitusta vastaava pistemäärä ja

      - kohtaan *Askellus* määrä,
        jolla pisteet muuttuvat sijoituksen huonontuessa. On huomattava, että tämä
        luku on negatiivinen, kun pisteet alenevat sijaluvun kasvaessa.

Jos halutaan sijoituksen mukana laskevat pisteet
kaikille tuloksen saaneille antaen viimeiselle esimerkiksi 1 piste, merkitään
kohtaan Sijasta luku 9999 ja kohtaan Sijaan paras sijoitus, jota tämä sääntö
koskee. Tieto Ens. piste koskee sitten viimeiseksi sijoittunutta ja paremmat
sijat saavat pisteensä askelluksen mukaan.  Jos pistemäärä nousee nopeammin
kärjessä, merkitään toiselle riville sijoitus, josta nousu kiihtyy. Tällöin on
saman sijoituksen oltava merkittynä ylemmän rivin kohtaan Sijaan ja seuraavan
rivin kohtaan Sijasta. Ohjelma jatkaa pisteiden määräämistä siitä pistemäärästä,
mihin ylemmän rivin sääntö johtaa.

Jos käytetään sijalukupisteitä, jotka kasvavat sijoituksen huonontuessa, merkitään kohtaan *Askellus* positiivinen luku. Tällöin voidaan
kaikkien sijoitusten ottaminen mukaan ilmaista luvulla 9999 kohdassa *Sijaan*.

Tulokseen perustuvat pisteet voidaan laskea joko niin, että pisteet alenevat
verrannollisesti aikaeroon voittajasta, tai niin, että pisteet ovat verrannolliset
voittajan ajan ja osanottajan ajan suhteeseen tai suhteen logaritmiin.
Edellisessä tapauksessa johtaisi hyvin suuri tappio negatiivisiin pisteisiin,
mutta pisteille voidaan antaa alaraja. Jälkimmäisessä tapauksessa suurikin
tappio antaa positiiviset pisteet, joten alarajaa ei tarvita. Aikaeroa voidaan
käsitellä sekä sellaisenaan lukuna, joka kertoo eron sekunnin sadasosina tai
suhteessa kärkiaikaan.

Pisteet voidaan määrittää summana sijalukupisteistä ja tulokseen
perustuvista pisteistä. Eri tekijöiden painoja voi muuttaa kertoimien avulla. Jos
esimerkiksi sijoituksille halutaan ensisijainen paino, voidaan käyttää hyvin suurta askellusta,
esim. miljoonaa.

Ohjelma käsittelee pisteitä kokonaislukuina, joten
desimaalien käyttö edellyttää pisteiden laskemista esimerkiksi 100-kertaisina,
jolloin niihin sisältyy kaksi desimaalia. Tulostusvaiheessa on mahdollista
lisätä desimaalipilkku oikeaan paikkaan käyttäen tulostuskaavakkeen valikon
valintaa *Muotoilu / Pisteiden desimaalit*.

#### 11.1.2 Yhteispisteiden laskenta

Yhteispisteet lasketaan summana päiväkohtaisista pisteistä kaikilta vaiheilta aktiivisena olevaan saakka. Huomioon otettavien pisteiden määrä
voidaan rajoittaa vaiheiden määrää alhaisemmaksi, jolloin huomioon otetaan parhaat pisteet tuohon rajaan saakka.

Joissain kilpailusarjoissa on mahdollista, että kilpailija osallistuu eri kerroilla eri sarjaan. Tällöin on tarjolla mahdollisuus
pisteiden laskemiseen erikseen kullekin sarjalle, mutta tämä vaihtoehto on toteutettu toistaiseksi vain Aktia Cupin sääntöjen mukaisena.

Pisteet voidaan laskea sekä tavanomaisille sarjoille
että sarjayhdistelmille, mutta samalle kilpailijalle voidaan kulloinkin antaa
vain yhdet pisteet, joten kukin sarjayhdistelmä pitää käsitellä kerralla
vaiheiden laskennasta tulosteiden laatimiseen ennen toisten samoja kilpailijoita
koskevien pisteiden laskentaa.

#### 11.1.3 Pisteiden laskenta suoraan tuloksesta

Ohjelma laskee pyydettäessä pisteet samalla, kun se
tallentaa tuloksen. Jotta näin saataisiin oikeat pisteet, ei pistemäärä saa
riippua muusta kuin kilpailijan omasta tuloksesta. Laskennassa käytettävä
vertailuaika on kiinnitettävä ennen kilpailua.
Jos lopulliset pisteet riippuvat sarjan voittotuloksesta, ovat kilpailun aikana
laskettavat pisteet likimäärin oikeat, jos voittoaika pystytään arvioimaan
ennalta kohtuullisen tarkoin.

Joissain tapauksissa voi
pisteiden laskennassa käytettävä kaava olla monimutkaisempi. Tällainen kaava voidaan
mahdollisesti määritellä kaavakkeen *Sarjatiedot* valikosta.
Täten laskettavat pisteet määrätään ilmoittamalla tulos, joka antaa nolla
pistettä sekä kertoimet, jotka kertovat, kuinka nopeasti pisteet kasvavat
tuloksen ollessa tuota rajaa paremman. Useimmiten on valinnasta *Tulospalvelu
/ Laske kilpailijoiden pisteet* kuitenkin tarkoituksenmukaisempi. Pisteiden
jatkuva laskenta otetaan käyttöön merkitsemällä *Laske vaiheen pisteet aina
tulosta tallennettaessa* sekä *mahdollisesti Laske yhteispisteet aina
tulosta tallennettaessa.* Lisäksi voidaan ohjelmaa pyytää laskemaa
yhteispisteet myös niille, jotka eivät saa käynnissä olevassa vaiheessa tulosta joko
hylkäyksen tai lähdöstä pois jäämisen takia. Tämä mahdollisuus on
tarjolla, koska aiempien vaiheiden pisteet voivat joissain tapauksissa
oikeutta hyväänkin sijoitukseen.

Merkintä *Käytä yhteispisteitä yhteistuloksena*
johtaa siihen, että yhteispisteet ja piste-ero yhteispisteiden kärkeen
näytetään kuuluttajan seurantanäytöillä.

#### 11.1.4 Pisteiden laskentakaavojen tallennus

Kaavakkeella *Pistelasku* määritellyt kaavat voi
tallentaa sarjatietoihin käyttämällä kaavakkeen valikon valintaa. Tallennus
tapahtuu seuraavasti:

- Tallennus tehdään kaikille valituille sarjoille.

  - Vaihtamalla valitut sarjat voidaan eri sarjoille
    tallentaa erilaiset kaavat

    - Kaavat tallennetaan vaihekohtaisesti sisältämään
      sekä kyseisen vaiheen vaihekohtaiset pisteet että yhteispisteiden laskennan
      kyseiseen vaiheeseen saakka. Eri vaiheille on kaavat tallennettava ko. vaiheen
      tulospalvelutilassa.

      - Valittuna olevien sarjojen kaavat tulevat sen
        mukaisiksi, kuin kaavakkeella näkyy
        määriteltynä. Muiden sarjojen kaavat tallennetaan aiemmassa muodossaan. Tiedostoon
        kirjoittaminen koskee tällä tavoin kaikkia sarjoja, koska kaikki tallennetaan samaa
        sarjatiedostoon.

Kun ohjelma käynnistetään tallennuksen jälkeen, lukee
ohjelma kaavat automaattisesti ja niitä voidaan käyttää painikkeen *Laske
kaikki pisteet kunkin sarjan tallennetun säännön mukaan* kautta. Kaavat
saadaan näytölle valitsemalla joku sarja käsiteltäväksi ja valitsemalla
valikosta *Lue sarjat tiedostosta*. Tällöin tuo ohjelma kyseisen sarjan
kaavat näytölle ja lisää käsiteltävien sarjojen listaan kaikki sarjat, joille on
määritelty nämä samat kaavat. Tämän jälkeen voidaan kaavoja muuttaa ja valita,
mitä sarjoja muutokset tulevat koskemaan.

Sarjatiedoista voidaan hakea myös edellisen vaiheen
kaavamääritykset. Haku koskee kaikkia sarjoja, mutta
näytölle saadaan yhden sarjan kaavat ja valituiksi tulevat ne sarjat, joilla
on samat kaavat. Kun tiedot tallennetaan tiedostoon, tallentuvat kaikki sarjat, myös
ne, joita ei ole katseltu.

#### 11.1.5 Pisteiden laskentakaavojen katselu

Kaikkia eri sarjoille ja eri vaiheille määriteltyjä
laskentakaavoja voi tarkastella taulukossa, joka avautuu pistelaskentakaavakkeen
valikkovalinnalla. Sarakkeiden merkitys selvinnee vertailemalla
määrittelykaavakkeen kenttiin. Taulukossa
ei voi muokata kaavoja.