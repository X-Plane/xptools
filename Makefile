BE_QUIET	:= > /dev/null 2>&1

# http://www.cgal.org/
# http://www.cgal.org/download.html
VER_CGAL	:= 3.3.1
# http://www.freetype.org/
# http://sourceforge.net/projects/freetype/files/
VER_FREETYPE	:= 2.3.9
# http://trac.osgeo.org/proj/
VER_LIBPROJ	:= 4.6.1
# http://trac.osgeo.org/geotiff/
VER_GEOTIFF	:= 1.2.5
# http://www.lib3ds.org/; TODO: new release 2.0, has API changes
VER_LIB3DS	:= 1.3.0
# http://www.coin3d.org/lib/dime; no releases yet
# https://svn.coin3d.org/repos/dime/trunk/
# svn co https://svn.coin3d.org/repos/dime/trunk dime
VER_LIBDIME	:= r175
# http://www.ijg.org/
# http://www.ijg.org/files/
VER_LIBJPEG	:= 7
# http://www.sqlite.org/
# http://www.sqlite.org/download.html ; use amalgamation tarball
VER_LIBSQLITE	:= 3.6.16
# http://www.libpng.org/
# http://www.libpng.org/pub/png/libpng.html
VER_LIBPNG	:= 1.2.38
# http://www.zlib.net/
VER_ZLIB	:= 1.2.3
# http://www.libtiff.org/
# ftp://ftp.remotesensing.org/pub/libtiff
VER_LIBTIFF	:= 4.0.0beta3
# http://shapelib.maptools.org/
# http://dl.maptools.org/dl/shapelib/
VER_LIBSHP	:= 1.2.10
# http://code.google.com/p/libsquish/
# http://code.google.com/p/libsquish/downloads/list
VER_LIBSQUISH	:= 1.10
# http://www.boost.org/
# http://sourceforge.net/projects/boost/files/
VER_BOOST	:= 1.39.0
# http://www.mesa3d.org/
# http://sourceforge.net/projects/mesa3d/files/
VER_MESA	:= 7.5
# http://expat.sourceforge.net/
# http://sourceforge.net/projects/expat/files/
VER_LIBEXPAT	:= 2.0.1

ARCHITECTURE	:= $(shell uname -m)
PLATFORM	:= $(shell uname)
ifneq (, $(findstring MINGW, $(PLATFORM)))
	PLATFORM	:= Mingw
	PLAT_MINGW	:= Yes
endif

ifeq ($(cross), m32)
ifeq ($(ARCHITECTURE), x86_64)
	MULTI_SUFFIX	:= 32
	M32_SWITCH	:= -m32
else
	cross		:= ""
endif
endif

ifeq ($(cross), mingw64)
ifdef PLAT_MINGW
	MULTI_SUFFIX	:= 64
	CROSSPREFIX	:= x86_64-pc-mingw32-
	CROSSHOST	:= x86_64-pc-mingw32
	ARCHITECTURE	:= x86_64
else
	cross		:= ""
endif
endif

DEFAULT_PREFIX		:= "`pwd`/../local"$(MULTI_SUFFIX)
DEFAULT_LIBDIR		:= "$(DEFAULT_PREFIX)/lib"
DEFAULT_INCDIR		:= "$(DEFAULT_PREFIX)/include"

ifeq ($(PLATFORM), Darwin)
	DEFAULT_MACARGS	:= -mmacosx-version-min=10.4 -arch i386 -arch ppc
endif
ifeq ($(PLATFORM), Linux)
	DEFAULT_MACARGS	:= -fpie
endif

# boost headers
ARCHIVE_BOOST		:= boost-headers-$(VER_BOOST).tar.gz

# mesa headers
ARCHIVE_MESA		:= mesa-headers-$(VER_MESA).tar.gz

