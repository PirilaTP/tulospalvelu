/* Linux stub for WinHTTP.h */
#ifndef _WINHTTP_STUB_H
#define _WINHTTP_STUB_H
#ifdef __linux__
typedef void* HINTERNET;
typedef const wchar_t* LPCWSTR;
#define WINHTTP_ACCESS_TYPE_NO_PROXY 0
#define WINHTTP_NO_PROXY_NAME NULL
#define WINHTTP_NO_PROXY_BYPASS NULL
#define WINHTTP_FLAG_SECURE 0x00800000
#define WINHTTP_NO_REFERER NULL
#define WINHTTP_DEFAULT_ACCEPT_TYPES NULL
#define WINHTTP_NO_ADDITIONAL_HEADERS NULL
#define WINHTTP_NO_REQUEST_DATA NULL
#define WINHTTP_QUERY_CONTENT_LENGTH 5
#define WINHTTP_QUERY_FLAG_NUMBER 0x20000000
static inline HINTERNET WinHttpOpen(LPCWSTR a, int b, LPCWSTR c, LPCWSTR d, int e) { return NULL; }
static inline HINTERNET WinHttpConnect(HINTERNET h, LPCWSTR s, int p, int r) { return NULL; }
static inline HINTERNET WinHttpOpenRequest(HINTERNET h, LPCWSTR v, LPCWSTR p, LPCWSTR ver, LPCWSTR ref, LPCWSTR* acc, int f) { return NULL; }
static inline int WinHttpSendRequest(HINTERNET h, LPCWSTR hdr, int hdrlen, void* opt, int optlen, int totlen, int ctx) { return 0; }
static inline int WinHttpReceiveResponse(HINTERNET h, void* r) { return 0; }
static inline int WinHttpQueryDataAvailable(HINTERNET h, unsigned long* s) { return 0; }
static inline int WinHttpReadData(HINTERNET h, void* b, unsigned long s, unsigned long* r) { return 0; }
static inline int WinHttpQueryHeaders(HINTERNET h, unsigned long i, void* n, void* b, unsigned long* bs, unsigned long* idx) { return 0; }
static inline int WinHttpCloseHandle(HINTERNET h) { return 0; }
static inline int WinHttpSetTimeouts(HINTERNET h, int a, int b, int c, int d) { return 0; }
#endif
#endif
