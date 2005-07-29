## Process this file with automake to generate Makefile.in.

LIBS = 
INCLUDES = -I$(top_srcdir)/include

if BUILD_WITH_MSVC
noinst_LIBRARIES = entities.lst
else
noinst_LTLIBRARIES = libentities.la
endif

## All source files used when building this submodule, including all
## headers.

EntitiesSources = \
	3DFace.cpp 3DFace.h \
	Arc.cpp Arc.h \
	Block.cpp Block.h \
	Circle.cpp Circle.h \
	Ellipse.cpp Ellipse.h \
	Entity.cpp Entity.h \
	ExtrusionEntity.cpp ExtrusionEntity.h \
	FaceEntity.cpp FaceEntity.h \
	Insert.cpp Insert.h \
	LWPolyline.cpp LWPolyline.h \
	Line.cpp Line.h \
	Point.cpp Point.h \
	Polyline.cpp Polyline.h \
	Solid.cpp Solid.h \
	Spline.cpp Spline.h \
	Trace.cpp Trace.h \
	UnknownEntity.cpp UnknownEntity.h \
	Vertex.cpp Vertex.h

libentities_la_SOURCES = \
	$(EntitiesSources)

entities_lst_SOURCES = \
	$(EntitiesSources)

entities.lst: Makefile $(entities_lst_OBJECTS)
	@echo "Linking entities.lst..."; \
	rm -f $@; \
	for i in $(entities_lst_OBJECTS); do echo $$i >>$@; done

## Installation information.

libentitiesincdir = $(includedir)/dime/entities

libentitiesinc_HEADERS = \
	../../include/dime/entities/3DFace.h \
	../../include/dime/entities/Arc.h \
	../../include/dime/entities/Block.h \
	../../include/dime/entities/Circle.h \
	../../include/dime/entities/Ellipse.h \
	../../include/dime/entities/Entity.h \
	../../include/dime/entities/ExtrusionEntity.h \
	../../include/dime/entities/FaceEntity.h \
	../../include/dime/entities/Insert.h \
	../../include/dime/entities/LWPolyline.h \
	../../include/dime/entities/Line.h \
	../../include/dime/entities/Point.h \
	../../include/dime/entities/Polyline.h \
	../../include/dime/entities/Solid.h \
	../../include/dime/entities/Spline.h \
	../../include/dime/entities/Trace.h \
	../../include/dime/entities/UnknownEntity.h \
	../../include/dime/entities/Vertex.h

install-libentitiesincHEADERS: $(libentitiesinc_HEADERS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(libentitiesincdir)
	@list='$(libentitiesinc_HEADERS)'; for p in $$list; do \
	  if test -f "$$p"; then d= ; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$d$$p $(DESTDIR)$(libentitiesincdir)/$$f"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(libentitiesincdir)/$$f; \
	done

uninstall-libentitiesincHEADERS:
	@$(NORMAL_UNINSTALL)
	@list='$(libentitiesinc_HEADERS)'; for p in $$list; do \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " rm -f $(DESTDIR)$(libentitiesincdir)/$$f"; \
	  rm -f $(DESTDIR)$(libentitiesincdir)/$$f; \
	done

