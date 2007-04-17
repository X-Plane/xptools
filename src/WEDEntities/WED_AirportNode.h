#ifndef WED_AIRPORTNODE_H
#define WED_AIRPORTNODE_H

#include "WED_GISPoint_Bezier.h"

class	WED_AirportNode : public WED_GISPoint_Bezier {

DECLARE_PERSISTENT(WED_AirportNode)

private:

	WED_PropIntEnumSet			attrs;
	
};

#endif /* WED_AIRPORTNODE_H */