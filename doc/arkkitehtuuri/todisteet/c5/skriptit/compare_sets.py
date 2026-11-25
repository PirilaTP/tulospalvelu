"""C-5: stricter cross-unit check of MSVC layouts. For each type key (name +
member names) the SET of layouts seen in a unit must be the same in every unit
of the program that has the key (a unit may legitimately define several anonymous
types with the same member names).
usage: python -I compare_sets.py <msvc-outdir>   (run from the script directory)
"""
import os, sys, glob, collections
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
src = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'compare_layout.py')).read()
ns = {}
exec(src.split('layouts = {}')[0].replace("outdir = sys.argv[1]", "outdir = None").replace("for a in sys.argv[2:]:", "for a in []:"), ns)
parse = ns['parse']
outdir = sys.argv[1]
progs = {'HkMaali': ['HkMaali', 'tputilv2', 'dbboxm'],
         'ViestiMaali': ['JukMaali', 'tputilv2', 'dbboxm'],
         'ViestiLuenta': ['JukLuenta', 'tputilv2', 'dbboxm']}
L = {}
for p in ['HkMaali', 'JukMaali', 'JukLuenta', 'tputilv2', 'dbboxm']:
    for f in sorted(glob.glob(os.path.join(outdir, p, '*.txt'))):
        L[p + '/' + os.path.basename(f)[:-4]] = parse(f)
for prog, projs in progs.items():
    per = collections.defaultdict(dict)
    for u, types in L.items():
        if u.split('/')[0] not in projs:
            continue
        d = collections.defaultdict(set)
        for name, size, mems in types:
            d[(name, tuple(t for _, t in mems))].add((size, tuple(o for o, _ in mems)))
        for k, v in d.items():
            per[k][u] = frozenset(v)
    bad = []
    for k, m in per.items():
        g = collections.defaultdict(list)
        for u, s in m.items():
            g[s].append(u)
        if len(g) > 1:
            bad.append((k, g))
    print('== %s: %d type keys, %d inconsistent' % (prog, len(per), len(bad)))
    for k, g in bad:
        print('  %s %s' % (k[0], list(k[1][:8])))
        for s, us in g.items():
            print('     sizes %s offsets-with-None %s  %d units %s' % (sorted(x[0] for x in s), any(None in x[1] for x in s), len(us), us[:4]))
