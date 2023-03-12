PLATFORM	:= $(shell uname)

ifneq (, $(findstring MINGW, $(PLATFORM)))
TARGETS :=	WED MeshTool ObjView DSFTool DDSTool XGrinder
else
TARGETS :=	WED MeshTool ObjView DSFTool DDSTool RenderFarm XGrinder RenderFarmUI
endif

.PHONY: $(TARGETS) all clean libs release release-test
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

$(TARGETS):
	@export LD_RUN_PATH='$${ORIGIN}/slib' && \
	$(MAKE) -s -f ./makerules/global/toplevel.mk TARGET=$(@) all

:
