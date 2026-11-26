# Vikakorttien kirjoitusohje

Ohje koskee kortteja hakemistossa `vikakortit/` sekä niiden vaikutusrivejä rekisterissä
`doc/arkkitehtuuri/rekisteri/vaikutukset.csv`. Suunnitelma: `doc/arkkitehtuuri/k-vaihe-suunnitelma.md`
luku 6.

## 1. Lukija

Kortti kirjoitetaan kokeneelle tulospalvelun käyttäjälle ja kilpailun järjestäjälle. Hän tuntee
kilpailut, leimauksen, väliajat, työasemat ja ohjelman käytön, mutta ei C++:aa, muistinhallintaa,
säikeitä eikä lukkoja. Lisäksi kortin pitää kertoa kehittäjälle riittävän tarkasti, mistä viassa on
kyse: siihen on luku 10 ja generoitu lohko 14.

Teknisiä termejä selitetään käytännön kautta. Esimerkiksi säie on ohjelman sisällä rinnakkain
toimiva työ, esimerkiksi verkkosanomien vastaanotto, ja lukko on vuoronumero, jonka avulla vain yksi
työ kerrallaan muuttaa samaa tietoa. Sama selitys kirjoitetaan sanastoon (`sanasto.md`), ja kortti
voi viitata siihen.

## 2. Ehdottomat säännöt

1. **Mitään ei keksitä.** Jokainen väite palautuu rekisteririviin (`havainnot.md`), riskiarvioon
   (`riskiarvio.csv`), auditoinnin raporttiin tai koodiin. Jos lähde ei vastaa kysymykseen,
   kirjoitetaan "aineisto ei kerro" tai "ei tiedossa".
2. **Ohjelmointivirhe ja käytännön seuraus erotetaan.** Puskuriylitys voi olla varmasti osoitettu,
   vaikka sen vaikutusta tuloksiin ei ole osoitettu. Kortti sanoo molemmat erikseen.
3. **Tiedon puuttuminen ei ole vaikutuksen puuttumista.** "Ei vaikutusta" kirjoitetaan vain, kun
   aineisto perustelee sen.
4. **Ei yleisyysarvioita** (usein, harvoin, todennäköisesti, yleensä, tyypillisesti), ellei lähde
   sano niin. Jos yleisyys riippuu käyttötavasta, sanotaan, mistä se riippuu. Tarkistus varoittaa
   näistä sanoista; jokainen varoitus pitää voida perustella lähteellä.
5. **Kiertotapana tai palautumiskeinona mainitaan vain aineistossa kirjattu keino**, esimerkiksi
   väliaikaiset suojaussuositukset (`muistiturvallisuus.md` luku 15) tai riskiarvion
   palautettavuus. Muuten: "Aineistossa ei ole kirjattua keinoa." Kirjoittajan oma järkeily
   merkitään "päätelty" ja sen perusteet kerrotaan, tai se jätetään pois.
6. **Kirjoitetussa tekstissä ei ole rivinumeroita eikä prioriteettia.** Ne ovat generoidussa
   lohkossa, jotta ne eivät vanhene erikseen. Funktioiden ja tiedostojen nimet saa kirjoittaa.
7. **Ei ohjeita vian hyödyntämiseen.** Verkkovioista kerrotaan, että verkossa oleva laite voi
   lähettää virheellisen sanoman, mutta ei sanoman rakennetta.
8. **Ei omia korjausehdotuksia.** Luku 13 kertoo vain, mitä `korjaussuunnitelma.md` sanoo.
9. Samaa asiaa ei toisteta useassa luvussa; myöhempi luku viittaa aiempaan. Erityisesti
   vahvistetun ja mahdollisen erottelu tehdään luvussa 1 lyhyesti ja luvussa 11 kokonaan; luku 7
   kuvaa vain tilanteen ja viittaa lukuun 11.
10. Tavusiirtymät, poikkeuskoodit, muuttujanimet ja kaavat kuuluvat lukuihin 10–11. Luvuissa 1–9
    asia sanotaan käytännön kielellä ("ohjelma pysähtyi äkillisesti").
11. Yhtenäinen muotoilu: sanastoviite muodossa "(ks. sanasto: Termi)", oma luku muodossa
    "luku N", kirjoittajan oma päättely merkinnällä [päätelty].
