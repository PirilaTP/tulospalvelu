# K-5: pirila-expert v5 -vastausten riippumaton arvio (Q4, Q5, Q7, Q10-Q12, Q20, Q28, Q33-Q39)

Arvioija: independent-verifier (Opus), 2026-11-27, vain luku. Kohde:
`doc/tietopohja/laatu/tarkastukset/k1-vastaukset-v5/q04.md`-`q39.md`. Vertailu: `k1-vastaukset-v3/`,
`k1-arvio.md` luku 6, `emit/perusvastaus.md`. Koodi luettu HEAD:sta (`TPsource/` ennallaan
revisiosta `f7fe553`, `CHECKPOINT.md:12`). Rekisterit luettu CSV-jasentimella sarakkeittain
(`prioriteetti`, `prioriteetti_aiempi`, `kk`, `kk_muut`, `tila`, `juurisyy`), jotta pitkien
rivien sarakesekaannus ei vaikuta arvioon.

Tarkistettu noin 95 viitetta: noin 45 koodiviitetta (path:line ja sisalto), 25 rekisterikenttaa
(prioriteetti/tila/kk/juurisyy), 15 vaikutusarvoa (`vaikutukset.csv`), 10 OP/Q/KE-viitetta.

## 1. Tulos

| | Hyvaksytty | Huomautuksin | Hylatty |
|---|--:|--:|--:|
| v5 (15 kysymysta) | 8: Q4, Q11, Q12, Q20, Q35, Q36, Q37, Q38 | 7: Q5, Q7, Q10, Q28, Q33, Q34, Q39 | 0 |
| v3 (14 kysymysta) | 7 | 7 | 0 |

Yksikaan vastaus ei tayta hylkaysehtoa. Kaikkien johtopaatos on oikein.

## 2. Kysymyskohtaiset arviot

### Q4 - HYVAKSYTTY
- Ydin taynna: kaksi vaihetta, lukijasaie -> `tall_emit` -> `addem`, kuittaus, kaksoiskappaleet,
  kasittely `pros_emit`, valitys; kertoo mita ei luettu.
- Tarkistettu oikein: `Hk/HkIV.cpp:1145-1152` (samatleimat/addem, return 0), `Hk/HkEmit.cpp:1657-1693`
  (kaannetty addem, `#if defined(EMIT)` alkaen :1549; TALLEMIT-versio :1275-1377 sisaltaa :1333),
  `Tp/TpLaitteet.cpp:910-917`, `Hk/HkIV.cpp:1820-1822`, `:1845-1852`, `Hk/HkEmit.cpp:3779/3867`
  (leimat -> pros_emit), `HkConsole/HkMain.cpp:152-158`, `Juk/VIv.cpp:1107-1108`.
- (1) Prioriteetit/tilat: kaikki 11 oikein (H-145 P3 KK-C9-2/KK-C10-2, H-168 P4, H-031 P2, H-163 P3 jne.).
- (2) Haut: kattavat; aineisto luettu vain riveille 1-327, kerrottu itse.
- (3) Viitteet: 1 epatarkkuus: "raakakortin kirjaus on tyhja" - se on vartija-arvo
  `TMAALI0/KSEK` (`Hk/HkIV.cpp:977`), jota addemin ehto `KSEK*em->kirjaus != TMAALI0`
  (`Hk/HkEmit.cpp:1678`) testaa. Johtopaatos (ei laheteta) oikein.
- (4) Kuollut koodi: ei. v3:n virhe (addem :1333, TALLEMIT) poistunut. `valitsimet.csv` TALLEMIT:
  luokka = dead-never-defined, tila = partly-dead; agentin merkinta vastaa luokka-saraketta.
- (5) OP-13 oikein (sen kohta 2 mainitsee H-145, H-168); Q86, Q100; KE-12, KE-13 ehdokkaina.
- (6) Tietopohja: aineisto `emit-luenta.md` ja `05-leimaukset.md` 1.5; johti oikeaan tietoon.

