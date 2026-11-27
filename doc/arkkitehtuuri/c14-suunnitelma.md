# C-14: koko tuotantolähdekoodin järjestelmällinen katselmointi – lähtötaso ja toteutussuunnitelma / Systematic review of all production source code – baseline and implementation plan

**Tila 2026-11-27:** lähtötason selvitys tehty, toteutussuunnitelma **odottaa käyttäjän
hyväksyntää**. Varsinaista katselmointia ei ole aloitettu. Tuotantokoodia, projektitiedostoja, CI:tä,
riskiarviota ja rekistereitä ei ole muutettu. Mitään ei ole käännetty eikä ajettu, lukuun ottamatta
auditoinnin omaa lukevaa mittaustyökalua `tyokalut/kattavuus.py`.

**State 2026-11-27:** baseline measured, plan **awaiting the user's approval**. The review itself has
not started. No production code, project file, CI, risk assessment or register was changed; nothing was
built or run except the audit's own read-only measurement script.

Lähtötason aineisto: `todisteet/c14/lahtotaso/` (toisto ja rajoitteet sen README:ssä).

---

## 1. Tiivistelmä / Summary

| Kysymys / Question | Vastaus / Answer |
|---|---|
| Miksi C-vaihe ei kattanut koko koodia? | C oli aihe- ja riskilähtöinen: 13 erää, kukin oman teemansa (tallennus, tiedonsiirto, tiedostot, varoitukset, muisti, säikeet, GUI-logiikka, tila, virheet, riippuvuudet, testaus, valitsimet, synteesi). Koodia luettiin väitteiden ja löydösten perässä, kattavuutta seurattiin vain tiedostotasolla neljällä karkealla tasolla, eikä mikään erä tavoitellut tiedoston tai funktion läpikäyntiä. GUI-lomakkeet jäivät vähälle (vain bcc32, ei kääntäjän analyysiä) ja osa rajattiin pois nimeltä (luku 2.9). |
| Kuinka paljon aiempaa voi hyödyntää? | **Havainnot ja ominaisuuskohtaiset inventaariot kokonaan** (229 riviä, säie- ja lukkokartta, valitsimet, asetusavaimet, riippuvuudet, tiedostomuodot, pysyvien tiedostojen virheenkäsittely ym., luku 2.6). **Funktiotasolla yhtäkään funktiota ei voi merkitä katselmoiduksi**, koska koko funktion läpikäynnistä ei ole kirjausta. Noin 290 funktiota (21 000 riviä) on luettu useasta näkökulmasta lähes kokonaan; ne voidaan katselmoida kevyemmin täydentäen. |
| Paljonko on katselmoimatta? | Aktiivista tuotantokoodia on 181 361 funktioriviä 6 381 funktiossa sekä noin 32 700 riviä funktioiden ulkopuolista koodia (määrittelyt, taulukot, lomakeluokat). **Ilman yhtäkään viitettä on 52 % funktioriveistä** (94 686 riviä, 5 113 funktiota). Vain noin 26 % funktioriveistä on viitatun rivin lähellä (±10 riviä). Täydestä katselmoinnista ei ole kirjausta millekään riville. |
| Merkittävimmät aukot | Tulosten muotoilu ja tulostus (`HkMuotoilu`, `VMuotoilu` ~90 % viittaamatta; `VTulostus` 57 %), XML/IOF-vienti (53–69 %), GUI-lomakkeet (HK 61 %, viesti 72 %, yhteiset 77 %), kilpailija- ja joukkuelomakkeet (80–86 %), tulosten ja pisteiden laskenta (`HkTls`: 37 % ilman viitteitä ja 36 % vain johdannaisissa; `UnitPistelasku`; arvonta `HkArv2`), tietokannan avainfunktiot (`dbboxm`: `addkey`, `delkey`, `nextkey`, `prevkey` 0 %), ratojen muokkaus (`UnitRadat` 97 %) ja konsoliohjelmien käyttöliittymä (~50 %). Luku 2.8. |
| Menetelmä | Funktio- ja määrittelyaluetason kattavuusrekisteri, joka tuotetaan ohjelmallisesti; katselmointi työpaketeittain tarkistuslistalla (18 näkökulmaa, myös liiketoimintalogiikka), HK- ja viestiversiot rinnakkain, kaikki käännöshaarat luokiteltuina, jokainen rajaus kirjattuna, vakavat havainnot ja otos "ei havaintoja" -yksiköistä riippumattomasti tarkastettuina. Luvut 3–6. |
| Työmäärä | Noin 80 katselmointiyksikköä (14 työpakettia) ja 25–35 riippumatonta tarkastuserää. Karkea arvio: 35–45 miljoonaa tokenia ja 30–45 tuntia agenttien työaikaa, eli 10–15 istuntoa CHECKPOINT-jatkoin. Lisäksi tulee uusien havaintojen jatkokäsittely (riskiarvio, kortit). Pilotti kalibroi arvion (luku 6.7). |
| Rajoitteet | Staattinen luku ilman ajoja. Täysi kattavuus ei tarkoita virheettömyyttä. Agenttikatselmoinnin ohitusosuus mitataan otoksella, ei poisteta. Liiketoimintasääntöjen vaatimuslähteenä on vain käyttöohje: lajisääntöjä ei ole repossa. Laitteita, verkkoa ja bcc32-GUI:n ajonaikaista käytöstä ei todenneta. Luku 9. |
| Vaikutus E- ja D-vaiheisiin | E-2 (haarojen täsmäytys) kannattaa tehdä ennen C-14:n toteutusta, koska se määrää katselmoitavan revision. E-1 (62 rivin riskiseulonta) kannattaa yhdistää C-14:ään, koska samat funktiot luetaan kuitenkin. KE-10–KE-24 käsitellään niiden koodin katselmointiyksiköissä. D alkaa C-14:n jälkeen; vaihtoehto aluekohtaisesta aloituksesta on omistajan päätös (luku 7.3). |

In short: phase C was topic- and risk-driven; findings and property inventories are fully reusable, but
no function can be counted as reviewed. 52 % of active function lines have no reference at all and only
about 26 % lie near a cited line. The plan adds a generated function-level coverage register, a
checklist-based review in 14 work packages, recorded scope exclusions, and independent review including a
sample of "no findings" units. Estimated about 80 units, 35–45 M tokens, 10–15 sessions.

---

## 2. Nykyinen kattavuus / Current coverage

### 2.1 Menetelmä / Method

Työkalu `tyokalut/kattavuus.py` (vain luku):

1. **Funktiot.** Jokaisen rekisteröidyn `.c`/`.cpp`/`.h`-tiedoston funktiot tunnistetaan
   aaltosulkeiden perusteella. Kommentit ja literaalit tyhjennetään ensin, ja esikääntäjän haarat
   käsitellään kuten ctags: kukin haara alkaa `#if`-kohdan syvyydestä. Jos haarojen sulkeet eroavat,
   valitaan tasapainoinen haara. Tällaisia kohtia oli 3; esimerkiksi `Tp/TpComY.cpp`:n `#if !DEBUG1`
   -haara on kuollut ja epätasapainoinen.
2. **Tuotteet ja aktiivisuus.** Tiedoston tuotteet saadaan projekteista, kirjastoista
   (`tputilv2`/`dbboxm` → konsoliohjelmat, `Tputil-XE`/`DBboxm-XE` → GUI) ja `#include`-sulkeumasta.
   Funktio on tuotteessa voimassa, ellei se ole kokonaan haarassa, jota C-12 ei havainnut voimassa
   olevaksi (`todisteet/c12/osaA0/testikohdat.csv`, 2 491 ehtokohtaa tuotteittain).
3. **Viitteet.** Kaikki `polku:rivi`-, `polku:a-b`- ja jatkomuotoiset `` `:N` `` -viitteet
   kerätään analyysiasiakirjoista: `doc/arkkitehtuuri/*.md`, `rekisteri/havainnot.md`,
   `todisteet/**/*.md`, `doc/tietopohja/**/*.md` sekä C-4:n varoitusluokitukset erillisenä lajina.
   Viitteitä on 11 786. Raakatulosteita (lokit, json, generoidut csv:t) ei käytetä. Yli 150 rivin
   väli on aluemaininta (166 kpl), eikä se merkitse funktioita.
4. **Näkökulma.** Viitteen näkökulma on sen lähteen erä tai aihe (C-1 … C-13). Johdannaisia ovat
   synteesi (C-13), rekisteri ja K-vaiheen tietopohja: niistä tuleva viite ei ole oma
   virhenäkökulmansa.
5. **Tarkkuus.** Karkea vertailuhaku löysi 6 220 funktion alkua, joista tunnistimelta puuttui 35;
   kaikki tarkistetut olivat kommentiksi jätettyä koodia. C-9:n riippumattoman jäsentimen 977
   funktiosta 968 löytyi; loput 9 johtuivat sen jäsentimen tulkinnoista. 108 viitettä jäi
   yksilöimättä, koska vain perusnimi oli annettu ja sama nimi on kahdella tiedostolla
   (esim. `UnitEmit.cpp`).

