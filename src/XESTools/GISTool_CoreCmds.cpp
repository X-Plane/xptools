/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "GISTool_CoreCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "MapTopology.h"
#include "MapOverlay.h"
#include "PerfUtils.h"
#include "MeshDefs.h"
#include "MapAlgs.h"
#include "XFileTwiddle.h"
#include "AptIO.h"
#include "DEMDefs.h"

#include "EnumSystem.h"
#include "FAA_Obs.h"
#include "XESIO.h"
#include "MemFileUtils.h"
#include "GISUtils.h"

#if OPENGL_MAP
#include "RF_Notify.h"
#include "RF_Msgs.h"
#endif

static int DoExtent(const vector<const char *>& args)
{
	gMapWest = atoi(args[0]);
	gMapSouth = atoi(args[1]);
	gMapEast = atoi(args[2]);
	gMapNorth = atoi(args[3]);
	if (gMapWest >= gMapEast ||
		gMapSouth >= gMapNorth ||
		 gMapWest < -180.0 ||
		 gMapEast > 180.0 ||
		 gMapSouth < -90.0 ||
		 gMapNorth > 90.0)
	{
		if (gVerbose)
			printf("Error: illegal bounds %d,%d -> %d, %d\n",
				gMapWest, gMapSouth, gMapEast, gMapNorth);
		return 1;
	}
	return 0;
}

static int DoBbox(const vector<const char *>& args)
{
	Point_2	sw, ne;
	CalcBoundingBox(gMap, sw, ne);
	printf("SW = %lf,%lf  NE = %lf, %lf\n", CGAL::to_double(sw.x()), CGAL::to_double(sw.y()), CGAL::to_double(ne.x()), CGAL::to_double(ne.y()));

	for (DEMGeoMap::iterator i = gDem.begin(); i != gDem.end(); ++i)
	{
		printf("DEM %s: SW = %lfx%lf, NE = %lfx%lf, %d by %d\n",
			FetchTokenString(i->first),
			i->second.mWest, i->second.mSouth, i->second.mEast, i->second.mNorth, i->second.mWidth, i->second.mHeight);
	}
	if (!gFAAObs.empty())
	{
		double	obs_lat_min =   90.0;
		double	obs_lat_max =  -90.0;
		double	obs_lon_min =  180.0;
		double	obs_lon_max = -180.0;
		for (FAAObsTable::iterator i = gFAAObs.begin(); i != gFAAObs.end(); ++i)
		{
			obs_lat_min = min(obs_lat_min, i->second.lat);
			obs_lat_max = max(obs_lat_max, i->second.lat);
			obs_lon_min = min(obs_lon_min, i->second.lon);
			obs_lon_max = max(obs_lon_max, i->second.lon);
		}
		printf("Bounds for all objects are: %lf,%lf -> %lf, %lf\n",
			obs_lon_min, obs_lon_max, obs_lat_min, obs_lat_max);
	}
	return 0;
}

/*
static int DoNearest(const vector<const char *>& args)
{
	Point2	p1, p2;
	double dist = gMap.smallest_dist(p1, p2);
	double	dist_m = dist * DEG_TO_NM_LAT * NM_TO_MTR;
	printf("Smallest dist is: %lf meters\n", dist_m);
	printf("From %lf,%lf to %lf, %lf\n", p1.x, p1.y, p2.x, p2.y);
	return 1;
}
*/

static int DoWaterCount(const vector<const char *>& args)
{
	double total = 0.0;
	for (Pmwx::Face_iterator face = gMap.faces_begin(); face != gMap.faces_end(); ++face)
	if (!face->is_unbounded())
	if (face->data().IsWater())
	{
		Pmwx::Ccb_halfedge_circulator circ, stop;
		circ = stop = face->outer_ccb();
		do {
			total += CGAL::to_double(Vector_2(stop->source()->point(), circ->source()->point()) *
					  Vector_2(stop->source()->point(), circ->target()->point()).perpendicular(CGAL::COUNTERCLOCKWISE));
			++circ;
		} while (circ != stop);

		for (Pmwx::Hole_iterator hole = face->holes_begin(); hole != face->holes_end(); ++hole)
		{
			circ = stop = *hole;
			do {
				total += CGAL::to_double(Vector_2(stop->source()->point(), circ->source()->point()) *
						  Vector_2(stop->source()->point(), circ->target()->point()).perpendicular(CGAL::COUNTERCLOCKWISE));
				++circ;
			} while (circ != stop);
		}
	}
	printf("TOTAL WATER AREA = %lf\n", total);
	return 1;
}

