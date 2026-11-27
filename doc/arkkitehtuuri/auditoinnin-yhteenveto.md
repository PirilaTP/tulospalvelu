# Auditoinnin yhteenveto / Audit summary (C-13)

Tämä on vaiheen C (erät C-1..C-12 ja kierros 1) yhteenveto. Se tehtiin erässä C-13 2026-11-26
rekistereistä. Analysoitu lähderevisio on `f7fe553`; tuotantokoodi on ollut siitä lähtien ennallaan
(`TPsource/`:n alle on lisätty vain C-11:n testitiedostot).
Totuuden lähde ovat rekisterit `rekisteri/havainnot.md`, `rekisteri/riskiarvio.csv` ja
`rekisteri/korjauskokonaisuudet.csv`; tämä dokumentti viittaa niihin eikä korvaa niitä. Seuralaisdokumentit
ovat `korjaussuunnitelma.md`, `regressiotestaussuunnitelma.md` ja `omistajan-paatokset.md`. Todisteet
ovat hakemistossa `todisteet/c13/`. C-13 oli synteesi, ei uusi lähdekoodiauditointi: erässä ei käännetty
eikä ajettu mitään. Tuotantokoodia ei muutettu koko vaiheessa C.

This is the summary of phase C (batches C-1..C-12 and round 1), produced in batch C-13 on 2026-11-26
from the registers. The analysed source revision is `f7fe553`; production code has not changed since
(only the C-11 test files were added under `TPsource/`). The
registers (`rekisteri/havainnot.md`, `rekisteri/riskiarvio.csv`, `rekisteri/korjauskokonaisuudet.csv`)
remain the source of truth; this document refers to them. Companion documents: `korjaussuunnitelma.md`,
`regressiotestaussuunnitelma.md`, `omistajan-paatokset.md`. Evidence is in `todisteet/c13/`. C-13 was a
synthesis, not a new source audit: nothing was built or run in it. No production code was changed in the
whole of phase C.

Polut ovat suhteessa hakemistoon `TPsource/V52/`, ellei toisin mainita. Merkinnät: [todennettu],
[päätelty], [tuntematon] (ks. `README.md`, "Varmuusasteet"). Luvut ovat todistusraportista
`todisteet/c13/raportit/osaA-kattavuus.md` ("raportti"). / Paths are relative to `TPsource/V52/`. Figures
come from the evidence report `todisteet/c13/raportit/osaA-kattavuus.md` ("the report").

## 1. Lähtöluvut ja täsmäytys / Starting figures and reconciliation

Lähtöluvut tarkistettiin koneellisesta pohjasta (`todisteet/c13/osaA0/`) ja validaattorista. Pohjan
tarkistus (`tarkistus_a0.out`) päättyy riviin "differences: 0" [todennettu]. Ainoa todellinen ero
oli analyysitaso 3 (22 tiedostoa, ei 12) ja vanhentunut viitemäärä (2 395, ei 2 249).

The starting figures were checked against the machine base (`todisteet/c13/osaA0/`) and the validator. The
base check ends with "differences: 0" [verified]. The only real differences were analysis level 3 (22
files, not 12) and a stale reference count (2,395, not 2,249).

| Suure / Figure | Täsmäytetty arvo / Reconciled value | Varmuus / Confidence |
|---|---|---|
| H-löydökset / H findings | 177: 176 avointa, 1 kumottu (H-099) / 176 open, 1 refuted | [todennettu] |
| T-kohteet / T items | 52 (T-001..T-052); T-001..T-010 ovat kierroksen 1 dokumenteista, vaikka pohja sijoittaa ne ID-välin mukaan erään C-1 / T-001..T-010 come from round-1 documents, although the base places them in C-1 by ID range | [todennettu], [päätelty] (jaottelu) |
| Kysymykset / Questions | Q1..Q112, ei puuttuvia; 28 ratkaistu, 72 avoin, 6 osittain, 3 omistajan päätös, 3 ajonaikainen todennus / none missing; 28 resolved, 72 open, 6 partly, 3 owner decision, 3 run-time | [todennettu] |
| Lähdeviitteet / Source references | 2 395: 1 969 rivinumerolla, 426 pelkkä tiedosto, 1 663 eri merkkijonoa / 1,969 with a line number, 426 file only, 1,663 distinct strings | [todennettu] |
| Esikääntäjävalitsimet / Preprocessor switches | 435 luokiteltua, 2 491 ehtokohdassa / 435 classified, at 2,491 condition sites (`rekisteri/valitsimet.csv`) | [todennettu] |
| Asetusavainrivit / Setting-key rows | 750: `Hk/HkInit.cpp` 291, `Juk/VInit.cpp` 206, `cbHk/UnitCfg.cpp` 135, `ViestiWin/UnitCfg.cpp` 118 (rivejä, ei yksilöllisiä avaimia / rows, not unique keys) | [todennettu] |
| Testimatriisi / Test matrix | 18 riviä T1..T18, ei ajettu / 18 rows, not run (`ominaisuusvalitsimet.md:433-434`, `ominaisuusvalitsimet.md:440-460`) | [todennettu] |
| Tiedostot ja analyysitasot / Files and levels | `rekisteri/tiedostot.csv`: 650 riviä, 273 179 riviä koodia; 631 tuotantotiedostoa, 270 050 riviä: taso 1: 474, taso 2: 135, taso 3: 22, taso 4: 0; lisäksi 19 testikehystiedostoa (3 129 riviä) tasolla 4 / 650 rows, 273,179 lines; 631 production files, 270,050 lines at levels 1: 474, 2: 135, 3: 22, 4: 0; plus 19 test-harness files (3,129 lines) at level 4 | [todennettu] |

Luku 2 395 on lähtötilanne (C-12:n sulkeminen); C-13:n omat asiakirjat ja rekisterimerkinnät nostivat
määrän 2 449:ään [todennettu validaattorilla C-13:n lopussa]. Lähdeviitteiden 2 395 on validaattorin (`tarkista_auditointi.py:76-100`) regex-osumien määrä. Validaattori
tarkistaa vain, että tiedosto on olemassa ja rivinumero on tiedoston sisällä
(`tarkista_auditointi.py:9-10`). Se ei koskaan vertaa sisältöä. Luku ei siis kerro viitteiden oikeellisuudesta
[todennettu]. / The 2,395 is the validator's regex match count. It only checks that the file exists and the
line number is inside the file. It never compares content. The figure says nothing about whether a
reference is correct [verified].

Lukujen täsmäytyksessä löytyi 21 vanhentunutta tai keskenään ristiriitaista väitettä (S1..S21, raportin
luku 2). Ne korjattiin C-13:n osassa I eli niitä ei korjata tässä dokumentissa [todennettu]. Ryhmittäin:

- `CHECKPOINT.md`: S1-S9. Taso 3 on 12 (oikein 22), viitemäärä 2 249 (oikein 2 395); H-017:n tarkastustila
  vanhentunut; tarkastustaulukko kattaa vain H-001..H-108; "ajettu testikehyksessä" -rivit puutteelliset;
  korkeita löydöksiä 22 tai 26 (oikein 34); erätaulukossa ei rivejä erille C-7, C-8 ja C-9; commit-lista
  päättyy kohtaan `c6205d2` ja "seuraava askel" on vanha; kumottujen väitteiden luettelo kattaa vain
  erät C-2..C-4. Korjattu C-13:n osassa I.
- `CHECKPOINT.md` S10: taso 2 -rivin maininta "C-10: riippuvuuskohdat" ei vastaa `rekisteri/tiedostot.csv`:ää
  (yhdenkään rivin huomautus ei mainitse C-10:tä tai C-12:ta) [päätelty]. Korjattu C-13:n osassa I.
- `README.md` S15: lukujärjestystaulukon rivit `riippuvuudet.md`, `testaus.md` ja
  `virheenkasittely.md` vanhentuneet. Korjattu C-13:n osassa I.
- `rekisteri/README.md` S12-S14: väite "yhtään H-löydöstä ei ole vahvistettu ajamalla" (H-041 on
  vahvistettu); vakavuusasteikko vajaa; tason 4 määritelmä ei vastaa 19 testitiedoston todellista
  tilaa [päätelty]. Korjattu C-13:n osassa I.
- `rekisteri/moduulit.md` S11: yhteenveto pysähtyy erään C-7 (477/134/20), hakemistokohtaiset luvut
  eroavat csv:stä. Korjattu C-13:n osassa I.
- `seuranta.md` S17-S18: erien C-1..C-3 ja C-11 rivit sanovat "valmis, tarkastettu" eivätkä "suljettu";
  Q3-rivi vanhentunut. Korjattu C-13:n osassa I.
- `suunnitelma.md` S16: C-12 ja C-13 merkitty "ei aloitettu". Korjattu C-13:n osassa I.
- Erä-raporttien keskinäiset ristiriidat S19-S21 (`tallenna-samanaikaisuus.md`, `tiedonsiirto.md`):
  H-021:n tila, H-038:n tila ja H-019:n luokittelu. Rekisteri on ratkaiseva; raportteja ei päivitetty.

In English: 21 stale or inconsistent statements were found (S1..S21, report section 2): in
`CHECKPOINT.md` (S1-S10), `README.md` (S15), `rekisteri/README.md` (S12-S14), `rekisteri/moduulit.md`
(S11), `seuranta.md` (S17-S18), `suunnitelma.md` (S16) and between batch reports (S19-S21). The first six
files were corrected in C-13 part I (korjattu C-13:n osassa I). For S19-S21 the register is authoritative and
the batch reports were not updated.

**Makroarvioijan artefakti / Macro evaluator artefact.** C-10:n ja C-12:n skriptit tallentavat
lausekkeella määritellyt makrot arvolla 1. Esimerkki: viestin `MAX_PORTTI` (`Juk/VDef.h:62`) näkyy
arvona 1, todellinen arvo on 132. Pohjassa on 238 tällaista määritystä (176 nimeä), joista 22 nimeä
testataan ehdoissa (13 niistä `DOCTEST_*`). Luettelo on `todisteet/c13/osaA0/makroartefaktit.csv`.
Mikään C-13:n johtopäätös ei käytä tällaista tallennettua arvoa [todennettu].
/ The C-10 and C-12 scripts record expression-defined macros with the value 1. For example the relay
`MAX_PORTTI` shows 1, real value 132. The base has 238 such definitions (176 names); 22 names are tested
in conditions (13 of them `DOCTEST_*`). List: `todisteet/c13/osaA0/makroartefaktit.csv`. No conclusion
of C-13 uses such a recorded value [verified].

## 2. Erien toteutuma / What each batch achieved

