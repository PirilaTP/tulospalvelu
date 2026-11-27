# 00 Lukuohje

<!-- malli: Käsikirjan lukupohja (K-3). Ohje: doc/tietopohja/laatu/k3-kirjoittajan-ohje.md.
Tässä luvussa ei ole toimintoja, joihin vikoja kytkettäisiin; luvun 5 vikaluettelon generointilohkoa ei
ole, koska luku ei kuvaa yhtä toimintoa. -->

> **Tila:** alustava K-3-aineisto, ei riippumattomasti tarkastettu. Lähderevisio: tuotantokoodi
> `f7fe553` (ennallaan, `doc/arkkitehtuuri/CHECKPOINT.md`). Varmuusmerkinnät: [todennettu],
> [päätelty], [tuntematon]; "luettu käsikirjaa varten" = luettu koodista K-vaiheessa, ei tarkastettu.

Tämä luku kertoo, mitä käsikirja on, mitä se ei ole, miten sen merkintöjä luetaan ja mihin se on
sidottu. Lue se ennen muita lukuja. Polut ovat suhteessa hakemistoon `TPsource/V52/`, ellei polku
ala `doc/`, `docs-site/` tai `.github/`. Lyhenteet: **HKM** = HkMaali, **HKW** = HkKisaWin,
**VM** = ViestiMaali, **VL** = ViestiLuenta, **VW** = ViestiWin.

**Julkisuus.** Käsikirja ja vikakortit kuvaavat korjaamattomia vikoja, myös sellaisia, jotka voivat
laueta verkosta. Omistaja päätti 2026-11-27 (OP-1, vaihtoehto a), että ne julkaistaan ennen
korjauksia (`doc/tietopohja/README.md`; OP-1 luvussa 14).

## 1. Mitä tapahtuu

### 1.1 Mitä käsikirja on

Käsikirja kertoo, **miten ohjelmisto oikeasti toimii** kilpailun järjestäjän ja kehittäjän
näkökulmasta: mitä ohjelmissa tapahtuu eri käyttötilanteissa, mitä kirjoitetaan mihin tiedostoon
ja milloin, mikä toimii eri tavoin eri ohjelmissa ja missä kohdissa on tunnettuja vikoja
[todennettu: `doc/arkkitehtuuri/k-vaihe-suunnitelma.md` luku 7.1]. Sen kohdeyleisö on kokenut
tulospalvelun käyttäjä tai järjestäjä, joka tuntee kilpailut, leimauksen, väliajat ja työasemat, sekä
kehittäjä, joka haluaa tietää, mihin koodiin toiminto perustuu. C++-osaamista ei oleteta; tekniset
termit selitetään käytännön kautta tai viitataan sanastoon (`doc/tietopohja/sanasto.md`).

Käsikirja on kirjoitettu **auditoinnin jälkeen** (vaihe K), ja sen lähteinä ovat auditoinnin
asiakirjat, rekisterit ja vikakortit. Koodia on luettu vain siellä, missä aineisto ei kerro
toimintaa (kohta 1.5).

### 1.2 Mitä käsikirja ei ole

- **Ei käyttöohje.** Painikekohtaiset ohjeet ovat käyttöohjeessa (`docs-site/docs/`). Käsikirja viittaa
  niihin merkinnällä [KO: luku] ja kertoo, mitä pinnan alla tapahtuu. Käyttöohjetta on käytetty
  vain käyttäjälle näkyvän toiminnan ja asetusten nimien lähteenä.
- **Ei uusi auditointi.** Käsikirja ei etsi uusia vikoja eikä ehdota korjauksia. Jos kirjoittaja
  huomasi lukiessaan jotakin epäilyttävää, se on kirjattu lukujen 7-kohtiin "Ehdokkaat (ei
  tarkastettu)" ilman havaintotunnusta ja koottu lukuun 14.
- **Ei toimintaohjeiden lähde.** Käsikirja ei anna toimintaohjeita eikä kiertotapoja, joita aineisto ei
  tue. Jos jokin ohje mainitaan, se on tuotu lähteestä ja sen varmuus on merkitty. Kokonaista
  palautumisohjetta kilpailun aikaisille vikatilanteille ei löytynyt käyttöohjeesta eikä raporteista
  (luku 13, kohta 7).
