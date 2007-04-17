#ifndef WED_AIRPORTSIGN_H
#define WED_AIRPORTSIGN_H

#include "WED_GISPoint_Heading.h"

class WED_AirportSign : public WED_GISPoint_Heading {

DECLARE_PERSISTENT(WED_AirportSign)

private:

	WED_PropIntEnum		style;
	WED_PropIntEnum		height;

};

#endif /* WED_AIRPORTSIGN_H */
