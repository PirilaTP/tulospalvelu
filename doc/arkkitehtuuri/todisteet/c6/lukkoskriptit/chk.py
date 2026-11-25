exec(open('g2.py').read())
fs=files_for('HKGUI')
for f in fs:
    for name,ln,ev in parse(f):
        if short(name) in ('memset','wcsncpy','swprintf','atoi','wcscpy','memcpy','endline','clrln'):
            print(f,ln,name,[ (k,a,bb) for p,k,a,bb,l in ev if k=='L'])
