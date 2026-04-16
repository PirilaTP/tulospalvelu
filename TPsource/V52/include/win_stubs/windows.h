/* Linux stub for windows.h */
#ifndef _WINDOWS_H_STUB
#define _WINDOWS_H_STUB

#ifdef __linux__

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cwchar>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <cstdarg>

/* Basic types */
typedef unsigned short USHORT;
typedef unsigned char BYTE;
typedef uint16_t WORD;
typedef int BOOL;
typedef long LONG;
typedef unsigned long ULONG;
typedef unsigned long DWORD;
typedef void* LPVOID;
typedef int HANDLE;
typedef int (*PHANDLER_ROUTINE)(unsigned long);

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define INVALID_HANDLE_VALUE ((int)-1)

#ifndef cdecl
#define cdecl
#endif
#define __fastcall
#define __stdcall
#define WINAPI
#define CALLBACK

typedef wchar_t WCHAR;
typedef wchar_t TCHAR;
typedef wchar_t* LPWSTR;
typedef char* LPSTR;
typedef short SHORT;
typedef void* HWND;
typedef void* HMENU;
typedef void* HFONT;
typedef unsigned long COLORREF;
#define TEXT(x) L##x

static inline int linux_GetCurrentDirectoryW(int n, wchar_t* b) {
    char tmp[512];
    if (getcwd(tmp, sizeof(tmp))) { mbstowcs(b, tmp, n); return 1; }
    return 0;
}
#define GetCurrentDirectory(n,b) linux_GetCurrentDirectoryW(n,b)
#define GetCurrentDirectoryW GetCurrentDirectory
#define GetMenuItemCount(m) 0
#define GetMenuStringW(m,i,b,n,f) 0
#define RemoveMenu(m,i,f) 0
#define MF_BYPOSITION 0x400
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((unsigned long)(BYTE)(b))<<16)))

/* File API */
#define GENERIC_READ    0x80000000UL
#define GENERIC_WRITE   0x40000000UL
#define FILE_SHARE_READ 1
#define OPEN_EXISTING   3
#define CREATE_ALWAYS   2
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_FLAG_WRITE_THROUGH 0
#define FILE_BEGIN SEEK_SET
#define FILE_CURRENT SEEK_CUR
#define ERROR_FILE_NOT_FOUND 2

static inline int linux_CreateFile(const char* name, unsigned long access, unsigned long share,
                                   void* sec, unsigned long disp, unsigned long flags, int tmpl) {
    int oflags = O_RDWR;
    if (disp == CREATE_ALWAYS) oflags |= O_CREAT | O_TRUNC;
    return open(name, oflags, 0644);
}
#define CreateFile(n,a,s,sec,d,f,t) linux_CreateFile(n,a,s,sec,d,f,t)

static inline unsigned linux_GetFileSize(int h, void* high) {
    off_t pos = lseek(h, 0, SEEK_CUR);
    off_t size = lseek(h, 0, SEEK_END);
    lseek(h, pos, SEEK_SET);
    return (unsigned)size;
}
#define GetFileSize(h,p) linux_GetFileSize(h,p)
#define CloseHandle(h) close(h)
#define SetFilePointer(h,off,hi,meth) lseek(h, off, meth)
#define _read(fd,buf,len) read(fd,buf,len)
#define _write(fd,buf,len) write(fd,buf,len)
#define _lseek(fd,off,w) lseek(fd,off,w)
#define _close(fd) close(fd)
#define _open(n,f,m) open(n,f,m)

/* Console stubs */
#define GetConsoleWindow() (0)
#define GetSystemMenu(w,b) (0)
#define DeleteMenu(m,i,f)
#define SC_CLOSE 0xF060
#define MF_BYCOMMAND 0
#define SetConsoleCtrlHandler(h,a) (1)

