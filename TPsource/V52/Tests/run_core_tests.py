#!/usr/bin/env python3
# Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
# Copyright (C) 2015 Pekka Pirila
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""Runner for the core component tests (TpCoreHk, TpCoreJuk).

Every test case of Tests/core/manifest.json runs in its own process with a
fresh temporary directory as the current directory (the sandbox) and a
timeout. The runner classifies each run:

  PASS   control test: the process ended normally and all checks passed
  XFAIL  defect test: the test reported the specific defect mechanism
         (TPRESULT observed=1) and all preconditions passed
  XPASS  defect test: the test ran correctly and the mechanism was NOT seen
  FAIL   anything else: timeout, crash, failed precondition, missing or
         duplicate TPRESULT, wrong finding or mechanism, test case not found
  SKIP   the test reported TPSKIP and nothing failed, or the program is not
         built and --allow-missing was given (otherwise that is a FAIL)

A crash or a timeout is never counted as evidence of a defect.

Usage (from any directory, Python 3.8+, standard library only):
  python -I run_core_tests.py [--out DIR] [--filter TEXT] [--keep] [--allow-missing]

Exit code: 0 when there is no FAIL and no XPASS, 1 otherwise, 2 on usage
errors. Results: DIR/results.json and DIR/results.md (default
Tests/results/<timestamp>/, ignored by git).

