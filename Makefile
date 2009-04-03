BUILDDIR	:= ./build
LIBRARIES	:= ./libs/local/.xpt_libs
# default to public debug_opt configuration
ifndef conf
conf		:= debug_opt
endif

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

.PHONY: all clean distclean libs ObjView WED DSFTool DDSTool ObjConverter \
MeshTool RenderFarm ac3d XGrinder

ifndef PLAT_LINUX
all: WED MeshTool ObjView DSFTool DDSTool ObjConverter RenderFarm ac3d XGrinder
else
all: WED MeshTool ObjView DSFTool DDSTool ObjConverter RenderFarm ac3d
endif

libs: $(LIBRARIES)

$(LIBRARIES):
	@$(MAKE) -s -C "./libs" all

ObjView: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

WED: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

DSFTool: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

DDSTool: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

ObjConverter: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

MeshTool: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

ifndef PLAT_LINUX
XGrinder: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk
endif

RenderFarm: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

ac3d:
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) \
	TARGET=$(TARGETDIR)/$@ -s -f makerules/global/toplevel.mk

clean:
	@echo "cleaning xptools tree, removing $(BUILDDIR)"
	@-rm -rf $(BUILDDIR)

distclean: clean
	@$(MAKE) -s -C "./libs" clean
