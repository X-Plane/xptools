/* 
 * Copyright (c) 2008, Laminar Research.
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

#ifndef MapTopology_H
#define MapTopology_H

#include "MapDefs.h"
#include "ProgressUtils.h"

/************************************************************************************************
 * FACE SETS AND EDGE SETS
 ************************************************************************************************
 * 
 * It is often useful to treat a group of faces or a group of edges as a unit...two examples:
 *
 * - A body of water might be made up of multiple faces becaue bridges split the body of water.
 *   So we use a face set to represent the entire body of water.
 *
 * - When merging or copying from one map to another, a single face or single edge in one map
 *   may be represented by many edges or faces in the new one!
 *
 * Generally when we have an edge set we mean a set of halfedges that form one or more rings.
 * (But the edge set is not ordered!)  When we have a face set, the area of the faces is not
 * necessarily continuous and may contain holes, islands, or whatever!
 *
 */

/*
 * FindEdgesForFace
 *
 * Given a face, return all halfedges that have the face on its left.
 * THIS DOES NOT CLEAR THE SET, FOR YOUR CONVENIENCE!!
 *
 */
void	FindEdgesForFace(Face_handle face, set<Halfedge_handle>& outEdges);

/*
 * FindFacesForEdgeSet
 *
 * Given a bounded edge set, return all faces within that edge set.
 * THIS DOES CLEAR THE FACE SET!
 *
 */
void	FindFacesForEdgeSet(const set<Halfedge_handle>& inEdges, set<Face_handle>& outFaces);

/*
 * FindEdgesForFaceSet
 *
 * Given a set of faces, finds the halfedges that bound the set of faces.  Halfedges facing the
 * inside of the face set are returned.
 *
 */
void	FindEdgesForFaceSet(const set<Face_handle>& inFaces, set<Halfedge_handle>& outEdges);

/*
 * FindInternalEdgesForFaceSet
 *
 * Given a set of edges that rings an area, return any edges inside.  Edges returned are guaranteed to not
 * be twins, but no guarantee about which edge we get!
 *
 */
void	FindInternalEdgesForEdgeSet(const set<Halfedge_handle>& inEdges, set<Halfedge_handle>& outEdges);

/*
 * FindAdjacentFaces
 *
 * Given a face, returns all faces that are touching this face.
 *
 */
void	FindAdjacentFaces(Face_handle inFace, set<Face_handle>& outFaces); 

/*
 * FindAdjacentWetFaces
 *
 * Given a face, returns all faces that are touching this face.
 *
 */
void	FindAdjacentWetFaces(Face_handle inFace, set<Face_handle>& outFaces); 

/*
 * IsAdjacentWater
 *
 * Return true if adjacent to any water poly.
 *
 */ 
bool		IsAdjacentWater(Face_const_handle in_face, bool unbounded_is_wet);

/*
 * FindConnectedWetFaces
 *
 * Given a water face, return all connected waterways.  This gives you a truly enclosed water
 * body, taking into account things like bridges.
 *
 */
void	FindConnectedWetFaces(Face_handle inFace, set<Face_handle>& outFaces); 

/*
 * CleanFace
 * 
 * Given a face on a map, this routine 'cleans' its interior, removing any
 * antennas, holes, or anything else in its interior.
 *
 * (This is a convenient high level swap.)
 *
 */
void	CleanFace(
			Pmwx&				inMap,
			Face_handle	inFace);

/*
 * RemoveUnboundedWater
 *
 * Similar to the routines above, this routine removes all vectors with water on both sides
 * and no transportation links.  The result is (among other things) a removal of unbounded water
 * from a map.  This can be useful for reducing a map to dry land and embedded lakes.
 *
 * This also removes antennas into the unbounded face.
 *
 * Performance: O(N*M) where N = number of removed halfedges, and M = average number
 * of halfedges in a CCB.
 *
 */
int RemoveUnboundedWater(Pmwx& ioMap);

/*
 * ReduceToWaterBodies
 *
 * This routine removes edges until the map consists only of faces that represent contiguous
 * water bodies, and the infinite face.  Please note that implicit in this is that edges
 * be constructed to form discrete water bodies up to the edge of the map's useful area.
 *
 * Performance: O(N*M) where N = number of removed halfedges, and M = average number
 * of halfedges in a CCB. 
 *
 */
void ReduceToWaterBodies(Pmwx& ioMap);

/*
 * SimplifyMap
 *
 * SimplifyMap removes all edges that do not separate distinct land uses, form the outer
 * CCB, or contain rivers/transportation.  This can include lines from unused data, like
 * geopolitical boundaries, or lines that are artifacts of the import.
 *
 * Performance: O(N*M) where N = number of removed halfedges, and M = average number
 * of halfedges in a CCB.
 *
 */
int SimplifyMap(Pmwx& ioMap, bool inKillRivers, ProgressFunc func);


#endif /* MapTopology_H */
