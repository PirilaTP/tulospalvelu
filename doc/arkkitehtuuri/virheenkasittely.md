# Virheenkäsittely ja resurssit / Error handling and resources (C-9 vaihe 1 / step 1)

Tila / status: vaihe 1 valmis ja tarkastettu (`f1eb195`); vaihe 2 (eristetyt koeohjelmat,
luku 12) valmis ja riippumattomasti tarkastettu, odottaa käyttäjän katselmointia ja
sulkemispäätöstä (2026-11-26). / Step 1 done (`f1eb195`); step 2 (isolated probes, section 12) done
and independently reviewed; awaiting the user's review and closing decision.

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

**Vaihe 2 (luku 12):** H-149 todennettiin ja nostettiin korkeaksi: konsolin viestiohjelmat
kaatuvat ensimmäiseen automaattiseen HTML-tulosteeseen tavallisessa konfiguraatiossa. H-145,
H-146, H-147, H-150 ja H-151 toistettiin testikehyksessä; uudet H-158 ja T-042. / Step 2: H-149
confirmed and raised to high; H-145, H-146, H-147, H-150, H-151 reproduced; new H-158, T-042.

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

## 11. Vaihe 2 / Step 2 (ehdotus vaiheen 1 lopussa; toteutus luvussa 12 / proposal at the end of step 1; done in section 12)

Eristetyt koeohjelmat C-6/C-8-mallilla, toimitettua ohjelmaa ajamatta: (1) H-149 viestin
`autofile` kahdesti H-yksitiedostotilassa MSVC:n ja mahdollisuuksien mukaan bcc32:n keolla (Q85);
(2) H-145 `tall_emit` suljetulla emit-kahvalla; (3) H-146 `writeajat` epäonnistuvalla kahvalla;
(4) H-147 `WriteLine`:n paluuarvo täydellä levyllä (Q83); (5) H-150 virtojen määrä 600 kierroksen
jälkeen (Q84). / Isolated probes for H-149, H-145, H-146, H-147 and H-150.

## 12. Vaihe 2: todentaminen eristetyillä koeohjelmilla / Step 2: verification with isolated probes (2026-11-26)

Käyttäjän hyväksymä vaihe 2. Koeohjelmat ovat C-6:n ja C-8:n mallia: projektit tuotetaan
`VS/Tests/TpCoreHk.vcxproj`:sta ja `TpCoreJuk.vcxproj`:sta (samat tuotantolähteet, määrittelyt ja
asetukset, Release ja Asan), käännetään repositorion ulkopuolelle, ja jokainen tapaus ajetaan omassa
hiekkalaatikossaan. Koeohjelma kutsuu muuttamatonta tuotantokoodia; vain globaali tila valmistellaan
ja kirjoitusvirhe injektoidaan tuotantokoodin ulkopuolelta. Toimitettuja ohjelmia ei ajettu, verkkoa
ei avattu, eikä `TPsource/`-puuhun tehty muutoksia. Lisäksi C++Builderin `bcc32`-ajonaikaiskirjaston
käytös mitattiin pienellä erillisellä ohjelmalla (ei tuotantokoodia). Todisteet:
`todisteet/c9/vaihe2/`; lopullinen ajo `koeohjelmat/tulokset/ajo5.txt` (27 tapausta), raakatulosteet
ja ASan-raportit `koeohjelmat/tulokset/raw/`.

Step 2, approved by the user: isolated probes in the C-6/C-8 pattern (projects generated from the
core test projects, built outside the repository, one sandboxed process per case), calling
unchanged production code with only global state prepared and the write failure injected from
outside. No shipped program, no network, nothing under `TPsource/` changed. A small bcc32 RTL
probe measured the C++Builder run-time library. Evidence in `todisteet/c9/vaihe2/`.

