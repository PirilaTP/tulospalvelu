# Omistajan päätökset / Owner decisions (C-13)

Laadittu 2026-11-26 erässä C-13. Tähän on koottu kaikki asiat, jotka auditoinnin aineiston mukaan
vaativat ohjelmiston omistajan tai ylläpitäjän päätöksen tai tiedon. **Yhtäkään päätöstä ei ole
tehty tässä asiakirjassa.** Suositukset ovat ehdotuksia ja on merkitty [ehdotus]; ne perustuvat
vain siihen, mitä auditoinnissa on nähty. Päätöksen puuttuminen ei estä vaiheen C sulkemista:
jokaisen kohdan vaikutus on kirjattu, ja päätöstä odottavat kokonaisuudet on merkitty
(`rekisteri/korjauskokonaisuudet.csv`, sarake `omistajan_paatokset`).

Written on 2026-11-26 in batch C-13. Everything that needs a decision or information from the owner
or maintainer of the software is collected here. **No decision is made in this document.**
Recommendations are proposals based only on the audit evidence. Missing decisions do not block
closing phase C: the effect of each is recorded and the dependent packages are marked.

**Tila 2026-11-26 (vaiheen C sulkeminen):** kaikki 18 päätöskohtaa ovat avoinna; yhtäkään ei ole
merkitty tehdyksi. OP-1, OP-2 ja OP-3 käsitellään ennen D-vaiheen aloittamista. Käyttäjän ohje
samalta päivältä on voimassa oleva rajoitus eikä OP-1:n ratkaisu: auditointihaaraa ei pushata
julkiseen repositorioon ilman erillistä hyväksyntää, ja aineistosta tarvitaan yksityinen
varmuuskopio (`korjaussuunnitelma.md` luku 9.3). / Status at the closing of phase C: all 18 items
are open; OP-1..OP-3 are handled before phase D starts. The user's instruction not to push the
audit branch to a public repository without separate approval is a standing constraint, not the
resolution of OP-1.

Kentät: **Asia** (mitä päätetään), **Miksi** (miksi päätös tarvitaan), **Vaihtoehdot ja
vaikutukset**, **Suositus**, **Riippuvat kokonaisuudet**, **Viimeistään** (missä kehittämisvaiheessa
päätös tarvitaan; vaiheet ja aallot: `korjaussuunnitelma.md` luvut 4 ja 6).

## Yhteenveto / Overview

| Tunnus / Id | Asia / Subject | Aiemmat tunnukset / Earlier ids | Viimeistään / Needed by |
|---|---|---|---|
| OP-1 | Auditointitulosten julkaisu, push ja suojausohjeet / publication of findings, push, mitigations | – | ennen D-vaiheen alkua |
| OP-2 | Testi- ja CI-käytäntö / test and CI policy | `testaus.md` 12.1 (kohdat 4, 6, 7), Q35, Q40 | aalto 0 |
| OP-3 | C++-standardi tuotantokoodissa / C++ standard in production code | Q9 | aalto 1 (oletus riittää) |
| OP-4 | Yhteensopivuuspolitiikka: sekaversiot, tiedosto- ja protokollamuutokset / compatibility policy | H-016, H-029, T-019, T-043 | aalto 2 (KK-C13-24); vaihe 4 |
| OP-5 | FILESEND-tiedostonsiirron rajaus / restricting FILESEND file transfer | H-071 | aalto 1, KK-C13-1 PR 4 |
| OP-6 | Vaiheen vaihdon tuettu käyttötapa ja nimeäminen / supported stage-change workflow | Q67, Q78, Q80; H-131–H-133 | aalto 1 sarja c; aalto 2 |
| OP-7 | MySQL-peilin asema, tuonti ja jakelu / status of the MySQL mirror | H-165, H-169, KK-C10-1 | aalto 2 |
| OP-8 | Kolmannen osapuolen lähdekoodin ehdot / third-party source terms | OM-C10-1, Q101, T-047 | ennen seuraavaa julkista julkaisua |
| OP-9 | Ominaisuuksien ja käytöksen tarkoituksellisuus / intended behaviour | Q60, Q75, Q111 (osat 1–2); H-158, H-172; H-116, H-109, H-111 | aalto 2 (kohdat 6–10); aallot 3–4 |
| OP-10 | Kuolleen koodin, orpojen tiedostojen ja valitsinten poisto / removal of dead code | Q111 (osa 3), Q10, Q11 | aalto 4 |
| OP-11 | Asetusavainten kanoninen muoto ja yksiköt / canonical keys and units | Q112, Q25; H-173, H-176 | aalto 3 (KK-C12-2 sarja b) |
| OP-12 | Tuetut tuotteet, kokoonpanot ja käyttöjärjestelmät / supported products and configurations | T-051, T-024, T-017, T-052 | vaihe 2 |
| OP-13 | Laitteet ja ulkoiset palvelut / devices and external services | Q86, Q99, Q100, Q44, Q56 | aalto 1 (testit); aalto 3 |
| OP-14 | Kentän käyttötavat (tietopyyntö) / field practice (information request) | Q29, Q47, Q81, Q90 | ei estä; tarkentaa järjestystä |
| OP-15 | Merkistökäytäntö / encoding policy | H-004, H-005, T-027 | aalto 4 |
| OP-16 | HK- ja viestihaarojen lähentämisen tavoite / goal of fork convergence | Q77, T-034 | vaihe 4 |
| OP-17 | Modernisoinnin selvityskohteet / modernisation topics to study | T-046, `riippuvuudet.md` 15 | vaihe 5 |
| OP-18 | Korjausjulkaisujen käytäntö / release practice for fixes | T-025, T-044, Q98 | ennen ensimmäistä D-julkaisua |

