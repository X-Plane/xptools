#ifndef WED_LIGHTFIXTURE_H
#define WED_LIGHTFIXTURE_H

#include "WED_GISPoint_Heading.h"

class WED_LightFixture : public WED_GISPoint_Heading {

DECLARE_PERSISTENT(WED_LightFixture)

public:

		void		SetLightType(int);
		void		SetAngle(double);

private:

	WED_PropIntEnum		light_type;
	WED_PropDoubleText	angle;

};

#endif /* WED_LIGHTFIXTURE_H */

