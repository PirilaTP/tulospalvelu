# Globaali muuttuva tila / Global mutable state (C-8 vaihe 1 / step 1, **suljettu / closed**)

Erä C-8, vaihe 1: globaalien ja staattisten muuttujien inventaario (osa A), elinkaari ja
vaiheen vaihto (osa B), johdettujen rakenteiden eheys (osa C), säikeiden yhteinen tila, jota
C-6 ei kattanut (osa D), ja riippumaton tarkastus (osa E). Revisio `be52dbf`
(tuotantolähdekoodi `TPsource/` ennallaan revisiosta `f7fe553`; sen jälkeen lisätty vain
testitiedostoja). Polut suhteessa `TPsource/V52/`. Päivä 2026-11-26.

Batch C-8, step 1: inventory of global and static variables (part A), lifecycle and stage
change (part B), integrity of derived structures (part C), shared state between threads not
covered by C-6 (part D) and independent review (part E). Revision `be52dbf` (production
sources unchanged since `f7fe553`; only test files added). Paths relative to `TPsource/V52/`.

Merkinnät / tags: [todennettu] = koodi luettu / code read; [päätelty] / [inferred];
[tuntematon] / [unknown]. Kaikki tulokset ovat **staattisia**: mitään ohjelmaa ei ajettu eikä
käännetty. / All results are **static**; nothing was run or built.

Raakatulokset / raw results: `todisteet/c8/` (README siellä / README there).

## 1. Yhteenveto / Summary

- **Inventaario / inventory:** skripti `tyokalut/globaalit.py` löysi ytimestä (`Hk/`, `Juk/`,
  `Tp/`, `com/`, `cbTp/`, `tputilv2/`) 1 200 eri globaalia tai tiedostotason staattista nimeä ja
  370 funktion sisäistä staattista muuttujaa sekä noin 45 500 viittausta niihin (myös GUI- ja
  konsolihakemistoista). Agentti luokitteli 983 tiedostotason nimeä ja 197 funktion sisäistä
  staattista seitsemään luokkaan; 217 ohitettiin syyn kanssa (luku 3). / The script found 1,200
  distinct global or file-static names and 370 function-local statics in the core, with about
  45,500 references; 983 + 197 were classified into seven categories, 217 skipped with a reason.
- **Uudet havainnot / new findings:** H-128–H-144 ja T-038–T-039 (luku 7). Tärkeimmät:
  - **H-129 (korkea / high):** vaiheen vaihto (`teejarj(true)`) nollaa numerohakemiston
    `idindex` puskurin mutta ei sen rivimäärää `nrec`; uudet rivit lisätään vanhojen perään
    ilman kapasiteettitarkistusta → kekoylivuoto ensimmäisessä vaiheen vaihdossa noin 1 500
    kilpailijasta alkaen (kolmivaiheisessa toisessa vaihdossa noin 1 000:sta). / the stage
    change zeroes the bib index buffer but not its record count → heap overflow from about 1,500
    competitors.
  - **H-128 (keskitaso–korkea / medium-high):** `setpos` ei koskaan poista numerohakemiston
    riviä (poistohaara on saavuttamaton), joten jokainen poisto ja numeronvaihto jättää
    `numero → 0` -rivin; numerojärjestyksessä etenevät silmukat (vienti, arvonta, sakkolista,
    radan tunnistus, meri3-pisteet) pysähtyvät siihen. Lisäksi numeron lasku vapaaseen numeroon
    voi jättää kilpailijan kokonaan hakemiston ulkopuolelle. / `setpos` never deletes; walks in
    bib order stop at stale entries; lowering a bib can drop the competitor from the index.
  - **H-130, H-131 (keskitaso / medium):** viestin sarjanvaihto ei päivitä sarjojen
    joukkuemääriä (`nilm`; päivityshaara on kuollutta koodia) ja HK:n vaiheen vaihto jättää
    aikajonotiedoston nimipohjan edellisen vaiheen numeroon, joten myöhemmin luotu jono
    kirjoittaa edellisen vaiheen tiedostoon. / relay series change leaves team counts wrong;
    stage change leaves the time-queue file-name template at the previous stage.
- **Elinkaari / lifecycle:** vaiheen vaihdon nollaustaulukko (luku 4.2; täysi taulukko osan B
  raportissa, noin 50 kohdetta / full table about 50 items);
  vanhaksi jäävät `aikaflname`, `kilpparam.maxvaluku`/`lsakmax`, sarjojen `mno`/`lsak`-rajaukset,
  `maaliajat`, `eralahto`/`era_akt`, korostusrajat ja `on_emit`-taulun laskuri. Kilpailua ei voi
  avata uudelleen käynnissä olevassa HK-prosessissa; `suljekilpailu` on kuollutta koodia.
- **Säikeet / threads:** kolme uutta vahvistettua tai osittain vahvistettua (H-136 `bdg_t`,
  H-137 `emithead2`, H-138 `openlukijaTCP`:n yhteinen laskuri); `volatile`-puutteista ei
  havaittavaa seurausta (kaikissa odotussilmukoissa on ulkoinen kutsu).
- **Itsestään korjautuvuus / self-healing:** johdetut rakenteet ovat vain muistissa ja
  rakennetaan uudelleen käynnistyksessä, joten useimmat eheysviat korjautuvat uudelleen-
  käynnistyksellä. Poikkeus: H-129:n muistivaurio. / derived structures live in memory only and
  are rebuilt at start; most integrity defects heal on restart, except H-129's memory damage.
- **Riippumaton tarkastus / review (Opus):** 16 väitettä: 8 vahvistettu, 3 vahvistettu
  muutetulla vakavuudella tai rajauksella, 5 osittain, 0 kumottu; lisäksi uusi vihje (`AJAT=`),
  jonka pääistunto todensi koodista (H-132). / 16 claims: 8 confirmed, 3 confirmed with changes,
  5 partly, 0 refuted; one new lead verified from code by the main session.

## 2. Rajaus ja menetelmä / Scope and method

Rajaus: `suunnitelma.md` "C-8 vaihe 1". Lähtötietoina, ei analysoitu uudelleen: C-1
(H-007–H-021), C-2 (H-022–H-039), C-3 (H-040–H-054), C-5, C-6 (H-087–H-108, Q30, Q67), C-7
(H-109–H-127). Rajattu pois: lomakkeiden sisäinen tila, lukitusjärjestys ja säikeiden
elinkaari, tiedostomuodot, kääntäjäerot, korjaukset ja ajonaikaiset kokeet.

