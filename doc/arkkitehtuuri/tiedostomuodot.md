# Pysyvät tiedostot ja eheys / Persistent files and integrity

Syväauditoinnin erä C-3. Revisio `f7fe553`. Polut suhteessa `TPsource/V52/`.
Staattinen analyysi ja käännösaikaiset koettimet (`sizeof`/`offsetof` MSVC:llä ja
bcc32:lla väliaikaishakemistossa); varsinaisia ohjelmia ei ajettu eikä
kilpailutietoja käsitelty. Jokainen merkittävä väite on käyty läpi riippumattomasti
(`independent-verifier`); "VAHVISTETTU" tarkoittaa riippumatonta lähdetarkastusta,
ei ajonaikaista todennusta. Havaintonumerot: `rekisteri/havainnot.md`. Aiemmat erät:
`tallenna-samanaikaisuus.md`, `tiedonsiirto.md`.

Deep audit batch C-3. Revision `f7fe553`. Paths relative to `TPsource/V52/`.
Static analysis plus compile-time probes (`sizeof`/`offsetof` with MSVC and bcc32
in a scratch directory); the real programs were not run and no competition data
was touched. Every significant claim was reviewed independently
(`independent-verifier`); "VAHVISTETTU" (confirmed) means an independent source
review, not runtime verification. Finding numbers: `rekisteri/havainnot.md`.
Previous batches: `tallenna-samanaikaisuus.md`, `tiedonsiirto.md`.

## Johtopäätös / Conclusion

Tiedostot ovat yksinkertaisia kiinteän tietuekoon taulukoita. Niissä ei ole
tunnistetta, versiota, tallennettua tietuepituutta eikä tarkistussummaa. Ainoa
eheystarkistus on `KILP.DAT`:n otsakkeen tietuemäärän ja tiedoston pituuden vertailu.
Tietuekoko riippuu kilpailun asetuksista (vaiheiden, osuuksien ja väliaikojen
määrä), joten saman ohjelman eri kilpailujen tiedostot eivät ole keskenään
vaihdettavissa, eikä mikään tunnista vanhaa muotoa automaattisesti (Q4 ratkaistu).
[todennettu, tarkastettu]

MSVC- ja bcc32-käännökset sopivat yhteen: kaikkien levylle ja verkkoon raakana
kirjoitettavien rakenteiden koko ja kenttien sijainnit ovat mitattuina samat, koska
ne ovat `#pragma pack(push,1)` -alueella (Q17 ja Q18 ratkaistu). HK ja viesti
eivät sen sijaan voi lukea toistensa tiedostoja.

Yhden prosessin sisällä `KILP.DAT` on pääosin hyvin suojattu: kirjoitus menee
suoraan levylle (`FILE_FLAG_WRITE_THROUGH`), toinen ohjelma ei voi avata samaa
tiedostoa kirjoitettavaksi, ja kaatuminen datan ja otsakkeen kirjoituksen välissä
siedetään. Heikot kohdat ovat (1) **palautuminen**: mikä tahansa otsakkeen ja
pituuden ristiriita estää käynnistyksen eikä korjaustyökalua ole (H-015
laajennus); (2) **kokorajat**: toimitetuissa käännöksissä tietue voi olla
puskuria suurempi (H-041, H-042); (3) **lukitus**: viestin `addfrec` ei vapauta
lukkoaan (H-040), ja useampi polku kirjoittaa tiedostoihin ilman lukkoa (H-045,
H-047, H-051).

Files are simple fixed-size record arrays with no magic, version, stored record
length or checksum. The only integrity check compares the `KILP.DAT` header's
record count with the file length. Record size depends on competition settings
(stages, legs, splits), so files of different competitions are not
interchangeable, and nothing detects an old format automatically (Q4 resolved).

MSVC and bcc32 builds agree: every struct written raw to disk or wire has the
same measured size and offsets, because all are inside `#pragma pack(push,1)`
(Q17 and Q18 resolved). HK and relay cannot read each other's files.

Within one process `KILP.DAT` is mostly well protected (write-through, exclusive
write sharing, crash between data and header write tolerated). Weak points are
(1) **recovery**: any header/length mismatch blocks startup with no repair tool
(H-015 extension); (2) **size limits**: in shipped builds a record can exceed its
buffer (H-041, H-042); (3) **locking**: relay `addfrec` never releases its lock
(H-040) and several paths write files without a lock (H-045, H-047, H-051).

## Tiedostot / Files

| Tiedosto / File | HK | Viesti / Relay | Muoto / Format | Kirjoittajat ja lukko / Writers and lock |
|---|---|---|---|---|
| `KILP.DAT` | kyllä / yes | kyllä / yes | dbboxm-tietuetiedosto / record file | HK: `kilpdat_CS` jokaisessa / on every `getrec`/`putrec`/`addrec`/`deleterec` (`Hk/HkDat.cpp:563-651`); viesti: `tall_CS`, `kilpdat_CS` vain / only `addfrec`/`delfrec` (`Juk/vdat.cpp:178-204`) |
| `COMFILE.DAT` | kyllä | kyllä | levyjono, otsakerivi + paikat / disk queue | `outb_CS` kaikilla poluilla / on all paths (`Tp/TpComY.cpp:476-530`, `:1405-1530`) |
| `EMIT<vaihe>.DAT` / `EMIT.DAT` | `EMIT<n>.DAT` | `EMIT.DAT` | `emittp`-taulukko ilman otsaketta / array, no header | `emit_CS`, poikkeukset / exceptions H-011, H-045 (`Hk/HkEmit.cpp:1572-1623`) |
| `AJAT*.LST` | `AJAT1.LST`, `LAJAT1.LST` | `AJAT.LST` | otsake "AJAT_V" + `aikatp`-taulukko / header + array | `ajat_CS` (`Hk/HkAjat.cpp:129-291`) |
| `SQLjonoVarm.dat` | GUI + MySQL | GUI + MySQL | 8 tavun parit / 8-byte pairs | `SQL_CS` (`cbHk/UnitMySQL.cpp:640-750`) |
| `peruscfg.cfg`, `LASKENTA.CFG` | kyllä | XML (`Juk/VXml.cpp:430`) | teksti / text | vain käynnistys / startup only (`Hk/HkInit.cpp:327-386`) |

