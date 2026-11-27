# 07 Viestit: joukkueet, osuudet, vaihdot ja yhteistulokset

<!-- malli: Käsikirjan lukupohja (K-3). Ohje: doc/tietopohja/laatu/k3-kirjoittajan-ohje.md.
Luvun 5 vikaluettelo generoidaan tiedostosta kasikirja/toiminnot.csv komennolla
python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kasikirja; sitä ei kirjoiteta käsin. -->

> **Tila:** alustava K-3-aineisto, ei riippumattomasti tarkastettu. Lähderevisio: tuotantokoodi
> `f7fe553` (ennallaan, `doc/arkkitehtuuri/CHECKPOINT.md`). Varmuusmerkinnät: [todennettu],
> [päätelty], [tuntematon]; "luettu käsikirjaa varten" = luettu koodista K-vaiheessa, ei tarkastettu.

Luku kuvaa viestikilpailun ohjelmat ViestiWin (VW), ViestiMaali (VM) ja ViestiLuenta (VL): miten
joukkue ja sen osuudet on esitetty, miten osuuksien vaihdot, lähdöt, hajonta ja tulokset syntyvät ja
miten sisäänluenta toimii. Luku täydentää muita lukuja eikä toista niitä: kilpailun perustaminen,
numerointi ja arvonta ovat luvussa 02, tiedostot luvussa 03, ajanotto luvussa 04 (kohta 1.9 viestin
osuuden ja pisteen päättely), Emit-kortin luenta ja leimantarkastus luvussa 05, tuloksen laskennan
yleiset periaatteet luvussa 06, tiedonsiirto luvussa 09, laitteet luvussa 10, tulosteet ja
julkaisu luvussa 11 ja virhetilanteet luvussa 13. Vaiheen vaihtoa viestissä ei ole (luku 08).
Polut ovat suhteessa hakemistoon `TPsource/V52/`. Lyhenteet: HKM = HkMaali, HKW = HkKisaWin.
Lähdemerkinnät: [AA: ...] = auditointiaineisto, [KO: ...] = käyttöohje (`docs-site/docs/viestiwin/`),
[luettu käsikirjaa varten, ei tarkastettu] = koodi luettu tätä lukua varten. Sana "viesti" tarkoittaa
tässä luvussa kaikkia joukkuekilpailuja, joita nämä ohjelmat käsittelevät (viesti, partio, rogaining);
erot ovat kohdassa 2.

## 1. Mitä tapahtuu

### 1.1 Kolme ohjelmaa, yksi ydin

| Ohjelma | Mikä | Kääntäjä | Tehtävä |
|---|---|---|---|
| **ViestiWin** (VW) | Windows-käyttöliittymä | C++Builder (`bcc32`) | kaikki toiminnot: valmistelu, osanottajat, ajanotto, Emit-luenta ja -tarkastus, tulokset, sisäänluenta-lomake |
| **ViestiMaali** (VM) | konsoliohjelma | MSVC | päävalikko: M)aali (ajanotto), L)askenta (maaliajan kirjaus käsin), K)orjaukset, T)ulokset, lE)imat (leimantarkastus), Y)hteys, A)setukset, P)oistu |
| **ViestiLuenta** (VL) | konsoliohjelma, supistettu käännös (`LUENTA`) | MSVC | päävalikko: K)orjaa, T)ulokset, E)mitluenta (Emit-kortin liittäminen osuuteen), Y)hteys, A)setukset, P)oistu; ei ajanottoa eikä leimantarkastusta |

[todennettu: `JukConsole/laskenta.cpp:28-66`, projektien lähdelistat `VS/V/JukMaali520.vcxproj`,
`VS/V/JukLuenta520.vcxproj`, `RADStudio10/ViestiWin.cbproj`]

Ytimen koodi on hakemistossa `Juk/` (tietue `kilptietue`, `osrec`, `sarjatietue`; tallennus
`vdat.cpp`; tulos `vkilp.cpp`; järjestys `VIx.cpp`; ajanotto `VIv.cpp`; emit `VEmit.cpp`; tulosteet
`VTulostus.cpp`; XML `VXml.cpp`). Se käännetään erikseen jokaiseen ohjelmaan, joten samat nimet ovat
eri rakenteita kuin HK:ssa (`yleiskuva.md`). VW:n lomakkeet ovat hakemistossa `ViestiWin/`, konsolin
valikot hakemistossa `JukConsole/`. Käännöksen koko riippuu valitsimista (kohta 3): CI:n ja
julkaisun Release-kokoonpano rakentaa 30 osuutta, 250 sarjaa ja 12 väliaikaa; `150os`-kokoonpanot
(250 osuutta, 25 sarjaa, 8 väliaikaa) eivät ole julkaisutyönkulussa [todennettu:
`.github/workflows/release.yml:30-36`, projektitiedostot].

Samaa kilpailua voi käsitellä usealla koneella ja usealla ohjelmalla: jokainen ohjelmainstanssi
käsittelee omaa kilpailukansiotaan ja omaa `KILP.DAT`-tiedostoaan, ja tiedot pidetään yhtenäisinä
tiedonsiirrolla (luku 09) [KO: viestiwin 1.2]. Käytännön järjestely, esimerkiksi mikä kone on
palvelin tai mikä asema on lähtö- tai vaihtoalueella, on käyttäjän valinta, eikä aineisto kuvaa
yhtä suositeltua kokoonpanoa [tuntematon].

### 1.2 Käsitteet: joukkue, osuus, vaihto, piste, hajonta

| Käsite | Mitä se on tässä ohjelmistossa | Lähde |
|---|---|---|
| **Joukkue** | yksi `kilptietue`: kilpailunumero (`kilpno`), sarja, seura ja joukkueen numero seuran sisällä (näytetään muodossa "Seura -2"), joukkueen nimi (`joukknimi`), joukkueen tunniste (`joukkid`), piiri, ilmoittautumislista, kolme pistekenttää ja osuustaulukko (`ostiet[]`) | `Juk/VDef.h:734-840` |
| **Osuus** | yksi `osrec` joukkueen taulukossa: juoksijan nimi, lisenssinumero, osuuden seura, arvo, **hajonta** (ratakoodi), Emit- tai RFID-koodit (`badge[2]`) ja laina-merkintä, tarkastusmerkintä `keskhyl`, yksilöllinen tai yhteislähtöaika `ylahto`, lähtölaji, ampumasakot, aikasakko ja ajat `va[0..VAIKALUKU]` | `Juk/VDef.h:660-720` |
| **Piste** | osuuden aikojen paikka: **0 = vaihto tai maali** (kirjain `M`), 1… = väliaika, −1 = lähtö (`L`). Piste 0 on viimeisen osuuden maali ja muiden osuuksien vaihtoon tulo | `Juk/vkilp.cpp:279-310`; [AA: luku 04] |
| **Vaihto** | edellisen osuuden juoksijan tulo vaihtoalueelle eli piste 0 edelliselle osuudelle. Ohjelma käyttää sitä seuraavan osuuden lähtöajan perusteena (kohta 1.6). Ei erillistä "vaihto"-tietuetta | [todennettu: `Juk/vkilp.cpp:329-345`] |
| **Osuuskoodi** | osuuden tunnus käyttöliittymässä: `1`, `2`, … tai rinnakkaisosuuksilla `3A`, `3B` | `Juk/vApu.cpp:208-250` |
| **Hajonta** | osuuden ratakoodi (enintään `HAJONTA` = 8 merkkiä): kertoo, mitä rataa juoksija juoksee, kun viestissä on useita ratavaihtoehtoja. Käytetään Emit-leimantarkastukseen, tulostukseen ratakohtaisesti ja viestinnässä | kohta 1.9 |
| **Yhteislähtö** | osuuden lähtö kellonaikaan, jolloin sarjan hitaammat joukkueet lähtevät yhdessä; joukkueen osuudella `ylahto` ja lähtölaji 0 (automaattinen) | kohta 1.5 |
| **Vaihdon sulkemisaika** | sarjan osuuskohtainen kellonaika (`ylkynnys`): jos edellinen osuus tulee vaihtoon sen jälkeen, seuraavalle osuudelle merkitään yhteislähtöaika | kohta 1.5 |
| **Suljettu osuus** | osuus, jonka edellinen osuus on hylätty, keskeyttänyt, ei lähtenyt tai poissa; tilaa näytetään merkillä `S` | kohta 1.7 |
| **Sisäänluenta** | Emit-kortin liittäminen osuuteen lähtö- tai vaihtoalueella ennen kuin kortti leimaa | kohta 1.11 |

### 1.3 Sarjan osuusrakenne

Sarjatieto (`sarjatietue`, `Juk/VDef.h:376-425`; luku XML:stä `Juk/VXml.cpp`, vanha tiedosto
`Juk/vSarjat.cpp`) kertoo, kuinka monta osuutta joukkueessa on ja miten ne eroavat toisistaan
[luettu käsikirjaa varten, ei tarkastettu]:

- **`ntosuus`** on joukkueen osuuksien (vaihtojen) määrä ja **`osuusluku`** juoksijapaikkojen määrä.
  Tavallisessa viestissä ne ovat yhtä suuret. **Rinnakkaisosuudella** (esimerkiksi osuus "3", jolla
  juoksee kolme juoksijaa, paikat 3A, 3B ja 3C) `nosuus[osuus]` on suurempi kuin 1; osuuksien
  `aosuus[]`- ja `yosuus[]`-taulukot kuvaavat, mitkä juoksijapaikat kuuluvat mihinkin joukkueosuuteen.
  `maxnosuus` on sarjan suurin rinnakkaisosuuden koko; tavallisessa viestissä 1.
- **Sääntö rinnakkaisosuudelle:** `ekaMaaliRatkaisee[]` = vain ensimmäisenä maaliin tullut ratkaisee
  osuuden tuloksen ja tilan; `lahtoEdEkaMaalista[]` = seuraava osuus lähtee edellisen
  rinnakkaisosuuden ensimmäisen maaliintulijan mukaan. Päätöslogiikka on erillisessä, yksikkötestatussa
  tiedostossa `Juk/VRinnakkaisOsuus.cpp` (95 riviä, analyysitaso 1).
- **Muita osuus- tai sarjakohtaisia kenttiä:** väliaikojen määrä (`valuku[]`), ampumapaikkojen määrä
  (`paikat[]`), aikasakon yksikkö (`tsak[]`, `amptsak[]`), sarjan lähtöaika (`lahto`), osuuden
  yhteislähtöaika (`ylahto[]`), vaihdon sulkemisaika (`ylkynnys[]`), lähtöväli (`lahtovali[]`),
  numeroinnin siirto (`knosiirto`) ja partiossa tuloksen määrääjä (`partiotls`).
- **Osuuksien enimmäismäärä on käännösaikainen** (`MAXOSUUSLUKU`, toimitetussa 30). Ohjelma ei
  rajaa XML:stä tai sarjalomakkeesta luettuja osuus-, väliaika- ja rinnakkaisosuusmääriä
  käännösaikaisiin rajoihin, joten ylitys ylikirjoittaa muistia (H-042). Vanhan sarjatiedoston
  luvussa on lisäksi `sscanf("%s")`-kutsu, joka kirjoittaa osuuskohtaisen `paikat[]`-arvon
  (ampumapaikat) väärään paikkaan (H-002).

