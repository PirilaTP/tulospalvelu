# Perusvastaus: Emit-luenta (agentti ennen kehitystä)

Ajettu 2026-11-27 `pirila-ask.sh`-vastaavalla käynnistyksellä (Sonnet). Työkalukutsut: Grep/TPsource 5, Grep/doc 10, Read/TPsource 13, Read/doc 3

**Vastaus:** Alla on kuvaus yksilökilpailun (HK) polusta. Se perustuu koodiin, jonka luin nyt. Viestiohjelmissa on samanlainen rakenne (`Juk/VIv.cpp:947` `tall_emit`, `:1118` `pros_emit`, `Juk/VCom.cpp:792` `laheta_em`), mutta en käynyt sitä läpi rivi riviltä. Kortin luenta ja tallennus kulkevat kahdessa vaiheessa. Lukijasäie tallettaa ensin raakaleimat Emit-tiedostoon. Vasta sen jälkeen `pros_emit` liittää kortin kilpailijaan, laskee ajat, tallentaa kilpailijatietueen ja lähettää muutokset.

**Perustelut:** (vaiheet ajan mukaan, luettu koodista nyt, ellei toisin mainita)

1. **Lukija ja tulkinta.** Lukijasäie (`lue_regnlyThread`, yksi per lukija) kutsuu `tall_emit`-funktiota (`Tp/TpLaitteet.cpp:911`, myös `:1098`, `:1248`, `:1365`, `:1475`). Jos lukija on tyyppiä yli 10, kutsutaan `tall_emit`, muuten `tall_regnly`. Kuittaus lukijalle annetaan sen mukaan, palauttiko funktio virheen (`:910-917`). [luettu koodista nyt]
2. **`tall_emit` muodostaa `emittp`-tietueen** (`Hk/HkIV.cpp:955`).
   - Lukijatyyppi (MTR, emiTag, SportIdent) määrää, miten kortin koodi, lukuaika ja leimat poimitaan (`:1045-1133`). emiTagille vaaditaan lisäksi, että leimojen lopussa on lukijakoodi (`:1081-1087`).
   - Lukuaika normalisoidaan (`:1135-1136`).
   - Jos tunnistustila on päällä (`emitTunnistus`), kortti vain näytetään (`emitLuettu`) eikä sitä tallenneta (`:1090-1093`).
   - SportIdent-haara on käännetty vain, jos `SPORTIDENT` on määritelty (`:1094`). En tarkistanut, kääntääkö jokin ohjelma sen.
3. **Kaksoisluennan esto.** Jos kortin koodi ja leimat ovat samat kuin saman lukijan edellisellä luennalla, kortti vain kuitataan eikä sitä lisätä (`:1145-1147`). Muuten se lisätään kutsulla `addem(&em, 0)` (`:1148-1150`). [luettu koodista nyt]
4. **`addem` (`Hk/HkEmit.cpp:1657`).** Funktio on käännetty, kun `EMIT` on määritelty (`:1549`).
   - Se ottaa `emit_CriticalSection`-lukon ja tarkistaa mahdollisen toiston (`estaemittoisto`, `on_emit`).
   - Se kirjoittaa tietueen Emit-tiedoston loppuun (`putem`, `:1669`), kasvattaa `emithead`-osoitinta ja päivittää kortin ja kilpailijan linkin (`setEmitJarr`, `:1670-1673`).
   - Jos käsittelyosoitin oli ajan tasalla, se siirtyy uuden tietueen ohi (`:1679-1680`).
   - Se välittää raakatietueen muille asemille (`laheta_em`, `:1681`) ja ajaa tarvittaessa automaattiraportin ja emit-analyysin.
   - Vastaanottajan puolella saapuva EMITT-paketti menee samaan `addem`-funktioon (`Hk/HkCom32.cpp:790-798`).
