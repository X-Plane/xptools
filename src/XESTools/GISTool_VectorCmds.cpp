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

#include "GISTool_VectorCmds.h"
#include "GISTool_Utils.h"
#include "GISTool_Globals.h"
#include "PerfUtils.h"
#include "MapOverlay.h"
#include "AssertUtils.h"
#include "ShapeIO.h"
#include "MapAlgs.h"
//#include "SDTSReadTVP.h"
#include "GISUtils.h"
#include "ParamDefs.h"
#include "MemFileUtils.h"
#include "Hydro.h"
//#include "TIGERRead.h"
#include "gshhs.h"
//#include "TIGERProcess.h"
//#include "TIGERImport.h"
//#include "VPFImport.h"
#include "CompGeomUtils.h"
#include "ConfigSystem.h"
#include <ctype.h>
#include "MapAlgs.h"
#include "MapHelpers.h"
#include "MapBuffer.h"
#include "NetAlgs.h"
#include "MapRaster.h"

#if OPENGL_MAP
	#include "RF_Msgs.h"
	#include "RF_Notify.h"
	#include "RF_Selection.h"
#endif

const double	kShapeFileEpsi = 0.1 / (DEG_TO_NM_LAT * NM_TO_MTR);

inline void EpsiClamp(double& v, double c, double e) { if (fabs(v - c) < e) v = c; }

inline void ClampCoord(double& v, double low, double hi, int grid, double& err, double epsi)
{
	double	v_new = v;
	v_new -= low;
	v_new *= (double) grid;
	v_new /= (hi - low);
	v_new = round(v_new);
	v_new *= (hi - low);
	v_new /= (double) grid;
	v_new += low;
	err = fabs(v_new - v);
	if (err < epsi)
		v = v_new;
	else
		err = 0.0;
}

/*

static void	import_tiger_repository(const string& rt)
{
	string root(rt);
	int	fnum = -1;
	if ((root.length() > 3) &&
		(root.substr(root.length()-4)==".RT1" ||
		 root.substr(root.length()-4)==".rt1"))
	{
		root = root.substr(0, root.length()-4);
		fnum = atoi(root.c_str() + root.length() - 5);
		root.erase(root.rfind('/'));
	}
	if ((root.length() > 3) &&
		(root.substr(root.length()-4)==".zip" ||
		 root.substr(root.length()-4)==".ZIP"))
	{
		fnum = atoi(root.c_str() + root.length() - 9);
	}

	if (fnum == -1)
	{
		fprintf(stderr,"Could not identify file %s as a TIGER file.\n", root.c_str());
	} else {

		MFFileSet * fs = FileSet_Open(root.c_str());
		if (fs)
		{
			printf("Reading %s/TGR%05d.RT1\n", root.c_str(), fnum);
			TIGER_LoadRT1(fs, fnum);
			printf("Reading %s/TGR%05d.RT2\n", root.c_str(), fnum);
			TIGER_LoadRT2(fs, fnum);
			printf("Reading %s/TGR%05d.RTP\n", root.c_str(), fnum);
			TIGER_LoadRTP(fs, fnum);
			printf("Reading %s/TGR%05d.RTI\n", root.c_str(), fnum);
			TIGER_LoadRTI(fs, fnum);
			printf("Reading %s/TGR%05d.RT7\n", root.c_str(), fnum);
			TIGER_LoadRT7(fs, fnum);
			printf("Reading %s/TGR%05d.RT8\n", root.c_str(), fnum);
			TIGER_LoadRT8(fs, fnum);

			FileSet_Close(fs);
		} else
			fprintf(stderr,"Could not open %s as a file set.\n", root.c_str());
	}
}

*/
/*
static int DoSDTSImport(const vector<const char *>& args)
{
	try {
		ImportSDTSTransferTVP(args[0], NULL, gMap);
	} catch(SDTSException& e) {
		fprintf(stderr,"SDTS Got exception: %s\n", e.what());
		return 1;
	}
	return 0;
}*/

