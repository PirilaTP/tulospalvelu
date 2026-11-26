# Regressiotestaussuunnitelma / Regression test plan (C-13)

Laadittu 2026-11-26 erässä C-13 (osa F). Tämä on suunnitelma: **C-13:ssa ei käännetty eikä ajettu
yhtään ohjelmaa tai testiä, eikä yhtäkään tässä kuvattua uutta testiä ole kirjoitettu.** Kaikki,
mikä on merkitty "ajettu", viittaa aiempien erien tallennettuihin tuloksiin. CI:tä ei ole muutettu.

Written on 2026-11-26 in batch C-13 (part F). This is a plan: **nothing was built or run in C-13 and
none of the new tests described here exists yet.** "Run" always refers to stored results of earlier
batches. CI has not been changed.

| Tieto / Data | Lähde / Source |
|---|---|
| Testausarkkitehtuuri ja C-11:n kehys / test architecture and the C-11 harness | `testaus.md` (ei toisteta tässä / not repeated here) |
| Testiaineiston inventaario rivitasolla / asset inventory, row level | `todisteet/c13/raportit/osaF-testausaineisto.md` |
| Korjauskokonaisuudet / fix packages | `korjaussuunnitelma.md`, `rekisteri/korjauskokonaisuudet.csv` |
| Konfiguraatiomatriisi T1–T18 / configuration matrix | `ominaisuusvalitsimet.md` luku 10 |

Huom. matriisin rivitunnukset T1–T18 eivät ole rekisterin teknisen velan tunnuksia T-001–T-052.

## 1. Nykytila / Current state

| Kohde / Item | Tila / State [todennettu inventaariosta] |
|---|---|
| `TpTest` (doctest, 46 tapausta) | toimii MSVC:llä; ajettu 46/46 2026-11-24; kattaa neljä puhdasta logiikkayksikköä, ei yhtään H-havaintoa |
| Ydintestit `TpCoreHk`, `TpCoreJuk` (Release ja Asan), ajuri `run_core_tests.py` | toimivat paikallisesti; viimeisin tallennettu ajo 2026-11-24: PASS 23, XFAIL 24 (47 manifestin riviä: 24 vikaa, 19 kontrollia, 4 tietoa); 15 havaintoa toistettu |
| Eristetyt koeohjelmat C-6, C-8, C-9 | lähteet ja tulokset todisteissa (`todisteet/c6/vaihe2/`, `todisteet/c8/koeohjelmat/`, `todisteet/c9/vaihe2/`); käännetään repon ulkopuolelle; 12 havaintoa; ei manifestia eikä odotetun tuloksen luokittelua |
| Toimitetun `HkMaali.exe`:n mustalaatikkoajo | ajettu kerran (H-041, rajapari 220/221); vaatii palomuurisäännöt, debuggerin ja interaktiivisen konsolin |
| Kahden prosessin loopback-kerros | suunniteltu, ei toteutettu |
| GUI-testit (HkKisaWin, ViestiWin) | ei yhtään; bcc32-ohjelmia ei ole ajettu auditoinnissa |
| bcc32-yksikkötestit | ei ole; doctest 2.4.11 ei käänny klassisella bcc32:lla |
| Laite-, SQL- ja monen työaseman testit | ei yhtään |
| `Tests/run.sh` | rikki: rivin 14 jatkomerkki puuttuu ja `-I Hk` puuttuu (yhdistäminen `6107c8f`); ei ajettu |
| CI | kääntää konsoliohjelmat (`v143`) ja GUI-ohjelmat (itseisännöity ajokone); **ei aja yhtään testiä**; julkaisupakettia ei käynnistetä |
| Konfiguraatiomatriisi T1–T18 | ei ajettu; katettu 1 rivi (T6, koeohjelmilla), osittain 4, ei lainkaan 12 |

Automaation esteet [todennettu]: ajuri ei hyväksy korjatun vian testin läpäisyä (XPASS palauttaa
virhekoodin 3), testiprojektit on sidottu työkaluversioon `v145`, kun CI käyttää `v143`:a (Q35),
ASanin saatavuus CI-kuvassa on tuntematon (Q35), push ja PR ovat toistaiseksi kiellettyjä, joten
CI-muutosta ei voi todentaa.

## 2. Testilajit / Kinds of tests

