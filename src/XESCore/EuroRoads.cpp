#include "EuroRoads.h"
#include "MapDefs.h"
#include "DEMDefs.h"
#include "DemToVector.h"
#include "DEMAlgs.h"
#include "perlin.h"
#include "ParamDefs.h"

#include "MapAlgs.h"

#define DEBUG_SHOW_AREAS 0

#define	MAX_BLOB 15.0

void	AddEuroRoads(
				Pmwx& 			ioBase,
				Pmwx& 			ioRoadSrc,
				const DEMGeo&	inSlope,
				const DEMGeo&	inUrban,
				int				inLU,
				ProgressFunc	inFunc)
{
	int x, y;
	
	Pmwx	road_area;
	
	DEMGeo	matches(inSlope.mWidth, inSlope.mHeight);
	matches.copy_geo(inSlope);

	for (y = 0; y < inSlope.mHeight; ++y)
	for (x = 0; x < inSlope.mWidth ; ++x)
	{
		if (inUrban.xy_nearest(inSlope.x_to_lon(x), inSlope.y_to_lat(y)) == inLU)
		{
			matches(x,y) = 1.0;
		} else {
			matches(x,y) = NO_DATA;
		}
	}
	
	DEMGeo	matches_orig(matches);
	
	for (y = 0; y < inSlope.mHeight; ++y)
	for (x = 0; x < inSlope.mWidth ; ++x)
	{
		int d = matches_orig.radial_dist(x, y, MAX_BLOB, 1.0);
		if (d != -1)
		{
			double r = (double) d / MAX_BLOB;
			
			float p = perlin_2d((double) x / 20.0, (double) y / 20.0, 1, 5, 0.5, 120);
			if (p > r)
			{
				matches(x,y) = 1.0;
			}
		}
	}

	for (y = 0; y < inSlope.mHeight; ++y)
	for (x = 0; x < inSlope.mWidth ; ++x)
	if (matches.get(x,y) == 1.0)
	if (inSlope.get(x,y) > 0.06)
		matches(x,y) = NO_DATA;

	DEMGeo	foo;
	InterpDoubleDEM(matches, foo);
	ReduceToBorder(foo,matches );
	
	DemToVector(matches, road_area, false, terrain_Marker_Features, inFunc);

	set<GISFace *>	faces;

	TopoIntegrateMaps(&ioBase, &road_area);
	MergeMaps(ioBase, road_area, 
			false, 		// Don't force props
			&faces, 		// Don't return face set
			true);		// pre integrated
			

	for (set<GISFace *>::iterator face = faces.begin(); face != faces.end(); ++face)
	{
		if ((*face)->mTerrainType == terrain_Marker_Features)
		{
#if !DEBUG_SHOW_AREAS		
			(*face)->mTerrainType = terrain_Natural;
			SwapFace(ioBase, ioRoadSrc, *face, NULL);
#endif			
		}
	}
	
	SimplifyMap(ioBase);
}
