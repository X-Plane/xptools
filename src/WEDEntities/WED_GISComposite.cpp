#include "WED_GISComposite.h"



WED_GISComposite::WED_GISComposite(WED_Archive * a, int i) : WED_Entity(a,i)
{
}

WED_GISComposite::~WED_GISComposite()
{
}

GISClass_t		WED_GISComposite::GetGISClass		(void				 ) const
{
	return gis_Composite;
}

const char *	WED_GISComposite::GetGISSubtype	(void				 ) const
{	
	return GetClass();
}

void			WED_GISComposite::GetBounds		(	   Bbox2&  bounds) const
{
	if (CacheBuild())	RebuildCache();
	bounds = mCacheBounds;
}

bool			WED_GISComposite::IntersectsBox	(const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(me);	
	if (!bounds.overlap(me)) return false;

	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if (GetNthEntity(i)->IntersectsBox(bounds)) return true;
	return false;
}

bool			WED_GISComposite::WithinBox		(const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(me);	
	if (bounds.contains(me)) return true;

	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if (!GetNthEntity(i)->WithinBox(bounds)) return false;
	return (n > 0);
}

bool			WED_GISComposite::PtWithin		(const Point2& p	 ) const
{
	Bbox2	me;
	GetBounds(me);	
	if (!me.contains(p)) return false;

	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if (GetNthEntity(i)->PtWithin(p)) return true;
	return false;
}

bool			WED_GISComposite::PtOnFrame		(const Point2& p, double d) const
{
	Bbox2	me;
	GetBounds(me);	
	me.p1 -= Vector2(d,d);
	me.p2 += Vector2(d,d);
	if (!me.contains(p)) return false;

	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if (GetNthEntity(i)->PtOnFrame(p, d)) return true;
	return false;
}

void			WED_GISComposite::Rescale(const Bbox2& old_bounds,const Bbox2& new_bounds)
{
	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		GetNthEntity(i)->Rescale(old_bounds,new_bounds);
}

int				WED_GISComposite::GetNumEntities(void ) const
{
	if (CacheBuild())	RebuildCache();
	return mEntities.size();
}

IGISEntity *	WED_GISComposite::GetNthEntity  (int n) const
{
	if (CacheBuild())	RebuildCache();
	return mEntities[n];
}


void	WED_GISComposite::RebuildCache(void) const
{
	mCacheBounds = Bbox2();
	mEntities.clear();
	int n = CountChildren();
	mEntities.reserve(n);
	for (int i = 0; i <  n; ++i)
	{
		IGISEntity * ent = dynamic_cast<IGISEntity *>(GetNthChild(i));
		if (ent)
		{
			Bbox2 child;
			ent->GetBounds(child);
			mCacheBounds += child;
			mEntities.push_back(ent);
		}	
	}
}
