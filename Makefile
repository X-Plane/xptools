BE_QUIET	:= > /dev/null 2>&1

# http://www.cgal.org/
# http://gforge.inria.fr/frs/?group_id=52
VER_CGAL	:= 3.9
# http://www.freetype.org/
# http://sourceforge.net/projects/freetype/files/
VER_FREETYPE	:= 2.3.11
# http://trac.osgeo.org/proj/
VER_LIBPROJ	:= 4.7.0
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
VER_LIBSQLITE	:= 3.6.21
# http://www.libpng.org/
# http://www.libpng.org/pub/png/libpng.html
VER_LIBPNG	:= 1.2.41
# http://www.zlib.net/
VER_ZLIB	:= 1.2.3
# http://www.libtiff.org/
# ftp://ftp.remotesensing.org/pub/libtiff
VER_LIBTIFF	:= 4.0.0beta5
# http://shapelib.maptools.org/
# http://dl.maptools.org/dl/shapelib/
VER_LIBSHP	:= 1.2.10
# http://code.google.com/p/libsquish/
# http://code.google.com/p/libsquish/downloads/list
VER_LIBSQUISH	:= 1.10
# http://www.boost.org/
# http://sourceforge.net/projects/boost/files/
VER_BOOST	:= 1_43_0
BOOST_SHORTVER	:= 1_43
# http://www.mesa3d.org/
# http://sourceforge.net/projects/mesa3d/files/
VER_MESA	:= 7.5
# http://expat.sourceforge.net/
# http://sourceforge.net/projects/expat/files/
VER_LIBEXPAT	:= 2.0.1
# http://gmplib.org/
# http://gmplib.org/#DOWNLOAD
VER_LIBGMP	:= 4.3.1
# http://www.mpfr.org/
# http://www.mpfr.org/mpfr-current/#download
VER_LIBMPFR	:= 2.4.2
# http://curl.haxx.se/
# http://curl.haxx.se/download.html
VER_LIBCURL := 7.36.0
# http://http://www.openssl.org/
# https://www.openssl.org/source/
VER_LIBSSL := 1.0.1g
# http://www.dimin.net/software/geojasper/
VER_GEOJASPER := 1.701.0.GEO

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
	CROSSPREFIX	:= x86_64-w64-mingw32-
	CROSSHOST	:= x86_64-w64-mingw32
	ARCHITECTURE	:= x86_64
else
	cross		:= ""
endif
else
ifdef PLAT_MINGW
	MULTI_SUFFIX	:=
#	CROSSPREFIX	:= i686-w64-mingw32-
#	CROSSHOST	:= i686-w64-mingw32
	ARCHITECTURE	:= i686
else
	cross		:= ""
endif
endif

DEFAULT_PREFIX		:= "`pwd`/../local"$(MULTI_SUFFIX)
DEFAULT_LIBDIR		:= "$(DEFAULT_PREFIX)/lib"
DEFAULT_INCDIR		:= "$(DEFAULT_PREFIX)/include"

ifeq ($(PLATFORM), Darwin)
	PLAT_DARWIN := Yes
	# Ben removed ppc and x86_64 to fix libgmp compilation
	DEFAULT_MACARGS	:= -isysroot /Developer/SDKs/MacOSX10.6.sdk -mmacosx-version-min=10.6 -arch i386
	VIS	:= -fvisibility=hidden
endif
ifeq ($(PLATFORM), Linux)
	PLAT_LINUX := Yes
	VIS	:= -fvisibility=hidden
endif

# boost
ARCHIVE_BOOST		:= boost_$(VER_BOOST).tar.gz

# mesa headers
ARCHIVE_MESA		:= mesa-headers-$(VER_MESA).tar.gz

# zlib
ARCHIVE_ZLIB		:= zlib-$(VER_ZLIB).tar.gz
AR_ZLIB			:= "$(CROSSPREFIX)ar rcs"
CC_ZLIB			:= "$(CROSSPREFIX)gcc"
CFLAGS_ZLIB		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_ZLIB		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_ZLIB		:= --prefix=$(DEFAULT_PREFIX)

