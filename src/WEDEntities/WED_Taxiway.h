#ifndef WED_TAXIWAY_H
#define WED_TAXIWAY_H

#include "WED_GISPolygon.h"

struct	AptTaxiway_t;

class	WED_Taxiway : public WED_GISPolygon {

DECLARE_PERSISTENT(WED_Taxiway)
public:

	void		SetSurface(int s);
	void		SetRoughness(double r);
	void		SetHeading(double h);
	double		GetHeading(void) const;

	int			GetSurface(void) const;
	
	void		Import(const AptTaxiway_t& x, void (* print_func)(void *, const char *, ...), void * ref);
	void		Export(		 AptTaxiway_t& x) const;
	
private:

	WED_PropIntEnum			surface;
	WED_PropDoubleText		roughness;
	WED_PropDoubleText		heading;
	
	WED_PropIntEnumSetUnion	lines;
	WED_PropIntEnumSetUnion	lights;
		
};

#endif /* WED_TAXIWAY_H */
