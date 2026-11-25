# Virheenkäsittely ja resurssit / Error handling and resources (C-9 vaihe 1 / step 1)

Tila / status: vaihe 1 valmis, riippumattomasti tarkastettu, odottaa käyttäjän katselmointia
(2026-11-26). / Step 1 done and independently reviewed; awaiting the user's review.

Analysoitu revisio / analysed revision: `7aaca40` (tuotantolähdekoodi ennallaan revisiosta
`f7fe553` / production sources unchanged since `f7fe553`). Polut suhteessa `TPsource/V52/`, ellei
toisin mainita. Kaikki tulokset ovat staattisia: mitään toimitettua ohjelmaa ei ajettu, eikä
`TPsource/`-puuhun tehty muutoksia. Ainoa ajo oli tarkastajan Win32-koe `FormatMessage`-kutsulla
(luku 7, H-157). / Paths relative to `TPsource/V52/`. All results are static: no shipped program
was run and nothing under `TPsource/` was changed. The only run was the reviewer's Win32 test of
`FormatMessage`.

Raakatulokset ja agenttien raportit / raw results and agent reports: `todisteet/c9/`.

## 1. Yhteenveto / Summary

Erä tarkasteli, mitä ohjelma tekee, kun kirjoitus, avaus tai varaus epäonnistuu kilpailun aikana,
vuotavatko kahvat ja muisti toistuvissa toiminnoissa ja missä tilassa ohjelma jatkaa
virheilmoituksen jälkeen. Pääpiirteet:

1. **Virheet huomataan useimmiten, mutta niistä ei toivuta.** `KILP.DAT`:n, emit-tiedoston ja
   AJAT-jonon kirjoitusvirheistä tulee modaali-ilmoitus, mutta paluuarvoa ei välitetä kutsujalle,
   eikä mitään yritetä uudelleen. Muisti jatkaa ajan tasalla ja levy jää jälkeen; uudelleenkäynnistys
   lataa vanhan tilan (H-015 laajennettuna, H-145, H-146, H-148).
2. **Emit-kortin tallennusvirhe kuitataan lukijalle onnistuneeksi** (H-145): kortti jää pois
   emit-tiedostosta, hakemistosta ja muilta työasemilta, ja saman kortin uusi luku tulkitaan
   vahvistukseksi.
3. **Tulos- ja automaattitiedostojen kirjoitusvirheitä ei havaita lainkaan** (H-147): tarkistus
   `WriteLine(...) == WEOF` ei voi laueta MSVC-ohjelmissa millään merkistöllä eikä GUI-ohjelmissa
   8-bittisillä merkistöillä. `autofile` tyhjentää julkaistavan tiedoston avatessaan.
4. **Elinkaarivirheitä automaattitulostuksessa**: vapautettua `PRFILE`-rakennetta käytetään ja se
   vapautetaan uudelleen viestin `autofile`:n oletustilassa joka kierroksella (H-149), HK:ssa
   TCP-tiedostonsiirron jälkeen (H-151) ja tulostimen vaihdon jälkeen (H-152); HK:n XML-vienti
   vuotaa `TextFl`:n ja mahdollisesti `FILE`-kahvan joka kierroksella (H-150).
5. **Virheilmoituksen "Poistu ohjelmasta" -valinta voi lukita GUI:n** (H-153), kun ilmoituksen
   antanut työsäie pitää lukkoa, jota lopetus tarvitsee.
6. **Kilpailupäivän aikana tyhjenevää resurssivuotoa ei löytynyt tavallisista tapahtumista.**
   GDI-fontit, tulostimen DC, `PRFILE`-paikat ja `TextFl`-oliot ovat tasapainossa; vuodot ovat
   virhepoluilla (pistokkeet H-155) tai erityisasetuksilla (H-150).
