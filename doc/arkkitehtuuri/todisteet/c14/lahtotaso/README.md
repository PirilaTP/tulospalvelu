# C-14: katselmointikattavuuden lähtötaso / Baseline of review coverage

Tuotettu 2026-11-27 revisiosta `f02a50b` (tuotantolähdekoodi tavu tavulta ennallaan revisiosta
`f7fe553`; välillä on muuttunut vain `Tests/` ja `VS/Tests/`). Skripti
`../../../tyokalut/kattavuus.py` (vain luku). Tulkinta: `../../../c14-suunnitelma.md` luku 2.

Produced on 2026-11-27 from revision `f02a50b` (production sources byte-identical to `f7fe553`) with
`tyokalut/kattavuus.py` (read-only). Interpretation: `c14-suunnitelma.md` section 2.

Toisto / reproduce (repon juuresta / from the repository root):
`python -I doc/arkkitehtuuri/tyokalut/kattavuus.py . doc/arkkitehtuuri/todisteet/c14/lahtotaso`

| Tiedosto / file | Sisältö / content |
|---|---|
| `funktiot.csv` | 7 252 funktiota: sijainti, tiedostoluokka, tuotteet, joissa funktio on voimassa (C-12:n haaratiedot), viitteet, viitealueen rivit (±10), näkökulmat (erä tai aihe), luokka `ei_viitteita` / `vain_johdettu` / `yksi_nakokulma` / `useita` / functions with references and perspectives |
| `tiedostot.csv` | 650 tiedostoa: luokka (`tuote`, `orpo`, `kolmas_osapuoli`, `testi`), tuotteet, funktiot, viitteet / per file |
| `hakemistot.csv` | hakemistoittain / per directory |
| `parit.csv` | HK- ja viestipuolen samannimiset funktiot kerroksittain (ydin, konsoli, GUI) ja niiden rivitason samankaltaisuus / same-named HK and relay functions with similarity |
| `ratkaisemattomat.csv` | viitteet, joiden polkua ei voitu yksilöidä (108; yleensä kahden tiedoston yhteinen nimi) / unresolved paths |

**Mitä luvut eivät tarkoita.** Viite tarkoittaa, että joku on katsonut kohtaa jonkin väitteen
vuoksi. Se ei tarkoita, että funktio on katselmoitu. "Viitealueen rivit" olettaa, että viitatun
rivin ympäriltä on luettu 10 riviä kumpaankin suuntaan; oletusta ei ole todennettu. Funktioiden
tunnistus on heuristinen: karkea vertailuhaku löysi 6 220 funktion alkua, joista tunnistimelta
puuttui 35, ja kaikki tarkistetut puuttuvat olivat kommentiksi jätettyä koodia.

**What the figures do not mean.** A reference shows that someone looked at the place for a claim, not
that the function was reviewed. Context lines (±10) are an unverified assumption. Function detection is
heuristic; against a crude pattern search it missed 35 of 6,220 heads, all checked ones commented out.
