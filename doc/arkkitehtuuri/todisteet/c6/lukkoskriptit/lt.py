exec(open('reg.py').read().split('if __name__ ==')[0])
import sys
for b in sys.argv[1:]:
    d,fs=build_direct(b)
    print('=====',b)
    for n,lks in sorted(d.items()):
        ks=[k for k in lks if k!='tall_CriticalSection']
        if not ks: continue
        print(n,{k.replace('_CriticalSection',''):sorted(set(x[0]+':'+str(x[1]) for x in v))[:3] for k,v in lks.items() if k!='tall_CriticalSection'})
