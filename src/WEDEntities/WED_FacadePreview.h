/*
 * Copyright (c) 2017, Laminar Research.
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

#ifndef WED_FacadePreview_H
#define WED_FacadePreview_H

#ifndef WED_ResourceMgr_H
#include "WED_ResourceMgr.h"
#endif

struct wall_map_t {
	
	wall_map_t() : vert(), hori(), min_w(), max_w(999.0) { }

	float	vert[4];   // for now planning to only collect ONE example for wall type.
	float	hori[4];
	float	min_w, max_w;   // range of widths supported by this wall definition
};

bool WED_MakeFacadePreview(fac_info_t& info, vector<wall_map_t> walls, string wall_tex, float roof_uv[4]);

#endif