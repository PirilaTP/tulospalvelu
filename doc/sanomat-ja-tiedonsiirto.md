# Sovelluksen sanomat ja tiedonsiirto

> Tila: kartoitus 2026-11-19. Kohde: `main` (f7fe553) ja vertailuna haara `feature/sportident-reader`
> (paikallinen viite, viimeisin commit 51a17a3). Laajuus: HkKisaWin ja ViestiWin. Konsoliohjelmat
> mainitaan vain, kun jaettu koodi käyttäytyy niissä eri tavalla.
>
> Dokumentin tarkoitus:
> 1. Kuvata kaikki sanomat, joita sovellus lähettää ja vastaanottaa, ja erityisesti **niiden rooli ja
>    käyttötarkoitus** kokonaisuudessa.
> 2. Varmentaa, miten REST-rajapinnan (`doc/rest-api-vaatimukset.md`) toiminnot kytkeytyvät olemassa
>    olevaan tiedonsiirtoon (luku 11).
> 3. Varmentaa SportIdent-haaran aiheuttamat muutokset sanomatasolla (luku 12,
>    `doc/sportident-review-items.md`).
>
> Rivinumerot viittaavat `main`-haaraan hakemistossa `TPsource/V52/`, ellei toisin mainita.

## 1. Kokonaiskuva

Sovelluksessa on seitsemän sanomaperhettä:

| # | Sanomaperhe | Suunta | Kuljetus | Rooli |
|---|---|---|---|---|
| A | Koneiden välinen tiedonsiirto (`combufrec`-paketit) | molemmat | UDP, TCP, RS-232 | Pitää kilpailun tiedot samoina kaikilla verkon koneilla (replikointi) |
| B | XML-tulossanomat | ulos | TCP (asiakas/palvelin) | Syöttää tulokset ja väliajat ulkoiselle tulospalvelulle tai näytölle |
| C | Tiedostojen siirto | molemmat / ulos | `combufrec` FILESEND (UDP), TCP STX…ETX | Jakaa tiedostoja (esim. asetukset ja tulosteet) koneille tai vastaanottajalle |
| D | Ajanotto- ja leimauslaitteiden sanomat | sisään (osin kyselyt ulos) | RS-232/USB, UDP, TCP | Tuottaa ajat, leimat ja kortinluennat |
| E | Ulkoisten palveluiden haut | sisään (pyyntö ulos) | HTTP(S), SQL | Hakee radioleimat (emiTag, SI Center) ja kortinluennat (emitSQL) |
| F | Muut lähtevät ilmoitukset | ulos | UDP broadcast, UDP, TCP, RS-232, SQL | Tulostaulut, valot, monitori, TCP-loki, MySQL-tietokanta |
| G | Sovelluksen sisäiset ikkunaviestit (`WM_MY…`) | sisäinen | Windows `SendMessage` | Siirtää taustasäikeiden ilmoitukset käyttöliittymäsäikeelle |

```
  Ajanottolaitteet  Leimauslaitteet / RFID      emiTag / SI Center (HTTP)   emitSQL
        |  D               |  D                         |  E                    | E
        v                  v                            v                       v
   tall_rivi()        tall_emit()/addem()        tall_etulos()/siParsePunch()  (luentatiedot)
   (aikajono)         (emit-tietueet)             -> tallenna()
        |                  |                            |
        +--------> laheta_aika / laheta_em / laheta (tallenna) <--- käyttöliittymä, REST-syötöt
                                   |
                          buflah(): lähtevä jono / COMFILE
                                   |  A  (SOH-kehys, ACK/NAK)
                 +-----------------+-----------------+------------------+
                 v                 v                 v                  v
           Muu kone (UDP)    XML-yhteys (B)    Tiedostosiirto (C)   Monitori, broadcast,
           tarkcom()         lahetaXML_TCP     sendtcpfile          TCP-loki, taulut (F)
           -> tallenna(kielto=yhteys)
```

## 2. Koneiden välinen tiedonsiirto: kehys ja jonot

### 2.1 Paketin rakenne

Paketti on `combufrec` (HK `Hk/HkDef.h:1235`, viesti `Juk/VDef.h`, rakenne "Tiedonsiirtopaketti").

| Kenttä | Merkitys |
|---|---|
| `lahetetty` | Paikallinen kenttä: levypuskurissa lähetysaika (1/100 s), 0 = lähettämättä. Ei siirry kehyksessä |
| `portti` | Paikallinen kenttä: vastaanottoyhteyden numero (1…) |
| `so` | `SOH` datapaketissa, `ACK`/`NAK` kuittauksessa |
| `id`, `iid` | Juokseva paketin numero ja sen komplementti (`id + iid = 255`) |
| `pkgclass` | Sanomaluokka (luvut 3 ja 4). HK: `char`, viesti: `UCHAR` |
| `len` | Datan pituus. Vastaanotto vaatii, että pituus vastaa luokan pituutta `class_len[]` |
| `checksum` | 16-bittinen summa datasta (`chksum`, `Tp/TpComY.cpp:613`) |
| `d` | Luokan mukainen unioni |

Luokkien pituudet asetetaan funktiossa `init_class_len` (HK `Hk/HkCom32.cpp:1082`, viesti
`Juk/VCom.cpp:1211`). Pituudet riippuvat käännösparametreista (`VALUKU`, `MAXOSUUSLUKU`,
`kilprecsize0`, `osrecsize0`), joten eri tavalla käännetyt ohjelmat eivät ymmärrä toistensa
paketteja. Paketissa ei ole versiotunnistetta (havainto SAN-7).

### 2.2 Kuittaus ja toisto (linkkikerros `Tp/TpComY.cpp`)

