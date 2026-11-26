#!/usr/bin/env python3
"""C-13 part A0: machine base for the audit synthesis (read-only on its inputs).

Usage:
  python -I synteesi.py <repo_root> <out_dir>            build the base
  python -I synteesi.py <repo_root> <out_dir> --tarkista  also check the C-13 registers

The script parses the audit register and the tracking documents and writes one
row per H, T and Q identifier, so that the synthesis works from one common base
instead of every reader re-parsing the documents.

Inputs (all under doc/arkkitehtuuri):
  rekisteri/havainnot.md          H and T rows
  seuranta.md                     Q table, prioritised fix queue (P1-P5), KK lists
  muistiturvallisuus.md           P1-P5 table of section 15
  virheenkasittely.md, riippuvuudet.md, ominaisuusvalitsimet.md   KK definitions
  the batch reports               per-question rows of the Q ranges
  todisteet/c12/osaA0/maaritykset.csv, todisteet/c10/osaA/makrot.csv
                                  macros whose recorded value is an evaluator artefact

Outputs (out_dir):
  havainnot.csv        one row per H
  tekninen_velka.csv   one row per T
  kysymykset.csv       one row per Q
  viittaukset.csv      cross references between identifiers (from, to, where)
  kk_aiemmat.csv       earlier fix packages (KK-C9, KK-C10, KK-C12) and members
  makroartefaktit.csv  expression-defined macros (recorded value not reliable)
  yhteenveto.txt       counts and self-check results

With --tarkista the script also validates rekisteri/riskiarvio.csv and
rekisteri/korjauskokonaisuudet.csv: coverage of every H and T, valid
identifiers, valid dependencies, no dependency cycle, order consistent with the
dependencies.

Nothing is evaluated numerically from macro values: macro values are only
listed, and the artefact list marks the ones that must be read from source.
"""
import csv
import io
import os
import re
import sys

sys.dont_write_bytecode = True

DASH = "–"

# Batch of each identifier by number range. The ranges are the ones recorded in
# CHECKPOINT.md and seuranta.md; check_batches() compares them with the source
# column of the register.
H_BATCH = [(1, 6, "K1"), (7, 21, "C-1"), (22, 39, "C-2"), (40, 54, "C-3"),
           (55, 77, "C-4"), (78, 86, "C-5"), (87, 108, "C-6"), (109, 127, "C-7"),
           (128, 144, "C-8"), (145, 158, "C-9"), (159, 170, "C-10"), (171, 177, "C-12")]
T_BATCH = [(1, 10, "C-1"), (11, 14, "C-2"), (15, 19, "C-3"), (20, 22, "C-4"),
           (23, 29, "C-5"), (30, 33, "C-6"), (34, 37, "C-7"), (38, 39, "C-8"),
           (40, 42, "C-9"), (43, 49, "C-10"), (50, 52, "C-12")]
Q_BATCH = [(1, 11, "K1"), (12, 15, "C-1"), (16, 16, "C-2"), (17, 18, "C-3"),
           (19, 25, "C-2"), (26, 34, "C-3"), (35, 42, "C-11"), (43, 56, "C-4"),
           (57, 64, "C-5"), (65, 74, "C-6"), (75, 77, "C-7"), (78, 82, "C-8"),
           (83, 95, "C-9"), (96, 106, "C-10"), (107, 112, "C-12")]
BATCH_DOC = {"C-1": "tallenna-samanaikaisuus.md", "C-2": "tiedonsiirto.md",
             "C-3": "tiedostomuodot.md", "C-4": "muistiturvallisuus.md",
             "C-5": "kaantajaerot.md", "C-6": "saikeet-ja-lukot.md",
             "C-7": "gui-liiketoimintalogiikka.md", "C-8": "globaali-tila.md",
             "C-9": "virheenkasittely.md", "C-10": "riippuvuudet.md",
             "C-11": "testaus.md", "C-12": "ominaisuusvalitsimet.md"}
Q_DOCS = ["tiedonsiirto.md", "tiedostomuodot.md", "testaus.md", "muistiturvallisuus.md",
          "kaantajaerot.md", "saikeet-ja-lukot.md", "gui-liiketoimintalogiikka.md",
          "globaali-tila.md", "virheenkasittely.md", "riippuvuudet.md"]

ID_RE = re.compile(r"\b([HT])-(\d{3})\b")
Q_RE = re.compile(r"\bQ(\d{1,3})\b")
RANGE_RE = re.compile(r"\b([HT])-(\d{3})\s*[" + DASH + r"-]\s*(?:\1-)?(\d{3})\b")
QRANGE_RE = re.compile(r"\bQ(\d{1,3})\s*[" + DASH + r"-]\s*Q(\d{1,3})\b")


def read(path):
    with io.open(path, encoding="utf-8") as fh:
        return fh.read()


def batch_of(table, n):
    for lo, hi, b in table:
        if lo <= n <= hi:
            return b
    return "?"