Taulukko tiivistää raportin luvun 3 taulukot A ja B. "Tark." = riippumattoman tarkastuksen tulos
(vahvistettu / osittain / kumottu). "Kriteerit" = täyttyivätkö erän päätöskriteerit. Täydet perustelut ja
lähdeviitteet ovat raportin luvussa 3. H/T/Q-määrät ovat A0-pohjan ID-välin mukaan [todennettu].

The table condenses the report's section 3 (tables A and B). "Tark." = independent review outcome
(confirmed / partly / refuted). "Kriteerit" = completion criteria met. Full basis and references are in the
report's section 3. H/T/Q counts follow the ID range in the A0 base [verified].

| Erä / Batch | Raportti / Report | Rajaus / Scope | Menetelmä / Method | H / T / Q | Tark. / Review | Kriteerit / Criteria | Aukko tai jatko / Gap or follow-up |
|---|---|---|---|---|---|---|---|
| Kierros 1 / Round 1 | `inventaario.md`, `merkistot.md`, `riippuvuudet.md` | Kaikki viisi ohjelmaa, vain inventaario / all five programs, inventory only | Skriptit, varoitukset, TpTest 46/46 / scripts, warnings | 6 / 10* / 11 | ei silloin / none then | ei muodollisia / no formal criteria | Q6, Q9-Q12 |
| C-1 | `tallenna-samanaikaisuus.md` | `tallenna()`-rinnakkaisuus / concurrency | Staattinen luku / static | 15 / 0* / 4 | 11 vahv., 1 ositt., 2 ei tark., 1 ei ratkea; 1 osaväite kumottu / 11 conf., 1 partly, 2 not reviewed, 1 undecidable | täyttyi / met | Viestin vastaanottopolku ja säikeet jäivät / relay receive path and threads |
| C-2 | `tiedonsiirto.md` | Asemien välinen synkronointi / station sync | Staattinen / static | 18 / 4 / 8 | 12 vahv., 4 ositt., 2 ei tark.; 4 väitettä kumottu / 12 conf., 4 partly, 2 not reviewed; 4 refuted | täyttyi / met | Ei oikeaa usean aseman ajoa; 8 skenaariota ajamatta / no real multi-station run |
| C-3 | `tiedostomuodot.md` | Pysyvät tiedostot, koot / persistent files, sizes | Staattinen + käännösaikaiset sizeof-kokeet / static + compile-time probes | 15 / 5 / 11 | 10 vahv., 5 ositt.; 5 kumottu / 10 conf., 5 partly; 5 refuted | täyttyi / met | Ei ajoa oikealla datalla; Q26-Q34 |
| C-4 | `muistiturvallisuus.md` | Varoitukset ja puskurit, 2 kääntäjää / warnings and buffers, 2 compilers | Käännökset, 275 rivin seula / builds, triage of 275 rows | 23 / 3 / 14 | 5 tark.: 5 vahv., 4 ositt.; 1 kumottu / 5 reviews | täyttyi rajattuna / met as limited | Noin 93 % varoituksista seulomatta; Q43-Q56 |
| C-5 | `kaantajaerot.md` | Pragma, rakenteiden asettelu, merkistöt / pragma, struct layout, charsets | Käännösajot, kokeet, tdump / compiler runs, probes | 9 / 7 / 8 | C1-C10 + 3 tarkennusta; 10 riviä kumottu tai heikennetty / 10 rows refuted or weakened | täyttyi (vaihe 2 lisätty) / met (step 2 added) | 161 kokonaisluvun jäsennyskandidaattia; Q60, Q63, Q64 |
| C-6 | `saikeet-ja-lukot.md` | Säikeet ja lukot / threads and locks | grep, luku; vaihe 2: erilliset MSVC-kokeet / step 2 probes | 22 / 4 / 10 | 20 väitettä: 12 vahv., 8 ositt.; H-099 kumottu / 20 claims | täyttyi / met | bcc32/GUI-ajo; H-091-sykli ajamatta / GUI run missing |
| C-7 | `gui-liiketoimintalogiikka.md` | 8 GUI-lomakeyksikköä, noin 16 000 riviä / 8 form units | Staattinen, Opus-kierrokset / static, Opus rounds | 19 / 4 / 3 | 16 väitettä: 10 vahv., 6 ositt., 0 kumottu; 9 epäilyä kumottu / 16 claims | pakollinen tavoite täyttyi, arvonta osittain / mandatory met, draw partly | J7-1..J7-7, ei aloitettu / none started |
| C-8 | `globaali-tila.md` | Globaali muuttuva tila / global mutable state | Skriptit, Sonnet + Opus, koe H-128/H-129 | 17 / 2 / 5 | 16 väitettä: 8 vahv., 3 muutoksin, 5 ositt., 0 kumottu / 16 claims | täyttyi / met | 153 funktiostaattista luokittelematta; Q78-Q82 |
| C-9 | `virheenkasittely.md` | Virheenkäsittely ja resurssit / error handling and resources | Skriptit, luku, vaihe 2: Asan-kokeet / step 2 probes | 14 / 3 / 13 | 19 väitettä: 16 vahv., 5 ositt., 1 osaväite kumottu / 19 claims | täyttyi / met | GUI-ajo, H-153; Q87-Q95 |
| C-10 | `riippuvuudet.md` | Ulkoiset riippuvuudet / external dependencies | Merkkiskripti, luku, PE-taulut / marker script, reading, PE tables | 12 / 7 / 11 | 15 väitettä: 11 vahv., 4 ositt.; N-9 kumottu HK:lle / 15 claims | täyttyi, suljettu ilman vaihetta 2 / met, no step 2 | Oikeat laitteet ja palvelimet; Q96-Q106 |
| C-11 | `testaus.md` | Testaus: testikehys ja yksi toimitetun ohjelman ajo / test harness and one shipped-program run | Testikehys (23 PASS, 24 XFAIL), H-041-ajo / harness runs, H-041 run | 0 / 0 / 8 | Testisuunnitelma: 13 riviä (4 vahv., 6 ositt., 2 kumottu, 1 sekalainen) / test plan review | ei alkuperäisessä muodossa: CI ei tehty / not as originally written: CI not done | CI, H-009/H-014-käsitestit; Q35-Q42 |
| C-12 | `ominaisuusvalitsimet.md` | Ominaisuusvalitsimet ja asetukset / switches and settings | Leksikaaliset skriptit, Sonnet + Opus / lexical scripts | 7 / 3 / 6 | 19 väitettä: 12 vahv., 7 ositt., 0 kumottu / 19 claims | täyttyi (7/7), suljettu ilman vaihetta 2 / met (7/7) | Testimatriisi ajamatta; Q107-Q112 |

\* Pohja sijoittaa T-001..T-010:n erään C-1 ID-välin mukaan; niiden lähteet ovat kierroksen 1 dokumentit
[päätelty]. / The base places T-001..T-010 in C-1 by ID range; their sources are round-1 documents [inferred].

Kaikki erät päättyivät riippumattomaan tarkastukseen ja rekisterin päivitykseen, paitsi että kierroksella 1
tarkastusta ei tehty. Erän C-11 päätöskriteeri 7 (toimitetun ohjelman ennalta lukittu tulos) ei täyttynyt:
H-041 on vahvistettu ajamalla käyttäjän hyväksymällä poikkeamalla (`testaus.md:1786-1790`). Tokenien
kokonaiskulutus on dokumentoitu vain osasta eriä (C-5, C-6, C-8, C-9, C-10, C-12); muiden osalta se on
[tuntematon]. / All batches ended with an independent review and a register update, except round 1.
C-11 criterion 7 (the pre-fixed shipped-program verdict) was not met; H-041 is confirmed by running through
a user-approved deviation. Token use is documented for only some batches; for the others it is [unknown].

## 3. Koodipohjan kattavuus / Coverage of the code base

Rekisteri `rekisteri/tiedostot.csv` kattaa 650 lähdetiedostoa (päätteet cpp, h, c ja H). Taso 1 = vain skriptillä
inventoitu, koodia ei luettu; taso 2 = kohdennettu luku (nimetyt funktiot tai rivit); taso 3 = rakenne
luettu, avainfunktiot luettu; taso 4 = koko tiedosto luettu ja tarkastettu virheiden varalta
(`rekisteri/README.md:26-34`). Taulukon kunkin tason sarakkeessa on tiedostoja / rivejä [todennettu].

The register covers 650 source files (extensions cpp, h, c and H). Level 1 = scripted inventory only, code not read; level 2 =
targeted reading; level 3 = structure read, key functions read; level 4 = whole file read and checked for
defects. Each level column shows files / lines [verified].

| Hakemisto / Directory | Tiedostoja / Files | Rivejä / Lines | Taso 1 / Level 1 | Taso 2 / Level 2 | Taso 3 / Level 3 | Taso 4 / Level 4 | Löydöstiedostoja / Files with H/T ids |
|---|--:|--:|--:|--:|--:|--:|--:|
| Hk | 27 | 46 032 | 6 / 728 | 18 / 41 418 | 3 / 3 886 | 0 / 0 | 20 |
| Juk | 23 | 38 605 | 3 / 331 | 20 / 38 274 | 0 / 0 | 0 / 0 | 19 |
| Tp | 13 | 11 704 | 7 / 498 | 5 / 8 021 | 1 / 3 185 | 0 / 0 | 5 |
| com | 3 | 1 807 | 0 / 0 | 2 / 409 | 1 / 1 398 | 0 / 0 | 3 |
| dbboxm | 13 | 2 375 | 6 / 1 333 | 5 / 603 | 2 / 439 | 0 / 0 | 2 |
| include | 21 | 1 883 | 13 / 823 | 5 / 775 | 3 / 285 | 0 / 0 | 3 |
| tputilv2 | 169 | 15 513 | 143 / 10 549 | 23 / 4 546 | 3 / 418 | 0 / 0 | 11 |
| HkConsole | 8 | 9 605 | 4 / 1 989 | 4 / 7 616 | 0 / 0 | 0 / 0 | 3 |
| JukConsole | 10 | 8 271 | 5 / 1 121 | 5 / 7 150 | 0 / 0 | 0 / 0 | 2 |
| cbHk | 149 | 67 530 | 127 / 38 996 | 18 / 19 605 | 4 / 8 929 | 0 / 0 | 22 |
| ViestiWin | 114 | 43 424 | 94 / 23 082 | 15 / 13 011 | 5 / 7 331 | 0 / 0 | 17 |
| cbTp | 73 | 15 115 | 63 / 11 170 | 10 / 3 945 | 0 / 0 | 0 / 0 | 9 |
| RADStudio10 | 2 | 436 | 1 / 185 | 1 / 251 | 0 / 0 | 0 / 0 | 0 |
| VS | 1 | 45 | 1 / 45 | 0 / 0 | 0 / 0 | 0 / 0 | 0 |
| Tests (vanhat / older) | 5 | 7 705 | 1 / 7 106 | 4 / 599 | 0 / 0 | 0 / 0 | 0 |
| Tests (C-11-testitiedostot / test files) | 19 | 3 129 | 0 / 0 | 0 / 0 | 0 / 0 | 19 / 3 129 | 0 |
| **Yhteensä / All** | 650 | 273 179 | 474 / 97 956 | 135 / 146 223 | 22 / 25 871 | 19 / 3 129 | 116 |

