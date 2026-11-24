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

"""Controlled black-box run of the shipped console HkMaali.exe
(doc/arkkitehtuuri/testaus.md, section 16). Windows only, standard library.

Isolation, all set up BEFORE the program starts and checked by a probe:
  - Windows AppContainer with NO capabilities: the OS (WFP) blocks every
    network connection, loopback included. No admin rights needed.
  - A fresh sandbox directory under %TEMP%; only the AppContainer SID gets
    access to it, so the program can write nowhere else (the probe checks a
    write outside the sandbox is refused).
  - A Job object with KILL_ON_JOB_CLOSE (process tree always ends with the
    driver) and DIE_ON_UNHANDLED_EXCEPTION (no Windows Error Reporting, so
    no crash dumps or dialogs outside the sandbox).
  - A console created by the driver (AllocConsole) and inherited by the
    program: the console host of an AppContainer process refuses input
    from outside (WriteConsoleInput: access denied), so the driver owns the
    console instead. Keys are sent only after the expected prompt text is on
    screen; per-step and total timeouts; on timeout the job is terminated.
The probe (isolation_probe.exe) runs first in the same container, sandbox
and job. If any probe check fails, the program is NOT started.

Mode "firewall" (--mode firewall, chosen by the user after run 0):
  - Windows Firewall block rules (inbound and outbound, all protocols) for
    the fixed program path FW_EXE, created by the user in an elevated shell
    (see FIREWALL_COMMANDS). The driver checks the rules exist, are enabled
    and point at FW_EXE, and that the firewall is on, before anything runs.
  - The probe is copied to FW_EXE itself, so the same rules apply to it; it
    must show TCP and UDP to an external address refused. Only then is the
    probe removed and the real program copied to FW_EXE.
  - The process runs with a WRITE_RESTRICTED token (restricting SID
    RESTRICTED): writes need a grant to that SID, which only the sandbox
    (FW_DIR) gets; the probe checks writes to %TEMP%, the profile, Documents
    and LOCALAPPDATA are refused. Reads are unaffected, so console reads
    work (AppContainer and low integrity both deny ReadConsoleOutput, which
    blanks the program's screen; C-11 run 0).
  - Loopback (127.0.0.1) is not filtered by Windows Firewall; the probe
    records it as information only.

C-11 stage 4 additions (testaus.md section 19), firewall mode only:
  - The probe must see the external TCP connect refused with 10013 exactly;
    it also checks that HKCU writes are refused, and the outside list
    includes D:\\ and G:\\ (G: is Google Drive reporting FAT32, so NTFS
    ACLs cannot be assumed: only the probe's actual write attempt counts).
    A drive that is not present is recorded as such. The probe is killed
    after 30 s and then counts as not verified.
  - Every launched process gets its own job (ActiveProcessLimit 1, so no
    child process can start; accounting recorded). The program's pid is
    recorded for the firewall log match (verify_log.py).
  - TCP/UDP, IPv4/IPv6 endpoints of the program (iphlpapi) are sampled on
    every poll, including the wait for exit; WER processes and children of
    the program are watched in the process list; Application log WER
    events and the WerSvc state are recorded (hkdebug.py).
  - --debug: the driver is the program's debugger (hkdebug.Debugger):
    every exception is recorded; a fatal one (0xC0000409, second chance,
    non-continuable) gets context, stack and frame evidence and the process
    is terminated before WER can start. --watch NAME sets a one-shot
    breakpoint at that function (PDB next to --exe) and at its return
    site, recording the frame before and after.
  - --crash-control: the debugger mode on crash_probe.exe (deliberate
    crashes), no HkMaali. --self-test: endpoint, process watcher, probe
    timeout, symbol and crash-control checks.
  - Script steps may wait for a cursor position ("cursor": [x, y]) and
    navigate with a key until the cursor reaches a field ("nav").

Usage:
  python -I run_hkmaali.py --exe PATH --probe PATH --script SCRIPT.json --out DIR
                           [--data SRC=DST ...] [--mode appcontainer|firewall]
                           [--debug] [--watch FUNCTION]
  python -I run_hkmaali.py --crash-control --crash-probe PATH --out DIR
  python -I run_hkmaali.py --self-test --exe PATH --probe PATH --crash-probe PATH --out DIR
SCRIPT.json: {"args": ["BOOT"], "steps": [{"wait": "text", "key": "\\r"}, ...],
              "expect_exit": true, "total_timeout": 60, "step_timeout": 10}
A step with "key": null only waits for the text. Optional in a step:
"cursor": [x, y] (also wait for the cursor there), "key_delay": seconds
between keys. A navigation step {"nav": "\\t", "until_cursor": [x, y],
"max": 30} presses the key until the cursor is at (x, y). After the last
step the driver waits for the process to exit. Optional "record_check":
{"recsize": N, "bufsize": M} compares the stack bytes past the watched
frame's buffer with the record bytes [M, N) of record 1 in KILP.DAT.
"""

import argparse
import ctypes
import datetime
import hashlib
import json
import os
import shutil
import socket
import subprocess
import sys
import tempfile
import threading
import time
from ctypes import wintypes

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hkdebug  # noqa: E402

k32 = ctypes.WinDLL('kernel32', use_last_error=True)
adv = ctypes.WinDLL('advapi32', use_last_error=True)
uenv = ctypes.WinDLL('userenv', use_last_error=True)
user32 = ctypes.WinDLL('user32', use_last_error=True)

CONTAINER = 'tulospalvelu.c11.hkmaali'
FW_DIR = os.path.join(os.environ.get('LOCALAPPDATA', ''), 'tulospalvelu-c11', 'hkmaali-fw')
FW_EXE = os.path.join(FW_DIR, 'HkMaali.exe')
FW_RULE = 'TP C11 HkMaali block'
FW_LOG_DIR = os.path.join(os.environ.get('LOCALAPPDATA', ''), 'tulospalvelu-c11', 'fwlog')
FW_LOG = os.path.join(FW_LOG_DIR, 'pfirewall.log')
FW_LOG_DEFAULT = r'%systemroot%\system32\LogFiles\Firewall\pfirewall.log'
# The firewall service gives its log an ACL this account cannot read; an
# administrator copies it next to it (the copy inherits the folder ACL).
FW_LOG_COPY = os.path.join(FW_LOG_DIR, 'pfirewall_copy.log')
LOG_COPY_COMMAND = ("Get-Content -LiteralPath '%s' | Set-Content -LiteralPath '%s' -Encoding ascii"
                    % (FW_LOG, FW_LOG_COPY))
LOGGING_ON_COMMAND = ("Set-NetFirewallProfile -Profile Domain,Private,Public -LogBlocked True "
                      "-LogFileName '%s'" % FW_LOG)
LOGGING_RESTORE_COMMAND = ("Set-NetFirewallProfile -Profile Domain,Private,Public -LogBlocked False "
                           "-LogFileName '%s'" % FW_LOG_DEFAULT)
FIREWALL_COMMANDS = [
    "New-NetFirewallRule -DisplayName '%s out' -Direction Outbound -Program '%s' -Action Block -Profile Any" % (FW_RULE, FW_EXE),
    "New-NetFirewallRule -DisplayName '%s in' -Direction Inbound -Program '%s' -Action Block -Profile Any" % (FW_RULE, FW_EXE),
]
SIDSTR = [None]   # container SID string, set in main()
EXTENDED_STARTUPINFO_PRESENT = 0x00080000
CREATE_NEW_CONSOLE = 0x00000010
CREATE_SUSPENDED = 0x00000004
CREATE_UNICODE_ENVIRONMENT = 0x00000400
PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES = 0x00020009
PROC_THREAD_ATTRIBUTE_HANDLE_LIST = 0x00020002
STARTF_USESTDHANDLES = 0x00000100
JOB_KILL_ON_CLOSE = 0x00002000
JOB_DIE_ON_UNHANDLED_EXCEPTION = 0x00000400


class SECURITY_CAPABILITIES(ctypes.Structure):
    _fields_ = [('AppContainerSid', ctypes.c_void_p), ('Capabilities', ctypes.c_void_p),
                ('CapabilityCount', wintypes.DWORD), ('Reserved', wintypes.DWORD)]


class STARTUPINFOW(ctypes.Structure):
    _fields_ = [('cb', wintypes.DWORD), ('lpReserved', wintypes.LPWSTR), ('lpDesktop', wintypes.LPWSTR),
                ('lpTitle', wintypes.LPWSTR), ('dwX', wintypes.DWORD), ('dwY', wintypes.DWORD),
                ('dwXSize', wintypes.DWORD), ('dwYSize', wintypes.DWORD), ('dwXCountChars', wintypes.DWORD),
                ('dwYCountChars', wintypes.DWORD), ('dwFillAttribute', wintypes.DWORD),
                ('dwFlags', wintypes.DWORD), ('wShowWindow', wintypes.WORD), ('cbReserved2', wintypes.WORD),
                ('lpReserved2', ctypes.c_void_p), ('hStdInput', wintypes.HANDLE),
                ('hStdOutput', wintypes.HANDLE), ('hStdError', wintypes.HANDLE)]


class STARTUPINFOEXW(ctypes.Structure):
    _fields_ = [('StartupInfo', STARTUPINFOW), ('lpAttributeList', ctypes.c_void_p)]


class PROCESS_INFORMATION(ctypes.Structure):
    _fields_ = [('hProcess', wintypes.HANDLE), ('hThread', wintypes.HANDLE),
                ('dwProcessId', wintypes.DWORD), ('dwThreadId', wintypes.DWORD)]


class IO_COUNTERS(ctypes.Structure):
    _fields_ = [(n, ctypes.c_ulonglong) for n in ('r', 'w', 'o', 'rb', 'wb', 'ob')]