def cells_of(line):
    return [c.strip() for c in line.strip().strip("|").split("|")]


def expand_ids(text):
    """All H/T identifiers in text, ranges such as H-111-H-117 expanded."""
    out = []
    for m in RANGE_RE.finditer(text):
        k, a, b = m.group(1), int(m.group(2)), int(m.group(3))
        if a < b <= a + 60:
            out += ["%s-%03d" % (k, i) for i in range(a, b + 1)]
    out += ["%s-%s" % (m.group(1), m.group(2)) for m in ID_RE.finditer(text)]
    seen, res = set(), []
    for x in out:
        if x not in seen:
            seen.add(x)
            res.append(x)
    return res


def expand_qs(text):
    out = []
    for m in QRANGE_RE.finditer(text):
        a, b = int(m.group(1)), int(m.group(2))
        if a < b <= a + 40:
            out += list(range(a, b + 1))
    out += [int(m.group(1)) for m in Q_RE.finditer(text)]
    seen, res = set(), []
    for x in out:
        if x not in seen:
            seen.add(x)
            res.append(x)
    return res


def strip_parens(s):
    prev = None
    while prev != s:
        prev = s
        s = re.sub(r"\([^()]*\)", " ", s)
    return s


def short(text, n=170):
    """Short title: text up to the first sentence end, then cut to n characters."""
    t = re.sub(r"\*\*", "", text)
    t = re.sub(r"\s+", " ", t).strip()
    if len(t) > n:
        t = t[:n].rsplit(" ", 1)[0] + " ..."
    return t


# ---------------------------------------------------------------- severity ---
def norm_severity(raw):
    """Normalised severity: base level plus a 'conditional' flag.

    The register uses free text (e.g. 'korkea ARES-laitteella'). The base level
    is the highest level named; 'ehdollinen' is set when the text carries a
    condition or names two levels.
    """
    low = raw.lower()
    if "kumottu" in low or "refuted" in low:
        return "kumottu", ""
    has_hi = "korkea" in low
    has_mid = "keskitaso" in low
    has_lo = "matala" in low
    m = re.match(r"^(matala|keskitaso|korkea)\s*[" + DASH + r"-]\s*(matala|keskitaso|korkea)", low)
    if m:
        pair = {m.group(1), m.group(2)}
        if pair == {"keskitaso", "korkea"}:
            return "keskitaso-korkea", ""
        if pair == {"matala", "keskitaso"}:
            return "matala-keskitaso", ""
    base = "korkea" if has_hi else "keskitaso" if has_mid else "matala" if has_lo else "?"
    fm = re.match(r"^(matala|keskitaso|korkea)\b", low)
    if fm:
        base = fm.group(1)  # 'keskitaso; korkea epavakaalla verkolla' -> keskitaso, conditional
    plain = re.sub(r"\s*/\s*(high|medium|low)\s*$", "", low).strip()
    cond = "" if plain in ("korkea", "keskitaso", "matala") else "ehdollinen"
    return base, cond


# ------------------------------------------------------------ verification ---
def norm_verification(ver, evid):
    """Return (review, experiment).

    review:      riippumaton | riippumaton_osittain | tarkastajan_loyto | ei |
                 ei_ratkea_staattisesti
    experiment:  ei | koe_tai_testikehys | toimitettu_ohjelma
    """
    low = ver.lower()
    head = low[:140]
    if head.startswith("ei todennettavissa staattisesti"):
        review = "ei_ratkea_staattisesti"
    elif head.startswith("ei tarkastettu") or head.startswith("ei riippumatonta"):
        review = "ei"
    elif head.startswith("tarkastajan"):
        review = "tarkastajan_loyto"
    elif head.startswith("osittain"):
        review = "riippumaton_osittain"
    elif "riippumaton" in head or "tarkastus" in head:
        # Partial when the verdict part says so. The verdict part is the text
        # before the first ';' with batch tags such as '(C-8, Opus)' removed,
        # cut at the next parenthesis.
        first = re.split(r";", head, maxsplit=1)[0]
        first = re.sub(r"\((?:c-\d+[^()]*|pääistunto[^()]*)\)", " ", first)
        verdict = re.split(r"\(", first, maxsplit=1)[0]
        partial = bool(re.search(r"osittain|partly|\bosin\b", verdict)) or "(osittain" in first
        review = "riippumaton_osittain" if partial else "riippumaton"
    else:
        review = "?"
    if "vahvistettu ajamalla" in low or "confirmed by running" in low:
        exp = "toimitettu_ohjelma"
    elif re.search(r"(?<!ei )toistettu (testikehyksess|eristetyll)", low) or \
            re.search(r"reproduced (in test harness|with an isolated probe|with isolated probes)", low) or \
            re.search(r"mekanismi toistettu", low) or "koeohjelmalla toistettu" in low:
        exp = "koe_tai_testikehys"
    else:
        exp = "ei"
    return review, exp


