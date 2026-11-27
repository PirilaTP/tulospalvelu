# 11 Tulosten julkaisu: tulosteet, HTML, XML, automaattitulostus ja SQL

<!-- malli: Käsikirjan lukupohja (K-3). Ohje: doc/tietopohja/laatu/k3-kirjoittajan-ohje.md.
Luvun 5 vikaluettelo generoidaan tiedostosta kasikirja/toiminnot.csv komennolla
python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kasikirja; sitä ei kirjoiteta käsin. -->

> **Tila:** alustava K-3-aineisto, ei riippumattomasti tarkastettu. Lähderevisio: tuotantokoodi
> `f7fe553` (ennallaan, `doc/arkkitehtuuri/CHECKPOINT.md`). Varmuusmerkinnät: [todennettu],
> [päätelty], [tuntematon]; "luettu käsikirjaa varten" = luettu koodista K-vaiheessa, ei tarkastettu.

Luku kertoo, miten laskettu tulos pääsee kilpailun ulkopuolelle: paperille, tekstitiedostoon, HTML- ja
XML-tiedostoiksi, automaattisesti ajastettuna tiedostona verkkopalvelimelle vietäväksi, sekä valinnaisena
kopiona SQL-tietokantaan. Tuloksen laskenta on luvussa 06, viestin erityispiirteet luvussa 07,
tiedonsiirto toisten työasemien kanssa luvussa 09 ja tallennusvirheet luvussa 13. Luku nojaa auditointiraportteihin
`doc/arkkitehtuuri/virheenkasittely.md` (jäljempänä "virheenkasittely.md"), `riippuvuudet.md` lukuihin 11 ja 12
(tietokanta, HTTP ja selain), `ominaisuusvalitsimet.md`:ään, vikakortteihin ja käyttöohjeeseen
`docs-site/docs/` (luvut 9.2–9.7). Polut ovat suhteessa hakemistoon `TPsource/V52/`. Lyhenteet: HKM = HkMaali,
HKW = HkKisaWin, VM = ViestiMaali, VW = ViestiWin, VL = ViestiLuenta.

Luku ei anna verkko- tai tietokantavikojen hyödyntämisohjeita, eikä se kuvaa verkkosanomien rakennetta.
Julkaistavien tiedostojen sisältö (kentät, sarakkeet) kuvataan vain sen verran kuin käyttäjä tarvitsee valitakseen
tulostelajin.

## 1. Mitä tapahtuu

### 1.1 Julkaisun kanavat yleiskuvana

| Kanava | Käynnistys | Mihin tulos menee | Ohjelmat |
|---|---|---|---|
| Käsin tehty tuloste | käyttäjä valitsee Tulosteet-toiminnon (GUI: tulostuskaavake, konsoli: tulostusvalikko) | kirjoitin, tekstitiedosto, HTML, XML tai liiton tiedostomuoto | kaikki viisi (viestissä tiedostolajeista osa varmistamatta) |
| Automaattitulostus kirjoittimelle (`autotulostus`) | ohjelma itse, kun sarjaan on tullut riittävästi uusia tuloksia | kirjoitin | HK ja viesti |
| Automaattinen tiedostotulostus (`autofile`) | ohjelma itse, määrävälein tai muutosten perusteella | HTML-, mobiili-HTML- tai XML-tiedosto levylle | kaikki viisi |
| Julkaisun jälkikomento (`KOMENTO=`) | `autofile` käynnistää ulkoisen komentotiedoston onnistuneen kierroksen jälkeen | käyttäjän komentotiedosto (esim. tiedostojen siirto palvelimelle) | kaikki viisi |
| Tiedoston lähetys TCP:llä (`FILETCPy`) | `autofile` kierroksen lopussa | TCP-yhteyden vastapää | kaikki viisi (`TCPSIIRTO`) |
| Reaaliaikainen XML-virta (`YHTEYSy=TCPX...`) | jokainen tuloksen muutos | TCP-yhteyden vastapää, esim. verkkotulospalvelu | kaikki viisi; ks. luku 09 |
| SQL-peili | jokainen kilpailijatietueen tallennus (jatkuva kirjoitus) tai käsin tehty vienti | MySQL-tietokanta | vain HKW ja VW |

### 1.2 Tulostuskohde ja tulostusparametrit

Kaikki tulostus kulkee yhden abstraktion, tulostekanavan (`PRFILE`, `include/tputil.h`), kautta. Kanava on jokin
seuraavista [luettu käsikirjaa varten, ei tarkastettu: `tputilv2/Wrttext.cpp:150-330`; `prno`-arvot]:

- **GDI-kirjoitin** (`prno` 2): sivutettu, fontit ja marginaalit Windowsin kirjoitinajurin mukaan; sivuille
  tulostetaan Windowsin piirtorajapinnalla.
- **Raakakirjoitin** (`prno` 1): rivit lähetetään kirjoittimelle sellaisinaan; sivunvaihtomerkit päättävät sivun.
- **Tekstitiedosto** (`prno` -2, `TextFl`-olio): HTML, XML ja muut tiedostolajit; käyttää tiedoston merkistöä
  (ANSI/ISO-8859-1 tai UTF-8) [todennettu: käyttöohje `hkkisawin/9.2_tiedostotulostus.md`].
- **Raakatiedosto** (`prno` -1) ja **tyhjä** (`prno` 0, tiedoston nimi `NUL`): ei tulostusta.

Jos kirjoittimen avaus epäonnistuu, ohjelma yrittää oletuskirjoitinta ja antaa tarvittaessa ilmoituksen
"Oletuskirjoittimen käyttöönotto ei onnistu" tai "Kirjoitinyhteyden avaaminen ei onnistunut" (`list`,
`Hk/HkTulostus.cpp:8290-8300`) [luettu käsikirjaa varten, ei tarkastettu]. `closeprfile` vapauttaa kanavan, mutta ottaa
osoittimen arvona eikä voi nollata kutsujan kenttää; tästä seuraa osoitinvikoja useassa kutsujassa (T-042; H-149,
H-151, H-152, jäljempänä).

Tulosteen ulkoasun ohjaa `tulostusparamtp`-rakenne: tulostettava (tulokset, lähtöluettelo, väliajat), rajaus
(lopettaneet, tuloksen saaneet, ilmoittautuneet, N parasta), vaiheiden määrä, tiedostolaji, merkistö ja
muotoilu `tulosmuottp`-rakenteesta (yli 60 lippua HTML- ja tulostustyyliin). Tulosluettelomuotoilut ovat
yhteensopivat HkMaalin ja HkKisaWinin kesken [todennettu: käyttöohje `hkkisawin/hkluku9.md`].

### 1.3 Käsin tehty tuloste

Käyttäjä valitsee tulosteen kohteen ja lajin tulostuskaavakkeella (GUI) tai tulostusvalikosta (konsoli). Kohteita
ovat kirjoitin (`P`), tiedosto (`I`, jolla tiedostolaji) sekä HTML (`H`), mobiili-HTML (`M`) ja XML (`X`). Ohjelma
tulostaa sarjoittain: `tulostasarja` käy kilpailijat läpi ja kirjoittaa kunkin rivin valitulle kanavalle
[luettu käsikirjaa varten, ei tarkastettu: `Hk/HkTulostus.cpp:5960-6560`, `:8274-8360`]. Käyttöohjeen mukaiset
tiedostolajit:

- tekstitiedosto: kentät erotettuina (csv), kentät kohdistettuina, jatkuva teksti ilman rivinvaihtoja sarjan sisällä
  (lehdistölle);
- HTML (normaali ja mobiilille tarkoitettu);
- XML: IOF-standardin versio 3.0, ohjelman oma muoto sekä (laajennettu) oma muoto, FIS- ja WRE-tulosten oma
  tiedostomuoto;
- liittojen tulostiedostot: Suunnistusliiton ja Hiihtoliiton tiedosto (SSL, SHL), WebScorer-lähtöluettelo,
  takaa-ajon lähtöluettelo kelloille [todennettu: käyttöohje `hkkisawin/9.2_tiedostotulostus.md`].

