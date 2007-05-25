#ifndef WED_WINDSOCK_H
#define WED_WINDSOCK_H

#include "WED_GISPoint.h"

struct	AptWindsock_t;

class	WED_Windsock : public WED_GISPoint {

DECLARE_PERSISTENT(WED_Windsock)

public:

		void		SetLit(int);

		void		Import(const AptWindsock_t& x);
		void		Export(		 AptWindsock_t& x) const;

private:

	WED_PropBoolText			lit;
	
};	

#endif /* WED_WINDSOCK_H */