/* SYSTEMTIME */
typedef struct {
    unsigned short wYear, wMonth, wDayOfWeek, wDay;
    unsigned short wHour, wMinute, wSecond, wMilliseconds;
} SYSTEMTIME;
static inline void GetSystemTime(SYSTEMTIME* st) {
    struct timeval tv; gettimeofday(&tv, NULL);
    struct tm* t = gmtime(&tv.tv_sec);
    st->wYear = t->tm_year+1900; st->wMonth = t->tm_mon+1;
    st->wDayOfWeek = t->tm_wday; st->wDay = t->tm_mday;
    st->wHour = t->tm_hour; st->wMinute = t->tm_min;
    st->wSecond = t->tm_sec; st->wMilliseconds = tv.tv_usec/1000;
}
typedef struct { long Bias; wchar_t StandardName[32]; wchar_t DaylightName[32]; } TIME_ZONE_INFORMATION;
static inline int GetTimeZoneInformation(TIME_ZONE_INFORMATION* tz) {
    time_t t = time(NULL); struct tm* lt = localtime(&t);
    tz->Bias = -lt->tm_gmtoff/60;
    return 0;
}
static inline void GetLocalTime(SYSTEMTIME* st) {
    struct timeval tv; gettimeofday(&tv, NULL);
    struct tm* t = localtime(&tv.tv_sec);
    st->wYear = t->tm_year+1900; st->wMonth = t->tm_mon+1;
    st->wDayOfWeek = t->tm_wday; st->wDay = t->tm_mday;
    st->wHour = t->tm_hour; st->wMinute = t->tm_min;
    st->wSecond = t->tm_sec; st->wMilliseconds = tv.tv_usec/1000;
}

/* Console input */
#define KEY_EVENT 1
#define LEFT_CTRL_PRESSED 0x0008
#define RIGHT_CTRL_PRESSED 0x0004
#define LEFT_ALT_PRESSED 0x0002
#define RIGHT_ALT_PRESSED 0x0001
#define SHIFT_PRESSED 0x0010
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_CAPITAL 0x14
#define VK_ESCAPE 0x1B
#define VK_RETURN 0x0D
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_DELETE 0x2E
#define VK_INSERT 0x2D
#define VK_HOME 0x24
#define VK_END 0x23
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_UP 0x26
#define VK_DOWN 0x28
#define VK_LEFT 0x25
#define VK_RIGHT 0x27
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_PROCESSKEY 0xE5
#define ReadConsoleInputA ReadConsoleInput
typedef struct {
    unsigned short EventType;
    union { struct {
        int bKeyDown;
        unsigned short wRepeatCount;
        unsigned short wVirtualKeyCode;
        unsigned short wVirtualScanCode;
        union { wchar_t UnicodeChar; char AsciiChar; } uChar;
        unsigned long dwControlKeyState;
    } KeyEvent; } Event;
} INPUT_RECORD;

