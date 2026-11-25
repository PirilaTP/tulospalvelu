# Seuranta / Tracking

Päivitä tätä tiedostoa jokaisen työvaiheen lopuksi. Dokumentointikattavuutta
seurataan erikseen testikattavuudesta.
Update this file at the end of every work step. Documentation coverage is tracked
separately from test coverage.

## Tila / Status

| Vaihe / Phase | Tila / State | Päivä / Date |
|---|---|---|
| A. Selvitys, kierros 1 (inventaario, yleiskuva, merkistöt, suunnitelma) / audit round 1 | valmis, odottaa katselmointia / done, awaiting review | 2026-11-24 |
| Auditointirekisteri ja projektin agentit / audit register and project agents (`rekisteri/`, `.claude/agents/`) | valmis / done | 2026-11-24 |
| B. Vikakorjaukset / bug fixes | ei aloitettu / not started | – |
| C, erä 1: `tallenna()`-samanaikaisuus / batch 1: `tallenna()` concurrency | valmis, tarkastettu / done, reviewed (`tallenna-samanaikaisuus.md`) | 2026-11-24 |
| C, erä 2: työasemien tiedonsiirto / batch 2: workstation communication | valmis, tarkastettu / done, reviewed (`tiedonsiirto.md`) | 2026-11-24 |
| C, erä 3: pysyvät tiedostot ja eheys / batch 3: persistent files and integrity | valmis, tarkastettu / done, reviewed (`tiedostomuodot.md`) | 2026-11-24 |
| C, erä 11: testiautomaatio, suunnittelu / batch 11: test automation, planning | valmis, tarkastettu, commitoitu `f5d793b` / done, reviewed, committed (`testaus.md`) | 2026-11-24 |
| C, erä 11, vaihe 1: H-025, H-040, H-042 / batch 11, stage 1 | valmis, hyväksytty, commitoitu `79d9957` / done, approved, committed (`testaus.md` luku 13) | 2026-11-24 |
| C, erä 11, vaihe 2: H-041, H-031, H-008, H-024, H-027 / batch 11, stage 2 | valmis, hyväksytty, commitoitu `cad2781` / done, approved, committed (`testaus.md` luku 14) | 2026-11-24 |
| C, erä 11, vaihe 3: H-026, H-032, H-022, H-023, H-007, H-001, H-002 / batch 11, stage 3 | valmis, hyväksytty, commitoitu / done, approved, committed (`testaus.md` luku 15) | 2026-11-24 |
| HkMaali.exe:n koeajot 0 ja 1 / black-box runs 0 and 1 | tehty palomuurieristyksellä, molemmat odotetusti; ensimmäinen AppContainer-yritys keskeytyi; ei H-vahvistuksia; done under firewall isolation, both as expected; first AppContainer attempt stopped; no H confirmed (`432ea3d`, `testaus.md` luku 17) | 2026-11-24 |
| C-11 vaihe 4: H-041 toimitetussa ohjelmassa / stage 4 | esiehdot `c6205d2`; ajot 220/221; ennalta kiinnitetty arvio "not confirmed" (kriteeri 7, loki-ikkuna); käyttäjän poikkeamapäätöksellä H-041 **vahvistettu ajamalla** / pre-fixed verdict not confirmed; H-041 confirmed by running by user-approved deviation (`testaus.md` 19.9–19.11) | 2026-11-24 |
| **C, erä 11 (C-11) kokonaisuutena / batch 11 as a whole** | **valmis / complete**; loppuyhteenveto / final summary `testaus.md` luku 20; raakatulokset / raw results `todisteet/c11/` | 2026-11-24 |
| **C, erä 4 (C-4) valmis / complete**, vaihe 2: syväanalyysi, tutkittavat rivit, muunnokset, kattavuus, kokonaisarvio / step 2 | **valmis, hyväksytty, suljettu, commitoitu** / done, approved, closed, committed (`muistiturvallisuus.md` luvut 9–17, H-070–H-077, T-022) | 2026-11-25 |
| C, erä 4 (C-4) muistiturvallisuus, vaihe 1: varoitusinventaario / memory safety, step 1: warning inventory | valmis, hyväksytty, riippumattomasti uudelleentarkastettu, commitoitu / done, approved, re-reviewed, committed (`muistiturvallisuus.md`, H-055–H-069, T-020, T-021, `todisteet/c4/`) | 2026-11-24 |
| **C, erä 5 (C-5) kääntäjäkohtainen käytös / compiler-specific behaviour, vaihe 1 / step 1** | **valmis, riippumattomasti tarkastettu, commitoitu** / done, independently reviewed, committed (`kaantajaerot.md`, H-078–H-081, T-023–T-028, H-004/H-005 laajennettu / extended, `todisteet/c5/`) | 2026-11-25 |
| C, erä 5 (C-5), vaihe 2 / step 2 | **valmis, riippumattomasti tarkastettu, commitoitu** / done, independently reviewed, committed `55897eb` (`kaantajaerot.md` luku 13: Q8 ratkaistu, `tyokalut/vertaa_omf.py`, H-082–H-086, T-029, H-042/H-079 laajennettu / extended; `todisteet/c5/vaihe2/`) | 2026-11-25 |
| **C, erä 5 (C-5) kokonaisuutena / batch 5 as a whole** | **valmis ja suljettu käyttäjän hyväksynnällä / complete and closed on approval** (`b17d536`, `55897eb`; sulkeminen `kaantajaerot.md` 13.7); avoinna Q60 (omistaja / owner), Q63, Q64; jatkotehtävät alla / follow-ups below | 2026-11-25 |
| **C, erä 6 (C-6) säikeet ja synkronointi, vaihe 1 / threads and synchronisation, step 1** | **valmis, riippumattomasti tarkastettu, commitoitu paikallisesti / done, independently reviewed, committed locally** (`saikeet-ja-lukot.md`, H-087–H-102, T-030–T-032, H-018/H-031/H-038 laajennettu / extended, Q30 ratkaistu / resolved, `todisteet/c6/`); vaihe 2 erillisellä päätöksellä / step 2 on a separate decision | 2026-11-25 |
| **C-6 vaihe 2 / step 2** (todennus ja jatkoanalyysi / verification and follow-up) | **valmis, riippumattomasti tarkastettu, commitoitu paikallisesti / done, independently reviewed, committed locally** (`saikeet-ja-lukot.md` luku 13; H-090, H-091 reunat, H-093, H-094 toistettu eristetyllä koeohjelmalla / reproduced with isolated probes, `todisteet/c6/vaihe2/`; H-090, H-093 → korkea; H-099 kumottu / refuted; uudet / new H-103–H-108, T-033; Q65, Q66, Q69, Q70–Q72, Q74 ratkaistu / resolved, Q67 osittain / partly); **C-6 suljettu / closed 2026-11-25** käyttäjän hyväksynnällä / with the user's approval; avoimet asiat jatkotehtävinä (alla / below) | 2026-11-25 |
| **C, erä 7 (C-7) GUI:n liiketoimintalogiikka, vaihe 1 / GUI business logic, step 1** | **valmis ja suljettu käyttäjän hyväksynnällä / done and closed on approval** (`e3a8b67`; `gui-liiketoimintalogiikka.md`; H-109–H-127, T-034–T-037; H-017 kavennettu / narrowed, H-085 ja H-069 laajennettu / extended; 9 jäsennysehdokasta ratkaistu / parse candidates resolved; Q15 osittain / partly, uudet / new Q75–Q77; `todisteet/c7/`); kattavuus ja jatkotyöjono J7-1–J7-7 luku 13 / coverage and follow-up queue in section 13 | 2026-11-25 |
| C, erä 8 (C-8) globaali muuttuva tila / global mutable state | **vaihe 1 rajattu, ei aloitettu / step 1 scoped, not started** (`suunnitelma.md` "C-8 vaihe 1"); odottaa aloituslupaa / awaiting approval | 2026-11-25 |
| C, erät 9–10 / batches 9–10 | ei aloitettu / not started (`suunnitelma.md`) | – |
| D. Kommentointi / commenting | ei aloitettu / not started | – |

