## Process this file with automake to generate Makefile.in.

CLEANFILES = \
        dime@DIME_MAJOR_VERSION@@SUFFIX@.dll \
        dime@DIME_MAJOR_VERSION@@SUFFIX@.pdb \
        dime@DIME_MAJOR_VERSION@@SUFFIX@.exp

DISTCLEANFILES = \
        ii_files/*.ii

SUBDIRS = classes entities objects records sections tables util convert .

INCLUDES = -I$(top_srcdir)/include

if BUILD_WITH_MSVC
lib_LIBRARIES = dime@DIME_MAJOR_VERSION@@SUFFIX@.lib
else
lib_LTLIBRARIES = libdime@SUFFIX@.la
endif

DimeSources = \
	Base.cpp Base.h \
	Basic.cpp Basic.h \
	Input.cpp Input.h \
	Layer.cpp Layer.h \
	Model.cpp Model.h \
	Output.cpp Output.h \
	RecordHolder.cpp RecordHolder.h \
	State.cpp State.h

libdime@SUFFIX@_la_LIBADD = \
	classes/libclasses.la entities/libentities.la objects/libobjects.la \
	records/librecords.la sections/libsections.la tables/libtables.la \
	util/libutil.la convert/libconvert.la

libdime@SUFFIX@_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

dime@DIME_MAJOR_VERSION@@SUFFIX@_lib_LIBADD = \
	classes/classes.lst entities/entities.lst objects/objects.lst \
	records/records.lst sections/sections.lst tables/tables.lst \
	util/util.lst convert/convert.lst

## All source files from the base directory used when building the
## library, including all headers.

libdime@SUFFIX@_la_SOURCES = \
	$(DimeSources)

dime@DIME_MAJOR_VERSION@@SUFFIX@_lib_SOURCES = \
	$(DimeSources)

libdimeincdir = $(includedir)/dime
libdimeinc_HEADERS = \
	../include/dime/Base.h \
	../include/dime/Basic.h \
	../include/dime/Input.h \
	../include/dime/Layer.h \
	../include/dime/Model.h \
	../include/dime/Output.h \
	../include/dime/RecordHolder.h \
	../include/dime/State.h

# Custom rule for linking a Visual C++ (MS Windows) library.

dime@DIME_MAJOR_VERSION@@SUFFIX@.lib: Makefile \
  $(dime@DIME_MAJOR_VERSION@@SUFFIX@_lib_OBJECTS) \
  $(dime@DIME_MAJOR_VERSION@@SUFFIX@_lib_DEPENDENCIES)
	@basename="dime@DIME_MAJOR_VERSION@@SUFFIX@"; \
	rm -f "$$basename.dll" "$$basename.lib"; \
	echo "Linking $$basename.lib..."; \
	echo $(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(dime@DIME_MAJOR_VERSION@@SUFFIX@_lib_OBJECTS) \
	  $(dime@DIME_MAJOR_VERSION@@SUFFIX@_lib_LIBADD) $(LIBS); \
	$(CC) $(LDFLAGS) $(LIBFLAGS) \
	  $(dime@DIME_MAJOR_VERSION@@SUFFIX@_lib_OBJECTS) \
	  $(dime@DIME_MAJOR_VERSION@@SUFFIX@_lib_LIBADD) $(LIBS);

install-data-local:
	@$(NORMAL_INSTALL)
	@basename="dime@DIME_MAJOR_VERSION@@SUFFIX@"; \
	for ext in lib; do \
	  file="$$basename.$$ext"; \
	  if test -f "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(libdir)"; \
	    echo " $(INSTALL_DATA)" "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	    $(INSTALL_DATA) "$$file" "$(DESTDIR)$(libdir)/$$file"; \
	  fi \
	done;

install-exec-local:
	@basename="dime@DIME_MAJOR_VERSION@@SUFFIX@"; \
	for ext in dll pdb; do \
	  file="$$basename.$$ext"; \
	  if test -f "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(bindir)"; \
	    echo " $(INSTALL_PROGRAM)" "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file"; \
	  fi \
	done

install-libdimeincHEADERS: $(libdimeinc_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(libdimeincdir)
	@list='$(libdimeinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(libdimeincdir)/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(libdimeincdir)/$$f; \
	done

uninstall-libdimeincHEADERS:
	@$(NORMAL_UNINSTALL)
	@list='$(libdimeinc_HEADERS)'; for p in $$list; do \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " rm -f $(DESTDIR)$(libdimeincdir)/$$f"; \
	  rm -f $(DESTDIR)$(libdimeincdir)/$$f; \
	done

uninstall-local:
	@basename="dime@DIME_MAJOR_VERSION@@SUFFIX@"; \
	for file in \
	  "$(DESTDIR)$(bindir)/$$basename.dll" \
	  "$(DESTDIR)$(bindir)/$$basename.pdb" \
	  "$(DESTDIR)$(libdir)/$$basename.lib"; do \
	  if test -f "$$file"; then \
	    echo " rm -f" "$$file"; \
	    rm -f "$$file"; \
	  fi \
	done; \
	baseincdir="$(DESTDIR)$(libdimeincdir)"; \
	for dir in \
	  "$$baseincdir/VRMLnodes" \
	  "$$baseincdir/actions" \
	  "$$baseincdir/classes" \
	  "$$baseincdir/convert" \
	  "$$baseincdir/entities" \
	  "$$baseincdir/objects" \
	  "$$baseincdir/records" \
	  "$$baseincdir/sections" \
	  "$$baseincdir/tables" \
	  "$$baseincdir/util" \
	  "$$baseincdir"; do \
	  if test -d "$$dir"; then \
	    echo " rmdir $$dir"; \
	    rmdir "$$dir" || exit 1; \
	  fi \
	done; \
	:

