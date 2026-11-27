// C-9 step 2: isolated probes for HK write-failure paths and automatic
// output (Hk/HkIV.cpp, Hk/HkEmit.cpp, Hk/HkAjat.cpp, Hk/HkTulostus.cpp,
// tputilv2 TextFl/openprfile). Every probe calls the real production code
// (linked as in TpCoreHk); only global state is prepared and a write failure
// is injected from outside the production code. Protocol:
// TPsource/V52/Tests/fixtures/TpCore.h (preconditions with REQUIRE, exactly
// one TPRESULT per defect test, a crash or timeout is never evidence).
// No network (fileyht stays -1, or points to an unopened link whose
// hComm entry is 0, so wrt_st_TCP returns before any socket call), no
// shipped program, synthetic data only.
//
// Write-failure injection:
//  - emit file and AJAT queue: the production fd is swapped for a second fd
//    of the same file opened read-only, so _lseek succeeds and _write fails
//    (the same return path as disk full or a lost share); the original fd
//    is restored afterwards.
//  - result files (TextFl/stdio): TextFl opens with the CRT secure share
//    mode, so no second open is possible while it is open; after the open
//    the stream's fd is replaced (_dup2) by a read-only duplicate of its own
//    handle (DuplicateHandle with reduced access), so the buffered data
//    cannot be written when stdio flushes, as with disk full during writing.
//
//  H-145  emit card whose write fails: tall_emit returns 0 (the value on
//         which the reader is acknowledged, Tp/TpLaitteet.cpp:911-916), the
//         card is cached in ed_em, emithead does not move, and a re-read of
//         the same card is taken as a confirmation (not stored).
//  H-146  writeajat advances fp1 before _write: after one failed row the
//         next successful write leaves the failed row's slot as it was, and
//         the restart read (initajat) does not see the row.
//  H-147  WriteLine returns the fputs/fputws result, compared with WEOF;
//         autofile truncates the published file and a flush failure is not
//         reported.
//  H-150  autofile kohde 'X' at stage 2+: one TextFl (and one FILE when
//         AUTOFILE.LST exists) per run is not freed.
//  H-151  after the sendtcpfile thread has closed aftulparam.lstf, the next
//         autofile reads the freed PRFILE in its entry guard (ASan).

#include <winsock2.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <malloc.h>
#include <psapi.h>
#include "doctest.h"
#include "fixtures/TpCore.h"
#include "HkDeclare.h"
#include "fixtures/HkFixture.h"
#include "TpLaitteet.h"

// Defined without a header declaration (Hk/HkIV.cpp, Hk/HkTulostus.cpp).
void initajat(INT ino, INT vaihda);
void htmlalku(wchar_t *wtitle, wchar_t *wheader, int frame, tulostusparamtp *tulprm);
void htmlloppu(tulostusparamtp *tulprm);

#pragma comment(lib, "psapi.lib")

// ---------- helpers ----------

static int countMessages(const char *needle)
{
	int n = 0;
	std::vector<tp::UiMessage> &m = tp::uiMessages();
	for (size_t i = 0; i < m.size(); i++)
		if (!needle || m[i].text.find(needle) != std::string::npos)
			n++;
	return n;
}

static std::string allMessages()
{
	std::string s;
	std::vector<tp::UiMessage> &m = tp::uiMessages();
	for (size_t i = 0; i < m.size(); i++)
		s += tp::fmt("[%d] %s; ", m[i].kesto, m[i].text.c_str());
	return s;
}

// Number of CRT FILE streams in use: opens NUL until the CRT refuses.
static int streamsInUse()
{
	std::vector<FILE *> v;
	for (;;) {
		FILE *f = fopen("NUL", "r");
		if (!f)
			break;
		v.push_back(f);
		if (v.size() > 5000)
			break;
	}
	int freeSlots = (int) v.size();
	for (size_t i = 0; i < v.size(); i++)
		fclose(v[i]);
	return _getmaxstdio() - freeSlots;
}

