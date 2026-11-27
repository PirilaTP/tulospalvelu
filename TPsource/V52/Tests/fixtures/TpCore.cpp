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

// Implementation of the shared core test helpers (TpCore.h) and the stubs
// that replace two tputilv2 objects:
//  - WRITEERR.obj (writeerror, writeerror_w, writeerrorOn): would wait for a
//    key press or open a MessageBox.
//  - perr.obj (perr, wperr): opens a MessageBox on a Windows API error.
// A stub must define every external symbol of the object it replaces;
// otherwise the linker pulls the library member in as well and reports
// duplicate symbols.

#include "TpCore.h"
#include <stdarg.h>
#include <process.h>

namespace tp {

bool sandboxOk()
{
	char token[200];
	DWORD n = GetEnvironmentVariableA("TP_SANDBOX_TOKEN", token, sizeof(token));
	if (n == 0 || n >= sizeof(token) || n < 16)
		return false;
	FILE *f = fopen(".tp_sandbox", "rb");
	if (!f)
		return false;
	char buf[200];
	size_t l = fread(buf, 1, sizeof(buf) - 1, f);
	fclose(f);
	buf[l] = 0;
	return strlen(token) == l && memcmp(token, buf, l) == 0;
}

static std::string clean(const std::string &s)
{
	std::string r(s);
	for (size_t i = 0; i < r.size(); i++)
		if (r[i] == '|' || r[i] == '\n' || r[i] == '\r')
			r[i] = ' ';
	return r;
}

void reportResult(const char *finding, const char *mechanism, bool observed,
	const std::string &detail)
{
	printf("TPRESULT|finding=%s|mechanism=%s|observed=%d|detail=%s\n",
		finding, mechanism, observed ? 1 : 0, clean(detail).c_str());
	fflush(stdout);
}

void reportSkip(const std::string &reason)
{
	printf("TPSKIP|reason=%s\n", clean(reason).c_str());
	fflush(stdout);
}

void info(const std::string &text)
{
	printf("TPINFO|%s\n", clean(text).c_str());
	fflush(stdout);
}

std::string fmt(const char *format, ...)
{
	char buf[1001];
	va_list ap;
	va_start(ap, format);
	_vsnprintf(buf, 1000, format, ap);
	va_end(ap);
	buf[1000] = 0;
	return std::string(buf);
}

struct ProbeArg {
	CRITICAL_SECTION *cs;
	int result;
};

static unsigned __stdcall probeThread(void *p)
{
	ProbeArg *a = (ProbeArg *) p;
	// A probe on the owning thread would succeed recursively (thread id
	// reuse); report that instead of a wrong answer.
	if ((DWORD) (ULONG_PTR) a->cs->OwningThread == GetCurrentThreadId()) {
		a->result = -2;
		return 0;
	}
	if (TryEnterCriticalSection(a->cs)) {
		LeaveCriticalSection(a->cs);
		a->result = 1;
	}
	else
		a->result = 0;
	return 0;
}

int lockFreeForOtherThread(CRITICAL_SECTION *cs)
{
	ProbeArg a;
	a.cs = cs;
	a.result = -1;
	HANDLE h = (HANDLE) _beginthreadex(NULL, 0, probeThread, &a, 0, NULL);
	if (h == 0)
		return -1;
	// TryEnterCriticalSection never blocks. Wait without a limit so that the
	// probe thread can never outlive a (stack) ProbeArg; the runner's process
	// timeout is the safety bound.
	DWORD w = WaitForSingleObject(h, INFINITE);
	CloseHandle(h);
	if (w != WAIT_OBJECT_0)
		return -1;
	return a.result;
}

// ---- controlled threads ----

static __declspec(thread) const char *t_threadTag = "";
static volatile LONG g_blockedThreads = 0;

struct RunCtx {
	ThreadFn fn;
	void *arg;
	const char *tag;
};

static unsigned __stdcall runThread(void *p)
{
	RunCtx *c = (RunCtx *) p;   // heap, never freed if the thread blocks
	t_threadTag = c->tag;
	c->fn(c->arg);
	return 0;
}

ThreadRun runOnThread(ThreadFn fn, void *arg, DWORD timeoutMs, const char *tag)
{
	ThreadRun r;
	r.outcome = THREAD_START_FAILED;
	r.threadId = 0;
	r.elapsedMs = 0;
	RunCtx *c = new RunCtx;
	c->fn = fn;
	c->arg = arg;
	c->tag = tag ? tag : "";
	unsigned id = 0;
	DWORD t0 = GetTickCount();
	HANDLE h = (HANDLE) _beginthreadex(NULL, 0, runThread, c, 0, &id);
	if (h == 0) {
		delete c;
		return r;
	}
	r.threadId = id;
	DWORD w = WaitForSingleObject(h, timeoutMs);
	r.elapsedMs = GetTickCount() - t0;
	if (w == WAIT_OBJECT_0) {
		r.outcome = THREAD_FINISHED;
		// The handle is kept open until the process ends, so the thread id
		// cannot be reused (e.g. by a probe thread) while a test compares
		// lock owners with it.
		delete c;
	}
	else {
		r.outcome = THREAD_BLOCKED;
		InterlockedIncrement(&g_blockedThreads);
		// handle and context are left to the process end on purpose
	}
	return r;
}

int blockedThreadCount()
{
	return (int) g_blockedThreads;
}

const char *threadTag()
{
	return t_threadTag;
}

void setThreadTag(const char *tag)
{
	t_threadTag = tag ? tag : "";
}

DWORD lockOwnerThread(CRITICAL_SECTION *cs)
{
	return (DWORD) (ULONG_PTR) cs->OwningThread;
}

std::vector<UiMessage> &uiMessages()
{
	static std::vector<UiMessage> v;
	return v;
}

long long fileSize(const char *name)
{
	WIN32_FILE_ATTRIBUTE_DATA d;
	if (!GetFileAttributesExA(name, GetFileExInfoStandard, &d))
		return -1;
	return ((long long) d.nFileSizeHigh << 32) | d.nFileSizeLow;
}

size_t readAt(const char *name, long long pos, void *buf, size_t len)
{
	FILE *f = fopen(name, "rb");
	if (!f)
		return 0;
	size_t n = 0;
	if (_fseeki64(f, pos, SEEK_SET) == 0)
		n = fread(buf, 1, len, f);
	fclose(f);
	return n;
}

}  // namespace tp

