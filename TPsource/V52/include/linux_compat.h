/*
 * Linux compatibility header for tulospalvelu
 * Supplements tptype.h (which already defines DWORD, HANDLE, LPVOID)
 */
#ifndef LINUX_COMPAT_H
#define LINUX_COMPAT_H

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

/* Types NOT in tptype.h */
typedef unsigned short USHORT;
typedef unsigned char BYTE;
typedef uint16_t WORD;
typedef long LONG;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#endif
#ifndef BOOL
#define BOOL int
#endif

#define __fastcall
#define __stdcall
#define WINAPI
#define CALLBACK

/* File API stubs */
#define GENERIC_READ    0x80000000
#define GENERIC_WRITE   0x40000000
#define FILE_SHARE_READ 1
#define OPEN_EXISTING   3
#define CREATE_ALWAYS   2
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_FLAG_WRITE_THROUGH 0

static inline HANDLE CreateFile(const char* name, DWORD access, DWORD share,
                                void* sec, DWORD disp, DWORD flags, HANDLE tmpl) {
    int oflags = O_RDWR;
    if (disp == CREATE_ALWAYS) oflags |= O_CREAT | O_TRUNC;
    int fd = open(name, oflags, 0644);
    return (HANDLE)fd;
}

static inline unsigned GetFileSize(HANDLE h, void* high) {
    int fd = (int)h;
    off_t pos = lseek(fd, 0, SEEK_CUR);
    off_t size = lseek(fd, 0, SEEK_END);
    lseek(fd, pos, SEEK_SET);
    return (unsigned)size;
}

#define CloseHandle(h) close((int)(h))
#define ReadFile(h,buf,len,nread,ovl) (*(nread)=read((int)(h),buf,len), *(nread)>=0)
#define WriteFile(h,buf,len,nwrit,ovl) (*(nwrit)=write((int)(h),buf,len), *(nwrit)>=0)
#define SetFilePointer(h,off,hi,meth) lseek((int)(h), off, meth)
#define FILE_BEGIN SEEK_SET
#define FILE_CURRENT SEEK_CUR

#define _read(fd, buf, len) read(fd, buf, len)
#define _write(fd, buf, len) write(fd, buf, len)
#define _lseek(fd, off, whence) lseek(fd, off, whence)
#define _close(fd) close(fd)

/* Console stubs */
#define GetConsoleWindow() ((HANDLE)0)
#define GetSystemMenu(w,b) ((HANDLE)0)
#define DeleteMenu(m,i,f)
#define SC_CLOSE 0xF060
#define MF_BYCOMMAND 0
typedef int (*PHANDLER_ROUTINE)(DWORD);
#define SetConsoleCtrlHandler(h,a) (1)

/* Critical sections */
typedef pthread_mutex_t CRITICAL_SECTION;
#define InitializeCriticalSection(cs) pthread_mutex_init(cs, NULL)
#define EnterCriticalSection(cs) pthread_mutex_lock(cs)
#define LeaveCriticalSection(cs) pthread_mutex_unlock(cs)
#define DeleteCriticalSection(cs) pthread_mutex_destroy(cs)

/* Threads */
static inline uintptr_t _beginthread_linux(void (*func)(void*), unsigned stack, void* arg) {
    pthread_t tid;
    pthread_create(&tid, NULL, (void*(*)(void*))func, arg);
    return (uintptr_t)tid;
}
#define _beginthread(func, stack, arg) _beginthread_linux(func, stack, arg)
#define Sleep(ms) usleep((ms)*1000)

/* Socket API */
typedef struct sockaddr_in SAIN;
#define SA struct sockaddr
#define SAINSIZE sizeof(struct sockaddr_in)
#define SOCKET_ERROR (-1)

typedef struct { int dummy; } WSADATA;
static inline int WSAStartup(WORD ver, WSADATA* d) { return 0; }
static inline int WSACleanup(void) { return 0; }
static inline int WSAGetLastError(void) { return errno; }
static inline void WSASetLastError(int e) { errno = e; }

/* wchar helpers */
static inline FILE* _wfopen(const wchar_t* wn, const wchar_t* wm) {
    char n[512], m[16];
    wcstombs(n, wn, sizeof(n));
    wcstombs(m, wm, sizeof(m));
    return fopen(n, m);
}

#define _wtoi(s) ((int)wcstol(s, NULL, 10))

static inline char* wcstooem(char* dst, const wchar_t* src, int len) {
    wcstombs(dst, src, len); return dst;
}
static inline char* wcstoansi(char* dst, const wchar_t* src, int len) {
    wcstombs(dst, src, len); return dst;
}
static inline wchar_t* ansitowcs(wchar_t* dst, const char* src, int len) {
    int i;
    for (i = 0; i < len && src[i]; i++) dst[i] = (unsigned char)src[i];
    dst[i] = 0;
    return dst;
}

#define stricmp strcasecmp
#define _stricmp strcasecmp
#define _wcsicmp wcscasecmp

/* swprintf: Windows version has no buffer-size parameter */
#include <cstdarg>
static inline int swprintf_win(wchar_t* buf, const wchar_t* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = vswprintf(buf, 65536, fmt, args);
    va_end(args);
    return ret;
}
#define swprintf swprintf_win

/* _ltow / _itow: convert number to wide string */
static inline wchar_t* _ltow(long val, wchar_t* buf, int radix) {
    if (radix == 10) swprintf_win(buf, L"%ld", val);
    else if (radix == 16) swprintf_win(buf, L"%lx", val);
    else swprintf_win(buf, L"%ld", val);
    return buf;
}
static inline wchar_t* _itow(int val, wchar_t* buf, int radix) {
    return _ltow((long)val, buf, radix);
}

/* Wide-char file open */
static inline int _wopen(const wchar_t* wn, int flags, ...) {
    char n[512];
    wcstombs(n, wn, sizeof(n));
    va_list args;
    va_start(args, flags);
    int mode = va_arg(args, int);
    va_end(args);
    return open(n, flags, mode);
}
static inline FILE* _wfdopen(int fd, const wchar_t* wm) {
    char m[16];
    wcstombs(m, wm, sizeof(m));
    return fdopen(fd, m);
}

/* WinVersion - the actual C code declares it as function, don't macro it */

#endif /* __linux__ */
#endif /* LINUX_COMPAT_H */
