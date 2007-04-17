#ifndef WED_RUNWAY_H
#define WED_RUNWAY_H

#include "WED_GISLine_Width.h"

class	WED_Runway : public WED_GISLine_Width {

DECLARE_PERSISTENT(WED_Runway)

private:

	WED_PropIntEnum			surface;
	WED_PropIntEnum			shoulder;
	WED_PropDoubleText		roughness;
	WED_PropBoolText		center_lites;
	WED_PropIntEnum			edge_lites;
	WED_PropBoolText		remaining_signs;
	
	WED_PropStringText		id1;
	WED_PropDoubleText		disp1;
	WED_PropDoubleText		blas1;
	WED_PropIntEnum			mark1;
	WED_PropIntEnum			appl1;
	WED_PropBoolText		tdzl1;
	WED_PropIntEnum			reil1;

	WED_PropStringText		id2;
	WED_PropDoubleText		disp2;
	WED_PropDoubleText		blas2;
	WED_PropIntEnum			mark2;
	WED_PropIntEnum			appl2;
	WED_PropBoolText		tdzl2;
	WED_PropIntEnum			reil2;	

};

#endif /* WED_RUNWAY_H */