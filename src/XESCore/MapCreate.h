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

#ifndef MapCreate_H
#define MapCreate_H

#include "MapDefs.h"

// Given a set of lines, if we know the data, this creates a new map that contains those edges, and tags the edges with the data.
// By convention, the GIS data is attached to the halfedge that goes in the direction of the input curves.  If two input curves
// go in opposite directions, the results are indeterminate.  If two input curves overlap, which data is used is indeterminate.
void	Map_CreateWithLineData(
					Pmwx&									out_map,
					const vector<Segment_2>&				input_curves,
					const vector<GIS_halfedge_data>&		input_data);

// Given a set of lines, this creates a new map.  For each entry in the input curves, we get back a vector of half-edge handles that
// represents the induced curves.  The returned vectors of half-edges _are_ in the same direction as the underlying curve, but are NOT
// necessarily in order.
//
// (Thus the union of all of the induced half edges for all of the original edges of a polygon should, in theory, fully surround
// the set of faces induced by the polygon.)
//
// Since the data set may contain overlaps, it is possible that halfedeges will be in more than one source curve vector.
void	Map_CreateReturnEdges(
					Pmwx&									out_map,
					const vector<Segment_2>&				input_curves,
					vector<vector<Pmwx::Halfedge_handle> >&	halfedge_handles);

#endif /* MapCreate_H */
