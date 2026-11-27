#!/usr/bin/env python3
"""Baseline of review coverage per function (read-only; C-14 planning tool).

Usage (from the repository root):
    python -I doc/arkkitehtuuri/tyokalut/kattavuus.py . <out_dir>

What it does:
1. Lists the functions of every .c/.cpp/.h/.hpp file in rekisteri/tiedostot.csv with a
   heuristic brace scanner (comments and literals blanked, preprocessor branches handled
   like ctags: every #if branch starts from the depth at #if, the first branch wins).
   It also records '#if 0' regions. It is not a C++ parser: see the limits below.
2. Collects every source reference 'path:line', 'path:a-b', 'path:a, b' and the bare
   follow-up form '`:N`' (attributed to the previous path on the same line) from the
   analysis documents: doc/arkkitehtuuri/*.md, rekisteri/havainnot.md, todisteet/**/*.md,
   doc/tietopohja/**/*.md. Raw tool output (logs, json, txt, generated csv) is not used.
   The C-4 warning triage files (todisteet/c4/triage_*_luokitus.csv) are counted as a
   separate source class, because a triaged warning is a one-line look, not a reading.
3. Maps each reference to the function whose span contains it. A range longer than
   WIDE lines is a region mention and does not mark functions.
4. Classifies files (product via projects, libraries and the #include closure; test;
   orphan; third party by its header) and marks a function inactive in a product when
   it lies wholly in a preprocessor branch that C-12 found not taken there
   (todisteet/c12/osaA0/testikohdat.csv).
5. Pairs same-named functions of the HK and relay side (core, console, GUI) and gives a
   line similarity of their bodies (parit.csv).
6. Writes funktiot.csv, tiedostot.csv, hakemistot.csv, parit.csv and
   ratkaisemattomat.csv to <out_dir>. Nothing in the repository is changed.

Limits: a heuristic scanner (macros that expand to braces, function-like macros and
unusual declarators can mislead it; preprocessor blocks whose branches disagree on braces
are resolved by preferring a balanced branch and counted); paths cited only by a base
name that two files share are left unresolved; a cited line is assumed to have been read
with CONTEXT lines around it.

A reference shows that someone looked at that place for some claim. It does not show
that the function was reviewed. The output is therefore an upper bound of "read at
least once", never a review state.

Production sources are byte-identical between the audit base f7fe553 and HEAD
(only Tests/ and VS/Tests/ changed), so references of any audit revision map to HEAD.
"""
import collections
import csv
import io
import os
import re
import sys

WIDE = 150
CONTEXT = 10  # lines around a cited line assumed to have been read with it
ROOT = os.path.abspath(sys.argv[1] if len(sys.argv) > 1 else ".")
OUT = sys.argv[2] if len(sys.argv) > 2 else None
SRC = os.path.join(ROOT, "TPsource", "V52")
DOC = os.path.join(ROOT, "doc", "arkkitehtuuri")
KB = os.path.join(ROOT, "doc", "tietopohja")
CODE_EXT = (".c", ".cpp", ".h", ".hpp")

