BE_QUIET	:= > /dev/null 2>&1

# TODO: add libexpat as separate library and adjust WED makerule accordingly

VER_CGAL	:= 3.3.1
# TODO: new release 2.3.9
VER_FREETYPE	:= 2.3.8
VER_LIBPROJ	:= 4.6.1
VER_GEOTIFF	:= 1.2.5
# TODO. new release 2.0, has API changes
VER_LIB3DS	:= 1.3.0
# TODO: new release r175, write Makefile, diff, create patch against r175
VER_LIBDIME	:= r174
# TODO: diff, create patch against 6b
VER_LIBJPEG	:= 6b.1
VER_LIBSQLITE	:= 3.6.11
VER_LIBPNG	:= 1.2.35
VER_ZLIB	:= 1.2.3
VER_LIBTIFF	:= 4.0.0beta3
# TODO: write Makefile, diff, create patch against 1.2.10
VER_LIBSHP	:= 1.2.10.1
# TODO: diff, create patch against 1.10
VER_LIBSQUISH	:= 1.10.1
VER_BOOST	:= 1.38.0
# TODO: new release 7.4
VER_MESA	:= 7.3

PLATFORM	:= $(shell uname)
ifneq (, $(findstring MINGW, $(PLATFORM)))
	PLATFORM	:= Mingw
endif

DEFAULT_PREFIX		:= "`pwd`/../local"
DEFAULT_LIBDIR		:= "`pwd`/../local/lib"
DEFAULT_INCDIR		:= "`pwd`/../local/include"

ifeq ($(PLATFORM), Darwin)
	DEFAULT_MACARGS	:= -mmacosx-version-min=10.4 -arch i386 -arch ppc
endif

# boost headers
ARCHIVE_BOOST		:= boost-headers-$(VER_BOOST).tar.gz

# mesa headers
ARCHIVE_MESA		:= mesa-headers-$(VER_MESA).tar.gz

# zlib
ARCHIVE_ZLIB		:= zlib-$(VER_ZLIB).tar.gz
AR_ZLIB			:= "ar rcs"
CFLAGS_ZLIB		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O3"
LDFLAGS_ZLIB		:= "-L$(DEFAULT_LIBDIR)"
CONF_ZLIB		:= --prefix=$(DEFAULT_PREFIX)

# libpng
ARCHIVE_LIBPNG		:= libpng-$(VER_LIBPNG).tar.gz
CFLAGS_LIBPNG		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O3"
LDFLAGS_LIBPNG		:= "-L$(DEFAULT_LIBDIR)"
CONF_LIBPNG		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBPNG		+= --enable-shared=no
CONF_LIBPNG		+= --enable-maintainer-mode
CONF_LIBPNG		+= --disable-dependency-tracking
CONF_LIBPNG		+= CCDEPMODE="depmode=none"

# freetype
ARCHIVE_FREETYPE	:= freetype-$(VER_FREETYPE).tar.gz
CFLAGS_FREETYPE		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O3"
LDFLAGS_FREETYPE	:= "-L$(DEFAULT_LIBDIR)"
CONF_FREETYPE		:= --prefix=$(DEFAULT_PREFIX)
CONF_FREETYPE		+= --enable-shared=no
CONF_FREETYPE		+= --with-zlib

# libjpeg
ARCHIVE_LIBJPEG		:= jpeg-$(VER_LIBJPEG).tar.gz
CFLAGS_LIBJPEG		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O3"
LDFLAGS_LIBJPEG		:= "-L$(DEFAULT_LIBDIR)"
CONF_LIBJPEG		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBJPEG		+= --enable-shared=no

