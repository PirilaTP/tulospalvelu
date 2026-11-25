# Kääntäjäkohtainen käytös / Compiler-specific behaviour (C-5)

Erä C-5, vaihe 1. Analysoitu revisio `1457e26` (`TPsource/` ennallaan revisiosta
`f7fe553`). Polut ovat suhteessa hakemistoon `TPsource/V52/`. Raakatulokset ja skriptit:
`todisteet/c5/`.

Batch C-5, step 1. Analysed revision `1457e26` (`TPsource/` unchanged since `f7fe553`).
Paths are relative to `TPsource/V52/`. Raw results and scripts: `todisteet/c5/`.

Merkinnät / labels: **[mitattu]** koeohjelma, käännöstulos tai binäärin tavut /
probe, build output or binary bytes; **[staattinen]** pääteltyä lähdekoodista /
inferred from source; **[todentamaton]** avoin riski / open risk.

## 1. Yhteenveto / Summary

Kääntäjien väliset erot eivät muuta levylle tai verkkoon kirjoitettavia rakenteita:
kentät ovat samoissa kohdissa MSVC:llä ja bcc32:lla, eikä yhdessäkään raakana
kirjoitettavassa rakenteessa ole `time_t`-, `long double`-, enum-, `bool`- tai
bittikenttäjäsentä. Käännösyksiköiden väliset asetteluerot ovat bcc32:lla
piileviä, eivät aktiivisia. Merkittävät erot ovat ajonaikaisia:

1. **Merkistö:** bcc32 lukee BOMittoman UTF-8:n samoin kuin MSVC, ANSI-koodisivuna.
   `merkistot.md`:n aiempi väite oli väärä. UTF-8-tiedostoissa on 36 ei-ASCII-
   merkkijonoa, joista 34 käännetään; ne ovat rikki kaikissa viidessä ohjelmassa,
   myös GUI-ohjelmissa (H-005, H-004). BOM korjaa molemmat kääntäjät (Q7 ratkaistu).
2. **Konsoliohjelman kaatuminen:** `TextFl` avaa UTF-16BE-tiedoston tilalla, jota
   MSVC:n CRT ei hyväksy, ja prosessi päättyy (H-078).
3. **Pieniä toiminnallisia eroja:** pinon ylivuoto `TextFl::Open`issa (H-079),
   32-bittinen `time_t` GUI:ssa (H-080), kuntosuunnistustilan emit-koodin vaihto
   vain GUI:ssa (H-081), `VersioInt` 520/521 (T-025), viestin `NREGNLY` 8/32 (T-024).

Compiler differences do not change any on-disk or on-wire structure: member offsets
match on MSVC and bcc32, and no raw-written struct has a `time_t`, `long double`,
enum, `bool` or bitfield member. Cross-unit layout differences in bcc32 are latent,
not active. The significant differences are at run time: bcc32 reads UTF-8 without a
BOM as the ANSI code page just like MSVC (the earlier claim in `merkistot.md` was
wrong; of 36 non-ASCII literals in UTF-8 files 34 are compiled and broken in all five
programs, GUI included; a BOM fixes both compilers);
MSVC console programs abort on a UTF-16BE text file (H-078); smaller differences
H-079, H-080, H-081, T-024, T-025.

| Tunnus / ID | Lyhyesti / In brief | Vakavuus / Severity |
|---|---|---|
| H-078 | `ccs=UTF-16BE` → MSVC-konsoliohjelma päättyy (0xC0000409) / MSVC console aborts | keskitaso / medium |
| H-005 (laajennettu / extended) | UTF-8-merkkijonot väärin kaikissa ohjelmissa, 34 käännettyä / UTF-8 literals wrong in all programs, 34 compiled | matala / low |
| H-004 (laajennettu / extended) | U+FFFD-merkkijonot väärin millä tahansa kääntäjällä, 6 kpl / wrong with any compiler | matala / low |
| H-079 | `TextFl::Open`: `wchar_t mode[16]` ylivuotaa 1–2 merkkiä / overflows | matala / low |
| H-080 | GUI: vuoden 2038 jälkeinen päivämäärä tallentuu nollana / post-2038 date stored as 0 | matala / low |
| H-081 | `KUNTOSUUNN`: emit-koodin vaihto vain GUI:ssa (`_BORLAND_` jaetussa ytimessä) / badge swap GUI-only | matala / low |
| T-023 | bcc32: sama tyyppi `-a8`:lla ja `-a1`:llä eri yksiköissä (46 tyyppiä), piilevä / latent cross-unit layout mismatch | matala / low |
| T-024 | Viestin `NREGNLY` 8 (konsoli) vs 32 (ViestiWin): laitenumerot ja `kanava` / relay device numbering | matala / low |
| T-025 | `VersioInt()` 521 (MSVC) vs 520 (bcc32) / version tag differs | matala / low |
| T-026 | Ajonaikaisen kirjaston semanttiset erot / RTL semantic differences | matala / low |
| T-027 | Merkistö riippuu käännöskoneen koodisivusta; auditointityökalun bugi korjattu / encoding depends on host code page; tool bug fixed | matala / low |
| T-028 | `#ifdef CONSOLE` ilman alaviivaa / without underscore (`Hk/HkIV.cpp:1637`) | matala / low |

Vaihe 2 (luku 13) lisäsi H-082–H-086 ja T-029, laajensi H-042:ta ja H-079:ää ja ratkaisi
Q8, Q57–Q59, Q61 ja Q62. / Step 2 (section 13) added H-082–H-086 and T-029, extended
H-042 and H-079, and resolved Q8, Q57–Q59, Q61 and Q62.

## 2. Menetelmä / Method

| Osa / Part | Menetelmä / Method | Tekijä / By |
|---|---|---|
| MSVC-asettelu / layout | `/Zs /d1reportAllClassLayout` jokaiselle 251 käännösyksikölle C-4:ssä tallennetuin komentorivein (HkMaali, JukMaali, JukLuenta, tputilv2, dbboxm Release); vertailu ohjelmittain (projekti + `tputilv2.lib` + `dbboxm.lib`, linkityskomennoista) / every unit with the recorded C-4 command lines; compared per program | pääistunto / main session |
| Optiomatriisi, ehdollinen koodi, bcc32-pakkaustila / option matrix, conditional code, bcc32 pack state | Tallennetut bcc32- ja CL-komentorivit; `cpp32` kaikille 405 GUI-käännösyksikölle, pragmatilan jäljitys, `bcc32 -S` -koetin ehdokastyypeille / recorded command lines; `cpp32` for all 405 GUI units, pragma state trace, `bcc32 -S` ground-truth probes | `build-dependency-analyst` |
| Tyypit, muotoilu, merkistö, RTL / types, formatting, encoding, RTL | Koeohjelmat molemmilla kääntäjillä projektien asetuksin, toimitettujen binäärien tavut / probes on both compilers, bytes of shipped binaries | `legacy-encoding-analyst` |
| Riippumaton tarkastus / independent review | Kumoamisyritys väitteittäin C1–C10, omat koeohjelmat, `tdump` toimitetuista objekteista / refutation attempt per claim, own probes, `tdump` of shipped objects | `independent-verifier` |

