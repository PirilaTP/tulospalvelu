import sys

exec(open('graph.py').read().split("if __name__")[0])
def run(b,depth):
    fs=files_for(b)
    funcs=collections.defaultdict(list)
    for f in fs:
        for name,ln,ev in parse(f):
            funcs[short(name)].append((f,ln,ev,name))
    direct=collections.defaultdict(set)
    for n,lst in funcs.items():
        for f,ln,ev,full in lst:
            for p,k,a,bb,l in ev:
                if k=='L' and a=='Enter': direct[n].add(bb)
    STOP=set('memset memcpy wcscpy wcsncpy swprintf atoi wcslen strlen wcscmp wcsncmp clrln endline sprintf strcpy strncpy wcscat memcmp'.split())
    for sname in STOP: funcs.pop(sname,None); direct.pop(sname,None)
    calls={n:set() for n in funcs}
    for n,lst in funcs.items():
        for f,ln,ev,full in lst:
            for p,k,a,bb,l in ev:
                if k=='C' and a in funcs and a!=n: calls[n].add(a)
    # lockset reach with depth limit, remember path
    def reach(n,d,seen):
        out={x:[n] for x in direct[n]}
        if d==0: return out
        for c in calls[n]:
            if c in seen: continue
            for x,pth in reach(c,d-1,seen|{c}).items():
                if x not in out: out[x]=[n]+pth
        return out
    cache={}
    def R(n):
        if n not in cache: cache[n]=reach(n,depth,{n})
        return cache[n]
    edges=collections.defaultdict(list)
    for n,lst in funcs.items():
        for f,ln,ev,full in lst:
            held=collections.Counter()
            for p,k,a,bb,l in ev:
                if k=='L':
                    if a=='Enter':
                        for h,c in held.items():
                            if c>0 and h!=bb: edges[(h,bb)].append((f,l,full,'direct'))
                        held[bb]+=1
                    elif a=='Leave': held[bb]-=1
                else:
                    if a in funcs and a!=n:
                        for h,c in held.items():
                            if c>0:
                                for y,pth in R(a).items():
                                    if y!=h: edges[(h,y)].append((f,l,full,'via '+'>'.join(pth)))
    return edges
b=sys.argv[0]
import sys as s2
