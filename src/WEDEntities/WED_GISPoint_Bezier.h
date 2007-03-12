#ifndef WED_GISPOINT_BEZIER_H
#define WED_GISPOINT_BEZIER_H

#include "WED_GISPoint.h"

class	WED_GISPoint_Bezier : public WED_GISPoint, public virtual IGISPoint_Bezier {

DECLARE_INTERMEDIATE(WED_GISPoint_Bezier)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual	void			Rescale			(const Bbox2& old_bounds, const Bbox2& new_bounds);	
	// IGISPoint_Bezier
	virtual	void	GetControlHandleLo (      Point2& p) const;
	virtual	void	SetControlHandleLo (const Point2& p)      ;
	virtual	void	GetControlHandleHi (      Point2& p) const;
	virtual	void	SetControlHandleHi (const Point2& p)      ;

private:

		WED_PropDoubleText		control_latitude;
		WED_PropDoubleText		control_longitude;

};

#endif /* WED_GISPOINT_BEZIER_H */