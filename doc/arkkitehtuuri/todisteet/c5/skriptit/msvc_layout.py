"""C-5: run MSVC front end with /d1reportAllClassLayout on every translation unit of
the console projects, using the CL command lines recorded in the C-4 build logs.

usage: python -I msvc_layout.py <c4-logdir> <outdir>
Writes <outdir>/<project>/<NNN>_<src>.txt per unit and <outdir>/units.json.
"""
import os, re, sys, json, subprocess, concurrent.futures as cf

logdir, outdir = sys.argv[1:3]
VCVARS = r'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars32.bat'
CL = r'C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Tools\MSVC\14.52.37025\bin\HostX86\x86\cl.exe'
PROJ = ['HkMaali', 'JukMaali', 'JukLuenta', 'tputilv2', 'dbboxm']
DROP = re.compile(r'^/(Fo|Fd|Fp|FR|Fr|Fa|Yu|Yc|GL|Zi|ZI|analyze|errorReport|FC|diagnostics|external:W)', re.I)

# environment from vcvars once
env_out = subprocess.run('cmd /c ""%s" >nul && set"' % VCVARS, capture_output=True, text=True, shell=True).stdout
env = dict(l.split('=', 1) for l in env_out.splitlines() if '=' in l)

jobs = []
units = {}
for p in PROJ:
    log = open(os.path.join(logdir, 'base-%s-Release.log' % p), encoding='utf-8-sig', errors='replace').read()
    proj = re.search(r'Project "([^"]+\.vcxproj)"', log).group(1)
    line = [l for l in log.splitlines() if 'CL.exe ' in l][0].strip()
    toks = line.split('CL.exe ', 1)[1].split()
    # re-join quoted tokens
    merged, buf = [], None
    for t in toks:
        if buf is not None:
            buf += ' ' + t
            if t.endswith('"'):
                merged.append(buf); buf = None
        elif t.count('"') == 1:
            buf = t
        else:
            merged.append(t)
    srcs = [t for t in merged if re.search(r'\.(cpp|c|cc)"?$', t, re.I)]
    opts = [t for t in merged if t not in srcs and not DROP.match(t) and t != '/c']
    wd = os.path.dirname(proj)
    od = os.path.join(outdir, p)
    os.makedirs(od, exist_ok=True)
    units[p] = {'vcxproj': proj, 'opts': opts, 'srcs': srcs}
    for n, s in enumerate(srcs):
        out = os.path.join(od, '%03d_%s.txt' % (n, os.path.splitext(os.path.basename(s))[0]))
        jobs.append((wd, opts, s.strip('"'), out))

def run(job):
    wd, opts, s, out = job
    cmd = [CL,'/nologo', '/Zs', '/d1reportAllClassLayout'] + opts + [s]
    r = subprocess.run(cmd, cwd=wd, env=env, capture_output=True)
    open(out, 'wb').write(r.stdout + r.stderr)
    return r.returncode, s

rcs = []
with cf.ThreadPoolExecutor(8) as ex:
    for rc, s in ex.map(run, jobs):
        rcs.append((s, rc))
        if rc:
            print('rc=%d %s' % (rc, s), flush=True)
json.dump({'units': units, 'rc': rcs}, open(os.path.join(outdir, 'units.json'), 'w'), indent=1)
print('units', len(jobs), 'failed', sum(1 for _, r in rcs if r))
