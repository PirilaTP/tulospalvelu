# SportIdent-haaran review items

> Tila: 2026-11-19. Kohde: `feature/sportident-reader`, paikallinen viite `origin/feature/sportident-reader`
> (viimeisin commit 51a17a3, 2026-09-28). Etähaku (`git fetch`) ei onnistunut tunnistautumisen
> takia, joten haarassa voi olla uudempia committeja. Haara ja `main` ovat erkaantuneet:
> 61 commitia vain haarassa, 50 vain mainissa (yhteinen kanta 9fda728).
>
> Havainnot tehtiin vertaamalla haaraa REST-rajapinnan vaatimuksiin (`doc/rest-api-vaatimukset.md`).
> Rajapintaan vaikuttavat kohdat on viety vaatimuksiin; viittaukset sarakkeessa *REST*.

Vakavuus: **K** = korkea (virheellinen data tai rikkoutuva käännös/testi), **M** = keskitaso
(käyttäytyminen voi yllättää, vaatii päätöksen), **A** = matala (siisteys, dokumentointi),
**I** = tieto / hyvä käytäntö.

## Yhteenveto

| # | Vak. | Aihe | Toimenpide | REST |
|---|---|---|---|---|
| SI-1 | K | Tietue luetaan lukon ulkopuolella `siParsePunch`-funktioissa (HK ja viesti) | Korjaa haarassa ennen mergeä | V-ARK-6 |
| SI-2 | M | Lähtöleima korvaa lähtöajan aina | Päätös ja dokumentointi | V-SYO-3b, K23 |
| SI-3 | M | SI Centerin haku useammalta instanssilta tallentaa samat leimat moneen kertaan | Ohje ja varoitus | V-WS-6c |
| SI-4 | M | Ajan lähde ei välity verkossa | Tieto suunnittelulle | V-WS-6, K22 |
| SI-5 | K | `Tests/run.sh`: mainin versio on rikki, haaran versio pudottaa mainin testit | Yhdistä listat mergessä | V-TST-5 |
| SI-6 | M | CI:n testityöt eivät aja mainin testejä | Lisää mainin testit `TpTest.vcxproj`:iin | V-TST-5 |
| SI-7 | A | IDE-artefakteja ja `.local`-tiedostoja versionhallinnassa | Erillinen siivous | — |
| SI-8 | A | `httphaku`-funktion katkaisutieto jää käyttämättä emiTag-haussa | Tarkista kutsujat | V-SEC-2 |
| SI-9 | I | Rakenteiden asettelun lukitus (`pack` + käännösaikainen tarkistus) | Hyvä käytäntö, käytetään REST-työssä | V-ARK-9 |
| SI-10 | I | `BADGEASKEL` on nyt ajonaikainen ja riippuu `badgelaji`sta | Tieto | V-MAL-5 |
| SI-11 | I | Merge-ristiriidat REST-työn kanssa | Merge ennen REST-vaihetta 1 | V-ARK-8 |

## SI-1 (K): Tietue luetaan lukon ulkopuolella

**Missä:** `Hk/HkIV.cpp` `siParsePunch` ja `Juk/VIv.cpp` `siParsePunch` (haara).

**Havainto:** Funktio lukee kilpailijatietueen (`kilp.GETREC(d)` / `GETREC(&kilp, d)`) ja muuttaa sitä
(`set_tulos`, `tall_lajat_pv`, `setMaali`) lukon ulkopuolella. `tall_CriticalSection` varataan vasta
tallennusta (`tallenna`) varten. Jos tiedonsiirto tai käyttöliittymä tallentaa saman kilpailijan
välissä, kyseinen muutos häviää, kun `siParsePunch` tallentaa vanhan kopion päälle.
SI Centerin haku tehdään omassa säikeessään (`_beginthread(siVahaku, …)`), joten tilanne on
mahdollinen esimerkiksi massamaalissa.

**Huomio:** Sama malli on jo mainissa funktiossa `tall_etulos` (`Hk/HkIV.cpp`: `kilp.GETREC(d)` noin
rivillä 652, `EnterCriticalSection` + `tallenna` noin rivillä 717). Siitä haara on mallin
todennäköisesti ottanut. Muualla koodissa (esim. `Juk/VIv.cpp`, rivit 568–569, 1266–1267,
2388–2389 haarassa) käytetään järjestystä *lukko → GETREC → muutos → tallenna*.

**Suositus:** Siirrä `EnterCriticalSection` ennen `GETREC`-kutsua molemmissa `siParsePunch`-funktioissa
(HK ja viesti) ennen mergeä. Korjaa `tall_etulos` erillisenä muutoksena mainiin. Lisää testi tai
vähintään koodikommentti järjestyksestä.