# libgmp
ARCHIVE_LIBGMP		:= gmp-$(VER_LIBGMP).tar.gz
CFLAGS_LIBGMP		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
CXXFLAGS_LIBGMP		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_LIBGMP		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBGMP		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBGMP		+= --enable-shared=no
CONF_LIBGMP		+= --enable-cxx
# no assembler code
ifdef PLAT_DARWIN
# Ben turned off to fix bug in WED
#CONF_LIBGMP		+= --enable-fat
CONF_LIBGMP		+= --host=none-apple-darwin
endif
ifdef PLAT_MINGW
CONF_LIBGMP		+= --host=none-pc-mingw32
#CONF_LIBGMP		+= --host=$(CROSSHOST)
endif
ifdef PLAT_LINUX
CONF_LIBGMP		+= --host=none-pc-linux-gnu
endif

# libmpfr
ARCHIVE_LIBMPFR		:= mpfr-$(VER_LIBMPFR).tar.gz
CFLAGS_LIBMPFR		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_LIBMPFR		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBMPFR		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBMPFR		+= --enable-shared=no
CONF_LIBMPFR		+= --disable-dependency-tracking
ifdef PLAT_MINGW
CONF_LIBMPFR		+= --host=$(CROSSHOST)
endif

# libexpat
ARCHIVE_LIBEXPAT	:= expat-$(VER_LIBEXPAT).tar.gz
CFLAGS_LIBEXPAT		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_LIBEXPAT	:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBEXPAT		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBEXPAT		+= --enable-shared=no
ifdef PLAT_MINGW
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
ifdef PLAT_MINGW
CONF_LIBPNG		+= --host=$(CROSSHOST)
endif

# freetype
ARCHIVE_FREETYPE	:= freetype-$(VER_FREETYPE).tar.gz
CFLAGS_FREETYPE		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_FREETYPE	:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_FREETYPE		:= --prefix=$(DEFAULT_PREFIX)
CONF_FREETYPE		+= --enable-shared=no
CONF_FREETYPE		+= --with-zlib
ifdef PLAT_MINGW
CONF_FREETYPE		+= --host=$(CROSSHOST)
endif

# libjpeg
ARCHIVE_LIBJPEG		:= jpeg-$(VER_LIBJPEG).tar.gz
CC_LIBJPEG		:= "$(CROSSPREFIX)gcc"
CFLAGS_LIBJPEG		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_LIBJPEG		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBJPEG		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBJPEG		+= --disable-dependency-tracking
CONF_LIBJPEG		+= --enable-shared=no
ifdef PLAT_MINGW
CONF_LIBJPEG		+= --host=$(CROSSHOST)
endif

# libtiff
ARCHIVE_LIBTIFF		:= tiff-$(VER_LIBTIFF).tar.gz
CFLAGS_LIBTIFF		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
CXXFLAGS_LIBTIFF	:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
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
ifdef PLAT_MINGW
CONF_LIBTIFF		+= --host=$(CROSSHOST)
endif

# libproj
ARCHIVE_LIBPROJ		:= proj-$(VER_LIBPROJ).tar.gz
CFLAGS_LIBPROJ		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_LIBPROJ		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBPROJ		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBPROJ		+= --enable-shared=no
CONF_LIBPROJ		+= --disable-dependency-tracking
CONF_LIBPROJ		+= CCDEPMODE="depmode=none"
ifdef PLAT_MINGW
CONF_LIBPROJ		+= --without-mutex
CONF_LIBPROJ		+= --host=$(CROSSHOST)
endif

# geotiff
ARCHIVE_GEOTIFF		:= libgeotiff-$(VER_GEOTIFF).tar.gz
AR_GEOTIFF		:= "$(CROSSPREFIX)ar"
LD_GEOTIFF		:= "$(CROSSPREFIX)ld"
CFLAGS_GEOTIFF		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_GEOTIFF		:= $(M32_SWITCH) -L$(DEFAULT_LIBDIR)
CONF_GEOTIFF		:= --prefix=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --enable-shared=no
CONF_GEOTIFF		+= --without-ld-shared
CONF_GEOTIFF		+= --with-zip=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --with-jpeg=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --with-libtiff=$(DEFAULT_PREFIX)
CONF_GEOTIFF		+= --with-proj=$(DEFAULT_PREFIX)
ifdef PLAT_MINGW
CONF_GEOTIFF		+= --host=$(CROSSHOST)
CONF_GEOTIFF		+= --target=$(CROSSHOST)
endif

