# K-5: riippumaton tarkastus, käsikirjan luvut 04, 05, 08, 09

Tarkastettu 2026-11-27, vain luku. Koodi: `TPsource/V52/` työpuu (tuotantokoodi `f7fe553`).
Polut suhteessa `TPsource/V52/`, ellei toisin mainita. Kaikki näyttö on staattista lukua;
mitään ei käännetty eikä ajettu.

Rajaus: jokainen [päätelty]-väite ja ne "luettu käsikirjaa varten" -väitteet, joilla on
toiminnallista painoa (lukot, paluuarvot, kuittaus, valitsimet, ohjelmakohtaisuus); rivinumerot ja
asetusnimet otoksena. Ei uutta auditointia; kolme sivuhavaintoa on merkitty ehdokkaiksi (luku 6).

## 1. Väitteet ja tulokset

### Luku 04 Ajanotto

| # | Väite (lyhennetty) | Tulos | Oma näyttö |
|---|---|---|---|
| 04-1 | Aika 1/10 000 s; `tall_rivi` kasvattaa aikaa, kunnes se poikkeaa | VAHVISTETTU | `Tp/TpDef.h:82,87`; `Hk/HkAjat.cpp:257-263` |
| 04-2 | `aikatp`-kentät; viestissä osuus ja lähde (luettu `VIv.cpp`:n käytöstä) | VAHVISTETTU | `Hk/HkDef.h:937-947`; viestin kentät suoraan `Juk/VDef.h:1029-1040`. 22/24 tavua täsmää (pack 1: `HkDef.h:219`, `VDef.h:295`) |
| 04-3 | MAXJONO 4/24; VL 24 [päätelty] | VAHVISTETTU | `Hk/HkDef.h:74`; `Juk/VDef.h:36` ehdoton, joten VL on todennettu |
| 04-4 | `tall_rivi` vaiheet 1-6 | VAHVISTETTU | `Hk/HkAjat.cpp:114-291`. Lisäys: konsolissa (`_CONSOLE`, `:170-196`) korvaus voi kysyä vahvistuksen K/E |
| 04-5 | `tall_rivi` on ainoa jonoon kirjoittava funktio | OSITTAIN | Ajon aikana kyllä; alustus ja luku kirjoittavat `puttm`illa (`Hk/HkIV.cpp:4008`, `:4096`, `:4223`) |
| 04-6 | `tall_regnly`: rivi ensin, kilpailija toiseksi, kilpailija rivin onnistumisesta riippumatta | VAHVISTETTU | `Hk/HkIV.cpp:2986-3025`: `set_tulos` muistissa ennen riviä, `tall_rivi`n paluuarvo hylätään, `tallenna` `tall`-lukon alla |
| 04-7 | Timy `c`/`h`/`k`/`e` -kehykset | VAHVISTETTU | `Hk/HkIV.cpp:2348-2364` |
| 04-8 | KELLO_ESTO paluu 0; RTR `maxero` 1. ylitys paluu 1 | VAHVISTETTU | `Hk/HkIV.cpp:2771-2777` |
| 04-9 | Jono ei alustettu tai täynnä: ilmoitus, ei tallennusta | VAHVISTETTU | `Hk/HkIV.cpp:2778-2784`, paluu **1** |
| 04-10 | §4 "Kellon kuittaus": kuitataan, vaikka aika hylätään (jono täynnä, ei alustettu) | KUMOTTU osittain | Nämä palauttavat 1 (`Hk/HkIV.cpp:2783`), jolloin Emit-kello saa uusintapyynnön eikä `+`:aa (`Tp/TpLaitteet.cpp:910-916`). KELLO_ESTO ja `tall_rivi`n kirjoitusvirhe ovat oikeita esimerkkejä. Timy ym. eivät käytä paluuarvoa (`Tp/TpLaitteet.cpp:716`, `:780`) |
| 04-11 | Hiihtolähtö: ensimmäinen osuma [päätelty] | VAHVISTETTU | `Hk/HkIV.cpp:153-175`; ehto `tunn_srj >= 0` puuttuu luvusta |
| 04-12 | `LÄHTÖIKKUNA=x` sekunteina, oletus 3 s | KUMOTTU (yksikkö) | Oletus `3*SEK` = 3000 ms (`Hk/HkGlobals.cpp:96`), jäsennin `tunn_raja = _wtoi(...)` ilman `*SEK` (`Hk/HkInit.cpp:1924`), vertailu ms:nä (`Hk/HkIV.cpp:167`). `LÄHTÖIKKUNA=3` = 3 ms; käyttöohje sanoo sekunteja (`docs-site/docs/hkkisawin/hkliite1.md:127`) |
| 04-13 | LÄHTÖRYHMÄ: yksi signaali kaikille erän osanottajille | OSITTAIN | Peräkkäiset ajat +1 oikein (`Hk/HkIV.cpp:2970-2991`); kilpailija kohdistetaan vain lajeissa N/P/U (`:2977-2979`), muuten rivit kno = 0 |
| 04-14 | `tall_etulos`: piste -1 vain PAKOTALAIKA- tai hiihtolähtötilassa | OSITTAIN | Piste -1 voi tulla `AIKALUKIJAx=VAINL`:stä tai lähdepisteestä; PAKOTALAIKA tai hiihtolähtö ratkaisee vain kirjauksen kilpailijalle (`Hk/HkIV.cpp:687-707`) |
| 04-15 | PAKOTALAIKA: myöhemmät luvut vain rajan sisällä | OSITTAIN | Ilman `PAKOTALAIKARAJA`a (oletus 0, `Hk/HkGlobals.cpp:404`) jokainen luku korvaa (`Hk/HkIV.cpp:591`) |
| 04-16 | `tall_etulos`: aika kilpailijalle vain, jos lähtöaika on | OSITTAIN | `A && B \|\| C` (`Hk/HkIV.cpp:676`): UUSINAIKA-haara ohittaa lähtöaikaehdon, ja `tls0` voi olla alustamaton |
| 04-17 | ESTÄNEG: hylkäys ilman jonoriviä | VAHVISTETTU | `Hk/HkIV.cpp:671-675` (vain kun `piste >= 0`) |
| 04-18 | §3 ESTÄYLIM: "ajanottorivin tallennus estetään" | OSITTAIN | Rivi tallennetaan, `status = olitulos` (`Hk/HkIV.cpp:3000-3004`); `haeAktAjat` piilottaa sen `atimemask`in mukaan (`Tp/TpLaitteet.cpp:195`) |
| 04-19 | `LÄHAIKAy=V` → vain AIKAT-paketteja | OSITTAIN; HK:ssa KUMOTTU | Viesti kyllä (`Juk/VCom.cpp:260`, `:363`, `:768`, `:799`). HK:n `laheta` (KILPT, KILPPVT, VAIN_TULOST, `Hk/HkCom32.cpp:186-188`) ja `laheta_em` (EMITT, `:773-775`) eivät tarkista `com_ajat == 2`:ta; rajaus koskee vain seura-, EXTRA-, tiedosto- ja kopiolähetystä (`:469`, `:529`, `:681`, `:701`, `:1319`; `Tp/TpComY.cpp:2980`) |
| 04-20 | `tark_aika` kirjoittaa vain jonon rivin [päätelty] | VAHVISTETTU | `Hk/HkCom32.cpp:401-455`; ainoa kilpailijahaara `talltulos` on `#if defined(TARK_LEIMAT)` (kuollut, `valitsimet.csv:272`). Voi merkitä [todennettu] |
| 04-21 | Viestin `com_ajat == 3`: lähdeyhteyden esto pois [päätelty] | VAHVISTETTU | `Juk/VCom.cpp:487`; arvo tulee asetuksesta `LÄHAIKAy=P` (vain viesti, `Juk/VInit.cpp:1265-1266`), joka puuttuu luvusta |
| 04-22 | VL: `tall_regnly`n runko pois | VAHVISTETTU | `Juk/VIv.cpp:1763-2238`; `VS/V/JukLuenta520.vcxproj:95` |
| 04-23 | VL: ei `AIKA_COM`ia, ei `tall_etulos`ta | VAHVISTETTU | `Juk/VDef.h:48-50`; `Tp/TpLaitteet.cpp:2402`; `Juk/VIv.cpp:157` |
| 04-24 | Viestin kellotuki ALGE, ALGE4, Comet, Timy, RTR | OSITTAIN | Myös FEIG, IMPINJ, SIRIT, ZEBRA (`Juk/VInit.cpp:1594-1625`) |
| 04-25 | §1.9 VUOROMAALI kieltää maalin automaattivalinnassa | KUMOTTU | Ainoa käyttö on viestin `tall_etulos` (`Juk/VIv.cpp:626`), jossa asetus **valitsee** maalin (piste 0), kun väliajat ovat täynnä; ilman sitä piste = `valuku+1`. Maalikellon `tall_regnly` ei käytä asetusta |
| 04-26 | Valitsintaulukko (ENNAKOI, AIKAVERT, TESTIAJAT, AUTOVA, VALUKIJA, MILLISEKAJAT) | VAHVISTETTU | `HkMaali.vcxproj:118`; `JukMaali520.vcxproj:97`; `HkKisaWin.cbproj:149`, `:180-181`; `ViestiWin.cbproj:134` (+ `Juk/VDef.h:38-40`); `Hk/HkDef.h:26` |
| 04-27 | §7 `maxntime` ei selvitetty | Täydennettävissä | `Hk/HkIV.cpp:4210-4215` |
| 04-28 | Päivän vaihtuminen [päätelty] | EI TODENNETTAVISSA | vaatii laitteen tai tallennetun liikenteen |