class JOBOBJECT_BASIC_LIMIT_INFORMATION(ctypes.Structure):
    _fields_ = [('PerProcessUserTimeLimit', ctypes.c_longlong), ('PerJobUserTimeLimit', ctypes.c_longlong),
                ('LimitFlags', wintypes.DWORD), ('MinimumWorkingSetSize', ctypes.c_size_t),
                ('MaximumWorkingSetSize', ctypes.c_size_t), ('ActiveProcessLimit', wintypes.DWORD),
                ('Affinity', ctypes.c_size_t), ('PriorityClass', wintypes.DWORD),
                ('SchedulingClass', wintypes.DWORD)]


class JOBOBJECT_EXTENDED_LIMIT_INFORMATION(ctypes.Structure):
    _fields_ = [('BasicLimitInformation', JOBOBJECT_BASIC_LIMIT_INFORMATION), ('IoInfo', IO_COUNTERS),
                ('ProcessMemoryLimit', ctypes.c_size_t), ('JobMemoryLimit', ctypes.c_size_t),
                ('PeakProcessMemoryUsed', ctypes.c_size_t), ('PeakJobMemoryUsed', ctypes.c_size_t)]


class KEY_EVENT_RECORD(ctypes.Structure):
    _fields_ = [('bKeyDown', wintypes.BOOL), ('wRepeatCount', wintypes.WORD),
                ('wVirtualKeyCode', wintypes.WORD), ('wVirtualScanCode', wintypes.WORD),
                ('UnicodeChar', wintypes.WCHAR), ('dwControlKeyState', wintypes.DWORD)]


class INPUT_RECORD(ctypes.Structure):
    _fields_ = [('EventType', wintypes.WORD), ('_pad', wintypes.WORD), ('KeyEvent', KEY_EVENT_RECORD)]


class COORD(ctypes.Structure):
    _fields_ = [('X', ctypes.c_short), ('Y', ctypes.c_short)]


class SMALL_RECT(ctypes.Structure):
    _fields_ = [(n, ctypes.c_short) for n in ('Left', 'Top', 'Right', 'Bottom')]


class CSBI(ctypes.Structure):
    _fields_ = [('dwSize', COORD), ('dwCursorPosition', COORD), ('wAttributes', wintypes.WORD),
                ('srWindow', SMALL_RECT), ('dwMaximumWindowSize', COORD)]


def sha256(path):
    h = hashlib.sha256()
    with open(path, 'rb') as f:
        for c in iter(lambda: f.read(65536), b''):
            h.update(c)
    return h.hexdigest()


def check(ok, what):
    if not ok:
        raise RuntimeError('%s failed (error %d)' % (what, ctypes.get_last_error()))


# ---------------------------------------------------------------- isolation

def container_sid():
    sid = ctypes.c_void_p()
    uenv.CreateAppContainerProfile.argtypes = [wintypes.LPCWSTR, wintypes.LPCWSTR, wintypes.LPCWSTR,
                                               ctypes.c_void_p, wintypes.DWORD, ctypes.POINTER(ctypes.c_void_p)]
    hr = uenv.CreateAppContainerProfile(CONTAINER, CONTAINER, 'C-11 HkMaali test container (no capabilities)',
                                        None, 0, ctypes.byref(sid))
    if hr & 0xFFFFFFFF == 0x800700B7:   # already exists
        uenv.DeriveAppContainerSidFromAppContainerName.argtypes = [wintypes.LPCWSTR, ctypes.POINTER(ctypes.c_void_p)]
        hr = uenv.DeriveAppContainerSidFromAppContainerName(CONTAINER, ctypes.byref(sid))
    if hr != 0:
        raise RuntimeError('AppContainer profile: HRESULT 0x%08X' % (hr & 0xFFFFFFFF))
    s = wintypes.LPWSTR()
    adv.ConvertSidToStringSidW.argtypes = [ctypes.c_void_p, ctypes.POINTER(wintypes.LPWSTR)]
    check(adv.ConvertSidToStringSidW(sid, ctypes.byref(s)), 'ConvertSidToStringSid')
    return sid, s.value


