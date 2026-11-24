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

A crash or a timeout is never counted as evidence of a defect, except in a
test whose manifest entry declares "termination": "asan" or "exception".
Such a test must arm itself (TPARM), print the TPRESULT from inside the ASan
report callback or the vectored exception handler, and the runner must
independently find the same evidence (ASan report on stderr, or TPCRASH
line) and the declared exit code. Anything less is FAIL. If such a test
survives (no crash), it may only report observed=0 after disarming (XPASS).

XPASS is never accepted: it makes the exit code non-zero and is listed in
an attention section of results.md and on stderr.

Usage (from any directory, Python 3.8+, standard library only):
  python -I run_core_tests.py [--out DIR] [--filter TEXT] [--keep] [--allow-missing]

Exit code: 0 when there is no FAIL and no XPASS, 1 when there is a FAIL,
3 when there is an XPASS but no FAIL, 2 on usage errors. Results: DIR/results.json and DIR/results.md (default
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

HARNESS_GLOBS = ['run_core_tests.py', 'doctest.h', 'core', 'fixtures', 'data']
PROJECT_FILES = ['VS/Tests/TpCore.sln', 'VS/Tests/TpCoreJuk.vcxproj', 'VS/Tests/TpCoreHk.vcxproj']
LIBS = ['../../TPexe/lib/V52/VS/Release/tputilv2.lib', '../../TPexe/lib/V52/VS/Release/dbboxm.lib']
SOURCE_DIRS = ['Juk', 'Hk', 'Tp', 'com', 'tputilv2', 'dbboxm', 'include', 'JukConsole', 'HkConsole',
               'Tests/core', 'Tests/fixtures']

# Environment variables a test must not inherit from the user's shell.
STRIP_ENV = ('ASAN_OPTIONS', 'ASAN_SYMBOLIZER_PATH', 'UBSAN_OPTIONS', '_NT_SYMBOL_PATH',
             '_NT_ALT_SYMBOL_PATH', '_NT_SYMCACHE_PATH')
PROD_SPEC = ['TPsource', ':(exclude)TPsource/V52/Tests', ':(exclude)TPsource/V52/VS/Tests']
TESTS_SPEC = ['TPsource/V52/Tests', 'TPsource/V52/VS/Tests']
ASAN_DLL = 'clang_rt.asan_dynamic-i386.dll'
FAILLINE_RE = re.compile(r'(ERROR|FATAL ERROR): (CHECK|REQUIRE|WARN)')

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
                              text=True, timeout=30).stdout.rstrip('\n')
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


def newest_source_mtime(program):
    """Newest mtime among the files the program's project compiles or lists
    (ClCompile/ClInclude in VS/Tests/<name>.vcxproj), the headers in its
    include directories and the shared test headers. Programs whose name
    has no project (or Asan variants) use the project of the base name."""
    name = program.replace('Asan', '')
    proj = os.path.join(V52, 'VS', 'Tests', name + '.vcxproj')
    files = []
    if os.path.isfile(proj):
        text = open(proj, encoding='utf-8').read()
        base = os.path.dirname(proj)
        for inc in re.findall(r'<Cl(?:Compile|Include) Include="([^"]+)"', text):
            files.append(os.path.normpath(os.path.join(base, inc)))
        m = re.search(r'<AdditionalIncludeDirectories>([^<]+)</AdditionalIncludeDirectories>', text)
        if m:
            for d in m.group(1).split(';'):
                if d.startswith('%('):
                    continue
                full = os.path.normpath(os.path.join(base, d))
                if os.path.isdir(full):
                    files += [os.path.join(full, f) for f in os.listdir(full) if f.lower().endswith('.h')]
        files.append(proj)
    else:
        for d in SOURCE_DIRS:
            for root, _, fs in os.walk(os.path.join(V52, d)):
                files += [os.path.join(root, f) for f in fs if f.lower().endswith(('.cpp', '.h'))]
    newest, newest_name = 0.0, ''
    for f in files:
        if os.path.isfile(f):
            m = os.path.getmtime(f)
            if m > newest:
                newest, newest_name = m, f
    return newest, os.path.relpath(newest_name, REPO).replace(os.sep, '/') if newest_name else ''


