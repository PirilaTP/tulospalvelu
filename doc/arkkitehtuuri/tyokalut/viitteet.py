#!/usr/bin/env python3
"""Source references of the register, checked against their pinned revision (read-only).

Usage (from the repository root):
    python -I doc/arkkitehtuuri/tyokalut/viitteet.py . [--raportti <out.csv>] [--kaikki]

For every H and T row of rekisteri/havainnot.md the `path:line` references in the
evidence column are read together with the row's Rev column. Paths are relative to
TPsource/V52 (as in the register).

1. Pinned check: the file exists in the pinned revision (git show <rev>:<path>) and
   every cited line is within it. A failure is an ERROR (exit code 1).
2. Drift report: the first cited line in the pinned revision is compared with the
   current working tree file, byte for byte (no decoding, so cp1252 and UTF-8 files
   are compared alike):
     ennallaan   same line number, same content
     siirtynyt   same content found exactly once elsewhere (new line number given)
     muuttunut   content not found, or found several times (ambiguous)
     poistettu   file no longer in the working tree
   Drift is information, not an error: the reference stays valid for its revision.
   By default only rows whose state is 'avoin' are reported (riskiarvio.csv and
   tekninen-velka.csv 'tila'); --kaikki reports every row.

Nothing is written unless --raportti is given. No network, no changes to the repo.
"""
import csv
import io
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
sys.dont_write_bytecode = True  # keep the repository free of __pycache__
import synteesi  # noqa: E402  (register parser shared with the synthesis script)

PATH_RE = re.compile(r"^([A-Za-z0-9_]+(?:/[A-Za-z0-9_.-]+)+\.(?:cpp|c|h|hpp|H|dfm|cbproj|vcxproj))(?::(~?[0-9][0-9, ~-]*))?$")
SPEC_RE = re.compile(r"^:(~?[0-9][0-9, ~-]*)$")
SRC_PREFIX = "TPsource/V52/"


def refs_of(evidence):
    """[(path, [line, ...])] in order; a bare `:N` belongs to the previous path."""
    out = []
    cur = None
    for tok in re.findall(r"`([^`]+)`", evidence):
        m = PATH_RE.match(tok)
        if m:
            cur = m.group(1)
            out.append((cur, lines_of(m.group(2))))
            continue
        m = SPEC_RE.match(tok)
        if m and cur:
            out.append((cur, lines_of(m.group(1))))
    return out


def lines_of(spec):
    if not spec:
        return []
    res = []
    for part in spec.replace("~", "").split(","):
        nums = [int(x) for x in re.findall(r"\d+", part)]
        if len(nums) == 1:
            res.append(nums[0])
        elif len(nums) >= 2:
            res += [nums[0], nums[1]]
    return res


class Git:
    def __init__(self, root):
        self.root = root
        self.cache = {}

    def show(self, rev, path):
        key = (rev, path)
        if key not in self.cache:
            p = subprocess.run(["git", "-C", self.root, "show", "%s:%s" % (rev, SRC_PREFIX + path)],
                               stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
            self.cache[key] = p.stdout.split(b"\n") if p.returncode == 0 else None
        return self.cache[key]


def states(doc):
    st = {}
    for name in ("riskiarvio.csv", "tekninen-velka.csv"):
        p = os.path.join(doc, "rekisteri", name)
        if os.path.exists(p):
            for r in csv.DictReader(io.open(p, encoding="utf-8", newline="")):
                st[r["id"]] = r.get("tila", "avoin")
    return st


def main(argv):
    root = argv[1] if len(argv) > 1 else "."
    doc = os.path.join(root, "doc", "arkkitehtuuri")
    report = argv[argv.index("--raportti") + 1] if "--raportti" in argv else None
    every = "--kaikki" in argv
    hrows, trows = synteesi.parse_register(doc)
    st = states(doc)
    git = Git(root)
    errors, rows, counts = [], [], {}
    current = {}
    nref = 0
    for r in hrows + trows:
        c = r["_cells"]
        evidence = c[2]
        rev = r["rev"]
        for path, lines in refs_of(evidence):
            nref += 1
            old = git.show(rev, path)
            if old is None:
                errors.append("%s: %s not in revision %s" % (r["id"], path, rev))
                continue
            n_old = len(old) if old[-1] != b"" else len(old) - 1
            for ln in lines:
                if ln < 1 or ln > n_old:
                    errors.append("%s: %s:%d beyond end of %s at %s (%d lines)" % (r["id"], path, ln, path, rev, n_old))
            if not lines or (not every and st.get(r["id"], "avoin") != "avoin"):
                continue
            first = lines[0]
            if first > n_old:
                continue
            if path not in current:
                fp = os.path.join(root, SRC_PREFIX, path)
                current[path] = open(fp, "rb").read().split(b"\n") if os.path.exists(fp) else None
            cur = current[path]
            want = old[first - 1].rstrip(b"\r")
            if cur is None:
                state, new = "poistettu", ""
            elif first <= len(cur) and cur[first - 1].rstrip(b"\r") == want:
                state, new = "ennallaan", first
            else:
                hits = [i + 1 for i, l in enumerate(cur) if l.rstrip(b"\r") == want]
                state, new = ("siirtynyt", hits[0]) if len(hits) == 1 else ("muuttunut", "")
            counts[state] = counts.get(state, 0) + 1
            rows.append([r["id"], rev, path, first, state, new])
    print("references: %d in %d rows; pinned check errors: %d" % (nref, len(hrows) + len(trows), len(errors)))
    print("drift (%s rows): %s" % ("all" if every else "open", ", ".join("%s %d" % kv for kv in sorted(counts.items()))))
    for row in rows:
        if row[4] != "ennallaan":
            print("  %s %s:%s -> %s %s" % (row[0], row[2], row[3], row[4], row[5]))
    if report:
        out = io.StringIO(newline="")
        w = csv.writer(out, lineterminator="\n")
        w.writerow(["id", "rev", "tiedosto", "rivi", "tila", "uusi_rivi"])
        w.writerows(rows)
        with open(report, "wb") as fh:
            fh.write(out.getvalue().encode("utf-8"))
    for e in errors:
        print("ERROR", e)
    print("OK" if not errors else "%d errors" % len(errors))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
