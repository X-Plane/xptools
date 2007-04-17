#ifndef WED_GISPOLYGON_H
#define WED_GISPOLYGON_H

#include "WED_Entity.h"
#include "IGIS.h"

class	WED_GISPolygon : public WED_Entity, public virtual IGISPolygon {

DECLARE_INTERMEDIATE(WED_GISPolygon)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual	void			GetBounds		(	   Bbox2&  bounds) const;
//	virtual	int				IntersectsBox	(const Bbox2&  bounds) const;
	virtual	bool			WithinBox		(const Bbox2&  bounds) const;
	virtual bool			PtWithin		(const Point2& p	 ) const;
	virtual bool			PtOnFrame		(const Point2& p, double d) const;
	virtual	void			Rescale(const Bbox2& old_bounds,const Bbox2& new_bounds);
	// IGISPolygon
	virtual			IGISPointSequence *		GetOuterRing(void )	const;	
	virtual			int						GetNumHoles (void ) const;
	virtual			IGISPointSequence *		GetNthHole  (int n)	const;

	virtual			void					DeleteHole  (int n)					;
	virtual			void					AddHole		(IGISPointSequence * r) ;

};

#endif /* WED_GISPOLYGON_H */
