import os,sys,fnmatch
sys.path.insert(0,'.')
import importlib.util
spec=importlib.util.spec_from_file_location('pe','pe.py')
src=open('pe.py').read().split('for p in sys.argv')[0]
exec(src)
S=r'C:\Program Files (x86)\Embarcadero\Studio\37.0'
dirs=[(S+r'\bin','*'),(S+r'\Redist\win32','*'),(os.environ['windir']+r'\SysWOW64','*.bpl')]
def find(n):
    for d,f in dirs:
        if not os.path.isdir(d):continue
        if not fnmatch.fnmatch(n.lower(),f.lower()):continue
        c=os.path.join(d,n)
        if os.path.exists(c):return c
res={}
q=list(imports(sys.argv[1])[0]);
for e in sys.argv[2:]: q+=imports(e)[0]
while q:
    n=q.pop(0);k=n.lower()
    if k in res:continue
    s=find(n);res[k]=s
    if s:
        i,d,_,_,_=imports(s); q+=i
        print('copy',n,s, '| delay:',d if d else '')
sys_=[k for k,v in res.items() if not v]
print('system/skipped:',sys_)