### Luku 05 Leimaukset

| # | Väite | Tulos | Näyttö |
|---|---|---|---|
| 05-1 | `tark_badge` asettaa vain T tai I | VAHVISTETTU | `Hk/HkEmit.cpp:2263-2311`; ylikirjoittaa aiemman merkinnän |
| 05-2 | `tarkista`n paluuarvot 0/-1/n/-2 | VAHVISTETTU | `Hk/HkEmit.cpp:2037-2039`, `:2059`, `:2231-2236`; viestin `tarkista`ssa ei ole -2:ta, ja viestin `tark_badge` käsittelee negatiivisen arvon epäonnistumisena (`Juk/VEmit.cpp:2697`) |
| 05-3 | `hyv_pv` laskee hyväksytyiksi -, I, T | VAHVISTETTU | `Hk/HkTls.cpp:1208-1211`; myös B, kun `hyvHavaittu` |
| 05-4 | `ytark`: I kuin - | VAHVISTETTU | `Hk/HkTls.cpp:1523-1524`, `:1536-1537` |
| 05-5 | Ei aikarajoja [päätelty] | EI TODENNETTAVISSA kokonaan | koko `tarkista` ja `kuvio` lukematta; merkintä oikea |
| 05-6 | `laskeEm_VA`: puuttuva loppurasti → 5 h | VAHVISTETTU | `Hk/HkEmit.cpp:6226-6228`; tulos 5 h - alku + pohja; puuttuva alkurasti antaa hiljaa 0 (`rastiVA`, `:6184-6195`) |
| 05-7 | Analyysi 0...3 h | VAHVISTETTU | `Hk/HkEmit.cpp:2717` |
| 05-8 | AUTOSARJA/AUTORATA-ehdot | VAHVISTETTU | `Hk/HkIV.cpp:1675-1698` |
| 05-9 | LEIMAT.LST `Hk/HkInit.cpp:2354` | OSITTAIN, väärä viite | `Hk/HkEmit.cpp:2335-2370`; ilman tiedostoa koodit = rastitunnukset (`:2359-2360`) |
| 05-10 | §2: `tark_badge` ottaa lukon vain tallennukseksi | KUMOTTU | `tall`-lukko kattaa `GETREC`in, `tarkista`n ja `tallenna`n (`Hk/HkEmit.cpp:2271-2307`) |
| 05-11 | Merkintä tallessa ilman maaliaikaa [päätelty] | VAHVISTETTU (järjestys) | `Hk/HkIV.cpp:1632`, sitten `:1657` ja `tallkilp`; seurauksen näyttäminen vaatii vikaruiskutusajon |
| 05-12 | VL: leimantarkastus ei [päätelty] | VAHVISTETTU | `Juk/VEmit.cpp` ei ole VL:n projektissa (`JukLuenta520.vcxproj:165-197`); kutsu on `#ifndef LUENTA` -lohkossa (`Juk/VIv.cpp:1116`, `:1581`). Huom: `EMITLEIMAT` on määritelty VL:ssä (`Juk/VDef.h:74-76`) |
| 05-13 | Viestin `tark_badge` | VAHVISTETTU | `Juk/VEmit.cpp:2674-2735` |
| 05-14 | ESITARK: VW "asetus", §3 "viestissä ESITARK" | KUMOTTU VW:n osalta | jäsennin `#ifdef ESITARK` (`Juk/VInit.cpp:2111`), valitsin vain VM:ssä (`JukMaali520.vcxproj:97`, `valitsimet.csv:152`); ViestiWin ei aseta `esitark`ia |
| 05-15 | EBA kuollut | VAHVISTETTU | `Hk/HkIV.cpp:1701-1705`; `valitsimet.csv:140` |
| 05-16 | Kuntosuunnistuksen koodinvaihto vain HKW | VAHVISTETTU | `Hk/HkIV.cpp:1663-1672`; `HkKisaWin.cbproj:181` |
| 05-17 | `pros_emit` pääsäikeessä (HKW) | VAHVISTETTU | `cbHk/WinHk.cpp:1611-1637` → `SendMessage` (`cbHk/UnitEmit.cpp:962`, `:969`) |

