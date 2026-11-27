# Ominaisuusvalitsimet ja asetukset / Feature switches and settings (C-12)

Erä C-12, vaihe 1 (2026-11-26, revisio `ab10f88`, `TPsource/` ennallaan revisiosta `f7fe553`).
Toteuttaa alkuperäisen vaiheen C kohdan 3 ("mitä kukin `#define` tekee"), ratkaisee Q2:n ja arvioi
T-008:n. Rajaus: `suunnitelma.md`, "C-12 vaihe 1: rajaus". Staattinen analyysi: mitään ei käännetty,
ajettu tai yhdistetty, eikä tuotantokoodia, projekteja tai CI:tä muutettu.

Batch C-12, step 1. Implements original phase C item 3, resolves Q2 and assesses T-008. Static only.

Polut ovat suhteessa hakemistoon `TPsource/V52/`. Merkinnät: [todennettu] = koodi luettu,
[päätelty] = päätelty, [tuntematon] = ei selvitetty. / Paths relative to `TPsource/V52/`.

## 1. Yhteenveto / Summary

- **Laajuus.** Viiden tuotteen ja neljän kirjaston lähteissä ehtona testataan **435 makroa**
  **2 491 ehtokohdassa** (`#if/#ifdef/#ifndef/#elif/#else`).
  - Luokat: 151 include-vartijaa, 39 kääntäjä- tai alustamakroa, 60 ominaisuusvalitsinta,
    5 muunnelmaa ja 2 pois pakotettua muunnelmaa, 5 päälle pakotettua ominaisuutta, 28 kokovakiota,
    5 diagnostiikkavalitsinta, 107 koskaan määrittelemätöntä, 6 käänteistä (opt-out), 15 TU-paikallista
    ja 12 muuta (`rekisteri/valitsimet.csv`).
  - Kuolleissa tai saavuttamattomissa haaroissa on noin 7 400 riviä (sisäkkäiset kahteen kertaan);
    suurimmat ovat `EI_OLE` 1 774, `MAKI` 1 232 ja `AMPUSUUNN` 967.
- **Q2 ratkaistu** (luku 4). Esimerkkejä:
  - `GAZ` = konsoli-HK:n sarjaporttitaulu;
  - `MERI5` = kolmen vaiheen pistekaava;
  - `LAJUNEN` = Sirit/Zebra-RFID-lukijat;
  - `PARTIO` = viestin partiojoukkueet.

  Vailla vaikutusta ovat `ORAVA` ja seuraavat projektimääritykset, joita mikään ehto ei testaa:
  `VERTAA`, `PISTEET`, `UUSI`, `MAAKNT`, `EMITAG`, `AMPU`, `ISODATA`, `JUKOHJ`, `JUKOLA`, `VAMAALI`
  ja `MILLISEKAJAT`. `MAKI` ja `HIIHTO` ovat käytännössä määrittelemättömiä kaikissa tuotteissa.
- **T-008 arvioitu** (luku 5). Konsolin ja GUI:n erot ovat pääosin tarkoituksellisia, ja GUI:ssa on
  vastaavat lomakkeet. Erilleen ajautumista ovat `DBGFILE`-valitsimen epäjohdonmukaisuus projekteittain ja toimittamattomien kokoonpanojen koot (T-051);
  ominaisuusaukko on viestin GUI:sta puuttuva esitarkastus (`ESITARK`).
- **Ajonaikaiset asetukset** (luku 6).
  - Todellisia jäsentimiä on kaksi, `Hk/HkInit.cpp` ja `Juk/VInit.cpp`, ja kumpikin käännetään sekä
    konsoliin että GUI:hin.
  - `cbHk/UnitCfg.cpp` ja `ViestiWin/UnitCfg.cpp` ovat asetuseditoreja: ne vain kuvaavat rivejä
    lomakkeen kenttiin. Editorit kirjoittavat avaimia, joita ajonaikainen jäsennin ei tunne (H-173).
  - Vain konsoliohjelmat ottavat avaimia komentoriviltä. Tiedostot luetaan kolmessa kerroksessa
    (`peruscfg.cfg` → asetustiedosto → komentorivi), ja myöhempi voittaa.
- **Muut kytkimet** (luku 7): tiedoston olemassaolo (esim. `AUTOFILE.LST`, `RADAT*.XML` → `.LST`,
  `KilpSrj.xml` → `KILP.SRJ`), `ikkunat.xml`, `Init.cfg`, yksi rekisteriarvo ja kilpailuparametrit
  (`kilplaji`, `alalaji`, kortin laji, `partio`, `rogaining`). Ympäristömuuttujia tuotekoodi ei lue.
- **Uudet havainnot** H-171–H-177 ja T-050–T-052. Laajennetut: H-036, H-134 ja T-008. Merkittävimmät:
  - H-171 (korkea, P2 sulkemisessa): ViestiWin kirjoittaa jokaisessa kilpailun avauksessa 158 tavua
    `cfgfilename[41]`:n yli viereiseen staattiseen muistiin (luku 13.2).
  - H-172 (keskitaso): viestin konsolin X)ML-valinta tuottaa lähes tyhjän tiedoston.
- **Tuotokset:** ominaisuusmatriisi (luku 9) ja riskiperusteinen testimatriisi (luku 10).

English:

- **Scope.** 435 tested macros at 2,491 sites. About 7,400 lines sit in dead branches.
- **Q2 and T-008.** Q2 is resolved. T-008 is assessed: the differences are mostly intended, with some
  drift (DBGFILE, `ESITARK`).
- **Parsers.** There are two run-time parsers. The two `UnitCfg.cpp` files are config editors, and they
  write keys the parser rejects. Only the console programs take keys on the command line.
- **Findings.** New H-171..H-177 and T-050..T-052. The most important are H-171 (ViestiWin `cfgfilename`
  overrun on every start) and H-172 (relay console X)ML output near empty).

## 2. Menetelmä, todennus ja kattavuus / Method, verification and coverage

| Osa / Part | Tekijä / By | Tuotos / Output |
|---|---|---|
| A0 | `tyokalut/valitsimet.py` (C-10:n malli, `riippuvuudet.build_model`) | `todisteet/c12/osaA0/`: makrot, ehtokohdat tuotteittain, määritykset, kokoonpanot |
| B0 | `tyokalut/asetusavaimet.py` | `todisteet/c12/osaB0/`: 957 avainvertailua, 239 peittoehdokasta, 166 kommentoitua |
| A | `build-dependency-analyst`, Sonnet (~205 k tokenia) | `todisteet/c12/raportit/osaA-esikaantajavalitsimet.md` |
| B | `architecture-analyst`, Sonnet (~245 k) | `raportit/osaB-asetukset-komentorivi.md` |
| C | `architecture-analyst`, Sonnet (~163 k) | `raportit/osaC-muut-kytkimet.md` |
| D | `independent-verifier`, Opus (~114 k) | `raportit/osaD-tarkastus.md`: 19 väitettä, 12 vahvistettu, 7 osittain, 0 kumottu |
| E | pääistunto / main session, `tyokalut/valitsinrekisteri.py` | tämä raportti, `rekisteri/valitsimet.csv`, `rekisteri/asetusavaimet.csv` |

Agentit käyttivät yhteensä noin 0,73 miljoonaa tokenia. A, B ja C ajettiin rinnakkain (enintään kolme
agenttia), D niiden jälkeen. / Agents about 0.73 M tokens in total.

**A0:n ja B0:n tarkistus ennen agenttikäyttöä.** Tarkistukset tehtiin C-10:n tapaan, ja tulokset on
kirjattu `todisteet/c12/osaA0/README.md`:hen ja `osaB0/README.md`:hen.

- Release-arvot vastaavat C-10:tä.
- Kokoonpanojen määritykset vastaavat projektitiedostojen XML:ää (15/15).
- Ehtokohtien määrä vastaa raakahakua (2 491 = 2 491).
- `inventaario.md`:n taulukko täsmää lukuun ottamatta Win64-ryhmän `VERSIONO`a.
- Otannassa löytyi oletusarvolohkojen virhe, joka korjattiin, ja korjauksen muuttamat 63 kohtaa
  käytiin läpi.
