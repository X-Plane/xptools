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

#ifndef WED_TERRASERVER_LAYER_H
#define WED_TERRASERVER_LAYER_H

#include "WED_MapLayer.h"
#include "GUI_Timer.h"

#define NUM_LEVELS 10

class AsyncImage;
class AsyncImageLocator;
class AsyncConnectionPool;

class	WED_TerraserverLayer : public WED_MapLayer, public GUI_Timer {
public:

						 WED_TerraserverLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_TerraserverLayer();

			void		ToggleVisible(void);

	// These provide generalized drawing routines.  Use this to draw background images and other such stuff.
	virtual	void		DrawVisualization		(bool inCurrent, GUI_GraphState * g);

	virtual	void		GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks);

	virtual	void		TimerFired(void);

private:

			const char *	ResString(int z);

	map<long long, AsyncImage*>		mImages[NUM_LEVELS];

	int mX1[NUM_LEVELS], mX2[NUM_LEVELS], mY1[NUM_LEVELS], mY2[NUM_LEVELS], mDomain[NUM_LEVELS], mHas[NUM_LEVELS];

	AsyncImageLocator *						mLocator[NUM_LEVELS];
	AsyncConnectionPool *					mPool;

	string									mData;
	int										mRes;
	bool									mVis;
	string									mStatus;

};

#endif /* WED_TERRASERVER_LAYER_H */
