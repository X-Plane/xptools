#include "WED_GISPolygon.h"



class Bezier_Seq_Iterator {
public:
	Bezier_Seq_Iterator(IGISPointSequence * seq, int n);
	Bezier_Seq_Iterator(const Bezier_Seq_Iterator& rhs);
	Bezier_Seq_Iterator& operator=(const Bezier_Seq_Iterator& rhs);
	
	bool operator==(const Bezier_Seq_Iterator& rhs) const;
	bool operator!=(const Bezier_Seq_Iterator& rhs) const;
	
	Bezier_Seq_Iterator& operator++(void);
	Bezier_Seq_Iterator operator++(int);
	
	Bezier2 operator*(void) const;
	
private:

	IGISPointSequence *	mSeq;
	int					mIndex;

	Bezier_Seq_Iterator();
};

WED_GISPolygon::WED_GISPolygon(WED_Archive * archive, int id) : 
	WED_Entity(archive, id)
{
}

WED_GISPolygon::~WED_GISPolygon()
{
}

GISClass_t		WED_GISPolygon::GetGISClass		(void				 ) const
{
	return gis_Polygon;
}

const char *	WED_GISPolygon::GetGISSubtype	(void				 ) const
{	
	return GetClass();
}


void			WED_GISPolygon::GetBounds		(	   Bbox2&  bounds) const
{
	CacheBuild();
	GetOuterRing()->GetBounds(bounds);
}

bool				WED_GISPolygon::IntersectsBox		(const Bbox2&  bounds) const
{
	Bbox2	me;
	
	// Quick check: if there is no union between our bbox and the bounds
	// no possible intersection - fast exit.
	GetBounds(me);
	if (!bounds.overlap(me)) return false;
	
	// We have a slight problem: rings don't have area.  So we will do TWO possible
	// tests:
	// 1. If any ring intersects the box, that means we have a border crossing...
	//    that is, some sub-segment registers as being at least partly in the box.
	//    We must intersect.
	// 2. If there are NO intersections then each ring of the polygon is either
	//    entirely inside or outside the bbox.  Therefore the bbox is either entirely
	//    inside or outside the polygon.  Test one corner.
	
	// For now we'll try case 2 first.  At least I am sure we can fast-exit
	// the "pt inside ring" case using a bbox test.  I don't know how well
	// our ring-box intersection code can fast-exit.
	
	if (PtWithin(bounds.p1)) return true;
	
	// Okay at least one point of the box is outside - if there is an interesection
	// there must be an edge crossing, um, somewhere.
	
	if (GetOuterRing()->IntersectsBox(bounds)) return true;
	
	int hh = GetNumHoles();
	for (int h = 0; h < hh; ++h)
	{
		if (GetNthHole(h)->IntersectsBox(bounds)) return true;
	}
	return false;
}

bool				WED_GISPolygon::WithinBox		(const Bbox2&  bounds) const
{
	return GetOuterRing()->WithinBox(bounds);
}


bool				WED_GISPolygon::PtWithin		(const Point2& p	 ) const
{
	// WARNING: rings do not contain the area inside them!  So PtWithin 
	// returns false for our sub-parts.  That's why we're using inside_polygon_bez.
	// If we want to fast-track exit this, we must do the bbox check ourselves!
	Bbox2	bounds;

	IGISPointSequence * outer_ring = GetOuterRing();
	// Fast exit: if the bbox of our outer ring doesn't contain the point, we can
	// go home happy fast.
	outer_ring->GetBounds(bounds);
	if (!bounds.contains(p)) return false;
	
	if (!inside_polygon_bez(
			Bezier_Seq_Iterator(outer_ring,0),
			Bezier_Seq_Iterator(outer_ring,outer_ring->GetNumPoints()),
			p))
		return false;
		
	int h = GetNumHoles();
	for (int n = 0; n < h; ++n)
	{
		IGISPointSequence * sq = GetNthHole(n);
		
		// Fast skip: if p is outside the bbox don't bother with a full
		// polygon check, which is rather expensive.
		sq->GetBounds(bounds);
		if (bounds.contains(p))
		if (inside_polygon_bez(
				Bezier_Seq_Iterator(sq,0),
				Bezier_Seq_Iterator(sq,sq->GetNumPoints()),
				p))
		return false;
	}
	return true;
}

