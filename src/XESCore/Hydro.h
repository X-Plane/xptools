#ifndef HYDRO_H
#define HYDRO_H

class	Pmwx;
class	DEMGeoMap;
class	GISHalfedge;

#include "ProgressUtils.h"

// Fully rebuild a map based on elevation and other DEM params.
void	HydroReconstruct(Pmwx& ioMap, DEMGeoMap& ioDem, const char * shapeFile, ProgressFunc inFunc);

// Simplify the coastlines of a complex map based on certain areas to cover.
void	SimplifyCoastlines(Pmwx& ioMap, double max_annex_area, ProgressFunc inFunc);


// for testing
void	SimplifyWaterCCB(Pmwx& ioMap, GISHalfedge * edge, bool is_outer_ccb);

#endif
