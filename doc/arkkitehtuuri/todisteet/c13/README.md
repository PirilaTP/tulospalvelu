# C-13: synteesin raakatulokset ja raportit / raw results and reports of the synthesis

Tuotettu 2026-11-26 revisiosta `3f378e2` (`TPsource/` ennallaan revisiosta `f7fe553`). Tulkinta:
`../../auditoinnin-yhteenveto.md`, `../../korjaussuunnitelma.md`,
`../../regressiotestaussuunnitelma.md`, `../../omistajan-paatokset.md`.

C-13 oli synteesi: mitään ei käännetty eikä ajettu, lukuun ottamatta auditoinnin omia inventaario-,
synteesi- ja validointiskriptejä. Lähdekoodia luettiin vain rajattujen väitteiden tarkistamiseksi;
kukin raportti kertoo, mitä se luki.

Raportit ovat agenttien loppuraportteja. Useimmat agentti kirjoitti itse tähän hakemistoon;
`osaD-yhteensopivuuskysymykset.md`:n ja `osaH-tarkastus.md`:n pääistunto kopioi agentin
työhakemistosta muuttamatta sisältöä. Agenttien väliaikaiset tunnisteet (JS-, FT-, O-, K-, S-)
ovat raporttikohtaisia; rekisterin tunnuksia ovat vain H-, T-, Q-, KK- ja OP-tunnukset.

Produced on 2026-11-26. C-13 was a synthesis: nothing was built or run except the audit's own
inventory, synthesis and validation scripts. The reports are the agents' final reports; labels such
as JS-, FT-, O-, K- and S- are local to a report.

| Tiedosto / File | Tekijä / By | Sisältö / Content |
|---|---|---|
| `osaA0/` | `tyokalut/synteesi.py`, pääistunto / main session | koneellinen pohja: yksi rivi kutakin H-, T- ja Q-tunnusta kohden, aiemmat KK-jäsenyydet, ristiviittaukset, makroartefaktit, yhteenveto; tarkistus `tarkistus_a0.py` ja sen tulos (`osaA0/README.md`) / machine base and its check |
| `raportit/osaA-kattavuus.md` | `architecture-analyst`, Sonnet | lukujen täsmäytys, vanhentuneet väitteet S1–S21, erien toteutuma, kattavuus, todennuksen vahvuus, avointen kysymysten luokittelu, rajoitteet / coverage and reliability |
| `raportit/osaB-inventointi.md`, `osaB/luokat.csv`, `osaB/suhteet.csv` | `architecture-analyst`, Sonnet | 27 vikaluokkaa (yksi kullekin avoimelle H:lle ja T:lle), 221 suhdetta, yhdessä korjattavat ryhmät, rekisterin epäjohdonmukaisuudet / defect classes and relations |
| `raportit/osaB-juurisyyt.md` | `architecture-analyst`, Opus | juurisyyt JS-1–JS-15, erien yli näkyvät syyt, korjausjärjestyksen kytkökset, pääistunnon ensimmäisen kokonaisuusluonnoksen kritiikki (27 muutospyyntöä) / root causes and critique of the first package draft |
| `raportit/osaC-riskiarvio.md`, `osaC/riskiarvio_luonnos.csv` | `architecture-analyst`, Opus | P1–P5:n käyttöhistoria ja yhtenäinen määritelmä, riskiulottuvuudet jokaiselle H:lle, rajatapaukset; luonnos ennen pääistunnon muutoksia (H-061, H-116) / uniform risk criteria and the draft before main-session overrides |
| `raportit/osaD-yhteensopivuuskysymykset.md` | `legacy-encoding-analyst`, Opus | seitsemän rajattua kysymystä kiireellisimpien korjausten vaikutuksesta tiedostoihin ja protokollaan (H-025, H-041, H-026, H-071, H-072 ym., H-129, H-171), vastattu koodista / compatibility questions for the most urgent fixes |
| `osaD/luonnos1_kk.csv` | pääistunto / main session | korjauskokonaisuuksien ensimmäinen luonnos ennen juurisyyanalyysin kritiikkiä; lopullinen on `../../rekisteri/korjauskokonaisuudet.csv` / first package draft |
| `osaD/tee_rekisterit.py` | pääistunto / main session | kertakäyttöinen skripti, jolla `../../rekisteri/riskiarvio.csv` ja `../../rekisteri/korjauskokonaisuudet.csv` tuotettiin Opus-luonnoksesta, pääistunnon kokonaisuusjaosta ja pääistunnon muutoksista (H-061, H-116, H-033, H-153, H-009); rekistereitä ylläpidetään tämän jälkeen käsin, ja `synteesi.py --tarkista` tarkistaa ne / one-off generator of the two registers, kept as provenance |
| `raportit/osaF-testausaineisto.md` | `build-dependency-analyst`, Sonnet | testiaineiston inventaario, automaation tila, 80 avointa todennustehtävää, matriisi T1–T18, P1/P2-aukot / test asset inventory |
| `raportit/osaH-tarkastus.md` | `independent-verifier`, Opus | riippumaton kokonaisarvio; käsittely `../../auditoinnin-yhteenveto.md` luku 11 / independent overall review |
| `SHA256SUMS` | – | tiivisteet / hashes |

Työnjako: Sonnet teki inventoinnit ja täsmäytykset (A, B-inventointi, F) sekä yhteenvedon
kattavuusosan kirjoituksen; Opus juurisyyt, riskiperusteet, yhteensopivuuskysymykset ja
riippumattoman arvion; pääistunto (Fable) A0:n, korjauskokonaisuudet (D), riippuvuudet ja
järjestyksen (E), kehittämissuunnitelman (G), testaussuunnitelman ja omistajan päätösten koonnin
sekä agenttien ehdotusten arvioinnin.

Toisto repon juuresta / reproduce from the repository root:

```
python -I doc/arkkitehtuuri/tyokalut/synteesi.py . doc/arkkitehtuuri/todisteet/c13/osaA0 --tarkista
python -I doc/arkkitehtuuri/todisteet/c13/osaA0/tarkistus_a0.py .
python -I doc/arkkitehtuuri/tyokalut/tarkista_auditointi.py .
```
