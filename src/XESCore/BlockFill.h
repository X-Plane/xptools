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

#ifndef BlockFill_H
#define BlockFill_H

#include "BlockDefs.h"
#include "MeshDefs.h"
#include "RTree2.h"
#include "MapDefs.h"

struct CoordTranslator2;

typedef RTree2<Pmwx::Face_handle, 4>		ForestIndex;

bool	init_block(
					CDT&					mesh,
					Pmwx::Face_handle		face,
					Block_2&				out_block,
					CoordTranslator2&		translator,
					int *					io_agb_fail);			// If not null, tells us if an attempt to apply an AGB rule with no facade fallback failed due to not-straight geometry.
					// returns true if block is not insanely small!

bool	apply_fill_rules(
					int						zoning,
					Pmwx::Face_handle		orig_face,
					Block_2&				block,
					CoordTranslator2&		translator,
					int						agb_did_fail);			// If true, our AGB rule didn't work, so pretend it doesn't exist.

void	extract_features(
					Block_2&				block,
					Pmwx::Face_handle		dest_face,
					CoordTranslator2&		translator,
					const DEMGeo&			forest_dem,
					ForestIndex&			forest_index);

bool	process_block(
					Pmwx::Face_handle		f, 
					CDT&					mesh,
					const DEMGeo&			ag_ok_approx_dem,
					const DEMGeo&			forest_dem,
					ForestIndex&			forest_index);




bool block_pts_from_ccb(
			Pmwx::Ccb_halfedge_circulator	he, 
			CoordTranslator2&				translator, 
			vector<block_pt>&				pts,
			double							dp_err_mtr,
			bool							is_hole);

float WidthForSegment(const pair<int,bool>& seg_type);


extern int num_block_processed;
extern int num_blocks_with_split;
extern int num_forest_split;
extern int num_line_integ;
#endif /* BlockFill_H */
