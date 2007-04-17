#ifndef WED_TOWERVIEWPOINT_H
#define WED_TOWERVIEWPOINT_H

#include "WED_GISPoint.h"

class	WED_TowerViewpoint : public WED_GISPoint {

DECLARE_PERSISTENT(WED_TowerViewpoint)

private:

	WED_PropDoubleText		height;
	
};

#endif /* WED_TOWERVIEWPOINT_H */
