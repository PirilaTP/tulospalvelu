"""MkDocs hook: the knowledge base and audit pages, generated at build time.

The pages of the sections "Kehittäjille" and "Ohjelmiston laatu" (except the hand-written
pages in docs/kehittajat/) and the web portal are produced from the canonical registers and
documents in ../doc/ by doc/arkkitehtuuri/tyokalut/sivusto.py. Nothing is written into
docs/; the generated files exist only inside the build. See docs-site/README.md.

What the hook does:
- on_config: appends the navigation of the two sections (sivusto.nav) and watches ../doc/
  and ../.claude/ in `mkdocs serve`;
- on_files: adds the generated pages and portal files to the build;
- on_page_markdown: inserts generated fragments at <!-- tp:NAME --> markers, appends the
  knowledge-base box to user-manual pages listed in kayttoohjeet.csv, and points the
  "edit this page" link of a generated page to its canonical source.
"""
import hashlib
import logging
import os
import re
import sys

from mkdocs.exceptions import PluginError
from mkdocs.structure.files import File

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
TOOLS = os.path.join(ROOT, "doc", "arkkitehtuuri", "tyokalut")
sys.dont_write_bytecode = True
if TOOLS not in sys.path:
    sys.path.insert(0, TOOLS)
import sivusto  # noqa: E402

log = logging.getLogger("mkdocs.hooks.tietopohja")
_cache = {"key": None, "site": None, "fragments": None}
MARKER = re.compile(r"<!--\s*tp:([a-z_]+)\s*-->")


def _sources_key():
    h = hashlib.sha256()
    for top in ("doc", ".claude"):
        for d, dirs, fs in os.walk(os.path.join(ROOT, top)):
            dirs[:] = [x for x in dirs if x not in ("__pycache__", "data")]
            for f in fs:
                st = os.stat(os.path.join(d, f))
                h.update(("%s/%s %d %d" % (d, f, st.st_size, st.st_mtime_ns)).encode("utf-8", "replace"))
    h.update((sivusto.Site.__module__ + str(os.stat(os.path.join(TOOLS, "sivusto.py")).st_mtime_ns)).encode())
    return h.hexdigest()


def on_config(config):
    nav = config.get("nav") or []
    nav.extend(sivusto.nav(ROOT))
    config["nav"] = nav
    watch = config.get("watch")
    if watch is not None:
        for w in (os.path.join(ROOT, "doc", "arkkitehtuuri"), os.path.join(ROOT, "doc", "tietopohja"),
                  os.path.join(ROOT, ".claude")):
            if w not in watch:
                watch.append(w)
    return config


def on_files(files, config):
    key = _sources_key()
    if _cache["key"] != key:
        cache = os.path.join(ROOT, "docs-site", ".cache")
        site = sivusto.Site(ROOT, site_url=config.get("site_url") or "", cache_dir=cache).generate()
        _cache.update(key=key, site=site, fragments=site.fragments())
        for w in site.warnings + site.linker.warnings:
            log.info("tietopohja: %s", w)
        log.info("tietopohja: %d pages, %d assets, %d manual boxes, %d source links", len(site.pages),
                 len(site.assets), len(site.boxes), site.linker.stats["lahdelinkki"])
    site = _cache["site"]
    for uri, content in list(site.pages.items()) + list(site.assets.items()):
        if files.get_file_from_path(uri) is not None:
            raise PluginError("tietopohja: generated page %s collides with a file in docs/" % uri)
        files.append(File.generated(config, uri, content=content))
    for page in site.boxes:
        if files.get_file_from_path(page) is None:
            raise PluginError("tietopohja: kayttoohjeet.csv names a page that does not exist: %s" % page)
    return files


def on_page_markdown(markdown, page, config, files):
    site = _cache["site"]
    if site is None:
        return markdown
    uri = page.file.src_uri
    frags = _cache["fragments"]

    def rep(m):
        name = m.group(1)
        if name not in frags:
            raise PluginError("tietopohja: unknown fragment <!-- tp:%s --> in %s" % (name, uri))
        return frags[name]

    markdown = MARKER.sub(rep, markdown)
    if uri in site.boxes:
        markdown = markdown.rstrip() + "\n\n" + site.boxes[uri]
    repo = (config.get("repo_url") or sivusto.DEFAULT_REPO).rstrip("/")
    if uri in site.edit:
        page.edit_url = "%s/edit/main/%s" % (repo, site.edit[uri])
    elif uri in site.pages:
        page.edit_url = None
    return markdown