### Luku 08 Vaiheen vaihto

| # | Väite | Tulos | Näyttö |
|---|---|---|---|
| 08-1 | `N_PV` 5, 200VA:ssa 2 | VAHVISTETTU | `Hk/HkDef.h:45-46`; `HkKisaWin.cbproj:101` (Config 200Release), `:299`, `:306`; oletus Release (`:8`) |
| 08-2 | VAIHE=n/V; arvo 0 cfg:ssä → kysymys | VAHVISTETTU | `Hk/HkInit.cpp:289-311`, `:703-711`, `:3188-3192`, `:3314-3322`; kysymys vain kun `ToimintaTila == 2` ja `n_pv_akt > 1`; suora `VAIHE=0` leikataan nollaan (`:708-709`) |
| 08-3 | `vaihdavaihe` 1-11 ja rivit | VAHVISTETTU | `Hk/HkIx.cpp:1855-2005` |
| 08-4 | Vaihe 10: merkintä levyjonoyhteyksille, paitsi lähteelle | OSITTAIN | `levypusk` on koko ohjelman muuttuja (`Hk/HkGlobals.cpp:363`; `Hk/HkIx.cpp:1975`); `lah_alku = 1` myös lähteelle, vain `lah_vaihto = 0` (`:1979-1980`); `opencomfile(1)` ajetaan toisesta yhteydestä alkaen kerran yhteyttä kohden (`:1976-1978`) |
| 08-5 | §1.6, §3, §7: muistijonoyhteydellä oleva asema ei saa pyyntöä [päätelty] | KUMOTTU (kehys) | ratkaisee vaihtavan aseman oma `COMFILE=EI`: silloin pyyntöä tai aloituspakettia ei lähde millekään yhteydelle; vastaanottajan jonotyypillä ei ole merkitystä. Sama kehys on KE-22:ssa |
| 08-6 | `tark_alku`: eri pituinen KILP.DAT → vain varoitus | OSITTAIN | konsolissa vastaanottosäie kysyy "J)atka, P)oistu" ja voi kutsua `lopetus()`ia (`Hk/HkCom32.cpp:857-864`) |
| 08-7 | 5 s:n raja ainoa pysäytys [päätelty] | VAHVISTETTU | `Hk/HkCom32.cpp:844-848` (staattinen `edvaihto`, yhteinen kaikille yhteyksille) sekä `vaiheenvaihto`-lippu (`cbHk/WinHk.cpp:2154`; `Hk/HkIx.cpp:1868`) |
| 08-8 | GUI `SendMessage`, konsoli suora kutsu | VAHVISTETTU | `cbHk/WinHk.cpp:2231-2235`; `HkConsole/HkCons0.cpp:869-872` |
| 08-9 | Etäkäsittelijä tekee saman kuin valikko | OSITTAIN | ei `Sleep(500)`:aa säikeiden pysäytyksen jälkeen (vrt. `cbHk/WinHk.cpp:2084` ja `:2157-2160`) |
| 08-10 | "Lue sarjat uudelleen" = sama funktio, lähettää pyynnön | VAHVISTETTU | `cbHk/WinHk.dfm:656-664`; `cbHk/WinHk.cpp:2060-2061`, `:2121` |
| 08-11 | Aikajonoa ei lueta uudelleen, kun ajanotto on pois [päätelty] | OSITTAIN | koskee vain jonoa 0 (`Hk/HkIx.cpp:1958-1972`); koko käsittely ohittuu, jos `aikajono[0]` puuttuu (`:1900`) |
| 08-12 | Asetustiedostoja ei lueta | VAHVISTETTU | ainoa luku on `luesarjat` (`Hk/HkSrj.cpp:62`) |
| 08-13 | Viestin `k_pv` kuolleissa lohkoissa [päätelty] | VAHVISTETTU | `Juk/VTulostus.cpp:7274`, `:7338`, `:7351`, `:7672`; `valitsimet.csv:181`, `:182`, `:275` |
| 08-14 | LOKI1 vaihekohtainen | OSITTAIN | nimi asetetaan vain käynnistyksessä (`Hk/HkInit.cpp:3367`); `vaihdavaihe` ei muuta sitä [päätelty] |

