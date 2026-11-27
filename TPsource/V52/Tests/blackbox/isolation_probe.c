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

   Isolation probe for run_hkmaali.py. Run by the driver inside the SAME
   AppContainer, sandbox and job as the program under test, BEFORE that
   program is started. It checks that the isolation holds and writes the
   results to probe_result.txt in its working directory:
     console  - a key sent by the driver arrives (console input works)
     cwd      - a file can be created in the sandbox
     outside  - a file can NOT be created at argv[1] (outside the sandbox)
     loopback - a TCP connect to 127.0.0.1:argv[2] (driver's listener) fails
     external - a TCP connect to 192.0.2.1:9 (TEST-NET-1, RFC 5737, no real
                host) fails immediately with WSAEACCES (10013); any other
                error, a refused socket or a timeout is a PROBLEM (C-11
                stage 4: only the firewall's own refusal counts)
     registry - HKCU writes are refused: creating the key
                HKCU\Software\tp_c11_outside_probe and opening HKCU\Software
                for KEY_SET_VALUE both fail with ERROR_ACCESS_DENIED
     external6_info - TCP connect to [2001:db8::1] (RFC 3849), recorded only
   No other network use. Usage:
     isolation_probe <outside-files> <loopback-port> [<tcp-port> <udp-port>]
   The external TCP/UDP destination ports default to 9; the driver passes
   random ports so the firewall's drop log entries can be matched to this
   probe (with the local source ports and UTC times recorded below).
*/
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "advapi32.lib")
#include <ws2tcpip.h>

static FILE *out;

static void res(const char *name, int ok, const char *detail)
{
	fprintf(out, "%s=%s|%s\n", name, ok ? "isolated_or_ok" : "PROBLEM", detail);
	fflush(out);
}

static unsigned short g_lastLocalPort;

static void utcStamp(char *b)
{
	SYSTEMTIME t;
	GetSystemTime(&t);
	sprintf(b, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute,
		t.wSecond, t.wMilliseconds);
}

static int tryConnect(unsigned long addr, unsigned short port, int *err, DWORD *ms)
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		*err = WSAGetLastError();
		*ms = 0;
		return -1;   /* socket refused: blocked */
	}
	u_long nb = 1;
	ioctlsocket(s, FIONBIO, &nb);
	struct sockaddr_in a;
	memset(&a, 0, sizeof(a));
	a.sin_family = AF_INET;
	a.sin_port = htons(port);
	a.sin_addr.s_addr = addr;
	DWORD t0 = GetTickCount();
	int r = connect(s, (struct sockaddr *) &a, sizeof(a));
	*err = r == 0 ? 0 : WSAGetLastError();
	{
		struct sockaddr_in la;
		int ll = sizeof(la);
		g_lastLocalPort = getsockname(s, (struct sockaddr *) &la, &ll) == 0 ? ntohs(la.sin_port) : 0;
	}
	if (r != 0 && *err == WSAEWOULDBLOCK) {
		fd_set w, e;
		FD_ZERO(&w);
		FD_ZERO(&e);
		FD_SET(s, &w);
		FD_SET(s, &e);
		struct timeval tv = { 3, 0 };
		int n = select(0, NULL, &w, &e, &tv);
		if (n == 0)
			*err = WSAETIMEDOUT;
		else if (FD_ISSET(s, &e)) {
			int so = 0, l = sizeof(so);
			getsockopt(s, SOL_SOCKET, SO_ERROR, (char *) &so, &l);
			*err = so;
		}
		else
			*err = 0;   /* connected */
	}
	*ms = GetTickCount() - t0;
	closesocket(s);
	return *err == 0 ? 1 : 0;
}

