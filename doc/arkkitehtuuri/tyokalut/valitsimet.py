#!/usr/bin/env python3
"""C-12 part A0: inventory of preprocessor switches (macros tested in #if).

Reuses the C-10 model (riippuvuudet.build_model): project evaluation, include
closure, tri-state #if evaluator and per-owner macro sets. For every macro that
is tested in an #if/#ifdef/#ifndef/#elif/defined() condition in any source or
header under TPsource/V52 it records:

  - test sites (file, line, directive, expression) and, per product, whether the
    branch is reachable (enclosing condition) and taken (branch condition);
  - definition sites: project defines per owner and configuration, and
    #define/#undef lines in the repo with their own condition;
  - the value per owner (5 products + 4 libraries) in the shipped
    configuration, and per other configuration where it differs;
  - derived definitions (#define X under a condition on Y -> X depends on Y);
  - include guards, TU-local definitions and never-defined macros;
  - dead branches: branches that are false in every product that contains the
    file, and files that no product contains.

The classification column is a suggestion from simple rules; part A of C-12
classifies the macros. The analysis is lexical (see riippuvuudet.py); other
configurations are evaluated with the same closure logic, libraries in Debug
only in the run that sets Debug for all owners.

Usage: python -I valitsimet.py <repo-root> <out-dir>
Writes: valitsimet.csv, testikohdat.csv, maaritykset.csv, kokoonpanot.csv,
        yhteenveto.txt
"""
import csv
import importlib.util
import os
import re
import sys
import xml.etree.ElementTree as ET
from collections import defaultdict, Counter

sys.dont_write_bytecode = True   # do not leave __pycache__ in the repo
HERE = os.path.dirname(os.path.abspath(__file__))
_spec = importlib.util.spec_from_file_location("riippuvuudet", os.path.join(HERE, "riippuvuudet.py"))
RV = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(RV)

PP_LINE = RV.PP_LINE
IDENT = re.compile(r"[A-Za-z_]\w*")
NOT_MACRO = {"defined"}
# Test projects: project defines only (not products), for kokoonpanot.csv
TEST_PROJECTS = [
    ("TpTest", "msvc", "VS/Tests/TpTest.vcxproj"),
    ("TpCoreHk", "msvc", "VS/Tests/TpCoreHk.vcxproj"),
    ("TpCoreJuk", "msvc", "VS/Tests/TpCoreJuk.vcxproj"),
]
COMPILER_NAMES = set(RV.PREDEF["msvc"]) | set(RV.PREDEF["bcc32"]) | {
    "WIN32", "_WIN64", "_DEBUG", "NDEBUG", "_LIB", "_MBCS", "__STDC__", "_WINDOWS", "__GNUC__",
    "__linux__", "_M_X64", "__clang__", "_CPPUNWIND", "__STDC_VERSION__", "_MSC_FULL_VER",
    "__WIN64__", "_Windows", "__DLL__", "_DLL", "__CONSOLE__", "__cplusplus_cli", "_ATL_VER",
    "WINVER", "_WIN32_WINNT", "_WIN32_IE", "STRICT", "__TCHAR_DEFINED", "_TCHAR_DEFINED",
    "_INC_TCHAR", "_CRT_SECURE_NO_DEPRECATE", "_CRT_NON_CONFORMING_SWPRINTFS",
    "_CRT_NON_CONFORMING_WCSTOK", "_CRT_SECURE_NO_WARNINGS"}


def project_configs(path):
    txt = open(path, "rb").read().decode("utf-8", errors="replace")
    if path.lower().endswith(".vcxproj"):
        return sorted(set(re.findall(r'ProjectConfiguration Include="([^"]+)"', txt)))
    cf = set(re.findall(r"Condition=\"'\$\(Config\)'=='([^']+)'", txt))
    cf.discard("Base")
    return sorted(cf)


