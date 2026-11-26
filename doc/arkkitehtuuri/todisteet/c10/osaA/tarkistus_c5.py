#!/usr/bin/env python3
"""C-10 part A, script check 1: compare riippuvuudet.py project evaluation with the
build evidence recorded in C-5 (MSVC command lines in todisteet/c5/msvc/units.json,
bcc32 translation units in bda/ana_*.json from todisteet/c5/bda.tgz).

Usage: python -I tarkistus_c5.py <repo-root> <osaA-dir> <extracted bda dir>
"""
import collections
import csv
import json
import os
import sys

root, osaa, bda = sys.argv[1:4]
src = os.path.join(root, "TPsource", "V52")


def rel(p):
    return os.path.relpath(os.path.normpath(p), src).replace(os.sep, "/").lower()


tu = collections.defaultdict(set)
for r in csv.DictReader(open(os.path.join(osaa, "kaannosyksikot.csv"), encoding="utf-8")):
    tu[r["owner"]].add(r["source"].lower())
prod = {r["owner"]: r for r in csv.DictReader(open(os.path.join(osaa, "tuotteet.csv"), encoding="utf-8"))}

units = json.load(open(os.path.join(root, "doc/arkkitehtuuri/todisteet/c5/msvc/units.json")))["units"]
mapn = {"HkMaali": "HkMaali", "JukMaali": "ViestiMaali", "JukLuenta": "ViestiLuenta",
        "tputilv2": "tputilv2", "dbboxm": "dbboxm"}
ok = True
print("MSVC (C-5 recorded command lines vs. project evaluation)")
for k, v in units.items():
    vd = os.path.dirname(v["vcxproj"].replace("C:\\dev\\tulospalvelu", root))
    rec = set(rel(os.path.join(vd, s.strip('"'))) for s in v["srcs"])  # one C-5 entry has a stray quote
    mine = tu[mapn[k]]
    o = v["opts"]
    d = set(o[i + 1] for i, x in enumerate(o) if x == "/D")
    d |= set(x[2:] for x in o if x.startswith("/D") and len(x) > 2)
    md = set(x for x in prod[mapn[k]]["defines"].split(";") if x)
    md -= {"%(PreprocessorDefinitions)"}
    print("  %-10s sources recorded %d, script %d; only recorded %s; only script %s" % (
        k, len(rec), len(mine), sorted(rec - mine), sorted(mine - rec)))
    print("             defines only recorded %s; only script %s" % (sorted(d - md), sorted(md - d)))
    print("             runtime script=%s recorded=%s" % (prod[mapn[k]]["runtime"],
                                                       [x for x in o if x.startswith(("/MT", "/MD"))]))
    if rec != mine:
        ok = False

print("bcc32 (C-5 recorded translation units vs. project evaluation)")
ana = [f for f in os.listdir(bda) if f.startswith("ana_") and f.endswith(".json")][0]
a = json.load(open(os.path.join(bda, ana)))
rec = collections.defaultdict(set)
for x in a:
    rec[x["proj"]].add(rel(os.path.join(src, "RADStudio10", x["src"])))
for p in sorted(rec):
    print("  %-10s recorded %d, script %d; only recorded %s; only script %s" % (
        p, len(rec[p]), len(tu[p]), sorted(rec[p] - tu[p]), sorted(tu[p] - rec[p])))
    if rec[p] != tu[p]:
        ok = False
print("RESULT:", "match" if ok else "DIFFERENCES")
