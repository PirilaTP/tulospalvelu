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
int _linux_cursor_row = 0, _linux_cursor_col = 0;

/* Shadow screen buffer: mirrors what's on the terminal so virdrect can read it back */
#define SHADOW_ROWS 50
#define SHADOW_COLS 80
static char shadow[SHADOW_ROWS][SHADOW_COLS];

void shadow_init() {
    memset(shadow, ' ', sizeof(shadow));
}
void shadow_put(int row, int col, char ch) {
    if (row >= 0 && row < SHADOW_ROWS && col >= 0 && col < SHADOW_COLS)
        shadow[row][col] = ch;
}
static void shadow_puts(int row, int col, const char* s, int maxcols) {
    /* Handle UTF-8: store first byte of each char, skip continuation bytes */
    int c = 0;  /* column counter */
    for (int i = 0; s[i] && (maxcols < 0 || c < maxcols); i++) {
        unsigned char ch = (unsigned char)s[i];
        if (ch < 0x80) {
            shadow_put(row, col + c, ch);
            c++;
        } else if ((ch & 0xC0) == 0xC0) {
            /* UTF-8 start byte — store the ISO-8859-1 equivalent if 2-byte */
            /* Decode: 0xC3 0xA4 = ä = 0xE4 in ISO-8859-1 */
            if ((ch & 0xE0) == 0xC0 && s[i+1]) {
                unsigned char b2 = (unsigned char)s[i+1];
                shadow_put(row, col + c, (char)(((ch & 0x1F) << 6) | (b2 & 0x3F)));
            } else {
                shadow_put(row, col + c, '?');
            }
            c++;
        }
        /* else: continuation byte (0x80-0xBF) — skip, don't increment column */
    }
}
static void shadow_clear_row(int row, int c0, int c1) {
    for (int c = c0; c <= c1 && c < SHADOW_COLS; c++)
        shadow_put(row, c, ' ');
}
void shadow_clear() { shadow_init(); }

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
    _linux_cursor_row = row;
    _linux_cursor_col = col;
    printf("\033[%d;%dH", row+1, col+1);
}

int vidspmsg(int r, int c, int f, int b, const char* s) {
    ansi_goto(r, c);
    printf("%s", s);  /* source strings are already UTF-8 */
    shadow_puts(r, c, s, -1);
    fflush(stdout);
    return 0;
}
int vidspwmsg(int r, int c, int f, int b, const wchar_t* s) {
    char buf[2048];
    wcs_to_utf8(buf, s, sizeof(buf));
    ansi_goto(r, c);
    printf("%s", buf);
    /* Store ISO-8859-1 version in shadow (truncate wchar_t to byte) */
    for (int i = 0; s[i]; i++)
        shadow_put(r, c + i, (char)(s[i] & 0xFF));
    fflush(stdout);
    return 0;
}
int viwrrect(int r0, int c0, int r1, int c1, const char* s, int f, int b, int m) {
    int cols = c1 - c0 + 1;
    const char* src = s ? s : "";
    for (int row = r0; row <= r1; row++) {
        ansi_goto(row, c0);
        int printed = 0;
        while (*src && printed < cols) {
            unsigned char ch = (unsigned char)*src;
            if (ch < 0x80) {
                /* ASCII — same in both encodings */
                putchar(ch); shadow_put(row, c0 + printed, ch);
                src++; printed++;
            } else if (ch >= 0xC0 && ch < 0xE0 && ((unsigned char)src[1] & 0xC0) == 0x80) {
                /* Valid UTF-8 2-byte sequence — pass through, decode for shadow */
                unsigned char b2 = (unsigned char)src[1];
                putchar(ch); putchar(b2);
                shadow_put(row, c0 + printed, (char)(((ch & 0x1F) << 6) | (b2 & 0x3F)));
                src += 2; printed++;
            } else if (ch >= 0xE0 && ch < 0xF0 && ((unsigned char)src[1] & 0xC0) == 0x80) {
                /* Valid UTF-8 3-byte — pass through */
                putchar(ch); putchar(src[1]); putchar(src[2]);
                shadow_put(row, c0 + printed, '?');
                src += 3; printed++;
            } else {
                /* ISO-8859-1 byte — convert to UTF-8 */
                putchar(0xC0 | (ch >> 6)); putchar(0x80 | (ch & 0x3F));
                shadow_put(row, c0 + printed, ch);
                src++; printed++;
            }
        }
        while (printed < cols) { putchar(' '); shadow_put(row, c0 + printed, ' '); printed++; }
    }
    fflush(stdout);
    return 0;
}

