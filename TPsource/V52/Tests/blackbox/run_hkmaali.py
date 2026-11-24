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

Usage:
  python -I run_hkmaali.py --exe PATH --probe PATH --script SCRIPT.json --out DIR
                           [--data SRC=DST ...] [--mode appcontainer|firewall]
SCRIPT.json: {"args": ["BOOT"], "steps": [{"wait": "text", "key": "\\r"}, ...],
              "expect_exit": true, "total_timeout": 60, "step_timeout": 10}
A step with "key": null only waits for the text. After the last step the
driver waits for the process to exit.
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


class Job:
    def __init__(self):
        k32.CreateJobObjectW.restype = wintypes.HANDLE
        self.h = k32.CreateJobObjectW(None, None)
        check(self.h, 'CreateJobObject')
        info = JOBOBJECT_EXTENDED_LIMIT_INFORMATION()
        info.BasicLimitInformation.LimitFlags = JOB_KILL_ON_CLOSE | JOB_DIE_ON_UNHANDLED_EXCEPTION
        check(k32.SetInformationJobObject(self.h, 9, ctypes.byref(info), ctypes.sizeof(info)),
              'SetInformationJobObject')

    def terminate(self):
        k32.TerminateJobObject(self.h, 0xDEAD0001)   # recognisable: killed by the driver


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


def launch(job, sid, exe, args, cwd, con, token=None):
    """Starts exe suspended, either in the AppContainer (sid) or with the
    given token (low integrity), with the driver's console as its standard
    input/output/error (and no other inherited handle); assigns it to the
    job; resumes. Returns PROCESS_INFORMATION."""
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
    if token is None:
        k32.CreateProcessW.argtypes = [wintypes.LPCWSTR, wintypes.LPWSTR, ctypes.c_void_p, ctypes.c_void_p,
                                       wintypes.BOOL, wintypes.DWORD, ctypes.c_void_p, wintypes.LPCWSTR,
                                       ctypes.c_void_p, ctypes.POINTER(PROCESS_INFORMATION)]
        check(k32.CreateProcessW(exe, cmd, None, None, True,
                                 EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED,
                                 None, cwd, ctypes.byref(si), ctypes.byref(pi)), 'CreateProcess')
    else:
        adv.CreateProcessAsUserW.argtypes = [wintypes.HANDLE, wintypes.LPCWSTR, wintypes.LPWSTR, ctypes.c_void_p,
                                             ctypes.c_void_p, wintypes.BOOL, wintypes.DWORD, ctypes.c_void_p,
                                             wintypes.LPCWSTR, ctypes.c_void_p, ctypes.POINTER(PROCESS_INFORMATION)]
        check(adv.CreateProcessAsUserW(token, exe, cmd, None, None, True,
                                       EXTENDED_STARTUPINFO_PRESENT | CREATE_SUSPENDED,
                                       None, cwd, ctypes.byref(si), ctypes.byref(pi)), 'CreateProcessAsUser')
    if not k32.AssignProcessToJobObject(job.h, pi.hProcess):
        k32.TerminateProcess(pi.hProcess, 1)
        raise RuntimeError('AssignProcessToJobObject failed (%d)' % ctypes.get_last_error())
    if k32.ResumeThread(pi.hThread) == 0xFFFFFFFF:
        k32.TerminateProcess(pi.hProcess, 1)
        raise RuntimeError('ResumeThread failed (%d)' % ctypes.get_last_error())
    k32.DeleteProcThreadAttributeList(attr)
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

    def key(self, ch):
        vk = {'\r': 0x0D, '\x1b': 0x1B}.get(ch, user32.VkKeyScanW(ord(ch)) & 0xFF)
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

