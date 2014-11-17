GeoTIFF Driver for JasPer(r) library based on libtiff(r) and libjpeg(r)

Copyright (c) 2003-2005 Dmitry V. Fedorov <www.dimin.net> <dima@dimin.net>
  DIMIN SOFTWARE <www.dimin.net>
  up to 1.2.2 DPI, INPE <www.dpi.inpe.br> and DIMIN SOFTWARE <www.dimin.net>
  All rights reserved

URLs:
  JasPer  : <http://www.ece.uvic.ca/~mdadams/jasper/> 
  libtiff : <http://www.libtiff.org/>
  libjpeg : <http://www.ijg.org/>

1.2.6 Compilation

Compile it the same way you would do with regular jasper.
For linux you need libjpeg 6b installed on your system.

Linux:
  configure
  make
  
Windows:
 use geojasper.dsw for VC 6
 for intel compiler link with libs: libirc.lib libm.lib

1.2.6 Implementation considerations:

Linux:
I moved library to libtiff 3.6.1 and libgeotiff 1.2.1 but since this
combination is not usually found on modern linux distributions
i compile and link both of them statically inside geojasper make...

Windows:
libtiff, libgeotiff and libjpeg are now compiled automatically into the
geojasper project.

Modification Considerations:

In order to add TIFF support for JasPer library few of JasPer files should be
modified, they are given at the distribution. These files are based on JasPer 1.701.0
so if you got newer library, find TIFF initialization routines and modify your lib.

Link against libtiff and libjpeg (if needed) so the TIFF driver would work.
libjpeg is needed for libtiff to support jpeg encoding, if you don't
need it, modify proper files in libtiff. 

For MSVC 6 or 7:
Place libtiff.lib and libjpeg.lib inside tiff directory so the Project will compile.
The libtiff source should be placed into the tiff\src dir. I left there header files.

For UNIX:
Compile in the same way as you do with normal jasper.
configure
make
