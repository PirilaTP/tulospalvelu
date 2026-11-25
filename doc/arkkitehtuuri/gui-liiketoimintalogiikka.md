# GUI:n liiketoimintalogiikka / GUI business logic (C-7 vaihe 1 / step 1)

Erä C-7, vaihe 1: moduulikuvaukset (osa A), liiketoimintalogiikan virheet (osa B), C-5:n
yhdeksän jäsennysehdokasta (osa C) ja riippumaton tarkastus (osa D). Revisio `7be7480`
(`TPsource/` ennallaan revisiosta `f7fe553`). Polut suhteessa `TPsource/V52/`. Päivä
2026-11-25.

Batch C-7, step 1: module descriptions (part A), business-logic defects (part B), the nine
C-5 parse candidates (part C) and independent review (part D). Revision `7be7480`
(`TPsource/` unchanged since `f7fe553`). Paths relative to `TPsource/V52/`.

Merkinnät / tags: [todennettu] = koodi luettu / code read; [päätelty] / [inferred];
[tuntematon] / [unknown]. Kaikki tulokset ovat **staattisia**: mitään ohjelmaa ei ajettu eikä
käännetty. / All results are **static**; nothing was run or built.

Raakatulokset / raw results: `todisteet/c7/` (README siellä / README there).

## 1. Yhteenveto / Summary

- **Kohde / target:** kahdeksan lomakeyksikköä, noin 16 000 riviä: `cbHk/UnitEmit.cpp`,
  `cbHk/UnitAjanotto.cpp`, `cbHk/UnitArvonta.cpp`, `cbHk/UnitKilpailijatiedot.cpp`,
  `ViestiWin/UnitEmit.cpp`, `ViestiWin/UnitAjanotto.cpp`, `ViestiWin/UnitArvonta.cpp` ja
  kilpailijatietolomakkeen viestivastine **`ViestiWin/UnitJoukkuetiedot.cpp`** (ei
  `OsuusTiedot.cpp`, joka on pelkkä katselulomake; luku 3.8). / eight form units; the relay
  counterpart of the competitor form is `UnitJoukkuetiedot.cpp`, not the read-only `OsuusTiedot.cpp`.
- **Rakenne / structure:** HK- ja viestiyksiköt ovat kopioimalla erkaantuneita haaroja, eivät
  jaettua koodia. Korjauksia on tehty vain toiselle puolelle (luku 4, T-034). Viestin
  emit-lomake sisältää itse kortin käsittelyn (`ProsEmit`), HK kutsuu ytimen `pros_emit`ia. /
  HK and relay units are copy-paste forks; fixes were applied to one side only; relay card
  processing lives in the form, HK's in the core.
- **Uudet viat / new defects:** **H-109–H-127** (19 riviä, 28 agentin ehdokasta N-1–N-28):
  korkea 3, keskitaso 6, matala–keskitaso 7, matala 3 (kaksi matalaa riviä kokoaa useita
  pieniä vikoja). Tekninen velka **T-034–T-037**. / 19 rows from 28 candidates: high 3,
  medium 6, low–medium 7, low 3; technical debt T-034–T-037.
- **Korkeat / high:**
  - **H-109** "Poista kilpailija" ilman tarkistusta: HK:ssa `dKilp = -1` kirjoittaa
    `kilparr[-1]`:een ja hävittää vapaiden tietueiden listan; kaksoispoisto kilpailutilassa
    linkittää tietueen itseensä, jolloin kaksi seuraavaa lisättyä kilpailijaa saavat saman
    tietueen. / delete without guard: heap underflow, lost free list, double delete links a
    record to itself and two new competitors share one record.
  - **H-110** viestin "Lähtöajat" jättää `tall_CriticalSection`in vapauttamatta jokaisen
    ohitetun tietueen kohdalla → työsäikeet (vastaanotto, emit-luenta) jumittuvat ohjelman
    loppuun asti. / relay start-time button leaks `tall` per skipped record; worker threads block.
  - **H-111** "Tallenna muutos" ajanottolomakkeessa muuttaa L-rivin (lähtöaika) maaliriviksi;
    HK kirjoittaa lähtöajan kilpailijan maaliajaksi (vain kokoonpanoissa, joissa on L-rivejä). /
    saving an L (start) row from the edit panel turns it into a finish row; HK writes the start
    clock as the finish time.
- **Laajennetut / extended:** H-017 (kavennettu ja uudet ilmentymät), H-085 (toinen laukaisija),
  H-069 (viittaus H-112:een). / H-017 narrowed with new manifestations; H-085 second trigger.
- **Osa C / part C:** yhdeksästä jäsennysehdokkaasta seitsemän on indeksin kannalta
  turvallisia, yksi on H-085 (lisäpolku) ja yksi osittain turvaton (matala, H-126). / 7 of 9
  safe as indices, 1 is H-085, 1 partly unsafe (low).
- **Riippumaton tarkastus / review:** 16 väitettä kahdella Opus-tarkastajalla; vahvistettu 10,
  osittain 6, kumottu 0. Tarkastus korjasi useita vaikutuksia (luku 8). / 16 claims, 10
  confirmed, 6 partly, 0 refuted; several effects corrected.

