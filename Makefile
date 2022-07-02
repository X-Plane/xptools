XP_TARGETS :=	WED ObjView DSFTool DDSTool XGrinder
TARGETS    :=	$(XP_TARGETS) MeshTool RenderFarm RenderFarmUI

.PHONY: $(TARGETS) all clean libs release

default: $(XP_TARGETS)

all: $(TARGETS)

debug:
	@$(MAKE) -s -C . conf=debug all

debug-opt:
	@$(MAKE) -s -C . conf=debug_opt all

release:
	@$(MAKE) -s -C . conf=release all

release-opt:
	@$(MAKE) -s -C . conf=release_opt all

libs:
	@$(MAKE) -s -C "./libs" all

clean:
	@$(MAKE) -s -f ./makerules/global/toplevel.mk clean

$(TARGETS):
	@export LD_RUN_PATH='$${ORIGIN}/slib' && \
	$(MAKE) -s -f ./makerules/global/toplevel.mk TARGET=$(@) all

:
