exec(open('p2.py').read())
tot=0;inn=0;bad=[]
for f in sorted(files):
    if files[f].strip()=='-': continue
    fl=parse2(f)
    raw=strip(open(ROOT+'/'+f,'rb').read().decode('cp1252',errors='replace'))
    t=len(lockre.findall(raw)); i=sum(1 for n,l,ev in fl for e in ev if e[1]=='L')
    tot+=t;inn+=i
    if t!=i: bad.append((f,t,i))
    nq=sum(1 for n,l,ev in fl if n=='?')
    if nq: print('unnamed',f,nq)
print(tot,inn,bad)