/*
// This fetches all the counties for a given degree, using a pre-index.
static int DoMakeTigerIndex(const vector<const char *>& args)
{
	for (int n = 0; n < args.size(); ++n)
	{
		gChains.clear();
		gLandmarks.clear();
		gPolygons.clear();

		string root(args[n]);
		int	fnum = -1;
		if ((root.length() > 3) &&
			(root.substr(root.length()-4)==".RT1" ||
			 root.substr(root.length()-4)==".rt1"))
		{
			root = root.substr(0, root.length()-4);
			fnum = atoi(root.c_str() + root.length() - 5);
			root.erase(root.rfind('/'));
		}
		if ((root.length() > 3) &&
			(root.substr(root.length()-4)==".zip" ||
			 root.substr(root.length()-4)==".ZIP"))
		{
			fnum = atoi(root.c_str() + root.length() - 9);
		}

		if (fnum == -1)
		{
			fprintf(stderr,"Could not identify file %s as a TIGER file.\n", root.c_str());
			return 1;
		} else {

			MFFileSet * fs = FileSet_Open(root.c_str());
			if (fs)
			{
				TIGER_LoadRT1(fs, fnum);
				TIGER_LoadRT2(fs, fnum);
				FileSet_Close(fs);
			} else
				fprintf(stderr,"Could not open %s as a file set.\n", root.c_str());
		}

		LatLonVector	v;

		double	latMin =  1000.0;
		double	latMax = -1000.0;
		double	lonMin =  1000.0;
		double	lonMax = -1000.0;

		for (ChainInfoMap::iterator i = gChains.begin();
			i != gChains.end(); ++i)
		{
			for (vector<Point2>::iterator l = i->second.shape.begin(); l != i->second.shape.end(); ++l)
			{
				latMin = min(latMin, l->y());
				latMax = max(latMax, l->y());
				lonMin = min(lonMin, l->x());
				lonMax = max(lonMax, l->x());
			}
		}

		printf("%s %f %f %f %f\n",
			args[n], latMin, latMax, lonMin, lonMax);
	}
	return 0;
}


// This fetches all the counties for a given degree, using a pre-index.
static int DoTigerIndex(const vector<const char *>& args)
{
		TigerMap	tigerMap;

	tigerMap.clear();
	ReadTigerIndex(args[0], tigerMap);
	if (tigerMap.empty())
	{
		fprintf(stderr,"Could not open tiger index %s\n",args[0]);
		return 1;
	} else {
		int	hashNum = gMapSouth * 360 + gMapWest;
		FileList& fl = tigerMap[hashNum];
		if (fl.empty())
		{
			fprintf(stderr,"No tiger files available for %d,%d\n", gMapSouth, gMapWest);
			return 1;
		}
		string	partial(args[0]);
		string::size_type div = partial.rfind('/');
		if (div != partial.npos)
			partial.erase(div+1);
		for (int n = 0; n < fl.size(); ++n)
		{
			string	full = partial + fl[n].name;
			import_tiger_repository(full);
		}
	}

	if (gVerbose) printf("Sorting...\n");
	{
		TIGER_EliminateZeroLengthShapePoints();
		TIGER_RoughCull(gMapWest, gMapSouth, gMapEast, gMapNorth);
		if (gChains.size() > 0)
			TIGER_PostProcess(gMap);
		else
			printf("Skipping post process - no chains.\n");
	}

	if (gVerbose)  printf("Read: %d chains, %d landmarks, %d polygons.\n",
		gChains.size(), gLandmarks.size(), gPolygons.size());

	if (gChains.size() > 0)
	{
//		StElapsedTime	timer("Importing");
		TIGERImport(gChains, gLandmarks, gPolygons, gMap, gProgress);
	} else
		printf("Skipping post process - no chains.\n");

	if (gVerbose) printf("Map contains: %d faces, %d half edges, %d vertices.\n",
								gMap.number_of_faces(),
								gMap.number_of_halfedges(),
								gMap.number_of_vertices());
	return 0;
}


// This code imports counties by name - ue to get a quick map of specific stuff.
static int DoTigerImport(const vector<const char *>& args)
{
	for (int n = 0; n < args.size(); ++n)
	{
		string	root = args[n];
		import_tiger_repository(root);
	}

	if (gVerbose) printf("Sorting...\n");
	{
		TIGER_EliminateZeroLengthShapePoints();
		TIGER_RoughCull(gMapWest, gMapSouth, gMapEast, gMapNorth);
		if (gChains.size() > 0)
			TIGER_PostProcess(gMap);
		else
			printf("Skipping post process - no chains.\n");
	}

	if (gVerbose)  printf("Read: %d chains, %d landmarks, %d polygons.\n",
		gChains.size(), gLandmarks.size(), gPolygons.size());

	if (gChains.size() > 0)
	{
//		StElapsedTime	timer("Importing");
		TIGERImport(gChains, gLandmarks, gPolygons, gMap, gProgress);
	} else
		printf("Skipping post process - no chains.\n");


	if (gVerbose) printf("Map contains: %d faces, %d half edges, %d vertices.\n",
								gMap.number_of_faces(),
								gMap.number_of_halfedges(),
								gMap.number_of_vertices());
	return 0;
}

struct	VPFCoverageInfo_t {
	int						topology;
	vector<VPF_FaceRule_t>	face_rules;
	vector<VPF_LineRule_t>	line_rules;
	vector<int>				trans_flags;
};

static map<string, VPFCoverageInfo_t>	sVPFRules;

bool	ReadVPFLine(const vector<string>& tokens, void * ref)
{
		string				coverage;
		int					topology, flag, feature;
		VPF_FaceRule_t		face_rule;
		VPF_LineRule_t		line_rule;

	if (tokens[0] == "VPF_COVERAGE")
	{
		if (TokenizeLine(tokens, " si", &coverage, &topology) != 3) return false;
		sVPFRules[coverage].topology = topology;

	}
	else if (tokens[0] == "VPF_FACE_RULE")
	{
		if (TokenizeLine(tokens, " sttttiee", &coverage,
			&face_rule.table,
			&face_rule.attr_column,
			&face_rule.ref_column,
			&face_rule.strval,
			&face_rule.ival,
			&face_rule.terrain_type,
			&face_rule.area_feature) != 9) return false;
		sVPFRules[coverage].face_rules.push_back(face_rule);
	}
	else if (tokens[0] == "VPF_LINE_RULE")
	{
		if (TokenizeLine(tokens, " sttttiei", &coverage,
			&line_rule.table,
			&line_rule.attr_column,
			&line_rule.ref_column,
			&line_rule.strval,
			&line_rule.ival,
			&line_rule.he_param,
			&line_rule.he_trans_flags) != 9) return false;
		sVPFRules[coverage].line_rules.push_back(line_rule);
	}
	else if (tokens[0] == "VPF_TRANS_FLAGS")
	{
		if (TokenizeLine(tokens, " sie", &coverage, &flag, &feature) != 4) return false;
		sVPFRules[coverage].trans_flags.push_back(flag);
		sVPFRules[coverage].trans_flags.push_back(feature);
	} else
		return false;
	return true;
}


static int DoVPFImport(const vector<const char *>& args)
{
	static bool first_time = true;
	if (first_time)
	{
		first_time = false;
		RegisterLineHandler("VPF_COVERAGE", ReadVPFLine, NULL);
		RegisterLineHandler("VPF_FACE_RULE", ReadVPFLine, NULL);
		RegisterLineHandler("VPF_LINE_RULE", ReadVPFLine, NULL);
		RegisterLineHandler("VPF_TRANS_FLAGS", ReadVPFLine, NULL);
		LoadConfigFile("vmap0_import.txt");

		for (map<string, VPFCoverageInfo_t>::iterator rules = sVPFRules.begin(); rules != sVPFRules.end(); ++rules)
		{
			rules->second.trans_flags.push_back(0);
			rules->second.face_rules.push_back(VPF_FaceRule_t());
			rules->second.line_rules.push_back(VPF_LineRule_t());
			rules->second.face_rules.back().table = NULL;
			rules->second.line_rules.back().table = NULL;
		}
	}

	// VMap uses this funky lettering scheme - they drop
	// the letters i and o to avoid confusion with 1 and 0.
	static char chrs[] = { "abcdefghjklmnpqrstuvwxyz" };

	const char * cov_dir = args[0];
	const char * cov_list = args[1];

	int west = (180 + atoi(args[2]) ) / 15;
	int south = (90 + atoi(args[3])) / 15;

	printf("Arg count: %d\n", args.size());
	printf("args: %s\n%s\n%s\n%s\n", args[0], args[1], args[2], args[3]);
	printf("West = %d, south = %d\n", west, south);

	char	tile[5];
	char	coverage[1024];
	tile[0] = chrs[west];
	tile[1] = toupper(chrs[south]);
	tile[2] = 0;
	if (args.size() > 4)
	{
		tile[1] = (chrs[south]);
		tile[2] = chrs[atoi(args[4])];
		tile[3] = chrs[atoi(args[5])];
		tile[4] = 0;
	}

	char	coverages[512];
	char *	cov = coverages, * found;
	strcpy(coverages,cov_list);
	bool	first = gMap.is_empty();
	bool	ok;
	bool	ok_any=false;
	while ((found = strtok(cov, ",")) != NULL)
	{
		if (sVPFRules.count(found) == 0)
		{
			printf("Error - don't know to import %s\n", found);
			return 1;
		}
		strcpy(coverage, cov_dir);
		strcat(coverage, found);
		if (first)
//		if (first || sVPFRules[found].topology != 3)
		{
//			gMap.unbounded_face()->mTerrainType = terrain_Natural;
			ok = VPFImportTopo3(coverage, tile, gMap,
					sVPFRules[found].topology == 3,
					&*sVPFRules[found].line_rules.begin(),
					&*sVPFRules[found].face_rules.begin(),
					&*sVPFRules[found].trans_flags.begin());
//			gMap.unbounded_face()->mTerrainType = terrain_Natural;

			if (ok && first)
			{
				Point_2	sw, ne;
				CalcBoundingBox(gMap, sw, ne);
				for (int x = CGAL::to_double(sw.x()) + 1; x < CGAL::to_double(ne.x()); ++x)
				{
					CGAL::insert_curve(gMap, Curve_2(Segment_2(Point_2(x,sw.y()),Point_2(x,ne.y()))));
				}
				for (int y = CGAL::to_double(sw.y()) + 1; y < CGAL::to_double(ne.y()); ++y)
				{
					CGAL::insert_curve(gMap, Curve_2(Segment_2(Point_2(sw.x(),y),Point_2(ne.x(),y))));
				}
			}

//			UnmangleBorder(gMap);
		} else {
			Pmwx	overlay;
			ok = VPFImportTopo3(coverage, tile, overlay,
					sVPFRules[found].topology == 3,
					&*sVPFRules[found].line_rules.begin(),
					&*sVPFRules[found].face_rules.begin(),
					&*sVPFRules[found].trans_flags.begin());
			if (gVerbose) printf("Merging maps  Dst faces = %d, src hedges = %d\n", gMap.number_of_faces(), overlay.number_of_halfedges());

			if (sVPFRules[found].topology < 3)
			{
				for (Pmwx::Face_iterator ff = overlay.faces_begin(); ff != overlay.faces_end(); ++ff)
					ff->data().mTerrainType = terrain_Natural;
			}

//			TopoIntegrateMaps(&gMap, &overlay);
			MergeMaps_legacy(gMap, overlay, true, NULL, true, gProgress);
		}

		if (ok)
		{
			ok_any=true;
			if (gVerbose) printf("Imported %s/%s\n", found, tile);
			if (gVerbose) printf("Map contains: %d faces, %d half edges, %d vertices.\n",
				gMap.number_of_faces(),
				gMap.number_of_halfedges(),
 				gMap.number_of_vertices());
//			PERFORMANCE NOTE: line insertion is a function of the averge numer of segments in a CCB.
//			Simplifying makes fewer complex large faces - this makes insert a lot slower.
//			SimplifyMap(gMap);
//			if (gVerbose) printf("(Post simplify: %d faces, %d half edges, %d vertices.)\n",
//				gMap.number_of_faces(),
//				gMap.number_of_halfedges(),
//				gMap.number_of_vertices());
		} else {
//			fprintf(stderr, "ERROR importing VPF file %s/%s\n", cov, tile);
			fprintf(stdout,"WARNING: problem importing VPF file %s/%s\n", found, tile);
//			return 1;
		}
		cov = NULL;
		first = false;
	}

	if(!ok_any)
	{
		fprintf(stderr, "ERROR: unable to import any coverages for %s.\n",tile);
	}
	return ok_any ? 0 : 1;
}
*/
static int DoGSHHSImport(const vector<const char *>& args)
{
	double c[4] = { (double) gMapWest, (double) gMapSouth, (double) gMapEast, (double) gMapNorth };
	if (ImportGSHHS(args[0], gMap, c))
	{
		if (gVerbose)printf("Map contains: %llu faces, %llu half edges, %llu vertices.\n",
			(unsigned long long)gMap.number_of_faces(),
			(unsigned long long)gMap.number_of_halfedges(),
			(unsigned long long)gMap.number_of_vertices());
	} else {
		printf("Error importing GSHHS file %s\n", args[0]);
		return 1;
	}
	return 0;
}
/*
static int DoTigerBounds(const vector<const char *>& args)
{
		TigerMap	tigerMap;

	ReadTigerIndex(args[0], tigerMap);
	if (tigerMap.empty()) {
		fprintf(stderr,"Could not open tiger index %s\n",args[0]);
		return 1;
	} else {
		float	lat_min = 90.0;
		float	lat_max =-90.0;
		float	lon_min = 180.0;
		float	lon_max =-180.0;
		for (TigerMap::iterator i = tigerMap.begin(); i != tigerMap.end(); ++i)
		{
			for (FileList::iterator f = i->second.begin(); f != i->second.end(); ++f)
			{
				lat_min = min(lat_min, f->lat_min);
				lat_max = max(lat_max, f->lat_max);
				lon_min = min(lon_min, f->lon_min);
				lon_max = max(lon_max, f->lon_max);
			}
		}
		int iymin = floor(lat_min);
		int iymax = ceil(lat_max);
		int ixmin = floor(lon_min);
		int ixmax = ceil(lon_max);
		printf("Total bounds: %d,%d to %d,%d\n", ixmin, iymin, ixmax, iymax);
		int count = 0;
		for (int y = iymin; y <= iymax; ++y)
		for (int x = ixmin; x <= ixmax; ++x)
		{
			int hash = y * 360 + x;
			if (tigerMap.find(hash) != tigerMap.end())
				++count, printf("call make_us_one_vec %d %d %d %d %+03d%+04d %+03d%+04d.xes\n",x,y,x+1,y+1,latlon_bucket(y), latlon_bucket(x), y,x);
		}
		printf("Total = %d\n", count);
	}
	return 0;
}
*/
#define HELP_SHAPE \
"-shapefile <mode> <feature> <err> <grid> <filename> [...<filename]\n" \
"Import a shape file.  Mode letters (similar to tar syntax are):\n" \
"r - roads - attempt to import arcs as roads.\n" \
"w - water - attempt to import arcs as water boundaries.\n" \
"l - land use - attempt to import polygons as land use.\n" \
"f - features - attempt to import polygons as area fatures.\n" \
"s - simple feature import.  Feature param is applied to all elements.\n" \
"m - feature map.  Feature param is a config text file that maps database properties to features.\n" \
"c - crop to current map bounds on import.  This can be faster than a separate cropping stage.\n" \
"o - overlay on existing map.  This can be slower than cleaning the vector space first.\n" \
"e - check for overlapping polygon errors.  Abort the import silently if we hit this case.\n" \
"<err> is the max error in meters to be allowed when simplifying imported roads.  Pass zero to\n"\
"import the data with no change.\n"
static int DoShapeImport(const vector<const char *>& args)
{
	shp_Flags flags = shp_None;
	if(strstr(args[0], "c"))	flags |= shp_Use_Crop;
	if(strstr(args[0], "o"))	flags |= shp_Overlay;
	if(strstr(args[0], "r"))	flags |= shp_Mode_Road;
	if(strstr(args[0], "l"))	flags |= shp_Mode_Landuse;
	if(strstr(args[0], "f"))	flags |= shp_Mode_Feature;
	if(strstr(args[0], "w"))	flags |= shp_Mode_Coastline;
	if(strstr(args[0], "s"))	flags |= shp_Mode_Simple;
	if(strstr(args[0], "m"))	flags |= shp_Mode_Map;
	if(strstr(args[0], "e"))	flags |= shp_ErrCheck;
	
	double err_margin = atof(args[2]);
	int grid_steps = atoi(args[3]);

	for(int n = 4; n < args.size(); ++n)
	{
		double b[4] = { (double) gMapWest, (double) gMapSouth, (double) gMapEast, (double) gMapNorth };
		
		Pmwx backup;
		
		if(flags & shp_ErrCheck)
			backup = gMap;
			
		if(!ReadShapeFile(args[n], gMap, flags, args[1], b, err_margin, grid_steps, gProgress))
		{
			if(flags & shp_ErrCheck)
			{
				gMap = backup;
			}
			else
			{
				if(gVerbose) printf("Failed to load shape file: %s\n", args[n]);
				return 1;
			}
		}
	}

#if OPENGL_MAP
	RF_Notifiable::Notify(rf_Cat_File, rf_Msg_FileLoaded, NULL);
#endif
	return 0;
}

