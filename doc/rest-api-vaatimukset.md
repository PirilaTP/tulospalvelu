# REST-rajapinta: kilpailun status, kuuluttaja (CIS) ja tilannekuva — vaatimukset

> Tila: luonnos 11 (2026-11-19). Perustuu koodin nykytilaan haarassa `main` (f7fe553) ja haaraan
> `feature/sportident-reader` (51a17a3, luku 2.4; havainnot `doc/sportident-review-items.md`).
> Päätökset K1–K21 ja K24 on päivitetty (luku 10), avoimet kysymykset K22–K23 (luku 13). Palvelun versio ja rajapintakuvaukset: luku 5.12. Käyttöönottomalli, edustapalvelin (Caddy) ja CIS- ja Tilannekuva-selainsovellusten jakelu: luku 9.6 (K24). Sanomien kartoitus: `doc/sanomat-ja-tiedonsiirto.md`. Välimuisti ja kuormitustesti: luvut 9.3–9.4. Tulosluettelot (HTML, JSON, PDF, IOF XML): luku 5.11, niiden välimuisti luku 9.5. Polut ja kenttänimet ovat ehdotuksia.
> Kenttänimet, polut ja OpenAPI-/AsyncAPI-dokumentaatio ovat englanniksi. Tämä vaatimusdokumentti on suomeksi.

## 1. Tavoite ja rajaus

Ohjelmaan lisätään HTTP/JSON-rajapinta ja WebSocket-tapahtumavirta. Niiden kautta erillinen
sovellus saa:

1. **kilpailun statuksen** (vastaa *Seuranta / Status* -kaavaketta),
2. **kuuluttajan tietojärjestelmän (Commentator Information System, CIS) tiedot** sarjan tai
   väliaikapisteen (rastikoodin) perusteella, kilpailijoiden tunnisteet taustatietojen
   yhdistämistä varten mukaan lukien,
3. **turvallisuuden tilannekuvan**: missä ja kuinka paljon kilpailijoita on maastossa,
4. **väliaikapisteiden leimaukset viiveettä** WebSocketin kautta,
5. **myöhemmässä julkaisussa** rajatut tietojen syötöt: *ei lähtenyt* ja *uusi lähtöaika*.

Loppukäyttäjille rajapinnan tiedot tarjotaan selainsovelluksilla **CIS** (kuuluttaja) ja
**Tilannekuva** (turvallisuus). Ne käyttävät vain tätä rajapintaa, ja edustapalvelin (Caddy) jakaa
ne samasta osoitteesta kuin rajapinnan (K24, luku 9.6.5). Sovellusten käyttöliittymävaatimukset
määritellään erikseen. Tässä dokumentissa ovat niiden jakelun ja rajapinnan käytön vaatimukset.

Toteutus koskee vain Windows-sovelluksia **HkKisaWin** (henkilökilpailu) ja **ViestiWin** (viesti).
Konsoliohjelmat HkMaali ja JukMaali eivät kuulu toteutukseen (K21, luku 2.3).

Lukurajapinta ei muuta kilpailun tietoja. Syöttötoiminnot (luku 7) ovat oman roolinsa takana, ja
ne toteutetaan vasta vaiheessa 5.

## 2. Nykytila koodissa

### 2.1 Mistä tiedot nyt tulevat

| Tieto | Nykyinen paikka (HK / Viesti) | Huomio |
|---|---|---|
| Status-lukumäärät | `cbHk/UnitStatus.cpp` `TFormStatus::Lukumaarat`; globaalit `nilmt, ntulost, nkeskt, nhylt, neilt, nesitt, emithead, nva[]`, sarjoittain `nilm[], ntulos[][], nkesk[], nhyl[], neil[], neilaika[], palkJaettu[]` | Laskenta ja VCL-ruudukon täyttö samassa silmukassa |
| Yhteyksien tila | `naytayhteydet()` (`comopen, comtype, ipparam, lahettamatta(), cjens`) | |
| Sarjan tilanne | `cbHk/TulosUnit.cpp` `TTilanneForm::PaivitaGrid`; `sijat2()`, `jalku[][]`, `JarjSeur()`, `pkarki[][]`, `kilptietue::tulos2/p_aika/p_sija/ytulos_p/tark`. Viesti: `ViestiWin/TulosUnit.cpp`, taulukot `[sarja][osuus][piste]` | Järjestys, erot ja korostukset muodostetaan suoraan ruudukon `Cells[][]`-merkkijonoiksi |
| Viimeiset tapahtumat | `Tapahtuma[MAXTAPAHTUMA=400]`, `addtapahtuma()` (`cbHk/UnitStatus.cpp:108`, `ViestiWin/UnitMain.cpp:389`); kutsut `Hk/HkDat.cpp:1247`, `Juk/vdat.cpp:795,807` | Puskuri on GUI-yksikössä (`UnicodeString`). Konsoliversioissa (`HkCons0.cpp:79`, `laskent0.cpp:38`) se on tyhjä |
| Maastossa | `cbHk/UnitMaastossa.cpp`, `ViestiWin/UnitMaastossa.cpp` | Lähtöpaikka ratatiedoista (`haerata`) |
| Lähestyjät (viesti) | `ViestiWin/UnitLahestyjat.cpp` | |
| Väliaikapisteet | HK: `sarjatietue::va_matka[pv][va]` (teksti, 22 merkkiä), `va_koodi[pv][va]`, `va_piilota[pv][va]` (julkisuus 0/1–3/10), `va_raja`. Viesti: `va_matka[osuus][va]` (8 merkkiä), `va_koodi[osuus][va]` | Etäisyys on vain tekstinä. Numeerinen etäisyys saadaan ratatiedoista `ratatp.rastikoodi[] / matka[]` rastikoodin perusteella (ohje 2.5) |
| Kilpailijan tunnisteet | HK: `kilptietue::lisno[0]` = kansallinen lisenssi (`type="local"`), `lisno[1]` = IOF-tunniste (`type="IOF"`, `Hk/HkXml_IOF30.cpp:1053`), `wrkoodi` = tekstikoodi (FIS/WRE). Viesti: `osrec::lisno` (vain yksi) | Viestissä ei ole erillistä IOF-tunnistetta |
| Taustatietolinkit | `tietolista.txt` → `HenkLinkki` (ohje 8.4), avaimena lisenssinumero tai maatunnuksellinen kansainvälinen koodi | |
| Ennätys / tavoite | `kilppvtp::enn`, `tav` (`HkEnnTav.cpp`) | Vain HkKisaWin |
| Korostukset | `korostus<pv>.lst` (VIP-luokat 1–3, kärjen sijarajat), ohje 8.3 | |
| Radioleimaukset | `laheta_ec(badge, piste, aika)` `Hk/HkCom32.cpp:498`; radioleimat `tall_etulos` (`Hk/HkIV.cpp:597`). SportIdent-haarassa: SRR/D3 `tallSRRleima` → `tall_etulos` (`Tp/TpLaitteet.cpp`), SI Center `siParsePunch` → `set_tulos`/`tall_lajat_pv` + `tallenna` (`HkIV.cpp`, `VIv.cpp`) | Leima tallentuu kilpailijatietueen aikana. Muille koneille välittyy tietueen päivitys ilman tietoa ajan lähteestä (SI-4) |
| Tietueen tallennus ja jakelu | `kilptietue::tallenna()` → `laheta()` (`Hk/HkDat.cpp:897`, `Hk/HkCom32.cpp:167`) | Muutos lähtee verkon kautta muille koneille, myös erilliseltä koneelta pääkoneelle |

### 2.2 Muut havainnot

- **Laskenta on sidottu lomakkeisiin.** Jos rajapinta laskee tiedot itse, sen luvut voivat
  poiketa ruudusta (V-ARK-1).
- **Pisteiden sisäinen numerointi:** `0` = maali, `1..valuku` = väliajat, `valuku+1` = yhteistulos,
  `valuku+2` = tilan muutos. Rajapinta ei näytä tätä numerointia.
- **Rastikoodi ei ole aina yksikäsitteinen sarjan sisällä.** Hiihdon kierroksilla sama koodi
  toistuu, ja järjestysnumero ratkaistaan aikarajoilla (`va_raja`). Siksi tunnisteena käytetään
  paria (koodi, ohituskerta) (V-VAP-3).
- **Aikayksiköt:** kilpailijatietueen ajat ovat millisekunteja (`CSEK 10`, `KSEK 100`, `SEK 1000`,
  `TUNTI 3600000`, `Tp/TpDef.h`). Aikajonotietueen `aikatp.t` yksikkö on 1/10000 s
  (`AIKAJAK = 10000/SEK = 10`, muunnos `purajak`). Kellonajat ovat suhteessa `t0`:aan (tunteina),
  ja vuorokauden ylitys käsitellään `NORMKELLO`/`NORMTULOS`-makroilla. *(Korjattu luonnoksessa 9:
  aiemmin virheellisesti "1/10000 s".)*
- **Samanaikaisuus:** tulosten käsittely suojataan `tall_CriticalSection`illa.
- **Merkistö:** lähdekoodi on cp1252, merkkijonot `wchar_t` (viestissä osin `char`). JSON tarvitsee UTF-8-muunnoksen.
- **Verkkokerros:** `com/WinUDP.cpp` sisältää Winsock-pohjaisen TCP-palvelimen. HTTP- ja
  WebSocket-palvelinta ei ole. `com/`- ja `tputilv2`-kerrokset käännetään myös konsoliprojekteihin (`VS/`).
- **Testit:** `Tests/` sisältää doctest-kehyksen (`run.sh`).

### 2.3 Konsoliohjelmat rajataan pois (K21)

Rajapinta toteutetaan vain HkKisaWiniin ja ViestiWiniin. Rajaus helpottaa testaamista, eikä
konsoliohjelmille ole käytännön tarvetta. Jaettu domain- ja apukoodi (`Hk/`, `Juk/`, `tputilv2/`,
`com/`) käännetään kuitenkin edelleen myös konsoliprojekteihin (`VS/Hk/HkMaali`, `VS/V/JukMaali520`).
Tästä seuraa yhteensopivuusvaatimus V-ARK-7.

### 2.4 SportIdent-haara (`feature/sportident-reader`)

Haara lisää SportIdent-lukijat (SRR-dongle, online-rastit D3, SI Center REST -haku `SIGPRS`),
kortin numeron uudelleenkäytön askeleen 10 000 000 (`BADGEASKEL` → `BadgeAskel()`), puhtaat
testattavat yksiköt `Tp/`-hakemistoon (`SITulkinta`, `SICenterJson`, `SID3Punch`, `EmitBadge`)
sekä g++- ja MSVC-yksikkötestit CI:hin. Rajapinnan kannalta olennaiset havainnot:

| Havainto | Vaikutus vaatimuksiin |
|---|---|
| Ajan lähde ei välity verkossa, eikä kohdistamaton leima näy muille koneille (SI-4) | V-WS-6, K16, K22 |
| Lähtöleima korvaa lähtöajan aina (SI-2) | V-SYO-3b, V-SYO-4, V-PST-5, V-TUR-4, K23 |
| SI Centerin haku usealta instanssilta tuplaa tallennukset (SI-3) | V-WS-6c |
| Tietue luetaan lukon ulkopuolella (SI-1, myös mainin `tall_etulos`) | V-ARK-6 |
| Rakenteiden pakkaus `tputil.h`/`HkDef.h` ja `char`-etumerkki (SI-9) | V-ARK-9, V-TST-5 |
| Kortin numero tallennetaan siirrettynä (`BadgeAskel`), viestissä indeksimerkki (SI-10) | V-MAL-5 |
| Status näyttää kortit myös SportIdent-kilpailussa (`badgelaji == 'I'`, `SIsana`) | V-STA-2, V-TUR-4 |
| Testattavuussääntö: puhtaat yksiköt `Tp/`-hakemistossa, g++ (32-bit `wchar_t`) ja MSVC | V-TST-5, V-ARK-4, V-YHT-2 |
| Valmis SI Centerin JSON-jäsennin `Tp/SICenterJson.cpp` | V-ARK-4 |
| `httphaku`-katkaisun käsittely (SI-8) | V-SEC-2 |
| Ristiriitaiset muutokset samoissa tiedostoissa (SI-11) | V-ARK-8, vaihe 0 |

## 3. Käyttöympäristö (K2)

- Rajapinta ajetaan **erillisellä koneella**, joka saa kilpailun tilanteen pääkoneelta nykyisellä
  tiedonsiirrolla (UDP/TCP/XML). Rajapintapalvelin on siis verkon asiakas, ei tiedon lähde.
- Tietojen tuoreus riippuu tiedonsiirrosta. Rajapinnan on kerrottava, kuinka tuoretta tieto on (V-API-10).
- Syötöt (luku 7) tehdään paikallisesti ja välitetään pääkoneelle samalla tavalla kuin
  muutokset, jotka tämän koneen käyttöliittymässä tehdään (`tallenna` → `laheta`).
- Palvelu on lähtökohtaisesti suljetussa kilpailuverkossa (luku 8).
- Selainsovelluksia käytettäessä rajapintakoneelle asennetaan edustapalvelin (Caddy), joka jakaa
  sovellukset ja välittää polut `/api/*` sovellukselle (K24, luku 9.6).

## 4. Käyttäjät, roolit ja käyttötapaukset

### 4.1 Roolit (K6)

| Rooli | Tarkoitus | Oikeudet |
|---|---|---|
| `public` | Tulostaulu, striimigrafiikka | Julkiset tulokset ja vain julkiset väliaikapisteet (julkisuustaso 10). Ei henkilötunnisteita |
| `commentator` | Kuuluttaja / CIS | `public` + väliaikapisteet palvelimelle asetetusta julkisuustasosta alkaen, kilpailijatunnisteet (IOF, lisenssi), taustatietolinkit, ennätys ja tavoite, VIP-luokat, WebSocket |
| `safety` | Turvallisuus / tilannekuva | `commentator` + kaikki pisteet julkisuudesta riippumatta, maastossa-listat, emit-/kortin numero, myöhässä olevat, CSV-vienti |
| `operations` | Kilpailunjohto, tekninen tuki | Status mukaan lukien tiedonsiirtoyhteydet ja rajapinnan oma tila |
| `entry` | Tietojen syöttö (vaihe 5) | Ei lähtenyt -merkintä ja uusi lähtöaika. Ei sisällä lukuoikeuksia, vaan ne annetaan erikseen |

Avaimella voi olla useita rooleja. Rooli tarkistetaan jokaisessa päätepisteessä ja WebSocket-tilauksessa.

### 4.2 Käyttötapaukset

| Tunnus | Rooli | Käyttötapaus |
|---|---|---|
| UC-1 | commentator | Sarjan tilanne maalissa tai valitussa rastikoodissa, päivittyy kyselemällä |
| UC-2 | commentator | Rastin (esim. lähestymisrasti) ohitukset viiveettä WebSocketilla, sijoineen ja eroineen |
| UC-3 | commentator | Rastille seuraavaksi odotetut kilpailijat ja arvioitu saapumisaika |
| UC-4 | commentator | Kilpailijan taustatiedot IOF-tunnisteella tai lisenssillä ulkoisesta lähteestä |
| UC-5 | commentator | Kärkivaihdokset ja VIP-ohitukset tapahtumina |
| UC-6 | commentator | Sarjan väliaikapisteet: numero, rastikoodi ja etäisyys lähdöstä |
| UC-7 | commentator | Seuraavat lähtijät sarjoittain |
| UC-8 | safety | Montako kilpailijaa on maastossa, missä sarjoissa ja radoilla, ja missä kukin on viimeksi havaittu |
| UC-9 | safety | Lista myöhässä olevista ja niistä, joiden tila on epäselvä |
| UC-10 | operations | Kokonaistilanne ja tiedonsiirtoyhteyksien tila |
| UC-11 | entry | Ei lähtenyt -merkintä lähdöstä, joka poistaa kilpailijan maastossa-luvusta (vaihe 5) |
| UC-12 | entry | Kilpailijan uusi lähtöaika (myöhästyminen, vaihto) väliaikalähtökilpailussa (vaihe 5) |

## 5. Toiminnalliset vaatimukset: lukurajapinta

Prioriteetit: **M** = pakollinen omassa vaiheessaan, **S** = pitäisi olla, **C** = voi tulla myöhemmin.
Kunkin vaatimuksen domain-kattavuus on **HK+V**, ellei toisin mainita.

### 5.1 Yleiset

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-API-1 | M | Lukupäätepisteet ovat HTTP GET -pyyntöjä, jotka palauttavat JSONia (`application/json; charset=utf-8`). Muut metodit sallitaan vain syöttöpäätepisteissä (luku 7); muuten vastaus on 405. |
| V-API-2 | M | Polut versioidaan: `/api/v1/...`. |
| V-API-3 | M | Kaikki valinnat annetaan pyynnön parametreina (K5). Palvelin ei käytä avoimien ikkunoiden asetuksia. Jokaisella parametrilla on dokumentoitu oletusarvo. |
| V-API-4 | M | Pisteet tunnistetaan vakaasti: `start`, `finish`, `total` (yhteistulos), ja väliaikapiste **rastikoodilla** muodossa `{code}` tai `{code}.{n}` (n:s ohitus, kun koodi toistuu) (K3). Sisäinen järjestysnumero annetaan vain tietokenttänä `index`. |
| V-API-5 | M | Ajat annetaan millisekunteina (`time_ms`, `behind_ms`, `elapsed_ms`) ja näyttömuotoisena merkkijonona (`time`, `behind`) kilpailun tarkkuuden (`laika`) mukaan. Kellonajat ilmoitetaan ISO 8601 -muodossa paikallisaikana aikavyöhykepoikkeamineen. Muunnokset tehdään luvun 2.2 yksiköistä (tietueen ajat ms, aikajono 1/10000 s), ja niille on yksikkötestit. |
| V-API-6 | M | Tila annetaan koodina IOF 3.0 `ResultStatus`-arvoina (`OK, DidNotStart, DidNotFinish, Disqualified, MissingPunch, NotCompeting, OverTime, Active, Inactive`) sekä ohjelman sisäisenä koodina (`H, K, E, M, O, X, B, V`). |
| V-API-7 | M | Virheet palautetaan seuraavasti: 400 virheellinen parametri, 401 puuttuva tai virheellinen avain, 403 rooli ei riitä, 404 tuntematon sarja/koodi/kilpailija, 409 ristiriita (syötöt), 503 tieto ei saatavilla. Vastauksen runko on `{"error": "<code>", "message": "..."}`. |
| V-API-8 | M | Sarjan voi antaa nimellä (`H21`) tai tunnisteella. Myös sarjayhdistelmä (`yhdsarjat`) kelpaa sarjaksi. |
| V-API-9 | M | Nimi ja seura annetaan osina (`first_name`, `last_name`, `club`, `club_short`, `organisation`, `country`). Asiakas päättää esitysmuodon. |
| V-API-10 | M | Jokainen vastaus sisältää `generated`, `stage`, `data_version` (muutoslaskuri, ks. V-VM-1; ei sama asia kuin palvelun versio V-VER-1) ja `source`: `{"mode": "network-client", "last_update": "...", "last_update_age_s": n}`, jotta asiakas näkee, onko tieto vanhentunut. |
| V-API-11 | M | Viestissä päätepisteillä on lisäksi osuus (`leg`) ja kilpailijana joukkue ja osuuden juoksija (`team`, `runner`). HK:ssa osuusparametri hylätään (400). |

### 5.2 Kilpailun perustiedot

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-KIL-1 | M | `GET /api/v1/competition`: nimi, päivämäärä, laji ja alalaji, ohjelmatyyppi (`individual` / `relay`), vaiheiden määrä ja aktiivinen vaihe, palvelimen kellonaika, aktiivinen erä. |
| V-KIL-2 | M | `GET /api/v1/classes`: sarjat: nimi, pitkä nimi, matka, ensimmäinen lähtö, luonne (erillinen / takaa-ajo / yhteislähtö), `hidden_results` (`piilotatulos`), ilmoittautuneiden määrä. Viestissä lisäksi osuuksien määrä ja yhteislähtöajat (`ylahto`). |
| V-KIL-3 | S | Sarjayhdistelmät listataan jäsensarjoineen. |

### 5.3 Väliaikapisteet (K3)

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-VAP-1 | M | `GET /api/v1/classes/{class}/splits` (viestissä `…/legs/{leg}/splits`) listaa sarjan väliaikapisteet järjestyksessä. Kustakin annetaan `index` (järjestysnumero 1..n), `code` (rastikoodi), `occurrence` (ohituskerta), `name` (`va_matka`-teksti), `distance_m` (etäisyys lähdöstä metreinä), `distance_source` ja `visibility` (julkisuustaso). Viimeisenä on maali (`code: "finish"`, `distance_m` = sarjan matka). |
| V-VAP-2 | M | Etäisyys lähdöstä haetaan tässä järjestyksessä: (1) sarjan radan `ratatp.matka[]` kumulatiivisena rastikoodin kohdalta, (2) `va_matka`-tekstistä jäsennetty luku (esim. `3.2 km`, `3200 m`, `3,2`), (3) `null`. `distance_source` kertoo lähteen (`course`, `text`, `none`). Jos sarjalla on useita ratoja (hajonnat), etäisyys annetaan välinä: `distance_min_m` (lyhin rata) ja `distance_max_m` (pisin rata). `distance_m` on silloin `null`. Kilpailijakohtaista etäisyyttä ei anneta (K9). Yhden radan sarjassa `distance_min_m = distance_max_m = distance_m`. |
| V-VAP-3 | M | Jos sama rastikoodi esiintyy sarjassa useasti, kukin esiintymä on oma pisteensä (`{code}.{n}`). Pelkkä `{code}` tarkoittaa ensimmäistä esiintymää. |
| V-VAP-4 | M | `GET /api/v1/controls` listaa kaikki kilpailussa väliaikapisteinä käytetyt rastikoodit. Kustakin annetaan sarjat ja osuudet, joissa koodi on käytössä, sekä kunkin `index` ja `distance_m`. Tämä on perusta pistekohtaisille hauille kaikista sarjoista. |
| V-VAP-5 | M | Roolin mukaiset julkisuussäännöt (luku 8.4) koskevat myös pistelistoja: piilotettu piste ei näy roolille, jolla ei ole siihen oikeutta. |
| V-VAP-6 | S | Rastin koordinaatit annetaan, jos ne on luettu IOF-ratatiedostosta (`IOFPoint`); muuten `null`. |

### 5.4 Status

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-STA-1 | M | `GET /api/v1/status` (rooli `commentator`/`operations`): yhteensä ja sarjoittain ilmoittautuneet, tulokset, keskeyttäneet, hylätyt, ei lähteneet, avoimet ja rastikoodeittain saadut ajat. Viestissä luvut annetaan lisäksi osuuksittain. |
| V-STA-2 | M | Kortteja käyttävässä kilpailussa (`badgelaji` `E` = Emit tai `I` = SportIdent) lisäksi esitetyt (`nesitt`) ja luetut kortit (`emithead`). Kenttänimet ovat järjestelmästä riippumattomia (`cards_presented`, `cards_read`), ja vastauksessa kerrotaan `card_system` (`emit`, `sportident`). |
| V-STA-3 | S | Sarjoittain viimeisimmän tuloksen aika ja palkintojenjaon tila (`palkJaettu`). |
| V-STA-4 | M | `GET /api/v1/status/connections` (rooli `operations`): tiedonsiirtoyhteydet: tyyppi, kohde, suunta, tila, jonossa, lähetetty. |
| V-STA-5 | S | `GET /api/v1/status/api` (rooli `operations`): rajapinnan tila: käynnissäoloaika, pyyntöjä minuutissa, WebSocket-asiakkaiden määrä, viimeisimmät virheet. |
| V-STA-6 | C | Automaattisen tiedostotulostuksen tila. |

### 5.5 Kuuluttaja (CIS): sarjan tilanne

`GET /api/v1/classes/{class}/standings?point=finish|{code}[.n]|total|start|dnf` (viestissä `/classes/{class}/legs/{leg}/standings`)

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-SRJ-1 | M | Palauttaa kilpailijat valitun pisteen järjestyksessä. Riveillä on sija, numero, nimi, seura, maa, lähtöaika, tulos, ero kärkeen ja tila. Järjestys ja sijat ovat samat kuin Tilanne-näytöllä samoilla valinnoilla, koska molemmat käyttävät samaa moduulia (V-ARK-1). |
| V-SRJ-2 | M | Jokaisella rivillä on `result_age_s` (aika tuloksen syntymisestä) tuoreiden tulosten korostusta varten. |
| V-SRJ-3 | M | `point=dnf` palauttaa hylätyt ja keskeyttäneet; `include=dnf,dsq,dns` lisää ne tilanteen loppuun. |
| V-SRJ-4 | M | `splits=all|{code},{code}` lisää riville väliajat: kustakin aika, sija pisteessä ja ero kärkeen. `split_mode=cumulative|leg` valitsee kokonaisajan tai osuusajan (rastiväli tai kierrosaika). |
| V-SRJ-5 | M | Viimeisin saavutettu piste aikoineen ja sijoineen (`last_passed`). |
| V-SRJ-6 | M | `point=start&upcoming=true` palauttaa lähtöä odottavat lähtöjärjestyksessä. Toimii myös sarjayhdistelmälle. |
| V-SRJ-7 | S | `order=smart` (`SmartJarj`): kukin kilpailija on tuoreimman väliaikansa mukaisella paikalla. |
| V-SRJ-8 | S | Matkalla oleville `expected`: arvioitu aika seuraavaan pisteeseen ja sija, johon arvio johtaisi (virtuaalinen sija). Arvio lasketaan mediaanivauhdilla (V-ARV-1). |
| V-ARV-1 | M | **Arvioitu saapumisaika (K10):** kilpailijan viimeisimmän havainnon aikaan lisätään sarjan (viestissä sarjan ja osuuden) **mediaaniaika** havaintopisteeltä arvioitavalle pisteelle. Mediaani lasketaan kilpailijoista, joilla on hyväksytty aika molemmissa pisteissä. Jos otoksia on vähemmän kuin konfiguraation `estimate.min_samples` (oletus 3), käytetään sarjan mediaanivauhtia (min/km) kerrottuna pisteiden välisellä etäisyydellä (5.3). Jos etäisyyttä ei tiedetä, arvio on `null`. Vastauksessa kerrotaan `estimate_method` (`median_leg`, `median_pace`, `none`) ja `estimate_samples`. |
| V-SRJ-9 | S | Monivaiheisessa kilpailussa yhteistulos, yhteissija ja ero, ja takaa-ajossa lähtöaikaero. Viestissä joukkueen kokonaisaika ja sija sekä osuuden juoksijan osuusaika ja -sija. |
| V-SRJ-10 | S | Lajin käyttämät ammunnan sakot ja pisteet. |
| V-SRJ-11 | S | Rivikentät `subclass`, `heat`, `course`; suodatus `heat=`. |
| V-SRJ-12 | S | Rivillä on VIP-korostusluokka (`vip`: 0–3, `korostus<pv>.lst`). |
| V-SRJ-13 | S | `status: "preliminary" | "official"`: tulos on alustava, kunnes sarjan palkintojenjako on merkitty tai tulokset vahvistettu. |

### 5.6 Kuuluttaja (CIS): väliaikapisteen tapahtumat

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-PST-1 | M | `GET /api/v1/controls/{code}/passings?classes=&limit=50&since={seq}` palauttaa pisteen ohitukset uusin ensin: `seq`, kellonaika, numero, nimi, seura, maa, sarja (ja osuus), `index` kyseisessä sarjassa, aika lähdöstä, sija ja ero pisteessä, tila, monivaiheisessa yhteistulos. |
| V-PST-2 | M | `code=all` palauttaa kaikkien pisteiden tapahtumat. |
| V-PST-3 | M | `since={seq}` palauttaa vain uudemmat tapahtumat (pollausta varten). Sisältö on sama kuin WebSocket-viesteissä (V-WS-4). |
| V-PST-4 | M | Tapahtumapuskuri toimii aina, kun rajapinta on päällä, riippumatta avoimista ikkunoista. Puskurin koko on asetettavissa (oletus 5000). |
| V-PST-5 | M | Tapahtumatyypit: `passing` (väliaika tai maali), `status` (hyl., kesk., eil.), `correction` (aiempi aika muuttui), `start_change` (lähtöaika muuttui), `punch` (tallennettu leimaus, V-WS-6). |
| V-PST-6 | S | Johdetut CIS-tapahtumat: `new_leader` (uusi kärki pisteessä), `top_n` (sijoille 1..N, N parametrina), `vip_passing`. |
| V-PST-7 | S | `GET /api/v1/controls/{code}/expected?classes=&window_s=` palauttaa kilpailijat, jotka ovat ohittaneet edellisen pisteen tai lähteneet mutta eivät vielä tätä pistettä. Kustakin annetaan arvioitu saapumisaika (V-ARV-1) ja virtuaalinen sija. Vastaa viestin *Lähestyjät*-näyttöä. |
| V-PST-8 | S | `GET /api/v1/controls/{code}/best?classes=` palauttaa sarjoittain parhaan ajan ja kärjen pisteessä sekä parhaan rastivälin edelliseltä pisteeltä. |

### 5.7 Kilpailija ja taustatiedot (CIS, K8)

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-KLP-1 | M | `GET /api/v1/competitors/{bib}` (viestissä `/teams/{bib}` ja `/teams/{bib}/legs/{leg}`) palauttaa perustiedot, lähtöajan, kaikki väliajat sijoineen ja eroineen, tuloksen ja tilan. Matkalla olevalle annetaan juoksuaika ja virtuaalinen sija. |
| V-KLP-2 | M | Kilpailijan tunnisteet (rooli `commentator`): `ids: {"iof": lisno[1], "national": lisno[0], "text": wrkoodi}`. Kenttä annetaan vain, jos arvo on olemassa. Tunnisteiden avulla CIS yhdistää ulkoiset taustatiedot (IOF Eventor / ranking, kansalliset lisenssirekisterit). |
| V-KLP-3 | M | Haku tunnisteella: `GET /api/v1/competitors?iof_id=…`, `?national_id=…`, `?text_id=…`, `?name=…`. |
| V-KLP-4 | S | `background_url` haetaan `tietolista.txt`-tiedostosta (`HenkLinkki`) samalla avaimella kuin nykyinen *Tietoa*-painike käyttää. |
| V-KLP-5 | S | Ennätys ja tavoite (`personal_best`, `target`), jos ne on tallennettu. |
| V-KLP-6 | S | Rankingpisteet tai arvontaryhmä (`arvo`, `pisteet[]`), jos niitä on käytetty arvonnassa. |
| V-KLP-7 | C | `GET /api/v1/competitors/compare?bibs=a,b` vertailee kahden kilpailijan väliaikoja. |
| V-KLP-8 | M | Viestissä juoksijalla on vain kansallinen tunniste (`osrec::lisno`). Rajapinta antaa `ids.iof = null`. IOF-tunnisteen lisäämisestä on erillinen muutosehdotus (`doc/viesti-iof-tunniste-muutosehdotus.md`), jota ei toteuteta tämän työn yhteydessä (K11). Skeemassa `ids.iof` on mukana jo nyt, jotta muutos ei vaadi rajapinnan versionvaihtoa. |

### 5.8 Turvallisuus: tilannekuva maastossa olevista

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-TUR-1 | M | `GET /api/v1/safety/summary` (rooli `safety`) palauttaa yhteensä, sarjoittain, radoittain ja lähtöpaikoittain: ilmoittautuneet, lähtöaika ohitettu, ei lähteneet, maastossa, maalissa, keskeyttäneet, hylätyt ja epäselvät. *Maastossa* = lähtöaika ohitettu ∧ ei maalitulosta ∧ ei tilaa K/H/E. Viestissä luvut annetaan osuuksittain, ja lisäksi joukkueet, joiden juoksija on maastossa. |
| V-TUR-2 | M | `GET /api/v1/safety/in-forest?class=&course=&start=&last_code=` palauttaa maastossa olevat. Kustakin annetaan numero, nimi, seura, sarja, rata, lähtöpaikka, lähtöaika, juoksuaika, `last_seen` (rastikoodi, kellonaika, etäisyys lähdöstä), `expected_finish` ja kortin numero. |
| V-TUR-3 | M | Myöhässä-merkintä (`overdue: true`), kun juoksuaika ylittää ensimmäisen raja-arvon seuraavista: sarjan aikaraja (`tul_raja`, jos asetettu), `safety.max_time_min` tai sarjan kärkiaika × `safety.leader_factor`. Jos sarjalla ei ole vielä maalituloksia, käytetään vain kahta ensimmäistä. Raja-arvot tulevat rajapinnan konfiguraatiosta (V-KONF-2), ja ne ovat **samat lajista riippumatta** (K12). Pyynnössä `safety`-rooli voi antaa tiukemmat tai väljemmät arvot (`max_time_min`, `leader_factor`) sen pyynnön ajaksi. Käytetty raja ja sen lähde ilmoitetaan vastauksessa. |
| V-TUR-4 | M | Epäselvät (`uncertain`): (a) lähtöaika ohitettu yli `safety.start_grace_min`, mutta kilpailijasta ei ole yhtään havaintoa eikä ei lähtenyt -merkintää; (b) maalileimaus tai maaliaika on, mutta korttia ei ole luettu (`badgelaji` `E` tai `I`). Lähtöleima (SI-2) ja mikä tahansa tallennettu väliaika lasketaan kohdan (a) havainnoksi. |
| V-TUR-5 | M | Tilannekuva ei noudata julkisuustasoja eikä `piilotatulos`-asetusta, koska turvallisuus vaatii kaikki havainnot. Siksi se on vain `safety`-roolin käytössä. |
| V-TUR-6 | M | `GET /api/v1/safety/last-seen` palauttaa maastossa olevien lukumäärät viimeksi havaitun rastikoodin mukaan, eli kuinka moni on minkäkin pisteen jälkeen. |
| V-TUR-7 | S | `format=csv` kaikista `safety`-listoista etsintäorganisaatiolle. |
| V-TUR-8 | S | Aikasarja `GET /api/v1/safety/history?interval_min=5`: maastossa olevien määrä ajan funktiona tapahtumapuskurista ja lähtöajoista laskettuna. |
| V-TUR-9 | M | Tilannekuva näyttää tiedon tuoreuden näkyvästi (V-API-10). Jos tiedonsiirto pääkoneelta on katkennut yli `safety.stale_s` sekuntia, vastaukseen tulee `stale: true`. |
| V-TUR-10 | M | **Vaihtoalueella olevien arvio (viesti).** `GET /api/v1/safety/changeover?class=&leg=` (rooli `safety`) arvioi, kuinka monta juoksijaa on vaihtoalueella. Määritelmät: *avoin* = osuuden juoksijan tila on `-` (Avoinna); *tark-merkintä* = mikä tahansa muu tila (`T, I, H, K, E, P, S`). Arvio lasketaan sarjoittain säännöillä V-TUR-11–13 ja annetaan yhteensä, sarjoittain ja osuuksittain. Kukin juoksija lasketaan enintään kerran. |
| V-TUR-11 | M | **Ennen lähtöä:** kun kellonaika on ennen sarjan lähtöaikaa (`Sarjat[srj].lahto`), vaihtoalueella ovat kaikki sarjan avoimet juoksijat kaikilta osuuksilta. |
| V-TUR-12 | M | **Lähdön jälkeen:** vaihtoalueella ovat osuuksien 2…n avoimet juoksijat, joiden joukkueen edellisellä osuudella ei vielä ole tark-merkintää. Edellisen osuuden tila määritetään joukkueen osuustilana (`kilptietue::tTark(yosuus-1)`), jolloin rinnakkaisosuuksien säännöt (`ekaMaaliRatkaisee`, `nosuus`) toimivat samoin kuin muualla ohjelmassa. Osuuden 1 juoksijoita tämä sääntö ei koske. |
| V-TUR-13 | M | **Maalissa, ei tarkastettu:** vaihtoalueen määrään lisätään aina, myös ennen lähtöä, jokaisen osuuden juoksija, jolla on maaliaika tai maalileimaus (`Maali(os, 0) > 0`), mutta jonka tila on yhä `-`. |
| V-TUR-14 | M | Osuuden juoksija, jonka osuuden yhteislähtöaika (`ylahto[os]`) on jo ohitettu, ei ole vaihtoalueella säännön V-TUR-12 perusteella, koska hän on lähtenyt yhteislähdössä. Hänet lasketaan maastossa oleviin (V-TUR-1). Sääntö V-TUR-13 koskee häntä normaalisti. |
| V-TUR-15 | M | Vastauksessa eritellään säännöittäin `waiting_before_start` (V-TUR-11), `waiting_for_handover` (V-TUR-12), `finished_unchecked` (V-TUR-13) ja `total`. `list=true` palauttaa myös juoksijat (joukkue, osuus, nimi, peruste). Arvio päivittyy tapahtumista samoin kuin muu tilannekuva, ja se noudattaa sääntöä V-TUR-9 (`stale`). |
| V-TUR-16 | S | Jos sarjoilla on eri vaihtoalueet (lähtö- tai maalipaikka `lno`/`mno`), arvio annetaan lisäksi paikoittain (`by_area`). |

