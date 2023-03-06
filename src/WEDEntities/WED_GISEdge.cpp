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
#include "GISUtils.h"
#include "WED_GroupCommands.h"
#include "WED_SimpleBezierBoundaryNode.h"
#include "WED_TaxiRoute.h"

TRIVIAL_COPY(WED_GISEdge, WED_Entity)

WED_GISEdge::WED_GISEdge(WED_Archive * parent, int id) : WED_Entity(parent, id),
	ctrl_lat_lo(this,PROP_Name("control_latitude_lo" ,XML_Name("edge","ctrl_latitude_lo" )),0.0,13,9),
	ctrl_lon_lo(this,PROP_Name("control_longitude_lo",XML_Name("edge","ctrl_longitude_lo")),0.0,14,9),
	ctrl_lat_hi(this,PROP_Name("control_latitude_hi" ,XML_Name("edge","ctrl_latitude_hi" )),0.0,13,9),
	ctrl_lon_hi(this,PROP_Name("control_longitude_hi",XML_Name("edge","ctrl_longitude_hi")),0.0,14,9)
{
}

WED_GISEdge::~WED_GISEdge()
{
}

GISClass_t		WED_GISEdge::GetGISClass		(void				 ) const
{
	return gis_Edge;
}

const char *	WED_GISEdge::GetGISSubtype	(void				 ) const
{
	return GetClass();
}

bool			WED_GISEdge::HasLayer			(GISLayer_t l) const
{
	return l == gis_Geo;
}

void			WED_GISEdge::GetBounds		(GISLayer_t l, Bbox2&  bounds) const
{
	RebuildCache(CacheBuild(cache_Spatial));
	bounds = mCacheBounds;
}

bool			WED_GISEdge::IntersectsBox	(GISLayer_t l,const Bbox2&  bounds) const
{
	RebuildCache(CacheBuild(cache_Spatial));
	if (!bounds.overlap(mCacheBounds)) return false;

	#if BENTODO
		this is not good enough
	#endif
	return true;
}