7. Valtaosa `writeerror`-kutsuista (arvio 85–90 %) on harmittomia: ilmoitus ja paluu ilman
   puolikasta päivitystä. Todellinen haitta on estävyys: ilmoitus pysäyttää kutsuvan säikeen,
   usein lukon alla (T-032), konsolissa Enteriä odottaen.

Uudet havainnot: H-145–H-157 (9 keskitason, 4 matalaa, joista 2 kokoomarivejä), T-040–T-041,
Q83–Q90; laajennukset H-044, H-071, H-105. Yksikään havainto ei ole korkea tai kriittinen
staattisen näytön perusteella; H-149 voi nousta korkeaksi, jos ajo osoittaa kaatumisen (Q85).

The batch examined failed writes, opens and allocations during a competition, leaks in repeated
operations, and the program state after an error report. Errors are mostly detected but never
recovered: return values are dropped and nothing is retried, so memory and disk diverge until a
restart loads the old state. An emit card whose write fails is still acknowledged to the reader
(H-145). Write errors on result and auto output files are never detected (H-147). Automatic
output has use-after-free and double-free paths (H-149, H-151, H-152) and a per-run leak in HK XML
export (H-150). The "exit" choice of an error dialog can deadlock the GUI (H-153). No ordinary
per-event leak exhausts a resource within a competition day. Most `writeerror` calls are
harmless; the real cost is that they block the calling thread, often under a lock. New:
H-145..H-157, T-040..T-041, Q83..Q90; H-044, H-071, H-105 extended. None is high on static
evidence; H-149 may become high if a run shows a crash (Q85).

## 2. Rajaus ja menetelmä / Scope and method

Rajaus: `suunnitelma.md` "C-9 vaihe 1". Osat: A koneellinen inventaario ja luokittelu, B pysyvän
datan virhepolut, C resurssien elinkaari, D tila virheen jälkeen ja lopetuspolut, E riippumaton
tarkastus (Opus), F dokumentointi. Rajattu pois: muistiturvallisuus (C-4), lukot ja säikeiden
lopetus (C-6), yhteysprotokolla (C-2), lomakesyötteet (C-7), ulkoiset kirjastot (C-10), korjaukset.

| Osa / Part | Tekijä, malli / By, model | Tokenit / tokens | Raportti / report |
|---|---|---|---|
| A | `tyokalut/resurssit.py` + `architecture-analyst`, Sonnet | 291 k | `todisteet/c9/osaA-luokittelu.md` |
| B | `defect-concurrency-auditor`, Sonnet | 210 k | `todisteet/c9/osaB-pysyva-data.md` |
| C | `defect-concurrency-auditor`, Sonnet | 286 k | `todisteet/c9/osaC-elinkaari.md` |
| D | `defect-concurrency-auditor`, Sonnet | 231 k | `todisteet/c9/osaD-virheen-jalkeen.md` |
| E | `independent-verifier`, Opus | 197 k | `todisteet/c9/osaE-tarkastus.md` |
| F | pääistunto / main session | – | tämä tiedosto / this file |

Agentit yhteensä noin 1,22 milj. tokenia. Poikkeama rajauksesta: osan B monivaiheinen päättely
tehtiin Sonnetilla, ja Opus käytettiin vain osassa E (ei erillistä Opus-ajoa B:lle). / Agents
about 1.22 M tokens; part B used Sonnet, Opus only in part E.

## 3. Inventaario / Inventory (osa / part A)

Skripti `tyokalut/resurssit.py` luettelee jokaisen resurssin hankinta- ja vapautuskutsun,
kirjoituskutsun, lopetuskutsun ja `writeerror`-kutsun, kohdistaa sen funktioon (`globaalit.py`:n
jäsennin) ja luokittelee paluuarvon käytön leksikaalisesti. Ensimmäinen ajo (v1, `osaA/`) oli
agenttien syöte; agentin otos (noin 135 riviä) löysi 12 systemaattista virhettä (S1–S12,
`osaA-luokittelu.md` 1.3), joista S1–S3 ja S5–S8 korjattiin versioon v2 (`osaA/v2/`).