# perspective of a source document (C batch or topic report)
TOPIC_DOCS = {
    "tallenna-samanaikaisuus.md": "C-1 tallennus/rinnakkaisuus",
    "tiedonsiirto.md": "C-2 tiedonsiirto",
    "tiedostomuodot.md": "C-3 tiedostot/eheys",
    "muistiturvallisuus.md": "C-5 muistiturvallisuus",
    "saikeet-ja-lukot.md": "C-6 rinnakkaisuus",
    "gui-liiketoimintalogiikka.md": "C-7 GUI-logiikka",
    "globaali-tila.md": "C-8 tila/elinkaari",
    "tietorakenteet-ja-tila.md": "C-8 tila/elinkaari",
    "virheenkasittely.md": "C-9 virheenkasittely",
    "riippuvuudet.md": "C-10 riippuvuudet/tietoturva",
    "testaus.md": "C-11 testaus",
    "regressiotestaussuunnitelma.md": "C-13 synteesi",
    "ominaisuusvalitsimet.md": "C-12 valitsimet/asetukset",
    "kaantajaerot.md": "C-5 kaantajaerot",
    "merkistot.md": "C-0 merkistot",
}
BATCH_DIRS = {
    "c4": "C-4 varoitukset", "c5": "C-5 muistiturvallisuus", "c6": "C-6 rinnakkaisuus",
    "c7": "C-7 GUI-logiikka", "c8": "C-8 tila/elinkaari", "c9": "C-9 virheenkasittely",
    "c10": "C-10 riippuvuudet/tietoturva", "c11": "C-11 testaus", "c12": "C-12 valitsimet/asetukset",
    "c13": "C-13 synteesi", "k0": "K-0",
}
# perspectives that are not a defect perspective of their own (synthesis, derivatives)
DERIVED = {"C-13 synteesi", "K tietopohja", "rekisteri", "muu dokumentti", "K-0"}


def perspective(path):
    rel = os.path.relpath(path, ROOT).replace(os.sep, "/")
    if rel.startswith("doc/tietopohja/"):
        return "K tietopohja"
    if rel == "doc/arkkitehtuuri/rekisteri/havainnot.md":
        return "rekisteri"
    m = re.match(r"doc/arkkitehtuuri/todisteet/([a-z0-9]+)/", rel)
    if m:
        if m.group(1) == "c4" and rel.endswith(".csv"):
            return "C-4 varoitusseulonta (rivi)"
        return BATCH_DIRS.get(m.group(1), "muu dokumentti")
    return TOPIC_DOCS.get(os.path.basename(rel), "muu dokumentti")


def source_documents():
    docs = []
    for f in sorted(os.listdir(DOC)):
        if f.endswith(".md"):
            docs.append(os.path.join(DOC, f))
    docs.append(os.path.join(DOC, "rekisteri", "havainnot.md"))
    for base in (os.path.join(DOC, "todisteet"), KB):
        for d, dn, fn in os.walk(base):
            dn.sort()
            for f in sorted(fn):
                p = os.path.join(d, f)
                if f.endswith(".md"):
                    docs.append(p)
                elif re.match(r"triage_.*_luokitus\.csv$", f):
                    docs.append(p)
    return docs


# ---------------------------------------------------------------- function scanner

def blank_comments_and_literals(t):
    out = []
    i, n = 0, len(t)
    while i < n:
        c = t[i]
        if c == "/" and i + 1 < n and t[i + 1] == "/":
            j = t.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif c == "/" and i + 1 < n and t[i + 1] == "*":
            j = t.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(re.sub(r"[^\n]", " ", t[i:j]))
            i = j
        elif c in "\"'":
            j = i + 1
            while j < n and t[j] != c and t[j] != "\n":
                j += 2 if t[j] == "\\" else 1
            j = min(j + 1, n)
            out.append(c + re.sub(r"[^\n]", " ", t[i + 1:j - 1]) + (c if j - 1 > i else ""))
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


NOT_NAMES = {"if", "while", "for", "switch", "return", "sizeof", "catch", "__declspec", "__attribute__",
             "defined", "PACKAGE", "__property", "throw", "__published"}
TYPE_KEYS = re.compile(r"^\s*(?:typedef\s+)?(?:struct|class|union|enum|namespace)\b")
QUAL_TAIL = re.compile(r"^\s*(?:const\b|volatile\b|throw\s*\([^)]*\)|__fastcall\b|override\b|\s)*$")


