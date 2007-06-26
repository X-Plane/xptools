#ifndef WED_TOWERVIEWPOINT_H
#define WED_TOWERVIEWPOINT_H

#include "WED_GISPoint.h"

struct	AptTowerPt_t;

class	WED_TowerViewpoint : public WED_GISPoint {

DECLARE_PERSISTENT(WED_TowerViewpoint)

public:

	void		SetHeight(double);
	
	void		Import(const AptTowerPt_t& x, void (* print_func)(void *, const char *, ...), void * ref);
	void		Export(		 AptTowerPt_t& x) const;

private:

	WED_PropDoubleText		height;
	
};

#endif /* WED_TOWERVIEWPOINT_H */
