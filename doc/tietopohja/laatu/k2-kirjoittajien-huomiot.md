# K-2: kirjoittajien huomiot erittäin / Writers' notes per batch

Kirjoittaja-agenttien loppuraporttien olennainen sisältö: ristiriidat rekisterin tai riskiarvion
kanssa, ehdokkaat ja aineiston aukot. Ristiriitoja ei ratkaista tässä vaiheessa muuttamalla
riskiarviota; ne käsitellään luvussa "Käsittely" (K-5) tai siirretään `seuranta.md`:n
KE-taulukkoon. Ohjelmatiedot, jotka kirjoittaja tarkisti, on tuotu `riskiarvio.csv`:hen ja
`tekninen-velka.csv`:hen (`ohjelmat_peruste = tarkistettu`).

## E02 (KK-C13-3, vaiheen vaihto, 11 korttia)

- Ohjelmat: rekisterin yläraja liian leveä; kaikki HK-ohjelmiin (HkKisaWin HkMaali), paitsi H-174
  kaikki viisi (koodi HK:ssa ja viestissä).
- H-013: silmukka on `teejarj`in lopussa myös käynnistyspolulla, ei vain vaiheen vaihdossa
  [päätelty]; rekisteri kuvaa vain vaiheen vaihdon.
- H-141: kaksoiskorttien eston laskuriosa laukeaa oletusasetuksella jokaisessa vaiheen vaihdossa;
  riskiarvio sanoo saavutettavuudeksi `asetus` → normaalikäyttö `ei_tiedossa`, riskiarvio tarkistettava.
- H-102, H-128: riskiarvio `ajanoton_jatkuvuus = ei`, mutta vapautetun muistin käyttö (Q69) tai
  numerohaun epäonnistuminen voi pysäyttää tai estää kirjauksen → `ei_tiedossa`.
- H-102, H-105, H-134: riskiarvio `tietoturva = ei`, vaikka vaiheen vaihdon voi laukaista toisen
  työaseman pyyntö (vrt. KE-2) → `ei_tiedossa`.
- Ehdokas: H-134:n polulla `vaihdavaihe` pitää tallennus-, ajanotto- ja emit-lukkoja, ja
  virheilmoitus annetaan lukkojen alla (GUI:ssa modaali) – kattaako H-097 tai H-145; ei tarkistettu.

## E01 (KK-C13-1, KK-C13-2, KK-C10-4; JS-1, 11 korttia)

- Ohjelmat rajattu: H-026 viestiohjelmiin (HK:n haara ohittaa tuntemattoman luokan); H-008 ja
  H-060 HK-ohjelmiin (`tall_ampsakko` tyhjä tynkä viestissä); H-070 neljään (FEIG vaatii `LAJUNEN`,
  ei ViestiLuennassa).
- H-056: riskiarvio `tietoturva = ei`, mutta emiTag-syöte voi tulla GUI:n TCP-palvelimelta
  (H-055, `muistiturvallisuus.md` luku 10) → `ei_tiedossa`; riskiarvio tarkistettava.
- H-008: rekisterin kuvaus ja `tallenna-samanaikaisuus.md` sanovat vian laukeavan oletusasetuksella,
  riskiarvion peruste ja koodi sanovat `tall_sak` ajetaan vain kun `sak_laji = 0` (oletus 2);
  kortti seuraa riskiarviota (normaalikäyttö `ei_perusteltu`), ristiriita kortin luvussa 12.
- H-060: riskiarvio `ajanoton_jatkuvuus = ei`, nollakirjoituksen kohde tuntematon → `ei_tiedossa`.
- Ehdokas: `tall_ampsakko` case '1' (`Hk/HkSak.cpp`): `fseek`/`fwrite` samalla rajaamattomalla
  indeksillä kuin H-060, eri kirjoituskohde (paikkatiedosto) – mahdollisesti H-060:n laajennus;
  ei tarkistettu rekisteristä.

## E03 (KK-C13-4, lukkojen vapautus, 11 korttia)

