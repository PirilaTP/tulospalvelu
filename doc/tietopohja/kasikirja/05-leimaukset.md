# 05 Emit-kortin luenta ja leimantarkastus

<!-- malli: Käsikirjan lukupohja (K-3). Ohje: doc/tietopohja/laatu/k3-kirjoittajan-ohje.md.
Luvun 5 vikaluettelo generoidaan tiedostosta kasikirja/toiminnot.csv komennolla
python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kasikirja; sitä ei kirjoiteta käsin. -->

> **Tila:** alustava K-3-aineisto, ei riippumattomasti tarkastettu. Lähderevisio: tuotantokoodi
> `f7fe553` (ennallaan, `doc/arkkitehtuuri/CHECKPOINT.md`). Varmuusmerkinnät: [todennettu],
> [päätelty], [tuntematon]; "luettu käsikirjaa varten" = luettu koodista K-vaiheessa, ei tarkastettu.

Tämä luku jatkaa siitä, mihin luku 04 (ajanotto) ja luku 10 (laitteet) jättävät. Luvun runko on
tarkastettu aineisto `aineisto/emit-luenta.md` (jäljempänä "aineisto"). Sen taulukoita ei toisteta:
kun tämä luku viittaa kohtaan "A3", "D5" tai "luku 8.6", tarkoitetaan aineiston vastaavaa kohtaa.
Lyhenteet: HKM = HkMaali, HKW = HkKisaWin, VM = ViestiMaali, VL = ViestiLuenta, VW = ViestiWin.
Polut ovat suhteessa hakemistoon `TPsource/V52/`.

## 1. Mitä tapahtuu

### 1.1 Kolme asiaa, jotka kannattaa pitää erillään

Emit-kortin käsittely koostuu kolmesta toisistaan riippumattomasta asiasta [todennettu:
aineisto luku 1, 2.2-2.6].

1. **Luenta.** Lukija antaa kortin koodin, lukuhetken ja leimat. Ohjelma lisää ne emit-tiedoston
   loppuun. Kortti ei vielä kuulu kenellekään kilpailijalle (aineisto A1-A4, B1-B7, C1-C6).
2. **Käsittely.** Eri säie ottaa emit-tiedostosta seuraavan käsittelemättömän kortin, etsii
   kilpailijan kortin koodilla, tarkastaa leimat radan mukaan, määrää maaliajan ja tallentaa tuloksen
   (aineisto D1-D12).
3. **Välitys.** Kilpailijan tulos ja emit-tietue lähtevät muille työasemille omina pakettinaan
   (aineisto F1-F5, G1-G5).

Tämä luku käsittelee kohdan 2 sisällön eli leimantarkastuksen, hylkäyksen ja leimoista lasketut
väliajat. Kohdat 1 ja 3 kuvataan aineistossa; tähän on koottu vain se, mitä leimantarkastuksen
ymmärtäminen vaatii.

### 1.2 Luennan ja välityksen tiivistelmä

Lukijan kuittaus (Emit-kellon `+`) kertoo enintään, että luennan ensimmäistä vaihetta on yritetty;
se ei kerro, että kortti on tallennettu tai yhdistetty kilpailijaan (aineisto 2.4, 8.3) [todennettu].
Kirjoitusfunktion kutsu, jonoon laitto, lähetys, vastaanotto ja kuittaus ovat eri asioita: kuittaus
tarkoittaa vastaanottajan muistipuskuria eikä sitä, että vastaanottaja on käsitellyt tai
tallentanut paketin (aineisto 8.2, F4-F5) [F5 päätelty]. Kilpailijan tulos ja emit-tietue kulkevat
eri paketteina, joten työasemien tilat voivat erota (aineisto 8.6).

Kaksoiskappaleet suodatetaan kolmella tasolla: laitteessa (LUKIJA: `lukutoisto`), `tall_emit`issa
(`samatleimat`, joka vertaa aikoja itseensä, joten ne eivät vaikuta; H-168) ja `addem`issa
(`on_emit`, avain lukuaika ja koodi; aineisto 8.5) [todennettu].

### 1.3 Mistä leimantarkastus lähtee

Leimantarkastus tarvitsee kolme asiaa [AA: käyttöohje `hkkisawin/6.1_leimantarkastuskaavake.md`;
koodi luettu käsikirjaa varten, ei tarkastettu]:

- **Lukija**, jonka asetus on `LUKIJAx`, `MTRx`, `EMITKELLO` tai `EMITAGx` (luku 10).
- **Ratatiedot:** radat ja rastien järjestys. HK lukee ne XML-tiedostosta `RADAT<n>.XML`
  (`lue_radat_xml`, `Hk/HkEmit.cpp:847`) tai tekstimuotoisesta `RADAT*.LST`-tiedostosta
  (`lue_radat`, `Hk/HkEmit.cpp:2857`) [AA: H-058, H-134].
- **Rastien leimasinkoodit:** HK lukee tiedoston `LEIMAT.LST` tai vaihekohtaisen `LEIMAT<n>.LST`
  (`Hk/HkInit.cpp:2354`; jos tiedostoa ei löydy, ohjelma kysyy, jatketaanko ilman sitä)
  [luettu käsikirjaa varten, ei tarkastettu].

Ilman rataa leimantarkastus ei onnistu: `tark_badge` palauttaa virhekoodin, ja käsittelijä näyttää
ilmoituksen "jo tarkastusmerkintä tai rataa ei löytynyt" (`Hk/HkIV.cpp:1633-1651`)
[luettu käsikirjaa varten, ei tarkastettu].

### 1.4 Käsittelyn kulku leimantarkastuksen kannalta (HK)

Käsittelyn vaiheet ovat `pros_emit`issa (`Hk/HkIV.cpp:1162-1868`; aineisto 2.6). Leimantarkastuksen
kannalta olennainen järjestys [luettu käsikirjaa varten, ei tarkastettu, ellei toisin mainita]:

1. **Kilpailijan haku kortin koodilla.** `tark_badge` kutsuu `bdg2kno(badge)`, joka antaa
   kilpailijanumeron koodista. Jos koodia ei ole millään kilpailijalla, kortti jää ilman
   kilpailijaa; ohjelma ohjaa sen vakanttitietueelle tai kysyy numeron (aineisto luku 4,
   "Tuntematon kortti") [AA: aineisto].
2. **Radan haku.** `haerata(kilp)` (`Hk/HkEmit.cpp:1957`) etsii radan kolmella tavalla tässä
   järjestyksessä: sarjalle määritelty rata, rata jonka tunnus on sama kuin sarjan nimi, kilpailijan
   oma ratakoodi. Ensimmäinen osuma kelpaa.
3. **Leimojen vertailu rataan.** `tarkista` (`Hk/HkEmit.cpp:2029`) käy kortin leimat läpi lopusta
   alkaen ja vertaa niitä radan rasteihin (kohta 1.5).
4. **Tarkastusmerkintä.** `tark_badge` (`Hk/HkEmit.cpp:2263`) asettaa kilpailijan tarkastustilaksi
   `T` (leimat kunnossa) tai `I` (leimausvirhe) ja tallentaa kilpailijan (kohta 1.6).
5. **Maaliaika.** Jos radan maalilaji on 4, maaliaika lasketaan kortin omista ajoista
   (`e_maaliaika`, `Hk/HkEmit.cpp:3618`); muuten käytetään ajanoton aikaa (luku 04) tai Emit-kellon
   maaliaikaa (`package` yli 10 000 000, aineisto B2). Ks. kohta 1.7.
6. **Tallennus ja välitys.** Aineisto D8-D10.
7. **Väliajat ja raportit.** `laskeEm_VA`, `addemitanal`, automaattinen raportti (kohta 1.9-1.10).

### 1.5 Miten `tarkista` päättää, onko suoritus kunnossa

Tässä kuvataan toiminta tulosten kannalta; algoritmin yksityiskohdat ovat koodissa
(`Hk/HkEmit.cpp:2029-2236`) [luettu käsikirjaa varten, ei tarkastettu].