- C-10:n tulosteet pysyivät tavulleen ennallaan.

**Riippumaton tarkastus (D).** Tarkastettiin 19 merkittävää väitettä.

- Yhtään väitettä ei kumottu.
- Korjauksia tuli kolme:
  - A1:n rivimäärä on 325 eikä 393.
  - B2:sta kumoutuivat `TAKAA-AJO` ja `NOLLAKÄRKI`, jotka HK hyväksyy.
  - C2 ei ole täysin hiljainen.
- Kaksi väitettä osoittautui vakavammiksi kuin agentit esittivät: B4 (H-171) ja A2 (H-172).
- Pääistunto todensi B4:n itse.

/ A0 and B0 were validated before agent use. Part D reviewed 19 claims: none refuted, three corrected,
two made more severe.

**Valmiusehdot / completion criteria** (`suunnitelma.md`):

| # | Ehto / Criterion | Tila / State |
|---|---|---|
| 1 | Jokainen testattu makro luokiteltu, ominaisuusvalitsimille merkitys, arvo ja määrityskohta / every macro classified | **täyttyy**: 435/435 luokiteltu (`rekisteri/valitsimet.csv`); include-vartijat ja kääntäjämakrot massavahvistettu. Koskaan määrittelemättömien merkitys on osin nimeen perustuva arvaus [päätelty] (luku 3.4) / met; meanings of never-defined macros partly name-based |
| 2 | Neljän jäsentimen avaimet ja komentorivi ohjelmittain / keys of the four parsers and the command line | **täyttyy**: 750 avainriviä (`rekisteri/asetusavaimet.csv`), tulkinta luku 6. Noin 80 vähäisen avaimen merkitys "katso koodi"; globaalien alkuarvoja ei käyty läpi yksitellen / met; ~80 minor keys "see code" |
| 3 | Tiedosto-, GUI-, rekisteri- ja kilpailuparametrikytkimet / other switches | **täyttyy** (luku 7); `ViestiWin/WinParamXml.cpp` oletettu HK:n vastineeksi [päätelty] |
| 4 | Q2 ratkaistu, T-008 arvioitu | **täyttyy** (luvut 4–5) |
| 5 | Riippuvuudet, ristiriidat, kuolleet ominaisuudet | **täyttyy** (luku 8) |
| 6 | Ominaisuus- ja testimatriisi | **täyttyy** (luvut 9–10) |
| 7 | Riippumaton tarkastus, validaattori, paikallinen commit | **täyttyy** (D; validaattori luku 12) |

## 3. Esikääntäjän valitsimet / Preprocessor switches

Koko luettelo: `rekisteri/valitsimet.csv`. Siinä ovat luokka, merkitys, arvo viidessä tuotteessa ja
kirjastoissa, poikkeavat arvot muissa kokoonpanoissa, määrityskohdat, ehtokohdat, tila ja tunnukset.
Agentin perustelut ovat raportissa `raportit/osaA-esikaantajavalitsimet.md`.

Arvot annetaan järjestyksessä HkMaali, ViestiMaali, ViestiLuenta, HkKisaWin, ViestiWin; `-` tarkoittaa
määrittelemätöntä. / Full list in the register; values in product order.

### 3.1 Muunnelmat / Variants

| Makro | Merkitys / Meaning | Arvot / Values | Huom. |
|---|---|---|---|
| `_CONSOLE` | konsoli- vs. GUI-käännös samasta lähteestä; 252 kohtaa, HK 3 494 ja viesti 4 942 eroriviä / console vs GUI build | 1,1,1,-,- | `include/tputil.h:474` |
| `_BORLAND_` | C++Builder-projektivalitsin: includet ja API-nimet; ainoa toiminnallinen lohko on KUNTOSUUNN-vaihto (H-081) / C++Builder switch | -,-,-,1,- | puuttuu ViestiWinistä; vaikutusta ei löytynyt (D/A3) |
| `LUENTA` | pelkkä kortinluku-viestiohjelma; poistaa ajanotto- ja tulostuskoodia (4 144 riviä) / reader-only relay | -,-,1,-,- | T-052 |
| `MAALI` | maali- ja ajanottotila, pakotettu päälle / finish mode, forced on | 1,1,1,1,1 | `Hk/HkDef.h:112`, `Juk/VDef.h:75` |
| `MAKI`, `HIIHTO` | mäki- ja hiihtomuunnelma (nimen perusteella); kumpaakaan ei määritellä missään (D/A4: `MAKI`:lla `#undef`, `HIIHTO` vain määrittelemätön) / never defined | käytännössä - kaikissa | `Hk/HkDef.h:33-35`, `:118-120`; 1 232 kuollutta riviä `MAKI`:n alla |

### 3.2 Ominaisuudet tuotteittain / Features per product

| Makro | Merkitys / Meaning | Arvot | Näyttö / Evidence |
|---|---|---|---|
| `EMIT`, `EMITLEIMAT`, `TCPSIIRTO`, `TCPLUKIJA`, `AUTOFILE`, `KONETUNN` | emit-kortit, emit-väliajat (johdettu), TCP-tiedostonsiirto (johtaa HK:ssa `AUTOFILE`n), TCP-lukija, automaattinen tulostus, naapurin konenimi | kaikissa 1 / all | `Hk/HkDef.h:54`, `:112`; `Tp/TpComY.cpp:1742` |
| `EMITHTTP` | emiTagin WinHTTP (`riippuvuudet.md` luku 10) | 1,1,-,1,1 | `Juk/VInit.cpp:1852` |
| `LAJUNEN` | Sirit/Zebra-UHF-RFID-lukijat | 1,1,-,1,1 | `include/SiritReader.h:10`; `Tp/TpLaitteet.cpp:669-673` |
| `VALUKIJA`, `AUTOVA` | väliaikalukijan välitys vertaisille; automaattinen väliaikakäsittely (viestissä `VALUKIJA` → `AUTOVA`) | 1,1,-,1,1 | `Hk/HkCom32.cpp:497`; `Juk/VDef.h:38-40`; `Hk/HkIV.cpp:3373` |
| `EMITFILE` | testisyöte `EMIT_IN.DAT`:sta (portti > 900) / test input file | 1,1,-,1,- | `Juk/VIv.cpp:810-892`; `#undef` LUENTA:ssa `Juk/VIv.cpp:47` |
| `SERVER` | konsolin TCP-palvelinapurit, `VARASERVER` | 1,1,1,-,- | `Tp/TpComY.cpp:2429`; `Hk/HkInit.cpp:1192` |
| `GAZ` | konsoli-HK:n sarjaporttitaulu (`GAZ=`, `GAZVAIHE=`); viestissä ei käyttäjää | 1,1,-,-,1 | `Hk/HkIV.cpp:3102`; `Hk/HkInit.cpp:1376` |
| `ENNAKOI` | ennakoitu numerosyöttö konsolin aikaruudulla (GUI:ssa `cbHk/UnitAjanotto.cpp:163`) | 1,1,-,-,- | `HkConsole/HkCons2.cpp:2121-2154` |
| `AIKAVERT` | konsoli-HK:n aikavertailuruutu (`VERTRAJAT=`); GUI:ssa `cbHk/VertAikaUnit.cpp` | 1,-,-,-,- | `HkConsole/HkCons2.cpp:2581` |
| `ALGETAULU` | viestin ALGE-tulostaulu (`TAULU_COM=`) | -,1,-,-,1 | `Juk/VInit.cpp:1008`; `JukConsole/lasknum5.cpp:17` |
| `ESITARK`, `TESTIAJAT` | emit-esitarkastusruutu; Ctrl-T testiaikojen lähetys | -,1,-,-,- | `Juk/VEmit.cpp:3702`; `JukConsole/vcons2.cpp:2299` |
| `PARTIO` | viestin partiojoukkueet (`kilpparam.partio`) | -,1,1,-,1 | `Juk/VEmit.cpp:1530`, `:2235` |
| `SEURAVAL` | seuravalintalista, `lah_seura` | 1,-,-,1,- | `HkConsole/HkKilp.cpp:61`; `Hk/HkCom32.cpp:460` |
| `JOUKKUETUL` | HK:n joukkuetulokset (vain HkKisaWin); viestin kopio kuollut | -,-,-,1,- | `Hk/HkTulostus.cpp:7601`; `Juk/VTulostus.cpp:7672` |
| `MERI5`, `JOUKKUEPISTEET`, `SAKKOCOM`, `NOMOUSE`, `HIIHTOLAHTO` | pakotettu HK:ssa päälle: kolmen vaiheen pisteet, joukkuepisteet, sakkokanava, ei hiirtä, hiihtolähtö / forced on in HK | 1,-,-,1,- | `Hk/HkDef.h:37-43`, `:79`, `:122-126` |
| `NOCOM_AIKA` | aikaleimaliikenne pois Luennasta; johdettu kirjoitusvirheen `NOCOM_AIMA` kautta | -,-,1,-,- | `Juk/VDef.h:49-51` |
| `DBGFILE`, `PROFILOI` | lukijan raakatavut tiedostoon (`DBGTOFILE`); viestin tiedonsiirron profilointi | -,1,1,1,- ja -,1,1,-,1 | `Tp/TpLaitteet.cpp:1624`; `Tp/TpComY.cpp:185` |
| `DEMO` | demoversio, enintään 30 (HK) tai 20 (viesti) kilpailijaa; vain Demo-kokoonpanoissa | - toimitetuissa | `Hk/HkIx.cpp:1581-1582`; `Juk/VIx.cpp:1077-1078` |