## 2. Rajaus ja menetelmä / Scope and method

Rajaus: `suunnitelma.md` "C-7 vaihe 1". Lähtötietoina, ei analysoitu uudelleen: H-010,
H-017, H-069, H-080, H-085, H-089, H-092, H-107, C-6:n säie- ja lukkoaiheet ja C-5:n
kääntäjäerot. Ei tokenbudjettia (käyttäjän linja C-6:sta).

Scope as planned; earlier findings and C-5/C-6 topics were inputs only. No token budget.

| Osa / Part | Agentti, malli / Agent, model | Tulos / Output |
|---|---|---|
| A | `architecture-analyst`, Sonnet | `todisteet/c7/osaA-moduulikuvaukset.md` |
| B + C | `defect-concurrency-auditor`, Sonnet | `todisteet/c7/osaBC-logiikkavirheet.md` (N-1–N-13) |
| B, täydennys / supplement | `defect-concurrency-auditor`, Sonnet | `todisteet/c7/osaB2-taydennys.md` (N-14–N-28; kattavuusaukot / coverage gaps) |
| D, kierros 1 / round 1 | `independent-verifier`, Opus | `todisteet/c7/osaD-tarkastus.md` (N-1–N-4, N-6–N-10, osa C:n `:916`) |
| D, kierros 2 / round 2 | `independent-verifier`, Opus | `todisteet/c7/osaD2-tarkastus.md` (N-14–N-20) |
| E | pääistunto / main session | tämä dokumentti, rekisterit / this document, registers |

Poikkeama rajauksesta: ensimmäinen B-ajo luki kilpailija- ja joukkuelomakkeesta vain otteita
eikä lukenut `Paivita`-funktioita, joten tehtiin täydentävä B-ajo; sen keskitason ja korkeat
väitteet tarkastettiin toisella kierroksella. Riippumaton tarkastus kattoi kaikki korkeat ja
keskitason väitteet sekä olemassa olevia havaintoja muuttavat väitteet. Matalat ja
tarkastamattomat matala–keskitason väitteet (N-5, N-11–N-13, N-21–N-28) ovat yhden agentin
tuloksia.

Deviation: the first B pass only sampled the competitor and team forms and skipped `Paivita`,
so a supplementary B pass was run and its medium/high claims reviewed in a second round. All
high and medium claims and all claims changing existing findings were reviewed; low claims
and unreviewed low–medium ones are single-agent results.

## 3. Moduulikuvaukset / Module descriptions

Tiivistelmä; funktiotaulukot ja ytimeen menevät kutsut: `todisteet/c7/osaA-moduulikuvaukset.md`.
Summary; full function tables and core calls in the raw part A report.

### 3.1 `cbHk/UnitEmit.cpp` (`TFormEmit`, HkKisaWin)

Emit-kortin käsittelylomake: näyttää uusimman korttitietueen, siihen liitetyn kilpailijan ja
tarkastetut leimaukset; kortin liittäminen kilpailijaan (myös nimihaulla ja
henkilörekisteristä), leimausten ja aikojen korjaus `FormEmitMuutokset`-dialogissa,
maaliajan ja tuloksen laskenta ja tulosteet. Uusi kortti käsitellään ytimen
`pros_emit`-funktiossa (`cbHk/UnitEmit.cpp:936` → `Hk/HkIV.cpp:1162`). Lisäksi
kuntosuunnistustilat (`kuntosuunnmoodi` 1, 2, 3, 98, 99; `esiluenta`, `kuntoLahto`), joiden
merkitys on dokumentoimatta (Q75).

Card form: shows the latest card, its competitor and checked punches; links cards to
competitors, edits punches in the modeless Muutokset dialog, computes the finish and result,
prints. New cards go through core `pros_emit`. Also hosts fitness-orienteering modes whose
semantics are undocumented (Q75).

Kirjoituspolut / write paths: `tallennaKilpailija` (`:1615`, lukon alla / under lock),
`TallennaMuutokset` (`:2628`, `:2648`, `:2618`), `TallennaVaihto` (`:2775`–`:2799`),
`MerkitseOhitetuksi` (`:2741`), `BtnPerKirjausClick` (`:3001`), `BtnOhitaClick` (`:3483`,
H-017), `Laskeuudelleen1Click` (`:3449`); emit-tietueet `Em.put`/`putem`, `setEmitJarr`,
`addbadge`. Aikajonoihin ei kosketa. / no time-queue access.

### 3.2 `ViestiWin/UnitEmit.cpp` (`TFormEmit`, ViestiWin)

Sama käyttötarkoitus joukkue–osuus-avaimella. Kortin käsittely on lomakkeessa:
`ProsEmit` (`ViestiWin/UnitEmit.cpp:893`) hakee joukkueen ja osuuden (`HaeKilpailija`,
`:732`), kysyy tarvittaessa (`KysyKilpailija`, `:767`), kirjoittaa tietueen ja kortin
`tall_CriticalSection`in alla ja lähettää kortin muille koneille (`laheta_em`, `:931`).
`ProsEmitMaali` (`:817`) vie kortin maaliajan myös aikajonoon (`tall_rivi`, `:852`).
Kuntosuunnistustiloja ja henkilörekisteriä ei ole; tilalla `jalkisyotto`, `esitark`,
`val_korkein`, `huomaatark`.

