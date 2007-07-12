#include "WED_GISChain.h"



WED_GISChain::WED_GISChain(WED_Archive * parent, int id) :
	WED_Entity(parent, id)
{
}

WED_GISChain::~WED_GISChain()
{
}

GISClass_t		WED_GISChain::GetGISClass		(void				 ) const
{
	return IsClosed() ? gis_Ring : gis_Chain;
}

const char *	WED_GISChain::GetGISSubtype	(void				 ) const
{	
	return GetClass();
}

void			WED_GISChain::GetBounds		(	   Bbox2&  bounds) const
{
	if (CacheBuild())	RebuildCache();
	bounds = mCacheBounds;
}

bool				WED_GISChain::IntersectsBox	(const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(me);
	if (!bounds.overlap(me)) return false;

	#if BENTODO
		this is not good enough
	#endif
	return true;
}

bool			WED_GISChain::WithinBox		(const Bbox2&  bounds) const
{	
	Bbox2	me;
	GetBounds(me);
	if (bounds.contains(me)) return true;

	int n = GetNumSides();
	for (int i = 0; i < n; ++i)
	{
		Segment2 s;
		Bezier2 b;
		if (GetSide(i,s,b))
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

bool			WED_GISChain::PtWithin		(const Point2& p	 ) const
{
	// Rings do NOT contain area!  They are just lines that are self-connected.
	return false;
}

bool			WED_GISChain::PtOnFrame		(const Point2& p, double d	 ) const
{
	Bbox2	me;
	GetBounds(me);
	me.p1 -= Vector2(d,d);
	me.p2 += Vector2(d,d);
	if (!me.contains(p)) return false;

	int c = GetNumSides();
	for (int n = 0; n < c; ++n)
	{
		Segment2 s;
		Bezier2 b;
		if (GetSide(n,s,b))
		{
			if (b.is_near(p,d)) return true;		
		} else {
			if (s.is_near(p,d)) return true;
		}		
	}
	return false;
}

void			WED_GISChain::Rescale			(const Bbox2& old_bounds,const Bbox2& new_bounds)
{
	int t = GetNumPoints();
	for (int n = 0; n <  t; ++n)
	{
		IGISPoint * p = GetNthPoint(n);
		p->Rescale(old_bounds,new_bounds);
	}
}

void			WED_GISChain::Rotate			(const Point2& ctr, double angle)
{
	int t = GetNumPoints();
	for (int n = 0; n <  t; ++n)
	{
		IGISPoint * p = GetNthPoint(n);
		p->Rotate(ctr, angle);
	}
}

int					WED_GISChain::GetNumPoints(void ) const
{
	if (CacheBuild())	RebuildCache();
	return mCachePts.size();
}

/*
void	WED_GISChain::DeletePoint(int n)
{
	WED_Thing * k = GetNthChild(n);
	k->SetParent(NULL, 0);
	k->Delete();
}
*/


IGISPoint *	WED_GISChain::GetNthPoint (int n) const
{
	if (CacheBuild())	RebuildCache();
	return mCachePts[n];
}


int			WED_GISChain::GetNumSides(void) const
{
	if (CacheBuild())	RebuildCache();
	int n = mCachePts.size();
	return (IsClosed()) ? n : (n-1);
}

bool		WED_GISChain::GetSide(int n, Segment2& s, Bezier2& b) const
{
	if (CacheBuild())	RebuildCache();

	int n1 = n;
	int n2 = (n + 1) % mCachePts.size();
	
	IGISPoint * p1 = mCachePts[n1];
	IGISPoint * p2 = mCachePts[n2];
	IGISPoint_Bezier * c1 = mCachePtsBezier[n1];
	IGISPoint_Bezier * c2 = mCachePtsBezier[n2];

	p1->GetLocation(b.p1);
	p2->GetLocation(b.p2);
	b.c1 = b.p1;		// Mirror end-points to controls so that if we are a half-bezier,
	b.c2 = b.p2;		// we don't have junk in our bezier.
	
	// If we have a bezier point, fetch i.  Null out our ptrs to the bezier point
	// if the bezier handle doesn't exist -- this is a flag to us!
	if (c1) if (!c1->GetControlHandleHi(b.c1)) c1 = NULL;
	if (c2) if (!c2->GetControlHandleLo(b.c2)) c2 = NULL;
	
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


void WED_GISChain::RebuildCache(void) const
{
	mCachePts.clear();
	mCachePtsBezier.clear();	
	int nc = CountChildren();
	mCachePts.reserve(nc);
	mCachePtsBezier.reserve(nc);
	
	for (int n = 0; n < nc; ++n)
	{
		WED_Thing * c = GetNthChild(n);
		IGISPoint *		   p = NULL;
		IGISPoint_Bezier * b = dynamic_cast<IGISPoint_Bezier *>(c);
		if (b) p = b; else p = dynamic_cast<IGISPoint *>(c);
		if (p)
		{
			mCachePts.push_back(p);
			mCachePtsBezier.push_back(b);
		}
	}
	
	int n = GetNumSides();
	mCacheBounds = Bbox2();
	
	for (int i = 0; i < n; ++i)
	{
		Segment2 s;
		Bezier2 b;
		if (GetSide(i,s,b))
		{
			Bbox2	bb;
			b.bounds(bb);
			mCacheBounds += bb;
		} else {
			mCacheBounds += s.p1;
			mCacheBounds += s.p2;
		}		
	}
}