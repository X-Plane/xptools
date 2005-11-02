#ifndef HYDRO_H
#define HYDRO_H

class	Pmwx;
class	DEMGeoMap;
class	GISHalfedge;
class	GISFace;
class	Bbox2;

#include "ProgressUtils.h"

// Fully rebuild a map based on elevation and other DEM params.
void	HydroReconstruct(Pmwx& ioMap, DEMGeoMap& ioDem, const char * mask_file,ProgressFunc inFunc);

// Simplify the coastlines of a complex map based on certain areas to cover.
void	SimplifyCoastlines(Pmwx& ioMap, const Bbox2& bounds, ProgressFunc inFunc);

// For testing
//void	SimplifyCoastlineFace(Pmwx& ioMap, GISFace * face);

bool	MakeWetMask(const char * inShapeDir, int lon, int lat, const char * inMaskDir);

#endif