Same purpose keyed by team and leg. Card processing is in the form (`ProsEmit`), including
the write under the lock, sending to peers and pushing the card finish time into the time
queue. No fitness modes or person register.

### 3.3 `cbHk/UnitAjanotto.cpp` (`TFormAjanotto`, HkKisaWin)

Käsi- ja puoliautomaattinen ajanotto: yksi tai kaksi aikajonoa ruudukoissa, ajan otto
näppäimellä (`lisaaAika`, `:558`), rivien lisäys, muokkaus ja poisto, rivin kilpailijanumero,
piste ja kortti. Rivin tallennus vie ajan kilpailijatietueeseen (`BtnTall2Click`, `:616`;
ruudukkomuokkain `DGajat1KeyPress`, `:1011`). Sarja- ja erälähdöt rivistä (`SarjaLahto`,
`:888`), Sirit-lukijan ohjaus ja jonon tuonti ja vienti tekstinä. Näyttöfunktio
`naytaRivi` kirjoittaa syöttötilassa 1 seuraavan numeron muistiosta riville ja kilpailijalle
(`:239`–`:272`, H-127).

Manual timing: time queues in grids, key-press timing, row edit/insert/delete, number, point
and badge per row; saving a row writes the time to the competitor. Class/heat starts, Sirit
controls, text import/export. The display function `naytaRivi` writes in input mode 1.

### 3.4 `ViestiWin/UnitAjanotto.cpp` (`TFormAjanotto`, ViestiWin)

Kuten HK, mutta rivillä on myös osuus (`upt.osuus`), joka johdetaan kortista tai joukkueen
seuraavasta keskeneräisestä osuudesta (`haeKnoOs`, `haeseurosuus`). Tallennus
`setMaali(osuus, piste, aika)`. Ei Sirit-ohjausta, eriä eikä syöttötiloja 4–6.
Paneelin tallennus (`Button2Click`, `:617`) päivittää vain jonon, ei joukkuetietuetta (Q76).

As HK plus a leg per row; writes with `setMaali`. No Sirit, heats or input modes 4–6. The
edit-panel save updates only the queue, not the team record (Q76).

### 3.5 `cbHk/UnitArvonta.cpp` (`TFormArvonta`, HkKisaWin)

Lähtöjärjestyksen arvonta (ytimen `arvo`, `cbHk/HkArv2.cpp:1129`), lähtöaikojen asetus
sarjan parametreista (`BtnToteutaClick`, `:338`), numerointi kahdeksalla tavalla
(`BtnNumeroiClick`, `:515`) ja sarjan jako useaksi sarjaksi (`ValmisteleJako`, `:806`;
`ToteutaJako`, `:901`). Kirjoitukset `kilp.tallenna` lukon alla; indeksit rakennetaan
uudelleen (`teeindeksi`, `luo_posarr`).

Start-order draw (core `arvo`), start-time assignment, eight numbering modes and class
splitting; writes under the lock; indexes rebuilt.

### 3.6 `ViestiWin/UnitArvonta.cpp` (`TFormArvonta`, ViestiWin)

Paljon suppeampi: joukkueiden numerointi (aakkos-, tallennus-, satunnais- ja
rankijärjestys, siirto) ja osuuksien lähtöajat (`BtnLahtoajatClick`, `:523`). Arvontaa ja
sarjan jakoa ei ole; `arvo(int)` on vain esitelty (`:33`). Satunnaisnumerointi sekoittaa
numerot Mersenne twisterillä (`arvonumerot`, `:328`). Numeroinnin törmäystarkistus
`varmista_vapaa` (`:351`) puuttuu HK:sta.

Much smaller: team numbering and leg start times; no draw or split; random numbering is an
MT shuffle; has a collision check that HK lacks.

### 3.7 `cbHk/UnitKilpailijatiedot.cpp` (`TFormKilpailijatiedot`, HkKisaWin)

Yhden kilpailijan selaus- ja muokkauslomake; päiväkohtainen ruudukko `PvGrid`. Muokkaukset
kopioon `Kilp`, alkuperäinen `Kilp1`; tallennus (`tallennaTiedot`, `:588`) lisää
(`addtall`, `:601`) tai kirjoittaa (`tallenna`, `:627`) vain, jos tietue ei ole muuttunut
muualla (`Kilp2` vs `Kilp1`, `:613`–`:616`; tarkistus lukon ulkopuolella). Poisto
`deltall` (`:1749`, H-109). Selaus numero-, nimi- ja sijoitusjärjestyksessä.

Browse/edit form for one competitor with a per-day grid; saves only if the record has not
changed elsewhere (check outside the lock); delete via `deltall` (H-109).

### 3.8 `ViestiWin/UnitJoukkuetiedot.cpp` (`TFormJoukkuetiedot`, ViestiWin)

