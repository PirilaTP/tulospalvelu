# Docs site

[MkDocs Material](https://squidfunk.github.io/mkdocs-material/) -pohjainen
dokumentaatiosivusto <https://pirilatp.github.io/tulospalvelu/>. Se kokoaa

- Pekka Pirilän tulospalveluohjelmien käyttöohjeet, tuotu alkuperäiseltä sivustolta
  [tulospalvelu.fi/pirila](https://www.tulospalvelu.fi/pirila/ohjeet/) (`docs/`),
- osion **Kehittäjille** ja osion **Ohjelmiston laatu** (auditointi, vikakortit, kattavuus,
  korjaussuunnitelma, interaktiivinen portaali), jotka **generoidaan rakennuksen aikana**
  repositorion kanonisista lähteistä hakemistossa `../doc/`.

Julkaisu GitHub Pagesiin tehdään vain käsin: Actions → *Docs* → *Run workflow* `main`-haarasta,
valinta *julkaise* päällä (`.github/workflows/docs.yml`). Pull requestit ja pushit, jotka koskevat
dokumentaatiota, vain validoivat ja rakentavat sivuston (artefakti *docs-site*). Työnkulku ei käännä
eikä julkaise tulospalveluohjelmia.

## Kehitys paikallisesti

Windows (PowerShell), repon juuresta:

```
cd docs-site
python -m venv .venv
.venv\Scripts\python -m pip install -r requirements.lock
.venv\Scripts\mkdocs serve
```

Linux, macOS tai Git Bash: `python3 -m venv .venv`, `.venv/bin/pip install -r requirements.lock`
(Git Bashissa `.venv/Scripts/...`), `.venv/bin/mkdocs serve`.

Avaa selaimessa <http://127.0.0.1:8000/tulospalvelu/>. Ensimmäinen rakennus kestää noin puoli
minuuttia (kattavuusskripti ajetaan ja tulos välimuistitetaan hakemistoon `.cache/`). `serve`
seuraa myös hakemistoja `../doc/arkkitehtuuri/`, `../doc/tietopohja/` ja `../.claude/`.

Tarvitaan Python 3.10+ ja Git (lähdekoodilinkit lasketaan git-historiasta; kloonaa koko historia,
ei `--depth 1`). `requirements.txt` kertoo suorat riippuvuudet, `requirements.lock` testatut
tarkat versiot.

## Generoidut osiot

`hooks/tietopohja.py` (MkDocs-hook, ei lisäosia) kutsuu `../doc/arkkitehtuuri/tyokalut/sivusto.py`:tä,
joka tuottaa rekistereistä, auditointidokumenteista, vikakorteista ja käsikirjasta:

- sivut `kehittajat/arkkitehtuuri/`, `kehittajat/kasikirja/`, `kehittajat/asetukset/`,
  `kehittajat/tietopohja/` ja koko osion `laatu/` (myös jokaisen vikakortin pysyvä sivu
  `laatu/vikakortit/H-NNN/` ja korjauskokonaisuuden sivu `laatu/korjauskokonaisuudet/KK-…/`);
- osioiden navigaation (`sivusto.NAV`; ei `mkdocs.yml`:ssä);
- portaalin verkkoversion `laatu/portaali/app/` (sama sovellus kuin `../doc/tietopohja/portaali/`);
- tietolaatikot käyttöohjesivuille, jotka on lueteltu tiedostossa
  `../doc/tietopohja/kasikirja/kayttoohjeet.csv` (alkuperäistä ohjetta ei muuteta);
- käsin kirjoitettujen sivujen merkintöjen `<!-- tp:NIMI -->` sisällön (agentit, skillit, työkalut,
  `CONTRIBUTING.md`).

Generoituja tiedostoja ei kirjoiteta hakemistoon `docs/`, eikä niitä muokata käsin: muutos tehdään
lähteeseen. Käsin kirjoitetut kehittäjäsivut ovat hakemistossa `docs/kehittajat/`.
Lähdekoodiviitteet (`polku:rivi`) muuttuvat GitHub-linkeiksi väitteen revisioon
(`../doc/arkkitehtuuri/tyokalut/lahdelinkit.py`).

Generoidut sivut voi kirjoittaa tarkasteltaviksi hakemistoon:
`python -I doc/arkkitehtuuri/tyokalut/sivusto.py . <hakemisto>` (repon juuresta).

## Tarkistukset

Repon juuresta, kuten CI (Git Bash; Linuxissa `.venv/bin/mkdocs`, PowerShellissa
`.venv\Scripts\mkdocs` ja `cd` erillisinä komentoina). Julkaisun portti:
`python -I docs-site/scripts/julkaisulupa.py .` (sallii julkaisun vain, kun OP-1 on päätetty).

```
python -I doc/arkkitehtuuri/tyokalut/sivusto.py . --tarkista
cd docs-site && .venv/Scripts/mkdocs build --strict --site-dir ../site && cd ..
python -I docs-site/scripts/tarkista_sivusto.py site --osoitteet docs-site/scripts/osoitteet.txt
node doc/arkkitehtuuri/tyokalut/portaali_testi.js site/laatu/portaali/app
```

`tarkista_sivusto.py` tarkistaa ilman verkkoa jokaisen sisäisen linkin ja ankkurin, jokaisen
GitHub-lähdelinkin (commit, polku ja rivi paikallisesta git-historiasta) ja sen, että kaikki
`scripts/osoitteet.txt`:n vanhat osoitteet ovat yhä olemassa.

## Sisällön uudelleentuonti

Käyttöohjesivut on alun perin tuotu `scripts/import_docs.py` -skriptillä, joka crawlaa
alkuperäiset WinCHM-sivustot, muuntaa HTML:n Markdowniksi ja generoi navitaulun
`scripts/_nav.json`. Sen pohjalta `scripts/build_mkdocs_config.py` tuottaa `mkdocs.yml`:n (myös
hookin ja laajennukset).

Uudelleentuontiin (ei yleensä tarpeen; markdown-tiedostoihin tehdään muutokset suoraan):

```bash
cd docs-site
.venv/bin/pip install requests beautifulsoup4 markdownify
.venv/bin/python scripts/import_docs.py
.venv/bin/python scripts/build_mkdocs_config.py
```

## Rakenne

```
docs-site/
├── docs/                  # markdown-sivut
│   ├── index.md           # etusivu
│   ├── pdf-manuaalit.md   # linkit PDF-tiedostoihin
│   ├── asentaminen/       # Asennus- ja tutustumisohjeet
│   ├── suunnistuskilpailu/ # Suunnistustutoriaali
│   ├── hiihtokilpailu/    # Hiihtotutoriaali
│   ├── hkkisawin/         # HkKisaWin-referenssi (120 sivua)
│   ├── viestiwin/         # ViestiWin-ohjeet
│   ├── kehittajat/        # kehittäjäosion käsin kirjoitetut sivut
│   ├── stylesheets/       # sivuston lisätyylit
│   └── images/            # sivuilla viitatut kuvat
├── hooks/tietopohja.py    # generoi osiot Kehittäjille ja Ohjelmiston laatu
├── mkdocs.yml             # generoitu konfiguraatio (build_mkdocs_config.py)
├── requirements.txt       # suorat riippuvuudet
├── requirements.lock      # testatut tarkat versiot (CI)
└── scripts/               # tuonti-, konfiguraatio- ja tarkistusskriptit
```
