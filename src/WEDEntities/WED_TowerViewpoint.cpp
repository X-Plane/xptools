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

#include "WED_TowerViewpoint.h"
#include "AptDefs.h"
#include "XESConstants.h"

DEFINE_PERSISTENT(WED_TowerViewpoint)
TRIVIAL_COPY(WED_TowerViewpoint, WED_GISPoint)

WED_TowerViewpoint::WED_TowerViewpoint(WED_Archive * a, int i) : WED_GISPoint(a,i),
	height(this,"Height", XML_Name("tower_viewpoint","height"),0,3,0)
{
}

WED_TowerViewpoint::~WED_TowerViewpoint()
{
}

void	WED_TowerViewpoint::SetHeight(double h)
{
	height = h;
}

void		WED_TowerViewpoint::Import(const AptTowerPt_t& x, void (* print_func)(void *, const char *, ...), void * ref)
{
	SetLocation(gis_Geo,x.location);
	height = x.height_ft * FT_TO_MTR;
	SetName(x.name);
}

void		WED_TowerViewpoint::Export(		 AptTowerPt_t& x) const
{
	GetLocation(gis_Geo,x.location);
	x.height_ft = height * MTR_TO_FT;
	GetName(x.name);
	x.draw_obj = 0;
}
