#***************************************************************************
#
# Makefile for DIME: Dxf Import, Manipulation, and Export Library
#
# Usage:
#       make 			Makes Dime release version
#	make shared		Makes Dime development version
#
#***************************************************************************

COPY = cp
SYMLINK = ln -sf

#***************************************************************************

all:: dime

screen:
	@echo -e "\033[34m\n" \
	"\t****************************************************************\n" \
	"\t*                       Compiling DIME                         *\n" \
	"\t****************************************************************\n" \
	"\033[0m";

dime: static
dime_mt: static_mt

static: makefile.release .release
static_mt: makefile.release_mt .release_mt

shared: makefile.dime .dime
shared_mt: makefile.dime_mt .dime_mt

objects:
	mkdir $@

releaseobjects:
	mkdir $@

.dime: objects
	@$(MAKE) -f makefile.dime

makefile.dime: dime.pro common.pro
	tmake -o makefile.dime dime.pro common.pro

.dime_mt: objects
	@$(MAKE) -f makefile.dime_mt

makefile.dime_mt: mt.pro dime.pro common.pro
	tmake -o makefile.dime_mt dime.pro mt.pro common.pro

.release: releaseobjects
	@$(MAKE) -f makefile.release

makefile.release: release.pro common.pro
	tmake -o makefile.release release.pro common.pro

.release_mt: releaseobjects
	@$(MAKE) -f makefile.release_mt

makefile.release_mt: mt.pro release.pro common.pro
	tmake -o makefile.release_mt release.pro mt.pro common.pro

clean: makefile.dime makefile.release makefile.dime_mt makefile.release_mt
	@$(MAKE) -f makefile.dime clean
	@$(MAKE) -f makefile.release clean
	@$(MAKE) -f makefile.dime_mt clean
	@$(MAKE) -f makefile.release_mt clean
	@$(RM) makefile.dime
	@$(RM) makefile.release
	@$(RM) makefile.dime_mt
	@$(RM) makefile.release_mt