### 3.3 Kokovakiot / Size constants

Kokovakiot ovat verkkotunnuksen sisällä samat konsolissa ja GUI:ssa (HK / viesti):

| Vakiot | HK / viesti |
|---|---|
| `HAJONTA` | 11 / 8 |
| `LJK` | 15 / 30 |
| `LPSARJA` | 31 / 40 |
| `LSEURA` | 31 / 50 |
| `MAXYHD` | 40 / 10 |
| `MAXNSEURA` | 600 / 2000 |
| `MAXSARJALUKU` | 200 / 250 |

Lähteet: `Hk/HkDef.h:168-212`, `Juk/VDef.h:107-203`. Kokovakioissa on kolme poikkeusta:

- `NREGNLY`: viestin konsolissa 8, GUI:ssa 32 (T-024).
- Toimittamattomat kokoonpanot:
  - `VALUKU=200` (HkMaali DebugV200 ja ReleaseVA200, HkKisaWin 200Debug ja 200Release) muuttaa
    `combufrec`-koon (`tiedostomuodot.md`).
  - Viestin 250 osuutta, 25 sarjaa ja 8 väliaikaa ovat käytössä ViestiMaalin Debug250- ja
    Release150os-kokoonpanoissa sekä ViestiWinin Debug Build-, Debug150os- ja Release150os-kokoonpanoissa.
    ViestiLuenta käyttää kaikissa kokoonpanoissa toimitettua kokoa (D/A6, T-051).
- `MAX_LAHPORTTI`: viestin otsikon oletus 32 ei ole voimassa missään, sillä jokainen projekti
  määrittelee arvon 64. Tämä korjaa C-10:n oletuksen: `katkkierto[]`:n koko on 64 (D/B1).

/ Size constants are equal within a domain; exceptions NREGNLY (T-024), non-shipped configurations
(T-051) and `MAX_LAHPORTTI` (64 everywhere).

### 3.4 Kuolleet ja koskaan määrittelemättömät / Dead and never-defined

107 makroa ei määritellä missään projektissa eikä lähteessä. Ryhmät (merkitykset osin nimen
perusteella [päätelty], `raportit/osaA` 1d):

- **Käytöstä poistettu koodi.** `EI_OLE` (1 774 riviä), `AMPUSUUNN` (ammunsuunnistus, 967), `SIIMPORT`
  (307), `IOF` (250), `SPORTIDENT` (243), `SWIM2000` (148), `HALIKKO` (153; valitsisi myös
  `MAXSARJALUKU 1`), FIS-pistetaulukot, `TAULU_2`, `TV1`, `TXTTV`, `COMETNO`, `EBA`, `TUTKA`,
  `KEHITYSLISTA` (tehtävälista `BuildNo.h`:ssa) sekä `DBG*`-jäljitykset.
- **Käänteiset (opt-out).** Näitä ei määritellä, joten niiden suojaama koodi on aina päällä:
  `EIMAALI` (2 027 riviä aina päällä), `EILEIMAT`, `NOREGNLY`, `TESTI`, `COMOFF`, `UINTI`.
- **Kirjoitusvirheet.**
  - `_BORLANDC_` (yksi alaviiva) suojaa vain `#pragma warn` -rivejä, joten ne eivät koskaan ole
    voimassa (D/A5).
  - `NOCOM_AIMA` on luultavasti kirjoitusvirhe sanasta `NOCOM_AIKA`.
  - `Juk/VDef.h:117` sisältää rivin `#endif !defined(...)` (T-050).
- **TU-paikalliset, testattu toisessa käännösyksikössä.**
  - `SEURATUL`: konsolin seuratulokset on käännetty pois HkMaalista (H-177).
  - `XML`: viestin konsolin XML-lohko (H-172).
  - `TARKNO`: vaihtoehtoinen tiedostoversio.
- **Projektimääritykset, joita mikään ehto ei testaa.** `VERTAA`, `UUSI`, `MAAKNT`, `PISTEET`,
  `EMITAG`, `AMPU`, `ISODATA`, `JUKOHJ`, `JUKOLA`, `VAMAALI`, `MILLISEKAJAT`, `VERSIONO`, `STRICT`.

/ 107 never-defined macros: retired code, opt-outs (always on), typos, TU-local ones tested elsewhere;
13 project defines have no consumer.

## 4. Q2: valitsinten merkitys / meaning of the switches

Q2 on ratkaistu. Jokaisen valitsimen merkitys on luvussa 3 ja rekisterissä; ne, joita kysymys nimeää:

| Valitsin | Vastaus / Answer | Näyttö |
|---|---|---|
| `GAZ` | konsoli-HK:n sarjaporttitulostaulu; GUI:ssa omat yksiköt (`cbHk/UnitTaulu.cpp`, `ViestiWin/UnitGAZ.cpp`); viestin konsolissa määritelty mutta vailla käyttöä | `Hk/HkIV.cpp:3102-3141`; `Hk/HkInit.cpp:1376-1392` |
| `ORAVA` | ei vaikutusta: molemmat testit ovat `SEURATUL`-lohkoissa, jotka on käännetty pois (H-177). Merkitys nimen perusteella sarjakysymyksen muunnelma [päätelty] | `HkConsole/HkCons3.cpp:1716`, `:2660` |
| `VERTAA`, `PISTEET`, `UUSI`, `MAAKNT` | ei vaikutusta; mikään ehto ei testaa niitä. `PISTEET=` on vain asetusavain | `Hk/HkInit.cpp:2027` |
| `MERI5` | kolmen vaiheen pistekaava `meri3pisteet`; pakotettu HK:ssa päälle | `Hk/HkTulostus.cpp:2528-2575`; `Hk/HkDef.h:37-39` |
| `LAJUNEN` | Sirit/Zebra-RFID-lukijatuki; nimi luultavasti asiakas [päätelty] | `include/SiritReader.h:10` |
| `PARTIO` | viestin partiojoukkueet yhdessä ajonaikaisen `kilpparam.partio`:n kanssa | `Juk/VEmit.cpp:1530`; `Juk/VXml.cpp:563-574` |
| `AIKAVERT`, `ENNAKOI`, `SEURAVAL`, `JOUKKUETUL`, `KONETUNN`, `AUTOVA`, `VALUKIJA`, `SAKKOCOM`, `ESITARK`, `TESTIAJAT`, `PROFILOI`, `DBGFILE`, `ALGETAULU`, `MILLISEKAJAT`, `AMPU` | luku 3.2 ja 3.4 | |
| `MAKI`, `HIIHTO` | käytännössä määrittelemättömiä kaikissa tuotteissa (luku 3.1). Arvioijan `?` johtuu ehdollisesta `#undef`:stä tai uudelleenmäärityksestä | `Hk/HkDef.h:33-35`, `:118-120` |
| `EMITFILE` (C-10:n tuntematon) | määritelty ViestiMaalissa, määrittelemätön ViestiLuennassa ja ViestiWinissä; tuntemattomuus johtuu ehdollisesta `#undef`:stä | `Juk/VIv.cpp:47` |

