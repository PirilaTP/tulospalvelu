// C-6 step 2: isolated probes for lock defects in the HK console core.
// Every probe calls the real production code (linked as in TpCoreHk); only
// global state is prepared here. Protocol: TPsource/V52/Tests/fixtures/TpCore.h
// (preconditions with REQUIRE/CHECK, exactly one TPRESULT per defect test,
// a crash or timeout is never evidence). No network connection is made:
// sockets are never created; connection records use INVALID_SOCKET.
//
//  P1 H-090  tall_regnly (Hk/HkIV.cpp:1984) enters ajat_CriticalSection at
//            :2674 and returns at :2771 / :2776 / :2782 / :3074 without
//            leaving it.
//  P2 H-093  wkirjloki (Hk/HkApu.cpp:546) -> wrt_st_TCP (com/WinUDP.cpp:896)
//            -> :942 writesocketerror -> wkirjloki ... unbounded recursion
//            when the TCP log link is down and reconnect is set.
//  P3 H-093  lock order loki -> tcpip (wkirjloki -> wrt_st_TCP :907) and
//            tcpip -> loki (wrt_st_TCP :915 -> writesocketerror -> wkirjloki
//            :560), and both together as a two-thread deadlock.
//  P4 H-094  add_bdg_t (Hk/HkIV.cpp:815) with vaiheenvaihto set starts a
//            consumer pros_bdg_t (:790) that exits at once (:796), so the
//            badge never reaches tall_bdg_t (:753).
//  P5 H-091  tall -> print: tall_elahto (Hk/HkIV.cpp:572) -> tallenna ->
//            lisaToimet (Hk/HkDat.cpp:1374) -> sivuTuloste
//            (Hk/HkTulostus.cpp:932); print -> tall: autotulostus (:1004)
//            -> tulostasarja (:5850).

#include <winsock2.h>
#include "doctest.h"
#include "fixtures/TpCore.h"
#include "HkDeclare.h"
#include "fixtures/HkFixture.h"
#include "TpLaitteet.h"
#include "HkMuotoilu.h"
#include <malloc.h>
#include <tlhelp32.h>
#include <dbghelp.h>
#include <stddef.h>

// Production functions not declared in the headers the core includes.
int tall_regnly(san_type *vastaus, INT r_no);                      // Hk/HkIV.cpp:1984
void add_bdg_t(UINT32 badge, INT r_no, char byte9, UINT32 tms);    // Hk/HkIV.cpp:815
void tall_elahto(int badge, int t);                                // Hk/HkIV.cpp:555
void initindex(void);                                              // Hk/HkIx.cpp:102
int wrt_st_TCP(HANDLE hC, int len, char *buf, int *nsent);         // com/WinUDP.cpp:896

// ------------------------------------------------------------- helpers

// Number of threads waiting on a held CRITICAL_SECTION (LockCount layout of
// Windows Server 2003 SP1 and later: bit 0 = 0 when locked, bit 1 = 0 when a
// waiter was woken, the remaining bits are the ones complement of the
// number of waiters).
static int csWaiters(CRITICAL_SECTION *cs)
{
	LONG lc = cs->LockCount;
	if (lc == -1)
		return 0;
	return (int) (((unsigned long) ~lc) >> 2);
}

static bool threadAlive(DWORD id)
{
	HANDLE h = OpenThread(SYNCHRONIZE, FALSE, id);
	if (!h)
		return false;
	bool alive = WaitForSingleObject(h, 0) == WAIT_TIMEOUT;
	CloseHandle(h);
	return alive;
}

static int processThreadCount()
{
	HANDLE s = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (s == INVALID_HANDLE_VALUE)
		return -1;
	THREADENTRY32 te;
	te.dwSize = sizeof(te);
	int n = 0;
	DWORD pid = GetCurrentProcessId();
	if (Thread32First(s, &te)) {
		do {
			if (te.th32OwnerProcessID == pid)
				n++;
		} while (Thread32Next(s, &te));
	}
	CloseHandle(s);
	return n;
}

static std::string csState(const char *name, CRITICAL_SECTION *cs)
{
	return tp::fmt("%s owner=%lu rec=%ld lockcount=%ld waiters=%d", name,
		(unsigned long) tp::lockOwnerThread(cs), (long) cs->RecursionCount,
		(long) cs->LockCount, csWaiters(cs));
}

// ===================================================== P1 H-090 tall_regnly

struct RegnlyArg {
	san_type msg;
	int ret;
};

static void runRegnly(void *p)
{
	RegnlyArg *a = (RegnlyArg *) p;
	a->ret = tall_regnly(&a->msg, 0);
}

// RTR-1 message (san_type.r1, Hk/HkDef.h:1424-1440): tunnus, cr1, n1, ' ',
// kno[4], ' ', min[3], ':', sek[2], '.', osat[2], CR. Parsed at
// Hk/HkIV.cpp:2072-2083 and :2615-2650.
static void rtrMessage(san_type *m, char tunnus, int mm, int ss)
{
	char b[40];
	memset(m, 0, sizeof(*m));
	// tunnus, cr1/n1/s1 = 3 blanks, kno "  12", blank, min, ':', sek, '.', osat, CR
	_snprintf(b, sizeof(b), "%c     12 %3d:%02d.00\r", tunnus, mm, ss);
	REQUIRE(b[3] == ' ');
	REQUIRE(b[8] == ' ');
	REQUIRE(b[18] == '\r');
	memcpy(m->bytes, b, 19);
}