Scope: `suunnitelma.md` "C-8 vaihe 1". Earlier batches are inputs and were not re-analysed.
Out of scope: form-internal state, lock order and thread lifecycle, file formats, compiler
differences, fixes, run-time probes.

| Osa / Part | Tekijä, malli / By, model | Tuotos / Output |
|---|---|---|
| A, skripti / script | pääistunto / main session | `tyokalut/globaalit.py`; `todisteet/c8/osaA/*.csv` |
| A, luokittelu / classification | `architecture-analyst`, Sonnet | `osaA/luokittelu.csv`, `osaA-luokittelu.md` |
| B, elinkaari / lifecycle | `architecture-analyst`, Sonnet | `osaB-elinkaari.md` (N-B1–N-B14) |
| C, eheys / integrity | `defect-concurrency-auditor`, Sonnet | `osaC-eheys.md` (N-C1–N-C15, L-1–L-7) |
| D, säikeet / threads | `defect-concurrency-auditor`, Sonnet | `osaD-saikeet.md` (N-D1–N-D6) |
| E, tarkastus / review | `independent-verifier`, Opus | `osaE-tarkastus.md` |

Skripti on leksikaalinen: se poistaa kommentit ja merkkijonot, pitää jokaisesta `#if`-ketjusta
vain ensimmäisen haaran (jotta aaltosulkeet pysyvät tasapainossa), seuraa aaltosulkeiden
syvyyttä ja kohdistaa viittaukset funktioihin. Kun syvyys menee sekaisin (kuolleessa
`#if`-koodissa epätasapainoiset sulkeet, esim. `Hk/HkTV.cpp`, `Tp/TpComY.cpp`), viittaus
kohdistetaan lähimpään edeltävään sarakkeesta 0 alkavaan funktion otsikkoon ja merkitään `~`:llä.
Kirjoitukset tunnistetaan heuristisesti (sijoitus, `++`/`--`, osoitteen otto tai kirjoittavan
kirjastofunktion ensimmäinen argumentti). Rajoitukset: luku 8.

The script is lexical: it strips comments and literals, keeps only the first branch of each
`#if` chain, tracks brace depth and attributes references to functions; when brace tracking
loses sync it falls back to the nearest column-0 function head (marked `~`). Writes are
detected heuristically. Limitations: section 8.

## 3. Inventaario / Inventory

### 3.1 Määrät / Counts

Tiedostotason määrittelyt ytimessä (`maarittelyt.csv`; ilman otsikoiden ja jäsennysvirheiden
rivejä) ja funktion sisäiset staattiset (`staattiset_funktiossa.csv`) / file-level definitions
and function-local statics in the core:

| Hakemisto / Dir | Globaali / global | Tiedoston `static` / file static | Funktion `static` / function static |
|---|--:|--:|--:|
| `Hk/` | 790 | 94 | 126 |
| `Juk/` | 549 | 62 | 91 |
| `Tp/` | 65 | 17 | 103 |
| `com/` | 1 | 14 | 4 |
| `cbTp/` | 35 | 52 | 13 |
| `tputilv2/` | 39 | 22 | 33 |

Rivit eivät ole eri nimiä: HK:n ja viestin yhteiset nimet (`HkGlobals.cpp` ja `VGlobals.cpp`)
lasketaan kahdesti, ja yhdessä lauseessa esitellyt nimet omina riveinään. Eri nimiä on 1 200.
/ Rows are not distinct names; distinct names: 1,200.

### 3.2 Luokat / Categories

Luokittelu `osaA/luokittelu.csv` (sarakkeet: nimi, määrittelytiedosto, luokka, alue, kirjoittajat,
säiehuomio, vaiheriippuvuus, huomio). "Molemmat" = määritelty sekä `HkGlobals.cpp`:ssä että
`VGlobals.cpp`:ssä; "yhteinen" = vain `Tp/`, `com/`, `cbTp/` tai `tputilv2/`. / Classification;
"both" = defined in both globals files; "shared" = only in the shared directories.

| Luokka / Category | HK | Viesti / relay | Molemmat / both | Yhteinen / shared | Yht. / total |
|---|--:|--:|--:|--:|--:|
| K1 asetus / configuration | 135 | 62 | 147 | 54 | 398 |
| K2 kilpailudata / competition data | 17 | 22 | 36 | 5 | 80 |
| K3 johdettu tila / derived state | 31 | 15 | 48 | 0 | 94 |
| K4 ajonaikaiset liput ja tilat / run-time flags and states | 8 | 5 | 39 | 16 | 68 |
| K5 yhteyspuskurit / communication buffers | 13 | 7 | 69 | 16 | 105 |
| K6 tulostus ja muotoilu / output and formatting | 49 | 29 | 109 | 25 | 212 |
| K7 muu / other | 5 | 0 | 1 | 20 | 26 |

Funktion sisäiset staattiset (`Hk/`, `Juk/`; 197): K3-välimuistit 48, K4 ensikutsu- ja
tilaliput 32, K5 19, K6 tulostuksen siirtotila 78, K7 paluupuskurit 19, K1 1. Ohitetut 217
tiedostotason nimeä: 139 `tag_*`-merkkijonotaulukkoa, 35 lomakeosoitinta, 17 kriittistä osiota,
16 säiekahvaa, 9 skriptin virheosumaa, 1 käyttämätön. / Function-local statics: 197 classified;
217 file-level names skipped with reasons.

### 3.3 Lukumäärien täsmäytys / Reconciliation of the counts

Tarkistettu sulkemisessa CSV-tiedostoista (`todisteet/c8/osaA/`; skriptiä ei tarvinnut ajaa
uudelleen). / Checked at closing from the CSVs; no re-run needed.

