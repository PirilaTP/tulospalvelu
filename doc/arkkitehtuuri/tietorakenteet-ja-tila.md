# Tietorakenteet ja tila / Data structures and state

Polut ovat suhteessa hakemistoon `TPsource/V52/`.
Paths are relative to `TPsource/V52/`.

## Keskeiset rakenteet / Core structures

Samannimiset luokat on määritelty erikseen HK:lle ja viestille, ja ne ovat
sisällöltään erilaisia. Yhteinen koodi (`Tp/`, `cbTp/`) näkee niistä sen, joka
otetaan mukaan käännöksessä (ks. `yleiskuva.md`). [todennettu: määrittelyrivit]

Same-named classes are defined separately for HK and relay, and their contents
differ. Shared code (`Tp/`, `cbTp/`) sees whichever one is included in the build
(see `yleiskuva.md`). [verified: definition lines]

| Luokka / Class | HK | Viesti / Relay | Sisältö / Content |
|---|---|---|---|
| `kilptietue` | `Hk/HkDef.h:496` | `Juk/VDef.h:734` | Kilpailija (HK) tai joukkue (viesti); nimet, seura, sarja, numero, tila / competitor (HK) or team (relay) |
| `kilppvtp` | `Hk/HkDef.h:440` | – | HK: yhden päivän/vaiheen tiedot (lähtö, emit-koodit, sakot, väliajat `va`) / one day or stage |
| `osrec` | – | `Juk/VDef.h:662` | Viesti: yhden osuuden juoksija ja ajat / relay leg: runner and times |
| `vatp` | `Hk/HkDef.h:402` | `Juk/VDef.h:644` | Väliaika: tulos, sija, koodi / split: result, rank, code |
| `sarjatietue` | `Hk/HkDef.h:687` | `Juk/VDef.h:376` | Sarja: väliaikapisteet, lähtöajat, maksut / class |
| `tulosmuottp` | `Hk/HkDef.h:242` | `Juk/VDef.h:464` | Tulosteen muotoilu / output formatting |
| `tulostusparamtp` | `Hk/HkDef.h:312` | `Juk/VDef.h:535` | Tulostuksen parametrit / print job parameters |
| `aikajonotp` | `Hk/HkDef.h:951` | `Juk/VDef.h:1044` | Ajanoton aikajono / timing queue |
| `emittp` | `Hk/HkDef.h:1192` | `Juk/VDef.h:1094` | Luettu emit-kortti / read emit card |
| `IOFCourse*`, `IOFPoint` | `Hk/HkDef.h:1052-1164` | `Juk/VDef.h:876-967` | Radat IOF-muodossa / courses in IOF form |
| `seuraTieto`, `seuraLuettelo` | `Hk/HkDef.h:878-891` | `Juk/VDef.h:431-443` | Seurat / clubs |
| `kaavatp` | `Hk/HkDef.h:633` | – | Pistekaava / points formula |
| `TextFl` | `include/tputil.h:124` | (yhteinen / shared) | Tekstitiedoston luku ja kirjoitus / text file I/O |
| `datafile` | `include/boxr.h` | (yhteinen / shared) | dbboxm-tietuetiedosto / record file |

Merkkijonot: HK:n `kilptietue` käyttää leveitä merkkejä (`wchar_t`), viestin
`osrec` kapeita (`char`, esim. `osrec::nimi`). Tämä selittää monet viestikoodin
`swprintf("%s", char*)`-virheet (`issue-drafts/03`). [todennettu: kentät;
päätelty: yhteys virheisiin]

Strings: HK's `kilptietue` uses wide characters (`wchar_t`), relay `osrec` uses
narrow ones (`char`, e.g. `osrec::nimi`). This explains many of the
`swprintf("%s", char*)` errors in the relay code (`issue-drafts/03`). [verified:
fields; inferred: link to the errors]

## Kilpailijatietojen elinkaari / Competitor data life cycle

| Vaihe / Step | Funktio / Function | Paikka / Location |
|---|---|---|
| Lataus levyltä muistiin / load from disk | `teejarj()` | `Hk/HkIx.cpp:1554` |
| Luku muistista / read from memory | `kilptietue::GETREC(d)` (kopio `kilparr[d]`:sta / copy of `kilparr[d]`) | `Hk/HkDat.cpp:597` |
| Muutos / change | `kilptietue::tallenna(...)` | `Hk/HkDat.cpp:897` |
| Kirjoitus muistiin ja levylle / write to memory and disk | `kilptietue::PUTREC(d, tallenna)` → `write(d, 0)` (KILP.DAT) ja / and `write(d, 1)` (SQL) | `Hk/HkDat.cpp:609`, `:581` |
| Lisäys / add | `kilptietue::addtall()`, `addfrec()` | `Hk/HkDat.cpp:674`, `:626` |
| Poisto / delete | `delfrec(d)` | `Hk/HkDat.cpp:640` |
| Lähetys muille / send to others | `laheta(...)` → `buflah(cn, &obuf)` | `Hk/HkCom32.cpp:167`, `Tp/TpComY.cpp:476` |