- Kortin muistissa on enintään `MAXNLEIMA` (50) leimaa koodeineen ja kortin kellon mukaisine
  aikoineen. Lukijalaitteen oma koodi (esimerkiksi 250) merkitsee luentapaikan. `haelukija` etsii
  sen paikan, josta tarkastus alkaa.
- Rata käydään läpi viimeisestä rastista alkaen. Jokaiselle radan rastille etsitään kortilta
  leima, jonka koodi vastaa rastia. Kortilla olevat ylimääräiset leimat (rataan kuulumattomat
  rastit, kisaa edeltävä nollaus) eivät estä hyväksyntää, ellei niiden takia radan oikea rasti jää
  puuttumaan.
- **Vapaa järjestys.** Radan rastikoodit 10000 ja 10001 rajaavat osuuden, jossa rastit saa käydä
  missä järjestyksessä tahansa (`vapaajarj`). Rastikuvioon (`kuvio`) voi merkitä rastin
  puuttuvaksi hyväksytysti: koodi 2 tarkoittaa, että rasti on merkitty puuttuvaksi ja se ohitetaan.
- **Paluuarvo.** 0 = kaikki rastit löytyivät, -1 = rataa tai kilpailijaa ei löytynyt, positiivinen
  luku = ensimmäisen puuttuvan rastin järjestysnumero, -2 = kaikki rastit löytyivät mutta
  kortin kello on nollautunut kesken radan, joten aikojen laskenta ei ole mahdollista
  (`Hk/HkEmit.cpp:2231-2236`).
- **Tulkinta.** Kun kutsuja pyytää tulkintataulukon (`tulkinta`), `tarkista` kirjaa jokaiselle
  kortin leimalle, mitä radan rastia se vastaa (positiivinen = oikea, negatiivinen = virheellinen
  tai ylimääräinen). Taulukkoa käyttävät leimaruudun värit, väliaikalaskenta ja raportit.

Hyväksytyn suorituksen kriteeri on siis rastien löytyminen radan järjestyksessä (tai vapaan
järjestyksen osuudella missä järjestyksessä tahansa). Aineisto ei kerro, onko ajoaikaisia
rajoituksia (esimerkiksi rastien välinen vähimmäisaika) tarkastuksessa; koodissa ei luetun osan
perusteella ole sellaista [päätelty; kohdan 7 avoin kysymys].

### 1.6 Tarkastusmerkintä ja hylkäys

Kilpailijan tarkastustila on yksi merkki (`kilptietue::tark`, vaihekohtainen). Leimantarkastus
käyttää näitä merkkejä [luettu käsikirjaa varten, ei tarkastettu; merkitykset: käyttöohje
`hkkisawin/6.1_leimantarkastuskaavake.md` ja `6.16_hylkaysesitysten_kasittely.md`]:

| Merkki | Merkitys | Kuka asettaa |
|---|---|---|
| `-` | ei tarkastusmerkintää (tulos ilman leimantarkastusta) | oletus; myös esitarkastus ja esiluenta (`Hk/HkEmit.cpp:2274-2275`) |
| `T` | leimat tarkastettu, suoritus virheetön | `tark_badge`, kun `tarkista` palauttaa 0 tai pienemmän (myös -2) |
| `I` | leimausvirhe: ohjelma on tehnyt kilpailijalle **hylkäysesityksen** | `tark_badge`, kun `tarkista` palauttaa positiivisen luvun |
| `H` | hylätty | käyttäjä valitsee leimantarkastuskaavakkeen kohdassa *Tarkastus* |
| `E`, `B`, `K`, `O`, `V` ym. | ei lähtenyt, keskeytti jne. | käyttäjä tai muu toiminto |

Ohjelma ei hylkää kilpailijaa itse. Se asettaa `I`-merkinnän, ja järjestäjä päättää
hylkäämisestä (`H`) kaavakkeella. Tämä on käyttöohjeen mukainen toiminta ("kilpailijalle tulee
hylkäysesitys. Kohdassa *Tarkastus* on sitten valittava suoritusta vastaava vaihtoehto")
[AA: käyttöohje] ja koodin mukainen (`tark_badge` asettaa vain `T` tai `I`; `Hk/HkEmit.cpp:2283-2308`)
[luettu käsikirjaa varten, ei tarkastettu].

`I` ei sulje kilpailijaa tuloksista: `hyv_pv` laskee hyväksytyiksi merkit `-`, `I` ja `T`
(`Hk/HkTls.cpp:1208-1211`), joten hylkäysesityksen saanut kilpailija on tuloslistalla, kunnes
tarkastusmerkintä muutetaan [luettu käsikirjaa varten, ei tarkastettu]. Monivaiheisessa
kilpailussa `ytark` käsittelee `I`:n kuin `-` (`Hk/HkTls.cpp:1517`, `:1534`).

Hylkäysesityksen käsittelylomake tallentaa päätökset kilpailijatietoihin (tekstikentissä on
17 merkkiä) ja XML-tiedostoon, jonka hakemisto annetaan asetuksella `RAPORTTIHAK=` [AA: käyttöohje
`6.16`]. Lomakkeen sisäistä toimintaa ei ole luettu.

Kun järjestäjä korjaa tarkastusmerkintöjä ryhmänä, Emit-lomakkeen aikojen uudelleenlasku antaa
valinnat "Virheettömät avoimet ja hylkäysesityksen saaneet hyväksytään samalla" ja "Merkitäänkö
samalla hyväksyntä myös virheettömille hylätyille ja hylkäysesitys leimausvirheille"
(`cbHk/UnitEmit.cpp:3378-3380`) [luettu käsikirjaa varten, ei tarkastettu]. Valinta voi siis muuttaa
suurta joukkoa tarkastusmerkintöjä kerralla; seurausta tuloksiin ei ole selvitetty.

**Kaksi kortin koodia (`kaksibadge`).** Kun asetus `kaksibadge` on 2, kilpailijalla on kaksi
koodia ja tarkastusmerkintä tallennetaan osuuskohtaisesti (`asakot[osuus]`). Kilpailijan
yhteismerkintä muodostetaan kummankin osan merkinnöistä (`Hk/HkEmit.cpp:2288-2306`,
`cbHk/UnitEmit.cpp:2655-2690`) [luettu käsikirjaa varten, ei tarkastettu; käyttötapaus
(esimerkiksi parisprintti) on aineistosta tuntematon].

### 1.7 Maaliaika ja lähtöaika kortilta

`pros_emit` määrää maaliajan kolmella tavalla, riippuen radasta ja lukijasta
(`Hk/HkIV.cpp:1675-1803`; aineisto D6-D7) [luettu käsikirjaa varten, ei tarkastettu]:

| Tilanne | Mistä maaliaika tulee |
|---|---|
| Radan `maalilaji` on 4 ja kortin `maali`-kenttä on tyhjä | kortin omista ajoista: viimeinen leimattu rasti ja kortin kello (`e_maaliaika`), siirrettynä kellonaikaan lukuhetken ja kortin kellon erotuksella |
| Emit-kello lähetti K-kehyksen (`package` yli 10 000 000 ja alle 11 000 000) ja `maalilaji` ei ole 4 | kellon antama maaliaika; aika viedään myös ajanottojonoon (`tall_rivi`) |
| Asetus `EMITAJAT` on päällä | leimantarkastuksessa määrätty aika kirjoitetaan ajanottojonoon jonoon `EMITAJAT=n` |
| Muu | maalin aika tulee ajanotosta (luku 04); kortti ei tuota aikaa |

Jos maaliaikaa ei kortilta pystytä määräämään, käsittelijä näyttää ilmoituksen "Kortin ...
maaliaikaa ei pystytty määräämään". Jos radalla on nollausennakko (`ennakko`, sekuntia ennen
nollausleimaa) ja lähtöaika voidaan laskea, ohjelma tallentaa lasketun lähtöajan kilpailijalle;
`lahtopoikk` antaa varoituksen, kun laskettu lähtöaika poikkeaa kilpailijalle merkitystä
(`Hk/HkIV.cpp:1717-1736`). Kun lähtöaikaa ei ole, ohjelma antaa lähtöajaksi 12.00.00, jos
`ennakko` on negatiivinen (ilman nollausleimaa), ja ilmoittaa siitä.