| Joukko / Set | Määrä / Count | Selitys / Explanation |
|---|--:|---|
| Määrittelyrivit / definition rows (`maarittelyt.csv`) | 1 751 | 1 479 globaalia, 261 tiedoston `static`, 6 jäsennysvirhettä, 5 otsikossa / 1,479 global, 261 file static, 6 parse errors, 5 in headers |
| **Eri tiedostotason nimet / distinct file-level names** | **1 200** | Rivit nimittäin yhdistettyinä: 326 nimeä on sekä `HkGlobals.cpp`:ssä että `VGlobals.cpp`:ssä (sama muuttuja kahdessa ohjelmassa) ja yhteensä 514 nimeä useammassa kuin yhdessä tiedostossa; 9 nimeä on toisaalla globaali ja toisaalla `static` / rows merged by name; 326 names are in both globals files (one variable in two programs), 514 in more than one file |
| – luokiteltu / classified | 983 | luokat K1–K7 / categories |
| – ohitettu / skipped | 217 | 139 `tag_*`-vakiotaulukkoa, 35 lomakeosoitinta, 17 kriittistä osiota, 16 säiekahvaa, 9 skriptin virheosumaa, 1 käyttämätön; syy rivikohtaisesti `luokittelu.csv`:ssä / reasons per row |
| **Funktion sisäiset staattiset / function-local statics** (`staattiset_funktiossa.csv`) | **370** | rivejä (muuttuja funktiossa), eri nimiä 191; erillinen joukko tiedostotason nimistä (8 nimeä esiintyy myös tiedostotason nimenä, ne ovat eri muuttujia) / rows, 191 distinct names; separate from the file-level set |
| – rajauksessa (`Hk/`, `Juk/`) / in scope | 217 | 126 + 91 |
| – – luokiteltu / classified | 197 | |
| – – ohitettu / skipped | 20 | vakiotaulukot, merkkijonot, säiekahvat / const tables, literals, thread handles |
| – rajauksen ulkopuolella / out of scope | 153 | `Tp/` 103, `tputilv2/` 33, `cbTp/` 13, `com/` 4 (poikkeama, luku 12 / deviation, section 12) |
| **Luokiteltu yhteensä / classified total** | **1 180** | 983 tiedostotason nimeä + 197 funktion staattista; eri yksiköt (nimi vs. muuttuja funktiossa), joten summa on vain laskennallinen / different units, the sum is arithmetic only |

Joukot eivät ole päällekkäisiä luokittelussa: jokainen 1 200 nimestä on `luokittelu.csv`:ssä
täsmälleen kerran (983 + 217), ja funktion staattiset ovat omina `funktio::nimi`-riveinään (197).
Päällekkäisyys on vain lähtöriveissä (sama nimi useassa tiedostossa) ja viittauksissa
(samannimiset muuttujat yhdistyvät, luku 8). / The classification sets do not overlap; overlap
exists only in definition rows and in references.

Luokittelu on nimen ja määrittelyn perusteella tehty [päätelty], ellei osan A raportissa ole
riviviitettä. K3-rakenteiden ylläpitäjät ja K4-lippujen kirjoittajat ja lukevat säikeet:
`todisteet/c8/osaA-luokittelu.md` havainnot 1–2. / Classification by name [inferred] unless a
line is cited; maintainers and flag/thread tables in the part A report.

## 4. Elinkaari / Lifecycle

### 4.1 Alustusjärjestys / Initialisation order

HK ja viesti käynnistyvät samassa järjestyksessä (`Hk/HkInit.cpp:3079-3716`,
`Juk/VInit.cpp:2538-2966`): oletusarvot → `lue_parametrit` → `luesarjat` → `KILP.DAT`:n avaus →
`teejarj(false)` → rata- ja emit-tietojen luku → `inittimer` → `initcom` → `taustaon = 1`.
K2/K3-globaalia ei lueta ennen alustusta [todennettu, osa B]. Viesti nollaa `Sarjat`in
(`asetaOletusarvot`, `Juk/VInit.cpp:2535`), HK ei. `maaliajat`ille nolla ei ole kelvollinen
alkuarvo (osa B N-B5, ei tarkastettu).

HK and relay start in the same order: defaults → parameters → classes → open `KILP.DAT` →
`teejarj(false)` → courses and emit → timer → comms → background flag. No K2/K3 global is read
before initialisation [verified, part B]. Relay resets `Sarjat`, HK does not.

### 4.2 Vaiheen vaihto / Stage change (`vaihdavaihe`, `Hk/HkIx.cpp:1855-2005`)

Kutsujat: konsolin valikko (`HkConsole/HkCons0.cpp:933`), verkkopyyntö (`Hk/HkCom32.cpp:844-847`
→ `pyyda_vaihdavaihe`), GUI:n `Vaiheenvaihto1Click` ja `VaihdaVaiheHandler`
(`cbHk/WinHk.cpp:2049-2229`). Säie- ja lukkonäkökulma: C-6 (H-094, H-097, H-102, H-107, T-033).
Viestissä ei ole vaiheen vaihtoa. / Callers: console, network request, two GUI handlers; thread
and lock aspects are C-6; relay has no stage change.

Nollaustaulukko (tiivistetty; täysi taulukko `todisteet/c8/osaB-elinkaari.md` luku B2) / reset
table (condensed; full table in the part B report):