Indeksitiedostoja ei ole: dbboxm:n B-puu-indeksikerros (`dbboxm/Dbbox.cpp`,
`addkey.cpp`, `findkey.cpp` …) on kuollutta koodia, ja indeksit (`kilpnoix`,
`nameindex`) rakennetaan muistiin käynnistyksessä. [todennettu grepillä,
tarkastettu] Ainoa kytkös on nimi `initindex`, joka on määritelty sekä dbboxm:ssä
että `Hk/HkIx.cpp:102`/`Juk/VIx.cpp:193` (Q28).

There are no index files: the dbboxm B-tree index layer is dead code, and the
indexes are built in memory at startup. The only link is the name `initindex`,
defined both in dbboxm and in `Hk/HkIx.cpp:102`/`Juk/VIx.cpp:193` (Q28).

## KILP.DAT: tiedostokerros / file layer

`dbboxm/dbbox1.cpp` on linkitetty toteutus (`Dbbox.cpp` ei ole käytössä). Tietue `r`
on kohdassa `r*recl` (`:64-100`); `recl` ei ole tiedostossa. `DATAREF` on
`unsigned int` (`include/boxr.h`). Tietue 0 on otsake (`updatedescr` `:142`,
`makefile` `:104`, `closefile` `:214`):

`dbboxm/dbbox1.cpp` is the linked implementation. Record `r` is at `r*recl`;
`recl` is not stored. Record 0 is the header:

| Siirtymä / Offset | Koko / Size | Sisältö / Content |
|--:|--:|---|
| 0 | 2 | `firstfree` (u16; 0xFFFF = ei / none) |
| 2 | 2 | `numberfree` (u16) |
| 4 | 2 | `int1` (u16, 0) |
| 6 | 2 | `int2` = `numrec` (u16) |
| 8 | 8 | nollaa / zero |
| 16 | 40 | `maaliajat[10]`, int32 (`Hk/HkGlobals.cpp:565`; viesti / relay `Tp/TpDef.h:123`) |
| 56 | … | tietueen loppu jaetusta puskurista `tarecbuf` / rest from shared `tarecbuf` (H-051) |

- 32-bittiset laskurit tallennetaan 16-bittisinä (`tacompress` `:34`, `taexpand`
  `:48`); yli 65 535 tietueen tiedostoa ei voi avata. Ei saavutettavissa
  (`MAXKILPNO` 9999). / Counters stored as 16 bits; unreachable in practice.
- `openfile` (`:161-211`) vaatii `flen >= reclen`, `reclen*numrec == flen` ja
  `int2 == numrec` tai `numrec-1` (yksi otsakkeeton lisäys siedetään, `:195`).
  Muuten "otsikkoalue virheellinen". Tämä on samalla epäsuora tietuepituuden
  tarkistus: väärän muodon tiedosto yleensä hylätään eikä lueta väärin. /
  Implicit record-length check: a wrong-format file is usually rejected, not misread.
- `ainauusirec = 1` (`:22`, HK `Hk/HkInit.cpp:3092` kommentoitu): `newrec` lisää aina
  loppuun; vapaiden lista ei koskaan ole käytössä, eikä numeroita käytetä
  uudelleen edes uudelleenkäynnistyksen jälkeen. Tämä on H-022:n perusta:
  tietuenumero on paikallinen tiedostoindeksi. / Always appends; numbers are
  local file indexes (basis of H-022).
- `deleterec` (`:262`) kirjoittaa poistetun tietueen päälle otsakkeen kuvan;
  ensimmäinen int16 on silloin nollasta poikkeava. Aktiivinen = `kilpstatus == 0`
  (`Hk/HkDat.cpp:567-573`; viesti `Juk/VIx.cpp`). / Deleted records overwritten;
  active means first int16 is 0.
- Avaus `CreateFile(..., FILE_SHARE_READ, ..., FILE_FLAG_WRITE_THROUGH)`: toinen
  kirjoittava prosessi estyy. HK ja viesti avaavat `KILP.DAT`:n ennen emit- ja
  `COMFILE`-tiedostoja, joten toinen ohjelma samassa hakemistossa pysähtyy ennen
  kuin ehtii katkaista `COMFILE.DAT`:n (`Hk/HkInit.cpp:3486` ennen `:3609`/`:3659`;
  `Juk/VInit.cpp:2721` ennen `:2882`/`:2925`). [todennettu]

## KILP.DAT: HK-tietue / HK record

HK sarjallistaa kenttä kerrallaan taulukoiden avulla (`Hk/HkDat.cpp:44-104`,
`luekentat` `:105`, `pack0` `:370`, `unpack0` `:432`), joten muoto ei riipu
kääntäjän rakenneasettelusta. Merkkijonot ovat `wchar_t` (2 tavua, UTF-16LE).
`unpack` nollaa tietueen ensin (`:485`).

HK serialises field by field through tables, so the format does not depend on
compiler struct layout. Strings are UTF-16LE.

Perustiedot, `kilprecsize0` = 360 [todennettu laskemalla taulukosta / summed]:

| Siirtymä / Offset | Koko / Size | Kenttä / Field |
|--:|--:|---|
| 0 | 2 | `kilpstatus` |
| 2 | 2 | `kilpno` (u16) |
| 4 | 8 | `lisno[2]` |
| 12 | 20 | `wrkoodi[10]` |
| 32 | 2 | `ilmlista` |
| 34 | 2 | `piiri` |
| 36 | 12 | `pisteet[3]` |
| 48 | 50 | `sukunimi[25]` |
| 98 | 50 | `etunimi[25]` |
| 148 | 32 | `arvo[16]` |
| 180 | 64 | `seura[32]` |
| 244 | 32 | `seuralyh[16]` |
| 276 | 32 | `yhdistys[16]` |
| 308 | 32 | `joukkue[16]` |
| 340 | 8 | `maa[4]` |
| 348 | 12 | `sarja`, `sukup`, `ikasarja`, `alisarja`, `synt`, `arvryhma` (2 tavua kukin / 2 bytes each) |