- **Ei takuu virheettömyydestä.** Se, ettei toiminnolle ole vikaa listattu, ei tarkoita, että toiminto
  on virheetön (kohta 1.6).
- **Ei todiste siitä, että ohjelma on ajettu.** Lähes kaikki käsikirjan väitteet perustuvat koodin ja
  asiakirjojen lukemiseen (kohta 1.4). Yhtään GUI-ohjelmaa, laitetta tai usean työaseman verkkoa ei ole
  ajettu auditoinnissa [todennettu: `auditoinnin-yhteenveto.md` luku 6, rajoitteet 2-4].

### 1.3 Luvut

| Luku | Aihe | Koskee erityisesti |
|---|---|---|
| 00 | Tämä lukuohje | kaikki |
| 01 | Ohjelmat ja kokonaisuus: viisi ohjelmaa, HK ja viesti, konsoli ja GUI, työasemat, kääntäjäerot | kaikki |
| 02 | Kilpailun elinkaari: perustaminen, avaaminen, sarjat, kilpailijat, radat, lähtöajat | HK ja viesti |
| 03 | Tiedostot ja tallennus, uudelleenkäynnistys | kaikki |
| 04 | Ajanotto: kellot, lähtö, maali, väliajat, aikajonot | kaikki |
| 05 | Emit-kortin luenta ja leimantarkastus | kaikki |
| 06 | Tuloslaskenta, tulostila, tulosten muuttaminen | kaikki (viestin sijoitusosat vain osin luettu) |
| 07 | Viestit: joukkueet, osuudet, vaihdot, yhteistulokset | viesti |
| 08 | Monivaiheiset kilpailut ja vaiheen vaihto | HK |
| 09 | Työasemien tiedonsiirto | kaikki |
| 10 | Laitteet: lukijat, RFID, kellot, näyttötaulut, tulostimet | kaikki |
| 11 | Tulosten julkaisu: tulosteet, HTML, XML, automaattitulostus, SQL | kaikki |
| 12 | Asetukset: asetustiedosto, komentorivi, erot ohjelmien välillä | kaikki |
| 13 | Virhetilanteet ja toipuminen | kaikki |
| 14 | Tunnetut rajoitteet ja mitä ei ole selvitetty | kaikki |

Luvuilla 02-13 on sama rakenne (kohta 1.7). Luvut 01, 12 ja 14 sidotaan toisiinsa: luku 01 kertoo,
mitä ohjelmia on olemassa, luku 12 kertoo, miten niitä säädetään, ja luku 14 kokoaa, mitä
aineisto ei kerro.

### 1.4 Varmuusmerkinnät

Jokaiselle olennaiselle väitteelle on merkitty, mihin se perustuu. Merkinnät ovat samat kaikissa
luvuissa, vaikka yksittäisen luvun kirjoittaja on voinut käyttää niiden rinnalla lyhyttä
lähdemerkintää.

| Merkintä | Merkitys | Mitä se **ei** tarkoita |
|---|---|---|
| [todennettu] | Lähde (asiakirja, rekisteri tai koodirivi) osoittaa asian. Lähde mainitaan merkinnän yhteydessä. | Ei tarkoita, että asia olisi **ajettu** tai mitattu, ellei se ole erikseen sanottu. |
| [päätelty] | Kirjoittaja tai lähde on päätellyt asian muista tiedoista. Peruste kerrotaan. | Ei ole havainto; päätelmä voi olla väärä. |
| [tuntematon] | Asiaa ei voi ratkaista aineistosta eikä koodista ilman ajoa, laitetta tai omistajan tietoa. | Ei tarkoita, että asia olisi kunnossa tai että sillä ei olisi vaikutusta. |
| luettu käsikirjaa varten, ei tarkastettu | Kirjoittaja luki kohdan koodista K-vaiheessa, koska auditointiaineisto ei kuvaa sitä. Merkintä näkyy myös muodoissa [luettu käsikirjaa varten] ja [K-luettu]. | Kohtaa ei ole auditoitu eikä toisen lukijan tarkastama. Siitä ei ole johdettu havaintoja. |
| [AA: asiakirja luku] | Tieto on jo auditointiaineistossa (`doc/arkkitehtuuri/`). Asiakirjan oma varmuusmerkintä säilyy, jos se on kirjattu. | - |
| [KO: luku] | Tieto on käyttöohjeessa (`docs-site/docs/`). | Käyttöohje on vuodelta 2015 (versio 5.20) ja voi poiketa koodista. |
| [mitattu], [staattinen], [todentamaton] | Auditoinnin raporttien (erityisesti `kaantajaerot.md`) omat merkinnät: koe tai käännöstulos; päätelty lähdekoodista; avoin riski. | - |

