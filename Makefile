BUILDDIR	:= build
LIBRARIES	:= libsrc/linux-specific/.3rdparty_libs
# default to public debug_opt configuration
ifndef conf
conf		:= debug_opt
endif
PLATFORM	:= $(shell uname)
ifneq (, $(findstring MINGW, $(PLATFORM)))
	PLATFORM	:= Mingw
endif
ifeq ($(cross), mingw)
	PLATFORM	:= Mingw
endif
TARGETDIR	:= $(BUILDDIR)/$(PLATFORM)/$(conf)

.SILENT:
.PHONY: all
.PHONY: clean
.PHONY: distclean

.PHONY: ObjView
.PHONY: WED
.PHONY: DSFTool
.PHONY: DDSTool
.PHONY: ObjConverter
.PHONY: MeshTool
.PHONY: RenderFarm
.PHONY: fonttool

all: WED MeshTool ObjView DSFTool DDSTool ObjConverter RenderFarm fonttool

$(LIBRARIES):
	$(MAKE) -C "libsrc/linux-specific"

ObjView: $(LIBRARIES)
	$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -f makerules/rules.mk

WED: $(LIBRARIES)
	$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -f makerules/rules.mk

DSFTool: $(LIBRARIES)
	$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -f makerules/rules.mk

DDSTool: $(LIBRARIES)
	$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -f makerules/rules.mk

ObjConverter: $(LIBRARIES)
	$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -f makerules/rules.mk

MeshTool: $(LIBRARIES)
	$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -f makerules/rules.mk

RenderFarm: $(LIBRARIES)
	$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -f makerules/rules.mk

fonttool: $(LIBRARIES)
	$(MAKE) conf=$(conf) cross=$(cross) BUILDDIR=$(TARGETDIR) TARGET=$(TARGETDIR)/$@ -f makerules/rules.mk

clean:
	-rm -rf $(BUILDDIR)

distclean:	clean
	$(MAKE) -C "libsrc/linux-specific" clean
