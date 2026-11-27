// C-9 step 2: isolated probes for the relay automatic output file (H-149).
// The real production autofile() of Juk/VTulostus.cpp is compiled into this
// probe (included below, as Tests/core/VTulostusTests.cpp does; TpCoreJuk
// leaves VTulostus.cpp out of its source list). Only global state is
// prepared here. Protocol: TPsource/V52/Tests/fixtures/TpCore.h
// (preconditions with REQUIRE, exactly one TPRESULT per defect test, a crash
// or timeout is never evidence; ASan reports are evidence only through
// tp::armCrash with expected substrings). No network: fileyht stays -1, so
// the sendtcpfile thread is never started. No shipped program is run.
//
//  H-149a  single-file mode (yksihtml, kohde 'H', the default when
//          AUTOFILE.LST is missing): closeprfile(aftulparam.lstf) at :7200
//          frees the PRFILE without clearing the pointer; htmlloppu at :7299
//          then writes through it (use after free).
//  H-149b  kohde 'T' in single-file mode: the same pointer is freed again
//          at :7321 (double free; 'T' has no htmlloppu in between).
//  H-149c  separate-files mode without a list (yksihtml false, no
//          AUTOFILE.LST): delete aflst at :7164 and again at :7295-7296.
//          TextFl lives in the uninstrumented tputilv2.lib, so ASan does
//          not see the destructor read the freed object; it reports the
//          access violation that follows in fclose (stack: ~TextFl called
//          from autofile at VTulostus.cpp:7295).
//  H-149d  kohde 'X': returns at :7186 with lstf dangling; the next run reads
//          it in the entry guard at :7150.
//  Q85     Release build (no ASan): single-file mode repeated, with the
//          CRT heap check after every cycle and the result file size.

#include <winsock2.h>
#include <malloc.h>
#include "doctest.h"
#include "fixtures/TpCore.h"
#include "../../../../../../TPsource/V52/Juk/VTulostus.cpp"
#include "../../../../../../TPsource/V52/Juk/vdat.cpp"    // as Tests/core/VAddfrecTests.cpp
#include "fixtures/VFixture.h"

static void afFixture(wchar_t kohde, bool yksihtml, bool withList)
{
	REQUIRE(tp::sandboxOk());
	tpv::initBase();
	sarjaluku = 0;
	nsarjayhd = 0;
	fileyht = -1;                  // no TCP file transfer, no thread
	memset(&autofileparam, 0, sizeof(autofileparam));
	wcscpy(autofileparam.afname, L"af_tulos.html");
	wcscpy(autofileparam.aflstnm, L"AUTOFILE.LST");
	autofileparam.afvali = 60 * SEK;
	aftulparam.lstf = NULL;
	aftulparam.kohde = kohde;
	aftulparam.yksihtml = yksihtml;
	aftulparam.merkit = L'A';
	aftulparam.tulmuot = filetulosmuot;
	if (withList) {
		FILE *f = fopen("AUTOFILE.LST", "wb");
		REQUIRE(f != NULL);
		fputs("\r\n", f);
		fclose(f);
	}
	REQUIRE(tp::fileSize("AUTOFILE.LST") == (withList ? 2 : -1));
}

#ifdef __SANITIZE_ADDRESS__

TEST_CASE("C9 H-149 control: relay autofile list mode (ASan)")
{
	afFixture(L'H', false, true);
	static const char *const expect[] = { "AddressSanitizer", 0 };
	tp::armCrash("H-149", "control-list-mode", "asan", expect);
	int r1 = autofile();
	int r2 = autofile();
	tp::disarmCrash();
	tp::info(tp::fmt("list mode: autofile -> %d, %d; lstf=%p; messages=%d", r1, r2,
		(void *) aftulparam.lstf, (int) tp::uiMessages().size()));
	tp::reportResult("H-149", "control-list-mode", false, "no ASan report in list mode (two runs)");
}

TEST_CASE("C9 H-149a relay autofile single-file mode writes through the freed PRFILE (ASan)")
{
	afFixture(L'H', true, false);
	static const char *const expect[] = { "heap-use-after-free", "VTulostus.cpp", 0 };
	tp::armCrash("H-149", "single-file-use-after-free", "asan", expect);
	autofile();
	tp::disarmCrash();
	tp::reportResult("H-149", "single-file-use-after-free", false, "no ASan report from autofile");
}

TEST_CASE("C9 H-149b relay autofile T mode frees the PRFILE twice (ASan)")
{
	afFixture(L'T', true, false);
	static const char *const expect[] = { "double-free", 0 };
	tp::armCrash("H-149", "t-mode-double-free", "asan", expect);
	autofile();
	tp::disarmCrash();
	tp::reportResult("H-149", "t-mode-double-free", false, "no ASan report from autofile");
}