### 5.9 WebSocket: leimaukset viiveettä (K4)

Pääkäyttötapa on pollaus (`since`). WebSocket on tarkoitettu väliaikapisteiden tapahtumiin.

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-WS-1 | M | `GET /api/v1/ws` päivitetään WebSocket-yhteydeksi (RFC 6455). Tuetaan vain tekstikehyksiä (JSON), ping/pongia ja sulkemista. Pakkausta ei tueta. |
| V-WS-2 | M | Tunnistautuminen tehdään avaimella päivityspyynnössä (`Authorization`-otsake tai `?key=`, koska selaimen WebSocket ei voi asettaa otsakkeita). Vähimmäisrooli on `commentator`. |
| V-WS-3 | M | Asiakas tilaa tapahtumia viestillä `{"op":"subscribe","controls":["150","151.2","finish"],"classes":["H21"],"types":["passing","new_leader"],"since":12345}`. Tilauksia voi lisätä ja poistaa (`unsubscribe`). |
| V-WS-4 | M | Palvelin lähettää tapahtumat samassa muodossa kuin V-PST-1, jokaisessa `seq`. Jos `since` on annettu ja tapahtumat ovat vielä puskurissa, ne lähetetään ensin. Muuten lähetetään `{"type":"gap"}`, ja asiakas hakee tilanteen REST-rajapinnasta. |
| V-WS-5 | M | Viive siitä, kun tieto on tallennettu tälle instanssille, WebSocket-viestiksi on alle 500 ms. Tapahtuma luodaan samassa kohdassa, jossa `addtapahtuma` nyt kutsutaan (`HkDat.cpp`, `vdat.cpp`), eli tallennuksen jälkeen. Vaatimus ei kata viivettä ennen tallennusta: SI Centerin hakuväli (`SIHAKUVÄLI`), GPRS-yhteys ja tiedonsiirto pääkoneelta. Tapahtumassa on `time_of_day` (leiman aika sellaisena kuin se on tallennettu, SRR:ssä `SRRKORTTIAIKA`-asetuksen mukaan leimasimen tai tietokoneen kello) ja `stored_at` (tallennushetki tällä instanssilla), jotta asiakas näkee kokonaisviiveen. |
| V-WS-6 | M | **Leimaustiedot (K13, K16).** Rajapinta ei avaa omaa yhteyttä leimauslaitteisiin, SI Centeriin tai pääkoneelle. Instanssi saa tiedot olemassa olevilla sovelluksen tiedonsiirto- ja lukijamäärityksillä. Koska ajan lähde ei välity verkossa (SI-4), `punch`-tapahtuma määritellään näin: se syntyy, kun tälle instanssille tallennetussa kilpailijatietueessa **väliaikapisteen** (rastikoodilla tunnistettu piste, `index` ≥ 1) aika asetetaan tai muuttuu. Kortinluennan rastileimat eivät tuota `punch`-tapahtumia, koska ne tallentuvat emit-tietueisiin eivätkä väliaikapisteisiin (tarkistettava, K22). Maali- ja lähtöajan muutokset julkaistaan `passing`- ja `start_change`-tapahtumina, ei `punch`-tapahtumina. Tapahtumassa on rastikoodi, `time_of_day`, `stored_at`, kilpailija ja kortin numero (V-MAL-5) `safety`-roolille. Tapahtuma julkaistaan vasta tallennuksen jälkeen. Tallentamaton, hylätty (esim. `siSaaTallentaa` estää korvauksen) tai kilpailijaan kohdistamaton leima ei tuota tapahtumaa. |
| V-WS-6a | M | Jos leimauksen tallennus johtaa väliaikaan, `punch`- ja `passing`-tapahtumat julkaistaan samalla `seq`-järjestyksellä (`punch` ensin). Asiakas voi tilata kumman tahansa tai molemmat. |
| V-WS-6b | S | Jos samaa leimausta vastaanotetaan useasti (uudelleenlähetys verkossa), tapahtuma julkaistaan vain kerran (kortti + koodi + aika). |
| V-WS-6c | M | **Yksi leimojen hakija.** SI Centerin haku (`SIGPRS`) ja suoraan kytketyt lukijat konfiguroidaan vain yhdelle instanssille (yleensä pääkone). Rajapintakone saa leimat tiedonsiirrossa (SI-3). Rajapinta näyttää kohdassa `GET /api/v1/status/api`, onko tällä instanssilla oma leimalähde, ja kirjaa lokiin varoituksen, jos rajapinta on päällä ja `SIGPRS` on asetettu. |
| V-WS-7 | M | Palvelin lähettää pingin 30 s välein ja sulkee yhteyden, jos pongia ei tule. Yhteyksien enimmäismäärä on asetettavissa (oletus 32). Hidas asiakas katkaistaan, kun sen lähetysjono ylittää rajan, eikä se saa hidastaa muita. |
| V-WS-8 | M | Viestit kuvataan AsyncAPI 3.0 -dokumentilla. OpenAPI-dokumentissa on päivityspäätepiste ja viittaus AsyncAPI-dokumenttiin. |

### 5.10 Päivittyminen (pollaus)

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-PAIV-1 | M | Palvelin ylläpitää muutoslaskuria kokonaisuutena sekä sarjoittain ja osuuksittain (V-VM-1). Vastauksissa on `ETag` (V-VM-7). `If-None-Match` palauttaa 304, jos tieto ei ole muuttunut. |
| V-PAIV-2 | M | 50 asiakasta voi kysellä kerran sekunnissa eri sarjoja tai osuuksia ilman, että tiedonsiirto hidastuu (V-SUO-1, V-SUO-2, V-KUO-*). |

### 5.11 Tulosluettelot: HTML, JSON, PDF ja IOF XML

Taustaa: ohjelmassa on jo yhteinen tulostusputki. `tulostusparamtp` määrittää kohteen (`kohde`:
`P` kirjoitin, `H` HTML, `M` mobiili-HTML, `X` XML), rajaukset ja muotoilun (`tulosmuottp`,
kenttämääritykset `aFld`, muotoilutiedosto `muottied`). `tulostasarja()` (`Hk/HkTulostus.cpp:5784`,
`Juk/VTulostus.cpp:5680`) tulostaa sarjan mihin tahansa kohteeseen. PDF syntyy nyt vain
kirjoitinajurin kautta (`GDIPRINTER`, esim. *Microsoft Print to PDF*, ohje 13.5). IOF XML 3.0
-vienti on toteutettu funktioissa `xmlIOF30ots/srjots/tulos/loppu` (`Hk/HkXml_IOF30.cpp:952–1037`,
`Juk/VXml_IOF30.cpp:938–1014`). Huomio: `tulostasarja()` kutsuu joissain tilanteissa
`sijat()`/`sijat2(srj, true, …)`, jotka tallentavat muuttuneet sijat ja lähettävät ne verkkoon
(`HkTulostus.cpp:5963–5966`). Lisäksi `tulostasarja()` pitää `tall_CriticalSection`ia koko
sarjan tulostuksen ajan (`HkTulostus.cpp:5850–6518`). Erillinen `print_CriticalSection` sarjallistaa
tulostukset (`Juk/VTulostus.cpp:723`). HTML-rivit muodostetaan kenttämääritysten mukaan
(`aFld[]`: `id`, `race`, `point`, `title`). Kunkin solun arvo tulee funktiosta `kilptietue::strval()`
(`HkTulostus.cpp` `htmltulos`, rivit 4126–4349; viestissä `Juk/VTulostus.cpp` `htmltulos`/`htmlkaikki*`).

**Päätepisteet (ehdotus):**

| Muoto | HK | Viesti |
|---|---|---|
| HTML | `GET /api/v1/results/classes/{class}.html` | `…/classes/{class}.html` (joukkueet), `…/classes/{class}/legs/{leg}.html` (osuus) |
| JSON | `GET /api/v1/results/classes/{class}.json` | kuten HTML, päätteellä `.json` |
| PDF | `GET /api/v1/results/classes/{class}.pdf` | kuten HTML, päätteellä `.pdf` |
| Useita sarjoja | `GET /api/v1/results.{html,json,pdf}?classes=…` | samoin, lisäksi `legs=` |
| IOF XML 3.0 | `GET /api/v1/results/iof.xml` (koko kilpailu) | `GET /api/v1/results/iof.xml` (koko kilpailu, `TeamResult`) |

#### 5.11.1 Yleiset

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-TLS-1 | M | Tulosluettelot muodostetaan sovelluksen olemassa olevalla tulostusputkella (`tulostusparamtp` + `tulostasarja`), ei erillisellä rajapintakohtaisella muotoilulla. Näin rajapinnan tulosluettelo vastaa sisällöltään ja järjestykseltään sovelluksesta tulostettua luetteloa samoilla valinnoilla. |
| V-TLS-2 | M | **Vain lukeva muodostus.** Rajapinnan kautta tehty muodostus ei saa tallentaa tietueita, kirjoittaa kilpailutiedostoja eikä lähettää tiedonsiirtosanomia. Tulostusputkeen lisätään lukutila (esim. `tulostusparamtp`-lippu), jossa `sijat()`/`sijat2()` eivät tallenna. Sijat otetaan lukumallista (V-VM-3) tai lasketaan muistiin tallentamatta. Todennetaan testillä (V-TST-4). |
| V-TLS-3 | M | **Muotoiluasetukset.** Rajapinnalla ei ole omia muotoiluasetuksia. Se käyttää sovellukseen tallennettuja muotoiluja: tulosmuotoilu `tulosmuottp` (fontit, marginaalit, sivun pituus, värit, CSS, otsikot), kenttämääritykset ja muotoilutiedosto (`muottied`) sekä lisätekstit (`paa_alkuteksti`, `sivu_alkuteksti`, `sivu_alateksti`, `lopputeksti`). Rajapinnan konfiguraatiossa voidaan nimetä käytettävä muotoilutiedosto erikseen HTML:lle ja PDF:lle (`documents.html.format_file`, `documents.pdf.format_file`). Oletuksena käytetään sovelluksen nykyisiä asetuksia. |
| V-TLS-4 | M | Muodostus tehdään yhdessä dokumenttityösäikeessä jonona, koska tulostusputki käyttää jaettua tilaa (`tulostusparamtp`, puskurit, GDI). Työsäie varaa `print_CriticalSection`in, jotta muodostus ei törmää sovelluksen omaan tulostukseen. Jonon pituus rajataan (`documents.queue_max`, oletus 10). Täyteen jonoon vastataan 503 + `Retry-After`, ellei välimuistissa ole palautettavaa versiota (V-TVM-4). Yksittäisen muodostuksen aikaraja on `documents.timeout_s` (oletus 30 s). |
| V-TLS-5 | M | **Välimuisti.** Dokumentit tallennetaan välimuistiin ja muodostetaan luvun 9.5 mukaisesti (V-TVM-1–14). |
| V-TLS-6 | M | Julkisuussäännöt (8.4) koskevat dokumentteja: piilotetut väliaikapisteet ja `piilotatulos`-sarjat jätetään pois roolin mukaan. Jos muotoilussa on kenttä, jota rooli ei saa nähdä (esim. kortin numero `F_BADGE`, lisenssinumerot `F_LISNO`/`F_LISNO2` roolille `public`), sarake poistetaan kaikista muodoista. Henkilötiedot rajataan kuten V-JUL-3, myös IOF XML:ssä (ei syntymäaikaa; tunnisteet vain `commentator`-roolille). Vähimmäisrooli on `public`. |
| V-TLS-7 | M | Vastauksessa on `Content-Type` (`text/html; charset=utf-8`, `application/json; charset=utf-8`, `application/pdf`, `application/xml; charset=utf-8`) ja `Content-Disposition`, jossa on kuvaava tiedostonimi (esim. `tulokset_H21_maali.pdf`). `download=false` näyttää dokumentin selaimessa. |
| V-TLS-8 | M | Tulosluettelossa näkyy muodostusaika ja tieto, onko tulos alustava vai virallinen (V-SRJ-13). Näin kopioitu tai tallennettu luettelo ei näytä lopulliselta, jos se on alustava. |

#### 5.11.2 Rajaukset (HTML, JSON ja PDF)

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-TLS-10 | M | HTML-, JSON- ja PDF-luettelot voi rajata sarjan (myös sarjayhdistelmän) ja viestissä osuuden lisäksi seuraavilla parametreilla, jotka vastaavat `tulostusparamtp`-kenttiä: `point` (maali tai väliaikapisteen rastikoodi, `piste`), `stage` ja `combined` (vaihe ja yhteistulos, `vaihelkm`), `from_rank`/`to_rank` (`enssija`/`viimsija`), `club`/`district` (`rajaus`, seura- ja piirirajaus), `subclass`, `age_class` (`ikasarjat`), `sex` (`sukuprajaus`), `splits=all` (`kaikkiva`), `include=dnf,dsq,dns,unchecked` (`tarkastamattomat`), `start_times` (`lahtoajat`) ja `lang` (`language`). |
| V-TLS-11 | M | Viestissä `legs/{leg}` tuottaa osuustulokset ja pelkkä `classes/{class}` joukkuetulokset. `legs=all` tuottaa joukkuetulokset kaikkien osuuksien ajoilla. |
| V-TLS-12 | M | Useamman sarjan dokumentissa sarjat ovat pyydetyssä järjestyksessä tai oletuksena sovelluksen sarjajärjestyksessä. PDF:ssä kukin sarja voi alkaa uudelta sivulta (`page_break=class`), muuten sovelluksen asetusten mukaan. |
| V-TLS-13 | S | `GET /api/v1/results/options` palauttaa käytettävissä olevat muotoiluprofiilit, kielet ja rajausparametrien sallitut arvot, jotta asiakas voi rakentaa valintalistat. |

#### 5.11.3 HTML

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-TLS-20 | M | HTML muodostetaan samalla tavalla kuin sovelluksen HTML-tiedostotulostus (`kohde` `H`, `htmlalku`/`htmlloppu`, `tulosmuottp`-värit, CSS ja taulukkomuoto). `style=mobile` käyttää mobiilimuotoilua (`kohde` `M`). |
| V-TLS-21 | M | Merkistö on UTF-8, ja sivun `<meta charset>` vastaa sitä. |
| V-TLS-22 | S | `fragment=true` palauttaa pelkän tulostaulukon ilman `<html>`/`<head>`-osia upotettavaksi toiseen sivuun. |
| V-TLS-23 | S | Sarjalinkit (`sarjalinkit`) osoittavat monen sarjan dokumentissa sivun sisäisiin ankkureihin, eivät tiedostoihin. |

