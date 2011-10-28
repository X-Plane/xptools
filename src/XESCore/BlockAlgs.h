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

#endif /* BlockAlgs_H */