**Virheinjektiot / fault injection.** Emit-tiedosto ja AJAT-jono: tuotannon tiedostokuvaaja
vaihdetaan saman tiedoston vain luku -kuvaajaan, jolloin `_lseek` onnistuu ja `_write` palauttaa
−1 (sama paluupolku kuin levyn täyttyessä tai verkkolevyn kadotessa). Tulostiedostot: `TextFl`
avaa tiedoston CRT:n suojatussa jakotilassa, joten toista avausta ei voi tehdä; avauksen jälkeen
virran kuvaaja vaihdetaan (`_dup2`) sen oman kahvan vain luku -kopioon (`DuplicateHandle`), jolloin
stdio-puskurin huuhtelu epäonnistuu kuten levyn täyttyessä kirjoituksen aikana. / Emit and AJAT:
the fd is swapped for a read-only fd of the same file; result files: after the open the stream's fd
is replaced by a read-only duplicate of its own handle, so the stdio flush fails.

### 12.1 H-149: viestin automaattitiedosto / relay automatic output file — **korkea / high**

| Mekanismi / mechanism | Koodi / code | Todennus / verification |
|---|---|---|
| (a) yhden tiedoston tila: `closeprfile` vapauttaa `PRFILE`n nollaamatta osoitinta, `htmlloppu` kirjoittaa sen kautta | `Juk/VTulostus.cpp:7200`, `:7299` | ASan heap-use-after-free `writehtml` (`Juk/VTulostus.cpp:1366`), vapautus `:7200` |
| (b) T-tila: sama rakenne vapautetaan uudelleen | `:7321` | ASan double-free `closeprfile` (`tputilv2/Wrttext.cpp:297`) |
| (c) erilliset tiedostot ilman `AUTOFILE.LST`:ia: `delete aflst` kahdesti; lisäksi (a) ja `fileyht < 0`:lla (b) | `:7162-7164`, `:7295-7296` | ASan: käyttöoikeusvirhe `fclose`:ssa, kutsuttu `~TextFl` ← `autofile:7295` (`TextFl` on instrumentoimattomassa kirjastossa; todiste yhdessä koodin kanssa) |
| (d) X-tila palaa `lstf` vapautettuna; seuraava ajo lukee sen | `:7179-7185`, `:7150` | ASan heap-use-after-free `:7150` |
| N1 listatila + TCP-yhteys: `sendtcpfile` vapauttaa `lstf`:n, seuraava ajo sulkee sen uudelleen | `:7131`, `:7150`, `:7260-7262` | Release: prosessi päättyy `0xC0000374` (seuraus, ei mekanismin todiste; mekanismi koodista ja tarkastajan resepti) |

**Saavutettavuus / reachability.** Tavallinen konfiguraatio (`HTML=nimi` konsolissa tai GUI:n
automaattitiedostodialogi oletusvalinnalla "erilliset tiedostot") ilman `AUTOFILE.LST`:ia johtaa
polkuun (c) (`Juk/VInit.cpp:919-929`, `:2526`; `ViestiWin/UnitAutoFile.cpp:49`, `:68`). Polku (a)
syntyy konsolin valikosta, `FILETCP`-asetuksesta tai GUI:n valinnasta "yhteen tiedostoon".
`fileyht` saa arvon jo ensimmäisestä minkä tahansa TCP-tiedonsiirtoyhteyden avauksesta
(`Tp/TpComY.cpp:1221-1222`). Koodi käännetään ViestiMaaliin, ViestiLuentaan (MSVC) ja
ViestiWiniin (bcc32); `AUTOFILE` on määritelty kaikissa (`RADStudio10/ViestiWin.cbproj:134`,
`VS/V/JukMaali520.vcxproj:97`).

**Release-käytös (MSVC, `/MT` kuten tuotanto) / Release behaviour (Q85).**

| Tila / mode | `fileyht < 0` | `fileyht ≥ 0` (TCP-yhteys, ilman verkkoa / no network) |
|---|---|---|
| (c) erilliset ilman listaa (oletus) / separate, no list (default) | kaatuu 1. kierroksella `0xC0000374` | kaatuu 1. kierroksella `0xC0000374` |
| (a) yksi tiedosto / single file | kaatuu 1. kierroksella | ei kaadu 10 kierroksella, keko kunnossa; vapautetun muistin lukuja; tiedostoa ei siirretä TCP:llä [päätelty] |
| T | kaatuu 1. kierroksella | – |
| listatila / list mode | ei kaadu 30 kierroksella (kontrolli ei kuitenkaan kirjoita sarjoja) | N1: kaatuu toisella kierroksella `0xC0000374` |