| Vaihe | Funktio | Kuvaus |
|---|---|---|
| Jonoon | `buflah` (`:476`) | Kirjoittaa paketin yhteyskohtaiseen lähtevään jonoon: muistiin (`outbuf`, `OUTBUFL` = 500) tai levypuskuriin (`COMFILE`, `levypusk`). Kutsutaan `outb_CriticalSection`-lukossa |
| Lähetys | `lahetapaketti` (`:1407`) | Lisää kehyksen (`SOH`, `id`, `iid`, `checksum`) ja lähettää. UDP:ssä odottaa kuittausta ja toistaa enintään 3 × toistokerroin |
| Kuittaus | `kirjaahyv` (`:1575`) | `ACK id iid id` siirtää jonon alkua; tiedostopaketeissa `kirjaa_tiedhyv` |
| Vastaanotto | `luepakettiUDP` (`:925`), `tiedonsiirtoTCP` (`:1257`), `tiedonsiirto` (RS) | Tarkistaa kehyksen, pituuden, numeroinnin ja summan, kuittaa (`ACK`) tai hylkää (`NAK`). Toistettu paketti kuitataan uudelleen mutta ei käsitellä toiseen kertaan |
| Tulojono | `addseur` (`:629`) | Kopioi paketin tulojonoon `inbuf` (`INBUFL` = 50). Avaussanoma tarkistetaan ensin (`tark_alku`) |
| Käsittely | `tarkcom` (HK `Hk/HkCom32.cpp:875`, viesti `Juk/VCom.cpp:893`) | Yksi säie, joka purkaa tulojonon ja kutsuu luokan käsittelijää (`tark_…`) |

`UDPvarmistamaton` ohittaa kuittauksen (bitti 1 lähetyksessä, bitti 2 vastaanotossa).

### 2.3 Välitys ja silmukoiden esto

Jokainen lähetysfunktio käy läpi kaikki yhteydet (`cn < maxyhteys`) ja ohittaa yhteyden, jos:

| Ehto | Merkitys |
|---|---|
| `!comopen[cn] \|\| !lahcom[cn]` | Yhteys kiinni tai pelkkä vastaanottoyhteys |
| `cn == kielto - 1` | Sanoma tuli tältä yhteydeltä. Vastaanotetun sanoman tallennus kutsutaan arvolla `kielto = cn + 1`, joten sanomaa ei palauteta lähettäjälle |
| `kielto && estavalitys[cn]` | Välitys muiden koneiden sanomille estetty (viestissä bittimaskina, `tarkkielto`, `Juk/VCom.cpp:239`) |
| `kielto < 0 && cn != -kielto - 1` | Kohdennettu lähetys yhdelle yhteydelle (uusinnat) |
| `lahec[cn] == 2`, `lahkellovain[cn]` | Yhteys vain emit-koodeille tai vain kellolle |
| `com_ajat[cn]` | 0 = aikajonon ajat eivät kulje, 2 = vain aikajonon ajat (muut sanomat ohitetaan) |
| `com_emit[cn]`, `com_emitva[cn]` | Kortinluennat ja emit-väliaikasanomat vain, jos sallittu. Vastaanotossa `tark_emit` hyväksyy vain arvon 1 |
| XML-yhteys (`comtpTCPXML`, `comtpTCPSRVXML`) | Vain tulos- ja väliaikasanomat (luku 6). HK ohittaa hyväksymismuutokset (`hyv_muutos`) |

Verkko voi siis olla ketju tai tähti. Jokainen kone välittää eteenpäin sanomat, jotka se on
tallentanut, paitsi takaisin lähettäjälle.

### 2.4 Avaussanoma (ALKUT, luokka 0)

| | |
|---|---|
| Lähettäjä | `lahetapaketti`, kun `lah_alku[cn]` on asetettu: yhteyden avaus tai uusinta. Sisältö `alkusanoma` (HK `:811`, viesti `:842`) |
| Sisältö | `tunn` = 1, `konetunn` (2 merkkiä), `vaihe` (`k_pv+1`, viestissä 0), `nrec` (KILP.DAT-tietueiden määrä), `flags` |
| Vastaanotto | `tark_alku` (HK `:821`, viesti `:851`) |
| Rooli | **Yhteensopivuuden tarkistus**: varoittaa, jos koneen KILP.DAT on eripituinen tai kone on eri vaiheessa. Eri vaiheessa oleva kone keskeyttää vastaanoton (`com_keskeytt`). **Vaiheen vaihtopyyntö**: kun `flags & 1`, vastaanottaja vaihtaa vaiheen (`pyyda_vaihdavaihe` → `WM_MYVAIHDAVAIHE`). Lisäksi naapurikoneen tunnus tallentuu (`naapuri[cn]`) |

## 3. Henkilökilpailun (HkKisaWin) sanomaluokat

Määrittelyt: `Hk/HkCom32.cpp:47–58`.

