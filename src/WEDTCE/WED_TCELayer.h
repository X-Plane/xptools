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

#ifndef WED_TCELayer_H
#define WED_TCELayer_H

class	GUI_Pane;
class	WED_MapZoomerNew;
class	IResolver;
class	GUI_GraphState;
class	IGISEntity;

class WED_TCELayer {
public:

						 WED_TCELayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_TCELayer();

	// These provide generalized drawing routines.  Use this to draw background images and other such stuff.
	virtual	void		DrawVisualization		(bool inCurrent, GUI_GraphState * g) { }
	virtual	void		DrawStructure			(bool inCurrent, GUI_GraphState * g) { }
	virtual	void		DrawSelected			(bool inCurrent, GUI_GraphState * g) { }

	// These draw specific entities.  Use these to draw pieces of the data model.  Only visible entities will be passed in!
	virtual	void		DrawEntityVisualization	(bool inCurrent, IGISEntity * entity, GUI_GraphState * g) { }
	virtual	void		DrawEntityStructure		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g) { }

	// Extra iterations over the entity hiearchy get very expensive.  This routine returns whether a layer wants
	// per-entity drawing passes for either structure or visualization.  We can also say whether we need "seleted" to be
	// calculated accurately - checking selection slows down the sped of drawing passes.
	virtual	void		GetCaps(bool& draw_ent_v, bool& draw_ent_s)=0;

protected:

	inline	WED_MapZoomerNew *	GetZoomer(void) const { return mZoomer; }
	inline	IResolver *			GetResolver(void) const { return mResolver; }
	inline	GUI_Pane *			GetHost(void) const { return mHost; }

private:

						 WED_TCELayer();

	WED_MapZoomerNew *		mZoomer;
	IResolver *				mResolver;
	GUI_Pane *				mHost;

};


#endif /* WED_TCELayer_H */
