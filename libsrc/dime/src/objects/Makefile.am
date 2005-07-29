## Process this file with automake to generate Makefile.in.

LIBS = 
INCLUDES = -I$(top_srcdir)/include

if BUILD_WITH_MSVC
noinst_LIBRARIES = objects.lst
else
noinst_LTLIBRARIES = libobjects.la
endif

## All source files used when building this submodule, including all
## headers.

ObjectsSources = \
	Object.cpp Object.h \
	UnknownObject.cpp UnknownObject.h

libobjects_la_SOURCES = \
	$(ObjectsSources)

objects_lst_SOURCES = \
	$(ObjectsSources)

objects.lst: Makefile $(objects_lst_OBJECTS)
	@echo "Linking objects.lst..."; \
	rm -f $@; \
	for i in $(objects_lst_OBJECTS); do echo $$i >>$@; done


## Installation information.

libobjectsincdir = $(includedir)/dime/objects
libobjectsinc_HEADERS = \
	../../include/dime/objects/Object.h \
	../../include/dime/objects/UnknownObject.h

install-libobjectsincHEADERS: $(libobjectsinc_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(libobjectsincdir)
	@list='$(libobjectsinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(libobjectsincdir)/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(libobjectsincdir)/$$f; \
	done

uninstall-libobjectsincHEADERS:
	@$(NORMAL_UNINSTALL)
	@list='$(libobjectsinc_HEADERS)'; for p in $$list; do \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " rm -f $(DESTDIR)$(libobjectsincdir)/$$f"; \
	  rm -f $(DESTDIR)$(libobjectsincdir)/$$f; \
	done