int main(int argc, char **argv)
{
	char buf[300];
	if (argc < 3)
		return 2;
	out = fopen("probe_result.txt", "w");
	if (!out)
		return 3;
	res("cwd", 1, "probe_result.txt created in the sandbox");

	/* the probe's own token: restricted? integrity level? admin member? */
	{
		HANDLE t = NULL;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &t)) {
			BYTE il[64];
			DWORD l = 0;
			DWORD rid = 0;
			if (GetTokenInformation(t, TokenIntegrityLevel, il, sizeof(il), &l)) {
				PSID sid = ((TOKEN_MANDATORY_LABEL *) il)->Label.Sid;
				rid = *GetSidSubAuthority(sid, *GetSidSubAuthorityCount(sid) - 1);
			}
			BOOL admin = FALSE;
			SID_IDENTIFIER_AUTHORITY nt = SECURITY_NT_AUTHORITY;
			PSID adm = NULL;
			if (AllocateAndInitializeSid(&nt, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
					0, 0, 0, 0, 0, 0, &adm)) {
				CheckTokenMembership(NULL, adm, &admin);
				FreeSid(adm);
			}
			fprintf(out, "token=info|restricted=%d integrity_rid=0x%lx admin_member=%d\n",
				IsTokenRestricted(t) ? 1 : 0, rid, admin ? 1 : 0);
			CloseHandle(t);
		}
	}

	/* console: wait (max 20 s) for one key from the driver */
	HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
	printf("PROBE READY\n");
	fflush(stdout);
	int gotKey = 0;
	DWORD t0 = GetTickCount();
	while (!gotKey && GetTickCount() - t0 < 20000) {
		if (WaitForSingleObject(in, 500) == WAIT_OBJECT_0) {
			INPUT_RECORD r;
			DWORD n = 0;
			if (ReadConsoleInputW(in, &r, 1, &n) && n == 1 && r.EventType == KEY_EVENT &&
				r.Event.KeyEvent.bKeyDown && r.Event.KeyEvent.uChar.UnicodeChar == L'X')
				gotKey = 1;
		}
	}
	res("console", gotKey, gotKey ? "key X received" : "no key received in 20 s");

	/* outside the sandbox: argv[1] is a ';'-separated list of files */
	{
		char list[2048], *p, *ctx = NULL;
		int allRefused = 1;
		char det[1200] = "";
		strncpy(list, argv[1], sizeof(list) - 1);
		list[sizeof(list) - 1] = 0;
		for (p = strtok_s(list, ";", &ctx); p; p = strtok_s(NULL, ";", &ctx)) {
			HANDLE h = CreateFileA(p, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (h != INVALID_HANDLE_VALUE) {
				CloseHandle(h);
				DeleteFileA(p);
				allRefused = 0;
				sprintf(buf, "%s CREATED; ", p);
			}
			else
				sprintf(buf, "%s refused %lu; ", p, GetLastError());
			if (strlen(det) + strlen(buf) < sizeof(det) - 1)
				strcat(det, buf);
		}
		res("outside", allRefused, det);
	}

	WSADATA w;
	int wsa = WSAStartup(MAKEWORD(2, 2), &w);
	sprintf(buf, "WSAStartup=%d", wsa);
	res("wsastartup", 1, buf);
	int err;
	DWORD ms;
	int r = tryConnect(htonl(INADDR_LOOPBACK), (unsigned short) atoi(argv[2]), &err, &ms);
	sprintf(buf, "connect 127.0.0.1:%s -> %s, error %d after %lu ms", argv[2],
		r == 1 ? "CONNECTED" : "failed", err, ms);
	res("loopback", r != 1, buf);
	unsigned short tcpPort = argc > 3 ? (unsigned short) atoi(argv[3]) : 9;
	unsigned short udpPort = argc > 4 ? (unsigned short) atoi(argv[4]) : 9;
	char ts[40];
	utcStamp(ts);
	r = tryConnect(inet_addr("192.0.2.1"), tcpPort, &err, &ms);
	fprintf(out, "tcp_sent=%s|dst=192.0.2.1:%u|src_port=%u\n", ts, tcpPort, g_lastLocalPort);
	/* isolated: refused at once with WSAEACCES (10013) only */
	int blocked = r == 0 && err == WSAEACCES && ms < 1000;
	sprintf(buf, "connect 192.0.2.1:%u -> %s, error %d after %lu ms", tcpPort, r == 1 ? "CONNECTED" : "failed", err, ms);
	res("external", blocked, buf);
	/* UDP: a single datagram to TEST-NET-1; blocked means sendto is refused
	   with WSAEACCES (no datagram leaves the machine). */
	{
		SOCKET u = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		int uerr = 0, ur = -1;
		if (u == INVALID_SOCKET)
			uerr = WSAGetLastError();
		else {
			struct sockaddr_in a;
			memset(&a, 0, sizeof(a));
			a.sin_family = AF_INET;
			a.sin_port = htons(udpPort);
			a.sin_addr.s_addr = inet_addr("192.0.2.1");
			utcStamp(ts);
			ur = sendto(u, "x", 1, 0, (struct sockaddr *) &a, sizeof(a));
			uerr = ur < 0 ? WSAGetLastError() : 0;
			{
				struct sockaddr_in la;
				int ll = sizeof(la);
				unsigned short lp = getsockname(u, (struct sockaddr *) &la, &ll) == 0 ? ntohs(la.sin_port) : 0;
				fprintf(out, "udp_sent=%s|dst=192.0.2.1:%u|src_port=%u\n", ts, udpPort, lp);
			}
			closesocket(u);
		}
		/* sendto returning success proves nothing (the firewall drops UDP
		   silently); the driver decides from the firewall drop log. */
		sprintf(buf, "udp sendto 192.0.2.1:%u returned %s, error %d (see firewall log)", udpPort,
			ur >= 0 ? "success" : "failure", uerr);
		res("udp_sendto_info", 1, buf);
	}
	/* IPv6, information only: a host without IPv6 connectivity fails
	   with another error before the firewall is involved. */
	{
		SOCKET s6 = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
		int e6 = 0;
		if (s6 == INVALID_SOCKET)
			e6 = WSAGetLastError();
		else {
			struct sockaddr_in6 a6;
			memset(&a6, 0, sizeof(a6));
			a6.sin6_family = AF_INET6;
			a6.sin6_port = htons(tcpPort);
			inet_pton(AF_INET6, "2001:db8::1", &a6.sin6_addr);
			e6 = connect(s6, (struct sockaddr *) &a6, sizeof(a6)) == 0 ? 0 : WSAGetLastError();
			closesocket(s6);
		}
		sprintf(buf, "connect [2001:db8::1]:%u -> error %d", tcpPort, e6);
		res("external6_info", 1, buf);
	}
	WSACleanup();

	/* HKCU writes must be refused (HKCU grants RESTRICTED read only). */
	{
		HKEY h = NULL;
		DWORD disp = 0;
		LONG c1 = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\tp_c11_outside_probe", 0, NULL, 0,
			KEY_WRITE, NULL, &h, &disp);
		LONG c2;
		if (c1 == ERROR_SUCCESS) {
			RegCloseKey(h);
			RegDeleteKeyW(HKEY_CURRENT_USER, L"Software\\tp_c11_outside_probe");
		}
		h = NULL;
		c2 = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software", 0, KEY_SET_VALUE, &h);
		if (c2 == ERROR_SUCCESS)
			RegCloseKey(h);
		sprintf(buf, "RegCreateKeyEx HKCU\\Software\\tp_c11_outside_probe -> %ld; "
			"RegOpenKeyEx HKCU\\Software KEY_SET_VALUE -> %ld", c1, c2);
		res("registry", c1 == ERROR_ACCESS_DENIED && c2 == ERROR_ACCESS_DENIED, buf);
	}
	fprintf(out, "done=1\n");
	fclose(out);
	return 0;
}