- Ohjelmat kavennettu: H-012, H-089 vain HkMaali; H-024, H-087, H-110 vain ViestiWin (GUI-polut);
  H-115, H-125, T-036 vain HkKisaWin.
- H-012: koodi tarkistaa lähdekilpailijan, riskiarvio kuvaa siirron poissaolevalle kilpailijalle;
  ristiriita kortin luvussa 12.
- H-088: riskiarvion saavutettavuus `normaalikaytto`, oma huomio sanoo vaativan tulostimen
  avausvirheen (sama asteikko-ongelma kuin KE-3) → normaalikäyttö `ei_perusteltu`.
- H-089: ajanotto ja lukkiutuminen `ei_perusteltu` Q65-kokeen perusteella (vain Windows 10.0.26100);
  vanhemmissa Windowseissa voi jumittaa.
- H-115: riskiarvion `kk_muut` = KK-C13-14, mutta korjaussuunnitelman kokonaisuudessa H-115 ei ole.
- H-040, H-090: riskiarvio `tietoturva = ei` ilman perustetta → `ei_tiedossa`.
- Ehdokas: H-024:n polulla `tark_trk` kirjoittaa sakkotiedot paikalliseen kopioon ennen varhaista
  paluuta, kopiota ei tallenneta (tarkoituksellisuus tuntematon); ei tarkistettu rekisteristä.

## E04 (KK-C13-5, KK-C13-7, lukitusjärjestys ja VCL, 13 korttia)

- Ohjelmat kavennettu GUI-ohjelmiin: H-009, H-091, H-107 HkKisaWin; H-103, H-106 ViestiWin;
  H-092, H-019, H-153, H-018, T-033 molemmat GUI:t; H-101 kaikki viisi.
- H-092: riskiarvion saavutettavuus `normaalikaytto`, mutta automaattiraportti verkosta tulleesta
  emit-sanomasta vaatii asetuksen COMAUTORAP → normaalikäyttö `ei_tiedossa`.
- H-019: saavutettavuus `normaalikaytto`, mutta ilmoitus vaatii virheen ja kaksi säiettä samaan
  aikaan; ajanotto `ei`, mutta yhteinen vastauskenttä voi pidentää odotusta → `ei_tiedossa`.
- H-009, H-092, H-103, H-153, H-018: riskiarvio `tietoturva = ei` ilman perustetta, laukaisija voi
  tulla verkosta → `ei_tiedossa`.
- H-103: vaiheen 1 väite "viestin emit ↔ rastiva" oli jo kumottu; kortti sanoo sen.
- Ehdokkaat (ei tarkistettu): viestin `uusiemitanalyysi(true)` kutsutaan myös lomakkeista
  `OsuusTiedot.cpp` ja `UnitYv.cpp` (H-103:n laukaisupolkujen laajennus); `talljarj` laskee
  sijoitukset sulkemisessa `tall`-lukon alla (osa H-153:n ketjua); `SendCopyThread`in lippu
  asetetaan vasta säikeessä (check-then-set, osa H-101:tä).

## E05 (KK-C13-8, KK-C13-9, säikeiden elinkaari ja soketit, 14 korttia)

- Ohjelmat kavennettu: H-031 vain HkMaali (`_CONSOLE`); H-094 HK-ohjelmat; H-097 vain HkKisaWin
  (ViestiWinin `cbTp/UnitEmitSQL.cpp` varjostaa lipun, kun `MAXOSUUSLUKU` määritelty); H-098 GUI:t.
- H-027, H-037, H-095, H-096, H-155: riskiarvio `tietoturva = ei` ilman perustetta → `ei_tiedossa`.
- H-157 (kokoomarivi): riskiarvio `ajanoton_jatkuvuus = ei`, mutta N-D3:n mukaan HkMaalin
  taustasäie odottaa Enteriä ja pysäyttää mm. kellon ja lähetyksen → `ei_tiedossa`.