#### 5.11.4 PDF

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-TLS-30 | M | **PDF syntyy sovelluksen kirjoitintulostuksen kautta (K19: Windowsin PDF-kirjoitinputki toistaiseksi).** Muodostus käyttää olemassa olevaa GDI-kirjoitinputkea (`printer = GDIPRINTER`, `startdocGDI`, `putwfldGDI`, `endpageGDI`), ja kohteena on konfiguraatiossa nimetty PDF-kirjoitin (`documents.pdf.printer`, oletus *Microsoft Print to PDF*). Tulosteen polku annetaan `DOCINFO.lpszOutput`-kentässä (nyt `NULL`, `tputilv2/Wrttext.cpp:734`), jolloin tallennusikkunaa ei avata. Näin PDF:n ulkoasu on sama kuin paperituloksissa: kenttien sijainnit, fontit, sivutus, ylä- ja alatekstit. Myös Unicode-nimet tulostuvat oikein Windowsin fonteilla. |
| V-TLS-31 | M | PDF tehdään väliaikaistiedostoon rajapinnan työhakemistossa (`documents.temp_dir`). Tiedosto palautetaan ja poistetaan muodostuksen jälkeen, ja välimuistiin jää vain tavujono. Kesken jääneet tiedostot siivotaan käynnistyksessä. |
| V-TLS-32 | M | Jos PDF-kirjoitinta ei ole asennettu tai muodostus epäonnistuu, vastaus on 503 (`error: "pdf_unavailable"`) ja syy kirjataan lokiin. Rajapinnan tila (V-STA-5) kertoo, onko PDF käytettävissä. |
| V-TLS-33 | M | Rajapinnan PDF-muodostus ei saa näkyä käyttäjän kirjoitinjonossa eikä häiritä samanaikaista tulostusta sovelluksesta. Jos sovellus tulostaa samaan aikaan, muodostus odottaa (V-TLS-4). |
| V-TLS-34 | M | PDF-muodostus on rajattu kerrokseen (esim. `PdfOutput`), jotta GDI-ajurin tilalle voidaan myöhemmin vaihtaa oma PDF-kirjoitin (uusi `printer`-tyyppi tulostusputkeen) muuttamatta rajapintaa. |
| V-TLS-35 | C | Oma PDF-kirjoitin ilman ajuririippuvuutta (PDF 1.4, upotettu TrueType-osajoukko Unicode-nimiä varten), joka käyttää samoja kenttä- ja sivumäärityksiä. Ei toteuteta nyt (K19). Harkitaan, jos ajuripohjainen ratkaisu osoittautuu käytössä riittämättömäksi. |

#### 5.11.5 IOF XML 3.0

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-TLS-40 | M | IOF XML 3.0 `ResultList` tuotetaan **vain koko kilpailusta**. Sarja- tai osuusrajauksia ei tueta (400, jos niitä annetaan). HK käyttää `PersonResult`- ja viesti `TeamResult`-rakennetta nykyisillä vientifunktioilla. |
| V-TLS-41 | M | `ResultList@status` on `Snapshot`, kun kilpailu on kesken, ja `Complete` vasta, kun kaikkien sarjojen tulokset on vahvistettu (V-SRJ-13). Nykyinen vienti kirjoittaa aina `Complete` (`HkXml_IOF30.cpp`, `createTime`/`status`-otsake). Lisäksi `createTime` ja `creator` täytetään. |
| V-TLS-42 | M | Parametrit: `stage` (vaihe), `splits=true|false` (väliajat `SplitTime`, oletus `true`). Väliajat noudattavat julkisuussääntöjä (V-TLS-6). |
| V-TLS-43 | M | Koko kilpailun XML muodostetaan enintään kerran `documents.iof_min_interval_s` aikana (oletus 30 s) ja vain, jos koko kilpailun versio on muuttunut. Muulloin palautetaan välimuistissa oleva versio (`ETag`, `Last-Modified`). |
| V-TLS-44 | M | Tiedosto validoidaan testeissä IOF XML 3.0 -skeemaa vasten (V-TST-4). |
| V-TLS-45 | S | Henkilötunnisteet kirjoitetaan `type`-attribuutein (`IOF`, `local`) kuten HK:n viennissä. Viestin osalta tämä riippuu muutosehdotuksesta K11. |

#### 5.11.6 JSON (HTML-tulosteen tietosisältö)

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-TLS-50 | M | JSON-tulosluettelon tietosisältö vastaa HTML-tulostetta samoilla parametreilla. Siinä ovat samat sarjat, samat rivit samassa järjestyksessä, samat sarakkeet samassa järjestyksessä ja samat arvot samassa muotoilussa (esim. `"42.23"`, `"+1.05"`, `"Hyl."`). Mukana ovat myös sarjan otsakerivi (nimi, matka, lukumäärät), väliotsikot (esim. keskeyttäneet ja hylätyt) sekä ylä- ja lopputekstit. Se ei sisällä esitystietoja (värit, CSS, fontit). |
| V-TLS-51 | M | **Sama putki.** JSON muodostetaan tulostusputken uutena kohteena (esim. `kohde` `J`). Sarakkeet ja arvot otetaan samasta kenttäsilmukasta (`aFld[]` + `kilptietue::strval()`), josta HTML-solut kirjoitetaan. Sisältöä ei lasketa erikseen. Näin JSON ja HTML eivät voi erota sisällöltään. |
| V-TLS-52 | M | Rakenne: `document` (otsikko, muodostusaika, `status` alustava/virallinen, versio, tekstit), `classes[]`. Kussakin sarjassa: `name`, `leg` (viesti), `header` (HTML-otsakerivin tiedot), `columns[]` ja `sections[]`. Sarakkeessa on `key`, `title` (sama otsikko kuin HTML:ssä), `field` (sisäinen kenttätunnus), `stage` (`race`) ja `point` (rastikoodi tai `finish`/`total`). Osiossa on `title` (tyhjä tai esim. "Keskeyttäneet") ja `rows[]`. Rivi on olio `{key: arvo}` sarakkeiden mukaan. |
| V-TLS-53 | M | Sarakeavaimet ovat englanninkielisiä ja vakaita. Niiden vastaavuus sisäisiin kenttätunnuksiin dokumentoidaan OpenAPI-skeemassa. Ehdotus (HK `HkMuotoilu.h`, viesti `VMuotoilu.h`): `F_SJ` → `rank`, `F_KNO` → `bib`, `F_NIMI` → `name`, `F_ETUNIMI` → `first_name`, `F_SUKUNIMI` → `last_name`, `F_ARVO` → `title`, `F_LISNO` → `national_id`, `F_LISNO2` → `iof_id`, `F_BADGE` → `card`, `F_SRA` → `club`, `F_SRALYH` → `club_short`, `F_MAA` → `country`, `F_YHD` → `organisation`, `F_JOUK` → `team`, `F_ALISRJ` → `subclass`, `F_TLS` → `result`, `F_ERO` → `behind`, `F_SAK` → `penalty`, `F_TARK` → `status`, `F_AIKA` → `time`, `F_PIST` → `points`, `F_OSASJ` → `stage_rank`, `F_OSATLS` → `stage_result`. Viestin lisäkentät: `F_EDSJ` → `previous_rank`, `F_JOUKNM` → `team_name`, `F_JOUKID` → `team_id`, `F_RATA` → `course`, `F_OSSRA` → `runner_club`, `F_SEURSJ` → `club_rank`, `F_VSJ` → `leg_rank`, `F_OSOTS` → `leg_title`. Jos sama kenttä on useita kertoja eri pisteille tai vaiheille, avaimeen lisätään tarkenne (esim. `result@101`, `result@stage2`). |
| V-TLS-54 | S | `typed=true` lisää riville `values`-olion, jossa ovat samat tiedot tyypitettyinä (`time_ms`, `rank`, `status`-koodi V-API-6). Muotoillut merkkijonot säilyvät ennallaan. |
| V-TLS-55 | M | JSON-tulosluettelo on eri asia kuin elävä `standings`-rajapinta (5.5). Tulosluettelo kuvaa julkaistavan tulosteen, ja `standings` antaa tietomallin kuuluttajalle. Molemmat dokumentoidaan erikseen. |

### 5.12 Palvelun versio ja rajapintakuvaukset

Tavoite: asiakas voi selvittää ilman erillistä dokumentaatiota, mikä palvelu ja versio vastaa, mitä
rajapinnan versioita ja ominaisuuksia se tukee, ja hakea koneluettavan kuvauksen juuri siitä
toteutuksesta, joka on käynnissä. Kuvaus ei siis ole verkkosivulla oleva, mahdollisesti eri version
dokumentti.

#### 5.12.1 Versio

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-VER-1 | M | `GET /api/version` (versioimaton polku, jotta asiakas löytää sen ennen kuin tietää tuetut versiot) ja sama sisältö polussa `GET /api/v1/version`. Vastaus: `service` (`tulospalvelu-rest`), `application` (`HkKisaWin`/`ViestiWin`), `domain` (`individual`/`relay`), `app_version` (`VERSIOKDI`, nyt `5.21`), `app_build` (`BUILD`, nyt `5.22.002/2026-07-31`, `Hk/BuildNo.h`), `git_commit` (lyhyt tiiviste, jos käännöksessä annettu, muuten `null`), `api_version` (semanttinen versio, esim. `1.0.0`), `api_versions_supported` (`["v1"]`), `openapi_version` ja `asyncapi_version` (kuvausten `info.version`), `websocket_protocol` (V-VER-5), `features` (V-VER-4), `started_at`, `uptime_s` ja `links` (V-SPEC-7). |
| V-VER-2 | M | `api_version` noudattaa semanttista versiointia. Patch: korjaus ilman sopimusmuutosta. Minor: taaksepäin yhteensopiva lisäys (uusi päätepiste, uusi valinnainen parametri, uusi kenttä, uusi tapahtumatyyppi). Asiakkaan on ohitettava tuntemattomat kentät ja tapahtumatyypit. Major: yhteensopimaton muutos, joka julkaistaan uudessa polussa (`/api/v2/`). Edellistä majoria tuetaan vähintään yhden julkaisun ajan rinnakkain. |
| V-VER-3 | M | Jokaisessa HTTP-vastauksessa on otsake `X-Api-Version: <api_version>`. Poistuvaksi merkityn päätepisteen vastauksessa on lisäksi `Deprecation`- ja `Sunset`-otsakkeet ja `Link`-viittaus korvaavaan päätepisteeseen. |
| V-VER-4 | M | `features` kertoo tämän instanssin käytössä olevat ominaisuudet, koska ne riippuvat käännöksestä ja asetuksista: `websocket`, `entry` (V-KONF `entry.enabled`), `documents.html`, `documents.json`, `documents.pdf` (PDF-kirjoitin löytyi), `documents.iof_xml`, `safety`, `sportident` (käännetty `SPORTIDENT`-ehdolla), `auth` (`on`/`off`). Asiakas päättelee käytettävissä olevat toiminnot tästä eikä virhevastauksista. |
| V-VER-5 | M | WebSocket: päivityspyyntö voi ehdottaa aliprotokollaa `Sec-WebSocket-Protocol: tulospalvelu.v1`. Palvelin hyväksyy tunnetun aliprotokollan ja hylkää tuntemattoman. Jos aliprotokollaa ei ehdoteta, käytetään oletusta `tulospalvelu.v1`. Ensimmäinen palvelimen viesti yhteyden avauduttua on `{"type":"hello","api_version":"1.0.0","app_version":"5.21","app_build":"…","protocol":"tulospalvelu.v1","stage":1,"seq":<viimeisin>,"heartbeat_s":30}`. Asiakas tietää siitä, mistä `seq`-arvosta jatkaa (V-WS-4). |
| V-VER-6 | M | Versiotiedot ja kuvaukset (5.12.2) ovat luettavissa ilman avainta myös, kun `auth.mode=on`, koska ne eivät sisällä kilpailun tietoja (asetus `docs.public`, oletus kyllä). `git_commit`, `started_at` ja `uptime_s` näytetään vain roolille `operations`, kun `docs.public_details=false`. |
| V-VER-7 | M | `GET /api/health`: kevyt tilakysely edustapalvelimelle ja valvonnalle (9.6). Vastaus 200 `{"status":"ok"}` tai 503 `{"status":"degraded","reasons":[…]}`, esim. kun tiedonsiirto on katkennut yli `safety.stale_s` (V-API-10) tai lukumallin rakennus on jumissa. Kysely ei ota `tall_CriticalSection`-lukkoa. Se ei vaadi avainta, eikä sitä lasketa avainkohtaiseen nopeusrajaan. |

#### 5.12.2 Rajapintakuvaukset

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-SPEC-1 | M | Palvelu palauttaa kuvaukset: `GET /api/v1/openapi.json`, `GET /api/v1/openapi.yaml` (OpenAPI 3.1, REST), `GET /api/v1/asyncapi.json` ja `GET /api/v1/asyncapi.yaml` (AsyncAPI 3.0, WebSocket). Sisältötyypit: `application/json`, `application/yaml` (YAML myös `charset=utf-8`). |
| V-SPEC-2 | M | **Yksi lähde.** Kuvausten lähde on versionhallinnassa (`doc/api/openapi.yaml`, `doc/api/asyncapi.yaml`, V-DOK-1). Käännös sulauttaa ne ohjelmaan resursseina (RC-tiedoston `RCDATA`), joten käynnissä oleva ohjelma palauttaa aina oman käännöksensä kuvauksen eikä lue tiedostoja levyltä. JSON-muoto tuotetaan käännösvaiheessa YAML-lähteestä tai lähde kirjoitetaan YAML:n JSON-yhteensopivaan osajoukkoon. JSON-muotoa ei ylläpidetä käsin. |
| V-SPEC-3 | M | Palvelin täydentää kuvauksen ajonaikaisesti vain seuraavilta osin: `servers` (OpenAPI) ja `servers` (AsyncAPI) muodostetaan asetuksesta `server.public_base_url` tai, jos sitä ei ole, pyynnön `Host`-otsakkeesta (vain luotetulta edustapalvelimelta tulevat `X-Forwarded-Host`/`-Proto` huomioidaan, V-SEC-5); `info.x-application` ja `info.x-app-build` (V-VER-1). Muu sisältö on sama kuin resurssissa, jotta ETag pysyy vakaana. |
| V-SPEC-4 | M | Kuvaus kattaa kaikki toteutetut päätepisteet, myös ne, jotka asetuksilla on suljettu (esim. syötöt). Suljettu ominaisuus näkyy laajennuskentässä `x-feature` (V-VER-4), ja sen tila luetaan `/api/version`-vastauksesta. Kuvaus on siis käännöskohtainen, ei asetuskohtainen. Roolit kuvataan `security`-määrittelyinä ja laajennuskentässä `x-roles`. |
| V-SPEC-5 | M | Kuvausvastauksissa on `ETag` (sisällön tiiviste) ja `Cache-Control: no-cache`, joten asiakas voi tarkistaa muutoksen ehdollisella pyynnöllä (304). |
| V-SPEC-6 | S | `GET /api/docs` palauttaa itsenäisen HTML-sivun ilman ulkoisia skriptejä: palvelun versio, linkit kuvauksiin, luettelo päätepisteistä (polku, metodi, roolit, kuvaus) ja WebSocket-viestityypeistä. Sivu muodostetaan kuvauksesta käännösvaiheessa ja sulautetaan resurssina. Interaktiivinen katselin (esim. Swagger UI tai Redoc) ei kuulu sovellukseen. Sen voi tarjota edustapalvelin staattisena sisältönä (V-PRX-6). |
| V-SPEC-7 | M | Juuripolku `GET /api` ja `GET /api/v1` palauttavat linkkiluettelon (`version`, `health`, `openapi`, `asyncapi`, `docs`, `ws`), jotta asiakas löytää kuvaukset yhdestä osoitteesta. |
| V-SPEC-8 | M | AsyncAPI-kuvauksessa on yhteys `/api/v1/ws`, aliprotokolla (V-VER-5), tilausviestit (V-WS-3), kaikki tapahtumatyypit skeemoineen (`passing`, `new_leader`, `gap`, `hello`, `error` ym.) ja avaimen välitystavat (V-WS-2). OpenAPI-kuvauksessa on päivityspäätepiste ja viittaus AsyncAPI-kuvaukseen (V-WS-8). |

## 6. Kenttien ja tietojen yhteinen malli

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-MAL-1 | M | Kilpailijarivin yhteinen perusrakenne (`CompetitorRef`): `bib, first_name, last_name, club, club_short, country, class, leg?, team?` on sama kaikissa päätepisteissä. |
| V-MAL-2 | M | Tulosrakenne (`PointResult`): `point, code, index, time_ms, time, rank, behind_ms, behind, status`. |
| V-MAL-3 | M | Kentät, joita ei ole saatavilla, annetaan arvolla `null`. Kenttiä ei jätetä pois, ellei rooli rajaa niitä. |
| V-MAL-4 | M | Kaikki skeemat kuvataan OpenAPI 3.1 -dokumentissa englanniksi. Dokumentti sisältää esimerkit ja roolivaatimukset (`security`) päätepisteittäin. |
| V-MAL-5 | M | **Kortin numero (SI-10).** Kortin numero (`card`) annetaan kortin todellisena numerona, samalla tulkinnalla kuin sovelluksen näyttö (`BadgeTeksti`/`siNaytettavaBadge`, askel `BadgeAskel()` kilpailun `badgelaji`n mukaan, viestissä myös indeksimerkki). Tallennettua siirrettyä arvoa ei paljasteta. Korttihaku (`safety`) tehdään todellisella numerolla. Jos sama kortti on usealla kilpailijalla, kaikki palautetaan. Kenttä on kokonaisluku (SportIdent-numerot voivat ylittää 9 999 999). |

## 7. Toiminnalliset vaatimukset: tietojen syöttö (vaihe 5)