Toimitettuja ohjelmia ei ajettu, verkkokokeita ei tehty, tuotantokoodia ja
projektiasetuksia ei muutettu. / Shipped programs were not run, no network tests,
no production code or project changes.

## 3. Käännösasetukset / Build settings

Toimitettavat Release-kokoonpanot, tallennetuista komentoriveistä [mitattu: komentorivit;
bcc32:n oletukset `bcc32 -h`:sta ja koettimista]. / Shipped Release configurations,
from the recorded command lines.

| Asetus / Setting | MSVC (HkMaali, ViestiMaali, ViestiLuenta, tputilv2, dbboxm) | HkKisaWin | ViestiWin | Tputil-XE | DBboxm-XE |
|---|---|---|---|---|---|
| `char` | etumerkitön / unsigned (`/J`) | **etumerkillinen** `tputil.h:28`:aan asti / **signed** until `include/tputil.h:28` | `-K` | `-K` | `-K` |
| Tasaus komentorivillä / alignment | 8 | `-a8` (oletus / default) | `-a8` | `-a8` | **`-a1`** |
| Tasaus `tputil.h`:n jälkeen / after `tputil.h` | 8 (pragma vain bcc32:lle / bcc32 only) | 1, ei palauteta / never restored | 1 | 1 | 1 |
| enum | `int` | `int` | **pienin / minimum (`-bi-`)** | **`-bi-`** | `int` |
| `wchar_t` | 2 | 2 | 2 | 2 | 2 |
| `TCHAR` | `wchar_t` (dbboxm: `char`, `_MBCS`) | `wchar_t` (`-tU`) | `wchar_t` | `wchar_t` | `char` |
| `time_t` | 64 bittiä / bits | **32** | **32** | **32** | **32** |
| `long double` | 8 | 10 | 10 | 10 | 10 |
| Ajonaikainen kirjasto / RTL | `/MT` staattinen / static | `-tR` dynaaminen + paketit / dynamic + packages | `-tR` | `-tR` | `-tR` |
| `_BORLAND_` | ei / no | kyllä / yes | **ei / no** | kyllä / yes | kyllä / yes |

Saman toimialueen konsoli- ja GUI-käännöksen erot jaetuissa ydintiedostoissa
(`Hk/`, `Juk/`, `Tp/`) ovat ominaisuusvalitsimia (T-008): HK:n konsolissa `GAZ`,
`AIKAVERT`, `SERVER`, `ENNAKOI`, GUI:ssa `JOUKKUETUL`, `DBGFILE`, `_BORLAND_`; viestissä
`NREGNLY` 8 vs 32 (T-024), `EMITFILE` vain ViestiMaalissa, `LUENTA` vain ViestiLuennassa.
Taulukko: `todisteet/c5/bda.tgz` (`tbl5_defines_console_vs_gui.csv`).

Console and GUI builds of the same domain differ in feature switches in the shared
core files (T-008); table in `bda.tgz`.

`_MSC_VER` esiintyy vain `Tests/doctest.h`:ssa. `__BORLANDC__`-lohkoista vain kaksi
muuttaa käytöstä: `include/tputil.h:27-29` (pragma, luku 4) ja `include/tputil.h:54-66`
(`wcstok`-makro, T-026). `_BORLAND_`-lohkoista käytöstä muuttaa vain `Hk/HkIV.cpp:1663-1673`
(H-081). / Only these compiler- or project-conditional blocks change behaviour; the
rest are includes, pragmas and API name substitutions (`tbl3_compiler_conditional.csv`).

## 4. Rakenteiden asettelu / Struct layout

### 4.1 Levylle ja verkkoon / On disk and on the wire

C-3 mittasi kokonaiskoot (`tiedostomuodot.md`, "Mitatut koot"). C-5 vertasi jokaisen
jäsenen offsetin: `aikatp`, `emittp`, `emitvatp`, `combufrec` kaikkine unionijäsenineen,
MSVC `/J` vs bcc32 HkKisaWinin ja ViestiWinin asetuksin: identtiset (HK 45 riviä,
viesti 32 riviä) [mitattu]. Lisäksi tarkastaja listasi kaikki repon tyypit, joihin
MSVC lisää täytettä (`TextFl`, `xml_node`, `comparamtp`, `sendtestparamtp`,
`C_san_type`, `autofileparam`, `llparamtp`, `TauluParamTp` ym.); mitään niistä ei
kirjoiteta raakana eikä lähetetä [mitattu + staattinen]. Rajaus: lista kattaa
konsolikäännösten tyypit; vain GUI:ssa olevia tyyppejä ei käyty läpi näin
[todentamaton].

C-3 measured whole sizes; C-5 compared every member offset of the raw-written and
network structs on both compilers: identical. The reviewer listed every repo type MSVC
pads; none is written raw or sent. GUI-only types were not checked this way.

### 4.2 MSVC: käännösyksiköiden välillä / across translation units

Kolmessa konsoliohjelmassa yhdenkään repon tyypin asettelu ei eroa käännösyksiköiden
välillä [mitattu]. Vertailu tehtiin kahdesti: avaimena nimi + jäsenluettelo
(`compare_sets.out`) ja tarkastajan toimesta pelkkä nimi kääntäjävaroitusten
poistamisen jälkeen. Jäljelle jääneet erot ovat yksikön omia paikallisia tyyppejä
(`Tp/TpComY.cpp:154` `Y_san_type` 218 tavua vs `Hk/HkDef.h`:n `r12` 217: sama
jäsenten sijainti, vain loppupehmuste eroaa; `Hk/HkCom32.cpp:112` `C_san_type` 268)
tai raportin tulkinta-artefakteja (varoitusrivit raportin keskellä). Raportin
mahdolliset puutteet (paikalliset luokat, käyttämättömät mallit) ovat Q61.

No repo type has a different layout in different units of the three console
programs. The remaining differences are unit-local types or report-parsing artefacts.

### 4.3 bcc32: `-a1`/`-a8` käännösyksiköiden välillä / across units (T-023)

`include/tputil.h:27-29` asettaa `#pragma option -K -a1` eikä palauta sitä. Jos
tyypin otsikko käsitellään ennen `tputil.h`:ta, tyyppi saa tasauksen 8, muuten 1.
VCL-lomakkeen otsikko käsitellään omassa `.cpp`:ssään ennen `tputil.h`:ta ja muissa
yksiköissä sen jälkeen. [mitattu: `cpp32` 405 yksiköstä + `bcc32 -S` -koettimet]