Viestiohjelman käyttöohje toteaa, että HTML-tulosteet ovat parhaiten valmiina ja SSL-tiedosto voidaan kirjoittaa
käynnissä olevan päivän osalta; "monet muutkin muodot saattavat toimia, mutta varmistaminen on kesken"
[todennettu: käyttöohje `viestiwin/9.2_tiedostotulostus.md`]. Ohjelmien toimintaa kutakin lajia kohden ei ole
auditoitu; muita kuin tässä luvussa nimettyjä vikoja ei voi sulkea pois (`auditoinnin-yhteenveto.md`).

### 1.4 HTML-tulosteet

HTML-tiedosto syntyy kolmessa vaiheessa: alku (`htmlalku`: otsikko, sivun nimi, kehys), sarjakohtaiset tulosrivit
(`tulostasarja` → `writehtml`), loppu (`htmlloppu`) [luettu käsikirjaa varten, ei tarkastettu: `Hk/HkTulostus.cpp:1440`,
`:6351`, `:7127-7130`]. Ulkoasuun vaikuttavat:

- muotoilut `tulosmuottp`: kaksi erillistä muotoilua, normaali (`filetulosmuot`) ja mobiili (`mobiltulosmuot`),
  yhtä monipuolisesti säädettäviä; mobiilin ero on vain oletusarvoissa [todennettu: käyttöohje
  `hkkisawin/9.2_tiedostotulostus.md` kohta 9.2.2];
- kehystiedostot `HTMLALK?.TXT` ja `HTMLLOP?.TXT` hakemistossa (tiedoston olemassaolo kytkee, ei asetusta)
  [todennettu: ominaisuusvalitsimet.md luku 7];
- erillinen css-tiedosto `tuloslue.css` ja XSL-tyylit XML-tulosteisiin [todennettu: käyttöohje];
- asetukset `HTMLTIT=` (sivun nimi) ja `HTMLOTS=` (otsikko) [todennettu: käyttöohje `hkkisawin/9.7`];
- lisätekstit ja kuvat pääotsikon alle (GUI avaa muokkausikkunan) [todennettu: käyttöohje].

Automaattisessa tiedostotulostuksessa merkistö määräytyy muotoilusta: jos HTML-muotoilun css-valinta on
responsiivinen (`CSS_RESP`), tiedosto kirjoitetaan UTF-8:na, muuten ANSI-merkistössä [luettu käsikirjaa varten, ei
tarkastettu: `Hk/HkTulostus.cpp:7101-7104`].

### 1.5 XML-tulosteet ja IOF 3.0

**HK.** Tulostuskaavakkeessa XML-tiedostolaji valitaan seuraavista (koodin kirjaintunnukset suluissa):
IOF 3.0 -tulosluettelo (`I`), WRE (IOF ilman seurakenttää, `J`), ohjelman oma XML (`O`), laajennettu oma XML
(`L`) ja FIS (`F`) [luettu käsikirjaa varten, ei tarkastettu: `Hk/HkTulostus.cpp:8326-8350`;
`Hk/HkDef.h:357`]. IOF 3.0 -tulosluettelon kirjoittavat `xmlIOF30ots` ja siihen liittyvät funktiot
(`Hk/HkXml_IOF30.cpp:952-1100`); tiedoston `creator`-kentässä on kiinteästi ohjelman nimi "HkKisaWin" ja
versio, vaikka tiedosto olisi tehty HkMaalilla [luettu käsikirjaa varten, ei tarkastettu: `:966-968`].

Automaattinen XML-tiedosto (`kohde = 'X'`) kirjoitetaan HK:ssa tulostusparametrilla, jonka XML-lajin tunnusta ei
aseteta missään `autofile`n polussa (haettu kaikista `xmlstd`-asetuksista), joten oletuksena syntyy
**ohjelman oma XML-muoto tyylitiedostoviittauksineen**, ei IOF 3.0 [päätelty: `Hk/HkTulostus.cpp:7140-7141`,
`Hk/HkXmlTulokset.cpp:122-168`; luettu käsikirjaa varten, ei tarkastettu]. IOF 3.0 -tulosluettelon saa siis
HK:sta käsin tehdyllä tulosteella; automaattisesti sitä ei ole todennettu syntyvän.

Monivaiheisessa kilpailussa HK:n automaattinen XML-tiedosto kirjoitetaan kahdesti: ensin vaiheen tulokset, sitten
yhteistulokset (`yht`-kierros) [luettu käsikirjaa varten, ei tarkastettu: `Hk/HkTulostus.cpp:7281-7314`].

**Viesti.** Viestissä XML-kirjoitus on `Juk/XMLtulos.cpp`:ssä. Se tuntee kolme omaa tunnisteistoa (lyhyt,
pitkä, IOF-tyyppinen) ja valitsee IOF 3.0 -tulosluettelon tulostusvalintojen bitillä (`optiot & 0x00400000`);
kirjoitus kutsuu `Juk/VXml_IOF30.cpp`:n funktioita [luettu käsikirjaa varten, ei tarkastettu: `Juk/XMLtulos.cpp:185-290`]. Mistä
GUI:n valinnasta bitti asetetaan, ei ole luettu. Vanha `IOF`-valitsimen takana oleva `iofots`-koodi on kuollutta
koodia (valitsinta ei määritellä missään projektissa) [todennettu: `rekisteri/valitsimet.csv`, ominaisuusvalitsimet.md
luku 3.4].

**Viestin XML-tuloste ei toimi kahdessa tilanteessa.** (1) Viestin automaattinen XML-tiedosto (asetus `XML=`)
sisältää vain loppuosan, koska tuloksia kirjoittava kutsu (`writexml`) on kommentoitu pois, ja ohjelma jättää
tulostekanavan osoittimen vapautettuun muistiin (H-158). (2) Viestin konsolin tulostusvalikon X)ML-valinta kirjoittaa
tiedoston ilman XML-esittelyä ja juurielementtiä, koska `XML`-valitsinta ei määritellä missään projektissa; sarjakohtaiset
osat kirjoitetaan, mutta niiden tunnisteita ei (H-172). ViestiWinin automaattitulostusikkuna tarjoaa vain HTML-muodon,
joten ViestiWinissä XML-automaatti valitaan asetuksella [todennettu: vikakortit H-158, H-172; käyttöohje viestille
`9.5`].

### 1.6 Automaattitulostus kirjoittimelle

Automaattitulostus kirjoittimelle on erillinen toiminto, jota ei saa käyttää yhtä aikaa tiedostojen
automaattitulostuksen kanssa [todennettu: käyttöohje `hkkisawin/9.6`, `9.7`]. Ohjelma pitää jokaiselle sarjalle
vertailulukua: jokainen uusi tulos tai korjaus kasvattaa sitä yhdellä, tulosten kirjoitus nollaa sen, ja
valitun aikavälin (oletus minuutti) välein luku kerrotaan 1,5:llä; kun sarja valmistuu, luku kasvaa
1001:llä. Jos jokin vertailuluku ylittää kynnysarvon (oletus 10), ohjelma tulostaa sarjan, jolla luku on suurin.
Kynnysarvo yli 16 000 voi jättää sarjan pysyvästi tulostamatta, ja yli 24 000 estää automaattitulostuksen
kokonaan [todennettu: käyttöohje `hkkisawin/9.6_automaattinen_tulostus.md`]. Käynnistys tulostuskaavakkeen
valinnasta tai HK:n asetuksella `AUTO=x/k/a/mmmm` (laajuus, kynnys, aika-askel, maalit; aika-askel luetaan sekunteina ja hyväksytään vain arvoilla 6-999, muuten jää oletus 60 s, `Hk/HkTulostus.cpp:6794-6795`, `Hk/HkIV.cpp:53`; kynnyksen oletus 10, `Hk/HkGlobals.cpp:278`); viestin asetustiedoston
jäsennin ei tunne avainta `AUTO=` (haettu merkkijonolla `AUTO` `Juk/VInit.cpp`:stä) [luettu käsikirjaa varten, ei
tarkastettu: `Hk/HkInit.cpp:1061`; `Hk/HkTulostus.cpp:969-1060`, `:6764-6830`].

Automaattitulostus tulostaa kaikki valitut maalit, ja sitä ohjataan kilpailijatietueen tallennuksessa
(`tallenna`) kasvatettavalla laskurilla (`prior`), jonka kasvatus riippuu siitä, että tulos tai korjaus on muuttunut
[luettu käsikirjaa varten, ei tarkastettu: `Hk/HkDat.cpp:1324-1340`]. HKW:ssa ohjelma ottaa `autotul`- ja `print`-
lukon tulostuksen ajaksi; valinta "Jatkuva sivutuloste" ottaa samat lukot tallennuksen aikana päinvastaisessa järjestyksessä kuin
automaattitulostus, joten kaksi säiettä voi jäädä odottamaan toistensa lukkoa (H-091). Konsolin kehote
("Lopetetaanko automaattinen tulostus") odottaa näppäintä lukkojen ollessa varattuina (T-032).