Jokaisella korjauksella on enintään kymmenen testilajia. Taulukko kertoo, mitä kukin tarkoittaa
tässä projektissa ja millä se tehdään.

| # | Laji / Kind | Tarkoitus / Purpose | Väline / Means | Tila / State |
|--:|---|---|---|---|
| 1 | Virheen todentaminen ennen korjausta / reproduce before the fix | osoittaa, että testi todella havaitsee vian (punainen) | ydintesti manifestissa lajilla "vika"; jos ei automatisoitavissa, kirjattu käsin tehtävä toisto | 27 havainnolle olemassa (15 kehyksessä, 12 koeohjelmina) |
| 2 | Korjauksen regressiotesti / regression test of the fix | sama testi läpäisee korjauksen jälkeen ja jää pysyväksi | manifestin laji vaihdetaan kontrolliksi; vaatii ajurin regressiotilan | ei vielä mahdollista (XPASS-käytäntö, OP-2) |
| 3 | Kokonaisuuden integraatiotestit / package integration tests | korjaukset toimivat yhdessä: sama polku alusta loppuun (esim. vastaanotto → tallennus → välitys) | ydintestit hallituilla säikeillä; kahden prosessin loopback | loopback puuttuu |
| 4 | Ohjelmakohtaiset testit / per-program tests | HK ja viesti erikseen; konsoli ja GUI erikseen, koska valitsimet ja koot eroavat | `TpCoreHk` ja `TpCoreJuk` toimitettavin määrityksin; ViestiLuennalle ei ole omaa kehystä | HK- ja viestikonsolin määritykset katettu; GUI- ja ViestiLuenta-määritykset eivät |
| 5 | Kääntäjäkohtaiset testit / per-compiler tests | sama koodi MSVC:llä ja bcc32:lla: koot, `swprintf`, `time_t`, RTL-erot, muistinhallinta | MSVC: doctest; bcc32: C-tyylinen koeohjelma (esikuva `rtl_bcc32.c`) tai vanhempi testikirjasto; rakenteiden kokojen käännösaikaiset tarkistukset | bcc32-puoli puuttuu |
| 6 | GUI-testit / GUI tests | lomakkeiden käsittelijät, VCL-säikeistys, viestisilmukka | käsin kirjoitetun käsikirjoituksen mukaan; myöhemmin mahdollisesti UI-automaatio | puuttuu |
| 7 | Laitetestit / device tests | lukijat ja kellot oikeilla laitteilla tai tallennetulla liikenteellä | TCP-tynkä ja sarjaporttipari simulointiin; oikea laite ennen julkaisua | puuttuu; laitteiden saatavuus OP-13 |
| 8 | Verkkotestit / network tests | työasemien välinen liikenne, sekaversiot, katkokset | loopback samalla koneella; kaksi konetta eristetyssä verkossa | puuttuu |
| 9 | Julkaistavan paketin testaus / release package test | ZIP:stä purettu ohjelma käynnistyy ja toimii puhtaalla koneella | virtuaalikone ilman kehitystyökaluja | puuttuu (Q98) |
| 10 | Käsin tehtävät hyväksymistestit / manual acceptance tests | kilpailun työnkulku alusta loppuun synteettisellä aineistolla | tarkistuslista (luku 6) | puuttuu |

Periaatteet:

- Testi käyttää muuttamatonta tuotantokoodia toimitettavin käännösasetuksin (C-11:n periaate).
  Testikoukkuja tuotantokoodiin lisätään vain päätöksellä OP-2.
- Testit eivät koske tuotantotietoihin, eivätkä avaa verkkoyhteyksiä oman koneen ulkopuolelle.
- "Toistettu testikehyksessä" osoittaa mekanismin, ei vian laukeamista toimitetussa ohjelmassa.
  Korjauksen hyväksyntä perustuu mekanismin poistumiseen ja siihen, ettei toiminta muutu.
- Suunniteltua testiä ei merkitä ajetuksi rekisteriin ennen kuin tulos on tallennettu.

## 3. Mitä testataan missäkin vaiheessa / What is tested when