Kysymyksen toinen osa, "miksi konsoli- ja GUI-HK eroavat", on luvussa 5. / Q2 resolved; the
console/GUI part is answered in section 5.

## 5. T-008: konsoli vs. GUI / console vs GUI

**HK (HkMaali vs. HkKisaWin)**

| Valitsin | Ero / Difference | Arvio / Assessment |
|---|---|---|
| `_CONSOLE` | TUI vs. VCL, 124 kohtaa ja 3 494 riviä | tarkoituksellinen / intended |
| `SERVER`, `GAZ`, `AIKAVERT`, `ENNAKOI` | konsolin käyttöliittymätoiminnot; GUI:ssa vastineet (`UnitTaulu`, `VertAikaUnit`, `UnitAjanotto`) | tarkoituksellinen |
| `JOUKKUETUL` | joukkuetulokset vain GUI:ssa | tarkoituksellinen [päätelty] |
| `DBGFILE` | puuttuu HkMaali Releasesta | ajautuminen / drift (T-051) |
| `_BORLAND_` | KUNTOSUUNN-kortinvaihto vain GUI:ssa (H-081) | näkyvä ero, H-081 |
| `ORAVA`, `VERTAA`, `PISTEET`, `UUSI`, `MAAKNT` | vain konsolin projektissa, ilman vaikutusta | kuollut määritys (T-050) |

**Viesti (ViestiMaali/ViestiLuenta vs. ViestiWin)**

| Valitsin | Ero / Difference | Arvio |
|---|---|---|
| `_CONSOLE`, `SERVER`, `TESTIAJAT`, `ENNAKOI` | konsolin käyttöliittymä | tarkoituksellinen |
| `ESITARK` | esitarkastusruutu vain ViestiMaalissa; GUI:ssa ei vastinetta | ominaisuusaukko [päätelty] (T-051) |
| `DBGFILE`, `EMITFILE` | puuttuvat ViestiWinistä (HkKisaWinissä ovat) | ajautuminen (T-051) |
| `NREGNLY` | 8 vs. 32 | T-024 |
| `LUENTA` | ViestiLuennasta puuttuvat `LAJUNEN`, `EMITHTTP`, `VALUKIJA`/`AUTOVA`, `ALGETAULU`, `ESITARK`, `ENNAKOI`, `TESTIAJAT` ja `EMITFILE` | supistettu käännös; RFID:n puuttuminen kortinluvusta on kysymys omistajalle (T-052, Q111) |

**Arvio:** T-008 säilyy tasolla keskitaso. Erot ovat pääosin tarkoituksellisia, mutta ajautuminen
(T-051) ja editorin ja jäsentimen ero (H-173) ovat riskejä, joita mikään ei valvo. / Mostly intended;
drift is unguarded.

## 6. Ajonaikaiset asetukset ja komentorivi / Run-time settings and command line

Avainluettelo: `rekisteri/asetusavaimet.csv` (750 riviä: jäsennin, avain, verrattu pituus, rivit,
`#if`-ehto ja voimassaolo tuotteittain). Merkitykset ja erot: `raportit/osaB-asetukset-komentorivi.md`
luvut 1–2.

### 6.1 Jäsentimet ja kerrokset / Parsers and layers

**Ajonaikaiset jäsentimet.** `lue_parametrit` on `Hk/HkInit.cpp`:ssä (HkMaali ja HkKisaWin, noin
230 avainta) ja `Juk/VInit.cpp`:ssä (viestin kolme ohjelmaa, noin 200 avainta).

**Avainrivin käsittely.**

- Rivi katkaistaan ensimmäiseen välilyöntiin ja muutetaan isoiksi kirjaimiksi; vain esikopiolistan
  avaimet säilyttävät rivin loppuosan (`Hk/HkInit.cpp:612-629`).
- Yli 77 merkin rivi katkeaa ilman ilmoitusta, ja enintään 199 riviä luetaan.
- Vertailu on etuliitevertailu pituudella n.

**Asetuseditorit.** `cbHk/UnitCfg.cpp` ja `ViestiWin/UnitCfg.cpp` (`TFormCfg::TulkitseTiedot`)
kuvaavat tunnetut rivit lomakkeen kenttiin. Tuntemattomat rivit kulkevat läpi muuttumattomina.

**Kerrokset ja komentorivi.**

- Kerrokset ovat `peruscfg.cfg`, sitten asetustiedosto (`LASKENTA.CFG` tai `CFG=`) ja sitten
  komentorivi; myöhempi voittaa.
- Komentoriviltä avaimia ottavat vain HkMaali, ViestiMaali ja ViestiLuenta. GUI:t kutsuvat
  `aloitus(0, 0, cfgflnm)` (`cbHk/WinHk.cpp:279`, `ViestiWin/UnitMain.cpp:220`), ja ViestiWin lukee
  komentoriviltä vain `INIT=`:n (D/B7).

**Pakolliset tiedot.**

- Mikään avain ei ole pakollinen, ja puuttuva asetustiedosto ohitetaan hiljaa, ellei `CFG`-avainta ole
  annettu.
- Tuntematon avain tuottaa ilmoituksen, ja käynnistys jatkuu.
- `_wtoi` muuttaa virheellisen luvun nollaksi.
- Käynnistys keskeytyy vain liian suureen `VAKANTTILUKU`un tai sarjojen lukuvirheeseen
  (`Hk/HkInit.cpp:3203-3205`, `:3254-3255`).

### 6.2 Erot ja viat / Differences and defects

**Editorin ja jäsentimen ero (H-173).** Editorit kirjoittavat avaimia, joita jäsennin ei tunne:

- `AIKACOM=`, vaikka jäsennin tuntee vain `AIKA_COM=`; editori myös lukee `AIKA_COM=`, joten
  edestakainen tallennus kadottaa asetuksen;
- `AUTOJAKELU=`;
- viestissä lisäksi `AIKAERO1=`/`AIKAERO2=`, `EMITALKU=`, `EMITJONO=`, `EMITAJAT` ja `SUORASYÖTTÖ`,
  jotka ovat ilmeisesti HK:sta kopioituja.

**Sama avain, eri merkitys HK:ssa ja viestissä.** Esimerkkejä (`raportit/osaB` 2.3):

- `AIKAKORJAUSn` on HK:ssa `* SEK / 100`, viestissä `* SEK` (Q112).
- `TARKNO` on HK:ssa sana, viestissä numero.
- `JONOT`- ja `SIRIT*PORT`-avaimilla on indeksi vain HK:ssa.
- Pelkkä `ALGE` valitsee viestissä ALGE-ajanoton, HK:ssa ei mitään.
- Viestin konsoli ei tunne avaimia `IKKUNAT`, `SQL*`, `KARTTA=` ja `VALONÄYTTÖ=`, jotka HK:n konsoli
  hyväksyy.

**Etuliitepeitto.** 239 ehdokkaasta todellisia ovat:

- haarat `TDC`, `S3` ja `S4`, joihin ei päästä, joten TDC8001-tukea ei voi ottaa käyttöön
  (`Hk/HkInit.cpp:1799-1837`);
- indeksoitu muoto `PISTEETn=`;
- toinen `LÄHDEPISTEET`.

