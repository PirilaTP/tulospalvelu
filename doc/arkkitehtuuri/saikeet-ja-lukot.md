# Säikeet ja lukot / Threads and locks (C-6 vaiheet 1–2 / steps 1–2)

Erä C-6, vaihe 1: säiekartta, elinkaari ja lopetus (osat A + D), lukkokartta ja
lukitusjärjestys (osat B + C) sekä riippumaton tarkastus (osa E). Revisio `f7fe553`
(`TPsource/` ennallaan). Polut suhteessa `TPsource/V52/`. Päivä 2026-11-25.

Vaiheen 2 todennus ja jatkoanalyysi: luku 13. / Step 2 verification and follow-up: section 13.

Batch C-6, step 1: thread map, lifecycle and shutdown (parts A + D), lock map and lock
order (parts B + C) and independent review (part E). Revision `f7fe553`. Paths relative
to `TPsource/V52/`.

Merkinnät / tags: [todennettu] = koodi luettu / code read; [päätelty] / [inferred];
[tuntematon] / [unknown]. Kaikki tämän erän tulokset ovat **staattisia**: mitään ohjelmaa
ei ajettu. / All results in this batch are **static**; nothing was run.

## 1. Yhteenveto / Summary

- **Säikeet / threads:** 67 tuotannon käynnistysriviä, joista **56 aktiivista**;
  **29 erillistä säiefunktiota** (41 määrittelyä, kun HK- ja viestikopiot lasketaan
  erikseen). Yhtäkään säiettä ei odoteta (join) eikä lopeteta väkisin; kahvoja ei käytetä.
  Lopetus perustuu lippuihin ja kiinteisiin `Sleep`-viiveisiin. / 67 production start
  lines, 56 active; 29 distinct entry functions. No thread is ever joined or terminated;
  handles are unused; stopping relies on flags and fixed sleeps.
- **Lukot / locks:** **22 erillistä nimeä**, ei 24 (suunnitelman luku sisälsi testien
  `cs`- ja `a`-nimet). Ydinlukot alustetaan `aloitus()`-funktiossa eikä niitä poisteta
  koskaan; poistetaan vain lomake- ja staattiset lukot (`tls`, `seur`, `sakkoKierr`, `etls`,
  `prof`, `tcp[]`). / 22 distinct names, not 24 (the plan counted test-only names). Core
  locks are initialised in `aloitus()` and never deleted.
- **Uudet viat / new defects:** H-087–H-102 (16 kpl): korkea 2, keskitaso–korkea 2,
  keskitaso 5, matala–keskitaso 1, matala 6 (luvut 7–8, `rekisteri/havainnot.md`). Neljä vapauttamatonta tai kahdesti
  vapautettua lukkoa on **vahvistettu lähdekoodivirhe** (deterministinen polku); kolme
  vastakkaisen lukitusjärjestyksen paria on **staattisesti mahdollinen, ajoituksesta
  riippuva lukkiutuminen** kahdella todellisella, samanaikaisella säikeellä ilman yhteistä
  porttilukkoa. Tekninen velka T-030–T-032. / New H-087–H-102; four unreleased or
  double-released locks are confirmed source defects; three lock-order inversions are
  statically possible, timing-dependent deadlocks between two real concurrent threads with
  no gate lock.
- **Q30 ratkaistu / resolved:** HK:n vaiheen vaihto **ei** sulje eikä avaa `KILP.DAT`:ia;
  se sulkee aikatiedostot ja emit-tiedoston ja vapauttaa emit-muistin kolmen lukon alla.
  Kommunikaatiosäikeet pysäytetään nollaamalla `comopen[]`; laitelukijasäikeitä ei
  pysäytetä (luku 5). / HK stage change does not close or reopen KILP.DAT; reader threads
  are not paused.
- **Riippumaton tarkastus / review:** 20 väitettä, kaksi Opus-tarkastajaa; ei yhtään
  kumottua kokonaan, 9 tarkennettiin tai heikennettiin (luku 8). / 20 claims, none fully
  refuted, 9 refined or weakened.

## 2. Menetelmä / Method

| Osa / Part | Tekijä / By | Malli / Model | Tokenit / tokens | Kutsut / calls |
|---|---|---|--:|--:|
| Raakadata ja rakennejäsenyys / raw hits and build membership | pääistunto / main session | – | – | – |
| A + D säiekartta, elinkaari / thread map, lifecycle | `architecture-analyst` | Sonnet | 315 799 | 162 |
| B + C lukkokartta, järjestys / lock map, order | `defect-concurrency-auditor` | Sonnet | 378 168 | 131 |
| E lukkoväitteet / lock claims L1–L10 | `independent-verifier` | Opus | 122 321 | 81 |
| E elinkaariväitteet / lifecycle claims T1–T10 | `independent-verifier` | Opus | 133 585 | 65 |
| **Yhteensä / total** | | | **949 873** | **439** |

Käyttäjä ohjeisti tälle erälle: ei keinotekoisia tokenrajoja, Sonnet inventointiin, Opus
vakavien väitteiden riippumattomaan tarkastukseen. Suunnitelman 380 k:n budjetti ei siksi
ollut sitova. / The user's instruction for this batch: no artificial token limits; Sonnet for
inventory, Opus for reviewing severe claims; the planned 380 k budget was not binding.

Raakaosumat (`_beginthread`, `CreateThread`, `*CriticalSection`) kerättiin `grep`illä ja
merkittiin projektijäsenyydellä (`rekisteri/tiedostot.csv`); tiedostot, jotka eivät kuulu
mihinkään projektiin (`Tp/TpComY32.cpp`, `cbTp/TpApu.cpp`), laskettiin erikseen eikä niitä
analysoitu. Sisäkkäiset lukitukset haettiin kertakäyttöskripteillä yhden kutsutason
syvyydeltä ja jokainen reuna vahvistettiin lukemalla; syvemmät kutsut jäljitettiin käsin
vain mainituissa kohdissa, joten reunalista on **alaraja**. Aineisto: `todisteet/c6/`.

Raw hits were collected with grep and annotated with build membership; unbuilt files were
counted separately and not analysed. Nested acquisitions were found by throwaway scripts one
call level deep and every edge was confirmed by reading; the edge list is a lower bound.

## 3. Säiekartta / Thread map

### 3.1 Pää- ja ohjaussäikeet / Main and control threads

| Säie / Thread | Alku / Entry | Ohjelmat / Programs | Huom. / Notes |
|---|---|---|---|
| VCL-pääsäie / main | `RADStudio10/HkKisaWin.cpp:131`, `RADStudio10/ViestiWin.cpp:124` | HkKisaWin, ViestiWin | Kaikki lomakkeiden käsittelijät; `lopetus1` kutsutaan `FormCloseQuery`stä (`cbHk/WinHk.cpp:793-804`, `ViestiWin/UnitMain.cpp:978-989`) |
| Konsolin pääsilmukka / console main | `HkConsole/HkMain.cpp:178`, `JukConsole/laskenta.cpp:98` | HkMaali; JukMaali520, JukLuenta520 | HK päättyy `_exit(0)` (`Hk/HkInit.cpp:3830`), viesti `exit(0)` (`Juk/vApu.cpp:1035`) |
| Käyttöjärjestelmän ohjaussäie / OS console-control | `HkConsole/HkCons0.cpp:85-96`, `JukConsole/laskent0.cpp:42-53` | konsolit / consoles | `CTRL_CLOSE` ym. ajaa `lopetus()`:n tällä säikeellä samaan aikaan pääsilmukan kanssa / runs `lopetus()` concurrently with the main loop |

### 3.2 Työsäikeet / Worker threads

Ellei toisin mainita: kahvaa ei käytetä, käynnistäjä ei odota, ainoa esto on mainittu
lippu. Lyhenteet: HKW = HkKisaWin, VW = ViestiWin, HKM = HkMaali, JM = JukMaali520,
JL = JukLuenta520. / Unless stated: handle unused, starter does not wait.

