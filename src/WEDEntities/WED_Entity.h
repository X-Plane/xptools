#ifndef WED_ENTITY_H
#define WED_ENTITY_H

#include "WED_Thing.h"

class	WED_Entity : public WED_Thing { 

DECLARE_INTERMEDIATE(WED_Entity)
	
private:

	WED_PropBoolText			locked;
	WED_PropBoolText			hidden;

};
	

#endif /* WED_ENTITY_H */