Muut ehdokkaat kumoutuvat esikopiolistan, vartijoiden tai järjestyksen takia (D/B6, H-176).

**Viat.**

- H-171: ViestiWinin `cfgfilename[41]` ylittyy.
- H-036 laajennettu: `KATKKIER`.
- H-176 kokoaa pienet viat: `UDPVIIVEET=`:n ensimmäinen arvo hukkuu viestissä, `IKKUNAOTS` muuttuu
  isoiksi kirjaimiksi ja katkeaa, ViestiLuennan RFID-avaimet valitsevat ALGE:n, `TARKNO=ON` antaa
  arvon 31 ja `fldn` ylilukee.

/ Two run-time parsers plus two editors. Editor/parser drift is H-173; HK vs relay semantics of the same
key differ; real shadowing in TDC/S3/S4 and `PISTEETn=`; defects H-171, H-036 and H-176.

## 7. Muut kytkimet / Other switches

Koko luettelo: `raportit/osaC-muut-kytkimet.md`. Toimintaa kytkevät seuraavat.

- **Tiedoston olemassaolo / file presence.** Tiedostot luetaan nykyhakemistosta, eli kilpailu on
  hakemisto.
  - `AUTOFILE.LST`: sarjakohtaiset HTML-, MHTML- ja XML-tiedostot; ilman sitä yksi yhdistetty tuloste.
    Vaatii `#ifdef AUTOFILE` ja `afvali`-avaimen (`Hk/HkTulostus.cpp:7147-7150`, H-149).
  - `RADAT<n>.XML`: puuttuessa käynnistys käyttää `RADAT<n>.LST`:tä ja `LEIMAT.LST`:tä, mutta vaiheen
    vaihto ei (H-134).
  - `KilpSrj.xml` → `KILP.SRJ`: myös jäsennysvirhe johtaa vanhaan tiedostoon (H-174).
  - `HTMLALK?.TXT`/`HTMLLOP?.TXT`: HTML-kehys.
  - Avaimen ottamat käyttöön: `TLSKERR.LST`, `SARJAYHD.LST`, `XMLPISTE.LST`, `TVPISTE.LST`,
    `LAHDEPISTEET*.LST`, `LAINAEMT.LST`, `korostus*.lst` (vain GUI), `pistesrj.lst` (vain GUI),
    `vaihevalinta.cfg` (HK), `AIKARAJAT.TXT` (rogaining), `HAJONNAT.LST` (H-063), `EMIT_IN.DAT`
    (`EMITFILE`).
  - Lukko- tai lipputiedostoja ja yhden instanssin tarkistusta ei ole.
- **GUI:n pysyvät asetukset / GUI persistence.**
  - `ikkunat.xml`: ikkunoiden asettelu ja ikkunakohtaiset valinnat, luetaan vain `IKKUNAT`-avaimella
    (`cbHk/WinParamXml.cpp:197-335`). Emit-lomakkeen valikkotila ja muuttujat eroavat (H-175).
  - `Init.cfg`: GUI:n käynnistystila (`ILM`, `TULOSP`, `KUNT`, `LUETT`, `AVAA`, `CFG=`).
  - `KilpailuLuettelo.xml`: kilpailuvalitsin.
  - `SQLjonoVarm.dat`: toistetaan, kun `CBVarmista` on valittuna (pääosin H-043).
  - Ainoa rekisterin käyttö on IE-selainemuloinnin arvo HKCU:ssa (`cbTp/cbTpApu.cpp:591-630`).
    `TIniFile`- ja `TRegistry`-käyttöä ei ole.
- **Ympäristö / environment.** Tuotekoodi ei lue ympäristömuuttujia. Prosessin syötteitä ovat
  komentorivi, nykyhakemisto ja exe-tiedoston nimi (rekisteriarvon nimi).
- **Kilpailuparametrit / competition parameters** (`KilpSrj.xml`).
  - HK:
    - `Discipline` (`kilplaji`) kytkee viisiottelun pisteet ja ampumahiihdon sakot
      (`Hk/HkInit.cpp:3286-3353`).
    - `Subtype` (`alalaji`) kytkee parikilpailun `kaksibadge`n ja ammunnan.
    - Muut: kortin laji (kytkee emit-luennan päälle ilman avainta, `Hk/HkInit.cpp:3301-3307`),
      `StartGate`, `VariableClass`/`VariableBib`, `CombinedResult` ja `RaceCount`.
  - Viesti: `Subtype` kytkee `partio`n ja `rogaining`in (`Juk/VXml.cpp:563-574`).

## 8. Riippuvuudet, ristiriidat ja kuollut koodi / Dependencies, conflicts, dead code

**Johdetut valitsimet** [todennettu]:

- `TCPSIIRTO` → `AUTOFILE` (HK, `Hk/HkDef.h:54`);
- `VALUKIJA` → `AUTOVA` (`Juk/VDef.h:38-40`);
- `(MAALI || AUTOVA) && EMIT` → `EMITLEIMAT`;
- `LUENTA` → `NOCOM_AIKA`.

`com/WinUDP.cpp:21-23` pakottaa `TCPSIIRTO`n itse (T-045). HK pakottaa päälle `MILLISEKAJAT`,
`MERI5`, `JOUKKUEPISTEET`, `SAKKOCOM`, `NOMOUSE` ja `HIIHTOLAHTO` ja pois `MAKI`n.

**Poissulkevat:**

- `LUENTA` sulkee pois `ALGETAULU`n, `EMITFILE`n, `ESITARK`in, `TESTIAJAT`in ja `ENNAKOI`n.
- `JOUKKUETUL` ja `SEURATUL` sulkevat pois `AMPUSUUNN`in.
- `HALIKKO` ohittaa `MKV`:n.
- `TAULU` ja `ALGETAULU` ovat rinnasteiset.

**Nimitörmäykset:**

- `GDIPRINTER` on tputilv2:n valitsin ja tulostintyypin vakio 7 (`Hk/HkMuotoilu.h:33`,
  `Juk/VDef.h:171`). Ne toimivat yhdessä vain siksi, että `tputilv2/Wrttext.cpp` ei sisällytä noita
  otsikoita (T-050).
- `ALGE` on tyhjä TU-paikallinen määritys ja myös laitetunnuksen merkkijono.
- Lyhyet makronimet `SU`, `EN`, `SV` ja `LANG` (`Tp/TpDef.h:66-70`): `EN`:n 25 osumaa jäivät
  tarkistamatta.

**Kuollut ja saavuttamaton koodi:** luku 3.4. Kuollut koodi on myös rakenteellisesti vanhentunutta:

- `SEURATUL`-lohkossa on syntaksivirhe (`HkConsole/HkCons3.cpp:606`, `wchar _t`).
- `XML`-lohko kutsuisi `xmlots`ia väärällä argumenttimäärällä (`JukConsole/VConTul.cpp:1320`).

Valitsimen kytkeminen päälle ei siis olisi turvallista ilman käännöstä ja testiä. / Switching such
code on would not compile.

## 9. Ominaisuusmatriisi / Feature matrix

Toimitettavat kokoonpanot; `x` = käytössä, `–` = ei. Lähde: `rekisteri/valitsimet.csv` ja luku 6–7.