// One RTR-1 time device on input 0 that reports on channel 4 (aika_jono[0][4]
// = queue 0, aika_tunnus[0][4] = point 0, i.e. finish). Time queue 1 is the
// identification queue (TUNNJONO=1). No time queue is open (aikajono[] all
// NULL), as before the timing has been started.
static void regnlyFixture(int tunnjonoValue)
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	tph::setStages(2, 10);   // tall_regnly constructs kilptietue locals (Q34)
	loki = 0;
	k_pv = 0;
	regnly[0] = LID_RTR;
	regnly_no[0] = 0;
	aika_jono[0][4] = 0;
	aika_tunnus[0][4] = 0;
	t0_regn[0] = 0;
	kerro_regnly = 0;
	vainoletus = 0;
	lajatfl = 0;
	lahdepistehaku = 0;
	lahdejonohaku = 0;
	maxjono = 1;
	kello_esto = 0;
	maxero = 2000000000L;
	tunnjono = tunnjonoValue;
	for (int i = 0; i < MAXJONO; i++)
		aikajono[i] = NULL;
	REQUIRE(tp::lockFreeForOtherThread(&ajat_CriticalSection) == 1);
}

// Runs tall_regnly once on a controlled thread and reports the lock state.
static tp::ThreadRun regnlyOnce(RegnlyArg *arg, char tunnus, int mm, int ss)
{
	rtrMessage(&arg->msg, tunnus, mm, ss);
	arg->ret = -99;
	tp::ThreadRun r = tp::runOnThread(runRegnly, arg, 5000, "regnly");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	return r;
}

static void reportRegnly(const char *mechanism, const tp::ThreadRun &r, int ret, const char *path)
{
	int probe = tp::lockFreeForOtherThread(&ajat_CriticalSection);
	REQUIRE(probe >= 0);
	DWORD owner = tp::lockOwnerThread(&ajat_CriticalSection);
	REQUIRE((probe == 0 ? owner == r.threadId : owner == 0));
	bool observed = probe == 0 && owner == r.threadId;
	tp::reportResult("H-090", mechanism, observed,
		tp::fmt("%s; tall_regnly returned %d in %lu ms on thread %lu, which has ended; other thread "
			"got ajat lock=%d; %s", path, ret, (unsigned long) r.elapsedMs, (unsigned long) r.threadId,
			probe, csState("ajat", &ajat_CriticalSection).c_str()));
}

// Control: without TUNNJONO the same message takes the else branch (:2678)
// and the :2782 return; the lock is never taken.
TEST_CASE("C6 H-090 control: tunnjono off and same message leaves ajat free")
{
	regnlyFixture(-1);
	static RegnlyArg arg;
	size_t m0 = tp::uiMessages().size();
	tp::ThreadRun r = regnlyOnce(&arg, 'A', 1, 5);
	CHECK(arg.ret == 1);
	CHECK(tp::uiMessages().size() == m0 + 1);   // :2779 "AJANOTTOA EI KAYNNISTETTY"
	CHECK(tp::lockFreeForOtherThread(&ajat_CriticalSection) == 1);
	CHECK(tp::lockOwnerThread(&ajat_CriticalSection) == 0);
	tp::info(tp::fmt("control thread %lu ret=%d %s", (unsigned long) r.threadId, arg.ret,
		csState("ajat", &ajat_CriticalSection).c_str()));
}

// :2777-2782: the time queue aikajono[0] is NULL (timing not started).
TEST_CASE("C6 H-090 tall_regnly time queue not started returns at 2782 holding ajat")
{
	regnlyFixture(1);
	static RegnlyArg arg;
	size_t m0 = tp::uiMessages().size();
	tp::ThreadRun r = regnlyOnce(&arg, 'A', 1, 5);
	REQUIRE(arg.ret == 1);
	REQUIRE(tp::uiMessages().size() == m0 + 1);
	REQUIRE(tp::uiMessages().back().text.find("AJANOTTOA EI") != std::string::npos);
	reportRegnly("tall-regnly-2782-queue-missing", r, arg.ret,
		"path :2674 enter, :2777-2782 return (message 'AJANOTTOA EI KAYNNISTETTY')");
}

// :2774-2776: first 'A' message whose clock differs from the PC clock by
// more than maxero. maxero = 0 here (default 1000000), so any nonzero bt
// takes the path. A missing message proves :2782 was not reached.
TEST_CASE("C6 H-090 tall_regnly clock difference over maxero returns at 2776 holding ajat")
{
	regnlyFixture(1);
	maxero = 0;
	static RegnlyArg arg;
	size_t m0 = tp::uiMessages().size();
	tp::ThreadRun r = regnlyOnce(&arg, 'A', 1, 5);
	REQUIRE(arg.ret == 1);
	REQUIRE(tp::uiMessages().size() == m0);   // not the :2779 message path
	reportRegnly("tall-regnly-2776-maxero", r, arg.ret,
		"path :2674 enter, :2774-2776 return (labs(bt) > maxero, first try); maxero set to 0");
}

