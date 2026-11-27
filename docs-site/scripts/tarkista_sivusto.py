#!/usr/bin/env python3
"""Check a built documentation site offline (no network).

Usage (from the repository root, after `mkdocs build`):
    python -I docs-site/scripts/tarkista_sivusto.py <site_dir> [--osoitteet <urls.txt>] [--kirjoita-osoitteet <urls.txt>]

Checks:
1. every relative link, image, stylesheet and script of every HTML page points to a file of
   the site, and every #anchor exists in the target page;
2. every GitHub link of this repository (blob/tree/commit) names a commit that exists locally
   and a path that exists in it, and its #Lnn line anchor is within the file;
3. with --osoitteet: every page URL in the given list (one path per line, e.g. from an older
   build) still exists, so old addresses keep working;
4. the portal and the search index exist.
Broken links on the new sections (kehittajat/, laatu/) are errors; on the imported user
manual pages they are reported as warnings (they were there before). Exit code 1 on errors.
"""
import collections
import html.parser
import os
import re
import subprocess
import sys
import urllib.parse

REPO = "https://github.com/PirilaTP/tulospalvelu"
SITE_PATH = "/tulospalvelu/"  # path of site_url (absolute links of 404.html)
NEW = ("kehittajat/", "laatu/")


class Page(html.parser.HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.ids = set()
        self.links = []

    def handle_starttag(self, tag, attrs):
        a = dict(attrs)
        for k in ("id", "name"):
            if a.get(k):
                self.ids.add(a[k])
        for k in ("href", "src"):
            if a.get(k) and not (tag == "link" and a.get("rel") in ("preconnect", "dns-prefetch")):
                self.links.append(a[k])


def git(root, *args):
    p = subprocess.run(["git", "-C", root] + list(args), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    return p.stdout if p.returncode == 0 else None


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    site = os.path.abspath(argv[1])
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    pages = {}
    for d, _, fs in os.walk(site):
        for f in fs:
            if f.endswith(".html"):
                p = os.path.join(d, f)
                rel = os.path.relpath(p, site).replace(os.sep, "/")
                pg = Page()
                with open(p, encoding="utf-8", errors="replace") as fh:
                    pg.feed(fh.read())
                pages[rel] = pg
    errors, warnings = [], []
    stats = collections.Counter()
    gh_cache = {}

    def report(src, msg):
        (errors if src.startswith(NEW) else warnings).append("%s: %s" % (src, msg))

    def gh_check(src, url):
        m = re.match(re.escape(REPO) + r"/(blob|tree|commit|edit|releases)/([^/#?]+)/?([^#?]*)(?:\?[^#]*)?(?:#L(\d+)(?:-L(\d+))?)?$", url)
        if not m:
            return
        kind, rev, path, a, b = m.groups()
        path = urllib.parse.unquote(path)
        if kind in ("edit", "releases"):
            stats["github_muu"] += 1
            return
        stats["github_" + kind] += 1
        if not re.match(r"^[0-9a-f]{7,40}$", rev):
            # a branch link (blob/main/...): the file must exist in the commit being built,
            # which becomes part of that branch when merged
            rev = "HEAD"
        key = (rev, path)
        if key not in gh_cache:
            if git(root, "cat-file", "-e", rev + "^{commit}") is None:
                gh_cache[key] = "commit %s not in the local repository" % rev
            elif kind == "commit":
                gh_cache[key] = None
            else:
                t = git(root, "cat-file", "-t", "%s:%s" % (rev, path))
                if t is None:
                    gh_cache[key] = "path %s not in %s" % (path, rev[:10])
                elif kind == "blob" and t.strip() != b"blob" or kind == "tree" and t.strip() != b"tree":
                    gh_cache[key] = "%s is not a %s in %s" % (path, kind, rev[:10])
                else:
                    gh_cache[key] = ("lines", git(root, "show", "%s:%s" % (rev, path)) or b"") if kind == "blob" else None
        res = gh_cache[key]
        if isinstance(res, tuple):
            n = res[1].count(b"\n") + (1 if res[1] and not res[1].endswith(b"\n") else 0)
            last = int(b or a or 0)
            if last > n:
                report(src, "line %d past end of %s (%d lines)" % (last, path, n))
        elif res:
            report(src, res)

    for src, pg in sorted(pages.items()):
        base = os.path.dirname(src)
        for link in pg.links:
            if link.startswith(REPO + "/"):
                gh_check(src, link)
                continue
            if re.match(r"^[a-z][a-z0-9+.-]*:", link) or link.startswith("//"):
                stats["ulkoinen"] += 1
                continue
            path, _, anchor = link.partition("#")
            path = urllib.parse.unquote(path.split("?")[0])
            if path.startswith(SITE_PATH):
                path = os.path.relpath(path[len(SITE_PATH):] or ".", base or ".").replace(os.sep, "/")
            if not path:
                target = src
            else:
                target = os.path.normpath(os.path.join(base, path)).replace(os.sep, "/")
                if target.startswith(".."):
                    report(src, "link outside the site: %s" % link)
                    continue
                full = os.path.join(site, target)
                if os.path.isdir(full):
                    target = (target.rstrip("/") + "/index.html").lstrip("./")
                    full = os.path.join(site, target)
                if not os.path.exists(full):
                    report(src, "missing target %s" % link)
                    continue
            stats["sisainen"] += 1
            if anchor and target.endswith(".html") and not anchor.startswith("/"):
                tp = pages.get(target)
                if tp is not None and urllib.parse.unquote(anchor) not in tp.ids:
                    report(src, "missing anchor %s" % link)
    for need in ("laatu/portaali/app/index.html", "laatu/portaali/app/data/data.js", "search/search_index.json"):
        if not os.path.exists(os.path.join(site, need)):
            errors.append("missing %s" % need)
    if "--osoitteet" in argv:
        old = [l.strip().lstrip("./") for l in open(argv[argv.index("--osoitteet") + 1], encoding="utf-8")
               if l.strip() and not l.startswith("#")]
        gone = [u for u in old if u.replace("\\", "/") not in pages]
        stats["vanhat_osoitteet"] = len(old)
        for u in gone:
            errors.append("old address no longer exists: %s" % u)
    if "--kirjoita-osoitteet" in argv:
        with open(argv[argv.index("--kirjoita-osoitteet") + 1], "w", encoding="utf-8", newline="\n") as fh:
            fh.write("\n".join(sorted(pages)) + "\n")
    for w in warnings[:30]:
        print("WARNING " + w)
    if len(warnings) > 30:
        print("WARNING ... %d more" % (len(warnings) - 30))
    for e in errors[:60]:
        print("ERROR " + e)
    print("pages %d; %s; %d errors, %d warnings" % (len(pages), ", ".join("%s %d" % kv for kv in sorted(stats.items())),
                                                    len(errors), len(warnings)))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
