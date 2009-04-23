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

#ifndef WED_TCE_H
#define WED_TCE_H

class	WED_TCEToolNew;
class	WED_TCELayer;
#include "ITexMgr.h"
class	IResolver;
class	ISelection;
class	IGISEntity;


#include "GUI_Pane.h"
#include "WED_MapZoomerNew.h"
#include "GUI_Listener.h"

class WED_TCE : public GUI_Pane, public WED_MapZoomerNew, public GUI_Listener {
public:

						 WED_TCE(IResolver * in_resolver);
	virtual				~WED_TCE();

			void		SetTool(WED_TCEToolNew * tool);
			void		AddLayer(WED_TCELayer * layer);

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

		IGISEntity *	GetGISBase();
		ISelection *	GetSel();

		TexRef			mTex;
		bool			mKillAlpha;

	vector<WED_TCELayer *>			mLayers;
	WED_TCEToolNew *				mTool;
	IResolver *						mResolver;

	int				mIsToolClick;
	int				mX;
	int				mY;

	int				mX_Orig;
	int				mY_Orig;
	int				mIsDownCount;
	int				mIsDownExtraCount;

	void	CalcBgknd(void);

};


#endif /* WED_TCE_H */