| Ominaisuus / Feature | Valitsin tai avain | HkMaali | ViestiMaali | ViestiLuenta | HkKisaWin | ViestiWin |
|---|---|---|---|---|---|---|
| Emit-kortit / emit cards | `EMIT`, `EMITLEIMAT` | x | x | x | x | x |
| TCP-siirto ja automaattinen tulostus / TCP send, autofile | `TCPSIIRTO`, `AUTOFILE` + `AUTOFILE.LST` | x | x | x | x | x |
| emiTag-HTTP | `EMITHTTP` + `ET*` | x | x | – | x | x |
| Sirit/Zebra-RFID | `LAJUNEN` + `SIRIT*`/`ZEBRA*` | x | x | – (avain → ALGE, H-176) | x | x |
| Väliaikalukija / split reader | `VALUKIJA`, `AUTOVA` | x | x | – | x | x |
| Emit-testisyöte / test input | `EMITFILE` | x | x | – | x | – |
| Sarjaporttitaulu / serial board (GAZ) | `GAZ` | x | (ei käyttäjää) | – | – (oma lomake) | – (oma lomake) |
| ALGE-taulu | `ALGETAULU` | – | x | – | – | x |
| Aikavertailu, ennakointi (konsoli) | `AIKAVERT`, `ENNAKOI` | x | ENNAKOI | – | lomake | – |
| Emit-esitarkastus / pre-check | `ESITARK` | – | x | – | – | – |
| Partio, rogaining | `PARTIO` + `Subtype` | – | x | x | – | x |
| Joukkuetulokset / team results | `JOUKKUETUL` | – | – | – | x | – |
| Konsolin seuratulokset / club results | `SEURATUL` | – (H-177) | – | – | – | – |
| Viestin konsolin X)ML | `XML` | – | rikki (H-172) | rikki (H-172) | – | – |
| Diagnostiikkatiedosto / debug file | `DBGFILE` + `DBGTOFILE` | – | x | x | x | – |
| Tiedonsiirron profilointi / comm profiling | `PROFILOI` + avain | – | x | x | – | x |
| MySQL-peili / SQL mirror | `SQL*` (GUI) | (avain jäsennetään, vaikutus [tuntematon]) | – | – | x | x |
| Asetukset komentoriviltä / keys on command line | `argv` | x | x | x | – | `INIT=` |
| Ikkunat, käynnistystila / layout, start mode | `ikkunat.xml`, `Init.cfg` | – | – | – | x | x |
| Laiteyhteyksiä / reader channels | `NREGNLY` | 32 | 8 | 8 | 32 | 32 |

## 10. Riskiperusteinen konfiguraatio- ja regressiotestimatriisi / Risk-based configuration test matrix

Rivi kertoo, voiko sen ajaa C-11:n testikehyksessä vai vaatiiko se toimitetun ohjelman, laitteen tai
verkon. Testikehyksen projektit ovat `TpCoreHk` (HkMaali Release -määritykset) ja `TpCoreJuk`
(JukMaali Release), ks. `testaus.md` 3.2. GUI-ohjelmia ja ViestiLuennan määrityksiä kehys ei kata.
Matriisia ei ole ajettu; sen ajaminen vaatii erillisen päätöksen.

/ Each row states whether it runs in the C-11 harness (`TpCoreHk`, `TpCoreJuk`) or needs a shipped
program, a device or a network. Not run; running needs a separate decision.

| # | Kokoonpano / Configuration | Ohjelmat | Aktivoi / Activates | Ajettavuus / Where it runs |
|---|---|---|---|---|
| T1 | Yksittäinen työasema, ei asetustiedostoa / single station, no cfg | kaikki | oletukset, hiljainen puuttuva cfg | kehys (HK, viesti); GUI: toimitettu ohjelma |
| T2 | Verkko: kaksi työasemaa UDP/TCP (`YHTEYSn=`) / network | kaikki | tiedonsiirto (C-2), `KONETUNN` | kehys loopbackilla (H-027-malli); GUI: toimitettu + verkko |
| T3 | Emit-luenta, `EMITFILE`-testisyöte (`EMIT_IN.DAT`) | HkMaali, ViestiMaali, HkKisaWin | emit-polku ilman laitetta | kehys (HK, viesti); ViestiLuenta/ViestiWin eivät tue |
| T4 | RFID: `SIRITn=TCP:` / Zebra, uudelleenyhteys (H-159) | HkMaali, ViestiMaali, GUI:t | `LAJUNEN`-koodi | simuloitu TCP-palvelin kehyksessä; todellinen: laite |
| T5 | RFID-avain ViestiLuennassa (H-176) | ViestiLuenta | `regnly[0]=3` | toimitettu ohjelma (kehyksessä ei LUENTA-määrityksiä) |
| T6 | Automaattinen tulostus `AUTOFILE.LST`:llä ja ilman (H-149), XML-vienti vaihe ≥ 2 (H-150) | kaikki | `AUTOFILE`, `HTML=`, `XML=` | kehys (C-9:n koeohjelmat) |
| T7 | SQL-peili (H-160, H-161), `SQLjonoVarm.dat` (H-043) | HkKisaWin, ViestiWin | `SQL*`, MySQL | toimitettu GUI + MySQL-palvelin |
| T8 | HK:n vaiheen vaihto, `RADAT2.XML` puuttuu tai rikki (H-129, H-134) | HkMaali, HkKisaWin | vaihetila, `.LST`-varapolku | kehys (C-8:n koe); GUI: toimitettu |
| T9 | emiTag `TCPS`/HTTP (H-055), `ETGPRS` (H-166) | ei ViestiLuenta | `EMITHTTP` | kehys syötepuskureilla; todellinen: verkko + palvelin |
| T10 | ViestiWin avaa kilpailun pitkällä polulla (H-171) | ViestiWin | `cfgfilename`-ylitys | toimitettu GUI (+ `.map` tai debuggeri) |
| T11 | Viestin konsoli X)ML (H-172) | ViestiMaali, ViestiLuenta | `xmlvalinnat` | toimitettu ohjelma (konsoli) |
| T12 | Asetuseditorin edestakainen tallennus (H-173) | HkKisaWin, ViestiWin | `AIKACOM=`, `AUTOJAKELU=` | toimitettu GUI; jäsennin itse kehyksessä (avainlista) |
| T13 | `KilpSrj.xml` rikki ja vanha `KILP.SRJ` (H-174) | kaikki | sarjojen varapolku | kehys (HK, viesti) |
| T14 | Kurssihajonnan tulostus ilman `HAJONNAT.LST`:ää (H-063) | ViestiMaali, ViestiWin | `fclose(NULL)` | kehys (`TpCoreJuk`, MSVC) |
| T15 | `NREGNLY` 8 vs. 32: `LUKIJA9`–`32` (T-024) | viesti | laitenumerointi | kehys (viesti 8) + toimitettu ViestiWin (32) |
| T16 | `KUNTOSUUNN` (H-081), MSVC vs. bcc32 | HkMaali, HkKisaWin | `_BORLAND_`-lohko | toimitettu GUI + kehys |
| T17 | Toimittamattomat kokoonpanot `VALUKU=200`, `MAXOSUUSLUKU=250` (T-051) | rakennuskohtainen | tiedosto- ja sanomakoko | vaatii kyseisen kokoonpanon käännöksen |
| T18 | `KATKKIER0` (H-036) | viesti | indeksi -1 | kehys (`TpCoreJuk` + ASan) |

Prioriteetti: ensin T6, T8, T14 ja T18, jotka ajetaan kehyksessä ja aktivoivat tunnettuja vikoja;
sitten T10, T11 ja T12, jotka vaativat toimitetun ohjelman. / Harness rows first.

## 11. Havainnot ja avoimet kysymykset / Findings and open questions

Uudet ja muuttuneet havainnot (`rekisteri/havainnot.md`):

