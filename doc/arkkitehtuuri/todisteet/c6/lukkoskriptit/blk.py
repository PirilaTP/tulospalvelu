exec(open('graph.py').read().split('if __name__')[0]); STOP=set()
import sys
BL=re.compile(r'\b(Sleep|SendMessage|PostMessage|Synchronize|ShowModal|MessageBox\w*|Application->\w+|wselectopt|selectopt|writeerror\w*|vidspwmsg|Wselectxy|Win_\w+|ShowMessage|Execute|WaitForSingleObject|WaitForMultipleObjects|send|recv|sendto|recvfrom|connect|accept|select|closesocket|gethostbyname|getaddrinfo|ReadFile|WriteFile|_beginthread|CreateThread|Application|fflush|_wfopen|fopen|_wsopen|wfopen|TextFl|Memo\w*->|Update|Refresh|Repaint|ProcessMessages|Form\w*->Show|Close|SQL\w*|mysql_\w+|odbc\w*|_commit|flushall|inputkey|readkbd|kbhit|ch *= *getch)\s*\(')
files={}
for l in open(SP+'/cs_prod_proj.txt',encoding='utf-8',errors='replace'):
    m=re.match(r'\./(.+?):\d+:.*\[projects: (.*)\]',l)
    if m and m.group(2).strip()!='-': files[m.group(1)]=m.group(2)
out=collections.defaultdict(list)
for f in sorted(files):
    src=open(os.path.join(ROOT,f),'rb').read().decode('cp1252',errors='replace')
    s=strip(src); lines=s.split(chr(10))
    ev=[]
    for i,L in enumerate(lines,1):
        for m in lockre.finditer(L): ev.append((i,m.group(1),m.group(2)))
    for idx,(i,a,lk) in enumerate(ev):
        if a!='Enter': continue
        end=None
        for (i2,a2,lk2) in ev[idx+1:]:
            if a2=='Leave' and lk2==lk: end=i2;break
        note=''
        if end is None: end=min(i+300,len(lines)); note='NOLEAVE'
        if end-i>500: note+='LONG'
        for ln in range(i,end+1):
            for m in BL.finditer(lines[ln-1]):
                out[(f,i,end,lk.replace('_CriticalSection',''),note)].append((ln,m.group(1)))
for k,v in out.items():
    if k[3]=='tall' and '-t' not in sys.argv: continue
    print('%s:%d-%d %s %s'%k)
    d=collections.OrderedDict()
    for ln,n in v: d.setdefault(n,[]).append(ln)
    print('    ',{n:l[:6] for n,l in d.items()})