typedef struct { short X; short Y; } COORD;
typedef struct { short Left; short Top; short Right; short Bottom; } SMALL_RECT;
typedef unsigned short CHAR_INFO;
typedef struct { COORD dwSize; COORD dwCursorPosition; unsigned short wAttributes; SMALL_RECT srWindow; COORD dwMaximumWindowSize; } CONSOLE_SCREEN_BUFFER_INFO;
#define SetConsoleWindowInfo(h,a,r) (1)
#define SetConsoleScreenBufferSize(h,s) (1)
typedef struct { int dwSize; int bVisible; } CONSOLE_CURSOR_INFO;
#define GetConsoleCursorInfo(h,p) (0)
extern int _linux_cursor_row, _linux_cursor_col;
static inline int linux_GetConsoleScreenBufferInfo(int h, CONSOLE_SCREEN_BUFFER_INFO* p) {
    memset(p, 0, sizeof(*p));
    p->dwSize.X = 80; p->dwSize.Y = 25;
    p->dwCursorPosition.X = _linux_cursor_col;
    p->dwCursorPosition.Y = _linux_cursor_row;
    p->srWindow.Right = 79; p->srWindow.Bottom = 24;
    p->dwMaximumWindowSize.X = 80; p->dwMaximumWindowSize.Y = 25;
    return 1;
}
#define GetConsoleScreenBufferInfo(h,p) linux_GetConsoleScreenBufferInfo(h,p)
#define GetLargestConsoleWindowSize(h) ((COORD){80,25})
extern void shadow_put(int row, int col, char ch);
static inline int linux_WriteConsoleOutputCharacterW(int h, const wchar_t* b, int n, COORD c, unsigned long* w) {
    printf("\033[%d;%dH", c.Y+1, c.X+1);
    for (int i = 0; i < n; i++) {
        wchar_t ch = b[i];
        if (ch < 0x80) putchar((char)ch);
        else if (ch < 0x800) { putchar(0xC0|(ch>>6)); putchar(0x80|(ch&0x3F)); }
        else { putchar(0xE0|(ch>>12)); putchar(0x80|((ch>>6)&0x3F)); putchar(0x80|(ch&0x3F)); }
        shadow_put(c.Y, c.X + i, (char)(ch & 0xFF));
    }
    fflush(stdout);
    if (w) *w = n;
    return 1;
}
#define WriteConsoleOutputCharacterW(h,b,n,c,w) linux_WriteConsoleOutputCharacterW(h,b,n,c,w)
#define WriteConsoleOutputAttribute(h,a,n,c,w) (1)
#define ScrollConsoleScreenBufferW(h,s,c,d,f) (1)
static inline int linux_SetConsoleCursorPosition(int h, COORD c) {
    _linux_cursor_row = c.Y;
    _linux_cursor_col = c.X;
    printf("\033[%d;%dH", c.Y+1, c.X+1);
    fflush(stdout);
    return 1;
}
#define SetConsoleCursorPosition(h,c) linux_SetConsoleCursorPosition(h,c)
static inline int linux_FillConsoleOutputCharacterW(int h, wchar_t c, int n, COORD co, unsigned long* w) {
    printf("\033[%d;%dH", co.Y+1, co.X+1);
    for (int i = 0; i < n; i++) {
        if (c < 0x80) putchar((char)c);
        else if (c < 0x800) { putchar(0xC0|(c>>6)); putchar(0x80|(c&0x3F)); }
        else { putchar(0xE0|(c>>12)); putchar(0x80|((c>>6)&0x3F)); putchar(0x80|(c&0x3F)); }
        shadow_put(co.Y, co.X + i, (char)(c & 0xFF));
    }
    fflush(stdout);
    if (w) *w = n;
    return 1;
}
#define FillConsoleOutputCharacterW(h,c,n,co,w) linux_FillConsoleOutputCharacterW(h,c,n,co,w)
#define FillConsoleOutputCharacter FillConsoleOutputCharacterW
#define FillConsoleOutputAttribute(h,a,n,co,w) (1)
#define SetConsoleTextAttribute(h,a)
#include <termios.h>
#include <poll.h>
static inline int linux_ReadConsoleInput(int h, INPUT_RECORD* rec, int n, unsigned long* nr) {
    /* Terminal should already be in raw mode (set by main) */
    /* Block until data is available using poll */
    struct pollfd rpfd = { STDIN_FILENO, POLLIN, 0 };
    while (poll(&rpfd, 1, 100) <= 0) { /* 100ms poll loop */ }
    unsigned char ch;
    int r = read(STDIN_FILENO, &ch, 1);
    if (r <= 0) { if (nr) *nr = 0; return 1; }
    memset(rec, 0, sizeof(INPUT_RECORD));
    rec->EventType = KEY_EVENT;
    rec->Event.KeyEvent.bKeyDown = 1;
    rec->Event.KeyEvent.wRepeatCount = 1;
    /* Map escape sequences for arrow keys etc */
    if (ch == 27) {
        unsigned char seq[4] = {0};
        read(STDIN_FILENO, &seq[0], 1);
        read(STDIN_FILENO, &seq[1], 1);
        if (seq[0] == '[') {
            switch(seq[1]) {
                case 'A': rec->Event.KeyEvent.wVirtualKeyCode = VK_UP; break;
                case 'B': rec->Event.KeyEvent.wVirtualKeyCode = VK_DOWN; break;
                case 'C': rec->Event.KeyEvent.wVirtualKeyCode = VK_RIGHT; break;
                case 'D': rec->Event.KeyEvent.wVirtualKeyCode = VK_LEFT; break;
                case 'H': rec->Event.KeyEvent.wVirtualKeyCode = VK_HOME; break;
                case 'F': rec->Event.KeyEvent.wVirtualKeyCode = VK_END; break;
                case '5': rec->Event.KeyEvent.wVirtualKeyCode = VK_PRIOR; read(STDIN_FILENO,&seq[2],1); break;
                case '6': rec->Event.KeyEvent.wVirtualKeyCode = VK_NEXT; read(STDIN_FILENO,&seq[2],1); break;
                case '3': rec->Event.KeyEvent.wVirtualKeyCode = VK_DELETE; read(STDIN_FILENO,&seq[2],1); break;
                default: rec->Event.KeyEvent.uChar.UnicodeChar = 27; break;
            }
        } else {
            rec->Event.KeyEvent.uChar.UnicodeChar = 27;
        }
    } else if (ch == '\r' || ch == '\n') {
        rec->Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
        rec->Event.KeyEvent.wVirtualScanCode = 28;
        rec->Event.KeyEvent.uChar.UnicodeChar = '\r';
    } else if (ch == 127 || ch == 8) {
        rec->Event.KeyEvent.wVirtualKeyCode = VK_BACK;
        rec->Event.KeyEvent.wVirtualScanCode = 14;
        rec->Event.KeyEvent.uChar.UnicodeChar = 8;
    } else if (ch == '\t') {
        rec->Event.KeyEvent.wVirtualKeyCode = VK_TAB;
        rec->Event.KeyEvent.wVirtualScanCode = 15;
        rec->Event.KeyEvent.uChar.UnicodeChar = '\t';
    } else if (ch >= 1 && ch <= 26) {
        rec->Event.KeyEvent.uChar.UnicodeChar = ch;
        rec->Event.KeyEvent.dwControlKeyState = LEFT_CTRL_PRESSED;
    } else {
        rec->Event.KeyEvent.uChar.UnicodeChar = (wchar_t)ch;
    }
    if (nr) *nr = 1;
    return 1;
}
#define ReadConsoleInput(h,r,n,nr) linux_ReadConsoleInput(h,r,n,nr)
#define ReadConsoleInputW ReadConsoleInput
static inline int linux_PeekConsoleInput(int h, INPUT_RECORD* rec, int n, unsigned long* nr) {
    struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
    int ready = poll(&pfd, 1, 10); /* 10ms wait to reduce CPU usage */
    if (nr) *nr = (ready > 0 && (pfd.revents & POLLIN)) ? 1 : 0;
    return 1;
}
#define PeekConsoleInput(h,r,n,nr) linux_PeekConsoleInput(h,r,n,nr)
#define PeekConsoleInputW PeekConsoleInput
#define GetNumberOfConsoleInputEvents(h,n) (*(n)=0,1)
#define FlushConsoleInputBuffer(h)
#define SetConsoleTitleW(t) (1)
#define SetConsoleTitle SetConsoleTitleW
#define GetStdHandle(n) (n)
#define STD_INPUT_HANDLE  0
#define STD_OUTPUT_HANDLE 1
#define STD_ERROR_HANDLE  2