| Luokka | Nimi | Lähetys | Vastaanotto | Sisältö | Rooli ja käyttötarkoitus |
|---|---|---|---|---|---|
| 0 | `ALKUT` | `alkusanoma` | `tark_alku` | ks. 2.4 | Yhteyden avaus, vaiheen vaihto |
| 1 | `KILPT` | `laheta(piste=0)` (`:167`) | `tark_kilp(…,1)` (`:242`) | Koko perustietue pakattuna (`pack0`), `dk` = tietueen paikka, `entno` = vanha kilpailijanumero (0 = uusi) | **Kilpailijan lisäys, poisto ja perustietojen muutos** (nimi, seura, sarja, numero, lisenssit). Vastaanotto: `addtall`, `deltall` tai `tallenna` |
| 2 | `KILPPVT` | `laheta(piste<0)` | `tark_kilp(…,2)` | Yhden vaiheen tiedot (`kilppvtp`). Aktiivinen vaihe ilman aikoja (`valuku` = 0), muut vaiheet aikoineen | **Vaihekohtaiset tiedot**: lähtöaika (arvottu), emit-koodit, erä, rata, vaiheen sarja |
| 3 | `VAIN_TULOST` | `laheta(piste=1…valuku+5)` | `tark_kilp(…,0)` | `bib`, `k_pv`, `vali`, `aika`. `vali`: −1 = lähtö, 0 = maali, >0 = väliaikapiste, `valuku+1` = tarkastusmerkintä (`tark`), `+2` = ammunnan sakot (`asakot`), `+3` = tasoitus | **Yksittäisen ajan tai tilan muutos.** Tämä on tärkein tulossanoma: maaliaika, väliaika, lähtöleima, hylkäys/keskeytys. Vastaanotto: `tall_tulos`, `set_tark` ja `tallenna` |
| 4 | `AIKAT` | `laheta_aika` (`:356`) | `tark_aika` (`:401`) | Kaksi `aikatp`-tietuetta: poistettava (`daika`) ja lisättävä (`iaika`) aikajonon rivi | **Ajanottojonon (maaliaikajono) replikointi**: rekisteröidyt ajat ennen ja jälkeen kilpailijaan kohdistamisen. Vastaanotto: `tall_rivi`. Ennen ajastimen käynnistystä enintään 30 sanomaa puskuroidaan |
| 6 | `EMITT` | `laheta_em` (`:766`) | `tark_emit` → `addem` (`:790`) | `emittp`: kortin numero, lukuaika, leimat, `package` (lähteen tunniste) | **Kortinluennan (emit, emiTag, SI) jakelu**: koneet tarkastavat leimat ja näyttävät luennan |
| 7 | `SEURAT` | `lah_seura` (`:461`, `SEURAVAL`) | `tark_seura` → `tall_seura` | Seuran nimi, lyhenne, maa, piiri | **Seurarekisterin päivitys** |
| 10 | `FILESEND` | `lah_tiedosto` (`Tp/TpComY.cpp:2898`) | `tark_tiedosto` (`:3000`) | Tiedoston nimi (`pos` = −1), lohkot (`pos` ≥ 0), loppu (`pos` = −2) | **Tiedoston jakelu** koneelta toiselle. Vain UDP-yhteydet, joilla `lah_tiedostot` sallii lajin (`flags`). Vastaanottaja välittää tiedoston eteenpäin |
| 13 | `EMITVA` | `laheta_emva` (`:746`), `laheta_emva_pois` (`:756`) | `tark_emitva` → `kirjemvaloki` | `emitvatp`: kilpailija, tulos, tarkastus, rastiväliajat | **Emit-väliaikojen julkaisu** ulkoiselle vastaanottajalle (XML-yhteys) ja lokiin. Ei muuta tulostietoja |
| 14 | `EXTRA` | ks. 3.1 | `tark_extra` (`:619`) | `d1` = alakoodi, `d2…d4` | Ohjaus- ja pienet tietosanomat |

### 3.1 EXTRA-alakoodit (HK)

| `d1` | Lähetys | Vastaanotto | Rooli |
|---|---|---|---|
| 1 | `laheta_ec` (`VALUKIJA`) | `tall_ec(badge, piste, aika)` | **Emit-koodin leima kohdistamatta**: väliaikalukija on lukenut kortin, ja vastaanottaja kohdistaa leiman kilpailijaan |
| 2, 3 | `lahetaaktivointi`, `lahetakierros` (`MAKI`) | `MAKIx`-ehdon takana: ei käännetä | Mäkihypyn aktivointi ja kierros (havainto SAN-4) |
| 4 | `lahetakello` (`:542`) | Ei käsittelijää `tark_extra`ssa | Kellonaika vastaanottajalle (havainto SAN-2) |
| 5 | `lahetaeralahto` (`:558`), myös `tallenna` (MERI5-lajit `NPU`) | Asettaa `eralahto[]` ja `era_akt`, välittää eteenpäin | **Erälähdön ajan jakelu** |
| 6 | `lahetaKurvinen` (`:579`) | `tulkKurvinen` → `tall_ampsakko` ja välitys | **Ammunnan tulos** (Kurvinen-taulu) koneelta toiselle |
| 7 | `lahetatestikaynnistys` | `kaynnistatesti` → testisyötön käynnistys | Kuormitustestin käynnistys kaikilla koneilla (luku 5) |
| 9 | `lahetasulku(tn)` | Sulkee ohjelman, jos `tn` on tyhjä tai tämän koneen tunnus. Muuten välittää | **Etäsulku** |

## 4. Viestin (ViestiWin) sanomaluokat

Määrittelyt: `Juk/VCom.cpp:62–75`. Tallennus vertaa vanhaa ja uutta tietuetta (`Juk/vdat.cpp:760–810`)
ja lähettää vain muuttuneet osat. `luokka`-parametri kertoo, miksi aikaa lähetetään (1 = ei
aikajonokoneille, 2 = aina; 4 + 16 × maski = osuuden muut tiedot).

| Luokka | Nimi | Lähetys | Vastaanotto | Sisältö | Rooli ja käyttötarkoitus |
|---|---|---|---|---|---|
| 0 | `ALKUT` | `alkusanoma` | `tark_alku` | ks. 2.4 (vaihe = 0) | Yhteyden avaus |
| 1 | `KILPT` | `laheta(os=0)` (`:351`) | `tark_kilp` (`:522`) | Joukkueen perustiedot (`kilprecsize0`) | **Joukkueen lisäys, poisto ja perustietojen muutos** |
| 2 | `OSUUST` | `laheta(os>0, luokka&4)` | `tark_osuus` (`:606`) | Osuustietue (`osrecsize0`) ja muutosmaski `muutos` | **Osuuden tietojen muutos**. Maskin bitit: 0x1 nimi, 0x2 `lisno`, 0x4 seura, 0x8 arvo, 0x10 hajonta, 0x20/0x40 kortit, 0x80/0x100 laina, 0x200 selitys, 0x400 uusi, 0x800 seuranta, 0x4000 `ylahto`, 0x8000 lähtölaji, 0x10000 `keskhyl` (tila), 0x20000 ammuntasakot, 0x40000 sakko. Vain merkityt kentät päivitetään |
| 3 | `VAIN_TULOST` | `laheta(os>0, piste)` | `tark_tls` (`:722`) | `osuus`, `piste` (0 = vaihto/maali, >0 = väliaika), `vatp` | **Osuuden ajan muutos** (vaihto, maali, väliaika, lähtö). Vastaanotto: `setMaali` ja `tallenna` |
| 4 | `AIKAT` | `laheta_aika` (`:400`) | `tark_aika` (`:427`) | Kuten HK | **Ajanottojonon replikointi** |
| 5 | `TARKT` | Ei lähettäjää nykykoodissa | `tark_trk` (`:680`) | Tila, sakot, ammuntasakot | Vanha tarkastussanoma. Tila kulkee nykyään `OSUUST`-sanomassa (havainto SAN-3) |
| 6 | `EMITT` | `laheta_em` (`:792`) | `tark_emit` → `addem` | `emittp` | **Kortinluennan jakelu** |
| 8 | `VEMITT` | `lahkvaihto` (`:761`) | `tark_vemit` → `tallkvaihto` | `ekno`, `eos`, `kno`, `os` | **Kortin siirto toiselle joukkueelle tai osuudelle** (kortinvaihto) |
| 9 | `YHTEISLAHTOT` | `laheta_yl` (`:252`) | `tark_ylahto` (`:299`) | `sarja`, `ylahto[os][0..1]` (aika ja kynnys), `fl` (`ylmyoh`); negatiivinen sarja = sarjan lähtöaika | **Sarjan ja osuuksien yhteislähtöjen jakelu**. Vastaanottaja kirjoittaa tapahtuma-XML:n (`kirjEventXml`) ja välittää eteenpäin |
| 10 | `FILESEND` | `lah_tiedosto` | `tark_tiedosto` | Kuten HK | Tiedoston jakelu |
| 13 | `EMITVA` | `laheta_em`, `laheta_emva` (`:1283`) | `tark_emitva` → `kirjemvaloki` | `emitvatp` | Emit-väliaikojen julkaisu |
| 14 | `EXTRA` | ks. 4.1 | `tark_extra` (`:1603`) | `d1…d5` | Ohjaus |