def run_probe(job, sid, sandbox, probe, outside, token=None, mode='appcontainer', ports=None):
    lst = Listener()
    con = Console()
    args = [outside, str(lst.port)] + ([str(ports[0]), str(ports[1])] if ports else [])
    pi = launch(job, sid, probe, args, sandbox, con, token)
    t0 = time.time()
    while 'PROBE READY' not in con.screen() and time.time() - t0 < 10:
        time.sleep(0.1)
    con.key('X')
    k32.WaitForSingleObject(pi.hProcess, 30000)
    code = wintypes.DWORD()
    k32.GetExitCodeProcess(pi.hProcess, ctypes.byref(code))
    con.close()
    lst.close()
    res = {'exit_code': code.value, 'listener_accepts': lst.accepts, 'checks': {}}
    try:
        with open(os.path.join(sandbox, 'probe_result.txt'), encoding='ascii') as f:
            for line in f:
                if '=' in line:
                    k, v = line.strip().split('=', 1)
                    res['checks'][k] = v
    except OSError as e:
        res['error'] = str(e)
    if mode == 'firewall':
        # Windows Firewall does not filter loopback: informational only.
        required = ('cwd', 'console', 'outside', 'external')
        loop_ok = True
    else:
        required = ('cwd', 'console', 'outside', 'loopback', 'external')
        loop_ok = lst.accepts == 0
    ok = (res['checks'].get('done') == '1' and loop_ok and
          all(res['checks'].get(k, '').startswith('isolated_or_ok') for k in required) and
          not any(os.path.exists(o) for o in outside.split(';')))
    res['required_checks'] = list(required)
    res['isolation_verified'] = ok
    return res


def run_program(job, sid, sandbox, exe, script, log, token=None):
    con = Console()
    pi = launch(job, sid, exe, script.get('args', []), sandbox, con, token)
    pid = pi.dwProcessId
    total = script.get('total_timeout', 60)
    t0 = time.time()
    steps = []
    net = []
    status = 'ok'
    windows = []
    for st in script.get('steps', []):
        deadline = time.time() + script.get('step_timeout', 10)
        seen = False
        waits = st['wait'] if isinstance(st['wait'], list) else [st['wait']]
        while time.time() < deadline and time.time() - t0 < total:
            scr = con.screen()
            w = windows_of(pid)
            if w:
                windows.append(w)
                break
            if all(x in scr for x in waits):
                seen = True
                break
            if k32.WaitForSingleObject(pi.hProcess, 0) == 0:
                break
            s = sockets_of(pid)
            if s:
                net.append(s)
            time.sleep(0.2)
        steps.append({'wait': st['wait'], 'seen': seen, 'key': st.get('key'),
                      't': round(time.time() - t0, 2)})
        log.write('--- screen after waiting for %r (seen=%s) ---\n%s\n' % (st['wait'], seen, con.screen()))
        if windows:
            status = 'window opened by the program: %r' % windows[-1]
            break
        if not seen:
            status = 'prompt not seen: %r' % st['wait']
            break
        if st.get('key') is not None:
            time.sleep(0.3)   # the program flushes the key buffer before reading
            for ch in st['key']:
                con.key(ch)
    last = con.screen()
    exited = False
    if status == 'ok':
        end = time.time() + max(1.0, total - (time.time() - t0))
        while time.time() < end:
            if k32.WaitForSingleObject(pi.hProcess, 200) == 0:
                exited = True
                break
            w = windows_of(pid)
            if w:
                windows.append(w)
                status = 'window opened by the program: %r' % w
                break
    exited = exited or k32.WaitForSingleObject(pi.hProcess, 0) == 0
    if not exited:
        status = status if status != 'ok' else 'did not exit in time'
        log.write('--- final screen (killing) ---\n%s\n' % con.screen())
        job.terminate()
        k32.WaitForSingleObject(pi.hProcess, 5000)
    code = wintypes.DWORD()
    k32.GetExitCodeProcess(pi.hProcess, ctypes.byref(code))
    log.write('--- last screen ---\n%s\n' % last)
    con.close()
    return {'status': status, 'exited_by_itself': exited, 'exit_code': code.value, 'windows': windows,
            'exit_code_hex': '0x%08X' % code.value, 'steps': steps, 'sockets_seen': net,
            'duration_s': round(time.time() - t0, 2)}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--exe', required=True)
    ap.add_argument('--probe', required=True)
    ap.add_argument('--script', required=True)
    ap.add_argument('--out', required=True)
    ap.add_argument('--data', action='append', default=[], help='SRC=DST copied into the sandbox')
    ap.add_argument('--probe-only', action='store_true')
    ap.add_argument('--mode', choices=('appcontainer', 'firewall'), default='appcontainer')
    ap.add_argument('--udp-verified', help='verification JSON written by verify_log.py (firewall mode)')
    a = ap.parse_args()
    if a.mode == 'firewall':
        return main_firewall(a)
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


