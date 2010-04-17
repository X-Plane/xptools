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

#include "WED_GISLine.h"
#include "AssertUtils.h"

TRIVIAL_COPY(WED_GISLine, WED_Entity)

WED_GISLine::WED_GISLine(WED_Archive * parent, int id) :
	WED_Entity(parent, id)
{
}

WED_GISLine::~WED_GISLine()
{
}

GISClass_t		WED_GISLine::GetGISClass		(void				 ) const
{
	return gis_Line;
}

const char *	WED_GISLine::GetGISSubtype	(void				 ) const
{
	return GetClass();
}

bool			WED_GISLine::HasLayer		(GISLayer_t l) const
{
	return GetSource()->HasLayer(l) && GetTarget()->HasLayer(l);
}

void			WED_GISLine::GetBounds		(GISLayer_t l,Bbox2&  bounds) const
{
	CacheBuild();
	Point2 p1,p2;
	GetSource()->GetLocation(l,p1);
	GetTarget()->GetLocation(l,p2);
	bounds = Bbox2(p1,p2);
}

bool			WED_GISLine::IntersectsBox	(GISLayer_t l,const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(l,me);
	if (!me.overlap(bounds)) return false;

	#if BENTODO
		not quite good enough??
	#endif
	return true;
}

bool			WED_GISLine::WithinBox		(GISLayer_t l,const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(l,me);
	return bounds.contains(me);
}

bool			WED_GISLine::PtWithin		(GISLayer_t l,const Point2& p	 ) const
{
	return false;
}

bool			WED_GISLine::PtOnFrame		(GISLayer_t l,const Point2& p, double dist ) const
{
	Bbox2	me;
	GetBounds(l,me);
	me.p1 -= Vector2(dist,dist);
	me.p2 += Vector2(dist,dist);
	if (!me.contains(p)) return false;

	Segment2 s;
	GetSource()->GetLocation(l,s.p1);
	GetTarget()->GetLocation(l,s.p2);
	return s.is_near(p,dist);
}

bool WED_GISLine::Cull(const Bbox2& b) const
{
	Bbox2	me;
	GetBounds(gis_Geo, me);
	return b.overlap(me);
}

void	WED_GISLine::Rescale			(GISLayer_t l,const Bbox2& old_bounds,const Bbox2& new_bounds)
{
	GetSource()->Rescale(l,old_bounds,new_bounds);
	GetTarget()->Rescale(l,old_bounds,new_bounds);
}

void	WED_GISLine::Rotate			(GISLayer_t l,const Point2& ctr, double angle)
{
	GetSource()->Rotate(l,ctr,angle);
	GetTarget()->Rotate(l,ctr,angle);
}


int					WED_GISLine::GetNumPoints(void ) const
{
	return 2;
}

//void				WED_GISLine::DeletePoint (int n)
//{
//	Assert(!"You cannot delete points from a line.");
//}

IGISPoint *	WED_GISLine::SplitSide   (const Point2& p, double dist)
{
	return NULL;
}

IGISPoint *	WED_GISLine::GetNthPoint (int n) const
{
	Assert(n == 0 || n == 1);
	return (n == 1) ? GetTarget() : GetSource();
}

bool		WED_GISLine::IsClosed(void) const
{
	return false;
}


IGISPoint *		WED_GISLine::GetSource(void) const
{
	IGISPoint * p = SAFE_CAST(IGISPoint,GetNthChild(0));
	Assert(p != NULL);
	return p;
}

IGISPoint *		WED_GISLine::GetTarget(void) const
{
	IGISPoint * p = SAFE_CAST(IGISPoint,GetNthChild(1));
	Assert(p != NULL);
	return p;
}

int					WED_GISLine::GetNumSides(void) const
{
	return 1;
}

bool				WED_GISLine::GetSide(GISLayer_t l,int n, Segment2& s, Bezier2& b) const
{
	Assert(n == 0);
	GetSource()->GetLocation(l,s.p1);
	GetTarget()->GetLocation(l,s.p2);
	return false;
}

void WED_GISLine::Reverse(GISLayer_t l)
{
	Point2	ends[2];
	GetSource()->GetLocation(l,ends[0]);
	GetTarget()->GetLocation(l,ends[1]);
	GetSource()->SetLocation(l,ends[1]);
	GetTarget()->SetLocation(l,ends[0]);
}