| Kohde / Item | Käsittely / Disposition | Näyttö / Evidence | Seuraus / Effect |
|---|---|---|---|
| `k_pv`, `arv_pv`, `obfilename[7]`, `aikajono[]->aikafname` | asetetaan / reset | `:1888-1894`, `:1931-1932` | – |
| `aikaflname[]` (jonotiedoston nimipohja / queue file-name template) | **vanha / stale** | asetus vain / set only `Hk/HkInit.cpp:3364-3367`; käyttö / used `Hk/HkIV.cpp:4227` | H-131 |
| laskurit / counters `nilm*`, `ntulos*`, `nhyl*`, `nkesk*`, `neil*`, `nvak*`, `nesit*`, `nva` | nollataan, lasketaan uudelleen / reset, recomputed | `:1913-1930`; `teejarj(true)` `:1759-1761` | ei `nyilmt`, `nytulost` (H-139) |
| `jalku`, `jarjtulos`, `jrjseur`, `yhdsija`, `bdg_kno`, `nbadge`, `emitjarr` | nollataan tai varataan uudelleen / reset or reallocated | `:1604-1661` | `emitjarr[1]` vain kun / only when `kaksibadge==2` |
| `idindex` | puskuri nollataan, `nrec` ei / buffer zeroed, `nrec` not | `:1610`, `:1757-1758` | **H-129** |
| `nameindex` | ennallaan, kelvollinen / kept, valid | `:1741-1745` | – |
| `ixarr`-hakemistot (`sarjaindex`, `ljarjindex` ...) | ennallaan; rakennetaan käytettäessä / kept; rebuilt on demand | `teeindeksi(..., true)` aina uudelleen / always rebuilds `:241` | korjaa osan A oletuksen / corrects part A |
| `Sarjat`, `pvparam`, `kilpparam` (XML) | luetaan uudelleen / reloaded | `:1934` → `Hk/HkSrj.cpp:72-75` → `Hk/HkXml.cpp:1663-1852` | lisäävä: `valuku` ei pienene / additive |
| `kilpparam.maxvaluku`, `lsakmax` | **vanha / stale** | `Hk/HkInit.cpp:3324-3347` | H-143 (N-B6) |
| `Sarjat[].mno`, `lsak` -rajaukset / clamps | **menetetään / lost** | `Hk/HkInit.cpp:3383-3392`; `Hk/HkXml.cpp:676`, `:695` | H-143 (N-B7) |
| `maaliajat[]` | **vanha / stale** | `Hk/HkDat.cpp:1320` | H-143 (N-B5) |
| `eralahto[]`, `era_akt` | **vanha / stale** | ei viittausta / no reference | H-141 |
| rata- ja emit-muisti / course and emit memory (`rata`, `kuvio`, `rvalit`, `rastiva`, `lainaemit`) | vapautetaan, luetaan uudelleen / freed, reloaded | `:1908`, `:1937` | puuttuva `RADAT<n>.XML` → `emitfl = 0` (H-134) |
| `emithead`, `emithead2`, `emitptr` | luetaan uudelleen / reloaded | `Hk/HkEmit.cpp:3059-3078` | ohitus esitarkastuksessa = H-105 |
| `on_emit::emitvaraus` / `nemitvar` | taulu vapautetaan, laskuri ei / table freed, count not | `Hk/HkEmit.cpp:1303-1327`, `:6367` | H-141 |
| aikajonojen sisältö / queue contents | luetaan uudelleen / reloaded | `:1904`, `:1970` | ei, jos jono on olemassa mutta ajanotto pois / not if queue exists but timing off [päätelty] |
| korostusrajat / highlight limits `korRaja`, `vipLaji` (GUI) | **vanha / stale** | `Hk/HkInit.cpp:217-266`, `:3624` | H-143 (N-B10) |
| `luelahdepisteet::luettu` | lukko: tiedostoa ei lueta uudelleen / latch | `Hk/HkIV.cpp:204-210` | H-143 (N-B9) |
| muut funktion staattiset / other function statics (`tall_emit::ed_em`, `tall_bdg_t::edbadge`, `pros_emit::edkno`, `tark_alku::var`) | vanha, vaaraton / stale, harmless [päätelty] | osa B / part B | – |

Pysyvä tila: vaiheen vaihdosta tallentuu vain `vaihevalinta.cfg` (`Hk/HkIx.cpp:1997-1998`).
/ The only persisted stage state is `vaihevalinta.cfg`.

### 4.3 Uudelleenavaus, kaksoiskäynnistys ja lopetus / Reopen, double start and shutdown

- **Uudelleenavaus / reopen:** käynnissä olevassa HK-prosessissa ei ole uudelleenavauspolkua.
  Tuonti sulkee ja avaa saman `datf2`:n ja lisää rivejä `addtall`illa
  (`cbHk/UnitImport.cpp:1405-1437`; H-047 kattaa tiedostopuolen). `TFormValitseKilp` kirjoittaa
  globaalit `kilpailu` ja `pvparam[0].Date` jo valinnassa, ennen `aloitus`ta
  (`cbHk/UnitValitseKilp.cpp:59-62`). `suljekilpailu` (`Hk/HkIx.cpp:1481-1512`) on kuollutta
  koodia (ei kutsujia) ja purkuna puutteellinen. / No reopen path in a running HK process;
  import reopens the same file; `suljekilpailu` is dead and incomplete.
- **Kaksoiskäynnistys (Q67) / `aloitus` twice:** C-6 kattoi lukkopuolen. Tilapuoli: tiedostonimien
  paikallaan tehtävä muokkaus ei ole toistettava, laskurit kaksinkertaistuvat ja varaukset
  vuotavat (H-133). Tarkastaja kavensi: saman kilpailun uusintayritys kaatuu jo `KILP.DAT`:n
  avaamiseen, koska ensimmäinen kahva jää auki ilman jakoa kirjoittajille (`dbboxm/dbbox1.cpp:176-178`;
  Win32-semantiikka [päätelty], Q80). / State side of Q67: H-133, narrowed by the reviewer.
- **Lopetus / shutdown:** `lopetus1` ei vapauta muistia eikä kirjoita lopputilaa; pysyvyys
  perustuu `PUTREC(d,1)`:n läpikirjoitukseen. Ei uusia vikoja H-038:n, H-095:n ja H-104:n
  lisäksi. / No new defects beyond H-038, H-095, H-104.

## 5. Johdettujen rakenteiden eheys / Integrity of derived structures

Invariantit ja ylläpitäjät (tiivistetty; täysi taulukko `osaC-eheys.md` luku 2) / invariants and
maintainers (condensed):

| Rakenne / Structure | Invariantti / Invariant | Ylläpitäjät / Maintainers | Tila / Status |
|---|---|---|---|
| `nilm`, `nilmt` (HK) | mukana olevat ei-poistetut kilpailijat sarjassa / present, non-deleted per class | `addjarjarr`/`remjarjarr` (`Hk/HkIx.cpp:800-1462`), `uusijarj`, `teejarj`, `vaihdavaihe` | symmetrinen / symmetric |
| `nvak` | vakantit (V) vaiheessa / vacant per stage | sama / same | **epäsymmetrinen / asymmetric (H-139)** |
| `nyilmt`, `nytulost` | kaikki ilmoittautuneet, yhteistulokset / totals | `++`/`--` vain / only | **ei nollausta / never reset (H-139)** |
| `jalku`, `jrjseur`, `jarjtulos`, `pkarki` | sijoitusketju sisältää täsmälleen laskurin kilpailijat / rank chain matches its counter | `addjarjarr`/`remjarjarr` | poisto tietuetunnisteen mukaan / removal by record identity |
| `idindex` (`getpos`/`setpos`) | numero → tietue, yksi elävä rivi / bib → record, one live entry | `setpos` (`Hk/HkIx.cpp:205-226`) | **rikki / broken (H-128, H-129)** |
| `nameindex` | nimi + sarja → tietue / name + class → record | `addtall`, `deltall`, `tallenna` | **päivitys vain nimen muuttuessa / name-only trigger (H-142)** |
| `bdg_kno`, `nbadge` | emit-koodi → numero / badge → bib | `addbadge`, `rembadge`, `teejarj` | **epäsymmetrinen / asymmetric (H-135)** |
| `emitjarr` | tietuepaikka → emit-tietue / slot → emit record | `setEmitJarr`, `addem`, `lueemitfile` | **ei tyhjennetä poistossa / not cleared on delete (H-140)** |
| viestin `nilm`, `nilmt`, `nilmtt` / relay team counts | joukkueet sarjassa / teams per class | `paivJoukkLkm` (`Juk/VIx.cpp:654-659`) | **sarjanvaihto ei päivitä / series change not applied (H-130)** |
| viestin `kilpnoix` | numero → tietue (suora taulukko) / direct array | `Juk/vdat.cpp:64-98` | kunnossa / OK |

