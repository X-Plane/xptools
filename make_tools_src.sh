#!/bin/sh

### WED

rm ../XPWebSite/scenery/code/xptools_src_$1.zip

zip -9 -r ../XPWebSite/scenery/code/xptools_src_$1.zip \
	README.txt \
	*.sh \
	SDK \
	libsrc \
	*.xcodeproj \
	*.mcp \
	OpenGL\ SDK \
	OpenGL-1.2 \
	TigerTools.mcp \
	SceneryToolsWin \
	src \
	 -x \*.DS_Store \*.xSYM \*CVS\* \*.o \*.lo \*.la \*.dylib \*.a \*.lib \*boost_1_31_0\* \
	 | tee file_list.txt