# zlib
ARCHIVE_ZLIB		:= zlib-$(VER_ZLIB).tar.gz
AR_ZLIB			:= "$(CROSSPREFIX)ar rcs"
CC_ZLIB			:= "$(CROSSPREFIX)gcc"
CFLAGS_ZLIB		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_ZLIB		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_ZLIB		:= --prefix=$(DEFAULT_PREFIX)

# expat
ARCHIVE_LIBEXPAT	:= expat-$(VER_LIBEXPAT).tar.gz
CFLAGS_LIBEXPAT		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_LIBEXPAT	:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBEXPAT		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBEXPAT		+= --enable-shared=no
ifeq ($(cross), mingw64)
CONF_LIBEXPAT		+= --host=$(CROSSHOST)
endif

# libpng
ARCHIVE_LIBPNG		:= libpng-$(VER_LIBPNG).tar.gz
CFLAGS_LIBPNG		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_LIBPNG		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBPNG		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBPNG		+= --enable-shared=no
CONF_LIBPNG		+= --enable-maintainer-mode
CONF_LIBPNG		+= --disable-dependency-tracking
CONF_LIBPNG		+= CCDEPMODE="depmode=none"
ifeq ($(cross), mingw64)
CONF_LIBPNG		+= --host=$(CROSSHOST)
endif

# freetype
ARCHIVE_FREETYPE	:= freetype-$(VER_FREETYPE).tar.gz
CFLAGS_FREETYPE		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_FREETYPE	:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_FREETYPE		:= --prefix=$(DEFAULT_PREFIX)
CONF_FREETYPE		+= --enable-shared=no
CONF_FREETYPE		+= --with-zlib
ifeq ($(cross), mingw64)
CONF_FREETYPE		+= --host=$(CROSSHOST)
endif

# libjpeg
ARCHIVE_LIBJPEG		:= jpeg-$(VER_LIBJPEG).tar.gz
CC_LIBJPEG		:= "$(CROSSPREFIX)gcc"
CFLAGS_LIBJPEG		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_LIBJPEG		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBJPEG		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBJPEG		+= --disable-dependency-tracking
CONF_LIBJPEG		+= --enable-shared=no
ifeq ($(cross), mingw64)
CONF_LIBJPEG		+= --host=$(CROSSHOST)
endif

# libtiff
ARCHIVE_LIBTIFF		:= tiff-$(VER_LIBTIFF).tar.gz
CFLAGS_LIBTIFF		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_LIBTIFF		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBTIFF		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBTIFF		+= --enable-shared=no
CONF_LIBTIFF		+= --enable-maintainer-mode
CONF_LIBTIFF		+= --disable-dependency-tracking
CONF_LIBTIFF		+= --with-jpeg-include-dir=$(DEFAULT_INCDIR)
CONF_LIBTIFF		+= --with-jpeg-lib-dir=$(DEFAULT_LIBDIR)
CONF_LIBTIFF		+= --with-zlib-include-dir=$(DEFAULT_INCDIR)
CONF_LIBTIFF		+= --with-zlib-lib-dir=$(DEFAULT_LIBDIR)
CONF_LIBTIFF		+= CCDEPMODE="depmode=none"
ifeq ($(cross), mingw64)
CONF_LIBTIFF		+= --host=$(CROSSHOST)
endif

# libproj
ARCHIVE_LIBPROJ		:= proj-$(VER_LIBPROJ).tar.gz
CFLAGS_LIBPROJ		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_LIBPROJ		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBPROJ		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBPROJ		+= --enable-shared=no
CONF_LIBPROJ		+= --disable-dependency-tracking
CONF_LIBPROJ		+= CCDEPMODE="depmode=none"
ifeq ($(cross), mingw64)
CONF_LIBPROJ		+= --host=$(CROSSHOST)
endif

