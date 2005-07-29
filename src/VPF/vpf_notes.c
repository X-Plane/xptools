VMAP0 NOTES

I
Reference Library Library Reference 			libref
Database Reference 								dbref
Political Entities 								polbnd
Place Names 									placenam
Data Libraries Library Reference 				Iibref
Tile Reference 									tileref
Boundaries 										bnd
Data Quality 									dq
Elevation 										elev
Hydrography 									hydro
Industry 										ind
Physiography 									phys
Populat ion 									pop
Transportat ion 								trans
Utilities 										util
Vegetat ion 									veg


Directory vmaplvO/lib/l
Coverage Attribute Table 						cat
Library Header Table 							lht
Geographic Reference Table 						grt
Data Quality Index File 						dqx
Data Quality Table 								dqt
Lineage Documentation File lineage. 			doc
Tile Reference Coverage Directory vmaplvO/lib/ tileref/
Feature Class Schema Table 						fcs
Tile Reference Area Feature Table tileref. 		aft
Tile Reference Text Feature Table tilereft. 		tft

Primitive Tablesz primitive table and indices
Library Reference Coverage Directory vma~;vO/lib/libref /
Feature Class Schema Table
Library Reference Line Feature Table libref. 	lft
Library Reference Text Feature Table libreft. 	tft
Primitive Tablesz primitive tables
and indices


Data Libraries I 
Librarv Reference		libref
Tile Reference			tileref	
Boundaries				bnd
Data Quality			dq
Elevation				elev
Hydrography				hydro
Industry				ind
Physiography			phys
Population				pop
Transportation			trans.
Utilities				util
Vegetation				veg
Library Reference		libref
Database Reference		dbref
Political Entities		polbnd
Place Names				placenam

------------------------------------------------------------------------------------------
COVERAGE=set of vector data of similar theme and one topology					(trans)
FEATURE CLASS=set of homogeneously typed geo entities representing same stuff 	(roads)

"The money tables"
edg - edges
fac - faces
end - entity nodes
cnd - connected nodes
txt (text who cares)

the 'cat' table lists the sub dir names for each
coverage and its topology, which is CONSISTENT within
a coverage, e.g. if trans is top 2 (network), then ALL
trans (road, rail, walk) are top 2.

".pft, .aft, .lft" are feature tables - 
they link the geometry from the files to metadata.
Note that there is only ONE set of geometry for a coverage
despite multiple thematic ideas.  So there is one massive
network blob for all transportation (albeit since it is topo 2
it may have NO interlinking!)

"fcs" table lists each feature class and the linkage
columns. 
"fca" lists each feature class, the type of 'stuff' in it
(always homogeneous, e.g. all pts or lines) and names it.

GEO DATA STUFF

Geo tables have MULTIPLE IDs, one for each feature class.
Geo data tags with triplets: first Id is the key we want.

Edges: half left and right face, and halfedge in CCW for both sides