### 1.4 Joukkueen perustaminen, muokkaus ja poisto

**Lomakkeet VW:ssä.** Osanottajia käsitellään taulukossa *Joukkueet* (monta joukkuetta kerralla,
ruudukko, suodatus) tai yhden joukkueen lomakkeella *Joukkuetiedot* (osuusruudukko `OsGrid`,
väliaikaruudukko `TlsGrid`; haku numerolla, nimellä, tunnistinkoodilla tai juoksijan nimellä).
Muutokset sallitaan painikkeella *Salli muutokset*; *Tallenna* tallentaa ja *Peruuta muutokset*
hylkää. Lomakkeella voi lisätä ja poistaa yhden joukkueen sekä siirtyä "toistuvien lisäysten" tilaan
[KO: viestiwin 7.1; luettu käsikirjaa varten, ei tarkastettu: `ViestiWin/UnitJoukkuetiedot.cpp`]. Lisäksi
- *Osuustietojen kopiointi* kopioi yhden osuuden henkilötiedot muille osuuksille (esimerkiksi
  parisprintissä lähdeosuus 1 → kohdeosuudet "3 5"); ohitettavaksi jäävät osuudet, joita sarjassa ei
  ole [KO: viestiwin 3.3; `ViestiWin/UnitOsuuskopiointi.cpp`, analyysitaso 1];
- *Juoksujärjestyksen vaihto* (painike `BtnJjVaihto`, apulomake `UnitJjVaihdot`) antaa kullekin
  juoksijalle uuden osuuskoodin. Toiminto (`ModalResult` 101) kopioi nimen ja valittaessa lisenssin,
  Emit-koodit ja laina-merkinnän sekä hajonnan uuteen osuuspaikkaan; muut osuuden tiedot (osuuden
  seura, arvo, tarkastusmerkintä, ajat) jäävät paikoilleen. Muutos tehdään muistissa olevaan
  kopioon, ja se tallentuu vasta lomakkeen *Tallenna*-painikkeella. Apulomake tarjoaa lisäksi nimien
  pilkkomisen etu- ja sukunimiin ja isot alkukirjaimet (kohdat 102-104) [KO: viestiwin 7.1; luettu
  käsikirjaa varten, ei tarkastettu: `ViestiWin/UnitJoukkuetiedot.cpp:1402-1499`].

**Konsolissa (VM, VL)** valikko *Korjaukset* tarjoaa L)isää, K)orjaa, P)oista, J)uoksujärjestys,
T)iedostosta (juoksujärjestys luetaan tiedostosta) ja Emit-koodit tiedostosta (`E`, ei VL:ssä)
[luettu käsikirjaa varten, ei tarkastettu: `JukConsole/vcons2.cpp:1855-1885`].

**Tallennus (`tallenna`).** Kaikki viestin tietueen muutokset kulkevat funktion `tallenna`
(`Juk/vdat.cpp:470-850`) kautta. Funktio ottaa itse `tall`-lukon ja vapauttaa sen lopussa
[AA: `tallenna-samanaikaisuus.md` "Viesti"; todennettu]. Järjestys [luettu käsikirjaa varten, ei
tarkastettu]:

1. tarkastetaan tietue (`kilpTarkasta`): tuntematon sarja → 0, tulospalvelutilassa kilpailunumero
   rajojen ulkopuolella → uusi vapaa numero, lähtölaji > 2 → 0, tuntematon tarkastusmerkki → `-`;
2. jos sarjalla on vaihdon sulkemisaika, merkitään yhteislähtöajat (kohta 1.5);
3. verrataan tallennettua tietuetta uuteen (`kilpcomp`), jolloin saadaan eroavuudet joukkueittain ja
   osuuksittain (perustiedot, tarkastusmerkki, ajat, lähtö, sakot, Emit-koodit, hajonta jne.);
4. päivitetään Emit-koodien hakutaulut (`addbadge`, `rembadge`); jos koodi on jo toisella joukkueella,
   muutos peruutetaan;
5. päivitetään nimihakemisto, jos seura tai joukkueen numero muuttui;
6. kirjoitetaan tietue muistiin (`PUTREC`) ja tiedostoon (`putrec_i`);
7. jos sarja vaihtui, joukkuemäärälaskurit pitäisi päivittää (`paivJoukkLkm`); tämä haara ei toimi
   (H-130);
8. sijoitusketjut päivitetään osuuksittain ja väliajoittain (`remjarjarr`, `addjarjarr`);
9. muutokset lähetetään muille asemille (kohta 1.12), päivitetään lukumäärät ja ajan näyttö.

Kirjoituskutsu ja onnistunut tallennus ovat eri asioita: vaiheessa 6 `PUTREC` kirjoittaa muistissa
olevaan taulukkoon ja `putrec_i` tiedostoon, eikä tiedoston kirjoitusvirhe välity käyttäjälle
(H-015) [AA: `virheenkasittely.md`; luettu käsikirjaa varten, ei tarkastettu].

**Lisäys ja poisto.** `addtall` lisää aina uuden tietueen: tarkistaa Emit-koodit, varaa tietueen
`addfrec`-funktiolla (joka sulkee ja avaa `KILP.DAT`:n uudelleen, jos tiedoston pituus muuttuu),
päivittää numero- ja nimihakemiston, joukkuemäärän ja sijoitusketjut, lähettää tietueen muille
asemille ja tallentaa sen `tallenna`lla (`Juk/vdat.cpp:852-905`). `deltall` poistaa Emit-linkit,
sijoitusketjut ja hakemistot ja vapauttaa tietueen (`:907-935`) [luettu käsikirjaa varten, ei
tarkastettu]. Tunnetut viat: `addfrec` ottaa `kilpdat_CS`-lukon kahdesti eikä vapauta sitä, joten
toinen joukkuetta lisäävä tai poistava säie jumittuu (H-040), eikä se tarkista uudelleenavauksen
onnistumista (H-148); "Poista kilpailija" -valinta ei tarkista valintaa tai tilaa, ja poisto ei
tyhjennä Emit-linkkiä, jonka uudelleenkäytetty tietuepaikka perii (H-109, H-140).

**Muokkauksen ristiriidat.** Lomake pitää joukkueesta kopion (`Kilp`) ja alkuperäisen (`Kilp1`) ja
kirjoittaa vain, jos tietue ei ole muuttunut muualla; VW tekee vertailun `tall`-lukon sisällä
(`ViestiWin/UnitJoukkuetiedot.cpp:518-528`), toisin kuin HKW [AA: `gui-liiketoimintalogiikka.md` 3.8, 4].
Pitkään auki oleva Emit-lomakkeen *Muutokset*-dialogi pitää kuitenkin kopiota ja kirjoittaa sen
kokonaisena myöhemmin, mikä kumoaa välillä tulleet muutokset (H-017). Sarjavalikon tyhjä valinta
(`ItemIndex = −1`) tallentaa sarjaksi −1, jonka ydin muuttaa hiljaa sarjaksi 0 (H-114).

### 1.5 Numerointi, lähtöajat ja yhteislähdöt

**Numerointi ja sarjan lähtöaika (VW).** Valinta *Valmistelu / Numerointi* antaa numerot aakkos-,
tallennus-, satunnais- tai rankijärjestyksessä (FIS-pisteet) ja asettaa lähtöajat joukkueen numeron
mukaan: sarjan alkunumeron joukkue saa osuudelle määritellyn yhteislähtöajan, ja seuraavat saavat
lähtövälin verran myöhemmän ajan; osuudelle ilman yhteislähtöaikaa ei anneta lähtöaikaa (paitsi
partiossa, jossa joukkue lähtee ensimmäisen osuuden ajalla) [KO: viestiwin 4.1; AA:
`gui-liiketoimintalogiikka.md` 3.6]. Koodin vastine on `BtnLahtoajatClick`
(`ViestiWin/UnitArvonta.cpp:523`); sen lukkovuoto jättää `tall`-lukon pääsäikeelle jokaisesta
ohitetusta tietueesta, ja työsäikeet (vastaanotto, emit- ja aikalukijat) pysähtyvät ohjelman loppuun
(H-110). Viestissä ei ole HK:n kaltaista lähtöjärjestyksen arvontaa tai sarjan jakoa: `arvo(int)` on vain esitelty
(`ViestiWin/UnitArvonta.cpp:33`); satunnaisnumerointi sekoittaa numerot Mersenne twisterillä ja
tarkistaa törmäykset (`varmista_vapaa`), jota HK:ssa ei ole [AA: `gui-liiketoimintalogiikka.md` 3.6].
Viestin numeroinnin tunnetut viat: rankinumerointi katkaisee 1 000 joukkueeseen sarjaa kohden ilman
ilmoitusta (H-122); esitarkistus ja ajonaikainen raja eroavat, ja keskeytys jättää osan
numeroiduksi mutta ilmoittaa onnistumisesta (H-123); numerosiirto käyttää numeroa tietuekursorina
(H-118); numerointitapa ja tuonnin laina-korttiehto määräytyvät asettamattomista muuttujista (H-069).
Lähtöaikojen osuusvalikon nimiöt ovat siirtyneet (H-113).

**Konsolissa (VM, VL)** *Asetukset → L)ähdöt* kysyy joko L)ähtöajat (sarjan lähtöaika, kaikille
sarjoille ruudukossa) tai Y)hteislähtö (sarjan yhteislähtöaika ja *Vaihto kiinni* -aika). L-valinnassa
sarjan lähtöajan muutos lähetetään muille asemille (`laheta_yl`) ja kirjoitetaan sarja-XML:ään
(`kirjEventXml`); Y-valinnan näytön ohjeteksti sanoo, että yhteislähtö merkitään automaattisesti, jos
vaihdon sulkemisaika on annettu, ja että käsinmerkintä tehdään yksi osuus kerrallaan [luettu
käsikirjaa varten, ei tarkastettu: `JukConsole/laskent0.cpp:427-640`]. Luvun 02 kohta 1.8 totesi konsolin lähtöaika-asetukset
aiemmin kirjaamattomiksi.