| | v1 | v2 |
|---|---|---|
| Tiedostoja / files | 417 | 417 |
| Kutsukohtia / call sites | 3 130 | 3 234 (+ `openprfile`, `_wfdopen`, `lopetus`, `perr` jne.; − 60 määrittelyä / definitions) |
| Ei käännetä yhteenkään tuotteeseen / not built | 211 | 230 (sarake `built`) |
| `fclose` "checked" | 20 (kaikki vääriä / all false) | 0 |
| `Open` / `Close` (TextFl) | 33 / 76 (enimmäkseen VCL-olioita) | 8 / 19 |

Jäljelle jäävät rajoitukset: epäsuora tarkistus (tavumäärä ulostuloparametrissa, kertyvä paluuarvo,
S4), kuollut koodi (S12) ja makrolla rajattu koodi (S10) näkyvät edelleen eläviä; tarkistusikkuna on
6 riviä (esim. `Tp/TpComY.cpp:3013`, tarkistus kohdassa `:3021`). Luokitus on työlista, ei
virhemäärä.

Ajovaiheluokitus (agentin sääntöpohjainen, v1-riveistä): S 370, K 699, U 1 546, L 29, kirjasto 127,
U/K 117, luokittelematon 31, ei käännetä 211. Funktioita, joissa on pysyvä kirjoitus tai
tarkistamaton hankinta: 235, joista kilpailun aikana toistuvia (K) 78.

The script lists acquire/release, write, exit and error-report calls with their function and a
lexical return-value class. The first run (v1) fed the agents; the agent's sample found 12
systematic errors, of which S1-S3 and S5-S8 were fixed in v2. Indirect checks, dead code and
macro-gated code still look live. The classification is a worklist, not a defect count. 78
functions that run repeatedly during a competition contain a persistent write or an unchecked
acquire.

## 4. Pysyvä data / Persistent data (osa / part B)

(1) havaitaanko, (2) kerrotaanko käyttäjälle, (3) yritetäänkö uudelleen tai voiko korjata
myöhemmin, (4) voiko tulos kadota tai muisti ja levy erota, (5) voiko osittainen kirjoitus
rikkoa tiedoston. / (1) detected, (2) operator told, (3) retry/recovery, (4) silent loss or
divergence, (5) partial-write corruption.

| Tiedosto / file | (1) | (2) | (3) | (4) | (5) | Havainnot / findings |
|---|---|---|---|---|---|---|
| `KILP.DAT` | tavumäärä tarkistetaan, kutsujat ohittavat −1:n (`dbboxm/dbbox1.cpp:84-100`) | modaali Kyllä/Ei, "Ei" → `lopetus` (`tputilv2/perr.cpp:28-60`) | ei; vain saman tietueen uusi tallennus tai vienti "DAT" | kyllä: muisti ensin (`Hk/HkDat.cpp:616`), levy luetaan vain käynnistyksessä | `numrec++` ennen kirjoitusta (`dbboxm/dbbox1.cpp:227-251`); otsikkoristiriita estää käynnistyksen | H-015, H-148, H-153 |
| emit | kyllä (`Hk/HkEmit.cpp:1600-1625`) | modaali lukijasäikeessä `emit`-lukon alla; virhekoodi väärä | ei | kyllä: kortti kuitattu, ei tiedostossa (H-145) | osittainen tietue katkaistaan käynnistyksessä (`Hk/HkEmit.cpp:3060`) | H-145, H-105 |
| AJAT | kyllä (`Hk/HkAjat.cpp:361-375`) | modaali `ajat`-lukon alla | ei kirjoitusvirheen jälkeen (H-146); vaiheen vaihto kirjoittaa koko tiedoston | kyllä | aukko tiedostoon | H-146, H-054 |
| `COMFILE.DAT` | kyllä | ilmoitus `outb`-lukon alla | ei, osoittimet etenevät | työasemat eroavat | puolikas paikka | H-044 |
| `SQLjonoVarm.dat` | ei | ei; avausvirhe poistaa varmistuksen hiljaa (`cbHk/UnitMySQL.cpp:691-692`) | ei | jono menetetään uudelleenkäynnistyksessä | – | H-014, H-043, H-156 |
| tapahtumaloki, `DATA_ERR.TXT` | ei | ei | ei | rivit katoavat; puskuroitu | – | H-156 |
| tulos- ja automaattitiedostot, XML | ei (H-147) | vain avausvirhe | seuraava kierros | julkaistu tiedosto tyhjä, osittainen tai vanha | tyhjennetään avatessa, ei väliaikaistiedostoa | H-147, H-149–H-152 |
| `vaihevalinta.cfg` | ei | ei | seuraava vaiheen vaihto | väärä vaihe uudelleenkäynnistyksessä | tyhjennetään ennen kirjoitusta | H-156 |

