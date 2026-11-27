# 14 Tunnetut rajoitteet ja mitä ei ole selvitetty

<!-- malli: Käsikirjan lukupohja (K-3). Ohje: doc/tietopohja/laatu/k3-kirjoittajan-ohje.md.
Tässä luvussa ei ole toimintoja, joihin vikoja kytkettäisiin; luvun 5 generointilohkoa ei ole. -->

> **Tila:** alustava K-3-aineisto, ei riippumattomasti tarkastettu. Lähderevisio: tuotantokoodi
> `f7fe553` (ennallaan, `doc/arkkitehtuuri/CHECKPOINT.md`). Varmuusmerkinnät: [todennettu],
> [päätelty], [tuntematon]; "luettu käsikirjaa varten" = luettu koodista K-vaiheessa, ei tarkastettu.

Luku kokoaa yhteen kaiken, mitä käsikirjan muut luvut eivät voi sanoa varmasti: auditoinnin kattavuuden ja
todennuksen rajat, avoimet kysymykset, kirjoittajien ehdokkaat, aineistojen väliset ristiriidat ja omistajan päätökset,
joista tiedon käyttö riippuu. Luvun tarkoitus on estää, että käsikirjan puute luetaan toiminnon virheettömyydeksi tai
tunnetun vian puute vian puuttumiseksi. Luku ei etsi uusia vikoja eikä anna toimintaohjeita. Polut ovat suhteessa
hakemistoon `TPsource/V52/`, ellei polku ala `doc/`. Luvut ovat päivämäärältä 2026-11-27; rekisterit ja `seuranta.md` ovat
ensisijaisia, joten tarkista tilatieto niistä (lukuohje 00, kohta 1.7).

## 1. Mitä tapahtuu

### 1.1 Auditoinnin kattavuus

Auditointi ei kata koko lähdekoodia [todennettu: `doc/arkkitehtuuri/auditoinnin-yhteenveto.md` luku 3]. Rekisteri
`rekisteri/tiedostot.csv` seuraa 650 lähdetiedostoa neljällä analyysitasolla:

| Taso | Merkitys |
|---|---|
| 1 | Inventoitu skriptillä: koko, merkistö, projektit, include-kaavio. **Koodia ei ole luettu.** |
| 2 | Kohdennettu: tiettyjä funktioita tai rivejä luettu tiettyä väitettä tai löydöstä varten |
| 3 | Rakenne: kaikki funktiot tunnistettu ja keskeiset luettu; tiedoston rooli dokumentoitu |
| 4 | Katselmoitu: koko tiedosto luettu ja tarkistettu vikojen varalta |

[todennettu: `rekisteri/README.md`.]

| Hakemisto | Tiedostoja | Rivejä | Taso 1 (tiedostoa / riviä) | Taso 2 | Taso 3 | Taso 4 |
|---|--:|--:|--:|--:|--:|--:|
| `Hk` | 27 | 46 032 | 6 / 728 | 18 / 41 418 | 3 / 3 886 | 0 |
| `Juk` | 23 | 38 605 | 3 / 331 | 20 / 38 274 | 0 | 0 |
| `Tp` | 13 | 11 704 | 7 / 498 | 5 / 8 021 | 1 / 3 185 | 0 |
| `com` | 3 | 1 807 | 0 | 2 / 409 | 1 / 1 398 | 0 |
| `dbboxm` | 13 | 2 375 | 6 / 1 333 | 5 / 603 | 2 / 439 | 0 |
| `include` | 21 | 1 883 | 13 / 823 | 5 / 775 | 3 / 285 | 0 |
| `tputilv2` | 169 | 15 513 | 143 / 10 549 | 23 / 4 546 | 3 / 418 | 0 |
| `HkConsole` | 8 | 9 605 | 4 / 1 989 | 4 / 7 616 | 0 | 0 |
| `JukConsole` | 10 | 8 271 | 5 / 1 121 | 5 / 7 150 | 0 | 0 |
| `cbHk` | 149 | 67 530 | 127 / 38 996 | 18 / 19 605 | 4 / 8 929 | 0 |
| `ViestiWin` | 114 | 43 424 | 94 / 23 082 | 15 / 13 011 | 5 / 7 331 | 0 |
| `cbTp` | 73 | 15 115 | 63 / 11 170 | 10 / 3 945 | 0 | 0 |
| `RADStudio10`, `VS`, vanhat `Tests` | 8 | 8 186 | 3 / 7 336 | 5 / 850 | 0 | 0 |
| **Yhteensä (tuotanto)** | 631 | 270 050 | 474 / 97 956 | 135 / 146 223 | 22 / 25 871 | 0 |

[todennettu: `auditoinnin-yhteenveto.md` luku 3; ryhmä "RADStudio10, VS, vanhat Tests" on rivien `RADStudio10` (2 tiedostoa),
`VS` (1) ja `Tests`-vanhat (5) summa. Lisäksi 19 C-11-testikehyksen tiedostoa (3 129 riviä) on tasolla 4; ne ovat auditoinnin
omia testilähteitä, eikä niitä ole tarkastettu virheiden varalta.]

Tulkinta [todennettu: `auditoinnin-yhteenveto.md` luku 3]:

- **36 % riveistä** (97 956 riviä, 474 tiedostoa) on tasolla 1, eli koodia ei ole luettu. Ilman kolmannen osapuolen
  `Tests/doctest.h`-tiedostoa luku on 90 850 riviä. GUI-lomakehakemistot (`cbHk`, `ViestiWin`, `cbTp`) muodostavat tästä
  284 tiedostoa ja 73 248 riviä (81 %); `tputilv2` 143 tiedostoa ja 10 549 riviä.
- **Taso 2 on kohdennettu.** Sen 146 223 riviä ovat kokonaisten tiedostojen kokoja, eivät luettuja rivejä. Neljä
  suurinta tiedostoa (`Juk/VTulostus.cpp`, `Hk/HkTulostus.cpp`, `Hk/HkEmit.cpp`, `Juk/VEmit.cpp`) luettiin löydöskohtien mukaan.