**Mitä [todennettu] tarkoittaa auditoinnissa.** Auditointi erottaa neljä todistelajia, ja ne ovat eri
asioita [todennettu: `auditoinnin-yhteenveto.md` luku 4]:

1. staattinen luku: yksi agentti lukee koodin;
2. riippumaton lähdetarkastus: toinen agentti lukee koodin ja yrittää kumota väitteen. Tämä on
   staattista, samaa mallisukua ja samaa repositoriota, eikä ihminen ole mukana;
3. mekanismin toisto testikehyksessä tai eristetyssä kokeessa (MSVC-konsolirakennelmat). Se osoittaa
   mekanismin, ei sitä, että laukaisija esiintyy käynnissä olevassa ohjelmassa;
4. vahvistus toimitetussa ohjelmassa: vain yksi vika (H-041), ja sekin käyttäjän hyväksymällä
   poikkeamalla ennalta lukitusta tuloksesta.

Kun käsikirja sanoo "[todennettu]", se tarkoittaa lähes aina lajia 1 tai 2. Vikakortti kertoo,
mikä laji on kyseessä.

### 1.5 "Luettu käsikirjaa varten, ei tarkastettu"

Auditointi ei lukenut koko koodia. Kun käsikirjan luku tarvitsi tietoa alueelta, jota aineisto ei
kuvaa (esimerkiksi arvonta, viestin konsoliohjelmien toiminnot tai laitekohtainen polku), kirjoittaja
sai lukea koodin toiminnan kuvaamiseksi [todennettu: `k-vaihe-suunnitelma.md` luku 7.3]. Tällainen kohta
on merkitty ja siihen on yleensä liitetty `tiedosto:rivi`. Merkinnän käytännön seuraukset:

- kohdan tiedot ovat yhden lukijan tulkinta eikä niitä ole tarkastettu toisella lukijalla;
- rivinumerot kuuluvat lähderevisioon `f7fe553` ja vanhenevat, jos koodi muuttuu;
- jos luku mainitsee vikaa muistuttavan seikan, se on "Ehdokkaat (ei tarkastettu)" luvun 7-kohdassa,
  ei rekisterin havainto. Ehdokkaille ei ole annettu H-tunnusta eikä riskiarviota muutettu.

Poikkeus: Emit-luennan selvitys (`doc/tietopohja/kasikirja/aineisto/emit-luenta.md`, josta luvut 05 ja
09 ammentavat) on riippumattomasti tarkastettu ja korjattu 2026-11-27 (`laatu/tarkastukset/emit/`).
Muut luvut ovat tämän tarkastuksen ulkopuolella.

### 1.6 Kattavuusrajoite

Auditointi ei kata koko lähdekoodia, ja tämä rajoittaa kaikkea käsikirjassa [todennettu:
`auditoinnin-yhteenveto.md` luvut 3, 4 ja 10]:

- noin 36 % koodin riveistä (474 tiedostoa) on vain inventoitu skriptillä, eikä niitä ole luettu;
  GUI-lomakehakemistot (`cbHk/`, `ViestiWin/`, `cbTp/`) muodostavat tästä 81 %;
- yhtäkään tuotantotiedostoa ei ole katselmoitu kokonaan;
- löydettyjen vikojen määrä on **alaraja**, ei kokonaismäärä;
- yhtään GUI-ohjelmaa, laitetta, usean aseman verkkoa tai SQL-palvelinta ei ole ajettu.

Käsikirjan luvut seuraavat tätä rajaa: jos aihe on alueella, jota auditointi ei lukenut, luvun 7-kohta
sanoo sen. Luku 14 kokoaa rajoitteet ja avoimet kysymykset yhteen.

