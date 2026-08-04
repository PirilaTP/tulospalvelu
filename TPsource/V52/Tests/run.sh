#!/bin/sh
# Kaantaa ja ajaa yksikkotestit g++:lla. Aja hakemistosta TPsource/V52:
#   ./Tests/run.sh
#
# Tama on kehityssilmukan nopea polku; Windowsilla kaytetaan VS/Tests/TpTest.sln.
set -e

OUT=${OUT:-./TpTest}

${CXX:-g++} -Wall -o "$OUT" \
	-I include -I Tp -I Tests \
	Tests/DoctestMain.cpp \
	Tests/TulkSITest.cpp \
	Tp/SITulkinta.cpp \
	tputilv2/T_time_l.cpp

exec "$OUT" "$@"
