#include "AptIO.h"
#include "ParamDefs.h"
#include "MemFileUtils.h"
#include "XESConstants.h"
#include "GISUtils.h"
#include "AssertUtils.h"

inline int hash_ll(int lon, int lat) {	return (lon+180) + 360 * (lat+90); }

enum {
	apt_Header = 1,
	apt_Seaplane = 16,
	apt_Heliport = 17,
	apt_Rwy = 10,
	apt_Tower = 14,
	apt_Ramp = 15,
	apt_Beacon = 18,
	apt_Windsock = 19,
	apt_ATCBase = 50,
	apt_ATCHighest = 56
};

static int	kATCCodes[] = {
					atc_Freq_AWOS,
					atc_Freq_UNICOM,
					atc_Freq_Clearance,
					atc_Freq_Ground,
					atc_Freq_Tower,
					atc_Freq_Departure,
					atc_Freq_Approach, -1 };
static int	kSurfaceCodes[] = {
					NO_VALUE,
					rwy_Surf_Asphalt,
					rwy_Surf_Concrete,
					rwy_Surf_Grass,
					rwy_Surf_Dirt,	
					rwy_Surf_Gravel,
					rwy_Surf_AsphaltHelo,
					rwy_Surf_ConcreteHelo,
					rwy_Surf_GrassHelo,	
					rwy_Surf_DirtHelo,
					rwy_Surf_AsphaltHoldLine,
					rwy_Surf_ConcreteHoldLine,
					rwy_Surf_DryLakebed,
					rwy_Surf_Waterway, -1 };
static int kVAPCode[] = {
					NO_VALUE,
					vap_Lite_None,
					vap_Lite_VASI,
					vap_Lite_PAPI,
					vap_Lite_PAPIShuttle, -1 };
static int kRunwayLiteCode[] = {
					NO_VALUE,
					rwy_Lite_None,
					rwy_Lite_Edge,		
					rwy_Lite_REIL,		
					rwy_Lite_CLL,		
					rwy_Lite_TDZ,		
					rwy_Lite_Taxi, -1 };
static int	kRunwayAppLiteCode[] = {
					NO_VALUE, 
					app_Lite_None,
					app_Lite_SSALS,		
					app_Lite_SALSF,		
					app_Lite_ALSFI,		
					app_Lite_ALSFII,	
					app_Lite_ODALS,		
					app_Lite_Calvert1,
					app_Lite_Calvert2, -1 };
static int	kShoulderCode[] = {
					rwy_Shoulder_None,
					rwy_Shoulder_Asphalt,
					rwy_Shoulder_Concrete, -1 };
static	int kRunwayMarkings[] = {
					rwy_Markings_None,
					rwy_Markings_Visual,
					rwy_Shoulder_Nonprecision,
					rwy_Shoulder_Precision, -1 };
static int kBeaconCodes[] = { 
					apt_Beacon_None,		
				    apt_Beacon_Airport,	
				    apt_Beacon_Seaport,	
				    apt_Beacon_Heliport,	
				    apt_Beacon_Military,	
				    apt_Beacon_WhiteStrobe, -1 };

int	EnumToCode(int eval, int * codes)
{
	int n = 0;
	while (codes[n] != -1 && codes[n] != eval)
		++n;
	DebugAssert(codes[n] != -1);
	return n;
}

void CalcRwyEndpoints(AptPavement_t * rwy, double heading, double len)
{
	// NOTE: if we were using some kind of cartesian projection scheme wedd have to add
	// (lon_ref - lon rwy) * sin(lat runway) to this degrees, rotating runways to the right 
	// of the reference point slightly CCW in the northern hemisphere to reflect the
	// map getting smaller at the pole.  But...we use a bogus projection, so - F it.
	double	heading_corrected = (heading) * DEG_TO_RAD;
	len *= 0.5;
	len *= FT_TO_MTR;
	Vector2	delta;
	delta.dx = len * sin(heading_corrected);
	delta.dy = len * cos(heading_corrected);
	
	Point2	ctr = rwy->ends.p1;
	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / cos(ctr.y * DEG_TO_RAD);
	delta.dx *= MTR_TO_DEG_LON;
	delta.dy *= MTR_TO_DEG_LAT;
	rwy->ends.p1 = ctr - delta;
	rwy->ends.p2 = ctr + delta;
}