Vaihetta kohden 152 tavua (`kilppvtpsize0`, kun `HAJONTA` = 11; C-11 mittasi ajossa 152, aiempi
luku 154 oli laskuvirhe) ja `valuku+2`
väliaikaa à 8 tavua (int32 `vatulos`, int16 `vasija`, int16 `vakoodi`):

    kilprecsize = 360 + n_pv * (152 + (valuku + 2) * 8)      (Hk/HkDat.cpp:176-177)

Per stage 152 bytes (C-11 measured 152 at run time; the earlier 154 was an arithmetic slip) plus `valuku+2` split entries of 8 bytes. `n_pv` ja `valuku`
luetaan sarjatiedostosta tai XML:stä ajon aikana (`Hk/HkSrj.cpp:100-158`,
`Hk/HkXml.cpp:439-442`); käännösaikaiset `N_PV` ja `VALUKU` ovat vain ylärajoja.
`n_pv` and `valuku` come from the series file or XML at run time; compile-time
`N_PV`/`VALUKU` are only caps.

Vaiheen kentät / stage fields (siirtymä / offset): `txt[10]` 0, `uusi` 20, `vac` 21,
`flags[2]` 22, `tav` 30, `enn` 34, `rata[12]` 38, `sarja` 62, `era` 64, `bib` 66,
`badge[2]` 68, `laina[2]` 76, `selitys[18]` 80, `pvpisteet[2]` 116, `tlahto_arv` 124,
`keskhyl` 128, `ampsakot[6]` 130, `tasoitus` 142, `sakko` 146, `ysija` 150.

## KILP.DAT: viestitietue / relay record

Viesti kopioi muistin rakenteen paloina (`Juk/vdat.cpp:91-110`, `#pragma pack(1)`,
`Juk/VDef.h:295-1524`): ensin `kilprecsize0` tavua perustietoja, sitten jokaisesta
osuudesta `osrecsize0 + (valuku+1)*6` tavua. Muistissa osuuden koko on
`sizeof(osrec)`, jossa väliaikataulukko on käännösaikaisen `VAIKALUKU`n mittainen,
joten levyllä osuudet ovat tiiviimmin kuin muistissa. [todennettu, tarkastettu;
toisen agentin väite "suora rakenteen kopio" kumottiin]

The relay copies the in-memory struct in pieces: `kilprecsize0` base bytes, then
`osrecsize0 + (valuku+1)*6` bytes per leg. In memory a leg is `sizeof(osrec)`
with a `VAIKALUKU`-long split array, so legs are packed tighter on disk. (The
claim "raw struct copy" was refuted.)

    kilprecsize = kilprecsize0 + n_os * (osrecsize0 + (valuku + 1) * 6)   (Juk/VInit.cpp:2661-2662)

| Mitta / Measure | Arvo / Value |
|---|---|
| `KILPRECSIZE0` / `OSRECSIZE0` (mitattu / measured, MSVC = bcc32) | 138 / 148 |
| `sizeof(osrec)` / `sizeof(kilptietue)`, `MAXOSUUSLUKU` 30, `VAIKALUKU` 12 | 226 / 6918 |
| sama, `MAXOSUUSLUKU` 250, `VAIKALUKU` 8 | 202 / 50 638 |
| Osuuden kentät / leg fields | `nimi[41]`, `lisno`, `osseura[31]`, `arvo[16]`, `hajonta[HAJONTA+1]`, `badge[2]`, `laina[2]`, 15 tavun unioni / union, `uusi`, `seuranta`, `kone`, `ossija`, `ylahto`, `lahtolaji`, `keskhyl`, `ampsakot[6]`, `sakko`, `va[]` |

Merkkijonot ovat 8-bittisiä; `kilpparam.chars` pakotetaan arvoon '8'
(`Juk/VInit.cpp:2485`), eli UTF-8. Vanhemmat tiedostot ovat OEM- tai
ANSI-merkistöä, eikä niitä voi erottaa. `n_os` ja `valuku` tulevat XML:stä
(`Juk/VXml.cpp:442-445`) ilman ylärajan tarkistusta (H-042). `HAJONTA` on 8 kaikissa
viestikäännöksissä, joten MSVC ja bcc32 sopivat yhteen.

Strings are 8-bit, forced to UTF-8; older OEM/ANSI files cannot be told apart.
`n_os` and `valuku` come from XML without an upper-bound check (H-042).

## Vanha muoto / Old format (Q4)

Vanhaa muotoa ei tunnisteta automaattisesti. Vanha `KILP.DAT` luetaan vain
erillisellä GUI-tuonnilla, jossa käyttäjä antaa muodon parametrit
(`Hk/HkInitVanha.cpp`, `cbHk/UnitImport.cpp:1105-1106`, `:1212-1214`; viesti
`ViestiWin/ConvDat.cpp`). Parametreja ei rajata, ja puskurit ovat kiinteitä (H-049).
Tavallisessa käynnistyksessä väärän muodon tiedosto yleensä hylätään pituustarkistuksessa.

There is no automatic detection. Old `KILP.DAT` files are read only by an explicit
GUI import where the user types the layout parameters; they are not clamped and
buffers are fixed (H-049). At normal startup a wrong-format file is usually
rejected by the length check.

## COMFILE.DAT

Rivi 0 on otsake (`maxyhteys` paikkaa à `combufsize`): ensimmäisessä paikassa
`so` = `maxyhteys` ja `len` = `combufsize`, muu nollaa. Yhteyden `cn` paketti `n` on
kohdassa `((n+1)*maxyhteys + cn) * combufsize` (`Tp/TpComY.cpp:2258-2380`, `buflah`
`:476-530`).

