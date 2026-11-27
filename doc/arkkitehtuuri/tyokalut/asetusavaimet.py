#!/usr/bin/env python3
"""C-12 part B0: string comparisons in the configuration parsers.

Lists every comparison of a variable with a string literal
(wmemcmpU/wcscmpU/wcsncmp/... (var, L"KEY"[, n])) in the given parser files,
with the enclosing function, the #if condition and per-product reachability
from the C-10 model (riippuvuudet.build_model), and the compared length.
It also lists prefix-shadowing candidates: within one function and one
variable, an earlier comparison whose compared prefix is a prefix of a later
literal (the later key may never be reached if both are in one if/else chain).

The output is raw evidence for part B; it does not decide which comparisons
are configuration keys.

Usage: python -I asetusavaimet.py <repo-root> <out-dir>
Writes: vertailut.csv, peittoehdokkaat.csv, kommentoidut.csv, yhteenveto.txt
"""
import csv
import importlib.util
import os
import re
import sys
from collections import Counter, defaultdict

sys.dont_write_bytecode = True   # do not leave __pycache__ in the repo
HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location("riippuvuudet", os.path.join(HERE, "riippuvuudet.py"))
RV = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(RV)

PARSERS = ["Hk/HkInit.cpp", "Juk/VInit.cpp", "cbHk/UnitCfg.cpp", "ViestiWin/UnitCfg.cpp"]
CMP = re.compile(r"\b(wmemcmpU|wcscmpU|wmemcmp|wcscmp|wcsncmp|_wcsnicmp|_wcsicmp|wcsicmp|memcmp|strcmp|"
                 r"strncmp|stricmp|_stricmp|strnicmp|_strnicmp|wcsncmpU|wcsstr|strstr)\s*\(\s*"
                 r"([^,()]+?)\s*,\s*L?\"((?:[^\"\\]|\\.)*)\"\s*(?:,\s*(\w+)\s*)?\)")
FUNC = re.compile(r"^(?:[\w:<>\*&~]+[ \t]+)+\**([\w:~]+)\s*\([^;]*$")


def main():
    root, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    m = RV.build_model(root)
    src = m.src
    rows = []
    for relp in PARSERS:
        p = m.all_files.get(RV.norm(os.path.join(src, relp)).lower())
        fi = m.cache.get(p)
        func = ""
        for i, line in enumerate(fi.lines, 1):
            fm = FUNC.match(line)
            if fm and not line.startswith((" ", "\t")) and not line.rstrip().endswith(";"):
                func = fm.group(1)
            for mm in CMP.finditer(line):
                fn, var, lit, n = mm.groups()
                r = m.reach(p, i)
                rows.append(dict(file=relp, line=i, function=func, cmp=fn, var=var.strip(), literal=lit,
                                 n=n or "", n_vs_len=("" if not n or not n.isdigit() else
                                                      ("=" if int(n) == len(lit) else
                                                       ("<" if int(n) < len(lit) else ">"))),
                                 condition=fi.cond[i], **{"act_" + k: m.summarize(v) for k, v in r.items()},
                                 text=fi.raw_lines[i - 1].strip()[:160]))
    # comparisons that exist only in comments (disabled key handlers)
    commented = []
    for relp in PARSERS:
        p = m.all_files.get(RV.norm(os.path.join(src, relp)).lower())
        fi = m.cache.get(p)
        for i, raw in enumerate(fi.raw_lines, 1):
            if fi.lines[i - 1].strip() == raw.strip():
                continue
            cl = set(x.groups() for x in CMP.finditer(fi.lines[i - 1]))
            for mm in CMP.finditer(raw):
                if mm.groups() not in cl:
                    commented.append([relp, i, mm.group(2).strip(), mm.group(3), mm.group(4) or "",
                                      raw.strip()[:160]])
    with open(os.path.join(out, "kommentoidut.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["file", "line", "var", "literal", "n", "text"])
        w.writerows(commented)

    cols = ["file", "line", "function", "cmp", "var", "literal", "n", "n_vs_len", "condition"] + \
           ["act_" + k for k in m.pnames] + ["text"]
    with open(os.path.join(out, "vertailut.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=cols)
        w.writeheader()
        w.writerows(rows)

    # prefix shadowing candidates
    groups = defaultdict(list)
    for r in rows:
        groups[(r["file"], r["function"], r["var"])].append(r)
    sh = []
    for key, lst in groups.items():
        for i, a in enumerate(lst):
            pref = a["literal"]
            if a["n"].isdigit():
                pref = pref[:int(a["n"])]
            elif a["cmp"] in ("wcscmpU", "wcscmp", "strcmp", "_wcsicmp", "wcsicmp", "stricmp", "_stricmp"):
                continue   # full comparison: no prefix match
            if not pref:
                continue
            for b in lst[i + 1:]:
                if b["literal"] != a["literal"] and b["literal"].upper().startswith(pref.upper()):
                    sh.append([key[0], key[1], key[2], a["line"], a["literal"], a["n"], b["line"], b["literal"],
                               b["n"], b["line"] - a["line"]])
    with open(os.path.join(out, "peittoehdokkaat.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["file", "function", "var", "first_line", "first_literal", "first_n", "later_line",
                    "later_literal", "later_n", "distance"])
        w.writerows(sh)

    with open(os.path.join(out, "yhteenveto.txt"), "w", encoding="utf-8") as f:
        f.write("C-12 part B0 parser comparisons (asetusavaimet.py)\n\n")
        for relp in PARSERS:
            rr = [r for r in rows if r["file"] == relp]
            f.write("%s: %d comparisons, distinct literals %d\n" % (relp, len(rr), len(set(r["literal"] for r in rr))))
            f.write("  by var: %s\n" % dict(Counter(r["var"] for r in rr).most_common(12)))
            f.write("  by function: %s\n" % dict(Counter(r["function"] for r in rr).most_common(12)))
            f.write("  n vs literal length: %s\n" % dict(Counter(r["n_vs_len"] for r in rr)))
        f.write("\nprefix shadowing candidates: %d\n" % len(sh))
        f.write("commented-out comparisons: %s\n" % dict(Counter(c[0] for c in commented)))


if __name__ == "__main__":
    main()
