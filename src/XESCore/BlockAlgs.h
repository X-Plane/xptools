/* 
 * Copyright (c) 2010, Laminar Research.
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

#ifndef BlockAlgs_H
#define BlockAlgs_H

#include "BlockDefs.h"

struct CoordTranslator2;

/*
struct block_create_t {
	BLOCK_face_data		data;
	Polygon_2			bounds;
};
*/

// Create a block from a set of polygons.  
// When two polygons overlap, the LATER polygon in the vector takes priority.
// If we want to specify that the area OUTSIDE the block (unbounded) has some
// property, we can pass the index of one polygon whose _outside_ is tagged.  Pass
// -1 for the index if we don't want this.
void	create_block(
					Block_2&									block,
					const vector<BLOCK_face_data>&				in_data,
					const vector<Block_2::X_monotone_curve_2>&	in_bounds,
					int											unbounded_idx);

//bool	can_insert_into_block(
//					Block_2&						block,
//					const Polygon_2&				bounds);
//					
//void	do_insert_into_block(
//					Block_2&						block,
//					const Polygon_2&				bounds,
//					const BLOCK_face_data&			data);

void clean_block(Block_2& block);

void simplify_block(Block_2& io_block, double max_err);

void find_major_axis(vector<block_pt>&	pts,
				Segment2 *			out_segment,
				Vector2 *			out_major,
				Vector2 *			out_minor,
				double				bounds[4]);

// Given a CCB halfedge circulator, this routine attempts to build a convex polygon that approximates it.  It has a number of
// built in fail-safes:
// - It won't merge sides that don't have the same AG spawning or width properties.
// - It won't merge sides beyond an error metric.
// It fails if it has fewer than 3 sides or the resulting shape isn't convex.  It also fails if the inset shape is not convex,
// or if any side is shorter than "min side len".
bool	build_convex_polygon(
				Pmwx::Ccb_halfedge_circulator									ccb,				// Outer boundary of face to translate.
				vector<pair<Pmwx::Halfedge_handle, Pmwx::Halfedge_handle> >&	sides,				// Per side: inclusive range of half-edges "consolidated" into the sides.
				const CoordTranslator2&											trans,				// Coord tranlator that gets us metric.
				Polygon2&														metric_bounds,		// Inset boundary in metric, first side matched to the list.
				double															max_err_mtrs,		// Limit :max err in meters in simplifying sides.
				double															min_side_len);		// Limit: shotest side length in polygon.

int	pick_major_axis(
				vector<pair<Pmwx::Halfedge_handle, Pmwx::Halfedge_handle> >&	sides,				// Per side: inclusive range of half-edges "consolidated" into the sides.
				Polygon2&														metric_bounds,		// Inset boundary in metric, first side matched to the list.
				Vector2&														v_x,
				Vector2&														v_y);
				

#endif /* BlockAlgs_H */