Row 0 is the header; packet `n` of link `cn` is at `((n+1)*maxyhteys + cn) * combufsize`.

| Siirtymä / Offset | Koko / Size | Kenttä / Field |
|--:|--:|---|
| 0 | 4 | `lahetetty` (0 = lähettämättä / unsent; muuten vuorokaudenaika / else time of day) |
| 4 | 2 | `portti` |
| 6 | 1 | `so` |
| 7 | 1 | `id` |
| 8 | 1 | `iid` |
| 9 | 1 | `pkgclass` |
| 10 | 2 | `len` |
| 12 | 2 | `checksum` |
| 14 | `class_len` | `d` |

Otsake on 14 tavua kaikissa käännöksissä (`Hk/HkDef.h:1235-1313`,
`Juk/VDef.h:1170-1259`). HK: `combufsize = max(class_len) + 14`
(`Hk/HkCom32.cpp:1095-1105`). Viesti: `combufsize = max(class_len)` ilman otsaketta
(`Juk/VCom.cpp:1236-1240`) → H-025. HK:n ja viestin `COMFILE.DAT` eivät siksi ole
yhteensopivia. Tarkistussummaa ei tarkisteta luettaessa; se lasketaan
lähetettäessä uudelleen (`:1507`).

The header is 14 bytes in every build. HK adds the header to `combufsize`, the
relay does not (H-025). The checksum is not checked on read; it is recomputed on send.

Uudelleenkäynnistys (`:2306-2330`): jokaiselle yhteydelle lasketaan peräkkäiset
kelvolliset rivit (`len > 0` ja `len == class_len[pkgclass]`) ja niistä lähetetyt.
Ensimmäinen virheellinen rivi lopettaa laskennan; myöhemmät jonossa olevat jäävät
lähettämättä (H-044). `pkgclass`-indeksiä ei rajata (`:2319`, H-032). Jos otsake on
rikki mutta pituus on `so*len`, tiedosto katkaistaan kysymättä (`:2290-2292`,
`:2349`, H-038).

Restart counts consecutive valid rows per link; the first invalid row stops the
scan (H-044). A damaged header with length `so*len` truncates the file silently (H-038).

## Mitatut koot / Measured sizes (Q18)

Mitattu koettimilla, jotka sisällyttävät repon otsikot projektien `-D`-määrityksin
(MSVC 19.52 `/J`; bcc32 7.80). MSVC ja bcc32 antavat samat arvot kaikille
koetelluille rakenteille. / Measured with probes including the repo headers with
the projects' defines; MSVC and bcc32 agree on every probed struct.

| Käännös / Build | `MAXOSUUSLUKU` / `VALUKU` | `sizeof(combufrec)` | Suurin `class_len` / largest | `combufsize` | Tarvitaan / needed |
|---|---|--:|---|--:|--:|
| HkMaali Release, HkKisaWin Release | `VALUKU` 250 | 2196 | `KILPPVT` 2178 (valuku 250) | 2192 | 2192 OK |
| HkMaali ReleaseVA200, HkKisaWin 200Release | `VALUKU` 200 | 1796 | 1778 | 1792 | OK |
| ViestiMaali/ViestiLuenta Release, ViestiWin Release Build | 30 | 258 | `EMITVA` 204 (n_os 4); `YHTEISLAHTOT` 244 (n_os 30) | 204 / 244 | 218 / 258 **H-025** |
| ViestiMaali Release150os, ViestiWin Debug Build | 250 | 2018 | 204 (n_os 4); 2004 (n_os 250) | 204 / 2004 | 218 / 2018 **H-025** |

| Rakenne / Struct | HK | Viesti / Relay |
|---|--:|--:|
| `emittp` | 188 | 188 |
| `emitvatp` | 236 | 204 |
| `aikatp` | 22 | 24 |
| `vatp` | 8 | 6 |
| `datafile` | 32 | 32 |

Toimitettavat konfiguraatiot: CI ja julkaisu kääntävät `/p:Configuration=Release`
(`.github/workflows/build.yml:25`, `release.yml:33`), ja `build-gui.ps1:234` käyttää
projektin oletusta (`HkKisaWin.cbproj:8` Release). HK:n Release-käännöksissä
`VALUKU` on siis oletus 250 ja `N_PV` 5 (`Hk/HkDef.h:45-58`); 200 on vain
lisäkonfiguraatioissa. Viestissä Release on `MAXOSUUSLUKU` 30. [todennettu,
tarkastettu; ristiriitainen väite "Release = 200" kumottiin]