**Yhteislähtö.** Kun sarjalle on annettu osuuden yhteislähtöaika (`ylahto[osuus]`) ja vaihdon
sulkemisaika (`ylkynnys[osuus]`, käyttöliittymässä *Vaihto sulj.* tai *Vaihto kiinni*), ohjelma
merkitsee yhteislähtöajan niiden joukkueiden osuudelle, joiden edellinen osuus tuli vaihtoon
sulkemisajan jälkeen. Merkintää ei tehdä, jos edellinen osuus on vielä kesken, koska kyse voisi olla
kirjauspuutteesta [KO: viestiwin 5.6]. Koodissa merkintä tehdään `tallenna`ssa joka kerta, kun
joukkueen tietue tallennetaan: tavallisessa viestissä suoraan (`Juk/vdat.cpp:518-532`: edellisen
osuuden vaihtoaika myöhempi kuin osuuden `ylkynnys` → osuudelle sarjan `ylahto`, muuten `ylahto`
tyhjennetään) ja rinnakkaisosuuksien sarjassa funktiolla `merk_yl` (`Juk/vApu.cpp:566-615`)
[luettu käsikirjaa varten, ei tarkastettu]. Merkintä tehdään vain osuuksille, joiden lähtölaji on 0
(automaattinen). Funktio `osrec::setMaali(−1, aika)` asettaa lähtölajin 1, joka vastaa käyttöohjeen
"yksilöllistä" lähtöä (kirjain `Y`), ja automatiikka ei muuta sitä; automaattinen merkintä kirjoittaa
`ylahto`-kentän suoraan eikä muuta lajia [todennettu: `Juk/vkilp.cpp:302-310`; KO: viestiwin 5.6].
Että lomakkeiden käsinsyöttö ja lähtökellon kirjaus käyttävät `setMaali`a, on päätelty. Lähtölajin
arvon 2 merkitys on [tuntematon]. Muutokset, jotka tekevät merkinnän aiheettomaksi,
poistetaan käyttöohjeen mukaan heti; `Yhteislähdöt`-lomake (`ViestiWin/UnitYLahdot.cpp`, analyysitaso 1) tallentaa
sarjatiedot XML:ään, lähettää ne muille asemille (`laheta_yl`, pakettiluokka `YHTEISLAHTOT`) ja ajaa
merkinnän sarjoille (`merk_yht_lahdot`) [luettu käsikirjaa varten, ei tarkastettu]. Vastaanottaja
tallentaa saapuneet yhteislähtöajat sarjatietoihinsa ja kirjoittaa sarja-XML:n (`tark_ylahto`,
`Juk/VCom.cpp:299-343`).

**Eralähtö ja lähtökello.** Kun lähtöaikataulu ei ole ennalta määrätty, lähtöajat voidaan ottaa
lähtökellon signaalista ja syöttää joukkueiden numerot käsin ajan kohdalle; aika kirjataan joukkueelle
yksilölliseksi lähtöajaksi (piste `L`). Parametri `LÄHTÖRYHMÄ=n` lisää kellopolussa (`tall_regnly`) lähtöpisteen (piste -1) kirjaukseen n-1 lisäriviä, joiden aika kasvaa yhdellä aikayksiköllä rivistä toiseen, ja kaikki rivit kirjataan samalle joukkueelle (`Juk/VIv.cpp:2190-2194`) [luettu käsikirjaa varten, ei tarkastettu]. HK toimii toisin: lisärivit ovat rivejä ilman kilpailijaa (kno = 0), ja kilpailija kohdistetaan vain lajeissa N, P ja U (`Hk/HkIV.cpp:2970-2991`, riippumaton tarkastus K-5 04-13), joten HK:n kuvausta ei voi lukea viestiin [KO: viestiwin 5.7; AA: luku 04 kohta 1.9].

### 1.6 Vaihto: ajan kirjaus ja seuraavan osuuden lähtö

Viestissä aika kuuluu osuudelle. Luku 04 kuvaa, miten ajanotto päättelee osuuden ja pisteen
(`tall_regnly`, `haeseurosuus`, `haeseurpiste`, `OSUUSOLETUS`, `OSUUSRAJA`, `JOUSTOVIESTI`) ja miten
aika tallennetaan aikajonoon. Tässä vain tulokseen vaikuttava osa:

- **Maali- tai vaihtoaika** tallentuu osuuden pisteeseen 0 (`osrec::setMaali(0, aika)`,
  `kilptietue::setMaali(osuus, 0, aika)`). Ajanottojonon rivi ja joukkueen tietue tallennetaan eri
  kutsuilla: ensin rivi, sitten tietue [AA: luku 04 kohta 1.1].
- **Tunnistinajanotto.** RFID- tai Emit-tunnisteella (`tall_etulos`, `Juk/VIv.cpp:539-650`) ohjelma
  hakee joukkueen tunnisteesta (`bdg2kno`) ja valitsee osuuden, jonka Emit- tai RFID-koodi vastaa
  tunnistetta ja jolla ei vielä ole vaihto- tai maaliaikaa (asetukset `OSUUSOLETUS`, `OSUUSRAJA`,
  `JOUSTOVIESTI` muuttavat valintaa), kirjoittaa ajan osuudelle (`setMaali`) ja tallentaa tietueen
  `tallenna`lla. Osuuden koodi on siksi oltava tallennettu etukäteen, esimerkiksi sisäänluennalla
  (kohta 1.11). Asetus `VUOROMAALI` ohjaa tämän polun pisteen valintaa: kun kaikki osuuden väliajat
  on kirjattu ja maalia ei ole, piste 0 (vaihto/maali) valitaan automaattisesti; ilman asetusta piste jää arvoon `valuku+1` (`Juk/VIv.cpp:620-626`; asetuksen asettaa `Juk/VInit.cpp:1457-1458`). Maalikellon `tall_regnly` ei käytä asetusta. Käyttöohjeen virke
  ("ellei ole annettu parametria VUOROMAALI, joka kieltää antamasta aikaa vaihdolle/maalille") on
  koodin vasta-asetelma; käytös on kuvattu koodista (`Juk/VIv.cpp:626`) [todennettu: riippumaton tarkastus K-5 04-25, `laatu/tarkastukset/k5/kasikirja.md`]. Ks. kohta 7.
- **Osuuden lähtöaika** (`kilptietue::Lahto(osuus)`, `Juk/vkilp.cpp:329-409`) valitaan tavallisessa
  viestissä (`maxnosuus == 1`) tässä järjestyksessä: (1) osuuden oma `ylahto` (yksilöllinen tai
  yhteislähtö); (2) muussa kuin partiossa edellisen osuuden vaihtoaika (`Maali(osuus − 1, 0)`); (3)
  sarjan lähtöaika. Rinnakkaisosuuksien sarjassa lähtö lasketaan edellisen osuuden tuloksesta,
  ensimmäisen maaliin tulleen ajasta tai k:nneksi nopeimman ajasta sen mukaan, onko
  `lahtoEdEkaMaalista` käytössä [luettu käsikirjaa varten, ei tarkastettu].
- **Ajanottolomake (VW).** *Tallenna muutos* -painike päivittää vain jonon rivin, ei joukkueen tietuetta
  (Q76 on auki: päivittääkö ydin tietueen); puuttuva `else` muuttaa A- ja L-rivit maaliriveiksi, mutta
  viestissä vain jonoon (H-111); piste yli sarjan rajan estetään viestissä (HK:ssa ei, H-117);
  Ctrl+Enter-pakotettu uudelleenlähetys ei toimi, koska argumentti on väärässä paikassa (H-121);
  `uusintaTallennus` hyväksyy negatiivisen osuuden ja indeksoi väärää taulukkoa (H-126); ajanottoruudukon
  sarjamuuttuja on asettamatta, joten aika voi päätyä väärälle osuudelle (H-069) [AA:
  `gui-liiketoimintalogiikka.md` 3.4, 5].
- **Kortin siirto toiselle joukkueelle tai osuudelle** (Emit-lomakkeen *Muutokset*, `TallennaVaihto`)
  käyttää alustamatonta `kno`-arvoa ja hyväksyy tyhjän numeron ja "Ei valittu" -osuuden, joten kortin
  numero voi korvautua pinon sisällöllä (H-112); osuuden pudotusvalikon "Ei valittu" indeksoi
  taulukkoa miinus kahdella (H-113); lomakkeen sulkeminen X:llä vastaa painiketta 2, joka on useissa
  Emit-dialogeissa tuhoava valinta (H-116) [AA: `gui-liiketoimintalogiikka.md` 5].

### 1.7 Tarkastusmerkintä ja osuuden sulkeutuminen

Osuuden tilaa kuvaa yksi merkki (`keskhyl`); arvot ovat `-` (avoinna), `T` (tarkastettu), `I`
(hylkäysesitys), `K` (keskeytti), `H` (hylätty), `E` (ei lähtenyt) ja `P` (poissa); muut arvot
korvataan hiljaa merkillä `-` (`kilptietue::SetTark`, `kilpTarkasta`). Käyttöliittymässä näytetään
lisäksi `S` (suljettu) [todennettu: `Juk/vkilp.cpp:1064-1160`, `:1317-1320`, `:1490-1494`].

- **Hyväksytty osuus** (`osHyv`) on osuus, jonka tila on `T`, `I` tai `-`; avoin osuus lasketaan siis
  hyväksytyksi. Joukkue on **hyväksytty osuuteen n asti** (`Hyv(n)`), kun kaikki osuudet 0…n ovat
  hyväksyttyjä; rinnakkaisosuudella ratkaisee tilan joko ensimmäisenä maaliin tullut (jos
  `ekaMaaliRatkaisee`) tai kaikki käytössä olevat paikat. Paikka on käytössä, jos sille on annettu nimi,
  Emit-koodi tai aika; pelkkä erotin `|` ei ole nimi (`PaikkaKaytossa`).
- **Suljettu osuus** (`Sulj(n)`): jos jokin edellinen osuus ei ole hyväksytty, osuus on suljettu.
  Osuuden tulos lasketaan silti vain, jos osuus on hyväksytty ja joukkue hyväksytty osuuteen asti tai
  asetus `SÄILHYL` (`kilpparam.sailhyl`) on päällä [todennettu: `Juk/vkilp.cpp:610-668`, `:671-715`;
  [KO: viestiwin 9.4]; oletus on pois päältä: `Juk/VInit.cpp:978-979` on ainoa asetuskohta ja `kilpparam` on alustamaton globaali, ks. luku 06 kohta 1.3] [luettu käsikirjaa varten, ei tarkastettu].
- **Joukkueen tila** (`Tark(−1)`, `TarkStr(−1)`): alkuosuuden `E` tai `P` ratkaisee; muuten joukkue on
  `K`, jos viimeinen osuus ei ole hyväksytty ja kukaan ei ole hylätty, `H`, jos jokin osuus on
  hylätty, ja `T`, jos kaikki osuudet on tarkastettu [todennettu: `Juk/vkilp.cpp:1147-1190`].
- **Ei lähtenyt** kirjataan osuuskohtaisesti, jotta metsässä olevien seuranta toimii, vaikka aiempi
  osuus on hylätty [KO: viestiwin 7.2].
- **Partio** (`kilpparam.partio` 1 tai 2): tulos ja hyväksyntä ratkeavat, kun kaikki jäsenten kortit
  on luettu; tieto kirjataan ensisijaisesti viimeisen jäsenen kohdalle [KO: viestiwin 10.2]. Partion
  leimantarkastus on `tark_partio` (luku 05). Konsolin partiokilpailu ja `AUTORAP` ilman
  listakirjoitinta kaataa ohjelman kortin luennassa (H-064).

### 1.8 Osuuden ja joukkueen tulos

Tulosfunktiot ovat luokassa `kilptietue` (`Juk/vkilp.cpp`). Aika on yksikössä 1/10 000 s; tulos 0
tarkoittaa, ettei tulosta ole. Luku 06 antaa yleiskuvan (kohta 1.3); tässä kuvataan viestin
rakenteet [luettu käsikirjaa varten, ei tarkastettu]:

