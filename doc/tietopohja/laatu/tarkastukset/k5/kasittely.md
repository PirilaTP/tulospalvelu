# K-5: kokonaisuuden laadunvarmistus ja sen käsittely / Overall QA and how it was handled

Päivämäärä 2026-11-27. Tarkastukset ovat vain lukevia; tuotantokoodia ei käännetty eikä ajettu.

## 1. Koneelliset tarkistukset

Todentavat rakenteen, lukumäärät, rekisterivastaavuuden, linkkien ja viitteiden olemassaolon ja
tuotosten ajantasaisuuden. **Ne eivät todenna väitteiden teknistä sisältöä.**

| Tarkistus | Tulos |
|---|---|
| Tunnukset ja kortit | 229 rekisteritunnusta, 229 korttia (177 H + 52 T), kaikki kuitattu rekisteririviä vasten |
| Vaikutukset | 176 avointa H × 16 ulottuvuutta = 2816 riviä; ristiintarkistus riskiarvion sarakkeisiin |
| Käsikirja | 15 lukua, 167 toimintokytkentää; jokainen avoin P1–P3-havainto kytketty vähintään yhteen toimintoon |
| Linkit | kortiston, käsikirjan ja README:iden suhteelliset linkit (yli 500) ratkeavat |
| Portaali | data ajan tasalla (`portaali.py --tarkista`); savutesti käy läpi kaikki näkymät, ei verkkoviitteitä |
| Auditointiaineisto | `tarkista_auditointi.py`, `synteesi.py --tarkista`, `viitteet.py` läpi |

## 2. Korttien sisältötarkastus (Opus)

Otos `laatu/k5-otos.json` (siemen 20261127): kaikki P1/P2-havainnot ja muut korkean vakavuuden
kortit (46 pakollista) sekä 37 satunnaista, yhteensä 83 korttia, erät Q1–Q6.

| Erä | Asiavirheet | Muut korjaukset |
|---|---|---|
| Q1 | 1 | 12 |
| Q2 | 2 | 17 |
| Q3 | 2 | 14 |
| Q4 | 1 | 12 |
| Q5 | 0 | 13 |
| Q6 | 0 kumottua, useita epätarkkuuksia | n. 12 |

Kaikki korjaukset tehtiin (editorikierrokset O1–O5). Järjestelmälliset virhetyypit S1–S14
(`laatu/k5-korjausohje.md`) korjattiin koko kortistosta, ei vain otoksesta. Käytännön vaikutukset
korjattiin rekisterin kautta (`tietopohja.py tuo`). Riskirekisterin kanssa ristiriitaiset kohdat
kirjattiin ehdokkaiksi (KE-14–KE-22); prioriteetteja tai havaintoja ei muutettu.

## 3. Käsikirjan sisältötarkastus (Opus)

Luvut 04, 05, 08 ja 09 (`kasikirja.md`): 79 väitettä, 50 vahvistettu, 20 osittain, 6 kumottu tai
osittain kumottu, 3 ei todennettavissa. Kaikki korjattu lähteestä tarkistaen; toistuvat virhetyypit
(HK/viesti-yleistys, koko ohjelman asetus kuvattu yhteyskohtaiseksi, kutsu ≠ onnistuminen, yksikkö
ja oletus katsomatta jäsentimestä, saman päivän korjaukset seuraamatta, liian heikot
[päätelty]-merkinnät) käytiin läpi myös muista yhdeksästä luvusta. Uudet ehdokkaat KE-23
(`LÄHTÖIKKUNA` millisekunteina) ja KE-24; KE-22:n virheellinen kehys korjattu.

## 4. Agentin uusintatesti

`k1-arvio.md` luku 9, `agentti.md`: 15 kysymystä, 8 hyväksytty / 7 huomautuksin / 0 hylätty;
väärät viitteet n. 4 % (aiemmin 8–10 %), väärät prioriteetit 0, kuollut koodi 0. Ohjeeseen lisättiin
kohdat 16–18; niiden kohdennettu pistokoe (`k1-vastaukset-v6/`) ei ole riippumaton arvio.

## 5. Rajoitteet

- Sisältö on tarkastettu otoksella: 146 korttia ja 11 käsikirjan lukua on tarkastettu vain
  koneellisesti ja virhetyyppikierroksilla.
- Mitään ei ole ajettu; "todennettu" tarkoittaa koodista luettua, ellei toisin mainita.
- Auditoinnin kattavuusrajoite (n. 36 % riveistä vain inventoitu) koskee koko tietopohjaa.
