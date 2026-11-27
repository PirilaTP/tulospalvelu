// Pekka Pirila's sports timekeeping program (Finnish: tulospalveluohjelma)
// Copyright (C) 2015 Pekka Pirila

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Relay core component tests: H-025 (combufsize lacks the packet header)
// and H-042 (XML leg and split counts are not clamped).
// Plans: doc/arkkitehtuuri/testaus.md, sections 5.3 and 5.4.
// Run only through Tests/run_core_tests.py (one process per test case).

// Standard and helper headers first: Juk/VDef.h:29 defines __fastcall away,
// which breaks C++ standard headers included after it.
#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include <stddef.h>
#include <io.h>
#include "VDeclare.h"
#include "../fixtures/VFixture.h"

// Defined in Juk/VCom.cpp / Tp/TpComY.cpp / Juk/VInit.cpp, not in headers.
extern int class_len[];
extern int combufsize;
void init_class_len(void);
int opencomfile(int vaihto);
void buflah(int cn, combufrec *obuf);
int luesarjat(void);

// Number of packet classes, Juk/VCom.cpp:75 (N_CLASS is local to that file).
static const int TP_N_CLASS = 15;

// Bytes before the data union d in a combufrec: lahetetty 4, portti 2,
// so/id/iid 3, pkgclass 1, len 2, checksum 2 (Juk/VDef.h:1165-1171).
static int headerSize()
{
	return (int) offsetof(combufrec, d);
}

static int largestClass()
{
	int best = 0;
	for (int i = 1; i < TP_N_CLASS; i++)
		if (class_len[i] > class_len[best])
			best = i;
	return best;
}

// ---------------------------------------------------------------- H-025

// (a) Sizes. Correct behaviour (as in HK, Hk/HkCom32.cpp:1095-1105): every
// packet incl. its 14-byte header fits in combufsize. Defect mechanism:
// combufsize == max(class_len), i.e. the header was left out
// (Juk/VCom.cpp:1236-1240).
TEST_CASE("H-025a relay combufsize vs header plus largest class")
{
	REQUIRE(tp::sandboxOk());
	tpv::initBase();
	REQUIRE(headerSize() == 14);

	const int nosList[2] = { 4, MAXOSUUSLUKU };
	bool mechanismAll = true;
	std::string detail;
	for (int k = 0; k < 2; k++) {
		kilpparam.n_os = nosList[k];
		init_class_len();
		int big = largestClass();
		int maxLen = class_len[big];
		REQUIRE(maxLen > 0);
		int need = headerSize() + maxLen;
		bool missingHeader = (combufsize == maxLen) && (combufsize < need);
		mechanismAll = mechanismAll && missingHeader;
		detail += tp::fmt("n_os=%d: combufsize=%d largest class=%d len=%d header+len=%d; ",
			nosList[k], combufsize, big, maxLen, need);
		// Correct behaviour would hold for every class.
		for (int i = 0; i < TP_N_CLASS; i++)
			if (combufsize < headerSize() + class_len[i])
				detail += tp::fmt("class %d does not fit; ", i);
	}
	tp::reportResult("H-025", "combufsize-excludes-header", mechanismAll, detail);
}

// (b) Disk queue. For one configuration: two packets of the largest class
// are queued with the production buflah() into a fresh queue file in the
// sandbox (obfilename, as production does). Returns true when the defect
// mechanism is seen: the slot stride is combufsize, so the last
// (header + len - combufsize) bytes of each packet are not stored; the file
// position where they belong is the start of the next slot. The tail is
// not overwritten, only never written.
static bool diskQueueCase(int nos, const wchar_t *wname, const char *name, std::string &detail)
{
	kilpparam.n_os = nos;
	init_class_len();
	const int big = largestClass();
	const int len = class_len[big];
	const int hdr = headerSize();
	const int full = hdr + len;
	REQUIRE(len > 0);
	REQUIRE(tp::fileSize(name) == -1);   // an existing file would prompt

	wcscpy(obfilename, wname);
	levypusk = 1;
	maxyhteys = 1;
	cjseur[0] = 0;
	cjens[0] = 0;
	kirjheti = 0;
	obfile = -1;
	REQUIRE(opencomfile(0) == 1);
	REQUIRE(obfile >= 0);
	REQUIRE(tp::fileSize(name) == (long long) maxyhteys * combufsize);
	size_t uiBefore = tp::uiMessages().size();

	// Buffers are larger than both full and combufsize so that buflah()
	// can read combufsize bytes and the test can hold the whole packet.
	const int bufLen = (full > combufsize ? full : combufsize) + 64;
	std::vector<unsigned char> p1(bufLen, 0), p2(bufLen, 0);
	combufrec *c1 = (combufrec *) &p1[0];
	combufrec *c2 = (combufrec *) &p2[0];
	c1->pkgclass = c2->pkgclass = (UCHAR) big;
	c1->len = c2->len = (UINT16) len;
	c1->so = c2->so = 1;
	c1->id = 1;
	c2->id = 2;
	for (int i = 0; i < len; i++) {
		p1[hdr + i] = (unsigned char) (0x11 + i * 7);
		p2[hdr + i] = (unsigned char) (0xC3 - i * 5);
	}
	buflah(0, c1);
	buflah(0, c2);
	REQUIRE(cjseur[0] == 2);
	REQUIRE(tp::uiMessages().size() == uiBefore);  // no write error reported
	_close(obfile);
	obfile = -1;

	const long long stride = combufsize;
	const long long pos1 = (1LL * maxyhteys + 0) * stride;   // slot of packet 1
	const long long pos2 = (2LL * maxyhteys + 0) * stride;   // slot of packet 2
	const long long fsize = tp::fileSize(name);
	// Header row + two packet slots, each exactly one stride.
	REQUIRE(fsize == 3 * stride);
	const size_t pre = (size_t) (stride < full ? stride : full);
	std::vector<unsigned char> f1(full, 0), f2(full, 0);
	size_t n1 = tp::readAt(name, pos1, &f1[0], full);
	size_t n2 = tp::readAt(name, pos2, &f2[0], full);

	// Precondition: what was written is the packet prefix (buflah sets
	// lahetetty to 0, which it already is).
	REQUIRE(n1 >= pre);
	REQUIRE(n2 >= pre);
	REQUIRE(memcmp(&f1[0], &p1[0], pre) == 0);
	REQUIRE(memcmp(&f2[0], &p2[0], pre) == 0);

	bool whole1 = n1 == (size_t) full && memcmp(&f1[0], &p1[0], full) == 0;
	long long lost = full - stride;
	bool tailIsNextSlot = lost > 0 &&
		memcmp(&f1[(size_t) stride], &p2[0], (size_t) lost) == 0;
	detail += tp::fmt("n_os=%d class=%d len=%d header=%d packet=%d stride(combufsize)=%lld "
		"bytes_not_stored=%lld file_size=%lld (=3 strides) slot1_whole=%d "
		"slot1_tail_position_holds_slot2_header=%d; ", nos, big, len, hdr, full, stride,
		lost > 0 ? lost : 0, fsize, whole1 ? 1 : 0, tailIsNextSlot ? 1 : 0);
	return !whole1 && lost > 0 && tailIsNextSlot;
}