| Käsite | Funktio | Mitä se laskee |
|---|---|---|
| **Joukkueen aika osuuden n jälkeen** (yhteistulos) | `Tulos(n, piste)` | tavallisessa tilanteessa osuuden maali − sarjan lähtöaika + sakkoaika (kumulatiivinen); yhteislähdössä edellisen osuuden yhteistulos + (osuuden maali − osuuden `ylahto`); muuten edellisen yhteistulos + (osuuden maali − edellisen osuuden maali). Vuorokauden yli menevät ajat normalisoidaan. Tulos on 0, jos osuus ei ole hyväksytty (`Hyv`) eikä `SÄILHYL` ole päällä. Kun `ylahto` on asetettu, partiossa (`partio` 1) tulos on maali − osuuden `ylahto` + osuuden sakot ja tilassa 2 pisin (maali − `ylahto`) kaikista osuuksista + viimeisen osuuden sakot; rogainingilla on oma haaransa |
| **Osuusaika** | `osTulos(n, piste)` | osuuden maali − osuuden lähtö (`Lahto(n)`) + osuuden sakot; vaatii osuuden hyväksynnän ja oletuksena myös, että joukkue on hyväksytty osuuteen asti |
| **Lopputulos** | `kokTulos()` | tavallisessa viestissä `Tulos(viimeinen osuus, 0)`; partiossa `partiotls`: 0 = viimeisen osuuden tulos, −1 = myöhäisin osuuden tulos (viimeisenä maaliin tullut), −2 = kaikilta kortin saaneilta osuuksilta vaaditaan tulos ja myöhäisin käytetään, n > 0 = osuuden n tulos |
| **Rinnakkaisosuuden ryhmän tulos** | `tTulos(n, piste)`, `aTulos`, `ekaMaaliTulos` | ryhmän tulos on joko ensimmäisenä maaliin tulleen tai hitaimman aika; puuttuvasta paikasta tulee "puutelisä" (`puutelisa`), jos kaikkia paikkoja ei ole vielä maalissa |
| **Sakkoaika** | `SakkoAika(n, kumulatiivinen)` | osuuden ampumasakkojen lukumäärä × sakkoyksikkö (`amptsak`) + osuuden oma `sakko`; summataan kumulatiivisesti, kun halutaan yhteistulos |
| **Osuuden lähtö** | `Lahto(n)` | kohta 1.6 |

Ampumasakot kirjataan osuudelle merkkijonona `ampsakot` (yksi merkki jokaista ampumapaikkaa kohti,
`paikat[]`); aikasakon yksikkö on sarjakohtainen [KO: viestiwin 5.9; `Juk/vkilp.cpp:717-752`].
Sakkokierroslomakkeen osuusnumeroa ei rajata, joten tyhjällä kentällä kirjoitus osuu tietueen
`seura`-kenttään ja suurella numerolla pinoon (H-082). Partion, rogainingin ja kierrosten laskennan
(`alalaji` `P`, `R`, `D`) viestikohtaiset haarat ovat `Tulos`- ja `kokTulos`-funktioissa; rogainingin
ja kierrosten haaroja ei ole kuvattu tässä tarkemmin [tuntematon].

### 1.9 Sijoitukset ja ketjut

Sijoitukset eivät ole tiedostossa tuloksina vaan ylläpidettyinä muistirakenteina. Jokaiselle sarjalle
ja osuudelle ylläpidetään linkitetyt ketjut (`jalku`, `jrjseur`, `jarjtulos`), joissa joukkueet ovat
järjestyksessä tulos- ja tilaryhmittäin: 0 hyväksytty tulos, 1 keskeyttänyt, 2 hylätty, 3 ei lähtenyt,
4 muu (avoin); lisäksi väliajoille omat ketjunsa ja viimeiselle indeksille osuusajan ketju
(`Juk/VIx.cpp:660-700`). Ketjut päivitetään `tallenna`ssa (`remjarjarr`, `addjarjarr`); viestissä
tallennuksessa päivitetään joka osuuden ja väliajan ketju erikseen, ja rinnakkaisosuudella jokainen
paikka päivittää ketjun omalla väliajallaan. Tietueessa on kentät joukkueen sijalle pisteittäin (`va[piste].sija`, `Sija(osuus, piste)`) ja
osuuden omalle sijalle (`ossija`, `osSija`); mikä näitä kenttiä päivittää, ei ole selvitetty
[tuntematon]. [luettu käsikirjaa varten, ei tarkastettu; AA: `globaali-tila.md` 5].

Joukkuemäärälaskurit (`nilm`, `nilmt`, `nilmtt`) ja numerohakemisto (`kilpnoix`, suora taulukko) ovat
muistissa; uudelleenkäynnistys rakentaa ne tietueista (`teejarj`) [AA: `globaali-tila.md` 4.1, 5].
Sarjan vaihto ei päivitä joukkuemääriä: `nilm[vanha]` jää liian suureksi ja `nilm[uusi]` liian
pieneksi uudelleenkäynnistykseen asti, ja `ossijat` varaa puskurin laskurin mukaan ja täyttää sen
ketjusta rajatta, joten liian pieni laskuri johtaa ylivuotoon (H-130, H-076). Väliaikaketjun linkit
voivat jäädä vanhaan sarjaan (H-142).

### 1.10 Hajonta

**Mitä hajonta on.** Viestissä, jossa eri juoksijat juoksevat eri ratavaihtoehtoja (hajonta), kullekin
osuudelle merkitään ratakoodi (`osrec::hajonta`, enintään 8 merkkiä; `HAJONTA = 8` kaikissa
viestikäännöksissä [AA: `tiedostomuodot.md` "KILP.DAT: viestitietue"]). Koodi syötetään
*Joukkuetiedot*-lomakkeen osuusruudukon ratasarakkeeseen (isot kirjaimet), joukkuetaulukkoon, tuonnissa,
*Siirrä lisätietoja* -toiminnolla (myös "ensimmäinen hajontakoodi kaikille osuuksille") tai Emit-lomakkeen
ratavalinnalla [luettu käsikirjaa varten, ei tarkastettu: `ViestiWin/UnitJoukkuetiedot.cpp:753-754`,
`UnitLisatiedot.cpp:90`, `UnitEmit.cpp:1164`, `:1245`, `UnitImport.cpp:305`]. Partiossa ohjelma
täyttää tyhjän hajontakentän sarjan nimellä käynnistyksessä, kun emit on käytössä (`teejarj`,
`Juk/VIx.cpp:1270-1271`).

**Mihin hajontaa käytetään.**

1. *Leimantarkastus.* Ohjelma hakee osuuden radan hajontakoodilla (`getrata(hajonta)`) ja tarkastaa
   kortin leimat sitä vasten (`Juk/VEmit.cpp`, useita kohtia, esimerkiksi `:1314`, `:1849`, `:2019`;
   luku 05); konsolin `pros_emit` käyttää hajontaa myös ilmoituksessa "Luettu aiemmin osuudelle …
   Rata osuudelta n", kun kortille ei löydy vapaata osuutta (`Juk/VIv.cpp:1296-1310`).
2. *Tulokset hajontaradoittain.* Tulostusvalinta *Hajontakohtaiset* kirjoittaa osuuskohtaiset tulokset
   rata kerrallaan, joko jokaiselle osuudelle erikseen tai yhdistäen sarjan kaikki saman radan
   juosseet osuudesta riippumatta [KO: viestiwin 9.4]. Rata-avaimina käytetään sarjaa, hajontakoodia ja
   osuusaikaa (`keyostls`, `Juk/VIx.cpp:372-411`). Kun tulokset tehdään osuuksittain, ratalistan antaa
   tiedosto `HAJONNAT.LST` (lohkot `S<sarja>` ja `O<n>`; yksi ratatunnus riviä kohti); kun tiedostoa
   ei ole tai kaikki osuudet tulostetaan yhdessä, radat haetaan ratataulukosta (`rata[]`; `seur_rata`,
   `Juk/VTulostus.cpp:8618-8700`)
   [luettu käsikirjaa varten, ei tarkastettu]. Puuttuva `HAJONNAT.LST` kaataa VM:n hajontatulosteessa
   (`fclose(NULL)`; GUI:ssa ei) (H-063). Omalla osuudellaan hylätyt tulevat luetteloon vain, kun
   `SÄILHYL` on annettu, ja vain jos he ovat saaneet ajan [KO: viestiwin 9.4].
3. *Ratakoodi tiedonsiirrossa.* Osuuden hajonta kulkee muille asemille osuuspaketin muutosmaskin
   kentässä (bitti 0x10, `Juk/VCom.cpp:633-634`); TCP-tekstisanomaan se tulee mukaan vain asetuksella
   `TCPHAJONTA` (`Juk/VCom.cpp:1110-1113`).
4. *Näyttö ja tuloste.* Asetus `NÄYTÄHAJONTA=EI` piilottaa hajonnan ViestiWinin osuustietoikkunasta;
   `RATAVAL=` vaikuttaa hajontatiedon tilankäyttöön tulosteessa (`Juk/VInit.cpp:902`, `:1226`).

Ratatiedot (radat ja ratakoodien selitykset) ovat luvuissa 02 ja 05.

### 1.11 Sisäänluenta

Sisäänluenta liittää Emit-kortin osuuteen ennen kilpailijan lähtöä: lähtö- tai vaihtoalueelle
siirtymisen yhteydessä luetaan osuuden tunnus ja kortti, ja kortin koodi tallentuu osuuden Emit-koodiksi
(`badge[0]`, tai `badge[1]`, kun kaksi koodia on käytössä). Käyttöohje kertoo, että tällä tavalla
viestissä, jossa koodeja ei ole tallennettu ennalta, sama kortti ei ole koskaan samanaikaisesti
useammalla kilpailijalla, ja kortin toistuva käyttö (samassa tai eri joukkueessa) tuottaa jokaiselle
suoritukselle oman koodin [KO: viestiwin 6.7 (tiedostossa 6.11_emitkoodien_lukeminen), 6.9]. Toiminta
on kahdessa ohjelmassa:

| | VW *Sisäänluenta (emit)* | VL *Emitluenta* (`E`) |
|---|---|---|
| Käynnistys | parametri `SISÄÄNLUENTA` (`=M`: myös normaali Emit-luenta); valikko *Tulospalvelu / Sisäänluenta (emit)* | aina päävalikossa |
| Joukkueen ja osuuden valinta | viivakoodi tai joukkueen numero ja osuus näppäimistöltä [KO: viestiwin 6.7] | viivakoodi tai vastaava `%`- tai `*`-alkuinen syöte, joka sisältää joukkueen numeron ja osuuden |
| Kortin luku | lukijasäie asettaa uuden kortin (`uusi_emit`); lomake näyttää sen (`NewCardHandler`) | sama: odotus näytöllä, `K)orjaa tieto näppäimistöltä`, `P)oimi edellinen luettu` |
| Tarkistukset | koodi 200 = viallinen kortti: kortti on vaihdettava; sama kortti jo luettu samalle osuudelle: ei muutettavaa; kortti varattuna toiselle osuudelle: kortti on vaihdettava, ellei aiemman haltijan osuuden tila ole hylätty, keskeytetty, ei lähtenyt tai poissa, jolloin ohjelma antaa aiemmalle haltijalle vapaan koodin; osuudella on jo eri koodi: kysytään vahvistus | koodi 200; sama kortti jo kirjattu; kortti varattuna toiselle osuudelle (vapautuu samoin, jos haltijan osuus ei ole hyväksytty); edellisellä osuudella ei korttia (K/E/L); osuudella on jo tulos tai merkintä (K/E); asetuksella `TARKAVOIMET`: kortti lukematta aiemmalta osuudelta |
| Tallennus | Enter tallentaa: `getrec`, koodin asetus ja `Tallenna` `tall`-lukon alla; Esc peruu | `H)yväksy tiedot`: koodin asetus ja `tallenna` |
| Edellinen kortti kesken | uutta korttia ei oteta, ennen kuin edellinen on tallennettu tai peruttu (`addluenta`, `hylatty_emit`) | vaiheet etenevät peräkkäin: joukkue ja osuus → kortti → hyväksyntä |
| Loki | parametri `LUENTALOKI` → `luentaloki.txt` (onnistuneet ja kesken jääneet tapahtumat) | – |
| Testi | `LUENTATESTI` lukee parit tiedostosta `emitrek.lst` | sama (`LUENTATESTI[A]`, `EMITREK.LST`) |