- **Yksikään tuotantotiedosto ei ole tasolla 4.** Taso 3 kattaa 25 871 riviä (9,5 %) 22 tiedostossa.
- Yhdelläkään hakemistolla ei ole pelkkää tasoa 1; lähimpänä ovat `VS`, `cbTp` (74 % riveistä tasolla 1), `tputilv2` (68 %),
  `cbHk` (58 %) ja `ViestiWin` (53 %).
- Rekisterin aukko: `cbHk/UnitCfg.cpp` (2 936 riviä) ja `ViestiWin/UnitCfg.cpp` (2 352) ovat tasolla 1, vaikka C-12 jäsensi
  niiden avaimet skriptillä (luku 12).
- **Löydettyjen vikojen määrä on alaraja**, ei kokonaismäärä. Se, ettei toiminnolle ole korttia tai vikaa, ei tarkoita, että
  toiminto on virheetön (`doc/tietopohja/README.md`).

**Tutkimatta jääneet alueet** (erä kerrallaan kirjatut syyt) [todennettu: `auditoinnin-yhteenveto.md` luku 3]:

| Alue | Tila |
|---|---|
| Arvonta-algoritmi `cbHk/HkArv2.cpp` (1 274 riviä, taso 1) | rajattu pois erästä C-7; ei aloitettu |
| 41 muuta lomaketiedostoa, jotka kirjoittavat kilpailudataa | ei aloitettu |
| 152 jäsennyskandidaattia (lomakkeet 96, ydin 56) | ei aloitettu |
| 153 funktiostaattista muuttujaa `Hk/`- ja `Juk/`-hakemistojen ulkopuolella | luetteloitu, ei luokiteltu |
| 161 kokonaisluvun jäsennyskandidaattia (9 selvitetty, 152 jäljellä) | ei luettu; sama joukko kuin 152 kandidaattia |
| Noin 93 % kääntäjävaroituksista (6 569 / 7 030) | seulomatta; bcc32:ssa ei muoto- eikä rajatarkistusta |
| Ajonaikaiset kohteet (laitteet, verkko, SQL, bcc32-GUI-ajo) | ei ajettu |

### 1.2 Todennuksen vahvuus ja rajat

Lähdeviitteen validointi (validaattori) ja tekninen todennus ovat eri asioita: validaattori tarkistaa vain, että tiedosto on
olemassa ja rivi on tiedoston sisällä, eikä vertaa sisältöä [todennettu: `auditoinnin-yhteenveto.md` luku 4]. Auditoinnin
todistelajit ovat staattinen luku, riippumaton lähdetarkastus (toinen agentti, ei ihmistä, sama mallisuku), mekanismin toisto
testikehyksessä ja vahvistus toimitetussa ohjelmassa (lukuohje 00, kohta 1.4).

Todennusluokat (yksi vahvin todiste kutakin löydöstä kohti) [todennettu: `auditoinnin-yhteenveto.md` luku 4]:

| Luokka | Merkitys | Löydöksiä |
|---|---|--:|
| a | kumottu | 1 (H-099) |
| b | vahvistettu toimitetussa ohjelmassa | 1 (H-041) |
| c | toistettu eristetyllä kokeella tai testikehyksessä (MSVC-konsolirakennelmat) | 26 |
| d | riippumaton lähdetarkastus, täysin vahvistettu | 89 |
| e | riippumaton tarkastus, osittain vahvistettu | 43 |
| f | tarkastajan oma löytö | 3 |
| g | yhden agentin staattinen löydös, ei koskaan tarkastettu | 13 |
| h | ei ratkaistavissa staattisesti (H-020, tarvitsee linkityskartan) | 1 |

Luvut ovat vaiheen C sulkemisen (C-13) lukuja, jolloin H-havaintoja oli 177 (176 avointa, 1 kumottu) ja T-kohteita 52; rekisterit
ovat ensisijaisia ja saattavat olla päivittyneet.

**Auditoinnin 13 rajoitetta** (L1-L13) rajaavat johtopäätösten luotettavuutta [todennettu: `auditoinnin-yhteenveto.md` luku 6]:

1. Vain H-041 on vahvistettu toimitetussa ohjelmassa (`HkMaali.exe`, debuggerissa, rajaparilla 220/221), ja sekin käyttäjän
   hyväksymällä poikkeamalla ennalta lukitusta tuloksesta; lukupolkua ja 250-tapausta ei ajettu.
2. Kaksikymmentäkuusi muuta löydöstä on toistettu vain eristetyissä MSVC-konsolirakennelmissa, joissa tila on asetettu käsin.
   Se osoittaa mekanismin, ei sitä, että laukaisija esiintyy käynnissä olevassa ohjelmassa.
3. bcc32-todisteet rajoittuvat rakennekoko- ja offset-kokeisiin, pragma-tilan jäljitykseen, objektitiedostojen vertailuun,
   RTL-konsolikokeisiin ja varoituksiin. Yhtään bcc32-GUI-ohjelmaa ei ole ajettu.
4. Laite-, verkko-, SQL- tai usean aseman ajoa ei ole. Laiteprotokollat, MySQL, emiTag HTTP ja Emit-palvelin on pääteltävissä
   vain koodista ja dokumentaatiosta.
5. GUI-lomakkeet ovat suurelta osin lukematta (284 tason 1 tiedostoa; vain 8 lomakeyksikköä luettiin liiketoimintalogiikan osalta).
6. C-10:n ja C-12:n makroarvioijat ovat leksikaalisia: lausekemakrot tallentuvat arvolla 1 (esimerkiksi viestin `MAX_PORTTI`
   näkyy arvona 1, vaikka todellinen arvo on 132). Tallennettuja arvoja ei saa käyttää tosiasioina.
7. "Riippumaton tarkastus" tarkoittaa toista agenttia: 13 löydöstä ei tarkastettu lainkaan, 43 vain osittain, 3 on tarkastajan
   löytöjä.
