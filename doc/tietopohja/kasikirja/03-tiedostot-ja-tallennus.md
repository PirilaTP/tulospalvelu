# 03 Tiedostot, tallennus ja uudelleenkäynnistys

<!-- malli: Käsikirjan lukupohja (K-3). Ohje: doc/tietopohja/laatu/k3-kirjoittajan-ohje.md.
Luvun 5 vikaluettelo generoidaan tiedostosta kasikirja/toiminnot.csv komennolla
python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kasikirja; sitä ei kirjoiteta käsin. -->

> **Tila:** alustava K-3-aineisto, ei riippumattomasti tarkastettu. Lähderevisio: tuotantokoodi
> `f7fe553` (ennallaan, `doc/arkkitehtuuri/CHECKPOINT.md`). Varmuusmerkinnät: [todennettu],
> [päätelty], [tuntematon]; "luettu käsikirjaa varten" = luettu koodista K-vaiheessa, ei tarkastettu.

Luku kertoo, mitä tiedostoja ohjelmat kirjoittavat, milloin kirjoitus tapahtuu, mitä "tallennettu"
tarkoittaa ja mitä ohjelman käynnistyessä uudelleen tapahtuu, myös kaatumisen jälkeen. Kilpailun
perustaminen ja avaaminen on luvussa 02, Emit-kortin luenta ja sen oma tallennusketju luvussa 05,
tiedonsiirron jonon kulku luvussa 10 ja virhetilanteet ja toipuminen luvussa 13. Polut ovat
suhteessa hakemistoon `TPsource/V52/`. Lyhenteet: HKM = HkMaali, HKW = HkKisaWin, VM = ViestiMaali,
VW = ViestiWin, VL = ViestiLuenta.

## 1. Mitä tapahtuu

### 1.1 Perusperiaate: muisti ensin, levy perässä

Kilpailijatiedot (HK) ja joukkuetiedot (viesti) ovat ajon aikana kokonaan muistissa. Jokainen
muutos kirjoitetaan heti myös levylle tietue kerrallaan; ohjelmassa ei ole "Tallenna kilpailu"
-toimintoa eikä erillistä tallennushetkeä [päätelty: `virheenkasittely.md` luku 4 ja
`globaali-tila.md` 4.3, jotka kuvaavat pysyvyyden perustuvan jokaisen muutoksen
`PUTREC(d, 1)`-kirjoitukseen; luku 02 kohta 1.10]. Muistissa olevat johdetut rakenteet
(numerohakemisto, nimihakemisto, sijoitusketjut, laskurit) eivät ole levyllä, vaan ne rakennetaan
joka käynnistyksessä uudelleen tietueista [todennettu: `tiedostomuodot.md` "Tiedostot" ja luku
02 kohta 1.3].

HK:ssa kirjoitusjärjestys on tärkeä: `kilptietue::tallenna` päivittää muistitaulukon tietueen
**ensin** ja kirjoittaa levylle vasta sen jälkeen (`PUTREC`, `Hk/HkDat.cpp:581-624`). Jos levykirjoitus
epäonnistuu, muisti on silti jo uusi ja ohjelma jatkaa sillä [todennettu: `virheenkasittely.md`
luku 4 ja 12.7; H-015]. Tietue ei voi tallentua puoliksi ohjelman sisällä, sillä `GETREC` ja
`PUTREC` kopioivat koko tietueen lukon alla; riski on menetetty päivitys, ei rikkinäinen tietue
[todennettu, tarkastettu: `tallenna-samanaikaisuus.md` "Johtopäätös"]. Toisin sanoen tässä luvussa
"tallennettu" voi tarkoittaa kolmea eri asiaa, joita ei pidä sekoittaa:

| Taso | Mitä on tapahtunut | Mistä sen tietää |
|---|---|---|
| Kutsuttu | `tallenna` tai `PUTREC` on ajettu | ei mistään; paluuarvoa ei useimmiten välitetä kutsujalle |
| Kirjoitettu | käyttöjärjestelmän `WriteFile` tai `_write` palautti oikean tavumäärän | virheessä ilmestyy ikkuna (luku 13), mutta ohjelma jatkaa |
| Kestävä | tavut ovat levyllä virtakatkonkin jälkeen | vain `KILP.DAT`:ssa (`FILE_FLAG_WRITE_THROUGH`); muissa tiedostoissa ei `_commit`ia (H-156, T-041) |

