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

#ifndef Agp_H
#define Agp_H

#include "XObjDefs.h"
#include "CompGeomDefs2.h"

#if AIRPORT_ROUTING
struct agp_t {
	struct obj {
		double  x,y,r;			// annotation position
		int		show_lo,show_hi;
		string	name;
	};
	string			base_tex;
	string			mesh_tex;
	int				hide_tiles;

	// The base tile in x,y,s,t quads.
	// Tyler says: OBVIOUSLY this is a vector of size 16, with values:
	//   Index:   0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
	//   Meaning: x1  y1  s1  t1  x2  y1  s2  t1  x2  y2  s2  t2  x1  y2  s1  t2
	// where (x1, y1) *may be* greater than (x2, y2).
	// This is designed basically for immediate use in WED's renderer.
	// (I can't believe you didn't know that........)
	vector<double>	tile;
	vector<obj>		objs;

	Bbox2 bounds_meters() const;
};

/**
 * @return True if we succeeded, false if we failed and the output AGP should be considered unusable
 */
bool load_agp(const string &disk_path, agp_t &out_info);

#endif

#endif /* Agp_H */