Tuotantolähdekoodi on tavu tavulta sama revisioissa `f7fe553` ja HEAD (välillä on muuttunut vain
`Tests/` ja `VS/Tests/`), joten kaikkien erien viitteet osuvat nykyiseen koodiin.

**Tärkeä rajaus tulkinnalle:** viite tarkoittaa, että kohtaa on katsottu jonkin väitteen vuoksi.
Se ei tarkoita funktion katselmointia. Siksi alla olevat luvut ovat **ylärajoja** sille, mitä on
edes osittain luettu. Katselmointitilaa niistä ei voi päätellä.

### 2.2 Tiedostot / Files

| Luokka / Class | Tiedostoja | Rivejä | Huomio |
|---|--:|--:|---|
| tuote (kuuluu vähintään yhteen viidestä ohjelmasta) | 568 | 252 342 | katselmoinnin kohde |
| orpo (ei projektissa eikä `#include`-sulkeumassa) | 37 | 8 183 | rajaus perustellaan (luku 5.6) |
| kolmas osapuoli (Blaise, Microsoft-esimerkit, MT19937; tunnistettu otsikosta) | 21 | 1 820 | tuotteissa; luku 5.7 |
| testi (`Tests/`, auditoinnin oma testikoodi ja doctest) | 24 | 10 834 | rajattu (luku 5.9) |

Vanha tiedostotaso (`rekisteri/tiedostot.csv`) ja viitteet tuotetiedostoissa:

| Vanha taso | Viitteitä on | Ei viitteitä |
|---|--:|--:|
| 1 inventoitu | 89 | 330 |
| 2 kohdennettu | 112 | 15 |
| 3 rakenne | 21 | 1 |
| 4 katselmoitu | 0 | 0 |

Tasolla 1 olevista 89 tiedostosta on viitteitä, joita tiedostorekisteri ei tunne. Tason 2
merkintä taas ei kerro, kuinka suuri osa tiedostosta luettiin.

### 2.3 Funktiotaso ryhmittäin / By function, per group

Aktiiviset funktiot (voimassa vähintään yhdessä tuotteessa). *ei* = ei yhtään viitettä; *joht.* =
viitteitä vain synteesistä, rekisteristä tai tietopohjasta; *yksi* = yksi virhenäkökulma; *usea* =
kaksi tai useampia; *±10* = funktiorivien osuus, joka on enintään 10 rivin päässä viitatusta rivistä.

| Ryhmä / Group | Funktioita | Rivejä | ei | joht. | yksi | usea | ±10 |
|---|--:|--:|--:|--:|--:|--:|--:|
| ydin HK (`Hk/`) | 724 | 36 146 | 11 316 | 2 524 | 7 491 | 14 815 | 44 % |
| ydin viesti (`Juk/`) | 698 | 29 382 | 12 398 | 2 337 | 6 060 | 8 587 | 34 % |
| yhteinen ydin (`Tp/`, `com/`) | 234 | 7 667 | 2 339 | 131 | 730 | 4 467 | 59 % |
| tietokanta (`dbboxm/`) | 42 | 740 | 459 | 12 | 97 | 172 | 35 % |
| konsoli-UI (`HkConsole/`, `JukConsole/`) | 156 | 14 149 | 5 976 | 790 | 4 372 | 3 011 | 15 % |
| GUI HK (`cbHk/`) | 2 185 | 47 431 | 28 827 | 1 755 | 10 238 | 6 611 | 17 % |
| GUI viesti (`ViestiWin/`) | 1 439 | 29 662 | 21 331 | 792 | 4 703 | 2 836 | 14 % |
| GUI yhteinen (`cbTp/`, `RADStudio10/`) | 556 | 8 784 | 6 807 | 91 | 1 273 | 613 | 15 % |
| apukirjastot (`tputilv2/`, `include/`) | 347 | 7 400 | 5 233 | 34 | 1 309 | 824 | 22 % |
| **yhteensä** | **6 381** | **181 361** | **94 686 (52 %)** | **8 466 (5 %)** | **36 273 (20 %)** | **41 936 (23 %)** | **26 %** |

Kokonaan passiivisia funktioita (missään tuotteessa ei voimassa) on 75 (3 721 riviä). 22 funktion
voimassaolo riippuu ehdosta, jota C-12:n arvioija ei ratkaissut (esim. `MAKI`, `HIIHTO`).

Viitteellisten 1 268 funktion viitealueen osuus funktiosta: ≥ 80 % 867 funktiolla, 50–80 % 154:llä
ja alle 50 % 247:llä. Suurten funktioiden lukeminen on siis ollut kohdennettua. Esimerkiksi
`tall_regnly` (1 091 riviä) ja `lue_parametrit` (HK 2 222 riviä, viesti 1 556 riviä) on luettu
kohdittain.

### 2.4 Tuotteittain / By product

| Ohjelma | Aktiivisia funktioita | Rivejä | ilman viitteitä | yksi näkökulma |
|---|--:|--:|--:|--:|
| HkMaali | 1 398 | 57 811 | 38 % | 19 % |
| ViestiMaali | 1 359 | 49 895 | 45 % | 20 % |
| ViestiLuenta | 1 122 | 39 305 | 47 % | 16 % |
| HkKisaWin | 4 035 | 104 785 | 51 % | 20 % |
| ViestiWin | 3 218 | 78 965 | 58 % | 17 % |

Funktio lasketaan jokaiseen tuotteeseen, jossa se on voimassa. Yhteinen koodi esiintyy siis
useassa rivissä.

### 2.5 Vain yhden näkökulman alueet / Areas seen from one perspective only

Näissä 36 273 rivissä (20 %) on viitteitä vain yhdestä erästä. Ne on siis luettu vain yhden
virhetyypin kannalta. Erittäin yleinen tapaus on asetusjäsennin, joka on luettu C-12:ssa vain
avaimen pituuden ja ehtojen kannalta.

| Ainoa näkökulma | Rivejä |
|---|--:|
| C-8 tila ja elinkaari | 9 078 |
| C-12 valitsimet ja asetukset | 8 629 |
| C-9 virheenkäsittely | 4 362 |
| C-7 GUI-logiikka | 3 124 |
| C-10 riippuvuudet ja tietoturva | 2 999 |
| C-5 muistiturvallisuus | 2 795 |
| C-6 rinnakkaisuus | 2 012 |
| C-11 testaus | 949 |
| C-5 kääntäjäerot | 918 |
| C-3 tiedostot ja eheys | 599 |
| C-1 tallennus | 551 |
| C-2 tiedonsiirto | 211 |
| C-0 merkistöt | 46 |

### 2.6 Hyödynnettävissä sellaisenaan: ominaisuuskohtaisesti kattavat tarkastukset / Reusable as is: property-complete inventories

Nämä tarkastukset kattavat **yhden ominaisuuden koko koodista**. Ne eivät katselmoi funktioita,
mutta ne poistavat kyseisen tarkistuksen tarpeen tai antavat sille valmiin luettelon.

| Tarkastus | Kattaa | Ei kata | Käyttö C-14:ssä |
|---|---|---|---|
| C-6 säiekartta (`saikeet-ja-lukot.md` luku 11) | kaikki 67 tuotannon säikeen käynnistyskohtaa | säikeiden sisällön | säikeen tunnistus valmiina; rinnakkaisuusnäkökulma luetaan säikeittäin |
| C-6 lukkokartta | kaikki 22 lukon nimeä, alustus ja poisto | reunalista vain yhden kutsutason syvyydeltä (alaraja, Q74); `tall`-lukon 263 kohtaa vain C-1:n perusteella | lukitusjärjestys tarkistetaan yksiköissä listaa vasten |
| C-12 valitsimet (`ominaisuusvalitsimet.md`) | 435 makroa, 2 491 ehtokohtaa, voimassaolo tuotteittain ja kokoonpanoittain | ehtojen sisällä olevan koodin | haarojen aktiivisuus ja kuollut koodi suoraan (luku 5) |
| C-12 asetusavaimet (`rekisteri/asetusavaimet.csv`) | kaikkien jäsentimien ja editorien avaimet, vertailupituudet | avaimen merkityksen ja vaikutuksen koodissa | asetusnäkökulman lähtöluettelo |
| C-10 riippuvuudet | kaikki ulkoiset riippuvuudet, kirjastot ja tiedostonimiliteraalit | käyttökohtien oikeellisuuden | tietoturva- ja resurssinäkökulman luettelo |
| C-9 osa A (`virheenkasittely.md` luku 10) | pysyvien tiedostojen kirjoitusvirheiden käsittely ja toistuvien toimintojen vuodot 11 hakemistossa | GUI-valikkojen lomakkeet, matalat kohdat yksitellen | virheenkäsittelynäkökulman lähtöluettelo |
| C-8 globaalit | 217 globaalia (`Hk/`, `Juk/`) luokiteltu | 153 funktiostaattista muuttujaa muualla | elinkaarinäkökulma; jäljellä olevat 153 kuuluvat W13:een |
| C-3 tiedostomuodot | tietuemuodot ja mitatut koot (MSVC, bcc32) | tiedostojen käsittelylogiikan | tietomalli- ja eheysnäkökulman pohja |
| C-2 tiedonsiirto | pakettiluokat, kehys ja kuittaus | kaikkien vastaanottohaarojen sisällön | W08:n lähtökohta |
| C-4 varoitukset | kaikki 7 030 yksilöllistä varoitusta | noin 93 % luokittelematta | yksikkö käy läpi omien funktioidensa varoitukset |
| C-5 kääntäjäerot ja UTF-8-literaalit | luetellut erot, kaikki literaalit | GUI-koodi vain bcc32:lla (ei kääntäjän analyysiä) | kääntäjäeronäkökulma |
| Rekisteri (229 havaintoa) | löydökset ja niiden riski | – | jokainen yksikkö tarkistaa funktioidensa havainnot; havaintoa ei löydetä uudelleen |