### 4.1 EXTRA-alakoodit (viesti)

| `d1` | Lähetys | Vastaanotto | Rooli |
|---|---|---|---|
| 1 | `laheta_ec` (`VALUKIJA`) | `tall_ec` | Emit-koodin leima kohdistamatta |
| 3 | `lahetaKnoSiirto(sarja, siirto)` | `tallKnoSiirto` (`Juk/vdat.cpp:966`) | **Sarjan joukkuenumeroiden siirto** |
| 4 | `lahetakello` | Ei käsittelijää | Kellonaika (SAN-2) |
| 7 | `lahetatestikaynnistys` | `kaynnistatesti` | Testisyötön käynnistys |
| 9 | `lahetasulku` | Etäsulku tai välitys | Etäsulku |

### 4.2 Tapahtumat

Viestin `tallenna` lisää tallennuksen yhteydessä tapahtuman (`addtapahtuma`), samoin HK:n `tallenna`
(`Hk/HkDat.cpp:1245`). Tapahtumat muodostetaan vastaanottavalla koneella sanoman tallennuksesta, eikä
niitä siirretä omana sanomanaan. Kuuluttajan "viimeiset tapahtumat" -näkymä ja REST-rajapinnan
tapahtumavirta (V-WS-6) nojaavat tähän.

## 5. Sanomien uusinta ja testisyöttö

| Toiminto | Funktio | Rooli |
|---|---|---|
| Uusinta aikavälillä | `merk_uusinta` (`Tp/TpComY.cpp:1111`) | Merkitsee levypuskurin paketit lähettämättömiksi aikarajoista alkaen. Uusi TCP-yhteys (`tcpkaynnistys`) uusii automaattisesti noin 4 minuuttia |
| Valittujen sarjojen uusinta | `uusintaTCPvalitut` (HK `:1130`, viesti `:1303`) | Lähettää valittujen sarjojen tulokset, väliajat tai lähtöajat uudelleen yhdelle yhteydelle (`kielto = -cn-1`). Käy läpi `jalku`/`JarjSeur`-listat `tall_CriticalSection`-lukossa |
| Numeroalueen uusinta | `lahetanumerotTCP` (`SERVER`) | Lähettää levypuskurin pakettinumerot uudelleen |
| Emit-väliaikojen uusinta | `emitva_uusinta`, `emitva_uusi_sarja` | Lähettää `EMITVA`-sanomat uudelleen |
| Viestin kaikki tulokset | `lahetakaikkiTCP` (`Juk/VCom.cpp:1006`) | Kaksoispisteillä eroteltu tekstirivi joukkueittain ja osuuksittain TCP-yhteydelle. Laskee ensin sijat (`merk_sijat`) |
| Testisyöttö | `ReadComTestParams` (`ComTestParam.txt`), `SendCopyThread`, `lahetaKopiot` | Toistaa tallennetun COMFILE-jonon sanomat alkuperäisessä tahdissa tai tasavälein. Tarkoitettu kuormitus- ja regressiotestaukseen. `EXTRA` 4 ja 9 suodatetaan pois |
| Jonon tarkastelu | `UnitComfile` (HK ja viesti) | Näyttää COMFILE-jonon paketit luokittain ja kilpailijoittain |

## 6. XML-tulossanomat TCP-yhteydellä (B)

| | |
|---|---|
| Yhteystyypit | `comtpTCPXML` (asiakas), `comtpTCPSRVXML` (palvelin), `Tp/TpComY.cpp` `lahetapaketti` → `lahetaXML_TCP` |
| Lähde | Sama lähtevä jono kuin luvussa 2, mutta paketti muunnetaan XML:ksi ennen lähetystä. Kuittausta ei ole: jonon alku siirtyy, kun koko sanoma on kirjoitettu |
| HK (`Hk/HkCom32.cpp:972`) | `KILPT` ja `VAIN_TULOST` (`vali ≥ -1`) → kilpailijan XML (`kirjXmlOo` väliaikatiedoston kautta, tai vanha muoto `xmlsanoma` STX…ETX-kehyksessä, `vanhaXMLsan`). `EMITVA` → `xmlemitvasanoma` |
| Viesti (`Juk/VCom.cpp:1055`) | `KILPT`, `VAIN_TULOST`, `TARKT`, `OSUUST` → joukkueen tai osuuden XML |
| Rooli | **Reaaliaikainen syöte ulkoiselle tulospalvelulle** (online-tulokset, TV-grafiikka). Tämä on lähin olemassa oleva vastine REST-rajapinnan tapahtumavirralle, mutta se on yksisuuntainen eikä tue kyselyjä |

## 7. Tiedostojen siirto (C)