def grant(path, sidstr):
    r = subprocess.run(['icacls', path, '/grant', '*%s:(OI)(CI)F' % sidstr], capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError('icacls: ' + r.stdout + r.stderr)


class JOBOBJECT_BASIC_ACCOUNTING_INFORMATION(ctypes.Structure):
    _fields_ = [('TotalUserTime', ctypes.c_longlong), ('TotalKernelTime', ctypes.c_longlong),
                ('ThisPeriodTotalUserTime', ctypes.c_longlong), ('ThisPeriodTotalKernelTime', ctypes.c_longlong),
                ('TotalPageFaultCount', wintypes.DWORD), ('TotalProcesses', wintypes.DWORD),
                ('ActiveProcesses', wintypes.DWORD), ('TotalTerminatedProcesses', wintypes.DWORD)]


JOB_LIMIT_ACTIVE_PROCESS = 0x00000008


class Job:
    """KILL_ON_JOB_CLOSE + DIE_ON_UNHANDLED_EXCEPTION; with active_limit the
    job refuses any process beyond that many (no child processes)."""
    def __init__(self, active_limit=None):
        k32.CreateJobObjectW.restype = wintypes.HANDLE
        self.h = k32.CreateJobObjectW(None, None)
        check(self.h, 'CreateJobObject')
        info = JOBOBJECT_EXTENDED_LIMIT_INFORMATION()
        info.BasicLimitInformation.LimitFlags = JOB_KILL_ON_CLOSE | JOB_DIE_ON_UNHANDLED_EXCEPTION
        if active_limit:
            info.BasicLimitInformation.LimitFlags |= JOB_LIMIT_ACTIVE_PROCESS
            info.BasicLimitInformation.ActiveProcessLimit = active_limit
        self.active_limit = active_limit
        check(k32.SetInformationJobObject(self.h, 9, ctypes.byref(info), ctypes.sizeof(info)),
              'SetInformationJobObject')

    def terminate(self):
        k32.TerminateJobObject(self.h, 0xDEAD0001)   # recognisable: killed by the driver

    def accounting(self):
        a = JOBOBJECT_BASIC_ACCOUNTING_INFORMATION()
        if not k32.QueryInformationJobObject(self.h, 1, ctypes.byref(a), ctypes.sizeof(a), None):
            return {'error': ctypes.get_last_error()}
        return {'TotalProcesses': a.TotalProcesses, 'ActiveProcesses': a.ActiveProcesses,
                'TotalTerminatedProcesses': a.TotalTerminatedProcesses, 'ActiveProcessLimit': self.active_limit}

    def close(self):
        if self.h:
            k32.CloseHandle(self.h)   # KILL_ON_JOB_CLOSE
            self.h = None


def close_pi(pi):
    for h in (pi.hThread, pi.hProcess):
        if h:
            k32.CloseHandle(h)
    pi.hThread = pi.hProcess = None


RESTRICTING_SID = 'S-1-5-12'   # NT AUTHORITY\RESTRICTED


def write_restricted_token():
    """Primary token of this process made WRITE_RESTRICTED with the single
    restricting SID RESTRICTED: write access then also needs a grant to that
    SID (only the sandbox has one); reads are unaffected, so console reads
    work (unlike AppContainer or low integrity, which deny ReadConsoleOutput)."""
    tok = wintypes.HANDLE()
    k32.GetCurrentProcess.restype = wintypes.HANDLE
    adv.OpenProcessToken.argtypes = [wintypes.HANDLE, wintypes.DWORD, ctypes.POINTER(wintypes.HANDLE)]
    check(adv.OpenProcessToken(k32.GetCurrentProcess(), 0x0002 | 0x0001 | 0x0008 | 0x0080 | 0x0100,
                               ctypes.byref(tok)), 'OpenProcessToken')
    sid = ctypes.c_void_p()
    adv.ConvertStringSidToSidW.argtypes = [wintypes.LPCWSTR, ctypes.POINTER(ctypes.c_void_p)]
    check(adv.ConvertStringSidToSidW(RESTRICTING_SID, ctypes.byref(sid)), 'ConvertStringSidToSid')

    class SAA(ctypes.Structure):
        _fields_ = [('Sid', ctypes.c_void_p), ('Attributes', wintypes.DWORD)]
    rs = (SAA * 1)(SAA(sid, 0))
    new = wintypes.HANDLE()
    adv.CreateRestrictedToken.argtypes = [wintypes.HANDLE, wintypes.DWORD, wintypes.DWORD, ctypes.c_void_p,
                                          wintypes.DWORD, ctypes.c_void_p, wintypes.DWORD, ctypes.c_void_p,
                                          ctypes.POINTER(wintypes.HANDLE)]
    check(adv.CreateRestrictedToken(tok, 0x8, 0, None, 0, None, 1, rs, ctypes.byref(new)),
          'CreateRestrictedToken(WRITE_RESTRICTED)')
    k32.CloseHandle(tok)
    return new


def low_integrity_token():
    """Primary token of this process with the LOW mandatory label."""
    TOKEN_ALL = 0x0002 | 0x0001 | 0x0008 | 0x0080 | 0x0100   # DUPLICATE, ASSIGN_PRIMARY, QUERY, ADJUST_DEFAULT, ADJUST_SESSIONID
    tok = wintypes.HANDLE()
    k32.GetCurrentProcess.restype = wintypes.HANDLE
    adv.OpenProcessToken.argtypes = [wintypes.HANDLE, wintypes.DWORD, ctypes.POINTER(wintypes.HANDLE)]
    check(adv.OpenProcessToken(k32.GetCurrentProcess(), TOKEN_ALL, ctypes.byref(tok)), 'OpenProcessToken')
    new = wintypes.HANDLE()
    adv.DuplicateTokenEx.argtypes = [wintypes.HANDLE, wintypes.DWORD, ctypes.c_void_p, ctypes.c_int, ctypes.c_int,
                                     ctypes.POINTER(wintypes.HANDLE)]
    check(adv.DuplicateTokenEx(tok, 0, None, 2, 1, ctypes.byref(new)), 'DuplicateTokenEx')
    sid = ctypes.c_void_p()
    adv.ConvertStringSidToSidW.argtypes = [wintypes.LPCWSTR, ctypes.POINTER(ctypes.c_void_p)]
    check(adv.ConvertStringSidToSidW('S-1-16-4096', ctypes.byref(sid)), 'ConvertStringSidToSid')

    class TML(ctypes.Structure):
        _fields_ = [('Sid', ctypes.c_void_p), ('Attributes', wintypes.DWORD)]
    tml = TML(sid, 0x20)   # SE_GROUP_INTEGRITY
    adv.GetLengthSid.argtypes = [ctypes.c_void_p]
    adv.SetTokenInformation.argtypes = [wintypes.HANDLE, ctypes.c_int, ctypes.c_void_p, wintypes.DWORD]
    check(adv.SetTokenInformation(new, 25, ctypes.byref(tml), ctypes.sizeof(tml) + adv.GetLengthSid(sid)),
          'SetTokenInformation(TokenIntegrityLevel)')
    k32.CloseHandle(tok)
    return new


def launch(job, sid, exe, args, cwd, con, token=None, debug=False):
    """Starts exe suspended, either in the AppContainer (sid) or with the
    given token (low integrity), with the driver's console as its standard
    input/output/error (and no other inherited handle); assigns it to the
    job; resumes. With debug the calling thread becomes its debugger
    (DEBUG_ONLY_THIS_PROCESS) and must run the debug loop.
    Returns PROCESS_INFORMATION (the caller closes its handles)."""
    size = ctypes.c_size_t()
    k32.InitializeProcThreadAttributeList(None, 2, 0, ctypes.byref(size))
    attr = ctypes.create_string_buffer(size.value)
    check(k32.InitializeProcThreadAttributeList(attr, 2, 0, ctypes.byref(size)), 'InitializeProcThreadAttributeList')
    k32.UpdateProcThreadAttribute.argtypes = [ctypes.c_void_p, wintypes.DWORD, ctypes.c_size_t, ctypes.c_void_p,
                                              ctypes.c_size_t, ctypes.c_void_p, ctypes.c_void_p]
    if sid is not None:
        sc = SECURITY_CAPABILITIES(sid, None, 0, 0)
        check(k32.UpdateProcThreadAttribute(attr, 0, PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES,
                                            ctypes.byref(sc), ctypes.sizeof(sc), None, None),
              'UpdateProcThreadAttribute')
    handles = (wintypes.HANDLE * 2)(con.inp, con.out)
    check(k32.UpdateProcThreadAttribute(attr, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
                                        handles, ctypes.sizeof(handles), None, None),
          'UpdateProcThreadAttribute(handle list)')
    si = STARTUPINFOEXW()
    si.StartupInfo.cb = ctypes.sizeof(si)
    si.StartupInfo.dwFlags = STARTF_USESTDHANDLES
    si.StartupInfo.hStdInput = con.inp
    si.StartupInfo.hStdOutput = con.out
    si.StartupInfo.hStdError = con.out
    si.lpAttributeList = ctypes.cast(attr, ctypes.c_void_p)
    pi = PROCESS_INFORMATION()
    cmd = ctypes.create_unicode_buffer(subprocess.list2cmdline([exe] + list(args)))
    flags = EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED | (hkdebug.DEBUG_ONLY_THIS_PROCESS if debug else 0)
    if token is None:
        k32.CreateProcessW.argtypes = [wintypes.LPCWSTR, wintypes.LPWSTR, ctypes.c_void_p, ctypes.c_void_p,
                                       wintypes.BOOL, wintypes.DWORD, ctypes.c_void_p, wintypes.LPCWSTR,
                                       ctypes.c_void_p, ctypes.POINTER(PROCESS_INFORMATION)]
        check(k32.CreateProcessW(exe, cmd, None, None, True, flags,
                                 None, cwd, ctypes.byref(si), ctypes.byref(pi)), 'CreateProcess')
    else:
        adv.CreateProcessAsUserW.argtypes = [wintypes.HANDLE, wintypes.LPCWSTR, wintypes.LPWSTR, ctypes.c_void_p,
                                             ctypes.c_void_p, wintypes.BOOL, wintypes.DWORD, ctypes.c_void_p,
                                             wintypes.LPCWSTR, ctypes.c_void_p, ctypes.POINTER(PROCESS_INFORMATION)]
        check(adv.CreateProcessAsUserW(token, exe, cmd, None, None, True, flags,
                                       None, cwd, ctypes.byref(si), ctypes.byref(pi)), 'CreateProcessAsUser')
    k32.DeleteProcThreadAttributeList(attr)
    if not k32.AssignProcessToJobObject(job.h, pi.hProcess):
        k32.TerminateProcess(pi.hProcess, 1)
        close_pi(pi)
        raise RuntimeError('AssignProcessToJobObject failed (%d)' % ctypes.get_last_error())
    if k32.ResumeThread(pi.hThread) == 0xFFFFFFFF:
        k32.TerminateProcess(pi.hProcess, 1)
        close_pi(pi)
        raise RuntimeError('ResumeThread failed (%d)' % ctypes.get_last_error())
    return pi


# ---------------------------------------------------------------- console

class Console:
    """A fresh console owned by the driver; started programs inherit it.
    Nothing is written to it by the driver (the program's initcon expects
    the cursor at 0,0, tputilv2/initcon.cpp).
    Known limit (C-11, run 0): inside the AppContainer ReadConsoleOutput
    fails with access denied, also on a screen buffer whose DACL grants the
    container SID (optional sidstr below, tried and not used), so the
    program's screen routines (tputilv2/viwrrectw.cpp, Viwrrect.cpp,
    virdrect.cpp) skip every write silently."""
    def __init__(self, sidstr=None):
        k32.FreeConsole()
        check(k32.AllocConsole(), 'AllocConsole')
        k32.CreateFileW.restype = wintypes.HANDLE
        GR_GW, SHARE, OPEN = 0xC0000000, 3, 3

        class SA(ctypes.Structure):
            _fields_ = [('n', wintypes.DWORD), ('sd', ctypes.c_void_p), ('inherit', wintypes.BOOL)]
        sa = SA(ctypes.sizeof(SA), None, True)   # inheritable, passed via the handle list only
        self.out = k32.CreateFileW('CONOUT$', GR_GW, SHARE, ctypes.byref(sa), OPEN, 0, None)
        self.inp = k32.CreateFileW('CONIN$', GR_GW, SHARE, ctypes.byref(sa), OPEN, 0, None)
        if self.out in (None, wintypes.HANDLE(-1).value) or self.inp in (None, wintypes.HANDLE(-1).value):
            raise RuntimeError('console handles not opened')
        # Preset the 80x50 size the program sets in initscreens
        # (HkConsole/HkKilp.cpp:513-514, tputilv2/consolesize.cpp): window
        # first shrunk, then buffer, then window; cursor stays at 0,0.
        k32.SetConsoleWindowInfo(self.out, True, ctypes.byref(SMALL_RECT(0, 0, 79, 24)))
        k32.SetConsoleScreenBufferSize(self.out, COORD(80, 50))
        k32.SetConsoleWindowInfo(self.out, True, ctypes.byref(SMALL_RECT(0, 0, 79, 49)))
        if sidstr:
            # Owner (the driver's user), SYSTEM and the container SID: full access.
            sd = ctypes.c_void_p()
            adv.ConvertStringSecurityDescriptorToSecurityDescriptorW.argtypes = [
                wintypes.LPCWSTR, wintypes.DWORD, ctypes.POINTER(ctypes.c_void_p), ctypes.c_void_p]
            check(adv.ConvertStringSecurityDescriptorToSecurityDescriptorW(
                'D:(A;;GA;;;OW)(A;;GA;;;SY)(A;;GA;;;%s)' % sidstr, 1, ctypes.byref(sd), None),
                'ConvertStringSecurityDescriptor')
            sa2 = SA(ctypes.sizeof(SA), sd, True)
            k32.CreateConsoleScreenBuffer.restype = wintypes.HANDLE
            buf = k32.CreateConsoleScreenBuffer(GR_GW, SHARE, ctypes.byref(sa2), 1, None)
            if buf in (None, wintypes.HANDLE(-1).value):
                raise RuntimeError('CreateConsoleScreenBuffer failed (%d)' % ctypes.get_last_error())
            check(k32.SetConsoleActiveScreenBuffer(buf), 'SetConsoleActiveScreenBuffer')
            k32.CloseHandle(self.out)
            self.out = buf

    def screen(self):
        info = CSBI()
        if not k32.GetConsoleScreenBufferInfo(self.out, ctypes.byref(info)):
            return ''
        w, h = info.dwSize.X, info.dwSize.Y
        buf = ctypes.create_unicode_buffer(w * h)
        n = wintypes.DWORD()
        k32.ReadConsoleOutputCharacterW(self.out, buf, w * h, COORD(0, 0), ctypes.byref(n))
        text = buf[:n.value]
        return '\n'.join(text[i:i + w].rstrip() for i in range(0, len(text), w))

    def cursor(self):
        info = CSBI()
        if not k32.GetConsoleScreenBufferInfo(self.out, ctypes.byref(info)):
            return None
        return [info.dwCursorPosition.X, info.dwCursorPosition.Y]

    def key(self, ch):
        vk = {'\r': 0x0D, '\x1b': 0x1B, '\t': 0x09}.get(ch, user32.VkKeyScanW(ord(ch)) & 0xFF)
        scan = user32.MapVirtualKeyW(vk, 0)
        recs = (INPUT_RECORD * 2)()
        for i, down in enumerate((True, False)):
            recs[i].EventType = 1
            recs[i].KeyEvent = KEY_EVENT_RECORD(down, 1, vk, scan, ch, 0)
        n = wintypes.DWORD()
        check(k32.WriteConsoleInputW(self.inp, recs, 2, ctypes.byref(n)), 'WriteConsoleInput')

    def close(self):
        k32.CloseHandle(self.out)
        k32.CloseHandle(self.inp)
        k32.FreeConsole()

    def reopen(self):
        self.close()
        self.__init__()


# ---------------------------------------------------------------- watching

def watched_dirs(sandbox_root):
    la = os.environ.get('LOCALAPPDATA', '')
    pd = os.environ.get('PROGRAMDATA', '')
    return [d for d in (os.environ.get('TEMP', ''), os.path.join(la, 'CrashDumps'),
                        os.path.join(la, 'Microsoft', 'Windows', 'WER'),
                        os.path.join(pd, 'Microsoft', 'Windows', 'WER'),
                        os.path.join(la, 'Packages', CONTAINER)) if d]


def snapshot(dirs, skip):
    """{path: (size, mtime)} for files under dirs (depth <= 3), skipping skip."""
    out = {}
    for d in dirs:
        base_depth = d.rstrip('\\/').count(os.sep)
        for root, subdirs, files in os.walk(d):
            if any(os.path.normcase(root).startswith(os.path.normcase(s)) for s in skip):
                subdirs[:] = []
                continue
            if root.count(os.sep) - base_depth >= 3:
                subdirs[:] = []
            for f in files:
                p = os.path.join(root, f)
                try:
                    st = os.stat(p)
                    out[p] = (st.st_size, st.st_mtime)
                except OSError:
                    pass
    return out


WNDENUMPROC = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)


