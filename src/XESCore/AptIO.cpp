#include "AptIO.h"
#include "ParamDefs.h"
#include "MemFileUtils.h"
#include "XESConstants.h"
#include "GISUtils.h"
#include "AssertUtils.h"

inline int hash_ll(int lon, int lat) {	return (lon+180) + 360 * (lat+90); }

void	parse_linear_codes(const string& codes, set<int> * attributes)
{
	attributes->clear();
	MFScanner scanner;
	MFS_init(&scanner, &*codes.begin(), &*codes.end());
	int code;
	while (!MFS_done(&scanner) && ((code = MFS_int(&scanner)) != 0))
		attributes->insert(code);
}

void	print_apt_poly(FILE * fi, const AptPolygon_t& poly)
{
	for (AptPolygon_t::const_iterator s = poly.begin(); s != poly.end(); ++s)
	{
		fprintf(fi,"%d % 010.6lf % 011.6lf", s->code,s->pt.y,s->pt.x);
		if (s->code == apt_lin_crv || s->code == apt_rng_crv || s-> code == apt_end_crv)
		fprintf(fi," % 010.6lf % 011.6lf", s->ctrl.y,s->ctrl.x);
		if (s->code != apt_end_seg && s->code != apt_end_crv)
		for (set<int>::const_iterator a = s->attributes.begin(); a != s->attributes.end(); ++a)
			fprintf(fi," %d",*a);
		fprintf(fi,CRLF);
	}
}

void CenterToEnds(Point2 location, double heading, double len, Segment2& ends)
{
	// NOTE: if we were using some kind of cartesian projection scheme wedd have to add
	// (lon_ref - lon rwy) * sin(lat runway) to this degrees, rotating runways to the right 
	// of the reference point slightly CCW in the northern hemisphere to reflect the
	// map getting smaller at the pole.  But...we use a bogus projection, so - F it.
	double	heading_corrected = (heading) * DEG_TO_RAD;
	len *= 0.5;
	Vector2	delta;
	delta.dx = len * sin(heading_corrected);
	delta.dy = len * cos(heading_corrected);
	
	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / cos(location.y * DEG_TO_RAD);
	delta.dx *= MTR_TO_DEG_LON;
	delta.dy *= MTR_TO_DEG_LAT;
	ends.p1 = location - delta;
	ends.p2 = location + delta;
}