HK-lomakkeen vastine samalla rakenteella (`EdtKilpnoChanged`, `naytaTiedot`, `tallennaTiedot`,
`paivitaMuutos`, `Kilp`/`Kilp1`/`dKilp`). Osuusruudukko `OsGrid` ja väliaikaruudukko
`TlsGrid`. "Muuttunut muualla" -vertailu tehdään lukon sisällä
(`ViestiWin/UnitJoukkuetiedot.cpp:518`–`:528`), toisin kuin HK:ssa. `OsuusTiedot.cpp` on
osuuden katselulomake ilman kirjoituksia (vain `uusiemitanalyysi`, `ViestiWin/OsuusTiedot.cpp:470`).

Counterpart of the HK form with leg and split grids; its conflict check is inside the lock,
unlike HK. `OsuusTiedot.cpp` is a read-only leg viewer.

## 4. HK- ja viestiversioiden erot / HK vs relay differences

| Pari / Pair | Ero / Difference | Merkitys / Significance |
|---|---|---|
| Emit | Kortin käsittely: HK ydin `pros_emit`, viesti lomakkeen `ProsEmit` / card processing core vs form | HK:n GUI ja konsoli jakavat käsittelyn, viesti ei [päätelty] |
| Emit | Viesti vie kortin maaliajan aikajonoon (`ViestiWin/UnitEmit.cpp:852`), HK ei / relay pushes card finish to the time queue | – |
| Emit | Viestin `TallennaMuutokset` pitää lukon koko toimenpiteen ajan, HK vapauttaa välillä / relay holds the lock throughout | H-092:n kannalta ei analysoitu / not analysed for H-092 |
| Ajanotto | Piste yli sarjan rajan: viesti suojaa (`ViestiWin/UnitAjanotto.cpp:1170`), HK ei (H-117) / relay guards, HK does not | yksipuolinen korjaus / one-sided fix |
| Ajanotto | Viestin `tall_rivi`-kutsusta puuttuu argumentti (H-121) / relay call drops an argument | kopiointivirhe / copy error |
| Ajanotto | A/L-jäsennys väärin molemmissa (H-111) / A/L parse wrong in both | yhteinen vika / shared defect |
| Arvonta | HK asettaa `NumKaikki`n, viesti ei (H-069) / HK sets it, relay does not | yksipuolinen / one-sided |
| Kilpailija/joukkue | Muuttunut muualla -vertailu: viesti lukon sisällä, HK ulkopuolella (H-017) / conflict check inside vs outside the lock | yksipuolinen korjaus [päätelty] |
| Kilpailija/joukkue | HK:n `EdtNimihakuChange` vertailee (`==`), viesti asettaa (`=`) (H-127) / HK compares, relay assigns | yksipuolinen korjaus |

Kumpi haara on uudempi, jäi selvittämättä (versiohistoriaa ei luettu; Q77). / Which fork is
newer was not determined (Q77). Ks. / see T-034.

## 5. Uudet havainnot / New findings

Rekisterin rivit: `rekisteri/havainnot.md`. N-tunnukset viittaavat raakatuloksiin. /
Register rows in `rekisteri/havainnot.md`; N IDs refer to the raw results.

