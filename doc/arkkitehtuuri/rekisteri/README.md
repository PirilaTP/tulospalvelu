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
| [valitsimet.csv](valitsimet.csv) | C-12: jokainen ehtona testattu makro: luokka, merkitys, arvo tuotteittain ja kokoonpanoittain, määrityskohdat, tila (`../tyokalut/valitsinrekisteri.py`) / every tested macro |
| [asetusavaimet.csv](asetusavaimet.csv) | C-12: asetusjäsentimien ja -editorien avaimet: verrattu pituus, rivit, ehto, voimassaolo tuotteittain / config keys of parsers and editors |
| [riskiarvio.csv](riskiarvio.csv) | C-13: yksi rivi kutakin H-havaintoa kohden: tekninen vakavuus, saavutettavuus, todennus, vaikutus tuloksiin ja ajanoton jatkuvuuteen, palautettavuus, tietoturva, prioriteetti P1–P5, aiempi prioriteetti ja muutos, ensisijainen korjauskokonaisuus (`kk`) ja toissijaiset (`kk_muut`), peruste ja huomiot. Prioriteetin ja kokonaisuuden osalta tämä tiedosto on ajantasainen lähde; `havainnot.md`:n riveillä olevat P- ja KK-merkinnät ovat erien historiaa / C-13: one row per H finding with the risk dimensions, priority and fix package; the current source for priority and package |
| riskiarvio.csv: K-0-sarakkeet / K-0 columns | Vaihe K-0 (2026-11-27) lisäsi sarakkeet `juurisyy` (JS-1–JS-15 tai `yksittainen`; kumotulla `-`), `juurisyy_muut` (pahentavat juurisyyt), `ohjelmat` (HkKisaWin, HkMaali, ViestiWin, ViestiMaali, ViestiLuenta tai `ei_tiedossa`), `ohjelmat_peruste` (`tiedosto` = yläraja: ohjelmat, joihin rivin viittaamat lähdetiedostot käännetään, `#ifdef`-haaroja huomioimatta; `tarkistettu` = tarkistettu kortin kirjoituksessa; `ei_tiedossa`), `tila` (`avoin`, `korjattu`, `kumottu`, `ei_korjata`) ja `korjaus` (commit ja testi, kun korjattu). Juurisyy on peräisin raportin `todisteet/c13/raportit/osaB-juurisyyt.md` liitteestä; tuottoskripti `todisteet/k0/tee_k0.py`. / Root cause, programs (upper bound unless checked), state and fix |
| [tekninen-velka.csv](tekninen-velka.csv) | K-0: yksi rivi kutakin T-riviä kohden, samat sarakkeet kuin yllä. T-rivin korjauskokonaisuus on vain `korjauskokonaisuudet.csv`:ssä (ei kopioida) / one row per T with the same columns; the package stays in the package register |
| [vaikutusulottuvuudet.csv](vaikutusulottuvuudet.csv), [vaikutukset.csv](vaikutukset.csv) | K-0: käytännön vaikutusten 16 ulottuvuutta (tulokset, ajanotto, leimaukset, tietojen säilyminen, kaatuminen, lukkiutuminen, hiljainen häiriö, tiedonsiirto, laitteet, julkaisu, tietoturva, normaalikäyttö, poikkeustilanne, havaittavuus, kiertotapa, palautuminen) ja niiden arviot pitkässä muodossa: rivi = havainto × ulottuvuus, arvo `vahvistettu` / `mahdollinen` / `ei_perusteltu` / `ei_tiedossa` / `ei_sovellu`, peruste ja lähde. Puuttuva rivi = ei arvioitu (ei sama kuin "ei vaikutusta"). Ristiriidat riskiarvion sarakkeiden `tulosten_oikeellisuus`, `ajanoton_jatkuvuus` ja `tietoturva` kanssa tarkistetaan. Kirjoitusohje: `doc/tietopohja/kirjoitusohje.md` luku 4 / practical impacts in long format; a missing row means not assessed |
| [korjauskokonaisuudet.csv](korjauskokonaisuudet.csv) | C-13: yksi rivi kutakin korjauskokonaisuutta kohden (34 + rivi `ULKOPUOLELLA`): jäsenet, juurisyy, riippuvuudet, aalto, työmäärä, muutosriski, yhteensopivuus, testit, omistajan päätökset, haara. Tarkistus: `python -I doc/arkkitehtuuri/tyokalut/synteesi.py . <out> --tarkista` (jokainen H ja T täsmälleen yhdessä kokonaisuudessa, ei syklejä, aallot riippuvuuksien mukaiset) / C-13: one row per fix package; checked by the script |
| [vaiheet.csv](vaiheet.csv) | Työvaiheiden ja erien tila (vaihe, nimi, tila, päivitetty, asiakirja, huomio); sivuston eteneminen- ja raporttisivut luetaan tästä. Päivitetään, kun vaihe tai erä etenee; `CHECKPOINT.md` jatkaa jatko-ohjeena / state of phases and batches, read by the site; updated when a phase advances |

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