Taso 1 -luvut eivät tarkoita, että kooditiedoston sisältö olisi tarkastettu. Tulkinta:

- **Vain inventaario.** 474 tiedostoa ja 97 956 riviä (36 % kaikista riveistä) on tasolla 1. Ilman
  kolmannen osapuolen `Tests/doctest.h`-tiedostoa (7 106 riviä) luku on 90 850 riviä. Siitä
  GUI-lomakehakemistot cbHk, ViestiWin ja cbTp muodostavat 284 tiedostoa ja 73 248 riviä (81 %);
  `tputilv2` 143 tiedostoa ja 10 549 riviä [todennettu].
- **Taso 2 on kohdennettu.** Sen 146 223 riviä ovat kokonaisten tiedostojen kokoja, ei luettuja rivejä.
  Neljä suurinta tiedostoa luettiin löydöskohtien mukaan, ei läpi: `Juk/VTulostus.cpp` (9 151 riviä),
  `Hk/HkTulostus.cpp` (8 840), `Hk/HkEmit.cpp` (6 432) ja `Juk/VEmit.cpp` (6 221) [todennettu].
- **Taso 3** kattaa 25 871 riviä (9,5 %) 22 tiedostossa. Yksikään tuotantotiedosto ei ole tasolla 4;
  19 tason 4 tiedostoa ovat auditoinnin kirjoittamia C-11-testikehyksen lähteitä, joita ei ole tarkastettu
  virheiden varalta [todennettu]. Tason 3 ja tason 4 tiedostoluettelot ovat raportin luvuissa 4.1 ja 4.2.
- Yhdelläkään hakemistolla ei ole pelkkää tasoa 1. Lähimpänä ovat `VS` (45 riviä), `cbTp` (74 % riveistä
  tasolla 1), `tputilv2` (68 %), `cbHk` (58 %) ja `ViestiWin` (53 %) [todennettu].
- 116 tiedostossa on H- tai T-tunnisteita; kaikki ovat tasolla 2 tai 3 [todennettu].
- Rekisterin aukko: `cbHk/UnitCfg.cpp` (2 936 riviä) ja `ViestiWin/UnitCfg.cpp` (2 352) ovat tasolla 1, vaikka
  C-12 jäsensi niiden avaimet skriptillä. `.dfm`- ja projektitiedostoilla ei ole tasoa [todennettu].

In English: level 1 (inventory only, code not read) holds 474 files and 97,956 lines (36 % of all lines);
without the third-party `Tests/doctest.h` it is 90,850 lines. The GUI form directories cbHk, ViestiWin and
cbTp hold 284 files and 73,248 lines of that (81 %). Level 2 means targeted reading: its 146,223 lines are
whole-file sizes, and the four largest files (`Juk/VTulostus.cpp`, `Hk/HkTulostus.cpp`, `Hk/HkEmit.cpp`,
`Juk/VEmit.cpp`) were read by site. Level 3 is 22 files (25,871 lines). No production file is level 4; the
19 level-4 files are audit-written harness sources. The level-3 and level-4 file lists are in the report's
sections 4.1 and 4.2.

**Tutkimatta jääneet alueet / Unexamined areas.** Syyt on annettu erä kerrallaan [todennettu]:

| Alue / Area | Syy / Reason |
|---|---|
| C-7 J7-1: arvonta-algoritmi `cbHk/HkArv2.cpp` (1 274 riviä, taso 1) / draw algorithm | Rajattu pois erästä; ei aloitettu / left out of the batch; not started |
| C-7 J7-2: 41 muuta lomaketiedostoa, jotka kirjoittavat kilpailudataa / 41 other writing forms | Ei aloitettu / not started |
| C-7 J7-3: 152 jäsennyskandidaattia (lomakkeet 96, ydin 56) / 152 parse candidates | Ei aloitettu / not started |
| C-8: 153 funktiostaattista muuttujaa hakemistojen `Hk/` ja `Juk/` ulkopuolella / 153 unclassified function statics | Luetteloitu, ei luokiteltu / listed, not classified |
| C-5: 161 kokonaisluvun jäsennyskandidaattia (9 selvitetty C-7:ssä, 152 jäljellä) / 161 candidates | Ei luettu; sama joukko kuin J7-3 / unread; overlaps J7-3 |
| C-4: noin 93 % varoituksista (6 569 / 7 030) seulomatta / about 93 % of warnings untriaged | Massaluokat; bcc32:ssa ei muoto- tai rajatarkistusta / bulk classes; bcc32 has no format or bounds analysis |
| Ajonaikaiset kohteet (C-6, C-9, C-10, C-12) / run-time items | Ei laitteita, verkkoa, SQL:ää eikä bcc32-GUI-ajoa; ks. luku 6 / no devices, network, SQL or bcc32 GUI runs; see section 6 |

## 4. Todennuksen vahvuus / Strength of verification

Lähdeviitteen validointi (validaattori) ja tekninen todennus ovat eri asioita. Alla olevat luokat kuvaavat
teknistä todennusta. Jokaiselle löydökselle käytetään vahvinta todistetta; luokka annetaan järjestyksessä
a (kumottu), b (toimitettu ohjelma), c (koe tai testikehys), sitten tarkastustilan mukaan [todennettu].

Source reference validation (the validator) and technical verification are different things. The classes
below describe technical verification. The strongest evidence per finding is used; a class is assigned in
the order a (refuted), b (shipped program), c (probe or harness), then by review state [verified].

| Luokka / Class | Merkitys / Meaning | Lkm / Count | Löydökset / Findings |
|---|---|--:|---|
| a | kumottu / refuted | 1 | H-099 |
| b | vahvistettu toimitetussa ohjelmassa / confirmed in a shipped program | 1 | H-041 |
| c | toistettu eristetyllä kokeella tai testikehyksellä / reproduced in an isolated probe or harness | 26 | C-11 (14): H-001, H-002, H-007, H-008, H-022..H-027, H-031, H-032, H-040, H-042. C-6 (4): H-090, H-091, H-093, H-094. C-8 (2): H-128, H-129. C-9 (6): H-145, H-146, H-147, H-149, H-150, H-151 |
| d | riippumaton lähdetarkastus, täysin vahvistettu / independent source review, fully confirmed | 89 | kaikki muut tilalla `riippumaton` (lista: `todisteet/c13/osaA0/havainnot.csv`) |
| e | riippumaton tarkastus, osittain vahvistettu / partly confirmed | 43 | lista raportin luvussa 5; H-032 on "osittain" mutta luokassa c / listed in the report's section 5; H-032 is "partly" but sits in class c |
| f | tarkastajan oma löytö / reviewer's own find | 3 | H-132, H-158, H-162 |
| g | yhden agentin staattinen löydös, ei koskaan tarkastettu / never independently reviewed | 13 | H-003, H-006, H-019, H-039, H-108, H-122, H-123, H-124, H-126, H-127, H-142, H-143, H-144 |
| h | ei ratkaistavissa staattisesti / not statically decidable | 1 | H-020 (tarvitsee linkityskartan, Q12 / needs a link map) |

Luokan c 26 löydöksen jako: 23 tarkastettu täysin, 1 osittain (H-032), 2 ei lainkaan (H-001, H-002).
Luokan c kokeet on tehty MSVC-konsolirakennelmilla; H-041 toistettiin myös testikehyksessä, mutta se on
luokiteltu vain luokkaan b [todennettu]. Tarkastajan löytöjä on lisäksi H-019 (`tallenna-samanaikaisuus.md:193`),
joka rekisterin mukaan on "ei tarkastettu" ja on siksi luokassa g; H-163 ja H-170 olivat C-10:n tarkastajan
löytöjä, mutta pohja sijoittaa ne luokkiin d ja e [päätelty]. / Class c split: 23 fully reviewed, 1 partly
(H-032), 2 not at all (H-001, H-002). All class c probes are MSVC console builds.

Luokka vakavuuden mukaan / Class by severity:

| Luokka / Class | korkea | keskitaso-korkea | keskitaso | matala-keskitaso | matala | kumottu | Yht. / Total |
|---|--:|--:|--:|--:|--:|--:|--:|
| a | 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| b | 1 | 0 | 0 | 0 | 0 | 0 | 1 |
| c | 19 | 1 | 6 | 0 | 0 | 0 | 26 |
| d | 13 | 0 | 46 | 7 | 23 | 0 | 89 |
| e | 1 | 0 | 18 | 8 | 16 | 0 | 43 |
| f | 0 | 0 | 1 | 1 | 1 | 0 | 3 |
| g | 0 | 0 | 2 | 4 | 7 | 0 | 13 |
| h | 0 | 0 | 0 | 0 | 1 | 0 | 1 |
| Yht. / Total | 34 | 1 | 73 | 20 | 48 | 1 | 177 |

Korkeista 34 löydöksestä 1 on vahvistettu toimitetussa ohjelmassa (H-041), 19 toistettu (c), 13 tarkastettu
täysin (d) ja 1 osittain (e: H-060). Yksikään ei ole luokassa g; H-001:llä ja H-002:lla ei ole riippumatonta
lähdetarkastusta [todennettu]. Vakavuuksista 23 on ehdollisia (d 14, c 6, e 3).
/ Of the 34 high findings: 1 shipped program, 19 reproduced, 13 fully reviewed, 1 partly reviewed. None is
class g. 23 severities are conditional.

**Kaksi tarkennusta / Two clarifications.**

1. Neljä todistelajia ovat eri asioita [todennettu]. (i) Staattinen luku: yksi agentti lukee koodin.
   (ii) Riippumaton lähdetarkastus: toinen agentti lukee koodin ja yrittää kumota väitteen; se on staattinen,
   samaa mallisukua ja samaa repositoriota, eikä ihminen ole mukana (`rekisteri/README.md:46-52`).
   (iii) Mekanismin toisto testikehyksessä tai eristetyssä kokeessa, joka on rakennettu muuttamattomasta
   tuotantokoodista (MSVC-konsolirakennelmat): osoittaa mekanismin, ei sitä, että laukaisija esiintyy
   käynnissä olevassa ohjelmassa. (iv) Vahvistus toimitetussa ohjelmassa: vain H-041, ja sekin
   käyttäjän hyväksymällä poikkeamalla ennalta lukitusta tuloksesta (`testaus.md:1786-1790`).
   / The four kinds of evidence are different things: (i) static reading; (ii) independent source review,
   meaning a second agent reading the code, not a human, same model family, static; (iii) reproduction of
   the mechanism in a harness or isolated probe built from unchanged production code (MSVC console builds),
   which shows the mechanism, not that the trigger occurs in the running program; (iv) confirmation in a
   shipped program, only H-041, by a user-approved deviation from the pre-fixed verdict.