# sqlite
ARCHIVE_LIBSQLITE	:= sqlite-amalgamation-$(VER_LIBSQLITE).tar.gz
CFLAGS_LIBSQLITE	:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_LIBSQLITE	:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBSQLITE		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBSQLITE		+= --enable-shared=no
CONF_LIBSQLITE		+= --disable-dependency-tracking
ifdef PLAT_MINGW
CONF_LIBSQLITE		+= --host=$(CROSSHOST)
endif

# lib3ds
ARCHIVE_LIB3DS		:= lib3ds-$(VER_LIB3DS).tar.gz
CFLAGS_LIB3DS		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_LIB3DS		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIB3DS		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIB3DS		+= --enable-shared=no
CONF_LIB3DS		+= --enable-maintainer-mode
CONF_LIB3DS		+= --disable-dependency-tracking
CONF_LIB3DS		+= CCDEPMODE="depmode=none"
ifdef PLAT_MINGW
CONF_LIB3DS		+= --host=$(CROSSHOST)
endif

# libcgal
ARCHIVE_CGAL		:= CGAL-$(VER_CGAL).tar.gz


# libsquish
ARCHIVE_LIBSQUISH	:= squish-$(VER_LIBSQUISH).tar.gz
CONF_LIBSQUISH		:= INSTALL_DIR=$(DEFAULT_PREFIX)
CONF_LIBSQUISH		+= CPPFLAGS="$(DEFAULT_MACARGS) -O2 -I$(DEFAULT_INCDIR) $(M32_SWITCH) $(VIS)"
CONF_LIBSQUISH		+= AR="$(CROSSPREFIX)ar" CXX="$(CROSSPREFIX)g++"

# libdime
ARCHIVE_LIBDIME		:= dime-$(VER_LIBDIME).tar.gz
CFLAGS_LIBDIME		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_LIBDIME		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBDIME		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBDIME		+= --enable-static=yes
CONF_LIBDIME		+= --enable-shared=no
CONF_LIBDIME		+= --disable-dependency-tracking
ifdef PLAT_MINGW
CONF_LIBDIME		+= --host=$(CROSSHOST)
endif

# libshp
ARCHIVE_LIBSHP		:= shapelib-$(VER_LIBSHP).tar.gz
CFLAGS_LIBSHP		:= "-I$(DEFAULT_MACARGS) $(DEFAULT_INCDIR) -O2 $(M32_SWITCH) $(VIS)"
LDFLAGS_LIBSHP		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH)"
CONF_LIBSHP		:= AR="$(CROSSPREFIX)ar" CC="$(CROSSPREFIX)gcc"
CONF_LIBSHP		+= cross=$(M32_SWITCH)

# libssl
ARCHIVE_LIBSSL		:= openssl-$(VER_LIBSSL).tar.gz
CONF_LIBSSL			:= --openssldir=$(DEFAULT_PREFIX)
ifdef PLAT_DARWIN
CONF_LIBSSL			+= darwin-i386-cc
endif
ifdef PLAT_MINGW
CONF_LIBSSL			+= mingw
endif
ifdef PLAT_LINUX
CONF_LIBSSL			+= linux-x86_64 
endif

# libcurl
ARCHIVE_LIBCURL		:= curl-$(VER_LIBCURL).tar.gz
CFLAGS_LIBCURL		:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_LIBCURL		:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH) -Wl,-search_paths_first"
CONF_LIBCURL		:= --prefix=$(DEFAULT_PREFIX)
CONF_LIBCURL		+= --enable-shared=no
CONF_LIBCURL		+= --with-ssl=$(DEFAULT_PREFIX) --without-libidn --disable-ldap 
CONF_LIBCURL		+= --disable-dependency-tracking

