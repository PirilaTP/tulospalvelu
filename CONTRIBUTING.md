# Kehittämiseen osallistuminen / Contributing

Tervetuloa kehittämään Pekka Pirilän tulospalveluohjelmia. Tämä ohje kertoo, miten muutokset
tehdään, katselmoidaan ja kirjataan niin, että auditoinnin tietopohja pysyy ajan tasalla.
Aloitusohje, kehitysympäristö ja arkkitehtuuri: https://pirilatp.github.io/tulospalvelu/kehittajat/.
Tekoälytyökalujen säännöt: `AGENTS.md`.

*English summary at the end.*

## Mistä aloittaa

1. Lue [kehittäjän aloitusohje](https://pirilatp.github.io/tulospalvelu/kehittajat/) ja
   `doc/arkkitehtuuri/merkistot.md` (tiedostojen merkistöt; tärkein yksittäinen sääntö).
2. Tutustu korjattaviin asioihin: [vikakortisto](https://pirilatp.github.io/tulospalvelu/laatu/vikakortit/),
   [korjauskokonaisuudet](https://pirilatp.github.io/tulospalvelu/laatu/korjauskokonaisuudet/) ja
   avoimet [issuet](https://github.com/PirilaTP/tulospalvelu/issues).
3. Kysy keskusteluissa tai issuessa ennen suurta muutosta.

## Muutoksen tekeminen

- **Haara.** Tee oma haara `main`ista. Nimeä se issuen tai korjauskokonaisuuden mukaan
  (`86-…`, `korjaus/kk-c13-1-…`; kokonaisuuden suunniteltu haaranimi on sarakkeessa `haara`).
- **Merkistö ja rivinvaihdot.** Älä muuta tiedoston merkistöä, BOMia tai rivinvaihtoja. Tarkista
  ennen committia: `git diff --stat` ei saa näyttää koko tiedoston muuttuneen.
- **Kommentit.** Uudet kommentit englanniksi ja ASCII-merkein. Alkuperäisiä suomenkielisiä
  kommentteja ei käännetä eikä kirjoiteta uudelleen.
- **Kieli ja standardi.** Tuotantokoodi on C++03 (`CLAUDE.md`); doctest-testit C++11.
- **Kääntäminen ja testit.** Konsoliohjelmat MSVC:llä, GUI C++Builderilla (`CLAUDE.md`).
  Yksikkötestit: `TPsource/V52/VS/Tests/TpTest.sln`. Kuvaa PR:ssä, mitä käänsit ja testasit.
- **Pienet PR:t.** Yksi asia kerrallaan; korjaus ja uudelleenmuotoilu erikseen.

## Auditointihavainnon korjaaminen

Auditoinnin rekisterit (`doc/arkkitehtuuri/rekisteri/`) ovat kanoninen tietolähde. Kun PR korjaa
havainnon:

1. Viittaa havaintoon tunnuksella (H-NNN, T-NNN) ja sen pysyvään osoitteeseen
   `https://pirilatp.github.io/tulospalvelu/laatu/vikakortit/H-NNN/`.
2. Päivitä samassa PR:ssä `rekisteri/riskiarvio.csv` (tai `tekninen-velka.csv`): `tila` =
   `korjattu`, `korjaus` = commit, ohjelmaversio (esim. `v2026.12.1`) ja testi.
3. Aja tietopohjan päivitys ja tarkistukset (`AGENTS.md`, luku "Maintaining the knowledge base").
   Vikakortti, käsikirja, sivusto ja portaali päivittyvät rekisteristä; niitä ei muokata käsin.
4. Kirjoita kortin luku "Miten vika korjattiin" (`doc/tietopohja/kirjoitusohje.md`).

Älä merkitse havaintoa korjatuksi, jos korjaus kattaa vain osan siitä; kirjaa jäljelle jäävä osa.

## Katselmointi

Katselmoija tarkistaa ainakin:

- muuttuneisiin tiedostoihin osuvat havainnot (`rekisteri/havainnot.md`, polut suhteessa
  `TPsource/V52/`) ja käsikirjan toiminnot (`doc/tietopohja/kasikirja/toiminnot.csv`);
- että käyttäytyminen ei muutu huomaamatta tavalla, jonka käsikirja tai käyttöohje kuvaa;
- tiedostomuodon, protokollan ja laiteyhteyden yhteensopivuuden (korjauskokonaisuuden
  sarakkeet `yhteensopivuus_*`) ja regressiotestit (`regressiotestaussuunnitelma.md`);
- merkistön, BOMin ja rivinvaihdot;
- rekisteririvien päivityksen, jos PR korjaa havainnon.

Tekoälyavusteinen katselmointi on sallittua; katselmoija vastaa johtopäätöksistä
(`AGENTS.md`, luku "Pull request review with the knowledge base").

## Dokumentaatio

| Mitä | Missä | Miten |
|---|---|---|
| Käyttöohjeet | `docs-site/docs/` (paitsi `kehittajat/`) | muokataan suoraan |
| Kehittäjäosion ohjesivut | `docs-site/docs/kehittajat/*.md` | muokataan suoraan; ei lukumääriä käsin |
| Arkkitehtuuri ja auditointi | `doc/arkkitehtuuri/` | kaksikielinen, `dokumentointistandardi.md` |
| Tietopohja | `doc/tietopohja/` | `doc/tietopohja/README.md` |
| Sivuston generoidut osat | ei tiedostoja | `doc/arkkitehtuuri/tyokalut/sivusto.py` tuottaa rakennuksessa |

Sivuston paikallinen rakentaminen: `docs-site/README.md`.

## GitHubin käyttö

Rekisterit pysyvät kanonisena lähteenä; GitHubin työkalut viittaavat niihin eivätkä kopioi niitä.

- **Issues.** Issue tehdään, kun joku aikoo tehdä asian (korjaus, toteutus, selvitys) tai kun
  käyttäjä raportoi ongelman. Auditointihavainnosta ei tehdä issueta automaattisesti eikä
  massana. Issueen kirjataan havaintotunnus ja kortin pysyvä osoite; havainnon kuvaus pysyy
  kortissa. Uusi vika, jota rekisterissä ei ole, lisätään rekisteriin issuen käsittelyn yhteydessä.
- **Projects.** Projektitaulu voi seurata korjauskokonaisuuksia (yksi kortti kokonaisuutta kohden,
  sarakkeet aalto ja tila). Totuus tilasta on rekisterin `tila`-sarakkeessa; taulu on näkymä.
- **Discussions.** Käyttö- ja tulkintakysymykset. Kun keskustelusta syntyy auditointia koskeva
  avoin kysymys, se kirjataan `seuranta.md`:hen Q-numerolla ja keskusteluun linkki.
- **Tunnisteet (labels).** Ehdotus: `havainto`, `tekninen-velka`, `korjauskokonaisuus`,
  `dokumentaatio`, `tietopohja`, `P1`–`P5`.
- **Tietoturva.** Uusista, rekisterissä olemattomista tietoturvavioista ilmoitetaan omistajan
  ohjeistamalla tavalla (omistajan päätös OP-1); älä julkaise hyödyntämisohjeita.

## English summary

Branch from `main`, keep each PR to one concern, never change a file's encoding, BOM or line
endings, write new comments in English ASCII, describe what you built and tested. The audit
registers in `doc/arkkitehtuuri/rekisteri/` are the single source of truth: a fix updates its
register row (`tila` = `korjattu`, `korjaus` = commit, version, test) in the same PR, and the
cards, handbook, site and portal regenerate from the registers. Reviews check the findings and
handbook functions that touch the changed files, compatibility and regression tests. GitHub issues
reference finding ids and permanent card URLs instead of copying them; no bulk issue creation.
AI agents follow `AGENTS.md`.