C++Builderin RTL (erillinen konsolikoe, `-WCMR`): toinen `free` palaa hiljaa ja lohko annetaan
uudelleen käyttöön. ViestiWin on VCL-ohjelma ajonaikaisine paketteineen, joten sen muistinhallinta
voi olla eri; vaikutus ViestiWinissä on mittaamatta (Q91). / bcc32 RTL: a second free returns
silently; ViestiWin's VCL memory manager is unmeasured.

**Vakavuus korkea [todennettu: koeohjelma]:** konsolin viestiohjelmat (ViestiMaali, ViestiLuenta)
kaatuvat ensimmäiseen automaattiseen HTML-tulosteeseen tavallisessa konfiguraatiossa ilman
listatiedostoa riippumatta TCP-yhteydestä, ja listatiedostonkin kanssa toiseen tulosteeseen, jos
TCP-yhteys on auki (N1). Kaatuminen toistuu jokaisella käynnistyksellä niin kauan kuin
automaattitiedosto on päällä. GUI:ssa kaksoisvapautus ja vapautetun muistin käyttö toistuvat joka
kierros `LukumaaratTh`-säikeessä, vaikutus mittaamatta. Tallennettu kilpailudata (`KILP.DAT`) ei
korruptoidu tämän vuoksi, mutta ohjelma pysähtyy kesken kilpailun. / High: the console relay
programs crash at the first automatic HTML output in the common configuration without a list file,
with or without a TCP link, and at the second one with a list file and a TCP link (N1); repeats
on every start while the feature is on. GUI: double free and use after free every cycle, effect
unmeasured.

### 12.2 H-145: emit-kortin tallennusvirhe / emit card write failure — keskitaso / medium

Toistettu testikehyksessä (`tall_emit` MTR-tietueella, emit-kuvaaja vain luku -tilassa):

| Vaihe / step | Tulos / result |
|---|---|
| kortin vastaanotto / card received | `tall_emit` → `addem` → `putem` → `_write` −1; yksi `wperr`-ilmoitus (operaatiokoodi 3, H-156) |
| muisti / memory | kortti `ed_em`-välimuistissa ja `on_emit`-taulussa ennen kirjoitusta; `emithead` ei kasva |
| kuittaus / acknowledgement | `tall_emit` palauttaa 0 aina; Emit-kellolla tästä seuraa kuittaus (`Tp/TpLaitteet.cpp:911-917`), muilla lukijoilla sanoma kulutetaan joka tapauksessa (MTR-pakettilaskuri etenee `:1247-1255`) |
| saman kortin uusi luku / same card again | vain vahvistus (`vahvistus` = korttinumero), ei tallennusta |
| toinen kortti välissä / another card in between | oletuksella `estaemittoisto = 1` kortti hylätään kaksoiskappaleena eikä tallennu koskaan saman istunnon aikana; arvolla 0 se tallentuu |
| muut työasemat / peers | `pros_emit` käsittelee vain `emithead2..emithead`, joten korttia ei yhdistetä kilpailijaan eikä lähetetä (`laheta_em` vain onnistuneelle) [todennettu: koodi] |
| uudelleenkäynnistys / restart | korttia ei ole tiedostossa; välimuistit ovat muistissa, joten uudelleenkäynnistyksen jälkeen kortin uusi luku tallentaa sen |

Johtopäätös: menetys ei ole hiljainen (ilmoitus), mutta tavallinen uudelleenluku ei palauta korttia
saman istunnon aikana; palautus vaatii käsin syötön tai uudelleenkäynnistyksen ja uuden luvun. /
Not silent, but an ordinary re-read does not recover the card within the session; recovery needs
manual entry or a restart and a re-read.

### 12.3 H-146: AJAT-jonon aukko / AJAT queue hole — keskitaso / medium

