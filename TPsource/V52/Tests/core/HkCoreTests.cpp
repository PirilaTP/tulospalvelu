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

// HK core component tests (TpCoreHk). First stage: only a smoke test that
// shows the HK core links and runs in the sandbox, and records the layout
// constants of this build. The HK defect tests (H-041, H-031, H-008) come
// in the next stage (doc/arkkitehtuuri/testaus.md, section 7).

#include "../doctest.h"
#include "../fixtures/TpCore.h"
#include "HkDeclare.h"

TEST_CASE("TpCoreHk smoke: sandbox and build constants")
{
	REQUIRE(tp::sandboxOk());
	tp::info(tp::fmt("N_PV=%d VALUKU=%d KILPBUFSIZE=%d", (int) N_PV, (int) VALUKU,
		(int) KILPBUFSIZE));
	CHECK(tp::uiMessages().empty());
}