/* Critical sections */
typedef pthread_mutex_t CRITICAL_SECTION;
#define InitializeCriticalSection(cs) pthread_mutex_init(cs, NULL)
#define EnterCriticalSection(cs) pthread_mutex_lock(cs)
#define LeaveCriticalSection(cs) pthread_mutex_unlock(cs)
#define DeleteCriticalSection(cs) pthread_mutex_destroy(cs)

/* Threads - accept both void(*)(void*) and void*(*)(void*) signatures */
static inline uintptr_t linux_beginthread(void (*func)(void*), unsigned stack, void* arg) {
    pthread_t tid;
    pthread_create(&tid, NULL, (void*(*)(void*))func, arg);
    return (uintptr_t)tid;
}
static inline uintptr_t linux_beginthread(void* (*func)(void*), unsigned stack, void* arg) {
    pthread_t tid;
    pthread_create(&tid, NULL, func, arg);
    return (uintptr_t)tid;
}
#define _beginthread(func, stack, arg) linux_beginthread(func, stack, arg)
#define Sleep(ms) usleep((unsigned)(ms)*1000)

/* Socket API */
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define closesocket close
#ifndef SA
typedef struct sockaddr_in SAIN;
#define SA struct sockaddr
#define SAINSIZE sizeof(struct sockaddr_in)
#endif
#define SOCKET_ERROR (-1)