def function_name(header):
    """Return the function name if the text before '{' is a function header, else None."""
    h = header.strip()
    if not h or "(" not in h or TYPE_KEYS.match(h) or h.startswith("extern \"") or h.endswith("="):
        return None
    if re.search(r"=\s*$", h) or re.match(r"^\s*(?:else|do|try)\b", h):
        return None
    # top-level paren groups
    groups, depth, start = [], 0, None
    for k, ch in enumerate(h):
        if ch == "(":
            if depth == 0:
                start = k
            depth += 1
        elif ch == ")" and depth:
            depth -= 1
            if depth == 0:
                groups.append((start, k))
    for s, e in groups:
        m = re.search(r"((?:[A-Za-z_]\w*\s*::\s*)*~?\s*(?:operator\s*[^\s(]+|[A-Za-z_]\w*))\s*$", h[:s])
        if not m:
            continue
        name = re.sub(r"\s+", "", m.group(1))
        last = name.split("::")[-1]
        if last in NOT_NAMES:
            continue
        before = h[:s - len(m.group(1))].rstrip()
        if before.endswith("=") or before.endswith("return"):
            return None
        tail = h[e + 1:]
        if QUAL_TAIL.match(tail) or re.match(r"^\s*:", tail):
            return name
        return None
    return None


def scan_functions(text):
    """Yield (name, first_line, last_line) and return '#if 0' ranges, 1-based lines."""
    t = blank_comments_and_literals(text)
    lines = t.split("\n")
    funcs, dead, asym = [], [], []
    depth = 0
    pp = []  # stack of [depth_at_if, finished branch states, is_if0, start line, state at #if]
    scopes = []  # stack of (kind, depth_before) kind in file/ns/class/func/other
    header = []  # text since last ; { } at a definable scope
    header_line = None
    func_stack = []
    cont = False
    for ln, line in enumerate(lines, 1):
        s = line.strip()
        if cont or s.startswith("#"):
            cont = s.endswith("\\")
            d = re.match(r"#\s*(\w+)\s*(.*)", s)
            if d:
                kw, arg = d.group(1), d.group(2).strip()
                if kw in ("if", "ifdef", "ifndef"):
                    pp.append([depth, [], kw == "if" and re.match(r"0\b", arg) is not None, ln,
                               (list(scopes), [list(f) for f in func_stack])])
                elif kw in ("elif", "else") and pp:
                    top = pp[-1]
                    top[1].append((depth, list(scopes), [list(f) for f in func_stack]))
                    if top[2]:
                        dead.append((top[3], ln))
                        top[2] = False
                    depth = top[0]
                    scopes[:] = list(top[4][0])
                    func_stack[:] = [list(f) for f in top[4][1]]
                elif kw == "endif" and pp:
                    top = pp.pop()
                    if top[2]:
                        dead.append((top[3], ln))
                    if top[1]:
                        states = top[1] + [(depth, list(scopes), [list(f) for f in func_stack])]
                        nets = [st[0] - top[0] for st in states]
                        pick = states[0]
                        if len(set(nets)) > 1:
                            # branches disagree: one is usually dead or broken; prefer a balanced one
                            asym.append(top[3])
                            if 0 in nets:
                                pick = states[nets.index(0)]
                        depth = pick[0]
                        scopes[:] = pick[1]
                        func_stack[:] = pick[2]
            continue
        for ch in line:
            in_func = any(k == "func" for k, _ in scopes)
            if ch == "{":
                kind = "other"
                if not in_func:
                    htxt = "".join(header)
                    name = function_name(htxt)
                    if name:
                        kind = "func"
                        func_stack.append([name, header_line or ln, None, depth])
                    elif re.match(r"^\s*(?:namespace\b|extern\s+\"C)", htxt):
                        kind = "ns"
                    elif TYPE_KEYS.match(htxt) or re.match(r"^\s*(?:template\s*<[^>]*>\s*)?(?:class|struct|union)\b", htxt):
                        kind = "class"
                scopes.append((kind, depth))
                depth += 1
                header, header_line = [], None
            elif ch == "}":
                if scopes:
                    kind, d0 = scopes.pop()
                    if kind == "func" and func_stack:
                        f = func_stack.pop()
                        funcs.append((f[0], f[1], ln))
                depth = max(depth - 1, 0)
                header, header_line = [], None
            elif ch == ";":
                if not in_func:
                    header, header_line = [], None
            else:
                if not in_func:
                    if not header and not ch.isspace():
                        header_line = ln
                    if header or not ch.isspace():
                        header.append(ch)
        if not in_func_any(scopes):
            header.append(" ")
    scan_functions.asym = asym
    return funcs, dead


