#!/usr/bin/env python3
"""C-13 A0: check the output of tyokalut/synteesi.py before it is used.

Usage: python -I tarkistus_a0.py <repo_root>

The expected values below were typed by hand from CHECKPOINT.md and seuranta.md
at commit 3f378e2 (not produced by synteesi.py), so that the comparison is
independent of the parser. Differences are printed; the main session judges
each one against the register row (see README.md in this directory).
"""
import csv
import io
import os
import re
import sys

sys.dont_write_bytecode = True
root = sys.argv[1]
base = os.path.join(root, "doc", "arkkitehtuuri", "todisteet", "c13", "osaA0")


def load(name):
    with io.open(os.path.join(base, name), encoding="utf-8", newline="") as fh:
        return list(csv.DictReader(fh))


def ids(spec, kind="H-"):
    out = set()
    for part in spec.replace(",", " ").split():
        m = re.match(r"(\d+)-(\d+)$", part)
        if m:
            out |= {"%s%03d" % (kind, i) for i in range(int(m.group(1)), int(m.group(2)) + 1)}
        else:
            out.add("%s%03d" % (kind, int(part)))
    return out


def qids(spec):
    return {"Q%d" % int(x[2:]) for x in ids(spec, "Q-")}


H = {r["id"]: r for r in load("havainnot.csv")}
T = {r["id"]: r for r in load("tekninen_velka.csv")}
Q = {r["id"]: r for r in load("kysymykset.csv")}
diffs = 0


def compare(title, got, want, scope=None):
    global diffs
    if scope is not None:
        got = got & scope
    extra, missing = sorted(got - want), sorted(want - got)
    ok = not extra and not missing
    print("%-58s %s" % (title, "OK (%d)" % len(want) if ok else "DIFF"))
    if extra:
        print("    in base, not expected:", " ".join(extra))
    if missing:
        print("    expected, not in base:", " ".join(missing))
    if not ok:
        diffs += 1


# --- counts (CHECKPOINT.md "Kattavuus") -------------------------------------
print("counts: H %d (expect 177), T %d (expect 52), Q %d (expect 112)" % (len(H), len(T), len(Q)))
if (len(H), len(T), len(Q)) != (177, 52, 112):
    diffs += 1

# --- high severity (CHECKPOINT.md "Vakavimmat": 26 after C-6, then C-7..C-12)
high = ids("1 2 7 8 9 14 22 23 24 25 26 27 31 32 40 41 42 55 56 70 71 72 87 90 91 93 "
           "109 110 111 129 149 159 160 171")
compare("high severity = documented 34", {i for i, r in H.items() if r["vakavuus"] == "korkea"}, high)
compare("refuted = H-099", {i for i, r in H.items() if r["tila"] == "kumottu"}, ids("99"))

# --- review state of H-001..H-108 (CHECKPOINT.md "Havainnot" table) ---------
scope = ids("1-108")
# H-017: CHECKPOINT.md at 3f378e2 still lists it as "not reviewed", but the register row says
# "riippumaton lahdetarkastus (C-7)" (narrowed and reviewed in C-7). The register is the source
# of truth; this check found the stale list, and CHECKPOINT.md is corrected in C-13 part I.
full = ids("17 7 8 9 11-16 18 22-29 31 33 36 37 40-43 45 46 48 50 53 54 55 56 58 59 61 62 64 66-73 75 76 "
           "4 5 78 79 81 82 83 84 86 87 88 90-94 97-99 103-107")
partly = ids("10 21 30 32 34 35 44 47 49 51 52 57 60 63 65 74 77 80 85 38 89 95 96 100-102")
none = ids("1-3 6 19 39 108")
compare("H-001..108 independently reviewed", {i for i, r in H.items() if r["tarkastus"] == "riippumaton"}, full, scope)
compare("H-001..108 reviewed partly", {i for i, r in H.items() if r["tarkastus"] == "riippumaton_osittain"}, partly, scope)
compare("H-001..108 not reviewed", {i for i, r in H.items() if r["tarkastus"] == "ei"}, none, scope)
compare("not statically decidable = H-020", {i for i, r in H.items() if r["tarkastus"] == "ei_ratkea_staattisesti"}, ids("20"))
# later batches: single-agent rows named in the batch closings
compare("H-109..177 not reviewed (C-7, C-8 closings)", {i for i, r in H.items() if r["tarkastus"] == "ei"},
        ids("122 123 124 126 127 142 143 144"), ids("109-177"))