static size_t heapUsed()
{
	_HEAPINFO hi;
	hi._pentry = NULL;
	size_t used = 0;
	while (_heapwalk(&hi) == _HEAPOK)
		if (hi._useflag == _USEDENTRY)
			used += hi._size;
	return used;
}

static DWORD handleCount()
{
	DWORD n = 0;
	GetProcessHandleCount(GetCurrentProcess(), &n);
	return n;
}

static void writeFile(const char *name, const char *text, int repeat)
{
	FILE *f = fopen(name, "wb");
	REQUIRE(f != NULL);
	for (int i = 0; i < repeat; i++)
		fputs(text, f);
	fclose(f);
}

// Swaps the CRT fd behind a just-opened TextFl for a read-only fd on the
// same file object (_dup2), so that stdio's buffered data cannot be written at the
// next flush. TextFl::File is private; the fd is found as the lowest free
// fd before the open (the CRT hands out the lowest free number) and checked
// by its final path.
static int nextFreeFd()
{
	int fd = _wopen(L"NUL", _O_RDONLY);
	REQUIRE(fd >= 0);
	_close(fd);
	return fd;
}

static bool fdIsFile(int fd, const wchar_t *name)
{
	HANDLE h = (HANDLE) _get_osfhandle(fd);
	if (h == INVALID_HANDLE_VALUE)
		return false;
	wchar_t path[600];
	DWORD n = GetFinalPathNameByHandleW(h, path, 600, 0);
	if (n == 0 || n >= 600)
		return false;
	size_t ln = wcslen(name);
	return n >= ln && _wcsicmp(path + n - ln, name) == 0;
}

// TextFl opens with the CRT's secure share mode, which refuses any other
// open of the file while it is open, so a read-only duplicate of the
// stream's own handle is used (DuplicateHandle with reduced access) instead
// of a second open.
static bool makeWritesFail(int fd, const wchar_t *name)
{
	(void) name;
	HANDLE h = (HANDLE) _get_osfhandle(fd), h2 = NULL;
	if (h == INVALID_HANDLE_VALUE ||
		!DuplicateHandle(GetCurrentProcess(), h, GetCurrentProcess(), &h2, FILE_GENERIC_READ, FALSE, 0))
		return false;
	int ro = _open_osfhandle((intptr_t) h2, _O_RDONLY | _O_BINARY);
	if (ro < 0)
		return false;
	int r = _dup2(ro, fd);
	_close(ro);
	return r == 0;
}

// ---------- H-145 emit ----------

static void emitFixture()
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	tph::setStages(1, 10);
	maxkilpno = 1000;
	sarjaluku = 0;
	nsarjayhd = 0;
	k_pv = 0;
	tph::makeEmptyKilpDat();
	REQUIRE(teejarj(false) == 0);
	emitfl = 1;
	esitark = 0;
	esiluenta = 0;
	luesuoraan = 0;
	REQUIRE(lueemitfile() == 0);
	REQUIRE(emitfile >= 0);
	REQUIRE(emithead == 0);
	regnly[0] = LID_MTR;
	t0_regn[0] = 0;
}

static void makeMtrCard(san_type *v, INT32 badge, UINT32 pkg, int sek)
{
	memset(v, 0, sizeof(*v));
	v->r13.pkgno = pkg;
	v->r13.badge[0] = (char) (badge & 0xff);
	v->r13.badge[1] = (char) ((badge >> 8) & 0xff);
	v->r13.badge[2] = (char) ((badge >> 16) & 0xff);
	v->r13.t_hour = 10;
	v->r13.t_min = 15;
	v->r13.t_sek = (char) sek;
	for (int i = 0; i < 5; i++) {
		v->r13.c[i].code = (char) (31 + i);
		v->r13.c[i].t1 = (UINT16) (60 * (i + 1));
	}
	v->r13.c[5].code = (char) 250;
	v->r13.c[5].t1 = 400;
}

