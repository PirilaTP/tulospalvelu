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

// main() of the core component test programs TpCoreHk and TpCoreJuk.
// The only file that compiles the doctest implementation. It switches off
// every Windows and CRT error dialog, so a crash ends the process with an
// exit code the runner can record instead of waiting for a user.
// TP_CORE_NAME is defined by the project (TpCoreHk / TpCoreJuk), unquoted.

#define DOCTEST_CONFIG_IMPLEMENT
// doctest must not catch crashes itself: a crash ends the process with its
// real exit code, and only the evidence protocol in TpCrash.cpp (checked by
// the runner) can turn a crash into a result.
#define DOCTEST_CONFIG_NO_WINDOWS_SEH
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#include "../doctest.h"

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <crtdbg.h>
#include <iostream>
#ifdef __SANITIZE_ADDRESS__
#include <sanitizer/asan_interface.h>
#endif
#include "../fixtures/TpCore.h"

#define TP_STR2(x) #x
#define TP_STR(x) TP_STR2(x)

#ifndef TP_CORE_NAME
#define TP_CORE_NAME TpCore
#endif

static void quietErrors()
{
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
}

// Build facts for the result log. The runner runs this test case once per
// executable and stores the TPINFO lines.
TEST_CASE("tp_build_info")
{
	tp::info("program=" TP_STR(TP_CORE_NAME));
#ifdef _MSC_FULL_VER
	tp::info(tp::fmt("msc_full_ver=%d", (int) _MSC_FULL_VER));
#endif
#ifdef _DEBUG
	tp::info("config=Debug");
#else
	tp::info("config=Release");
#endif
#ifdef __SANITIZE_ADDRESS__
	tp::info("asan=1");
#else
	tp::info("asan=0");
#endif
#ifdef _CHAR_UNSIGNED
	tp::info("char_unsigned=1");
#else
	tp::info("char_unsigned=0");
#endif
#ifdef MAXOSUUSLUKU
	tp::info("MAXOSUUSLUKU=" TP_STR(MAXOSUUSLUKU));
#endif
#ifdef VAIKALUKU
	tp::info("VAIKALUKU=" TP_STR(VAIKALUKU));
#endif
#ifdef MAXSARJALUKU
	tp::info("MAXSARJALUKU=" TP_STR(MAXSARJALUKU));
#endif
	tp::info(tp::fmt("sizeof_void_ptr=%d", (int) sizeof(void *)));
	CHECK(true);
}

int main(int argc, char **argv)
{
	quietErrors();
	// Unbuffered output, so check failures printed before a deliberate
	// crash reach the runner.
	setvbuf(stdout, NULL, _IONBF, 0);
	std::cout << std::unitbuf;
#ifdef __SANITIZE_ADDRESS__
	__asan_set_error_report_callback(tp::asanReportCallback);
#endif
	doctest::Context context;
	context.setOption("no-colors", true);
	context.applyCommandLine(argc, argv);
	int res = context.run();
	fflush(stdout);
	fflush(stderr);
	// A test may leave a production thread blocked on a lock on purpose.
	// Normal exit would run static destructors and DLL detach while that
	// thread holds locks, so end the process directly; the doctest summary
	// is already printed and the runner checks the count against the
	// manifest.
	int blocked = tp::blockedThreadCount();
	if (blocked > 0) {
		printf("TPEXIT|blocked_threads=%d|mode=terminate\n", blocked);
		fflush(stdout);
		TerminateProcess(GetCurrentProcess(), (UINT) res);
	}
	return res;
}