| Vaihe / Point | Testit / Tests |
|---|---|
| Ennen korjausta / before the fix | lajin 1 testi punaisena; lähtötason tallennus: ydintestien tulos, kontrollit, tuotettujen tiedostojen (HTML, XML, `KILP.DAT`) tiivisteet samalla aineistolla |
| PR:ssä / in the PR | lajit 1–2 (punainen → vihreä), kaikki kontrollit, `TpTest`, ASan-kokoonpano, käännös MSVC:llä ja bcc32:lla; vastinhaaran tarkistus kirjattu |
| Kokonaisuuden valmistuessa / when a package completes | lajit 3–5; lajit 6–8 sen mukaan, mitä kokonaisuus koskee (luku 4) |
| Ennen julkaisua / before a release | lajit 9–10; konfiguraatiomatriisin rivit, joita julkaisun korjaukset koskevat; yhteensopivuustestit (luku 6) |

## 4. Testit kokonaisuuksittain / Tests per package

Sarakkeet: **Toisto nyt** = havainnot, joille on toistava testi tai koeohjelma jo olemassa
[todennettu]; **Puuttuu** = havainnot, joille testi on kirjoitettava; **Lajit** = tarvittavat
testilajit luvun 2 numeroin (1–2 kuuluvat aina); **Ennen julkaisua** = mitä on lisäksi ajettava.
P1–P3-kokonaisuudet ovat ensin; P4–P5-kokonaisuuksille riittää lajien 1–2 lisäksi mainittu.

