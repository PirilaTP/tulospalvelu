#!/usr/bin/env python3
"""Shared generation layer: documentation-site pages and web portal data from the registers.

Usage (from the repository root):
    python -I doc/arkkitehtuuri/tyokalut/sivusto.py . <out_dir>   write every generated page and asset
    python -I doc/arkkitehtuuri/tyokalut/sivusto.py . --tarkista  validate the mapping files and links
The MkDocs hook docs-site/hooks/tietopohja.py calls generate() at every `mkdocs build` and
`mkdocs serve`, so the site never needs a hand-maintained copy of the knowledge base.

Sources (canonical, never written here):
    doc/arkkitehtuuri/rekisteri/*          registers (findings, risk, impacts, packages, files)
    doc/arkkitehtuuri/*.md                 audit and architecture documents
    doc/tietopohja/vikakortit/, kasikirja/ defect cards, handbook, toiminnot.csv, kayttoohjeet.csv
    doc/arkkitehtuuri/todisteet/c14/       coverage baseline; tyokalut/kattavuus.py is re-run
    .claude/agents/, .claude/skills/       agent and skill definitions (front matter)
Output: Markdown pages under the site URIs below (kehittajat/..., laatu/...), the portal
(laatu/portaali/index.html, app.js, tyyli.css, data/data.js) and the info boxes added to user
manual pages listed in kayttoohjeet.csv. No numbers are written by hand: every count on the
generated pages is computed here from the sources.

Source references become GitHub links at the revision of the claim (lahdelinkit.py).
Nothing in the repository is changed. No network access.
"""
import collections
import csv
import hashlib
import html
import io
import json
import os
import posixpath
import re
import shutil
import subprocess
import sys
import tempfile
import unicodedata

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.dont_write_bytecode = True
import lahdelinkit  # noqa: E402
import portaali  # noqa: E402
import synteesi  # noqa: E402
import tietopohja as tp  # noqa: E402

DEFAULT_REPO = "https://github.com/PirilaTP/tulospalvelu"
PORTAL = "laatu/portaali/app/index.html"
PROGRAMS = ["HkKisaWin", "HkMaali", "ViestiWin", "ViestiMaali", "ViestiLuenta"]

# ------------------------------------------------------------------ site map ---
# Canonical documents published as pages: (repo path, site URI, navigation title).
ARCH = [
    ("README.md", "index.md", "Arkkitehtuuridokumentaatio"),
    ("yleiskuva.md", "yleiskuva.md", "Ohjelmiston yleiskuva"),
    ("inventaario.md", "inventaario.md", "Inventaario"),
    ("tietorakenteet-ja-tila.md", "tietorakenteet-ja-tila.md", "Tietorakenteet ja tila"),
    ("tiedostomuodot.md", "tiedostomuodot.md", "Tiedostomuodot"),
    ("tiedonsiirto.md", "tiedonsiirto.md", "Tiedonsiirtoprotokollat"),
    ("saikeet-ja-lukot.md", "saikeet-ja-lukot.md", "Säikeet ja lukot"),
    ("globaali-tila.md", "globaali-tila.md", "Globaali tila"),
    ("riippuvuudet.md", "riippuvuudet.md", "Riippuvuudet ja laitteet"),
    ("ominaisuusvalitsimet.md", "ominaisuusvalitsimet.md", "Asetukset ja käännösvalitsimet"),
    ("kaantajaerot.md", "kaantajaerot.md", "Kääntäjäerot"),
    ("merkistot.md", "merkistot.md", "Merkistöt"),
    ("dokumentointistandardi.md", "dokumentointistandardi.md", "Dokumentointistandardi"),
]
REPORTS = [
    ("auditoinnin-yhteenveto.md", "Auditoinnin yhteenveto"),
    ("tallenna-samanaikaisuus.md", "Tallennus ja rinnakkaisuus (C-1)"),
    ("muistiturvallisuus.md", "Muistiturvallisuus (C-4)"),
    ("gui-liiketoimintalogiikka.md", "GUI:n liiketoimintalogiikka (C-7)"),
    ("virheenkasittely.md", "Virheenkäsittely ja resurssit (C-9)"),
    ("testaus.md", "Testaus ja todennus (C-11)"),
    ("korjaussuunnitelma.md", "Korjaussuunnitelma"),
    ("regressiotestaussuunnitelma.md", "Regressiotestaussuunnitelma"),
    ("omistajan-paatokset.md", "Omistajan päätökset"),
    ("suunnitelma.md", "Auditointisuunnitelma"),
    ("seuranta.md", "Seuranta ja avoimet kysymykset"),
    ("c14-suunnitelma.md", "C-14: koko koodin katselmointi"),
    ("k-vaihe-suunnitelma.md", "Vaihe K: tietopohja"),
]
REGISTER_DOCS = [("README.md", "index.md", "Rekisterin rakenne"), ("havainnot.md", "havainnot.md", "Havaintorekisteri"),
                 ("moduulit.md", "moduulit.md", "Moduulikohtainen kattavuus")]
KB_DOCS = [("README.md", "kehittajat/tietopohja/index.md", "Tietopohjan rakenne ja ylläpito"),
           ("kirjoitusohje.md", "kehittajat/tietopohja/kirjoitusohje.md", "Korttien kirjoitusohje"),
           ("sanasto.md", "laatu/sanasto.md", "Sanasto")]

# Hand-written pages in docs-site/docs (their text has no counts; generated fragments are
# inserted at <!-- tp:NAME --> markers by the hook).
NAV = [
    {"Kehittäjille": [
        {"Aloitusohje": "kehittajat/index.md"},
        {"Ohjelmiston yleiskuva": "kehittajat/arkkitehtuuri/yleiskuva.md"},
        {"Kehitysympäristö": "kehittajat/kehitysymparisto.md"},
        {"Arkkitehtuuri": [{t: "kehittajat/arkkitehtuuri/" + u} for _, u, t in ARCH
                           if u not in ("yleiskuva.md", "tiedostomuodot.md", "tiedonsiirto.md", "ominaisuusvalitsimet.md")]},
        {"Ohjelmien erot ja yhteiset osat": "kehittajat/kasikirja/01-ohjelmat.md"},
        {"Toiminnallinen käsikirja": "@kasikirja"},
        {"Tietorakenteet ja tiedostomuodot": "kehittajat/arkkitehtuuri/tiedostomuodot.md"},
        {"Tiedonsiirtoprotokollat": "kehittajat/arkkitehtuuri/tiedonsiirto.md"},
        {"Laiteintegraatiot": "kehittajat/laitteet.md"},
        {"Asetukset ja käännösvalitsimet": [
            {"Yleiskuva": "kehittajat/arkkitehtuuri/ominaisuusvalitsimet.md"},
            {"Asetusavaimet": "kehittajat/asetukset/asetusavaimet.md"},
            {"Esikääntäjän valitsimet": "kehittajat/asetukset/valitsimet.md"}]},
        {"Tekoälyavusteinen kehitys": "kehittajat/tekoaly.md"},
        {"Tietopohjan ylläpito": [{t: u} for _, u, t in KB_DOCS if u.startswith("kehittajat/")] +
            [{"Auditointiin osallistuminen": "kehittajat/auditointi.md"}]},
        {"Kehittämiseen osallistuminen": "kehittajat/osallistuminen.md"},
    ]},
    {"Ohjelmiston laatu": [
        {"Yleiskuva ja nykytila": "laatu/index.md"},
        {"Auditointiraportit": [{"Raporttien hakemisto": "laatu/raportit/index.md"}] +
            [{t: "laatu/raportit/" + f} for f, t in REPORTS
             if f not in ("korjaussuunnitelma.md", "regressiotestaussuunnitelma.md", "omistajan-paatokset.md")]},
        {"Kattavuus ja rajoitteet": [{"Kattavuus": "laatu/kattavuus/index.md"},
                                     {"Tiedostoittain": "laatu/kattavuus/tiedostot.md"}]},
        {"Vikakortisto": "laatu/vikakortit/index.md"},
        {"Tunnetut ongelmat": "laatu/tunnetut-ongelmat.md"},
        {"Tekninen velka": "laatu/tekninen-velka.md"},
        {"Juurisyyt": "laatu/juurisyyt.md"},
        {"Vaikutukset kilpailutoimintaan": "laatu/vaikutukset.md"},
        {"Korjauskokonaisuudet": "laatu/korjauskokonaisuudet/index.md"},
        {"Korjaussuunnitelma": "laatu/raportit/korjaussuunnitelma.md"},
        {"Regressiotestaus": "laatu/raportit/regressiotestaussuunnitelma.md"},
        {"Avoimet kysymykset ja päätökset": [{"Kysymykset": "laatu/kysymykset.md"},
                                            {"Omistajan päätökset": "laatu/raportit/omistajan-paatokset.md"}]},
        {"Eteneminen": "laatu/eteneminen.md"},
        {"Rekisterit": [{t: "laatu/rekisteri/" + u} for _, u, t in REGISTER_DOCS]},
        {"Sanasto": "laatu/sanasto.md"},
        {"Interaktiivinen portaali": "laatu/portaali.md"},
    ]},
]

ID_RE = re.compile(r"(?<![\w/#.-])(H-\d{3}|T-\d{3}|KK-C\d+-\d+|OP-\d+|JS-\d+|Q\d{1,3})(?![\w-])")
HEADING_ID = re.compile(r"^(#{1,6})\s+\**(OP-\d+|KK-C\d+-\d+|JS-\d+|[HT]-\d{3}|Q\d{1,3})\b(.*)$")
LINK_MD = re.compile(r"(!?)\[((?:[^\[\]]|\[[^\]]*\])*)\]\(\s*<?([^()\s<>]+(?:\([^)\s]*\))?)>?(\s+\"[^\"]*\")?\s*\)")
CODE = re.compile(r"(`+)(.+?)\1")
URL_BARE = re.compile(r"(?<![(<\"'=])\b(https?://[^\s<>()\]]+[^\s<>()\].,;:!?'\"])")
LIST_ITEM = re.compile(r"^(\s*)([-*+]|\d+[.)])\s+")


# ---------------------------------------------------------------- slugs ---
def strip_inline(text):
    t = re.sub(r"\{[#.][^}]*\}\s*$", "", text)
    t = re.sub(r"!?\[([^\]]*)\]\([^)]*\)", r"\1", t)
    t = re.sub(r"<[^>]+>", "", t)
    t = t.replace("`", "").replace("**", "").replace("__", "")
    t = re.sub(r"(?<!\w)[*_](.+?)[*_](?!\w)", r"\1", t)
    return t.strip()


def slug_pm(text):
    """Python-Markdown toc default slugify (what MkDocs produces here)."""
    v = unicodedata.normalize("NFKD", html.unescape(strip_inline(text))).encode("ascii", "ignore").decode("ascii")
    v = re.sub(r"[^\w\s-]", "", v).strip().lower()
    return re.sub(r"[-\s]+", "-", v)


def slug_gh(text):
    """GitHub anchor of a heading (what the canonical documents were written against)."""
    v = strip_inline(text).lower()
    v = re.sub(r"[^\w\- ]", "", v, flags=re.U)
    return v.replace(" ", "-")


def heading_index(md):
    """Map GitHub-style anchors (and explicit {#id}) of a page to the anchors MkDocs makes."""
    out = {}
    seen_pm = collections.Counter()
    seen_gh = collections.Counter()
    seen_ids = set()
    fence = None
    for line in md.split("\n"):
        s = line.strip()
        if fence:
            if s.startswith(fence):
                fence = None
            continue
        if s.startswith("```") or s.startswith("~~~"):
            fence = s[:3]
            continue
        m = re.match(r"^(#{1,6})\s+(.*?)\s*#*\s*$", line)
        if not m:
            continue
        text = m.group(2)
        ex = re.search(r"\{#([^}\s]+)[^}]*\}\s*$", text)
        hm = HEADING_ID.match(line)
        if not ex and hm and hm.group(2) not in seen_ids:
            seen_ids.add(hm.group(2))
            ex = re.match(r"(.*)", "{#%s}" % hm.group(2))
            ex = re.search(r"\{#([^}\s]+)[^}]*\}", ex.group(1))
        pm = ex.group(1) if ex else slug_pm(text)
        if not ex:
            n = seen_pm[pm]
            seen_pm[pm] += 1
            if n:
                pm = "%s_%d" % (pm, n)
        gh = slug_gh(text)
        g = seen_gh[gh]
        seen_gh[gh] += 1
        if g:
            gh = "%s-%d" % (gh, g)
        out.setdefault(gh, pm)
        out.setdefault(pm, pm)
        if ex:
            out.setdefault(ex.group(1).lower(), pm)
    return out