Syöttötoiminnot ovat `entry`-roolin takana ja toteutetaan myöhemmässä julkaisussa. Ne
määritellään nyt, jotta arkkitehtuuri (tallennus, roolit, audit-loki) tehdään valmiiksi
oikein. Ennen toteutusta päätepisteet palauttavat 501.

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-SYO-1 | M | `POST /api/v1/competitors/{bib}/dns` merkitsee kilpailijan ei lähteneeksi aktiivisessa vaiheessa (`set_tark(L'E')`). Viestissä `POST /api/v1/teams/{bib}/legs/{leg}/dns` merkitsee osuuden juoksijan. |
| V-SYO-2 | M | `DELETE /api/v1/competitors/{bib}/dns` peruu merkinnän, jos kilpailijan tila on edelleen `E` ja muutos on tehty rajapinnan kautta tai `force=true`. |
| V-SYO-3 | M | `PUT /api/v1/competitors/{bib}/start-time` rungolla `{"start": "13:42:30", "reason": "late"}` asettaa uuden lähtöajan (`tall_laika` / vaihekohtainen `tlahto`). |
| V-SYO-3a | M | **Uusi lähtöaika sallitaan vain väliaikalähtökilpailuissa (K14):** kilpailijan sarjalla on aktiivisessa vaiheessa lähtöväli (`Sarjat[srj].lahtovali[k_pv] != 0`), eikä sarja ole takaa-ajo (`luonne[k_pv] != 2`). Lähtöporttilähtö (`hiihtolahto`) on väliaikalähtö. Yhteislähtö, takaa-ajo ja viesti hylätään vastauksella 422 (`error: "not_interval_start"`). Viestissä päätepistettä ei ole. `force` ei ohita tätä tarkistusta. |
| V-SYO-4 | M | Syöttö hylätään (409), jos kilpailijalla on jo maalitulos (DNS), tai jos hänellä on väliaika tai maaliaika, joka on ristiriidassa uuden lähtöajan kanssa (lähtöaika). Uusi lähtöaika hylätään myös, jos kilpailijalla on toteutunut lähtöaika (lähtöleima tai lähtöportin aika), joka poikkeaa arvotusta lähtöajasta (`error: "actual_start_exists"`). `force=true` ohittaa tarkistuksen vain, jos avaimella on oikeus `entry:force`. |
| V-SYO-3b | M | **Lähtöleiman ja syötön järjestys (SI-2).** SportIdent-haaran nykyinen sääntö on, että myöhemmin tuleva lähtöleima korvaa lähtöajan aina, myös rajapinnan kautta asetetun. Rajapinta ei estä tätä, mutta tunnistaa tilanteen: jos rajapinnan kautta asetettu lähtöaika muuttuu myöhemmin muuta kautta, siitä syntyy `start_change`-tapahtuma, jossa on `previous_source: "api"`, ja merkintä audit-lokiin. Lopullinen sääntö päätetään (K23). |
| V-SYO-5 | M | Optimistinen lukitus: pyyntö sisältää kilpailijan `ETag`-arvon (`If-Match`). Jos tietue on muuttunut välissä, vastaus on 412. |
| V-SYO-6 | M | Muutos tallennetaan samalla tavalla kuin käyttöliittymän muutos (`kilptietue::tallenna` → `laheta`), jolloin se välittyy pääkoneelle ja muille koneille nykyisellä tiedonsiirrolla. Rajapinta ei kirjoita tiedostoon ohi tämän polun. |
| V-SYO-7 | M | Jos koneelta ei ole lähtevää yhteyttä pääkoneelle (tai lähetys on kielletty), syöttö hylätään (503). Paikallinen muutos, joka ei leviä, ei ole sallittu. |
| V-SYO-8 | M | Jokainen syöttö kirjataan audit-lokiin: aika, avaimen tunnus, kilpailija, vanha ja uusi arvo, syy, tulos. Loki on luettavissa `operations`-roolilla (`GET /api/v1/audit`). |
| V-SYO-9 | M | Syöttö näkyy tapahtumavirrassa tyyppinä `status` tai `start_change` (V-PST-5), jolloin tilannekuva päivittyy heti. |
| V-SYO-10 | M | Syöttöpäätepisteet ovat käytössä vain, kun `entry.enabled` on päällä (V-KONF-2), vaikka avaimella olisi `entry`-rooli. Oletuksena ne ovat pois päältä. `force` edellyttää lisäksi asetusta `entry.allow_force`. |
| V-SYO-11 | S | Erätoiminto `POST /api/v1/dns` listalle numeroita (lähdön ei lähteneet -lista). Rivikohtainen tulos palautetaan. |

## 8. Turvallisuus, tunnistautuminen ja julkisuus

### 8.1 Tunnistautuminen (K6)

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-AUT-1 | M | Sovelluskohtaiset API-avaimet määritellään rajapinnan konfiguraatiossa (`restapi.xml`, V-KONF-2). Kullakin avaimella on tunnus (`client_id`), kuvaus, roolit ja valinnaisesti sallitut IP-osoitteet ja voimassaoloaika. Avaimesta tallennetaan tiiviste (SHA-256), ei selkotekstiä. |
| V-AUT-2 | M | Avain välitetään otsakkeessa `Authorization: Bearer <key>` tai `X-Api-Key`. WebSocketissa sallitaan myös `?key=`. |
| V-AUT-3 | M | Tunnistautumisen voi kytkeä pois suljetussa verkossa (`auth=off`). Silloin jokaisella pyynnöllä on asetuksen `auth.anonymous_roles` roolit (oletus `public, commentator`). Kun `auth.mode=on`, ilman avainta tulevalla pyynnöllä on vain `anonymous_roles`-roolit. `safety`, `operations` ja `entry` vaativat aina avaimen, ellei niitä ole erikseen sallittu. |
| V-AUT-4 | S | Avaimia voi lisätä ja poistaa käynnistämättä ohjelmaa uudelleen (asetustiedoston uudelleenluku). |
| V-AUT-5 | C | TLS (HTTPS/WSS). Ei 1. versiossa, koska verkko on suljettu. Tarvittaessa TLS hoidetaan edustapalvelimella (9.6, V-PRX-5). Sovellukseen ei toteuteta TLS:ää. |

### 8.2 Rajapinnan oma konfiguraatio (K12)

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-KONF-1 | M | Rajapinnalla on oma konfiguraatiotiedosto `restapi.xml` kilpailun työkansiossa. Se on erillään sovelluksen parametreista, jotta tapahtumakohtaiset raja-arvot (esim. massatapahtumat) voidaan säätää koskematta ajanoton asetuksiin. Rajapinta käynnistetään sovelluksen parametrilla `RESTAPI` (tiedoston polku tai `RESTAPI=1` oletustiedostolle). |
| V-KONF-2 | M | Tiedostossa on vähintään seuraavat osat oletusarvoineen. Puuttuva arvo korvataan oletuksella, ja käynnistyksessä kirjataan lokiin käytetyt arvot. Oletukset ovat samat lajista riippumatta. |
| V-KONF-3 | M | Tiedosto luetaan uudelleen ilman ohjelman uudelleenkäynnistystä (`POST /api/v1/admin/reload`, rooli `operations`, tai käyttöliittymän valinta). Virheellinen tiedosto hylätään, jolloin edelliset arvot jäävät voimaan ja virhe kirjataan. |
| V-KONF-4 | S | `GET /api/v1/admin/config` (rooli `operations`) näyttää voimassa olevat arvot ilman avaintiivisteitä. |

V-KONF-2:n osat ja oletusarvot:

| Osa | Asetus | Oletus |
|---|---|---|
| `server` | `port`, `bind_address`, `max_connections`, `max_request_bytes`, `cors_origins`, `public_base_url`, `trusted_proxies` | 8080, `0.0.0.0`, 64, 16384, ei mitään, ei (päätellään pyynnöstä), ei mitään |
| `docs` | `public`, `public_details` | kyllä, ei |
| `auth` | `mode` (`on`/`off`), `anonymous_roles`, `keys[]` (client_id, kuvaus, SHA-256-tiiviste, roolit, IP-rajaus, voimassaolo) | `on`, `public, commentator` |
| `visibility` | `commentator_min_level`, `allow_birth_year` | 1, ei |
| `events` | `buffer_size` | 5000 |
| `websocket` | `max_clients`, `ping_interval_s`, `max_queue` | 32, 30, 1000 |
| `safety` | `max_time_min`, `leader_factor`, `start_grace_min`, `stale_s` | 240, 2,0, 10, 60 |
| `estimate` | `min_samples` | 3 |
| `entry` | `enabled`, `allow_force` | ei, ei |
| `rate_limit` | `requests_per_min_per_key` | 600 |
| `cache` | `max_bytes`, `rebuild_min_interval_ms`, `stale_max_ms`, `snapshot_interval_ms` | 64 MB, 250, 1000, 1000 |
| `documents` | `html.format_file`, `pdf.format_file`, `pdf.printer`, `temp_dir`, `queue_max`, `timeout_s`, `min_interval_s` (HTML/JSON, PDF), `iof_min_interval_s`, `stale_max_s`, `format_check_interval_s`, `prewarm_minutes`, `prewarm_max`, `cache_max_bytes`, `disk_cache` | sovelluksen asetukset, sovelluksen asetukset, *Microsoft Print to PDF*, `restapi_tmp`, 10, 30, 10 / 30, 30, 120, 30, 10, 50, 128 MB, pois |

### 8.3 Palvelun suojaus

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-SEC-1 | M | Rajapinta ei ole oletuksena päällä (V-KONF-1). Portti ja kuunneltava osoite määritellään konfiguraatiossa. |
| V-SEC-2 | M | Pyynnön koko, otsakkeiden määrä, yhteyksien määrä ja pyyntönopeus avainta kohden rajataan. Virheellinen HTTP-pyyntö katkaistaan kaatamatta ohjelmaa. Rajan ylitys havaitaan aina erikseen, eikä se johda katkaistun pyynnön käsittelyyn (vrt. SI-8): liian suuri runko → 413, liian suuret otsakkeet → 431. |
| V-SEC-3 | S | CORS-otsakkeet ovat asetettavissa (sallitut originit). Edustapalvelinprofiilissa selainsovellukset ovat samassa originissa kuin rajapinta, joten CORSia ei tarvita (V-PRX-10). Asetus on muita kuin edustapalvelimen jakamia sovelluksia varten. Oletuksena yhtään originia ei sallita. |
| V-SEC-4 | M | Epäonnistuneet tunnistautumiset kirjataan lokiin. |
| V-SEC-5 | M | `X-Forwarded-For`, `Forwarded`, `X-Forwarded-Host` ja `X-Forwarded-Proto` huomioidaan vain, kun yhteys tulee asetuksen `server.trusted_proxies` osoitteesta. Muuten otsakkeet ohitetaan. Avaimen IP-rajaus (V-AUT-1), nopeusraja (V-SEC-2) ja lokit käyttävät näin saatua asiakkaan osoitetta. |

### 8.4 Julkisuus ja henkilötiedot

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-JUL-1 | M | Väliaikapisteen julkisuus: `public` näkee vain tason 10, `commentator` palvelimen asetuksen mukaisen tason (oletus ≥ 1), `safety` kaikki. Asiakas voi pyytää vain tiukempaa rajaa (`min_level`). |
| V-JUL-2 | M | Sarjan `piilotatulos` estää tulokset ja sijat muilta kuin `safety`-roolilta. |
| V-JUL-3 | M | Syntymäaikaa, osoitetta, maksutietoja tai ilmoittautujan yhteystietoja ei palauteta. Syntymävuosi annetaan `commentator`-roolille vain, jos se on sallittu asetuksella. Kortin numero annetaan vain `safety`-roolille. |
| V-JUL-4 | M | Viestin `kuuluttaja`-tila (`Juk/VInit.cpp:701`) ja sen tarkkuusrajoitukset (`laika` vs. `laika2`) koskevat rajapintaa samoin kuin näyttöjä. |

## 9. Ei-toiminnalliset ja arkkitehtuurivaatimukset

### 9.1 Ei-toiminnalliset

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-SUO-1 | M | `tall_CriticalSection` pidetään vain lukumallin kenttien kopioinnin ajan (V-VM-2). Sijojen laskenta, JSONin muodostus ja lähetys tehdään lukon ulkopuolella. Pyyntöjen käsittely ei koskaan pidä lukkoa: lukkoa pitää vain lukumallin rakentaja. Tiedonsiirron vastaanotto ei saa hidastua (V-KUO-5). |
| V-SUO-2 | M | Kun kilpailijoita (HK) tai joukkueita (viesti) on 3000, väliaikapisteitä 10 ja 50 asiakasta kyselee kerran sekunnissa eri sarjoja tai osuuksia, sarjan tilanteen vastausajan 95. persentiili on alle 200 ms ja tilannekuvan yhteenvedon alle 300 ms. Vaatimus koskee myös tilannetta, jossa koko 3000 on yhdessä sarjassa. Todennetaan kuormitustestillä (9.4). |
| V-SUO-5 | M | Kuorma tallennuslukkoon riippuu muutosten määrästä, ei kyselijöiden määrästä. Muuttumattoman tiedon kysely ei kosketa tietuetaulukkoa eikä lukkoa (V-VM-6). |
| V-SUO-3 | M | HTTP- ja WebSocket-käsittely tehdään omissa säikeissään, ei VCL-pääsäikeessä. Lomakkeiden tietoja ei kosketa. |
| V-LUO-1 | M | Virheellinen pyyntö tai rajapinnan virhe ei kaada ohjelmaa. Virheet kirjataan lokiin. |
| V-LUO-2 | M | Jos rajapintaa ei saa käynnistettyä (portti varattu), ohjelma toimii muuten normaalisti ja näyttää virheen. |
| V-YHT-1 | M | JSON on UTF-8:aa. `wchar_t`- ja `char`-merkkijonot (cp1252) muunnetaan oikein. |
| V-YHT-2 | M | Ei uusia kolmannen osapuolen riippuvuuksia (C++Builder Community Edition ilman lisäosia). SHA-1 (WebSocket-kättely), SHA-256 (API-avainten tiivisteet) ja Base64 toteutetaan puhtaina omina yksikköinä (`Tp/`), jotka testataan standardin testivektoreilla (RFC 3174, FIPS 180-4, RFC 4648, RFC 6455 -kättelyesimerkki). `bcrypt.lib` ei ole käytettävissä g++-testeissä (V-TST-5). |
| V-YHT-3 | M | Jaetut kerrokset ovat C++03-yhteensopivia, koska ne käännetään myös VS2022-konsoliprojekteihin (V-ARK-7). |
| V-DOK-1 | M | OpenAPI 3.1 (`doc/api/openapi.yaml`) ja AsyncAPI 3.0 (`doc/api/asyncapi.yaml`) englanniksi sekä käyttöohjeen luku `docs-site`-sivustolle (suomeksi). Samat tiedostot palautetaan palvelusta (V-SPEC-1–2). Kuvausten `info.version` on sama kuin `api_version` (V-VER-2). |
| V-DOK-2 | M | Käyttöönotto-ohje: edustapalvelinprofiili (Caddy) selainsovelluksineen sekä pelkkä sisäänrakennettu palvelin ilman selainsovelluksia (`doc/api/deploy/`, V-PRX-8). |
| V-TST-1 | M | Tietojen poiminta, JSON-sarjallistus, reititys, roolitarkistus ja WebSocket-kehysten käsittely yksikkötestataan `Tests/`-kehyksellä ilman GUI:ta. Molemmille domaineille tehdään testiaineisto. |
| V-TST-2 | S | OpenAPI-skeemaa vasten tehdään sopimustesti (vastausten validointi). |
| V-TST-3 | M | Lukumallin sijat ja erot ovat samat kuin `sijat2`/`merk_sijat`-laskennan tulos samalla aineistolla, myös tasatulosten ja hylättyjen osalta. Testataan molemmille domaineille (V-VM-3). |
| V-TST-4 | M | Tulosluettelot: (a) muodostus rajapinnan kautta ei muuta kilpailutiedostoa eikä lähetä sanomia (tiedosto- ja sanomavertailu ennen ja jälkeen); (b) HTML:n sisältö vastaa sovelluksen HTML-tulostusta samoilla valinnoilla; (c) IOF XML validoituu IOF 3.0 -skeemaa vasten molemmissa domaineissa; (d) JSON-luettelon sarakkeet, rivit ja arvot ovat samat kuin HTML-luettelon solut samoilla parametreilla (vertailu HTML-taulukon soluista); (e) PDF:n kaappaus ja toisto tuottavat saman sivumäärän ja tekstin kuin suora GDI-tulostus. |
| V-TST-5 | M | **Testattavuus SportIdent-haaran käytännön mukaan** (`Tests/README.md` haarassa). Testattava logiikka kirjoitetaan puhtaisiin käännösyksiköihin (`Tp/` tai vastaava), jotka eivät lue globaaleja eivätkä sisällä `windows.h`:ta tai VCL:ää. Sovitinosa (globaalit, Winsock, VCL) pidetään ohuena. Puhtaita yksiköitä ovat ainakin HTTP-pyynnön jäsennin, WebSocket-kehykset, JSON-kirjoitin ja -lukija, UTF-8-muunnos, tiivisteet, sijojen ja erojen laskenta taulukoista (V-VM-3), mediaaniarvio (V-ARV-1), vaihtoaluesäännöt (V-TUR-10–16), myöhässä-säännöt (V-TUR-3/4), kortin numeron tulkinta (V-MAL-5) ja välimuistin avain- ja versiologiikka. Testit ajetaan CI:ssä sekä g++:lla (`run.sh`, Linux, 32-bittinen `wchar_t`, etumerkillinen `char`) että MSVC:llä (`VS/Tests/TpTest.vcxproj`). UTF-8-muunnos käsittelee sekä 16-bittisen (korvausparit) että 32-bittisen `wchar_t`:n, ja `char`-arvojen käsittely ei riipu etumerkistä. |
| V-TST-6 | M | **Kuvaus ja toteutus pysyvät samoina.** Yksikkötesti vertaa reitityskerroksen reittitaulukkoa (polku, metodi, roolit) OpenAPI-kuvauksen `paths`-osaan molempiin suuntiin, ja WebSocketin tapahtumatyyppien luetteloa AsyncAPI-kuvaukseen. Testi myös tarkistaa, että kuvausten `info.version` on sama kuin koodin `api_version`-vakio ja että sulautettu resurssi on sama kuin lähdetiedosto. Ero kaataa CI:n. |