Lisäksi kevyeen, täydentävään katselmointiin kelpaa noin 292 funktiota (20 748 riviä), jotka on
luettu useasta näkökulmasta vähintään 80-prosenttisesti viitealueen mukaan. Niidenkin osalta
tarkistuslista käydään läpi, mutta jo kirjatut näkökulmat voidaan kuitata viittauksella.

### 2.7 Mikä aiemmasta ei riitä / What is not sufficient

- **Vanhan tason 3 tiedostot (22 kpl):** rakenne ja avainfunktiot on luettu, kaikkia funktioita
  ei. Taso 3 ei siis vastaa katselmointia.
- **Kohdennettu luku (taso 2):** se on rajattu väitteen todentamiseen. Viereinen koodi on usein
  luettu vain siltä osin kuin väite vaati.
- **Yhden näkökulman alueet (luku 2.5):** liiketoimintalogiikkaa ei arvioitu, kun luettiin
  esimerkiksi vain lukitusta tai asetusavaimen pituutta.
- **K-vaiheen tietopohjan lukemiset:** ne ovat kortin tai luvun kirjoittajan tarkistuksia, eivät
  katselmointia. K-5:n riippumaton otos koski väitteitä, ei funktioita.
- **Riippumattoman tarkastuksen tila** on kirjattu väitteittäin, ei funktioittain. Funktiotasolla
  se on **tuntematon**.

### 2.8 Merkittävimmät kattavuusaukot / Largest coverage gaps

Viittaamattomien aktiivisten funktiorivien mukaan, liiketoiminta- ja eheysriski huomioiden:

| Alue | Tiedostot (esimerkit) | Viittaamatta | Miksi merkittävä |
|---|---|--:|---|
| Tulosten muotoilu ja tulostus | `Hk/HkMuotoilu.cpp` 92 %, `Juk/VMuotoilu.cpp` 89 %, `Juk/VTulostus.cpp` 57 %, `Hk/HkTulostus.cpp` 33 % | ~10 000 riviä | julkaistavat tulokset: hiljaiset tulosvirheet |
| XML- ja IOF-vienti | `Hk/HkXml*.cpp`, `Juk/VXml*.cpp` 53–69 % | ~3 900 | tulospalveluiden ja liittojen syötteet |
| Tulos- ja pistelaskenta | `Hk/HkTls.cpp` (37 % + 36 % vain johdannaisissa), `cbHk/UnitPistelasku.cpp` (20/28 funktiota), `Juk/vkilp.cpp` 54 % | ~2 700 (ilman viitteitä tai vain johdannaisissa) | sijoitukset, pisteet, yhteistulokset |
| Arvonta | `cbHk/HkArv2.cpp` (C-7 rajasi pois; 9/20 funktiota ilman viitteitä) | ~480 | lähtöjärjestyksen oikeellisuus |
| Tietokannan avaimet | `dbboxm/addkey`, `delkey`, `nextkey`, `prevkey` 0 % | ~370 | indeksien eheys |
| Leimantarkastus ja emit | `Hk/HkEmit.cpp` 39 %, `Juk/VEmit.cpp` 43 % | ~4 600 | hyväksyntä ja hylkäys |
| GUI-lomakkeet | `cbHk/` 61 %, `ViestiWin/` 72 %, `cbTp/` 77 % viittaamatta; kilpailija- ja joukkuelomakkeet 80–86 %, `cbTp/UnitRadat.cpp` 97 % | ~57 000 | käyttäjän tekemät tietomuutokset |
| Konsoliohjelmien käyttöliittymä | `HkConsole/HkCons2.cpp`, `HkCons3.cpp`, `JukConsole/vcons2.cpp` | ~4 200 | konsolin ajanotto- ja laskentapolut (`aikavert`, `maaliaika`, `laskenta`) |
| Laitelukijat | `Tp/ZebraReader.cpp` 81 % | ~640 | laitteelta tuleva syöte |
| Määrittelyt funktioiden ulkopuolella | 32 700 riviä, enimmäkseen lomakeluokkia; `Hk/HkDef.h`, `Juk/VDef.h` | – | tietuerakenteet, taulukoiden koot, alkuarvot |

### 2.9 Miksi aiempi auditointi ei kattanut koko koodia / Why the earlier audit did not cover everything

1. **Tavoite.** C-vaiheen tehtävä oli arvioida riskit ja tuottaa korjaussuunnitelma. Erät
   valittiin aiheittain ja riskin mukaan (`suunnitelma.md`), ei koodin kattavuuden mukaan.
2. **Lukutapa.** Koodia luettiin väitteestä tai löydöksestä lähtien, ja kutsuketjuja seurattiin
   vain tarvittaessa. Jokainen erä lisäsi viitteitä, mutta funktioiden läpikäyntiä ei kirjattu.
3. **Mittaus.** Kattavuutta seurattiin vain tiedostotasolla neljällä karkealla tasolla.
   Funktio- tai riviväliä ei seurattu, joten aukot eivät näkyneet.
4. **GUI.** Lomakekoodi käännetään vain bcc32:lla, josta puuttuvat muoto-, polku- ja
   rajatarkistukset. Lomakkeita pidettiin pääosin ohuina, ja C-7 käsitteli niistä kahdeksan
   tiedostoa.
5. **Nimetyt rajaukset ja keskeneräiset jonot:** C-7 J7-1 (arvonta), J7-2 (41 kirjoittavaa
   lomaketta) ja J7-3 (152 jäsennyskohtaa); C-8:n 153 staattista muuttujaa; C-4:n 93 %
   luokittelematta (`auditoinnin-yhteenveto.md` luku 3).
6. **Liiketoimintalogiikka.** Mikään erä ei ollut tulosten, sijoitusten, pisteiden ja muotoilun
   oikeellisuuserä. Niitä luettiin vain teknisen vian yhteydessä.

### 2.10 Tuntematon / Unknown

- Kuinka paljon viitatun rivin ympäriltä todella luettiin (±10 on oletus).
- Mitkä funktiot on tarkastettu riippumattomasti: kirjaus on väitekohtainen.
- Agenttien työkalulokeja ei ole tallennettu, joten luettuja mutta mainitsematta jääneitä kohtia ei
  tunneta. Todellinen luettu määrä voi siis olla viitteitä suurempi, mutta sitä ei voi todentaa.

---

## 3. Kattavuuden mittaaminen / Measuring coverage

### 3.1 Taso / Granularity

**Perusyksikkö on funktio**, jota täydentää **määrittelyalue**: yhtenäinen funktioiden
ulkopuolinen koodilohko, kuten struct, class, globaalit, taulukon alustus tai sanomakartta.
Perustelut:

- Funktio on luonteva katselmoinnin, havaintojen ja muutosten yksikkö. Sen voi tunnistaa
  ohjelmallisesti (tarkkuus luvussa 2.1), ja sen sisällöstä voi laskea tiivisteen.
- **Tiedosto ja luokka** ovat koosteita: tiedosto on katselmoitu, kun sen kaikki funktiot ja
  määrittelyalueet ovat tasolla K4. Luokka (C++Builder-lomake) kootaan nimen etuliitteestä.
- **Rivivälit** ovat vain sijainteja. Rivitason seuranta olisi raskas ja hauras ylläpitää, eikä se
  kerro katselmoinnin sisällöstä mitään lisää.
- **Koodilohko** suuren funktion sisällä (esim. `lue_parametrit` 2 222 riviä, `tall_regnly` 1 091
  riviä): funktio voidaan jakaa kirjauksessa nimettyihin osiin rivivälein. Rekisteri kokoaa
  funktion tilan sen osista.