# geotiff
ARCHIVE_GEOTIFF		:= libgeotiff-$(VER_GEOTIFF).tar.gz
AR_GEOTIFF		:= "$(CROSSPREFIX)ar"
LD_GEOTIFF		:= "$(CROSSPREFIX)ld"
CFLAGS_GEOTIFF		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_GEOTIFF		:= $(M32_SWITCH) -L$(DEFAULT_LIBDIR)
CONF_GEOTIFF		:= --prefix=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --enable-shared=no
CONF_GEOTIFF		+= --without-ld-shared
CONF_GEOTIFF		+= --with-zip=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --with-jpeg=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --with-libtiff=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --with-proj=$(DEFAULT_PREFIX)
ifeq ($(cross), mingw64)
CONF_GEOTIFF		+= --host=$(CROSSHOST)
CONF_GEOTIFF		+= --target=$(CROSSHOST)
endif

# sqlite
ARCHIVE_LIBSQLITE	:= sqlite-amalgamation-$(VER_LIBSQLITE).tar.gz
CFLAGS_LIBSQLITE	:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_LIBSQLITE	:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBSQLITE		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBSQLITE		+= --enable-shared=no
CONF_LIBSQLITE		+= --disable-dependency-tracking
ifeq ($(cross), mingw64)
CONF_LIBSQLITE		+= --host=$(CROSSHOST)
endif

# lib3ds
ARCHIVE_LIB3DS		:= lib3ds-$(VER_LIB3DS).tar.gz
CFLAGS_LIB3DS		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_LIB3DS		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIB3DS		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIB3DS		+= --enable-shared=no
CONF_LIB3DS		+= --enable-maintainer-mode
CONF_LIB3DS		+= --disable-dependency-tracking
CONF_LIB3DS		+= CCDEPMODE="depmode=none"
ifeq ($(cross), mingw64)
CONF_LIB3DS		+= --host=$(CROSSHOST)
endif

# cgal
# note that 3.3.1 skips the build of libCGALPDB because of compilation errors
# when compiling with gcc > 4.4 (header pickyness), we can patch that if we ever
# happen to need that specific library
ARCHIVE_CGAL		:= CGAL-$(VER_CGAL).tar.gz
CFLAGS_CGAL		:= -I$(DEFAULT_INCDIR)
CFLAGS_CGAL		+= -O2 -frounding-math $(M32_SWITCH) $(DEFAULT_MACARGS)
LDFLAGS_CGAL		:= -L$(DEFAULT_LIBDIR) $(M32_SWITCH)
CONF_CGAL		:= --CXXFLAGS '$(CFLAGS_CGAL)'
CONF_CGAL		+= --CXX '$(CROSSPREFIX)g++'
CONF_CGAL		+= --LDFLAGS '$(LDFLAGS_CGAL)'
CONF_CGAL		+= --BOOST_INCL_DIR $(DEFAULT_INCDIR)
CONF_CGAL		+= --prefix $(DEFAULT_PREFIX)
CONF_CGAL		+= --disable-shared
CONF_CGAL		+= --verbose

# libsquish
ARCHIVE_LIBSQUISH	:= squish-$(VER_LIBSQUISH).tar.gz
CONF_LIBSQUISH		:= INSTALL_DIR=$(DEFAULT_PREFIX)
CONF_LIBSQUISH		+= CPPFLAGS="$(DEFAULT_MACARGS) -O2 -I$(DEFAULT_INCDIR) $(M32_SWITCH)"
CONF_LIBSQUISH		+= AR="$(CROSSPREFIX)ar" CXX="$(CROSSPREFIX)g++"

# libdime
ARCHIVE_LIBDIME		:= dime-$(VER_LIBDIME).tar.gz
CFLAGS_LIBDIME		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_LIBDIME		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBDIME		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBDIME		+= --enable-static=yes
CONF_LIBDIME		+= --enable-shared=no
CONF_LIBDIME		+= --disable-dependency-tracking
ifeq ($(cross), mingw64)
CONF_LIBDIME		+= --host=$(CROSSHOST)
endif