8. Lähdeviitteiden validointi ei ole tekninen todennus.
9. Rekisteri seuraa tasoa vain 650 lähdetiedostolle; `.dfm`-, projekti- ja CI-tiedostoilla ei ole tasoa.
10. Taso 2 on "kohdennettu": suurimmat tiedostot luettiin löydöskohtien mukaan.
11. Kaikki on sidottu revisioon `f7fe553` ja paikallisiin rakennelmiin 2026-11-24; CI käyttää työkaluversiota v143, paikallinen v145.
12. Noin 93 % kääntäjävaroituksista jäi seulomatta, eikä varoituksen puuttuminen ole todiste: tunnetut puskurivirheet H-041,
    H-042, H-052, H-055 ja H-060 eivät tuottaneet varoitusta.
13. Korjausprioriteetti oli ennen C-13:a asetettu vain osalle löydöksistä; C-13:n yhtenäinen riskiarvio antoi prioriteetin kaikille
    avoimille.

Lisäksi auditoinnin oma kokonaisarvio toteaa, että auditoinnin tarkastuksissa **noin neljännes - puolet tarkastetuista väitteistä
tarkentui**, ja että prioriteetit ovat yhden arvioijan harkintaa yhtenäisin perustein, eivät mittaustuloksia
[todennettu: `auditoinnin-yhteenveto.md` luku 10]. Saavutettavuus ja seuraukset ovat suurelta osin pääteltyjä.

### 1.3 Mitä käsikirja kattaa ja mitä ei: lukujen 7-kohdat koottuna

Käsikirjan luvut ovat alustavia: niitä ei ole tarkastettu riippumattomasti, lukuun ottamatta Emit-aineistoa
(`aineisto/emit-luenta.md`, tarkastus `laatu/tarkastukset/emit/tarkastus.md`) (lukuohje 00, kohta 1.5). Alla on lukujen
7-kohtien keskeinen sisältö; yksityiskohdat ovat kunkin luvun kohdassa 7.

| Luku | Keskeiset selvittämättömät kohdat |
|---|---|
| 01 Ohjelmat | ViestiLuennan asema (käyttöohje vs. julkaisu); `GAZ`-valitsimen merkitys viestissä; kuolleiden valitsinten tarkoitus (OP-10); bcc32-GUI:n ajonaikainen käytös; HK:n ja viestin työasemien yhteistoiminta |
| 02 Elinkaari | arvonta-algoritmi (`HkArv2.cpp`) ja uudelleenarvonta [tuntematon]; sarja- ja perustamislomakkeet (taso 1); ilmoittautumisten tuonti (IRMA, Kilmo, CSV); VM:n ja VL:n kilpailijoiden ja lähtöaikojen käsittely [tuntematon]; Q75-Q80; KE-2, KE-5, KE-7 |
| 03 Tiedostot | virtakatko kirjoituksen aikana (Q27); `CTRL_CLOSE` (Q73); verkkolevyn kahva katkon jälkeen (Q87); levypuskurittomuus käytännössä (Q90); AJAT-rivien käyttö (Q94); viestin vastaanottopolku ja VL:n tiedostokäytännöt; automaattisten varmuuskopioiden puuttuminen päätelty, ei kattavasti haettu |
| 04 Ajanotto | maalikellon todellinen käytös (Q86, Q100, OP-13); päivän vaihtuminen; AJAT-rivit (Q94); onko viestin ajanottopaneeli täysin sama VM:ssä ja VW:ssä (Q76); `tall_ec`, `vahaku`, emiTag-palvelinhaku vain pääpiirteissä |
| 05 Leimaukset | `tarkista`-säännön rajatapaukset ei ajettu; onko aika- tai rastiväliraja-tarkistusta; `vaihda_badge`; hylkäyslomakkeen XML; viestin osuuden valinta (`val_korkein`, joustoviesti, partio); kuntosuunnistustilat (Q75, OP-9); KE-10, KE-11, KE-12; Q86, Q100 |
| 06 Tuloslaskenta | viestin sijoitus- ja pistefunktiot luettu vain nimiltään [tuntematon]; rinnakkaisosuuksien tulos; pyöristysindeksien vastaavuus käyttöohjeeseen; erikoislajien laskentapolut; konsoliohjelmien tulosten muutostoiminnot [tuntematon]; Q75, Q76, Q82; KE-5, KE-9 |
| 07 Viestit | partio ja rogaining (lomake, `tark_partio`, laskenta); rinnakkaisosuuksien käyttö kilpailuissa; luennan testitila; lähtölaji 2; VM:n `SISÄÄNLUENTA`; `VUOROMAALI`-ristiriita; Q76, Q77, Q111; mikä `150os`-kokoonpano on toimitettu |
| 08 Vaiheen vaihto | OP-6 (tuettu käyttötapa), Q78 (`n_lisaa`), Q67, Q80, Q68, Q110, Q82; H-129:n kekoylivuoto ja `VAIHE=V`-polku yhdessä ajamatta; KE-2, KE-21 |
| 09 Tiedonsiirto | kahden aseman toimintaa ei ajettu; `HYVKAIKKI`- ja `BRO`-käytäntö tuntematon; `COMFILE=EI` viestissä; Q19-Q23 ja Q90 (Q17, Q18 ja Q24 on ratkaistu, katso kohta 1.6) |
| 10 Laitteet | laitteiden todellinen käytös (Q86, Q100, OP-13); Impinj-silta ja RADIOEMIT-laitteen alkuperä; Q96; TCP-lukijan valvonta; näyttötaulujen vastapään käytös; tulostimet; `VALOT`/`VALONÄYTTÖ`; Q81 |
| 11 Julkaisu | Q91-Q93, Q95, Q97-Q98, Q104-Q106, Q108; HK:n `autofile` ilman `AUTOFILE.LST`; automaattitulosteiden ja SQL-peilin yleisyys kilpailuissa; mikään väite ei perustu kokeeseen oikealla kilpailulla, palvelimella tai MySQL:llä |
| 12 Asetukset | Q107, KE-8; Q112 ja Q25 (OP-11); noin 80 vähäisen avaimen merkitys; asetuseditorien kenttäkohtainen toiminta (taso 1) |
| 13 Virhetilanteet | Q73, Q85-Q87, Q89-Q95; H-152 siirretty myöhempään testausvaiheeseen; koottua palautumisohjetta ei löytynyt; KE-5, KE-6, KE-13 |

### 1.4 Avoimet kysymykset (Q)

