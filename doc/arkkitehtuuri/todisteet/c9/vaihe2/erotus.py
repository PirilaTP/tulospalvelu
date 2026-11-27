"""C-9 step 2, part 4: compare the v1 and v2 inventories of resurssit.py.

Reads todisteet/c9/osaA/kutsut.csv (v1) and osaA/v2/kutsut.csv (v2) and the
part B/C/D/E reports, and writes erotus.txt (counts) and
erotus_uudet_K.csv (v2-only rows in built core files that no report or
register row mentions, i.e. candidates for unexamined error paths).
Usage: python -I erotus.py   (run from this directory)
"""
import csv
import os
import re
from collections import Counter

HERE = os.path.dirname(os.path.abspath(__file__))
C9 = os.path.normpath(os.path.join(HERE, '..'))
DOC = os.path.normpath(os.path.join(C9, '..', '..'))


def load(p):
    with open(p, encoding='utf-8') as f:
        return list(csv.DictReader(f))


v1 = load(os.path.join(C9, 'osaA', 'kutsut.csv'))
v2 = load(os.path.join(C9, 'osaA', 'v2', 'kutsut.csv'))
k1 = {(r['file'], r['line'], r['call']): r for r in v1}
k2 = {(r['file'], r['line'], r['call']): r for r in v2}
only1 = [k1[k] for k in k1 if k not in k2]
only2 = [k2[k] for k in k2 if k not in k1]
both = [k for k in k1 if k in k2]
changed = Counter((k1[k]['use'], k2[k]['use']) for k in both if k1[k]['use'] != k2[k]['use'])

# text of every report and the register, for "is this site mentioned"
texts = ''
for name in ('osaA-luokittelu.md', 'osaB-pysyva-data.md', 'osaC-elinkaari.md',
             'osaD-virheen-jalkeen.md', 'osaE-tarkastus.md'):
    with open(os.path.join(C9, name), encoding='utf-8') as f:
        texts += f.read()
with open(os.path.join(DOC, 'rekisteri', 'havainnot.md'), encoding='utf-8') as f:
    texts += f.read()


def mentioned(r):
    base = r['file'].split('/')[-1]
    ln = int(r['line'])
    for m in re.finditer(re.escape(base) + r':(\d+)(?:-(\d+))?((?:, ?:?\d+(?:-\d+)?)*)', texts):
        nums = [m.group(1)] + ([m.group(2)] if m.group(2) else [])
        rest = re.findall(r'(\d+)(?:-(\d+))?', m.group(3) or '')
        ranges = [(int(m.group(1)), int(m.group(2) or m.group(1)))]
        ranges += [(int(a), int(b or a)) for a, b in rest]
        for a, b in ranges:
            if a - 5 <= ln <= b + 5:
                return True
    return False


core = ('Hk/', 'Juk/', 'Tp/', 'com/', 'tputilv2/', 'dbboxm/')
cand = [r for r in only2 if r['built'] == 'yes' and r['file'].startswith(core)
        and r['use'] not in ('checked', 'assigned-checked', 'returned')
        and r['group'] not in ('error',) and not mentioned(r)]

with open(os.path.join(HERE, 'erotus.txt'), 'w', encoding='utf-8', newline='\n') as f:
    f.write('v1 rows %d, v2 rows %d, common %d, only v1 %d, only v2 %d\n\n' %
            (len(v1), len(v2), len(both), len(only1), len(only2)))
    f.write('only in v1, by call (removed: definitions/prototypes S6, VCL Open/Close S5):\n')
    for c, n in Counter(r['call'] for r in only1).most_common():
        f.write('  %-16s %5d\n' % (c, n))
    f.write('\nonly in v2, by group/call (new names S2, S8; rows moved by S6 line fixes):\n')
    for (g, c), n in sorted(Counter((r['group'], r['call']) for r in only2).items()):
        f.write('  %-8s %-16s %5d\n' % (g, c, n))
    f.write('\ncommon rows whose use class changed (v1 -> v2):\n')
    for (a, b), n in changed.most_common():
        f.write('  %-20s -> %-20s %5d\n' % (a, b, n))
    f.write('\nv2-only rows in built core files, result not checked, not mentioned in any C-9 report '
            'or the register: %d (erotus_uudet_K.csv)\n' % len(cand))
    for (g, c), n in sorted(Counter((r['group'], r['call']) for r in cand).items()):
        f.write('  %-8s %-16s %5d\n' % (g, c, n))
with open(os.path.join(HERE, 'erotus_uudet_K.csv'), 'w', encoding='utf-8', newline='') as f:
    w = csv.writer(f, lineterminator='\n')
    w.writerow(['file', 'line', 'function', 'group', 'kind', 'call', 'use'])
    for r in sorted(cand, key=lambda r: (r['file'], int(r['line']))):
        w.writerow([r['file'], r['line'], r['function'], r['group'], r['kind'], r['call'], r['use']])
print(open(os.path.join(HERE, 'erotus.txt'), encoding='utf-8').read())