### Luku 09 Tiedonsiirto

| # | Väite | Tulos | Näyttö |
|---|---|---|---|
| 09-1 | MAX_LAHPORTTI 64 kaikissa viidessä | VAHVISTETTU | `Hk/HkDef.h:82`; vcxprojit; `ViestiWin.cbproj:195`, `:211` |
| 09-2 | BRO ilman kuittausta | VAHVISTETTU | `Hk/HkInit.cpp:1112-1115`; `Tp/TpComY.cpp:1531-1536` |
| 09-3 | Elossaolo: 5, lähettäjän NAK vähentää, "Ei" yli 10 kierroksen jälkeen | OSITTAIN | myös vastaanottaja vähentää yli 1 s:n tauon jälkeen (`Tp/TpComY.cpp:976-983`); vastapuolen NAK nostaa arvoon 5 (`:976-990`), joten "On" voi tarkoittaa pelkkiä tyhjäkäyntikyselyjä |
| 09-4 | Elävyysvalvonta vain UDP:ssä [päätelty] | OSITTAIN | TCP:llä oma "Ei"-tila (`Tp/TpComY.cpp:1285-1287`) |
| 09-5 | `buflah`; muistijono 50, varoitukset 25 ja 38 | VAHVISTETTU | `Tp/TpComY.cpp:476-535`; `Hk/HkDef.h:144` |
| 09-6 | Kuittaus ennen käsittelyä [päätelty] | VAHVISTETTU | ACK, kun `addseur` onnistuu (`Tp/TpComY.cpp:1048-1056`, `:1084-1092`) |
| 09-7 | Yksi käsittelemätön paketti; muuten NAK [päätelty] | VAHVISTETTU | `Tp/TpComY.cpp:648-650`, `:1097-1100` |
| 09-8 | Edellinen tai kaksi edellistä numeroa kuitataan toistona | OSITTAIN | vain välittömästi edellinen kuitataan (`Tp/TpComY.cpp:1026-1027`); vanhemmat saavat NAK:n |
| 09-9 | `tarkcom` 50/15 ms | VAHVISTETTU | `Hk/HkCom32.cpp:882`; `Juk/VCom.cpp:900` |
| 09-10 | XML-TCP etenee kirjoituksen onnistuessa | VAHVISTETTU | `Hk/HkCom32.cpp:1048-1075`; myös `len == 0` siirtää jonoa |
| 09-11 | EMITT-käsittelyn ehdot; VL ei vastaanota | VAHVISTETTU | `Hk/HkCom32.cpp:917-924`; `Juk/VCom.cpp:950-962` |
| 09-12 | VL:n AIKAT → virheilmoitus | VAHVISTETTU | `Juk/VCom.cpp:934-940` |
| 09-13 | Tuntematon luokka: HK ohittaa, viesti pysähtyy | VAHVISTETTU | `Hk/HkCom32.cpp:942-947`; `Juk/VCom.cpp:970-974` |
| 09-14 | COMFILE=EI kommentoitu pois viestissä | VAHVISTETTU | `Juk/VInit.cpp:1216-1220` |
| 09-15 | LÄHKELLO ja TCPTULOS vain HK | VAHVISTETTU | `Hk/HkInit.cpp:1480`, `:1029` |
| 09-16 | HYVKAIKKI ohittaa numerotarkistuksen ja kuittaa | VAHVISTETTU, täydennettävä | `Hk/HkInit.cpp:1092-1096`; `Tp/TpComY.cpp:1043-1044`: `hyvp` asetetaan ennen `addseur`ia, joten myös jonoon mahtumaton paketti kuitataan |
| 09-17 | TCP-asiakas ei yritä uudelleen; katko pysyvä (§1.2, §1.8, §4) | KUMOTTU osittain | muodostetulla yhteydellä on uudelleenyhteys (`Tp/TpComY.cpp:1285-1290`; `com/WinUDP.cpp:617-627`); pysyvä vain epäonnistunut ensimmäinen avaus (`Tp/TpComY.cpp:1273-1275`). Sama H-027-kortissa (rivit 25-28) ja KE-19:ssä |
| 09-18 | COMFILE-kysymys, kun on lähetettävää | OSITTAIN | kysytään, kun tiedostossa on yksikin paketti; vaiheen vaihdossa ei kysytä (`Tp/TpComY.cpp:2331`) |
| 09-19 | Valitsintaulukko 3.2 | VAHVISTETTU | `valitsimet.csv:185`, `:257`, `:259`, `:261`, `:276`, `:277`, `:298` |
| 09-20 | "Tiedostot eri koneilla eivät ole yhtäpitäviä" | VAHVISTETTU | `Hk/HkCom32.cpp:339-348` |

