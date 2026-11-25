exec(open('graph.py').read().split('if __name__')[0])
import sys
files={}
for l in open(SP+'/cs_prod_proj.txt',encoding='utf-8',errors='replace'):
    m=re.match(r'\./(.+?):\d+:.*\[projects: (.*)\]',l)
    if m and m.group(2).strip()!='-': files[m.group(1)]=m.group(2)
only=sys.argv[1] if len(sys.argv)>1 else 'nontall'
for f in sorted(files):
    src=open(os.path.join(ROOT,f),'rb').read().decode('cp1252',errors='replace')
    s=strip(src); lines=s.split(chr(10))
    ev=[]
    for i,L in enumerate(lines,1):
        for m in lockre.finditer(L): ev.append((i,m.group(1),m.group(2)))
    for idx,(i,a,lk) in enumerate(ev):
        if a!='Enter': continue
        t=lk.startswith('tall')
        if (only=='nontall' and t) or (only=='tall' and not t): continue
        # depth tracking from Enter line: find matching Leave at same or shallower depth
        depth=0; found=None; rets=[]
        # compute until next Leave of same lock in file order
        end=None
        for (i2,a2,lk2) in ev[idx+1:]:
            if a2=='Leave' and lk2==lk: end=i2;break
        if end is None: end=min(i+400,len(lines)); 
        # scan
        for ln in range(i+1,end):
            L=lines[ln-1]
            # a Leave of same lock between? (handled by end)
            if re.search(r'\b(return|goto)\b',L):
                rets.append((ln,L.strip()[:60]))
        # also "break"/continue handled only if enclosing loop outside; skip
        # check if a Leave occurs just before return on previous 6 lines
        bad=[]
        for ln,txt in rets:
            seg=' '.join(lines[max(i,ln-8):ln])
            mine=[m for m in lockre.finditer(seg) if m.group(1)=='Leave' and m.group(2)==lk]
            if not mine: bad.append((ln,txt))
        if bad or end==min(i+400,len(lines)) and not any(a2=='Leave' and lk2==lk for (i2,a2,lk2) in ev[idx+1:]):
            print('%s:%d-%d %s'%(f,i,end,lk.replace('_CriticalSection','')),bad[:5])
