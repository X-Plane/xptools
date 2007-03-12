#include "WED_Entity.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

START_CASTING(WED_Entity)
INHERITS_FROM(WED_Thing)
END_CASTING


WED_Entity::WED_Entity(WED_Archive * parent, int id) :
	WED_Thing(parent, id),
	locked(this,"locked","WED_entities","locked",0),
	hidden(this,"hidden","WED_entities","hidden",0)
{
}

WED_Entity::~WED_Entity()
{
}