[KO: viestiwin 6.7, 1.4.5; luettu käsikirjaa varten, ei tarkastettu: `ViestiWin/UnitLuenta.cpp:370-665`,
`JukConsole/laskemtn.cpp:296-780`, `Juk/VIv.cpp:920-931`, `:1095-1112`, `Juk/VInit.cpp:764-772`,
`:2144-2162`]

Sisäänluenta tallentaa muutoksen joukkueen tietueeseen `tallenna`n kautta, joten se lähtee muille
asemille tavallisena osuustietopakettina (kohta 1.12). VW:ssä sisäänluenta ja normaali Emit-luenta
käyttävät samaa kortinlukijaa: parametri `SISÄÄNLUENTA` (ilman `=M`) kytkee Emit-tarkastuksen pois
(`emitfl = 0`, otsikko "Vaihe: Sisäänluenta"); valikon avaus asettaa sisäänluentatilan lipun päälle
(`SisLuenta`: 1) tai pois (`Emitluenta`: 0), ja kortti ohjautuu sen mukaan, kumpi lomake avattiin
viimeksi [luettu käsikirjaa varten, ei tarkastettu: `ViestiWin/UnitMain.cpp:236-282`, `:1744-1752`,
`:1819-1826`]. VM:ssä ei ole sisäänluentalomaketta: tiedosto `laskemtn.cpp` kuuluu vain VL:n
projektiin [todennettu: projektitiedostot].

ViestiLuenta on supistettu käännös: ilman `LAJUNEN`, `EMITHTTP`, `VALUKIJA`/`AUTOVA`, `ALGETAULU`,
`ESITARK` ja `EMITFILE` (T-052; `EMITFILE` on emit-testisyöte tiedostosta `EMIT_IN.DAT`, ei emit-tiedosto).
Lisäksi VL:n projektista puuttuu `Juk/VEmit.cpp`, joten VL ei käsittele emit-tietueita eikä tallenna niitä
tiedostoon, se ei tue RFID-lukijoita eikä `emiTag`-palvelinta, ja RFID-avaimet (`SIRIT`, `ZEBRA`,
`FEIG`, `IMPINJ`) valitsevat hiljaa ALGE-ajanoton (H-176) [AA: `rekisteri/havainnot.md` T-052, H-176;
`rekisteri/valitsimet.csv`; todennettu: `VS/V/JukLuenta520.vcxproj`].

### 1.12 Muutosten välitys muille asemille

Kun `tallenna` huomaa muutoksen, se lähettää muille asemille yhden tai useamman paketin
(pakettiluokat `KILPT`, `OSUUST`, `TARKT`, `VAIN_TULOST`, `YHTEISLAHTOT`) [AA: `tiedonsiirto.md`
"Sarjallistaminen", "HK ja viesti"; luettu käsikirjaa varten, ei tarkastettu: `Juk/vdat.cpp:765-815`,
`Juk/VCom.cpp`]:

- **perustiedot** (joukkueen nimi, seura, sarja, numero jne.): koko perustietue `memcpy`llä (`KILPT`);
- **osuuden tiedot**: vain muutosmaskin kentät (`OSUUST`): nimi, lisenssi, osuuden seura, arvo,
  hajonta, Emit-koodit ja laina, selitys, lähtöaika ja -laji, tarkastusmerkintä, ampumasakot ja sakko;
- **aika**: jokainen muuttunut osuuden aika omana pakettinaan (`VAIN_TULOST`); jos aika on ajanoton
  viimeisin aika eikä osuuden perustiedot ole muuttuneet, sitä ei lähetetä koneelle, jolle
  ajanottojono lähetetään (luokka 1, muuten 2) [päätelty koodin kommentista, `Juk/vdat.cpp:588-592`,
  `:770-810`];
- **tarkastusmerkintä, ampumasakot ja aikasakko** kulkevat osuuspaketin muutosmaskissa (bitit
  0x10000-0x40000, `Juk/VCom.cpp:655-661`); erillisen `TARKT`-luokan vastaanottokäsittelijä `tark_trk`
  on olemassa, mutta sen lähettäjää ei ole luettu;
- **yhteislähtöajat** (`YHTEISLAHTOT`) sarjakohtaisesti.

Vastaanottaja tarkistaa viestissä joukkueen numeron jokaisesta pakettiluokasta (OSUUST, TARKT,
VAIN_TULOST) toisin kuin HK, joka tarkistaa vain KILPT:n. Jos tietueet ovat eri koneilla
epäyhtäpitävät, ohjelma ilmoittaa "Tiedostot eri koneilla eivät ole yhtäpitäviä". Viestissä
vastaanotto, jonka pakettiluokkaa ei tunneta, pysäyttää koko vastaanoton pysyvästi (H-026);
`combufsize` ilman otsakkeen 14 tavua aiheuttaa jonon ylivuodon ja suurimman paketin lopun
korruptoitumisen (H-025); `tark_trk` palaa lukko hallussa kuuluttajatilassa (H-024); konfiguraation
`KATKKIER` indeksi on tarkistamatta (H-036). Viestin vastaanottopolkua kokonaisuutena ei ole
jäljitetty [AA: `tallenna-samanaikaisuus.md`]. Kutsu, lähetys, vastaanotto ja kuittaus ovat eri
vaiheita (luku 09): `tallenna` kutsuu `laheta`a, joka laittaa paketin lähetysjonoon; jonoon laitto ei
takaa lähetystä eikä vastaanotto kuittausta.

### 1.13 Tulosluettelot ja tulosteet

Tulosteissa valitaan joukkue- tai osuustulokset, tulostettavat osuudet (yksi tai kaikki valittuun
asti), järjestys (vaihto, osuus, aakkos, numero) ja rajaus (tulokset, hylätyt, keskeyttäneet, ei
lähteneet) sekä kohde: paperi, tekstitiedosto, HTML, XML, Emit-raportti [KO: viestiwin 9.2-9.4; luettu
käsikirjaa varten, ei tarkastettu: `tulostusparamtp`, `Juk/VDef.h:529-640`]. Tulosteiden julkaisu ja
automaattitulosteet ovat luvussa 11; viestin tulostuskoodiin kohdistuvat tunnetut viat ovat:

- `memset(stsk, 0, sizeof(st))` kirjoittaa 120 tavua 40-tavuiseen puskuriin tekstitulosteen
  joukkue- ja osuusriveillä, kun osuudella on ampumapaikkoja (`paikat[osuus]`) (H-001; toistettu
  testikehyksessä);
- `ossijat` voi ylivuotaa (H-076, H-130);
- Emit-osuustulosteen HTML-polku ei vapauta `print`- ja `rastiva`-lukkoja (H-087);
- automaattinen XML-tiedosto sisältää vain loppuosan ja vapautettu tulosteosoitin jää käyttöön (H-149,
  H-158); konsolin X)ML-valinta ei kirjoita esittelyä (H-172).

## 2. Erot ohjelmien välillä

| Asia | VW | VM | VL |
|---|---|---|---|
| Käyttöliittymä | VCL-lomakkeet | konsoli | konsoli |
| Ajanotto (maalikellot, tunnistimet, CTS) | kyllä | kyllä (`M`) | **ei** (ei `ajanotto()`/`laskenta()`) |
| Maaliajan kirjaus käsin | ajanottolomake | `L)askenta` | ei |
| Leimantarkastus | Emit-lomake (`Emit-luenta`) | `lE)imat` | ei: `E)mitluenta` on eri toiminto, Emit-tunnistus |
| Sisäänluenta | lomake `Sisäänluenta (emit)` | ei | `E)mitluenta` |
| Emit-tiedosto (`EMIT.DAT`) ja emit-tietueiden käsittely (`Juk/VEmit.cpp`) | kyllä | kyllä | **ei** (`VEmit.cpp` ei kuulu projektiin) |
| RFID, emiTag, väliaikalukijat | kyllä | kyllä | **ei** |
| Palvelinvalikko `Y)hteys` | – (lomakkeet) | kyllä (`SERVER`) | kyllä (`SERVER`) |
| Näyttötaulu (`ALGETAULU`) | kyllä | kyllä | ei |
| Juoksujärjestyksen vaihto | apulomake | `K J)uoksujärjestys`, `T)iedostosta` | sama |
| Lähtöajat ja yhteislähtö | `Valmistelu / Numerointi`, `Tulospalvelu / Yhteislähdöt` | `A)setukset / L)ähdöt` | sama |
| Tulosteet | tulostuslomake | `T)ulokset` | sama |
| Puuttuva `HAJONNAT.LST` hajontatulosteessa | ei kaadu | kaatuu (H-063) | `seur_rata`-funktion runko on `#ifndef LUENTA` -lohkossa [luettu käsikirjaa varten] |
| Julkaisuun mukana | kyllä (`Release Build`) | kyllä (`Release`) | kyllä (`Release`) |

[todennettu: projektitiedostot ja `JukConsole/laskenta.cpp`; luettu käsikirjaa varten, ei tarkastettu
muilta osin]

**HK ja viesti.** HK ja viesti ovat eri ohjelmat ja eri tietuerakenteet: ne eivät voi keskustella
keskenään (viestin aloituspaketti sisältää vaiheen 0 ja HK:n pyyntö hylätään viestillä
"Henkilökohtainsen kilpailun ohjelma pyytää yhteyttä", `Juk/VCom.cpp:842-865`). Viestin aikajono
(24 tavun rivi) ja `COMFILE.DAT` eivät ole yhteensopivia HK:n kanssa (`tiedostomuodot.md`). VW:n ja
HKW:n lomakkeet ovat kopioimalla erkaantuneita haaroja: korjauksia on tehty vain toiselle puolelle
(T-034, `gui-liiketoimintalogiikka.md` luku 4): piste yli sarjan rajan estetään viestissä mutta ei HK:ssa;
viestin `tall_rivi`-kutsusta puuttuu oikea argumentti; "muuttunut muualla" -vertailu on VW:ssä lukon
sisällä; `NumKaikki` asetetaan HK:ssa mutta ei viestissä.