| ID | Lyhyesti / Briefly | Vakavuus |
|---|---|---|
| H-171 | ViestiWin: `wcsncpy(cfgfilename, ..., 120)` 41 merkin puskuriin jokaisessa kilpailun avauksessa | korkea / high (P2, luku 13.2) |
| H-172 | Viestin konsolin X)ML tuottaa tiedoston ilman XML-kehystä (`XML` määrittelemätön, vanhentunut lohko) | keskitaso / medium |
| H-173 | Asetuseditorit kirjoittavat avaimia, joita jäsennin ei tunne; `AIKACOM=`/`AIKA_COM=` -kierros kadottaa asetuksen | matala–keskitaso |
| H-174 | `KilpSrj.xml`:n jäsennysvirhe johtaa hiljaa vanhaan `KILP.SRJ`:hen; väärät ilmoitukset | matala–keskitaso |
| H-175 | Emit-lomakkeen valikkotila ei vastaa `lukutoisto`a eikä `vahvistaaina`a (HK ja ViestiWin) | matala–keskitaso |
| H-176 | Pienet asetus- ja valitsinviat (kokoomarivi) | matala |
| H-177 | HkMaalin konsolin seuratulokset ja `ORAVA` käännetty pois (`SEURATUL` vain TU-paikallinen) | matala |
| H-036 | laajennettu: indeksikartta; `MAX_LAHPORTTI` on 64 | ennallaan |
| H-134 | laajennettu: vaiheen vaihdossa jäsennysvirhe käsitellään onnistumisena vapautetun ratamuistin päällä | matala–keskitaso → keskitaso |
| T-008 | arvioitu (luku 5) | ennallaan |
| T-050 | Valitsinten hygienia: kuollut koodi, kirjoitusvirheet, käyttämättömät määritykset, nimitörmäys | |
| T-051 | Kokoonpanojen ja jäsentimien ajautuminen | |
| T-052 | ViestiLuennan supistettu käännös ilman dokumentoitua rajausta | |

Päällekkäisyydet todettiin ilman uutta tunnusta: C3 on H-063 (T-020) ja C4 pääosin H-043.

Uudet avoimet kysymykset (`seuranta.md`):

- Q107: mitä H-171 ylikirjoittaa (bcc32:n `.map`).
- Q108: H-172:n tiedoston sisältö ajossa.
- Q109: tarkistaako `GETREC` rajat.
- Q110: H-134:n jäsennysvirhehaara ajossa.
- Q111 omistajalle: `SEURATUL`/`ORAVA`, RFID ViestiLuennassa, kuolleiden valitsinten poisto.
- Q112: `AIKAKORJAUS`-yksikkö HK:ssa ja viestissä.

## 12. Rajaukset ja jatko / Limits and continuation

**Rajaukset.**

- Analyysi on staattinen: esikääntäjän tulosta ei ajettu, joten arvot perustuvat leksikaaliseen
  arvioijaan. Otsikoiden järjestystä se ei mallinna; `MAXTUNNUS`-, `LENIMI`- ja `LSNIMI`-sarakkeiden
  poikkeamat on selitetty raportissa A.
- Koskaan määrittelemättömien ryhmien merkitykset ovat osin nimeen perustuvia.
- Noin 80 vähäisen avaimen merkitys on "katso koodi", eikä globaalien alkuarvoja käyty läpi
  yksitellen.
- Ikkunakohtaisista valinnoista selvitettiin vain Emit-lomake.

**Rajattu pois:** korjaukset, C-13, valitsinten poisto tai yhdistäminen, kommentointi ja testimatriisin
ajaminen.

**Jatko.** Korjauskokonaisuudet ja priorisointi tehdään C-13:ssa. Ajoa vaativat kysymykset Q107, Q108
ja Q110 ajetaan vain erillisellä luvalla.

**Validaattori.** `python -I doc/arkkitehtuuri/tyokalut/tarkista_auditointi.py .` meni läpi
commitissa.

/ Static, lexical; meanings of retired switches partly name-based; fixes, C-13, switch removal,
commenting and running the matrix out of scope; runs only on permission.

## 13. Sulkeminen / Closure (2026-11-26)

Käyttäjä hyväksyi C-12:n sulkemisen ilman toista työerää. Tässä luvussa ei tehty uutta laajaa
analyysiä. Siihen sisältyvät kattavuuden ja johdonmukaisuuden tarkistus, H-171:n vakavuuden
rajattu tarkistus, havaintojen liittäminen korjauskokonaisuuksiin ja avoimien kysymysten
sijoittaminen. / Closed on approval without a second step; no new broad analysis.

### 13.1 Kattavuus ja johdonmukaisuus / Coverage and consistency

**Valmiusehdot ja vaiheen C kohta 3.**

- Kaikki seitsemän valmiusehtoa täyttyvät (luku 2).
- Alkuperäisen vaiheen C kohta 3 ("mitä kukin `#define` tekee") on toteutettu esikääntäjän
  valitsinten osalta kokonaan: kaikki ehtona testatut makrot on luokiteltu, ja
  ominaisuusvalitsimille on annettu merkitys.
- Ehtoina testaamattomat `#define`-vakiot (1 681 nimeä, `todisteet/c12/osaA0/maaritykset.csv`)
  eivät ole ominaisuusvalitsimia. Ne on luetteloitu mutta niitä ei ole selitetty; tämä on
  tietoinen rajaus.

**Tilamerkinnät.**

- Q2 on merkitty ratkaistuksi (`seuranta.md`).
- T-008 on arvioitu (luku 5). Jäljellä olevat kehittämistarpeet ovat T-051:ssä:
  - yhteinen avainluettelo jäsentimille ja editoreille;
  - `DBGFILE`-, `ESITARK`- ja `EMITFILE`-erojen tarkoituksellisuuden kirjaaminen;
  - toimittamattomien kokoonpanojen kokojen yhtenäistäminen tai merkitseminen.

  Nämä kuuluvat korjauskokonaisuuteen KK-C12-2 (13.3).

**Rekisterien johdonmukaisuus.** Tarkistettu koneellisesti.

- Luvun 3.1–3.2 arvosarakkeiden 29 makroa vastaavat `rekisteri/valitsimet.csv`:tä.
- Luokkamäärät (435) ja `rekisteri/asetusavaimet.csv`:n 750 riviä vastaavat lukua 1.
- Rekisterit tuotetaan raakatuloksista skriptillä (`tyokalut/valitsinrekisteri.py`), ja A0- ja
  B0-tulosteet syntyvät uudelleen tavulleen samoina.

**Arvioijan artefaktit.** Lausekkeella määritellyt makrot näkyvät arvolla 1, koska leksikaalinen
arvioija ei laske lausekkeita. Ehtona testattuja tällaisia ovat esimerkiksi `LENIMI`, `LSNIMI`,
`KSEK` ja `LANG` (22 makroa, joista 13 on testikehyksen `DOCTEST_*`-makroja). Sama koskee C-10:n
`makrot.csv`:tä: siinä viestin `MAX_PORTTI=1`, vaikka todellinen arvo on 64 + 2·32 + 4 = 132
(`Juk/VDef.h:62`). Johtopäätöksiin tämä ei vaikuta, sillä H-171:ssä arvo luettiin lähteestä ja
objektitiedostosta.

**Matriisien kattavuus.**

- Ominaisuusmatriisi (luku 9) kattaa toimitettavat kokoonpanot ja 20 ominaisuutta. Toimittamattomat
  kokoonpanot ovat luvussa 3.3 ja T-051:ssä.
- Testimatriisi (luku 10) kattaa kaikki rajauksen vaatimat rivit:
  - normaalit käyttökokoonpanot T1–T4, T6 ja T7;
  - merkittävät ohjelmakohtaiset erot T15–T17;
  - rajauksessa nimetyt vikoja aktivoivat yhdistelmät: H-149 ja H-150 (T6), H-129 (T8), H-159 (T4),
    H-160 ja H-161 (T7), H-055 ja H-166 (T9);
  - C-12:n omat havainnot T5 ja T10–T14 sekä H-036 (T18).
- **Testimatriisia ei ole ajettu.** Mikään sen rivi ei ole todennettu tulos.

**Kuollut koodi.** Kuolleiden haarojen, käyttämättömien valitsinten tai ominaisuuksien poistamisesta
ei ole tehty päätöksiä. Ne odottavat omistajan päätöstä (Q111) ja C-13:n korjaussuunnitelmaa.

/ All criteria met; phase C item 3 done for switches; Q2 resolved; T-008 needs in T-051/KK-C12-2;
registers checked against the report; expression-valued macros show as 1 (incl. C-10 `MAX_PORTTI`);
matrices cover the required rows; the test matrix has not been run; no removal decisions.

### 13.2 H-171:n vakavuus / severity of H-171