void CalcRwyCenter(const AptPavement_t * rwy, Point2& center, double& len, double& heading)
{
	center = rwy->ends.midpoint();
	Vector2	dir(rwy->ends.p1, rwy->ends.p2);

	double	aspect = cos(center.y * DEG_TO_RAD);
	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / aspect;
	double DEG_TO_MTR_LON = DEG_TO_MTR_LAT * aspect;
	
	dir.dx *= DEG_TO_MTR_LON;
	dir.dy *= DEG_TO_MTR_LAT;
	
	len = sqrt(dir.squared_length());
	heading = RAD_TO_DEG * atan2(dir.dx, dir.dy);	
	if (heading < 0.0) heading += 360.0;
}

#if OPENGL_MAP
void CalcRwyOGL(int apt_code, AptPavement_t * rwy)
{
	double	aspect = cos(rwy->ends.midpoint().y * DEG_TO_RAD);
	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / aspect;
	double DEG_TO_MTR_LON = DEG_TO_MTR_LAT * aspect;
	
	double rwy_len = LonLatDistMetersWithScale(rwy->ends.p1.x, rwy->ends.p1.y, rwy->ends.p2.x, rwy->ends.p2.y, DEG_TO_MTR_LON, DEG_TO_MTR_LAT);
	Vector2	rwy_dir(rwy->ends.p1,  rwy->ends.p2);
	rwy_dir.dx *= DEG_TO_MTR_LON;
	rwy_dir.dy *= DEG_TO_MTR_LAT;
	
	rwy_dir.normalize();
	
	Vector2	rwy_right = rwy_dir.perpendicular_cw();
	Vector2	rwy_left = rwy_dir.perpendicular_ccw();
	rwy_right *= (rwy->width_ft * 0.5 * FT_TO_MTR);
	rwy_left *= (rwy->width_ft * 0.5 * FT_TO_MTR);

	rwy_left.dx *= MTR_TO_DEG_LON;
	rwy_left.dy *= MTR_TO_DEG_LAT;
	rwy_right.dx *= MTR_TO_DEG_LON;
	rwy_right.dy *= MTR_TO_DEG_LAT;
	
	Point2	pts[4];
	
	pts[0] = rwy->ends.p1 + rwy_left;
	pts[1] = rwy->ends.p2 + rwy_left;
	pts[2] = rwy->ends.p2 + rwy_right;
	pts[3] = rwy->ends.p1 + rwy_right;	
	for (int n = 0; n < 4; ++n)
	{
		rwy->quad_coords.push_back(pts[n].x);
		rwy->quad_coords.push_back(pts[n].y);
		if (apt_code == apt_Type_Seaport) {
			rwy->quad_colors.push_back(0.0);
			rwy->quad_colors.push_back(0.0);
			rwy->quad_colors.push_back(0.6);
		} else if (apt_code == apt_Type_Heliport) {
			rwy->quad_colors.push_back(0.6);
			rwy->quad_colors.push_back(0.0);
			rwy->quad_colors.push_back(0.3);
		} else {
			rwy->quad_colors.push_back(0.6);
			rwy->quad_colors.push_back(0.6);
			rwy->quad_colors.push_back(0.6);
		}
	}
	if (rwy->blast1_ft != 0.0)
	{
		pts[0] = rwy->ends.midpoint(-rwy->blast1_ft * FT_TO_MTR / rwy_len) + rwy_left;
		pts[1] = rwy->ends.p1 + rwy_left;
		pts[2] = rwy->ends.p1 + rwy_right;
		pts[3] = rwy->ends.midpoint(-rwy->blast1_ft * FT_TO_MTR / rwy_len) + rwy_right;	
		for (int n = 0; n < 4; ++n)
		{
			rwy->quad_coords.push_back(pts[n].x);
			rwy->quad_coords.push_back(pts[n].y);
			rwy->quad_colors.push_back(0.8);
			rwy->quad_colors.push_back(0.8);
			rwy->quad_colors.push_back(0.0);
		}		
	}
	if (rwy->blast2_ft != 0.0)
	{
		pts[0] = rwy->ends.p2 + rwy_left;
		pts[1] = rwy->ends.midpoint(1.0 + rwy->blast2_ft * FT_TO_MTR / rwy_len) + rwy_left;
		pts[2] = rwy->ends.midpoint(1.0 + rwy->blast2_ft * FT_TO_MTR / rwy_len) + rwy_right;
		pts[3] = rwy->ends.p2 + rwy_right;	
		for (int n = 0; n < 4; ++n)
		{
			rwy->quad_coords.push_back(pts[n].x);
			rwy->quad_coords.push_back(pts[n].y);
			rwy->quad_colors.push_back(0.8);
			rwy->quad_colors.push_back(0.8);
			rwy->quad_colors.push_back(0.0);
		}		
	}
	if (rwy->disp1_ft != 0.0)
	{
		pts[0] = rwy->ends.p1 + rwy_left;
		pts[1] = rwy->ends.midpoint(rwy->disp1_ft * FT_TO_MTR / rwy_len) + rwy_left;
		pts[2] = rwy->ends.midpoint(rwy->disp1_ft * FT_TO_MTR / rwy_len) + rwy_right;
		pts[3] = rwy->ends.p1 + rwy_right;	
		for (int n = 0; n < 4; ++n)
		{
			rwy->quad_coords.push_back(pts[n].x);
			rwy->quad_coords.push_back(pts[n].y);
			rwy->quad_colors.push_back(0.8);
			rwy->quad_colors.push_back(0.8);
			rwy->quad_colors.push_back(0.8);
		}		
	}
	if (rwy->disp2_ft != 0.0)
	{
		pts[0] = rwy->ends.midpoint(1.0 - rwy->disp2_ft * FT_TO_MTR / rwy_len) + rwy_left;
		pts[1] = rwy->ends.p2 + rwy_left;
		pts[2] = rwy->ends.p2 + rwy_right;
		pts[3] = rwy->ends.midpoint(1.0 - rwy->disp2_ft * FT_TO_MTR / rwy_len) + rwy_right;	
		for (int n = 0; n < 4; ++n)
		{
			rwy->quad_coords.push_back(pts[n].x);
			rwy->quad_coords.push_back(pts[n].y);
			rwy->quad_colors.push_back(0.8);
			rwy->quad_colors.push_back(0.8);
			rwy->quad_colors.push_back(0.8);
		}		
	}
}
#endif /* OPENGL_MAP */