### 9.2 Arkkitehtuuri

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-ARK-1 | M | Tiedot poimitaan VCL:stä riippumattomilla moduuleilla (`Hk/HkSeuranta.cpp`, `Juk/VSeuranta.cpp`), jotka palauttavat rakenteisia rivejä. Rajapinta käyttää niitä. `TulosUnit::PaivitaGrid` (HK ja Viesti) siirretään vaiheittain käyttämään samoja moduuleja, jotta ruutu ja rajapinta eivät erkane. |
| V-ARK-2 | M | Tapahtumapuskuri siirretään GUI-yksiköistä jaettuun kerrokseen ilman `UnicodeString`-tyyppiä. Siihen lisätään juokseva `seq`, rastikoodi ja tapahtumatyyppi. Nykyiset *Viimeiset tapahtumat* -näytöt lukevat samaa puskuria. |
| V-ARK-3 | M | HTTP/1.1- ja WebSocket-palvelin toteutetaan Winsockilla `com/`-kerrokseen. Perustelut: ohjelmassa on jo Winsock-pohjainen tiedonsiirto (`com/WinUDP.cpp`), sama toteutus kattaa HTTP:n ja WebSocketin, ja kerros pysyy VCL-riippumattomana ja yksikkötestattavana. C++Builderin Indy-komponentit (`TIdHTTPServer`) olisivat konsolirajauksen (K21) jälkeen mahdollinen vaihtoehto HTTP:lle, mutta WebSocket-tuki pitäisi silti tehdä itse. Palvelin palvelee vain dynaamista sisältöä ja omat kuvauksensa (5.12). Staattisen sisällön jakelu, selainsovellukset ja TLS jätetään edustapalvelimelle (9.6, K24). |
| V-ARK-4 | M | JSON-kirjoitin (escape, UTF-8) ja suppea yleinen JSON-lukija (syöttöpäätepisteet, WebSocket-tilausviestit) tehdään puhtaina yksikköinä `Tp/`-hakemistoon SportIdent-haaran käytännön mukaan (V-TST-5). Haaran `Tp/SICenterJson.cpp` on SI Centerin vastauksiin erikoistunut jäsennin. Sitä ei muuteta tässä työssä, mutta sen voi myöhemmin siirtää käyttämään yleistä lukijaa. |
| V-ARK-5 | M | Kerrokset: `com/` (HTTP, WS), `tputilv2` (JSON, tiivisteet), yhteinen reititys- ja roolikerros (domain-riippumaton), domain-kohtaiset poimintamoduulit (Hk, Juk). Rajapinta käynnistetään vain HkKisaWinissä ja ViestiWinissä. |
| V-ARK-6 | M | Syöttötoiminnot (luku 7) käyttävät olemassa olevia tallennusfunktioita, eivät omaa kirjoituslogiikkaa. Ne suoritetaan samassa sarjallistetussa kontekstissa kuin käyttöliittymän muutokset ja aina järjestyksessä *`tall_CriticalSection` → `GETREC` → muutos → `tallenna` → vapautus*. Tietuetta ei lueta lukon ulkopuolella, koska silloin samanaikainen tallennus voi kadota (SI-1). |
| V-ARK-7 | M | **Konsoliversioiden yhteensopivuus.** HkMaali ja JukMaali eivät käynnistä rajapintaa, eikä niihin lisätä sen toimintoja. Jaettuun koodiin (`Hk/`, `Juk/`, `tputilv2/`, `com/`) tehtävät muutokset eivät saa rikkoa konsoliversioiden käännöstä (CI:n `build.yml`) eivätkä muuttaa niiden toimintaa. Rajapinnan omat lähdetiedostot (HTTP/WS-palvelin, reititys, lukumallit, välimuistit, dokumenttityösäie) lisätään vain RAD Studio -projekteihin (`HkKisaWin.cbproj`, `ViestiWin.cbproj`). Jaettuun tallennuspolkuun lisättävät kytkennät (muutoslaskurit V-VM-1, tapahtumapuskuri V-ARK-2) toimivat konsoliversioissa ilman vaikutusta, kuten nykyinen tyhjä `addtapahtuma` (`HkConsole/HkCons0.cpp:79`, `JukConsole/laskent0.cpp:38`). Tulostusputken lukutila (V-TLS-2) ja JSON-kohde (V-TLS-51) eivät muuta konsoliversion nykyistä tulostusta. |
| V-ARK-8 | M | **Riippuvuus SportIdent-haaraan.** REST-toteutus aloitetaan vasta, kun `feature/sportident-reader` on yhdistetty mainiin korjauksin SI-1, SI-5 ja SI-6 (`doc/sportident-review-items.md`). Vaihtoehtoisesti työ aloitetaan haaran päältä. Näin vältetään ristiriidat samoissa tiedostoissa (SI-11), ja REST-työ voi käyttää haaran testi-infrastruktuuria ja `BadgeTeksti`-muotoilua. |
| V-ARK-9 | M | **Rakenteiden asettelu (SI-9).** Kaikki rajapinnan rakenteet, jotka jaetaan käännösyksiköiden kesken (lukumallit, tapahtumapuskurin tietue, välimuistin avaimet), lukitaan omalla `#pragma pack(push, n)`/`pop`-parilla, ja niille lisätään käännösaikaiset tarkistukset (`offsetof`/`sizeof`, kuten haaran `SIResultTp`). Ne eivät saa periytyä `tputil.h`:n `-a1`-asetuksesta tai `HkDef.h`:n `pack(push, 1)`-alueelta. `Interlocked*`-funktioilla käsiteltävät muutoslaskurit (V-VM-1) ja viitelaskurit (V-VM-6) ovat erillisiä, 4 tavuun tasattuja `volatile LONG` -muuttujia, eivät pakattujen rakenteiden kenttiä. |

### 9.3 Välimuisti ja lukumalli

Taustaa: nykyinen Tilanne-näytön polku kutsuu jokaisella päivityksellä `sijat2()`-funktiota
(`HkTulostus.cpp:1165`; viestissä `merk_sijat`). Se kopioi jokaisessa pisteessä jokaisen
kilpailijan koko tietueen kahdesti (`GETREC`), kirjoittaa muuttuneet sijat levylle
(`PUTREC(d, 1)`) ja tekee kaiken `tall_CriticalSection`in sisällä. Jos yhden 3000 kilpailijan
sarjan tilannetta kyselee 50 asiakasta sekunnissa, tämä tarkoittaa miljoonia tietuekopioita
sekunnissa lukon sisällä. Järjestetyt listat (`jalku`, `JarjSeur`) ovat sen sijaan valmiina, koska
ne päivitetään tallennuksen yhteydessä (`addjarjarr`).

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-VM-1 | M | **Muutoslaskurit.** Tallennuspolkuun (`HkDat.cpp` `tlsmuutos`/`addtapahtuma`-kohta, viestissä `vdat.cpp`) lisätään laskuri, joka kasvaa jokaisessa muutoksessa: yksi koko kilpailulle ja yksi sarjaa kohden (viestissä sarjaa ja osuutta kohden). Myös sarjatietojen ja väliaikapisteiden muutos kasvattaa laskuria. Laskurit päivitetään atomisesti (`InterlockedIncrement`), eikä niiden lukeminen vaadi lukkoa. |
| V-VM-2 | M | **Sarjan lukumalli.** Kullekin sarjalle (viestissä sarja + osuus) rakennetaan kevyt, muuttumaton taulukko, jossa ovat vain rajapinnan tarvitsemat kentät pisteittäin järjestettynä. Kentät luetaan järjestettyjen listojen mukaan suoraan muistissa olevasta tietuetaulukosta (`kilparr`) koko tietuetta kopioimatta. Viestissä luetaan vain tarvittavan osuuden tiedot. Lukko on varattuna vain kopioinnin ajan. |
| V-VM-3 | M | **Sijat ilman kirjoitusta.** Lukumallin sijat ja erot lasketaan lukon ulkopuolella järjestyksestä (tasatuloksilla sama sija). Rajapinta ei kutsu `sijat2`- eikä `merk_sijat`-funktiota, eikä se kirjoita tietueita tai tiedostoja lukupyyntöjen yhteydessä. |
| V-VM-4 | M | **Rakennus tarvittaessa ja yhdistettynä.** Lukumalli rakennetaan, kun sitä pyydetään ja sarjan versio on muuttunut. Sama sarja rakennetaan enintään kerran `cache.rebuild_min_interval_ms` aikana (oletus 250 ms). Kun usea pyyntö tarvitsee saman sarjan yhtä aikaa, vain yksi rakentaa. Muut saavat edellisen version, jos se on enintään `cache.stale_max_ms` vanha (oletus 1000 ms), tai odottavat rakennuksen valmistumista. Vastauksen `data_version` kertoo, mistä versiosta se on tehty. |
| V-VM-5 | M | **Koko kilpailun lukumalli.** Taustasäie rakentaa kaikista kilpailijoista (viestissä kaikista osuuksien juoksijoista) kevyen taulukon, kun koko kilpailun versio on muuttunut, kuitenkin enintään kerran `cache.snapshot_interval_ms` aikana (oletus 1000 ms). Tilannekuva (V-TUR-*), vaihtoalue, `/controls/{code}/*` kaikista sarjoista, kilpailijahaku ja status lukevat tätä taulukkoa koskematta tietuetaulukkoon. |
| V-VM-6 | M | **Versioiden vaihto.** Valmis lukumalli vaihdetaan kerralla uuteen. Lukijat käyttävät mallia viitelaskurin avulla (C++03, `InterlockedIncrement`/`InterlockedDecrement`), eikä käytössä olevaa mallia koskaan muuteta. Vanha malli vapautetaan, kun viimeinen lukija on valmis. |
| V-VM-7 | M | **Vastausvälimuisti.** Valmis JSON tallennetaan välimuistiin. Avain on polku, normalisoidut parametrit (järjestetty, oletusarvot täytetty) ja rooli. `ETag` on lukumallin versio ja parametrien tiiviste. `If-None-Match`-osuma palauttaa 304 rakentamatta mitään. |
| V-VM-8 | M | **Kellosta riippuvat kentät.** Lukumalliin tallennetaan vain aikaleimat. Ajan kuluessa muuttuvat kentät (`result_age_s`, juoksuaika, `expected`, `overdue`, `uncertain`, seuraavat lähtijät) lasketaan JSONia muodostettaessa. Jos vastauksessa on tällaisia kenttiä, ETag-arvoon ja välimuistin avaimeen lisätään sekunti, jolloin vastaus muodostetaan avainta kohden enintään kerran sekunnissa. |
| V-VM-9 | M | **Muistin rajaus.** Vastausvälimuistin koko on enintään `cache.max_bytes` (oletus 64 MB), ja vähiten käytetyt vastaukset poistetaan ensin. Lukumalleja on enintään yksi voimassa oleva versio sarjaa tai osuutta kohden, sekä mahdolliset lukijoiden vielä käyttämät vanhat versiot. |
| V-VM-10 | M | **Välimuistin tila** näkyy kohdassa `GET /api/v1/status/api` (V-STA-5): osumaprosentti, rakennusten määrä ja kesto (keskiarvo, 95. persentiili, suurin), lukon pitoaika rakennuksessa (95. persentiili, suurin), muistinkäyttö ja poistojen määrä. |
| V-VM-11 | S | Tilanne-ikkuna (`TulosUnit`, HK ja viesti) siirretään lukemaan samaa lukumallia (V-ARK-1). Silloin näyttö ei enää kutsu `sijat2`-funktiota jokaisella päivityksellä. Siirto tehdään erillisenä muutoksena vaiheen 2 jälkeen. |
| V-VM-12 | M | Tapahtumapuskuri ja WebSocket eivät käytä välimuistia. Tapahtumat julkaistaan suoraan puskurista (V-ARK-2, V-WS-5). |

### 9.4 Kuormitustesti

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-KUO-1 | M | **Testiaineisto.** Generaattori luo kilpailun, jossa on 3000 kilpailijaa (HK) tai 3000 joukkuetta kolmella osuudella (viesti) ja 10 väliaikapistettä. Kustakin domainista tehdään kaksi muunnelmaa: (a) 100 sarjaa, (b) yksi 3000 kilpailijan tai joukkueen sarja. |
| V-KUO-2 | M | **Tulosvirta.** Testi syöttää rajapintakoneelle tuloksia ja väliaikoja nykyisen tiedonsiirron kautta (sama UDP/TCP-yhteys kuin pääkoneelta). Tasainen kuorma on 20 tallennusta sekunnissa, ja massamaalipurske 100 tallennusta sekunnissa 60 sekunnin ajan. Mukana ovat myös korjaukset ja tilamuutokset. |
| V-KUO-3 | M | **Asiakkaat.** 50 pollaavaa asiakasta, jotka kukin kyselevät kerran sekunnissa eri sarjaa tai osuutta (`standings`, osa parametrilla `splits=all`) ja käyttävät `If-None-Match`-otsaketta. Lisäksi 5 asiakasta kyselee `safety/summary`- ja `safety/changeover`-päätepisteitä, 5 asiakasta `controls/{code}/passings?since=`-päätepistettä ja 10 WebSocket-asiakasta tilaa rastitapahtumia. Pahin tapaus ajetaan myös siten, että kaikki 50 kyselevät samaa 3000 kilpailijan sarjaa. |
| V-KUO-3a | M | **Dokumentit.** Edellisten lisäksi 5 asiakasta hakee 10 sekunnin välein satunnaisen sarjan tai osuuden HTML-, JSON- tai PDF-luettelon, ja 1 asiakas hakee koko kilpailun IOF XML:n minuutin välein. |
| V-KUO-4 | M | **Mittaukset.** Vastausajat päätepisteittäin (mediaani, 95. ja 99. persentiili, suurin), 304-osuuksien osuus, lukumallin rakennusten määrä ja kesto, `tall_CriticalSection`in pitoaika rakennuksessa, tallennusviive (sanoman vastaanotosta tallennukseen) ilman rajapintaa ja rajapinnan kanssa, WebSocket-viive (tallennuksesta viestiin), suoritin ja muisti. Mittaukset luetaan V-VM-10-tiedoista ja kuormitustyökalusta. |
| V-KUO-5 | M | **Hyväksymisrajat.** `standings`-vastausten 95. persentiili alle 200 ms ja `safety/summary`-vastausten alle 300 ms (V-SUO-2). Lukon pitoaika yhdessä rakennuksessa enintään 20 ms 3000 kilpailijan sarjalla. Tallennusviive kasvaa rajapinnan kanssa enintään 50 ms (95. persentiili). WebSocket-viive alle 500 ms (V-WS-5). Muisti pysyy `cache.max_bytes`-rajan ja lukumallien koon summan alla. Virheitä tai katkenneita yhteyksiä ei ole. Rajat ovat tavoitteita, ja ne tarkistetaan ensimmäisen mittauksen jälkeen. |
| V-KUO-5a | M | **Dokumenttien hyväksymisrajat.** Välimuistista palautettu dokumentti alle 100 ms (95. persentiili). Uusi HTML alle 1 s ja uusi PDF alle 5 s yhdelle sarjalle. Koko kilpailun IOF XML (3000 kilpailijaa tai joukkuetta) alle 10 s. Lukon pitoaika yhden sarjan kaappauksessa enintään 50 ms 3000 kilpailijan sarjalla, myös PDF:ssä (V-TVM-8). Dokumenttien muodostus ei saa heikentää V-KUO-5:n `standings`-vastausaikoja eikä tallennusviivettä yli rajojen. |
| V-KUO-6 | M | **Toteutus.** Kuormitustyökalu ja aineistogeneraattori sijoitetaan hakemistoon `Tests/load/` (Python-standardikirjasto tai oma C++-työkalu, ei uusia riippuvuuksia ohjelmaan). Testi ajetaan Windowsilla HkKisaWin- ja ViestiWin-ohjelmilla. Se voidaan käynnistää käsin tai GitHub Actionsin erillisellä työnkululla (`workflow_dispatch`) itse ylläpidetyllä Windows-ajokoneella. Tulokset tallennetaan raporttina (`Tests/load/results/`) versiotunnuksella. |
| V-KUO-7 | M | Kuormitustesti ajetaan vaiheen 2 lopussa (sarjan tilanne ja välimuisti) ja vaiheen 3 lopussa (WebSocket ja tilannekuva), sekä ennen jokaista julkaisua, joka muuttaa poimintamoduuleja, välimuistia tai tallennuspolkua. |
| V-KUO-8 | S | Vertailumittaus nykyisellä toteutustavalla, jossa jokainen pyyntö laskee tilanteen kuten Tilanne-ikkuna (`sijat2`), dokumentoidaan kerran perustelemaan välimuistin tarve. |

### 9.5 Tulosteiden välimuisti

