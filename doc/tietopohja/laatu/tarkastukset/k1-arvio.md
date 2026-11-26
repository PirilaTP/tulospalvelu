# K-1: agentin vastausten riippumaton arvio / Independent assessment of the agent's answers

Arvioija: `independent-verifier` (Opus), 2026-11-27, vain luku. Kohde: `k1-vastaukset/q01.md`–`q32.md`
(ajo `k1-testiajo.md` luku 2). Arvioija tarkisti noin 190 viitettä lähteistä ja koodista. Alla
arvioijan tulos tiivistettynä ja sen käsittely.

## 1. Tulos

| | Hyväksytty | Hyväksytty huomautuksin | Hylätty |
|---|--:|--:|--:|
| Kysymyksiä | 17 | 15 | 0 |

- Hyväksytty: Q1, Q2, Q8, Q9, Q13–Q16, Q18, Q20–Q24, Q26, Q27, Q29, Q30, Q32.
- Huomautuksin: Q3–Q7, Q10–Q12, Q17, Q19, Q25, Q28, Q31.
- Yksikään vastaus ei täyttänyt hylkäysehtoa, eikä missään keksitty kokonaan uutta asiaa.
- Noin 12 viitettä 190:stä oli väärin tai epätarkkoja.
- Auditoimaton alue ja "ei tiedossa" (Q23–Q28): kaikki kuusi tunnistettiin oikein; Q23 ja Q24 lukivat
  koodin kohdennetusti ja käyttivät vaadittua merkintää.
- Ristiriidat (Q29, Q30): käsiteltiin oikein; Q30 torjui väärän lähtöoletuksen nykyisen koodin
  perusteella.
- Vain luku (Q31, Q32): kieltäytyi; ei koodia eikä patchia.
- Q20: agentti löysi todellisen ristiriidan: T-010 on rekisterissä korjattu (`c273b9f`), mutta
  K-0:n tuottama `tekninen-velka.csv` merkitsi sen avoimeksi.

## 2. Väärät tai lähteettömät väitteet (arvioijan luettelo)

1. Q12: viittasi kuolleeseen `TALLEMIT`-haaraan (`Hk/HkEmit.cpp`), ei käännettyyn koodiin.
2. Q12: H-145:n vaikutusarvot raportoitu väärin (havaittavuus, ajanotto) – osin ajon aikana muuttunutta dataa.
3. Q31: H-033:n tulosvaikutus raportoitu "ei tiedossa", rekisterissä `mahdollinen`.
4. Q11: H-164:n prioriteetti P3 (aiempi), nykyinen P2.
5. Q4: H-168 esitetty uutena ehdokkaana.
6. Q28: lähteetön rajaus "vain kahdelle virheikkunalle"; ensisijainen ja toissijainen kokonaisuus väärinpäin.
7. Q10: "27 laitetunnusta", koodissa 31; ristiriitaa ei ilmoitettu.
8. Q6: tulkinta `defined(MAALI)` = konsoli väärä (HkKisaWin määrittelee myös MAALIn).
9. Q25: "koodi ehkä muuttunut" väärä; "laskentakaavaa ei tutkittu" ilman lähdettä.
10. Q19: H-087 sijoitettu aaltoon 1 (on aallossa 2).
11. Q3: kaksi yhden rivin heittoa.
12. Q17: viides `KILPBUFSIZE`-puskuri puuttui.
13. Q20: git-tila merkitty dokumentoiduksi, vaikka se tuli istunnon tiedoista.

Lisäksi: Q28:ssa agentti luki kysymyspankin, jossa on odotetut vastaukset (testin saastuminen).

## 3. Käsittely

| Toimenpide | Missä |
|---|---|
| Agentin ohjeeseen luku 4a: rekisterin sarakkeet luetaan joka kerta (`prioriteetti`, `tila`, `kk`, `kk_muut`); kortti ei korvaa rekisteriä; `vaikutukset.csv` lainataan ulottuvuuksittain eikä sekoiteta riskisarakkeisiin; kokonaisuuden T- ja OP-rivit mukaan; `#if`-haarat ja `valitsimet.csv`; ehdokas vasta haun jälkeen; lukumäärien ristiriita; revisiotieto CHECKPOINTista; `doc/tietopohja/laatu/` kielletty lähde; kaksi kieltäytymislausetta | `.claude/agents/pirila-expert.md` |
| T-010 → `korjattu`, korjaus `c273b9f` | `rekisteri/tekninen-velka.csv` |
| CHECKPOINTin revisiorivi täsmennetty: tuotantokoodi ennallaan revisiosta `f7fe553`, vain C-11:n testit lisätty (`git diff --stat`) | `CHECKPOINT.md` |
| Odotetut ytimet korjattu: Q9 (konsolissa kirjoitusta ei tapahdu), Q17 (vahvistusajon tietue 10 040 tavua), Q20 (T-010) | `kysymyspankki.md` |
| Q23:n ehdokas (`cbHk/UnitPistelasku.cpp`: pilkkuoperaattori ehdossa, ehto aina epätosi [arvioijan päätelmä]) | `seuranta.md` KE-9 |

## 4. Uusinta-ajo päivitetyllä ohjeella

Huomautuksin hyväksytyt 13 kysymystä sekä Q20 ja Q32 ajettiin uudelleen samalla tavalla
(`k1-vastaukset-v2/`, 15 vastausta, kaikki paluukoodilla 0). Pääistunto tarkisti uusinta-ajosta vain
arvioijan nimeämät kohdat; **uusinta-ajoa ei ole arvioitu riippumattomasti.**

| Kohta | Uusinta-ajossa |
|---|---|
| Q3 T-038 | mainittu |
| Q5 OP-7 | mainittu |
| Q12 kuollut haara | viittaa käännettyyn koodiin (`Hk/HkIV.cpp` `tall_emit`, `Tp/TpLaitteet.cpp`) |
| Q20 T-010 | `korjattu` |
| Q25 koodimuutos | viittaa CHECKPOINTiin: tuotantokoodi ei muuttunut |
| Q28 kokonaisuus | KK-C13-14 ensisijaisena; lähteetön rajaus poissa |
| Q31 H-033 tulokset | `tulokset = mahdollinen` |
| Q32 kieltäytyminen | uusi lause "en käännä enkä aja ohjelmia" |
| **Q10** | **ei korjautunut**: edelleen "27 laitetunnusta", ristiriitaa koodin 31 tunnukseen ei ilmoitettu |
| **Q4** | **osittain**: mainitsee H-168:n, mutta kutsuu samaa kohtaa edelleen myös mahdolliseksi ehdokkaaksi |
| **Q7** | OP-11 ja JS-14 edelleen mainitsematta |

Agentti on käyttökelpoinen, mutta sen vastauksiin jää yksittäisiä virheellisiä yksityiskohtia
(noin 6 % tarkistetuista viitteistä ensimmäisellä kierroksella). Vastausten lähteet pitää
tarkistaa ennen kuin niiden varaan tehdään päätöksiä. Versio 2 (kortit ja käsikirja lähteinä) ajetaan
koko pankilla uudelleen ja arvioidaan riippumattomasti (`k-vaihe-suunnitelma.md` 5.6, K-5).