**Seuraava askel / Next step:** C-7 on suljettu (2026-11-25). Seuraavaksi C-8:n vaihe 1
(globaali muuttuva tila), rajaus `suunnitelma.md` "C-8 vaihe 1"; alkaa vasta käyttäjän
luvalla. / C-7 closed; next C-8 step 1 (global mutable state) on approval.
Kiireellisesti
korjattavat H-071, H-072, H-055 ja H-070 korjataan omissa haaroissaan erillisellä
päätöksellä; väliaikaiset suojaussuositukset `muistiturvallisuus.md` luku 15. / C-5 closed;
next C-6 step 1 (thread and lock inventory) on approval; urgent fixes H-071, H-072, H-055,
H-070 on separate decision (queue below).

**Avoimet jatkotehtävät C-11:stä / open follow-ups from C-11** (eivät estäneet
sulkemista / did not block closing; `testaus.md` 20.4):
- H-009: käsitesti GUI:ssa (pakotettu ajoitus) / manual GUI test (forced timing)
- H-014: käsitesti HkKisaWin + paikallinen MySQL / manual test with local MySQL
- kahden prosessin loopback-kerros (H-022, H-032, H-027); H-041:n lukupolku (Q37) ja 250;
  testit CI:hin (Q35, Q36, Q40) / two-process loopback layer; H-041 read path; CI
- palomuurisääntöjen `TP C11 HkMaali block in/out` poisto (käyttäjä, ylläpitäjä) /
  removal of the C-11 firewall rules (user, admin)

**Jatkotyöjono C-7:stä / follow-up queue from C-7** (eivät estäneet sulkemista / did not block
closing; `gui-liiketoimintalogiikka.md` luku 13). GUI-syötteiden tarkastus ei ole valmis
kokonaisuutena. / The GUI input review is not complete as a whole.