// :2770-2771: KELLO_ESTO. First message with TUNNJONO off stores edaika
// (:2772, no lock); the second, 1 s later, with TUNNJONO on, enters the lock
// and returns at :2771 because it is within kello_esto (200 = 2 s).
TEST_CASE("C6 H-090 tall_regnly KELLO_ESTO returns at 2771 holding ajat")
{
	regnlyFixture(-1);
	kello_esto = 200;
	static RegnlyArg arg1, arg2;
	size_t m0 = tp::uiMessages().size();
	regnlyOnce(&arg1, 'A', 1, 5);
	REQUIRE(arg1.ret == 1);
	REQUIRE(tp::uiMessages().size() == m0 + 1);   // first one: :2782 without lock
	REQUIRE(tp::lockFreeForOtherThread(&ajat_CriticalSection) == 1);
	tunnjono = 1;
	tp::ThreadRun r = regnlyOnce(&arg2, 'A', 1, 6);
	REQUIRE(arg2.ret == 0);
	REQUIRE(tp::uiMessages().size() == m0 + 1);   // second one: no new message
	reportRegnly("tall-regnly-2771-kello-esto", r, arg2.ret,
		"path :2674 enter, :2770-2771 return (second time 1 s after the first, kello_esto=200)");
}

// Code 'G' (RTR clock check line) is parsed (:2616) but is outside 'A'..'F'
// (:2773), so the function falls to :3074 without the leave at :3027.
TEST_CASE("C6 H-090 tall_regnly code G returns at 3074 holding ajat")
{
	regnlyFixture(1);
	static RegnlyArg arg;
	size_t m0 = tp::uiMessages().size();
	tp::ThreadRun r = regnlyOnce(&arg, 'G', 1, 5);
	REQUIRE(arg.ret == 0);
	REQUIRE(tp::uiMessages().size() == m0);
	reportRegnly("tall-regnly-3074-code-outside-a-f", r, arg.ret,
		"path :2674 enter, :2773 skipped (code G), :3074 return");
}

// ========================================== P2/P3 H-093 TCP log and locks

// Copy of the file-static record type of com/WinUDP.cpp:92-112. Its size
// and field offsets are checked against the debug information of this exe
// before use (tcpTable below).
struct TcpConnMirror {
	int serverfl;
	int connected;
	int reconnect;
	ipparamtp *ipparam;
	SOCKET sockmaster;
	SOCKET sockdata;
	struct sockaddr_in masteraddr;
	struct sockaddr_in dataaddr;
	struct sockaddr destaddr;
	long rtt;
	long srtt;
	long rttvar;
	long rto;
	char naapuri[4];
	char str_ip[64];
	char *rcvbuf;
	int bufhead;
	int buftail;
	long lastmsg;
};

static const int WINUDP_MAX_PORTTI = 128;   // com/WinUDP.cpp:47

static bool g_symInit = false;

static bool symInit(std::string &why)
{
	if (g_symInit)
		return true;
	char dir[MAX_PATH];
	DWORD n = GetModuleFileNameA(NULL, dir, MAX_PATH);
	if (n == 0 || n >= MAX_PATH) {
		why = "GetModuleFileName";
		return false;
	}
	char *slash = strrchr(dir, '\\');
	if (slash)
		*slash = 0;
	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_NO_PROMPTS);
	// Search path = exe directory only (no symbol server).
	if (!SymInitialize(GetCurrentProcess(), dir, TRUE)) {
		why = tp::fmt("SymInitialize error %lu", GetLastError());
		return false;
	}
	g_symInit = true;
	return true;
}

static int fieldOffset(HANDLE proc, ULONG64 base, ULONG structType, const wchar_t *name)
{
	DWORD count = 0;
	if (!SymGetTypeInfo(proc, base, structType, TI_GET_CHILDRENCOUNT, &count) || count == 0)
		return -1;
	size_t sz = sizeof(TI_FINDCHILDREN_PARAMS) + count * sizeof(ULONG);
	TI_FINDCHILDREN_PARAMS *fc = (TI_FINDCHILDREN_PARAMS *) calloc(1, sz);
	fc->Count = count;
	int result = -1;
	if (SymGetTypeInfo(proc, base, structType, TI_FINDCHILDREN, fc)) {
		for (DWORD i = 0; i < count; i++) {
			WCHAR *nm = 0;
			if (SymGetTypeInfo(proc, base, fc->ChildId[i], TI_GET_SYMNAME, &nm) && nm) {
				if (wcscmp(nm, name) == 0) {
					DWORD off = 0;
					if (SymGetTypeInfo(proc, base, fc->ChildId[i], TI_GET_OFFSET, &off))
						result = (int) off;
				}
				LocalFree(nm);
			}
		}
	}
	free(fc);
	return result;
}