//"-shapefile_raster <mode> <feature> <filename> <layer>n" 
static int DoShapeRaster(const vector<const char *>& inArgs)
{
	int layer = LookupToken(inArgs[3]);
	if(layer == -1)
	{
		fprintf(stderr,"Unknown raster layer %d\n", layer);
		return 1;		
	}
	if (gDem.count(layer) == 0)
	{
		fprintf(stderr,"Layer %s is not initialized.\n", inArgs[3]);
		return 1;
	}

	shp_Flags flags = shp_None;
	if(strstr(inArgs[0], "s"))	flags |= shp_Mode_Simple;
	if(strstr(inArgs[0], "m"))	flags |= shp_Mode_Map;
	
	if(!RasterShapeFile(
				inArgs[2],
				gDem[layer],
				flags,
				inArgs[1],
				gProgress))
	{
		fprintf(stderr,"Unable to load shapefile: %s\n",inArgs[2]);
		return 1;
	}
	else
	{
		if(gVerbose) printf("Loaded shapefile: %s\n", inArgs[2]);
	}
	return 0;
}


/*
int DoWetMask(const vector<const char *>& args)
{
	MakeWetMask(args[0], gMapWest, gMapSouth, args[1]);
	return 0;
}
*/

struct debug_lock_traits {
	bool is_locked(Pmwx::Vertex_handle v) const { 
		Pmwx::Halfedge_handle h1(v->incident_halfedges());
		Pmwx::Halfedge_handle h2(h1->next());
//		if(h1->face()->data().IsWater() == h1->twin()->face()->data().IsWater())
//		{
//			debug_mesh_point(cgal2ben(v->point()),1,1,1);
//			return true;
//		}	
		Point2 p1(cgal2ben(h1->source()->point()));
		Point2 p2(cgal2ben(h1->target()->point()));
		Point2 p3(cgal2ben(h2->target()->point()));
		Vector2 v1(p1,p2);
		Vector2 v2(p2,p3);
//		if (v1.dot(v2) < 0.0)
//		{
//			debug_mesh_point(p2,1,0,0);
//			return true;
//		}

		if(h1->source()->degree() == 2)
		{
			Point2 p0(cgal2ben(h1->prev()->source()->point()));
			Vector2 v0(p0,p1);			
			Vector2	vn(p1,p3);
			if(v0.dot(vn) < 0.0)
			{
//				debug_mesh_point(p2,1,0,0);
				return true;
			}
		}
		if(h2->target()->degree() == 2)
		{
			Point2 p4(cgal2ben(h2->next()->target()->point()));
			Vector2 vn(p1,p3);			
			Vector2	v3(p3,p4);
			if(vn.dot(v3) < 0.0)
			{
//				debug_mesh_point(p2,0,1,0);
				return true;
			}
		}
		return false;
		

	}
	void remove(Pmwx::Vertex_handle v) const { 
//		debug_mesh_point(cgal2ben(v->point()),1,1,0); 
	}
};