`doc/arkkitehtuuri/seuranta.md` sisältää kysymykset Q1-Q112. Vaiheen C sulkemisessa 84 kysymystä ei ollut tilassa
"ratkaistu" (72 avointa, 6 osittain ratkaistua, 3 omistajan päätöstä ja 3 ajonaikaista todennusta); luokittelu on ehdotus
[päätelty: `auditoinnin-yhteenveto.md` luku 6]:

| Luokka | Merkitys | Kysymykset |
|---|---|---|
| 1 | ratkaistu: dokumentit osoittavat vastauksen (4) | Q3, Q24, Q26, Q53 |
| 2 | ajonaikainen todennus vaiheessa D (39) | Q12, Q21, Q28, Q33-Q37, Q39, Q40, Q43-Q46, Q48, Q52, Q56, Q63, Q67, Q68, Q73, Q80, Q86, Q87, Q89, Q91, Q93, Q95-Q98, Q100, Q102, Q103, Q105-Q108, Q110 |
| 3 | omistajan päätös (16) | Q9, Q10, Q11, Q25, Q29, Q47, Q60, Q75, Q77, Q78, Q81, Q90, Q99, Q101, Q111, Q112 |
| 4 | hyväksytty avoimeksi: staattinen, ei tarvita ennen korjauksia (25) | Q6, Q14, Q15, Q19, Q20, Q22, Q23, Q27, Q31, Q32, Q38, Q41, Q42, Q49-Q51, Q54, Q55, Q64, Q76, Q79, Q82, Q94, Q104, Q109 |

[todennettu: `auditoinnin-yhteenveto.md` luku 6; luokan 2 tunnukset on luettu taulukon merkinnöistä ja yhdistetty välit.]
Seuranta voi olla edennyt: tarkista tila `seuranta.md`:stä. Luokan 1 kysymyksiä ei ollut päivitetty rivitasolle; kysymyksissä
Q14, Q31 ja Q42 C-3- ja C-4-aineisto voi jo vastata kysymykseen, ketjua ei jäljitetty [tuntematon].

**Kysymysten sisältö.** Yksittäisten Q-kysymysten sisältö on `seuranta.md`:n taulukossa ja lukujen 7-kohdissa.
Kysymykset, joiden vastaus riippuu **käytännön kilpailutoiminnasta** (OP-14): Q29, Q47, Q81, Q90 (esimerkiksi ajetaanko
kilpailuja ilman levypuskuria, ajetaanko useita lukijoita yhdellä HK-instanssilla).

### 1.5 Kirjoittajien ehdokkaat: KE-taulukko

`seuranta.md` sisältää taulukon "Vaiheen K ehdokkaat ja tarkistuskohteet" (KE-1-KE-23). Vaihe K ei ole auditointi: kohdille ei
anneta H-tunnusta, riskiarviota ei muuteta, eikä niitä ole tarkastettu. Käsittely on suunniteltu riskiseulontaan E-1 tai korjauksen
yhteyteen; E-1:tä ei ole aloitettu. Alla on tiivistelmä [luettu 2026-11-27; varmuus kunkin rivin omasta sarakkeesta].

| KE | Koskee | Mitä | Varmuus |
|---|---|---|---|
| KE-1 | H-033 | `unpack`-haarassa väliajat kirjoitetaan alueen ulkopuolisen olion osoittimen kautta; kirjoitus voi osua mielivaltaiseen osoitteeseen | päätelty koodista, ei ajettu |
| KE-2 | H-129 | toisen työaseman vaiheenvaihtopyyntö käynnistää vaihdon ilman vahvistusta ja toistettuna kasvattaa hakemiston laskuria; riskiarvion rivi ristiriidassa | päätelty |
| KE-3 | H-145 | riskiarvion saavutettavuus `normaalikaytto`, vaikka vika vaatii emit-tiedoston kirjoitusvirheen; asteikossa ei ole arvoa ympäristövirheelle | dokumenttien ristiriita |
| KE-4 | H-041 | ylivuotava osa on viimeisen vaiheen väliaikatietoa; voiko siihen päätyä verkosta tai laitteelta tullutta tietoa | päätelty; alkuperää ei jäljitetty |
| KE-5 | H-109 | HK:n `GETREC` antaa virheilmoituksen tietuenumerolla -1, mutta `deltall` jatkaa nollatulla tietueella | luettu koodista, ei tarkastettu |
| KE-6 | H-093 | lokisäikeen pino 40 kt, muiden 1 Mt; kirjoittaako lokisäie itse lokia | avoin kysymys |
| KE-7 | H-133 | `initindex` ei nollaa `idindex->nrec`iä | päätelty (C-8:n tarkastus) |
| KE-8 | H-171 | alustamaton `baud`-arvo sarjayhteyden avauksessa, kun `baudst` on nollattu; käyttävätkö laitepolut `baudst`-taulua | ratkaisematta |
| KE-9 | - | `cbHk/UnitPistelasku.cpp`: pilkkuoperaattori tekee ehdosta aina epätoden, joten yhdistelmäsarjoihin ei ilmeisesti valikoidu ketään; tiedosto auditoimaton | päätelty kahden lukijan toimesta, ei ajettu |
| KE-10 | H-145 (läheinen) | emit-käsittelyssä tulos tallennetaan ja kilpailijapaketit jonotetaan ennen emit-tietueen uudelleenkirjoitusta; jos kirjoitus epäonnistuu, vertaisilla on tulos ilman emit-tietuetta; sama VM:ssä, VW tekee linkityksen ennen kirjoitusta | päätelty koodista |
| KE-11 | H-137 (läheinen) | uudelleenkäynnistyksen tai emit-tiedoston uudelleenluvun jälkeen käsittelemättömät paikalliset emit-tietueet, joita seuraa vertaiselta tullut käsitelty, jäävät automaattikäsittelyn ulkopuolelle | päätelty koodista |
| KE-12 | - | HKW: kun käsittelyjono kohtaa jo käsitellyn emit-tietueen, lomake näyttää tyhjän "Virhe luennassa" -ikkunan | päätelty; vaatii kahden aseman ajon |
| KE-13 | H-162 | rekisterin kuvaus "emit-käsittely pysähtyy" on ristiriidassa koodin kanssa: lukko on uudelleentuleva ja `pros_emit` ajetaan pääsäikeessä, joten pysähtyy vastaanotto (`tarkcom`) | päätelty; rekisteriä ei muutettu |
| KE-14 | H-073 | riskiarvion perustelu "tavallinen data" ei pidä: ylitys vaatii noin 34 (äärimmäiset arvot) tai noin 45 rastia (tavalliset) | laskettu koodista, ei ajettu |
| KE-15 | H-008 | lukko jää päälle myös oletusasetuksella, kun sanoma tulee ampumalaitteelta; sanoma välitetään muille asemille; riskiarvion saavutettavuus ja peruste ovat väärin | luettu koodista, ei ajettu |
| KE-16 | useita (mm. H-015, H-019, H-044, H-145, H-154) | järjestelmällinen: riskiarvion saavutettavuus `normaalikaytto` vioille, jotka vaativat ympäristövirheen (levy täynnä, verkkolevyn katko, tulostinvirhe) | dokumenttien ristiriita |
| KE-17 | useita (mm. H-009, H-022, H-027, H-041, H-093) | järjestelmällinen: riskiarvion `tietoturva = ei` ilman perustetta, vaikka laukaisija voi tulla verkosta tai laitteelta | vaikutusrivit `ei_tiedossa` |
| KE-18 | H-028 | `tcpip_CS` on kaikkien soketien yhteinen lukko, joten sen jääminen pysäyttää kaiken TCP-liikenteen; rekisteri ja riskiarvio kuvaavat sen lähetyslukoksi | luettu koodista |
| KE-19 | H-027 | jo muodostetulle asiakasyhteydelle on uudelleenyhteyspolku, vain epäonnistunut ensimmäinen avaus on pysyvä; riskiarvio kuvaa katkon pysyväksi | luettu koodista, ei ajettu |
| KE-20 | H-141 | kaksoiskorttien eston laskurivirhe on myös viestissä | päätelty |
| KE-21 | H-129 | HKW:n valikko "Lue sarjat ja muut tiedot uudelleen" kutsuu vaiheen vaihdon funktiota nykyisellä vaiheella ja lähettää pyynnön muille, joten H-129 koskee myös yksivaiheista kilpailua | päätelty |
| KE-22 | useita | kokoomarivi: noin 15 uutta ehdokasta, ei tarkastettu (alla) | päätelty tai luettu koodista |
| KE-23 | - (asetus `LÄHTÖIKKUNA`, HK) | `LÄHTÖIKKUNA=x` luetaan millisekunteina (`Hk/HkInit.cpp:1924`; oletus 3000 ms, `Hk/HkGlobals.cpp:96`), käyttöohjeen mukaan sekunteina; `LÄHTÖIKKUNA=3` tarkoittaisi 3 ms | luettu koodista, ei ajettu; riippumaton tarkastus K-5 vahvisti luvun 04 osalta |

