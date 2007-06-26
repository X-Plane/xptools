#ifndef WED_AIRPORT_H
#define WED_AIRPORT_H

#include "WED_GISComposite.h"

struct	AptInfo_t;

class	WED_Airport : public WED_GISComposite {

DECLARE_PERSISTENT(WED_Airport)

public:

	void		GetICAO(string& icao) const;
	int			GetAirportType(void) const;

	void		SetAirportType(int airport_type);
	void		SetElevation(double elev);
	void		SetHasATC(int has_atc);
	void		SetICAO(const string& icao);

	void		Import(const AptInfo_t& info, void (* print_func)(void *, const char *, ...), void * ref);
	void		Export(		 AptInfo_t& info) const;

private:

	WED_PropIntEnum			airport_type;
	WED_PropDoubleText		elevation;
	WED_PropBoolText		has_atc;
	WED_PropStringText		icao;
	
};


#endif /* WED_AIRPORT_H */