### Q5 - HYVAKSYTTY HUOMAUTUKSIN
- Johtopaatos oikein (valinnainen peili vain GUI:ssa, KILP.DAT ensisijainen, OP-7 avoin).
- (1) H-160 P2, H-162/163/165/169 P3, KK:t oikein; riviviitteet 161-170 oikein (rivi = id+1).
- (2) Puute: odotetun ytimen `kilptietue::write`-polkua ei selvitetty. Olisi loytynyt:
  `Hk/HkDat.cpp:581-595` (mihin == 1 -> kirjKilpSQL), PUTREC kirjoittaa vain kun
  datatallfl[i] (`Hk/HkDat.cpp:617-621`), oletus `datatallfl[2] = {1, 0}` (`Hk/HkGlobals.cpp:597`),
  GUI asettaa datatallfl[1] = sqlparam.sqltallennus (`cbHk/UnitMySQL.cpp:61`), konsolin
  kirjKilpSQL on tyhja tynka (`HkConsole/HkCons0.cpp:141-143`). Tama olisi tehnyt agentin
  [paatelty]-vaitteesta "HkMaali ei kirjoita" koodista luetun.
- (3) Tarkistetut viitteet oikein (`cbHk/UnitMySQL.cpp:696-702`, HkGlobals-oletukset, `Hk/HkInit.cpp:753`).
- (4) ei. (5) OP-7, Q5 (ratkaistu), Q97/Q98/Q105/Q106. (6) Tietopohjaa ei kaytetty.

### Q7 - HYVAKSYTTY HUOMAUTUKSIN
- Oikein: kaksi jasenninta (`Hk/HkInit.cpp:552`, `Juk/VInit.cpp:639`), silmukka :574 / :661,
  `#ifndef _CONSOLE` IKKUNAT/KUULUTTAJA `Juk/VInit.cpp:693-705`, puskurit 201/41
  (`Hk/HkInit.cpp:59`, `Juk/VInit.cpp:74`), H-171 P2, H-173 P3, KK-C12-2, OP-11 avoin.
- (2) JS-14 puuttuu edelleen (kolmas kierros): H-173:n juurisyy = JS-14 ja odotettu ydin nimeaa sen.
  Haku peruscfg/LASKENTA.CFG rajattiin jasenninfunktioon; nimi on tiedoston alussa
  (`Hk/HkInit.cpp:59`, `Juk/VInit.cpp:74`).
- (5) OP-11, Q112 oikein. (6) ei kaytetty.

### Q10 - HYVAKSYTTY HUOMAUTUKSIN
- Korjautunut: lukumaara "dokumentti 27, koodi 31" (`include/TpLaitteet.h:17-47`, 31 LID-riviä,
  tarkistettu) ja ristiriita ilmoitettu; H-164 = P2 (aiemmin P3), v4:n virhe poistunut.
- (1) H-159 P2, H-164 P2, H-145 P3 oikein.
- (3) Virheet: taulukon rivi "Emit-kello ja RTR (RTR, RTR2, EMITKELLO) ... ei ViestiLuenta" yhdistaa
  RTR:n [inferred]-rivin (osaD-laitteet.md:23) ja EMITKELLOn (osaD:31 "HK, relay"); Emit-tarkastus
  totesi ViestiLuennan kaantavan Emit-kellon jasennyksen (`Juk/VInit.cpp:1751-1767`). Lisaksi
  H-145:n suunta kaantynyt ("Emit-kello kuittaa kortin"; ohjelma kuittaa kellolle).
- (2) Laitenimia ei haettu havainnot.md:sta (kerrottu). (6) Aineistoa ei kaytetty, vaikka
  emit-luenta.md luku 7 olisi korjannut ViestiLuenta-rivin.

### Q11 - HYVAKSYTTY
- Tarkistettu: `Tp/TpLaitteet.cpp:1816-1817` ja `:1845-1846` (if !openConnection -> regnly_open = 0),
  `:1864` reconnectTCP else-haarassa, `:1872-1874` ilmoitus; openSirit2 palauttaa 0 epaonnistuessa
  (`:2238-2242`, `:2262-2263`; agentin rivit yhden rivin heitolla); Zebra palauttaa 1 epaonnistuessa
  (`Tp/ZebraReader.cpp:933-943`), joten saie paattyy ensimmaiseen onnistuneeseen uudelleenyhteyteen. Oikein.