static int cardsInFile(INT32 badge)
{
	long long sz = tp::fileSize("EMIT1.DAT");
	int n = 0;
	for (long long p = 0; p + (long long) sizeof(emittp) <= sz; p += sizeof(emittp)) {
		emittp e;
		REQUIRE(tp::readAt("EMIT1.DAT", p, &e, sizeof(e)) == sizeof(e));
		if (e.badge == badge)
			n++;
	}
	return n;
}

TEST_CASE("C9 H-145 control: emit card is stored")
{
	emitFixture();
	san_type v;
	UINT32 vahv = 0;
	makeMtrCard(&v, 12345, 1, 10);
	int r = tall_emit(&v, &vahv, 0);
	tp::info(tp::fmt("tall_emit=%d emithead=%d file=%lld cards=%d msgs=%s", r, (int) emithead,
		tp::fileSize("EMIT1.DAT"), cardsInFile(12345), allMessages().c_str()));
	CHECK(r == 0);
	CHECK(emithead == 1);
	CHECK(cardsInFile(12345) == 1);
}

static void emitWriteFailure(bool estaToisto, const char *mech)
{
	emitFixture();
	estaemittoisto = estaToisto ? 1 : 0;
	san_type vA, vB;
	UINT32 vahv = 0;
	makeMtrCard(&vA, 12345, 1, 10);
	makeMtrCard(&vB, 23456, 2, 20);
	int fd = emitfile;
	int ro = _wopen(emitfname, _O_RDONLY | _O_BINARY);
	REQUIRE(ro >= 0);
	emitfile = ro;                                   // injection: _write fails
	size_t m0 = tp::uiMessages().size();
	int r1 = tall_emit(&vA, &vahv, 0);
	int head1 = emithead;
	std::string msg1 = allMessages();
	emitfile = fd;                                   // fault removed
	_close(ro);
	vahv = 0;
	makeMtrCard(&vA, 12345, 1, 10);                  // tall_emit edits the buffer
	int r2 = tall_emit(&vA, &vahv, 0);               // same card again
	int head2 = emithead;
	UINT32 vahv2 = vahv;
	int r3 = tall_emit(&vB, &vahv, 0);               // another card
	int head3 = emithead;
	makeMtrCard(&vA, 12345, 1, 10);
	int r4 = tall_emit(&vA, &vahv, 0);               // card A after another card
	int head4 = emithead;
	int inFileA = cardsInFile(12345), inFileB = cardsInFile(23456);
	tp::info(tp::fmt("estaemittoisto=%d", estaToisto ? 1 : 0));
	tp::info(tp::fmt("failed write: tall_emit=%d emithead=%d new messages=%d (%s)", r1, head1,
		(int) (tp::uiMessages().size() - m0), msg1.c_str()));
	tp::info(tp::fmt("same card re-read: tall_emit=%d emithead=%d vahvistus=%u", r2, head2, vahv2));
	tp::info(tp::fmt("other card B: tall_emit=%d emithead=%d", r3, head3));
	tp::info(tp::fmt("card A after B: tall_emit=%d emithead=%d", r4, head4));
	tp::info(tp::fmt("file: %lld bytes, card A records=%d, card B records=%d", tp::fileSize("EMIT1.DAT"),
		inFileA, inFileB));
	bool observed = r1 == 0 && head1 == 0 && head2 == 0 && vahv2 == 12345 && inFileB == 1;
	if (estaToisto)
		observed = observed && inFileA == 0;
	tp::reportResult("H-145", mech, observed,
		tp::fmt("failed write returned %d (ACK value), head %d; re-read confirmation %u, not stored; "
			"after another card A stored %d time(s)", r1, head1, vahv2, inFileA));
}

TEST_CASE("C9 H-145 emit write failure with duplicate suppression (default)")
{
	emitWriteFailure(true, "card-acked-cached-lost");
}

TEST_CASE("C9 H-145 emit write failure without duplicate suppression")
{
	emitWriteFailure(false, "card-acked-cached-reread-after-other-card");
}

// ---------- H-146 AJAT ----------