5. **Käsittelyn käynnistys.**
   - GUI (HkKisaWin): `ProsEmitThread` (`cbHk/WinHk.cpp:1611-1639`) kysyy, onko käsittelemätöntä tietuetta (`FormEmit->Tietue >= emithead-1`). Jos on, se kutsuu `HaeUusiTietue`. Tämä lähettää `SendMessage`-viestin pääsäikeelle, ja `NewCardHandler` (`cbHk/UnitEmit.cpp:880`) kutsuu `pros_emit(&EKilp, msgtxt)` (`:936`).
   - Käsittely ei käynnisty, jos vaiheenvaihto on kesken tai lomake ei ole valmis.
   - Käsittely ei myöskään käynnisty, jos lomakkeella on tallentamattomia muutoksia. Silloin näytetään "Uusi kortti, kun muutoksia tallentamatta" (`:909-912`).
   - Konsoli (HkMaali): pääsilmukka kutsuu `pros_emit(0,0)` (`Hk/HkEmit.cpp:3867`).
6. **`pros_emit` (`Hk/HkIV.cpp:1162`).** Funktio käsittelee yhden tietueen kerrallaan (`in_pros_emit`, `:1184-1186`).
   - Se lukee tietueen kohdasta `emithead2` (`:1187`). Tietue, jonka `kirjaus` ei ole `TMAALI0/KSEK`, ohitetaan (`:1188-1192`).
   - Sen jälkeen se tunnistaa kilpailijan kortin koodista (`bdg2kno`, `:1540`).
   - Jos kortti on jo luettu tai kilpailijalla on jo merkintä, GUI-haara kirjaa varoituksen (`:1550-1571`). Konsoli kysyy S/T/O (`:1285-1338`).
   - Jos korttia ei tunneta, kortti sijoitetaan vakanttipaikalle tai luodaan uusi vakanttitietue (`:1577-1622`, GUI). Konsolissa kysytään kilpailijanumero, haku tai lisäys (`:1340-1517`).
   - Koodi 200 (käyttäjän "menettämä" koodi) korvataan uudella vapaalla koodilla, tai se estetään esiluennassa. GUI-haara on `:1523-1538` ja konsolihaara `:1205-1274`.
7. **Tarkistus ja ajat.**
   - `tark_badge` (`:1633`) tarkistaa leimat rataa vasten.
   - Jos rata puuttuu tai kortin kello on nollautunut, ilmoitus näytetään (`:1633-1653`).
   - Automaattinen sarjan tai radan vaihto (`autosarja`, `autorata`) tehdään kohdissa `:1675-1698`.
   - Jos radan `maalilaji == 4`, maaliaika lasketaan kortin leimoista (`e_maaliaika`, `:1700-1746`). Lähtöaika asetetaan, jos rata sallii (`:1716-1740`).
   - Maaliaika korjataan aikakorjauksella ja kirjataan aikajonoon (`tall_rivi`, `:1748-1775`) ja kilpailijan tulokseksi (`set_tulos`, `:1803-1819`).
8. **Kilpailijan tallennus.** `kilp.tallenna(d, TRUE, 0, 0, &entkilp)` kutsutaan `tall_CriticalSection`-lukon sisällä (`HkIV.cpp:1820-1822`).
   - Funktio (`Hk/HkDat.cpp:897`) vertaa uutta tietuetta vanhaan (`kilpcmp`, `:979`).
   - Se päivittää sijoitusindeksit (`:1024-1027`, `:1202-1207`, `:1224-1230`) ja nimi-indeksin (`:1031-1038`).
   - Se päivittää Emit-linkit, jos koodi muuttui (`:1042-1189`).
   - Se kirjoittaa tietueen levylle ja muistitaulukkoon (`PUTREC(d, 1)`, `:1211`) ja päivittää tapahtumanäytön (`addtapahtuma`, `:1245-1249`) sekä ammuntasakot (`add_sakko`, `:1250-1251`).
   - Se kirjaa lokiin, jos loki on päällä (`:1348-1372`), ja kasvattaa automaattitulostuksen laskureita (`:1324-1344`).
   - Jos tarkastus johtaa esitarkastukseen tai esiluentaan (`esitark`, `esiluenta`), tallennus tehdään lyhyemmällä haaralla (`HkIV.cpp:1825-1833`).
