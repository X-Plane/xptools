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
#include "WTPM.h"
#include "CompGeomUtils.h"
#include "AssertUtils.h"
#include "MapDefs.h"

int	gWTPMErrors = 0;

Vector2					WTPM_GetSpurVector(WTPM_DirectedLinePtr line);
void					WTPM_SortLinesAroundPoints(const WTPM_NodeVector& inNodes);
void					WTPM_FindLineConnections(const WTPM_LineVector& inLines);
WTPM_DirectedLinePtr	WTPM_FindLeftMostEdgeInSet(const set<WTPM_DirectedLinePtr>& inEdges);
void					WTPM_RemoveRingFromSet(const WTPM_DirectedLinePtr& anEdge, set<WTPM_DirectedLinePtr>& inEdgeSet);
WTPM_DirectedLinePtr	WTPM_FindNextLine(const WTPM_DirectedLinePtr& inLine);
void					WTPM_FindPolygonRings(const WTPM_FaceVector& inFaces);

// Given a half-edge this routine returns a vector pointing away from the source of the half-edge.
// (If line.second is true, we're the left halfedge, go from shape pt 0 to 1, other wise go from
// shape pt n-1 to n-2.
Vector2	WTPM_GetSpurVector(WTPM_DirectedLinePtr line)
{
	if (line.second)
		return Vector2(line.first->shape[0], line.first->shape[1]);
	else
		return Vector2(line.first->shape[line.first->shape.size() - 1], line.first->shape[line.first->shape.size() - 2]);
}

// BAS NOTE ON IMPLEMENTATION:
// Given files that are topologically sound (like the TIGER/Line files) but not spatially distinct,
// we _could_ also utilize the left and right face information to resolve the sorting of overlapping
// vectors.  However we would still have to use angle when we have antennas into a face that share
// a point.   This comes down to whether we want to solve this problem spatially or topologically.
// While the TIGER documents recommend a topological approach, we continue to use a spatial one for
// two reasons:
// 1. It will allow us to use this code on data that has less topological information, and
// 2. If the data cannot be spatially resolved, it is going to drive the rendering software insane
// 	  later anyway, so we might as well discover this on import and not have spatially bogus data.

// Given a set of nodes, this routine sorts the edges coming into the node into clockwise order.
// We do this using a simple insert-sort; it's slow, but the sorting semantics are a little funky...
// we're not really a weak ordered set here.  We could probably define a mathematically correct
// version of Is_CCW_Between that serves as a sorting function starting at 0 and going backward,
// but it doesn't really matter...the valence of vertices in a TPM s generally low.
void	WTPM_SortLinesAroundPoints(const WTPM_NodeVector& inNodes)
{
	int spur, pos, i;

	for (WTPM_NodeVector::const_iterator node = inNodes.begin(); node != inNodes.end(); ++node)
	{
		WTPM_Node * us = *node;
		if (us->lines.size() > 2)
		{
			// Build a table...the first item is a vector pointing away from our node, the second
			// is the foreign key of that line.
			vector<pair<Vector2, WTPM_DirectedLinePtr> >		workingVecs, putVecs;
			for (spur = 0; spur < us->lines.size(); ++spur)
				workingVecs.push_back(pair<Vector2, WTPM_DirectedLinePtr>(WTPM_GetSpurVector(us->lines[spur]), us->lines[spur]));

			// For each spur we are goigg to try to find out where it belongs by iterating
			// through the already placed vectors and seeing if we go in between.
			for (spur = 0; spur < workingVecs.size(); ++spur)
			{
				bool	found = false;
				for (pos = 1; pos < putVecs.size(); ++pos)
				{
					if (Is_CCW_Between(putVecs[pos].first, workingVecs[spur].first, putVecs[pos-1].first))
					{
						found = true;
						putVecs.insert(putVecs.begin()+pos, workingVecs[spur]);
						break;
					}
				}
				if (!found)
					putVecs.push_back(workingVecs[spur]);
			}

			// Finally copy the whole mess back into the node.
			us->lines.clear();
			for (i = 0; i < putVecs.size(); ++i)
			{
				us->lines.push_back(putVecs[i].second);
			}
		}
	}
}