| Säie / Entry | Käynnistäjä / Started by | Ohjelmat | Instanssit / Instances | Odottaa / Waits | Päättyy / Ends |
|---|---|---|---|---|---|
| `tiedonsiirto` `Tp/TpComY.cpp:1658` | `opencom` `:2213` ← `initcom` ← `Hk/HkInit.cpp:3659`, `Juk/VInit.cpp:2925` | kaikki / all | 1 / RS-yhteys | sarja-I/O, `Sleep` kun `!comopen` | `comfl==0` (`:1678`) |
| `tiedonsiirtoUDP` `Tp/TpComY.cpp:1799` | `luepakettiUDP` `:953` (säie käynnistää säikeen) | kaikki | 1 / UDP | `Sleep` kun `!comopen` (`:1810`) | `comfl && !ts_close[cn]` (`:1809`) |
| `luepakettiUDP` `Tp/TpComY.cpp:925` | `opencom` `:2217` | kaikki | 1 / UDP | estävä `read_UDP` (`:967`) | `comfl && luepaketteja` (`:956`) |
| `tiedonsiirtoTCP` `Tp/TpComY.cpp:1257` | `opencom` `:2227` | kaikki | 1 / TCP | recv, `Sleep` | `comfl && !inLopetus` (`:1277`) |
| `lue_regnlyThread` `Tp/TpLaitteet.cpp:1747` | `opencom` `Tp/TpComY.cpp:2233`; `openSirit` `Tp/TpLaitteet.cpp:2184` ← `Hk/HkIV.cpp:4190-4197` | kaikki | 1 / lukija | laitteen luku | `comfl`, `regnly_open`, `inLopetus` (`Tp/TpLaitteet.cpp:1803`); `comopen` tarkistetaan vain ennen silmukkaa (`:1800`) |
| `tarkcom` `Hk/HkCom32.cpp:875`, `Juk/VCom.cpp:893` | `initcom` `Tp/TpComY.cpp:2415` (kerran, staattinen lippu) | kaikki | 1 | `Sleep(50)` | `while (comfl)` (`Hk/HkCom32.cpp:880`); ei välitä `comopen`ista |
| `TCPacceptTh` `com/WinUDP.cpp:645`, `TCPrcvTh` `:546`, `UDPstreamrcvTh` `:1065` | `openportTCP` `:778`, `:807`, `:881`; accept `:662`; `openportUDP` `:300` | kaikki | 1 / yhteys; jokainen accept voi aloittaa uuden `TCPrcvTh`:n (H-028) | estävä `accept`/`recv` | `comfl` ja yhteysrakenteen NULL (H-096) |
| `SendCopyThread` `Tp/TpComY.cpp:3124` | `cbHk/UnitComfile.cpp:1037`, `:1051`; `ViestiWin/UnitComfile.cpp:967`; `HkConsole/HkCons0.cpp:1326`; `JukConsole/laskent0.cpp:825` | kaikki | 0..n; lippu asetetaan säikeessä (`:3146`) (H-101) | `Sleep` | `inSendCopy` tai `taustaon==0` |
| `comajanotto` `Tp/TpLaitteet.cpp:2405` | `inittimer` `Hk/HkIV.cpp:4278`, `Juk/VIv.cpp:2944` | kaikki | 1 | estävä `WaitCommEvent` (`Tp/TpLaitteet.cpp:2424`) | `while (taustaon)`, mutta vasta seuraavan CTS-tapahtuman jälkeen |
| `pros_bdg_t` `Hk/HkIV.cpp:790`, `Juk/VIv.cpp:724` | `add_bdg_t` `Hk/HkIV.cpp:831`, `Juk/VIv.cpp:763` (laiska) | kaikki | HK: luodaan uudelleen jokaisen poistumisen jälkeen; viesti: kerran | `Sleep(50)` | HK `taustaon && !vaiheenvaihto && timerfl` (`Hk/HkIV.cpp:796`); poistaa itse lukkonsa ja puskurinsa (H-094) |
| `vahaku` `Hk/HkIV.cpp:3501`, `Juk/VIv.cpp:2370` | `etHaku` `Hk/HkIV.cpp:3582`, `Juk/VIv.cpp:2476` | kaikki | 1 (`inhttpHaku`) | estävä HTTP | palaa itse; lopetus odottaa `inhttpHaku`a (`Hk/HkInit.cpp:3803`) |
| `taustaThr` `Hk/HkIV.cpp:3711`, `Juk/VIv.cpp:2530` | HK `Hk/HkInit.cpp:3708` vain `_CONSOLE`; viesti `Juk/VInit.cpp:2958` | HK: HKM; viesti: VW, JM, JL | 1 | `Sleep(10)` | `while (taustaon)`; `taustasulj` (`Hk/HkIV.cpp:3722`) asetetaan, mutta kukaan ei lue sitä |
| `sendtcpfile` `Hk/HkTulostus.cpp:7031`, `Juk/VTulostus.cpp:7102` | `autofile` `Hk/HkTulostus.cpp:7331`, `Juk/VTulostus.cpp:7314` | kaikki | 1 (lippu) | pitää `tcp_CriticalSection[fileyht]` koko lähetyksen | palaa itse |
| `gazTh` `Hk/HkIV.cpp:3200` | `writegaz` `Hk/HkIV.cpp:3351` | HKM (`GAZ`) | 1 | `Sleep` | palaa itse |
| `file2emitThread` `Juk/VIv.cpp:869` | `Juk/VIv.cpp:892` | VW, JM, JL | 1 | tiedosto | palaa itse |
| `naytatulosTh` `Tp/TpLaitteet.cpp:503`, `naytaluentaTh` `:547` | `:541`, `:572` | kaikki | 1; `naytaluentaTh` ilman estoa | `Sleep` | palaa itse |
| `LukumaaratTh` `cbHk/WinHk.cpp:1829`, `ViestiWin/UnitMain.cpp:535` | `cbHk/WinHk.cpp:371`, `:2136`, `:2218`; `ViestiWin/UnitMain.cpp:272` | HKW, VW | 1, voi tilapäisesti olla 2 | `Sleep(100/300)`, `SendMessage` | `taustaon && lkmThreadOn` |
| `ProsEmitThread` `cbHk/WinHk.cpp:1611`, `ViestiWin/UnitMain.cpp:1313` | `cbHk/WinHk.cpp:379`, `:2143`, `:2225`, `:2293`; `ViestiWin/UnitMain.cpp:278`, `:1991` | HKW, VW | 1, voi tilapäisesti olla 2 | `Sleep`, `SendMessage` | `ProsEmitOn` |
| `DisplTimerThread` `cbHk/UnitEmit.cpp:834`, `ViestiWin/UnitEmit.cpp:687` | `cbHk/UnitEmit.cpp:891`, `:953`; `ViestiWin/UnitEmit.cpp:1010` | HKW, VW | 1 | `Sleep(200)`, `SendMessage` | liput; VW:ssä ei `taustaon`/`vaiheenvaihto` |
| `ScrollThread` `cbHk/TulosUnit.cpp:1884`, `ViestiWin/TulosUnit.cpp:879` | `cbHk/TulosUnit.cpp:2033`, `ViestiWin/TulosUnit.cpp:1018` | HKW, VW | 1 / lomake | `Sleep`, `SendMessage` | HK myös `!vaiheenvaihto` (H-097) |
| `HaeAvoimetTh` `cbHk/UnitSakkoKierr.cpp:256`, `ViestiWin/UnitSakkoKierr.cpp:285` | `FormCreate` `cbHk/UnitSakkoKierr.cpp:170`, `ViestiWin/UnitSakkoKierr.cpp:195` | HKW, VW | 1 / lomake | `Sleep(1000)`, `SendMessage` | lippu; HK myös `!vaiheenvaihto` (H-097) |
| `kirjSQLTh` `cbHk/UnitMySQL.cpp:645`, `ViestiWin/UnitMySQL.cpp:593` | `kirjKilpSQL` `cbHk/UnitMySQL.cpp:752`, `ViestiWin/UnitMySQL.cpp:692` | HKW, VW | 1 (tarkistus ja käynnistys kilpailevat, H-100) | `Sleep` | HK myös `!vaiheenvaihto` (H-014) |
| `haeSQLTh` `cbTp/UnitEmitSQL.cpp:60` | `:76`, `:445` | HKW, VW | 0..n | `Sleep(HakuVali)` | HK myös `!vaiheenvaihto` (H-097) |
| `PaivThread` `cbTp/UnitEmiTag.cpp:213`, `cbTp/UnitMTR.cpp:398` | `cbTp/UnitEmiTag.cpp:252`, `cbTp/UnitMTR.cpp:262` | HKW, VW | 1 (`hPaivThread`) | `Sleep(500)` | `_endthread()`; MTR ei nollaa kahvaa (H-098) |
| `NaytaTulosTimerThread` `cbTp/UnitNaytaTulos.cpp:96` | `:139` | HKW, VW | 1 | `Sleep(200)` | liput |
| `SelTimerThread` `cbTp/UnitSelect3.cpp:105` | `:124` | HKW, VW | 0..n | `Sleep(200)` | palaa itse |

`cbTp/UnitEmitSQL.cpp` ja `cbTp/UnitNaytaTulos.cpp` määrittelevät oman staattisen
`vaiheenvaihto`-muuttujan vain viestin käännöksessä (`#ifdef MAXOSUUSLUKU`); HK käyttää
globaalia. / Those two files define a local `vaiheenvaihto` only in the relay build.

### 3.3 Lukumäärät / Counts

| | Määrä / Count |
|---|--:|
| Tuotannon käynnistysrivit / production start lines | 67 |
| – kommentoitu `CreateThread` (`Hk/HkTulostus.cpp:7330`, `Juk/VTulostus.cpp:7313`) | 2 |
| – `Tp/TpComY32.cpp` (ei missään projektissa / in no project) | 7 |
| – lohkokommentissa / in a block comment (`ViestiWin/UnitMain.cpp:557`) | 1 |
| – ei käänny / does not compile: `luepakettiTCP` (`Tp/TpComY.cpp:2222`; vain `luepakettiTCPSI` on olemassa `SIIMPORT`-ehdon alla, jota mikään projekti ei määrittele / only `luepakettiTCPSI` exists, under an undefined `SIIMPORT`) | 1 |
| **Aktiiviset / active** | **56** |
| Erilliset elävät säiefunktiot / distinct live entry names | 29 (41 määrittelyä / definitions) |
| Aktiiviset käynnistyskohdat ohjelmittain / active sites per program | HKW 40, VW 34, HKM 21, JM 21, JL 21 |

`gazTh` käännetään HkKisaWiniin, mutta `GAZ` ei ole määritelty siellä [päätelty]. `ping_thread`
(`Tp/TpComY.cpp:554`, `#ifdef PING`) ei käynnisty koskaan. / `gazTh` is compiled into HkKisaWin
but `GAZ` is not defined there; `ping_thread` is never started.

## 4. Elinkaari ja lopetus / Lifecycle and shutdown

### 4.1 Yleistä / General

Tuotantokoodissa ei ole `WaitForSingleObject`-, `WaitForMultipleObjects`-, `TerminateThread`-
eikä `SuspendThread`-kutsuja; ne ovat vain `Tests/`-hakemistossa. Säikeiden kahvat vain
tallennetaan muuttujiin; poikkeus on `hPaivThread`, jota käytetään käynnistyksen estona.
Lopetus = lippu + kiinteä `Sleep`. Ainoa todellinen odotus on `while (inhttpHaku) Sleep(200)`
(`Hk/HkInit.cpp:3803`, `Juk/vApu.cpp:1009`). Ydinlukkojen `DeleteCriticalSection`-lohkot ovat
kommenteissa (`Hk/HkInit.cpp:3807-3826`, `Juk/vApu.cpp:1013-1030`), joten globaalien lukkojen
käyttöä poiston jälkeen ei synny. Tämä täydentää H-038:aa (ei odotusta lopetuksessa) ja on
kirjattu tekniseksi velaksi T-030. [todennettu, tarkastettu]

No production code waits for or terminates threads; handles are only stored. Stopping is a flag
plus a fixed sleep. Core-lock deletes are commented out, so globals are never used after delete.
This complements H-038 and is recorded as T-030.

### 4.2 Sulkemisjärjestys / Shutdown sequence

**HkKisaWin:** `FormCloseQuery` (`cbHk/WinHk.cpp:793-804`) → `talljarj()` → `lopetus1`
(`Hk/HkInit.cpp:3730`):
1. `lkmThreadOn=0; inkirjSQLThread=false` (`:3739-3740`); `inLopetus=1`, `Sleep(200)`.
2. `regnly[]=0; ProsEmitOn=false`; `taustaon=0` (`:3752-3756`).
3. `suljeajat(-1,true)` (`:3775` → `Hk/HkAjat.cpp:384-410`) **poistaa `aikajono`-oliot ilman
   `ajat_CS`:ää**, kun `tarkcom` vielä pyörii (H-095).
4. `closecom` (`:3779`): noin 3,5 s `Sleep`ejä + 1 s / TCP-portti; `closec` poistaa
   `tcp_CriticalSection[cn]`:n (`Tp/TpComY.cpp:1898`); `comfl=0` vasta `:1916`.
5. `remtimer` (`:3780`); `closefile(&datf2)` (`:3790`) ilman lukkoa.
6. Lokit, `inhttpHaku`-odotus, `cleanWSA`. Paluu VCL:lle, joka tuhoaa lomakkeet.

**ViestiWin:** `lopetus1` (`Juk/vApu.cpp:944`): liput ensin (`:961-966`), `suljeajat` (`:980`)
ennen `closecom`ia (`:986`) kuten HK:ssa, mutta `closefile(&datf2)` on **`tall_CS`:n sisällä**
(`:991-993`). / Same order as HK, but `datf2` is closed under `tall_CS`.