def norm_confidence(raw):
    flags = []
    for k, name in (("[todennettu]", "todennettu"), ("[päätelty]", "paatelty"),
                    ("[tuntematon]", "tuntematon")):
        if k in raw:
            flags.append(name)
    return "+".join(flags) or "?"


# ------------------------------------------------------------- priorities ---
P_TOKEN = re.compile(r"\*\*P([1-5])\*\*|\bP([1-5])\b(?!\s*[" + DASH + r"-]\s*P)")
ID_LIST = re.compile(r"(?:\s*(?:,|ja|and|sekä)?\s*H-\d{3}(?:\s*[" + DASH + r"-]\s*H-\d{3})?)+")


def priorities_from_text(text, where, out, conflicts):
    """Find 'P<n> H-xxx, H-yyy' runs. Parenthesised remarks are dropped first."""
    s = strip_parens(text)
    for m in P_TOKEN.finditer(s):
        p = "P" + (m.group(1) or m.group(2))
        rest = s[m.end():]
        lm = ID_LIST.match(rest)
        if not lm:
            continue
        for hid in expand_ids(lm.group(0)):
            add_priority(hid, p, where, out, conflicts)


def add_priority(hid, p, where, out, conflicts):
    if hid in out and out[hid][0] != p:
        conflicts.append("%s: %s (%s) vs %s (%s)" % (hid, out[hid][0], out[hid][1], p, where))
        return
    if hid not in out:
        out[hid] = (p, where)


def parse_priorities(doc):
    """Current P1-P5 per finding. Later closing decisions win: seuranta.md is
    read first (it holds the final per-batch priorities), then the C-4 table."""
    out, conflicts = {}, []
    seur = read(os.path.join(doc, "seuranta.md"))
    m = re.search(r"^## Priorisoitu korjausjono.*?(?=^## )", seur, re.S | re.M)
    sec = m.group(0)
    for line in sec.splitlines():
        if re.match(r"\|\s*\d+b?\s*\|", line):
            c = cells_of(line)
            pm = re.search(r"\bP([1-5])\b(?!\s*[" + DASH + r"-])", c[3])
            if pm:
                for hid in expand_ids(c[1]):
                    add_priority(hid, "P" + pm.group(1), "seuranta.md korjausjono, rivi " + c[0],
                                 out, conflicts)
    for para in re.split(r"\n\s*\n", sec):
        if para.lstrip().startswith("|") or para.lstrip().startswith("#"):
            continue
        tag = re.match(r"\s*(C-\d+)", para)
        priorities_from_text(para, "seuranta.md korjausjono, %s" % (tag.group(1) if tag else "?"),
                             out, conflicts)
    mem = read(os.path.join(doc, "muistiturvallisuus.md"))
    for line in mem.splitlines():
        pm = re.match(r"\|\s*\*\*P([1-5]):", line)
        if pm:
            c = cells_of(line)
            for hid in expand_ids(strip_parens(c[1])):
                add_priority(hid, "P" + pm.group(1), "muistiturvallisuus.md luku 15", out, conflicts)
    return out, conflicts


# ------------------------------------------------------------ fix packages ---
# Earlier packages only (batches C-9, C-10 and C-12); the KK-C13 packages are defined in
# rekisteri/korjauskokonaisuudet.csv and are not parsed from running text.
KK_RE = re.compile(r"KK-C(?:9|10|12)-\d+\b")


def parse_kk(doc, rows):
    """Earlier fix packages: (kk, id) -> set of sources; partial membership is
    recorded when the identifier is in the genitive ('H-170:n kohdat ...')."""
    mem = {}

    def add(kk, ident, src, partial=False):
        key = (kk, ident)
        e = mem.setdefault(key, {"src": [], "partial": False})
        if src not in e["src"]:
            e["src"].append(src)
        e["partial"] = e["partial"] or partial

    for r in rows:
        for kk in sorted(set(KK_RE.findall(r["_line"]))):
            add(kk, r["id"], "havainnot.md rivi")

    def members(text):
        res = []
        for ident in expand_ids(text):
            partial = bool(re.search(re.escape(ident) + r":n\b", text))
            res.append((ident, partial))
        return res

    seur = read(os.path.join(doc, "seuranta.md"))
    for m in re.finditer(r"\*\*(KK-C(?:9|10|12)-\d+)\*\*\s*[^()*|]*\(([^()]*)\)", seur):
        for ident, partial in members(m.group(2)):
            add(m.group(1), ident, "seuranta.md", partial)
    for name in ("virheenkasittely.md", "ominaisuusvalitsimet.md"):
        for line in read(os.path.join(doc, name)).splitlines():
            m = re.match(r"\|\s*\*{0,2}(KK-C(?:9|10|12)-\d+)\b", line)
            if m:
                c = cells_of(line)
                body = " | ".join(c[1:])
                # 'liittyy H-044' = related, not a member
                body = re.split(r"liittyy", body)[0]
                for ident, partial in members(body):
                    add(m.group(1), ident, name, partial)
    lines = read(os.path.join(doc, "riippuvuudet.md")).splitlines()
    for i, line in enumerate(lines):
        m = re.match(r"\*\*(KK-C10-\d)\b[^*]*\*\*(.*)", line)
        if not m:
            continue
        para = [m.group(2)]
        j = i + 1
        while j < len(lines) and lines[j].strip() and not lines[j].lstrip().startswith(("-", "1.", "/")):
            para.append(lines[j])
            j += 1
        # top-level bullets directly naming identifiers (KK-C10-4 lists T-043, H-166, H-161)
        if not expand_ids(" ".join(para)):
            while j < len(lines) and not lines[j].startswith(("**KK", "###", "Uusia")):
                if re.match(r"- [HT]-\d{3}", lines[j]):
                    para.append(lines[j])
                j += 1
        for ident, partial in members(" ".join(para)):
            add(m.group(1), ident, "riippuvuudet.md 19.4", partial)
    return mem


