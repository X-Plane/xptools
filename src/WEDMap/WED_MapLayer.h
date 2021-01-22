/*
 * Copyright (c) 2007, Laminar Research.
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

#ifndef WED_MAPLAYER_H
#define WED_MAPLAYER_H

#include "GUI_Defs.h"

#define MIN_PIXELS_PREVIEW 5.0   // cutt off preview if object is (roughly) smaller than this many pixels.
											// For airport lines/light, show structural preview if they are smaller than this instead

class	WED_Camera;
class	WED_MapProjection;
class	WED_MapZoomerNew;
class	GUI_GraphState;
class	IGISEntity;
class	WED_Thing;
class	IResolver;
class	GUI_Pane;


struct FilterSpec 
{
	FilterSpec(const char * p) { e.push_back(p); }
	FilterSpec(const char * p, const char * p2) { e.push_back(p); e.push_back(p2); }
	FilterSpec(const char * p, const char * p2, const char * p3) { e.push_back(p); e.push_back(p2); e.push_back(p3); }
	
	bool operator==(const char * rhs) const { return e.size() == 1 && rhs == e.front(); }
    bool operator==(const FilterSpec& rhs) const { return rhs.e.size() == e.size() && rhs.e == e; }
        
	vector<const char *> e;
};

typedef vector<FilterSpec> MapFilter_t;

class	WED_MapLayer {
public:

						 WED_MapLayer(GUI_Pane * host, const WED_MapProjection * projection,
							WED_Camera * camera, IResolver * resolver);
	virtual				~WED_MapLayer();
	
	virtual	int					HandleClickDown(int inX, int inY, int inButton, GUI_KeyFlags modifiers) { return 0; }
	virtual	void				HandleClickDrag(int inX, int inY, int inButton, GUI_KeyFlags modifiers) {			}
	virtual	void				HandleClickUp  (int inX, int inY, int inButton, GUI_KeyFlags modifiers)	{			}
	

	// These provide generalized drawing routines.  Use this to draw background images and other such stuff.
	virtual	void		DrawVisualization		(bool inCurrent, GUI_GraphState * g) { }
	virtual	void		DrawStructure			(bool inCurrent, GUI_GraphState * g) { }
	virtual	void		DrawSelected			(bool inCurrent, GUI_GraphState * g) { }

	// These draw specific entities.  Use these to draw pieces of the data model.  Only visible entities will be passed in!
	virtual	bool		DrawEntityVisualization	(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected) { return false; }
	virtual	bool		DrawEntityStructure		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected) { return false; }

			bool		IsVisible(void) const;
			void		SetVisible(bool visibility);
	virtual	void		ToggleVisible(void);
			void		SetFilter(const MapFilter_t * hide_filter_ptr, const MapFilter_t * lock_filter_ptr); // client MUST retain storage!!!

	// Extra iterations over the entity hiearchy get very expensive.  This routine returns whether a layer wants
	// per-entity drawing passes for either structure or visualization.  We can also say whether we need "seleted" to be
	// calculated accurately - checking selection slows down the sped of drawing passes.
	// Finally, "wants_clicks" tells if we want to interact with the mouse.  Tool derivatives do NOT need to set this -
	// it is assumed that ALL tools get clicks.  But by asking for clicks, layers can jump in and grab the mouse too.
	virtual	void		GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)=0;

	bool				IsVisibleNow(IGISEntity * ent) const;
	bool				IsVisibleNow(WED_Thing * ent) const;
	bool				IsLockedNow(IGISEntity * ent) const;
	bool				IsLockedNow(WED_Thing * ent) const;

protected:

	const WED_MapProjection *	GetProjection(void) const { return mProjection; }
	WED_Camera *				GetCamera(void) const { return mCamera; }
	inline	IResolver *			GetResolver(void) const { return mResolver; }
	inline	GUI_Pane *			GetHost(void) const { return mHost; }


	// WED defines two types of icons: furniture icons for all the stuff in an airport (VASI/PAPI, signs, windsocks) and airport
	// icons for the iconic representation of an entire airport at far view.  In both cases we have two vars:
	//
	// The scale is how many pixels per meter the icon is rendered at.  Thus the icons have 'physical' size.
	// The icon radius is the current size of the icon in pixels at the current zoom - it is based on measuring one canonical icon
	// resource to know its size.  Note that this means that all icons in a family (airports, furnitures) must be close to the same size
	// for click testing to work.
			double				GetFurnitureIconScale(void) const;
			double				GetFurnitureIconRadius(void) const;
			double				GetAirportIconScale(void) const;
			double				GetAirportIconRadius(void) const;
	inline	int					GetAirportTransWidth(void) const { return mAirportTransWidth; }

private:

	friend		class	WED_Map;				// for visible-now filter accessors

						 WED_MapLayer();

	bool					mVisible;

	IResolver *				mResolver;
	GUI_Pane *				mHost;

	double					mAirportRadius;
	double					mFurnitureRadius;
	double					mAirportFactor;
	double					mFurnitureFactor;

	int						mAirportTransWidth;
	
	const MapFilter_t *		mHideFilter;
	const MapFilter_t *		mLockFilter;

	const WED_MapProjection *	mProjection;
	WED_Camera *				mCamera;

};

class	WED_MapLayerWithZoomer : public WED_MapLayer {
public:

	WED_MapLayerWithZoomer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);

protected:
	WED_MapZoomerNew *	GetZoomer(void) const { return mZoomer; }

private:

	friend		class	WED_Map;		// for visible-now filter accessors

	WED_MapLayerWithZoomer();

	WED_MapZoomerNew *		mZoomer;

};


#endif /* WED_MAPLAYER_H */
