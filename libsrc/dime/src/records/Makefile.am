## Process this file with automake to generate Makefile.in.

LIBS = 
INCLUDES = -I$(top_srcdir)/include

if BUILD_WITH_MSVC
noinst_LIBRARIES = records.lst
else
noinst_LTLIBRARIES = librecords.la
endif

RecordsSources = \
	DoubleRecord.cpp DoubleRecord.h \
	FloatRecord.cpp FloatRecord.h \
	HexRecord.cpp HexRecord.h \
	Int16Record.cpp Int16Record.h \
	Int32Record.cpp Int32Record.h \
	Int8Record.cpp Int8Record.h \
	Record.cpp Record.h \
	StringRecord.cpp StringRecord.h

librecords_la_SOURCES = \
	$(RecordsSources)

records_lst_SOURCES = \
	$(RecordsSources)

records.lst: Makefile $(records_lst_OBJECTS)
	@echo "Linking records.lst..."; \
	rm -f $@; \
	for i in $(records_lst_OBJECTS); do echo $$i >>$@; done


## Installation information.

librecordsincdir = $(includedir)/dime/records
librecordsinc_HEADERS = \
	../../include/dime/records/DoubleRecord.h \
	../../include/dime/records/FloatRecord.h \
	../../include/dime/records/HexRecord.h \
	../../include/dime/records/Int16Record.h \
	../../include/dime/records/Int32Record.h \
	../../include/dime/records/Int8Record.h \
	../../include/dime/records/Record.h \
	../../include/dime/records/StringRecord.h

install-librecordsincHEADERS: $(librecordsinc_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(librecordsincdir)
	@list='$(librecordsinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(librecordsincdir)/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(librecordsincdir)/$$f; \
	done

uninstall-librecordsincHEADERS:
	@$(NORMAL_UNINSTALL)
	@list='$(librecordsinc_HEADERS)'; for p in $$list; do \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " rm -f $(DESTDIR)$(librecordsincdir)/$$f"; \
	  rm -f $(DESTDIR)$(librecordsincdir)/$$f; \
	done