bool			WED_GISEdge::WithinBox		(GISLayer_t l,const Bbox2&  bounds) const
{
	RebuildCache(CacheBuild(cache_Spatial));
	if (bounds.contains(mCacheBounds)) return true;

	int n = GetNumSides();
	for (int i = 0; i < n; ++i)
	{
		Bezier2 b;
		GetSide(l,i,b);
		Bbox2	bb;
		b.bounds(bb);
			// Ben says: bounding-box of a bezier is assured to touch the extreme points of the bezier.
			// Thus if the bezier's bounding box is inside, the whole bezier must be inside since all extreme
			// points are.  And since there is no "slop", this is the only test we need.
		if (!bounds.contains(bb)) return false;
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
	me.expand(d,d);
	if (!me.contains(p)) return false;

	int c = GetNumSides();
	for (int n = 0; n < c; ++n)
	{
		Bezier2 b;
		if (GetSide(l,n,b))
		{
			if (b.is_near(p,d)) return true;
		} else {
			if (b.as_segment().is_near(p,d)) return true;
		}
	}
	return false;
}

bool WED_GISEdge::Cull(const Bbox2& b) const
{
	RebuildCache(CacheBuild(cache_Spatial));
	return b.overlap(mCacheBounds);
}

void			WED_GISEdge::Rescale			(GISLayer_t l,const Bbox2& old_bounds,const Bbox2& new_bounds)
{
	StateChanged();
	int t = GetNumPoints();
	for (int n = 0; n <  t; ++n)
	{
		IGISPoint * p = GetNthPoint(n);
		p->Rescale(l,old_bounds,new_bounds);
	}
}

void			WED_GISEdge::Rotate(GISLayer_t l,const Point2& ctr, double a)
{
	if(l == gis_Geo)
	{
		for (int i = 0; i < this->GetNumPoints(); ++i)
		{
			IGISPoint * p = GetNthPoint(i);
			p->Rotate(l,ctr, a);
		}
	}
}

int					WED_GISEdge::GetNumPoints(void ) const
{
	RebuildCache(CacheBuild(cache_Topological));
	return mCachePts.size();
}

// Todo: figure out how to make cache update if a source is changed. During merge() and other it operating directly on the sources, bypassing
// out needs to update the cache

IGISPoint *	WED_GISEdge::GetNthPoint (int n) const
{
	if (n == 0)
		return dynamic_cast<IGISPoint*>(GetNthSource(0));
	else if (n == GetNumPoints() - 1)
		return dynamic_cast<IGISPoint*>(GetNthSource(1));
	else
	{
		RebuildCache(CacheBuild(cache_Topological));
		return mCachePts[n];
	}
}

int					WED_GISEdge::GetNumSides(void) const
{
	RebuildCache(CacheBuild(cache_Topological));
	return mCachePts.size() - 1;
}

bool				WED_GISEdge::GetSide  (GISLayer_t l,int n, Bezier2& b) const
{
	RebuildCache(CacheBuild(cache_Topological));

	// n=-1 is a pseudo-side - it treats the whole edge as a single side
	int n1 = n == -1 ? 0 : n;
	int n2 = n == -1 ? GetNumPoints() - 1 : n + 1;

	if(n1 == 0)
		dynamic_cast<IGISPoint*>(GetNthSource(0))->GetLocation(l, b.p1);
	else
		mCachePts[n1]->GetLocation(l, b.p1);
//	mCachePts[n1]->GetLocation(l, b.p1);
	if (n1 == 0)
		b.c1 = b.p1 + Vector2(ctrl_lon_lo.value, ctrl_lat_lo.value);
	else
		mCachePtsBezier[n1]->GetControlHandleHi(l, b.c1);

	if (n2 == GetNumPoints() - 1)
		dynamic_cast<IGISPoint*>(GetNthSource(1))->GetLocation(l, b.p2);
	else
		mCachePts[n2]->GetLocation(l, b.p2);
	if (n2 == GetNumPoints() - 1)
		b.c2 = b.p2 + Vector2(ctrl_lon_hi.value, ctrl_lat_hi.value);
	else
		mCachePtsBezier[n2]->GetControlHandleLo(l, b.c2);

	return (b.p1 != b.c1 || b.p2 != b.c2);
}

bool				WED_GISEdge::IsClosed(void) const
{
	return false;
}

void WED_GISEdge::Reverse(GISLayer_t l)
{
	StateChanged();
	swap(ctrl_lat_lo.value, ctrl_lat_hi.value);
	swap(ctrl_lon_lo.value, ctrl_lon_hi.value);

	WED_Thing * p0 = GetNthSource(1);
	RemoveSource(p0);
	AddSource(p0, 0);

	Point2 pt1, pt2, c11, c12, c21, c22;
	bool b1s, b2s;
	WED_GISPoint_Bezier *p1, *p2;

	const int n = CountChildren();
	for(int i = 0; i < n/2; i++)
	{
		p1 = dynamic_cast<WED_GISPoint_Bezier *>(GetNthChild(i));
		p2 = dynamic_cast<WED_GISPoint_Bezier *>(GetNthChild(n-1-i));

		p1->GetLocation(gis_Geo, pt1);
		p2->GetLocation(gis_Geo, pt2);

		p1->GetControlHandleLo(gis_Geo, c11);
		b1s = p1->IsSplit();
		if(b1s) p1->GetControlHandleHi(gis_Geo, c12);

		p2->GetControlHandleLo(gis_Geo, c21);
		b2s = p2->IsSplit();
		if(b2s) p2->GetControlHandleHi(gis_Geo, c22);

		p2->SetLocation(gis_Geo, pt1);
		p1->SetLocation(gis_Geo, pt2);

		p2->SetSplit(b1s);
		p2->SetControlHandleHi(gis_Geo, c11);
		if(b1s) p2->SetControlHandleLo(gis_Geo, c12);
		p1->SetSplit(b2s);
		p1->SetControlHandleHi(gis_Geo, c21);
		if(b2s) p1->SetControlHandleLo(gis_Geo, c22);
	}

	if(n & 1)
	{
		p1 = dynamic_cast<WED_GISPoint_Bezier *>(GetNthChild(n/2));

		p1->GetControlHandleLo(gis_Geo, c11);
		b1s = p1->IsSplit();
		if(b1s) p1->GetControlHandleHi(gis_Geo, c12);
		p1->SetControlHandleHi(gis_Geo, c11);
		if(b1s) p1->SetControlHandleLo(gis_Geo, c12);
	}
	CacheInval(cache_Topological);
}

void WED_GISEdge::Shuffle(GISLayer_t l)
{
	this->Reverse(l);
}

WED_Thing *		WED_GISEdge::CreateSplitNode()
{
	WED_Thing * p1 = GetNthSource(0);
	WED_Thing * np = dynamic_cast<WED_Thing*>(p1->Clone());
	return np;
}

IGISPoint *	WED_GISEdge::SplitSide(const Point2& p, double dist)  // MM: add argument what segment to split ?
{
	//TODO:mroe: thats to keep old behaviour for TaxiRoute edges for now
	if(this->GetGISSubtype() == WED_TaxiRoute::sClass)
	{
		return SplitEdge(p,dist);
	}

	StateChanged();
	Bezier2		nearest_side_b;

	int nearest_side = -1;
	bool nearest_is_b = false;
	double nearest_dist = 999.0; // dist * dist;

	int ns = this->GetNumSides();
	for(int i = 0; i < ns; i++)
	{
		Bezier2		b;
		double 		d;
		bool is_b = this->GetSide(gis_Geo,i,b);
		if (b.p1 == p || b.p2 == p) return nullptr;
		if (is_b)
			d = Segment2(p, b.midpoint(b.approx_t_for_xy(p.x(), p.y()))).squared_length();
		else
			d = b.as_segment().squared_distance(p);
		if(d < nearest_dist)
		{
			nearest_dist = d;
			nearest_side = i;
			nearest_is_b = is_b;
			nearest_side_b = b;
		}
	}

	if(nearest_side < 0) return nullptr;       // nothing is close enough

	auto np = WED_SimpleBezierBoundaryNode::CreateTyped(GetArchive());

	np->SetParent(this, nearest_side);
	np->SetName("Shape Point");

	if(nearest_is_b)
	{
		double t = nearest_side_b.approx_t_for_xy(p.x(), p.y());
		Bezier2 b1, b2;
		nearest_side_b.partition(b1, b2, t);
		SetSideBezier(gis_Geo, b1, nearest_side);
		SetSideBezier(gis_Geo, b2, nearest_side+1);
	}
	else
	{
		Point2 pp = nearest_side_b.as_segment().projection(p);
		np->SetLocation(gis_Geo, pp);
	}

	return dynamic_cast<IGISPoint *>(np);
}


IGISPoint *	WED_GISEdge::SplitEdge(const Point2& p, double dist)  // MM: add argument what segment to split ?
{
	StateChanged();
	int			hit_point = -1;
	int			nearest_side = -1;
	bool		nearest_is_b = false;
	Bezier2		nearest_side_b;
	int			ns = this->GetNumSides();

	double 		nearest_dist = 999.0;
	for(int i = 0; i < ns; i++)
	{
		Bezier2		b;
		double		d;
		bool is_b = this->GetSide(gis_Geo,i,b);
		if(b.p1 == p) hit_point = i;
		if(i == ns-1 && b.p2 == p) hit_point = ns;
		if(hit_point > -1)
		{
			nearest_side = i;
			nearest_is_b = is_b;
			nearest_side_b = b;
			break;
		}

		if (is_b)
			d = Segment2(p, b.midpoint(b.approx_t_for_xy(p.x(), p.y()))).squared_length();
		else
			d = b.as_segment().squared_distance(p);

		if(d < nearest_dist)
		{
			nearest_dist = d;
			nearest_side = i;
			nearest_is_b = is_b;
			nearest_side_b = b;
		}
	}

	if(nearest_side < 0)
	{
		return nullptr;       // nothing is close enough
	}

	if(hit_point == 0 || hit_point == ns)
	{
		return nullptr; // for now, just abort

		int num_view = CountViewers();
		int src = min(hit_point,1);
		WED_Thing * np = dynamic_cast<WED_Thing *>(GetNthSource(src)->Clone());

		return dynamic_cast<IGISPoint *>(np);
	}

	Bezier2 prev_side_b;
	if(hit_point > 0 && hit_point < ns)
		GetSide(gis_Geo, hit_point-1, prev_side_b);

	WED_Thing * np = CreateSplitNode();

	WED_Thing * p1 = GetNthSource(0);
	WED_Thing * p2 = GetNthSource(1);
	np->SetParent(p1->GetParent(), p1->GetMyPosition()+1);

	string name;
	GetName(name);
	np->SetName(name + "_split");

	WED_GISEdge * me2 = dynamic_cast<WED_GISEdge*>(this->Clone()); // this also clones all children it may have
	me2->SetParent(this->GetParent(),this->GetMyPosition()+1);

	this->AddSource(np, 1);
	this->RemoveSource(p2);

	me2->AddSource(np, 0);
	me2->RemoveSource(p1);

	if(p1 == p2) // this edge is a loop , wrong, RemoveSource removes p1 and p2 because they are the same , Put it back to be able todo the split .
	{
		this->AddSource(p1, 0);
		me2->AddSource(p1, 1);
	}

	//printf("WED_SplitEdge this, me2 children %d %d, nr_side %d, hit %d ns %d\n", this->CountChildren(), me2->CountChildren(), nearest_side, hit_point, ns);
	if(ns > 1)              // delete existing ShapePoints that are on the abandoned side of the intersection
	{
		set<WED_Thing *> obsolete_nodes;
		for(int i = 0; i < ns-1; i++)
		{
			if(i < nearest_side || i == (hit_point - 1))
			{
				obsolete_nodes.insert(me2->GetNthChild(i));
			}
			if(i >= nearest_side || i == (hit_point - 1))
			{
				obsolete_nodes.insert(this->GetNthChild(i));
			}
		}

		WED_RecursiveDelete(obsolete_nodes);
		RebuildCache(CacheBuild(cache_Spatial));
	}

	if(nearest_is_b)
	{
		Bezier2 b1, b2;
		if(hit_point < 0)
		{
			nearest_side_b.partition(b1, b2, nearest_side_b.approx_t_for_xy(p.x(), p.y()));
		}
		else
		{
			b1 = prev_side_b;
			b2 = nearest_side_b;
			nearest_side--;
		}
		DebugAssert(nearest_side < GetNumSides());
		this->SetSideBezier(gis_Geo, b1, nearest_side);
		me2->SetSideBezier(gis_Geo, b2, 0);
	}
	else
	{
		Segment2 s1, s2;
		if(hit_point < 0)
		{
			Point2  pp = nearest_side_b.as_segment().projection(p);
			s1.p1 = nearest_side_b.p1;
			s1.p2 = pp;
			s2.p1 = pp;
			s2.p2 = nearest_side_b.p2;
		}
		else
		{
			s1.p1 = prev_side_b.p1;
			s1.p2 = p;
			s2.p1 = p;
			s2.p2 = nearest_side_b.p2;
			nearest_side--;
		}
		this->SetSide(gis_Geo, s1, nearest_side);
		me2->SetSide(gis_Geo, s2, 0);
	}

	return dynamic_cast<IGISPoint *>(np);
}

void		WED_GISEdge::SetSide(GISLayer_t layer, const Segment2& s, int n)
{
	DebugAssert(n < (CountChildren() + 2));

	StateChanged();
	if(n <= 0)
		dynamic_cast<IGISPoint*>(GetNthSource(0))->SetLocation(gis_Geo,s.p1);
	else
		dynamic_cast<IGISPoint*>(GetNthChild(n-1))->SetLocation(gis_Geo, s.p1);
	if( n < 0 || n >= CountChildren())
		dynamic_cast<IGISPoint*>(GetNthSource(1))->SetLocation(gis_Geo, s.p2);
	else
		dynamic_cast<IGISPoint*>(GetNthChild(n))->SetLocation(gis_Geo, s.p2);

	if(n <= 0)
	{
		ctrl_lat_lo = 0.0;
		ctrl_lon_lo = 0.0;
	}
	if( n < 0 || n >= CountChildren())
	{
		ctrl_lat_hi = 0.0;
		ctrl_lon_hi = 0.0;
	}
	CacheInval(cache_Topological);
}

void		WED_GISEdge::SetSideBezier(GISLayer_t layer, const Bezier2& b, int n)
{
	DebugAssert(n < GetNumSides());

	StateChanged();
	if(n <= 0)
	{
		dynamic_cast<IGISPoint*>(GetNthSource(0))->SetLocation(gis_Geo, b.p1);
		ctrl_lat_lo = b.c1.y() - b.p1.y();
		ctrl_lon_lo = b.c1.x() - b.p1.x();
	}
	else
	{
		auto bp = dynamic_cast<IGISPoint_Bezier *>(GetNthPoint(n));
		DebugAssert(bp != nullptr);
		if(bp)
		{
			bp->SetLocation(gis_Geo, b.p1);
			bp->SetSplit(true);
			bp->SetControlHandleHi(gis_Geo, b.c1);
		}
	}

	if(n < 0 || n >= CountChildren())
	{
		dynamic_cast<IGISPoint*>(GetNthSource(1))->SetLocation(gis_Geo, b.p2);
		ctrl_lat_hi = b.c2.y() - b.p2.y();
		ctrl_lon_hi = b.c2.x() - b.p2.x();
	}
	else
	{
		auto bp = dynamic_cast<IGISPoint_Bezier *>(GetNthPoint(n+1));
		DebugAssert(bp != nullptr);
		if(bp)
		{
			bp->SetLocation(gis_Geo, b.p2);
			bp->SetSplit(true);
			bp->SetControlHandleLo(gis_Geo, b.c2);
		}
	}
	CacheInval(cache_Topological);
}

void		WED_GISEdge::Validate(void)
{
	WED_Entity::Validate();

	DebugAssert(CountSources() == 2);
	DebugAssert(CountViewers() == 0);

	// DebugAssert(CountChildren() == 0);   ok for roads only, though

	IGISPoint * p = SAFE_CAST(IGISPoint, GetNthSource(0));
	DebugAssert(p);
				p = SAFE_CAST(IGISPoint, GetNthSource(1));
	DebugAssert(p);
}

void		WED_GISEdge::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
	WED_Entity::GetNthPropertyInfo(n, info);
	if(!this->CanBeCurved())
	if (n == PropertyItemNumber(&ctrl_lat_hi) ||
		n == PropertyItemNumber(&ctrl_lat_lo) ||
		n == PropertyItemNumber(&ctrl_lon_hi) ||
		n == PropertyItemNumber(&ctrl_lon_lo))
	{
		info.prop_name = ".";
		info.can_edit = 0;
		info.can_delete = 0;
	}
}