### 3.2 Generoitu osa: `rekisteri/koodialueet.csv`

Tuotetaan komennolla `kattavuus.py alueet`. Käsin sitä ei muokata.

| Sarake | Sisältö |
|---|---|
| `alue` | pysyvä tunniste: `polku#nimi[#n]` (n = järjestysnumero samannimisille), määrittelyalueelle `polku#@rivi-alku` |
| `polku`, `nimi`, `alku`, `loppu`, `rivit`, `laji` | sijainti; `laji` = funktio / määrittely |
| `tiedostoluokka` | tuote / orpo / kolmas_osapuoli / testi / generoitu |
| `tuotteet`, `aktiivinen`, `ehto_tuntematon` | tuotteet ja voimassaolo (C-12:n haaratiedot), kokoonpanot, joissa voimassaolo eroaa toimitetusta |
| `kaantaja` | MSVC / bcc32 / molemmat (projekteista) |
| `tiiviste` | normalisoidun sisällön SHA-256 (kommentit ja tyhjät pois): muutosten tunnistus |
| `paketti`, `yksikko` | työpaketti ja katselmointiyksikkö (`c14-yksikot.csv`:n sääntöjen mukaan) |
| `pari`, `samankaltaisuus` | HK- tai viestivastine ja rivitason samankaltaisuus |
| `lahtotaso` | viitteet ja näkökulmat ennen C-14:ää (kuten `todisteet/c14/lahtotaso/funktiot.csv`) |
| `rajaus` | tyhjä tai rajauksen tunnus (`rajaukset.csv`) |

### 3.3 Kirjattava osa: `rekisteri/katselmoinnit.csv`

Pitkä muoto: yksi rivi on alueen yksi katselmointi. Katselmoija kirjoittaa sen yksikön
loppuraportin liitteeksi, ja pääistunto tuo sen työkalulla (`kattavuus.py tuo`), joka tarkistaa
rivit. Käsin ei ylläpidetä muuta.

| Sarake | Sisältö |
|---|---|
| `alue` | `koodialueet.csv`:n tunniste |
| `katselmointi` | yksikön tunnus (`C14-W04-03`) |
| `syvyys` | K1–K5 (luku 3.4) |
| `nakokulmat` | tarkistuslistan kohdat L1–L18, jotka käytiin läpi; ei-sovellettavat merkitään erikseen (`L11=ei_sovellu`) |
| `tiiviste` | alueen tiiviste katselmointihetkellä |
| `revisio` | lähdekoodin revisio |
| `tuotteet`, `kokoonpanot` | tuotteet ja kokoonpanot, joiden haarat käytiin läpi |
| `tulos` | `ei_havaintoja` / ehdokastunnukset / rekisterin tunnukset (H, T, KE) |
| `riippumaton` | `ei` / `otos` / tarkastuksen tunnus ja tulos |
| `aukot` | mitä jäi tekemättä ja miksi (esim. `ajo_tarvitaan`, `vaatimus_puuttuu`) |
| `malli` | katselmoijan malli (Sonnet / Opus) |

### 3.4 Syvyystasot / Depth levels

| Taso | Merkitys | Vastaa vanhaa tasoa |
|---|---|---|
| K0 | ei käsitelty | 0–1 |
| K1 | viitattu: lähtötaso, ei katselmointia (generoitu viitteistä) | 2 |
| K2 | kohdennettu: tietty väite tai näkökulma | 2 |
| K3 | rakenne: rooli ja kutsut tunnistettu, tarkistuslistaa ei käyty läpi | 3 |
| K4 | katselmoitu: koko alue luettu ja tarkistuslista käyty läpi kaikissa voimassa olevissa haaroissa | 4 |
| K5 | K4, ja havainnot tai "ei havaintoja" -tulos on tarkastettu riippumattomasti | 4 + tarkastus |

Funktion voimassa oleva taso on suurin taso niistä katselmoinneista, joiden tiiviste on sama kuin
nykyinen tiiviste. Jos tiiviste on muuttunut, alue on **vanhentunut**: taso putoaa K1:ksi, ja alue
nousee uudelleenkatselmointijonoon. Vanha `tiedostot.csv`:n `analyysitaso` voidaan johtaa tästä,
joten sitä ei tarvitse ylläpitää käsin.

### 3.5 Muutosten seuranta / Change tracking

- Tiiviste lasketaan normalisoidusta sisällöstä, joten pelkkä rivien siirtyminen ei vanhenna
  katselmointia. Siirtynyt alue tunnistetaan tunnisteen ja tiivisteen perusteella.
- Haaroja `origin/88`, `/90`, `/92` ja `/95` (E-2) ei arvioida tässä. Jos ne yhdistetään, työkalu
  ilmoittaa tiivisteestä niiden muuttamat funktiot uudelleenkatselmoitaviksi.
- D-vaiheen korjaus-PR:ssä ajetaan `kattavuus.py tila`. Muuttuneiden funktioiden on oltava
  uudelleen tasolla K4 ennen yhdistämistä (kirjataan `k-vaihe-suunnitelma.md` 9.4:n
  ylläpito-ohjeeseen).

### 3.6 Työkalut ja tarkistukset / Tools and checks

Vain luku, laajennetaan nykyisestä `kattavuus.py`:stä:

| Komento | Tekee |
|---|---|
| `alueet` | tuottaa `koodialueet.csv`:n (funktiot, määrittelyalueet, tiivisteet, tuotteet, parit, paketit) |
| `tuo <csv>` | tuo katselmointirivit: tunniste on olemassa, tiiviste täsmää, tasot ja näkökulmat ovat sallituja, ja jokaisella yksikön alueella on rivi tai rajaus; virheen sattuessa mitään ei kirjoiteta |
| `tila` | kattavuus paketeittain, tuotteittain ja tasoittain; vanhentuneet alueet; uudelleenkatselmointijono |
| `tarkista` | jokaisella aktiivisella alueella on K4/K5 tai kirjattu rajaus; ei vanhentuneita; liitetään `tarkista_auditointi.py`:n yhteyteen |

Ennen ensimmäistä yksikköä työkalu tarkastetaan riippumattomasti (Opus). Sen funktioluetteloa
verrataan C-9:n jäsentimeen, karkeaan hakuun ja otokseen käsin luettuja tiedostoja. Kääntäjän
symbolitaulua ei käytetä, koska tuotantokoodia ei käännetä ilman erillistä lupaa.

### 3.7 Mitä ei rakenneta / What is deliberately not built

- Ei rivitason kirjanpitoa eikä käsin ylläpidettävää tiedostotaulukkoa.
- Ei rinnakkaista havaintorekisteriä: ehdokkaat kulkevat nykyistä reittiä (luku 4.4).
- Ei uutta portaalia: kattavuus lisätään nykyisen portaalin kattavuusnäkymään.

---

## 4. Katselmointimenettely / Review procedure

### 4.1 Katselmointiyksikkö ja kulku / Unit and flow

Yksikkö on noin 1 500–2 500 aktiivista riviä ydinkoodia tai 2 500–3 500 riviä lomakekoodia. Se
kootaan yhdestä toiminnallisesta kokonaisuudesta, ja HK- ja viestiversiot ovat samassa
yksikössä, kun ne ovat pari.

1. **Valmistelu (pääistunto, työkalu):** yksikön alueluettelo, voimassa olevat haarat tuotteittain,
   aiemmat havainnot ja KE-ehdokkaat alueella, ominaisuusinventaarioiden rivit (luku 2.6),
   C-4:n varoitukset, käyttöohjeen ja käsikirjan luvut, parien samankaltaisuus.
2. **Katselmointi (agentti):** jokainen alue luetaan kokonaan, ja tarkistuslista käydään läpi
   jokaisen voimassa olevan haaran osalta. Kutsujat ja kutsuttavat luetaan siltä osin kuin
   oikeellisuus riippuu niistä.
3. **Kirjaus:** jokaiselle alueelle tulee rivi, jossa on syvyys, käydyt näkökulmat ja tulos.
   Jokaisesta ehdokkaasta kirjataan näyttö (`polku:rivi`) ja varmuus. "Ei havaintoja" on
   hyväksyttävä tulos, kun näkökulmat on kirjattu.
4. **Tuonti ja tarkistus (pääistunto):** `kattavuus.py tuo`. Puuttuvat alueet palautetaan
   yksikölle.
5. **Riippumaton tarkastus (Opus):** luku 4.5.
6. **Rekisteröinti:** luku 4.4.

### 4.2 Tarkistuslista / Checklist (L1–L18)

Jokainen kohta on kysymys. Agentti vastaa jokaiseen tai merkitsee sen ei-sovellettavaksi
perustellen.

