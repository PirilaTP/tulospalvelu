#!/usr/bin/env python3
"""Source references as GitHub links (shared by sivusto.py and portaali.py; read-only).

A reference in the audit documents is a code span such as `Hk/HkEmit.cpp:123`,
`TPsource/V52/Juk/VCom.cpp:968-970`, `Tp/TpComY.cpp:1001, 2399`, a bare follow-up `:2399`
(same path as the previous reference on the line) or a path without lines. Paths of
production code are relative to TPsource/V52 as in the register.

Revision rule (never "latest main"):
- a reference of production code is linked at the revision the claim was made against
  (register column Rev; for documents the audit base revision, the most common Rev);
- if that revision is not the audit base but the file is byte-identical in the audit base
  (same blob), the audit base is used instead, because the audit base is a published
  commit of main and the audit branch commits may not be published;
- a file that does not exist in the claim revision (for example tests added later) must
  exist in the build commit; it is linked on the published branch (main);
- documents and tools outside TPsource/V52 must exist in the build commit and are linked on
  the published branch (main), so the links survive a squash merge.
Line numbers are checked against the file in the linked revision; a line past the end
drops the line anchor and is counted as a warning.

Nothing is written; only `git` is called (cat-file, ls-tree, rev-parse).
"""
import collections
import os
import posixpath
import re
import subprocess

SRC_PREFIX = "TPsource/V52/"
DASH = "[-–]"
EXT = r"(?:cpp|c|hpp|h|H|dfm|cbproj|vcxproj|sln|md|csv|py|js|ps1|sh|yml|yaml|json|txt|html|css)"
PATH = r"((?:\.{1,2}/)*(?:[A-Za-z0-9_.\-]+/)*[A-Za-z0-9_\-][A-Za-z0-9_.\-]*\." + EXT + r")"
SPEC = r"(\d+(?:\s*" + DASH + r"\s*\d+)?(?:\s*,\s*:?\d+(?:\s*" + DASH + r"\s*\d+)?)*)"
CODE_REF = re.compile(r"^" + PATH + r"(?:\s*:\s*" + SPEC + r")?(?:\s.*)?$")
BARE = re.compile(r"^:\s*" + SPEC + r"$")


def first_span(spec):
    part = re.split(r"\s*,\s*", spec.strip())[0].lstrip(":")
    m = re.match(r"(\d+)(?:\s*" + DASH + r"\s*(\d+))?", part)
    if not m:
        return None
    a = int(m.group(1))
    b = int(m.group(2)) if m.group(2) else a
    return (a, b) if a <= b else (b, a)


class Git:
    """Cached, read-only access to the repository objects."""

    def __init__(self, root):
        self.root = root
        self._full = {}
        self._trees = {}
        self._lower = {}
        self._blob = {}
        self._lines = {}
        self._batch = None

    def out(self, *args):
        p = subprocess.run(["git", "-C", self.root] + list(args), stdout=subprocess.PIPE,
                           stderr=subprocess.DEVNULL)
        return p.stdout.decode("utf-8", "replace").strip() if p.returncode == 0 else None

    def full(self, rev):
        if rev not in self._full:
            self._full[rev] = self.out("rev-parse", "--verify", "--quiet", rev + "^{commit}")
        return self._full[rev]

    def tree(self, rev):
        full = self.full(rev)
        if full is None:
            return set()
        if full not in self._trees:
            listing = self.out("-c", "core.quotepath=off", "ls-tree", "-r", "--name-only", full) or ""
            self._trees[full] = set(listing.split("\n")) if listing else set()
            low = collections.defaultdict(list)
            for p in self._trees[full]:
                low[p.lower()].append(p)
            self._lower[full] = low
        return self._trees[full]

    def canonical(self, rev, path):
        """Exact-case path in rev (GitHub URLs are case-sensitive), or None."""
        full = self.full(rev)
        t = self.tree(rev)
        if path in t:
            return path
        c = self._lower.get(full, {}).get(path.lower(), [])
        return c[0] if len(c) == 1 else None

    def blob(self, rev, path):
        key = (rev, path)
        if key not in self._blob:
            self._blob[key] = self.out("rev-parse", "--verify", "--quiet", "%s:%s" % (self.full(rev), path))
        return self._blob[key]

    def lines(self, rev, path):
        key = (rev, path)
        if key not in self._lines:
            if self._batch is None:
                self._batch = subprocess.Popen(["git", "-C", self.root, "cat-file", "--batch"],
                                               stdin=subprocess.PIPE, stdout=subprocess.PIPE)
            self._batch.stdin.write(("%s:%s\n" % (self.full(rev), path)).encode("utf-8"))
            self._batch.stdin.flush()
            head = self._batch.stdout.readline().decode("utf-8", "replace").split()
            if len(head) < 3 or head[1] == "missing":
                self._lines[key] = None
            else:
                size = int(head[2])
                data = self._batch.stdout.read(size)
                self._batch.stdout.read(1)
                n = data.count(b"\n")
                if data and not data.endswith(b"\n"):
                    n += 1
                self._lines[key] = n
        return self._lines[key]

    def close(self):
        if self._batch is not None:
            self._batch.stdin.close()
            self._batch.wait()
            self._batch = None