| ID | N | Havainto / Finding | Vakavuus / Severity | Tarkastus / Review |
|---|---|---|---|---|
| H-109 | N-14 | "Poista kilpailija" ilman `dKilp`- ja tilatarkistusta (HK ja viesti): `-1` → `kilparr[-1]` ja vapaiden lista menetetään; `0` → tiedoston otsake vapaiden listaan; kaksoispoisto → itseensä linkitetty tietue, kaksi lisäystä samaan tietueeseen / delete without guard | korkea / high | vahvistettu / confirmed (D2) |
| H-110 | N-3 | Viestin `BtnLahtoajatClick`: `continue` ennen `LeaveCriticalSection`ia → `tall` jää pääsäikeelle / lock leak | korkea / high | vahvistettu / confirmed (D1) |
| H-111 | N-1 | Ajanoton "Tallenna muutos": puuttuva `else` muuttaa A/L-rivin maaliriviksi; HK kirjoittaa L-rivin lähtöajan maaliajaksi / missing else turns A/L rows into finish rows | korkea / high (HK, L-rivit / L rows); muuten matala / otherwise low | osittain / partly (D1) |
| H-112 | N-2 | Viestin `TallennaVaihto`: alustamaton `kno`, `d = -1` ja `os = -1` hyväksytään; kortin numero korvautuu pinon roskalla ja tallennetaan / uninitialised `kno`, invalid `d`/`os` | keskitaso / medium | vahvistettu, vaikutus korjattu / confirmed, effect corrected (D1) |
| H-113 | N-6 | Muokattavat pudotusvalikot ja "Ei valittu" indeksinä: viestin `CBLAOsuudet` nimiöt kahdella siirtyneet (valinta "1. osuus" muuttaa osuutta 3) ja −1 → kirjoitus `ostiet[-2]`:een; viestin Siirrä `osuus = -1`; HK `CBNumPaiva` / combobox index misuse | keskitaso / medium | osittain / partly (D1) |
| H-114 | N-18 | Sarjavalikon `ItemIndex = -1` kilpailija- ja joukkuelomakkeessa: viesti siirtää joukkueen hiljaa sarjaan 0; HK `sijat(-1)` → mahdollinen kasaylivuoto / series combo -1 | keskitaso / medium | vahvistettu, vaikutus pahempi / confirmed, worse (D2) |
| H-115 | N-19 | HK `naytaTiedot`: `PvGrid->Row` alueen ulkopuolella → VCL-poikkeus `tall`-lukon sisällä ilman `try/finally` → lukko jää päälle / grid exception inside the lock | keskitaso / medium | vahvistettu / confirmed (D2) |
| H-116 | N-15 | `select3`-dialogin sulkeminen X:llä palauttaa 2 = painike 2, joka on useassa emit-dialogissa tuhoava valinta / closing select3 with X equals button 2 | keskitaso / medium | vahvistettu / confirmed (D2) |
| H-117 | N-7 | HK ajanotto: piste yli sarjan rajan tyhjentää vanhan tuloksen eikä kirjoita uutta; `tall_rivi`n paluuarvoa ei tarkisteta / out-of-range point clears old result silently | keskitaso / medium | vahvistettu / confirmed (D1) |
| H-118 | N-4 | Viestin numerosiirto (Toiminto 4) asetustilassa käyttää kilpailijanumeroa tietuekursorina / bib used as record cursor | matala–keskitaso / low–medium | vahvistettu / confirmed (D1) |
| H-119 | N-16 | HK `lueKilp3`: väärä numero kopioi paikanpitäjän tilan, tuloksen ja lähtöajan olemassa olevalle kilpailijalle ilman kysymystä (vaikuttaa `kuntoLahto`-tilassa) / overwrite without confirmation | matala–keskitaso / low–medium | osittain / partly (D2) |
| H-120 | N-17 | HK: kortin paikanpitäjätietue jää omistamaan kortin, kun numero kirjoitetaan käsin tiloissa 0 ja 1; ydin kieltäytyy siirrosta virheilmoituksella / placeholder keeps the card | matala–keskitaso / low–medium [päätelty] | osittain / partly (D2) |
| H-121 | N-20 | Viestin `tall_rivi`-kutsu: `pakLah` menee `kielto`-paikkaan → Ctrl+Enter-pakotettu uudelleenlähetys ei pakota ja ohittaa ensimmäisen yhteyden / argument shift | matala–keskitaso / low–medium | vahvistettu / confirmed (D2) |
| H-122 | N-25 | Viestin rankinumerointi katkaisee 1000 joukkueeseen sarjaa kohden ilman ilmoitusta / silent 1000-team cap | matala–keskitaso / low–medium | ei tarkastettu / not reviewed |
| H-123 | N-5 | Viestin numerointi: esitarkistuksen ja ajonaikaisen rajan ero, osittainen kirjoitus keskeytyksessä, onnistumisilmoitus silti / partial renumber with success message | matala–keskitaso / low–medium | ei tarkastettu / not reviewed |
| H-124 | N-13 | HK numeroiden siirto ja `SprLisa`: ei raja- eikä päällekkäisyystarkistusta / no range or duplicate check | matala–keskitaso / low–medium | ei tarkastettu / not reviewed |
| H-125 | N-9 | HK lähtöaikojen asetus jakaa `pari`lla ilman nollatarkistusta lukon sisällä; nolla mahdollinen vain TakaaAjo-lomakkeesta tai CSV-/tekstituonnista / division by `pari` | matala / low | osittain / partly (D1) |
| H-126 | N-11, N-23, N-24, N-27, C:633 | Viestin pienet logiikkaviat (kokoomarivi) / relay minor defects (collective) | matala / low | ei tarkastettu / not reviewed |
| H-127 | N-12, N-21, N-22, N-24, N-26, N-28, Ctrl+E | HK:n (ja N-21:n osalta molempien) pienet logiikkaviat (kokoomarivi) / HK minor defects (collective) | matala / low | ei tarkastettu / not reviewed |

### 5.1 Korkeat havainnot tarkemmin / High findings in detail

**H-109.** Valikkokohtaa "Poista kilpailija" ei estetä missään tilassa
(`cbHk/UnitKilpailijatiedot.cpp:1746`–`:1752`, `ViestiWin/UnitJoukkuetiedot.cpp:1365`–`:1371`).
(1) Lomake luodaan arvolla `dKilp = -1` (`cbHk/UnitKilpailijatiedot.cpp:1518`); poisto ennen
kilpailijan valintaa kutsuu `deltall(-1)`. HK:n `PUTREC` tarkistaa vain ylärajan
(`Hk/HkDat.cpp:612`), joten `kilparr[-1] = *this` kirjoittaa taulukon eteen ja `operator=`
kirjoittaa `pv`-osoittimen kautta; kaatuminen todennäköinen [päätelty]. `deleterec(-1)`
asettaa `firstfree`-arvoksi 0xFFFFFFFF, jolloin koko vapaiden tietueiden lista menetetään
(`dbboxm/dbbox1.cpp:227`–`:270`). (2) "Lisää" → "Peruuta" jättää `dKilp = 0`; poisto vie
tiedoston otsaketietueen vapaiden listaan, ja seuraava lisätty kilpailija katoaa. (3)
Kilpailutilassa (`ToimintaTila == 2`) oletusselausjärjestys "Sijoitus" ei siirry pois
poistetusta kilpailijasta (`cbHk/UnitKilpailijatiedot.cpp:1157`–`:1176`), joten käyttäjä voi
painaa Poista uudelleen; toinen `deltall` linkittää tietueen itseensä, kaksi seuraavaa
lisäystä saavat saman tietueen ja kolmas todennäköisesti otsakkeen [päätelty]. Viestissä
paikallinen `kilptietue` on alustamaton (T-035). Ajonaikainen vahvistus puuttuu.

