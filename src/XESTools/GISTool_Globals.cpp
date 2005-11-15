#include "GISTool_Globals.h"

#include "MapDefs.h"
#include "DEMDefs.h"
#include "MeshDefs.h"

Pmwx				gMap;
DEMGeoMap			gDem;
//CDT					gTriangulationLo;
CDT					gTriangulationHi;

vector<Point2>		gMeshPoints;
vector<Point2>		gMeshLines;

bool				gVerbose = true;
bool				gTiming = false;
ProgressFunc		gProgress = ConsoleProgressFunc;

int					gMapWest  = -180;
int					gMapSouth = -90;
int					gMapEast  =  180;
int					gMapNorth =  90;

AptVector			gApts;
AptIndex				gAptIndex;
