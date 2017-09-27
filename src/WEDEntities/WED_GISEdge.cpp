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

TRIVIAL_COPY(WED_GISEdge, WED_Entity)

WED_GISEdge::WED_GISEdge(WED_Archive * parent, int id) : WED_Entity(parent, id),
	ctrl_lat_lo(this,"control_latitude_lo" ,XML_Name("edge","ctrl_latitude_lo" ),0.0,13,9),
	ctrl_lon_lo(this,"control_longitude_lo",XML_Name("edge","ctrl_longitude_lo"),0.0,14,9),
	ctrl_lat_hi(this,"control_latitude_hi" ,XML_Name("edge","ctrl_latitude_hi" ),0.0,13,9),
	ctrl_lon_hi(this,"control_longitude_hi",XML_Name("edge","ctrl_longitude_hi"),0.0,14,9)

/*	mScL(this,"S (Ctrl Low)", "WED_texturenode_bezier","sc_lo", 0.0,5,4),
	mTcL(this,"T (Ctrl Low)", "WED_texturenode_bezier","tc_lo", 0.0,5,4),
	mScH(this,"S (Ctrl Hi)", "WED_texturenode_bezier","sc_hi", 0.0,5,4),
	mTcH(this,"T (Ctrl Hi)", "WED_texturenode_bezier","tc_hi", 0.0,5,4)*/
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
//	return GetNthPoint(0)->HasLayer(l) &&
//		   GetNthPoint(1)->HasLayer(l);
}