**Osuudet, partio ja rogaining.** `kilpparam.partio` (1 tai 2) ja `kilpparam.rogaining` määräytyvät
kilpailun alalajista (`P`, `R`; `Juk/VXml.cpp:562-572`). Partiossa joukkue lähtee yhdessä, tulos ja
hyväksyntä ratkeavat vasta kun kaikki kortit on luettu ja `partiotls` valitsee lopputuloksen. Rogainingin
tulos käyttää `Tulos`-funktion omaa haaraa. Parametrit `PARTIO` ja `ROGAINING` on poistettu
(ilmoitus "poistettu käytöstä. Katso ohjeita", `Juk/VInit.cpp`). VW:n lomaketta `Partio` (`UnitPartio.cpp`,
analyysitaso 1) ei ole luettu.

## 3. Asetukset ja käännösvalitsimet

| Avain tai valitsin | Ohjelmat | Vaikutus | Lähde |
|---|---|---|---|
| `MAXOSUUSLUKU` (30 tai 250), `MAXSARJALUKU` (250 tai 25), `VAIKALUKU` (12 tai 8) | VW, VM, VL | osuuksien, sarjojen ja väliaikojen enimmäismäärä käännöksessä; `Release` 30/250/12, `Release150os` 250/25/8 (VL: kaikki 30/250/12) | `VS/V/*.vcxproj`, `RADStudio10/ViestiWin.cbproj`; T-051 |
| `HAJONTA` = 8 | VW, VM, VL | hajontakoodin enimmäispituus | `Juk/VDef.h:192`, projektit |
| `LUENTA` | VL | supistettu käännös: ei ajanottoa, ei leimantarkastusta, ei emit-tiedostoa | `VS/V/JukLuenta520.vcxproj`; T-052 |
| `EMITFILE`, `ESITARK` | VM | emit-testisyöte tiedostosta `EMIT_IN.DAT`; esitarkastusnäyttö; ei VW:ssä eikä VL:ssä | `rekisteri/valitsimet.csv` |
| `VALUKIJA`, `LAJUNEN`, `EMITHTTP`, `ALGETAULU` | VW, VM (ei VL) | väliaikalukija, RFID-lukijat, emiTag-HTTP, näyttötaulu | projektitiedostot; `rekisteri/valitsimet.csv` |
| `SERVER` | VM, VL | `Y)hteys`-valikko | `JukConsole/laskenta.cpp` |
| `SISÄÄNLUENTA[=M]` | VW (avain jäsennetään kaikissa) | sisäänluentatila; `=M` pitää myös Emit-luennan | `Juk/VInit.cpp:764-769`; `ViestiWin/UnitMain.cpp:245-247` |
| `LUENTALOKI`, `LUENTATESTI[A]` | VW, VL | `luentaloki.txt`; testaus `emitrek.lst`-tiedostosta | `Juk/VInit.cpp:770`, `:2144` |
| `TARKAVOIMET`, `SALLIEMITENSIN` | VL | varoitus aiemman osuuden lukemattomasta kortista; kortti ennen osuuden valintaa | `Juk/VInit.cpp:2151-2162` |
| `esta_uusikaytto` (muuttuja, oletus päällä; asetusavainta ei etsitty) | VM, VL | saman kortin uudelleenkäytön esto | `Juk/VGlobals.cpp:424` |
| `JOUSTOVIESTI`, `JOUSTOPAKOTA` | VW, VM, VL | osuuden valinta tunnisteesta | luku 04 |
| `OSUUSOLETUS=`, `OSUUSRAJA=`, `VUOROMAALI`, `SAMAPISTE`, `UUSINAIKA` | jäsennetään VW, VM, VL; vaikuttaa VW, VM (VL:ssä ei ole `tall_etulos`-polkua, `LUENTA`, `Juk/VIv.cpp:157`) | osuuden ja pisteen päättely ajanotossa | luku 04; `Juk/VInit.cpp:1349-1457` |
| `SÄILHYL` | VW, VM, VL | hylätyn osuuden aika säilytetään tuloksissa (`kilpparam.sailhyl`) | `Juk/VInit.cpp:977`; [KO: viestiwin 9.4] |
| `NÄYTÄHAJONTA=EI`, `RATAVAL=`, `TCPHAJONTA` | VW; VW, VM, VL; VM, VW, VL | hajonnan näyttö, tilankäyttö tulosteessa, lähetys TCP-sanomassa | `Juk/VInit.cpp:902`, `:961`, `:1226` |
| `KATKKIERTO` | VW, VM, VL | silmukan katkaisu tiedonsiirrossa; indeksi tarkistamatta (H-036) | `Juk/VInit.cpp:1137` |
| `AIKAKORJAUSn` | VW, VM, VL | jonokohtainen aikakorjaus; viesti kertoo sekunneilla, HK sadasosilla; tarkoitettu yksikkö on auki (Q112) | `seuranta.md` Q112 |
| Tiedostot `HAJONNAT.LST`, `SARJAYHD.LST`, `lahdepisteet.lst`, `lahdejonot.lst`, `lainaemit.lst` | VW, VM, VL | ratalistat tuloksiin, sarjayhdistelmät, ajan lähdepisteet, lainakortit | [KO: viestiwin 2.3, 5.5, 6.7, 9.4] |

## 4. Vaihtoehtoiset polut ja virhetilanteet

| Tilanne | Mitä tapahtuu | Ohjelmat | Lähde |
|---|---|---|---|
| Joukkueen lisäys verkossa samanaikaisesti toisella säikeellä | `addfrec` jättää `kilpdat_CS`:n ottamatta pois; toinen säie jumittuu | VW, VM, VL | [AA: H-040] |
| Levy täynnä joukkueen lisäyksessä | `addfrec` sulkee ja avaa `KILP.DAT`:n; avaus hylätään, kahva jää suljetuksi, ja uusi joukkue puuttuu myös muistista | VW, VM | [AA: H-148] |
| Poista kilpailija ennen valintaa tai lisäys-Peruuta-poisto | virheellinen tietuenumero; poisto voi hävittää vapaiden tietueiden luettelon | VW (ja HKW) | [AA: H-109] |
| Joukkueen sarja vaihtuu | `nilm` ei päivity; väliaikaketjut vanhaan sarjaan | VW, VM, VL | [AA: H-130, H-142] |
| Sarja-XML sisältää liikaa osuuksia tai väliaikoja | puskuriylitys; GUI:n sarjalomake hyväksyy yli 30 osuutta ja 12 väliaikaa | VW, VM, VL | [AA: H-042] |
| KILP.DAT-vienti, kun `valuku < VAIKALUKU` | vienti kirjoittaa muistin olion eikä `pack`-tulosta; osuudet ≥ 1 väärissä kohdissa | VW | [AA: H-046] |
| Sarjan vaihto käyttäjän tekemänä `vaihdakilpsrj`:llä | `KILP.DAT`-kirjoitus ilman lukkoa | VW, VM, VL | [AA: H-047] |
| "Lähtöajat" painettaessa | `tall`-lukko vuotaa jokaisesta ohitetusta tietueesta | VW | [AA: H-110] |
| Numerointi keskeytyy | jo numeroidut jäävät voimaan, silti "Toiminto toteutettu" | VW | [AA: H-123] |
| Ajanottolomakkeen *Tallenna muutos* | tietue ei päivity (Q76); L- ja A-rivit muuttuvat maaliriveiksi vain jonossa | VW | [AA: H-111; Q76] |
| Kortti luettu ennen edellisen tallennusta | kortti hylätään ilmoituksella; kortti kirjataan lokiin | VW | `ViestiWin/UnitLuenta.cpp:424-440` |
| Kortti on koodi 200 | "Viallinen Emit-kortti"; kortti on vaihdettava | VW, VL | `UnitLuenta.cpp:370-380`, `laskemtn.cpp:700-710` |
| Kortti varattuna toisen osuuden haltijalle | haltijan koodi vapautetaan automaattisesti vain, jos haltijan osuus on hylätty, keskeytetty, ei lähtenyt tai poissa; muuten kortti on vaihdettava | VW, VL | `UnitLuenta.cpp:385-409` |
| Lukijasäie ei tallenna tallennettavaa korttia | uusi kortti hylätään, kunnes edellinen on tallennettu | VW | `Juk/VIv.cpp:920-931` |
| VM:ssä hajontatuloste ilman `HAJONNAT.LST`:ää | `fclose(NULL)` päättää ohjelman | VM | [AA: H-063] |
| Konsolin partio ja `AUTORAP` ilman listakirjoitinta | NULL-osoitin kortin luennassa | VM | [AA: H-064] |
| Sakkokierroslomake, osuus tyhjä tai > 30 | kirjoitus tietueen `seura`-kenttään tai pinoon, ja tietue lähetetään | VW | [AA: H-082] |
| `ossijat` ylittää puskurin | kasan ylivuoto kun osuustuloksen saaneita on laskuria enemmän | VW, VM, VL | [AA: H-076, H-130] |
| Tuntematon pakettiluokka vastaanotossa | koko vastaanotto pysähtyy | VW, VM, VL | [AA: H-026] |
| Viesti-ohjelman tiedonsiirtopaketti | otsake ei ole mitoituksessa; jonon loppu korruptoituu | VW, VM, VL | [AA: H-025] |
| Kuuluttajatila, TARKT-paketti | `tark_trk` palaa lukko hallussa | VW | [AA: H-024] |
| Hylkäysraportti tai Emit-tuloste lukon alla | `print_CS` jää omistajalle | VW | [AA: H-087, H-088] |
| Lähtöaika asetetaan eri osuudelle kuin valitaan | pudotusvalikon nimiöt siirtyneet kahdella | VW | [AA: H-113] |
| Käynnistyksessä asetustiedoston nimi on pitkä | puskuri 41 `wchar_t` ja kopio 120 merkkiin täytettynä | VW | [AA: H-171] |

**Kutsu ja onnistuminen.** Lomakkeen *Tallenna* kutsuu `tallenna`a, joka kirjoittaa muistiin ja
tiedostoon sekä laittaa paketit lähetysjonoon. Mikään näistä vaiheista ei kerro, että tieto olisi
perillä toisella asemalla; tiedoston kirjoitusvirhettä ei viestissä ilmoiteta (H-015). Sisäänluennassa
"Tallennettu. Lue seuraava viivakoodi" -ilmoitus tulee, kun `Tallenna`-kutsu on palannut
[luettu käsikirjaa varten, ei tarkastettu: `ViestiWin/UnitLuenta.cpp:581-640`].

## 5. Tunnetut viat tässä toiminnossa

