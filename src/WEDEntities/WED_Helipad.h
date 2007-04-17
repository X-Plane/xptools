#ifndef WED_HELIPAD_H
#define WED_HELIPAD_H

#include "WED_GISPoint_HeadingWidthLength.h"

class	WED_Helipad : public WED_GISPoint_HeadingWidthLength {

DECLARE_PERSISTENT(WED_Helipad)

private:

	WED_PropIntEnum		surface;
	WED_PropIntEnum		markings;
	WED_PropIntEnum		shoulder;
	WED_PropDoubleText	roughness;
	WED_PropIntEnum		edgelights;

};

#endif