# libtiff
ARCHIVE_LIBTIFF		:= tiff-$(VER_LIBTIFF).tar.gz
CFLAGS_LIBTIFF		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O3"
LDFLAGS_LIBTIFF		:= "-L$(DEFAULT_LIBDIR)"
CONF_LIBTIFF		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBTIFF		+= --enable-shared=no
CONF_LIBTIFF		+= --enable-maintainer-mode
CONF_LIBTIFF		+= --disable-dependency-tracking
CONF_LIBTIFF		+= --with-jpeg-include-dir=$(DEFAULT_INCDIR)
CONF_LIBTIFF		+= --with-jpeg-lib-dir=$(DEFAULT_LIBDIR)
CONF_LIBTIFF		+= --with-zlib-include-dir=$(DEFAULT_INCDIR)
CONF_LIBTIFF		+= --with-zlib-lib-dir=$(DEFAULT_LIBDIR)
CONF_LIBTIFF		+= CCDEPMODE="depmode=none"

# libproj
ARCHIVE_LIBPROJ		:= proj-$(VER_LIBPROJ).tar.gz
CFLAGS_LIBPROJ		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O3"
LDFLAGS_LIBPROJ		:= "-L$(DEFAULT_LIBDIR)"
CONF_LIBPROJ		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBPROJ		+= --enable-shared=no
CONF_LIBPROJ		+= --enable-maintainer-mode
CONF_LIBPROJ		+= --disable-dependency-tracking
CONF_LIBPROJ		+= CCDEPMODE="depmode=none"

# geotiff
ARCHIVE_GEOTIFF		:= libgeotiff-$(VER_GEOTIFF).tar.gz
CFLAGS_GEOTIFF		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O3"
LDFLAGS_GEOTIFF		:= "-L$(DEFAULT_LIBDIR)"
CONF_GEOTIFF		:= --prefix=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --enable-shared=no
CONF_GEOTIFF		+= --without-ld-shared
CONF_GEOTIFF		+= --with-zip=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --with-jpeg=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --with-libtiff=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --with-proj=$(DEFAULT_PREFIX)

# sqlite
ARCHIVE_LIBSQLITE	:= sqlite-$(VER_LIBSQLITE).tar.gz
CFLAGS_LIBSQLITE	:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O3"
LDFLAGS_LIBSQLITE	:= "-L$(DEFAULT_LIBDIR)"
CONF_LIBSQLITE		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBSQLITE		+= --enable-shared=no
CONF_LIBSQLITE		+= --disable-tcl

# lib3ds
ARCHIVE_LIB3DS		:= lib3ds-$(VER_LIB3DS).tar.gz
CFLAGS_LIB3DS		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O3"
LDFLAGS_LIB3DS		:= "-L$(DEFAULT_LIBDIR)"
CONF_LIB3DS		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIB3DS		+= --enable-shared=no
CONF_LIB3DS		+= --enable-maintainer-mode
CONF_LIB3DS		+= --disable-dependency-tracking
CONF_LIB3DS		+= CCDEPMODE="depmode=none"

# cgal
ARCHIVE_CGAL		:= CGAL-$(VER_CGAL).tar.gz
CFLAGS_CGAL		:= $(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR)
CFLAGS_CGAL		+= -O3 -frounding-math
CONF_CGAL		:= --CXXFLAGS "$(CFLAGS_CGAL)"
CONF_CGAL		+= --BOOST_INCL_DIR $(DEFAULT_INCDIR)
CONF_CGAL		+= --prefix $(DEFAULT_PREFIX)
CONF_CGAL		+= --disable-shared

# libsquish
ARCHIVE_LIBSQUISH	:= squish-$(VER_LIBSQUISH).tar.gz
CONF_LIBSQUISH		:= INSTALL_DIR=$(DEFAULT_PREFIX)
CONF_LIBSQUISH		+= CPPFLAGS="$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR)"

# libdime
ARCHIVE_LIBDIME		:= libdime-$(VER_LIBDIME).tar.gz
CFLAGS_LIBDIME		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O3"
LDFLAGS_LIBDIME		:= "-L$(DEFAULT_LIBDIR)"
CONF_LIBDIME		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBDIME		+= --enable-static
CONF_LIBDIME		+= --enable-shared=no

# libshp
ARCHIVE_LIBSHP		:= shapelib-$(VER_LIBSHP).tar.gz
CFLAGS_LIBSHP		:= "-I$(DEFAULT_MACARGS) $(DEFAULT_INCDIR) -O3"
LDFLAGS_LIBSHP		:= "-L$(DEFAULT_LIBDIR)"
CONF_LIBSHP		:= --prefix=$(DEFAULT_PREFIX)


# platform specific tweaks
ifeq ($(PLATFORM), Darwin)
	AR_ZLIB			:= "libtool -static -o"
	CONF_LIBTIFF		+= --with-apple-opengl-framework
endif
ifeq ($(PLATFORM), Mingw)
endif
ifeq ($(PLATFORM), Linux)
endif

# targets
.PHONY: all clean boost_headers mesa_headers zlib libpng libfreetype libjpeg \
libtiff libproj libgeotiff libsqlite lib3ds libcgal libsquish libdime libshp

ifeq ($(PLATFORM), Darwin)
all: boost_headers mesa_headers zlib libpng libfreetype libjpeg libtiff \
 libproj libgeotiff libsqlite lib3ds libcgal libsquish
else
all: boost_headers mesa_headers zlib libpng libfreetype libjpeg libtiff \
 libproj libgeotiff libsqlite lib3ds libcgal libsquish libdime libshp
endif
	@touch ./local/.xpt_libs

clean:
	@echo "cleaning 3rd-party libraries..."
	@-rm -rf ./local


boost_headers: ./local/include/.xpt_boost
./local/include/.xpt_boost:
	@echo "extracting boost headers..."
	@-mkdir -p "./local/include"
	@tar -C "./local/include" -xzf "./archives/$(ARCHIVE_BOOST)"
	@touch $@


mesa_headers: ./local/include/.xpt_mesa
./local/include/.xpt_mesa:
	@echo "extracting mesa headers..."
	@-mkdir -p "./local/include/mesa"
	@tar -C "./local/include/mesa" -xzf "./archives/$(ARCHIVE_MESA)"
	@touch $@


zlib: ./local/lib/.xpt_zlib
./local/lib/.xpt_zlib:
	@tar -xzf "./archives/$(ARCHIVE_ZLIB)"
	@cd "zlib-$(VER_ZLIB)" && \
	chmod +x configure && \
	AR=$(AR_ZLIB) CFLAGS=$(CFLAGS_ZLIB) LDFLAGS=$(LDFLAGS_ZLIB) \
	./configure $(CONF_ZLIB) $(BE_QUIET)
	@$(MAKE) -C "zlib-$(VER_ZLIB)" $(BE_QUIET)
	@$(MAKE) -C "zlib-$(VER_ZLIB)" install $(BE_QUIET)
	@-rm -rf zlib-$(VER_ZLIB)
	@touch $@


libpng: zlib ./local/lib/.xpt_libpng
./local/lib/.xpt_libpng:
	@echo "building libpng..."
	@tar -xzf "./archives/$(ARCHIVE_LIBPNG)"
	@cd "libpng-$(VER_LIBPNG)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBPNG) LDFLAGS=$(LDFLAGS_LIBPNG) \
	./configure $(CONF_LIBPNG) $(BE_QUIET)
	@$(MAKE) -C "libpng-$(VER_LIBPNG)" $(BE_QUIET)
	@$(MAKE) -C "libpng-$(VER_LIBPNG)" install $(BE_QUIET)
	@-rm -rf libpng-$(VER_LIBPNG)
	@touch $@


libfreetype: zlib ./local/lib/.xpt_libfreetype
./local/lib/.xpt_libfreetype:
	@echo "building libfreetype..."
	@tar -xzf "./archives/$(ARCHIVE_FREETYPE)"
	@cd "freetype-$(VER_FREETYPE)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_FREETYPE) LDFLAGS=$(LDFLAGS_FREETYPE) \
	./configure $(CONF_FREETYPE) $(BE_QUIET)
	@$(MAKE) -C "freetype-$(VER_FREETYPE)" $(BE_QUIET)
	@$(MAKE) -C "freetype-$(VER_FREETYPE)" install $(BE_QUIET)
	@-rm -rf freetype-$(VER_FREETYPE)
	@touch $@


libjpeg: ./local/lib/.xpt_libjpeg
./local/lib/.xpt_libjpeg:
	@echo "building libjpeg..."
	@tar -xzf "./archives/$(ARCHIVE_LIBJPEG)"
	@cd "jpeg-$(VER_LIBJPEG)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBJPEG) LDFLAGS=$(LDFLAGS_LIBJPEG) \
	./configure $(CONF_LIBJPEG) $(BE_QUIET)
	@$(MAKE) -C "jpeg-$(VER_LIBJPEG)" $(BE_QUIET)
	@$(MAKE) -C "jpeg-$(VER_LIBJPEG)" install-lib install-headers \
	$(BE_QUIET)
	@-rm -rf jpeg-$(VER_LIBJPEG)
	@touch $@


libtiff: zlib libjpeg ./local/lib/.xpt_libtiff
./local/lib/.xpt_libtiff:
	@echo "building libtiff..."
	@tar -xzf "./archives/$(ARCHIVE_LIBTIFF)"
	@cd "tiff-$(VER_LIBTIFF)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBTIFF) LDFLAGS=$(LDFLAGS_LIBTIFF) \
	./configure $(CONF_LIBTIFF) $(BE_QUIET)
	@$(MAKE) -C "tiff-$(VER_LIBTIFF)" $(BE_QUIET)
	@$(MAKE) -C "tiff-$(VER_LIBTIFF)" install $(BE_QUIET)
	@-rm -rf tiff-$(VER_LIBTIFF)
	@touch $@


libproj: ./local/lib/.xpt_libproj
./local/lib/.xpt_libproj:
	@echo "building libproj..."
	@tar -xzf "./archives/$(ARCHIVE_LIBPROJ)"
	@cd "proj-$(VER_LIBPROJ)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBPROJ) LDFLAGS=$(LDFLAGS_LIBPROJ) \
	./configure $(CONF_LIBPROJ) $(BE_QUIET)
	@$(MAKE) -C "proj-$(VER_LIBPROJ)" $(BE_QUIET)
	@$(MAKE) -C "proj-$(VER_LIBPROJ)" install $(BE_QUIET)
	@-rm -rf proj-$(VER_LIBPROJ)
	@touch $@


libgeotiff: zlib libjpeg libtiff libproj ./local/lib/.xpt_libgeotiff
./local/lib/.xpt_libgeotiff:
	@echo "building libgeotiff..."
	@tar -xzf "./archives/$(ARCHIVE_GEOTIFF)"
	@cd "libgeotiff-$(VER_GEOTIFF)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_GEOTIFF) LDFLAGS=$(LDFLAGS_GEOTIFF) \
	./configure $(CONF_GEOTIFF) $(BE_QUIET)
	@$(MAKE) -C "libgeotiff-$(VER_GEOTIFF)" $(BE_QUIET)
	@$(MAKE) -C "libgeotiff-$(VER_GEOTIFF)" install $(BE_QUIET)
	@-rm -rf libgeotiff-$(VER_GEOTIFF)
	@-rm -rf ./local/lib/libgeotiff.so*
	@touch $@


