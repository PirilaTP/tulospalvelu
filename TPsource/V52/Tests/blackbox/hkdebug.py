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

"""Debugger and monitoring helpers of run_hkmaali.py (C-11 stage 4,
doc/arkkitehtuuri/testaus.md section 19). Windows only, standard library.
The driver is 64-bit Python; the programs under test are x86 (WOW64), so
thread contexts are read with Wow64GetThreadContext.

  Debugger   - runs a launch function in its own thread (the creating
               thread must own the debug loop), handles every debug event,
               records exceptions, sets one-shot int3 breakpoints at
               symbol addresses, and terminates the process on a fatal
               exception (0xC0000409 fast fail, a second-chance exception or
               a non-continuable one) BEFORE continuing it, so Windows Error
               Reporting is never reached. Handles from debug events are
               closed as the documentation requires.
  Symbols    - DbgHelp in this process with the image at its run-time base
               (ASLR) and SYMOPT_EXACT_SYMBOLS; checks the PDB matches.
  endpoints  - TCP/UDP, IPv4/IPv6 endpoints of a pid (iphlpapi).
  ProcWatcher - polls the process list: WER processes (WerFault,
               WerFaultSecure, wermgr) and children of watched pids.
  wer_events / wersvc_state - Application log events 1000/1001/1005 and
               the WerSvc service state.
  evaluate_h041 - the acceptance criteria of section 18.4 as code.
"""

import ctypes
import datetime
import os
import struct
import subprocess
import threading
import time
from ctypes import wintypes

k32 = ctypes.WinDLL('kernel32', use_last_error=True)
iph = ctypes.WinDLL('iphlpapi', use_last_error=True)

DEBUG_ONLY_THIS_PROCESS = 0x00000002
DBG_CONTINUE = 0x00010002
DBG_EXCEPTION_NOT_HANDLED = 0x80010001
STATUS_BREAKPOINT = 0x80000003
STATUS_WX86_BREAKPOINT = 0x4000001F
STATUS_STACK_BUFFER_OVERRUN = 0xC0000409
FATAL_CODES = (STATUS_STACK_BUFFER_OVERRUN, 0xC0000602, 0xC0000374)
KILLED_BY_DEBUGGER = 0xDEAD0409
KILLED_ON_STOP = 0xDEAD0002
WER_NAMES = ('werfault.exe', 'werfaultsecure.exe', 'wermgr.exe')

k32.GetCurrentProcess.restype = wintypes.HANDLE
k32.OpenProcess.restype = wintypes.HANDLE
k32.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
k32.CloseHandle.argtypes = [wintypes.HANDLE]
k32.TerminateProcess.argtypes = [wintypes.HANDLE, wintypes.UINT]
k32.WaitForSingleObject.argtypes = [wintypes.HANDLE, wintypes.DWORD]
k32.GetExitCodeProcess.argtypes = [wintypes.HANDLE, ctypes.POINTER(wintypes.DWORD)]
k32.ReadProcessMemory.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t,
                                  ctypes.POINTER(ctypes.c_size_t)]
k32.WriteProcessMemory.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t,
                                   ctypes.POINTER(ctypes.c_size_t)]
k32.FlushInstructionCache.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_size_t]
k32.ContinueDebugEvent.argtypes = [wintypes.DWORD, wintypes.DWORD, wintypes.DWORD]


# ---------------------------------------------------------------- structures

class EXCEPTION_RECORD(ctypes.Structure):
    _fields_ = [('ExceptionCode', wintypes.DWORD), ('ExceptionFlags', wintypes.DWORD),
                ('ExceptionRecord', ctypes.c_void_p), ('ExceptionAddress', ctypes.c_void_p),
                ('NumberParameters', wintypes.DWORD), ('ExceptionInformation', ctypes.c_size_t * 15)]


class EXCEPTION_DEBUG_INFO(ctypes.Structure):
    _fields_ = [('ExceptionRecord', EXCEPTION_RECORD), ('dwFirstChance', wintypes.DWORD)]


class CREATE_PROCESS_DEBUG_INFO(ctypes.Structure):
    _fields_ = [('hFile', wintypes.HANDLE), ('hProcess', wintypes.HANDLE), ('hThread', wintypes.HANDLE),
                ('lpBaseOfImage', ctypes.c_void_p), ('dwDebugInfoFileOffset', wintypes.DWORD),
                ('nDebugInfoSize', wintypes.DWORD), ('lpThreadLocalBase', ctypes.c_void_p),
                ('lpStartAddress', ctypes.c_void_p), ('lpImageName', ctypes.c_void_p), ('fUnicode', wintypes.WORD)]


class CREATE_THREAD_DEBUG_INFO(ctypes.Structure):
    _fields_ = [('hThread', wintypes.HANDLE), ('lpThreadLocalBase', ctypes.c_void_p),
                ('lpStartAddress', ctypes.c_void_p)]


class EXIT_DEBUG_INFO(ctypes.Structure):
    _fields_ = [('dwExitCode', wintypes.DWORD)]


class LOAD_DLL_DEBUG_INFO(ctypes.Structure):
    _fields_ = [('hFile', wintypes.HANDLE), ('lpBaseOfDll', ctypes.c_void_p),
                ('dwDebugInfoFileOffset', wintypes.DWORD), ('nDebugInfoSize', wintypes.DWORD),
                ('lpImageName', ctypes.c_void_p), ('fUnicode', wintypes.WORD)]


class DEBUG_EVENT_U(ctypes.Union):
    _fields_ = [('Exception', EXCEPTION_DEBUG_INFO), ('CreateThread', CREATE_THREAD_DEBUG_INFO),
                ('CreateProcessInfo', CREATE_PROCESS_DEBUG_INFO), ('Exit', EXIT_DEBUG_INFO),
                ('LoadDll', LOAD_DLL_DEBUG_INFO), ('_raw', ctypes.c_byte * 256)]


class DEBUG_EVENT(ctypes.Structure):
    _fields_ = [('dwDebugEventCode', wintypes.DWORD), ('dwProcessId', wintypes.DWORD),
                ('dwThreadId', wintypes.DWORD), ('u', DEBUG_EVENT_U)]


