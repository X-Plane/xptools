#***************************************************************************
#
# Makefile for DIME: Dxf Import, Manipulation, and Export Library
#
# Usage:
#       make 			Makes Dime release version
#	make debug		Makes Dime development version
#
#***************************************************************************

COPY = cp
SYMLINK = ln -sf

release: makefile.release .release

debug: makefile.dime .dime

objects:
	mkdir $@

releaseobjects:
	mkdir $@

.dime: objects
	@$(MAKE) -f makefile.dime

makefile.dime: dime.pro common.pro
	tmake -o makefile.dime dime.pro common.pro

.release: releaseobjects
	@$(MAKE) -f makefile.release

makefile.release: release.pro common.pro
	tmake -o makefile.release release.pro common.pro

clean: makefile.dime makefile.release
	@$(MAKE) -f makefile.dime clean
	@$(MAKE) -f makefile.release clean
	@$(RM) makefile.dime
	@$(RM) makefile.release