| Kokonaisuus / Package | Toisto nyt / Reproduced today | Puuttuu / Missing | Lajit / Kinds | Ennen julkaisua / Before release |
|---|---|---|---|---|
| KK-C13-1 (P1) | H-032, H-026 (kehys) | H-071, H-072, H-061: uusi ydintesti vastaanottopolulle (paketin syöttö `addseur`in kautta); H-033: KILPPVT-paketti alueen ulkopuolisella vaiheindeksillä (ASan) | 3, 4, 8 | loopback: korjattu ↔ korjaamaton ohjelma, kaikki pakettiluokat; tiedostonsiirto (`LÄHTIEDOSTOT`) |
| KK-C13-2 (P2) | H-008 (kehys) | H-055, H-070, H-056, H-060: ydintesti lukijan syötepolulle synteettisellä sanomalla | 4, 7 | tallennettu tai oikea laiteliikenne vähintään yhdelle lukijatyypille |
| KK-C13-3 (P2) | H-128, H-129 (koeohjelma, vain hakemiston rivimäärä) | itse kekoylivuoto noin 1 500 tietueella (ASan); H-102: haku hallitulla säikeellä nollauksen aikana; H-013: tallennusten ja lähetysten määrä vaiheen vaihdossa ennen ja jälkeen; H-134 rikotulla `RADAT<n>.XML`:llä (Q110); H-105, H-174, H-141 | 3, 4, 6, 8, 10 | HK:n monivaiheinen kilpailu GUI:ssa ja konsolissa, vaiheen vaihto käynnissä olevassa prosessissa |
| KK-C13-4 (P2) | H-024, H-040 (kehys), H-090 (koeohjelma) | H-087, H-110: GUI-käsikirjoitus (ViestiWin); H-115, H-125 | 4, 5, 6 | ViestiWin: HTML-emit-tuloste ja "Lähtöajat" emit-liikenteen aikana |
| KK-C9-1 (P2) | H-149, H-150, H-151 (koeohjelma, ASan) | H-152 GUI:ssa; ViestiWinin ja HkKisaWinin käytös (Q91, Q95) | 4, 5, 6 | tuotettu HTML tavu tavulta sama nyt toimivilla poluilla; nyt kaatuvalla polulla vertailu listatilan tulosteeseen; virtojen määrä pitkässä ajossa |
| KK-C10-2 (P2) | – | H-159: TCP-tynkälukija (Q96); H-164: sarjaporttipari tai käsin | 4, 7 | oikea Sirit/Zebra-lukija ja USB-sarjasovittimen irrotus |
| KK-C12-2 (P2) | – | H-171: käännösaikainen kokotarkistus ja GUI-avaus pitkällä polulla (Q107); H-036: ydintesti ASanilla | 4, 5, 6 | nykyiset asetustiedostot luetaan samoin (avainvertailu) |
| KK-C13-5 (P2) | H-091, H-093 (koeohjelma) | H-009 käsin GUI:ssa (C-11:n avoin käsitesti); H-092, H-103; `writeerror_w` työsäikeestä lukon alla | 3, 5, 6 | GUI-ajo jatkuvalla sivutulosteella ja TCP-lokilla |
| KK-C13-7 (P2) | – | H-107: GUI asetuksella ja lukijalla; H-106 | 6 | ViestiWin ja HkKisaWin lukijaliikenteellä |
| KK-C13-8 (P2) | H-031 (kehys), H-094 (koeohjelma) | H-097 GUI:ssa; lopetus ASanilla (H-095, H-038) | 3, 4, 6, 7 | vaiheen vaihto ja lopetus lukijoiden ollessa kytkettyinä |
| KK-C13-9 (P2) | H-027 (kehys) | H-028: toinen yhteys samaan porttiin; H-155 | 3, 8 | vastapuolen uudelleenkäynnistys kesken kilpailun |
| KK-C13-10 (P2) | H-007, H-022, H-023 (kehys) | kahden aseman todellinen polku; hylätyn päivityksen näkyvyys | 3, 4, 8 | kaksi asemaa, samanaikaiset lisäykset ja monivaiheinen kilpailu |
| KK-C13-13 (P2) | H-001 (kehys) | H-073: paketti `EMVALOKI`-asetuksella; H-059; tulostepolut | 4 | tulosteiden vertailu samalla aineistolla |
| KK-C13-14 (P2) | – | H-109, H-111, H-116: GUI-käsikirjoitukset (J7-4); H-116:n toisto (virheikkunan sulkeminen X:llä) jo ensimmäisen erän valmisteluissa; ydinosat ydintesteinä, jos erotettavissa | 5, 6, 10 | molemmat GUI-ohjelmat: poisto, ajanoton rivin muokkaus, virheikkunan sulkeminen X:llä |
| KK-C13-24 (P2) | H-025 (kehys) | vanhan `COMFILE.DAT`:n avaus; H-048, H-050 | 3, 4, 8 | päivitys vanhasta versiosta jonotiedoston kanssa; tiedostonsiirto korjattu ↔ korjaamaton |
| KK-C10-1 (P2) | – | kaikki: paikallinen MySQL (Q97); H-014 on C-11:n avoin käsitesti | 6, SQL | palvelinkatko, virheellinen lause, vienti ja tuonti |
| KK-C10-4 (P2) | – | H-029: väärennetty datagrammi loopbackissa | 8 | eristetty verkko |
| KK-C13-11 (P3) | H-041, H-042 (kehys; H-041 myös toimitetussa ohjelmassa) | H-041:n lukupolku (Q37); H-046 | 4 | olemassa oleva `KILP.DAT` luetaan tavu tavulta samoin |
| KK-C13-6 (P3) | – | kilpailutilanteet hallituilla säikeillä | 3, 8 | rasitusajo kahdella asemalla |
| KK-C13-16 (P3) | – | rakenteiden vertailu tapahtumasarjan jälkeen | 4, 10 | käsin: poisto, sarjan ja numeron vaihto, sitten lukumäärät, sijoitukset ja numerojärjestyksen tulosteet |
| KK-C9-2 (P3) | H-145, H-146, H-147 (koeohjelma, virheinjektio) | H-015, H-044, H-148; kuittaus oikealla lukijalla (Q86, Q100); verkkolevy (Q87) | 3, 5, 7 | levy täynnä ja verkkolevyn katko käsin |
| KK-C10-3 (P3) | – | julkaisupaketin käynnistys puhtaalla koneella (Q98) | 9 | sama |
| KK-C12-1 (P3) | – | XML-tiedoston sisältö (Q108) | 4, 10 | vastaanottavan järjestelmän tarkistus, jos sellainen on |
| KK-C13-23 (P3) | – | suunnitellaan kokonaisuuden suunnittelun yhteydessä | 3, 8 | sekaversiot ja vanhat tiedostot |
| KK-C13-12 (P4) | H-002 (kehys) | tiedostotestit lukijoittain; `TextFl::Open` (C-5:n koeohjelma H-079 sovitettuna) | 5 | – |
| KK-C13-15 (P4) | – | GUI-käsikirjoitukset; bcc32:n negatiivinen `new` (Q63) | 5, 6 | – |
| KK-C9-3 (P4) | – | ydintesti puuttuvalla `HAJONNAT.LST`:llä (matriisin T14) | 4 | – |
| KK-C13-17 (P5) | – | objektivertailu; merkkijonojen tavuvertailu; `KUNTOSUUNN` molemmilla kääntäjillä (T16) | 5 | käyttöliittymän tekstit molemmissa GUI-ohjelmissa |
| KK-C12-3 (P5) | – | objektivertailu kaikilla toimitettavilla kokoonpanoilla | 5 | – |
| KK-C13-19–22 | – | määritellään kunkin suunnitteludokumentissa; edellytys: lajit 3–8 olemassa | 3–8 | – |