static void ajatFixture()
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	maxntime = 20;
	maxjono = 0;
	lajatfl = 0;
	sail_ajat = 1;
	loki = 0;
	if (aikajono[0]) {
		delete aikajono[0];
		aikajono[0] = NULL;
	}
	aikajono[0] = new aikajonotp(maxntime);
	aikajono[0]->jono = 0;
	wcscpy(aikajono[0]->aikafname, L"AJAT1.LST");
	// vaihda = 1 is the default start (Hk/HkIV.cpp:4231 with ajanottofl -1):
	// rows are set to empty placeholders, no prompt; sail_ajat keeps old rows.
	initajat(0, 1);
	REQUIRE(aikajono[0]->afhandle > 0);
	REQUIRE(aikajono[0]->rwtime == 0);
}

static void putRow(int kno, int sek, int expectRet = 0)
{
	aikatp u;
	memset(&u, 0, sizeof(u));
	u.t = (10 * 3600 + sek) * SEK * AIKAJAK / KSEK;
	u.kno = kno;
	u.piste = 0;
	u.date = 0;
	INT r = tall_rivi(0, &u, NULL, NULL, 0, 0, 0, false);
	CHECK(r == expectRet);
}

static std::string fileRows(int n)
{
	std::string s;
	for (int i = 1; i <= n; i++) {
		aikatp a;
		if (tp::readAt("AJAT1.LST", (long long) i * sizeof(aikatp), &a, sizeof(a)) != sizeof(a))
			break;
		s += tp::fmt("slot%d:kno=%d,t=%d; ", i, (int) a.kno, (int) a.t);
	}
	return s;
}

static std::string memRows()
{
	std::string s;
	for (int i = 0; i < aikajono[0]->rwtime; i++) {
		aikatp a;
		gettm(i, &a, 0);
		s += tp::fmt("row%d:kno=%d,t=%d; ", i, (int) a.kno, (int) a.t);
	}
	return s;
}

TEST_CASE("C9 H-146 control: AJAT rows are written and read back")
{
	ajatFixture();
	putRow(101, 10);
	putRow(102, 20);
	putRow(103, 30);
	std::string f = fileRows(4);
	delete aikajono[0];
	aikajono[0] = new aikajonotp(maxntime);
	wcscpy(aikajono[0]->aikafname, L"AJAT1.LST");
	initajat(0, 1);
	tp::info("file: " + f);
	tp::info("after restart: " + memRows());
	CHECK(aikajono[0]->rwtime == 3);
}

TEST_CASE("C9 H-146 AJAT write failure leaves a hole that the restart reads")
{
	ajatFixture();
	putRow(101, 10);
	int fd = aikajono[0]->afhandle;
	int ro = _wopen(L"AJAT1.LST", _O_RDONLY | _O_BINARY);
	REQUIRE(ro >= 0);
	aikajono[0]->afhandle = ro;                       // injection: _write fails
	int fpBefore = aikajono[0]->fp1;
	putRow(102, 20);
	int fpAfter = aikajono[0]->fp1;
	std::string msgs = allMessages();
	aikajono[0]->afhandle = fd;                       // fault removed
	_close(ro);
	putRow(103, 30);
	putRow(104, 40);
	std::string mem = memRows();
	std::string f = fileRows(5);
	aikatp slot2;
	REQUIRE(tp::readAt("AJAT1.LST", 2 * (long long) sizeof(aikatp), &slot2, sizeof(slot2)) == sizeof(slot2));
	tp::info(tp::fmt("fp1 before failed row %d, after %d; messages: %s", fpBefore, fpAfter, msgs.c_str()));
	tp::info("memory before restart: " + mem);
	tp::info("file before restart: " + f);
	// restart: a new queue object reads the file (sail_ajat = keep old times)
	delete aikajono[0];
	aikajono[0] = new aikajonotp(maxntime);
	wcscpy(aikajono[0]->aikafname, L"AJAT1.LST");
	initajat(0, 1);
	std::string after = memRows();
	tp::info(tp::fmt("after restart: rwtime=%d rtime=%d: ", aikajono[0]->rwtime, aikajono[0]->rtime) + after);
	putRow(105, 25);                                 // a later time between 102 and 103
	tp::info("after restart + row 105 (10:00:25): " + memRows());
	bool missing102 = after.find("kno=102,") == std::string::npos;
	bool observed = fpAfter > fpBefore && slot2.kno != 102 && missing102;
	tp::reportResult("H-146", "fp1-advanced-before-failed-write", observed,
		tp::fmt("fp1 %d->%d on the failed row; slot 2 kno=%d t=%d; row 102 after restart %s",
			fpBefore, fpAfter, (int) slot2.kno, (int) slot2.t, missing102 ? "missing" : "present"));
}