class Linker:
    """Turns reference code spans into GitHub URLs; collects statistics and warnings."""

    def __init__(self, root, repo_url, audit_rev, build_rev="HEAD", branch="main"):
        self.branch = branch
        self.git = Git(root)
        self.repo = repo_url.rstrip("/")
        self.audit = self.git.full(audit_rev) or audit_rev
        self.build = self.git.full(build_rev) or build_rev
        self.stats = collections.Counter()
        self.warnings = []
        self._resolve_cache = {}

    def close(self):
        self.git.close()

    # ---------------------------------------------------------------- paths ---
    def resolve(self, path, doc_dir=None, rev=None):
        """Return (repo_path, revision) or None. doc_dir is the repo-relative directory of the
        document the reference is in (for ../ links and bare document names)."""
        rev = rev or self.audit
        key = (path, doc_dir, rev)
        if key in self._resolve_cache:
            return self._resolve_cache[key]
        res = self._resolve(path, doc_dir, rev)
        self._resolve_cache[key] = res
        return res

    def _resolve(self, path, doc_dir, rev):
        p = path.replace("\\", "/")
        cands = []
        if doc_dir is not None and (p.startswith("./") or p.startswith("../")):
            cands.append(posixpath.normpath(posixpath.join(doc_dir, p)))
        p = re.sub(r"^(?:\./)+", "", p)
        if p.startswith("../"):
            p = re.sub(r"^(?:\.\./)+", "", p)
        if p.startswith(SRC_PREFIX):
            cands.append(p)
        else:
            cands += [SRC_PREFIX + p, p]
            if doc_dir is not None:
                cands.append(posixpath.normpath(posixpath.join(doc_dir, p)))
                for base in ("doc/arkkitehtuuri", "doc/tietopohja"):
                    cands.append(posixpath.normpath(posixpath.join(base, p)))
        for c in cands:
            got = self._at(c, rev)
            if got:
                return got
        # unique suffix match among production sources, then the whole tree
        head = self.git.tree(self.build) | self.git.tree(rev)
        suffix = "/" + p.lower()
        for scope in (SRC_PREFIX.lower(), ""):
            hits = sorted({x for x in head if x.lower().startswith(scope) and x.lower().endswith(suffix)})
            if len(hits) == 1:
                return self._at(hits[0], rev)
            if len(hits) > 1:
                return None
        return None

    def _at(self, path, rev):
        """Pick the revision for an existing repo path (see the module docstring)."""
        if path.startswith(SRC_PREFIX):
            c = self.git.canonical(rev, path)
            if c:
                if rev != self.audit and self.git.full(rev) != self.audit:
                    ca = self.git.canonical(self.audit, c)
                    if ca and self.git.blob(self.audit, ca) == self.git.blob(rev, c):
                        return ca, self.audit
                return c, self.git.full(rev) or rev
            c = self.git.canonical(self.build, path)
            if c:
                return c, self.build
            return None
        c = self.git.canonical(self.build, path)
        if c:
            return c, self.build
        return None

    def url(self, path, rev, span=None, kind="blob"):
        # files resolved in the build commit (documents, tools, tests added after the audit) are
        # linked on the published branch, so the link survives any merge strategy; lines are
        # still checked against the build commit
        shown = self.branch if rev == self.build else rev
        u = "%s/%s/%s/%s" % (self.repo, kind, shown, path)
        if span:
            n = self.git.lines(rev, path)
            a, b = span
            if n is not None and b > n:
                self.stats["rivi_yli"] += 1
                self.warnings.append("%s@%s: line %d past end (%d lines)" % (path, rev[:7], b, n))
                return u
            if path.lower().endswith(".md"):
                u += "?plain=1"
            u += "#L%d" % a + ("-L%d" % b if b != a else "")
        return u

    # ------------------------------------------------------------ code spans ---
    def code(self, text, rev=None, last=None, doc_dir=None):
        """Return (url or None, new_last). `last` is the (path, rev) of the previous
        reference on the same line, for the bare follow-up form `:N`."""
        t = text.strip()
        m = BARE.match(t)
        if m:
            if last is None:
                return None, last
            span = first_span(m.group(1))
            self.stats["lahdelinkki"] += 1
            return self.url(last[0], last[1], span), last
        m = CODE_REF.match(t)
        if not m:
            return None, last
        path, spec = m.group(1), m.group(2)
        if "/" not in path and not spec and not path.lower().endswith((".md", ".csv", ".py", ".js")):
            return None, last  # a bare file name without lines is too ambiguous to link
        got = self.resolve(path, doc_dir, rev)
        if not got:
            if spec:
                self.stats["ratkaisematon"] += 1
            return None, None
        rpath, rrev = got
        span = first_span(spec) if spec else None
        self.stats["lahdelinkki"] += 1
        # a bare follow-up `:N` continues only a source-file reference that itself had lines
        # (`a.cpp:12`, `:15`); after a bare path or a document it is too ambiguous to link
        keep = (rpath, rrev) if spec and not rpath.lower().endswith((".md", ".csv", ".json", ".txt")) else None
        return self.url(rpath, rrev, span), keep