2. Lähdeviitteiden validointi ei ole tekninen todennus [todennettu]. Validaattori tarkistaa vain, että
   tiedosto on olemassa ja rivi on tiedoston sisällä; se ei vertaa sisältöä, eikä se skannaa hakemistoa
   `todisteet/` (`tarkista_auditointi.py:9-10`, `tarkista_auditointi.py:76-100`). Luku 2 395 ei siis todista
   mitään väitettä oikeaksi. / Reference validation is not technical verification: it checks existence and
   range only, never content, and does not scan `todisteet/`.

Vahvuus painottuu erille, joissa kokeet tai C-11-testikehys toistivat vanhoja löydöksiä (C-2 8, C-6 4,
C-9 6). Erillä C-4 (23 löydöstä), C-10 (12) ja C-12 (7) ei ole koe- eikä ajotodistetta lainkaan.
Heikoimmat rivit ovat C-7 (5 tarkastamatonta 19:stä), C-8 (3 tarkastamatonta, 1 tarkastajan löytö) ja
kierros 1 (H-001, H-002, H-003, H-006 ilman riippumatonta tarkastusta) [todennettu].
/ Strength is concentrated where probes or the C-11 harness reproduced older findings. C-4, C-10 and C-12
have no probe or run evidence at all. The weakest rows are C-7, C-8 and round 1.

## 5. Kumotut ja heikennetyt väitteet / Refuted and weakened claims

Vain yksi väite on koskaan päätynyt kumotuksi H-riviksi: H-099 (C-6 vaihe 2; `initcom` palauttaa aina 0,
joten epäonnistumishaara on kuollutta koodia). Muut kumoamiset tapahtuivat ennen rekisteröintiä tai
poistivat osan rekisteröidystä väitteestä. Määrät ovat raporttien mainitsemia kohteita; A0-pohja ei sisällä
niitä [todennettu].

Only one claim ever became a refuted H row: H-099. All other refutations happened before a claim was
registered, or removed part of a registered one. Counts are items listed in the batch reports; the A0 base
does not hold them [verified].

| Erä / Batch | Kumottu / Refuted | Heikennetty tai korjattu / Weakened or corrected |
|---|--:|--:|
| Kierros 1 / Round 1 | 3 | 0 |
| C-1 | 1 | 1 |
| C-2 | 4 | 5 |
| C-3 | 5 | 5 |
| C-4 | 6 | 4 tai enemmän / 4+ |
| C-5 | 7 | 3 |
| C-6 | 1 H + 8 | 8 |
| C-7 | 9 | 6 |
| C-8 | 0 | 8 |
| C-9 | 8 | 5 |
| C-10 | 1 | 4 |
| C-11 | 2 | 6 |
| C-12 | 2 avainta / 2 keys | 7 |

Kumoamisten laji vaihtelee: C-5, C-7 ja C-9 luettelevat rekisteröimättömiä epäilyjä; C-2 ja C-3 agenttien
luonnoksista tulleita väitteitä; C-11 kumoaa osia testisuunnitelmasta, ei koodista. Siksi sarakkeita ei
voi laskea yhteen yhdeksi virhesuhteeksi [päätelty]. Lähteet ja yksittäiset kohteet ovat raportin luvussa 6.

Päätelmät [päätelty]:

- H-rivitasolla kumoamisosuus on 1 / 177 (0,6 %) tarkastuksen jälkeen. Luku on matala, koska kumoaminen
  tapahtuu ennen rekisteröintiä. Erillä, jotka raportoivat molemmat puolet, noin neljännes tai puolet
  tarkastetuista väitteistä kaventui tai kumoutui osittain: C-6 8 / 20, C-7 6 / 16, C-8 8 / 16,
  C-9 5 / 19, C-10 4 / 15, C-12 7 / 19. Lisäksi 5-10 epäilyä erää kohti pudotettiin pois.
- Varoituspolulla oli korkein raakavirhesuhde: C-4:n seulassa 141 / 275 luokitellusta varoituksesta (51 %)
  oli vääriä hälytyksiä, ja 14 / 33 tutkittavasta rivistä (42 %).
- Rekisteröidyt mekanismit pitivät yleensä paikkansa; heikkous on vaikutuksessa ja saavutettavuudessa,
  jonka tarkastajat korjasivat 6-9 väitteessä erää kohti.
- Luokan g 13 tarkastamatonta riviä ja 3 tarkastajan löytöä ovat tämän kuvan ulkopuolella; niiden
  virhesuhde on [tuntematon].

Johtopäätös: yhden agentin staattiseen löydökseen ei voi luottaa ilman tarkastusta, sillä tarkastettuja
väitteitä kavensi tai korjasi karkeasti joka neljäs-toinen. Mekanismit olivat luotettavampia kuin
vaikutusarviot [päätelty].

In English: at H-row level the refutation rate is 1 of 177 (0.6 %) after review. This is low because
refutation happens before registration. In the batches that report both sides, roughly a quarter to a half
of reviewed claims were narrowed or partly refuted, and 5 to 10 further suspicions per batch were dropped.
The C-4 warning triage had 141 false positives in 275 classified rows (51 %) and 14 of 33 investigate rows
(42 %). Registered mechanisms mostly held; the weakness was in impact and reachability. A single-agent
static finding should not be trusted without a second review. The false-positive rate of the 13 never-reviewed
rows and the 3 reviewer finds is [unknown].

## 6. Avoimet kysymykset ja analyysin rajoitteet / Open questions and limits of the analysis

Kysymyksistä 84 ei ole tilassa "ratkaistu" (72 avoin, 6 osittain, 3 omistajan päätös, 3 ajonaikainen
todennus). Raportin luku 7 luokittelee ne vaiheen C sulkemisehdon 3 mukaan. Luokittelu on ehdotus
[päätelty]; kysymyskohtainen peruste on todistusraportin luvussa 7. Luokan 2 kohdat sijoitetaan
dokumenttiin `regressiotestaussuunnitelma.md` ja luokan 3 kohdat dokumenttiin `omistajan-paatokset.md`.
Pääistunto poikkesi raportista yhdessä kohdassa: Q25 (MONITORI-portti koodissa 14770, käyttöohjeessa
14900) on kirjattu T-014:ksi, mutta kumpi on oikein, on omistajan päätös (OP-11), joten se on luokassa 3
eikä 1. Luokan 1 neljä kysymystä on merkitty ratkaistuiksi `seuranta.md`:ään.

Of the questions, 84 are not "resolved" (72 open, 6 partly, 3 owner decision, 3 run-time verification).
The report's section 7 classifies them for closing condition 3 of phase C. The classification is a proposal
[inferred]; the per-question basis is in the evidence report section 7. Class 2 items are placed in
`regressiotestaussuunnitelma.md` and class 3 items in `omistajan-paatokset.md`.

| Luokka / Class | Merkitys / Meaning | Lkm / Count | Q-tunnukset / Q ids |
|---|---|--:|---|
| 1 | ratkaistu: dokumentit osoittavat vastauksen / resolved | 4 | Q3, Q24, Q26, Q53 |
| 2 | ajonaikainen todennus vaiheessa D (käännös, ajo, laite, verkko, CI) / run-time verification in phase D | 39 | Q12, Q21, Q28, Q33, Q34, Q35, Q36, Q37, Q39, Q40, Q43, Q44, Q45, Q46, Q48, Q52, Q56, Q63, Q67, Q68, Q73, Q80, Q86, Q87, Q89, Q91, Q93, Q95, Q96, Q97, Q98, Q100, Q102, Q103, Q105, Q106, Q107, Q108, Q110 |
| 3 | omistajan päätös / owner decision | 16 | Q9, Q10, Q11, Q25, Q29, Q47, Q60, Q75, Q77, Q78, Q81, Q90, Q99, Q101, Q111, Q112 |
| 4 | hyväksytty avoimeksi (staattinen, ei tarvita ennen korjauksia) / accepted as open | 25 | Q6, Q14, Q15, Q19, Q20, Q22, Q23, Q27, Q31, Q32, Q38, Q41, Q42, Q49, Q50, Q51, Q54, Q55, Q64, Q76, Q79, Q82, Q94, Q104, Q109 |
| | **Yhteensä / Total** | 84 | |

Luokan 1 viisi kysymystä näyttävät vastatuilta myöhemmissä dokumenteissa, mutta `seuranta.md`:n
kysymysrivejä ei ole päivitetty; rekisterin ylläpitäjän on vahvistettava ne [päätelty]. Kysymyksissä Q14,
Q31 ja Q42 C-3- ja C-4-aineisto voi jo vastata kysymykseen; ketjua ei jäljitetty loppuun [tuntematon].
/ The five class-1 questions look answered by later documents, but the question rows were not updated; the
register owner must confirm them [inferred]. For Q14, Q31 and Q42 the chain was not fully traced [unknown].

**Analyysin rajoitteet / Limits of the analysis.** Seuraavat 13 rajoitetta rajaavat johtopäätösten
luotettavuutta (raportin luku 8, L1-L13) [todennettu, ellei toisin mainita]:

1. Vain H-041 on vahvistettu toimitetussa ohjelmassa (`HkMaali.exe`, debuggerissa, rajaparilla 220/221),
   ja sekin käyttäjän hyväksymällä poikkeamalla ennalta lukitusta tuloksesta "ei vahvistettu". H-041:n
   lukupolkua ja 250-tapausta ei ajettu. / Only H-041 is confirmed in a shipped program, by a user-approved
   deviation; its read path and the 250 case were not run.
2. 26 muuta löydöstä on "toistettu" vain eristetyissä MSVC-konsolirakennelmissa, joissa tuotantokoodi on
   ennallaan mutta tila asetettu käsin (C-11 14, C-6 4, C-8 2, C-9 6). Se osoittaa mekanismin, ei
   laukaisijan esiintymistä käynnissä olevassa ohjelmassa; GUI-ohjelmat ovat bcc32-rakennelmia, joita
   ei ajettu. / 26 more findings are reproduced only in isolated MSVC console builds with hand-set state;
   the bcc32 GUI builds were never run.
3. bcc32-todisteet rajoittuvat rakennekoko- ja offset-kokeisiin (C-3), pragma-tilan jäljitykseen ja
   OMF-vertailuun (C-5), RTL-konsolikokeisiin (C-5, C-9), varoituksiin (C-4) ja VCL:n kaltaiseen
   FPU-kokeeseen (Q57). Yhtään bcc32-GUI-ohjelmaa ei ajettu. / bcc32 evidence is limited to probes,
   warnings and object comparison; no bcc32 GUI program was run.
