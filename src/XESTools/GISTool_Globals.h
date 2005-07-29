#ifndef GISTOOL_GLOBALS_H
#define GISTOOL_GLOBALS_H

#include "ProgressUtils.h"
#include "AptDefs.h"

class Pmwx;
class CDT;
class DEMGeoMap;

extern Pmwx					gMap;
extern DEMGeoMap			gDem;
//extern CDT					gTriangulationLo;
extern CDT					gTriangulationHi;

extern bool					gVerbose;
extern bool					gTiming;
extern ProgressFunc			gProgress;

extern	int					gMapWest;
extern	int					gMapSouth;
extern	int					gMapEast;
extern	int					gMapNorth;

extern AptVector			gApts;
extern AptIndex				gAptIndex;

#endif
