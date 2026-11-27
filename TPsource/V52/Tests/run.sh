#!/bin/sh
# Kaantaa ja ajaa yksikkotestit g++:lla. Aja hakemistosta TPsource/V52:
#   ./Tests/run.sh
#
# Tama on kehityssilmukan nopea polku; Windowsilla kaytetaan VS/Tests/TpTest.sln.
# Tiedostolistan pitaa vastata VS/Tests/TpTest.vcxproj:ia.
set -e

OUT=${OUT:-./TpTest}

${CXX:-g++} -Wall -o "$OUT" \
	-I include -I Tp -I Hk -I Juk -I Tests \
	Tests/DoctestMain.cpp \
	Tests/TulkSITest.cpp \
	Tests/SICenterJsonTest.cpp \
	Tests/SID3PunchTest.cpp \
	Tests/EmitBadgeTest.cpp \
	Tests/VRinnakkaisOsuusTest.cpp \
	Tests/HkEnnTavTest.cpp \
	Tp/SITulkinta.cpp \
	Tp/SICenterJson.cpp \
	Tp/SID3Punch.cpp \
	Tp/EmitBadge.cpp \
	Juk/VRinnakkaisOsuus.cpp \
	Hk/HkEnnTav.cpp \
	tputilv2/T_time_l.cpp \
	tputilv2/aikatowst_s.cpp \
	tputilv2/putint_r.cpp

exec "$OUT" "$@"
