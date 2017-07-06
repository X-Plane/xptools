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
	
	wall_map_t() : vert(), hori(), scale_x(1.0f) ,scale_y(1.0f), basem(0.0f) { }

	float		vert[4];   // for now planning to only collect ONE example for wall type.
	float		hori[4];
	float		scale_x, scale_y;
	float		basem;
};

bool WED_MakeFacadePreview(fac_info_t& info, vector<wall_map_t> wall, string wall_tex, float tex_size[2],
	string roof_tex,  float roof_scale[2]);

#endif