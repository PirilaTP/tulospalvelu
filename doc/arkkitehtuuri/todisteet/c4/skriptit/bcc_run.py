"""C-4: re-run the bcc32 compile commands recorded by build-gui.ps1 with all
warnings enabled (-w), writing objects and PCH files to a scratch directory.

usage: python -I bcc_run.py <build-log> <workdir(RADStudio10)> <outdir> <mode: orig|wall>

mode orig = recorded command unchanged except output paths (baseline)
mode wall = same, plus -w (all warnings) appended after the recorded options
"""
import os, re, subprocess, sys, json, time

log, workdir, outdir, mode = sys.argv[1:5]
outdir = os.path.abspath(outdir)
os.makedirs(outdir, exist_ok=True)
import ctypes
_b = ctypes.create_unicode_buffer(1024)
ctypes.windll.kernel32.GetShortPathNameW(outdir, _b, 1024)
outdir = _b.value
assert ' ' not in outdir, outdir
text = open(log, encoding='utf-8-sig', errors='replace').read().splitlines()
cmds = []
i = 0
while i < len(text):
    m = re.match(r'bcc32 command line for "(.*)"$', text[i])
    if m:
        src = m.group(1)
        parts = []
        i += 1
        while i < len(text) and text[i].startswith('  '):
            parts.append(text[i][2:])
            i += 1
        cmds.append((src, ''.join(parts).rstrip()))
    else:
        i += 1

pch = os.path.join(outdir, 'c4.pch')
results = []
t0 = time.time()
for n, (src, cmd) in enumerate(cmds):
    orig = cmd
    obj = os.path.join(outdir, '%03d_%s.obj' % (n, os.path.splitext(os.path.basename(src))[0]))
    cmd = re.sub(r' -o\S+', lambda _: ' -o' + obj, cmd, count=1)
    cmd = re.sub(r' -n\S+', lambda _: ' -n' + outdir, cmd, count=1)
    cmd = re.sub(r' -H=\S+', lambda _: ' -H=' + pch, cmd, count=1)
    assert cmd.endswith(src), (src, cmd[-200:])
    if mode == 'wall':
        cmd = cmd[:-len(src)] + '-w -g0 ' + src
    exe, rest = cmd.split('.exe ', 1)
    line = '"' + exe + '.exe" ' + rest
    p = subprocess.run(line, cwd=workdir, capture_output=True, text=True,
                       encoding='cp1252', errors='replace')
    results.append({'n': n, 'src': src, 'rc': p.returncode, 'orig_cmd': orig,
                    'cmd': line, 'out': p.stdout + p.stderr})
    print('%3d rc=%d %s' % (n, p.returncode, src), flush=True)
json.dump({'log': log, 'mode': mode, 'seconds': time.time() - t0, 'results': results},
          open(os.path.join(outdir, 'results.json'), 'w', encoding='utf-8'), indent=1)