bool				WED_GISPolygon::PtOnFrame		(const Point2& p, double d) const
{
	// Fast case: normally we must check all inner holes if we "miss" the outer hole.
	// BUT: if we are OUTSIDE the outer hole, we can early exit and not test a damned
	// thing. 
	Bbox2	me;
	GetBounds(me);
	me.p1 -= Vector2(d,d);
	me.p2 += Vector2(d,d);	
	if (!me.contains(p)) return false;
	
	if (GetOuterRing()->PtOnFrame(p,d)) return true;
	
	int h = GetNumHoles();
	for (int n = 0; n < h; ++n)
	{
		IGISPointSequence * sq = GetNthHole(n);
		if (sq->PtOnFrame(p,d)) return true;
	}
	return false;
}

void			WED_GISPolygon::Rescale(const Bbox2& old_bounds,const Bbox2& new_bounds)
{
	GetOuterRing()->Rescale(old_bounds, new_bounds);
	int h = GetNumHoles();
	for (int n = 0; n < h; ++n)
	{
		GetNthHole(n)->Rescale(old_bounds,new_bounds);
	}
}

void			WED_GISPolygon::Rotate(const Point2& ctr, double angle)
{
	GetOuterRing()->Rotate(ctr, angle);
	int h = GetNumHoles();
	for (int n = 0; n < h; ++n)
	{
		GetNthHole(n)->Rotate(ctr, angle);
	}
}


IGISPointSequence *		WED_GISPolygon::GetOuterRing(void )	const
{
	IGISPointSequence * sq = SAFE_CAST(IGISPointSequence,GetNthChild(0));
	DebugAssert(sq != NULL);
	return sq;
}

int						WED_GISPolygon::GetNumHoles (void ) const
{
	return CountChildren()-1;
}

IGISPointSequence *		WED_GISPolygon::GetNthHole  (int n)	const
{
	IGISPointSequence * sq = SAFE_CAST(IGISPointSequence,GetNthChild(n+1));
	DebugAssert(sq != NULL);
	return sq;
}

void				WED_GISPolygon::DeleteHole  (int n)
{
	WED_Thing * h = GetNthChild(n);
	h->SetParent(NULL, 0);
	h->Delete();
}

void				WED_GISPolygon::AddHole		(IGISPointSequence * r)
{
	WED_Thing * t = SAFE_CAST(WED_Thing, r);
	DebugAssert(t != NULL);	
	DebugAssert(r->GetGISClass() == gis_Ring);
	t->SetParent(this,CountChildren());
}

Bezier_Seq_Iterator::Bezier_Seq_Iterator(IGISPointSequence * seq, int n) :
	mSeq(seq), mIndex(n)
{
}

Bezier_Seq_Iterator::Bezier_Seq_Iterator(const Bezier_Seq_Iterator& rhs) :
	mSeq(rhs.mSeq),
	mIndex(rhs.mIndex)
{
}

Bezier_Seq_Iterator& Bezier_Seq_Iterator::operator=(const Bezier_Seq_Iterator& rhs)
{
	mSeq = rhs.mSeq;
	mIndex = rhs.mIndex;
	return *this;
}

bool Bezier_Seq_Iterator::operator==(const Bezier_Seq_Iterator& rhs) const
{
	return mIndex == rhs.mIndex && mSeq == rhs.mSeq;
}

bool Bezier_Seq_Iterator::operator!=(const Bezier_Seq_Iterator& rhs) const
{
	return mIndex != rhs.mIndex || mSeq != rhs.mSeq;
}

Bezier_Seq_Iterator& Bezier_Seq_Iterator::operator++(void)
{
	++mIndex;
	return *this;
}

Bezier_Seq_Iterator Bezier_Seq_Iterator::operator++(int)
{
	Bezier_Seq_Iterator me(*this);
	++mIndex;
	return me;
}

Bezier2 Bezier_Seq_Iterator::operator*(void) const
{
	Bezier2 ret;
	Segment2 s;
	if (!mSeq->GetSide(mIndex, s, ret))
	{
		ret.p1 = ret.c1 = s.p1;
		ret.p2 = ret.c2 = s.p2;
	}
	return ret;
}