## 2. Korjattavat kohdat

| Luku | Nykyinen väite | Korjaus | Lähde |
|---|---|---|---|
| 04 §1.7, §3 | `LÄHTÖIKKUNA=x` sekunteina | arvo luetaan millisekunteina, oletus 3000 ms; merkitse ristiriidaksi käyttöohjeen kanssa ja ehdokkaaksi | `Hk/HkInit.cpp:1924`; `Hk/HkGlobals.cpp:96` |
| 04 §1.8 (ja huomiot 04/05/10) | `LÄHAIKAy=V` → vain AIKAT | viestissä kyllä; HK lähettää silti KILPT-, KILPPVT-, VAIN_TULOST- ja EMITT-paketit | `Hk/HkCom32.cpp:186-188`, `:773-775` |
| 04 §1.8, §3 | `com_ajat == 3` ilman asetusta | lisää `LÄHAIKAy=P` (vain viesti) | `Juk/VInit.cpp:1265` |
| 04 §1.9 | VUOROMAALI kieltää maalin | koodi valitsee maalin, kun väliajat ovat täynnä, vain `tall_etulos`issa; ristiriita käyttöohjeen kanssa | `Juk/VIv.cpp:626` |
| 04 §1.7 | LÄHTÖRYHMÄ kaikille erän osanottajille | vain lajeissa N/P/U; muuten n riviä ilman kilpailijaa | `Hk/HkIV.cpp:2970-2991` |
| 04 §1.5 kohdat 3-4 | piste -1 ja PAKOTALAIKA-raja | ks. 04-14 ja 04-15 | `Hk/HkIV.cpp:591`, `:687-707` |
| 04 §3 | ESTÄYLIM estää rivin tallennuksen | rivi tallentuu `status`-merkinnällä | `Hk/HkIV.cpp:3000-3004` |
| 04 §4 "Kellon kuittaus" | jono täynnä tai ei alustettu saa kuittauksen | näissä paluu 1 → Emit-kellolle uusintapyyntö; muut kellot eivät käytä paluuarvoa | `Hk/HkIV.cpp:2783`; `Tp/TpLaitteet.cpp:716`, `:780`, `:910-916` |
| 04 §1.3 | ainoa jonoon kirjoittava funktio | "ainoa ajonaikainen" | `Hk/HkIV.cpp:4008`, `:4223` |
| 04 §1.1, §2 | VL MAXJONO [päätelty]; viestin kentät "käytöstä" | [todennettu] | `Juk/VDef.h:36`, `:1029-1040` |
| 04 §7 | `maxntime` ei selvitetty | kaava | `Hk/HkIV.cpp:4210-4215` |
| 05 §1.3 | `Hk/HkInit.cpp:2354` | `Hk/HkEmit.cpp:2335-2370`; ilman LEIMAT.LST:tä koodit = rastitunnukset | sama |
| 05 §2 (jälkikappale) | `tark_badge` lukitsee vain tallennuksen | lukko kattaa `GETREC`, `tarkista` ja `tallenna` | `Hk/HkEmit.cpp:2271-2307` |
| 05 §2, §3 | ESITARK VW:ssä ja "viestissä" | vain VM | `Juk/VInit.cpp:2111`; `valitsimet.csv:152` |
| 05 §2 | VL [päätelty] | [todennettu]: `VEmit.cpp` puuttuu VL:n projektista | `JukLuenta520.vcxproj:165-197` |
| 05 §1.6 | `hyv_pv` -, I, T | lisää B (`hyvHavaittu`) | `Hk/HkTls.cpp:1210` |
| 05 §1.9a | 5 h | 5 h - alku (+ pohja); puuttuva alkurasti → 0 | `Hk/HkEmit.cpp:6219-6232` |
| 08 §1.4.10, §1.6.1, §3, §7 | vaihtopyyntö "levyjonoyhteyksille"; muistijonoasema ei saa pyyntöä | `levypusk` on globaali: vaihtava asema, jolla `COMFILE=EI`, ei lähetä pyyntöä eikä aloituspakettia millekään yhteydelle; lähde saa aloituspaketin ilman lippua; korjaa myös KE-22 | `Hk/HkIx.cpp:1974-1981`; `Hk/HkGlobals.cpp:363` |
| 08 §1.6.2, §4 | eri pituinen KILP.DAT vain varoittaa | konsolissa J/P-kysymys vastaanottosäikeessä | `Hk/HkCom32.cpp:857-864` |
| 08 §1.6.3 | etäkäsittelijä tekee saman kuin valikko | ei 500 ms:n odotusta | `cbHk/WinHk.cpp:2084` |
| 08 §1.2 | arvo 0 → kysymys | vain tulospalvelutila ja `n_pv_akt > 1`; vain cfg-tiedosto | `Hk/HkInit.cpp:3191`, `:3314`, `:708` |
| 08 §1.5 | aikajonot [päätelty] | vain jono 0; ohittuu, jos `aikajono[0]` puuttuu | `Hk/HkIx.cpp:1900`, `:1958-1972` |
| 08 §1.5 | (puuttuu) | lokin nimi ei vaihdu [päätelty] | `Hk/HkInit.cpp:3367` |
| 08 §7 | "H-129 ja KE-taulukko eivät mainitse" | viittaa KE-21:een | `doc/arkkitehtuuri/seuranta.md:388` |
| 09 §1.2, §1.8, §4 | TCP-katko pysyvä | vain epäonnistunut ensimmäinen avaus; katkennut yhteys yritetään uudelleen | `Tp/TpComY.cpp:1273-1290`; `com/WinUDP.cpp:617-627`; H-027-kortti; KE-19 |
| 09 §1.3 | laskuri ja valvonta | ks. 09-3 ja 09-4 | `Tp/TpComY.cpp:976-990`, `:1285-1287` |
| 09 §1.8.1 | kaksi edellistä numeroa kuitataan | vain edellinen | `Tp/TpComY.cpp:1019-1030` |
| 09 §3.1 | HYVKAIKKI | kuittaa myös jonoon mahtumattoman paketin (kuittaus ≠ vastaanotto) | `Tp/TpComY.cpp:1043-1056` |
| 09 §1.11 | kysymys, kun on lähetettävää | kysytään, kun on yksikin paketti; ei vaiheen vaihdossa | `Tp/TpComY.cpp:2331` |

