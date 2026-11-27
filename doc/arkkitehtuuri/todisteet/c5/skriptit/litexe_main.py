"""C-5: which UTF-8 literals (as mis-decoded cp1252) appear in which shipped exe.
usage: python -I litexe_main.py <repo-root>"""
import re, glob, os, sys
root = sys.argv[1]
lit = re.compile(r'(L?)"((?:[^"\\\n]|\\.)*)"')
items = []
for f in ['Hk/HkAjat.cpp', 'Juk/vajat.cpp', 'Tp/TpLaitteet.cpp']:
    raw = open(os.path.join(root, 'TPsource/V52', f), 'rb').read().decode('utf-8').split('\n')
    for i, l in enumerate(raw, 1):
        if l.lstrip().startswith('//'):
            continue
        for m in lit.finditer(l):
            s = m.group(2)
            if not any(ord(c) > 127 for c in s):
                continue
            parts = re.split(r'%[-0-9.]*[a-zA-Z]|\\.', s)
            frag = max(parts, key=lambda x: (any(ord(c) > 127 for c in x), len(x)))
            items.append((f, i, m.group(1) == 'L', frag.encode('utf-8').decode('cp1252', errors='replace')))
exes = sorted(glob.glob(os.path.join(root, 'TPexe/**/*.exe'), recursive=True))
bins = [open(e, 'rb').read() for e in exes]
print('site kind ' + ' '.join(os.path.basename(e) for e in exes))
for f, i, w, moj in items:
    enc = moj.encode('utf-16-le') if w else moj.encode('cp1252', errors='replace')
    print('%s:%d %s %s' % (f, i, 'W' if w else 'N', ' '.join('X' if enc in b else '.' for b in bins)))
