#include "WED_GISLine.h"
#include "AssertUtils.h"



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


void			WED_GISLine::GetBounds		(	   Bbox2&  bounds) const
{
	CacheBuild();
	Point2 p1,p2;
	GetSource()->GetLocation(p1);
	GetTarget()->GetLocation(p2);
	bounds = Bbox2(p1,p2);
}

bool			WED_GISLine::IntersectsBox	(const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(me);
	if (!me.overlap(bounds)) return false;
	
	#if BENTODO
		not quite good enough??
	#endif
	return true;
}

bool			WED_GISLine::WithinBox		(const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(me);
	return bounds.contains(me);
}

bool			WED_GISLine::PtWithin		(const Point2& p	 ) const
{
	return false;
}

bool			WED_GISLine::PtOnFrame		(const Point2& p, double dist ) const
{
	Bbox2	me;
	GetBounds(me);
	me.p1 -= Vector2(dist,dist);
	me.p2 += Vector2(dist,dist);
	if (!me.contains(p)) return false;
	
	Segment2 s;
	GetSource()->GetLocation(s.p1);
	GetTarget()->GetLocation(s.p2);	
	return s.is_near(p,dist);
}

void	WED_GISLine::Rescale			(const Bbox2& old_bounds,const Bbox2& new_bounds)
{
	GetSource()->Rescale(old_bounds,new_bounds);
	GetTarget()->Rescale(old_bounds,new_bounds);
}

void	WED_GISLine::Rotate			(const Point2& ctr, double angle)
{
	GetSource()->Rotate(ctr,angle);
	GetTarget()->Rotate(ctr,angle);
}


int					WED_GISLine::GetNumPoints(void ) const
{
	return 2;
}

//void				WED_GISLine::DeletePoint (int n)
//{
//	Assert(!"You cannot delete points from a line.");
//}

IGISPoint *	WED_GISLine::SplitSide   (int n)
{
	Assert(!"You cannot split a line.");
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

bool				WED_GISLine::GetSide(int n, Segment2& s, Bezier2& b) const
{
	Assert(n == 0);
	GetSource()->GetLocation(s.p1);
	GetTarget()->GetLocation(s.p2);
	return false;
}