def scan_tests(fi):
    """Directive sites in one file: list of dicts with line, directive, expr,
    macros, encl (tuple: condition to reach the directive), branch (tuple:
    condition for the branch body), end (last line of the branch body)."""
    sites = []
    stack = []
    guard_pending = fi.guard is not None
    open_sites = []   # per frame: index of the site whose branch is open
    logical = RV.join_continuations(fi.lines)
    for s, ln in logical:
        m = PP_LINE.match(s)
        if not m:
            continue
        d, rest = m.group(1), m.group(2).strip()
        if d in ("if", "ifdef", "ifndef"):
            if d == "ifdef":
                e = "defined(%s)" % rest.split()[0] if rest else "0"
            elif d == "ifndef":
                e = "!defined(%s)" % rest.split()[0] if rest else "0"
            else:
                e = rest
            is_guard = False
            if guard_pending and not stack:
                guard_pending = False
                is_guard = True
            encl = RV.FileInfo._cond_tuple(stack)
            stack.append({"prev": [], "cur": "1" if is_guard else e, "sites": []})
            site = dict(line=ln + 1, directive=d, expr=rest, cexpr=e, encl=encl,
                        branch=RV.FileInfo._cond_tuple(stack), guard=is_guard, end=None)
            sites.append(site)
            stack[-1]["sites"].append(site)
            open_sites.append(site)
        elif d in ("elif", "else"):
            if not stack:
                continue
            if open_sites and open_sites[-1]["end"] is None:
                open_sites[-1]["end"] = ln
            fr = stack[-1]
            fr["prev"].append(fr["cur"])
            fr["cur"] = rest if d == "elif" else None
            site = dict(line=ln + 1, directive=d, expr=rest, cexpr=rest if d == "elif" else "",
                        encl=RV.FileInfo._cond_tuple(stack[:-1]), branch=RV.FileInfo._cond_tuple(stack),
                        guard=False, end=None)
            sites.append(site)
            fr["sites"].append(site)
            open_sites[-1] = site
        elif d == "endif":
            if stack:
                for st in stack.pop()["sites"]:
                    st["frame_end"] = ln + 1
                if open_sites:
                    st = open_sites.pop()
                    if st["end"] is None:
                        st["end"] = ln
    for st in sites:
        st["macros"] = sorted(set(x for x in IDENT.findall(st["cexpr"]) if x not in NOT_MACRO))
        if st["end"] is None:
            st["end"] = len(fi.lines)
        st.setdefault("frame_end", len(fi.lines))
        # body lines of the branch, excluding the directive itself
        st["body"] = max(0, st["end"] - st["line"])
    return sites


def fmt_val(o, name, defs_unk):
    d, unk = defs_unk
    if name in d:
        return str(d[name])
    if name in unk:
        return "?"
    return "-"


