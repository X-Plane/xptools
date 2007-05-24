#ifndef WED_AIRPORT_H
#define WED_AIRPORT_H

#include "WED_GISComposite.h"

class	WED_Airport : public WED_GISComposite {

DECLARE_PERSISTENT(WED_Airport)

public:

	void		GetICAO(string& icao) const;
	int			GetAirportType(void) const;

private:

	WED_PropIntEnum			airport_type;
	WED_PropBoolText		elevation;
	WED_PropBoolText		has_atc;
	WED_PropStringText		icao;
	
};


#endif /* WED_AIRPORT_H */
