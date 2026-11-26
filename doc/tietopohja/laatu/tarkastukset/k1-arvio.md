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

## 5. Toinen arvio: uusinta-ajon 15 vastausta (Opus)

Riippumaton arvioija (`independent-verifier`, Opus, 2026-11-27) arvioi `k1-vastaukset-v2/`:n
15 vastausta ja tarkisti noin 165 viitettä. Tulos: 4 hyväksytty (Q6, Q17, Q31, Q32), 11
huomautuksin, 0 hylätty; noin 13 virheellistä viitettä (noin 8 %). Ensimmäisen kierroksen
virheistä useimmat korjautuivat, mutta samat virhetyypit toistuivat uusissa kohdissa: viittaus
koodiin, jota ei käännetä (SPORTIDENT, `LAJUNENxx`, `LUENTA`-haara), vanhentunut prioriteetti
(H-164 P3, nykyinen P2), väite git-tilasta istunnon tiedoista (Q20, Q28), rekisteröity havainto
ehdokkaana (Q4/H-168), lukumäärän ristiriita ilmoittamatta (Q10), OP-11 ja JS-14 puuttuivat (Q7).

Käsittely: agentin ohjeeseen luku 4b "Mandatory checks before you answer" (lukumäärät koodia vasten,
rekisteröity vai uusi, OP ja Q aiheen mukaan, käännösehto jokaiselle viitteelle ja luetellulle
kohteelle, viitteen sisältö, rekisterikenttiä ei saa kuitata "en tarkistanut", revisiotieto vain
CHECKPOINTista, pitkien osumarivien luku riviltä) sekä vastausmuotoon pakollinen
**Tarkistukset**-rivi, jolla haut ja niiden tulokset näkyvät. Yksittäisten kysymysten vastauksia
ei kirjattu ohjeeseen.

Välikokeet (pääistunnon pistokokeet, `claude -p --agent pirila-expert --tools Read,Grep,Glob`):
luvun 4b ensimmäisellä versiolla Q10, Q34–Q38 korjaantuivat, Q4 ja Q7 eivät. Tarkistusrivin
lisäämisen jälkeen Q7 (OP-11) ja Q33 (lukkojen laskenta koodista) korjaantuivat. Q4:ssä Sonnet
kertoi itse jättäneensä symbolihaun tekemättä; **Opus samalla ohjeella teki haun ja tunnisti
H-168:n** (`k1-vastaukset-v3/q04-opus.md`, ajettu ennen luvun 4b kohtia 6–15).

## 6. Kolmas arvio: 14 vastausta päivitetyllä ohjeella (Opus)

Ajettu fail-closed-käynnistyksellä, Sonnet, jokainen omana prosessinaan: arvion huomauttamat Q4,
Q5, Q7, Q10, Q11, Q12, Q20, Q28 ja uudet Q33–Q38 (`k1-vastaukset-v3/`). Uudet kysymykset testaavat
samoja periaatteita eri kohdissa: lukumäärä dokumentissa ja koodissa (Q33, lukot), rekisteröity
havainto (Q34, H-090), omistajan päätös aiheen mukaan (Q35, OP-15), käännösvalitsin ja ohjelmaero
(Q36, RFID ViestiLuennassa), nykyinen koodi vs auditointirevisio (Q37, H-001) ja vain luku (Q38).

Riippumaton arvioija tarkisti noin 110 viitettä ja ajoi Tarkistukset-rivien haut uudelleen:

| | Hyväksytty | Huomautuksin | Hylätty |
|---|--:|--:|--:|
| Kysymyksiä | 7 (Q12, Q20, Q28, Q35, Q36, Q37, Q38) | 7 (Q4, Q5, Q7, Q10, Q11, Q33, Q34) | 0 |

- **Poistuneet virhetyypit:** rekisteröity havainto ehdokkaana (Q4, Q34), git-tila istunnosta
  (kaikki viittaavat CHECKPOINTiin), testikansion lukeminen, vaikutusarvojen ja riskisarakkeiden
  sekoittaminen, kuollut haara Q12:ssa.
- **Jäljellä:** väärä prioriteetti (H-164), kuollut haara Q4:ssä (`TALLEMIT`), lukumäärän ristiriita
  Q10:ssä, JS-14 Q7:ssä, rekisterikenttiä ja OP-tiloja jätetään hakematta.
- Kaikkien 14 vastauksen johtopäätös oli oikein; noin 8–10 % kooditason viitteistä väärin,
  kasautuen käännösehtoihin ja konsoli/GUI-eroihin.
- Arvioijan käyttökelpoisuusarvio: kilpailun järjestäjälle käyttökelpoinen, epävarmuus merkitty
  johdonmukaisesti; kehittäjälle hakuapu mutta ei lähde sellaisenaan – jokainen `path:line` ja sen
  `#if` tarkistettava ennen käyttöä.

Käsittely: ohjeen luvun 4b kohdat 10–15 (kaikki funktion määrittelyt, `_CONSOLE`-lohkot,
prioriteetti vain tämän ajon hausta, lukumäärä kahtena lukuna, vähemmän mutta tarkistetut
sivuhavainnot, merkintä kertoo todellisen lähteen).

## 7. Viimeinen kohdennettu uusinta (pääistunnon pistokoe, ei riippumatonta arviota)

Q4, Q10 ja Q11 ajettiin kohtien 10–15 jälkeen (`k1-vastaukset-v4/`):

| Kohta | Tulos |
|---|---|
| Q4 kuollut `TALLEMIT`-haara | korjautui: viittaa käännettyyn `addem`iin, kertoo toisen määrittelyn olevan ehdon sisällä |
| Q4 `_CONSOLE` | korjautui: "vain konsoliohjelmassa" |
| Q4 SportIdent | korjautui: "vain jos `SPORTIDENT` on määritelty" |
| Q10 lukumäärä | korjautui: "dokumentti 27, koodi 31", ristiriita ilmoitettu |
| Q10 H-164 | **ei korjautunut**: väittää hakeneensa riskiarviosta, mutta kirjoittaa P3 (sarakkeen `prioriteetti_aiempi` arvo; nykyinen P2) |
| Q11 H-164 | korjautui: "P3 → P2" |

## 8. Agentin jäljellä olevat puutteet

1. Sarakkeiden sekoittuminen pitkillä CSV-riveillä (prioriteetti vs aiempi prioriteetti) toistuu
   Sonnetilla ohjeesta huolimatta.
2. Sonnet jättää osan pakollisista hauista tekemättä, varsinkin kun vastaus mainitsee monta
   sivuhavaintoa; puute näkyy Tarkistukset-rivillä, mikä tekee siitä havaittavan.
3. Käännösehdot tarkistetaan nyt pyydetyistä funktioista, mutta pitkissä vastauksissa yksittäisiä
   viitteitä kuolleeseen tai toisen kokoonpanon haaraan voi jäädä.
4. JS-tunnukset (juurisyy) puuttuvat usein, vaikka ne ovat rekisterissä.
5. Opus noudatti tarkistuksia paremmin yhdessä kokeessa (Q4); yksi koe ei riitä yleistykseen.
6. Ajojen työkalulokeja ei tallennettu, joten Tarkistukset-rivin hakujen tekemistä ei voi todentaa
   muuten kuin toistamalla haut.

Suositus: kilpailun järjestäjän kysymyksiin Sonnet riittää johtopäätösten tasolla; kehittäjän
kysymyksiin ja kaikkeen, mitä käytetään päätöksen pohjana, `-Model opus` ja lähteiden tarkistus.
Seuraava kokonainen arvio tehdään versiolle 2 (kortit lähteinä) K-5:ssä.
