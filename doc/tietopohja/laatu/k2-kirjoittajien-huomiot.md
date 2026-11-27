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