// ---------- H-147 result files ----------

TEST_CASE("C9 H-147 WriteLine return value on a failed flush vs WEOF")
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	const wchar_t *types = L"AW8";
	std::string detail;
	bool anyMissed = false;
	for (int ti = 0; types[ti]; ti++) {
		wchar_t name[20];
		swprintf(name, 20, L"weof_%c.txt", types[ti]);
		char trl[2] = { (char) types[ti], 0 };
		int fd = nextFreeFd();
		PRFILE *pf = openprfile(name, -1, 1, 2, trl, 0);    // as autofile, HkTulostus.cpp:7108
		REQUIRE(pf != NULL);
		REQUIRE(fdIsFile(fd, name));
		REQUIRE(makeWritesFail(fd, name));                  // injection
		tulostusparamtp prm;
		prm.lstf = pf;
		size_t m0 = tp::uiMessages().size();
		int minRet = 0x7fffffff, fails = 0, calls = 0;
		wchar_t line[200];
		for (int i = 0; i < 199; i++) line[i] = L'x';
		line[198] = L'\n';
		line[199] = 0;
		// Direct calls show the return value; puts_f/writeline show the check.
		for (int i = 0; i < 100; i++) {
			int r = pf->u.Fl->WriteLine(line);
			calls++;
			if (r < minRet) minRet = r;
			if (r == -1 || r == (int) WEOF) fails++;
		}
		for (int i = 0; i < 100 && prm.lstf == pf; i++) {
			prm.puts_f(line);
			prm.writeline(line);
		}
		int msgs = (int) (tp::uiMessages().size() - m0);
		closeprfile(pf);
		long long sz = tp::fileSize(tp::fmt("weof_%c.txt", (char) types[ti]).c_str());
		detail += tp::fmt("type %c: %d WriteLine calls, %d returned -1 or WEOF, min %d (WEOF=%d); "
			"puts_f/writeline messages %d; file %lld bytes. ", (char) types[ti], calls, fails, minRet,
			(int) WEOF, msgs, sz);
		if (fails > 0 && msgs == 0)
			anyMissed = true;
	}
	tp::info(detail);
	tp::reportResult("H-147", "weof-check-misses-write-error", anyMissed, detail);
}

static void afHkFixture(wchar_t kohde, bool yksihtml)
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	tph::setStages(2, 10);
	sarjaluku = 0;
	nsarjayhd = 0;
	k_pv = 0;
	fileyht = -1;
	memset(&autofileparam, 0, sizeof(autofileparam));
	wcscpy(autofileparam.afname, L"af_tulos.html");
	wcscpy(autofileparam.aflstnm, L"AUTOFILE.LST");
	aftulparam.lstf = NULL;
	aftulparam.kohde = kohde;
	aftulparam.yksihtml = yksihtml;
	if (!aftulparam.sarjalista) {
		aftulparam.sarjalista = new INT16[MAXSARJALUKU + MAXYHD];
		memset(aftulparam.sarjalista, 0, (MAXSARJALUKU + MAXYHD) * 2);
	}
}

