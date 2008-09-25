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

#include "WED_MapToolNew.h"


WED_MapToolNew::WED_MapToolNew(const char * tname, GUI_Pane * h, WED_MapZoomerNew * z, IResolver * i) : WED_MapLayer(h, z, i), tool_name(tname)
{
	has_anchor1 = false;
	has_anchor2 = false;
	has_distance = false;
	has_heading = false;
}

WED_MapToolNew::~WED_MapToolNew()
{
}

void		WED_MapToolNew::PropEditCallback(int before)
{
}

int					WED_MapToolNew::CountSubs(void) { return 0; }
IPropertyObject *	WED_MapToolNew::GetNthSub(int n) { return NULL; }

const char *	WED_MapToolNew::GetToolName(void) const
{
	return tool_name.c_str();
}

bool				WED_MapToolNew::GetAnchor1(Point2& a)	{ if (has_anchor1 ) a = anchor1 ; return has_anchor1 ; }
bool				WED_MapToolNew::GetAnchor2(Point2& a)	{ if (has_anchor2 ) a = anchor2 ; return has_anchor2 ; }
bool				WED_MapToolNew::GetDistance(double& d)	{ if (has_distance) d = distance; return has_distance; }
bool				WED_MapToolNew::GetHeading(double& h)	{ if (has_heading ) h = heading ; return has_heading ; }

void				WED_MapToolNew::SetAnchor1(const Point2& a)	{ has_anchor1  = true; anchor1  = a; }
void				WED_MapToolNew::SetAnchor2(const Point2& a)	{ has_anchor2  = true; anchor2  = a; }
void				WED_MapToolNew::SetDistance(double d)		{ has_distance = true; distance = d; }
void				WED_MapToolNew::SetHeading(double h)		{ has_heading  = true; heading  = h; }

void				WED_MapToolNew::ClearAnchor1(void)	{ has_anchor1  = false; }
void				WED_MapToolNew::ClearAnchor2(void)	{ has_anchor2  = false; }
void				WED_MapToolNew::ClearDistance(void)	{ has_distance = false; }
void				WED_MapToolNew::ClearHeading(void)	{ has_heading  = false; }
