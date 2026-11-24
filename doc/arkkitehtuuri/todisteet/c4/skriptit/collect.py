"""C-4: collect MSVC and bcc32 warnings into one CSV.

usage: python -I collect.py <c4dir> <out.csv>
Rows: compiler, version, mode, project, config, code, file, line, message.
'file' is relative to TPsource/V52 when inside it, else 'SYSTEM:' + path.
"""
import csv, glob, json, os, re, sys

c4, out = sys.argv[1:3]
MSVC_VER = 'MSVC 19.52.37029 (v145, VS 18 Build Tools 14.52.37025)'
BCC_VER = 'bcc32 7.80 (C++Builder 13 CE, Studio 37.0)'
ROOT = 'c:\\dev\\tulospalvelu\\tpsource\\v52\\'


def norm(path, base=None):
    p = path.strip().replace('/', '\\')
    if base and not re.match(r'^[a-zA-Z]:', p):
        p = os.path.normpath(os.path.join(base, p))
    p = os.path.normpath(p)
    if p.lower().startswith(ROOT):
        rel = p[len(ROOT):].replace('\\', '/')
        return CANON.get(rel.lower(), rel)
    return 'SYSTEM:' + p


import subprocess
CANON = {}
for f in subprocess.run(['git', '-C', 'C:/dev/tulospalvelu', 'ls-files', 'TPsource/V52'], capture_output=True, text=True).stdout.splitlines():
    rel = f[len('TPsource/V52/'):]
    CANON[rel.lower()] = rel
rows = set()
# MSVC: file(line[,col]): warning Cnnnn: message [project]
rx = re.compile(r'^\s*(?:\d+>)?(.+?)\((\d+)(?:,\d+)?\): warning (C\d+): (.*?)(?: \[[^\]]+\])?\s*$')
for f in sorted(glob.glob(os.path.join(c4, 'logs', '*.warn.log'))):
    parts = os.path.basename(f)[:-len('.warn.log')].split('-')
    if parts[1] == 'link':
        mode, proj, cfg = 'W3+LTCG', parts[2], parts[3]
    else:
        mode, proj, cfg = parts[0], parts[1], parts[2]
    for line in open(f, encoding='utf-8-sig', errors='replace'):
        m = rx.match(line)
        if m:
            rows.add(('MSVC', MSVC_VER, {'base': 'W3', 'w4a': 'W4+analyze'}.get(mode, mode), proj, cfg,
                      m.group(3), norm(m.group(1)), int(m.group(2)), m.group(4).strip()))

# bcc32: Warning Wnnnn file line: message
brx = re.compile(r'^Warning (W\d+) (.+?) (\d+): (.*)$')
radstudio = 'C:\\dev\\tulospalvelu\\TPsource\\V52\\RADStudio10'
for f in sorted(glob.glob(os.path.join(c4, 'bccout', '*', '*', 'results.json'))):
    mode = f.split(os.sep)[-3]
    proj = f.split(os.sep)[-2]
    d = json.load(open(f, encoding='utf-8'))
    for r in d['results']:
        for line in r['out'].splitlines():
            m = brx.match(line.strip())
            if m:
                rows.add(('bcc32', BCC_VER, 'recorded' if mode == 'orig' else 'recorded+-w', proj,
                          'Release', m.group(1), norm(m.group(2), radstudio), int(m.group(3)),
                          m.group(4).strip()))

with open(out, 'w', newline='', encoding='utf-8') as fh:
    w = csv.writer(fh)
    w.writerow(['compiler', 'version', 'mode', 'project', 'config', 'code', 'file', 'line', 'message'])
    for r in sorted(rows, key=lambda r: (r[0], r[2], r[3], r[4], r[6], r[7], r[5])):
        w.writerow(r)
print(len(rows), 'rows')
