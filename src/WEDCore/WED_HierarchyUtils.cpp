#include "WED_HierarchyUtils.h"
#include "WED_Entity.h"

bool IgnoreVisiblity(WED_Thing* v)
{
	return true;
}

bool EntityNotHidden(WED_Thing* v)
{
	DebugAssert(v != NULL);
	return !static_cast<WED_Entity*>(v)->GetHidden();
}

bool ThingNotHidden(WED_Thing * t)
{
	WED_Entity* ent = dynamic_cast<WED_Entity*>(t);
	if (ent != NULL)
	{
		return !ent->GetHidden();
	}
	else
	{
		return true;
	}
}

bool TakeAlways(WED_Thing* v)
{
	return true;
}