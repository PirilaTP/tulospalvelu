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

"""Offline self-tests of the evaluation code of run_hkmaali.py, hkdebug.py
and verify_log.py (C-11 stage 4). No process is started.

Run: python -I test_run_hkmaali.py   (standard library only)

They make sure that a crash, a timeout, a wrong exception or a wrong place
is never evaluated as H-041 "confirmed by running", and that the log and
stack comparisons do what the report says.
"""

import copy
import os
import sys
import tempfile
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hkdebug  # noqa: E402
import run_hkmaali as r  # noqa: E402
import verify_log  # noqa: E402

BASE = 0x00B00000
HK = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', '..', 'TPexe', 'Hk', 'V521',
                  'HkMaali.exe')


E = hkdebug.HKMAALI_EXPECT
GOOD_LOG = {'ok': True, 'program_ok': True, 'post_probe_ok': True, 'covers_run': True}
NOISE = r'C:\Users\x\AppData\Local\Temp\DiagOutputDir\RdClientAutoTrace\MSRDCEventProcessor_0.etl'


def common():
    return {'probe': {'isolation_verified': True}, 'post_probe': {'isolation_verified': True},
            'exe_sha256': E['sha256'], 'exe_copy_sha256': E['sha256'], 'outside_changes': [NOISE],
            'outside_probe_file_exists': False, 'registry_key_exists_after': False}


def clean_run():
    return {'sockets_seen': [], 'job': {'TotalProcesses': 1},
            'wer': {'processes': {'found': [], 'polls': 40}, 'events_matching': [], 'wersvc_after': '1  STOPPED'}}


def control_run():
    rec = common()
    rec['run'] = dict(clean_run(), status='ok', exited_by_itself=True, kilp_dat_size=20000, debugger={
        'symbols': {'pdb_ok': True, 'pdb_guid': E['rsds_guid']}, 'fatal': None, 'fatal_count': 0,
        'unexpected_breakpoints': [], 'exceptions': [],
        'breakpoints': [{'kind': 'return', 'cookie_intact': True, 'return_intact': True}]})
    return rec


def overflow_run():
    rec = common()
    fe = '0x0019E000'
    fatal = {'code': '0xC0000409', 'params': ['0x00000002'], 'sym': '__report_gsfailure+0x5',
             'rva': hkdebug.hexs(E['report_gsfailure_rva']), 'first_chance': False, 'thread': 11, 't': 5.0,
             'evidence': {'context': {'Ebp': fe}, 'ret_at_esp': {'rva': '0x00023237'},
                          'ebp_chain': [{'ret': '0x00000000'}],
                          'watched_frame': {'frame_ebp': fe, 'cookie_intact': False, 'return_intact': False,
                                            'entry_return_sym': 'kilptietue::addtall+0x123', 'entry_thread': 11}}}
    rec['run'] = dict(clean_run(), status='fatal', steps=[{'seen': True}] * 6, kilp_dat_size=20080,
                      exit_code=hkdebug.KILLED_BY_DEBUGGER,
                      stack_vs_record={'equal': True, 'compared_bytes': 40},
                      debugger={'symbols': {'pdb_ok': True, 'pdb_guid': E['rsds_guid']},
                                'image_base': hkdebug.hexs(BASE), 'fatal_count': 1, 'unexpected_breakpoints': [],
                                'terminated_by_debugger': True,
                                'exceptions': [{'code': '0xC0000409', 'first_chance': False, 't': 5.0}],
                                'fatal': fatal})
    return rec


def logs():
    return {'control': dict(GOOD_LOG), 'overflow': dict(GOOD_LOG)}