// Locates the file-static TCPyhteys[] (com/WinUDP.cpp:115) through the
// exe's PDB and checks that TcpConnMirror matches its element type.
static TcpConnMirror **tcpTable(std::string &why)
{
	if (!symInit(why))
		return 0;
	HANDLE proc = GetCurrentProcess();
	char sbuf[sizeof(SYMBOL_INFO) + 256];
	SYMBOL_INFO *sym = (SYMBOL_INFO *) sbuf;
	memset(sbuf, 0, sizeof(sbuf));
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);
	sym->MaxNameLen = 255;
	if (!SymFromName(proc, "TCPyhteys", sym)) {
		why = tp::fmt("SymFromName(TCPyhteys) error %lu", GetLastError());
		return 0;
	}
	if (sym->Size != WINUDP_MAX_PORTTI * sizeof(void *)) {
		why = tp::fmt("TCPyhteys size %lu", (unsigned long) sym->Size);
		return 0;
	}
	ULONG64 base = sym->ModBase;
	DWORD ptrType = 0, structType = 0;
	ULONG64 len = 0;
	if (!SymGetTypeInfo(proc, base, sym->TypeIndex, TI_GET_TYPEID, &ptrType) ||
		!SymGetTypeInfo(proc, base, ptrType, TI_GET_TYPEID, &structType) ||
		!SymGetTypeInfo(proc, base, structType, TI_GET_LENGTH, &len)) {
		why = "type information of TCPyhteys not available";
		return 0;
	}
	if (len != sizeof(TcpConnMirror)) {
		why = tp::fmt("record size %lu, mirror %lu", (unsigned long) len, (unsigned long) sizeof(TcpConnMirror));
		return 0;
	}
	struct { const wchar_t *n; int off; } f[] = {
		{L"serverfl", (int) offsetof(TcpConnMirror, serverfl)},
		{L"connected", (int) offsetof(TcpConnMirror, connected)},
		{L"reconnect", (int) offsetof(TcpConnMirror, reconnect)},
		{L"ipparam", (int) offsetof(TcpConnMirror, ipparam)},
		{L"sockdata", (int) offsetof(TcpConnMirror, sockdata)},
		{L"dataaddr", (int) offsetof(TcpConnMirror, dataaddr)},
		{L"lastmsg", (int) offsetof(TcpConnMirror, lastmsg)},
	};
	for (size_t i = 0; i < sizeof(f) / sizeof(f[0]); i++) {
		int o = fieldOffset(proc, base, structType, f[i].n);
		if (o != f[i].off) {
			why = tp::fmt("field %ls offset %d, mirror %d", f[i].n, o, f[i].off);
			return 0;
		}
	}
	tp::info(tp::fmt("TCPyhteys at 0x%08lX (PDB), element size %lu, offsets checked",
		(unsigned long) sym->Address, (unsigned long) len));
	return (TcpConnMirror **) (ULONG_PTR) sym->Address;
}

static TcpConnMirror g_conn[2];
static ipparamtp g_ip[2];
static TcpConnMirror **g_table;

// A never connected client record: no socket (INVALID_SOCKET), peer
// 0.0.0.0:0. Nothing in it can reach the network.
static void initConn(int i, int connected, int reconnect)
{
	memset(&g_ip[i], 0, sizeof(g_ip[i]));
	g_ip[i].iptype = ipTCPCLIENT;
	wcscpy(g_ip[i].destaddr, L"0.0.0.0");
	g_ip[i].noreconnect = !reconnect;
	memset(&g_conn[i], 0, sizeof(g_conn[i]));
	g_conn[i].serverfl = 0;
	g_conn[i].connected = connected;
	g_conn[i].reconnect = reconnect;
	g_conn[i].ipparam = &g_ip[i];
	g_conn[i].sockmaster = INVALID_SOCKET;
	g_conn[i].sockdata = INVALID_SOCKET;
	g_conn[i].dataaddr.sin_family = AF_INET;
	strcpy(g_conn[i].str_ip, "0.0.0.0");
	g_conn[i].rttvar = 750;
	g_conn[i].rto = 3000;
}

// Log to TCP as openTCPloki (Tp/TpComY.cpp:2238-2255) leaves it: loki = 1,
// lokiTCP = 1, hLoki = connection 1 (TCPyhteys[0]). Connection 2
// (TCPyhteys[1]) stands for any other TCP link. comfl = 1 as with any
// configured link.
static void logFixture(int c0connected, int c0reconnect, int c1connected)
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	std::string why;
	g_table = tcpTable(why);
	if (!g_table)
		tp::info("tcpTable: " + why);
	REQUIRE(g_table != 0);
	REQUIRE(g_table[0] == 0);
	REQUIRE(g_table[1] == 0);
	WSADATA wd;
	REQUIRE(WSAStartup(MAKEWORD(2, 2), &wd) == 0);
	initConn(0, c0connected, c0reconnect);
	initConn(1, c1connected, 0);
	g_table[0] = &g_conn[0];
	g_table[1] = &g_conn[1];
	lokitulparam.lstf = NULL;
	siritloki = false;
	loki = 1;
	lokiTCP = 1;
	hLoki = (HANDLE) 1;
	comfl = 1;
	REQUIRE(tp::lockFreeForOtherThread(&loki_CriticalSection) == 1);
	REQUIRE(tp::lockFreeForOtherThread(&tcpip_CriticalSection) == 1);
}

static wchar_t g_logLine[] = L"C6 probe log line";

struct RecArg {
	volatile LONG caught;
	volatile LONG returned;
	volatile DWORD code;
	volatile ULONG_PTR fault;
	volatile LONG depth;
	ULONG_PTR stackBase;
	ULONG_PTR stackLimit;
};

// Runs on the overflowed stack: only reads the exception record and the
// lock's recursion count (= number of nested wkirjloki frames past :560).
static int soFilter(EXCEPTION_POINTERS *ep, RecArg *a)
{
	a->code = ep->ExceptionRecord->ExceptionCode;
	if (ep->ExceptionRecord->NumberParameters >= 2)
		a->fault = ep->ExceptionRecord->ExceptionInformation[1];
	a->depth = loki_CriticalSection.RecursionCount;
	return a->code == EXCEPTION_STACK_OVERFLOW ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}

static void logGuarded(RecArg *a)
{
	__try {
		wkirjloki(g_logLine);
		a->returned = 1;
	}
	__except (soFilter(GetExceptionInformation(), a)) {
		a->caught = 1;
	}
}

