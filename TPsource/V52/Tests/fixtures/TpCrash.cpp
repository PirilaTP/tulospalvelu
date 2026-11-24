// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
// Copyright (C) 2015 Pekka Pirila

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Crash evidence for tests that end the process on purpose (TpCore.h).
// Output is written with WriteFile to the stdout handle from a stack buffer,
// so it works inside an exception handler or the ASan report callback
// without CRT stream locks. The runner checks every field independently
// (run_core_tests.py, classify), so this code only reports what it sees.

#include "TpCore.h"
#include <dbghelp.h>
#ifdef __SANITIZE_ADDRESS__
#include <sanitizer/asan_interface.h>
#endif

#pragma comment(lib, "dbghelp.lib")

namespace tp {

static const int MAXEXPECT = 8;
static char g_finding[32];
static char g_mechanism[64];
static char g_source[16];
static const char *g_expect[MAXEXPECT + 1];
static volatile LONG g_armed = 0;
static volatile LONG g_fired = 0;
static ExceptionExpect g_ex;
static bool g_symOk = false;

static void rawOut(const char *s)
{
	DWORD n;
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteFile(h, s, (DWORD) strlen(s), &n, NULL);
	FlushFileBuffers(h);
}

void armCrash(const char *finding, const char *mechanism, const char *source,
	const char *const *expectSubstrings)
{
	strncpy(g_finding, finding, sizeof(g_finding) - 1);
	strncpy(g_mechanism, mechanism, sizeof(g_mechanism) - 1);
	strncpy(g_source, source, sizeof(g_source) - 1);
	int i = 0;
	for (; expectSubstrings && expectSubstrings[i] && i < MAXEXPECT; i++)
		g_expect[i] = expectSubstrings[i];
	g_expect[i] = 0;
	fflush(stdout);
	char b[200];
	_snprintf(b, sizeof(b) - 1, "TPARM|finding=%s|mechanism=%s|source=%s\n", finding, mechanism, source);
	b[sizeof(b) - 1] = 0;
	rawOut(b);
	InterlockedExchange(&g_armed, 1);
}

void disarmCrash()
{
	InterlockedExchange(&g_armed, 0);
	fflush(stdout);
	rawOut("TPDISARM|\n");
}

#ifdef __SANITIZE_ADDRESS__
void asanReportCallback(const char *report)
{
	if (InterlockedExchange(&g_fired, 1))
		return;
	char b[600];
	if (!g_armed || strcmp(g_source, "asan") != 0) {
		rawOut("TPCRASH|source=asan|armed=0\n");
		return;
	}
	for (int i = 0; g_expect[i]; i++) {
		if (!strstr(report, g_expect[i])) {
			_snprintf(b, sizeof(b) - 1, "TPCRASH|source=asan|armed=1|match=0|missing=%s\n", g_expect[i]);
			b[sizeof(b) - 1] = 0;
			rawOut(b);
			return;
		}
	}
	// First report line after the "ERROR:" header, for the detail text.
	const char *w = strstr(report, "WRITE of size");
	if (!w)
		w = strstr(report, "READ of size");
	char first[120] = "";
	if (w) {
		int k = 0;
		while (w[k] && w[k] != '\n' && w[k] != '\r' && k < (int) sizeof(first) - 1) {
			first[k] = w[k] == '|' ? '/' : w[k];
			k++;
		}
		first[k] = 0;
	}
	_snprintf(b, sizeof(b) - 1,
		"TPRESULT|finding=%s|mechanism=%s|observed=1|source=asan|detail=ASan report matched all expected "
		"strings; %s\n", g_finding, g_mechanism, first);
	b[sizeof(b) - 1] = 0;
	rawOut(b);
}
#endif

// Appends the symbol names (inline frames first) for addr to out.
static void symbolChain(DWORD64 addr, char *out, size_t outLen)
{
	out[0] = 0;
	if (!g_symOk)
		return;
	HANDLE proc = GetCurrentProcess();
	char sbuf[sizeof(SYMBOL_INFO) + 256];
	SYMBOL_INFO *sym = (SYMBOL_INFO *) sbuf;
	DWORD inl = SymAddrIncludeInlineTrace(proc, addr);
	DWORD ctx = 0, frameIndex = 0;
	if (inl > 0 && SymQueryInlineTrace(proc, addr, 0, addr, addr, &ctx, &frameIndex)) {
		for (DWORD i = 0; i < inl; i++) {
			memset(sbuf, 0, sizeof(sbuf));
			sym->SizeOfStruct = sizeof(SYMBOL_INFO);
			sym->MaxNameLen = 255;
			DWORD64 disp = 0;
			if (SymFromInlineContext(proc, addr, ctx + i, &disp, sym)) {
				if (out[0])
					strncat(out, ">", outLen - strlen(out) - 1);
				strncat(out, sym->Name, outLen - strlen(out) - 1);
			}
		}
	}
	memset(sbuf, 0, sizeof(sbuf));
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);
	sym->MaxNameLen = 255;
	DWORD64 disp = 0;
	if (SymFromAddr(proc, addr, &disp, sym)) {
		if (out[0])
			strncat(out, ">", outLen - strlen(out) - 1);
		strncat(out, sym->Name, outLen - strlen(out) - 1);
	}
}