static int DoCropGrid(const vector<const char *>& args)
{
	gMap.unbounded_face()->data().mTerrainType = terrain_Natural;

	CGAL::insert_curve(gMap,Curve_2(Segment_2(Point_2(gMapWest,gMapSouth),Point_2(gMapEast,gMapSouth))));
	CGAL::insert_curve(gMap,Curve_2(Segment_2(Point_2(gMapWest,gMapNorth),Point_2(gMapEast,gMapNorth))));
	CGAL::insert_curve(gMap,Curve_2(Segment_2(Point_2(gMapWest,gMapSouth),Point_2(gMapWest,gMapNorth))));
	CGAL::insert_curve(gMap,Curve_2(Segment_2(Point_2(gMapEast,gMapSouth),Point_2(gMapEast,gMapNorth))));

//	for (int x = sw.x; x <= ne.x; ++x)
//	{
//		gMap.insert_edge(Point2(x,sw.y),Point2(x,ne.y), NULL, NULL);
//	}
//	for (int y = sw.y; y <= ne.y; ++y)
//	{
//		gMap.insert_edge(Point2(sw.x,y),Point2(ne.x,y), NULL, NULL);
//	}
	return 0;
}

static int DoCrop(const vector<const char *>& args)
{
	if (gMap.number_of_halfedges() > 0)
		CropMap(gMap, gMapWest, gMapSouth, gMapEast, gMapNorth, false, gProgress);

	printf("Map contains: %d faces, %d half edges, %d vertices.\n",
		gMap.number_of_faces(),
		gMap.number_of_halfedges(),
		gMap.number_of_vertices());

	set<int>	nukable;
	for (DEMGeoMap::iterator i = gDem.begin(); i != gDem.end(); ++i)
	{
		if (i->second.mWest > gMapEast ||
			i->second.mEast < gMapWest ||
			i->second.mSouth > gMapNorth ||
			i->second.mNorth < gMapSouth)
		{
			nukable.insert(i->first);
		} else {
			if (i->second.mWest < gMapWest ||
				i->second.mEast > gMapEast ||
				i->second.mSouth < gMapSouth ||
				i->second.mNorth > gMapNorth)
			{
				DEMGeo	croppy;
				i->second.subset(croppy,
								 i->second.x_lower(gMapWest),
								 i->second.y_lower(gMapSouth),
								 i->second.x_upper(gMapEast),
								 i->second.y_upper(gMapNorth));
				i->second = croppy;
			}
		}
	}

	for(set<int>::iterator q = nukable.begin(); q != nukable.end(); ++q)
		gDem.erase(*q);

	if (!gFAAObs.empty())
	{
		FAAObsTable::iterator i, n;
		for (i = gFAAObs.begin(); i != gFAAObs.end(); )
		{
			n = i;
			++n;
			if (i->second.lon < gMapWest ||
				i->second.lat < gMapSouth ||
				i->second.lon > gMapEast ||
				i->second.lat > gMapNorth)
			{
				gFAAObs.erase(i);
			}
			i = n;
		}
	}
	return 0;
}

static int DoValidate(const vector<const char *>& args)
{
	bool	is_valid = gMap.is_valid();
	if (gVerbose)
		printf("Map %s valid.\n", is_valid ? "is" : "is not");
	if (!is_valid)
	{
		fprintf(stderr,"Validation check failed for map %d,%d -> %d,%d\n", gMapWest, gMapSouth, gMapEast, gMapNorth);
		return 1;
	}
	return 0;
}