**Konsolit / consoles:** HkMaali `lopetus1` → `_exit(0)`; JukMaali/JukLuenta → `exit(0)`
(staattiset destruktorit ajetaan säikeiden eläessä / static destructors run while threads live).
`ExitHandler` ajaa `lopetus()`:n ohjaussäikeellä ohittaen `talljarj`in. Jos prosessi tapetaan
ennen `closefile`a, `KILP.DAT` aukeaa silti (otsake kirjoitetaan `addrec`issä heti, tietueet
`FILE_FLAG_WRITE_THROUGH`), mutta viimeaikaisten poistojen vapaalistapaikat ja `maaliajat`-tavut
jäävät vanhoiksi (`dbboxm/dbbox1.cpp:180`, `:195-208`, `:250`, `:262-270`) [päätelty, tarkastettu];
ei rekisteröity viaksi, Q73. / If killed before `closefile`, KILP.DAT still opens; only free-list
slots and `maaliajat` are stale. Not registered; Q73.

Lopetus voi alkaa myös työsäikeestä: `lopetus()` kutsutaan mm. `Hk/HkCom32.cpp:865` (`tarkcom`) ja
`Hk/HkDat.cpp:1379` (indeksointivirhe `tall_CS`:n ollessa varattuna). GUI:ssa `lopetus()` →
`SuljeKaikki` (`cbHk/WinHk.cpp:1781-1787`) on synkroninen `SendMessage` [ehdokas, luku 9].

### 4.3 Kilpailun tai tapahtuman vaihto / Changing the competition

Viestissä ei ole vaiheen vaihtoa. Kilpailun uudelleenavaus samassa prosessissa tapahtuu
tuonnin kautta: tuonti sulkee ja avaa elävän `datf2`:n ilman lukkoa
(`cbHk/UnitImport.cpp:1434-1436`, `ViestiWin/UnitImport.cpp:1113-1116`) – tämä on jo H-047.
Avoin: voiko `aloitus` ajautua kahdesti samassa GUI-prosessissa (`cbHk/WinHk.cpp:279-283`),
jolloin `InitializeCriticalSection` ajettaisiin elävien lukkojen päälle (Q67).
/ Relay has no stage change; reopening via import is H-047; whether `aloitus` can run twice is Q67.

## 5. Q30: HK:n vaiheen vaihto säikeiden ollessa käynnissä / HK stage change with threads running

**Vastaus / answer [todennettu, tarkastettu]:** `vaihdavaihe` (`Hk/HkIx.cpp:1855-2005`) ei sulje
eikä avaa `KILP.DAT`:ia (`datf2`); `kilparr`ia ei varata uudelleen. Kutsujat: GUI:n valikko
(`cbHk/WinHk.cpp:2080-2146`, pääsäie), etäkomento (`Hk/HkCom32.cpp:847` → `pyyda_vaihdavaihe`
→ `SendMessage` vastaanottosäikeestä, `cbHk/WinHk.cpp:2149-2235`) ja konsolin näppäimistö /
`tarkcom` (`HkConsole/HkCons0.cpp:869-871`, `:933`).

Järjestys / sequence:
1. GUI: `vaiheenvaihto=true; lkmThreadOn=0; ProsEmitOn=false; Sleep(500)`; lomakkeet poistetaan
   (`cbHk/WinHk.cpp:2080-2115`). Konsoli: evätty vaihto jättää lipun päälle (H-031).
2. `comopen[]` kopioidaan ja nollataan **ennen lukkoja** (`Hk/HkIx.cpp:1886-1887`):
   kommunikaatiosäikeet tyhjäkäyvät seuraavalla kierroksellaan.
3. `tall → ajat → emit` (`:1897-1899`, vapautus `:1984-1986`):
   `suljeajat(-1,false)`; jos `emitfl>0`: `vapautaemitmuisti` (`Hk/HkEmit.cpp:6365-6401`) ja
   `fclose(femitfile)` (`Hk/HkIx.cpp:1909-1911`); `luesarjat`, `teejarj(true)` (uusintahaara
   `:1603-1613`: `idindex->ix` nollataan `:1610`, nimihakemisto rakennetaan uudelleen lukemalla
   tietueet); emit avataan uudelleen vain ehdoilla (`:1936-1946`); `initajat`; `opencomfile(1)`
   vasta toisesta `levypusk`-yhteydestä alkaen (`:1974-1981`; yhdellä yhteydellä uutta
   vaihekohtaista tiedostoa ei avata, Q68); `comopen` palautetaan (`:1983`).
4. GUI käynnistää uudet `LukumaaratTh`- ja `ProsEmitThread`-säikeet ja nollaa lipun.

| Säie / Thread | Tila vaihdon aikana / State during the change |
|---|---|
| `tiedonsiirto*`, `luepakettiUDP` | tyhjäkäynti `comopen`in vuoksi; korkeintaan yksi kesken oleva kierros / idle via `comopen` |
| `tarkcom` | GUI-etäkomennossa estynyt `SendMessage`en; muuten jatkaa ja jää odottamaan lukkoja / blocked or waits on the locks |
| `lue_regnlyThread` | **ei pysähdy**; kutsuu `add_bdg_t`:tä ja `tall_regnly`ä (jälkimmäinen odottaa lukkoja) / **not paused** (H-094) |
| `comajanotto` | ei pysähdy; `tee_aika` → `getpos` ilman `tall_CS`:ää (H-102) |
| `taustaThr`, `LukumaaratTh`, `ProsEmitThread` | ohittavat työn lipun vuoksi / skip on flag |
| `ScrollThread`, `HaeAvoimetTh`, `haeSQLTh`, `kirjSQLTh` | **poistuvat eivätkä käynnisty uudelleen** (H-097, H-014) / exit, not restarted |
| `pros_bdg_t` | poistuu ja vapauttaa jononsa ja lukkonsa; luodaan uudelleen seuraavasta merkistä (H-094) |
| `vahaku`, `TCPrcvTh`, `UDPstreamrcvTh`, `sendtcpfile`, `SendCopyThread` | omat puskurit ja tiedostot; ei havaittua ristikkäistä käyttöä [päätelty] |

## 6. Lukkokartta / Lock map

"Enter" = elävien `EnterCriticalSection`-kohtien määrä käännetyssä koodissa (kommentit
poistettu, `#if`-lohkoja ei arvioitu). `tall` on analysoitu C-1:ssä
(`tallenna-samanaikaisuus.md`), tässä vain lukitusjärjestyksen solmuna.

| Lukko / Lock | Oliot / Objects | Alustus / Init | Poisto / Delete | Suojaa / Protects [päätelty] | Enter | Ohjelmat |
|---|---|---|---|---|--:|---|
| `tall` | 1 globaali / global | `Hk/HkInit.cpp:3100`, `Juk/VInit.cpp:2557` | ei koskaan / never | kilpailijatietueet, tulokset | 263 | kaikki |
| `outb` | 1 | `Hk/HkInit.cpp:3103`, `Juk/VInit.cpp:2560` | ei | lähtevä jono, `COMFILE.DAT` | 41 | kaikki |
| `inb` | 1 | `Hk/HkInit.cpp:3102`, `Juk/VInit.cpp:2559` | ei | saapuva rengas | 7 | kaikki |
| `tcpip` | 1 (kaikille soketeille / all sockets) | `Hk/HkInit.cpp:3108`, `Juk/VInit.cpp:2565` | ei | `TCPyhteys[]`, `UDPyhteys[]`, soketit | 11 | kaikki |
| `tcp[MAX_PORTTI]` | 1 taulukko | `Tp/TpComY.cpp:1224`, `:1947`; `Hk/HkTulostus.cpp:7016`; `Juk/VTulostus.cpp:7087` | `Tp/TpComY.cpp:1898` | TCP-yhteyden kirjoitukset | 6 | kaikki |
| `kilpdat` | 1 | `Hk/HkInit.cpp:3104`, `Juk/VInit.cpp:2561` | ei | `kilparr`, `KILP.DAT` | 11 | kaikki |
| `ajat` | 1 | `Hk/HkInit.cpp:3105`, `Juk/VInit.cpp:2562` | ei | aikajonot | 5 | kaikki |
| `emit` | 1 | `Hk/HkInit.cpp:3106`, `Juk/VInit.cpp:2563` | ei | emit-tiedosto ja -indeksi | 18 | kaikki |
| `rastiva` | 1 | `Hk/HkInit.cpp:3107`, `Juk/VInit.cpp:2564` | ei | väliaika-analyysi | 11 | kaikki |
| `print` | 1 | `Hk/HkInit.cpp:3206`, `Juk/VInit.cpp:2632` | ei | tulostin ja listat | 30 | kaikki |
| `autotul` | 1 | `Hk/HkInit.cpp:3207`, `Juk/VInit.cpp:2566` | ei | automaattitulostus ja -tiedosto | 4 | kaikki |
| `loki` | 1 | `Hk/HkInit.cpp:3101`, `Juk/VInit.cpp:2558` | ei | loki | 2 | kaikki |
| `idindex` | 1 (HK) | `Hk/HkInit.cpp:3099` | ei | id → tietue (`getpos`) | 9 | HKW, HKM |
| `seura` | 1 (HK) | `Hk/HkInit.cpp:3109` | ei | seurataulukko | 5 | HKW, HKM |
| `seur_badge` | 1 (HK) | `Hk/HkInit.cpp:3110` | ei | vapaan emit-kortin haku | 1 | HKW, HKM |
| `prof` | 1 (viesti, `PROFILOI`) | `Tp/TpComY.cpp:204` | `Tp/TpComY.cpp:234` | profiilirengas | 1 | VW, JM, JL |
| `etls` | 1 staattinen / ohjelma | `Hk/HkIV.cpp:830`, `Juk/VIv.cpp:762` (laiska) | `Hk/HkIV.cpp:747`, `Juk/VIv.cpp:686` (kuluttajasäie itse) | merkkitapahtumien rengas | 4 | kaikki |
| `SQL` | 1 staattinen / GUI | `cbHk/UnitMySQL.cpp:661`, `ViestiWin/UnitMySQL.cpp:608` (työsäikeessä) | ei | vain `SQLjonoVarm.dat` (jonon indeksit eivät) | 6 | HKW, VW |
| `sakkoGrid` | 3 staattista (`cbHk/UnitASakot.cpp:27`, `cbHk/UnitASuunn.cpp:27`, `ViestiWin/UnitASakot.cpp:28`) | lomake | ei | sakkolomakkeen taulukko | 5 | HKW, VW |
| `sakkoKierrCriticalSection` | 1 / GUI | `cbHk/UnitSakkoKierr.cpp:171` | `cbHk/UnitSakkoKierr.cpp:180` | sakkokierroslistat | 2 | HKW, VW |
| `tls` | lomakkeen jäsen / form member (`cbHk/TulosUnit.h:334`, `ViestiWin/TulosUnit.h:186`) | konstruktori (`cbHk/TulosUnit.cpp:72`) | destruktori (`cbHk/TulosUnit.cpp:85`, `ViestiWin/TulosUnit.cpp:51`) | `paivitysflag` | 2 | HKW, VW |
| `seur` | lomakkeen jäsen (`ViestiWin/UnitSeurGr.h:62`) | vain `#ifdef EMIT` (`ViestiWin/UnitSeurGr.cpp:40`) | ehdoitta (`ViestiWin/UnitSeurGr.cpp:54`) | `paivitysflag` | 2 | VW |

Lukko-oliot ohjelmittain (staattiset; `tcp[]` yhtenä) / lock objects per program: HkKisaWin 20
(+ `tls` / tilannelomake), HkMaali 16, ViestiWin 17 (+ `tls`, `seur` / lomake), JukMaali520 ja
JukLuenta520 14. `tietorakenteet-ja-tila.md` luetteli aiemmin 20 nimeä (puuttuivat `etls`,
`seur`); korjattu. Lukemattomia kuolleita osumia: `Tp/TpComY32.cpp` 22, `cbTp/TpApu.cpp` 8.

## 7. Lukitusjärjestys / Lock order

### 7.1 Reunat / Edges (A → B: B otetaan A:n ollessa varattuna / B taken while A held)

| Reuna / Edge | HK | Viesti / Relay |
|---|---|---|
| autotul → print → tall | `Hk/HkTulostus.cpp:985`, `:1004`, `:5850` | `Juk/VTulostus.cpp:704`, `:723`, `:730` |
| print → tall | `Hk/HkTulostus.cpp:6628-6680`; `Hk/HkEmit.cpp:5165-5206`; `cbHk/HkKisaLahtLuet.cpp:584`, `:904`, `:381` | `Juk/VEmit.cpp:5771-5850` |
| print → emit, print → rastiva | `Hk/HkEmit.cpp:4355`, `:4385`, `:4387` | `Juk/VEmit.cpp:5018`, `:5060`, `:5062` |
| emit → print | `Hk/HkEmit.cpp:4279-4280`; `:1665` → `:1684` → `:4576` | `Juk/VEmit.cpp:4923-4924`; `:1510` → `:1536` |
| emit → rastiva, emit → outb | `Hk/HkEmit.cpp:6405-6406`, `:1687`, `:1681` | `Juk/VEmit.cpp:1524`, `:1526` |
| tall → ajat → emit | `Hk/HkIx.cpp:1897-1899` | `Juk/VIv.cpp:642` (tall → ajat) |
| tall → emit, tall → outb, tall → seur_badge | `Hk/HkEmit.cpp:5367-5368`, `:1183`, `:1178`; `Hk/HkCom32.cpp:1184` | `Juk/vdat.cpp:657`, `:775` |
| tall → print (ehdollinen / cfg) | `Hk/HkDat.cpp:1373` → `:784` → `Hk/HkTulostus.cpp:932` | `Juk/VTulostus.cpp:6244` → `Juk/VEmitHtml.cpp:208` |
| ajat → tall (ehdollinen) | `Hk/HkIV.cpp:2674` → `:2963`, `:2999` | ei löytynyt / none |
| ajat → outb, ajat → loki | `Hk/HkAjat.cpp:286-287`, `:156` | `Juk/vajat.cpp:247`, `:130` |
| outb → kilpdat; idindex → kilpdat; sakkoKierr → idindex → kilpdat | `Hk/HkCom32.cpp:200`; `Hk/HkIx.cpp:318`; `cbHk/UnitSakkoKierr.cpp:65-74` | `GETREC` |
| kilpdat → SQL (cfg) | `Hk/HkDat.cpp:615` → `cbHk/UnitMySQL.cpp:741` | `ViestiWin/UnitMySQL.cpp:655` |
| loki → tcpip | `Hk/HkApu.cpp:560` → `:576`, `:583` → `com/WinUDP.cpp:907` | `Juk/vApu.cpp:94-119` |
| tcpip → loki | `com/WinUDP.cpp:915`, `:923` → `:175-183` | sama / same |
| tcp[cn] → tcpip | `Hk/HkCom32.cpp:1049-1050`; `Hk/HkTulostus.cpp:7042`, `:7046` | `Juk/VCom.cpp:1044`; `Juk/VTulostus.cpp:7113-7117` |
| inb → outb → tcpip (vain käynnistys / startup) | `Hk/HkInit.cpp:3650-3668` | `Juk/VInit.cpp:2921-2925` |

Lehtilukot (eivät ota muita) / leaf locks: `kilpdat` (paitsi SQL), `idindex` (paitsi kilpdat),
`seura`, `seur_badge`, `prof`, `sakkoGrid`, `tls`, `seur`, `etls`, `SQL`. Graafi on syklitön
lukuun ottamatta kohdan 7.2 pareja. / The graph is acyclic apart from the pairs in 7.2.

### 7.2 Syklit / Cycles

| Pari / Pair | Säikeet / Threads | Ehto / Condition | Luokka / Class | Tulos / Result |
|---|---|---|---|---|
| tall ↔ print | lukijasäie tai mikä tahansa `tallenna`n V/E → '-' -muutoksen tallentava säie vs. GUI-pääsäie (`lahtoluettelo` "vain läsnä olevat") tai `LukumaaratTh` (`autotulostus`) | HkKisaWin, "Jatkuva sivutuloste" päällä, GDI-tulostin | staattisesti mahdollinen, ajoituksesta riippuva / static, timing-dependent | **H-091**, vahvistettu tarkastuksessa |
| print ↔ emit | `tarkcom` (`tark_emit` → `addem` → `emit_rap`) vs. pääsäie (`tulostaRastiVa` → `getem` → `lueem`) | emit verkosta, `comautorap`, `emittulparam`, samanaikainen väliaikatuloste | sama | **H-092** |
| loki ↔ tcpip | mikä tahansa lokia kirjoittava säie vs. TCP-lähetyksen virheen lokittava säie | LOKI TCP:n kautta (`Tp/TpComY.cpp:2254`) | sama | **H-093** (+ rajaton rekursio) |
| tall ↔ ajat | pääsäie `vaihdavaihe` vs. lukijasäie `tall_regnly` | `TUNNJONO` + `lahdepistehaku` + `lahtoryhma > 1` | ehdokas / candidate, hyvin epätodennäköinen | ei rekisteröity (luku 9) |
| print → tall → emit → print | – | – | ehdokas | ei rekisteröity |
| viestin tall ↔ ajat, inb ↔ outb | – | – | **kumottu / refuted**: käänteistä reunaa ei ole tai vain käynnistyksessä | – |
| viestin emit ↔ rastiva | pääsäie `uusiemitanalyysi` vs. `tarkcom`/lukija `addem` | ViestiWin | vaiheessa 1 virheellisesti kumottu; **korjattu vaiheessa 2** / wrongly refuted in step 1, corrected in step 2 | **H-103** (luku 13) |

Pelkkä vastakkainen järjestys ei riittänyt: jokaisesta hyväksytystä parista tarkastaja osoitti
kaksi eri säiettä samassa ohjelmassa, jotka voivat ajaa polut yhtä aikaa, eikä yhteistä
porttilukkoa ole. Yksikään ei ole deterministinen; kaikki vaativat asetuksen ja ajoituksen.
/ For each accepted pair the reviewer showed two distinct threads in one program that can run
the paths concurrently with no common gate lock. None is deterministic.

### 7.3 Estävät kutsut lukon hallussa / Blocking under a lock (uudet kohdat H-009:n ja H-021:n lisäksi)

| Malli / Pattern | Esimerkki / Example | Lukko | Jumi vai viive / Hang or latency |
|---|---|---|---|
| Estävät soketti- ja nimipalvelukutsut yhden globaalin lukon alla / blocking socket and DNS calls under one global lock | `com/WinUDP.cpp:714-821` (`connect` `:773`, DNS `:756`), `:378`, `select` 10 s `:911`, `Sleep(1000)` `:343`; käynnistyksessä `inb`+`outb` koko `initcom`in ajan (`Hk/HkInit.cpp:3650-3668`) | `tcpip` (+ `loki`, `inb`, `outb`) | viive, rajattu; kaikki verkko- ja lokikirjoitukset pysähtyvät (T-031) |
| `writeerror_w`/`SendMessage` työsäikeessä lukon alla / worker waits for the UI under a lock | `Tp/TpComY.cpp:1133`, `:1471`, `:1602`; `Hk/HkAjat.cpp:217`, `:234`; `Hk/HkIV.cpp:2779`, `:2856`; `Juk/vajat.cpp:177`; `Juk/VEmit.cpp:1460` | `outb`, `ajat`, `emit` | jumi mahdollinen kuten H-009, vain virhepoluilla (T-032) |
| Konsolin kehotteet lukon alla / console prompts | `Hk/HkTulostus.cpp:1029-1031`; `Hk/HkEmit.cpp:5165-5255`; `Juk/VTulostus.cpp:742` | `autotul`, `print` | jumi, kunnes käyttäjä painaa näppäintä (T-032) |
| MySQL-kysely ja `Sleep` `tall`in alla / SQL query under `tall` | `cbHk/UnitExport.cpp:426-450`; `ViestiWin/UnitExport.cpp:411-435` | `tall` | viive koko viennin ajan (T-032) |
| Ulkoinen komento / external command | `Hk/HkTulostus.cpp:7345`; `Juk/VTulostus.cpp:7329` | `autotul` | viive (T-032) |

Q14 (`tulostasarja` ja `SendMessage`) osittain ratkaistu: GUI-polulta ei löytynyt `SendMessage`a
(kuvioihin perustuva haku); konsolin kehotteet vapauttavat `tall`in `readkbd`in ajaksi.

## 8. Riippumaton tarkastus / Independent review

| Väite / Claim | Tulos / Verdict | Muutos / Change |
|---|---|---|
| L1 `EmitHtmlOsuus` vuotaa `print`/`rastiva` | vahvistettu / confirmed | polku käyttäjän toiminnosta todennettu; automaattiset polut eivät tavoita; jäätyminen `tarkcom`in kautta → H-087 |
| L2 `UnitHylkRap` | vahvistettu | lisäksi `Leave` ilman `Enter`iä, kun `lstf != NULL` → H-088 |
| L3 `tarkraportit` kahdesti `Leave` | osittain / partly | vain konsoli (GUI:n `case 2` kuollut) → H-089 |
| L4 `tall_regnly` jättää `ajat`in | vahvistettu, laajempi | `Leave` on A–F-lohkon sisällä → H-090 |
| L5 tall ↔ print | vahvistettu | laukaisija laajempi (mikä tahansa V/E → '-' -tallennus) → H-091 |
| L6 print ↔ emit | vahvistettu | `lueem` ohittaa lukon vain `!ProsEmitOn` → H-092 |
| L7 loki ↔ tcpip | vahvistettu | rekursion rajattomuus ei ratkea staattisesti (Q66) → H-093 |
| L8 `aloitus` jättää `inb`in | vahvistettu; **kumottu vaiheessa 2** / refuted in step 2 | `initcom` palauttaa aina 0, haara kuollut (luku 13) → H-099 |
| L9 SQL-jono | osittain | uudelleenalustus ei koske elävää lukkoa (heikennetty) → H-100 |
| L10 lukkomäärä | osittain | "ei koskaan poisteta" pätee vain ydinlukkoihin |
| T1 ei odotusta | vahvistettu | päällekkäinen H-038:n kanssa → T-030, ei uutta H:ta |
| T2 Q30 | vahvistettu, korjauksin | `opencomfile(1)` vain toisesta yhteydestä alkaen; kaksi GUI-polkua |
| T3 `etls` | vahvistettu, pahempi | koko GUI-ikkunan ajan jokainen merkki luo ja purkaa jonon; konsolissa pysyvä H-031:n jälkeen → H-094 |
| T4 `TCPacceptTh` | osittain | **kaksoisvapautus liioiteltu**; jäljelle kapea käyttö vapautuksen jälkeen → matala, H-096 |
| T5 sulkemisjärjestys | osittain | konkreettinen polku `tarkcom` → `tall_rivi` (`Hk/HkAjat.cpp:126`) → H-095; `datf2` ilman lukkoa = H-038 |
| T6 säikeet eivät käynnisty uudelleen | vahvistettu | → H-097 |
| T7 `UnitMTR` | vahvistettu | toiminnallinen vaikutus: toinen tiedostoon luenta pysähtyy → H-098 |
| T8 vaihdon ikkuna | osittain | ei vapautettua muistia; vain väärä tai puuttuva paikka; `obfile`-päällekkäisyys ajoituksella poissuljettu → H-102 |
| T9 `SendCopyThread` | osittain | laukaisu etäviestistä; `BtnLahValClick` työsäikeessä; `UnitEmitSQL`-kohta heikko (ei-visuaalinen) → H-101, H-018 |
| T10 konsolin sulku | osittain / ei ratkea | `KILP.DAT`-seuraus lievä → ei rekisteröity, Q73 |