| Koodi | Näkökulma | Keskeiset kysymykset |
|---|---|---|
| L1 | Liiketoimintalogiikan oikeellisuus | Tekeekö koodi sen, mitä käyttöohje ja käsikirja sanovat? Ovatko HK- ja viestiversio samaa mieltä samasta säännöstä? Onko ehdoissa käänteisiä, puuttuvia tai päällekkäisiä tapauksia? |
| L2 | Tulokset ja sijoitukset | Tasatulokset, hylätyt ja keskeyttäneet, puuttuvat ajat, aikojen ja sijojen järjestys, yhteis- ja vaihetulokset, viestin osuudet ja rinnakkaisosuudet, pisteet ja kaavat, pyöristys |
| L3 | Ajanotto ja aikojen käsittely | yksiköt (1/100 s, ms, s), keskiyön ylitys, negatiiviset ja nolla-ajat, kellojen siirrot ja korjaukset, aikavyöhyke ja vuorokausi, ylivuoto 16/32 bitissä |
| L4 | Muistiturvallisuus | puskurit, merkkijonot (leveät ja kapeat, `swprintf`-koot), osoittimien elinkaari, alustamattomat arvot |
| L5 | Taulukot, indeksit ja rajat | jokaisen indeksin lähde ja raja, myös −1 ja suurin arvo; ulkoisesta syötteestä tulevat indeksit |
| L6 | Tietorakenteiden elinkaari ja omistajuus | kuka varaa ja vapauttaa, kopiot ja vanhentuneet kopiot, nollaus kilpailun ja vaiheen vaihdossa |
| L7 | Tiedostot ja tietojen eheys | kirjoitusjärjestys, virheiden käsittely, osittaiset kirjoitukset, indeksit (`dbboxm`) |
| L8 | Säikeet, lukot ja rinnakkaisuus | mikä säie ajaa tämän, mikä lukko suojaa, lukitusjärjestys (C-6:n kartta), lukon vapautus kaikilla poluilla |
| L9 | Työasemien välinen tiedonsiirto | lähetys ja vastaanotto, kuittaus, uudelleenlähetys, ristiriitainen tila, versiot |
| L10 | Virheenkäsittely ja palautuminen | paluuarvot, ilmoitukset, jatketaanko virheen jälkeen oikeassa tilassa |
| L11 | Käyttöliittymän tietomuutokset | validointi, peruutus, tallennuksen kattavuus, samanaikaisuus vastaanoton kanssa, lomakkeen vanha kopio |
| L12 | Asetukset ja käännösvalitsimet | avaimen jäsennys, yksikkö ja oletus, haarat tuotteittain, kokoonpanoerot |
| L13 | Laiteyhteydet | sanomien jäsennys, pituudet, laitteen antamat indeksit, katkot |
| L14 | Ohjelmien väliset erot | onko ero tarkoituksellinen; parin samankaltaisuus; kopioituneet virheet |
| L15 | Tietoturva | verkosta tai tiedostosta tuleva syöte, polut, komentojen suoritus, SQL, HTTP |
| L16 | Resurssienhallinta | tiedostokahvat, GDI, muisti, soketit; toistuvat toiminnot |
| L17 | Poikkeukselliset kilpailukokoonpanot | monivaiheinen, viesti rinnakkaisosuuksin, takaa-ajo, sakot, pisteet, MAKI/HIIHTO-haarat, suuret määrät (sarjat, rastit, osuudet) |
| L18 | Hiljaiset tulosvirheet | voiko virhe muuttaa tulosta, sijaa tai julkaisua ilman ilmoitusta; havaittavuus |

Lisäksi kysytään jokaisesta kohdasta E-1:n viisi kysymystä (`korjaussuunnitelma.md` 9.1), jos
alueella on E-1:n rivi.

### 4.3 Liiketoimintalogiikka / Business logic

- **Vaatimuslähteet:** käyttöohje (`docs-site/`), käsikirja (`doc/tietopohja/kasikirja/`),
  koodin oma sisäinen johdonmukaisuus ja HK- ja viestiversioiden vertailu. Lajiliittojen sääntöjä
  ei ole repossa, eikä niitä haeta verkosta. Jos sääntö puuttuu, alueelle kirjataan aukko
  `vaatimus_puuttuu` ja kysymys omistajalle. Q-kysymyksen tai omistajan päätöksen kohdan
  tarvitseminen ratkaistaan integroinnissa.
- **Menetelmä normaalilta näyttävälle koodille:** jokaisesta laskentafunktiosta kirjataan
  syötteet, tulos ja invariantit (esim. "sija kasvaa ajan mukana", "hylätty ei saa sijaa").
  Sitten tarkistetaan reunatapausten luettelo (L2, L3, L17) ja verrataan paria. Kun
  laskentatulos kulkee muotoilun läpi, poikkeamat kirjataan näkökulmalla L18.
- **Esimerkkitapausten laskeminen käsin** on sallittua ja suositeltavaa. Koodia ei ajeta.

### 4.4 Havaintojen kirjaus / Recording findings

- Yksikkö tuottaa **ehdokkaita** väliaikaisin tunnuksin (`C14-W04-03-N1`), samaan tapaan kuin
  C-erät.
- Ehdokas, joka koskee jo rekisteröityä havaintoa, kirjataan sen tunnuksella lisätietona:
  laajuus, uusi polku, kumoaminen. Havainnon tekstiä tai prioriteettia ei muuteta.
- Uudet havainnot saavat H- ja T-tunnukset pääistunnossa vasta riippumattoman tarkastuksen
  jälkeen. Riskiulottuvuudet ja prioriteetti määritetään C-13:n menetelmällä (luku 9 rekisterissä).
  Juurisyy on JS-n tai `yksittainen`, ja korjauskokonaisuus valitaan nykyisistä tai uusi
  perustellaan. Hyväksyttyjen rivien prioriteetteja ei muuteta itsenäisesti: muutosehdotukset
  kirjataan KE-taulukkoon käyttäjän päätettäviksi.
- Jokainen ehdokas linkitetään alueeseen (`katselmoinnit.csv`:n `tulos`).

### 4.5 Riippumaton tarkastus / Independent review

| Kohde | Laajuus |
|---|---|
| P1–P2-ehdokkaat, tietojen eheys, ajanoton pysähtyminen, hiljaiset tulosvirheet, tietoturva | kaikki (Opus) |
| Muut keskitason ehdokkaat | kaikki, eräpohjaisesti |
| Matalat ehdokkaat | 20 % otos |
| **"Ei havaintoja" -yksiköt** | **15 % alueista satunnaisotoksena**, jossa uudelleenkatselmointi on sokea: tarkastaja ei näe ensimmäisen tulosta. Ohitusosuus mitataan, ja jos se ylittää 10 % vakavissa näkökulmissa, paketin yksiköt katselmoidaan uudelleen. |
| Työkalu (`kattavuus.py`) | ennen pilottia (luku 3.6) |

Tarkastus kumoaa tai heikentää väitteitä samalla tavalla kuin C-erissä. Kumotut kirjataan, ja ne
näkyvät yksikön tuloksessa.

### 4.6 Syvyyden todentaminen / Evidencing depth

K4 vaatii yksikön raportilta funktiokohtaisen taulukon (alue, käydyt näkökulmat, tulos), jonka
työkalu tarkistaa alueluetteloa vasten. Pelkkä "tiedosto käyty läpi" ei riitä. Jos yksikön raportti
ei kata kaikkia alueita, puuttuvat alueet jäävät K0/K1:ksi ja palautuvat jonoon.

---

## 5. Käännöshaarat ja rajaukset / Build branches and exclusions

Tavoite: **mitään koodia ei jätetä hiljaisesti pois.** Jokainen alue on joko katselmoitu tai
rajattu perustellusti. Rajaukset kirjataan tiedostoon `rekisteri/rajaukset.csv` (tunnus, alueet
sääntönä tai luettelona, peruste ja hyväksyjä), ja työkalu tarkistaa ne.