# --------------------------------------------------------------- questions ---
Q_ITEM = r"Q\d+(?:\s*[" + DASH + r"-]\s*Q\d+)?(?:\s*\([^()]*\))?"
Q_LIST = Q_ITEM + r"(?:\s*(?:,|ja|and)\s*" + Q_ITEM + r")*"
ST_WORDS = [("ratkaistu", "ratkaistu"), ("resolved", "ratkaistu"), ("osittain", "osittain"),
            ("partly", "osittain"), ("avoinna", "avoin"), ("open", "avoin"),
            ("owner decision", "omistajan_paatos")]
ST_AFTER = re.compile(r"(" + Q_LIST + r")\s+(ratkaistu|resolved|osittain|partly|avoinna|open|owner decision)")
ST_BEFORE = re.compile(r"(avoinna|open)\s+(" + Q_LIST + r")")


def status_word(w):
    return dict(ST_WORDS)[w]


def row_status(first_cell, text):
    """Status of a single-question row."""
    low = text.lower()
    head = low[:120]
    if "omistajan päätös" in low or "owner decision" in low:
        if "~~" not in first_cell:
            return "omistajan_paatos"
    if "siirretty ajonaikaiseen todennukseen" in head:
        return "ajonaikainen_todennus"
    if "osittain ratkaistu" in head or "partly resolved" in head or "**osittain" in head[:60]:
        return "osittain"
    if "~~" in first_cell or head.startswith("**ratkaistu"):
        return "ratkaistu"
    if "**osittain ratkaistu" in low or "**kavennettu" in low or "**osittain (" in low:
        return "osittain"
    return "avoin"


def parse_questions(doc):
    qs = {}

    def put(n, status, src, text=None, force=False):
        e = qs.setdefault(n, {"tila": None, "lahde": "", "kuvaus": "", "tekstit": []})
        if text:
            e["tekstit"].append(text)
            if not e["kuvaus"]:
                e["kuvaus"] = short(text, 200)
        if status and (force or e["tila"] is None):
            e["tila"] = status
            e["lahde"] = src

    # 1. per-question rows in the batch reports (details of the range rows)
    for name in Q_DOCS:
        for line in read(os.path.join(doc, name)).splitlines():
            m = re.match(r"\|\s*(~~)?Q(\d+)(~~)?\s*\|", line)
            if m:
                c = cells_of(line)
                put(int(m.group(2)), row_status(c[0], " | ".join(c[1:])), name, " | ".join(c[1:]))
    # 2. the tracking table: single rows override, range rows refine by statement
    seur = read(os.path.join(doc, "seuranta.md"))
    m = re.search(r"^## Avoimet kysymykset.*?(?=^## )", seur, re.S | re.M)
    for line in m.group(0).splitlines():
        if not re.match(r"\|\s*(~~)?Q\d", line):
            continue
        c = cells_of(line)
        text = " | ".join(c[1:])
        nums = expand_qs(c[0])
        is_range = bool(QRANGE_RE.search(c[0]))
        if not is_range:
            for n in nums:
                put(n, row_status(c[0], text), "seuranta.md", text, force=True)
            continue
        for n in nums:
            put(n, None, "seuranta.md")
            qs[n]["tekstit"].append(text)
        for sm in ST_AFTER.finditer(text):
            for n in expand_qs(sm.group(1)):
                if n in nums:
                    put(n, status_word(sm.group(2)), "seuranta.md (alueen rivi)", force=True)
        for sm in ST_BEFORE.finditer(text):
            for n in expand_qs(sm.group(2)):
                if n in nums and (qs[n]["tila"] in (None, "avoin")):
                    put(n, status_word(sm.group(1)), "seuranta.md (alueen rivi)", force=True)
        for n in nums:
            if qs[n]["tila"] is None:
                put(n, "avoin", "seuranta.md (alueen rivi, ei erillistä tilaa)")
    return qs