**KE-22:n ehdokkaat** (kokoomarivi `seuranta.md`:ssä; ei rekisterissä, ei tarkastettu): ViestiWinin `MTRn`/`EMITAGn`/`ETGPRSn`
lukevat laitenumeron yhdestä merkistä; `kirjSQLrivi` palauttaa `false` ja jonon häntä siirtyy; sarja- ja TCP-polulla pituudeltaan 0
ilmoitettu määrittelemätön luokka voi läpäistä tarkistuksen; `kirjsarjat`-muotoilu ilman välilyöntiä (ampumapaikat);
`TFormSelect3`:n ajastin vapautettuun lomakkeeseen; HK:n `BtnTall2Click` `(char)_wtoi` (pisteet yli 127);
H-070:n tarkistussumman luku voi ylittää 40 kt:n pinon; H-094: NULL-kirjoitus, kun uusi käsittelysäie ajaa `close_bdg_t`:n ennen
ensimmäistä korttia; `kilptietue::tarkstr` tyhjentää vain osoittimen koon verran; `Juk/vSarjat.cpp`:n `strcpy(sarjanimi, s)` rajaamatta;
`ViestiWin/UnitKilpMaaritys.cpp`:n puuttuva `else` (`partio`); juoksujärjestyksen vaihto ilman yksikäsitteisyystarkistusta;
vaiheen vaihtopyyntö lähetetään vain, kun lähettävällä asemalla on levyjono käytössä (`levypusk` on koko ohjelman asetus, ei yhteyskohtainen; `COMFILE=EI`-asemalta pyyntö ei lähde millekään yhteydelle; kehys korjattu riippumattomassa tarkastuksessa, `Hk/HkIx.cpp:1974-1981`); `openprfile` ilman oletustulostinta avaa tiedoston `PRN`;
`kirjemvaloki`: `ansitowcs(st, rata, 39)` kenttään `rata[8]`.

**Lukujen 7-kohtien ehdokkaat, jotka eivät ole KE-taulukossa.** Seuraavat kirjoittajat kirjasivat ehdokkaiksi, eikä niitä löytynyt
`havainnot.md`:stä eikä KE-taulukosta; ne ovat lukemisen sivuhavaintoja, eivät tarkastettuja havaintoja, eikä niille ole tunnusta:

- **UDP:n väärän paketin numeron resynkronointi** (`initid`): väärän numeron paketti asettaa merkin, jonka jälkeen seuraava paketti
  hyväksytään uutena lähtökohtana; ohitettujen pakettien menetyksen mahdollisuus on päättelemättä (`Tp/TpComY.cpp:1008-1036`;
  luku 09) [luettu käsikirjaa varten, ei tarkastettu].
- **XML-TCP:n kuittaus:** jono etenee soketille kirjoituksen onnistuessa ilman vastapään kuittausta; katkotilanteessa paketteja
  voi siirtyä jonosta pois ilman, että vastapää on saanut ne (H-027:n pysyvä katko koskee vain epäonnistunutta ensimmäistä avausta, KE-19) (`Hk/HkCom32.cpp:1058-1074`; luku 09) [päätelty].
- **"Vain muuttuneet" -valinta** (`fprior`): jos laskuri ei kasva, valinta voi jättää sarjat kirjoittamatta (`Hk/HkTulostus.cpp:7264`,
  `Juk/VTulostus.cpp:7253`; luku 11) [luettu käsikirjaa varten, ei tarkastettu].
