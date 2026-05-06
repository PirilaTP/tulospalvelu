# JBang IT tests

Java-pohjaiset integraatiotestit aiempien Python-testien rinnalla.
Erot/edut:

- KILP.DAT-luku ja -kirjoitus tehdään suoraan **`fi.pirila:pirila-comm-common`**-jarrin kautta (`KilpReader`, `TulospalveluProtocol`). Offsetit ja layout-detektointi pysyy synkassa tuotantokoodin kanssa automaattisesti.
- Webadminin UI-ohjaus on **Playwright Java** — sama API kuin webadminin omissa Browserless-testeissä.
- HkMaali käynnistetään **pty4j**:llä (Linux/macOS PTY); Pythonin `pty.fork`:n vastine.

## Vaatimukset

- `jbang` (>= 0.130 testattu 0.138)
- HkMaali käännetty: `cd TPsource/V52 && make`
- Webadmin-jar paketoitu: `cd webadmin && mvn package -DskipTests`
- pirila-comm asennettu paikallisrepoon: `cd pirila-comm && mvn install -DskipTests`
- Playwrightin chromium: jbang lataa driverin automaattisesti, mutta järjestelmäkirjastot (libnss3, libnspr4, libasound2, libcups2 jne.) tarvitaan. JBang varoittaa puuttuvista mutta selain ajetaan silti.

## Ajaminen

```bash
cd IT/jbang
jbang run TwoDayBothOpen.java
jbang run TwoDayAfterQualifier.java
jbang run TwoDayAllDone.java
```

Kukin testi:
- Setup-vaihe (data-hakemistot, KILP.DAT-mutaatiot) ~1 s
- Käynnistää MA (HkMaali) → main menu ~5 s
- Käynnistää webadmin (Spring Boot) ~3 s
- Playwright kortin vaihto ~5 s
- Synkronointi + verifikaatio ~6 s

Kokonaisaika ~20–25 s per testi.

Lokit epäonnistuessa: `/tmp/jb-2day-*-MA.log` ja `/tmp/webadmin-<port>.log`.

## Rakenne

| Tiedosto | Rooli |
|----------|-------|
| `Harness.java` | Jaetut luokat: `HkMaali` (pty4j), `Webadmin` (Process+Playwright), KILP.DAT-apurit. |
| `TwoDayBothOpen.java` | Molemmat osat avoinna → vaihto kohdistuu pv[0]+pv[1]. |
| `TwoDayAfterQualifier.java` | pv[0] valmis → vain pv[1] muuttuu. |
| `TwoDayAllDone.java` | Kaikki valmiina → viimeinen osa muuttuu (käyttäjän syöte ei putoa). |

Testit jakavat `Harness.java`:n `//SOURCES`-direktiivillä, joten muutos
yhteen kohtaan päivittyy kaikkiin testeihin yhdellä kertaa.