| Sanoma | Kuvaus | Rooli |
|---|---|---|
| `FILESEND` (luokka 10) | `lah_tiedosto` jakaa tiedoston lohkoina väliaikatiedoston `__FCOMFILE__.TMP` kautta. Absoluuttiset polut sallitaan vain, jos `lahkaikkitied` on päällä | Asetus- ja ratatiedostojen jakelu koneille |
| Automaattitulosteen TCP-siirto | `sendtcpfile` (`Hk/HkTulostus.cpp:7031`, `Juk/VTulostus.cpp`): STX + tiedoston sisältö + ETX yhteydelle `fileyht` | Automaattisesti muodostetun tulostiedoston (HTML/XML) toimitus vastaanottajalle |

## 8. Laitteiden ja ulkoisten palveluiden saapuvat sanomat (D ja E)

### 8.1 Laitteet (`regnly`, `Tp/TpLaitteet.cpp`)

Laitteet luetaan säikeessä `lue_regnlyThread` (`:1747`) ja tulkitaan funktiossa `lue_regnly`
(`:1515`). Laitteen tyyppi on `regnly[r_no]` (`include/TpLaitteet.h:17–47`).

| Ryhmä | Laitteet (`LID_…`) | Sanoma | Tallennus | Siirtyy verkkoon |
|---|---|---|---|---|
| Ajanottolaitteet (valokenno, maalikello) | `ALGE`, `COMET`, `ALGE4`, `TIMY`, `ALGETUTKA`, `TDC`, `ASC`, `ASC3`, `SW2000`, `TUOMARI` | Laitekohtainen tekstirivi: kanava, aika | `tall_rivi` (aikajono) | `AIKAT` |
| Rajapinnan pulssi | Sarjaportin CTS (`comajanotto`, `:2405`) | Signaalin nousu | `tee_aika` (aikajono) | `AIKAT` |
| Emit-kellot ja -lukijat | `RTR`, `RTR2`, `EMITKELLO` (`lue_EmitKello`), `LUKIJA`, `MTR`, `EMITAG` | Emit-protokollan kehykset (kortti, leimat, aika) | `tall_emit`/`addem` (kortinluenta), `tall_ec` (koodi), `tall_etulos` (radioleima) | `EMITT`, `EMITVA`, `EXTRA`/1, `VAIN_TULOST` |
| SportIdent | `SPORTIDENT` (`lue_SI`, `tulkSI`) | SI-asemaprotokolla (STX…ETX, CRC). Ohjelma lähettää kyselyt ja kuittaukset | `tall_emit` | `EMITT` |
| RFID | `SIRIT`, `FEIG`, `IMPINJ`, `ZEBRA` (LLRP, `Tp/ZebraReader.cpp`), `ARES`, `RADIOEMIT` | Tunnistetapahtuma: tagi, aika, antenni | `tall_rivi` (aikajono, kohdistettuna) | `AIKAT` (ja `VAIN_TULOST`, kun kohdistettu) |
| Ammunta | `KURVINEN` | Ammuntapaikan tulosrivi | `tall_ampsakko` | `EXTRA`/6 |
| Tietopalvelut | `ETGPRS`, `ETSQL`, `SIIMPORT` | ks. 8.2 | | |
| Lähtevät näytöt | `VALOT`, `CPOWER`, `RTNM` | ks. luku 9 | | |

Laiteasetukset (`aseta_emitag`, `aseta_mtr`, `aseta_s4`, `SiritSync`) lähettävät laitteelle
konfigurointikomentoja, kuten aseman koodin ja kellonajan. Ne eivät muuta kilpailun tietoja.

### 8.2 Ulkoiset palvelut

| Palvelu | Toteutus | Pyyntö | Tallennus | Rooli |
|---|---|---|---|---|
| emiTag GPRS -palvelin (`EMITHTTP`) | `etHaku`/`vahaku` (`Hk/HkIV.cpp:3500`, `Juk/VIv.cpp`), `httphaku` (`com/gethttp.cpp:125`) | `GET /emitphp/get_stream.php?Y=<sarja>&time=<pvm aika>&Mmin=<n>` | `tulk_emiTag` → `tall_etulos` | **Radioleimat** (väliaikapisteet) palvelimelta. Haku omassa säikeessään hakuvälin mukaan |
| emitSQL (`cbTp/UnitEmitSQL.cpp`) | MySQL-kysely `tags`-tauluun | `SELECT Y, E, N, C, M, time …` | Kuten emiTag | Radioleimat tietokannasta |
| SI-tiedostotuonti (`SIIMPORT`) | `luepakettiTCPSI`, `si_tcpimport` (`Tp/TpComY.cpp:1958–2076`) | Tekstikomennot `gaterepeat`, `gatelisten true/false`, `exit`; vastaus `dt\t…`-rivit | `tall_regnly` | Ajat SI-portilta. **Ei käännetä GUI-ohjelmiin** (`SIIMPORT` ei ole määritelty, havainto SAN-6) |

## 9. Muut lähtevät ilmoitukset (F)

| Kohde | Toteutus | Muoto | Rooli |
|---|---|---|---|
| UDP-yleislähetys | `broadcasttulos` (`Hk/HkCom32.cpp:143`), kun `broadcasts` | `KT:nnnnn:kkkk:v:p:aika\r\n` (koneen tunnus, juokseva numero, kilpailija, vaihe, piste, aika) | Kevyt tulosilmoitus kuuntelijoille (esim. näyttöohjelmat). Lähetetään jokaisesta HK:n aikasanomasta (`laheta`, piste 1…valuku+2) |
| Monitori | `lahetaMonitorille` (`Tp/TpComY.cpp:442`) | `Kone:KT;YhtLkm:n;AvLkm:n;Jonossa:n;JonoLkm:n` UDP:llä (oletuksena broadcast, portti `monportbase + i`) | **Tiedonsiirron valvonta**: yhteyksien ja jonojen tila keskitettyyn valvontaan |
| TCP-loki | `comtpTCPLOKI`, `openTCPloki`, `wrt_st_TCP(hLoki, …)` (`Hk/HkApu.cpp:576`, `Juk/vApu.cpp:47`) | Lokirivit | Lokin etäkirjaus |
| GAZ- ja ALGE-taulut | `gazTh`, `wrt_st_x(cn_gaz, …)` (`Hk/HkIV.cpp:3351`) | Taulun rivit | Maalin tulostaulu |
| Valot, C-Power, RTNM | `naytatulos`, `naytaluenta` (`Tp/TpLaitteet.cpp:503–575`) | Laitekohtainen | Tulos tai luennan kuittaus näyttöön tai valoihin |
| MySQL | `cbHk/UnitMySQL.cpp`, `ViestiWin/UnitMySQL.cpp` (`kirjSQLTh`) | `INSERT`/`UPDATE` tauluihin `osanotot`, `osottopv` ja `tulos` | **Tietojen vienti tietokantaan** (esim. online-palvelu) |
| Automaattitiedostot | `autofile`, `kirjEventXml` | HTML ja XML levylle | Tulossivut ja tapahtuma-XML (luku 7 siirtää niitä tarvittaessa) |