static int DoLoad(const vector<const char *>& args)
{
	if (gVerbose) printf("Loading file %s...\n", args[0]);
	MFMemFile * load = MemFile_Open(args[0]);
	if (load)
	{
		ReadXESFile(load, &gMap, /*&gTriangulationHi*/NULL, &gDem, &gApts, gProgress);
		IndexAirports(gApts, gAptIndex);
		MemFile_Close(load);

	} else {
		fprintf(stderr,"Could not load file %s.\n", args[0]);
		return 1;
	}
	if (gVerbose)
			printf("Map contains: %d faces, %d half edges, %d vertices.\n",
				gMap.number_of_faces(),
				gMap.number_of_halfedges(),
				gMap.number_of_vertices());

#if OPENGL_MAP
	RF_Notifiable::Notify(rf_Cat_File, rf_Msg_FileLoaded, NULL);
#endif
	return 0;
}

static int DoOverlay(const vector<const char *>& args)
{
	if (gVerbose) printf("Overlaying file %s...\n", args[0]);
	MFMemFile * load = MemFile_Open(args[0]);
	Pmwx		theMap;
	if (load)
	{
		ReadXESFile(load, &theMap, NULL, NULL, NULL, gProgress);
		MemFile_Close(load);

	} else {
		fprintf(stderr,"Could not load file.\n");
		return 1;
	}
	if (gVerbose)
			printf("Map contains: %d faces, %d half edges, %d vertices.\n",
				theMap.number_of_faces(),
				theMap.number_of_halfedges(),
				theMap.number_of_vertices());

	RemoveUnboundedWater(theMap);
	if (gVerbose)
			printf("Without Water Map contains: %d faces, %d half edges, %d vertices.\n",
				theMap.number_of_faces(),
				theMap.number_of_halfedges(),
				theMap.number_of_vertices());

	OverlayMap_legacy(gMap, theMap);
	if (gVerbose)
			printf("Merged Map contains: %d faces, %d half edges, %d vertices.\n",
				gMap.number_of_faces(),
				gMap.number_of_halfedges(),
				gMap.number_of_vertices());
	return 0;
}

static int DoMerge(const vector<const char *>& args)
{
	if (gVerbose) printf("Merging file %s...\n", args[0]);
	MFMemFile * load = MemFile_Open(args[0]);
	Pmwx		theMap;
	if (load)
	{
		ReadXESFile(load, &theMap, NULL, NULL, NULL, gProgress);
		MemFile_Close(load);

	} else {
		fprintf(stderr,"Could not load file.\n");
		return 1;
	}
	if (gVerbose)
			printf("Map contains: %d faces, %d half edges, %d vertices.\n",
				theMap.number_of_faces(),
				theMap.number_of_halfedges(),
				theMap.number_of_vertices());

//	TopoIntegrateMaps(&gMap, &theMap);
	MergeMaps_legacy(gMap, theMap, false, NULL, true, gProgress);
	if (gVerbose)
			printf("Merged Map contains: %d faces, %d half edges, %d vertices.\n",
				gMap.number_of_faces(),
				gMap.number_of_halfedges(),
				gMap.number_of_vertices());
	return 0;
}