Tietueen numero `d` on sen paikka `KILP.DAT`-tiedostossa ja `kilparr`-taulukossa.
Kilpailijanumero (`kilpno`, `id()`) on eri asia; niiden välillä on indeksi
(`getpos`). [todennettu: `Hk/HkDat.cpp`; päätelty: indeksin toteutus]

The record number `d` is its position in `KILP.DAT` and in the `kilparr` array.
The bib number (`kilpno`, `id()`) is a different thing; an index maps between
them (`getpos`). [verified: `Hk/HkDat.cpp`; inferred: index implementation]

## Globaali tila / Global state

Lähes kaikki tila on globaaleissa muuttujissa. Määrittelyt ovat tiedostoissa
`Hk/HkGlobals.cpp` ja `Juk/VGlobals.cpp`, ja niitä vastaavat `extern`-esittelyt
otsikoissa:

Almost all state is in global variables. They are defined in `Hk/HkGlobals.cpp`
and `Juk/VGlobals.cpp`, with matching `extern` declarations in headers:

| Otsikko / Header | `extern`-rivejä / lines |
|---|--:|
| `Hk/HkDeclare.h` | 453 |
| `Juk/VDeclare.h` | 396 |
| `Hk/HkMuotoilu.h` | 51 |
| `Juk/VMuotoilu.h` | 42 |
| `cbHk/HkWinDef.h` | 20 |
| `include/TpLaitteet.h` | 16 |

Tärkeimpiä / Most important [todennettu: esittelyt; päätelty: roolit]:

- `kilparr`, `nrec`, `maxrec` – kaikki kilpailijat muistissa / all competitors in memory
- `datf2` – `KILP.DAT`-tiedoston kahva / file handle
- `Sarjat[]`, `sarjaluku` – sarjat / classes
- `kilpparam`, `pvparam[]` – kilpailun ja vaiheiden parametrit / competition and stage parameters
- `k_pv` – aktiivinen päivä tai vaihe / active day or stage
- `ToimintaTila` – 0 = ei avattu, 1 = esivalmistelu, 2 = tulospalvelu / 0 = not open, 1 = preparation, 2 = results service (`Hk/HkGlobals.cpp:189`)
- `comopen[]`, `comtype[]`, `lahcom[]`, `outbuf[]`, `inbuf` – yhteydet ja jonot / connections and queues
- `nameindex` – nimihakemisto / name index

## Säikeet / Threads

Säikeet käynnistetään funktioilla `_beginthread` tai `CreateThread`.
[todennettu: käynnistyskohdat; tuntematon: tarkka elinikä ja lopetus]

Threads are started with `_beginthread` or `CreateThread`. [verified: start sites;
unknown: exact lifetime and shutdown]

| Säie / Thread | Käynnistetään / Started in | Tehtävä / Task [päätelty / inferred] |
|---|---|---|
| `tiedonsiirto`, `tiedonsiirtoUDP`, `tiedonsiirtoTCP` | `Tp/TpComY.cpp` | Yhteyskohtainen lähetys ja vastaanotto / per-connection send and receive |
| `luepakettiUDP`, `luepakettiTCP`, `tarkcom` | `Tp/TpComY.cpp` | Pakettien luku ja tarkistus / packet reading and checking |
| `TCPacceptTh`, `TCPrcvTh`, `UDPstreamrcvTh` | `com/WinUDP.cpp` | Socket-tason kuuntelu / socket-level listening |
| `lue_regnlyThread` | `Tp/TpLaitteet.cpp`, `Tp/TpComY.cpp` | Laitteiden (ajanotto, lukijat) luku / reading devices |
| `comajanotto` | `Hk/HkIV.cpp`, `Juk/VIv.cpp` | Ajanottoyhteys / timing connection |
| `gazTh` | `Hk/HkIV.cpp` | GAZ-tulostaulu / scoreboard |
| `file2emitThread` | `Juk/VIv.cpp` | Emit-tietojen luku tiedostosta / emit data from file |
| `taustaThr` | `Hk/HkInit.cpp`, `Juk/VInit.cpp` | Taustatehtävät / background tasks |
| `sendtcpfile` | `Hk/HkTulostus.cpp`, `Juk/VTulostus.cpp` | Tiedoston lähetys TCP:llä / file sending over TCP |
| `SendCopyThread` | `cbHk/UnitComfile.cpp`, `HkConsole/HkCons0.cpp`, `JukConsole/laskent0.cpp`, `ViestiWin/UnitComfile.cpp` | Kopioiden lähetys / sending copies |
| `ProsEmitThread`, `LukumaaratTh` | `cbHk/WinHk.cpp`, `ViestiWin/UnitMain.cpp` | Emit-käsittely, lukumäärät / emit processing, counts |
| `DisplTimerThread` | `cbHk/UnitEmit.cpp`, `ViestiWin/UnitEmit.cpp` | Näytön päivitys / display refresh |
| `kirjSQLTh`, `haeSQLTh` | `cbHk/UnitMySQL.cpp`, `ViestiWin/UnitMySQL.cpp`, `cbTp/UnitEmitSQL.cpp` | SQL-kirjoitus ja -haku / SQL write and fetch |