| Ohjelma / Program | Ehdokkaita / candidates | `sizeof` eroaa / differs | Jäsenten offsetit eroavat / member offsets differ |
|---|--:|--:|---|
| HkKisaWin | 45 | 25 (23 VCL-lomaketta / forms, `comparamtp` 16/13, `sendtestparamtp` 464/461) | `TFormEmitMuutokset::dKilp` (`cbHk/UnitEmitMuutokset.h:103`), `TFormNappain::MaxVa` (`cbTp/UnitNappain.h:54`), `sendtestparamtp` |
| ViestiWin | 37 | 21 VCL-lomaketta / forms | `TFormNappain` |

Miksi vaikutus on piilevä [mitattu + staattinen, riippumattomasti tarkastettu]:

- Jokaisen lomakkeen VMT määritellään vain lomakkeen omassa (`-a8`) yksikössä;
  `InstanceSize` on `-a8`-koko + 4 (esim. `TFormEmitMuutokset` 0x4F4 = 1264 + 4).
  Toinen yksikkö luo lomakkeen tämän VMT:n kautta (`UnitEmit.obj`), joten varattu
  olio on aina suurempi asettelu, eikä kutsujan `sizeof` vaikuta. [mitattu: `tdump`]
- Muista yksiköistä luetut jäsenet (`Arvo`, `Aika`, `FileName`, `Vastaus`, `Sanoma`,
  `Optiot`, `Valittu`, `KeyCode`, `KeyShift`, `nMukana`, `NimiHaku`) ovat samassa
  kohdassa molemmissa asetteluissa; `UnitEmit.obj` kirjoittaa `NimiHaku`n offsettiin
  0x4E4 kuten lomakkeen oma yksikkö. `dKilp`, `MaxVa` ja `Piste` ovat vain omassa
  yksikössään. [mitattu + staattinen]
- `comparamtp` ja `sendtestparamtp` ovat `-a8`:lla vain yksikössä `Hk/HkEnnTav.cpp`,
  joka ei käytä niitä. `ipparamtp` on 144 tavua samoin offsetein kummallakin.
- RTL-otsikot suojaavat itsensä (`#pragma option push -a8`, `pack(push,_CRT_PACKING)`).

Riski: jos joku lisää lomakkeen otsikkoon jäsenen, jota käytetään toisesta
yksiköstä, tai muuttaa include-järjestystä, ero muuttuu aktiiviseksi ilman
varoitusta. Sama koskee `char`-etumerkkiä (luku 6). Q62: esikäännettyjen otsikoiden
(`-H`) vaikutus todennettiin vain toimitettujen objektien tavuista.

`tputil.h` sets `-a1` and never restores it, so a header parsed before it gets
alignment 8 and after it 1. HkKisaWin has 25 and ViestiWin 21 types whose size
differs between units. The effect is latent: each form's VMT (and `InstanceSize`) is
defined only in its own `-a8` unit, so allocation always uses the larger layout, and
every member touched from other units has the same offset in both layouts (checked
in object code). It becomes active if someone adds such a cross-unit access or
changes include order.

## 5. Merkistö / Encoding (Q7)

[mitattu: koeohjelmat `cp1252`, UTF-8 ilman BOMia, UTF-8 BOMilla; molemmat kääntäjät
projektien asetuksin; toimitettujen objektien ja ohjelmien tavut]

| Tiedoston merkistö / File encoding | Kapea / narrow `"ä"` | Leveä / wide `L"ä"` | `'ä'` | MSVC = bcc32? |
|---|---|---|---|---|
| cp1252 | `E4` | `00E4` | `E4` | kyllä / yes |
| UTF-8 ilman BOMia / without BOM | `C3 A4` | `00C3 00A4` (rikki / mojibake) | monimerkkivakio / multichar `0xC3A4` | kyllä / yes |
| UTF-8 BOMilla / with BOM | `E4` | `00E4` | `E4` | kyllä / yes |

- **Q7 ratkaistu:** BOM toimii bcc32:lla kapeille ja leveille merkkijonoille sekä
  päätiedostossa että sisällytetyssä otsikossa. / A BOM works on bcc32 too.
- **`merkistot.md`:n aiempi johtopäätös oli väärä.** `Tp/TpLaitteet.cpp:2581` on
  `#ifdef _CONSOLE` -lohkossa (rivit 2505–2954), joten bcc32 ei käännä sitä. GUI:n
  oikea "Asema lähettää" tulee cp1252-tiedostosta `cbTp/UnitEmiTag.cpp:168`.
  / The earlier evidence was misattributed.
- **Auditointityökalun bugi:** `tyokalut/utf8_literaalit.py` hylkäsi kirjaimen `n`
  (`[^"\\n]` raw-merkkijonossa), ohitti escape-merkit ja tulosti vain kolme osumaa
  tiedostoa kohti. Korjattu C-5:ssä; korjattu ajo löytää 36 merkkijonoa
  (`todisteet/c5/utf8_literaalit_c5.out`). / Tool bug fixed; 36 literals.

Rikkinäiset merkkijonot ohjelmittain [mitattu: väärin tulkittu merkkijono haettu
`TPexe/`-hakemiston toimitetuista ohjelmista, `todisteet/c5/litexe_main.out`] /
broken literals per shipped program (searched in the `TPexe/` binaries):

| Tiedosto / File | Rivit / Lines | HkMaali | ViestiMaali | ViestiLuenta | HkKisaWin | ViestiWin |
|---|---|---|---|---|---|---|
| `Hk/HkAjat.cpp` (U+FFFD, H-004) | 191 | X | – | – | – | – |
| `Hk/HkAjat.cpp` (U+FFFD, H-004) | 234, 235 | X | – | – | X | – |
| `Juk/vajat.cpp` (U+FFFD, H-004) | 155, 191, 193 | – | X | – | – | X |
| `Juk/vajat.cpp` (ä) | 417 | – | – | – | – | X |
| `Tp/TpLaitteet.cpp` | 1548, 1874, 1892, 1948 | X | X | X | X | X |
| `Tp/TpLaitteet.cpp` | 2223 | X | X | – | X | X |
| `Tp/TpLaitteet.cpp` | 1914 (`EMITFILE`) | X | X | – | X | – |
| `Tp/TpLaitteet.cpp` (`_CONSOLE`) | 2523–2859 (17 kpl) | X | X | – | – | – |
| `Tp/TpLaitteet.cpp` (`_CONSOLE`) | 2891, 2909, 2914, 2943 | X | X | X | – | – |
| `Tp/TpLaitteet.cpp` (`DBGEMIT`, ei määritelty / undefined) | 1178, 1280 | – | – | – | – | – |

