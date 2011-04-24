all: gen_roads gen_path osm_tile osm2shape split_image GenTerrain shape2xon


gen_roads: gen_roads.cpp
	g++ gen_roads.cpp -o gen_roads

gen_path: genpath.c
	gcc genpath.c -o genpath

osm_tile: osm_tile.c
	gcc osm_tile.c -lz -lexpat -o osm_tile

osm2shape: osm2shape.c
	gcc osm2shape.c -lz -lexpat -lshp -o osm2shape

split_image: SplitImage.c
	gcc SplitImage.c -o split_image

GenTerrain: GenTerrain.c
	gcc GenTerrain.c -o GenTerrain

shape2xon: shape2xon.cpp
	g++ shape2xon.cpp -o shape2xon -lshp -O3
