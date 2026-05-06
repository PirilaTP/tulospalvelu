# JBang IT tests

Java/JBang-pohjaiset integraatiotestit Python-testien rinnalla
(`IT/test_*.py`). Erot/edut:

- KILP.DAT-luku ja -kirjoitus tehdään suoraan **`fi.pirila:pirila-comm-common`**-jarrin
  kautta (`KilpReader`, `TulospalveluProtocol`). Offsetit ja layout-detektointi
  pysyvät synkassa tuotantokoodin kanssa automaattisesti.
- Webadminin UI-ohjaus on **Playwright Java** — sama API kuin webadminin omissa
  Browserless-testeissä. Ei toista runtimea pidettäväksi.
- HkMaali käynnistetään **pty4j**:llä (Linux/macOS PTY); Pythonin `pty.fork`:n vastine.

## Vaatimukset

- `jbang` (>= 0.130, testattu 0.138)
- HkMaali käännetty: `cd TPsource/V52 && make`
- Webadmin-jar paketoitu: `cd webadmin && mvn package -DskipTests`
- pirila-comm asennettu paikallisrepoon: `cd pirila-comm && mvn install -DskipTests`
- Playwrightin chromium: jbang lataa driverin automaattisesti, mutta
  järjestelmäkirjastot (libnss3, libnspr4, libasound2, libcups2 jne.)
  tarvitaan. JBang varoittaa puuttuvista mutta selain ajetaan silti.

## Testit

| Tiedosto | Mitä testaa |
|----------|-------------|
| `EmitChange.java` | Yksi HkMaali, kortin vaihto UI:sta → uudelleenkäynnistys → tarkistus että muutos säilyi. |
| `UdpSync.java` | 2× HkMaali UDP-yhteydellä, kummasta tahansa tehty muutos näkyy toisessa. |
| `ThreeNodeSync.java` | 2× HkMaali + webadmin (tähti, MA = hubi). 6 synkronointi­suuntaa. |
| `FourNodeLahemitO.java` | 4× HkMaali, BE on `lähemit=O`-leimantarkastuskone. Varmistus että lähemit=O ei estä KILPPVT:n vastaanottoa. |
| `FourNodeWithWebadmin.java` | 3× HkMaali (MA, WI, BE-lähemit=O) + webadmin. **12 synkronointi­suuntaa**. |
| `WebadminForward.java` | webadmin → MA → {WI, M2-lähemit=O}. Tuotantoasettelua jäljittelevä forward-bugin regressio. |
| `TwoDayBothOpen.java` | Kaksipäiväinen kisa, kortinvaihto webadminista kun molemmat osat avoinna → muutos kahteen osaan. |
| `TwoDayAfterQualifier.java` | Karsinta jo tuloksellinen → vain finaaliin. |
| `TwoDayAllDone.java` | Kaikki osat valmiina → vaihto viimeiseen osaan. |
| `FourNodeNikondata.java` | ⚠ KNOWN FLAKY. Sama topologia kuin FourNodeWithWebadmin, mutta `kisat/nikondataa` (763 kilpailijaa). Phase A:n MA-UI-vaihto ei propagoidu luotettavasti tämän datasetin kanssa — ks. tiedoston otsikkokommentti. Vastaava kattavuus saadaan FourNodeWithWebadmin:lla. |

## Ajaminen

```bash
cd IT/jbang
jbang run TwoDayBothOpen.java
```

Kukin testi yksittäin (~20–60 s). Ensimmäinen ajo lataa JDK 21:n ja
deps:t (~1 min); seuraavat käyttävät välimuistia.

Kaikki kerralla:
```bash
for t in EmitChange UdpSync ThreeNodeSync FourNodeLahemitO FourNodeWithWebadmin WebadminForward TwoDayBothOpen TwoDayAfterQualifier TwoDayAllDone; do
    jbang run $t.java || echo "FAIL: $t"
done
```

Lokit epäonnistuessa: `/tmp/jb-*.log` ja `/tmp/webadmin-<port>.log`.

## Yhteinen koodi

`Harness.java` jaetaan testien välillä `//SOURCES Harness.java`-direktiivillä.
Tärkeimmät:

- `HkMaali` — pty4j-käärin, daemon-pumpurisäie kerää PTY:n stdout:n
  synkronoituun StringBuilderiin. `acceptAndWait(timeoutSec)` ohittaa
  käynnistyspromptit (J/L/H/Enter/Vahvista). `navigateToKorjaa`,
  `changeEmit`, `escapeToMain`, `readCompetitorEmit` mirroroivat Python-harnessin.
- `Webadmin` — Spring Boot -prosessi + Playwright-helperit (`changeEmit`, `checkEmit`).
- `Connection` — record (`localPort`, `peerHost`, `peerPort`, `lahemitSuffix`).
- `setupDataDir(...)` — `Connection...`-vararg-versio. `setupDataDirRaw(...)` täysin custom cfg:lle.
- **`preRaceSourceData()`** — laazyna luotu derivaatti `/tmp/HkKisaWinDataPreRace` jossa
  jokaisen kilpailijan kaikki osat on nollattu (keskhyl/finishTime/ysija = 0).
  Kortinvaihto-testeissä realistinen lähtötilanne (juoksija huomaa väärän kortin
  ennen lähtöä). Default-data säilyy ennallaan tulosvalmistuneille testeille
  (esim. `TwoDayAfterQualifier` joka odottaa pv[0]='T').
- KILP.DAT-apurit (`findRecordByKilpno`, `readPvBadge`, `readPvStatus`,
  `clearPvStatus`, `setPvResult`) käyttävät pirila-comm-common -jarrin
  `KilpReader`ia ja `TulospalveluProtocol`a — offsetit ja layout-detektointi
  pysyvät synkassa tuotantokoodin kanssa automaattisesti.
