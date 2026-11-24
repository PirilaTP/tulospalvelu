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

    # ---- crash tests with evidence (termination asan / exception)

    ASAN_T = {'kind': 'defect', 'finding': 'H-041', 'mechanism': 'm', 'termination': 'asan',
              'expect_exit': ['77'],
              'evidence': {'bug_type': 'stack-buffer-overflow', 'access': 'WRITE',
                           'frame': 'kilptietue::write', 'variable': 'buf', 'variable_size': 10000,
                           'stack_contains': ['kilptietue::write']}}
    VEH_T = {'kind': 'defect', 'finding': 'H-027', 'mechanism': 'n', 'termination': 'exception',
             'expect_exit': ['0xC0000005'],
             'evidence': {'code': '0xC0000005', 'access': 'read', 'target': '0x0000000C',
                          'thread': 'tcp-client', 'symbol': 'reconnectTCP'}}
    ASAN_ERR = ('==1==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x1 at pc 0x2\n'
                'WRITE of size 8 at 0x0113f978 thread T0\n'
                '    #0 0x6556d00d in _asan_memset+0x1 (x.dll+0x1)\n'
                '    #1 0x004911a8 in kilppvtp::pack C:\\a\\HkDat.cpp:221\n'
                '    #2 0x004911b0 in kilptietue::write C:\\a\\HkDat.cpp:586\n'
                'Address 0x0113f978 is located in stack of thread T0 at offset 10016 in frame\n'
                '    #0 0x0049112f in kilptietue::write C:\\a\\HkDat.cpp:581\n'
                '  This frame has 1 object(s):\n'
                "    [16, 10016) 'buf' <== Memory access at offset 10016 overflows this variable\n"
                '==1==ABORTING\n')
    ARM_A = 'TPARM|finding=H-041|mechanism=m|source=asan\n'
    RES_A = 'TPRESULT|finding=H-041|mechanism=m|observed=1|source=asan|detail=x\n'
    ARM_V = 'TPARM|finding=H-027|mechanism=n|source=exception\n'
    CRASH_V = ('TPCRASH|source=veh|code=0xC0000005|access=read|target=0x0000000C|thread=tcp-client|'
               'symbols=reconnectTCP>tiedonsiirtoTCP\n')
    RES_V = 'TPRESULT|finding=H-027|mechanism=n|observed=1|source=exception|detail=x\n'

    def test_asan_full_evidence_is_xfail(self):
        self.assertEqual(r.classify(self.ASAN_T, 77, False, self.ARM_A + self.RES_A, self.ASAN_ERR)[0], 'XFAIL')

    def test_asan_report_without_tpresult_is_fail(self):
        self.assertEqual(r.classify(self.ASAN_T, 77, False, self.ARM_A, self.ASAN_ERR)[0], 'FAIL')

    def test_asan_tpresult_without_report_is_fail(self):
        self.assertEqual(r.classify(self.ASAN_T, 77, False, self.ARM_A + self.RES_A, '')[0], 'FAIL')

    def test_asan_wrong_exit_is_fail(self):
        self.assertEqual(r.classify(self.ASAN_T, 1, False, self.ARM_A + self.RES_A, self.ASAN_ERR)[0], 'FAIL')

    def test_asan_without_arm_is_fail(self):
        self.assertEqual(r.classify(self.ASAN_T, 77, False, self.RES_A, self.ASAN_ERR)[0], 'FAIL')

    def test_asan_wrong_variable_size_is_fail(self):
        err = self.ASAN_ERR.replace('[16, 10016)', '[16, 9016)')
        self.assertEqual(r.classify(self.ASAN_T, 77, False, self.ARM_A + self.RES_A, err)[0], 'FAIL')

    def test_asan_wrong_frame_is_fail(self):
        err = self.ASAN_ERR.replace('#0 0x0049112f in kilptietue::write', '#0 0x0049112f in other')
        self.assertEqual(r.classify(self.ASAN_T, 77, False, self.ARM_A + self.RES_A, err)[0], 'FAIL')

    def test_asan_two_reports_is_fail(self):
        self.assertEqual(r.classify(self.ASAN_T, 77, False, self.ARM_A + self.RES_A,
                                    self.ASAN_ERR + self.ASAN_ERR)[0], 'FAIL')

    def test_asan_check_failed_before_crash_is_fail(self):
        out = 'x.cpp(1): ERROR: REQUIRE( a ) is NOT correct!\n' + self.ARM_A + self.RES_A
        self.assertEqual(r.classify(self.ASAN_T, 77, False, out, self.ASAN_ERR)[0], 'FAIL')

    def test_asan_timeout_is_fail(self):
        self.assertEqual(r.classify(self.ASAN_T, None, True, self.ARM_A + self.RES_A, self.ASAN_ERR)[0], 'FAIL')

    def test_crash_test_survived_observed1_is_fail(self):
        o = self.ARM_A + 'TPDISARM|\n' + self.RES_A + '[doctest] test cases: 1 | 1 passed | 0 failed | 0 skipped\n'
        self.assertEqual(r.classify(self.ASAN_T, 0, False, o, '')[0], 'FAIL')

    def test_crash_test_survived_disarmed_observed0_is_xpass(self):
        o = (self.ARM_A + 'TPDISARM|\n' + self.RES_A.replace('observed=1', 'observed=0')
             + '[doctest] test cases: 1 | 1 passed | 0 failed | 0 skipped\n')
        self.assertEqual(r.classify(self.ASAN_T, 0, False, o, '')[0], 'XPASS')

    def test_veh_full_evidence_is_xfail(self):
        self.assertEqual(r.classify(self.VEH_T, 0xC0000005, False, self.ARM_V + self.CRASH_V + self.RES_V,
                                    '')[0], 'XFAIL')

    def test_veh_wrong_target_is_fail(self):
        c = self.CRASH_V.replace('0x0000000C', '0x00000010')
        self.assertEqual(r.classify(self.VEH_T, 0xC0000005, False, self.ARM_V + c + self.RES_V, '')[0], 'FAIL')

    def test_veh_wrong_symbol_is_fail(self):
        c = self.CRASH_V.replace('reconnectTCP>', 'other>')
        self.assertEqual(r.classify(self.VEH_T, 0xC0000005, False, self.ARM_V + c + self.RES_V, '')[0], 'FAIL')

    def test_veh_crash_without_tpcrash_is_fail(self):
        self.assertEqual(r.classify(self.VEH_T, 0xC0000005, False, self.ARM_V + self.RES_V, '')[0], 'FAIL')

    def test_unarmed_crash_in_normal_test_is_fail(self):
        self.assertEqual(self.st(DEFECT, 0xC0000005, False, OBS1 + self.CRASH_V), 'FAIL')

    def test_blocked_threads_must_match_manifest(self):
        t = dict(DEFECT, blocked_threads=1)
        self.assertEqual(self.st(t, 0, False, out(extra=OBS1)), 'FAIL')
        self.assertEqual(self.st(t, 0, False, out(extra=OBS1 + 'TPEXIT|blocked_threads=1|mode=terminate\n')),
                         'XFAIL')
        self.assertEqual(self.st(DEFECT, 0, False, out(extra=OBS1 + 'TPEXIT|blocked_threads=1\n')), 'FAIL')

    def test_arm_in_normal_test_is_fail(self):
        self.assertEqual(self.st(DEFECT, 0, False, out(extra=self.ARM_A.replace('H-041', 'H-040') + OBS1)), 'FAIL')

    def test_survived_result_before_disarm_is_fail(self):
        o = (self.ARM_A + self.RES_A.replace('observed=1', 'observed=0') + 'TPDISARM|\n'
             + '[doctest] test cases: 1 | 1 passed | 0 failed | 0 skipped\n')
        self.assertEqual(r.classify(self.ASAN_T, 0, False, o, '')[0], 'FAIL')

    def test_veh_out_of_process_symbol_mismatch_is_fail(self):
        saved = r.resolve_rva
        try:
            r.resolve_rva = lambda exe, rva: 'otherFunction'
            c = self.CRASH_V.replace('|symbols=', '|rva=0x00002E97|symbols=')
            self.assertEqual(r.classify(self.VEH_T, 0xC0000005, False, self.ARM_V + c + self.RES_V, '',
                                        'x.exe')[0], 'FAIL')
            r.resolve_rva = lambda exe, rva: 'reconnectTCP'
            self.assertEqual(r.classify(self.VEH_T, 0xC0000005, False, self.ARM_V + c + self.RES_V, '',
                                        'x.exe')[0], 'XFAIL')
        finally:
            r.resolve_rva = saved

    def test_base_env_strips_sanitizer_and_tp_variables(self):
        keys = ['ASAN_OPTIONS', 'ASAN_SYMBOLIZER_PATH', 'UBSAN_OPTIONS', 'TP_SANDBOX_TOKEN', '_NT_SYMBOL_PATH']
        saved = {k: os.environ.get(k) for k in keys}
        try:
            for k in keys:
                os.environ[k] = 'x'
            env = r.base_env()
            for k in keys:
                self.assertNotIn(k, env)
        finally:
            for k, v in saved.items():
                if v is None:
                    os.environ.pop(k, None)
                else:
                    os.environ[k] = v

    def test_asan_size_and_summary_evidence(self):
        err = self.ASAN_ERR.replace('==1==ABORTING', 'SUMMARY: AddressSanitizer: stack-buffer-overflow '
                                    'C:\\a\\HkDat.cpp:221 in vatp::pack\n==1==ABORTING')
        t = dict(self.ASAN_T)
        t['evidence'] = dict(self.ASAN_T['evidence'], size=8, summary_contains='HkDat.cpp:221 in vatp::pack')
        self.assertEqual(r.classify(t, 77, False, self.ARM_A + self.RES_A, err)[0], 'XFAIL')
        t['evidence'] = dict(t['evidence'], size=160)
        self.assertEqual(r.classify(t, 77, False, self.ARM_A + self.RES_A, err)[0], 'FAIL')
        t['evidence'] = dict(t['evidence'], size=8, summary_contains='VTulostus.cpp:2228')
        self.assertEqual(r.classify(t, 77, False, self.ARM_A + self.RES_A, err)[0], 'FAIL')

    def test_blocked_thread_with_observed0_is_fail(self):
        t = dict(DEFECT, blocked_threads=1)
        self.assertEqual(self.st(t, 0, False, out(extra=OBS0 + 'TPEXIT|blocked_threads=1\n')), 'FAIL')

    def test_blocked_thread_absent_with_observed0_is_xpass(self):
        t = dict(DEFECT, blocked_threads=1)
        self.assertEqual(self.st(t, 0, False, out(extra=OBS0)), 'XPASS')
        self.assertEqual(self.st(t, 0, False, out(extra=OBS1)), 'FAIL')

    def test_parse_asan_fields(self):
        b = r.parse_asan(self.ASAN_ERR)[0]
        self.assertEqual((b['bug_type'], b['access'], b['size'], b['frame']),
                         ('stack-buffer-overflow', 'WRITE', '8', 'kilptietue::write'))
        self.assertIn('kilptietue::write', b['stack'])
        self.assertEqual(b['vars'], [{'name': 'buf', 'size': 10000, 'flagged': True}])

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