Yhteensä 34 käännettyä merkkijonoa, joista 6 on U+FFFD-merkkijonoja; kaksi on kuollutta
koodia. Kaikissa viidessä ohjelmassa on rikkinäisiä tekstejä. / 34 compiled literals
(6 with U+FFFD), 2 in dead code; all five programs are affected.

U+FFFD-merkkijonot ovat väärin millä tahansa kääntäjällä: lähdetiedostossa on tavut
`EF BF BD`, joten edes oikea UTF-8-tulkinta ei palauttaisi alkuperäistä "Ä":tä.
Alkuperäiset tekstit ovat commitissa `50db2f8`. Korjaus vaatii siis tekstien
palauttamisen ja joko BOMin tai cp1252-tallennuksen kaikille kolmelle tiedostolle.
Ei-ASCII-merkkivakioita ei ole BOMittomissa UTF-8-tiedostoissa; cp1252-tiedostojen
149 merkkivakiota ovat oikein molemmilla kääntäjillä, kun `char` on etumerkitön.

U+FFFD literals are wrong with any compiler. A fix must restore the texts from
`50db2f8` and add a BOM or re-save as cp1252. No non-ASCII char constants exist in
UTF-8 files without a BOM.

Tarkennus / nuance (T-027): kumpikaan kääntäjä ei lue BOMitonta tiedostoa kiinteänä
cp1252:na vaan käännöskoneen ANSI-koodisivuna; projekteissa ei ole `/utf-8`:aa
[staattinen; mitattu vain koodisivulla 1252].

## 6. `char`-etumerkki / signedness

HkKisaWinissä ei ole `-K`:ta. 128 käännösyksikköä on etumerkillisiä `tputil.h:28`:aan
asti, 13 yksikköä ei sisällytä `tputil.h`:ta lainkaan. Kaikissa niissä ennen pragmaa
on vain esittelyjä, eikä yhdessäkään ole etumerkistä riippuvaa repon koodia [staattinen,
riippumattomasti tarkastettu]. **Nykyistä käytöksen eroa ei ole.** Piilevät kohdat,
jotka toimivat vain `/J`:n tai `-K`:n ansiosta (T-023): `Hk/HkInit.cpp:141-211`
(`utrlate['Ä'] = 91`: etumerkillisenä indeksi −60), `Juk/VInit.cpp:116-423`,
`HkConsole/HkCons0.cpp:113-117`, `tputilv2/scanconv.cpp`, `tputilv2/upcasesc.cpp:28-32`.
`TpTest.vcxproj`:sta puuttuu `/J`, mutta sen tiedostoissa ei ole etumerkkiriippuvuutta.
VCL/RTL-otsikoiden inline-koodia ei tutkittu (Q62).

No current difference; the listed sites work only because of `/J` or `-K`.

## 7. Ajonaikaiset erot / Run-time differences

| # | Kohta / Site | MSVC | bcc32 | Seuraus / Consequence | ID |
|---|---|---|---|---|---|
| 1 | `tputilv2/TextFileClass.cpp:301-303`, `:357-359` `ccs=UTF-16BE` | virheellinen parametri → 0xC0000409 / invalid parameter → fast fail [mitattu] | avaa tiedoston, ohittaa / opens, ignores [mitattu] | Konsoliohjelma kaatuu, jos mikä tahansa luettava tekstitiedosto alkaa `FE FF`: tunnistus `:166`, `:269-279`; 68 lukukohtaa, mm. käynnistyksen asetustiedosto `Hk/HkInit.cpp:296`, `:339`, `Juk/VInit.cpp:441`, sarjatiedosto `Hk/HkSrj.cpp:76` / console aborts on a UTF-16BE file, also at startup | H-078 |
| 2 | `tputilv2/TextFileClass.cpp:347-359` `wchar_t mode[16]` | `"wb, ccs=UTF-16LE"` 17, `"r+b, …"` 18 merkkiä → pinon ylivuoto; /GS havaitsee (0xC0000409) HkMaalissa ja ViestiMaalissa [mitattu, vaihe 2] / stack overflow, caught by /GS | korvaa `Open`in tallennetun EBP:n 2 tai 4 tavua → kaatuminen paluun jälkeen [mitattu koeohjelmassa, vaihe 2] / overwrites the saved EBP | Laukeaa, kun olemassa oleva tiedosto on tunnistettu UTF-16:ksi ja `Open` kutsutaan uudelleen: `HkConsole/HkCons3.cpp:1564`, `cbHk/UnitSakkoKierr.cpp:47`, `:201`, `:235`, `ViestiWin/UnitSakkoKierr.cpp:48`, `:226`, `:264` / when a UTF-16 file is re-opened | H-079 |
| 3 | `Tp/TpApu.cpp:98-123` `date_time_t` | 64-bittinen / 64-bit | `mktime` vuodelle 2040 → −1 → 0 [mitattu]; `time_t_date` toimii vuoteen 2106 [mitattu] | Aikatietueen päivämääräkenttään GUI:ssa syötetty vuoden 2038 jälkeinen päivä tallentuu nollana ja näkyy 1.1.1970 (`cbHk/UnitAjanotto.cpp:523`, `:627`); tekstimuotoinen aikatuonti korvaa sen tämän päivän päivämäärällä (`Hk/HkAjat.cpp:494-496`, `Juk/vajat.cpp:471`). Kilpailun päivämäärä (`DateNo`) ei käytä `time_t`:tä / post-2038 date in GUI time record stored as 0 | H-080 |
| 4 | `Tp/TpApu.cpp:66-69` `VersioInt()` | 521 | 520 (x87-tarkkuus, laskettu ajossa / x87 precision, computed at run time) [mitattu: koetin + `TpApu.obj`] | Eri versiotunniste: `Hk/HkIV.cpp:4116` (`AJAT_V…`), `cbHk/WinHk.cpp:198`, `Hk/HkInit.cpp:3166`. Lukijat vertaavat vain 6 merkkiä (`Hk/HkIV.cpp:4026`, `Juk/VIv.cpp:2701`) tai ohittavat arvon (`Hk/HkSrj.cpp:111-114`, `Hk/HkXml.cpp:434`, `Juk/VXml.cpp:437`), joten yhteensopivuus ei rikkoudu. Viestin "5.20" antaa 520 molemmilla / tag differs, no reader depends on it | T-025 |
| 5 | `swprintf(buf, n, …)` katkaisu / truncation | päättää `buf[n-1]` / terminates | **ei päätä** / does not terminate [mitattu] | GUI:ssa 29 laskurimuotoista kutsua + 1 `swprintf_s` (vaiheen 1 lista "38" oli väärä); vain 5 voi katketa: virheilmoitukset `Hk/HkTulostus.cpp:1486`, `:1528`, `Juk/VTulostus.cpp:1372`, `:1394`, `:1410` (polku ≥ 161–166 merkkiä ja kirjoitusvirhe) → `writeerror_w` lukee pinoa yli ja näyttää roskaa (vaihe 2, luku 13) / only 5 sites can truncate; stack over-read in the error dialog | T-026 |
| 6 | `_wtoi`, `_wtol`, `atoi` ylivuoto / overflow | rajaa / saturates | pyörähtää / wraps; `wcstol` → 0 [mitattu] | GUI hyväksyy ylipitkästä syötteestä väärän arvon (esim. `cbHk/UnitImport.cpp:657`) / GUI accepts bogus values | T-026 |
| 7 | `iswalnum` C-lokaalissa / in C locale | `ä` on kirjain / alnum | `ä` ei ole / not alnum [mitattu] | `htmlid` (`Hk/HkMuotoilu.cpp:758`, käyttö `Hk/HkTulostus.cpp:4991`) tuottaa eri HTML-tunnisteen konsolissa ja GUI:ssa; saman tulosteen linkit pysyvät yhtenäisinä / different HTML ids | T-026 |
| 8 | `cbHk/UnitSeurat.cpp:442` `_wsetlocale(LC_CTYPE, L"fi-FI")` | toimii / applies | palauttaa "C", ei vaikutusta / no effect [mitattu] | Ei näkyvää vaikutusta (vertailu omalla funktiolla) / no visible effect | T-026 |
| 9 | `wcstok` 3 argumentilla / 3-arg (`include/tputil.h:54-66`, `tputilv2/atold.cpp:20`, `tputilv2/DateNo.cpp:20`, `tputilv2/IntInList.cpp:19`) | konteksti kutsukohtainen / per-call | konteksti pudotetaan; tila säiekohtainen / dropped; per-thread [mitattu, staattinen ja dynaaminen RTL / static and dynamic RTL, vaihe 2] | Ei lomittaista käyttöä löytynyt (6 ehdokasta luettu) / no interleaved use found | T-026 |

