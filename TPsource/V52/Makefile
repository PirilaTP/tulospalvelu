
# HkMaali Linux/macOS build - adapted from VS/Hk/HkMaali/HkMaali.vcxproj
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  # Build x86_64 on macOS - legacy code uses #pragma pack(1) with pointers,
  # which causes unaligned access crashes on ARM64. Rosetta handles this fine.
  CXX = g++
  PLATFORM_FLAGS = -arch x86_64
  PLATFORM_LDFLAGS = -arch x86_64
else
  CXX = g++
  PLATFORM_FLAGS = -fno-pie -fno-PIC -mcmodel=large
  PLATFORM_LDFLAGS = -no-pie
endif
CXXFLAGS = -std=c++11 -O2 -g -w -fpermissive -Wno-error $(PLATFORM_FLAGS) \
           -include include/win_stubs/windows.h -include sys/stat.h \
           -D_fastcall= -D__linux__ -D_CONSOLE -DBIT32 -DEMIT \
           -DGAZ -DNREGNLY=32 -DMAX_LAHPORTTI=64 -DAUTOFILE -DEMITFILE \
           -DMAALI -DKONETUNN -DTCPSIIRTO -DSERVER -DENNAKOI -DAUTOVA \
           -DVALUKIJA -DUUSI -DPISTEET -DORAVA -DSEURAVAL -DMAAKNT \
           -DEMITHTTP -DTCPLUKIJA -DLAJUNEN -DVERTAA -DSAKKOCOM -DAIKAVERT \
           -DUDPYHTEYS -DEMITLEIMAT
INCLUDES = -Iinclude/win_stubs -Iinclude -IHkConsole -IHk -ITp

# dbboxm library sources
DBBOX_SRC = dbboxm/dbbox1.cpp

# tputilv2 library sources (case-insensitive filesystem issues - use wildcard)
TPUTIL_DIR = tputilv2 TpUtilV2 TPUTILv2

# HkMaali sources
HK_SRC = HkConsole/HkMain.cpp HkConsole/HkCons0.cpp HkConsole/HkCons2.cpp \
         HkConsole/HkCons3.cpp HkConsole/HkConsHelp.cpp HkConsole/HkKilp.cpp \
         Hk/HkAjat.cpp Hk/HkApu.cpp Hk/HkCom32.cpp Hk/HkDat.cpp \
         Hk/HkEmit.cpp Hk/HkGlobals.cpp Hk/HkIV.cpp Hk/HkInit.cpp \
         Hk/HkIx.cpp Hk/HkMuotoilu.cpp Hk/HkSak.cpp Hk/HkSrj.cpp \
         Hk/HkTls.cpp Hk/HkTulostus.cpp Hk/HkXml.cpp Hk/HkXmlTulokset.cpp \
         Hk/HkXml_IOF30.cpp \
         Tp/TpApu.cpp Tp/TpComY.cpp Tp/TpLaitteet.cpp \
         com/WinUDP.cpp com/gethttp.cpp com/winCom.cpp \
         tputilv2/xmlparser.cpp

# All tputilv2 .cpp files except GUI/test-specific ones
TPUTIL_SRC = $(filter-out tputilv2/LibTest.cpp tputilv2/UnitTest.cpp tputilv2/TextFileClass-2.cpp \
             tputilv2/Utf8Conv.cpp tputilv2/Viwrrect.cpp tputilv2/viwrrectw.cpp tputilv2/virdrect.cpp \
             tputilv2/Wrttext.cpp tputilv2/scbox.cpp tputilv2/drawlines.cpp \
             tputilv2/winversion.cpp tputilv2/processspecialkey.cpp \
             tputilv2/kbflush.cpp tputilv2/globals.cpp tputilv2/clearfra.cpp \
             tputilv2/aakjarjws2-v.cpp tputilv2/aakjarjs2-v.cpp \
             tputilv2/aakjarjws-v.cpp tputilv2/aakjarjs-v.cpp \
             tputilv2/aakjarjws2-2.cpp tputilv2/aakjarjs2-2.cpp \
             tputilv2/aakjarjws-2.cpp tputilv2/aakjarjs-2.cpp \
             tputilv2/Inputstr.cpp, \
             $(wildcard tputilv2/*.cpp))

ALL_SRC = $(HK_SRC) $(DBBOX_SRC) $(TPUTIL_SRC)
ALL_OBJ = $(patsubst %.cpp,build/%.o,$(ALL_SRC))

TARGET = HkMaali

.PHONY: all clean first-try

all: $(TARGET)

# Quick test: just try to compile one file to see errors
first-try:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c Hk/HkGlobals.cpp -o /dev/null 2>&1 | head -30

$(TARGET): $(ALL_OBJ)
	$(CXX) $(CXXFLAGS) $(PLATFORM_LDFLAGS) -o $@ $^ -lpthread

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf build $(TARGET)
