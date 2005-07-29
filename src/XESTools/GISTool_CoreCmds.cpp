#include "GISTool_CoreCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "PerfUtils.h"
#include "MapAlgs.h"
#include "XFileTwiddle.h"
#include "MeshDefs.h"
#include "AptIO.h"
#include "DEMDefs.h"
#include "EnumSystem.h"
#include "FAA_Obs.h"
#include "XESIO.h"
#include "MemFileUtils.h"
#include "GISUtils.h"

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
	Point2	sw, ne;
	CalcBoundingBox(gMap, sw, ne);
	printf("SW = %lf,%lf  NE = %lf, %lf\n", sw.x, sw.y, ne.x, ne.y);
	
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
	is_valid = ValidateMapDominance(gMap);
	if (gVerbose)
		printf("Map %s dominance valid.\n", is_valid ? "is" : "is not");				 
	if (!is_valid) 
	{
		fprintf(stderr,"Dominance Validation check failed for map %d,%d -> %d,%d\n", gMapWest, gMapSouth, gMapEast, gMapNorth);
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
		ReadXESFile(load, gMap, gTriangulationHi, gDem, gApts, gProgress);
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
	return 0;	
}

static int DoOverlay(const vector<const char *>& args)
{
	if (gVerbose) printf("Overlaying file %s...\n", args[0]);
	MFMemFile * load = MemFile_Open(args[0]);
	Pmwx		theMap;
	if (load)
	{
		DEMGeoMap	dems;
		AptVector	apts;
		CDT			theMesh;
		ReadXESFile(load, theMap, theMesh, dems, apts, gProgress);
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
				
	OverlayMap(gMap, theMap);
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
		if (!f->IsWater())
			++nland;
		if (nland > 0)
			break;
	}
	if (!gDem.empty() || (nland > 0))
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
	Point2	sw, ne;
	CalcBoundingBox(gMap, sw, ne);
	
	
	for (int w = sw.x; w < ne.x; ++w)
	for (int s = sw.y; s < ne.y; ++s)	
	{
		vector<Point2>	pts;
		pts.push_back(Point2(w  ,s  ));
		pts.push_back(Point2(w+1,s  ));
		pts.push_back(Point2(w+1,s+1));
		pts.push_back(Point2(w  ,s+1));
		
		Pmwx	cutout;		
		CropMap(gMap, cutout, pts, gProgress);
		
		SimplifyMap(cutout);
		
		int nland = 0;
		for (Pmwx::Face_iterator f = cutout.faces_begin(); f != cutout.faces_end(); ++f)
		{
			if (!f->IsWater())
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

static int DoSimplify(const vector<const char *>& args)
{
	if (gVerbose)
		printf("Halfedges before simplify: %d\n", gMap.number_of_halfedges());		
	SimplifyMap(gMap);
	if (gVerbose)	
		printf("Halfedges after simplify: %d\n", gMap.number_of_halfedges());			
	return 0;
}

static	GISTool_RegCmd_t		sCoreCmds[] = {
{ "-crop", 			0, 0, DoCrop, 			"Crop the map and DEMs to the current extent.", "" },
{ "-bbox", 			0, 0, DoBbox, 			"Show bounds of all maps.", "" },
{ "-extent", 		4, 4, DoExtent, 		"Set the bounds for further crop and import commands.", "" },
{ "-validate", 		0, 0, DoValidate, 		"Test vector map integrity.", "" },
{ "-load", 			1, 1, DoLoad, 			"Load an XES file.", "" },
{ "-save", 			1, 1, DoSave, 			"Save an XES file.", "" },
{ "-cropsave", 		1, 1, DoCropSave, 		"Save only extent as an XES file.", "" },
{ "-overlay", 		1, 1, DoOverlay, 		"Superimpose a second vector map.", "" },
{ "-simplify",		0, 0, DoSimplify,		"Remove unneeded vectors.", "" },

{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterCoreCmds(void)
{
	GISTool_RegisterCommands(sCoreCmds);
}
