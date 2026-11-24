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

// Shared helpers for the core component tests (TpCoreHk, TpCoreJuk).
// See doc/arkkitehtuuri/testaus.md.
//
// Protocol with the runner (Tests/run_core_tests.py):
//  - Every test case runs in its own process, with a fresh temporary
//    directory as the current directory. The runner writes a marker file
//    ".tp_sandbox" there and passes the same token in the environment
//    variable TP_SANDBOX_TOKEN. tp::sandboxOk() must be REQUIREd before any
//    file operation, so production code can only touch that directory.
//  - A defect test prints exactly one line
//      TPRESULT|finding=H-nnn|mechanism=<id>|observed=<0|1>|detail=<text>
//    observed=1 means the specific defect mechanism was seen. The doctest
//    assertions in the test only check preconditions; a crash, a timeout or
//    a failed precondition is never evidence of the defect.
//  - TPSKIP|reason=<text> marks a test whose prerequisites are missing.

#ifndef TP_CORE_TEST_H
#define TP_CORE_TEST_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

namespace tp {

// True when the current directory holds the runner's marker file and its
// content equals TP_SANDBOX_TOKEN.
bool sandboxOk();

// Prints the TPRESULT line (see above).
void reportResult(const char *finding, const char *mechanism, bool observed,
	const std::string &detail);

// Prints a TPSKIP line.
void reportSkip(const std::string &reason);

// Prints a free-form TPINFO line for the result log.
void info(const std::string &text);

// Formatting helper (printf style, max 1000 chars).
std::string fmt(const char *format, ...);

// Tries to enter cs from a separate thread and leaves it again on success.
// Returns 1 if the other thread got the lock, 0 if it did not, -1 if the
// probe thread could not be run, -2 if the probe thread happens to be the
// recorded owner (thread id reuse; the result would be meaningless). A probe from the owning thread would always
// succeed because CRITICAL_SECTION is recursive.
int lockFreeForOtherThread(CRITICAL_SECTION *cs);

// Messages captured by the writeerror/writeerror_w/perr/wperr stubs
// (TpCore.cpp). A perr call is recorded with kesto = -100.
struct UiMessage {
	std::string text;
	int kesto;
};
std::vector<UiMessage> &uiMessages();

// ---- Controlled threads (TpCore.cpp) ----
// The main thread never waits without a limit for production code that may
// block. fn runs on a new thread; arg must be heap or static (a blocked
// thread may outlive the caller's frame). THREAD_BLOCKED registers the
// thread; CoreMain then ends the process with TerminateProcess after the
// doctest summary and prints TPEXIT|blocked_threads=N.
enum ThreadOutcome { THREAD_START_FAILED = -1, THREAD_FINISHED = 0, THREAD_BLOCKED = 1 };
typedef void (*ThreadFn)(void *arg);
struct ThreadRun {
	ThreadOutcome outcome;
	DWORD threadId;
	DWORD elapsedMs;
};
// The thread handle is never closed, so its id stays unique for the rest
// of the process.
ThreadRun runOnThread(ThreadFn fn, void *arg, DWORD timeoutMs, const char *tag = "");
int blockedThreadCount();
// Tag of the current thread set by runOnThread ("" on other threads).
const char *threadTag();
// Sets the tag of the current thread (for threads not started by
// runOnThread).
void setThreadTag(const char *tag);
// Owner thread id of a CRITICAL_SECTION (internal field OwningThread).
DWORD lockOwnerThread(CRITICAL_SECTION *cs);

// ---- Loopback helpers (TpNet.cpp; 127.0.0.1 only) ----
// A free TCP port on 127.0.0.1 (bind to port 0, read it, close; no listen).
// 0 if none could be obtained.
unsigned short freeLoopbackPort();
// Reserves a 127.0.0.1 port that refuses connections: the socket stays
// bound (not listening, SO_EXCLUSIVEADDRUSE) until releaseHeldPort, so no
// other process can take the port meanwhile. 0 on failure.
unsigned short holdRefusingLoopbackPort();
void releaseHeldPort();
// Attempt-counting listener on 127.0.0.1 (one at a time). Uses
// SO_CONDITIONAL_ACCEPT, so every connection attempt is seen and either
// rejected (phase 1) or accepted and closed (phase 2). Returns the port, 0
// on failure.
unsigned short startAttemptListener();
void attemptListenerAccept(bool accept);     // switch phase
int attemptListenerCount(int phase);         // attempts seen in phase 1 or 2
void stopAttemptListener();
// Connects to 127.0.0.1:port once; 0 on success, else the WSA error code.
int loopbackConnect(unsigned short port);

// ---- Crash evidence (TpCrash.cpp) ----
// armCrash prints TPARM. Afterwards a crash is reported by the ASan report
// callback (ASan builds) or by the vectored exception handler installed with
// installExceptionEvidence (non-ASan builds), which print TPCRASH/TPRESULT
// lines. disarmCrash prints TPDISARM; a test may report observed=0 only after
// disarming. The runner checks the same evidence independently.
void armCrash(const char *finding, const char *mechanism, const char *source,
	const char *const *expectSubstrings = 0);
void disarmCrash();
struct ExceptionExpect {
	DWORD code;              // e.g. EXCEPTION_ACCESS_VIOLATION
	int access;              // 0 read, 1 write, 8 execute
	ULONG_PTR target;        // faulting data address
	const char *threadTag;   // only exceptions on this tagged thread count
	const char *symbol;      // function expected in the symbol chain
};
// Initialises DbgHelp (symbols from the exe directory only) and installs the
// handler. Returns false if not possible (then the test must not arm).
bool installExceptionEvidence(const ExceptionExpect &e);
#ifdef __SANITIZE_ADDRESS__
void asanReportCallback(const char *report);
#endif

// Size of a file in bytes, -1 if it cannot be opened.
long long fileSize(const char *name);

// Reads len bytes at offset pos. Returns the number of bytes read.
size_t readAt(const char *name, long long pos, void *buf, size_t len);

}  // namespace tp

#endif
