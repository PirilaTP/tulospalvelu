"""Validate the audit documentation, register and agent definitions (read-only).

Usage: python -I tarkista_auditointi.py <repo_root>
Checks:
  - rekisteri/tiedostot.csv: columns, analysis levels 0-4, every file exists,
    every finding ID used is defined in rekisteri/havainnot.md
  - rekisteri/havainnot.md: every H/T row has all fields, IDs unique, and an
    evidence-type value (static vs runtime) is present
  - `path:line` references in doc/arkkitehtuuri/*.md: file exists (relative to
    TPsource/V52 or repo root) and the line number is within the file
  - .claude/agents/*.md: front matter has name, description, tools, model
Exit code 0 = all checks pass.
"""
import csv, glob, os, re, sys

root = sys.argv[1]
src = os.path.join(root, 'TPsource', 'V52')
doc = os.path.join(root, 'doc', 'arkkitehtuuri')
errors = []


def err(msg):
    errors.append(msg)


def resolve(p):
    for base in (src, root, doc):
        c = os.path.join(base, p)
        if os.path.exists(c):
            return c
        d, f = os.path.split(c)
        if os.path.isdir(d):
            for x in os.listdir(d):
                if x.lower() == f.lower():
                    return os.path.join(d, x)
    return None


# --- findings register ---
hav = open(os.path.join(doc, 'rekisteri', 'havainnot.md'), encoding='utf-8').read()
ids = []
for line in hav.splitlines():
    m = re.match(r'\|\s*([HT]-\d{3})\s*\|', line)
    if not m:
        continue
    ids.append(m.group(1))
    cells = [c.strip() for c in line.strip().strip('|').split('|')]
    expected = 9 if m.group(1).startswith('H') else 7
    if len(cells) != expected:
        err(f'havainnot: {m.group(1)} has {len(cells)} cells, expected {expected}')
    if any(c == '' for c in cells):
        err(f'havainnot: {m.group(1)} has an empty cell')
    last = cells[-1].lower()
    if 'staattinen' not in last and 'static' not in last:
        err(f'havainnot: {m.group(1)} evidence type missing (last column)')
dup = {i for i in ids if ids.count(i) > 1}
if dup:
    err(f'havainnot: duplicate IDs {sorted(dup)}')
idset = set(ids)

# --- per-file register ---
rows = list(csv.DictReader(open(os.path.join(doc, 'rekisteri', 'tiedostot.csv'), encoding='utf-8', newline='')))
need = ['polku', 'hakemisto', 'rivit', 'merkisto', 'projektit', 'analyysitaso', 'arkkitehtuuri',
        'kommentointi', 'tarkistettu_rev', 'havainnot', 'huomiot', 'inventoitu_rev']
if rows and list(rows[0].keys()) != need:
    err(f'tiedostot.csv columns differ: {list(rows[0].keys())}')
levels = {}
for r in rows:
    if r['analyysitaso'] not in ('0', '1', '2', '3', '4'):
        err(f"tiedostot.csv {r['polku']}: bad level {r['analyysitaso']}")
    levels[r['analyysitaso']] = levels.get(r['analyysitaso'], 0) + 1
    if 'poistunut' not in r['huomiot'] and not os.path.exists(os.path.join(src, r['polku'])):
        err(f"tiedostot.csv {r['polku']}: file missing")
    for h in r['havainnot'].split():
        if h not in idset:
            err(f"tiedostot.csv {r['polku']}: finding {h} not in havainnot.md")
    if r['analyysitaso'] in ('2', '3', '4') and not r['tarkistettu_rev']:
        err(f"tiedostot.csv {r['polku']}: level {r['analyysitaso']} without tarkistettu_rev")

# --- source references ---
refpat = re.compile(r'`([A-Za-z0-9_./-]+\.(?:cpp|c|h|hpp|H|cbproj|vcxproj|sh))((?::~?\d+(?:-\d+)?)?)`')
nrefs = 0
for md in glob.glob(os.path.join(doc, '*.md')) + glob.glob(os.path.join(doc, 'rekisteri', '*.md')):
    text = open(md, encoding='utf-8').read()
    for m in refpat.finditer(text):
        p, ln = m.group(1), m.group(2)
        if '/' not in p:
            continue  # bare file names refer to a directory named nearby
        nrefs += 1
        f = resolve(p)
        if not f:
            err(f'{os.path.basename(md)}: missing file {p}')
            continue
        if ln:
            n = int(re.findall(r'\d+', ln)[-1])
            with open(f, 'rb') as fh:
                total = fh.read().count(b'\n') + 1
            if n > total:
                err(f'{os.path.basename(md)}: {p}{ln} beyond end ({total} lines)')

# --- agents ---
agents = glob.glob(os.path.join(root, '.claude', 'agents', '*.md'))
for a in agents:
    t = open(a, encoding='utf-8').read()
    fm = re.match(r'---\n(.*?)\n---\n', t, re.S)
    if not fm:
        err(f'{os.path.basename(a)}: no front matter'); continue
    keys = dict(l.split(':', 1) for l in fm.group(1).splitlines() if ':' in l)
    for k in ('name', 'description', 'tools', 'model'):
        if k not in keys:
            err(f'{os.path.basename(a)}: missing {k}')
    if keys.get('model', '').strip() not in ('opus', 'sonnet'):
        err(f'{os.path.basename(a)}: model {keys.get("model")}')
    if keys.get('name', '').strip() + '.md' != os.path.basename(a):
        err(f'{os.path.basename(a)}: name does not match file name')

print(f'findings: {len(ids)} ({sum(i.startswith("H") for i in ids)} H, {sum(i.startswith("T") for i in ids)} T)')
print(f'register rows: {len(rows)}, levels: {dict(sorted(levels.items()))}')
print(f'source references checked: {nrefs}')
print(f'agents: {len(agents)}')
for e in errors:
    print('ERROR', e)
print('OK' if not errors else f'{len(errors)} errors')
sys.exit(1 if errors else 0)
