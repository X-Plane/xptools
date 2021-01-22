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

#include "WED_GISComposite.h"

TRIVIAL_COPY(WED_GISComposite, WED_Entity)

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

bool			WED_GISComposite::HasLayer	(GISLayer_t l) const
{
	RebuildCache(CacheBuild(cache_Topological));
	return (l == gis_UV) ? mHasUV : true;
}


void			WED_GISComposite::GetBounds		(GISLayer_t l, Bbox2&  bounds) const
{
	RebuildCache(CacheBuild(cache_Spatial|cache_Topological));
	bounds = (l == gis_UV) ? mCacheBoundsUV : mCacheBounds;
}

// This fixes a nasty WED quirk: GIS composites "accumulate" their children's space, even if the children
// are locked.  So we hack our point testers - if we can find a WED entity, check its lock status.  Gross but
// necessary.
static bool IsWEDLocked(IGISEntity * g)
{
	WED_Entity * e = dynamic_cast<WED_Entity *>(g);
	if(!e) return false;
	return e->GetLocked() || e->GetHidden();
}

bool			WED_GISComposite::IntersectsBox	(GISLayer_t l, const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(l,me);
	if (!bounds.overlap(me)) return false;

	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if (GetNthEntity(i)->IntersectsBox(l,bounds)) 
		if(!IsWEDLocked(GetNthEntity(i)))		
			return true;
	return false;
}

bool			WED_GISComposite::WithinBox		(GISLayer_t l, const Bbox2&  bounds) const
{
	Bbox2	me;
	GetBounds(l, me);
	if (bounds.contains(me)) return true;

	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if (!GetNthEntity(i)->WithinBox(l, bounds)) return false;
	return (n > 0);
}

bool			WED_GISComposite::PtWithin		(GISLayer_t l, const Point2& p	 ) const
{
	Bbox2	me;
	GetBounds(l, me);
	if (!me.contains(p)) return false;

	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if (GetNthEntity(i)->PtWithin(l, p)) 
		if(!IsWEDLocked(GetNthEntity(i)))				
			return true;
	return false;
}

bool			WED_GISComposite::PtOnFrame		(GISLayer_t l, const Point2& p, double d) const
{
	Bbox2	me;
	GetBounds(l, me);
	me.p1 -= Vector2(d,d);
	me.p2 += Vector2(d,d);
	if (!me.contains(p)) return false;

	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if (GetNthEntity(i)->PtOnFrame(l, p, d)) 
		if(!IsWEDLocked(GetNthEntity(i)))		
			return true;
	return false;
}

Bbox3 WED_GISComposite::GetVisibleBounds() const
{
	RebuildCache(CacheBuild(cache_Spatial | cache_Topological));
	return mCacheVisibleBounds;
}

bool WED_GISComposite::Cull(const Bbox2& b) const
{
	Bbox2 me;
	this->GetBounds(gis_Geo, me);
	me.expand(GLOBAL_WED_ART_ASSET_FUDGE_FACTOR);

	if(!b.overlap(me))
		return false;
	
	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
		if(GetNthEntity(i)->Cull(b))
			return true;
	return false;	
}

void			WED_GISComposite::Rescale(GISLayer_t l, const Bbox2& old_bounds,const Bbox2& new_bounds)
{
	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
	{
		IGISEntity * ge = GetNthEntity(i);
		// Big-ass hack: if a composite contains a taxi layout, it will tell all edges AND all nodes to move,
		// and the edes will tell the nodes to move (once for each viewer) and we get a borked move.  So...
		// just don't pass msg to edges.
		//
		// This breaks down if the taxi layout is PARTLY grouped...that's something I am going to punt on for now.
		if(ge->GetGISClass() != gis_Edge)
			ge->Rescale(l, old_bounds,new_bounds);
	}
}

void			WED_GISComposite::Rotate(GISLayer_t l, const Point2& ctr, double angle)
{
	int n = GetNumEntities();
	for (int i = 0; i < n; ++i)
	{
		IGISEntity * ge = GetNthEntity(i);
		if(ge->GetGISClass() != gis_Edge)	
			ge->Rotate(l, ctr, angle);
	}
}

int				WED_GISComposite::GetNumEntities(void ) const
{
	RebuildCache(CacheBuild(cache_Topological));
	return mEntities.size();
}

IGISEntity *	WED_GISComposite::GetNthEntity  (int n) const
{
	RebuildCache(CacheBuild(cache_Topological));
	return mEntities[n];
}


void	WED_GISComposite::RebuildCache(int flags) const
{
	if(flags & cache_Topological)
	{
		mEntities.clear();
		int n = CountChildren();
		mHasUV = (n > 0);
		mEntities.reserve(n);
		for (int i = 0; i <  n; ++i)
		{
			IGISEntity * ent = dynamic_cast<IGISEntity *>(GetNthChild(i));
			if (ent)
			{
				if(mHasUV && !ent->HasLayer(gis_UV))
					mHasUV = false;
				mEntities.push_back(ent);
			}
		}
	}

	if(flags & cache_Spatial)
	{
		mCacheBounds = Bbox2();
		mCacheBoundsUV = Bbox2();
		mCacheVisibleBounds = Bbox3();
		int n = mEntities.size();
		for (int i = 0; i <  n; ++i)
		{
			IGISEntity * ent = mEntities[i];
			if (ent)
			{
				Bbox2 child;
				ent->GetBounds(gis_Geo,child);
				mCacheBounds += child;
				if(mHasUV && ent->HasLayer(gis_UV))
				{
					ent->GetBounds(gis_UV,child);
					mCacheBoundsUV += child;
				} else
					mHasUV = false;
				mCacheVisibleBounds += ent->GetVisibleBounds();
			}
		}
	}		
}
