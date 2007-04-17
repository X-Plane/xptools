#include "WED_GISPolygon.h"

START_CASTING(WED_GISPolygon)
IMPLEMENTS_INTERFACE(IGISEntity)
IMPLEMENTS_INTERFACE(IGISPolygon)
INHERITS_FROM(WED_Entity)
END_CASTING

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

void			WED_GISPolygon::GetBounds		(	   Bbox2&  bounds) const
{
	GetOuterRing()->GetBounds(bounds);
}

bool				WED_GISPolygon::WithinBox		(const Bbox2&  bounds) const
{
	return GetOuterRing()->WithinBox(bounds);
}

bool				WED_GISPolygon::PtWithin		(const Point2& p	 ) const
{
	IGISPointSequence * outer_ring = GetOuterRing();
	if (!inside_polygon_bez(
			Bezier_Seq_Iterator(outer_ring,0),
			Bezier_Seq_Iterator(outer_ring,outer_ring->GetNumPoints()),
			p))
		return false;
		
	int h = GetNumHoles();
	for (int n = 0; n < h; ++n)
	{
		IGISPointSequence * sq = GetNthHole(n);

		if (!inside_polygon_bez(
				Bezier_Seq_Iterator(sq,0),
				Bezier_Seq_Iterator(sq,sq->GetNumPoints()),
				p))
		return false;
	}
	return true;
}

bool				WED_GISPolygon::PtOnFrame		(const Point2& p, double d) const
{
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
#if !DEV
examine this -- does this make sense?
#endif

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

