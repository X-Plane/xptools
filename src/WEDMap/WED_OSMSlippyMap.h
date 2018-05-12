/*
 * Copyright (c) 2015, Laminar Research.
 *
 * Created by Ben Supnik on 12/18/15.
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
 */

#ifndef WED_OSMSlippyMap_h
#define WED_OSMSlippyMap_h

class	WED_file_cache_request;

#include "GUI_Timer.h"
#include "WED_MapLayer.h"

class	WED_SlippyMap : public WED_MapLayer, public GUI_Timer {
public:

					 WED_SlippyMap(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual			~WED_SlippyMap();
	
	virtual	void	DrawVisualization(bool inCurrent, GUI_GraphState * g);
	virtual	void	GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks);
	virtual	void	TimerFired(void);
			void	SetMode(int mode);
			int		GetMode(void);

private:

			void	finish_loading_tile();

	WED_file_cache_request* m_cache_request;

	//The texture cache, where they key is the tile texture path on disk and the value is the texture id
	map<string,int>				m_cache;

			int		mMapMode;
			string	url_printf_fmt;
			string	dir_printf_fmt;
			bool	is_jpg_not_png;
};

#endif /* WED_OSMSlippyMap_h */
