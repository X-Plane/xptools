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

#include "WED_TextureNode.h"

DEFINE_PERSISTENT(WED_TextureNode)
TRIVIAL_COPY(WED_TextureNode, WED_GISPoint)

WED_TextureNode::WED_TextureNode(WED_Archive * a, int i) : WED_GISPoint(a,i),
	mS(this,"S",XML_Name("texture_node","s"),0.0,5,4),
	mT(this,"T",XML_Name("texture_node","t"),0.0,5,4)
{
}

WED_TextureNode::~WED_TextureNode()
{
}

bool		WED_TextureNode::HasLayer(GISLayer_t l) const
{
	if(l == gis_UV) return true;
	return WED_GISPoint::HasLayer(l);
}


void		WED_TextureNode::SetLocation(GISLayer_t l, const Point2& st)
{
	if(l == gis_UV)
	{
		if (st.x() != mS.value || st.y() != mT.value)
		{
			StateChanged();
			mS = st.x();
			mT = st.y();
			CacheInval(cache_Spatial);
			CacheBuild(cache_Spatial);			
		}
	} else
		WED_GISPoint::SetLocation(l,st);
}

void		WED_TextureNode::GetLocation(GISLayer_t l,	   Point2& st) const
{
	if(l == gis_UV)
	{
		CacheBuild(cache_Spatial);
		st.x_ = mS.value;
		st.y_ = mT.value;
	} else WED_GISPoint::GetLocation(l,st);
}