Shipped configurations: CI and release build `Release`; the GUI script uses the
project default (Release). HK Release builds therefore use `VALUKU` 250 and
`N_PV` 5; relay Release uses `MAXOSUUSLUKU` 30. (The conflicting claim "Release =
200" was refuted.)

## Kääntäjävalitsimet / Compiler switches (Q17)

- **Q17 ratkaistu / resolved:** bcc32 noudattaa `#pragma option -K` ja
  `#pragma option -a1` -direktiivejä kesken käännösyksikön (mitattu). Kaikki
  käännökset laskevat tarkistussumman etumerkittömin tavuin (`/J` MSVC:ssä,
  `-K` tai `tputil.h` bcc32:ssa), joten tarkistussummat täsmäävät. Merkitystä on vain
  parittoman pituisilla luokilla (`AIKAT` 45/49, `TARKT` 17). / bcc32 honours both
  mid-unit; all builds compute checksums with unsigned bytes and agree.
- `include/tputil.h:27-29` asettaa `#pragma option -K -a1` eikä palauta niitä:
  bcc32:lla jokainen myöhemmin määritelty rakenne pakataan tavun tarkkuudella
  (`struct{char;int}` = 5, MSVC 8). Raakana kirjoitettavat rakenteet ovat kaikki
  `pack(push,1)`-alueella, joten tiedostot eivät eroa (tarkastettu `sizeof`-pohjaiset
  `_read`/`_write`/`fread`/`fwrite`). T-016. / Never restored; no raw-written struct
  affected.
- `#pragma -K -a1` ilman sanaa `option` (43 tiedostoa, esim. `tputilv2/AIKATOSH.cpp:20`)
  ei tee bcc32:lla mitään eikä anna varoitusta (mitattu). T-016.
- `HkKisaWin.cbproj`:sta puuttuu `-K` (`BCC_DefaultCharUnsigned`), toisin kuin muista
  kolmesta projektista; `char` on etumerkitön vasta `tputil.h`:n jälkeen. T-016.
- `JukLuenta520.vcxproj`: kaikilla neljällä konfiguraatiolla samat määritykset
  (`MAXOSUUSLUKU` 30 myös Debug250/Release150os) (`:95`, `:112`, `:131`, `:152`). T-017.
- `MAXNVAIHE`a ei ole lähdekoodissa. / does not exist.

## Muut tiedostot / Other files

**Emit-tiedosto / emit file.** Ei otsaketta; `emittp`-taulukko, tietuenumero =
indeksi, `emithead = filelength / sizeof(emittp)` (`Hk/HkEmit.cpp:3060`,
`Juk/VEmit.cpp:3609`). Kesken jäänyt viimeinen tietue ohitetaan ja seuraava lisäys
kirjoittaa sen päälle. `putem` ja `lueem` käyttävät erillisiä `_lseek`- ja
`_write`/`_read`-kutsuja jaetulla kahvalla, joten jokaisen kutsujan on pidettävä
`emit_CS`:ää (H-011, H-045). `emittp`: `package` 0, `badge` 4, `year` 8, `week` 10,
`maali` 12, `time` 16, `ctrlcode[50]` 20, `ctrltime[50]` 70, `sc`/`pc`/`lc`/`kilpno`/`osuus`
170-178, `badge0` 180, `kirjaus` 184.

No header; record number = index. A torn last record is ignored and overwritten.
`putem`/`lueem` use separate seek and I/O calls on a shared handle, so every caller
must hold `emit_CS` (H-011, H-045).

**AJAT-jono / time queue.** Tietue 0 on `sizeof(aikatp)` tavun otsake, jossa
ASCII "AJAT_V<n>" (`Hk/HkIV.cpp:4040-4045`); vain 6 ensimmäistä tavua tarkistetaan
(`:4026`), versionumeroa ei. Ilman otsaketta tiedosto luetaan vanhana muotona.
Tiedosto kirjoitetaan avattaessa kokonaan uudelleen. HK:n (22 t) ja viestin (24 t)
tiedostot eivät ole vaihdettavissa. H-054. / Header "AJAT_V<n>", only 6 bytes checked;
rewritten on open; HK and relay differ.

**SQLjonoVarm.dat.** Jono 8 tavun pareja {int32 tietuenumero, int32 jonon pituus};
uudelleenkäynnistyksessä viimeiset 8 tavua kertovat montako paria toistetaan
(`cbHk/UnitMySQL.cpp:665-681`). Append-tila rikkoo päivityksen (H-043).
/ 8-byte pairs; append mode breaks the update (H-043).

**Konfiguraatio / configuration.** `peruscfg.cfg` ja sitten `LASKENTA.CFG` (tai
`CFG=`), `readcfg` (`Hk/HkInit.cpp:327-386`) `TextFl`-luokalla. Merkistö
tunnistetaan tiedostokohtaisesti (BOM, muuten heuristiikka ANSI/OEM/UTF-8;
`tputilv2/TextFileClass.cpp:125-222`); uudet tiedostot UTF-8 BOMilla. Rivit
katkaistaan 77 merkkiin hiljaa; `;` aloittaa kommentin; enintään 200 parametria;
tuntematon avain antaa varoituksen. Kiinteät puskurit: H-052.
/ Encoding detected per file; lines silently cut at 77 characters; unknown key
gives a warning; fixed buffers (H-052).

## Eheys ja vikatilanteet / Integrity and failure modes

| Tilanne / Situation | Seuraus / Consequence | Havainto / Finding |
|---|---|---|
| Kaatuminen datan ja otsakkeen kirjoituksen välissä / crash between data and header write | Siedetään (`int2 == numrec-1`) / tolerated | – |
| Levy täynnä `addrec`issa / disk full in `addrec` | Otsake väittää yhden tietueen enemmän → seuraava käynnistys epäonnistuu; myöhempi lisäys jättää nollatietueen, joka ladataan aktiivisena / header claims one more record → next start fails; later append leaves a zero record loaded as active | H-015 (laajennus / extension) |
| `getrec` epäonnistuu / fails | `kilptietue::read` purkaa alustamattoman pinopuskurin / unpacks an uninitialised stack buffer (`Hk/HkDat.cpp:563-573`) | H-015 |
| `COMFILE.DAT`-kirjoitus epäonnistuu / write fails | Edellinen paketti lähetetään uudelleen uudella id:llä tai paketti katoaa; uudelleenkäynnistys pysähtyy aukkoon / previous packet resent or packet lost; restart stops at the gap | H-044 |
| `KIRJHETI`-bitti 2 | Ei vaikutusta: `fobfile` on aina NULL, `fflush(NULL)` / no effect | H-044 |
| Otsake rikki, pituus `so*len` / damaged header | `COMFILE.DAT` katkaistaan kysymättä / truncated silently | H-038 |
| Sähkökatko kirjoitusläpäisevän lisäyksen aikana / power cut during write-through append | Tuntematon (Q27) / unknown | – |
| Kaksi ohjelmaa samassa hakemistossa / two programs in one directory | Toinen pysähtyy `KILP.DAT`:iin ennen kuin koskee muihin / second stops at `KILP.DAT` | – |

## Löydökset / Findings

| ID | Löydös / Finding | Vakavuus / Severity | Tarkastus / Review |
|---|---|---|---|
| H-040 | Viesti: `addfrec` ottaa `kilpdat_CS`:n kahdesti eikä vapauta (`Juk/vdat.cpp:183`, `:190`); verkotetussa viestissä joukkueen lisäys yhdellä säikeellä ja lisäys tai poisto toisella → ohjelma jumiutuu (toinen pitää `tall_CS`:ää) / relay `addfrec` never releases its lock → freeze when a second thread adds or deletes a team | korkea / high | VAHVISTETTU (myös pääistunto / also main session) |
| H-041 | HK: `char buf[KILPBUFSIZE]` = 10 000, mutta toimitetuissa Release-käännöksissä (`VALUKU` 250, `N_PV` 5) tietue voi olla 11 200 tavua (C-11: mitattu ajossa / measured at run time; aiemmin / earlier 11 210) (n_pv 5, valuku ≥ 220) → pinon ylivuoto `getrec`issa (`Hk/HkDef.h:98-103`; `Hk/HkDat.cpp:565`, `:583`, `:629`, `:1433`; `cbHk/UnitExport.cpp:464`; vain raja-arvotarkistus `Hk/HkSrj.cpp:156`) / stack overflow in shipped Release builds | korkea | VAHVISTETTU (ristiriita ratkaistu / conflict resolved) |
| H-042 | Viesti: XML:n osuus-, väliaika- ja aktiivisten osuuksien määriä ei rajata käännösaikaisiin rajoihin (`Juk/VXml.cpp:442-445`, `:482`; ainoa raja `ViestiWin/UnitKilpMaaritys.cpp:271-276`) → `char buf[KILPRECSIZE]` (`Juk/vdat.cpp:145`, `:153`, `:181`) ja `valuku[MAXOSUUSLUKU]` (`Juk/vSarjat.cpp:60-70`) ylivuotavat / relay XML counts unclamped → buffer overflows | korkea | VAHVISTETTU (pahempi / worse) |
| H-043 | `SQLjonoVarm.dat` avataan append-tilassa (`a+b`), joten päivitys `fseek(-4)`+`fwrite` lisää 4 tavua loppuun ~300 ms välein → tiedosto kasvaa (~1 Mt/vrk), parit menevät sekaisin, uudelleenkäynnistys lähettää vääriä tietueita SQL:ään ja hukkaa oikeat (`cbHk/UnitMySQL.cpp:663`, `:703-719`; `ViestiWin/UnitMySQL.cpp:610`, `:657-658`) / append mode breaks in-place update | keskitaso / medium (vain `CBVarmista` / opt-in) | VAHVISTETTU |
| H-044 | `COMFILE.DAT`-kirjoitusvirhe kasvattaa silti `cjseur`ia → edellinen paketti lähetetään uudelleen uudella id:llä tai nollattu paikka merkitään lähetetyksi; uudelleenkäynnistys pysähtyy ensimmäiseen aukkoon; `KIRJHETI & 2` ei vaikuta (`Tp/TpComY.cpp:497-517`, `:1415`, `:1465-1513`, `:2316-2324`, `:503`) / write failure still advances the queue | keskitaso | OSITTAIN (laukaisin levy täynnä tai I/O-virhe / trigger disk full or I/O error) |
| H-045 | HK: `uusiemitanalyysi` → `lueemitfile` lukee ja indeksoi emit-tiedoston uudelleen ilman `emit_CS`:ää, kun `addem`/`putem` kirjoittaa toisessa säikeessä; vrt. `lueRatatiedotUudelleen`, joka lukitsee (`Hk/HkEmit.cpp:6403-6432`, `:3029-3110`; `cbHk/VertAikaUnit.cpp:60-64`) / emit file reloaded without lock | keskitaso | VAHVISTETTU (mekanismi; kilpailutilanne [päätelty] / race inferred) |
| H-046 | Viesti: "kirjoita KILP.DAT" -vienti kirjoittaa muistin olion eikä `pack`-tulosta → osuudet ≥ 1 väärissä kohdissa, kun `valuku < VAIKALUKU` (`Juk/vdat.cpp:950-951`; `ViestiWin/UnitExport.cpp:481`); HK tekee oikein (`Hk/HkDat.cpp:1435-1444`) / relay export writes the raw object | keskitaso | VAHVISTETTU |
| H-047 | Viesti: `vaihdakilpsrj` kirjoittaa `KILP.DAT`:iin ilman lukkoa (`ViestiWin/UnitSarja.cpp:50-63`) ja tuonti sulkee ja avaa elävän `datf2`:n ilman lukkoa (`cbHk/UnitImport.cpp:1434-1436`, `ViestiWin/UnitImport.cpp:1113-1116`); lisäksi viestin `addfrec` sulkee ja avaa tiedoston jokaisen lisäyksen jälkeen, mutta `tallenna`n `putrec_i` ei ota `kilpdat_CS`:ää / unlocked writes and live close/reopen | keskitaso | OSITTAIN (todennäköisin seuraus menetetty päivitys / most likely lost update) |
| H-048 | Uudelleenlähetys `lahetanumerotTCP` lukee levyjonoa askeleella `sizeof(combufrec)`, vaikka kirjoitus käyttää `combufsize`a → väärä paketti (`Hk/HkCom32.cpp:1202-1205`, `Juk/VCom.cpp:1376-1384`, `Tp/TpComY.cpp:495`); vain konsoliversioiden `SERVER`-valikko / wrong stride in resend | matala / low | VAHVISTETTU |
| H-049 | Vanhan muodon tuonti: käyttäjän parametreja ei rajata; `char buf[1000]` (`Hk/HkInitVanha.cpp:50`, `:506`), `getrec`-tulos ohitetaan (`:509`), `oemtowcs` kirjoittaa ilman lopetusmerkkiä tai yhden yli (`:328`, `:400`); viestin `ViestiWin/ConvDat.cpp:145`, `:150` ylivuotaa staattisen rakenteen / old-format import overflows | keskitaso | OSITTAIN (rivinumerot tarkennettu / line numbers refined) |
| H-050 | `UnitComfile`-katselin asettaa tiedoston otsakkeen pituuden **globaaliin** `combufsize`en (muuttaa käynnissä olevan tiedonsiirron askeleen), lukee `fread(&combuf, RecLen)` paikalliseen rakenteeseen (pinon ylivuoto) ja jakaa nollalla, jos `so == 0` (`cbHk/UnitComfile.cpp:145-176`, `ViestiWin/UnitComfile.cpp:135-160`) / viewer overwrites global stride, stack overflow, divide by zero | keskitaso | VAHVISTETTU (pahempi) |
| H-051 | dbboxm:n staattinen `tarecbuf` on yhteinen kaikille tiedostoille ilman lukkoa; vienti päälangalla voi kirjoittaa väliaikaistiedoston otsakkeen `KILP.DAT`:iin → seuraava käynnistys epäonnistuu (`dbboxm/dbbox1.cpp:20`, `:106-157`, `:214-221`, `:262-270`; `cbHk/UnitExport.cpp:466-483`) / shared static header buffer | matala | OSITTAIN (ikkuna mikrosekunteja / window microseconds) |
| H-052 | Konfiguraatio: `wcscpy` staattisiin puskureihin `tunn_srj_nimi[12]` ja `baudst[][20]` (`Hk/HkInit.cpp:93`, `:2088`, `:63`, `:1253`); `line[l-1]`, kun `l == 0` (`:360`) / config buffer overflows | matala | OSITTAIN |
| H-053 | `lueem` ohittaa `_read`-tuloksen (`Hk/HkEmit.cpp:1581`); viesti vertaa `_read(...) < sizeof(emittp)` ilman muunnosta, joten -1 jää huomaamatta (`Juk/VEmit.cpp:3631`, `:4937`, `:5928`) / read errors undetected | matala | VAHVISTETTU (korjaus: `:1464` on `_write`) |
| H-054 | AJAT-jono: viesti kirjoittaa uudelleen vain `mxtime-1` tietuetta (`Juk/VIv.cpp:2771`) → viimeinen katoaa täydestä jonosta; virhepolut kutsuvat `_close(fl)` pituudella eikä kahvalla (`Hk/HkIV.cpp:4023`, `Juk/VIv.cpp:2698`) → vuoto tai väärän kahvan sulkeminen / last record lost, wrong handle closed | matala | VAHVISTETTU |

Laajennetut aiemmat havainnot / extended earlier findings:

- **H-015:** lisäksi uudelleenkäynnistys estyy, nollatietue ladataan aktiivisena ja
  `read` purkaa alustamattoman puskurin (yllä). / plus startup refusal, zero record
  loaded as active, uninitialised read.
- **H-025:** vahvistettu mitaten. Muistijonon `memcpy` (`Tp/TpComY.cpp:530`) ei ole
  viestissä käytössä (`levypusk` aina päällä, `Juk/VInit.cpp:1217-1219`); todellinen
  polku on levyjono: `_write(..., combufsize)` pudottaa 14 viimeistä tavua, ja
  lähetettäessä loppu täyttyy edellisen paketin tavuilla, joista lasketaan kelvollinen
  tarkistussumma → vastaanottaja hyväksyy väärän sisällön. / Confirmed by measurement;
  the real path is the disk queue, the tail is filled from the previous packet and
  gets a valid checksum.
- **H-032:** tarkennettu. Yhden saapuvan paikan ylivuoto on mahdollinen kaikissa
  käännöksissä myös HK:ssa (luokat, joiden `class_len` on 0: HK 5, 8, 9, 11, 12);
  koko kasalohkon ylitys vain viestissä `MAXOSUUSLUKU` 30, joka on toimitettu
  oletus. Tarkistussumma on pelkkä summa, joten yksi väärennetty UDP-paketti riittää.
  Vakavuus nostettu viestissä korkeaksi. / Slot overflow in all builds; heap-block
  overflow only in relay with `MAXOSUUSLUKU` 30 (shipped default); severity raised
  to high for relay.
- **H-011:** vahvistettu; kilpailutilanne voi myös kirjoittaa tietueen väärään kohtaan,
  ei vain lukea vanhaa. / can also misplace a write.
- **H-022:** vahvistettu; tietuenumero on paikallinen tiedostoindeksi. / record number is a local file index.
- **H-030:** `lahetetty` on vuorokaudenaika ilman päivämäärää [päätelty]. / time of day, no date.
- **H-038:** tiedosto katkaistaan myös kysymättä, kun otsake on rikki ja pituus on `so*len`.
  / also truncated silently when the header is damaged.

Kumotut väitteet / refuted claims: viestin `pack` on suora rakenteen kopio;
HK:n Release-käännöksissä `VALUKU` on 200; `COMFILE`-tiedoston osoittimen
kilpailutilanne (`outb_CS` on kaikilla poluilla); kaksi ohjelmaa samoilla
tiedostoilla katkaisee `COMFILE.DAT`:n; tiedostonimien OEM/ANSI-ongelma.
Refuted: relay `pack` is a raw struct copy; HK Release uses `VALUKU` 200; COMFILE
file-pointer race; two instances truncating COMFILE; OEM/ANSI file-name issue.

Tekninen velka / technical debt: T-015 (viestin kuolleet nimen katkaisusilmukat,
`Juk/vkilp.cpp:945`, `:956`, `:1391`, `:1421`), T-016 (`-K`/`-a1`-pragmat), T-017
(`JukLuenta520.vcxproj`), T-018 (dbboxm:n kuollut indeksikerros), T-019 (ei tunnistetta,
versiota eikä korjaustyökalua).

## Testattavuus / Testability

Kaikki testit tehdään synteettisillä tiedostoilla väliaikaishakemistossa; ei oikeita
kilpailutietoja eikä tuotantojärjestelmiä. Useimmat ovat yksikkötestejä nykyiseen
`TpTest`-projektiin (doctest), koska `dbboxm` ja pakkausfunktiot ovat irrotettavissa.

All tests use synthetic files in a temp directory; no real competition data or
production systems. Most are unit tests for the existing `TpTest` project
(doctest), since `dbboxm` and the pack functions can be isolated.

| ID | Testi / Test | Odotettu / Expected | Vika näkyy / Failure | Deterministinen / Deterministic |
|---|---|---|---|---|
| H-040 | `addtall` säikeessä A; sen jälkeen `kilpdat_CriticalSection.OwningThread`/`RecursionCount` tai `deltall` säikeessä B aikakatkaisulla / then check lock owner or call `deltall` on thread B with timeout | Lukko vapaa / lock free | B ei palaa / B never returns | kyllä / yes |
| H-041 | `kilptietue::read` tietueella n_pv 5, valuku 250 `/RTC`- tai ASan-käännöksessä; tai `static_assert(KILPBUFSIZE >= 360 + N_PV*(152+(VALUKU+2)*8))` | Ei ylivuotoa / no overflow | Pinon korruptio / stack corruption | kyllä |
| H-042 | `loadSoftwareData` XML:llä, jossa `Legs` > `MAXOSUUSLUKU` | Hylätään / rejected | Ylivuoto / overflow | kyllä |
| H-043 | Avaa "a+b", kirjoita 8 tavua, `fseek(-4, SEEK_END)`, kirjoita 4 | Koko 8 / size 8 | Koko 12 / size 12 | kyllä |
| H-015 | Synteettinen `KILP.DAT`, otsake `int2 = numrec+1`; `openfile` | Korjaus tai selvä virhe / repair or clear error | Käynnistys estyy / start refused | kyllä |
| H-044 | Testikäännös, jossa `_write` epäonnistuu kerran `buflah`issa; sitten `lahetapaketti` | Ei lähetystä tai sama paketti / no send or same packet | Edellinen paketti uudella id:llä / previous packet with new id | kyllä |
| H-046 | Vie synteettinen joukkue (`valuku` 1, kaksi osuutta), lue `getrec_i`:llä, vertaa | Sama / equal | Osuus 2 eroaa / leg 2 differs | kyllä |
| H-025 | Käynnistystarkistus `combufsize >= 14 + max(class_len)` | Tosi / true | Epätosi viestissä / false in relay | kyllä |
| H-045, H-047, H-051 | Rasitustesti kahdella säikeellä ja lokitetuilla `putrec`/`putem`-siirtymillä / stress with logged offsets | Oikeat siirtymät / correct offsets | Väärä siirtymä / wrong offset | ei, tarvitsee testikoukun / no, needs test hook |

Ehdotus: lisää `TpTest`iin `static_assert`-tarkistukset kaikille mitatuille koille
(taulukko yllä), jolloin käännös pysähtyy, jos rakenne muuttuu tai käännökset
eroavat. / Proposal: add `static_assert`s for all measured sizes to `TpTest`, so a
build fails if a layout changes or builds diverge.

**C-11:** H-041:n kaava ei sovi `static_assert`iin: 360 ja 152 lasketaan ajossa, ja
nykyarvoilla (11 200 > 10 000) tarkistus estäisi käännöksen. Korvattu ajonaikaisella
tarkistuksella; tämän taulukon testit on tarkennettu tiedostossa `testaus.md`. /
H-041's formula does not suit a `static_assert` (computed at run time; would break
the build today). Replaced by a run-time check; the tests above are refined in
`testaus.md`.

