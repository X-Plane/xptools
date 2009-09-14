/*
 * Copyright (c) 2008, Laminar Research.
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

#include "WED_TextureBezierNode.h"

DEFINE_PERSISTENT(WED_TextureBezierNode)
TRIVIAL_COPY(WED_TextureBezierNode, WED_GISPoint_Bezier)

WED_TextureBezierNode::WED_TextureBezierNode(WED_Archive * a, int i) : WED_GISPoint_Bezier(a,i),
	mS(this,"S","WED_texturenode","s",0.0,5,4),
	mT(this,"T","WED_texturenode","t",0.0,5,4),
	mScL(this,"S (Ctrl Low)", "WED_texturenode_bezier","sc_lo", 0.0,5,4),
	mTcL(this,"T (Ctrl Low)", "WED_texturenode_bezier","tc_lo", 0.0,5,4),
	mScH(this,"S (Ctrl Hi)", "WED_texturenode_bezier","sc_hi", 0.0,5,4),
	mTcH(this,"T (Ctrl Hi)", "WED_texturenode_bezier","tc_hi", 0.0,5,4)
{
}

WED_TextureBezierNode::~WED_TextureBezierNode()
{
}


bool		WED_TextureBezierNode::HasLayer(GISLayer_t l) const
{
	if(l == gis_UV) return true;
	return WED_GISPoint_Bezier::HasLayer(l);
}

void		WED_TextureBezierNode::SetLocation(GISLayer_t l,const Point2& st)
{
	if(l == gis_UV)
	{
		if (st.x() != mS.value || st.y() != mT.value)
		{
			StateChanged();
			mS = st.x();
			mT = st.y();
			CacheInval();
			CacheBuild();
		}
	}
	else 
		WED_GISPoint_Bezier::SetLocation(l,st);
}

void		WED_TextureBezierNode::GetLocation(GISLayer_t l,	   Point2& st) const
{
	if(l == gis_UV)
	{
		st.x_ = mS.value;
		st.y_ = mT.value;
	} else 
		WED_GISPoint_Bezier::GetLocation(l,st);
}

bool	WED_TextureBezierNode::GetControlHandleLo (GISLayer_t layer,      Point2& p) const
{
	if(!WED_GISPoint_Bezier::GetControlHandleLo(layer,p)) return false;
	if(layer == gis_UV) 
	{
		p.x_ = mScL.value + mS.value;
		p.y_ = mTcL.value + mT.value;	
	}
	return true;
}

bool	WED_TextureBezierNode::GetControlHandleHi (GISLayer_t layer,      Point2& p) const
{
	if(!WED_GISPoint_Bezier::GetControlHandleHi(layer,p)) return false;
	if(layer == gis_UV) 
	{
		p.x_ = mScH.value + mS.value;
		p.y_ = mTcH.value + mT.value;	
	}
	return true;
}

void	WED_TextureBezierNode::SetControlHandleLo (GISLayer_t layer,const Point2& p)
{
	if(layer == gis_UV)
	{
		if(p.x() != mScL.value + mS.value ||
		   p.y() != mTcL.value + mT.value)
		{
			StateChanged();
			mScL = p.x() - mS.value;
			mTcL = p.y() - mT.value;
			CacheInval();
			CacheBuild();
		}
	} else
		WED_GISPoint_Bezier::SetControlHandleLo(layer,p);
}

void	WED_TextureBezierNode::SetControlHandleHi (GISLayer_t layer,const Point2& p)
{
	if(layer == gis_UV)
	{
		if(p.x() != mScH.value + mS.value ||
		   p.y() != mTcH.value + mT.value)
		{
			StateChanged();
			mScH = p.x() - mS.value;
			mTcH = p.y() - mT.value;
			CacheInval();
			CacheBuild();
		}
	} else
		WED_GISPoint_Bezier::SetControlHandleHi(layer,p);
}
