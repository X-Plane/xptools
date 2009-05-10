TARGETS :=	WED MeshTool ObjView DSFTool DDSTool ObjConverter RenderFarm \
		ac3d XGrinder RenderFarmUI

.PHONY: $(TARGETS) all clean distclean libs release linkclean release-test

all: $(TARGETS)

debug:
	@$(MAKE) -s -C . conf=debug all

debug-opt:
	@$(MAKE) -s -C . conf=debug_opt all

release:
	@$(MAKE) -s -C . conf=release all

release-opt:
	@$(MAKE) -s -C . conf=release_opt all

release-test:
	@$(MAKE) -s -C . conf=release_test all

libs:
	@$(MAKE) -s -C "./libs" all

clean:
	@$(MAKE) -s -f ./makerules/global/toplevel.mk clean

linkclean:
	@$(MAKE) -s -f ./makerules/global/toplevel.mk TARGET=$(target) linkclean

distclean: clean
	@$(MAKE) -s -C "./libs" clean

$(TARGETS): libs
	@export LD_RUN_PATH='$${ORIGIN}/slib' && \
	$(MAKE) -s -f ./makerules/global/toplevel.mk TARGET=$(@) all