Kumotut tai heikennetyt väitteet / refuted or weakened: T4:n kaksoisvapautus; L9:n "elävän
lukon uudelleenalustus"; L3:n GUI-saavutettavuus; T10:n `KILP.DAT`-korruptio; T9:n
`UnitEmitSQL.cpp:63` VCL-käyttönä; sykliehdokkaat viestin tall ↔ ajat, emit ↔ rastiva ja
inb ↔ outb. Suunnitelman "24 lukkoa" ja "76 käynnistyskohtaa" (sisälsivät testikoodin).

## 9. Todentamattomat riskiehdokkaat / Unverified risk candidates (ei rekisteröity / not registered)

| Ehdokas / Candidate | Näyttö / Evidence | Mikä ratkaisisi / What would decide |
|---|---|---|
| `LukumaaratTh`/`ProsEmitThread` kahdentuvat, kun liput käännetään nopeasti / duplicate threads | `cbHk/WinHk.cpp:2081-2082`, `:2135-2138`, `:2288-2294` | ajo; vaikutus todennäköisesti vain ylimääräinen kysely |
| Lomakkeen ja säikeen elinikä: `SendMessage` poistettuun lomakkeeseen / form deleted while a thread sends | `cbHk/UnitSakkoKierr.cpp:174-182`, `:259-260`; `cbHk/UnitEmit.cpp:840`; `cbHk/TulosUnit.cpp:1889-1895`; `cbTp/UnitSelect3.cpp:113-115` | ajo lomakkeen sulkemisella; `select3`-elinikä (`cbTp/cbTpApu.cpp:270`) |
| `openSirit`in staattinen `acn` kahdelle lukijalle / shared static argument | `Tp/TpLaitteet.cpp:2179-2184`, `:1754` | `LAJUNEN`-määrittely projekteissa (Q72) |
| `closeportUDP` vapauttaa ilman lukkoa, `UDPstreamrcvTh` ilman NULL-tarkistusta | `com/WinUDP.cpp:191-205`, `:1078` | `read_UDP_0`:n lukeminen |
| `closec` poistaa `tcp_CS[cn]`:n `sendtcpfile`n ollessa käynnissä; `init[cn]` jää 1:ksi | `Tp/TpComY.cpp:1898`, `:1223-1226`; `Hk/HkTulostus.cpp:7042-7061` | uudelleenavauspolkua ei löytynyt |
| `tcp_CS[fileyht]` alustetaan joka kierroksella, ohitetaan `kysy`-tilassa | `Hk/HkTulostus.cpp:7012-7016` | lukeminen |
| `lopetus()` `tall_CS`:n sisältä työsäikeessä → GUI:n `SendMessage` | `Hk/HkDat.cpp:1379`; `cbHk/WinHk.cpp:1781-1787` | ottaako `talljarj` `tall`in |
| `close_profile` vs. `put_profile` | `Tp/TpComY.cpp:210-243` | vain diagnostiikka |
| `vapautaemitmuisti` vapauttaa `rastiva`n ilman `rastiva_CS`:ää; vanha emit-kahva, jos uudelleenavaus ohitetaan | `Hk/HkEmit.cpp:6397-6399`; `Hk/HkIx.cpp:1909-1944`; `Hk/HkEmit.cpp:1580`, `:1607` | Q69 |
| tall ↔ ajat (HK) ja print → tall → emit → print | luku 7.2 | ajo tai syvempi jäljitys |
| Viestin `aloitus`-vastine `inb`in osalta | `Juk/VInit.cpp:2921-2928` | lukeminen |

## 10. Avoimet kysymykset / Open questions

| # | Kysymys / Question |
|---|---|
| Q65 | Mitä `LeaveCriticalSection` ei-omistajalta tekee kohdejärjestelmässä (H-089, H-088)? Application Verifier (Locks). / Effect of a non-owner Leave |
| Q66 | Pysyykö lokisoketin virhe (rajaton rekursio, H-093) vai muuttuuko se 10054:ksi tai aikakatkaisuksi? / Does the log-socket error persist? |
| Q67 | Voiko `aloitus` ajautua kahdesti samassa GUI-prosessissa (`cbHk/WinHk.cpp:252`, `:279-283`) ja alustaa elävät lukot uudelleen? / Can `aloitus` run twice? |
| Q68 | Onko tarkoituksellista, ettei `opencomfile(1)` avaa uuden vaiheen `COMFILE`a yhdellä yhteydellä (`Hk/HkIx.cpp:1974-1981`)? / Single-connection skip intended? |
| Q69 | Käyttävätkö lukijapolut `rata`a, `kuvio`ta tai `getpos`ia ilman lukkoa `vapautaemitmuisti`n aikana? / Reader use of emit memory during the free |
| Q70 | Käyttääkö `TCPrcvTh` yhteysrakennetta accept-säikeen vapautuksen jälkeen (H-096)? |
| Q71 | Kaatuuko viestin `tark_aika` samoin sulkemisessa (H-095:n viestivastine)? |
| Q72 | Onko `LAJUNEN` määritelty HkKisaWinissä, HkMaalissa tai ViestiWinissä? |
| Q73 | Konsolin `CTRL_CLOSE`-aikaraja kohdejärjestelmässä ja `maaliajat`-tavujen käyttö uudelleenkäynnistyksessä |
| Q74 | Syvemmät kutsut `tall`in alla (`TarkKilpailijat`, `tark_partio` `Juk/VEmit.cpp:1532`): puuttuuko reunoja? / deeper callees |

## 11. Kattavuus / Coverage

- Säiekartta kattaa kaikki 67 tuotannon käynnistysriviä (56 aktiivista; loput syineen).
  `lue_regnlyThread`in pääsilmukka luettiin vain lopetusehdon osalta.
- Lukkokartta kattaa kaikki 22 nimeä ja niiden alustus- ja poistokohdat. `tall`in 263 kohtaa
  vain C-1:n perusteella. Reunalista yhden kutsutason syvyydeltä + käsin jäljitetyt ketjut
  (alaraja, Q74).
- Riippumattomasti tarkastettu 20 väitettä (kaikki uudet korkeat ja keskitason väitteet ja
  lukkiutumisehdokkaat). Matalat ehdokkaat (luku 9) tarkastamatta.
- Ei ajoja, ei koeohjelmia, ei tuotantokoodin muutoksia. Ajonaikaista todennusta varten
  tarkastajat ehdottivat Application Verifieriä (Locks, Heaps) ja lukitusjärjestyksen lokitusta.

Coverage: all 67 start lines and all 22 lock names mapped; edge list one call level deep plus
hand-traced chains (a lower bound); 20 claims independently reviewed; low candidates not
reviewed; nothing run.

## 12. Seuraava vaihe / Next step

**Vaihe 2 tehty, ks. luku 13.** / Step 2 done, see section 13.

Suunnitelman mukaan vaihe 2 (ehdokkaiden syväanalyysi) erillisellä päätöksellä. Ehdotus:
(1) korkeat H-087 ja H-091 sekä H-090 ja H-094 testikehyksessä (C-11:n `TpCore*`-mallilla:
lukon omistaja ja `RecursionCount`, kahden säikeen lukitusjärjestys), (2) Q65–Q67, Q69, Q74,
(3) luvun 9 ehdokkaat, (4) VCL-säieturvallisuus kokonaisuutena H-018:n jatkona. Ei aloitettu.

Per the plan, step 2 (deep analysis of candidates) on a separate decision. Proposal: harness
tests for H-087, H-091, H-090, H-094; Q65–Q67, Q69, Q74; the section 9 candidates; VCL thread
safety as an extension of H-018. Not started.

## 13. Vaihe 2: todennus ja jatkoanalyysi / Step 2: verification and follow-up (2026-11-25)

Käyttäjän hyväksymä vaihe 2: viiden havainnon (H-087, H-090, H-091, H-093, H-094) tarkat
suorituspolut ja todennus, avoimet kysymykset Q65–Q67, Q69, Q74 (ja sivutuotteena Q70–Q72),
luvun 9 ehdokkaat sekä VCL-säieturvallisuus H-018:n jatkona. Revisio `bf5b566`
(`TPsource/` ennallaan revisiosta `f7fe553`). Tuotantokoodia, projektiasetuksia tai CI:tä ei
muutettu; toimitettuja ohjelmia ei käynnistetty; verkkoyhteyksiä ei muodostettu.

Step 2 (approved by the user): exact paths and verification of H-087, H-090, H-091, H-093,
H-094; Q65–Q67, Q69, Q74; the section 9 candidates; VCL thread safety. No production code,
project settings or CI changed; no shipped program started; no network connections.

### 13.1 Menetelmä / Method

| Osa / Part | Tekijä / By | Malli / Model | Tokenit / tokens | Kutsut / calls |
|---|---|---|--:|--:|
| Viiden havainnon polut, Q66, Q69, Q74 / five findings, Q66, Q69, Q74 | `defect-concurrency-auditor` | Opus | 226 299 | 120 |
| Luvun 9 ehdokkaat, Q67, Q70–Q72 / section 9, Q67, Q70–Q72 | `defect-concurrency-auditor` | Sonnet | 169 166 | 68 |
| VCL-säieturvallisuus / VCL thread safety | `architecture-analyst` | Sonnet | 145 673 | 65 |
| Eristetyt koeohjelmat / isolated probes (P1–P5) | `general-purpose` | Opus | 259 871 | 88 |
| Riippumaton tarkastus, väitteet C1–C11 / review of claims | `independent-verifier` | Opus | 98 458 | 70 |
| Riippumaton tarkastus, koeohjelmat / review of the probes | `independent-verifier` | Opus | 116 123 | 42 |
| Q65-koe, projektipohja, ajuri, dokumentointi / Q65 probe, project, runner, write-up | pääistunto / main session | – | – | – |