// Thread body: wkirjloki under a stack overflow guard. After a caught
// overflow the guard page is restored, so the thread ends normally.
static void runLogGuarded(void *p)
{
	RecArg *a = (RecArg *) p;
	NT_TIB *tib = (NT_TIB *) NtCurrentTeb();
	a->stackBase = (ULONG_PTR) tib->StackBase;
	logGuarded(a);
	a->stackLimit = (ULONG_PTR) tib->StackLimit;
	if (a->caught)
		_resetstkoflw();
}

static void runLog(void *)
{
	wkirjloki(g_logLine);
}

static char g_dataMsg[] = "x";

static void runWrtTcp2(void *)
{
	int nsent = 0;
	wrt_st_TCP((HANDLE) 2, 1, g_dataMsg, &nsent);
}

// Control: log link down but reconnect off (NORECONNECT). wrt_st_TCP takes
// the :949-951 branch; wkirjloki returns once and leaves loki.
TEST_CASE("C6 H-093 control: TCP log down without reconnect returns")
{
	logFixture(0, 0, 0);
	static RecArg a;
	LONG we0 = writeerrorOn = 12345;
	tp::ThreadRun r = tp::runOnThread(runLogGuarded, &a, 10000, "logrec");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	CHECK(a.returned == 1);
	CHECK(a.caught == 0);
	CHECK(g_ip[0].status == 1);               // :940 was reached
	CHECK(writeerrorOn == we0);
	CHECK(tp::lockFreeForOtherThread(&loki_CriticalSection) == 1);
	CHECK(tp::lockFreeForOtherThread(&tcpip_CriticalSection) == 1);
	tp::info(tp::fmt("control returned=%ld caught=%ld %s", (long) a.returned, (long) a.caught,
		csState("loki", &loki_CriticalSection).c_str()));
}

// Defect: log link down, reconnect on (the default, noreconnect = 0).
// wkirjloki :560 enter loki -> :583 wrt_st_TCP -> :938-942 writesocketerror
// (err 0 != 10054) -> com/WinUDP.cpp:182 wkirjloki -> ... before reconnectTCP
// (:943) is ever called. Evidence: a stack overflow caught on the probe
// thread, the loki recursion count at that moment (= nesting depth) and that
// reconnectTCP left no trace (writeerrorOn and sockdata unchanged).
TEST_CASE("C6 H-093 wkirjloki TCP log reconnect recursion overflows the stack")
{
	logFixture(0, 1, 0);
	static RecArg a;
	LONG we0 = writeerrorOn = 12345;
	tp::ThreadRun r = tp::runOnThread(runLogGuarded, &a, 20000, "logrec");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	REQUIRE((a.returned == 1) != (a.caught == 1));
	bool noReconnect = writeerrorOn == we0 && g_conn[0].sockdata == INVALID_SOCKET;
	unsigned long used = (unsigned long) (a.stackBase - a.fault);
	bool observed = a.caught == 1 && a.code == EXCEPTION_STACK_OVERFLOW && a.depth >= 50 &&
		tp::lockOwnerThread(&loki_CriticalSection) == r.threadId && noReconnect && g_ip[0].status == 1;
	tp::reportResult("H-093", "wkirjloki-wrt_st_TCP-reconnect-recursion", observed,
		tp::fmt("exception 0x%08lX caught=%ld returned=%ld; nested wkirjloki frames holding loki at the "
			"fault=%ld; stack used %lu bytes (base 0x%08lX, fault 0x%08lX), about %lu bytes per level; "
			"reconnectTCP reached=%d (writeerrorOn=%ld, sockdata=%s); %s",
			(unsigned long) a.code, (long) a.caught, (long) a.returned, (long) a.depth, used,
			(unsigned long) a.stackBase, (unsigned long) a.fault,
			a.depth > 0 ? used / (unsigned long) a.depth : 0UL, noReconnect ? 0 : 1, (long) writeerrorOn,
			g_conn[0].sockdata == INVALID_SOCKET ? "INVALID_SOCKET" : "changed",
			csState("loki", &loki_CriticalSection).c_str()));
}

// Control for the edges: no lock held by the test, log link benign (down,
// no reconnect). Y's send error goes through writesocketerror -> wkirjloki
// -> wrt_st_TCP(log) (seen as g_ip[0].status = 1) and both calls return.
TEST_CASE("C6 H-093 control: send error with free locks returns")
{
	logFixture(0, 0, 1);
	g_ip[0].status = 0;
	tp::ThreadRun ry = tp::runOnThread(runWrtTcp2, 0, 10000, "Y");
	REQUIRE(ry.outcome == tp::THREAD_FINISHED);
	CHECK(g_ip[0].status == 1);   // the error message reached the log path
	CHECK(g_conn[1].connected == 0);
	tp::ThreadRun rx = tp::runOnThread(runLog, 0, 10000, "X");
	REQUIRE(rx.outcome == tp::THREAD_FINISHED);
	CHECK(tp::lockFreeForOtherThread(&loki_CriticalSection) == 1);
	CHECK(tp::lockFreeForOtherThread(&tcpip_CriticalSection) == 1);
}