| Kohde | Käsittely |
|---|---|
| **Viisi tuotetta ja kirjastot** | Alue katselmoidaan kerran. Sen voimassa olevat haarat käydään läpi jokaisen tuotteen osalta, jossa se on (`tuotteet`). Kirjastokoodi kuuluu kaikkiin linkittäviin tuotteisiin. |
| **MSVC ja C++Builder** | Lähde on sama. Kääntäjäerot (C-5, `kaantajaerot.md`) ovat tarkistuslistan kohta L4/L12. GUI-koodi on vain bcc32:ta, joten sen muotoilu- ja rajavirheet tarkistetaan käsin, koska kääntäjän analyysiä ei ole. |
| **Aktiiviset valitsimet** | Haarat ovat C-12:n taulusta voimassa tuotteittain. Haara, joka on voimassa vain osassa tuotteita, käydään läpi kyseisten tuotteiden osalta ja merkitään. |
| **Tuntemattomat ehdot** (22 funktiota sekä haarat, kuten `MAKI`, `HIIHTO`) | Käsitellään aktiivisina: molemmat haarat katselmoidaan, ja ehto kirjataan. |
| **Vaihtoehtoiset kokoonpanot** (Debug, Demo, DebugV200, ReleaseVA200, Debug250, Release150os jne.) | Vain erotuksen katselmointi: haarat, joiden voimassaolo eroaa toimitetusta (C-12:n kokoonpanotaulu), käydään läpi, jos kokoonpanoa voidaan käyttää. Rakennetaanko jokin niistä käyttöön, on omistajan tieto. |
| **Kuollut koodi** (75 kokonaan passiivista funktiota, 3 721 riviä; noin 7 400 riviä kuolleissa haaroissa) | Kevyt tarkistus: vahvistetaan passiivisuus, arvioidaan, voiko kokoonpanon muutos aktivoida sen (riski), ja kirjataan poistoehdokkaaksi (T). Täyttä katselmointia ei tehdä; rajaus R-kuollut. |
| **Vanhat toteutukset ja orvot** (37 tiedostoa, 8 183 riviä; esim. `Tp/TpComY32.cpp` = vanha `TpComY.cpp`:n kopio, `Hk/HkTV.cpp`, `tputilv2/aakjarj*-2/-v.cpp`) | Vahvistetaan ohjelmallisesti, ettei tiedosto ole projektissa eikä sulkeumassa. Kopioiden ero aktiiviseen versioon mitataan. Rajaus R-orpo, ja havainnot T-047/T-007:n tapaan. |
| **Kolmas osapuoli** (21 tiedostoa, 1 820 riviä, tuotteissa) | Pieni määrä, joten **katselmoidaan kokonaan** (W13). MT19937 verrataan tunnettuun algoritmiin; lisenssikysymykset kuuluvat jo OP:hen (Q101). |
| **Generoitu koodi** (`RADStudio10/`-pääohjelmat, lomakkeiden `__published`-esittelyt, `.dfm`) | `.dfm`-tiedostoista luetaan ohjelmallisesti tapahtumakytkennät (mikä käsittelijä on käyttäjän tavoitettavissa) ja kenttärajat (`MaxLength` ym.) L11:n syötteeksi. IDE:n tuottamat esittelyt rajataan pois perustellusti (R-generoitu), ja niiden tyypit tarkistetaan työkalulla. |
| **Testikoodi** (`Tests/`, auditoinnin oma kehys) | Rajattu (R-testi). Ei tuotetta. |
| **Määrittelyalueet** (32 700 riviä) | Tietuerakenteet, taulukkokoot ja alkuarvot (`HkDef.h`, `VDef.h`, `*Globals.cpp`) katselmoidaan W01/W13:ssa. Lomakeluokkien esittelyt kuuluvat R-generoituun. |

---

## 6. Toteutus / Implementation

### 6.1 Työpaketit / Work packages

Koot ovat aktiivisia funktiorivejä (lähtötaso). *Viittaamatta* = ilman viitettä. Yksiköiden määrä
on arvio.

| Paketti | Sisältö | Rivejä | Viittaamatta | Yksiköitä | Malli |
|---|---|--:|--:|--:|---|
| W01 | Tietomalli ja tallennus: `HkDat`, `vdat`, `vkilp`, `HkTls`, `vApu`, `HkApu`, `dbboxm`, määrittelyt `HkDef.h`/`VDef.h` | 7 569 | 38 % | 5 | Opus |
| W02 | Asetukset ja alustus: `HkInit`, `VInit` (`lue_parametrit` osiin), `HkInitVanha` | 6 798 | 11 % | 4 | Opus |
| W03 | Ajanotto ja laitteet: `HkIV`, `VIv`, `TpLaitteet`, `ZebraReader`, `SiritReader`, `HkAjat`, `vajat`, `HkSak` | 10 965 | 12 % | 6 | Opus |
| W04 | Leimat ja emit: `HkEmit`, `VEmit` | 11 692 | 41 % | 6 | Opus |
| W05 | Sarjat, tulokset, sijoitukset: `HkSrj`, `vSarjat`, `HkIx`, `VIx`, `VRinnakkaisOsuus`, `HkEnnTav` | 4 259 | 21 % | 3 | Opus |
| W06 | Tulostus ja muotoilu: `HkTulostus`, `VTulostus`, `HkMuotoilu`, `VMuotoilu`, `TpApu`, `VEmitHtml` | 19 401 | 54 % | 9 | Opus |
| W07 | XML ja IOF: `HkXml*`, `VXml*`, `XMLtulos` | 7 096 | 55 % | 4 | Opus |
| W08 | Tiedonsiirto: `TpComY`, `HkCom32`, `VCom`, `com/` | 6 155 | 23 % | 4 | Opus |
| W09 | Konsoli-UI: `HkConsole/`, `JukConsole/` | 14 149 | 42 % | 6 | Sonnet, laskentapolut Opus |
| W10 | GUI HK (`cbHk/`): ajanotto ja emit; kilpailijat; sarjat ja radat; tulokset ja muotoilu; asetukset; tuonti ja vienti; arvonta (`HkArv2`); yhteydet; muut | 47 431 | 61 % | 16 | Sonnet; laskenta, arvonta ja tietomuutokset Opus |
| W11 | GUI viesti (`ViestiWin/`): samat ryhmät W10:n pareina | 29 662 | 72 % | 8 | Sonnet; erot Opus |
| W12 | GUI yhteiset (`cbTp/`): radat (`UnitRadat`), emiTag, FIS ym. | 8 784 | 77 % | 3 | Sonnet |
| W13 | Apukirjastot ja kolmas osapuoli: `tputilv2/`, `include/`, Blaise ja MS, MT19937, C-8:n 153 staattista muuttujaa | 7 400 + 1 820 | 71 % | 4 | Sonnet |
| W14 | Kokoonpanoerot, kuollut koodi, orvot, generoitu koodi | – | – | 3 | Sonnet + Opus-arvio |
| **Yht.** | | **181 361** + määrittelyt | **52 %** | **~81** | |

W01–W08 ovat ydinkoodia (73 935 riviä). Kaikki luvut tulevat taulusta
`todisteet/c14/lahtotaso/funktiot.csv`, ja lopullinen jako tehdään työkalulla (`paketti`,
`yksikko`).

### 6.2 HK- ja viestiversiot / HK and relay pairs

Parit (`todisteet/c14/lahtotaso/parit.csv`, samannimiset funktiot kerroksittain; rivit molemmat
puolet yhteensä):

| Kerros | identtinen (≥ 0,98) | lähes sama (0,80–0,98) | eriytynyt (0,40–0,80) | eri toteutus |
|---|---|---|---|---|
| ydin | 121 paria / 3 319 riviä | 78 / 6 818 | 137 / 16 462 | 78 / 19 045 |
| konsoli | 5 / 132 | 5 / 752 | 15 / 1 646 | 21 / 6 912 |
| GUI | 435 / 6 451 | 176 / 9 924 | 225 / 17 065 | 82 / 8 880 |

Käsittely:

- **Identtinen:** katselmoidaan kerran. Toinen puoli kuitataan samalla kirjauksella, kun tiiviste
  täsmää ympäristöä myöten (globaalit ja tietueet ovat eri tyyppiä HK:ssa ja viestissä: L14).
- **Lähes sama ja eriytynyt:** toinen puoli katselmoidaan kokonaan ja toinen erotuksena. Jokainen
  ero luokitellaan tarkoitukselliseksi, kopioituneeksi virheeksi tai korjaukseksi, joka puuttuu
  toiselta puolelta. Kolmas tapaus on havaintoehdokas, esimerkiksi H-141 ja KE-20.
- **Eri toteutus:** kumpikin katselmoidaan omanaan, mutta samassa yksikössä, jotta sääntöjen
  ristiriidat näkyvät.

Säästö on rajallinen: noin 10–15 % ydin- ja GUI-riveistä. Hyöty on ennen kaikkea siinä, että
eriytymisestä syntyvät virheet löytyvät.

### 6.3 Agentit ja konteksti / Agents and context

- **Malli:** Opus ydinkoodiin, laskentaan, tietomuutoksiin ja riippumattomaan tarkastukseen.
  Sonnet lomakkeiden ja apukoodin rutiinikatselmointiin; vakavat ehdokkaat ohjataan aina
  Opus-tarkastukseen. Enintään kolme agenttia rinnakkain, eikä keinotekoisia tokenrajoja.
- **Yksikön ohje** (`laatu/c14-katselmoijan-ohje.md`, laaditaan C14-0:ssa): menettely,
  tarkistuslista, kirjausmuoto, merkistösäännöt (`merkistot.md`; cp1252-luku `iconv`illa) ja
  kielto muokata tiedostoja yksikön ulkopuolella.
- **Yksikön tehtävänanto** (tuotetaan työkalulla): alueluettelo riviväleineen, voimassa olevat
  haarat tuotteittain, aiemmat havainnot ja KE-rivit, ominaisuusinventaarioiden otteet, varoitukset,
  käsikirjan ja käyttöohjeen kohdat sekä parit.