- H-038: COMFILE-kysymys "Säilytetäänkö?" tulee käynnistyksessä (`opencomfile`), ei lopetuksessa.
- Ehdokkaat (ei tarkistettu): `PaivThread` (`cbTp/UnitMTR.cpp`) asettaa `in_aseta_mtr`-merkin
  `_endthread()`in jälkeen, joten se ei toteudu (mahdollisesti H-098:aa); `TCPacceptTh`in `break`
  jättää `tcpip_CS`-lukon (rekisterissä H-028:ssa; ehto kytkeytyy H-096:n ikkunaan).

## E06 (KK-C13-6, KK-C13-10, synkronointi ja vastaanotetun tietueen soveltaminen, 11 korttia)

- Ohjelmat kavennettu: H-010, H-011, H-007, H-022, H-023 HK-ohjelmat (viesti lukitsee tai tarkistaa);
  H-045, H-051 vain HkKisaWin; H-047 kaikki paitsi HkMaali; H-137 ei ViestiLuentaa.
- H-022, H-023: riskiarvio `tietoturva = ei` ilman perustetta; vaihetieto ja tulos kohdistetaan
  tarkistamatta, eikä protokollassa ole todennusta (H-029) → `ei_tiedossa`.
- H-136: riskiarvio `ajanoton_jatkuvuus = ei`, mutta väärä aika siirtää maali- ja väliaikoja; raja
  ajanoton ja tulosten välillä tulkinnanvarainen (rivi noudattaa riskiarviota).
- H-045, H-047, H-051: saavutettavuus `operaattori` tulkittu tavalliseksi käyttäjän toimeksi
  (normaalikäyttö `mahdollinen`, poikkeustilanne `ei_perusteltu`) – tulkinta tarkistettava K-5:ssä.
- H-051: riskiarvio `tulosten_oikeellisuus = epasuora`, kuvattu seuraus on tiedoston otsakkeen
  sekoittuminen → `ei_tiedossa`.
- H-144 (kokoomarivi): kaikki kolme kohtaa yhden analyysin löydöksiä; riskiarvio `ajanotto = ei`,
  mutta N-D6:ssa odottava säie voi jäädä silmukkaan → `ei_tiedossa`.

## E07 (KK-C13-23, KK-C13-24, replikointi ja lähetysjono, 10 korttia)

- Ohjelmat kavennettu: H-021 vain HK (muistijono valitaan vain HK:ssa); H-025 vain viestiohjelmat;
  H-048 vain konsoliohjelmat (`SERVER`-valitsin); H-016, H-039 HK (viestin polku avoin).
- H-016, H-034: saavutettavuus `normaalikaytto`, mutta laukaisu vaatii yhteyskatkon tai
  uudelleenkäynnistyksen → `ei_tiedossa`.
- H-030: `lahetaXML_TCP` muodostaa sanoman kilpailijan nykytilasta (`GETREC`), joten rekisterin
  "vanhat tilannekuvat automaattisesti XML-TCP:lle" ei pidä sellaisenaan [luettu koodista,
  ei tarkastettu]; kortin luku 12.
- H-048: työkalu lähettää kilpailijan nykytilan, ei väärää tallennettua pakettia; seuraus väärä tai
  puuttuva uudelleenlähetys [luettu koodista].
- H-025: muistijonon ylivuoto ei ole saavutettavissa viestissä (levyjono aina käytössä,
  `testaus.md` 13.4); kaatuminen `ei_perusteltu`, tietojen säilyminen `vahvistettu` (testikehys).
- T-019: rekisterin "ei tietuepituutta" ei päde `COMFILE.DAT`:iin (otsakerivillä paikan pituus).
- Kirjoittajan johdettuja suosituksia ([Perusteltu, testaamaton suositus]) H-021, H-030, H-039,
  H-048, H-050 – tarkistettava K-5:ssä.
- Ehdokkaat (ei tarkistettu): H-035:n väliaikaispuskurin kopio voi ylittää alkion rajan 6 tavulla;
  H-048:ssa väärästä kohdasta luettu kilpailijaviite menee `GETREC`iin tarkistamatta; H-050:n
  katselin ei palauta `combufsize`a.