- (1) H-159 P2 KK-C10-2, H-164 P2 (aiemmin P3) oikein. Vaikutusarvot oikein.
- (6) H-159-kortti (luvut 3 ja 10) kaytetty; korttiin perustuva "ViestiWinissa ei kasin pyydettavaa
  uudelleenyhteytta" pitaa (reconnectSirit vain `cbHk/UnitAjanotto.cpp`, `HkConsole/HkCons0.cpp`,
  `JukConsole/laskent0.cpp`, `Tp/TpLaitteet.cpp`).

### Q12 - HYVAKSYTTY
- `Tp/TpLaitteet.cpp:909-918` oikein (pyynto = "?", kuittaus = "+", maaritelty :858); putem palauttaa
  virheen 1 (`Hk/HkEmit.cpp:1613-1638`). H-145 P3, KK-C9-2/KK-C10-2, vaikutusarvot oikein; OP-13 avoin.
- Agentti jatti tall_emitin virhepolun dokumentin varaan; koodi vahvistaa: addem palauttaa
  res = 0 myos kun putem epaonnistuu (`Hk/HkEmit.cpp:1667`, `:1690`), tall_emit palauttaa 0
  (`Hk/HkIV.cpp:1152`), ja on_emit kirjaa kortin ennen kirjoitusta (`Hk/HkEmit.cpp:1664`), joten
  uusintaluku hylataan. Vastaa agentin kuvausta.
- (2) #if-ehtoja ei tarkistettu (kerrottu). (6) H-145-korttia ei kaytetty.

### Q20 - HYVAKSYTTY
- 177 H-rivia, 0 korjattua, H-099 kumottu, T-010 korjattu c273b9f; tarkistettu CSV:sta.
  Revisio CHECKPOINTista (:12). OP/Q:n jattaminen hakematta perusteltu.

### Q28 - HYVAKSYTTY HUOMAUTUKSIN
- `cbTp/cbTpApu.cpp:445-463`, `cbTp/UnitSelect3.cpp:38-60`, UnitSelect3.dfm ilman BorderIcons/OnClose;
  tarkistettu oikein. H-116 P2 (aiempi P3), KK-C13-14 (+KK-C13-5), vaikutukset oikein; [paatelty, ei ajettu].
- (5) OP-tunnistus: KK-C13-14:n OP-14 on rekisterin mukainen, mutta aiheen oma paatos on
  OP-9 kohta (6) (X:lla sulkeminen = painike 2, H-116; `omistajan-paatokset.md:212-214`, `:385`).
  Agentti loysi OP-9:n mutta jatti yhteyden tarkistamatta.

### Q33 - HYVAKSYTTY HUOMAUTUKSIN
- Laskettu koodista: 15 HK-nimea `Hk/HkGlobals.cpp:20-34`, 12 viestin `Juk/VGlobals.cpp:20-31`,
  muut 7 (etls, prof, SQL, sakkoGrid, sakkoKierr, tls, seur); kaikki rivit tarkistettu; 22 tasmaa.
- (3) Virhe: "JukMaali520 ja JukLuenta520 14. Jalkimmaisten kahden luvun mukana on lomakkeiden
  tls/seur-lukkoja". `saikeet-ja-lukot.md:261-262` liittaa tls:n HkKisaWiniin ja tls+seur:n
  ViestiWiniin, ei konsoliohjelmiin.

### Q34 - HYVAKSYTTY HUOMAUTUKSIN
- Koodi tarkistettu: lukon otto `Hk/HkIV.cpp:2673-2675`, ainoa vapautus `:3027-3029`, return-polut
  `:2770-2783`, goto loppu `:2825/2827/2832` -> label `:3073` (ohittaa vapautuksen). Agentin
  [paatelty] haelahdejono-reitti on mahdollinen: `:2738` on lukon if/else-rakenteen jalkeen.
