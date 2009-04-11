/*
 * Copyright (c) 2004, Laminar Research.
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
#include "FAA_Obs.h"
#include "MemFileUtils.h"
#include "ParamDefs.h"
#include "SimpleIO.h"
#include "DEMDefs.h"
#include "MapDefsCGAL.h"
#include "MapAlgs.h"
#include "GISTool_Globals.h"

FAAObsTable		gFAAObs;

const char *	gObsNames [] = {
	"AM_RADIO_TOWER",
	"POLE",
	"TOWERS",
	"TOWER",
	"CRANE",
	"CATENARY",
	"BLDG",
	"CTRL TWR",
	"T-L TWR",
	"ELEVATOR",
	"WINDMILL",
	"RIG",
	"STACK",
	"REFINERY",
	"TANK",
	"BLDG-TWR",
	"TRAMWAY",
	"STACKS",
	"SPIRE",
	"CRANE T",
	"PLANT",
	"DOME",
	"SIGN",
	"BRIDGE",
	"ARCH",
	"COOL TWR",
	"MONUMENT",
	"BALLOON",
	"DAM",
	0
};

static	int		kFeatureTypes [] = {
	feat_RadioTower,
	feat_Pole,
	feat_RadioTower,
	feat_RadioTower,
	feat_Crane,
	NO_VALUE,		// "CATENARY",
	feat_Building,
	NO_VALUE,		// "CTRL TWR",
	NO_VALUE,		// "T-L TWR",
	feat_Elevator,
	feat_Windmill,
	NO_VALUE,		// "RIG",
	feat_Smokestack,
	feat_Refinery,
	feat_Tank,
	feat_Building,
	feat_Tramway,
	feat_Smokestacks,
	feat_Spire,
	feat_Crane,
	feat_Plant,
	feat_Dome,
	feat_Sign,
	NO_VALUE,		// "BRIDGE",
	feat_Arch,
	feat_CoolingTower,
	feat_Monument,
	NO_VALUE,		// "BALLOON",
	feat_Dam,
	0
};

static	int	kConvertLegacyObjTypes[] = {
	NO_VALUE,
	NO_VALUE,
//	feat_Skyscraper,
	feat_Building,		// BEN SAYS: importing as sky-scraper means no obj match. :-(
	feat_RadioTower,
	NO_VALUE,
	feat_CoolingTower,
	feat_Smokestack,
	NO_VALUE
};

static	int	kRobinToFeatures[] = {
	NO_VALUE,
	NO_VALUE,
	feat_BeaconNDB,
	feat_BeaconVOR,
	feat_BeaconILS,
	feat_BeaconLDA,
	feat_BeaconGS,
	feat_MarkerBeacon,
	feat_MarkerBeacon,
	feat_MarkerBeacon
};

static int	GetObjType(const char * str)
{
	int n = 0;
	while (gObsNames[n])
	{
		if (!strcmp(gObsNames[n],str))
			return n;
		++n;
	}
	return 0;
}

bool	LoadFAARadarFile(const char * inFile, bool isApproach)
{
	MFMemFile *	f = NULL;
	MFTextScanner * s = NULL;
	bool		ok = false;

	f = MemFile_Open(inFile);
	if (f == NULL) goto bail;

	s = TextScanner_Open(f);
	if (s == NULL) goto bail;

	while (!TextScanner_IsDone(s))
	{
		const char * c = TextScanner_GetBegin(s);
		if (*c == '_') { TextScanner_Next(s); break; }
		TextScanner_Next(s);
	}
	while (!TextScanner_IsDone(s))
	{
		const char * c = TextScanner_GetBegin(s);
		if (*c == '_') { TextScanner_Next(s); break; }
		TextScanner_Next(s);
	}

	while (!TextScanner_IsDone(s))
	{
		FAAObs_t	obs;

		const char * lp = TextScanner_GetBegin(s);
		int o = isApproach ? 46 : 59;
		if (lp[59] != ' ')
		{

			double lat_deg_tens = lp[o  ] - '0';
			double lat_deg_ones = lp[o+1] - '0';
			double lat_min_tens = lp[o+3] - '0';
			double lat_min_ones = lp[o+4] - '0';
			double lat_sec_tens = lp[o+6] - '0';
			double lat_sec_ones = lp[o+7] - '0';
			double lat_hun_tens = lp[o+9] - '0';
			double	neg = (lp[o+10] != 'S') ? 1.0 : -1.0;

			obs.lat = neg * (lat_deg_tens * 10.0 +
							 lat_deg_ones *  1.0 +
							 lat_min_tens * 10.0 / 60.0 +
							 lat_min_ones *  1.0 / 60.0 +
							 lat_sec_tens * 10.0 / 3600.0 +
							 lat_sec_ones *  1.0 / 3600.0 +
							 lat_hun_tens * 10.0 / 360000.0);

			o = isApproach ? 59 : 71;
			double lon_deg_huns = lp[o  ] - '0';
			double lon_deg_tens = lp[o+1] - '0';
			double lon_deg_ones = lp[o+2] - '0';
			double lon_min_tens = lp[o+4] - '0';
			double lon_min_ones = lp[o+5] - '0';
			double lon_sec_tens = lp[o+7] - '0';
			double lon_sec_ones = lp[o+8] - '0';
			double lon_hun_tens = lp[o+10] - '0';
			neg = (lp[o+11] == 'E') ? 1.0 : -1.0;

			obs.lon = neg * (lon_deg_huns * 100.0 +
							 lon_deg_tens * 10.0 +
							 lon_deg_ones *  1.0 +
							 lon_min_tens * 10.0 / 60.0 +
							 lon_min_ones *  1.0 / 60.0 +
							 lon_sec_tens * 10.0 / 3600.0 +
							 lon_sec_ones *  1.0 / 3600.0 +
							 lon_hun_tens * 10.0 / 360000.0);
			obs.agl = TextScanner_ExtractLong(s, 73, 77) * 0.3048;
			obs.msl = TextScanner_ExtractLong(s, 78, 83) * 0.3048;

			if (isApproach)
			{
				obs.agl = TextScanner_ExtractLong(s,20,23);
				obs.msl = obs.agl + TextScanner_ExtractLong(s,71,76);
				obs.kind = feat_RadarASR;
			} else {
				obs.agl = NO_VALUE;
				obs.msl = TextScanner_ExtractLong(s,85,90);
				obs.kind = feat_RadarARSR;
			}

			gFAAObs.insert(FAAObsTable::value_type(HashLonLat(obs.lon, obs.lat), obs));

#if 0
		printf("   %lf,%lf   %f (%f)    %d (%s)       '%s' (%s)\n",
			obs.lon, obs.lat, obs.msl, obs.agl,
			obs.kind, FetchTokenString(obs.kind),
			obs.kind_str.c_str(), obs.freq.c_str());
#endif
		}
		TextScanner_Next(s);
	}

	ok = true;

bail:
	if (f) MemFile_Close(f);
	if (s) TextScanner_Close(s);
	return ok;
}



bool	LoadFAAObsFile(const char * inFile)
{
	MFMemFile *	f = NULL;
	MFTextScanner * s = NULL;
	bool		ok = false;

	f = MemFile_Open(inFile);
	if (f == NULL) goto bail;

	s = TextScanner_Open(f);
	if (s == NULL) goto bail;

	while (!TextScanner_IsDone(s))
	{
		const char * c = TextScanner_GetBegin(s);
		if (*c == '-') { TextScanner_Next(s); break; }
		TextScanner_Next(s);
	}

	while (!TextScanner_IsDone(s))
	{
		FAAObs_t	obs;
		TextScanner_ExtractString(s,58, 66, obs.kind_str, true);
		TextScanner_ExtractString(s,68, 72, obs.freq, true);
		if (!obs.freq.empty())
			obs.kind = 0;
		else
			obs.kind = GetObjType(obs.kind_str.c_str());

		obs.kind = kFeatureTypes[obs.kind];

		const char * lp = TextScanner_GetBegin(s);
		double lat_deg_tens = lp[29] - '0';
		double lat_deg_ones = lp[30] - '0';
		double lat_min_tens = lp[32] - '0';
		double lat_min_ones = lp[33] - '0';
		double lat_sec_tens = lp[35] - '0';
		double lat_sec_ones = lp[36] - '0';
		double lat_hun_tens = lp[38] - '0';
		double lat_hun_ones = lp[39] - '0';
		double	neg = (lp[40] != 'S') ? 1.0 : -1.0;

		obs.lat = neg * (lat_deg_tens * 10.0 +
						 lat_deg_ones *  1.0 +
						 lat_min_tens * 10.0 / 60.0 +
						 lat_min_ones *  1.0 / 60.0 +
						 lat_sec_tens * 10.0 / 3600.0 +
						 lat_sec_ones *  1.0 / 3600.0 +
						 lat_hun_tens * 10.0 / 360000.0 +
						 lat_hun_ones *  1.0 / 360000.0);

		double lon_deg_huns = lp[43] - '0';
		double lon_deg_tens = lp[44] - '0';
		double lon_deg_ones = lp[45] - '0';
		double lon_min_tens = lp[47] - '0';
		double lon_min_ones = lp[48] - '0';
		double lon_sec_tens = lp[50] - '0';
		double lon_sec_ones = lp[51] - '0';
		double lon_hun_tens = lp[53] - '0';
		double lon_hun_ones = lp[54] - '0';
		neg = (lp[55] == 'E') ? 1.0 : -1.0;

		obs.lon = neg * (lon_deg_huns * 100.0 +
						 lon_deg_tens * 10.0 +
						 lon_deg_ones *  1.0 +
						 lon_min_tens * 10.0 / 60.0 +
						 lon_min_ones *  1.0 / 60.0 +
						 lon_sec_tens * 10.0 / 3600.0 +
						 lon_sec_ones *  1.0 / 3600.0 +
						 lon_hun_tens * 10.0 / 360000.0 +
						 lon_hun_ones *  1.0 / 360000.0);
		obs.agl = TextScanner_ExtractLong(s, 73, 77) * 0.3048;
		obs.msl = TextScanner_ExtractLong(s, 78, 83) * 0.3048;

		gFAAObs.insert(FAAObsTable::value_type(HashLonLat(obs.lon, obs.lat), obs));

#if 0	//DEV
		printf("   %lf,%lf   %f (%f)    %d (%s)       '%s' (%s)\n",
			obs.lon, obs.lat, obs.msl, obs.agl,
			obs.kind, FetchTokenString(obs.kind),
			obs.kind_str.c_str(), obs.freq.c_str());
#endif
		TextScanner_Next(s);
	}

	ok = true;

bail:
	if (f) MemFile_Close(f);
	if (s) TextScanner_Close(s);
	return ok;
}

bool	WriteDegFile(const char * inFile, int lon, int lat)
{
	pair<FAAObsTable::iterator,FAAObsTable::iterator>	range =
		gFAAObs.equal_range(HashLonLat(lon,lat));
	if (range.first == range.second) return true;
	FILE * fi = fopen(inFile, "w");
	if (!fi) return false;
	fprintf(fi, "# lon lat msl agl kind\n");
	for (FAAObsTable::iterator i = range.first; i != range.second; ++i)
	{
		if (i->second.kind != NO_VALUE)
		fprintf(fi, "%lf %lf %f %f %s\n",
			i->second.lon,
			i->second.lat,
			i->second.msl,
			i->second.agl,
			FetchTokenString(i->second.kind));
	}
	fclose(fi);
	return 1;
}

bool	ReadDegFile(const char * inFile)
{
	MFMemFile *	f = NULL;
	MFTextScanner * s = NULL;
	bool ok = false;
	f = MemFile_Open(inFile);
	if (f == NULL) goto bail;
	s = TextScanner_Open(f);
	if (s == NULL) goto bail;
	while (!TextScanner_IsDone(s))
	{
		const char * l = TextScanner_GetBegin(s);
		if (*l != '#')
		{
			FAAObs_t	obs;
			char	buf[256];
			if (sscanf(l, "%lf %lf %f %f %s",
				&obs.lon, &obs.lat, &obs.msl, &obs.agl, buf) == 5)
			{
//				printf("Got: %lf %lf %f %f %s\n",
//					obs.lon, obs.lat, obs.msl, obs.agl, buf);
				obs.kind = LookupToken(buf);
				gFAAObs.insert(FAAObsTable::value_type(HashLonLat(obs.lon, obs.lat), obs));
			}
		}
		TextScanner_Next(s);
	}

	ok = true;
bail:
	if (s) TextScanner_Close(s);
	if (f) MemFile_Close(f);
	return ok;
}


bool	ReadAPTNavAsObs(const char * inFile)
{
		MFMemFile * 		f = NULL;
		MFTextScanner * 	s = NULL;
		bool 				ok = false;
		FAAObs_t			obs;
		int					rec_type;
		double				lat, lon;
		int					beacon_type;

	f = MemFile_Open(inFile);		if (f == NULL) goto bail;
	s = TextScanner_Open(f);	if (s == NULL) goto bail;


	while (!TextScanner_IsDone(s))
	{
		if (TextScanner_FormatScan(s, "i", &rec_type) == 1)
		{
			switch(rec_type) {
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
				if (TextScanner_FormatScan(s, "idd", &rec_type, &lat, &lon) == 3)
				{
					obs.kind = kRobinToFeatures[rec_type];
					obs.lat = lat;
					obs.lon = lon;
					obs.agl = obs.msl = DEM_NO_DATA;
					gFAAObs.insert(FAAObsTable::value_type(HashLonLat(obs.lon, obs.lat), obs));
				}
				break;

			case 19:
				if (TextScanner_FormatScan(s, "idd", &rec_type, &lat, &lon) == 3)
				{
					obs.kind = feat_Windsock;
					obs.lon = lon;
					obs.lat = lat;
					obs.agl = DEM_NO_DATA;
					obs.msl = DEM_NO_DATA;
					gFAAObs.insert(FAAObsTable::value_type(HashLonLat(obs.lon, obs.lat), obs));
				}
				break;
			case 18:
				if (TextScanner_FormatScan(s, "iddi", &rec_type, &lat, &lon, &beacon_type) == 4)
				if (beacon_type == 1 || beacon_type == 4)	// Land airport or military field
				{
					obs.kind = feat_RotatingBeacon;
					obs.lon = lon;
					obs.lat = lat;
					obs.agl = DEM_NO_DATA;
					obs.msl = DEM_NO_DATA;
					gFAAObs.insert(FAAObsTable::value_type(HashLonLat(obs.lon, obs.lat), obs));
				}
				break;
			}
		}
		TextScanner_Next(s);
	}
	ok = true;
bail:
	if (s) TextScanner_Close(s);
	if (f) MemFile_Close(f);
	return ok;
}

bool	LoadLegacyObjectArchive(const char * inFile)
{
	MFMemFile * f = MemFile_Open(inFile);
	MemFileReader reader(MemFile_GetBegin(f), MemFile_GetEnd(f), platform_BigEndian);
	int count;
	reader.ReadInt(count);
	while (count--)
	{
		double lat, lon, ele;
		int kind;
		reader.ReadInt(kind);
		reader.ReadDouble(lat);
		reader.ReadDouble(lon);
		reader.ReadDouble(ele);
		if (kind == 8)
		{
			char c;
			do {
				reader.ReadBulk(&c, 1,false);
			} while (c != 0);
		} else {
			FAAObs_t	obs;
			obs.agl = ele * FT_TO_MTR;
			obs.msl = DEM_NO_DATA;
			obs.lat = lat;
			obs.lon = lon;
			obs.kind = kConvertLegacyObjTypes[kind];
			if (obs.kind != NO_VALUE)
				gFAAObs.insert(FAAObsTable::value_type(HashLonLat(obs.lon, obs.lat), obs));
		}
	}
	MemFile_Close(f);
	return true;
}

void ApplyObjects(Pmwx& ioMap)
{
	if (gFAAObs.empty()) return;

	Point_2	sw, ne;
	CalcBoundingBox(ioMap, sw, ne);
// 	ioMap.Index();

	int	placed = 0;

	CGAL::Arr_landmarks_point_location<Arrangement_2>	locator(gMap);

	for (FAAObsTable::iterator i = gFAAObs.begin(); i != gFAAObs.end(); ++i)
	{
		if (i->second.kind != NO_VALUE)
		{
			Point_2 loc = Point_2(i->second.lon, i->second.lat);

			CGAL::Object obj = locator.locate(loc);
			Face_const_handle ff;
			if(CGAL::assign(ff,obj))
			{
				Face_handle f = ioMap.non_const_handle(ff);
				GISPointFeature_t	feat;
					feat.mFeatType = i->second.kind;
					feat.mLocation = loc;
					if (i->second.agl != DEM_NO_DATA)
						feat.mParams[pf_Height] = i->second.agl;
					feat.mInstantiated = false;
					f->data().mPointFeatures.push_back(feat);
					++placed;
	#if 0
					printf("Placed %s at %lf, %lf\n",
						FetchTokenString(i->second.kind), i->second.lon, i->second.lat);
	#endif
//				if (v.size() > 1)
//					fprintf(stderr,"WARNING (%d,%d): Point feature %lf, %lf matches multiple areas.\n",gMapWest, gMapSouth, CGAL::to_double(loc.x()), CGAL::to_double(loc.y()));

			}
		}
	}
	printf("Placed %d objects.\n", placed);
}


int	GetObjMinMaxHeights(map<int, float>& mins, map<int, float>& maxs)
{
	for (FAAObsTable::iterator obs = gFAAObs.begin(); obs != gFAAObs.end(); ++obs)
	if (obs->second.agl != DEM_NO_DATA)
	{
		if (mins.count(obs->second.kind) == 0)
			mins[obs->second.kind] = 999999;
		if (maxs.count(obs->second.kind) == 0)
			maxs[obs->second.kind] = 0;
		mins[obs->second.kind] = min(mins[obs->second.kind], obs->second.agl);
		maxs[obs->second.kind] = max(maxs[obs->second.kind], obs->second.agl);
	}
	return gFAAObs.size();
}