## E08 (KK-C13-11, KK-C13-13, kiinteät puskurit, 13 korttia)

- Ohjelmat kavennettu: H-046 vain ViestiWin; H-064 vain ViestiMaali; H-068 vain HkMaali; H-073
  neljä (ViestiLuenta avoin); H-075 ViestiMaali ja ViestiLuenta; H-077 GUI:t; H-067 konsolit.
- Kääntäjäero kortin luvussa 11 (b): MSVC-konsolit tarkistavat pinon suoja-arvon, bcc32-GUI:ssa
  ylikirjoitus voi olla hiljainen [päätelty]; ei mitattu näille vioille.
- H-062, H-067, H-068, H-077: riskiarvio `ajanoton_jatkuvuus = ei` (H-062 myös tietoturva), mutta
  kirjoituskohde tuntematon tai alustamaton → `ei_tiedossa`.
- H-064: tietoturva `ei_tiedossa` (laukaiseva luenta voi tulla verkosta [päätelty]); riskiarvion
  palautettavuus `uudelleenkaynnistys` vs deterministinen toistuminen samalla asetuksella.
- H-003: rekisterin "kaikki 16 kohtaa löytyivät uudelleen" on huomautus, ei riippumaton tarkastus.
- Ehdokkaat (ei tarkistettu): `Juk/VEmit.cpp` `kirjemvaloki`: `ansitowcs(st, rata, 39)` kenttään
  `char rata[8]`; `Juk/VIx.cpp` `sprintf(prs, ...)` puskurin koko tarkistamatta.

## E09 (KK-C13-14, GUI: dataa korruptoivat toiminnot, 10 korttia)

- Ohjelmat kavennettu: H-111 GUI:t (ajanoton paneeli); H-112, H-121 vain ViestiWin; H-114, H-117
  GUI:t; T-035 vain viestiohjelmat (HK:n `kilptietue` alustaa itsensä).
- Saavutettavuus `operaattori` tulkittu: tavallinen toimenpide → normaalikäyttö `mahdollinen`;
  virheellinen syöte (H-114, H-117) → normaalikäyttö `ei_perusteltu`, poikkeustilanne `mahdollinen`.
- Laukaisuehdon suorat käänteiset ohjeet ("älä paina Tallenna muutos L-rivillä") merkitty
  [Perusteltu, testaamaton suositus] – tarkistettava K-5:ssä.
- H-112: C-7-raportin N-2 ja sen luvun 8 korjaus ristiriidassa `getpos`in paluuarvosta; kortti
  noudattaa koodia (`Juk/vdat.cpp`: -1 vain alueen ulkopuolisille, 0 käyttämättömille).
- H-113: myös `BtnTallennaClick` suojaamaton, kun joukkueen numero on vaihtunut (raportin luvun 8
  korjaus kumoaa N-6:n).
- Ehdokkaat (haettu havainnot.md:stä ja KE-taulukosta, ei vastaavaa; ei tarkistettu):
  HK:n `BtnTall2Click` (`cbHk/UnitAjanotto.cpp`) tallentaa pisteen `(char)_wtoi(...)`, joten 128+
  kiertyy negatiiviseksi ja 256 muuttuu maaliksi [päätelty]; ViestiWinin `Button2Click`
  (`ViestiWin/UnitAjanotto.cpp`) kuvaa A:n arvoon -1, vaikka näytön taulukossa A = -2, eikä L:lle ole
  haaraa (raportin N-1:ssä, ei H-111:n rivillä).

## E10 (KK-C13-15, GUI: syöterajat ja pienet logiikkaviat, 14 korttia)

- Ei yhtään vahvistettu-arvoa (kaikki luettu koodista tai päätelty). H-122, H-123, H-124, H-126, H-127
  ilman riippumatonta tarkastusta (J7-5).
- Kokoomarivit purettu: H-126 viisi kohtaa (riskiarvio ei nostanut yhtään), H-127 seitsemän (N-22
  nostettu).
