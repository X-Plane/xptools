#ifndef WED_SEALANE_H
#define WED_SEALANE_H

#include "WED_GISLine_Width.h"

class	WED_Sealane : public WED_GISLine_Width {

DECLARE_PERSISTENT(WED_Sealane)

private:

	WED_PropBoolText		buoys;
	WED_PropStringText		id1;
	WED_PropStringText		id2;

};	

#endif /* WED_SEALANE_H */