TEST_CASE("C9 H-149c relay autofile separate-files mode without a list deletes aflst twice (ASan)")
{
	afFixture(L'H', false, false);
	static const char *const expect[] = { "TextFl::~TextFl", "VTulostus.cpp:7295", 0 };
	tp::armCrash("H-149", "no-list-double-delete", "asan", expect);
	autofile();
	tp::disarmCrash();
	tp::reportResult("H-149", "no-list-double-delete", false, "no ASan report from autofile");
}

TEST_CASE("C9 H-149d relay autofile X mode leaves lstf dangling for the next run (ASan)")
{
	afFixture(L'X', false, false);
	int r1 = autofile();
	tp::info(tp::fmt("X run 1 -> %d; lstf after run 1 = %p (freed if non-null)", r1,
		(void *) aftulparam.lstf));
	REQUIRE(aftulparam.lstf != NULL);
	static const char *const expect[] = { "heap-use-after-free", "VTulostus.cpp", 0 };
	tp::armCrash("H-149", "x-mode-dangling-guard", "asan", expect);
	autofile();
	tp::disarmCrash();
	tp::reportResult("H-149", "x-mode-dangling-guard", false, "no ASan report in the second run");
}

#else  // Release: behaviour without ASan (Q85), never evidence of the mechanism

TEST_CASE("C9 Q85 Release control: relay autofile list mode repeated")
{
	afFixture(L'H', false, true);
	int bad = 0;
	for (int i = 0; i < 30; i++) {
		int r = autofile();
		int h = _heapchk();
		if (h != _HEAPOK || r != 0)
			bad++;
	}
	tp::info(tp::fmt("list mode: 30 cycles, %d with a heap check failure or non-zero return", bad));
	tp::reportResult("Q85", "release-control-list-mode", bad > 0, tp::fmt("%d bad cycles of 30", bad));
}

TEST_CASE("C9 Q85 Release relay autofile single-file mode repeated")
{
	afFixture(L'H', true, false);
	int cycles = 0, bad = 0;
	for (int i = 0; i < 30; i++) {
		tp::info(tp::fmt("cycle %d: calling autofile", i + 1));
		int r = autofile();
		int h = _heapchk();
		long long sz = tp::fileSize("af_tulos.html");
		tp::info(tp::fmt("cycle %d: autofile=%d heapchk=%d (OK=%d) size=%lld lstf=%p messages=%d",
			i + 1, r, h, _HEAPOK, sz, (void *) aftulparam.lstf, (int) tp::uiMessages().size()));
		cycles++;
		if (h != _HEAPOK)
			bad++;
	}
	tp::reportResult("Q85", "release-repeat", bad > 0,
		tp::fmt("%d cycles completed, heap check failed after %d of them", cycles, bad));
}

// fileyht >= 0 is set by the first opened TCP link (Tp/TpComY.cpp:1221-1222),
// so networked relays take the sendtcpfile branch (:7307-7318) instead of
// the second closeprfile. hComm[yht] is 0 here, so wrt_st_TCP would return
// before any socket call; no network.
TEST_CASE("C9 Q85 Release relay autofile single-file mode with a TCP file link configured")
{
	afFixture(L'H', true, false);
	const int yht = 5;
	REQUIRE(hComm[yht] == 0);
	InitializeCriticalSection(&tcp_CriticalSection[yht]);
	comfl = 0;
	fileyht = yht;
	int bad = 0;
	for (int i = 0; i < 10; i++) {
		tp::info(tp::fmt("cycle %d: calling autofile", i + 1));
		int r = autofile();
		Sleep(200);                                  // sendtcpfile thread
		int h = _heapchk();
		char tail[40] = {0};
		long long sz = tp::fileSize("af_tulos.html");
		if (sz > 20)
			tp::readAt("af_tulos.html", sz - 20, tail, 20);
		for (int k = 0; k < 20; k++) if (tail[k] == '\r' || tail[k] == '\n') tail[k] = ' ';
		tp::info(tp::fmt("cycle %d: autofile=%d heapchk=%d size=%lld ends '%s'", i + 1, r, h, sz, tail));
		if (h != _HEAPOK) bad++;
	}
	long long sz = tp::fileSize("af_tulos.html");
	std::string all(sz > 0 ? (size_t) sz : 0, '\0');
	if (sz > 0) tp::readAt("af_tulos.html", 0, &all[0], (size_t) sz);
	bool hasEnd = all.find("</HTML>") != std::string::npos || all.find("</html>") != std::string::npos;
	tp::reportResult("Q85", "release-repeat-tcp-link", bad > 0 || !hasEnd,
		tp::fmt("10 cycles, heap check failed after %d; published file %lld bytes, closing </HTML> %s",
			bad, sz, hasEnd ? "present" : "missing"));
}

