/* C-9 step 2: run-time library probe for the C++Builder classic compiler
 * (bcc32), the compiler of HkKisaWin and ViestiWin. Not production code:
 * it answers what the bcc32 RTL itself does (Q83, Q84, Q85), with the same
 * RTL linkage as the GUI programs (dynamic RTL, multithreaded).
 * Build: rtl_bcc32.cmd. Run in an empty directory: rtl_bcc32.exe <case>
 *   weof    fputs/fputws return value when the write fails (Q83)
 *   nfile   how many FILE streams can be open at once (Q84)
 *   dfree   free() of an already freed block (Q85)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>

static void weof_case(void)
{
	FILE *f;
	int r;
	wint_t w = WEOF;

	f = fopen("rtl_w.txt", "wb");
	fputs("x", f);
	fclose(f);
	/* a stream opened for reading only: every write fails at once */
	f = fopen("rtl_w.txt", "rb");
	setvbuf(f, NULL, _IONBF, 0);
	r = fputs("abc", f);
	printf("RTL|fputs on a read-only stream returned %d (EOF=%d)\n", r, EOF);
	r = fputws(L"abc", f);
	printf("RTL|fputws on a read-only stream returned %d (WEOF as int=%d, EOF=%d, sizeof(wint_t)=%d)\n",
		r, (int) w, EOF, (int) sizeof(wint_t));
	printf("RTL|fputws result == WEOF: %d\n", r == WEOF);
	fclose(f);
}

static void nfile_case(void)
{
	int n = 0;
	FILE *f[5000];
	while (n < 5000 && (f[n] = fopen("NUL", "r")) != NULL)
		n++;
	printf("RTL|fopen succeeded %d times before failing (plus stdin, stdout, stderr)\n", n);
	while (n > 0)
		fclose(f[--n]);
}

static void dfree_case(void)
{
	char *p = (char *) malloc(100), *q;
	int i;
	memset(p, 1, 100);
	free(p);
	printf("RTL|first free done\n");
	fflush(stdout);
	free(p);
	printf("RTL|second free of the same block returned\n");
	fflush(stdout);
	for (i = 0; i < 4; i++) {
		q = (char *) malloc(100);
		printf("RTL|malloc after double free: %p (freed block was %p)\n", q, p);
	}
	fflush(stdout);
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return 2;
	if (!strcmp(argv[1], "weof"))
		weof_case();
	else if (!strcmp(argv[1], "nfile"))
		nfile_case();
	else if (!strcmp(argv[1], "dfree"))
		dfree_case();
	printf("RTL|end\n");
	return 0;
}