// Edge loki -> tcpip: the test holds tcpip; X calls wkirjloki on a log link
// that is marked connected. X takes loki (:560) and waits for tcpip in
// wrt_st_TCP (:907).
TEST_CASE("C6 H-093 edge loki to tcpip: wkirjloki holds loki and waits for tcpip")
{
	logFixture(1, 0, 0);
	EnterCriticalSection(&tcpip_CriticalSection);
	DWORD me = GetCurrentThreadId();
	tp::ThreadRun rx = tp::runOnThread(runLog, 0, 2000, "X");
	REQUIRE(rx.outcome == tp::THREAD_BLOCKED);
	REQUIRE(tp::lockOwnerThread(&tcpip_CriticalSection) == me);
	DWORD lokiOwner = tp::lockOwnerThread(&loki_CriticalSection);
	int w = csWaiters(&tcpip_CriticalSection);
	bool observed = lokiOwner == rx.threadId && w == 1 && threadAlive(rx.threadId);
	tp::reportResult("H-093", "edge-loki-then-tcpip", observed,
		tp::fmt("X=%lu blocked >= %lu ms; %s; %s (holder = test thread %lu)", (unsigned long) rx.threadId,
			(unsigned long) rx.elapsedMs, csState("loki", &loki_CriticalSection).c_str(),
			csState("tcpip", &tcpip_CriticalSection).c_str(), (unsigned long) me));
	// tcpip stays held: releasing it would let X hit the invalid socket and
	// recurse (P2 mechanism). CoreMain ends the process.
}

// Edge tcpip -> loki: the test holds loki; Y sends on connection 2 whose
// socket is invalid. Y takes tcpip (:907), select fails (WSAENOTSOCK,
// not 10054), writesocketerror (:915) -> wkirjloki waits for loki (:560).
TEST_CASE("C6 H-093 edge tcpip to loki: send error holds tcpip and waits for loki")
{
	logFixture(0, 0, 1);
	EnterCriticalSection(&loki_CriticalSection);
	DWORD me = GetCurrentThreadId();
	tp::ThreadRun ry = tp::runOnThread(runWrtTcp2, 0, 2000, "Y");
	REQUIRE(ry.outcome == tp::THREAD_BLOCKED);
	REQUIRE(tp::lockOwnerThread(&loki_CriticalSection) == me);
	DWORD tcpOwner = tp::lockOwnerThread(&tcpip_CriticalSection);
	int w = csWaiters(&loki_CriticalSection);
	bool observed = tcpOwner == ry.threadId && w == 1 && threadAlive(ry.threadId);
	tp::reportResult("H-093", "edge-tcpip-then-loki", observed,
		tp::fmt("Y=%lu blocked >= %lu ms; %s; %s (holder = test thread %lu)", (unsigned long) ry.threadId,
			(unsigned long) ry.elapsedMs, csState("tcpip", &tcpip_CriticalSection).c_str(),
			csState("loki", &loki_CriticalSection).c_str(), (unsigned long) me));
}

// Both edges at once. The test holds tcpip; Y (send on connection 2) waits
// for tcpip; then X (wkirjloki, log link marked connected) takes loki and
// also waits for tcpip. The test releases tcpip. If Y gets it, Y's send
// error needs loki (X) while X needs tcpip (Y): deadlock. If X gets it
// first, X recurses (P2 mechanism on the invalid log socket); that is caught
// on X's thread and reported as observed=0.
TEST_CASE("C6 H-093 two-thread deadlock loki and tcpip")
{
	logFixture(1, 0, 1);
	static RecArg ax;
	EnterCriticalSection(&tcpip_CriticalSection);
	tp::ThreadRun ry = tp::runOnThread(runWrtTcp2, 0, 1000, "Y");
	REQUIRE(ry.outcome == tp::THREAD_BLOCKED);
	REQUIRE(csWaiters(&tcpip_CriticalSection) == 1);
	tp::ThreadRun rx = tp::runOnThread(runLogGuarded, &ax, 1000, "X");
	REQUIRE(rx.outcome == tp::THREAD_BLOCKED);
	REQUIRE(tp::lockOwnerThread(&loki_CriticalSection) == rx.threadId);
	REQUIRE(csWaiters(&tcpip_CriticalSection) == 2);
	LeaveCriticalSection(&tcpip_CriticalSection);
	Sleep(2000);
	DWORD lo = tp::lockOwnerThread(&loki_CriticalSection);
	DWORD to = tp::lockOwnerThread(&tcpip_CriticalSection);
	bool xAlive = threadAlive(rx.threadId), yAlive = threadAlive(ry.threadId);
	bool observed = lo == rx.threadId && to == ry.threadId && xAlive && yAlive &&
		csWaiters(&loki_CriticalSection) == 1 && csWaiters(&tcpip_CriticalSection) == 1 && ax.caught == 0;
	int pl = tp::lockFreeForOtherThread(&loki_CriticalSection);
	int pt = tp::lockFreeForOtherThread(&tcpip_CriticalSection);
	tp::reportResult("H-093", "deadlock-loki-tcpip-two-threads", observed,
		tp::fmt("2 s after release: X=%lu alive=%d, Y=%lu alive=%d; %s; %s; other thread got loki=%d "
			"tcpip=%d; X overflow caught=%ld depth=%ld", (unsigned long) rx.threadId, xAlive ? 1 : 0,
			(unsigned long) ry.threadId, yAlive ? 1 : 0, csState("loki", &loki_CriticalSection).c_str(),
			csState("tcpip", &tcpip_CriticalSection).c_str(), pl, pt, (long) ax.caught, (long) ax.depth));
}

// ============================================== P4 H-094 add_bdg_t consumer

static const int REGNLY_SENTINEL = -777777;

struct BdgArg {
	UINT32 badge;
};

static void runAddBdg(void *p)
{
	add_bdg_t(((BdgArg *) p)->badge, 0, 0, 0);
}

