#!/usr/bin/env python3
"""Generate mkdocs.yml from docs/.._nav.json produced by import_docs.py."""

import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
DOCS = HERE.parent / "docs"
OUT = HERE.parent / "mkdocs.yml"
NAV_JSON = HERE / "_nav.json"

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
    - navigation.instant
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

plugins:
  - search:
      lang: fi

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


def main():
    data = json.loads(NAV_JSON.read_text(encoding="utf-8"))

    lines = [HEADER, "nav:"]
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
