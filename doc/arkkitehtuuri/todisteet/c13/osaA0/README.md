# C-13 osa A0: synteesin koneellinen pohja / machine base of the synthesis

Tuotettu skriptillä `../../../tyokalut/synteesi.py` rekisteristä ja seurantadokumenteista.
Tarkoitus: yksi yhteinen pohja, josta kaikki C-13:n osat lukevat tunnukset, jottei jokainen lukija
jäsennä asiakirjoja erikseen. Skripti tarkistettiin ennen agenttikäyttöä.

Produced by `tyokalut/synteesi.py` from the register and the tracking documents; validated before
any agent used it.

| Tiedosto / File | Sisältö / Content |
|---|---|
| `havainnot.csv` | yksi rivi kutakin H:ta kohden: erä, tila, normalisoitu vakavuus ja ehdollisuus, varmuus, tarkastuksen tila, koe, prioriteetti ennen C-13:a lähteineen, aiemmat KK-jäsenyydet, viittaukset / one row per H |
| `tekninen_velka.csv` | yksi rivi kutakin T:tä kohden / one row per T |
| `kysymykset.csv` | yksi rivi kutakin Q:ta kohden: tila ja sen lähde / one row per Q |
| `viittaukset.csv` | tunnusten väliset viittaukset rivien teksteistä / cross references |
| `kk_aiemmat.csv` | KK-C9-, KK-C10- ja KK-C12-kokonaisuuksien jäsenet lähteineen; osittainen jäsenyys merkitty / earlier packages |
| `makroartefaktit.csv` | makrot, joiden määritys ei ole numeerinen literaali / macros not defined by a numeric literal |
| `yhteenveto.txt` | määrät ja skriptin omat tarkistukset / counts and self-checks |
| `tarkistus_a0.py`, `tarkistus_a0.out` | riippumaton vertailu käsin kirjattuihin odotusarvoihin / independent comparison with hand-typed expectations |

## Tarkistus ennen käyttöä / Validation before use

`tarkistus_a0.py` vertaa pohjaa odotusarvoihin, jotka kirjattiin käsin `CHECKPOINT.md`:stä ja
`seuranta.md`:stä commitissa `3f378e2`, ei skriptin tulosteesta. Tulos: `differences: 0`.

Tarkistuksessa korjattiin kolme skriptivirhettä [todennettu]:

1. Vakavuuden normalisointi luki rivin "keskitaso; korkea epävakaalla verkolla" (H-016) korkeaksi;
   perustaso on nyt ensimmäinen mainittu taso ja ehto merkitään erikseen. Korkeita on 34, kuten
   erien sulkemisissa.
2. H-061 ("riippumaton lähdetarkastus; vaikutus ei todennettavissa staattisesti") luokittui
   staattisesti ratkeamattomaksi; luokka annetaan nyt vain, kun todennussarake alkaa niin (H-020).
3. Osittaisen tarkastuksen tunnistus ei löytänyt muotoa "(C-8, Opus), ... osittain" (H-130).

Yksi ero jäi ja osoittautui lähteen virheeksi: `CHECKPOINT.md` listasi H-017:n tarkastamattomaksi,
vaikka rekisterin rivi sanoo "riippumaton lähdetarkastus (C-7)". Rekisteri on oikeassa; odotusarvo
korjattiin ja `CHECKPOINT.md` korjataan osassa I.

Skriptin tulkintaa vaativat kohdat [päätelty]:

- **Erä** johdetaan tunnuksen numerovälistä. H-021:n lähdesarake viittaa `tiedonsiirto.md`:hen,
  vaikka väli on C-1; T-001–T-010 ovat kierroksen 1 dokumenteista.
- **Kysymysten tila** alueriveillä (Q19–Q25, Q26–Q34, Q35–Q42, Q43–Q56) luetaan erien raporttien
  kysymystaulukoista; tila "avoin" tarkoittaa, ettei riviä ole merkitty ratkaistuksi. Lopullinen
  luokittelu on `../raportit/osaA-kattavuus.md`:n luvussa 7.
- **Prioriteetti** on tilanne ennen C-13:a; ajantasainen on `../../../rekisteri/riskiarvio.csv`.

## Makroarvojen artefakti / The macro evaluator artefact

C-10:n ja C-12:n skriptit arvioivat makrot leksikaalisesti. Makro, jonka määritys ei ole
numeerinen literaali (lauseke tai toisen nimen alias), on niiden tulosteissa arvolla 1. Esimerkki:
viestin `MAX_PORTTI` on `todisteet/c10/osaA/makrot.csv`:ssä 1, vaikka `Juk/VDef.h:62` määrittelee
sen lausekkeena, jonka arvo on 132.

`makroartefaktit.csv` luettelee tällaiset määritykset: 238 määritystä, 176 eri nimeä, joista 22
on ehtona testattuja (13 testikirjaston `DOCTEST_*`-makroa), kuten C-12:n sulkemisessa todettiin.
`synteesi.py` ei laske makroarvoista mitään, eikä yksikään C-13:n johtopäätös perustu tällaiseen
tallennettuun arvoon: koot, joihin on vedottu (H-171, H-041, H-025), luettiin lähteestä.

The C-10 and C-12 scripts evaluate macros lexically, so a macro not defined by a numeric literal is
recorded as 1. The list has 238 definitions and 176 names, 22 of them tested. No C-13 conclusion
uses such a recorded value.