# geojasper
ARCHIVE_LIBJASPER	:= jasper-$(VER_GEOJASPER).tar.gz
CFLAGS_LIBJASPER	:= "$(DEFAULT_MACARGS) -I$(DEFAULT_INCDIR) -O2 $(M32_SWITCH)"
LDFLAGS_LIBJASPER	:= "-L$(DEFAULT_LIBDIR) $(M32_SWITCH) -Wl,-search_paths_first"
CONF_LIBJASPER		:= --prefix=$(DEFAULT_PREFIX)

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
.PHONY: all clean boost mesa_headers zlib libpng libfreetype libjpeg \
libtiff libproj libgeotiff libsqlite lib3ds libcgal libsquish libdime libshp \
libexpat libgmp libmpfr libssl libcurl

all: ./local$(MULTI_SUFFIX)/.xpt_libs
./local$(MULTI_SUFFIX)/.xpt_libs: boost mesa_headers zlib libpng \
libfreetype libjpeg libtiff libproj libgeotiff libsqlite lib3ds libcgal \
libsquish libdime libshp libexpat libgmp libmpfr libssl libcurl libjasper
	@touch ./local$(MULTI_SUFFIX)/.xpt_libs

clean:
	@echo "cleaning 3rd-party libraries, removing `pwd`/local"
	@-rm -rf ./local
	@-rm -rf ./local32
	@-rm -rf ./local64

boost: ./local$(MULTI_SUFFIX)/lib/.xpt_boost
./local$(MULTI_SUFFIX)/lib/.xpt_boost:
	@echo "building boost..."
	@tar -xzf "./archives/$(ARCHIVE_BOOST)"
ifdef PLAT_DARWIN
	@cd "boost_$(VER_BOOST)" && \
	chmod +x bootstrap.sh && \
	./bootstrap.sh --prefix=$(DEFAULT_PREFIX) --with-libraries=thread \
	--libdir=$(DEFAULT_PREFIX)/lib $(BE_QUIET) && \
	./bjam cxxflags="$(VIS) $(DEFAULT_MACARGS)" $(BE_QUIET) && \
	./bjam install $(BE_QUIET)
	@cd local/lib && \
	rm -f *.dylib*
endif
ifdef PLAT_LINUX
	@cd "boost_$(VER_BOOST)" && \
	chmod +x bootstrap.sh && \
	./bootstrap.sh --prefix=$(DEFAULT_PREFIX) --with-libraries=thread \
	--libdir=$(DEFAULT_PREFIX)/lib $(BE_QUIET) && \
	./bjam cxxflags="$(VIS)" $(BE_QUIET) && \
	./bjam install $(BE_QUIET)
	@cd local/lib && \
	rm -f *.so*
endif
ifdef PLAT_MINGW
	@cp patches/0001-boost-tss-mingw.patch "boost_$(VER_BOOST)" && \
	cd "boost_$(VER_BOOST)" && \
	patch -p1 < ./0001-boost-tss-mingw.patch $(BE_QUIET) && \
	bjam.exe install --toolset=gcc --prefix=$(DEFAULT_PREFIX) \
	--libdir=$(DEFAULT_PREFIX)/lib --with-thread $(BE_QUIET)
	@cd local/include && \
	ln -sf boost-$(BOOST_SHORTVER)/boost boost $(BE_QUIET) && \
	rm -rf boost-$(BOOST_SHORTVER)
	@cd local/lib && \
	ln -sf libboost_thread*-mt-$(BOOST_SHORTVER).a libboost_thread.a && \
	rm -f *.lib
endif
	@-rm -rf boost_$(VER_BOOST)
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

