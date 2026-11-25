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
