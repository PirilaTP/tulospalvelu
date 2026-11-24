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

// Relay (Juk) core fixture for TpCoreJuk. Reproduces the parts of the
// relay start-up (Juk/VInit.cpp aloitus) that a test needs, without the
// console, configuration files or communication threads.
// Must be included after VDeclare.h.

#ifndef TP_VFIXTURE_H
#define TP_VFIXTURE_H

// Defined in Juk/VInit.cpp but not declared in any header.
void asetaOletusarvot(void);

namespace tpv {

// Common base state: all critical sections the relay start-up creates
// (Juk/VInit.cpp:2557-2566), quiet UI (aani = 0, status_on = 0) and the
// default parameters (asetaOletusarvot, Juk/VInit.cpp:2460).
void initBase();

// Record sizes as computed after the series are read
// (Juk/VInit.cpp:2661-2662).
void computeRecordSizes();

}  // namespace tpv

#endif
