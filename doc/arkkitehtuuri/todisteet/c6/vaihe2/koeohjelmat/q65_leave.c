/* C-6 step 2, Q65: what LeaveCriticalSection does when called by a thread
   that does not own the lock, or once too often by the owner.
   Isolated probe: no production code, no files, no network.
   Mirrors the patterns of H-088 (Leave without Enter), H-089 (owner leaves
   twice) and a non-owner Leave. Build: cl /nologo /W3 q65_leave.c */
#include <windows.h>
#include <stdio.h>

static CRITICAL_SECTION cs;

static void dump(const char *tag)
{
	printf("  %-34s LockCount=%ld RecursionCount=%ld OwningThread=%lu\n", tag,
		cs.LockCount, cs.RecursionCount, (unsigned long)(ULONG_PTR)cs.OwningThread);
}

static DWORD WINAPI th_enter_hold(LPVOID p)
{
	HANDLE release = (HANDLE)p;
	EnterCriticalSection(&cs);
	WaitForSingleObject(release, INFINITE);
	return 0;
}

static DWORD WINAPI th_leave(LPVOID p)
{
	(void)p;
	LeaveCriticalSection(&cs);
	return 0;
}

static DWORD WINAPI th_try(LPVOID p)
{
	(void)p;
	if (TryEnterCriticalSection(&cs)) {
		LeaveCriticalSection(&cs);
		return 1;
	}
	return 0;
}

static DWORD WINAPI th_enter(LPVOID p)
{
	(void)p;
	EnterCriticalSection(&cs);
	LeaveCriticalSection(&cs);
	return 1;
}

static DWORD run(LPTHREAD_START_ROUTINE fn, void *arg, DWORD ms, int *finished)
{
	DWORD code = 0;
	HANDLE h = CreateThread(NULL, 0, fn, arg, 0, NULL);
	*finished = WaitForSingleObject(h, ms) == WAIT_OBJECT_0;
	if (*finished)
		GetExitCodeThread(h, &code);
	return code;
}

int main(void)
{
	int fin;
	DWORD r;
	HANDLE rel;
	OSVERSIONINFOW v;

	ZeroMemory(&v, sizeof(v));
	v.dwOSVersionInfoSize = sizeof(v);
#pragma warning(suppress: 4996)
	GetVersionExW(&v);
	printf("Q65 probe, Windows %lu.%lu build %lu (manifest-less: may report 6.2)\n",
		v.dwMajorVersion, v.dwMinorVersion, v.dwBuildNumber);

	/* S1: non-owner Leave while another thread holds the lock */
	printf("S1 non-owner Leave while held by thread A\n");
	InitializeCriticalSection(&cs);
	rel = CreateEvent(NULL, TRUE, FALSE, NULL);
	CloseHandle(CreateThread(NULL, 0, th_enter_hold, rel, 0, NULL));
	Sleep(200);
	dump("A holds");
	r = run(th_try, NULL, 2000, &fin);
	printf("  third thread TryEnter before bad Leave: %s\n", fin ? (r ? "GOT LOCK" : "refused") : "hung");
	run(th_leave, NULL, 2000, &fin);
	printf("  thread B Leave returned: %s\n", fin ? "yes" : "hung");
	dump("after B Leave");
	r = run(th_try, NULL, 2000, &fin);
	printf("  third thread TryEnter after bad Leave: %s\n", fin ? (r ? "GOT LOCK (exclusion broken)" : "refused") : "hung");
	r = run(th_enter, NULL, 2000, &fin);
	printf("  third thread Enter after bad Leave: %s\n", fin ? "got lock" : "BLOCKED >2 s");
	SetEvent(rel);
	Sleep(200);
	dump("A ended (still holding per its view)");
	r = run(th_enter, NULL, 2000, &fin);
	printf("  fourth thread Enter: %s\n", fin ? "got lock" : "BLOCKED >2 s");

	/* S2: owner enters once and leaves twice (H-089 pattern) */
	printf("S2 owner Enter once, Leave twice\n");
	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);
	LeaveCriticalSection(&cs);
	dump("after first Leave");
	LeaveCriticalSection(&cs);
	dump("after second Leave");
	r = run(th_try, NULL, 2000, &fin);
	printf("  other thread TryEnter: %s\n", fin ? (r ? "got lock" : "REFUSED (lock looks held)") : "hung");
	r = run(th_enter, NULL, 2000, &fin);
	printf("  other thread Enter: %s\n", fin ? "got lock" : "BLOCKED >2 s");
	EnterCriticalSection(&cs);
	printf("  owner thread Enter again: returned\n");
	dump("owner after re-Enter");
	LeaveCriticalSection(&cs);
	r = run(th_enter, NULL, 2000, &fin);
	printf("  other thread Enter after owner Enter/Leave: %s\n", fin ? "got lock" : "BLOCKED >2 s");

	/* S3: Leave on a free lock by a thread that never entered (H-088 lstf path) */
	printf("S3 Leave on a free lock\n");
	InitializeCriticalSection(&cs);
	run(th_leave, NULL, 2000, &fin);
	dump("after stray Leave");
	r = run(th_enter, NULL, 2000, &fin);
	printf("  other thread Enter: %s\n", fin ? "got lock" : "BLOCKED >2 s");
	r = run(th_try, NULL, 2000, &fin);
	printf("  other thread TryEnter: %s\n", fin ? (r ? "got lock" : "REFUSED") : "hung");

	/* S4: outer frame holds the lock, an inner path enters once and leaves
	   twice (H-089 inside an outer hold): is the outer hold lost? */
	printf("S4 outer hold + inner Enter once, Leave twice\n");
	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);          /* outer frame */
	EnterCriticalSection(&cs);          /* inner path */
	LeaveCriticalSection(&cs);
	LeaveCriticalSection(&cs);          /* extra Leave */
	dump("outer frame still 'holds'");
	r = run(th_try, NULL, 2000, &fin);
	printf("  other thread TryEnter during outer hold: %s\n", fin ? (r ? "GOT LOCK (outer hold lost)" : "refused") : "hung");
	LeaveCriticalSection(&cs);          /* outer frame's own Leave */
	dump("after outer Leave");
	r = run(th_enter, NULL, 2000, &fin);
	printf("  other thread Enter afterwards: %s\n", fin ? "got lock" : "BLOCKED >2 s");

	printf("done\n");
	fflush(stdout);
	ExitProcess(0);
	return 0;
}
