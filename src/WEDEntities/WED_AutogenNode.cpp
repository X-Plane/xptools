/* 
 * Copyright (c) 2020, Laminar Research.
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

#include "WED_AutogenNode.h"
#include "WED_AutogenPlacement.h"

DEFINE_PERSISTENT(WED_AutogenNode)
TRIVIAL_COPY(WED_AutogenNode,WED_GISPoint_Bezier)

WED_AutogenNode::WED_AutogenNode(WED_Archive * a, int i) : WED_GISPoint_Bezier(a,i)
	,spawning(this,PROP_Name("Spawn Tiles", XML_Name("autogen_node","spawn_tiles")), 1)
{
}

WED_AutogenNode::~WED_AutogenNode()
{
}

bool	WED_AutogenNode::GetSpawning(void) const
{
	return spawning.value;
}

void	WED_AutogenNode::SetSpawning(bool sp)
{
	spawning = sp;
}

bool	WED_AutogenNode::HasLayer		(GISLayer_t layer	) const
{
	if(layer == gis_Param) return true;
	return WED_GISPoint_Bezier::HasLayer(layer);
}

void	WED_AutogenNode::GetLocation		(GISLayer_t l,       Point2& p) const
{
	if(l == gis_Param) p = Point2(spawning.value,0.0);
	else				WED_GISPoint_Bezier::GetLocation(l, p);
}

bool	WED_AutogenNode::GetControlHandleLo (GISLayer_t l,       Point2& p) const
{
	GISLayer_t rl = (l == gis_Param) ? gis_Geo : l;
	if(!WED_GISPoint_Bezier::GetControlHandleLo(rl,p)) return false;
	if (l == gis_Param) p = Point2(spawning.value,0.0);
	return true;
}

bool	WED_AutogenNode::GetControlHandleHi (GISLayer_t l,       Point2& p) const
{
	GISLayer_t rl = (l == gis_Param) ? gis_Geo : l;
	if(!WED_GISPoint_Bezier::GetControlHandleHi(rl,p)) return false;
	if (l == gis_Param) p = Point2(spawning.value,0.0);
	return true;
}