// Two configurations: n_os 4 (largest class EMITVA, sent only with the
// LAHEMVA setting) and n_os MAXOSUUSLUKU (largest class YHTEISLAHTOT).
// Correct behaviour: each slot holds header + len bytes of the packet.
TEST_CASE("H-025b relay disk queue stores whole packet")
{
	REQUIRE(tp::sandboxOk());
	tpv::initBase();
	std::string detail;
	bool o1 = diskQueueCase(4, L"COMFILE4.DAT", "COMFILE4.DAT", detail);
	bool o2 = diskQueueCase(MAXOSUUSLUKU, L"COMFILE.DAT", "COMFILE.DAT", detail);
	tp::reportResult("H-025", "disk-slot-truncates-packet", o1 && o2, detail);
}

// ---------------------------------------------------------------- H-042

// Control: the data path works for a file inside the compile-time limits,
// so a rejection of the over-limit file cannot be a fixture error.
TEST_CASE("H-042 control: in-limit KilpSrj.xml is read")
{
	REQUIRE(tp::sandboxOk());
	tpv::initBase();
	REQUIRE(tp::fileSize("KilpSrj.xml") > 0);
	wcscpy(sarjaxmlfname, L"KilpSrj.xml");
	int r = luesarjat();
	tp::info(tp::fmt("luesarjat=%d n_os=%d valuku=%d n_os_akt=%d sarjaluku=%d ui=%d",
		r, kilpparam.n_os, kilpparam.valuku, kilpparam.n_os_akt, sarjaluku,
		(int) tp::uiMessages().size()));
	CHECK(r == 0);
	CHECK(kilpparam.n_os == MAXOSUUSLUKU);
	CHECK(kilpparam.valuku == VAIKALUKU);
	CHECK(kilpparam.n_os_akt == MAXOSUUSLUKU);
	CHECK(tp::uiMessages().empty());
}

// Over-limit file: Legs = LegCount = MAXOSUUSLUKU+1, Intermediaries =
// VAIKALUKU+1 (data/juk/h042_over_limit.xml). Correct behaviour: the file is
// rejected (non-zero return) or the values are clamped. Defect mechanism:
// luesarjat() returns 0 and kilpparam holds values above the compile-time
// array sizes (Juk/VXml.cpp:442, :445, :482).
TEST_CASE("H-042 relay XML counts above compile-time limits")
{
	REQUIRE(tp::sandboxOk());
	tpv::initBase();
	REQUIRE(tp::fileSize("KilpSrj.xml") > 0);
	wcscpy(sarjaxmlfname, L"KilpSrj.xml");
	int r = luesarjat();
	bool over = kilpparam.n_os > MAXOSUUSLUKU || kilpparam.n_os_akt > MAXOSUUSLUKU ||
		kilpparam.valuku > VAIKALUKU;
	bool exact = kilpparam.n_os == MAXOSUUSLUKU + 1 && kilpparam.n_os_akt == MAXOSUUSLUKU + 1 &&
		kilpparam.valuku == VAIKALUKU + 1;
	// observed only if accepted AND the stored values are the file values.
	bool observed = r == 0 && over && exact;
	// Record size the start-up would derive next (Juk/VInit.cpp:2661-2662),
	// compared with the compile-time buffer size; reported only.
	tpv::computeRecordSizes();
	tp::reportResult("H-042", "xml-counts-not-clamped", observed,
		tp::fmt("luesarjat=%d n_os=%d (max %d) n_os_akt=%d valuku=%d (max %d) ui_messages=%d; "
			"derived kilprecsize=%d vs KILPRECSIZE=%d (overflow itself not run)",
			r, kilpparam.n_os, MAXOSUUSLUKU, kilpparam.n_os_akt, kilpparam.valuku, VAIKALUKU,
			(int) tp::uiMessages().size(), (int) kilpparam.kilprecsize, (int) KILPRECSIZE));
}
