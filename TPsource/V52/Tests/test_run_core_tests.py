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

"""Self-tests of the result classification in run_core_tests.py.

Run: python -I test_run_core_tests.py   (standard library only)

They make sure that a crash, a timeout, a failed precondition or a result
for the wrong mechanism is never classified as XFAIL (defect reproduced).
"""

import os
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import run_core_tests as r  # noqa: E402

DEFECT = {'kind': 'defect', 'finding': 'H-040', 'mechanism': 'm1'}
CONTROL = {'kind': 'control'}


def out(passed=1, failed=0, cases=1, extra=''):
    return (extra + '\n[doctest] test cases: %d | %d passed | %d failed | 0 skipped\n'
            % (cases, passed, failed))


OBS1 = 'TPRESULT|finding=H-040|mechanism=m1|observed=1|detail=x\n'
OBS0 = 'TPRESULT|finding=H-040|mechanism=m1|observed=0|detail=x\n'


class Classify(unittest.TestCase):
    def st(self, test, rc, timed_out, stdout):
        return r.classify(test, rc, timed_out, stdout)[0]

    def test_defect_observed_is_xfail(self):
        self.assertEqual(self.st(DEFECT, 0, False, out(extra=OBS1)), 'XFAIL')

    def test_defect_not_observed_is_xpass(self):
        self.assertEqual(self.st(DEFECT, 0, False, out(extra=OBS0)), 'XPASS')

    def test_timeout_is_fail_even_with_result(self):
        self.assertEqual(self.st(DEFECT, None, True, out(extra=OBS1)), 'FAIL')

    def test_crash_without_summary_is_fail(self):
        self.assertEqual(self.st(DEFECT, 0xC0000005, False, OBS1), 'FAIL')

    def test_crash_exit_code_with_summary_is_fail(self):
        self.assertEqual(self.st(DEFECT, 0xC0000409, False, out(extra=OBS1)), 'FAIL')

    def test_failed_precondition_is_fail(self):
        self.assertEqual(self.st(DEFECT, 1, False, out(passed=0, failed=1, extra=OBS1)), 'FAIL')

    def test_missing_result_is_fail(self):
        self.assertEqual(self.st(DEFECT, 0, False, out()), 'FAIL')

    def test_two_results_is_fail(self):
        self.assertEqual(self.st(DEFECT, 0, False, out(extra=OBS1 + OBS1)), 'FAIL')

    def test_wrong_mechanism_is_fail(self):
        s = 'TPRESULT|finding=H-040|mechanism=other|observed=1|detail=x\n'
        self.assertEqual(self.st(DEFECT, 0, False, out(extra=s)), 'FAIL')

    def test_wrong_finding_is_fail(self):
        s = 'TPRESULT|finding=H-041|mechanism=m1|observed=1|detail=x\n'
        self.assertEqual(self.st(DEFECT, 0, False, out(extra=s)), 'FAIL')

    def test_bad_observed_value_is_fail(self):
        s = 'TPRESULT|finding=H-040|mechanism=m1|observed=yes|detail=x\n'
        self.assertEqual(self.st(DEFECT, 0, False, out(extra=s)), 'FAIL')

    def test_test_case_not_found_is_fail(self):
        self.assertEqual(self.st(DEFECT, 0, False, out(passed=0, cases=0)), 'FAIL')

    def test_skip(self):
        self.assertEqual(self.st(DEFECT, 0, False, out(extra='TPSKIP|reason=no port\n')), 'SKIP')

    def test_skip_does_not_hide_failure(self):
        s = 'TPSKIP|reason=x\n' + OBS1
        self.assertEqual(self.st(DEFECT, 1, False, out(passed=0, failed=1, extra=s)), 'FAIL')

    def test_skip_does_not_hide_crash(self):
        self.assertEqual(self.st(DEFECT, 0xC0000005, False, out(extra='TPSKIP|reason=x\n')), 'FAIL')

    def test_info_lines_keep_equals_signs(self):
        self.assertEqual(r.parse_info('TPINFO|a=1 b=2\nTPINFO|plain\n'), ['a=1 b=2', 'plain'])

    def test_control_pass(self):
        self.assertEqual(self.st(CONTROL, 0, False, out()), 'PASS')

    def test_control_failed_check_is_fail(self):
        self.assertEqual(self.st(CONTROL, 1, False, out(passed=0, failed=1)), 'FAIL')

    def test_control_with_result_line_is_fail(self):
        self.assertEqual(self.st(CONTROL, 0, False, out(extra=OBS1)), 'FAIL')

    def test_manifest_names_are_literal(self):
        # doctest splits --test-case filters at commas and treats * and ? as
        # wildcards; the runner also checks exact names with --list-test-cases.
        import json
        with open(r.MANIFEST, encoding='utf-8') as f:
            m = json.load(f)
        ids = set()
        for t in m['tests']:
            for ch in ',*?\\':
                self.assertNotIn(ch, t['test_case'], t['id'])
            self.assertNotIn(t['id'], ids)
            ids.add(t['id'])
            if t['kind'] == 'defect':
                self.assertTrue(t.get('finding') and t.get('mechanism'), t['id'])


if __name__ == '__main__':
    unittest.main()
