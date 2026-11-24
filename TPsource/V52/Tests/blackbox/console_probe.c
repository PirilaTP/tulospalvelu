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

   Diagnostic probe for run_hkmaali.py: repeats, in the same AppContainer
   and console set-up, the console calls the program makes at start-up
   (tputilv2/initcon.cpp:49-84, a screen write and a Beep) and records the
   result of each call in console_probe.txt, instead of opening a
   MessageBox like PERR does. Not part of the program under test.
*/
#include <windows.h>
#include <stdio.h>

static FILE *out;

static void rec(const char *what, BOOL ok)
{
	fprintf(out, "%s=%s|error %lu\n", what, ok ? "ok" : "FAILED", ok ? 0UL : GetLastError());
	fflush(out);
}

int main(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD mode = 0, n = 0;
	out = fopen("console_probe.txt", "w");
	if (!out)
		return 3;
	HANDLE ho = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hi = GetStdHandle(STD_INPUT_HANDLE);
	BOOL b = GetConsoleScreenBufferInfo(ho, &csbi);
	rec("GetConsoleScreenBufferInfo", b);
	if (b)
		fprintf(out, "cursor=%d,%d size=%d,%d\n", csbi.dwCursorPosition.X, csbi.dwCursorPosition.Y,
			csbi.dwSize.X, csbi.dwSize.Y);
	rec("SetConsoleOutputCP(850)", SetConsoleOutputCP(850));
	rec("SetConsoleTitle", SetConsoleTitleA("console probe"));
	b = GetConsoleMode(hi, &mode);
	rec("GetConsoleMode(stdin)", b);
	rec("SetConsoleMode(stdin)", SetConsoleMode(hi, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT |
		ENABLE_PROCESSED_INPUT | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)));
	COORD c = { 0, 24 };
	b = WriteConsoleOutputCharacterW(ho, L"CONSOLE PROBE TEXT", 18, c, &n);
	rec("WriteConsoleOutputCharacterW", b && n == 18);
	SMALL_RECT r = { 0, 0, 79, 49 };
	COORD sz = { 80, 50 };
	rec("SetConsoleScreenBufferSize(80x50)", SetConsoleScreenBufferSize(ho, sz));
	rec("SetConsoleWindowInfo", SetConsoleWindowInfo(ho, TRUE, &r));
	/* Reads and writes as tputilv2/viwrrectw.cpp, Viwrrect.cpp, virdrect.cpp
	   and the full-buffer clear of scpclr.cpp:29-40 (2 calls per row). */
	{
		CHAR_INFO ci[80];
		SMALL_RECT rr = { 0, 3, 79, 3 };
		COORD bs = { 80, 1 }, bc = { 0, 0 };
		rec("ReadConsoleOutputW", ReadConsoleOutputW(ho, ci, bs, bc, &rr));
		rr.Left = 0; rr.Top = 3; rr.Right = 79; rr.Bottom = 3;
		rec("ReadConsoleOutputA", ReadConsoleOutputA(ho, ci, bs, bc, &rr));
		for (int i = 0; i < 80; i++) {
			ci[i].Char.UnicodeChar = i < 16 ? L"CONSOLE PROBE W2"[i] : L' ';
			ci[i].Attributes = 0x0F;
		}
		rr.Left = 0; rr.Top = 3; rr.Right = 79; rr.Bottom = 3;
		rec("WriteConsoleOutputW", WriteConsoleOutputW(ho, ci, bs, bc, &rr));
		CONSOLE_SCREEN_BUFFER_INFO c2;
		GetConsoleScreenBufferInfo(ho, &c2);
		DWORD t1 = GetTickCount(), w;
		BOOL okc = TRUE;
		for (int y = 0; y < c2.dwSize.Y; y++) {
			COORD at = { 0, (SHORT) y };
			okc &= FillConsoleOutputCharacterW(ho, L' ', c2.dwSize.X, at, &w);
			okc &= FillConsoleOutputAttribute(ho, 0x0F, c2.dwSize.X, at, &w);
		}
		fprintf(out, "ClearAllRows=%s|%d rows in %lu ms\n", okc ? "ok" : "FAILED", c2.dwSize.Y, GetTickCount() - t1);
		fflush(out);
		COORD c5 = { 0, 5 };
		b = WriteConsoleOutputCharacterW(ho, L"CONSOLE PROBE CLEARED", 21, c5, &n);
		rec("WriteAfterClear", b && n == 21);
	}
	DWORD t0 = GetTickCount();
	b = Beep(600, 150);
	fprintf(out, "Beep=%s|%lu ms\n", b ? "ok" : "FAILED", GetTickCount() - t0);
	fprintf(out, "done=1\n");
	fclose(out);
	Sleep(1500);   /* keep the text on screen for the driver */
	return 0;
}
