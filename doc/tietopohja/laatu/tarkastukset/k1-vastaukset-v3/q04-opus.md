**Vastaus:** HK-ohjelmissa (HkMaali, HkKisaWin) kortin luenta kulkee kahdessa vaiheessa. Ensin lukijasäie purkaa lukijan sanoman, suodattaa toistetun luvun ja kirjoittaa kortin emit-tiedoston jonoon. Sen jälkeen käyttöliittymän kutsuma `pros_emit` ottaa jonosta seuraavan käsittelemättömän kortin. Se etsii kortin numerolla kilpailijan, tarkastaa leimat, päättelee maaliajan ja tallentaa kilpailijan tuloksen. Tallennus päivittää muistin, levyn, järjestykset ja muille työasemille lähetettävän sanoman. Auditointi löysi tältä polulta useita avoimia havaintoja, joista tärkein on H-145.

**Perustelut (vaiheittain, HK):**
1. **Lukijasäie.** `lue_regnlyThread` avaa sarja-, UDP- tai TCP-yhteyden ja kutsuu `lue_regnly`-funktiota, joka kokoaa sanoman — `Tp/TpLaitteet.cpp:1747-1786`, `:1515-1574` [luettu koodista nyt]. `lue_regnly` välittää sanoman laitekohtaiselle lukufunktiolle (`lue_LUKIJA`, `lue_MTR`, `lue_EmitKello`, `lue_EMITAG`), joka kutsuu `tall_emit`-funktiota — `todisteet/c8/osaD-saikeet.md` rivi 28 [dokumentoitu; välivaiheita en lukenut itse].
2. **Sanoman purku.** `tall_emit` muuntaa sanoman kortin tietueeksi (`emittp`): kortin numero, rastikoodit ja leimausajat.
   - Lukuhetken aika tulee laitteen mukaan: Emit-kellossa sanomassa on maaliaika (`Hk/HkIV.cpp:978-1016`), Emit-lukijassa aika haetaan korttiaikataulusta `hae_bdg_t` (`:1018-1044`) ja MTR:ssä se on laitteen kello (`:1045-1072`). [luettu koodista nyt]
   - Jos `emitTunnistus` on päällä, korttia ei tallenneta, vaan se vain tunnistetaan (`:1090-1093`). [luettu koodista nyt]
3. **Toiston suodatus.** Jos sama kortti tulee samalta lukijalta samoin leimoin, se vain kuitataan (`vahvistus`). Muuten kortti talletetaan (`addem`). Funktio palauttaa aina 0. — `Hk/HkIV.cpp:1145-1152` [luettu koodista nyt]
4. **Emit-tiedostoon.** `addem` kirjoittaa kortin emit-lukon alla tiedoston loppuun (`putem`) ja kasvattaa tietuelaskuria `emithead`. Asetuksella `estaemittoisto` toisto voidaan torjua myös `on_emit`-tarkistuksella. — `Hk/HkEmit.cpp:1657-1689`, `:1600-1625` [luettu koodista nyt]. Saman tiedoston toinen `addem`/`putem` (`:1275-1377`) on `TALLEMIT`-haarassa, joka on merkitty valitsinrekisterissä tilaan `dead-never-defined` (ei koskaan määritelty), joten sitä ei käännetä [dokumentoitu].
5. **Käsittelyjono.** Kortti jää odottamaan, koska sen kirjausaika on vielä tyhjä (`Hk/HkIV.cpp:977`). `pros_emit` käsittelee yhden kortin kerrallaan osoittimen `emithead2` kohdalta, kun `ProsEmitOn` on päällä (`:1184-1195`). [luettu koodista nyt]
   - Konsolissa (HkMaali) kutsu tulee emit-näkymän silmukasta, `Hk/HkEmit.cpp:3867` (`#ifdef _CONSOLE`). [luettu koodista nyt]
   - GUI:ssa (HkKisaWin) kutsu tulee lomakkeelta `cbHk/UnitEmit.cpp:936`; mikä tapahtuma kutsun käynnistää, jäi tarkistamatta. [luettu koodista nyt]
6. **Kilpailija ja leimantarkastus.** `tark_badge` hakee kilpailijan kortin numerolla (`bdg2kno`) ja tarkastaa leimat (`tarkista`). Hyväksytty saa merkinnän `T`, virheellinen merkinnän `I` ja kolme äänimerkkiä, ja tieto tallennetaan (`tallenna`). — `Hk/HkEmit.cpp:2263-2313` [luettu koodista nyt]
   - Konsolin ja GUI:n kysymykset eroavat, esimerkiksi kun kortti on luettu jo toiselle kilpailijalle (`Hk/HkIV.cpp:1204-1623`, `#ifdef _CONSOLE` / `#else`) [luettu koodista nyt; haaroja en lukenut kokonaan].
7. **Maaliaika ja tulos.** [luettu koodista nyt]
   - Asetukset `autosarja`/`autorata` voivat vaihtaa sarjan tai radan (`:1675-1698`).
   - Jos radan maalilaji on 4, maaliaika lasketaan kortista (`e_maaliaika`, `:1700-1745`).
   - Emit-kellon maaliaika viedään ajanottojonoon (`tall_rivi`, `:1747-1778`). Asetuksella `autokilp` kortti liitetään seuraavaan nimeämättömään ajanottoriviin (`:1779-1802`).
   - Lopuksi tulos asetetaan (`set_tulos`) ja kilpailija tallennetaan lukon `tall_CriticalSection` alla (`:1803-1823`).
