# Vikakorttipilotti: lukudokumentti

> **Generoitu tarkastusaineisto - älä muokkaa käsin.** Tuotettu komennolla
> `python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kooste`. Sisältö on korttien
> (`doc/tietopohja/vikakortit/`) sellaisenaan koottu kopio; korjaukset tehdään kortteihin tai
> rekistereihin, minkä jälkeen dokumentti tuotetaan uudelleen. `tietopohja.py tarkista` ilmoittaa,
> jos dokumentti on vanhentunut. Tämä ei ole vikarekisteri: ensisijainen lähde on
> `doc/arkkitehtuuri/rekisteri/`.
>
> **Luottamuksellinen:** kuvaa korjaamattomia vikoja, myös verkosta laukeavia. Ei julkiseen
> repositorioon ilman omistajan hyväksyntää (OP-1).


## Sisällys

| Kortti | Otsikko | Tila | Prioriteetti | Tekninen vakavuus | Ohjelmointivirheen todennus |
|---|---|---|---|---|---|
| [H-033](#h-033) | Toiselta työasemalta tullut virheellinen vaiheen numero voi kirjoittaa HK-ohjelman muistiin väärään paikkaan | avoin | P1 | korkea | luettu koodista ja riippumattomasti tarkastettu |
| [H-041](#h-041) | Suuri kilpailukokoonpano ylittää tietueen pinopuskurin: HkMaali kaatui kilpailijaa lisättäessä, kun väliaikapisteitä oli 221 | avoin | P3 | korkea | vahvistettu ajamalla toimitettua ohjelmaa |
| [H-071](#h-071) | Verkosta tullut tiedostonsiirtosanoma voi kaataa ohjelman sekä luoda tai korvata tiedostoja ilman tarkistusta | avoin | P1 | korkea | luettu koodista ja riippumattomasti tarkastettu |
| [H-093](#h-093) | Kun lokiyhteys katkeaa, HK-ohjelma voi kaatua tai jumiutua lokia kirjoittaessaan | avoin | P2 | korkea | toistettu eristetyllä kokeella tai testikehyksessä |
| [H-099](#h-099) | Epäilty: HK:n käynnistys jättäisi saapuvien sanomien lukon varatuksi (kumottu) | kumottu | - | kumottu | kumottu |
| [H-109](#h-109) | "Poista kilpailija" ilman valittua kilpailijaa voi vioittaa tietueiden listan ja sekoittaa myöhemmin lisättyjä kilpailijoita | avoin | P2 | korkea | luettu koodista ja riippumattomasti tarkastettu |
| [H-129](#h-129) | Vaiheen vaihto suurella kilpailulla voi vioittaa HK-ohjelman muistia | avoin | P2 | korkea | toistettu eristetyllä kokeella tai testikehyksessä |
| [H-145](#h-145) | Jos emit-tiedoston kirjoitus epäonnistuu, luettu kortti kuitataan lukijalle mutta jää tallentumatta | avoin | P3 | keskitaso | toistettu eristetyllä kokeella tai testikehyksessä |
| [H-159](#h-159) | Sirit- tai Zebra-RFID-lukijan uudelleenyhteys lopettaa lukijan luennan, kunnes ohjelma käynnistetään uudelleen | avoin | P2 | korkea | luettu koodista ja riippumattomasti tarkastettu |
| [H-171](#h-171) | ViestiWin kirjoittaa joka kilpailun avauksessa 158 tavua asetustiedoston nimen puskurin ohi viereiseen muistiin | avoin | P2 | korkea | luettu koodista ja riippumattomasti tarkastettu |
| [T-038](#t-038) | Vaiheen vaihto nollaa tilaa tapauskohtaisesti, ei yhdestä paikasta (HK) | avoin | - | keskitaso | - |
| [T-042](#t-042) | Tulostus- ja tiedostokanavan sulkeva funktio ei voi nollata kutsujan osoitinta, joten suljettuun kanavaan jää viittauksia | avoin | - | keskitaso | - |

## Lukuohje

### Tekninen vakavuus ja korjausprioriteetti

Kortin yhteenvedossa ja teknisissä tiedoissa on kaksi eri asiaa:

- **Tekninen vakavuus** kertoo, kuinka paha seuraus on, jos vika laukeaa: korkea,
  keskitaso-korkea, keskitaso, matala-keskitaso tai matala. Erän alkuperäinen vakavuus
  ("Vakavuus (rekisteri)") voi poiketa vaiheen C-13 yhtenäisestä arviosta ("Riskiarvio",
  `riskiarvio.csv`); tällöin jälkimmäinen on voimassa ja ero on perusteltu.
- **Prioriteetti P1-P5** on korjausjärjestyksen arvio, joka yhdistää vakavuuden ja sen, kuka tai
  mikä vian voi laukaista (`auditoinnin-yhteenveto.md` luku 9.2):
  - **P1** verkossa oleva kolmas osapuoli voi laukaista oletusasetuksin, ja seuraus on muistin tai
    tiedoston ylikirjoitus, tiedoston korvaus tai tulosten hiljainen vääristyminen;
  - **P2** verkosta oletusasetuksin mutta lievempi seuraus, verkosta asetuksella tai laitteelta,
    tai tavallisessa käytössä toistuva pysähtyminen, jumiutuminen, kaatuminen tai tulosten hiljainen
    virhe;
  - **P3** toimitetussa ohjelmassa, mutta kapeampi seuraus tai harvinaisempi edellytys;
  - **P4** vaatii virheellisen paikallisen tiedoston tai käyttäjän virheen, tai matala vakavuus;
  - **P5** kosmeettinen, kuollut tai toimittamaton koodi, kokoomarivit.

Vakava vika voi olla matalalla prioriteetilla, jos sen laukaiseminen vaatii harvinaisen
tilanteen, ja päinvastoin.

### Todentamistasot

"Ohjelmointivirheen todennus" kertoo, miten **itse virhe** on osoitettu. Se ei kerro, onko
käytännön seuraus osoitettu (siihen on vaikutustaulukko).

| Taso | Merkitys |
|---|---|
| toimitettu_ohjelma | vahvistettu ajamalla toimitettua ohjelmaa |
| koe_tai_testikehys | toistettu eristetyllä kokeella tai testikehyksessä |
| riippumaton | luettu koodista ja riippumattomasti tarkastettu |
| riippumaton_osittain | luettu koodista; riippumaton tarkastus vahvisti osittain |
| tarkastajan_loyto | riippumattoman tarkastajan oma löytö |
| ei | yhden analyysin löydös, ei riippumattomasti tarkastettu |
| ei_ratkea_staattisesti | ei ratkaistavissa koodia lukemalla |

Varmuusmerkinnät tekstissä: [todennettu] = osoitettu lähteestä tai koodista; [päätelty] =
johdettu, ei osoitettu; [tuntematon] / ei tiedossa = aineisto ei ratkaise.

### Käytännön vaikutusten luokat

Luvun 6 taulukko tulee rekisteristä `vaikutukset.csv`. Jokaiselle 16 kysymykselle on yksi arvo:

| Arvo | Merkitys |
|---|---|
| **Vahvistettu** | Juuri tämä seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla. Koodin lukeminen ei riitä. |
| Mahdollinen | Seuraus johtuu koodista, mutta sitä ei ole osoitettu; tai se riippuu perusteessa nimetyistä olosuhteista. |
| Ei vaikutusta (perusteltu) | Aineisto osoittaa, ettei seurausta synny. |
| Ei tiedossa | Aineisto ei ratkaise kysymystä. **Tiedon puute ei tarkoita, ettei vaikutusta ole.** |
| Ei sovellu | Kysymys ei koske tätä vikaa. |
| _Ei arvioitu_ | Riviä ei ole vielä arvioitu. |

### Auditoinnin kattavuusrajoite

Auditointi ei kata koko lähdekoodia: noin 36 % riveistä on vain inventoitu, yhtäkään
tuotantotiedostoa ei ole katselmoitu kokonaan, ja havaintojen määrä on alaraja
(`auditoinnin-yhteenveto.md` luvut 3, 4 ja 10). Vain yksi vika (H-041) on vahvistettu ajamalla
toimitettua ohjelmaa. Kortin puuttuminen ei tarkoita, että toiminto on virheetön.

### Lähderevisio

- Korttien lähdeviitteet (rivinumerot) on kirjattu revisioihin: `4b48749`, `7aaca40`, `ab10f88`, `f7fe553`. Tuotantokoodi `TPsource/` ei ole muuttunut revision `f7fe553` jälkeen (`doc/arkkitehtuuri/CHECKPOINT.md`).
- Kortit ja rekisterit luettiin työpuusta, jonka viimeisin commit oli `86b4c62`.
- Rekisteritiedostojen tiivisteet tuottohetkellä (SHA-256, 12 merkkiä): `havainnot.md` cb94426a9dd9; `riskiarvio.csv` a8ce72643604; `tekninen-velka.csv` e941ce171686; `vaikutukset.csv` bfee628a2976; `korjauskokonaisuudet.csv` 5c19cce94bf2.
- Jokaisen kortin teksti on tarkistettu sen omien rekisteririvien tilaa vastaan; tiiviste on kortin teknisissä tiedoissa ("Rekisteririvien tiiviste nyt").


## Kortit


---

<a id="h-033"></a>

## H-033 – Toiselta työasemalta tullut virheellinen vaiheen numero voi kirjoittaa HK-ohjelman muistiin väärään paikkaan

> **H-033** · tila: **avoin** · prioriteetti: **P1** · tekninen vakavuus: korkea
> Ohjelmointivirheen todennus: luettu koodista ja riippumattomasti tarkastettu. Riskiarvion saavutettavuus: verkosta oletusasetuksin (tarkemmin luvussa 4).
> Ohjelmat: HkKisaWin, HkMaali (tarkistettu kortin kirjoituksen yhteydessä).
> Korjauskokonaisuus: KK-C13-1 Työasemaprotokollan vastaanoton rajatarkistukset (aalto 1).

### 1. Lyhyesti

HK-ohjelma (HkKisaWin tai HkMaali) ottaa verkosta vastaan kilpailijan vaihekohtaisen tiedon ja käyttää sanomassa olevaa vaiheen numeroa tarkistamatta, mahtuuko numero ohjelman omaan vaihetaulukkoon. Jos numero on taulukon ulkopuolella, ohjelma kirjoittaa noin 20 kentän verran tietoa taulukon ulkopuolelle muistiin.

**Luettu koodista ja riippumattomasti tarkastettu:** ohjelmointivirhe ja se, että polku on käytössä oletusasetuksin.
**Vahvistettu käytännön seuraus:** ei yhtään. Ajonaikaista vaikutusta ei ole toistettu.
**Mahdollinen:** muistin ylikirjoitus, jonka seurausta ei tunneta (ohjelman kaatuminen, väärä arvo jossain muualla tai ei näkyvää oiretta).

### 2. Mitä ohjelma yrittää tehdä

Työasemat vaihtavat kilpailijoiden tietoja verkossa (ks. sanasto: Työasema, Sanoma). Kun toinen työasema lähettää yhden kilpailijan tiedot yhdeltä vaiheelta (monivaiheisen kilpailun yksi osakilpailu, ks. sanasto: Vaihe), vastaanottava HK-ohjelma purkaa sanoman kilpailijan tietueen siihen vaiheeseen, jonka numero sanomassa on. Toiminto on käytössä, kun työasemien välinen tiedonsiirto on käytössä.

### 3. Mikä menee vikaan

Sanomassa oleva vaiheen numero voi olla myös negatiivinen. Ohjelma käyttää lukua suoraan järjestysnumerona (ks. sanasto: Indeksi) vaihetaulukossa. Taulukon koko vastaa kilpailun vaiheiden lukumäärää. Numeroa ei verrata taulukon kokoon.

Jos numero on negatiivinen tai suurempi kuin taulukon viimeinen paikka, ohjelma kirjoittaa noin 20 kentän tiedot taulukon ulkopuolella olevaan muistiin (ks. sanasto: Puskuriylitys, ylikirjoitus). Aineisto ei kerro, mitä taulukon vieressä muistissa on.

Saman funktion muu tarkistus koskee kilpailijan järjestysnumeroa: jos se on nolla tai negatiivinen, ohjelma näyttää ilmoituksen "Virheellinen sanoma tiedonsiirrosta" ja hylkää sanoman. Vaiheen numeroa ei tarkisteta missään sanomatyypin haarassa.

### 4. Missä tilanteessa vika voi ilmetä

- **Ohjelmat:** HkKisaWin ja HkMaali. Perusteena on, että vikaa sisältävä HK:n vastaanottotiedosto käännetään näihin. Viestiohjelmien vastaavaa vastaanottopolkua ei ole tässä havainnossa tarkistettu.
- **Asetus:** rekisterin ja riskiarvion mukaan vastaanottopolku on käytössä oletusasetuksin; jokainen vaihekohtaisen kilpailijatiedon sanoma kulkee tämän koodin kautta. Erillistä asetusta ei tarvita.
- **Lähettäjä:** protokollassa ei ole lähettäjän tunnistusta (H-029), joten sanoman voi lähettää mikä tahansa verkossa oleva laite tai ohjelma.
- **Laukaisuehto:** sanomassa oleva vaiheen numero on taulukon ulkopuolella. Korjaussuunnitelma mainitsee yhtenä tapauksena toisen työaseman päivityksen, jossa on enemmän vaiheita kuin vastaanottajalla.
- **Normaalikäyttö vai poikkeustilanne:** aineisto ei kerro, lähettääkö kelvollisesti asetettu työasema koskaan taulukon ulkopuolisen numeron. Kun työasemien vaihemäärät poikkeavat toisistaan tai sanoma on virheellinen, ehto täyttyy.

### 5. Mitä käyttäjä voi huomata

Aineisto ei kerro, mitä käyttäjä näkee, koska ajonaikaista vaikutusta ei ole toistettu. Koodista voi päätellä kolme mahdollisuutta: ohjelma kaatuu; ylikirjoitus osuu muuhun tietoon ja näkyy vääränä arvona muualla; tai mitään näkyvää ei tapahdu (ks. sanasto: Puskuriylitys, Hiljainen virhe). Virheilmoitusta vaiheen numerosta ohjelma ei anna. Vika voi siis jäädä kokonaan huomaamatta.

### 6. Vaikutukset kilpailuun

| Kysymys | Arvio | Peruste |
|---|---|---|
| Vaikutus: Voiko aika, tulos, sijoitus, pisteet tai hylkäys olla väärä? | Mahdollinen | Muistin ylikirjoitus voisi vioittaa muistissa olevaa tietoa; riskiarvio: välillinen vaikutus. Tulosvaikutusta ei ole osoitettu. |
| Vaikutus: Voiko maali- tai väliaikojen kirjaus pysähtyä tai katketa? | Mahdollinen | Ohjelman kaatuminen pysäyttäisi työaseman ajanoton (riskiarvio: pysäyttää). Kaatumista ei ole toistettu. |
| Vaikutus: Voiko leimaus- tai kortintiedon luku jäädä tekemättä tai mennä väärin? | Ei tiedossa | Aineisto ei kerro vaikutusta leimaus- tai kortintiedon lukuun. |
| Vaikutus: Voiko tallennettu tai vastaanotettu tieto kadota tai vioittua? | Mahdollinen | Ylikirjoitus osuu muistissa taulukon viereiseen tietoon; mitä siinä on, aineisto ei kerro. |
| Vaikutus: Voiko ohjelma kaatua tai sulkeutua odottamatta? | Mahdollinen | Taulukon ulkopuolinen kirjoitus keossa voi kaataa ohjelman; ei toistettu. |
| Vaikutus: Voiko ohjelma tai yksittäinen toiminto jäädä jumiin? | Ei tiedossa | Aineisto ei kerro jumiutumisesta. |
| Vaikutus: Voiko jokin mennä väärin ilman ilmoitusta tai näkyvää oiretta? | Mahdollinen | Ohjelma ei tarkista indeksiä eikä ilmoita ylikirjoituksesta; seuraus voi olla väärä arvo muualla. |
| Vaikutus: Voiko vika vaikuttaa tiedonsiirtoon tai muihin työasemiin? | Mahdollinen | Virhe syntyy työasemien välisen sanoman vastaanotossa; kaatunut vastaanottaja ei vastaanota enää. |
| Vaikutus: Voiko vika vaikuttaa lukijoihin, kelloihin, näyttötauluihin tai tulostimiin? | Ei tiedossa | Aineisto ei kerro vaikutusta laitteisiin. |
| Vaikutus: Voiko vika vaikuttaa tulosteisiin, HTML- tai XML-tiedostoihin, tulospalveluun tai SQL-peiliin? | Ei tiedossa | Aineisto ei kerro vaikutusta tulosteisiin tai julkaisuun. |
| Vaikutus: Voiko ulkopuolinen tai verkossa oleva laite käyttää vikaa hyväkseen? | Mahdollinen | Verkossa oleva laite voi lähettää virheellisen sanoman; lähettäjää ei tunnisteta (H-029). Hyödynnettävyyttä ei ole osoitettu. |
| Ilmeneminen: Voiko vika ilmetä tavallisessa kilpailukäytössä ilman erityisasetusta tai virhettä? | Ei tiedossa | Polku on käytössä oletusasetuksin, mutta aineisto ei kerro, lähettääkö kelvollinen työasema taulukon ulkopuolisen vaihenumeron. |
| Ilmeneminen: Voiko vika ilmetä poikkeustilanteessa (virhe, katkos, virheellinen syöte, harvinainen asetus)? | Mahdollinen | Virheellinen sanoma tai vastapuolen suurempi vaihemäärä täyttää laukaisuehdon. |
| Ilmeneminen: Voiko käyttäjä huomata vian sen ilmetessä? | Ei tiedossa | Vaihenumeroa ei tarkisteta eikä virheilmoitusta anneta; ajonaikaista oiretta ei ole toistettu. |
| Hallinta: Onko aineistossa kirjattu keino välttää vika tai pienentää sen riskiä? | Ei tiedossa | Aineistossa ei ole kirjattua keinoa; luvun 15 suositukset nimeävät muut havainnot. |
| Hallinta: Onko aineistossa kirjattu keino palautua vian seurauksista kilpailun aikana? | Mahdollinen | Riskiarvio: palautettavuus uudelleenkäynnistyksellä; tehoa ei ole kokeiltu. |

Arvot: **Vahvistettu** = seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla; Mahdollinen = seuraa koodista, mutta seurausta ei ole osoitettu; Ei vaikutusta (perusteltu) = aineisto osoittaa, ettei seurausta synny; Ei tiedossa = aineisto ei ratkaise; _Ei arvioitu_ = riviä ei ole vielä arvioitu. Tiedon puute ei tarkoita, ettei vaikutusta ole. Lähteet: `doc/arkkitehtuuri/rekisteri/vaikutukset.csv`.

### 7. Mitä kilpailun aikana voi tapahtua

Vahvistetun ja mahdollisen erottelu on luvussa 11. Tilanne kilpailun aikana:

- HK-työasema, joka vastaanottaa virheellisen vaiheen numeron, kirjoittaa muistiinsa väärään paikkaan. Riskiarvio pitää ajanottoa pysähtyvänä (ohjelman kaatuminen pysäyttäisi sen työaseman ajanoton) ja vaikutusta tuloksiin välillisenä. Mitä tarkalleen vioittuisi, aineisto ei kerro.
- Korjaussuunnitelman mainitsema tapaus: toisella työasemalla on kilpailussa enemmän vaiheita kuin vastaanottajalla, ja sen lähettämän vaihekohtaisen tiedon numero on vastaanottajan taulukon ulkopuolella. Tapaus on mahdollinen; sitä ei ole toistettu.

### 8. Palautuminen ja uudelleenkäynnistys

Riskiarvio luokittelee palautettavuuden muotoon "uudelleenkäynnistys": ohjelman käynnistäminen uudelleen on arvion mukaan tie takaisin. Keinon tehoa ei ole kokeiltu. Aineisto ei kerro, vaikuttaako yksi virheellinen sanoma useampaan työasemaan, eikä sitä, mitä tallentamattomalle tiedolle käy.

### 9. Mitä käyttäjä voi tehdä riskin pienentämiseksi

Aineistossa ei ole kirjattua keinoa tälle havainnolle. Väliaikaiset suojaussuositukset (`muistiturvallisuus.md` luku 15: erillinen ajanottoverkko, palomuurin sallittujen lista) nimeävät muut havainnot (H-071, H-072, H-032, H-061), eivät tätä. Niiden vaikutusta tähän havaintoon ei ole arvioitu.

### 10. Tekninen juurisyy

Funktio `tark_kilp` (`Hk/HkCom32.cpp`) käsittelee vaihekohtaisen tietosanoman (KILPPVT). Haarassa, joka purkaa sanoman kilpailijan tietueen vaiheeseen, sanoman 16-bittinen etumerkillinen vaiheindeksi indeksoi taulukkoa `pv[]` suoraan `unpack0`- tai `unpack`-kutsussa. Samaa indeksiä annetaan myös `Sarja`- ja `setSarja`-kutsuille; niiden sisäistä toimintaa ei ole tässä tarkistettu. Taulukko varataan `kilptietue`-rakentajassa (`Hk/HkDat.cpp`) kokoon `new kilppvtp[kilpparam.n_pv]`. Rajatarkistusta ei ole. Sanomatyypin haarassa `kaikki==0` (väliaikasanoma) on `vali`-tarkistus, mutta vaiheindeksiä ei tarkisteta missään haarassa; ainoa muu tarkistus koskee kilpailijan järjestysnumeroa.

Riippumattoman tarkastuksen huomio, kirjattu ehdokkaaksi seuranta.md:hen, ei tarkastettu: kun väliaikoja on, `unpack` kirjoittaa väliajat myös alueen ulkopuolisen olion `va`-osoittimen kautta, jolloin kirjoitus voisi osua mielivaltaiseen osoitteeseen eikä vain taulukon viereiseen muistiin.

Juurisyy on JS-1: ulkoista syötettä ei validoida rajalla.

### 11. Mitä on todennettu

**(a) Ohjelmointivirhe.** Koodi on luettu. C-13:n riippumaton kokonaistarkastus (K1) ja pääistunto lukivat saman kohdan ja vahvistivat vian. Todennuksen laji on staattinen, lähdekoodi. Erän C-2 alkuperäinen arvio oli vakavuus "matala"; sitä ei ole perusteltu, ja C-13:n riskiarvio pitää vakavuutta korkeana.

**(b) Käytännön seuraus.** Mitään seurausta ei ole osoitettu. Ajonaikaista vaikutusta ei ole toistettu eikä testikehyksessä ajettu. Korjaussuunnitelma sanoo, ettei toistavaa testiä ole nyt olemassa.

### 12. Mitä ei vielä tiedetä

- Mitä muistissa on taulukon vieressä ja mikä on ylikirjoituksen seuraus (kaatuminen, väärä arvo vai ei mitään). Aineisto ei kerro. ASan-testi (muistivirheitä havaitseva testikokoonpano) on korjaussuunnitelmassa osa korjausta.
- Lähettääkö kelvollinen työasema normaalikäytössä koskaan taulukon ulkopuolista vaiheen numeroa. Aineisto ei kerro.
- Onko viestiohjelmien vastaanottopolussa vastaava puute. Sitä ei ole tässä havainnossa tarkistettu.
- Rekisterissä ei ole tähän havaintoon liitettyä avointa kysymystä (Q).

### 13. Miten vika on tarkoitus korjata

Korjaussuunnitelman kokonaisuus KK-C13-1 (Työasemaprotokollan vastaanoton rajatarkistukset), kolmas pull request (PR 3): vaiheindeksi rajataan taulukon kokoon. Vastapuolen päivitys, jossa on enemmän vaiheita kuin vastaanottajalla, hylätään. Virheellinen paketti kirjataan lokiin eikä pysäytä vastaanottoa. Sanomamuoto ei muutu, joten korjattu ja korjaamaton ohjelma toimivat yhdessä. Testiksi suunnitellaan uusi kehystesti ASan-kokoonpanossa.

Havaintoon ei liity omistajan päätöstä. Kokonaisuuden päätös OP-5 koskee saman kokonaisuuden FILESEND-osaa (H-071).

### 14. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | H-033 |
| Rekisterin kuvaus | `tark_kilp` käyttää verkosta tullutta vaiheindeksiä tarkistamatta / uses wire stage index unchecked **C-13:n uudelleenarvio / re-assessment (2026-11-26):** riippumaton kokonaistarkastus (K1) ja pääistunto lukivat koodin: etumerkillinen 16-bittinen vaiheindeksi `ibuf->d.pv.pv` KILPPVT-paketista indeksoi kekotaulukkoa `pv[]` (`new kilppvtp[n_pv]`) tarkistamatta, ja `unpack0`/`unpack` kirjoittaa noin 20 kenttää alueen ulkopuoliseen olioon; polku on käytössä oletusasetuksin. Erän C-2 vakavuutta "matala" ei ole perusteltu; C-13:n riskiarviossa tekninen vakavuus korkea ja prioriteetti P1, korjauskokonaisuus KK-C13-1 (`riskiarvio.csv`). Ajonaikaista vaikutusta ei ole toistettu / network-default heap write through an unchecked wire stage index; rated high and P1 in the C-13 risk register; not reproduced at run time |
| Erä | C-2 |
| Tila | avoin |
| Vakavuus (rekisteri) | matala / low |
| Varmuus (rekisteri) | [todennettu] |
| Todennus (rekisteri) | riippumaton lähdetarkastus / independent source review |
| Prioriteetti | P1 |
| Riskiarvio | vakavuus korkea; saavutettavuus verkko_oletus; todennus riippumaton; tulokset epasuora; ajanotto pysayttaa; palautettavuus uudelleenkaynnistys; tietoturva verkko |
| Riskiarvion peruste | Main session after the independent review (K1), read from code: Hk/HkCom32.cpp:274-277 indexes the heap array pv[] (new kilppvtp[n_pv], Hk/HkDat.cpp:206) with a signed 16-bit stage index taken from the KILPPVT packet without any check and unpacks about 20 fields into the out-of-range object; every KILPPVT packet reaches it with default settings. Network-default heap write = P1 by the uniform definition. Register severity is matala (C-2), which was never justified; C-13 rates the technical severity korkea. The Opus draft capped it at P4 because of the low register severity and itself noted that a write outside pv[] would make it P1. Run-time effect not reproduced: an ASan harness test is part of the fix. |
| Lähde (raportti) | `../tiedonsiirto.md` |
| Näytön laji | staattinen, lähdekoodi / static, source |
| Juurisyy | JS-1 Ulkoista syötettä ei validoida rajalla |
| Ohjelmat | HkKisaWin HkMaali (peruste: tarkistettu) |
| Korjauskokonaisuus | KK-C13-1 Työasemaprotokollan vastaanoton rajatarkistukset; vaihe 1, aalto 1; haara `korjaus/kk-c13-1-vastaanoton-rajatarkistukset` |
| Muut kokonaisuudet | KK-C13-10 |
| Omistajan päätökset, joissa havainto mainitaan | - |
| Kokonaisuuden KK-C13-1 omistajan päätökset | OP-5 FILESEND-tiedostonsiirron rajaus (avoin) |
| Kysymykset | - |
| Liittyvät havainnot | - |
| Lähdeviitteet (revisio f7fe553, polut `TPsource/V52/`-hakemistosta) | `Hk/HkCom32.cpp:274-279`; `Hk/HkDat.cpp:206`; C-13: `Hk/HkCom32.cpp:270-279`, `Hk/HkDat.cpp:204-209` |

Rekisteririvien tiiviste nyt: `15fb661d9a7e`.


---

<a id="h-041"></a>

## H-041 – Suuri kilpailukokoonpano ylittää tietueen pinopuskurin: HkMaali kaatui kilpailijaa lisättäessä, kun väliaikapisteitä oli 221

> **H-041** · tila: **avoin** · prioriteetti: **P3** · tekninen vakavuus: korkea
> Ohjelmointivirheen todennus: vahvistettu ajamalla toimitettua ohjelmaa. Riskiarvion saavutettavuus: tietyllä asetuksella tai kokoonpanolla (tarkemmin luvussa 4).
> Ohjelmat: HkKisaWin, HkMaali (tarkistettu kortin kirjoituksen yhteydessä).
> Korjauskokonaisuus: KK-C13-11 Tietueen kokorajat (aalto 1).

### 1. Lyhyesti

HK-ohjelmat käsittelevät kilpailijan tietueen kiinteän kokoisessa 10 000 tavun muistialueessa. Kun kilpailussa on viisi vaihetta ja vähintään 221 väliaikapistettä, tietue on tätä suurempi, ja ohjelma kirjoittaa muistialueen yli. Toimitettu ohjelma sallii enintään 5 vaihetta ja 250 väliaikapistettä. Nämä ovat käännöksen enimmäismääriä, eivät kilpailun oletusasetuksia: uuden kilpailun oletus on 2 vaihetta ja 10 väliaikapistettä. Enimmäismäärillä tietue olisi muistialuetta suurempi.

**Vahvistettu ajamalla toimitettua ohjelmaa:** toimitettu HkMaali.exe kaatui, kun kilpailija lisättiin konsolin lisäystoiminnolla ja väliaikapisteitä oli 221. Pisteillä 220 ei tapahtunut mitään. Tämä on ainoa auditoinnissa toimitetulla ohjelmalla vahvistettu vika. Vahvistus tehtiin debuggerin (virheenjäljitysohjelman) alla ja käyttäjän hyväksymällä poikkeamalla alkuperäisestä hyväksymiskriteeristä (ks. luku 11).
**Mahdollinen:** muut seuraukset ja muut ohjelmat. Lukupolkua, 250 väliaikapistettä, käyttöä ilman debuggeria ja HkKisaWinia ei ole ajettu.

### 2. Mitä ohjelma yrittää tehdä

Kun HK-ohjelma lukee kilpailijan tiedostosta (`KILP.DAT`), kirjoittaa kilpailijan tiedostoon tai lisää uuden kilpailijan, se pakkaa tai purkaa kilpailijan tietueen (nimi, sarja, vaiheet, väliajat) tilapäiseen muistialueeseen (ks. sanasto: Tietue, Muistialue, puskuri, Pino). Alueen koko on kiinteä luku 10 000 tavua. Tietueen todellinen koko riippuu kilpailun vaiheiden ja väliaikapisteiden määrästä.

### 3. Mikä menee vikaan

Muistialueen koko on kiinteä käännösaikainen luku, mutta tietueen koko määräytyy vasta ajonaikaisista asetuksista. Kun vaiheita on viisi, tietue mahtuu muistialueeseen juuri ja juuri 220 väliaikapisteellä mutta ei enää 221 pisteellä. Enimmäismäärillä (viisi vaihetta, 250 pistettä) tietue on selvästi muistialuetta suurempi. Tarkat koot ovat luvussa 11. Vain viiden vaiheen kokoonpano voi ylittää rajan.

Ohjelma kirjoittaa ylimenevät tavut muistialueen viereiseen muistiin pinossa (ks. sanasto: Puskuriylitys, ylikirjoitus; Pino). Pinossa on myös funktion paluuosoite ja kääntäjän asettama suoja-arvo, jonka muuttuminen saa Windowsin lopettamaan ohjelman. Samanlaisia pinopuskureita on viisi, myös HkKisaWinin vientitoiminnossa.

### 4. Missä tilanteessa vika voi ilmetä

- **Ohjelmat:** HkMaali (vahvistettu ajamalla) ja HkKisaWin (vain koodista: vastaava pinopuskuri GUI:n vientitoiminnossa ja sama oletusarvo; ei ajettu). Viestiohjelmia vika ei koske tässä havainnossa; mahdollinen vastaava raja viestin 250-käännöksissä on avoin kysymys Q31.
- **Asetus:** viisi vaihetta ja vähintään 221 väliaikapistettä (XML-asetustiedostossa `Intermediaries`). Toimitettu ohjelma sallii enintään 5 vaihetta ja 250 pistettä; ne ovat käännöksen enimmäismääriä, eivät kilpailun oletusasetuksia (uuden kilpailun oletus on 2 vaihetta ja 10 pistettä). Kokoonpano on riskiarvion mukaan lukittu ennen kilpailua. Alle 221 pisteellä ylitystä ei synny: rajapari 220/221 osoitti 220:n ehjäksi.
- **Toimenpide:** vahvistettu laukaisu oli kilpailijan lisäys konsolissa. Testikehyksessä (ASan, eristetty koe) ylitys näkyi myös kilpailijan kirjoituksessa (`write`). Lukupolun (`read`) ylitystä ei ole näytetty.
- **Normaalikäyttö vai poikkeustilanne:** vika vaatii suuren kokoonpanon. Aineisto ei kerro, onko vähintään 221 väliaikapisteen kilpailuja käytössä todellisissa kilpailuissa.

### 5. Mitä käyttäjä voi huomata

Vahvistetussa ajossa HkMaali.exe lopetettiin äkillisesti debuggerin alla (äkillinen pysäytys, tekniset tiedot luvussa 11). Kilpailijan tietue ehti kirjoittua `KILP.DAT`-tiedostoon ennen pysäytystä. Käyttäjälle näkyvää virheikkunaa ei ole osoitettu: ajo tehtiin debuggerin alla, eikä Windowsin virheraportointi ollut mukana. Se, että käyttäjä näkisi kaatumisen virheikkunana, on [päätelty]. Aineisto ei kerro, mitä käyttäjä näkisi ilman debuggeria.

### 6. Vaikutukset kilpailuun

| Kysymys | Arvio | Peruste |
|---|---|---|
| Vaikutus: Voiko aika, tulos, sijoitus, pisteet tai hylkäys olla väärä? | Mahdollinen | Riskiarvio: välillinen vaikutus; pysähtyminen kesken lisäyksen voi jättää tiedon kirjaamatta. Ei ajettu. |
| Vaikutus: Voiko maali- tai väliaikojen kirjaus pysähtyä tai katketa? | Mahdollinen | Ohjelman pysähtyminen lopettaisi työaseman kirjauksen (riskiarvio: pysäyttää); kirjausta ei kokeiltu. |
| Vaikutus: Voiko leimaus- tai kortintiedon luku jäädä tekemättä tai mennä väärin? | Ei tiedossa | Lukupolkua (ReadFile) ei ole näytetty; aineisto ei kerro vaikutusta leimauksiin. |
| Vaikutus: Voiko tallennettu tai vastaanotettu tieto kadota tai vioittua? | Mahdollinen | Tietue kirjoitettiin tiedostoon ennen pysäytystä; tallentamaton tieto voisi kadota pysähdyksessä. Menetystä ei ole osoitettu. |
| Vaikutus: Voiko ohjelma kaatua tai sulkeutua odottamatta? | **Vahvistettu** | Toimitettu HkMaali.exe pysähtyi (0xC0000409) kilpailijaa lisättäessä 5 vaiheella ja 221 pisteellä; 220:llä ei. Debuggerin alla, käyttäjän poikkeamapäätöksellä. |
| Vaikutus: Voiko ohjelma tai yksittäinen toiminto jäädä jumiin? | Ei tiedossa | Aineisto ei kerro jumiutumisesta. |
| Vaikutus: Voiko jokin mennä väärin ilman ilmoitusta tai näkyvää oiretta? | Ei tiedossa | Ajossa suojatoiminto lopetti ohjelman; käyttäytymistä ilman suojaa ei ole ajettu. |
| Vaikutus: Voiko vika vaikuttaa tiedonsiirtoon tai muihin työasemiin? | Ei tiedossa | Aineisto ei kerro vaikutusta tiedonsiirtoon tai muihin työasemiin. |
| Vaikutus: Voiko vika vaikuttaa lukijoihin, kelloihin, näyttötauluihin tai tulostimiin? | Ei tiedossa | Aineisto ei kerro vaikutusta laitteisiin. |
| Vaikutus: Voiko vika vaikuttaa tulosteisiin, HTML- tai XML-tiedostoihin, tulospalveluun tai SQL-peiliin? | Ei tiedossa | Aineisto ei kerro vaikutusta julkaisuun. |
| Vaikutus: Voiko ulkopuolinen tai verkossa oleva laite käyttää vikaa hyväkseen? | Ei tiedossa | Ylivuotava osa on viimeisen vaiheen väliaikatietoa, jota voi tulla myös verkosta tai laitteilta (päätelty); hyödynnettävyyttä ei ole tutkittu. |
| Ilmeneminen: Voiko vika ilmetä tavallisessa kilpailukäytössä ilman erityisasetusta tai virhettä? | Ei vaikutusta (perusteltu) | Vaatii 5 vaihetta ja vähintään 221 väliaikapistettä; rajapari 220/221 osoitti 220:n ehjäksi. |
| Ilmeneminen: Voiko vika ilmetä poikkeustilanteessa (virhe, katkos, virheellinen syöte, harvinainen asetus)? | **Vahvistettu** | Suuri asetus (221 pistettä) laukaisi virheen toimitetussa ohjelmassa. |
| Ilmeneminen: Voiko käyttäjä huomata vian sen ilmetessä? | Mahdollinen | Ohjelma pysähtyi äkillisesti; käyttäjälle näkyvä kaatuminen ilman debuggeria on päätelty, ei osoitettu. |
| Hallinta: Onko aineistossa kirjattu keino välttää vika tai pienentää sen riskiä? | Ei tiedossa | Aineistossa ei ole kirjattua keinoa. |
| Hallinta: Onko aineistossa kirjattu keino palautua vian seurauksista kilpailun aikana? | Mahdollinen | Riskiarvio kirjaa palautettavuudeksi käsin; keinoa ei ole kokeiltu. |

Arvot: **Vahvistettu** = seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla; Mahdollinen = seuraa koodista, mutta seurausta ei ole osoitettu; Ei vaikutusta (perusteltu) = aineisto osoittaa, ettei seurausta synny; Ei tiedossa = aineisto ei ratkaise; _Ei arvioitu_ = riviä ei ole vielä arvioitu. Tiedon puute ei tarkoita, ettei vaikutusta ole. Lähteet: `doc/arkkitehtuuri/rekisteri/vaikutukset.csv`.

### 7. Mitä kilpailun aikana voi tapahtua

Vahvistetun ja mahdollisen erottelu on luvussa 11. Tilanne kilpailun aikana:

- HkMaali.exe pysähtyy, kun kilpailija lisätään ja kokoonpanossa on 5 vaihetta ja 221 väliaikapistettä. Kilpailijan tietue kirjoitettiin tiedostoon ennen pysäytystä.
- Pysähtyminen kesken kilpailun lopettaa sen työaseman ajanoton kirjauksen (riskiarvio: ajanotto pysähtyy), ja tallentamaton tieto voi kadota. Seurausta tuloksiin pidetään välillisenä. Näitä seurauksia ei ole ajettu (mahdollinen).
- Ei tiedetä, kaatuuko HK jo käynnistyksessä, jos se lukee jokaisen tietueen (Q37).

### 8. Palautuminen ja uudelleenkäynnistys

Riskiarvio luokittelee palautettavuuden muotoon "käsin"; se on aineistoon kirjattu palautumiskeino, jonka tehoa ei ole kokeiltu. Konkreettista toimenpidettä ei ole kuvattu. Aineisto ei kerro, auttaako uudelleenkäynnistys: jos kaatuminen toistuu aina, kun kokoonpano on sama ja lisäys tehdään, uudelleenkäynnistys ei poista syytä [päätelty]. Aineisto ei kerro, koskeeko vika muita työasemia: se on yhden ohjelman sisäinen muistivirhe, eikä leviämistä verkon kautta ole osoitettu.

### 9. Mitä käyttäjä voi tehdä riskin pienentämiseksi

Aineistossa ei ole kirjattua keinoa. Rajapari 220/221 osoittaa, ettei vika synny 220 väliaikapisteellä, mutta sitä ei ole kirjattu käyttäjälle keinoksi, eikä muiden vaihe- ja väliaikapistemäärien yhdistelmien käyttäytymistä ole ajettu.

### 10. Tekninen juurisyy

Tietueen pinopuskuri on `char buf[KILPBUFSIZE]` (`Hk/HkDef.h`), ja `KILPBUFSIZE` on 10 000, kun `VALUKU` on vähintään 31. Tietueen koko `kilprecsize` lasketaan ajossa asetuksista (`Hk/HkDat.cpp`): 360 + n_pv × (152 + 8 × (valuku + 2)) tavua. Viidellä vaiheella ja 220 pisteellä tulos on 360 + 5 × 1928 = 10 000, joten vain viisi vaihetta voi ylittää rajan. Puskureita on viisi: `kilptietue::read`, `write`, `addfrec`, yksi lisäkohta samassa tiedostossa ja GUI:n `cbHk/UnitExport.cpp`. Ainoa rajatarkistus on `Hk/HkSrj.cpp`:ssä, ja se vertaa asetuksia käännöksen enimmäismääriin (`N_PV` 5, `VALUKU` 250), ei puskurin kokoon; XML-lukupolku ohittaa sen. Enimmäismäärät ovat käännöksen sallimat rajat, eivät kilpailun oletusasetuksia (uuden kilpailun oletus: `n_pv` 2, `valuku` 10, `cbHk/WinHk.cpp`). HkMaali Release ei ohita enimmäismääriä, ja HkKisaWin ohittaa ne vain 200Release-konfiguraatiossa. Pysäytyksessä ylikirjoitettiin pinon suoja-arvo (eväste), tallennettu kehysosoitin, paluuosoite ja yksi parametri; Windowsin poikkeuskoodi oli 0xC0000409 (nopea pysäytys).

Juurisyy on JS-3 (kapasiteetti ei seuraa ajonaikaista kokoa). Pahentava tekijä on JS-14 (asetusten ja valitsinten ajautuminen).

### 11. Mitä on todennettu

**(a) Ohjelmointivirhe.** Todennettu kolmella tavalla:
- koot ja kokoonpanot mitattu testikehyksessä: `kilprecsize` 10 000 (220 pistettä), 10 040 (221) ja 11 200 (250) > 10 000; `pack` kirjoitti tasan 11 200 tavua;
- ASan (muistivirheitä havaitseva testikokoonpano) näytti pinopuskurin ylityksen funktioissa `kilptietue::write` ja `kilptietue::addfrec`;
- ajamalla toimitettua HkMaali.exe-ohjelmaa (SHA-256 `61034e5d…`) konsolin kautta (kilpailijan lisäys), debuggerin alla, rajaparilla 220/221: 221 pisteellä ohjelma pysähtyi pinon suoja-arvon tarkistuksessa ja 40 tavua muistialueen ohi oli nollattu (suoja-arvo, tallennettu kehysosoitin, paluuosoite ja parametri); 220:llä muistipaikat pysyivät ehjinä.

Vahvistuksen taso: ennalta kiinnitetty arviointi antoi tuloksen "ei vahvistettu", koska loki-ikkunaan osui edellisen kontrolliajon oma jälkikoeohjelma. Käyttäjä hyväksyi dokumentoidun poikkeaman, ja havainto luokiteltiin "vahvistettu ajamalla" tällä päätöksellä (`testaus.md` luvut 19.9–19.11). Raakatuloksia, kriteereitä tai arviointikoodia ei muutettu, eikä ajoa toistettu.

**(b) Käytännön seuraus.** Vahvistettu: toimitettu HkMaali.exe pysähtyy kilpailijaa lisättäessä rajaparin ylemmällä arvolla debuggerin alla. Ei osoitettu: hyödynnettävyys, käyttö ilman suojaa tai debuggeria, näkyvä kaatuminen ilman debuggeria, lukupolku, 250 väliaikapistettä, HkKisaWin ja se, että pakattu tieto olisi muuta kuin nollia.

### 12. Mitä ei vielä tiedetä

- Lukupolku (`ReadFile`): kaatuuko ohjelma jo käynnistyksessä, jos se lukee jokaisen tietueen (Q37).
- Käyttäytyminen 250 väliaikapisteellä (tietue 11 200 tavua) ja muilla yhdistelmillä; vahvistus tehtiin vain 221 pisteellä.
- Käyttäytyminen ilman debuggeria ja ilman kääntäjän suojatoimintoa (`/GS`); käyttäjälle näkyvä kaatuminen on vain päätelty.
- HkKisaWin (bcc32): käyttäytymistä ei ole ajettu; käsin tehtävä testi on suunniteltu.
- Käytetäänkö vähintään 221 väliaikapisteen kokoonpanoja todellisissa kilpailuissa. Aineisto ei kerro.
- Voiko ylivuotavaan osaan päätyä verkosta tai laitteilta tullutta väliaikatietoa. Ylivuotava osa on viimeisen vaiheen väliaikatietoa [päätelty]; hyödynnettävyyttä ei ole tutkittu, ja siksi tietoturvan vaikutusrivi on ei tiedossa.
- Rekisteri liittää havaintoon kysymykset Q26–Q42 (Q26 ja Q30 ratkaistu, muut avoimia); täydellinen luettelo on luvussa 14. Kortin kannalta olennaisimmat: Q31 (onko viestin 250-käännöksessä vastaava raja), Q34 (ajetaanko tietueen rakentaja ennen kuin `valuku` on luettu, jolloin ylivuoto voisi syntyä keossa) ja Q37 (lukupolku käynnistyksessä).

### 13. Miten vika on tarkoitus korjata

Korjaussuunnitelman kokonaisuus KK-C13-11 (Tietueen kokorajat), ensimmäinen pull request: viisi pinopuskuria mitoitetaan suurimman tietueen mukaan tai tietue varataan keosta. Valinta tehdään toteutuksessa. Kekovaraus on suunnitelman mukaan varmempi, ellei pinon riittävyyttä mitata. `KILPBUFSIZE` ei saa olla tietuepituus, tiedostosiirtymä eikä pakettipituus. Korjaus ei muuta tiedostomuotoa. Hyväksymiskriteeri: tietue arvoilla 5 vaihetta ja 250 pistettä kirjoitetaan ja luetaan ilman ylivuotoa (ASan), ja olemassa oleva `KILP.DAT` luetaan tavu tavulta samoin. Nykyiset kehystestit (rajapari 220/221) kelpaavat.

Päätöstä odottavat osat: vaihtoehto, jossa liian suuri kokoonpano hylätään käynnistyksessä, muuttaisi nykyistä toimintaa ja on omistajan päätös (OP-12, avoin). Kokonaisuuden toinen osa (H-046) odottaa päätöstä OP-14 (kentän käyttötavat).

### 14. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | H-041 |
| Rekisterin kuvaus | HK: `char buf[KILPBUFSIZE]` = 10000, mutta toimitetuissa Release-käännöksissä (`VALUKU` 250, `N_PV` 5) tietue voi olla 11 200 tavua (n_pv 5, valuku ≥ 221; C-11 mitattu ajossa, aiemmin laskettu 11 210 / measured at run time, earlier computed 11 210) → pinon ylivuoto / stack overflow in shipped Release builds |
| Erä | C-3 |
| Tila | avoin |
| Vakavuus (rekisteri) | korkea / high |
| Varmuus (rekisteri) | [todennettu] koot ja konfiguraatiot / sizes and configs; [päätelty] saavutettavuus / reachability |
| Todennus (rekisteri) | riippumaton lähdetarkastus; **toistettu testikehyksessä / reproduced in test harness** (C-11: `kilprecsize` 11 200 > 10 000; `pack` kirjoittaa 11 200 tavua; ASan: stack-buffer-overflow WRITE `kilptietue::write`-funktion `buf`-muuttujassa (10 000 tavua); C-11 vaihe 4: ASan myös `kilptietue::addfrec`issa (H-041b4); **vahvistettu ajamalla / confirmed by running** toimitetussa HkMaali.exe:ssä (SHA-256 `61034e5d…`) konsolin lisäyspolulla rajaparilla 220/221 debuggerin alla: 221 → 0xC0000409/2 `addfrec`in evästetarkistuksessa, eväste, EBP, paluuosoite ja parametri nolliksi, KILP.DAT 20 080; 220 ehjä. Ennalta kiinnitetty arvio "not confirmed" (kriteeri 7: loki-ikkunaan osui kontrolliajon jälkikoeohjelma, pid 26236); taso käyttäjän erillisellä poikkeamapäätöksellä (`../testaus.md` 19.9–19.11). Ei näytetty: lukupolku (ReadFile), valuku 250, käytös ilman `/GS`:ää tai debuggeria / ASan stack-buffer-overflow in write and addfrec; confirmed by running in the shipped program by user-approved deviation; pre-fixed verdict was not confirmed; read path not shown) |
| Prioriteetti | P3 |
| Riskiarvio | vakavuus korkea; saavutettavuus asetus; todennus toimitettu_ohjelma; tulokset epasuora; ajanotto pysayttaa; palautettavuus kasin; tietoturva ei |
| Riskiarvion peruste | Only defect confirmed in a shipped program, but it needs n_pv 5 and valuku >= 221, a configuration fixed before the event, so P3 (unchanged). Confirmation used a user-approved deviation; read path and valuku 250 not shown; HK; confirmation strength neither lowers nor raises the rank. |
| Lähde (raportti) | `../tiedostomuodot.md`, `../testaus.md` |
| Näytön laji | staattinen + käännösaikainen koetin / static + compile-time probe + testiajo / test run TpCoreHk Release + Asan (H-041a, b3, b2, b4) + toimitetun ohjelman ajo debuggerin alla / shipped-program run under debugger (C-11 vaihe 4) |
| Juurisyy | JS-3 Kapasiteetti ei seuraa ajonaikaista kokoa; pahentavat: JS-14 Asetusten ja valitsinten ajautuminen |
| Ohjelmat | HkKisaWin HkMaali (peruste: tarkistettu) |
| Korjauskokonaisuus | KK-C13-11 Tietueen kokorajat; vaihe 1, aalto 1; haara `korjaus/kk-c13-11-tietueen-kokorajat` |
| Omistajan päätökset, joissa havainto mainitaan | OP-12 Tuetut tuotteet, kokoonpanot ja käyttöjärjestelmät (avoin) |
| Kokonaisuuden KK-C13-11 omistajan päätökset | OP-14 Kentän käyttötavat (tietopyyntö) (avoin) |
| Kysymykset | Q26 (ratkaistu), Q27 (avoin), Q28 (avoin), Q29 (avoin), Q30 (ratkaistu), Q31 (avoin), Q32 (avoin), Q33 (avoin), Q34 (avoin), Q35 (avoin), Q36 (avoin), Q37 (avoin), Q38 (avoin), Q39 (avoin), Q40 (avoin), Q41 (avoin), Q42 (avoin) |
| Liittyvät havainnot | - |
| Lähdeviitteet (revisio f7fe553, polut `TPsource/V52/`-hakemistosta) | `Hk/HkDef.h:45-58`, `:98-103`; `Hk/HkDat.cpp:176-177`, `:565`, `:583`, `:629`, `:1433`; `cbHk/UnitExport.cpp:464`; `Hk/HkSrj.cpp:156`; `VS/Hk/HkMaali/HkMaali.vcxproj`; `RADStudio10/HkKisaWin.cbproj:8` |

Rekisteririvien tiiviste nyt: `fe88bb796999`.


---

<a id="h-071"></a>

## H-071 – Verkosta tullut tiedostonsiirtosanoma voi kaataa ohjelman sekä luoda tai korvata tiedostoja ilman tarkistusta

> **H-071** · tila: **avoin** · prioriteetti: **P1** · tekninen vakavuus: korkea
> Ohjelmointivirheen todennus: luettu koodista ja riippumattomasti tarkastettu. Riskiarvion saavutettavuus: verkosta oletusasetuksin (tarkemmin luvussa 4).
> Ohjelmat: HkKisaWin, HkMaali, ViestiWin, ViestiMaali, ViestiLuenta (tarkistettu kortin kirjoituksen yhteydessä).
> Korjauskokonaisuus: KK-C13-1 Työasemaprotokollan vastaanoton rajatarkistukset (aalto 1).

### 1. Lyhyesti

Kun ohjelma ottaa vastaan tiedoston toiselta työasemalta (FILESEND-toiminto), se ei tarkista kahta asiaa: sanomassa ilmoitettua tiedostopalan pituutta eikä sitä, mihin tiedosto kirjoitetaan. Vastaanottokoodin kutsut on tarkistettu kaikissa viidessä ohjelmassa (luku 4), joten vika koskee niitä kaikkia. Vastaanotossa ei ole asetusta, jolla sen voisi kytkeä pois, eikä lähettäjää tunnisteta.

**Luettu koodista ja riippumattomasti tarkastettu:** molemmat puutteet. Laukeaminen on päätelty, ei ajettu.
**Vahvistettu käytännön seuraus:** ei yhtään. Mikään C-4:n uusista havainnoista ei ole vahvistettu ajamalla.
**Mahdollinen:** ohjelman kaatuminen tai muistin sisällön päätyminen tiedostoon; mikä tahansa ohjelman kirjoitettavissa oleva tiedosto voidaan luoda tai korvata; vajaa tiedosto välitetään muille työasemille.

### 2. Mitä ohjelma yrittää tehdä

Työasemat voivat siirtää tiedostoja toisilleen verkon kautta (LÄHTIEDOSTOT-ominaisuus; omistajan päätösasiakirjan OP-5 mukaan dokumentoitu ominaisuus). Lähettäjä lähettää tiedoston nimen ja sen jälkeen sisällön paloina. Vastaanottaja avaa nimetyn tiedoston, kirjoittaa palat siihen ja lopuksi tarvittaessa välittää tiedoston edelleen muille työasemille.

### 3. Mikä menee vikaan

Vastaanottaja luottaa sanomaan kahdessa kohdassa.

1. **Pituus.** Jokainen tiedostopala kertoo oman pituutensa. Ohjelma ei vertaa pituutta siihen, kuinka paljon tietoa vastaanottopuskuriin (ks. sanasto: Muistialue, puskuri) on oikeasti tullut. Jos ilmoitettu pituus on liian suuri, ohjelma lukee puskurin loppuun asti ja sen ohi. Seurauksena voi olla ohjelman kaatuminen tai se, että muistissa oleva muu tieto kirjoitetaan tiedostoon; kumpaakaan ei ole ajettu.
2. **Tiedoston nimi ja polku.** Ohjelma avaa sanomassa nimetyn tiedoston kirjoitettavaksi tarkistamatta polkua. Jos tiedosto on olemassa, sen sisältö korvataan. Verkosta voi siis luoda tai korvata minkä tahansa tiedoston, jonka ohjelma saa kirjoittaa.

Lisäksi C-9:n tarkastuksessa kirjattiin kolmas puute: jos jokin tiedostopala epäonnistuu tai tulee väärässä järjestyksessä, ohjelma sulkee tiedoston ilmoittamatta. Kun siirron loppusanoma tulee, ohjelma välittää vajaan tiedoston muille työasemille.

### 4. Missä tilanteessa vika voi ilmetä

- **Ohjelmat:** HkKisaWin, HkMaali, ViestiWin, ViestiMaali ja ViestiLuenta. Perusteena on, että vastaanottokoodi (`Tp/TpComY.cpp`) ja sen kutsut sekä HK:n (`Hk/HkCom32.cpp`) että viestin (`Juk/VCom.cpp`) vastaanottajassa käännetään näihin. Ohjelmia ei ole ajettu.
- **Asetus:** ainoa ehto on, että vastaanotto on käytössä ja yhteys on auki. Asetuslippua, joka estäisi vian, ei ole.
- **Lähettäjä:** protokollassa ei ole lähettäjän tunnistusta (H-029) eikä tarkistussummassa avainta. Sanoman voi siis lähettää mikä tahansa työasemaverkossa oleva laite tai ohjelma. Riskiarvion mukaan vika on tavoitettavissa verkosta oletusasetuksin; laukeaminen on [päätelty].
- **Normaalikäyttö vai poikkeustilanne:** aineisto ei kerro, tuottaako kelvollinen lähettäjä koskaan virheellisen pituuden. Laukeamiseen riittää virheellinen tai väärennetty sanoma, tai siirron epäonnistuminen kesken (C-9-puute).

### 5. Mitä käyttäjä voi huomata

Aineisto ei kerro varmasti, koska vikaa ei ole ajettu. Koodin perusteella mahdolliset oireet ovat:

- ohjelma kaatuu (liian suuri pituus);
- tiedosto muuttuu tai ilmestyy ohjelman kirjoittamaan paikkaan, vaikka käyttäjä ei ole sitä pyytänyt;
- toiselle työasemalle välittyy vajaa tiedosto.

Vika voi jäädä kokonaan huomaamatta, jos tiedosto korvataan tai vajaa tiedosto välitetään, koska ohjelma ei anna tästä ilmoitusta (ks. sanasto: Hiljainen virhe).

### 6. Vaikutukset kilpailuun

| Kysymys | Arvio | Peruste |
|---|---|---|
| Vaikutus: Voiko aika, tulos, sijoitus, pisteet tai hylkäys olla väärä? | Mahdollinen | Tiedostoja voi korvautua; riskiarvio: välillinen vaikutus. Tulosvaikutusta ei ole osoitettu. |
| Vaikutus: Voiko maali- tai väliaikojen kirjaus pysähtyä tai katketa? | Mahdollinen | Kaatuminen pysäyttäisi ajanoton (riskiarvio: pysäyttää); ei ajettu. |
| Vaikutus: Voiko leimaus- tai kortintiedon luku jäädä tekemättä tai mennä väärin? | Ei tiedossa | Aineisto ei kerro vaikutusta leimausten vastaanottoon. |
| Vaikutus: Voiko tallennettu tai vastaanotettu tieto kadota tai vioittua? | Mahdollinen | Verkosta voi luoda tai korvata minkä tahansa ohjelman kirjoitettavissa olevan tiedoston; vajaa tiedosto välitetään. |
| Vaikutus: Voiko ohjelma kaatua tai sulkeutua odottamatta? | Mahdollinen | Pituuden ylilukeminen puskurin ohi voi kaataa ohjelman; ei ajettu. |
| Vaikutus: Voiko ohjelma tai yksittäinen toiminto jäädä jumiin? | Ei tiedossa | Aineisto ei kerro jumiutumisesta. |
| Vaikutus: Voiko jokin mennä väärin ilman ilmoitusta tai näkyvää oiretta? | Mahdollinen | Epäonnistunut tai epäjärjestyksessä tullut pala sulkee tiedoston ilmoittamatta ja vajaa tiedosto välitetään. |
| Vaikutus: Voiko vika vaikuttaa tiedonsiirtoon tai muihin työasemiin? | Mahdollinen | Loppusanoma levittää tiedoston muille vertaisille; vajaa tiedosto voi levitä. |
| Vaikutus: Voiko vika vaikuttaa lukijoihin, kelloihin, näyttötauluihin tai tulostimiin? | Ei tiedossa | Aineisto ei kerro vaikutusta laitteisiin. |
| Vaikutus: Voiko vika vaikuttaa tulosteisiin, HTML- tai XML-tiedostoihin, tulospalveluun tai SQL-peiliin? | Mahdollinen | Mikä tahansa kirjoitettavissa oleva tiedosto voi korvautua, myös tuotetut tulostiedostot; ei osoitettu. |
| Vaikutus: Voiko ulkopuolinen tai verkossa oleva laite käyttää vikaa hyväkseen? | Mahdollinen | Verkosta tavoitettavissa oletusasetuksin ilman lähettäjän tunnistusta; laukeaminen päätelty, ei ajettu. |
| Ilmeneminen: Voiko vika ilmetä tavallisessa kilpailukäytössä ilman erityisasetusta tai virhettä? | Ei tiedossa | Aineisto ei kerro, tuottaako kelvollinen lähettäjä virheellisen pituuden. |
| Ilmeneminen: Voiko vika ilmetä poikkeustilanteessa (virhe, katkos, virheellinen syöte, harvinainen asetus)? | Mahdollinen | Virheellinen tai väärennetty sanoma tai kesken epäonnistunut siirto täyttää ehdon. |
| Ilmeneminen: Voiko käyttäjä huomata vian sen ilmetessä? | Ei tiedossa | Tiedoston korvaus ja vajaan tiedoston välitys tapahtuvat ilman ilmoitusta; kaatumisen näkyvyyttä ei ole ajettu. |
| Hallinta: Onko aineistossa kirjattu keino välttää vika tai pienentää sen riskiä? | Mahdollinen | Kirjatut suojaussuositukset: erillinen verkko, palomuurin sallittujen lista, vähimmät oikeudet; tehoa ei ole kokeiltu. |
| Hallinta: Onko aineistossa kirjattu keino palautua vian seurauksista kilpailun aikana? | Mahdollinen | Ainoa kirjattu keino on varmuuskopiot erilliseen paikkaan; riskiarvio kuitenkin: ei palautettavissa. Tehoa ei ole kokeiltu. |

Arvot: **Vahvistettu** = seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla; Mahdollinen = seuraa koodista, mutta seurausta ei ole osoitettu; Ei vaikutusta (perusteltu) = aineisto osoittaa, ettei seurausta synny; Ei tiedossa = aineisto ei ratkaise; _Ei arvioitu_ = riviä ei ole vielä arvioitu. Tiedon puute ei tarkoita, ettei vaikutusta ole. Lähteet: `doc/arkkitehtuuri/rekisteri/vaikutukset.csv`.

### 7. Mitä kilpailun aikana voi tapahtua

Vahvistetun ja mahdollisen erottelu on luvussa 11. Tilanne kilpailun aikana:

- Työasema, joka vastaanottaa virheellisen sanoman, voi kaatua, jolloin sen ajanotto pysähtyisi (riskiarvio: ajanotto pysähtyy). Kilpailun tiedostoja tai muita ohjelman kirjoitettavissa olevia tiedostoja voi korvautua. Vajaa tiedosto voi levitä muille työasemille.
- Riskiarvion mukaan vika vaikuttaa tuloksiin vain välillisesti. Tulosvaikutusta ei ole osoitettu.

### 8. Palautuminen ja uudelleenkäynnistys

Riskiarvio luokittelee palautettavuuden muotoon "ei palautettavissa". Aineistossa ei ole kirjattua keinoa palauttaa korvattu tiedosto ohjelman omin keinoin. Ainoa kirjattu liittyvä toimenpide on varmuuskopiot kilpailutiedostoista erilliseen paikkaan (`muistiturvallisuus.md` luku 15, suositus 3); sen tehoa ei ole kokeiltu. Aineisto ei kerro, auttaako ohjelman uudelleenkäynnistys kaatumisen jälkeen. Vika koskee jokaista työasemaa, joka vastaanottaa sanomia, ja vajaa tiedosto voi levitä muille työasemille.

### 9. Mitä käyttäjä voi tehdä riskin pienentämiseksi

Kirjatut väliaikaiset suojaussuositukset (`muistiturvallisuus.md` luku 15). Auditointi ei muuttanut verkko-, palomuuri- eikä ohjelma-asetuksia, eikä suositusten tehoa ole kokeiltu.

1. Ajanottoverkko erilleen: työasemat omaan suljettuun verkkoonsa tai VLANiin, ei yleisö-, vieras- tai julkista Wi-Fiä.
2. Palomuurin sallittujen lista: tiedonsiirtoportit (UDP/TCP) vain tunnettujen työasemien osoitteista.
3. Vähimmät oikeudet ja oma työhakemisto: ohjelma tavallisena käyttäjänä kilpailukohtaisessa hakemistossa, ei järjestelmänvalvojana; kilpailutiedostoista varmuuskopiot erilliseen paikkaan.

Luvun 16 mukaan tapahtumaverkkoa, jossa on muita laitteita tai yleisö, ei voi pitää turvallisena ennen korjauksia.

### 10. Tekninen juurisyy

Vastaanottofunktio `tark_tiedosto` (`Tp/TpComY.cpp`) käsittelee FILESEND-luokan sanoman. Sanoman sisäinen pituuskenttä (32-bittinen luku) annetaan sellaisenaan `fwrite`-kutsulle, eikä sitä verrata luokan pituuteen eikä vastaanottopuskurin `inbuf` kokoon. Siksi `fwrite` voi lukea puskurin ohi. Tiedosto avataan funktiolla `_wfopen` tilassa `"wb"` sanomassa olevalla nimellä ilman polkutarkistusta. Epäonnistuneen kirjoituksen tai väärässä järjestyksessä tulleen palan jälkeen tiedosto suljetaan ilman virheilmoitusta, mutta vastaanotetun määrän kirjanpito jää voimaan, joten loppusanoma välittää vajaan tiedoston eteenpäin (`lah_tiedosto`). Funktiota kutsutaan HK:n (`Hk/HkCom32.cpp`) ja viestin (`Juk/VCom.cpp`) pakettiluokkien käsittelystä.

Juurisyy on JS-1: ulkoista syötettä ei validoida rajalla. Vika löytyi C4389-varoitusrivin jäljityksestä.

### 11. Mitä on todennettu

**(a) Ohjelmointivirhe.** Koodi on luettu ja riippumaton tarkastus (C-4 vaihe 2) on vahvistanut sen. Todennuksen laji on staattinen. Rekisterin varmuusmerkintä: koodi [todennettu], laukeaminen [päätelty].

**(b) Käytännön seuraus.** Mitään seurausta ei ole osoitettu ajamalla tai testikehyksessä. `muistiturvallisuus.md` luvun 15 mukaan ainoa ajossa vahvistettu muistivika on H-041.

### 12. Mitä ei vielä tiedetä

- Mitä tapahtuu käytännössä: kaatuuko ohjelma, päätyykö muistin sisältöä tiedostoon, ja minne tiedosto kirjoittuu. Aineisto ei kerro. Ehdotettu myöhempi ajonaikainen tarkistus on yksi paketti H-071:lle ja H-072:lle eristetyssä HkMaali-instanssissa; sitä ei ole tehty, ja se vaatii erillisen luvan.
- Tuottaako kelvollinen lähettäjä virheellisen pituuden. Aineisto ei kerro.
- Rekisterissä ei ole tähän havaintoon liitettyä avointa kysymystä (Q).

### 13. Miten vika on tarkoitus korjata

Korjaussuunnitelman kokonaisuus KK-C13-1 (Työasemaprotokollan vastaanoton rajatarkistukset). Sisäinen pituus rajataan luokan pituuteen omana pull requestinaan. Tämä ei riko tiedostonsiirto-ominaisuutta. Tiedostopolun rajaus tehdään vasta päätöksen OP-5 jälkeen omana pull requestinaan, koska rajaus rikkoisi `/K`-valitsimen ja ylähakemistoon lähetyksen. Sanomamuoto ei muutu, joten korjattu ja korjaamaton ohjelma toimivat yhdessä. Korjaus tehdään sekä HK:n että viestin vastaanottopolkuun.

Päätöstä odottavat osat: OP-5 (FILESEND-tiedostonsiirron rajaus, avoin; vaihtoehdot: ei rajausta, vain työhakemisto oletuksena tai vastaanotto asetuksen taakse). Havainto mainitaan myös päätöksessä OP-1 (auditointitulosten julkaisu ja suojausohjeet).

### 14. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | H-071 |
| Rekisterin kuvaus | FILESEND-vastaanotto: paketin sisäistä pituutta `d.fl.len` (INT32) ei verrata luokan pituuteen → `fwrite` lukee vastaanottopuskurin `inbuf` yli (kaatuminen tai kasan sisältöä tiedostoon); `pos == -1` avaa paketissa nimetyn tiedoston tilassa `"wb"` ilman polkutarkistusta → verkosta voi luoda tai korvata minkä tahansa ohjelman kirjoitettavissa olevan tiedoston; `pos == -2` levittää tiedoston muille vertaisille; ei asetuslippua eikä lähettäjän tunnistusta (H-029), tarkistussummassa ei avainta / FILESEND: unchecked length and network-chosen file path **C-9:** epäonnistunut tai epäjärjestyksessä tullut lohko sulkee tiedoston ilman ilmoitusta, ja loppupaketti välittää vajaan tiedoston eteenpäin (`Tp/TpComY.cpp:3004-3033`, N-B7, H-156) / C-9: a partial file is forwarded |
| Erä | C-4 |
| Tila | avoin |
| Vakavuus (rekisteri) | korkea / high |
| Varmuus (rekisteri) | [todennettu] koodi; [päätelty] laukeaminen / trigger |
| Todennus (rekisteri) | riippumaton lähdetarkastus / independent source review |
| Prioriteetti | P1 |
| Riskiarvio | vakavuus korkea; saavutettavuus verkko_oletus; todennus riippumaton; tulokset epasuora; ajanotto pysayttaa; palautettavuus ei_palautettavissa; tietoturva verkko |
| Riskiarvion peruste | Unchanged: FILESEND length over-read and network-chosen file path; any file writable by the program can be created or replaced. Also forwards partial files (H-156 item N-B7). |
| Lähde (raportti) | `../muistiturvallisuus.md` |
| Näytön laji | staattinen; löytyi C4389-rivin (`Tp/TpComY.cpp:3022`) jäljityksestä / found by tracing a C4389 line |
| Juurisyy | JS-1 Ulkoista syötettä ei validoida rajalla |
| Ohjelmat | HkKisaWin HkMaali ViestiWin ViestiMaali ViestiLuenta (peruste: tarkistettu) |
| Korjauskokonaisuus | KK-C13-1 Työasemaprotokollan vastaanoton rajatarkistukset; vaihe 1, aalto 1; haara `korjaus/kk-c13-1-vastaanoton-rajatarkistukset` |
| Muut kokonaisuudet | KK-C10-4 |
| Omistajan päätökset, joissa havainto mainitaan | OP-1 Auditointitulosten julkaisu, push ja suojausohjeet (avoin), OP-5 FILESEND-tiedostonsiirron rajaus (avoin) |
| Kysymykset | - |
| Liittyvät havainnot | H-029 H-156 |
| Lähdeviitteet (revisio f7fe553, polut `TPsource/V52/`-hakemistosta) | `Tp/TpComY.cpp:1001`, `:2399`, `:3004-3037`; `Hk/HkCom32.cpp:939-940`, `:1105`; `Juk/VCom.cpp:968`, `:1240`; `Hk/HkDef.h:1301-1307`; `Juk/VDef.h:1247-1253` |

Rekisteririvien tiiviste nyt: `f2d646adedca`.


---

<a id="h-093"></a>

## H-093 – Kun lokiyhteys katkeaa, HK-ohjelma voi kaatua tai jumiutua lokia kirjoittaessaan

> **H-093** · tila: **avoin** · prioriteetti: **P2** · tekninen vakavuus: korkea
> Ohjelmointivirheen todennus: toistettu eristetyllä kokeella tai testikehyksessä. Riskiarvion saavutettavuus: tietyllä asetuksella tai kokoonpanolla (tarkemmin luvussa 4).
> Ohjelmat: HkKisaWin, HkMaali (tarkistettu kortin kirjoituksen yhteydessä).
> Korjauskokonaisuus: KK-C13-5 Lukitusjärjestys ja estävät kutsut lukon alla (writeerror_w) (aalto 2).

### 1. Lyhyesti

Henkilökilpailun ohjelmat (HK) voivat lähettää lokirivinsä TCP-yhteyden kautta. Jos tämä lokiyhteys
on poikki, jokaisen lokirivin kirjoitus epäonnistuu, ja virheen kirjaaminen yrittää kirjoittaa
uuden lokirivin samaan poikki olevaan yhteyteen. Tämä toistuu itsestään, kunnes ohjelman
käyttämä muistialue (pino) loppuu. Lisäksi kaksi säiettä voi jäädä odottamaan toisiaan ikuisesti.

- **Vahvistettu (eristetty koeohjelma):** rekursio päättyi pinon ylivuotoon, ja kahden säikeen
  keskinäinen lukkiutuminen toistui.
- **Mahdollinen (seuraa koodista, ei osoitettu toimitetussa ohjelmassa):** ohjelma kaatuu tai
  jäätyy kesken kilpailun, jolloin ajanotto pysähtyy. Prosessin kaatumista ei ajettu.
- Vika koskee vain HK:ta ja vain, kun lokiyhteys on otettu käyttöön asetuksella ja ohjelmalla on
  lisäksi toinen tiedonsiirtoyhteys. Viesti ei koske (luku 4).
- Vahvistetun ja mahdollisen tarkka erottelu on luvussa 11.

### 2. Mitä ohjelma yrittää tehdä

Ohjelma kirjoittaa toiminnastaan lokia, ja asetuksella `TCPL` loki lähetetään TCP-yhteyden
kautta. Kun lähetys lokiyhteyteen epäonnistuu, ohjelman pitäisi kirjata virhe, avata
lokiyhteys uudelleen ja jatkaa. Aineisto ei kerro, mihin lokiyhteyden toinen
pää käytännössä on tarkoitettu.

### 3. Mikä menee vikaan

Vikoja on kaksi, ja ne ovat saman rakenteen seurauksia.

**Rajaton rekursio.** Kun lokiyhteys on katkennut, jokainen lokirivi yrittää ensin lähettää
rivin, lähetys epäonnistuu, ja epäonnistumisen käsittely kirjoittaa siitä uuden lokirivin. Uusi
lokirivi yrittää lähettää itsensä samaan yhteyteen ja epäonnistuu samalla tavalla. Ohjelma ei
koskaan pääse siihen kohtaan, jossa yhteys avattaisiin uudelleen. Kierros toistuu, kunnes pino
loppuu (ks. [pino](../sanasto.md)); tämän jälkeen ohjelman odotetaan kaatuvan, mutta
kaatumista ei ole ajettu (luku 11). Vertaus: kirjuri kirjaa virheen
kirjanpitoon, kirjaus epäonnistuu samasta syystä, ja hän kirjaa tästä uuden virheen, ja niin
edelleen.

**Kahden säikeen lukkiutuminen.** Ohjelmassa on kaksi lukkoa: toinen lokin kirjoitukselle ja
toinen TCP-lähetyksille. Lukko on vuoronumeron kaltainen järjestely, jonka haltija saa yksin
käyttää tietoa (ks. [lukko](../sanasto.md), [säie](../sanasto.md)). Lokia kirjoittava
säie ottaa ensin lokin lukon ja sitten lähetyksen lukon. TCP-lähetyksessä epäonnistuva säie
ottaa ne päinvastaisessa järjestyksessä: sillä on lähetyksen lukko, ja virheen kirjaus vaatii
lokin lukon. Jos molemmat ovat liikkeellä yhtä aikaa, kumpikin pitää sitä lukkoa, jota toinen
odottaa, eikä kumpikaan pääse eteenpäin ([lukkiutuminen](../sanasto.md)).
Tämä toinen vika vaatii, että lähetys epäonnistuu todellisella lähetys- tai valintavirheellä
lähetyksen lukon sisällä. Pelkkä poikki oleva lokiyhteys käynnistää rekursion mutta ei tätä
lukkiutumista; tarkat ehdot ovat luvussa 10.

### 4. Missä tilanteessa vika voi ilmetä

- **Ohjelmat:** HK. Lähdekoodi on yhteistä HkKisaWinille ja HkMaalille, ja koeohjelma rakennettiin
  HK:n yhteisestä ytimestä. Ero GUI- ja konsoliohjelman välillä (bcc32 vs. MSVC) on
  tarkistamatta (päätelty yhteisestä lähteestä).
- **Viesti ei koske:** viestiohjelmat eivät voi ottaa TCP-lokia käyttöön, koska viestin
  käynnistyskoodi ei koskaan aseta TCP-lokin asetusarvoa (lähdekoodin luku, C-6 vaihe 2).
- **Asetus:** yhteysasetuksen tyyppi `TCPL` (TCP-loki).
- **Toinen yhteys:** vähintään yksi muu tiedonsiirtoyhteys on avattu. Rekursio vaatii tämän:
  polun tarkistama ehto asetetaan vain varsinaisten tiedonsiirtoyhteyksien avauksessa, ei
  lokiyhteyden avauksessa.
- **Uudelleenyhteys:** rekursio vaatii lisäksi, että lokiyhteyden uudelleenyhteysasetus on
  käytössä ja että yhteys ei ole palvelinpuolen yhteys (tekniset ehdot luvussa 10).
- **Laukaisu:** lokiyhteys on poikki (vastaanotto tai lähetys on havainnut katkon). Rekursio ei
  tarvitse erillistä pistokevirhettä.
- Tilanne on asetukseen sidottu poikkeustilanne (katkos), ei tavallinen kilpailukäyttö ilman
  asetusta. Aineisto ei kerro, kuinka moni asennus käyttää TCP-lokia.

### 5. Mitä käyttäjä voi huomata

Aineisto ei kerro, mitä käyttäjä näkee ennen kaatumista tai jumiutumista. Koodista seuraa, että
ohjelma voi lopettaa toimintansa kaatuessaan (pinon ylivuoto) tai että yksittäiset toiminnot
voivat odottaa lukkoa, jota ei vapauteta. Se, voiko vika jäädä huomaamatta, ei ole tiedossa.

### 6. Vaikutukset kilpailuun

| Kysymys | Arvio | Peruste |
|---|---|---|
| Vaikutus: Voiko aika, tulos, sijoitus, pisteet tai hylkäys olla väärä? | Mahdollinen | Riskiarvion mukaan vaikutus tuloksiin on epäsuora: kaatuva tai jumiutuva ohjelma ei kirjaa; tarkkaa polkua ei ole osoitettu. |
| Vaikutus: Voiko maali- tai väliaikojen kirjaus pysähtyä tai katketa? | Mahdollinen | Kaatuminen ja kahden säikeen lukkiutuminen on osoitettu koeohjelmassa, mutta ajanoton pysähtymistä ei ole osoitettu toimitetussa ohjelmassa. |
| Vaikutus: Voiko leimaus- tai kortintiedon luku jäädä tekemättä tai mennä väärin? | Ei tiedossa | Aineisto ei kerro, vaikuttaako kaatuminen tai jumiutuminen leimausten lukuun. |
| Vaikutus: Voiko tallennettu tai vastaanotettu tieto kadota tai vioittua? | Ei tiedossa | Aineisto ei kerro, katoaako tietoa kaatumisessa. |
| Vaikutus: Voiko ohjelma kaatua tai sulkeutua odottamatta? | Mahdollinen | Koeohjelma sieppasi pinon ylivuodon rekursiossa, mutta prosessin kaatumista ei ajettu; GUI-ohjelmassa seuraus voi olla virheikkuna (päätelty). |
| Vaikutus: Voiko ohjelma tai yksittäinen toiminto jäädä jumiin? | **Vahvistettu** | Koeohjelmassa tuotannon funktiot jättivät kaksi säiettä odottamaan toisiaan; yhteystietue asetettiin muistiin käsin; toimitettua ohjelmaa ei ajettu. |
| Vaikutus: Voiko jokin mennä väärin ilman ilmoitusta tai näkyvää oiretta? | Ei tiedossa | Aineisto ei kerro, voiko vika tuottaa väärän tuloksen ilman oiretta; kaatuminen ja jumiutuminen eivät ole hiljaisia. |
| Vaikutus: Voiko vika vaikuttaa tiedonsiirtoon tai muihin työasemiin? | Mahdollinen | Jumiutunut säie voi pitää TCP-lähetyksen lukkoa, jolloin kyseisen ohjelman muut lähetykset odottavat; päätelty lukkojen rakenteesta, ei kokeiltu. |
| Vaikutus: Voiko vika vaikuttaa lukijoihin, kelloihin, näyttötauluihin tai tulostimiin? | Ei tiedossa | Aineisto ei käsittele vaikutusta lukijoihin, kelloihin, tauluihin tai tulostimiin. |
| Vaikutus: Voiko vika vaikuttaa tulosteisiin, HTML- tai XML-tiedostoihin, tulospalveluun tai SQL-peiliin? | Ei tiedossa | Aineisto ei käsittele vaikutusta tulosteisiin, HTML- tai XML-tiedostoihin tai tulospalveluun. |
| Vaikutus: Voiko ulkopuolinen tai verkossa oleva laite käyttää vikaa hyväkseen? | Ei tiedossa | Riskiarvio sanoo tietoturvasta ei, mutta perustetta ei ole kirjattu; aineisto ei erittele, voiko verkon puoli aiheuttaa lokiyhteyden katkon. |
| Ilmeneminen: Voiko vika ilmetä tavallisessa kilpailukäytössä ilman erityisasetusta tai virhettä? | Ei vaikutusta (perusteltu) | Vika edellyttää TCPL-asetusta ja toista tiedonsiirtoyhteyttä, joten se ei ilmene ilman erityisasetusta. |
| Ilmeneminen: Voiko vika ilmetä poikkeustilanteessa (virhe, katkos, virheellinen syöte, harvinainen asetus)? | **Vahvistettu** | Rekursio käynnistyy, kun lokiyhteys on poikki ja asetukset ovat edellä mainitut; osoitettu koeohjelmassa tilan asettamisella käsin. |
| Ilmeneminen: Voiko käyttäjä huomata vian sen ilmetessä? | Mahdollinen | Kaatuminen tai jumiutuminen seuraa koodista ja on käyttäjälle näkyvä; aineisto ei kerro, mitä ruudulla näkyy. |
| Hallinta: Onko aineistossa kirjattu keino välttää vika tai pienentää sen riskiä? | Ei tiedossa | Aineistossa ei ole kirjattua keinoa; muistiturvallisuus.md luku 15 ei koske havaintoa. |
| Hallinta: Onko aineistossa kirjattu keino palautua vian seurauksista kilpailun aikana? | Mahdollinen | Riskiarvion palautettavuus on uudelleenkäynnistys; tehoa ei ole kokeiltu eikä kerrota, toimiiko ohjelma poikki olevan lokiyhteyden kanssa. |

Arvot: **Vahvistettu** = seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla; Mahdollinen = seuraa koodista, mutta seurausta ei ole osoitettu; Ei vaikutusta (perusteltu) = aineisto osoittaa, ettei seurausta synny; Ei tiedossa = aineisto ei ratkaise; _Ei arvioitu_ = riviä ei ole vielä arvioitu. Tiedon puute ei tarkoita, ettei vaikutusta ole. Lähteet: `doc/arkkitehtuuri/rekisteri/vaikutukset.csv`.

### 7. Mitä kilpailun aikana voi tapahtua

Kilpailussa, jossa HK-ohjelmalla on lokiyhteys (`TCPL`) ja toinen tiedonsiirtoyhteys:

- Kun lokiyhteyden tila on "poikki", lokirivit käynnistävät rekursion, joka päättyy pinon
  ylivuotoon. Toimitetussa ohjelmassa ylivuoto voisi kaataa koko prosessin; GUI-ohjelmassa
  seuraus voisi olla virheikkuna [päätelty].
- Lokia kirjoittava säie ja TCP-lähetyksessä epäonnistuva säie voivat jäädä kumpikin
  odottamaan toisen pitämää lukkoa. Lukkiutuminen vaatii lukon sisällä todellisen
  lähetysvirheen (luku 10).
- Kaatuminen tai jumiutuminen pysäyttäisi ajanoton (riskiarvio). Samaa lähetyslukkoa
  käyttävät muutkin tiedonsiirtolähetykset kyseisessä ohjelmassa voisivat odottaa pysyvästi,
  jos lukkiutuminen pitää lähetyslukkoa [päätelty lukkojen rakenteesta, ei kokeiltu].

Mikä näistä on osoitettu koeohjelmassa ja mikä vain seuraa koodista, on luvussa 11.

Tuloksiin kohdistuvat seuraukset ovat riskiarvion mukaan epäsuoria. Aineisto ei kuvaa
tarkemmin, miten tulos voisi mennä väärin.

### 8. Palautuminen ja uudelleenkäynnistys

Riskiarvion palautettavuus on "uudelleenkäynnistys". Aineisto ei kerro, miten ohjelma
käyttäytyy uudelleenkäynnistyksen jälkeen, jos lokiyhteys on yhä poikki. Muiden työasemien
ohjelmat: aineisto ei kerro, vaikuttaako yhden työaseman jumiutuminen tai kaatuminen niihin
muuten kuin siten, että kyseinen työasema lakkaa lähettämästä.

### 9. Mitä käyttäjä voi tehdä riskin pienentämiseksi

Aineistossa ei ole kirjattua keinoa. Väliaikaiset suojaussuositukset (`muistiturvallisuus.md`
luku 15) eivät koske tätä havaintoa.

### 10. Tekninen juurisyy

Juurisyy on JS-4 (sivuvaikutukset globaalien lukkojen alla).

- `wkirjloki` (`Hk/HkApu.cpp`) pitää lokin lukkoa (`loki_CS`) ja ottaa kirjoittaessaan
  lähetyksen lukon (`tcpip_CS`).
- `wrt_st_TCP` (`com/WinUDP.cpp`) pitää lähetyksen lukkoa lähetyksen ajan. Lukon sisällä
  `select`- tai `send`-kutsun todellinen virhe, jonka koodi on muu kuin 10054, saa sen
  kutsumaan `writesocketerror`ia, joka kutsuu `wkirjloki`a. Vain tässä polussa lukkojen
  järjestys on päinvastainen kuin lokin kirjoituksessa (ABBA-lukkiutuminen); 10054-virhe
  suodatetaan pois (`writesocketerror`).
- Poikki-tilan kutsu (`Reconnect TCP`) tehdään lähetyksen lukon ulkopuolella, joten se ei
  tuota lukkiutumista vaan rekursion. Rekursio vaatii, että lokiyhteyden `reconnect` on
  nollasta poikkeava ja `serverfl` on 0, ja että `comfl` on asetettu (toisen
  tiedonsiirtoyhteyden avauksessa). Rekursiossa `writesocketerror` saa virhekoodin 0, joten
  suodatin (virhe 10054) ei pysäytä sitä. Uudelleentulolle ei ole estoa, `lokiTCP`-arvoa ei
  nollata, eikä `reconnectTCP`:tä saavuteta, koska se on rekursion jälkeen. Rekursio ei siis
  tarvitse varsinaista sokettivirhettä, vain tilan "poikki" (`connected == 0`).
- Raportti mainitsee kierroksen kooksi noin 2 kt pinoa ja pinon kooksi 1 Mt (501 kierrosta
  koeohjelmassa). Lokisäikeen pino on raportin mukaan 40 kt ja muiden 1 Mt; aineisto ei kerro,
  mikä säie käytännössä kirjoittaa lokia.

### 11. Mitä on todennettu

**(a) Ohjelmointivirhe.** Toistettu eristetyllä koeohjelmalla (C6ProbeHk, C-6 vaihe 2;
`todisteet/c6/vaihe2/`), joka kutsuu tuotannon funktioita sellaisenaan MSVC:llä. Kaksi täyttä
ajoa antoi samat tulokset.
- Rekursio: pinon ylivuoto (virhekoodi `0xC00000FD`) 501 sisäkkäisen `wkirjloki`-kutsun jälkeen;
  `reconnectTCP` ei koskaan saavutettu. Koeohjelma sieppasi ylivuodon; prosessin kaatumista ei
  ajettu. Vertailukoe (`reconnect=0`) palasi normaalisti.
- Lukkiutuminen: lokin lukon omistaja X, lähetyksen lukon omistaja Y, kummallakin yksi odottaja,
  molemmat säikeet elossa.
- Viestin kumoaminen: lähdekoodin luku, ei koetta.

**(b) Käytännön seuraus.** Koeohjelma ei todista seurauksia kilpailussa. Yhteystietue kirjoitettiin
muistiin käsin (ei oikeaa soketin avausta eikä oikeaa katkosta), "yhdistetty"-tila asetettiin
käsin eikä ajoitusta mallinnettu. Toimitettua ohjelmaa ei ole ajettu, joten kaatumista tai
jumiutumista oikeassa kilpailussa ei ole osoitettu.

### 12. Mitä ei vielä tiedetä

- Q66 (pysyykö lokisoketin virhe vai muuttuuko se): ratkaistu. Rekursio ei tarvitse
  sokettivirhettä (luku 10).
- Todentamatta: milloin `comfl`-ehto toteutuu oikealla kokoonpanolla (seurannan jatkotehtävä),
  oikeiden soketien ja ajoituksen vaikutus, sekä bcc32:lla käännetyn GUI-ohjelman
  kutsukehysten koko (pinon ylivuodon raja voi poiketa).
- Mitä käyttäjä näkee ennen kaatumista tai jumiutumista.
- Riskiarvion tietoturva-arvo "ei": perustetta ei ole kirjattu, eikä aineisto erittele, voiko
  verkon puoli aiheuttaa lokiyhteyden katkon. Vaikutusrivi on siksi ei tiedossa.

### 13. Miten vika on tarkoitus korjata

Korjausmallissa havainto kuuluu kokonaisuuteen KK-C13-5 (Lukitusjärjestys ja estävät kutsut
lukon alla). Havainnon osalta suunnitelma sanoo: lokitus ei ota soketin lukkoa lokilukon alla,
eikä lähetysvirhe kirjaa lokiin rekursiivisesti. Se on oma PR:nsä (PR 1), ja testi on koeohjelma
siirrettynä testeihin. Hyväksymiskriteeri on, ettei lähetysvirhe aiheuta rekursiota.
Suunnitelma varoittaa, että lukitusjärjestyksen muutos voi siirtää lukkiutumisen toiseen
kohtaan ja että muutos todennetaan GUI-ajolla ennen julkaisua. Rakenteellinen osa
(dokumentoitu lukitusjärjestys) tehdään myöhemmin. Havaintoon ei liity omistajan päätöstä
(OP).

### 14. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | H-093 |
| Rekisterin kuvaus | LOKI TCP:n kautta: `wkirjloki` pitää `loki_CS`:ää ja ottaa `tcpip_CS`:n; `wrt_st_TCP` pitää `tcpip_CS`:ää ja lokittaa lähetysvirheen (`writesocketerror` → `wkirjloki`) → ABBA minkä tahansa lokittavan ja minkä tahansa TCP-lähetyksessä epäonnistuvan säikeen välillä; jos epäonnistuva soketti on lokisoketti, sama säie rekursoi ilman rajaa → pinon ylivuoto / ABBA loki↔tcpip and unbounded recursion C-6 vaihe 2: rekursio ei tarvitse sokettivirhettä: kun lokiyhteys on katki (`connected==0`) ja `comfl` päällä (toinen tiedonsiirtoyhteys), jokainen lokirivi rekursoi `:938-942` → pinon ylivuoto ennen `reconnectTCP`:tä (Q66 ratkaistu); koeohjelmassa 501 tasoa / 1 Mt; myös todellinen kahden säikeen lukkiutuminen toistettu; **koskee vain HK:ta**: viesti ei voi asettaa TCP-lokia (`Juk/VInit.cpp:1064-1091`) / step 2: deterministic recursion to stack overflow reproduced; two-thread deadlock reproduced; HK only, relay refuted |
| Erä | C-6 |
| Tila | avoin |
| Vakavuus (rekisteri) | korkea / high (HK, TCP-loki + muu yhteys / TCP log + another link); viesti ei koske / relay not affected |
| Varmuus (rekisteri) | [todennettu] polut ja rekursion rajattomuus / paths and unbounded recursion |
| Todennus (rekisteri) | riippumaton lähdetarkastus / independent source review (C-6); **toistettu eristetyllä koeohjelmalla / reproduced with an isolated probe** (C-6 vaihe 2: pinon ylivuoto 501 tasolla, kahden säikeen lukkiutuminen; yhteystietue asetettu muistiin, ei oikeaa sokettia / connection record set in memory, no real socket) |
| Prioriteetti | P2 |
| Riskiarvio | vakavuus korkea; saavutettavuus asetus; todennus koe_tai_testikehys; tulokset epasuora; ajanotto pysayttaa; palautettavuus uudelleenkaynnistys; tietoturva ei |
| Riskiarvion peruste | Unchanged: TCP log with another link recurses to stack overflow whenever the log link is down; reproduced. HK only; relay refuted. |
| Lähde (raportti) | `../saikeet-ja-lukot.md` |
| Näytön laji | staattinen, lähdekoodi / static, source + testiajo / test run C6ProbeHk (C-6 vaihe 2, `todisteet/c6/vaihe2/`) |
| Juurisyy | JS-4 Sivuvaikutukset globaalien lukkojen alla |
| Ohjelmat | HkKisaWin HkMaali (peruste: tarkistettu) |
| Korjauskokonaisuus | KK-C13-5 Lukitusjärjestys ja estävät kutsut lukon alla (writeerror_w); vaihe 1, aalto 2; haara `korjaus/kk-c13-5-lukitusjarjestys` |
| Omistajan päätökset, joissa havainto mainitaan | - |
| Kysymykset | Q66 (ratkaistu) |
| Liittyvät havainnot | - |
| Lähdeviitteet (revisio f7fe553, polut `TPsource/V52/`-hakemistosta) | `Hk/HkApu.cpp:560`, `:576`, `:583`; `com/WinUDP.cpp:907-936`, `:915`, `:923`, `:175-183`; `Juk/vApu.cpp:94-119`; `Tp/TpComY.cpp:2254`; `com/WinUDP.cpp:938-943`, `:931-932`, `:603`; `Tp/TpComY.cpp:2204`, `:2238`; `Hk/HkInit.cpp:1133-1137` |

Rekisteririvien tiiviste nyt: `4a589e03f168`.


---

<a id="h-099"></a>

## H-099 – Epäilty: HK:n käynnistys jättäisi saapuvien sanomien lukon varatuksi (kumottu)

> **H-099** · tila: **kumottu** · vakavuus: kumottu
> Ohjelmat: HkKisaWin, HkMaali, ViestiWin, ViestiMaali, ViestiLuenta (yläraja: ohjelmat, joihin viitatut lähdetiedostot käännetään).

### 1. Lyhyesti

Auditoinnin ensimmäisessä vaiheessa epäiltiin, että HK-ohjelman käynnistys voisi epäonnistua
tiedonsiirron alustuksessa ja jättää samalla yhden [lukon](../sanasto.md) varatuksi, jolloin
sanomien vastaanotto jäisi jumiin. Jatkotarkastuksessa selvisi, että kyseinen epäonnistumispolku
ei ole mahdollinen: ohjelmakoodin haara, jossa lukko jäisi varatuksi, ei koskaan suoritu.
Epäily oli väärä, eikä siitä seuraa vikaa.

### 2. Mitä alun perin väitettiin

C-6-erän (säikeet ja lukot) ensimmäisessä vaiheessa kirjattiin, että HK:n käynnistysrutiini
`aloitus` palaisi virhekoodilla 1 niin, että saapuvien sanomien lukko (`inb_CS`) jää varatuksi,
jos tiedonsiirron alustus (`initcom`) epäonnistuu varauksen virheeseen. Tämän seurauksena GUI
jatkaisi rajoitetussa tilassa, ja `tarkcom` sekä jo käynnistetyt vastaanottosäikeet jäisivät
odottamaan lukkoa. Ensimmäisessä vaiheessa väite luokiteltiin vahvistetuksi lähdekoodin
luvun perusteella. Viestiohjelmien vastaavaa kohtaa ei rekisterin todennuskentän mukaan
tarkastettu tässä yhteydessä.

### 3. Miksi väite kumottiin

C-6-erän toisessa vaiheessa riippumaton lähdetarkastus kumosi väitteen, ja pääistunto vahvisti
kumoamisen. Menetelmä oli lähdekoodin luku; ohjelmaa ei ajettu. Peruste: `initcom` palauttaa
kaikilla poluillaan aina arvon 0. Koska `aloitus` palauttaa virheen vain, kun `initcom` palauttaa
nollasta poikkeavan arvon, `return(1)`-haara on kuollut koodi (koodi, joka ei koskaan
suoritu). Lukkoa ei siis koskaan jää varatuksi tässä kohdassa.

### 4. Mitä tämä tarkoittaa käyttäjälle

Käyttäjälle ei ole seurausta: väitettyä lukkiutumista ei synny. Raportin mukaan viestiohjelman
vastaava kohta (`Juk/VInit.cpp`) on tasapainossa: lukko varataan ja vapautetaan oikein.

Samaan käynnistysvaiheeseen liittyy kuitenkin voimassa oleva, erillinen asia. Käynnistys
(`aloitus`) voi ajautua uudelleen samassa GUI-prosessissa, jos ensimmäinen ajo epäonnistuu
varhain, ja silloin se ei ole turvallisesti uudelleen ajettava (H-133, Q67). Se ei ole sama asia
kuin tämä kumottu epäily.

### 5. Mitä ei tiedetä

- Kumoaminen koskee vain `initcom`in paluuarvoa ja sen seurauksena kuollutta haaraa. Q67:n
  osa, jossa `aloitus` ajetaan uudelleen epäonnistuneen käynnistyksen jälkeen, on osittain
  avoin: kutsuttujen funktioiden sisäiset apusäikeet ennen epäonnistumiskohtaa on tarkastamatta.
- Aineisto ei kerro, tarkastettiinko viestiohjelmien vastine ajamalla vai lukemalla. Rekisterin
  todennuskenttä sanoo "viesti tarkastamatta", mutta kuvaus ja raportti sanovat vastineen
  olevan tasapainossa.

### 6. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | H-099 |
| Rekisterin kuvaus | HK `aloitus` palaa 1 `inb_CS` varattuna, jos `initcom` epäonnistuu (varausvirhe); GUI jatkaa rajoitetussa tilassa, ja `tarkcom` sekä jo käynnistetyt vastaanottosäikeet estyvät / returns holding `inb` on init failure **KUMOTTU C-6 vaiheessa 2 / REFUTED in C-6 step 2:** `initcom` palauttaa aina 0, joten `return(1)`-haara on kuollut koodi; viestin vastine on tasapainossa / `initcom` always returns 0, the branch is dead |
| Erä | C-6 |
| Tila | kumottu |
| Vakavuus (rekisteri) | ei vika (kumottu) / not a defect (refuted) |
| Varmuus (rekisteri) | [todennettu] |
| Todennus (rekisteri) | riippumaton lähdetarkastus / independent source review (C-6; viesti tarkastamatta / relay not checked); C-6 vaihe 2: kumottu riippumattomassa tarkastuksessa, pääistunto vahvisti / refuted by independent review, confirmed by the main session |
| Lähde (raportti) | `../saikeet-ja-lukot.md` |
| Näytön laji | staattinen, lähdekoodi / static, source |
| Juurisyy | - |
| Ohjelmat | HkKisaWin HkMaali ViestiWin ViestiMaali ViestiLuenta (peruste: tiedosto) |
| Omistajan päätökset, joissa havainto mainitaan | - |
| Kysymykset | - |
| Liittyvät havainnot | - |
| Lähdeviitteet (revisio f7fe553, polut `TPsource/V52/`-hakemistosta) | `Hk/HkInit.cpp:3650`, `:3659-3662`; `Tp/TpComY.cpp:2422`; `cbHk/WinHk.cpp:279-283`; `Hk/HkCom32.cpp:885`; `Tp/TpComY.cpp:2376-2426` |

Rekisteririvien tiiviste nyt: `a0817f66a2b2`.


---

<a id="h-109"></a>

## H-109 – "Poista kilpailija" ilman valittua kilpailijaa voi vioittaa tietueiden listan ja sekoittaa myöhemmin lisättyjä kilpailijoita

> **H-109** · tila: **avoin** · prioriteetti: **P2** · tekninen vakavuus: korkea
> Ohjelmointivirheen todennus: luettu koodista ja riippumattomasti tarkastettu. Riskiarvion saavutettavuus: käyttäjän toimenpiteestä (tarkemmin luvussa 4).
> Ohjelmat: HkKisaWin, ViestiWin (tarkistettu kortin kirjoituksen yhteydessä).
> Korjauskokonaisuus: KK-C13-14 GUI: dataa korruptoivat toiminnot (aalto 2).

### 1. Lyhyesti

HkKisaWinin ja ViestiWinin valikkotoiminto "Poista kilpailija" ei tarkista, onko lomakkeella
oikeasti valittu kilpailija, eikä sitä, missä tilassa lomake on. Jos poisto tehdään, kun mitään
kilpailijaa ei ole valittu, tai kun uuden kilpailijan lisäys on juuri peruttu, ohjelma "poistaa"
tietueen, jota ei ole. Tämä voi hävittää tiedoston vapaiden paikkojen luettelon tai sotkea sen
niin, että seuraavaksi lisättävä kilpailija katoaa tai kaksi kilpailijaa saa saman tietueen.

Vahvistettu: ei mitään seurausta. Vika on luettu lähdekoodista ja riippumaton tarkastaja on
vahvistanut koodiluennan, mutta ohjelmaa ei ole ajettu tämän vian osalta. Kaikki tässä kuvatut
seuraukset ovat siis mahdollisia: ne seuraavat koodista, mutta niitä ei ole nähty ajossa.

### 2. Mitä ohjelma yrittää tehdä

Käyttäjä valitsee kilpailijan tietoja näyttävässä lomakkeessa valikosta "Poista kilpailija".
Ohjelma kysyy vahvistuksen ("Poista näytetty kilpailija", Kyllä/Ei). Kun käyttäjä vastaa
Kyllä, ohjelma poistaa näytetyn kilpailijan: se poistaa kilpailijan hakuindekseistä, merkitsee
hänen tietuepaikkansa vapaaksi ja lähettää poiston muille työasemille, jos tiedonsiirto on
käytössä. Vapaaksi merkitty paikka kirjataan tiedoston vapaiden paikkojen luetteloon, ja seuraava
lisättävä kilpailija sijoitetaan siihen.

### 3. Mikä menee vikaan

Vahvistuskysely tulee, mutta ohjelma ei tarkista, mikä kilpailija on "näytettynä". Lomakkeella on
muistissa tietuenumero, ja sen arvo voi olla jotain muuta kuin oikea kilpailija.
Aineisto kuvaa kolme tilannetta (ks. sanasto: Indeksi, Tietue).

1. **Lomake avattu, kilpailijaa ei valittu.** Tietuenumero on tällöin -1. Henkilökilpailun
   ohjelmassa (HkKisaWin) tallennus tarkistaa vain, ettei numero ole liian suuri, joten ohjelma
   kirjoittaa kilpailijataulukon alkua edeltävään muistipaikkaan. Lisäksi tiedostokerros kirjaa
   "vapaan paikan" numeroksi -1, jolloin koko vapaiden paikkojen luettelo katoaa.
2. **Lisäys aloitettu ja peruttu ("Lisää" ja sitten "Peruuta").** Tietuenumeroksi jää 0. Poisto
   vie silloin tiedoston otsaketietueen vapaiden paikkojen luetteloon. Seuraavaksi lisätty
   kilpailija katoaa.
3. **Kilpailutilassa oletusselaus "Sijoitus".** Selaus ei siirry pois juuri poistetusta
   kilpailijasta, joten käyttäjä voi painaa "Poista" uudelleen samalle tietueelle. Toinen poisto
   linkittää tietueen itseensä vapaiden luettelossa. Tämän jälkeen kaksi seuraavaa lisättyä
   kilpailijaa saa saman tietueen, ja kolmas saa päätellysti otsaketietueen [päätelty].

ViestiWinissä (joukkuelomake) vastaava poisto toimii samalla tavalla. Siellä poistossa käytettävä
paikallinen tietue on lisäksi alustamaton (T-035), joten luku voi jättää siihen satunnaista tietoa.

### 4. Missä tilanteessa vika voi ilmetä

- Ohjelmat: HkKisaWin (kilpailijatietolomake) ja ViestiWin (joukkuetietolomake). Laukaisu on
  lomakkeen valikkotoiminto. Muissa ohjelmissa (HkMaali, ViestiMaali, ViestiLuenta) tätä
  laukaisuketjua ei ole tarkistettu.
- Laukaisija on käyttäjän oma toimenpide: poisto ennen kilpailijan valintaa, poisto lisäyksen
  perumisen jälkeen tai toistettu poisto kilpailutilassa. Mitään erityisasetusta tai laitetta
  aineisto ei vaadi. Verkosta tai laitteelta vika ei laukea.
- Samaan poistorutiiniin pääsee myös luettelopohjaisesta poistosta: kilpailija- tai
  joukkueluettelon rivillä tehtävästä pysyvästä poistosta (HkKisaWinin osanottajaluettelo ja
  ViestiWinin joukkueluettelo; `globaali-tila.md` luku 5). Aineisto ei kerro, mitkä tietuenumerot
  tämä polku voi antaa poistolle; sitä ei ole ajettu.
- Kyse on käyttäjän virhetoimenpiteestä lomakkeessa, ei virhetilanteesta ohjelmassa. Yleisyydestä
  aineisto ei kerro mitään.

### 5. Mitä käyttäjä voi huomata

Aineisto ei kuvaa, mitä käyttäjä ensimmäisenä näkee. Koodista seuraa:

- Vahvistuskysely näkyy normaalisti, eikä mikään siinä kerro, että poistettavaa kilpailijaa ei
  ole.
- Tilanteessa 1 HK:n tietueen luku antaa koodin mukaan virheilmoituksen "Ohjelma yrittää hakea
  olematonta kilpailijaa", mutta poisto jatkuu silti. Ohjelma voi kaatua (päätelty koodista, ei
  ajettu).
- Tilanteissa 2 ja 3 aineisto ei kirjaa mitään ilmoitusta. Oire näkyy vasta myöhemmin: lisätty
  kilpailija puuttuu tai kahden kilpailijan tiedot ovat samassa tietueessa.
- Vika voi siis jäädä huomaamatta siihen asti, kun lisätty kilpailija yritetään hakea.

### 6. Vaikutukset kilpailuun

| Kysymys | Arvio | Peruste |
|---|---|---|
| Vaikutus: Voiko aika, tulos, sijoitus, pisteet tai hylkäys olla väärä? | Mahdollinen | Kaksoispoiston jälkeen kaksi lisättyä kilpailijaa saa saman tietueen, jolloin tiedot voivat sekoittua. Ei ajettu. |
| Vaikutus: Voiko maali- tai väliaikojen kirjaus pysähtyä tai katketa? | Mahdollinen | Vapaiden tietueiden luettelo voi sotkeutua ja HK voi kaatua poiston yhteydessä; riskiarvion mukaan häiritsee. Ei ajettu. |
| Vaikutus: Voiko leimaus- tai kortintiedon luku jäädä tekemättä tai mennä väärin? | Ei tiedossa | Aineisto ei käsittele leimausten vastaanottoa tämän vian osalta. |
| Vaikutus: Voiko tallennettu tai vastaanotettu tieto kadota tai vioittua? | Mahdollinen | Vapaiden paikkojen luettelo voi hävitä tai otsaketietue joutua luetteloon, jolloin lisätty kilpailija voi kadota. Ei ajettu. |
| Vaikutus: Voiko ohjelma kaatua tai sulkeutua odottamatta? | Mahdollinen | HK kirjoittaa taulukon alkua edeltävään muistiin, kun tietuenumero on -1; kaatuminen on päätelty koodista, ei ajettu. |
| Vaikutus: Voiko ohjelma tai yksittäinen toiminto jäädä jumiin? | Ei tiedossa | Aineisto ei käsittele jumiutumista tämän vian osalta. |
| Vaikutus: Voiko jokin mennä väärin ilman ilmoitusta tai näkyvää oiretta? | Mahdollinen | Lisäyksen peruttua tai toisen poiston jälkeen aineisto ei kirjaa ilmoitusta; lisätty kilpailija voi kadota tai tietue jakautua kahdelle ilman näkyvää oiretta. Ei ajettu. |
| Vaikutus: Voiko vika vaikuttaa tiedonsiirtoon tai muihin työasemiin? | Ei tiedossa | Poisto lähetetään koodin mukaan muille työasemille, mutta aineisto ei selvitä virheellisen poiston vaikutusta niissä. |
| Vaikutus: Voiko vika vaikuttaa lukijoihin, kelloihin, näyttötauluihin tai tulostimiin? | Ei sovellu | Vika ei liity lukijoihin, kelloihin, näyttötauluihin tai tulostimiin. |
| Vaikutus: Voiko vika vaikuttaa tulosteisiin, HTML- tai XML-tiedostoihin, tulospalveluun tai SQL-peiliin? | Ei tiedossa | Aineisto ei käsittele vaikutusta tulosteisiin tai julkaisuun. |
| Vaikutus: Voiko ulkopuolinen tai verkossa oleva laite käyttää vikaa hyväkseen? | Ei vaikutusta (perusteltu) | Riskiarvion mukaan tietoturvavaikutusta ei ole; vika laukeaa käyttäjän toimenpiteestä lomakkeessa, ei verkosta. |
| Ilmeneminen: Voiko vika ilmetä tavallisessa kilpailukäytössä ilman erityisasetusta tai virhettä? | Mahdollinen | Laukeaa lomakkeen valikkotoiminnosta ilman erityisasetusta, mutta vaatii käyttäjän virheellisen toimenpiteen (poisto ilman valintaa tai lisäyksen perumisen jälkeen). Ei ajettu. |
| Ilmeneminen: Voiko vika ilmetä poikkeustilanteessa (virhe, katkos, virheellinen syöte, harvinainen asetus)? | Mahdollinen | Laukaisijana on käyttäjän virheellinen toimenpide lomakkeessa, ei ympäristövirhe. Ei ajettu. |
| Ilmeneminen: Voiko käyttäjä huomata vian sen ilmetessä? | Ei tiedossa | Aineisto ei kuvaa näkyvää oiretta; vika voi näkyä vasta myöhemmin. |
| Hallinta: Onko aineistossa kirjattu keino välttää vika tai pienentää sen riskiä? | Ei tiedossa | Aineistossa ei ole kirjattua keinoa. |
| Hallinta: Onko aineistossa kirjattu keino palautua vian seurauksista kilpailun aikana? | Mahdollinen | Riskiarvio kirjaa palautettavuudeksi käsin; keinoa ei ole kuvattu eikä kokeiltu. |

Arvot: **Vahvistettu** = seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla; Mahdollinen = seuraa koodista, mutta seurausta ei ole osoitettu; Ei vaikutusta (perusteltu) = aineisto osoittaa, ettei seurausta synny; Ei tiedossa = aineisto ei ratkaise; _Ei arvioitu_ = riviä ei ole vielä arvioitu. Tiedon puute ei tarkoita, ettei vaikutusta ole. Lähteet: `doc/arkkitehtuuri/rekisteri/vaikutukset.csv`.

### 7. Mitä kilpailun aikana voi tapahtua

Kaikki alla oleva on mahdollista, ei vahvistettua (koodista luettu, ei ajettu; ks. luku 11).

- Operaattori avaa kilpailijatietolomakkeen, ei valitse ketään ja painaa "Poista kilpailija" ja
  Kyllä. HK:ssa tämä voi kirjoittaa muistiin väärään paikkaan ja hävittää vapaiden paikkojen
  luettelon. Seurauksena voi olla ohjelman kaatuminen kesken ajanoton tai lisättävien
  kilpailijoiden sijoittuminen väärin. Riskiarvio kirjaa ajanoton jatkuvuudelle seuraukseksi
  "häiritsee", ei pysäytä.
- Operaattori aloittaa uuden kilpailijan lisäyksen, peruu sen ja poistaa. Seuraava lisätty
  kilpailija katoaa.
- Kilpailutilassa operaattori poistaa kilpailijan ja painaa vielä kerran "Poista", koska
  selaus ei siirtynyt pois poistetusta. Kaksi seuraavaa lisättyä kilpailijaa jakaa tietueen, ja
  toisen tiedot ylikirjoittavat toisen. Tulokset voivat tällöin päätyä väärälle kilpailijalle (päätelty tietueen jakamisesta, ei osoitettu).
  Riskiarvio kirjaa tulosten oikeellisuudelle seuraukseksi "suora".

Mitään seurausta ei ole nähty ajossa. Sitä, mitä käyttäjä näkee juuri kaatumisen tai
tietueiden sekoittumisen hetkellä, aineisto ei kuvaa.

### 8. Palautuminen ja uudelleenkäynnistys

- Riskiarvio kirjaa palautettavuudeksi "käsin". Aineisto ei kuvaa, miten palautus tehdään.
- Auttaako uudelleenkäynnistys, aineisto ei kerro. Se ei myöskään kerro, onko vioittunut
  vapaiden paikkojen luettelo ehditty kirjoittaa tiedostoon.
- Muut työasemat: kun tiedonsiirto on käytössä, poisto lähetetään koodin mukaan muille
  työasemille (sekä viestin että HK:n poistorutiini lähettävät poiston). Aineisto ei selvitä, mitä
  virheellinen poisto niissä aiheuttaa.

### 9. Mitä käyttäjä voi tehdä riskin pienentämiseksi

Aineistossa ei ole kirjattua keinoa. Muistiturvallisuuden väliaikaiset suojaussuositukset
(`muistiturvallisuus.md` luku 15) eivät käsittele tätä vikaa.

### 10. Tekninen juurisyy

Lomakkeen käsittelijä `Poistakilpailija1Click` (`cbHk/UnitKilpailijatiedot.cpp` ja
`ViestiWin/UnitJoukkuetiedot.cpp`) kutsuu `deltall`ia lomakkeen tietuenumerolla `dKilp`
tarkistamatta, että numero on kelvollinen ja että lomake on tilassa, jossa kilpailija on
valittuna. Lomake luodaan arvolla -1, ja lisäyksen peruminen jättää arvon 0.

HK:ssa `kilptietue::PUTREC` tarkistaa vain ylärajan, joten negatiivinen indeksi kirjoittaa
`kilparr`-taulukon alkua edeltävään muistiin. `deleterec` (`dbboxm/dbbox1.cpp`) kirjaa poistetun
tietueen vapaiden luetteloon ilman rajatarkistusta: arvo -1 asettaa vapaiden luettelon alun
arvoon "ei mitään" ja luettelo menetetään, arvo 0 sijoittaa otsaketietueen luetteloon ja toinen
poisto samalle tietueelle linkittää sen itseensä. Viestin vastaava poistorutiini on `deltall`
(`Juk/vdat.cpp`).

Juurisyy JS-1 (ulkoista syötettä ei validoida rajalla; tässä lomakkeen arvoa käytetään indeksinä
tarkistamatta). Pahentava tekijä JS-6 (muistitila rakennetaan vain käynnistyksessä). Viestissä
lisäksi T-035 (viestin tietueella ei ole konstruktoria).

### 11. Mitä on todennettu

(a) Ohjelmointivirhe: luettu koodista ja riippumattomasti tarkastettu lähdekoodista (erä C-7).
Tarkastus korjasi tiedoston nimen (`dbbox1.cpp`, ei `Dbbox.cpp`) ja nosti vakavuuden
keskitasosta korkeaksi, koska -1 hävittää koko vapaiden luettelon. Ajoa tai testikehystä ei ole.

(b) Käytännön seuraus: mitään ei ole osoitettu ajamalla. Ajonaikainen vahvistus on kirjattu
jatkotyöjonoon (J7-4 `gui-liiketoimintalogiikka.md`:ssä, vaatii erillisen luvan).

### 12. Mitä ei vielä tiedetä

- Kaatuuko HK:n ohjelma tilanteessa 1: aineisto sanoo vain, että kaatuminen on päätelty.
- Saako kolmas lisäys otsaketietueen: päätelty, ei osoitettu.
- Mitä viestin alustamaton paikallinen tietue aiheuttaa ajossa (T-035; liittyvä Q76 koskee
  viestin joukkuetietuetta).
- Vaikuttavatko seuraukset muihin työasemiin.
- Onko ViestiLuenta, HkMaali tai ViestiMaali altis tälle vialle: ei tarkistettu.
- OP-9 kohta 8: saako tuloksen saaneen kilpailijan poistaa (omistajan päätös, avoin).

### 13. Miten vika on tarkoitus korjata

`korjaussuunnitelma.md`, kokonaisuus KK-C13-14 (aalto 2), PR 1: poisto sallitaan vain valitulle ja
kelvolliselle tietueelle. Suunnitelma rajaa korjauksen siten, että H-109:ssä hylätään vain
virheellinen valinta eli tilanne, jossa kilpailijaa ei ole valittu. Hyväksymiskriteeri: dokumentoitu
GUI-testiaskel toistaa vian ennen korjausta ja läpäisee sen korjauksen jälkeen molemmissa
ohjelmissa.

Päätöstä odottaa OP-9 kohta 8: saako tuloksen saaneen kilpailijan poistaa ja mitä poiston pitää
estää. Suunnitelman mukaan oletuskorjaukset eivät odota päätöstä. Samassa poistopolussa ovat H-139
ja H-140 (KK-C13-16).

### 14. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | H-109 |
| Rekisterin kuvaus | "Poista kilpailija" (HK ja viesti) ilman `dKilp`- ja tilatarkistusta: `dKilp = -1` (lomake avattu, ei valintaa) → HK `PUTREC(-1)` kirjoittaa `kilparr[-1]`:een ja `deleterec(-1)` hävittää vapaiden tietueiden listan; `dKilp = 0` (Lisää → Peruuta) → tiedoston otsake vapaiden listaan, seuraava lisäys katoaa; kilpailutilassa oletusselaus ei siirry poistetusta, joten toinen poisto linkittää tietueen itseensä ja kaksi seuraavaa lisäystä saavat saman tietueen / delete without guard: heap underflow, lost free list, double delete makes two adds share a record |
| Erä | C-7 |
| Tila | avoin |
| Vakavuus (rekisteri) | korkea / high |
| Varmuus (rekisteri) | [todennettu] koodi; [päätelty] kaatuminen ja kolmas lisäys / crash and third add |
| Todennus (rekisteri) | riippumaton lähdetarkastus / independent source review (C-7) |
| Prioriteetti | P2 |
| Riskiarvio | vakavuus korkea; saavutettavuus operaattori; todennus riippumaton; tulokset suora; ajanotto hairitsee; palautettavuus kasin; tietoturva ei |
| Riskiarvion peruste | Unchanged: Delete competitor without a selection corrupts the free list and can make two adds share a record. HK and relay. |
| Lähde (raportti) | `../gui-liiketoimintalogiikka.md` |
| Näytön laji | staattinen, lähdekoodi / static, source |
| Juurisyy | JS-1 Ulkoista syötettä ei validoida rajalla; pahentavat: JS-6 Muistitila rakennetaan vain käynnistyksessä |
| Ohjelmat | HkKisaWin ViestiWin (peruste: tarkistettu) |
| Korjauskokonaisuus | KK-C13-14 GUI: dataa korruptoivat toiminnot; vaihe 1, aalto 2; haara `korjaus/kk-c13-14-gui-datakorruptio` |
| Omistajan päätökset, joissa havainto mainitaan | OP-9 Ominaisuuksien tarkoituksellisuus (avoin) |
| Kokonaisuuden KK-C13-14 omistajan päätökset | OP-14 Kentän käyttötavat (tietopyyntö) (avoin) |
| Kysymykset | - |
| Liittyvät havainnot | - |
| Lähdeviitteet (revisio f7fe553, polut `TPsource/V52/`-hakemistosta) | `cbHk/UnitKilpailijatiedot.cpp:1746-1752`, `:1518`, `:1157-1176`; `ViestiWin/UnitJoukkuetiedot.cpp:1365-1371`; `Hk/HkDat.cpp:597-621`, `:741-766`; `dbboxm/dbbox1.cpp:227-270`; `Juk/vdat.cpp:907-939` |

Rekisteririvien tiiviste nyt: `fec35c48e97d`.


---

<a id="h-129"></a>

## H-129 – Vaiheen vaihto suurella kilpailulla voi vioittaa HK-ohjelman muistia

> **H-129** · tila: **avoin** · prioriteetti: **P2** · tekninen vakavuus: korkea
> Ohjelmointivirheen todennus: toistettu eristetyllä kokeella tai testikehyksessä. Riskiarvion saavutettavuus: tietyllä asetuksella tai kokoonpanolla (tarkemmin luvussa 4).
> Ohjelmat: HkKisaWin, HkMaali (tarkistettu kortin kirjoituksen yhteydessä).
> Korjauskokonaisuus: KK-C13-3 Vaiheen vaihdon ja käynnistyksen tila (HK) (aalto 1).

### 1. Lyhyesti

Kun HK-ohjelmassa vaihdetaan monivaiheisen kilpailun vaihetta käynnissä olevassa ohjelmassa,
ohjelma rakentaa kilpailunumeroiden hakemiston uudelleen mutta unohtaa nollata sen rivilaskurin.
Uudet rivit lisätään vanhojen perään, ja suurella kilpailulla ne ylittävät hakemiston varatun
tilan ja kirjoittuvat viereisen muistin päälle.

- **Vahvistettu (eristetty koeohjelma):** vaiheen vaihto jättää laskurin vanhaan arvoonsa, ja
  seuraava lisäys menee sen perään.
- **Mahdollinen (laskennallinen, ylivuotoa ei ajettu):** muistin ylikirjoitus ensimmäisessä
  vaiheen vaihdossa noin 1 500 kilpailijasta alkaen. Seuraus on ohjelman kaatuminen tai
  hiljainen muistivaurio.

### 2. Mitä ohjelma yrittää tehdä

Monivaiheisessa kilpailussa (esimerkiksi usean päivän kilpailussa) käyttäjä siirtää ohjelman
käsittelemään seuraavaa vaihetta ilman, että ohjelmaa käynnistetään uudelleen (sanasto:
[vaiheen vaihto](../sanasto.md)). Ohjelman pitäisi rakentaa muistissa olevat
hakemistot uuden vaiheen tiedoista siten, että ne vastaavat tuoretta käynnistystä.

### 3. Mikä menee vikaan

Kilpailunumerohakemisto on ohjelman muistissa oleva taulukko, josta ohjelma löytää kilpailijan
tietueen numerolla. Hakemistolla on kiinteä tila ([puskuri](../sanasto.md)), joka on mitoitettu
kilpailijamäärän mukaan lisättynä varauksella. Taulukolla on lisäksi laskuri, joka kertoo,
montako riviä siinä on.

Vaiheen vaihdossa ohjelma tyhjentää taulukon sisällön, mutta ei nollaa laskuria. Kun taulukko
täytetään uudelleen, uudet rivit lisätään laskurin osoittaman paikan perään, siis vanhojen,
nyt tyhjien rivien jälkeen. Kapasiteetin tarkistusta ei ole. Kun vanhoja ja uusia rivejä on
yhteensä enemmän kuin tilaa on, ylimenevät rivit kirjoitetaan taulukon ulkopuolelle
([puskuriylitys](../sanasto.md)). Vertaus: kirjanpitäjä tyhjentää vihkon sivut mutta pitää
rivinumeron entisellään ja aloittaa uuden kirjauksen vihkon puolivälistä, jolloin loppu ei
mahdu vihkoon.

### 4. Missä tilanteessa vika voi ilmetä

- **Ohjelmat:** HK, eli HkKisaWin ja HkMaali. Vaiheen vaihtoa ei ole viestiohjelmissa
  (raportin mukaan; vaiheen vaihdon funktiota kutsutaan vain HK:n lähteistä).
- **Toimenpide:** vaiheen vaihto käynnissä olevassa ohjelmassa. Se voidaan laukaista GUI:n
  vaiheenvaihtotoiminnolla, konsoliohjelman valikosta tai toisen työaseman vaihtopyynnöllä.
  Vaihtopyyntö käynnistää vaihdon vastaanottavassa ohjelmassa ilman vahvistuskysymystä. Jos
  toinen työasema toistaa saman vaiheen pyynnön yli viiden sekunnin välein, vaihto ajetaan
  uudelleen, ja rivilaskuri kasvaa joka kerta. Raja voi siis täyttyä toistolla myös pienemmässä
  kilpailussa [päätelty koodista]; ajamalla tätä ei ole osoitettu. Sanoman rakennetta ei
  kuvata tässä.
- **Kilpailun koko:** raja on laskennallinen ja riippuu hakemiston mitoituksesta. Raportin
  mukaan oletusasetuksilla ylivuoto alkaa ensimmäisessä vaiheen vaihdossa noin 1 500
  kilpailijasta ja kolmivaiheisen kilpailun toisessa vaihdossa noin 1 000 kilpailijasta.
  Mitoitus riippuu asetuksesta `n_lisaa` (oletus 1000), joten raja siirtyy, jos asetus on
  muu (Q78).
- **Rajaa laskevat tekijät:** jokainen kilpailijan poisto ja numeronvaihto lisää laskuriin
  tyhjiä rivejä (H-128), joten raja tulee vastaan pienemmällä kilpailijamäärällä.
- **Normaalikäyttö vai poikkeus:** vaihe voidaan vaihtaa tavallisella toiminnolla, ilman
  virhettä. Riskiarvio pitää saavutettavuutena asetusta, mikä ei sovi yhteen vaihtopyynnön
  kanssa; ristiriita on kirjattu avoimeksi kysymykseksi (luku 12). Aineisto ei kerro, onko vaiheen vaihto käynnissä olevassa ohjelmassa käytössä
  oikeissa asennuksissa vai käynnistetäänkö ohjelma uudelleen vaiheiden välillä (Q78, OP-6).
  Aineisto ei kerro myöskään, kuinka suuria kilpailuja asennuksissa ajetaan.

### 5. Mitä käyttäjä voi huomata

Aineisto ei kuvaa näkyvää oiretta. Raportin päätelmän mukaan seuraus voi olla ohjelman
kaatuminen vaiheen vaihdon aikana tai muun muistin hiljainen vioittuminen. Hiljainen
vioittuminen voi siis jäädä huomaamatta, mutta sen seurauksia ei ole selvitetty.

### 6. Vaikutukset kilpailuun

| Kysymys | Arvio | Peruste |
|---|---|---|
| Vaikutus: Voiko aika, tulos, sijoitus, pisteet tai hylkäys olla väärä? | Mahdollinen | Riskiarvion mukaan vaikutus tuloksiin on epäsuora: muistivaurio tai kaatuminen vaiheen vaihdossa; ylivuotoa ei ajettu. |
| Vaikutus: Voiko maali- tai väliaikojen kirjaus pysähtyä tai katketa? | Mahdollinen | Kaatuminen vaiheen vaihdossa pysäyttäisi ohjelman; ylivuotoa ei ajettu, eikä aineisto kerro vaiheen vaihdon ajoitusta ajanottoon nähden. |
| Vaikutus: Voiko leimaus- tai kortintiedon luku jäädä tekemättä tai mennä väärin? | Ei tiedossa | Aineisto ei käsittele vaikutusta leimausten lukuun. |
| Vaikutus: Voiko tallennettu tai vastaanotettu tieto kadota tai vioittua? | Ei tiedossa | Raportin mukaan väärää johdettua tietoa ei kirjoiteta levylle, mutta ylivuodon vaikutusta muistissa oleviin tietueisiin ei ole selvitetty. |
| Vaikutus: Voiko ohjelma kaatua tai sulkeutua odottamatta? | Mahdollinen | Kekoylivuoto seuraa koodista ja laskennasta (noin 1 500 kilpailijasta), mutta ylivuotoa ei ajettu eikä vahvistettu toimitetussa ohjelmassa. |
| Vaikutus: Voiko ohjelma tai yksittäinen toiminto jäädä jumiin? | Ei tiedossa | Aineisto ei käsittele jumiutumista tässä havainnossa. |
| Vaikutus: Voiko jokin mennä väärin ilman ilmoitusta tai näkyvää oiretta? | Mahdollinen | Raportin päätelmän mukaan seuraus voi olla muun muistin hiljainen vioittuminen; ei osoitettu. |
| Vaikutus: Voiko vika vaikuttaa tiedonsiirtoon tai muihin työasemiin? | Mahdollinen | Toisen työaseman vaiheen vaihtopyyntö käynnistää vaihdon vastaanottajassa ilman vahvistusta; vaikutus muihin työasemiin päätelty koodista, ei ajettu. |
| Vaikutus: Voiko vika vaikuttaa lukijoihin, kelloihin, näyttötauluihin tai tulostimiin? | Ei tiedossa | Aineisto ei käsittele vaikutusta lukijoihin, kelloihin, tauluihin tai tulostimiin. |
| Vaikutus: Voiko vika vaikuttaa tulosteisiin, HTML- tai XML-tiedostoihin, tulospalveluun tai SQL-peiliin? | Ei tiedossa | Aineisto ei käsittele vaikutusta tulosteisiin, HTML- tai XML-tiedostoihin tai tulospalveluun. |
| Vaikutus: Voiko ulkopuolinen tai verkossa oleva laite käyttää vikaa hyväkseen? | Ei tiedossa | Riskiarvio sanoo tietoturvasta ei, mutta vaiheen vaihdon voi laukaista verkkopyyntö; aineisto ei arvioi tämän merkitystä. |
| Ilmeneminen: Voiko vika ilmetä tavallisessa kilpailukäytössä ilman erityisasetusta tai virhettä? | Ei tiedossa | Vaiheen vaihto on tavallinen toiminto, mutta riskiarvio pitää saavutettavuutena asetusta; ristiriita avoin (tietopohjan avoin kysymys). |
| Ilmeneminen: Voiko vika ilmetä poikkeustilanteessa (virhe, katkos, virheellinen syöte, harvinainen asetus)? | Ei vaikutusta (perusteltu) | Laukeaa ilman poikkeustilannetta: tavallinen vaiheen vaihto riittää. |
| Ilmeneminen: Voiko käyttäjä huomata vian sen ilmetessä? | Mahdollinen | Kaatuminen olisi näkyvä, mutta hiljaista muistivauriota ei välttämättä huomaa; aineisto ei kerro oireista. |
| Hallinta: Onko aineistossa kirjattu keino välttää vika tai pienentää sen riskiä? | Ei tiedossa | Aineistossa ei ole kirjattua keinoa; uudelleenkäynnistys ohjeeksi on vain vaihtoehto päätöksessä OP-6. |
| Hallinta: Onko aineistossa kirjattu keino palautua vian seurauksista kilpailun aikana? | Mahdollinen | Riskiarvion palautettavuus on uudelleenkäynnistys: hakemisto rakennetaan käynnistyksessä uudelleen; ei kokeiltu. |

Arvot: **Vahvistettu** = seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla; Mahdollinen = seuraa koodista, mutta seurausta ei ole osoitettu; Ei vaikutusta (perusteltu) = aineisto osoittaa, ettei seurausta synny; Ei tiedossa = aineisto ei ratkaise; _Ei arvioitu_ = riviä ei ole vielä arvioitu. Tiedon puute ei tarkoita, ettei vaikutusta ole. Lähteet: `doc/arkkitehtuuri/rekisteri/vaikutukset.csv`.

### 7. Mitä kilpailun aikana voi tapahtua

Monivaiheisessa HK-kilpailussa, jossa vaihe vaihdetaan käynnissä olevassa ohjelmassa:

Vahvistetun ja mahdollisen erottelu on luvussa 11. Tilanne kilpailun aikana:

- Vaiheen vaihdon jälkeen hakemiston rivilaskuri on edelleen vanha, ja seuraava lisäys menee
  sen perään.
- Kun rivien määrä ylittää hakemiston tilan, ohjelma kirjoittaa varatun muistin yli. Ohjelma
  voi kaatua tai muu muisti voi vioittua hiljaa. Aineisto ei kerro, tapahtuuko vaiheen vaihto
  käytännössä kilpailun ajanoton aikana vai sen ulkopuolella.
- Riskiarvion mukaan ajanotto pysähtyy ohjelman kaatuessa; tulokset voivat muuttua
  epäsuorasti.
- Toisen työaseman vaihtopyyntö käynnistää vaihdon vastaanottajassa ilman vahvistusta, ja
  toistetut pyynnöt kasvattavat laskuria (luku 4). Vaikutus muihin työasemiin on mahdollinen
  [päätelty koodista]; sitä ei ole ajettu.
- Jos kilpailu on pienempi kuin luvussa 4 kuvattu raja, hakemisto toimii, mutta laskuri kasvaa
  jokaisessa vaiheen vaihdossa (osaraportin mukaan).

### 8. Palautuminen ja uudelleenkäynnistys

Rekisterin ja riskiarvion palautettavuus on "uudelleenkäynnistys". Raportin mukaan uudelleenkäynnistys
rakentaa hakemiston uudelleen, ja raportti nimeää H-129:n muistivaurion poikkeukseksi itsestään
korjautuvista vioista (`globaali-tila.md` luvut 5 ja 11): jo tapahtunut muistivaurio ei korjaudu
ennen uudelleenkäynnistystä. Aineisto ei kerro, miten ohjelma käyttäytyy kaatumisen jälkeen.
Toisen työaseman vaihtopyyntö voi käynnistää vaihdon myös vastaanottajassa (luku 4), joten vika
voi ilmetä muussakin kuin vaihdon pyytäneessä työasemassa; sitä ei ole ajettu. Hakemisto ja sen laskuri ovat vain ohjelman muistissa: raportin mukaan väärää
johdettua tietoa ei kirjoiteta levylle.

### 9. Mitä käyttäjä voi tehdä riskin pienentämiseksi

Aineistossa ei ole kirjattua keinoa. Omistajan päätöksessä OP-6 on vaihtoehtona, että
vaiheen vaihdon sijaan ohjeistettaisiin käynnistämään ohjelma uudelleen, mutta päätös on avoin
eikä sen tehoa ole selvitetty. Väliaikaiset suojaussuositukset (`muistiturvallisuus.md` luku 15)
eivät koske tätä havaintoa.

### 10. Tekninen juurisyy

Juurisyy on JS-6 (muistitila rakennetaan vain käynnistyksessä); pahentava tekijä JS-3
(kapasiteetti ei seuraa ajonaikaista kokoa).

- `vaihdavaihe` kutsuu `teejarj(true)`-funktiota (`Hk/HkIx.cpp`). Se tyhjentää
  `idindex`-puskurin `memset`illä mutta ei nollaa `idindex->nrec`iä. `luo_posarr` nollaa sen.
- Sen jälkeen `setpos`-silmukka lisää rivit `nrec`:n osoittaman kohdan perään, ja `addix`
  (`tputilv2/dbindex.cpp`) kirjoittaa ilman kapasiteettitarkistusta.
- Puskurin koko on `maxrec + 2` riviä. `maxrec` lasketaan vain käynnistyshaarassa:
  kilpailutilassa (`ToimintaTila == 2`) `KILP.DAT`:n koko plus `n_lisaa`, muussa tilassa
  `maxkilpno`; vähintään 3000, enintään `MAXKILPNO` (`Hk/HkIx.cpp`).
- Vaihtopyyntö: `Hk/HkCom32.cpp` kutsuu `pyyda_vaihdavaihe`a ilman vahvistusta, ja sama
  vaihe ajetaan uudelleen yli 5 s välein; GUI:n käsittelijä (`cbHk/WinHk.cpp`) tarkistaa vain,
  ettei lähettämättömiä sanomia ole.
- Ylivuoto syntyy, kun `nrec` ennen vaihtoa ja kilpailijoiden määrä ylittävät yhdessä
  `maxrec + 2`. Rajat on tarkistettu riippumattomassa lähdetarkastuksessa (C-8).
- Sama rivi kuin H-102:ssa (puuttuva lukko); H-102 on eri vika.

### 11. Mitä on todennettu

**(a) Ohjelmointivirhe.** Mekanismi on toistettu eristetyllä koeohjelmalla (C8ProbeHk,
`todisteet/c8/koeohjelmat/`), joka kutsuu tuotannon funktioita `vaihdavaihe`, `setpos` ja
`luo_posarr` sellaisenaan; vain globaali tila valmisteltiin (tyhjä `KILP.DAT`). Kaksi ajoa
antoi saman tuloksen. Koe lisäsi viisi hakemistoriviä ja ajoi `vaihdavaihe`-funktion tyhjällä
`KILP.DAT`:lla: laskuri jäi arvoon 5 (oikea arvo 0), ja seuraava lisäys meni kohtaan 6.
Kontrollikokeessa `luo_posarr` nollasi laskurin. Hakemiston kapasiteetti oli 1002 riviä.
Itse kekoylivuotoa ei ajettu. Se on laskennallinen seuraus.

**(b) Käytännön seuraus.** Ei ole osoitettu. Toimitettua ohjelmaa (`HkKisaWin.exe`,
`HkMaali.exe`) ei ole ajettu vaiheen vaihdossa.

### 12. Mitä ei vielä tiedetä

- Q78 (avoin): mikä `n_lisaa` oikeissa asennuksissa on ja vaihdetaanko vaihetta käynnissä
  olevassa ohjelmassa vai käynnistetäänkö se uudelleen.
- Kekoylivuodon toisto (noin 1 500 tietuetta, kekotarkistin kuten PageHeap tai ASan) on
  seurannan jatkotehtävä.
- Seuraus ylivuodon jälkeen (kaatuminen vai hiljainen vaurio) on päätelty, ei osoitettu.
- Ovatko kilpailut asennuksissa niin suuria, että raja ylittyy: aineisto ei kerro.
- Riskiarvion arvot tietoturva "ei" ja saavutettavuus "asetus" ovat ristiriidassa
  vaihtopyynnön ja tavallisen käytön kanssa: vaiheen vaihto on tavallinen toiminto, eikä
  vaihtopyynnön käsittelyä ole rajattu asetuksella. Riskiarvio arvioidaan uudelleen D-vaiheen
  riskiseulonnassa E-1; siihen asti vaikutusrivit tietoturva ja normaalikäyttö ovat ei tiedossa.

### 13. Miten vika on tarkoitus korjata

Havainto kuuluu kokonaisuuteen KK-C13-3 (Vaiheen vaihdon ja käynnistyksen tila, HK), sarjaan a
(numerohakemisto): `idindex`in rivimäärä ja paikka nollataan samassa kohdassa ja saman lukon
alla kuin puskuri (H-129 ja H-102), ja poistohaara tehdään saavutettavaksi (H-128). Sarja
tehdään ensimmäisenä, ja sen testeinä ovat C-8:n koeohjelma ja uusi testi itse
kekoylivuodolle (noin 1 500 tietuetta, ASan). Hyväksymiskriteeri: vaiheen vaihdon jälkeen
hakemiston rivimäärä on sama kuin tuoreessa käynnistyksessä samalla aineistolla. Sarja voidaan
palauttaa PR kerrallaan.

Päätöstä odottaa OP-6 (vaiheen vaihdon tuettu käyttötapa, avoin). Omistajan päätösasiakirjan
ehdotus on, että kekoylivuoto ja siihen liittyvät korjataan joka tapauksessa.

### 14. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | H-129 |
| Rekisterin kuvaus | Vaiheen vaihdon `teejarj(true)` nollaa `idindex`-puskurin (`memset`) mutta ei `idindex->nrec`iä; `setpos`-silmukka lisää uudet rivit vanhojen nollarivien perään ja `addix` kirjoittaa ilman kapasiteettitarkistusta `maxrec+2` rivin puskurin yli → kekoylivuoto ensimmäisessä vaihdossa noin 1 500 kilpailijasta alkaen (toisessa noin 1 000:sta; poistot ja numeronvaihdot H-128 laskevat rajaa). Eri vika kuin H-102 (sama rivi, puuttuva lukko) (N-C3) / stage change does not reset the bib index record count → heap overflow |
| Erä | C-8 |
| Tila | avoin |
| Vakavuus (rekisteri) | korkea / high |
| Varmuus (rekisteri) | [todennettu] koodi ja rajat / code and thresholds; [päätelty] kilpailukoot / event sizes |
| Todennus (rekisteri) | riippumaton lähdetarkastus (C-8, Opus), vahvistettu, rajat korjattu / confirmed, thresholds corrected; Q78; **mekanismi toistettu eristetyllä koeohjelmalla / mechanism reproduced with an isolated probe** (C-8 sulkeminen: tuotannon `vaihdavaihe` jättää `idindex->nrec` = 5 tyhjällä `KILP.DAT`:lla, seuraava `setpos` lisää perään (6); kontrolli `luo_posarr` nollaa; kaksi ajoa / production stage change leaves the count, next add appends; control resets; two runs); itse kekoylivuotoa ei ajettu (laskennallinen) eikä vahvistettu toimitetussa ohjelmassa / the heap overflow itself was not run (arithmetic) nor confirmed in a shipped program; korjausjonossa P2 (alustava) / fix queue P2 (provisional) |
| Prioriteetti | P2 |
| Riskiarvio | vakavuus korkea; saavutettavuus asetus; todennus koe_tai_testikehys; tulokset epasuora; ajanotto pysayttaa; palautettavuus uudelleenkaynnistys; tietoturva ei |
| Riskiarvion peruste | Confirmed P2 (was provisional): an HK stage change from about 1500 competitors (1000 on the second) overflows the bib index heap buffer; mechanism reproduced. Overflow itself arithmetic, not run; Q78 (n_lisaa, in-process stage changes); stage-change package also carries H-102, H-128, H-131..H-134. |
| Lähde (raportti) | `../globaali-tila.md` |
| Näytön laji | staattinen, lähdekoodi / static, source + testiajo / test run C8ProbeHk (C-8 sulkeminen / closing, `todisteet/c8/koeohjelmat/`) |
| Juurisyy | JS-6 Muistitila rakennetaan vain käynnistyksessä; pahentavat: JS-3 Kapasiteetti ei seuraa ajonaikaista kokoa |
| Ohjelmat | HkKisaWin HkMaali (peruste: tarkistettu) |
| Korjauskokonaisuus | KK-C13-3 Vaiheen vaihdon ja käynnistyksen tila (HK); vaihe 1, aalto 1; haara `korjaus/kk-c13-3-vaiheen-vaihto` |
| Omistajan päätökset, joissa havainto mainitaan | OP-6 Vaiheen vaihdon tuettu käyttötapa ja nimeäminen (avoin) |
| Kysymykset | Q78 (avoin) |
| Liittyvät havainnot | H-128 H-102 |
| Lähdeviitteet (revisio f7fe553, polut `TPsource/V52/`-hakemistosta) | `Hk/HkIx.cpp:1573-1577`, `:1600`, `:1610`, `:1757-1758`; `tputilv2/dbindex.cpp:51-71`; `Hk/HkGlobals.cpp:394`; kutsujat / callers `cbHk/WinHk.cpp:2126`, `:2209`, `Hk/HkCom32.cpp:847`, `HkConsole/HkCons0.cpp:871`, `:933` |

Rekisteririvien tiiviste nyt: `5cbe6df65d00`.


---

<a id="h-145"></a>

## H-145 – Jos emit-tiedoston kirjoitus epäonnistuu, luettu kortti kuitataan lukijalle mutta jää tallentumatta

> **H-145** · tila: **avoin** · prioriteetti: **P3** · tekninen vakavuus: keskitaso
> Ohjelmointivirheen todennus: toistettu eristetyllä kokeella tai testikehyksessä. Riskiarvion saavutettavuus: tavallisessa käytössä (tarkemmin luvussa 4).
> Ohjelmat: HkKisaWin, HkMaali, ViestiWin, ViestiMaali, ViestiLuenta (yläraja: ohjelmat, joihin viitatut lähdetiedostot käännetään).
> Korjauskokonaisuus: KK-C9-2 Tallennusvirheiden välitys, kuittaus ja palautus (aalto 3).

### 1. Lyhyesti

Kun ohjelma ei saa kirjoitettua juuri luettua emit-korttia emit-tiedostoon (esimerkiksi levy on
täynnä), käyttäjä saa virheilmoituksen, mutta ohjelma toimii muuten kuin kortti olisi tallennettu.
Kortti jää pois emit-tiedostosta, hakemistosta ja muilta työasemilta, eikä sitä liitetä
kilpailijaan. Saman kortin uusi luku ei tallenna korttia, vaan antaa pelkän vahvistuksen.

Vahvistettu (HK:n koodilla eristetyssä kokeessa, kirjoitusvirhe tuotettu tahallaan): kortti ei
tallennu, kirjoitusvirheen jälkeen ohjelman paluuarvo on sama kuin onnistuneessa luvussa, ja
oletusasetuksella kortti ei tallennu saman istunnon aikana edes sen jälkeen, kun toinen kortti on
luettu. Mahdollinen (koodista luettu, ei kokeiltu): Emit-kello saa onnistumiskuittauksen, joten
kilpailijaa ei pyydetä lukemaan korttia uudelleen, ja kilpailijan tulos tai leimantarkastus jää
puuttumaan, kunnes tieto syötetään käsin.

### 2. Mitä ohjelma yrittää tehdä

Kun kilpailija lukee emit-korttinsa maalissa, ohjelma tallentaa kortin leimaustiedot
emit-tiedostoon, lisää kortin hakemistoon, liittää sen kilpailijaan ja lähettää sen muille
työasemille. Lukijalaite saa kortin vastaanotosta kuittauksen. Kuittaus kertoo laitteelle, että
kortti on käsitelty.

### 3. Mikä menee vikaan

Ohjelma kirjaa kortin muistissa olevaan "viimeksi luetut" -välimuistiin ja merkitsee sen
luetuksi ennen kuin kirjoitus tiedostoon on onnistunut. Jos kirjoitus epäonnistuu, virheestä
annetaan ikkunailmoitus, mutta virhetietoa ei välitetä eteenpäin. Kortin käsittelyfunktio
palauttaa aina saman "onnistui"-arvon. Tiedoston pituutta ja hakemistoa ei päivitetä,
eikä korttia lähetetä muille työasemille.

Kun sama kortti luetaan uudelleen, ohjelma tunnistaa sen muistin perusteella jo luetuksi ja antaa
vain vahvistuksen (kortin numero). Kortti jää tallentamatta. Asetus `estaemittoisto` (oletuksena
1) hylkää saman kortin uuden luvun kaksoiskappaleena. Arvolla 0 kortti tallentuu, kun joku toinen
kortti on ensin luettu.

Kuittaus lähtee lukijalle vain Emit-kellolla. Muilla lukijatyypeillä sanoma kulutetaan joka
tapauksessa, joten kortti ei jää odottamaan uutta lähetystä.

### 4. Missä tilanteessa vika voi ilmetä

- Edellytys on emit-tiedoston kirjoitusvirhe, jonka syitä aineisto nimeää: levy täynnä,
  katkennut verkkolevy (jos emit-tiedosto on verkkopolussa) tai tiedosto lukittuna. EMIT-toiminnon
  pitää olla käytössä.
- Ohjelmat: koodi on HK:n (`HkIV.cpp`, `HkEmit.cpp`) ja viestin (`VIv.cpp`, `VEmit.cpp`) yhteistä
  emit-polkua. Koe tehtiin HK:n koodilla MSVC-käännöksellä. Viestin polku on luettu koodista.
- Kyse on poikkeustilanteesta (ympäristövirhe), ei erityisasetuksen tai käyttäjän virheen
  seurauksesta. Yleisyydestä aineisto ei kerro mitään. Riskiarvion mukaan vika laukeaa
  tavallisessa käytössä, vaikka laukaisu vaatii kirjoitusvirheen; ristiriita on kirjattu
  avoimeksi kysymykseksi (luku 12).

### 5. Mitä käyttäjä voi huomata

- Kirjoitusvirhe tuottaa ikkunailmoituksen (kokeessa ohjelma kutsui ilmoitusfunktiota; oikeaa
  ikkunaa ei kokeessa näytetty, eikä ole osoitettu, että ilmoitus näkyy jokaisesta
  epäonnistuneesta kirjoituksesta). Ilmoitus nimeää tiedoston, mutta ei kerro, että kortti on jäänyt tallentumatta.
- Ilmoitus tulee lukijasäikeestä (ks. sanasto: Säie), joka pitää emit-tietoa suojaavaa lukkoa
  (ks. sanasto: Lukko) ilmoituksen ajan. Lukijasäie odottaa ilmoitusikkunan sulkemista
  [päätelty]. Korttia ei liitetä kilpailijaan.
- Jos ilmoitus jää huomaamatta tai suljetaan, vika voi jäädä huomaamatta siihen asti, kun
  kilpailijan emit-tulosta tai leimantarkastusta tarvitaan. Aineisto ei kuvaa, mitä lukija näyttää
  tai tekee kuittauksen jälkeen (Q86, Q100).

### 6. Vaikutukset kilpailuun

| Kysymys | Arvio | Peruste |
|---|---|---|
| Vaikutus: Voiko aika, tulos, sijoitus, pisteet tai hylkäys olla väärä? | Mahdollinen | Kortti ei päädy kilpailijalle, joten emit-tulos tai leimantarkastus voi puuttua tai olla väärä kunnes tieto syötetään käsin; päätelty, ei ajettu. |
| Vaikutus: Voiko maali- tai väliaikojen kirjaus pysähtyä tai katketa? | Ei tiedossa | Riskiarvion mukaan ajanotto ei pysähdy, mutta lukijasäie voi odottaa ilmoitusikkunaa lukon alla (päätelty); aineisto ei ratkaise. |
| Vaikutus: Voiko leimaus- tai kortintiedon luku jäädä tekemättä tai mennä väärin? | **Vahvistettu** | Eristetyssä kokeessa epäonnistuneen kirjoituksen kortti ei tallennu, vaikka funktio palauttaa onnistumisarvon; saman kortin uusi luku antaa vain vahvistuksen. |
| Vaikutus: Voiko tallennettu tai vastaanotettu tieto kadota tai vioittua? | **Vahvistettu** | Kokeessa luettu kortti ei tallennu emit-tiedostoon; oletusasetuksella kortti ei tallennu saman istunnon aikana. |
| Vaikutus: Voiko ohjelma kaatua tai sulkeutua odottamatta? | Ei tiedossa | Aineisto ei kirjaa kaatumista tämän vian seurauksena. |
| Vaikutus: Voiko ohjelma tai yksittäinen toiminto jäädä jumiin? | Mahdollinen | Ilmoitus annetaan lukijasäikeessä emit-lukon alla, jolloin säie voi odottaa ilmoitusikkunan sulkemista; päätelty, ei ajettu. |
| Vaikutus: Voiko jokin mennä väärin ilman ilmoitusta tai näkyvää oiretta? | Mahdollinen | Ilmoitus annetaan, mutta se ei kerro kortin jääneen tallentamatta, ja uusi luku antaa vain vahvistuksen; päätelty kokeen tuloksista. |
| Vaikutus: Voiko vika vaikuttaa tiedonsiirtoon tai muihin työasemiin? | Mahdollinen | Tallentamatonta korttia ei lähetetä muille työasemille; luettu koodista, ei ajettu. |
| Vaikutus: Voiko vika vaikuttaa lukijoihin, kelloihin, näyttötauluihin tai tulostimiin? | Mahdollinen | Emit-kello saa kuittauksen, vaikka kortti ei tallentunut; laitteen reaktio on avoin (Q86, Q100). Luettu koodista. |
| Vaikutus: Voiko vika vaikuttaa tulosteisiin, HTML- tai XML-tiedostoihin, tulospalveluun tai SQL-peiliin? | Ei tiedossa | Aineisto ei käsittele vaikutusta tulosteisiin tai julkaisuun. |
| Vaikutus: Voiko ulkopuolinen tai verkossa oleva laite käyttää vikaa hyväkseen? | Ei vaikutusta (perusteltu) | Riskiarvion mukaan tietoturvavaikutusta ei ole; laukaisijana on ympäristön kirjoitusvirhe. |
| Ilmeneminen: Voiko vika ilmetä tavallisessa kilpailukäytössä ilman erityisasetusta tai virhettä? | Ei tiedossa | Vika vaatii emit-tiedoston kirjoitusvirheen, mutta riskiarvion saavutettavuus on normaalikaytto; ristiriita riskiarvion kanssa jää avoimeksi. |
| Ilmeneminen: Voiko vika ilmetä poikkeustilanteessa (virhe, katkos, virheellinen syöte, harvinainen asetus)? | **Vahvistettu** | Virhe tuotettiin eristetyssä kokeessa vain luku -kahvalla ja vika toistui. |
| Ilmeneminen: Voiko käyttäjä huomata vian sen ilmetessä? | Mahdollinen | Kokeessa ilmoitusfunktiota kutsuttiin, mutta ilmoitusikkunaa ei näytetty; näkyykö ilmoitus jokaisesta epäonnistuneesta kirjoituksesta, ei ole osoitettu. |
| Hallinta: Onko aineistossa kirjattu keino välttää vika tai pienentää sen riskiä? | Ei tiedossa | Aineistossa ei ole kirjattua keinoa vian välttämiseksi. |
| Hallinta: Onko aineistossa kirjattu keino palautua vian seurauksista kilpailun aikana? | Mahdollinen | Riskiarvio kirjaa palautettavuudeksi käsin (käsin syöttö tai uudelleenkäynnistys ja uusi luku); keinoja ei ole kokeiltu. Kokeessa osoitettu palautuminen vaati oletuksesta poikkeavan asetuksen. |

Arvot: **Vahvistettu** = seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla; Mahdollinen = seuraa koodista, mutta seurausta ei ole osoitettu; Ei vaikutusta (perusteltu) = aineisto osoittaa, ettei seurausta synny; Ei tiedossa = aineisto ei ratkaise; _Ei arvioitu_ = riviä ei ole vielä arvioitu. Tiedon puute ei tarkoita, ettei vaikutusta ole. Lähteet: `doc/arkkitehtuuri/rekisteri/vaikutukset.csv`.

### 7. Mitä kilpailun aikana voi tapahtua

Kilpailun aikana emit-tiedoston levy täyttyy tai verkkolevy katkeaa. Joukko kortteja luetaan
maalissa. Vahvistetun ja mahdollisen erottelu on luvussa 11. Tilanne:

- Epäonnistuneen kirjoituksen kortti ei tallennu emit-tiedostoon. Saman kortin uusi luku antaa
  vain vahvistuksen. Oletusasetuksella (`estaemittoisto` 1) kortti ei tallennu saman istunnon
  aikana, vaikka toinen kortti luettaisiin välissä. Asetuksella 0 kortti tallentuu, kun toinen
  kortti on luettu ja virhe on poistunut.
- Emit-kello saa kuittauksen, joten kilpailija ei saa merkkiä uudelleen luvun tarpeesta.
  Kortin tietoja ei saa muilta työasemilta. Kilpailijalta puuttuu emit-tulos tai
  leimantarkastus (mahdollinen, luettu koodista).
- Jos ilmoitusikkuna jää auki, lukijasäie odottaa, ja lukijan lähettämät tiedot odottavat
  portin puskurissa [päätelty].

### 8. Palautuminen ja uudelleenkäynnistys

- Riskiarvio kirjaa palautettavuudeksi "käsin". Kirjatut keinot: käsin syöttö (lomake
  `LisaTietue`) tai ohjelman uudelleenkäynnistys ja kortin uusi luku. Uudelleenkäynnistyksen jälkeen
  korttia ei ole tiedostossa, ja välimuisti on tyhjä, joten uusi luku tallentaa kortin
  (koodista päätelty; kokeessa ei ajettu).
- Oletusasetuksella tavallinen uusi luku ei palauta korttia saman istunnon aikana (vahvistettu
  kokeessa).
- Muut työasemat eivät saa korttia, koska sitä ei lähetetä. Vika ei muuten koske muita työasemia.

### 9. Mitä käyttäjä voi tehdä riskin pienentämiseksi

Aineistossa ei ole kirjattua keinoa vian välttämiseksi. Palautumiskeinot ovat luvussa 8. Kokeessa
asetus `estaemittoisto` = 0 salli kortin tallennuksen toisen kortin jälkeen. Aineisto ei suosittele
asetuksen muuttamista.

### 10. Tekninen juurisyy

`tall_emit` (HK: `HkIV.cpp`, viesti: `VIv.cpp`) tallentaa kortin `ed_em`-välimuistiin ennen kuin
`addem` on kirjoittanut sen, ja palauttaa aina 0. HK:n `addem` (`HkEmit.cpp`) palauttaa 0 myös,
kun `putem` epäonnistuu (arvo 1 tarkoittaa vain kaksoiskappaletta). Viestin `addem` palauttaa 1,
mutta kukaan ei käytä arvoa. Epäonnistumisessa `emithead`, hakemisto ja `laheta_em` jäävät
tekemättä. `on_emit` rekisteröi kortin jo ennen kirjoitusta. Emit-kellon lukijasäie
(`TpLaitteet.cpp`) kuittaa kortin, koska paluuarvo on 0.

Juurisyy JS-11 (virheet ikkunoina, ei paluuarvoina). Kokonaisuuden juurisyyt: T-040 ja T-041.

### 11. Mitä on todennettu

(a) Ohjelmointivirhe: toistettu eristetyllä kokeella (C-9 vaihe 2, `todisteet/c9/vaihe2/`).
HK:n tuotantokoodi ajettiin MSVC Release -käännöksellä, ja kirjoitusvirhe tuotettiin vaihtamalla
emit-tiedoston kahva vain luku -kahvaksi. Riippumaton tarkastus vahvisti kokeen reiluksi ja
tarkensi kaksi kohtaa (kuittaus vain Emit-kellolla, menetys istunnon ajaksi).

(b) Käytännön seuraus: kokeessa osoitettu, että kortti ei tallennu, emit-tiedosto ei kasva,
ilmoitus annetaan ja uusi luku antaa vain vahvistuksen. Kokeessa käytettiin MTR-lukijan
paketinkäsittelyä, jolloin kuittausta lukijalle ei ajettu. Kuittaus Emit-kellolle on luettu
koodista. Se, mitä lukijalaite kuittauksen jälkeen tekee, ei ole tiedossa.

### 12. Mitä ei vielä tiedetä

- Q86 (osittain avoin): pitääkö lukija kuittausta lopullisena ja palauttaako laitteen oma
  uudelleenlähetys kortin. Koodin puoli on selvitetty, laitteen puoli ei.
- Q100: Emit-kellon käytös ilman kuittausta.
- Onko ilmoitusikkuna, joka jää auki ilman valvontaa, realistinen tilanne. Sen mukaan vakavuus
  voisi nousta keskitasosta korkeaksi (päätelty, tarkastuksessa).
- Koe ajettiin HK:n koodilla testikehyksessä, ei valmiilla ohjelmalla. Viestin polkua ja
  bcc32-käännettyjä GUI-ohjelmia ei ole ajettu; ne on luettu koodista. ViestiLuenta: ei
  tarkistettu.
- Aineisto ei kerro, miten vika vaikuttaa julkaisuihin.
- Riskiarvion saavutettavuus on normaalikaytto, vaikka vika vaatii emit-tiedoston
  kirjoitusvirheen. Riskiarvion sarakkeessa ei ole arvoa ympäristövirheelle; vaikutusrivi
  normaalikäyttö on siksi ei tiedossa, ja riskiarvio tarkistetaan.

### 13. Miten vika on tarkoitus korjata

`korjaussuunnitelma.md`, kokonaisuus KK-C9-2 (aalto 3): kirjoitusvirhe palautetaan kutsujalle ja
kortti kuitataan lukijalle vasta onnistuneen tallennuksen jälkeen. Testit: C-9:n koeohjelmat,
virheinjektio ja laitetesti. Yhteensopivuus: tiedostomuodot ennallaan, kuittauksen ajoitus muuttuu.

Päätöstä odottaa OP-13 kohta 2: pitääkö lukija kuittausta lopullisena ja saako saman kortin
toisen luennan hyväksyä. Omistajan päätösten mukaan kuittausjärjestyksen muutosta ei tehdä
oletuskorjauksena ennen laitetestiä.

### 14. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | H-145 |
| Rekisterin kuvaus | Emit-tiedoston kirjoitusvirhe: `tall_emit` tallentaa kortin `ed_em`-välimuistiin ennen `addem`ia ja palauttaa aina 0, joten lukijalle lähtee kuittaus; HK:n `addem` palauttaa 0 myös epäonnistuneella `putem`illa (viestin 1 ohitetaan); `emithead`, hakemisto ja `laheta_em` jäävät tekemättä, eikä saman kortin uusi luku tallenna sitä (`samatleimat` → vahvistus). Käyttäjä näkee vain modaali-ilmoituksen lukijasäikeessä `emit`-lukon alla / emit write failure: card ACKed to the reader, cached as read, not stored, indexed or sent, not retried **C-9 vaihe 2:** toistettu virheinjektiolla; kuittaus seuraa vain Emit-kellolla (`Tp/TpLaitteet.cpp:911-917`), muilla lukijoilla sanoma kulutetaan joka tapauksessa; oletuksella `estaemittoisto = 1` kortti ei tallennu saman istunnon aikana edes toisen kortin jälkeen, uudelleenkäynnistys ja uusi luku tai käsin syöttö palauttaa / step 2: reproduced; ACK only on the Emit clock; lost for the session Korjauskokonaisuus / fix package **KK-C9-2** (tallennusvirheiden välitys, kuittaus ja palautus, juurisyyt / root causes **T-040**, **T-041**; kokonaiskuva / overview `../virheenkasittely.md` 12.7, 13.4). |
| Erä | C-9 |
| Tila | avoin |
| Vakavuus (rekisteri) | keskitaso / medium |
| Varmuus (rekisteri) | [todennettu] |
| Todennus (rekisteri) | toistettu testikehyksessä / reproduced in test harness (C-9 vaihe 2, `../todisteet/c9/vaihe2/`); riippumaton lähdetarkastus / independent source review; Q86 |
| Prioriteetti | P3 |
| Riskiarvio | vakavuus keskitaso; saavutettavuus normaalikaytto; todennus koe_tai_testikehys; tulokset suora; ajanotto ei; palautettavuus kasin; tietoturva ei |
| Riskiarvion peruste | Kept P3 (first in P3): an emit write failure ACKs the card to an Emit clock and drops it for the session; needs an environmental write failure and the operator gets a modal message. Reproduced by fault injection; ACK only on the Emit clock; recovery by restart and re-read or manual entry; KK-C9-2. |
| Lähde (raportti) | `../virheenkasittely.md` |
| Näytön laji | staattinen, lähdekoodi / static, source |
| Juurisyy | JS-11 Virheet ikkunoina, ei paluuarvoina |
| Ohjelmat | HkKisaWin HkMaali ViestiWin ViestiMaali ViestiLuenta (peruste: tiedosto) |
| Korjauskokonaisuus | KK-C9-2 Tallennusvirheiden välitys, kuittaus ja palautus; vaihe 2, aalto 3; haara `korjaus/kk-c9-2-tallennusvirheet` |
| Muut kokonaisuudet | KK-C10-2 |
| Omistajan päätökset, joissa havainto mainitaan | OP-13 Laitteet ja ulkoiset palvelut (avoin) |
| Kysymykset | Q86 (osittain) |
| Liittyvät havainnot | T-040 T-041 |
| Lähdeviitteet (revisio 7aaca40, polut `TPsource/V52/`-hakemistosta) | `../todisteet/c9/osaB-pysyva-data.md` N-B1; `Hk/HkIV.cpp:1145-1151`; `Hk/HkEmit.cpp:1657-1693`; `Juk/VIv.cpp:1099-1113`; `Tp/TpLaitteet.cpp:911-916`, `:1248-1254`; `../virheenkasittely.md` luku 12.2 |

Rekisteririvien tiiviste nyt: `909c0073ed0c`.


---

<a id="h-159"></a>

## H-159 – Sirit- tai Zebra-RFID-lukijan uudelleenyhteys lopettaa lukijan luennan, kunnes ohjelma käynnistetään uudelleen

> **H-159** · tila: **avoin** · prioriteetti: **P2** · tekninen vakavuus: korkea
> Ohjelmointivirheen todennus: luettu koodista ja riippumattomasti tarkastettu. Riskiarvion saavutettavuus: ajanotto- tai leimauslaitteen yhteydestä tai sen lähettämästä tiedosta (tarkemmin luvussa 4).
> Ohjelmat: HkKisaWin, HkMaali, ViestiWin, ViestiMaali (tarkistettu kortin kirjoituksen yhteydessä).
> Korjauskokonaisuus: KK-C10-2 Laiteyhteyksien hallinta (aalto 1).

### 1. Lyhyesti

Kun Sirit- tai Zebra-RFID-lukijan verkkoyhteys katkeaa ja ohjelma yrittää avata sen uudelleen,
ohjelma tulkitsee uudelleenyhteyden tuloksen väärinpäin. Seurauksena lukijaa lukeva säie päättyy,
eikä ohjelma enää lue lukijaa ennen kuin ohjelma käynnistetään uudelleen. Käyttäjä saa
ilmoituksen "Häiriö. Ajanoton/leimantarkastuksen jatko vaatii uudelleenkäynnistyksen".

Vahvistettu: ei mitään seurausta ajamalla. Vika on luettu koodista ja riippumaton tarkastaja on
vahvistanut koodiluennan. Mahdollinen (koodista, ei ajettu): RFID-leimaukset eivät tule ohjelmaan
uudelleenyhteyden jälkeen, ja ajanotto lukijan osalta pysähtyy kilpailun ajaksi tai kunnes ohjelma
on käynnistetty uudelleen.

### 2. Mitä ohjelma yrittää tehdä

Ohjelma lukee RFID-lukijalta (Sirit tai Zebra FX9600) leimauksia verkkoyhteyden kautta.
Lukemista varten ohjelmassa on yksi säie lukijaa kohti: säie on ohjelman sisällä rinnakkain toimiva
työ (ks. sanasto: Säie). Jos yhteys katkeaa, säie odottaa noin viisi sekuntia ja yrittää
avata yhteyden uudelleen. Sama uudelleenyhteys tehdään, kun käyttäjä pyytää sitä käsin ajanoton
lomakkeelta.

### 3. Mikä menee vikaan

Uudelleenyhteyden avaava funktio ilmoittaa onnistumisen yhdellä arvolla ja epäonnistumisen
toisella (tarkat arvot luvussa 10). Lukijasäikeen silmukka tulkitsee tuloksen
päinvastoin: kun yhteys avattiin onnistuneesti, se asettaa lukijan "ei käytössä" -tilaan, ja
silmukka päättyy. Sen jälkeen säie kirjoittaa ilmoituksen ja lopettaa.

- Sirit: uudelleenyhteyttä yritettäessä funktio palauttaa aina 0. Säie päättyy jokaisen
  uudelleenyhteysyrityksen jälkeen, onnistui yritys tai ei.
- Zebra: säie yrittää uudelleen viiden sekunnin välein, kun lukijaan ei saada yhteyttä, mutta
  päättyy ensimmäiseen onnistuneeseen uudelleenyhteyteen.

Ohjelmassa ei ole kirjattua tapaa käynnistää lukijasäiettä uudelleen kesken ajon. Käynnistysfunktio
palaa heti, jos lukija on jo käynnistetty ja ohjelma on kilpailutilassa, ja tilan nollaa vain
lukijan pysäytys, jota maalikellon sulkeminen kutsuu vain jos lukija on vielä käytössä (riippumaton
tarkastus, koodista).

### 4. Missä tilanteessa vika voi ilmetä

- Ohjelmat: HkMaali, ViestiMaali, HkKisaWin ja ViestiWin, kun lukijaksi on määritelty Sirit tai
  Zebra (asetukset `SIRITn=` / `ZEBRAn=`). Koodi on mukana vain, kun käännöksessä on `LAJUNEN`-
  valitsin, eikä sitä ole ViestiLuennassa.
- Laukaisu on verkkoyhteyden katkeaminen (lukijan virrankatko, kaapelin irrotus, lukijan
  uudelleenkäynnistys) tai käyttäjän pyytämä käsin tehtävä uudelleenyhteys. Käsin tehtävä
  uudelleenyhteys (Sirit) ei vaadi vikatilannetta.
- Rekisterin mukaan vika on alkuperäisessä, vuoden 2015 koodissa.
- Yleisyydestä aineisto ei kerro mitään.

### 5. Mitä käyttäjä voi huomata

- Ohjelma antaa koodin mukaan ilmoituksen "Häiriö. Ajanoton/leimantarkastuksen jatko vaatii
  uudelleenkäynnistyksen", kun säie päättyy. Ilmoitus jää antamatta, jos ohjelma on jo
  sulkeutumassa tai käyttäjä on pyytänyt katkaisua.
- Muuten ohjelma näyttää toimivan, mutta lukijan leimauksia ei tule. Aineisto ei kuvaa, näkyykö
  lukijan tila ikkunassa.
- Ajossa tätä ei ole nähty (Q96), joten oire on koodista päätelty.

### 6. Vaikutukset kilpailuun

| Kysymys | Arvio | Peruste |
|---|---|---|
| Vaikutus: Voiko aika, tulos, sijoitus, pisteet tai hylkäys olla väärä? | Mahdollinen | Riskiarvion mukaan epäsuora: RFID-leimaukset eivät tule ohjelmaan uudelleenyhteyden jälkeen ja tulokset voivat jäädä puutteellisiksi; ei ajettu. |
| Vaikutus: Voiko maali- tai väliaikojen kirjaus pysähtyä tai katketa? | Mahdollinen | Lukijasäie päättyy uudelleenyhteyden jälkeen eikä luenta jatku ennen ohjelman uudelleenkäynnistystä; riskiarvion mukaan pysäyttää. Ei ajettu. |
| Vaikutus: Voiko leimaus- tai kortintiedon luku jäädä tekemättä tai mennä väärin? | Mahdollinen | Sirit- tai Zebra-lukijan leimauksia ei lueta uudelleenyhteyden jälkeen; päätelty koodista. |
| Vaikutus: Voiko tallennettu tai vastaanotettu tieto kadota tai vioittua? | Ei tiedossa | Aineisto ei kerro säilyvätkö lukijalaitteessa olevat leimaukset siihen asti kun luenta jatkuu. |
| Vaikutus: Voiko ohjelma kaatua tai sulkeutua odottamatta? | Ei tiedossa | Säie päättyy ja ilmoitus annetaan; aineisto ei käsittele ohjelman sulkeutumista tästä syystä. |
| Vaikutus: Voiko ohjelma tai yksittäinen toiminto jäädä jumiin? | Ei vaikutusta (perusteltu) | Silmukka poistuu ja säie päättyy; se ei jää odottamaan, vaikka luenta lakkaa. |
| Vaikutus: Voiko jokin mennä väärin ilman ilmoitusta tai näkyvää oiretta? | Ei tiedossa | Koodin mukaan ilmoitus annetaan säikeen päättyessä, mutta ajossa ei todennettu (Q96) eikä aineisto kerro, näkyykö se ilman valvontaa. |
| Vaikutus: Voiko vika vaikuttaa tiedonsiirtoon tai muihin työasemiin? | Ei tiedossa | Aineisto ei käsittele vaikutusta tiedonsiirtoon tai muihin työasemiin. |
| Vaikutus: Voiko vika vaikuttaa lukijoihin, kelloihin, näyttötauluihin tai tulostimiin? | Mahdollinen | Sirit- tai Zebra-lukija lakkaa syöttämästä tietoa ohjelmalle uudelleenyhteyden jälkeen; ei ajettu. |
| Vaikutus: Voiko vika vaikuttaa tulosteisiin, HTML- tai XML-tiedostoihin, tulospalveluun tai SQL-peiliin? | Ei tiedossa | Aineisto ei käsittele vaikutusta tulosteisiin tai julkaisuun. |
| Vaikutus: Voiko ulkopuolinen tai verkossa oleva laite käyttää vikaa hyväkseen? | Ei vaikutusta (perusteltu) | Vian laukaisee yhteyskatko tai käyttäjän tekemä uudelleenyhteys, ei verkosta tuleva sisältö. |
| Ilmeneminen: Voiko vika ilmetä tavallisessa kilpailukäytössä ilman erityisasetusta tai virhettä? | Mahdollinen | Käyttäjän pyytämä uudelleenyhteys (Sirit) laukaisee saman haaran ilman vikatilannetta; vaatii Sirit- tai Zebra-lukijan käyttöönoton. Ei ajettu. |
| Ilmeneminen: Voiko vika ilmetä poikkeustilanteessa (virhe, katkos, virheellinen syöte, harvinainen asetus)? | Mahdollinen | Yhteyskatko (virrankatko, kaapelin irrotus, lukijan uudelleenkäynnistys) laukaisee uudelleenyhteyden; päätelty koodista. |
| Ilmeneminen: Voiko käyttäjä huomata vian sen ilmetessä? | Mahdollinen | Koodin mukaan ohjelma antaa ilmoituksen, että jatko vaatii uudelleenkäynnistyksen, kun säie päättyy; ei ajettu. |
| Hallinta: Onko aineistossa kirjattu keino välttää vika tai pienentää sen riskiä? | Mahdollinen | muistiturvallisuus.md luvun 15 suositus 5 (seurataan ajanottosyötteen etenemistä, käynnistetään uudelleen, jos lukija pysähtyy) on kirjattu havainnoille H-056 ja H-070; sen tehoa tähän vikaan ei ole arvioitu. |
| Hallinta: Onko aineistossa kirjattu keino palautua vian seurauksista kilpailun aikana? | Mahdollinen | Riskiarvion mukaan palautus tapahtuu ohjelman uudelleenkäynnistyksellä; keinoa ei ole kokeiltu. |

Arvot: **Vahvistettu** = seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla; Mahdollinen = seuraa koodista, mutta seurausta ei ole osoitettu; Ei vaikutusta (perusteltu) = aineisto osoittaa, ettei seurausta synny; Ei tiedossa = aineisto ei ratkaise; _Ei arvioitu_ = riviä ei ole vielä arvioitu. Tiedon puute ei tarkoita, ettei vaikutusta ole. Lähteet: `doc/arkkitehtuuri/rekisteri/vaikutukset.csv`.

### 7. Mitä kilpailun aikana voi tapahtua

Kaikki alla oleva on mahdollista (koodista luettu, ei ajettu; ks. luku 11).

Kilpailun aikana RFID-lukijan virta katkeaa tai kaapeli irtoaa hetkeksi. Ohjelma avaa yhteyden
uudelleen ja antaa ilmoituksen, että jatko vaatii uudelleenkäynnistyksen. Lukijan leimauksia ei
lueta, ennen kuin ohjelma on käynnistetty uudelleen. Riskiarvio kirjaa ajanoton jatkuvuudelle
seuraukseksi "pysäyttää" ja tulosten oikeellisuudelle "epäsuora". Aineisto ei kuvaa tarkemmin,
miten puuttuvat leimaukset vaikuttavat tuloksiin, eikä sitä, voiko laite säilyttää tiedot
uudelleenkäynnistykseen asti.

### 8. Palautuminen ja uudelleenkäynnistys

- Riskiarvio kirjaa palautettavuudeksi "uudelleenkäynnistys". Aineistossa ei kerrota, onko
  uudelleenkäynnistys turvallinen kesken kilpailun, eikä sitä, mitä leimauksille tapahtuu katkon
  aikana.
- Muut työasemat: aineisto ei kerro. Vika koskee koodin mukaan yhtä lukijasäiettä kullakin
  työasemalla.

### 9. Mitä käyttäjä voi tehdä riskin pienentämiseksi

Aineistossa ei ole tälle havainnolle kirjattua keinoa vian välttämiseksi. Väliaikaisten
suojaussuositusten (`muistiturvallisuus.md` luku 15) suositus 5 koskee laitelukijoita:
seurataan, että ajanottosyöte etenee, ja käynnistetään ohjelma uudelleen, jos lukija pysähtyy.
Suositus on kirjattu havainnoille H-056 ja H-070; sen tehoa tähän vikaan ei ole arvioitu.
Ainoa kirjattu palautuskeino on ohjelman uudelleenkäynnistys (luku 8).

### 10. Tekninen juurisyy

`lue_regnlyThread`-silmukka (`Tp/TpLaitteet.cpp`) kutsuu `getRfidReader(r_no)->openConnection(r_no,
true)` ja tekee ehdon `if (!openConnection(...)) regnly_open[r_no] = 0;`. Rajapinta
(`IRfidReader.h`) määrittelee 0 = onnistui. Sirit-toteutus `openSirit2` palauttaa uudelleenyhteys-
tilassa aina 0 (1 vain, jos TCP-yhteyden avaus epäonnistuu muussa kuin uudelleenyhteystilassa).
Zebra-toteutus (`Tp/ZebraReader.cpp`) palauttaa 0 onnistuneesta ja 1 epäonnistuneesta avauksesta.
Zebra-haarassa ehto on sama, mutta Zebran avaus palauttaa 0 vasta onnistuessaan, joten säie
päättyy vasta ensimmäiseen onnistuneeseen uudelleenyhteyteen.

Käyttäjän pyytämä uudelleenyhteys Sirit-lukijalle (`reconnectSirit`, `cbHk/UnitAjanotto.cpp`)
käyttää samaa haaraa.

Juurisyy JS-9 (laiteyhteydet käsin, ei elävyysvalvontaa). Kokonaisuuden juurisyy T-048.

### 11. Mitä on todennettu

(a) Ohjelmointivirhe: koodipolku luettu pääistunnossa ja vahvistettu riippumattomassa
lähdetarkastuksessa (C-10 osa F, K1): paluuarvojen merkitys, silmukan päättyminen ja
ohjelman käynnistysfunktion estävä ehto. Ajoa tai koeohjelmaa ei ole.

(b) Käytännön seuraus: ei osoitettu. C-10:ssä ei ajettu mitään, joten ajonaikainen vaikutus
on päätelty koodista.

### 12. Mitä ei vielä tiedetä

- Q96: päättyykö Sirit/Zebra-lukijasäie uudelleenyhteyden jälkeen ajossa. Suunniteltu koe on
  ajo TCP-tynkälukijaa vastaan.
- Mitä lukijalaite tekee, kun ohjelma ei lue sitä (pysyvätkö leimaukset laitteessa).
- Näkyykö lukijan tila ohjelman ikkunassa ilmoituksen lisäksi.
- Onko ViestiLuenta altis viralle: koodi ei ole mukana siinä käännöksessä (`LAJUNEN`).

### 13. Miten vika on tarkoitus korjata

`korjaussuunnitelma.md`, kokonaisuus KK-C10-2 (aalto 1), PR 1: H-159:n ehto korjataan
ensimmäisenä. Suunnitelma kuvaa sen yhden ehdon korjaukseksi, joka voidaan palauttaa yksinään.
Hyväksymiskriteeri: uudelleenyhteyden jälkeen lukijasäie jatkaa (testi TCP-tynkälukijalla) ja
laitetesti vähintään yhdellä oikealla lukijalla ennen julkaisua.

Päätöstä odottava: OP-13 (laitteet ja ulkoiset palvelut): mitä laitteita on käytettävissä
testaukseen. H-159:llä ei ole omaa OP-kohtaa.

### 14. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | H-159 |
| Rekisterin kuvaus | Sirit/Zebra-lukijan uudelleenyhteys testaa `openConnection`in paluuarvon väärinpäin (`if (!...openConnection(r_no, true)) regnly_open[r_no] = 0;`), vaikka 0 = onnistui; Siritin `openSirit2` palauttaa uudelleenyhteystilassa aina 0, joten lukijasäie päättyy jokaisen uudelleenyhteysyrityksen jälkeen, Zebran ensimmäisen onnistuneen jälkeen; ilmoitus "Häiriö... vaatii uudelleenkäynnistyksen", eikä säiettä voi käynnistää uudelleen ilman ohjelman uudelleenkäynnistystä (alkuperäinen 2015) / RFID reconnect checks the result inverted: the reader thread ends after a reconnect until the program is restarted Prioriteetti / priority **P2**; korjauskokonaisuus / fix package **KK-C10-2** (`../riippuvuudet.md` 19.3–19.4). |
| Erä | C-10 |
| Tila | avoin |
| Vakavuus (rekisteri) | korkea / high |
| Varmuus (rekisteri) | [todennettu] mekanismi; [päätelty] ajonaikainen vaikutus / run-time effect |
| Todennus (rekisteri) | riippumaton lähdetarkastus / independent source review (C-10 F, K1) |
| Prioriteetti | P2 |
| Riskiarvio | vakavuus korkea; saavutettavuus laite; todennus riippumaton; tulokset epasuora; ajanotto pysayttaa; palautettavuus uudelleenkaynnistys; tietoturva ei |
| Riskiarvion peruste | Unchanged: Sirit/Zebra reconnect ends the reader thread until restart. Four products; not verified at run time. |
| Lähde (raportti) | `../riippuvuudet.md` luku / section 16 |
| Näytön laji | staattinen, lähdekoodi / static, source; ei ajonaikaista todennusta / not verified at run time (C-10) |
| Juurisyy | JS-9 Laiteyhteydet käsin, ei elävyysvalvontaa |
| Ohjelmat | HkKisaWin HkMaali ViestiWin ViestiMaali (peruste: tarkistettu) |
| Korjauskokonaisuus | KK-C10-2 Laiteyhteyksien hallinta; vaihe 1, aalto 1; haara `korjaus/kk-c10-2-laiteyhteydet` |
| Omistajan päätökset, joissa havainto mainitaan | - |
| Kokonaisuuden KK-C10-2 omistajan päätökset | OP-13 Laitteet ja ulkoiset palvelut (avoin) |
| Kysymykset | Q96 (avoin) |
| Liittyvät havainnot | - |
| Lähdeviitteet (revisio 4b48749, polut `TPsource/V52/`-hakemistosta) | `Tp/TpLaitteet.cpp:1805-1819`, `:1835-1847`, `:1872-1874`, `:1898`, `:2242`, `:2253-2254`, `:2262`, `:2329`; `Tp/ZebraReader.cpp:939-947`, `:1001`; `cbHk/UnitAjanotto.cpp:1622`; `../todisteet/c10/raportit/osaD-laitteet.md` N-1, `osaF-tarkastus.md` K1 |

Rekisteririvien tiiviste nyt: `cb4df2fee777`.


---

<a id="h-171"></a>

## H-171 – ViestiWin kirjoittaa joka kilpailun avauksessa 158 tavua asetustiedoston nimen puskurin ohi viereiseen muistiin

> **H-171** · tila: **avoin** · prioriteetti: **P2** · tekninen vakavuus: korkea
> Ohjelmointivirheen todennus: luettu koodista ja riippumattomasti tarkastettu. Riskiarvion saavutettavuus: tavallisessa käytössä (tarkemmin luvussa 4).
> Ohjelmat: ViestiWin (tarkistettu kortin kirjoituksen yhteydessä).
> Korjauskokonaisuus: KK-C12-2 Asetusten hallinta (aalto 1).

### 1. Lyhyesti

Kun ViestiWin avaa kilpailun, se kopioi asetustiedoston nimen muistialueeseen, joka on paljon lyhyempi kuin ohjelman kirjoittama määrä. Ylimenevä osa kirjoittuu viereisen muistin päälle riippumatta siitä, kuinka lyhyt nimi on. Ylikirjoitus tapahtuu jokaisessa avauksessa. Tarkat koot ovat luvussa 10.

**Luettu koodista ja riippumattomasti tarkastettu:** ylitys, sen koko ja polku. Puskurin sijoittelu muistissa on luettu ohjelman olemassa olevasta käännöstiedostosta, ei ajosta.
**Vahvistettu käytännön seuraus:** ei yhtään.
**Mahdollinen:** viereisten asetusmuuttujien nollautuminen tai vääristyminen (sarjaporttien oletusasetus, listauksen ohjaus tulostimelle). Ajonaikaiset seuraukset ovat tuntemattomia (Q107). Ohjelman kaatumista ei ole oletettu eikä väitetty.

### 2. Mitä ohjelma yrittää tehdä

Kun käyttäjä avaa kilpailun ViestiWinissä (avausikkunasta tai `Init.cfg`-tiedoston `CFG=`-rivin mukaan), ohjelma lukee kilpailun asetustiedoston. Ensin se tallentaa asetustiedoston nimen omaan muistialueeseensa (ks. sanasto: Asetustiedosto, Muistialue, puskuri) ja lukee sitten asetukset.

### 3. Mikä menee vikaan

Nimelle varattu muistialue on lyhyt, ja nimen kopiointi pyytää kopioimaan selvästi enemmän kuin alueeseen mahtuu. Kopiointifunktio täyttää kohteen nollilla koko pyydettyyn pituuteen asti, joten jokainen ei-tyhjä nimi ylittää alueen. Ohi menevä osa on pääosin nollia, mutta pitkän nimen merkkejä valuu alueen ohi jo 42. merkistä alkaen (ks. sanasto: Puskuriylitys, ylikirjoitus). Hyvin pitkä nimi jää lisäksi päättämättä.

Mitä muistialueen viereen osuu, riippuu kääntäjän sijoittelusta. Rekisterin mukaan GUI-ohjelman (bcc32) nykyisessä käännöstiedostossa ylikirjoitettavalla alueella ovat listauksen ohjaukseen liittyvä merkkijono, kaksi käyttämätöntä muuttujaa ja sarjaporttien asetusmerkkijonot porteilta 1–3.

### 4. Missä tilanteessa vika voi ilmetä

- **Ohjelmat:** vain ViestiWin. Perusteena on, että avauskutsu tehdään ViestiWinin lomakkeen koodista ja tiedoston nimi annetaan aina. Konsoliohjelmat kutsuvat polkua ilman nimeä, joten niissä kirjoitus ei tapahdu (raportin mukaan). HK:ssa vastaava muistialue on selvästi suurempi.
- **Asetus ja toimenpide:** ei erityisasetusta. Ylitys tapahtuu joka kerta, kun ViestiWin avaa kilpailun, myös uudelleenavauksessa. Nimi on aina ei-tyhjä täysi polku.
- **Normaalikäyttö vai poikkeustilanne:** normaalikäyttö; virhettä, katkosta tai harvinaista asetusta ei tarvita. Ylikirjoitus on deterministinen. Se, mitä se rikkoo, riippuu kääntäjän sijoittelusta ja voi muuttua, kun tiedostoon lisätään muuttujia.

### 5. Mitä käyttäjä voi huomata

Aineisto ei kerro (Q107). Ohjelma ei ilmoita ylikirjoituksesta. Koodin perusteella mahdollisia oireita on kaksi:

- sarjaporttien oletusasetus (`9600,n,8,1`) häviää porteilta 1–3, joten sarjayhteys ilman `BAUD`-asetusta jää numeeristen oletusten varaan [päätelty];
- jos nimi on yli 65 merkkiä pitkä ja sen 66. merkki on `1`, GUI:n listaus ohjautuu tulostimelle tiedoston sijaan [päätelty ylikirjoitettavan alueen järjestyksestä; tarkkaa sijaintia ei ole varmistettu, Q107].

Vika voi jäädä kokonaan huomaamatta, jos ylikirjoitettavilla alueilla ei ole merkitystä käyttäjän asetuksille.

### 6. Vaikutukset kilpailuun

| Kysymys | Arvio | Peruste |
|---|---|---|
| Vaikutus: Voiko aika, tulos, sijoitus, pisteet tai hylkäys olla väärä? | Ei tiedossa | Riskiarvio: välillinen vaikutus, mutta aineisto ei kerro tulosketjua (Q107). |
| Vaikutus: Voiko maali- tai väliaikojen kirjaus pysähtyä tai katketa? | Ei tiedossa | Riskiarvio: ei vaikutusta, mutta sama raportti toteaa porttien 1-3 oletuksen häviävän; ajonaikaiset seuraukset tuntemattomia (Q107). |
| Vaikutus: Voiko leimaus- tai kortintiedon luku jäädä tekemättä tai mennä väärin? | Ei tiedossa | Aineisto ei kerro vaikutusta leimauksiin. |
| Vaikutus: Voiko tallennettu tai vastaanotettu tieto kadota tai vioittua? | Ei tiedossa | Ylikirjoitus koskee staattista muistia, ei tiedostotietoa; muuta ei kerrota. |
| Vaikutus: Voiko ohjelma kaatua tai sulkeutua odottamatta? | Ei tiedossa | Kaatumista ei ole oletettu eikä väitetty; ajonaikaiset seuraukset tuntemattomia (Q107). |
| Vaikutus: Voiko ohjelma tai yksittäinen toiminto jäädä jumiin? | Ei tiedossa | Aineisto ei kerro jumiutumisesta. |
| Vaikutus: Voiko jokin mennä väärin ilman ilmoitusta tai näkyvää oiretta? | Mahdollinen | Ylikirjoituksesta ei ilmoiteta; viereiset asetukset nollautuvat tai saavat nimen merkkejä. |
| Vaikutus: Voiko vika vaikuttaa tiedonsiirtoon tai muihin työasemiin? | Mahdollinen | Porttien 1-3 oletusasetus 9600,n,8,1 häviää; sarjayhteys jää numeeristen oletusten varaan (päätelty). |
| Vaikutus: Voiko vika vaikuttaa lukijoihin, kelloihin, näyttötauluihin tai tulostimiin? | Mahdollinen | Ylitys peittää tulostinohjauksen kentän pitkällä nimellä (mahdollinen); sarjanopeusasetukset koskevat tiedonsiirtoyhteyksiä 1-3, ei laitteita. |
| Vaikutus: Voiko vika vaikuttaa tulosteisiin, HTML- tai XML-tiedostoihin, tulospalveluun tai SQL-peiliin? | Mahdollinen | Listauksen kohde voisi muuttua tulostimeksi, jos nimen 66. merkki on 1. |
| Vaikutus: Voiko ulkopuolinen tai verkossa oleva laite käyttää vikaa hyväkseen? | Ei vaikutusta (perusteltu) | Syöte on paikallisen operaattorin valitsema tiedostopolku, ei verkosta; riskiarvio: tietoturva ei. |
| Ilmeneminen: Voiko vika ilmetä tavallisessa kilpailukäytössä ilman erityisasetusta tai virhettä? | Mahdollinen | Ylitys tapahtuu jokaisessa kilpailun avauksessa; luettu koodista ja objektitiedostosta, ei ajettu. |
| Ilmeneminen: Voiko vika ilmetä poikkeustilanteessa (virhe, katkos, virheellinen syöte, harvinainen asetus)? | Ei vaikutusta (perusteltu) | Laukeaa ilman poikkeustilannetta: jokaisessa avauksessa, myös normaalisti. |
| Ilmeneminen: Voiko käyttäjä huomata vian sen ilmetessä? | Ei tiedossa | Ajonaikaista oiretta ei ole tutkittu (Q107). |
| Hallinta: Onko aineistossa kirjattu keino välttää vika tai pienentää sen riskiä? | Ei tiedossa | Aineistossa ei ole kirjattua keinoa. |
| Hallinta: Onko aineistossa kirjattu keino palautua vian seurauksista kilpailun aikana? | Mahdollinen | Riskiarvio: palautettavuus uudelleenkäynnistyksellä; tehoa ei ole kokeiltu ja ylikirjoitus toistuu joka avauksessa. |

Arvot: **Vahvistettu** = seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla; Mahdollinen = seuraa koodista, mutta seurausta ei ole osoitettu; Ei vaikutusta (perusteltu) = aineisto osoittaa, ettei seurausta synny; Ei tiedossa = aineisto ei ratkaise; _Ei arvioitu_ = riviä ei ole vielä arvioitu. Tiedon puute ei tarkoita, ettei vaikutusta ole. Lähteet: `doc/arkkitehtuuri/rekisteri/vaikutukset.csv`.

### 7. Mitä kilpailun aikana voi tapahtua

Vahvistetun ja mahdollisen erottelu on luvussa 11. Ajonaikaista tarkistusta ei ole tehty.

- Sarjaporttien asetusten häviäminen porteilta 1–3 vaikuttaisi sarjaliikenteellä toimiviin tiedonsiirtoyhteyksiin 1–3, joilla ei ole omaa `BAUD`-asetusta [päätelty]. Aineisto ei kerro, käyttävätkö laitepolut samaa asetustaulua. Listauksen ohjautuminen tulostimelle vaikuttaisi tulosteisiin. Riskiarvio pitää vaikutusta tuloksiin välillisenä ja ajanoton jatkuvuuteen ei vaikuttavana. Tämä riskiarvion arvio ei ole ajossa todennettu: sama raportti toteaa, että sarjayhteys porteilla 1–3 jää oletusten varaan.

### 8. Palautuminen ja uudelleenkäynnistys

Riskiarvio luokittelee palautettavuuden muotoon "uudelleenkäynnistys". Keinon tehoa ei ole kokeiltu. Koska ylikirjoitus tapahtuu jokaisessa kilpailun avauksessa ja arvot palautuvat ohjelman käynnistyessä alkuarvoihin, uudelleenkäynnistys ei näytä poistavan syytä [päätelty]. Aineisto ei kerro, kuinka asetukset palautetaan kesken kilpailun. Vika on yhden työaseman ohjelman sisäinen; sen vaikutuksesta muihin työasemiin aineisto ei kerro.

### 9. Mitä käyttäjä voi tehdä riskin pienentämiseksi

Aineistossa ei ole kirjattua keinoa. Ylikirjoitus tapahtuu kaikilla ei-tyhjillä nimillä, joten nimen lyhentäminen ei koodin mukaan poista sitä; se muuttaa vain sitä, mitä alueelle kirjoittuu [päätelty].

### 10. Tekninen juurisyy

Nimelle varattu alue on 41 merkkiä eli 82 tavua (`wchar_t` on 2 tavua), ja kopiointi pyytää 120 merkkiä. Koska `wcsncpy` täyttää kohteen nollilla pyydettyyn pituuteen asti, jokainen ei-tyhjä nimi kirjoittaa 240 tavua, joista 158 tavua (79 merkkiä) menee puskurin ohi. Nimen merkkejä valuu ohi jo 42. merkistä alkaen; ne osuvat ensin `autostr`-taulukon alkuun (siirtymä 0x5C). Raja 65 koskee vain kenttää `autostr[24]`: nimen 66. merkki (indeksi 65) päätyy siihen, ja arvo `'1'` ohjaa listauksen tulostimelle (`Juk/VInit.cpp`, `ominaisuusvalitsimet.md` luku 13). Kentän tarkka siirtymä on varmistamatta (`.map`, Q107). Yli 119 merkin nimi jää päättämättä.

Funktio `lue_parametrit` (`Juk/VInit.cpp`) kopioi asetustiedoston nimen kutsulla `wcsncpy(cfgfilename, cfgflname, 120)` staattiseen muuttujaan `wchar_t cfgfilename[41]`. Funktio `wcsncpy` täyttää kohteen nollilla pyydettyyn pituuteen asti, joten kopio kirjoittaa 120 merkkiä aina. Funktiota kutsutaan polusta `Initialisoi` (`ViestiWin/UnitMain.cpp`) → `aloitus` → `lue_parametrit`. Sijoittelu on luettu bcc32-objektista (`RADStudio10/Release_Build/obj/VInit.obj`), ei ajosta: puskuri on `_DATA`-osassa siirtymässä 0xA, ja ylikirjoitettava alue peittää siirtymät 0xA–0xF9. Kutsu tehdään avaimella `BAUDn` asetettavaan `baudst`-taulukkoon, joka koskee tiedonsiirtoyhteyttä n (työasemien sarjayhteydet 1–3; `Juk/VInit.cpp`, `Tp/TpComY.cpp`). Sarjaporttitaulukko `baudst` on `MAX_PORTTI` × 20 merkkiä, ja `MAX_PORTTI` on 132 (`Juk/VDef.h`). Puskurikoko `wchar_t cfgfilename[201]` on HK:ssa (`Hk/HkInit.cpp`).

Juurisyy on JS-3: kapasiteetti ei seuraa ajonaikaista kokoa.

### 11. Mitä on todennettu

**(a) Ohjelmointivirhe.** Ylitys, sen koko ja polku on todennettu lähdekoodista. C-12:n riippumaton lähdetarkastus ja pääistunto ovat vahvistaneet sen. Sijoittelu on todennettu olemassa olevan bcc32-objektin alkuarvoista (ei ajonaikaisesti).

**(b) Käytännön seuraus.** Ei osoitettu. Ajonaikaisia seurauksia ei ole tutkittu (Q107). Kaatumista ei ole oletettu eikä väitetty.

### 12. Mitä ei vielä tiedetä

- Q107: mitä bcc32 sijoittaa ViestiWinissä `cfgfilename`n jälkeen ja nollautuvatko `autostr` ja `baudst` käytännössä. Kysymys on siirretty ajonaikaiseen todentamiseen (`.map`-tiedosto tai vahtipiste), joka ajetaan vain erillisellä luvalla.
- Kuinka kokonaisuudessaan ylikirjoitus vaikuttaa sarjaporttien toimintaan käytännössä. Aineisto ei kerro.
- Testimatriisin rivi T10 (pitkä polku) on suunniteltu, mutta ei ajettu.

### 13. Miten vika on tarkoitus korjata

Korjaussuunnitelman kokonaisuus KK-C12-2 (Asetusten hallinta), sarja a, ensimmäinen pull request: ViestiWinin `cfgfilename` saa saman koon kuin HK:ssa (201 merkkiä, kopio 120). Kopioitava pituus ei saa ylittää puskuria. Korjaus ei muuta tiedostomuotoa: nimeä vain luetaan ja näytetään, sitä ei kirjoiteta tiedostoon eikä lähetetä. Hyväksymiskriteerit: kopiointi ei ylitä puskuria ja sarjaporttien oletusasetus säilyy kilpailun avauksen jälkeen. Testiksi suunnitellaan käännösaikainen kokotarkistus ja ViestiWinin GUI-avaus pitkällä polulla (Q107).

Päätöstä odottavat osat: ei tälle havainnolle. Kokonaisuuden sarja b (avainten yhdenmukaistaminen) odottaa päätöstä OP-11 ja tuettujen kokoonpanojen osalta OP-12.

### 14. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | H-171 |
| Rekisterin kuvaus | ViestiWin: `aloitus` → `lue_parametrit` kopioi asetustiedoston nimen `wcsncpy(cfgfilename, cfgflname, 120)` staattiseen puskuriin `wchar_t cfgfilename[41]` (82 tavua bcc32:ssa, wchar_t = 2 tavua). `wcsncpy` täyttää nollilla 120 merkkiin asti, joten jokainen ei-tyhjä nimi kirjoittaa 240 tavua, eli 158 tavua (79 merkkiä) yli polun pituudesta riippumatta. Kutsu tapahtuu jokaisessa kilpailun avauksessa (`Initialisoi` → `aloitus(0, 0, cfgflnm)`, nimi aina avausdialogista tai `Init.cfg`:n `CFG=`-riviltä). Nykyisen lähteen bcc32-objektissa (`RADStudio10/Release_Build/obj/VInit.obj`, `_DATA`) puskuri on siirtymässä 0xA ja kirjoitusalue 0xA–0xF9 peittää `autostr[25]`, `korkeat`, `star` ja `baudst[0]`–`baudst[2]` (osin); `baudst` on 132 × 20 merkkiä (`MAX_PORTTI` = 132). Lyhyellä polulla alueeseen tulee nollia (oletus `9600,n,8,1` häviää porteilta 1–3; `asetaOletusarvot` on jo kopioinut sen kaikille porteille); yli 65 merkin polulla polun merkkejä päätyy `autostr[24]`:een (arvo `'1'` ohjaisi GUI:n listauksen tulostimelle), yli 71 merkin polulla `baudst[0]`:aan; `korkeat` ja `star` ovat kuolleita. HK:ssa puskuri on 201 merkkiä. Toistuva staattisen muistin ylikirjoitus normaalikäytössä → korkea, P2 / ViestiWin overwrites 158 bytes of static data on every competition open |
| Erä | C-12 |
| Tila | avoin |
| Vakavuus (rekisteri) | korkea / high (P2) |
| Varmuus (rekisteri) | [todennettu] ylitys, koko ja polku lähteestä; sijoittelu [todennettu] olemassa olevan bcc32-objektin alkuarvoista (ei ajonaikaisesti); ajonaikaiset seuraukset [tuntematon] (Q107) |
| Todennus (rekisteri) | riippumaton lähdetarkastus (C-12 D, Opus), pääistunto; sijoittelu objektitiedostosta C-12:n sulkemisessa / object-file layout at the C-12 closing |
| Prioriteetti | P2 |
| Riskiarvio | vakavuus korkea; saavutettavuus normaalikaytto; todennus riippumaton; tulokset epasuora; ajanotto ei; palautettavuus uudelleenkaynnistys; tietoturva ei |
| Riskiarvion peruste | Unchanged: ViestiWin overwrites 158 bytes of static data at every competition open. Run-time effect unknown (Q107); layout from the object file; HK buffer is 201 characters. |
| Lähde (raportti) | `../ominaisuusvalitsimet.md` |
| Näytön laji | staattinen, lähdekoodi / static, source |
| Juurisyy | JS-3 Kapasiteetti ei seuraa ajonaikaista kokoa |
| Ohjelmat | ViestiWin (peruste: tarkistettu) |
| Korjauskokonaisuus | KK-C12-2 Asetusten hallinta; vaihe 1, aalto 1; haara `korjaus/kk-c12-2-asetukset` |
| Omistajan päätökset, joissa havainto mainitaan | OP-11 Asetusavainten kanoninen muoto ja yksiköt (avoin) |
| Kokonaisuuden KK-C12-2 omistajan päätökset | OP-12 Tuetut tuotteet, kokoonpanot ja käyttöjärjestelmät (avoin) |
| Kysymykset | Q107 (ajonaikainen_todennus) |
| Liittyvät havainnot | - |
| Lähdeviitteet (revisio ab10f88, polut `TPsource/V52/`-hakemistosta) | `Juk/VInit.cpp:74-78`; `Juk/VInit.cpp:652`; `Juk/VInit.cpp:2460-2506`; `Juk/VInit.cpp:2570`; `Juk/VInit.cpp:2685`; `Juk/VDef.h:62`; `ViestiWin/UnitMain.cpp:192-220`; `ViestiWin/UnitMain.cpp:1403`; `ViestiWin/UnitMain.cpp:1542`; `Hk/HkInit.cpp:59` |

Rekisteririvien tiiviste nyt: `0062ade69c11`.


---

<a id="t-038"></a>

## T-038 – Vaiheen vaihto nollaa tilaa tapauskohtaisesti, ei yhdestä paikasta (HK)

> **T-038** · tila: **avoin** · vakavuus: keskitaso
> Ohjelmat: HkKisaWin, HkMaali (tarkistettu kortin kirjoituksen yhteydessä).
> Korjauskokonaisuus: KK-C13-20 Vaihetilan ja johdettujen rakenteiden keskitetty hallinta (aalto 5).

### 1. Mistä on kyse

Tämä ei ole yksittäinen vika vaan rakenne, joka tekee vioista todennäköisempiä (sanasto:
[tekninen velka](../sanasto.md)).

HK-ohjelma voi vaihtaa monivaiheisen kilpailun vaihetta käynnissä olevassa ohjelmassa
([vaiheen vaihto](../sanasto.md)). Ohjelman käynnistyksessä kaikki muistissa olevat tiedot
rakennetaan alusta. Vaiheen vaihdossa (`vaihdavaihe`) ei ole vastaavaa yhtä rakennuskohtaa
eikä luetteloa siitä, mikä tieto riippuu vaiheesta. Ohjelma nollaa jokaisen kohteen erikseen
omassa kohdassaan. Lisäksi käynnistys (`aloitus`) muokkaa tiedostonimiä paikallaan
(merkkien siirto nimessä ja numeron vaihto).

Vertaus: käynnistys on kuin uusi aloitus puhtaalta pöydältä, kun taas vaiheen vaihto on pöydän
siivoamista muistin varassa ilman luetteloa siitä, mitä kaikkea pöydällä on. Jokainen uusi
vaiheesta riippuva muuttuja on unohtumisvaarassa, ellei joku muista lisätä sitä siivouskohtaan.

Rakenne koskee HkKisaWinia ja HkMaalia. Viestiohjelmissa ei ole vaiheen vaihtoa.

### 2. Mitä haittaa siitä on nykyisin

Aineiston mukaan jokainen uusi vaiheesta riippuva globaali muuttuja jää helposti vanhaksi
vaiheen vaihdon jälkeen, koska mikään ei pakota nollaamaan sitä. Käytännön haitta
on ne viat, joita tämä on jo aiheuttanut (kohta 3). Haitta ylläpidolle: korjaaja joutuu
etsimään vaiheen vaihdon nollauskohdat käsin. Muuta haittaa aineisto ei kirjaa.

### 3. Mihin vikoihin se liittyy

Rekisterin mukaan rakenne on taustalla havainnoissa H-131, H-132, H-133, H-141 ja H-143:

- **H-131:** vaiheen vaihto päivittää olemassa olevien aikajonojen tiedostonimen vaihenumeron,
  mutta ei nimipohjaa. Myöhemmin luotu jono voi kirjoittaa edellisen vaiheen tiedostoon (päätelty).
- **H-132:** yksivaiheisessa kilpailussa käynnistys poistaa viidennen merkin aikajonotiedoston
  nimestä, myös käyttäjän antamasta nimestä.
- **H-133:** käynnistys ei ole turvallisesti uudelleen ajettava; tiedostonimen muokkaus
  paikallaan ei ole toistettavissa.
- **H-141:** vaiheen vaihto ei nollaa erälähtötietoa eikä päällekkäisten korttien laskuria.
- **H-143:** kokoomarivi pienistä elinkaaren vioista, esimerkiksi vanhaksi jäävä `maaliajat`.

Samasta aihepiiristä on myös T-039 (johdettujen rakenteiden käsin ylläpito) ja H-129, jossa
numerohakemiston rivilaskuri jää nollaamatta.

### 4. Mitä hyötyä sen vähentämisestä olisi

Korjaussuunnitelman tavoite on, että vaiheen vaihdon jälkeen muistin hakemistot,
tiedostokahvat ja ratatiedot vastaavat tuoretta käynnistystä, ja että käynnistys ja
vaiheen vaihto käyttävät samaa nollaus- ja uudelleenrakennuspolkua sekä johdettujen
rakenteiden invarianttitarkistusta. Muuta mitattua hyötyä aineisto ei kirjaa.

### 5. Miten se on suunniteltu käsiteltäväksi

Kokonaisuus KK-C13-20 (Vaihetilan ja johdettujen rakenteiden keskitetty hallinta): yksi
nollaus- ja uudelleenrakennuspolku, jota sekä käynnistys että vaiheen vaihto käyttävät, ja
invarianttitarkistus johdetuille rakenteille. Työmäärä on suunnitelmassa arvioitu
erittäin suureksi ja muutosriski korkeaksi. Tiedostomuodot pysyvät ennallaan, mutta
tiedostonimien käsittely vaiheittain on suunniteltava. Kokonaisuus riippuu kokonaisuuksista
KK-C13-3, KK-C13-16 ja KK-C13-8, ja se vaatii kattavat vaiheen vaihdon regressiotestit ennen
aloitusta. Yksittäiset vaiheen vaihdon viat korjataan ensin kokonaisuudessa KK-C13-3.
Päätöstä odottaa OP-6 (vaiheen vaihdon tuettu käyttötapa ja nimeäminen, avoin).

### 6. Mitä ei tiedetä

- Käytetäänkö vaiheen vaihtoa käynnissä olevassa ohjelmassa oikeissa asennuksissa vai
  käynnistetäänkö ohjelma uudelleen (Q78, OP-6). Jos uudelleenkäynnistys on ohjeena, osa
  korjauksista on vähemmän kiireellisiä (OP-6:n perustelu).
- Täydellistä luetteloa vaiheesta riippuvista muuttujista ei ole tässä kortissa; osaraportti
  (`globaali-tila.md` luku 4.2) sisältää nollaustaulukon.

### 7. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | T-038 |
| Rekisterin kuvaus | Vaiheriippuvaa tilaa ei nollata keskitetysti: `vaihdavaihe` nollaa kohteita tapauskohtaisesti ja `aloitus` muokkaa tiedostonimiä paikallaan (`wmemmove`, numeron vaihto), joten uusi vaiheriippuva globaali jää helposti vanhaksi (H-131, H-132, H-133, H-141, H-143) / no central reset for stage-dependent state |
| Erä | C-8 |
| Tila | avoin |
| Vakavuus (rekisteri) | keskitaso / medium |
| Varmuus (rekisteri) | [todennettu] |
| Näytön laji | staattinen, lähdekoodi / static, source |
| Juurisyy | JS-6 Muistitila rakennetaan vain käynnistyksessä |
| Ohjelmat | HkKisaWin HkMaali (peruste: tarkistettu) |
| Korjauskokonaisuus | KK-C13-20 Vaihetilan ja johdettujen rakenteiden keskitetty hallinta; vaihe 4, aalto 5; haara `rakenne/kk-c13-20-vaihetila` |
| Omistajan päätökset, joissa havainto mainitaan | - |
| Kokonaisuuden KK-C13-20 omistajan päätökset | OP-6 Vaiheen vaihdon tuettu käyttötapa ja nimeäminen (avoin) |
| Kysymykset | - |
| Liittyvät havainnot | H-131 H-132 H-133 H-141 H-143 |
| Lähdeviitteet (revisio f7fe553, polut `TPsource/V52/`-hakemistosta) | `Hk/HkIx.cpp:1855-2005`; `Hk/HkInit.cpp:3357-3372`; `../globaali-tila.md` luku 4.2 |

Rekisteririvien tiiviste nyt: `ad98e0833df2`.


---

<a id="t-042"></a>

## T-042 – Tulostus- ja tiedostokanavan sulkeva funktio ei voi nollata kutsujan osoitinta, joten suljettuun kanavaan jää viittauksia

> **T-042** · tila: **avoin** · vakavuus: keskitaso
> Ohjelmat: HkKisaWin, HkMaali, ViestiWin, ViestiMaali, ViestiLuenta (yläraja: ohjelmat, joihin viitatut lähdetiedostot käännetään).
> Korjauskokonaisuus: KK-C9-1 PRFILE-osoittimien elinkaari ja autofile-polku (aalto 1).

### 1. Mistä on kyse

Ohjelma kirjoittaa tulosteita (tulostimelle, tiedostoon, HTML- tai XML-julkaisuun) yhteisen
rakenteen, `PRFILE`n, kautta. Rakenne on kuin kanava, jonka kautta tuloste kulkee. Kun tuloste on
valmis, kanava suljetaan funktiolla `closeprfile`, joka vapauttaa rakenteen muistin ja poistaa sen
ohjelman omasta kanavaluettelosta.

Ongelma on se, että funktio saa kanavan osoitteen arvona, kopiona. Se ei voi tyhjentää sitä
muuttujaa, josta kutsuja osoitteen otti. Kutsuja jää pitämään osoitetta, joka ei enää osoita
mihinkään kelvolliseen. Vertaus: suljetun huoneen avain, joka jää taskuun, vaikka huone on jo
annettu toiselle. Lisäksi sama osoite kopioidaan joissakin kohdissa useaan rakenteeseen, jolloin
yhden kopion sulkeminen jättää muut kopiot osoittamaan suljettuun kanavaan.

Kutsujat nollaavat osoittimensa tapauskohtaisesti, tai eivät lainkaan. Tämä on rakenteen
ominaisuus eikä yksittäisen kutsun virhe, joten sama virhe voi toistua jokaisessa uudessa
kutsupaikassa (päätelty).

### 2. Mitä haittaa siitä on nykyisin

Aineisto kirjaa haitan vain niiden vikojen kautta, joita rakenne on aiheuttanut (luku 3):
vapautettua kanavaa käytetään uudelleen tai se vapautetaan toiseen kertaan. Seuraus riippuu
ohjelmasta ja käännöksestä, ja muistinhallinnan käyttäytyminen eroaa MSVC:llä ja C++Builderilla
(bcc32) käännettyjen ohjelmien välillä (Q91, Q95). Käyttäjälle ja ylläpidolle koituvasta haitasta
muuten, esimerkiksi korjauksen vaikeudesta, aineisto ei kerro erikseen.

### 3. Mihin vikoihin se liittyy

Rakenne on vikojen juurisyy seuraavissa (korttien tekstit ovat omissa korteissaan):

- H-149: viestin automaattitulostus (`autofile`) vapauttaa kanavan ja käyttää sitä sen jälkeen
  sekä vapauttaa sen uudelleen. Toistettu testikehyksessä (ASan); tavallisella asetuksella
  testikehyksessä (MSVC Release) käännös päättyi virhekoodiin ensimmäisellä kierroksella.
- H-151: HK:n tiedostonsiirtosäie sulkee kanavan nollaamatta osoitinta, ja seuraava
  automaattitulostus lukee vapautettua muistia. Toistettu testikehyksessä.
- H-152: tulostimen vaihto sulkee kanavan, mutta automaattitulostuksen kopio osoittaa edelleen
  suljettuun. Ei toistettu; vaatii GUI-käsittelijän.
- H-158: viestin automaattisen XML-tiedoston sisältö on vajaa, ja kanavan osoitin jää
  osoittamaan vapautettuun (sivuhavainto).

Kokonaisuuteen KK-C9-1 kuuluvat lisäksi H-150 ja H-154, mutta ne eivät ole T-042:n liittyvien
havaintojen luettelossa.

### 4. Mitä hyötyä sen vähentämisestä olisi

Aineiston mukaan suljetun kanavan käyttö ja kaksoisvapautus poistuvat, kun osoitin nollataan
jokaisessa sulkevassa kutsupaikassa. Rakenteellinen muutos (funktio nollaa kutsujan osoittimen)
poistaisi sen, että jokainen uusi kutsupaikka voi unohtaa nollauksen. Muuta perusteltua hyötyä
aineisto ei kirjaa.

### 5. Miten se on suunniteltu käsiteltäväksi

`korjaussuunnitelma.md`, kokonaisuus KK-C9-1 (aalto 1):

- Rajattu ratkaisu: osoitin nollataan jokaisessa sulkevassa kutsupaikassa ja kaksoispoisto
  poistetaan (H-149, H-151). Kopioitu osoitin päivitetään (H-152).
- Rakenteellinen ratkaisu: `closeprfile` nollaa kutsujan osoittimen. Se on rajapinnan muutos
  kaikissa kutsupaikoissa ja tulee suunnitelman viimeisenä vaiheena (PR 4).
- Julkaistavien HTML- ja XML-tiedostojen sisältö ei saa muuttua; tämä tarkistetaan vertaamalla
  tiedostoja tavu tavulta ennen korjausta ja sen jälkeen. Poikkeus on viestin yhden tiedoston
  polku, joka kaatuu nykyisellään: sille vertailukohta on saman aineiston tuloste listatilassa
  ja käsin tarkistettu sisältö (hyväksymiskriteeri 2).
- Päätöstä odottavia omistajan päätöksiä ei ole.

### 6. Mitä ei tiedetä

- Q91: miten ViestiWinin (VCL) muistinhallinta käyttäytyy H-149:n kaksoisvapautuksessa.
- Q93: toimiiko `sendtcpfile` oikein tiedonsiirtoyhteyden kanssa.
- Q95: HkKisaWinin virrat ja vaiheen vaihto (H-150).
- Onko kaikki kutsupaikat löydetty. Aineisto ei väitä kattavuutta.
- GUI-ohjelmien (bcc32) käyttäytyminen: ei ajettu.

### 7. Tekniset tiedot ja lähteet

| Kenttä | Arvo |
|---|---|
| Tunnus | T-042 |
| Rekisterin kuvaus | `closeprfile` ottaa `PRFILE`-osoittimen arvona eikä voi nollata kutsujan kenttää; kutsujat nollaavat sen tapauskohtaisesti tai eivät lainkaan, ja sama osoitin kopioidaan rakenteiden välillä (H-149, H-151, H-152) / closeprfile cannot clear the caller's pointer Korjauskokonaisuus / fix package **KK-C9-1** (`PRFILE`-elinkaari, juurisyy / root cause **T-042**; H-149, H-151, H-152, H-158; `../virheenkasittely.md` 13.4). |
| Erä | C-9 |
| Tila | avoin |
| Vakavuus (rekisteri) | keskitaso / medium |
| Varmuus (rekisteri) | [todennettu] |
| Näytön laji | staattinen, lähdekoodi; koeohjelma / static, source; probe |
| Juurisyy | JS-8 Kahvoilla ja osoittimilla ei omistajaa |
| Ohjelmat | HkKisaWin HkMaali ViestiWin ViestiMaali ViestiLuenta (peruste: tiedosto) |
| Korjauskokonaisuus | KK-C9-1 PRFILE-osoittimien elinkaari ja autofile-polku; vaihe 1, aalto 1; haara `korjaus/kk-c9-1-prfile-elinkaari` |
| Omistajan päätökset, joissa havainto mainitaan | - |
| Kysymykset | - |
| Liittyvät havainnot | H-149 H-151 H-152 H-158 |
| Lähdeviitteet (revisio 7aaca40, polut `TPsource/V52/`-hakemistosta) | `tputilv2/Wrttext.cpp:279-300`; `Juk/VTulostus.cpp:7131`, `:7200`; `Hk/HkTulostus.cpp:7060`; `Hk/HkInit.cpp:3474` |

Rekisteririvien tiiviste nyt: `94eebd5faf50`.
