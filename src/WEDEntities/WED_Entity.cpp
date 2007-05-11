#include "WED_Entity.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

WED_Entity::WED_Entity(WED_Archive * parent, int id) :
	WED_Thing(parent, id),
	locked(this,"Locked","WED_entities","locked",0),
	hidden(this,"Hidden","WED_entities","hidden",0)
{
}

WED_Entity::~WED_Entity()
{
}

int		WED_Entity::GetLocked(void) const
{
	return locked.value;
}

int		WED_Entity::GetHidden(void) const
{
	return hidden.value;
}
