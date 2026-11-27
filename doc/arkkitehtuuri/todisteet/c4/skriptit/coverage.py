import re,glob,json,os,subprocess,collections
def rel(p):
    p=p.strip(chr(34)).replace(chr(92),'/').lower()
    while p.startswith('../'): p=p[3:]
    return p
allm=set()
for f in glob.glob('logs/*.log'):
    if f.endswith('.warn.log'): continue
    for line in open(f,encoding='utf-8-sig',errors='replace'):
        if 'cl.exe' in line.lower() and ' /c ' in line:
            for s in re.findall(r'(\S+\.(?:cpp|c))(?=[\s"]|$)',line):
                allm.add(rel(s))
bcc=set()
for f in glob.glob('bccout/wall/*/results.json'):
    for r in json.load(open(f,encoding='utf-8'))['results']:
        s=r['src'].replace(chr(92),'/').lower()
        bcc.add(rel(s) if s.startswith('../') else 'radstudio10/'+s)
files=[l for l in subprocess.run(['git','-C','C:/dev/tulospalvelu','ls-files','TPsource/V52'],capture_output=True,text=True).stdout.splitlines() if l.lower().endswith(('.cpp','.c'))]
repo=set(f[len('TPsource/V52/'):].lower() for f in files)
print('MSVC TUs',len(allm),'in repo',len(allm&repo),'bcc32 TUs',len(bcc),'in repo',len(bcc&repo),'both',len(allm&bcc))
print('outside repo set MSVC', sorted(allm-repo)[:5], 'bcc', sorted(bcc-repo)[:5])
print('cpp/c in repo',len(repo),'compiled by either',len(repo&(allm|bcc)),'by none',len(repo-(allm|bcc)))
print('none by dir',collections.Counter(x.split('/')[0] for x in repo-(allm|bcc)))
print('MSVC by dir',collections.Counter(x.split('/')[0] for x in allm))
print('bcc-only by dir',collections.Counter(x.split('/')[0] for x in bcc-allm))
json.dump({'msvc':sorted(allm),'bcc':sorted(bcc),'none':sorted(repo-(allm|bcc))},open('coverage.json','w'),indent=0)