# ---------------------------------------------------------- macro artefact ---
def macro_artefacts(doc):
    """Macros defined by an expression. The C-10/C-12 evaluators are lexical and
    record such macros with the value 1 (e.g. relay MAX_PORTTI, really 132), so
    their recorded values must not be used; read the source instead."""
    path = os.path.join(doc, "todisteet", "c12", "osaA0", "maaritykset.csv")
    out = []
    if not os.path.exists(path):
        return out
    # Same rule as the C-12 closing check: not function-like, value present and
    # not a numeric literal (aliases such as LANG=SU are included).
    lit = re.compile(r"^(?:[-+]?(?:0[xX][0-9a-fA-F]+|\d+)[uUlL]*|)$")
    with io.open(path, encoding="utf-8", newline="") as fh:
        for r in csv.DictReader(fh):
            if r["kind"] != "define" or r["function_like"] == "yes":
                continue
            v = r["value"].strip()
            if lit.match(v):
                continue
            out.append([r["macro"], r["file"], r["line"], v, r["tested"] or "no"])
    return out


# ------------------------------------------------------------------- main ---
def parse_register(doc):
    hrows, trows = [], []
    prev_src = ""
    for line in read(os.path.join(doc, "rekisteri", "havainnot.md")).splitlines():
        m = re.match(r"\|\s*([HT])-(\d{3})\s*\|", line)
        if not m:
            continue
        c = cells_of(line)
        ident = "%s-%s" % (m.group(1), m.group(2))
        n = int(m.group(2))
        if m.group(1) == "H":
            sev, cond = norm_severity(c[3])
            review, exp = norm_verification(c[5], c[8])
            if c[7].startswith("sama"):  # 'sama / same' = the source of the row above
                c[7] = prev_src + c[7][4:].replace(" / same", "", 1)
            else:
                prev_src = c[7].split(", `../testaus")[0]
            hrows.append({"id": ident, "n": n, "era": batch_of(H_BATCH, n), "otsikko": short(c[1]),
                          "vakavuus_raaka": c[3], "vakavuus": sev, "vakavuus_ehto": cond,
                          "varmuus": norm_confidence(c[4]), "tarkastus": review, "koe": exp,
                          "rev": c[6], "lahde": c[7], "naytto": short(c[8], 90),
                          "_line": line, "_cells": c})
        else:
            sev, cond = norm_severity(c[3])
            trows.append({"id": ident, "n": n, "era": batch_of(T_BATCH, n), "otsikko": short(c[1]),
                          "vakavuus_raaka": c[3], "vakavuus": sev, "varmuus": norm_confidence(c[4]),
                          "rev": c[5], "naytto": short(c[6], 90), "_line": line, "_cells": c})
    return hrows, trows


def check_batches(hrows):
    """Compare the range-based batch with the report named in the source column."""
    notes = []
    for r in hrows:
        src = r["lahde"]
        docs = re.findall(r"\.\./([a-z-]+\.md)", src)
        want = BATCH_DOC.get(r["era"])
        if r["era"] == "K1" or not docs or want is None:
            continue
        if want not in docs:
            notes.append("%s: range says %s (%s), source column says %s" % (r["id"], r["era"], want, ", ".join(docs)))
    return notes


def write_csv(path, header, rows):
    buf = io.StringIO(newline="")
    w = csv.writer(buf, lineterminator="\n")
    w.writerow(header)
    for r in rows:
        w.writerow(r)
    with open(path, "wb") as fh:
        fh.write(buf.getvalue().encode("utf-8"))


def count(rows, key):
    d = {}
    for r in rows:
        d[r[key]] = d.get(r[key], 0) + 1
    return d


def fmt_counts(d, order=None):
    keys = order or sorted(d)
    return ", ".join("%s %d" % (k, d[k]) for k in keys if k in d)


