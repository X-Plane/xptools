## Process this file with automake to generate Makefile.in.

LIBS = 
INCLUDES = -I$(top_srcdir)/include

if BUILD_WITH_MSVC
noinst_LIBRARIES = tables.lst
else
noinst_LTLIBRARIES = libtables.la
endif

## All source files used when building this submodule, including all
## headers.

TablesSources = \
	LayerTable.cpp LayerTable.h \
	Table.cpp Table.h \
	TableEntry.cpp TableEntry.h \
	UCSTable.cpp UCSTable.h \
	UnknownTable.cpp UnknownTable.h

libtables_la_SOURCES = \
	$(TablesSources)

tables_lst_SOURCES = \
	$(TablesSources)

tables.lst: Makefile $(tables_lst_OBJECTS)
	@echo "Linking tables.lst..."; \
	rm -f $@; \
	for i in $(tables_lst_OBJECTS); do echo $$i >>$@; done

libtablesincdir = $(includedir)/dime/tables
libtablesinc_HEADERS = \
	../../include/dime/tables/LayerTable.h \
	../../include/dime/tables/Table.h \
	../../include/dime/tables/TableEntry.h \
	../../include/dime/tables/UCSTable.h \
	../../include/dime/tables/UnknownTable.h

install-libtablesincHEADERS: $(libtablesinc_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(libtablesincdir)
	@list='$(libtablesinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(libtablesincdir)/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(libtablesincdir)/$$f; \
	done

uninstall-libtablesincHEADERS:
	@$(NORMAL_UNINSTALL)
	@list='$(libtablesinc_HEADERS)'; for p in $$list; do \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " rm -f $(DESTDIR)$(libtablesincdir)/$$f"; \
	  rm -f $(DESTDIR)$(libtablesincdir)/$$f; \
	done