# ------------------------------------------------------------ utilities ---
def rcsv(path):
    return tp.rcsv(path) if os.path.exists(path) else []


def cell(s):
    """Text for a Markdown table cell."""
    return str(s if s is not None else "").replace("\r", " ").replace("\n", " ").replace("|", "&#124;")


def mdlink(text, target):
    return "[%s](%s)" % (text, target)


def pct(a, b):
    return "%d %%" % round(100.0 * a / b) if b else "–"


def fmt_int(n):
    return "{:,}".format(n).replace(",", " ")


def read(path):
    with io.open(path, encoding="utf-8") as fh:
        return fh.read().replace("\r\n", "\n")


def front_title(md):
    m = re.search(r"^# (.+)$", md, re.M)
    return m.group(1).strip() if m else ""


def site_config(root):
    """site_url and repo_url from docs-site/mkdocs.yml (without a YAML library)."""
    cfg = {"site_url": "", "repo_url": DEFAULT_REPO}
    p = os.path.join(root, "docs-site", "mkdocs.yml")
    if os.path.exists(p):
        for line in read(p).split("\n"):
            m = re.match(r"^(site_url|repo_url):\s*(\S+)", line)
            if m:
                cfg[m.group(1)] = m.group(2)
    return cfg


# ------------------------------------------------------------------ site ---
class Site:
    def __init__(self, root, repo_url=None, site_url=None, cache_dir=None, kattavuus_dir=None):
        self.root = os.path.abspath(root)
        cfg = site_config(self.root)
        self.repo = (repo_url or cfg["repo_url"]).rstrip("/")
        self.site_url = site_url if site_url is not None else cfg["site_url"]
        self.data = tp.Data(self.root)
        self.doc = self.data.doc
        revs = collections.Counter(r["rev"] for r in self.data.reg.values())
        self.audit_rev = revs.most_common(1)[0][0] if revs else "HEAD"
        self.linker = lahdelinkit.Linker(self.root, self.repo, self.audit_rev)
        self.git = self.linker.git
        self.cache_dir = cache_dir
        self.kattavuus_dir = kattavuus_dir
        self.pages = collections.OrderedDict()   # uri -> markdown
        self.assets = collections.OrderedDict()  # uri -> bytes
        self.edit = {}                           # uri -> repo path of the canonical source
        self.src2uri = {}                        # repo path -> uri
        self.anchors = {}                        # uri -> heading index
        self.warnings = []
        self.boxes = {}                          # docs-site page path -> markdown appended
        self._register_sources()

    # ------------------------------------------------------- source mapping ---
    def _register_sources(self):
        d = "doc/arkkitehtuuri/"
        for f, u, _ in ARCH:
            self.src2uri[d + f] = "kehittajat/arkkitehtuuri/" + u
        for f, _ in REPORTS:
            self.src2uri[d + f] = "laatu/raportit/" + f
        for f, u, _ in REGISTER_DOCS:
            self.src2uri[d + "rekisteri/" + f] = "laatu/rekisteri/" + u
        for f, u, _ in KB_DOCS:
            self.src2uri["doc/tietopohja/" + f] = u
        hb = os.path.join(self.root, "doc", "tietopohja", "kasikirja")
        for f in sorted(os.listdir(hb)):
            if re.match(r"\d{2}-.*\.md$", f):
                self.src2uri["doc/tietopohja/kasikirja/" + f] = "kehittajat/kasikirja/" + (
                    "index.md" if f.startswith("00-") else f)
        ad = os.path.join(hb, "aineisto")
        if os.path.isdir(ad):
            for f in sorted(os.listdir(ad)):
                if f.endswith(".md"):
                    self.src2uri["doc/tietopohja/kasikirja/aineisto/" + f] = "kehittajat/kasikirja/aineisto/" + f
        for i in self.data.reg:
            self.src2uri["doc/tietopohja/vikakortit/%s/%s.md" % (i[0], i)] = "laatu/vikakortit/%s.md" % i
        self.src2uri["doc/tietopohja/vikakortisto.md"] = "laatu/vikakortit/index.md"
        self.src2uri["doc/tietopohja/portaali/index.html"] = "laatu/portaali.md"
        self._rows = None

    def id_target(self, ident):
        if re.match(r"[HT]-\d{3}$", ident):
            return "laatu/vikakortit/%s.md" % ident if ident in self.data.reg else None
        if ident.startswith("KK-"):
            return "laatu/korjauskokonaisuudet/%s.md" % ident if ident in self.data.packs else None
        if ident.startswith("OP-"):
            return "laatu/raportit/omistajan-paatokset.md#" + ident if ident in self.data.op else None
        if ident.startswith("JS-"):
            return "laatu/juurisyyt.md#" + ident if ident in self.data.js else None
        if ident.startswith("Q"):
            n = int(ident[1:])
            return "laatu/kysymykset.md#" + ident if n in self.data.qs else None
        return None

    def rel(self, page_uri, target):
        """Relative link from a page to a site URI (with optional #anchor)."""
        t, _, anchor = target.partition("#")
        r = posixpath.relpath(t, posixpath.dirname(page_uri) or ".")
        return r + ("#" + anchor if anchor else "")

    def gh(self, path, rev=None, kind="blob"):
        return "%s/%s/%s/%s" % (self.repo, kind, rev or self.linker.branch, path)

    # -------------------------------------------------- markdown transform ---
    def transform(self, md, page_uri, src_path=None, rev=None, autolink=True):
        """Canonical Markdown -> site Markdown (links, anchors, Python-Markdown layout)."""
        md = re.sub(r"<!--.*?-->\n?", "", md, flags=re.S)
        src_dir = posixpath.dirname(src_path) if src_path else None
        out = []
        fence = None
        prev = ""
        stack = []  # indents of open list markers
        item_para = False  # a blank-separated paragraph continues the current list item
        seen_head = set()
        for line in md.split("\n"):
            s = line.strip()
            if fence:
                out.append(line)
                if s.startswith(fence):
                    fence = None
                prev = line
                continue
            if s.startswith("```") or s.startswith("~~~"):
                fence = s[:3]
                if prev.strip() and not stack:
                    out.append("")
                out.append(re.sub(r"^\s+", " " * (4 * len(stack)), line) if stack else line)
                prev = line
                continue
            if not s:
                out.append("")
                prev = line
                continue
            m = LIST_ITEM.match(line)
            indent = len(line) - len(line.lstrip(" "))
            if m:
                while stack and stack[-1] > indent:
                    stack.pop()
                if stack and stack[-1] == indent:
                    depth = len(stack) - 1
                else:
                    if not stack and prev.strip() and not LIST_ITEM.match(prev):
                        out.append("")
                    stack.append(indent)
                    depth = len(stack) - 1
                if item_para and prev.strip():
                    out.append("")  # a paragraph inside the previous item would swallow this marker
                item_para = False
                line = " " * (4 * depth) + line.lstrip(" ")
            elif stack and indent > 0:
                if not prev.strip():
                    item_para = True
                depth = sum(1 for x in stack if x < indent)
                line = " " * (4 * max(depth, 1)) + line.lstrip(" ")
            else:
                if stack and not prev.strip():
                    stack = []
                if s.startswith("|") and prev.strip() and not prev.strip().startswith("|"):
                    out.append("")
                if s.startswith(">") and prev.strip() and not prev.strip().startswith(">"):
                    out.append("")
            hm = HEADING_ID.match(line)
            if line.lstrip().startswith("#"):
                stack = []
                if prev.strip():
                    out.append("")
                if hm and "{#" not in line and hm.group(2) not in seen_head:
                    seen_head.add(hm.group(2))
                    line = line.rstrip() + " {#%s}" % hm.group(2)
                out.append(self.inline(line, page_uri, src_dir, rev, autolink=False))
            else:
                out.append(self.inline(line, page_uri, src_dir, rev, autolink))
            prev = line
        return "\n".join(out)

    def link_target(self, target, page_uri, src_dir):
        if self.site_url and target.startswith(self.site_url):
            rest, _, anchor = target[len(self.site_url):].partition("#")
            docs = os.path.join(self.root, "docs-site", "docs")
            for cand in (rest + "index.md", rest.rstrip("/") + ".md", rest):
                if cand in self.pages or (cand.endswith(".md") and os.path.exists(os.path.join(docs, *cand.split("/")))):
                    return self.rel(page_uri, cand + ("#" + anchor if anchor else ""))
            return target
        if re.match(r"^[a-z]+:", target) or target.startswith("//"):
            return target
        path, _, anchor = target.partition("#")
        if not path:
            return target
        repo_path = posixpath.normpath(posixpath.join(src_dir or "", path)) if src_dir is not None else path
        if repo_path in self.src2uri:
            uri = self.src2uri[repo_path]
            if anchor:
                anchor = self.map_anchor(uri, anchor)
            return self.rel(page_uri, uri + ("#" + anchor if anchor else ""))
        m = re.match(r"^(.*?)(?:#L(\d+)(?:-L(\d+))?)?$", target)
        lines = anchor if anchor.startswith("L") else ""
        canon = self.git.canonical(self.linker.build, repo_path)
        if canon:
            return self.gh(canon) + ("#" + lines if lines else "")
        tree = [p for p in self.git.tree(self.linker.build) if p.startswith(repo_path.rstrip("/") + "/")]
        if tree:
            return self.gh(repo_path.rstrip("/"), kind="tree")
        self.warnings.append("%s: link target %s not in the repository" % (page_uri, target))
        return None

    def map_anchor(self, uri, anchor):
        if uri not in self.anchors:
            return anchor
        idx = self.anchors[uri]
        return idx.get(anchor) or idx.get(anchor.lower()) or anchor

    def inline(self, line, page_uri, src_dir, rev, autolink=True):
        keep = []

        def hold(s):
            keep.append(s)
            return "\x00%d\x00" % (len(keep) - 1)

        def rep_link(m):
            bang, text, target, title = m.group(1), m.group(2), m.group(3), m.group(4) or ""
            new = self.link_target(target, page_uri, src_dir)
            if new is None:
                return hold(text)
            return hold("%s[%s](%s%s)" % (bang, text, new, title))

        line = LINK_MD.sub(rep_link, line)
        line = re.sub(r"<(https?://[^>]+)>", lambda m: hold(m.group(0)), line)
        parts = []
        pos = 0
        last = None
        for m in CODE.finditer(line):
            parts.append(("t", line[pos:m.start()]))
            parts.append(("c", m.group(0), m.group(2)))
            pos = m.end()
        parts.append(("t", line[pos:]))
        out = []
        for p in parts:
            if p[0] == "c":
                full, inner = p[1], p[2]
                url = None
                path = inner.strip()
                doc_uri = self.doc_code_target(path, src_dir)
                if doc_uri:
                    url = self.rel(page_uri, doc_uri)
                else:
                    url, last = self.linker.code(inner, rev=rev, last=last, doc_dir=src_dir)
                out.append(hold("[%s](%s)" % (full, url)) if url else hold(full))
            else:
                if "|" in p[1]:
                    last = None  # a new table cell: a bare `:N` never continues the previous cell
                t = URL_BARE.sub(lambda m: hold("<%s>" % m.group(1)), p[1])
                if autolink:
                    t = ID_RE.sub(lambda m: self.id_link(m.group(1), page_uri, hold), t)
                out.append(t)
        res = "".join(out)
        for _ in range(3):
            res = re.sub("\x00(\\d+)\x00", lambda m: keep[int(m.group(1))], res)
        return res

    def id_link(self, ident, page_uri, hold):
        t = self.id_target(ident)
        if not t or t.split("#")[0] == page_uri and "#" not in t:
            return ident
        return hold("[%s](%s)" % (ident, self.rel(page_uri, t)))

    def doc_code_target(self, path, src_dir):
        """A code span naming a published document (e.g. `korjaussuunnitelma.md`)."""
        if not path.endswith(".md") or " " in path:
            return None
        cands = [path]
        if src_dir:
            cands.append(posixpath.normpath(posixpath.join(src_dir, path)))
        cands += ["doc/arkkitehtuuri/" + path, "doc/tietopohja/" + path]
        for c in cands:
            if c in self.src2uri:
                return self.src2uri[c]
        return None

    # ---------------------------------------------------------- page helpers ---
    def add(self, uri, md, edit=None):
        self.pages[uri] = md
        if edit:
            self.edit[uri] = edit

    def source_note(self, src):
        return ('!!! abstract "Kanoninen lähde"\n    Tämä sivu on generoitu tiedostosta [`%s`](%s). '
                "Muutokset tehdään lähteeseen, ei sivustoon. Lähdekoodiviitteet on linkitetty "
                "revisioon, jota vastaan väite on tehty.\n\n" % (src, self.gh(src)))

    def publish_doc(self, src, uri, rev=None, note=True):
        md = read(os.path.join(self.root, src))
        self.anchors[uri] = heading_index(md)
        return src, uri, md, rev, note

    # ---------------------------------------------------------------- build ---
    def generate(self):
        docs = []
        d = "doc/arkkitehtuuri/"
        for f, u, _ in ARCH:
            docs.append(self.publish_doc(d + f, "kehittajat/arkkitehtuuri/" + u))
        for f, _ in REPORTS:
            docs.append(self.publish_doc(d + f, "laatu/raportit/" + f))
        for f, u, _ in REGISTER_DOCS:
            docs.append(self.publish_doc(d + "rekisteri/" + f, "laatu/rekisteri/" + u))
        for f, u, _ in KB_DOCS:
            docs.append(self.publish_doc("doc/tietopohja/" + f, u))
        for src, uri in sorted(self.src2uri.items()):
            if src.startswith("doc/tietopohja/kasikirja/"):
                docs.append(self.publish_doc(src, uri))
        for src, uri, md, rev, note in docs:
            body = self.transform(md, uri, src, rev)
            body = re.sub(r"^(# .*\n)", lambda m: m.group(1) + "\n" + (self.source_note(src) if note else ""),
                          body, count=1, flags=re.M)
            self.add(uri, body, src)
        self.page_cards()
        self.page_cards_index()
        self.page_known()
        self.page_tdebt()
        self.page_roots()
        self.page_impacts()
        self.page_packs()
        self.page_questions()
        self.page_progress()
        self.page_reports()
        self.page_coverage()
        self.page_settings()
        self.page_quality_index()
        self.page_portal()
        self.manual_boxes()
        self.linker.close()
        return self

    # ----------------------------------------------------------- vikakortit ---
    def card_sections(self, ident):
        p = self.data.path(ident)
        if not os.path.exists(p):
            return None
        return read(p)

    def status_line(self, ident):
        sr = self.data.state_row(ident)
        return sr.get("tila", "?")

    def page_cards(self):
        for ident in sorted(self.data.reg):
            uri = "laatu/vikakortit/%s.md" % ident
            r = self.data.reg[ident]
            text = self.card_sections(ident)
            sr = self.data.state_row(ident)
            src = "doc/tietopohja/vikakortit/%s/%s.md" % (ident[0], ident)
            if text and "<selkokielinen otsikko" not in text:
                self.anchors[uri] = heading_index(text)
                body = self.transform(text, uri, src, r["rev"])
                head, _, rest = body.partition("\n")
                notes = []
            else:
                head = "# %s – %s" % (ident, r["otsikko"])
                rest = ("\n> **%s** · tila: **%s**\n\nTälle tunnukselle ei ole vielä kirjoitettu selkokielistä "
                        "korttia. Rekisterin kuvaus: %s\n" % (ident, sr.get("tila", "?"), r["otsikko"]))
                notes = ["Kortti puuttuu: sivu on muodostettu rekisteririvistä."]
                src = None
            tila = sr.get("tila", "?")
            foot = ["", "---", "", '<div class="tp-pysyva" markdown>', "",
                    "**Pysyvä osoite:** `%slaatu/vikakortit/%s/`" % (self.site_url, ident) +
                    " · [Portaalissa](%s)" % self.rel(uri, PORTAL) + "#/kortti/" + ident +
                    " · Rekisteri: [havaintorekisteri](%s)" % self.rel(uri, "laatu/rekisteri/havainnot.md") +
                    " (revisio `%s`)" % r["rev"], ""]
            if tila == "korjattu" and sr.get("korjaus"):
                foot.append("**Korjattu:** %s" % self.fix_links(sr["korjaus"], uri))
                foot.append("")
            if src:
                foot.append("Kortin lähde: [`%s`](%s). Kortin generoidut lohkot tuotetaan rekistereistä "
                            "(`tietopohja.py lohkot`); uusi tieto lisätään rekisteriin, ei korttiin." % (src, self.gh(src)))
            foot += notes + ["", "</div>", ""]
            if tila == "kumottu":
                banner = '!!! success "Kumottu"\n    Epäily osoittautui aiheettomaksi. Perustelu alla.\n'
            elif tila == "korjattu":
                banner = '!!! success "Korjattu"\n    Vika on korjattu: %s\n' % self.fix_links(sr.get("korjaus", ""), uri)
            else:
                banner = ""
            self.add(uri, head + "\n\n" + banner + rest + "\n".join(foot), src)

    def fix_links(self, text, uri):
        """Commit hashes and version tags in the `korjaus` column as GitHub links."""
        t = re.sub(r"\b([0-9a-f]{7,40})\b", lambda m: "[`%s`](%s/commit/%s)" % (m.group(1), self.repo, m.group(1)), text)
        t = re.sub(r"\b(v\d{4}\.\d{2}\.\d+)\b", lambda m: "[%s](%s/releases/tag/%s)" % (m.group(1), self.repo, m.group(1)), t)
        return t

    def rows(self):
        """Findings with their derived fields, sorted by id (cached)."""
        if self._rows is not None:
            return self._rows
        out = []
        for i in sorted(self.data.reg, key=lambda x: (x[0], x)):
            r = self.data.reg[i]
            sr = self.data.state_row(i)
            rk = self.data.risk.get(i, {})
            out.append({"id": i, "laji": i[0], "otsikko": self.card_title(i), "tila": sr.get("tila", "?"),
                        "p": rk.get("prioriteetti", "-") if i[0] == "H" else "-",
                        "vakavuus": rk.get("tekninen_vakavuus") or r["vakavuus"],
                        "todennus": rk.get("todennus", "-"), "ohjelmat": sr.get("ohjelmat", "").split(),
                        "juurisyy": sr.get("juurisyy", ""), "kk": self.data.primary.get(i, "-"), "era": r["era"],
                        "saavutettavuus": rk.get("saavutettavuus", "-")})
        self._rows = out
        return out

    def card_title(self, ident):
        p = self.data.path(ident)
        if os.path.exists(p):
            first = read(p).split("\n", 1)[0]
            if first.startswith("# ") and "<selkokielinen" not in first:
                return first[2:].split(" – ", 1)[-1].strip()
        return self.data.reg[ident]["otsikko"]

    def card_heading_anchor(self, ident, prefix):
        idx = self.anchors.get("laatu/vikakortit/%s.md" % ident, {})
        for gh, pm in idx.items():
            if re.match(r"^\d*-?" + re.escape(slug_pm(prefix)), pm):
                return pm
        return None

    def link_id(self, ident, page_uri):
        t = self.id_target(ident)
        return "[%s](%s)" % (ident, self.rel(page_uri, t)) if t else ident

    def find_table(self, rows, uri, cols=("id", "otsikko", "p", "ohjelmat", "todennus", "kk")):
        names = {"id": "Tunnus", "otsikko": "Kuvaus", "p": "P", "ohjelmat": "Ohjelmat", "todennus": "Todennus",
                 "kk": "Kokonaisuus", "tila": "Tila", "vakavuus": "Vakavuus", "juurisyy": "Juurisyy", "era": "Erä"}
        L = ["| " + " | ".join(names[c] for c in cols) + " |", "|" + "---|" * len(cols)]
        for r in rows:
            vals = []
            for c in cols:
                v = r[c]
                if c == "id":
                    v = self.link_id(v, uri)
                elif c == "kk":
                    v = self.link_id(v, uri) if v != "-" else "–"
                elif c == "juurisyy":
                    v = self.link_id(v, uri) if v.startswith("JS-") else v
                elif c == "ohjelmat":
                    v = ", ".join(v)
                elif c == "todennus":
                    v = tp.TODENNUS.get(v, v)
                else:
                    v = cell(v)
                vals.append(v)
            L.append("| " + " | ".join(vals) + " |")
        return L

    def page_cards_index(self):
        uri = "laatu/vikakortit/index.md"
        rows = self.rows()
        H = [r for r in rows if r["laji"] == "H"]
        T = [r for r in rows if r["laji"] == "T"]
        st = collections.Counter(r["tila"] for r in rows)
        written = sum(1 for r in rows if os.path.exists(self.data.path(r["id"])))
        L = ["# Vikakortisto", "", self.coverage_warning(uri), "",
             "Jokaisella auditointihavainnolla (H) ja teknisen velan rivillä (T) on pysyvä sivu osoitteessa "
             "`laatu/vikakortit/<tunnus>/`. Kortit ovat rekisterin selkokielisiä johdannaisia; rekisteri on ensisijainen "
             "lähde. Suodatus ja haku: [interaktiivinen portaali](%s)." % self.rel(uri, "laatu/portaali.md"), "",
             "| Laji | Rivejä | Kortti kirjoitettu |", "|---|--:|--:|",
             "| Havainnot (H) | %d | %d |" % (len(H), sum(1 for r in H if os.path.exists(self.data.path(r["id"])))),
             "| Tekninen velka (T) | %d | %d |" % (len(T), sum(1 for r in T if os.path.exists(self.data.path(r["id"])))),
             "| Yhteensä | %d | %d |" % (len(rows), written), "",
             "Tila: " + ", ".join("%s %d" % (k, v) for k, v in sorted(st.items())) + ".", ""]
        for p in ["P1", "P2", "P3", "P4", "P5"]:
            sel = [r for r in H if r["tila"] == "avoin" and r["p"] == p]
            L += ["## Avoimet havainnot, %s {#%s}" % (p, p.lower()), "", "%d havaintoa." % len(sel), ""]
            L += self.find_table(sel, uri) + [""]
        other = [r for r in H if r["tila"] != "avoin"]
        if other:
            L += ["## Korjatut ja kumotut havainnot {#muut}", ""]
            L += self.find_table(other, uri, ("id", "otsikko", "tila", "kk")) + [""]
        L += ["## Tekninen velka {#tekninen-velka}", "", "Erillinen näkymä: [Tekninen velka](%s)." % self.rel(uri, "laatu/tekninen-velka.md"), ""]
        L += self.find_table(T, uri, ("id", "otsikko", "tila", "juurisyy", "kk")) + [""]
        L += self.generated_note()
        self.add(uri, "\n".join(L))

    def generated_note(self):
        return ["", "---", "", "*Sivu on generoitu rekistereistä (`doc/arkkitehtuuri/tyokalut/sivusto.py`). "
                "Luvut lasketaan rekistereistä jokaisella sivuston rakennuskerralla.*", ""]

    def coverage_warning(self, uri):
        lv = self.file_levels()
        tot = sum(v["rivit"] for v in lv.values())
        l1 = lv.get("1", {}).get("rivit", 0)
        prod4 = self.product_full_files()
        c = self.coverage_summary()
        return ('!!! warning "Kattavuusrajoite"\n    Auditointi ei kata koko lähdekoodia. Tuotantokoodin '
                "funktioriveistä %s on sellaisia, joihin ei ole viitattu yhdessäkään auditoinnin asiakirjassa, ja "
                "%s kaikista lähdekoodiriveistä on tiedostoissa, jotka on vain inventoitu. Viitattukaan koodi ei ole "
                "katselmoitu: kokonaan katselmoituja tuotantotiedostoja on %d ja C-14:n tasolle K4 kirjattuja "
                "koodialueita %d. Havaintojen määrä on alaraja; kortin puuttuminen ei tarkoita, että toiminto on "
                "virheetön. [Kattavuus](%s)" % (pct(c["ei_r"], c["rivit"]), pct(l1, tot), prod4, c["k4"],
                                               self.rel(uri, "laatu/kattavuus/index.md")))

    def coverage_summary(self):
        """Function-level figures used by the coverage banner (kattavuus.py, C-14 registers)."""
        if not hasattr(self, "_covsum"):
            funcs = rcsv(os.path.join(self.coverage_run(), "funktiot.csv"))
            prod = [f for f in funcs if f["tiedostoluokka"] == "tuote" and f["aktiivinen"].strip()]
            best, stale = self.review_state()
            self._covsum = {"rivit": sum(int(f["rivit"] or 0) for f in prod),
                            "ei_r": sum(int(f["rivit"] or 0) for f in prod if f["luokka"] == "ei_viitteita"),
                            "k4": sum(1 for v in best.values() if v >= "K4")}
        return self._covsum

    def review_state(self):
        """C-14: valid level per code area and the stale areas (c14-suunnitelma.md 3.4).
        The valid level is the highest level among reviews whose hash equals the current hash;
        an area is stale when it has reviews but none at the current hash."""
        reg = os.path.join(self.doc, "rekisteri")
        cur = {a["alue"]: a for a in rcsv(os.path.join(reg, "koodialueet.csv"))}
        best, seen = {}, set()
        for r in rcsv(os.path.join(reg, "katselmoinnit.csv")):
            a = cur.get(r.get("alue"))
            if not a:
                continue
            seen.add(a["alue"])
            if a.get("tiiviste") and r.get("tiiviste") and a["tiiviste"] != r["tiiviste"]:
                continue
            lvl = r.get("syvyys", "K0")
            if lvl > best.get(a["alue"], "K0"):
                best[a["alue"]] = lvl
        return best, seen - set(best)

    def file_levels(self):
        if not hasattr(self, "_levels"):
            lv = {}
            for r in rcsv(os.path.join(self.doc, "rekisteri", "tiedostot.csv")):
                d = lv.setdefault(r["analyysitaso"], {"tiedostot": 0, "rivit": 0})
                d["tiedostot"] += 1
                d["rivit"] += int(r["rivit"] or 0)
            self._levels = lv
        return self._levels

    def product_full_files(self):
        return sum(1 for r in rcsv(os.path.join(self.doc, "rekisteri", "tiedostot.csv"))
                   if r["analyysitaso"] == "4" and not r["polku"].startswith(("Tests/", "VS/Tests/")))

    # ------------------------------------------------------- tunnetut ongelmat ---
    def page_known(self):
        uri = "laatu/tunnetut-ongelmat.md"
        rows = [r for r in self.rows() if r["laji"] == "H" and r["tila"] == "avoin"]
        imp = self.data.imp
        VAL = {"vahvistettu": "vahvistettu", "mahdollinen": "mahdollinen", "ei_perusteltu": "ei",
               "ei_tiedossa": "ei tiedossa", "ei_sovellu": "–", None: "ei arvioitu"}
        L = ["# Tunnetut ongelmat", "", self.coverage_warning(uri), "",
             "Tällä sivulla ovat auditoinnin avoimet havainnot ohjelmittain kilpailun järjestäjän näkökulmasta. "
             "Ne ovat **auditointihavaintoja**: suurinta osaa ei ole todennettu ajamalla ohjelmaa (sarake "
             "*Todennus*). Prioriteetti P1 on kiireellisin. Jokaisen kortin luvussa \"Mitä käyttäjä voi tehdä "
             "riskin pienentämiseksi\" kerrotaan, onko keino todennettu vai perusteltu mutta testaamaton.", "",
             "Sarakkeet *Normaalikäytössä* ja *Kiertotapa* tulevat rekisteristä `vaikutukset.csv` "
             "(ulottuvuudet `normaalikaytto` ja `kiertotapa`).", ""]
        for prog in PROGRAMS:
            sel = sorted([r for r in rows if prog in r["ohjelmat"]], key=lambda r: (r["p"], r["id"]))
            L += ["## %s" % prog, "", "%d avointa havaintoa; alla P1–P3 (%d), muut [vikakortistossa](%s)." % (
                len(sel), sum(1 for r in sel if r["p"] in ("P1", "P2", "P3")), self.rel(uri, "laatu/vikakortit/index.md")), "",
                "| Tunnus | Kuvaus | P | Normaalikäytössä | Kiertotapa | Todennus |", "|---|---|---|---|---|---|"]
            for r in sel:
                if r["p"] not in ("P1", "P2", "P3"):
                    continue
                n = imp.get(r["id"], {}).get("normaalikaytto", {}).get("arvo")
                k = imp.get(r["id"], {}).get("kiertotapa", {}).get("arvo")
                anchor = self.card_heading_anchor(r["id"], "Mitä käyttäjä voi tehdä")
                kt = VAL.get(k, k)
                if anchor and k in ("vahvistettu", "mahdollinen"):
                    kt = "[%s](%s)" % (kt, self.rel(uri, "laatu/vikakortit/%s.md#%s" % (r["id"], anchor)))
                L.append("| %s | %s | %s | %s | %s | %s |" % (self.link_id(r["id"], uri), cell(r["otsikko"]), r["p"],
                                                                VAL.get(n, n), kt, tp.TODENNUS.get(r["todennus"], r["todennus"])))
            L.append("")
        L += self.generated_note()
        self.add(uri, "\n".join(L))

    def page_tdebt(self):
        uri = "laatu/tekninen-velka.md"
        T = [r for r in self.rows() if r["laji"] == "T"]
        by = collections.defaultdict(list)
        for r in T:
            by[r["juurisyy"] or "-"].append(r)
        L = ["# Tekninen velka", "", "Teknisen velan rivit (T) eivät ole yksittäisiä vikoja vaan rakenteita, jotka tekevät "
             "vioista todennäköisempiä tai korjaamisesta vaikeampaa. Ryhmittely juurisyyn mukaan (`tekninen-velka.csv`).", "",
             "%d riviä; tila: %s." % (len(T), ", ".join("%s %d" % kv for kv in sorted(collections.Counter(r["tila"] for r in T).items()))), ""]
        for js in sorted(by, key=lambda x: (not x.startswith("JS-"), int(x[3:]) if x.startswith("JS-") else 0, x)):
            L += ["## %s" % (self.data.js_text(js) if js != "-" else "Ei juurisyytä"), ""]
            L += self.find_table(by[js], uri, ("id", "otsikko", "tila", "ohjelmat", "kk")) + [""]
        L += self.generated_note()
        self.add(uri, "\n".join(L))

    def page_roots(self):
        uri = "laatu/juurisyyt.md"
        summ = synteesi.read(os.path.join(self.doc, "auditoinnin-yhteenveto.md"))
        mech = {}
        for m in re.finditer(r"\| \d+ \| \*\*(JS-\d+)\*\* ([^|]+?) \| ([^|]+) \|", summ):
            mech[m.group(1)] = (m.group(2).strip(), m.group(3).strip())
        rows = self.rows()
        L = ["# Juurisyyt", "", "Juurisyyt JS-1–JS-%d ovat pysyviä tunnuksia (lähde: [auditoinnin yhteenveto](%s), "
             "jäsenyys `riskiarvio.csv` ja `tekninen-velka.csv`, sarakkeet `juurisyy` ja `juurisyy_muut`)." % (
                 len(self.data.js), self.rel(uri, "laatu/raportit/auditoinnin-yhteenveto.md")), "",
             "| Juurisyy | Ensisijaisia | Pahentavana | Avoimia P1–P2 |", "|---|--:|--:|--:|"]
        keys = sorted(self.data.js, key=lambda j: int(j[3:]))
        for js in keys:
            prim = [r for r in rows if r["juurisyy"] == js]
            worse = [i for i in self.data.reg if js in self.data.state_row(i).get("juurisyy_muut", "").split()]
            hot = sum(1 for r in prim if r["tila"] == "avoin" and r["p"] in ("P1", "P2"))
            L.append("| [%s](#%s) %s | %d | %d | %d |" % (js, js, cell(self.data.js[js]), len(prim), len(worse), hot))
        single = [r for r in rows if r["juurisyy"] == "yksittainen"]
        L += ["", "Yksittäisiä vikoja ilman yhteistä juurisyytä: %d." % len(single), ""]
        for js in keys:
            name, mtext = mech.get(js, (self.data.js[js], ""))
            prim = [r for r in rows if r["juurisyy"] == js]
            worse = sorted(i for i in self.data.reg if js in self.data.state_row(i).get("juurisyy_muut", "").split())
            L += ["## %s %s {#%s}" % (js, name.split(" / ")[0], js), "", self.transform(mtext, uri), "",
                  "**Ensisijainen juurisyy (%d):**" % len(prim), ""]
            L += self.find_table(prim, uri, ("id", "otsikko", "tila", "p", "kk")) + [""]
            if worse:
                L += ["**Pahentava juurisyy (%d):** " % len(worse) + ", ".join(self.link_id(i, uri) for i in worse), ""]
        L += self.generated_note()
        self.add(uri, "\n".join(L))

    def page_impacts(self):
        uri = "laatu/vaikutukset.md"
        open_h = [i for i in self.data.reg if i[0] == "H" and self.data.state_row(i).get("tila") == "avoin"]
        vals = ["vahvistettu", "mahdollinen", "ei_perusteltu", "ei_tiedossa", "ei_sovellu", None]
        names = {"vahvistettu": "Vahvistettu", "mahdollinen": "Mahdollinen", "ei_perusteltu": "Ei (perusteltu)",
                 "ei_tiedossa": "Ei tiedossa", "ei_sovellu": "Ei sovellu", None: "Ei arvioitu"}
        L = ["# Vaikutukset kilpailutoimintaan", "",
             "Käytännön vaikutukset on arvioitu jokaiselle havainnolle %d ulottuvuudella (`vaikutusulottuvuudet.csv`, "
             "`vaikutukset.csv`). Taulukossa ovat avoimet havainnot (%d). **Vahvistettu** = seuraus on osoitettu kokeella, "
             "testikehyksessä tai ajamalla; *Mahdollinen* = seuraa koodista, mutta seurausta ei ole osoitettu; "
             "*Ei arvioitu* ei tarkoita, ettei vaikutusta ole." % (len(self.data.dims), len(open_h)), "",
             "| Ryhmä | Kysymys | " + " | ".join(names[v] for v in vals) + " |", "|---|---|" + "--:|" * len(vals)]
        for d in self.data.dims:
            c = collections.Counter(self.data.imp.get(i, {}).get(d["ulottuvuus"], {}).get("arvo") for i in open_h)
            L.append("| %s | [%s](#%s) | %s |" % (d["ryhma"], cell(d["nimi"]), d["ulottuvuus"].replace("_", "-"),
                                                  " | ".join(str(c.get(v, 0)) for v in vals)))
        L.append("")
        for d in self.data.dims:
            L += ["## %s {#%s}" % (d["nimi"], d["ulottuvuus"].replace("_", "-")), "", "*%s*" % d["kysymys"], ""]
            for v in ("vahvistettu", "mahdollinen"):
                ids = sorted(i for i in open_h if self.data.imp.get(i, {}).get(d["ulottuvuus"], {}).get("arvo") == v)
                if ids:
                    L += ["**%s (%d):** " % (names[v], len(ids)) + ", ".join(self.link_id(i, uri) for i in ids), ""]
        L += self.generated_note()
        self.add(uri, "\n".join(L))

    # ---------------------------------------------------- korjauskokonaisuudet ---
    def page_packs(self):
        uri = "laatu/korjauskokonaisuudet/index.md"
        packs = [p for p in self.data.packs.values()]
        L = ["# Korjauskokonaisuudet", "", "Havainnot on ryhmitelty %d korjauskokonaisuudeksi (`korjauskokonaisuudet.csv`, "
             "[korjaussuunnitelma](%s)). Kokonaisuudet ovat **suunniteltuja**; toteutus alkaa vasta D-vaiheessa. "
             "Jokaisella kokonaisuudella on pysyvä sivu `laatu/korjauskokonaisuudet/<tunnus>/`." % (
                 sum(1 for p in packs if p["kk"].startswith("KK-")), self.rel(uri, "laatu/raportit/korjaussuunnitelma.md")), "",
             "| Kokonaisuus | Nimi | P | Aalto | Vaihe | Jäseniä | Avoimia | Riippuu | Omistajan päätökset |", "|---|---|---|---|---|--:|--:|---|---|"]
        for p in sorted(packs, key=lambda p: (p["aalto"] or "9", p["kk"])):
            mem = (p["havainnot"] + " " + p["tekninen_velka"]).split()
            op = sum(1 for i in mem if self.data.state_row(i).get("tila") == "avoin")
            L.append("| %s | %s | %s | %s | %s | %d | %d | %s | %s |" % (
                self.link_id(p["kk"], uri), cell(p["nimi"]), p["prioriteetti"], p["aalto"], p["vaihe"], len(mem), op,
                " ".join(self.link_id(x, uri) for x in p["riippuu"].split()) or "–",
                " ".join(self.link_id(x, uri) for x in p["omistajan_paatokset"].split()) or "–"))
        L += ["", "Riippuvuuskaavio on [portaalissa](%s)." % (self.rel(uri, PORTAL) + "#/kokonaisuudet")]
        L += self.generated_note()
        self.add(uri, "\n".join(L))
        plan = os.path.join(self.doc, "korjaussuunnitelma.md")
        plan_idx = heading_index(read(plan))
        plan_heads = [(h, a) for h, a in plan_idx.items()]
        for p in packs:
            if not p["kk"].startswith("KK-"):
                continue
            u = "laatu/korjauskokonaisuudet/%s.md" % p["kk"]
            mem = [i for i in (p["havainnot"] + " " + p["tekninen_velka"]).split() if i in self.data.reg]
            rows = [r for r in self.rows() if r["id"] in mem]
            sec = next((a for h, a in plan_heads if p["kk"].lower() in h), None)
            L = ["# %s %s" % (p["kk"], p["nimi"]), "",
                 "| Kenttä | Arvo |", "|---|---|",
                 "| Tyyppi | %s |" % cell(p["tyyppi"]), "| Prioriteetti | %s (jakauma %s) |" % (p["prioriteetti"], cell(p["prioriteettijakauma"])),
                 "| Vaihe ja aalto | vaihe %s, aalto %s |" % (p["vaihe"], p["aalto"]),
                 "| Juurisyy | %s |" % (" ".join(self.link_id(j, u) for j in p["juurisyy"].split()) or "–"),
                 "| Riippuu | %s |" % (" ".join(self.link_id(x, u) for x in p["riippuu"].split()) or "–"),
                 "| Riippuu osittain | %s |" % (" ".join(self.link_id(x, u) for x in p["riippuu_osat"].split()) or "–"),
                 "| Rinnakkain | %s |" % (" ".join(self.link_id(x, u) for x in p["rinnakkain"].split()) or "–"),
                 "| Työmäärä / muutosriski | %s / %s |" % (cell(p["tyomaara"]), cell(p["muutosriski"])),
                 "| Yhteensopivuus (tiedosto, laite, protokolla) | %s; %s; %s |" % (
                     cell(p["yhteensopivuus_tiedosto"]), cell(p["yhteensopivuus_laite"]), cell(p["yhteensopivuus_protokolla"])),
                 "| Testit | %s |" % cell(p["testit"]),
                 "| Omistajan päätökset | %s |" % (" ".join(self.link_id(x, u) for x in p["omistajan_paatokset"].split()) or "–"),
                 "| Kysymykset | %s |" % (" ".join(self.link_id(x, u) for x in p["kysymykset"].split()) or "–"),
                 "| Suunniteltu haara | `%s` |" % p["haara"], "| Palautettavuus | %s |" % cell(p["palautettavuus"]), "",
                 "Kokonaisuus on suunniteltu, ei toteutettu, ellei jäsenen tila ole *korjattu*." +
                 (" Korjaussuunnitelmassa: [%s](%s)." % (p["kk"], self.rel(u, "laatu/raportit/korjaussuunnitelma.md#" + sec)) if sec else ""),
                 "", "## Jäsenet", ""]
            L += self.find_table(rows, u, ("id", "otsikko", "tila", "p", "ohjelmat")) + [""]
            L += self.generated_note()
            self.add(u, "\n".join(L))

    # ------------------------------------------------------------- kysymykset ---
    def page_questions(self):
        uri = "laatu/kysymykset.md"
        qs = self.data.qs
        st = collections.Counter(q["tila"] or "?" for q in qs.values())
        L = ["# Avoimet kysymykset ja omistajan päätökset", "",
             "Kysymykset Q1–Q%d ovat lähtöisin [seurannasta](%s); tila on luettu koneellisesti. Omistajan päätökset "
             "OP-1–OP-%d: [omistajan päätökset](%s). Päätöstä ei merkitä tehdyksi ilman omistajan hyväksyntää." % (
                 max(qs) if qs else 0, self.rel(uri, "laatu/raportit/seuranta.md"), len(self.data.op),
                 self.rel(uri, "laatu/raportit/omistajan-paatokset.md")), "",
             "Kysymysten tila: " + ", ".join("%s %d" % kv for kv in sorted(st.items())) + ".", "",
             "## Omistajan päätökset", "", "| Päätös | Aihe | Tila | Kokonaisuudet |", "|---|---|---|---|"]
        for o, d in sorted(self.data.op.items(), key=lambda kv: int(kv[0][3:])):
            packs = [p["kk"] for p in self.data.packs.values() if o in p["omistajan_paatokset"].split()]
            L.append("| %s | %s | %s | %s |" % (self.link_id(o, uri), cell(d["nimi"]), cell(d["tila"]),
                                                " ".join(self.link_id(k, uri) for k in packs) or "–"))
        L += ["", "## Kysymykset", "", "| Kysymys | Tila | Kuvaus |", "|---|---|---|"]
        for n, q in sorted(qs.items()):
            L.append('| <span id="Q%d"></span>Q%d | %s | %s |' % (n, n, cell(q["tila"] or "?"),
                                                                 self.transform(cell(q["kuvaus"]), uri)))
        L += self.generated_note()
        self.add(uri, "\n".join(L))

    # -------------------------------------------------------------- eteneminen ---
    def phases(self):
        return rcsv(os.path.join(self.doc, "rekisteri", "vaiheet.csv"))

    def page_progress(self):
        uri = "laatu/eteneminen.md"
        L = ["# Kehityksen ja korjausten eteneminen", "", "## Työvaiheet", "",
             "Lähde: `rekisteri/vaiheet.csv` (vaiheiden tila päivitetään sinne, kun vaihe etenee).", "",
             "| Vaihe | Sisältö | Tila | Päivitetty | Asiakirja |", "|---|---|---|---|---|"]
        for r in self.phases():
            doc = r["asiakirja"]
            u = self.src2uri.get("doc/arkkitehtuuri/" + doc) if doc else None
            L.append("| %s | %s | %s | %s | %s |" % (r["vaihe"], cell(r["nimi"]), cell(r["tila"]), r["pvm"],
                                                     mdlink(doc, self.rel(uri, u)) if u else cell(doc)))
        rows = self.rows()
        st = collections.Counter(r["tila"] for r in rows)
        L += ["", "## Havaintojen tila", "", "| Tila | H | T |", "|---|--:|--:|"]
        for k in sorted(st):
            L.append("| %s | %d | %d |" % (k, sum(1 for r in rows if r["tila"] == k and r["laji"] == "H"),
                                          sum(1 for r in rows if r["tila"] == k and r["laji"] == "T")))
        L += ["", "## Korjauskokonaisuudet aalloittain", "",
              "| Aalto | Kokonaisuuksia | Jäseniä | Avoimia | Korjattuja | Muita |", "|---|--:|--:|--:|--:|--:|"]
        waves = collections.defaultdict(list)
        for p in self.data.packs.values():
            if p["kk"].startswith("KK-"):
                waves[p["aalto"]].append(p)
        for w in sorted(waves):
            mem = [i for p in waves[w] for i in (p["havainnot"] + " " + p["tekninen_velka"]).split()]
            c = collections.Counter(self.data.state_row(i).get("tila") for i in mem)
            L.append("| %s | %d | %d | %d | %d | %d |" % (w, len(waves[w]), len(mem), c.get("avoin", 0), c.get("korjattu", 0),
                                                          len(mem) - c.get("avoin", 0) - c.get("korjattu", 0)))
        fixed = [r for r in rows if r["tila"] == "korjattu"]
        L += ["", "## Korjatut havainnot", ""]
        if fixed:
            L += ["| Tunnus | Kuvaus | Korjaus (commit, versio, testi) |", "|---|---|---|"]
            for r in fixed:
                L.append("| %s | %s | %s |" % (self.link_id(r["id"], uri), cell(r["otsikko"]),
                                               self.fix_links(cell(self.data.state_row(r["id"]).get("korjaus", "")), uri)))
        else:
            L.append("Yhtään havaintoa ei ole vielä merkitty korjatuksi (sarake `tila` = `korjattu` "
                     "rekistereissä `riskiarvio.csv` ja `tekninen-velka.csv`). Korjauksen yhteydessä sarakkeeseen "
                     "`korjaus` kirjataan commit, ohjelmaversio ja testi; ne näkyvät tässä ja kortilla automaattisesti.")
        L += ["", "Koodin katselmoinnin (C-14) eteneminen: [Kattavuus](%s#c-14)." % self.rel(uri, "laatu/kattavuus/index.md")]
        L += self.generated_note()
        self.add(uri, "\n".join(L))

    def page_reports(self):
        uri = "laatu/raportit/index.md"
        eras = collections.Counter(r["era"] for r in self.data.reg.values())
        L = ["# Auditointiraportit", "", "Jokainen auditointierä tuotti raportin. Havaintojen määrä erittäin lasketaan "
             "rekisterin sarakkeesta *Erä*; havainto voi myöhemmin olla laajentunut tai kumoutunut.", "",
             "| Vaihe | Sisältö | Tila | Raportti | Havaintoja (H+T) |", "|---|---|---|---|--:|"]
        for r in self.phases():
            doc = r["asiakirja"]
            u = self.src2uri.get("doc/arkkitehtuuri/" + doc) if doc else None
            L.append("| %s | %s | %s | %s | %s |" % (r["vaihe"], cell(r["nimi"]), cell(r["tila"]),
                                                     mdlink(doc, self.rel(uri, u)) if u else cell(doc or "–"),
                                                     eras.get(r["vaihe"], "") or ""))
        if eras.get("K1"):
            L += ["", "Kierroksen 1 (inventaarion) havaintoja: %d (erä *K1*)." % eras["K1"]]
        L += ["", "Raakatulokset, lokit ja koeohjelmat ovat repositoriossa hakemistossa "
              "[`doc/arkkitehtuuri/todisteet/`](%s) eikä niitä julkaista sivuina." % self.gh("doc/arkkitehtuuri/todisteet", kind="tree"),
              "Työn tarkistuspiste: [`CHECKPOINT.md`](%s)." % self.gh("doc/arkkitehtuuri/CHECKPOINT.md")]
        L += self.generated_note()
        self.add(uri, "\n".join(L))

    # -------------------------------------------------------------- kattavuus ---
    def coverage_run(self):
        """Run kattavuus.py (read-only) into a cache directory; return the directory."""
        if self.kattavuus_dir:
            return self.kattavuus_dir
        key = hashlib.sha256(((self.git.out("rev-parse", "HEAD") or "") +
                              (self.git.out("status", "--porcelain", "--", "doc", "TPsource") or "")).encode()).hexdigest()[:16]
        base = self.cache_dir or os.path.join(tempfile.gettempdir(), "tp-sivusto")
        out = os.path.join(base, "kattavuus-" + key)
        if not os.path.exists(os.path.join(out, "funktiot.csv")):
            os.makedirs(out, exist_ok=True)
            p = subprocess.run([sys.executable, "-I", os.path.join(HERE, "kattavuus.py"), self.root, out],
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            if p.returncode != 0:
                raise RuntimeError("kattavuus.py failed: " + p.stdout.decode("utf-8", "replace")[-500:])
            for old in os.listdir(base):
                if old.startswith("kattavuus-") and old != "kattavuus-" + key:
                    shutil.rmtree(os.path.join(base, old), ignore_errors=True)
        self.kattavuus_dir = out
        return out

    def page_coverage(self):
        uri = "laatu/kattavuus/index.md"
        kd = self.coverage_run()
        funcs = rcsv(os.path.join(kd, "funktiot.csv"))
        files = rcsv(os.path.join(kd, "tiedostot.csv"))
        reg = os.path.join(self.doc, "rekisteri")
        alueet = rcsv(os.path.join(reg, "koodialueet.csv"))
        reviews = rcsv(os.path.join(reg, "katselmoinnit.csv"))
        rajaukset = rcsv(os.path.join(reg, "rajaukset.csv"))
        lv = self.file_levels()
        tot_lines = sum(v["rivit"] for v in lv.values())
        tot_files = sum(v["tiedostot"] for v in lv.values())
        LEVEL = {"1": "vain inventoitu", "2": "kohdennetusti luettu", "3": "rakenne luettu", "4": "kokonaan katselmoitu"}
        prod = [f for f in funcs if f["tiedostoluokka"] == "tuote" and f["aktiivinen"].strip()]
        cls = collections.Counter(f["luokka"] for f in prod)
        cls_r = collections.Counter()
        for f in prod:
            cls_r[f["luokka"]] += int(f["rivit"] or 0)
        prod_lines = sum(cls_r.values())
        base_rev = self.audit_rev
        changed = self.git.out("diff", "--name-only", base_rev, "HEAD", "--", "TPsource/V52",
                               ":(exclude)TPsource/V52/Tests", ":(exclude)TPsource/V52/VS/Tests") or ""
        changed = [c for c in changed.split("\n") if c]
        head = self.git.out("rev-parse", "--short", "HEAD") or "?"
        # current state per code area (C-14 registers, when they exist)
        best, stale = self.review_state()
        k4 =sum(1 for v in best.values() if v >= "K4")
        k5 = sum(1 for v in best.values() if v >= "K5")
        L = ["# Auditoinnin kattavuus ja rajoitteet", "",
             '!!! warning "Lähdeviite ei ole katselmointi"\n    Se, että lähdekoodikohtaan on viitattu jossakin auditoinnin '
             "asiakirjassa, tarkoittaa vain, että joku on katsonut kohtaa jonkin väitteen vuoksi. Se ei tarkoita, että funktio "
             "on katselmoitu. Kokonaan katselmoiduksi lasketaan vain C-14:n kirjaama taso K4 tai K5.", "",
             "## Yhteenveto", "",
             "| Mittari | Arvo | Lähde |", "|---|---|---|",
             "| Lähdetiedostoja rekisterissä | %s tiedostoa, %s riviä | `rekisteri/tiedostot.csv` |" % (fmt_int(tot_files), fmt_int(tot_lines)),
             "| Vain inventoitu (taso 1) | %s riveistä (%d tiedostoa) | `tiedostot.csv` |" % (
                 pct(lv.get("1", {}).get("rivit", 0), tot_lines), lv.get("1", {}).get("tiedostot", 0)),
             "| Kokonaan katselmoituja tuotantotiedostoja (taso 4) | %d | `tiedostot.csv` |" % self.product_full_files(),
             "| Aktiivisia tuotantofunktioita | %s (%s riviä) | `kattavuus.py` |" % (fmt_int(len(prod)), fmt_int(prod_lines)),
             "| – ei yhtään viitettä | %s funktiota, %s riveistä | `kattavuus.py` |" % (fmt_int(cls.get("ei_viitteita", 0)), pct(cls_r.get("ei_viitteita", 0), prod_lines)),
             "| – viitattu vain johdetuissa asiakirjoissa | %s funktiota | `kattavuus.py` |" % fmt_int(cls.get("vain_johdettu", 0)),
             "| – viitattu yhdestä näkökulmasta | %s funktiota | `kattavuus.py` |" % fmt_int(cls.get("yksi_nakokulma", 0)),
             "| – viitattu useasta näkökulmasta | %s funktiota | `kattavuus.py` |" % fmt_int(cls.get("useita", 0)),
             "| Katselmoitu kokonaan (K4–K5, C-14) | %d koodialuetta, joista riippumattomasti tarkastettu (K5) %d | `rekisteri/katselmoinnit.csv` |" % (k4, k5),
             "| Vanhentuneet katselmoinnit | %d | tiiviste muuttunut |" % len(stale), "",
             "Viitetiedot laskettiin ajamalla `tyokalut/kattavuus.py` revisiosta `%s` (työpuu). Auditoinnin perusrevisio on "
             "`%s`; tuotantokoodissa on sen jälkeen muuttunut **%d** tiedostoa%s." % (
                 head, base_rev, len(changed), " – viitteet voivat olla vanhentuneita näissä: " + ", ".join("`%s`" % c for c in changed[:20]) if changed else
                 " (testihakemistoja `Tests/` ja `VS/Tests/` ei lasketa), joten auditoinnin viitteet osuvat nykyiseen koodiin"), "",
             "## Mitä on auditoitu ja mitä vain inventoitu {#tasot}", "",
             "Tiedostokohtainen taso on kirjattu erittäin (`analyysitaso`):", "",
             "| Taso | Merkitys | Tiedostoja | Rivejä | Osuus riveistä |", "|---|---|--:|--:|--:|"]
        for k in sorted(lv):
            L.append("| %s | %s | %d | %s | %s |" % (k, LEVEL.get(k, k), lv[k]["tiedostot"], fmt_int(lv[k]["rivit"]), pct(lv[k]["rivit"], tot_lines)))
        L += ["", "Taso 4 on toistaiseksi vain auditoinnin omissa testitiedostoissa. Tiedostoittainen erittely: "
              "[Tiedostoittain](%s)." % self.rel(uri, "laatu/kattavuus/tiedostot.md"), "",
              "## Mihin lähdekoodialueisiin on viitattu {#viitteet}", "",
              "Funktiotaso hakemistoittain (aktiiviset tuotantofunktiot; viite = funktion sisälle osuva `polku:rivi`):", "",
              "| Hakemisto | Funktioita | Rivejä | Ei viitteitä | Vain johdettu | Yksi näkökulma | Useita | Ilman viitettä riveistä |",
              "|---|--:|--:|--:|--:|--:|--:|--:|"]
        by = collections.defaultdict(list)
        for f in prod:
            by[f["polku"].split("/")[0]].append(f)
        for dname in sorted(by):
            fs = by[dname]
            c = collections.Counter(f["luokka"] for f in fs)
            rr = sum(int(f["rivit"] or 0) for f in fs)
            nr = sum(int(f["rivit"] or 0) for f in fs if f["luokka"] == "ei_viitteita")
            L.append("| `%s/` | %d | %s | %d | %d | %d | %d | %s |" % (dname, len(fs), fmt_int(rr), c.get("ei_viitteita", 0),
                                                                    c.get("vain_johdettu", 0), c.get("yksi_nakokulma", 0), c.get("useita", 0), pct(nr, rr)))
        prods = collections.defaultdict(lambda: [0, 0, 0])
        for f in prod:
            for pr in f["aktiivinen"].split():
                prods[pr][0] += 1
                prods[pr][1] += int(f["rivit"] or 0)
                if f["luokka"] == "ei_viitteita":
                    prods[pr][2] += int(f["rivit"] or 0)
        L += ["", "Tuotteittain (funktio lasketaan jokaiseen tuotteeseen, jossa se on voimassa):", "",
              "| Ohjelma | Funktioita | Rivejä | Ilman viitettä riveistä |", "|---|--:|--:|--:|"]
        for pr in PROGRAMS:
            if pr in prods:
                a, b, c = prods[pr]
                L.append("| %s | %d | %s | %s |" % (pr, a, fmt_int(b), pct(c, b)))
        excl = collections.Counter(f["tiedostoluokka"] for f in files)
        excl_r = collections.Counter()
        for f in files:
            excl_r[f["tiedostoluokka"]] += int(f["rivit"] or 0)
        L += ["", "## Rajaukset {#rajaukset}", "",
              "Tiedostoluokat (`kattavuus.py`): tuotteiden ulkopuolinen koodi on rajattu tai katselmoidaan kevyemmin "
              "[C-14-suunnitelman luvun 5](%s) mukaisesti." % self.rel(uri, "laatu/raportit/c14-suunnitelma.md#" +
                                                                    self.map_anchor("laatu/raportit/c14-suunnitelma.md", "5-käännöshaarat-ja-rajaukset--build-branches-and-exclusions")), "",
              "| Tiedostoluokka | Tiedostoja | Rivejä | Käsittely |", "|---|--:|--:|---|"]
        HOW = {"tuote": "katselmoidaan (C-14)", "orpo": "ei missään tuotteessa: rajaus R-orpo, kopioiden ero mitataan",
               "kolmas_osapuoli": "katselmoidaan kokonaan (W13); lisenssit omistajan päätöksessä",
               "testi": "auditoinnin oma testikoodi: rajaus R-testi", "generoitu": "rajaus R-generoitu"}
        for k in sorted(excl):
            L.append("| %s | %d | %s | %s |" % (k, excl[k], fmt_int(excl_r[k]), HOW.get(k, "")))
        inactive = [f for f in funcs if f["tiedostoluokka"] == "tuote" and not f["aktiivinen"].strip()]
        L += ["", "Kokonaan passiivisia (missään tuotteessa voimassa olemattomia) funktioita: %d (%s riviä); rajaus "
              "R-kuollut." % (len(inactive), fmt_int(sum(int(f["rivit"] or 0) for f in inactive))), ""]
        if rajaukset:
            L += ["Kirjatut rajaukset (`rekisteri/rajaukset.csv`):", "", "| Tunnus | Peruste | Hyväksyjä |", "|---|---|---|"]
            for r in rajaukset:
                L.append("| %s | %s | %s |" % (cell(r.get("tunnus", "")), cell(r.get("peruste", "")), cell(r.get("hyvaksyja", ""))))
            L.append("")
        else:
            L += ["Rajauksia ei ole vielä kirjattu rekisteriin `rekisteri/rajaukset.csv` (luodaan C-14:ssä).", ""]
        L += ["## C-14: katselmoinnin eteneminen työpaketeittain {#c-14}", ""]
        plan = read(os.path.join(self.doc, "c14-suunnitelma.md"))
        wp = re.findall(r"^\| (W\d\d) \| ([^|]+) \| ([^|]+) \| ([^|]+) \| ([^|]+) \| ([^|]+) \|$", plan, re.M)
        phase = {r["vaihe"]: r for r in self.phases()}.get("C-14", {})
        L += ["Tila: **%s** (`rekisteri/vaiheet.csv`). Suunnitelma: [C-14](%s)." % (
            phase.get("tila", "ei tiedossa"), self.rel(uri, "laatu/raportit/c14-suunnitelma.md")), ""]
        pk = collections.defaultdict(lambda: {"alueet": 0, "rivit": 0, "k4": 0, "k5": 0, "vanha": 0})
        for a in alueet:
            d = pk[a.get("paketti", "") or "-"]
            d["alueet"] += 1
            d["rivit"] += int(a.get("rivit") or 0)
            lvl = best.get(a["alue"], "K0")
            d["k4"] += lvl >= "K4"
            d["k5"] += lvl >= "K5"
            d["vanha"] += a["alue"] in stale
        L += ["| Paketti | Sisältö | Suunnitellut rivit | Viittaamatta (lähtötaso) | Yksiköitä | Alueita | K4+ | K5 (riippumaton) | Vanhentunut |",
              "|---|---|--:|--:|--:|--:|--:|--:|--:|"]
        for w in wp:
            d = pk.get(w[0], {"alueet": 0, "k4": 0, "k5": 0, "vanha": 0})
            L.append("| %s | %s | %s | %s | %s | %s | %d | %d | %d |" % (
                w[0], self.transform(cell(w[1]), uri), w[2].strip(), w[3].strip(), w[4].strip(),
                d["alueet"] if alueet else "–", d["k4"], d["k5"], d["vanha"]))
        planned = {w[0] for w in wp}
        for name in sorted(k for k in pk if k not in planned):
            d = pk[name]
            L.append("| %s | *paketti puuttuu suunnitelman taulukosta* | – | – | – | %d | %d | %d | %d |" % (
                cell(name), d["alueet"], d["k4"], d["k5"], d["vanha"]))
        L += ["", ("Koodialueiden rekisteriä `rekisteri/koodialueet.csv` ei ole vielä tuotettu, joten katselmointia ei ole "
                   "aloitettu: yhtäkään aluetta ei ole kirjattu tasolle K4." if not alueet else
                   "Alueet: `rekisteri/koodialueet.csv`; katselmoinnit: `rekisteri/katselmoinnit.csv`."),
              "Sarakkeet *Suunnitellut rivit*, *Viittaamatta* ja *Yksiköitä* ovat C-14-suunnitelman taulukosta (luku 6.1); "
              "muut lasketaan rekistereistä. Riippumaton tarkastus = taso K5.", ""]
        L += self.generated_note()
        self.add(uri, "\n".join(L))
        # per file
        u2 = "laatu/kattavuus/tiedostot.md"
        regf = {r["polku"]: r for r in rcsv(os.path.join(reg, "tiedostot.csv"))}
        L = ["# Kattavuus tiedostoittain", "", "Taso = auditoinnin kirjaama tiedostotaso (1 inventoitu, 2 kohdennettu, 3 rakenne, "
             "4 kokonaan). Funktiot ja viitteet: `kattavuus.py`. Tiedostonimi on linkki lähdekoodiin auditoinnin "
             "perusrevisiossa `%s`. Viitteet eivät tarkoita katselmointia." % base_rev, ""]
        bydir = collections.defaultdict(list)
        for f in files:
            bydir[f["polku"].split("/")[0]].append(f)
        for dname in sorted(bydir):
            L += ["## %s/" % dname, "", "| Tiedosto | Luokka | Taso | Rivejä | Funktioita | Viitattuja funktioita | Viitteitä |",
                  "|---|---|---|--:|--:|--:|--:|"]
            for f in sorted(bydir[dname], key=lambda x: x["polku"].lower()):
                got = self.linker.resolve(f["polku"], None, self.audit_rev)
                name = "[`%s`](%s)" % (f["polku"], self.linker.url(got[0], got[1])) if got else "`%s`" % f["polku"]
                L.append("| %s | %s | %s | %s | %s | %s | %s |" % (name, f["tiedostoluokka"], regf.get(f["polku"], {}).get("analyysitaso", f["taso"]),
                                                                  f["rivit"], f["funktiot"], f["funktiot_viitteella"], f["viitteet"]))
            L.append("")
        L += self.generated_note()
        self.add(u2, "\n".join(L))

    # ------------------------------------------------------- asetukset ---
    def page_settings(self):
        reg = os.path.join(self.doc, "rekisteri")
        uri = "kehittajat/asetukset/asetusavaimet.md"
        rows = rcsv(os.path.join(reg, "asetusavaimet.csv"))
        L = ["# Asetusavaimet", "", "Asetustiedostojen ja komentorivin avaimet jäsentimittäin (`rekisteri/asetusavaimet.csv`, "
             "erä C-12; tulkinta [Asetukset ja käännösvalitsimet](%s)). Sarakkeet ohjelmittain kertovat, onko avain voimassa "
             "ohjelmassa. %d riviä." % (self.rel(uri, "kehittajat/arkkitehtuuri/ominaisuusvalitsimet.md"), len(rows)), ""]
        by = collections.defaultdict(list)
        for r in rows:
            by[r["jasennin"]].append(r)
        for j in sorted(by):
            L += ["## `%s`" % j, "", "%s." % cell(by[j][0]["rooli"]), "",
                  "| Avain | Muuttuja | Rivit | Ehto | " + " | ".join(PROGRAMS) + " |", "|---|---|---|---|" + "---|" * len(PROGRAMS)]
            for r in by[j]:
                first = re.split(r"[ ,;]+", r["rivit"].strip())[0] if r["rivit"].strip() else ""
                rr = self.linker.resolve(j, None, self.audit_rev) if first.isdigit() else None
                lines = "[%s](%s)" % (cell(r["rivit"]), self.linker.url(rr[0], rr[1], (int(first), int(first)))) if rr else cell(r["rivit"])
                L.append("| `%s` | `%s` | %s | %s | %s |" % (cell(r["avain"]), cell(r["muuttuja"]), lines, cell(r["ehto"]),
                                                            " | ".join(cell(r.get(p, "")) for p in PROGRAMS)))
            L.append("")
        L += self.generated_note()
        self.add(uri, "\n".join(L))
        uri = "kehittajat/asetukset/valitsimet.md"
        rows = rcsv(os.path.join(reg, "valitsimet.csv"))
        L = ["# Esikääntäjän valitsimet", "", "Jokainen ehtona testattu makro (`rekisteri/valitsimet.csv`, erä C-12): luokka, "
             "merkitys ja arvo ohjelmittain. %d riviä." % len(rows), ""]
        by = collections.defaultdict(list)
        for r in rows:
            by[r["luokka"] or "-"].append(r)
        for k in sorted(by):
            L += ["## %s" % k, "", "| Makro | Merkitys | " + " | ".join(PROGRAMS) + " | Tila |", "|---|---|" + "---|" * len(PROGRAMS) + "---|"]
            for r in sorted(by[k], key=lambda x: x["nimi"]):
                L.append("| `%s` | %s | %s | %s |" % (cell(r["nimi"]), cell(r["merkitys"]), " | ".join(cell(r.get(p, "")) for p in PROGRAMS), cell(r["tila"])))
            L.append("")
        L += self.generated_note()
        self.add(uri, "\n".join(L))

    # --------------------------------------------------------- etusivu ---
    def page_quality_index(self):
        uri = "laatu/index.md"
        rows = self.rows()
        H = [r for r in rows if r["laji"] == "H"]
        T = [r for r in rows if r["laji"] == "T"]
        oh = [r for r in H if r["tila"] == "avoin"]
        pr = collections.Counter(r["p"] for r in oh)
        tv = collections.Counter(r["todennus"] for r in H)
        st = collections.Counter(r["tila"] for r in H)
        packs = [p for p in self.data.packs if p.startswith("KK-")]
        op_open = sum(1 for o in self.data.op.values() if o["tila"].startswith("avoin"))
        qst = collections.Counter(q["tila"] or "?" for q in self.data.qs.values())
        L = ["# Ohjelmiston laatu", "",
             "Tämä osio julkaisee tulospalveluohjelmiston auditoinnin: mitä koodista on tutkittu, mitä on löydetty, "
             "kuinka varmoja löydökset ovat ja miten ne on tarkoitus korjata. Kaikki luvut lasketaan rekistereistä "
             "jokaisella sivuston rakennuskerralla.", "", self.coverage_warning(uri), "",
             "## Neljä tiedon lajia", "",
             "| Laji | Missä | Merkitys |", "|---|---|---|",
             "| Alkuperäinen dokumentaatio | [Käyttöohjeet](%s) | Pekka Pirilän käyttöohjeet sellaisinaan |" % self.rel(uri, "index.md"),
             "| Koodista todennettu toiminta | [Toiminnallinen käsikirja](%s) | kuvaus nykyisestä koodista lähdeviittein |" % self.rel(uri, "kehittajat/kasikirja/index.md"),
             "| Auditointihavainnot | [Vikakortisto](%s), [rekisteri](%s) | vikoja ja riskejä; useimpia ei ole todennettu ajamalla |" % (
                 self.rel(uri, "laatu/vikakortit/index.md"), self.rel(uri, "laatu/rekisteri/havainnot.md")),
             "| Suunnitellut muutokset | [Korjauskokonaisuudet](%s), [korjaussuunnitelma](%s) | suunniteltu, ei toteutettu |" % (
                 self.rel(uri, "laatu/korjauskokonaisuudet/index.md"), self.rel(uri, "laatu/raportit/korjaussuunnitelma.md")), "",
             "## Nykytila", "", "| Vaihe | Tila | Päivitetty |", "|---|---|---|"]
        for r in self.phases():
            if r.get("taso") == "vaihe":
                L.append("| %s %s | %s | %s |" % (r["vaihe"], cell(r["nimi"]), cell(r["tila"]), r["pvm"]))
        L += ["", "Kaikki vaiheet: [Eteneminen](%s)." % self.rel(uri, "laatu/eteneminen.md"), "",
              "## Havainnot", "",
              "| | Määrä |", "|---|--:|",
              "| Havaintoja (H) | %d |" % len(H)] + ["| – %s | %d |" % (k, v) for k, v in sorted(st.items())] + [
              "| Teknisen velan rivejä (T) | %d |" % len(T),
              "| Korjauskokonaisuuksia | %d |" % len(packs),
              "| Omistajan päätöksiä avoinna | %d / %d |" % (op_open, len(self.data.op)),
              "| Kysymyksiä | %d (%s) |" % (len(self.data.qs), ", ".join("%s %d" % kv for kv in sorted(qst.items()))), "",
              "**Avoimet havainnot prioriteetin mukaan:** " + " · ".join(
                  "[%s](%s#%s) %d" % (p, self.rel(uri, "laatu/vikakortit/index.md"), p.lower(), pr.get(p, 0)) for p in ["P1", "P2", "P3", "P4", "P5"]), "",
              "## Kuinka varmoja havainnot ovat", "", "| Todennus | Havaintoja |", "|---|--:|"]
        for k, v in sorted(tv.items(), key=lambda kv: -kv[1]):
            L.append("| %s | %d |" % (tp.TODENNUS.get(k, k), v))
        L += ["", "## Mistä aloittaa", "",
              "- Kilpailun järjestäjä: [Tunnetut ongelmat](%s) ja [Vaikutukset kilpailutoimintaan](%s)." % (
                  self.rel(uri, "laatu/tunnetut-ongelmat.md"), self.rel(uri, "laatu/vaikutukset.md")),
              "- Kehittäjä: [Korjauskokonaisuudet](%s), [Juurisyyt](%s), [Regressiotestaus](%s)." % (
                  self.rel(uri, "laatu/korjauskokonaisuudet/index.md"), self.rel(uri, "laatu/juurisyyt.md"),
                  self.rel(uri, "laatu/raportit/regressiotestaussuunnitelma.md")),
              "- Kaikki yhdessä näkymässä hakuineen ja suodattimineen: [Interaktiivinen portaali](%s)." % self.rel(uri, "laatu/portaali.md"),
              "- Auditoinnin raportit: [Auditointiraportit](%s); kattavuus: [Kattavuus ja rajoitteet](%s)." % (
                  self.rel(uri, "laatu/raportit/index.md"), self.rel(uri, "laatu/kattavuus/index.md"))]
        L += self.generated_note()
        self.add(uri, "\n".join(L))

    # ---------------------------------------------------------- portaali ---
    def page_portal(self):
        uri = "laatu/portaali.md"
        pdir = os.path.join(self.root, "doc", "tietopohja", "portaali")
        for f in ("index.html", "app.js", "tyyli.css"):
            with open(os.path.join(pdir, f), "rb") as fh:
                self.assets["laatu/portaali/app/" + f] = fh.read()
        d = portaali.build(self.root, web={"sivusto": "../../../", "linker": self.linker, "tila": "verkko"},
                           kattavuus_dir=self.coverage_run())
        payload = json.dumps(d, ensure_ascii=False, separators=(",", ":")).replace("</", "<\\/")
        self.assets["laatu/portaali/app/data/data.js"] = ("// GENERATED by doc/arkkitehtuuri/tyokalut/sivusto.py - do not edit\n"
                                                      "window.TIETOPOHJA = " + payload + ";\n").encode("utf-8")
        m = d["meta"]
        L = ["# Interaktiivinen tietopohjaportaali", "",
             "[Avaa portaali](%s){ .md-button .md-button--primary }" % self.rel(uri, PORTAL), "",
             "Portaali kokoaa koko tietopohjan yhteen selainnäkymään: havaintojen haku ja suodatus (ohjelma, prioriteetti, "
             "vakavuus, tila, erä, juurisyy, korjauskokonaisuus, todentamistaso ja käytännön vaikutus), vikakortit "
             "käyttäjä- ja teknisenä näkymänä, toiminnallinen käsikirja kortteihin linkitettynä, juurisyyt, "
             "korjauskokonaisuuksien riippuvuuskaavio, omistajan päätökset, kysymykset, eteneminen, testaus, kattavuus "
             "ja sanasto. Portaali toimii tavallisessa selaimessa; se ei hae mitään verkosta.", "",
             "Pysyvät osoitteet ovat tämän sivuston sivuja (esim. `laatu/vikakortit/H-071/`); portaalin osoitteet "
             "(`portaali/app/#/kortti/H-071`) ovat näkymiä samaan dataan.", "",
             "## Tietojen ajantasaisuus", "",
             "| | |", "|---|---|",
             "| Data tuotettu commitista | `%s` (%s) |" % (m["commit"], m["tuotettu"]),
             "| Lähdetiiviste | `%s` |" % m["lahdetiiviste"],
             "| Lähdeviitteiden revisiot | %s |" % ", ".join("`%s`" % r for r in m["viiterevisiot"]),
             "| Havaintoja / kortteja / käsikirjan lukuja | %d / %d / %d |" % (
                 len(d["havainnot"]), sum(1 for x in d["havainnot"] if x["kortti"]), len(d["kasikirja"])), "",
             "Sivusto ja portaalin data tuotetaan samasta rakennusajosta, joten ne ovat aina samassa tilassa. "
             "Paikallinen versio: `doc/tietopohja/portaali/avaa.ps1` tai `avaa.sh` (ks. "
             "[Tietopohjan rakenne ja ylläpito](%s))." % self.rel(uri, "kehittajat/tietopohja/index.md")]
        L += self.generated_note()
        self.add(uri, "\n".join(L))

    # ------------------------------------------------------ käyttöohjeet ---
    def manual_boxes(self):
        """Info boxes for user-manual pages (doc/tietopohja/kasikirja/kayttoohjeet.csv)."""
        path = os.path.join(self.root, "doc", "tietopohja", "kasikirja", "kayttoohjeet.csv")
        funcs = {r["toiminto"]: r for r in rcsv(os.path.join(self.root, "doc", "tietopohja", "kasikirja", "toiminnot.csv"))}
        for r in rcsv(path):
            page = r["sivu"].strip()
            uri = page
            L = ['!!! info "Tekninen tietopohja – lisätty, ei osa alkuperäistä ohjetta"', "",
                 "    Yllä oleva ohje on alkuperäinen käyttöohje. Tämä laatikko on lisätty automaattisesti "
                 "tietopohjasta (`doc/tietopohja/kasikirja/kayttoohjeet.csv`), eikä se muuta ohjetta.", ""]
            ch = r["luku"].strip()
            for c in ch.split():
                cu = "kehittajat/kasikirja/%s.md" % c
                if cu in self.pages:
                    L.append("    - **Koodista todennettu toiminta:** [%s](%s)" % (front_title(read(os.path.join(
                        self.root, "doc", "tietopohja", "kasikirja", c + ".md"))), self.rel(uri, cu)))
            for t in [x.strip() for x in r["toiminnot"].split("|") if x.strip()]:
                f = funcs.get(t)
                if f:
                    L.append("    - **Toimintokuvaus:** %s ([%s](%s))" % (t, f["luku"], self.rel(uri, "kehittajat/kasikirja/%s.md" % f["luku"])))
            ids = r["havainnot"].split()
            if ids:
                L += ["    - **Tunnetut ongelmat** (auditointihavaintoja; tila ja todennus kortilla):", ""]
                for i in ids:
                    sr = self.data.state_row(i)
                    rk = self.data.risk.get(i, {})
                    adv = self.card_heading_anchor(i, "Mitä käyttäjä voi tehdä")
                    extra = (" – [mitä voi tehdä](%s)" % self.rel(uri, "laatu/vikakortit/%s.md#%s" % (i, adv))) if adv else ""
                    L.append("        - [%s](%s) %s (%s%s)%s" % (i, self.rel(uri, "laatu/vikakortit/%s.md" % i), self.card_title(i),
                                                               sr.get("tila", "?"), (", " + rk["prioriteetti"]) if rk.get("prioriteetti") else "", extra))
                L.append("")
            L.append("    Kiertotapojen ja suositusten varmuus on merkitty kortilla (todennettu toimintaohje / perusteltu, "
                     "testaamaton suositus). [Ohjelmiston laatu](%s)" % self.rel(uri, "laatu/index.md"))
            self.boxes[page] = "\n".join(L) + "\n"

    # ------------------------------------------------------ fragments ---
    def fragments(self):
        """Generated fragments for <!-- tp:NAME --> markers in hand-written pages."""
        f = {}
        rows = []
        for name in sorted(os.listdir(os.path.join(self.root, ".claude", "agents"))):
            if name.endswith(".md"):
                fm = self.front_matter(os.path.join(self.root, ".claude", "agents", name))
                rows.append("| `%s` | %s | %s | %s |" % (fm.get("name", name[:-3]), cell(fm.get("model", "")), cell(fm.get("tools", "")),
                                                         cell(fm.get("description", ""))))
        f["agentit"] = "\n".join(["| Agentti | Malli | Työkalut | Tehtävä |", "|---|---|---|---|"] + rows)
        rows = []
        sk = os.path.join(self.root, ".claude", "skills")
        for name in sorted(os.listdir(sk)):
            p = os.path.join(sk, name, "SKILL.md")
            if os.path.exists(p):
                fm = self.front_matter(p)
                rows.append("| `/%s` | %s |" % (fm.get("name", name), cell(fm.get("description", ""))))
        f["skillit"] = "\n".join(["| Skill | Tehtävä |", "|---|---|"] + rows)
        rows = []
        for name in sorted(os.listdir(HERE)):
            if name.endswith((".py", ".js")):
                text = read(os.path.join(HERE, name))
                m = re.search(r'^(?:#!.*\n)?(?:"""|//\s*)(.+)', text, re.M)
                rows.append("| [`%s`](%s) | %s |" % (name, self.gh("doc/arkkitehtuuri/tyokalut/" + name), cell(m.group(1).strip().rstrip('"') if m else "")))
        f["tyokalut"] = "\n".join(["| Työkalu | Tehtävä |", "|---|---|"] + rows)
        contrib = read(os.path.join(self.root, "CONTRIBUTING.md"))
        contrib = re.sub(r"^# .*\n", "", contrib, count=1)
        f["contributing"] = ('!!! abstract "Kanoninen lähde"\n    Tämä sivu näyttää repositorion tiedoston '
                             "[`CONTRIBUTING.md`](%s).\n\n" % self.gh("CONTRIBUTING.md") +
                             self.transform(contrib, "kehittajat/osallistuminen.md", "CONTRIBUTING.md"))
        return f

    def front_matter(self, path):
        text = read(path)
        m = re.match(r"---\n(.*?)\n---", text, re.S)
        out = {}
        if m:
            for line in m.group(1).split("\n"):
                k, _, v = line.partition(":")
                if v:
                    out[k.strip()] = v.strip()
        return out


def nav(root):
    """Navigation of the two sections, with the handbook chapters expanded."""
    hb = os.path.join(root, "doc", "tietopohja", "kasikirja")
    chapters = [{"Lukuohje": "kehittajat/kasikirja/index.md"}]
    for f in sorted(os.listdir(hb)):
        if re.match(r"\d{2}-.*\.md$", f) and not f.startswith(("00-", "01-")):
            chapters.append({front_title(read(os.path.join(hb, f))) or f[:-3]: "kehittajat/kasikirja/" + f})
    ad = os.path.join(hb, "aineisto")
    if os.path.isdir(ad):
        for f in sorted(os.listdir(ad)):
            if f.endswith(".md"):
                chapters.append({"Aineisto: " + (front_title(read(os.path.join(ad, f))) or f[:-3]):
                                 "kehittajat/kasikirja/aineisto/" + f})

    def expand(items):
        out = []
        for it in items:
            (k, v), = it.items()
            if v == "@kasikirja":
                out.append({k: chapters})
            elif isinstance(v, list):
                out.append({k: expand(v)})
            else:
                out.append({k: v})
        return out
    return expand(NAV)


# --------------------------------------------------------------- checks ---
def check(root):
    """Mapping files and generated links: return a list of errors."""
    errs = []
    s = Site(root).generate()
    docs = os.path.join(root, "docs-site", "docs")
    funcs = {r["toiminto"] for r in rcsv(os.path.join(root, "doc", "tietopohja", "kasikirja", "toiminnot.csv"))}
    for r in rcsv(os.path.join(root, "doc", "tietopohja", "kasikirja", "kayttoohjeet.csv")):
        if not os.path.exists(os.path.join(docs, r["sivu"])):
            errs.append("kayttoohjeet.csv: page %s does not exist" % r["sivu"])
        for c in r["luku"].split():
            if "kehittajat/kasikirja/%s.md" % c not in s.pages:
                errs.append("kayttoohjeet.csv %s: chapter %s unknown" % (r["sivu"], c))
        for t in [x.strip() for x in r["toiminnot"].split("|") if x.strip()]:
            if t not in funcs:
                errs.append("kayttoohjeet.csv %s: function %r not in toiminnot.csv" % (r["sivu"], t))
        for i in r["havainnot"].split():
            if i not in s.data.reg:
                errs.append("kayttoohjeet.csv %s: %s not in the register" % (r["sivu"], i))
        if not r.get("peruste", "").strip() or not r.get("tarkistettu", "").strip():
            errs.append("kayttoohjeet.csv %s: peruste and tarkistettu are required" % r["sivu"])
    for r in s.phases():
        if r["asiakirja"] and not os.path.exists(os.path.join(root, "doc", "arkkitehtuuri", r["asiakirja"])):
            errs.append("vaiheet.csv %s: document %s missing" % (r["vaihe"], r["asiakirja"]))
    errs += ["warning: " + w for w in s.warnings + s.linker.warnings]
    return errs, s


def main(argv):
    if len(argv) < 3:
        print(__doc__)
        return 2
    root = argv[1]
    if argv[2] == "--tarkista":
        errs, s = check(root)
        hard = [e for e in errs if not e.startswith("warning: ")]
        for e in errs:
            print(e)
        print("pages %d, assets %d, manual boxes %d, source links %d (unresolved %d, past end %d); %d errors, %d warnings" % (
            len(s.pages), len(s.assets), len(s.boxes), s.linker.stats["lahdelinkki"], s.linker.stats["ratkaisematon"],
            s.linker.stats["rivi_yli"], len(hard), len(errs) - len(hard)))
        return 1 if hard else 0
    out = argv[2]
    s = Site(root).generate()
    for uri, md in list(s.pages.items()) + list(s.assets.items()):
        p = os.path.join(out, *uri.split("/"))
        os.makedirs(os.path.dirname(p), exist_ok=True)
        with open(p, "wb") as fh:
            fh.write(md.encode("utf-8") if isinstance(md, str) else md)
    for page, box in s.boxes.items():
        p = os.path.join(out, "_kayttoohjelaatikot", *page.split("/"))
        os.makedirs(os.path.dirname(p), exist_ok=True)
        with open(p, "wb") as fh:
            fh.write(box.encode("utf-8"))
    print("written %d pages, %d assets, %d manual boxes to %s" % (len(s.pages), len(s.assets), len(s.boxes), out))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
