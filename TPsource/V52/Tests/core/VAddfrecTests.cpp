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

// H-040: relay addfrec() enters kilpdat_CriticalSection twice and never
// leaves it (Juk/vdat.cpp:178-192). Plan: doc/arkkitehtuuri/testaus.md 5.1.
//
// addfrec is static, so this file includes Juk/vdat.cpp itself; the
// TpCoreJuk project leaves vdat.cpp out of its source list. Nothing in
// vdat.cpp is changed.

// Standard and helper headers first (Juk/VDef.h:29 defines __fastcall away).
#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include "../../Juk/vdat.cpp"
#include "../fixtures/VFixture.h"

// Opens an empty KILP.DAT (dataf2name, relative) in the sandbox and a
// kilparr large enough for the records the test adds.
static void openEmptyKilpDat()
{
	tpv::initBase();
	tpv::computeRecordSizes();
	REQUIRE(tp::fileSize(dataf2name) == -1);
	REQUIRE(makefile(&datf2, dataf2name, kilpparam.kilprecsize) == 0);
	kilparr = (char *) calloc(16, kilpparam.kilprecsize);
	REQUIRE(kilparr != NULL);
}

// Control: the probe distinguishes a free lock from a lock held by this
// thread, and delfrec (Juk/vdat.cpp:194-204), the other production user of
// kilpdat_CriticalSection, leaves the lock free. The record it deletes is
// added with dbboxm addrec directly, not with addfrec. Without this control
// the H-040 result would not mean anything.
TEST_CASE("H-040 control: lock probe and balanced lock use")
{
	REQUIRE(tp::sandboxOk());
	tpv::initBase();
	CHECK(tp::lockFreeForOtherThread(&kilpdat_CriticalSection) == 1);
	EnterCriticalSection(&kilpdat_CriticalSection);
	CHECK(tp::lockFreeForOtherThread(&kilpdat_CriticalSection) == 0);
	LeaveCriticalSection(&kilpdat_CriticalSection);
	CHECK(tp::lockFreeForOtherThread(&kilpdat_CriticalSection) == 1);

	openEmptyKilpDat();
	kilptietue k;
	k.nollaa();
	char buf[KILPRECSIZE];
	DATAREF d = 0;
	addrec(&datf2, &d, k.pack(buf));
	REQUIRE(d > 0);
	REQUIRE(d < datf2.numrec);
	delfrec((int) d);
	CHECK(tp::lockFreeForOtherThread(&kilpdat_CriticalSection) == 1);
	CHECK(kilpdat_CriticalSection.RecursionCount == 0);
	CHECK(tp::uiMessages().empty());
}

// Correct behaviour: after addfrec() returns, another thread can take
// kilpdat_CriticalSection. Defect mechanism: the lock is still owned by the
// calling thread, with two recursive acquisitions and no release.
TEST_CASE("H-040 relay addfrec leaves kilpdat_CriticalSection held")
{
	REQUIRE(tp::sandboxOk());
	openEmptyKilpDat();
	REQUIRE(tp::lockFreeForOtherThread(&kilpdat_CriticalSection) == 1);

	kilptietue k;
	k.nollaa();
	int d = -1;
	addfrec(&k, &d);

	// Preconditions: the record was really added (file grew, reopen OK).
	REQUIRE(d > 0);
	REQUIRE((UINT32) d < datf2.numrec);
	REQUIRE(tp::uiMessages().empty());

	int probe = tp::lockFreeForOtherThread(&kilpdat_CriticalSection);
	REQUIRE(probe >= 0);
	DWORD owner = (DWORD) (ULONG_PTR) kilpdat_CriticalSection.OwningThread;
	LONG recursion = kilpdat_CriticalSection.RecursionCount;
	bool ownedByMe = owner == GetCurrentThreadId();
	bool observed = probe == 0 && ownedByMe;
	tp::reportResult("H-040", "addfrec-lock-not-released", observed,
		tp::fmt("d=%d numrec=%u other_thread_got_lock=%d owner_is_caller=%d "
			"RecursionCount=%ld (internal field; 2 = two Enter, no Leave)",
			d, (unsigned) datf2.numrec, probe, ownedByMe ? 1 : 0, (long) recursion));
}
