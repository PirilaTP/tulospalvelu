import sys
A=sys.argv[1:]
exec(open('g2.py').read())
b=A[0]; d=int(A[1])
e=run(b,d)
for (x,y),lst in sorted(e.items()):
    xs=x.replace('_CriticalSection','');ys=y.replace('_CriticalSection','')
    print('%s -> %s'%(xs,ys))
    seen=set()
    for f,l,full,k in lst:
        key=(f,full,k)
        if key in seen: continue
        seen.add(key)
        print('    %s:%d %s %s'%(f,l,full,k))
