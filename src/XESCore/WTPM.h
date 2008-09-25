/*
 * Copyright (c) 2004, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#ifndef WTPM_H
#define WTPM_H
/*

	WEAK TOPOLOGICAL PLANAR MAP - THEORY OF OPERATION

	WTPM is a component used to ease import of topological vector data.
	Typically topological exports from GIS sources do not contain full
	DCEL-style links.  These structures provide a base set of topological
	units that can construct full DCEL topology from basic topological
	links.  They can also instantiate a real CGAL planar map.

	Use this translation import as a base for your custom importer.
	You will need to:

	 - Subclass the WTPM GT structures (line, point, face) as the basis
	   for the in-memory storage of your own database.
	 - Doubly link all required links for the structures.
	 - Call the utility routines to recompute the full topological mapping.
	 - Call the export utility to build a planar map.

	DIRECTED-NESS

	Many GIS exports define an edge as a single entity with a left edge
	and a right edge, based on standing at the start and looking toward
	the stop node.

	We define a directed line pointer as one of the two half-edges
	on an edge.  If the boolean flag is true, we're the left halfedge
	(same direction as our supporting edge), if false then the right,
	pointing toward the start from the stop.

	A node's line list needs to be references to the half-edge of the
	line pointing AWAY from the node.  In other words, if a line
	X has a node Y as its start, node Y should have a directed line
	X with the true flag, since iet is X's left halfedge that goes
	away from Y.

	Next and previous pointers on a line refer to traversal around
	the polygon on the left and right sides, so these references
	are always to half-edges as expected.

	A polygon's boundaries refer to the halfedge closer to the polygon.
	So if polygon P refers to halfedge X with a true flag, edge X should
	ahve polygon P as its left polygon.

 */

#include <vector>
#include "CompGeomDefs2.h"
#include "MapDefs.h"

extern	int gWTPMErrors;

struct	WTPM_Node;
struct	WTPM_Line;
struct	WTPM_Face;

typedef	WTPM_Node *		WTPM_NodePtr;
typedef	WTPM_Line *		WTPM_LinePtr;
typedef	WTPM_Face *		WTPM_FacePtr;
typedef	pair<WTPM_LinePtr, bool>	WTPM_DirectedLinePtr;	// True means left halfedge

typedef	vector<WTPM_LinePtr>			WTPM_LineVector;
typedef	vector<WTPM_DirectedLinePtr>	WTPM_DirectedLineVector;

struct	Vertex;

struct	WTPM_Node {
//  You must provide
	Point2					location;
// Calculated by WTPMCreateBackLinks, sorted by WTPM_RestoreTopology
	WTPM_DirectedLineVector	lines;
// Added by WTPM_ExportToMap
	GISVertex *				pm_vertex;

};

struct	WTPM_Line {
// You must provide
	WTPM_Face *				leftFace;
	WTPM_Face *				rightFace;
	WTPM_Node *				startNode;
	WTPM_Node *				endNode;
	vector<Point2>			shape;
// Calculated by WTPM_RestoreTopology
	WTPM_DirectedLinePtr	nextLeft;
	WTPM_DirectedLinePtr	nextRight;
//	WTPM_DirectedLinePtr	prevLeft;
//	WTPM_DirectedLinePtr	prevRight;
// Added by WTPM_ExportToMap
	typedef vector<GISHalfedge *>					HalfedgeVector;
	typedef	pair<HalfedgeVector, HalfedgeVector>	EdgePair;
	EdgePair				pm_edges;
};

struct WTPM_Face {
// You must provide
	bool					isWorld;
// Calculated by WTPM_CreateBackLinks
	WTPM_LineVector			edgesSame;		// Have us as left poly
	WTPM_LineVector			edgesOpposite;	// Have us as right poly
// Calculated by WTPM_RestoreTopology
	WTPM_LinePtr			outerRing;
	WTPM_LineVector			innerRings;
// Added by WTPM_ExportToMap
	GISFace *				pm_face;
};

typedef	vector<WTPM_Node *>		WTPM_NodeVector;
typedef	vector<WTPM_Line *>		WTPM_LineVector;
typedef	vector<WTPM_Face *>		WTPM_FaceVector;

void	WTPM_CreateBackLinks(
					const WTPM_LineVector& 	inLines);

void	WTPM_RestoreTopology(
					const WTPM_NodeVector&	inNodes,
					const WTPM_LineVector&	inLines,
					const WTPM_FaceVector&	inFaces);

void	WTPM_ExportToMap(
					const WTPM_NodeVector&	inNodes,
					const WTPM_LineVector&	inLines,
					const WTPM_FaceVector&	inFaces,
					Pmwx& 					pmwx);

#endif