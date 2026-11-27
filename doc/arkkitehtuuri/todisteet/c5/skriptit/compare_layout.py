"""C-5: compare MSVC class layouts across translation units of one program.

usage: python -I compare_layout.py <msvc-outdir> <program>=<proj>+<proj>... [...]
Prints, per program, every type (name + member list) whose size/offsets differ
between units, with the units grouped by layout. Writes layouts.json.
"""
import os, re, sys, json, glob, collections

outdir = sys.argv[1]
progs = {}
for a in sys.argv[2:]:
    name, projs = a.split('=')
    progs[name] = projs.split('+')

HEAD = re.compile(r'^class (.+?)\tsize\((\d+)\):')
MEM = re.compile(r'^\s*(\d+)?\s*\|(.*)$')

def parse(path):
    types = []
    lines = open(path, encoding='cp1252', errors='replace').read().splitlines()
    i = 0
    while i < len(lines):
        m = HEAD.match(lines[i])
        if not m:
            i += 1; continue
        name, size = m.group(1), int(m.group(2))
        i += 1
        bars = 0
        mems = []
        while i < len(lines):
            l = lines[i]
            if l.strip() == '+---':
                bars += 1
                i += 1
                if bars == 2:
                    break
                continue
            mm = MEM.match(l)
            if mm:
                off = mm.group(1)
                txt = re.sub(r'\s+', ' ', mm.group(2).strip())
                if not txt.startswith('<alignment member>'):
                    mems.append((int(off) if off else None, txt))
            elif l.strip() == '':
                break
            i += 1
        types.append((name, size, tuple(mems)))
    return types

layouts = {}
for p in sorted({x for v in progs.values() for x in v}):
    for f in sorted(glob.glob(os.path.join(outdir, p, '*.txt'))):
        layouts[p + '/' + os.path.basename(f)[:-4]] = parse(f)

report = {}
for prog, projs in progs.items():
    # key -> layout -> [units]
    tab = collections.defaultdict(lambda: collections.defaultdict(list))
    for unit, types in layouts.items():
        if unit.split('/')[0] not in projs:
            continue
        seen = set()
        for name, size, mems in types:
            key = (name, tuple(t for _, t in mems))
            lay = (size, tuple(o for o, _ in mems))
            if (key, lay) in seen:
                continue
            seen.add((key, lay))
            tab[key][lay].append(unit)
    diffs = []
    for key, lays in tab.items():
        if len(lays) > 1:
            diffs.append({'name': key[0], 'members': list(key[1]),
                          'layouts': [{'size': l[0], 'offsets': list(l[1]), 'units': u}
                                      for l, u in sorted(lays.items(), key=lambda kv: -len(kv[1]))]})
    report[prog] = {'types': len(tab), 'differing': diffs}
    print('== %s: %d distinct types, %d with differing layout' % (prog, len(tab), len(diffs)))
    for d in diffs:
        print('  %s (%d members)' % (d['name'], len(d['members'])))
        for l in d['layouts']:
            us = l['units']
            print('     size %d in %d units: %s%s' % (l['size'], len(us), ', '.join(us[:6]), ' ...' if len(us) > 6 else ''))
json.dump(report, open(os.path.join(outdir, 'layout_diffs.json'), 'w'), indent=1)