`printf`-perheen muotoilut (`%s`, `%S`, `%hs`, `%ls`, `%c`, `%C`) toimivat samoin
molemmilla kääntäjillä ilman `setlocale`-kutsua; muotoiluvirheet (C4477) tuottavat
saman roskan kummallakin. Kokonaisluvut, `bool`, `wchar_t`, `long`, `__int64` ja
negatiivisten jako ja siirto ovat samat. / printf-family formats, fundamental type
sizes and negative division/shift behave the same on both compilers.

## 8. Muut erot / Other differences

- **H-081 `KUNTOSUUNN`:** `Hk/HkIV.cpp:1663-1673` (`#if defined(_BORLAND_)`) vaihtaa
  kuntosuunnistustilassa kilpailijalle seuraavan vapaan emit-koodin ja tallentaa
  tietueen. Konsolissa tila asetetaan samalla avaimella (`Hk/HkInit.cpp:796-803`),
  mutta lohkoa ei käännetä, joten HkMaali käsittelee saman leimauksen eri tavalla.
  Onko ero tarkoituksellinen, ei ratkea staattisesti (Q60). ViestiWinissä
  `_BORLAND_`-määrityksen puuttuminen vaikuttaa vain `#include <vcl.h>` -riveihin.
  / In fitness-orienteering mode only the GUI swaps in the next free badge; whether
  intended is an owner question.
- **T-024 `NREGNLY`:** viestin konsoliohjelmat käyttävät arvoa 8, ViestiWin oletusta
  32 (`Juk/VDef.h:54-55`). Asetustiedoston jäsennin rajaa laitenumerot arvoon
  `NREGNLY-1` (`Juk/VInit.cpp:1771-1772`, `:1808-1875`), joten `LUKIJA9`–`LUKIJA32`
  ovat ViestiWinissä kelvollisia mutta konsolissa ne kuvautuvat laitteeksi 8 ja voivat
  törmätä `LUKIJA8`:n kanssa. Indeksi tallentuu ja välittyy `kanava`-arvona
  (`Juk/VIv.cpp:562`), joten sama asetustiedosto tuottaa eri `kanava`-arvot.
  Sivuvika: ehto `fldn[3] <= L'0'+NREGNLY` hyväksyy laitenumeroiksi merkit `:`–`P`
  (`Juk/VInit.cpp:1809`, `:1828`, `:1875`). Binäärirakenteet eivät riipu arvosta.
- **T-028:** `Hk/HkIV.cpp:1637` `#ifdef CONSOLE` (ei alaviivaa) on aina epätosi, joten
  konsoli näyttää GUI:n viestin, joka viittaa sillä puuttuvaan lomakkeeseen.
- **enum-koko:** ViestiWin ja Tputil-XE käyttävät `-bi-`:tä; repon ainoa enum
  `tuntype` (`Hk/HkDef.h:975`, `Juk/VDef.h:995`) on vain muistissa olevassa
  `kentta[]`-taulukossa eikä näy kirjastoille. Ei vaikutusta [staattinen].
- **`WIN32` kirjastoissa:** Tputil-XE ja DBboxm-XE eivät määrittele `WIN32`:ta
  komentorivillä; se tulee `windows.h`:sta, joten `include/boxr.h:20`, `:30` valitsee
  jäsenen nimen include-järjestyksen mukaan. Koko on 32 tavua kaikissa [mitattu].
- **Linkitysjärjestys:** GUI-linkityksessä `debug_build`-kirjastot ovat ennen
  `release_build`-kirjastoja; hakemistoa ei ole, mutta jos olisi, Debug-objekteja
  voisi päätyä Release-ohjelmaan [staattinen, todentamaton].

## 9. Kumotut ja heikentyneet väitteet / Refuted and weakened claims

| Väite / Claim | Tulos / Result |
|---|---|
| `merkistot.md`: bcc32 lukee BOMittoman UTF-8:n oikein / reads UTF-8 correctly | **kumottu / refuted** (luku 5) |
| `merkistot.md`: rikkinäisiä merkkijonoja 3 / 3 broken literals | **kumottu**: 36 (työkalun bugi / tool bug) |
| `merkistot.md`: merkistöä ei voi yhtenäistää muuttamatta toisen ohjelmaryhmän tekstejä / cannot unify without changing one group | **kumottu**: BOM korjaa molemmat / a BOM fixes both |
| Alustava: `TpLaitteet.cpp:1892` kuollutta koodia (`SIIMPORT`) / dead code | **kumottu**: merkkijono on HkKisaWinin ja ViestiWinin objekteissa [mitattu] |
| Alustava: `time_t`-kuluttajat `Hk/HkXml.cpp:546`, `cbHk/UnitVaiheMaar.cpp:103` | **kumottu**: käyttävät `DateNo`:ta, eivät `time_t`:tä; todelliset kohdat H-080:ssa |
| Alustava: bcc32:n `time_t_date` ei toimi 2038 jälkeen / fails after 2038 | **kumottu**: toimii vuoteen 2106; vain `mktime`-suunta pettää |
| Alustava: `_BORLAND_`-lohko on konsolissa saavuttamaton / unreachable in console | **kumottu**: tila asetetaan konsolissakin → H-081 |
| Alustava: `NREGNLY` ei vaikuta mihinkään tallennettuun / affects nothing stored | **heikentynyt**: `kanava`-arvot ja asetustiedostot eroavat (T-024) |
| MSVC: asetteluerot käännösyksiköiden välillä / cross-unit layout differences | ei yhtään / none (luku 4.2) |
| `wcstok`-kontekstin pudotus rikkoo jäsennyksen / breaks parsing | ei konkreettista tapausta; tila säiekohtainen / none found |