8. **Jälkityöt.** [luettu koodista nyt]
   - Kortin tietue päivitetään kilpailijanumerolla ja lähetetään muille työasemille (`putem`, `setEmitJarr`, `laheta_em`, `:1845-1852`).
   - Sen jälkeen näytetään tulos, lasketaan emit-väliajat, varoitetaan lainakortista, tulostetaan automaattinen rasti-raportti ja varoitetaan, jos kilpailijalla ei ole aikaa (`:1853-1863`).
   - Lopuksi `emithead2` siirtyy seuraavaan korttiin (`:1864`).
9. **Tallennus.** `kilptietue::tallenna` päivittää muistin, `KILP.DAT`-tiedoston, järjestykset ja lähtevän sanomajonon — `yleiskuva.md`, luku "Päävirta" [dokumentoitu].

**Auditoinnin havainnot tällä polulla** (kaikki tilassa `avoin`):
- **H-145** (P3, KK-C9-2): jos emit-tiedoston kirjoitus epäonnistuu, `tall_emit` palauttaa silti 0. Emit-kello saa kuittauksen, ja kortti putoaa pois saman istunnon ajaksi. — `riskiarvio.csv` [auditointihavainto]
  - Vaikutukset (`vaikutukset.csv`): `tulokset = mahdollinen`, `leimaukset = vahvistettu`, `tietojen_sailyminen = vahvistettu`, `kiertotapa = ei_tiedossa`.
  - Palautuminen: riskiarvion sarake `palautettavuus` = `kasin` (käsin syöttö, tai uudelleenkäynnistys ja kortin uusi luku); keinoja ei ole kokeiltu.
  - Nykykoodi vastaa kuvausta: `Hk/HkIV.cpp:1150-1152` [luettu koodista nyt].
- **H-168** (P4): toiston tarkistus `samatleimat` vertaa leimausaikaa itseensä. Siksi saman kortin aito uusi luku samoin rastikoodein hylätään ja kuitataan. Sama virhe näkyy nykykoodissa: `Hk/HkIV.cpp:949`. [auditointihavainto]
- **H-136** (P3): korttiaikataulua `hae_bdg_t` käsitellään ilman lukkoa, kun lukijasäikeitä on vähintään kaksi. [auditointihavainto]
- **H-137** (P3): muualta välitetty, jo käsitelty kortti voi saada `pros_emit`-käsittelyn ohittamaan yhden paikallisen kortin. Kortti jää emit-tiedostoon ja voidaan käsitellä käsin. [auditointihavainto]
- **H-105** (P3): vaiheen vaihdon jälkeen emit-tiedoston kahva jää vanhentuneeksi, kun käytössä on ESITARK tai ESILUENTA. [auditointihavainto]
- **H-092** (P3): emit- ja tulostuslukko voivat lukita toisensa ristiin (ABBA-lukkiutuminen). [auditointihavainto]

**Koskee ohjelmia:** HK: HkMaali (konsoli, MSVC) ja HkKisaWin (GUI, bcc32). Viestiohjelmissa (ViestiMaali, ViestiLuenta, ViestiWin) on omat vastineet: `Juk/VIv.cpp:947` `tall_emit`, `:1118` `pros_emit`, kutsukohta `Juk/VEmit.cpp:4534`. Rakenne on samanlainen [päätelty `yleiskuva.md`:stä ja funktioiden nimistä; viestin koodia en lukenut].

**Tarkistukset (luku 4b):**
- **Havainnot:** haettu `tall_emit pros_emit addem putem tark_badge hae_bdg_t lue_regnly estaemittoisto samatleimat` tiedostoista havainnot.md ja seuranta.md → H-087, H-092, H-102, H-103, H-105, H-136, H-137, H-145, H-157, H-168. Tilat ja prioriteetit on luettu `riskiarvio.csv`:stä, ja kaikki ovat avoimia. Polulla mainitut ovat yllä. H-087 ja H-103 koskevat vain viestiohjelmia ja H-102 vaiheen vaihtoa, joten ne jäivät pois. H-157 on koontirivi (P5), jonka `samatleimat`-kohta on H-168. Uusia havaintoehdokkaita ei ole.
- **OP/Q:** haettu `emit lukija luenta kortti` → OP-13 (laitteet ja ulkoiset palvelut) avoin. Seurannassa Q86 (Emit-kellon kuittaus) on koodin osalta vastattu ja laitteen osalta siirretty Q100:aan, joka on avoin.
- **Lukumäärät:** ei lukumääriä.
- **Käännösehdot:** `tall_emit` ja `pros_emit` ovat `EMITLEIMAT`-haarassa, joka on valitsinrekisterin mukaan kaikissa viidessä ohjelmassa; `TALLEMIT` on kuollut haara (`dead-never-defined`); konsolin ja GUI:n haarat on eroteltu yllä.

**Tietolaji:** A (nykyinen koodi) kulun kuvauksessa; C (auditointihavainnot) havaintojen kohdalla. Yhtään korjausta ei ole kirjattu (D). Korjauspaketit KK-C9-2, KK-C10-2 ja KK-C13-x ovat suunnitelmia, joita ei ole toteutettu (E).

**Rajoitteet:**
- Laitekohtaisia lukufunktioita (`lue_LUKIJA` ym.), `tarkista`-funktion leimantarkastusta, konsolin ja GUI:n kysymyshaaroja ja viestiohjelmien polkua en lukenut kokonaan.
- `CHECKPOINT.md`:n mukaan tuotantokoodi ei ole muuttunut auditoinnin revision `f7fe553` jälkeen, ja rekisterin rivinumerot vastaavat lukemaani koodia.
- Mitään ei ole ajettu.