def in_func_any(scopes):
    return any(k == "func" for k, _ in scopes)


# ---------------------------------------------------------------- references

PATH = r"(?:TPsource/V52/)?((?:[A-Za-z0-9_]+/)*[A-Za-z0-9_]+\.(?:cpp|c|hpp|h|H))"
REF = re.compile(PATH + r"\s*:\s*(\d+(?:\s*[-–]\s*\d+)?(?:\s*,\s*:?\d+(?:\s*[-–]\s*\d+)?)*)")
BARE = re.compile(r"`:(\d+(?:\s*[-–]\s*\d+)?)`")


def parse_spans(spec):
    out = []
    for part in re.split(r"\s*,\s*", spec):
        part = part.lstrip(":")
        m = re.match(r"(\d+)(?:\s*[-–]\s*(\d+))?$", part.strip())
        if m:
            a = int(m.group(1))
            b = int(m.group(2)) if m.group(2) else a
            if b < a:
                a, b = b, a
            out.append((a, b))
    return out


def collect_refs(files_by_rel, by_base):
    refs = []  # (file_rel, a, b, perspective, doc_rel)
    unresolved = collections.Counter()
    for p in source_documents():
        try:
            text = io.open(p, encoding="utf-8", errors="replace").read()
        except OSError:
            continue
        persp = perspective(p)
        drel = os.path.relpath(p, ROOT).replace(os.sep, "/")
        for line in text.split("\n"):
            last = None
            pos = 0
            items = []
            for m in REF.finditer(line):
                items.append((m.start(), m.group(1), m.group(2)))
            for m in BARE.finditer(line):
                items.append((m.start(), None, m.group(1)))
            for start, path, spec in sorted(items, key=lambda x: x[0]):
                if path is not None:
                    rel = path if path in files_by_rel else None
                    if rel is None:
                        cands = by_base.get(os.path.basename(path).lower(), [])
                        tail = [c for c in cands if c.endswith("/" + path) or c == path]
                        if len(tail) == 1:
                            rel = tail[0]
                        elif len(cands) == 1 and "/" not in path:
                            rel = cands[0]
                    if rel is None:
                        unresolved[path] += 1
                        last = None
                        continue
                    last = rel
                elif last is None:
                    continue
                for a, b in parse_spans(spec):
                    refs.append((last, a, b, persp, drel))
    return refs, unresolved


# ---------------------------------------------------------------- file classes

PRODUCT_OF = {"HkMaali": ["HkMaali"], "JukMaali520": ["ViestiMaali"], "JukLuenta520": ["ViestiLuenta"],
              "HkKisaWin": ["HkKisaWin"], "ViestiWin": ["ViestiWin"],
              "tputilv2": ["HkMaali", "ViestiMaali", "ViestiLuenta"], "dbboxm": ["HkMaali", "ViestiMaali", "ViestiLuenta"],
              "Tputil-XE": ["HkKisaWin", "ViestiWin"], "DBboxm-XE": ["HkKisaWin", "ViestiWin"]}
PRODUCTS = ["HkMaali", "ViestiMaali", "ViestiLuenta", "HkKisaWin", "ViestiWin"]
TEST_PROJECTS = {"TpTest", "TpCoreHk", "TpCoreJuk"}
THIRD_PARTY = re.compile(rb"Blaise Computing|Microsoft Source Code Samples|Makoto Matsumoto|doctest is a")
INCLUDE = re.compile(r'^\s*#\s*include\s*["<]([^">]+)[">]', re.M)