Kattavuus [todennettu rekistereistä]: P1- ja P2-havaintoja on 40, joista 19:lle on toistava testi
tai koeohjelma (H-032 sekä 18 P2:ta) ja 21:lle ei (H-033, H-061, H-071, H-072 sekä 17 P2:ta).
P3-havaintoja on 62, joista seitsemälle on toisto (H-001, H-041, H-042, H-094, H-145, H-146,
H-147). Puuttuvista P1/P2-testeistä noin puolet on kirjoitettavissa nykyiseen MSVC-kehykseen
(verkko- ja laitesyötteet); loput vaativat GUI:n (H-087, H-107, H-109, H-110, H-111, H-116, H-171),
MySQL:n ja GUI:n (H-160) tai laitteen (H-164). Vain GUI-ohjelmia koskee yhdeksän P2-havaintoa:
edellä mainitut seitsemän sekä H-091 (koeohjelma toistaa lukitusreunat MSVC:llä, ei GUI:ta) ja
H-160.

## 5. Infrastruktuurityöt / Infrastructure work (KK-C13-18)

Järjestyksessä; kohdat 1–4 ovat aalto 0 ja ensimmäisen toteutuserän edellytys.

| # | Työ / Work | Peruste / Reason | Päätös / Decision |
|--:|---|---|---|
| 1 | Ajurin regressiotila: korjatun vian testi muutetaan kontrolliksi samassa PR:ssä kuin korjaus, tai ajuri hyväksyy XPASS:n merkityille tapauksille | nyt korjaus näkyy virhekoodina 3 | OP-2 (`testaus.md` 12.1 kohta 4) |
| 2 | C-6:n, C-8:n ja C-9:n koeohjelmat `Tests/core`-hakemistoon manifestin alle | 12 havainnon toisto on nyt repon ulkopuolelle käännettäviä kopioprojekteja | – |
| 3 | `Tests/run.sh`: korjaus (jatkomerkki, `-I Hk`) tai poisto | H-006; g++-käännöksen toimivuus tuotantoyksiköillä on tuntematon | OP-2 (kohta 6) |
| 4 | Lähtötason tallennus ja yhden komennon paikallinen ajo | vertailukohta jokaiselle korjaukselle | – |
| 5 | Uudet ydintestit verkko- ja laitesyötteille (H-033, H-071, H-072, H-061, H-055, H-070, H-056, H-060, H-073, H-159) | P1/P2-aukot, jotka voi täyttää nykyisellä kehyksellä; H-159 vaatii TCP-tynkälukijan, joka kuuluu ensimmäisen erän valmisteluihin | – |
| 6 | Kahden prosessin loopback-kerros (127.0.0.1, erilliset hakemistot) | C-11:n avoin jatkotehtävä; tarvitaan kokonaisuuksille KK-C13-1, -9, -10, -24 | palomuurisääntö tarvitaan, koska ohjelma kuuntelee kaikissa osoitteissa (H-029) |
| 7 | GUI-testien käsikirjoitukset HkKisaWinille ja ViestiWinille | yhdeksän P2-havaintoa koskee vain GUI-ohjelmia; H-110:n, H-171:n ja H-116:n käsikirjoitukset kuuluvat ensimmäisen erän valmisteluihin | – |
| 8 | bcc32-testitapa: C-tyylinen koeohjelma tai bcc32:lla kääntyvä testikirjasto | doctest ei käänny; RTL-erot (C-5, C-9) vaikuttavat korjauksiin | – |
| 9 | Laitesimulaatio: TCP-tynkälukija (Sirit/Zebra, emiTag), sarjaporttipari | laitteita ei ole käytettävissä jokaisessa ajossa | OP-13 |
| 10 | Paikallinen MySQL-testiympäristö | KK-C10-1 | OP-7 |
| 11 | Testit CI:hin: `TpTest` ja ydintestit jokaisesta PR:stä, työkaluversion riippumattomuus, ASan jos kuvassa on (Q35), tulosten tallennus | JS-13 | OP-1, OP-2; push-rajoituksen purku |
| 12 | Varoitusten perustaso (T-021) ja uusien varoitusten esto muutetuissa tiedostoissa; `/we4789` vain, jos se toimii LTCG:n kanssa (Q39) | H-001:n kaltaiset viat näkyvät varoituksina | – |
| 13 | Julkaisupaketin testi: käynnistys puhtaalla koneella, testiportti ennen paketointia | Q98; KK-C10-3 | OP-18 |

