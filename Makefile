BUILDDIR	:= build
LIBRARIES	:= libsrc/linux-specific/.3rdparty_libs

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
	$(MAKE) BUILDDIR=$(BUILDDIR) TARGET=$(BUILDDIR)/$@ -f makerules/rules.mk all

WED: $(LIBRARIES)
	$(MAKE) BUILDDIR=$(BUILDDIR) TARGET=$(BUILDDIR)/$@ -f makerules/rules.mk all

DSFTool: $(LIBRARIES)
	$(MAKE) BUILDDIR=$(BUILDDIR) TARGET=$(BUILDDIR)/$@ -f makerules/rules.mk all

DDSTool: $(LIBRARIES)
	$(MAKE) BUILDDIR=$(BUILDDIR) TARGET=$(BUILDDIR)/$@ -f makerules/rules.mk all

ObjConverter: $(LIBRARIES)
	$(MAKE) BUILDDIR=$(BUILDDIR) TARGET=$(BUILDDIR)/$@ -f makerules/rules.mk all

MeshTool: $(LIBRARIES)
	$(MAKE) BUILDDIR=$(BUILDDIR) TARGET=$(BUILDDIR)/$@ -f makerules/rules.mk all

RenderFarm: $(LIBRARIES)
	$(MAKE) BUILDDIR=$(BUILDDIR) TARGET=$(BUILDDIR)/$@ -f makerules/rules.mk all

fonttool: $(LIBRARIES)
	$(MAKE) BUILDDIR=$(BUILDDIR) TARGET=$(BUILDDIR)/$@ -f makerules/rules.mk all

clean:
	-rm -rf $(BUILDDIR)

distclean:	clean
	$(MAKE) -C "libsrc/linux-specific" clean