Kun kortin kellon nollautuminen kesken radan on havaittu (`tarkista` palauttaa -2), kortin ajoista
laskettava maaliaika ja väliajat eivät ole luotettavia; käsittelijä ilmoittaa asian
(`Hk/HkIV.cpp:1649`).

### 1.8 Radan ja sarjan automaattinen valinta

Asetukset `AUTOSARJA` ja `AUTORATA` (`Hk/HkInit.cpp`) sallivat, että ohjelma valitsee radan kortin
leimojen mukaan. `etsierata` käy kaikki radat läpi ja valitsee hyväksytyn suorituksen radoista sen,
jossa on eniten rasteja; yhtä monirastisista ensimmäisen (käyttöohje 6.1 kertoo saman) [AA:
käyttöohje; luettu käsikirjaa varten, ei tarkastettu]. Valinta vaihtaa kilpailijan sarjan
(`autosarja`) tai ratakoodin (`autorata`) ja asettaa tarkastusmerkinnän `T`, kun uusi rata antaa
virheettömän suorituksen (`Hk/HkIV.cpp:1675-1699`). Automaattinen valinta tehdään vain, jos kilpailijalla
ei ole rataa tai uusi rata on pidempi tai nykyinen rata ei hyväksy leimoja.

### 1.9 Väliajat leimoista

Leimoista syntyy kahdenlaisia väliaikoja. Ne ovat eri asioita [luettu käsikirjaa varten, ei
tarkastettu, ellei toisin mainita].

**a) Sarjan väliaikapisteiden tulokset kortilta (`laskeEm_VA`).** Sarjalle voi määritellä
väliaikapisteen (`mtb`-taulukko), jonka aika lasketaan kortin leimoista: alku- ja loppurasti
(`alkukdi`, `loppukdi`), tulokseen lisättävä pohjapiste (`pohja`) ja piste, johon aika tallentuu
(`Hk/HkEmit.cpp:6198-6240`). Käyttöohjeen mukaan tämä on tarkoitettu kilpailumuotoihin, joissa osa
rastiväleistä sisältyy tulokseen ja osa ohitetaan (`hkkisawin/6.14_tulosten_laskenta_rastivaliajoista.md`)
[AA: käyttöohje]. Sama rasti: vain kortin ensimmäinen leimaus kelpaa. Laskenta ajetaan, kun kortti
on käsitelty ja sarjalle on valittu "laske kortilta" (`laskemtb`; `Hk/HkIV.cpp:1854-1855`), tai
käyttäjän painikkeella kaikille kerralla. `laskeEm_VA(d, false)` kutsuu `tallenna`a ilman `tall`-lukkoa
(H-010; aineisto D11). Jos kortilta puuttuu loppurasti, aika asetetaan 5 tunniksi (`Hk/HkEmit.cpp:6226-6228`):
tuloksena on siis tarkoituksellisen suuri arvo eikä tyhjä.

**b) Rastivälianalyysi (`emitanal`).** Kun asetus `EMITANAL` on päällä, `addemitanal` laskee
jokaisesta hyväksytystä kortista rastivälien ajat (`laskeemitvaliajat`) ja päivittää
rastiväliaikojen tilastot (`rastiva`, `rvalit`): sarjakohtaiset summat ja järjestetyt ajat, joita
rastiväli-vertailu ja raportit käyttävät (`Hk/HkEmit.cpp:2652-2794`). Vain osuus 0 ja kortti, jonka
`tarkista` palauttaa 0, otetaan mukaan. Tilastoihin otetaan vain rastivälit, joiden aika on 0...3 tuntia
(`Hk/HkEmit.cpp:2730-2740`).

**c) EMITVA-paketti.** `LÄHEMVA`-asetuksen yhteydelle lähetetään jokaisesta kortista paketti
(`tee_emva`, `Hk/HkEmit.cpp:6112`), jossa on kortin koodi, kilpailijanumero, tulos, tarkastusmerkintä,
rata ja jokaisen rastin koodi ja aika (enintään `maxemvalah` rastia). Vastaanottaja voi kirjata sen
lokiin asetuksella `EMVALOKI` (H-073) [AA: `asetusavaimet.csv`; luettu käsikirjaa varten].
EMITVA:n käyttötapaus (esimerkiksi ulkopuolinen väliaikanäyttö) on aineistosta tuntematon.

**d) Muut väliaikalähteet.** emiTag-väliajat palvelimelta ja `tall_ec`-paketit (`AUTOVA`,
`VALUKIJA`) ovat ajanottoa eivätkä leimantarkastusta; ne kuvataan luvussa 04.

### 1.10 Raportit ja tulosteet

Automaattinen leimausraportti (`emit_rap`, `Hk/HkEmit.cpp:4556`) tulostetaan, kun `AUTORAP` on
päällä: aina (arvo yli 1), vain `I`-merkityille kilpailijoille (arvo 1 ja `tark == I`) tai
sarjoille, joille on `autorapsrj`. Se ajetaan käsittelyn lopussa (`Hk/HkIV.cpp:1859-1861`) ja
vastaanotetun, jo käsitellyn kortin yhteydessä (`addem`; aineisto C5, G3), kun `comautorap`,
`kilpno` ja `emittulparam`-ehto täyttyvät. Tulostus käyttää kirjoitinta, jonka käyttäjä on
valinnut (luku 10, kohta "Tulostimet"). Muita tulosteita: rastiväliaikatuloste (`tulostaRastiVa`,
asetus `RASTIVATULOSTE` käynnistää tilan, jossa kortin luenta tulostaa kilpailijan väliajat)
[AA: käyttöohje `6.8_rastivaliaikatulosteet.md`; luettu käsikirjaa varten, ei tarkastettu],
tarkastusraportit (`tark_rap`, `tarkraportit`, `Hk/HkEmit.cpp:4986-5144`) ja leimalista (`emitlista`).
Tulosteiden sisältöä ei ole luettu.

### 1.11 Korjaukset leimantarkastuksessa

Kun kortti on tallentunut väärälle kilpailijalle tai vakantille, järjestäjä siirtää leimat oikealle
kilpailijalle Emit-lomakkeen muutosikkunalla (HKW, VW) tai konsolin kysymyksillä (HKM, VM).
Tämä toteutetaan funktioilla `vaihda_badge`, `addbadge`, `rembadge` ja `vaihdakilp`
(`Hk/HkEmit.cpp:1034-1189`, `:3374`). Käyttäjälle tärkeät seuraukset (aineisto 2.7 kohta E3,
luku 8.6 rivi 10):

- Koodin vaihto päivittää kaikkien kyseiseen koodiin liittyvien emit-tietueiden `kilpno`:n
  `tallenna`ssa, ilman `emit`-lukkoa (H-011).
- Koodien ja kilpailijoiden välinen hakutaulukko (`bdg_kno`) ja emit-linkit (`emitjarr`)
  ylläpidetään käsin; kahden koodin epäsymmetria (H-135) ja poistetun kilpailijan jäännöslinkki
  (H-140) ovat tunnetut kohdat.
- Lomakkeen sulkeminen X-painikkeella voi valita tuhoavan vaihtoehdon (H-116).
- Käyttäjä voi ohittaa kortin (`Em.kilpno = 0`, `Em.badge = 0`; aineisto luku 4).

**Lainakortit.** Tiedosto `LAINAEMT.LST` (tai `LAINAEMIT.LST`) sisältää lainakorttien koodit
(`lue_lainat`, `Hk/HkEmit.cpp:1399-1446`). Kun asetus `LAINAT` on päällä ja kortti on lainakortti,
käsittelijä näyttää ilmoituksen "LAINAKORTTI" (`Hk/HkIV.cpp:1856-1858`) [luettu käsikirjaa
varten, ei tarkastettu]. Lainakortin uudelleenkäyttö samalla radalla on yksi H-168:n
kaksoisluentatapauksista.