Lisäksi GUI:ssa on muita säikeitä (`ScrollThread`, `PaivThread`, `HaeAvoimetTh`,
`naytatulosTh`, `naytaluentaTh`, `SelTimerThread`, `NaytaTulosTimerThread`).
The GUI also has other threads (listed above).

## Synkronointi / Synchronisation

Jaettua tilaa suojaavat nimetyt `CRITICAL_SECTION`it, jotka alustetaan
`aloitus()`-funktiossa (`Hk/HkInit.cpp:3099-3110`) ja GUI-lomakkeissa:

Shared state is protected by named `CRITICAL_SECTION`s initialised in `aloitus()`
(`Hk/HkInit.cpp:3099-3110`) and in GUI forms:

`kilpdat` (kilparr ja KILP.DAT), `tall`, `idindex`, `ajat`, `emit`, `rastiva`,
`inb`/`outb` (tiedonsiirron jonot / comms queues), `tcpip`, `tcp`, `seura`,
`seur_badge`, `loki`, `print`, `autotul`, `prof`, `tls`, `SQL`, `sakkoGrid`,
`sakkoKierr`.

Havainto: `kilptietue::tallenna()`:n alussa on kommentoitu rivi
`// EnterCriticalSection(&tall_CriticalSection);` (`Hk/HkDat.cpp:941`), eli funktio ei
itse lukitse. Osa kutsujista lukitsee (`cbHk/HkKisaLahtLuet.cpp:381` ym.), ja sitä
kutsutaan myös tiedonsiirrosta (`Hk/HkCom32.cpp:331`, `:395`). Jos kutsuja on
useasta säikeestä samanaikaisesti, tallennukset voivat mennä limittäin. On
selvitettävä, missä säikeessä tiedonsiirron käsittely ajetaan. [todennettu: rivit;
tuntematon: säikeistys → `seuranta.md`]

Observation: `kilptietue::tallenna()` has, near its start, the commented-out line
`// EnterCriticalSection(&tall_CriticalSection);` (`Hk/HkDat.cpp:941`), so the function
does not lock by itself. Some callers lock (`cbHk/HkKisaLahtLuet.cpp:381` and
others), and it is also called from communications (`Hk/HkCom32.cpp:331`, `:395`).
If it is called from several threads at once, saves may interleave. It must be
established which thread runs the communications handling. [verified: lines;
unknown: threading → `seuranta.md`]

## Levymuoto ja yhteensopivuus / Disk format and compatibility

`KILP.DAT` sisältää `kilptietue::pack()`-funktion tuottamat tavut, ja tietueen
koko on `kilpparam.kilprecsize`. Vanhan muodon lukemiseen on erillinen koodi
(`Hk/HkInitVanha.cpp`, `kilpparam_v`, `datf2v`). Jos tietueeseen lisätään kenttiä,
on tarkistettava sekä `pack`/`unpack` että vanhan muodon muunnos. Tiedonsiirron
paketit käyttävät samoja `pack`-funktioita (`Hk/HkCom32.cpp:198-216`), joten
muutos vaikuttaa myös eri versioiden väliseen tiedonsiirtoon. [todennettu:
kutsut; päätelty: seuraukset]

`KILP.DAT` holds the bytes produced by `kilptietue::pack()`, and the record size
is `kilpparam.kilprecsize`. There is separate code for reading the old format
(`Hk/HkInitVanha.cpp`, `kilpparam_v`, `datf2v`). If fields are added to the
record, both `pack`/`unpack` and the old-format conversion must be checked.
Communication packets use the same `pack` functions (`Hk/HkCom32.cpp:198-216`),
so the change also affects communication between different versions. [verified:
calls; inferred: consequences]
