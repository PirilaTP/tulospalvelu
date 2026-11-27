# 10 Laitteet: lukijat, emiTag, RFID, kellot, näyttötaulut, tulostimet

<!-- malli: Käsikirjan lukupohja (K-3). Ohje: doc/tietopohja/laatu/k3-kirjoittajan-ohje.md.
Luvun 5 vikaluettelo generoidaan tiedostosta kasikirja/toiminnot.csv komennolla
python -I doc/arkkitehtuuri/tyokalut/tietopohja.py . kasikirja; sitä ei kirjoiteta käsin. -->

> **Tila:** alustava K-3-aineisto, ei riippumattomasti tarkastettu. Lähderevisio: tuotantokoodi
> `f7fe553` (ennallaan, `doc/arkkitehtuuri/CHECKPOINT.md`). Varmuusmerkinnät: [todennettu],
> [päätelty], [tuntematon]; "luettu käsikirjaa varten" = luettu koodista K-vaiheessa, ei tarkastettu.

Luku kertoo, mitä laitteita ohjelmat tukevat ja miten laite liitetään ohjelmaan. Se nojaa
laiteauditointiin `doc/arkkitehtuuri/todisteet/c10/raportit/osaD-laitteet.md` (jäljempänä
"osaD") ja `doc/arkkitehtuuri/riippuvuudet.md` lukuun 13. Laitteen tuottaman tiedon käsittely
jatkuu luvuissa 04 (ajat) ja 05 (Emit-kortit). Polut ovat suhteessa hakemistoon `TPsource/V52/`.
Lyhenteet: HKM = HkMaali, HKW = HkKisaWin, VM = ViestiMaali, VW = ViestiWin, VL = ViestiLuenta.

Laitteiden toimintaa ei ole ajettu: yksikään tämän luvun väite laitteen käytöksestä ei perustu
laitteella tehtyyn kokeeseen (OP-13). Laitemallien nimet esiintyvät vain käyttöohjeessa
(osaD A11).

## 1. Mitä tapahtuu

### 1.1 Laitteen yhteys ohjelmaan

Ohjelman asetustiedostossa annetaan laitteen tyyppi ja yhteys, esimerkiksi `TIMY=7` (Timy
sarjaportissa COM7), `LUKIJA1=5` (Emit-lukija COM5) tai `LUKIJA1=TCP:192.168.1.20:4001`
[AA: käyttöohje `hkkisawin/5.1`, `5.2`]. Käynnistyksessä `start_regnly`
(`Tp/TpLaitteet.cpp:1879-2126`) avaa yhteyden ja käynnistää jokaiselle luku- tai kellolaitteelle
oman taustasäikeen `lue_regnlyThread` (`Tp/TpLaitteet.cpp:1747-1875`; jakso 20 ms, Siritillä 50 ms)
[todennettu: osaD luku 2; saikeet-ja-lukot.md luku 3].

Yhteystavat [todennettu: osaD luku 2]:

- **Sarjaportti.** `\\.\COM%d` (toimii myös yli COM9), nopeus kiinteä laitetyypin mukaan
  (taulukko kohdassa 1.2), ei laitteistokättelyä, RTS ja DTR päällä, ei XON/XOFF-ohjausta.
  Virheellinen avaus tuottaa ilmoituksen eikä uudelleenyritystä (`Tp/TpComY.cpp:675-681`).
  USB-sarjamuuntimen irrotusta tai uutta COM-numeroa ohjelma ei havaitse; ohjelma on avattava
  uudelleen oikealla numerolla [AA: käyttöohje `6.10`, `liite_7` ja osaD luku 6].
- **TCP-asiakas** (`LUKIJAn=TCP:ip:portti`, Sirit, Zebra): `connect()` ilman aikakatkaisua; yhteyden
  katketessa säie päättyy, ellei kyseessä ole aikakatkaisu; uusi yritys 15 s välein, kun
  `reconnect` on päällä (pakotettu pois Siritillä ja Zebralla). Avausyritysten viive on 5 s, 55 s
  ja 180 s, ja laskuri on yhteinen kaikille lukijoille (H-138).
- **UDP** ja **SjBox** (sarjaliikenne IP-verkon yli): `LUKIJAn=UDP` tai `TCP:`; `TCPLUKIJA`-valitsin
  on kaikissa viidessä ohjelmassa.
- **HTTP** (emiTag-palvelin, `EMITHTTP`) ja **SQL** (`ETSQL`): ks. luku 10.5 ja osaD osa C.

Ohjelma ei luettele USB-laitteita eikä seuraa laitteiden kytkentä- tai poistotapahtumia (osaD
yhteenveto). Jos TCP-lukija on "yhteydessä" mutta vastapää ei lähetä, ohjelma ei huomaa sitä ellei
käytössä ole `/P`-kysely (LUKIJA) tai `SIRITPOLL` (T-048).

### 1.2 Laiteperheet ja mitä ne tekevät

