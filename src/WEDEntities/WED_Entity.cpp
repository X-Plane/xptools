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

#include "WED_Entity.h"
#include "IODefs.h"
#include "WED_Errors.h"

WED_Entity::WED_Entity(WED_Archive * parent, int id) :
	WED_Thing(parent, id),
	locked(this,"Locked", XML_Name("hierarchy","locked"),0),
	hidden(this,"Hidden", XML_Name("hierarchy","hidden"),0),
	cache_valid_(0)
{
}

WED_Entity::~WED_Entity()
{
}

void WED_Entity::CopyFrom(const WED_Entity * rhs)
{
	WED_Thing::CopyFrom(rhs);
	cache_valid_ &= ~cache_All;
}

int		WED_Entity::GetLocked(void) const
{
	return locked.value;
}

int		WED_Entity::GetHidden(void) const
{
	return hidden.value;
}

// Read from DB or undo mem - in both cases, mark our cache as invalid...the real core data has probably been
// splatted.

bool 	WED_Entity::ReadFrom(IOReader * reader)
{
	WED_Thing::ReadFrom(reader);
	return true;
}

void	WED_Entity::PostChangeNotify(void)
{
	CacheInval(cache_All);
}

void	WED_Entity::CacheInval(int flags)
{
	// BEN SAYS: theoretically speaking, if we are invalid, we MUST have invalidated our
	// parents previously.  So...do not run up the parent chain if we are already invalid,
	// to prevent NlogN time access to N operations on entities.
	int new_invals = flags & cache_valid_;
	
	if (new_invals)
	{
		cache_valid_ &= ~new_invals;
		WED_Thing *  p = GetParent();
		if (p)
		{
			WED_Entity * e = dynamic_cast<WED_Entity *>(p);
			if (e)
				e->CacheInval(new_invals);
		}
		set<WED_Thing *>	viewers;
		GetAllViewers(viewers);
		for(set<WED_Thing *>::iterator v=  viewers.begin(); v != viewers.end(); ++v)
		{
			WED_Entity * e = dynamic_cast<WED_Entity *>(*v);
			if(e)
				e->CacheInval(new_invals);
		}		
	}
}

int	WED_Entity::CacheBuild(int flags) const
{
	int needed_flags = flags & ~cache_valid_;
	cache_valid_ |= needed_flags;
	return needed_flags;
}

void	WED_Entity::AddChild(int id, int n)
{
	CacheInval(cache_All);
	WED_Thing::AddChild(id, n);
}

void	WED_Entity::RemoveChild(int id)
{
	CacheInval(cache_All);
	WED_Thing::RemoveChild(id);
}

void	WED_Entity::AddViewer(int id)
{
	WED_Thing::AddViewer(id);
	CacheInval(cache_All);
	// FIRST we add the viewer.  Then by inval our own topo, we inval our viewer's topo which is what we REALLY wanted to do.
	// Override of AddSource might have been less weird.
}

void	WED_Entity::RemoveViewer(int id)
{
	WED_Thing::RemoveViewer(id);
	CacheInval(cache_All);
}