Taustaa: tulostusputki pitää `tall_CriticalSection`ia koko sarjan tulostuksen ajan
(`HkTulostus.cpp:5850–6518`). GDI-tulostuksessa tämä tarkoittaa, että verkosta saapuvien tulosten
tallennus odottaa koko kirjoitinajurin työn ajan. Tämä voi kestää suuren sarjan PDF:ssä sekunteja.
Dokumentin muodostus on siis paljon kalliimpaa kuin JSON-tilanne (9.3). Välimuistin tavoite on, että
saman sarjan ja saman version dokumentti muodostetaan vain kerran, ja että lukkoa pidetään vain
datan läpikäynnin ajan, ei ajuri- tai tiedosto-operaatioiden aikana.

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-TVM-1 | M | **Avain.** Välimuistin avain on muoto (`html`, `json`, `pdf`, `iof`), sarjat ja osuudet, normalisoidut parametrit (V-TLS-10, oletusarvot täytettyinä, järjestettyinä), rooli (V-TLS-6) ja kieli. |
| V-TVM-1a | M | Välimuistin täyttäminen ei muuta dataa eikä kasvata versioita, koska rajapinnan muodostus ei koskaan tallenna sijoja (V-TLS-2). Muuten muodostus voisi mitätöidä oman tuloksensa. |
| V-TVM-2 | M | **Riippuvuudet ja versio.** Dokumentin versio koostuu (a) sisältyvien sarjojen versioista (V-VM-1; viestin osuusdokumentissa sarja + osuus, joukkuedokumentissa sarjan kaikki osuudet; sarjayhdistelmässä jäsensarjat), (b) **muotoiluversiosta**, joka kasvaa, kun `tulosmuottp`, kenttämääritykset, muotoilutiedosto (`muottied`) tai lisätekstitiedostot muuttuvat, sekä (c) sarja- ja kilpailutietojen versiosta (sarjanimet, matkat, väliaikapisteet, kilpailun nimi). ETag on näiden ja avaimen tiiviste, ei dokumentin sisällön tiiviste, koska muodostusaika muuttaa sisältöä. |
| V-TVM-3 | M | **Muotoiluversion seuranta.** Muotoiluversio kasvaa, kun asetukset tallennetaan sovelluksessa (kutsu tallennuskohdasta). Lisäksi muotoilu- ja lisätekstitiedostojen muutosaika tarkistetaan enintään kerran `documents.format_check_interval_s` aikana (oletus 30 s), jolloin myös käsin muokatut tiedostot havaitaan. |
| V-TVM-4 | M | **Vanhan version palautus muodostuksen ajaksi.** Jos välimuistissa on vanhempi versio, se palautetaan heti otsakkein `X-Result-Version`, `Age` ja `X-Stale: true`, ja uusi muodostus laitetaan jonoon. Pyytäjä odottaa vain, jos dokumenttia ei ole välimuistissa lainkaan, enintään `documents.timeout_s`. Vanhaa versiota palautetaan enintään `documents.stale_max_s` ajan (oletus 120 s). Sen jälkeen pyytäjä odottaa uutta versiota. |
| V-TVM-5 | M | **Tihein muodostusväli.** Sama avain muodostetaan enintään kerran `documents.min_interval_s` aikana (oletus HTML/JSON 10 s, PDF 30 s, IOF XML `documents.iof_min_interval_s` 30 s). Väliin tulevat muutokset yhdistetään seuraavaan muodostukseen. |
| V-TVM-6 | M | **Päällekkäiset pyynnöt.** Samaa avainta on jonossa enintään yksi työ. Saman avaimen samanaikaiset pyynnöt odottavat samaa tulosta. |
| V-TVM-7 | M | **Prioriteetit.** Työjonossa HTML ja JSON menevät PDF:n edelle, ja PDF IOF XML:n edelle. Esilämmitys (V-TVM-9) on aina viimeisenä. Pyytäjän odottama työ menee saman prioriteetin taustatyön edelle. |
| V-TVM-8 | M | **Lukon pito: kaappaus ja toisto.** Tulostusputkesta tehdään kaksivaiheinen. (1) *Kaappaus* `tall_CriticalSection`in sisällä: putki ajetaan tavallisesti, mutta kohde kirjoittaa muistiin. HTML ja JSON tallentuvat merkkijonoiksi. PDF:n GDI-kutsut (`startdocGDI`, `startpageGDI`, `selectfontGDI`, `putwfldGDI`, `endpageGDI`, `enddocGDI`) tallentuvat komentolistaksi. Tekstin mittaukseen (`textlengthGDI`) käytetään saman PDF-kirjoittimen tietokontekstia (`CreateIC`), jolloin ajuria ei käynnistetä. (2) *Toisto* lukon ulkopuolella: komentolista ajetaan PDF-kirjoittimelle. Lukon pitoaika on siten sama kuin HTML-muodostuksessa, eikä ajurin hitaus näy tallennukselle. |
| V-TVM-9 | S | **Esilämmitys.** Jos dokumenttia on pyydetty viimeisen `documents.prewarm_minutes` (oletus 10 min) aikana ja sen versio muuttuu, se muodostetaan taustalla uudelleen pienimmällä prioriteetilla. Seuraava pyyntö saa silloin heti tuoreen version. Esilämmitettävien määrä rajataan (`documents.prewarm_max`, oletus 50). |
| V-TVM-10 | S | **Yhteinen kaappaus HTML:lle ja JSONille.** Jos sama avain (paitsi muoto) on jonossa molemmille, putki ajetaan kerran ja kohde tuottaa molemmat (V-TLS-51). |
| V-TVM-11 | M | **Muisti.** Dokumenttivälimuistin koko on enintään `documents.cache_max_bytes` (oletus 128 MB), erillään JSON-välimuistista (V-VM-9). Vähiten käytetyt poistetaan ensin. PDF:t ja IOF XML voidaan säilyttää välimuistin sijaan levyllä `documents.temp_dir`-hakemistossa (`documents.disk_cache`, oletus pois). Levyvälimuisti tyhjennetään käynnistyksessä. |
| V-TVM-12 | M | **HTTP.** Vastauksissa on `ETag`, `Last-Modified` (muodostusaika) ja `Cache-Control: no-cache`. Asiakas voi siis säilyttää tiedoston, mutta tarkistaa sen joka kerta (`If-None-Match` → 304). Pakkausta (gzip) ei tueta ensimmäisessä versiossa, koska se vaatisi uuden riippuvuuden (V-YHT-2). |
| V-TVM-13 | M | **Johdonmukaisuus.** Dokumentin versio luetaan ennen kaappausta. Jos versio on muuttunut kaappauksen aikana (esim. lukko vapautui välissä), dokumentille annetaan ennen kaappausta luettu versio ja uusi muodostus laitetaan jonoon. Dokumentti ei siis koskaan väitä olevansa uudempi kuin on. |
| V-TVM-14 | M | **Seuranta.** `GET /api/v1/status/api` (V-STA-5) näyttää muodoittain osumaprosentin, vanhan version palautusten määrän, muodostusten määrän ja keston (kaappaus ja toisto erikseen), lukon pitoajan kaappauksessa, jonon pituuden ja muistinkäytön. |

### 9.6 Käyttöönottomalli: sisäänrakennettu palvelin ja edustapalvelin (K24)

#### 9.6.1 Kysymys

Kannattaako asennukseen lisätä valmis HTTP-palvelin (edustapalvelin, *reverse proxy*), joka jakaa
staattiset resurssit ja välittää dynaamisen sisällön tässä määriteltävälle REST-laajennukselle?

Vaihtoehdot:

| | A. Vain sisäänrakennettu palvelin | B. Edustapalvelin pakollinen | C. Sisäänrakennettu palvelin, edustapalvelin selainsovelluksia varten (valittu, K24) |
|---|---|---|---|
| Kuvaus | HkKisaWin/ViestiWin kuuntelee itse porttia ja palvelee kaiken (V-ARK-3) | Edustapalvelin kuuntelee porttia, jakaa staattiset tiedostot ja välittää polut `/api/*` sovellukselle osoitteeseen `127.0.0.1` | Sovellus toimii yksin kuten A. Asennusohjeessa on profiili, jossa edustapalvelin lisätään eteen ilman muutoksia sovellukseen |
| Asennus kilpailupaikalla | Yksi ohjelma, ei lisäasennuksia | Toinen ohjelma, oma asennus, oma päivitys ja oma asetustiedosto | Edustapalvelin asennetaan, kun käytetään selainsovelluksia, TLS:ää tai suuren yleisön tulossivuja. Muuten riittää A |
| Vikaantumiskohdat | Yksi | Kaksi. Jos edustapalvelin pysähtyy, rajapinta on poissa, vaikka sovellus toimii | Kuten A, ellei profiilia oteta käyttöön |
| Staattinen sisältö | Vain sulautetut kuvaukset ja ohjesivu (V-SPEC). Ei yleistä tiedostojakelua | Tehokas tiedostojakelu: tulossivut (`autofile`-hakemisto), katselinsovellukset, Swagger UI, kuvat | Edustapalvelimella, kun se on käytössä |
| TLS (HTTPS/WSS) | Ei (V-AUT-5) | Kyllä. Valmiit varmenteet, myös automaattiset | Edustapalvelimella, kun se on käytössä |
| Suuri yleisömäärä | Rajattu yhteysmäärä (`max_connections` 64). Riittää suljetun verkon käyttäjille (V-SUO-2: 50 asiakasta) | Edustapalvelin hoitaa tuhansia yhteyksiä, pakkauksen ja välimuistin staattisille tiedostoille | Kuten B, kun käytössä |
| Vaikutus suorituskykyyn | Raskas työ on lukumallissa, välimuisteissa ja tulostusputkessa (9.3–9.5). Ne ovat sovelluksessa joka tapauksessa | Ei vähennä sovelluksen raskasta työtä. Dynaamiset vastaukset kulkevat sovelluksen kautta | Kuten B |
| Turvallisuus | Rajat ja todennus sovelluksessa (V-SEC) | Lisäsuoja: pyyntöjen suodatus ja rajat ennen sovellusta. Sovellus voi kuunnella vain `127.0.0.1`:tä | Kuten B, kun käytössä. Avainten todennus ja roolit pysyvät aina sovelluksessa |
| Kehitys ja testaus | Kaikki testattavissa yhdessä ohjelmassa | Lisää testattavaa: otsakkeet, aikakatkaisut, WebSocketin välitys, puskurointi | Molemmat profiilit testataan (V-PRX-9) |
| Lisenssit ja jakelu | Ei uusia | Kolmannen osapuolen ohjelma jakelussa tai asennusohjeessa | Ohjelmaa ei jaeta release-ZIP:ssä. Ohjeessa viitataan sen omaan jakeluun |

#### 9.6.2 Johtopäätös

Edustapalvelin ei poista tarvetta sovelluksen omalle HTTP- ja WebSocket-palvelimelle. Dynaaminen
sisältö syntyy sovelluksen muistissa olevista tiedoista, ja lukumalli, välimuistit ja
tulostusputki ovat sovelluksessa (9.3–9.5) riippumatta siitä, mikä ohjelma ottaa yhteydet vastaan.
Tällä työllä ei myöskään ole merkittävästi staattista sisältöä: kuvaukset ja ohjesivu ovat pieniä
ja kuuluvat käännökseen (V-SPEC-2). Pakollinen edustapalvelin (B) lisäisi kilpailupaikalle
asennettavan ja valvottavan osan, mutta ei poistaisi yhtään sovellukseen tehtävää osaa.

Edustapalvelimesta on hyötyä, kun (1) tuloksia jaetaan suurelle yleisölle (staattiset
tulossivut `autofile`-hakemistosta ja välimuistitetut JSON-vastaukset), (2) tarvitaan TLS, esim.
jos palvelu avataan suljetun verkon ulkopuolelle, tai (3) halutaan interaktiivinen
kuvausten katselin samasta osoitteesta, tai (4) loppukäyttäjille tarjotaan selainsovellukset
CIS ja Tilannekuva. Sovellukset jaetaan samasta originista kuin rajapinta, joten CORSia ei
tarvita, ja käyttäjä tarvitsee vain yhden osoitteen.

**Päätös (K24): vaihtoehto C, edustapalvelimena suositellaan Caddya.** Edustapalvelin kuuluu
käyttöönottoon aina, kun loppukäyttäjille tarjotaan CIS- tai Tilannekuva-sovellus, TLS tai suuren
yleisön tulossivut. Rajapinta toimii myös ilman sitä (V-PRX-1), esim. kun ulkoinen järjestelmä
käyttää rajapintaa suoraan suljetussa verkossa. Sovellus tehdään edustapalvelinvalmiiksi
(V-PRX-1–7), ja asennusohjeeseen tulee valmis Caddy-profiili (V-PRX-8). Sovelluksen oma palvelin
hoitaa vain dynaamisen sisällön ja omat kuvauksensa. Yleistä staattisen tiedoston jakelua
(hakemistolistaus, MIME-tyypit, `Range`-pyynnöt) ei toteuteta sovellukseen.

#### 9.6.3 Edustapalvelimen valinta

| Ohjelmisto | Windows-tuki | WebSocket-välitys | TLS | Asetukset | Arvio |
|---|---|---|---|---|---|
| **Caddy** (Apache 2.0) | Yksi suoritettava tiedosto, ajettavissa Windowsin palveluna | Automaattinen | Sisäänrakennettu, myös itse allekirjoitettu sisäverkossa | Lyhyt `Caddyfile` | **Valittu (K24)**: helpoin asentaa ja ohjeistaa kilpailupaikalla |
| nginx (BSD) | Windows-versio on virallisesti rajoitettu (select-pohjainen, ei suositella tuotantoon) | Vaatii `Upgrade`-otsakkeiden määrittelyn | Kyllä | `nginx.conf` | Toimii, mutta Windows-versio on heikompi |
| IIS + URL Rewrite + ARR | Windowsin mukana (Pro), lisäosat erikseen | WebSocket-ominaisuus asennettava | Kyllä | Hallintakonsoli, `web.config` | Raskas asentaa ja ohjeistaa satunnaiselle käyttäjälle |
| Apache httpd (Apache 2.0) | Kolmannen osapuolen Windows-käännökset | `mod_proxy_wstunnel` | Kyllä | `httpd.conf` | Toimii, mutta enemmän asetuksia kuin Caddyssa |

Muu edustapalvelin on mahdollinen, kun se täyttää vaatimukset V-PRX-4–7. Ohjeet ja testit tehdään
vain Caddylle.

#### 9.6.4 Vaatimukset

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-PRX-1 | M | Rajapinta toimii täysin ilman edustapalvelinta. Mikään rajapinnan toiminto ei edellytä edustapalvelinta. Selainsovellukset (9.6.5) edellyttävät sen. |
| V-PRX-2 | M | Sovellus voi kuunnella vain paikallista osoitetta (`server.bind_address=127.0.0.1`), jolloin vain samalla koneella oleva edustapalvelin pääsee siihen. |
| V-PRX-3 | M | Vastauksissa ja kuvauksissa ei ole kovakoodattuja absoluuttisia osoitteita. Linkit ovat suhteellisia tai muodostetaan asetuksesta `server.public_base_url` (V-SPEC-3). Polkujen etuliite on `/api`, joten edustapalvelin voi välittää sen sellaisenaan. |
| V-PRX-4 | M | Asiakkaan osoite luetaan välitysotsakkeista vain luotetulta edustapalvelimelta (V-SEC-5). Avainten todennus, roolit, nopeusrajat ja julkisuussäännöt pysyvät sovelluksessa. Edustapalvelimen rajat ovat lisäsuoja. |
| V-PRX-5 | M | TLS hoidetaan edustapalvelimella. Sovellus ei toteuta TLS:ää (V-AUT-5). Kun `X-Forwarded-Proto=https` tulee luotetulta edustapalvelimelta, kuvausten `servers` käyttää `https`/`wss`-osoitteita. |
| V-PRX-6 | M | Profiilissa edustapalvelin jakaa staattisesti (a) selainsovellukset CIS ja Tilannekuva (9.6.5), (b) sovelluksen automaattitulosteiden hakemiston (`autofile`, HTML/XML-tulossivut, prioriteetti S) ja (c) interaktiivisen kuvausten katselimen, joka lukee kuvauksen osoitteesta `/api/v1/openapi.json` (prioriteetti S). Edustapalvelin ei lue kilpailun datatiedostoja (KILP.DAT ym.). |
| V-PRX-7 | M | Välityksen yhteensopivuus: (a) WebSocketin `Upgrade` välitetään, ja edustapalvelimen tyhjäkäynnin aikakatkaisu on pidempi kuin `websocket.ping_interval_s` (V-WS-7). (b) Sovelluksen `ETag`/`304`, `Cache-Control`, `Retry-After` ja 503-vastaukset välitetään muuttamatta. Jos edustapalvelin pakkaa vastaukset, ETag muuttuu heikoksi (`W/`), ja sovellus hyväksyy heikon ETagin ehdollisessa pyynnössä. (c) PDF- ja muut dokumenttivastaukset välitetään puskuroimatta liian pienellä rajalla (kokoraja ≥ `documents.cache_max_bytes`:n suurin yksittäinen dokumentti). (d) Edustapalvelin käyttää terveystarkistukseen `/api/health`-polkua (V-VER-7). |
| V-PRX-8 | M | Toimitetaan valmis Caddy-asetustiedosto (`doc/api/deploy/Caddyfile`) ja käyttöönotto-ohje (V-DOK-2): Caddyn lataus, asennus ja käynnistys Windowsin palveluna, portit, `trusted_proxies`, `public_base_url`, sovellushakemistot, palomuuri ja vianetsintä. Ohjeessa mainitaan testattu Caddyn versio. Caddyn ohjelmatiedostoa ei sisällytetä release-ZIP-pakettiin, vaan se ladataan Caddyn omasta jakelusta. Asetustiedosto ja selainsovellukset ovat paketissa (V-PRX-14). |
| V-PRX-9 | M | Kuormitustesti (9.4) ja sopimustestit (V-TST-2) ajetaan molemmissa profiileissa: suoraan sovellukseen ja edustapalvelimen kautta. WebSocketin viive (V-WS-5) mitataan myös edustapalvelimen kautta. |

#### 9.6.5 Selainsovellukset: CIS ja Tilannekuva