Luottavat kuluttajat / trusting consumers: `sijat2` (`Hk/HkTulostus.cpp:1172-1173`) ja `ossijat`
(`Juk/VTulostus.cpp:960-965`) varaavat puskurin laskurista ja täyttävät sen ketjusta ilman rajaa:
liian pieni laskuri (H-109, H-130) johtaa ylivuotoon. Numerojärjestyksen läpikäynnit
(`getpos(x, true)` + `nextpos()`) lopettavat ensimmäiseen arvoon ≤ 0 (H-128). / Consumers size
buffers from counters and fill from chains without bounds; bib-order walks stop at the first
value ≤ 0.

Kaikki johdetut rakenteet ovat vain muistissa; `teejarj(false)` käynnistyksessä rakentaa ne
tietueista. Ainoa kesken ajon tapahtuva uudelleenrakennus on HK:n vaiheen vaihto (H-129). Väärää
johdettua tietoa ei kirjoiteta levylle. H-109:n mekanismi on `deltall`issa itsessään (ei
`kilpstatus`-tarkistusta), ja siihen pääsee myös luettelopohjaisesta poistosta
(`cbHk/UnitOsanottajat.cpp:1386`, `ViestiWin/UnitJoukkueet.cpp:1144`). / All derived structures
are memory-only and rebuilt at start; H-109's mechanism is in `deltall` and also reachable from
list-based deletes.

## 6. Säikeiden yhteinen tila / Shared state between threads

Osa D kävi läpi C-6:n säieluettelon (16 säiettä, kutsusyvyys 1–5) ja osan A luokat K2–K7.
Uudet (C-6:n ulkopuoliset) / new beyond C-6:

| ID | Tila / State | Kirjoittaja / Writer | Lukija / Reader | Lukko nyt / Lock today |
|---|---|---|---|---|
| H-136 | `bdg_t[8]` (`Hk/HkIV.cpp:136-142`) | `pros_bdg_t` → `tall_bdg_t` `:775-779` | lukijasäikeet → `hae_bdg_t` `:862-872` (HK myös kirjoittaa / HK also writes) | ei / none |
| H-137 | `emithead2` | `tarkcom` → `addem` `Hk/HkEmit.cpp:1679-1680` | pääsäie `pros_emit` `Hk/HkIV.cpp:1184-1189` | `emit` vain `addem`/`lueem` sisällä / inside only |
| H-138 | `openlukijaTCP::count` (`Tp/TpComY.cpp:2123`) | jokainen lukijasäie / each reader thread | muut lukijasäikeet / other readers | ei / none |
| H-144 | staattiset paluupuskurit (`asakkoStr` ...), `gazline`, `perr_ptrs2` | `kirjSQLTh`, `gazTh`, työsäikeet / workers | pääsäie / main thread | ei yhteistä / no common lock |

`volatile`-puutteista ei löytynyt havaittavaa seurausta: jokaisessa toisen säikeen lippua
odottavassa silmukassa on ulkoinen kutsu (`Sleep`, `SendMessage`, `recv` ...), joka pakottaa
lukemaan arvon uudelleen. Hylätyt ehdokkaat syineen: `osaD-saikeet.md` luku 6 (esim. lukija-
kohtaisesti indeksoidut staattiset, `tall`-lukon alla toimivat tulostuksen staattiset). /
No observable consequence from missing `volatile`; rejected candidates in the part D report.

## 7. Uudet havainnot / New findings

Rekisteri: `rekisteri/havainnot.md`. N-tunnukset viittaavat raakaraportteihin. Vakavuus on
tarkastajan korjaama, kun se poikkeaa analyytikon arviosta. / Register IDs; N IDs refer to the
raw reports; severity as corrected by the reviewer.

| ID | N | Lyhyesti / In short | Vakavuus / Severity | Tarkastus / Review |
|---|---|---|---|---|
| H-128 | N-C1, N-C2 | `setpos` ei poista; numerojärjestyksen läpikäynnit katkeavat; numeron lasku voi pudottaa kilpailijan hakemistosta / never deletes, walks stop, lowered bib can vanish | keskitaso–korkea / medium-high | vahvistettu; **toistettu eristetysti** (luku 11) / confirmed; reproduced in isolation |
| H-129 | N-C3 | `teejarj(true)` ei nollaa `idindex->nrec` → kekoylivuoto / heap overflow at stage change | korkea / high | vahvistettu; **mekanismi toistettu eristetysti** (luku 11) / confirmed; mechanism reproduced |
| H-130 | N-C10 | viestin sarjanvaihto ei päivitä `nilm`iä (kuollut haara); `ossijat`-ylivuoto kapeammalla ehdolla / relay team counts | keskitaso / medium | osittain / partly |
| H-131 | N-B3 | vaiheen vaihto ei päivitä `aikaflname`-pohjaa / time-queue template stale | keskitaso / medium | vahvistettu / confirmed |
| H-132 | tarkastajan vihje / reviewer lead | yksivaiheisessa kilpailussa `AJAT=`-nimen 5. merkki poistetaan / one-stage `AJAT=` name loses its 5th character | matala–keskitaso / low-medium | pääistunto todensi koodista / verified from code by main session |
| H-133 | N-B1, N-B2 | `aloitus` ei ole turvallinen uudelleen ajettava (Q67): nimet, laskurit, vuodot / not safe to re-run | matala–keskitaso / low-medium | osittain / partly |
| H-134 | N-B12 | puuttuva `RADAT<n>.XML` vaiheen vaihdossa → emit-tarkastus pois ilman tekstivaraa / emit off without fallback | matala–keskitaso / low-medium | vahvistettu / confirmed |
| H-135 | N-C6 (+ N-C7, N-C15) | `bdg_kno`:n ylläpito `tallenna`ssa: kaksi koodia + numeronvaihto, `kaksibadge==1`, tila P / badge map maintenance | keskitaso–matala / medium-low | N-C6 vahvistettu, muut ei / others not reviewed |
| H-136 | N-D1 | `bdg_t[]` ilman lukkoa → väärä `em.time` / unlocked badge-time table | keskitaso–matala / medium-low | osittain / partly |
| H-137 | N-D2 | `emithead2`:n tarkista-ja-toimi → kortti ohitetaan automaattikäsittelystä / card skipped | keskitaso–matala / medium-low | vahvistettu / confirmed (mekanismi) |
| H-138 | N-D3 | `openlukijaTCP`:n yhteinen `count` → toisen lukijan ~4 min viive / shared back-off | matala–keskitaso / low-medium | osittain / partly |
| H-139 | N-C4 (+ N-C5) | HK-laskureiden epäsymmetria ja nollaamattomat summat / counter asymmetry | matala–keskitaso / low-medium | N-C4 vahvistettu / confirmed |
| H-140 | N-C8 | `emitjarr` jää poistossa → uusi kilpailija perii emit-linkin / slot reuse inherits emit link | matala–keskitaso / low-medium | vahvistettu / confirmed |
| H-141 | N-B4, N-B11 | vaiheen vaihdossa nollaamatta: `eralahto`/`era_akt`, `nemitvar` / not reset at stage change | matala / low | osittain, vahvistettu / partly, confirmed |
| H-142 | N-C7, N-C9, N-C11–N-C14 (+ L-huomiot) | eheyden pienet viat (kokoomarivi) / minor integrity defects | matala / low | ei tarkastettu / not reviewed |
| H-143 | N-B5–N-B10, N-B13, N-B14 | elinkaaren pienet viat (kokoomarivi) / minor lifecycle defects | matala / low | ei tarkastettu / not reviewed |
| H-144 | N-D4–N-D6 | säikeiden pienet viat (kokoomarivi) / minor thread defects | matala / low | ei tarkastettu / not reviewed |

