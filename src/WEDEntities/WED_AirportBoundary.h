#ifndef WED_AIRPORTBOUNDARY_H
#define WED_AIRPORTBOUNDARY_H

#include "WED_GISPolygon.h"

struct	AptBoundary_t;

class	WED_AirportBoundary : public WED_GISPolygon {

DECLARE_PERSISTENT(WED_AirportBoundary)

public:
	
		void	Import(const AptBoundary_t& x);
		void	Export(		 AptBoundary_t& x) const;

};

#endif /* WED_AIRPORTBOUNDARY_H */