- **XML-automaatti monivaiheisessa kilpailussa:** `xmlots` kutsutaan kahdesti, `xmlloppu` kerran (`Hk/HkTulostus.cpp:7140-7141`, `:7314-7324`;
  luku 11) [luettu käsikirjaa varten, ei tarkastettu].
- **Aikajonon rivi tallennetaan ennen kilpailijaa** (luvut 04, 05, 10; K-3-huomiot) [ei haettu rekisteristä].
- **Käytös, joka ei ole vika:** HKM luo tyhjän `KILP.DAT`:n väärästä kansiosta käynnistettäessä ilmoituksella; ei tarkastettu,
  onko ilmoitus näkyvillä riittävän kauan (luku 02) [ei kirjattu havainnoksi].
- **Käytös, joka on huomioitava:** `hyv_pv` laskee tarkastamattoman (`-`) ja hylkäysesityksen (`I`) saaneen hyväksytyksi, joten
  hyväksyttyihin rajautuva tulosluettelo sisältää myös ne; tarkoitettu käytös [päätelty koodista, luku 06].

### 1.6 Aineistojen väliset ristiriidat

Seuraavat ristiriidat ovat tulleet vastaan käsikirjaa kirjoitettaessa; useimpia ei ole ratkaistu. Etusija (lukuohje 00, kohta 4):
rekisterit, raportit, kortit, käyttöohje, koodi; koodi on totuus siitä, mitä se tekee.

| Ristiriita | Osapuolet | Tila |
|---|---|---|
| MONITORI-oletusportti | koodi 14770; käyttöohje (`hkkisawin/hkliite1.md`) 14900 (T-014) | omistajan päätös (Q25, OP-11) |
| `VUOROMAALI` | käyttöohje (viestiwin 5.4) sanoo parametrin kieltävän ajan antamisen vaihdolle tai maalille; koodi valitsee pisteeksi 0, kun väliajat ovat täynnä (`Juk/VInit.cpp`, `Juk/VIv.cpp`) | ratkaisematta [tuntematon] |
| `COMFILE=EI` viestissä | käyttöohje (`viestiwin/hkliite1.md`) listaa; viestin jäsennin ohittaa (kommentoitu, `Juk/VInit.cpp:1218`) | ratkaisematta |
| ViestiLuennan asema | käyttöohje: ei enää versiossa 5.20; julkaisutyönkulku rakentaa ja paketoi (`.github/workflows/release.yml`) | OP-12 |
| `GAZ`-valitsin | `valitsimet.csv`: HK-konsoli, viestissä ei käyttäjää; VM:n ja VW:n projektit määrittelevät, VW:ssä on `UnitGAZ.cpp` | ratkaisematta |
| `EMITAG` ViestiWinissä | valitsin määritelty vain osassa kokoonpanoja eikä testata missään `#if`-ehdossa | ei vaikutusta toimintaan [todennettu: `ViestiWin.cbproj:195` määrittelee valitsimen; haku `#if`/`#ifdef`-ehdoista `Juk/`, `Hk/`, `ViestiWin/` ei löydä `EMITAG`-testiä; luettu käsikirjaa varten] |
| H-056:n ohjelmat | riskiarvio listaa viisi ohjelmaa; ARES-avain on vain HK:ssa | ratkaisematta |
| `sarjatietue.luonne` | kommentti (`Hk/HkDef.h`) vs. koodin negatiivisen arvon käsittely | ratkaisematta |
| Käyttöohjeen tiedosto 6.14 | tiedoston otsikko on "6.15" | käyttöohjeen sisäinen epäjohdonmukaisuus (K-3-huomiot, luku 02) |
| `IKKUNAOTS=` | käyttöohje: teksti sellaisenaan; viestin jäsennin muuttaa isoiksi ja katkaisee (H-176) | rekisterissä |
| `peruscfg.cfg` | koodi lukee ensin; käyttöohjeessa ei mainita | käyttöohjeen aukko |
| Luku 09, kohta 7 | luettelee Q17, Q18 ja Q24 avoimina, mutta `seuranta.md` merkitsee ne ratkaistuiksi (C-3, C-13) | käsikirjan lukua ei ole päivitetty |
| H-162 | rekisterin kuvaus vs. koodi (KE-13) | rekisteriä ei muutettu |
| Riskiarvion rivit | KE-2, KE-3, KE-14-KE-19: saavutettavuus, tietoturva ja peruste | E-1:ssä |
| `CLAUDE.md` | polku `TPexe\V\JukMaali.exe` ei vastaa nykytilaa (`TPexe/Juk/V52/`) | `yleiskuva.md` |
| Käyttöohjeen versio | käyttöohje 5.20 (2015); koodi 5.21 | lukuohje 00 |

### 1.7 Omistajan päätökset OP-1 - OP-18

`doc/arkkitehtuuri/omistajan-paatokset.md` kokoaa asiat, jotka vaativat ohjelmiston omistajan tai ylläpitäjän päätöksen tai tiedon.
**Yhtäkään päätöstä ei ole tehty tässä asiakirjassa.** Tilanne luettu 2026-11-27: kaikkien 18 kohdan kenttä "Tila / status" on
**avoin**; mikään ei ole "päätetty" [todennettu: `omistajan-paatokset.md`; asiakirjan otsikon tila on 2026-11-26].
Päätösten tila ei ole luvun muissa osissa; tarkista se tiedostosta.