4. Laite-, verkko-, SQL- tai usean aseman ajoa ei ole. Laiteprotokollat, MySQL, emiTag HTTP ja Emit-palvelin
   on pääteltävissä vain koodista ja dokumentaatiosta (Q96-Q100). Ainoa pistoketesti on loopback-TCP:n
   kieltäytyminen testikehyksessä; C-10:ssä ja C-12:ssa ei käännetty eikä ajettu mitään. / No device,
   network, SQL or multi-station run exists.
5. GUI-lomakkeet ovat suurelta osin lukematta: 284 tason 1 lomaketiedostoa (73 248 riviä); vain 8
   lomakeyksikköä luettiin liiketoimintalogiikan osalta (C-7); 41 muuta kirjoittavaa lomaketta, 152
   jäsennyskandidaattia ja arvonta-algoritmi puuttuvat. / GUI forms are mostly unread.
6. C-10:n ja C-12:n makroarvioijat ovat leksikaalisia: 238 lausekemakroa (176 nimeä) tallentuu arvolla 1.
   Relay `MAX_PORTTI` näkyy arvona 1, todellinen arvo 132 (`Juk/VDef.h:62`). Tallennettuja arvoja ei saa
   käyttää tosiasioina. / The C-10 and C-12 macro evaluators are lexical; recorded values must not be
   used as facts.
7. "Riippumaton tarkastus" tarkoittaa, että toinen agentti luki koodin ja yritti kumota väitteen (sama
   mallisuku, sama repositorio, ei ihmistä; staattinen). 13 löydöstä ei tarkastettu lainkaan, 43 vain
   osittain ja 3 ovat tarkastajan löytöjä [todennettu] (lukumäärät), [päätelty] (riippumattomuuden raja).
   / "Independent review" means a second agent re-read the code; no human, static.
8. Lähdeviitteiden validointi ei ole tekninen todennus: validaattori tarkistaa vain tiedoston olemassaolon
   ja rivin sijainnin eikä vertaa sisältöä; se skannaa vain `doc/arkkitehtuuri/*.md` ja `rekisteri/*.md`.
   / Reference validation checks existence and range only, never content.
9. Rekisteri seuraa tasoa vain 650 lähdetiedostolle (cpp, h, c). `.dfm`-, projekti- ja CI-tiedostoilla ei ole
   tasoa; C-10 inventoi 161 lomaketta erikseen; C-10:n ja C-12:n luku ei näy tiedostotasolla. / The register
   tracks levels for 650 source files only; forms, project and CI files have no level.
10. Taso 2 on "kohdennettu": sen 146 223 riviä ovat kokonaisten tiedostojen kokoja. Suurimmat tason 2
    tiedostot sisältävät paljon löydöksiä mutta luettiin löydöskohtien mukaan. / Level 2 is targeted; its
    lines are whole-file sizes, and the largest files were read by site.
11. Kaikki on sidottu revisioon `f7fe553` ja 2026-11-24 paikallisiin rakennelmiin (PE-taulut, bcc32-lokit);
    CI käyttää työkaluversiota v143, paikallinen rakennelma v145 (Q35). / Everything is pinned to revision
    `f7fe553` and to local builds of 2026-11-24; CI uses v143, local builds v145.
12. C-4:n varoituspolku jätti noin 93 % varoituksista (6 569 / 7 030) seulomatta ja noin 6 500
    luokittelematta; tunnetut puskurivirheet H-041, H-042, H-052, H-055 ja H-060 eivät tuottaneet varoitusta,
    joten varoituksen puuttuminen ei ole todiste. / About 93 % of warnings were left untriaged; known
    buffer defects produced no warning, so absence of a warning is no evidence.
13. Korjausprioriteetti on asetettu 115 löydökselle 177:stä; 62:lla sitä ei ole, niiden joukossa 12 korkeaa
    löydöstä (H-007, H-008, H-009, H-014, H-022, H-023, H-024, H-025, H-026, H-027, H-031, H-040). / Fix
    priorities exist for 115 of 177 H findings; 62 have none, among them 12 of the 34 high findings.

Kohta 13 kuvaa tilannetta ennen C-13:a: yhtenäinen riskiarvio (luku 9) antoi prioriteetin kaikille
176 avoimelle havainnolle. / Item 13 describes the state before C-13; the uniform assessment in
section 9 gave every open finding a priority.

## 7. Yhteiset juurisyyt / Common root causes

Rekisterin 176 avointa havaintoa ja 52 teknisen velan riviä luokiteltiin ensin koneellisesti 27
vikaluokkaan (`todisteet/c13/osaB/luokat.csv`) ja sen jälkeen etsittiin syyt, joiden vuoksi sama
vikaluokka toistuu (`todisteet/c13/raportit/osaB-juurisyyt.md`, Opus; yhdeksän mekanismia
tarkistettu koodista). Tuloksena on 15 juurisyytä. Tunnukset JS-1–JS-15 ovat pysyviä nimiä;
riskijärjestys on taulukon järjestys (suoraan aiheutetut P1-, P2- tai korkean vakavuuden rivit).

The 176 open findings and 52 debt rows were first classified mechanically into 27 defect classes and
then traced to the causes that make a class recur. The result is 15 root causes; the table is in
risk order.

| # | Juurisyy / Root cause | Mekanismi / Mechanism | Suorat rivit / Direct rows | Joista P1, P2 tai korkea / Of which P1, P2 or high | T-rivit / T rows | Ohjelmat / Programs |
|--:|---|---|--:|--:|---|---|
| 1 | **JS-1** Ulkoista syötettä ei validoida rajalla / no validated-input boundary | paketin, laitesanoman, tiedoston ja lomakekentän indeksit ja pituudet käytetään sellaisinaan | 27 | 10 | T-022, T-037 | kaikki |
| 2 | **JS-2** Lukkoparit ilman vartijaa, ei poikkeusrajaa / unscoped lock pairs | `Enter`/`Leave` käsin; ytimessä (`Hk/`, `Juk/`, `Tp/`, `dbboxm/`) ei yhtään `try`-lohkoa | 14 | 7 | T-036 (vain GUI) | HK ja viesti, konsoli ja GUI |
| 3 | **JS-3** Kapasiteetti ei seuraa ajonaikaista kokoa / capacity not tied to run-time size | puskuri on käännösaikainen vakio, tietueen tai tekstin koko ajonaikainen; rajaamaton `swprintf` | 14 | 5 | ei omaa riviä | kaikki, kokoonpanoittain |
| 4 | **JS-4** Sivuvaikutukset globaalien lukkojen alla / side effects under coarse locks | tulostus, lokitus, soketit, SQL ja virheikkunat lukon sisällä; `writeerror_w` odottaa pääsäiettä | 9 | 3 | T-031, T-032, T-033 | HK ja viesti |
| 5 | **JS-5** Koko tietue, viimeisin voittaa / whole-record last-writer-wins | replikointi ja lomakkeet korvaavat koko tietueen ilman versiota tai alkuperää | 8 | 3 | ei omaa riviä (H-016) | pääosin HK |
| 6 | **JS-6** Muistitila rakennetaan vain käynnistyksessä / start-up-only state | vaiheen vaihto nollaa tapauskohtaisesti; johdetut rakenteet ylläpidetään käsin pareittain | 14 | 2 | T-038, T-039 | vaihe: HK; johdetut: molemmat |
| 7 | **JS-8** Kahvoilla ja osoittimilla ei omistajaa / no ownership rule | osoitin kopioidaan rakenteiden välillä; sulkija ei voi nollata kutsujan kenttää | 11 | 2 | T-042, T-020 | HK ja viesti |
| 8 | **JS-9** Laiteyhteydet käsin, ei elävyysvalvontaa / hand-rolled device connections | jokaisella lukijalla oma avaus-, uusinta- ja virhekoodi | 5 | 2 | T-048 | kaikki lukijoita käyttävät |
| 9 | **JS-7** Säikeet lipulla ja `Sleep`illä / flag-and-sleep threads | ei odotusta; `vaiheenvaihto` on ainakin seitsemässä säiesilmukassa poistumisehto, muissa tauko | 8 | 2 | T-030 | HK (vaihe), molemmat (lopetus) |
| 10 | **JS-10** Jaettu tila ja VCL ilman omistajaa / shared state and VCL without owner | globaalit ja staattiset ilman lukkoa; VCL-oliot työsäikeistä | 17 | 1 | T-003, T-033 | HK ja viesti; VCL vain GUI |
| 11 | **JS-12** Kopioimalla haarautunut koodi / copy-forked code | HK ja viesti, konsoli ja GUI; korjaus vain toisessa haarassa, myös ytimessä | 5 | 0 (pahentaa 5) | T-034, T-008, T-035 | molemmat |
| 12 | **JS-13** Todennuksen puute / verification gap | varoitustaso matala, testejä ei ajeta, julkaisupakettia ei käynnistetä | 3 | 0 (pahentaa 2) | T-021, T-044 | kaikki |
| 13 | **JS-11** Virheet ikkunoina, ei paluuarvoina / errors as dialogs | `void`-funktiot ja modaali-ikkunat; ei atomista kirjoitusta | 10 | 0 | T-040, T-041 | kaikki |
| 14 | **JS-14** Asetusten ja valitsinten ajautuminen / settings and switch drift | neljä jäsennintä, kaksi editoria, valitsimet projektien ulkopuolella | 4 | 0 | T-045, T-050, T-051, T-017, T-024, T-052 | kaikki |
| 15 | **JS-15** Kaksi kääntäjää ilman siirrettävyyskäytäntöä / two toolchains | RTL-, pakkaus- ja merkistöerot MSVC:n ja bcc32:n välillä | 4 | 0 | T-004, T-016, T-023, T-025–T-027 | MSVC ja bcc32 |
| – | Yksittäiset viat ilman yhteistä syytä / isolated defects | – | 23 | 3 (H-001, H-002, H-111) | – | – |

Havaintokohtainen kytkentä juurisyihin on raportin liitteessä (luku 9); kokonaisuudet, joissa
kunkin juurisyyn korjaukset ovat, ovat `korjaussuunnitelma.md`:n luvussa 2.

**Erien yli näkyvät syyt, joita yksittäinen erä ei voinut nähdä** [todennettu koodista, ellei
toisin merkitty]:

- `writeerror_w` (`cbTp/cbTpApu.cpp:467-490`) on yksi apufunktio havaintojen H-009, H-019, H-021,
  H-153 ja H-167 sekä T-032:n ja T-033:n takana: se lähettää viestin pääsäikeelle synkronisesti ja
  odottaa vastausta kutsujan lukkojen ollessa varattuina.