## OP-1 Auditointitulosten julkaisu, push ja suojausohjeet

- **Asia.** Milloin ja miten auditointidokumentit, testikoodi ja korjaukset viedään julkiseen
  repositorioon, ja julkaistaanko käyttäjille väliaikaiset suojausohjeet ennen korjauksia.
- **Miksi.** Dokumentit kuvaavat verkosta oletusasetuksin laukeavia muistivikoja (H-071, H-072,
  H-032) riittävän tarkasti niiden hyödyntämiseksi. Repositorio on julkinen GitHubissa, ja push
  sekä PR:t on toistaiseksi estetty käyttäjän päätöksellä. Ilman pushia CI-muutoksia ei voi
  todentaa eikä PR-työnkulkua käyttää.
- **Vaihtoehdot ja vaikutukset.**
  (a) Kaikki julkiseksi heti: avoin, mutta viat ovat tiedossa ennen korjausta.
  (b) Korjaukset ensin yksityisessä haarassa tai haarukassa, dokumentit julki korjausjulkaisun
  yhteydessä: viive, mutta ei ikkunaa, jossa viat ovat julkisia ja korjaamatta.
  (c) Dokumentit julki ilman P1- ja P2-havaintojen yksityiskohtia: vaatii rekisterin jakamisen.
  Suojausohjeet (`muistiturvallisuus.md` luku 15: erillinen ajanottoverkko, palomuurin sallittujen
  lista, vähimmät oikeudet) voi julkaista käyttöohjeessa kertomatta vikojen yksityiskohtia.
- **Suositus [ehdotus].** (b), ja suojausohjeet käyttöohjeeseen heti. Ohjelmistoa käytetään
  tyypillisesti suljetussa kilpailuverkossa, mikä pienentää mutta ei poista riskiä.
- **Riippuvat kokonaisuudet.** KK-C13-18 sarja b; kaikkien korjausten PR-työnkulku.
- **Viimeistään.** Ennen D-vaiheen ensimmäistä PR:ää.

## OP-2 Testi- ja CI-käytäntö

- **Asia.** (1) Miten tunnettu, korjaamaton vika näkyy testiajossa ja miten korjattu vika muuttuu
  pysyväksi regressiotestiksi. (2) Korjataanko vai poistetaanko `Tests/run.sh` (H-006). (3) Saako
  tuotantokoodiin lisätä testikoukkuja. (4) Ajetaanko testit CI:ssä ja käytetäänkö itseisännöityä
  ajokonetta myös testeihin.
- **Miksi.** `testaus.md` 12.1:n päätökset 4, 6 ja 7 ovat auki. Ajuri palauttaa nyt virhekoodin,
  kun vika on korjattu, joten ensimmäinen korjaus rikkoisi testiajon.
- **Vaihtoehdot ja vaikutukset.** (1) Testin laji vaihdetaan samassa PR:ssä kuin korjaus
  (yksinkertainen, historia näkyy manifestissa) tai ajuri saa regressiotilan. (2) `run.sh`:n
  korjaus antaisi g++-käännöksen puhtaille yksiköille, mutta tuotantoyksiköiden kääntyminen
  g++:lla on tuntematon; poisto on pienin työ. (3) Ilman koukkuja osa vioista (H-009) jää käsin
  testattavaksi. (4) CI:n testit vaativat OP-1:n.
- **Suositus [ehdotus].** (1) lajin vaihto korjaus-PR:ssä; (2) poisto, kunnes Linux-käännökselle on
  tarve; (3) ei koukkuja vaiheessa 1; (4) kyllä, vaiheessa 2.
- **Riippuvat kokonaisuudet.** KK-C13-18; välillisesti kaikki.
- **Viimeistään.** Aalto 0.

## OP-3 C++-standardi tuotantokoodissa

- **Asia.** Sallitaanko C++11 tai uudempi tuotantokoodissa (Q9).
- **Miksi.** Projektien kielitaso on C++03; yksikkötestit vaativat C++11:n. Lukkovartijat ja
  omistajuuden ilmaiseminen ovat helpompia uudemmalla standardilla, mutta klassisen bcc32:n tuki
  on rajallinen.
