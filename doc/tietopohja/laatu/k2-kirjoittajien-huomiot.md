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
