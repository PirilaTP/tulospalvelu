#!/usr/bin/env python3
"""K-0 one-off generator: adds the knowledge-base columns to the registers.

Usage (from the repository root):
    python -I doc/arkkitehtuuri/todisteet/k0/tee_k0.py .

What it does (once; later maintenance is by hand, checked by synteesi.py --tarkista):
  1. rekisteri/riskiarvio.csv: appends columns juurisyy, juurisyy_muut, ohjelmat,
     ohjelmat_peruste, tila, korjaus. Existing columns and values are not changed.
  2. rekisteri/tekninen-velka.csv (new): one row per T with the same columns.
     The fix package of a T row stays only in korjauskokonaisuudet.csv.
  3. omistajan-paatokset.md: adds a status line to every OP section (all open).
  4. rekisteri/vaikutusulottuvuudet.csv and rekisteri/vaikutukset.csv are written by
     hand (not by this script).

Sources:
  juurisyy      todisteet/c13/raportit/osaB-juurisyyt.md, section 9 (row -> root cause);
                'ISO' there becomes 'yksittainen'.
  ohjelmat      programs whose project compiles a file cited in the evidence column of
                havainnot.md, via tiedostot.csv 'projektit'. This is an UPPER BOUND
                (ohjelmat_peruste = 'tiedosto'): #ifdef branches and unused library
                objects are not considered. No cited production file -> 'ei_tiedossa'.
  tila          'avoin', H-099 'kumottu'.
"""
import csv
import io
import os
import re
import sys

PROJECT_TO_PROGRAMS = {
    "HkKisaWin": ["HkKisaWin"],
    "ViestiWin": ["ViestiWin"],
    "HkMaali": ["HkMaali"],
    "JukMaali520": ["ViestiMaali"],
    "JukLuenta520": ["ViestiLuenta"],
    "Tputil-XE": ["HkKisaWin", "ViestiWin"],
    "DBboxm-XE": ["HkKisaWin", "ViestiWin"],
    "tputilv2": ["HkMaali", "ViestiMaali", "ViestiLuenta"],
    "dbboxm": ["HkMaali", "ViestiMaali", "ViestiLuenta"],
}
ORDER = ["HkKisaWin", "HkMaali", "ViestiWin", "ViestiMaali", "ViestiLuenta"]
FILE_RE = re.compile(r"`([A-Za-z0-9_]+/[A-Za-z0-9_./-]+\.(?:cpp|h|c|H|hpp|dfm))(?::[0-9, -]+)?`")
NEW_COLS = ["juurisyy", "juurisyy_muut", "ohjelmat", "ohjelmat_peruste", "tila", "korjaus"]


def read_csv(path):
    with io.open(path, encoding="utf-8", newline="") as fh:
        return list(csv.reader(fh))


def write_csv(path, rows):
    out = io.StringIO(newline="")
    csv.writer(out, lineterminator="\n").writerows(rows)
    with open(path, "wb") as fh:
        fh.write(out.getvalue().encode("utf-8"))


def root_causes(doc):
    text = io.open(os.path.join(doc, "todisteet/c13/raportit/osaB-juurisyyt.md"), encoding="utf-8").read()
    part = text[text.index("## 9. Appendix"):]
    res = {}
    for line in part.splitlines():
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if not cells or not re.match(r"[HT]-\d{3}$", cells[0]):
            continue
        if cells[0].startswith("H-"):
            prim, worse = cells[2], cells[3]
        else:
            js = re.findall(r"JS-\d+", cells[1])
            prim, worse = (js[0] if js else "-"), " ".join(js[1:])
        prim = "yksittainen" if prim in ("ISO", "-") else prim
        worse = " ".join(re.findall(r"JS-\d+", worse))
        res[cells[0]] = (prim, worse)
    return res


def programs(doc):
    proj = {}
    for r in csv.DictReader(io.open(os.path.join(doc, "rekisteri/tiedostot.csv"), encoding="utf-8", newline="")):
        proj[r["polku"]] = r["projektit"].split()
    text = io.open(os.path.join(doc, "rekisteri/havainnot.md"), encoding="utf-8").read()
    res = {}
    for line in text.splitlines():
        m = re.match(r"\| ([HT]-\d{3}) \|", line)
        if not m:
            continue
        cells = line.split(" | ")
        evidence = cells[2] if len(cells) > 2 else ""
        progs = set()
        for f in FILE_RE.findall(evidence):
            for p in proj.get(f, []):
                progs.update(PROJECT_TO_PROGRAMS.get(p, []))
        res[m.group(1)] = [p for p in ORDER if p in progs]
    return res


def main(root):
    doc = os.path.join(root, "doc", "arkkitehtuuri")
    js = root_causes(doc)
    progs = programs(doc)

    rpath = os.path.join(doc, "rekisteri/riskiarvio.csv")
    rows = read_csv(rpath)
    if "juurisyy" in rows[0]:
        sys.exit("riskiarvio.csv already has the K-0 columns")
    rows[0] += NEW_COLS
    missing = []
    for r in rows[1:]:
        i = r[0]
        prim, worse = js.get(i, ("", ""))
        if not prim:
            missing.append(i)
        p = progs.get(i, [])
        refuted = r[2] == "kumottu"
        r += [("-" if refuted else prim), ("" if refuted else worse), " ".join(p) or "ei_tiedossa",
              "tiedosto" if p else "ei_tiedossa", "kumottu" if refuted else "avoin", ""]
    write_csv(rpath, rows)

    trows = [["id"] + NEW_COLS]
    for n in range(1, 53):
        i = "T-%03d" % n
        prim, worse = js.get(i, ("", ""))
        if not prim:
            missing.append(i)
        p = progs.get(i, [])
        trows.append([i, prim, worse, " ".join(p) or "ei_tiedossa", "tiedosto" if p else "ei_tiedossa",
                      "avoin", ""])
    write_csv(os.path.join(doc, "rekisteri/tekninen-velka.csv"), trows)

    opath = os.path.join(doc, "omistajan-paatokset.md")
    raw = open(opath, "rb").read().decode("utf-8")
    if "**Tila / status.**" not in raw:
        raw, n = re.subn(r"(\n## OP-\d+ [^\n]*\n\n)(- \*\*Asia\.\*\*)",
                         r"\1- **Tila / status.** avoin / open\n\2", raw)
        if n != 18:
            sys.exit("expected 18 OP sections, found %d" % n)
        open(opath, "wb").write(raw.encode("utf-8"))

    print("root cause missing for:", missing or "none")
    print("H rows without a cited production file:",
          [r[0] for r in rows[1:] if r[-3] == "ei_tiedossa"])
    print("T rows without a cited production file:", [r[0] for r in trows[1:] if r[4] == "ei_tiedossa"])


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else ".")