| Tunnus | Asia | Liittyvät kohdat | Tila | Viimeistään |
|---|---|---|---|---|
| OP-1 | Auditointitulosten julkaisu, push ja suojausohjeet | - | päätetty 2026-11-27: (a) julkaistaan ennen korjauksia | ennen D-vaiheen alkua |
| OP-2 | Testi- ja CI-käytäntö | Q35, Q40 | avoin | aalto 0 |
| OP-3 | C++-standardi tuotantokoodissa | Q9 | avoin | aalto 1 (oletus riittää) |
| OP-4 | Yhteensopivuuspolitiikka: sekaversiot, tiedosto- ja protokollamuutokset | H-016, H-029, T-019, T-043 | avoin | aalto 2; vaihe 4 |
| OP-5 | FILESEND-tiedostonsiirron rajaus | H-071 | avoin | aalto 1 |
| OP-6 | Vaiheen vaihdon tuettu käyttötapa ja nimeäminen | Q67, Q78, Q80; H-131-H-133 | avoin | aalto 1-2 |
| OP-7 | MySQL-peilin asema, tuonti ja jakelu | H-165, H-169 | avoin | aalto 2 |
| OP-8 | Kolmannen osapuolen lähdekoodin ehdot | Q101, T-047 | avoin | ennen seuraavaa julkista julkaisua |
| OP-9 | Ominaisuuksien ja käytöksen tarkoituksellisuus | Q60, Q75, Q111; H-158, H-172; H-116, H-109, H-111 | avoin | aalto 2-4 |
| OP-10 | Kuolleen koodin, orpojen tiedostojen ja valitsinten poisto | Q10, Q11, Q111 (osa 3) | avoin | aalto 4 |
| OP-11 | Asetusavainten kanoninen muoto ja yksiköt | Q112, Q25; H-173, H-176 | avoin | aalto 3 |
| OP-12 | Tuetut tuotteet, kokoonpanot ja käyttöjärjestelmät | T-051, T-024, T-017, T-052 | avoin | vaihe 2 |
| OP-13 | Laitteet ja ulkoiset palvelut | Q86, Q99, Q100, Q44, Q56 | avoin | aalto 1 (testit), aalto 3 |
| OP-14 | Kentän käyttötavat (tietopyyntö) | Q29, Q47, Q81, Q90 | avoin | ei estä; tarkentaa järjestystä |
| OP-15 | Merkistökäytäntö | H-004, H-005, T-027 | avoin | aalto 4 |
| OP-16 | HK- ja viestihaarojen lähentämisen tavoite | Q77, T-034 | avoin | vaihe 4 |
| OP-17 | Modernisoinnin selvityskohteet | T-046 | avoin | vaihe 5 |
| OP-18 | Korjausjulkaisujen käytäntö | T-025, T-044, Q98 | avoin | ennen ensimmäistä D-julkaisua |

Käsikirjan lukujen kannalta olennaisimmat: **OP-1** (julkaisu: päätetty 2026-11-27, aineisto julkaistaan ennen
korjauksia; jokainen julkaisutoimi hyväksytään erikseen), **OP-6**
(luku 08), **OP-9** (luvut 05, 06, 11, 12), **OP-11** (luku 12), **OP-12** (luvut 01, 12), **OP-13** (luvut 04, 05, 10) ja **OP-14**
(käytännön käyttötavat). Odottavatko päätökset korjausta, katso `korjaussuunnitelma.md` ja `rekisteri/korjauskokonaisuudet.csv`.
Käsikirja ei tee eikä oleta päätöksiä: kun se sanoo "omistajan päätös", kyseinen kohta on yllä.

### 1.8 Miten rajoitteet luetaan

- **Ei korttia tai vikaa ei tarkoita, ettei vikaa ole.** Katso toiminnon luvun 7-kohta ja tämän luvun kohta 1.1.
- **Tunnettu vika ei tarkoita, että se on tapahtunut.** Vahvistettu käytännön seuraus ja mahdollinen seuraus on erotettu korteissa
  (vaikutusrivit); kuolleen ja toimittamattoman koodin viat eivät koske toimitettuja ohjelmia.
- **Ehdokas ei ole havainto.** KE-taulukon ja lukujen 7-kohtien ehdokkaat on lueteltu avoimina; niitä ei ole tarkastettu.
- **[päätelty] ja [tuntematon] ovat eri asioita.** Päätelmälle on peruste; tuntematonta ei voi ratkaista aineistosta ilman ajoa,
  laitetta tai omistajan tietoa.
- **Bcc32-GUI-ohjelmien käytös on suurelta osin mittaamatta**: sama lähde voi toimia ikkunaohjelmassa eri tavoin kuin konsolissa.

## 2. Erot ohjelmien välillä

Rajoitteet koskevat eri ohjelmia eri tavoin:

| Rajoite | HKM | HKW | VM | VL | VW |
|---|---|---|---|---|---|
| Ajo auditoinnissa (kokeet) | MSVC-konsolikoe (testikehys) | **ei ajettu** | MSVC-konsolikoe (testikehys) | ei katettu testikehyksessä | **ei ajettu** |
| Auditoimaton osuus | `HkConsole`: 4 tiedostoa 8:sta tasolla 1 (1 989 riviä) | `cbHk` 58 % riveistä ja `cbTp` 74 % tasolla 1 | `JukConsole`: 5 tiedostoa 10:stä tasolla 1 (1 121 riviä) | kuten VM | `ViestiWin` 53 % ja `cbTp` 74 % tasolla 1 |
| Toiminnot luettu vain osin | tulosten muuttamistoiminnot lukuun ottamatta sarjasiirtoa [tuntematon] (luku 06) | arvonta (`HkArv2.cpp`, taso 1); lisäksi 41 muuta kirjoittavaa lomaketta (HK ja viesti) | kilpailija- ja tulosmuutostoiminnot [tuntematon] (luvut 02, 06) | omat tiedostokäytännöt (luku 03) | `UnitCfg.cpp` (taso 1); partio ja rogaining (luku 07) |
| Laiteajo | ei | ei | ei | ei | ei |

[todennettu: `auditoinnin-yhteenveto.md` luvut 3 ja 6; luvut 02, 03, 06, 07. Kääntäjätodisteiden raja: kaikki luokan c kokeet ovat
MSVC-konsolirakennelmia.] ViestiLuenta on käsikirjassa heikoimmin katettu ohjelma: se on supistettu käännös, jonka rajausta ei ole
dokumentoitu (T-052), ja sen käyttötarve on ristiriitainen (kohta 1.6).

## 3. Asetukset ja käännösvalitsimet

Tällä luvulla ei ole omia asetuksia. Rajoitteiden kannalta olennaista:

- Käännösvalitsin voi poistaa koodin toimitetusta ohjelmasta; kuolleen koodin vikoja ei lueta toimitetun ohjelman vioiksi. 107
  makroa on koskaan määrittelemättömiä (T-050).