Toistettu testikehyksessä (`tall_rivi` → `writeajat`, aloitus `initajat(0, 1)` = oletus-
käynnistys, `ajanottofl = -1`; arvolla 1 kutsuja täyttää rivit samalla paikkamerkillä, joten
tilat ovat samat): epäonnistuneella rivillä `fp1` 2 → 3, ilmoitus annetaan; seuraavat rivit
kirjoittuvat oikeisiin paikkoihinsa, mutta epäonnistuneen rivin paikka jää käynnistyksessä
kirjoitetuksi paikkamerkiksi (`t = TMAALI0*AIKAJAK`, `kno = 0`). Uudelleenkäynnistyksessä rivi
puuttuu, ja jonon keskelle jää tyhjä rivi, jonka aika on pienin mahdollinen; myöhempi aika
lisätään sen jälkeen. Aukko korjautuu vain, jos rivi tai aiempi rivi muuttuu myöhemmin
(`fp1` lasketaan) tai vaihe vaihtuu. Vaikutus tuloksiin [päätelty]: kilpailijalle jo kirjatut
ajat ovat `KILP.DAT`:ssa erillisen polun kautta, joten menetys koskee AJAT-jonon riviä ja sen
kohdistamatonta aikaa (Q94). / fp1 advances on the failed row; the slot keeps the start-up
placeholder; after a restart the row is missing and an empty row stays inside the queue.

### 12.4 H-147: tulostiedostot / result files — keskitaso / medium

| Väite / claim | Tulos / result |
|---|---|
| `WriteLine == WEOF` -tarkistus | MSVC: epäonnistunut huuhtelu palauttaa −1 tyypeillä A, W ja 8 (WEOF = 65535); `puts_f`/`writeline` eivät ilmoita mitään; tyyppi V palauttaa merkkimäärän. bcc32-RTL: myös `fputws` palauttaa −1 → tarkistus on kuollut kaikissa viidessä tuotteessa (Q83) |
| tiedoston tyhjennys / truncation | `autofile`:n avaus (`openprfile(..., 2, ...)`, `Hk/HkTulostus.cpp:7108`) tyhjentää tiedoston heti (0 tavua ennen ensimmäistä kirjoitusta) |
| epäonnistunut kirjoitus / failed write | tiedosto jää tyhjäksi (huuhtelu epäonnistui ensimmäisellä kerralla) tai yleisesti osittaiseksi; ei ilmoitusta; vanha ehjä tiedosto on menetetty |
| lukijat generoinnin aikana / readers | MSVC: lukija saa jakorikkomuksen (virhe 32), ei osittaista sisältöä; epäonnistuneen ajon jälkeen lukija näkee tyhjän tai osittaisen tiedoston seuraavaan onnistuneeseen ajoon asti. bcc32: avaustila mittaamatta (Q92) |
| lukija pitää tiedostoa auki / a reader holds the file | `autofile`:n avaus epäonnistuu: kaksi ilmoitusta (`openprfile` ja `autofile`, jälkimmäinen 2 s), vanha tiedosto säilyy; ilmoitukset pysäyttävät taustasäikeen `autotul`-lukon alla |

Kilpailutiedon eheys muistissa ja `KILP.DAT`:ssa ei riipu tulostiedostosta; vaikutus rajoittuu
julkaistuun tulokseen, joka voi olla tyhjä, osittainen tai vanha ilman merkintää. / In-memory and
KILP.DAT data are unaffected; the published file can be empty, partial or stale without notice.

Korjaus vaiheen 1 tekstiin: vaiheen 1 väite "lukija näkee tyhjän tai osittaisen tiedoston
generoinnin aikana" ei päde MSVC-ohjelmissa (jakorikkomus); se pätee epäonnistuneen ajon jälkeen.
Tarkastajan maininta `tputilv2/Wrttext.cpp:447`:stä ei ole samaa vikaa: `fputwc` palauttaa
virheessä `WEOF`:in, joten se vertailu toimii.

### 12.5 Resurssit / resources: H-150, H-151, H-152

