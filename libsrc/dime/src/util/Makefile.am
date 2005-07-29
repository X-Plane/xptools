## Process this file with automake to generate Makefile.in.

LIBS = 
INCLUDES = -I$(top_srcdir)/include

if BUILD_WITH_MSVC
noinst_LIBRARIES = util.lst
else
noinst_LTLIBRARIES = libutil.la
endif

UtilSources = \
	Array.cpp Array.h \
	BSPTree.cpp BSPTree.h \
	Box.cpp Box.h \
	Dict.cpp Dict.h \
	Linear.cpp Linear.h \
	MemHandler.cpp MemHandler.h

libutil_la_SOURCES = \
	$(UtilSources)

util_lst_SOURCES = \
	$(UtilSources)

util.lst: Makefile $(util_lst_OBJECTS)
	@echo "Linking util.lst..."; \
	rm -f $@; \
	for i in $(util_lst_OBJECTS); do echo $$i >>$@; done

## Installation information.

libutilincdir = $(includedir)/dime/util
libutilinc_HEADERS = \
	../../include/dime/util/Array.h \
	../../include/dime/util/BSPTree.h \
	../../include/dime/util/Box.h \
	../../include/dime/util/Dict.h \
	../../include/dime/util/Linear.h \
	../../include/dime/util/MemHandler.h

install-libutilincHEADERS: $(libutilinc_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(libutilincdir)
	@list='$(libutilinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(libutilincdir)/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(libutilincdir)/$$f; \
	done

uninstall-libutilincHEADERS:
	@$(NORMAL_UNINSTALL)
	@list='$(libutilinc_HEADERS)'; for p in $$list; do \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " rm -f $(DESTDIR)$(libutilincdir)/$$f"; \
	  rm -f $(DESTDIR)$(libutilincdir)/$$f; \
	done