# libshp
ARCHIVE_LIBSHP		:= shapelib-$(VER_LIBSHP).tar.gz
CFLAGS_LIBSHP		:= "-I$(DEFAULT_MACARGS) $(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_LIBSHP		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBSHP		:= AR="$(CROSSPREFIX)ar" CC="$(CROSSPREFIX)gcc"
CONF_LIBSHP		+= cross=$(M32_SWITCH)


# platform specific tweaks
ifeq ($(PLATFORM), Darwin)
	AR_ZLIB			:= "libtool -static -o"
	CONF_LIBTIFF		+= --with-apple-opengl-framework
	LDFLAGS_GEOTIFF     	+= -Z
endif
ifeq ($(PLATFORM), Mingw)
endif
ifeq ($(PLATFORM), Linux)
endif

# targets
.PHONY: all clean boost_headers mesa_headers zlib libpng libfreetype libjpeg \
libtiff libproj libgeotiff libsqlite lib3ds libcgal libsquish libdime libshp \
libexpat

ifneq ($(gitlibs), 1)
all: ./local$(MULTI_SUFFIX)/.xpt_libs
./local$(MULTI_SUFFIX)/.xpt_libs: boost_headers mesa_headers zlib libpng \
libfreetype libjpeg libtiff libproj libgeotiff libsqlite lib3ds libcgal \
libsquish libdime libshp libexpat
	@touch ./local$(MULTI_SUFFIX)/.xpt_libs
else
ifeq ($(PLATFORM), Mingw)
all: ./local$(MULTI_SUFFIX)/.xpt_libs
./local$(MULTI_SUFFIX)/.xpt_libs:
	@-rm -rf ./local
	git clone git://dev.x-plane.com/xptools-libs-win32.git ./local
endif
ifeq ($(PLATFORM), Darwin)
all: ./local$(MULTI_SUFFIX)/.xpt_libs
./local$(MULTI_SUFFIX)/.xpt_libs:
	@-rm -rf ./local
	git clone git://dev.x-plane.com/xptools-libs-macosx.git ./local
endif
ifeq ($(PLATFORM), Linux)
ifeq ($(ARCHITECTURE), x86_64)
all: ./local$(MULTI_SUFFIX)/.xpt_libs
./local$(MULTI_SUFFIX)/.xpt_libs:
	@-rm -rf ./local
	git clone git://dev.x-plane.com/xptools-libs-linux64.git ./local
endif
endif
endif

clean:
	@echo "cleaning 3rd-party libraries, removing `pwd`/local"
	@-rm -rf ./local
	@-rm -rf ./local32
	@-rm -rf ./local64


boost_headers: ./local$(MULTI_SUFFIX)/include/.xpt_boost
./local$(MULTI_SUFFIX)/include/.xpt_boost:
	@echo "extracting boost headers..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@tar -C "./local$(MULTI_SUFFIX)/include" -xzf "./archives/$(ARCHIVE_BOOST)"
	@touch $@


mesa_headers: ./local$(MULTI_SUFFIX)/include/.xpt_mesa
./local$(MULTI_SUFFIX)/include/.xpt_mesa:
	@echo "extracting mesa headers..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include/mesa"
	@tar -C "./local$(MULTI_SUFFIX)/include/mesa" -xzf "./archives/$(ARCHIVE_MESA)"
	@touch $@


zlib: ./local$(MULTI_SUFFIX)/lib/.xpt_zlib
./local$(MULTI_SUFFIX)/lib/.xpt_zlib:
	@echo "building zlib..."
	@tar -xzf "./archives/$(ARCHIVE_ZLIB)"
	@cd "zlib-$(VER_ZLIB)" && \
	chmod +x configure && \
	AR=$(AR_ZLIB) CC=$(CC_ZLIB) CFLAGS=$(CFLAGS_ZLIB) \
	LDFLAGS=$(LDFLAGS_ZLIB) \
	./configure $(CONF_ZLIB) $(BE_QUIET)
	@$(MAKE) -C "zlib-$(VER_ZLIB)" $(BE_QUIET)
	@$(MAKE) -C "zlib-$(VER_ZLIB)" install $(BE_QUIET)
	@-rm -rf zlib-$(VER_ZLIB)
	@touch $@