[todennettu: `virheenkasittely.md` luku 4 (kestävyys) ja 12.7; `tiedostomuodot.md` "KILP.DAT:
tiedostokerros"]

### 1.2 Tiedostot yhdellä silmäyksellä

Kaikki tiedostot ovat joko kiinteän tietuekoon taulukoita tai tekstiä. Binaaritiedostoissa ei ole
tunnistetta, versiota, tallennettua tietuepituutta eikä tarkistussummaa; ainoa eheystarkistus on
`KILP.DAT`:n otsakkeen tietuemäärän vertailu tiedoston pituuteen [todennettu, tarkastettu:
`tiedostomuodot.md` "Johtopäätös"]. HK ja viesti eivät voi lukea toistensa binaaritiedostoja, ja
saman ohjelman eri kilpailujen tiedostot eivät ole keskenään vaihdettavia, koska tietuekoko riippuu
vaiheiden, osuuksien ja väliaikojen määrästä [todennettu, tarkastettu: sama].

| Tiedosto | Ohjelmat | Sisältö | Kirjoittaja ja milloin |
|---|---|---|---|
| `KILP.DAT` | kaikki viisi | kilpailijat (HK) tai joukkueet ja osuudet (viesti); tietue 0 on otsake | jokainen tietueen muutos heti (`PUTREC`, `addrec`); GUI ja konsoli samaa koodia |
| Emit-tiedosto: `EMIT<vaihe>.DAT` (HK), `EMIT.DAT` (viesti) | HK ja viesti, jossa Emit-käsittely on päällä | luetut Emit-kortit, `emittp`-tietue 188 tavua | kortin luennan yhteydessä lisäys tiedoston loppuun; käsittelyn jälkeen tietue kirjoitetaan uudelleen (luku 05) |
| Ajanottotiedosto: `AJAT1.LST`, `LAJAT1.LST` (HK), `AJAT.LST` (viesti) | HK ja viesti | ajanottojonon rivit (aika, numero, piste, tunniste) | rivi kerrallaan ajan syntyessä; avattaessa tiedosto kirjoitetaan uudelleen |
| `COMFILE.DAT` (monivaiheisessa `COMFILE1.DAT` jne.) | kaikki | lähetysjonon levypuskuri: tiedonsiirtopaketit | jokainen jonoon laitettu paketti kirjoitetaan jonoon laittohetkellä |
| `SQLjonoVarm.dat` | HKW ja VW, vain MySQL-peili ja asetus `CBVarmista` | SQL-kirjoitusjonon varmistus | n. 300 ms välein (GUI) |
| `peruscfg.cfg`, `LASKENTA.CFG` (tai `CFG=`) | kaikki | asetukset | ohjelma lukee käynnistyksessä; ohjelma itse ei kirjoita niitä, asetuseditori kyllä |
| `vaihevalinta.cfg` | HK | viimeksi valittu vaihe | vaiheen vaihdossa |
| `KilpSrj.xml` (varana `KILP.SRJ`), `RADAT1.XML` | HK; viesti vastaavilla | sarja- ja ratatiedot | luetaan käynnistyksessä ja vaiheen vaihdossa; kirjoitus lomakkeilta |
| Tulos-, automaatti- ja vientitiedostot (HTML, XML, teksti) | kaikki | julkaistavat tulokset | tulostus- tai automaattikierros |
| `DATA_ERR.TXT`, tapahtumalokit | HK | ilmoitukset, lokirivit | tapahtuman yhteydessä, puskuroituna |

[lähteet: `tiedostomuodot.md` "Tiedostot", "Muut tiedostot"; `virheenkasittely.md` luku 4;
käyttöohje `suunnistuskilpailu/tietojen_jakelu.md` ja `hkkisawin/hkliite1.md` nimien osalta]

Indeksitiedostoja ei ole: dbboxm:n B-puu-indeksikerros on kuollutta koodia, ja kaikki hakemistot
rakennetaan muistiin käynnistyksessä [todennettu grepillä, tarkastettu: `tiedostomuodot.md`
"Tiedostot"].

### 1.3 KILP.DAT

**Rakenne.** Tietue `r` on tiedostossa kohdassa `r * tietuekoko`; tietuekokoa ei ole tiedostossa,
vaan ohjelma laskee sen asetuksista. Tietue 0 on otsake (vapaiden lista, tietuemäärä, `maaliajat`).
HK-tietue on 360 tavua perustietoja ja vaihetta kohden 152 tavua sekä `valuku+2` väliaikaa à 8
tavua; merkkijonot ovat UTF-16LE. Viestin tietue on perustiedot ja osuudet peräkkäin, ja merkkijonot
ovat 8-bittisiä (pakotettu UTF-8:ksi) [todennettu, tarkastettu: `tiedostomuodot.md` "KILP.DAT: HK-tietue",
"viestitietue"]. Tietueen kokoa ei voi päätellä tiedostosta, joten väärillä asetuksilla avattu tiedosto
yleensä hylätään pituustarkistuksessa eikä lueta väärin [todennettu: `tiedostomuodot.md`
"KILP.DAT: tiedostokerros"].

**Avaus.** Tiedosto avataan niin, että muut prosessit saavat vain lukea sitä
(`FILE_SHARE_READ`) ja kirjoitus on läpikirjoittava (`FILE_FLAG_WRITE_THROUGH`)
(`dbboxm/dbbox1.cpp:118`, `:180`). Seuraus: toinen ohjelma samassa hakemistossa pysähtyy
`KILP.DAT`:iin ennen kuin ehtii koskea `COMFILE.DAT`:iin tai emit-tiedostoon
[todennettu: `tiedostomuodot.md`, sama kohta; `Hk/HkInit.cpp:3486` ennen `:3609`/`:3659`].
Jos tiedostoa ei ole, HKM luo tyhjän automaattisesti kilpailukansioon (`Hk/HkInit.cpp:3478-3482`),
joten väärästä kansiosta käynnistetty ohjelma ei pysähdy virheeseen [luettu käsikirjaa varten,
ei tarkastettu; ks. luku 02 kohta 7.3].

`openfile` vaatii, että tiedoston pituus on tietuekoon monikerta ja että otsakkeen tietuemäärä on
yhtä suuri kuin tiedostosta laskettu määrä tai yhtä pienempi (yhden otsakkeettoman lisäyksen
`addrec` ehtii tehdä ennen kaatumista) (`dbboxm/dbbox1.cpp:161-211`). Muussa tapauksessa käynnistys
estyy ilmoitukseen "otsikkoalue virheellinen", eikä korjaustyökalua ole [todennettu, tarkastettu:
`tiedostomuodot.md`; luku 02 kohta 1.3].

**Kirjoitus.**

- *Päivitys:* `PUTREC` kirjoittaa koko tietueen paikalleen `kilpdat_CriticalSection`-lukon alla
  (HK, `Hk/HkDat.cpp:597-624`). Viestin `tallenna` lukitsee itse `tall_CriticalSection`in
  (`Juk/vdat.cpp:496`, vapautus `:850`) [todennettu, tarkastettu: `tallenna-samanaikaisuus.md`].
- *Lisäys:* `newrec` lisää aina tiedoston loppuun (`ainauusirec = 1`); vapaiden lista ei ole
  käytössä, eikä tietuenumeroita käytetä uudelleen. Tietuenumero on siksi paikallinen
  tiedostoindeksi, ja eri työasemilla sama kilpailija voi olla eri tietuenumerolla [todennettu:
  `tiedostomuodot.md`, "ainauusirec"; H-022].
- *Poisto:* `deleterec` kirjoittaa poistetun tietueen päälle otsakkeen kuvan, jolloin tietueen
  ensimmäinen kenttä on nollasta poikkeava; aktiivinen tietue on sellainen, jonka `kilpstatus == 0`
  [todennettu: `tiedostomuodot.md`].
- *Järjestys lisäyksessä:* datan ja otsakkeen kirjoitus ovat erillisiä; kaatuminen niiden välissä
  siedetään, koska avaus hyväksyy otsakkeen tietuemäärän olevan yhden pienempi kuin tiedostosta
  laskettu (`int2 == numrec-1`). Tietuemäärä kasvatetaan muistissa ennen levykirjoitusta
  (`dbboxm/dbbox1.cpp:227-251`), joten levykirjoituksen epäonnistuminen jättää otsakkeen
  ristiriitaiseksi [todennettu: `tiedostomuodot.md` "Eheys ja vikatilanteet";
  `virheenkasittely.md` luku 4].
- *Muille asemille:* sama `tallenna` lähettää muutoksen kaikkien yhteyksien lähtöjonoon, myös silloin,
  kun levykirjoitus epäonnistui [todennettu: `emit-luenta.md` 2.7 E6 ja `virheenkasittely.md` 12.7].

**Sulkeminen.** `lopetus1` sulkee `KILP.DAT`:n (`closefile`, `Hk/HkInit.cpp:3790`). Sulkemisessa
ei kirjoiteta tietueita: jokainen tietue on jo kirjoitettu. HK:n GUI kutsuu sulkiessaan ensin
`talljarj`ia, joka laskee sijoitukset uudelleen muistissa (`sijat2`, `Hk/HkApu.cpp:298-311`) [luettu
käsikirjaa varten, ei tarkastettu]; se ei kirjoita tiedostoa.

### 1.4 Emit-tiedosto

Emit-tiedosto on `emittp`-tietueiden taulukko ilman otsaketta: tietuenumero on indeksi, ja
tietueiden määrä saadaan tiedoston pituudesta (`emithead = filelength / sizeof(emittp)`)
(`Hk/HkEmit.cpp:3060`, `Juk/VEmit.cpp:3609`). Vain osittain kirjoitettu viimeinen tietue ohitetaan,
ja seuraava lisäys kirjoittaa sen päälle [todennettu, tarkastettu: `tiedostomuodot.md` "Emit-tiedosto"].

Kirjoitus tapahtuu kahdessa vaiheessa [todennettu: `emit-luenta.md` 2.3, 8.1]:

1. Kun kortti luetaan, `addem` lisää tietueen tiedoston loppuun (`putem`).
2. Kun kortti on käsitelty ja yhdistetty kilpailijaan, tietue kirjoitetaan uudelleen samaan
   paikkaan (`putem(&em, emithead2, 0)`).

Lisäksi `kilptietue::tallenna` kirjoittaa emit-tietueita uudelleen, kun kilpailijan emit-koodi tai
numero muuttuu (`putem(..., 1)`). Tämä tapahtuu HK:ssa ilman `emit`-lukkoa, ja viestissä lukon kanssa
[todennettu, tarkastettu: `tallenna-samanaikaisuus.md` "Mitä tallenna muokkaa" ja "Viesti"]. Tietueen
kirjoitus ja luku käyttävät erillisiä `_lseek`- ja `_write`/`_read`-kutsuja jaetulla kahvalla, joten
jokaisen kutsujan pitäisi pitää `emit_CS`:ää [todennettu: `tiedostomuodot.md`; H-011, H-045].

Emit-kortin ketju kokonaisuudessaan (luenta, kuittaus lukijalle, käsittely, lähetys) on luvussa 05
ja aineistossa `aineisto/emit-luenta.md`; tässä luvussa tärkeintä on, että kuittaus lukijalle ja
tallennus levylle ovat eri asioita (kohta 1.9).

### 1.5 Ajanottotiedosto

Ajanottojono (`aikajono[ino]`) on muistissa aikajärjestyksessä oleva rivien taulukko, ja sillä on
levytiedosto. Tiedoston ensimmäinen tietue on otsake, jossa on teksti `AJAT_V<n>`; ohjelma
tarkistaa vain kuusi ensimmäistä merkkiä, ei versionumeroa. Ilman otsaketta tiedosto luetaan
vanhana muotona. HK:n rivi on 22 tavua ja viestin 24 tavua, joten tiedostot eivät ole vaihdettavissa
[todennettu: `tiedostomuodot.md` "AJAT-jono"; `Hk/HkIV.cpp:4026`, `:4040-4045`]. Tiedosto kirjoitetaan
avattaessa kokonaan uudelleen [todennettu, tarkastettu: sama].

Rivin kirjoitus (`tall_rivi` → `writeajat`) tapahtuu, kun aika syntyy tai riviä muutetaan; HK:ssa tämä
on erillinen vaihe ennen kilpailijan tallennusta: ensin jonon rivi, sitten kilpailija (`tallenna`)
(luku 04 kohta 1.1). Jonon rivi ja kilpailijan tulos ovat kaksi eri levyyn kirjoitettua asiaa
[todennettu: luku 04 kohta 1.1, `Hk/HkIV.cpp:2998-3030` luettu käsikirjaa varten]. Vaiheen vaihto
kirjoittaa tiedoston koko sisällön uudelleen [todennettu: `virheenkasittely.md` luku 4 taulukko, rivi AJAT].

Tiedoston nimi tulee asetuksista `AJAT=nimi` ja `LAJAT=nimi` (lähtöportin ajat eri tiedostoon);
oletus on HK:ssa `AJAT1.LST` (vaiheen numero nimessä) [KO: `hkkisawin/hkliite1.md`; AA:
`tiedostomuodot.md`]. Yksivaiheisessa kilpailussa HK poistaa nimestä viidennen merkin, myös
käyttäjän antamasta nimestä (H-132); vaiheen vaihdon jälkeen avattu uusi jono voi saada edellisen
vaiheen nimen (H-131) [todennettu: vikakortit H-131, H-132].

### 1.6 COMFILE.DAT: lähetysjonon levypuskuri

`COMFILE.DAT` varmistaa tiedonsiirron: jokainen lähtevä paketti kirjoitetaan siihen jonoon
laittohetkellä, ja lähetys tapahtuu tästä tallennetusta kopiosta. Tiedosto on tasainen taulukko:
rivi 0 on otsake, ja yhteyden `cn` paketti `n` on kohdassa `((n+1)*maxyhteys + cn) * combufsize`.
Paketin otsake on 14 tavua (`lahetetty`, portti, `so`, `id`, `iid`, `pkgclass`, `len`,
`checksum`) [todennettu: `tiedostomuodot.md` "COMFILE.DAT"; `Tp/TpComY.cpp:476-530`]. Levyjono on
oletus sekä HK:ssa että viestissä; viestissä sitä ei voi kytkeä pois (`Juk/VInit.cpp:1217-1219`).
HK:ssa muistijono kytketään asetuksella `COMFILE=EI` (käyttöohje `hkkisawin/hkliite1.md`; muuttuja
`levypusk`, `Hk/HkInit.cpp:1256-1259`). Asetus koskee koko ohjelmaa, ei yksittäistä yhteyttä: kaikki
yhteydet käyttävät samaa jonotyyppiä, ja `levypusk` ratkaisee myös, lähteekö vaiheen vaihdon
pyyntö (luku 02 kohta 1.9). Muistijono pudottaa paketin, kun jono (`OUTBUFL` = 50) on täynnä (H-021) [todennettu: `tiedonsiirto.md` "Jonot ja
levypuskuri"].

Kirjoitus, lähetys ja kuittaus ovat kolme eri asiaa: pakettia ei merkitä lähetetyksi ennen kuin
vastaanottaja on kuitannut sen; kuittaus kertoo kuitenkin vain, että paketti on vastaanottajan
muistissa, ei että se on käsitelty tai tallennettu levylle [todennettu, tarkastettu: `tiedonsiirto.md`
"Kehys ja kuittaus"; `emit-luenta.md` 2.8 F4–F5; F5 päätelty]. Jos `COMFILE.DAT`-kirjoitus
epäonnistuu, jono etenee silti, ja edellinen paketti lähtee uudelleen uudella tunnuksella tai tyhjä
paikka merkitään lähetetyksi (H-044).

Monivaiheisessa kilpailussa tiedostoja on yksi vaihetta kohden: `COMFILE1.DAT`, `COMFILE2.DAT`
jne. [KO: `suunnistuskilpailu/tietojen_jakelu.md`]. Käyttöohje neuvoo poistamaan `COMFILE.DAT`:n, kun
`KILP.DAT` jaetaan eri tietokoneille, koska tiedosto voi sisältää lähettämättömiä sanomia, jotka
aiheuttavat virhetilanteita [KO: sama].

### 1.7 SQL-peilin varmistustiedosto SQLjonoVarm.dat

MySQL-peili on valinnainen ja vain GUI-ohjelmissa (HKW, VW); konsoliohjelmissa ei ole toimivaa
SQL-koodia. `KILP.DAT` ja muistitaulukko ovat ensisijaiset, ja SQL:stä luetaan takaisin vain
käyttäjän käynnistämässä tuonnissa [todennettu: `riippuvuudet.md` luku 11]. SQL-kirjoitukset kulkevat
muistissa olevan jonon kautta; jos asetus `CBVarmista` on päällä, jonon tila varmistetaan tiedostoon
`SQLjonoVarm.dat`: 8 tavun pareja {tietuenumero, jonon pituus}, ja uudelleenkäynnistyksessä
viimeiset 8 tavua kertovat, montako paria toistetaan (`cbHk/UnitMySQL.cpp:665-681`) [todennettu:
`tiedostomuodot.md` "SQLjonoVarm.dat"]. Tiedosto avataan lisäystilassa, joten paikallaan päivitys
lisää tiedoston loppuun, tiedosto kasvaa ja uudelleenkäynnistys voi lähettää vääriä tietueita
(H-043). Peilin kirjoitus voi lakata vaiheen vaihdon jälkeen ilman ilmoitusta (H-014).

### 1.8 Asetustiedostot

`lue_parametrit` lukee `peruscfg.cfg`:n ja sitten `LASKENTA.CFG`:n (tai komentorivin `CFG=`-tiedoston)
ja komentorivin; luku tapahtuu vain käynnistyksessä (`Hk/HkInit.cpp:327-386`). Merkistö tunnistetaan
tiedostokohtaisesti (BOM, muuten arvaus ANSI/OEM/UTF-8); uudet tiedostot kirjoitetaan UTF-8:na BOMin
kanssa. Rivit katkaistaan hiljaa 77 merkkiin, `;` aloittaa kommentin, parametreja voi olla enintään
200, ja tuntemattomasta avaimesta tulee varoitus [todennettu: `tiedostomuodot.md` "Konfiguraatio";
luku 02 kohta 1.3]. Asetuseditori kirjoittaa joitakin avaimia, joita ohjelman oma lukija ei tunne;
tunnetuin on `AIKA_COM=`, joka tulee takaisin nimellä `AIKACOM=` ja katoaa (H-173).

`vaihevalinta.cfg` kirjoitetaan vaiheen vaihdossa; tiedosto tyhjennetään ennen kirjoitusta, joten
virhe jättää väärän tai tyhjän vaiheen seuraavaan käynnistykseen (H-156; kohta N-B9). Käyttöohje
neuvoo antamaan asetuksen `vaihe=v` ja huolehtimaan, että `vaihevalinta.cfg` osoittaa oikeaan
vaiheeseen, koska uudelleenkäynnistys väärään vaiheeseen aiheuttaa lieviä ongelmia [KO: luku 02
kohta 1.9, joka viittaa `hkkisawin` 10.5].

### 1.9 Kirjoitus vs onnistunut tallennus

Ohjelman virhekäsittely on rakenteeltaan sama kaikissa pysyvissä tiedostoissa: virhe huomataan,
siitä näytetään ikkuna, mutta paluuarvo ei välity kutsujalle, uudelleenyritystä ei ole ja kutsuja
jatkaa kuin tallennus olisi onnistunut [todennettu, tarkastettu: `virheenkasittely.md` luku 1,
T-040]. Taulukko kokoaa, mitä kunkin tiedoston kirjoitusvirhe jättää jälkeensä:

| Tiedosto | Muistissa | Levyllä | Muille asemille | Uudelleenkäynnistyksen jälkeen | Havainto |
|---|---|---|---|---|---|
| `KILP.DAT` | uusi | vanha tietue; lisäyksessä otsake voi olla ristiriidassa | lähetetty | vanha tietue; ristiriitainen otsake estää käynnistyksen | H-015, H-148 |
| Emit-tiedosto | kortti välimuistissa | puuttuu | ei lähetetä | korttia ei ole; uusi luku tallentaa sen | H-145 |
| Ajanottotiedosto | rivi olemassa | tyhjä paikkamerkkirivi | n/a | rivi puuttuu, jonon keskelle jää tyhjä rivi | H-146 |
| `COMFILE.DAT` | jono etenee | puolikas tai tyhjä paikka | paketti jää lähettämättä tai edellinen lähtee uudelleen | uudelleenkäynnistys pysähtyy ensimmäiseen aukkoon | H-044 |
| Tulos- ja automaattitiedosto | – | tyhjä tai osittainen; vanha ehjä tiedosto menetetty | – | kirjoitetaan seuraavalla kierroksella | H-147 |

[todennettu: `virheenkasittely.md` 12.7 ja 12.2–12.4, joissa kohdat on toistettu testikehyksessä
MSVC Release -käännöksellä; GUI-käännösten (bcc32, VCL) ajonaikainen käytös on mittaamatta]

Lukijalle annettu kuittaus on vielä erikseen: Emit-kellolle annetaan kuittaus `+\r`, kun
`tall_emit` palauttaa 0, myös silloin kun tallennus epäonnistui (H-145), ja myös hylätylle
kaksoiskappaleelle (H-168) [todennettu: `riippuvuudet.md` luku 13, Q86; `virheenkasittely.md`
12.2]. Muut lukijat kuluttavat sanoman joka tapauksessa. Laitteen tulkinta kuittauksesta on
[tuntematon] (Q86, laitedokumentaatio puuttuu).

### 1.10 Uudelleenkäynnistys

Normaali uudelleenkäynnistys etenee luvun 02 kohdan 1.3 ketjun mukaan: asetukset, vaihe, sarjat,
`KILP.DAT`, muistirakenteet, radat ja emit-tiedosto, yhteydet ja laitteet. Tässä käynnistyksen
pysyvyyteen liittyvät kohdat.

**KILP.DAT.** Kaikki tietueet luetaan muistiin (`teejarj(false)`), ja hakemistot rakennetaan
uudelleen [todennettu: luku 02 kohta 1.3]. Ohjelma palaa tilaan, jonka levytiedosto sisältää. Jos
joku kirjoitus oli epäonnistunut ilman että käyttäjä huomasi sen, vanha tietue palaa (H-015).

**Ajanottotiedosto.** Käynnistyksessä ohjelma kysyy, säilytetäänkö aiemmat ajat. Konsolissa
kysymys on "AJANOTTOTIEDOSTON AVAAMINEN - Säilytetäänkö aiemmat ajanottotiedot (K/E)", GUI:ssa
"Luetaanko aiemmat tiedot levyltä" ja vaihtoehdot *Luetaan*, *Aiemmat tuhotaan* ja *Säilytä vanhat ja
poistu*; lähtöajoille on oma vastaava kysymys (`Hk/HkIV.cpp:4040-4075`) [luettu käsikirjaa varten, ei
tarkastettu]. Asetus `AJAT=nimi/S` säilyttää tiedoston kysymättä [KO: `hkkisawin/hkliite1.md`].
Vastaus *Aiemmat tuhotaan* on peruuttamaton: ohjelma kirjoittaa tiedoston avatessaan kokonaan uudelleen
[todennettu, tarkastettu: `tiedostomuodot.md`; koodi luettu käsikirjaa varten].

**COMFILE.DAT.** Ohjelma laskee jokaiselle yhteydelle peräkkäiset kelvolliset rivit ja niistä lähetetyt
(`Tp/TpComY.cpp:2306-2330`). Jos tiedostossa on lähettämättömiä paketteja, ohjelma kysyy: konsolissa
"Tiedostossa COMFILE N lähetettyä ja M lähetettävää tietoa S)äilytä T)uhoa" ja GUI:ssa
"Säilytetäänkö?" (Kyllä/Ei) (`Tp/TpComY.cpp:2335-2360`) [luettu käsikirjaa varten, ei
tarkastettu]. Asetus `COMFILE=S` säilyttää tiedoston kysymättä [KO: `hkkisawin/hkliite1.md`]; se toimii myös viestissä (`levypusk = 2`, `Juk/VInit.cpp:1219`), ja kysymys ohitetaan myös vaiheen vaihdossa (ehto `levypusk == 1 && !vaihto`, `Tp/TpComY.cpp:2331`) [luettu käsikirjaa varten, ei tarkastettu]. Vastaus
"Ei" tai "Tuhoa" hävittää lähettämättömät paketit pysyvästi (H-038). Jos otsake on yhteensopimaton
(`MAXYHTEYS` tai tietuekoko eri), ohjelma kysyy, tuhotaanko vanha tiedosto vai keskeytetäänkö
käynnistys; jos otsake on rikki mutta tiedoston pituus on `so*len`, tiedosto katkaistaan kysymättä
(`:2290-2292`, `:2349`, H-038) [todennettu: `tiedostomuodot.md` "COMFILE.DAT"; ensimmäinen kysely
luettu käsikirjaa varten]. Ensimmäinen virheellinen rivi lopettaa laskennan, ja sen jälkeiset
jonossa olevat paketit jäävät lähettämättä (H-044). Muistijonon paketit katoavat aina
[todennettu: `tiedonsiirto.md` "Kättely ja lopetus"].

**Emit-tiedosto.** `lueemitfile` (`Hk/HkEmit.cpp:3029-3112`) lukee tiedoston, rakentaa `emitjarr`-linkit
uudelleen ja vertaa tietueiden `kilpno`a `KILP.DAT`:n koodeihin. Eroista kirjoitetaan ilmoitus
`DATA_ERR.TXT`:hen. Kaksoiskappaleiden esto (`on_emit`) ei talletu levylle, joten se on käynnistyksen
jälkeen tyhjä (H-034) [todennettu: `emit-luenta.md` 2.10]. `emithead2` asetetaan heti viimeisen
käsitellyn tietueen jälkeen (`:3103-3104`); mitkä kortit jäävät automaattikäsittelyn ulkopuolelle,
on luvussa 05 (H-137, ehdokas KE-11).

**Tiedonsiirto.** Uudelleenyhteys alkaa `ALKUT`-paketilla, ja järjestysnumero nollautuu. Levyjono
säilyttää kuittaamattomat paketit ja lähettää vain kuittaamattoman loppuosan alkuperäisessä
järjestyksessä; kuitattuja ei lähetetä uudelleen [todennettu, tarkastettu: `tiedonsiirto.md` "Jonot ja
levypuskuri"]. Matkalla ollut paketti voidaan soveltaa kahdesti (H-034).

**Sarjat, radat.** Jos `KilpSrj.xml` on virheellinen, ohjelma korvaa sen hiljaa vanhalla `KILP.SRJ`:llä
(H-174, luku 02). Tämä koskee myös käynnistystä kaatumisen jälkeen.

### 1.11 Kaatuminen ja virtakatko

Mitä kaatuminen (prosessin päättyminen, tapettu ohjelma, virtakatko) jättää jälkeensä, tiedoston
mukaan [kokoelma: `tiedostomuodot.md` "Eheys ja vikatilanteet"; `saikeet-ja-lukot.md` 4.2;
`virheenkasittely.md` luvut 4 ja 6]:

| Tilanne | Seuraus | Varmuus |
|---|---|---|
| Prosessi päättyy tai tapetaan datan ja otsakkeen kirjoituksen välissä (`addrec`) | `KILP.DAT` avautuu (otsake saa olla yhden jäljessä) | [todennettu, tarkastettu] |
| Prosessi tapetaan ennen `closefile`a | `KILP.DAT` avautuu silti; viimeaikaisten poistojen vapaalistapaikat ja `maaliajat`-tavut otsakkeessa voivat olla vanhoja (Q73) | [päätelty, tarkastettu]; ei rekisteröity viaksi |
| Virtakatko tietueen kirjoituksen aikana | tuntematon (Q27); läpikirjoitus kiertää käyttöjärjestelmän välimuistin, mutta levyn oma välimuisti ja kesken jäänyt kirjoitus ovat avoimia | [tuntematon] |
| Emit-, AJAT-, `COMFILE`- ja lokitiedostot kaatumisessa | `kirjheti`-liput suojaavat prosessin kaatumiselta, mutta ei virtakatkolta, koska `_commit`ia ei ole | [todennettu: `virheenkasittely.md` luku 4; H-156, T-041] |
| Kaatuminen emit-tietueen kirjoituksen aikana | puolikas viimeinen tietue ohitetaan ja seuraava lisäys kirjoittaa sen päälle | [todennettu, tarkastettu] |
| Muistijonon lähettämättömät paketit | katoavat aina | [todennettu] |
| HkMaalin konsoli (`_exit(0)`) | `lopetus1` päättyy `_exit`iin, joka ei huuhtele C-kirjaston puskureita eikä sulje `aftulparam.lstf`:ää; tekstitiedostot, joihin kirjoitetaan C-virran kautta, voivat jäädä vajaiksi | [todennettu: `virheenkasittely.md` luku 6; H-157] |
| Ohjelma kaatuu automaattitulosteeseen (VM, VL: H-149) | `KILP.DAT` ei korruptoidu tämän vuoksi, mutta ohjelma pysähtyy kesken kilpailun | [todennettu: koeohjelma, `virheenkasittely.md` 12.1] |

Tiedostot eivät kerro, onko edellinen ajo päättynyt siististi: otsakkeessa ei ole "puhdas sulku" -lippua
eikä tarkistussummaa [päätelty: `tiedostomuodot.md` "Johtopäätös", jossa todetaan, ettei mikään tunnista
muotoa tai eheyttä lukuun ottamatta otsakkeen tietuemäärää]. Käynnistys ei siis tee korjausajoa eikä
vertaa emit-tiedoston ja `KILP.DAT`:n tilaa muuhun kuin paikalliseen kilpailijakoodien vertailuun
(`lueemitfile`). Ainoa korjauskeino eriytyneille asemille on käsin kopioitava `KILP.DAT`
[todennettu, tarkastettu: `tiedonsiirto.md` "Johtopäätös"].

### 1.12 Varmuuskopiot

Ohjelma ei tee itse varmuuskopioita `KILP.DAT`:sta. Käyttöohjeessa mainitaan kilpailutietojen kirjoitus
siirtotiedostoon (csv, xml), MySQL-kantaan tai "ajan tasalla olevaan kopioon käytössä olevasta
tiedostosta KILP.DAT" (*Tiedostot / Kirjoita siirtotiedostoon*) [KO: `hkkisawin/1.4.1_valinta_tiedostot.md`];
ohjelma voi siis tuottaa kopion, mutta ei tee sitä automaattisesti [päätelty: koodista ei löydy ajastettua
kopiointia, mutta kattavaa hakua ei ole tehty; [tuntematon] siltä osin].

## 2. Erot ohjelmien välillä

| Kohta | HK (HKM, HKW) | Viesti (VM, VW, VL) |
|---|---|---|
| `KILP.DAT`-tietue | 360 t + vaiheet; UTF-16LE | perustiedot + osuudet; 8-bittinen, UTF-8 |
| `KILP.DAT`-lukko | `kilpdat_CS` jokaisessa getrec/putrec; `tallenna` itse ei lukitse | `tall_CS` `tallenna`ssa; `kilpdat_CS` vain `addfrec`/`delfrec` |
| Lisäys | `addrec` | `addfrec` ottaa lukon kahdesti eikä vapauta (H-040), sulkee ja avaa tiedoston jokaisen lisäyksen jälkeen ja kaatuu tarkistamattomaan avaukseen levyn täyttyessä (H-148) |
| Vienti "kirjoita KILP.DAT" | kirjoittaa `pack`-tuloksen | kirjoittaa muistin olion, joten osuudet ≥ 1 ovat väärissä kohdissa, kun `valuku < VAIKALUKU` (H-046) |
| Emit-tiedosto | `EMIT<vaihe>.DAT`, `emitvatp` 236 t | `EMIT.DAT`, `emitvatp` 204 t |
| Ajanottotiedosto | `AJAT1.LST`, `LAJAT1.LST`, rivi 22 t | `AJAT.LST`, rivi 24 t; uudelleenkirjoitus jättää viimeisen rivin pois täydestä jonosta (H-054) |
| `COMFILE.DAT` | otsake 14 t mukana `combufsize`ssa; muistijono mahdollinen | `combufsize` ilman otsaketta: paketin loppu ei tallennu (H-025); levyjono aina |
| Vaiheen vaihto | vaihevalinta.cfg, uudet tiedostot | ei vaiheen vaihtoa |
| Peruskonfiguraatio | `peruscfg.cfg`, `LASKENTA.CFG` | XML (`Juk/VXml.cpp:430`) |
| SQL-peili, `SQLjonoVarm.dat` | HKW | VW |

Konsoli ja GUI: samat tiedostot ja sama tallennuskoodi; erot ovat kysymysten ja ilmoitusten tavassa
(konsolin rivi tai valinta, GUI:n ikkuna) ja siinä, että SQL ja `SQLjonoVarm.dat` ovat vain GUI:ssa.
GUI-ohjelmat ovat bcc32/VCL-käännöksiä ja konsoliohjelmat MSVC-käännöksiä; ajonaikainen käytös on
mitattu vain MSVC:llä [todennettu: `virheenkasittely.md` luku 12 ja 13.2]. `ViestiLuenta` käyttää samaa
`KILP.DAT`-koodia kuin ViestiMaali; sen muista tiedostoista (emit, ajat) ei ole omaa erittelyä
[päätelty: sama lähdekoodi, `riippuvuudet.md` luku 13 mainitsee Emit-kellon ei-VL-tuen; [tuntematon]
tarkemmin].

HK:n ja viestin tiedostot ovat yhteensopimattomia, eikä HK- ja viestiasema voi olla yhteydessä keskenään
(viesti hylkää vaiheen ≠ 0) [todennettu, tarkastettu: `tiedonsiirto.md` "Kättely ja lopetus"].

## 3. Asetukset ja käännösvalitsimet

| Asetus tai valitsin | Ohjelmat | Vaikutus | Lähde |
|---|---|---|---|
| `AJAT=nimi[/S]`, `LAJAT=nimi` | HK | ajanottotiedoston nimi; `/S` säilyttää kysymättä | [KO: `hkkisawin/hkliite1.md`]; `rekisteri/asetusavaimet.csv` (`MAALI`-ehto) |
| `COMFILE=S`, `COMFILE=EI`, `MAXYHTEYS=n` | `S`: HK ja viesti; `EI`: vain HK (viestissä levyjono aina, jäsennin ohittaa `EI`:n); koko ohjelman asetus | levyjonon säilytys kysymättä; muistijono; `COMFILE.DAT`:n rivimäärä | [KO: `hkkisawin/hkliite1.md`, `viestiwin/hkliite1.md`]; `tiedonsiirto.md` |
| `BOOT` | HK | käynnistys ilman vahvistuksia; muut kyselyt ohitetaan erikseen (`AJAT=/S`, `COMFILE=S`) | [KO: `hkkisawin/hkliite1.md`] |
| `vaihe=v`, `vaihevalinta.cfg` | HK | vaiheen valinta käynnistyksessä | luku 02 kohta 1.3 |
| `CFG=` | kaikki | vaihtoehtoinen asetustiedosto | `tiedostomuodot.md` |
| `SQLHOST`, `SQLDATABASE`, `SQLUSER`, `SQLPASSWORD`, `SQLSTART`, `SQLTALLENNUS`, `CBVarmista` | HKW, VW | MySQL-peili; varmistustiedosto | `riippuvuudet.md` luku 11 |
| `MAALI` | kaikki | pakotettu päälle kaikissa tuotteissa (`valitsimet.csv`, rivi `PP:MAALI`) | `rekisteri/valitsimet.csv` |
| `VALUKU`, `N_PV`, `MAXOSUUSLUKU` | HK, viesti | ylärajat; tietuekoko lasketaan ajon aikana sarjatiedoista | `tiedostomuodot.md` "Mitatut koot" |
| `TALLEMIT` | – | ei koskaan määritelty (`dead-never-defined`); `res = putem` -muunnos ei ole käytössä | `valitsimet.csv` |
| `SQLDATA` | – | ei koskaan määritelty; viestin SQL-koodi kuollut | `valitsimet.csv` |
| `UDPYHTEYS` | kaikki | määritelty suoraan lähdetiedostoissa; UDP-säikeet käynnistyvät | `valitsimet.csv`; `tallenna-samanaikaisuus.md` |

Toimitetut kokoonpanot: CI ja julkaisu kääntävät `Release`-kokoonpanon, jossa HK:n `VALUKU` on 250 ja
`N_PV` 5, ja viestin `MAXOSUUSLUKU` 30. Muut kokoonpanot (200, 150os) ovat lisäkonfiguraatioita
[todennettu, tarkastettu: `tiedostomuodot.md` "Mitatut koot"]. Tämä vaikuttaa tietuekokoon: `KILP.DAT`
on kooltaan sidottu käännökseen ja kilpailun asetuksiin.

## 4. Vaihtoehtoiset polut ja virhetilanteet

- **Levy täynnä `addrec`issa:** otsake väittää yhden tietueen enemmän kuin tiedostossa on, joten
  seuraava käynnistys epäonnistuu; myöhempi lisäys voi jättää nollatietueen, joka ladataan aktiivisena.
  `getrec`in epäonnistuessa `read` purkaa alustamattoman pinopuskurin (H-015, laajennus) [todennettu:
  `tiedostomuodot.md` "Eheys ja vikatilanteet"].
- **Virheilmoitus KILP.DAT-kirjoituksesta:** `perr` näyttää modaali-ikkunan, jonka teksti on englanniksi
  ("... Error N from putrec on line N: ... Continue execution?", otsikko "Console API Error", napit
  Kyllä/Ei); "Ei" kutsuu `lopetus()`ia (`tputilv2/perr.cpp:28-60`) [luettu käsikirjaa varten]. Ikkuna
  näytetään `kilpdat_CS`-lukko varattuna (H-015) ja voi lukita GUI:n, jos näyttävä säie pitää lukkoa,
  jota lopetus tarvitsee (H-153) [todennettu: `virheenkasittely.md` luvut 4, 6]. Ks. luku 13.
- **Emit-tiedoston lukuvirhe:** `lueem` ohittaa `_read`-tuloksen (HK) tai vertaa tulosta ilman muunnosta
  (viesti), joten lukuvirhe jää huomaamatta ja puskuriin jää vanha tai alustamaton sisältö (H-053).
- **Kaksi ohjelmaa samassa hakemistossa:** toinen pysähtyy `KILP.DAT`:iin ennen kuin ehtii katkaista
  `COMFILE.DAT`:n [todennettu: `tiedostomuodot.md`].
- **Samanaikaiset kirjoittajat:** useita polkuja kirjoittaa tiedostoihin ilman lukkoa tai lukon
  ulkopuolella olevalla pitkäaikaisella kopiolla: emit-tiedosto (H-011, H-045), sarjanumeroiden siirto,
  tuonti ja joukkueen lisäys viestissä (H-047), lomakkeiden pitkään pitämä tietuekopio (H-017), dbboxm:n
  jaettu otsakepuskuri viennissä (H-051) [todennettu, vaihtelevasti tarkastettu: `tallenna-samanaikaisuus.md`,
  `tiedostomuodot.md`].
- **Tietueen koko ylittää puskurin:** HK:n `getrec`-puskuri on 10 000 tavua, mutta toimitetussa
  Release-käännöksessä tietue voi olla 11 200 tavua, kun `n_pv` = 5 ja `valuku` ≥ 220 (H-041); viestissä XML:n
  osuus- ja väliaikamääriä ei rajata (H-042) [todennettu].
- **Vanhan muodon tiedosto:** ei tunnisteta automaattisesti. Tavallisessa käynnistyksessä väärän muodon
  tiedosto yleensä hylätään pituustarkistuksessa; vanha `KILP.DAT` luetaan vain GUI:n erillisellä
  tuonnilla, jossa käyttäjä antaa muodon parametrit (H-049) [todennettu: `tiedostomuodot.md` "Vanha muoto"].
- **Replikoinnin eriytyminen:** viimeksi saapunut voittaa; sanomissa ei ole versiota eikä aikaleimaa;
  jälkikäteen ei korjata (H-016) [todennettu, tarkastettu: `tiedonsiirto.md` "Johtopäätös"].
- **COMFILE.DAT-katselin:** asettaa otsakkeen pituuden globaaliin `combufsize`en, jolloin käynnissä oleva
  jono voi sotkeutua (H-050).

## 5. Tunnetut viat tässä toiminnossa

<!-- generoitu:viat alku (tietopohja.py lohkot; älä muokkaa käsin) -->
| Toiminto | Havainto | Tila | Prioriteetti | Kuvaus |
|---|---|---|---|---|
| Kilpailijatietueen tallennus ja lukitus (tallenna, PUTREC) | [H-007](../vikakortit/H/H-007.md) | avoin | P2 | Toiselta asemalta tullut päivitys voi kumota samaan aikaan paikallisesti tehdyn muutoksen ja levittää vanhan tiedon muille asemille (HK) |
| Kilpailijatietueen tallennus ja lukitus (tallenna, PUTREC) | [H-008](../vikakortit/H/H-008.md) | avoin | P2 | Virheellinen ampumasakkosanoma voi jättää tallennuksen lukon päälle, jolloin tallennukset eivät etene |
| Kilpailijatietueen tallennus ja lukitus (tallenna, PUTREC) | [H-009](../vikakortit/H/H-009.md) | avoin | P3 | HkKisaWin voi jäädä odottamaan itseään, kun työsäie pyytää käyttöliittymältä jotain kilpailijatietojen lukko hallussaan |
| Kilpailijatietueen tallennus ja lukitus (tallenna, PUTREC) | [H-010](../vikakortit/H/H-010.md) | avoin | P3 | Noin 35 kohtaa tallentaa kilpailijan tiedot ilman lukkoa, ja tallennus muuttaa rakenteita, joilla ei ole omaa lukkoa (HK) |
| Kilpailijatietueen tallennus ja lukitus (tallenna, PUTREC) | [H-017](../vikakortit/H/H-017.md) | avoin | P3 | Emit-muutokset-ikkuna voi kirjoittaa kilpailijan vanhan tietueen takaisin ja kumota välissä tulleet muutokset |
| KILP.DAT-kirjoitusvirhe (muisti ensin, levy perässä) | [H-015](../vikakortit/H/H-015.md) | avoin | P3 | Jos kilpailijatiedoston KILP.DAT kirjoitus levylle epäonnistuu, ohjelma jatkaa muistissa olevilla tiedoilla ja levylle jää vanha tieto |
| KILP.DAT:n tietuekoko ja tietueen puskuri | [H-041](../vikakortit/H/H-041.md) | avoin | P3 | Suuri kilpailukokoonpano ylittää tietueen pinopuskurin: HkMaali kaatui kilpailijaa lisättäessä, kun väliaikapisteitä oli 221 |
| KILP.DAT:n tietuekoko ja tietueen puskuri | [H-042](../vikakortit/H/H-042.md) | avoin | P3 | Viestikilpailun XML-tiedoston osuus- ja väliaikamääriä ei rajata, joten tietuepuskuri voi ylittyä |
| KILP.DAT:n vienti, tuonti ja jaettu otsakepuskuri | [H-046](../vikakortit/H/H-046.md) | avoin | P3 | ViestiWinin KILP.DAT-vienti kirjoittaa tietueet väärässä muodossa, kun väliaikapisteitä on vähemmän kuin käännöksen enimmäismäärä |
| KILP.DAT:n vienti, tuonti ja jaettu otsakepuskuri | [H-047](../vikakortit/H/H-047.md) | avoin | P3 | Sarjanumeroiden siirto, tuonti ja joukkueen lisäys kirjoittavat kilpailijatiedostoon tai sulkevat sen ilman lukkoa, kun muu säie kirjoittaa samaan tiedostoon |
| KILP.DAT:n vienti, tuonti ja jaettu otsakepuskuri | [H-049](../vikakortit/H/H-049.md) | avoin | P4 | Vanhan KILP.DAT-tiedoston tuonti voi ylikirjoittaa muistia, jos tuontilomakkeen luvut eivät vastaa tiedostoa |
| KILP.DAT:n vienti, tuonti ja jaettu otsakepuskuri | [H-051](../vikakortit/H/H-051.md) | avoin | P4 | Kilpailijatiedoston vienti voi kirjoittaa väliaikaistiedoston otsakkeen kilpailijatiedostoon, koska otsakepuskuri on yhteinen (HkKisaWin) |
| Viestin joukkueen lisäys (addfrec) | [H-040](../vikakortit/H/H-040.md) | avoin | P2 | Viestissä joukkueen lisäys jättää tiedostolukon päälle, ja toisen säikeen lisäys tai poisto voi jumiuttaa ohjelman |
| Viestin joukkueen lisäys (addfrec) | [H-148](../vikakortit/H/H-148.md) | avoin | P4 | Viestissä uuden joukkueen lisäys voi levyn täyttyessä jättää joukkueen pois muistista ja tehdä KILP.DAT:sta käyttökelvottoman istunnon ajaksi |
| Emit-tiedoston kirjoitus ja uudelleenluku | [H-011](../vikakortit/H/H-011.md) | avoin | P3 | Kilpailijan emit-koodin tai numeron muutos kirjoittaa emit-tiedostoon ilman emit-lukkoa, kun lukija voi kirjoittaa samaan tiedostoon (HK) |
| Emit-tiedoston kirjoitus ja uudelleenluku | [H-045](../vikakortit/H/H-045.md) | avoin | P3 | Emit-analyysin kytkeminen päälle kesken ajon lukee emit-tiedoston uudelleen ilman emit-lukkoa (HkKisaWin) |
| Emit-tiedoston kirjoitus ja uudelleenluku | [H-053](../vikakortit/H/H-053.md) | avoin | P4 | Jos emit-tiedoston luku levyltä epäonnistuu, ohjelma ei huomaa sitä ja jatkaa puskurin vanhalla tai alustamattomalla sisällöllä |
| Emit-tiedoston kirjoitus ja uudelleenluku | [H-145](../vikakortit/H/H-145.md) | avoin | P3 | Jos emit-tiedoston kirjoitus epäonnistuu, luettu kortti kuitataan lukijalle mutta jää tallentumatta |
| Ajanottotiedoston kirjoitus ja nimeäminen | [H-054](../vikakortit/H/H-054.md) | avoin | P4 | Viestin aikajonon uudelleenkirjoitus jättää täydestä jonosta viimeisen rivin pois, ja lukuvirheessä suljetaan väärä kahva |
| Ajanottotiedoston kirjoitus ja nimeäminen | [H-131](../vikakortit/H/H-131.md) | avoin | P3 | Vaiheen vaihdon jälkeen avattu aikajono voi tallentaa ajat edellisen vaiheen tiedostoon |
| Ajanottotiedoston kirjoitus ja nimeäminen | [H-132](../vikakortit/H/H-132.md) | avoin | P4 | Yksivaiheisessa kilpailussa ohjelma poistaa ajanottotiedoston nimestä yhden merkin, myös käyttäjän antamasta nimestä |
| Ajanottotiedoston kirjoitus ja nimeäminen | [H-146](../vikakortit/H/H-146.md) | avoin | P3 | Jos aikajonon (AJAT) rivin kirjoitus levylle epäonnistuu, rivi merkitään silti kirjoitetuksi ja levylle jää tyhjä aukko |
| Lähetysjonon levypuskuri COMFILE.DAT | [H-021](../vikakortit/H/H-021.md) | avoin | P3 | Kun lähetysjono pidetään muistissa (COMFILE=EI), täysi jono pudottaa sanomia, ja jonon täyttymisestä varoittava ilmoitus voi jäädä odottamaan käyttäjää lukkojen ollessa varattuina |
| Lähetysjonon levypuskuri COMFILE.DAT | [H-025](../vikakortit/H/H-025.md) | avoin | P2 | Viestiohjelmien lähetysjonon paikka on 14 tavua liian lyhyt, joten suurimman sanomatyypin loppuosa ei tallennu jonoon ja voi lähteä väärällä sisällöllä |
| Lähetysjonon levypuskuri COMFILE.DAT | [H-044](../vikakortit/H/H-044.md) | avoin | P3 | Jos lähetysjonon levytiedoston COMFILE.DAT kirjoitus tai luku epäonnistuu, jono etenee silti: paketti jää lähettämättä tai edellinen paketti lähtee uudelleen |
| Lähetysjonon levypuskuri COMFILE.DAT | [H-048](../vikakortit/H/H-048.md) | avoin | P4 | Konsolin uudelleenlähetysvalikon sanomanumerovalinta lukee lähetysjonosta väärän kohdan, joten aiottujen kilpailijoiden tietoja ei lähetetä uudelleen |
| Lähetysjonon levypuskuri COMFILE.DAT | [H-050](../vikakortit/H/H-050.md) | avoin | P3 | Lähetysjonotiedoston katselin muuttaa käynnissä olevan tiedonsiirron jonon paikan pituutta ja voi sotkea jonon tai kaataa ohjelman |
| COMFILE.DAT:n säilytyskysely käynnistyksessä | [H-038](../vikakortit/H/H-038.md) | avoin | P3 | Ohjelman lopetus ei odota taustasäikeitä: KILP.DAT voidaan sulkea kirjoittajan alta, ja COMFILE-kysymykseen vastaus Ei hävittää lähettämättömät tiedot |
| Emit-kaksoiskappaleiden esto uudelleenkäynnistyksessä | [H-034](../vikakortit/H/H-034.md) | avoin | P4 | Yhteyden palautuessa tai ohjelman käynnistyessä uudelleen sama sanoma tai emit-kortti voi tulla käsitellyksi kahdesti |
| SQL-peilin varmistustiedosto SQLjonoVarm.dat | [H-014](../vikakortit/H/H-014.md) | avoin | P3 | Vaiheen vaihdon jälkeen SQL-peili lakkaa päivittymästä, eikä ohjelma ilmoita siitä |
| SQL-peilin varmistustiedosto SQLjonoVarm.dat | [H-043](../vikakortit/H/H-043.md) | avoin | P3 | SQL-peilin varmistustiedosto kasvaa ja sekoittuu, ja uudelleenkäynnistyksen jälkeen peiliin voi lähteä vääriä tietoja |
| Asetuseditorin kirjoittamat asetukset | [H-173](../vikakortit/H/H-173.md) | avoin | P3 | Asetuseditorit tallentavat avaimia, joita ohjelma ei tunne, ja aikalähteen asetus AIKA_COM katoaa tallennettaessa editorilla |
| Tiedostojen kestävyys (ei atomista kirjoitusta, ei _commit) | [H-156](../vikakortit/H/H-156.md) | avoin | P5 | Pysyvän tiedon kirjoituksen pienet virhepolut: osa kirjoitusvirheistä jää huomaamatta tai ohjelma ilmoittaa onnistumisesta epäonnistuneen kirjoituksen jälkeen |
| Tiedostojen kestävyys (ei atomista kirjoitusta, ei _commit) | [T-040](../vikakortit/T/T-040.md) | avoin | - | Virheet kerrotaan käyttäjälle ikkunoina työsäikeistä eikä palauteta kutsujalle, joten kutsuja ei tiedä tallennuksen epäonnistuneen |
| Tiedostojen kestävyys (ei atomista kirjoitusta, ei _commit) | [T-041](../vikakortit/T/T-041.md) | avoin | - | Tiedostoja ei kirjoiteta kokonaisina tai kestävästi: julkaistavat tiedostot tyhjennetään paikallaan, eikä tallennusta pakoteta levylle |

Generoitu tiedostosta `kasikirja/toiminnot.csv` ja rekistereistä (tila, prioriteetti); älä muokkaa käsin.
<!-- generoitu:viat loppu -->

## 6. Mihin tieto perustuu

- Asiakirjat: `doc/arkkitehtuuri/tiedostomuodot.md` (koko dokumentti, erityisesti "Tiedostot", "KILP.DAT",
  "COMFILE.DAT", "Muut tiedostot", "Eheys ja vikatilanteet", "Mitatut koot"); `tallenna-samanaikaisuus.md`
  (koko); `virheenkasittely.md` luvut 1, 4, 6, 12.2–12.4, 12.7; `tiedonsiirto.md` "Johtopäätös",
  "Kättely ja lopetus", "Jonot ja levypuskuri"; `saikeet-ja-lukot.md` 4.1–4.2; `riippuvuudet.md` luvut
  11 ja 13; `doc/tietopohja/kasikirja/aineisto/emit-luenta.md` luvut 2.3, 2.7–2.10, 8.1.
- Rekisteri: `havainnot.md`, `riskiarvio.csv` (tila `avoin` kaikilla tässä luvussa mainituilla; H-099
  kumottu, ei mainita), `valitsimet.csv` (`MAALI`, `TALLEMIT`, `SQLDATA`, `UDPYHTEYS`), `asetusavaimet.csv`.
- Vikakortit: `doc/tietopohja/vikakortit/H/` kortit H-015, H-044, H-131, H-132, H-145, H-146, H-148, H-156
  (tässä luvussa viitattu).
- Käyttöohje (vain asetusnimet ja käyttäjälle näkyvät kysymykset): `docs-site/docs/hkkisawin/hkliite1.md`,
  `hkkisawin/1.4.1_valinta_tiedostot.md`, `suunnistuskilpailu/tietojen_jakelu.md`,
  `viestiwin/hkliite1.md`.
- Koodi uutena (luettu käsikirjaa varten, ei tarkastettu): `Tp/TpComY.cpp:2280-2365` (COMFILE-kysely
  käynnistyksessä); `Hk/HkIV.cpp:3995-4075` (AJAT-avaus ja kysely); `Hk/HkApu.cpp:298-311` (`talljarj`);
  `cbHk/WinHk.cpp:793-805` (sulkeminen); `tputilv2/perr.cpp:28-100` ja `dbboxm/dbbox1.cpp:60-100`
  (KILP.DAT-virheikkuna); `cbTp/cbTpApu.cpp:445-495` (`writeerror_w`).
- Käännösvalitsimet tarkistettu: `MAALI` pakotettu päälle kaikissa viidessä tuotteessa; `TALLEMIT` ja
  `SQLDATA` kuolleita (`valitsimet.csv`); GUI/konsoli-ero `_CONSOLE`.

## 7. Mitä ei ole selvitetty

- **Q27** Virtakatko kirjoitusläpäisevän lisäyksen aikana: seuraus [tuntematon].
- **Q73** Konsolin `CTRL_CLOSE`: mitä `maaliajat`-tavuille tapahtuu, kun prosessi tapetaan ennen
  `closefile`a [päätelty, ei rekisteröity].
- **Q87** Verkkolevyn kahvan kirjoitukset katkon jälkeen: epäonnistuvatko pysyvästi vai yhdistääkö SMB
  uudelleen (vaikuttaa H-015, H-146).
- **Q90** Ajetaanko kilpailuja muistipuskurilla (`COMFILE=EI`), jolloin lähettämättömät sanomat katoavat
  joka lopetuksessa.
- **Q94** Käytetäänkö ajanottotiedoston riviä koskaan tulosten uudelleenrakentamiseen (H-146:n
  vaikutus tuloksiin).
- **Q86** Emit-kellon tulkinta kuittauksesta (laitedokumentaatio puuttuu).
- Muut avoimet kysymykset: tallennuksen osalta `tallenna-samanaikaisuus.md` "Avoimet kysymykset"
  (linkkerikartta, `unpack0`:n kenttien kattavuus, VCL-poikkeus lukon aikana).
- Tutkimatta: viestin vastaanottopolku ja sen lukitus (`tallenna-samanaikaisuus.md` "Viesti");
  `ViestiLuenta`n omat tiedostokäytännöt; AJAT-tiedoston avauksen kaikkien kyselyjen ja asetusten
  (`sail_ajat`, `LAJAT`) tarkka vuorovaikutus (luettu vain osittain käsikirjaa varten).
- Automaattisten varmuuskopioiden puuttuminen on päätelty, ei kattavasti haettu.
- Ehdokkaat (ei tarkastettu): ei uusia havaintoja. Kaikki tässä luvussa mainitut viat ovat jo
  rekisterissä. Haettu `havainnot.md`:stä ja `seuranta.md`:n KE-taulukosta: KE-5 (`deltall` nollatulla
  tietueella `GETREC`-virheen jälkeen) ja KE-10 (emit-tietueen kirjoitusvirhe käsittelyvaiheessa)
  liittyvät tähän lukuun; KE-10 on kuvattu luvussa 05 ja aineistossa `emit-luenta.md` 8.1.