| # | Tehtävä / Task | Sisältö / Content | Tyyppi / Type |
|---|---|---|---|
| J7-1 | Arvonta-algoritmi / draw algorithm | `cbHk/HkArv2.cpp` (`arvo`, `arvovaihe`, `arvonumerot`; 1 274 riviä, taso 1): kaksoisnumerot ja -ajat, rajat, uudelleenarvonta, jo lähteneet; C-7:n kysymys 4 jäi tältä osin auki / C-7 question 4 open for the draw itself | staattinen / static |
| J7-2 | Muut kilpailutietoja kirjoittavat lomakkeet / other forms writing competition data | 41 lomaketiedostoa `cbHk/`, `ViestiWin/`, `cbTp/` (koneellinen haku: `tallenna`, `addtall`, `deltall`, `putem`, `tall_rivi`, `setMaali`, `tall_tulos`, `set_tulos`). Ensin: `cbHk/UnitSprinthiihto.cpp`, `cbHk/UnitAikaSiirto.cpp`, `cbTp/UnitAikakorjaus.cpp`, `cbHk/UnitNollaus.cpp`, `ViestiWin/UnitNollaus.cpp`, `cbTp/UnitFIS.cpp`, `cbHk/UnitIlm2.cpp`, `cbHk/LoppuKilpUnit.cpp`, `ViestiWin/UnitLuenta.cpp`, `cbHk/UnitPistelasku.cpp`, `cbHk/UnitKilpailijaOnline.cpp`, `cbHk/UnitEmitMuutokset.cpp`; tuonti (`UnitImport`) käsitelty osittain C-3:ssa / first the listed units; import partly covered in C-3 | staattinen / static |
| J7-3 | 152 jäsennysehdokasta / parse candidates | `todisteet/c5/vaihe2/dca.tgz` → `dca/atoi_index_candidates.csv` ilman C-7:n yhdeksää: lomakkeet 96 (`cbHk/` 55, `ViestiWin/` 30, `cbTp/` 11), ydin 56 (`Hk/` 32, `Juk/` 23, `Tp/` 1); voidaan yhdistää J7-2:een tiedostoittain / may be merged with J7-2 per file | staattinen / static |
| J7-4 | Ajonaikaiset vahvistukset / run-time confirmations | ensisijaisesti **H-109** (`kilparr[-1]`, kaksoispoisto ja sitä seuraavat lisäykset, `remjarjarr` irrotetulle tietueelle), **H-110** (työsäikeiden pysähtyminen "Lähtöajat"-painalluksen jälkeen), **H-116** (X → 2 tässä C++Builder-versiossa); lisäksi H-111, H-112, H-113, H-114, H-115, H-120, H-125 (luku 10) | ajo, erillinen lupa / run, separate permission |
| J7-5 | Riippumaton tarkastus / independent review | H-122, H-123, H-124, H-126, H-127 (yhden agentin tulokset) ennen niiden korjaamista / before fixing | staattinen / static |
| J7-6 | Avoimet kysymykset / open questions | Q75 (kuntosuunnistustilat), Q76 (viestin jonotallennus ja joukkuetietue), Q77 (haarojen ikä, versiohistoria), Q15:n jäljellä oleva osa (`tallenna`-polun poikkeukset itse ytimessä) | staattinen / static |
| J7-7 | Korjaussuunnittelu / fix planning | T-034: jokainen HK- tai viestilomakkeen korjaus tarkistetaan myös vastinhaarasta (luku 4); T-035, T-036, T-037 yhteisinä korjausmalleina / check every fix against the other fork | korjausvaihe / fix phase |

**Avoimet jatkotehtävät C-6:sta / open follow-ups from C-6** (eivät estäneet sulkemista /
did not block closing; `saikeet-ja-lukot.md` 13.10, 13.11). Siirretty myöhempään testaus- tai
korjausvaiheeseen; vaativat toimitetun ohjelman tai bcc32-käännöksen ajamista:
- bcc32-käännöksen (HkKisaWin, ViestiWin) ajonaikainen todennus; erityisesti H-091:n sykli
  GUI:ssa (koeohjelmat ajoivat vain MSVC-konsolikäännöstä) / run-time verification of the bcc32 build
- H-087 ja H-103 ViestiWinissä (GUI-koodia ei voi linkittää konsolikoeohjelmaan; ehdotus:
  debuggeri tai Application Verifier, lukon `OwningThread`/`RecursionCount`) / H-087, H-103 in ViestiWin
- Q67: `aloitus`in kutsumien funktioiden apusäikeet ennen epäonnistumiskohtaa / helper threads
- Q68 (`opencomfile(1)` yhdellä yhteydellä) ja Q73 (konsolin `CTRL_CLOSE`, `maaliajat`)
- muut ajoa vaativat: H-090:n esiintyvyys oikeilla laitteilla, H-093:n `comfl`-ehto oikealla
  kokoonpanolla, H-094:n vapautetun käytön kilpa, H-106:n ristisäikeinen `TEdit::SetText`,
  H-107:n `ToimintaTila==1` + lukija, `SendMessage` tuhottavaan ikkunaan (lomake-elinikä),
  Q65 vanhemmilla Windowseilla ja odottajien kanssa / other items needing runs
- Q65:n vaikutus arviointiin on kirjattu H-088:aan ja H-089:ään (`rekisteri/havainnot.md`):
  tällä Windowsilla ylimääräinen tai väärän säikeen `Leave` ei jumita vaan vapauttaa lukon,
  jolloin seuraus on suojaamaton kriittinen alue; vakavuudet ennallaan (keskitaso) / Q65's
  effect on H-088/H-089 is recorded; severities unchanged

**Avoimet jatkotehtävät C-5:stä / open follow-ups from C-5** (eivät estäneet sulkemista /
did not block closing; `kaantajaerot.md` 13.6–13.7):
- **Syötteiden jäsennyksen läpikäynti / integer-parse review:** 161 lukematonta
  kokonaislukujäsennysehdokasta (`_wtoi`, `_wtol`, `atoi`; indeksi, varaus tai silmukan raja)
  GUI-ohjelmiin käännettävissä yksiköissä: lomakkeet `cbHk/` 60, `ViestiWin/` 34, `cbTp/` 11;
  ydin `Hk/` 32, `Juk/` 23, `Tp/` 1 (ydin käännetään myös konsoliin). Lista:
  `todisteet/c5/vaihe2/dca.tgz` → `dca/atoi_index_candidates.csv`. Erillinen jatkotehtävä;
  ei vielä numeroitu erä, voidaan yhdistää erään 7 (GUI:n liiketoimintalogiikka) päätöksellä;
  C-7:n vaihe 1 ratkaisi kohdetiedostojen 9 ehdokasta (`gui-liiketoimintalogiikka.md` luku 7:
  7 turvallisia, 1 = H-085, 1 osittain H-126), muut 152 ovat C-7:n jatkotyöjonossa (J7-3). /
  161 unread candidates (105 in form units, 56 in core units); separate follow-up, may be
  merged with batch 7 by decision.