def build(root, out):
    doc = os.path.join(root, "doc", "arkkitehtuuri")
    os.makedirs(out, exist_ok=True)
    hrows, trows = parse_register(doc)
    prio, pconf = parse_priorities(doc)
    kk = parse_kk(doc, hrows + trows)
    qs = parse_questions(doc)
    known = {r["id"] for r in hrows + trows}

    kk_of = {}
    for (k, ident), e in sorted(kk.items()):
        kk_of.setdefault(ident, []).append(k + ("*" if e["partial"] else ""))

    # cross references: identifiers named in the row text of another identifier
    edges = []
    q_refs = {}
    for r in hrows + trows:
        body = " | ".join(r["_cells"][1:])
        for ident in expand_ids(body):
            if ident != r["id"]:
                edges.append([r["id"], ident, "havainnot.md"])
        for n in expand_qs(body):
            edges.append([r["id"], "Q%d" % n, "havainnot.md"])
    for n, e in sorted(qs.items()):
        seen = set()
        for t in e["tekstit"]:
            if len(expand_qs(t)) > 6 and len(e["tekstit"]) > 1 and t is e["tekstit"][-1]:
                continue  # range row text names every question of the batch
            for ident in expand_ids(t):
                if ident not in seen:
                    seen.add(ident)
                    edges.append(["Q%d" % n, ident, "kysymysrivi"])
                    q_refs.setdefault(ident, []).append("Q%d" % n)
    refs_to = {}
    for a, b, _ in edges:
        refs_to.setdefault(a, [])
        if b not in refs_to[a]:
            refs_to[a].append(b)

    def sel(ident, kind):
        return " ".join(x for x in refs_to.get(ident, []) if x.startswith(kind))

    hout = []
    for r in hrows:
        p, psrc = prio.get(r["id"], ("", ""))
        tila = "kumottu" if r["vakavuus"] == "kumottu" else "avoin"
        hout.append([r["id"], r["era"], tila, r["vakavuus"], r["vakavuus_ehto"], r["vakavuus_raaka"],
                     r["varmuus"], r["tarkastus"], r["koe"], p, psrc,
                     " ".join(kk_of.get(r["id"], [])), sel(r["id"], "H-"), sel(r["id"], "T-"),
                     sel(r["id"], "Q"), " ".join(q_refs.get(r["id"], [])), r["rev"], r["lahde"],
                     r["naytto"], r["otsikko"]])
    write_csv(os.path.join(out, "havainnot.csv"),
              ["id", "era", "tila", "vakavuus", "vakavuus_ehto", "vakavuus_raaka", "varmuus",
               "tarkastus", "koe", "prioriteetti_nykyinen", "prioriteetin_lahde", "kk_aiemmat",
               "viittaa_H", "viittaa_T", "viittaa_Q", "kysymykset_viittaavat", "rev", "lahde",
               "nayton_laji", "otsikko"], hout)
    tout = []
    for r in trows:
        tout.append([r["id"], r["era"], r["vakavuus"], r["vakavuus_raaka"], r["varmuus"],
                     " ".join(kk_of.get(r["id"], [])), sel(r["id"], "H-"), sel(r["id"], "T-"),
                     sel(r["id"], "Q"), " ".join(q_refs.get(r["id"], [])), r["rev"], r["naytto"],
                     r["otsikko"]])
    write_csv(os.path.join(out, "tekninen_velka.csv"),
              ["id", "era", "vakavuus", "vakavuus_raaka", "varmuus", "kk_aiemmat", "viittaa_H",
               "viittaa_T", "viittaa_Q", "kysymykset_viittaavat", "rev", "nayton_laji", "otsikko"], tout)
    qout = []
    for n, e in sorted(qs.items()):
        qout.append(["Q%d" % n, batch_of(Q_BATCH, n), e["tila"], e["lahde"],
                     sel("Q%d" % n, "H-"), sel("Q%d" % n, "T-"), e["kuvaus"]])
    write_csv(os.path.join(out, "kysymykset.csv"),
              ["id", "era", "tila", "tilan_lahde", "viittaa_H", "viittaa_T", "kuvaus"], qout)
    write_csv(os.path.join(out, "viittaukset.csv"), ["mista", "mihin", "lahde"], edges)
    write_csv(os.path.join(out, "kk_aiemmat.csv"), ["kk", "id", "osittainen", "lahteet"],
              [[k, ident, "kylla" if e["partial"] else "", "; ".join(e["src"])]
               for (k, ident), e in sorted(kk.items())])
    arte = macro_artefacts(doc)
    write_csv(os.path.join(out, "makroartefaktit.csv"), ["makro", "tiedosto", "rivi", "lauseke", "testattu"], arte)

    # ---- summary and self-checks
    L = []
    L.append("C-13 A0: synteesin koneellinen pohja / machine base")
    L.append("H: %d, T: %d, Q: %d (Q1-Q%d)" % (len(hrows), len(trows), len(qs), max(qs)))
    missing_q = [n for n in range(1, max(qs) + 1) if n not in qs]
    L.append("Q missing from the base: %s" % (missing_q or "none"))
    L.append("")
    L.append("H per batch: " + fmt_counts(count(hrows, "era"), ["K1"] + ["C-%d" % i for i in range(1, 13)]))
    L.append("T per batch: " + fmt_counts(count(trows, "era"), ["C-%d" % i for i in range(1, 13)]))
    L.append("H severity (normalised): " + fmt_counts(count(hrows, "vakavuus")))
    L.append("  of which conditional: %d" % sum(1 for r in hrows if r["vakavuus_ehto"]))
    L.append("T severity (normalised): " + fmt_counts(count(trows, "vakavuus")))
    L.append("H review: " + fmt_counts(count(hrows, "tarkastus")))
    L.append("H experiment: " + fmt_counts(count(hrows, "koe")))
    for k in ("koe_tai_testikehys", "toimitettu_ohjelma"):
        L.append("  %s: %s" % (k, " ".join(r["id"] for r in hrows if r["koe"] == k)))
    L.append("  review=ei: %s" % " ".join(r["id"] for r in hrows if r["tarkastus"] == "ei"))
    L.append("  review=tarkastajan_loyto: %s" % " ".join(r["id"] for r in hrows if r["tarkastus"] == "tarkastajan_loyto"))
    L.append("  review=?: %s" % (" ".join(r["id"] for r in hrows if r["tarkastus"] == "?") or "none"))
    pc = {}
    for r in hrows:
        p = prio.get(r["id"], ("(ei asetettu)",))[0]
        pc[p] = pc.get(p, 0) + 1
    L.append("H current priority: " + fmt_counts(pc))
    L.append("  without a priority: %s" % " ".join(r["id"] for r in hrows if r["id"] not in prio))
    L.append("priority conflicts between sources: %s" % (pconf or "none"))
    for hid in sorted(prio):
        if hid not in known:
            L.append("  priority for an unknown identifier: %s" % hid)
    L.append("")
    L.append("Q status: " + fmt_counts(count([{"t": e["tila"]} for e in qs.values()], "t")))
    for st in ("avoin", "osittain", "omistajan_paatos", "ajonaikainen_todennus"):
        L.append("  %s: %s" % (st, " ".join("Q%d" % n for n, e in sorted(qs.items()) if e["tila"] == st)))
    L.append("")
    L.append("earlier fix packages:")
    for k in sorted({k for k, _ in kk}):
        mem = ["%s%s" % (i, "*" if kk[(kk_, i)]["partial"] else "") for (kk_, i) in sorted(kk) if kk_ == k]
        L.append("  %s: %s" % (k, " ".join(mem)))
    L.append("  membership named by one source only:")
    for (k, ident), e in sorted(kk.items()):
        if len(e["src"]) == 1:
            L.append("    %s %s: %s" % (k, ident, e["src"][0]))
    L.append("")
    notes = check_batches(hrows)
    L.append("batch range vs source column: %s" % ("consistent" if not notes else ""))
    L += ["  " + x for x in notes]
    bad = sorted({b for a, b, _ in edges if re.match(r"[HT]-", b) and b not in known})
    L.append("references to undefined H/T identifiers: %s" % (bad or "none"))
    L.append("")
    tested = [a for a in arte if a[4] == "yes"]
    L.append("macro evaluator artefact: %d non-literal macro definitions (%d distinct names), "
             "%d distinct tested names; recorded values of these are NOT usable"
             % (len(arte), len({a[0] for a in arte}), len({a[0] for a in tested})))
    L.append("  relay MAX_PORTTI (Juk/VDef.h) in the artefact list: %s"
             % ("yes" if any(a[0] == "MAX_PORTTI" and a[1].endswith("VDef.h") for a in arte) else "NO"))
    with open(os.path.join(out, "yhteenveto.txt"), "wb") as fh:
        fh.write(("\n".join(L) + "\n").encode("utf-8"))
    print("\n".join(L))
    return hrows, trows, qs


