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

Usage: python -I verify_log.py RUN.json OUT.json
Exit code 0 when both packets are matched, 1 otherwise.
"""

import json
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run_hkmaali as r  # noqa: E402


def main():
    run = json.load(open(sys.argv[1], encoding='utf-8'))
    checks = (run.get('probe') or {}).get('checks', {})
    res, ok = r.match_probe_in_log(checks)
    entries, _ = r.read_drop_log()
    out = {'run_json': os.path.abspath(sys.argv[1]), 'fw_exe': r.FW_EXE, 'log_copy': r.FW_LOG_COPY,
           'log_copy_sha256': r.sha256(r.FW_LOG_COPY) if os.path.exists(r.FW_LOG_COPY) else None,
           'drop_entries_total': len(entries) if entries is not None else None,
           'rules': (run.get('firewall') or {}).get('rules'), 'log': res, 'ok': ok,
           'verified_epoch': time.time(), 'verified': time.strftime('%Y-%m-%dT%H:%M:%S')}
    with open(sys.argv[2], 'w', encoding='utf-8') as f:
        json.dump(out, f, indent=2)
    print(json.dumps({'ok': ok, 'matched': res.get('matched_lines'), 'expected': res.get('expected'),
                      'drop_entries_total': out['drop_entries_total']}, indent=1))
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main())
