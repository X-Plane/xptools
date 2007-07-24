#ifndef WED_GISPOINT_BEZIER_H
#define WED_GISPOINT_BEZIER_H

#include "WED_GISPoint.h"

class	WED_GISPoint_Bezier : public WED_GISPoint, public virtual IGISPoint_Bezier {

DECLARE_INTERMEDIATE(WED_GISPoint_Bezier)

public:

	// IGISEntity
	virtual	GISClass_t		GetGISClass		(void				 ) const;
	virtual	void			Rescale			(const Bbox2& old_bounds, const Bbox2& new_bounds);	
	virtual	void			Rotate			(const Point2& center, double angle);
	// IGISPoint_Bezier
	
	virtual	bool	GetControlHandleLo (      Point2& p) const;
	virtual	bool	GetControlHandleHi (      Point2& p) const;
	virtual	bool	IsSplit			   (void		   ) const;

	virtual	void	SetControlHandleLo (const Point2& p)      ;
	virtual	void	SetControlHandleHi (const Point2& p)      ;
	virtual	void	DeleteHandleLo	   (void		   )	  ;
	virtual	void	DeleteHandleHi	   (void		   )	  ;
	virtual	void	SetSplit		   (bool is_split  )	  ;
	virtual	void	Reverse			   (void		   )	  ;

private:

		WED_PropBoolText		is_split;

		WED_PropDoubleText		ctrl_lat_lo;			// NOTE: THESE ARE STORED AS DELTAS!!!
		WED_PropDoubleText		ctrl_lon_lo;
		WED_PropDoubleText		ctrl_lat_hi;
		WED_PropDoubleText		ctrl_lon_hi;

};

#endif /* WED_GISPOINT_BEZIER_H */