### 1.7 Luvun rakenne ja luvun 5 vikaluettelo

Luvuilla 02-13 (ja 01 ja 12) on seitsemän kohtaa:

1. **Mitä tapahtuu**: toiminta vaihe vaiheelta käyttäjän näkökulmasta ja sen alla teknisesti.
2. **Erot ohjelmien välillä**: HK ja viesti, konsoli ja GUI.
3. **Asetukset ja käännösvalitsimet**: olennaiset asetusavaimet ja valitsimet.
4. **Vaihtoehtoiset polut ja virhetilanteet.**
5. **Tunnetut viat tässä toiminnossa**: vikaluettelo.
6. **Mihin tieto perustuu**: lähteet.
7. **Mitä ei ole selvitetty.**

Kohdan 5 taulukko ei ole käsin kirjoitettu. Se generoidaan tiedostosta
`doc/tietopohja/kasikirja/toiminnot.csv` ja rekistereistä (tila ja prioriteetti luetaan
`riskiarvio.csv`:stä ja `tekninen-velka.csv`:stä) komennolla
`python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kasikirja`. Siksi **prioriteetit, tilat ja
korjauskokonaisuudet eivät ole luvun tekstissä**: ne vanhenisivat. Tekstissä on vain tunnukset. Jos
tilaa tai prioriteettia tarvitaan, katso luvun 5 taulukko, vikakortti tai portaali (kohta 1.8). Taulukon
rivi sanoo, mihin toimintoon vika on kytketty; sama vika voi olla useassa luvussa, koska se voi koskea
useita toimintoja.

### 1.8 Suhde muuhun aineistoon

| Aineisto | Mikä se on | Suhde käsikirjaan |
|---|---|---|
| **Vikakortit** (`doc/tietopohja/vikakortit/H/`, `T/`) | Selkokielinen kortti kustakin havainnosta (H) ja teknisestä velasta (T): mikä menee vikaan, milloin, mitä käyttäjä voi huomata, vaikutus kilpailuun | Käsikirjan luvun 5 rivit linkittävät kortteihin. Käsikirja kertoo toiminnon, kortti vian. Käsikirjassa ei toisteta kortin sisältöä. |
| **Rekisterit** (`doc/arkkitehtuuri/rekisteri/`) | Auditoinnin ensisijainen lähde: `havainnot.md`, `riskiarvio.csv`, `tekninen-velka.csv`, `vaikutukset.csv`, `valitsimet.csv`, `asetusavaimet.csv`, `tiedostot.csv` | Jos käsikirja ja rekisteri ovat ristiriidassa, **rekisteri voittaa**. Kortti ja käsikirja ovat johdannaisia. |
| **Auditointiraportit** (`doc/arkkitehtuuri/*.md`) | Osajärjestelmäkohtaiset analyysit (tiedonsiirto, tiedostomuodot, säikeet ja lukot, virheenkäsittely ym.) | Käsikirjan pääasiallinen lähde. Raporttien rivinumerot koskevat niiden omaa revisiota. |
| **Käyttöohje** (`docs-site/docs/`) | Käyttäjälle kirjoitettu ohje, versio 5.20 (2015) | Käsikirja viittaa siihen, mutta ei korvaa sitä. Käyttöohje voi olla vanhentunut tai ristiriidassa koodin kanssa (kohta 4). |
| **Portaali** (`doc/tietopohja/portaali/`) | Paikallinen, staattinen HTML-selain havainnoille, korteille, kysymyksille ja kattavuudelle; avataan suoraan tiedostona, ei palvelinta | Sama data kuin rekistereissä ja korteissa; käsikirjan ja korttien välinen linkitys kulkee `toiminnot.csv`:n kautta. Portaali ei ole julkinen. |
| **Asiantuntija-agentti** (`.claude/agents/pirila-expert.md`, komento `/pirila-ask`) | Vain-luku-agentti, joka vastaa kysymyksiin lähteineen ja varmuusmerkintöineen | Käyttää samaa aineistoa. Agentin vastauksissa on todettu yksittäisiä vääriä yksityiskohtia (`laatu/tarkastukset/k1-arvio.md`); tarkista lähteet ennen päätöksiä. |

