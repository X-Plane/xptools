#ifndef WED_ENTITY_H
#define WED_ENTITY_H

/*
	WED_Entity - THEROY OF OPERATION
	
	WED_Entity provides the implementation base for all spatial WED_thigns, that is, everything you can see on the map.
	Generally:
	
	Any final derivative of WED_Entity can be caste to IGISEntity.  (But note that since WED_Entity itself doesn't
	do this, this support is not automatic.)
	
	All children of WED_Entities are WED_Entities, that is, we don't have random stuff jammed into the map.
	
	This class provides the "locked" and "hidden" property provided to anything on the map.
	
*/

#include "WED_Thing.h"

class	WED_Entity : public WED_Thing { 

DECLARE_INTERMEDIATE(WED_Entity)

public:

		int		GetLocked(void) const;
		int		GetHidden(void) const;
private:

	WED_PropBoolText			locked;
	WED_PropBoolText			hidden;

};
	

#endif /* WED_ENTITY_H */
