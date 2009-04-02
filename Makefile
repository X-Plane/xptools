BUILDDIR	:= ./build
LIBRARIES	:= ./libs/local/.xpt_libs
# default to public debug_opt configuration
ifndef conf
conf		:= debug_opt
endif

PLATFORM	:= $(shell uname)
ARCHITECTURE	:= $(shell uname -m)
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

ifeq ($(ARCHITECTURE), i386)
	ARCH_I386	:= Yes
endif
ifeq ($(ARCHITECTURE), i686)
	ARCH_I386	:= Yes
endif
ifeq ($(ARCHITECTURE), x86_64)
	ARCH_X86_64	:= Yes
endif
TARGETDIR	:= $(BUILDDIR)/$(PLATFORM)/$(conf)

.PHONY: all clean distclean libs ObjView WED DSFTool DDSTool ObjConverter \
MeshTool RenderFarm fonttool ac3d XGrinder

ifdef PLAT_MINGW
all: WED MeshTool ObjView DSFTool DDSTool ObjConverter RenderFarm ac3d XGrinder
else
all: WED MeshTool ObjView DSFTool DDSTool ObjConverter RenderFarm ac3d
endif

libs: $(LIBRARIES)

$(LIBRARIES):
	@$(MAKE) -C "./libs" all

ObjView: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -s -f makerules/rules.mk

WED: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -s -f makerules/rules.mk

DSFTool: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -s -f makerules/rules.mk

DDSTool: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -s -f makerules/rules.mk

ObjConverter: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -s -f makerules/rules.mk

MeshTool: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -s -f makerules/rules.mk

XGrinder: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -s -f makerules/rules.mk

RenderFarm: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -s -f makerules/rules.mk

fonttool: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -s -f makerules/rules.mk

ac3d: libs
	@$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -s -f makerules/rules.mk

clean:
	@echo "cleaning xptools tree, removing $(BUILDDIR)"
	@-rm -rf $(BUILDDIR)

distclean:	clean
	@$(MAKE) -s -C "./libs" clean