#ifndef _WINUDP_WSA_DEFINED
typedef struct { int dummy; } WSADATA;
#endif
int WSAStartup(int laji, WSADATA *WSAdata);
void WSACleanup(void);
int WSAGetLastError(void);
void WSASetLastError(int iError);
#define MAKEWORD(a,b) ((WORD)(((BYTE)(a))|(((WORD)((BYTE)(b)))<<8)))
#define INADDR_NONE ((unsigned long)0xFFFFFFFF)

/* MessageBox stubs */
#define MB_YESNO 0
#define IDYES 6
#define IDNO 7
static inline int MessageBoxW(void* h, const wchar_t* t, const wchar_t* c, unsigned f) { return IDNO; }
static inline int MessageBoxA(void* h, const char* t, const char* c, unsigned f) { return IDNO; }
#define MB_OK 0
#define MB_SETFOREGROUND 0x10000
#define MB_TASKMODAL 0x2000
#define MB_ICONEXCLAMATION 0x30
#define MAKELANGID(p,s) 0
#define LANG_NEUTRAL 0
#define SUBLANG_DEFAULT 0
#define SUBLANG_ENGLISH_US 0
typedef wchar_t* LPTSTR;
static inline void MessageBeep(unsigned t) { }

/* WSA error codes mapped to POSIX */
#define WSAEACCES EACCES
#define WSAEISCONN EISCONN
#define WSAEADDRINUSE EADDRINUSE
#define WSAEADDRNOTAVAIL EADDRNOTAVAIL
#define WSAEINVAL EINVAL
#define WSAENOTSOCK ENOTSOCK
#define WSAECONNREFUSED ECONNREFUSED
#define WSAETIMEDOUT ETIMEDOUT
#define WSAECONNRESET ECONNRESET
#define WSAENETUNREACH ENETUNREACH
#define WSAEHOSTUNREACH EHOSTUNREACH
#define WSAESHUTDOWN ESHUTDOWN
#define WSAEALREADY EALREADY
#define WSAEHOSTDOWN EHOSTDOWN
#define WSAEINPROGRESS EINPROGRESS
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAENOTCONN ENOTCONN
#define WSAECONNABORTED ECONNABORTED
#define WSAEMSGSIZE EMSGSIZE
#define WSAENOPROTOOPT ENOPROTOOPT
#define WSAEPROTONOSUPPORT EPROTONOSUPPORT
#define WSAEOPNOTSUPP EOPNOTSUPP
#define WSAEPFNOSUPPORT EPFNOSUPPORT
#define WSAEAFNOSUPPORT EAFNOSUPPORT
#define WSAEDESTADDRREQ EDESTADDRREQ
#define WSAENETDOWN ENETDOWN
#define WSAENETRESET ENETRESET