The delete menu item is never disabled. (1) `dKilp = -1` reaches `deltall(-1)`: HK writes
`kilparr[-1]` (crash likely) and `deleterec(-1)` drops the whole free list. (2) Add→Cancel
leaves `dKilp = 0`; deleting puts the file header on the free list. (3) In competition mode
the default browse order does not move off a deleted competitor; a second delete self-links
the record, so the next two adds share one record. Not confirmed by running.

**H-110.** `ViestiWin/UnitArvonta.cpp:544` ottaa `tall_CriticalSection`in silmukan alussa,
`:546`–`:547` jatkaa (`continue`) poistetuilla ja valitsemattomien sarjojen tietueilla, ja
ainoa `Leave` on `:576`. Käytännössä jokainen painallus jättää lukon pääsäikeen haltuun
moninkertaisesti. Pääsäie jatkaa (rekursiivinen lukko), mutta jokainen `tall`ia tarvitseva
työsäie (vastaanotto, emit- ja aikalukijat) pysähtyy ohjelman loppuun asti; GUI:n
jäätyminen myöhemmin C-6:n lukkoreunojen kautta on mahdollinen [päätelty]. Lähtöaikojen
siirto on uskottava kilpailupäivän toimenpide (lähdön viivästys). Ei C-6:n kirjauksissa.

The loop enters `tall` each iteration and `continue`s without leaving for skipped records;
worker threads that need `tall` block until exit. Shifting start times is a plausible
race-day action. Not recorded in C-6.

**H-111.** `cbHk/UnitAjanotto.cpp:632`–`:640`: A- ja L-haarojen jälkeen erillinen
`if (cp == L'M') ... else upt.piste = _wtoi(...)` ajaa `else`-haaran, ja `_wtoi(L"A")` = 0
→ piste 0 (maali). Paneeli näyttää kirjaimet (`naytaAktAika`, `:521`), joten "Tallenna
muutos" millä tahansa A- tai L-rivillä muuttaa sen maaliriviksi. A-rivit (automaattinen
piste) tuottavat saman tuloksen kuin ruudukon oletus, joten haitta on vähäinen. L-rivit
(lähtöaika numeroineen: hiihdon lähtökirjaus, LAJAT-jono, aaltolähdöt) aiheuttavat
todellisen vahingon: lähtöaika tyhjennetään, ja lähtökellon aika kirjoitetaan
kilpailijan maaliajaksi (`:659`, `:672`); varoitus tulee vain, jos maaliaika on jo olemassa.
Viestissä vika kirjoittaa vain jonoon, ei joukkuetietueeseen (matala).

A missing `else` turns A/L rows into finish rows on panel save. A rows: low harm. L rows
(start times with a bib, used in ski start recording and wave starts): the start is cleared
and the start clock is written as the finish. Relay: queue only, low.

## 6. Laajennetut havainnot / Extended findings

- **H-017 kavennettu / narrowed:** `cbHk/UnitKilpailijatiedot.cpp:613`–`:626` sisältää
  "muuttunut muualla" -tarkistuksen; jäljelle jää vain lyhyt ikkuna tarkistuksen ja
  lukon välillä (matala). Varsinaiset ilmentymät ovat emit-lomakkeen ei-modaalisessa
  Muutokset-dialogissa: `Kilp_0` otetaan talteen (`cbHk/UnitEmit.cpp:2469`) ja kirjoitetaan
  kokonaisena myöhemmin (`:2694`, `:2722`–`:2736`, `:2791`); viesti samoin
  (`ViestiWin/UnitEmit.cpp:2209`, `:2269`). Vakavuus ennallaan (keskitaso), varmuus nyt
  [todennettu] koodi. / The competitor form has a conflict check (small window remains);
  the real manifestations are the Emit Muutokset dialog snapshots in both programs.
- **H-085, toinen laukaisija / second trigger:** jakotavassa 'P' `ToteutaJako` käy läpi
  ruudukon kaikki rivit, mutta `msrj[]` sisältää vain kelvolliset rivit
  (`cbHk/UnitArvonta.cpp:949`–`:984`, `:1048`–`:1060`); tyhjä tai virheellinen sarjanimi
  välirivillä antaa alustamattoman `msrj[rs]`-arvon → `setSarjaAll(roska)`. Vakavuus
  ennallaan (keskitaso). / In split mode 'P' a blank row yields an uninitialised series index.