def classify(files, by_base):
    """Products per file (projects, libraries and #include closure) and a class per file."""
    prods = {k: set() for k in files}
    tests = {k: False for k in files}
    incl = {}
    for rel, r in files.items():
        for pr in r["projektit"].split():
            prods[rel].update(PRODUCT_OF.get(pr, []))
            if pr in TEST_PROJECTS:
                tests[rel] = True
        p = os.path.join(SRC, rel.replace("/", os.sep))
        try:
            t = open(p, "rb").read().decode("latin-1")
        except OSError:
            t = ""
        out = []
        for m in INCLUDE.finditer(t):
            name = m.group(1).replace("\\", "/")
            cands = [c for c in by_base.get(os.path.basename(name).lower(), [])]
            near = [c for c in cands if os.path.dirname(c) == os.path.dirname(rel)]
            out.extend(near or cands)
        incl[rel] = out
    # propagate products and test use through includes until stable
    changed = True
    while changed:
        changed = False
        for rel in files:
            for h in incl[rel]:
                if not prods[rel] <= prods[h]:
                    prods[h] |= prods[rel]
                    changed = True
                if tests[rel] and not tests[h]:
                    tests[h] = True
                    changed = True
    cls = {}
    for rel in files:
        p = os.path.join(SRC, rel.replace("/", os.sep))
        head = open(p, "rb").read(4000) if os.path.exists(p) else b""
        if THIRD_PARTY.search(head):
            cls[rel] = "kolmas_osapuoli"
        elif rel.startswith("Tests/") or rel.startswith("VS/Tests"):
            cls[rel] = "testi"
        elif prods[rel]:
            cls[rel] = "tuote"
        elif tests[rel]:
            cls[rel] = "vain_testi"
        else:
            cls[rel] = "orpo"
    return prods, cls


def inactive_ranges():
    """Per file and product the line ranges of preprocessor branches not taken (C-12 site table)."""
    p = os.path.join(DOC, "todisteet", "c12", "osaA0", "testikohdat.csv")
    out = collections.defaultdict(lambda: collections.defaultdict(list))
    unknown = collections.defaultdict(list)
    for r in csv.DictReader(io.open(p, encoding="utf-8")):
        a = int(r["line"]) + 1
        b = int(r["line"]) + int(r["body_lines"] or 0)
        if b < a:
            continue
        for pr in PRODUCTS:
            reach, take = r["reach_" + pr], r["take_" + pr]
            if reach == "n" or (reach == "y" and take == "n"):
                out[r["file"]][pr].append((a, b))
            elif reach == "y" and take not in ("y", "n"):
                unknown[r["file"]].append((a, b))
    return out, unknown


# ---------------------------------------------------------------- main

