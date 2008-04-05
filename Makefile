CC=gcc
CPP=g++
LINK=g++
DEFINES=-DLIN=1 -DIBM=0 -DAPL=0
INCLUDES=\
	-Isrc/Env \
	-Isrc/DSF \
	-Isrc/Utils

CFLAGS=$(DEFINES) $(INCLUDES) -include src/Obj/XDefs.h
CPPFLAGS=$(DEFINES) $(INCLUDES) -include src/Obj/XDefs.h

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

%.o: %.cp
	$(CPP) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

dsftool: $(SRC_DSFTool)
	$(LINK) -o DSFTool $(SRC_DSFTool)

clean:
	rm -f $(SRC_DSFTool)