Portaalin generoitu data (`portaali/data/data.js`) tuotetaan rekistereistä komennolla
`tietopohja.py`; sitä ei muokata käsin [todennettu: `k-vaihe-suunnitelma.md` luku 8.1].

## 2. Erot ohjelmien välillä

Käsikirja koskee viittä toimitettavaa ohjelmaa, ja jokaisessa luvussa on kohta "Erot ohjelmien välillä".
Lukijan kannalta tärkeintä on erottaa **kilpailutyyppi** ja **käyttöliittymä**:

| Lukijan tilanne | Aloita | Huomaa |
|---|---|---|
| Henkilökilpailu (HK: HKW ja HKM) | 01, 02, 04, 05, 06, 08, 12 | Monivaiheiset kilpailut ja vaiheen vaihto (08) ovat vain HK:ssa. |
| Viestikilpailu (viesti: VW, VM, VL) | 01, 02, 07, 04, 05, 06, 12 | Viestissä ei ole vaiheita vaan osuuksia (07). Viestin sijoitus- ja pistefunktiot on luettu vain osin (06, luku 7). |
| Kaikki, jotka ajavat usean työaseman verkkoa | 09, 03, 13 | HK:n ja viestin tiedonsiirtokoodi on osin yhteistä (`Tp/`), mutta tietorakenteet ovat eri. |
| Laitteet (lukijat, kellot, taulut) | 10, 05, 04 | Laitteiden todellista käytöstä ei ole ajettu (luku 14). |
| Asetustiedoston kirjoittaja | 12, 01 | Sama avain voi tarkoittaa HK:ssa ja viestissä eri asiaa (12). |

**Konsoli ja GUI.** Sama lähdekoodi käännetään konsoliohjelmaksi (HKM, VM, VL; MSVC; `_CONSOLE`)
ja ikkunaohjelmaksi (HKW, VW; C++Builder bcc32). Konsoli- ja GUI-versio samasta kilpailutyypistä
eivät sisällä täsmälleen samoja toimintoja eivätkä toimi kaikissa kohdissa samoin (luku 01). Jos
käsikirjan kohta ei sano, mitä ohjelmaa se koskee, kohta koskee kaikkia viittä; luvut sanovat
poikkeukset kohdassa 2.

**Kokoonpano ratkaisee.** Se, että koodissa on jokin toiminto, ei tarkoita, että toimitettu ohjelma
sisältää sen: käännösvalitsin voi jättää sen pois (esimerkiksi viestin konsolin XML-valinta tai HKM:n
seuratulokset). Käsikirja ei kuvaa kuollutta koodia toimitetun ohjelman ominaisuutena
(luku 01 ja luku 12).

## 3. Asetukset ja käännösvalitsimet

Tällä luvulla ei ole omia asetuksia. Lukijan kannalta olennaista:

- **Valitsin** (käännösvalitsin) on kytkin, jonka asento ratkeaa jo käännettäessä; se määrää, mitkä
  koodin osat ovat toimitetussa ohjelmassa. Valitsinten tila on rekisterissä
  `doc/arkkitehtuuri/rekisteri/valitsimet.csv`, ja käsikirja tarkistaa jokaisen kuvaamansa kohdan
  valitsimen sieltä ja projektitiedostoista (luku 01 ja 12).
- **Asetusavain** on asetustiedoston rivi muotoa `AVAIN=arvo`. Eri ohjelmat tuntevat osin eri avaimet
  (luku 12). Avainten luettelo on rekisterissä `rekisteri/asetusavaimet.csv`.
- **Prioriteetit, tilat ja kokonaisuudet** luetaan luvun 5 taulukosta, ei tekstistä (kohta 1.7).

## 4. Vaihtoehtoiset polut ja virhetilanteet

**Kun aineistot ovat ristiriidassa.** Käytä seuraavaa etusijajärjestystä auditointitiedoissa (ylempi
voittaa) ja ilmoita ristiriita [todennettu: `.claude/agents/pirila-expert.md` luku 2]:

1. rekisterit (`rekisteri/`), `seuranta.md` (Q), `omistajan-paatokset.md` (OP);
2. auditointiraportit (`doc/arkkitehtuuri/*.md`);
3. tietopohja (vikakortit, sanasto);
4. käyttöohje (`docs-site/docs/`);
5. lähdekoodi (`TPsource/V52/`): se on totuus siitä, mitä koodi tekee. Auditointitietoja se ei ylitä.

