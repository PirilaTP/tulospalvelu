/* Linux stubs for functions not applicable on Linux */
#ifdef __linux__
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <windows.h>
#include <tputil.h>
/* HkDeclare.h causes conflicts - define autofileparam type manually */

/* Console display using ANSI escape sequences - narrow output only */
int inv_fore = 0, inv_back = 7, norm_fore = 7, norm_back = 0;

static void wcs_to_utf8(char* dst, const wchar_t* src, int maxlen) {
    int i = 0;
    while (*src && i < maxlen - 4) {
        wchar_t ch = *src++;
        if (ch < 0x80) { dst[i++] = (char)ch; }
        else if (ch < 0x800) { dst[i++] = 0xC0 | (ch >> 6); dst[i++] = 0x80 | (ch & 0x3F); }
        else { dst[i++] = 0xE0 | (ch >> 12); dst[i++] = 0x80 | ((ch >> 6) & 0x3F); dst[i++] = 0x80 | (ch & 0x3F); }
    }
    dst[i] = 0;
}

static void ansi_goto(int row, int col) {
    printf("\033[%d;%dH", row+1, col+1);
}

int vidspmsg(int r, int c, int f, int b, const char* s) {
    ansi_goto(r, c);
    printf("%s", s);
    fflush(stdout);
    return 0;
}
int vidspwmsg(int r, int c, int f, int b, const wchar_t* s) {
    char buf[2048];
    wcs_to_utf8(buf, s, sizeof(buf));
    ansi_goto(r, c);
    printf("%s", buf);
    fflush(stdout);
    return 0;
}
int viwrrect(int r0, int c0, int r1, int c1, const char* s, int f, int b, int m) {
    ansi_goto(r0, c0);
    int maxchars = c1 - c0 + 1;
    const char* src = s ? s : "";
    int printed = 0;
    while (*src && printed < maxchars) { putchar(*src++); printed++; }
    while (printed < maxchars) { putchar(' '); printed++; }
    fflush(stdout);
    return 0;
}
int viwrrectw(int r0, int c0, int r1, int c1, const wchar_t* s, int f, int b, int m) {
    ansi_goto(r0, c0);
    int maxchars = c1 - c0 + 1;
    const wchar_t* src = s ? s : L"";
    int printed = 0;
    char utf8[8];
    while (*src && printed < maxchars) {
        wchar_t ch = *src++;
        int i = 0;
        if (ch < 0x80) { utf8[i++] = (char)ch; }
        else if (ch < 0x800) { utf8[i++] = 0xC0|(ch>>6); utf8[i++] = 0x80|(ch&0x3F); }
        else { utf8[i++] = 0xE0|(ch>>12); utf8[i++] = 0x80|((ch>>6)&0x3F); utf8[i++] = 0x80|(ch&0x3F); }
        utf8[i] = 0;
        fputs(utf8, stdout);
        printed++;
    }
    while (printed < maxchars) { putchar(' '); printed++; }
    fflush(stdout);
    return 0;
}
int virdrect(int r0, int c0, int r1, int c1, char* s, int m) { return 0; }

int scbox(int ur, int uc, int lr, int lc, int t, char c, int a) {
    /* Unicode box-drawing: t=0 single, t=1 double */
    const char* hz  = t ? "═" : "─";
    const char* vt  = t ? "║" : "│";
    const char* tl  = t ? "╔" : "┌";
    const char* tr  = t ? "╗" : "┐";
    const char* bl  = t ? "╚" : "└";
    const char* br  = t ? "╝" : "┘";
    int w = lc - uc - 1;

    ansi_goto(ur, uc); printf("%s", tl);
    for (int i = 0; i < w; i++) printf("%s", hz);
    printf("%s", tr);

    for (int r = ur+1; r < lr; r++) {
        ansi_goto(r, uc); printf("%s", vt);
        ansi_goto(r, lc); printf("%s", vt);
    }

    ansi_goto(lr, uc); printf("%s", bl);
    for (int i = 0; i < w; i++) printf("%s", hz);
    printf("%s", br);
    fflush(stdout);
    return 0;
}