C-11:n keskeneräiset jatkotehtävät (`testaus.md` 20.4) sijoittuvat näin: käsitestit H-009 → KK-C13-5
ja H-014 → KK-C10-1; loopback-kerros → kohta 6; H-041:n lukupolku (Q37) → KK-C13-11; testit CI:hin
(Q35, Q36, Q40) → kohta 11. Palomuurisääntöjen `TP C11 HkMaali block in/out` poisto on käyttäjän
ylläpitotehtävä eikä kuulu D-vaiheeseen. CI-työn yksityiskohtainen luettelo (13 kohtaa) on
inventaarion luvussa 3.5.

## 6. Julkaisun testaus / Release testing

Tarkistuslista jokaiselle julkaisulle, jossa on tuotantokoodin korjauksia [ehdotus]:

1. Kaikki viisi ohjelmaa ja neljä kirjastoa kääntyvät CI:n asetuksin (MSVC `v143`, bcc32) ja
   paikallisesti; ei uusia varoituksia muutetuissa tiedostoissa.
2. `TpTest` ja ydintestit läpi, myös ASan-kokoonpano.
3. Julkaisu-ZIP puretaan puhtaalle koneelle; jokainen ohjelma käynnistyy; GUI-ohjelmat avaavat
   esimerkkikilpailun.
4. Yhteensopivuus: edellisen julkaisun tuottamat `KILP.DAT`, emit-, AJAT- ja asetustiedostot
   avataan uudella versiolla; `COMFILE.DAT` sen mukaan, mitä `korjaussuunnitelma.md` luku 5 sanoo.
5. Sekaversiot: uusi ja edellinen versio samassa verkossa (HK–HK, viesti–viesti), ellei julkaisun
   ohje vaadi yhtäaikaista päivitystä.
6. Käsin tehtävä hyväksymisajo synteettisellä aineistolla: HK-kilpailu (ilmoittautuminen, lähtö,
   maali, emit-luenta, tulosteet, vaiheen vaihto) ja viestikilpailu (osuudet, vaihdot, automaattinen
   HTML-tuloste), kummassakin vähintään kaksi työasemaa ja sekä GUI- että konsoliohjelma.
7. Laitetesti niillä laitetyypeillä, joiden koodia julkaisu muuttaa.
8. Julkaisutiedot: korjatut havainnot, yhteensopivuushuomiot ja mahdolliset päivitysohjeet.

## 7. Avoimet ajotehtävät / Open run-time obligations

Auditoinnin erissä kertyi 80 avointa todennustehtävää (inventaarion luku 4, rivi kerrallaan
edellytyksineen) [todennettu lukumäärät inventaariosta]:

| Testityyppi / Test type | Määrä / Count | Esimerkkejä / Examples |
|---|--:|---|
| Automaattinen ydintesti (MSVC) / automated harness test | 23 | H-071, H-072, H-055, H-070; H-129:n ylivuoto; Q110, Q34 |
| Automaattinen bcc32-testi / automated bcc32 test | 8 | Q45, Q63, Q91; RTL-erot |
| Integraatio (loopback, usea työasema) / integration | 10 | H-022, H-027, H-032 oikealla polulla; Q21, Q93 |
| GUI | 15 | H-087, H-103, H-109, H-110, H-116, H-152, H-153; Q80, Q95, Q107 |
| Laite / device | 5 | Q44 (ARES), Q86 ja Q100 (Emit), Q96 (Sirit/Zebra), emiTag |
| SQL | 3 | Q97, Q105, Q106 |
| Julkaisupaketti / release package | 1 | Q98 |
| Käsin tehtävä hyväksyntä / manual acceptance | 15 | H-009, H-014, Q87, Q89, Q108 |

Avoimista kysymyksistä 39 on luokiteltu ajonaikaiseen todennukseen D-vaiheessa
(`auditoinnin-yhteenveto.md` luku 6). Ne sisältyvät näihin tehtäviin ja ovat kokonaisuuksien
`kysymykset`-sarakkeessa (`rekisteri/korjauskokonaisuudet.csv`). Yksikään näistä ei estä
ensimmäisen toteutuserän rajattuja korjauksia; Q43 (H-061) ja Q44 (H-056) ratkeavat korjauksen
testiä kirjoitettaessa.