### 1.12 Viestin leimantarkastus

Viesti käyttää samaa ajatusta, mutta tarkastus on joukkue- ja osuuskohtainen
(`Juk/VEmit.cpp:2004` `tarkista`, `:2659` `tark_badge`) [luettu käsikirjaa varten, ei tarkastettu]:

- `tark_badge` etsii joukkueen osuuden, jolle kortin koodi on merkitty (`ostiet[os].badge[0]`) ja
  jolle emit-linkkiä ei vielä ole (`EmitJarr(kilpno, os) < 0`); osuus tallennetaan `em->osuus`:iin.
  Asetus `val_korkein` valitsee korkeimman sopivan osuuden.
- Hyväksytty suoritus merkitään osuudelle `T` (jos aiempi merkintä oli `-`), virheellinen `I` (jos
  aiempi merkintä oli `-` tai `T`). Partiokilpailussa (`partio`) merkintöjä ei aseteta tässä
  vaan `tark_partio`ssa.
- Maaliaika lasketaan `e_maaliaika(em, kilp, os)`:lla (`Juk/VEmit.cpp:1298`).
- Joustoviesti (`JOUSTOVIESTI`) valitsee osuuden sen mukaan, mille osuudelle kortti sopii
  (`haejoustoosuus`, `Juk/VIv.cpp:1733`; luku 04).
- Viestin konsolin partiokilpailu ja `AUTORAP` ilman listakirjoitinta kaataa ohjelman kortin
  luennassa (H-064).

## 2. Erot ohjelmien välillä

Taulukko täydentää aineiston lukua 7. Perusta: aineisto luku 7 ja projektien määritykset (aineisto
luku 6).

| Asia | HKM | HKW | VM | VW | VL |
|---|---|---|---|---|---|
| Emit-tiedoston luku ja tallennus | kyllä | kyllä | kyllä | kyllä | ei (kortti vain sisäänluentanäkymälle) |
| Leimantarkastus (`tarkista`, `tark_badge`) | kyllä (`Hk/HkEmit.cpp`) | kyllä | kyllä (`Juk/VEmit.cpp`, osuuskohtainen) | kyllä | ei [päätelty: `LUENTA`-käännös ei tallenna emit-tietuetta, aineisto luku 7] |
| Käsittelyn käynnistys | `leimat()`-näyttö | Emit-lomake, `ProsEmitThread` | konsolin `pros_emit` | `ProsEmitThread`, `TFormEmit::ProsEmit` | sisäänluentanäkymä |
| Käyttäjän kysymykset (tuntematon kortti, jo luettu) | kysely | automaattinen valinta, ilmoitus | kysely | `KysyKilpailija` | - |
| Tarkastusmerkintä | `kilptietue::tark` | sama | osuuskohtainen (`Tark(os, ...)`) | sama | - |
| Rastivälianalyysi (`EMITANAL`) | kyllä | kyllä | kyllä (`laskeemitvaliajat` `Juk/VEmit.cpp:3126`) | kyllä | ei |
| `laskeEm_VA` (väliajat kortilta) | kyllä | kyllä | kyllä (`Juk/VEmit.cpp:6178`, osuuskohtainen) | kyllä | ei |
| Kuntosuunnistuksen koodinvaihto | ei (vain `_BORLAND_`) | kyllä (`kuntosuunnmoodi`) | ei | ei | - |
| Esitarkastus (`ESITARK`) | asetus | asetus | käännösmakro ja asetus | asetus | ei |

[Taulukko perustuu aineiston lukuihin 6-7 ja `Hk/HkIV.cpp`-, `Juk/VEmit.cpp`-lukuun; VW:n osuus on
päätelty VM:n yhteisestä koodista ja aineiston luvusta 7.]

HK:ssa leimantarkastus ja kilpailijan haku tehdään `tall`-lukon ulkopuolella kortin luvussa
(`pros_emit` pääsäikeessä) ja `tark_badge` ottaa lukon vain tarkastusmerkinnän tallennukseksi.
Viestin `tallenna` lukitsee itse, HK:n ei (aineisto luku 7) [AA].

## 3. Asetukset ja käännösvalitsimet

Käännösvalitsimet (`EMIT`, `MAALI`, `EMITLEIMAT`, `LUENTA`, `EMITFILE`, `EMITHTTP`, `LAJUNEN`,
`TCPLUKIJA` jne.), tilat ja ohjelmakohtaisuus: aineiston luku 6, jonka projektimääritysten rivit
on tarkistettu uudelleen. Tässä vain leimantarkastukseen vaikuttavat avaimet. Asetuskohdat luettu
käsikirjaa varten (`Hk/HkInit.cpp`, `Juk/VInit.cpp`); avainten lista:
`doc/arkkitehtuuri/rekisteri/asetusavaimet.csv`.

| Asetus | Ohjelmat | Vaikutus | Lähde |
|---|---|---|---|
| `LUKIJAx`, `MTRx`, `EMITKELLO`, `EMITAGx` | HK ja viesti | lukija ja yhteys (luku 10) | [AA: osaD luku 1] |
| `EMIT` | HK ja viesti | käynnistää Emit-käsittelyn ilman lukijaa | [AA: käyttöohje `hkliite1.md:167`] |
| `ESTÄEMITTOISTO` / `SALLIEMITTOISTO` | HK ja viesti | `on_emit`-hylkäys (oletus: estetty) | aineisto luku 6 |
| `LUKUTOISTO` | HK ja viesti | LUKIJAn vaatimat identtiset sanomat | aineisto luku 6; H-175 |
| `AUTOSARJA`, `AUTORATA` | HK | rata tai sarja leimojen perusteella | `Hk/HkIV.cpp:1675-1699` [luettu] |
| `ESITARK`, `ESILUENTA` | HK; viestissä `ESITARK` | `ESITARK.DAT`; tarkastusta ei tehdä | aineisto luku 4, 6 |
| `AUTORAP` | HK ja viesti | automaattinen leimausraportti | `Hk/HkInit.cpp:2561` [luettu] |
| `EMITANAL` | HK ja viesti | rastivälianalyysi | `Hk/HkInit.cpp:2356` [luettu] |
| `LÄHEMITn[=I/O/V]`, `LÄHEMVA` | HK ja viesti | EMITT- ja EMITVA-paketit yhteydelle | aineisto luku 6; `Hk/HkInit.cpp:2726` [luettu] |
| `EMVALOKI` | HK ja viesti | EMITVA-paketin loki (H-073) | `Hk/HkInit.cpp:2732` [luettu] |
| `EMITAJAT[=n]` | HK ja viesti | leimantarkastuksessa määrätyt ajat ajanottojonoon | [AA: käyttöohje `hkliite1.md:207`] |
| `LAINAT`, `LAINALUETTELO` | HK | lainakorttien ilmoitus | `Hk/HkInit.cpp:2191-2240` [luettu] |
| `ORIGEMVABADGE` | HK | EMITVA:ssa kortin alkuperäinen koodi | `Hk/HkInit.cpp:2592` [luettu] |
| `MAALILEIMASIN=E` | HK | maalissa ei leimasinta | `Hk/HkInit.cpp:2530` [luettu; vaikutus `tee_emva`ssa ja radan rastiluvussa] |
| `RAPORTTIHAK=` | HK | hylkäysraporttien hakemisto | [AA: käyttöohje `hkliite1.md:182`] |
| `RASTIVATULOSTE`, `LISTA` | HKW | kortin luenta tulostaa väliajat | [AA: käyttöohje `6.8`] |
| `kaksibadge` | HK ja viesti | kaksi koodia kilpailijalla | `Hk/HkEmit.cpp:2288` [luettu] |

Käännösvalitsimista tässä luvussa merkitsevät: `EMITLEIMAT` (leimantarkastuksen koodi mukana),
`LUENTA` (VL: ei emit-tiedostoa), `_BORLAND_` (HKW: kuntosuunnistuksen koodinvaihto), `_CONSOLE`
(konsolin ja GUI:n eri haarat `pros_emit`issa) ja `EBA`, joka ei ole määritelty missään ohjelmassa
(kuollut haara `Hk/HkIV.cpp:1701-1705`) [AA: `valitsimet.csv`; aineisto luku 6].

