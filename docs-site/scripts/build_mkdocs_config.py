#!/usr/bin/env python3
"""Generate mkdocs.yml from docs/.._nav.json produced by import_docs.py."""

import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
DOCS = HERE.parent / "docs"
OUT = HERE.parent / "mkdocs.yml"
NAV_JSON = HERE / "_nav.json"
# Navigation label translations: {"sv": {"<fi label>": "<sv label>", ...}, "en": {...}}
NAV_TRANSLATIONS_JSON = HERE / "nav_translations.json"

# Language versions (mkdocs-static-i18n, suffix structure: page.md = fi,
# page.sv.md = sv, page.en.md = en). Missing translations fall back to Finnish.
LANGUAGES = [
    {
        "locale": "fi",
        "name": "Suomi",
        "default": True,
        "palette_names": ("Vaihda tummaan teemaan", "Vaihda vaaleaan teemaan"),
    },
    {
        "locale": "sv",
        "name": "Svenska",
        "site_name": "Pekka Piriläs resultatprogram",
        "site_description": "Bruksanvisningar för Pekka Piriläs resultatprogram "
        "(HkKisaWin, ViestiWin, HkMaali).",
        "palette_names": ("Byt till mörkt tema", "Byt till ljust tema"),
    },
    {
        "locale": "en",
        "name": "English",
        "site_name": "Pekka Pirilä's results software",
        "site_description": "User manuals for Pekka Pirilä's results software "
        "(HkKisaWin, ViestiWin, HkMaali).",
        "palette_names": ("Switch to dark mode", "Switch to light mode"),
    },
]

HEADER = """\
site_name: Pekka Pirilän tulospalveluohjelmat
site_description: Vanhojen tulospalveluohjelmien (HkKisaWin, ViestiWin, HkMaali)
  käyttöohjeet, tuotu alkuperäisiltä sivuilta markdown-muotoon.
site_url: https://pirilatp.github.io/tulospalvelu/
repo_url: https://github.com/PirilaTP/tulospalvelu
repo_name: PirilaTP/tulospalvelu
edit_uri: edit/main/docs-site/docs/

theme:
  name: material
  language: fi
  features:
    # navigation.instant left out: incompatible with the i18n plugin's
    # contextual language switcher (same page in the other language)
    - navigation.tracking
    - navigation.tabs
    - navigation.sections
    - navigation.top
    - navigation.footer
    - search.suggest
    - search.highlight
    - content.code.copy
    - content.action.edit
    - toc.follow
  palette:
    - media: "(prefers-color-scheme: light)"
      scheme: default
      primary: indigo
      accent: indigo
      toggle:
        icon: material/brightness-7
        name: Vaihda tummaan teemaan
    - media: "(prefers-color-scheme: dark)"
      scheme: slate
      primary: indigo
      accent: indigo
      toggle:
        icon: material/brightness-4
        name: Vaihda vaaleaan teemaan
  icon:
    repo: fontawesome/brands/github
  font:
    text: Inter
    code: JetBrains Mono

markdown_extensions:
  - admonition
  - attr_list
  - md_in_html
  - tables
  - toc:
      permalink: true
  - pymdownx.highlight:
      anchor_linenums: true
  - pymdownx.inlinehilite
  - pymdownx.snippets
  - pymdownx.superfences
  - pymdownx.details

extra:
  social:
    - icon: fontawesome/brands/github
      link: https://github.com/PirilaTP/tulospalvelu

"""


def emit_nav(nav, indent):
    lines = []
    pad = "  " * indent
    for entry in nav:
        for title, value in entry.items():
            # YAML-escape title if it contains special chars
            key = f'"{title}"' if any(c in title for c in ':#-?[]{}') else title
            if isinstance(value, str):
                lines.append(f"{pad}- {key}: {value}")
            else:
                lines.append(f"{pad}- {key}:")
                lines.extend(emit_nav(value, indent + 1))
    return lines


def yaml_str(s):
    # JSON strings are valid YAML double-quoted scalars
    return json.dumps(s, ensure_ascii=False)


def emit_plugins():
    nav_tr = {}
    if NAV_TRANSLATIONS_JSON.exists():
        nav_tr = json.loads(NAV_TRANSLATIONS_JSON.read_text(encoding="utf-8"))

    lines = [
        "plugins:",
        "  - search",
        "  - i18n:",
        "      docs_structure: suffix",
        "      fallback_to_default: true",
        "      reconfigure_material: true",
        "      reconfigure_search: true",
        "      languages:",
    ]
    for lang in LANGUAGES:
        lines.append(f"        - locale: {lang['locale']}")
        lines.append(f"          name: {lang['name']}")
        lines.append("          build: true")
        if lang.get("default"):
            lines.append("          default: true")
        for key in ("site_name", "site_description"):
            if key in lang:
                lines.append(f"          {key}: {yaml_str(lang[key])}")
        if not lang.get("default"):
            dark, light = lang["palette_names"]
            lines += [
                "          theme:",
                "            palette:",
                '              - media: "(prefers-color-scheme: light)"',
                "                scheme: default",
                "                primary: indigo",
                "                accent: indigo",
                "                toggle:",
                "                  icon: material/brightness-7",
                f"                  name: {yaml_str(dark)}",
                '              - media: "(prefers-color-scheme: dark)"',
                "                scheme: slate",
                "                primary: indigo",
                "                accent: indigo",
                "                toggle:",
                "                  icon: material/brightness-4",
                f"                  name: {yaml_str(light)}",
            ]
        translations = nav_tr.get(lang["locale"])
        if translations:
            lines.append("          nav_translations:")
            for fi, tr in translations.items():
                lines.append(f"            {yaml_str(fi)}: {yaml_str(tr)}")
    lines.append("")
    return "\n".join(lines) + "\n"


def main():
    data = json.loads(NAV_JSON.read_text(encoding="utf-8"))

    lines = [HEADER + emit_plugins(), "nav:"]
    lines.append("  - Etusivu: index.md")

    for site in data:
        if not site["nav"]:
            continue
        lines.append(f"  - \"{site['title']}\":")
        lines.append(f"    - Yleiskatsaus: {site['slug']}/index.md")
        lines.extend(emit_nav(site["nav"], indent=2))

    lines.append("  - PDF-manuaalit: pdf-manuaalit.md")

    OUT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {OUT}")


if __name__ == "__main__":
    main()
