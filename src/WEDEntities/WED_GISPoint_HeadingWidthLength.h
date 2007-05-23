#ifndef WED_GISPOINT_HEADINGWIDTHLENGTH_H
#define WED_GISPOINT_HEADINGWIDTHLENGTH_H

#include "WEd_GISPoint_Heading.h"

class	WED_GISPoint_HeadingWidthLength : public WED_GISPoint_Heading, public virtual IGISPoint_WidthLength {

DECLARE_INTERMEDIATE(WED_GISPoint_HeadingWidthLength)

public:

	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual bool			PtWithin		(const Point2& p	 ) const;

	// IGISPoint_WidthLength
	virtual	double	GetWidth (void		 ) const;
	virtual	void	SetWidth (double width)      ;
	virtual	double	GetLength(void		 ) const;
	virtual	void	SetLength(double width)      ;

	virtual	void	GetCorners(Point2 corners[4]) const;

private:

		WED_PropDoubleText		width;
		WED_PropDoubleText		length;

};

#endif /* WED_GISPOINT_HEADINGWIDTHLENGTH_H */
