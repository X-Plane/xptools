/* 
 * Copyright (c) 2009, Laminar Research.
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

#include "WED_GISEdge.h"

TRIVIAL_COPY(WED_GISEdge, WED_Entity)

WED_GISEdge::WED_GISEdge(WED_Archive * parent, int id) : WED_Entity(parent, id)
{
}

WED_GISEdge::~WED_GISEdge()
{
}

GISClass_t		WED_GISEdge::GetGISClass		(void				 ) const
{
	return gis_Chain;
}

const char *	WED_GISEdge::GetGISSubtype	(void				 ) const
{
	return GetClass();
}

bool			WED_GISEdge::HasLayer			(GISLayer_t l) const
{
	if(CacheBuild()) RebuildCache();
	return mCacheHasUV;
}

void			WED_GISEdge::GetBounds		(GISLayer_t l, Bbox2&  bounds) const
{
	if (CacheBuild())	RebuildCache();
	bounds = l == gis_UV ? mCacheBoundsUV : mCacheBounds;
}

bool			WED_GISEdge::IntersectsBox	(GISLayer_t l,const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(l,me);
	if (!bounds.overlap(me)) return false;

	#if BENTODO
		this is not good enough
	#endif
	return true;
}

bool			WED_GISEdge::WithinBox		(GISLayer_t l,const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(l,me);
	if (bounds.contains(me)) return true;

	int n = GetNumSides();
	for (int i = 0; i < n; ++i)
	{
		Segment2 s;
		Bezier2 b;
		if (GetSide(l,i,s,b))
		{
			Bbox2	bb;
			b.bounds(bb);
			// Ben says: bounding-box of a bezier is assured to touch the extreme points of the bezier.
			// Thus if the bezier's bounding box is inside, the whole bezier must be inside since all extreme
			// points are.  And since there is no "slop", this is the only test we need.
			if (!bounds.contains(bb)) return false;
		} else {
			if (!bounds.contains(s.p1)) return false;
			if (!bounds.contains(s.p2)) return false;
		}
	}
	return true;
}

bool			WED_GISEdge::PtWithin		(GISLayer_t l,const Point2& p	 ) const
{
	return false;
}

bool			WED_GISEdge::PtOnFrame		(GISLayer_t l,const Point2& p, double d) const
{
	Bbox2	me;
	GetBounds(l,me);
	me.p1 -= Vector2(d,d);
	me.p2 += Vector2(d,d);
	if (!me.contains(p)) return false;

	int c = GetNumSides();
	for (int n = 0; n < c; ++n)
	{
		Segment2 s;
		Bezier2 b;
		if (GetSide(l,n,s,b))
		{
			if (b.is_near(p,d)) return true;
		} else {
			if (s.is_near(p,d)) return true;
		}
	}
	return false;
}

void			WED_GISEdge::Rescale			(GISLayer_t l,const Bbox2& old_bounds,const Bbox2& new_bounds)
{
	int t = GetNumPoints();
	for (int n = 0; n <  t; ++n)
	{
		IGISPoint * p = GetNthPoint(n);
		p->Rescale(l,old_bounds,new_bounds);
	}
}

void			WED_GISEdge::Rotate			(GISLayer_t l,const Point2& center, double angle)
{
	int t = GetNumPoints();
	for (int n = 0; n <  t; ++n)
	{
		IGISPoint * p = GetNthPoint(n);
		p->Rotate(l,center, angle);
	}
}

int					WED_GISEdge::GetNumPoints(void ) const
{
	if (CacheBuild())	RebuildCache();
	return mCachePts.size();
}

IGISPoint *	WED_GISEdge::GetNthPoint (int n) const
{
	if (CacheBuild())	RebuildCache();
	return mCachePts[n];
}

int					WED_GISEdge::GetNumSides(void) const
{
	if (CacheBuild())	RebuildCache();
	int n = mCachePts.size();
	return n-1;
}

bool				WED_GISEdge::GetSide  (GISLayer_t l,int n, Segment2& s, Bezier2& b) const
{
	if (CacheBuild())	RebuildCache();

	int n1 = n;
	int n2 = (n + 1) % mCachePts.size();

	IGISPoint * p1 = mCachePts[n1];
	IGISPoint * p2 = mCachePts[n2];
	IGISPoint_Bezier * c1 = mCachePtsBezier[n1];
	IGISPoint_Bezier * c2 = mCachePtsBezier[n2];

	p1->GetLocation(l,b.p1);
	p2->GetLocation(l,b.p2);
	b.c1 = b.p1;		// Mirror end-points to controls so that if we are a half-bezier,
	b.c2 = b.p2;		// we don't have junk in our bezier.

	// If we have a bezier point, fetch i.  Null out our ptrs to the bezier point
	// if the bezier handle doesn't exist -- this is a flag to us!
	if (c1) if (!c1->GetControlHandleHi(l,b.c1)) c1 = NULL;
	if (c2) if (!c2->GetControlHandleLo(l,b.c2)) c2 = NULL;

	// If we have neither end, we either had no bezier pt, or the bezier pt has no control handle.
	// Simpify down to a segment and return it -- some code may use this 'fast case'.
	if (!c1 && !c2)
	{
		s.p1 = b.p1;
		s.p2 = b.p2;
		return false;
	}
	return true;
}

bool				WED_GISEdge::IsClosed(void) const
{
	return false;
}

void WED_GISEdge::Reverse(GISLayer_t l)
{
	if (CacheBuild())	RebuildCache();
	int n,t,np = GetNumPoints();
	vector<Point2>	p(np);
	vector<Point2>	p_l(np);
	vector<Point2>	p_h(np);
	vector<int>		split(np);
	vector<int>		has_lo(np);
	vector<int>		has_hi(np);

	for(n = 0; n < np; ++n)
	{
		mCachePtsBezier[n]->GetLocation(l,p[n]);
		has_lo[n] = mCachePtsBezier[n]->GetControlHandleLo(l,p_l[n]);
		has_hi[n] = mCachePtsBezier[n]->GetControlHandleHi(l,p_h[n]);
		split[n] = mCachePtsBezier[n]->IsSplit();
	}

	for(n = 0; n < np; ++n)
	{
		t = np - n - 1;
		mCachePtsBezier[t]->SetLocation(l,p[n]);
		mCachePtsBezier[t]->SetSplit(split[n]);

		if (has_lo[n])	mCachePtsBezier[t]->SetControlHandleHi(l,p_l[n]);
		else			mCachePtsBezier[t]->DeleteHandleHi();

		if (has_hi[n])	mCachePtsBezier[t]->SetControlHandleLo(l,p_h[n]);
		else			mCachePtsBezier[t]->DeleteHandleLo();
	}
}


IGISPoint *	WED_GISEdge::SplitSide   (const Point2& p, double dist)
{
	int s = GetNumSides();
	int best = -1;
	double d = dist*dist;
	Segment2	best_p;
	for(int n = 0; n < s; ++n)
	{
		Segment2 s;
		Bezier2 b;
		if(!GetSide(gis_Geo, n, s, b))
		{
			double dd = s.squared_distance(p);
			if (dd < d)
			{
				d = dd;
				best = n;
				best_p = s;
			}
		}
	}
	if(best != -1)
	{
		WED_Thing * np = dynamic_cast<WED_Thing*>(dynamic_cast<WED_Thing*>(GetNthPoint(best))->Clone());
		IGISPoint * npp = dynamic_cast<IGISPoint *>(np);		
		Point2 bpp_proj=best_p.projection(p);
		npp->SetLocation(gis_Geo, bpp_proj);

		if(HasLayer(gis_UV))
		{
			double t = sqrt(best_p.squared_distance(bpp_proj)) /
					   sqrt(best_p.squared_length());
			Segment2 uvs;
			Bezier2  uvb;
			if(GetSide(gis_UV, best, uvs, uvb))
				npp->SetLocation(gis_UV, uvb.midpoint(t));
			else
				npp->SetLocation(gis_UV, uvs.midpoint(t));
		}
		np->SetParent(this, best);

		return npp;
	}
	return NULL;
}



void WED_GISEdge::RebuildCache(void) const
{
	mCachePts.clear();
	mCachePtsBezier.clear();
	int nc = CountChildren();
	mCachePts.reserve(nc);
	mCachePtsBezier.reserve(nc);
	mCacheHasUV = nc > 0;

	WED_Thing * src = GetNthSource(0);
	WED_Thing * dst = GetNthSource(1);

	IGISPoint * src_p = dynamic_cast<IGISPoint *>(src);
	IGISPoint * dst_p = dynamic_cast<IGISPoint *>(dst);

	mCachePts.push_back(src_p);
	mCachePtsBezier.push_back(NULL);		// intentional - even if we are anchored to a bezier, the control handle isn't NECESSARILY ours - pretend we can't see it.

	int n;
	for (n = 0; n < nc; ++n)
	{
		WED_Thing * c = GetNthChild(n);
		IGISPoint *		   p = NULL;
		IGISPoint_Bezier * b = dynamic_cast<IGISPoint_Bezier *>(c);
		if (b) p = b; else p = dynamic_cast<IGISPoint *>(c);
		if (p)
		{
			mCachePts.push_back(p);
			mCachePtsBezier.push_back(b);
			
			if(mCacheHasUV && !p->HasLayer(gis_UV))
				mCacheHasUV = false;
		}
	}

	mCachePts.push_back(dst_p);
	mCachePtsBezier.push_back(NULL);		// intentional - even if we are anchored to a bezier, the control handle isn't NECESSARILY ours - pretend we can't see it.

	n = GetNumSides();
	mCacheBounds = Bbox2();
	mCacheBoundsUV = Bbox2();
	for (int i = 0; i < n; ++i)
	{
		Segment2 s;
		Bezier2 b;
		if (GetSide(gis_Geo,i,s,b))
		{
			Bbox2	bb;
			b.bounds(bb);
			mCacheBounds += bb;
		} else {
			mCacheBounds += s.p1;
			mCacheBounds += s.p2;
		}
		
		if(mCacheHasUV)
		{
			if (GetSide(gis_UV,i,s,b))
			{
				Bbox2	bb;
				b.bounds(bb);
				mCacheBoundsUV += bb;
			} else {
				mCacheBoundsUV += s.p1;
				mCacheBoundsUV += s.p2;
			}
		}
	}
}
