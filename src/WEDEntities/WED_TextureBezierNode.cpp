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
#include "GISUtils.h"

DEFINE_PERSISTENT(WED_TextureBezierNode)
TRIVIAL_COPY(WED_TextureBezierNode, WED_GISPoint_Bezier)

WED_TextureBezierNode::WED_TextureBezierNode(WED_Archive * a, int i) : WED_GISPoint_Bezier(a,i),
	mS(this,"S",				XML_Name("texture_node","s"),	  0.0,5,4),
	mT(this,"T",				XML_Name("texture_node","t"),	  0.0,5,4),
	mScL(this,"S (Ctrl Low)",	XML_Name("texture_node","sc_lo"), 0.0,5,4),
	mTcL(this,"T (Ctrl Low)",	XML_Name("texture_node","tc_lo"), 0.0,5,4),
	mScH(this,"S (Ctrl Hi)",	XML_Name("texture_node","sc_hi"), 0.0,5,4),
	mTcH(this,"T (Ctrl Hi)",	XML_Name("texture_node","tc_hi"), 0.0,5,4)
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
			CacheInval(cache_Spatial);
			CacheBuild(cache_Spatial);
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
	if(!WED_GISPoint_Bezier::GetControlHandleLo(gis_Geo,p)) return false;
	if(layer == gis_UV) 
	{
		p.x_ = mScL.value + mS.value;
		p.y_ = mTcL.value + mT.value;	
	}
	return true;
}

bool	WED_TextureBezierNode::GetControlHandleHi (GISLayer_t layer,      Point2& p) const
{
	if(!WED_GISPoint_Bezier::GetControlHandleHi(gis_Geo,p)) return false;
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
			CacheInval(cache_Spatial);
			CacheBuild(cache_Spatial);
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
			CacheInval(cache_Spatial);
			CacheBuild(cache_Spatial);
		}
	} else
		WED_GISPoint_Bezier::SetControlHandleHi(layer,p);
}


void			WED_TextureBezierNode::Rescale			(GISLayer_t l, const Bbox2& old_bounds, const Bbox2& new_bounds)
{
	if(l == gis_Geo)	WED_GISPoint_Bezier::Rescale(l,old_bounds,new_bounds);
	else
	{
		Point2 p;
		GetLocation(l,p);
		
		StateChanged();

		mS.value = old_bounds.rescale_to_x(new_bounds,mS.value);
		mT.value = old_bounds.rescale_to_y(new_bounds,mT.value);
		
		mScL.value = old_bounds.rescale_to_xv(new_bounds,mScL.value);
		mTcL.value = old_bounds.rescale_to_yv(new_bounds,mTcL.value );
		mScH.value = old_bounds.rescale_to_xv(new_bounds,mScH.value);
		mTcH.value = old_bounds.rescale_to_yv(new_bounds,mTcH.value );

		CacheInval(cache_Spatial);
		CacheBuild(cache_Spatial);
	}
}

void			WED_TextureBezierNode::Rotate			(GISLayer_t l, const Point2& ctr, double a)
{
	if(l == gis_Geo)	WED_GISPoint_Bezier::Rotate(l,ctr,a);
	else
	if (a != 0.0)
	{
		Point2 p;
		GetLocation(l,p);
		StateChanged();

		Point2	pt_old_lo(p.x() + mScL.value, p.y() + mTcL.value);
		Point2	pt_old_hi(p.x() + mScH.value, p.y() + mTcH.value);
		Vector2	v_old_lo = VectorLLToMeters(ctr, Vector2(ctr,pt_old_lo));
		Vector2	v_old_hi = VectorLLToMeters(ctr, Vector2(ctr,pt_old_hi));
		double old_len_lo = sqrt(v_old_lo.squared_length());
		double old_len_hi = sqrt(v_old_hi.squared_length());

		double old_ang_lo = VectorMeters2NorthHeading(ctr,ctr,v_old_lo);
		double old_ang_hi = VectorMeters2NorthHeading(ctr,ctr,v_old_hi);
		Vector2	v_new_lo;
		Vector2	v_new_hi;

		NorthHeading2VectorMeters(ctr, ctr, old_ang_lo + a, v_new_lo);
		NorthHeading2VectorMeters(ctr, ctr, old_ang_hi + a, v_new_hi);
		v_new_lo.normalize();
		v_new_hi.normalize();
		v_new_lo *= old_len_lo;
		v_new_hi *= old_len_hi;

		v_new_lo = VectorMetersToLL(ctr,v_new_lo);
		v_new_hi = VectorMetersToLL(ctr,v_new_hi);

		WED_GISPoint::Rotate(l,ctr,a);
		GetLocation(l,p);

		mScL.value = ctr.x() + v_new_lo.dx - p.x();
		mScH.value = ctr.x() + v_new_hi.dx - p.x();
		mTcL.value = ctr.y() + v_new_lo.dy - p.y();
		mTcH.value = ctr.y() + v_new_hi.dy - p.y();
		CacheInval(cache_Spatial);
		CacheBuild(cache_Spatial);

	}
	
}