bool	ReadAptFile(const char * inFileName, AptVector& outApts)
{
	outApts.clear();
	MFMemFile * f = MemFile_Open(inFileName);
	if (f == NULL) return false;
	
	bool ok = ReadAptFileMem(MemFile_GetBegin(f), MemFile_GetEnd(f), outApts);
	MemFile_Close(f);
	return ok;
}

bool	ReadAptFileMem(const char * inBegin, const char * inEnd, AptVector& outApts)
{
	outApts.clear();

	MFTextScanner * s = TextScanner_OpenMem(inBegin, inEnd);
	bool ok = true;
	TextScanner_Next(s);
	if (TextScanner_IsDone(s)) 
		ok = false;
		
	set<string>		centers;
	string			lat_str, lon_str, rot_str, len_str, wid_str;
	bool			hit_prob = false;
	
	bool forceDone = false;
	while (!TextScanner_IsDone(s) && !forceDone)
	{
		int		rec_code;
		string	dis, blas;
		int		len_code, liting_code, surf_code, shoulder_code, marking_code;
		int		bcn_code;
		float	rwy_heading;
		AptPavement_t * rwy;
		
		if (TextScanner_FormatScan(s, "i", &rec_code) != 1)
		{
			TextScanner_Next(s);
			continue;
		}
		
		switch(rec_code) {
		case apt_Header:
		case apt_Seaplane:
		case apt_Heliport:
			centers.clear();
			hit_prob = false;
			outApts.push_back(AptInfo_t());
			if (TextScanner_FormatScan(s, "iiiiTT|",
				&rec_code,
				&outApts.back().elevation_ft,
				&outApts.back().has_atc_twr,
				&outApts.back().default_buildings,
				&outApts.back().icao,
				&outApts.back().name) != 6) 
				ok = false;
			if (rec_code == apt_Header)	outApts.back().kind_code = apt_Type_Airport;
			if (rec_code == apt_Seaplane)	outApts.back().kind_code = apt_Type_Seaport;
			if (rec_code == apt_Heliport)	outApts.back().kind_code = apt_Type_Heliport;
			break;							
		case apt_Rwy:
			outApts.back().pavements.push_back(AptPavement_t());
			// Mark both of these as invalid until we get one.
			outApts.back().beacon.color_code = NO_VALUE;
			outApts.back().tower.draw_obj = -1;
			rwy = &outApts.back().pavements.back();
			if (outApts.back().kind_code == apt_Type_Airport)
			if (!hit_prob)
			if (TextScanner_FormatScan(s, " TT TT  T", &lat_str, &lon_str, &rot_str, &len_str, &wid_str) == 9)
			{
				lat_str += ' ';
				lat_str += lon_str;
				lat_str += ' ';
				lat_str += rot_str;
				lat_str += ' ';
				lat_str += len_str;
				lat_str += ' ';
				lat_str += wid_str;
				if (centers.count(lat_str) > 0)
				{
					hit_prob = true;
					printf("WARNING: duplicate runway for airport '%s' %s: %s\n", outApts.back().icao.c_str(), outApts.back().name.c_str(), lat_str.c_str());
				}
				centers.insert(lat_str);
			}
			if (TextScanner_FormatScan(s, "iddTfiTTdiiiifi",
					&rec_code,
					&rwy->ends.p1.y,
					&rwy->ends.p1.x,
					&rwy->name,
					&rwy_heading,
					&len_code,
					&dis,
					&blas,
					&rwy->width_ft,
					&liting_code,
					&surf_code,
					&shoulder_code,
					&marking_code,
					&rwy->roughness_ratio,
					&rwy->distance_markings) != 15)
				ok = false;
			if (sscanf(dis.c_str(),"%d.%d", &rwy->disp1_ft,&rwy->disp2_ft) != 2)
				ok = false;
			if (sscanf(blas.c_str(),"%d.%d", &rwy->blast1_ft,&rwy->blast2_ft) != 2)
				ok = false;
			rwy->vap_lites_code1 = kVAPCode			 [(liting_code / 100000) % 10];
			rwy->edge_lites_code1= kRunwayLiteCode   [(liting_code / 10000) % 10];
			rwy->app_lites_code1 = kRunwayAppLiteCode[(liting_code / 1000) % 10];
			rwy->vap_lites_code2 = kVAPCode			 [(liting_code / 100) % 10];
			rwy->edge_lites_code2= kRunwayLiteCode   [(liting_code / 10) % 10];
			rwy->app_lites_code2 = kRunwayAppLiteCode[(liting_code / 1) % 10];
			
			rwy->surf_code = kSurfaceCodes[surf_code];
			rwy->shoulder_code = kShoulderCode[shoulder_code];		
			rwy->marking_code = kRunwayMarkings[marking_code];		
			
			CalcRwyEndpoints(rwy, rwy_heading, len_code);
#if OPENGL_MAP
			CalcRwyOGL(outApts.back().kind_code,rwy);
#endif						
			break;
		case apt_Tower:
			if (TextScanner_FormatScan(s, "iddfiT|",
				&rec_code,
				&outApts.back().tower.location.y,
				&outApts.back().tower.location.x,
				&outApts.back().tower.height_ft,
				&outApts.back().tower.draw_obj,
				&outApts.back().tower.name) != 6)
			ok = false;
			break;				
		case apt_Ramp:
			outApts.back().gates.push_back(AptGate_t());
			if (TextScanner_FormatScan(s, "iddfT|", 
				&rec_code, 
				&outApts.back().gates.back().location.y,
				&outApts.back().gates.back().location.x,
				&outApts.back().gates.back().heading,
				&outApts.back().gates.back().name) != 5)
			ok = false;
			break;
		case apt_Beacon:
			if (TextScanner_FormatScan(s, "iddiT|",
				&rec_code,
				&outApts.back().beacon.location.y,
				&outApts.back().beacon.location.x,
				&bcn_code,
				&outApts.back().beacon.name) != 5)
			ok = false;
			outApts.back().beacon.color_code = kBeaconCodes[bcn_code];			
			break;
		case apt_Windsock:
			outApts.back().windsocks.push_back(AptWindsock_t());
			if (TextScanner_FormatScan(s, "iddiT|",
				&rec_code,
				&outApts.back().windsocks.back().location.y,
				&outApts.back().windsocks.back().location.x,
				&outApts.back().windsocks.back().lit,
				&outApts.back().windsocks.back().name) != 5)
			ok = false;
			break;
		case 600:
		case 703:
		case 715:
		case 810:
			break;
		case 99:
			forceDone = true;
			break;
		default:
			if (rec_code >= apt_ATCBase && rec_code <= apt_ATCHighest)
			{
				outApts.back().atc.push_back(AptATCFreq_t());
				if (TextScanner_FormatScan(s, "iiT|",
					&rec_code,
					&outApts.back().atc.back().freq,
					&outApts.back().atc.back().name) != 3)
				ok = false;
				outApts.back().atc.back().atc_type = kATCCodes[rec_code - apt_ATCBase];
			} else 
				ok = false;
			break;
		}
		TextScanner_Next(s);
	}
	TextScanner_Close(s);
	
	for (int a = 0; a < outApts.size(); ++a)
	{
		outApts[a].bounds = Bbox2(outApts[a].pavements[0].ends.p1);
	
//		outApts[a].bounds = Bbox2(outApts[a].tower.location);
//		outApts[a].bounds += outApts[a].beacon.location;
//		for (int w = 0; w < outApts[a].windsocks.size(); ++w)
//			outApts[a].bounds += outApts[a].windsocks[w].location;
//		for (int r = 0; r < outApts[a].gates.size(); ++r)
//			outApts[a].bounds += outApts[a].gates[r].location;
		for (int p = 0; p < outApts[a].pavements.size(); ++p)
		{
			outApts[a].bounds += outApts[a].pavements[p].ends.p1;
			outApts[a].bounds += outApts[a].pavements[p].ends.p2;
		}
	}
	return ok;
}