- **Kirjoitusoikeus:** agentti kirjoittaa vain yksikön raportin ja CSV-liitteen
  `todisteet/c14/<paketti>/` -hakemistoon. Rekistereihin kirjoittaa vain pääistunto työkalulla.

### 6.4 Järjestys / Order

Riskiä ja riippuvuuksia seuraten. Ensin ydin, jotta lomakkeiden katselmoija tietää, mitä kutsuttu
koodi tekee.

1. **C14-0 Valmistelu:** työkalun laajennus (`alueet`, `tuo`, `tila`, `tarkista`, määrittelyalueet,
   `.dfm`-kytkennät, yksikköjako), työkalun riippumaton tarkastus, katselmoijan ohje ja
   rajausrekisterin pohja.
2. **C14-1 Pilotti** (pysähdys hyväksyntää varten): kolme yksikköä, joista yksi on
   laskentalogiikkaa (W05 tai `HkTls`), yksi lomakekokonaisuus pareineen (W10/W11) ja yksi
   `dbboxm`-avaimet. Mukana ovat riippumaton tarkastus ja sokea "ei havaintoja" -otos. Mitataan
   tokenit, aika ja ehdokkaiden ja kumottujen määrä, ja tarkistuslistaa kalibroidaan.
3. **C14-2 Ydin:** W01, W03, W04, W05, W08, W02, W06, W07.
4. **C14-3 Käyttöliittymät:** W09, W10/W11 pareittain, W12.
5. **C14-4 Loput:** W13 ja W14 sekä rajausten hyväksyntä.
6. **C14-5 Integrointi:** luku 7.
7. **C14-6 Kokonaistarkastus:** Opus-kokonaisarvio, hyväksymiskriteerit (luku 8) ja sulkeminen
   käyttäjän päätöksellä.

Jokaisen vaiheen jälkeen tehdään paikallinen commit. Rekisterit päivitetään vasta tarkastuksen
jälkeen.

### 6.5 Keskeyttäminen ja jatkaminen / Pause and resume

- `CHECKPOINT.md`: vaihe, valmiit yksiköt, avoimet tarkastukset, seuraava askel.
- Jatkamisohje `todisteet/c14/eteneminen.md` (vastaa `k-eteneminen.md`:tä): yksikkö on valmis, kun
  sen raportti on tuotu ja `kattavuus.py tila` näyttää alueet tasolla K4. Keskeneräinen yksikkö
  aloitetaan alusta. Raportit tallennetaan heti agentin palattua.

### 6.6 Seuranta / Tracking during the work

`kattavuus.py tila` tuottaa pakettikohtaisen etenemistaulukon: alueet ja rivit tasoittain, vanhentuneet
alueet, avoimet tarkastukset sekä ehdokkaat ja niiden tila. Taulukko lisätään portaalin
kattavuusnäkymään ja CHECKPOINTin tilariville.

### 6.7 Työmäärä / Effort

| Erä | Määrä | Tokenit (arvio) | Agenttiaika (arvio) |
|---|--:|--:|--:|
| Katselmointiyksiköt | ~81 | 25–30 M (200–400 k / yksikkö) | 20–30 h |
| Riippumaton tarkastus (ehdokkaat ja sokea otos) | 25–35 erää | 7–10 M | 6–10 h |
| C14-0 ja pilotti | – | 2–3 M | 3–4 h |
| Integrointi (rekisterit, riskiarvio, kokonaisuudet, testaussuunnitelma) | – | 2–3 M | 3–5 h |
| **Yhteensä ilman tietopohjan jatkotyötä** | | **35–45 M** | **30–45 h**, 10–15 istuntoa |

Perusteet: K-vaiheen agentit käyttivät 200 000–450 000 tokenia tehtävää kohden. Yksikkö lukee noin
2 000 riviä koodia useaan kertaan sekä kontekstiasiakirjat. Kolme agenttia rinnakkain.

**Uudet havainnot ovat suurin epävarmuus.** C-vaihe tuotti 229 riviä lukemalla noin neljänneksen
funktioriveistä riskiperusteisesti. Loppuosa on keskimäärin vähemmän riskialtista, mutta mukaan
tulee liiketoimintalogiikka. Varovainen arvio on 150–500 uutta ehdokasta, joista suurin osa
matalia. Arvio tarkentuu pilotissa. Uudet havainnot lisäävät riskiarvion, korjauskokonaisuuksien
ja tietopohjan työtä (luku 7). Esimerkiksi täysi vikakortti jokaiselle uudelle havainnolle
vastaisi K-2:n kokoista työtä.

### 6.8 Pilotin hyväksymisehdot / Pilot go/no-go

Jatkon edellytykset:

- työkalu on tarkastettu, eikä sokea otos paljasta yli 10 %:n ohitusosuutta vakavissa näkökulmissa;
- yksikön kirjaus täyttää K4:n ehdot (luku 4.6);
- mittaukset (tokenit, aika ja ehdokkaat yksikköä kohden) ja arvio koko työstä on päivitetty.

Käyttäjä hyväksyy jatkon.

---

## 7. Tulosten yhdistäminen ja suhde muihin vaiheisiin / Integration and relation to other phases

### 7.1 Kohteet / Targets

| Kohde | Miten C-14 päivittää |
|---|---|
| `rekisteri/havainnot.md` | uudet H- ja T-rivit tarkastettujen ehdokkaiden pohjalta; olemassa oleville riveille vain laajuutta ja polkuja koskevia lisätietoja |
| `rekisteri/tekninen-velka.csv` | uudet T-rivit, kuten kuollut koodi, orvot ja kopiot |
| `rekisteri/riskiarvio.csv` | uusille riveille C-13:n menetelmällä; muutosehdotukset vanhoihin KE-taulukon kautta |
| juurisyyt (`juurisyy`, `todisteet/c13/raportit/osaB-juurisyyt.md`) | uudet rivit JS-1–JS-15:een tai uusi JS perusteltuna |
| `rekisteri/korjauskokonaisuudet.csv` | jäsenet ja tarvittaessa uudet kokonaisuudet; `synteesi.py --tarkista` |
| `regressiotestaussuunnitelma.md` | testit uusille P1–P3-havainnoille; liiketoimintalogiikan testitapaukset (L2/L3/L17:n esimerkkilaskelmat) |
| `rekisteri/tiedostot.csv` | `analyysitaso` johdetaan kattavuusrekisteristä |
| vikakortisto (`doc/tietopohja/vikakortit/`) | generoidut lohkot päivittyvät (`lohkot`). Uusille havainnoille kortit: laajuus (kaikki vai P1–P3 ensin) on käyttäjän valinta, luku 10 |
| käsikirja | `toiminnot.csv`-kytkennät uusille havainnoille, liiketoimintasääntöjen kuvaukset lukuun 06; vanhentuneet [päätelty]-merkinnät korjataan |
| portaali | kattavuusnäkymä funktio- ja pakettitasolla `koodialueet.csv`:stä |
| asiantuntija-agentti | reittitaulukkoon C-14:n raportit; sääntö: alueen katselmointitaso kerrotaan vastauksessa |
| `auditoinnin-yhteenveto.md` | uusi kattavuusluku (korvaa luvun 3 tulkinnan); kattavuusrajoitteen teksti tietopohjassa päivitetään vasta hyväksymisen jälkeen |

Rinnakkaista järjestelmää ei synny: kattavuusrekisteri on uusi rekisteri, mutta havainnot,
riskit ja kokonaisuudet pysyvät nykyisissä rekistereissä.

### 7.2 E-vaihe ja KE-ehdokkaat / Phase E and candidates

- **E-1 (62 rivin riskiseulonta):** suositus on yhdistää C-14:ään. Kunkin rivin funktio luetaan
  joka tapauksessa omassa yksikössään, ja E-1:n viisi kysymystä lisätään yksikön tehtävänantoon.
  E-1:n tulosmuoto ja valmiusehto säilyvät (`riskiarvio.csv`:n `muutos`, `peruste`, `huomio`,
  pääistunnon arvio). Jos D halutaan aloittaa ennen C-14:n valmistumista, E-1 tehdään erikseen
  ensin: se on pieni.
- **E-2 (haarojen täsmäytys):** suositus on tehdä ennen C14-1:tä. Neljä haaraa muuttaa
  tuotantotiedostoja, jotka ovat C-14:n ytimessä (`HkEmit`, `HkIV`, `VEmit`, `HkInit`, `VInit`,
  `TpLaitteet`, `HkDef.h`, `cbHk/UnitIlm2`, `UnitTKanta`). Katselmoitava revisio on päätettävä
  (OP-18). Jos lähtökohta pysyy `main`issa, haarojen muutokset katselmoidaan myöhemmin erotuksena
  tiivisteiden avulla (luku 3.5).