## 3. Väärin kytketyt tai puuttuvat havainnot (`toiminnot.csv`)

Kaikki avoimet P1–P3-havainnot on kytketty johonkin lukuun (tarkistettu skriptillä `riskiarvio.csv`:tä vasten).

Puuttuvat kytkennät:
- **H-056** (ARES-maalikellon lukijasäie pyörii, ajanottosyöte pysähtyy) puuttuu luvun 04 riviltä "Maalikellon ajanotto"; se on nyt vain luvussa 05.
- **H-136** ja **H-094** ovat luvun 04 virhetaulukossa mutta eivät luvun 04 kytkennöissä; H-136 siirtää maali- ja väliaikoja.
- **H-090** puuttuu luvun 04 riviltä "Tunnistinajanotto", vaikka §1.5 kuvaa sen TUNNJONO-tilassa.
- **H-164**: jos sama silmukka koskee maalikelloja, kytke myös lukuun 04 [tuntematon].

Kyseenalaiset kytkennät:
- Luku 05 "Emit-tietueen ja tuloksen välitys": H-110 ja H-162 koskevat vastaanoton pysäyttävää lukkoa; parempi paikka on luvun 09 "Vastaanotto ja kuittaus".
- Luku 05 "Emit-kortin käsittely": H-146 ja H-090 vain `EMITAJAT`-polulla; perustelu puuttuu `lahde`-sarakkeesta.
- Luku 09 "Vastaanotto ja kuittaus": H-095 kuuluu riville "Ajanottotietojen siirto", jossa se jo on.

