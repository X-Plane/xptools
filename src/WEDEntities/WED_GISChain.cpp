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

void			WED_GISChain::GetBounds		(	   Bbox2&  bounds) const
{
	int n = GetNumSides();
	bounds = Bbox2();
	
	for (int i = 0; i < n; ++i)
	{
		Segment2 s;
		Bezier2 b;
		if (GetSide(i,s,b))
		{
			Bbox2	bb;
			b.bounds(bb);
			bounds += bb;
		} else {
			bounds += s.p1;
			bounds += s.p2;
		}		
	}
}

bool				WED_GISChain::IntersectsBox	(const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(me);
	return bounds.overlap(me);
}

bool			WED_GISChain::WithinBox		(const Bbox2&  bounds) const
{	
	int n = GetNumSides();
	for (int i = 0; i < n; ++i)
	{
		Segment2 s;
		Bezier2 b;
		if (GetSide(i,s,b))
		{
			Bbox2	bb;
			b.bounds(bb);
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

int					WED_GISChain::GetNumPoints(void ) const
{
	return CountChildren();
}

void	WED_GISChain::DeletePoint(int n)
{
	WED_Thing * k = GetNthChild(n);
	k->SetParent(NULL, 0);
	k->Delete();
}


IGISPoint *	WED_GISChain::GetNthPoint (int n) const
{
	IGISPoint * p = SAFE_CAST(IGISPoint,GetNthChild(n));
	DebugAssert(p != NULL);
	return p;
}


int			WED_GISChain::GetNumSides(void) const
{
	int n = CountChildren();
	return (IsClosed()) ? n : (n-1);
}

bool		WED_GISChain::GetSide(int n, Segment2& s, Bezier2& b) const
{
	int n1 = n;
	int n2 = (n + 1) % this->GetNumPoints();
	
	IGISPoint * p1 = this->GetNthPoint(n1);
	IGISPoint * p2 = this->GetNthPoint(n2);
	IGISPoint_Bezier * c1 = (p1->GetGISClass()==gis_Point_Bezier) ? SAFE_CAST(IGISPoint_Bezier,p1) : NULL;
	IGISPoint_Bezier * c2 = (p2->GetGISClass()==gis_Point_Bezier) ? SAFE_CAST(IGISPoint_Bezier,p2) : NULL;

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

