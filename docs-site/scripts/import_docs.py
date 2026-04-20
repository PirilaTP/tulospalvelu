#!/usr/bin/env python3
"""Import WinCHM-generated help sites into Markdown for MkDocs.

Fetches each source site's static TOC (webhelplefth.htm), parses the
chapter/section tree, downloads each content page, converts it to
Markdown, rewrites inter-page links and image references, and emits a
mkdocs.yml nav fragment per site.
"""

from __future__ import annotations

import json
import re
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path
from urllib.parse import urljoin, urlparse

import requests
from bs4 import BeautifulSoup
from markdownify import MarkdownConverter


BASE = "https://www.tulospalvelu.fi/pirila/"

# Each source gets a slug (dest subdir under docs/) and a title shown in nav.
SOURCES = [
    ("ohjeet/Asentaminen/",      "asentaminen",       "Asentaminen ja tutustuminen"),
    ("ohjeet/Suunnistuskilpailu/", "suunnistuskilpailu", "Suunnistuskilpailun tulospalvelu"),
    ("ohjeet/Hiihtokilpailu/",   "hiihtokilpailu",    "Hiihtokilpailun tulospalvelu"),
    ("ohjeet/Help/",             "hkkisawin",         "HkKisaWin-referenssi"),
    ("vieohjeet/help/",          "viestiwin",         "ViestiWin"),
]

SESSION = requests.Session()
SESSION.headers["User-Agent"] = "pirila-docs-import/1.0"


@dataclass
class TocNode:
    title: str
    href: str | None              # relative to site's scr/ dir, e.g. "1.1_Asentaminen.htm"
    depth: int                    # 0 = top chapter, 1 = section, 2 = subsection
    slug: str = ""                # filename stem of the converted markdown
    children: list["TocNode"] = field(default_factory=list)


def slugify(stem: str) -> str:
    """WinCHM filenames use Latin letters + underscores already; keep as-is
    (minus extension and punctuation), just lowercase for URL cleanliness."""
    s = re.sub(r"\.[a-zA-Z]+$", "", stem)
    s = re.sub(r"[^A-Za-z0-9_\-\.]+", "_", s)
    s = re.sub(r"_+", "_", s).strip("_")
    return s.lower() or "index"


def fetch(url: str) -> str:
    """Fetch URL, tolerating site's inconsistent encoding (some files utf-8,
    some iso-8859-1 / windows-1252)."""
    r = SESSION.get(url, timeout=30)
    r.raise_for_status()
    # Respect declared charset; fallback to UTF-8 which matches most pages.
    if not r.encoding or r.encoding.lower() == "iso-8859-1":
        # The webhelp pages declare UTF-8 in a meta tag but the server often
        # sends iso-8859-1 as default. Try utf-8 first, fall back to cp1252.
        for enc in ("utf-8", "cp1252"):
            try:
                return r.content.decode(enc)
            except UnicodeDecodeError:
                continue
    return r.text


def parse_toc(html: str) -> list[TocNode]:
    """Parse webhelplefth.htm into a flat list of TocNodes, nested by depth.

    WinCHM encodes depth as a chain of <img src='icons/line.gif'> prefixes
    before the branch/leaf icon. Depth = number of 'line.gif' images preceding
    the node's own icon (tshaped/upangle/etc.)."""
    soup = BeautifulSoup(html, "html.parser")
    root: list[TocNode] = []
    stack: list[TocNode] = []

    for div in soup.find_all("div"):
        nobr = div.find("nobr", recursive=False)
        if not nobr:
            continue
        # Count leading line.gif images = indentation depth.
        imgs = nobr.find_all("img", recursive=False)
        depth = sum(1 for img in imgs if img.get("src", "").endswith("line.gif"))
        a_tags = nobr.find_all("a", href=True, recursive=False)
        # The anchor we want has target='content'; others are collapse toggles.
        target = next((a for a in a_tags if a.get("target") == "content"), None)
        if target is None:
            continue
        title = target.get("title") or target.get_text(strip=True)
        href = target["href"]
        # Treat href="#" or anchor-only hrefs as branch-only nodes (no content page).
        if not href or href.startswith("#"):
            href = None
        node = TocNode(title=title.strip(), href=href, depth=depth)
        # Drop deeper entries off the stack
        while stack and stack[-1].depth >= depth:
            stack.pop()
        if stack:
            stack[-1].children.append(node)
        else:
            root.append(node)
        stack.append(node)

    return root