static void bdgFixture(bool phaseChange)
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	timerfl = 1;
	taustaon = 1;
	vaiheenvaihto = phaseChange;
	lahdepistehaku = 0;
	ohitatoisto = 0;
	for (int i = 0; i < MAXJONO; i++)
		aikajono[i] = NULL;
	for (int i = 0; i <= NREGNLY; i++)
		kaikki_ajat[i] = -1;   // aloitus default (Hk/HkInit.cpp:3144); tall_etulos returns at :609 without a queue
	regnlyhetki[0] = REGNLY_SENTINEL;
}

// tall_bdg_t sets regnlyhetki[r_no] unconditionally (:758); that is the only
// effect observed here. Polls up to ms milliseconds.
static bool waitRegnlyChange(DWORD ms)
{
	DWORD t0x = GetTickCount();
	while (GetTickCount() - t0x < ms) {
		if (regnlyhetki[0] != REGNLY_SENTINEL)
			return true;
		Sleep(10);
	}
	return regnlyhetki[0] != REGNLY_SENTINEL;
}

TEST_CASE("C6 H-094 control: without vaiheenvaihto the badge reaches tall_bdg_t")
{
	bdgFixture(false);
	int th0 = processThreadCount();
	static BdgArg a = {4711};
	tp::ThreadRun r = tp::runOnThread(runAddBdg, &a, 5000, "addbdg");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	bool processed = waitRegnlyChange(2000);
	int th1 = processThreadCount();
	CHECK(processed);
	tp::info(tp::fmt("control: processed=%d regnlyhetki[0]=%d threads before=%d after=%d (consumer alive)",
		processed ? 1 : 0, regnlyhetki[0], th0, th1));
	CHECK(th1 == th0 + 1);
}

TEST_CASE("C6 H-094 add_bdg_t during vaiheenvaihto: consumer exits and badge is never processed")
{
	bdgFixture(true);
	int th0 = processThreadCount();
	static BdgArg a1 = {4711}, a2 = {4712};
	tp::ThreadRun r1 = tp::runOnThread(runAddBdg, &a1, 5000, "addbdg");
	REQUIRE(r1.outcome == tp::THREAD_FINISHED);
	bool p1 = waitRegnlyChange(1000);   // 20 x the consumer's Sleep(50)
	int th1 = processThreadCount();
	tp::ThreadRun r2 = tp::runOnThread(runAddBdg, &a2, 5000, "addbdg");
	REQUIRE(r2.outcome == tp::THREAD_FINISHED);
	bool p2 = waitRegnlyChange(1000);
	int th2 = processThreadCount();
	bool observed = !p1 && !p2 && regnlyhetki[0] == REGNLY_SENTINEL;
	tp::reportResult("H-094", "add-bdg-t-consumer-exits-in-vaiheenvaihto", observed,
		tp::fmt("two badges added 1 s apart; regnlyhetki[0]=%d (sentinel %d) after 1 s each; process "
			"threads before=%d after first=%d after second=%d (no consumer left running)",
			regnlyhetki[0], REGNLY_SENTINEL, th0, th1, th2));
}

// ============================================== P5 H-091 tall <-> print

static char g_fakePrinter[4096];   // never dereferenced before the lock

// Competitor bib 12, class 0, status 'E' (did not start) on stage 0, badge
// 500, written to KILP.DAT in the sandbox and loaded with the production
// loader teejarj(false) (Hk/HkIx.cpp:1554), which builds kilparr, the
// indexes, the result order arrays and the badge index bdg_kno.
static void printFixture()
{
	REQUIRE(tp::sandboxOk());
	tph::initBase();
	kilpparam.n_pv_akt = 2;
	tph::setStages(2, 10);
	loki = 0;
	k_pv = 0;
	sarjaluku = 1;
	nsarjayhd = 0;
	emitfl = 1;
	comfl = 0;
	maxyhteys = 0;
	minkilpno = 1;
	ToimintaTila = 2;
	Sarjat[0].mno[0] = Sarjat[0].mno[1] = 1;
	for (int i_pv = 0; i_pv < N_PV; i_pv++) {   // aloitus defaults, Hk/HkInit.cpp:3151-3161
		pvparam[i_pv].laika = 8;
		pvparam[i_pv].laikasj = 8;
		pvparam[i_pv].laika2 = 8;
		pvparam[i_pv].laikava = 8;
		pvparam[i_pv].pyor[0] = 1;
		for (int j = 1; j <= 5; j++)
			pvparam[i_pv].pyor[j] = SEK;
	}
	tph::makeEmptyKilpDat();
	{
		kilptietue x;   // after setStages (Q34)
		x.nollaa();
		x.setIdAll(12);
		wcscpy(x.sukunimi, L"Xkoski");
		wcscpy(x.seura, L"SEURA");
		x.setSarjaAll(0);
		x.set_tark(L'E', 0);
		x.pv[0].badge[0] = 500;
		char buf[KILPBUFSIZE];
		x.pack(buf);
		DATAREF d = 0;
		addrec(&datf2, &d, buf);
		REQUIRE(d == 1);
	}
	REQUIRE(teejarj(false) == 0);
	REQUIRE(nrec == 2);
	REQUIRE(getpos(12) == 1);
	REQUIRE(bdg2kno(500) == 12);
	emitjono = 0;
	for (int i = 0; i < MAXJONO; i++)
		aikajono[i] = NULL;
	eNollausAjat = 0;
	kilptietue k;
	k.GETREC(1);
	REQUIRE(k.tark(0) == L'E');
}

