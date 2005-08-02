THIS FILE IS OBSOLETE

/* 
 * Copyright (c) 2004, Laminar Research.
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
#ifndef XPROTODEFS_H
#define XPROTODEFS_H

#include <vector>
#include "XObjDefs.h"

struct	ProtoSeg_t {
	XObj	obj;
	
//	bool	can_stretch;		// Stretch this instead of repeating it
//	bool	stretch_tex_too;	// Adjust S when stretching
//	bool	can_chop_left;		// Chop this to make it work
//	bool	can_chop_right;		// Chop this to make it work

	int		repeats;			// Repeats N times each time we need a new layer	
	string	name;
};
typedef	vector<ProtoSeg_t>	SegVector_t;

struct	ProtoWall_t {
	SegVector_t		segments;
//	float			min_length;	// Criteria when this is applicable, or -1 for none
//	float			max_length;	//
//	float			probability;// Some way to apply walls
};
typedef	vector<ProtoWall_t>	WallVector_t;

enum {
	layer_Facade,
	layer_Roof,
	layer_FinalRoof
};

struct	ProtoLayer_t {
	int				layer_type;
	
	float			top_inset;
	float			bottom_inset;
	
	WallVector_t	walls;
	float			height;
	int				repeats;
	
	// We need more for walled roofs and S&T
};

typedef	vector<ProtoLayer_t>	LayerVector_t;

struct	Prototype_t {
	bool			closed;
	LayerVector_t	layers;
};	
	
#endif
