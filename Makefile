CC=gcc
CPP=g++
LINK=g++
DEFINES=-DLIN=1 -DIBM=0 -DAPL=0 -DDEV=0 -LIL=1 -BIG=0
INCLUDES=\
	-Isrc/Env \
	-Isrc/DSF \
	-Isrc/GUI \
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
	-Ilibsrc/CGAL-3.0/include/CGAL/config/powerpc_Darwin-8.8_g++-4.0.1 \
	-Ilibsrc/CGAL-3.0/include \
	-Ilibsrc/squish-1.10 \
	-I/usr/include/freetype2 \
	-ISDK/PVR \

CFLAGS=$(DEFINES) $(INCLUDES) -include src/Obj/XDefs.h -include limits.h
CPPFLAGS=$(DEFINES) $(INCLUDES) -include src/Obj/XDefs.h -include limits.h

SRC_squish=\
libsrc/squish-1.10/alpha.o \
libsrc/squish-1.10/clusterfit.o \
libsrc/squish-1.10/colourblock.o \
libsrc/squish-1.10/colourfit.o \
libsrc/squish-1.10/colourset.o \
libsrc/squish-1.10/maths.o \
libsrc/squish-1.10/rangefit.o \
libsrc/squish-1.10/singlecolourfit.o \
libsrc/squish-1.10/squish.o


SRC_DSFTool=\
src/DSF/DSFLib.o \
src/DSF/DSFLibWrite.o \
src/DSF/DSFPointPool.o \
src/DSFTools/DSFToolCmdLine.o \
src/DSFTools/DSF2Text.o \
src/DSFTools/ENV2Overlay.o \
src/Env/EnvParser.o \
src/Env/Persistence.o \
src/Utils/AssertUtils.o \
src/Utils/EndianUtils.o \
src/Utils/FileUtils.o \
src/Utils/md5.o \
src/Utils/XChunkyFileUtils.o

SRC_DDSTool=\
src/XPTools/DDSTool.o \
src/Utils/BitmapUtils.o \
src/Utils/PlatformUtils.lin.o

SRC_ObjConverter=\
src/Obj/ObjConvert.o \
src/Obj/ObjPointPool.o \
src/Obj/XObjBuilder.o \
src/Obj/XObjDefs.o \
src/Obj/XObjReadWrite.o \
src/Obj/XObjWriteEmbedded.o \
src/Utils/ObjUtils.o \
src/Utils/AssertUtils.o \
src/Utils/MatrixUtils.o \
src/XPTools/ConvertObj3DS.o \
src/XPTools/ConvertObj.o \
src/XPTools/ConvertObjDXF.o \
SDK/PVR/PVRTTriStrip.o \
SDK/PVR/PVRTGeometry.o

SRC_ObjView=\
src/Obj/ObjPointPool.o \
src/Obj/XObjDefs.o \
src/Obj/XObjReadWrite.o \
src/Obj/ObjDraw.o \
src/ObjEdit/OE_Zoomer3d.o \
src/Utils/ObjUtils.o \
src/Utils/BitmapUtils.o \
src/Utils/TexUtils.o \
src/Utils/trackball.o \
src/Utils/XUtils.o \
src/Utils/GeoUtils.o \
src/Utils/MatrixUtils.o \
src/Utils/PlatformUtils.lin.o \
src/UI/XWin.lin.o \
src/UI/XWinGL.lin.o \
src/UI/xdnd.o \
src/UI/XGUIApp.o \
src/XPTools/ViewObj.o

SRC_WED=\
src/UI/FontMgr.o \
src/UI/XGUIApp.o \
src/UI/XWinGL.lin.o \
src/UI/XWin.lin.o \
src/WEDCore/WED_Application.o \
src/WEDCore/WED_AppMain.o \
src/WEDCore/WED_Archive.o \
src/WEDCore/WED_Buffer.o \
src/WEDCore/WED_Document.o \
src/WEDCore/WED_EnumSystem.o \
src/WEDCore/WED_Errors.o \
src/WEDCore/WED_FastBuffer.o \
src/WEDCore/WED_Package.o \
src/WEDCore/WED_PackageMgr.o \
src/WEDCore/WED_Persistent.o \
src/WEDCore/WED_Properties.o \
src/WEDCore/WED_PropertyHelper.o \
src/WEDCore/WED_TexMgr.o \
src/WEDCore/WED_UndoLayer.o \
src/WEDCore/WED_UndoMgr.o

SRC_FONTTOOL=\
src/fonttool/fonttool.o


SRC_ObjView+=$(SRC_squish)
SRC_DDSTool+=$(SRC_squish)

SRC_MeshTool=\
src/MeshTool/MeshTool.o

all:	objview dsftool ddstool objconverter

%.o: %.cp
	$(CPP) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@	

%.o: %.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

objview: $(SRC_ObjView)
	$(LINK) -o ObjView -lGL -lGLU -lpng $(SRC_ObjView)

dsftool: $(SRC_DSFTool)
	$(LINK) -o DSFTool $(SRC_DSFTool)

ddstool: $(SRC_DDSTool)
	$(LINK) -o DDSTool -lpng $(SRC_DDSTool)

objconverter: $(SRC_ObjConverter)
	$(LINK) -o ObjConverter -l3ds -ldime $(SRC_ObjConverter)

meshtool: $(SRC_MeshTool)
	$(LINK) -o MeshTool $(SRC_MeshTool)

wed: $(SRC_WED)
	$(LINK) -o WED $(SRC_WED)

fonttool: $(SRC_FONTTOOL)
	$(LINK) -o fonttool -lfreetype -lz -lpng -s $(SRC_FONTTOOL)

clean:
	rm -f $(SRC_DSFTool) $(SRC_DDSTool) $(SRC_ObjConverter) $(SRC_MeshTool) $(SRC_ObjView) $(SRC_WED) $(SRC_FONTTOOL)

distclean:	clean
	-rm -f ObjView
	-rm -f DDSTool
	-rm -f DSFTool
	-rm -f MeshTool
	-rm -f WED
	-rm -f ObjConverter
	-rm -f fonttool