def windows_of(pid):
    """Visible top-level windows of pid (a MessageBox is one; the console
    window belongs to the console host, not to the program)."""
    found = []

    def cb(hwnd, _):
        p = wintypes.DWORD()
        user32.GetWindowThreadProcessId(hwnd, ctypes.byref(p))
        if p.value == pid and user32.IsWindowVisible(hwnd):
            buf = ctypes.create_unicode_buffer(256)
            user32.GetWindowTextW(hwnd, buf, 256)
            found.append(buf.value)
        return True
    user32.EnumWindows(WNDENUMPROC(cb), 0)
    return found


def sockets_of(pid):
    r = subprocess.run(['netstat', '-ano'], capture_output=True, text=True, errors='replace')
    return [l.strip() for l in r.stdout.splitlines() if l.split() and l.split()[-1] == str(pid)]


class Listener:
    """127.0.0.1 listener for the probe's loopback check; counts accepts."""
    def __init__(self):
        self.s = socket.socket()
        self.s.bind(('127.0.0.1', 0))
        self.s.listen(4)
        self.s.settimeout(0.2)
        self.port = self.s.getsockname()[1]
        self.accepts = 0
        self.stop = False
        self.t = threading.Thread(target=self.run, daemon=True)
        self.t.start()

    def run(self):
        while not self.stop:
            try:
                c, _ = self.s.accept()
                self.accepts += 1
                c.close()
            except OSError:
                pass

    def close(self):
        self.stop = True
        self.t.join(2)
        self.s.close()


# ---------------------------------------------------------------- runs

PROBE_TIMEOUT_S = 30


def run_probe(job, sid, sandbox, probe, outside, token=None, mode='appcontainer', ports=None,
              timeout_s=PROBE_TIMEOUT_S, args_override=None):
    """Runs the isolation probe; it is killed after timeout_s and then
    counts as not verified. args_override is for the driver self-test."""
    lst = Listener()
    con = Console()
    args = [outside, str(lst.port)] + ([str(ports[0]), str(ports[1])] if ports else [])
    if args_override is not None:
        args = args_override
    pi = launch(job, sid, probe, args, sandbox, con, token)
    pid = pi.dwProcessId
    t0 = time.time()
    while 'PROBE READY' not in con.screen() and time.time() - t0 < min(10, timeout_s):
        if k32.WaitForSingleObject(pi.hProcess, 0) == 0:
            break
        time.sleep(0.1)
    con.key('X')
    left = max(0, timeout_s - (time.time() - t0))
    timed_out = k32.WaitForSingleObject(pi.hProcess, int(left * 1000)) != 0
    if timed_out:
        k32.TerminateProcess(pi.hProcess, 0xDEAD0003)
        k32.WaitForSingleObject(pi.hProcess, 5000)
    code = wintypes.DWORD()
    k32.GetExitCodeProcess(pi.hProcess, ctypes.byref(code))
    close_pi(pi)
    con.close()
    lst.close()
    res = {'exit_code': code.value, 'exit_code_hex': '0x%08X' % code.value, 'pid': pid, 'timed_out': timed_out,
           'duration_s': round(time.time() - t0, 2), 'listener_accepts': lst.accepts, 'checks': {}}
    if timed_out:
        res['isolation_verified'] = False
        res['error'] = 'probe killed after %d s' % timeout_s
        return res
    try:
        with open(os.path.join(sandbox, 'probe_result.txt'), encoding='ascii') as f:
            for line in f:
                if '=' in line:
                    k, v = line.strip().split('=', 1)
                    res['checks'][k] = v
    except OSError as e:
        res['error'] = str(e)
    ext10013 = True
    if mode == 'firewall':
        # Windows Firewall does not filter loopback: informational only.
        # External TCP counts only when refused with 10013 (the firewall's
        # WSAEACCES), checked here too, not only in the probe.
        required = ('cwd', 'console', 'outside', 'external', 'registry')
        loop_ok = True
        ext10013 = ', error 10013 after ' in res['checks'].get('external', '')
        res['external_10013'] = ext10013
    else:
        required = ('cwd', 'console', 'outside', 'loopback', 'external')
        loop_ok = lst.accepts == 0
    if mode == 'firewall':
        res['registry_key_left'] = reg_probe_key_exists()
    ok = (res['checks'].get('done') == '1' and loop_ok and ext10013 and code.value == 0 and
          not res.get('registry_key_left') and
          all(res['checks'].get(k, '').startswith('isolated_or_ok') for k in required) and
          not any(os.path.exists(o) for o in outside.split(';') if o))
    res['required_checks'] = list(required)
    res['isolation_verified'] = ok
    return res


def _poll_net(pid, net):
    try:
        s = hkdebug.endpoints([pid])
    except OSError as e:
        s = ['endpoint query failed: %s' % e]
    if s:
        net.append({'t': round(time.time(), 2), 'endpoints': s})
    return s


def _kilp_size(sandbox):
    p = os.path.join(sandbox, 'KILP.DAT')
    return os.path.getsize(p) if os.path.exists(p) else None


def run_program(job, sid, sandbox, exe, script, log, token=None, debug=False, image_path=None, watch=None):
    """Runs the program with the console script. Every poll samples the
    program's network endpoints (also while waiting for the exit), windows
    and the KILP.DAT size. With debug the program runs under hkdebug's
    Debugger (image_path: the original exe with its PDB, for symbols)."""
    con = Console()
    args = script.get('args', [])
    wer_since = hkdebug.utcnow()
    wersvc_before = hkdebug.wersvc_state()
    watcher = hkdebug.ProcWatcher()
    watcher.start()
    dbg = None
    if debug:
        dbg = hkdebug.Debugger(lambda: launch(job, sid, exe, args, sandbox, con, token, debug=True),
                               image_path=image_path, watch=watch)
        dbg.start()
        dbg.ready.wait(30)
        if dbg.error or dbg.pi is None:
            watcher.stop()
            con.close()
            return {'status': 'launch failed: %s' % dbg.error, 'exited_by_itself': False}
        pi = dbg.pi
    else:
        pi = launch(job, sid, exe, args, sandbox, con, token)
    pid = pi.dwProcessId
    watcher.watch_pid(pid)
    total = script.get('total_timeout', 60)
    t0 = time.time()
    steps = []
    net = []
    status = 'ok'
    windows = []
    polls = [0]

    def alive():
        return k32.WaitForSingleObject(pi.hProcess, 0) != 0

    def poll():
        polls[0] += 1
        _poll_net(pid, net)
        w = windows_of(pid)
        if w:
            windows.append(w)
        return w

    for st in script.get('steps', []):
        deadline = time.time() + script.get('step_timeout', 10)
        seen = False
        rec = {'t_start': round(time.time() - t0, 2)}
        if 'nav' in st:
            target = st['until_cursor']
            trail = []
            for _ in range(st.get('max', 30)):
                if not alive() or time.time() - t0 >= total:
                    break
                cur = con.cursor()
                trail.append(cur)
                if cur == target:
                    seen = True
                    break
                time.sleep(0.3)
                con.key(st['nav'])
                end = time.time() + 3
                while time.time() < end and con.cursor() == cur and alive():
                    poll()
                    time.sleep(0.1)
            rec.update({'nav': st['nav'], 'until_cursor': target, 'seen': seen, 'cursor_trail': trail})
        else:
            waits = st['wait'] if isinstance(st['wait'], list) else [st['wait']]
            want_cur = st.get('cursor')
            while time.time() < deadline and time.time() - t0 < total:
                scr = con.screen()
                if poll():
                    break
                if all(x in scr for x in waits) and (want_cur is None or con.cursor() == want_cur):
                    seen = True
                    break
                if not alive():
                    break
                time.sleep(0.2)
            rec.update({'wait': st['wait'], 'cursor_wanted': want_cur, 'cursor': con.cursor(), 'seen': seen,
                        'key': st.get('key')})
        rec['t'] = round(time.time() - t0, 2)
        rec['kilp_dat_size'] = _kilp_size(sandbox)
        steps.append(rec)
        log.write('--- screen after step %d %r (seen=%s, cursor=%s, KILP.DAT=%s) ---\n%s\n' % (
            len(steps), st.get('wait', st.get('nav')), seen, con.cursor(), rec['kilp_dat_size'], con.screen()))
        if windows:
            status = 'window opened by the program: %r' % windows[-1]
            break
        if not seen:
            status = 'step not completed: %r' % (st.get('wait') or st.get('until_cursor'))
            break
        if st.get('key') is not None:
            time.sleep(0.3)   # the program flushes the key buffer before reading
            for ch in st['key']:
                con.key(ch)
                if st.get('key_delay'):
                    time.sleep(st['key_delay'])
    last = con.screen()
    exited = False
    if status == 'ok':
        end = time.time() + max(1.0, total - (time.time() - t0))
        while time.time() < end:
            if k32.WaitForSingleObject(pi.hProcess, 200) == 0:
                exited = True
                break
            if poll():
                status = 'window opened by the program: %r' % windows[-1]
                break
    exited = exited or not alive()
    if not exited:
        status = status if status != 'ok' else 'did not exit in time'
        log.write('--- final screen (killing) ---\n%s\n' % con.screen())
        if dbg:
            dbg.stop()
        job.terminate()
        k32.WaitForSingleObject(pi.hProcess, 5000)
    _poll_net(pid, net)   # after exit: should be empty; recorded if not
    code = wintypes.DWORD()
    k32.GetExitCodeProcess(pi.hProcess, ctypes.byref(code))
    log.write('--- last screen ---\n%s\n' % last)
    out = {'status': status, 'exited_by_itself': exited, 'exit_code': code.value, 'windows': windows,
           'exit_code_hex': '0x%08X' % code.value, 'pid': pid, 'steps': steps, 'sockets_seen': net,
           'endpoint_polls': polls[0], 'duration_s': round(time.time() - t0, 2),
           'kilp_dat_size': _kilp_size(sandbox), 'job': job.accounting()}
    if dbg:
        dbg.done.wait(15)
        d = dict(dbg.rec)
        d['loop_finished'] = dbg.done.is_set()
        d['error'] = dbg.error
        if not dbg.done.is_set():
            d['error'] = (d['error'] or '') + ' debug loop still running; handles left open'
            out['debugger'] = d
            out['status'] = 'debugger did not finish'
            con.close()
            watcher.stop()
            return out
        out['debugger'] = d
        f = d.get('fatal')
        if f:
            out['status'] = 'fatal exception %s at %s (%s), process terminated by the debugger' % (
                f.get('code'), f.get('address'), f.get('sym'))
            out['exited_by_itself'] = False
    close_pi(pi)
    con.close()
    time.sleep(1.5)   # WER would start within this time after a crash
    proc = watcher.stop()
    ev_all, ev_hit = hkdebug.wer_events(wer_since, [os.path.basename(exe), 'crash_probe'])
    out['wer'] = {'processes': proc, 'events_since_start': ev_all, 'events_matching': ev_hit,
                  'wersvc_before': wersvc_before, 'wersvc_after': hkdebug.wersvc_state()}
    rc = script.get('record_check')
    if rc and out.get('debugger', {}).get('fatal'):
        out['stack_vs_record'] = stack_vs_record(out['debugger']['fatal'], sandbox, rc)
    return out