void	 WED_GISEdge::RebuildCache(int flags) const
{
	if (flags & cache_Topological)
	{
		mCachePts.clear();
		mCachePtsBezier.clear();
		int nc = CountChildren() + 2;
		mCachePts.reserve(nc);
		mCachePtsBezier.reserve(nc);

		for (int n = 0; n < nc; ++n)
		{
			WED_Thing* c;
			if (n == 0)
				c = GetNthSource(0);
			else if (n > this->CountChildren())
				c = GetNthSource(1);
			else
				c = GetNthChild(n - 1);

			IGISPoint* p = nullptr;
			IGISPoint_Bezier* b = dynamic_cast<IGISPoint_Bezier*>(c);
			if (b) p = b; else p = dynamic_cast<IGISPoint*>(c);
//			if (p)
			{
				mCachePts.push_back(p);
				mCachePtsBezier.push_back(b);
			}
		}
	}

	if (flags & cache_Spatial)
	{
		int m = GetNumPoints();
		for (int mm = 0; mm < m; ++mm)
		{
			Bbox2 temp;
			WED_Thing* c = GetNthChild(mm);
			IGISEntity* p = dynamic_cast<IGISEntity*>(c);
			if (p)
				p->GetBounds(gis_Geo, temp);
		}

		int n = GetNumSides();			// We MUST ensure that this only builds topo cache or we are dead dead dead!!
		mCacheBounds = Bbox2();

		for (int i = 0; i < n; ++i)
		{
			Bezier2 b;
			GetSide(gis_Geo, i, b);
			Bbox2	bb;
			b.bounds(bb);
			mCacheBounds += bb;
		}
	}
}