static void runElahto(void *)
{
	tall_elahto(500, 0);
}

static void runAutotul(void *)
{
	autotulostus();
}

// Control: page printing switched on, but not to a GDI printer, so
// sivuTuloste returns at :930. tall_elahto changes E -> '-' and returns.
TEST_CASE("C6 H-091 control: tall_elahto saves the start without page printing")
{
	printFixture();
	lisaToimetOn = true;
	sivutulostusparam.printer = 0;
	sivutulostusparam.lstf = (PRFILE *) g_fakePrinter;
	tp::ThreadRun r = tp::runOnThread(runElahto, 0, 10000, "A");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	kilptietue k;
	k.GETREC(1);
	CHECK(k.tark(0) == L'-');
	CHECK(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);
	CHECK(tp::lockFreeForOtherThread(&print_CriticalSection) == 1);
}

// Edge tall -> print. lisaToimetOn, GDIPRINTER and a non-NULL lstf are what
// the GUI form cbTp/UnitSivuTuloste.cpp:92-98 sets; the console program never
// sets lisaToimetOn. The test holds print; A runs tall_elahto (tall :572)
// -> tallenna -> lisaToimet -> sivuTuloste and waits for print (:932).
TEST_CASE("C6 H-091 edge tall to print: tall_elahto holds tall and waits for print")
{
	printFixture();
	lisaToimetOn = true;
	sivutulostusparam.printer = GDIPRINTER;
	sivutulostusparam.lstf = (PRFILE *) g_fakePrinter;
	EnterCriticalSection(&print_CriticalSection);
	DWORD me = GetCurrentThreadId();
	tp::ThreadRun r = tp::runOnThread(runElahto, 0, 3000, "A");
	REQUIRE(r.outcome == tp::THREAD_BLOCKED);
	REQUIRE(tp::lockOwnerThread(&print_CriticalSection) == me);
	DWORD tallOwner = tp::lockOwnerThread(&tall_CriticalSection);
	int w = csWaiters(&print_CriticalSection);
	bool observed = tallOwner == r.threadId && w == 1 && threadAlive(r.threadId);
	tp::reportResult("H-091", "edge-tall-then-print", observed,
		tp::fmt("A=%lu blocked >= %lu ms; %s; %s (holder = test thread %lu); lisaToimetOn and GDIPRINTER "
			"set by the probe (GUI-only settings)", (unsigned long) r.threadId, (unsigned long) r.elapsedMs,
			csState("tall", &tall_CriticalSection).c_str(), csState("print", &print_CriticalSection).c_str(),
			(unsigned long) me));
}

// Automatic printing set up as the console menu does (HkConsole/HkCons3.cpp
// :819-825: autotulostusparam from the list settings, kohde 'P'), printing
// to a text file in the sandbox (TXTTAB).
static bool autotulFixture()
{
	printFixture();
	autofl = 1;
	tauko = 0;
	spoolfl = 0;
	kynnys = 1;
	prior[0] = 1;
	Sarjat[0].vakanttisarja = 0;
	static wchar_t fname[] = L"autotul.txt";
	autotulostusparam.printer = TXTTAB;
	autotulostusparam.kohde = L'P';
	autotulostusparam.lstf = openprfile(fname, -1, 0, 0, NULL, 0);
	return autotulostusparam.lstf != NULL;
}

TEST_CASE("C6 H-091 control: autotulostus prints a class with free locks")
{
	REQUIRE(autotulFixture());
	tp::ThreadRun r = tp::runOnThread(runAutotul, 0, 10000, "B");
	REQUIRE(r.outcome == tp::THREAD_FINISHED);
	CHECK(tp::lockFreeForOtherThread(&tall_CriticalSection) == 1);
	CHECK(tp::lockFreeForOtherThread(&print_CriticalSection) == 1);
	CHECK(tp::lockFreeForOtherThread(&autotul_CriticalSection) == 1);
	tp::info(tp::fmt("control: prior[0]=%d after the run (0 = class printed and priority reset)", prior[0]));
}

// Edge print -> tall: the test holds tall; B runs autotulostus, takes
// autotul (:985) and print (:1004), and waits for tall in tulostasarja
// (:5850).
TEST_CASE("C6 H-091 edge print to tall: autotulostus holds print and waits for tall")
{
	REQUIRE(autotulFixture());
	EnterCriticalSection(&tall_CriticalSection);
	DWORD me = GetCurrentThreadId();
	tp::ThreadRun r = tp::runOnThread(runAutotul, 0, 3000, "B");
	REQUIRE(r.outcome == tp::THREAD_BLOCKED);
	REQUIRE(tp::lockOwnerThread(&tall_CriticalSection) == me);
	DWORD printOwner = tp::lockOwnerThread(&print_CriticalSection);
	int w = csWaiters(&tall_CriticalSection);
	bool observed = printOwner == r.threadId && w == 1 && threadAlive(r.threadId);
	tp::reportResult("H-091", "edge-print-then-tall", observed,
		tp::fmt("B=%lu blocked >= %lu ms; %s; %s; %s (holder = test thread %lu)", (unsigned long) r.threadId,
			(unsigned long) r.elapsedMs, csState("print", &print_CriticalSection).c_str(),
			csState("autotul", &autotul_CriticalSection).c_str(), csState("tall", &tall_CriticalSection).c_str(),
			(unsigned long) me));
}