Tekninen velka / technical debt:

- **T-038** Vaiheriippuvaa tilaa ei nollata keskitetysti: `vaihdavaihe` nollaa kohteita
  tapauskohtaisesti, ja `aloitus` muokkaa tiedostonimiä paikallaan (`wmemmove`, numeron vaihto),
  joten jokainen uusi vaiheriippuva globaali on unohtumisvaarassa (H-131, H-132, H-133, H-141,
  H-143). / No central reset for stage-dependent state; file names patched in place.
- **T-039** Johdetut rakenteet ylläpidetään käsin pareittain (`addjarjarr`/`remjarjarr`,
  `addbadge`/`rembadge`, `setpos`) eri kutsupaikoissa ilman invarianttitarkistusta tai
  kesken ajon käytettävää uudelleenrakennusta; ainoa korjaus on uudelleenkäynnistys (H-128,
  H-130, H-135, H-139, H-140). / Derived structures maintained by hand-paired calls, no
  invariant check or mid-run rebuild.

Vaikutusarvio kolmelle tärkeimmälle / impact of the three most important [päätelty / inferred]:

- **H-129:** kekoylivuoto kesken kilpailun vaiheen vaihdossa; seuraus voi olla kaatuminen tai
  muun muistin hiljainen korruptio, joka ei korjaudu ennen uudelleenkäynnistystä. / heap overflow
  at the stage change: crash or silent memory corruption.
- **H-128:** **hiljainen tietojen oikeellisuusongelma.** Ohjelma ei anna virheilmoitusta, vaan
  numerojärjestyksessä etenevät toiminnot jättävät kilpailijoita pois: vienti numerojärjestyksessä,
  arvonnan läpikäynti (`SeurPos`), sakkokierroslista, meri3-pisteet ja radan tunnistus
  (`haeratakilp`). Numeron lasku voi lisäksi kadottaa kilpailijan numerohaulta, jolloin emit- ja
  koodilinkit jäävät hiljaa tekemättä. Korjautuu uudelleenkäynnistyksellä tai arvonnan lopun
  `luo_posarr`illa, mutta jo tuotettu vienti tai lista on puutteellinen. / silent data-correctness
  problem: no error message, competitors silently missing from bib-order exports, draw walk,
  penalty list, points and course identification; output already produced stays incomplete.
- **H-131:** **eri vaiheiden tiedostojen sekoittuminen.** Vaiheen vaihdon jälkeen luotu aikajono
  käyttää edellisen vaiheen nimipohjaa, joten vaiheen n+1 ajat kirjoitetaan vaiheen n tiedostoon
  (esim. `AJAT1.LST`), jossa ovat jo vaiheen n ajat. Kun vaiheen n aikoja myöhemmin luetaan, mukana
  on toisen vaiheen aikoja, ja vaiheen n+1 omasta tiedostosta ne puuttuvat. / times of stage n+1
  are written into stage n's file and mix with stage n's times.

Korjausjärjestys [päätelty] / fix priority [inferred]: H-129 (yksirivinen: `idindex->nrec = 0`
`memset`in viereen) ja H-128 (`setpos`) ensin, koska ne koskevat jokaista monivaiheista HK-
kilpailua ja niille voi kirjoittaa doctest-testin (`Tests/`, `HkIx.cpp` + `dbindex.cpp`); sitten
H-130, H-131, H-132. / H-129 and H-128 first, both testable with doctest; then H-130–H-132.
H-129 on lisätty kiireelliseen korjausjonoon alustavalla prioriteetilla P2 (`seuranta.md`). /
H-129 added to the urgent fix queue, provisional P2.

T-038 ja T-039 ovat arkkitehtuurin kehityskohteita, eivät yksittäisiä korjauksia: ne kannattaa
ottaa huomioon, kun vaiheen vaihtoa tai johdettuja rakenteita korjataan (keskitetty nollaus,
invarianttitarkistus tai uudelleenrakennus). / T-038 and T-039 are architectural development
items, to be considered when the stage change or derived structures are fixed.

## 8. Osan A korjaukset ja skriptin rajoitukset / Part A corrections and script limitations

Korjaukset osan A luokitteluun (osat B ja E) / corrections:

- `teejarj` ei laske uudelleen `ixarr`-hakemistoja (`sarjaindex`, `ljarjindex` ...); käyttäjät
  rakentavat ne `teeindeksi(..., true)`:lla, joka rakentaa aina uudelleen (`teeindeksi::eday`
  on kuollut). / `teejarj` does not recompute the `ixarr` indexes; consumers rebuild them.
- `suljekilpailu` on kuollutta koodia; `sjfl`illä ei ole lukijaa; `emitpos`ia ei ole; `yhtjonot`
  on vakio; `tmaali`lle ei löytynyt kirjoittajaa (Q82); `tausta::prev_t` on alustuslukko. / dead
  or constant names.