### 1.7 Automaattinen tiedostotulostus (`autofile`)

`autofile` on ajastettu tehtävä, joka kirjoittaa sarjojen HTML-, mobiili-HTML- tai XML-tiedoston levylle.
Käyttäjälle se näkyy GUI:n automaattitiedostokaavakkeena (`cbHk/UnitAutoFile.cpp`) tai asetuksina
`HTML=`, `MHTML=` ja `XML=` [todennettu: käyttöohje `hkkisawin/9.7_automaattinen_tiedostotulostus.md`;
`Hk/HkInit.cpp:978-1010`].

**Asetukset (HK).** Käyttöohjeen mukaan: `HTML=väli` (tiedostot määritetään `AUTOFILE.LST`:ssä), `HTML=nimi/väli`
(yksi yhdistetty tiedosto), `HTML=nimi/väli/S` (sarjakohtaiset tiedostot, ja `S` rajaa tiedostoon vain
muuttuneet sarjat); `MHTML=` kirjoittaa mobiilimuodossa; `XML=` XML-muodossa; `HTMLTIT=`, `HTMLOTS=` sivun nimi ja
otsikko; `KOMENTO=` ulkoinen komento tulostuksen jälkeen. GUI-kaavakkeella valitaan lisäksi tiedostolaji
(HTML, mobiili, XML), väli sekunteina, yksi vai erilliset tiedostot, vaiheet, XML:n väliaikalaji (emit-väliajat,
muut), komento ja "vain muuttuneet" [todennettu: käyttöohje; luettu käsikirjaa varten, ei tarkastettu:
`cbHk/UnitAutoFile.cpp:40-100`].

**Mikä käynnistää kierroksen.** HKW:ssa taustasäie `LukumaaratTh` kutsuu `tarkAutoTulostus` (`cbHk/WinHk.cpp:1795-1830`),
HKM:ssa konsolin päälooppi (`Hk/HkIV.cpp:3676-3685`), viestissä vastaavasti `Juk/VIv.cpp:2443` ja `ViestiWin/UnitMain.cpp:520`.
Kierros ajetaan aina, kun asetettu väli on kulunut (`afvali`). Jos `autofile` palauttaa virheen (tiedoston avaus
epäonnistuu tai edellisen TCP-lähetyksen tulostekanava on vielä auki), seuraava yritys tehdään 50 sekunnin kuluttua
[luettu käsikirjaa varten, ei tarkastettu; HK: `Hk/HkIV.cpp:3676-3685`].

**Muuttuneiden sarjojen rajaus.** Kun valinta "vain muuttuneet" (`/S`, GUI: "Vaadi muutos") on päällä, `autofile` ohittaa sarjan,
jonka muutoslaskuri (`fprior`) on nolla, ja nollaa laskurin ennen tiedoston avausta. Siksi epäonnistunut kirjoitus pudottaa sarjan
päivityksen: seuraavalla kierroksella laskuri on nolla eikä sarjaa kirjoiteta uudelleen, ennen kuin siihen tulee uusi muutos
(H-147). Ohjelman käynnistyksen tai katkon jälkeen kaikki sarjat saa kirjoitettua poistamalla rastin [todennettu: käyttöohje;
H-147 rekisteririvi; luettu käsikirjaa varten, ei tarkastettu: `Hk/HkTulostus.cpp:7264-7266`]. HK:ssa laskuria kasvattaa
`tallenna`, mutta vain jos joko ulkoinen komento on annettu ja kierrosväli on asetettu (ja kierros ei ole "kaikki sarjat"),
tai kyseisen maalin automaattitulostus kirjoittimelle on päällä, ja lisäksi tulos tai korjaus on muuttunut
(`Hk/HkDat.cpp:1322-1336`) [luettu käsikirjaa varten, ei tarkastettu]; viestissä laskuria kasvatetaan vain, kun maalin
automaattitulostus on päällä (`Juk/vdat.cpp:821-824`) [luettu käsikirjaa varten, ei tarkastettu]. Ks. luvun 7 ehdokas.

**`AUTOFILE.LST`.** Listatiedosto (`AUTOFILE.LST` kilpailuhakemistossa) määrää, mitkä sarjat ja pisteet kirjoitetaan
mihinkin tiedostoon. HK:n rivit ovat muotoa `sarja piste tiedosto` tai `html=tiedosto piste sarjaluettelo`
(mobiili `mhtml=`); viestissä `sarja osuus piste`. Pisteen numero 0 on maali tai vaihto. Ilman listaa HK kirjoittaa
yhden yhdistetyn tulosteen tai sarjakohtaiset tiedostot asetusten mukaan [todennettu: käyttöohje; ominaisuusvalitsimet.md
luku 7]. Mitä HK:n `autofile` tekee sarjakohtaisessa HTML-tilassa ilman listatiedostoa, ei ole selvitetty (kohta 7).

**Viestin ja HK:n automaattitiedoston koodi on eri koodia.** Viestin `autofile` (`Juk/VTulostus.cpp:7085-7330`) ja HK:n
(`Hk/HkTulostus.cpp:7066-7355`) ovat erilliset funktiot. HK:n funktio käännetään HK-ohjelmiin (HKM, HKW) ja viestin funktio viestiohjelmiin (VM, VL, VW), kun `AUTOFILE` on
määritelty; HKW:ssa `AUTOFILE` johdetaan `TCPSIIRTO`-valitsimesta (T-045) [todennettu:
`rekisteri/valitsimet.csv`; ominaisuusvalitsimet.md luku 7].

**Mitä kierroksen lopussa tapahtuu.**

1. Tulostekanava suljetaan, ja jos `FILETCPy` on annettu, kanava jätetään avoimeksi ja lähetetään TCP-yhteydelle
   omassa säikeessä (`sendtcpfile`, `_beginthread`); säie sulkee kanavan lähetyksen jälkeen.
2. Jos jokin sarja kirjoitettiin ja komento on annettu (`KOMENTO=`, GUI: "Jatko"), ohjelma ajaa komennon
   (`system`, `>NUL`) ja odottaa sen päättymistä [luettu käsikirjaa varten, ei tarkastettu: `Hk/HkTulostus.cpp:7330-7350`].
   Käyttöohje vaatii, että komento toimii ilman käyttäjää ja ettei se tulosta näytölle. Ohjelma ajaa komennon myös silloin,
   kun tiedoston kirjoitus epäonnistui (`virheenkasittely.md` luku 12.7, ks. luku 13).
3. `autofile`:n kierroksen ajan on `autotul`-lukko päällä.

**Tulostiedoston avaus tyhjentää tiedoston.** Ohjelma avaa julkaistavan tiedoston kirjoitustilassa, jolloin vanha sisältö
häviää heti (ei väliaikaistiedostoa eikä uudelleennimeämistä). Jos kirjoitus epäonnistuu (levy täynnä, tiedosto lukittu),
ohjelma ei havaitse sitä kirjoituksen aikana, koska tulosrivien kirjoitusfunktiot vertaavat paluuarvoa väärään
virhearvoon (H-147). Tuloksena julkaistu tiedosto voi olla tyhjä, osittainen tai vanha ilman ilmoitusta. Jos lukija
(esimerkiksi tiedostoja kopioiva ohjelma) pitää tiedostoa auki, avaus epäonnistuu ja ohjelma antaa kaksi ilmoitusta
taustasäikeen pysyessä `autotul`-lukon alla [todennettu: virheenkasittely.md luku 12.4]. MSVC-ohjelmissa lukija saa
jakorikkomuksen eikä osittaista sisältöä; bcc32-ohjelmissa avaustilaa ei ole mitattu (Q92).

### 1.8 Julkaisu palvelimelle: komento, TCP-tiedosto ja XML-virta