- **Vaihtoehdot ja vaikutukset.** Pysyä C++03:ssa (toimii varmasti molemmilla kääntäjillä; apuluokat
  kirjoitetaan itse) tai sallia osajoukko, jonka molemmat kääntäjät tukevat (vaatii selvityksen).
- **Suositus [ehdotus].** Rajatut korjaukset C++03-yhteensopivina; kysymys ratkaistaan vaiheen 3
  alussa yhdessä OP-17:n kanssa.
- **Riippuvat kokonaisuudet.** KK-C13-4 (vartijan toteutustapa), KK-C13-19.
- **Viimeistään.** Ei estä aaltoa 1; tarvitaan vaiheessa 3.

## OP-4 Yhteensopivuuspolitiikka

- **Asia.** (1) Pitääkö eri ohjelmaversioiden toimia samassa verkossa, vai päivitetäänkö kaikki
  työasemat aina yhtä aikaa. (2) Saako tiedostojen ja sanomien muotoa muuttaa (versio,
  tarkistussumma, todennus) ja millä ehdoilla. (3) Saako UDP-vastapuolen osoitteen vaihtumista
  rajata (H-029). (4) Mitä vastaanottaja tekee ristiriitaiselle päivitykselle: hylkää vai yhdistää,
  ja mihin vaiheeseen pelkkä tulospäivitys kuuluu soveltaa (H-022, H-023).
- **Miksi.** Useimmat korjaukset eivät muuta muotoja. Poikkeukset: H-025:n korjaus muuttaa viestin
  `COMFILE.DAT`:n tietuepituuden (vanha jono ei kelpaa uudelle versiolle; KK-C13-24); replikoinnin
  rakenteelliset viat (H-016, H-030, H-039) ja tiedostojen tunnisteen puute (T-019) vaativat uusia
  kenttiä (KK-C13-23); todennus muuttaisi protokollaa (KK-C10-4). H-029:n korjaus voi estää
  käyttötavan, jossa vastapuolen osoite vaihtuu.
- **Vaihtoehdot ja vaikutukset.**
  (a) Tiukka yhteensopivuus: ei muotomuutoksia; H-025 korjataan vain tavalla, joka säilyttää
  jonotiedoston, tai jätetään dokumentoiduksi rajoitukseksi; rakenteelliset viat jäävät.
  (b) Hallittu muutos: muoto saa muuttua julkaisujen välissä, kun vanhat tiedostot luetaan, muutos
  tunnistetaan ja päivitysohje annetaan; sekaversioita ei tueta muutoksen yli.
  (c) Versioneuvottelu: ohjelmat tunnistavat toistensa version; suurin työ, paras pitkällä
  aikavälillä.
- **Suositus [ehdotus].** (b) aallolle 2 (KK-C13-24: päivitys kilpailujen välissä, FILESEND-luokan
  pituus jäädytetty); (c):stä päätetään vasta vaiheen 4 suunnittelussa. H-029: rajaus asetuksen
  taakse, oletuksena nykyinen käytös, kunnes käyttötapa on selvitetty.
- **Riippuvat kokonaisuudet.** KK-C13-24, KK-C10-4, KK-C13-23; KK-C13-10 (vain hylkäyksen
  näkyvyys).
- **Viimeistään.** Aalto 2; laajemmin vaihe 4.

## OP-5 FILESEND-tiedostonsiirron rajaus

- **Asia.** Saako vastaanottava ohjelma rajata verkosta vastaanotetun tiedoston kohdehakemiston.
- **Miksi.** Tiedostonsiirto työasemien välillä on dokumentoitu ominaisuus (`LÄHTIEDOSTOT`,
  käyttöohjeen liitteet). Vastaanottaja ei tarkista mitään: nimetty tiedosto avataan
  kirjoitettavaksi, myös koko polulla (`/K`) tai ylähakemistossa. H-071:n pituustarkistus ei riko
  ominaisuutta, mutta polun rajaus rikkoisi `/K`:n.
- **Vaihtoehdot ja vaikutukset.** (a) Ei rajausta: mikä tahansa verkon kone voi korvata tiedostoja
  ohjelman oikeuksin. (b) Oletuksena vain työhakemisto; koko polku sallitaan vastaanottajan
  asetuksella: turvallinen oletus, nykyiset `/K`-käyttäjät lisäävät asetuksen. (c) Vastaanotto
  kokonaan asetuksen taakse.
- **Suositus [ehdotus].** (b).
- **Riippuvat kokonaisuudet.** KK-C13-1 PR 4. PR:t 1–3 eivät riipu tästä.
- **Viimeistään.** Aalto 1.

## OP-6 Vaiheen vaihdon tuettu käyttötapa ja nimeäminen

