#
# Makefile for spin compiler
# Copyright (c) 2011-2018 Total Spectrum Software Inc.
# Distributed under the MIT License (see COPYING for details)
#
# if CROSS is defined, we are building a cross compiler
# possible targets are: win32, rpi
# Note that you may have to adjust your compiler names depending on
# which Linux distribution you are using (e.g. ubuntu uses
# "i586-mingw32msvc-gcc" for mingw, whereas Debian uses
# "i686-w64-mingw32-gcc"
#
# to build a release .zip, do "make zip" and "make spincvt.zip"
#

ifeq ($(CROSS),win32)
#  CC=i586-mingw32msvc-gcc
  CC=i686-w64-mingw32-gcc
  EXT=.exe
  BUILD=./build-win32
else ifeq ($(CROSS),rpi)
  CC=arm-linux-gnueabihf-gcc
  EXT=
  BUILD=./build-rpi
else ifeq ($(CROSS),linux32)
  CC=gcc -m32
  EXT=
  BUILD=./build-linux32
else
  CC=gcc
  EXT=
  BUILD=./build
endif

INC=-I. -I$(BUILD)

# byacc will fail some of the error tests, but mostly works
#YACC = byacc -s
YACC = bison
CFLAGS = -g -Wall $(INC)
#CFLAGS = -g -Og -Wall -Wc++-compat -Werror $(INC)
LIBS = -lm
RM = rm -rf

VPATH=.:util:frontends:frontends/basic:frontends/spin:frontends/c:backends:backends/asm:backends/cpp:backends/dat

LEXHEADERS = $(BUILD)/spin.tab.h $(BUILD)/basic.tab.h $(BUILD)/cgram.tab.h ast.h frontends/common.h

PROGS = $(BUILD)/testlex$(EXT) $(BUILD)/spin2cpp$(EXT) $(BUILD)/fastspin$(EXT)

UTIL = dofmt.c flexbuf.c lltoa_prec.c strupr.c strrev.c

# FIXME lexer should not need cppexpr.c (it belongs in CPPBACK)
LEXSRCS = lexer.c symbol.c ast.c expr.c $(UTIL) preprocess.c cppexpr.c
PASMBACK = outasm.c assemble_ir.c optimize_ir.c inlineasm.c
CPPBACK = outcpp.c cppfunc.c outgas.c # cppexpr.c
SPINSRCS = common.c spinc.c $(LEXSRCS) functions.c cse.c loops.c pasm.c outdat.c outlst.c spinlang.c basiclang.c $(PASMBACK) $(CPPBACK)

LEXOBJS = $(LEXSRCS:%.c=$(BUILD)/%.o)
SPINOBJS = $(SPINSRCS:%.c=$(BUILD)/%.o)
OBJS = $(SPINOBJS) $(BUILD)/spin.tab.o $(BUILD)/basic.tab.o $(BUILD)/cgram.tab.o

SPIN_CODE = sys/p1_code.spin.h sys/p2_code.spin.h sys/common.spin.h sys/float.spin.h sys/gcalloc.spin.h

all: $(BUILD) $(PROGS)

$(BUILD)/testlex$(EXT): testlex.c $(LEXOBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD)/spin.tab.c $(BUILD)/spin.tab.h: frontends/spin/spin.y
	$(YACC) -p spinyy -t -b $(BUILD)/spin -d frontends/spin/spin.y

$(BUILD)/basic.tab.c $(BUILD)/basic.tab.h: frontends/basic/basic.y
	$(YACC) -p basicyy -t -b $(BUILD)/basic -d frontends/basic/basic.y

$(BUILD)/cgram.tab.c $(BUILD)/cgram.tab.h: frontends/c/cgram.y
	$(YACC) -p cgramyy -t -b $(BUILD)/cgram -d frontends/c/cgram.y

$(BUILD)/spinc.o: spinc.c $(SPIN_CODE)