libexpat: ./local$(MULTI_SUFFIX)/lib/.xpt_libexpat
./local$(MULTI_SUFFIX)/lib/.xpt_libexpat:
	@echo "building libexpat..."
	@tar -xzf "./archives/$(ARCHIVE_LIBEXPAT)"
	@cd "expat-$(VER_LIBEXPAT)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBEXPAT) LDFLAGS=$(LDFLAGS_LIBEXPAT) \
	./configure $(CONF_LIBEXPAT) $(BE_QUIET)
	@$(MAKE) -C "expat-$(VER_LIBEXPAT)" $(BE_QUIET)
	@$(MAKE) -C "expat-$(VER_LIBEXPAT)" install $(BE_QUIET)
	@-rm -rf expat-$(VER_LIBEXPAT)
	@touch $@


libpng: ./local$(MULTI_SUFFIX)/lib/.xpt_libpng
./local$(MULTI_SUFFIX)/lib/.xpt_libpng: ./local$(MULTI_SUFFIX)/lib/.xpt_zlib
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


libfreetype: ./local$(MULTI_SUFFIX)/lib/.xpt_libfreetype
./local$(MULTI_SUFFIX)/lib/.xpt_libfreetype: ./local$(MULTI_SUFFIX)/lib/.xpt_zlib
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

libjpeg: ./local$(MULTI_SUFFIX)/lib/.xpt_libjpeg
./local$(MULTI_SUFFIX)/lib/.xpt_libjpeg:
	@echo "building libjpeg..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@-mkdir -p "./local$(MULTI_SUFFIX)/lib"
	@tar -xzf "./archives/$(ARCHIVE_LIBJPEG)"
	@cp patches/0001-libjpeg-fix-boolean-type-width.patch \
	"jpeg-$(VER_LIBJPEG)" && cd "jpeg-$(VER_LIBJPEG)" && \
	patch -p1 < ./0001-libjpeg-fix-boolean-type-width.patch $(BE_QUIET)
	@cd "jpeg-$(VER_LIBJPEG)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBJPEG) LDFLAGS=$(LDFLAGS_LIBJPEG) CC=$(CC_LIBJPEG) \
	./configure $(CONF_LIBJPEG) $(BE_QUIET)
	@$(MAKE) -C "jpeg-$(VER_LIBJPEG)" $(BE_QUIET)
	@$(MAKE) -C "jpeg-$(VER_LIBJPEG)" install \
	$(BE_QUIET)
	@-rm -rf jpeg-$(VER_LIBJPEG)
	@touch $@


libtiff: ./local$(MULTI_SUFFIX)/lib/.xpt_libtiff
./local$(MULTI_SUFFIX)/lib/.xpt_libtiff: ./local$(MULTI_SUFFIX)/lib/.xpt_zlib ./local$(MULTI_SUFFIX)/lib/.xpt_libjpeg
	@echo "building libtiff..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@-mkdir -p "./local$(MULTI_SUFFIX)/lib"
	@tar -xzf "./archives/$(ARCHIVE_LIBTIFF)"
	@cd "tiff-$(VER_LIBTIFF)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBTIFF) LDFLAGS=$(LDFLAGS_LIBTIFF) \
	./configure $(CONF_LIBTIFF) $(BE_QUIET)
	@$(MAKE) -C "tiff-$(VER_LIBTIFF)" $(BE_QUIET)
	@$(MAKE) -C "tiff-$(VER_LIBTIFF)" install $(BE_QUIET)
	@-rm -rf tiff-$(VER_LIBTIFF)
	@touch $@