## Avoimet kysymykset / Open questions

| # | Kysymys / Question |
|---|---|
| Q26 | Rajataanko HK:n XML:stä luetut `n_pv`/`valuku` (`Hk/HkXml.cpp:439-442`) myöhemmin ennen `luekentat`ia? / Are HK XML counts clamped later? |
| Q27 | Voiko NTFS jättää kirjoitusläpäisevän lisäyksen jälkeen pituuden, joka ei ole tietuekoon monikerta (sähkökatko)? / Can NTFS leave a non-multiple length after a power cut? |
| Q28 | Voiko dbboxm:n `initindex` voittaa linkityksessä `Hk/HkIx.cpp`:n tai `Juk/VIx.cpp`:n version? / Can dbboxm `initindex` win at link time? |
| Q29 | Mihin viestin viety `.dat` käytetään; avataanko se `KILP.DAT`:na (H-046 → tietojen menetys)? / How is the relay export used downstream? |
| ~~Q30~~ | **Ratkaistu C-6:ssa / resolved in C-6** (`saikeet-ja-lukot.md` luku 5): `KILP.DAT`:ia ei suljeta; emit- ja aikatiedostot suljetaan kolmen lukon alla, lukijasäikeitä ei pysäytetä (H-094, H-102). Alkuperäinen: miten HK:n vaiheen vaihto sulkee ja avaa `KILP.DAT`:n ja emit-tiedoston säikeiden ollessa käynnissä (`Hk/HkIx.cpp:1898-1985`)? / KILP.DAT is not closed; reader threads not paused |
| Q31 | Onko ViestiWinin `MAXOSUUSLUKU` 250 -käännöksissä H-041:n kaltainen puskuriraja? / H-041 analogue in relay 250 builds? |
| Q32 | `t_time_l`-aikaleiman (`lahetetty`) yksikkö ja kierto / unit and wraparound |
| Q33 | Miten `mbtowc` (`ansitowchar`) käsittelee tavuja > 127 "C"-lokaalissa MSVC:llä ja bcc32:lla (ANSI/OEM-konfiguraatiot, vanhat tuonnit)? / `mbtowc` above 127 in the "C" locale? |
| Q34 | Ajetaanko kilptietueen konstruktori (`new vatp[kilpparam.valuku+2]`) ennen kuin `valuku` on luettu (globaalit oliot) → kasan ylivuoto `unpack`issa? / Constructor before `valuku` is read? |