## 4. Vaihtoehtoiset polut ja virhetilanteet

Aineiston luvut 4 (vaihtoehtoiset polut), 8 (onnistuminen) ja 9 (virhetilanteet) koskevat tätä
lukua sellaisenaan. Lisäksi leimantarkastukselle ominaiset tilanteet:

| Tilanne | Mitä tapahtuu | Lähde |
|---|---|---|
| Kilpailijalle ei löydy rataa | `tarkista` palauttaa -1; ei tarkastusmerkintää; käsittelijä ilmoittaa "jo tarkastusmerkintä tai rataa ei löytynyt" | [luettu käsikirjaa varten] `Hk/HkEmit.cpp:2059`, `Hk/HkIV.cpp:1633-1651` |
| Kortin kello on nollautunut kesken radan | `tarkista` palauttaa -2; tarkastusmerkintä `T`, mutta aikoja ei voi laskea; ilmoitus | [luettu käsikirjaa varten] |
| Radalla ei ole yhtään rastia | sama kuin radan puuttuminen: `tarkista` palauttaa -1 | [luettu käsikirjaa varten] `Hk/HkEmit.cpp:2058` |
| Ratatiedosto puuttuu vaiheen vaihdossa | emit-tarkastus lakkaa vaiheen ajaksi (`emitfl = 0`), vaikka tekstimuotoinen vara olisi olemassa | [AA: H-134] |
| Ratatiedoston lyhyt rivi tai yli 50 rastia | muistin ylitys ratatiedoston luvussa (HK: kasa, viesti: pino) | [AA: H-058] |
| `esitark` tai `esiluenta` | `tarkista` palauttaa 0 heti; tarkastusmerkinnäksi `-`; tulosta ei lasketa | `Hk/HkEmit.cpp:2038-2039`; `Hk/HkIV.cpp:1825-1833` [luettu] |
| Kaksi korttia (`kaksibadge == 2`) | osuuskohtaiset merkinnät `asakot`issa; yhteismerkintä kummankin mukaan | [luettu] `Hk/HkEmit.cpp:2288-2306` |
| `AUTOSARJA` tai `AUTORATA` päällä, kuntosuunnistustila 1 tai 3, mikään rata ei hyväksy leimoja | tarkastusmerkinnäksi `I` | [luettu] `Hk/HkIV.cpp:1695-1697` |
| Viestin partio + `AUTORAP` ilman listakirjoitinta | konsoliohjelma kaatuu kortin luennassa | [AA: H-064] |
| Emit-käsittelyn ja tulostuksen lukitusjärjestys | `emit` ja `print` voivat jäädä ristiin; ikkuna on yhden raportin mittainen | [AA: H-092; viestissä H-103, H-087] |

Seuraus, jonka vuoksi leimantarkastusta ei voi käsitellä erillään tallennuksesta: `tark_badge`
tallentaa tarkastusmerkinnän omassa `tallenna`-kutsussaan (`Hk/HkEmit.cpp:2308`), ja
`pros_emit` tallentaa kilpailijan sen jälkeen uudelleen maaliajan kanssa (aineisto D8). Kilpailijan
tarkastusmerkintä voi siis olla tallessa ilman maaliaikaa, jos käsittely keskeytyy tai jälkimmäinen
tallennus epäonnistuu (`KILP.DAT`-virhe: aineisto E5, H-015) [päätelty luetusta järjestyksestä].

## 5. Tunnetut viat tässä toiminnossa

