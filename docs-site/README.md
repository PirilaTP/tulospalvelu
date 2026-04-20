# Docs site

[MkDocs Material](https://squidfunk.github.io/mkdocs-material/) -pohjainen
dokumentaatiosivusto, joka kokoaa Pekka Pirilän tulospalveluohjelmien ohjeet
alkuperäiseltä sivustolta [tulospalvelu.fi/pirila](https://www.tulospalvelu.fi/pirila/ohjeet/).

Deployataan automaattisesti GitHub Pagesille kun `main`-branchiin puskaan muutoksia
`docs-site/`-hakemistoon (ks. `.github/workflows/docs.yml`).

## Kehitys paikallisesti

```bash
cd docs-site
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt
.venv/bin/mkdocs serve
```

Avaa selaimessa http://127.0.0.1:8000/.

## Sisällön uudelleentuonti

Sivut on alun perin tuotu `scripts/import_docs.py` -skriptillä, joka crawlaa
alkuperäiset WinCHM-sivustot, muuntaa HTML:n Markdowniksi ja generoi
`docs/.._nav.json`-navitaulun. Sen pohjalta `scripts/build_mkdocs_config.py`
tuottaa `mkdocs.yml`:n.

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
│   └── images/            # sivuilla viitatut kuvat
├── mkdocs.yml             # generoitu konfiguraatio
├── requirements.txt       # build-riippuvuudet
└── scripts/               # tuontiskriptit (kehittäjille)
```