- **H-150 (keskitaso / medium) toistettu:** XML-automaattivienti vaiheessa ≥ 2 vuotaa joka
  kierroksella `TextFl`-olion (86 tavua keosta). Jos `AUTOFILE.LST` on olemassa, myös yksi
  CRT-virta ja yksi ytimen kahva: MSVC:ssä virrat täyttyivät 511/512:een noin 508 kierroksessa
  (60 s välillä noin 8,5 h), minkä jälkeen kaikilla muilla samanaikaisilla avauksilla on käytössä
  enintään yksi vapaa paikka; itse vienti jatkoi. bcc32-RTL:ssä virtoja on 50 (47 + 3 vakiovirtaa),
  joten HkKisaWinissä raja tulee alle 45 kierroksessa (alle 45 min). Seuraukset GUI:ssa
  (ilmoitukset joka kierros, emit-tiedoston `_wfdopen` NULL seuraavassa vaiheen vaihdossa) ovat
  päättelyä (Q95). Ilman `AUTOFILE.LST`:ia vuoto on noin 124 kt vuorokaudessa 60 s välillä, ei
  käytännön merkitystä. Vakavuus pysyy keskitasona kapean konfiguraation vuoksi; nousee korkeaksi,
  jos GUI-ajo osoittaa tietojen katoavan.
- **H-151 (keskitaso / medium) toistettu ASanilla:** `sendtcpfile`-säie vapauttaa
  `aftulparam.lstf`:n, ja seuraava `autofile` lukee sen (`Hk/HkTulostus.cpp:7093`). Koe tehtiin
  ilman verkkoa (`hComm` 0, `wrt_st_TCP` palaa heti, `com/WinUDP.cpp:903-904`); vapautus tapahtuu
  lähetyksen tuloksesta riippumatta, joten asetelma edustaa todellista yhteyttä. Mikä tahansa
  TCP-yhteys riittää.
- **H-152 siirretty myöhempään testausvaiheeseen / deferred:** todentaminen vaatii GUI-käsittelijän
  (`cbHk/UnitStatus.cpp:698-706`) ja tulostettavat tulokset; tarkastaja yhtyy.

### 12.6 Inventaarioskriptin kattavuus / inventory script coverage

| | |
|---|---|
| Korjattu v2:ssa / fixed in v2 | S1 (ehdoton `if`/`for` ei ole tarkistus), S2 (`openfile`/`makefile` palauttavat arvon; `putrec_i`, `addrec0`), S3 (tarkistusikkuna lauseen lopusta, uudelleensijoitus katkaisee), S5 (VCL-olioiden `Open`/`Close` pois), S6 (määrittelyt ja esittelyt pois), S7 (`built`-sarake), S8 (käärefunktiot `openprfile`/`closeprfile`, `_wfdopen`, `_chsize`, `_wfopen_s`, `CopyFile`, `system`, `lopetus`, `perr`/`wperr`/`iowerr`) |
| Korjaamatta / not fixed | S4 epäsuora tarkistus (tavumäärä ulostuloparametrissa, kertyvä paluuarvo) – vaatisi funktiokohtaisen mallin; S9 `new`-tulosten luokittelu – `new` heittää poikkeuksen, ja `TextFl`-avaukset käytiin läpi käsin (180/186 tarkistetaan); S10 esikääntäjä – vaikutus mitattu mitättömäksi; S11 varamenetelmän funktiokohdistus – osoittautui oikeaksi otoksessa; S12 kuollut koodi – vaatisi tietovuoanalyysin. Nämä ovat luokittelun tarkkuusrajoja, eivät puuttuvia kohteita |
| Määrät / counts | v1 3 130, v2 3 234 riviä; yhteisiä 3 019; vain v1:ssä 111 (82 VCL-olion `Open`/`Close`, 29 määrittelyä); vain v2:ssa 215 (uudet nimet) |
| Puuttuneet kohteet / missing sites | v2:n uusista riveistä 22 tarkistamatonta ydinriviä ei mainittu missään raportissa. Pääistunto ja tarkastaja lukivat ne: 3 × `putrec_i` (H-015:n malli), 3 × `lopetus` (käynnistys; konsolin J/P-kysely `tark_alku`ssa tietoliikennesäikeessä, H-153:n perhettä), 16 × `closeprfile` (15 harmitonta; `Juk/VTulostus.cpp:7131` on `sendtcpfile` → N1, liitetty H-149:ään) |
| Vaikutus osiin B ja C / effect on parts B and C | 67 riviä siirtyi "checked" → "ignored" (S1). Ydinkirjoitukset niistä: `fflush` (H-156), käynnistyksen varaukset, `teejarj` (H-157), `DBG`-vianetsintätiedostot (`DBG` ei määritelty missään). Agentit lukivat koodin suoraan eivätkä nojanneet luokitukseen, joten johtopäätökset eivät muuttuneet; ainoa uusi asia oli N1 |

