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
// probe thread could not be run. A probe from the owning thread would always
// succeed because CRITICAL_SECTION is recursive.
int lockFreeForOtherThread(CRITICAL_SECTION *cs);

// Messages captured by the writeerror/writeerror_w/perr/wperr stubs
// (TpCore.cpp). A perr call is recorded with kesto = -100.
struct UiMessage {
	std::string text;
	int kesto;
};
std::vector<UiMessage> &uiMessages();

// Size of a file in bytes, -1 if it cannot be opened.
long long fileSize(const char *name);

// Reads len bytes at offset pos. Returns the number of bytes read.
size_t readAt(const char *name, long long pos, void *buf, size_t len);

}  // namespace tp

#endif