- Funktion sisäisten staattisten luokittelu on nimen perusteella (esim. `edtls` on K6 eikä K3). /
  local-static classes are by name.

Skriptin rajoitukset / script limitations:

- Vain jokaisen `#if`-ketjun ensimmäinen haara luetaan; `#else`-haarojen määrittelyt puuttuvat. /
  Only the first `#if` branch is scanned.
- Kohdistamattomia viittauksia (`<file scope>`) on 3 318, lähes kaikki esittelyotsikoissa ja
  globaalitiedostoissa; 5 112 viittausta on kohdistettu varamenetelmällä (`~funktio`). Osan A
  luokittelu tehtiin ennen varamenetelmää, joten sen kirjoittajaluettelot ovat alarajoja. /
  3,318 unattributed references remain; 5,112 attributed by the fallback; part A's writer lists
  predate the fallback and are lower bounds.
- Kuudessa rivissä `parse-error` (kuollut koodi `Hk/HkTV.cpp`, samalle riville määritelty
  operaattori `Juk/VEmit.cpp:939`, `cbTp/UnitNaytaTulos.cpp:94`). / six parse-error rows.
- Yleisnimet (`n`, `p`, `rivi`, `ok`) yhdistävät eri muuttujia; HK:n ja viestin samannimiset
  globaalit yhdistyvät. Osoittimien, `memcpy`n ja jäsenfunktioiden kautta tehdyt kirjoitukset
  näkyvät vain "A"-merkintänä tai eivät lainkaan (esim. `kilparr`). / Generic names merge;
  writes through pointers or members are seen only as "A" or not at all.

## 9. Avoimet kysymykset / Open questions

| Q | Kysymys / Question | Liittyy / Relates to |
|---|---|---|
| Q78 | Mikä `n_lisaa` asennuksissa on käytössä, ja vaihdetaanko vaihetta käynnissä olevassa prosessissa vai käynnistetäänkö uudelleen? / Real `n_lisaa` and stage-change practice | H-129 |
| Q79 | `SeurPos` case 0 välittää tietuepaikan `getpos`ille, joka odottaa numeroa (`cbHk/UnitArvonta.cpp:221-223` vs `:421`) / record position passed as bib | H-128 |
| Q80 | Kaatuuko saman kilpailun uusintayritys `KILP.DAT`:n jakorikkomukseen (GUI-koe)? / Sharing violation on same-process retry | H-133, Q67 |
| Q81 | Ajetaanko tapahtumissa kahta tai useampaa lukijaa yhdellä HK-instanssilla, ja mikä Winsock-virhe sammutetusta TCP-lukijasta tulee? / Multiple readers in practice | H-136, H-138 |
| Q82 | Mihin `tmaali` kirjoitetaan? / Where is `tmaali` written? | H-143 |

## 10. Kattavuus ja resurssit / Coverage and resources

Valmistumisehdot (`suunnitelma.md`) / completion criteria:

- Osan A luettelo kattaa ytimen globaalit ja staattiset; puuttuvat on kirjattu syineen (217
  ohitettua, `#else`-haarat, lomakkeiden oma tila rajattu pois). **Täyttyy.** / met.
- Vaiheen vaihdon nollaustaulukko: luku 4.2 ja osan B taulukko. **Täyttyy.** / met.
- Johdettujen rakenteiden invariantit ja ylläpitäjät: luku 5 ja osan C luku 2. **Täyttyy.** / met.
- Uudet merkittävät havainnot tarkastettu riippumattomasti: kaikki keskitason ja sitä vakavammat
  väitteet (16). Matalat kokoomarivit H-142–H-144 ovat yhden agentin tuloksia. **Täyttyy.** / met.

Lukematta tai osittain / not read or partly: `lue_parametrit` (2 200 riviä) alkua pidemmälle,
viestin `luesarjat` ja GUI:n `Initialisoi`, viestin funktion sisäiset staattiset vastineet,
`addem`/`lueemitfile`-sisuskalut, HK:n yhteistulosketjujen avainrakennus, kuusi C-6:n säiettä
(`HaeAvoimetTh`, `haeSQLTh`, `ScrollThread`, `DisplTimerThread`, `NaytaTulosTimerThread`,
TCP/UDP-vastaanotto). Yksityiskohdat osaraporttien kattavuusluvuissa. / Details in the coverage
sections of the part reports.

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `architecture-analyst` (Sonnet; A) | 177 797 | 36 |
| `architecture-analyst` (Sonnet; B, sis. tallennus / incl. saving) | 328 521 | 113 |
| `defect-concurrency-auditor` (Sonnet; C) | 281 061 | 83 |
| `defect-concurrency-auditor` (Sonnet; D, sis. tallennus / incl. saving) | 357 910 | 107 |
| `independent-verifier` (Opus; E) | 183 350 | 81 |
| **Yhteensä / total** | **1 328 639** | **420** |

Ei tokenbudjettia (rajauksen mukaisesti). Skripti, pääistunnon todennus (H-132) ja dokumentointi
pääistunnossa. / No token budget per the scope; script, H-132 check and documentation in the main
session.

## 11. Eristetty todennus / Isolated verification (H-128, H-129; sulkeminen / closing 2026-11-26)

Toteutettu C-6:n vaiheen 2 eristetyn koeohjelman mallilla (`todisteet/c6/vaihe2/`): projekti
`C8ProbeHk` tuotetaan kopioimalla `VS/Tests/TpCoreHk.vcxproj` (samat tuotantolähteet,
määrittelyt ja asetukset), testilähteeksi vaihdetaan `todisteet/c8/koeohjelmat/c8_hk_probes.cpp`,
ja käännös tehdään repositorion ulkopuolelle (`C:\temp\c8build`, MSVC 19.52, Release, Win32).
`TPsource/`-puuhun, projektiasetuksiin tai CI:hin ei tehty muutoksia. Testit kutsuvat tuotannon
funktioita (`setpos`, `getpos`, `nextpos`, `luo_posarr`, `vaihdavaihe` → `teejarj(true)`)
sellaisinaan; vain globaali tila valmistellaan (kiinnike kuten H-031:n testissä: tyhjä
`KILP.DAT`, tuotannon `teejarj(false)`). Jokainen tapaus ajettiin omassa prosessissaan ja
hiekkalaatikossaan kahdesti; tulokset olivat samat.