libsqlite: ./local/lib/.xpt_libsqlite
./local/lib/.xpt_libsqlite:
	@echo "building libsqlite..."
	@tar -xzf "./archives/$(ARCHIVE_LIBSQLITE)"
	@cd "sqlite-$(VER_LIBSQLITE)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBSQLITE) LDFLAGS=$(LDFLAGS_LIBSQLITE) \
	./configure $(CONF_LIBSQLITE) $(BE_QUIET)
	@$(MAKE) -C "sqlite-$(VER_LIBSQLITE)" $(BE_QUIET)
	@$(MAKE) -C "sqlite-$(VER_LIBSQLITE)" install $(BE_QUIET)
	@-rm -rf sqlite-$(VER_LIBSQLITE)
	@touch $@


lib3ds: ./local/lib/.xpt_lib3ds
./local/lib/.xpt_lib3ds:
	@echo "building lib3ds..."
	@tar -xzf "./archives/$(ARCHIVE_LIB3DS)"
	@cd "lib3ds-$(VER_LIB3DS)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIB3DS) LDFLAGS=$(LDFLAGS_LIB3DS) \
	./configure $(CONF_LIB3DS) $(BE_QUIET)
	@$(MAKE) -C "lib3ds-$(VER_LIB3DS)" $(BE_QUIET)
	@$(MAKE) -C "lib3ds-$(VER_LIB3DS)" install $(BE_QUIET)
	@-rm -rf lib3ds-$(VER_LIB3DS)
	@touch $@


libsquish: ./local/lib/.xpt_libsquish
./local/lib/.xpt_libsquish:
	@echo "building libsquish..."
	@tar -xzf "./archives/$(ARCHIVE_LIBSQUISH)"
	@cd "squish-$(VER_LIBSQUISH)" && \
	$(MAKE) $(CONF_LIBSQUISH) install $(BE_QUIET)
	@-rm -rf squish-$(VER_LIBSQUISH)
	@touch $@


libcgal: boost_headers zlib ./local/lib/.xpt_libcgal
./local/lib/.xpt_libcgal:
	@echo "building libcgal..."
	@tar -xzf "./archives/$(ARCHIVE_CGAL)"
	@cp patches/cgal-no-description-0001.patch \
	"CGAL-$(VER_CGAL)/include" && cd "CGAL-$(VER_CGAL)/include" && \
	patch -p1 < ./cgal-no-description-0001.patch $(BE_QUIET)
	@cd "CGAL-$(VER_CGAL)" && \
	chmod +x install_cgal && \
	./install_cgal $(CONF_CGAL) $(BE_QUIET)
	@-rm -rf CGAL-$(VER_CGAL)
	@touch $@


# TODO: write own Makefiles for these two to make the buildable on MacOS
libdime: ./local/lib/.xpt_libdime
./local/lib/.xpt_libdime:
	@echo "building libdime..."
	@tar -xzf "./archives/$(ARCHIVE_LIBDIME)"
	@cd "libdime-$(VER_LIBDIME)" && \
	CFLAGS=$(CFLAGS_LIBDIME) CXXFLAGS=$(CFLAGS_LIBDIME) \
	LDFLAGS=$(LDFLAGS_LIBDIME) \
	./configure $(CONF_LIBDIME) $(BE_QUIET)
	@$(MAKE) -C "libdime-$(VER_LIBDIME)" $(BE_QUIET)
	@$(MAKE) -C "libdime-$(VER_LIBDIME)" install $(BE_QUIET)
	@-rm -rf libdime-$(VER_LIBDIME)
	@touch $@


libshp: ./local/lib/.xpt_libshp
./local/lib/.xpt_libshp:
	@echo "building libshp..."
	@-mkdir -p "./local/include"
	@-mkdir -p "./local/lib"
	@tar -xzf "./archives/$(ARCHIVE_LIBSHP)"
	@$(MAKE) -C "shapelib-$(VER_LIBSHP)" lib $(BE_QUIET)
	@cp -Lp shapelib-$(VER_LIBSHP)/*.h ./local/include
	@cp shapelib-$(VER_LIBSHP)/.libs/libshp.a ./local/lib
	@-rm -rf shapelib-$(VER_LIBSHP)
	@touch $@