Ohjelma ei itse siirrä tiedostoja WWW-palvelimelle: aiemmat sisäänrakennetut FTP- ja SFTP-siirrot (`AutoJakelu.xml`,
`AUTOJAKELU`) on poistettu käytöstä, ja tilalla on `KOMENTO=` ulkoinen komentotiedosto. Käyttöohje kuvaa siihen
sopivan `rsync`+SSH-mallin; se on käyttöohjeen suositus, ei ohjelman ominaisuus [todennettu: käyttöohje
`hkkisawin/9.7`, `viestiwin/9.5`, `hkkisawin/liite_10._komentotiedoston_automaattinen_suorittaminen.md`].

Kaksi muuta TCP-pohjaista reittiä ovat:

- **Tiedoston lähetys** (`FILETCPy=`): `autofile`n kirjoittama tiedosto lähetetään TCP-yhteyden vastapäähän. Lähetyssäie
  vapauttaa tulostekanavan lähetyksen jälkeen, mutta ohjelma ei nollaa osoitinta, joten seuraava automaattitulostus lukee
  vapautettua muistia (HK, H-151). Viestissä `fileyht`-muuttuja asetetaan jo ensimmäisestä TCP-yhteydestä
  riippumatta yhteyden tyypistä, ja saman osoittimen kaksoisvapautus tapahtuu kierroksella, jolla TCP-yhteys on auki
  (H-149) [todennettu: vikakortit H-149, H-151; virheenkasittely.md luku 12.1].
- **Reaaliaikainen XML-virta** (`YHTEYSy=TCPX...`, valinnat `XMLSARJA=`, `TCPTULOS=`, `TCPAUTOALKU`): jokainen tuloksen muutos
  lähtee XML-muotoisena ulkoiselle palvelulle; tämä on tiedonsiirtoyhteys, ja sen jono, uudelleenlähetys ja kuittaus on
  kuvattu luvussa 09. Yhteyden ensimmäisessä käytössä ohjelma ajaa automaattisen uusinnan (H-030).

Käyttöohjeen mukaan `TCPAUTOALKU` käynnistää tiedostojen automaattisen TCP-siirron ohjelman käynnistyttyä
[todennettu: käyttöohje `hkkisawin/hkliite1.md`]. UDP-broadcast-tulosvirta (`broadcasttulos`) on HK:ssa kuollutta koodia (T-013).

### 1.9 SQL-peili

MySQL-peili on valinnainen rinnakkainen kopio kilpailijatiedoista, jota on vain GUI-ohjelmissa HKW ja VW. `KILP.DAT` ja
ohjelman muistitaulukko pysyvät ensisijaisina; SQL:stä luetaan takaisin vain käyttäjän käynnistämässä tuonnissa
(valinta 6, `lue_SQL`), joka tekee SQL:stä tuhoavasti ensisijaisen (H-169) [todennettu: riippuvuudet.md luku 11].
Konsoliohjelmissa ei ole toimivaa SQL-koodia: HKM jäsentää `SQL*`-avaimet mutta mikään ei käytä niitä, ja viestin
`SQL*`-avaimet jäsennetään vain GUI:ssa [todennettu: `todisteet/c10/raportit/osaB-tietokanta.md` luku 1;
`rekisteri/asetusavaimet.csv`].

**Mitä kirjoitetaan ja milloin.** Jokainen kilpailijatietueen tallennus (`PUTREC`) laittaa tietueen numeron peilin
jonoon (`kirjKilpSQL`); erillinen kirjoitussäie (`kirjSQLTh`) ottaa numeroita jonosta ja kirjoittaa tietueen
tietokantaan (`kirjSQLrivi`): uusi rivi `INSERT`, olemassa oleva `UPDATE` erotuksena nykyiseen tietokantariviin. Jono on
muistissa (10 000 paikkaa); täyttyessään se ylikirjoittaa vanhimman (H-014). Jonon osoittimia (`head`, `tail`) päivittävät tuottajat ja kuluttaja ilman lukkoa, ja kaksi kuluttajaa voi käynnistyä (H-100). Jos valinta "Varmista jono" (`CBVarmista`)
on päällä, jonon sisältö kirjataan myös tiedostoon `SQLjonoVarm.dat` (luku 03), joka avataan lisäystilassa ja kasvaa ja sekoittuu päivityksessä (H-043) [todennettu: riippuvuudet.md luku 11,
tiedostomuodot.md; luettu käsikirjaa varten, ei tarkastettu: `cbHk/UnitMySQL.cpp:636-760`]. Tietokantataulut:
HK `osanotot`, `osottopv`, `tulos`; viesti `vosanotot`, `osuudet`, `vtulos`. Luontilauseet ovat tiedostoissa `kilpsql.txt` ja
`vkilpsql.txt`, eivät koodissa; kenttävastaavuudet ovat raportissa `osaB-tietokanta.md` [todennettu].

**Käyttöönotto.** Asetukset `SQLHOST`, `SQLDATABASE`, `SQLUSER`, `SQLPASSWORD`, `SQLSTART` (yhteys heti käynnistyksessä) ja
`SQLTALLENNUS` (jatkuva kirjoitus) tai GUI-valikko Tiedostot/MySQL ja valintaruutu "Jatkuva tiedonsiirto". Yhteys avataan
`TSQLConnection`-komponentilla (dbExpress → `dbxmys.dll` → MySQL-asiakaskirjasto); parametrit rakennetaan koodissa joka
avauksella, eikä portti, merkistö, aikakatkaisu eikä salaus ole asetettavissa. Tunnukset ovat asetustiedostossa
selväkielisinä (T-043) [todennettu: riippuvuudet.md luku 11].

**Vienti ja tuonti.** Valinta Vienti→SQL (valinta 3) kirjoittaa kaikki kilpailijat; jos jatkuva kirjoitus ei ole päällä,
vienti ajetaan pääsäikeessä `tall`-lukko päällä. SQL-poikkeus jättää lukon päälle (H-162). Tuonti (valinta 6) korvaa
paikalliset tietueet SQL:n riveillä, myös poistetuilla tai vajailla (H-169) [todennettu: vikakortit H-162; riippuvuudet.md
luku 11].

**Mitä kaikki voi mennä pieleen.** Yksikin SQL-virhe kirjoitussäikeessä (palvelinkatkos, heittomerkki nimessä,
virheellinen lause) lopettaa koko ohjelman (H-160); useat päivityslauseet ovat virheellisiä jo tavallisella datalla (H-161);
vaiheen vaihto pysäyttää peilin kirjoituksen ilman ilmoitusta (H-014, vain HK); käynnistyksessä `SQLSTART`-virhe ohittaa
loput alustuksesta (H-163); SQL-viennin lukko jää päälle virhetilanteessa (H-162). Julkaisu-ZIP ei sisällä MySQL-tiedostoja
(`dbxmys.dll`, `libmysql.dll`, `kilpsql.txt`, `vkilpsql.txt`), joten peili ei toimi paketista asennetulla koneella ilman
niiden erillistä hankintaa (H-165). Kilpailudata ei tuhoudu SQL-viasta itsestään, ellei ohjelma pääty (H-160) tai
tuontia käytetä (H-169) [päätelty: riippuvuudet.md luku 11; vikakortit].

### 1.10 Mitä käyttäjä näkee

- **Tulosteessa** näkyy ohjelman tuottama sisältö; virhettä tiedoston kirjoituksessa ei ilmoiteta (H-147), joten käyttäjä
  huomaa vian vasta tarkistamalla tiedoston (koko, aikaleima, sisältö).
- **Automaattitulostuksen tila:** GUI:n tilanäytössä näkyy korkein vertailuluku (`autostatus`); konsolin tilarivillä
  vastaava luku [luettu käsikirjaa varten, ei tarkastettu: `Hk/HkTulostus.cpp:996-1002`].
- **Ilmoitukset** tulevat modaalisina ikkunoina, joita ei voi ohittaa ilman käyttäjää; taustasäie jää odottamaan
  `autotul`-lukon alla (T-032).
- **SQL:** peilin tila näkyy MySQL-lomakkeen jonolaskurissa (`EdtJono`); kirjoitusvirheestä ohjelma voi päättyä (H-160).

### 1.11 Tulostuskoodin sisäiset viat

Tulostuksen rivinmuodostuskoodissa on lisäksi ohjelmointivirheitä, jotka eivät riipu julkaisukanavasta vaan siitä, mitä ja
millä asetuksilla tulostetaan. Ne on kuvattu omissa korteissaan; tässä on vain yhden rivin kuvaus ja ohjelmat
[todennettu: rekisteririvit ja vikakortit, ohjelmat kortin "Ohjelmat"-rivin mukaan]:

| Havainto | Ohjelmat | Mitä tulostuksessa tapahtuu |
|---|---|---|
| H-001 | VW, VM, VL | tekstitulosteen joukkue- ja osuusrivi kirjoittaa 120 tavua 40-tavuiseen puskuriin, kun osuudella on ampumapaikkoja |
| H-059 | HKW, HKM, VM, VL | ajat tiedostoon (`AjatTiedostoon`) ja viestin konsolin maalitulostus: rajaamaton muotoilu pinopuskuriin; tiedoston avausvirhe päättää MSVC-ohjelman |
| H-062 | kaikki viisi | `sendwline`: tiedostohaara kirjoittaa alustamatonta muistia, kun kirjoitinta ei saada auki |
| H-063 | VW, VM | hajontatuloste: `fclose(NULL)` päättää ViestiMaalin, kun `HAJONNAT.LST` puuttuu |
| H-068 | HKM | konsolin aikajonotuloste: alustamaton osoitin, kun ensimmäisellä rivillä ei ole aikaa |
| H-075 | VM, VL | viestin konsolin maalitulostus: alustamaton fonttiosoitin muilla kuin kahdella kirjoitintyypillä |
| H-076, H-130 | VW, VM, VL | sijoituspuskuri (`ossijat`) ylivuotaa tavallisellakin viestillä, jossa on väliaikoja; joukkuemäärät eivät päivity sarjan vaihdossa |
| H-087 | VW | Emit-osuustuloste HTML:nä (`EmitHtmlOsuus`) ei vapauta `print`- ja `rastiva`-lukkoja |

Vikojen ilmenemisen ehdot ja seuraukset ovat korteissa; tämän luvun toimintokytkennässä ne liitetään kohtaan "Tulostuskoodi:
rivien muodostus". Luettelo ei ole tyhjentävä, koska tulostuskoodi (`Hk/HkTulostus.cpp`, `Juk/VTulostus.cpp`) on auditoitu vain
osin (`rekisteri/tiedostot.csv`).

## 2. Erot ohjelmien välillä

| Asia | HkMaali | HkKisaWin | ViestiMaali | ViestiWin | ViestiLuenta |
|---|---|---|---|---|---|
| Käsin tehty tuloste | konsolin valikko | GUI-kaavake | konsolin valikko | GUI-kaavake | konsolin valikko |
| Tiedostolajit | HTML, XML, liittotiedostot, tekstit | sama | HTML parhaiten valmis; muut varmistamatta | sama | sama |
| IOF 3.0 -tulosluettelo | kyllä (käsin) | kyllä (käsin) | `XMLtulos`-polku, valinta konsolissa rikki (H-172) | `XMLtulos`-polku; valintaa ei luettu | sama kuin VM |
| Automaattitulostus kirjoittimelle | kyllä (`AUTO=` tai valikko) | kyllä | valikko; `AUTO=` ei jäsennetä | valikko | valikko |
| `autofile` | kyllä (HK:n oma koodi) | kyllä | kyllä (viestin oma koodi) | kyllä | kyllä |
| Automaattinen XML | oma XML-muoto, vaiheessa 2+ kaksi kierrosta (H-150) | sama | vain loppuosa (H-158) | vain loppuosa (H-158) | vain loppuosa (H-158) |
| Tulostekanavan osoittimen vika | H-151 (TCP), H-152 vain GUI | H-151, H-152 | H-149 | H-149 | H-149 |
| `KOMENTO=` | kyllä | kyllä | kyllä | kyllä | kyllä |
| `FILETCPy` | kyllä | kyllä | kyllä | kyllä | kyllä |
| SQL-peili | ei toimintaa (avaimet jäsennetään) | kyllä | ei | kyllä | ei |
| Vaiheen vaihto ja SQL | – | peili pysähtyy (H-014) | – | ei testiä vaiheenvaihdosta | – |

[todennettu: vikakortit H-014, H-149–H-152, H-158, H-172; `rekisteri/valitsimet.csv`; riippuvuudet.md luku 11].
**Tulostekanavan vioista.** HK:n ja viestin `autofile`-koodi on eri koodia: H-149 ja H-158 koskevat viestiä, H-150–H-152 HK:ta.
H-149:n vaikutus on viestin konsoliohjelmissa (ViestiMaali, ViestiLuenta) ohjelman päättyminen ensimmäisellä automaattisella
HTML-tulosteella, kun tiedosto on tilassa "erilliset ilman listatiedostoa", ja ViestiWinissä (bcc32) sama koodi ajetaan
joka kierros, mutta vaikutusta ei ole mitattu (Q91) [todennettu: vikakortti H-149; virheenkasittely.md luku 12.1].

## 3. Asetukset ja käännösvalitsimet

### 3.1 Asetukset

| Avain | Ohjelmat | Vaikutus |
|---|---|---|
| `HTML=väli`, `HTML=nimi/väli[/S]`, `MHTML=...` | kaikki (`AUTOFILE`) | automaattinen HTML- tai mobiili-HTML-tulostus; välin yksikkö sekuntia; HK:ssa oletusväli 60 s, kun tiedoston nimi annetaan ilman väliä ja väliä ei ole annettu aiemmin (`Hk/HkInit.cpp:979-999`; viestin jäsentimen oletusta ei ole tarkistettu) |
| `XML=...` | kaikki (`AUTOFILE`) | automaattinen XML-tulostus; viestissä vain loppuosa (H-158) |
| `HTMLTIT=`, `HTMLOTS=` | kaikki (`AUTOFILE`) | sivun nimi ja otsikko |
| `AUTO=x/k/a/mmmm` | HK | automaattitulostus kirjoittimelle |
| `KOMENTO=` | kaikki (`AUTOFILE`) | ulkoinen komento automaattitulostuksen jälkeen |
| `FILETCPy=...`, `XMLSARJA=`, `TCPAUTOALKU` | kaikki (`TCPSIIRTO`) | tiedoston lähetys TCP:llä; XML-virran sarjarajaus |
| `SQLHOST`, `SQLDATABASE`, `SQLUSER`, `SQLPASSWORD`, `SQLSTART`, `SQLTALLENNUS` | HKW, VW (avaimet jäsennetään HKM:ssäkin, ei vaikutusta) | SQL-peili |
| `CBVarmista` (valintaruutu) | HKW, VW | peilin jonon varmistustiedosto `SQLjonoVarm.dat` |
| `ETSQL`, `ETHOST`, ... | HKW, VW, HKM, VM | emiTag-palvelinhaku (luku 10), ei tulosten julkaisu |

Tiedostokytkimet (tiedoston olemassaolo kytkee toimintoa): `AUTOFILE.LST` (sarjakohtaiset tiedostot), `HTMLALK?.TXT` ja
`HTMLLOP?.TXT` (HTML-kehys), `tuloslue.css` (css), `kilpsql.txt`, `vkilpsql.txt` (SQL-taulujen luonti, käyttäjän ajettavaksi)
[todennettu: ominaisuusvalitsimet.md luku 7; käyttöohje].

### 3.2 Käännösvalitsimet

| Valitsin | HKM | VM | VL | HKW | VW | Merkitys |
|---|---|---|---|---|---|---|
| `AUTOFILE` | kyllä | kyllä | kyllä | johdettu `TCPSIIRTO`sta | kyllä | automaattinen tiedostotulostus |
| `TCPSIIRTO` | kyllä | kyllä | kyllä | kyllä | kyllä | TCP-tiedostonsiirto; HK:ssa johtaa `AUTOFILE`n (T-045) |
| `XML` (viestin konsolin tulosvalikko) | ei käytössä | ei määritelty | ei määritelty | ei käytössä | ei käytössä | X)ML-lohko ei käänny mukaan (H-172) |
| `IOF` | ei määritelty | ei määritelty | ei määritelty | ei määritelty | ei määritelty | vanha IOF-tulostus kuollutta koodia (`rekisteri/valitsimet.csv`) |
| `SQLDATA` | ei määritelty | ei määritelty | ei määritelty | ei määritelty | ei määritelty | viestin `mysql_*`-lohko kuollutta koodia |
| `EMITHTTP`, `EMITAG` | kyllä | kyllä | kyllä | kyllä | kyllä | emiTag-palvelinhaku (luku 10) |