## SI-2 (M): Lähtöleima korvaa lähtöajan aina

**Missä:** `siParsePunch` (HK: `kilp.tall_lajat_pv(tm, k_pv)` tyypeille `Start` ja `SI_PUNCH_START`;
viesti: `kilp.setMaali(osuus, -1, tm)`).

**Havainto:** Maali- ja väliajat suojataan funktiolla `siSaaTallentaa` (olemassa olevaa aikaa ei
korvata `uusinaika`-rajan ulkopuolella), mutta lähtöaikaa ei suojata. Koodikommentti perustelee
tämän: lähtöaikakentässä on myös arvottu aika, joten "jo asetettu" -ehto estäisi kaikki lähtöleimat
väliaikalähdössä. Seuraukset:
- Myöhässä saapuva tai toistuva lähtöleima (GPRS-viive) korvaa käsin korjatun lähtöajan.
- REST-rajapinnan tulevassa syöttötoiminnossa (*uusi lähtöaika*, V-SYO-3) asetettu aika korvautuu,
  jos kilpailija leimaa lähdön myöhemmin, ja päinvastoin.

**Suositus:** Päätä sääntö. Esimerkiksi: lähtöleima korvaa lähtöajan vain, jos toteutunutta
lähtöaikaa ei vielä ole tai jos uusi leima on `uusinaika`-rajan sisällä. Kirjaa valittu sääntö
ohjeisiin (liite 12) ja lokiin.

## SI-3 (M): SI Centerin haku usealta instanssilta

**Havainto:** Jos `SIGPRS`/`SIHOST` on asetettu useammalle verkon koneelle (esim. pääkoneelle ja
kuuluttajan koneelle), jokainen hakee samat leimat ja tallentaa ne (`tallenna`), ja tallennukset
lähetetään verkkoon. `siSaaTallentaa` estää ajan korvaamisen, mutta tallennuksia ja sanomia syntyy
moninkertaisesti. Lähtöaikaa ei suojata (SI-2), joten lähtöaika voi vaihdella koneiden välillä.

**Suositus:** Kirjaa ohjeeseen, että SI Centerin hakua ajaa vain yksi instanssi. Harkitse
käynnistysvaroitusta, jos sama `SIGPRS` havaitaan toiselta koneelta (vaatisi sanoman, ei kiireellinen).

## SI-4 (M): Ajan lähde ei välity verkossa

**Havainto:** SRR- ja D3-leimat tallennetaan funktiolla `tall_etulos` (`Tp/TpLaitteet.cpp`
`tallSRRleima`) ja SI Centerin leimat funktioilla `set_tulos` + `tallenna`. Muille koneille ne
välittyvät tavallisina kilpailijatietueen päivityksinä, joissa ei ole tietoa ajan lähteestä
(radio, online-rasti, maalikamera, käsin). `EMITPKG_SPORTIDENT` erottaa lähteen vain
kortinlukutietueissa (`emittp.package`). Kilpailijalle kohdistamattomat leimat jäävät vastaanottavan
koneen paikalliseen aikajonoon (`tall_rivi`).

**Vaikutus:** Lähteen mukaan suodattaminen (esim. REST-päätös K16: vain radio- ja online-rastit) ei
onnistu vastaanottavalla koneella. Kyse ei ole virheestä vaan suunnittelurajoitteesta.

**Suositus:** Ei muutosta haaraan nyt. Jos lähdetieto tarvitaan myöhemmin, se on tiedonsiirron
sanomamuodon muutos (vrt. `doc/viesti-iof-tunniste-muutosehdotus.md`).

## SI-5 (K): `Tests/run.sh`