// This routine finds the next node in a CCB for a half-edge based on sorted nodes.
void	WTPM_FindLineConnections(const WTPM_LineVector& inLines)
{
	WTPM_DirectedLineVector::iterator	meSpur;
	WTPM_LinePtr						us;

	for (WTPM_LineVector::const_iterator line = inLines.begin(); line != inLines.end(); ++line)
	{
		us = *line;
		WTPM_DirectedLinePtr	usLeft(us, true);
		WTPM_DirectedLinePtr	usRight(us, false);

		// Find us in its rotation list.  Go forward one clockwise, that's who we should point to backward.
		meSpur = find(us->startNode->lines.begin(), us->startNode->lines.end(), usLeft);
#if DEV
		if (meSpur == us->startNode->lines.end())
			printf("ASSERTION ERROR: cannot find spur in start node list.\n");
#endif
		++meSpur;
		if (meSpur == us->startNode->lines.end())
			meSpur = us->startNode->lines.begin();
		us->nextRight = *meSpur;

//		if (us->nextRight.second)
//			us->nextRight.first->prevLeft = usRight;
//		else
//			us->nextRight.first->prevRight = usRight;

		// Find us in the rotation list.  Go forward one clcokwise, that's who we should point to forward.
		meSpur = find(us->endNode->lines.begin(), us->endNode->lines.end(), usRight);
#if DEV
		if (meSpur == us->endNode->lines.end())
			printf("ASSERTION ERROR: cannot find spur in end node list.\n");
#endif
		++meSpur;
		if (meSpur == us->endNode->lines.end())
			meSpur = us->endNode->lines.begin();
		us->nextLeft = *meSpur;

//		if (us->nextLeft.second)
//			us->nextLeft.first->prevLeft = usLeft;
//		else
//			us->nextLeft.first->prevRight = usLeft;
	}
}

// Find the left-most edge in a set.  We do this by simply comparing X coordinates of all shape points
// in each edge in the set.  We know that this edge is on the outer CCB of at least one polygon.
WTPM_DirectedLinePtr			WTPM_FindLeftMostEdgeInSet(const set<WTPM_DirectedLinePtr>& inEdges)
{
	WTPM_DirectedLinePtr bestKey;
	bool	has = false;
	double	bestX;
	for (set<WTPM_DirectedLinePtr>::const_iterator iter = inEdges.begin(); iter != inEdges.end(); ++iter)
	{
		for (int i = 0; i < iter->first->shape.size(); ++i)
		{
			if (!has || iter->first->shape[i].x() < bestX)
			{	
				has = true;
				bestKey = *iter;
				bestX = iter->first->shape[i].x();
			}
		}
	}
	return bestKey;
}


// Remove anEdge and every other edge in its ring from inEdgeSet.  thePolygon defines
// which side of the edge we want.  This effectively moves one ring from the polygon from the set.
void		WTPM_RemoveRingFromSet(const WTPM_DirectedLinePtr& anEdge, set<WTPM_DirectedLinePtr>& inEdgeSet)
{
	set<WTPM_DirectedLinePtr>	origSet(inEdgeSet);
	WTPM_DirectedLinePtr	iter = anEdge, stop = anEdge;
	do {
		if (inEdgeSet.find(iter) == inEdgeSet.end())
		{
			gWTPMErrors++;
#if DEV
			printf("WARNING: problem constructing topological ring.\n");
			if (origSet.find(iter) == origSet.end()) printf("We never had this edge.\n"); else printf("We deleted this edge.\n");
			printf("Originally had %d edges, now we have %d\n", origSet.size(), inEdgeSet.size());
#endif
		}
		inEdgeSet.erase(iter);
		iter = WTPM_FindNextLine(iter);
	} while (iter != stop);
}

// Find our halfedge's next edge.  A convenience routine to deal with our directed edge logic.
WTPM_DirectedLinePtr		WTPM_FindNextLine(const WTPM_DirectedLinePtr& inLine)
{
	if (inLine.second)
		return inLine.first->nextLeft;
	else
		return inLine.first->nextRight;
}

// Given the lines all connected, this routine figures out which lines represent
// rings.
void	WTPM_FindPolygonRings(const WTPM_FaceVector& inFaces)
{
	WTPM_FacePtr			us;
	WTPM_DirectedLinePtr	leftMost;
	for (WTPM_FaceVector::const_iterator face = inFaces.begin(); face != inFaces.end(); ++face)
	{
		us = *face;

		set<WTPM_DirectedLinePtr>		unusedEdges;

		for (WTPM_LineVector::iterator i = us->edgesSame.begin(); i != us->edgesSame.end(); ++i)
			unusedEdges.insert(WTPM_DirectedLinePtr(*i, true));
		for (WTPM_LineVector::iterator i = us->edgesOpposite.begin(); i != us->edgesOpposite.end(); ++i)
			unusedEdges.insert(WTPM_DirectedLinePtr(*i, false));

		leftMost = WTPM_FindLeftMostEdgeInSet(unusedEdges);
		us->outerRing = leftMost.first;
		WTPM_RemoveRingFromSet(leftMost, unusedEdges);
		while (!unusedEdges.empty())
		{
			leftMost = WTPM_FindLeftMostEdgeInSet(unusedEdges);
			us->innerRings.push_back(leftMost.first);
			WTPM_RemoveRingFromSet(leftMost, unusedEdges);
		}
	}
}

