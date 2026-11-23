"""List non-ASCII string literals in UTF-8 (no BOM) source files.

MSVC reads such files as the system code page (cp1252), so these literals
are compiled wrong in the console programs. See doc/arkkitehtuuri/merkistot.md.
Usage: python -I utf8_literaalit.py <inventory.json> <repo>/TPsource/V52
The JSON comes from inventaario.py.
"""
import sys, os, re, json
sys.stdout.reconfigure(encoding="utf-8")
inv = json.load(open(sys.argv[1], encoding='utf-8'))
root = sys.argv[2]
lit = re.compile(r'(L?)"((?:[^"\\n]|\.)*)"')
for f in inv['files']:
    if f['encoding'] != 'utf-8' or not f['path'].lower().endswith(('.c', '.cpp', '.h')):
        continue
    t = open(os.path.join(root, f['path']), encoding='utf-8').read()
    t = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), t, flags=re.S)
    hits = []
    for i, line in enumerate(t.split('\n'), 1):
        code = re.sub(r'//.*', '', line) if '"' not in line.split('//')[0] else line
        for m in lit.finditer(code):
            if any(ord(c) > 127 for c in m.group(2)):
                hits.append((i, m.group(1) + '"' + m.group(2)[:50] + '"'))
    if hits:
        print(f"{f['path']}  [{', '.join(p.split('/')[-1] for p in f['projects']) or '-'}]  {len(hits)} kpl")
        for h in hits[:3]:
            print('    ', h[0], h[1])