class WOW64_CONTEXT(ctypes.Structure):
    _fields_ = [('ContextFlags', wintypes.DWORD), ('Dr', wintypes.DWORD * 6), ('FloatSave', ctypes.c_byte * 112),
                ('SegGs', wintypes.DWORD), ('SegFs', wintypes.DWORD), ('SegEs', wintypes.DWORD),
                ('SegDs', wintypes.DWORD), ('Edi', wintypes.DWORD), ('Esi', wintypes.DWORD),
                ('Ebx', wintypes.DWORD), ('Edx', wintypes.DWORD), ('Ecx', wintypes.DWORD),
                ('Eax', wintypes.DWORD), ('Ebp', wintypes.DWORD), ('Eip', wintypes.DWORD),
                ('SegCs', wintypes.DWORD), ('EFlags', wintypes.DWORD), ('Esp', wintypes.DWORD),
                ('SegSs', wintypes.DWORD), ('ExtendedRegisters', ctypes.c_byte * 512)]


WOW64_CONTEXT_FULL = 0x00010007
assert ctypes.sizeof(WOW64_CONTEXT) == 716

k32.WaitForDebugEvent.argtypes = [ctypes.POINTER(DEBUG_EVENT), wintypes.DWORD]
k32.Wow64GetThreadContext.argtypes = [wintypes.HANDLE, ctypes.POINTER(WOW64_CONTEXT)]
k32.Wow64SetThreadContext.argtypes = [wintypes.HANDLE, ctypes.POINTER(WOW64_CONTEXT)]


def hexs(v):
    return None if v is None else '0x%08X' % (v & 0xFFFFFFFF)


# ---------------------------------------------------------------- memory

def read_mem(hproc, addr, n):
    buf = ctypes.create_string_buffer(n)
    got = ctypes.c_size_t()
    if not k32.ReadProcessMemory(hproc, ctypes.c_void_p(addr), buf, n, ctypes.byref(got)) or got.value != n:
        return None
    return buf.raw


def read_u32(hproc, addr):
    b = read_mem(hproc, addr, 4)
    return None if b is None else struct.unpack('<I', b)[0]


def write_mem(hproc, addr, data):
    got = ctypes.c_size_t()
    ok = k32.WriteProcessMemory(hproc, ctypes.c_void_p(addr), data, len(data), ctypes.byref(got))
    k32.FlushInstructionCache(hproc, ctypes.c_void_p(addr), len(data))
    return bool(ok) and got.value == len(data)


def get_ctx(hthread):
    c = WOW64_CONTEXT()
    c.ContextFlags = WOW64_CONTEXT_FULL
    if not k32.Wow64GetThreadContext(hthread, ctypes.byref(c)):
        return None
    return c


def ctx_dict(c):
    if c is None:
        return None
    return {r: hexs(getattr(c, r)) for r in ('Eip', 'Esp', 'Ebp', 'Eax', 'Ebx', 'Ecx', 'Edx', 'Esi', 'Edi',
                                             'EFlags')}


# ---------------------------------------------------------------- PE / symbols