class HelpMarkdownConverter(MarkdownConverter):
    """Tweaks for WinCHM help pages."""

    def convert_img(self, el, text, parent_tags):
        # Drop the decorative nav icons; keep real images.
        src = el.get("src", "")
        if "/icons/" in src or src.startswith("icons/"):
            return ""
        return super().convert_img(el, text, parent_tags)


def html_to_markdown(html: str, link_map: dict[str, str]) -> tuple[str, list[str]]:
    """Return (markdown, image_srcs_to_download)."""
    soup = BeautifulSoup(html, "html.parser")

    # Strip frameset/script/style that sometimes bleed in from header fragments.
    for tag in soup(["script", "style", "meta", "link"]):
        tag.decompose()

    # WinCHM wraps pages in #winchm_template_top (breadcrumb + prev/next buttons)
    # and #winchm_template_content (actual content). Drop the top bar and extract
    # only the content div if present.
    for top in soup.find_all("div", id="winchm_template_top"):
        top.decompose()
    content_div = soup.find("div", id="winchm_template_content")
    if content_div is not None:
        # Replace the soup's body with just the content div.
        new_soup = BeautifulSoup("<html><body></body></html>", "html.parser")
        new_soup.body.append(content_div)
        soup = new_soup

    # Drop the per-page copyright footer. Original pages end with:
    #   <hr><font size="1">Copyright YYYY Pekka Pirilä</font>
    # The family has donated the docs under the same GPLv3 license as the code,
    # so these per-page notices are redundant with the repo-level license.
    for font in soup.find_all("font"):
        if "Copyright" in font.get_text() and "Piril" in font.get_text():
            # Remove preceding <hr> if present
            prev = font.find_previous_sibling()
            while prev and prev.name in (None, "br"):  # skip whitespace/br
                prev = prev.find_previous_sibling()
            if prev and prev.name == "hr":
                prev.decompose()
            font.decompose()

    # Rewrite inter-page links: ".htm#anchor" -> "target.md" (flat within site).
    for a in soup.find_all("a", href=True):
        href = a["href"]
        if href.startswith("#") or href.startswith("http"):
            continue
        # Strip fragment
        base = href.split("#", 1)[0]
        if base in link_map:
            a["href"] = link_map[base]

    # Collect image sources for downloading later.
    images: list[str] = []
    for img in soup.find_all("img"):
        src = img.get("src", "")
        if "/icons/" in src or src.startswith("icons/"):
            continue
        if src and not src.startswith(("http://", "https://")):
            images.append(src)
            # Rewrite to point to images folder at docs root
            img["src"] = f"../images/{Path(src).name}"

    # Prefer the <body> content if present.
    body = soup.body or soup
    html_fragment = str(body)

    converter = HelpMarkdownConverter(
        heading_style="ATX",
        bullets="-",
        strip=["font"],
    )
    md = converter.convert(html_fragment).strip()
    # Collapse 3+ blank lines.
    md = re.sub(r"\n{3,}", "\n\n", md)
    return md, images


def flatten_toc(nodes: list[TocNode]) -> list[TocNode]:
    out: list[TocNode] = []
    def walk(ns: list[TocNode]):
        for n in ns:
            out.append(n)
            walk(n.children)
    walk(nodes)
    return out


