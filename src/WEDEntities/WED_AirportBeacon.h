#ifndef WED_AIRPORTBEACON_H
#define WED_AIRPORTBEACON_H

#include "WED_GISPoint.h"

struct AptBeacon_t;

class	WED_AirportBeacon : public WED_GISPoint {

DECLARE_PERSISTENT(WED_AirportBeacon)

public:

	void	Import(const AptBeacon_t& x, void (* print_func)(void *, const char *, ...), void * ref);
	void	Export(		 AptBeacon_t& x) const;

	void			SetKind(int kind);

private:

	WED_PropIntEnum			kind;
	
};

#endif

