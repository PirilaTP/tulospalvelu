#!/bin/sh
# usage: build.sh <mode base|w4a> <name> <vcxproj> <config>
MSB="/c/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/amd64/MSBuild.exe"
S=C:/Users/KAYTTA~1/AppData/Local/Temp/claude/a25cf22582434270/4e70ca9a-c468-40d9-9c1d-031b60074777/scratchpad/c4
mode=$1; name=$2; proj=$3; cfg=$4
out="$S/out/$mode/$name/$cfg/"
extra="-p:ForceImportAfterCppTargets=$S/base.props"
[ "$mode" = w4a ] && extra="-p:ForceImportAfterCppTargets=$S/w4analyze.props"
mkdir -p "$S/logs"
"$MSB" "$proj" -t:ClCompile -p:PlatformToolset=v145 -p:Configuration=$cfg -p:Platform=Win32 "-p:OutDir=$out" "-p:IntDir=$out" -m -nologo -v:m \
  ${extra:+"$extra"} "-flp1:logfile=$S/logs/$mode-$name-$cfg.log;verbosity=normal;encoding=utf-8" \
  "-flp2:logfile=$S/logs/$mode-$name-$cfg.warn.log;warningsonly;encoding=utf-8" > "$S/logs/$mode-$name-$cfg.console.txt" 2>&1
echo "$mode $name $cfg exit=$?"
