import csv,re,collections,json
reg=open('C:/dev/tulospalvelu/doc/arkkitehtuuri/rekisteri/havainnot.md',encoding='utf-8').read()
refs=collections.defaultdict(list)  # file -> [(lo,hi,H)]
for line in reg.splitlines():
    m=re.match(r'\| (H-\d+) \|',line)
    if not m: continue
    h=m.group(1)
    cur=None
    for tok in re.findall(r'`([^`]+)`',line):
        mm=re.match(r'^([\w./]+\.(?:cpp|h)):(\d+)(?:-(\d+))?$',tok)
        if mm: cur=mm.group(1); lo=int(mm.group(2)); hi=int(mm.group(3) or lo); refs[cur].append((lo,hi,h)); continue
        mm=re.match(r'^:(\d+)(?:-(\d+))?$',tok)
        if mm and cur: lo=int(mm.group(1)); hi=int(mm.group(2) or lo); refs[cur].append((lo,hi,h))
r=list(csv.DictReader(open('warnings_all.csv',encoding='utf-8')))
u={}
for x in r:
    k=(x['compiler'],x['file'],int(x['line']),x['code'])
    e=u.setdefault(k,{'msg':x['message'],'cfg':set()})
    e['cfg'].add(x['project']+'/'+x['config']+'/'+x['mode'])
out=csv.writer(open('warnings_unique.csv','w',newline='',encoding='utf-8'))
out.writerow(['compiler','code','file','line','message','configs','related_H'])
hits=collections.Counter()
for k,e in sorted(u.items()):
    hs=sorted(set(h for lo,hi,h in refs.get(k[1],[]) if lo-3<=k[2]<=hi+3))
    for h in hs: hits[(h,k[3])]+=1
    out.writerow([k[0],k[3],k[1],k[2],e['msg'],';'.join(sorted(e['cfg'])),';'.join(hs)])
for (h,c),n in sorted(hits.items()): print(h,c,n)
