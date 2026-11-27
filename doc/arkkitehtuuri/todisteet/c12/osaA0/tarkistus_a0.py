"""C-12 A0 validation: valitsimet.py output vs C-10 makrot.csv, inventaario.md
switch table, raw directive counts and raw per-configuration project defines."""
import csv, os, re, sys, glob
import xml.etree.ElementTree as ET
from collections import defaultdict

repo, out, c10 = sys.argv[1], sys.argv[2], sys.argv[3]
src = os.path.join(repo, "TPsource", "V52")
ok = True

# 1. Release values vs C-10 makrot.csv
mk = {}
for r in csv.DictReader(open(os.path.join(c10, "makrot.csv"), encoding="utf-8")):
    d = dict(x.split("=", 1) for x in r["defined_known"].split(";") if x)
    mk[r["owner"]] = (d, set(x for x in r["unknown"].split(";") if x))
rows = list(csv.DictReader(open(os.path.join(out, "valitsimet.csv"), encoding="utf-8")))
bad = 0
for r in rows:
    for o, (d, u) in mk.items():
        exp = d.get(r["macro"], "?" if r["macro"] in u else "-")
        if r["val_" + o] != exp:
            bad += 1
            if bad < 10:
                print("  MISMATCH", r["macro"], o, r["val_" + o], exp)
print("1. Release values vs C-10 makrot.csv: %d rows x %d owners, mismatches %d" % (len(rows), len(mk), bad))
ok &= bad == 0

# 2. union of project defines over all configs vs inventaario.md table
inv = {}
txt = open(os.path.join(repo, "doc/arkkitehtuuri/inventaario.md"), encoding="utf-8").read()
for m in re.finditer(r"^\| (HkKisaWin|ViestiWin|HkMaali|JukMaali|JukLuenta|tputilv2 \(MSVC\)|Tputil-XE \(C\+\+Builder\)) \| `([^`]*)`", txt, re.M):
    inv[m.group(1).split()[0]] = set(x.split("=")[0] for x in m.group(2).split())
name_map = {"HkKisaWin": "HkKisaWin", "ViestiWin": "ViestiWin", "HkMaali": "HkMaali",
            "JukMaali": "ViestiMaali", "JukLuenta": "ViestiLuenta", "tputilv2": "tputilv2", "Tputil-XE": "Tputil-XE"}
kp = list(csv.DictReader(open(os.path.join(out, "kokoonpanot.csv"), encoding="utf-8")))
for invn, own in name_map.items():
    u = set()
    for r in kp:
        if r["owner"] == own:
            u |= set(x.split("=")[0] for x in r["defines"].split(";") if x)
    u -= {"NDEBUG", "_DEBUG", "UNICODE", "_UNICODE", "_MBCS", "_CRT_SECURE_NO_DEPRECATE",
          "_CRT_NON_CONFORMING_SWPRINTFS", "_CRT_NON_CONFORMING_WCSTOK", "_LIB", "_CRT_SECURE_NO_WARNINGS"}
    e = inv.get(invn, set()) - {"UNICODE", "_UNICODE", "_MBCS", "_LIB"}
    print("2. %-10s script-only=%s  table-only=%s" % (invn, sorted(u - e), sorted(e - u)))

# 3. raw directive count vs sites
raw = 0
for p in glob.glob(os.path.join(src, "**", "*"), recursive=True):
    if not p.lower().endswith((".cpp", ".c", ".h", ".hpp")):
        continue
    parts = p.replace("\\", "/").split("/")
    if any(x in (".git", "obj", "Win32", "__history", "build-logs", "Release", "Asan") for x in parts):
        continue
    t = open(p, "rb").read().decode("latin-1")
    t = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), t, flags=re.S)
    t = re.sub(r"//[^\n]*", "", t)
    raw += len(re.findall(r"^[ \t]*#[ \t]*(?:if|ifdef|ifndef|elif|else)\b", t, re.M))
sites = sum(1 for _ in open(os.path.join(out, "testikohdat.csv"), encoding="utf-8")) - 1
print("3. raw #if/#ifdef/#ifndef/#elif/#else (comments stripped) = %d, sites = %d" % (raw, sites))

# 4. per-configuration defines straight from the vcxproj XML (independent parse)
ns = "{http://schemas.microsoft.com/developer/msbuild/2003}"
for own, rel in [("HkMaali", "VS/Hk/HkMaali/HkMaali.vcxproj"), ("ViestiMaali", "VS/V/JukMaali520.vcxproj"),
                 ("ViestiLuenta", "VS/V/JukLuenta520.vcxproj"), ("tputilv2", "VS/Libs/tputilv2.vcxproj")]:
    root = ET.parse(os.path.join(src, rel)).getroot()
    for idg in root.iter(ns + "ItemDefinitionGroup"):
        m = re.search(r"=='([^']+)'", idg.get("Condition", ""))
        if not m:
            continue
        cfg = m.group(1)
        pd = idg.find(ns + "ClCompile/" + ns + "PreprocessorDefinitions")
        xml = set(x for x in (pd.text if pd is not None else "").split(";") if x and not x.startswith("%("))             - {"UNICODE", "_UNICODE", "_MBCS"}   # CharacterSet adds these on the script side
        scr = [r for r in kp if r["owner"] == own and r["config"] == cfg]
        s = set(x for x in scr[0]["defines"].split(";") if x) - {"UNICODE", "_UNICODE", "_MBCS"} if scr else None
        res = "OK" if s == xml else "DIFF script-only=%s xml-only=%s" % (sorted((s or set()) - xml), sorted(xml - (s or set())))
        print("4. %-12s %-20s %s" % (own, cfg, res))
        ok &= res == "OK"
print("RESULT", "OK" if ok else "CHECK")
