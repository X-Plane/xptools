Structure similar to but not quite a 'pmwx'.

Nodes
Edges are ordered lists of nodes.
Polygons are ordered lists of edges.

Derived back-ptr from edge to 2 polygons gives us adjacency, if desired.

No holes allowed - user is required to create holes by inserting arbitrary edges into polygons.
(Is this restriction okay?  Probably easiest from an editing point of view.)

BUILDING THE LAYOUT:
1. Load airport.
2. Convert bezier chain to segment chain.
3. Build back-links from edges to polygons.  (means for a given exit from a polygon through an edge u know who you hit)
4a. We can triangulate in N^2 time for a given polygon using a greedy triangulation method: for each consecutive pair - 
	cut off to a tri-fan IF no other vertex is inside the polygon and if it's not a sliver.)
4b. "Y- monotone curve" a line segment where aby vertical line will always hit the segment at most once.
	A polygon made of 2 Y monotone curves is a Y mconotone poly and can be triangulated in linear time by
	sort the vertices vertically (subsort horizontally) then just build strips and fans down the list by vertex.
	
	You can reduce a polygon to a y-monotone polygon by breaking it into trapezoids via horizontal lines through all
	vertices.  If a trapezoid has the top pt on the right and bottom on the left, insert a diagonal cutting up
	the polygon.  Once these cuts are made, the resulting polys are y-monotone

	http://algolist.manual.ru/maths/geom/polygon/decompose_seidel.php
	
	
In memory structure would be:
pts
lines - 
	vector of pts
polygon - 
	vector of lines
	vector of edge-touching triangles
	triangulation:
	each triangle should point to 3 other triangles/polygon lines

An airport now has:
1. Point list
2. Polygon side list - ordered of points (and control points)
3. Polygon: 
	Ordered list of sides (optionally reversed)
	Pavement info
4. Marking Lines: 
	Ordered list of points (and contorl points)
	Line Marking color
5. Airport Perimeter or fence
	Ordered List of Points
	(fence must be manually closed!)
6. \