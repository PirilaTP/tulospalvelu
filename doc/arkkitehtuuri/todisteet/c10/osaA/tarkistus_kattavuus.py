#!/usr/bin/env python3
"""C-10 part A, script check 2: marker coverage and unknown conditions.

(a) For a set of key API names, compare raw `grep -w` style occurrences in the
    scanned sources (including comments) with rows in osumat.csv; the difference
    must be explained by comments (the script strips comments) or by files that
    are not scanned (Tests/, VS/).
(b) Which macros make conditions unknown, per product.
(c) Rows whose file is compiled/included by no product (dead code for products).

Usage: python -I tarkistus_kattavuus.py <repo-root> <osaA-dir>
"""
import collections
import csv
import os
import re
import sys

sys.path.insert(0, os.path.join(sys.argv[1], "doc", "arkkitehtuuri", "tyokalut"))
from riippuvuudet import read_text, strip_comments_keep_strings  # noqa: E402

root, osaa = sys.argv[1], sys.argv[2]
src = os.path.join(root, "TPsource", "V52")
rows = list(csv.DictReader(open(os.path.join(osaa, "osumat.csv"), encoding="utf-8")))
P = ["HkMaali", "ViestiMaali", "ViestiLuenta", "HkKisaWin", "ViestiWin"]

KEYS = ["WinHttpOpen", "WinHttpSendRequest", "WinHttpSetTimeouts", "WinHttpSetOption",
        "TSQLConnection", "TWebBrowser", "SetCommState", "WSAStartup", "RegOpenKeyEx",
        "mysql_real_connect", "LID_EMIT", "ExecSQL", "Navigate", "CoInitialize"]
raw = collections.Counter()
nocomment = collections.Counter()
for dp, dns, fns in os.walk(src):
    reld = os.path.relpath(dp, src).replace(os.sep, "/")
    if reld.split("/")[0] in ("Tests", "VS", "RADStudio10"):
        continue
    dns[:] = [d for d in dns if d not in ("obj", "Win32", "__history", "Release", "Asan")]
    for fn in fns:
        if not fn.lower().endswith((".cpp", ".c", ".h", ".hpp")):
            continue
        t = read_text(os.path.join(dp, fn))
        c = strip_comments_keep_strings(t)
        for k in KEYS:
            raw[k] += len(re.findall(r"\b%s\b" % k, t))
            nocomment[k] += len(re.findall(r"\b%s\b" % k, c))
tok = collections.Counter(r["token"] for r in rows)
print("(a) key           raw  outside-comments  osumat.csv")
bad = 0
for k in KEYS:
    flag = "" if tok[k] == nocomment[k] else "  <-- DIFF"
    bad += bool(flag)
    print("    %-20s %4d %6d %6d%s" % (k, raw[k], nocomment[k], tok[k], flag))
print("    differences:", bad)

print("(b) macros behind unknown conditions")
mk = {r["owner"]: set(r["unknown"].split(";")) for r in
      csv.DictReader(open(os.path.join(osaa, "makrot.csv"), encoding="utf-8"))}
for p in P:
    c = collections.Counter()
    for r in rows:
        if r[p + "_active"] == "unknown":
            names = set(re.findall(r"\w+", r["condition"]))
            for o in [p, "tputilv2", "dbboxm", "Tputil-XE", "DBboxm-XE"]:
                for n in names & mk.get(o, set()):
                    c[n] += 1
    print("    %-12s %s" % (p, c.most_common(12)))

print("(c) marker rows (non-include) in files no product compiles or includes")
c = collections.Counter()
for r in rows:
    if r["category"] == "sys-include":
        continue
    if not any(r[p] for p in P):
        c[r["file"]] += 1
for f, n in c.most_common():
    print("    %-45s %d" % (f, n))
