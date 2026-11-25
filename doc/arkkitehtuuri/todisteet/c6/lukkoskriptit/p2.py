BS=chr(92)
import re,sys,os,json,collections
ROOT='C:/dev/tulospalvelu/TPsource/V52'
SP='C:/Users/NIKO-P~1/AppData/Local/Temp/claude/a25cf22582434270/5b064a44-a282-45a7-b0ea-7cf5eac38f0c/scratchpad/c6'
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
NL=chr(10)
def parse2(f):
    raw=open(ROOT+'/'+f,'rb').read().decode('cp1252',errors='replace')
    s=strip(raw)
    lines=s.split(NL)
    funcs=[]
    i=0;n=len(lines)
    cur=None
    while i<n:
        L=lines[i]
        if cur is None:
            if L.startswith('{'):
                j=i-1;hdr=[]
                while j>=0 and lines[j].strip()!='' and not lines[j].rstrip().endswith(';') and not lines[j].startswith('}') and not lines[j].lstrip().startswith('#'):
                    hdr.insert(0,lines[j]);j-=1
                h=' '.join(hdr)
                m=re.search(r'([~\w:]+)\s*\(',h)
                name=m.group(1) if m and m.group(1) not in KW else '?'
                cur=[name,i+1,[]]
            else:
                m=re.match(r'^[A-Za-z_][\w:~\*&\s<>]*?([~\w:]+)\s*\([^;]*\)\s*(const)?\s*\{\s*$',L)
                if m and m.group(1) not in KW and not L.startswith(('if','for','while','switch','else')):
                    cur=[m.group(1),i+1,[]]
        if cur is not None:
            for m in lockre.finditer(L):
                cur[2].append((m.start(),'L',m.group(1),m.group(2),i+1))
            for m in re.finditer(r'\b([A-Za-z_]\w*)\s*\(',L):
                if m.group(1) in KW or 'CriticalSection' in m.group(1): continue
                cur[2].append((m.start(),'C',m.group(1),None,i+1))
            if L.startswith('}'):
                funcs.append((cur[0],cur[1],cur[2]));cur=None
        i+=1
    if cur: funcs.append((cur[0],cur[1],cur[2]))
    return funcs