<!-- generoitu:viat alku (tietopohja.py lohkot; älä muokkaa käsin) -->
| Toiminto | Havainto | Tila | Prioriteetti | Kuvaus |
|---|---|---|---|---|
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [H-164](../vikakortit/H/H-164.md) | avoin | P2 | Sarjaporttiin kytketty lukija, joka katoaa kesken ajon, jää hiljaa lukematta, eikä ohjelma avaa porttia uudelleen eikä ilmoita siitä |
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [H-138](../vikakortit/H/H-138.md) | avoin | P4 | Yhden TCP-lukijan epäonnistuneet yhteysyritykset pidentävät muiden TCP-lukijoiden yhteydenavauksen odotusta jopa neljällä minuutilla |
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [H-155](../vikakortit/H/H-155.md) | avoin | P4 | Tiedonsiirron pistokkeita (soketteja) jää sulkematta virhe- ja katkaisutilanteissa |
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [H-055](../vikakortit/H/H-055.md) | avoin | P2 | emiTag-sanoman leimapaikan numero voi kirjoittaa ohjelman pinomuistin taulukon ulkopuolelle |
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [H-056](../vikakortit/H/H-056.md) | avoin | P2 | Viallinen sanoma ARES-, Sirit- tai emiTag-lukijalta voi jättää lukijasäikeen pyörimään ja pysäyttää kyseisen laitteen syötteen |
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [H-170](../vikakortit/H/H-170.md) | avoin | P5 | Kokoomarivi: laitteiden, HTTP:n, selaimen ja SQL-sarakkeiden pienet viat (15 erillistä kohtaa) |
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [H-136](../vikakortit/H/H-136.md) | avoin | P3 | Lukijan kortti-aikataulu on kahden säikeen yhteinen eikä lukittu, joten kortin aika voi mennä väärin ja siirtää maali- ja väliaikoja |
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [H-094](../vikakortit/H/H-094.md) | avoin | P3 | Vaiheen vaihdon aikana lukijalta tullut merkki jää käsittelemättä, ja merkkijono voidaan vapauttaa kirjoituksen alta |
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [H-168](../vikakortit/H/H-168.md) | avoin | P4 | Emit-kortin toinen luenta samalla lukijalla samoin rastikoodein hylätään hiljaa kaksoiskappaleena ja kuitataan lukijalle |
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [H-098](../vikakortit/H/H-098.md) | avoin | P4 | MTR-lukijan toinen tiedostoon-luenta samassa istunnossa pysähtyy, koska päivityssäie päättyy väärässä kohdassa |
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [T-048](../vikakortit/T/T-048.md) | avoin | - | Ohjelma ei valvo, onko lukija tai muu laite yhä elossa, joten katkennut yhteys voi näyttää toimivalta |
| Emit-kortin luenta lukijalta (lukijasäie, tall_emit) | [T-024](../vikakortit/T/T-024.md) | avoin | - | Viestin lukijakanavien enimmäismäärä on konsoliohjelmissa 8 ja ViestiWinissä 32, joten sama asetustiedosto tulkitaan eri tavoin |
| Emit-tietueen tallennus emit-tiedostoon (addem, putem, uudelleenluku) | [H-145](../vikakortit/H/H-145.md) | avoin | P3 | Jos emit-tiedoston kirjoitus epäonnistuu, luettu kortti kuitataan lukijalle mutta jää tallentumatta |
| Emit-tietueen tallennus emit-tiedostoon (addem, putem, uudelleenluku) | [H-156](../vikakortit/H/H-156.md) | avoin | P5 | Pysyvän tiedon kirjoituksen pienet virhepolut: osa kirjoitusvirheistä jää huomaamatta tai ohjelma ilmoittaa onnistumisesta epäonnistuneen kirjoituksen jälkeen |
| Emit-tietueen tallennus emit-tiedostoon (addem, putem, uudelleenluku) | [H-053](../vikakortit/H/H-053.md) | avoin | P4 | Jos emit-tiedoston luku levyltä epäonnistuu, ohjelma ei huomaa sitä ja jatkaa puskurin vanhalla tai alustamattomalla sisällöllä |
| Emit-tietueen tallennus emit-tiedostoon (addem, putem, uudelleenluku) | [H-011](../vikakortit/H/H-011.md) | avoin | P3 | Kilpailijan emit-koodin tai numeron muutos kirjoittaa emit-tiedostoon ilman emit-lukkoa, kun lukija voi kirjoittaa samaan tiedostoon (HK) |
| Emit-tietueen tallennus emit-tiedostoon (addem, putem, uudelleenluku) | [H-045](../vikakortit/H/H-045.md) | avoin | P3 | Emit-analyysin kytkeminen päälle kesken ajon lukee emit-tiedoston uudelleen ilman emit-lukkoa (HkKisaWin) |
| Emit-tietueen tallennus emit-tiedostoon (addem, putem, uudelleenluku) | [H-105](../vikakortit/H/H-105.md) | avoin | P3 | Vaiheen vaihdon jälkeen emit-kortit voivat kirjoittua suljettuun tai väärään tiedostoon |
| Emit-tietueen tallennus emit-tiedostoon (addem, putem, uudelleenluku) | [H-034](../vikakortit/H/H-034.md) | avoin | P4 | Yhteyden palautuessa tai ohjelman käynnistyessä uudelleen sama sanoma tai emit-kortti voi tulla käsitellyksi kahdesti |
| Emit-tietueen tallennus emit-tiedostoon (addem, putem, uudelleenluku) | [H-141](../vikakortit/H/H-141.md) | avoin | P4 | Vaiheen vaihto säilyttää edellisen vaiheen erälähdön ja kaksoiskorttien eston laskurin |
| Emit-tietueen tallennus emit-tiedostoon (addem, putem, uudelleenluku) | [T-032](../vikakortit/T/T-032.md) | avoin | - | Ohjelma odottaa käyttäjän vastausta, tietokantaa tai ulkoista komentoa lukko hallussaan |
| Emit-tietueen tallennus emit-tiedostoon (addem, putem, uudelleenluku) | [T-040](../vikakortit/T/T-040.md) | avoin | - | Virheet kerrotaan käyttäjälle ikkunoina työsäikeistä eikä palauteta kutsujalle, joten kutsuja ei tiedä tallennuksen epäonnistuneen |
| Emit-tietueen tallennus emit-tiedostoon (addem, putem, uudelleenluku) | [T-041](../vikakortit/T/T-041.md) | avoin | - | Tiedostoja ei kirjoiteta kokonaisina tai kestävästi: julkaistavat tiedostot tyhjennetään paikallaan, eikä tallennusta pakoteta levylle |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-031](../vikakortit/H/H-031.md) | avoin | P2 | HkMaalissa evätty vaiheen vaihto jättää ohjelman tilaan, jossa myöhemmät vaihdot eivät tee mitään ja taustakäsittely voi pysähtyä |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-163](../vikakortit/H/H-163.md) | avoin | P3 | Jos SQL-yhteys epäonnistuu käynnistyksessä, loput käynnistyksen vaiheet jäävät tekemättä |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-018](../vikakortit/H/H-018.md) | avoin | P3 | Taustasäikeet kirjoittavat suoraan ikkunoiden palkkeihin ja kenttiin, vaikka niitä saa käsitellä vain käyttöliittymän pääsäie |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-107](../vikakortit/H/H-107.md) | avoin | P2 | HkKisaWin voi kaatua ensimmäisestä kortinluvusta, jos tarvittavaa lomaketta ei ole avattu (asetus RASTIVATULOSTE tai valmistelutila) |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-012](../vikakortit/H/H-012.md) | avoin | P3 | HkMaali: emit-tietojen siirto poissaolevaksi merkityltä kilpailijalta jättää kilpailijatietojen lukon päälle |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-010](../vikakortit/H/H-010.md) | avoin | P3 | Noin 35 kohtaa tallentaa kilpailijan tiedot ilman lukkoa, ja tallennus muuttaa rakenteita, joilla ei ole omaa lukkoa (HK) |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-120](../vikakortit/H/H-120.md) | avoin | P4 | HkKisaWinissä kortin paikanpitäjätietue jää omistamaan kortin, kun numero kirjoitetaan käsin, ja oikea kilpailija tallentuu ilman korttia |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-119](../vikakortit/H/H-119.md) | avoin | P4 | HkKisaWinin emit-lomakkeessa väärin kirjoitettu numero kopioi paikanpitäjän tiedot olemassa olevalle kilpailijalle ilman kysymystä |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-081](../vikakortit/H/H-081.md) | avoin | P5 | Kuntosuunnistustilassa HkKisaWin vaihtaa luetun kortin koodin seuraavaan vapaaseen, HkMaali ei |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-112](../vikakortit/H/H-112.md) | avoin | P3 | ViestiWinin "Vaihda kilpailijoiden leimaukset ristiin" voi korvata joukkueen kortin numeron roskatiedolla, jos joukkueen numero tai osuus puuttuu |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-017](../vikakortit/H/H-017.md) | avoin | P3 | Emit-muutokset-ikkuna voi kirjoittaa kilpailijan vanhan tietueen takaisin ja kumota välissä tulleet muutokset |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-116](../vikakortit/H/H-116.md) | avoin | P2 | Valinta- tai virheikkunan sulkeminen X-painikkeella tulkitaan painikkeeksi 2: emit-lomakkeissa tuhoava valinta, virheikkunassa ohjelman lopetus |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-175](../vikakortit/H/H-175.md) | avoin | P4 | Emit-lomakkeen valikon rastit eivät aina vastaa sitä, mitä ohjelma oikeasti tekee: "Vaadi kaksoisluenta" ja "Vahvista aina" voivat näyttää väärää tilaa |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-126](../vikakortit/H/H-126.md) | avoin | P5 | ViestiWinin viisi pientä logiikkavikaa: väärä osuuden tulkinta uusintatallennuksessa, alustamaton ilmoitusteksti, kuluva muistiolinja, viallinen merkkitarkistus ja rajaamaton osuuden jäsennys (kokoomarivi) |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-127](../vikakortit/H/H-127.md) | avoin | P5 | HkKisaWinin ja ViestiWinin pienet logiikkavikoja: "Ei badgea" ilman vakanttia, hidas sarjan selaus, väärin päin annetut argumentit tuloksen tyhjennyksessä ja muita (kokoomarivi) |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-137](../vikakortit/H/H-137.md) | avoin | P3 | Toiselta asemalta välitetty käsitelty kortti voi saada seuraavan paikallisen kortin jäämään pois automaattikäsittelystä |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-146](../vikakortit/H/H-146.md) | avoin | P3 | Jos aikajonon (AJAT) rivin kirjoitus levylle epäonnistuu, rivi merkitään silti kirjoitetuksi ja levylle jää tyhjä aukko |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-140](../vikakortit/H/H-140.md) | avoin | P3 | Poistetun kilpailijan emit-linkki voi periytyä uudelle kilpailijalle, joka näyttää jo luetulta |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-135](../vikakortit/H/H-135.md) | avoin | P4 | Kilpailijan numeron tai tilan muutos voi jättää leimakoodin väärälle numerolle (HK) |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-134](../vikakortit/H/H-134.md) | avoin | P3 | Jos uuden vaiheen rata-XML puuttuu tai on virheellinen, vaiheen vaihto poistaa emit-tarkastuksen käytöstä tai käyttää vapautettua ratamuistia |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-105](../vikakortit/H/H-105.md) | avoin | P3 | Vaiheen vaihdon jälkeen emit-kortit voivat kirjoittua suljettuun tai väärään tiedostoon |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-141](../vikakortit/H/H-141.md) | avoin | P4 | Vaiheen vaihto säilyttää edellisen vaiheen erälähdön ja kaksoiskorttien eston laskurin |
| Emit-kortin käsittely ja kilpailijan haku (pros_emit, Emit-lomake) | [H-090](../vikakortit/H/H-090.md) | avoin | P2 | HK: kellosanoman käsittely jättää aikajonon lukon päälle asetuksella TUNNJONO, ja käsin kirjattavat ajat ja vaiheen vaihto estyvät |
| Leimantarkastus ja hylkäysesitys (tarkista, tark_badge, tarkastusmerkinnät) | [H-058](../vikakortit/H/H-058.md) | avoin | P4 | Käsin tehty rata-tiedosto (RADAT.LST) voi ylittää ratataulukon; viestissä myös yli 50 rastin rata kirjoittaa rajan yli |
| Leimantarkastus ja hylkäysesitys (tarkista, tark_badge, tarkastusmerkinnät) | [H-134](../vikakortit/H/H-134.md) | avoin | P3 | Jos uuden vaiheen rata-XML puuttuu tai on virheellinen, vaiheen vaihto poistaa emit-tarkastuksen käytöstä tai käyttää vapautettua ratamuistia |
| Leimantarkastus ja hylkäysesitys (tarkista, tark_badge, tarkastusmerkinnät) | [H-135](../vikakortit/H/H-135.md) | avoin | P4 | Kilpailijan numeron tai tilan muutos voi jättää leimakoodin väärälle numerolle (HK) |
| Leimantarkastus ja hylkäysesitys (tarkista, tark_badge, tarkastusmerkinnät) | [H-140](../vikakortit/H/H-140.md) | avoin | P3 | Poistetun kilpailijan emit-linkki voi periytyä uudelle kilpailijalle, joka näyttää jo luetulta |
| Leimantarkastus ja hylkäysesitys (tarkista, tark_badge, tarkastusmerkinnät) | [H-116](../vikakortit/H/H-116.md) | avoin | P2 | Valinta- tai virheikkunan sulkeminen X-painikkeella tulkitaan painikkeeksi 2: emit-lomakkeissa tuhoava valinta, virheikkunassa ohjelman lopetus |
| Leimantarkastus ja hylkäysesitys (tarkista, tark_badge, tarkastusmerkinnät) | [H-011](../vikakortit/H/H-011.md) | avoin | P3 | Kilpailijan emit-koodin tai numeron muutos kirjoittaa emit-tiedostoon ilman emit-lukkoa, kun lukija voi kirjoittaa samaan tiedostoon (HK) |
| Väliajat leimoista (laskeEm_VA, rastivälianalyysi, EMITVA) | [H-010](../vikakortit/H/H-010.md) | avoin | P3 | Noin 35 kohtaa tallentaa kilpailijan tiedot ilman lukkoa, ja tallennus muuttaa rakenteita, joilla ei ole omaa lukkoa (HK) |
| Väliajat leimoista (laskeEm_VA, rastivälianalyysi, EMITVA) | [H-073](../vikakortit/H/H-073.md) | avoin | P2 | Emit-välilokin rivi ylittää muistialueen, kun EMVALOKI on päällä ja saapuvassa emit-paketissa on paljon rasteja |
| Väliajat leimoista (laskeEm_VA, rastivälianalyysi, EMITVA) | [H-045](../vikakortit/H/H-045.md) | avoin | P3 | Emit-analyysin kytkeminen päälle kesken ajon lukee emit-tiedoston uudelleen ilman emit-lukkoa (HkKisaWin) |
| Väliajat leimoista (laskeEm_VA, rastivälianalyysi, EMITVA) | [H-092](../vikakortit/H/H-092.md) | avoin | P3 | Emit-tiedon tallennus ja väliaikaraportin tulostus voivat jäädä odottamaan toisiaan, kun raportit tulostetaan automaattisesti |
| Väliajat leimoista (laskeEm_VA, rastivälianalyysi, EMITVA) | [H-103](../vikakortit/H/H-103.md) | avoin | P3 | ViestiWinin emit-tietojen uudelleenanalyysi ja saapuvan emit-kortin käsittely voivat jäädä odottamaan toisiaan |
| Leimausraportit ja tulosteet (emit_rap, tark_rap, rastiväliaikatuloste) | [H-092](../vikakortit/H/H-092.md) | avoin | P3 | Emit-tiedon tallennus ja väliaikaraportin tulostus voivat jäädä odottamaan toisiaan, kun raportit tulostetaan automaattisesti |
| Leimausraportit ja tulosteet (emit_rap, tark_rap, rastiväliaikatuloste) | [H-103](../vikakortit/H/H-103.md) | avoin | P3 | ViestiWinin emit-tietojen uudelleenanalyysi ja saapuvan emit-kortin käsittely voivat jäädä odottamaan toisiaan |
| Leimausraportit ja tulosteet (emit_rap, tark_rap, rastiväliaikatuloste) | [H-087](../vikakortit/H/H-087.md) | avoin | P2 | ViestiWin: emit-tietojen HTML-tulostus tiedostoon jättää tulostuslukot päälle, ja ohjelma jäätyy seuraavan emit-tiedon saapuessa |
| Leimausraportit ja tulosteet (emit_rap, tark_rap, rastiväliaikatuloste) | [H-064](../vikakortit/H/H-064.md) | avoin | P3 | Viestin konsoliohjelma kaatuu kortin luennassa, kun partiokilpailussa on automaattinen raportti mutta ei listakirjoitinta |
| Leimausraportit ja tulosteet (emit_rap, tark_rap, rastiväliaikatuloste) | [H-091](../vikakortit/H/H-091.md) | avoin | P2 | Jos HkKisaWinissä on päällä jatkuva sivutuloste, tallennus ja tulostus voivat jäädä odottamaan toisiaan |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-044](../vikakortit/H/H-044.md) | avoin | P3 | Jos lähetysjonon levytiedoston COMFILE.DAT kirjoitus tai luku epäonnistuu, jono etenee silti: paketti jää lähettämättä tai edellinen paketti lähtee uudelleen |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-021](../vikakortit/H/H-021.md) | avoin | P3 | Kun lähetysjono pidetään muistissa (COMFILE=EI), täysi jono pudottaa sanomia, ja jonon täyttymisestä varoittava ilmoitus voi jäädä odottamaan käyttäjää lukkojen ollessa varattuina |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-038](../vikakortit/H/H-038.md) | avoin | P3 | Ohjelman lopetus ei odota taustasäikeitä: KILP.DAT voidaan sulkea kirjoittajan alta, ja COMFILE-kysymykseen vastaus Ei hävittää lähettämättömät tiedot |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-030](../vikakortit/H/H-030.md) | avoin | P3 | Tiedonsiirron uudelleenlähetys lähettää jonosta vanhoja, jo kuitattuja tilannekuvia ja voi kumota vastaanottajan uudemmat muutokset |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-027](../vikakortit/H/H-027.md) | avoin | P2 | TCP-asiakasyhteys ei yritä uudelleen ensimmäisen epäonnistumisen jälkeen, ja TCPXML-uudelleenyhteys voi kaataa ohjelman |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-028](../vikakortit/H/H-028.md) | avoin | P2 | TCP-palvelinporttiin tuleva toinen yhteys korvaa elävän yhteyden sulkematta sitä ja käynnistää toisen lukusäikeen samaan puskuriin |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-029](../vikakortit/H/H-029.md) | avoin | P2 | Mikä tahansa UDP-paketti voi ohjata aseman lähetykset toiseen osoitteeseen, myös silloin kun osoite on asetettu käsin |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-032](../vikakortit/H/H-032.md) | avoin | P1 | UDP-yhteydellä saapuva liian pitkä paketti voi kirjoittaa vastaanottojonon muistialueen yli |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-034](../vikakortit/H/H-034.md) | avoin | P4 | Yhteyden palautuessa tai ohjelman käynnistyessä uudelleen sama sanoma tai emit-kortti voi tulla käsitellyksi kahdesti |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-073](../vikakortit/H/H-073.md) | avoin | P2 | Emit-välilokin rivi ylittää muistialueen, kun EMVALOKI on päällä ja saapuvassa emit-paketissa on paljon rasteja |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-025](../vikakortit/H/H-025.md) | avoin | P2 | Viestiohjelmien lähetysjonon paikka on 14 tavua liian lyhyt, joten suurimman sanomatyypin loppuosa ei tallennu jonoon ja voi lähteä väärällä sisällöllä |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-007](../vikakortit/H/H-007.md) | avoin | P2 | Toiselta asemalta tullut päivitys voi kumota samaan aikaan paikallisesti tehdyn muutoksen ja levittää vanhan tiedon muille asemille (HK) |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-022](../vikakortit/H/H-022.md) | avoin | P2 | Kahden aseman samanaikaiset lisäykset voivat saada saman tietuenumeron, ja vaihetieto tallentuu väärälle kilpailijalle ilman numeron tarkistusta (HK) |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-023](../vikakortit/H/H-023.md) | avoin | P2 | Monivaiheisessa kilpailussa tulos voi tallentua vastaanottajan omaan vaiheeseen, ja yhteys hyväksytään, vaikka vaiheen vaihto evätään (HK) |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-033](../vikakortit/H/H-033.md) | avoin | P1 | Toiselta työasemalta tullut virheellinen vaiheen numero voi kirjoittaa HK-ohjelman muistiin väärään paikkaan |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-026](../vikakortit/H/H-026.md) | avoin | P2 | Viestiohjelma lakkaa käsittelemästä saapuvia tietoja, kun se saa paketin, jonka lajia se ei tunne |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-016](../vikakortit/H/H-016.md) | avoin | P3 | Yhteyskatkon aikana kahdella asemalla tehdyt muutokset samaan tietoon voivat vaihtua ristiin, eikä ohjelma huomaa sitä |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-039](../vikakortit/H/H-039.md) | avoin | P3 | Jos asemat on kytketty silmukaksi, vanha tieto voi saapua toista reittiä ja korvata uudemman |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-110](../vikakortit/H/H-110.md) | avoin | P2 | ViestiWin: Lähtöajat-painike jättää kilpailijatietojen lukon päälle, ja vastaanotto- ja lukijasäikeet pysähtyvät ohjelman loppuun asti |
| Emit-tietueen ja tuloksen välitys ja vastaanotto (EMITT, KILPT, jonot) | [H-162](../vikakortit/H/H-162.md) | avoin | P3 | SQL-vienti voi virheeseen pysähtyessään jättää tallennuslukon päälle, jolloin tietoja vastaanottavat säikeet jäävät odottamaan |
| Kilpailijan tuloksen tallennus Emit-käsittelyssä (tallenna, KILP.DAT, SQL-peili) | [H-015](../vikakortit/H/H-015.md) | avoin | P3 | Jos kilpailijatiedoston KILP.DAT kirjoitus levylle epäonnistuu, ohjelma jatkaa muistissa olevilla tiedoilla ja levylle jää vanha tieto |
| Kilpailijan tuloksen tallennus Emit-käsittelyssä (tallenna, KILP.DAT, SQL-peili) | [H-153](../vikakortit/H/H-153.md) | avoin | P3 | Jos virheilmoituksen antanut työsäie pitää lukkoa, ilmoituksen valinta "Poistu ohjelmasta" jättää ohjelman jumiin eikä sulje sitä |
| Kilpailijan tuloksen tallennus Emit-käsittelyssä (tallenna, KILP.DAT, SQL-peili) | [H-009](../vikakortit/H/H-009.md) | avoin | P3 | HkKisaWin voi jäädä odottamaan itseään, kun työsäie pyytää käyttöliittymältä jotain kilpailijatietojen lukko hallussaan |
| Kilpailijan tuloksen tallennus Emit-käsittelyssä (tallenna, KILP.DAT, SQL-peili) | [H-014](../vikakortit/H/H-014.md) | avoin | P3 | Vaiheen vaihdon jälkeen SQL-peili lakkaa päivittymästä, eikä ohjelma ilmoita siitä |
| Kilpailijan tuloksen tallennus Emit-käsittelyssä (tallenna, KILP.DAT, SQL-peili) | [H-160](../vikakortit/H/H-160.md) | avoin | P2 | Yksikin tietokantavirhe SQL-peilin kirjoituksessa lopettaa koko ohjelman |

