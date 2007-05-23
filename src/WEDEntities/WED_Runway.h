#ifndef WED_RUNWAY_H
#define WED_RUNWAY_H

#include "WED_GISLine_Width.h"

class	WED_Runway : public WED_GISLine_Width {

DECLARE_PERSISTENT(WED_Runway)

public:

	virtual	bool			PtWithin		(const Point2& p	 ) const;


	// These routines return a rectangle for the given sub-rect of the runway.  Like all rects
	// they are clockwise, with the first point on the left side of the runway (looking from the low
	// to high end) at the low end.
	
	// These routines return false if these elements aren't there.
	bool		GetCornersBlas1(Point2 corners[4]) const;
	bool		GetCornersBlas2(Point2 corners[4]) const;
	bool		GetCornersDisp1(Point2 corners[4]) const;
	bool		GetCornersDisp2(Point2 corners[4]) const;
	bool		GetCornersShoulders(Point2 corners[8]) const;

	void		SetSurface(int);
	void		SetShoulder(int);
	void		SetRoughness(double);
	void		SetCenterLights(int);
	void		SetEdgeLights(int);
	void		SetRemainingSigns(int);
	void		SetMarkings1(int);
	void		SetAppLights1(int);
	void		SetTDZL1(int);
	void		SetREIL1(int);
	void		SetMarkings2(int);
	void		SetAppLights2(int);
	void		SetTDZL2(int);
	void		SetREIL2(int);


	double		GetDisp1(void) const;
	double		GetDisp2(void) const;
	double		GetBlas1(void) const;
	double		GetBlas2(void) const;

	void		SetDisp1(double disp1);
	void		SetDisp2(double disp2);
	void		SetBlas1(double blas1);
	void		SetBlas2(double blas2);

private:

	WED_PropIntEnum			surface;
	WED_PropIntEnum			shoulder;
	WED_PropDoubleText		roughness;
	WED_PropBoolText		center_lites;
	WED_PropIntEnum			edge_lites;
	WED_PropBoolText		remaining_signs;
	
	WED_PropStringText		id1;
	WED_PropDoubleText		disp1;
	WED_PropDoubleText		blas1;
	WED_PropIntEnum			mark1;
	WED_PropIntEnum			appl1;
	WED_PropBoolText		tdzl1;
	WED_PropIntEnum			reil1;

	WED_PropStringText		id2;
	WED_PropDoubleText		disp2;
	WED_PropDoubleText		blas2;
	WED_PropIntEnum			mark2;
	WED_PropIntEnum			appl2;
	WED_PropBoolText		tdzl2;
	WED_PropIntEnum			reil2;	

};

#endif /* WED_RUNWAY_H */