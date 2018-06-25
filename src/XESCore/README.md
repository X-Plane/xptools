RenderFarm Readme
===========================


Acronyms used throughout the code
--------------------------------------------
- XES (**X**-Plane **E**ditable **S**cenery)---this is a high level binary format for GIS data used by RenderFarm
- [DEM](https://en.wikipedia.org/wiki/USGS_DEM) (**D**igital **E**levation **M**odel)---a raster based format (i.e., uses a bunch of cells in a grid)
- [SRTM](https://en.wikipedia.org/wiki/Shuttle_Radar_Topography_Mission) (**S**huttle **R**adar **T**opography **M**ission)---high res topographic data for the earth
- PMWX (**P**lanar **M**ap **W**ith interse**X**ions)---this is "the map"; the 2-d arrangement of our planes/edges/etc. 
    It's a sub-class of `CGAL::Arrangement_2` that is used for the "big" map (there's a separate class for single blocks).
    A Pmwx's x/y coordinates are always lon/lat, which makes it analogous to `CGAL::Planar_map_2` in modern CGAL 
    (but RenderFarm was written based on "rull rull old" CGAL).
- CDT: **C**onstrained **D**elaunay **t**riangulation
    - [Delaunay triangulation](https://en.wikipedia.org/wiki/Delaunay_triangulation): no point in the set of triangulated point is inside the circumcircle of any triangle in the DT
    - Avoids sliver triangles
    - Maximizes the minimum angle, not the edge length of the triangles 
- CCB (**C**ounter **C**lockwise **B**oundary)---a set of half-edges, the left side of which face... well, the face they define.
    - Holes in the face are instead clockwise (again, so that their left side is to the face)

About Terrain Types
-----------------------------------

Fundamentally there are high level "terrain types" and then there are rules that result in actual terrain file selections.  When we use the rendering farm (same code as meshtool) in its default mode, we only have three terrain types:

- Natural
- Airport
- Water

Basically the source terrain type is an enumeration applied to a polygon in the vector map - those polygons (assuming the terrain type changes) induce edges in the triangle mesh, and the terrain type enumeration is then transfered to each triangle.  

Final terrain files are selected on a per-triangle basis, and one of the many "matching rules" used to select the final terrain file is a match against the terrain type enumeration.  So basically the notion of "airport" vs "natural" is that it lets us have two sets of selection rules...the main set (900 rules maybe??) for natural terrain, but then another dozen rules just to select the right kind of grass for airports.  (Of course within the airport rule set we consider rainfall, latitude, temperature, etc.)

The "custom terrain" facility currently provided to apply orthophotos basically works as follows...each time you invent a "custom terrain type" (associated with an orthophoto) mesh tool:

- Generates a new terrain type enum to be used only for that orthophoto.
- Generates a new rule to map that terrain type to a terrain file based on the orthophoto.  The rule usually says "terrain type must match, nothing else matters", giving the user their orthophoto, no questions asked.

So....

> I have a lot of terrain that should be either shingle, moraine, scree or 
> ice.  What is the right way to make this happen?

Probably NOT using the custom terrain type facility that MeshTool provides - it's really provided for orthophotos and isn't flexible enough...in particular, natural terrain gets blended at its borders...this happens thanks to properties in the config files, but when a rule is created on the fly for custom terrain, the border is set to 0.

Clearly based on your previous work you are at a level where you could use any of the facilities in the code base.

So I think you could set these terrain types up in a few ways...I think it depends on how you KNOW what terrain type you should have.  The existing rule set does NOT pay a lot of attention to the land use in the XES files (res is 121x121 per tile btw) because it comes from GLCC, whose data accuracy is just horrible and atrocious.  We regularly ignore GLCC and stick in whatever we think looks good based on climate/terrain morphology.  With that in mind, you can do some combination of:

1. Import new land use data before applying terrain, feeding it from a new data source and/or

2. Invent new land-use enums as desired and/or

3. Customizing, changing, or adding new rules for natural terrain to get scree based on the conditions you want (which may or may not involve land use enums, either invented by you or older) and/or

4. Inventing new final .ter file types if the ones we have aren't good enough.

Basically you need to decide how rule driven vs. data driven you want to be.  For example, one option would be:

- Invent your own scree type.
- Overlay your scree on our landuse raster data by importing from your sources.
- Add a new rule that says "if I see Andrew's scree, just use the global scenery scree.ter file type regardless of most conditions".

This would essentially override terrain file selection.

Code Overview
---------------------------------------

`XESCore/DEMTables.h` and `.cpp` contain a few key pieces of code:

- `NaturalTerrainInfo_t`---this struct defines one "rule" for picking a final terrain file based on a host of conditions and
- `FindNaturalTerrain()`---this is the function that, given about a billion things it reads out of the mesh, picks a final terrain based on the global table of `NaturalTerrainInfo_t` (`gNaturalTerrainInfo`).
- `LoadDEMTables()`---init code where the global rules tables get built from config files.

`XESCore/EnumSystem.h` contains the token table.  Basically the idea is that all "enumerated" codes live in a global number space...when a XES files is read in, it contains its own mapping from its own enum numbers to string names, so this lets us translate from the old numbering (from when the file was written) to the current numbering, adding any new codes that are in the XES file but not hard-coded into MeshTool.

I mention this because you'll see a huge list of types in `ParamDefs.h` including those started at `lu_usgs_INTERRUPTED_AREAS`---those are our internal codes for the GLCC landuse data. The reason there are two kinds of urban data is Sergio took the dataset and refined some of the landuse types using photoshop, for example differentiating two kinds of urban.

In the config files, `oge2_import.txt` maps the numeric codes as published in the raw GLCC to our string enums, and `landuse_translate.txt` then does a mapping that "dumbs them down"---Sergio looked at the data and went "I don't need this many land uses and most of them are wrong anyway".

So hopefully that will give you some leads...this is stuff I don't normally post in the MeshTool docs because it requires code changes, but with a little hacking you can probably get just about any landuse-assignment effect you want.

Overview of the scenery generation process
---------------------------------------------

The process of generating scenery is a series of data transformations. At a high level, it looks like this:

1. Read DEM data into gDem (via `raster_import` and maybe `-load` commands)
2. Prepare the global vector map (`gMap`, a PMWX)---maybe via `-load`ing XES data, maybe via... well, lots of other places
    - Includes burning in airports from `gApts`
3. Triangulate a CDT mesh (`gTriangulationHi`) out of the global map and DEM data (`gMap` and `gDem`, respectively)
4. Assign terrain types to the mesh (`gTriangulationHi`) based on the `gDem` data
5. Build the DSF.
    - Takes point objects, polygons, and network data from `gMap` vector data
    - Takes elevation, bathymetry, and urban density data from `gDem`
    - Takes mesh data from the `gTriangulationHi` CDT

Numeric Types, CGAL, etc.
----------------------------------

There are three fundamentally tricky GIS algorithms in the rendering process (everything else is application and raster data)...

- Planar map with intersections, used to both merge down polygonal terrain type descriptions and to find the smallest contiguous areas around vector features.
- Delauney triangulation with constraints for the mesh with induced triangle edges for terrain type changes.
- Buffers around polygons, used to buffer out (for airports) and in (for city blocks)...two separate algs are actually used.

[**Tyler notes**: From here on in this section is suspect! We no longer have a hand-rolled PMWX... we're just using a `CGAL::Arrangement_2` types!]

When I started the code, CGAL didn't have the third and I used CGAL for the first two.  At the time, their planar map with intersections was buggy, and after trying to fix bugs only to hit other ones for a while, I got so fed up I rolled my own.  In particular, the templating was so heavy in the CGAL implementation that, for the compilers at the time (CodeWarrior!  Yuck!) the code was very opaque to debug.

While the CGAL implementation of the planar map attempts to be correct using a kernel that can provide the equivalent to exact mumble mumble (predicates or constructions, not sure which it needs) my design was much more hokey: it attempts to create consistent results for a sub-set of uses that I actually need.  That subset is:

- Correct operation when no edges in the input set cross each other (the topo integrate function attempts to pre-split edges using the exact same vertices to meet this condition) and
- Correct operation on input data that is horizontal or vertical (so we don't blow up when tiling/cutting data).

So you're seeing issues with the near_collinear test and location, but there's actually another operation that can go haywire: there is a counter-clockwise-inbetween predicate (given three vectors out of a vertex, how are they ordered?) that is used to sort the edges out of a vertex...assuming you've figured out who is connected to whom, this sorting is needed to build polygons that don't overlap each other...there are various precision cases that can make this primitive blow up.

Unfortunately I always seem to work on the scenery code under fire, which is why we've limped along with the mess we have now for two renders (the 820 render and the 900 render---I think the 800 us-only render was done with CGAL's pmwx, which was also painful at the time).

I think I am faced with three possible strategies:

1. Increase the firepower of the underlying comp-geom algorithms so they don't blow up.  This would imply putting CGAL numerics under the planar map, or using one of theirs (CGAL has been revved several times ... it's possible that they have one that is correct now.  The triangulations have never given me trouble of their own.)

2. Heavily process the input data to assure that it meets the limited abilities of an inexact planar map.  The question then becomes: is this even possible, and how much do we have to smack the data.  TopoIntegrate is sort of an early attempt to do this, but it has its own problems (hrm...PreTopoIntegrateMap? :-)  In particular, having vector data from unrelated data sources is the norm, so there is always a risk of slivering effects between the data.

3. Change the usage of these algorithms to require less heavy processing.  For example, right now we flatten all of the vector data and all of the terrain type polygons into one giant map (whose polygons then have to be merged back together).  It might be possible by avoiding the mixing of datasets to limit the pain the map goes through.

Of the three choices, 3 is the least preferable to me...the reason is that I would like to (in future renderings) start picking vector features as terrain type breaks.  For example, rather than have the farm terrain type be a photo of a set of fields that tiles, I'd like to have a terrain type for each crop, and change terrain types at road intersections.  This means that the vector roads would define the crop pattern but would "promote" vector roads into terrain type polygons...thus roads would continue to need to be in the map.

(The big problem with farm land is that in the US where the bread basket is totally flat the repetition of the farm textures is horribly obvious and our art team has to apply a ton of texture memory to provide variation.  Given that we have to build a mesh anyway and there isn't a huge need to follow the boring terrain, we could spend those polygons to build non-repeating patterns that would look convincing even at large distances.)

So my question is: what was the total set of changes you had to make to the planar map, topointegrate function, and map merge/overlay functions to get your data to "sink"?  Having not mucked with the latest CGAL in a while, I would really like your input on whether option 1 (trust CGAL to work right) is an option.



What can I pipe to `wed_cmds`?
----------------------------------------
In `RF_Main.cpp`, see `sUtilCmds` for a list of all "utility" commands you can use to modify the behavior of RenderFarmUI.