- **Asia.** (1) Onko vaiheen vaihto käynnissä olevassa prosessissa tuettu käyttötapa, vai
  ohjeistetaanko käynnistämään ohjelma uudelleen (Q78). (2) Onko aikajonotiedoston nimen viidennen
  merkin poisto yksivaiheisessa kilpailussa tarkoitettu nimeämiskäytäntö (H-132). (3) Tarkoittaako
  `vaiheenvaihto`-lippu säikeille taukoa vai poistumista ja uudelleenkäynnistystä.
- **Miksi.** Vaiheen vaihdon polussa on 12 havaintoa (KK-C13-3) ja säikeiden osalta lisää
  (KK-C13-8, H-014, H-097). Jos uudelleenkäynnistys on suositeltu tapa, osa korjauksista on
  vähemmän kiireellisiä ja `aloitus`in uudelleenajettavuus (H-133) voi jäädä rajoitukseksi.
- **Vaihtoehdot ja vaikutukset.** (a) Vaihto käynnissä olevassa prosessissa tuetaan täysin: kaikki
  sarjat korjataan, laajin testaus. (b) Tuetaan, mutta saman prosessin uusintayritystä epäonnistuneen
  käynnistyksen jälkeen ei: sarja c kevenee. (c) Uudelleenkäynnistys ohjeeksi: vain kekoylivuoto ja
  jumiutumiset korjataan.
- **Suositus [ehdotus].** Kekoylivuoto (H-129) ja siihen liittyvät korjataan joka tapauksessa
  (aalto 1). Lipun merkitykseksi tauko, jos omistajalla ei ole syytä muuhun; nimeämistä ei muuteta
  ilman tietoa käytännöstä.
- **Riippuvat kokonaisuudet.** KK-C13-3 sarja c, KK-C13-8 PR 2, KK-C10-1 (H-014), KK-C13-20.
- **Viimeistään.** Aalto 2.

## OP-7 MySQL-peilin asema, tuonti ja jakelu

- **Asia.** (1) Tuetaanko MySQL-peilausta jatkossa. (2) Hyväksytäänkö periaate, ettei valinnaisen
  peilin virhe saa lopettaa ohjelmaa eikä pysäyttää tallennusta. (3) Mitä SQL-tuonnin (`lue_SQL`)
  kuuluu tehdä tietueille, joita kannassa ei ole (H-169). (4) Jaetaanko `dbxmys.dll` ja
  `libmysql.dll` julkaisupaketissa (H-165; lisenssi ja jakeluoikeus). (5) Mikä on tarkoitettu
  SQL-skeema niissä kohdissa, joissa lauseet ja taulut eroavat (H-161).
- **Miksi.** `KILP.DAT` on ensisijainen tietolähde ja MySQL valinnainen peili vain GUI-ohjelmissa.
  Nykyisin SQL-virhe lopettaa GUI-ohjelman (H-160), tuonti voi korvata paikalliset tietueet
  poistetuilla (H-169), eikä julkaisupaketti sisällä ominaisuuden tiedostoja.
- **Vaihtoehdot ja vaikutukset.** Tuki jatkuu (korjataan KK-C10-1, jakelu selvitetään), tuki
  jäädytetään (vain kaatumisen esto H-160, H-162, H-163; muu dokumentoidaan rajoitukseksi) tai
  ominaisuus poistetaan käytöstä (pienin ylläpito, mutta nykyiset käyttäjät menettävät sen).
- **Suositus [ehdotus].** Periaate (2) hyväksytään ja H-160, H-162, H-163 korjataan
  joka tapauksessa; muu sen mukaan, onko ominaisuudella käyttäjiä.
- **Riippuvat kokonaisuudet.** KK-C10-1 (PR 4–5), KK-C10-3.
- **Viimeistään.** Aalto 2.

## OP-8 Kolmannen osapuolen lähdekoodin ehdot

- **Asia.** Miten käsitellään Blaise Computing Inc:n (16 tiedostoa, "Version 6.00 (C) 1983–1989")
  ja Microsoftin esimerkkikoodin (3 tiedostoa, 1993–1997) tiedostot, joissa ei ole lisenssitekstiä
  eikä GPL-otsikkoa (OM-C10-1, Q101, T-047).
- **Miksi.** Projekti on julkaistu GPLv3-lisenssillä. Aineistosta ei selviä, oliko alkuperäisellä
  tekijällä oikeus jakaa tiedostot edelleen, kattaako GPLv3-julkaisu ne ja koskeeko asia julkaistuja
  binäärejä. Auditointi ei tee oikeudellisia johtopäätöksiä (`riippuvuudet.md` 19.5).
- **Vaihtoehdot ja vaikutukset.** Selvityspyyntö oikeudenhaltijoilta; tiedostojen korvaaminen
  omalla toteutuksella (11 Blaise-tiedostoa käännetään kirjastoihin; mitkä linkittyvät ohjelmiin,
  on selvittämättä); merkintä README:hen; oikeudellinen arvio.
