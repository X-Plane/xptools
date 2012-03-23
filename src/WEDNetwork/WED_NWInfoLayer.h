/*
 * Copyright (c) 2011, mroe.
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

#ifndef WED_NWINFOLAYER_H
#define WED_NWINFOLAYER_H

#include "WED_MapLayer.h"
#include "GUI_Timer.h"
#include "GUI_Listener.h"


class	WED_NWInfoLayer : public GUI_Listener, public WED_MapLayer {
public:

					WED_NWInfoLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver);
		virtual	~WED_NWInfoLayer();


			void	ReceiveMessage(	GUI_Broadcaster * inSrc,intptr_t inMsg,intptr_t inParam);

	virtual void	DrawVisualization		(bool inCurrent, GUI_GraphState * g);

	virtual void	GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks);

	virtual	int					HandleClickDown(int inX, int inY, int inButton, GUI_KeyFlags modifiers);
	virtual	void				HandleClickDrag(int inX, int inY, int inButton, GUI_KeyFlags modifiers);
	virtual	void				HandleClickUp  (int inX, int inY, int inButton, GUI_KeyFlags modifiers);
	


private:

	float	x, y;
	int	is_click;

			float	mColor[4];
};

#endif /* WED_NWINFOLAYER_H */
