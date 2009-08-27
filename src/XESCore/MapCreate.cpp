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

#include "MapCreate.h"
#include "AssertUtils.h"

void	Map_CreateWithLineData(
					Pmwx&									out_map,
					const vector<Segment_2>&				input_curves,
					const vector<GIS_halfedge_data>&		input_data)
{
	DebugAssert(input_curves.size() == input_data.size());

	out_map.clear();

	int n;

	vector<Curve_2>	curves;
	curves.resize(input_curves.size());

	for(n = 0; n < input_curves.size(); ++n)
		curves[n] = Curve_2(input_curves[n], n);

	CGAL::insert_curves(out_map, curves.begin(), curves.end());

	for(Pmwx::Edge_iterator eit = out_map.edges_begin(); eit != out_map.edges_end(); ++eit)
	{
		DebugAssert(eit->curve().data().size() >= 1);

		// CGAL maintains a lot of information for us that makes life easy:
		// 1.	The underlying curve of an edge is a sub-curve of the input curve - it is NEVER flipped.  is_directed_right tells whether
		//		it is lex-right.*
		// 2.	Each half-edge's direction tells us if the half-edge is lex-right...strangely, "SMALLER" means lex-right.
		// Putting these two things together, we can easily detect which of two half-edges is in the same vs. opposite direction of the input
		// curve.
		// * lex-right means lexicographically x-y larger...means target is to the right of source UNLESS it's vertical (then UP = true, down = false).
		Halfedge_handle he = he_is_same_direction(eit) ? eit : eit->twin();

		int cid = eit->curve().data().front();
		DebugAssert(cid >= 0 && cid < input_data.size());
		he->set_data(input_data[cid]);
//		he->data().mDominant = true;
//		he->twin()->data().mDominant = false;

		// Do NOT leave the keys in the map...
		eit->curve().data().clear();
	}
}

void	Map_CreateReturnEdges(
					Pmwx&									out_map,
					const vector<Segment_2>&				input_curves,
					vector<vector<Pmwx::Halfedge_handle> >&	halfedge_handles)
{
	out_map.clear();
	halfedge_handles.resize(input_curves.size());

	int n;

	vector<Curve_2>	curves;
	curves.resize(input_curves.size());

	for(n = 0; n < input_curves.size(); ++n)
		curves[n] = Curve_2(input_curves[n], n);

	CGAL::insert_curves(out_map, curves.begin(), curves.end());

	for(Pmwx::Edge_iterator eit = out_map.edges_begin(); eit != out_map.edges_end(); ++eit)
	{
		DebugAssert(eit->curve().data().size() >= 1);

		// We are going to go through each key and find the matching curve.  Note that if we had two overlapping curves in
		// opposite directions in the input data, we will pick the two TWIN half-edges, since we go with the direction
		// of the source curve.  This means that input data based on a wide set of source polygons that overlap will
		// create correct rings.
		for(EdgeKey_iterator k = eit->curve().data().begin(); k != eit->curve().data().end(); ++k)
		{
			
			Halfedge_handle he = he_get_same_direction(eit);
			halfedge_handles[*k].push_back(he);
		}

		// Do NOT leave the keys in the map...
		eit->curve().data().clear();
	}
}