SQL-peilin koodi on GUI-projektien lähdetiedostoissa (`cbHk/UnitMySQL.cpp`, `ViestiWin/UnitMySQL.cpp`) eikä sitä ohjaa
käännösvalitsin; konsoliprojekteissa ei ole vastaavaa lähdettä [todennettu: osaB-tietokanta.md luku 1]. Projektien määritykset:
`VS/Hk/HkMaali/HkMaali.vcxproj`, `VS/V/JukMaali520.vcxproj`, `VS/V/JukLuenta520.vcxproj`, `RADStudio10/HkKisaWin.cbproj`,
`RADStudio10/ViestiWin.cbproj` (tarkistettu 2026-11-27).

## 4. Vaihtoehtoiset polut ja virhetilanteet

| Tilanne | Mitä tapahtuu | Lähde |
|---|---|---|
| Julkaistavan tiedoston kirjoitus epäonnistuu (levy täynnä, lukittu) | tiedosto jää tyhjäksi, osittaiseksi tai vanhaksi, ei ilmoitusta (H-147); `KOMENTO` ajetaan silti | [todennettu: virheenkasittely.md 12.4, 12.7] |
| Lukija pitää tulostiedostoa auki | avaus epäonnistuu, kaksi ilmoitusta, vanha tiedosto säilyy; taustasäie odottaa `autotul`-lukon alla | [todennettu: virheenkasittely.md 12.4] |
| Viestin automaattinen HTML, erilliset tiedostot ilman `AUTOFILE.LST` | konsoliohjelmat päättyvät ensimmäisellä kierroksella (H-149; MSVC Release testikehyksessä); ViestiWinissä vaikutus mittaamatta (Q91) | [todennettu: vikakortti H-149] |
| Viestin automaattinen HTML, yksi tiedosto tai TCP-yhteys auki | kierroksella voi tapahtua vapautetun muistin luku tai kaksoisvapautus (H-149) | [todennettu: virheenkasittely.md 12.1] |
| Viestin automaattinen XML | vain loppuosa (H-158) | [todennettu: vikakortti H-158] |
| HK:n automaattinen XML vaiheessa 2+ | vuoto jokaisella kierroksella; jos `AUTOFILE.LST` on olemassa, tiedostovirrat loppuvat (MSVC ≈ 508 kierroksen, bcc32 alle 45 kierroksen jälkeen oletusvälillä) ja kaikki muut tiedostoavaukset epäonnistuvat (H-150; seuraus GUI:ssa päätelty, Q95) | [todennettu: virheenkasittely.md 12.5] |
| HK:n automaattitulostus TCP-lähetyksen jälkeen | lukee vapautettua muistia seuraavalla kierroksella; sarjakohtaisessa tilassa kaksoisvapautus (H-151); jos lähetyssäikeen käynnistys epäonnistuu, `autofile` palaa jatkossa hiljaa | [todennettu: vikakortti H-151] |
| Tuloskirjoittimen vaihto GUI:ssa kesken automaattitulostuksen | automaattitulostus käyttää vapautettua tulostekanavaa (H-152; ei toistettu) | [todennettu: vikakortti H-152] |
| Viestin konsolin X)ML | tiedosto ilman XML-esittelyä ja juurielementtiä (H-172) | [todennettu: vikakortti H-172] |
| SQL-virhe kirjoitussäikeessä | koko ohjelma päättyy (H-160); tietokoneen puskuroimattomat tiedostot (esim. emit-tiedosto) jäävät kirjoittamatta | [todennettu: vikakortti H-160] |
| SQL-päivityslause virheellinen | virhe tavallisella datalla; peili jää vanhaksi tai ohjelma päättyy (H-161) | [todennettu: vikakortti H-161] |
| SQL-vienti keskeytyy poikkeukseen | `tall`-lukko jää päälle; tietoa vastaanottavat työsäikeet pysähtyvät, käyttöliittymä näyttää toimivan (H-162) | [todennettu: vikakortti H-162] |
| `SQLSTART` ilman palvelinta, ajuria tai kirjastoa | käynnistyksen loppuosa ohitetaan, mm. emit-käsittelyn käynnistys (H-163) | [todennettu: vikakortti H-163] |
| Julkaisu-ZIP:stä asennettu GUI-ohjelma, MySQL käyttöön | `dbxmys.dll` ja `libmysql.dll` puuttuvat paketista (H-165) | [todennettu: vikakortti H-165] |
| Vaiheen vaihto, HKW ja jatkuva SQL-kirjoitus | peilin kirjoitussäie lopettaa eikä käynnisty uudelleen; poistoja ei viedä SQL:ään (H-014) | [todennettu: vikakortti H-014] |
| SQL-tuonti | korvaa paikalliset tietueet (H-169) | [todennettu: riippuvuudet.md luku 11] |
| ViestiWinin HTML-osuustuloste (Emit, `EmitHtmlOsuus`) | ei vapauta `print`- ja `rastiva`-lukkoja, jokainen tulostettu joukkue vuotaa yhden rekursiotason (H-087) | [todennettu: rekisteririvi H-087] |

HK:n `autofile`:n kierroksen virheet eivät pysäytä ohjelmaa: palautusarvo 1 (avaus epäonnistui tai edellinen TCP-lähetys kesken)
ajoittaa uuden yrityksen 50 sekunnin päähän [luettu käsikirjaa varten, ei tarkastettu: `Hk/HkIV.cpp:3676-3685`,
`cbHk/WinHk.cpp:1810-1822`].

## 5. Tunnetut viat tässä toiminnossa

