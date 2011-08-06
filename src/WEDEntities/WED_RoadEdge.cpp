/* 
 * Copyright (c) 2010, Laminar Research.
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

#include "WED_RoadEdge.h"
#include "WED_EnumSystem.h"

#if AIRPORT_ROUTING

DEFINE_PERSISTENT(WED_RoadEdge)
TRIVIAL_COPY(WED_RoadEdge, WED_GISEdge)


WED_RoadEdge::WED_RoadEdge(WED_Archive * a, int i) : WED_GISEdge(a,i),
	layer(this,"Layer",SQL_Name("WED_roadedge", "layer"),XML_Name("road_edge","layer"),0,2),
	subtype(this,"Type",SQL_Name("WED_roadedge", "subtype"),XML_Name("road_edge","sub_type"), RoadSubType, road_Highway)
{
}

WED_RoadEdge::~WED_RoadEdge()
{
}

bool			WED_RoadEdge::IsOneway(void) const
{
	return false;
}

void	WED_RoadEdge::SetLayer(int l)
{
	layer = l;
}
void	WED_RoadEdge::SetSubtype(int s)
{
	subtype = s;
}

#endif
