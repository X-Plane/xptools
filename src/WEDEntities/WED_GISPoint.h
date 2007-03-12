#ifndef WED_GISPOINT_H
#define WED_GISPOINT_H

#include "WED_Entity.h"
#include "IGIS.h"

class	WED_GISPoint : public WED_Entity, public virtual IGISPoint { 

DECLARE_INTERMEDIATE(WED_GISPoint)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual	void			GetBounds		(	   Bbox2&  bounds) const;
//	virtual	int				IntersectsBox	(const Bbox2&  bounds) const;
	virtual	bool			WithinBox		(const Bbox2&  bounds) const;
	virtual bool			PtWithin		(const Point2& p	 ) const;
	virtual bool			PtOnFrame		(const Point2& p, double dist) const;
	virtual	void			Rescale			(const Bbox2& old_bounds, const Bbox2& new_bounds);	
	// IGISPoint
	virtual	void	GetLocation(      Point2& p) const;
	virtual	void	SetLocation(const Point2& p)      ;	
	
private:

		WED_PropDoubleText		latitude;
		WED_PropDoubleText		longitude;

};		

#endif /* WED_GISPOINT_H */