Kestävyys: `KILP.DAT` avataan `FILE_FLAG_WRITE_THROUGH`-lipulla; emit-, AJAT-, `COMFILE`- ja
lokitiedostoissa ei ole `_commit`ia, joten `kirjheti`-liput suojaavat prosessin kaatumiselta mutta
eivät virtakatkolta (H-156). / KILP.DAT uses write-through; the other files have no `_commit`.

## 5. Resurssien elinkaari / Resource lifecycle (osa / part C)

Tavallisissa kilpailun aikaisissa tapahtumissa (tulostus, automaattitulostus, kortin luku,
yhteyden uusinta, vaiheen vaihto) ei löytynyt vuotoa, joka tyhjentäisi resurssin päivän aikana.
Tarkistettu tasapainoiseksi: GDI-fontit (luodaan kerran ja välimuistitetaan, `tputilv2/Wrttext.cpp:556-588`;
`DeleteObject`ia ei ole koodissa lainkaan, mutta kasvu on rajattu), tulostimen DC ja
`PRFILE`-paikat (16), `TextFl`, `vaihdavaihe`+`teejarj(true)` (käyttää taulukot uudelleen,
`Hk/HkIx.cpp:1563-1617`; vain `aloitus`-uusinta vuotaa, H-133), `httphaku`, `reconnectTCP`.

Vuodot ja elinkaarivirheet:

| Havainto / finding | Kasvu / growth | Raja ja seuraus / limit and effect |
|---|---|---|
| H-150 HK `autofile`, XML, vaihe ≥ 2 | `TextFl` + `FILE`, jos `AUTOFILE.LST` on olemassa, joka kierros | UCRT 512 virtaa ≈ 500 kierrosta; C++Builderin `_NFILE_` 50 (Q84) ≈ 45 min 60 s välillä; sen jälkeen kaikki stdio-avaukset epäonnistuvat |
| H-155 pistokkeet avausvirheellä | 1–2 pistoketta epäonnistunutta TCP-yritystä kohden | lyhytaikaiset portit ≈ 16 000 → noin 11 h jatkuvia hylättyjä yrityksiä |
| H-149, H-151, H-152 | ei kasvua; käyttö vapautuksen jälkeen | kekovaurio tai kaatuminen; ajoa ei tehty (Q85) |
| H-154 tulostimen DC | ei kasvua | automaattitulostus loppuu hiljaa yhden taustatulostusvirheen jälkeen |

No ordinary per-event leak exhausts a resource in a day. Fonts, printer DC, PRFILE slots and
TextFl are balanced. The leaks are on error paths or in special configurations; the lifecycle
bugs are use-after-free and double free in automatic output.

## 6. Tila virheen jälkeen ja lopetuspolut / State after an error and exit paths (osa / part D)