def build_link_map(nodes: list[TocNode]) -> dict[str, str]:
    """Map original filename -> destination markdown filename."""
    result: dict[str, str] = {}
    for n in flatten_toc(nodes):
        if not n.href:
            continue
        stem = Path(n.href).name              # "2.1_Uuden_kilpailun_luominen.htm"
        slug = slugify(stem)
        n.slug = slug
        result[stem] = f"{slug}.md"
        # Also map prefixed variant "scr/foo.htm"
        result[f"scr/{stem}"] = f"{slug}.md"
    return result


def import_site(rel_path: str, dest_slug: str, title: str, docs_root: Path,
                images_dir: Path) -> dict:
    site_url = urljoin(BASE, rel_path)
    print(f"\n=== {title} ({site_url}) ===")
    try:
        toc_html = fetch(urljoin(site_url, "webhelplefth.htm"))
    except requests.HTTPError as e:
        print(f"  ! no TOC at webhelplefth.htm ({e}); skipping")
        return {"slug": dest_slug, "title": title, "nav": []}

    nodes = parse_toc(toc_html)
    if not nodes:
        print("  ! empty TOC; skipping")
        return {"slug": dest_slug, "title": title, "nav": []}

    # Ensure per-site dir
    site_dir = docs_root / dest_slug
    site_dir.mkdir(parents=True, exist_ok=True)

    # Pre-compute filename mapping so link rewriting can resolve cross-page refs.
    link_map = build_link_map(nodes)

    downloaded_imgs: set[str] = set()
    page_count = 0
    for node in flatten_toc(nodes):
        if not node.href:
            continue
        page_url = urljoin(site_url, node.href)
        try:
            html = fetch(page_url)
        except requests.HTTPError as e:
            print(f"  ! {node.href}: {e}")
            continue

        md, images = html_to_markdown(html, link_map)

        # Prepend H1 title if the converted markdown doesn't already start
        # with one.
        if not re.match(r"^\s*#\s", md):
            md = f"# {node.title}\n\n{md}"

        out_path = site_dir / f"{node.slug}.md"
        out_path.write_text(md, encoding="utf-8")
        page_count += 1

        # Fetch images (once per unique path).
        for img_src in images:
            if img_src in downloaded_imgs:
                continue
            downloaded_imgs.add(img_src)
            img_url = urljoin(page_url, img_src)
            try:
                r = SESSION.get(img_url, timeout=30)
                r.raise_for_status()
                (images_dir / Path(img_src).name).write_bytes(r.content)
            except requests.HTTPError as e:
                print(f"    ! image {img_src}: {e}")
        time.sleep(0.05)

    print(f"  {page_count} pages, {len(downloaded_imgs)} images")
    return {
        "slug": dest_slug,
        "title": title,
        "nav": serialize_nav(nodes, dest_slug),
    }


def serialize_nav(nodes: list[TocNode], slug_prefix: str) -> list:
    """Produce mkdocs-compatible nested nav list.

    Each entry is either {"Title": "path.md"} for leaves with href, or
    {"Title": [children...]} for branches without own href, or branches
    with both get an "Overview" child pointing to their page."""
    out: list = []
    for n in nodes:
        entry_path = f"{slug_prefix}/{n.slug}.md" if n.slug else None
        if n.children:
            child_list: list = []
            if entry_path:
                child_list.append({n.title: entry_path})
            child_list.extend(serialize_nav(n.children, slug_prefix))
            out.append({n.title: child_list})
        elif entry_path:
            out.append({n.title: entry_path})
    return out


def main() -> int:
    docs_root = Path(__file__).resolve().parent.parent / "docs"
    images_dir = docs_root / "images"
    docs_root.mkdir(parents=True, exist_ok=True)
    images_dir.mkdir(parents=True, exist_ok=True)

    result = []
    for rel_path, slug, title in SOURCES:
        result.append(import_site(rel_path, slug, title, docs_root, images_dir))

    nav_path = docs_root.parent / "scripts" / "_nav.json"
    nav_path.write_text(json.dumps(result, ensure_ascii=False, indent=2),
                        encoding="utf-8")
    print(f"\nWrote nav summary to {nav_path.relative_to(docs_root.parent)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