/* wchar helpers */
static inline FILE* _wfopen(const wchar_t* wn, const wchar_t* wm) {
    char n[512], m[16];
    wcstombs(n, wn, sizeof(n));
    wcstombs(m, wm, sizeof(m));
    return fopen(n, m);
}
static inline int _wfopen_s(FILE** fp, const wchar_t* wn, const wchar_t* wm) {
    *fp = _wfopen(wn, wm);
    return *fp ? 0 : errno;
}
#define _wtoi(s) ((int)wcstol(s, NULL, 10))
#define _wtol(s) wcstol(s, NULL, 10)
#define _wtof(s) wcstof(s, NULL)

static inline char* wcstooem(char* d, const wchar_t* s, int n) { wcstombs(d,s,n); return d; }
static inline char* wcstoansi(char* d, const wchar_t* s, int n) { wcstombs(d,s,n); return d; }
static inline wchar_t* ansitowcs(wchar_t* d, const char* s, int n) {
    /* Direct ISO-8859-1 byte to wchar_t (don't use mbstowcs - locale-dependent) */
    int i;
    for (i = 0; i < n && s[i]; i++) d[i] = (unsigned char)s[i];
    d[i] = 0;
    return d;
}

static inline int _wrename(const wchar_t* o, const wchar_t* n) {
    char oa[512], na[512];
    wcstombs(oa, o, sizeof(oa));
    wcstombs(na, n, sizeof(na));
    return rename(oa, na);
}

#define stricmp strcasecmp
#define _stricmp strcasecmp
#define _wcsicmp wcscasecmp

/* wcstok: Linux requires 3 args, Windows uses 2 */
static inline wchar_t* wcstok_2arg(wchar_t* s, const wchar_t* d) {
    static wchar_t* _state;
    return wcstok(s, d, &_state);
}
#define wcstok(s,d) wcstok_2arg(s,d)

/* Missing functions */
#include <wctype.h>
#define _itoa(v,b,r) sprintf(b,"%d",v)
#define _ltoa(v,b,r) sprintf(b,"%ld",(long)(v))
static inline wchar_t* _itow_fn(int v, wchar_t* b, int r) {
    if (r == 16) swprintf(b, 20, L"%x", v);
    else swprintf(b, 20, L"%d", v);
    return b;
}
#define _itow(v,b,r) _itow_fn(v,b,r)
static inline wchar_t* _ltow(long v, wchar_t* b, int r) {
    if (r == 16) swprintf(b, 20, L"%lx", v);
    else swprintf(b, 20, L"%ld", v);
    return b;
}
typedef long long __int64;

/* Console events */
#define CTRL_C_EVENT 0
#define CTRL_BREAK_EVENT 1
#define CTRL_CLOSE_EVENT 2
#define CTRL_LOGOFF_EVENT 5
#define CTRL_SHUTDOWN_EVENT 6

#include <climits>
#include <sys/stat.h>

static inline long _filelength(int fd) {
    struct stat st;
    if (fstat(fd, &st) == 0) return st.st_size;
    return -1;
}
#define _chsize(fd, size) ftruncate(fd, size)
#define _swab(src, dst, n) swab(src, dst, n)
#define _atoi64(s) atoll(s)

/* FormatMessage stub */
#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x100
#define FORMAT_MESSAGE_FROM_SYSTEM 0x1000
static inline unsigned long FormatMessage(unsigned long f, const void* s, unsigned long m,
    unsigned long l, wchar_t* b, unsigned long n, void* a) {
    if (f & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
        static wchar_t _fmtbuf[256];
        swprintf(_fmtbuf, 256, L"Error %lu", m);
        *(wchar_t**)b = _fmtbuf;
    } else if (b && n > 0) {
        swprintf(b, n, L"Error %lu", m);
    }
    return 1;
}
#define FormatMessageW FormatMessage
#define LocalFree(p)
#define GetLastError() errno
#define GetCurrentProcess() 0
#define ZeroMemory(p,n) memset(p,0,n)
#define WINHTTP_ACCESS_TYPE_DEFAULT_PROXY 0