<!-- generoitu:viat alku (tietopohja.py lohkot; älä muokkaa käsin) -->
| Toiminto | Havainto | Tila | Prioriteetti | Kuvaus |
|---|---|---|---|---|
| Sarjan osuusrakenne ja rinnakkaisosuudet (sarjatietue, KilpSrj.xml, vSarjat) | [H-042](../vikakortit/H/H-042.md) | avoin | P3 | Viestikilpailun XML-tiedoston osuus- ja väliaikamääriä ei rajata, joten tietuepuskuri voi ylittyä |
| Sarjan osuusrakenne ja rinnakkaisosuudet (sarjatietue, KilpSrj.xml, vSarjat) | [H-002](../vikakortit/H/H-002.md) | avoin | P4 | Vanhan muodon sarjatiedosto (KILP.SRJ) saa viestiohjelman kirjoittamaan tekstiä väärään muistipaikkaan |
| Joukkueen tallennus ja muutosten lähetys (viesti: tallenna) | [H-015](../vikakortit/H/H-015.md) | avoin | P3 | Jos kilpailijatiedoston KILP.DAT kirjoitus levylle epäonnistuu, ohjelma jatkaa muistissa olevilla tiedoilla ja levylle jää vanha tieto |
| Joukkueen tallennus ja muutosten lähetys (viesti: tallenna) | [H-130](../vikakortit/H/H-130.md) | avoin | P3 | Viestissä sarjaa vaihdettaessa joukkuemäärät jäävät vääriksi uudelleenkäynnistykseen asti |
| Joukkueen tallennus ja muutosten lähetys (viesti: tallenna) | [H-142](../vikakortit/H/H-142.md) | avoin | P5 | Nimihakemisto, sijoitusketjut ja laskurit voivat jäädä vääriksi sarjan tai numeron muutoksen jälkeen (kokoomarivi) |
| Joukkueen lisäys ja poisto (viesti: addtall, deltall, addfrec) | [H-040](../vikakortit/H/H-040.md) | avoin | P2 | Viestissä joukkueen lisäys jättää tiedostolukon päälle, ja toisen säikeen lisäys tai poisto voi jumiuttaa ohjelman |
| Joukkueen lisäys ja poisto (viesti: addtall, deltall, addfrec) | [H-148](../vikakortit/H/H-148.md) | avoin | P4 | Viestissä uuden joukkueen lisäys voi levyn täyttyessä jättää joukkueen pois muistista ja tehdä KILP.DAT:sta käyttökelvottoman istunnon ajaksi |
| Joukkueen lisäys ja poisto (viesti: addtall, deltall, addfrec) | [H-109](../vikakortit/H/H-109.md) | avoin | P2 | "Poista kilpailija" ilman valittua kilpailijaa voi vioittaa tietueiden listan ja sekoittaa myöhemmin lisättyjä kilpailijoita |
| Joukkueen lisäys ja poisto (viesti: addtall, deltall, addfrec) | [H-140](../vikakortit/H/H-140.md) | avoin | P3 | Poistetun kilpailijan emit-linkki voi periytyä uudelle kilpailijalle, joka näyttää jo luetulta |
| Joukkueen lisäys ja poisto (viesti: addtall, deltall, addfrec) | [H-047](../vikakortit/H/H-047.md) | avoin | P3 | Sarjanumeroiden siirto, tuonti ja joukkueen lisäys kirjoittavat kilpailijatiedostoon tai sulkevat sen ilman lukkoa, kun muu säie kirjoittaa samaan tiedostoon |
| Joukkueen ja osuuden muokkaus lomakkeella (Joukkuetiedot, Joukkueet) | [H-017](../vikakortit/H/H-017.md) | avoin | P3 | Emit-muutokset-ikkuna voi kirjoittaa kilpailijan vanhan tietueen takaisin ja kumota välissä tulleet muutokset |
| Joukkueen ja osuuden muokkaus lomakkeella (Joukkuetiedot, Joukkueet) | [H-114](../vikakortit/H/H-114.md) | avoin | P3 | Sarjavalikkoon kirjoitettu teksti siirtää joukkueen hiljaa sarjaan 0 (ViestiWin) tai sotkee muistia (HkKisaWin) |
| Joukkueen ja osuuden muokkaus lomakkeella (Joukkuetiedot, Joukkueet) | [H-127](../vikakortit/H/H-127.md) | avoin | P5 | HkKisaWinin ja ViestiWinin pienet logiikkavikoja: "Ei badgea" ilman vakanttia, hidas sarjan selaus, väärin päin annetut argumentit tuloksen tyhjennyksessä ja muita (kokoomarivi) |
| Juoksujärjestyksen vaihto (Joukkuetiedot: BtnJjVaihto, UnitJjVaihdot) | [H-126](../vikakortit/H/H-126.md) | avoin | P5 | ViestiWinin viisi pientä logiikkavikaa: väärä osuuden tulkinta uusintatallennuksessa, alustamaton ilmoitusteksti, kuluva muistiolinja, viallinen merkkitarkistus ja rajaamaton osuuden jäsennys (kokoomarivi) |
| Viestin numerointi ja osuuksien lähtöajat | [H-110](../vikakortit/H/H-110.md) | avoin | P2 | ViestiWin: Lähtöajat-painike jättää kilpailijatietojen lukon päälle, ja vastaanotto- ja lukijasäikeet pysähtyvät ohjelman loppuun asti |
| Viestin numerointi ja osuuksien lähtöajat | [H-118](../vikakortit/H/H-118.md) | avoin | P4 | ViestiWinin "Numeroiden muuttaminen" valmisteluvaiheessa etsii seuraavaa joukkuetta joukkueen numerosta, joten osa joukkueista ohitetaan tai siirretään kahdesti |
| Viestin numerointi ja osuuksien lähtöajat | [H-122](../vikakortit/H/H-122.md) | avoin | P4 | ViestiWinin rankiin perustuva numerointi jättää yli 1000 joukkueen sarjan loput joukkueet numeroimatta ilman ilmoitusta |
| Viestin numerointi ja osuuksien lähtöajat | [H-123](../vikakortit/H/H-123.md) | avoin | P4 | ViestiWinin numerointi voi keskeytyä kesken jättäen osan joukkueista numeroiduiksi, ja ohjelma ilmoittaa silti toiminnon onnistuneen |
| Viestin numerointi ja osuuksien lähtöajat | [H-069](../vikakortit/H/H-069.md) | avoin | P4 | ViestiWinissä kolmea arvoa ei koskaan aseteta, joten numeroinnin tapa, tuonnin lainakorttiehto ja ajanottoruudukon osuuskoodin tulkinta voivat perustua satunnaiseen arvoon |
| Viestin numerointi ja osuuksien lähtöajat | [H-113](../vikakortit/H/H-113.md) | avoin | P3 | Muokattavien valikoiden "Ei valittu" ja kirjoitettu teksti sekä siirtyneet osuusnimiöt kohdistavat toiminnon väärään osuuteen tai väärään paikkaan muistissa |
| Vaihtoajan kirjaus ja osuuden määräytyminen ajanottolomakkeella (viesti) | [H-111](../vikakortit/H/H-111.md) | avoin | P2 | Ajanoton "Tallenna muutos" muuttaa lähtö- tai automaattipisteen rivin maaliriviksi ja voi kirjoittaa lähtökellon ajan kilpailijan maaliajaksi |
| Vaihtoajan kirjaus ja osuuden määräytyminen ajanottolomakkeella (viesti) | [H-121](../vikakortit/H/H-121.md) | avoin | P4 | ViestiWinin ajanotossa Ctrl+Enter ei pakota aikarivin uudelleenlähetystä muille työasemille ja ohittaa ensimmäisen yhteyden |
| Vaihtoajan kirjaus ja osuuden määräytyminen ajanottolomakkeella (viesti) | [H-126](../vikakortit/H/H-126.md) | avoin | P5 | ViestiWinin viisi pientä logiikkavikaa: väärä osuuden tulkinta uusintatallennuksessa, alustamaton ilmoitusteksti, kuluva muistiolinja, viallinen merkkitarkistus ja rajaamaton osuuden jäsennys (kokoomarivi) |
| Vaihtoajan kirjaus ja osuuden määräytyminen ajanottolomakkeella (viesti) | [H-069](../vikakortit/H/H-069.md) | avoin | P4 | ViestiWinissä kolmea arvoa ei koskaan aseteta, joten numeroinnin tapa, tuonnin lainakorttiehto ja ajanottoruudukon osuuskoodin tulkinta voivat perustua satunnaiseen arvoon |
| Kortin siirto toiselle joukkueelle tai osuudelle (Emit-lomake: Muutokset, TallennaVaihto) | [H-112](../vikakortit/H/H-112.md) | avoin | P3 | ViestiWinin "Vaihda kilpailijoiden leimaukset ristiin" voi korvata joukkueen kortin numeron roskatiedolla, jos joukkueen numero tai osuus puuttuu |
| Kortin siirto toiselle joukkueelle tai osuudelle (Emit-lomake: Muutokset, TallennaVaihto) | [H-113](../vikakortit/H/H-113.md) | avoin | P3 | Muokattavien valikoiden "Ei valittu" ja kirjoitettu teksti sekä siirtyneet osuusnimiöt kohdistavat toiminnon väärään osuuteen tai väärään paikkaan muistissa |
| Kortin siirto toiselle joukkueelle tai osuudelle (Emit-lomake: Muutokset, TallennaVaihto) | [H-116](../vikakortit/H/H-116.md) | avoin | P2 | Valinta- tai virheikkunan sulkeminen X-painikkeella tulkitaan painikkeeksi 2: emit-lomakkeissa tuhoava valinta, virheikkunassa ohjelman lopetus |
| Osuuden sakot ja sakkokierrokset (viesti) | [H-082](../vikakortit/H/H-082.md) | avoin | P4 | ViestiWinin sakkokierroslomakkeessa tyhjä tai liian suuri osuusnumero kirjoittaa joukkueen tietueeseen tai sen ohi, ja virheellinen tietue tallennetaan ja lähetetään |
| Sijoitusketjut ja osuussijat (viesti: addjarjarr, ossijat) | [H-076](../vikakortit/H/H-076.md) | avoin | P3 | Viestin osuussijoitusten laskenta ylittää varaamansa muistin XML-tulosteessa, kun joukkueilla on väliaikoja |
| Sijoitusketjut ja osuussijat (viesti: addjarjarr, ossijat) | [H-130](../vikakortit/H/H-130.md) | avoin | P3 | Viestissä sarjaa vaihdettaessa joukkuemäärät jäävät vääriksi uudelleenkäynnistykseen asti |
| Sijoitusketjut ja osuussijat (viesti: addjarjarr, ossijat) | [H-142](../vikakortit/H/H-142.md) | avoin | P5 | Nimihakemisto, sijoitusketjut ja laskurit voivat jäädä vääriksi sarjan tai numeron muutoksen jälkeen (kokoomarivi) |
| Hajonta: ratakoodi osuudella ja tulokset hajontaradoittain | [H-063](../vikakortit/H/H-063.md) | avoin | P4 | Kurssihajontatuloste päättää ViestiMaali-ohjelman, kun tiedostoa HAJONNAT.LST ei ole |
| Hajonta: ratakoodi osuudella ja tulokset hajontaradoittain | [H-087](../vikakortit/H/H-087.md) | avoin | P2 | ViestiWin: emit-tietojen HTML-tulostus tiedostoon jättää tulostuslukot päälle, ja ohjelma jäätyy seuraavan emit-tiedon saapuessa |
| Viestin tulosluettelot ja tulosteet (textjtulos, textosuus, automaattitiedostot) | [H-001](../vikakortit/H/H-001.md) | avoin | P3 | Viestin tekstitulosteen rivi nollaa 120 tavua apumuuttujan jälkeistä muistia, kun osuudelle on määritelty ampumapaikkoja |
| Viestin tulosluettelot ja tulosteet (textjtulos, textosuus, automaattitiedostot) | [H-149](../vikakortit/H/H-149.md) | avoin | P2 | Viestiohjelman automaattinen HTML-tuloste kaataa konsoliohjelman jo ensimmäisellä kierroksella, kun automaattitulostus on päällä |
| Viestin tulosluettelot ja tulosteet (textjtulos, textosuus, automaattitiedostot) | [H-158](../vikakortit/H/H-158.md) | avoin | P3 | Viestin automaattinen XML-julkaisu (asetus XML=) ei kirjoita tuloksia tiedostoon, ja tulostekanavan osoitin jää vapautettuun muistiin |
| Viestin tulosluettelot ja tulosteet (textjtulos, textosuus, automaattitiedostot) | [H-172](../vikakortit/H/H-172.md) | avoin | P3 | Viestin konsoliohjelman tulostusvalikon X)ML-valinta kirjoittaa tiedoston ilman XML-kehystä, joten tiedosto on lähes tyhjä |
| ViestiLuennan supistettu kokoonpano (LUENTA) | [T-052](../vikakortit/T/T-052.md) | avoin | - | ViestiLuenta on supistettu käännös ilman RFID-tukea ja muita ominaisuuksia, eikä rajausta ole dokumentoitu |
| ViestiLuennan supistettu kokoonpano (LUENTA) | [H-176](../vikakortit/H/H-176.md) | avoin | P5 | Kymmenen pientä asetusten ja valitsinten vikaa: osa asetuksista jää huomiotta, tulkitaan väärin tai ei toimi (kokoomarivi) |
| Joukkueen ja osuuden muutosten välitys työasemien välillä (KILPT, OSUUST, VAIN_TULOST, TARKT) | [H-024](../vikakortit/H/H-024.md) | avoin | P2 | Viestin kuuluttajatilassa verkosta tuleva tarkastustilan päivitys jättää kilpailijatietojen lukon päälle, ja ohjelma voi jumiutua |
| Joukkueen ja osuuden muutosten välitys työasemien välillä (KILPT, OSUUST, VAIN_TULOST, TARKT) | [H-025](../vikakortit/H/H-025.md) | avoin | P2 | Viestiohjelmien lähetysjonon paikka on 14 tavua liian lyhyt, joten suurimman sanomatyypin loppuosa ei tallennu jonoon ja voi lähteä väärällä sisällöllä |
| Joukkueen ja osuuden muutosten välitys työasemien välillä (KILPT, OSUUST, VAIN_TULOST, TARKT) | [H-026](../vikakortit/H/H-026.md) | avoin | P2 | Viestiohjelma lakkaa käsittelemästä saapuvia tietoja, kun se saa paketin, jonka lajia se ei tunne |
| Joukkueen ja osuuden muutosten välitys työasemien välillä (KILPT, OSUUST, VAIN_TULOST, TARKT) | [H-036](../vikakortit/H/H-036.md) | avoin | P4 | Viestin asetus KATKKIER kirjoittaa taulukon ulkopuolelle, jos avaimen viimeinen merkki ei ole numero 1–9, koska numeroa ei tarkisteta |
| Partio ja rogaining viestin ytimessä (kilpparam.partio, tark_partio) | [H-064](../vikakortit/H/H-064.md) | avoin | P3 | Viestin konsoliohjelma kaatuu kortin luennassa, kun partiokilpailussa on automaattinen raportti mutta ei listakirjoitinta |