#define HELP_REDUCE_VECTORS \
"-reduce_vectors <tolerance>\n"\
"This command will reduce the number of vertices in the map, without destroying the topological relations.\n"\
"The tolerance is in meters and is the farthest any point will end up from another point.  Area features that\n"\
"are smaller than the tolerance may be completely collapsed.\n"
int DoReduceVectors(const vector<const char *>& args)
{
	int b = gMap.number_of_halfedges();
//	MapSimplify(gMap, atof(args[0]));

	arrangement_simplifier<Pmwx,debug_lock_traits> simplifier;
	simplifier.simplify(gMap, atof(args[0]),debug_lock_traits(), gProgress);

	int a = gMap.number_of_halfedges();
	printf("Before: %d, after: %d\n", b,a);
	return 0;
}

#define HELP_DESLIVER \
"-desliver <tolerance>\n"\
"This command changes the land class of very thin areas to reduce the number of triangles needed in the mesh."
int DoDesliver(const vector<const char *> &args)
{
	int r = MapDesliver(gMap, atof(args[0]), gProgress);
	printf("Flipped %d faces.\n",r);
	return 0;
}


#define HELP_KILL_SLIVER_WATER \
"-kill_sliver_water <tolerance>\n"\
"This command changes the land class of very wet areas to remove polygonal rivers that we can't afford.\n"
int DoKillSliverWater(const vector<const char *> &args)
{
	int r = KillSliverWater(gMap, atof(args[0]), gProgress);
	printf("Flipped %d faces.\n",r);
	return 0;
}