## 10. Sisäiset ikkunaviestit (G)

Taustasäikeet ilmoittavat käyttöliittymälle Windows-viesteillä (`cbHk/HkWinDef.h:102–118`,
`ViestiWin/VWinDef.h:89–103`). Kaikki lähetetään funktiolla **`SendMessage`**, joka on synkroninen:
lähettävä säie odottaa, kunnes käyttöliittymäsäie on käsitellyt viestin.

| Viesti | Lähettäjä | Rooli |
|---|---|---|
| `WM_MYMSGDSPMSG`, `WM_MYMSGDSPERRORMSG`, `WM_MYMSGDSPWARNMSG` | `writeerror_w` ym. (`cbTp/cbTpApu.cpp:377–487`) | Ilmoitus- ja virheikkunat taustasäikeistä |
| `WM_MYUUSIAIKA` | `paivita_aikanaytto` (`cbHk/WinHk.cpp:1876`), jota kutsutaan mm. `tallenna`n lopussa (`Hk/HkDat.cpp:1261`, `Juk/vdat.cpp:819`) | Aikanäytön päivitys |
| `WM_MYVAIHDAVAIHE` | `pyyda_vaihdavaihe` (`cbHk/WinHk.cpp:2231`), avaussanoma `flags & 1` | Vaiheen vaihto verkon pyynnöstä |
| `WM_MYMSGCLOSEAPPL` | `tark_extra` 9 | Etäsulku |
| `WM_MYMSGNEWCARD`, `WM_MYDISPLTIMER`, `WM_MYECODE` | Emit-käsittely | Uusi kortinluenta näkymään |
| `WM_MYLUKUMAARAT`, `WM_MYPAIVITA`, `WM_MYSCROLL`, `WM_MYADDSAKKO`, `WM_MYHAEAVOIMET`, `WM_MYTIEDOT`, `WM_MYSELECTOPT` | Laskenta- ja näyttösäikeet | Näkymien päivitys |

**Merkitys rajapinnalle:** `tallenna` kutsuu `paivita_aikanaytto`a, ja kutsu tehdään usein
`tall_CriticalSection`-lukon sisällä. Säie, joka tallentaa, odottaa siis käyttöliittymäsäiettä.
Jos käyttöliittymäsäie samaan aikaan odottaa samaa lukkoa, ohjelma lukkiutuu. Ks. 11.3 ja havainto SAN-8.

## 11. Kytkentä REST-rajapintaan

REST-palvelin toimii omalla koneellaan (V-KONF, K2). Se saa tietonsa tavallisena verkkokoneena
luvun 2 tiedonsiirrolla, ja sen syötöt kulkevat samaa reittiä muille koneille.

### 11.1 Mistä sanomista rajapinnan tiedot syntyvät

| REST-toiminto | Vaatimus | Lähdesanomat | Edellytys REST-koneen yhteydelle | Huomio |
|---|---|---|---|---|
| Kilpailijat ja joukkueet, perustiedot | V-KLP | HK `KILPT`, `KILPPVT`; viesti `KILPT`, `OSUUST` | Normaali vastaanottoyhteys | Viestin osuustiedot päivittyvät maskin mukaan. `lisno` kulkee, IOF-tunniste ei (ks. `doc/viesti-iof-tunniste-muutosehdotus.md`) |
| Lähtöajat, yhteislähdöt | V-SRJ, V-TUR | HK `KILPPVT`, `VAIN_TULOST` (`vali` = −1), `EXTRA`/5; viesti `YHTEISLAHTOT`, `OSUUST` (0x4000) | Normaali | Sarjan perusasetukset (lähtöväli, luonne) eivät kulje sanomina: ne luetaan koneen omista tiedostoista. REST-koneella on oltava samat kilpailuasetukset (`FILESEND` tai kopio) |
| Maali- ja väliajat, tila | V-SRJ, V-PST, V-STA | HK `VAIN_TULOST`; viesti `VAIN_TULOST`, `OSUUST` (0x10000) | Normaali | Sijat lasketaan vastaanottavalla koneella (`sijat2`/`merk_sijat`), eikä niitä siirretä |
| Väliaikapisteen leimatapahtuma (WebSocket) | V-WS-6, V-WS-6c | `VAIN_TULOST` väliaikapisteelle, joka on tallentunut REST-koneelle (`tallenna` → `addtapahtuma`) | Normaali | Sanomassa ei ole tietoa ajan lähteestä (radio, kortinluenta, käsisyöttö), havainto SAN-9 / SI-4, K22 |
| Kortinluennat, luettujen määrä | V-STA-2, V-TUR-4 | `EMITT` | **`com_emit[cn] = 1` REST-koneen vastaanottoyhteydellä.** Muuten `tark_emit` hylkää sanoman | Lisättävä konfigurointiohjeeseen |
| Maaliaikajono (kohdistamattomat ajat) | V-TUR (maalissa olevat) | `AIKAT` | `com_ajat[cn] ≥ 1` | Ilman tätä rajapinta ei näe kohdistamattomia maaliaikoja |
| Emit-väliajat | (ei vaatimusta) | `EMITVA` | `com_emitva` | Käsitellään vain lokiin, ei tarvita |
| Kilpailun vaihe | V-STA | Avaussanoma (`vaihe`, `flags & 1`) | Normaali | Vaiheen vaihto verkosta vaihtaa REST-koneen vaiheen. Rajapinnan on mitätöitävä välimuisti ja suljettava WebSocket-tilaukset tai ilmoitettava muutoksesta (V-VM) |
| Tiedonsiirron tila | V-STA-4 | Linkkikerroksen tila (`yhtfl`, `lahettamatta`), monitorisanoman sisältö | — | Samat luvut kuin `lahetaMonitorille`-sanomassa |

