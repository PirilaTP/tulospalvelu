"""C-6 step 2: run the isolated probe test cases, one process each.

Same protocol as TPsource/V52/Tests/run_core_tests.py (sandbox directory with
.tp_sandbox marker and TP_SANDBOX_TOKEN, timeout, kill on hang), but without
its manifest, because the probe programs are not part of the core suite.
Usage: python -I aja.py <build dir> <output file>
The probe decides the mechanism itself (TPRESULT line); this script only
records exit code, duration and output. A timeout or a crash is recorded,
never interpreted.
"""
import os
import secrets
import shutil
import subprocess
import sys
import tempfile
import time

CASES = [  # (program, test case, timeout s)
]

def load_cases():
    here = os.path.dirname(os.path.abspath(__file__))
    with open(os.path.join(here, 'tapaukset.txt'), encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                prog, case, tmo = line.split('|')
                CASES.append((prog, case, int(tmo)))

def main():
    build, outfile = sys.argv[1], sys.argv[2]
    load_cases()
    out = []
    for prog, case, tmo in CASES:
        exe = os.path.join(build, prog + '.exe')
        sandbox = tempfile.mkdtemp(prefix='c6probe_')
        token = secrets.token_hex(16)
        with open(os.path.join(sandbox, '.tp_sandbox'), 'w', encoding='ascii') as f:
            f.write(token)
        env = dict(os.environ)
        env['TP_SANDBOX_TOKEN'] = token
        t0 = time.time()
        p = subprocess.Popen([exe, '-tc=' + case, '--no-colors=true', '--no-version=true'],
                             cwd=sandbox, env=env, stdout=subprocess.PIPE,
                             stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL,
                             creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        try:
            data, _ = p.communicate(timeout=tmo)
            status = 'exit=%d' % p.returncode
        except subprocess.TimeoutExpired:
            subprocess.run(['taskkill', '/T', '/F', '/PID', str(p.pid)], capture_output=True)
            data, _ = p.communicate()
            status = 'TIMEOUT'
        dt = time.time() - t0
        text = data.decode('utf-8', 'replace').replace('\r\n', '\n')
        lines = [l for l in text.split('\n') if l.startswith(('TP', '[doctest] test cases', '[doctest] assertions'))
                 or 'ERROR' in l or 'FAILED' in l]
        out.append('=== %s | %s | %s | %.1f s\n%s\n' % (prog, case, status, dt, '\n'.join(lines)))
        shutil.rmtree(sandbox, ignore_errors=True)
        print(out[-1])
    with open(outfile, 'w', encoding='utf-8', newline='\n') as f:
        f.write(''.join(out))

if __name__ == '__main__':
    main()