Linkitetyt lopetusfunktiot (Q4 ratkaistu): HkMaali `Hk/HkInit.cpp:3730` päättyy `_exit(0)`:aan
(`:3830`, ei CRT-huuhtelua); ViestiMaali ja ViestiLuenta `Juk/vApu.cpp:944`, `exit(0)` (`:1035`);
GUI-ohjelmissa samat rungot ilman prosessin lopetusta, ikkuna sulkeutuu VCL:n kautta.
`tputilv2/globals.cpp` ei ole yhdessäkään projektissa. / Which `lopetus1` is linked into each
product; `globals.cpp` is not linked.

Lopetuspolut kilpailun aikana: Esc (H-104: `inLopetus` ennalta asetettu → ei vaikutusta),
verkkokomento `tark_extra` 9 (`Hk/HkCom32.cpp:652-662`, tarkoituksellinen, 3 s varoitus),
`perr`/`wperr` "Ei" mistä tahansa säikeestä (`tputilv2/perr.cpp:92-94`) ja GUI:n
`writeerror_w`-dialogin "Poistu ohjelmasta" (`cbTp/cbTpApu.cpp:451-462`). Kaksi viimeistä voivat
lukita GUI:n (H-153). HK-konsolin `_exit` ei sulje `aftulparam.lstf`:ää eikä odota taustasäiettä
(H-157). Uudelleenkäynnistys palauttaa `KILP.DAT`:n, AJAT-jonon ja levypuskuroidun `COMFILE`n;
muistipuskuroidut lähettämättömät sanomat menetetään.

Konsolin `writeerror_w(..., 0)` odottaa Enteriä kutsuvassa säikeessä
(`tputilv2/Writeerr.cpp:66-94`): yhteyssäikeet (joka 10. virhe, `Tp/TpComY.cpp:853`, `:1074`),
`tarkcom`, lukijasäikeet ja `taustaThr` (automaattitulostus) voivat jäädä odottamaan; GUI:ssa
kutsuva säie odottaa `perr_ptrs2.vast`:ia (H-009, H-019, H-144). Osan D otos: noin 10–15 riviä
368:sta jättää puolikkaan tilan (`todisteet/c9/osaD-virheen-jalkeen.md` luku 4).

## 7. Uudet havainnot / New findings

Tarkastus (osa E, Opus): 19 väitettä 21 rivinä; 16 vahvistettu, 5 osittain; yksi osaväite kumottu
(osan C väite, että viestin `autofile` ei käänny). Kaksi väitettä osoittautui vanhojen havaintojen kaksoiskappaleiksi
(N-D1 = H-044, H-134:n laajennus = H-105). Tarkastaja löysi kaksi uutta asiaa; H-149:n pääistunto
todensi koodista (`Juk/VTulostus.cpp:7141-7164`, `:7200`, `:7295-7321`; `tputilv2/Wrttext.cpp:279-300`).
/ Review: 19 claims in 21 rows, 16 confirmed, 5 partly, one sub-claim refuted; two duplicates; two new items from the
reviewer, H-149 re-read by the main session.