void	IndexAirports(const AptVector& apts, AptIndex& index)
{
	index.clear();
	for (int a = 0; a < apts.size(); ++ a)
	{
		Point2 ctr(
			(apts[a].bounds.xmin()+apts[a].bounds.xmax())*0.5,
			(apts[a].bounds.ymin()+apts[a].bounds.ymax())*0.5);
		
		int lon = ctr.x;
		int lat = ctr.y;
		index.insert(AptIndex::value_type(hash_ll(lon, lat), a));
	}
}

void	FindAirports(const Bbox2& bounds, const AptIndex& index, set<int>& apts)
{
	apts.clear();
	int x1 = floor(bounds.xmin() - 0.5);
	int x2 =  ceil(bounds.xmax() + 0.5);
	int y1 = floor(bounds.ymin() - 0.5);
	int y2 =  ceil(bounds.ymax() + 0.5);
	for (int x = x1; x <= x2; ++x)
	for (int y = y1; y <= y2; ++y)
	{
		pair<AptIndex::const_iterator,AptIndex::const_iterator>	range = index.equal_range(hash_ll(x,y));
		for (AptIndex::const_iterator i = range.first; i != range.second; ++i)
		{
			apts.insert(i->second);
		}
	}
}

bool	WriteAptFile(const char * inFileName, const AptVector& inApts)
{
	FILE * fi = fopen(inFileName, "w");
	if (fi == NULL) return false;
	bool ok = WriteAptFileOpen(fi, inApts);
	fclose(fi);
	return ok;
}


