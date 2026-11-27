import re,os,sys,collections,pickle
exec(open('scan.py').read().split("res={}")[0].replace("files={}","files={}"))  # reuse helpers: strip, lockre, KW
ROOT='C:/dev/tulospalvelu/TPsource/V52'
builds={'HKGUI':['Hk','cbHk','cbTp','Tp','com'],'HKCON':['Hk','HkConsole','Tp','com'],
        'RGUI':['Juk','ViestiWin','cbTp','Tp','com'],'RCON':['Juk','JukConsole','Tp','com']}
skip={'Tp/TpComY32.cpp','cbTp/TpApu.cpp'}
dirs=set(d for v in builds.values() for d in v)
parsed={}
def parse(f):
    if f in parsed: return parsed[f]
    src=open(os.path.join(ROOT,f),'rb').read().decode('cp1252',errors='replace')
    s=strip(src)
    funcs=[];stack=[];last=0
    for i,c in enumerate(s):
        if c=='{':
            hdr=re.sub(r'(?m)^[ 	]*#.*(\
.*)*$','',s[last:i]).strip()
            mm=re.search(r'([~\w:]+)\s*\(',hdr)
            isfn=mm and not re.match(r'(class|struct|namespace|enum|union)\b',hdr) and mm.group(1) not in KW and not any(t[0]=='f' for t in stack) and not re.search(r'\b(typedef|struct|class)\b[^()]*$',hdr)
            stack.append(('f',i,mm.group(1)) if isfn else ('b',i,None)); last=i+1
        elif c=='}':
            if stack:
                t=stack.pop()
                if t[0]=='f': funcs.append((t[2],t[1],i))
            last=i+1
        elif c==';' and not stack: last=i+1
    fl=[]
    for name,a,b in funcs:
        body=s[a:b]
        ev=[]
        for m in lockre.finditer(body):
            ev.append((m.start(),'L',m.group(1),m.group(2),s.count('\n',0,a+m.start())+1))
        for m in re.finditer(r'\b([A-Za-z_]\w*)\s*\(',body):
            if m.group(1) in KW or 'CriticalSection' in m.group(1): continue
            ev.append((m.start(),'C',m.group(1),None,s.count('\n',0,a+m.start())+1))
        ev.sort()
        fl.append((name,s.count('\n',0,a)+1,ev))
    parsed[f]=fl
    return fl
def files_for(b):
    out=[]
    for d in builds[b]:
        for fn in os.listdir(os.path.join(ROOT,d)):
            if fn.endswith('.cpp') and d+'/'+fn not in skip: out.append(d+'/'+fn)
    return out
def short(n): return n.split('::')[-1]
def analyse(b,lockkey):
    fs=files_for(b)
    funcs=collections.defaultdict(list)  # short name -> [(file,line,ev,fullname)]
    for f in fs:
        for name,ln,ev in parse(f):
            funcs[short(name)].append((f,ln,ev,name))
    direct=collections.defaultdict(set)
    for n,lst in funcs.items():
        for f,ln,ev,full in lst:
            for p,k,a,bb,l in ev:
                if k=='L' and a=='Enter': direct[n].add(bb)
    # transitive
    trans={n:set(direct[n]) for n in funcs}
    calls={n:set() for n in funcs}
    for n,lst in funcs.items():
        for f,ln,ev,full in lst:
            for p,k,a,bb,l in ev:
                if k=='C' and a in funcs: calls[n].add(a)
    ch=True
    while ch:
        ch=False
        for n in funcs:
            for c in calls[n]:
                if not trans[c]<=trans[n]: trans[n]|=trans[c];ch=True
    edges=collections.defaultdict(list)
    for n,lst in funcs.items():
        for f,ln,ev,full in lst:
            held=collections.Counter()
            for p,k,a,bb,l in ev:
                if k=='L':
                    if a=='Enter':
                        for h,c in held.items():
                            if c>0 and h!=bb: edges[(h,bb)].append((f,l,full,'direct',''))
                        held[bb]+=1
                    elif a=='Leave': held[bb]-=1
                else:
                    if a in funcs and a!=short(full):
                        for h,c in held.items():
                            if c>0:
                                for y in trans[a]:
                                    if y!=h: edges[(h,y)].append((f,l,full,'via',a))
    return edges,funcs,trans,direct
if __name__=='__main__':
    for b in builds:
        edges,funcs,trans,direct=analyse(b,None)
        print('=====',b)
        for (x,y),lst in sorted(edges.items()):
            nd=[e for e in lst if e[3]=='direct']
            print(x.replace('_CriticalSection',''),'->',y.replace('_CriticalSection',''),'direct:',len(nd),'via:',len(lst)-len(nd))
