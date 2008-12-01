VER_CGAL      = 3.3.1
VER_FREETYPE  = 2.3.7
VER_LIBPROJ   = 4.6.1
VER_GEOTIFF   = 1.2.5
VER_LIB3DS    = 1.3.0
VER_LIBDIME   = r174
VER_LIBJPEG   = 6b
VER_LIBSQLITE = 3.6.3
VER_LIBPNG    = 1.2.32
VER_ZLIB      = 1.2.3
VER_LIBTIFF   = 4.0.0beta2
VER_LIBSHP    = 1.2.10
VER_LIBSQUISH = 1.10.1

all: lib/libCGAL.a lib/libfreetype.a lib/libproj.a lib/lib3ds.a lib/libdime.a lib/libjpeg.a lib/libsqlite3.a lib/libpng12.a lib/libz.a lib/libtiff.a lib/libshp.a lib/libsquish.a lib/libgeotiff.a
	touch .3rdparty_libs

lib/libCGAL.a:
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/CGAL-$(VER_CGAL).tar.gz"
	cp cgal_patch "CGAL-$(VER_CGAL)/include" && cd "CGAL-$(VER_CGAL)/include" && patch -p1 < ./cgal_patch
	cd "CGAL-$(VER_CGAL)" && chmod +x install_cgal && ./install_cgal --prefix=/tmp/cgalbuild --disable-shared
	cp -Lpr /tmp/cgalbuild/include/* ./include
	cp -Lpr /tmp/cgalbuild/lib/*.a ./lib
	-rm -rdf /tmp/cgalbuild
	-rm -rdf CGAL-$(VER_CGAL)
lib/libfreetype.a: lib/libz.a
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/freetype-$(VER_FREETYPE).tar.gz" && cd "freetype-$(VER_FREETYPE)" && chmod +x configure && ./configure --enable-shared=no
	$(MAKE) -C "freetype-$(VER_FREETYPE)"
	cp -Lpr freetype-$(VER_FREETYPE)/include/* ./include
	cp freetype-$(VER_FREETYPE)/objs/.libs/libfreetype.a ./lib
	-rm -rdf freetype-$(VER_FREETYPE)
lib/libproj.a:
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/proj-$(VER_LIBPROJ).tar.gz" && cd "proj-$(VER_LIBPROJ)" && chmod +x configure && ./configure
	$(MAKE) -C "proj-$(VER_LIBPROJ)"
	cp -Lp proj-$(VER_LIBPROJ)/src/*.h ./include
	cp proj-$(VER_LIBPROJ)/src/.libs/libproj.a ./lib
	-rm -rdf proj-$(VER_LIBPROJ)
lib/libgeotiff.a: lib/libz.a lib/libproj.a lib/libtiff.a lib/libproj.a
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/libgeotiff-$(VER_GEOTIFF).tar.gz" && cd "libgeotiff-$(VER_GEOTIFF)" && chmod +x configure && ./configure --without-ld-shared --with-libtiff=`pwd`/.. --with-proj=`pwd`/..
	$(MAKE) -C "libgeotiff-$(VER_GEOTIFF)"
	cp -Lp libgeotiff-$(VER_GEOTIFF)/*.h ./include
	cp -Lp libgeotiff-$(VER_GEOTIFF)/*.inc ./include
	cp libgeotiff-$(VER_GEOTIFF)/libgeotiff.a ./lib
	-rm -rdf libgeotiff-$(VER_GEOTIFF)
lib/lib3ds.a:
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/lib3ds-$(VER_LIB3DS).tar.gz" && cd "lib3ds-$(VER_LIB3DS)" && chmod +x configure && ./configure
	$(MAKE) -C "lib3ds-$(VER_LIB3DS)"
	mkdir include/lib3ds
	cp -Lp lib3ds-$(VER_LIB3DS)/lib3ds/*.h ./include/lib3ds
	cp lib3ds-$(VER_LIB3DS)/lib3ds/.libs/lib3ds.a ./lib
	-rm -rdf lib3ds-$(VER_LIB3DS)
lib/libdime.a:
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/libdime-$(VER_LIBDIME).tar.gz" && cd "libdime-$(VER_LIBDIME)" && chmod +x configure && ./configure --enable-static
	$(MAKE) -C "libdime-$(VER_LIBDIME)"
	cp -Lpr libdime-$(VER_LIBDIME)/include/dime ./include
	cp libdime-$(VER_LIBDIME)/src/.libs/libdime.a ./lib
	-rm -rdf libdime-$(VER_LIBDIME)
lib/libjpeg.a:
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/jpeg-$(VER_LIBJPEG).tar.gz" && cd "jpeg-$(VER_LIBJPEG)" && chmod +x configure && ./configure
	$(MAKE) -C "jpeg-$(VER_LIBJPEG)"
	cp -Lp jpeg-$(VER_LIBJPEG)/*.h ./include
	cp jpeg-$(VER_LIBJPEG)/libjpeg.a ./lib
	-rm -rdf jpeg-$(VER_LIBJPEG)
lib/libsqlite3.a:
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/sqlite-$(VER_LIBSQLITE).tar.gz" && cd "sqlite-$(VER_LIBSQLITE)" && chmod +x configure && ./configure
	$(MAKE) -C "sqlite-$(VER_LIBSQLITE)"
	cp -Lp sqlite-$(VER_LIBSQLITE)/sqlite3.h ./include
	cp -Lp sqlite-$(VER_LIBSQLITE)/src/sqlite3ext.h ./include
	cp -Lp sqlite-$(VER_LIBSQLITE)/.libs/libsqlite3.a ./lib
	-rm -rdf sqlite-$(VER_LIBSQLITE)
lib/libpng12.a: lib/libz.a
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/libpng-$(VER_LIBPNG).tar.gz" && cd "libpng-$(VER_LIBPNG)" && chmod +x configure && ./configure
	$(MAKE) -C "libpng-$(VER_LIBPNG)"
	-rm -rf libpng-$(VER_LIBPNG)/config.h
	cp -Lpr libpng-$(VER_LIBPNG)/*.h ./include
	cp -Lp libpng-$(VER_LIBPNG)/.libs/libpng12.a ./lib
	cp -Lp libpng-$(VER_LIBPNG)/.libs/libpng.a ./lib
	-rm -rdf libpng-$(VER_LIBPNG)
lib/libz.a:
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/zlib-$(VER_ZLIB).tar.gz" && cd "zlib-$(VER_ZLIB)" && chmod +x configure && ./configure
	$(MAKE) -C "zlib-$(VER_ZLIB)"
	cp -Lp zlib-$(VER_ZLIB)/*.h ./include
	cp zlib-$(VER_ZLIB)/libz.a ./lib
	-rm -rdf zlib-$(VER_ZLIB)
lib/libtiff.a:
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/tiff-$(VER_LIBTIFF).tar.gz" && cd "tiff-$(VER_LIBTIFF)" && chmod +x configure && ./configure
	$(MAKE) -C "tiff-$(VER_LIBTIFF)"
	cp -Lp tiff-$(VER_LIBTIFF)/libtiff/*.h ./include
	cp -Lp tiff-$(VER_LIBTIFF)/libtiff/*.hxx ./include
	cp tiff-$(VER_LIBTIFF)/libtiff/.libs/libtiff.a ./lib
	cp tiff-$(VER_LIBTIFF)/libtiff/.libs/libtiffxx.a ./lib
	-rm -rdf tiff-$(VER_LIBTIFF)
lib/libshp.a:
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/shapelib-$(VER_LIBSHP).tar.gz"
	$(MAKE) -C "shapelib-$(VER_LIBSHP)" lib
	cp -Lp shapelib-$(VER_LIBSHP)/*.h ./include
	cp shapelib-$(VER_LIBSHP)/.libs/libshp.a ./lib
	-rm -rdf shapelib-$(VER_LIBSHP)
lib/libsquish.a:
	-mkdir "include"
	-mkdir "lib"
	tar -xzf "archives/squish-$(VER_LIBSQUISH).tar.gz"
	$(MAKE) -C "squish-$(VER_LIBSQUISH)"
	cp -Lp squish-$(VER_LIBSQUISH)/*.h ./include
	cp squish-$(VER_LIBSQUISH)/libsquish.a ./lib
	-rm -rdf squish-$(VER_LIBSQUISH)

.PHONY: clean

clean:
	-rm -rdf include
	-rm -rdf lib
	-rm -f .3rdparty_libs