- H-126, H-127: riskiarvio `ajanoton_jatkuvuus = ei`, mutta N-24 kuluttaa muistionumeron kirjaamatta
  tai kirjoittaa ajan ilman vahvistusta → `ei_tiedossa`.
- Ohjelmat kavennettu: H-120 vain HkKisaWin (`!_CONSOLE`-haara); H-175 GUI:t (valikko vain GUI:ssa).

## E12 (KK-C13-16, -20, -21, -22, johdetut rakenteet ja arkkitehtuuri, 11 korttia)

- H-066: riskiarvio `tulosten_oikeellisuus = suora`, aineisto osoittaa vain kaksoiskappaleen
  seuraluetteloon → `ei_tiedossa`; viestin `TallennaSeura`lle ei löytynyt kutsujaa (vain HK).
- T-039: rekisterin rajaus HK liian kapea (nimeää H-130:n ja H-140:n) → kaikki viisi.
- T-001: GUI-otsikot vain ilman `_CONSOLE`ia → GUI:t; T-002: juurisyy JS-10 ei sovi
  kehäriippuvuuksiin (juurisyyraportti listaa JS-10:lle vain T-003 ja T-033).
- H-142: rekisterin kuvaus ja `globaali-tila.md`:n taulukko eroavat N-C7:n sijoituksesta; kortti
  noudattaa rekisteriä. H-135: riskiarvio palautettavuus `kasin`, raportti uudelleenkäynnistys.
- Kiertotapa `mahdollinen` (uudelleenkäynnistys perusteltu, testaamaton) H-130, H-135, H-139,
  H-140, H-142.
- Ehdokkaat (ei tarkistettu): `nvak` (H-139) käytössä myös `cbHk/UnitSarjatiedot.cpp`:n
  lähtöaikojen ja numeroalueiden laskennassa; arvonnan `arvonumerot`: mahdollinen yhden alkion
  virhe (`nn = nilm_pv - nvak - 1`); `nyilmt > 0` -ehto `cbHk/UnitKilpMaaritys.cpp`:ssä.
- Sanasto: "Johdettu rakenne" puuttuu, vaikka kortit viittaavat siihen.

## E11 (KK-C13-12, KK-C13-19, paikalliset tiedostot ja tuonti, 11 korttia)

- Kortit kertovat laukaisevan tiedoston ja sen alkuperän (vanha ohjelmaversio, käsin tehty,
  editorilla UTF-16:ksi tallennettu).
- H-079: rekisterin konsolipolku (`HkConsole/HkCons3.cpp`) on lohkossa `SEURATUL`, joka ei käänny
  mihinkään tuotteeseen (H-177); vain GUI-lomakkeet saavutettavia. Uudelleenkäynnistys ei poista
  syytä, jos tiedosto pysyy UTF-16:na (riskiarvio palautettavuus `uudelleenkaynnistys`).
- H-086: loki tyhjenee vain ohjelman ajon ensimmäisellä avauksella (`if (!KierrFile)`), ei joka
  kerta kuten rekisteri sanoo.
- H-074: rekisterin ehto `valuku ≤ 2` pätee vain HK:n lähdejonotiedostoon; lähdepistetiedostossa
  ero syntyy pistenumerosta `valuku`+2 millä tahansa `valuku`-arvolla.
- H-065: riskiarvio `ajanoton_jatkuvuus = ei`, roska-indeksi voisi kaataa käynnistyksessä →
  `ei_tiedossa`.
- Ohjelmat kavennettu: H-049, H-079, H-086, T-026 GUI:t; H-058, H-074 ei ViestiLuentaa; H-078 vain
  MSVC-konsolit.
- Ehdokas (havainnot.md:stä ei vastaavaa): `Juk/vSarjat.cpp` `luesarjat`: `strcpy(Sarjat[i].sarjanimi, s)`
  kopioi enintään 79 merkkiä kenttään `sarjanimi[LSARJA+1]` (viestissä 11) rajaamatta; syöte vanhan
  muodon `KILP.SRJ`.