**Havainto:**
- **Main:** rivin `Juk/VRinnakkaisOsuus.cpp` lopusta puuttuu `\`. Seuraavat rivit eivät kuulu
  käännöskomentoon, joten `HkEnnTavTest.cpp`, `HkEnnTav.cpp`, `aikatowst_s.cpp` ja
  `putint_r.cpp` eivät käänny. Lisäksi `set -e` -tilassa skripti yrittää ajaa
  `Tests/HkEnnTavTest.cpp`:tä komentona ja päättyy virheeseen. Tiedostolla ei myöskään ole
  suoritusoikeutta (100644).
- **Haara:** `run.sh` on kirjoitettu uudelleen SI-testeille (suoritusoikeus 100755), ja siitä
  puuttuvat mainin testit (`VRinnakkaisOsuusTest`, `HkEnnTavTest`) sekä `-I Juk`.

**Suositus:** Yhdistä mergessä molempien listat ja hakupolut (`-I include -I Tp -I Juk -I Tests`).
Tarkista, että `HkEnnTav.cpp` kääntyy g++:lla ilman globaaleja (haaran README:n sääntö). Korjaa
mainin puuttuva `\` ja suoritusoikeus, jos merge viivästyy.

## SI-6 (M): CI:n testityöt eivät aja mainin testejä

**Havainto:** Haara lisää `build.yml`:iin työt `unit-tests-gcc` (Ubuntu, `run.sh`) ja
`unit-tests-msvc` (`VS/Tests/TpTest.sln`). `TpTest.vcxproj` sisältää vain SI-testit.

**Suositus:** Lisää mainin testit (`VRinnakkaisOsuusTest.cpp`, `HkEnnTavTest.cpp` ja niiden
lähdetiedostot) `TpTest.vcxproj`:iin mergessä, jotta molemmat työt ajavat samat testit.

## SI-7 (A): IDE-artefaktit versionhallinnassa

**Havainto:** Haara lisää tiedoston `TPsource/V52/VS/V/.vs/JukMaali520/v18/TestStore/0/testlog.manifest`
(binääri). Mainissa on jo 15 tiedostoa `.vs/`-hakemistoista (mm. `Browse.VC.db`,
`CopilotIndices`) sekä `*.cbproj.local`-tiedostoja, joita haara muuttaa
(`HkKisaWin.cbproj.local`: uudet `Tp/SI*.cpp`-polut).

**Suositus:** Erillinen siivous-PR: `.gitignore`en `.vs/` ja `*.local`, ja tiedostot pois indeksistä
(`git rm --cached`). Ei estä mergeä.

## SI-8 (A): `httphaku`-funktion katkaisutieto

**Havainto:** `com/gethttp.cpp`: `getwebPage`/`httphaku` saivat parametrin `katkaistu`, joka
kertoo, ettei vastaus mahtunut puskuriin. Korjaus estää aiemman datan sekoittumisen. Oletusarvo
on `NULL` (`HkDeclare.h:163`, `VDeclare.h:260`). emiTag-haku (`HkIV.cpp:3580`, `VIv.cpp:2477`)
ei välitä parametria, joten katkaisu jää siellä huomaamatta (data ei enää sekoitu, mutta vastaus
voi olla vajaa).

**Suositus:** Välitä `katkaistu` myös emiTag-haussa ja kirjaa katkaisu lokiin.

## SI-9 (I): Rakenteiden asettelun lukitus

**Havainto:** `tputil.h` asettaa klassiselle Borland-kääntäjälle 1 tavun pakkauksen
(`#pragma option -K -a1`) kaikelle sen jälkeen käännettävälle. Lisäksi `HkDef.h`:ssa on laaja
`#pragma pack(push, 1)` -alue. Haara korjasi tästä johtuneen bugin (ad1e6ae: `SIResultTp`-rakenteen
`ct[]` oli eri kohdassa eri käännösyksiköissä) lukitsemalla asettelun (`pack(push, 8)`/`pack(push, 4)`)
ja lisäämällä käännösaikaiset tarkistukset (`offsetof`/`sizeof`).

**Hyvä käytäntö:** Otetaan käyttöön REST-työssä kaikille rakenteille, jotka jaetaan käännösyksiköiden
kesken (V-ARK-9). Samoin `-K` (etumerkitön `char`) vs. g++:n etumerkillinen `char` (ea7fc66).

## SI-10 (I): `BADGEASKEL` on ajonaikainen

**Havainto:** `BADGEASKEL` on nyt `BadgeAskel()` = `siBadgeAskel(kilpparam.badgelaji)`: 1 000 000
Emit-kilpailussa ja 10 000 000 SportIdent-kilpailussa. Staattisia käyttöjä (taulukkokoot,
`case`-arvot, globaalit alustukset) ei löytynyt (26 käyttökohtaa). Tallennetun kortin numeron
tulkinta riippuu siis kilpailun tunnistinlajista. Näyttö käyttää `BadgeTeksti`/`siNaytettavaBadge`-funktioita.

**Vaikutus:** REST-rajapinnan `card`-kentät on muodostettava samalla tulkinnalla (V-MAL-5).

## SI-11 (I): Merge-ristiriidat REST-työn kanssa

Haara ja REST-työ muuttavat samoja tiedostoja: `Tests/run.sh`, `.github/workflows/build.yml`,
`RADStudio10/HkKisaWin.cbproj`, `ViestiWin.cbproj`, `cbHk/UnitStatus.cpp`, `ViestiWin/UnitStatus.cpp`,
`Hk/HkTulostus.cpp` ja `Juk/VTulostus.cpp` (`strval`, `F_BADGE`), `Hk/HkDef.h`, `Juk/VDef.h`.

**Suositus:** Merge haara mainiin (korjauksin SI-1, SI-5, SI-6) ennen REST-vaiheen 1 aloittamista
(V-ARK-8).
