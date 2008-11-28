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

#ifndef MapOverlay_H
#define MapOverlay_H

#include "ProgressUtils.h"
#include "MapDefs.h"

// These opeations work on entire maps - they use sweep line, so they are very good for two huge maps.
// But if one map is much smaller than the other, you eat the cost of the big map!!!


// Merges the ocntents of A and B into result.
// Faces are marked as contained if they were bounded faces in B.  Conflicts are resolved in favor of B.
void	MapMerge(Pmwx& src_a, Pmwx& src_b, Pmwx& result);

// Overlays "top" on top of "bottom - all properties are taken from top UNLESS bottom is under an unbounded part of top.
// Faces that were bounded in top ("in") top are set as contained, A is not.
void	MapOverlay(Pmwx& bottom, Pmwx& top, Pmwx& result);









// These ops insert a polygon "manually".  They are not as efficient as sweep, but computational complexity is "local" to the 
// inserted polygon, which is a big win for a tiny polygon in a huge map.


// Merge the polygon into the map.  The polygon must be simple.  If desired, the entire face set within the polygon is returned.
// If locator is not NULL, it must be a valid locator for the dest map.
void			MapMergePolygon(Pmwx& io_dst, const Polygon_2& src, set<Face_handle> * out_faces, Locator * loc);
void			MapMergePolygonWithHoles(Pmwx& io_dst, const Polygon_with_holes_2& src, set<Face_handle> * out_faces, Locator * loc);
void			MapMergePolygonSet(Pmwx& io_dst, const Polygon_set_2& src, set<Face_handle> * out_faces, Locator * loc);

// Overlay the polygon, gutting anything inside it.  Polygon must be simple and non-empty!
// If locator is not NULL, it must be a valid locator for the dest map.
Face_handle		MapOverlayPolygon(Pmwx& io_dst, const Polygon_2& src, Locator * loc);
Face_handle		MapOverlayPolygonWithHoles(Pmwx& io_dst, const Polygon_with_holes_2& src, Locator * loc);
void			MapOverlayPolygonSet(Pmwx& io_dst, const Polygon_set_2& src, Locator * loc, set<Face_handle> * faces);




// These are stubs from the old API - need to be migrated some day.
/*
 * OverlayMap
 *
 * Inserts all part of inSrc into inDst.  inSrc must not have antennas outside of the holes in
 * the unbounded face.  InSrc is left with gutted holes of inDst's remains where land was, 
 * inDst has contents overwritten.
 *
 */
void OverlayMap(
			Pmwx& 	inDst, 
			Pmwx& 	inSrc);

/*
 * MergeMaps
 *
 * This routine copies the contents of ioSrcMap into ioDstMap.
 *
 * Performance: O((N+F)*M) where N is the number of halfedges inserted into the
 * dest map and M is the average number of halfedges in a CCB in the dest map, and
 * F is the number of faces in the source map that have important data that must be copied.
 *
 * For optimal performance, ioDstMap should have faces with small numbers of bounds
 * and ioSrcMap should have fewer or shorter halfedges, and fewer faces that have 
 * data that need to be copied.
 *
 * If inForceProps is true, when there is a property conflict for terrain type or area property
 * on a face, the srcMap will win; otherwise the dstMap will win.  A merge takes place where 
 * there is no conflicts.
 *
 * If outFaces is not NULL, then the handle of every face in the dst map that had a property
 * in the source map is returned.  Two warnings: "empty" faces (terrain natural, no area 
 * feature) are not included, and faces that are non-empty are copied even if the dest-map has
 * a property and inForceProps is false (in which case the face in outFaces did not receive a
 * property from the source.  (A typical use might be to put a bogus mark or terrain on all
 * source faces and thus receive an idea of where in the destination map your source map 
 * ended up, without having to do a bunch of fac-relocates from edge bounds.
 *
 */
void MergeMaps(Pmwx& ioDstMap, Pmwx& ioSrcMap, bool inForceProps, set<Face_handle> * outFaces, bool pre_integrated, ProgressFunc func);



#endif /* MapOverlay_H */