void EndsToCenter(const Segment2& ends, Point2& center, double& len, double& heading)
{
	center = ends.midpoint();
	Vector2	dir(ends.p1, ends.p2);

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
		if (apt_code == apt_seaport) {
			rwy->quad_colors.push_back(0.0);
			rwy->quad_colors.push_back(0.0);
			rwy->quad_colors.push_back(0.6);
		} else if (apt_code == apt_heliport) {
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
	string codez;
	string			lat_str, lon_str, rot_str, len_str, wid_str;
	bool			hit_prob = false;
	AptPolygon_t *	open_poly = NULL;
	Point2			pt,ctrl;	
	
	bool forceDone = false;
	while (!TextScanner_IsDone(s) && !forceDone)
	{
		int		rec_code;
		string	dis, blas;
		int		len_code, liting_code;
		Point2	center_loc;
		float	rwy_heading;
		AptPavement_t * rwy;
		
		if (TextScanner_FormatScan(s, "i", &rec_code) != 1)
		{
			TextScanner_Next(s);
			continue;
		}
		
		switch(rec_code) {
		case apt_airport:
		case apt_seaport:
		case apt_heliport:
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
			outApts.back().kind_code = rec_code;
			break;							
		case apt_rwy_old:
			outApts.back().pavements.push_back(AptPavement_t());
			// Mark both of these as invalid until we get one.
			outApts.back().beacon.color_code = NO_VALUE;
			outApts.back().tower.draw_obj = -1;
			rwy = &outApts.back().pavements.back();
			if (outApts.back().kind_code == apt_airport)
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
			if (TextScanner_FormatScan(s, "iddTfiTTfiiiifi",
					&rec_code,
					&center_loc.y,
					&center_loc.x,
					&rwy->name,
					&rwy_heading,
					&len_code,
					&dis,
					&blas,
					&rwy->width_ft,
					&liting_code,
					&rwy->surf_code,
					&rwy->shoulder_code,
					&rwy->marking_code,
					&rwy->roughness_ratio,
					&rwy->distance_markings) != 15)
				ok = false;
			if (sscanf(dis.c_str(),"%d.%d", &rwy->disp1_ft,&rwy->disp2_ft) != 2)
				ok = false;
			if (sscanf(blas.c_str(),"%d.%d", &rwy->blast1_ft,&rwy->blast2_ft) != 2)
				ok = false;
			rwy->vap_lites_code1 = (liting_code / 100000) % 10;
			rwy->edge_lites_code1= (liting_code / 10000) % 10;
			rwy->app_lites_code1 = (liting_code / 1000) % 10;
			rwy->vap_lites_code2 = (liting_code / 100) % 10;
			rwy->edge_lites_code2= (liting_code / 10) % 10;
			rwy->app_lites_code2 = (liting_code / 1) % 10;
			
			CenterToEnds(center_loc,rwy_heading,len_code * FT_TO_MTR, rwy->ends);
#if OPENGL_MAP
			CalcRwyOGL(outApts.back().kind_code,rwy);
#endif						
			break;
		case apt_tower_loc:
			if (TextScanner_FormatScan(s, "iddfiT|",
				&rec_code,
				&outApts.back().tower.location.y,
				&outApts.back().tower.location.x,
				&outApts.back().tower.height_ft,
				&outApts.back().tower.draw_obj,
				&outApts.back().tower.name) != 6)
			ok = false;
			break;				
		case apt_startup_loc:
			outApts.back().gates.push_back(AptGate_t());
			if (TextScanner_FormatScan(s, "iddfT|", 
				&rec_code, 
				&outApts.back().gates.back().location.y,
				&outApts.back().gates.back().location.x,
				&outApts.back().gates.back().heading,
				&outApts.back().gates.back().name) != 5)
			ok = false;
			break;
		case apt_beacon:
			if (TextScanner_FormatScan(s, "iddiT|",
				&rec_code,
				&outApts.back().beacon.location.y,
				&outApts.back().beacon.location.x,
				&outApts.back().beacon.color_code,
				&outApts.back().beacon.name) != 5)
			ok = false;
			break;
		case apt_windsock:
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
		case 850:
			break;
		case apt_sign:
			outApts.back().signs.push_back(AptSign_t());
			if (TextScanner_FormatScan(s,"iddfiiT|",
					&rec_code,
					&outApts.back().signs.back().location.y,
					&outApts.back().signs.back().location.x,
					&outApts.back().signs.back().heading,
					&outApts.back().signs.back().style_code,
					&outApts.back().signs.back().size_code,
					&outApts.back().signs.back().text) != 6)
			ok = false;
			break;
		case apt_papi:
			outApts.back().lights.push_back(AptLight_t());
			if (TextScanner_FormatScan(s, "iddiffT|",
					&rec_code,
					&outApts.back().lights.back().location.y,
					&outApts.back().lights.back().location.x,
					&outApts.back().lights.back().light_code,
					&outApts.back().lights.back().heading,
					&outApts.back().lights.back().angle,
					&outApts.back().lights.back().name) != 7)
			ok = false;
			break;
		case apt_rwy_new:
			outApts.back().runways.push_back(AptRunway_t());
			if (TextScanner_FormatScan(s, "ifiifiiiTddffiiiiTddffiiii",
				&rec_code,
				&outApts.back().runways.back().width_mtr,
				&outApts.back().runways.back().surf_code,
				&outApts.back().runways.back().shoulder_code,
				&outApts.back().runways.back().roughness_ratio,
				&outApts.back().runways.back().has_centerline,
				&outApts.back().runways.back().edge_light_code,
				&outApts.back().runways.back().has_distance_remaining,

				&outApts.back().runways.back().id[0],
				&outApts.back().runways.back().ends.p1.y,
				&outApts.back().runways.back().ends.p1.x,
				&outApts.back().runways.back().disp_mtr[0],
				&outApts.back().runways.back().blas_mtr[0],
				&outApts.back().runways.back().marking_code[0],
				&outApts.back().runways.back().app_light_code[0],
				&outApts.back().runways.back().has_tdzl[0],
				&outApts.back().runways.back().reil_code[0],

				&outApts.back().runways.back().id[1],
				&outApts.back().runways.back().ends.p2.y,
				&outApts.back().runways.back().ends.p2.x,
				&outApts.back().runways.back().disp_mtr[1],
				&outApts.back().runways.back().blas_mtr[1],
				&outApts.back().runways.back().marking_code[1],
				&outApts.back().runways.back().app_light_code[1],
				&outApts.back().runways.back().has_tdzl[1],
				&outApts.back().runways.back().reil_code[1]) != 26)
			ok = false;
			break;
		case apt_sea_new:
			outApts.back().sealanes.push_back(AptSealane_t());
			if (TextScanner_FormatScan(s, "ifiTddTdd",
				&rec_code,
				&outApts.back().sealanes.back().width_mtr,
				&outApts.back().sealanes.back().has_buoys,
				&outApts.back().sealanes.back().id[0],
				&outApts.back().sealanes.back().ends.p1.y,
				&outApts.back().sealanes.back().ends.p1.x,
				&outApts.back().sealanes.back().id[2],
				&outApts.back().sealanes.back().ends.p2.y,
				&outApts.back().sealanes.back().ends.p2.x) != 9)
			ok = false;
			break;
		case apt_heli_new:
			outApts.back().helipads.push_back(AptHelipad_t());
			if (TextScanner_FormatScan(s,"iTddfffiiifi",
				&rec_code,
				&outApts.back().helipads.back().id,
				&outApts.back().helipads.back().location.y,
				&outApts.back().helipads.back().location.x,
				&outApts.back().helipads.back().heading,
				&outApts.back().helipads.back().length_mtr,
				&outApts.back().helipads.back().width_mtr,
				&outApts.back().helipads.back().surface_code,
				&outApts.back().helipads.back().marking_code,
				&outApts.back().helipads.back().shoulder_code,
				&outApts.back().helipads.back().roughness_ratio,
				&outApts.back().helipads.back().edge_light_code) != 12)
			ok = false;
			break;		
		case apt_taxi_new:
			outApts.back().taxiways.push_back(AptTaxiway_t());
			if (TextScanner_FormatScan(s,"iiffT|",
				&rec_code,
				&outApts.back().taxiways.back().surface_code,
				&outApts.back().taxiways.back().roughness_ratio,
				&outApts.back().taxiways.back().heading,
				&outApts.back().taxiways.back().name) != 5)
			ok = false;			
			open_poly = &outApts.back().taxiways.back().area;
			break;
		case apt_free_chain:
			outApts.back().lines.push_back(AptMarking_t());
			if (TextScanner_FormatScan(s,"iT|",&rec_code,&outApts.back().lines.back().name) != 2) 
				ok = false;
			open_poly = &outApts.back().lines.back().area;
			break;
		case apt_boundary:
			outApts.back().boundaries.push_back(AptBoundary_t());
			if (TextScanner_FormatScan(s,"iT|",&rec_code,&outApts.back().boundaries.back().name) != 2) 
				ok = false;
			open_poly = &outApts.back().boundaries.back().area;
			break;
		case apt_lin_seg:
		case apt_rng_seg:
			open_poly->push_back(AptLinearSegment_t());			
			if (TextScanner_FormatScan(s,"iddT|",
				&open_poly->back().code,
				&open_poly->back().pt.y,
				&open_poly->back().pt.x,
				&codez) != 4) ok = false;
			parse_linear_codes(codez,&open_poly->back().attributes);
			break;
		case apt_lin_crv:
		case apt_rng_crv:
			open_poly->push_back(AptLinearSegment_t());			
			if (TextScanner_FormatScan(s,"iddddT|",
				&open_poly->back().code,
				&open_poly->back().pt.y,
				&open_poly->back().pt.x,
				&open_poly->back().ctrl.y,
				&open_poly->back().ctrl.x,
				&codez) != 6) ok = false;
			parse_linear_codes(codez,&open_poly->back().attributes);
			break;
		case apt_end_seg:
			open_poly->push_back(AptLinearSegment_t());			
			if (TextScanner_FormatScan(s,"idd",
				&open_poly->back().code,
				&open_poly->back().pt.y,
				&open_poly->back().pt.x) != 3) ok = false;
			break;
		case apt_end_crv:
			open_poly->push_back(AptLinearSegment_t());			
			if (TextScanner_FormatScan(s,"idddd",
				&open_poly->back().code,
				&open_poly->back().pt.y,
				&open_poly->back().pt.x,
				&open_poly->back().ctrl.y,
				&open_poly->back().ctrl.x) != 5) ok = false;
			parse_linear_codes(codez,&open_poly->back().attributes);
			break;
		case apt_done:
			forceDone = true;
			break;
		default:
			if (rec_code >= apt_freq_awos && rec_code <= apt_freq_ctr)
			{
				outApts.back().atc.push_back(AptATCFreq_t());
				if (TextScanner_FormatScan(s, "iiT|",
					&outApts.back().atc.back().atc_type,
					&outApts.back().atc.back().freq,
					&outApts.back().atc.back().name) != 3)
				ok = false;
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
		fprintf(fi, CRLF);
		fprintf(fi, "%d %6d %d %d %s %s" CRLF, apt->kind_code, apt->elevation_ft, 
				apt->has_atc_twr, apt->default_buildings, 
				apt->icao.c_str(), apt->name.c_str());

		for (AptRunwayVector::const_iterator rwy = apt->runways.begin(); rwy != apt->runways.end(); ++rwy)
		{
			fprintf(fi,"%d %4.0f %d %d %.2f %d %d %d "
						"%s % 010.6lf % 011.6lf %4.0f %4.0f %d %d %d %d "
						"%s % 010.6lf % 011.6lf %4.0f %4.0f %d %d %d %d" CRLF,
						apt_rwy_new, rwy->width_mtr, rwy->surf_code, rwy->shoulder_code, rwy->roughness_ratio, rwy->has_centerline, rwy->edge_light_code, rwy->has_distance_remaining,
						rwy->id[0].c_str(),rwy->ends.p1.y,rwy->ends.p1.x, rwy->disp_mtr[0],rwy->blas_mtr[0], rwy->marking_code[0],rwy->app_light_code[0], rwy->has_tdzl[0], rwy->reil_code[0],
						rwy->id[1].c_str(),rwy->ends.p2.y,rwy->ends.p2.x, rwy->disp_mtr[1],rwy->blas_mtr[1], rwy->marking_code[1],rwy->app_light_code[1], rwy->has_tdzl[1], rwy->reil_code[1]);
		}

		for(AptSealaneVector::const_iterator sea = apt->sealanes.begin(); sea != apt->sealanes.end(); ++sea)
		{
			fprintf(fi,"%d %4.0f %d %s % 010.6lf % 011.6lf %s % 010.6lf % 011.6lf" CRLF,
					apt_sea_new, sea->width_mtr, sea->has_buoys,
					sea->id[0].c_str(), sea->ends.p1.y, sea->ends.p1.x,
					sea->id[1].c_str(), sea->ends.p2.y, sea->ends.p2.x);
		}

		for (AptPavementVector::const_iterator pav = apt->pavements.begin(); pav != apt->pavements.end(); ++pav)
		{
			double heading, len;
			Point2	center;
			EndsToCenter(pav->ends, center, len, heading);
			fprintf(fi,"%2d % 010.6lf % 011.6lf %s %6.2lf %6.0lf %4d.%04d %4d.%04d %4.0f "
					   "%d%d%d%d%d%d %02d %d %d %3.2f %d" CRLF, apt_rwy_old,
				center.y, center.x, pav->name.c_str(), heading, len * MTR_TO_FT,
				pav->disp1_ft, pav->disp2_ft, pav->blast1_ft, pav->blast2_ft, pav->width_ft, 
				pav->vap_lites_code1,
				pav->edge_lites_code1,
				pav->app_lites_code1,
				pav->vap_lites_code2,
				pav->edge_lites_code2,
				pav->app_lites_code2, 
				pav->surf_code, 
				pav->shoulder_code,
				pav->marking_code, 
				pav->roughness_ratio, pav->distance_markings);
		}
		

		for(AptHelipadVector::const_iterator heli = apt->helipads.begin(); heli != apt->helipads.end(); ++heli)
		{
			fprintf(fi,"%d %s % 010.6lf % 011.6lf %6.2lf %4.0f %4.0f %d %d %d %.2f %d" CRLF,
				apt_heli_new, heli->id.c_str(), heli->location.y, heli->location.x, heli->heading, heli->length_mtr, heli->width_mtr,
						heli->surface_code,heli->marking_code,heli->shoulder_code,heli->roughness_ratio,heli->edge_light_code);		
		}

		for (AptTaxiwayVector::const_iterator taxi = apt->taxiways.begin(); taxi != apt->taxiways.end(); ++taxi)
		{
			fprintf(fi, "%d %d %.2f %6.4f %s" CRLF, apt_taxi_new, taxi->surface_code, taxi->roughness_ratio, taxi->heading, taxi->name.c_str());
			print_apt_poly(fi,taxi->area);
		}

		for (AptBoundaryVector::const_iterator bound = apt->boundaries.begin(); bound != apt->boundaries.end(); ++bound)
		{
			fprintf(fi, "%d %s" CRLF, apt_boundary, bound->name.c_str());
			print_apt_poly(fi,bound->area);
		}

		for (AptMarkingVector::const_iterator lin = apt->lines.begin(); lin != apt->lines.end(); ++lin)
		{
			fprintf(fi, "%d %s" CRLF, apt_free_chain, lin->name.c_str());
			print_apt_poly(fi,lin->area);
		}

		for (AptLightVector::const_iterator light = apt->lights.begin(); light != apt->lights.end(); ++light)
		{
			fprintf(fi,"%d % 010.6lf % 011.6lf %d %6.4lf %3.1f %s" CRLF,
					apt_papi, light->location.y, light->location.x, light->light_code,
					light->heading, light->angle, light->name.c_str());
		}

		for (AptSignVector::const_iterator sign = apt->signs.begin(); sign != apt->signs.end(); ++sign)
		{
			fprintf(fi,"%d % 010.6lf % 011.6lf %6.4lf %d %d %s" CRLF,
					apt_sign, sign->location.y, sign->location.x, sign->heading,
					sign->style_code, sign->size_code, sign->text.c_str());
		}


		if (apt->tower.draw_obj != -1)
			fprintf(fi, "%2d % 010.6lf % 011.6lf %6.2f %d %s" CRLF, apt_tower_loc,
				apt->tower.location.y, apt->tower.location.x, apt->tower.height_ft,
				apt->tower.draw_obj, apt->tower.name.c_str());
			
		for (AptGateVector::const_iterator gate = apt->gates.begin(); gate != apt->gates.end(); ++gate)
		{
			fprintf(fi, "%2d % 010.6lf % 011.6lf %6.2f %s" CRLF, apt_startup_loc,
				gate->location.y, gate->location.x, gate->heading, gate->name.c_str());
		}	
		
		if (apt->beacon.color_code != NO_VALUE)
			fprintf(fi, "%2d % 010.6lf % 011.6lf %d %s" CRLF, apt_beacon, apt->beacon.location.y,
				apt->beacon.location.x, apt->beacon.color_code, apt->beacon.name.c_str());
		
		for (AptWindsockVector::const_iterator sock = apt->windsocks.begin(); sock != apt->windsocks.end(); ++sock)
		{
			fprintf(fi, "%2d % 010.6lf % 011.6lf %d %s" CRLF, apt_windsock, sock->location.y, sock->location.x,
				sock->lit, sock->name.c_str());
		}
		
		for (AptATCFreqVector::const_iterator atc = apt->atc.begin(); atc != apt->atc.end(); ++atc)
		{
			fprintf(fi, "%2d %d %s" CRLF, atc->atc_type,
					atc->freq, atc->name.c_str());
		}
		
	}
	fprintf(fi, "%d" CRLF, apt_done);
	return true;
}