| Laite | Mitä antaa ohjelmalle | Yhteys ja nopeus | Asetus | Vastaanotto |
|---|---|---|---|---|
| **Emit-kello** (RTR1, RTR2, EMITKELLO; ent. Regnly) | maaliaika, väliaika; EMITKELLO lisäksi Emit-kortin tiedot | sarja 1200 (RTR1), 4800 (EMITKELLO) | `REGNLYx=n`, `EMITKELLO=n` | `lue_EmitKello` (`Tp/TpLaitteet.cpp:856-939`) |
| **Emit-lukija** (LUKIJA) | Emit-kortin koodi, leimat, luentahetki | sarja 9600 tai 115200 (`/U`), 2 stop; TCP, UDP | `LUKIJAx=n` | `lue_LUKIJA` (`:948-1122`) |
| **Emit MTR** | kortin leimat (vetoprotokolla `/GB`) | sarja 9600 | `MTRx=n` | `lue_MTR` (`:1126-1295`) |
| **emiTag** (ECB1, ETS1, ECU1) | leimat ja väliajat | sarja 9600 (`/U` 115200, `/C` 19200) | `EMITAGx=n` | `lue_EMITAG` (`:1299-1372`) |
| **emiTag palvelimelta** | väliajat HTTP:llä tai SQL:llä | WinHTTP, SQL SSH:n yli | `ETGPRSy`, `ETSQL`, `ETHOST`, `ETHAKUVÄLI`, `ETDATE`, `ETTIME` | `vahaku`, `haeSQLTh` |
| **ALGE Timy, S3, S4, Comet, ASC, TDC** | maalikellon, lähtökellon ajat | sarja: S3 2400 (7 databittiä, 2 stop), S4 4800, Comet 2400, Timy 9600, ASC 2400 | `TIMYx=n`, `ALGE=n`, `ALGE4=n`, `COMET=n`, `ASC=n` | yleinen rivinlukija (`:1584-1695`) |
| **ARES** | maalikello (SOH-EOT, 41 tavua) | sarja 9600 | `ARESn` | `lue_ARES` (`:696-736`) |
| **RADIOEMIT** | radiolinkillä välitetty Emit | sarja 9600 | `RADIOEMIT` | yleinen rivinlukija; laite on aineistosta tuntematon |
| **Sirit, Zebra FX9600** (RFID, UHF) | tunniste ja luentahetki | TCP: Sirit 50008/50007, Zebra 5084 (LLRP v1) | `SIRITn=TCP:..`, `ZEBRAn=TCP:..` | Sirit: `openSirit2`, `siritaika`; Zebra: `Tp/ZebraReader.cpp` |
| **FEIG** (RFID) | tunniste ja aika | sarja 38400, pariteetti E | `FEIG` | `:805-851` |
| **Impinj** | CSV-rivi "antenni, EPC, aika" ulkoisesta sillasta | TCP [päätelty: baud 0 taulussa] | `IMPINJ` | `tall_regnly`; alkuperä tuntematon |
| **CTS-pulssi** (AIKA_COM) | aika tietokoneen kellosta CTS-reunalla | sarja 9600 8N1 | `AIKA_COM=n`, `AIKA_ESTO=` | `comajanotto` (`:2405-2431`) |
| **Ampumasakkolaite** (KURVINEN) | sakkotieto ampumahiihtoon | sarja `sak_baud` | `SAKKO_COM`, `SAKKO_YHT` | `tall_ampsakko` |
| **Valot** (VALOT) | signaalivalot RTS/DTR | sarja 9600 | `VALOT=n` | lähetys `asetavalo` (konsoli) |
| **Tulostaulut** GAZ, ALGE, C-Power, RTNM | vain lähetys, ei lukua | sarja 2400, UDP, TCP | `GAZ=n`, `TAULU_COM=`, `CPOWERn=`, `RTNMn=` | ks. kohta 1.7 |

Lähteet: osaD luku 1 (laiteluettelo ja oletusnopeudet); rivit [AA]. Taulukko kuvaa
koodissa olevat tukirutiinit. Ohjelmakohtainen tuki on kohdassa 2.

### 1.3 Emit-lukijat ja kortinluku

Kortinlukijoiden (EMITKELLO, LUKIJA, MTR, emiTag) tekniikka on kuvattu luvuissa 05 ja osaD luvuissa
2 ja 4. Tiivistelmä käyttäjän kannalta [todennettu: osaD, riippuvuudet.md luku 13]:

| Laite | Kuittaus lukijalle | Kaksoisluennan esto | Tarkiste |
|---|---|---|---|
| Emit-kello (EMITKELLO) | `+\r`, kun `tall_emit` palauttaa 0 (myös hylätylle kaksoiskappaleelle ja kun tallennus epäonnistui, H-145); muuten `?\r` uusintapyyntö enintään 200 ms välein | `tall_emit` (`samatleimat`, H-168) ja `addem` (`on_emit`) | kenttäpituudet |
| RTR1, RTR2 | kellon kuittaus `tall_regnly`:n paluuarvosta | - | RTR2: 7-bittinen summa; RTR1: kenttäpituudet |
| LUKIJA | ei kuittausta | kortti hyväksytään, kun puskurissa on `lukutoisto` (oletus 2) identtistä sanomaa; 6 tikin portti | kaksi tarkistussummaa (XOR 0xDF) |
| MTR | vetoprotokolla: ohjelma pyytää `/GB` + pakettinumero; `pkg_alku` etenee `tall_emit`-kutsun jälkeen paluuarvosta riippumatta (N-6) | `addem` | summa |
| emiTag | ei kuittausta | `addem` | ei tarkistetta |

Kuittaus on siis ainoastaan Emit-kellon ominaisuus. Se kertoo, että ohjelma on yrittänyt kortin
vastaanottoa, ei sitä, että kilpailijan tulos on tallennettu (luku 05). Laitteen oma käytös
kuittaamatta jäävälle kortille on tuntematon (Q86, Q100, OP-13).

Kortin luenta, kaksoiskappaleet ja käsittely kuvataan luvussa 05; TCP-lukijoiden uudelleenyritys ja
valvonta kohdassa 1.9.

### 1.4 Maalikellot ja lähtökellot

Maalikello antaa ajan tai sarjan aikoja; laitteen kanava kuvautuu pisteeksi ja jonoksi
(`PISTEET=`, `JONOT=`; luku 04 kohta 1.4). Laitteen mukaan ohjelma tulkitsee sen kehyksen eri
tavalla [luettu käsikirjaa varten, ei tarkastettu: `Hk/HkIV.cpp:1984-2560`]:

- **Timy / TDC8001:** rivit 27 tavua; aikalaji `C` tai `F`; tunnus `c` (rivin peruutus) ja `h`/`k`/`e` (tilamerkinnät, vain kun kello lähettää kilpailijanumeron: `TIMYNO`); kanava 0...8;
  laitenumero 1 oletuksena, toinen Timy `TIMYn=` (useita samanaikaisesti, käyttöohje `5.1.3`).
  Timy voidaan liittää USB:llä apuohjelman `AlgeTimyUsbToTcp` kautta TCP:llä; ohjelmat eivät tue
  Timyn suoraa USB-yhteyttä [AA: käyttöohje `liite_7`].
- **ALGE S3 (Timer S3) ja S4:** S3 20-tavuiset rivit (osaD A10); S4:n tyhjä rivi ohitetaan.
  Kanavan tunnuskirjaimet (`alge_tunn`) kuvautuvat pisteeseen.
- **Comet:** rivi 22 tavua; kanavatunnus on 2, 4, 1 tai 8.
- **ASC1:** lähtökello, rivi 19 tavua.
- **Emit RTR1/RTR2:** kuittaus ja kellon ja tietokoneen kellon eron mittaus ("Maalikello on
  tietokoneen kelloa ... edellä/jäljessä", kehys `G`); RTR2 ampumasakkokehys `J`.
- **ARES:** SOH-EOT-kehys, 41 tavua.

Kellon aika = kellon ilmoittama aika + `t0_regn` (kellon ja tietokoneen kellon siirtymä; asetus
`AIKAERO`). Kellon ja tietokoneen kellon ero tarkistetaan vain RTR-perheellä. Kellon käytöstä ei
ole aineistoa laitteen puolelta: esimerkiksi kellon kuittauksen merkitys, `?\r`-uusintapyynnön
sisältö ja `-`-vastauksen merkitys ovat tuntemattomia (osaD luku 4).

**AIKA_COM.** CTS-signaalin nousu (valokenno, lähtöportti) antaa ajan tietokoneen kellosta:
`comajanotto` odottaa `WaitCommEvent(EV_CTS)`-tapahtumaa ja kutsuu `tee_aika(1)`; `AIKA_ESTO` rajoittaa
tihentymää (`50*aika_esto` ms). Silmukka ei nuku eikä käsittele virheitä, joten kadonnut portti jättää
sen pyörimään (osaD N-11) [todennettu: osaD luku 2, N-11].

### 1.5 RFID-lukijat (Sirit, Zebra, FEIG, Impinj)

RFID-tunnisteiden ajanotto on käännösvalitsimen `LAJUNEN` takana: mukana HKM, VM, HKW ja VW, ei
VL:ssä (T-052) [todennettu: `valitsimet.csv`, projektit]. Käyttöohje toteaa, että aktiivisessa
käytössä on Siritin laitteiden ja niiden kanssa yhteensopivien Motorolan laitteiden tuki
[AA: `hkkisawin/5.2`].

- **Sirit (FX9500):** kaksi TCP-yhteyttä (tapahtuma 50008, komento 50007), tekstimuotoiset
  tapahtumat kiinteine siirtymineen (`p+19` tunniste, `p+17` aika `HH:MM:SS.mmm`), liittymän
  tervehdys `event.connection id = N`, `10 < N < 99`. Aika-asetus `SIRITSYNC` asettaa lukijan
  kellon. Valvonta `SIRITPOLL` [todennettu: osaD luku 2, A1-A2].
- **Zebra FX9600:** LLRP v1, portti 5084; lukija on NTP-synkronoitu (UTC-aika). Ohjelma vastaa vain
  KEEPALIVE-viesteihin [todennettu: osaD luku 2, A3].
- **FEIG:** sarja 38400, pariteetti E; kehys tarkistetaan CRC-16:lla (0x8408). Malli on tuntematon
  (A9).
- **Impinj:** CSV-rivit, jotka tulevat ulkoisesta sillasta; ohjelma ei puhu Impinjin protokollaa
  (A13, [tuntematon]).
- **Asetukset `SIRITREUNA`, `SIRITARRIVE`, `SIRITDEPART`:** koskevat Siritin tapahtumareunaa
  (saapuminen, lähtö) [päätelty avainten nimistä; AA: osaD luku 1 luettelee avaimet].

Tunnisteen aika tallennetaan jonoon luvun 04 mukaan (`tall_etulos`). Kilpailija haetaan tunnisteesta
(`bdg2kno`); RFID-lukijan antama piste määräytyy `lahdepisteet`-tiedostosta tai
`AIKALUKIJAx`-asetuksesta [AA: käyttöohje `5.2`].

Tunnetut asiat: uudelleenyhteys testaa `openConnection`in paluuarvon väärinpäin, jolloin Siritin
lukijasäie päättyy jokaisen uudelleenyhteysyrityksen jälkeen ja Zebran ensimmäisen onnistuneen
jälkeen; lukija on sen jälkeen pois käytöstä, kunnes ohjelma käynnistetään uudelleen (H-159; Q96).
RFID-lukijan avauksessa pistokevuodot (H-155).

### 1.6 Ampumasakot, valot ja muut

- **KURVINEN (ampumasakko).** Sarjaportilla tulevat sakkoviestit (`tall_ampsakko`); käännetty vain
  HKM:ään ja HKW:hen (`SAKKOCOM`). Viestin sakkojen käsittely tapahtuu `5.9`-toiminnolla käsin
  [AA: osaD luku 3; viestiwin `5.9_sakkojen_kasittely.md`].
- **VALOT.** Signaalivalojen ohjaus sarjaportin RTS/DTR-linjoilla: `asetavalo`
  (`JukConsole/laskemtn.cpp:248-275`); `LAJUNEN`-valitsimen alla [todennettu: osaD luku 1; tarkoitus
  päätelty].
- **Emit-lukijan liikennevalot.** Asetus `VALONÄYTTÖ=yy` ottaa käyttöön lukijalle (`LUKIJA=n`) valot,
  joiden kesto on `yy` sekunnin kymmenyksinä [AA: käyttöohje `hkliite1.md:195`; laitteen vastaanotto
  ei ole luettu].
- **Kuolleet laiteosat.** SPORTIDENT, SIIMPORT, TUOMARI, ALGETUTKA (Tutka), SW2000, ASC3, RESERV_1
  ovat kuolleita; RTNM ja CPOWER ovat lähetysrutiineja, ja niiden lukijat ovat tyhjiä [todennettu:
  osaD yhteenveto; `valitsimet.csv`]. Näitä ei esitetä toimitettujen ohjelmien ominaisuuksina.

### 1.7 Näyttötaulut

Taulujen ohjaus on pääosin yksisuuntaista: ohjelma kirjoittaa, taulu ei vastaa. Virheitä ei
raportoida kutsujalle [todennettu: osaD luku 2; HkIV.cpp:3205].

| Taulu | Yhteys | Asetukset | Ohjelmat |
|---|---|---|---|
| **GAZ** (tulostaulu) | sarja 2400 8N1 (pakotettu), 23 merkkiä + CR rivillä | `GAZ=x`, `GAZVAIHE=x`, `GAZRIVIy=` | `GAZ`: HKM, VM, VW (HKW:ssä vain `GAZCOM`); `GAZ=<yksi numero>` (COM1-COM8), `gaz_com > 8` palaa (osaD luku 6) |
| **ALGE-taulu** (`ALGETAULU`) | sarja (`TAULU_COM`, `TAULU_BAUD`) tai UDP 51702 | `TAULU_COM=`, `TAULU_BAUD=`, `TAULUVIIVE=` | VM, VW (valitsin `ALGETAULU`) |
| **C-Power** | sarja 115200 tai TCP 5200; 16-bittinen summa; sarjakäärö 0xA5..0xAE | `CPOWERn=TCP:..` | lähetys vain GUI:ssa (`cbHk/UnitTaulu.cpp:166-171`, `ViestiWin/UnitGAZ.cpp:188`) [osaD luku 1] |
| **RTNM** (UDP-taulu) | UDP, `srvport = destport + 10` | `RTNMn=UDP..` | GUI |
| **Käyttäjän näyttö** | tulostaulumainen seurantanäyttö (`Seuranta / Tilanne`): automaattinen skrollaus, väriskeema | käyttöohje `8.5` | HKW (GUI) |

Tunnetut rajoitukset: GAZ-taulun COM-numero on yksinumeroinen asetuksessa (`GAZ=<yksi numero>`);
viestin konsolin taulun `taulu_com` 9...64 avataan COM-porttina mutta kirjoitetaan ja suljetaan
tiedostona, joten näyttö ei saa mitään (osaD N-14) [todennettu: osaD luku 6, N-14]. Tämä koskee
viestin konsolia (VM).

### 1.8 Tulostimet

Ohjelma käyttää Windowsin tulostinajureita; mikä tahansa koneella oleva tulostin ml.
verkkotulostimet kelpaa [AA: käyttöohje `hkkisawin/luku_9.1_kirjoittimet.md`]. Toiminta
koodissa [luettu käsikirjaa varten, ei tarkastettu: `include/tputil.h:170-226`
(`PRFILE`), `tputilv2/Wrttext.cpp:147-260` (`openprfile`)]:

- **Kohteen nimi.** Tulostuskohteen nimi on joko tulostimen nimi, `PRN` (Windowsin
  oletustulostin, `GetDefaultPrinterW`), `NUL` (ei tulostusta) tai tiedoston nimi. Asetus
  `LISTA`, `LISTA=OLETUS` tai `LISTA=PRN` valitsee oletustulostimen; `LISTA=nimi` valitsee muun
  tulostimen tai tiedoston (`Hk/HkInit.cpp:943-953`). GUI:ssa tulostimen voi valita valikosta
  *Kirjoitintulosteet / Valitse kirjoitin* (käyttöohje `6.8`).
- **Tulostustapa.** `GDIkirjoitin` (tulostusfunktion argumentti) valitsee tavan: `-1` = tekstitiedosto,
  `0` = tulostin raakadatana (`OpenPrinterW`), muu = GDI-tulostin (`CreateDCW`, sivumitat
  `GetDeviceCaps`, oletusmarginaalit ja kirjasimet). Tulostintyypit (`Hk/HkMuotoilu.h:25-33`:
  EPSON, LJETII, POSTSCRIPT, PROPRIEMU, TXTTAB, CANON, GDIPRINTER = 7) valitsevat ohjauskoodit.
- **Epäonnistunut avaus.** Jos oletustulostin on määritelty mutta sen avaus epäonnistuu,
  ilmoitus on "Oletuskirjoittimen käyttöönotto ei onnistu" ja kohde muuttuu `NUL`:ksi (ei
  tulosteita). Jos oletustulostinta ei ole määritelty, ilmoitus on "Oletuskirjoitinta ei ole
  määritelty" ja ohjelma yrittää sen jälkeen avata tiedostoa nimeltä `PRN`. Jos nimetty tulostin ei
  aukea, ohjelma yrittää avata samannimisen **tiedoston** ja kirjoittaa tulosteen siihen
  (`prno = -1`, `_wopen(prtfname, ...)`) [luettu käsikirjaa varten, ei tarkastettu; seuraus:
  väärin kirjoitetun tulostinnimen tuloste päätyy tiedostoon virheilmoituksen sijaan; `PRN`-tiedoston
  käytös Windowsissa on tuntematon].
- **Liitäntöjä enintään 16** (`PRLKM`); kun kaikki ovat käytössä, ilmoitus "Kaikki kirjoitinliitännät
  käytössä. Tulostus ei onnistu".
- **Rajaus.** Tämä luku käsittelee vain tulostimen avausta ja valintaa. Automaattitulosteet ja
  tulostustiedostot sekä tulostusmuotoilu kuuluvat tulostuksen omaan lukuun.

Tulostusmuotoilu (sarakkeet, HTML, pohjat) ei kuulu tähän lukuun; tulostusmuotoilun toimintaa ei ole
luettu.

### 1.9 Laitteen valvonta ja palautuminen

Ohjelma ei valvo laitteen elossaoloa kattavasti (T-048) [todennettu: riippuvuudet.md luku 13,
osaD N-2, N-3]:

| Tilanne | Mitä tapahtuu | Lähde |
|---|---|---|
| Sarjaportti katoaa (USB-sovitin irti) | `quesize`-virhe tyhjentää puskurin; lukijaa ei avata uudelleen eikä siitä ilmoiteta; konsolissa ei ilmaisinta, GUI:ssa edistymispalkki pysähtyy. Emit-kortit voi lukea uudelleen, kun ohjelma on käynnistetty. Muiden sarjalaitteiden ajat menetetään (H-164) | osaD N-2; H-164 |
| TCP-lukija ei vastaa, mutta yhteys on auki | ei havaintoa, ellei `/P` (LUKIJA) tai `SIRITPOLL` | osaD N-3; T-048 |
| TCP-lukijan avaus epäonnistuu | uusi yritys 5 s, 55 s, 180 s; laskuri on yhteinen kaikille lukijoille (H-138); virhepolun pistokevuoto (H-155) | osaD; H-138, H-155 |
| Sirit tai Zebra katkaisee yhteyden | lukijasäie päättyy uudelleenyhteysyrityksen jälkeen (H-159, Q96) | osaD N-1 |
| Lukijaan ei saada kehystä | kehys virheellinen: ohjelma hylkää ja pyytää uudelleen vain Emit-kellolla | osaD luku 2 |
| Useita Sirit-lukijoita | viestissä Siritin portit asetetaan vain lukijalle 0, HK:ssa jokaiselle `SIRITn`:lle (osaD luku 3); osaD:n N-9 on kumottu HK:n osalta | osaD luku 3; riippuvuudet.md luku 16 |
| ARES-lukija, 41+ tavua ilman kehystä ja ilman NUL-tavua | lukijasäie pyörii 100 % CPU:lla ja ajanotto pysähtyy | H-056 |
| `sallisummavirhe` päällä GUI:ssa | tarkistussummat poistetaan käytöstä kaikilta lukijoilta (N-13) | osaD N-13 |

Käyttöohjeen suositus: Timy kannattaa liittää sarjaporttimuuntimella, ja asennus tehdään valmiiksi
useampaan USB-porttiin; vikatilanteessa vaihdetaan portti ja käynnistetään ohjelma uudelleen
[AA: käyttöohje `liite_7`].

### 1.10 Laiteyhteys ilman laitetta

`EMITFILE`-valitsimella (HKM, HKW, VM) kortinluku voidaan syöttää tiedostosta `EMIT_IN.DAT`
(portti > 900; käyttöohjeen mukaan `MTRx=T`); se on testikäyttöön tarkoitettu polku (luku 05).
Lähtöaikojen testilähetys `TESTIAJAT` (Ctrl-T) on VM:ssä [todennettu: `valitsimet.csv`,
käyttöohje `hkliite1.md:171`]. Kellon emulaattori voi syöttää ajat TCP:llä Timyn
viestimuodossa [AA: käyttöohje `5.1.2`].

## 2. Erot ohjelmien välillä

Taulukko osaD luvusta 3 ja `valitsimet.csv`:stä; projektien määritykset tarkistettu uudelleen
(`VS/Hk/HkMaali/HkMaali.vcxproj`, `VS/V/JukMaali520.vcxproj`, `VS/V/JukLuenta520.vcxproj`,
`RADStudio10/HkKisaWin.cbproj`, `RADStudio10/ViestiWin.cbproj`):

| Asia | HKM | VM | VL | HKW | VW |
|---|---|---|---|---|---|
| Tyyppi | konsoli | konsoli | konsoli (supistettu) | GUI | GUI |
| `NREGNLY` (lukijoiden enimmäismäärä) | 32 | 8 | 8 | 32 | 32 |
| Emit-lukijat (`LUKIJA`, `MTR`, `EMITKELLO`) | kyllä | kyllä | kyllä | kyllä | kyllä |
| `EMITAG` makro | ei | kyllä | ei | ei | kyllä |
| emiTag HTTP (`EMITHTTP`) | kyllä | kyllä | ei | kyllä | kyllä |
| RFID (`LAJUNEN`: Sirit, Zebra, FEIG, Impinj) | kyllä | kyllä | **ei** (avaimet hyväksytään hiljaa ja valitsevat ALGE-ajanoton: H-176, T-052) | kyllä | kyllä |
| `TCPLUKIJA` | kyllä | kyllä | kyllä | kyllä | kyllä |
| Maalikellot (ALGE, Timy, Comet, RTR) | kyllä | kyllä | ei | kyllä | kyllä |
| ARES, ASC, TDC, RADIOEMIT | kyllä | ei | ei | kyllä | ei |
| `AIKA_COM` (CTS-pulssi) | kyllä | kyllä | ei (`NOCOM_AIKA`) | kyllä | kyllä |
| `VALUKIJA`, `AUTOVA` | kyllä | kyllä | ei | kyllä | kyllä |
| `GAZ` | kyllä | kyllä | ei | ei (vain `GAZCOM`) | kyllä |
| `ALGETAULU` | ei | kyllä | ei | ei | kyllä |
| `SAKKOCOM` | kyllä | ei | ei | kyllä | ei |
| `C-Power` lähetys | ei | ei | ei | GUI | GUI |
| MTR- ja emiTag-ohjauslomake | konsolivalikot (`aseta_mtr`, `aseta_emitag`) | konsolivalikot | ei | GUI (`cbTp/UnitMTR.cpp`, `UnitEmiTag.cpp`) | GUI |

Lähteet: osaD luku 3; `valitsimet.csv` (rivit `LAJUNEN`, `EMITHTTP`, `GAZ`, `ALGETAULU`, `SAKKOCOM`,
`AUTOVA`, `VALUKIJA`, `LUENTA`, `NOCOM_AIKA`); projektit tarkistettu 2026-11-27. `NREGNLY`-erot: T-024. Kuolleet haarat jätetty pois (kohta 1.6).
`ViestiWin`in `EMITAG` määritelty `cbproj`-määrityksessä rivillä 195 mutta ei jokaisessa
kokoonpanossa (rivi 211 ei sisällä sitä) [luettu käsikirjaa varten, ei tarkastettu]; makroa ei testata
missään `#if`-ehdossa, joten kokoonpanon valinta ei vaikuta toimintaan (osaD "Summary").

## 3. Asetukset ja käännösvalitsimet

Avaimet: `rekisteri/asetusavaimet.csv`; käyttöohjeen taulukko `hkkisawin/hkliite1.md`.

| Asetus | Ohjelmat | Vaikutus | Lähde |
|---|---|---|---|
| `TIMYx=n`, `ALGE=n`, `ALGE4=n`, `COMET=n`, `ASC=n` | HK; viestissä ALGE, ALGE4, COMET, TIMY | maalikello ja portti | [AA: osaD luku 1] |
| `ARES`, `RADIOEMIT`, `TDC`, `ASC` | vain HK | kello | [AA: osaD luku 3] |
| `REGNLYx=n/...`, `EMITKELLO=n[/w/i/a]` | HK ja viesti (ei VL) | Emit-kello | [AA: osaD luku 1] |
| `LUKIJAx=n` / `TCP:ip:portti` / `UDP` / `/P` / `/U`, `MTRx=n`, `EMITAGx=n[/U/C]` | kaikki | kortinlukijat | [AA: osaD luku 1] |
| `KELLOBAUD=` | HK, viesti | maalikellon nopeus (indeksi 2...9, enintään 38400) | [AA: osaD luku 6] |
| `AIKA_COM=`, `AIKA_ESTO=`, `KELLO_ESTO=` | HK, viesti (`AIKA_COM`/`AIKA_ESTO` ei VL) | CTS-pulssi; kaksoisaikojen esto | [AA] |
| `AIKAERO`, `AIKAKORJAUSy=` | HK, viesti | kellon siirtymä | [AA: käyttöohje] |
| `REGNLYVIIVE=xx` | HK | RTR-pyyntöjen väli; koodin `static pyyntoviive = 200` peittää asetuksen, joten arvo ei vaikuta [päätelty] (N-7) | osaD N-7 |
| `SIRITn=TCP:..`, `SIRITREUNA`, `SIRITARRIVE`, `SIRITDEPART`, `SIRITEVENTPORT/CMDPORT/POLL/MASK/SYNC/LOKI` | `LAJUNEN` | Sirit | [AA: osaD luku 1] |
| `ZEBRAn=TCP:..`, `ZEBRADEPART`, `ZEBRACLEANUP`, `ZEBRAGPI` | `LAJUNEN` | Zebra | [AA: osaD luku 1] |
| `FEIG`, `IMPINJ` | `LAJUNEN` | RFID | [AA: osaD luku 1] |
| `ETGPRSy`, `ETSQL`, `ETHOST`, `ETHAKUVÄLI`, `ETDATE`, `ETTIME`, `ECAIKA` | `EMITHTTP` | emiTag-palvelin | [AA: osaD luku 1] |
| `GAZ=x`, `GAZVAIHE=x`, `GAZRIVIy=`, `TAULU_COM=`, `TAULU_BAUD=`, `TAULUVIIVE=`, `CPOWERn=`, `RTNMn=`, `TAULUOT`, `TAULUOTS=` | taulut | kohdat 1.7 | `asetusavaimet.csv` |
| `SAKKO_COM`, `SAKKO_YHT` | HK (`SAKKOCOM`) | ampumasakot | [AA: osaD luku 1] |
| `VALOT=`, `VALONÄYTTÖ=yy` | `LAJUNEN`; lukijan valo | valot | [AA] |
| `LISTA`, `LISTA=OLETUS/PRN/nimi` | HK; viesti vastaava | tulostin tai tiedosto | `Hk/HkInit.cpp:943-953` [luettu] |
| `AIKACOM=` | HKW:n editori | jäsennin ei tunne avainta; asetus ei tule voimaan (H-173) | [AA: H-173] |

Käännösvalitsimet: `LAJUNEN`, `TCPLUKIJA`, `EMITHTTP`, `EMITAG`, `GAZ`, `ALGETAULU`, `SAKKOCOM`,
`AUTOVA`, `VALUKIJA`, `LUENTA` (VL), `NOCOM_AIKA` (VL), `_CONSOLE`; taulukko kohdassa 2. Kuolleet
(ei määritelty missään ohjelmassa): `SPORTIDENT`, `SIIMPORT`, `TUTKA`, `SWIM2000` ja `MAKI`
(`#undef` `Hk/HkDef.h:33-34`), `COMETNO`, `EBA` [AA: `valitsimet.csv`; osaD luku 1].

## 4. Vaihtoehtoiset polut ja virhetilanteet

| Tilanne | Mitä tapahtuu | Lähde |
|---|---|---|
| Lukija irrotetaan käytön aikana | ks. kohta 1.9 | H-164 |
| Lukijan sarjaportti on jo toisella ohjelmalla | avaus epäonnistuu; ilmoitus; ei uudelleenyritystä | osaD luku 2 |
| Laitteen nopeus väärä | sanomat jäävät tulkitsematta; ohjelma ei ilmoita nopeusvirheestä [päätelty: kehykset hylätään] | [päätelty] |
| Useita lukijoita yhdessä HK-instanssissa | yhteinen uudelleenyrityslaskuri (H-138); lukitsematon koodi-aika-taulu (H-136); Q81 | [AA] |
| Kellon aikaero tietokoneeseen | RTR: ilmoitus ja ensimmäisen kehyksen hylkäys (`maxero`) | luku 04 |
| Vaiheen vaihto lukijan syöttäessä | `bdg_t`-tunnisteen aika voi hävitä (H-094) | [AA] |
| `MTR`-luenta toisen kerran samassa istunnossa GUI:ssa | pysähtyy, kunnes lomake suljetaan (H-098) | [AA] |
| Tulostin ei aukea | väärän nimen tuloste menee tiedostoon; oletustulostin puuttuu: ei tulostetta (kohta 1.8) | [luettu käsikirjaa varten] |
| TCP-lukijan yhteys katkeaa | HK ja viesti: säie päättyy (`recv <= 0`), jollei aikakatkaisu; uudelleenyritys 15 s `reconnect`-asetuksella | osaD luku 2 |
| Laite ei vastaa Siritin kyselyyn | "Sirit ei vastaa" vain `lue_SiritCmd`-polulla; yleinen polku pois `LAJUNENxx`-asetuksella | osaD luku 2 |
| Firmware muuttuu | kiinteät kehyskoot (19...462 tavua), `MAXNLEIMA` = 50, Siritin tekstisiirtymät, LLRP v1 | osaD luku 5 |

## 5. Tunnetut viat tässä toiminnossa

<!-- generoitu:viat alku (tietopohja.py lohkot; älä muokkaa käsin) -->
| Toiminto | Havainto | Tila | Prioriteetti | Kuvaus |
|---|---|---|---|---|
| Laitteen yhteys ja valvonta (sarjaportti, TCP, UDP, lukijasäie) | [H-164](../vikakortit/H/H-164.md) | avoin | P2 | Sarjaporttiin kytketty lukija, joka katoaa kesken ajon, jää hiljaa lukematta, eikä ohjelma avaa porttia uudelleen eikä ilmoita siitä |
| Laitteen yhteys ja valvonta (sarjaportti, TCP, UDP, lukijasäie) | [H-138](../vikakortit/H/H-138.md) | avoin | P4 | Yhden TCP-lukijan epäonnistuneet yhteysyritykset pidentävät muiden TCP-lukijoiden yhteydenavauksen odotusta jopa neljällä minuutilla |
| Laitteen yhteys ja valvonta (sarjaportti, TCP, UDP, lukijasäie) | [H-155](../vikakortit/H/H-155.md) | avoin | P4 | Tiedonsiirron pistokkeita (soketteja) jää sulkematta virhe- ja katkaisutilanteissa |
| Laitteen yhteys ja valvonta (sarjaportti, TCP, UDP, lukijasäie) | [T-048](../vikakortit/T/T-048.md) | avoin | - | Ohjelma ei valvo, onko lukija tai muu laite yhä elossa, joten katkennut yhteys voi näyttää toimivalta |
| Maalikellot ja lähtökellot (ALGE, Timy, Comet, RTR, ARES) | [H-056](../vikakortit/H/H-056.md) | avoin | P2 | Viallinen sanoma ARES-, Sirit- tai emiTag-lukijalta voi jättää lukijasäikeen pyörimään ja pysäyttää kyseisen laitteen syötteen |
| Maalikellot ja lähtökellot (ALGE, Timy, Comet, RTR, ARES) | [H-090](../vikakortit/H/H-090.md) | avoin | P2 | HK: kellosanoman käsittely jättää aikajonon lukon päälle asetuksella TUNNJONO, ja käsin kirjattavat ajat ja vaiheen vaihto estyvät |
| Maalikellot ja lähtökellot (ALGE, Timy, Comet, RTR, ARES) | [H-170](../vikakortit/H/H-170.md) | avoin | P5 | Kokoomarivi: laitteiden, HTTP:n, selaimen ja SQL-sarakkeiden pienet viat (15 erillistä kohtaa) |
| RFID-lukijat (Sirit, Zebra, FEIG, Impinj) | [H-159](../vikakortit/H/H-159.md) | avoin | P2 | Sirit- tai Zebra-RFID-lukijan uudelleenyhteys lopettaa lukijan luennan, kunnes ohjelma käynnistetään uudelleen |
| RFID-lukijat (Sirit, Zebra, FEIG, Impinj) | [H-108](../vikakortit/H/H-108.md) | avoin | P4 | Kahden Sirit-lukijan lukusäikeet voivat saada saman lukijanumeron, jolloin toinen lukija jää lukematta |
| RFID-lukijat (Sirit, Zebra, FEIG, Impinj) | [H-070](../vikakortit/H/H-070.md) | avoin | P2 | FEIG-lukijan vastauksen ilmoittama pituus voi ylittää ohjelman puskurit ja kirjoittaa muistin yli |
| emiTag-laite ja emiTag-palvelin (EMITAG, ETGPRS, ETSQL) | [H-055](../vikakortit/H/H-055.md) | avoin | P2 | emiTag-sanoman leimapaikan numero voi kirjoittaa ohjelman pinomuistin taulukon ulkopuolelle |
| emiTag-laite ja emiTag-palvelin (EMITAG, ETGPRS, ETSQL) | [H-166](../vikakortit/H/H-166.md) | avoin | P3 | emiTag-väliaikojen nouto verkosta voi liittää kaksi sanomaa yhdeksi riviksi, pysähtyä lyhyeen riviin tai jäsentää virhesivun tietona |
| emiTag-laite ja emiTag-palvelin (EMITAG, ETGPRS, ETSQL) | [H-097](../vikakortit/H/H-097.md) | avoin | P3 | HkKisaWinissa vaiheen vaihdon jälkeen jatkuva SQL-emit-haku, tulosten vieritys ja avointen haku jäävät pois käytöstä |
| emiTag-laite ja emiTag-palvelin (EMITAG, ETGPRS, ETSQL) | [H-170](../vikakortit/H/H-170.md) | avoin | P5 | Kokoomarivi: laitteiden, HTTP:n, selaimen ja SQL-sarakkeiden pienet viat (15 erillistä kohtaa) |
| MTR-laite ja GUI-ohjauslomake | [H-098](../vikakortit/H/H-098.md) | avoin | P4 | MTR-lukijan toinen tiedostoon-luenta samassa istunnossa pysähtyy, koska päivityssäie päättyy väärässä kohdassa |
| MTR-laite ja GUI-ohjauslomake | [H-170](../vikakortit/H/H-170.md) | avoin | P5 | Kokoomarivi: laitteiden, HTTP:n, selaimen ja SQL-sarakkeiden pienet viat (15 erillistä kohtaa) |
| Ampumasakkolaite (KURVINEN) ja sakkoviestit | [H-060](../vikakortit/H/H-060.md) | avoin | P2 | Ampumapaikan numero ampumasanomassa voi nollata ohjelman muistia taulukon ulkopuolelta |
| Näyttötaulut (GAZ, ALGE-taulu, C-Power, RTNM) | [H-170](../vikakortit/H/H-170.md) | avoin | P5 | Kokoomarivi: laitteiden, HTTP:n, selaimen ja SQL-sarakkeiden pienet viat (15 erillistä kohtaa) |

Generoitu tiedostosta `kasikirja/toiminnot.csv` ja rekistereistä (tila, prioriteetti); älä muokkaa käsin.
<!-- generoitu:viat loppu -->

## 6. Mihin tieto perustuu

- `doc/arkkitehtuuri/todisteet/c10/raportit/osaD-laitteet.md` (laiteluettelo, kehykset, kuittaus,
  Q86, N-1...N-16, oletukset A1-A13); `doc/arkkitehtuuri/riippuvuudet.md` luvut 13 ja 16
  (laiteperheiden taulukko, H-159...H-170, T-048); `saikeet-ja-lukot.md` luku 3 (lukijasäikeet);
  `ominaisuusvalitsimet.md`; `rekisteri/valitsimet.csv`; `rekisteri/asetusavaimet.csv`;
  `rekisteri/havainnot.md`; `rekisteri/riskiarvio.csv`.
- Käyttöohje: `docs-site/docs/hkkisawin/5.1`, `5.2`, `6.10`, `8.5`, `hkliite1.md`, `liite_7`, `liite_9`,
  `luku_9.1_kirjoittimet.md`; `viestiwin/5.9`.
- Projektit: `VS/Hk/HkMaali/HkMaali.vcxproj:154`, `VS/V/JukMaali520.vcxproj:135`,
  `VS/V/JukLuenta520.vcxproj:131`, `RADStudio10/HkKisaWin.cbproj:149`, `:180-181`,
  `RADStudio10/ViestiWin.cbproj:134`, `:194-195` (tarkistettu uudelleen tätä lukua varten).
- Koodi [luettu käsikirjaa varten, ei tarkastettu]: `Hk/HkIV.cpp:1984-2560` (kellokehykset),
  `Tp/TpLaitteet.cpp:2405-2431`, `include/tputil.h:170-226` ja `tputilv2/Wrttext.cpp:147-260`
  (tulostin), `Hk/HkMuotoilu.h:25-33`, `Hk/HkInit.cpp:943-953`.
- Toimintokytkentä: `kasikirja/toiminnot.csv`.

## 7. Mitä ei ole selvitetty

- Laitteiden todellinen käytös (kuittaus, uusintapyyntö, MTR:n ja emiTagin muisti, kellon
  kuittauksen merkitys): Q86, Q100, OP-13; vaatii laitteet tai tallennetun liikenteen.
- Impinj-siltaohjelman ja RADIOEMIT-laitteen alkuperä; FEIG-malli; Siritin ja Zebran
  laiteohjelmistoriippuvuus (osaD A1-A4, A9, A13).
- Päättyykö Sirit/Zebra-lukijasäie uudelleenyhteyden jälkeen (Q96; H-159 on päätelty koodista, ei
  ajettu).
- Toimiiko TCP-lukijan uudelleenyhteys ja valvonta käytännössä; onko `/P`-kysely
  päätepalvelimella vastattu (osaD avoin 7).
- Näyttötaulujen vastaanottajalaitteet (GAZ, ALGE-taulu, C-Power, RTNM): vain lähetys on luettu,
  vastapään käytös ei ole tiedossa.
- Tulostimet: tulostusmuotoilun sisältö ja tulostimien ajurit; tulostusjonon käytös (taustatulostus)
  ei ole selvitetty. Ei aineistoa siitä, mitä tulostinmalleja käytetään.
- Onko `VALOT` ja `VALONÄYTTÖ` käytössä jossakin kokoonpanossa (osaD: tarkoitus päätelty).
- Laitteiden `NREGNLY`-rajat käytännössä: useita lukijoita yhdessä instanssissa (Q81).
- ViestiWinin `EMITAG`-kokoonpanotilat eri `cbproj`-konfiguraatioissa: makro ei vaikuta toimintaan.
- Käyttöohjeen tuki `radiomodeemeille` ja SendECodes-ohjelmalle: ohjelma on repon ulkopuolella eikä
  sitä ole tarkasteltu.