static int DoSave(const vector<const char *>& args)
{
	int nland = 0;
	for (Pmwx::Face_iterator f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
	{
		if (!f->data().IsWater())
			++nland;
		if (nland > 0)
			break;
	}
	if (!gDem.empty() || (nland > 0) || distance(gMap.unbounded_face()->holes_begin(),gMap.unbounded_face()->holes_end()) > 1)
	{
		if (gVerbose) printf("Saving file %s\n", args[0]);
		WriteXESFile(args[0], gMap, gTriangulationHi, gDem, gApts, gProgress);
		return 0;
	} else {
		printf("Not writing file %s - no DEMs and no land!\n", args[0]);
		fprintf(stderr, "Not writing file %s - no DEMs and no land!\n", args[0]);
		return 1;
	}
}

static int DoCropSave(const vector<const char *>& args)
{
	Point_2	sw, ne;
	CalcBoundingBox(gMap, sw, ne);


	for (int w = CGAL::to_double(sw.x()); w < CGAL::to_double(ne.x()); ++w)
	for (int s = CGAL::to_double(sw.y()); s < CGAL::to_double(ne.y()); ++s)
	{
		vector<Point_2>	pts;
		pts.push_back(Point_2(w  ,s  ));
		pts.push_back(Point_2(w+1,s  ));
		pts.push_back(Point_2(w+1,s+1));
		pts.push_back(Point_2(w  ,s+1));

		Pmwx	cutout;
		CropMap(gMap, cutout, pts, gProgress);

		SimplifyMap(cutout, false, gProgress);

		int nland = 0;
		for (Pmwx::Face_iterator f = cutout.faces_begin(); f != cutout.faces_end(); ++f)
		{
			if (!f->data().IsWater())
				++nland;
			if (nland > 0)
				break;
		}
		if (nland > 0)
		{
			char	fbuf[1024];
			sprintf(fbuf,"%s%+03d%+04d/", args[0], latlon_bucket(s), latlon_bucket(w));
			MakeDirExist(fbuf);
			sprintf(fbuf,"%s%+03d%+04d/%+03d%+04d.xes", args[0], latlon_bucket(s), latlon_bucket(w), s, w);
			if (gVerbose) printf("Saving file %s\n", fbuf);
			DEMGeoMap	dem;
			AptVector	apt;
			CDT			mesh;
			WriteXESFile(fbuf, cutout, mesh, dem, apt, gProgress);
		} else {
			printf("Not writing file %s - no DEMs and no land!\n", args[0]);
			fprintf(stderr, "Not writing file %s - no DEMs and no land!\n", args[0]);
		}
	}
	return 0;
}

static int DoTagOrigin(const vector<const char *>& args)
{
	float o = atof(args[0]);
	for(Pmwx::Face_iterator f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
		f->data().mParams[af_OriginCode] = o;
	if (gVerbose)
		printf("Set %d faces to have origin ode %f\n",gMap.number_of_faces(), o);
	return 0;
}

static int DoSimplify(const vector<const char *>& args)
{
	if (gVerbose)
		printf("Halfedges before simplify: %d\n", gMap.number_of_halfedges());
	SimplifyMap(gMap, false, gProgress);
	if (gVerbose)
		printf("Halfedges after simplify: %d\n", gMap.number_of_halfedges());
	return 0;
}

static	GISTool_RegCmd_t		sCoreCmds[] = {
{ "-crop", 			0, 0, DoCrop, 			"Crop the map and DEMs to the current extent.", "" },
{ "-cropgrid",		0, 0, DoCropGrid, 		"Crop the map along 1x1 degree grid lines.", "" },
{ "-bbox", 			0, 0, DoBbox, 			"Show bounds of all maps.", "" },
//{ "-nearest_dist",  0, 0, DoNearest, 		"Returns closest two pts on map.", "" },
{ "-water_count",	0, 0, DoWaterCount,		"Count amount of water in file.", "" },
{ "-extent", 		4, 4, DoExtent, 		"Set the bounds for further crop and import commands.", "" },
{ "-validate", 		0, 0, DoValidate, 		"Test vector map integrity.", "" },
{ "-load", 			1, 1, DoLoad, 			"Load an XES file.", "" },
{ "-save", 			1, 1, DoSave, 			"Save an XES file.", "" },
{ "-cropsave", 		1, 1, DoCropSave, 		"Save only extent as an XES file.", "" },
{ "-overlay", 		1, 1, DoOverlay, 		"Superimpose/replace a second vector map.", "" },
{ "-merge", 		1, 1, DoMerge,			"Superimpose/merge a second vector map.", "" },
{ "-simplify",		0, 0, DoSimplify,		"Remove unneeded vectors.", "" },
{ "-tag_origin",	1, 1, DoTagOrigin,		"Apply origin code X to this map.", "" },

{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterCoreCmds(void)
{
	GISTool_RegisterCommands(sCoreCmds);
}