## E13 (KK-C10-1, KK-C9-1, SQL ja PRFILE, 13 korttia)

- SQL-korteissa yhtenäinen kuvaus peilistä: valinnainen, vain GUI-ohjelmat, ei oletuksena,
  asetukset `SQLHOST` ym., kirjastot eivät julkaisu-ZIPissä (H-165).
- H-014: riskiarvio `ajanoton_jatkuvuus = hairitsee` ilman kuvattua mekanismia → `ei_tiedossa`;
  H-014, H-160, H-161: tietoturva `ei` mutta laukaisija avoin (KE-2, Q105) → `ei_tiedossa`.
- H-161: ajanotto `ei`, mutta virheellinen lause johtaa H-160:n mukaan ohjelman päättymiseen →
  `ei_tiedossa`. H-163: tulokset `ei`, mutta emit-jälkikäsittelysäie jää käynnistämättä → `ei_tiedossa`.
- H-162: rekisterin "emit-käsittely pysähtyy" ristiriidassa koodin kanssa (KE-13).
- H-154: saavutettavuus `normaalikaytto`, mutta vaatii tulostinvirheen → `ei_tiedossa`.
- Ohjelmat kavennettu: H-014 vain HkKisaWin; H-100, H-161, H-043 GUI:t; H-149 viestiohjelmat;
  H-150, H-151 HK; H-152 HkKisaWin.
- Ehdokkaat (ei tarkistettu): viennin valinta 3 pitää `tall`-lukkoa koko viennin ajan (T-032?);
  `kirjSQLTh` käyttää VCL-komponentteja työsäikeestä; yhteyden avaus ilman aikakatkaisua
  pääsäikeessä (osan B ehdokas D8).

## E14 (KK-C9-2, KK-C9-3, tallennusvirheiden välitys, 12 korttia)

- Systemaattinen ristiriita: H-015, H-044, H-053, H-054, H-146, H-148, H-156 – riskiarvion
  saavutettavuus `normaalikaytto`, vaikka vika vaatii ympäristövirheen (levy täynnä, verkkolevyn
  katko, I/O-virhe) → normaalikäyttö `ei_tiedossa`; sama asteikko-ongelma kuin KE-3.
- Ohjelmat kavennettu: H-063, H-054, H-146, H-053 ei ViestiLuentaa (`#ifndef LUENTA` tai tiedosto
  ei projektissa); H-148 vain viestin `addfrec` (ViestiWin, ViestiMaali).
- H-044: riskiarvion peruste sanoo "ei jäsen" KK-C9-2:sta, mutta `korjauskokonaisuudet.csv` luettelee
  sen jäseneksi; kortti seuraa jäsenyyttä.
- H-156: ajanotto `ei`, mutta väärä vaihe käynnistyksessä voi vaikuttaa → `ei_tiedossa`; kohta N-B9
  ansaitsisi riskiarvion mukaan oman rivin (ei tehty).
- H-147: Q92 avoin `virheenkasittely.md`:ssä, ratkaistu staattisesti `seuranta.md`:ssä; kortti
  seuraa `seuranta.md`:tä.
- Ehdokas (ei tarkistettu): H-054:n väärä `_close(fl)` – MSVC:n virheellisen parametrin käsittely
  voisi päättää ohjelman.

## E17 (KK-C12-3, valitsinhygienia ja kuollut koodi, 15 korttia)

- Ohjelmat kavennettu: T-028 vain HkMaali (`#ifdef CONSOLE` vs `_CONSOLE`); H-177 vain HkMaali;
  T-015 viestiohjelmat; T-007, T-011 ei yhtään (yksikään projekti ei käännä tiedostoja).
- T-008: rekisterin juurisyy JS-12, saman kokonaisuuden muilla JS-14 (kirjoitettu sellaisenaan).
- T-007: `dbboxm/Dbbox.cpp` inventaarion orpolistalla, mutta T-018 viittaa siihen kuolleen
  B-puukerroksen lähteenä.
