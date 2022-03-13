/*
 * Copyright (c) 2021, Laminar Research.
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

#ifndef WED_TerrainLayer_H
#define WED_TerrainLayer_H

#include "WED_MapLayer.h"
#include "CompGeomDefs2.h"
#include "CompGeomDefs3.h"

struct terrain_t {
	int		index;
	Bbox2	bounds;
	int		width;
	int		height;
	vector<float> dem;
	int		water_idx;
	vector<int> apt_idx;
	int		current_color;

	struct vert_data_t {
		Point2	LonLat;
		float	height;
		float	para1;
		float	para2;
		vert_data_t(double *p) : LonLat({p[0], p[1]}), height(p[2]), para1(p[5]), para2(p[6]) {};
	};

	struct patch_t {
		int topology;
		int color;
		Bbox2 bounds;
		vector<vert_data_t> verts;
	};
	vector<patch_t> patches;
};

enum {
	color_water,
	color_airport,
	color_land
};

class WED_TerrainLayer : public WED_MapLayer {
public:

						 WED_TerrainLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_TerrainLayer();

	virtual	void		DrawVisualization		(bool inCurrent, GUI_GraphState * g);
	virtual	void		GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks);

private:

	void				LoadTerrain(Bbox2& bounds);
	unordered_map<string,terrain_t>	mTerrains;
};

#endif /* WED_TerrainLayer_H */