- **Suositus.** Ei suositusta: asia on oikeudellinen.
- **Riippuvat kokonaisuudet.** KK-C10-3.
- **Viimeistään.** Ennen seuraavaa julkista julkaisua, jos jakelua muutetaan.

## OP-9 Ominaisuuksien tarkoituksellisuus

- **Asia.** Onko seuraava käytös tarkoitettua:
  (1) kuntosuunnistustilan emit-koodin vaihto vain GUI-ohjelmassa (`KUNTOSUUNN`, H-081, Q60);
  (2) `kuntosuunnmoodi`n arvojen 1, 2, 3, 98, 99 merkitys (Q75; H-119, H-120);
  (3) HkMaalin konsolin seuratulosten ja `ORAVA`-koodin poiskääntäminen (H-177, Q111);
  (4) ViestiLuennan supistus ilman RFID-tukea (T-052, Q111);
  (5) viestin XML-tulosteen sisältö konsolissa ja automaattitulosteessa (H-158, H-172);
  (6) mitä valinta- tai virheikkunan sulkeminen X:llä tai Alt+F4:llä tarkoittaa; nyt se on sama
  kuin painike 2 eli emit-lomakkeissa tuhoava valinta ja virheikkunoissa "Poistu ohjelmasta"
  (H-116);
  (7) saako työsäikeen virheilmoitus ylipäätään tarjota ohjelman lopetusta (H-153, H-009);
  (8) saako tuloksen saaneen kilpailijan poistaa ja mitä poiston pitää estää (H-109);
  (9) hylätäänkö ajanoton virheellinen piste ilmoituksella vai rajataanko se, ja miten A- ja
  L-rivejä kuuluu käsitellä (H-111, H-117);
  (10) lasketaanko `nvak`iin tila P, ja kuuluuko sarja nimihakemiston avaimeen (H-139, H-142).
- **Miksi.** Koodista näkyy ero tai puute, mutta ei se, onko se tahallinen. Korjaus väärään suuntaan
  muuttaisi toimivaa käytöstä. Kohdissa 6–10 vika on selvä, mutta oikea käytös on valinta.
- **Vaihtoehdot ja vaikutukset.** Kullekin: tarkoitettu (dokumentoidaan, havainto suljetaan) tai
  vika (korjataan kokonaisuudessaan).
- **Suositus.** Kohdat 1–5: ei suositusta; vaatii omistajan tai käyttäjien tiedon. Kohdat 6–10
  [ehdotus]: oletuskorjaus on vaarattomin vaihtoehto (X = ei toimintoa tai OK, ei lopetusta
  työsäikeen ilmoituksesta, vain virheellisen valinnan hylkäys, virheellisen pisteen hylkäys
  ilmoituksella), ja se tehdään päätöstä odottamatta; päätös voi tarkentaa käytöstä myöhemmin.
- **Riippuvat kokonaisuudet.** KK-C13-17 (1), KK-C13-15 sarja c (2), KK-C12-3 (3, 4), KK-C12-1 (5),
  KK-C13-14 (6, 8, 9), KK-C13-5 (7), KK-C13-16 (10).
- **Viimeistään.** Kohdat 6–9 aalto 2; muut aallot 3–4.

## OP-10 Kuolleen koodin, orpojen tiedostojen ja valitsinten poisto

- **Asia.** Saako poistaa koskaan määrittelemättömien valitsinten haarat (noin 7 400 riviä,
  T-050), kuolleen riippuvuuskoodin (T-049), vanhat kopiot (T-011) ja 32 orpoa tiedostoa (T-007);
  ovatko orvot käytössä muualla (Q10, Q11).
- **Miksi.** Poisto pienentää ylläpidettävää koodia ja varoitusten määrää, mutta historiallinen
  koodi voi olla tarpeen (muut haarat, tulevat lajit kuten `MAKI`, `AMPUSUUNN`).
- **Vaihtoehdot ja vaikutukset.** Ei poisteta (ei riskiä, velka säilyy); poistetaan valitsin
  kerrallaan objektivertailulla todentaen (historia säilyy Gitissä); siirretään arkistohakemistoon.
- **Suositus [ehdotus].** Poisto valitsin kerrallaan vasta vaiheessa 3; lajivalitsimet (`MAKI`,
  `AMPUSUUNN`) vain omistajan nimenomaisella luvalla.
- **Riippuvat kokonaisuudet.** KK-C12-3.
- **Viimeistään.** Aalto 4.

## OP-11 Asetusavainten kanoninen muoto ja yksiköt

- **Asia.** (1) `AIKAKORJAUSn`: HK kertoo arvon `SEK / 100`, viesti `SEK`; kumpi yksikkö on
  tarkoitettu (Q112). (2) Editorit kirjoittavat avaimia, joita jäsennin ei tunne (`AIKACOM=` ym.,
  H-173): kumpi kirjoitusasu on oikea. (3) `UDPVIIVEET=` hukkaa ensimmäisen arvon (H-176).
  (4) MONITORI-portti: koodissa 14770, käyttöohjeessa 14900 (T-014, Q25).
