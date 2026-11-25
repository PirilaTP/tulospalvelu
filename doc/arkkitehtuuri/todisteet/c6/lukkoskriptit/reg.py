exec(open('graph.py').read().split("if __name__")[0])
import re
files_all=[]
for l in open(SP+'/cs_prod_proj.txt',encoding='utf-8',errors='replace'): pass
STOP=set('memset memcpy wcscpy wcsncpy swprintf atoi wcslen strlen wcscmp wcsncmp clrln endline sprintf strcpy strncpy wcscat memcmp'.split())
# lock-taking functions: per dir-group direct sets (name -> {lock: [(file,line)]})
def build_direct(b):
    fs=files_for(b)
    d=collections.defaultdict(lambda: collections.defaultdict(list))
    for f in fs:
        for name,ln,ev in parse(f):
            sn=short(name)
            if sn in STOP or sn=='?': continue
            for p,k,a,bb,l in ev:
                if k=='L' and a=='Enter': d[sn][bb].append((f,l))
    return d,fs
def regions(b):
    d,fs=build_direct(b)
    out=[]
    for f in fs:
        src=open(os.path.join(ROOT,f),'rb').read().decode('cp1252',errors='replace')
        s=strip(src); lines=s.split(chr(10))
        ev=[]  # (line, kind, lock)
        for i,L in enumerate(lines,1):
            for m in lockre.finditer(L): ev.append((i,m.start(),m.group(1),m.group(2)))
        for idx,(i,c,a,lk) in enumerate(ev):
            if a!='Enter': continue
            # find end: next Leave same lock
            end=None
            for (i2,c2,a2,lk2) in ev[idx+1:]:
                if a2=='Leave' and lk2==lk: end=i2;break
            if end is None: end=i+400; note='NOLEAVE'
            else: note=''
            if end-i>600: note+='LONG'
            inner=[(i2,a2,lk2) for (i2,c2,a2,lk2) in ev[idx+1:] if i<=i2<=end and lk2!=lk and a2=='Enter']
            calls=[]
            for ln in range(i,min(end,len(lines))+1):
                for m in re.finditer(r'\b([A-Za-z_]\w*)\s*\(',lines[ln-1]):
                    nm=m.group(1)
                    if nm in d and nm not in STOP:
                        for lk2,site in d[nm].items():
                            if lk2!=lk: calls.append((ln,nm,lk2))
            if inner or calls: out.append((f,i,end,lk,note,inner,calls))
    return out
if __name__=='__main__':
    import sys
    b=sys.argv[1]
    for f,i,e,lk,note,inner,calls in regions(b):
        lk=lk.replace('_CriticalSection','')
        print('%s:%d-%d %s %s'%(f,i,e,lk,note))
        for x in inner: print('   DIRECT',x[0],x[2].replace('_CriticalSection',''))
        seen=set()
        for ln,nm,l2 in calls:
            if (nm,l2) in seen: continue
            seen.add((nm,l2));print('   CALL %d %s -> %s'%(ln,nm,l2.replace('_CriticalSection','')))