### 11.2 Mihin rajapinnan syötöt menevät

| REST-syöttö | Vaatimus | Toteutus | Syntyvät sanomat |
|---|---|---|---|
| DNS (ei lähtenyt) | V-SYO-1…2 | lukko → `GETREC` → `set_tark`/`SetTark` → `tallenna(kielto=0)` (V-ARK-6) | HK `VAIN_TULOST` (`vali = valuku+1`); viesti `OSUUST` (0x10000). XML-yhteyksille HK ei lähetä hyväksymismuutosta |
| Uusi lähtöaika (väliaikalähtö) | V-SYO-3…4 | lukko → `GETREC` → lähtöajan asetus → `tallenna` | HK `KILPPVT` ja/tai `VAIN_TULOST` (`vali` = −1) |

Syötöt lähetetään kaikille yhteyksille (`kielto = 0`), ja vastaanottajat välittävät ne eteenpäin
(2.3). Koska REST-kone on tavallinen verkkokone, syötön vaikutus näkyy muilla koneilla samalla
viiveellä kuin käyttöliittymästä tehty muutos. `pakota`-kenttää ei käytetä (oletus 0).

### 11.3 Vaatimuksiin vietävät tarkennukset

1. **V-KONF / käyttöönotto-ohje:** REST-koneen vastaanottoyhteydellä on oltava `com_emit = 1`
   (kortinluennat) ja `com_ajat ≥ 1` (aikajono). Lähetyssuunta on tarpeen vain, jos syötöt (vaihe 5)
   ovat käytössä.
2. **V-ARK-6:** Syöttöjen tallennusta ei saa tehdä säikeessä, joka pitää muita lukkoja kuin
   `tall_CriticalSection`, eikä REST-säie saa odottaa käyttöliittymäsäiettä lukon sisällä
   (`paivita_aikanaytto` → `SendMessage`). Ks. SAN-8.
3. **V-VM (välimuisti):** Versionumero kasvaa jokaisesta `tarkcom`in käsittelemästä sanomasta, joka
   johtaa `tallenna`/`addtall`/`deltall`-kutsuun, sekä `YHTEISLAHTOT`-, `EXTRA`/5- ja `SEURAT`-sanomista.
   Vaiheen vaihto mitätöi koko välimuistin.
4. **V-WS-6:** Leimatapahtuma syntyy REST-koneen `tallenna`sta riippumatta siitä, tuliko aika
   verkosta, laitteelta vai SI Centeristä. Tämä on V-WS-6:n nykyinen määrittely.
5. **V-SEC:** Verkon tiedonsiirtoportti ei ole todennettu. Kuka tahansa samassa verkossa voi lähettää
   `EXTRA`/9-etäsulun tai muuttaa tietoja. REST-rajapinnan todennus ei suojaa tätä reittiä, joten
   suljettu verkko on edellytys (K7).

## 12. SportIdent-haaran muutokset sanomatasolla

### 12.1 Yhteenveto

**Haara ei lisää uusia sanomaluokkia eikä muuta `combufrec`-rakennetta.** Uudet tietolähteet
kytkeytyvät olemassa oleviin tallennusreitteihin, ja tieto leviää verkkoon nykyisillä sanomilla.

| Uusi sanoma (haara) | Suunta | Tulkinta | Tallennus | Verkkoon | Huomio |
|---|---|---|---|---|---|
| SI-aseman kortinluenta (useita korttityyppejä, EXT-protokolla, BSM8-asemat) | sisään; ohjelma lähettää lohkopyynnöt (`SI9pyynto_b0/b1` ym.) ja kuittaukset | `lue_SI`, `tulkSI`, `Tp/SITulkinta.cpp` | `tall_emit`, `em.package = EMITPKG_SPORTIDENT + r_no` | `EMITT` | Vastaanottaja tunnistaa SI-kortin `package`-kentästä (`ON_SPORTIDENT_EM`). Vanha versio ei tunne arvoa (SAN-10) |
| SRR-/D3-radioleima (online-rasti) | sisään | `lue_SID3`, `tallSRRleima`, `Tp/SID3Punch.cpp` | `tall_etulos` → `tallenna` | `VAIN_TULOST` (HK), `VAIN_TULOST`/`OSUUST` (viesti) | Kuten emiTag-radioleimat |
| SI Center REST API | pyyntö ulos (HTTPS 443), vastaus JSON | `siVahaku` (säie), `SICenterJson`, `siParsePunch` | Maali/väliaika: `set_tulos`/`setMaali` ja `tallenna`; lähtö: `tall_lajat_pv`/`setMaali(-1)`; kohdistamaton: `tall_rivi` | `VAIN_TULOST`, `KILPPVT`, `AIKAT` | `GET /api/rest/v1/punches?modem=…&after=…` tai `&afterId=…`. Tietue luetaan lukon ulkopuolella (SI-1) |
| `httphaku` (HTTPS, katkaisutieto) | — | `com/gethttp.cpp` | — | — | `secure`-lippu on nyt käytössä. Katkaisutieto `katkaistu` (oletus NULL) jää emiTag-kutsuissa käyttämättä (SI-8) |
| `VCom.cpp`: `AIKAT`, `EMITT`, `EMITVA`, `VEMITT` | molemmat | — | — | — | Ehdot `EMITLEIMAT` ja `!LUENTA` poistettu. ViestiWinissä `EMITLEIMAT` on jo määritelty (`MAALI && EMIT`), joten **GUI:n käyttäytyminen ei muutu**. Konsolin luentaohjelma (`LUENTA`) alkaa lähettää ja vastaanottaa näitä luokkia (konsoli ei kuulu laajuuteen, V-ARK-7) |

### 12.2 Varmennetut vaikutukset

