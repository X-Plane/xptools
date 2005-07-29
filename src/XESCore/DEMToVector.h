#ifndef DEMTOVECTOR_H
#define DEMTOVECTOR_H

struct	DEMGeo;
class	Pmwx;
#include "ProgressUtils.h"

void DemToVector(DEMGeo& ioDEM, Pmwx& ioMap, bool doSmooth, int inPositiveTerrain, ProgressFunc inFunc);

#endif
