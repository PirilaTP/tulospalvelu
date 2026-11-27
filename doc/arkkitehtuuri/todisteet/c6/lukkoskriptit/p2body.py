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