## 10. Avoimet kysymykset / Open questions

| # | Kysymys / Question |
|---|---|
| ~~Q57~~ | **Ratkaistu koeohjelmalla (vaihe 2) / resolved by probe:** HkKisaWinin tavoin rakennetussa VCL-ohjelmassa ohjaussana on 0x037F (laajennettu tarkkuus; vaiheen 1 arvaus $1332 oli väärä), ja oikea `VersioInt` palauttaa 520 (0x027F:lla 521). Toimitettua ohjelmaa ei ajettu; DLL:t voisivat muuttaa sanaa [todentamaton] / CW 0x037F in a VCL probe; shipped exe not run |
| ~~Q58~~ | **Ratkaistu / resolved (vaihe 2):** kyllä, säiekohtainen myös dynaamisella RTL:llä (`cc32370mt.dll`) [mitattu] / yes, per thread |
| ~~Q59~~ | **Ratkaistu / resolved (vaihe 2):** kyllä, kun `TextFl::Open` on oma funktio (`tputilv2.lib`, HkMaali ja ViestiMaali): 0xC0000409 [mitattu]. ViestiLuennassa (`/GL`) ei ole saavutettavaa kutsujaa / yes; no reachable caller in ViestiLuenta |
| Q60 | Onko H-081:n ero (emit-koodin vaihto vain GUI:ssa) tarkoituksellinen? Omistajan päätös / owner decision |
| ~~Q61~~ | **Ratkaistu / resolved (vaihe 2):** vain instantioimattomat mallit puuttuvat; paikalliset ja käyttämättömät luokat raportoidaan [mitattu] / only never-instantiated templates are omitted |
| ~~Q62~~ | **Ratkaistu pääosin / mostly resolved (vaihe 2):** `-H` ei muuta `-K`/`-a1`-tilaa missään 258 GUI-yksikössä [mitattu]; HkKisaWinissä ei löytynyt etumerkistä riippuvaa inline-koodia, joka instantioituisi molemmissa tiloissa [mitattu tarkistetuilta osin; tarkastaja ei toistanut koko hakua] / PCH does not change the pragma state; no signedness-dependent inline code instantiated in both states |
| Q63 | Heittääkö bcc32:n `new int[n]` poikkeuksen negatiivisella `n`:llä vai varaako se muistia (H-085)? / does bcc32 `new int[negative]` throw? |
| Q64 | Onko `wkello()`:n pituus sellainen, että `ViestiWin/UnitSakkoKierr.cpp`:n kumoamispolun sarakejäsennys (`SubString(15,2)`) on oikein (H-082)? / is the undo-path column parsing right? |

## 11. Rajoitukset / Limitations

- Kaikki vaikutusarviot ovat staattisia tai koeohjelmiin perustuvia; toimitettuja
  ohjelmia ei ajettu. / Effects are static or probe-based; shipped programs not run.
- GUI-tyyppien raakakirjoitus ja VCL/RTL-otsikoiden inline-koodi tarkistettiin vaiheessa 2
  (luku 13). / GUI-only raw writes and VCL/RTL inline code were checked in step 2.
- GUI-yksiköiden 1 786 `_wtoi`/`_wtol`/`atoi`-kutsusta luettiin syvällisesti noin 25;
  161 heuristista indeksi-, varaus- tai silmukkaehdokasta on lukematta. / Of 1,786
  integer-parse calls in GUI units about 25 were read in depth; 161 candidates unread.
- Koodisivumittaukset vain koodisivulla 1252. / Code page measured only on 1252.

## 12. Vaihe 2: rajaus / Step 2: scope

Suunnitelman erän 5 tavoitteet (`suunnitelma.md`: `#pragma -K -a1`, asettelu
käännösyksiköiden välillä, `/J`, `wchar_t`, merkistöt, Q7) on käsitelty vaiheessa 1.
Jäljelle jäivät vaiheen 1 rajoitukset ja avoimet kysymykset sekä kääntäjäkohtainen
todennusmenetelmä, jota kommentointivaihe D tarvitsee. Vaihe 2 on staattinen:
koeohjelmat ja kopiot vain väliaikaishakemistossa, ei tuotantokoodin, projektien tai
CI:n muutoksia, ei toimitettujen ohjelmien ajoja eikä verkkokokeita.

The batch 5 goals of the plan were covered in step 1. What remains are the step 1
limitations and open questions, plus the compiler-specific verification method that
the commenting phase D needs. Step 2 is static: probes and copies only in the scratch
folder; no production, project or CI changes, no runs of shipped programs, no network
tests.

| # | Tehtävä / Task | Menetelmä / Method |
|---|---|---|
| 2A | Q8: bcc32-objektien vertailu kommenttimuutoksille (vastine MSVC:n `vertaa_objektit.py`:lle) / bcc32 object comparison for comment-only changes | Tallennetut bcc32-komentorivit, lähdekopio väliaikaishakemistossa, OMF-tietueiden vertailu ilman aikaleimoja ja debug-tietoja / recorded command lines, scratch copy, OMF record comparison |
| 2B | Vain GUI:ssa olevat tyypit: kirjoitetaanko jokin raakana tiedostoon tai verkkoon (luvun 4.1 aukko) / GUI-only types written raw? | Staattinen haku `cbHk/`, `cbTp/`, `ViestiWin/` / static search |
| 2C | T-026:n kohtien saavutettavuus: `swprintf(buf, n, …)`-katkaisu bcc32:lla ja `_wtoi`/`_wtol`-ylivuoto GUI:ssa / reachability of T-026 sites | Staattinen luenta / static reading |
| 2D | Q57 (FPU-ohjaussana VCL-ohjelmassa), Q58 (dynaamisen RTL:n `wcstok`), Q59 (/GS ja H-079), Q62 (PCH ja pragmatila) / probes | Pienet koeohjelmat / small probes |

## 13. Vaihe 2: tulokset / Step 2: results

