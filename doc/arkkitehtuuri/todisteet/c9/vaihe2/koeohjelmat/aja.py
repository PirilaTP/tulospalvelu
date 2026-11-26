"""C-9 step 2: run the isolated probe test cases, one process each.

Same protocol as TPsource/V52/Tests/run_core_tests.py and the C-8 probes
(sandbox directory with .tp_sandbox marker and TP_SANDBOX_TOKEN, timeout,
kill on hang). Cases come from tapaukset.txt: build|program|test case|timeout.
Usage: python -I aja.py <build dir> <output file> [raw output dir]
The probe decides the mechanism itself (TPRESULT line); this script records
exit code, duration and the TP lines, and (optionally) the full output of
each case, including ASan reports. A timeout or a crash is recorded, never
interpreted.
"""
import os
import re
import secrets
import shutil
import subprocess
import sys
import tempfile
import time


def load_cases(here):
    cases = []
    with open(os.path.join(here, 'tapaukset.txt'), encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith('#'):
                build, prog, case, tmo = line.split('|')
                cases.append((build, prog, case, int(tmo)))
    return cases


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    build, outfile = sys.argv[1], sys.argv[2]
    rawdir = sys.argv[3] if len(sys.argv) > 3 else None
    if rawdir:
        os.makedirs(rawdir, exist_ok=True)
    out = []
    for bld, prog, case, tmo in load_cases(here):
        exe = os.path.join(build, bld, prog + '.exe')
        sandbox = tempfile.mkdtemp(prefix='c9probe_')
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
            status = 'exit=%d (0x%08X)' % (p.returncode, p.returncode & 0xffffffff)
        except subprocess.TimeoutExpired:
            subprocess.run(['taskkill', '/T', '/F', '/PID', str(p.pid)], capture_output=True)
            data, _ = p.communicate()
            status = 'TIMEOUT'
        dt = time.time() - t0
        text = data.decode('utf-8', 'replace').replace('\r\n', '\n')
        text = text.replace(sandbox, '<sandbox>')
        lines = [l for l in text.split('\n') if l.startswith(('TP', '[doctest] test cases', '[doctest] assertions'))
                 or 'ERROR' in l or 'FAILED' in l or l.startswith('SUMMARY: AddressSanitizer')]
        out.append('=== %s | %s | %s | %s | %.1f s\n%s\n' % (bld, prog, case, status, dt, '\n'.join(lines)))
        if rawdir:
            fn = re.sub(r'[^A-Za-z0-9_-]+', '_', '%s_%s_%s' % (bld, prog, case))[:120] + '.txt'
            with open(os.path.join(rawdir, fn), 'w', encoding='utf-8', newline='\n') as f:
                f.write(text)
        shutil.rmtree(sandbox, ignore_errors=True)
        print(out[-1], flush=True)
    with open(outfile, 'w', encoding='utf-8', newline='\n') as f:
        f.write(''.join(out))


if __name__ == '__main__':
    main()