## 4. Ristiriidat vikakorttien ja käsikirjan välillä

- H-027-kortti (rivit 25-28) ja KE-19 ovat oikeassa, luku 09 väärässä (TCP-uudelleenyhteys).
- KE-21 on olemassa, vaikka luku 08 §7 sanoo muuta.
- KE-22:n kohta "vaihtopyynnön lippu vain levyjonoyhteyksille" perustuu samaan väärään kehykseen kuin luku 08.
- VUOROMAALI: kirjoittajien huomiot kuvaavat koodin oikein, mutta luku 04 §1.9 ei mainitse ristiriitaa.
- LÄHAIKAy=V: kirjoittajien huomiot ja luku 04 yleistävät viestin toiminnan HK:hon.

## 5. Järjestelmälliset virhetyypit

1. **HK:n ja viestin yleistys**: LÄHAIKAy=V, ESITARK VW:ssä, -2-paluuarvo viestissä, VUOROMAALI.
2. **Globaali vai yhteyskohtainen**: `levypusk`, mistä seuraa väärä ehdokas (luku 08, KE-22).
3. **Kutsujan paluuarvon käsittely tarkistamatta**: kellon kuittaus, HYVKAIKKI, UDP-toisto.
4. **Asetuksen yksikkö ja oletus tarkistamatta jäsentimestä**: LÄHTÖIKKUNA, PAKOTALAIKARAJA = 0.
5. **Samana päivänä tehtyjä tarkennuksia ei seurattu**: KE-19, KE-21, H-027-kortti.
6. **[päätelty] liian heikkona**, vaikka yksi define- tai projektirivi todentaa: VL MAXJONO, VEmit.cpp, TARK_LEIMAT.
   [AA]-väitteitä on otettu aineistosta tarkistamatta: 05-10, 09-17.