libgmp: ./local$(MULTI_SUFFIX)/lib/.xpt_libgmp
./local$(MULTI_SUFFIX)/lib/.xpt_libgmp:
	@echo "building libgmp..."
	@tar -xzf "./archives/$(ARCHIVE_LIBGMP)"
	@cd "gmp-$(VER_LIBGMP)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBGMP) CXXFLAGS=$(CXXFLAGS_LIBGMP) LDFLAGS=$(LDFLAGS_LIBGMP) \
	./configure $(CONF_LIBGMP) $(BE_QUIET)
	@$(MAKE) -C "gmp-$(VER_LIBGMP)" $(BE_QUIET)
	@$(MAKE) -C "gmp-$(VER_LIBGMP)" install $(BE_QUIET)
	@-rm -rf gmp-$(VER_LIBGMP)
	@touch $@

libmpfr: ./local$(MULTI_SUFFIX)/lib/.xpt_libmpfr
./local$(MULTI_SUFFIX)/lib/.xpt_libmpfr: ./local$(MULTI_SUFFIX)/lib/.xpt_libgmp
	@echo "building libmpfr..."
	@tar -xzf "./archives/$(ARCHIVE_LIBMPFR)"
	@cd "mpfr-$(VER_LIBMPFR)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBMPFR) LDFLAGS=$(LDFLAGS_LIBMPFR) \
	./configure $(CONF_LIBMPFR) $(BE_QUIET)
	@$(MAKE) -C "mpfr-$(VER_LIBMPFR)" $(BE_QUIET)
	@$(MAKE) -C "mpfr-$(VER_LIBMPFR)" install $(BE_QUIET)
	@-rm -rf mpfr-$(VER_LIBMPFR)
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
	@cp patches/0001-libtiff-fix-types.patch \
	"tiff-$(VER_LIBTIFF)" && cd "tiff-$(VER_LIBTIFF)" && \
	patch -p1 < ./0001-libtiff-fix-types.patch \
	$(BE_QUIET)
	@cd "tiff-$(VER_LIBTIFF)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBTIFF) CXXFLAGS=$(CXXFLAGS_LIBTIFF) LDFLAGS=$(LDFLAGS_LIBTIFF) \
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
	@cp patches/0001-libproj-disable-win32-mutex.patch \
	"proj-$(VER_LIBPROJ)" && cd "proj-$(VER_LIBPROJ)" && \
	patch -p1 < .//0001-libproj-disable-win32-mutex.patch $(BE_QUIET)
	@cd "proj-$(VER_LIBPROJ)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBPROJ) LDFLAGS=$(LDFLAGS_LIBPROJ) \
	./configure $(CONF_LIBPROJ) $(BE_QUIET)
	@$(MAKE) -C "proj-$(VER_LIBPROJ)" -j1 $(BE_QUIET)
	@$(MAKE) -C "proj-$(VER_LIBPROJ)" install -j1 $(BE_QUIET)
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
	-@$(MAKE) -C "libgeotiff-$(VER_GEOTIFF)" -j1 $(BE_QUIET)
	-@$(MAKE) -C "libgeotiff-$(VER_GEOTIFF)" install -j1 $(BE_QUIET)
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


libcgal: ./local$(MULTI_SUFFIX)/lib/.xpt_libcgal
./local$(MULTI_SUFFIX)/lib/.xpt_libcgal: \
./local$(MULTI_SUFFIX)/lib/.xpt_zlib \
./local$(MULTI_SUFFIX)/lib/.xpt_libgmp \
./local$(MULTI_SUFFIX)/lib/.xpt_libmpfr \
./local$(MULTI_SUFFIX)/lib/.xpt_boost
	@echo "building libcgal..."
	@-mkdir -p "./local$(MULTI_SUFFIX)/include"
	@-mkdir -p "./local$(MULTI_SUFFIX)/lib"
	@tar -xzf "./archives/$(ARCHIVE_CGAL)"
	#@cp patches/0001-libcgal-3.4-various-fixes.patch \
	#"CGAL-$(VER_CGAL)" && cd "CGAL-$(VER_CGAL)" && \
	#patch -p1 < ./0001-libcgal-3.4-various-fixes.patch $(BE_QUIET)
