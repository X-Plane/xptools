BUILDDIR	:= ./build
LIBRARIES	:= ./libs/local/.xpt_libs
# default to public debug_opt configuration
ifndef conf
conf		:= debug_opt
endif
PLATFORM	:= $(shell uname)
ifneq (, $(findstring MINGW, $(PLATFORM)))
	PLATFORM	:= Mingw
endif
TARGETDIR	:= $(BUILDDIR)/$(PLATFORM)/$(conf)

.PHONY: all clean distclean libs ObjView WED DSFTool DDSTool ObjConverter \
MeshTool RenderFarm fonttool ac3d

all: WED MeshTool ObjView DSFTool DDSTool ObjConverter RenderFarm ac3d

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