def doctest_version():
    try:
        with open(os.path.join(HERE, 'doctest.h'), encoding='ascii', errors='replace') as f:
            text = f.read()
        parts = [re.search(r'#define DOCTEST_VERSION_%s (\d+)' % k, text) for k in ('MAJOR', 'MINOR', 'PATCH')]
        return '.'.join(p.group(1) for p in parts) if all(parts) else ''
    except OSError:
        return ''


def base_env():
    env = dict(os.environ)
    for k in list(env):
        u = k.upper()
        if u in STRIP_ENV or u.startswith('TP_') or u.startswith('ASAN_') or u.endswith('SAN_OPTIONS'):
            del env[k]
    return env


_listed = {}


def listed_test_cases(exe):
    """Exact test case names of a program (doctest --list-test-cases)."""
    if exe not in _listed:
        tmp = tempfile.mkdtemp(prefix='tpcore_list_')
        try:
            p = subprocess.run([exe, '--list-test-cases', '--no-colors=true', '--no-version=true'],
                               capture_output=True, timeout=60, stdin=subprocess.DEVNULL, cwd=tmp,
                               env=base_env(), creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        finally:
            shutil.rmtree(tmp, ignore_errors=True)
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


def parse_asan(stderr):
    """One dict per AddressSanitizer error block on stderr."""
    blocks = []
    parts = re.split(r'^==\d+==ERROR: AddressSanitizer: ', stderr, flags=re.M)
    for part in parts[1:]:
        b = {'bug_type': part.split()[0] if part.split() else '', 'access': '', 'size': '',
             'thread': '', 'stack': [], 'frame': '', 'vars': []}
        m = re.search(r'^(READ|WRITE) of size (\d+) at \S+ thread (T\d+)', part, re.M)
        if m:
            b['access'], b['size'], b['thread'] = m.group(1), m.group(2), m.group(3)
        loc = re.search(r'is located in stack of thread T\d+ at offset \d+ in frame', part)
        head = part[:loc.start()] if loc else part
        b['stack'] = re.findall(r'^\s*#\d+ 0x[0-9a-fA-F]+ in (\S+)', head, re.M)
        if loc:
            m = re.search(r'#\d+ 0x[0-9a-fA-F]+ in (\S+)', part[loc.end():])
            if m:
                b['frame'] = m.group(1)
        for m in re.finditer(r"\[(\d+), (\d+)\) '([^']+)'([^\n]*)", part):
            b['vars'].append({'name': m.group(3), 'size': int(m.group(2)) - int(m.group(1)),
                              'flagged': 'overflows this variable' in m.group(4)})
        blocks.append(b)
    return blocks


def resolve_rva(exe, rva):
    """Resolves exe-relative address rva to a symbol name out of process,
    with DbgHelp (ctypes) and the PDB next to the exe only. Returns '' if
    it cannot be resolved. Independent of the in-process handler."""
    try:
        import ctypes
        from ctypes import wintypes
        d = ctypes.WinDLL('dbghelp.dll')
    except (ImportError, OSError):
        return ''
    h = wintypes.HANDLE(0x7E57C0DE)   # pseudo handle for this lookup only
    d.SymSetOptions(0x00000002 | 0x00000200 | 0x00080000)   # UNDNAME, FAIL_CRITICAL_ERRORS, NO_PROMPTS
    d.SymInitialize.argtypes = [wintypes.HANDLE, ctypes.c_char_p, wintypes.BOOL]
    d.SymCleanup.argtypes = [wintypes.HANDLE]
    d.SymLoadModuleEx.restype = ctypes.c_uint64
    d.SymLoadModuleEx.argtypes = [wintypes.HANDLE, wintypes.HANDLE, ctypes.c_char_p, ctypes.c_char_p,
                                  ctypes.c_uint64, wintypes.DWORD, ctypes.c_void_p, wintypes.DWORD]
    d.SymFromAddr.argtypes = [wintypes.HANDLE, ctypes.c_uint64, ctypes.POINTER(ctypes.c_uint64), ctypes.c_void_p]
    if not d.SymInitialize(h, os.path.dirname(exe).encode('mbcs'), False):
        return ''
    try:
        base = d.SymLoadModuleEx(h, None, exe.encode('mbcs'), None, 0x10000000, os.path.getsize(exe), None, 0)
        if not base:
            return ''
        # SYMBOL_INFO: SizeOfStruct 88, MaxNameLen at offset 80, Name at 84.
        buf = ctypes.create_string_buffer(88 + 256)
        ctypes.c_ulong.from_buffer(buf, 0).value = 88
        ctypes.c_ulong.from_buffer(buf, 80).value = 255
        disp = ctypes.c_uint64(0)
        if not d.SymFromAddr(h, base + rva, ctypes.byref(disp), buf):
            return ''
        return buf.raw[84:84 + 256].split(b'\x00')[0].decode('ascii', 'replace')
    finally:
        d.SymCleanup(h)


def _check_tpresult(test, results):
    if len(results) != 1:
        return 'expected exactly one TPRESULT line, got %d' % len(results)
    r = results[0]
    if r.get('finding') != test['finding'] or r.get('mechanism') != test['mechanism']:
        return 'TPRESULT for %s/%s, expected %s/%s' % (
            r.get('finding'), r.get('mechanism'), test['finding'], test['mechanism'])
    if r.get('observed') not in ('0', '1'):
        return 'TPRESULT without observed=0/1'
    return ''


def _asan_evidence_problem(ev, blocks):
    if len(blocks) != 1:
        return 'expected exactly one ASan report, got %d' % len(blocks)
    b = blocks[0]
    for k in ('bug_type', 'access', 'frame'):
        if k in ev and b.get(k) != ev[k]:
            return 'ASan %s is %r, expected %r' % (k, b.get(k), ev[k])
    if 'variable' in ev:
        hit = [v for v in b['vars'] if v['name'] == ev['variable'] and v['flagged']]
        if not hit:
            return 'ASan report does not flag variable %r' % ev['variable']
        if 'variable_size' in ev and hit[0]['size'] != ev['variable_size']:
            return 'variable %r size %d, expected %d' % (ev['variable'], hit[0]['size'], ev['variable_size'])
    for name in ev.get('stack_contains', []):
        if not any(name in f for f in b['stack']):
            return 'ASan stack does not contain %r' % name
    return ''


def _veh_evidence_problem(ev, crashes, exe=None):
    veh = [c for c in crashes if c.get('source') == 'veh']
    if len(veh) != 1 or len(crashes) != 1:
        return 'expected exactly one TPCRASH|source=veh line, got %d (%d total)' % (len(veh), len(crashes))
    c = veh[0]
    for k in ('code', 'access', 'target', 'thread'):
        if k in ev and c.get(k, '').lower() != str(ev[k]).lower():
            return 'TPCRASH %s is %r, expected %r' % (k, c.get(k), ev[k])
    if 'symbol' in ev and ev['symbol'] not in c.get('symbols', '').split('>'):
        return 'TPCRASH symbols %r do not contain %r' % (c.get('symbols'), ev['symbol'])
    if 'symbol' in ev and exe:
        # Independent check: resolve the faulting rva out of process.
        try:
            rva = int(c.get('rva', ''), 16)
        except ValueError:
            return 'TPCRASH without a valid rva'
        name = resolve_rva(exe, rva)
        if name != ev['symbol']:
            return 'rva 0x%X resolves out of process to %r, expected %r' % (rva, name, ev['symbol'])
    return ''


def classify(test, rc, timed_out, stdout, stderr='', exe=None):
    """Returns (status, reason, tpresult or None)."""
    if timed_out:
        return 'FAIL', 'timeout (not accepted as defect evidence)', None
    term = test.get('termination', 'normal')
    results = parse_lines(stdout, 'TPRESULT')
    arms = parse_lines(stdout, 'TPARM')
    disarms = parse_lines(stdout, 'TPDISARM')
    crashes = parse_lines(stdout, 'TPCRASH')
    exits = parse_lines(stdout, 'TPEXIT')
    asan = parse_asan(stderr)
    m = SUMMARY_RE.search(stdout)
    rcu = (rc & 0xFFFFFFFF) if rc is not None else None
    blocked = int(exits[-1].get('blocked_threads', '0')) if exits else 0

    if term == 'normal' or m:
        if not m:
            return 'FAIL', 'exit code %s without doctest summary (crash?)' % rc, None
        cases, passed, failed = (int(x) for x in m.groups())
        if cases != 1:
            return 'FAIL', 'doctest ran %d test cases, expected exactly 1' % cases, None
        if failed or rc != 0:
            return 'FAIL', 'precondition or check failed (doctest failed=%d, exit %s)' % (failed, rc), None
        if crashes or asan:
            return 'FAIL', 'crash evidence in a run that ended normally', None
        if blocked != test.get('blocked_threads', 0):
            return 'FAIL', 'blocked threads %d, manifest expects %d' % (blocked, test.get('blocked_threads', 0)), None
        skips = parse_lines(stdout, 'TPSKIP')
        if skips:
            return 'SKIP', 'test reported TPSKIP: ' + skips[0].get('reason', ''), None
        kind = test['kind']
        if kind in ('control', 'info'):
            if results or arms:
                return 'FAIL', 'control test printed TPRESULT/TPARM', None
            return 'PASS', 'all checks passed', None
        problem = _check_tpresult(test, results)
        if problem:
            return 'FAIL', problem, results[0] if results else None
        r = results[0]
        if term != 'normal':
            # A crash test that survived: only a disarmed observed=0 counts.
            if len(arms) != 1 or len(disarms) != 1 or r.get('observed') != '0':
                return 'FAIL', 'crash test survived without a clean disarmed observed=0', r
            order = [l.split('|')[0] for l in stdout.splitlines()
                     if l.startswith(('TPARM|', 'TPDISARM|', 'TPRESULT|'))]
            if order != ['TPARM', 'TPDISARM', 'TPRESULT']:
                return 'FAIL', 'crash test lines out of order: %s' % order, r
            return 'XPASS', 'defect mechanism NOT observed (no crash; review the finding)', r
        if arms or disarms:
            return 'FAIL', 'TPARM/TPDISARM in a normal test', r
        if r.get('observed') == '1':
            return 'XFAIL', 'defect mechanism observed', r
        return 'XPASS', 'defect mechanism NOT observed (review the finding)', r

    # Crash test that terminated (no doctest summary).
    expect = [int(x, 0) & 0xFFFFFFFF for x in test.get('expect_exit', [])]
    if rcu not in expect:
        return 'FAIL', 'exit code %s not in expected %s' % (
            '0x%08X' % rcu if rcu is not None else None, test.get('expect_exit')), None
    if FAILLINE_RE.search(stdout):
        return 'FAIL', 'a doctest check failed before the crash', None
    if len(arms) != 1 or arms[0].get('finding') != test['finding'] or arms[0].get('mechanism') != test['mechanism']:
        return 'FAIL', 'expected exactly one matching TPARM, got %d' % len(arms), None
    if disarms:
        return 'FAIL', 'test disarmed before the crash', None
    problem = _check_tpresult(test, results)
    if problem:
        return 'FAIL', problem, results[0] if results else None
    r = results[0]
    if r.get('observed') != '1' or r.get('source') != term:
        return 'FAIL', 'TPRESULT observed=%s source=%s, expected 1/%s' % (r.get('observed'), r.get('source'), term), r
    lines = stdout.splitlines()
    i_arm = next(i for i, l in enumerate(lines) if l.startswith('TPARM|'))
    i_res = next(i for i, l in enumerate(lines) if l.startswith('TPRESULT|'))
    if i_res < i_arm:
        return 'FAIL', 'TPRESULT printed before TPARM', r
    ev = test.get('evidence', {})
    if term == 'asan':
        if crashes:
            return 'FAIL', 'unexpected TPCRASH line in an ASan test', r
        problem = _asan_evidence_problem(ev, asan)
    elif term == 'exception':
        if asan:
            return 'FAIL', 'unexpected ASan report in an exception test', r
        problem = _veh_evidence_problem(ev, crashes, exe)
    else:
        problem = 'unknown termination %r' % term
    if problem:
        return 'FAIL', problem, r
    return 'XFAIL', 'defect mechanism observed (%s evidence verified by runner)' % term, r


def run_one(test, exe, args, results_dir):
    rec = {
        'id': test['id'], 'program': test['program'], 'test_case': test['test_case'],
        'kind': test['kind'], 'finding': test.get('finding', ''),
        'mechanism': test.get('mechanism', ''), 'expected': test.get('expected', ''),
        'timeout_s': test.get('timeout', 30),
        'termination': test.get('termination', 'normal'),
        'expect_exit': test.get('expect_exit', []),
        'evidence_expected': test.get('evidence', {}),
        'env_overrides': test.get('env', {}),
    }
    rec['started'] = datetime.datetime.now().isoformat(timespec='seconds')
    if not os.path.isfile(exe):
        st = 'SKIP' if args.allow_missing else 'FAIL'
        rec.update(status=st, reason='program not built: ' + exe, actual='program not built',
                   verification=VERIFICATION[st])
        return rec
    if test.get('needs_asan_runtime') and not os.path.isfile(os.path.join(os.path.dirname(exe), ASAN_DLL)):
        rec.update(status='FAIL', reason='ASan runtime %s missing next to the exe' % ASAN_DLL,
                   actual='ASan runtime missing', verification=VERIFICATION['FAIL'])
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

    env = base_env()
    env.update(test.get('env', {}))
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
    status, reason, r = classify(test, rc, timed_out, stdout, stderr, exe)
    if test.get('termination', 'normal') == 'asan':
        rec['asan_reports'] = parse_asan(stderr)
    crashes = parse_lines(stdout, 'TPCRASH')
    if crashes:
        rec['tpcrash'] = crashes
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
    lines = ['# Core test results', '']
    attention = [r for r in records if r['status'] in ('XPASS', 'FAIL')]
    if attention:
        lines += ['## Huomio / Attention', '',
                  'XPASS = vikaa ei toistettu: havainto tarkastettava, ei hyvaksytty tulos. / '
                  'defect NOT reproduced: review the finding, not an accepted result.', '']
        for r in attention:
            lines.append('- **%s** %s (%s/%s): %s' % (r['status'], r['id'], r.get('finding', ''),
                                                    r.get('mechanism', ''), r.get('reason', '')))
        lines.append('')
    lines += ['| | |', '|---|---|']
    for k in ('started', 'git_head', 'git_branch', 'production_rev', 'production_dirty', 'tests_rev',
              'tests_dirty', 'repo_dirty', 'host', 'python', 'doctest_version', 'summary'):
        lines.append('| %s | `%s` |' % (k, meta[k]))
    for prog, info in meta['programs'].items():
        lines.append('| %s | `%s` sha256 `%s`; %s |' % (
            prog, info.get('path', ''), info.get('sha256', '')[:16], ', '.join(info.get('build_info', []))))
    lines += ['', '| ID | Kind | Finding | Status | Verification | Actual |', '|---|---|---|---|---|---|']
    for r in records:
        st = '**XPASS (!)**' if r['status'] == 'XPASS' else '**%s**' % r['status']
        lines.append('| %s | %s | %s | %s | %s | %s |' % (
            r['id'], r['kind'], r.get('finding', ''), st, r['verification'],
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
        'production_rev': git('log', '-1', '--format=%H %cI', '--', *PROD_SPEC),
        'tests_rev': git('log', '-1', '--format=%H %cI', '--', *TESTS_SPEC),
        'tests_dirty': git('status', '--porcelain', '--', *TESTS_SPEC) != '',
        'tests_status': git('status', '--porcelain', '--', *TESTS_SPEC).splitlines(),
        'manifest_sha256': sha256(MANIFEST),
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
            newest, newest_name = newest_source_mtime(prog)
            info['older_than_sources'] = newest > mt
            pdb = os.path.splitext(exe)[0] + '.pdb'
            if os.path.isfile(pdb):
                info['pdb_sha256'] = sha256(pdb)
            dll = os.path.join(os.path.dirname(exe), ASAN_DLL)
            if os.path.isfile(dll):
                info['asan_runtime_sha256'] = sha256(dll)
            if newest > mt:
                info['newer_source'] = newest_name
                print('WARNING: %s is older than %s; rebuild before trusting results' % (rel, newest_name))
        meta['programs'][prog] = info
    # Tests run strictly one after another (loopback ports, sandboxes).
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
    meta['xpass_ids'] = [r['id'] for r in records if r['status'] == 'XPASS']
    meta['fail_ids'] = [r['id'] for r in records if r['status'] == 'FAIL']
    with open(os.path.join(out, 'results.json'), 'w', encoding='utf-8') as f:
        json.dump({'meta': meta, 'results': records}, f, indent=2)
    print('summary:', ', '.join('%s=%d' % kv for kv in sorted(counts.items())), '| results:', out)
    for r in records:
        if r['status'] == 'XPASS':
            sys.stderr.write('!!! XPASS (defect NOT reproduced - review finding, not accepted): %s (%s/%s)\n'
                             % (r['id'], r.get('finding'), r.get('mechanism')))
        elif r['status'] == 'FAIL':
            sys.stderr.write('!!! FAIL %s: %s\n' % (r['id'], r.get('reason')))
    if counts.get('FAIL'):
        return 1
    return 3 if counts.get('XPASS') else 0


if __name__ == '__main__':
    sys.exit(main())
