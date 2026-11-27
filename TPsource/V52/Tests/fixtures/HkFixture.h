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

// HK (individual competition) core fixture for TpCoreHk. Reproduces the
// parts of the console start-up (HkConsole/HkMain.cpp wmain, Hk/HkInit.cpp
// aloitus) that a test needs, without console, configuration files or
// communication threads. Must be included after HkDeclare.h.
//
// Rule (Q34): every kilptietue, including kilparr, must be constructed
// after kilpparam.n_pv and kilpparam.valuku are final, because the
// constructor allocates pv[n_pv] and va[valuku+2].

#ifndef TP_HKFIXTURE_H
#define TP_HKFIXTURE_H

namespace tph {

// Critical sections as Hk/HkInit.cpp:3099-3110 and :3206-3207, aani = 0,
// status_on = 0, and the kilpparam defaults of HkConsole/HkMain.cpp:185-201.
void initBase();

// Sets n_pv / valuku and runs the production luekentat(0)
// (Hk/HkDat.cpp:105-178), which computes the record sizes.
void setStages(int n_pv, int valuku);

// Creates an empty KILP.DAT (dataf2name, relative to the sandbox) with the
// current record size and leaves datf2 open.
void makeEmptyKilpDat();

}  // namespace tph

#endif