preproc: preprocess.c $(UTIL)
	$(CC) $(CFLAGS) -DTESTPP -o $@ $^ $(LIBS)

clean:
	$(RM) $(PROGS) $(BUILD)/* *.zip

test: lextest asmtest cpptest errtest p2test runtest
#test: lextest asmtest cpptest errtest runtest

lextest: $(PROGS)
	$(BUILD)/testlex

asmtest: $(PROGS)
	(cd Test; ./asmtests.sh)

cpptest: $(PROGS)
	(cd Test; ./cpptests.sh)

errtest: $(PROGS)
	(cd Test; ./errtests.sh)

p2test: $(PROGS)
	(cd Test; ./p2bin.sh)

runtest: $(PROGS)
	(cd Test; ./runtests.sh)

$(BUILD)/spin2cpp$(EXT): spin2cpp.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD)/fastspin$(EXT): fastspin.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/spin.tab.o: $(BUILD)/spin.tab.c $(LEXHEADERS)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/basic.tab.o: $(BUILD)/basic.tab.c $(LEXHEADERS)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/cgram.tab.o: $(BUILD)/cgram.tab.c $(LEXHEADERS)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/lexer.o: frontends/lexer.c $(LEXHEADERS)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/%.o: %.c
	$(CC) -MMD -MP $(CFLAGS) -o $@ -c $<

#
# convert a .spin file to a header file
# note that xxd does not 0 terminate its generated string,
# which is what the sed script will do
#
sys/%.spin.h: sys/%.spin
	xxd -i $< | sed 's/\([0-9a-f]\)$$/\0, 0x00/' > $@
sys/%.bas.h: sys/%.bas
	xxd -i $< | sed 's/\([0-9a-f]\)$$/\0, 0x00/' > $@

#
# automatic dependencies
# these .d files are generated by the -MMD -MP flags to $(CC) above
# and give us the dependencies
# the "-" sign in front of include says not to give any error or warning
# if a file is not found
#
-include $(SPINOBJS:.o=.d)

#
# targets to build a .zip file for a release
#
spin2cpp.exe: .PHONY
	$(MAKE) CROSS=win32
	cp build-win32/spin2cpp.exe .

fastspin.exe: .PHONY
	$(MAKE) CROSS=win32
	cp build-win32/fastspin.exe .

spin2cpp.linux: .PHONY
	$(MAKE) CROSS=linux32
	cp build-linux32/spin2cpp ./spin2cpp.linux

COMMONDOCS=COPYING Changelog.txt docs
ALLDOCS=README.md Fastspin.md $(COMMONDOCS)

zip: fastspin.exe spin2cpp.exe
	zip -r spin2cpp.zip $(ALLDOCS) spin2cpp.exe fastspin.exe
	zip -r fastspin.zip fastspin.exe Fastspin.md docs

#
# target to build a windows spincvt GUI
#
FREEWRAP=/opt/freewrap/linux64/freewrap
FREEWRAPEXE=/opt/freewrap/win32/freewrap.exe

spincvt.zip: .PHONY
	rm -f spincvt.zip
	rm -rf spincvt
	$(MAKE) CROSS=win32
	mkdir -p spincvt/bin
	cp build-win32/spin2cpp.exe spincvt/bin
	cp propeller-load/propeller-load.exe spincvt/bin
	cp spinconvert/spinconvert.tcl spincvt
	mkdir -p spincvt/examples
	cp -rp spinconvert/examples/*.spin spincvt/examples
	cp -rp spinconvert/examples/*.def spincvt/examples
	cp -rp spinconvert/README.txt COPYING spincvt
	cp -rp docs spincvt
	(cd spincvt; $(FREEWRAP) spinconvert.tcl -w $(FREEWRAPEXE))
	rm spincvt/spinconvert.tcl
	zip -r spincvt.zip spincvt
	rm -rf spincvt


.PHONY:
