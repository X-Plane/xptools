#ifndef WED_WINDSOCK_H
#define WED_WINDSOCK_H

#include "WED_GISPoint.h"

class	WED_Windsock : public WED_GISPoint {

DECLARE_PERSISTENT(WED_Windsock)

public:

		void		SetLit(int);

private:

	WED_PropBoolText			lit;
	
};	

#endif /* WED_WINDSOCK_H */
