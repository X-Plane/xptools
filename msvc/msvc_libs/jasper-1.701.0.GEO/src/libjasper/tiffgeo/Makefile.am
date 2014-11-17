# Copyright (c) 2003-06 Dmitry Fedorov
# All rights reserved. 

noinst_LTLIBRARIES = libgtif.la
libgtif_la_SOURCES = \
  fax3sm_winnt.c \
  tif_aux.c \
  tif_close.c \
  tif_codec.c \
  tif_color.c \
  tif_compress.c \
  tif_dir.c \
  tif_dirinfo.c \
  tif_dirread.c \
  tif_dirwrite.c \
  tif_dumpmode.c \
  tif_error.c \
  tif_extension.c \
  tif_fax3.c \
  tif_flush.c \
  tif_getimage.c \
  tif_jpeg.c \
  tif_luv.c \
  tif_lzw.c \
  tif_next.c \
  tif_open.c \
  tif_packbits.c \
  tif_pixarlog.c \
  tif_predict.c \
  tif_print.c \
  tif_read.c \
  tif_strip.c \
  tif_swab.c \
  tif_thunder.c \
  tif_tile.c \
  tif_version.c \
  tif_warning.c \
  tif_write.c \
  tif_zip.c \
  tif_unix.c \
  cpl_csv_incode.c \
  cpl_serv.c \
  geo_extra.c \
  geo_free.c \
  geo_get.c \
  geo_names.c \
  geo_new.c \
  geo_normalize.c \
  geo_print.c \
  geo_set.c \
  geo_tiffp.c \
  geotiff_proj4.c \
  geo_trans.c \
  geo_write.c \
  xtiff.c \
  csv/datum.c \
  csv/ellipsoid.c \
  csv/gcs.c \
  csv/pcs.c \
  csv/prime_meridian.c \
  csv/projop_wparm.c \
  csv/unit_of_measure.c
    
INCLUDES = -I$(top_srcdir)/src/libjasper/include \
           -I$(top_srcdir)/src/libjasper/tiffgeo \
           -I$(top_srcdir)/src/libproj
  
#libgtif_la_LDFLAGS = -DHAVE_LIBPROJ -DHAVE_PROJECTS_H
CPPFLAGS = -DHAVE_LIBPROJ -DHAVE_PROJECTS_H

