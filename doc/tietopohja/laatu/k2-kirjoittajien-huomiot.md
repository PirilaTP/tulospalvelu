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