void			WED_GISEdge::GetBounds		(GISLayer_t l, Bbox2&  bounds) const
{
	CacheBuild(cache_Spatial);

	Segment2	s;
	Bezier2		b;
	if(GetSide(l,0,s,b))
		b.bounds(bounds);
	else
		bounds = Bbox2(s.p1,s.p2);
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

bool WED_GISEdge::Cull(const Bbox2& b) const
{
	Bbox2	me;
	GetBounds(gis_Geo, me);
	return b.overlap(me);
}

void			WED_GISEdge::Rescale			(GISLayer_t l,const Bbox2& old_bounds,const Bbox2& new_bounds)
{
	int t = GetNumPoints();
	for (int n = 0; n <  t; ++n)
	{
		IGISPoint * p = GetNthPoint(n);
		p->Rescale(l,old_bounds,new_bounds);
	}
	StateChanged();
	if(l == gis_Geo)
	{
		ctrl_lon_lo.value = old_bounds.rescale_to_xv(new_bounds,ctrl_lon_lo.value );
		ctrl_lat_lo.value = old_bounds.rescale_to_yv(new_bounds,ctrl_lat_lo.value );
		ctrl_lon_hi.value = old_bounds.rescale_to_xv(new_bounds,ctrl_lon_hi.value );
		ctrl_lat_hi.value = old_bounds.rescale_to_yv(new_bounds,ctrl_lat_hi.value );
	}
/*	if(l == gis_UV)
	{
		mScL.value = old_bounds.rescale_to_xv(new_bounds,mScL.value);
		mTcL.value = old_bounds.rescale_to_yv(new_bounds,mTcL.value );
		mScH.value = old_bounds.rescale_to_xv(new_bounds,mScH.value);
		mTcH.value = old_bounds.rescale_to_yv(new_bounds,mTcH.value );
	}*/
	
}

void			WED_GISEdge::Rotate			(GISLayer_t l,const Point2& ctr, double a)
{
	if (a != 0.0)
	if(l == gis_Geo)
	{
		Point2 p1;
		Point2 p2;
		GetNthPoint(0)->GetLocation(l,p1);
		GetNthPoint(1)->GetLocation(l,p2);
		StateChanged();
		
		Point2	pt_old_lo(p1.x() + ctrl_lon_lo.value, p1.y() + ctrl_lat_lo.value);
		Point2	pt_old_hi(p2.x() + ctrl_lon_hi.value, p2.y() + ctrl_lat_hi.value);
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

		GetNthPoint(0)->Rotate(l,ctr,a);
		GetNthPoint(1)->Rotate(l,ctr,a);

		GetNthPoint(0)->GetLocation(l,p1);
		GetNthPoint(1)->GetLocation(l,p2);

		ctrl_lon_lo.value = ctr.x() + v_new_lo.dx - p1.x();
		ctrl_lon_hi.value = ctr.x() + v_new_hi.dx - p2.x();
		ctrl_lat_lo.value = ctr.y() + v_new_lo.dy - p1.y();
		ctrl_lat_hi.value = ctr.y() + v_new_hi.dy - p2.y();
	}
}

int					WED_GISEdge::GetNumPoints(void ) const
{
	return 2;
}

IGISPoint *	WED_GISEdge::GetNthPoint (int n) const
{
	return dynamic_cast<IGISPoint *>(GetNthSource(n));
}

int					WED_GISEdge::GetNumSides(void) const
{
	return 1;
}

bool				WED_GISEdge::GetSide  (GISLayer_t l,int n, Segment2& s, Bezier2& b) const
{
	GetNthPoint(0)->GetLocation(l,b.p1);
	GetNthPoint(1)->GetLocation(l,b.p2);

	b.c1 = b.p1 + Vector2(ctrl_lon_lo.value,ctrl_lat_lo.value);
	b.c2 = b.p2 + Vector2(ctrl_lon_hi.value,ctrl_lat_hi.value);
	
	s.p1 = b.p1;
	s.p2 = b.p2;
	
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

	WED_Thing * p2 = GetNthSource(1);
	RemoveSource(p2);
	AddSource(p2, 0);
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



IGISPoint *	WED_GISEdge::SplitSide   (const Point2& p, double dist)
{
	Segment2	s;
	Bezier2		b;
	bool is_b = GetSide(gis_Geo,0,s,b);
	if (s.p1 == p || s.p2 == p) return NULL;

	
	WED_Thing * p1 = GetNthSource(0);
	WED_Thing * p2 = GetNthSource(1);

	WED_Thing * np = CreateSplitNode();
	np->SetParent(p1->GetParent(), p1->GetMyPosition()+1);
	
	string name;
	np->GetName(name);
	name += "(split)";
	np->SetName(name);
	
	WED_GISEdge * me2 = dynamic_cast<WED_GISEdge*>(this->Clone());
	
	me2->SetParent(this->GetParent(),this->GetMyPosition()+1);
	
	this->AddSource(np,1);
	this->RemoveSource(p2);
	
	me2->AddSource(np,0);
	me2->RemoveSource(p1);
	
	if(is_b)
	{
		double t = b.approx_t_for_xy(p.x(), p.y());
		Bezier2 b1, b2;
		b.partition(b1, b2, t);
		this->SetSideBezier(gis_Geo, b1);
		me2->SetSideBezier(gis_Geo,b2);
	}
	else
	{
		Segment2 s1(s), s2(s);
		
		s1.p2 = p;
		s2.p1 = p;
		
		this->SetSide(gis_Geo, s1);
		me2->SetSide(gis_Geo, s2);
	}

	return dynamic_cast<IGISPoint *>(np);	
}

void		WED_GISEdge::SetSide(GISLayer_t layer, const Segment2& s)
{
	StateChanged();
	GetNthPoint(0)->SetLocation(gis_Geo,s.p1);
	GetNthPoint(1)->SetLocation(gis_Geo,s.p2);
	ctrl_lat_lo = 0.0;
	ctrl_lon_lo = 0.0;
	ctrl_lat_hi = 0.0;
	ctrl_lon_hi = 0.0;
}

void		WED_GISEdge::SetSideBezier(GISLayer_t layer, const Bezier2& b)
{
	StateChanged();
	GetNthPoint(0)->SetLocation(gis_Geo,b.p1);
	GetNthPoint(1)->SetLocation(gis_Geo,b.p2);
	ctrl_lat_lo = b.c1.y() - b.p1.y();
	ctrl_lon_lo = b.c1.x() - b.p1.x();
	ctrl_lat_hi = b.c2.y() - b.p2.y();
	ctrl_lon_hi = b.c2.x() - b.p2.x();
}


void		WED_GISEdge::Validate(void)
{
	WED_Entity::Validate();
	
	DebugAssert(CountSources() == 2);
	DebugAssert(CountViewers() == 0);
	DebugAssert(CountChildren() == 0);
	
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