- (1) H-090 P2, avoin, KK-C13-4 oikein, mutta "kokonaisuus KK-C13-4 (tila valiton)": valiton on
  korjauskokonaisuudet.csv:n sarakkeen tyyppi arvo, ei tila (sarakesekaannus). Lisaksi
  kiertotapa raportoitu "perusteltu", rekisterissa mahdollinen.
- (5) OP-3 (KK-C13-4:n paatos) oikein; Q15 oikein rajattu pois.

### Q35 - HYVAKSYTTY
- OP-15 avoin (`omistajan-paatokset.md:309-313`), luvut 631/275/204/148 (`merkistot.md:8`),
  H-004/H-005 P5 KK-C13-17, T-004/T-027 avoin, commitit 5a65c64/50db2f8; kaikki oikein.

### Q36 - HYVAKSYTTY
- `VS/V/JukLuenta520.vcxproj` (0 x LAJUNEN, 4 x LUENTA), `VS/V/JukMaali520.vcxproj` (4 x LAJUNEN),
  `Tp/TpLaitteet.cpp:1706-1713`, `:1765-1783`, `Juk/VInit.cpp:1544-1592`, `:1604-1629`,
  JukLuenta520.vcxproj:196-202; kaikki oikein; OP-9 (4), T-052, Q111 oikein.
- Avoin rajaus: ViestiWinin LAJUNEN jai lukematta; se on maaritelty (`RADStudio10/ViestiWin.cbproj:134`).

### Q37 - HYVAKSYTTY
- `Juk/VTulostus.cpp:2189/2191/2228` ja `:3941/3943/3981` tarkistettu; ei ymparoivaa #if-lohkoa;
  H-001 P3, KK-C13-13 (P2) ja vaikutukset oikein; tietolajit erotettu, git-vertailua ei vaiteta.
- Pieni: `omistajan-paatokset.md:303` (OP-14) jai tunnistamatta, kerrottu itse.

### Q38 - HYVAKSYTTY
- Kieltaytyi; ei avannut doc/tietopohja/laatu/-hakemistoa.

### Q39 - HYVAKSYTTY HUOMAUTUKSIN (ks. luku 3)

## 3. Q39: aineisto vs koodi, ja perusvastauksen virhe

Perusvastauksen virhe on poistunut. Perusvastaus (`emit/perusvastaus.md` kohta 4): "Se valittaa
raakatietueen muille asemille (laheta_em, :1681)". v5 Q39 kohta 4: raakakorttia ei laheteta,
koska laheta_em ajetaan vain kun `KSEK*em->kirjaus != TMAALI0` (`Hk/HkEmit.cpp:1678-1681`), ja
tall_emit asettaa `em.kirjaus = TMAALI0/KSEK` (`Hk/HkIV.cpp:977`). Tarkistettu, oikein.
Lahetys tapahtuu pros_emitin lopussa (`Hk/HkIV.cpp:1845-1852`) ja vastaanottajan addemissa
(kielto = cn+1, kirjaus asetettu).

Osuudet (15 numeroitua vaihetta seka virhetilanne- ja havaintoluettelot):
- Koodista nyt luettu ja oikeaksi todettu: vaiheet 2, 3, 4, 5, 9, 11, 14 (osittain 8). Viitteet
  `Hk/HkIV.cpp:1145-1151`, `Hk/HkEmit.cpp:1657-1693`, `Hk/HkIV.cpp:1815-1866`, `Tp/TpLaitteet.cpp:910-917`,
  `Hk/HkCom32.cpp:766-798` (myos LAHEMITn-arvot `Hk/HkInit.cpp:2711-2722` ja tark_emitin
  com_emit 1 / -1 -ehto `Hk/HkCom32.cpp:793`) pitavat kaikki.