- **Miksi.** Korjaus muuttaa nykyisten asetustiedostojen tulkintaa. Käyttöohjeet, esimerkit ja
  Git-historia eivät kerro yksikköä.
- **Vaihtoehdot ja vaikutukset.** Koodi on ohje (dokumentaatio korjataan), dokumentaatio on ohje
  (koodi korjataan ja vanha muoto hyväksytään edelleen) tai molemmat muodot hyväksytään.
- **Suositus [ehdotus].** Molemmat kirjoitusasut hyväksytään jäsentimessä; yksiköstä ei
  suositusta ilman käyttäjätietoa.
- **Riippuvat kokonaisuudet.** KK-C12-2 sarja b.
- **Viimeistään.** Aalto 3; ei estä H-171:n korjausta aallossa 1.

## OP-12 Tuetut tuotteet, kokoonpanot ja käyttöjärjestelmät

- **Asia.** Mitkä ohjelmat ja käännöskokoonpanot ovat tuettuja: viisi toimitettavaa ohjelmaa,
  toimittamattomat kokoonpanot (`VALUKU=200`, `MAXOSUUSLUKU=250`; T-051, T-017), viestin
  `NREGNLY` 8 konsolissa ja 32 ViestiWinissä (T-024), ViestiLuenta, sekä tuetut Windows-versiot.
  Lisäksi: suurennetaanko puskureita vai hylätäänkö rajan ylittävä kokoonpano (H-041, H-042);
  oletuskorjaus suurentaa puskurin eikä muuta nykyistä toimintaa.
- **Miksi.** Testimatriisin laajuus ja korjausten kohdistus riippuvat tästä. Toimittamattomilla
  kokoonpanoilla on eri tietue- ja sanomakoot.
- **Vaihtoehdot ja vaikutukset.** Vain toimitettavat kokoonpanot tuetaan (pienin matriisi; muut
  poistetaan projekteista tai merkitään tukemattomiksi) tai myös nimetyt muut (matriisi kasvaa).
- **Suositus [ehdotus].** Vain julkaisun tuottamat kokoonpanot, kunnes muille ilmoitetaan tarve.
- **Riippuvat kokonaisuudet.** KK-C12-2, KK-C13-18 (matriisin laajuus), KK-C10-3.
- **Viimeistään.** Vaihe 2.

## OP-13 Laitteet ja ulkoiset palvelut

- **Asia.** (1) Mitkä laitteet ovat käytettävissä testaukseen (Emit, emiTag, Sirit/Zebra, FEIG,
  ARES, ampumalaite, C-Power). (2) Lukijan kuittauksen merkitys: pitääkö lukija kuittausta
  lopullisena, ja saako saman kortin toisen luennan hyväksyä (Q86, Q100; H-145, H-168).
  (3) emiTag-palvelimen `195.159.103.189:1379` omistaja, vastausmuoto ja HTTPS-tuki (Q99; H-166,
  T-043). (4) Laiteprotokollien dokumentaatio (Q44, Q56).
- **Miksi.** Laiteyhteyksien ja kuittausjärjestyksen korjauksia ei voi todentaa ilman laitteita tai
  tallennettua liikennettä, ja oikea käytös riippuu laitteen toiminnasta.
- **Vaihtoehdot ja vaikutukset.** Laitteet lainaan testijaksoksi; tallennettu liikenne
  simulaattorille; korjaus vain simuloituna (riski jää julkaisuun).
- **Suositus [ehdotus].** Tallennettu liikenne jokaisesta käytössä olevasta laitetyypistä
  mahdollisimman pian; kuittausjärjestyksen muutos (H-145) vasta laitetestin jälkeen.
- **Riippuvat kokonaisuudet.** KK-C13-2, KK-C10-2, KK-C9-2, KK-C10-4.
- **Viimeistään.** Aalto 1 (testit); aalto 3 (H-145).

## OP-14 Kentän käyttötavat (tietopyyntö)

- **Asia.** Tietoja siitä, miten ohjelmia käytetään kilpailuissa: avataanko viestin vientitiedosto
  `KILP.DAT`:na (Q29, H-046); käytetäänkö käsin muokattuja `RADAT.LST`-tiedostoja tai yli 50
  rastin ratoja (Q47); useita lukijoita yhdellä HK-instanssilla (Q81); ajetaanko ilman levypuskuria
  (Q90); muokataanko lähtöportin L-rivejä ajanoton paneelissa (H-111); XML-TCP-yhteyksien yleisyys
  (H-030); ampumahiihtoviestien yleisyys (H-001).
- **Miksi.** Vastaukset voivat nostaa tai laskea yksittäisten havaintojen prioriteettia
  (`rekisteri/riskiarvio.csv`, sarake `huomio`), mutta eivät muuta ensimmäistä toteutuserää.