void	WTPM_CreateBackLinks(
					const WTPM_LineVector& 	inLines)
{
	for (WTPM_LineVector::const_iterator line = inLines.begin(); line != inLines.end(); ++line)
	{
		WTPM_Line * us = *line;

		us->startNode->lines.push_back(WTPM_DirectedLinePtr(us, true));
		us->endNode->lines.push_back(WTPM_DirectedLinePtr(us, false));

		us->leftFace->edgesSame.push_back(us);
		us->rightFace->edgesOpposite.push_back(us);
	}
}

void	WTPM_RestoreTopology(
					const WTPM_NodeVector&	inNodes,
					const WTPM_LineVector&	inLines,
					const WTPM_FaceVector&	inFaces)
{
	gWTPMErrors = 0;

	// First we sort all of the lines around our vertices in a clockwise manner
	// based on their headings.
	WTPM_SortLinesAroundPoints(inNodes);

	// Now we can find our polygon sides...every halfedge's next side is
	// clockwise of it around a vertex.
	WTPM_FindLineConnections(inLines);

	// Once edges are linked into CCBs we can identify rings...the leftmost ring of
	// a polygon is always an outer ring; all others are inner rings.
	WTPM_FindPolygonRings(inFaces);
}

void	WTPM_ExportToMap(
					const WTPM_NodeVector&	inNodes,
					const WTPM_LineVector&	inLines,
					const WTPM_FaceVector&	inFaces,
					Pmwx& 					pmwx)
{
	pmwx.clear();
	Arr_accessor	dcel(pmwx);
	
	DVertex *			new_v;
	int					n, hole_counter = 0;
	DHalfedge *			myHalfedge;
	WTPM_NodeVector::const_iterator node;
	WTPM_LineVector::const_iterator line;
	WTPM_FaceVector::const_iterator face;
	WTPM_LineVector::const_iterator hole;
	WTPM_Line::HalfedgeVector::iterator e;

	// STEP 1 - CREATE VERTICES FOR EVERY NODE

	for (node = inNodes.begin(); node != inNodes.end(); ++node)
	{
		new_v = dcel.new_vertex(ben2cgal((*node)->location));
		if ((*node)->location == Point2())
			printf ("Null pt.\n");
		(*node)->pm_vertex = new_v;
	}

	// STEP 2 - CREATE HALFEDGES FOR EVERY LINE AND VERTICES FOR EVERY SHAPE POINT
	// AND CREATE LINKING WITHIN ALL LINES

	for (line = inLines.begin(); line != inLines.end(); ++line)
	{
		// First for each chain, we are going to create vertices for
		// every shape point.  The vector vertices will have these vertices.
		// The start and end node must be checked against the node map by
		// foreign key ID so we share them with other lines.  Shape points
		// are never shared, so we just blast through them.

			vector<DVertex *>	vertices;
	
		vertices.push_back((*line)->startNode->pm_vertex);	

		for (n = 1; n < ((*line)->shape.size() - 1); ++n)
		{
			new_v = dcel.new_vertex(ben2cgal((*line)->shape[n]));
		if ((*line)->shape[n] == Point2())
			printf ("Null pt.\n");
			vertices.push_back(new_v);
		}

		vertices.push_back((*line)->endNode->pm_vertex);

		// Now we build half-edges for each segment in the chain.
		// The first side goes in the oreintation of the line, the
		// second side goes against.  Both vectors are ordered from
		// start to stop though!!!

		for (n = 1; n < vertices.size(); ++n)
		{
			if (vertices[n-1]->point() == vertices[n]->point())
			{
				fprintf(stderr,"WARNING: ZERO LENGTH SHAPED SEGMENT!\n");
#if DEV
				for (int d = 0; d < (*line)->shape.size(); ++d)
				{
					printf("   pt: %lf,%lf \n", (*line)->shape[d].x(),(*line)->shape[d].y());
				}
#endif
			}
//			PM_Curve_2	seg1(vertices[n-1]->point(), vertices[n]->point());
//			PM_Curve_2	seg2(vertices[n]->point(), vertices[n-1]->point());
			Curve_2		curve(Segment_2(vertices[n-1]->point(), vertices[n]->point()));
			DHalfedge * e1, *e2;
			e1 = dcel.new_edge(curve);
			e2 = e1->opposite();
			e1->set_opposite(e2);
			e2->set_opposite(e1);
			e1->set_vertex(vertices[n]);
			e2->set_vertex(vertices[n-1]);
			vertices[n]->set_halfedge(e1);
			vertices[n-1]->set_halfedge(e2);
//			e1->set_curve(seg1);
//			e2->set_curve(seg2);
			(*line)->pm_edges.first.push_back(e1);
			(*line)->pm_edges.second.push_back(e2);
		}

		// Now link the edges to each other in a row.

		for (n = 1; n < (*line)->pm_edges.first.size(); ++n)
		{
			(*line)->pm_edges.first[n-1]->set_next((*line)->pm_edges.first[n]);
			(*line)->pm_edges.second[n]->set_next((*line)->pm_edges.second[n-1]);
		}
	}

	// STEP 3 - INTERLINK LINES TO FORM CCW BOUNDARIES

	for (line = inLines.begin(); line != inLines.end(); ++line)
	{
		WTPM_Line::EdgePair&	myParts = (*line)->pm_edges;
		WTPM_Line::EdgePair&	nextLeftParts = (*line)->nextLeft.first->pm_edges;
		WTPM_Line::EdgePair&	nextRightParts = (*line)->nextRight.first->pm_edges;

		if ((*line)->nextLeft.second)
			myParts.first.back()->set_next(nextLeftParts.first.front());
		else
			myParts.first.back()->set_next(nextLeftParts.second.back());

		if ((*line)->nextRight.second)
			myParts.second.front()->set_next(nextRightParts.first.front());
		else
			myParts.second.front()->set_next(nextRightParts.second.back());
	}

	// STEP 4 - CREATE POLYGONS AND LINK THEM TO EDGES

	for (face = inFaces.begin(); face != inFaces.end(); ++face)
	{
		// First create the face (or use the world face) and index it.
		DFace * new_face = &*pmwx.unbounded_face();
		if (!(*face)->isWorld)
			new_face = dcel.new_face();			
		(*face)->pm_face = new_face;

		// For non-world polygons we need to set our outer CCB.
		// This requires checking the edge orientation; if we are
		// on the left side of the Ždge then the halfedge with the
		// edge is on our CCB, otherwise it's the other one.

		// For the world polygon, due to the weirdness of our
		// code, we should have our outer ring actually be a hole.
		// (well, except that maybe we are an emtpy map.

		if ((*face)->isWorld && (*face)->outerRing == NULL)
		{
			DebugAssert((*face)->innerRings.empty());
			continue;
		}
		DebugAssert((*face)->outerRing != NULL);

		if ((*face)->outerRing->leftFace == (*face))
			myHalfedge = (*face)->outerRing->pm_edges.first.front();
		else
			myHalfedge = (*face)->outerRing->pm_edges.second.front();

		if ((*face)->isWorld)
			new_face->add_hole(myHalfedge);
		else
			new_face->set_halfedge(myHalfedge);

		// Now go through each hole and do the same test.
		for (hole = (*face)->innerRings.begin();
			hole != (*face)->innerRings.end(); ++hole)
		{
			if ((*hole)->leftFace == (*face))
				myHalfedge = (*hole)->pm_edges.first.front();
			else
				myHalfedge = (*hole)->pm_edges.second.front();
			new_face->add_hole(myHalfedge);

			if (!(*face)->isWorld)
				++hole_counter;
		}
	}

	// STEP 5 - LINK EDGES TO THEIR FACES

	for (line = inLines.begin(); line != inLines.end(); ++line)
	{
		DFace* face = (*line)->leftFace->pm_face;
		
		for (e = (*line)->pm_edges.first.begin(); e != (*line)->pm_edges.first.end(); ++e)
			(*e)->set_face(face);

		face = (*line)->rightFace->pm_face;

		for (e = (*line)->pm_edges.second.begin(); e != (*line)->pm_edges.second.end(); ++e)
			(*e)->set_face(face);
	}
}
