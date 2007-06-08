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

void			WED_GISComposite::GetBounds		(	   Bbox2&  bounds) const
{
	bounds = Bbox2();
	int n = GetNumEntities();
	for (int i = 0; i <  n; ++i)
	{
		Bbox2 child;
		GetNthEntity(i)->GetBounds(child);
		bounds += child;
	}
}

bool			WED_GISComposite::IntersectsBox	(const Bbox2&  bounds) const
{
	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if (GetNthEntity(i)->IntersectsBox(bounds)) return true;
	return false;
}

bool			WED_GISComposite::WithinBox		(const Bbox2&  bounds) const
{
	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if (!GetNthEntity(i)->WithinBox(bounds)) return false;
	return (n > 0);
}

bool			WED_GISComposite::PtWithin		(const Point2& p	 ) const
{
	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if (GetNthEntity(i)->PtWithin(p)) return true;
	return false;
}

bool			WED_GISComposite::PtOnFrame		(const Point2& p, double d) const
{
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
	int cc = CountChildren();
	int t = 0;
	for (int n = 0; n < cc; ++n)
	if (dynamic_cast<IGISEntity *>(GetNthChild(n)))
		++t;
	return t;
}

IGISEntity *	WED_GISComposite::GetNthEntity  (int n) const
{
	int cc = CountChildren();
	for (int c = 0; c < cc; ++c)
	{
		IGISEntity * ent = dynamic_cast<IGISEntity *>(GetNthChild(c));
		if (ent)
		{
			if (n == 0) return ent;
			--n;
		}
	}
	DebugAssert(!"Bad entity nubmer.");
	return NULL;
}