class EvaluateH041(unittest.TestCase):
    def verdict(self, c, o, lg=None):
        return hkdebug.evaluate_h041(c, o, lg if lg is not None else logs())['verdict']

    def test_all_criteria_confirm(self):
        res = hkdebug.evaluate_h041(control_run(), overflow_run(), logs())
        self.assertEqual(res['verdict'], 'confirmed by running', res['criteria'])

    def mutate(self, which, fn):
        c, o = control_run(), overflow_run()
        fn(c if which == 'c' else o)
        return self.verdict(c, o)

    def fatal(self, x):
        return x['run']['debugger']['fatal']

    def frame(self, x):
        return x['run']['debugger']['fatal']['evidence']['watched_frame']

    def test_crash_without_control_is_not_confirmed(self):
        self.assertEqual(self.mutate('c', lambda x: x['run'].update(kilp_dat_size=10000)), 'not confirmed')

    def test_control_with_exception_is_not_confirmed(self):
        self.assertEqual(self.mutate('c', lambda x: x['run']['debugger'].update(fatal={'code': '0xC0000005'})),
                         'not confirmed')

    def test_other_exception_code_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: self.fatal(x).update(code='0xC0000005')), 'not confirmed')

    def test_other_fast_fail_code_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: self.fatal(x).update(params=['0x00000007'])), 'not confirmed')

    def test_other_exception_address_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: self.fatal(x).update(rva='0x00001234')), 'not confirmed')

    def test_wrong_place_is_not_confirmed(self):
        def f(x):
            self.fatal(x)['evidence']['ret_at_esp'] = {'rva': '0x00012345'}
        self.assertEqual(self.mutate('o', f), 'not confirmed')

    def test_ebp_not_watched_frame_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: self.fatal(x)['evidence']['context'].update(Ebp='0x1')),
                         'not confirmed')

    def test_wrong_caller_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: self.frame(x).update(entry_return_sym='main+0x10')),
                         'not confirmed')

    def test_other_thread_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: self.frame(x).update(entry_thread=12)), 'not confirmed')

    def test_intact_cookie_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: self.frame(x).update(cookie_intact=True)), 'not confirmed')

    def test_intact_return_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: self.frame(x).update(return_intact=True)), 'not confirmed')

    def test_not_terminated_by_debugger_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x['run'].update(exit_code=0)), 'not confirmed')

    def test_two_fatal_exceptions_are_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x['run']['debugger'].update(fatal_count=2)), 'not confirmed')

    def test_earlier_second_chance_exception_is_not_confirmed(self):
        # realistic shape: the debugger keeps the FIRST fatal exception; an
        # earlier different second-chance exception means the fatal record
        # is not the 0xC0000409 one, or a later 0xC0000409 is not the first
        def f(x):
            x['run']['debugger']['exceptions'].insert(0, {'code': '0xC0000005', 'first_chance': False, 't': 4.0})
            x['run']['debugger']['fatal_count'] = 2
        self.assertEqual(self.mutate('o', f), 'not confirmed')

    def test_unexpected_breakpoint_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x['run']['debugger'].update(
            unexpected_breakpoints=[{'address': '0x1'}])), 'not confirmed')

    def test_wrong_file_size_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x['run'].update(kilp_dat_size=10040)), 'not confirmed')

    def test_stack_record_mismatch_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x['run'].update(stack_vs_record={'equal': False})),
                         'not confirmed')

    def test_pdb_mismatch_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x['run']['debugger']['symbols'].update(pdb_ok=False)),
                         'not confirmed')

    def test_other_binary_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x.update(exe_sha256='b', exe_copy_sha256='b')), 'not confirmed')

    def test_post_probe_missing_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x.pop('post_probe')), 'not confirmed')

    def test_wer_process_is_not_confirmed(self):
        def f(x):
            x['run']['wer']['processes']['found'] = [{'name': 'WerFault.exe'}]
        self.assertEqual(self.mutate('o', f), 'not confirmed')

    def test_wersvc_running_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x['run']['wer'].update(wersvc_after='4  RUNNING')),
                         'not confirmed')

    def test_child_process_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x['run'].update(job={'TotalProcesses': 2})), 'not confirmed')

    def test_sockets_are_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x['run'].update(sockets_seen=[{'endpoints': ['TCP']}])),
                         'not confirmed')

    def test_outside_change_is_not_confirmed(self):
        def f(x):
            x['outside_changes'] = [NOISE, r'C:\Users\x\AppData\Local\Temp\new.txt']
        self.assertEqual(self.mutate('o', f), 'not confirmed')

    def test_wer_dir_change_is_never_noise(self):
        self.assertFalse(hkdebug.outside_noise(r'C:\ProgramData\Microsoft\Windows\WER\DiagOutputDir'
                                               r'\RdClientAutoTrace\x.etl'))
        self.assertTrue(hkdebug.outside_noise(NOISE))

    def test_unverified_isolation_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x.update(probe={'isolation_verified': False})), 'not confirmed')

    def test_ui_step_missing_is_not_confirmed(self):
        self.assertEqual(self.mutate('o', lambda x: x['run'].update(steps=[{'seen': True}, {'seen': False}])),
                         'not confirmed')

    def test_log_not_covering_run_is_not_confirmed(self):
        lg = logs()
        lg['overflow']['covers_run'] = False
        self.assertEqual(self.verdict(control_run(), overflow_run(), lg), 'not confirmed')

    def test_program_drops_are_not_confirmed(self):
        lg = logs()
        lg['control']['program_ok'] = False
        self.assertEqual(self.verdict(control_run(), overflow_run(), lg), 'not confirmed')

    def test_missing_logs_are_not_confirmed(self):
        self.assertEqual(hkdebug.evaluate_h041(control_run(), overflow_run())['verdict'], 'not confirmed')


