BUILDDIR	:= ./build
# default to public debug_opt configuration
ifndef conf
conf		:= debug_opt
endif

ARCHITECTURE	:= $(shell uname -m)
PLATFORM	:= $(shell uname)
ifneq (, $(findstring MINGW, $(PLATFORM)))
PLATFORM	:= Mingw
PLAT_MINGW	:= Yes
endif
ifeq ($(PLATFORM), Linux)
PLAT_LINUX	:= Yes
endif
ifeq ($(PLATFORM), Darwin)
PLAT_DARWIN	:= Yes
endif
TARGETDIR	:= $(BUILDDIR)/$(PLATFORM)/$(conf)

ifeq ($(cross), m32)
ifeq ($(ARCHITECTURE), x86_64)
	MULTI_SUFFIX	:= 32
	M32_SWITCH	:= -m32
else
	cross		:= ""
endif
endif

.PHONY: all clean distclean libs ObjView WED DSFTool DDSTool ObjConverter \
MeshTool RenderFarm ac3d XGrinder

ifndef PLAT_LINUX
all: WED MeshTool ObjView DSFTool DDSTool ObjConverter RenderFarm ac3d XGrinder
else
all: WED MeshTool ObjView DSFTool DDSTool ObjConverter RenderFarm ac3d
endif

libs:
	@$(MAKE) -s -C "./libs" cross=$(cross) gitlibs=$(gitlibs) all

ObjView: libs
	@export LD_RUN_PATH='$${ORIGIN}:$${ORIGIN}/lib' && $(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

WED: libs
	@export LD_RUN_PATH='$${ORIGIN}:$${ORIGIN}/lib' && $(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

DSFTool:
	@export LD_RUN_PATH='$${ORIGIN}:$${ORIGIN}/lib' && $(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

DDSTool: libs
	@export LD_RUN_PATH='$${ORIGIN}:$${ORIGIN}/lib' && $(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

ObjConverter: libs
	@export LD_RUN_PATH='$${ORIGIN}:$${ORIGIN}/lib' && $(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

MeshTool: libs
	@export LD_RUN_PATH='$${ORIGIN}:$${ORIGIN}/lib' && $(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

ifndef PLAT_LINUX
XGrinder: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk
endif

RenderFarm: libs
	@export LD_RUN_PATH='$${ORIGIN}:$${ORIGIN}/lib' && $(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

ac3d:
	@export LD_RUN_PATH='$${ORIGIN}:$${ORIGIN}/lib' && $(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

clean:
	@echo "cleaning xptools tree, removing $(BUILDDIR)"
	@-rm -rf $(BUILDDIR)

distclean: clean
	@$(MAKE) -s -C "./libs" clean