<!-- generoitu:viat alku (tietopohja.py lohkot; älä muokkaa käsin) -->
| Toiminto | Havainto | Tila | Prioriteetti | Kuvaus |
|---|---|---|---|---|
| Käsin tehty tuloste: kirjoitin, tekstitiedosto, HTML (tulostuskaavake ja konsolin tulostusvalikko) | [H-147](../vikakortit/H/H-147.md) | avoin | P3 | Tulos- ja automaattitiedoston kirjoitusvirhettä ei huomata, ja julkaistava tiedosto tyhjennetään ennen kirjoitusta, joten se voi jäädä tyhjäksi tai vajaaksi ilman ilmoitusta |
| Tulostuskoodi: rivien muodostus (tulostasarja, puskurit ja osoittimet) | [H-001](../vikakortit/H/H-001.md) | avoin | P3 | Viestin tekstitulosteen rivi nollaa 120 tavua apumuuttujan jälkeistä muistia, kun osuudelle on määritelty ampumapaikkoja |
| Tulostuskoodi: rivien muodostus (tulostasarja, puskurit ja osoittimet) | [H-059](../vikakortit/H/H-059.md) | avoin | P3 | Aikajonon vienti tekstitiedostoon ylittää rivipuskurin pitkillä nimillä ja seuroilla, ja tiedoston avauksen epäonnistuminen sulkee konsoliohjelman |
| Tulostuskoodi: rivien muodostus (tulostasarja, puskurit ja osoittimet) | [H-062](../vikakortit/H/H-062.md) | avoin | P3 | Tulostus tiedostoon, kun kirjoitinta ei saada auki, kirjoittaa listatiedostoon tekstin sijaan alustamatonta muistia |
| Tulostuskoodi: rivien muodostus (tulostasarja, puskurit ja osoittimet) | [H-063](../vikakortit/H/H-063.md) | avoin | P4 | Kurssihajontatuloste päättää ViestiMaali-ohjelman, kun tiedostoa HAJONNAT.LST ei ole |
| Tulostuskoodi: rivien muodostus (tulostasarja, puskurit ja osoittimet) | [H-068](../vikakortit/H/H-068.md) | avoin | P5 | HkMaalin aikajonon tulostus kirjoittaa alustamattoman osoittimen osoittamaan paikkaan, kun jonon ensimmäisellä rivillä ei ole aikaa |
| Tulostuskoodi: rivien muodostus (tulostasarja, puskurit ja osoittimet) | [H-075](../vikakortit/H/H-075.md) | avoin | P4 | Viestin konsolin aikajonon tulostus lukee alustamattoman kirjasinosoittimen, kun kirjoitin on muu kuin LJETII tai GDI-kirjoitin |
| Tulostuskoodi: rivien muodostus (tulostasarja, puskurit ja osoittimet) | [H-076](../vikakortit/H/H-076.md) | avoin | P3 | Viestin osuussijoitusten laskenta ylittää varaamansa muistin XML-tulosteessa, kun joukkueilla on väliaikoja |
| Tulostuskoodi: rivien muodostus (tulostasarja, puskurit ja osoittimet) | [H-130](../vikakortit/H/H-130.md) | avoin | P3 | Viestissä sarjaa vaihdettaessa joukkuemäärät jäävät vääriksi uudelleenkäynnistykseen asti |
| Emit-osuustuloste HTML:nä (viesti, EmitHtmlOsuus) | [H-087](../vikakortit/H/H-087.md) | avoin | P2 | ViestiWin: emit-tietojen HTML-tulostus tiedostoon jättää tulostuslukot päälle, ja ohjelma jäätyy seuraavan emit-tiedon saapuessa |
| Viestin konsolin tulostusvalikon X)ML-valinta | [H-172](../vikakortit/H/H-172.md) | avoin | P3 | Viestin konsoliohjelman tulostusvalikon X)ML-valinta kirjoittaa tiedoston ilman XML-kehystä, joten tiedosto on lähes tyhjä |
| Automaattitulostus kirjoittimelle (autotulostus, AUTO=) | [H-091](../vikakortit/H/H-091.md) | avoin | P2 | Jos HkKisaWinissä on päällä jatkuva sivutuloste, tallennus ja tulostus voivat jäädä odottamaan toisiaan |
| Automaattinen tiedostotulostus HK:ssa (autofile: HTML, mobiili-HTML, XML, AUTOFILE.LST) | [H-147](../vikakortit/H/H-147.md) | avoin | P3 | Tulos- ja automaattitiedoston kirjoitusvirhettä ei huomata, ja julkaistava tiedosto tyhjennetään ennen kirjoitusta, joten se voi jäädä tyhjäksi tai vajaaksi ilman ilmoitusta |
| Automaattinen tiedostotulostus HK:ssa (autofile: HTML, mobiili-HTML, XML, AUTOFILE.LST) | [H-150](../vikakortit/H/H-150.md) | avoin | P2 | HK:n automaattinen XML-vienti vuotaa tiedostovirran joka kierroksella, ja virtojen loppuessa C-kirjaston tiedostoavaukset epäonnistuvat |
| Automaattinen tiedostotulostus HK:ssa (autofile: HTML, mobiili-HTML, XML, AUTOFILE.LST) | [H-151](../vikakortit/H/H-151.md) | avoin | P2 | HK:n tiedostonsiirtosäie sulkee tulostekanavan jättäen osoitteen, ja seuraava automaattitulostus käyttää suljettua kanavaa |
| Automaattinen tiedostotulostus HK:ssa (autofile: HTML, mobiili-HTML, XML, AUTOFILE.LST) | [H-152](../vikakortit/H/H-152.md) | avoin | P3 | Tuloskirjoittimen vaihto valikosta jättää automaattitulostuksen käyttämään suljettua tulostekanavaa |
| Automaattinen tiedostotulostus viestissä (autofile: HTML, XML, AUTOFILE.LST) | [H-147](../vikakortit/H/H-147.md) | avoin | P3 | Tulos- ja automaattitiedoston kirjoitusvirhettä ei huomata, ja julkaistava tiedosto tyhjennetään ennen kirjoitusta, joten se voi jäädä tyhjäksi tai vajaaksi ilman ilmoitusta |
| Automaattinen tiedostotulostus viestissä (autofile: HTML, XML, AUTOFILE.LST) | [H-149](../vikakortit/H/H-149.md) | avoin | P2 | Viestiohjelman automaattinen HTML-tuloste kaataa konsoliohjelman jo ensimmäisellä kierroksella, kun automaattitulostus on päällä |
| Automaattinen tiedostotulostus viestissä (autofile: HTML, XML, AUTOFILE.LST) | [H-158](../vikakortit/H/H-158.md) | avoin | P3 | Viestin automaattinen XML-julkaisu (asetus XML=) ei kirjoita tuloksia tiedostoon, ja tulostekanavan osoitin jää vapautettuun muistiin |
| Automaattinen tiedostotulostus viestissä (autofile: HTML, XML, AUTOFILE.LST) | [H-106](../vikakortit/H/H-106.md) | avoin | P3 | ViestiWinin automaattitulostuksen tilanäytön kenttiä päivitetään suoraan taustasäikeistä |
| Tiedoston lähetys TCP-yhteydellä (FILETCPy, sendtcpfile) | [H-149](../vikakortit/H/H-149.md) | avoin | P2 | Viestiohjelman automaattinen HTML-tuloste kaataa konsoliohjelman jo ensimmäisellä kierroksella, kun automaattitulostus on päällä |
| Tiedoston lähetys TCP-yhteydellä (FILETCPy, sendtcpfile) | [H-151](../vikakortit/H/H-151.md) | avoin | P2 | HK:n tiedostonsiirtosäie sulkee tulostekanavan jättäen osoitteen, ja seuraava automaattitulostus käyttää suljettua kanavaa |
| SQL-peilin kirjoitus ja jono (kirjKilpSQL, kirjSQLTh, kirjSQLrivi) | [H-014](../vikakortit/H/H-014.md) | avoin | P3 | Vaiheen vaihdon jälkeen SQL-peili lakkaa päivittymästä, eikä ohjelma ilmoita siitä |
| SQL-peilin kirjoitus ja jono (kirjKilpSQL, kirjSQLTh, kirjSQLrivi) | [H-043](../vikakortit/H/H-043.md) | avoin | P3 | SQL-peilin varmistustiedosto kasvaa ja sekoittuu, ja uudelleenkäynnistyksen jälkeen peiliin voi lähteä vääriä tietoja |
| SQL-peilin kirjoitus ja jono (kirjKilpSQL, kirjSQLTh, kirjSQLrivi) | [H-100](../vikakortit/H/H-100.md) | avoin | P4 | SQL-peilin jono päivittyy usealta säikeeltä ilman lukkoa, ja kirjoittajia voi käynnistyä kaksi |
| SQL-peilin kirjoitus ja jono (kirjKilpSQL, kirjSQLTh, kirjSQLrivi) | [H-160](../vikakortit/H/H-160.md) | avoin | P2 | Yksikin tietokantavirhe SQL-peilin kirjoituksessa lopettaa koko ohjelman |
| SQL-peilin kirjoitus ja jono (kirjKilpSQL, kirjSQLTh, kirjSQLrivi) | [H-161](../vikakortit/H/H-161.md) | avoin | P3 | SQL-peilin päivityslauseet ovat osin virheellisiä, ja nimen heittomerkki rikkoo lauseen |
| SQL-peilin vienti ja tuonti (valinnat 3 ja 6) | [H-162](../vikakortit/H/H-162.md) | avoin | P3 | SQL-vienti voi virheeseen pysähtyessään jättää tallennuslukon päälle, jolloin tietoja vastaanottavat säikeet jäävät odottamaan |
| SQL-peilin vienti ja tuonti (valinnat 3 ja 6) | [H-169](../vikakortit/H/H-169.md) | avoin | P3 | SQL-tuonti voi korvata ohjelman omia kilpailijatietoja poistetuilla tai vajailla tiedoilla |
| SQL-peilin käyttöönotto (SQLSTART, SQLHOST, ajuri ja asiakaskirjasto) | [H-163](../vikakortit/H/H-163.md) | avoin | P3 | Jos SQL-yhteys epäonnistuu käynnistyksessä, loput käynnistyksen vaiheet jäävät tekemättä |
| SQL-peilin käyttöönotto (SQLSTART, SQLHOST, ajuri ja asiakaskirjasto) | [H-165](../vikakortit/H/H-165.md) | avoin | P3 | Julkaisupaketista puuttuvat MySQL-ominaisuuden tiedostot ja ohjetiedostot, vaikka käyttöohje lupaa ne toimitukseen |
| SQL-peilin käyttöönotto (SQLSTART, SQLHOST, ajuri ja asiakaskirjasto) | [T-043](../vikakortit/T/T-043.md) | avoin | - | Ohjelman verkko- ja tietokantayhteydet ovat suojaamattomia: ei todennusta, ei salausta, tunnukset selväkielisinä |

