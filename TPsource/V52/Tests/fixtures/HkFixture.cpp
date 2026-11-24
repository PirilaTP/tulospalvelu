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

// HK core fixture, see HkFixture.h.

#include "../doctest.h"
#include "TpCore.h"
#include "HkDeclare.h"
#include "HkFixture.h"

namespace tph {

void initBase()
{
	static bool done = false;
	if (done)
		return;
	done = true;
	// Hk/HkInit.cpp:3099-3110, :3206-3207
	InitializeCriticalSection(&idindex_CriticalSection);
	InitializeCriticalSection(&tall_CriticalSection);
	InitializeCriticalSection(&loki_CriticalSection);
	InitializeCriticalSection(&inb_CriticalSection);
	InitializeCriticalSection(&outb_CriticalSection);
	InitializeCriticalSection(&kilpdat_CriticalSection);
	InitializeCriticalSection(&ajat_CriticalSection);
	InitializeCriticalSection(&emit_CriticalSection);
	InitializeCriticalSection(&rastiva_CriticalSection);
	InitializeCriticalSection(&tcpip_CriticalSection);
	InitializeCriticalSection(&seura_CriticalSection);
	InitializeCriticalSection(&seur_badge_CriticalSection);
	InitializeCriticalSection(&print_CriticalSection);
	InitializeCriticalSection(&autotul_CriticalSection);
	aani = 0;
	status_on = 0;
	// HkConsole/HkMain.cpp:185-201 (wmain defaults; aloitus is not run)
	kilpparam.kilplaji = L'S';
	kilpparam.alalaji = L' ';
	kilpparam.badgelaji = L'E';
	kilpparam.lsnimi = LSNIMI;
	kilpparam.lenimi = LENIMI;
	kilpparam.larvo = LARVO;
	kilpparam.lseura = LSEURA;
	kilpparam.lseuralyh = LLYH;
	kilpparam.ljoukkue = LJK;
	kilpparam.lyhdistys = LYHD;
	kilpparam.lwrkoodi = LWRKOODI;
	kilpparam.maxasakko = 5;
	kilpparam.WisV = true;
	kilpparam.seuralyh_on = true;
	kilpparam.n_pv = 2;
	kilpparam.n_pv_akt = 1;
	kilpparam.valuku = 10;
	kilpparam.sakkoyks = RRSAKKO;
}

void setStages(int n_pv, int valuku)
{
	kilpparam.n_pv = n_pv;
	kilpparam.valuku = valuku;
	luekentat(0);
}

void makeEmptyKilpDat()
{
	static char cst[200];
	// Same conversion as Hk/HkInit.cpp:3480; dataf2name is L"KILP.DAT".
	wcstooem(cst, dataf2name, 100);
	REQUIRE(tp::fileSize(cst) == -1);
	REQUIRE(makefile(&datf2, cst, kilpparam.kilprecsize) == 0);
}

}  // namespace tph