class StackVsRecord(unittest.TestCase):
    def test_equal_and_unequal(self):
        d = tempfile.mkdtemp()
        rec = bytes(10040) + bytes(10000) + bytes(range(40))
        with open(os.path.join(d, 'KILP.DAT'), 'wb') as f:
            f.write(rec)
        rc = {'recsize': 10040, 'bufsize': 10000}
        fatal = {'evidence': {'watched_frame': {'slots_hex': bytes(range(40)).hex() + 'ffffffff'}}}
        self.assertTrue(r.stack_vs_record(fatal, d, rc)['equal'])
        fatal['evidence']['watched_frame']['slots_hex'] = bytes(44).hex()
        self.assertFalse(r.stack_vs_record(fatal, d, rc)['equal'])
        self.assertIn('error', r.stack_vs_record({'evidence': {}}, d, rc))


class ProgramDrops(unittest.TestCase):
    def test_pid_and_window(self):
        import datetime
        t = datetime.datetime(2026, 11, 24, 22, 0, 0).timestamp()
        run = {'run': {'pid': 4242}, 'probe': {'pid': 7}, 'program_started_epoch': t, 'finished_epoch': t + 30}
        e = [{'date': '2026-11-24', 'time': '22:00:10', 'pid': '4242', '_line': 'A 4242'},
             {'date': '2026-11-24', 'time': '22:00:05', 'pid': '7', '_line': 'B 7'},
             {'date': '2026-11-24', 'time': '21:00:00', 'pid': '9', '_line': 'C 9'}]
        res, ok = verify_log.program_drops(run, e)
        self.assertFalse(ok)
        self.assertEqual(res['program_pid_drops'], ['A 4242'])
        self.assertEqual(res['drops_in_window_not_probe'], ['A 4242'])
        res, ok = verify_log.program_drops(run, e[1:])
        self.assertTrue(ok)
        self.assertEqual(res['drops_in_window'], ['B 7'])

    def test_send_drop_from_other_pid_in_window_is_not_ok(self):
        import datetime
        t = datetime.datetime(2026, 11, 24, 22, 0, 0).timestamp()
        run = {'run': {'pid': 4242}, 'probe': {'pid': 7}, 'post_probe': {'pid': 8},
               'program_started_epoch': t, 'finished_epoch': t + 30}
        e = [{'date': '2026-11-24', 'time': '22:00:20', 'pid': '8', '_line': 'x SEND 8'},
             {'date': '2026-11-24', 'time': '22:00:21', 'pid': '99', '_line': 'y RECEIVE 99'}]
        self.assertTrue(verify_log.program_drops(run, e)[1])
        e.append({'date': '2026-11-24', 'time': '22:00:22', 'pid': '99', '_line': 'z SEND 99'})
        res, ok = verify_log.program_drops(run, e)
        self.assertFalse(ok)
        self.assertEqual(res['send_drops_in_window_not_probe'], ['z SEND 99'])

    def test_missing_pid_is_not_ok(self):
        self.assertFalse(verify_log.program_drops({'run': {}}, [])[1])


class CrashChecks(unittest.TestCase):
    def test_missing_fatal_is_a_problem(self):
        rec = {'debugger': {'loop_finished': True, 'symbols': {'pdb_ok': True}}, 'job': {'ActiveProcesses': 0},
               'exit_code': 0, 'wer': {'processes': {'found': []}, 'events_matching': []}}
        self.assertTrue(r.check_crash_scenario('gs104_overflow', rec, {}))

    def test_wer_process_is_a_problem(self):
        rec = {'debugger': {'loop_finished': True, 'symbols': {'pdb_ok': True}, 'breakpoints': []},
               'job': {'ActiveProcesses': 0}, 'exit_code': 0,
               'wer': {'processes': {'found': [{'name': 'WerFault.exe', 'why': ['name']}]}, 'events_matching': []}}
        self.assertTrue(any('WER' in x for x in r.check_crash_scenario('gs64_no_overflow', rec, {})))


@unittest.skipUnless(os.path.exists(HK), 'shipped HkMaali.exe not present')
class PeInfo(unittest.TestCase):
    def test_hkmaali(self):
        pe = hkdebug.pe_info(HK)
        self.assertEqual(pe['image_base'], 0x400000)
        self.assertEqual(pe['machine'], '0x0000014C')
        self.assertEqual(pe['rsds']['guid'], r.HKMAALI_EXPECT['rsds_guid'])


if __name__ == '__main__':
    unittest.main(verbosity=1)
