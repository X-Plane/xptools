CC=gcc
CPP=g++
LINK=g++
DEFINES=-DLIN=1 -DIBM=0 -DAPL=0 -DDEV=0 -LIL=1 -BIG=0
INCLUDES=\
	-Isrc/Env \
	-Isrc/DSF \
	-Isrc/Obj \
	-Isrc/Utils \
	-Ilibsrc/squish-1.10

CFLAGS=$(DEFINES) $(INCLUDES) -include src/Obj/XDefs.h
CPPFLAGS=$(DEFINES) $(INCLUDES) -include src/Obj/XDefs.h

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
src/Utils/ObjUtils.o \
src/XPTools/ConvertObj3DS.o \
src/XPTools/ConvertObj.o \
src/XPTools/ConvertObjDXF.o

SRC_DDSTool+=$(SRC_squish)

%.o: %.cp
	$(CPP) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@	

%.o: %.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

dsftool: $(SRC_DSFTool)
	$(LINK) -o DSFTool $(SRC_DSFTool)

ddstool: $(SRC_DDSTool)
	$(LINK) -o DDSTool -lpng $(SRC_DDSTool)

objconverter: $(SRC_ObjConverter)
	$(LINK) -o ObjConverter -l3ds -ldime $(SRC_ObjConverter)

clean:
	rm -f $(SRC_DSFTool)