libproj: ./local$(MULTI_SUFFIX)/lib/.xpt_libproj
./local$(MULTI_SUFFIX)/lib/.xpt_libproj:
	@echo "building libproj..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@-mkdir -p "./local$(MULTI_SUFFIX)/lib"
	@tar -xzf "./archives/$(ARCHIVE_LIBPROJ)"
	@cd "proj-$(VER_LIBPROJ)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBPROJ) LDFLAGS=$(LDFLAGS_LIBPROJ) \
	./configure $(CONF_LIBPROJ) $(BE_QUIET)
	@$(MAKE) -C "proj-$(VER_LIBPROJ)" $(BE_QUIET)
	@$(MAKE) -C "proj-$(VER_LIBPROJ)" install $(BE_QUIET)
	@-rm -rf proj-$(VER_LIBPROJ)
	@touch $@


libgeotiff: ./local$(MULTI_SUFFIX)/lib/.xpt_libgeotiff
./local$(MULTI_SUFFIX)/lib/.xpt_libgeotiff: ./local$(MULTI_SUFFIX)/lib/.xpt_zlib ./local$(MULTI_SUFFIX)/lib/.xpt_libjpeg \
./local$(MULTI_SUFFIX)/lib/.xpt_libtiff ./local$(MULTI_SUFFIX)/lib/.xpt_libproj
	@echo "building libgeotiff..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@-mkdir -p "./local$(MULTI_SUFFIX)/lib"
	@tar -xzf "./archives/$(ARCHIVE_GEOTIFF)"
	@cd "libgeotiff-$(VER_GEOTIFF)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_GEOTIFF) LDFLAGS="$(LDFLAGS_GEOTIFF)" \
	LD_SHARED="$(LD_GEOTIFF)" AR="$(AR_GEOTIFF)" \
	./configure $(CONF_GEOTIFF) $(BE_QUIET)
	@$(MAKE) -C "libgeotiff-$(VER_GEOTIFF)" $(BE_QUIET)
	@$(MAKE) -C "libgeotiff-$(VER_GEOTIFF)" install $(BE_QUIET)
	@-rm -rf libgeotiff-$(VER_GEOTIFF)
	@-rm -rf ./local/lib/libgeotiff.so*
	@touch $@


libsqlite: ./local$(MULTI_SUFFIX)/lib/.xpt_libsqlite
./local$(MULTI_SUFFIX)/lib/.xpt_libsqlite:
	@echo "building libsqlite..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@-mkdir -p "./local$(MULTI_SUFFIX)/lib"
	@tar -xzf "./archives/$(ARCHIVE_LIBSQLITE)"
	@cd "sqlite-$(VER_LIBSQLITE)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBSQLITE) LDFLAGS=$(LDFLAGS_LIBSQLITE) \
	./configure $(CONF_LIBSQLITE) $(BE_QUIET)
	@$(MAKE) -C "sqlite-$(VER_LIBSQLITE)" $(BE_QUIET)
	@$(MAKE) -C "sqlite-$(VER_LIBSQLITE)" install $(BE_QUIET)
	@-rm -rf sqlite-$(VER_LIBSQLITE)
	@touch $@


lib3ds: ./local$(MULTI_SUFFIX)/lib/.xpt_lib3ds
./local$(MULTI_SUFFIX)/lib/.xpt_lib3ds:
	@echo "building lib3ds..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@-mkdir -p "./local$(MULTI_SUFFIX)/lib"
	@tar -xzf "./archives/$(ARCHIVE_LIB3DS)"
	@cd "lib3ds-$(VER_LIB3DS)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIB3DS) LDFLAGS=$(LDFLAGS_LIB3DS) \
	./configure $(CONF_LIB3DS) $(BE_QUIET)
	@$(MAKE) -C "lib3ds-$(VER_LIB3DS)" $(BE_QUIET)
	@$(MAKE) -C "lib3ds-$(VER_LIB3DS)" install $(BE_QUIET)
	@-rm -rf lib3ds-$(VER_LIB3DS)
	@touch $@