Riippumattoman tarkastuksen lukumäärät (vaihe 1, osa E) täsmäytettynä: 19 väitettä, joista
kohta 19 jaettiin kolmeksi (19a–c) → 21 riviä. Vahvistettu 16 (1, 2, 5, 6, 7, 8, 9, 11, 12, 13,
14, 15, 17, 18, 19b, 19c), osittain 5 (3, 4, 10, 16, 19a). Osaväitteet: kohdan 7 osa "viestin
`autofile` ei käänny" kumottiin (kohta itse vahvistettiin HK:lle); kohdan 12 osa "ensimmäisestä
virheestä tulee ilmoitus" oli väärin; kohdat 13 ja 17 vahvistettiin mutta todettiin H-044:n ja
H-105:n kaksoiskappaleiksi. / Step 1 review reconciled: 19 claims, item 19 split into three → 21
rows; 16 confirmed, 5 partly; sub-claims: item 7's relay part refuted, item 12's message wording
wrong; items 13 and 17 duplicates.

### 12.7 Palautuminen kriittisten tietojen kirjoitusvirheistä / Recovery from write failures of critical data

| | `KILP.DAT` | emit-tiedosto / emit file | AJAT-jono / queue | julkaistava tulostiedosto / published result file |
|---|---|---|---|---|
| Havaitaanko / detected | kyllä (tavumäärä) | kyllä | kyllä | ei (H-147); vain avausvirhe |
| Välittyykö kutsujalle / reported to caller | ei (−1 ohitetaan, `write` palauttaa 0; H-015) | ei (`tall_emit` palauttaa 0; H-145) | ei (`writeajat` void, `tall_rivi`n tulos ohitetaan; H-117) | ei |
| Ilmoitus / operator told | modaali Kyllä/Ei, "Ei" lopettaa (H-153) | modaali, väärä virhekoodi (H-156) | modaali | ei; avausvirheessä kaksi ilmoitusta |
| Uudelleenyritys / retry | ei | ei | ei (osoitin jo edennyt; H-146) | seuraava kierros |
| Tieto muistissa / in memory | kyllä (muisti ensin) | vain välimuistissa, ei käsittelyssä | kyllä | kilpailudata kyllä |
| Tieto levyllä / on disk | vanha tietue; lisäyksessä otsikko voi olla ristiriidassa (H-148) | puuttuu | aukko (paikkamerkki) | tyhjä tai osittainen; vanha menetetty |
| Onnistumiskuittaus virheestä huolimatta / success acknowledged anyway | – (muisti päivitetty, lähetys muille tehty) | kyllä: Emit-kellon kuittaus; muut lukijat kuluttavat sanoman | – | ohjelma ei erota onnistumista epäonnistumisesta; `autokomento` ajetaan |
| Palautettavissa / recoverable | sama tietue uudelleen tallennettaessa; vienti "DAT" muistista | käsin syöttö tai uudelleenkäynnistys + uusi luku | aiemman rivin muutos tai vaiheen vaihto | seuraava onnistunut kierros (paitsi `vainMuutt`) |
| Uudelleenkäynnistys / restart | lataa vanhan tietueen; otsikkoristiriita estää käynnistyksen | korttia ei ole | rivi puuttuu, tyhjä rivi keskellä | tiedosto kirjoitetaan seuraavalla kierroksella |
| Näyttö / evidence | koodi (osa B), H-015, H-148 | koeohjelma | koeohjelma | koeohjelma |

### 12.8 Muutokset rekisteriin / register changes

- **H-149** keskitaso → **korkea**; mekanismit (a)–(d) toistettu ASanilla, Release-kaatuminen
  mitattu; saavutettavuus korjattu (oletuspolku (c)); N1 liitetty (tarkastajan löytö, toistettu
  Release-seurauksena).