#define HELP_KILL_SLOPED_WATER \
"-kill_sloped_water <gradient>\n"\
"This removes wet polygons whose net slope is approximately more than 1/x^2 where x is the gradient factor.\n"
int DoKillSlopedWater(const vector<const char *> &args)
{
	int r = KillSlopedWater(gMap, gDem[dem_Elevation], atof(args[0]), gProgress);
	printf("Flipped %d faces.\n",r);
	return 0;
}

int DoKillTunnels(const vector<const char *>& args)
{
	int k = KillTunnels(gMap);
	printf("Removed %d tunnels.\n",k);
	return 0;
}




#if OPENGL_MAP && DEV
int DoCheckRoads(const vector<const char *>& args)
{
	debug_network(gMap);
	return 0;
}
#endif

int DoFixRoads(const vector<const char *>& args)
{
	repair_network(gMap);
	return 0;
}

#define HELP_REMOVE_OUTSETS \
"-remove_outsets max_len max_area\n" \
"This command removes any square-sh outset piers in the water that are less than the length and area requirements."
int DoRemoveOutsets(const vector<const char *>& args)
{
	double len = atof(args[0]);
	double area = atof(args[1]);
	int k = RemoveOutsets(gMap, len*len,area);
	if (gVerbose)
		printf("Removed %d outsets.\n",k);
	return 0;
}

