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

class	WED_MapZoomerNew;
class	GUI_GraphState;
class	IGISEntity;
class	IResolver;
class	GUI_Pane;

class	WED_MapLayer {
public:

						 WED_MapLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_MapLayer();

	// These provide generalized drawing routines.  Use this to draw background images and other such stuff.
	virtual	void		DrawVisualization		(bool inCurrent, GUI_GraphState * g)	{ }
	virtual	void		DrawStructure			(bool inCurrent, GUI_GraphState * g) { }
	virtual	void		DrawSelected			(bool inCurrent, GUI_GraphState * g) { }

	// These draw specific entities.  Use these to draw pieces of the data model.  Only visible entities will be passed in!
	virtual	bool		DrawEntityVisualization	(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected) { return false; }
	virtual	bool		DrawEntityStructure		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected) { return false; }

			bool		IsVisible(void) const;
	virtual	void		ToggleVisible(void);

	// Extra iterations over the entity hiearchy get very expensive.  This routine returns whether a layer wants
	// per-entity drawing passes for either structure or visualization.  We can also say whether we need "seleted" to be
	// calculated accurately - checking selection slows down the sped of drawing passes.
	virtual	void		GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel)=0;

protected:

	inline	WED_MapZoomerNew *	GetZoomer(void) const { return mZoomer; }
	inline	IResolver *			GetResolver(void) const { return mResolver; }
	inline	GUI_Pane *			GetHost(void) const { return mHost; }

			double				GetFurnitureIconScale(void) const;
			double				GetFurnitureIconRadius(void) const;
			double				GetAirportIconScale(void) const;
			double				GetAirportIconRadius(void) const;
	inline	int					GetAirportTransWidth(void) const { return mAirportTransWidth; }

private:

						 WED_MapLayer();

	bool					mVisible;

	WED_MapZoomerNew *		mZoomer;
	IResolver *				mResolver;
	GUI_Pane *				mHost;

	double					mAirportRadius;
	double					mFurnitureRadius;
	double					mAirportFactor;
	double					mFurnitureFactor;

	int						mAirportTransWidth;



};


#endif /* WED_MAPLAYER_H */