- **H-069:** viestin `TallennaVaihto`n alustamaton `kno` (bcc32 W8013, C-4:n triage) on nyt
  osa H-112:ta. / The uninitialised `kno` is now part of H-112.

## 7. Osa C: jäsennysehdokkaat / Part C: parse candidates

| Ehdokas / Candidate | Ratkaisu / Verdict | Peruste / Reason |
|---|---|---|
| `cbHk/UnitEmit.cpp:2819` | turvallinen indeksinä / safe as index | `EmitGrid->RowCount` = `MAXNLEIMA+1` kiinteästi (`cbHk/UnitEmit.cpp:445`), silmukka `r < RowCount`; arvo voi kiertyä `char`-tyypissä (matala) / value may wrap |
| `cbHk/UnitEmit.cpp:2820` | turvallinen / safe | sama raja; `UINT16`-arvo kiertyy / value wraps |
| `cbHk/UnitEmit.cpp:2974` | turvallinen indeksinä / safe as index | `ARow < RowCount`; `(unsigned char)`-muunnos katkaisee yli 255:n ajat elävässä `Em`issä tallennukseen asti / truncation until save |
| `cbHk/UnitArvonta.cpp:916` | **H-085** + toinen laukaisija / second trigger | `ValmisteleJako` kirjoittaa kelvollisten rivien solut ensin; −1 tulee laskennasta (H-085); lisäpolku tyhjän rivin kautta (luku 6) |
| `cbHk/UnitKilpailijatiedot.cpp:757` | turvallinen / safe | `_wtoi` on arvo, ei indeksi; `ValPist` 0 tai 1, `pisteet[3]` |
| `ViestiWin/UnitEmit.cpp:2356` | turvallinen / safe | `RowCount` = 51, taulukot 50 (`ViestiWin/UnitEmit.cpp:446`) |
| `ViestiWin/UnitEmit.cpp:2357` | turvallinen / safe | sama / same |
| `ViestiWin/UnitEmit.cpp:2504` | turvallinen indeksinä / safe as index | kuten `:2974` / as `:2974` |
| `ViestiWin/UnitAjanotto.cpp:633` | osittain / partly (matala / low, H-126) | ei rajatarkistusta ennen jonoon tallennusta; lukevat `Maali`/`setMaali` hylkäävät rajan ulkopuoliset; `uusintaTallennus` ilman `>= 0` |

Muut 152 C-5:n ehdokasta jäävät erilliseksi jatkotehtäväksi (`seuranta.md`). / The other 152
remain a separate follow-up.

## 8. Riippumattoman tarkastuksen korjaukset / Corrections from independent review

Tarkastajat eivät kumonneet yhtään väitettä, mutta korjasivat vaikutuksia ja vakavuuksia:

- N-1 (H-111): yhtenäinen "korkea" → korkea vain L-riveillä, A-riveillä matala.
- N-2 (H-112): kortti korvautuu alustamattomalla pinodatalla eikä tyhjene; `os = -1`
  kirjoittaa olion ulkopuolelle; `getpos` palauttaa 0 eikä −1 käyttämättömälle numerolle.
- N-6 (H-113): `ostiet[-2]` on pinokirjoitus, ei tietueeseen; nimiöiden siirtymä ei ole
  kosmeettinen; `UnitEmitMuutokset.cpp`:n toinen kutsu ei ole suojattu.
- N-8: rajoittuu jakotapaan 'P'.
- N-14 (H-109): vakavuus keskitaso → korkea; `dbboxm/Dbbox.cpp` ei ole käännöksessä, oikea
  tiedosto `dbboxm/dbbox1.cpp`; `-1` hävittää vapaiden listan.
- N-16, N-17 (H-119, H-120): keskitaso → matala–keskitaso; esiluennan tallennus nollaa
  kentät suunnitellusti; ristiriita vain tiloissa 0 ja 1 käsin kirjoitetulla numerolla.
- N-18 (H-114): viestissä ydin ei hylkää vaan siirtää joukkueen sarjaan 0; HK:ssa `sijat(-1)`.
- N-15 (H-116): lisäkutsuja `cbHk/UnitEmit.cpp:1511`–`:1514`.

Reviewers refuted none but corrected effects and severities as listed.

## 9. Kattavuus / Coverage