def main():
    reg = list(csv.DictReader(io.open(os.path.join(DOC, "rekisteri", "tiedostot.csv"), encoding="utf-8")))
    files = {r["polku"]: r for r in reg}
    by_base = collections.defaultdict(list)
    for k in files:
        by_base[os.path.basename(k).lower()].append(k)
    funcs = {}
    dead = {}
    asym_total = []
    for rel, r in files.items():
        if not rel.lower().endswith(CODE_EXT):
            continue
        p = os.path.join(SRC, rel.replace("/", os.sep))
        if not os.path.exists(p):
            continue
        text = open(p, "rb").read().decode("latin-1").replace("\r\n", "\n").replace("\r", "\n")
        fs, dd = scan_functions(text)
        asym_total.extend((rel, x) for x in scan_functions.asym)
        funcs[rel] = [{"name": n, "a": a, "b": b, "refs": 0, "persp": set(), "docs": set()} for n, a, b in fs]
        dead[rel] = dd
    refs, unresolved = collect_refs(files, by_base)
    prods, fclass = classify(files, by_base)
    inact, unk = inactive_ranges()
    wide = 0
    file_refs = collections.Counter()
    file_persp = collections.defaultdict(set)
    outside = collections.Counter()
    for rel, a, b, persp, drel in refs:
        file_refs[rel] += 1
        file_persp[rel].add(persp)
        if b - a > WIDE:
            wide += 1
            continue
        hit = False
        for f in funcs.get(rel, []):
            if f["a"] <= b and a <= f["b"]:
                f["refs"] += 1
                f.setdefault("span", set()).update(range(max(f["a"], a - CONTEXT), min(f["b"], b + CONTEXT) + 1))
                f["persp"].add(persp)
                f["docs"].add(drel)
                hit = True
        if not hit:
            outside[rel] += 1

    if OUT:
        os.makedirs(OUT, exist_ok=True)
        with open(os.path.join(OUT, "funktiot.csv"), "w", encoding="utf-8", newline="") as fh:
            w = csv.writer(fh, lineterminator="\n")
            w.writerow(["polku", "funktio", "alku", "loppu", "rivit", "tiedostoluokka", "tuotteet", "taso",
                        "if0", "aktiivinen", "ehto_tuntematon", "viitteet", "viitealueen_rivit", "nakokulmat", "virhenakokulmia", "luokka"])
            for rel in sorted(funcs):
                for f in funcs[rel]:
                    own = sorted(x for x in f["persp"] if x not in DERIVED)
                    cls = "ei_viitteita" if not f["refs"] else ("vain_johdettu" if not own else
                                                                ("yksi_nakokulma" if len(own) == 1 else "useita"))
                    in0 = any(a <= f["a"] <= b for a, b in dead[rel])
                    act = [x for x in PRODUCTS if x in prods[rel]
                           and not any(a <= f["a"] and f["b"] <= b for a, b in inact[rel][x])]
                    unsure = any(a <= f["a"] and f["b"] <= b for a, b in unk[rel])
                    w.writerow([rel, f["name"], f["a"], f["b"], f["b"] - f["a"] + 1, fclass[rel],
                                " ".join(x for x in PRODUCTS if x in prods[rel]), files[rel]["analyysitaso"],
                                int(in0), " ".join(act), int(unsure), f["refs"], len(f.get("span", ())), "; ".join(sorted(f["persp"])), len(own), cls])
    # summary
    by_dir = collections.OrderedDict()
    tot = collections.Counter()
    for rel in sorted(funcs):
        r = files[rel]
        d = r["hakemisto"]
        c = by_dir.setdefault(d, collections.Counter())
        for f in funcs[rel]:
            ln = f["b"] - f["a"] + 1
            own = [x for x in f["persp"] if x not in DERIVED]
            key = "ei" if not f["refs"] else ("joh" if not own else ("yksi" if len(own) == 1 else "usea"))
            for cc in (c, tot):
                cc["f"] += 1
                cc["l"] += ln
                cc["f_" + key] += 1
                cc["l_" + key] += ln
        for cc in (c, tot):
            cc["dead"] += sum(b - a + 1 for a, b in dead[rel])
    print("file classes:", dict(collections.Counter(fclass.values())))
    print("files scanned %d, functions %d, function lines %d" % (len(funcs), tot["f"], tot["l"]))
    print("preprocessor blocks whose branches disagree on braces: %d in %d files" % (
        len(asym_total), len(set(r for r, _ in asym_total))))
    print("references %d (wide %d, outside any function %d), unresolved paths %d" % (
        len(refs), wide, sum(outside.values()), sum(unresolved.values())))
    for k in ("ei", "joh", "yksi", "usea"):
        print("  %-5s functions %5d  lines %7d" % (k, tot["f_" + k], tot["l_" + k]))
    if OUT:
        with open(os.path.join(OUT, "hakemistot.csv"), "w", encoding="utf-8", newline="") as fh:
            w = csv.writer(fh, lineterminator="\n")
            w.writerow(["hakemisto", "funktiot", "funktiorivit", "ei_viitteita_f", "ei_viitteita_r",
                        "vain_johdettu_f", "yksi_nakokulma_f", "yksi_nakokulma_r", "useita_f", "useita_r", "if0_rivit"])
            for d, c in list(by_dir.items()) + [("YHTEENSA", tot)]:
                w.writerow([d, c["f"], c["l"], c["f_ei"], c["l_ei"], c["f_joh"], c["f_yksi"], c["l_yksi"],
                            c["f_usea"], c["l_usea"], c["dead"]])
        with open(os.path.join(OUT, "tiedostot.csv"), "w", encoding="utf-8", newline="") as fh:
            w = csv.writer(fh, lineterminator="\n")
            w.writerow(["polku", "rivit", "tiedostoluokka", "tuotteet", "projektit", "taso", "funktiot", "funktiot_viitteella", "viitteet",
                        "viitteet_funktioiden_ulkopuolella", "nakokulmat", "if0_rivit"])
            for rel in sorted(funcs):
                fs = funcs[rel]
                w.writerow([rel, files[rel]["rivit"], fclass[rel], " ".join(x for x in PRODUCTS if x in prods[rel]),
                            files[rel]["projektit"], files[rel]["analyysitaso"], len(fs),
                            sum(1 for f in fs if f["refs"]), file_refs[rel], outside[rel],
                            "; ".join(sorted(file_persp[rel])), sum(b - a + 1 for a, b in dead[rel])])
        write_pairs(funcs, OUT)
        with open(os.path.join(OUT, "ratkaisemattomat.csv"), "w", encoding="utf-8", newline="") as fh:
            w = csv.writer(fh, lineterminator="\n")
            w.writerow(["polku", "viitteet"])
            for k, v in unresolved.most_common():
                w.writerow([k, v])
    return 0