- Aineistosta tai dokumenteista: vaiheet 1, 6, 7, 10, 12, 13, 15, ohjelmaerot ja kaikki havaintokytkennat.
- Arvio: noin 60 % sisallosta aineiston varassa, noin 40 % aineiston vaitteiden kohdennettua
  uudelleenlukua koodista. Uusia koodialueita agentti ei lukenut. Tama vastaa odotettua tapaa
  (ensisijaisesti aineisto, ei pelkka koodin luku). Hylkaysehto ei tayty.
- Ytimen kohdat: kaksi vaihetta OK; kaannetty addem ja TALLEMIT-haara kuolleena OK; kuittaus ennen
  onnistumisen varmistusta (H-145) OK; tallennus -> putem -> lahetys OK; vastaanottajan kasittely OK;
  ViestiLuenta ei tallenna emit-tietuetta (`Juk/VIv.cpp:1107-1110`, `Juk/VCom.cpp:950-952`) OK.

Huomautukset:
1. (2) Tarkistukset-rivin "haettu addem, tall_emit, pros_emit (havainnot.md) -> ei osumia" on vaara:
   havainnot.md:ssa on 7 osumariviä (H-087, H-092, H-102, H-103, H-105, H-137, H-145). Agentti
   paikkasi aineiston luvulla 10, joten tulos ei vaaristynyt, mutta haku epaonnistui.
2. Valikoiva havaintoluettelo: aineiston luku 10.1 kytkee ketjuun noin 70 havaintoa, vastaus nimeaa
   6 (+KE-10). Puuttuvat mm. vastaanottovaiheen P1-havainnot H-032 ja H-033 (KK-C13-1) seka H-092
   (emit -> print -lukitus juuri addem -> emit_rap -polulla, jonka vastaus mainitsee kohdassa 14).
   Ohjeen kohta 14 (vahemman mutta tarkistettuja) selittaa karsinnan, mutta P1:t olisi kuulunut mainita.
3. Odotettua lahdetta `kasikirja/05-leimaukset.md` ei kaytetty.
4. HkKisaWinin kasittelyehdoista puuttuu `FormEmit->Tietue >= emithead-1` (`cbHk/WinHk.cpp:1626-1629`);
   aineiston tarkastuksessa nimetty puute toistuu, pieni.

## 4. Kooste virhetyypeittain (v5)

| Tyyppi | v5-loydos |
|---|---|
| (1) Vaarat prioriteetit | 0 (kaikki noin 25 tarkistettua prioriteettia oikein; H-164 P2 nyt oikein Q10:ssa ja Q11:ssa) |
| (1) Vaarat tilat / sarakkeet | 1: Q34 tyyppi raportoitu tilana; lisaksi 1 vaikutusarvo vaarin (Q34 kiertotapa) |
| (2) Puutteelliset pakolliset haut | Q7 JS-14; Q5 kilptietue::write; Q39 havainnot.md-haku "ei osumia" (vaarin); Q28 OP-9 tarkistamatta; Q10 laitenimet; Q36 ViestiWinin LAJUNEN (kaksi viimeista agentin itse kertomia) |
| (3) Vaarat viitteet | 4 noin 95:sta (noin 4 %): Q10 EMITKELLO/ViestiLuenta, Q10 H-145:n suunta, Q33 tls/seur-ohjelmat, Q4 kirjaus "tyhja". Lisaksi yhden rivin heittoja (Q11) |
| (4) Kuollut koodi / vaara ohjelma | 0 kuollutta koodia; 1 ohjelmavirhe (Q10 ViestiLuenta ja Emit-kello) |
| (5) OP/Q | Q28 OP-9 (6) ohi; muuten OP-3, OP-7, OP-9, OP-11, OP-13, OP-15 oikein |
| (6) Tietopohja | Kaytetty Q4, Q39 (aineisto) ja Q11 (kortti); kaikki tarkistetut tietopohjaperaiset vaitteet pitivat. Q10 olisi hyotynyt aineistosta (luku 7). Kortteja H-001/H-090/H-116/H-145 ei kaytetty Q37/Q34/Q28/Q12:ssa (rekisteri luettiin suoraan, mika on ohjeen mukaista) |