def pe_info(path):
    """Preferred image base, SizeOfImage, machine, DllCharacteristics and the
    CodeView RSDS GUID/age/PDB path of a PE file (no DbgHelp)."""
    with open(path, 'rb') as f:
        d = f.read()
    pe = struct.unpack_from('<I', d, 0x3C)[0]
    machine = struct.unpack_from('<H', d, pe + 4)[0]
    nsec = struct.unpack_from('<H', d, pe + 6)[0]
    optsize = struct.unpack_from('<H', d, pe + 20)[0]
    opt = pe + 24
    magic = struct.unpack_from('<H', d, opt)[0]
    if magic != 0x10B:
        raise ValueError('not a PE32 image')
    image_base = struct.unpack_from('<I', d, opt + 28)[0]
    size_of_image = struct.unpack_from('<I', d, opt + 56)[0]
    dllchar = struct.unpack_from('<H', d, opt + 70)[0]
    dbg_rva, dbg_size = struct.unpack_from('<II', d, opt + 96 + 6 * 8)
    secs = []
    for i in range(nsec):
        s = opt + optsize + i * 40
        va, vsize, raw_size, raw_ptr = (struct.unpack_from('<I', d, s + 12)[0], struct.unpack_from('<I', d, s + 8)[0],
                                        struct.unpack_from('<I', d, s + 16)[0], struct.unpack_from('<I', d, s + 20)[0])
        secs.append((va, max(vsize, raw_size), raw_ptr))

    def off(rva):
        for va, sz, rp in secs:
            if va <= rva < va + sz:
                return rp + rva - va
        return None
    rsds = None
    o = off(dbg_rva) if dbg_rva else None
    if o is not None:
        for i in range(dbg_size // 28):
            typ, size, rva, ptr = struct.unpack_from('<IIII', d, o + i * 28 + 12)
            if typ == 2 and d[ptr:ptr + 4] == b'RSDS':
                g = d[ptr + 4:ptr + 20]
                a = struct.unpack_from('<I', d, ptr + 20)[0]
                name = d[ptr + 24:ptr + size].split(b'\0')[0].decode('ascii', 'replace')
                guid = '{%08X-%04X-%04X-%s-%s}' % (struct.unpack_from('<I', g, 0)[0], struct.unpack_from('<H', g, 4)[0],
                                                   struct.unpack_from('<H', g, 6)[0], g[8:10].hex().upper(),
                                                   g[10:16].hex().upper())
                rsds = {'guid': guid, 'age': a, 'pdb_path': name}
    return {'image_base': image_base, 'size_of_image': size_of_image, 'machine': hexs(machine),
            'dll_characteristics': hexs(dllchar), 'rsds': rsds}


class GUID(ctypes.Structure):
    _fields_ = [('Data1', wintypes.DWORD), ('Data2', wintypes.WORD), ('Data3', wintypes.WORD),
                ('Data4', ctypes.c_ubyte * 8)]

    def text(self):
        return '{%08X-%04X-%04X-%s-%s}' % (self.Data1, self.Data2, self.Data3, bytes(self.Data4[:2]).hex().upper(),
                                           bytes(self.Data4[2:]).hex().upper())


class IMAGEHLP_MODULEW64(ctypes.Structure):
    _fields_ = [('SizeOfStruct', wintypes.DWORD), ('BaseOfImage', ctypes.c_uint64), ('ImageSize', wintypes.DWORD),
                ('TimeDateStamp', wintypes.DWORD), ('CheckSum', wintypes.DWORD), ('NumSyms', wintypes.DWORD),
                ('SymType', ctypes.c_int), ('ModuleName', wintypes.WCHAR * 32), ('ImageName', wintypes.WCHAR * 256),
                ('LoadedImageName', wintypes.WCHAR * 256), ('LoadedPdbName', wintypes.WCHAR * 256),
                ('CVSig', wintypes.DWORD), ('CVData', wintypes.WCHAR * (260 * 3)), ('PdbSig', wintypes.DWORD),
                ('PdbSig70', GUID), ('PdbAge', wintypes.DWORD), ('PdbUnmatched', wintypes.BOOL),
                ('DbgUnmatched', wintypes.BOOL), ('LineNumbers', wintypes.BOOL), ('GlobalSymbols', wintypes.BOOL),
                ('TypeInfo', wintypes.BOOL), ('SourceIndexed', wintypes.BOOL), ('Publics', wintypes.BOOL),
                ('MachineType', wintypes.DWORD), ('Reserved', wintypes.DWORD)]


class SYMBOL_INFOW(ctypes.Structure):
    _fields_ = [('SizeOfStruct', wintypes.ULONG), ('TypeIndex', wintypes.ULONG), ('Reserved', ctypes.c_uint64 * 2),
                ('Index', wintypes.ULONG), ('Size', wintypes.ULONG), ('ModBase', ctypes.c_uint64),
                ('Flags', wintypes.ULONG), ('Value', ctypes.c_uint64), ('Address', ctypes.c_uint64),
                ('Register', wintypes.ULONG), ('Scope', wintypes.ULONG), ('Tag', wintypes.ULONG),
                ('NameLen', wintypes.ULONG), ('MaxNameLen', wintypes.ULONG), ('Name', wintypes.WCHAR * 512)]


# sizeof(SYMBOL_INFOW) with Name[1], as DbgHelp expects (x64: 88)
SYMBOL_INFOW_BASE_SIZE = (SYMBOL_INFOW.Name.offset + ctypes.sizeof(wintypes.WCHAR) + 7) // 8 * 8
assert SYMBOL_INFOW_BASE_SIZE == 88

_sym_lock = threading.Lock()
_sym_next = [0x5000]


class Symbols:
    """Offline symbolization of one image at a given base with DbgHelp.
    The pseudo process handle is a unique number (DbgHelp only uses it as a
    key); the image is read from image_path, its PDB from the same folder."""
    SYMOPT_UNDNAME, SYMOPT_DEFERRED, SYMOPT_FAIL_CRIT, SYMOPT_EXACT = 0x2, 0x4, 0x200, 0x400

    def __init__(self, image_path, base):
        self.dh = ctypes.WinDLL('dbghelp', use_last_error=True)
        dh = self.dh
        dh.SymInitializeW.argtypes = [wintypes.HANDLE, wintypes.LPCWSTR, wintypes.BOOL]
        dh.SymLoadModuleExW.restype = ctypes.c_uint64
        dh.SymLoadModuleExW.argtypes = [wintypes.HANDLE, wintypes.HANDLE, wintypes.LPCWSTR, wintypes.LPCWSTR,
                                        ctypes.c_uint64, wintypes.DWORD, ctypes.c_void_p, wintypes.DWORD]
        dh.SymFromAddrW.argtypes = [wintypes.HANDLE, ctypes.c_uint64, ctypes.POINTER(ctypes.c_uint64),
                                    ctypes.POINTER(SYMBOL_INFOW)]
        dh.SymFromNameW.argtypes = [wintypes.HANDLE, wintypes.LPCWSTR, ctypes.POINTER(SYMBOL_INFOW)]
        dh.SymGetModuleInfoW64.argtypes = [wintypes.HANDLE, ctypes.c_uint64, ctypes.POINTER(IMAGEHLP_MODULEW64)]
        dh.SymCleanup.argtypes = [wintypes.HANDLE]
        with _sym_lock:
            _sym_next[0] += 4
            self.h = wintypes.HANDLE(_sym_next[0])
        self.image_path = os.path.abspath(image_path)
        self.base = base
        dh.SymSetOptions(self.SYMOPT_UNDNAME | self.SYMOPT_FAIL_CRIT | self.SYMOPT_EXACT)
        if not dh.SymInitializeW(self.h, os.path.dirname(self.image_path), False):
            raise RuntimeError('SymInitialize failed (%d)' % ctypes.get_last_error())
        got = dh.SymLoadModuleExW(self.h, None, self.image_path, None, base, 0, None, 0)
        if not got:
            raise RuntimeError('SymLoadModuleEx failed (%d)' % ctypes.get_last_error())
        m = IMAGEHLP_MODULEW64()
        m.SizeOfStruct = ctypes.sizeof(m)
        self.module = {}
        if dh.SymGetModuleInfoW64(self.h, base, ctypes.byref(m)):
            self.module = {'sym_type': m.SymType, 'loaded_pdb': m.LoadedPdbName, 'pdb_guid': m.PdbSig70.text(),
                           'pdb_age': m.PdbAge, 'pdb_unmatched': bool(m.PdbUnmatched), 'image_size': m.ImageSize}
        self.size = m.ImageSize

    def pdb_ok(self, rsds):
        """SymPdb (3), not unmatched, GUID equal to the image's RSDS GUID."""
        return (self.module.get('sym_type') == 3 and not self.module.get('pdb_unmatched') and rsds is not None and
                self.module.get('pdb_guid') == rsds.get('guid'))

    def name(self, addr):
        """'func+0xNN' or None when addr is outside the image or unknown."""
        if addr is None or not (self.base <= addr < self.base + (self.size or 0)):
            return None
        si = SYMBOL_INFOW()
        si.SizeOfStruct = SYMBOL_INFOW_BASE_SIZE
        si.MaxNameLen = 511
        disp = ctypes.c_uint64()
        if not self.dh.SymFromAddrW(self.h, addr, ctypes.byref(disp), ctypes.byref(si)):
            return None
        return '%s+0x%X' % (si.Name, disp.value)

    def addr(self, *names):
        for n in names:
            si = SYMBOL_INFOW()
            si.SizeOfStruct = SYMBOL_INFOW_BASE_SIZE
            si.MaxNameLen = 511
            if self.dh.SymFromNameW(self.h, n, ctypes.byref(si)):
                return si.Address
        return None

    def close(self):
        self.dh.SymCleanup(self.h)


# ---------------------------------------------------------------- debugger

class Debugger(threading.Thread):
    """Owns the debug loop. launch_fn() runs in this thread and must create
    the process with DEBUG_ONLY_THIS_PROCESS and return PROCESS_INFORMATION.
    image_path: the image (with its PDB next to it) used for symbols.
    watch: name of a function to trace (one-shot breakpoint at its entry and,
    from the return address found there, at its return site)."""

    def __init__(self, launch_fn, image_path=None, watch=None, cookie_names=('__security_cookie',
                                                                             '___security_cookie')):
        super().__init__(daemon=True)
        self.launch_fn = launch_fn
        self.image_path = image_path
        self.watch = watch
        self.cookie_names = cookie_names
        self.ready = threading.Event()
        self.done = threading.Event()
        self.stop_req = threading.Event()
        self.pi = None
        self.error = None
        self.rec = {'events': {}, 'exceptions': [], 'fatal': None, 'breakpoints': [], 'exit_code': None,
                    'dlls_loaded': 0, 'initial_breakpoints': 0, 'terminated_by_debugger': False,
                    'fatal_count': 0, 'unexpected_breakpoints': []}
        self.hproc = None
        self.threads = {}
        self.base = None
        self.sym = None
        self.bps = {}          # addr -> (orig byte, kind)
        self.frame = None      # watched function frame from its entry breakpoint

    def stop(self):
        self.stop_req.set()

    def run(self):
        try:
            self.pi = self.launch_fn()
        except Exception as e:   # noqa: BLE001 - reported to the caller
            self.error = 'launch: %s' % e
            self.ready.set()
            self.done.set()
            return
        self.ready.set()
        try:
            self.loop()
        except Exception as e:   # noqa: BLE001
            self.error = 'debug loop: %r' % e
            try:
                k32.TerminateProcess(self.pi.hProcess, KILLED_ON_STOP)
            except Exception:    # noqa: BLE001
                pass
        finally:
            if self.sym:
                self.sym.close()
            self.done.set()

    def count(self, name):
        self.rec['events'][name] = self.rec['events'].get(name, 0) + 1

    def loop(self):
        ev = DEBUG_EVENT()
        kill_sent = False
        while True:
            if not k32.WaitForDebugEvent(ctypes.byref(ev), 100):
                if self.stop_req.is_set() and not kill_sent:
                    k32.TerminateProcess(self.pi.hProcess, KILLED_ON_STOP)
                    kill_sent = True
                continue
            code = ev.dwDebugEventCode
            cont = DBG_CONTINUE
            if code == 1:
                self.count('exception')
                cont = self.on_exception(ev)
            elif code == 2:
                self.count('create_thread')
                self.threads[ev.dwThreadId] = ev.u.CreateThread.hThread
            elif code == 3:
                self.count('create_process')
                cp = ev.u.CreateProcessInfo
                if cp.hFile:
                    k32.CloseHandle(cp.hFile)
                self.hproc = cp.hProcess
                self.threads[ev.dwThreadId] = cp.hThread
                self.base = cp.lpBaseOfImage
                self.rec['image_base'] = hexs(self.base)
                self.on_image()
            elif code == 4:
                self.count('exit_thread')
                self.threads.pop(ev.dwThreadId, None)
            elif code == 5:
                self.count('exit_process')
                self.rec['exit_code'] = ev.u.Exit.dwExitCode
                self.rec['exit_code_hex'] = hexs(ev.u.Exit.dwExitCode)
                k32.ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, DBG_CONTINUE)
                return
            elif code == 6:
                self.count('load_dll')
                self.rec['dlls_loaded'] += 1
                if ev.u.LoadDll.hFile:
                    k32.CloseHandle(ev.u.LoadDll.hFile)
            else:
                self.count('other_%d' % code)
            k32.ContinueDebugEvent(ev.dwProcessId, ev.dwThreadId, cont)

    # -- image and breakpoints

    def on_image(self):
        if not self.image_path:
            return
        try:
            self.sym = Symbols(self.image_path, self.base)
            pe = pe_info(self.image_path)
            self.rec['symbols'] = dict(self.sym.module, pdb_ok=self.sym.pdb_ok(pe['rsds']), image_rsds=pe['rsds'])
        except Exception as e:   # noqa: BLE001
            self.rec['symbols'] = {'error': str(e)}
            self.sym = None
            return
        if self.watch:
            a = self.sym.addr(self.watch)
            self.rec['watch'] = {'name': self.watch, 'addr': hexs(a),
                                 'rva': hexs(a - self.base) if a else None}
            if a and self.rec['symbols'].get('pdb_ok'):
                self.set_bp(a, 'entry')

    def set_bp(self, addr, kind):
        orig = read_mem(self.hproc, addr, 1)
        ok = orig is not None and write_mem(self.hproc, addr, b'\xCC')
        if ok:
            self.bps[addr] = (orig, kind)
        self.rec['breakpoints'].append({'kind': kind, 'addr': hexs(addr), 'set': ok,
                                        'sym': self.sym.name(addr) if self.sym else None})

    def frame_slots(self, ebp, n=11):
        """Dwords [ebp-4 .. ebp-4+4n): cookie, saved EBP, return address,
        first argument, then the caller's frame."""
        raw = read_mem(self.hproc, ebp - 4, 4 * n)
        return None if raw is None else raw

    def cookie(self):
        a = self.sym.addr(*self.cookie_names) if self.sym else None
        return a, (read_u32(self.hproc, a) if a else None)

    def on_breakpoint(self, ev, addr):
        orig, kind = self.bps.pop(addr)
        write_mem(self.hproc, addr, orig)
        th = self.threads.get(ev.dwThreadId)
        c = get_ctx(th)
        hit = {'kind': kind, 'addr': hexs(addr), 'sym': self.sym.name(addr), 't': time.time(),
               'thread': ev.dwThreadId, 'context': ctx_dict(c)}
        if c is not None:
            if kind == 'entry':
                ret = read_u32(self.hproc, c.Esp)
                arg = read_u32(self.hproc, c.Esp + 4)
                fe = c.Esp - 4   # push ebp; mov ebp,esp (checked from the disassembly)
                ca, cv = self.cookie()
                self.frame = {'ebp': fe, 'entry_esp': c.Esp, 'ret': ret, 'saved_ebp': c.Ebp, 'arg0': arg,
                              'cookie_addr': ca, 'cookie': cv, 'thread': ev.dwThreadId}
                hit.update({'return_address': hexs(ret), 'return_sym': self.sym.name(ret),
                            'caller_ebp': hexs(c.Ebp), 'arg0': hexs(arg), 'frame_ebp': hexs(fe),
                            'cookie_addr': hexs(ca), 'cookie': hexs(cv),
                            'expected_cookie_slot': hexs(cv ^ fe) if cv is not None else None})
                if ret is not None:
                    self.set_bp(ret, 'return')
            elif kind == 'return' and self.frame:
                fe = self.frame['ebp']
                raw = self.frame_slots(fe)
                hit.update(self.slot_report(raw, fe))
                hit['esp_after_return'] = hexs(c.Esp)
            c.Eip = addr
            if not k32.Wow64SetThreadContext(th, ctypes.byref(c)):
                hit['set_context_error'] = ctypes.get_last_error()
        self.rec['breakpoints'].append(hit)
        return DBG_CONTINUE

    def slot_report(self, raw, fe):
        if raw is None:
            return {'slots_error': 'read failed'}
        d = struct.unpack('<%dI' % (len(raw) // 4), raw)
        _, cv = self.cookie()
        exp = (cv ^ fe) if cv is not None else None
        f = self.frame
        return {'frame_ebp': hexs(fe), 'slots_hex': raw.hex(), 'cookie_slot': hexs(d[0]),
                'expected_cookie_slot': hexs(exp), 'cookie_intact': exp is not None and d[0] == exp,
                'saved_ebp_slot': hexs(d[1]), 'saved_ebp_at_entry': hexs(f['saved_ebp']),
                'saved_ebp_intact': d[1] == f['saved_ebp'], 'return_slot': hexs(d[2]),
                'return_at_entry': hexs(f['ret']), 'return_intact': d[2] == f['ret'],
                'return_slot_sym': self.sym.name(d[2]), 'arg0_slot': hexs(d[3]), 'arg0_at_entry': hexs(f['arg0']),
                'arg0_intact': d[3] == f['arg0']}

    # -- exceptions

    def on_exception(self, ev):
        er = ev.u.Exception.ExceptionRecord
        code = er.ExceptionCode
        addr = er.ExceptionAddress or 0
        first = bool(ev.u.Exception.dwFirstChance)
        if code in (STATUS_BREAKPOINT, STATUS_WX86_BREAKPOINT):
            if addr in self.bps:
                return self.on_breakpoint(ev, addr)
            # WOW64 start-up gives two (64-bit loader, then 0x4000001F for the
            # 32-bit one); any further int3 not set by the driver is recorded
            # as unexpected (evaluate_h041 refuses it).
            self.rec['initial_breakpoints'] += 1
            if self.rec['initial_breakpoints'] > 2:
                self.rec['unexpected_breakpoints'].append(
                    {'code': hexs(code), 'address': hexs(addr), 'thread': ev.dwThreadId,
                     'sym': self.sym.name(addr) if self.sym else None})
            return DBG_CONTINUE
        info = [hexs(er.ExceptionInformation[i]) for i in range(min(er.NumberParameters, 15))]
        e = {'code': hexs(code), 'flags': hexs(er.ExceptionFlags), 'address': hexs(addr), 'first_chance': first,
             'params': info, 'thread': ev.dwThreadId, 't': time.time(),
             'sym': self.sym.name(addr) if self.sym else None,
             'rva': hexs(addr - self.base) if self.base and addr >= self.base else None}
        if len(self.rec['exceptions']) < 200:
            self.rec['exceptions'].append(e)
        fatal = code in FATAL_CODES or not first or (er.ExceptionFlags & 1)
        if not fatal:
            return DBG_EXCEPTION_NOT_HANDLED
        e['fatal'] = True
        self.rec['fatal_count'] += 1
        if self.rec['fatal'] is not None:
            # keep the first fatal exception; a later one is only listed
            return DBG_CONTINUE
        self.rec['fatal'] = e
        try:
            e['evidence'] = self.collect(ev)
        except Exception as x:   # noqa: BLE001
            e['evidence_error'] = repr(x)
        ok = k32.TerminateProcess(self.hproc, KILLED_BY_DEBUGGER)
        self.rec['terminated_by_debugger'] = bool(ok)
        return DBG_CONTINUE

    def collect(self, ev):
        """Context, EBP chain, raw stack with symbols, and the watched
        function's frame slots compared with their values at entry."""
        th = self.threads.get(ev.dwThreadId)
        c = get_ctx(th)
        out = {'context': ctx_dict(c)}
        if c is None:
            return out
        chain = []
        ebp = c.Ebp
        for _ in range(16):
            nxt = read_u32(self.hproc, ebp)
            ret = read_u32(self.hproc, ebp + 4)
            if nxt is None or ret is None:
                chain.append({'ebp': hexs(ebp), 'unreadable': True})
                break
            chain.append({'ebp': hexs(ebp), 'next_ebp': hexs(nxt), 'ret': hexs(ret),
                          'ret_sym': self.sym.name(ret) if self.sym else None})
            if nxt <= ebp or nxt - ebp > 0x100000:
                break
            ebp = nxt
        out['ebp_chain'] = chain
        raw = read_mem(self.hproc, c.Esp, 0x100)
        scan = []
        if raw:
            for i in range(0, len(raw), 4):
                v = struct.unpack_from('<I', raw, i)[0]
                s = self.sym.name(v) if self.sym else None
                if s:
                    scan.append({'at': hexs(c.Esp + i), 'value': hexs(v), 'sym': s})
        out['stack_scan_code_addresses'] = scan
        # __report_gsfailure of this CRT is frameless (C-11 crash control):
        # the return address into the failing function is at [ESP] and EBP
        # is still that function's frame.
        r0 = read_u32(self.hproc, c.Esp)
        out['ret_at_esp'] = {'value': hexs(r0), 'sym': self.sym.name(r0) if self.sym else None,
                             'rva': hexs(r0 - self.base) if r0 is not None and self.base and r0 >= self.base
                             else None}
        # deeper heuristic scan: code addresses above the watched frame
        if self.frame:
            fe = self.frame['ebp']
            out['watched_frame'] = self.slot_report(self.frame_slots(fe), fe)
            out['watched_frame']['entry_return_sym'] = self.sym.name(self.frame['ret'])
            out['watched_frame']['entry_thread'] = self.frame.get('thread')
            deep = read_mem(self.hproc, fe + 40, 0x400)
            ds = []
            if deep:
                for i in range(0, len(deep), 4):
                    v = struct.unpack_from('<I', deep, i)[0]
                    s = self.sym.name(v)
                    if s:
                        ds.append({'at': hexs(fe + 40 + i), 'value': hexs(v), 'sym': s})
            out['stack_scan_above_frame_heuristic'] = ds[:24]
        if self.sym:
            out['module_rva_of_eip'] = hexs(c.Eip - self.base) if c.Eip >= self.base else None
        return out


# ---------------------------------------------------------------- endpoints

class MIB_TCPROW_OWNER_PID(ctypes.Structure):
    _fields_ = [('state', wintypes.DWORD), ('la', wintypes.DWORD), ('lp', wintypes.DWORD),
                ('ra', wintypes.DWORD), ('rp', wintypes.DWORD), ('pid', wintypes.DWORD)]


class MIB_TCP6ROW_OWNER_PID(ctypes.Structure):
    _fields_ = [('la', ctypes.c_ubyte * 16), ('lscope', wintypes.DWORD), ('lp', wintypes.DWORD),
                ('ra', ctypes.c_ubyte * 16), ('rscope', wintypes.DWORD), ('rp', wintypes.DWORD),
                ('state', wintypes.DWORD), ('pid', wintypes.DWORD)]


class MIB_UDPROW_OWNER_PID(ctypes.Structure):
    _fields_ = [('la', wintypes.DWORD), ('lp', wintypes.DWORD), ('pid', wintypes.DWORD)]


class MIB_UDP6ROW_OWNER_PID(ctypes.Structure):
    _fields_ = [('la', ctypes.c_ubyte * 16), ('lscope', wintypes.DWORD), ('lp', wintypes.DWORD),
                ('pid', wintypes.DWORD)]


iph.GetExtendedTcpTable.argtypes = [ctypes.c_void_p, ctypes.POINTER(wintypes.DWORD), wintypes.BOOL, wintypes.ULONG,
                                    ctypes.c_int, wintypes.ULONG]
iph.GetExtendedUdpTable.argtypes = iph.GetExtendedTcpTable.argtypes


def _table(fn, af, cls, row):
    size = wintypes.DWORD(0)
    fn(None, ctypes.byref(size), False, af, cls, 0)
    for _ in range(4):
        buf = ctypes.create_string_buffer(size.value + 4096)
        size = wintypes.DWORD(len(buf))
        r = fn(buf, ctypes.byref(size), False, af, cls, 0)
        if r == 0:
            n = struct.unpack_from('<I', buf.raw, 0)[0]
            rs = ctypes.sizeof(row)
            return [row.from_buffer_copy(buf.raw, 4 + i * rs) for i in range(n)]
        if r != 122:   # ERROR_INSUFFICIENT_BUFFER
            raise OSError('iphlpapi table error %d' % r)
    raise OSError('iphlpapi table kept growing')


def _ip4(v):
    return '.'.join(str(b) for b in struct.pack('<I', v))


def _port(v):
    return ((v & 0xFF) << 8) | ((v >> 8) & 0xFF)


def _ip6(b):
    return ':'.join('%x' % struct.unpack('>H', bytes(b[i:i + 2]))[0] for i in range(0, 16, 2))


def endpoints(pids):
    """All TCP (any state) and UDP endpoints, IPv4 and IPv6, owned by pids."""
    pids = set(pids)
    out = []
    for r in _table(iph.GetExtendedTcpTable, 2, 5, MIB_TCPROW_OWNER_PID):
        if r.pid in pids:
            out.append('TCP %s:%d -> %s:%d state %d pid %d' % (_ip4(r.la), _port(r.lp), _ip4(r.ra), _port(r.rp),
                                                              r.state, r.pid))
    for r in _table(iph.GetExtendedTcpTable, 23, 5, MIB_TCP6ROW_OWNER_PID):
        if r.pid in pids:
            out.append('TCP6 [%s]:%d -> [%s]:%d state %d pid %d' % (_ip6(r.la), _port(r.lp), _ip6(r.ra),
                                                                   _port(r.rp), r.state, r.pid))
    for r in _table(iph.GetExtendedUdpTable, 2, 1, MIB_UDPROW_OWNER_PID):
        if r.pid in pids:
            out.append('UDP %s:%d pid %d' % (_ip4(r.la), _port(r.lp), r.pid))
    for r in _table(iph.GetExtendedUdpTable, 23, 1, MIB_UDP6ROW_OWNER_PID):
        if r.pid in pids:
            out.append('UDP6 [%s]:%d pid %d' % (_ip6(r.la), _port(r.lp), r.pid))
    return out


# ---------------------------------------------------------------- processes

class PROCESSENTRY32W(ctypes.Structure):
    _fields_ = [('dwSize', wintypes.DWORD), ('cntUsage', wintypes.DWORD), ('th32ProcessID', wintypes.DWORD),
                ('th32DefaultHeapID', ctypes.c_size_t), ('th32ModuleID', wintypes.DWORD),
                ('cntThreads', wintypes.DWORD), ('th32ParentProcessID', wintypes.DWORD),
                ('pcPriClassBase', ctypes.c_long), ('dwFlags', wintypes.DWORD), ('szExeFile', wintypes.WCHAR * 260)]


k32.CreateToolhelp32Snapshot.restype = wintypes.HANDLE
k32.Process32FirstW.argtypes = [wintypes.HANDLE, ctypes.POINTER(PROCESSENTRY32W)]
k32.Process32NextW.argtypes = [wintypes.HANDLE, ctypes.POINTER(PROCESSENTRY32W)]


def processes():
    """[(pid, ppid, exe name)] of all processes."""
    h = k32.CreateToolhelp32Snapshot(0x2, 0)
    if not h or h == wintypes.HANDLE(-1).value:
        return []
    out = []
    e = PROCESSENTRY32W()
    e.dwSize = ctypes.sizeof(e)
    ok = k32.Process32FirstW(h, ctypes.byref(e))
    while ok:
        out.append((e.th32ProcessID, e.th32ParentProcessID, e.szExeFile))
        ok = k32.Process32NextW(h, ctypes.byref(e))
    k32.CloseHandle(h)
    return out


class ProcWatcher(threading.Thread):
    """Polls the process list every `period` s. Records processes named in
    names (lower case) that were not running at start, and children of the
    pids added with watch_pid(). Short-lived processes between two polls
    can be missed; the self-test measures what is caught."""

    def __init__(self, names=WER_NAMES, period=0.1):
        super().__init__(daemon=True)
        self.names = tuple(n.lower() for n in names)
        self.period = period
        self.parents = set()
        self.base = {(p, n.lower()) for p, _, n in processes()}
        self.seen = {}
        self.polls = 0
        self.halt = threading.Event()

    def watch_pid(self, pid):
        self.parents.add(pid)

    def run(self):
        while not self.halt.is_set():
            t = time.time()
            for pid, ppid, name in processes():
                ln = name.lower()
                key = (pid, ln)
                if key in self.seen or key in self.base:
                    continue
                why = []
                if ln in self.names:
                    why.append('name')
                if ppid in self.parents:
                    why.append('child of %d' % ppid)
                if why:
                    self.seen[key] = {'pid': pid, 'ppid': ppid, 'name': name, 'first_seen': t, 'why': why}
            self.polls += 1
            self.halt.wait(self.period)

    def stop(self):
        self.halt.set()
        self.join(5)
        return {'polls': self.polls, 'period_s': self.period, 'names': list(self.names),
                'parents': sorted(self.parents), 'found': list(self.seen.values())}


def pid_alive(pid):
    h = k32.OpenProcess(0x1000 | 0x00100000, False, pid)   # QUERY_LIMITED_INFORMATION | SYNCHRONIZE
    if not h:
        return False
    alive = k32.WaitForSingleObject(h, 0) != 0
    k32.CloseHandle(h)
    return alive


# ---------------------------------------------------------------- WER

def wersvc_state():
    r = subprocess.run(['sc', 'query', 'WerSvc'], capture_output=True, text=True, errors='replace')
    for line in r.stdout.splitlines():
        if 'STATE' in line or 'TILA' in line.upper():
            return line.split(':', 1)[-1].strip()
    return r.stdout.strip()[-200:]


def wer_events(since_utc, names):
    """Application log events 1000 (Application Error), 1001 (Windows Error
    Reporting), 1002 (hang) and 1005 created at or after since_utc
    (datetime, UTC). Returns (all events as text blocks, the ones that
    mention one of names)."""
    ts = since_utc.strftime('%Y-%m-%dT%H:%M:%S.000Z')
    q = ("*[System[(EventID=1000 or EventID=1001 or EventID=1002 or EventID=1005) and "
         "TimeCreated[@SystemTime>='%s']]]" % ts)
    r = subprocess.run(['wevtutil', 'qe', 'Application', '/q:' + q, '/f:text', '/rd:true', '/c:50'],
                       capture_output=True, text=True, errors='replace')
    blocks = [b.strip() for b in r.stdout.split('Event[') if b.strip()]
    hits = [b for b in blocks if any(n.lower() in b.lower() for n in names)]
    return {'query_since_utc': ts, 'returncode': r.returncode, 'stderr': r.stderr.strip()[:300],
            'count': len(blocks), 'events': [b[:1500] for b in blocks[:10]]}, [b[:1500] for b in hits]


def utcnow():
    return datetime.datetime.now(datetime.timezone.utc)


# ---------------------------------------------------------------- H-041

# The shipped program under test (testaus.md 17.1, 18.1): pinned, not only
# "copy equals source".
HKMAALI_EXPECT = {'sha256': '61034e5dbb18817985d6fe64343b5d1ee2aa9fbab91495bd43549eaa0d7d297d',
                  'rsds_guid': '{B0500861-3E83-4118-B0DA-E8E331F12614}', 'addfrec': 0x00423120,
                  'ret_sites': (0x00423237, 0x00423200), 'cookie': 0x004F8700, 'image_base': 0x00400000,
                  'report_gsfailure_rva': 0x000A1295}
ADDFREC_RET_SITES_RVA = (0x23237, 0x23200)   # after both __security_check_cookie calls (dumpbin, section 18.1)


def outside_noise(path):
    """Changes outside the sandbox that are system activity seen in every run
    of C-11 (runs 0, 1, the crash control and the self-test, with and without
    the program): Remote Desktop client traces under %TEMP%. Rule fixed
    before the H-041 runs (testaus.md 19.6). Anything under WER or
    CrashDumps is never noise."""
    u = path.upper()
    if '\\WER\\' in u or 'CRASHDUMPS' in u:
        return False
    return '\\DIAGOUTPUTDIR\\RDCLIENTAUTOTRACE\\' in u and u.endswith('.ETL')


def evaluate_h041(control, overflow, logs=None, recsize_ctl=10000, recsize_ovf=10040):
    """Applies the acceptance criteria of testaus.md 18.4 as refined in 19.6
    to two run records (run.json of run_hkmaali.py) and, for criterion 7,
    the verify_log.py results of both runs (logs = {'control': {...},
    'overflow': {...}}). Returns the criteria and a verdict: 'confirmed by
    running' only if every criterion holds; otherwise 'not confirmed'."""
    res = {}
    logs = logs or {}

    def put(k, ok, detail):
        res[k] = (bool(ok), detail)

    def prov(r):
        r = r or {}
        sym = (r.get('run', {}).get('debugger', {}).get('symbols') or {})
        return (r.get('probe', {}).get('isolation_verified') is True and
                r.get('exe_sha256') == HKMAALI_EXPECT['sha256'] and
                r.get('exe_copy_sha256') == HKMAALI_EXPECT['sha256'] and
                sym.get('pdb_ok') is True and sym.get('pdb_guid') == HKMAALI_EXPECT['rsds_guid'] and
                (r.get('post_probe') or {}).get('isolation_verified') is True)
    put('1_isolation_and_provenance', prov(control) and prov(overflow),
        'probe before and after verified; exe and copy SHA-256 and PDB GUID equal the pinned values, both runs')

    c = (control or {}).get('run', {})
    cd = c.get('debugger', {})
    ctl_bp = [b for b in cd.get('breakpoints', []) if b.get('kind') == 'return' and 'cookie_intact' in b]
    put('2_control_pass',
        c.get('status') == 'ok' and c.get('exited_by_itself') is True and not cd.get('fatal') and
        cd.get('fatal_count', 0) == 0 and not cd.get('unexpected_breakpoints') and
        not any(not e.get('first_chance', True) for e in cd.get('exceptions', [])) and
        c.get('kilp_dat_size') == 2 * recsize_ctl and len(ctl_bp) == 1 and ctl_bp[0].get('cookie_intact') is True
        and ctl_bp[0].get('return_intact') is True,
        {'status': c.get('status'), 'kilp_dat_size': c.get('kilp_dat_size'), 'fatal': cd.get('fatal'),
         'return_bp': ctl_bp[:1]})

    o = (overflow or {}).get('run', {})
    od = o.get('debugger', {})
    steps_o = [s.get('seen') for s in o.get('steps', [])]
    put('3_overflow_ui_path', bool(o.get('steps')) and all(steps_o), {'steps_seen': steps_o})

    f = od.get('fatal') or {}
    ev = f.get('evidence') or {}
    wf = ev.get('watched_frame') or {}
    chain = ev.get('ebp_chain') or []
    base = int(od.get('image_base') or '0x0', 16)
    ret_rvas = [int(x['ret'], 16) - base for x in chain[:2] if x.get('ret')]
    esp_ret = (ev.get('ret_at_esp') or {}).get('rva')
    if esp_ret:
        ret_rvas.insert(0, int(esp_ret, 16))
    ebp_is_frame = (ev.get('context') or {}).get('Ebp') == wf.get('frame_ebp') and wf.get('frame_ebp') is not None
    second = [e for e in od.get('exceptions', []) if not e.get('first_chance', True)]
    first_second_is_fatal = not second or second[0] is f or (second[0].get('t') == f.get('t') and
                                                              second[0].get('code') == f.get('code'))
    put('4_exception',
        f.get('code') == hexs(STATUS_STACK_BUFFER_OVERRUN) and (f.get('params') or [None])[0] == hexs(2) and
        f.get('rva') == hexs(HKMAALI_EXPECT['report_gsfailure_rva']) and
        any(r in ADDFREC_RET_SITES_RVA for r in ret_rvas) and ebp_is_frame and
        od.get('fatal_count') == 1 and first_second_is_fatal and not od.get('unexpected_breakpoints') and
        f.get('thread') is not None and f.get('thread') == wf.get('entry_thread') and
        od.get('terminated_by_debugger') is True and o.get('exit_code') == KILLED_BY_DEBUGGER and
        'kilptietue::addtall' in (wf.get('entry_return_sym') or ''),
        {'code': f.get('code'), 'params': f.get('params'), 'rva': f.get('rva'), 'sym': f.get('sym'),
         'return_rvas': [hexs(r) for r in ret_rvas], 'ebp_is_watched_frame': ebp_is_frame,
         'fatal_count': od.get('fatal_count'), 'thread': f.get('thread'), 'entry_thread': wf.get('entry_thread'),
         'caller_at_entry': wf.get('entry_return_sym'), 'exit_code': hexs(o.get('exit_code'))})

    put('5_cookie_overwritten', wf.get('cookie_intact') is False and wf.get('return_intact') is False,
        {k: wf.get(k) for k in ('cookie_slot', 'expected_cookie_slot', 'return_slot', 'return_at_entry',
                                'saved_ebp_slot', 'saved_ebp_at_entry')})

    match = o.get('stack_vs_record') or {}
    put('6_kilp_dat', o.get('kilp_dat_size') == 2 * recsize_ovf and match.get('equal') is True and
        match.get('compared_bytes') == recsize_ovf - 10000,
        {'kilp_dat_size': o.get('kilp_dat_size'), 'stack_vs_record': match})

    clean = True
    det = {}
    for name, r in (('control', control), ('overflow', overflow)):
        r = r or {}
        rr = r.get('run', {})
        w = rr.get('wer') or {}
        lg = logs.get(name) or {}
        changes = r.get('outside_changes')
        real = [p for p in (changes or []) if not outside_noise(p.replace('deleted: ', ''))]
        ok = (changes is not None and not real and not w.get('processes', {}).get('found') and
              w.get('processes', {}).get('polls', 0) > 0 and not w.get('events_matching') and
              'STOPPED' in (w.get('wersvc_after') or '') and not rr.get('sockets_seen') and
              rr.get('job', {}).get('TotalProcesses') == 1 and not r.get('outside_probe_file_exists') and
              not r.get('registry_key_exists_after') and
              lg.get('ok') is True and lg.get('program_ok') is True and lg.get('post_probe_ok') is True and
              lg.get('covers_run') is True)
        det[name] = {'outside_changes_not_noise': real, 'wer_processes': w.get('processes', {}).get('found'),
                     'wer_events': w.get('events_matching'), 'wersvc_after': w.get('wersvc_after'),
                     'sockets': rr.get('sockets_seen'), 'job': rr.get('job'),
                     'log': {k: lg.get(k) for k in ('ok', 'program_ok', 'post_probe_ok', 'covers_run')}, 'ok': ok}
        clean = clean and ok
    put('7_clean_isolation_record', clean, det)
    verdict = 'confirmed by running' if all(v[0] for v in res.values()) else 'not confirmed'
    return {'criteria': {k: {'ok': v[0], 'detail': v[1]} for k, v in res.items()}, 'verdict': verdict}