TEST_CASE("C9 H-147 control: autofile replaces the published file")
{
	afHkFixture(L'H', true);
	writeFile("af_tulos.html", "OLD RESULTS\r\n", 200);
	long long before = tp::fileSize("af_tulos.html");
	int r = autofile(0);
	long long after = tp::fileSize("af_tulos.html");
	char head[16] = {0};
	tp::readAt("af_tulos.html", 0, head, 15);
	tp::info(tp::fmt("autofile=%d size %lld -> %lld, starts '%s', messages=%d", r, before, after, head,
		(int) tp::uiMessages().size()));
	CHECK(r == 0);
	CHECK(after > 0);
}

// autofile's own sequence for one file, with the production functions it
// calls: openprfile(afname, -1, 1, 2, merkit) (HkTulostus.cpp:7108), html
// lines through writehtml/puts_f (htmlalku/htmlloppu), closeprfile
// (:7319-7321). The write failure is injected after the open; autofile
// itself cannot be paused between its open and its first write.
TEST_CASE("C9 H-147 autofile sequence truncates the published file and hides the failed flush")
{
	afHkFixture(L'H', true);
	writeFile("af_tulos.html", "OLD RESULTS\r\n", 200);
	long long before = tp::fileSize("af_tulos.html");
	aftulparam.merkit = L'A';
	int fd = nextFreeFd();
	aftulparam.lstf = openprfile(autofileparam.afname, -1, 1, 2, (char *) &aftulparam.merkit, 0);
	REQUIRE(aftulparam.lstf != NULL);
	long long atOpen = tp::fileSize("af_tulos.html");
	HANDLE rd = CreateFileW(L"af_tulos.html", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, 0, NULL);
	DWORD rdErr = rd == INVALID_HANDLE_VALUE ? GetLastError() : 0;
	if (rd != INVALID_HANDLE_VALUE)
		CloseHandle(rd);
	REQUIRE(fdIsFile(fd, L"af_tulos.html"));
	REQUIRE(makeWritesFail(fd, L"af_tulos.html"));      // injection
	htmlalku(autofileparam.wtitlea, autofileparam.wheadera, 0, &aftulparam);
	for (int i = 0; i < 60; i++)
		aftulparam.writehtml(L"<tr><td>1</td><td>Testi Kilpailija</td><td>Seura</td><td>1.23.45</td></tr>\n");
	htmlloppu(&aftulparam);
	closeprfile(aftulparam.lstf);
	aftulparam.lstf = NULL;
	long long after = tp::fileSize("af_tulos.html");
	int msgs = (int) tp::uiMessages().size();
	tp::info(tp::fmt("old %lld bytes; right after open %lld; a reader opening during generation: error %lu; "
		"after the writes and close %lld bytes; messages=%d (%s)", before, atOpen, rdErr, after, msgs,
		allMessages().c_str()));
	bool observed = atOpen == 0 && after == 0 && msgs == 0;
	tp::reportResult("H-147", "truncate-then-silent-flush-failure", observed,
		tp::fmt("old file %lld bytes, 0 at open: %s, after failed writes %lld bytes, %d messages",
			before, atOpen == 0 ? "yes" : "no", after, msgs));
}

