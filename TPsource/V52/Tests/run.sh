#!/bin/sh
# Kaantaa ja ajaa yksikkotestit g++:lla. Aja hakemistosta TPsource/V52:
#   ./Tests/run.sh
#
# Tama on kehityssilmukan nopea polku; Windowsilla kaytetaan VS/Tests/TpTest.sln.
set -e

OUT=${OUT:-./TpTest}

${CXX:-g++} -Wall -o "$OUT" \
	-I include -I Tp -I Juk -I Tests \
	Tests/DoctestMain.cpp \
	Tests/VRinnakkaisOsuusTest.cpp \
	Juk/VRinnakkaisOsuus.cpp

exec "$OUT" "$@"
