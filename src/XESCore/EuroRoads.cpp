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
	matches.copy_geo_from(inSlope);

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
			true,
			inFunc);		// pre integrated
			

	for (set<GISFace *>::iterator face = faces.begin(); face != faces.end(); ++face)
	{
		if ((*face)->mTerrainType == terrain_Marker_Features)
		{
#if !DEBUG_SHOW_AREAS		
			(*face)->mTerrainType = terrain_Natural;

// BEN SAYS:
///	This was an attempt to copy a small section and reuse it.  It does NOT work becaus ethe copy cost is way more expensive than other factors in this alg.
/*			
			Pmwx	roadCopy(ioRoadSrc);
			Bbox2	box;
			Pmwx::Ccb_halfedge_circulator	circ = (*face)->outer_ccb();
			Pmwx::Ccb_halfedge_circulator	start = circ;
			box = Bbox2(circ->source()->point());
			do {
				box += circ->source()->point();
				++circ;
			} while (circ != start);
			
			Vector2	delta(box.p1);
			
			for (Pmwx::Vertex_iterator v = roadCopy.vertices_begin(); v != roadCopy.vertices_end(); ++v)
			{
				roadCopy.UnindexVertex(v);
				v->point() += delta;
				roadCopy.ReindexVertex(v);
			}		*/
			
			SwapFace(ioBase, ioRoadSrc, *face, NULL);
#endif			
		}
	}

	SimplifyMap(ioBase, false);
}