static LONG CALLBACK vehHandler(EXCEPTION_POINTERS *ep)
{
	EXCEPTION_RECORD *er = ep->ExceptionRecord;
	if (er->ExceptionCode != g_ex.code || !g_armed)
		return EXCEPTION_CONTINUE_SEARCH;
	if (strcmp(threadTag(), g_ex.threadTag ? g_ex.threadTag : "") != 0)
		return EXCEPTION_CONTINUE_SEARCH;
	if (InterlockedExchange(&g_fired, 1))
		return EXCEPTION_CONTINUE_SEARCH;
	int access = er->NumberParameters >= 2 ? (int) er->ExceptionInformation[0] : -1;
	ULONG_PTR target = er->NumberParameters >= 2 ? er->ExceptionInformation[1] : 0;
	DWORD64 pc = (DWORD64) (ULONG_PTR) er->ExceptionAddress;
	HMODULE mod = 0;
	GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR) er->ExceptionAddress, &mod);
	char syms[400];
	symbolChain(pc, syms, sizeof(syms));
	const char *acc = access == 0 ? "read" : access == 1 ? "write" : access == 8 ? "execute" : "unknown";
	char b[800];
	_snprintf(b, sizeof(b) - 1,
		"TPCRASH|source=veh|code=0x%08lX|access=%s|target=0x%08lX|thread=%s|rva=0x%08lX|symbols=%s\n",
		(unsigned long) er->ExceptionCode, acc, (unsigned long) target, threadTag(),
		(unsigned long) (pc - (DWORD64) (ULONG_PTR) mod), syms);
	b[sizeof(b) - 1] = 0;
	rawOut(b);
	bool match = access == g_ex.access && target == g_ex.target;
	if (match && g_ex.symbol) {
		match = false;
		const char *p = syms;
		size_t l = strlen(g_ex.symbol);
		while ((p = strstr(p, g_ex.symbol)) != 0) {
			if ((p == syms || p[-1] == '>') && (p[l] == 0 || p[l] == '>')) {
				match = true;
				break;
			}
			p += l;
		}
	}
	if (match) {
		_snprintf(b, sizeof(b) - 1,
			"TPRESULT|finding=%s|mechanism=%s|observed=1|source=exception|detail=%s access to 0x%08lX "
			"on thread %s in %s\n", g_finding, g_mechanism, acc, (unsigned long) target, threadTag(), syms);
		b[sizeof(b) - 1] = 0;
		rawOut(b);
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

bool installExceptionEvidence(const ExceptionExpect &e)
{
#ifdef __SANITIZE_ADDRESS__
	(void) e;
	return false;   // ASan owns access violations in instrumented builds
#else
	g_ex = e;
	char dir[MAX_PATH];
	DWORD n = GetModuleFileNameA(NULL, dir, MAX_PATH);
	if (n == 0 || n >= MAX_PATH)
		return false;
	char *slash = strrchr(dir, '\\');
	if (slash)
		*slash = 0;
	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS |
		SYMOPT_NO_PROMPTS | SYMOPT_LOAD_LINES);
	// Search path = exe directory only: no symbol server, no _NT_SYMBOL_PATH.
	g_symOk = SymInitialize(GetCurrentProcess(), dir, TRUE) != FALSE;
	if (!g_symOk)
		return false;
	// Force the exe's symbols now, so the handler only does lookups.
	char sbuf[sizeof(SYMBOL_INFO) + 256];
	SYMBOL_INFO *sym = (SYMBOL_INFO *) sbuf;
	memset(sbuf, 0, sizeof(sbuf));
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);
	sym->MaxNameLen = 255;
	DWORD64 disp = 0;
	if (!SymFromAddr(GetCurrentProcess(), (DWORD64) (ULONG_PTR) &installExceptionEvidence, &disp, sym))
		return false;
	return AddVectoredExceptionHandler(1, vehHandler) != NULL;
#endif
}

}  // namespace tp