- **H-079: saavutettavuus ja vakavuuden uudelleenarviointi / reachability and severity
  re-assessment:** nykyinen vakavuus matala. Selvitettävä (staattisesti): (1) avaako jokin
  saavutettava polku tavallisessa käytössä UTF-16-tiedoston uudelleen (`UnitSakkoKierr`
  molemmissa GUI:issa, `HkConsole/HkCons3.cpp:1564`) ja voiko ohjelma itse tuottaa sellaisen
  tiedoston; (2) ovatko ylivuotavat merkit aina kiinteä `", ccs=UTF-16LE"`/`BE`-loppu, jolloin
  vaikutus rajautuu kaatumiseen [päätelty, ei todennettu]; (3) voiko tiedosto tulla verkosta
  (esim. H-071:n tiedostokirjoitus) – ei analysoitu. Ajonaikainen vahvistus toimitetulla
  ohjelmalla vain erillisellä luvalla. / Static questions: reachable re-open in normal use,
  whether the overflowing bytes are always the fixed suffix (crash only), whether the file
  can arrive over the network (not analysed); run-time check only with permission.
- **Q60** omistajan päätettäväksi (`KUNTOSUUNN`-ero, H-081); **Q63** ja **Q64** avoimia
  teknisiä kysymyksiä (pieni koeohjelma tai lukeminen riittää). / Q60 owner decision; Q63, Q64
  open technical questions.
- Ajonaikaiset vahvistukset toimitetuilla ohjelmilla (FPU-sana / `VersioInt`, H-078, H-079)
  vain erillisellä luvalla. / Run-time confirmations only with permission.

Havainnot (H/T) ja niiden todennustila: `rekisteri/havainnot.md`.
Findings (H/T) and their verification status: `rekisteri/havainnot.md`.

## Priorisoitu korjausjono / Prioritised fix queue

Lähde: `muistiturvallisuus.md` luku 15 (C-4); tämä tiivistelmä ei muuta sen prioriteetteja.
Korjaukset tehdään omissa haaroissaan erillisellä päätöksellä, eikä yhtäkään ole aloitettu.
Väliaikaiset suojaussuositukset: `muistiturvallisuus.md` luku 15. / Source: memory-safety
section 15; this summary does not change its priorities. Fixes go in their own branches on a
separate decision; none started.

| # | Havainto / Finding | Syöte / Input | Tila / Status |
|--:|---|---|---|
| 1 | **H-071** FILESEND: pituus ja tiedostopolku / length and file path | verkko, oletusasetukset / network, default settings | **kiireellinen / urgent** (P1) |
| 2 | **H-072** `EXTRA`-paketin indeksit / indices | verkko, oletusasetukset / network, default settings | **kiireellinen / urgent** (P1) |
| 3 | **H-055** emiTag `P`-kentän indeksi / index | lukija, GUI `TCPS`, HTTP-pollaus (asetus) / reader, TCPS, polling (setting) | **kiireellinen / urgent** (P2) |
| 4 | **H-070** FEIG-lukijan kasaylivuoto / heap overflow | laite / device | **kiireellinen / urgent** (P2) |
| 5 | H-032, H-061 | verkko (UDP; sarja, binääri-TCP) / network | P1; H-061:n `pkglen` todennettava ensin (Q43) / verify first |
| 6 | H-056, H-060, H-073 | laite tai verkko asetuksella / device or network with a setting | P2 |
| 7 | P3–P5 | tavallinen data, paikallinen tiedosto, operaattori / ordinary data, local file, operator | `muistiturvallisuus.md` luku 15 |

C-6:n havainnot (H-087–H-108) ovat lukkiutumisia, vapauttamattomia lukkoja, elinkaaren
kilpailutilanteita ja VCL-käyttöä työsäikeistä; verkosta laukeavia syötevirheitä ei ole.
Päivitetty sijoitus vaiheen 2 jälkeen [päätelty] (`saikeet-ja-lukot.md` 13.9): **P2** H-087,
H-091, H-090, H-093 (jäätyminen tai kaatuminen kilpailun aikana; H-090 ja H-093 todennettu
koeohjelmalla), H-107 (kaatuminen ensimmäisestä luennasta asetuksella); **P3** H-094, H-103,
H-104, H-105, H-106, H-088, H-089, H-092, H-097; **P4** H-095, H-096, H-098, H-100, H-101,
H-102, H-108; H-099 kumottu ja poistettu jonosta. / Updated after step 2 [inferred]: P2 H-087,
H-091, H-090, H-093, H-107; P3 H-094, H-103–H-106, H-088, H-089, H-092, H-097; P4 the rest;
H-099 refuted and removed.

C-7:n havainnot (H-109–H-127) laukeavat käyttäjän toiminnosta lomakkeessa, eivät verkosta.
Alustava sijoitus [päätelty] (`gui-liiketoimintalogiikka.md` luku 11): **P2** H-109 (tietojen
korruptio), H-110 (työsäikeet pysähtyvät); **P3** H-111–H-117; **P4** H-118–H-125; **P5** H-126,
H-127. / C-7 findings are triggered by operator actions in forms, not by the network;
provisional placement P2 H-109, H-110; P3 H-111–H-117; P4 H-118–H-125; P5 H-126, H-127.

C-5:n havainnoille (H-078–H-086) ei ole osoitettu verkkosaavutettavuutta; ne vaativat
paikallisen tiedoston tai operaattorin syötteen. Alustava sijoitus [päätelty]: P4 H-078,
H-079 (uudelleenarviointi kesken, ks. jatkotehtävät), H-082, H-085; P5 H-080, H-081, H-083,
H-084, H-086. H-042:n laajennus kuuluu H-042:n mukana P3:een. / C-5 findings have no shown
network reachability; provisional placement [inferred] P4 H-078, H-079 (re-assessment
pending), H-082, H-085; P5 the rest; H-042 stays P3.

## Moduulien dokumentointi / Module documentation