12. Käännöksen enimmäismäärä (esimerkiksi `VALUKU`, `N_PV`) ei ole kilpailun oletusasetus; kortti
    sanoo kumpi.

## 3. Luvut

| Luku | Kysymys, johon vastataan |
|---|---|
| 1 Lyhyesti | Mitä tapahtuu ja kuinka pahaa se on, vahvistettu ja mahdollinen erikseen |
| 2 Mitä ohjelma yrittää tehdä | Mikä toiminto on käynnissä |
| 3 Mikä menee vikaan | Mitä ohjelma tekee väärin |
| 4 Missä tilanteessa vika voi ilmetä | Laukaisuehdot: ohjelma, asetus, laite, toimenpide; normaalikäyttö vai poikkeustilanne |
| 5 Mitä käyttäjä voi huomata | Näkyvät oireet; voiko vika jäädä kokonaan huomaamatta |
| 6 Vaikutukset kilpailuun | Generoitu taulukko vaikutusrekisteristä (luku 4) |
| 7 Mitä kilpailun aikana voi tapahtua | Käytännön kuvaus; vahvistettu ja mahdollinen merkitään |
| 8 Palautuminen ja uudelleenkäynnistys | Voiko palautua kilpailun aikana; auttaako uudelleenkäynnistys; muut työasemat |
| 9 Mitä käyttäjä voi tehdä | Kirjatut keinot riskin pienentämiseksi |
| 10 Tekninen juurisyy | Kehittäjälle: funktio, mekanismi, juurisyy JS tai "yksittäinen vika" |
| 11 Mitä on todennettu | (a) ohjelmointivirhe, (b) käytännön seuraus |
| 12 Mitä ei vielä tiedetä | Avoimet kysymykset Q ja todentamaton osa |
| 13 Miten vika on tarkoitus korjata | Korjausperiaate ja päätöstä odottavat osat OP |
| 14 Tekniset tiedot ja lähteet | Generoitu |

Kumotun havainnon kortissa (`_malli-H-kumottu.md`) ja teknisen velan kortissa (`_malli-T.md`) on
omat lukunsa.

Kortin pituutta ei rajata: selkokielinen osuus saa olla pitkä, jos vian ymmärtäminen sitä vaatii.
Lyhyet virkkeet, yksi asia virkkeessä.

## 4. Vaikutusrivit (`rekisteri/vaikutukset.csv`)

Käytännön vaikutukset kirjataan rekisteriin, ei korttiin: kortin luku 6 generoidaan niistä, ja
tuleva portaali suodattaa niiden mukaan. Ulottuvuudet ja niiden kysymykset ovat tiedostossa
`rekisteri/vaikutusulottuvuudet.csv` (16 kpl). Yksi rivi on yksi havainto ja yksi ulottuvuus:

`id,ulottuvuus,arvo,peruste,lahde`

| Arvo | Milloin |
|---|---|
| `vahvistettu` | Juuri tämä seuraus on osoitettu kokeella, testikehyksessä tai ohjelmaa ajamalla. Koodin lukeminen ei riitä. |
| `mahdollinen` | Seuraus johtuu koodista tai on sen suora jatke, mutta sitä ei ole osoitettu; tai seuraus riippuu olosuhteista, jotka peruste nimeää. |
| `ei_perusteltu` | Aineisto osoittaa, ettei seurausta synny (peruste kertoo miksi). |
| `ei_tiedossa` | Aineisto ei ratkaise kysymystä. |
| `ei_sovellu` | Kysymys ei koske havaintoa (esimerkiksi laitekysymys vian, joka ei liity laitteisiin). |
| rivi puuttuu | Ei vielä arvioitu. Portaali ja kortti näyttävät "Ei arvioitu". |

Ilmenemis- ja hallintaulottuvuuksissa (normaalikäyttö, poikkeustilanne, havaittavuus, kiertotapa,
palautuminen) `vahvistettu` tarkoittaa, että ominaisuus on osoitettu, ja `mahdollinen`, että se
seuraa koodista. Esimerkiksi kiertotapa `mahdollinen` = aineistossa on kirjattu keino, jonka
tehoa ei ole kokeiltu.

Tarkennukset (pilotin kysymykset):

- **Kiertotapa ja palautuminen:** jos aineistossa ei ole kirjattua keinoa, arvo on
  `ei_tiedossa` (peruste "aineistossa ei ole kirjattua keinoa"). `ei_perusteltu` vain, jos aineisto
  osoittaa, ettei keinoa voi olla. Omistajan päätöksen vaihtoehto ei ole kirjattu keino.
- **Normaalikäyttö ja poikkeustilanne** arvioidaan erikseen, eikä `ei_sovellu` ole niissä
  sallittu. Poikkeustilanne tarkoittaa virhettä, katkosta, virheellistä syötettä tai harvinaista
  asetusta. Jos vika laukeaa tavallisessa käytössä ilman mitään näistä, poikkeustilanne on
  `ei_perusteltu` perusteella "laukeaa ilman poikkeustilannetta".
- **Riskiarvion palautettavuus** (`itsestaan`, `uudelleenkaynnistys`, `kasin`) on kirjattu
  palautumiskeino: arvo `mahdollinen`, ellei keinoa ole kokeiltu (`vahvistettu`). `muistiturvallisuus.md`
  luvun 15 etukäteen tehtävä varmuuskopio on kirjattu palautumiskeino, vaikka riskiarvio sanoisi
  `ei_palautettavissa` (se kuvaa ohjelman omaa kykyä palautua).
- **Luvun 15 suositukset** mainitaan jokaisessa kortissa, jonka laukaisu- tai oiretilanteeseen ne
  sopivat, myös kun suositus on kirjoitettu toiselle havainnolle; silloin sanotaan "suositus on
  kirjattu havainnolle H-NNN; sen tehoa tähän vikaan ei ole arvioitu".
- **`vahvistettu` ilmenemis- ja hallintaulottuvuuksissa** vain, kun juuri kysytty ominaisuus on
  osoitettu. Koe, joka osoittaa mekanismin, ei osoita kaatumista, ilmoituksen näkymistä tai
  palautumista, ellei niitä havaittu kokeessa.
- Jos riskiarvion sarake estää arvon, jonka kirjoittaja pitää oikeana, rivi saa arvon
  `ei_tiedossa`, ja ristiriita kirjataan avoimeksi kysymykseksi riskiarvion tarkistamiseksi.

`peruste` on lyhyt suomenkielinen lause. `lahde` on rekisteritunnus, asiakirja ja luku tai
`tiedosto:rivi`; se on pakollinen arvoille `vahvistettu`, `mahdollinen` ja `ei_perusteltu`.

`synteesi.py --tarkista` tarkistaa sallitut arvot ja ristiriidat riskiarvioon: jos
`riskiarvio.csv` sanoo tuloksista, ajanotosta tai tietoturvasta "ei", vaikutusrivi ei voi olla
`vahvistettu` tai `mahdollinen`; jos riskiarvio sanoo vaikutuksen olevan, rivi ei voi olla
`ei_perusteltu`. Ristiriita korjataan rekisteriin, ei korttiin.

## 5. Työnkulku

1. `python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . runko H-NNN`
2. Lue rekisteririvi, riskiarvio, korjauskokonaisuus, erän raportin kohta ja tarvittaessa
   viitatut koodirivit. Uutta vikaa ei etsitä; jos sellainen huomataan, se kirjataan ehdokkaaksi
   `seuranta.md`:hen eikä korttiin.
3. Kirjoita vaikutusrivit `vaikutukset.csv`:hen ja kortin luvut.
4. Jos ohjelmat tarkistettiin, päivitä `riskiarvio.csv`:n `ohjelmat` ja `ohjelmat_peruste =
   tarkistettu`.
5. `tietopohja.py . lohkot H-NNN`, sitten `tietopohja.py . kuittaa H-NNN`.
6. `tietopohja.py . tarkista` ja `synteesi.py . <tmp> --tarkista`.

Kun rekisteririvi myöhemmin muuttuu, `tarkista` ilmoittaa kortin vanhentuneeksi. Kortti luetaan
uudelleen muuttunutta riviä vasten, korjataan tarvittaessa ja kuitataan.
