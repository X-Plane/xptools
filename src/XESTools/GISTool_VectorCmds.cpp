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

#if OPENGL_MAP
	#include "RF_Msgs.h"
	#include "RF_Notify.h"
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
	double c[4] = { gMapWest, gMapSouth, gMapEast, gMapNorth };
	if (ImportGSHHS(args[0], gMap, c))
	{
		if (gVerbose)printf("Map contains: %d faces, %d half edges, %d vertices.\n",
			gMap.number_of_faces(),
			gMap.number_of_halfedges(),
			gMap.number_of_vertices());
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
"-shapefile <mode> <feature> <filename>\n" \
"Import a shape file.  Mode letters (similar to tar syntax are):\n" \
"r - roads - attempt to import arcs as roads.\n" \
"l - land use - attempt to import polygons as land use.\n" \
"f - features - attempt to import polygons as area fatures.\n" \
"s - simple feature import.  Feature param is applied to all elements.\n" \
"m - feature map.  Feature param is a config text file that maps database properties to features.\n" \
"c - crop to current map bounds on import.  This can be faster than a separate cropping stage.\n" \
"o - overlay on existing map.  This can be slower than cleaning the vector space first.\n" \
"q - attempt quick import.  For some shape files, this will assume correct noding, no overlaps, etc.  Fast, but may fail.  Only one area feature/landuse can be applied to all imported shapes.\n"
static int DoShapeImport(const vector<const char *>& args)
{
	shp_Flags flags = shp_None;
	if(strstr(args[0], "c"))	flags |= shp_Use_Crop;
	if(strstr(args[0], "o"))	flags |= shp_Overlay;
	if(strstr(args[0], "q"))	flags |= shp_Fast;
	if(strstr(args[0], "r"))	flags |= shp_Mode_Road;
	if(strstr(args[0], "l"))	flags |= shp_Mode_Landuse;
	if(strstr(args[0], "f"))	flags |= shp_Mode_Feature;
	if(strstr(args[0], "s"))	flags |= shp_Mode_Simple;
	if(strstr(args[0], "m"))	flags |= shp_Mode_Map;


	double b[4] = { gMapWest, gMapSouth, gMapEast, gMapNorth };
	if(!ReadShapeFile(args[2], gMap, flags, args[1], b, gProgress))
		return 1;


#if OPENGL_MAP
	RF_Notifiable::Notify(rf_Cat_File, rf_Msg_FileLoaded, NULL);
#endif
	return 0;
}


/*
int DoWetMask(const vector<const char *>& args)
{
	MakeWetMask(args[0], gMapWest, gMapSouth, args[1]);
	return 0;
}
*/

static	GISTool_RegCmd_t		sVectorCmds[] = {
//{ "-sdts", 			1, 1, 	DoSDTSImport, 			"Import SDTS VTP vector map.", "" },
//{ "-tigermakeidx",	1, -1,	DoMakeTigerIndex,		"Make index line for files", "" },
//{ "-tiger", 		1, -1, 	DoTigerImport, 			"Import tiger line file.", "" },
//{ "-tigerindex", 	1, 1, 	DoTigerIndex, 			"Import tiger line files.", "" },
//{ "-tigerbounds", 	1, 1, 	DoTigerBounds, 			"Show all tiger files for a given location.", "" },
//{ "-vpf", 			4, 6, 	DoVPFImport, 			"Import VPF coverage <path> <coverages> <lon> <lat> [<sublon> <sublat>]", "" },
{ "-gshhs", 		1, 1, 	DoGSHHSImport, 			"Import GSHHS shorelines.", "" },
{ "-shapefile", 	3, 3, 	DoShapeImport, 			"Import ESRI Shape File.", HELP_SHAPE },
//{ "-wetmask",		2, 2,	DoWetMask,				"Make wet mask for file", "" },
{ 0, 0, 0, 0, 0, 0 }
};

void	RegisterVectorCmds(void)
{
	GISTool_RegisterCommands(sVectorCmds);
}