Generoitu tiedostosta `kasikirja/toiminnot.csv` ja rekistereistä (tila, prioriteetti); älä muokkaa käsin.
<!-- generoitu:viat loppu -->

## 6. Mihin tieto perustuu

Lähteet:

- **Auditointiraportit:** `doc/arkkitehtuuri/virheenkasittely.md` (luvut 12.1 H-149, 12.4 H-147, 12.5 H-150–H-152, 12.7);
  `doc/arkkitehtuuri/riippuvuudet.md` luvut 11 (tietokanta), 12 (HTTP ja selain), 16; `todisteet/c10/raportit/osaB-tietokanta.md`;
  `doc/arkkitehtuuri/ominaisuusvalitsimet.md` luvut 3.4, 4, 7, 11; `doc/arkkitehtuuri/rekisteri/valitsimet.csv`,
  `rekisteri/asetusavaimet.csv`; `doc/arkkitehtuuri/tiedostomuodot.md` ("SQLjonoVarm.dat", tiedostot).
- **Vikakortit:** H-014, H-149, H-150, H-151, H-152, H-158, H-160, H-161, H-162, H-163, H-165, H-172
  (`doc/tietopohja/vikakortit/H/`); lisäksi rekisteririvit H-043, H-087, H-147, H-169, T-013, T-032, T-042, T-043, T-045.
- **Käyttöohje:** `docs-site/docs/hkkisawin/9.2_tiedostotulostus.md`, `9.6_automaattinen_tulostus.md`,
  `9.7_automaattinen_tiedostotulostus.md`, `hkluku9.md`, `hkliite1.md`, `liite_10._komentotiedoston_automaattinen_suorittaminen.md`;
  `viestiwin/9.2_tiedostotulostus.md`, `9.5_automaattinen_tulostus.md` (vain käyttäjälle näkyvä toiminta ja asetusten nimet).
- **Luettu käsikirjaa varten, ei tarkastettu:** `Hk/HkTulostus.cpp:969-1060` (`autotulostus`), `:5960-6560`, `:6764-6830`,
  `:7030-7355` (`sendtcpfile`, `autofile`), `:8274-8360` (`list`); `Hk/HkXml_IOF30.cpp:952-1100`; `Hk/HkXmlTulokset.cpp:110-170`;
  `Hk/HkInit.cpp:978-1060`; `Hk/HkIV.cpp:3676-3685`; `Hk/HkDat.cpp:1324-1340`; `Juk/XMLtulos.cpp:185-290`; `Juk/VTulostus.cpp:7085-7330`;
  `cbHk/UnitAutoFile.cpp:40-100`; `cbHk/WinHk.cpp:1795-1830`; `cbHk/UnitMySQL.cpp:636-760`; `tputilv2/Wrttext.cpp:150-330`.
- **Muut luvut:** 03 (`SQLjonoVarm.dat`), 06 (tuloslaskenta), 07 (viestit), 09 (tiedonsiirto), 10 (laitteet), 13 (virhetilanteet).

## 7. Mitä ei ole selvitetty

**Avoimet kysymykset (seuranta.md):**

- Ratkaistu auditoinnissa: Q83 (bcc32:n `fputws` palauttaa -1, joten H-147:n tarkistus on kuollut kaikissa viidessä tuotteessa),
  Q84 (bcc32-RTL:ssä 50 virtaa), Q85 MSVC:n osalta (kaksoisvapautus päättää ohjelman ensimmäisellä kierroksella; bcc32-RTL:n
  pelkkä `free` on hiljainen) ja Q88 (viestin GUI ajaa `autofile`n `LukumaaratTh`-työsäikeessä) [todennettu: virheenkasittely.md
  luku 12.10].
- Q91: miten ViestiWinin VCL-muistinhallinta reagoi H-149:n kaksoisvapautukseen (ajokoe avoin).
- Q92: avaako bcc32-RTL:n `_wfopen_s` kirjoitusvirran yksinoikeudella (MSVC mitattu: lukija saa jakorikkomuksen).
- Q93: rikkooko `sendtcpfile`n tiedostovirta vastapään jäsennyksen, kun lähetys kulkee tavallisella tiedonsiirtoyhteydellä.
- Q95: HkKisaWinin tiedostovirtojen määrä vakiotilassa ja `femitfile`n NULL vaiheen vaihdossa (H-150).
- Q97: lopettaako SQL-virhe kirjoitussäikeessä ohjelman ja jääkö `tall`-lukko päälle viennissä (H-160, H-162); Q98: GUI-ohjelmien
  käynnistys ja SQL julkaisu-ZIP:stä puhtaalla koneella (H-163, H-165).
- Q104: viestin `vert0` SQL-kierroksen jälkeen (H-161); Q105: voiko SQL-lauseisiin liitettävän merkkijonon sisältöä hallita
  ulkoapäin ja mitkä ovat MySQL-käyttäjän oikeudet (injektiota ei ole vahvistettu); Q106: leviävätkö `lue_SQL`:n korvaukset
  `tallenna`n kautta muille työasemille; Q108: mitä viestin konsolin X)ML-tiedosto sisältää ajossa.
- Ei ole selvitetty: mitä HK:n `autofile` tekee sarjakohtaisessa HTML-tilassa ilman `AUTOFILE.LST`-tiedostoa; mistä
  ViestiWinin tulostuskaavakkeen valinnasta viestin IOF 3.0 -tulosluettelon bitti asetetaan; kuinka yleisiä
  automaattitulosteet, XML-automaatti ja SQL-peili ovat todellisissa asennuksissa (kaikkien vikakorttien yhteinen
  tuntematon).
- Tulostimen (GDI, raakakirjoitin) toimintaa ei ole ajettu; sivutus, fontit ja marginaalit on luettu vain tulostusparametreina.
- Käytännön ajokoe: yksikään tämän luvun väite ei perustu kokeeseen oikealla kilpailutietokannalla, oikealla palvelimella
  tai MySQL-instanssilla.

**Ehdokkaat (ei tarkastettu).** Haettu `havainnot.md`:stä ja `seuranta.md`:n KE-taulukosta (hakusanat `fprior`, `vainMuutt`,
`autokomentolaji`, `xmlots`): seuraavat sivuhavainnot eivät ole rekisterissä eivätkä KE-taulukossa (H-147 mainitsee `vainMuutt`-tilan
vain tiedoston kirjoitusvirheen yhteydessä). Ne ovat tämän aineiston lukemisen havaintoja, eivät tarkastettuja havaintoja, eikä
niille ole annettu tunnusta:

- **"Vain muuttuneet" -valinta ja muutoslaskuri.** Kun valinta on päällä, `autofile` kirjoittaa vain sarjat, joiden `fprior` on
  nollasta poikkeava. HK:ssa `fprior` kasvaa vain, jos asetettu ulkoinen komento ja kierrosväli ovat voimassa tai kyseisen
  maalin automaattitulostus kirjoittimelle on päällä (`Hk/HkDat.cpp:1322-1336`); viestissä vain jälkimmäisellä ehdolla
  (`Juk/vdat.cpp:821-824`). Jos laskuri ei kasva, valinta "vain muuttuneet" saattaisi jättää sarjat kirjoittamatta kokonaan; ei
  ajettu, joten vaikutusta ei tiedetä (`Hk/HkTulostus.cpp:7264`, `Juk/VTulostus.cpp:7253`) [luettu käsikirjaa varten, ei
  tarkastettu; ei tunnusta].
- **XML-automaatti monivaiheisessa kilpailussa.** Silmukka `do ... while (k_pv && kohde == 'X' && !yht++)` kutsuu `xmlots`ia
  jokaisella kahdesta kierroksesta (vaihe, yhteistulokset), mutta `xmlloppu`a vain silmukan jälkeen kerran
  (`Hk/HkTulostus.cpp:7140-7141`, `:7314-7324`). Kahden kierroksen jälkeisen tiedoston rakenne (kaksi alkuosaa, yksi loppuosa)
  on selvittämättä [luettu käsikirjaa varten, ei tarkastettu; ei tunnusta].

**Aiemmin kirjattu ehdokas, joka sivuaa lukua** (`seuranta.md` KE-taulukko): KE-13 (H-162:n rekisterikuvaus "emit-käsittely
pysähtyy" on ristiriidassa koodin kanssa: lukko on uudelleentuleva, joten pysähtyy vastaanotto (`tarkcom`), ei pääsäikeen
käsittely).