# ---------------------------------------------------------------- HK / relay pairs

PAIR_SIDE = {"Hk": ("ydin", "HK"), "Juk": ("ydin", "viesti"), "HkConsole": ("konsoli", "HK"),
             "JukConsole": ("konsoli", "viesti"), "cbHk": ("GUI", "HK"), "ViestiWin": ("GUI", "viesti")}


def write_pairs(funcs, out):
    """Same-named functions on the HK and relay side of each layer, with a line-level similarity."""
    import difflib
    side = {"HK": collections.defaultdict(list), "viesti": collections.defaultdict(list)}
    cache = {}
    for rel, fs in funcs.items():
        d = rel.split("/")[0]
        if d not in PAIR_SIDE:
            continue
        layer, s = PAIR_SIDE[d]
        for f in fs:
            side[s][(layer, f["name"])].append((rel, f))

    def body(rel, f):
        if rel not in cache:
            t = open(os.path.join(SRC, rel.replace("/", os.sep)), "rb").read().decode("latin-1")
            cache[rel] = blank_comments_and_literals(t.replace("\r\n", "\n")).split("\n")
        return [re.sub(r"\s+", " ", x).strip() for x in cache[rel][f["a"] - 1:f["b"]] if x.strip()]

    with open(os.path.join(out, "parit.csv"), "w", encoding="utf-8", newline="") as fh:
        w = csv.writer(fh, lineterminator="\n")
        w.writerow(["kerros", "funktio", "hk_polku", "hk_alku", "hk_rivit", "viesti_polku", "viesti_alku",
                    "viesti_rivit", "samankaltaisuus", "luokka", "monikko"])
        for key in sorted(side["HK"]):
            if key not in side["viesti"]:
                continue
            (ra, fa), (rb, fb) = side["HK"][key][0], side["viesti"][key][0]
            ba, bb = body(ra, fa), body(rb, fb)
            ratio = difflib.SequenceMatcher(None, ba[:800], bb[:800], autojunk=False).ratio()
            cls = ("identtinen" if ratio >= .98 else "lahes_sama" if ratio >= .8 else
                   "eriytynyt" if ratio >= .4 else "eri_toteutus")
            many = int(len(side["HK"][key]) > 1 or len(side["viesti"][key]) > 1)
            w.writerow([key[0], key[1], ra, fa["a"], fa["b"] - fa["a"] + 1, rb, fb["a"], fb["b"] - fb["a"] + 1,
                        "%.3f" % ratio, cls, many])


if __name__ == "__main__":
    sys.exit(main())