Tehty 2026-11-25. Tehtävät 2A (pääistunto), 2B–2C (`defect-concurrency-auditor`), 2D
(`legacy-encoding-analyst`); kaikki tarkastettu riippumattomasti (`independent-verifier`).
Raakatulokset: `todisteet/c5/vaihe2/`. / Done on 2026-11-25; all independently reviewed.

### 13.1 Q8: bcc32-objektien vertailu / bcc32 object comparison (2A)

Uusi työkalu `tyokalut/vertaa_omf.py` on MSVC:n `vertaa_objektit.py`:n vastine
GUI-tiedostoille. Se vertaa OMF-tietueet järjestyksessä ja ohittaa vain tietueet, joissa
on lähdetiedoston nimi ja aikaleima (`THEADR`, COMENT E8, E9), tulostehakemiston polku
(COMENT FB, alalaji 06) tai rivinumerot (`LINNUM`, `LINSYM`). [mitattu]

| Koe / Test (10 GUI-yksikköä / units: HkKisaWin 4, ViestiWin 3, Tputil-XE 2, DBboxm-XE 1) | Tulos / Result |
|---|---|
| Uudelleenkäännös / rebuild | 0 / 10 eroaa / differ |
| Kommenttirivit tiedoston alkuun ja kommentti rivin perään / comment lines + trailing comment | 1 / 10 (`tputilv2/perr.cpp`, `__LINE__`) |
| Kommentti funktion sisällä, monirivisen lausekkeen sisällä, cp1252-tavuja kommentissa, kommentti sisällytetyssä otsikossa (`Hk/HkDef.h`) / comment inside a function, inside a multi-line expression, cp1252 bytes, in an included header | sama / equal (tarkastaja / reviewer) |
| Yksi uusi globaali määrittely / one new global definition | 10 / 10 eroaa / differ |
| `ct[8]` → `ct[9]` (`tputilv2/kello.cpp`) | eroaa / differs (`LEDATA`) |
| `#pragma resource` pois, `package(smart_init)` pois, `#pragma link` lisätty / removed, removed, added | eroaa / differs (FB09, FB0A + FB0C, FB08) |

Tarkastus löysi työkalun ensimmäisestä versiosta puutteen: se ohitti koko COMENT FB
-luokan, jolloin `#pragma resource`-, `link`- ja `package`-muutokset jäivät näkymättä
(lomake olisi kaatunut ajossa `EResNotFound`-virheeseen). Korjattu ennen committia:
vain alalaji 06 ohitetaan, ja hakemistotila ilmoittaa myös vain jälkimmäisessä
hakemistossa olevat objektit. Ehdot: ennen- ja jälkeen-käännökset tehdään samalla
`-H`-asetuksella (jaettu PCH muuttaa kolmen funktion pinokehyksen kokoa ja tuottaisi
turhia eroja, ei vääriä yhtäsuuruuksia), ja `__LINE__`-tiedostoissa rivien siirto
muuttaa koodia oikeasti (`merkistot.md`, rajoitukset). Jos tiedosto päättyy
EOF-merkkiin `0x1A`, sen jälkeen lisättyä tekstiä kääntäjä ei lue. **Q8 ratkaistu.**

The new tool `tyokalut/vertaa_omf.py` is the bcc32 counterpart of `vertaa_objektit.py`.
Rebuild and comment-only changes compare equal (except the `__LINE__` file); every code
change is detected. The review found that the first version ignored the whole COMENT
FB class and so missed `#pragma resource`/`link`/`package` changes; fixed before the
commit. Build before and after with the same `-H` setting. **Q8 resolved.**

### 13.2 Vain GUI:ssa olevat tyypit / GUI-only types (2B)

Yhtäkään vain GUI:ssa olevaa rakennetta, luokkaa tai unionia ei kirjoiteta raakana
tiedostoon eikä lähetetä verkkoon [staattinen, riippumattomasti tarkastettu]. GUI
kirjoittaa raakana vain ytimen tyyppejä `pack(push,1)`-alueilta (`emittp`,
`combufrec`, `aikatp`, viestin `kilptietue`), `int`-taulukoita, tavupuskureita tai
tekstiä; vanhojen muotojen tietueilla on oma `pack(push,1)` (`ViestiWin/ConvDat.cpp`,
`Hk/HkInitVanha.cpp`). Luvun 4.1 johtopäätös pätee siis myös GUI-tyyppeihin.
/ No GUI-only type is written raw; the section 4.1 conclusion also holds for them.

### 13.3 T-026:n kohtien saavutettavuus / Reachability of T-026 sites (2C)

- **`swprintf(buf, n, …)`:** GUI-koodissa on 29 laskurimuotoista kutsua ja yksi
  `swprintf_s`. Vain viisi voi katketa: virheilmoitukset `Hk/HkTulostus.cpp:1486`,
  `:1528` (`:1505` on kommentoitu pois) ja `Juk/VTulostus.cpp:1372`, `:1394`, `:1410`,
  muotoa `swprintf(msg, 199, L"…%s…", FileName)`. Katkaisu vaatii vähintään 161–166
  merkin polun ja epäonnistuneen kirjoituksen. bcc32 kirjoittaa 199 merkkiä ilman
  päätettä [mitattu], ja `writeerror_w` (`cbTp/cbTpApu.cpp:480-489`) kopioi enintään
  512 merkkiä pinosta virheikkunaan: ylilukua, ei kirjoitusta eikä todennäköistä
  kaatumista. `Juk/VEmitHtml.cpp:216` on sekava `swprintf`-kutsu, jonka tulos
  ylikirjoitetaan ennen lukua (T-029). Yksikään kutsuja ei käytä paluuarvoa pituutena.
- **`_wtoi`/`_wtol`-ylivuoto:** bcc32:n pyörähdys ei mahdollista yhtään vikaa, jota
  tavallinen syöte ei jo laukaisisi: yhdessäkään `.dfm`-tiedostossa ei ole
  `NumbersOnly`-ominaisuutta, joten käyttäjä voi aina kirjoittaa `-1`, `0` tai tyhjän.
  Varsinaiset viat ovat kääntäjästä riippumattomia puuttuvia rajatarkistuksia
  GUI-lomakkeissa, joita C-4 ei analysoinut: H-082–H-085 ja H-042:n laajennus.

- Only 5 of 29 count-form calls can truncate (error messages, long path plus a write
  error): stack over-read into the dialog. Integer wrap adds no defect that plain input
  could not trigger; the real defects are missing bound checks in GUI forms (below).

### 13.4 Koeohjelmat / Probes (2D)