9. **Välitys muille asemille.** Jos yhteys on käytössä (`comfl`), `tallenna` kutsuu `laheta` jokaiselle muutokselle: perustiedot (KILPT), päiväkohtaiset tiedot (KILPPVT) ja tulos- tai tarkastustiedot (VAIN_TULOST) (`HkDat.cpp:1276-1309`). `laheta` (`Hk/HkCom32.cpp:167`) laittaa paketin jokaisen avoimen yhteyden lähtöjonoon (`buflah`). `outb_CriticalSection` suojaa jonoa (`:186-236`). Yhteyttä, josta muutos tuli, ei lähetetä takaisin (`kielto`, `:188-189`).
10. **Lopuksi** (`HkIV.cpp:1845-1867`):
    - `putem` kirjoittaa päivitetyn Emit-tietueen, jossa on nyt kilpailijanumero.
    - Kortin ja kilpailijan linkki päivitetään (`setEmitJarr`).
    - Päivitetty tietue lähetetään (`laheta_em(&em,1,0)`), ja Emit-analyysi päivitetään tarvittaessa.
    - Näyttö päivitetään (`naytatulos`).
    - Väliaikalaskenta ajetaan (`laskeEm_VA`), jos sarjalle on laskenta päällä. Tämä tehdään lukon ulkopuolella.
    - Lainakortin varoitus, automaattiraportti ja "ei aikaa" -ilmoitus tehdään ehtojen mukaan.
    - Käsittelyosoitin `emithead2` kasvatetaan ja `in_pros_emit` nollataan.

**Auditointihavainnot, jotka koskevat tätä polkua** (prioriteetit ja tilat haettu `riskiarvio.csv`:stä tällä ajolla):
- **H-011** (P3, avoin, KK-C13-6): Emit-tiedosto kirjoitetaan ilman `emit_CS`-lukkoa, joten lukijasäikeen kirjoitukset voivat lomittua. Vaikutus on päätelty. Esimerkiksi `HkIV.cpp:1331`, `:1504` ja `:1845` kutsuvat `putem(..., 0)`. Rivin 0 antaa funktiolle `putem` ohjeen ottaa lukko itse (`HkEmit.cpp:1605-1606`), joten tämä näyttää ristiriitaiselta auditointikuvauksen kanssa. Kyse voi olla toisista kutsupaikoista (`putem(...,1)`: `HkDat.cpp:1114`, `:1146`, `:1169`). En selvittänyt asiaa tarkemmin. [dokumentoitu; ristiriita luettu]
- **H-137** (P3, avoin, KK-C13-6): eteenpäin välitetty jo käsitelty kortti voi saada paikallisen kortin ohittumaan automaattikäsittelystä. Kortti jää Emit-tiedostoon ja sen voi käsitellä käsin. [dokumentoitu]
- **H-007** (P2, avoin, KK-C13-10): vastaanotettu paketti voi kirjoittaa yli paikallisen muokkauksen, ja virhe leviää muille asemille. [dokumentoitu]
- **H-016** (P3, avoin, KK-C13-23): "viimeinen kirjoittaja voittaa" -protokolla ilman versiota. [dokumentoitu]
- **H-034** (P4, avoin, KK-C13-23): yhteyden katkon aikana matkalla ollut paketti voidaan soveltaa kahdesti. [dokumentoitu]
- **H-010** (P3, avoin, KK-C13-6): noin 35 `tallenna`-kutsupaikkaa on ilman lukkoa. `HkIV.cpp:1608` on tällainen paikka (vakanttipaikan tallennus). [dokumentoitu; rivi luettu]
- Pakettiin KK-C13-6 kuuluvat lisäksi H-045, H-047, H-051, H-136 ja H-144. Avoimet kysymykset Q69 ja Q81 liittyvät pakettiin (Q81: useita lukijoita yhdellä HK-instanssilla, avoin). Mitään näistä ei ole korjattu (kaikki `tila` = `avoin`).