ifdef PLAT_DARWIN
	@cd "CGAL-$(VER_CGAL)" && \
	export MACOSX_DEPLOYMENT_TARGET=10.6 && CXXFLAGS="-fvisibility=hidden" cmake \
	-DCMAKE_INSTALL_PREFIX=$(DEFAULT_PREFIX) -DCMAKE_BUILD_TYPE=Release \
	-DBUILD_SHARED_LIBS=FALSE \
	-DCGAL_CXX_FLAGS="-isysroot /Developer/SDKs/MacOSX10.6.sdk -arch i386 -I$(DEFAULT_INCDIR)" \
	-DCGAL_MODULE_LINKER_FLAGS="-L$(DEFAULT_LIBDIR)" \
	-DCGAL_SHARED_LINKER_FLAGS="-L$(DEFAULT_LIBDIR)" \
	-DCGAL_EXE_LINKER_FLAGS="-L$(DEFAULT_LIBDIR)" \
	-DGMP_INCLUDE_DIR=$(DEFAULT_INCDIR) \
	-DGMP_LIBRARIES_DIR=$(DEFAULT_LIBDIR) \
	-DGMP_LIBRARIES=$(DEFAULT_LIBDIR)/libgmp.a \
	-DGMPXX_INCLUDE_DIR=$(DEFAULT_INCDIR) \
	-DGMPXX_LIBRARIES=$(DEFAULT_LIBDIR)/libgmpxx.a \
	-DMPFR_INCLUDE_DIR=$(DEFAULT_INCDIR) \
	-DMPFR_LIBRARIES_DIR=$(DEFAULT_LIBDIR) \
	-DMPFR_LIBRARIES=$(DEFAULT_LIBDIR)/libmpfr.a \
	-DCMAKE_CXX_COMPILER=/usr/bin/g++ \
	-DCMAKE_C_COMPILER=/usr/bin/gcc \
	-DWITH_CGAL_ImageIO=OFF -DWITH_CGAL_PDB=OFF -DWITH_CGAL_Qt3=OFF \
	-DWITH_CGAL_Qt4=OFF $(BE_QUIET) . && \
	make $(BE_QUIET) && make install $(BE_QUIET)
endif
ifdef PLAT_LINUX
	@cd "CGAL-$(VER_CGAL)" && \
	cmake . -DCMAKE_INSTALL_PREFIX=$(DEFAULT_PREFIX) \
	-DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=FALSE \
	-DCGAL_CXX_FLAGS="$(VIS) -I$(DEFAULT_INCDIR)" \
	-DCGAL_MODULE_LINKER_FLAGS="-L$(DEFAULT_LIBDIR)" \
	-DCGAL_SHARED_LINKER_FLAGS="-L$(DEFAULT_LIBDIR)" \
	-DCGAL_EXE_LINKER_FLAGS="-L$(DEFAULT_LIBDIR)" \
	-DGMP_INCLUDE_DIR=$(DEFAULT_INCDIR) \
	-DGMP_LIBRARIES_DIR=$(DEFAULT_LIBDIR) \
	-DGMP_LIBRARIES=$(DEFAULT_LIBDIR)/libgmp.a \
	-DGMPXX_INCLUDE_DIR=$(DEFAULT_INCDIR) \
	-DGMPXX_LIBRARIES=$(DEFAULT_LIBDIR)/libgmpxx.a \
	-DMPFR_INCLUDE_DIR=$(DEFAULT_INCDIR) \
	-DMPFR_LIBRARIES_DIR=$(DEFAULT_LIBDIR) \
	-DMPFR_LIBRARIES=$(DEFAULT_LIBDIR)/libmpfr.a \
	-DWITH_CGAL_ImageIO=OFF -DWITH_CGAL_PDB=OFF -DWITH_CGAL_Qt3=OFF \
	-DWITH_CGAL_Qt4=OFF -DBoost_INCLUDE_DIR=$(DEFAULT_PREFIX)/include \
	-DBOOST_ROOT=$(DEFAULT_PREFIX) $(BE_QUIET) && \
	make $(BE_QUIET) && make install $(BE_QUIET)