TEST_CASE("C9 H-147 autofile with the published file locked whole")
{
	afHkFixture(L'H', true);
	writeFile("af_tulos.html", "OLD RESULTS\r\n", 200);
	long long before = tp::fileSize("af_tulos.html");
	HANDLE h = CreateFileW(L"af_tulos.html", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	REQUIRE(h != INVALID_HANDLE_VALUE);       // a reader that denies writers
	int r = autofile(0);
	CloseHandle(h);
	long long after = tp::fileSize("af_tulos.html");
	tp::info(tp::fmt("reader denies writers: autofile=%d size %lld -> %lld, messages: %s", r, before, after,
		allMessages().c_str()));
	tp::reportResult("H-147", "open-failure-reported-old-file-kept", r == 1 && after == before && countMessages(NULL) > 0,
		tp::fmt("return %d, size %lld -> %lld, %d messages", r, before, after, countMessages(NULL)));
}

// ---------- H-150 leak ----------

static void leakRun(bool withList, int maxRuns)
{
	afHkFixture(L'X', false);
	k_pv = 1;
	if (withList)
		writeFile("AUTOFILE.LST", "\r\n", 1);
	REQUIRE(tp::fileSize("AUTOFILE.LST") == (withList ? 2 : -1));
	int s0 = streamsInUse();
	DWORD h0 = handleCount();
	size_t m0 = heapUsed();
	int firstFail = 0, runs = 0;
	for (int i = 1; i <= maxRuns; i++) {
		int r = autofile(0);
		runs = i;
		if (r != 0 && !firstFail) {
			firstFail = i;
			tp::info(tp::fmt("run %d: autofile=%d, messages: %s", i, r, allMessages().c_str()));
		}
		if (i == 1 || i % 100 == 0 || (firstFail && i == firstFail))
			tp::info(tp::fmt("run %d: streams %d (+%d), handles %lu (+%ld), heap used %Iu (+%Id)", i,
				streamsInUse(), streamsInUse() - s0, handleCount(), (long) (handleCount() - h0),
				heapUsed(), (ptrdiff_t) (heapUsed() - m0)));
		if (firstFail && i > firstFail + 2)
			break;
	}
	int ds = streamsInUse() - s0;
	ptrdiff_t dm = (ptrdiff_t) (heapUsed() - m0);
	tp::reportResult("H-150", withList ? "xml-stage2-leak-with-list" : "xml-stage2-leak-without-list",
		withList ? ds > runs / 2 : dm > 0,
		tp::fmt("%d runs, first failing run %d, streams +%d, handles +%ld, heap +%Id bytes (%.1f per run)",
			runs, firstFail, ds, (long) (handleCount() - h0), dm, (double) dm / runs));
}

TEST_CASE("C9 H-150 autofile XML stage 2 with AUTOFILE.LST")
{
	leakRun(true, 700);
}

TEST_CASE("C9 H-150 autofile XML stage 2 without AUTOFILE.LST")
{
	leakRun(false, 700);
}

TEST_CASE("C9 H-150 control: autofile XML stage 1")
{
	afHkFixture(L'X', false);
	k_pv = 0;
	writeFile("AUTOFILE.LST", "\r\n", 1);
	int s0 = streamsInUse();
	size_t m0 = heapUsed();
	for (int i = 0; i < 200; i++)
		CHECK(autofile(0) == 0);
	tp::info(tp::fmt("stage 1, 200 runs: streams +%d, heap +%Id", streamsInUse() - s0,
		(ptrdiff_t) (heapUsed() - m0)));
	CHECK(streamsInUse() - s0 == 0);
}

// ---------- H-151 dangling lstf after sendtcpfile ----------

#ifdef __SANITIZE_ADDRESS__
TEST_CASE("C9 H-151 next autofile reads the PRFILE freed by sendtcpfile (ASan)")
{
	afHkFixture(L'H', true);
	const int yht = 5;
	REQUIRE(hComm[yht] == 0);                         // wrt_st_TCP returns at once
	InitializeCriticalSection(&tcp_CriticalSection[yht]);
	taustaon = 0;                                     // skip the send loop
	fileyht = yht;
	REQUIRE(autofile(0) == 0);
	Sleep(1000);                                      // let sendtcpfile finish
	REQUIRE(tp::lockFreeForOtherThread(&tcp_CriticalSection[yht]) == 1);
	tp::info(tp::fmt("after run 1: aftulparam.lstf=%p (closed by the thread)", (void *) aftulparam.lstf));
	REQUIRE(aftulparam.lstf != NULL);
	static const char *const expect[] = { "heap-use-after-free", "HkTulostus.cpp", 0 };
	tp::armCrash("H-151", "dangling-lstf-after-sendtcpfile", "asan", expect);
	autofile(0);
	tp::disarmCrash();
	tp::reportResult("H-151", "dangling-lstf-after-sendtcpfile", false, "no ASan report in the second run");
}
#endif
