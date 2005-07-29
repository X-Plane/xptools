

CC = g++ -I../include -DNDEBUG 
LD = g++ -L../build
# LD = g++ -L../build-tmake

LIBS = -ldime

.SUFFIXES: .cpp .o

all:: dxf2vrml

clean::
	@rm *.o dxf2vrml

.cpp.o:
	$(CC) -c $<

dxf2vrml: dxf2vrml.o
	$(LD) -o $@ $^ $(LIBS)

