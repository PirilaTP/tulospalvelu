# FX9600 / ZEBRA -konfiguraatioparametrit

Tämä on kooste kaikista konfiguraatioparametreista, jotka vaikuttavat Zebra
**FX9600** -lukijaan (`ZEBRA=`). Kaikki arvot on luettu suoraan koodista
(rivinumerot viitteinä); oletukset on vahvistettu globaaleista ja
jäsennyskohdista. Parametrit luetaan `HkInit.cpp`:ssä (`#if defined(LAJUNEN)`).

> **Tärkein sääntö muualla:** älä riko toimivaa FX9500/SIRIT-polkua. Tämä
> dokumentti kuvaa vain ZEBRA-puolta; SIRIT-puoli on erillinen.

## 1. ZEBRA:n omat parametrit

| Parametri | Mitä tekee | Oletus | Yksikkö / muoto |
|---|---|---|---|
| `ZEBRA=` / `ZEBRA<n>=` | Ottaa FX9600:n käyttöön ja asettaa osoitteen (`regnly[rno] = LID_ZEBRA`). Numero valitsee lukijaindeksin: `ZEBRA=`/`ZEBRA1=` → lukija 1, `ZEBRA2=` → lukija 2. | — | `ZEBRA=TCP:<ip>[:portti]`; ilman porttia käytetään **5084**. |
| `ZEBRADEPART=` | Aika, jonka tagi saa olla näkymättä ennen kuin se tulkitaan kentästä poistuneeksi (takareuna/depart). | **700** | millisekunti (ms). Annettu arvo < 1 → pakotetaan 700. |
| `ZEBRADEPARTCLEANUP=` | Aika, jonka jälkeen depart-merkitty slotti vapautetaan uudelleenkäyttöön (säätää, kuinka pian sama tagi voi tuottaa uuden arrive/depart-tapahtuman). | **0** | millisekunti (ms). **0 = automaatti = 10 × ZebraDepart.** Annettu < 0 → 0. |

Viitteet: `HkInit.cpp:1772-1781` (luenta), `ZebraReader.cpp:41,456,466,893-894`,
`openConnection`-portin oletus `ZebraReader.cpp` (`destport==0 → LLRP_PORT`).

## 2. SIRITin kanssa jaetut parametrit, jotka koskevat MYÖS ZEBRA:a

| Parametri | Mitä tekee | Oletus | Muoto |
|---|---|---|---|
| `SIRITREUNA=` | Mitä reunaa luetaan. ZEBRA: etureuna = `siritreuna & 2`, takareuna = `siritreuna & 1`. | **T (takareuna)** | Yksi kirjain: `E` = etureuna/arrive (arvo 2), `T` = takareuna/depart (1), `M` = molemmat (3). |
| `SIRITMASK=` | EPC-maski, jolla vieraat tagit suodatetaan jo ennen emittiä. ZEBRA käyttää tarkalleen samaa `SiritMask`-arvoa kuin SIRIT. | **tyhjä (ei suodatusta)** | `<tyyppi><hex>`, ks. alla. |

Viitteet: `HkGlobals.cpp:510` (`siritreuna = 1`), `HkInit.cpp:1718-1730,1768-1770`,
`TpLaitteet.cpp:114` (`char SiritMask[31];` → nollattu = tyhjä),
`ZebraReader.cpp:643-666` (maskilogiikka), `:760` (etureuna), `:620` (takareuna).

### SIRITMASK — maski VAATII tyyppikirjaimen alkuun

Maskin **ensimmäinen merkki on tyyppikirjain** (X/Z/V/P), loput heksaa. Ilman
kelvollista tyyppikirjainta (tai jos arvo on tyhjä / alle 1 merkki) **maski ei
suodata mitään**. Esim. oikein: `SIRITMASK=V1234`. Vertailu kohdistuu EPC:n
ensimmäisiin `strlen(maski)-1` heksamerkkiin.

| Tyyppi | Vaikutus (tagi **pudotetaan** jos…) |
|---|---|
| `X` | EPC **==** maski (tarkka osuma poistetaan) |
| `Z` | `(maski & EPC) != 0` |
| `V` | EPC **≠** maski → eli **vain maskia vastaavat läpäisevät** ("vain nämä") |
| `P` | `(maski & EPC) == 0` |

## 3. Kiinteät arvot (#define, EI konfiguroitavissa)

| Nimi | Arvo | Selitys |
|---|---|---|
| `GPI_PORT` | **1** | GPI-portti, johon kytkin on johdotettu. Kovakoodattu. **`ZEBRAGPI=`-parametria ei ole olemassa** (kommentin mukaan mahdollinen myöhemmin). |
| `LLRP_PORT` | **5084** | LLRP:n oletusportti, jos `ZEBRA=`-rivillä ei anneta porttia. |

Viitteet: `ZebraReader.cpp:41,45`.

> Poistettu (2026-07-11): `ZEBRA_CLOCKWARN_US`-rivi. Vakiota ei ole enää koodissa;
> `measureClockOffset()` kirjaa kellopoikkeaman lokiin ehdoitta (`loki`-lipun
> ollessa päällä), ilman varoituskynnystä.

## 4. Kello — NTP, ei SIRITSYNC

FX9600:n **kelloa ei aseteta ohjelmasta**. Synkronointi tapahtuu **NTP:llä
lukijan omassa konfiguraatiossa**, ohjelman ulkopuolella. Yhteyttä avattaessa
ohjelma näyttää lukijan ja PC:n kellon **poikkeaman** (READER_EVENT_NOTIFICATION
→ UTCTimestamp), mutta ei korjaa lukijan kelloa. Vain kellonaika (hh:mm:ss)
ratkaisee; päivämäärä hylätään. Ks. CLAUDE.md "Tunnetut asiat".

## 5. SIRIT-etuliitteiset parametrit jotka EIVÄT koske ZEBRA:a

Nimi voi harhauttaa — nämä ovat SIRIT-kohtaisia, vaikka prefiksi on `SIRIT`:

| Parametri | Miksi ei koske ZEBRA:a |
|---|---|
| `SIRITSYNC` | Asettaa vain SIRITin kellon (`SiritSync(true)`). ZEBRA:n `sync()` on no-op; **ZEBRA:n kello hoituu NTP:llä**, ei SIRITSYNC:llä. |
| `SIRITPOLL=` | SIRITin pollausväli. ZEBRA käyttää tapahtumapohjaista LLRP-luentaa, ei pollausta. |
| `SIRITEVENTPORT=`, `SIRITCMDPORT=` | SIRITin kaksikanavainen data/komento. ZEBRA käyttää yhtä LLRP-yhteyttä (5084), ei komentokanavaa. |
| `SIRITARRIVE=`, `SIRITDEPART=` | SIRITin `register_event`-merkkijonot. ZEBRA rakentaa oman ROSpecin (`buildAddRospec`). |
| `SIRITLOKI` | ZEBRA-lokitus käyttää yleistä `loki`-lippua, ei `siritloki`:a. |

Viitteet: `TpLaitteet.cpp` (`SiritSync`, lukijasilmukan ZEBRA-haara 1835-1852),
`ZebraReader.cpp` (`sync()` no-op).
