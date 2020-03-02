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

#ifndef WED_MAP_H
#define WED_MAP_H

#include "CompGeomDefs2.h"
#include "GUI_Pane.h"
#include "GUI_Button.h"
#include "WED_MapZoomerNew.h"
#include "GUI_Listener.h"
#include "GUI_Commander.h"
#include <stdint.h>
#include "WED_MapLayer.h"

class	WED_MapLayer;
class	WED_MapToolNew;
class	IResolver;
class	IGISEntity;
class	ISelection;

class	WED_Map : public GUI_Pane, public WED_MapZoomerNew, public GUI_Listener, public GUI_Commander {
public:

						 WED_Map(IResolver * in_resolver, GUI_Commander * cmdr);
	virtual				~WED_Map();

			void		SetTool(WED_MapToolNew * tool);
			void		AddLayer(WED_MapLayer * layer);
	
			void		SetFilter(const string& name, const MapFilter_t& hide_filter, const MapFilter_t& lock_filter);

	virtual void		SetBounds(int x1, int y1, int x2, int y2);
	virtual void		SetBounds(int inBounds[4]);

	virtual	void		Draw(GUI_GraphState * state);

	virtual	int			MouseDown(int x, int y, int button);
	virtual	void		MouseDrag(int x, int y, int button);
	virtual	void		MouseUp  (int x, int y, int button);
	virtual	int			MouseMove(int x, int y);
	virtual	int			ScrollWheel(int x, int y, int dist, int axis);

	virtual	int			HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags);

	virtual	void		ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam);

private:

			void		DrawVisFor(WED_MapLayer * layer, int current, const Bbox2& bounds, IGISEntity * what, GUI_GraphState * g, ISelection * sel, int depth);
			void		DrawStrFor(WED_MapLayer * layer, int current, const Bbox2& bounds, IGISEntity * what, GUI_GraphState * g, ISelection * sel, int depth);

		IGISEntity *	GetGISBase();
		ISelection *	GetSel();


	vector<WED_MapLayer *>			mLayers;
	WED_MapToolNew *				mTool;
	IResolver *						mResolver;

	MapFilter_t						mHideFilter;
	MapFilter_t						mLockFilter;
	string							mFilterName;

	WED_MapLayer *	mClickLayer;
	int				mX;
	int				mY;

	int				mX_Orig;
	int				mY_Orig;
	int				mIsDownCount;
	int				mIsDownExtraCount;
	
	GUI_Button *	mTiltButton[4];
};


#endif