- `vaiheenvaihto`-lippu on poistumisehto ainakin seitsemässä säiesilmukassa (H-014, H-031, H-094,
  H-097; juurisyyraportti löysi viisi, riippumaton tarkastus seitsemän, luettelo
  `korjaussuunnitelma.md`:n kohdassa KK-C13-8).
- HK lukee vastaanotetun tietueen ennen lukkoa ja viesti lukon sisällä (`Hk/HkCom32.cpp:260` ja
  `:326`; `Juk/VCom.cpp:579`): haarojen ero ulottuu ytimeen, ei vain lomakkeisiin.
- Kiinteä koko vastaan ajonaikainen määrä (JS-3) selittää H-041:n, H-042:n, H-025:n, H-129:n ja
  H-171:n, mutta ei P1-verkkovikoja (ne ovat JS-1) [osittain vahvistettu].
- Testien puuttumista ei voitu osoittaa yksipuolisten korjausten syyksi [ei osoitettu].

**Rekisteriin kirjattu laajennus.** Juurisyyanalyysin koodintarkistuksessa löytyi H-116:n toinen
ilmenemismuoto, jonka pääistunto luki koodista: `errorMsgHandler` (`cbTp/cbTpApu.cpp:445-463`)
tulkitsee `ShowModal`in tuloksen 2 valinnaksi "Poistu ohjelmasta", ja lomakkeella
(`cbTp/UnitSelect3.cpp:38-60`) ei ole sulkemiskäsittelijää, joten ikkunan sulkeminen X:llä
palauttaa `mrCancel`in eli arvon 2. Minkä tahansa GUI-virheikkunan sulkeminen X:llä lopettaisi
siis ohjelman [todennettu koodi; VCL:n paluuarvo ja kokonaisvaikutus päätelty, ei ajettu]. Tämä on
kirjattu H-116:n laajennukseksi eikä uudeksi tunnukseksi (sama mekanismi), ja H-116:n prioriteetti
on nostettu P3:sta P2:ksi; se palaa P3:ksi, jos GUI-ajo osoittaa, ettei X palauta arvoa 2.

/ Cross-batch causes: one helper (`writeerror_w`) behind seven rows; the stage flag as an exit
condition in five thread loops; the HK/relay fork reaching into the core. A second manifestation of
H-116 (closing any GUI error dialog with X would end the program) was read from the code and
recorded as an extension of H-116, raised to P2 pending a GUI run.

## 8. Rekisterin yhtenäistäminen / Unifying the register

Yhtään tunnusta ei poistettu, numeroitu uudelleen eikä yhdistetty. Yhtenäistäminen on tehty
viittauksin: `todisteet/c13/osaB/suhteet.csv` (221 suhdetta) ja `rekisteri/riskiarvio.csv`:n
sarakkeet `kk` ja `kk_muut`.

| Suhde / Relation | Lkm / Count | Esimerkkejä / Examples |
|---|--:|---|
| sama koodikohta / same code site | 61 | H-149–H-152 ja H-158 (`autofile`); H-128, H-129, H-102, H-013 (`teejarj`, `setpos`) |
| korjaus ratkaisee / fix resolves | 60 | T-042 → H-149, H-151, H-152; T-038 → H-131–H-134, H-141, H-143 |
| korjattava yhdessä / fix together | 35 | H-129 + H-134; H-008 + H-060; H-025 + H-048 + H-050; H-040 + H-148 + H-047 |
| seuraus / consequence | 27 | H-161 → H-160; H-165 → H-163; H-031 → H-094 |
| sama vika / same defect | 15 | H-061 on H-032:n luokka toisella kuljetuksella; H-097 ja H-014 (sama säikeen pysähtyminen); H-144 toistaa H-019:n; C-12:n C3 = H-063, B1 = H-036 |
| ristiriita / inconsistency | 12 | ks. alla |
| HK–viesti-vastine / HK–relay counterpart | 11 | H-117 ja H-121; H-130 ja H-139 |

Suhteista osa on [päätelty] rivien tekstistä; varmuus on merkitty riveittäin.

Rekisterin epäjohdonmukaisuudet ja niiden käsittely C-13:ssa:

| Epäjohdonmukaisuus / Inconsistency | Käsittely / Handling |
|---|---|
| 61 avoimella havainnolla ei ollut prioriteettia, niiden joukossa 12 korkeaa (H-007, H-008, H-009, H-014, H-022–H-027, H-031, H-040), joista 10 oli toistettu testikehyksessä | prioriteetti annettu kaikille (luku 9) |
| Vakavuus ja prioriteetti eri suuntiin (H-061 keskitaso ja P1; H-002 korkea ja P4) | vakavuus ja prioriteetti ovat eri ulottuvuuksia (luku 9); perustelu rivillä |
| H-134 P3, vaikka se korjataan H-129:n (P2) kanssa | rivin prioriteetti on itsenäinen arvio; kokonaisuus ajoitetaan korkeimman jäsenensä mukaan |
| Sama havainto kahdessa aiemmassa kokonaisuudessa (H-149, H-150, H-161, H-166, H-170, H-176, T-043) | yksi ensisijainen kokonaisuus, muut sarakkeessa `kk_muut` |
| H-134:n rivi viittaa KK-C9-2:een, jota kokonaisuuden taulukko ei luettele | ensisijainen KK-C13-3 (C-12:n sulkemisen päätös), toissijainen KK-C9-2 |
| Kokoomarivit (H-126, H-127, H-142–H-144, H-156, H-157, H-170, H-176) kätkevät riviä vakavampia kohtia | merkitty `riskiarvio.csv`:n sarakkeeseen `huomio`; puretaan ennen korjausta (`korjaussuunnitelma.md` luku 8) |
| H-021:n erä: tunnusvälin mukaan C-1, lähdesarakkeen mukaan C-2 | kirjattu; ei vaikuta arvioon |
| `CHECKPOINT.md`:n ja muiden asiakirjojen vanhentuneet luvut (S1–S21, luku 1) | korjattu osassa I |

/ Nothing was deleted, renumbered or merged; unification is by references (221 relations). The
table lists the register inconsistencies and how C-13 handled each.

## 9. Yhtenäinen riskiarvio / Uniform risk assessment

### 9.1 Ulottuvuudet / Dimensions

Jokaisella havainnolla on `rekisteri/riskiarvio.csv`:ssä erilliset ulottuvuudet. Vakavuus kuvaa
teknistä vaikutusta, jos vika laukeaa; prioriteetti on korjausjärjestyksen arvio. Ne pidetään
erillään: korkea vakavuus voi olla P4 (H-002: vain vanhan muodon tiedosto) ja keskitaso P2 (H-060,
H-150).

| Ulottuvuus / Dimension | Arvot ja jakauma (176 avointa) / Values and distribution |
|---|---|
| tekninen vakavuus / technical severity | korkea 35, keskitaso–korkea 2, keskitaso 73, matala–keskitaso 20, matala 46 |
| saavutettavuus / reachability | verkko oletusasetuksin 9, verkko asetuksella 5, laite 6, normaalikäyttö 45, asetus 45, operaattori 51, paikallinen tiedosto 12, ei toimitetussa 3 |
| todennus / verification | luvun 4 luokat |
| vaikutus tulosten oikeellisuuteen / effect on results | suora 47, epäsuora 104, ei 25 |
| vaikutus ajanoton jatkuvuuteen / effect on continuity | pysäyttää 59, häiritsee 32, ei 85 |
| palautettavuus / recoverability | itsestään 7, uudelleenkäynnistys 64, käsin 83, ei palautettavissa 12, ei sovellu 10 |
| tietoturva / security | verkosta 15, paikallinen 11, ei 150 |

Vakavuus poikkeaa rekisterin tekstistä kolmella rivillä (H-033 matala → korkea, H-107 keskitaso →
keskitaso–korkea, H-158 matala → keskitaso); perustelu on rivin `huomio`-sarakkeessa. Rekisterin omaa vakavuussaraketta ei
muutettu.

### 9.2 P1–P5: käyttö ja yhtenäinen määritelmä / P1–P5: usage and the uniform definition

Asteikko P1–P5 säilyy. Alkuperäinen määritelmä (`muistiturvallisuus.md` luku 15) perustui siihen,
kuka vian voi laukaista. Erät C-6–C-12 käyttivät P2:ta myös vioille, jotka toistuvat tavallisessa
käytössä ja pysäyttävät palvelun tai korruptoivat tuloksia ilman hyökkääjää (H-149, H-171, H-150,
H-159, H-160). C-13 kirjaa molemmat käytöt yhdeksi määritelmäksi
(`todisteet/c13/raportit/osaC-riskiarvio.md` luku 2); rivi saa korkeimman tason, jonka ehdon se
täyttää.

| P | Ehto / Condition | Vastaavuus aiempaan / Correspondence |
|---|---|---|
| **P1** | Ajanottoverkon kolmas osapuoli voi laukaista oletusasetuksin, ja vaikutus on muisti- tai tiedostokirjoitus, tiedoston korvaus tai tulosten hiljainen korruptio. | C-4:n P1; vaikutusehto nyt eksplisiittinen |
| **P2** | (a) Verkosta oletusasetuksin, mutta vaikutus keskitasoa tai vain saatavuuteen; (b) verkosta asetuksella tai ajanottolaitteelta, vaikutus vähintään keskitasoa; (c) ilman hyökkääjää tavallisessa käytössä pysäyttää, jäädyttää tai kaataa ohjelman tai tarvittavan säikeen, tai korruptoi, hukkaa tai kohdistaa väärin tuloksia hiljaa, ja toistuu tai on todennäköinen. | C-4:n P2 = (b); C-6–C-12:n käytäntö = (c); (a) uusi täsmennys |
| **P3** | Saavutettavissa toimitetussa ohjelmassa, mutta vaikutus kapeampi tai edellytys harvinaisempi (I/O-virhe, harvinainen asetusyhdistelmä, hyvin suuri kilpailu, operaattori pitää dialogia auki). | C-4:n ja C-10:n P3 yhdistettynä |
| **P4** | Vaatii virheellisen tai käsin muokatun paikallisen tiedoston tai operaattorin virheen, tai matala vakavuus toiminnallisella vaikutuksella. Matala vakavuus ei nouse P4:n yläpuolelle. | ennallaan |
| **P5** | Kosmeettinen, kuollut tai toimittamaton koodi, kokoomarivit (vakavammat kohdat merkitty erikseen). | ennallaan |

Säännöt: (1) todennuksen vahvuus ei laske vahvistetun mekanismin prioriteettia, mutta todentamaton
saavutettavuusoletus näkyy `huomio`-sarakkeessa; (2) yhdessä korjattavat rivit pitävät oman
arvonsa, ja kokonaisuus ajoitetaan korkeimman jäsenensä mukaan; (3) verkosta oletusasetuksin
saavutettava vika, jonka vaikutus on tuntematon, arvioidaan pahemman vaihtoehdon mukaan, kunnes
vaikutus on todennettu (H-061) [pääistunnon sääntö].