| Moduuli / Module | Arkkitehtuuri / Architecture | Koodikommentit / Code comments |
|---|---|---|
| `Hk/` | yleiskuva, päävirta / overview, main flow | – |
| `Juk/` | yleiskuva (päätelty HK:n pohjalta) / overview (inferred from HK) | – |
| `Tp/` | yleiskuva, säikeet, säie- ja lukkokartta (`saikeet-ja-lukot.md`) / overview, threads, thread and lock map | – |
| `com/` | kuljetus (`tiedonsiirto.md`) / transport | – |
| `cbHk/`, `ViestiWin/`, `cbTp/` | logiikkaa sisältävät tunnistettu; kahdeksan liiketoimintalomaketta kuvattu (`gui-liiketoimintalogiikka.md`) / forms with logic identified; eight business-logic forms described | – |
| `tputilv2/`, `include/`, `dbboxm/` | rajapinta mainittu / interface mentioned | – |
| `HkConsole/`, `JukConsole/` | käynnistys / startup | – |

## Avoimet kysymykset / Open questions

| # | Kysymys / Question | Missä / Where |
|---|---|---|
| ~~Q1~~ | **Ratkaistu / resolved 2026-11-24:** työsäie `tarkcom`; samanaikaisuus mahdollinen lukitsemattomien kutsujen kautta / worker thread `tarkcom`; concurrency possible via unlocked callers (`tallenna-samanaikaisuus.md`, H-007, H-010) | – |
| Q65–Q74 | Erän C-6 kysymykset (ei-omistajan `Leave`, lokisoketin virheen pysyvyys, `aloitus` kahdesti, `opencomfile(1)` yhdellä yhteydellä, emit-muistin lukijat vaihdon aikana, `TCPrcvTh` vapautuksen jälkeen, viestin `tark_aika` lopetuksessa, `LAJUNEN`, konsolin `CTRL_CLOSE`, syvemmät lukitusreunat) / batch C-6 questions. **Vaihe 2:** Q65, Q66, Q69, Q70, Q71, Q72, Q74 ratkaistu, Q67 osittain; Q68 ja Q73 avoinna / step 2: resolved except Q67 (partly), Q68, Q73 | `saikeet-ja-lukot.md`, luvut / sections 10, 13.5 |
| Q12 | Kumpi `add_sakko` linkitetään HkKisaWiniin? / Which `add_sakko` is linked? | `cbHk/UnitASakot.cpp:269`, `cbHk/UnitASuunn.cpp:289` |
| ~~Q13~~ | **Ratkaistu / resolved (C-2):** kyllä, kaikki perustiedot paitsi koon 0 kentät; ei vaiheita / yes, all base fields except size-0 fields; no stages (`tiedonsiirto.md`) | – |
| Q14 | Kutsuuko `tulostasarja` lukon aikana `SendMessage`-polkua? **Osittain ratkaistu (C-6):** GUI-polulta ei löytynyt; konsolin kehotteet vapauttavat lukon / Does `tulostasarja` reach a `SendMessage` while holding the lock? **Partly resolved (C-6):** none found on the GUI path | `Hk/HkTulostus.cpp:5850`–`:6419` |
| Q15 | Voiko VCL-poikkeus `tallenna`-polulla jättää lukon päälle? **Osittain ratkaistu (C-7):** kyllä lomakkeiden käsittelijöissä (H-115, H-125, T-036); `tallenna`-polkua itseään ei tutkittu / Can a VCL exception leave the lock held? **Partly resolved (C-7):** yes in form handlers | `tallenna-samanaikaisuus.md`, `gui-liiketoimintalogiikka.md` luku 10 |
| Q75 | `kuntosuunnmoodi`-arvojen (1, 2, 3, 98, 99), `esiluenta`n ja `kuntoLahto`n merkitys / semantics of the fitness modes | `cbHk/UnitEmit.cpp:199-320` |
| Q76 | Päivittääkö viestin ydin joukkuetietueen, kun ajanoton paneelista tallennetaan vain jonorivi? / Does the relay core update the team record after a queue-only save? | `ViestiWin/UnitAjanotto.cpp:657` |
| Q77 | Kumpi HK/viesti-haara on uudempi yksipuolisissa korjauksissa (versiohistoria, T-034)? / Which fork is newer per one-sided fix? | `gui-liiketoimintalogiikka.md` luku 4 |
| ~~Q16~~ | **Ratkaistu / resolved (C-2):** viestin vastaanotto lukitsee ennen lukua, ei H-007:ää; omat viat H-024–H-026 / relay locks before reading, no H-007; own defects H-024–H-026 | – |
| ~~Q17~~, ~~Q18~~ | **Ratkaistu / resolved (C-3):** bcc32 noudattaa `-K`:ta kesken yksikön, tarkistussummat täsmäävät; koot mitattu kaikista käännöksistä / bcc32 honours `-K` mid-unit; sizes measured for all builds | `tiedostomuodot.md` |
| Q19–Q25 | Erän C-2 kysymykset (AIKAT-kaksoiskappaleet, TCP-virta, rengas, `tark_ylahto`, `Kysy`, kirjastot, MONITORI-portti) / batch C-2 questions | `tiedonsiirto.md`, "Avoimet kysymykset" |
| Q26–Q34 | Erän C-3 kysymykset (Q30 ratkaistu C-6:ssa / resolved in C-6: `saikeet-ja-lukot.md` luku 5; HK:n XML-rajat, NTFS-katkos, `initindex`, viestin vienti, vaiheen vaihto, viestin 250-käännös, `t_time_l`, `mbtowc`, konstruktorijärjestys) / batch C-3 questions | `tiedostomuodot.md`, "Avoimet kysymykset" |
| Q35–Q42 | Erän C-11 kysymykset (v143 ja ASan CI:ssä, `LockCount`, H-041 käynnistyksessä, H-001:n sisääntulo, `/we4789` ja LTCG, interaktiivinen CI-istunto, viestin `addtall`-säikeet, HK:n luokkapituudet) / batch C-11 questions | `testaus.md`, "Avoimet kysymykset" |
| Q43–Q56 | Erän C-4 kysymykset (`pkglen`, ARES-kehys, bcc32:n `atoi` ja ASLR, `mrivi`, `RADAT.LST`, GUI:n muotoilumerkkijonot, emiTag-TCP-palvelin konsolissa, FEIG viestissä, `ossijat`-valikot, `EMVALOKI`, `lahriviCPower`, XML-tuonnin paikat, emiTag-kortin vaikutus) / batch C-4 questions | `muistiturvallisuus.md`, "Avoimet kysymykset" |
| Q2 | Mitä kukin ominaisuusvalitsin tekee (`GAZ`, `ORAVA`, `VERTAA`, `MERI5`, `LAJUNEN`, `PARTIO` …)? Miksi konsoli- ja GUI-HK eroavat? / What does each feature switch do? Why do console and GUI HK differ? | `inventaario.md` |
| ~~Q3~~ | **Osittain ratkaistu / partly resolved (C-2):** kehys, kuittaus ja yhteensopivuus kuvattu; tarkat koot avoinna (Q18) / frame, ack and compatibility documented; exact sizes open | `tiedonsiirto.md` |
| ~~Q4~~ | **Ratkaistu / resolved (C-3):** muoto kuvattu tavutasolla; vanhaa muotoa ei tunnisteta automaattisesti, vain GUI-tuonti käyttäjän parametrein / layout documented; no automatic old-format detection, only GUI import with user parameters | `tiedostomuodot.md` |
| Q5 | MySQL-taulujen rakenne ja milloin SQL on käytössä / SQL table layout and when SQL is used | `cbHk/UnitMySQL.cpp` |
| Q6 | Ovatko kirjastoon käännetyt `main`-funktiot (`tputilv2/ctrlhndl.cpp` ym.) esikääntäjällä pois? / Are the `main` functions in the library excluded by the preprocessor? | `inventaario.md` |
| ~~Q7~~ | **Ratkaistu / resolved (C-5):** kyllä, kapeat ja leveät merkkijonot sekä merkkivakiot oikein myös sisällytetyissä otsikoissa; BOMiton UTF-8 on väärin molemmilla kääntäjillä / yes; UTF-8 without BOM is wrong on both compilers | `kaantajaerot.md` luku 5 |
| Q57–Q64 | Erän C-5 kysymykset; Q57–Q59, Q61 ja Q62 ratkaistu vaiheessa 2; avoinna Q60 (`KUNTOSUUNN`-eron tarkoitus, **omistajan päätös**), Q63 (bcc32:n negatiivinen `new`) ja Q64 (`UnitSakkoKierr`in kumoamispolun jäsennys) **avoimina teknisinä kysymyksinä** C-5:n sulkemisen jälkeen / batch C-5 questions; Q60 owner decision, Q63 and Q64 open technical questions after closing C-5 | `kaantajaerot.md`, "Avoimet kysymykset" |
| ~~Q8~~ | **Ratkaistu / resolved (C-5 vaihe 2):** bcc32-objektien OMF-vertailu `tyokalut/vertaa_omf.py` / OMF comparison of bcc32 objects | `kaantajaerot.md` luku 13.1, `merkistot.md` |
| Q9 | Sallitaanko C++11 tuotantokoodissa? / Is C++11 allowed in production code? | `riippuvuudet.md` |
| Q10 | Ovatko orvot tiedostot käytössä jossain (muut haarat, ulkoiset käännökset)? / Are orphan files used anywhere? | `inventaario.md` |
| Q11 | Mihin `Tp/TpComY32.cpp` (vanha kopio) ja `tputilv2/*-2.cpp`, `*-v.cpp` -versiot liittyvät? / What are the old copies for? | `inventaario.md` |

## Epäillyt viat / Suspected defects

Kirjattu issue-luonnoksiksi (paikallinen `issue-drafts/`, ei repossa).
Recorded as issue drafts (local `issue-drafts/`, not in the repo).

| # | Vika / Defect | Vakavuus / Severity | Tila / State |
|---|---|---|---|
| 01 | `memset(stsk, 0, sizeof(st))` ylivuoto / overrun, `Juk/VTulostus.cpp:2228`, `:3981` | muistin korruptio / memory corruption | luonnos / draft |
| 02 | `sscanf("%s")` `int`-kenttään / into `int`, `Juk/vSarjat.cpp:377`, `:400` | muistin korruptio / memory corruption | luonnos / draft |
| 03 | `printf`/`swprintf`-muotoiluvirheet, 16 kohtaa / format mismatches, 16 places | kaatuminen (2) tai väärä teksti / crash (2) or wrong text | luonnos / draft |
| 04 | U+FFFD käyttäjälle näkyvissä teksteissä / in UI strings (`Hk/HkAjat.cpp`, `Juk/vajat.cpp`); UTF-8-teksti konsolissa / UTF-8 text in console (`Tp/TpLaitteet.cpp:2581`) | väärä teksti / wrong text | luonnos / draft |
| 05 | `Tests/run.sh`: puuttuva `\` ja `-I Hk` (merge `6107c8f`); testejä ei ajeta CI:ssä / missing `\` and `-I Hk`; tests not run in CI. `TpTest.sln` itse toimii: 46/46 testiä läpi 2026-11-24 / `TpTest.sln` itself works: 46/46 pass | testit eivät aja automaattisesti / tests not run automatically | luonnos / draft |

## Tekninen velka / Technical debt

- Ydin riippuu GUI-otsikoista (`Hk/HkDeclare.h:23-28`, `Juk/VDeclare.h:26-27`) / core depends on GUI headers
- Kehäriippuvuudet `Hk ↔ Tp`, `Juk ↔ Tp`, `cbTp ↔ cbHk/ViestiWin` / cyclic dependencies
- Noin 850 `extern`-globaalia / about 850 extern globals
- Sekoitetut merkistöt; kaksi sekoitettua tiedostoa / mixed encodings; two mixed files
- Indy- ja ODBC-riippuvuudet ilman käyttöä / Indy and ODBC dependencies without use (`riippuvuudet.md`)
- JukLuenta linkittää vanhaan `vc10/`-hakemistoon / links against old `vc10/`
- 32 orpoa tiedostoa / 32 orphan files (`inventaario.md`)
- Konsoli- ja GUI-versiot eri valitsimilla / console and GUI versions with different switches
- Tiedostoissa ei tunnistetta, versiota eikä tarkistussummaa; ei korjaustyökalua (T-019); pragma-hygienia (T-016); dbboxm:n kuollut indeksikerros (T-018) / no magic, version or checksum; pragma hygiene; dead index layer
- `makitp maki;` luokan ulkopuolella `#ifdef MAKI`-lohkossa (`Hk/HkDef.h`, `kilppvtp`:n jälkeen) – ei käännetä nykyisillä asetuksilla / outside a class in an `#ifdef MAKI` block – not compiled with current settings