// ---- UI stubs (replace tputilv2 WRITEERR.obj) ----

namespace tp {
static volatile LONG g_stubCalls = 0;
static const char *volatile g_quietTag = 0;

long stubCallCount()
{
	return g_stubCalls;
}

void setQuietStubTag(const char *tag)
{
	g_quietTag = tag;
}

// Counts the call; true if this call must stay quiet (see TpCore.h).
static bool stubEnter(int kesto)
{
	InterlockedIncrement(&g_stubCalls);
	const char *q = g_quietTag;
	if (q && strcmp(threadTag(), q) == 0) {
		Sleep(kesto > 0 && kesto < 50 ? kesto : 50);
		return true;
	}
	return false;
}
}  // namespace tp

int writeerrorOn;

void writeerror(char *msg, int kesto)
{
	if (tp::stubEnter(kesto))
		return;
	tp::UiMessage m;
	m.text = msg ? msg : "";
	m.kesto = kesto;
	tp::uiMessages().push_back(m);
	tp::info(std::string("writeerror: ") + m.text + tp::fmt(" (kesto %d)", kesto));
}

void writeerror_w(wchar_t *msg, int kesto)
{
	if (tp::stubEnter(kesto))
		return;
	char buf[400];
	int i = 0;
	// ASCII-only transcription; other characters become '?'.
	for (; msg && msg[i] && i < (int) sizeof(buf) - 1; i++)
		buf[i] = (msg[i] >= 32 && msg[i] < 127) ? (char) msg[i] : '?';
	buf[i] = 0;
	tp::UiMessage m;
	m.text = buf;
	m.kesto = kesto;
	tp::uiMessages().push_back(m);
	tp::info(std::string("writeerror_w: ") + m.text + tp::fmt(" (kesto %d)", kesto));
}

// ---- perr stubs (replace tputilv2 perr.obj) ----

void perr(char *szFileName, int line, char *szApiName, unsigned long dwError)
{
	if (tp::stubEnter(0))
		return;
	tp::UiMessage m;
	m.text = tp::fmt("perr: %s line %d %s error %lu", szFileName ? szFileName : "",
		line, szApiName ? szApiName : "", dwError);
	m.kesto = -100;
	tp::uiMessages().push_back(m);
	tp::info(m.text);
}

void wperr(LPWSTR szFileName, int line, LPWSTR szApiName, DWORD dwError)
{
	if (tp::stubEnter(0))
		return;
	char f[200], a[100];
	int i;
	for (i = 0; szFileName && szFileName[i] && i < 199; i++)
		f[i] = (szFileName[i] >= 32 && szFileName[i] < 127) ? (char) szFileName[i] : '?';
	f[i] = 0;
	for (i = 0; szApiName && szApiName[i] && i < 99; i++)
		a[i] = (szApiName[i] >= 32 && szApiName[i] < 127) ? (char) szApiName[i] : '?';
	a[i] = 0;
	tp::UiMessage m;
	m.text = tp::fmt("wperr: %s line %d %s error %lu", f, line, a, (unsigned long) dwError);
	m.kesto = -100;
	tp::uiMessages().push_back(m);
	tp::info(m.text);
}
