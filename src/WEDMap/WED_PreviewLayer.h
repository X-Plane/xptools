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

// We need int values for layer groups - these weird numbers actually came out of X-Plane's internal engine...who knew.
// The important thing is that the spacing is enough to ensure separation even when we have lots of runways or taxiways.
enum {
	group_Terrain	= 5,				
	group_Beaches	= 25,				

	group_AirportsBegin	= 60,			
		group_ShouldersBegin = 70,
		group_ShouldersEnd = 90,
		group_TaxiwaysBegin = 100,		
		group_TaxiwaysEnd = 1000,		
		group_RunwaysBegin = 1100,		
		group_RunwaysEnd = 1900,		
		
		group_Markings = 1920,			
		
	group_AirportsEnd = 1930,
	
	group_Footprints	= 1940,			
	group_Roads			= 1950,
	group_Objects		= 1960,
	group_LightObjects	= 1965,
};

// To draw the preview in X-Plane draw order, we build a draw-obj functor for each item, sort them, then draw.
// We'll subclass this for each kind of preview we make.
struct	WED_PreviewItem {
	int			layer;
	WED_PreviewItem(int l) : layer(l) { }
	virtual	int	 get_layer(void) { return layer; }
	virtual void draw_it(WED_MapZoomerNew * zoomer, GUI_GraphState * g, float pavement_alpha)=0;
};


class WED_PreviewLayer  : public WED_MapLayer {
public:

						 WED_PreviewLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_PreviewLayer();

			void		SetPavementTransparency(float alpha);
			float		GetPavementTransparency(void) const;
			void		SetObjDensity(int density);
			int			GetObjDensity(void) const;

	virtual	bool		DrawEntityVisualization		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected);
	virtual	void		GetCaps						(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel);
	virtual	void		DrawVisualization			(bool inCurent, GUI_GraphState * g);

private:

	float							mPavementAlpha;
	int								mObjDensity;
	
	// This stuff is built temporarily between the entity and final draw.
	vector<WED_PreviewItem *>	mPreviewItems;
	int							mRunwayLayer;		// Keep adding 1 to layer as we find runways, etc.  This means the runway's layer order
	int							mTaxiLayer;			// IS the hierarchy/export order, which is good.
	int							mShoulderLayer;


};

#endif /* WED_PreviewLayer_H */