| Yksikkö / Unit | K1 syötteet / input | K2 kortti / card | K3 ajanotto / timing | K4 arvonta / draw | K5 peruutus / cancel | Taso / Level |
|---|---|---|---|---|---|---|
| `cbHk/UnitEmit.cpp` | H-127 (N-12) | H-119, H-120, H-116 | – | – | H-017 | 3 |
| `ViestiWin/UnitEmit.cpp` | H-112, H-113 | H-112, H-116 | (`ProsEmitMaali`) ei vikaa / none | – | H-017 | 3 |
| `cbHk/UnitAjanotto.cpp` | H-111, H-117 | – | H-111, H-117, H-127 | – | `tall_rivi` (H-117) | 3 |
| `ViestiWin/UnitAjanotto.cpp` | H-126 | – | H-111, H-121, H-126 | – | ei vikaa / none | 3 |
| `cbHk/UnitArvonta.cpp` | H-113, H-124, H-125 | – | – | H-085, H-124; satunnaisuus kunnossa / randomness fine | ei peruutusta / no rollback | 3 |
| `ViestiWin/UnitArvonta.cpp` | H-113, H-118 | – | – | H-110, H-122, H-123 | H-123 | 3 |
| `cbHk/UnitKilpailijatiedot.cpp` | H-114, H-115, H-127 | – | – | – | H-109 | 3 |
| `ViestiWin/UnitJoukkuetiedot.cpp` | H-114, H-126 | – | – | – | H-109 | 3 |

Arvontaa itseään (`cbHk/HkArv2.cpp`) ei analysoitu (rajauksen ulkopuolella). Tasoa 4 ei
annettu, koska kaikkia tiedostoja ei luettu rivi riviltä. / The draw itself (`HkArv2.cpp`) was
out of scope. Level 4 not given since files were not all read line by line.

**Kumotut epäilyt / refuted suspicions:** `getAktTime` rivillä −1 (UINT32-raja); viestin
`lahtolaji` −1 (UINT8, rajataan heti); arvonnan satunnaisuus (staattinen MT-tila, ei
uudelleensiementämistä); HK `TallennaMuutokset` hukkaa `Em.kilpno`n (ydin päivittää);
`EmitGrid->RowCount` kasvaa (kiinteä); `keyn` alustamaton `nextix`ille (vain tulos);
`naytaRivi` soveltaa vanhaa riviä uudelleen (`kno` asetetaan ensimmäisellä kerralla);
viestin `LLaji[lahtolaji+1]`; HK `Paivita` ilman `d`:tä.

## 10. Avoimet kysymykset ja ajoa vaativat asiat / Open questions and items needing runs

| # | Kysymys / Question |
|---|---|
| Q75 | `kuntosuunnmoodi`-arvojen 1, 2, 3, 98, 99 sekä `esiluenta`n ja `kuntoLahto`n merkitys; H-119, H-120 ja N-22 riippuvat niistä / semantics of the fitness modes |
| Q76 | Päivittääkö viestin ydin joukkuetietueen, kun ajanoton paneelista tallennetaan vain jonorivi (`ViestiWin/UnitAjanotto.cpp:657`)? / Does the relay core update the team record after a queue-only save? |
| Q77 | Kumpi HK/viesti-haara on uudempi kussakin yksipuolisessa korjauksessa (versiohistoria)? / Which fork is newer per one-sided fix? |
| Q15 | **Osittain ratkaistu / partly resolved:** kyllä, lomakkeiden käsittelijöissä VCL-poikkeus `Enter`/`Leave`-parin välissä jättää `tall`in päälle (H-115, H-125, T-036); `tallenna`-polkua itseään ei tutkittu |

Ajoa vaativat (siirretään testausvaiheeseen, kuten C-6:n jatkotehtävät) / need runs:
H-109 (`kilparr[-1]`, kaksoispoiston kolmas lisäys, `remjarjarr` irrotetulle tietueelle),
H-110 (työsäikeiden pysähtyminen), H-111 (L-rivi paneelista), H-112/H-113 (olion ulkopuolisten
kirjoitusten osuma bcc32:n asettelussa), H-114 (`nilm[-1]`/`Sarjat[-1]` linkitysasettelussa),
H-115 (VCL-poikkeus ja lukon laskuri), H-116 (X → 2 tässä C++Builder-versiossa), H-120
(ristiriita tiloissa 0 ja 1), H-125 (`EDivByZero` bcc32:ssa).

## 11. Korjausjärjestys / Fix priority [päätelty / inferred]

C-4:n asteikolla (`muistiturvallisuus.md` luku 15) ja C-6:n sijoituksen rinnalla:
**P2** H-109, H-110 (tietojen korruptio tai työsäikeiden pysähtyminen tavallisella
käytöllä kilpailun aikana); **P3** H-111, H-112, H-113, H-114, H-115, H-116, H-117;
**P4** H-118–H-125; **P5** H-126, H-127. Korjauksia ei aloitettu.

On the C-4 scale: P2 H-109, H-110; P3 H-111–H-117; P4 H-118–H-125; P5 H-126, H-127. No
fixes started.

## 12. Resurssien käyttö / Resource usage

| Agentti / Agent | Tokenit / tokens | Työkalukutsut / tool calls |
|---|--:|--:|
| `architecture-analyst` (Sonnet; osa A) | 231 406 | 47 |
| `defect-concurrency-auditor` (Sonnet; osat B + C) | 228 191 | 71 |
| `defect-concurrency-auditor` (Sonnet; B-täydennys / supplement) | 287 065 | 92 |
| `independent-verifier` (Opus; kierros 1 / round 1) | 126 440 | 64 |
| `independent-verifier` (Opus; kierros 2 / round 2) | 131 759 | 68 |
| **Yhteensä / total** | **1 004 861** | **342** |
