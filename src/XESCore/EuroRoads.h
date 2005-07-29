#ifndef EUROROADS_H
#define EUROROADS_H

#include "ProgressUtils.h"

class	Pmwx;
struct	DEMGeo;

void	AddEuroRoads(
				Pmwx& 			ioBase,
				Pmwx& 			ioRdSrc,
				const DEMGeo&	inSlope, 
				const DEMGeo&	inUrban,
				int				inLU,
				ProgressFunc	inProg);

#endif