endif
ifdef PLAT_MINGW
	@cd "CGAL-$(VER_CGAL)" && \
	cmake -G "MSYS Makefiles" . -DCMAKE_INSTALL_PREFIX=$(DEFAULT_PREFIX) \
	-DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=FALSE \
	-DCGAL_CXX_FLAGS="-I$(DEFAULT_INCDIR)" \
	-DCGAL_MODULE_LINKER_FLAGS="-L$(DEFAULT_LIBDIR)" \
	-DCGAL_SHARED_LINKER_FLAGS="-L$(DEFAULT_LIBDIR)" \
	-DCGAL_EXE_LINKER_FLAGS="-L$(DEFAULT_LIBDIR)" \
	-DGMP_INCLUDE_DIR=$(DEFAULT_INCDIR) \
	-DGMP_LIBRARIES_DIR=$(DEFAULT_LIBDIR) \
	-DGMP_LIBRARIES=$(DEFAULT_LIBDIR)/libgmp.a \
	-DGMPXX_INCLUDE_DIR=$(DEFAULT_INCDIR) \
	-DGMPXX_LIBRARIES=$(DEFAULT_LIBDIR)/libgmpxx.a \
	-DMPFR_INCLUDE_DIR=$(DEFAULT_INCDIR) \
	-DMPFR_LIBRARIES_DIR=$(DEFAULT_LIBDIR) \
	-DMPFR_LIBRARIES=$(DEFAULT_LIBDIR)/libmpfr.a \
	-DWITH_CGAL_ImageIO=OFF -DWITH_CGAL_PDB=OFF -DWITH_CGAL_Qt3=OFF \
	-DWITH_CGAL_Qt4=OFF $(BE_QUIET) && \
	make $(BE_QUIET) && make install $(BE_QUIET)
endif
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

libssl: ./local$(MULTI_SUFFIX)/lib/.xpt_libssl
./local$(MULTI_SUFFIX)/lib/.xpt_libssl:
	@echo "buliding libssl"
	tar -xzf "./archives/$(ARCHIVE_LIBSSL)"
	cd "openssl-$(VER_LIBSSL)" && \
	chmod +x Configure && \
	./Configure $(CONF_LIBSSL)
	cd "openssl-$(VER_LIBSSL)" && make install
	@-rm -rf "openssl-$(VER_LIBSSL)"
	@touch $@

libcurl: ./local$(MULTI_SUFFIX)/lib/.xpt_libcurl
./local$(MULTI_SUFFIX)/lib/.xpt_libcurl:
	@echo "building libcurl..."
	@tar -xzf "./archives/$(ARCHIVE_LIBCURL)"
ifdef PLAT_MINGW
	@cd "curl-$(VER_LIBCURL)" && rm src/tool_hugehelp.c
endif
	cd "curl-$(VER_LIBCURL)" && \
	chmod +x configure && \
	CFLAGS=$(CFLAGS_LIBCURL) LDFLAGS=$(LDFLAGS_LIBCURL) \
	./configure $(CONF_LIBCURL) $(BE_QUIET)
	@$(MAKE) -C "curl-$(VER_LIBCURL)" $(BE_QUIET)
	@$(MAKE) -C "curl-$(VER_LIBCURL)" install $(BE_QUIET)
	@-rm -rf curl-$(VER_LIBCURL)
	@touch $@

libjasper: ./local$(MULTI_SUFFIX)/lib/.xpt_libjasper
./local$(MULTI_SUFFIX)/lib/.xpt_libjasper: ./local$(MULTI_SUFFIX)/lib/.xpt_libjpeg
	@echo "building libjasper..."
	@tar -xzf "./archives/$(ARCHIVE_LIBJASPER)"
	@cd "jasper-$(VER_GEOJASPER)" && \
	chmod u+x configure && \
	CFLAGS=$(CFLAGS_LIBJASPER) LDFLAGS=$(LDFLAGS_LIBJASPER) \
	./configure $(CONF_LIBJASPER) $(BE_QUIET)
	@$(MAKE) -C "jasper-$(VER_GEOJASPER)" $(BE_QUIET)
	@$(MAKE) -C "jasper-$(VER_GEOJASPER)" install $(BE_QUIET)
	@rm -rf "jasper-$(VER_GEOJASPER)"
	@touch $@