## 5. v3 vs v5

| Virhetyyppi | v3 (k1-arvio luku 6) | v5 (tama arvio) |
|---|---|---|
| Vaarat prioriteetit | 1 (H-164 P3, Q4; sama virhe v4:n Q10:ssa) | 0 |
| Vaarat tilat/sarakkeet | ei eritelty (rekisterikentat jatettiin hakematta) | 1 (Q34 tyyppi tilana) + 1 vaikutusarvo |
| Puutteelliset haut | useita: JS-14 (Q7), rekisterikentat ja OP-tilat hakematta, Q4 symbolihaku | 4-6; kaikki paitsi Q39:n hakuvirhe agentin itse ilmoittamia; JS-14 edelleen |
| Vaarat viitteet / osuus tarkistetuista | noin 9-11 / noin 110 (8-10 %) | 4 / noin 95 (noin 4 %) |
| Kuollut koodi | 1 (Q4 addem TALLEMIT :1333) | 0 |
| Lukumaaran ristiriita ilmoittamatta | 1 (Q10) | 0 |
| Rekisteroity havainto ehdokkaana | 0 | 0 |
| Arvosanat (hyv / huom / hyl) | 7 / 7 / 0 | 8 / 7 / 0 |

Arvio: virheet vahenivat. Selvin parannus on Emit-ketjussa (Q4, Q39): v3:n Q4 kuvasi kuolleen
addemin, jatti tallennuksen lukematta ja antoi vaaran H-164-prioriteetin; perusvastaus vaitti
raakatietueen lahtevan muille asemille. v5:ssa nama virheet ovat poissa, ja ero johtuu
todennakoisesti aineistosta [paatelty]: emit-luenta.md nimeaa kaannetyn addemin ja kirjaus-ehdon,
ja agentti tarkisti juuri ne koodista. Muualla parannus naytaa tulevan ohjeen luvun 4b kohdista
(prioriteetti CSV:sta, lukumaarat kahtena lukuna), ei tietopohjasta: 12 vastausta 15:sta ei
kayttanyt tietopohjaa lainkaan. Tietopohjasta ei siirtynyt vastauksiin vaaraa tietoa.

Jaljella olevat yleiset puutteet:
1. JS-tunnukset jaavat yha pois (Q7 JS-14, kolmas kierros), vaikka havainnon rivi luetaan.
2. Sarakesekaannus siirtyi riskiarviosta toiseen rekisteriin: korjauskokonaisuudet.csv:n tyyppi tilana (Q34).
3. OP aiheen mukaan vs KK:n mukaan: agentti ottaa kokonaisuuden OP:n eika tarkista aiheen OP:ta (Q28).
4. Tarkistukset-rivin "ei osumia" ei ole luotettava (Q39): hakumuoto voi olla vaara, eika agentti
   kokeile toista muotoa. Tyokalulokit puuttuvat edelleen.
5. Ohjelmakohtaiset vaitteet otetaan dokumentin taulukosta yhdistellen (Q10: RTR ja EMITKELLO).
6. Laajassa vastauksessa havaintoluettelo karsitaan ilman priorisointia (Q39: P1-havainnot pois).
7. Tietopohjan kaytto on kapeaa: kortteja ja kasikirjan lukuja ei haeta, ellei kysymys ole Emit-ketju.

## 6. Ratkaisematta

- Miksi Q39:n havainnot.md-haku epaonnistui (hakumuoto vai tekematon haku)? Ei selvia ilman tyokalulokia.
- Q10: ViestiLuenta ja Emit-kello on staattisesti kaannetty (Emit-tarkastus), ajossa todentamatta.
- Q34:n haelahdejono-polku (`Hk/HkIV.cpp:2738` -> `:2821`) on staattisesti mahdollinen. Se vaatii
  lahdejonohaku-asetuksen ja ajon, eika sita ole rekisterissa erikseen.
- Kahden yksittaisen virheen (Q33, Q34) syyta, sarakesekaannus vai lukuvirhe, ei voi erottaa ilman tyokalulokia.