def main_firewall(a):
    os.makedirs(a.out, exist_ok=True)
    script = json.load(open(a.script, encoding='utf-8'))
    rec = {'started': datetime.datetime.now().isoformat(timespec='seconds'), 'mode': 'firewall',
           'exe': os.path.abspath(a.exe), 'exe_sha256': sha256(a.exe),
           'probe_sha256': sha256(a.probe), 'driver_sha256': sha256(os.path.abspath(__file__)),
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
    outs = [os.path.join(os.environ.get('TEMP', ''), 'tp_c11_outside_probe.txt'),
            os.path.join(os.environ.get('USERPROFILE', ''), 'tp_c11_outside_probe.txt'),
            os.path.join(os.environ.get('USERPROFILE', ''), 'Documents', 'tp_c11_outside_probe.txt'),
            os.path.join(os.environ.get('LOCALAPPDATA', ''), 'tp_c11_outside_probe.txt')]
    for o in outs:
        if os.path.exists(o):
            raise RuntimeError('outside probe file already exists: ' + o)
    outside = ';'.join(outs)
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
        job = Job()
        # 1. probe at the program's own path (same firewall rules)
        shutil.copyfile(a.probe, FW_EXE)
        import random
        ports = (random.randint(40000, 49999), random.randint(40000, 49999))
        rec['probe_ports'] = {'tcp': ports[0], 'udp': ports[1]}
        rec['probe'] = run_probe(job, None, FW_DIR, FW_EXE, outside, token, 'firewall', ports)
        # UDP drops cannot be read at run time (log ACL). Gate: an earlier
        # probe run's TCP+UDP drops matched in the log copy (verify_log.py),
        # same rules, at most 2 hours old. This run's own packets are matched
        # afterwards from a new log copy.
        gate_ok, gate = udp_gate(a.udp_verified, state)
        rec['probe']['udp_gate'] = gate
        rec['probe']['firewall_log_check'] = 'pending: match after the run from a new log copy (verify_log.py)'
        rec['probe']['isolation_verified'] = rec['probe']['isolation_verified'] and gate_ok
        if not rec['probe']['isolation_verified']:
            rec['result'] = 'NOT RUN: isolation not verified'
        elif a.probe_only:
            rec['result'] = 'probe only'
        else:
            # 2. fresh directory, the real program at the same path
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
            with open(os.path.join(a.out, 'screens.log'), 'w', encoding='utf-8') as log:
                rec['run'] = run_program(job, None, FW_DIR, FW_EXE, script, log, token)
            rec['result'] = rec['run']['status']
        k32.CloseHandle(job.h)   # KILL_ON_JOB_CLOSE
        time.sleep(1)
        after = snapshot(dirs, skip)
        rec['outside_changes'] = sorted(p for p in after if after[p] != before.get(p)) + \
            sorted('deleted: ' + p for p in before if p not in after)
        rec['outside_probe_file_exists'] = any(os.path.exists(o) for o in outs)
        files = {}
        for f in sorted(os.listdir(FW_DIR)):
            p = os.path.join(FW_DIR, f)
            if os.path.isfile(p):
                files[f] = {'size': os.path.getsize(p), 'sha256': sha256(p)}
        rec['sandbox'] = FW_DIR
        rec['sandbox_files'] = files
        # keep a copy of the sandbox for the report
        shutil.copytree(FW_DIR, os.path.join(a.out, 'sandbox_copy'), dirs_exist_ok=True)
    rec['git_status'] = subprocess.run(['git', '-C', os.path.dirname(os.path.abspath(__file__)), 'status',
                                        '--porcelain', '--ignored'], capture_output=True, text=True).stdout
    rec['finished'] = datetime.datetime.now().isoformat(timespec='seconds')
    with open(os.path.join(a.out, 'run.json'), 'w', encoding='utf-8') as f:
        json.dump(rec, f, indent=2)
    print(json.dumps({k: rec.get(k) for k in ('result', 'sandbox', 'firewall_ok')}, indent=1))
    return 0


if __name__ == '__main__':
    sys.exit(main())