- **H-145, H-146, H-147, H-150, H-151:** todennus "toistettu testikehyksessä"; tekstit tarkennettu
  (H-145: kuittaus vain Emit-kellolla, menetys istunnon ajaksi; H-147: kaikki tyypit ja molemmat
  kääntäjät).
- **Uusi H-158 (matala):** viestin automaattinen XML-tiedosto sisältää vain loppuosan, koska
  `writexml` on kommentoitu pois (`Juk/VTulostus.cpp:7179-7181`). Tarkastajan sivuhavainto,
  pääistunto luki koodin.
- **Uusi T-042:** `closeprfile` ottaa osoittimen arvona eikä voi nollata kutsujan kenttää; kutsujat
  nollaavat sen tapauskohtaisesti tai eivät lainkaan (H-149, H-151, H-152, N1).

### 12.9 Korjausjärjestys / fix priority [päätelty / inferred]

P2 (kiireellinen) **H-149** (konsolin viestiohjelmat kaatuvat oletuskonfiguraatiossa). P3 H-145,
H-146, H-151, H-147, H-150. P4 H-148, H-152, H-153, H-154, H-155. P5 H-156, H-157, H-158. Korjaukset
tehdään omissa haaroissaan erillisellä päätöksellä; mitään ei ole aloitettu. / P2 H-149; P3 H-145,
H-146, H-151, H-147, H-150; P4 H-148, H-152–H-155; P5 H-156–H-158. No fix started.

### 12.10 Kysymykset / questions

Ratkaistu / resolved: **Q83** (bcc32 `fputws` palauttaa −1, ei `WEOF`:ia), **Q84** (bcc32-RTL:ssä
50 virtaa; GUI:n vakiotilan virtamäärä Q95:ssä), **Q85** MSVC:n osalta (STATUS_HEAP_CORRUPTION
ensimmäisellä kierroksella; bcc32-RTL:n pelkkä `free` hiljainen; VCL-muistinhallinta Q91:ssä),
**Q88** (viestin GUI ajaa `autofile`:n `LukumaaratTh`-työsäikeessä, `ViestiWin/UnitMain.cpp:535-541`).
Kavennettu / narrowed: **Q86** (kuittaus koskee vain Emit-kelloa; laitteen tulkinta avoin).
Uudet / new: **Q91** ViestiWinin VCL-muistinhallinnan reaktio H-149:n kaksoisvapautukseen;
**Q92** avaako bcc32-RTL:n `_wfopen_s` kirjoitusvirran yksinoikeudella; **Q93** rikkooko
`sendtcpfile`n STX/tiedosto/ETX-virta vastapään pakettijäsennyksen, kun `fileyht` on tavallinen
tiedonsiirtoyhteys; **Q94** käytetäänkö AJAT-riviä koskaan tulosten uudelleenrakentamiseen
(H-146); **Q95** HkKisaWinin virtamäärä vakiotilassa ja `femitfile`-NULLin vaikutus vaiheen
vaihdossa (H-150).

### 12.11 Sulkemisarvio / closing assessment

Vaiheen 2 tavoitteet täyttyivät: H-149 todennettu ja vakavuus nostettu, H-145–H-147 todennettu
virheinjektiolla, H-150:n vuoto mitattu, H-151 toistettu, H-152 perustellusti siirretty,
inventaarion erotus käyty läpi, palautumisen kokonaiskuva kirjattu, uudet väitteet tarkastettu
riippumattomasti ja validaattori menee läpi. Avoimet asiat (Q86, Q91–Q95, H-152:n ajo) vaativat
GUI-ajon, VCL-koeohjelman tai laitteen eivätkä estä sulkemista. Suositus: **C-9 voidaan sulkea**
käyttäjän hyväksynnällä; avoimet asiat siirretään jatkotehtäviksi (`seuranta.md`). / Step 2 goals
met; open items need a GUI run, a VCL probe or hardware and do not block closing. Recommendation:
C-9 can be closed on the user's approval, with the open items moved to follow-ups.