TEST_CASE("C9 Q85 Release relay autofile separate-files mode without a list")
{
	afFixture(L'H', false, false);
	for (int i = 0; i < 5; i++) {
		tp::info(tp::fmt("cycle %d: calling autofile", i + 1));
		int r = autofile();
		tp::info(tp::fmt("cycle %d: autofile=%d heapchk=%d", i + 1, r, _heapchk()));
	}
	tp::reportResult("Q85", "release-repeat-no-list", false, "5 cycles completed");
}

// Review question 4: path (c) (separate files, no AUTOFILE.LST) with a TCP
// file link configured (hComm 0, no network). The double delete of aflst
// (:7164, :7295) does not depend on fileyht.
TEST_CASE("C9 Q85 Release relay autofile separate-files mode without a list, TCP link configured")
{
	afFixture(L'H', false, false);
	const int yht = 5;
	REQUIRE(hComm[yht] == 0);
	InitializeCriticalSection(&tcp_CriticalSection[yht]);
	comfl = 0;
	fileyht = yht;
	for (int i = 0; i < 3; i++) {
		tp::info(tp::fmt("cycle %d: calling autofile", i + 1));
		int r = autofile();
		Sleep(200);
		tp::info(tp::fmt("cycle %d: autofile=%d heapchk=%d", i + 1, r, _heapchk()));
	}
	tp::reportResult("Q85", "release-no-list-tcp-link", false, "3 cycles completed");
}

// Review item N1 (static claim): in list mode with a TCP link, sendtcpfile
// (:7104-7133) frees aftulparam.lstf without clearing it; the next autofile
// passes the guard at :7150 and, for the first series with results, calls
// closeprfile on the freed PRFILE at :7262. Setup as in the review recipe:
// lstf opened with the production openprfile, the production sendtcpfile
// called directly (hComm 0, so wrt_st_TCP returns before any socket call),
// then autofile in list mode with one series that has a result count. The
// double free at :7262 comes before tulostasarja (:7290). Release only: a
// process end with 0xC0000374 is recorded by aja.py as a consequence, not
// as evidence; the evidence is the code path plus the progress lines.
TEST_CASE("C9 N1 Release relay list mode after sendtcpfile closes lstf")
{
	afFixture(L'H', false, false);
	FILE *f = fopen("AUTOFILE.LST", "wb");
	REQUIRE(f != NULL);
	fputs("N1SARJA 1 0 n1_sarja.html\r\n", f);
	fclose(f);
	sarjaluku = 1;
	strcpy(Sarjat[0].sarjanimi, "N1SARJA");
	Sarjat[0].ntosuus = 1;
	ntulos[0][0][0] = 1;
	REQUIRE(haesarja_w(L"N1SARJA", true) == 0);
	const int yht = 5;
	REQUIRE(hComm[yht] == 0);
	InitializeCriticalSection(&tcp_CriticalSection[yht]);
	comfl = 0;
	fileyht = yht;
	aftulparam.lstf = openprfile(L"n1_edellinen.html", -1, 1, 2, (char *) &aftulparam.merkit, 0);
	REQUIRE(aftulparam.lstf != NULL);
	PRFILE *old = aftulparam.lstf;
	sendtcpfile(&aftulparam);                       // frees lstf (:7131)
	tp::info(tp::fmt("after sendtcpfile: aftulparam.lstf=%p (freed, same as opened %p)",
		(void *) aftulparam.lstf, (void *) old));
	REQUIRE(aftulparam.lstf == old);
	tp::info("calling autofile in list mode (expect closeprfile on the freed PRFILE at :7262)");
	int r = autofile();
	tp::info(tp::fmt("autofile returned %d, heapchk=%d", r, _heapchk()));
	tp::reportResult("N1", "list-mode-after-sendtcpfile", false, "autofile returned without a heap failure");
}

TEST_CASE("C9 Q85 Release relay autofile T mode repeated")
{
	afFixture(L'T', true, false);
	int cycles = 0, bad = 0;
	for (int i = 0; i < 30; i++) {
		tp::info(tp::fmt("cycle %d: calling autofile", i + 1));
		int r = autofile();
		int h = _heapchk();
		tp::info(tp::fmt("cycle %d: autofile=%d heapchk=%d size=%lld", i + 1, r, h,
			tp::fileSize("af_tulos.html")));
		cycles++;
		if (h != _HEAPOK)
			bad++;
	}
	tp::reportResult("Q85", "release-repeat-t", bad > 0,
		tp::fmt("%d cycles completed, heap check failed after %d of them", cycles, bad));
}

#endif