/* Serial port stubs */
typedef struct {
    unsigned long DCBlength;
    unsigned long BaudRate;
    int fRtsControl, fDtrControl, fOutxCtsFlow, fOutxDsrFlow;
    int fInX, fOutX;
    unsigned char ByteSize, Parity, StopBits;
} DCB;
#define FillMemory(p,n,v) memset(p,v,n)
#define BuildCommDCBW BuildCommDCB
#define GetCommTimeouts(h,t) (0)
#define MAXDWORD 0xFFFFFFFF
#define RTS_CONTROL_DISABLE 0
#define RTS_CONTROL_ENABLE 1
#define DTR_CONTROL_ENABLE 1
#define DTR_CONTROL_DISABLE 0
#define EV_CTS 0x0008
#define MS_CTS_ON 0x0010
#define NOPARITY 0
#define ONESTOPBIT 0
#define CBR_9600 9600
static inline int GetCommState(int h, DCB* d) { return 0; }
static inline int SetCommState(int h, DCB* d) { return 0; }
static inline int SetCommMask(int h, unsigned long m) { return 0; }
static inline int WaitCommEvent(int h, unsigned long* e, void* o) { return 0; }
static inline int GetCommModemStatus(int h, unsigned long* s) { return 0; }
static inline int BuildCommDCB(const wchar_t* s, DCB* d) { return 0; }
static inline int SetupComm(int h, int i, int o) { return 0; }
typedef struct { int ReadIntervalTimeout; int ReadTotalTimeoutMultiplier; int ReadTotalTimeoutConstant; int WriteTotalTimeoutMultiplier; int WriteTotalTimeoutConstant; } COMMTIMEOUTS;
static inline int SetCommTimeouts(int h, COMMTIMEOUTS* t) { return 0; }
static inline int PurgeComm(int h, unsigned long f) { return 0; }
#define PURGE_RXCLEAR 0x0008
#define PURGE_TXCLEAR 0x0004

/* COMSTAT for serial port */
typedef struct { unsigned long cbInQue; unsigned long cbOutQue; } COMSTAT;
static inline int ClearCommError(int h, unsigned long* e, COMSTAT* s) {
    if (s) { s->cbInQue = 0; s->cbOutQue = 0; }
    if (e) *e = 0;
    return 1;
}
#define CreateFileW(n,a,s,sec,d,f,t) linux_CreateFile("",a,s,sec,d,f,t)
static inline int ReadFile(int h, void* buf, unsigned long len, unsigned long* nread, void* ovl) {
    int r = read(h, buf, len); if (nread) *nread = r > 0 ? r : 0; return r >= 0;
}
static inline int WriteFile(int h, const void* buf, unsigned long len, unsigned long* nwrit, void* ovl) {
    int r = write(h, buf, len); if (nwrit) *nwrit = r > 0 ? r : 0; return r >= 0;
}
#define EscapeCommFunction(h,f) (0)
#define SETDTR 5
#define CLRDTR 6
#define SETRTS 3
#define CLRRTS 4

/* Wide-char file open */
static inline int _wopen(const wchar_t* wn, int flags, ...) {
    char n[512];
    wcstombs(n, wn, sizeof(n));
    return open(n, flags, 0644);
}
static inline FILE* _wfdopen(int fd, const wchar_t* wm) {
    char m[16];
    wcstombs(m, wm, sizeof(m));
    return fdopen(fd, m);
}

/* 64-bit seek - macOS lseek is already 64-bit, Linux needs lseek64 */
#ifdef __APPLE__
#define _lseeki64(fd, off, w) lseek(fd, off, w)
#else
#define _lseeki64(fd, off, w) lseek64(fd, off, w)
#endif

#define _snwprintf swprintf

/*
 * swprintf: Windows uses swprintf(buf, fmt, ...) without buffer size.
 * C99/Linux requires swprintf(buf, size, fmt, ...).
 * Redefine AFTER all internal uses of C99 swprintf above.
 */