See doc/arkkitehtuuri/testaus.md.
"""

import argparse
import datetime
import hashlib
import json
import os
import platform
import re
import secrets
import shutil
import subprocess
import sys
import tempfile
import time

HERE = os.path.dirname(os.path.abspath(__file__))          # TPsource/V52/Tests
V52 = os.path.dirname(HERE)                                  # TPsource/V52
REPO = os.path.dirname(os.path.dirname(V52))
MANIFEST = os.path.join(HERE, 'core', 'manifest.json')

VERIFICATION = {
    'XFAIL': 'toistettu testikehyksessa / reproduced in test harness',
    'XPASS': 'ei toistunut testikehyksessa / not reproduced in test harness',
    'PASS': '- (kontrolli / control)',
    'FAIL': 'ei ratkennut / undecided',
    'SKIP': 'ei ratkennut / undecided',
}

HARNESS_GLOBS = ['run_core_tests.py', 'core', 'fixtures', 'data']
PROJECT_FILES = ['VS/Tests/TpCore.sln', 'VS/Tests/TpCoreJuk.vcxproj', 'VS/Tests/TpCoreHk.vcxproj']
LIBS = ['../../TPexe/lib/V52/VS/Release/tputilv2.lib', '../../TPexe/lib/V52/VS/Release/dbboxm.lib']
SOURCE_DIRS = ['Juk', 'Hk', 'Tp', 'com', 'tputilv2', 'dbboxm', 'include', 'JukConsole', 'HkConsole',
               'Tests/core', 'Tests/fixtures']

SUMMARY_RE = re.compile(r'\[doctest\] test cases:\s*(\d+)\s*\|\s*(\d+) passed\s*\|\s*(\d+) failed')


def sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for chunk in iter(lambda: f.read(65536), b''):
            h.update(chunk)
    return h.hexdigest()


def git(*args):
    try:
        return subprocess.run(['git', '-C', REPO] + list(args), capture_output=True,
                              text=True, timeout=30).stdout.strip()
    except (OSError, subprocess.SubprocessError):
        return ''


def parse_info(text):
    """TPINFO lines are free text (they may contain '=')."""
    return [line[len('TPINFO|'):] for line in text.splitlines() if line.startswith('TPINFO|')]


def harness_hashes():
    """sha256 of every harness file, so uncommitted test code is identified."""
    out = {}
    for g in HARNESS_GLOBS:
        base = os.path.join(HERE, g)
        files = [base] if os.path.isfile(base) else [
            os.path.join(d, f) for d, _, fs in os.walk(base) for f in fs if '__pycache__' not in d]
        for f in sorted(files):
            out[os.path.relpath(f, REPO).replace(os.sep, '/')] = sha256(f)
    for rel in PROJECT_FILES + LIBS:
        f = os.path.normpath(os.path.join(V52, rel))
        out[os.path.relpath(f, REPO).replace(os.sep, '/')] = sha256(f) if os.path.isfile(f) else 'missing'
    return out


def newest_source_mtime():
    newest, name = 0.0, ''
    for d in SOURCE_DIRS:
        for root, _, files in os.walk(os.path.join(V52, d)):
            for f in files:
                if f.lower().endswith(('.cpp', '.h')):
                    m = os.path.getmtime(os.path.join(root, f))
                    if m > newest:
                        newest, name = m, os.path.join(root, f)
    return newest, os.path.relpath(name, REPO).replace(os.sep, '/') if name else ''


def doctest_version():
    try:
        with open(os.path.join(HERE, 'doctest.h'), encoding='ascii', errors='replace') as f:
            text = f.read()
        parts = [re.search(r'#define DOCTEST_VERSION_%s (\d+)' % k, text) for k in ('MAJOR', 'MINOR', 'PATCH')]
        return '.'.join(p.group(1) for p in parts) if all(parts) else ''
    except OSError:
        return ''


_listed = {}


def listed_test_cases(exe):
    """Exact test case names of a program (doctest --list-test-cases)."""
    if exe not in _listed:
        p = subprocess.run([exe, '--list-test-cases', '--no-colors=true', '--no-version=true'],
                           capture_output=True, timeout=60, stdin=subprocess.DEVNULL)
        names = set()
        for line in p.stdout.decode('ascii', 'replace').splitlines():
            line = line.rstrip()
            if line and not line.startswith('[doctest]') and not line.startswith('====='):
                names.add(line)
        _listed[exe] = names
    return _listed[exe]


def kill_tree(p):
    """Kills the test process and any children it started."""
    try:
        subprocess.run(['taskkill', '/T', '/F', '/PID', str(p.pid)], capture_output=True, timeout=30)
    except (OSError, subprocess.SubprocessError):
        pass
    try:
        p.kill()
    except OSError:
        pass


def parse_lines(text, prefix):
    out = []
    for line in text.splitlines():
        if line.startswith(prefix + '|'):
            fields = {}
            for part in line[len(prefix) + 1:].split('|'):
                if '=' in part:
                    k, v = part.split('=', 1)
                    fields[k] = v
                else:
                    fields.setdefault('text', part)
            out.append(fields)
    return out


def classify(test, rc, timed_out, stdout):
    """Returns (status, reason, tpresult or None)."""
    if timed_out:
        return 'FAIL', 'timeout (not accepted as defect evidence)', None
    m = SUMMARY_RE.search(stdout)
    if not m:
        return 'FAIL', 'exit code %s without doctest summary (crash?)' % rc, None
    cases, passed, failed = (int(x) for x in m.groups())
    if cases != 1:
        return 'FAIL', 'doctest ran %d test cases, expected exactly 1' % cases, None
    if failed or rc != 0:
        return 'FAIL', 'precondition or check failed (doctest failed=%d, exit %s)' % (failed, rc), None
    skips = parse_lines(stdout, 'TPSKIP')
    if skips:
        return 'SKIP', 'test reported TPSKIP: ' + skips[0].get('reason', ''), None
    kind = test['kind']
    results = parse_lines(stdout, 'TPRESULT')
    if kind in ('control', 'info'):
        if results:
            return 'FAIL', 'control test printed TPRESULT', None
        return 'PASS', 'all checks passed', None
    if len(results) != 1:
        return 'FAIL', 'expected exactly one TPRESULT line, got %d' % len(results), None
    r = results[0]
    if r.get('finding') != test['finding'] or r.get('mechanism') != test['mechanism']:
        return 'FAIL', 'TPRESULT for %s/%s, expected %s/%s' % (
            r.get('finding'), r.get('mechanism'), test['finding'], test['mechanism']), r
    if r.get('observed') == '1':
        return 'XFAIL', 'defect mechanism observed', r
    if r.get('observed') == '0':
        return 'XPASS', 'defect mechanism NOT observed (review the finding)', r
    return 'FAIL', 'TPRESULT without observed=0/1', r


def run_one(test, exe, args, results_dir):
    rec = {
        'id': test['id'], 'program': test['program'], 'test_case': test['test_case'],
        'kind': test['kind'], 'finding': test.get('finding', ''),
        'mechanism': test.get('mechanism', ''), 'expected': test.get('expected', ''),
        'timeout_s': test.get('timeout', 30),
    }
    rec['started'] = datetime.datetime.now().isoformat(timespec='seconds')
    if not os.path.isfile(exe):
        st = 'SKIP' if args.allow_missing else 'FAIL'
        rec.update(status=st, reason='program not built: ' + exe, actual='program not built',
                   verification=VERIFICATION[st])
        return rec
    if test['test_case'] not in listed_test_cases(exe):
        rec.update(status='FAIL', reason='test case name not found exactly (case-sensitive) in '
                   '--list-test-cases', actual='test case name not exact',
                   verification=VERIFICATION['FAIL'])
        return rec

    sandbox = tempfile.mkdtemp(prefix='tpcore_')
    token = secrets.token_hex(16)
    with open(os.path.join(sandbox, '.tp_sandbox'), 'w', encoding='ascii') as f:
        f.write(token)
    data = []
    for d in test.get('data', []):
        src = os.path.join(HERE, d['src'])
        dst = os.path.join(sandbox, d['dst'])
        shutil.copyfile(src, dst)
        data.append({'src': 'TPsource/V52/Tests/' + d['src'], 'dst': d['dst'], 'sha256': sha256(src)})
    rec['data'] = data

    env = dict(os.environ)
    env['TP_SANDBOX_TOKEN'] = token
    env['TP_TEST'] = '1'
    cmd = [exe, '--test-case=' + test['test_case'], '--no-colors=true', '--no-version=true']
    rec['command'] = subprocess.list2cmdline(cmd).replace(V52 + os.sep, '').replace('/', os.sep) + \
        '   (cwd = sandbox)'
    t0 = time.time()
    timed_out = False
    flags = getattr(subprocess, 'CREATE_NO_WINDOW', 0)
    p = subprocess.Popen(cmd, cwd=sandbox, env=env, stdin=subprocess.DEVNULL,
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE, creationflags=flags)
    try:
        out, err = p.communicate(timeout=rec['timeout_s'])
    except subprocess.TimeoutExpired:
        timed_out = True
        kill_tree(p)
        try:
            out, err = p.communicate(timeout=30)
        except subprocess.TimeoutExpired:
            out, err = b'', b'(output lost: pipes still held after kill)'
    rec['duration_s'] = round(time.time() - t0, 3)
    rc = p.returncode
    rec['exit_code'] = rc if rc is None or rc >= 0 else rc & 0xFFFFFFFF
    rec['exit_code_hex'] = '0x%08X' % (rec['exit_code'] & 0xFFFFFFFF) if rc is not None else ''
    stdout = out.decode('ascii', 'replace')
    stderr = err.decode('ascii', 'replace')
    status, reason, r = classify(test, rc, timed_out, stdout)
    rec.update(status=status, reason=reason, verification=VERIFICATION[status])
    if r:
        rec['observed'] = r.get('observed')
        rec['actual'] = r.get('detail', '')
    else:
        rec['actual'] = reason
    rec['info'] = parse_info(stdout)
    rec['sandbox_files'] = sorted(n for n in os.listdir(sandbox) if n != '.tp_sandbox')

    log = os.path.join(results_dir, rec['id'] + '.log')
    with open(log, 'w', encoding='utf-8') as f:
        f.write('command: %s\nexit: %s timed_out: %s\n--- stdout ---\n%s\n--- stderr ---\n%s\n'
                % (rec['command'], rec['exit_code_hex'], timed_out, stdout, stderr))
    rec['log'] = os.path.basename(log)

    if args.keep or status not in ('PASS', 'XFAIL'):
        rec['sandbox'] = sandbox
    else:
        shutil.rmtree(sandbox, ignore_errors=True)
    return rec


def write_md(path, meta, records):
    lines = ['# Core test results', '',
             '| | |', '|---|---|']
    for k in ('started', 'git_head', 'git_branch', 'repo_dirty', 'production_dirty', 'host', 'python',
              'doctest_version', 'summary'):
        lines.append('| %s | `%s` |' % (k, meta[k]))
    for prog, info in meta['programs'].items():
        lines.append('| %s | `%s` sha256 `%s`; %s |' % (
            prog, info.get('path', ''), info.get('sha256', '')[:16], ', '.join(info.get('build_info', []))))
    lines += ['', '| ID | Kind | Finding | Status | Verification | Actual |', '|---|---|---|---|---|---|']
    for r in records:
        lines.append('| %s | %s | %s | **%s** | %s | %s |' % (
            r['id'], r['kind'], r.get('finding', ''), r['status'], r['verification'],
            r.get('actual', '').replace('|', '/')))
    lines += ['', 'Details, commands, expected results and data: `results.json` and `<id>.log`.', '']
    with open(path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines))


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--out', help='results directory (default Tests/results/<timestamp>)')
    ap.add_argument('--filter', default='', help='run only tests whose id contains this text')
    ap.add_argument('--keep', action='store_true', help='keep all sandboxes (default: keep only non-PASS/XFAIL)')
    ap.add_argument('--allow-missing', action='store_true',
                    help='report a program that is not built as SKIP instead of FAIL')
    args = ap.parse_args()

    with open(MANIFEST, encoding='utf-8') as f:
        manifest = json.load(f)
    stamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
    out = os.path.abspath(args.out or os.path.join(HERE, 'results', stamp))
    os.makedirs(out, exist_ok=True)

    meta = {
        'started': datetime.datetime.now().isoformat(timespec='seconds'),
        'git_head': git('rev-parse', 'HEAD'),
        'git_branch': git('rev-parse', '--abbrev-ref', 'HEAD'),
        'tpsource_dirty': git('status', '--porcelain', '--', 'TPsource') != '',
        'tpsource_status': git('status', '--porcelain', '--', 'TPsource').splitlines(),
        'repo_dirty': git('status', '--porcelain') != '',
        'production_dirty': git('status', '--porcelain', '--', 'TPsource',
                                ':(exclude)TPsource/V52/Tests', ':(exclude)TPsource/V52/VS/Tests') != '',
        'harness_sha256': harness_hashes(),
        'doctest_version': doctest_version(),
        'host': platform.platform(),
        'python': platform.python_version(),
        'programs': {},
    }
    records = []
    tests = [t for t in manifest['tests'] if args.filter in t['id']]
    for prog, rel in manifest['programs'].items():
        exe = os.path.join(V52, rel)
        info = {'path': 'TPsource/V52/' + rel}
        if os.path.isfile(exe):
            info['sha256'] = sha256(exe)
            mt = os.path.getmtime(exe)
            info['mtime'] = datetime.datetime.fromtimestamp(mt).isoformat(timespec='seconds')
            newest, newest_name = newest_source_mtime()
            info['older_than_sources'] = newest > mt
            if newest > mt:
                info['newer_source'] = newest_name
                print('WARNING: %s is older than %s; rebuild before trusting results' % (rel, newest_name))
        meta['programs'][prog] = info
    for t in tests:
        exe = os.path.join(V52, manifest['programs'][t['program']])
        try:
            rec = run_one(t, exe, args, out)
        except Exception as e:  # one bad entry must not abort the run
            rec = {'id': t.get('id', '?'), 'program': t.get('program', ''), 'test_case': t.get('test_case', ''),
                   'kind': t.get('kind', ''), 'finding': t.get('finding', ''), 'status': 'FAIL',
                   'reason': 'runner error: %r' % (e,), 'actual': 'runner error: %r' % (e,),
                   'verification': VERIFICATION['FAIL']}
        if t['kind'] == 'info':
            meta['programs'][t['program']]['build_info'] = rec.get('info', [])
        records.append(rec)
        print('%-6s %-16s %s' % (rec['status'], rec['id'], rec.get('actual', '')))

    counts = {}
    for r in records:
        counts[r['status']] = counts.get(r['status'], 0) + 1
    meta['summary'] = counts
    meta['finished'] = datetime.datetime.now().isoformat(timespec='seconds')
    with open(os.path.join(out, 'results.json'), 'w', encoding='utf-8') as f:
        json.dump({'meta': meta, 'results': records}, f, indent=2)
    write_md(os.path.join(out, 'results.md'), meta, records)
    print('summary:', ', '.join('%s=%d' % kv for kv in sorted(counts.items())), '| results:', out)
    bad = counts.get('FAIL', 0) + counts.get('XPASS', 0)
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