def main():
    root, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    base = RV.build_model(root)
    src = base.src
    owners = list(base.owners)
    pnames = base.pnames

    # ---------------------------------------------- other configurations
    cfgs = {}
    for name, comp, rel, cfg, libs in RV.PRODUCTS:
        cfgs[name] = (rel, base.owners[name].get("config") or cfg, project_configs(os.path.join(src, rel)))
    for name, (comp, rel, cfg) in RV.LIBS.items():
        cfgs[name] = (rel, base.owners[name].get("config") or cfg, project_configs(os.path.join(src, rel)))
    alt = {o: [c for c in cfgs[o][2] if c != cfgs[o][1]] for o in owners}
    # run 0 = the one with "Debug" for every owner that has one, then the rest in order
    runs = []
    nruns = max(len(v) for v in alt.values())
    ordered = {}
    for o, lst in alt.items():
        dbg = [c for c in lst if c.split("|")[0] in ("Debug", "Debug Build")]
        ordered[o] = dbg + [c for c in lst if c not in dbg]
    for i in range(nruns):
        ov = {o: lst[i] for o, lst in ordered.items() if i < len(lst)}
        runs.append(ov)
    alt_defs = defaultdict(dict)   # owner -> cfg -> (defs, unk)
    alt_proj = defaultdict(dict)   # owner -> cfg -> project defines list
    for ov in runs:
        m = RV.build_model(root, ov)
        for o, c in ov.items():
            alt_defs[o][c] = m.odefs[o]
            alt_proj[o][c] = m.owners[o]["defines"]

    # ------------------------------------------------- sites and defines
    cache = base.cache
    sites_by_file = {}
    for p, fi in cache.items():
        if fi is None:
            continue
        sites_by_file[p] = scan_tests(fi)

    def rel(p):
        return os.path.relpath(p, src).replace("\\", "/")

    def in_products(p):
        return [pn for pn in pnames if any(base.owner_files[o].get(p) for o in base.product_owners[pn])]

    # Default-value blocks "#ifndef X / #define X v / #endif": the owner macro set
    # already contains X from this very block, which would make the block look
    # dead. Inside such a block X is evaluated as it stands before the block:
    # project define, or an unconditional #define in another header of the
    # owner's closure; otherwise undefined.
    defaulted = defaultdict(list)   # file -> [(X, first line, #endif line)]
    for p, sl in sites_by_file.items():
        fi = cache[p]
        for st in sl:
            mm = re.match(r"^!defined\((\w+)\)$", st["cexpr"].replace(" ", ""))
            if st["directive"] == "ifndef" and mm:
                x = mm.group(1)
                if any(n == x and k == "define" and st["line"] < ln <= st["end"]
                       for n, ln, c, k, v, fl in fi.defines):
                    defaulted[p].append((x, st["line"], st["frame_end"]))
    proj_level = {o: RV.define_map(base.owners[o]["defines"], base.owners[o]["compiler"], o) for o in owners}
    hdr_uncond = defaultdict(set)   # macro -> files with an unconditional header #define
    for p, fi in cache.items():
        if fi and not p.lower().endswith(RV.SRC_EXT):
            for n, ln, c, k, v, fl in fi.defines:
                if k == "define" and not c:
                    hdr_uncond[n].add(p)

    def to_int(v):
        try:
            return int(re.sub(r"[uUlL]+$", "", v), 0) if v else 1
        except ValueError:
            return 1

    def defs_at(p, o, conds, line):
        d, unk = base.local_defs(p, *base.odefs[o])
        fi = cache[p]
        if p.lower().endswith(RV.SRC_EXT) and fi:
            # TU-local "#ifndef X / #define X" counts as a definition (C-10 made X unknown)
            for n, ln, c, k, v, fl in fi.defines:
                if k == "define" and len(c) == 1 and c[0].replace(" ", "") == "!defined(%s)" % n \
                        and n not in base.odefs[o][0]:
                    d = dict(d)
                    d[n] = to_int(v)
                    unk = set(unk) - {n}
        flat = " ".join(conds).replace(" ", "")
        for x, b0, b1 in defaulted.get(p, ()):
            if not (b0 <= line <= b1) or "defined(%s)" % x not in flat:
                continue
            d, unk = dict(d), set(unk) - {x}
            if x in proj_level[o]:
                d[x] = proj_level[o][x]
            elif any(f != p and f in base.owner_files[o] for f in hdr_uncond.get(x, ())):
                pass   # defined before by another header (order not modelled)
            else:
                d.pop(x, None)
        return d, unk

    def eval_in_product(p, conds, pn, line):
        """Per product: '' not in product, else y/n/? (any owner y -> y)."""
        res = []
        for o in base.product_owners[pn]:
            if not base.owner_files[o].get(p):
                continue
            d, unk = defs_at(p, o, conds, line)
            res.append(RV.eval_cond_tuple(conds, d, unk))
        if not res:
            return ""
        if True in res:
            return "y"
        if None in res:
            return "?"
        return "n"

    site_rows = []
    tested = defaultdict(list)   # macro -> list of (file, site)
    dead_rows = []
    for p in sorted(sites_by_file):
        r = rel(p)
        prods = in_products(p)
        for st in sites_by_file[p]:
            reach = {pn: eval_in_product(p, st["encl"], pn, st["line"]) for pn in pnames}
            take = {pn: eval_in_product(p, st["branch"], pn, st["line"]) for pn in pnames}
            present = [pn for pn in pnames if take[pn]]
            if not present:
                status = "not-in-product"
            elif all(take[pn] == "n" for pn in present):
                status = "dead" if any(reach[pn] != "n" for pn in present) else "unreachable"
            elif all(take[pn] == "y" for pn in present):
                status = "always"
            elif any(take[pn] == "?" for pn in present):
                status = "unknown"
            else:
                status = "varies"
            site_rows.append([r, st["line"], st["directive"], st["expr"][:160], ";".join(st["macros"]),
                              "guard" if st["guard"] else "", " && ".join("(%s)" % c for c in st["encl"])[:300],
                              st["body"]] + [reach[pn] for pn in pnames] + [take[pn] for pn in pnames] + [status])
            for mname in st["macros"]:
                tested[mname].append((p, st, status, take))

    # definitions
    macro_defs = defaultdict(list)
    for p, fi in cache.items():
        if not fi:
            continue
        for name, ln, cond, kind, val, fnlike in fi.defines:
            macro_defs[name].append((p, ln, cond, kind, val, fnlike))
    guards = set(fi.guard for fi in cache.values() if fi and fi.guard)

    # project defines per owner/config
    proj_def = defaultdict(list)   # macro -> ["owner:cfg=val"]
    for o in owners:
        allc = [(cfgs[o][1], base.owners[o]["defines"])] + sorted(alt_proj[o].items())
        for c, dl in allc:
            for x in dl:
                x = x.strip()
                if not x:
                    continue
                k = x.split("=", 1)[0].strip()
                v = x.split("=", 1)[1].strip() if "=" in x else ""
                proj_def[k].append("%s:%s%s" % (o, c, "=" + v if v else ""))

    def suggest(name):
        defs = macro_defs.get(name, [])
        hdr_defs = [x for x in defs if not x[0].lower().endswith(RV.SRC_EXT)]
        vals = [x[4] for x in defs if x[3] == "define"]
        if name in guards:
            return "include-guard"
        if name in COMPILER_NAMES or (name.startswith("__") and name not in proj_def):
            return "compiler-platform"
        if name in ("_CONSOLE", "_BORLAND_"):
            return "variant"
        if not defs and name not in proj_def:
            return "never-defined"
        if defs and all(x[0].lower().endswith(RV.SRC_EXT) for x in defs) and name not in proj_def:
            return "tu-local"
        numeric = [v for v in vals if re.match(r"^\(?\s*-?(0[xX])?[0-9A-Fa-f]+[uUlL]*\s*\)?$", v) and v not in ("1", "0")]
        if any(x[3] == "define" and x[2] and x[2][-1].replace(" ", "") == "!defined(%s)" % name for x in hdr_defs) \
                and numeric:
            return "default-constant"
        if numeric:
            return "constant"
        if any(x[5] for x in defs):
            return "function-macro"
        return "feature"

    pcols = owners
    macro_rows = []
    for name in sorted(tested, key=str.lower):
        lst = tested[name]
        files = Counter(rel(p) for p, st, s, t in lst)
        stat = Counter(s for p, st, s, t in lst)
        n_in_prod = sum(1 for p, st, s, t in lst if s != "not-in-product")
        vals = {o: fmt_val(o, name, base.odefs[o]) for o in owners}
        altv = []
        for o in owners:
            for c, du in sorted(alt_defs[o].items()):
                v = fmt_val(o, name, du)
                if v != vals[o]:
                    altv.append("%s:%s=%s" % (o, c, v))
        defs = macro_defs.get(name, [])
        dsites = ["%s:%d%s%s%s" % (rel(p), ln, " undef" if kind == "undef" else "",
                                   "=" + val[:30] if val and kind == "define" else "",
                                   " if " + " && ".join(cond)[:80] if cond else "")
                  for p, ln, cond, kind, val, fnl in defs]
        derived = sorted(set(x for p, ln, cond, kind, val, fnl in defs
                             if kind == "define" for c in cond for x in IDENT.findall(c)
                             if x not in NOT_MACRO and x != name))
        pv = [vals[o] for o in pnames]
        uniform = "yes" if len(set(pv)) == 1 else "no"
        macro_rows.append([name, suggest(name), len(lst), len(files), n_in_prod,
                           ";".join("%s(%d)" % kv for kv in files.most_common(12)) + (";..." if len(files) > 12 else ""),
                           ";".join("%s=%d" % kv for kv in sorted(stat.items()))] +
                          [vals[o] for o in pcols] + [uniform, ";".join(altv),
                           ";".join(sorted(set(proj_def.get(name, [])))), len(defs), " | ".join(dsites[:10]) +
                           (" | ..." if len(dsites) > 10 else ""), ";".join(derived)])

    with open(os.path.join(out, "valitsimet.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["macro", "suggested_class", "n_test_sites", "n_files", "n_sites_in_products", "files",
                    "site_status"] + ["val_%s" % o for o in pcols] +
                   ["uniform_in_products", "other_config_values", "project_defines", "n_define_sites",
                    "define_sites", "defined_under"])
        w.writerows(macro_rows)

    with open(os.path.join(out, "testikohdat.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["file", "line", "directive", "expr", "macros", "guard", "enclosing", "body_lines"] +
                   ["reach_%s" % p for p in pnames] + ["take_%s" % p for p in pnames] + ["status"])
        w.writerows(site_rows)

    with open(os.path.join(out, "maaritykset.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["macro", "file", "line", "kind", "value", "function_like", "condition", "tested"])
        for name in sorted(macro_defs, key=str.lower):
            for p, ln, cond, kind, val, fnl in macro_defs[name]:
                w.writerow([name, rel(p), ln, kind, val[:120], "yes" if fnl else "", " && ".join(cond)[:200],
                            "yes" if name in tested else ""])

    with open(os.path.join(out, "kokoonpanot.csv"), "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["owner", "project", "config", "shipped", "defines", "added_vs_shipped", "removed_vs_shipped"])
        for o in owners:
            rl, shipped, allc = cfgs[o]
            sd = base.owners[o]["defines"]
            w.writerow([o, rl, shipped, "yes", ";".join(sd), "", ""])
            for c in allc:
                if c == shipped:
                    continue
                dl = alt_proj[o].get(c)
                if dl is None:
                    w.writerow([o, rl, c, "", "NOT EVALUATED", "", ""])
                    continue
                w.writerow([o, rl, c, "", ";".join(dl), ";".join(sorted(set(dl) - set(sd))),
                            ";".join(sorted(set(sd) - set(dl)))])
        for o, comp, rl in TEST_PROJECTS:
            path = os.path.join(src, rl)
            if not os.path.exists(path):
                continue
            for c in project_configs(path):
                info = RV.project_info(src, comp, rl, c)
                w.writerow([o, rl, c, "", ";".join(info["defines"]), "", ""])

    # dead branch summary per file
    with open(os.path.join(out, "yhteenveto.txt"), "w", encoding="utf-8") as f:
        f.write("C-12 part A0 switch inventory (valitsimet.py)\n\n")
        f.write("configurations evaluated (shipped first):\n")
        for o in owners:
            f.write("  %-12s shipped=%s other=%s\n" % (o, cfgs[o][1], ", ".join(ordered[o]) or "-"))
        f.write("\nruns: %s\n" % runs)
        f.write("\ntested macros: %d   test sites: %d   files with sites: %d\n" % (
            len(tested), len(site_rows), sum(1 for v in sites_by_file.values() if v)))
        f.write("macros #defined or #undef'd anywhere: %d\n" % len(macro_defs))
        cls = Counter(r[1] for r in macro_rows)
        f.write("\nsuggested classes: %s\n" % dict(sorted(cls.items())))
        st = Counter(r[-1] for r in site_rows)
        f.write("site status: %s\n" % dict(sorted(st.items())))
        dead_lines = sum(r[7] for r in site_rows if r[-1] in ("dead", "unreachable"))
        f.write("body lines in dead/unreachable branches (in products, nested counted twice): %d\n" % dead_lines)
        f.write("\nunknown (?) macros per owner:\n")
        for o in owners:
            f.write("  %s: %s\n" % (o, ";".join(sorted(base.odefs[o][1]))))
        f.write("\nfeature-like macros with values differing between products:\n")
        for r in macro_rows:
            if r[1] in ("feature", "variant", "default-constant", "constant") and r[7 + len(pcols)] == "no":
                f.write("  %-16s %s\n" % (r[0], " ".join("%s=%s" % (o, r[7 + i]) for i, o in enumerate(pnames))))


if __name__ == "__main__":
    main()