| # | Tulos / Result | Näyttö / Evidence |
|---|---|---|
| Q57 | VCL-ohjelmassa ohjaussana 0x037F; `VersioInt` = 520, 0x027F:lla 521 (T-025:n syy vahvistui) / CW 0x037F; 520 | [mitattu, toistettu / measured, reproduced] |
| Q58 | `wcstok` säiekohtainen myös dynaamisella RTL:llä / per thread with the dynamic RTL | [mitattu] |
| Q59 | MSVC /GS: 0xC0000409, kun `Open` on oma funktio; `mode` päättyy suoraan evästeen alle. bcc32: ylivuoto osuu `Open`in tallennettuun EBP:hen (`"wb"`/`"rb"`: 2 alinta tavua nollaksi, `"r+b"`: 0x00000045) → kaatuminen paluun jälkeen / caught by /GS; bcc32 clobbers the saved EBP | [mitattu koeohjelmassa; tarkastaja varmisti kehyksen omalla `-S`-käännöksellä] |
| Q61 | Vain instantioimattomat mallit puuttuvat raportista / only never-instantiated templates are omitted | [mitattu] |
| Q62 | `-H` ei muuta pragmatilaa (258 yksikköä); se muuttaa pinokehyksen kokoa kolmessa funktiossa. Etumerkistä riippuvaa inline-koodia ei instantioidu kummassakin tilassa; ainoa etumerkistä riippuva otsikkomalli `std::lexicographical_compare<const char*>` ei ole käytössä (T-023) / PCH keeps the pragma state; no signedness-dependent inline code in use | [mitattu; tarkastaja tarkisti otoksen] |

### 13.5 Uudet ja laajennetut havainnot / New and extended findings

| ID | Lyhyesti / In brief | Vakavuus / Severity |
|---|---|---|
| H-082 | `ViestiWin/UnitSakkoKierr.cpp:153`, `:251`: osuusnumeroa ei rajata; `ostiet[os].ampsakot[…]++/--` osuu tyhjällä kentällä saman tietueen `seura`-kenttään ja suurella numerolla pinoon, ja tietue tallennetaan ja lähetetään / leg number unchecked; record corrupted, saved and sent | keskitaso / medium |
| H-083 | `cbTp/UnitMaalikello.cpp:271-353`: toisen laitteen numerolla ei ylärajaa (HkKisaWin) → kirjoitus `[NREGNLY]`-globaalien yli; `:252` yhden yli; `wcscpy` rajaamatta `ipparam[].destaddr[64]`:ään (molemmat GUI:t) / device number and IP text unbounded | matala / low |
| H-084 | `cbHk/UnitTaulu.cpp:316`, `:495`, `ViestiWin/UnitGAZ.cpp:515`: näyttötaulun lomakekenttien indeksi ilman rajaa / display-board form indices unbounded | matala / low |
| H-085 | `cbHk/UnitArvonta.cpp:916`, `:1021`, `:1052`: `d[-1]`-kirjoitus kasaan laskennallisesta syötteestä; yli 20 kohdesarjaa ylivuotaa pinotaulukot `msrj[20]`, `nsrj[20]`, `nv[20]`, `nu[20]` / draw: heap and stack overflows | keskitaso / medium |
| H-086 | `tputilv2/TextFileClass.cpp:322-359`: `TextFl::Open` käsittelee tilan `"a"` kirjoituksena ja avaa `"wb"`:llä → sakkokierrosloki tyhjenee joka kerta, kun lomake avataan (`cbHk/UnitSakkoKierr.cpp:201`, `ViestiWin/UnitSakkoKierr.cpp:226`) / append mode truncates the log | matala / low |
| H-042 (laajennettu / extended) | GUI-polku: `ViestiWin/UnitSarja.cpp:100`, `:216-249`, `:576` hyväksyy yli 30 osuutta ja 12 väliaikaa ennen `maaraaOsuusluku`a / GUI path accepts more than 30 legs | korkea / high (ennallaan / unchanged) |
| H-079 (laajennettu / extended) | Mitattu: /GS kaataa konsolin, bcc32 korvaa tallennetun EBP:n; saavutettavat kutsujat `UnitSakkoKierr` (molemmat GUI:t) ja `HkConsole/HkCons3.cpp:1564` / measured effect and reachable callers | matala / low (ennallaan / unchanged) |
| T-029 | `Juk/VEmitHtml.cpp:216` sekava `swprintf`, kuollut tallennus / garbled dead store | matala / low |

Tarkastaja kumosi tai tarkensi: `UnitMaalikello` ViestiWinissä ei ole saavutettavissa
(laite 2 piilotettu); `UnitArvonta :916` ei vaadi käyttäjän syötettä vaan syntyy
laskennasta; `VEmitHtml.cpp`:n ylikirjoitus on rivillä 267. ViestiLuennan `/GL`-huoli
H-079:ssä on aiheeton, koska sillä ei ole saavutettavaa kutsujaa. / Refined by the review.

### 13.6 C-5:n jäljellä olevat tavoitteet / Remaining C-5 goals

Suunnitelman erän 5 tavoitteet ja vaiheen 1 avoimet kysymykset on käsitelty Q60:tä
lukuun ottamatta (omistajan päätös). Jäljelle jäävät: Q60, Q63, Q64; GUI-yksiköiden 161
lukematonta kokonaislukujäsennysehdokasta (järjestelmällinen läpikäynti kuuluu
GUI-lomakkeiden syötteiden analyysiin, ei kääntäjäeroihin); toimitettujen ohjelmien
ajonaikaiset vahvistukset (FPU-sana, H-078, H-079), jotka vaativat ajoluvan. Ehdotus:
C-5 suljetaan tähän, ja GUI-lomakkeiden syötteiden tarkistus otetaan omaksi eräkseen.

All batch 5 goals and step 1 questions are covered except Q60 (owner decision).
Remaining: Q60, Q63, Q64; 161 unread integer-parse candidates in GUI units (belongs to
a GUI form input review, not to compiler differences); run-time confirmations that
need permission to run the shipped programs. Proposal: close C-5 here and make GUI form
input validation its own batch.

### 13.7 Sulkeminen / Closure

Käyttäjä hyväksyi C-5:n sulkemisen 2026-11-25 (`b17d536`, `55897eb`). Uusia C-5-analyysejä
tai -testejä ei tehty. Q60 jää omistajan ratkaistavaksi, Q63 ja Q64 avoimiksi teknisiksi
kysymyksiksi. Jatkotehtävät (161 jäsennysehdokasta, joista 105 lomakeyksiköissä ja 56
ytimessä; H-079:n saavutettavuus ja vakavuus; ajonaikaiset vahvistukset luvalla) ja C-5:n
havaintojen alustava sijoitus korjausjonoon: `seuranta.md`.

The user approved closing C-5 on 2026-11-25. No new C-5 analysis or tests. Q60 stays with
the owner; Q63 and Q64 stay open. Follow-ups (161 parse candidates: 105 in form units, 56
in core units; H-079 reachability and severity; run-time checks with permission) and the
provisional fix-queue placement of C-5 findings: `seuranta.md`.