static void put_wchar_utf8(wchar_t ch) {
    if (ch < 0x80) { putchar((char)ch); }
    else if (ch < 0x800) { putchar(0xC0|(ch>>6)); putchar(0x80|(ch&0x3F)); }
    else { putchar(0xE0|(ch>>12)); putchar(0x80|((ch>>6)&0x3F)); putchar(0x80|(ch&0x3F)); }
}

int viwrrectw(int r0, int c0, int r1, int c1, const wchar_t* s, int f, int b, int m) {
    int cols = c1 - c0 + 1;
    const wchar_t* src = s ? s : L"";
    for (int row = r0; row <= r1; row++) {
        ansi_goto(row, c0);
        int printed = 0;
        while (*src && printed < cols) { put_wchar_utf8(*src++); printed++; }
        while (printed < cols) { putchar(' '); printed++; }
    }
    fflush(stdout);
    return 0;
}
int virdrect(int r0, int c0, int r1, int c1, char* s, int m) {
    int cols = c1 - c0 + 1;
    for (int row = r0; row <= r1; row++) {
        for (int col = 0; col < cols; col++) {
            int sr = row, sc = c0 + col;
            *s++ = (sr >= 0 && sr < SHADOW_ROWS && sc >= 0 && sc < SHADOW_COLS)
                   ? shadow[sr][sc] : ' ';
        }
    }
    return 0;
}

void clrln(int row) {
    ansi_goto(row, 0);
    printf("\033[2K");
    shadow_clear_row(row, 0, SHADOW_COLS - 1);
    fflush(stdout);
}

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
    for (int i = 0; i < n; i++) { printf("─"); shadow_put(r, c+i, '-'); }
    fflush(stdout);
}
void draw_hline2(int r, int c, int n) {
    ansi_goto(r, c);
    for (int i = 0; i < n; i++) { printf("═"); shadow_put(r, c+i, '='); }
    fflush(stdout);
}
void draw_vline(int r, int c, int n) {
    for (int i = 0; i < n; i++) { ansi_goto(r+i, c); printf("│"); shadow_put(r+i, c, '|'); }
    fflush(stdout);
}
void draw_vline2(int r, int c, int n) {
    for (int i = 0; i < n; i++) { ansi_goto(r+i, c); printf("║"); shadow_put(r+i, c, '|'); }
    fflush(stdout);
}
void draw_grchar(int r, int c, int ch) {
    ansi_goto(r, c);
    shadow_put(r, c, '+');
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
static PRFILE _linux_dummy_prfile;
PRFILE* openprfile(wchar_t* n, int a, int b, int c, char* d, int e) {
    memset(&_linux_dummy_prfile, 0, sizeof(_linux_dummy_prfile));
    return &_linux_dummy_prfile;
}
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
    if (!setlocale(LC_ALL, "C.UTF-8"))
        setlocale(LC_ALL, "C.utf8");

    /* Put terminal in raw mode */
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(restore_terminal);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;  /* 100ms timeout */
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    printf("\033[2J");     /* clear screen */
    printf("\033[?25h");  /* show cursor */
    shadow_init();
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

/* Shared globals (also used by Inputstr.cpp, Pageedit.cpp) */
int monirivi = 0;
/* inputstr is now compiled from Inputstr.cpp */
/* inputwstr delegates to the real inputwstr2 in Inputwstr.cpp */
extern wchar_t* inputwstr2(wchar_t* s, unsigned l, int x, int y, const wchar_t* term, wchar_t* tc, int numfl);
wchar_t* inputwstr(wchar_t* s, unsigned int len, int r, int c, const wchar_t* p, wchar_t* d, int f) {
    return inputwstr2(s, len, r, c, p, d, f);
}