| Peruste / Criterion | Havainto / Finding | Näyttö |
|---|---|---|
| Puskurin koko / buffer size | `static wchar_t cfgfilename[41]`: 41 merkkiä = 82 tavua (bcc32:ssa `wchar_t` on 2 tavua) | `Juk/VInit.cpp:74` |
| Kirjoitettava pituus / length written | `wcsncpy(..., cfgflname, 120)` kirjoittaa aina 120 merkkiä = 240 tavua (lähde ja nollatäyte), eli 79 merkkiä = 158 tavua yli puskurin, polun pituudesta riippumatta; yli 119 merkin polulla tulos jää lisäksi päättämättä | `Juk/VInit.cpp:651-653` |
| Sijainti / location | tiedoston staattinen, alustettu data (`_DATA`). Nykyisen lähteen bcc32-objektissa (`RADStudio10/Release_Build/obj/VInit.obj`, käännetty 2026-11-24 muuttumattomasta lähteestä) `L"LASKENTA.CFG"` on siirtymässä 0xA ja `baudst`in alkuarvo siirtymässä 0x98. Väli vastaa lähdejärjestystä `cfgfilename`, `autostr[25]`, `korkeat`, `star`, `baudst`, kun int-muuttujat tasataan 4 tavuun | `Juk/VInit.cpp:74-78`; objektin `_DATA`-alkuarvot (LEDATA), luettu skriptillä, ei ajettu |
| Ylikirjoitettava alue / overwritten | siirtymät 0xA–0xF9 kokonaan: `autostr` (0x5C–0x8D), `korkeat` (0x90), `star` (0x94), `baudst[0]` ja `baudst[1]` sekä 9 merkkiä `baudst[2]`:sta (`baudst` on `MAX_PORTTI` × 20 = 132 × 20 merkkiä) | `Juk/VDef.h:62` |
| Seuraukset / consequences | Lyhyellä polulla alueelle tulee nollia. Porttien 1–3 oletus `9600,n,8,1` häviää, sillä `asetaOletusarvot` kopioi sen kaikille porteille ennen `lue_parametrit`ia. Sarjayhteys ilman `BAUDn=`-avainta jää näillä porteilla numeeristen oletusten varaan [päätelty]. Polun merkit päätyvät dataan seuraavasti: merkki 65 `autostr[24]`:een, jolloin arvo `'1'` ohjaa GUI:n listauksen tulostimelle (`:2685`); merkit 71– `baudst[0]`:aan. `korkeat` ja `star` ovat kuolleita. Ajonaikaiset vaikutukset ovat [tuntematon] (Q107) | `Juk/VInit.cpp:2460-2506`, `:2570`, `:2601`, `:2685` |
| Saavutettavuus / reachability | Jokainen ViestiWinin kilpailun avaus: `Initialisoi` (avausdialogi `ViestiWin/UnitMain.cpp:1403`, `:1426` tai `Init.cfg`:n `CFG=` `:1542`) → `aloitus(0, 0, cfgflnm)` (`:220`) → `lue_parametrit`. Nimi on aina ei-tyhjä täysi polku. Konsoliohjelmat kutsuvat polkua `cfgflname = NULL`/`0`, joten ne eivät ylitä. HK:ssa puskuri on 201 merkkiä | `ViestiWin/UnitMain.cpp:192-220`; `Hk/HkInit.cpp:59` |
| Toistuvuus / recurrence | Joka kerta, kun ViestiWin avaa kilpailun, myös uudelleenavauksessa. Ylikirjoitus on deterministinen; uhrit riippuvat kääntäjän sijoittelusta ja muuttuvat, kun tiedostoon lisätään muuttujia | – |

**Arvio.** Kyseessä on normaalissa käytössä toistuva staattisen muistin ylikirjoitus (158 tavua),
joten vakavuus on **korkea** ja prioriteetti **P2**. P1 ei ole perusteltu, koska ylitystä ei voi
laukaista verkosta: syöte on paikallisen operaattorin valitsema tiedostopolku.

Ajonaikaista kaatumista ei edellytetty eikä väitetä. Ylitys, sen koko ja polku on todennettu
lähteestä ja sijoittelu olemassa olevasta objektitiedostosta; ajonaikaiset seuraukset ovat
avoimena kysymyksenä Q107.

/ A recurring 158-byte overwrite of static data on every ViestiWin competition open: **high, P2**
(not P1: not triggerable from the network). The layout was read from the existing object file,
not from a run; run-time effects are open (Q107).

### 13.3 Havaintojen kytkökset ja korjauskokonaisuudet / Links and fix packages

**H-134.** H-134 on kytketty seuraaviin:

- vaiheen vaihdon havainnot: H-129, joka on samassa `vaihdavaihe`-polussa, sekä H-131–H-133;
- globaali tila ja nollaus: T-038 ja T-039 (C-8);
- virheenkäsittely: KK-C9-2, jossa virhe ilmoitetaan mutta tila jatkuu;
- muistirakenteiden elinkaari: C-4:n vapautetun muistin käytön luokka.

H-134 korjataan vaiheen vaihdon kokonaisuudessa yhdessä H-129:n kanssa (`rekisteri/havainnot.md`).

**Korjauskokonaisuudet** C-13:n syötteeksi; mitään ei ole korjattu:

| Tunnus | Sisältö / Content | Havainnot |
|---|---|---|
| KK-C12-1 | XML-, HTML- ja automaattiset julkaisutiedostot / XML, HTML and automatic publication files | H-172, H-149 ja H-150 (myös KK-C9-1:n `PRFILE`-elinkaari), H-176:n HTML-pohjaosa |
| KK-C12-2 | Asetusten hallinta: jäsentimet, editorit, kokoonpanot / settings management | H-171, H-173, H-036, H-176:n asetusosat, T-051 |
| KK-C12-3 | Valitsinhygienia ja kuollut koodi / switch hygiene and dead code | T-050, T-052, H-177; ei poistopäätöksiä, vaatii omistajan päätöksen (Q111) |

**Prioriteetit** [päätelty]:

- **P2:** H-171.
- **P3:** H-134 (yhdessä H-129:n kanssa), H-172, H-173, H-174.
- **P4:** H-175.
- **P5:** H-176, H-177.

Rekisteriin jäävät H-171–H-177, T-050–T-052 sekä laajennetut H-036, H-134 ja T-008.

### 13.4 Avoimet kysymykset / Open questions

- **Q107, Q108 ja Q110** on siirretty myöhempään ajonaikaiseen todentamiseen. Ne ajetaan vain
  erillisellä luvalla.
- **Q109** jää avoimeksi staattisena kysymyksenä.
- **Q111** odottaa omistajan päätöstä. Sitä ei ratkaista oletuksilla. Avoimia osia on kolme:
  - onko konsolin seuratulosten (`SEURATUL`, `ORAVA`) poistuminen tarkoituksellinen;
  - tarvitaanko ViestiLuennassa RFID-tukea;
  - saako kuolleet ominaisuusvalitsimet poistaa.
- **Q112** jää avoimeksi. Havaittu ero säilyy: HK kertoo arvon `SEK / 100`
  (`Hk/HkInit.cpp:1464`), viesti `SEK` (`Juk/VInit.cpp:1693`).
  - Käyttöohjeet eivät kerro yksikköä. Molemmissa ohjeissa avain on muodossa `AIKAKORJAUSy=xx`
    (`docs-site/docs/hkkisawin/hkliite1.md:130`, `docs-site/docs/viestiwin/hkliite1.md:92`).
  - Repossa ei ole esimerkkiasetuksia, joissa avain esiintyisi.
  - Git-historiassa on vain vuoden 2015 tuonti (`50db2f8`).
  - HK:n konsolin interaktiivinen kysymys "(1/100 s)" (`HkConsole/HkCons2.cpp:1465`) koskee eri
    syöttöpolkua, eikä se ole yksiselitteinen vastaus.
- Ohjelmien toimintaa ei muutettu.

/ Q107, Q108, Q110 moved to later run-time verification; Q111 is the owner's decision; Q112 stays open
because the manuals, examples and history give no unambiguous unit.