Done with the C-6 step 2 isolated-probe pattern: a generated copy of the core test project with
this batch's probe file, built outside the repository. No change under `TPsource/`, to project
settings or CI. The probes call the production functions unchanged; only global state is
prepared. Each case ran in its own sandboxed process, twice, with identical results.

| Tapaus / Case | Tulos / Result |
|---|---|
| H-128 kontrolli / control | läpikäynti numerosta 104 käy 6, 7, 8 / walk visits 6, 7, 8 |
| H-128a (N-C1) | `setpos(105, 0)`:n jälkeen `idindex->nrec` 3 → 4 (rivi lisättiin, ei poistettu) ja läpikäynti pysähtyy: `6 (stop 0)`, numero 106 jää pois / entry added, walk stops, bib 106 missed — **TPRESULT observed=1** |
| H-128b (N-C2) | numeron 61 → 60 vaihdon jälkeen `getpos(60)` = −1 (oikein 7); `setpos(60, 7)` ei lisännyt riviä / bib lowered becomes unfindable — **observed=1** |
| H-129 kontrolli / control | tuotannon `luo_posarr` nollaa `idindex->nrec`in (5 → 0) / resets the count |
| H-129 (N-C3) | tuotannon `vaihdavaihe(1, 0)` (r = 0, `k_pv` = 1) jättää `idindex->nrec` = 5, vaikka `KILP.DAT`:ssa ei ole tietueita (oikein 0); seuraava `setpos` lisää perään (6) / count kept, next add appends — **observed=1** |

Näytön tasot / levels of evidence:

| Havainto / Finding | Lähdekoodista vahvistettu / confirmed from source | Eristetyllä testillä toistettu / reproduced in an isolated test | Toimitetussa ohjelmassa vahvistettu / confirmed in a shipped program |
|---|---|---|---|
| H-128 (N-C1, N-C2) | kyllä (osa C, Opus-tarkastus) / yes | **kyllä**: molemmat mekanismit / yes, both mechanisms | **ei** / no |
| H-129 (N-C3) | kyllä, rajat korjattu / yes, thresholds corrected | **mekanismi kyllä** (laskuri ei nollaudu, lisäys perään); itse kekoylivuotoa (yli `maxrec + 2` rivin) ei ajettu, se on laskennallinen seuraus / mechanism yes; the overflow itself was not run, it is an arithmetic consequence | **ei** / no |

Toimitetun ohjelman (`HkKisaWin.exe`, `HkMaali.exe`) käyttäytymistä ei ole ajonaikaisesti
vahvistettu. Ylivuodon todentaminen vaatisi noin 1 500 kilpailijan tietueet ja kekotarkistimen
(PageHeap tai ASan-käännös); se jätetään jatkotehtäväksi. / Shipped-program behaviour is not
verified at run time; reproducing the overflow itself needs about 1,500 records and a heap
checker, left as a follow-up.

Raakatulokset / raw results: `todisteet/c8/koeohjelmat/tulokset/` (kaksi ajoa, käännösloki /
two runs, build log).

## 12. Sulkeminen / Closure (2026-11-26)

Vertailu rajaukseen (`suunnitelma.md` "C-8 vaihe 1") / comparison with the scope:

| Rajauksen tavoite / Scope goal | Toteutus / Done | Tila / State |
|---|---|---|
| A: koneellinen luettelo ytimen globaaleista ja staattisista, kirjoittajat ja lukijat; luokittelu / scripted inventory, writers and readers, classification | `globaalit.py`, CSV:t, 1 200 nimeä luokiteltu tai ohitettu syineen; täsmäytys luku 3.3 / all 1,200 names classified or skipped with reasons | täyttyy / met |
| B: alustusjärjestys, vaiheen vaihdon nollaustaulukko, uudelleenavaus, `aloitus` kahdesti, lopetus / lifecycle | luku 4, osan B taulukko / section 4 | täyttyy / met |
| C: johdettujen rakenteiden invariantit, ylläpitäjät, rikkovat polut, vaikutus tuloksiin / derived structures | luku 5, osa C / section 5 | täyttyy / met |
| D: säikeiden yhteinen tila C-6:n ulkopuolelta / shared state beyond C-6 | luku 6, osa D / section 6 | täyttyy, poikkeama alla / met, deviation below |
| E: riippumaton tarkastus uusille korkeille ja keskitason väitteille / independent review | 16 väitettä, Opus / 16 claims | täyttyy / met |
| F: dokumentointi, rekisterit, todisteet, validaattori, paikallinen commit / documentation | `4884374` ja tämä sulkemiscommit / and this closing commit | täyttyy / met |

Poikkeamat (eivät estä sulkemista; kirjattu jatkotehtäviksi) / deviations (do not block
closing; recorded as follow-ups):

1. Funktion sisäiset staattiset luokiteltiin vain `Hk/`- ja `Juk/`-hakemistoista; `Tp/`-,
   `tputilv2/`-, `cbTp/`- ja `com/`-hakemistojen 153 funktion staattista ovat luettelossa
   (`staattiset_funktiossa.csv`) mutta luokittelematta. Osa D kävi ne läpi säikeiden kannalta
   (esim. H-138, hylätyt `tputilv2`-apufunktiot). / 153 function-local statics outside `Hk/`
   and `Juk/` are listed but not classified; part D covered them for thread use.
2. Osa D ei kattanut kuutta C-6:n säiettä (`HaeAvoimetTh`, `haeSQLTh`, `ScrollThread`,
   `DisplTimerThread`, `NaytaTulosTimerThread`, TCP/UDP-vastaanotto); ne ovat C-6:n, C-2:n tai
   lomakkeiden sisäisen tilan aluetta. / six C-6 threads not covered by part D.
3. Osan A luokittelu tehtiin ennen funktiokohdistuksen varamenetelmää, joten sen
   kirjoittajaluettelot ovat alarajoja (luku 8). / part A writer lists are lower bounds.
4. Matalat kokoomarivit H-142–H-144 ovat yhden agentin tuloksia ilman riippumatonta
   tarkastusta. / H-142–H-144 not independently reviewed.

Olennaista tavoitetta ei puutu: rajauksen valmistumisehdot (luku 10) täyttyvät. **C-8 on
suljettu käyttäjän hyväksynnällä 2026-11-26.** Avoimet asiat: `seuranta.md` "Avoimet
jatkotehtävät C-8:sta". / No essential goal is missing; completion criteria met. **C-8 closed
with the user's approval.** Open items: `seuranta.md`.