# --- experiments ------------------------------------------------------------
harness = ids("25 40 42 31 8 24 27 26 32 22 23 7 1 2")       # C-11, without H-041
probes = ids("90 91 93 94 128 129 145 146 147 149 150 151")  # C-6, C-8, C-9
compare("probe or harness (C-11 14 + C-6 4 + C-8 2 + C-9 6)",
        {i for i, r in H.items() if r["koe"] == "koe_tai_testikehys"}, harness | probes)
compare("shipped program = H-041", {i for i, r in H.items() if r["koe"] == "toimitettu_ohjelma"}, ids("41"))

# --- priorities (seuranta.md fix queue, muistiturvallisuus.md 15) -----------
want_p = {
    "P1": ids("71 72 32 61"),
    "P2": ids("55 70 129 56 60 73 159 160 171 149 150 87 91 90 93 107 109 110"),
    "P3": ids("41 42 1 59 76 62 162 169 164 166 163 161 165 134 172 173 174 145 146 151 147 "
              "94 103 104 105 106 88 89 92 97 111-117"),
    "P4": ids("2 57 58 74 65 63 64 75 52 49 167 168 175 148 152 153 154 155 95 96 98 100 101 102 108 "
              "118-125 78 79 82 85"),
    "P5": ids("66 67 68 69 77 3 170 176 177 156 157 158 126 127 80 81 83 84 86"),
}
for p in sorted(want_p):
    compare("current priority %s" % p, {i for i, r in H.items() if r["prioriteetti_nykyinen"] == p}, want_p[p])
allp = set().union(*want_p.values())
compare("no priority recorded yet (62)", {i for i, r in H.items() if not r["prioriteetti_nykyinen"]}, set(H) - allp)

# --- questions (CHECKPOINT.md, seuranta.md) ---------------------------------
compare("Q resolved", {i for i, r in Q.items() if r["tila"] == "ratkaistu"},
        qids("1 2 4 5 7 8 13 16 17 18 30 57 58 59 61 62 65 66 69 70 71 72 74 83 84 85 88 92 "
             # marked resolved in seuranta.md by C-13 part I (osaA-kattavuus.md section 7, class 1)
             "3 24 26 53"))
compare("Q partly", {i for i, r in Q.items() if r["tila"] == "osittain"}, qids("14 15 67 86 91"))
compare("Q owner decision", {i for i, r in Q.items() if r["tila"] == "omistajan_paatos"}, qids("60 101 111"))
compare("Q moved to run-time verification", {i for i, r in Q.items() if r["tila"] == "ajonaikainen_todennus"},
        qids("107 108 110"))

# --- earlier fix packages (batch closings) ----------------------------------
kk = {}
for r in load("kk_aiemmat.csv"):
    kk.setdefault(r["kk"], set()).add(r["id"])
want_kk = {
    "KK-C9-1": ids("149 151 152 158") | ids("42", "T-"),
    "KK-C9-2": ids("15 145 146 147 148 153 156 134") | ids("40 41", "T-"),
    "KK-C9-3": ids("150 155"),
    "KK-C10-1": ids("160 161 162 163 169 170") | ids("43", "T-"),
    "KK-C10-2": ids("159 164 166 167 170") | ids("48", "T-"),
    "KK-C10-3": ids("165") | ids("5 44 46 49", "T-"),
    "KK-C10-4": ids("166 161") | ids("43", "T-"),
    "KK-C12-1": ids("172 149 150 176"),
    "KK-C12-2": ids("171 173 36 176") | ids("51", "T-"),
    "KK-C12-3": ids("177") | ids("50 52", "T-"),
}
compare("earlier package identifiers", set(kk), set(want_kk))
for k in sorted(want_kk):
    compare("members of %s" % k, kk.get(k, set()), want_kk[k])

# --- macro evaluator artefact (ominaisuusvalitsimet.md 13.1) -----------------
arte = load("makroartefaktit.csv")
tested = {r["makro"] for r in arte if r["testattu"] == "yes"}
print("artefact: tested non-literal names %d (C-12 closing: 22, of which DOCTEST_ %d, expect 13)"
      % (len(tested), sum(1 for n in tested if n.startswith("DOCTEST_"))))
if len(tested) != 22:
    diffs += 1
mp = [r for r in arte if r["makro"] == "MAX_PORTTI" and r["tiedosto"] == "Juk/VDef.h"]
print("artefact: relay MAX_PORTTI listed: %s (%s)" % (bool(mp), mp[0]["lauseke"] if mp else "-"))
if not mp:
    diffs += 1

print("differences: %d" % diffs)
sys.exit(1 if diffs else 0)