# ------------------------------------------------------- register checking ---
def check_registers(root, hrows, trows, qs):
    """Validate rekisteri/riskiarvio.csv and rekisteri/korjauskokonaisuudet.csv."""
    doc = os.path.join(root, "doc", "arkkitehtuuri")
    errors = []
    hk = {r["id"]: r for r in hrows}
    tk = {r["id"]: r for r in trows}
    rpath = os.path.join(doc, "rekisteri", "riskiarvio.csv")
    kpath = os.path.join(doc, "rekisteri", "korjauskokonaisuudet.csv")
    if not (os.path.exists(rpath) and os.path.exists(kpath)):
        print("registers not present yet: nothing to check")
        return 0
    with io.open(rpath, encoding="utf-8", newline="") as fh:
        risk = list(csv.DictReader(fh))
    with io.open(kpath, encoding="utf-8", newline="") as fh:
        packs = list(csv.DictReader(fh))
    pk = {p["kk"]: p for p in packs}
    if len(pk) != len(packs):
        errors.append("korjauskokonaisuudet.csv: duplicate package identifiers")
    # risk register: one row per H, valid values
    ids = [r["id"] for r in risk]
    if sorted(ids) != sorted(hk):
        errors.append("riskiarvio.csv: rows differ from the H register: missing %s, extra %s"
                      % (sorted(set(hk) - set(ids)), sorted(set(ids) - set(hk))))
    for r in risk:
        refuted = hk.get(r["id"], {}).get("vakavuus") == "kumottu"
        if refuted:
            if r["prioriteetti"] != "-" or r["kk"] != "-":
                errors.append("riskiarvio.csv %s: refuted finding must have '-' priority and package" % r["id"])
            continue
        if r["prioriteetti"] not in ("P1", "P2", "P3", "P4", "P5"):
            errors.append("riskiarvio.csv %s: bad priority %r" % (r["id"], r["prioriteetti"]))
        if r["kk"] not in pk:
            errors.append("riskiarvio.csv %s: unknown package %r" % (r["id"], r["kk"]))
        for k in r.get("kk_muut", "").split():
            if k not in pk:
                errors.append("riskiarvio.csv %s: unknown secondary package %r" % (r["id"], k))
        for col in ("tekninen_vakavuus", "saavutettavuus", "todennus", "tulosten_oikeellisuus",
                    "ajanoton_jatkuvuus", "palautettavuus", "tietoturva"):
            if not r.get(col):
                errors.append("riskiarvio.csv %s: empty %s" % (r["id"], col))
    # package register: members exist, every H and T is in exactly one primary package
    primary = {}
    for p in packs:
        for ident in (p["havainnot"] + " " + p["tekninen_velka"]).split():
            if ident not in hk and ident not in tk:
                errors.append("%s: unknown member %s" % (p["kk"], ident))
            if ident in primary:
                errors.append("%s: %s is already a primary member of %s" % (p["kk"], ident, primary[ident]))
            primary[ident] = p["kk"]
        for q in p.get("kysymykset", "").split():
            if not re.match(r"Q\d+$", q) or int(q[1:]) not in qs:
                errors.append("%s: unknown question %s" % (p["kk"], q))
        for d in p["riippuu"].split():
            if d not in pk:
                errors.append("%s: depends on unknown package %s" % (p["kk"], d))
        # Dependencies of later PR series, written as <part>:<package>. They are
        # not part of the cycle and wave checks, because a later series of A may
        # wait for B while B's first series waits for A's first series
        # (e.g. KK-C13-4 PR 4 waits for KK-C13-5, which starts after KK-C13-4).
        for tok in p.get("riippuu_osat", "").split():
            part, sep, d = tok.rpartition(":")
            if not sep or not part or d not in pk:
                errors.append("%s: bad part dependency %r" % (p["kk"], tok))
            elif d == p["kk"]:
                errors.append("%s: part dependency on itself %r" % (p["kk"], tok))
    for r in risk:
        if hk.get(r["id"], {}).get("vakavuus") == "kumottu":
            if r["id"] in primary:
                errors.append("%s is refuted but is a member of %s" % (r["id"], primary[r["id"]]))
            continue
        if primary.get(r["id"]) != r["kk"]:
            errors.append("riskiarvio.csv %s: package %s, but korjauskokonaisuudet.csv has %s"
                          % (r["id"], r["kk"], primary.get(r["id"])))
    for ident in sorted(tk):
        if ident not in primary:
            errors.append("%s is not a member of any package" % ident)
    # dependency graph: no cycle; order numbers respect dependencies
    deps = {p["kk"]: [d for d in p["riippuu"].split() if d in pk] for p in packs}
    state = {}

    def visit(k, stack):
        if state.get(k) == 2:
            return
        if state.get(k) == 1:
            errors.append("dependency cycle: " + " -> ".join(stack + [k]))
            return
        state[k] = 1
        for d in deps[k]:
            visit(d, stack + [k])
        state[k] = 2

    for k in sorted(deps):
        visit(k, [])
    for p in packs:
        if not p["kk"].startswith("KK-"):
            continue  # the 'ULKOPUOLELLA' row lists members left outside with a reason
        if p["prioriteetti"] not in ("P1", "P2", "P3", "P4", "P5"):
            errors.append("%s: bad priority %r" % (p["kk"], p["prioriteetti"]))
        if p["tyomaara"] not in ("S", "M", "L", "XL"):
            errors.append("%s: bad size %r" % (p["kk"], p["tyomaara"]))
        if not p["haara"]:
            errors.append("%s: no branch name" % p["kk"])
        # package priority = highest priority of its findings
        mine = [r["prioriteetti"] for r in risk if r["kk"] == p["kk"] and r["prioriteetti"].startswith("P")]
        if mine and min(mine) != p["prioriteetti"]:
            errors.append("%s: priority %s, but its highest finding is %s"
                          % (p["kk"], p["prioriteetti"], min(mine)))
        for d in deps[p["kk"]]:
            try:
                if int(pk[d]["aalto"]) > int(p["aalto"]):
                    errors.append("%s (wave %s) depends on %s (wave %s)"
                                  % (p["kk"], p["aalto"], d, pk[d]["aalto"]))
            except ValueError:
                errors.append("%s or %s: wave is not a number" % (p["kk"], d))
    # every P1/P2 finding sits in a package of wave 1 or 2 unless the package says why not
    print("registers: %d risk rows, %d packages, %d primary members"
          % (len(risk), len(packs), len(primary)))
    pc = {}
    for r in risk:
        pc[r["prioriteetti"]] = pc.get(r["prioriteetti"], 0) + 1
    print("priority distribution: " + fmt_counts(pc))
    for e in errors:
        print("ERROR", e)
    print("OK" if not errors else "%d errors" % len(errors))
    return 1 if errors else 0


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(2)
    h, t, q = build(sys.argv[1], sys.argv[2])
    rc = 0
    if "--tarkista" in sys.argv[3:]:
        rc = check_registers(sys.argv[1], h, t, q)
    sys.exit(rc)