**Omistajan päätökset ja kysymykset:** OP-13 (laitteet ja lukijan kuittauksen merkitys) on avoin. OP-9 (kuntosuunnistuksen Emit-koodin vaihto vain GUI:ssa) on avoin. Muuta tähän aiheeseen liittyvää en löytänyt. Korjauksia ei ole toteutettu, ja ne ovat tietolajia E (suunniteltu).

**Koskee ohjelmia:** HkKisaWin (GUI) ja HkMaali (konsoli) yksilökilpailussa. Konsolin ja GUI:n haarat on eroteltu kohdissa 5, 6 ja 8. Viestiohjelmat vain viitteenä.

**Tarkistukset (luku 4b):**
- Havainnot: haettu `emit`, `addem`, `pros_emit`, `putem`, `tallenna`, `laheta` aiheesta havainnot- ja riskiarvio-rekistereistä. Poimin tunnukset H-007, H-010, H-011, H-016, H-034, H-045, H-047, H-102 ja H-137. Seuranta.md:stä haettu Q60/Q69/Q81. Q60 ja Q69 rivejä en löytänyt omalla hakullani.
- OP/Q: haettu `emit`/`Emit` tiedostosta `omistajan-paatokset.md`. Osumat OP-9 (avoin) ja OP-13 (avoin). Q81 avoin.
- Lukumäärät: dokumentin "145 kutsupaikkaa, ~110 lukon kanssa" (`tallenna-samanaikaisuus.md`) ja "~35 lukitsematonta" ovat auditointilukuja. Niitä en laskenut koodista, joten en käytä niitä tässä.
- Käännösehdot: `addem` on `#if defined(EMIT)` -haarassa (`HkEmit.cpp:1549`). `pros_emit`n GUI-haara on `#else // !_CONSOLE` (`HkIV.cpp:1519-1623`) ja konsolihaara `#ifdef _CONSOLE` (`:1204-1518`). SportIdent-haara vaatii `SPORTIDENT`-määrittelyn. En tarkistanut `valitsimet.csv`:stä, onko mikään toimitettava ohjelma määritellyt sitä. Kuvattu `addem`-versio on käännettyjen haarojen mukainen. `HkEmit.cpp:1333` on toinen `addem`-määritys `EMIT && TALLEMIT` -ehdon sisällä, enkä kuvannut sitä.

**Tietolaji:** A (luettu koodista nyt) polun kuvaus; C (auditointihavainnot) H-numeroidut kohdat; E (suunniteltu) OP/KK-viitteet.

**Rajoitteet:**
- Vastaus perustuu kohdennettuun lähdekoodin tarkasteluun. `Hk/HkIV.cpp` ja `Hk/HkDat.cpp` ovat auditoituja vain osittain.
- Revisio: CHECKPOINT.md:n mukaan tuotantokoodi ei ole muuttunut analysoidun revision `f7fe553` jälkeen.
- Jätin pois: Emit-tiedoston alustuksen, `laskeEm_VA`-funktion sisällön (väliaikalaskenta), `tark_badge`- ja `e_maaliaika`-funktioiden yksityiskohdat, TCP/UDP-kuljetuksen ja vastaanottopään pakettien soveltamisen (`tark_kilp`) sekä `buflah`-lähetyksen jonon tyhjennyksen.
- Kohdassa 5 luin koodista vain GUI:n `ProsEmitThread`-funktion ja konsolin `HkEmit.cpp:3867`-kutsun. Konsolin pääsilmukkaa en lukenut.
- Tässä vastauksessa ei ole uusia havaintoehdokkaita.
- Sain ajon aikana muistutuksen siitä, että vastaus viipyy. Vastaus on tästä syystä tiivistetty.