| Kohta | Tulos |
|---|---|
| Sanomien muoto ja pituudet | Ennallaan. `pragma pack` -lukitus (ad1e6ae) varmistaa, että `combufrec` ja `emittp` pysyvät samankokoisina eri kääntäjillä (SI-9) |
| Uudet tallennusreitit | Kaikki kulkevat `tallenna`n, `tall_emit`in tai `tall_rivi`n kautta, joten tiedot replikoituvat ja tapahtumat syntyvät normaalisti |
| Ajan lähde | Ei kulje sanomassa. Vastaanottaja ei erota SI Center- ja SRR-leimoja muista ajoista (SI-4, SAN-9) |
| Useampi hakija | Jos usea kone hakee SI Centeristä, jokainen tallentaa ja lähettää samat leimat. Sanomia syntyy moninkertaisesti (SI-3, V-WS-6c) |
| Lähtöleima | Korvaa lähtöajan aina, ja muutos leviää `KILPPVT`/`VAIN_TULOST`-sanomana (SI-2, K23) |

## 13. Havainnot

Vakavuus kuten `doc/sportident-review-items.md`: **K** korkea, **M** keskitaso, **A** matala, **I** tieto.

| # | Vak. | Havainto | Missä | Suositus |
|---|---|---|---|---|
| SAN-1 | K | HK:n `tark_kilp` lukee tietueen (`kilp.GETREC(d)`) **lukon ulkopuolella** ja tallentaa muokatun kopion lukon sisällä. Jos käyttöliittymä, laitesäie tai SI Center tallentaa saman kilpailijan välissä, sen muutos häviää. Viestin `tark_osuus`, `tark_tls` ja `tark_trk` lukitsevat ennen lukua. Sama malli kuin SI-1 | `Hk/HkCom32.cpp:259` | Siirrä lukko ennen `GETREC`-kutsua samassa korjauksessa kuin SI-1 ja `tall_etulos` |
| SAN-2 | M | `EXTRA`/4 (kello) lähetetään (`lahetakello`, Tila-näkymä), mutta `tark_extra`ssa ei ole sille käsittelijää. Kellon synkronointi sanomalla ei toimi, tai se on toteutettu muualla | `Hk/HkCom32.cpp:619`, `Juk/VCom.cpp:1603` | Selvitä tarkoitus. Toteuta tai poista painike |
| SAN-3 | A | Viestin `TARKT` (5): vastaanotto ja XML-muunnos on, lähettäjää ei. Tila kulkee nykyään `OSUUST`-sanomassa | `Juk/VCom.cpp:680` | Säilytä yhteensopivuuden vuoksi. Merkitse vanhentuneeksi |
| SAN-4 | A | HK `EXTRA`/2 ja /3 (mäkihyppy) lähetetään `MAKI`-ehdolla, mutta vastaanotto on ehdon `MAKIx` takana, joten sitä ei käännetä | `Hk/HkCom32.cpp:633` | Tieto. Ei vaikuta GUI-käännöksiin |
| SAN-5 | M | `luepakettiUDP` käyttää vastaanotetun paketin luokkaa taulukon indeksinä (`class_len[cbuf->pkgclass]`) ennen kuin tarkistaa sen. HK:ssa luokka on etumerkillinen `char`. Virheellinen paketti voi lukea taulukon ulkopuolelta | `Tp/TpComY.cpp:997–1000` | Tarkista `0 <= pkgclass < N_CLASS` ennen indeksointia. Lisää testi |
| SAN-6 | A | `SIIMPORT`-käännöksessä `opencom` käynnistää funktion `luepakettiTCP`, mutta toteutuksen nimi on `luepakettiTCPSI`. Ei käänny, jos `SIIMPORT` määritellään | `Tp/TpComY.cpp:2222`, `:1958` | Tieto. Korjaa, jos ominaisuus otetaan käyttöön |
| SAN-7 | M | Paketissa ei ole versio- eikä rakennetunnistetta. Avaussanoma tarkistaa vain tietueiden määrän ja vaiheen. Eri versioiden tai käännösparametrien ohjelmat voivat tulkita paketit väärin, jos pituudet sattuvat täsmäämään | `tark_alku` | Lisää avaussanomaan rakenneversio ja tietueiden koot (`kilprecsize0`, `osrecsize0`). Edellytys IOF-tunnisteen muutosehdotukselle |
| SAN-8 | M | `tallenna` kutsuu `paivita_aikanaytto` → `SendMessage(FormMain, WM_MYUUSIAIKA)` usein `tall_CriticalSection`-lukon sisällä. Lukon pitäjä odottaa käyttöliittymäsäiettä | `Hk/HkDat.cpp:1261`, `Juk/vdat.cpp:819` | REST-toteutuksessa: tallennuksen viive riippuu käyttöliittymästä, eikä REST-säie saa pitää muita lukkoja tallennuksen aikana. Harkitse `PostMessage`ia (erillinen muutos) |
| SAN-9 | M | Ajan lähdettä (laite, radio, kortinluenta, käsisyöttö, verkko) ei siirretä sanomissa. Vastaanottaja tietää vain, että aika muuttui | `VAIN_TULOST` | Tieto vaatimuksille (V-WS-6, K22). Lähteen välitys vaatisi uuden kentän tai luokan ja SAN-7:n versioinnin |
| SAN-10 | I | SI-kortinluennan `package` = 30000000 + lukija. Vanha versio, joka ei tunne arvoa, käsittelee luennan emit-luentana. Badge-näyttö (`BADGEASKEL`) on haarassa ajonaikainen | haara `Hk/HkDef.h` | Kaikilla verkon koneilla sama versio mergen jälkeen. Kirjaa julkaisutietoihin |
| SAN-11 | I | Tiedonsiirtoportit eivät todenna lähettäjää. `EXTRA`/9 sulkee ohjelman, ja `FILESEND` kirjoittaa tiedoston suhteellisella polulla, absoluuttisella vain jos `lahkaikkitied` | luvut 3.1 ja 7 | Suljettu verkko on edellytys (V-SEC, K7). Ei muutosta tässä työssä |

## 14. Ylläpito

- Uusi sanomaluokka tai alakoodi kirjataan tähän dokumenttiin: lähettäjä, vastaanottaja, sisältö,
  rooli sekä yhteystyyppien suodattimet (2.3).
- REST-työn vaihe 0 (SI-haaran merge) päivittää luvun 12 haaran lopulliseen tilaan.
- Havainnot SAN-1 ja SAN-5 kannattaa korjata ennen REST-vaihetta 1, koska REST-kone käyttää
  samaa vastaanottoreittiä.
