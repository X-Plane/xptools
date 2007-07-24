#include "WED_Entity.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

WED_Entity::WED_Entity(WED_Archive * parent, int id) :
	WED_Thing(parent, id),
	locked(this,"Locked","WED_entities","locked",0),
	hidden(this,"Hidden","WED_entities","hidden",0),
	cache_valid(false)
{
}

WED_Entity::~WED_Entity()
{
}

void WED_Entity::CopyFrom(const WED_Entity * rhs)
{
	WED_Thing::CopyFrom(rhs);
	CacheInval();
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

void 	WED_Entity::ReadFrom(IOReader * reader)
{
	CacheInval();
	WED_Thing::ReadFrom(reader);
}

void	WED_Entity::FromDB(sqlite3 * db, const map<int,int>& mapping)
{
	CacheInval();
	WED_Thing::FromDB(db,mapping);
}

void	WED_Entity::CacheInval(void)
{
	// BEN SAYS: theoretically speaking, if we are invalid, we MUST have invalidated our
	// parents previously.  So...do not run up the parent chain if we are already invalid,
	// to prevent NlogN time access to N operations on entities.
	if (cache_valid)
	{
		cache_valid = false;
		WED_Thing *  p = GetParent();
		if (p)
		{
			WED_Entity * e = dynamic_cast<WED_Entity *>(p);
			if (e)
				e->CacheInval();
		}	
	}
}

bool	WED_Entity::CacheBuild(void) const
{
	if (cache_valid)	return false;
	cache_valid = true;	return true ;
}

void	WED_Entity::AddChild(int id, int n)
{
	CacheInval();
	WED_Thing::AddChild(id, n);	
}

void	WED_Entity::RemoveChild(int id)
{
	CacheInval();
	WED_Thing::RemoveChild(id);
}