void draw_hline(int r, int c, int n) {
    ansi_goto(r, c);
    for (int i = 0; i < n; i++) printf("─");
    fflush(stdout);
}
void draw_hline2(int r, int c, int n) {
    ansi_goto(r, c);
    for (int i = 0; i < n; i++) printf("═");
    fflush(stdout);
}
void draw_vline(int r, int c, int n) {
    for (int i = 0; i < n; i++) { ansi_goto(r+i, c); printf("│"); }
    fflush(stdout);
}
void draw_vline2(int r, int c, int n) {
    for (int i = 0; i < n; i++) { ansi_goto(r+i, c); printf("║"); }
    fflush(stdout);
}
void draw_grchar(int r, int c, int ch) {
    ansi_goto(r, c);
    switch(ch) {
        case 0xDA: printf("┌"); break; case 0xBF: printf("┐"); break;
        case 0xC0: printf("└"); break; case 0xD9: printf("┘"); break;
        case 0xC4: printf("─"); break; case 0xB3: printf("│"); break;
        case 0xC3: printf("├"); break; case 0xB4: printf("┤"); break;
        case 0xC2: printf("┬"); break; case 0xC1: printf("┴"); break;
        case 0xC5: printf("┼"); break;
        default: printf("+"); break;
    }
    fflush(stdout);
}
void draw_grchar2(int r, int c, int ch) { draw_grchar(r, c, ch); }

/* Print stubs */
int prblocksize = 4096;
PRFILE* openprfile(wchar_t* n, int a, int b, int c, char* d, int e) { return (PRFILE*)0; }
void closeprfile(PRFILE* p) {}
int sendchars(char* s, int n, PRFILE* p) { return 0; }
int sendwchars(wchar_t* s, int n, PRFILE* p) { return 0; }
int sendwline(wchar_t* s, PRFILE* p) { return 0; }
int startdocGDI(PRFILE* p) { return 0; }
int enddocGDI(PRFILE* p) { return 0; }
int startpageGDI(PRFILE* p) { return 0; }
int endpageGDI(PRFILE* p) { return 0; }
int abortdocGDI(PRFILE* p) { return 0; }
int putwfldGDI(wchar_t* s, int a, int b, int c, PRFILE* p) { return 0; }
int selectfontGDI(PRFILE* p, GDIfontTp* f) { return 0; }

/* Windows version stub */
int WinVersion() { return 0; }

/* autofileparam not provided here - it is in HkGlobals.cpp */
wchar_t sarjanimi[1][11] = {L""};

/* Time conversion stub */
char* aikatostr_l2s(char* buf, INT32 aika, long tt0) {
    long t = aika;
    int h = t / 360000;
    t %= 360000;
    int m = t / 6000;
    t %= 6000;
    int s = t / 100;
    int cs = t % 100;
    sprintf(buf, "%d:%02d:%02d.%d", h, m, s, cs/10);
    return buf;
}

/* main not provided here - it's in HkMain.cpp */

#endif

/* Linux entry point: convert args and call wmain */
#include <signal.h>
#include <execinfo.h>
#include <locale.h>
extern int wmain(int argc, wchar_t* argv[]);

static void crash_handler(int sig) {
    void *array[30];
    int size = backtrace(array, 30);
    fprintf(stderr, "\n=== Signal %d ===\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    _exit(1);
}

static struct termios orig_termios;
static void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
    printf("\033[?25h");  /* show cursor */
    printf("\033[0m");    /* reset colors */
    fflush(stdout);
}

int main(int argc, char* argv[]) {
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
    setlocale(LC_ALL, "C.UTF-8");

    /* Put terminal in raw mode */
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(restore_terminal);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;  /* 100ms timeout */
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    printf("\033[2J");     /* clear screen */
    fflush(stdout);

    wchar_t** wargv = new wchar_t*[argc];
    for (int i = 0; i < argc; i++) {
        int len = strlen(argv[i]) + 1;
        wargv[i] = new wchar_t[len];
        mbstowcs(wargv[i], argv[i], len);
    }
    int ret = wmain(argc, wargv);
    for (int i = 0; i < argc; i++) delete[] wargv[i];
    delete[] wargv;
    return ret;
}

/* Additional missing symbols */
int monirivi = 0;
char* inputstr(char* s, unsigned int len, int r, int c, const char* p, char* d, int f) { return s; }
wchar_t* inputwstr(wchar_t* s, unsigned int len, int r, int c, const wchar_t* p, wchar_t* d, int f) { return s; }
