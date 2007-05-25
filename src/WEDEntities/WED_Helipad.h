#ifndef WED_HELIPAD_H
#define WED_HELIPAD_H

#include "WED_GISPoint_HeadingWidthLength.h"

struct	AptHelipad_t;

class	WED_Helipad : public WED_GISPoint_HeadingWidthLength {

DECLARE_PERSISTENT(WED_Helipad)

public:

		void		SetSurface(int);
		void		SetMarkings(int);
		void		SetShoulder(int);
		void		SetRoughness(double);
		void		SetEdgeLights(int);

		int			GetSurface(void) const;
		
	void	Import(const AptHelipad_t& x);
	void	Export(		 AptHelipad_t& x) const;
			
private:

	WED_PropIntEnum		surface;
	WED_PropIntEnum		markings;
	WED_PropIntEnum		shoulder;
	WED_PropDoubleText	roughness;
	WED_PropIntEnum		edgelights;

};

#endif
