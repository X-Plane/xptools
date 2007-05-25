#ifndef WED_SEALANE_H
#define WED_SEALANE_H

#include "WED_GISLine_Width.h"

struct	AptSealane_t;

class	WED_Sealane : public WED_GISLine_Width {

DECLARE_PERSISTENT(WED_Sealane)

public:

	void		SetBuoys(int);
	
	void		Import(const AptSealane_t& x);
	void		Export(		 AptSealane_t& x) const;

private:

	WED_PropBoolText		buoys;

};	

#endif /* WED_SEALANE_H */