Selainsovellukset ovat staattisia HTML-, JavaScript- ja CSS-tiedostoja. Ne käyttävät vain tätä
rajapintaa (REST ja WebSocket), eikä niillä ole omaa palvelinosaa. Sovellusten käyttöliittymä
määritellään omissa dokumenteissaan. Alla ovat jakelun ja rajapinnan käytön vaatimukset.

| Tunnus | Pri | Vaatimus |
|---|---|---|
| V-PRX-10 | M | **Sama origin.** Edustapalvelin jakaa sovellukset poluissa `/cis/` ja `/tilannekuva/` samasta osoitteesta kuin rajapinnan `/api/*`. Sovellukset käyttävät suhteellisia polkuja (`/api/v1/…`, `/api/v1/ws`), joten ne toimivat sekä `http`- että `https`-profiilissa ilman asetuksia. Juuripolku `/` ohjaa sovellusten valintasivulle. |
| V-PRX-11 | M | **Avaimet eivät ole sovelluksessa.** Sovellustiedostoissa ei ole API-avaimia. Käyttäjä syöttää avaimen sovelluksen aloitussivulla, tai sovellus toimii ilman avainta `auth.anonymous_roles`-rooleilla (V-AUT-3). Avain säilytetään selaimen istuntomuistissa (`sessionStorage`), ja se lähetetään otsakkeessa `Authorization` ja WebSocketissa parametrina `?key=` (V-WS-2). Tilannekuva vaatii `safety`-roolin avaimen (V-TUR-5). Edustapalvelin ei lisää avaimia pyyntöihin, koska silloin rooli ei enää olisi käyttäjäkohtainen. |
| V-PRX-12 | M | **Avain ei päädy lokeihin.** Rajapinta (HkKisaWin/ViestiWin) ja edustapalvelin peittävät `key`-parametrin ja `Authorization`-otsakkeen pääsylokeista. Caddy-asetustiedosto (V-PRX-8) sisältää tämän suodatuksen. |
| V-PRX-13 | M | **Versioiden yhteensopivuus.** Sovellus lukee käynnistyessään `GET /api/version` (V-VER-1). Jos `api_version`-pääversio ei ole tuettu tai tarvittava ominaisuus puuttuu `features`-listasta (esim. `safety`, `websocket`), sovellus näyttää selkeän ilmoituksen eikä yritä käyttää puuttuvaa toimintoa. WebSocket-yhteys pyytää aliprotokollaa `tulospalvelu.v1` (V-VER-5). |
| V-PRX-14 | M | **Lähde ja jakelu.** Sovellusten lähdekoodi on versionhallinnassa (`web/cis/`, `web/tilannekuva/`). Release-paketti sisältää valmiit sovellustiedostot ja Caddy-asetustiedoston hakemistossa `web/`. Sovellukset eivät lataa skriptejä tai fontteja ulkoisista verkko-osoitteista, koska kilpailuverkko voi olla suljettu. |
| V-PRX-15 | M | **Välimuistiohjeet.** Edustapalvelin palauttaa sovellusten `index.html`-tiedostot otsakkeella `Cache-Control: no-cache` ja versioidut resurssit (tiedostonimessä tiiviste) pitkällä välimuistiajalla. Uusi sovellusversio tulee näin käyttöön sivun uudelleenlatauksella. |
| V-PRX-16 | S | **Lisärajaus Tilannekuvalle.** Caddy-asetustiedostossa on valmiina valinnainen IP-rajaus polulle `/tilannekuva/` (esim. vain kilpailukeskuksen verkko). Varsinainen suojaus on silti `safety`-rooli rajapinnassa (V-PRX-4). |
| V-PRX-17 | M | **Yhteyskatkot.** Sovellukset näyttävät tiedon tuoreuden (`generated`, `stale`, V-API-10, V-TUR-9). Kun WebSocket-yhteys katkeaa, ne yhdistävät uudelleen viiveellä, joka kasvaa, ja jatkavat viimeisestä `seq`-arvosta (V-WS-4). Jos tapahtumat ovat jo poistuneet puskurista, ne hakevat tilanteen pollaamalla. |
| V-PRX-18 | M | **Testaus.** Kummallekin sovellukselle tehdään päästä päähän -testi, joka ajetaan Caddyn kautta testidatalla (V-TST-5) ja varmistaa, että sovellus käyttää vain kuvauksessa (OpenAPI, AsyncAPI) olevia polkuja ja viestityyppejä. |

## 10. Päätökset

| # | Päätös |
|---|---|
| K1 | Toteutetaan HkKisaWiniin ja ViestiWiniin. (Alun perin myös HkMaali; korvattu päätöksellä K21.) |
| K2 | Rajapinta ajetaan erillisellä koneella, joka saa tiedot verkosta (luku 3, V-API-10, V-TUR-9, V-SYO-6/7). |
| K3 | Väliaikapisteet tunnistetaan rastikoodilla. Toistuva koodi tunnistetaan muodossa `{code}.{n}`. Sarjan pisteet listataan numeroineen ja etäisyyksineen (5.3). |
| K4 | Pääkäyttötapa on pollaus. Väliaikapisteiden leimauksille tehdään WebSocket (5.9). |
| K5 | Kaikki valinnat annetaan pyynnön parametreina (V-API-3). |
| K6 | Lähtökohtana on suljettu verkko. Sovelluskohtaiset API-avaimet ja roolit (4.1, 8.1). |
| K7 | Kenttänimet sekä OpenAPI- ja AsyncAPI-dokumentaatio englanniksi. |
| K8 | Vaatimukset perustuvat CIS-järjestelmään (5.5–5.7, lisenssi- ja IOF-tunnisteet V-KLP-2/3) ja turvallisuuden tilannekuvaan (5.8). |
| — | Syötöt *ei lähtenyt* ja *uusi lähtöaika* ovat `entry`-roolin takana ja toteutetaan vaiheessa 5 (luku 7). |
| K9 | Useamman radan sarjassa etäisyys annetaan välinä (`distance_min_m`–`distance_max_m`) (V-VAP-2). |
| K10 | Saapumisaika arvioidaan sarjan mediaaniajalla tai mediaanivauhdilla (V-ARV-1). |
| K11 | Viestin IOF-tunnisteesta on erillinen muutosehdotus `doc/viesti-iof-tunniste-muutosehdotus.md`. Sitä ei toteuteta tämän työn yhteydessä (V-KLP-8). |
| K12 | Myöhässä- ja epäselvyysrajat ovat samat lajista riippumatta, ja ne määritellään rajapinnan omassa konfiguraatiossa (V-KONF-1–4, V-TUR-3/4). |
| K13 | Leimaukset vastaanotetaan olemassa olevilla sovelluksen tiedonsiirtomäärityksillä, ja ne julkaistaan rajapintaan vasta, kun ne on tallennettu tälle instanssille (V-WS-6). |
| K14 | Uusi lähtöaika rajapinnan kautta sallitaan vain väliaikalähtökilpailuissa (V-SYO-3a). |
| K15 | Konfiguraation `safety`-oletukset hyväksytty: 240 min, kärkiaika × 2,0, 10 min lähdön jälkeen, katkos 60 s (V-KONF-2). |
| K16 | `punch`-tapahtumina julkaistaan vain radio- ja online-rastien leimaukset. Emit-luennan leimat näkyvät väliaikoina ja kilpailijatiedoissa, mutta eivät tapahtumina (V-WS-6). |
| — | Viestin vaihtoalueella olevien arvio (V-TUR-10–16). |
| K17 | Vaihtoaluesääntö "lähdön jälkeen" koskee kaikkia osuuksia 2…n (V-TUR-12). |
| K18 | Yhteislähdössä lähteneet eivät ole vaihtoalueella (V-TUR-14). Vaatimuksen prioriteetti on nostettu tasolle M. |
| — | Tulosluettelot sarja- ja osuuskohtaisesti HTML-, JSON- ja PDF-muodossa tarkemmin rajattuina. IOF XML vain koko kilpailusta. PDF tehdään sovelluksen kirjoitintulostuksen muotoiluilla (5.11). JSON-luettelon tietosisältö vastaa HTML-tulostetta (5.11.6). |
| K19 | PDF muodostetaan toistaiseksi Windowsin PDF-kirjoitinputkella (V-TLS-30). Oma PDF-kirjoitin (V-TLS-35) jätetään myöhemmäksi. |
| K20 | (Kumottu päätöksellä K21: HkMaalia ei toteuteta.) |
| — | SportIdent-haaran havainnot viety vaatimuksiin (luku 2.4, V-WS-5/6/6c, V-SYO-3b/4, V-STA-2, V-TUR-4, V-MAL-5, V-SEC-2, V-YHT-2, V-TST-5, V-ARK-4/6/8/9). Aikayksikkö korjattu (luku 2.2). |
| K21 | Toteutus rajataan HkKisaWiniin ja ViestiWiniin. Konsoliohjelmille ei ole käytännön tarvetta, ja rajaus helpottaa testaamista. Jaettu koodi pidetään konsoliversioissa käännettävänä ja toiminnaltaan ennallaan (V-ARK-7). |
| — | Tulosteiden välimuisti: versiopohjainen avain, vanhan version palautus muodostuksen ajaksi, kaappaus lukon sisällä ja toisto sen ulkopuolella (9.5). |
| — | Suorituskyky varmistetaan lukumallilla ja vastausvälimuistilla (9.3), ja se todennetaan kuormitustestillä, jossa on 50 asiakasta ja 3000 kilpailijaa tai joukkuetta (9.4). |
| — | Palvelun versio (`/api/version`, semanttinen versiointi, `X-Api-Version`, WebSocketin `hello` ja aliprotokolla) ja palvelusta haettavat OpenAPI- ja AsyncAPI-kuvaukset, jotka sulautetaan käännökseen (5.12). Vastausten muutoslaskurin nimi on `data_version`. |
| K24 | Edustapalvelin otetaan käyttöönottomalliin (vaihtoehto C), ja edustapalvelimeksi suositellaan Caddya. Sen avulla loppukäyttäjille tarjotaan REST-rajapintaan perustuvat selainsovellukset CIS ja Tilannekuva samasta osoitteesta kuin rajapinta. Rajapinta toimii edelleen myös ilman edustapalvelinta (9.6, V-PRX-1–18). |

## 11. Vaiheistus

| Vaihe | Sisältö | Domain |
|---|---|---|
| 0. Esitehtävät | SportIdent-haaran merge korjauksin SI-1, SI-5, SI-6 (V-ARK-8). Testi-infrastruktuurin yhdistäminen (mainin ja haaran testit molempiin CI-töihin) | HK+V |
| 1. Perusta | HTTP-palvelin, JSON, avaimet ja roolit, `/version`, `/health`, `/competition`, `/classes`, `/classes/{c}/splits`, `/controls`, `/status`, OpenAPI-runko ja AsyncAPI-luonnos, kuvausten sulautus ja jakelu (V-SPEC-1–5, 7), kuvauksen ja reittitaulukon vertailutesti (V-TST-6), edustapalvelinvalmius (V-PRX-1–5, V-SEC-5) ja Caddy-profiilin runko (V-PRX-8) | HK+V |
| 2. CIS | Poimintamoduulit (V-ARK-1), muutoslaskurit ja lukumallit (V-VM-1–6), vastausvälimuisti ja ETag (V-VM-7–10), tapahtumapuskuri (V-ARK-2), `standings`, `passings`, `competitors` tunnisteineen. CIS-selainsovellus ja sen jakelu Caddyn kautta (V-PRX-6, 10–15, 18). Kuormitustesti (V-KUO-7) molemmissa profiileissa (V-PRX-9) | HK+V |
| 3. WebSocket ja tilannekuva | `/ws` aliprotokollineen ja `hello`-viesteineen (V-VER-5), AsyncAPI-kuvaus valmiiksi (V-SPEC-8), koko kilpailun lukumalli (V-VM-5), `safety/*`, `expected`, johdetut tapahtumat. Tilannekuva-selainsovellus (V-PRX-16–18), CIS-sovellukseen WebSocket. WebSocket edustapalvelimen kautta (V-PRX-7). Kuormitustesti (V-KUO-7) molemmissa profiileissa | HK+V |
| 4. Tulosluettelot ja viimeistely | HTML-, JSON-, PDF- ja IOF XML -tulosluettelot (5.11) ja niiden välimuisti (9.5), tulostusputken lukutila (V-TLS-2), CSV, historia, taustatietolinkit, sopimustestit, Tilanne-ikkunan siirto lukumalliin (V-VM-11), ohjesivu (V-SPEC-6), tulossivujen ja kuvausten katselimen jakelu edustapalvelimelta (V-PRX-6 b–c), käyttöönotto-ohje valmiiksi (V-DOK-2) | HK+V |
| 5. Syötöt | DNS, uusi lähtöaika, audit-loki, `entry`-rooli käyttöön | HK+V |

## 12. Esimerkkejä (luonnos)

```json
GET /api/version

{
  "service": "tulospalvelu-rest", "application": "HkKisaWin", "domain": "individual",
  "app_version": "5.21", "app_build": "5.22.002/2026-07-31", "git_commit": null,
  "api_version": "1.0.0", "api_versions_supported": ["v1"],
  "openapi_version": "1.0.0", "asyncapi_version": "1.0.0", "websocket_protocol": "tulospalvelu.v1",
  "features": {"websocket": true, "entry": false, "documents": {"html": true, "json": true, "pdf": true, "iof_xml": true},
               "safety": true, "sportident": true, "auth": "on"},
  "links": {"openapi": "/api/v1/openapi.json", "asyncapi": "/api/v1/asyncapi.json", "docs": "/api/docs", "health": "/api/health"}
}
```

```json
GET /api/v1/classes/H21/splits

{
  "generated": "2026-06-14T14:32:05+03:00", "data_version": 18234, "stage": 1,
  "source": {"mode": "network-client", "last_update": "2026-06-14T14:32:04+03:00", "last_update_age_s": 1},
  "class": "H21",
  "splits": [
    {"index": 1, "code": "101", "occurrence": 1, "name": "3.2 km", "distance_m": 3200, "distance_source": "course", "visibility": 10},
    {"index": 2, "code": "150", "occurrence": 1, "name": "Lähestyminen", "distance_m": 9100, "distance_source": "course", "visibility": 10},
    {"index": 3, "code": "finish", "occurrence": 1, "name": "Maali", "distance_m": 9500, "distance_source": "class", "visibility": 10}
  ]
}
```

```json
WebSocket ← {"op":"subscribe","controls":["150"],"classes":["H21","D21"],"types":["passing","new_leader"],"since":18200}
WebSocket → {"type":"passing","seq":18235,"time_of_day":"2026-06-14T14:32:06+03:00",
             "control":"150","class":"H21","index":2,
             "competitor":{"bib":112,"first_name":"Matti","last_name":"Meikäläinen","club":"Karhulan Katajaiset","country":"FIN",
                           "ids":{"iof":12345,"national":56789}},
             "elapsed_ms":2543000,"time":"42.23","rank":1,"behind_ms":0,"status":"OK"}
WebSocket → {"type":"new_leader","seq":18236,"control":"150","class":"H21","bib":112,"previous_leader_bib":87,"margin_ms":4000}
```

```json
GET /api/v1/safety/summary          (rooli: safety)

{
  "generated": "2026-06-14T14:32:05+03:00", "stale": false,
  "totals": {"registered": 1240, "start_passed": 1102, "dns": 31, "in_forest": 312, "finished": 748, "dnf": 9, "dsq": 2, "uncertain": 4, "overdue": 3},
  "by_class": [{"class": "H21", "in_forest": 22, "overdue": 1}],
  "by_last_seen": [{"code": "start", "count": 140}, {"code": "101", "count": 98}, {"code": "150", "count": 74}]
}
```

```json
GET /api/v1/safety/changeover?class=H21          (rooli: safety, viesti)

{
  "generated": "2026-06-14T14:32:05+03:00", "stale": false,
  "total": {"waiting_before_start": 0, "waiting_for_handover": 118, "finished_unchecked": 6, "total": 124},
  "by_class": [{"class": "H21", "phase": "after_start", "waiting_for_handover": 118, "finished_unchecked": 6, "total": 124}],
  "by_leg": [{"class": "H21", "leg": 2, "waiting_for_handover": 31, "finished_unchecked": 4},
             {"class": "H21", "leg": 3, "waiting_for_handover": 45, "finished_unchecked": 0},
             {"class": "H21", "leg": 1, "waiting_for_handover": 0,  "finished_unchecked": 2}]
}
```

## 13. Avoimet kysymykset

| # | Kysymys | Vaikutus |
|---|---|---|
| K22 | Tallentuvatko kortinluennan (Emit tai SportIdent) rastiajat koskaan väliaikapisteiden aikoihin (`va[]`)? Esimerkiksi jokin asetus voisi kopioida emit-väliajat tai maalilaji 4 voisi ottaa lopputuloksen kortilta. Jos tallentuvat, `punch`-tapahtumien määritelmä (V-WS-6) ei erota niitä radioleimoista, ja tarvitaan lisäehto tai sanomamuodon lähdetieto (SI-4). Tarkistetaan vaiheen 2 alussa. | V-WS-6, K16 |
| K23 | Mikä on lähtöleiman ja käsin tai rajapinnan kautta asetetun lähtöajan sääntö (SI-2)? Vaihtoehdot: (a) nykyinen, eli leima korvaa aina; (b) leima korvaa vain, jos toteutunutta lähtöaikaa ei ole tai se on `uusinaika`-rajan sisällä; (c) käsin tai rajapinnan kautta asetettu aika on suojattu. Päätös koskee SportIdent-haaraa ja V-SYO-3b:tä. | V-SYO-3b, SI-2 |
