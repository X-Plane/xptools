#ifndef WED_AIRPORTSIGN_H
#define WED_AIRPORTSIGN_H

#include "WED_GISPoint_Heading.h"

struct	AptSign_t;

class WED_AirportSign : public WED_GISPoint_Heading {

DECLARE_PERSISTENT(WED_AirportSign)

public:

		void		SetStyle(int style);
		void		SetHeight(int height);

		void		Import(const AptSign_t& x);
		void		Export(		 AptSign_t& x) const;

private:

	WED_PropIntEnum		style;
	WED_PropIntEnum		height;

};

#endif /* WED_AIRPORTSIGN_H */