## 8. Konfiguraatiomatriisi T1–T18 / Configuration matrix

Matriisi on `ominaisuusvalitsimet.md`:n luvussa 10; sitä ei ole ajettu. Rivit kytkeytyvät
kokonaisuuksiin näin:

| Rivi / Row | Kokonaisuus / Package | Laji / Kind | Katettu nyt / Covered today |
|---|---|---|---|
| T1 yksi asema ilman asetustiedostoa | KK-C12-2 | 4, 10 | osittain (HkMaalin ajot 0–1) |
| T2 kaksi asemaa UDP/TCP | KK-C13-1, KK-C13-9, KK-C13-10 | 3, 8 | osittain (yhden prosessin pakettitestit) |
| T3 emit-luenta testisyötteellä | KK-C9-2 | 4 | ei |
| T4 RFID, uudelleenyhteys | KK-C10-2 | 4, 7 | ei |
| T5 RFID-avain ViestiLuennassa | KK-C12-2, KK-C12-3 | 10 | ei |
| T6 `autofile` listalla ja ilman | KK-C9-1 | 4 | kyllä (koeohjelmat, MSVC) |
| T7 SQL-peili | KK-C10-1 | SQL | ei |
| T8 HK:n vaiheen vaihto, rikkinäinen `RADAT2.XML` | KK-C13-3, KK-C13-8 | 4, 6 | osittain |
| T9 emiTag `TCPS` / HTTP | KK-C13-2, KK-C10-2 | 4, 7 | ei |
| T10 ViestiWin, pitkä polku | KK-C12-2 | 6 | ei |
| T11 viestikonsolin X)ML | KK-C12-1 | 10 | ei |
| T12 asetuseditorin edestakainen tallennus | KK-C12-2 | 6 | ei |
| T13 rikkinäinen `KilpSrj.xml` | KK-C13-3 | 4 | viereinen |
| T14 hajontatuloste ilman `HAJONNAT.LST`:ia | KK-C9-3 | 4 | ei |
| T15 `NREGNLY` 8 ja 32 | KK-C12-2 | 4, 6 | ei |
| T16 `KUNTOSUUNN` MSVC ja bcc32 | KK-C13-17 | 5 | ei |
| T17 toimittamattomat kokoonpanot | KK-C12-2 (OP-12) | 4 | vain käännös |
| T18 `KATKKIER0` | KK-C12-2 | 4 | ei |

Matriisin oma järjestys on T6, T8, T14, T18 (kehysrivit) ja sitten T10, T11, T12 (toimitettu
ohjelma). Ensimmäinen toteutuserä kattaa rivit T6, T8 (osin), T4 (tynkä) ja T10.

## 9. Kattavuus ja rajoitteet / Coverage and limits

- Suunnitelma kattaa jokaisen P1–P3-kokonaisuuden kolmessa kohdassa: ennen korjausta, korjauksen
  jälkeen ja ennen julkaisua (luku 4). Arkkitehtuurikokonaisuuksien KK-C13-19, -20 ja -23 testit
  määritellään vasta niiden suunnittelussa.
- Tallennettu lähtötaso (PASS 23, XFAIL 24) ajettiin puusta, jossa testitiedostot olivat vielä
  commitoimatta; lähtötaso ajetaan uudelleen puhtaasta puusta ennen ensimmäistä korjausta.
- Nykyinen automaatio on vain MSVC-konsolikäännöksille. bcc32-GUI-ohjelmien korjaukset
  todennetaan aluksi käsin; tämä on suunnitelman suurin heikkous, ja kohdat 7–8 luvussa 5
  pienentävät sitä.
- Laite-, SQL- ja monen koneen testit riippuvat ympäristöistä, joita ei ole vielä olemassa
  (OP-7, OP-13).
- Testikehys asettaa tilan käsin; se ei korvaa hyväksymisajoa toimitetulla ohjelmalla.

/ The plan covers every P1-P3 package before the fix, after it and before a release. Automation
exists only for MSVC console builds; bcc32 GUI fixes are verified manually at first. Device, SQL and
multi-machine tests depend on environments that do not exist yet.
