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

#include "WED_AirportNode.h"
#include "WED_EnumSystem.h"

DEFINE_PERSISTENT(WED_AirportNode)
TRIVIAL_COPY(WED_AirportNode, WED_GISPoint_Bezier)

WED_AirportNode::WED_AirportNode(WED_Archive * a, int i) : WED_GISPoint_Bezier(a,i),
	attrs(this,".Attributes",       XML_Name("markings","marking"),LinearFeature, 0),
	lines(this,"Line Attributes",   XML_Name("",""),".Attributes",line_SolidYellow,line_BWideBrokenDouble, 1),
	lights(this,"Light Attributes", XML_Name("",""),".Attributes",line_TaxiCenter,line_BoundaryEdge, 1)
{
}

WED_AirportNode::~WED_AirportNode()
{
}

void	WED_AirportNode::SetAttributes(const set<int>& in_attrs)
{
	attrs = in_attrs;
}

void		WED_AirportNode::GetAttributes(set<int>& out_attrs) const
{
	out_attrs = attrs.value;
}