static inline int swprintf_win(wchar_t* buf, const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vswprintf(buf, 65536, fmt, args);
    va_end(args);
    return ret;
}
/* C99 overload: swprintf(buf, size, fmt, ...) */
static inline int swprintf_win(wchar_t* buf, size_t n, const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vswprintf(buf, n, fmt, args);
    va_end(args);
    return ret;
}
#define swprintf swprintf_win
#undef _snwprintf
#define _snwprintf swprintf_win
#define wsprintf swprintf_win
#define wsprintfW swprintf_win

/* Charset conversion - own UTF-8 decoder, locale-independent */
#define CP_UTF8 65001
static inline int MultiByteToWideChar(unsigned cp, unsigned fl, const char* mb, int mblen,
    wchar_t* wc, int wclen) {
    const unsigned char* s = (const unsigned char*)mb;
    int slen = (mblen == -1) ? (int)strlen(mb)+1 : mblen;
    if (wclen == 0) return slen; /* rough estimate */
    int i = 0, o = 0;
    if (cp == CP_UTF8) {
        /* UTF-8 decoding */
        while (i < slen && o < wclen - 1) {
            unsigned char c = s[i];
            if (c == 0) { break; }
            else if (c < 0x80) { wc[o++] = c; i++; }
            else if ((c & 0xE0) == 0xC0 && i+1 < slen) {
                wc[o++] = ((c & 0x1F) << 6) | (s[i+1] & 0x3F); i += 2;
            } else if ((c & 0xF0) == 0xE0 && i+2 < slen) {
                wc[o++] = ((c & 0x0F) << 12) | ((s[i+1] & 0x3F) << 6) | (s[i+2] & 0x3F); i += 3;
            } else { wc[o++] = c; i++; }
        }
    } else {
        /* ISO-8859-1 / single-byte: direct byte-to-wchar_t mapping */
        while (i < slen && o < wclen - 1) {
            unsigned char c = s[i++];
            if (c == 0) break;
            wc[o++] = c;
        }
    }
    wc[o] = 0;
    return o;
}
static inline int WideCharToMultiByte(unsigned cp, unsigned fl, const wchar_t* wc, int wclen,
    char* mb, int mblen, const char* def, int* used) {
    if (mblen == 0) { return wclen > 0 ? wclen*4 : (int)wcslen(wc)*4+1; }
    int i = 0, o = 0;
    int slen = (wclen == -1) ? (int)wcslen(wc)+1 : wclen;
    while (i < slen && o < mblen - 4) {
        wchar_t ch = wc[i++];
        if (ch == 0) break;
        if (ch < 0x80) { mb[o++] = (char)ch; }
        else if (ch < 0x800) { mb[o++] = 0xC0|(ch>>6); mb[o++] = 0x80|(ch&0x3F); }
        else { mb[o++] = 0xE0|(ch>>12); mb[o++] = 0x80|((ch>>6)&0x3F); mb[o++] = 0x80|(ch&0x3F); }
    }
    mb[o] = 0;
    return o;
}

static inline void Beep(int freq, int dur) { /* no-op on Linux */ }
#define GetVersion() 0
#define SetConsoleOutputCP(cp) (1)
#define FreeConsole() (1)
#define AllocConsole() (1)
#define GetConsoleMode(h,m) (*(m)=0,1)
#define SetConsoleMode(h,m) (1)
#define ENABLE_LINE_INPUT 0x0002
#define ENABLE_PROCESSED_INPUT 0x0001
#define ENABLE_ECHO_INPUT 0x0004
#define ENABLE_WINDOW_INPUT 0x0008
#define ENABLE_MOUSE_INPUT 0x0010
#define WriteConsoleW(h,b,n,w,r) (*(w)=n,1)
#define ReadConsoleW(h,b,n,r,p) (*(r)=0,0)

#define GetModuleHandle(x) ((void*)0)
#define GetProcAddress(h,n) ((void*)0)

#endif /* __linux__ */
#endif /* _WINDOWS_H_STUB */
