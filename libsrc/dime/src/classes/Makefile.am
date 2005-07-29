## Process this file with automake to generate Makefile.in.

INCLUDES = -I$(top_srcdir)/include
LIBS = 

if BUILD_WITH_MSVC
noinst_LIBRARIES = classes.lst
else
noinst_LTLIBRARIES = libclasses.la
endif

ClassesSources = \
	Class.cpp Class.h \
	UnknownClass.cpp UnknownClass.h

libclasses_la_SOURCES = \
	$(ClassesSources)

classes_lst_SOURCES = \
	$(ClassesSources)

classes.lst: Makefile $(classes_lst_OBJECTS)
	@echo "Linking classes.lst..."; \
	rm -f $@; \
	for i in $(classes_lst_OBJECTS); do echo $$i >>$@; done

classesincdir = $(includedir)/dime/classes
classesinc_HEADERS = \
	../../include/dime/classes/Class.h \
	../../include/dime/classes/UnknownClass.h

install-classesincHEADERS: $(classesinc_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(classesincdir)
	@list='$(classesinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(classesincdir)/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(classesincdir)/$$f; \
	done

uninstall-classesincHEADERS:
	@$(NORMAL_UNINSTALL)
	@list='$(classesinc_HEADERS)'; for p in $$list; do \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " rm -f $(DESTDIR)$(classesincdir)/$$f"; \
	  rm -f $(DESTDIR)$(classesincdir)/$$f; \
	done