- **Riippuvat kokonaisuudet.** KK-C13-11, KK-C13-12, KK-C13-14, KK-C13-6, KK-C13-23.
- **Viimeistään.** Ei estä; hyödyllinen ennen aaltoa 2.

## OP-15 Merkistökäytäntö

- **Asia.** Miten korjataan lähdetiedostojen sekalaiset merkistöt: lisätäänkö BOM, otetaanko
  `/utf-8` käyttöön tai koodataanko tiedostoja uudelleen (H-004, H-005, T-004, T-027).
- **Miksi.** Molemmat kääntäjät lukevat BOMittoman UTF-8:n väärin, joten osa teksteistä näkyy
  väärin. Projektin muokkaussääntö kieltää merkistön muuttamisen ilman päätöstä, ja kevään 2026
  kommentointikierros rikkoi käyttöliittymän tekstejä.
- **Vaihtoehdot ja vaikutukset.** Ei muuteta (viat jäävät, ei riskiä); korjataan vain rikkinäiset
  merkkijonot tavutasolla (pieni, todennettavissa); yhtenäinen merkistö koko puulle
  (suurin hyöty ja suurin riski; vaatii objekti- ja merkkijonovertailun).
- **Suositus [ehdotus].** Rikkinäisten merkkijonojen korjaus tavutasolla; yhtenäistäminen
  erillisenä päätöksenä vaiheessa 3.
- **Riippuvat kokonaisuudet.** KK-C13-17.
- **Viimeistään.** Aalto 4.

## OP-16 HK- ja viestihaarojen lähentämisen tavoite

- **Asia.** Halutaanko HK- ja viestikoodia yhdistää, ja kumpi haara on ohjeellinen, kun korjaus on
  tehty vain toiseen (Q77, T-034).
- **Miksi.** Haarat ovat erkaantuneet kopioimalla sekä lomakkeissa että ytimessä. Git-historiassa
  on vain vuoden 2015 tuonti, joten haarojen ikää ei voi päätellä.
- **Vaihtoehdot ja vaikutukset.** Haarat pysyvät erillään ja jokainen korjaus tarkistetaan
  molemmista (nykyinen sääntö; kaksinkertainen työ jatkuu); yhteinen koodi siellä, missä käytös on
  sama (vaatii vertailun ja testit); täysi yhdistäminen (suurin riski).
- **Suositus [ehdotus].** Tarkistussääntö voimaan heti; yhdistämisestä päätetään vasta vaiheen 4
  vertailun jälkeen.
- **Riippuvat kokonaisuudet.** KK-C13-22; KK-C13-14, KK-C13-15 (tarkistussääntö).
- **Viimeistään.** Vaihe 4.

## OP-17 Modernisoinnin selvityskohteet

- **Asia.** Mitkä pitkän aikavälin vaihtoehdot selvitetään: GUI-kääntäjä (klassinen bcc32 vai
  C++Builderin Clang-kääntäjä, 64-bittisyys), kielistandardi, `TWebBrowser`in (IE-moottori)
  korvaaminen, tietokanta-ajuri (dbExpress vai FireDAC), IPv6, konsoliohjelmien asema.
- **Miksi.** Osa riippuvuuksista on vanhentuvia alustan osia (T-046). Tämä suunnitelma ei oleta
  yhtäkään vaihtoa, eikä mikään vaiheiden 1–4 työ edellytä niitä.
- **Vaihtoehdot ja vaikutukset.** `korjaussuunnitelma.md` 6.5.
- **Suositus.** Ei suositusta teknologiasta. Selvitykset kannattaa ajoittaa vaiheen 2 jälkeen, kun
  testit suojaavat muutoksia.
- **Riippuvat kokonaisuudet.** KK-C13-21; vaihe 5.
- **Viimeistään.** Vaihe 5.

## OP-18 Korjausjulkaisujen käytäntö

- **Asia.** Julkaistaanko välittömät korjaukset omana julkaisunaan vai kootaanko ne; miten versio
  numeroidaan; kuka hyväksyy julkaisun; miten julkaisupaketti testataan (Q98). Lisäksi: missä
  järjestyksessä avoimet ominaisuushaarat ja korjaukset yhdistetään. Paikallisten viitteiden mukaan
  ainakin neljä marraskuussa 2026 päivättyä haaraa muuttaa tuotantokoodia revision `f7fe553`
  jälkeen, osin samoissa tiedostoissa kuin korjattavat havainnot (`cbTp/cbTpApu.cpp`,
  `Hk/HkInit.cpp`, `Juk/VInit.cpp`, `cbHk/UnitExport.cpp`).
- **Miksi.** Versiotunniste `VersioInt` on MSVC:llä 521 ja bcc32:lla 520 (T-025), joten konsoli ja
  GUI ilmoittavat eri version. Julkaisupakettia ei nyt käynnistetä ennen julkaisua (T-044).
