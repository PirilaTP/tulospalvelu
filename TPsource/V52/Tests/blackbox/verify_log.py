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

"""Matches an isolation probe's TCP and UDP test packets (from a
run_hkmaali.py run.json) against DROP entries in the administrator's copy
of the Windows Firewall log (run_hkmaali.FW_LOG_COPY) and writes the result
as JSON. A match needs protocol, destination 192.0.2.1, the probe's random
destination port and its recorded source port, within 3 minutes.

For a program run (C-11 stage 4) it also requires: no DROP entry with the
program's pid and no outgoing DROP from another non-probe pid while the
program ran, and the post-run probe's packets in the copy (so the copy
covers the program window).

Usage: python -I verify_log.py RUN.json OUT.json
Exit code 0 when both packets are matched, 1 otherwise.
"""

import json
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run_hkmaali as r  # noqa: E402


def program_drops(run, entries):
    """C-11 stage 4: DROP entries whose pid column is the program's pid
    (run.json run.pid), and all DROP entries logged while the program ran
    (local time, +-60 s), whatever their pid. Neither is expected; both are
    reported. Returns (result, no_program_drops)."""
    import datetime as dt
    pid = (run.get('run') or {}).get('pid')
    t0, t1 = run.get('program_started_epoch'), run.get('finished_epoch')
    by_pid, in_window = [], []
    for e in entries or []:
        if pid is not None and e.get('pid') == str(pid):
            by_pid.append(e['_line'])
        if t0 and t1:
            try:
                lt = dt.datetime.strptime(e['date'] + ' ' + e['time'], '%Y-%m-%d %H:%M:%S').timestamp()
            except (KeyError, ValueError):
                continue
            if t0 - 60 <= lt <= t1 + 60:
                in_window.append(e['_line'])
    probe_pids = {str((run.get(k) or {}).get('pid')) for k in ('probe', 'post_probe')}
    others = [l for l in in_window if l.split()[-1] not in probe_pids]
    # outgoing drops (path column SEND) from any other pid in the window fail
    # too: the block rules only cover the test path, so they would be ours
    others_send = [l for l in others if ' SEND ' in l]
    return ({'program_pid': pid, 'program_pid_drops': by_pid, 'window_epoch': [t0, t1],
             'drops_in_window': in_window, 'drops_in_window_not_probe': others,
             'send_drops_in_window_not_probe': others_send},
            pid is not None and not by_pid and not others_send)


def main():
    run = json.load(open(sys.argv[1], encoding='utf-8'))
    checks = (run.get('probe') or {}).get('checks', {})
    res, ok = r.match_probe_in_log(checks)
    entries, fields = r.read_drop_log()
    out = {'run_json': os.path.abspath(sys.argv[1]), 'fw_exe': r.FW_EXE, 'log_copy': r.FW_LOG_COPY,
           'log_copy_sha256': r.sha256(r.FW_LOG_COPY) if os.path.exists(r.FW_LOG_COPY) else None,
           'log_copy_mtime': (time.strftime('%Y-%m-%dT%H:%M:%S', time.localtime(os.path.getmtime(r.FW_LOG_COPY)))
                              if os.path.exists(r.FW_LOG_COPY) else None),
           'drop_entries_total': len(entries) if entries is not None else None,
           'log_has_pid_field': bool(fields) and 'pid' in fields,
           'rules': (run.get('firewall') or {}).get('rules'), 'log': res, 'ok': ok,
           'verified_epoch': time.time(), 'verified': time.strftime('%Y-%m-%dT%H:%M:%S')}
    if run.get('run'):
        pd, pd_ok = program_drops(run, entries)
        out['program'] = pd
        out['program_ok'] = pd_ok and out['log_has_pid_field']
        # bracketing probe after the program: its drops in the copy show the
        # copy was taken after the whole program window was logged
        post = (run.get('post_probe') or {})
        pres, pok = r.match_probe_in_log(post.get('checks', {}))
        out['post_probe_log'] = pres
        out['post_probe_ok'] = pok and post.get('isolation_verified') is True
        out['covers_run'] = pok
    with open(sys.argv[2], 'w', encoding='utf-8') as f:
        json.dump(out, f, indent=2)
    print(json.dumps({'ok': ok, 'matched': res.get('matched_lines'), 'expected': res.get('expected'),
                      'drop_entries_total': out['drop_entries_total'], 'program': out.get('program'),
                      'program_ok': out.get('program_ok'), 'post_probe_ok': out.get('post_probe_ok'),
                      'covers_run': out.get('covers_run')}, indent=1))
    good = ok and out.get('program_ok', True) and out.get('post_probe_ok', True)
    return 0 if good else 1


if __name__ == '__main__':
    sys.exit(main())