#define HELP_REMOVE_ISLANDS \
"-remove_islands max_area_mtr\n" \
"This command removes any tiny islands of land or water that are below a certain square m size."
int DoRemoveIslands(const vector<const char *>& args)
{
	double area = atof(args[0]);
	int k = RemoveIslands(gMap, area);
	if (gVerbose)
		printf("Removed %d islands\n",k);
	return 0;
}

#define HELP_REMOVE_WET_ANTENNAS \
"-remove_wet_antennas dist_lo dist_hi\n" \
"Removes roads that hang into the water.  Land-fill bridges near land by <dist>\n"
int DoRemoveWetAntennas(const vector<const char *>& args)
{
	int k = KillWetAntennaRoads(gMap);
	if(gVerbose) printf("Killed %d roads.\n",  k);
	k = LandFillStrandedRoads(gMap, atof(args[0]),atof(args[1]));
	if(gVerbose) printf("Filled %d roads-ponds.\n",  k);

	return 0;	
}

int DoBufferWater(const vector<const char *>& args)
{
	double inset = atof(args[0]);
	for(Pmwx::Face_handle f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
		f->set_contained(f->data().IsWater());
	Polygon_set_2 all_water(gMap), buffered_water;
	gMap.clear();
	if(inset == 0.0)
		buffered_water = all_water;
	else
		BufferPolygonSet(all_water, inset, buffered_water);
	all_water.clear();
	gMap = buffered_water.arrangement();
	buffered_water.clear();
	for(Pmwx::Face_handle f = gMap.faces_begin(); f != gMap.faces_end(); ++f)
		f->data().mTerrainType = f->contained() ? terrain_Water : NO_VALUE;

#if OPENGL_MAP
	RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
#endif
	
	return 0;
}


//void	ColorFaces(set<Face_handle>&	io_faces);
int DoRasterDEM(const vector<const char *>& args)
{
	DEMGeo& d(gDem[LookupToken(args[0])]);
	MapFromDEM(d,0,0,d.mWidth,d.mHeight,0,gMap,NULL);
#if OPENGL_MAP
	RF_Notifiable::Notify(rf_Cat_File, rf_Msg_VectorChange, NULL);
#endif
	return 0;
}

static	GISTool_RegCmd_t		sVectorCmds[] = {
//{ "-sdts", 			1, 1, 	DoSDTSImport, 			"Import SDTS VTP vector map.", "" },
//{ "-tigermakeidx",	1, -1,	DoMakeTigerIndex,		"Make index line for files", "" },
//{ "-tiger", 		1, -1, 	DoTigerImport, 			"Import tiger line file.", "" },
//{ "-tigerindex", 	1, 1, 	DoTigerIndex, 			"Import tiger line files.", "" },
//{ "-tigerbounds", 	1, 1, 	DoTigerBounds, 			"Show all tiger files for a given location.", "" },
//{ "-vpf", 			4, 6, 	DoVPFImport, 			"Import VPF coverage <path> <coverages> <lon> <lat> [<sublon> <sublat>]", "" },
{ "-gshhs", 		1, 1, 	DoGSHHSImport, 			"Import GSHHS shorelines.", "" },
{ "-shapefile", 	5, -1, 	DoShapeImport, 			"Import ESRI Shape File.", HELP_SHAPE },
{ "-shapefile_raster", 4, 4, DoShapeRaster,			"Raster shapefile.", "" },
{ "-reduce_vectors", 1, 1,	DoReduceVectors,		"Simplify vector map by a certain error distance.", HELP_REDUCE_VECTORS },
{ "-remove_outsets", 2, 2, DoRemoveOutsets,			"Remove square outset piers from water areas.", HELP_REMOVE_OUTSETS },
{ "-remove_islands", 1, 1, DoRemoveIslands,			"Remove square outset piers from water areas.", HELP_REMOVE_ISLANDS },
{ "-remove_wet_antennas", 2, 2, DoRemoveWetAntennas,	"Remove roads that hang out into the water.", HELP_REMOVE_WET_ANTENNAS },	
{ "-buffer_water", 1, 1, DoBufferWater,				"Buffer water by a certain amonut.", "" },
{ "-desliver",		1, 1, DoDesliver,				"Remove slivered polygons.", HELP_DESLIVER },
{ "-kill_sliver_water",1,1,DoKillSliverWater,		"Remove slivers of water",HELP_KILL_SLIVER_WATER },
{ "-kill_sloped_water",1,1,DoKillSlopedWater,		"Remove slopeds of water",HELP_KILL_SLOPED_WATER },
{ "-kill_tunnels",0,0,DoKillTunnels,				"Remove all tunnel vectors", "" },
#if OPENGL_MAP && DEV
{ "-check_roads",	0, 0, DoCheckRoads,				"Check roads for errors.", "" },
#endif
{ "-fix_roads",	0, 0, DoFixRoads,				"Fix road errors.", "" },
{ "-raster_dem",	1, 1, DoRasterDEM,				"Map Color.", "" },
//{ "-wetmask",		2, 2,	DoWetMask,				"Make wet mask for file", "" },
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterVectorCmds(void)
{
	GISTool_RegisterCommands(sVectorCmds);
}
