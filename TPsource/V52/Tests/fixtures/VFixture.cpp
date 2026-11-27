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

// Relay core fixture, see VFixture.h.

#include "TpCore.h"
#include "VDeclare.h"
#include "VFixture.h"

namespace tpv {

void initBase()
{
	static bool done = false;
	if (done)
		return;
	done = true;
	// Same set as Juk/VInit.cpp:2557-2566 and :2632.
	InitializeCriticalSection(&tall_CriticalSection);
	InitializeCriticalSection(&loki_CriticalSection);
	InitializeCriticalSection(&inb_CriticalSection);
	InitializeCriticalSection(&outb_CriticalSection);
	InitializeCriticalSection(&kilpdat_CriticalSection);
	InitializeCriticalSection(&ajat_CriticalSection);
	InitializeCriticalSection(&emit_CriticalSection);
	InitializeCriticalSection(&rastiva_CriticalSection);
	InitializeCriticalSection(&tcpip_CriticalSection);
	InitializeCriticalSection(&autotul_CriticalSection);
	InitializeCriticalSection(&print_CriticalSection);
	aani = 0;
	status_on = 0;
	asetaOletusarvot();
}

void computeRecordSizes()
{
	kilpparam.osrecsize = kilpparam.osrecsize0 + (kilpparam.valuku + 1) * kilpparam.vatpsize;
	kilpparam.kilprecsize = kilpparam.kilprecsize0 + kilpparam.n_os * kilpparam.osrecsize;
}

}  // namespace tpv
