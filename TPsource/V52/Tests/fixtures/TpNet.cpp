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

// Loopback-only socket helpers for the core tests (TpCore.h). Every socket
// here is bound to 127.0.0.1; nothing listens on other interfaces, so no
// firewall prompt and no traffic outside the machine.
// winsock2.h must come before windows.h, so this file does not include
// TpCore.h first.

#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include "TpCore.h"

namespace tp {

static bool wsaUp()
{
	static bool ok = false;
	if (!ok) {
		WSADATA w;
		ok = WSAStartup(MAKEWORD(2, 2), &w) == 0;
	}
	return ok;
}

static void loopback(sockaddr_in *a, unsigned short port)
{
	memset(a, 0, sizeof(*a));
	a->sin_family = AF_INET;
	a->sin_port = htons(port);
	a->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
}

unsigned short freeLoopbackPort()
{
	if (!wsaUp())
		return 0;
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return 0;
	sockaddr_in a;
	loopback(&a, 0);
	unsigned short port = 0;
	if (bind(s, (sockaddr *) &a, sizeof(a)) == 0) {
		int l = sizeof(a);
		if (getsockname(s, (sockaddr *) &a, &l) == 0)
			port = ntohs(a.sin_port);
	}
	closesocket(s);
	return port;
}

static SOCKET g_held = INVALID_SOCKET;

unsigned short holdRefusingLoopbackPort()
{
	if (!wsaUp() || g_held != INVALID_SOCKET)
		return 0;
	g_held = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_held == INVALID_SOCKET)
		return 0;
	BOOL on = TRUE;
	setsockopt(g_held, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char *) &on, sizeof(on));
	sockaddr_in a;
	loopback(&a, 0);
	int l = sizeof(a);
	if (bind(g_held, (sockaddr *) &a, sizeof(a)) != 0 || getsockname(g_held, (sockaddr *) &a, &l) != 0) {
		closesocket(g_held);
		g_held = INVALID_SOCKET;
		return 0;
	}
	return ntohs(a.sin_port);   // bound, never listen(): connects are refused
}

void releaseHeldPort()
{
	if (g_held != INVALID_SOCKET) {
		closesocket(g_held);
		g_held = INVALID_SOCKET;
	}
}

// ---- attempt-counting listener ----

static SOCKET g_ls = INVALID_SOCKET;
static HANDLE g_lth = 0;
static volatile LONG g_phase = 1;       // 1 reject, 2 accept
static volatile LONG g_count[3];

static int CALLBACK acceptCondition(LPWSABUF, LPWSABUF, LPQOS, LPQOS, LPWSABUF, LPWSABUF,
	GROUP *, DWORD_PTR)
{
	LONG ph = g_phase;
	InterlockedIncrement(&g_count[ph]);
	return ph == 1 ? CF_REJECT : CF_ACCEPT;
}

static unsigned __stdcall acceptLoop(void *)
{
	for (;;) {
		SOCKET c = WSAAccept(g_ls, NULL, NULL, acceptCondition, 0);
		if (c != INVALID_SOCKET) {
			closesocket(c);
			continue;
		}
		int e = WSAGetLastError();
		if (e == WSAECONNREFUSED || e == WSATRY_AGAIN || e == WSAECONNRESET)
			continue;           // rejected attempt (phase 1)
		break;                  // listener closed
	}
	return 0;
}

unsigned short startAttemptListener()
{
	if (!wsaUp() || g_ls != INVALID_SOCKET)
		return 0;
	g_count[0] = g_count[1] = g_count[2] = 0;
	g_phase = 1;
	g_ls = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_ls == INVALID_SOCKET)
		return 0;
	BOOL on = TRUE;
	setsockopt(g_ls, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char *) &on, sizeof(on));
	setsockopt(g_ls, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (const char *) &on, sizeof(on));
	sockaddr_in a;
	loopback(&a, 0);
	int l = sizeof(a);
	if (bind(g_ls, (sockaddr *) &a, sizeof(a)) != 0 || listen(g_ls, 8) != 0 ||
		getsockname(g_ls, (sockaddr *) &a, &l) != 0) {
		closesocket(g_ls);
		g_ls = INVALID_SOCKET;
		return 0;
	}
	g_lth = (HANDLE) _beginthreadex(NULL, 0, acceptLoop, NULL, 0, NULL);
	return ntohs(a.sin_port);
}

void attemptListenerAccept(bool accept)
{
	InterlockedExchange(&g_phase, accept ? 2 : 1);
}

int attemptListenerCount(int phase)
{
	return (phase == 1 || phase == 2) ? (int) g_count[phase] : -1;
}

int loopbackConnect(unsigned short port)
{
	if (!wsaUp())
		return -1;
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return WSAGetLastError();
	sockaddr_in a;
	loopback(&a, port);
	int r = connect(s, (sockaddr *) &a, sizeof(a)) == 0 ? 0 : WSAGetLastError();
	closesocket(s);
	return r;
}

void stopAttemptListener()
{
	if (g_ls != INVALID_SOCKET) {
		closesocket(g_ls);
		g_ls = INVALID_SOCKET;
	}
	if (g_lth) {
		WaitForSingleObject(g_lth, 5000);
		CloseHandle(g_lth);
		g_lth = 0;
	}
}

}  // namespace tp
