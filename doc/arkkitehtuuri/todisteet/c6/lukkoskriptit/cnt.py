import re,collections
rows=[l.rstrip('\n') for l in open('live.txt')]
P=['HkKisaWin','ViestiWin','HkMaali','JukLuenta520','JukMaali520']
cnt=collections.defaultdict(lambda: collections.Counter())
glob=collections.defaultdict(lambda: collections.Counter())
for r in rows:
    m=re.match(r'(.+?):(\d+) (\w+) (\S+) \[(.*)\]',r)
    if not m: continue
    f,ln,act,lk,pr=m.groups()
    if pr.strip()=='-': continue
    lk=lk.replace('_CriticalSection','')
    key=lk
    if lk=='sakkoGrid': key='sakkoGrid@'+f.split('/')[0]+'/'+f.split('/')[1]
    if lk in('SQL','tls','sakkoKierrCriticalSection','sakkoKierr','seur','etls'): key=lk+'@'+f.split('/')[0]
    for p in pr.split():
        cnt[key][(p,act)]+=1
for k in sorted(cnt):
    print(k, {p:(cnt[k][(p,'Enter')],cnt[k][(p,'Leave')]) for p in P if cnt[k][(p,'Enter')] or cnt[k][(p,'Leave')]})