- T-029: `swprintf`-kutsu suoritetaan, kun kilpailijalla on tulos, joten kyse ei ole puhtaasti
  kuolleesta koodista; objektivertailun hyväksymiskriteeri ei sellaisenaan päde.
- Ehdokas (ei tarkistettu): `#ifdef CONSOLE` -sekaannus voi olla muissakin tiedostoissa.

## E15 (KK-C10-2, KK-C10-3, laitteet ja jakelu, 15 korttia)

- Mitään ei ajettu (C-10 staattinen); seuraukset enintään `mahdollinen`.
- H-167: riskiarvio `normaalikaytto`, mutta jumi vaatii emiTag-noudon asetuksen ja epäonnistuneen
  noudon sulkemishetkellä → `ei_perusteltu`; ohjelmat GUI:t.
- H-166, H-170: riskiarvio `tietoturva = ei`; vastausta ei todenneta (T-043) / `sallisummavirhe`
  ohittaa tarkistussumman → `ei_tiedossa`.
- H-108: ohjelmat HK (viestin asetukset eivät salli toista Sirit-lukijaa, päätelty).
- H-138: riippumaton tarkastus vahvisti osittain; laukaisu alkuperäistä kapeampi.

## E16 (KK-C12-1, KK-C12-2, julkaisu-XML ja asetukset, 10 korttia)

- H-036: rekisterin ja osaD:n raja "merkit `r` ja suuremmat" → laskennan mukaan `q` (113 − 49 = 64);
  käytännössä ääkköset ja `{`–`~` (avain muutetaan isoiksi kirjaimiksi).
- H-172: `xmlots`ia kutsuu vain `list` (`#ifndef _CONSOLE`), viestin `autofile`ssa kutsu
  kommentoitu → konsoleissa tiedosto käytännössä tyhjä [päätelty]; ohjelmat ViestiMaali ja
  ViestiLuenta (ei ViestiWin).
- H-158: konsoleissa tiedosto tyhjä; `AUTOFILE.LST`:n kanssa XML-haara ei avaa tiedostoa; haara
  palaa ennen TCP-lähetystä ja `KOMENTO=`-ajoa.
- H-173: riskiarvio tulokset `ei`, mutta ViestiWinin `AIKAERO1=`/`AIKAERO2=` jäävät lukijalta
  huomiotta, ja aikaero vaikuttaa aikaleimoihin → `ei_tiedossa`.
- H-052, T-051: ilmoituspuskurin raja laskettuna 47 merkkiä (rekisteri noin 45); konsolin `CFG=`
  katkaistaan 40 merkkiin → koskee käytännössä GUI:ta [päätelty].
- H-176 kohta 6: viestin `TARKNO=EI` → 21, ei kytke tarkistusta pois [päätelty]; rekisteri mainitsee
  vain `ON`. Viestin `htmlloppu` vastaava (myös ViestiLuenta).
- T-014: koodin oletusportti 14770, käyttöohjeessa 14900.

## E18 (KK-C13-17, KK-C13-18, merkistöt, kääntäjät, testaus; T-010, 12 korttia)

- H-080: riskiarvio tulokset `suora`, mutta päivämäärän alkuperäiset käyttäjät kumottiin ja käyttö on
  päätelty → `mahdollinen` (aikajonon järjestysavain); riskiarvio tarkistettava. Korjaussuunnitelma ei
  kuvaa H-080:n korjausperiaatetta.
- H-081: rekisteri liittää KK-C13-17:ään, vaikka kyse on konsoli- ja ikkunahaarojen erosta (Q60, OP-9).
- Ohjelmat kavennettu: H-080, T-016, T-023 GUI:t (bcc32); T-025 HK.
- T-010: korjattu commitissa `c273b9f` (2026-11-24); kortti kertoo korjatut kohdat.
- Ehdokas (ei tarkistettu): H-004:n vahvistuskehotteen korvausmerkkiteksti käännetty HkMaaliin mutta
  ei HkKisaWiniin (`kaantajaerot.md` taulukko).
