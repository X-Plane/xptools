BINARIES = gen_roads10 gen_roads genpath osm_tile osm2shape split_image GenTerrain shape2xon

all: $(BINARIES)

LOCAL_PATH = ../../libs/local
LOCAL_OPTS = -I$(LOCAL_PATH)/include -L$(LOCAL_PATH)/lib

gen_roads10: gen_roads10.cpp
	g++ gen_roads10.cpp -o gen_roads10

gen_roads: gen_roads.cpp
	g++ gen_roads.cpp -o gen_roads

gen_path: genpath.c
	gcc genpath.c -o genpath

osm_tile: osm_tile.c
	gcc osm_tile.c $(LOCAL_OPTS) -lm -lz -lexpat -o osm_tile

osm2shape: osm2shape.c
	gcc osm2shape.c $(LOCAL_OPTS) -lz -lexpat -lshp -o osm2shape

split_image: SplitImage.c
	gcc SplitImage.c -o split_image

GenTerrain: GenTerrain.c
	gcc GenTerrain.c -o GenTerrain

shape2xon: shape2xon.cpp
	g++ shape2xon.cpp $(LOCAL_OPTS) -o shape2xon -lshp -O3

clean:
	-rm -f $(BINARIES)
