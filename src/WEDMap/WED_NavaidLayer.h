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

#ifndef WED_NavaidLayer_H
#define WED_NavaidLayer_H

#include "WED_MapLayer.h"
#include "CompGeomDefs2.h"

struct navaid_t {
	int		type;
	Point2 	lonlat;
	float	heading;
	string	name;
	string	icao;
	int     freq;     // ATC tower freq for airports, in kHz
	string	rwy;      // Or some other informative text
	vector<Polygon2> shape; // airspaces only
};

struct MFMemFile;

class WED_NavaidLayer : public WED_MapLayer {
public:

						 WED_NavaidLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_NavaidLayer();

	virtual	void		DrawVisualization		(bool inCurrent, GUI_GraphState * g);
	virtual	void		GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks);

private:

	class navaid_list {      // helper class to enforce vector is sorted anytime iterated over using cbegin(lon)
	public:
					navaid_list();
		auto		empty() const { return nav_list.empty(); }
		auto		cbegin() const { return nav_list.cbegin(); }
		vector<navaid_t>::const_iterator cbegin(double longitude);   // starts iterating at first navaid >= longitude
		auto		cend() const { return nav_list.cend(); }
		void		insert(const navaid_t& aid);
		void		replace(vector<navaid_t>::const_iterator which, const navaid_t& aid);

	private:
		vector<navaid_t> nav_list;
		int			best_begin;
	};

	void		LoadNavaids(void);
	void		parse_nav_dat(MFMemFile* str, bool merge);
	void		parse_atc_dat(MFMemFile* str);

	navaid_list			mNavaids;
};

#endif /* WED_NavaidLayer_H */
