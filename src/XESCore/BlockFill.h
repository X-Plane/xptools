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

struct CoordTranslator2;

bool	init_block(
					CDT&					mesh,
					Pmwx::Face_handle		face,
					Block_2&				out_block,
					CoordTranslator2&		translator);		// returns true if block is not insanely small!

bool	apply_fill_rules(
					int						zoning,
					Block_2&				block,
					CoordTranslator2&		translator,
					DEMGeo&					forest_dem);

void	extract_features(
					Block_2&				block,
					Pmwx::Face_handle		dest_face,
					CoordTranslator2&		translator);

bool	process_block(
					Pmwx::Face_handle		f, 
					CDT&					mesh,
					DEMGeo&					forest_dem);

#endif /* BlockFill_H */
