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
	vector<double>	tile;	// the base tile in x,y,s,t quads.
	vector<obj>		objs;
};

/**
 * @return True if we succeeded, false if we failed and the output AGP should be considered unusable
 */
bool load_agp(const string &disk_path, agp_t &out_info);

#endif

#endif /* Agp_H */