- Rekisterin makroarvot ovat leksikaalisen arvioijan tuotoksia, ja lausekkeella määritellyt arvot ovat väärin (esimerkiksi viestin
  `MAX_PORTTI` näkyy arvona 1, vaikka todellinen arvo on 132); älä käytä niitä tosiasioina
  [todennettu: `auditoinnin-yhteenveto.md` luku 6, rajoite 6; `ominaisuusvalitsimet.md` 13.1].
- Toimittamattomien kokoonpanojen (`VALUKU=200`, `MAXOSUUSLUKU=250`) tietue- ja sanomakoot eivät ole auditoinnin tarkastelemia
  samoin kuin toimitettujen (T-051).

## 4. Vaihtoehtoiset polut ja virhetilanteet

- **Kun aihe on auditoimattomalla alueella.** Auditointi ei ole tarkastanut aluetta, ja käsikirjan vastaava kohta on kirjoitettu koodin
  kohdennetun luvun perusteella (merkintä "luettu käsikirjaa varten"). Voit kysyä asiantuntija-agentilta (`/pirila-ask`), joka lukee
  koodin rajatusti ja sanoo vastauksen perustuvan kohdennettuun tarkasteluun.
- **Kun rekisteri ja käsikirja ovat ristiriidassa.** Rekisteri voittaa; ristiriita on kohdassa 1.6 tai KE-taulukossa.
- **Kun tieto vanhenee.** Käsikirjan rivinumerot ja ehdokkaat sidotaan revisioon `f7fe553`. Jos tuotantokoodi muuttuu, kohtien
  paikkansapitävyys on tarkistettava. `tietopohja.py tarkista` ilmoittaa vanhentuneet kortit; käsikirjan tekstiä se ei tarkista.
- **Kun aineisto ei kerro.** "Aineisto ei kerro" on oikea vastaus; sitä ei täydennetä oletuksilla. Esimerkiksi laitteen käytöstä ilman
  kuittausta tai kahden aseman todellisesta toiminnasta ei ole todistetta (kohta 1.3).

## 5. Tunnetut viat tässä toiminnossa

<!-- generoitu:viat alku (tietopohja.py lohkot; älä muokkaa käsin) -->
Tähän lukuun ei ole kytketty toimintoja (`kasikirja/toiminnot.csv`).
<!-- generoitu:viat loppu -->

Tällä luvulla ei ole omaa toimintoa, johon viat kytkettäisiin. Vikaluettelo on kunkin toimintoluvun (02-13 ja 01) kohdassa 5.
Kattavuuden ja todentamisen rajoitteet koskevat kaikkia näitä luetteloita: löydettyjen vikojen määrä on alaraja.

## 6. Mihin tieto perustuu

- Asiakirjat: `doc/arkkitehtuuri/auditoinnin-yhteenveto.md` (luvut 3, 4, 6, 10), `seuranta.md` (avoimet kysymykset ja
  "Vaiheen K ehdokkaat ja tarkistuskohteet" KE-1-KE-23), `omistajan-paatokset.md` (OP-1-OP-18, kenttä "Tila / status"),
  `rekisteri/README.md` (analyysitasot), `doc/tietopohja/README.md` (kattavuusrajoite, luottamuksellisuus),
  `doc/tietopohja/laatu/k3-kirjoittajien-huomiot.md` (kirjoittajien ehdokkaat ja ristiriidat).
- Käsikirjan luvut: kunkin luvun kohta 7 (luvut 02-13) luettu tätä lukua varten ja koottu kohtaan 1.3.
- Rekisteri: `rekisteri/tiedostot.csv` (analyysitasot), `riskiarvio.csv`, `havainnot.md` (ehdokashaut KE-ehdokkaiden kohdalla).
- Koodi uutena: ei luettu tätä lukua varten; ristiriitojen koodiviitteet ovat peräisin lukujen 07, 09 ja 12 sekä K-3-huomioiden
  kirjauksista [luettu käsikirjaa varten, ei tarkastettu].
- Todentamisen taso: luvun luvut (kattavuus, todennusluokat, kysymysluokat) on poimittu asiakirjoista sellaisinaan; ne ovat
  C-13:n tilanne. Tarkistus nykyisiä rekistereitä vasten on tehty vain OP-kohtien tilan osalta (kaikki avoin) ja KE-taulukon osalta
  (23 riviä).

## 7. Mitä ei ole selvitetty

- Kysymysten ajantasainen tila: tämän luvun luvut (84 ei-ratkaistua kysymystä ja luokittelu) ovat vaiheen C sulkemisen tilanne;
  `seuranta.md` on voinut muuttua. Sitä, onko kaikki Q-luokan 1 kysymykset ja Q14, Q31 ja Q42 merkitty ratkaistuiksi, ei ole selvitetty.
- Luvun 09 kohta 7 on osin vanhentunut (Q17, Q18, Q24): kirjoittajan kuvaus perustuu aiempaan tilaan; muita lukujen 7-kohtien Q-viittauksia
  ei ole tarkistettu rivi riviltä seuranta.md:tä vasten.
- Ehdokkaiden (KE-1-KE-23 ja lukujen ehdokkaat) todellinen luonne on selvittämättä: ne voivat olla vikoja, jo tunnettujen vikojen
  ilmentymiä, kuollutta koodia tai väärin luettuja. Tarkistus on suunniteltu E-1:een, jota ei ole aloitettu.
- Käsikirjan lukujen riippumaton tarkastus on tekemättä (Emit-aineistoa lukuun ottamatta).
- Kuinka kattava käsikirja on käyttäjän näkökulmasta: ei ole selvitetty, tuleeko käytännön kilpailutilanteita, joita yksikään luku
  ei kata. Palautumisohjetta kilpailun aikaisiin vikatilanteisiin ei löytynyt (luku 13).
- Ehdokkaat (ei tarkastettu): ei uusia havaintoja tässä luvussa. Haettu `havainnot.md`:stä ja `seuranta.md`:n KE-taulukosta: tämän luvun
  ehdokaslistat ovat kootut muista luvuista (kohta 1.5).
