#!/bin/sh
CC=g++
CFLAGS="-x c++ -I../Obj -I ../Utils -I../../SDK/ac3d -I. -include XPExporterMac_xcode.h"

$CC $CFLAGS -c ac_utils.c
$CC $CFLAGS -c bitmap_match.c
$CC $CFLAGS -c obj8_export.c
$CC $CFLAGS -c obj8_import.c
$CC $CFLAGS -c obj_radius.c
$CC $CFLAGS -c obj_tools.c
$CC $CFLAGS -c xp_plugin.c
$CC $CFLAGS -c ../Obj/ObjConvert.cpp -o ObjConvert.o
$CC $CFLAGS -c ../Obj/ObjPointPool.cpp -o ObjPointPool.o
$CC $CFLAGS -c ../Obj/XObjDefs.cpp -o XObjDefs.o
$CC $CFLAGS -c ../Obj/XObjReadWrite.cpp -o XObjReadWrite.o

g++ -bundle_loader /Applications/ac3dmac/ac3d.app/Contents/MacOS/ac3d -bundle -flat_namespace -undefined warning *.o -o XPluginSupport.p
