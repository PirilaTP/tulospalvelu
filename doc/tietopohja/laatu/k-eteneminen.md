# Vaihe K: eteneminen ja jatko-ohje / Progress and resume notes

Päivitetään jokaisen välicommitin yhteydessä. Käyttäjän hyväksyntä 2026-11-27: pilotin rakenne,
tietomalli ja yksityiskohtaisuus hyväksytty; K-2–K-5 toteutetaan itsenäisesti hyväksymiskriteerien
mukaan (`doc/arkkitehtuuri/k-vaihe-suunnitelma.md`). Ei pushia, PR:iä, haarojen yhdistämistä,
E-1:tä eikä D-vaihetta; omistajan päätöksiä ei tehdä.

## Tila

| Osa | Tila |
|---|---|
| Emit-selvitys (K-3-aineisto) | `kasikirja/aineisto/emit-luenta.md` kirjoitettu (Opus), riippumattomasti tarkastettu ja korjattu (`tarkastukset/emit/`); ehdokkaat KE-10–KE-13 |
| Agentin kehitys | luku 3a (laajat ketjukysymykset), reittitaulukko erien raportteihin; `pirila-learn` suunniteltu (suunnitelma 5.7) |
| K-2 kortit | erät `k2-erat.json` (E01–E18, 217 korttia) + 9 pilottikortin päivitys uuteen muotoon; kaikki 229 korttia kirjoitettu ja kuitattu (E01–E18, pilotin päivitys); Opus-laadunvarmistus seuraavaksi |
| K-3 käsikirja | pohja `kasikirja/_malli-luku.md`, ohje `k3-kirjoittajan-ohje.md`; ei aloitettu |
| K-4 portaali | `portaali/index.html`, `app.js`, `tyyli.css`, generaattori `tyokalut/portaali.py`, savutesti `tyokalut/portaali_testi.js` |
| K-5 | ei aloitettu |

## Jatkaminen

1. Kirjoittaja-agentin tehtävänanto: `k2-kirjoittajan-ohje.md`; erät: `k2-erat.json`. Valmiin erän
   tunnistaa siitä, ettei kortin otsikossa ole tekstiä `<selkokielinen otsikko`.
2. Erän valmistuttua: `tietopohja.py . tuo <vaikutukset_Exx.csv> <ohjelmat_Exx.csv>`,
   `tietopohja.py . lohkot`, `tietopohja.py . kuittaa <erän tunnukset>`, huomiot tiedostoon
   `k2-kirjoittajien-huomiot.md`.
3. Kaikki tarkistukset: `tarkista_auditointi.py`, `synteesi.py --tarkista`, `viitteet.py`,
   `tietopohja.py tarkista`, `portaali.py --tarkista`, `node tyokalut/portaali_testi.js`.