bool	WriteAptFileOpen(FILE * fi, const AptVector& inApts)
{	
	fprintf(fi, "%c" CRLF, APL ? 'A' : 'I');
	fprintf(fi, "715 Generated by WorldEditor" CRLF);
	
	
	for (AptVector::const_iterator apt = inApts.begin(); apt != inApts.end(); ++apt)
	{
		int 									apt_code = apt_Header;
		if (apt->kind_code == apt_Type_Seaport) apt_code = apt_Seaplane;
		if (apt->kind_code == apt_Type_Heliport)apt_code = apt_Heliport;
		fprintf(fi, CRLF);
		fprintf(fi, "%d %6d %d %d %s %s" CRLF, apt_code, apt->elevation_ft, 
				apt->has_atc_twr, apt->default_buildings, 
				apt->icao.c_str(), apt->name.c_str());

		for (AptPavementVector::const_iterator rwy = apt->pavements.begin(); rwy != apt->pavements.end(); ++rwy)
		{
			double heading, len;
			Point2	center;
			CalcRwyCenter(&*rwy, center, len, heading);
			fprintf(fi,"%2d % 010.6lf % 011.6lf %s %6.2lf %6.0lf %4d.%04d %4d.%04d %4.0lf "
					   "%d%d%d%d%d%d %02d %d %d %3.2f %d" CRLF, apt_Rwy,
				center.y, center.x, rwy->name.c_str(), heading, len * MTR_TO_FT,
				rwy->disp1_ft, rwy->disp2_ft, rwy->blast1_ft, rwy->blast2_ft, rwy->width_ft, 
				EnumToCode(rwy->vap_lites_code1, kVAPCode),
				EnumToCode(rwy->edge_lites_code1, kRunwayLiteCode),
				EnumToCode(rwy->app_lites_code1, kRunwayAppLiteCode),
				EnumToCode(rwy->vap_lites_code2, kVAPCode),
				EnumToCode(rwy->edge_lites_code2, kRunwayLiteCode),
				EnumToCode(rwy->app_lites_code2, kRunwayAppLiteCode),
				EnumToCode(rwy->surf_code, kSurfaceCodes),
				EnumToCode(rwy->shoulder_code, kShoulderCode),
				EnumToCode(rwy->marking_code, kRunwayMarkings),
				rwy->roughness_ratio, rwy->distance_markings);
		}
		
		if (apt->tower.draw_obj != -1)
			fprintf(fi, "%2d % 010.6lf % 011.6lf %6.2f %d %s" CRLF, apt_Tower,
				apt->tower.location.y, apt->tower.location.x, apt->tower.height_ft,
				apt->tower.draw_obj, apt->tower.name.c_str());
			
		for (AptGateVector::const_iterator gate = apt->gates.begin(); gate != apt->gates.end(); ++gate)
		{
			fprintf(fi, "%2d % 010.6lf % 011.6lf %6.2f %s" CRLF, apt_Ramp,
				gate->location.y, gate->location.x, gate->heading, gate->name.c_str());
		}	
		
		if (apt->beacon.color_code != NO_VALUE)
			fprintf(fi, "%2d % 010.6lf % 011.6lf %d %s" CRLF, apt_Beacon, apt->beacon.location.y,
				apt->beacon.location.x, EnumToCode(apt->beacon.color_code, kBeaconCodes), apt->beacon.name.c_str());
		
		for (AptWindsockVector::const_iterator sock = apt->windsocks.begin(); sock != apt->windsocks.end(); ++sock)
		{
			fprintf(fi, "%2d % 010.6lf % 011.6lf %d %s" CRLF, apt_Windsock, sock->location.y, sock->location.x,
				sock->lit, sock->name.c_str());
		}
		
		for (AptATCFreqVector::const_iterator atc = apt->atc.begin(); atc != apt->atc.end(); ++atc)
		{
			fprintf(fi, "%2d %d %s" CRLF, apt_ATCBase + EnumToCode(atc->atc_type, kATCCodes),
					atc->freq, atc->name.c_str());
		}
		
	}
	fprintf(fi, "99" CRLF);
	return true;
}
