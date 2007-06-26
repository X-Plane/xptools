#ifndef WED_RAMPPOSITION_H
#define WED_RAMPPOSITION_H

#include "WED_GISPoint_Heading.h"

struct	AptGate_t;

class	WED_RampPosition : public WED_GISPoint_Heading {

DECLARE_PERSISTENT(WED_RampPosition)

public:

	void	Import(const AptGate_t& x, void (* print_func)(void *, const char *, ...), void * ref);
	void	Export(		 AptGate_t& x) const;

};

#endif /* WED_RAMPPOSITION_H */