7. **Väärä tiedostoviite**: 05-9; muut otoksen rivinumerot täsmäsivät.

## 6. Sivuhavaintoja (ehdokkaita; ei tarkastettu, rekisteriä ei haettu kokonaan)

- **L1** `LÄHTÖIKKUNA` luetaan millisekunteina (`Hk/HkInit.cpp:1924`); `doc/arkkitehtuuri` ei mainitse yksikköä.
- **L2** `tall_etulos`: UUSINAIKA ohittaa lähtöaikaehdon, ja `tls0` voi olla alustamaton (`Hk/HkIV.cpp:676`).
- **L3** `tark_badge`: `er` on alustamaton, kun `esiluenta == 1` (`Hk/HkEmit.cpp:2273-2275`, `:2308`).

## 7. Avoimet kysymykset

- A1: Mikä `atimemask` on oletuksena, eli piiloutuvatko ESTÄYLIM-rivit näkymästä? Selviää asetuskohdasta tai ajolla.
- A2: Vaikuttaako 500 ms:n odotuksen puuttuminen etäpyynnössä? Selviää ajolla, jossa kortti luetaan vaihtopyynnön aikana.
- A3: Koskeeko H-164 myös `lue_regnlyThread`ia? Selviää lukemalla silmukka.
- A4: Kun `aikajono[0]` puuttuu mutta jono 1 (LAJAT) on olemassa, jono 1 nimetään uudelleen ilman suljentaa ja uudelleenlukua (`Hk/HkIx.cpp:1890-1903`). Kirjoittaako se vanhat rivit uuden vaiheen tiedostoon? Selviää ajolla.
- A5: Lokitiedoston nimi vaiheen vaihdon jälkeen. Selviää ajolla.
- A6: 05-5: koko `tarkista`- ja `kuvio`-polku on lukematta.