libsquish: ./local$(MULTI_SUFFIX)/lib/.xpt_libsquish
./local$(MULTI_SUFFIX)/lib/.xpt_libsquish:
	@echo "building libsquish..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@-mkdir -p "./local$(MULTI_SUFFIX)/lib"
	@tar -xzf "./archives/$(ARCHIVE_LIBSQUISH)"
	@cp patches/0001-libsquish-gcc-4.3-header-fix.patch \
	"squish-$(VER_LIBSQUISH)" && cd "squish-$(VER_LIBSQUISH)" && \
	patch -p1 < ./0001-libsquish-gcc-4.3-header-fix.patch $(BE_QUIET)
	@cd "squish-$(VER_LIBSQUISH)" && \
	$(MAKE) $(CONF_LIBSQUISH) install $(BE_QUIET)
	@-rm -rf squish-$(VER_LIBSQUISH)
	@touch $@


libcgal: boost_headers ./local$(MULTI_SUFFIX)/lib/.xpt_libcgal
./local$(MULTI_SUFFIX)/lib/.xpt_libcgal: ./local$(MULTI_SUFFIX)/lib/.xpt_zlib
	@echo "building libcgal..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@-mkdir -p "./local$(MULTI_SUFFIX)/lib"
	@tar -xzf "./archives/$(ARCHIVE_CGAL)"
	@cp patches/0001-libcgal-various-fixes.patch \
	"CGAL-$(VER_CGAL)" && cd "CGAL-$(VER_CGAL)" && \
	patch -p1 < ./0001-libcgal-various-fixes.patch $(BE_QUIET)
	@cd "CGAL-$(VER_CGAL)" && \
	chmod +x install_cgal && \
	./install_cgal $(CONF_CGAL) $(BE_QUIET)
	@-rm -f ./local$(MULTI_SUFFIX)/lib/*.so*
	@-rm -rf CGAL-$(VER_CGAL)
	@touch $@


libdime: ./local$(MULTI_SUFFIX)/lib/.xpt_libdime
./local$(MULTI_SUFFIX)/lib/.xpt_libdime:
	@echo "building libdime..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@-mkdir -p "./local$(MULTI_SUFFIX)/lib"
	@tar -xzf "./archives/$(ARCHIVE_LIBDIME)"
	@cd "dime-$(VER_LIBDIME)" && \
	CFLAGS=$(CFLAGS_LIBDIME) CXXFLAGS=$(CFLAGS_LIBDIME) \
	LDFLAGS=$(LDFLAGS_LIBDIME) \
	./configure $(CONF_LIBDIME) $(BE_QUIET)
	@$(MAKE) -C "dime-$(VER_LIBDIME)" $(BE_QUIET)
	@$(MAKE) -C "dime-$(VER_LIBDIME)" install $(BE_QUIET)
	@-rm -rf dime-$(VER_LIBDIME)
	@touch $@


libshp: ./local$(MULTI_SUFFIX)/lib/.xpt_libshp
./local$(MULTI_SUFFIX)/lib/.xpt_libshp:
	@echo "building libshp..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@-mkdir -p "./local$(MULTI_SUFFIX)/lib"
	@tar -xzf "./archives/$(ARCHIVE_LIBSHP)"
	@cp patches/0001-libshp-fix-makefile-for-multiple-platforms.patch \
	"shapelib-$(VER_LIBSHP)" && cd "shapelib-$(VER_LIBSHP)" && \
	patch -p1 < ./0001-libshp-fix-makefile-for-multiple-platforms.patch \
	$(BE_QUIET)
	@$(MAKE) -C "shapelib-$(VER_LIBSHP)" $(CONF_LIBSHP) lib $(BE_QUIET)
	@cp -Lp shapelib-$(VER_LIBSHP)/*.h ./local$(MULTI_SUFFIX)/include
	@cp shapelib-$(VER_LIBSHP)/.libs/libshp.a ./local$(MULTI_SUFFIX)/lib
	@-rm -rf shapelib-$(VER_LIBSHP)
	@touch $@