Generoitu tiedostosta `kasikirja/toiminnot.csv` ja rekistereistä (tila, prioriteetti); älä muokkaa käsin.
<!-- generoitu:viat loppu -->

## 6. Mihin tieto perustuu

- Luenta, tallennus, välitys, vastaanotto, virhe- ja palautuspolut: `aineisto/emit-luenta.md`
  (riippumattomasti tarkastettu 2026-11-27, `doc/tietopohja/laatu/tarkastukset/emit/tarkastus.md`)
  ja sen lähteet: `doc/arkkitehtuuri/saikeet-ja-lukot.md`, `virheenkasittely.md`,
  `tiedonsiirto.md`, `tiedostomuodot.md`, `tallenna-samanaikaisuus.md`,
  `todisteet/c10/raportit/osaD-laitteet.md`.
- Käyttäjän näkökulma leimantarkastukseen ja hylkäykseen: `docs-site/docs/hkkisawin/6.1`,
  `6.8`, `6.14`, `6.16`, `suunnistuskilpailu/leimantarkastus.md`, `hkkisawin/hkliite1.md`
  (asetusten nimet vain).
- Koodi [luettu käsikirjaa varten, ei tarkastettu]: `Hk/HkEmit.cpp:1034-1189`, `:1399-1446`,
  `:1957-2310`, `:2652-2794`, `:3618-3770`, `:6112-6240`; `Hk/HkIV.cpp:1625-1868` (kohdat
  1.3, 1.7-1.8, 1.10); `Hk/HkTls.cpp:1208-1211`, `:1500-1545`; `cbHk/UnitEmit.cpp:2655-2700`, `:3378-3380`;
  `Juk/VEmit.cpp:1298`, `:2004`, `:2659-2740`, `:3126`, `:6178`; `Juk/VIv.cpp:1640-1700`, `:1733`;
  `Hk/HkInit.cpp` (asetusavaimet rivit kohdassa 3).
- Rekisteri: `doc/arkkitehtuuri/rekisteri/havainnot.md`, `riskiarvio.csv` (ohjelmat ja
  saavutettavuus), `valitsimet.csv`, `asetusavaimet.csv`.
- Toimintokytkentä: `kasikirja/toiminnot.csv` (rivit "Leimantarkastus ja hylkäysesitys",
  "Väliajat leimoista" ja aineiston rivit).

## 7. Mitä ei ole selvitetty

- `tarkista`: tarkka järjestys- ja vapaa-järjestyssääntö rajatapauksissa (rasti kahdesti radalla,
  rastikoodin 1 ja 2 `kuvio`-merkinnät, osittain luettu). Ei ajettu testitapauksilla. Mitään
  "hyväksytty/hylätty"-päätelmää tietylle kortille ei voi tämän luvun perusteella tehdä.
- Onko leimantarkastuksessa aikarajoja tai rastivälien vähimmäisaikoja: luettu koodi ei sisällä
  sellaisia, mutta koko `tarkista`-ketjua ja `kuvio`-tiedoston lukua ei ole tarkastettu.
- `vaihda_badge` GUI:n "luettu jo" -polulla (aineiston Q-K3-5).
- Hylkäysesityksen käsittelylomakkeen tallennus (XML-tiedoston rakenne, 17 merkin kentät).
- `tark_rap`, `tarkraportit`, `emitlista`, `tulostaRastiVa`: tulosteiden sisältö.
- Aineiston avoimet kysymykset ja ehdokkaat KE-10-KE-13 (aineisto luku 12) koskevat tätä lukua
  sellaisenaan: erityisesti KE-10 (käsittelyvaiheen emit-kirjoitusvirhe jättää vertaiset ilman
  emit-tietuetta), KE-11 (uudelleenluku ohittaa käsittelemättömät kortit) ja KE-12 (tyhjä
  "Virhe luennassa" -ikkuna). Q86, Q100, OP-13: lukijan kuittauksen merkitys laitteessa.
- Viestin osuuden valinnan yksityiskohdat (`val_korkein`, joustoviesti, partio) on vain mainittu;
  `tark_partio`n tarkastus on lukematta.
- Kuntosuunnistustilojen (`kuntosuunnmoodi` 1-3) käyttö ja `kaksibadge`-käyttötapaus: Q75, OP-9.
- EMITVA:n vastaanottajan käyttötapaus (mikä ohjelma sitä käyttää): ei aineistossa.