### 9.3 Jakauma ja muutokset / Distribution and changes

| | ei asetettu / none | P1 | P2 | P3 | P4 | P5 |
|---|--:|--:|--:|--:|--:|--:|
| Ennen C-13:a / before | 61 | 4 | 18 | 37 | 37 | 19 |
| C-13:n jälkeen / after | 0 | 5 | 35 | 62 | 47 | 27 |

- **P1 (5):** H-032, H-033, H-061, H-071, H-072. H-033 oli erässä C-2 vakavuudeltaan matala eikä
  sillä ollut prioriteettia; riippumaton tarkastus osoitti koodista, että verkosta tuleva
  vaiheindeksi indeksoi kekotaulukkoa tarkistamatta (luku 11, K1).
- **Uudet P2 (aiemmin ilman prioriteettia):** H-007, H-008, H-022, H-023, H-024, H-025, H-026,
  H-027, H-028, H-029, H-031, H-040, H-128. Kaikki paitsi H-028, H-029 ja H-128 ovat korkean
  vakavuuden rivejä eristä C-1–C-3; H-128 korjataan H-129:n kanssa.
- **Nostetut:** H-111 P3 → P2 (lähtöportin aika tallentuu hiljaa maaliajaksi), H-116 P3 → P2
  (luku 7), H-151 P3 → P2 (H-149:n HK-vastine), H-164 P3 → P2 (sama vaikutus kuin H-159), H-064 ja
  H-152 P4 → P3, H-153 P4 → P3 (H-116:n laajennuksen seuraus), H-158 P5 → P3, H-069 P5 → P4.
- **Lasketut:** ei yhtään.

Opus-arvion luonnoksesta pääistunto poikkesi neljässä kohdassa [ehdotus, perustelu rivillä]:
H-061 pidettiin P1:ssä (arvio ehdotti P2:ta, koska vakavuus on keskitaso ja vaikutus tuntematon;
uutta näyttöä ei ole, ja C-4:n sulkemisessa hyväksytty arvo säilyy säännön 3 nojalla); H-116
nostettiin P2:ksi koodista luetun laajennuksen perusteella; H-033 nostettiin P4:stä P1:ksi ja sen
tekninen vakavuus korkeaksi riippumattoman tarkastuksen ja pääistunnon koodinluvun perusteella; ja
H-153 nostettiin P3:ksi. Rekisterin omaa vakavuussaraketta ei muutettu; H-033:n riville on lisätty
C-13:n huomautus.

Kaksi rinnastusta, joita tarkastus kysyi: H-001 (P3) ja H-171 (P2) ovat molemmat deterministisiä
ylikirjoituksia, mutta H-171 laukeaa jokaisessa ViestiWinin kilpailun avauksessa ja H-001 vain
tulosteessa, jossa on ampumapaikat (ampumahiihtoviesti); H-064 (P3) ja H-107 (P2) ovat molemmat
asetuksesta riippuvia kaatumisia, ja ero perustuu siihen, kuinka tavallinen asetus on. Kumpikin ero
on [päätelty] ja riippuu käyttötavasta (OP-14). H-009 on P3, koska lukkiutuminen vaatii
ampumasakkojen käytön tai virhepolun ja ajoituksen; sitä ei ole laskettu todennuksen vahvuuden
vuoksi. Avoimiksi jätetyt rajatapaukset, joissa
prioriteetti riippuu käyttötavasta (H-111, H-028, H-001, H-115, H-030), ovat
`omistajan-paatokset.md`:n kohdassa OP-14.

P2:ssa on 35 riviä. Määritelmää ei kavennettu luvun pienentämiseksi; järjestys tason sisällä on
`korjaussuunnitelma.md`:n luvussa 4.2.

/ P1-P5 is kept. One definition now covers both the original reachability axis and the later usage
(recurring stop or silent corruption in ordinary use is P2). Every open finding has a priority: P1 5,
P2 35, P3 62, P4 47, P5 27. Nine priorities were raised, none lowered; the main session deviated
from the Opus draft on H-061 (kept at P1), H-116 (raised to P2), H-033 (raised to P1 after the
independent review) and H-153 (raised to P3).

## 10. Vaiheen C kokonaistulos / Overall result of phase C

**Mitä auditointi osoittaa** [päätelty rekistereistä]:

1. Ohjelmisto toimii tuotannossa, ja sen tiedosto- ja sanomarakenteet ovat molemmilla kääntäjillä
   samat. Auditointi ei löytänyt syytä kirjoittaa ohjelmistoa uudelleen.
2. Vakavin riski on rajattu ja korjattavissa: viisi verkosta oletusasetuksin laukeavaa vikaa (P1)
   ja 35 P2-vikaa. Niille on rajattu korjaus ilman muoto- tai työnkulkumuutosta kolmea poikkeusta
   lukuun ottamatta: viestin lähetysjonon tietuepituus (H-025) sekä H-029 ja H-071:n kohdepolun
   rajaus, jotka vaativat ensin omistajan päätöksen (OP-4, OP-5).
3. Viat eivät ole satunnaisia: 153 avoimesta havainnosta 176:sta palautuu 15 juurisyyhyn, ja
   neljä ensimmäistä (syötteen validointi, lukkoparit, kapasiteetti, sivuvaikutukset lukon alla)
   ovat suorana syynä 25 riville, jotka ovat P1, P2 tai korkean vakavuuden rivejä (luku 7; laskettu
   juurisyyraportissa ennen yhtenäistä riskiarviota).
4. Todennuksen taso on epätasainen. Yksi vika on vahvistettu toimitetussa ohjelmassa, 26
   mekanismia on toistettu koeohjelmalla tai testikehyksessä, ja loput perustuvat koodin lukemiseen
   ja toisen agentin tarkastukseen. GUI-ohjelmia ei ole ajettu.
5. Kattavuus on osittainen: 36 % riveistä on vain inventoitu, eikä yhtäkään tuotantotiedostoa ole
   katselmoitu kokonaan. Havaintojen määrä on alaraja, ei kokonaismäärä.

**Jäljellä olevat riskit ja rajoitteet:**

- Tutkimattomissa alueissa (284 lomaketiedostoa, arvonta-algoritmi, 152 jäsennysehdokasta, 153
  funktion staattista muuttujaa) voi olla samojen juurisyiden vikoja, joita ei ole rekisterissä.
- Saavutettavuus ja seuraukset ovat suurelta osin pääteltyjä; korjausta edeltävä toisto (testi)
  voi kumota tai muuttaa havainnon. Auditoinnin tarkastuksissa noin neljännes – puolet
  tarkastetuista väitteistä tarkentui.
- bcc32-GUI-ohjelmien käytös voi poiketa MSVC-koeohjelmista (muistinhallinta, RTL).
- Riskiarvion prioriteetit ovat yhden arvioijan (Opus) ja pääistunnon harkintaa yhtenäisin
  perustein; ne eivät ole mittaustuloksia.
- Laite-, verkko- ja SQL-ympäristöissä ei ole ajettu mitään.

**C-13:n valmiusehdot** (`suunnitelma.md`, "C-13 vaihe 1: rajaus"):

| # | Ehto / Criterion | Toteutuma / Outcome |
|--:|---|---|
| 1 | Erien tavoitteiden toteutuma ja tutkimatta jääneet alueet / batch goals and unexamined areas | luvut 2, 3 ja 6; `todisteet/c13/raportit/osaA-kattavuus.md` |
| 2 | Jokainen H, T ja Q pohjassa; todennuksen laji; kumotut ja osittaiset merkitty / every id in the base | `todisteet/c13/osaA0/`; luku 4 |
| 3 | Päällekkäisyydet ja juurisyyt viittauksin; mitään ei poistettu / by reference, nothing deleted | luvut 7 ja 8 |
| 4 | Riskiulottuvuudet ja prioriteetti jokaisella avoimella H:lla; P1–P5:n käyttö ja vastaavuus / risk dimensions and priority | luku 9; `rekisteri/riskiarvio.csv` |
| 5 | Jokainen avoin H ja T kokonaisuudessa tai perustellusti ulkopuolella / every open H and T in a package | `rekisteri/korjauskokonaisuudet.csv`; ulkopuolella vain T-010 (korjattu) |
| 6 | Testaussuunnitelma kattaa P1–P3-kokonaisuudet / test plan for P1–P3 packages | `regressiotestaussuunnitelma.md` luku 4 |
| 7 | Viisi vaihetta, teknologiaa ei lukita / five stages, no lock-in | `korjaussuunnitelma.md` luku 6 |
| 8 | Omistajan päätökset koottu / owner decisions collected | `omistajan-paatokset.md` (OP-1–OP-18) |
| 9 | Opus-tarkastus tehty ja käsitelty, validaattori läpi, paikallinen commit / review, validator, commit | luku 11; validaattorin ja commitin tila `CHECKPOINT.md` |

Riippumattoman tarkastuksen arvio ehdosta 6: osittain, koska arkkitehtuurikokonaisuuksien
KK-C13-19, -20 ja -23 testit määritellään vasta niiden suunnittelussa; P1–P3-korjauskokonaisuudet
on katettu.

## 11. Riippumaton kokonaisarvio ja sen käsittely / Independent overall review and its handling

Opus-tarkastaja (`independent-verifier`) sai tehtäväkseen kyseenalaistaa synteesin johtopäätökset
yhdeksällä kysymyksellä: suunnitelman ulkopuolelle jääneet riskit, prioriteettien johdonmukaisuus,
juurisyyt, kokonaisuuksien koko, järjestyksen turvallisuus, testauksen riittävyys, yhteensopivuus,
perusteettomat teknologia- tai ominaisuuspäätökset ja D-vaiheen aloitettavuus. Raportti:
`todisteet/c13/raportit/osaH-tarkastus.md` (17 huomiota K1–K17). Tarkastaja ajoi rekisterien
tarkistukset, laski luvut uudelleen rekistereistä ja luki koodista rajatapaukset. Sen yleisarvio:
synteesi kelpaa D-vaiheen lähtökohdaksi yhden korjauskierroksen jälkeen.

Pääistunto ei hyväksynyt huomioita sellaisinaan, vaan tarkisti kiistanalaiset koodista
(K1, K6, K10, K14, K16) ja rekistereistä. Käsittely:

| # | Huomio / Finding | Arvio / Verdict | Toimenpide / Action |
|---|---|---|---|
| K1 | H-033 on verkosta oletusasetuksin laukeava kekokirjoitus, mutta P4:ssä ja ensimmäisen erän ulkopuolella | **hyväksytty**; luettu `Hk/HkCom32.cpp:274-279` ja `Hk/HkDat.cpp:206` | P1, tekninen vakavuus korkea, siirretty KK-C13-1:een ja ensimmäiseen erään; luvut päivitetty |
| K2 | H-116:n laajennus pitää; H-153 on nyt saavutettavissa tavallisella X-painalluksella | hyväksytty | H-153 P4 → P3; X:n merkitys päätökseksi (OP-9); H-116:n toisto GUI:ssa ensimmäisen erän valmisteluihin |
| K3 | H-116 ja `writeerror_w`-muutos koskevat samaa käsittelijää | hyväksytty | järjestys ja "ei rinnakkain" kirjattu; `riippuu_osat` |
| K4 | Osa riippuvuuksista oli vain tekstissä | hyväksytty | uusi rekisterisarake `riippuu_osat`, jonka skripti tarkistaa; H-087 käytännössä aaltoon 2; SQL-korjausten järjestys perusteltu ja sääntö 4.5(3) korjattu |
| K4 | H-040, H-047 ja H-148 pitäisi korjata yhdessä | **osittain**: H-040:n lukon vapautus on itsenäinen eikä pahenna muita | H-040 aallossa 1; H-047 ja H-148 yhdessä myöhemmin |
| K5 | H-008 ja H-060 jakavat kutsupaikat, mutta olivat rinnakkain tehtävissä kokonaisuuksissa | hyväksytty | H-008 siirretty KK-C13-2:een, sama PR kuin H-060 |
| K6 | H-013:n korjaus käynnistää silmukan, joka tallentaa ja lähettää tietueita jokaisessa vaiheen vaihdossa | **hyväksytty**; luettu `Hk/HkIx.cpp:1826-1840` | H-013 pois ensimmäisestä erästä, oma PR selvityksen jälkeen |
| K7 | Ensimmäisen erän aukot: testit H-102:lle ja H-013:lle, tynkälukija, GUI-käsikirjoitukset, H-128:n toinen puoli, tavuvertailun vertailukohta, GUI-ohjelmat hyväksymisajossa | hyväksytty | lisätty `korjaussuunnitelma.md`:n lukuihin 3.3 ja 7 sekä testaussuunnitelmaan |
| K8 | Korjaustavan valintaa vaativia havaintoja ei ollut päätösasiakirjassa | hyväksytty | uusi taulukko `omistajan-paatokset.md`:ssä: oletuskorjaus, joka ei odota päätöstä, ja päätös, joka tarkentaa sitä |
| K9a | H-001 ja H-171, H-064 ja H-107 sekä H-009:n perustelu | **osittain**: prioriteetteja ei muutettu | erot kirjattu lukuun 9.3; H-009:n perustelu korjattu (ei todennuksen vahvuuden vuoksi) |
| K10 | `vaiheenvaihto` on poistumisehto ainakin seitsemässä silmukassa, ei viidessä | **hyväksytty**; kaikki seitsemän riviä luettu | luettelo KK-C13-8:ssa; luvut 7 korjattu |
| K11 | Ytimen 56 jäsennysehdokasta ovat samaa luokkaa kuin P1-viat, mutta jätetty vaiheeseen 3; J7-5 sijoittamatta | hyväksytty | läpikäynti vaiheeseen 1 (KK-C13-1); J7-5 ennen KK-C13-15:tä |
| K12 | Yleistykset "jokaiselle P2:lle rajattu korjaus", KK-C10-4:n vaihe, KK-C13-12:n prioriteetti | hyväksytty | tekstit korjattu poikkeuksineen |
| K13 | Luku- ja viitevirheet (puuttuva luku 11, Q25, GUI-havaintojen määrä, PR-määrä, OP-11:n ajoitus, "TPsource ennallaan", `seuranta.md`) | hyväksytty | korjattu |
| K14 | Yhteensopivuusväitteet pitävät (H-025, H-041, H-071, H-171, H-129); pinon riittävyys ristiriidassa; uusi ehdokas: asetustiedoston nimi 80 merkin puskuriin | hyväksytty; puskuri luettu `Juk/VInit.cpp:433-445` ja `Hk/HkInit.cpp:331-343` | H-041: pino vai keko jätetty toteutukseen perusteluineen; ehdokas kirjattu H-052:n laajennukseksi ja KK-C12-2:n ensimmäiseen PR-sarjaan |
| K15 | Teknologiaa ei ole lukittu; suositukset on merkitty ehdotuksiksi | vahvistettu | – |
| K16 | Yhdistämättömät haarat muuttavat tuotantokoodia revision `f7fe553` jälkeen, myös korjattavissa kohdissa | **hyväksytty**; neljä marraskuun haaraa todennettu paikallisista viitteistä | pohjan tarkistus ensimmäisen erän valmisteluihin; OP-18 |
| K17 | Pienet huomiot: H-030 riippuu arkkitehtuurikokonaisuudesta, lähtötaso ajettu likaisesta puusta, H-072:n kaikki `EXTRA`-tapaukset, H-127 N-22 | hyväksytty | kirjattu |

Tarkastuksen kolme heikointa kohtaa ja niiden tila: (1) priorisointi luotti erän C-2 tutkimattomaan
vakavuuteen (H-033) – korjattu, mutta sama voi koskea muita erien C-1–C-3 matalia rivejä, joita ei
käyty uudelleen läpi; (2) järjestyssäännöt olivat tekstissä – nyt rekisterissä ja tarkistettavissa;
(3) ensimmäinen erä nojasi testiympäristöihin, joita aalto 0 ei tuota – lisätty valmisteluihin.

Avoimiksi jäävät tarkastajan kysymykset, jotka ratkeavat vain ajamalla: palauttaako VCL tässä
C++Builder-versiossa X:stä arvon 2 (H-116, H-153); mitä alueen ulkopuolinen vaiheindeksi tekee
ajossa (H-033); riittääkö työsäikeiden pino bcc32:lla (H-041); onko `swprintf` 80 merkin
puskuriin rajattu jommassakummassa ajonaikaisessa kirjastossa (H-052:n laajennus).

/ An Opus reviewer challenged the synthesis on nine questions and returned 17 findings. The main
session checked the disputed ones against the code and the registers instead of accepting them
as given. One must-fix was confirmed: H-033 is a network-default heap write that had been left at
P4; it is now P1, in KK-C13-1 and in the first batch. Other accepted corrections: H-013 removed
from the first batch (its fix would change network behaviour), part-level dependencies moved from
prose into a checked register column, H-008 moved next to H-060, seven (not five) stage-flag thread
loops, the 56 core parse candidates moved to stage 1, fix-choice decisions added to the owner
document, a re-base and re-check step for unmerged branches. Two points were only partly accepted
(H-040 stays an independent wave-1 fix; H-001 and H-064 keep their priorities with the reasons
written down).

## 12. Vaiheen C sulkeminen / Closing of phase C

Käyttäjä hyväksyi 2026-11-26 erän C-13 ja koko vaiheen C sulkemisen sekä korjaus- ja
testaussuunnitelman D-vaiheen lähtökohdaksi. Hyväksyntä ei ole lupa aloittaa tuotantokoodin
korjaamista.

| # | Sulkemisen edellytys / Closing condition (`suunnitelma.md`) | Toteutuma / Outcome |
|--:|---|---|
| 1 | C-13 valmis ja käyttäjän hyväksymä | kyllä (commit `e8daa65`, hyväksyntä 2026-11-26) |
| 2 | Jokaisen erän C-1–C-13 tila "suljettu" | kyllä: C-4–C-10 ja C-12 aiemmilla päätöksillä, C-11 päätöksellä 2026-11-24, C-1–C-3 ja C-13 tällä päätöksellä (`seuranta.md`) |
| 3 | Avoimet kysymykset luokiteltu | kyllä: 32 ratkaistu, 39 ajonaikaiseen todennukseen D-vaiheessa, 16 omistajan päätökseksi, 25 hyväksytty avoimiksi (luku 6) |
| 4 | Korjaus- ja testaussuunnitelma hyväksytty D-vaiheen lähtökohdaksi | kyllä, käyttäjän päätös 2026-11-26 |
| 5 | Omistajan päätöslista toimitettu; päätöksiä odottavat kokonaisuudet merkitty | kyllä: `omistajan-paatokset.md` käyttäjällä, 18 kohtaa avoinna; riippuvat kokonaisuudet rekisterin sarakkeessa `omistajan_paatokset` |
| 6 | `CHECKPOINT.md` ja `README.md` kuvaavat lopputuloksen | kyllä |
| 7 | Validaattori menee läpi | kyllä (`tarkista_auditointi.py`, `synteesi.py --tarkista`, `tarkistus_a0.py`) |
| 8 | Kaikki commitoitu paikallisesti | kyllä; pushia ei ole tehty |

**Mitä sulkeminen tarkoittaa ja mitä ei.** Vaihe C on suljettu, koska sen suunnitellut erät on
tehty ja niiden tulokset koottu. Se ei tarkoita, että koko lähdekoodi olisi katselmoitu:

- 474 tiedostoa ja 97 956 riviä (36 %) on vain inventoitu skriptillä; niiden koodia ei ole luettu.
- Yhtäkään tuotantotiedostoa ei ole luettu kokonaan vikojen varalta (taso 4: 0).
- 284 GUI-lomaketiedostoa, arvonta-algoritmi, 41 muuta kilpailutietoja kirjoittavaa lomaketta,
  152 jäsennysehdokasta ja 153 funktion staattista muuttujaa ovat tutkimatta.
- 177 havainnosta yksi on vahvistettu toimitetussa ohjelmassa ja 26 toistettu koeohjelmalla tai
  testikehyksessä; muut perustuvat koodin lukemiseen. GUI-ohjelmia ei ole ajettu.
- Havaintojen määrä on alaraja. H-033:n tapaus osoitti, että myös kirjattujen havaintojen
  aiempi vakavuusarvio voi olla väärä; rajattu seulonta on D-vaiheen ennakkotehtävä
  (`korjaussuunnitelma.md` luku 9.1).

Jatko: D-vaiheen ennakkotehtävät E-1–E-4 (`korjaussuunnitelma.md` luku 9) ja vaihe K, ohjelmiston
tietopohja ja käsikirja (`k-vaihe-suunnitelma.md`). Kumpaakaan ei ole aloitettu.

/ The user approved the closing of C-13 and of phase C on 2026-11-26 and accepted the fix and
test plans as the starting point of phase D, which is not permission to start fixing. All eight
closing conditions are met. Closing does not mean the code base has been fully reviewed: 36 % of
the lines are inventory only, no production file was read in full, the GUI forms are largely
unread, one defect is confirmed in a shipped program and 26 mechanisms are reproduced; the number
of findings is a lower bound. Next: the preparatory tasks E-1..E-4 and phase K, neither started.