Generoitu tiedostosta `kasikirja/toiminnot.csv` ja rekistereistä (tila, prioriteetti); älä muokkaa käsin.
<!-- generoitu:viat loppu -->

## 6. Mihin tieto perustuu

- Asiakirjat: `doc/arkkitehtuuri/gui-liiketoimintalogiikka.md` luku 4 (HK ja viesti: erot) ja luvut
  3.2-3.8, 5 (lomakkeiden kuvaukset ja uudet havainnot H-109-H-127); `yleiskuva.md` (ohjelmat,
  yhteinen koodi käännetään kahdesti); `globaali-tila.md` luvut 4.1, 5; `tiedostomuodot.md`
  ("KILP.DAT: viestitietue", "Tiedostot"); `tallenna-samanaikaisuus.md` "Viesti";
  `tiedonsiirto.md` (pakettiluokat, "HK ja viesti"); `rekisteri/havainnot.md`, `rekisteri/riskiarvio.csv`
  (ohjelmat), `rekisteri/valitsimet.csv`, `rekisteri/tiedostot.csv` (analyysitasot).
- Käyttöohje (nimet ja asetusten tarkoitus): `docs-site/docs/viestiwin/` kohdat 1.2, 1.4.5, 3.2
  (parisprintti), 4.1, 5.1, 5.4-5.9, 6.7-6.9, 7.1-7.2, 9.4, 10.2. Sivu `5.3_osuuden_m_r_ytyminen_ajanotossa.md` puuttuu
  alkuperäisestä dokumentaatiosta.
- Koodi [luettu käsikirjaa varten, ei tarkastettu, ellei toisin mainita]: `Juk/VDef.h:380-425`,
  `:529-840`; `Juk/vkilp.cpp:77-1740` (tulosfunktiot, tila, sakot); `Juk/vdat.cpp:178-210`, `:285-310`,
  `:470-935` (`addfrec`, `tallenna`, `addtall`, `deltall`, yhteislähtömerkinnät); `Juk/vApu.cpp:191-250`,
  `:566-660`; `Juk/VRinnakkaisOsuus.cpp`; `Juk/VIx.cpp:372-411`, `:611-720`, `:1062-1300`;
  `Juk/VIv.cpp:539-650`, `:920-931`, `:1095-1112`; `Juk/VCom.cpp:260-343`, `:606-700`, `:842-865`,
  `:1050-1170`; `Juk/VInit.cpp:764-772`, `:902`, `:961`, `:1226`, `:1349-1460`, `:2144-2162`;
  `Juk/VTulostus.cpp:8610-8700`; `Juk/VXml.cpp:441-830`, `:1760-1850`; `JukConsole/laskenta.cpp`,
  `JukConsole/vcons2.cpp:1495-1885`, `JukConsole/laskent0.cpp:406-640`, `JukConsole/laskemtn.cpp:296-780`;
  `ViestiWin/UnitJoukkuetiedot.cpp:1402-1499`, `UnitLuenta.cpp:370-665`, `UnitYLahdot.cpp:100-260`,
  `UnitMain.cpp:236-282`, `:1744-1826`, `UnitKilpMaaritys.cpp:100-111`.
- Käännösvalitsimet ja projektit: `VS/V/JukMaali520.vcxproj` (VM; `Release`: 30/250/12),
  `VS/V/JukLuenta520.vcxproj` (VL), `RADStudio10/ViestiWin.cbproj` (VW), `.github/workflows/release.yml`.
  Konsolin ja GUI:n erottelu: `#ifdef _CONSOLE`; ViestiLuennan erottelu: `#ifdef LUENTA`.
- Toimintokytkentä: `kasikirja/toiminnot.csv`.

## 7. Mitä ei ole selvitetty

- **Ehdokkaat (ei tarkastettu).** Seuraavia ei ole löytynyt `havainnot.md`:stä eikä `seuranta.md`:n
  KE-taulukosta; ne on luettu tätä lukua varten eikä niistä ole annettu havaintotunnusta.
  1. *Kilpailun määritysten alalaji ja partio* (`ViestiWin/UnitKilpMaaritys.cpp:100-111`): koodissa on
     `if (alalaji == 'P') {... partio = 2 ...}` ja heti perään erillinen `if (alalaji == 'R') {...} else
     { partio = 0; rogaining = false; }`. Alalajilla `P` jälkimmäinen `else` ajetaan, ja `partio` palaa
     nollaksi. XML:n luku (`Juk/VXml.cpp:562-566`) asettaa arvon uudelleen alalajista, joten vaikutus olisi
     voimassa vain kunnes tapahtumatiedot luetaan XML:stä uudelleen (esimerkiksi käynnistyksessä) [päätelty koodista; ei ajettu; tiedosto
     analyysitasolla 1].
  2. *Juoksujärjestyksen vaihto* (`BtnJjVaihto`, kohta 101): osuuskoodeille ei tarkisteta, että jokainen
     kohdeosuus esiintyy vain kerran; jos kaksi riviä saa saman osuuskoodin, myöhempi korvaa aiemman,
     ja osuus, jolle ei osoiteta mitään, säilyttää vanhan juoksijansa, joten sama juoksija voi esiintyä
     kahdesti ja toinen puuttua. Virheellinen osuuskoodi ohitetaan hiljaa [päätelty koodista; ei ajettu].
     Samassa funktiossa oleva `wst[wcslen(wst-1)]` on jo H-126:ssa.
- **Ristiriita käyttöohjeen ja koodin välillä: `VUOROMAALI`.** Käyttöohje (viestiwin 5.4) sanoo
  "Aikaa ei kirjata väliaikapisteiden täytyttyä vaihdolle/maalille, ellei ole annettu parametria
  VUOROMAALI, joka kieltää antamasta aikaa vaihdolle/maalille." Koodi (`Juk/VInit.cpp:1457`, kommentti
  "Antaa ajan maaliajaksi, kun väliajat ovat täynnä"; `Juk/VIv.cpp:626`) valitsee asetuksella pisteeksi
  0, kun väliajat ovat täynnä. Luku 04 kuvaa koodin mukaisen toiminnan (`valuku+1` ilman asetusta). Kumpi on tarkoitettu, ei ole ratkaistu [tuntematon]; vain tunnistinpolku
  (`tall_etulos`) on luettu.
- **Osuuden ja pisteen päättely**, ajanottolomakkeen ydinpolku (Q76), Emit-lomakkeen sisäinen toiminta
  viestissä ja `tark_partio` ovat luvuissa 04-05 tai lukematta.
- **Rinnakkaisosuuksien käytäntö kilpailussa.** Koodin rakenne (`ekaMaaliRatkaisee`,
  `lahtoEdEkaMaalista`, `puutelisa`) on luettu, mutta aineisto ei kerro, mitä kilpailumuotoja ne
  tarkoittavat (esimerkiksi parisprintti, [KO: viestiwin 3.3]) eikä sitä, miten niitä käytetään
  oikeissa kilpailuissa [tuntematon].
- **Luennan testitila** (`LUENTATESTI`, `emitrek.lst` / `EMITREK.LST`): luettu vain pääpiirteissään.
- **Lähtölaji 2** (`osrec::lahtolaji`): käyttöohje mainitsee vain automaattisen ja yksilöllisen; koodi
  hyväksyy arvot 0-2 [tuntematon].
- **Partio ja rogaining**: tulosfunktioiden haarat on luettu, mutta lomake `Partio`, `tark_partio` ja
  rogainingin laskenta ovat lukematta ([AA: luku 05 kohta 7]).
- **Viestin vastaanottopolku kokonaisuutena** (`tark_osuus`, `tark_kilp` jne.) on vain osittain luettu
  [AA: `tallenna-samanaikaisuus.md`]; Q76, Q77 (haarojen ikä) ja Q111 (RFID ViestiLuentaan, omistajan
  päätös) ovat avoimia.
- **Sisäänluenta VM:ssä**: parametrin `SISÄÄNLUENTA` vaikutusta VM:ssä ei ole selvitetty; kortin
  vastaanottohaara (`addluenta`) on käännöksessä vain GUI:ssa (`#ifndef _CONSOLE`).
- **Julkaisutyönkulku**: mikä `150os`-kokoonpano on toimitettu käyttäjille, ei ole varmistettu (T-051).