**Eristetyt koeohjelmat / isolated probes** (`todisteet/c6/vaihe2/koeohjelmat/`):
- `q65_leave.c`: puhdas Win32-koe ilman tuotantokoodia / pure Win32, no production code.
- `C6ProbeHk.vcxproj` (tuotettu `tee_projektit.py`:llä): kopio `VS/Tests/TpCoreHk.vcxproj`:sta eli
  HkMaalin tuotantolähteet ja määrittelyt (MSVC, `_CONSOLE`) + testikehyksen `CoreMain.cpp` ja
  `fixtures/` + `c6_hk_probes.cpp`. Rakennus repositorion ulkopuolelle (`rakenna.cmd`), ajo
  `aja.py`:llä samalla hiekkalaatikkoprotokollalla kuin `run_core_tests.py` (oma prosessi,
  `.tp_sandbox`, aikaraja). Olemassa olevia testitiedostoja tai projekteja ei muutettu.
  / a copy of the TpCoreHk project (HkMaali's sources and defines) plus the probe file, built
  outside the repository and run one process per case in a sandbox.
- Viestin tutkitut viat (H-087, H-103) ovat GUI-koodissa (`#ifndef _CONSOLE`), joten niitä ei voi
  linkittää konsolikoeohjelmaan; niiden näyttö on lähdekoodi. / The relay findings are GUI-only
  and cannot be linked into a console probe.

**Tulkinta / interpretation:** eristetyn koeohjelman tulos osoittaa mekanismin *tuotantokoodissa
MSVC:llä ja konsolin määrittelyillä*; se ei yksin osoita, että toimitettu ohjelma (erityisesti
bcc32-käännetty HkKisaWin) käyttäytyy samoin, eikä sitä, kuinka usein ehto syntyy oikeassa
kilpailussa. Jokaisen kokeen rajaukset ovat luvussa 13.3. / A probe shows the mechanism in the
production code (MSVC, console defines); it does not by itself show the shipped program's
behaviour or how often the condition arises.

### 13.2 Luokittelu / Classification

Luokat / classes: **L** = vahvistettu lähdekoodista (deterministinen polku) / confirmed from
source; **K** = todennettu eristetyllä koeohjelmalla / verified with an isolated probe;
**S** = staattisesti mahdollinen, ajonaikaisesti todentamaton / statically possible, not verified
at run time; **X** = kumottu tai olennaisesti heikentynyt / refuted or materially weakened.

| Havainto / Finding | Luokka / Class | Muutos / Change | Vakavuus / Severity |
|---|---|---|---|
| H-087 viestin `EmitHtmlOsuus` | **L** (GUI, ei linkitettävissä / not linkable) | vahvistettu; `print`-vuoto yksin pysäyttää automaattitulostuksen, GUI jäätyy vasta, kun emit-tietue saapuu ja pääsäie lukee emitin (`rastiva`-vuoto + `addem`) | korkea / high (ennallaan) |
| H-090 `tall_regnly` jättää `ajat`in | **L + K** | neljä paluupolkua todennettu (`:2782`, `:2776`, `:2771`, koodi 'G' `:3074`); **'G'-kellotietue vuotaa joka kerta**, joten `TUNNJONO`-asetuksella pito on lähes varma; lisäpolku `goto loppu` (`:2825-2832`, ei ajettu) | **korkea / high** (oli keskitaso–korkea), kun `TUNNJONO` käytössä |
| H-091 HK tall ↔ print | **K (molemmat reunat) + S (sykli)** | reuna tall → print (`tall_elahto` → `tallenna` → `sivuTuloste`) ja print → tall (`autotulostus` → `tulostasarja`) todennettu tuotantokoodissa; kahden säikeen lukkiutumista ei ajettu; `lisaToimetOn` asetetaan vain GUI:ssa | korkea / high (ennallaan) |
| H-093 loki ↔ tcpip, rekursio (HK) | **L + K** (rekursio), **K** (ABBA kahdella säikeellä) | rekursio pinon ylivuotoon (501 tasoa, noin 2 kt / taso, 1 Mt:n pino) ilman soketti­virhettä, kun lokiyhteys on katki ja `comfl` päällä (vaatii toisen tiedonsiirtoyhteyden); todellinen kahden säikeen lukkiutuminen toistettu; **viesti kumottu** (TCP-lokia ei voi asettaa) | **korkea / high** HK (oli keskitaso–korkea); viesti ei koske |
| H-094 `etls` | **L + K** (menetys), **S** (vapautetun käyttö) | merkki ei koskaan käsitelty vaihdon aikana (todennettu); `vahaku` ei tuota ikkunan aikana (tarkennus) | keskitaso / medium (ennallaan) |
| H-099 HK `aloitus` jättää `inb`in | **X** | `initcom` palauttaa aina 0 (`Tp/TpComY.cpp:2376-2426`), joten haara `Hk/HkInit.cpp:3659-3661` on kuollut / dead branch | **kumottu / refuted** |
| Viestin emit ↔ rastiva (luku 7.2) | **S** → uusi **H-103** | vaiheen 1 "kumottu" oli väärä: käänteinen reuna kulkee `uusiemitanalyysi` → `lueem` | keskitaso / medium |

### 13.3 Koeohjelmien tulokset / Probe results

Kaksi täyttä ajoa samoin tuloksin (`tulokset/c6_hk_probes.txt`, `..._ajo2.txt`); kääntäjä MSVC
19.52.37029 x86 (`tulokset/rakennus.txt`). Kaikki säikeet jotka jäivät lukkoon, lopetettiin
`CoreMain`in `TerminateProcess`illa; kaatumista tai aikakatkaisua ei käytetty näyttönä.
/ Two full runs, identical; blocked threads ended by CoreMain; no crash or timeout used as evidence.

| Koe / Probe | Tuotantopolku / production path | Tulos / Result | Mitä ei osoita / What it does not show |
|---|---|---|---|
| P1 H-090 (4 + kontrolli) | `tall_regnly` RTR-1-sanoma `Hk/HkIV.cpp:2072-2083`, `:2615-2650` → `Enter(ajat)` `:2673-2676` → paluu `:2782`, `:2776`, `:2771`, `:3074` | työsäikeen päätyttyä `ajat`in omistaja on yhä se, `RecursionCount` 1, muu säie ei saa lukkoa; kontrolli (`tunnjono=-1`) vapaa | `:2776`-koe käyttää epärealistista `maxero=0`:aa; oikean laitteen ja asetusyhdistelmän esiintyvyys |
| P2 H-093 rekursio | `wkirjloki` `Hk/HkApu.cpp:560`, `:583` → `wrt_st_TCP` `com/WinUDP.cpp:938-942` → `writesocketerror` `:175-182` → `wkirjloki` … | `0xC00000FD` 501 sisäkkäisen `wkirjloki`-kehyksen jälkeen, `reconnectTCP` ei koskaan saavutettu; kontrolli (`reconnect=0`) palaa | yhteystietue kirjoitettiin muistiin PDB:n tyyppitiedolla tarkistetuin siirtymin (ei `openTCPloki`a, ei oikeaa katkosta); probe sieppasi ylivuodon, tuotannossa se kaataisi prosessin; bcc32:n kehyskoot |
| P3 H-093 ABBA | (a) `wkirjloki` pitää `loki`a, odottaa `tcpip`iä; (b) `wrt_st_TCP` pitää `tcpip`iä, `select`-virhe → `wkirjloki` odottaa `loki`a; (c) molemmat yhtä aikaa | (a), (b) reunat ja (c) todellinen kahden säikeen lukkiutuminen: `loki`n omistaja X, `tcpip`in Y, kummallakin yksi odottaja, molemmat säikeet elossa | oikeat soketit ja ajoitus; "yhdistetty"-tila asetettiin käsin |
| P4 H-094 | `add_bdg_t` `Hk/HkIV.cpp:815` → laiska kuluttaja `pros_bdg_t` `:790` poistuu `:796` | `vaiheenvaihto`lla merkkiä ei käsitelty (`regnlyhetki[0]` ennallaan, kuluttajasäiettä ei jäänyt); kontrolli käsitelty 2 s:ssa | vapautetun käytön kilpa (ei yritetty); ikkunan todellinen pituus |
| P5 H-091 reunat | (a) `tall_elahto` `:572` → `tallenna` → `lisaToimet` `Hk/HkDat.cpp:1374`, `:784-787` → `sivuTuloste` `Hk/HkTulostus.cpp:932`; (b) `autotulostus` `:985`, `:1004` → `tulostasarja` `:5850` | (a) säie pitää `tall`ia ja odottaa `print`iä; (b) säie pitää `autotul`ia ja `print`iä ja odottaa `tall`ia; kontrollit vapaat | `lisaToimetOn` asetettu käsin (vain GUI-lomake asettaa sen), GDI-tulostin ja `lstf` vain lippuina; kahden säikeen sykliä ei ajettu; bcc32-käännöstä ei ajettu |
| Q65 (`q65_leave.c`) | puhdas Win32 | ks. 13.5 | muut Windows-versiot; Application Verifier |

**Koeohjelmien tarkastus / probe review** (Opus): kaikki kokeet kutsuvat todellista tuotantofunktiota;
projekti eroaa `TpCoreHk.vcxproj`:sta vain polkujen, GUID:n ja testilähteiden osalta; ainoat korvaukset
ovat testikehyksen `writeerror*`/`perr`-tynkät. Yhtään koetta ei kumottu. Rajaukset, jotka on otettava
huomioon / caveats:
1. Keinotekoinen tila on oikotie, ei mahdoton polku: `maxero=0` (`:2776`), `tunnjono`n vaihto
   `KELLO_ESTO`-kokeessa, `comfl=1` (vaatii tuotannossa toisen yhteyden), PDB-siirtymin kirjoitetut
   yhteystietueet, `INVALID_SOCKET` virhelähteenä (10054 ei synnytä reunaa, koska sitä ei lokiteta),
   `lisaToimetOn` ilman GUI-lomaketta ja tekaistu `lstf`. / Artificial state is a shortcut, not an
   impossible path.
2. H-091 on sykli vain HkKisaWinissä: konsolissa `lisaToimetOn` on aina epätosi
   (`Hk/HkGlobals.cpp:251`), joten HkMaalissa on vain reuna print → tall. Polkujen haarat
   (`Hk/HkDat.cpp:784-789`, `:1373-1375`; `Hk/HkTulostus.cpp:924-932`, `:969-1004`, `:5850`) ovat
   ilman `#ifdef`iä, joten GUI-päätelmä on staattinen; bcc32-käännöstä ei ajettu. / H-091 is a cycle
   only in HkKisaWin; the GUI conclusion is static.
3. Pinon syvyys (501 tasoa) riippuu kääntäjästä ja pinon koosta; 40 kt:n pinolla noin 20 tasoa.
4. Odottajan estymiskohta päätellään lukon odottajamäärästä (`LockCount`), ei pinovedoksesta.
5. Kokeet osoittavat, että lukko jää varatuksi tai säie estyy; ne eivät osoita, mitkä tuotannon
   säikeet sen jälkeen pysähtyvät. / Probes show a held lock or a blocked thread, not what stalls next.
6. H-090:n `goto loppu` -polku (`:2825-2832`) vaatii, että `lahdejonohaku` muuttaa jonon lukon
   ottamisen jälkeen; sitä ei ajettu.
7. Q65: yksi ajo, x86 WOW64:ssä, ei odottajia S1:ssä; vanhemmissa Windowseissa (ennen Server 2003
   SP1:tä) ylimääräinen `Leave` voi näyttää lukon varatulta ja jumittaa; Microsoft dokumentoi
   tapauksen virheeksi. S4 (ulomman otteen menetys) on pelkkää rekursiolaskentaa eikä riipu
   versiosta.

Tarkastuksen uudet huomiot / new observations: konsolin `vaihdavaihe` palaa `vaiheenvaihto` päällä
(`Hk/HkIx.cpp:1873-1874`, H-031), jolloin H-094:n menetys on pysyvä; `close_bdg_t`:n ja `add_bdg_t`:n
vapautetun käytön kilpa (`Hk/HkIV.cpp:812`, `:845-852`) ei toteutunut ajoissa eikä ole todennettu.

### 13.4 Viiden havainnon suorituspolut / Execution paths of the five findings

**H-087 (ViestiWin).** Pääsäie: `TTulosteForm::ButtonTulostaClick` (`ViestiWin/TulostusUnit2.cpp:257`;
rajaus R, sisältö E, kohde tiedosto, tyyppi HTML; `uusiemitanalyysi(true)` `:407-408` takaa
`emitanal_fl`:n) → `list` (`Juk/VTulostus.cpp:8709`, `:8823-8827`) → `tulostasarja` (`tall`
`:5746`–`:6431`) → `EmitHtmlOsuus` (`Juk/VEmitHtml.cpp:187`): `Enter(print)` `:208`,
`Enter(rastiva)` `:223`, ei yhtään `Leave`a. Lukot jäävät pääsäikeelle (rekursio: pääsäie ei
jumiudu itse). Seuraukset: `autotulostus` (`taustaThr`/`LukumaaratTh`) jää odottamaan `print`iä
`autotul`ia pitäen → automaattitulostus loppuu; `tarkcom` (`Juk/VCom.cpp:828`) tai lukija
(`Juk/VIv.cpp:1110`) → `addem` pitää `emit`iä (`Juk/VEmit.cpp:1510`) ja odottaa `rastiva`a
(`:1524`, `:3243`) → kaikki saapuva liikenne pysähtyy; pääsäikeen seuraava `lueem` (`:1409`,
`ProsEmitOn`) odottaa `emit`iä → GUI jäätyy. Ei ajoitusehtoa: riittää, että emit-tietue saapuu
tulostuksen jälkeen. Vain ViestiWin (konsolissa runko tyhjä, `:189`).

**H-090 (HK).** Lukijasäie `lue_regnlyThread` → laitejäsentimet → `tall_regnly`
(`Hk/HkIV.cpp:1984`): `Enter(ajat)` `:2674` ehdolla `tunnjono>=0 && jono!=tunnjono && piste==0`;
vapautus `:3027-3028` on A–F-lohkon sisällä. Vapauttamattomat paluut: `:2771` (`KELLO_ESTO`),
`:2776` (kelloero), `:2782` (ajanottoa ei käynnistetty / jono täynnä), mikä tahansa muu tunnus
(esim. kellosanoma 'G' `:3058`/`:3074`) ja `goto loppu` `:2825-2832`. Lukijasäie ei jumiudu
itse; sen sijaan käsiajanotto (`cbHk/UnitAjanotto.cpp` → `tall_rivi` `Hk/HkAjat.cpp:128-129`),
`tarkcom`in aikapaketit (`Hk/HkCom32.cpp:446`) ja vaiheen vaihto (`Hk/HkIx.cpp:1898`) estyvät;
vaiheen vaihto pitää `tall`ia ja odottaa `ajat`ia, jolloin lukijan seuraava `Enter(tall)` sulkee
syklin. Asetus `TUNNJONO` (`Hk/HkInit.cpp:2007-2012`); HkKisaWin ja HkMaali.

**H-091 (HkKisaWin).** Reuna tall → print: mikä tahansa `tall`ia pitävä tallennus, jossa tila
muuttuu V/E → '-' (`Hk/HkDat.cpp:1373-1374` → `:784-787` → `sivuTuloste` `Hk/HkTulostus.cpp:932`);
esim. lukijan lähtöimpulssi (`Hk/HkIV.cpp:2968-2982`, `:3031`), `tarkcom`, lomakkeet. Reuna
print → tall: `LukumaaratTh` → `autotulostus` (`:985`, `:1004`) → `tulostasarja` (`:5850`) tai
pääsäikeen `lahtoluettelo` (`cbHk/HkKisaLahtLuet.cpp:584` → `:381`). Asetukset: "Jatkuva
sivutuloste" GDI-tulostimeen (`cbTp/UnitSivuTuloste.cpp:92-100`) ja automaattitulostus tai
"vain läsnä olevat" -lähtöluettelo. Ei porttilukkoa; `lahtoluettelo` pitää `print`iä koko
luettelon ajan.

**H-093 (HK).** Asetus `YHTEYS…=TCPL` (`Hk/HkInit.cpp:1133-1137`) + vähintään yksi muu
tiedonsiirtoyhteys (`comfl` asetetaan vain `opencom`issa `Tp/TpComY.cpp:2204`; `openTCPloki`
ei aseta sitä). Rekursio: kun lokiyhteyden `connected==0` (vastaanottosäie `com/WinUDP.cpp:603`
tai epäonnistunut lähetys `:931-932`), jokainen lokirivi → `:938-942` → `writesocketerror(2, 0, …)`
(10054-suodatin ei estä, `err==0`) → `wkirjloki` → … ennen `reconnectTCP`:tä (`:943`). Ei
uudelleentulon estoa; `lokiTCP`ia ei nollata. Pinon ylivuoto; lokisäikeen pino 40 kt (`:778`),
muut 1 Mt. ABBA: lokittava säie (`loki` → `tcpip`) vs. TCP-lähetyksessä epäonnistuva säie
(`tcpip` → `loki`). Viesti: `Juk/VInit.cpp` ei koskaan anna `-2`:ta (`:1064-1091`), joten
`lokiTCP` = 0 → ei koske viestiä (Q66).

**H-094 (HK).** Tuottajat lukijasäikeessä (`Tp/TpLaitteet.cpp:494` `Hk/HkIV.cpp:1074`:n kautta,
`:1055`); `vahaku` on suljettu `!vaiheenvaihto`-ehdolla (`Hk/HkIV.cpp:3566`, `:3587`). Kuluttaja
`pros_bdg_t` poistuu `vaiheenvaihto`sta tyhjentämättä jonoa ja `close_bdg_t` poistaa lukon ja
vapauttaa puskurin ilman lukkoa (`:745-751`); `add_bdg_t` testaa NULLin ilman lukkoa (`:826`).
Koko vaihdon ajan jokainen merkki luo uuden jonon ja säikeen, joka poistuu heti → merkki
menetetään (deterministinen, todennettu). Vapautetun käyttö tai NULL-kirjoitus on ajoituksesta
riippuva.

### 13.5 Avoimet kysymykset / Open questions

| # | Tila / Status | Vastaus / Answer |
|---|---|---|
| Q65 | **ratkaistu / resolved** (koe / probe, Windows 10.0.26100) | Ei-omistajan `LeaveCriticalSection` **vapauttaa toisen säikeen pitämän lukon** (poissulku rikkoutuu, ei jumia); ylimääräinen `Leave` vapaaseen lukkoon vie `RecursionCount`in −1:een, mutta seuraava `Enter` toimii; ylimääräinen `Leave` ulomman kehyksen pitäessä lukkoa **vapauttaa ulomman otteen**. Siis H-088:n ja H-089:n vaikutus tällä Windowsilla on suojaamaton kriittinen alue, ei jumi. Käytös on dokumentoimatonta; muita versioita ei kokeiltu. / A non-owner Leave releases another thread's hold; an extra Leave inside an outer hold releases the outer hold; no hang on this build. |
| Q66 | **ratkaistu / resolved** (lähde + koe P2) | Ei rajaa: rekursio jatkuu pinon ylivuotoon; polku `:938-942` ei tarvitse sokettivirhettä. Vain HK. |
| Q67 | **osittain / partly** | `aloitus` voi ajautua uudelleen samassa GUI-prosessissa, kun ensimmäinen ajo epäonnistuu varhain (painikkeet jäävät käyttöön, `cbHk/WinHk.cpp:279-290`, `:1176-1262`; `ViestiWin/UnitMain.cpp:220-228`) → pitämättömien lukkojen uudelleenalustus, tiedosto- ja muistivuodot. "Myöhäinen epäonnistuminen `inb` varattuna ja säikeet käynnissä" ei ole mahdollinen (`initcom` palauttaa aina 0; säikeet vasta `Hk/HkInit.cpp:3708`/`Juk/VInit.cpp:2958`). Kutsuttujen funktioiden sisäiset apusäikeet ennen epäonnistumiskohtaa tarkastamatta. Ei rekisteröity. |
| Q69 | **ratkaistu / resolved** | HkMaali: konsolin `leimat()` → `pros_emit` lukee `rata`a, `haerata`a ja `bdg2kno`a ilman lukkoa (`Hk/HkIV.cpp:1676-1699`), kun `tarkcom` ajaa etäpyynnöstä `vaihdavaihe`n (`Hk/HkCom32.cpp:847`) → kapea vapautetun käytön ikkuna; lisäksi `bdg_kno` rakennetaan uudelleen paikallaan lukijan lukiessa (väärä numero). HkKisaWin: sarjallistettu pääsäikeellä. Kirjattu H-102:n laajennukseksi. |
| Q70 | ratkaistu | Kyllä, kapea ikkuna sulkemisessa (`com/WinUDP.cpp:589-641`, `:674-681`); H-096:n laajennus. |
| Q71 | ratkaistu | Viestissä `remtimer` ajetaan ennen `closecom`ia, joten H-095:n 2,5 s:n ikkunaa ei ole; vain käynnissä oleva `tark_aika`-kutsu (`Juk/VCom.cpp:445-450`, `Juk/vApu.cpp:980-986`). Huomautus H-095:een. |
| Q72 | ratkaistu | `LAJUNEN` on määritelty HkKisaWinissä, ViestiWinissä, HkMaalissa ja JukMaali520:ssä, ei JukLuenta520:ssa. |
| Q74 | **ratkaistu / resolved** | Uudet reunat: tall → loki (HK `Hk/HkDat.cpp:932`, `:1367`, `:1370`; viesti `Juk/vdat.cpp:844` → `Tp/TpApu.cpp:842`), viestin rastiva → emit (`Juk/VEmit.cpp:6148` → `:6157` → `:1409`) → **sykli H-103**; tall → idindex/kilpdat (lehtiä). `tark_partio` tulostaa ilman `print_CS`:ää (`Juk/VEmit.cpp:2391-2443`); `TarkKilpailijat` → `kirj_err_file` modaalinen dialogi `vaihdavaihe`n lukkojen alla (`Hk/HkEmit.cpp:2833`). Kolmen lukon ketju print → tall → emit → print vaatii vaiheen vaihdon ja H-091:n ja H-092:n ehdot yhtä aikaa; ei omaa riviä. |

### 13.6 Luvun 9 ehdokkaat / Section 9 candidates

| Ehdokas / Candidate | Tulos / Result |
|---|---|
| `LukumaaratTh`/`ProsEmitThread` kahdentuminen | S, matala: vain kun `FormEmit` on NULL tai vanha säie on estynyt yli 500 ms; vaikutus ylimääräinen kysely. Ei rekisteröity. |
| Lomakkeen ja säikeen elinikä | S: ei determinististä vapautetun käyttöä; ikkunat sulkemisessa (`UnitSakkoKierr` `Sleep(1500)`, `ScrollThread`, `SelTimerThread` ikkunan X-painikkeella) tai vaiheen vaihdossa (`DisplTimerThread`, 500 ms + 600 ms marginaali). Ei rekisteröity; ks. 13.7. |
| `openSirit`in staattinen `acn` | **H-108** (S, matala–keskitaso): kaksi tai useampi Sirit-lukija → säikeet voivat saada saman lukijanumeron. |
| `closeportUDP` / `UDPstreamrcvTh` | S, hyvin kapea; H-096-perheen huomautus. |
| `closec` ja `sendtcpfile` | X: `closec` vain lopetuksessa, ei uudelleenavausta. |
| `tcp_CS[fileyht]` uudelleenalustus | X: premissi väärä (`startaf` ei kierroksittain); sivuhuomio: epäonnistunutta tiedosto-TCP-yhteyttä ei yritetä uudelleen. |
| `lopetus()` `tall_CS`:n sisältä → `SendMessage` | X lukkiutumisena (`talljarj` palaa `inLopetus`illa); johti uuteen **H-104**:ään. |
| `close_profile` vs. `put_profile` | S, vain diagnostiikka-asetus `PROFILOI`; ei rekisteröity. |
| `vapautaemitmuisti` / vanha emit-kahva | **H-105** (L, keskitaso): `ESITARK`/`ESILUENTA` + vaiheen vaihto → kirjoitus suljettuun tai uudelleenkäytettyyn tiedostokahvaan. `rastiva`n vapautus ilman `rastiva_CS`:ää: päällekkäisyyttä ei löytynyt. |
| tall ↔ ajat (HK) | S: lähdepisteen muuttuminen −1:ksi `piste==0`:sta todentamatta; ei rekisteröity. Huom. H-090:n vuoto tekee tall ↔ ajat -lukkiutumisesta todennäköisemmän (luku 13.4). |
| print → tall → emit → print | huomautus H-091:een ja H-092:een. |
| Viestin `aloitus` ja `inb` | X: tasapainossa (`Juk/VInit.cpp:2921-2928`). |

### 13.7 VCL-säieturvallisuus / VCL thread safety (H-018:n jatko / continuation)

**Mekanismi / mechanism.** Ainoa työsäikeestä käyttöliittymään käytetty siirto on synkroninen
`SendMessage` ikkunakahvaan + `MESSAGE_HANDLER`; `Synchronize`a, `Queue`ta, `TThread`iä,
`PostMessage`a, `ProcessMessages`ia ja `TTimer`iä ei ole (`cbHk/`, `ViestiWin/`, `cbTp/`, ydin)
[todennettu, haku]. `SendMessage` jäädyttää työsäikeen, kunnes pääsäie käsittelee viestin —
lukkoa pitäen tämä on H-009/H-021/T-032-mallin lukkiutumisriski. / The only marshalling is
synchronous `SendMessage`; no Synchronize, Queue, PostMessage, ProcessMessages or TTimer.

**Suorat VCL-käytöt työsäikeistä / direct VCL access from workers** (vahvistetut, H-018:n lisäksi):

| Säie / Thread | Kohta / Site | Olio / Object | Luokka / Class |
|---|---|---|---|
| ViestiWin `taustaThr` (~10 ms) ja `LukumaaratTh` → `autotulostus` → `autostatus` | `ViestiWin/UnitStatus.cpp:532-535`; `Juk/VTulostus.cpp:719` | `FormStatus->Edt…->Text` (4 kpl), `autotul_CS` hallussa | **L, H-106** (aina, kun automaattitulostus päällä; HK kirjoittaa vain lippuja `cbHk/UnitStatus.cpp:586-596`) |
| ViestiWin `ProsEmitThread` | `ViestiWin/UnitMain.cpp:1355-1356` | `FormLuenta->ProgressBar1` | L, H-018:n laajennus |
| HK lukijasäie → `emitLuettu` | `cbHk/UnitEmit.cpp:966-970`; `Hk/HkIV.cpp:1090-1091` | `FormEmit->Handle`, `FormEmit` NULL kunnes lomake avataan | **L, H-107** (`RASTIVATULOSTE`); viestin `emitLuettu` kumottu (ei kutsujia) |
| HK `pros_bdg_t` → `tall_badge` | `cbHk/UnitOsanottajat.cpp:1833-1835` | `FormOsanottajat->Handle`, NULL kunnes lomake avataan | **L, H-107** (`ToimintaTila==1` + merkkilukija) |
| `haeSQLTh`, `kirjSQLTh` | `cbTp/UnitEmitSQL.cpp:63`, `:178`; `cbHk/UnitMySQL.cpp:444-476`, `:699` | dbExpress-komponentit jaettu pääsäikeen kanssa; `inSQLquery` ilman lukkoa | S, H-018:n laajennus |

**Lomakkeiden tuhoutuminen / forms destroyed while used.** HK:n `vaihdavaihe` poistaa `FormEmit`in,
`TulosteForm`in ja `FormOsanottajat`in ja nollaa osoittimet ilman säikeiden odotusta (`cbHk/WinHk.cpp:2084-2107`,
`:2167-2190`); `DisplTimerThread` ja `ProsEmitThread` tarkistavat lipun ennen `Sleep`iä mutta eivät
sen jälkeen (`cbHk/UnitEmit.cpp:834-841`; `cbHk/WinHk.cpp:1614-1632`). Marginaalit 500 ms ja 600 ms
(`~TFormEmit`) → S, ei rekisteröity. Viestissä lomakkeita ei poisteta ennen lopetusta. Lopetuksessa
`ScrollThread` (`TilFrm[]` ei nollata) ja `HaeAvoimetTh` voivat lähettää puoliksi tuhotulle
lomakkeelle → S, vain lopetuksessa.

**Tietojen muuttuminen kesken päivityksen / data changed during a UI update.** Pääsäie ajaa
`vaihdavaihe`n itse, joten UI-käsittelijä voi lomittua vain viestisilmukassa: `vaihdavaihe` kutsuu
`writeerror_w`:tä (`Hk/HkIx.cpp:1938`, `:1944`) ja `TarkKilpailijat` → `kirj_err_file`a modaalisella
dialogilla, kun `tall`, `ajat` ja `emit` ovat hallussa (`:1897-1899`). Modaalinen silmukka
käsittelee työsäikeiden `SendMessage`t, ja käsittelijä perii lukot (rekursio) ja näkee puoliksi
uudelleen ladatut tiedot. Vain `LukumaaraReqHandler` tarkistaa `vaiheenvaihto`n
(`cbHk/WinHk.cpp:1792`). S, virhepolulla → kirjattu tekniseksi velaksi **T-033**. Tuonti
(`cbHk/UnitImport.cpp:1428-1436`) on H-047. `HaeAvoimetHandler` ottaa `tall_CS`:n pääsäikeessä
(`cbHk/UnitSakkoKierr.cpp:99`), mikä tekee jokaisesta `tall`ia pitävästä `SendMessage`-koukusta
lukkiutumisriskin (T-033, H-021:n malli).

### 13.8 Uudet ja muuttuneet havainnot / New and changed findings

Uudet / new: **H-103** viestin rastiva ↔ emit (keskitaso, S), **H-104** HK `lopetus` ei tee mitään
ennalta asetetulla `inLopetus`illa (keskitaso, L), **H-105** vanha emit-kahva vaiheen vaihdon jälkeen
(keskitaso, L), **H-106** ViestiWin `autostatus` VCL-kirjoitukset työsäikeistä (keskitaso, L),
**H-107** HK:n työsäikeen koukut käyttävät NULL-lomaketta (keskitaso, L), **H-108** `openSirit`in
staattinen argumentti (matala–keskitaso, S); **T-033** käyttöliittymäsiirron rakenne.
Muutetut / changed: H-090 ja H-093 → korkea; H-099 kumottu; H-087, H-091, H-094 todennus
päivitetty; H-018, H-088, H-089 (Q65), H-095 (Q71), H-096 (Q70), H-102 (Q69) laajennettu.
Luvun 7.2 rivi "viestin emit ↔ rastiva kumottu" on virheellinen (H-103).

Rekisteröimättä jätetyt (ei perusteltua saavutettavuutta tai vain teoreettinen) / not registered:
`LukumaaratTh`-kahdentuminen, lomake-elinikäikkunat, `closeportUDP`, `close_profile`, tall ↔ ajat,
Q67:n uudelleenalustus.

### 13.9 Korjausjärjestys / Fix priority (alustava / provisional, [päätelty])

- **P2:** H-087, H-091, H-090, H-093 (jäätyminen tai kaatuminen kilpailun aikana tavallisella
  käyttäjän toiminnolla tai asetuksella; H-090 ja H-093 todennettu koeohjelmalla), H-107
  (kaatuminen ensimmäisestä luennasta, kun asetus päällä).
- **P3:** H-094, H-103, H-104, H-105, H-106, H-088, H-089, H-092, H-097.
- **P4:** H-095, H-096, H-098, H-100, H-101, H-102, H-108; H-099 poistettu (kumottu).

### 13.10 Kattavuus ja rajaukset / Coverage and limits

- Kaikki viisi pyydettyä havaintoa, Q65–Q67, Q69, Q74 (+ Q70–Q72), luvun 9 kaikki 11 ehdokasta ja
  VCL-kysymykset käsitelty. Riippumattomasti tarkastettu 11 staattista väitettä ja kaikki
  koeohjelmat.
- Ajonaikaisesti todennettu vain eristetyissä koeohjelmissa (MSVC, konsolin määrittelyt); bcc32-
  GUI-käännöstä ja toimitettuja ohjelmia ei ajettu. H-087 ja H-103 ovat lähdekoodinäytön varassa.
- Avoimiksi jäävät: Q68, Q73 (vaiheesta 1), Q67:n apusäikeet, H-090:n esiintyvyys oikeilla
  laitteilla, `SendMessage` tuhottavaan ikkunaan (lomake-elinikä), ristisäikeinen `TEdit::SetText`
  (H-106:n tarkka vaikutus), `timerfl` GUI:n vaiheen vaihdossa ja `ToimintaTila==1` lukijoiden kanssa
  (H-107:n `tall_badge`-polku).

All five requested findings, the open questions, all 11 section-9 candidates and the VCL questions
were covered; 11 static claims and all probes independently reviewed. Run-time evidence comes only
from isolated MSVC console probes; H-087 and H-103 rest on source evidence.
