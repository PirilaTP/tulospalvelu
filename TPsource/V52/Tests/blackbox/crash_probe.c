/* Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
   Copyright (C) 2015 Pekka Pirila

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Deliberately crashing control program for the debugger mode of
   run_hkmaali.py (C-11 stage 4, doc/arkkitehtuuri/testaus.md section 19).
   Built as x86 with /GS and frame pointers, like the shipped HkMaali.exe,
   so the driver's debugger, breakpoint and stack-cookie code can be
   checked on a known crash before the real program is run. No network,
   no files outside its working directory (child.txt only).

   Modes (argv[1]):
     gs N       victim(N): zeroes N bytes of a 64-byte stack buffer. N 64
                fits (control); N 104 overwrites the /GS cookie, saved EBP,
                return address and more with zeros, as HkMaali's addfrec
                does with the record tail -> __report_gsfailure ->
                __fastfail(2) -> 0xC0000409.
     fastfail   __fastfail(FAST_FAIL_STACK_COOKIE_CHECK_FAILURE) directly.
     av         write to address 0: unhandled access violation.
     child      starts "crash_probe sleep 60000" as a child, writes the
                result to child.txt, then crashes as "gs 104".
     sleep N    sleeps N ms, exit code 0.
*/
#include <windows.h>
#include <intrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Keeps the buffer alive so /O2 cannot drop the writes. */
__declspec(noinline) static int sink(volatile char *p, int n)
{
	int s = 0, i;
	for (i = 0; i < n && i < 64; i++)
		s += p[i];
	return s;
}

/* Same shape as kilptietue::addfrec: a char stack buffer filled by a loop
   whose length comes from elsewhere. */
__declspec(noinline) int victim(int n)
{
	char buf[64];
	volatile char *p = buf;
	int i;
	for (i = 0; i < n; i++)
		p[i] = 0;
	return sink(buf, n);
}

static int child(void)
{
	wchar_t self[MAX_PATH], cmd[MAX_PATH + 40];
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	FILE *f;
	BOOL ok;
	DWORD err;
	GetModuleFileNameW(NULL, self, MAX_PATH);
	swprintf(cmd, MAX_PATH + 40, L"\"%s\" sleep 60000", self);
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	ok = CreateProcessW(self, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	err = ok ? 0 : GetLastError();   /* before fopen changes it */
	f = fopen("child.txt", "w");
	if (f) {
		if (ok)
			fprintf(f, "created pid=%lu\n", pi.dwProcessId);
		else
			fprintf(f, "failed error=%lu\n", err);
		fclose(f);
	}
	if (ok) {
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	return victim(104);
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return 2;
	if (!strcmp(argv[1], "gs") && argc > 2)
		return victim(atoi(argv[2]));
	if (!strcmp(argv[1], "fastfail"))
		__fastfail(2);
	if (!strcmp(argv[1], "av")) {
		*(volatile int *) 0 = 1;
		return 0;
	}
	if (!strcmp(argv[1], "child"))
		return child();
	if (!strcmp(argv[1], "sleep") && argc > 2) {
		Sleep((DWORD) atoi(argv[2]));
		return 0;
	}
	return 2;
}