| ID | Lyhyesti / In short | Vakavuus / severity | Lähde / source |
|---|---|---|---|
| H-145 | Emit-kirjoitusvirhe: kortti kuitataan lukijalle ja merkitään luetuksi, ei tallenneta eikä lähetetä / emit write failure: card ACKed, cached, lost | keskitaso | N-B1 |
| H-146 | AJAT `writeajat` siirtää `fp1`:n ennen kirjoitusta; epäonnistunut rivi jää aukoksi / fp1 advanced before write | keskitaso | N-B2 |
| H-147 | Tulos- ja automaattitiedostojen kirjoitusvirheitä ei havaita (`== WEOF`), tiedosto tyhjennetään avatessa, `vainMuutt` pudottaa päivityksen / output write errors undetected | keskitaso | N-B3, E |
| H-148 | Viestin `addfrec`: tarkistamaton uudelleenavaus; levy täynnä → kahva suljettu ja uusi joukkue puuttuu muistista / unchecked reopen | keskitaso | N-B4 |
| H-149 | Viestin `autofile`: `PRFILE` vapautetaan ja käytetään uudelleen joka kierros oletustilassa; `aflst` poistetaan kahdesti / relay autofile use-after-free and double free | keskitaso | E uusi 1 |
| H-150 | HK `autofile`: `aflst`-vuoto (ja `FILE`) joka kierros XML-viennissä vaiheessa ≥ 2 / leak per run | keskitaso | N-C1 |
| H-151 | `aftulparam.lstf` jää osoittamaan vapautettuun `sendtcpfile`-säikeen jälkeen / dangling after thread | keskitaso | N-C2 |
| H-152 | `autotulostusparam.lstf` vanhentunut alias tulostimen vaihdon jälkeen / stale alias | keskitaso | N-C5 |
| H-153 | Virhedialogin "Poistu"/"Ei" lukon haltijasta lukitsee GUI:n / exit choice deadlocks GUI | keskitaso | N-D2 |
| H-154 | Tulostimen DC poistetaan virheen jälkeen eikä avata uudelleen; tulostus loppuu hiljaa / printer DC lost | matala | N-C6 |
| H-155 | Pistokkeet vuotavat avausvirhepoluilla (TCP, UDP, hyväksytyt yhteydet) / socket leaks | matala | N-C3 |
| H-156 | Pysyvän datan pienet virhepolut (kokoomarivi) / minor persistent-data error paths | matala | N-B5–N-B11 |
| H-157 | Virhepolkujen ja lopetuksen pienet viat (kokoomarivi) / minor error-path and exit defects | matala | N-C4, N-C7, N-C8, N-D3–N-D7, D-4, E uusi 2 |
| T-040 | Virheet raportoidaan modaali-ikkunoilla työsäikeistä eikä paluuarvoina; ei uudelleenyritystä eikä yhteistä virhekäytäntöä / errors as dialogs, not return values | keskitaso | B, D |
| T-041 | Tiedostoja ei kirjoiteta atomisesti eikä kestävästi: tyhjennys paikallaan, ei väliaikaistiedostoa ja uudelleennimeämistä, ei `_commit`ia, ei tarkistussummaa / no atomic or durable writes | keskitaso | B |

Laajennukset / extensions: **H-044** (lukuvirhe lähettää edellisen paketin uudelleen ja kuittaus
merkitsee alkuperäisen lähetetyksi, N-D1); **H-071** (FILESEND välittää vajaan tiedoston, N-B7, myös
H-156:ssa); **H-105** (H-134-haaran jälkeen kortit kirjoitetaan suljettuun kahvaan; jo H-105:ssä).

Yksityiskohdat ja näyttö: `rekisteri/havainnot.md` ja osaraportit. / Details in the register and
the part reports.

## 8. Osan A korjaukset ja kumotut väitteet / Part A corrections and refuted claims

- `addem` (HK) ei palauta 1:tä kirjoitusvirheessä; `res = putem` -muunnos on määrittelemättömän
  `TALLEMIT`-makron alla (`Hk/HkEmit.cpp:1275-1377`). / HK `addem` returns 0 on failure.
- `writeaika` on kuollut molemmissa haaroissa. / `writeaika` is dead.
- `autofile`:n sarjakohtainen avausvirhe ei ole hiljainen: `openprfile` näyttää ilmoituksen
  (`tputilv2/Wrttext.cpp:171-176`) ja pysäyttää `LukumaaratTh`:n (GUI) tai `taustaThr`:n (konsoli)
  `autotul`-lukon alla. / Not silent; it blocks the background thread.
- Viestin `autofile` käännetään (`AUTOFILE` määritelty `RADStudio10/ViestiWin.cbproj:134` ja
  `VS/V/JukMaali520.vcxproj:97`); osan C päinvastainen väite kumottu. / Relay `autofile` is built.