- **E-3 (säilytys):** C-14 lisää luottamuksellista aineistoa. Varmuuskopio tulee entistä
  tärkeämmäksi, ja julkiseen repoon vientikielto on voimassa.
- **E-4:** OP-1–OP-3 eivät estä C-14:ää.
- **KE-1–KE-24:** ehdokkaat ovat yksiköiden syötteitä. Yksikkö, jonka alueella ehdokas on,
  todentaa tai kumoaa sen. Ehdokkaan käsittely rekisteriin noudattaa nykyistä menettelyä, eikä
  KE-taulukon rivejä muuteta muuten kuin käsittelytuloksen kirjauksella.

### 7.3 D-vaihe / Phase D

- **Oletus:** D alkaa, kun C-14 on hyväksytty ja sen havainnot on integroitu, koska korjausjärjestys
  ja kokonaisuudet voivat muuttua.
- **Vaihtoehto (omistajan päätös):** aluekohtainen aloitus. Korjauskokonaisuus voi alkaa, kun sen
  kaikkien jäsenten alueet ovat tasolla K4/K5 ja alueen ehdokkaat on integroitu. Tämä nopeuttaa
  P1-korjauksia, mutta riskinä on, että myöhemmin löytyvä saman juurisyyn kohta muuttaa jo tehtyä
  korjausta.
- C-14 ei muuta jo hyväksyttyä korjaussuunnitelmaa itsenäisesti. Muutokset tehdään integroinnissa
  käyttäjän hyväksyttäviksi.

---

## 8. Hyväksymiskriteerit / Acceptance criteria

C-14 on valmis, kun kaikki seuraavat ehdot täyttyvät. Tarkistukset 1–4 ja 8 ovat ohjelmallisia
(`kattavuus.py tarkista`), muut kirjataan hyväksymisraporttiin.

1. **Kaikki lähdetiedostot käsitelty:** jokainen `tiedostot.csv`:n tiedosto on luokiteltu, ja
   jokaisella sen alueella on katselmointi tai rajaus.
2. **Aktiivinen koodi katselmoitu:** jokainen alue, joka on voimassa vähintään yhdessä tuotteessa
   tai jonka ehto on tuntematon, on tasolla K4 tai K5 voimassa olevalla tiivisteellä. Kevyesti
   täydennetyt alueet (luku 2.6) täyttävät saman ehdon kirjatuin näkökulmin.
3. **Rajaukset perusteltu:** jokaisella rajatulla alueella on `rajaukset.csv`:n rivi, jolla on
   peruste ja hyväksyntä. Rajattuja aktiivisia alueita ei ole ilman käyttäjän hyväksyntää.
4. **Käännöshaarat:** jokaisen alueen kirjauksessa on käydyt tuotteet. Tuotekohtaiset haarat ja
   tuntemattomat ehdot on käyty läpi, ja vaihtoehtoisten kokoonpanojen erotus on käsitelty tai
   rajattu.
5. **Havainnot:** jokainen ehdokas on rekisteröity, yhdistetty olemassa olevaan, kumottu tai
   siirretty KE-taulukkoon perustellen. Uusilla riveillä on riskiarvio, juurisyy, kokonaisuus ja
   testi (P1–P3).
6. **Riippumaton tarkastus:** kaikki vakavat (luku 4.5) on tarkastettu. Sokean otoksen
   ohitusosuus on mitattu ja raportoitu jokaisesta paketista, ja raja ylittäneet on katselmoitu
   uudelleen.
7. **Tietopohja:** rekisterit, kortiston generoidut osat, käsikirjan kytkennät ja portaali on
   päivitetty, ja kaikki nykyiset tarkistukset menevät läpi.
8. **Seurattavuus:** kattavuus on luettavissa ohjelmallisesti, eikä yhtään aluetta ole
   vanhentuneena.
9. **Rehellinen raportointi:** loppuraportti kertoo, mitä katselmointi ei todenna (luku 9).
   Tietopohjan kattavuusrajoite muutetaan muotoon, joka kuvaa täyden katselmoinnin rajat, mutta
   sitä ei poisteta.

**Täysi katselmointikattavuus ei tarkoita, että ohjelmisto olisi virheetön.** Se tarkoittaa, että
jokainen aktiivinen koodialue on luettu kokonaan ja arvioitu kirjatuista näkökulmista ja että
otoksella on mitattu, kuinka paljon katselmointi jättää huomaamatta.

---

## 9. Menetelmän rajoitteet / Limitations

- **Staattinen:** mitään ei ajeta. Ajonaikaiset ilmiöt, kuten ajoitus, laitteet, verkko, SQL ja
  bcc32-GUI:n ajonaikainen käytös, jäävät päätelmiksi. Ne siirretään testaussuunnitelmaan.
- **Agenttikatselmoinnilla on ohitusosuus.** K-5:n otoksessa noin 6 väitettä 83 kortista oli
  asiavirheitä, ja käsikirjan 79 väitteestä 6 kumoutui. Vastaava virhe- ja ohitusosuus on
  odotettavissa. Siksi otos mitataan, eikä tasoa K4 pidetä todisteena virheettömyydestä.
- **Vaatimukset:** lajisääntöjä ei ole saatavilla. Liiketoimintalogiikan oikeellisuus arvioidaan
  käyttöohjetta ja sisäistä johdonmukaisuutta vasten, mikä voi jättää huomaamatta säännön, jota
  käyttöohje ei kerro.
- **Työkalun heuristiikka:** funktioiden tunnistus ei ole C++-jäsennin. Makrot, jotka laajenevat
  aaltosulkeiksi, ja epätavalliset esittelyt voivat harhauttaa sitä. Tarkkuus on mitattu (luku 2.1),
  ja epäselvät kohdat luetellaan.
- **Haarojen voimassaolo** perustuu C-12:n kolmiarvoiseen arvioijaan. Sen tuntemattomat ehdot
  käsitellään aktiivisina, mikä lisää työtä.
- **Kontekstin raja:** suuret funktiot (58 yli 300 rivin, suurin 2 222 riviä) jaetaan osiin.
  Osien välinen vuorovaikutus tarkistetaan erikseen.
- **Lähtötason epävarmuus:** ±10 rivin oletus, perusnimiviitteet (108) ja tallentamattomat
  työkalulokit. Lähtötaso ei vaikuta C-14:n tulokseen, vain lähtötason raportointiin.

---

## 10. Avoimet valinnat käyttäjälle / Open choices for the user

Näitä ei ole päätetty. Suositukset ovat ehdotuksia.

1. **Suunnitelman hyväksyntä ja aloitus** C14-0:sta ja pilotista (pysähdys pilotin jälkeen).
2. **E-2 ennen C-14:ää** (suositus) ja katselmoitava revisio (OP-18).
3. **E-1 yhdistetään C-14:ään** (suositus) vai tehdään erikseen ensin.
4. **D-vaiheen ajoitus:** C-14:n jälkeen (oletus) vai aluekohtaisesti (luku 7.3).
5. **Uusien havaintojen tietopohjakäsittely:** kortti kaikille vai ensin P1–P3:lle, muille
   rekisteririvi ja generoitu yhteenveto.
6. **Vaihtoehtoiset kokoonpanot:** mitkä ei-toimitetut kokoonpanot ovat käytössä tai
   käyttöön otettavissa (vaikuttaa luvun 5 erotuskatselmointiin).
7. **Liiketoimintasääntöjen lähteet:** onko omistajalla sääntöaineistoa (lajiliittojen säännöt,
   aiemmat määrittelyt), joka voidaan tuoda paikallisesti.
8. **Työkalun tarkistus kääntäjällä** (valinnainen): symbolitaulun vertailu edellyttäisi
   käännöstä, mikä vaatii erillisen luvan. Ilman sitä tarkistus tehdään luvun 3.6 keinoin.

---

## 11. Lähteet / Sources

- `todisteet/c14/lahtotaso/` (tämän luvun luvut), `tyokalut/kattavuus.py`.
- `auditoinnin-yhteenveto.md` luvut 3, 4 ja 10; `rekisteri/README.md` (analyysitasot).
- `saikeet-ja-lukot.md` luku 11, `virheenkasittely.md` luku 10, `globaali-tila.md` luku 2,
  `muistiturvallisuus.md` luku 2, `gui-liiketoimintalogiikka.md` luku 9,
  `ominaisuusvalitsimet.md` luku 2, `riippuvuudet.md` luvut 8 ja 19, `inventaario.md` (orvot).
- `todisteet/c12/osaA0/testikohdat.csv` (haarojen voimassaolo), `todisteet/c9/osaA/funktiot.csv`
  (tunnistimen vertailu).
- `korjaussuunnitelma.md` luku 9 (E-1–E-4), `seuranta.md` (KE-1–KE-24),
  `k-vaihe-suunnitelma.md` 9.4 (ylläpito).
