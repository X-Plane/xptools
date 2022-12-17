/*
 * Copyright (c) 2009, Laminar Research.
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

#ifndef WED_PreviewLayer_H
#define WED_PreviewLayer_H

#include "WED_MapLayer.h"

struct	XObj8;
struct	agp_t;
class	ITexMgr;

// We need int values for layer groups - these weird numbers actually came out of X-Plane's internal engine...who knew.
// The important thing is that the spacing is enough to ensure separation even when we have lots of runways or taxiways.
enum {
	group_Terrain        = 1,        // no negative terrain offsets, positive up to + 9
	group_Beaches        = 20,       // good for +/- 9 offset

	group_AirportsBegin	 = 40,       // only negative offsets added to this

	group_UnpavedTaxiwaysBegin = 50,
	group_UnpavedTaxiwaysEnd = 1050,

	group_UnpavedRunwaysBegin =1070,
	group_UnpavedRunwaysEnd = 1090,

	group_ShouldersBegin = 1110,
	group_ShouldersEnd   = 1130,
	
	group_TaxiwaysBegin  = 1150,      // max 1000 taxiways PLUS +/- 9 offset
	group_TaxiwaysEnd    = 2150,
	
	group_RunwaysBegin   = 2170,     // max 20 runways PLUS +/- 9 offset
	group_RunwaysEnd     = 2190,

	group_Markings       = 2200,     // good for +/- 9 offset

	group_AirportsEnd    = 2210,     // only positive offsets added to this

	group_Footprints	 = 2230,
	group_Roads          = 2250,
	group_Objects        = 2270,
	group_LightObjects   = 2290,
};

// To draw the preview in X-Plane draw order, we build a draw-obj functor for each item, sort them, then draw.
// We'll subclass this for each kind of preview we make.
struct	WED_PreviewItem {
	int			layer;
	WED_PreviewItem(int l) : layer(l) { }
	virtual ~WED_PreviewItem() { }
	virtual	int	 get_layer(void) { return layer; }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float pavement_alpha)=0;
};


class WED_PreviewLayer  : public WED_MapLayer {
public:

struct Options {
			int		minLineThicknessPixels = MIN_PIXELS_PREVIEW;
	};
						 WED_PreviewLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_PreviewLayer();

			void		SetPavementTransparency(float alpha);
			float		GetPavementTransparency(void) const;
			void		SetObjDensity(int density);
			int			GetObjDensity(void) const;
			void		SetOptions(const Options& options);

	virtual	bool		DrawEntityVisualization		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected);
	virtual	void		GetCaps						(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks);
	virtual	void		DrawVisualization			(bool inCurent, GUI_GraphState * g);

private:

	float							mPavementAlpha;
	int								mObjDensity;

	// This stuff is built temporarily between the entity and final draw.
	vector<WED_PreviewItem *>	mPreviewItems;
	int							mRunwayLayer;		// Keep adding 1 to layer as we find runways, etc.  This means the runway's layer order
	int							mTaxiLayer;			// IS the hierarchy/export order, which is good.
	int							mShoulderLayer;
	Options						mOptions;

};

void draw_obj_at_xyz(ITexMgr * tman, const XObj8 * o, double x, double y, double z, float heading, GUI_GraphState * g);
void draw_agp_at_xyz(ITexMgr * tman, const agp_t * agp, double x, double y, double z, float height, float heading, GUI_GraphState * g, int tile_idx = 0);
int layer_group_for_string(const char * s, int o, int def);

#endif /* WED_PreviewLayer_H */