def stack_vs_record(fatal, sandbox, rc):
    """Compares the bytes past the watched frame's buffer (slots read at the
    fatal exception, starting at frame EBP-4 = end of the buffer) with the
    record bytes [bufsize, recsize) of record 1 in KILP.DAT."""
    wf = (fatal.get('evidence') or {}).get('watched_frame') or {}
    hx = wf.get('slots_hex')
    if not hx:
        return {'error': 'no frame slots'}
    stack = bytes.fromhex(hx)
    n = rc['recsize'] - rc['bufsize']
    try:
        with open(os.path.join(sandbox, 'KILP.DAT'), 'rb') as f:
            f.seek(rc['recsize'] + rc['bufsize'])
            rec = f.read(n)
    except OSError as e:
        return {'error': str(e)}
    cmp_n = min(n, len(stack))
    return {'compared_bytes': cmp_n, 'stack_hex': stack[:cmp_n].hex(), 'record_hex': rec[:cmp_n].hex(),
            'equal': cmp_n == n and stack[:cmp_n] == rec[:cmp_n], 'record_all_zero': not any(rec)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe')
    ap.add_argument('--probe')
    ap.add_argument('--script')
    ap.add_argument('--out', required=True)
    ap.add_argument('--data', action='append', default=[], help='SRC=DST copied into the sandbox')
    ap.add_argument('--probe-only', action='store_true')
    ap.add_argument('--mode', choices=('appcontainer', 'firewall'), default='appcontainer')
    ap.add_argument('--udp-verified', help='verification JSON written by verify_log.py (firewall mode)')
    ap.add_argument('--debug', action='store_true', help='run the program under the driver as debugger')
    ap.add_argument('--watch', help='function to trace with one-shot breakpoints (needs --debug and the PDB)')
    ap.add_argument('--crash-control', action='store_true', help='debugger-mode control with crash_probe.exe')
    ap.add_argument('--self-test', action='store_true', help='driver self-tests (includes the crash control)')
    ap.add_argument('--crash-probe', help='crash_probe.exe built by build_probes.cmd')
    a = ap.parse_args()
    if a.crash_control:
        return crash_control_main(a)
    if a.self_test:
        return self_test_main(a)
    if not (a.exe and a.probe and a.script):
        ap.error('--exe, --probe and --script are required for a run')
    if a.mode == 'firewall':
        return main_firewall(a)
    if a.debug:
        ap.error('--debug is only supported with --mode firewall')
    os.makedirs(a.out, exist_ok=True)
    script = json.load(open(a.script, encoding='utf-8'))
    rec = {'started': datetime.datetime.now().isoformat(timespec='seconds'),
           'exe': os.path.abspath(a.exe), 'exe_sha256': sha256(a.exe),
           'probe_sha256': sha256(a.probe), 'driver_sha256': sha256(os.path.abspath(__file__)),
           'script': script, 'script_sha256': sha256(a.script), 'container': CONTAINER}
    root = tempfile.mkdtemp(prefix='hkmaali_run_')
    sandbox = os.path.join(root, 'sandbox')
    os.makedirs(sandbox)
    outside = os.path.join(root, 'outside_probe.txt')   # next to the sandbox, not granted
    rec['sandbox'] = sandbox
    sid, sidstr = container_sid()
    SIDSTR[0] = sidstr
    rec['container_sid'] = sidstr
    grant(sandbox, sidstr)
    data = []
    for d in a.data:
        src, dst = d.split('=', 1)
        shutil.copyfile(src, os.path.join(sandbox, dst))
        data.append({'src': src, 'dst': dst, 'sha256': sha256(src)})
    rec['data'] = data
    exe_copy = os.path.join(sandbox, os.path.basename(a.exe))
    probe_copy = os.path.join(root, 'probe', os.path.basename(a.probe))
    os.makedirs(os.path.dirname(probe_copy))
    shutil.copyfile(a.probe, probe_copy)
    grant(os.path.dirname(probe_copy), sidstr)
    shutil.copyfile(a.exe, exe_copy)
    rec['exe_copy_sha256'] = sha256(exe_copy)
    dirs = watched_dirs(root)
    skip = [root, os.path.abspath(a.out), os.path.join(os.environ.get('TEMP', ''), 'claude')]
    before = snapshot(dirs, skip)
    rec['files_before'] = sorted(os.listdir(sandbox))
    # No critical-error or GP-fault dialogs in started processes (inherited).
    k32.SetErrorMode(0x0001 | 0x0002)
    job = Job()
    probe_box = os.path.join(root, 'probe')
    rec['probe'] = run_probe(job, sid, probe_box, probe_copy, outside)
    if not rec['probe']['isolation_verified']:
        rec['result'] = 'NOT RUN: isolation not verified'
    elif a.probe_only:
        rec['result'] = 'probe only'
    else:
        with open(os.path.join(a.out, 'screens.log'), 'w', encoding='utf-8') as log:
            rec['run'] = run_program(job, sid, sandbox, exe_copy, script, log)
        rec['result'] = rec['run']['status']
    k32.CloseHandle(job.h)   # KILL_ON_JOB_CLOSE: nothing survives the driver
    time.sleep(1)
    after = snapshot(dirs, skip)
    rec['outside_changes'] = sorted(p for p in after if after[p] != before.get(p)) + \
        sorted('deleted: ' + p for p in before if p not in after)
    files = {}
    for f in sorted(os.listdir(sandbox)):
        p = os.path.join(sandbox, f)
        files[f] = {'size': os.path.getsize(p), 'sha256': sha256(p)}
    rec['sandbox_files'] = files
    rec['git_status'] = subprocess.run(['git', '-C', os.path.dirname(os.path.abspath(__file__)), 'status',
                                        '--porcelain', '--ignored'], capture_output=True, text=True).stdout
    rec['finished'] = datetime.datetime.now().isoformat(timespec='seconds')
    with open(os.path.join(a.out, 'run.json'), 'w', encoding='utf-8') as f:
        json.dump(rec, f, indent=2)
    print(json.dumps({k: rec.get(k) for k in ('result', 'sandbox')}, indent=1))
    return 0


def firewall_state():
    """Reads (no admin needed) the block rules for FW_EXE and the profiles."""
    ps = ("$r = Get-NetFirewallRule -DisplayName '%s*' -ErrorAction SilentlyContinue; "
          "$out = @(); foreach ($x in $r) { $f = $x | Get-NetFirewallApplicationFilter; "
          "$out += [pscustomobject]@{Name=$x.DisplayName; Enabled=[string]$x.Enabled; Action=[string]$x.Action; "
          "Direction=[string]$x.Direction; Program=$f.Program; Profile=[string]$x.Profile} }; "
          "$p = Get-NetFirewallProfile | ForEach-Object { [pscustomobject]@{Name=$_.Name; Enabled=[string]$_.Enabled} }; "
          "[pscustomobject]@{Rules=$out; Profiles=$p} | ConvertTo-Json -Depth 4") % FW_RULE
    r = subprocess.run(['powershell', '-NoProfile', '-Command', ps], capture_output=True, text=True)
    try:
        st = json.loads(r.stdout)
    except ValueError:
        return {'error': r.stdout + r.stderr}, False
    rules = st.get('Rules') or []
    if isinstance(rules, dict):
        rules = [rules]
    profiles = st.get('Profiles') or []
    if isinstance(profiles, dict):
        profiles = [profiles]
    dirs = set()
    for x in rules:
        if (x.get('Enabled') == 'True' and x.get('Action') == 'Block' and
                os.path.normcase(x.get('Program') or '') == os.path.normcase(FW_EXE) and x.get('Profile') == 'Any'):
            dirs.add(x.get('Direction'))
    ok = dirs == {'Inbound', 'Outbound'} and profiles and all(p.get('Enabled') == 'True' for p in profiles)
    return {'rules': rules, 'profiles': profiles}, bool(ok)


def logging_state():
    """Drop logging must be on in every profile, into FW_LOG (readable here)."""
    ps = ("Get-NetFirewallProfile | ForEach-Object { [pscustomobject]@{Name=$_.Name; "
          "LogBlocked=[string]$_.LogBlocked; LogFileName=$_.LogFileName} } | ConvertTo-Json")
    r = subprocess.run(['powershell', '-NoProfile', '-Command', ps], capture_output=True, text=True)
    try:
        prof = json.loads(r.stdout)
    except ValueError:
        return {'error': r.stdout + r.stderr}, False
    if isinstance(prof, dict):
        prof = [prof]
    ok = bool(prof) and all(x.get('LogBlocked') == 'True' and
                            os.path.normcase(os.path.expandvars(x.get('LogFileName') or '')) ==
                            os.path.normcase(FW_LOG) for x in prof)
    return prof, ok


def prepare_log_dir():
    """FW_LOG_DIR is ours; the firewall service writes the log there."""
    os.makedirs(FW_LOG_DIR, exist_ok=True)
    r = subprocess.run(['icacls', FW_LOG_DIR, '/grant', 'NT SERVICE\\mpssvc:(OI)(CI)M'],
                       capture_output=True, text=True)
    return r.returncode == 0, (r.stdout + r.stderr).strip()


def read_drop_log():
    """Parsed DROP lines of FW_LOG: list of dicts keyed by the #Fields header."""
    try:
        with open(FW_LOG_COPY, encoding='ascii', errors='replace') as f:
            lines = f.read().splitlines()
    except OSError as e:
        return None, str(e)
    fields = None
    out = []
    for l in lines:
        if l.startswith('#Fields:'):
            fields = l[len('#Fields:'):].split()
            continue
        if not l or l.startswith('#') or not fields:
            continue
        v = l.split()
        d = dict(zip(fields, v))
        if d.get('action') == 'DROP':
            d['_line'] = l
            out.append(d)
    return out, fields


def match_probe_in_log(checks, wait_s=0):
    """Finds the DROP entries of the probe's own TCP and UDP packets: same
    protocol, destination 192.0.2.1, the probe's random destination port and
    its recorded local source port, logged within 3 minutes of the send
    time (log times are local). Polls up to wait_s (the service writes the
    log with a delay)."""
    import datetime as dt
    want = {}
    for key, proto in (('tcp_sent', 'TCP'), ('udp_sent', 'UDP')):
        v = checks.get(key, '')
        parts = v.split('|')
        if len(parts) < 3:
            return {'error': 'probe did not record %s' % key}, False
        t = dt.datetime.strptime(parts[0], '%Y-%m-%dT%H:%M:%S.%fZ').replace(tzinfo=dt.timezone.utc).astimezone()
        dport = parts[1].split(':')[-1]
        sport = parts[2].split('=')[1]
        want[proto] = {'time_local': t.isoformat(timespec='seconds'), 'dst_port': dport, 'src_port': sport,
                       '_t': t.replace(tzinfo=None)}
    found = {}
    deadline = time.time() + wait_s
    fields = None
    while True:
        entries, fields = read_drop_log()
        for e in entries or []:
            proto = e.get('protocol')
            w = want.get(proto)
            if not w or proto in found:
                continue
            if e.get('dst-ip') != '192.0.2.1' or e.get('dst-port') != w['dst_port']:
                continue
            if w['src_port'] != '0' and e.get('src-port') != w['src_port']:
                continue
            try:
                lt = dt.datetime.strptime(e['date'] + ' ' + e['time'], '%Y-%m-%d %H:%M:%S')
            except (KeyError, ValueError):
                continue
            if abs((lt - w['_t']).total_seconds()) > 180:
                continue
            found[proto] = e['_line']
        if len(found) == 2 or time.time() >= deadline:
            break
        time.sleep(5)
    for w in want.values():
        w.pop('_t')
    res = {'expected': want, 'log_fields': fields, 'matched_lines': found}
    return res, set(found) == {'TCP', 'UDP'}


def udp_gate(path, state):
    if not path:
        return False, 'no --udp-verified file'
    try:
        v = json.load(open(path, encoding='utf-8'))
    except (OSError, ValueError) as e:
        return False, 'cannot read %s: %s' % (path, e)
    age = time.time() - v.get('verified_epoch', 0)
    same_rules = json.dumps(v.get('rules'), sort_keys=True) == json.dumps(state.get('rules'), sort_keys=True)
    ok = bool(v.get('ok')) and v.get('fw_exe') == FW_EXE and same_rules and 0 <= age <= 7200
    return ok, {'file': path, 'ok': v.get('ok'), 'age_s': int(age), 'same_rules': same_rules,
                'matched_lines': v.get('log', {}).get('matched_lines')}


def fresh_fw_dir():
    """Empties FW_DIR (it only ever holds this driver's files) and grants
    the restricting SID modify access, so the write-restricted process may
    write there and nowhere else."""
    os.makedirs(FW_DIR, exist_ok=True)
    for n in os.listdir(FW_DIR):
        p = os.path.join(FW_DIR, n)
        if os.path.isdir(p):
            shutil.rmtree(p)
        else:
            os.remove(p)
    subprocess.run(['icacls', FW_DIR, '/setintegritylevel', 'medium'], capture_output=True, text=True)
    r = subprocess.run(['icacls', FW_DIR, '/grant', '*%s:(OI)(CI)M' % RESTRICTING_SID], capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError('icacls grant: ' + r.stdout + r.stderr)


PROBE_NAME = 'tp_c11_outside_probe.txt'
REG_PROBE_KEY = r'Software\tp_c11_outside_probe'


def outside_paths():
    """Files the probe must fail to create: user folders (NTFS) plus the
    roots of D: and G: and G:\\Oma Drive (Google Drive, reports FAT32, so
    NTFS ACLs cannot be assumed there). Returns (paths, drive info)."""
    env = os.environ.get
    outs = [os.path.join(env('TEMP', ''), PROBE_NAME),
            os.path.join(env('USERPROFILE', ''), PROBE_NAME),
            os.path.join(env('USERPROFILE', ''), 'Documents', PROBE_NAME),
            os.path.join(env('LOCALAPPDATA', ''), PROBE_NAME)]
    drives = {}
    for root in ('D:\\', 'G:\\'):
        present = os.path.isdir(root)
        drives[root] = {'present': present}
        if present:
            outs.append(os.path.join(root, PROBE_NAME))
    if os.path.isdir('G:\\Oma Drive'):
        outs.append(os.path.join('G:\\Oma Drive', PROBE_NAME))
        drives['G:\\']['oma_drive'] = True
    for o in outs:
        if os.path.exists(o):
            raise RuntimeError('outside probe file already exists: ' + o)
    return outs, drives


def reg_probe_key_exists():
    import winreg
    try:
        winreg.CloseKey(winreg.OpenKey(winreg.HKEY_CURRENT_USER, REG_PROBE_KEY))
        return True
    except OSError:
        return False


def provenance(exe):
    pdb = os.path.splitext(exe)[0] + '.pdb'
    info = {'exe_sha256': sha256(exe), 'pe': hkdebug.pe_info(exe)}
    if os.path.exists(pdb):
        info['pdb'] = pdb
        info['pdb_sha256'] = sha256(pdb)
    return info


def main_firewall(a):
    os.makedirs(a.out, exist_ok=True)
    script = json.load(open(a.script, encoding='utf-8'))
    rec = {'started': datetime.datetime.now().isoformat(timespec='seconds'), 'mode': 'firewall',
           'started_epoch': time.time(), 'exe': os.path.abspath(a.exe), 'exe_sha256': sha256(a.exe),
           'provenance': provenance(a.exe), 'debug': a.debug, 'watch': a.watch,
           'probe_sha256': sha256(a.probe), 'driver_sha256': sha256(os.path.abspath(__file__)),
           'hkdebug_sha256': sha256(hkdebug.__file__),
           'script': script, 'script_sha256': sha256(a.script), 'fw_exe': FW_EXE,
           'firewall_commands': FIREWALL_COMMANDS}
    state, fw_ok = firewall_state()
    rec['firewall'] = state
    rec['firewall_ok'] = fw_ok
    lstate, log_ok = logging_state()
    rec['logging'] = lstate
    rec['logging_ok'] = log_ok
    rec['logging_commands'] = {'on': LOGGING_ON_COMMAND, 'restore': LOGGING_RESTORE_COMMAND}
    fw_ok = fw_ok and log_ok
    outs, drives = outside_paths()
    rec['outside_paths'] = outs
    rec['drives'] = drives
    outside = ';'.join(outs)
    if reg_probe_key_exists():
        raise RuntimeError('HKCU\\' + REG_PROBE_KEY + ' already exists')
    if not fw_ok:
        rec['result'] = ('NOT RUN: firewall block rules for %s missing/disabled, firewall off, '
                         'or drop logging not on into %s' % (FW_EXE, FW_LOG))
    else:
        fresh_fw_dir()
        dirs = watched_dirs(FW_DIR)
        skip = [FW_DIR, os.path.abspath(a.out), os.path.join(os.environ.get('TEMP', ''), 'claude')]
        before = snapshot(dirs, skip)
        k32.SetErrorMode(0x0001 | 0x0002)
        token = write_restricted_token()
        # 1. probe at the program's own path (same firewall rules), own job
        shutil.copyfile(a.probe, FW_EXE)
        import random
        ports = (random.randint(40000, 49999), random.randint(40000, 49999))
        rec['probe_ports'] = {'tcp': ports[0], 'udp': ports[1]}
        pjob = Job(active_limit=1)
        rec['probe'] = run_probe(pjob, None, FW_DIR, FW_EXE, outside, token, 'firewall', ports)
        rec['probe']['job'] = pjob.accounting()
        pjob.close()
        rec['probe']['registry_key_left'] = reg_probe_key_exists()
        # UDP drops cannot be read at run time (log ACL). Gate: an earlier
        # probe run's TCP+UDP drops matched in the log copy (verify_log.py),
        # same rules, at most 2 hours old. This run's own packets are matched
        # afterwards from a new log copy.
        gate_ok, gate = udp_gate(a.udp_verified, state)
        rec['probe']['udp_gate'] = gate
        rec['probe']['firewall_log_check'] = 'pending: match after the run from a new log copy (verify_log.py)'
        rec['probe']['isolation_verified'] = (rec['probe']['isolation_verified'] and gate_ok and
                                              not rec['probe']['registry_key_left'])
        if not rec['probe']['isolation_verified']:
            rec['result'] = 'NOT RUN: isolation not verified'
        elif a.probe_only:
            rec['result'] = 'probe only'
        else:
            # 2. fresh directory, the real program at the same path, own job
            fresh_fw_dir()
            data = []
            for d in a.data:
                src, dst = d.split('=', 1)
                shutil.copyfile(src, os.path.join(FW_DIR, dst))
                data.append({'src': src, 'dst': dst, 'sha256': sha256(src)})
            rec['data'] = data
            shutil.copyfile(a.exe, FW_EXE)
            rec['exe_copy_sha256'] = sha256(FW_EXE)
            rec['files_before'] = sorted(os.listdir(FW_DIR))
            job = Job(active_limit=1)
            rec['program_started_epoch'] = time.time()
            with open(os.path.join(a.out, 'screens.log'), 'w', encoding='utf-8') as log:
                rec['run'] = run_program(job, None, FW_DIR, FW_EXE, script, log, token, debug=a.debug,
                                         image_path=os.path.abspath(a.exe), watch=a.watch)
            rec['program_finished_epoch'] = time.time()
            rec['result'] = rec['run']['status']
            job.close()
        time.sleep(1)
        after = snapshot(dirs, skip)
        rec['outside_changes'] = sorted(p for p in after if after[p] != before.get(p)) + \
            sorted('deleted: ' + p for p in before if p not in after)
        rec['outside_changes_not_noise'] = [x for x in rec['outside_changes']
                                            if not hkdebug.outside_noise(x.replace('deleted: ', ''))]
        rec['outside_probe_file_exists'] = any(os.path.exists(o) for o in outs)
        rec['registry_key_exists_after'] = reg_probe_key_exists()
        files = {}
        for f in sorted(os.listdir(FW_DIR)):
            p = os.path.join(FW_DIR, f)
            if os.path.isfile(p):
                files[f] = {'size': os.path.getsize(p), 'sha256': sha256(p)}
        rec['sandbox'] = FW_DIR
        rec['sandbox_files'] = files
        # keep a copy of the sandbox for the report
        shutil.copytree(FW_DIR, os.path.join(a.out, 'sandbox_copy'), dirs_exist_ok=True)
        if rec.get('run'):
            # 3. bracketing probe AFTER the program: its packets must also be
            # in the log copy, which shows the copy covers the whole program
            # window (the log is appended in time order; verify_log.py
            # post_probe_ok / covers_run).
            fresh_fw_dir()
            shutil.copyfile(a.probe, FW_EXE)
            ports2 = (random.randint(40000, 49999), random.randint(40000, 49999))
            rec['post_probe_ports'] = {'tcp': ports2[0], 'udp': ports2[1]}
            pjob = Job(active_limit=1)
            rec['post_probe'] = run_probe(pjob, None, FW_DIR, FW_EXE, outside, token, 'firewall', ports2)
            rec['post_probe']['job'] = pjob.accounting()
            pjob.close()
            fresh_fw_dir()
        k32.CloseHandle(token)
    rec['git_status'] = subprocess.run(['git', '-C', os.path.dirname(os.path.abspath(__file__)), 'status',
                                        '--porcelain', '--ignored'], capture_output=True, text=True).stdout
    rec['finished'] = datetime.datetime.now().isoformat(timespec='seconds')
    rec['finished_epoch'] = time.time()
    rec['program_window_epoch'] = [rec.get('program_started_epoch'), rec.get('program_finished_epoch')]
    with open(os.path.join(a.out, 'run.json'), 'w', encoding='utf-8') as f:
        json.dump(rec, f, indent=2)
    print(json.dumps({k: rec.get(k) for k in ('result', 'sandbox', 'firewall_ok')}, indent=1))
    return 0


# ---------------------------------------------------------------- crash control

CRASH_DIR = os.path.join(os.environ.get('LOCALAPPDATA', ''), 'tulospalvelu-c11', 'crashctl')

# (name, args, job active limit, expectation function name)
CRASH_SCENARIOS = [
    ('gs64_no_overflow', ['gs', '64'], 1),
    ('gs104_overflow', ['gs', '104'], 1),
    ('fastfail', ['fastfail'], 1),
    ('access_violation', ['av'], 1),
    ('child_refused_by_job', ['child'], 1),
    ('child_killed_with_job', ['child'], None),
]


def fresh_dir(path):
    os.makedirs(path, exist_ok=True)
    for n in os.listdir(path):
        p = os.path.join(path, n)
        shutil.rmtree(p) if os.path.isdir(p) else os.remove(p)
    r = subprocess.run(['icacls', path, '/grant', '*%s:(OI)(CI)M' % RESTRICTING_SID], capture_output=True, text=True)
    if r.returncode != 0:
        raise RuntimeError('icacls grant: ' + r.stdout + r.stderr)


def _ok(cond, msg, problems):
    if not cond:
        problems.append(msg)


def check_crash_scenario(name, r, extra):
    """Expectations of one crash-control scenario; returns problems."""
    p = []
    d = r.get('debugger') or {}
    f = d.get('fatal') or {}
    ev = f.get('evidence') or {}
    wf = ev.get('watched_frame') or {}
    _ok(d.get('loop_finished'), 'debug loop did not finish', p)
    _ok(not d.get('error'), 'debugger error %r' % d.get('error'), p)
    _ok((d.get('symbols') or {}).get('pdb_ok'), 'PDB not matched', p)
    # the child of child_killed_with_job is still in the job until the job is closed
    want_active = 1 if name == 'child_killed_with_job' else 0
    _ok(r.get('job', {}).get('ActiveProcesses') == want_active, 'job active processes %r' % r.get('job'), p)
    if name == 'gs64_no_overflow':
        ret = [b for b in d.get('breakpoints', []) if b.get('kind') == 'return' and 'cookie_intact' in b]
        ent = [b for b in d.get('breakpoints', []) if b.get('kind') == 'entry' and 'return_address' in b]
        _ok(not f, 'unexpected fatal exception', p)
        _ok(r.get('exit_code') == 0, 'exit code %r' % r.get('exit_code_hex'), p)
        _ok(ent and (ent[0].get('return_sym') or '').startswith('main+'), 'entry breakpoint caller not main', p)
        _ok(ret and ret[0].get('cookie_intact') and ret[0].get('return_intact') and ret[0].get('saved_ebp_intact'),
            'return breakpoint: frame not intact', p)
    else:
        code = '0xC0000005' if name == 'access_violation' else '0xC0000409'
        _ok(f.get('code') == code, 'fatal code %r, expected %s' % (f.get('code'), code), p)
        _ok(d.get('terminated_by_debugger'), 'not terminated by the debugger', p)
        _ok(r.get('exit_code') == hkdebug.KILLED_BY_DEBUGGER, 'exit code %r' % r.get('exit_code_hex'), p)
        if code == '0xC0000409':
            _ok((f.get('params') or [None])[0] == '0x00000002', 'fast-fail code %r' % f.get('params'), p)
        else:
            _ok(f.get('first_chance') is False, 'AV fatal not second chance', p)
            _ok(any(e.get('code') == code and e.get('first_chance') for e in d.get('exceptions', [])),
                'first-chance AV not recorded', p)
        if name in ('gs104_overflow', 'child_refused_by_job', 'child_killed_with_job'):
            _ok(((ev.get('ret_at_esp') or {}).get('sym') or '').startswith('victim+'),
                'stack: return address at ESP not in victim', p)
            _ok((ev.get('context') or {}).get('Ebp') == wf.get('frame_ebp'), 'EBP is not the victim frame', p)
            _ok((f.get('sym') or '').startswith('__report_gsfailure+'), 'exception not in __report_gsfailure', p)
            _ok(wf.get('cookie_intact') is False and wf.get('return_intact') is False and
                wf.get('saved_ebp_intact') is False, 'watched frame not overwritten', p)
            _ok((wf.get('slots_hex') or '')[:80] == '00' * 40, 'overwritten bytes not the 40 zero bytes', p)
    if name == 'child_refused_by_job':
        _ok(extra.get('child_txt', '').startswith('failed') and r.get('job', {}).get('TotalProcesses') == 1,
            'child not refused: %r, job %r' % (extra.get('child_txt'), r.get('job')), p)
    if name == 'child_killed_with_job':
        txt = extra.get('child_txt', '')
        _ok(txt.startswith('created pid='), 'child not created: %r' % txt, p)
        _ok(extra.get('child_alive_after_job_close') is False, 'child survived the job', p)
        _ok(any('child of' in ' '.join(x.get('why', [])) for x in r['wer']['processes'].get('found', [])),
            'process watcher did not see the child', p)
    werp = [x for x in r['wer']['processes'].get('found', []) if 'name' in x.get('why', [])]
    _ok(not werp, 'WER process seen: %r' % werp, p)
    _ok(not r['wer'].get('events_matching'), 'WER/Application Error event mentions the program', p)
    return p


def crash_control(probe_exe, out_dir):
    """Runs every scenario of CRASH_SCENARIOS under the debugger with the
    write-restricted token, each in its own job. Returns the record."""
    os.makedirs(out_dir, exist_ok=True)
    exe = os.path.join(CRASH_DIR, 'crash_probe.exe')
    res = {'probe_sha256': sha256(probe_exe), 'crash_dir': CRASH_DIR, 'scenarios': {},
           'wersvc_before': hkdebug.wersvc_state()}
    since = hkdebug.utcnow()
    dirs = watched_dirs(CRASH_DIR)
    skip = [CRASH_DIR, os.path.abspath(out_dir), os.path.join(os.environ.get('TEMP', ''), 'claude')]
    before = snapshot(dirs, skip)
    k32.SetErrorMode(0x0001 | 0x0002)
    token = write_restricted_token()
    all_ok = True
    for name, args, limit in CRASH_SCENARIOS:
        fresh_dir(CRASH_DIR)
        shutil.copyfile(probe_exe, exe)
        shutil.copyfile(os.path.splitext(probe_exe)[0] + '.pdb', os.path.join(CRASH_DIR, 'crash_probe.pdb'))
        job = Job(active_limit=limit)
        script = {'args': args, 'steps': [], 'total_timeout': 20}
        with open(os.path.join(out_dir, 'crash_%s.log' % name), 'w', encoding='utf-8') as log:
            r = run_program(job, None, CRASH_DIR, exe, script, log, token, debug=True, image_path=exe,
                            watch='victim')
        extra = {}
        cp = os.path.join(CRASH_DIR, 'child.txt')
        if os.path.exists(cp):
            extra['child_txt'] = open(cp, encoding='ascii').read().strip()
        job.close()
        time.sleep(0.5)
        if extra.get('child_txt', '').startswith('created pid='):
            cpid = int(extra['child_txt'].split('=')[1])
            extra['child_pid'] = cpid
            extra['child_alive_after_job_close'] = hkdebug.pid_alive(cpid)
        r['extra'] = extra
        r['problems'] = check_crash_scenario(name, r, extra)
        r['ok'] = not r['problems']
        all_ok = all_ok and r['ok']
        res['scenarios'][name] = r
    k32.CloseHandle(token)
    time.sleep(3)
    after = snapshot(dirs, skip)
    res['outside_changes'] = sorted(p for p in after if after[p] != before.get(p)) + \
        sorted('deleted: ' + p for p in before if p not in after)
    ev_all, ev_hit = hkdebug.wer_events(since, ['crash_probe'])
    res['wer_events_since_start'] = ev_all
    res['wer_events_matching'] = ev_hit
    res['wersvc_after'] = hkdebug.wersvc_state()
    wer_dirs_changed = [p for p in res['outside_changes'] if '\\WER\\' in p.upper() or 'CRASHDUMPS' in p.upper()]
    res['wer_dirs_changed'] = wer_dirs_changed
    res['ok'] = all_ok and not ev_hit and not wer_dirs_changed
    return res


def crash_control_main(a):
    if not a.crash_probe:
        raise SystemExit('--crash-probe is required')
    res = crash_control(a.crash_probe, a.out)
    with open(os.path.join(a.out, 'crash_control.json'), 'w', encoding='utf-8') as f:
        json.dump(res, f, indent=2)
    print(json.dumps({'ok': res['ok'], 'scenarios': {k: v['problems'] for k, v in res['scenarios'].items()},
                      'wer_events_matching': len(res['wer_events_matching']),
                      'outside_changes': res['outside_changes']}, indent=1))
    return 0 if res['ok'] else 1


# ---------------------------------------------------------------- self-test

HKMAALI_EXPECT = hkdebug.HKMAALI_EXPECT


def self_test_main(a):
    os.makedirs(a.out, exist_ok=True)
    res = {'started': datetime.datetime.now().isoformat(timespec='seconds'),
           'driver_sha256': sha256(os.path.abspath(__file__)), 'hkdebug_sha256': sha256(hkdebug.__file__),
           'isolation_probe_sha256': sha256(a.probe), 'crash_probe_sha256': sha256(a.crash_probe),
           'tests': {}}
    T = res['tests']

    # 1. endpoint enumeration finds this process's own sockets (positive control)
    socks = []
    try:
        s4 = socket.socket()
        s4.bind(('127.0.0.1', 0))
        s4.listen(1)
        u4 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        u4.bind(('127.0.0.1', 0))
        socks += [s4, u4]
        want = ['TCP 127.0.0.1:%d' % s4.getsockname()[1], 'UDP 127.0.0.1:%d' % u4.getsockname()[1]]
        try:
            s6 = socket.socket(socket.AF_INET6)
            s6.bind(('::1', 0))
            s6.listen(1)
            socks.append(s6)
            want.append('TCP6 [0:0:0:0:0:0:0:1]:%d' % s6.getsockname()[1])
        except OSError:
            pass
        got = hkdebug.endpoints([os.getpid()])
        miss = [w for w in want if not any(g.startswith(w) for g in got)]
        T['endpoints_positive_control'] = {'ok': not miss, 'wanted': want, 'missing': miss, 'got': got}
    finally:
        for s in socks:
            s.close()

    # 2. process watcher catches a short-lived process (positive control)
    fresh_dir(CRASH_DIR)
    cexe = os.path.join(CRASH_DIR, 'crash_probe.exe')
    shutil.copyfile(a.crash_probe, cexe)
    w = hkdebug.ProcWatcher(names=('crash_probe.exe',))
    w.start()
    job = Job(active_limit=1)
    con = Console()
    pi = launch(job, None, cexe, ['sleep', '300'], CRASH_DIR, con)
    spid = pi.dwProcessId
    k32.WaitForSingleObject(pi.hProcess, 10000)
    close_pi(pi)
    con.close()
    time.sleep(0.3)
    wr = w.stop()
    job.close()
    T['process_watcher_positive_control'] = {'ok': any(x['pid'] == spid for x in wr['found']),
                                             'process_lifetime_ms': 300, 'watcher': wr}

    # 3. probe timeout: a probe that never finishes is killed and not verified
    job = Job(active_limit=1)
    pr = run_probe(job, None, CRASH_DIR, cexe, '', None, 'firewall', None, timeout_s=5,
                   args_override=['sleep', '60000'])
    job.close()
    T['probe_timeout'] = {'ok': pr.get('timed_out') is True and pr.get('isolation_verified') is False and
                          not hkdebug.pid_alive(pr['pid']), 'probe': pr}

    # 4. symbols of the shipped program at its preferred base
    st = {}
    try:
        pe = hkdebug.pe_info(a.exe)
        sym = hkdebug.Symbols(a.exe, HKMAALI_EXPECT['image_base'])
        st = {'pe': pe, 'module': sym.module, 'pdb_ok': sym.pdb_ok(pe['rsds']),
              'addfrec': hkdebug.hexs(sym.addr('kilptietue::addfrec')),
              'ret_site_names': [sym.name(x) for x in HKMAALI_EXPECT['ret_sites']],
              'cookie': hkdebug.hexs(sym.addr('__security_cookie', '___security_cookie')),
              'addtall': hkdebug.hexs(sym.addr('kilptietue::addtall'))}
        sym.close()
        st['ok'] = (pe['rsds'] and pe['rsds']['guid'] == HKMAALI_EXPECT['rsds_guid'] and st['pdb_ok'] and
                    st['addfrec'] == hkdebug.hexs(HKMAALI_EXPECT['addfrec']) and
                    st['cookie'] == hkdebug.hexs(HKMAALI_EXPECT['cookie']) and
                    all((n or '').startswith('kilptietue::addfrec+') for n in st['ret_site_names']))
    except Exception as e:   # noqa: BLE001
        st = {'ok': False, 'error': repr(e)}
    T['hkmaali_symbols'] = st

    # 5. isolation probe at the firewall path (rules must exist; drop log not needed)
    state, fw_ok = firewall_state()
    if not fw_ok:
        T['isolation_probe_direct'] = {'ok': False, 'error': 'firewall rules not in place', 'state': state}
    else:
        outs, drives = outside_paths()
        fresh_fw_dir()
        shutil.copyfile(a.probe, FW_EXE)
        token = write_restricted_token()
        job = Job(active_limit=1)
        import random
        ports = (random.randint(40000, 49999), random.randint(40000, 49999))
        pr = run_probe(job, None, FW_DIR, FW_EXE, ';'.join(outs), token, 'firewall', ports)
        job.close()
        k32.CloseHandle(token)
        pr['registry_key_left'] = reg_probe_key_exists()
        pr['outside_file_left'] = any(os.path.exists(o) for o in outs)
        T['isolation_probe_direct'] = {'ok': pr.get('isolation_verified') and not pr['registry_key_left'] and
                                       not pr['outside_file_left'], 'drives': drives, 'outside_paths': outs,
                                       'probe': pr, 'note': 'UDP drop not verified here (needs the drop log)'}
        fresh_fw_dir()

    # 6. debugger mode on deliberate crashes
    cc = crash_control(a.crash_probe, a.out)
    T['crash_control'] = {'ok': cc['ok'], 'problems': {k: v['problems'] for k, v in cc['scenarios'].items()},
                          'wer_events_matching': cc['wer_events_matching'], 'wer_dirs_changed': cc['wer_dirs_changed'],
                          'outside_changes': cc['outside_changes']}
    with open(os.path.join(a.out, 'crash_control.json'), 'w', encoding='utf-8') as f:
        json.dump(cc, f, indent=2)
    res['ok'] = all(t.get('ok') for t in T.values())
    res['finished'] = datetime.datetime.now().isoformat(timespec='seconds')
    with open(os.path.join(a.out, 'self_test.json'), 'w', encoding='utf-8') as f:
        json.dump(res, f, indent=2)
    print(json.dumps({k: bool(v.get('ok')) for k, v in T.items()}, indent=1))
    print('self-test ok' if res['ok'] else 'SELF-TEST FAILED')
    return 0 if res['ok'] else 1


if __name__ == '__main__':
    sys.exit(main())