- **Vaihtoehdot ja vaikutukset.** Pienet, usein toistuvat korjausjulkaisut (nopea hyöty, enemmän
  testikierroksia) tai yksi koottu julkaisu aallon jälkeen (vähemmän kierroksia, pidempi odotus).
- **Suositus [ehdotus].** Julkaisu aallon 1 jälkeen ja toinen aallon 2 jälkeen, kumpikin
  `regressiotestaussuunnitelma.md`:n luvun 6 tarkistuslistalla; versiotunnisteen ero korjataan
  ennen kuin versiota käytetään yhteensopivuuden tunnistamiseen.
  Yhdistämisjärjestys [ehdotus]: joko ominaisuushaarat ensin ja rekisterin rivien uudelleentarkistus,
  tai korjaukset ensin ja haarojen sovitus; tärkeintä on päättää ennen ensimmäistä korjaus-PR:ää.
- **Riippuvat kokonaisuudet.** KK-C10-3, KK-C13-17 (T-025); ensimmäisen toteutuserän valmistelut.
- **Viimeistään.** Yhdistämisjärjestys ennen ensimmäistä korjaus-PR:ää; muu ennen ensimmäistä
  D-vaiheen julkaisua.

## Korjaustavan valintaa vaativat havainnot / Findings whose fix needs a choice

Riippumaton tarkastus (K8) ja juurisyyraportti (luku 7) nimesivät havaintoja, joissa vika on selvä
mutta oikea käytös on valinta. Taulukko kertoo oletuskorjauksen, joka ei muuta nykyistä toimintaa
enempää kuin vian poisto vaatii ja joka ei odota päätöstä [ehdotus], sekä päätöksen, joka voi
tarkentaa sitä.

| Havainnot / Findings | Oletuskorjaus ilman päätöstä / Default fix without a decision | Päätös / Decision |
|---|---|---|
| H-116 | ikkunan sulkeminen X:llä ei valitse painiketta 2 | OP-9 (6) |
| H-153, H-009 | työsäie, joka pitää lukkoa, ei käynnistä ohjelman lopetusta ilmoituksesta | OP-9 (7) |
| H-109 | poisto hylätään, kun tietuetta ei ole valittu | OP-9 (8) |
| H-111, H-117 | puuttuva `else` korjataan; virheellinen piste hylätään ilmoituksella | OP-9 (9) |
| H-139, H-142 | ei oletuskorjausta; odottaa päätöstä | OP-9 (10) |
| H-022, H-023 | ristiriitainen päivitys hylätään ja kirjataan lokiin | OP-4 (4) |
| H-041, H-042 | puskuri mitoitetaan suurimman sallitun kokoonpanon mukaan; määrät rajataan | OP-12 |
| H-161 | lauseen syntaksivirhe korjataan; skeemaa ei muuteta | OP-7 (5) |
| H-169 | ei oletuskorjausta; odottaa päätöstä | OP-7 (3) |
| H-071 | pituus tarkistetaan; polkua ei rajata | OP-5 |
| H-029 | ei oletuskorjausta; väliaikaiset suojaussuositukset voimassa | OP-4 (3) |
| H-025 | korjataan yhteensopivuusmenettelyllä (päivitys kilpailujen välissä) | OP-4 (1) |
| H-014, H-097, H-031 | H-031: lippu palautetaan; muut odottavat yhtä päätöstä | OP-6 (3) |
| H-132, H-133 | ei oletuskorjausta; odottaa päätöstä | OP-6 (1, 2) |
| H-145, H-168 | ei oletuskorjausta ennen laitetestiä | OP-13 (2) |
| H-046 | ei oletuskorjausta; odottaa tietoa käytöstä | OP-14 |
| H-081, H-119, H-120, H-175 | ei oletuskorjausta; odottaa päätöstä | OP-9 (1, 2) |
| H-158, H-172, H-177 | ei oletuskorjausta; odottaa päätöstä | OP-9 (3, 5), OP-10 |
| H-173, H-176 | jäsennin hyväksyy molemmat kirjoitusasut; yksikköä ei muuteta | OP-11 |
| H-004, H-005 | rikkinäiset merkkijonot korjataan tavutasolla | OP-15 |

## Avoimien kysymysten kytkentä / Link to the open questions

Auditoinnin 84 ratkaisemattomasta kysymyksestä 16 on luokiteltu omistajan päätöksiksi
(`auditoinnin-yhteenveto.md` luku 6). Ne sijoittuvat näin: Q9 → OP-3; Q10, Q11 → OP-10; Q29, Q47,
Q81, Q90 → OP-14; Q60, Q75, Q111 → OP-9 (Q111:n kolmas osa OP-10); Q77 → OP-16; Q78 → OP-6;
Q99 → OP-13; Q101 → OP-8; Q25, Q112 → OP-11.

/ Of the 84 unresolved audit questions, 16 are classified as owner decisions; the list above maps
each to an OP item.