Käytännössä: jos käsikirja sanoo asian, jonka nykyinen koodi kumoaa, uskotaan koodia ja asia
ilmoitetaan ristiriitana. Kaikki auditoinnin "avoin" tai "korjattu" -tiedot luetaan rekisterin
sarakkeesta `tila`; koodin muutokset eivät päivity käsikirjaan itsestään.

**Ristiriitoja, jotka käsikirja on jo löytänyt**, on koottu lukuun 14 (esimerkiksi käyttöohjeen ja
koodin MONITORI-portti, `VUOROMAALI`, `COMFILE=EI` viestissä, ViestiLuennan asema).

**Kun käsikirja ei vastaa kysymykseen.** Katso ensin luvun 7-kohta ja luku 14. Jos alue on
auditoimaton, kysy asiantuntija-agentilta (`/pirila-ask`), joka lukee koodin rajatusti ja merkitsee
vastauksen perustuvan kohdennettuun lähdekoodin tarkasteluun. Käsikirja ei ole tyhjentävä.

## 5. Tunnetut viat tässä toiminnossa

<!-- generoitu:viat alku (tietopohja.py lohkot; älä muokkaa käsin) -->
Tähän lukuun ei ole kytketty toimintoja (`kasikirja/toiminnot.csv`).
<!-- generoitu:viat loppu -->

Tällä luvulla ei ole omaa toimintoa, johon viat kytkettäisiin. Viat luetaan luvuista 02-13 (kunkin
luvun kohta 5). Kattavuuden ja todentamisen rajat sekä käsikirjan avoimet kohdat on koottu lukuun 14.

## 6. Mihin tieto perustuu

- Käsikirjan tehtävänanto ja rakenne: `doc/tietopohja/laatu/k3-kirjoittajan-ohje.md`,
  `doc/tietopohja/kasikirja/_malli-luku.md`, `doc/arkkitehtuuri/k-vaihe-suunnitelma.md` luvut 7 ja 8.
- Kattavuus, todennuksen vahvuus ja rajoitteet: `doc/arkkitehtuuri/auditoinnin-yhteenveto.md` luvut 3, 4,
  6 ja 10; `doc/arkkitehtuuri/rekisteri/README.md` (analyysitasot).
- Tietopohjan rakenne ja ylläpito: `doc/tietopohja/README.md`, `doc/tietopohja/kirjoitusohje.md`,
  `doc/tietopohja/sanasto.md`.
- Aineistojen etusija ja agentti: `.claude/agents/pirila-expert.md` luvut 1-3.
- Käyttöohjeen versio ja päiväys: `docs-site/docs/viestiwin/hkkisahelp.md` (versio 5.20, 19.4.2015).
- Lähderevisio: tuotantokoodi `f7fe553`. `doc/arkkitehtuuri/CHECKPOINT.md`:n mukaan tuotantokoodi ei ole
  muuttunut sen jälkeen; käsikirjan luvut ovat 2026-11-27.
- Koodia ei luettu uutena tätä lukua varten.

## 7. Mitä ei ole selvitetty

- Käsikirjan lukuja 02-13 ja 01, 12 ja 14 ei ole tarkastettu riippumattomasti, lukuun ottamatta
  Emit-aineistoa (kohta 1.5). Niissä voi olla virheitä, joita kukaan ei ole tarkistanut.
- Ei ole selvitetty, miten tarkasti käyttöohje (versio 5.20, 2015) vastaa nykyistä koodia (versiotunnus
  koodissa 5.21). Ristiriidat on kirjattu vain siellä, missä ne tulivat vastaan.
- Ei ole selvitetty, kuinka paljon portaalin ja korttien tieto on päivittynyt käsikirjan kirjoituksen
  jälkeen: rekisteritilan muutos näkyy kortissa ja luvun 5 taulukossa vasta, kun `tietopohja.py` ajetaan.
- Ehdokkaat (ei tarkastettu): ei uusia havaintoja tässä luvussa. Haettu `havainnot.md`:stä ja
  `seuranta.md`:n KE-taulukosta: ei tähän lukuun liittyviä.