- `luepakettiTCPSI` käännetään mutta sillä ei ole kutsujaa (ei `SIIMPORT`-ehtoa); `Hk/HkTV.cpp`
  ei ole missään projektissa ja `lah_tv`-kutsu on määrittelemättömän `TV1`:n alla
  (`Hk/HkDat.cpp:1231-1239`). / Dead code clarified.
- Kumottu tilaongelmina: D-2 `tee_aika`, D-3 `add`, D-17 `reconnectTCP`, D-18 `wrt_ch`; C-4 fontit
  (rajattu), C-11 `kirjSQLTh` (rajattu), vaiheen vaihdon `teejarj`-vuoto.

## 9. Avoimet kysymykset / Open questions

| Q | Kysymys / question | Liittyy / relates |
|---|---|---|
| Q83 | Palauttavatko C++Builderin `fputs`/`fputws` virheessä `EOF`:n vai `WEOF`:n? / bcc32 RTL return on error | H-147 |
| Q84 | Rajoittaako bcc32-RTL avoimet virrat 50:een (`_NFILE_`)? / stream limit | H-150 |
| Q85 | Miten MSVC- ja bcc32-keko reagoivat viestin `autofile`:n kaksoisvapautukseen? / heap reaction | H-149 |
| Q86 | Pitääkö lukija kuittausta lopullisena, ja palauttaako kortin uusi luku menetetyn kortin? / reader ACK semantics | H-145 |
| Q87 | Epäonnistuvatko verkkolevyn kahvan kirjoitukset pysyvästi katkon jälkeen vai yhdistääkö SMB uudelleen? / SMB handle after drop | H-146, H-015 |
| Q88 | Missä säikeessä viestin GUI ajaa `autofile`:n (`ViestiWin/UnitMain.cpp:520`)? / relay GUI thread | H-147 |
| Q89 | Saako työsäikeen konsoli-ilmoitukselle tarkoitettu näppäin perille, vai kuluttaako päälooppi sen? / console key routing | H-157 |
| Q90 | Ajetaanko kilpailuja muistipuskurilla (`levypusk` pois), jolloin lähettämättömät sanomat katoavat joka lopetuksessa? / memory-only outbuf in practice | luku 6 |

## 10. Kattavuus ja resurssit / Coverage and resources

Osan A luettelo kattaa kaikki 11 kohdehakemistoa (417 `.c`/`.cpp`-tiedostoa); 242 tiedostossa ei
ole rivejä, mikä on odotettua (`osaA-luokittelu.md` luku 5). Jokaisen pysyvän tiedoston
kirjoitusvirheen käsittely on kuvattu (luku 4), toistuvien toimintojen vuodot käyty läpi (luku 5),
uudet keskitason väitteet tarkastettu riippumattomasti (luku 7). Ei tarkastettu: H-155–H-157:n
matalat osat yksitellen, GUI-valikoiden `new TForm` -käsittelijät (U-vaihe), säikeiden `pros_bdg_t`,
`gazTh` ja `vahaku` konsoli-ilmoitukset. / Coverage: all 11 directories; every persistent file's
write-error handling described; repeated-operation leaks reviewed; medium claims reviewed. Not
covered: low items one by one, GUI menu form handlers, three threads' console prompts.

## 11. Vaihe 2 / Step 2 (ehdotus / proposal, erillisellä päätöksellä / on a separate decision)

Eristetyt koeohjelmat C-6/C-8-mallilla, toimitettua ohjelmaa ajamatta: (1) H-149 viestin
`autofile` kahdesti H-yksitiedostotilassa MSVC:n ja mahdollisuuksien mukaan bcc32:n keolla (Q85);
(2) H-145 `tall_emit` suljetulla emit-kahvalla; (3) H-146 `writeajat` epäonnistuvalla kahvalla;
(4) H-147 `WriteLine`:n paluuarvo täydellä levyllä (Q83); (5) H-150 virtojen määrä 600 kierroksen
jälkeen (Q84). / Isolated probes for H-149, H-145, H-146, H-147 and H-150.
