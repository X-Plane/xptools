TYPE		:= LIBDYNAMIC
CFLAGS		:= -include src/Obj/XDefs.h
CXXFLAGS	:= -include src/Obj/XDefs.h
LDFLAGS		:= -static -static-libgcc
LIBS		:= 
ifeq ($(PLATFORM), Mingw)
DEFINES		:= -DDIME_NOT_DLL -DUNDER_CE
endif

SOURCES :=\
src/DSF/DSFLib.cpp \
src/DSF/DSFLibWrite.cpp \
src/DSF/DSFPointPool.cpp \
src/DSF/tri_stripper_101/tri_stripper.cpp \
src/Utils/AssertUtils.cpp \
src/Utils/FileUtils.cpp \
src/Utils/XChunkyFileUtils.cpp \
src/Utils/md5.c \
src/Obj/ObjConvert.cpp \
src/Obj/ObjPointPool.cpp \
src/Obj/XObjBuilder.cpp \
src/Obj/XObjDefs.cpp \
src/Obj/XObjReadWrite.cpp \
src/AC3DPlugins/obj_editor.cpp \
src/AC3DPlugins/ac_utils.cpp \
src/AC3DPlugins/bitmap_match.cpp \
src/AC3DPlugins/dsf_export.cpp \
src/AC3DPlugins/obj8_export.cpp \
src/AC3DPlugins/obj8_import.cpp \
src/AC3DPlugins/obj_anim.cpp \
src/AC3DPlugins/obj_model.cpp \
src/AC3DPlugins/obj_panel.cpp \
src/AC3DPlugins/obj_radius.cpp \
src/AC3DPlugins/obj_tools.cpp \
src/AC3DPlugins/obj_update.cpp \
src/AC3DPlugins/prefs.cpp \
src/AC3DPlugins/tcl_utils.cpp \
src/AC3DPlugins/TclStubs.cpp \
src/AC3DPlugins/uv_mapper.cpp \
src/AC3DPlugins/xp_plugin.cpp

ifeq ($(PLATFORM), Linux)
MESA_HEADERS = -Ilibs/local/include/mesa
endif

INCLUDEPATHS :=\
-Ilibs/local/include \
-Ilibs/local/include/freetype2 \
$(MESA_HEADERS) \
-Ilibsrc/expat/xmlparse \
-Ilibsrc/expat/xmltok \
-Isrc/Env \
-Isrc/DSF \
-Isrc/DSF/tri_stripper_101 \
-Isrc/GUI \
-Isrc/GUI/mmenu \
-Isrc/Interfaces \
-Isrc/Obj \
-Isrc/ObjEdit \
-Isrc/OGLE \
-Isrc/UI \
-Isrc/WEDCore \
-Isrc/WEDDocs \
-Isrc/WEDEntities \
-Isrc/WEDImportExport \
-Isrc/WEDLayers \
-Isrc/WEDMap \
-Isrc/WEDProperties \
-Isrc/WEDResources \
-Isrc/WEDWindows \
-Isrc/WorldEditor \
-Isrc/XPCompat \
-Isrc/XPWidgets \
-Isrc/Utils \
-Isrc/XESCore \
-Isrc/XESTools \
-Isrc/Installer \
-Isrc/Network \
-ISDK/ac3d

LIBPATHS :=\
-Llibs/local/lib
