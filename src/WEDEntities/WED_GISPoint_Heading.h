#ifndef WED_GISPOINT_HEADING_H
#define WED_GISPOINT_HEADING_H

#include "IGIS.h"
#include "WED_GISPoint.h"

class	WED_GISPoint_Heading : public WED_GISPoint, public virtual IGISPoint_Heading {

DECLARE_INTERMEDIATE(WED_GISPoint_Heading)

public:

	virtual	GISClass_t		GetGISClass		(void				 ) const;

	virtual	double	GetHeading(void			 ) const;
	virtual	void	SetHeading(double heading)      ;

	virtual	void	Rotate			(const Point2& center, double angle);

private:

		WED_PropDoubleText		heading;

};

#endif