## Päätökset / Decisions

| Päivä / Date | Päätös / Decision | Peruste / Rationale |
|---|---|---|
| 2026-11-24 | Arkkitehtuuridokumentit hakemistoon `doc/arkkitehtuuri/` / location | Kehittäjäanalyysien viereen; `docs-site/` on käyttäjille / next to developer analyses; `docs-site/` is for users |
| 2026-11-24 | Dokumentit kaksikielisiä samassa tiedostossa, suomi ensin / bilingual, Finnish first | Suomalaiset ylläpitäjät ja ulkopuoliset lukijat / Finnish maintainers and outside readers |
| 2026-11-24 | Uudet koodikommentit englanniksi ja ASCII:na / new code comments English, ASCII | Merkistöongelmat (`merkistot.md`) / encoding problems |
| 2026-11-24 | Pekan alkuperäiset kommentit säilytetään / Pekka's original comments are kept | Historiallinen tieto / historical knowledge |
| 2026-11-24 | Ensimmäisellä kierroksella ei muuteta `TPsource/`-hakemistoa / no `TPsource/` changes in round 1 | Ensin ymmärrys, sitten muutokset / understanding before changes |
| 2026-11-24 | C-11: testit haaralle `testit/vikojen-todentaminen`, ajuri Pythonilla, uusi todennustaso `toistettu testikehyksessä`; ei tuotantokoodin muutoksia, testikoukkuja eikä CI-muutoksia; testikoodi commitoidaan vain erillisellä hyväksynnällä / tests on their own branch, Python runner, new verification level; no production, hook or CI changes; test code committed only with approval | Käyttäjän ohje / user instruction |
| 2026-11-24 | Syväauditointi erinä riskijärjestyksessä, enintään 3 analyysiagenttia rinnakkain, jokainen erä riippumattomasti tarkastettu / deep audit in risk-ordered batches, max 3 parallel analysis agents, each batch independently reviewed | Käyttäjän ohje / user instruction |
| 2026-11-24 | C-11 vaihe 4: menetelmät A (toimitettu HkMaali.exe debuggerin alla) + C (komponenttitestit), ei B; rajapari 220/221, ei 250; WER vain debuggeritilalla, HKCU-muutos vain erillisellä luvalla; pudotuslokituksen ylläpitäjäkomennot ajaa käyttäjä; esiehdot erillisenä paikallisena committina, ajotuloksia ei commitoida ilman hyväksyntää; H-041 "vahvistettu ajamalla" vain yksiselitteisellä näytöllä / stage 4: methods A + C, no B; pair 220/221; WER via debugger mode only; user runs admin commands; prerequisites committed separately, run results only with approval | Käyttäjän ohje / user instruction |
| 2026-11-24 | C-4 vaihe 1: vain varoitusinventaario ja luokittelu; ei tuotantokoodin, projektiasetusten tai CI:n muutoksia; analyysikäännökset väliaikaishakemistoon MSBuildin `ForceImportAfterCppTargets`-ohjauksella; ReleaseVA200 ajettu `PlatformToolset=v145`-ohituksella; ei committia ilman hyväksyntää / warning inventory and triage only; no production, project or CI changes; analysis builds redirected to scratch; ReleaseVA200 with a toolset override; no commit without approval | Käyttäjän ohje / user instruction |
| 2026-11-25 | C-5 suljettu käyttäjän hyväksynnällä; ei uusia C-5-analyysejä tai -testejä; Q60 omistajalle, Q63–Q64 avoimina; 161 jäsennysehdokasta ja H-079:n uudelleenarviointi jatkotehtäviksi; C-6 valmistellaan mutta ei aloiteta. Työtapa C-6:sta alkaen: Sonnet oletusmallina, Opus vain perustelluissa vaativissa tarkastuksissa, ei päällekkäistä analyysiä, työerälle ennalta rajaus ja tokenbudjetti, olemassa olevat todisteet ennen uusia koeohjelmia, riippumaton tarkastus riskiperusteisesti / C-5 closed; follow-ups recorded; C-6 prepared, not started; working method from C-6: Sonnet by default, Opus only for justified demanding reviews, no overlapping analysis, scope and token budget fixed up front, existing evidence before new probes, risk-based independent review | Käyttäjän päätös; C-5:n agenttien kulutus noin 1,56 milj. tokenia / user decision; C-5 agents used about 1.56 M tokens |
| 2026-11-25 | C-6 vaihe 1 aloitettu ja tehty käyttäjän luvalla; tälle erälle ei keinotekoisia tokenrajoja (suunnitelman 380 k ei sitova), Sonnet inventointiin, Opus vakavien väitteiden riippumattomaan tarkastukseen; paikallinen commit sallittu, ei pushia / C-6 step 1 started and done on approval; no artificial token limits for this batch, Sonnet for inventory, Opus for reviewing severe claims; local commit allowed, no push | Käyttäjän ohje / user instruction |
| 2026-11-25 | C-6 vaihe 2 tehty käyttäjän luvalla: eristetyt koeohjelmat sallittu (ei tuotantotietoja, ei verkkoyhteyksiä, ei toimitettujen ohjelmien käynnistystä); koeohjelmat ja niiden projekti todisteina `todisteet/c6/vaihe2/`, ei muutoksia `TPsource/`-puuhun; ei tokenrajoja; paikallinen commit, ei pushia; C-7 vasta C-6:n sulkemispäätöksen jälkeen / C-6 step 2 done on approval; isolated probes allowed, kept as evidence outside TPsource | Käyttäjän ohje / user instruction |
| 2026-11-25 | C-6 suljettu käyttäjän hyväksynnällä; ei uusia C-6-analyysejä tai -testejä; ajoa vaativat avoimet asiat (bcc32, H-087/H-103 ViestiWinissä, Q67:n apusäikeet, Q68, Q73 ym.) jatkotehtäviksi; korjausjärjestys säilyy (P2 H-087, H-090, H-091, H-093, H-107), korjauksia ei aloiteta; C-7:n vaihe 1 rajataan mutta ei aloiteta / C-6 closed on approval; open items deferred; fix order kept; C-7 step 1 scoped, not started | Käyttäjän päätös / user decision |
| 2026-11-25 | C-7:n vaihe 1 aloitettu käyttäjän luvalla ja tehty rajauksen mukaan; poikkeama: täydentävä B-ajo kattavuusaukkojen vuoksi ja toinen Opus-tarkastuskierros; ei tokenrajaa (noin 1,0 milj.); paikallinen commit, ei pushia / C-7 step 1 started on approval and done as scoped; deviation: supplementary B pass and second review round; about 1.0 M tokens; local commit, no push | Käyttäjän lupa / user approval |
| 2026-11-25 | C-7 suljettu käyttäjän hyväksynnällä kattavuustarkistuksen jälkeen; ei uusia C-7-analyysejä; tekemättä jääneet osat jatkotyöjonoon J7-1–J7-7 (arvonta-algoritmi, 41 muuta lomaketta, 152 jäsennysehdokasta, ajonaikaiset vahvistukset, tarkastamattomat rivit, Q15/Q75–Q77, haarojen korjaussuunnittelu); C-8:n vaihe 1 rajataan mutta ei aloiteta / C-7 closed on approval after the coverage check; gaps to queue J7-1–J7-7; C-8 step 1 scoped, not started | Käyttäjän päätös / user decision |
| 2026-11-25 | C-5 vaihe 1: riippumaton tarkastus ja dokumentointi ilman välihyväksyntöjä, paikallinen commit sallittu, ei pushia; sen jälkeen C-5:n jäljellä olevat tavoitteet rajattuna staattisena työeränä; ei tuotantokoodin, projektiasetusten tai CI:n muutoksia, ei toimitettujen ohjelmien ajoja eikä verkkokokeita, ei uusia laajoja testikehyksiä; epäselvyydet avoimiksi kysymyksiksi / step 1 review and write-up without intermediate approvals, local commit allowed, no push; then the remaining C-5 goals as a bounded static step; no production, project or CI changes, no runs of shipped programs, no network tests, no new large harnesses | Käyttäjän ohje / user instruction |
| 2026-11-25 | C-4 suljettu käyttäjän hyväksynnällä; H-071, H-072, H-055 ja H-070 kiireellisesti korjattaviksi; väliaikaiset suojaukset vain suosituksina, verkko- tai tuotantoasetuksia ei muutettu; C-5 seuraavaksi aloitusluvan jälkeen / C-4 closed on approval; urgent fixes marked; mitigations as recommendations only; C-5 next on approval | Käyttäjän päätös / user decision |
| 2026-11-25 | C-4 vaihe 2 (viimeinen): vain analyysi; ei ASan-sarjoja, uusia testikehyksiä, GUI-ajoja, korjauksia eikä CI-muutoksia; vaihe 1 commitoitiin hyväksynnän ja uudelleentarkastuksen jälkeen (`81bcad9`), vaihetta 2 ei commitoida ilman hyväksyntää; uusi H vain, jos kyse ei ole rekisteröidyn vian toisesta ilmenemismuodosta / analysis only; step 1 committed after approval and re-review; step 2 not committed without approval | Käyttäjän ohje / user instruction |
| 2026-11-24 | H-041: ennalta kiinnitetty arvio "not confirmed" säilyy; käyttäjä hyväksyi dokumentoidun poikkeaman (prosessi 26236 = kontrolliajon jälkikoeohjelma, ei HkMaali.exe:n liikennettä) ja H-041 nostetaan tasolle "vahvistettu ajamalla"; raakatuloksia ja kriteerejä ei muuteta; uutta ajoa ei tehdä; C-11 suljetaan, H-009 ja H-014 jäävät avoimiksi; seuraavaksi C-4 / pre-fixed verdict kept; user-approved deviation (pid 26236); H-041 confirmed by running; C-11 closed; next C-4 | Käyttäjän päätös / user decision |

## Oletukset, joita ei ole todennettu / Unverified assumptions

- Viestin (`Juk/`) päävirta vastaa HK:n virtaa (`yleiskuva.md`) / relay main flow matches HK
- Säikeiden tehtävät on päätelty nimistä ja käynnistyspaikoista; `tallenna`-polut on jäljitetty erässä 1, muut tehtävät eivät / thread tasks inferred from names and start sites; `tallenna` paths traced in batch 1, other tasks not
- Tiedostojen roolit on päätelty nimistä (`riippuvuudet.md`) / file roles inferred from names
- ~~`bcc32` käsittelee kaikki UTF-8-tiedostot oikein~~ **kumottu C-5:ssä / refuted in C-5:** bcc32 lukee BOMittoman UTF-8:n ANSI-koodisivuna kuten MSVC; aiempi todiste oli väärin tulkittu (`kaantajaerot.md` luku 5) / reads UTF-8 without BOM as the ANSI code page like MSVC