Tasolla 4 on vain 19 C-11:n testitiedostoa, jotka auditointi itse kirjoitti; yhtäkään
tuotantotiedostoa ei ole tasolla 4. Erät C-10 ja C-12 eivät päivittäneet tiedostokohtaisia tasoja
(skripti-inventaario on taso 1). / Only the 19 C-11 test files written by the audit are at level 4;
no production file is. Batches C-10 and C-12 did not update per-file levels.

## Havaintojen kentät / Finding fields

Jokaisella havainnolla on / Every finding has:

- **ID**: `H-nnn` (vika / defect), `T-nnn` (tekninen velka / technical debt)
- **Näyttö / evidence**: `polku:rivi`, suhteessa / relative to `TPsource/V52/`
- **Vakavuus / severity**: korkea / keskitaso / matala (high / medium / low); käytössä ovat myös välimuodot keskitaso–korkea ja matala–keskitaso sekä ehdolliset arvot (esim. "korkea SQL-käytössä"); kriittistä ei ole annettu yhdellekään riville. Normalisoitu arvo: `../todisteet/c13/osaA0/havainnot.csv` / intermediate and conditional values are also in use; no row is critical
- **Varmuus / confidence**: [todennettu] / [päätelty] / [tuntematon]
- **Todennus / verification**: `ei tarkastettu` (not reviewed), `riippumaton lähdetarkastus` (independent source review: another agent re-read the code and tried to refute the claim), `toistettu testikehyksessä` (reproduced in test harness: the unchanged production functions, compiled with the shipped build settings, showed the defect mechanism in an automated test with a synthetic fixture; the actual program was not run, see `../testaus.md`), `vahvistettu ajamalla` (confirmed by running the product), `kumottu` (refuted), `ei todennettavissa staattisesti` (cannot be settled by reading code)
- **Revisio / revision**: commit, jota vastaan havainto tehtiin / commit analysed
- **Näytön laji / evidence type**: `staattinen` (lähdekoodi luettu; mitään ei ajettu / source read, nothing run), tai mitä muuta näyttöä on: kääntäjävaroitus, käännöstulos, testiajo / or what other evidence exists: compiler warning, build output, test run

Riippumaton lähdetarkastus ei ole ajonaikainen todennus. "Toistettu
testikehyksessä" tarkoittaa, että tuotannon funktiot näyttivät vikamekanismin
testiohjelmassa, jonka tila on asetettu käsin; se ei osoita, että vika laukeaa
tai millaiset seuraukset sillä on käynnissä olevassa ohjelmassa. Havainto on
"vahvistettu ajamalla" vasta, kun vika on toistettu käynnissä olevassa ohjelmassa.
Tällä hetkellä ainoa sellainen on H-041 (toimitettu HkMaali.exe, C-11 vaihe 4, käyttäjän
poikkeamapäätöksellä; `../testaus.md` 19.11).

An independent source review is not runtime verification. "Reproduced in test
harness" means the production functions showed the defect mechanism in a test
program whose state was set by hand; it does not show that the defect triggers,
or what it causes, in the running program. A finding is "confirmed by running"
only when the defect has been reproduced in the running program. At present only
H-041 is (shipped HkMaali.exe, C-11 stage 4, by a user-approved deviation).

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
