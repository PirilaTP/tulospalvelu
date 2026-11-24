# Auditointirekisteri / Audit register

## Tarkoitus / Purpose

Rekisteri kertoo, mitä koodista on oikeasti tutkittu, mitä on löydetty ja kuinka
varmoja löydökset ovat. Se on olemassa, jotta kukaan ei luule koko koodia
katselmoiduksi, kun vasta osa on luettu. Rekisteriä päivitetään jokaisen
analyysierän lopuksi.

The register records what has actually been examined in the code, what was found
and how certain the findings are. It exists so that nobody assumes the whole code
base has been reviewed when only part of it has been read. It is updated at the
end of every analysis batch.

| Tiedosto / File | Sisältö / Content |
|---|---|
| [tiedostot.csv](tiedostot.csv) | Jokainen lähdetiedosto: koko, merkistö, projektit, analyysitaso, dokumentit, havainnot, tarkistettu revisio / every source file |
| [moduulit.md](moduulit.md) | Moduulikohtainen kattavuus / per-module coverage |
| [havainnot.md](havainnot.md) | Viat, tekninen velka ja muut löydökset todisteineen / defects, technical debt and other findings with evidence |

Avoimet kysymykset ovat tiedostossa `../seuranta.md` (Q-numerot).
Open questions are in `../seuranta.md` (Q numbers).

## Analyysitasot / Analysis levels

| Taso / Level | Merkitys / Meaning |
|---|---|
| 0 | Ei käsitelty / not handled |
| 1 | Inventoitu koneellisesti: koko, merkistö, projektit, include-kaavio. Koodia ei ole luettu. / Inventoried by script only; code not read |
| 2 | Kohdennettu: tietyt funktiot tai rivit luettu tiettyä väitettä tai löydöstä varten (ks. `huomiot`) / Targeted: specific functions or lines read for a claim or finding |
| 3 | Rakenne: kaikki funktiot tunnistettu ja keskeiset luettu; tiedoston rooli dokumentoitu / Structure: all functions identified, key ones read, role documented |
| 4 | Katselmoitu: koko tiedosto luettu ja tarkistettu vikojen varalta / Reviewed: the whole file read and checked for defects |

Tasoa ei koskaan nosteta automaattisesti. Taso 4 vaatii, että tiedosto on luettu
kokonaan; hakemistotason tai skriptillä tehty analyysi on aina taso 1.

The level is never raised automatically. Level 4 requires reading the whole file;
directory-level or scripted analysis is always level 1.

## Havaintojen kentät / Finding fields

Jokaisella havainnolla on / Every finding has:

- **ID**: `H-nnn` (vika / defect), `T-nnn` (tekninen velka / technical debt)
- **Näyttö / evidence**: `polku:rivi`, suhteessa / relative to `TPsource/V52/`
- **Vakavuus / severity**: kriittinen / korkea / keskitaso / matala (critical / high / medium / low)
- **Varmuus / confidence**: [todennettu] / [päätelty] / [tuntematon]
- **Todennus / verification**: `ei tarkastettu` (not reviewed), `riippumaton lähdetarkastus` (independent source review: another agent re-read the code and tried to refute the claim), `vahvistettu ajamalla` (confirmed by running the product), `kumottu` (refuted), `ei todennettavissa staattisesti` (cannot be settled by reading code)
- **Revisio / revision**: commit, jota vastaan havainto tehtiin / commit analysed
- **Näytön laji / evidence type**: `staattinen` (lähdekoodi luettu; mitään ei ajettu / source read, nothing run), tai mitä muuta näyttöä on: kääntäjävaroitus, käännöstulos, testiajo / or what other evidence exists: compiler warning, build output, test run

Riippumaton lähdetarkastus ei ole ajonaikainen todennus. Havainto on
"vahvistettu ajamalla" vasta, kun vika on toistettu käynnissä olevassa ohjelmassa.
Tällä hetkellä yksikään H-havainto ei ole sellainen.

An independent source review is not runtime verification. A finding is
"confirmed by running" only when the defect has been reproduced in the running
program. At present no H finding is.

## Päivittäminen / Updating

```sh
python -I doc/arkkitehtuuri/tyokalut/inventaario.py . <tmp>/inv.json
python -I doc/arkkitehtuuri/tyokalut/rekisteri.py <tmp>/inv.json doc/arkkitehtuuri/rekisteri/tiedostot.csv <git-rev>
```

Skripti päivittää mitatut sarakkeet ja säilyttää käsin täytetyt (`analyysitaso`,
`arkkitehtuuri`, `kommentointi`, `tarkistettu_rev`, `havainnot`, `huomiot`).
Poistuneet tiedostot jäävät rekisteriin merkinnällä "poistunut".

The script refreshes measured columns and keeps manual ones. Removed files stay in
the register marked "poistunut".
