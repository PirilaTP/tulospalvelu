BS=chr(92)
import re,sys,os,json,collections
ROOT='C:/dev/tulospalvelu/TPsource/V52'
SP='C:/Users/KAYTTA~1/AppData/Local/Temp/claude/a25cf22582434270/5b064a44-a282-45a7-b0ea-7cf5eac38f0c/scratchpad/c6'
files={}
# all files in the project list (built) from cs_prod_proj + any file in built dirs
for l in open(SP+'/cs_prod_proj.txt',encoding='utf-8',errors='replace'):
    m=re.match(r'\./(.+?):\d+:.*\[projects: (.*)\]',l)
    if m: files[m.group(1)]=m.group(2)
def strip(src):
    out=[];i=0;n=len(src);st=0
    while i<n:
        c=src[i]
        if st==0:
            if src.startswith('/*',i): st=1;out.append('  ');i+=2;continue
            if src.startswith('//',i): st=2;out.append('  ');i+=2;continue
            if c=='"':
                st=3;out.append(c);i+=1;continue
            if c=="'":
                st=4;out.append(c);i+=1;continue
            out.append(c);i+=1
        elif st==1:
            if src.startswith('*/',i): st=0;out.append('  ');i+=2
            else: out.append('\n' if c=='\n' else ' ');i+=1
        elif st==2:
            if c=='\n': st=0;out.append(c)
            else: out.append(' ')
            i+=1
        elif st==3:
            if c==BS: out.append('  ');i+=2;continue
            if c=='"': st=0;out.append(c)
            else: out.append('\n' if c=='\n' else ' ')
            i+=1
        else:
            if c==BS: out.append('  ');i+=2;continue
            if c=="'": st=0;out.append(c)
            else: out.append(' ')
            i+=1
    return ''.join(out)
lockre=re.compile(r'\b(Enter|Leave|TryEnter|Initialize|Delete)CriticalSection\s*\(\s*&?\s*([\w\.\->]+?)(\[[^\]]*\])?\s*\)')
KW=set('if for while switch return sizeof catch else do case defined'.split())
res={}
for f,proj in sorted(files.items()):
    p=os.path.join(ROOT,f)
    src=open(p,'rb').read().decode('cp1252',errors='replace')
    s=strip(src)
    # events by position
    ev=[]
    for m in lockre.finditer(s):
        ev.append((m.start(),'L',m.group(1),m.group(2).replace('this->','').replace('->','.')))
    for m in re.finditer(r'\b([A-Za-z_]\w*)\s*\(',s):
        if m.group(1) in KW or 'CriticalSection' in m.group(1): continue
        ev.append((m.start(),'C',m.group(1),None))
    ev.sort()
    # function boundaries
    funcs=[] # (name,startpos,endpos)
    depth=0;stack=[];last=0
    for i,c in enumerate(s):
        if c=='{':
            if not stack or stack[-1][0]!='f':
                hdr=s[last:i]
                hdr2=hdr.strip()
                mm=re.search(r'([~\w:]+)\s*\(',hdr2)
                isfn = mm and not re.match(r'\s*(class|struct|namespace|enum|union)\b',hdr2) and mm.group(1) not in KW
                if isfn and not (stack and stack[-1][0]=='f'):
                    stack.append(('f',i,mm.group(1)))
                else: stack.append(('b',i,None))
            else: stack.append(('b',i,None))
            last=i+1
        elif c=='}':
            if stack:
                t=stack.pop()
                if t[0]=='f': funcs.append((t[2],t[1],i))
            last=i+1
        elif c==';' and not stack: last=i+1
    res[f]=dict(proj=proj,funcs=funcs,ev=ev,s=s)
    lines=s
import pickle
pickle.dump(res,open('C:/Users/KAYTTA~1/AppData/Local/Temp/claude/a25cf22582434270/5b064a44-a282-45a7-b0ea-7cf5eac38f0c/scratchpad/c6b/scan.pkl','wb'))
# live lock hits
cnt=collections.Counter();per=collections.defaultdict(collections.Counter)
live=[]
for f,r in res.items():
    s=r['s']
    for pos,k,a,b in r['ev']:
        if k=='L':
            ln=s.count('\n',0,pos)+1
            live.append((f,ln,a,b,r['proj']))
            cnt[(a,b)]+=1
open('C:/Users/KAYTTA~1/AppData/Local/Temp/claude/a25cf22582434270/5b064a44-a282-45a7-b0ea-7cf5eac38f0c/scratchpad/c6b/live.txt','w').write('\n'.join('%s:%d %s %s [%s]'%x for x in live))
print(len(live))
c2=collections.Counter()
for f,ln,a,b,pr in live:
    if pr.strip()!='-': c2[(b,a)]+=1
for k,v in sorted(c2.items()): print(k,v)
