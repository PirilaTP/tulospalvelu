import sys
exec(open('g2.py').read())
b=sys.argv[1] if len(sys.argv)>1 else 'HKGUI'
d=int(sys.argv[2]) if len(sys.argv)>2 else 1
e=run(b,d)
for (x,y),lst in sorted(e.items()):
    xs=x.replace('_CriticalSection','');ys=y.replace('_CriticalSection','')
    if 'tall' in (xs,ys) and False: continue
    print('%s -> %s'%(xs,ys))
    seen=set()
    for f,l,full,k in lst:
        key=(f,full,k)
        if key in seen: continue
        seen.add(key)
        print('    %s:%d %s %s'%(f,l,full,k))
