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

#include "AptIO.h"
#include "ParamDefs.h"
#include "MemFileUtils.h"
#if OPENGL_MAP
//#include "Airports.h"
#endif
#include "XESConstants.h"
#include "GISUtils.h"
#include "AssertUtils.h"
#include "CompGeomUtils.h"
#include "STLUtils.h"

// for now
#define	ATC_VERS 1000


#if OPENGL_MAP
#include "Airports.h"
void	GenerateOGL(AptInfo_t * a);
#endif

const char * ramp_type_strings[] = { "misc","gate","tie_down","hangar", 0 };
const char * pattern_strings[] = { "left", "right", 0 };
const char * equip_strings[] = { "heavy", "jets", "turboprops", "props", "helos", 0 };
const char * op_strings[] = { "arrivals", "departures" };

// LLLHHH
void divide_heading(int * lo, int * hi)
{
	*hi = (*lo % 1000);
	*lo = (*lo / 1000);
}

int scan_bitfields(const char * str, const char * bits[], int all_value)
{
	if(all_value && strcmp(str,"all") == 0)
		return all_value;
		
	int r = 0;
	int n = 0;
	int b = 1;
	while(bits[n])
	{
		if(strstr(str, bits[n]))
			r |= b;
		++n;
		b <<= 1;
	}
	return r;
}

int scan_enum(const char * str, const char * enums[])
{
	int n = 0;
	while(enums[n])
	{
		if(strstr(str, enums[n]))
			return n;
		++n;
	}
	return -1;
}


void print_bitfields(int(*fprintf)(void *, const char *, ...), void * fi, int flags, const char * bits[])
{
	int n = 0, b = 1;
	bool any = false;
	while(bits[n])
	{
		if(flags & b)
		{
			if(any) fprintf(fi,"|");
			any = true;
			fprintf(fi,"%s", bits[n]);
		}
		++n;
		b <<= 1;
	}
}

static void	parse_linear_codes(const string& codes, set<int> * attributes)
{
	attributes->clear();
	MFScanner scanner;
	MFS_init(&scanner, &*codes.begin(), &*codes.end());
	int code;
	while (!MFS_done(&scanner) && ((code = MFS_int(&scanner)) != 0))
		attributes->insert(code);
}

static void	print_apt_poly(int (*fprintf)(void * fi, const char * fmt, ...), void * fi, const AptPolygon_t& poly)
{
	for (AptPolygon_t::const_iterator s = poly.begin(); s != poly.end(); ++s)
	{
		fprintf(fi,"%d % 012.8lf % 013.8lf", s->code,CGAL2DOUBLE(s->pt.y()),CGAL2DOUBLE(s->pt.x()));
		if (s->code == apt_lin_crv || s->code == apt_rng_crv || s-> code == apt_end_crv)
		fprintf(fi," % 012.8lf % 013.8lf", CGAL2DOUBLE(s->ctrl.y()),CGAL2DOUBLE(s->ctrl.x()));
		if (s->code != apt_end_seg && s->code != apt_end_crv)
		for (set<int>::const_iterator a = s->attributes.begin(); a != s->attributes.end(); ++a)
			fprintf(fi," %d",*a);
		fprintf(fi,CRLF);
	}
}

static void CenterToEnds(POINT2 location, double heading, double len, SEGMENT2& ends)
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

	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / cos(location.y() * DEG_TO_RAD);
	delta.dx *= MTR_TO_DEG_LON;
	delta.dy *= MTR_TO_DEG_LAT;
	ends.p1 = location - delta;
	ends.p2 = location + delta;
}

static void EndsToCenter(const Segment2& ends, Point2& center, double& len, double& heading)
{
	center = ends.midpoint();
	Vector2	dir(ends.p1, ends.p2);

	double	aspect = cos(center.y() * DEG_TO_RAD);
	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / aspect;
	double DEG_TO_MTR_LON = DEG_TO_MTR_LAT * aspect;

	dir.dx *= DEG_TO_MTR_LON;
	dir.dy *= DEG_TO_MTR_LAT;

	len = sqrt(dir.squared_length());
	heading = RAD_TO_DEG * atan2(dir.dx, dir.dy);
	if (heading < 0.0) heading += 360.0;
}

static void CenterToCorners(Point2 location, double heading, double len, double width, Point2 corners[4])
{
	// NOTE: if we were using some kind of cartesian projection scheme wedd have to add
	// (lon_ref - lon rwy) * sin(lat runway) to this degrees, rotating runways to the right
	// of the reference point slightly CCW in the northern hemisphere to reflect the
	// map getting smaller at the pole.  But...we use a bogus projection, so - F it.
	double	heading_corrected = (heading) * DEG_TO_RAD;
	len *= 0.5;
	width *= 0.5;
	Vector2	delta;
	delta.dx = len * sin(heading_corrected);
	delta.dy = len * cos(heading_corrected);

	Vector2	lateral;
	lateral.dx = width *  cos(heading_corrected);
	lateral.dy = width * -sin(heading_corrected);

	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / cos(location.y() * DEG_TO_RAD);
	delta.dx *= MTR_TO_DEG_LON;
	delta.dy *= MTR_TO_DEG_LAT;
	lateral.dx *= MTR_TO_DEG_LON;
	lateral.dy *= MTR_TO_DEG_LAT;

	corners[0] = location - lateral - delta;
	corners[1] = location - lateral + delta;
	corners[2] = location + lateral + delta;
	corners[3] = location + lateral - delta;
}


string	ReadAptFile(const char * inFileName, AptVector& outApts)
{
	outApts.clear();
	MFMemFile * f = MemFile_Open(inFileName);
	if (f == NULL) return string("memfile_open failed");

	string err = ReadAptFileMem(MemFile_GetBegin(f), MemFile_GetEnd(f), outApts);
	MemFile_Close(f);
	return err;
}

string	ReadAptFileMem(const char * inBegin, const char * inEnd, AptVector& outApts)
{
	outApts.clear();

	MFTextScanner * s = TextScanner_OpenMem(inBegin, inEnd);
	string ok;

	int ln = 0;

	// Versioning:
	// 703 (base)
	// 715 - addded vis flag to tower
	// 810 - added vasi slope to towers
	// 850 - added next-gen stuff

		int vers = 0;

	if (TextScanner_IsDone(s))
		ok = string("File is empty.");
	if (ok.empty())
	{
		string app_win;
		if (TextScanner_FormatScan(s, "T", &app_win) != 1) ok = "Invalid header";
		if (app_win != "a" && app_win != "A" && app_win != "i" && app_win != "I") ok = string("Invalid header:") + app_win;
		TextScanner_Next(s);
		++ln;
	}
	if (ok.empty())
	{
		if (TextScanner_FormatScan(s, "i", &vers) != 1) ok = "Invalid version";
		if (vers != 703 && vers != 715 && vers != 810 && vers != 850 && vers != 1000) ok = "Illegal Version";
		TextScanner_Next(s);
		++ln;
	}

	set<string>		centers;
	string codez;
	string			lat_str, lon_str, rot_str, len_str, wid_str;
	bool			hit_prob = false;
	AptPolygon_t *	open_poly = NULL;
	Point2			pt,ctrl;

	bool forceDone = false;
	while (ok.empty() && !TextScanner_IsDone(s) && !forceDone)
	{
		int		rec_code;
		string	dis, blas, vasi;
		int		len_code, liting_code;
		POINT2	center_loc;
		float	rwy_heading;
		AptPavement_t * rwy;
		double p1x, p1y, p2x, p2y;

		if (TextScanner_FormatScan(s, "i", &rec_code) != 1)
		{
			TextScanner_Next(s);
			++ln;
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
				ok = "Illegal line (airport, seaport or heliport)";
			outApts.back().kind_code = rec_code;
			outApts.back().beacon.color_code = apt_beacon_none;
			outApts.back().tower.draw_obj = -1;
			break;
		case apt_rwy_old:
			outApts.back().pavements.push_back(AptPavement_t());
			// Mark both of these as invalid until we get one.
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
			if (TextScanner_FormatScan(s, "iddTfiTTfiiiifiT",
					&rec_code,
					&p1y,
					&p1x,
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
					&rwy->distance_markings,
					&vasi) < 15)
				ok = "Illegal old runway";
				center_loc = POINT2(p1x, p1y);
			if (sscanf(dis.c_str(),"%d.%d", &rwy->disp1_ft,&rwy->disp2_ft) != 2)
				ok = string("Illegal displaced threshholds in old runway") + dis;
			if (sscanf(blas.c_str(),"%d.%d", &rwy->blast1_ft,&rwy->blast2_ft) != 2)
				ok = string("Illegal blast-pads in old runway: ") + blas;

			rwy->vasi_angle1 = rwy->vasi_angle2 = 300;
			if (vers >= 810)
			{
				if (sscanf(blas.c_str(),"%d.%d", &rwy->vasi_angle1,&rwy->vasi_angle2) != 2)
					ok = string("Illegal VASI in old runway: ") + blas;
			}
			if(rwy->vasi_angle1 == 0) rwy->vasi_angle1 = 300;
			if(rwy->vasi_angle2 == 0) rwy->vasi_angle2 = 300;

			rwy->vap_lites_code1 = (liting_code / 100000) % 10;
			rwy->edge_lites_code1= (liting_code / 10000) % 10;
			rwy->app_lites_code1 = (liting_code / 1000) % 10;
			rwy->vap_lites_code2 = (liting_code / 100) % 10;
			rwy->edge_lites_code2= (liting_code / 10) % 10;
			rwy->app_lites_code2 = (liting_code / 1) % 10;

			CenterToEnds(center_loc,rwy_heading,len_code * FT_TO_MTR, rwy->ends);
			break;
		case apt_tower_loc:
			if (TextScanner_FormatScan(s, "iddfiT|",
				&rec_code,
				&p1y,
				&p1x,
				&outApts.back().tower.height_ft,
				&outApts.back().tower.draw_obj,
				&outApts.back().tower.name) < (vers >= 715 ? 5 : 4))
			ok = "Illegal tower loc";
				outApts.back().tower.location = POINT2(p1x, p1y);
			if(vers < 715) outApts.back().tower.draw_obj = 1;
			break;
		case apt_startup_loc:
			outApts.back().gates.push_back(AptGate_t());
			if (TextScanner_FormatScan(s, "iddfT|",
				&rec_code,
				&p1y,
				&p1x,
				&outApts.back().gates.back().heading,
				&outApts.back().gates.back().name) < 4)
			ok = "Illegal startup loc";
				outApts.back().gates.back().location = POINT2(p1x, p1y);
				outApts.back().gates.back().type = atc_ramp_misc;
				outApts.back().gates.back().equipment = atc_traffic_all;
			break;
		case apt_beacon:
			if (TextScanner_FormatScan(s, "iddiT|",
				&rec_code,
				&p1y,
				&p1x,
				&outApts.back().beacon.color_code,
				&outApts.back().beacon.name) < 4)
			ok = "Illegal apt beacon";
				outApts.back().beacon.location = POINT2(p1x, p1y);
			break;
		case apt_windsock:
			outApts.back().windsocks.push_back(AptWindsock_t());
			if (TextScanner_FormatScan(s, "iddiT|",
				&rec_code,
				&p1y,
				&p1x,
				&outApts.back().windsocks.back().lit,
				&outApts.back().windsocks.back().name) < 4)
			ok = "Illegal windsock";
				outApts.back().windsocks.back().location = POINT2(p1x, p1y);
			break;
		case apt_sign:
			if (vers < 850) ok = "Error: apt signs not allowed before 850";
			outApts.back().signs.push_back(AptSign_t());
			if (TextScanner_FormatScan(s,"iddfiiT|",
					&rec_code,
					&p1y,
					&p1x,
					&outApts.back().signs.back().heading,
					&outApts.back().signs.back().style_code,
					&outApts.back().signs.back().size_code,
					&outApts.back().signs.back().text) != 7)
			ok = "Illegal apt sign";
				outApts.back().signs.back().location = POINT2(p1x, p1y);
			break;
		case apt_papi:
			if (vers < 850) ok = "Error: stand-alone light fixtures not allowed before 850";
			outApts.back().lights.push_back(AptLight_t());
			if (TextScanner_FormatScan(s, "iddiffT|",
					&rec_code,
					&p1y,
					&p1x,
					&outApts.back().lights.back().light_code,
					&outApts.back().lights.back().heading,
					&outApts.back().lights.back().angle,
					&outApts.back().lights.back().name) < 6)
			ok = "Illegal PAPI";
				outApts.back().lights.back().location = POINT2(p1x, p1y);
			break;
		case apt_rwy_new:
			if (vers < 850) ok = "Error: new runways not allowed before 850";
			outApts.back().runways.push_back(AptRunway_t());


			if (outApts.back().kind_code == apt_airport)
			if (!hit_prob)
			if (TextScanner_FormatScan(s, " T       TT       TT",&wid_str,&lat_str,&lon_str,&rot_str,&len_str)==20)
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
				&p1y,
				&p1x,
				&outApts.back().runways.back().disp_mtr[0],
				&outApts.back().runways.back().blas_mtr[0],
				&outApts.back().runways.back().marking_code[0],
				&outApts.back().runways.back().app_light_code[0],
				&outApts.back().runways.back().has_tdzl[0],
				&outApts.back().runways.back().reil_code[0],

				&outApts.back().runways.back().id[1],
				&p2y,
				&p2x,
				&outApts.back().runways.back().disp_mtr[1],
				&outApts.back().runways.back().blas_mtr[1],
				&outApts.back().runways.back().marking_code[1],
				&outApts.back().runways.back().app_light_code[1],
				&outApts.back().runways.back().has_tdzl[1],
				&outApts.back().runways.back().reil_code[1]) != 26)
			ok = "Illegal new runway";
			outApts.back().runways.back().ends = SEGMENT2(POINT2(p1x, p1y), POINT2(p2x, p2y));
			break;
		case apt_sea_new:
			if (vers < 850) ok = "Error: new sealanes not allowed before 850";
			outApts.back().sealanes.push_back(AptSealane_t());
			if (TextScanner_FormatScan(s, "ifiTddTdd",
				&rec_code,
				&outApts.back().sealanes.back().width_mtr,
				&outApts.back().sealanes.back().has_buoys,
				&outApts.back().sealanes.back().id[0],
				&p1y,
				&p1x,
				&outApts.back().sealanes.back().id[1],
				&p2y,
				&p2x) != 9)
			ok = "Illegal new seaway";
			outApts.back().sealanes.back().ends = SEGMENT2(POINT2(p1x, p1y), POINT2(p2x, p2y));
			break;
		case apt_heli_new:
			if (vers < 850) ok = "Error: new helipads not allowed before 850";
			outApts.back().helipads.push_back(AptHelipad_t());
			if (TextScanner_FormatScan(s,"iTddfffiiifi",
				&rec_code,
				&outApts.back().helipads.back().id,
				&p1y,
				&p1x,
				&outApts.back().helipads.back().heading,
				&outApts.back().helipads.back().length_mtr,
				&outApts.back().helipads.back().width_mtr,
				&outApts.back().helipads.back().surface_code,
				&outApts.back().helipads.back().marking_code,
				&outApts.back().helipads.back().shoulder_code,
				&outApts.back().helipads.back().roughness_ratio,
				&outApts.back().helipads.back().edge_light_code) != 12)
			ok = "Illegal new helipad";
			outApts.back().helipads.back().location = POINT2(p1x, p1y);
			break;
		case apt_taxi_new:
			if (vers < 850) ok = "Error: new taxiways not allowed before 850";
			outApts.back().taxiways.push_back(AptTaxiway_t());
			if (TextScanner_FormatScan(s,"iiffT|",
				&rec_code,
				&outApts.back().taxiways.back().surface_code,
				&outApts.back().taxiways.back().roughness_ratio,
				&outApts.back().taxiways.back().heading,
				&outApts.back().taxiways.back().name) < 4)
			ok = "Illegal new taxi";
			open_poly = &outApts.back().taxiways.back().area;
			break;
		case apt_free_chain:
			if (vers < 850) ok = "Error: new free lines not allowed before 850";
			outApts.back().lines.push_back(AptMarking_t());
			if (TextScanner_FormatScan(s,"iT|",&rec_code,&outApts.back().lines.back().name) < 1)
				ok = "Illegal free chain";
			open_poly = &outApts.back().lines.back().area;
			break;
		case apt_boundary:
			if (vers < 850) ok = "Error: new apt boundary not allowed before 850";
			outApts.back().boundaries.push_back(AptBoundary_t());
			if (TextScanner_FormatScan(s,"iT|",&rec_code,&outApts.back().boundaries.back().name) < 1)
				ok = "Illegal boundary";
			open_poly = &outApts.back().boundaries.back().area;
			break;
		case apt_lin_seg:
		case apt_rng_seg:
			if (vers < 850) ok = "Error: new linear segments allowed before 850";
			codez.clear();
			open_poly->push_back(AptLinearSegment_t());
			if (TextScanner_FormatScan(s,"iddT|",
				&open_poly->back().code,
				&p1y,
				&p1x,
				&codez) < 3) ok = "Illegal straight segment";
				open_poly->back().pt = POINT2(p1x, p1y);
			parse_linear_codes(codez,&open_poly->back().attributes);
			break;
		case apt_lin_crv:
		case apt_rng_crv:
			if (vers < 850) ok = "Error: new curved segments allowed before 850";
			codez.clear();
			open_poly->push_back(AptLinearSegment_t());
			if (TextScanner_FormatScan(s,"iddddT|",
				&open_poly->back().code,
				&p1y,
				&p1x,
				&p2y,
				&p2x,
				&codez) < 5) ok = "Illegal curved segment";
				open_poly->back().pt = POINT2(p1x, p1y);
				open_poly->back().ctrl = POINT2(p2x, p2y);
			parse_linear_codes(codez,&open_poly->back().attributes);
			break;
		case apt_end_seg:
			if (vers < 850) ok = "Error: new end segments allowed before 850";
			open_poly->push_back(AptLinearSegment_t());
			if (TextScanner_FormatScan(s,"idd",
				&open_poly->back().code,
				&p1y,
				&p1x) != 3) ok = "Illegal straight end.";
				open_poly->back().pt = POINT2(p1x, p1y);
			break;
		case apt_end_crv:
			if (vers < 850) ok = "Error: new end curves allowed before 850";
			codez.clear();
			open_poly->push_back(AptLinearSegment_t());
			if (TextScanner_FormatScan(s,"idddd",
				&open_poly->back().code,
				&p1y,
				&p1x,
				&p2y,
				&p2x) != 5) ok = "Illegal curved end";
			open_poly->back().pt = POINT2(p1x, p1y);
			open_poly->back().ctrl = POINT2(p2x, p2y);
			parse_linear_codes(codez,&open_poly->back().attributes);
			break;
		case apt_startup_loc_new:
			if(vers < ATC_VERS) ok = "Error: no new ramp starts in older apt.dat files.";
			else if(outApts.empty()) ok = "Error: new ramp start outside airport.";
			else
			{
				AptGate_t gate;
				string equip, ramp_type;
				if (TextScanner_FormatScan(s, "iddfTTT|",
					&rec_code,
					&p1y,
					&p1x,
					&gate.heading,
					&ramp_type,
					&equip,
					&gate.name) < 4)
					ok = "Illegal startup loc";
				else		
				{					
					gate.type = scan_enum(ramp_type.c_str(), ramp_type_strings);
					if(gate.type == -1)
						ok = "Illegal ramp type";
					else 
					{
						gate.equipment = scan_bitfields(equip.c_str(), equip_strings, atc_traffic_all);
						gate.location = POINT2(p1x, p1y);
						outApts.back().gates.push_back(gate);
					}
				}
			}
			break;			
		case apt_flow_def:
			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: flow outside an airport.";
			else {
				outApts.back().flows.push_back(AptFlow_t());
				if(TextScanner_FormatScan(s,"iT|",
					&rec_code,
					&outApts.back().flows.back().name) != 2) ok = "Error: bad apt flow record.";
			}
			break;
		case apt_flow_wind:
			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: wind rule outside of an airport";
			else if(outApts.back().flows.empty()) ok = "Error: wind rule outside of flow.";
			else
			{
				AptWindRule_t	wr;
				if(TextScanner_FormatScan(s,"iTiii",&rec_code,&wr.icao,&wr.dir_lo_degs_mag, &wr.dir_hi_degs_mag,&wr.max_speed_knots) != 5)
					ok = "ERROR: bad wind rule record.";
				else				
					outApts.back().flows.back().wind_rules.push_back(wr);
			}
			break;

		case apt_flow_ceil:
			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: ceiling rule outside of an airport";
			else if(outApts.back().flows.empty()) ok = "Error: ceiling rule outside of flow.";
			else
			{				
				if(TextScanner_FormatScan(s,"iTi",&rec_code,&outApts.back().flows.back().icao,&outApts.back().flows.back().ceiling_ft) != 3)
					ok = "ERROR: ceiling wind rule record.";
			}
			break;

		case apt_flow_vis:
			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: vis rule outside of an airport";
			else if(outApts.back().flows.empty()) ok = "Error: vis rule outside of flow.";
			else
			{
				if(TextScanner_FormatScan(s,"iTi",&rec_code,&outApts.back().flows.back().icao,&outApts.back().flows.back().visibility_sm) != 3)
					ok = "ERROR: bad vis rule record.";
			}
			break;
		case apt_flow_time:
			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: time rule outside of an airport";
			else if(outApts.back().flows.empty()) ok = "Error: time rule outside of flow.";
			else
			{
				AptTimeRule_t	tr;
				if(TextScanner_FormatScan(s,"iii",&rec_code,&tr.start_zulu,&tr.end_zulu) != 3)
					ok = "ERROR: bad time rule record.";
				else				
					outApts.back().flows.back().time_rules.push_back(tr);
			}
			break;
		case apt_flow_pattern:
			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: traffic pattern outside an airport.";
			else if (outApts.back().flows.empty()) ok = "Error: traffic pattern outside a flow.";
			else {
				string side;
				if(TextScanner_FormatScan(s,"iTT", &rec_code,
					&outApts.back().flows.back().pattern_runway,
					&side) != 3) ok =  "Error: incorrect pattrn record";
				else
					outApts.back().flows.back().pattern_side = scan_bitfields(side.c_str(), pattern_strings,0);
			}
			break;
		case apt_flow_rwy_rule:
			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: runway use outside an airport.";
			else if (outApts.back().flows.empty()) ok = "Error: runway use outside a flow.";
			else {
				outApts.back().flows.back().runway_rules.push_back(AptRunwayRule_t());
				string op, equip;
				if(TextScanner_FormatScan(s,"iTiTTiiT|", &rec_code,
					&outApts.back().flows.back().runway_rules.back().runway,
					&outApts.back().flows.back().runway_rules.back().dep_freq,
					&op,
					&equip,
					&outApts.back().flows.back().runway_rules.back().dep_heading_lo,
					&outApts.back().flows.back().runway_rules.back().ini_heading_lo,
					&outApts.back().flows.back().runway_rules.back().name) != 8) ok = "Error: incorrect runway use rule.";
				else
				{
					outApts.back().flows.back().runway_rules.back().operations = scan_bitfields(op.c_str(),op_strings, atc_op_all);
					outApts.back().flows.back().runway_rules.back().equipment = scan_bitfields(equip.c_str(), equip_strings, atc_traffic_all);
					divide_heading(
										&outApts.back().flows.back().runway_rules.back().dep_heading_lo,
										&outApts.back().flows.back().runway_rules.back().dep_heading_hi);
					divide_heading(
										&outApts.back().flows.back().runway_rules.back().ini_heading_lo,
										&outApts.back().flows.back().runway_rules.back().ini_heading_hi);
				}
			}
			break;


		case apt_taxi_header:
			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: taxi layout use outside an airport.";
			else {
				if(TextScanner_FormatScan(s,"iT|",&rec_code, &outApts.back().taxi_route.name) < 1)
					ok = "Illegal taxi layout header.";
			}
			break;
		case apt_taxi_node:
			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: taxi layout node outside an airport.";
			else {
				outApts.back().taxi_route.nodes.push_back(AptRouteNode_t());
				string flags;
				if(TextScanner_FormatScan(s,"iddTiT|",
					&rec_code,
					&outApts.back().taxi_route.nodes.back().location.y_,
					&outApts.back().taxi_route.nodes.back().location.x_,
					&flags,
					&outApts.back().taxi_route.nodes.back().id,
					&outApts.back().taxi_route.nodes.back().name) < 5) ok = "illegal taxi node.";
			}
			break;
		case apt_taxi_edge:
			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: taxi layout edge outside an airport.";
			else {
				outApts.back().taxi_route.edges.push_back(AptRouteEdge_t());
				string oneway_flag, runway_flag;
				if(TextScanner_FormatScan(s,"iiiTTT|",
					&rec_code,
					&outApts.back().taxi_route.edges.back().src,
					&outApts.back().taxi_route.edges.back().dst,
					&oneway_flag,
					&runway_flag,
					&outApts.back().taxi_route.edges.back().name) < 5) ok = "Error: illegal taxi layout edge.";
				outApts.back().taxi_route.edges.back().oneway = oneway_flag == "oneway";
				outApts.back().taxi_route.edges.back().runway = runway_flag == "runway";

			}
			break;
		case apt_taxi_shape:
			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: taxi layout edge outside an airport.";
			else if (outApts.back().taxi_route.edges.empty()) ok = "Error: taxi layout shape point without an edge.";
			else {
				outApts.back().taxi_route.edges.back().shape.push_back(Point2());
				if(TextScanner_FormatScan(s,"idd", &rec_code,
					&outApts.back().taxi_route.edges.back().shape.back().y_,
					&outApts.back().taxi_route.edges.back().shape.back().x_) != 3) ok = "Error: illegal shape point record.";
			}
			break;
		case apt_taxi_active:

			if(vers < ATC_VERS) ok = "Error: no ATC data in older apt.dat files.";
			else if (outApts.empty()) ok = "Error: taxi active zone outside an airport.";
			else if (outApts.back().taxi_route.edges.empty()) ok = "Error: taxi taxi active zone without an edge.";
			else {
				string flags, runways;
				if(TextScanner_FormatScan(s,"iTT", &rec_code, &flags, &runways) != 3) ok = "Error: illegal active zone record.";
				vector<string> runways_parsed;
				tokenize_string(runways.begin(), runways.end(), back_inserter(runways_parsed), ',');
				if(flags.find("departure") != flags.npos)
					copy(runways_parsed.begin(),runways_parsed.end(), set_inserter(outApts.back().taxi_route.edges.back().hot_depart));
				if(flags.find("arrival") != flags.npos)
					copy(runways_parsed.begin(),runways_parsed.end(), set_inserter(outApts.back().taxi_route.edges.back().hot_arrive));
				if(flags.find("ils") != flags.npos)
					copy(runways_parsed.begin(),runways_parsed.end(), set_inserter(outApts.back().taxi_route.edges.back().hot_ils));
			}
			break;

		case apt_done:
			forceDone = true;
			break;
		default:
			if (rec_code >= apt_freq_awos && rec_code <= apt_freq_dep)
			{
				outApts.back().atc.push_back(AptATCFreq_t());
				if (TextScanner_FormatScan(s, "iiT|",
					&outApts.back().atc.back().atc_type,
					&outApts.back().atc.back().freq,
					&outApts.back().atc.back().name) < 2)	// ATC name can be blank in v9...sketchy but apparently true.
				ok = "Illegal ATC frequency";
			} else
				ok = "Illegal unknown record";
			break;
		}
		TextScanner_Next(s);
		++ln;
	}
	TextScanner_Close(s);

	if (!ok.empty())
	{
		char buf[50];
		sprintf(buf," (Line %d)",ln);
		ok += buf;
	}

	for (AptVector::iterator a = outApts.begin(); a != outApts.end(); ++a)
	{
		a->bounds = Bbox2();
		if (a->tower.draw_obj != -1)
			a->bounds = Bbox2(a->tower.location);
		if(a->beacon.color_code != apt_beacon_none)
			a->bounds += a->beacon.location;
		for (int w = 0; w < a->windsocks.size(); ++w)
			a->bounds += a->windsocks[w].location;
		for (int r = 0; r < a->gates.size(); ++r)
			a->bounds += a->gates[r].location;
		for (AptPavementVector::iterator p = a->pavements.begin(); p != a->pavements.end(); ++p)
		{
			a->bounds +=  p->ends.source();
			a->bounds +=  p->ends.target();
		}
		for (AptRunwayVector::iterator r = a->runways.begin(); r != a->runways.end(); ++r)
		{
			a->bounds +=  r->ends.source();
			a->bounds +=  r->ends.target();
		}
		for(AptSealaneVector::iterator s = a->sealanes.begin(); s != a->sealanes.end(); ++s)
		{
			a->bounds +=  s->ends.source();
			a->bounds +=  s->ends.target();
		}
		for(AptHelipadVector::iterator h = a->helipads.begin(); h != a->helipads.end(); ++h)
			a->bounds +=  h->location;

		for(AptTaxiwayVector::iterator t = a->taxiways.begin(); t != a->taxiways.end(); ++t)
		for(AptPolygon_t::iterator pt = t->area.begin(); pt != t->area.end(); ++pt)
		{
			a->bounds +=  pt->pt;
			if(pt->code == apt_lin_crv || pt->code == apt_rng_crv || pt-> code == apt_end_crv)
				a->bounds +=  pt->ctrl;
		}

		for(AptBoundaryVector::iterator b = a->boundaries.begin(); b != a->boundaries.end(); ++b)
		for(AptPolygon_t::iterator pt = b->area.begin(); pt != b->area.end(); ++pt)
		{
			a->bounds +=  pt->pt;
			if(pt->code == apt_lin_crv || pt->code == apt_rng_crv || pt-> code == apt_end_crv)
				a->bounds +=  pt->ctrl;
		}

		//a->bounds.expand(0.001);

		#if OPENGL_MAP
			GenerateOGL(&*a);
		#endif
	}
	return ok;
}

bool	WriteAptFile(const char * inFileName, const AptVector& inApts, int version)
{
	FILE * fi = fopen(inFileName, "wb");
	if (fi == NULL) return false;
	bool ok = WriteAptFileOpen(fi, inApts, version);
	fclose(fi);
	return ok;
}


bool	WriteAptFileOpen(FILE * fi, const AptVector& inApts, int version)
{
	return WriteAptFileProcs((int (*)(void *, const char *,...))fprintf,fi,inApts,version);
}

bool	WriteAptFileProcs(int (* fprintf)(void * fi, const char * fmt, ...), void * fi, const AptVector& inApts, int version)
{
	DebugAssert(version == 850 || version == 1000);
	fprintf(fi, "%c" CRLF, APL ? 'A' : 'I');
	fprintf(fi, "%d Generated by WorldEditor" CRLF, version);


	bool has_atc = (version >= 1000);

	for (AptVector::const_iterator apt = inApts.begin(); apt != inApts.end(); ++apt)
	{
		fprintf(fi, CRLF);
		fprintf(fi, "%d %6d %d %d %s %s" CRLF, apt->kind_code, apt->elevation_ft,
				apt->has_atc_twr, apt->default_buildings,
				apt->icao.c_str(), apt->name.c_str());

		for (AptRunwayVector::const_iterator rwy = apt->runways.begin(); rwy != apt->runways.end(); ++rwy)
		{
			fprintf(fi,"%d %4.2f %d %d %.2f %d %d %d "
						"%s % 012.8lf % 013.8lf %4.0f %4.0f %d %d %d %d "
						"%s % 012.8lf % 013.8lf %4.0f %4.0f %d %d %d %d" CRLF,
						apt_rwy_new, rwy->width_mtr, rwy->surf_code, rwy->shoulder_code, rwy->roughness_ratio, rwy->has_centerline, rwy->edge_light_code, rwy->has_distance_remaining,
						rwy->id[0].c_str(),CGAL2DOUBLE(rwy->ends.source().y()),CGAL2DOUBLE(rwy->ends.source().x()), rwy->disp_mtr[0],rwy->blas_mtr[0], rwy->marking_code[0],rwy->app_light_code[0], rwy->has_tdzl[0], rwy->reil_code[0],
						rwy->id[1].c_str(),CGAL2DOUBLE(rwy->ends.target().y()),CGAL2DOUBLE(rwy->ends.target().x()), rwy->disp_mtr[1],rwy->blas_mtr[1], rwy->marking_code[1],rwy->app_light_code[1], rwy->has_tdzl[1], rwy->reil_code[1]);
		}

		for(AptSealaneVector::const_iterator sea = apt->sealanes.begin(); sea != apt->sealanes.end(); ++sea)
		{
			fprintf(fi,"%d %4.2f %d %s % 012.8lf % 013.8lf %s % 012.8lf % 013.8lf" CRLF,
					apt_sea_new, sea->width_mtr, sea->has_buoys,
					sea->id[0].c_str(), CGAL2DOUBLE(sea->ends.source().y()), CGAL2DOUBLE(sea->ends.source().x()),
					sea->id[1].c_str(), CGAL2DOUBLE(sea->ends.target().y()), CGAL2DOUBLE(sea->ends.target().x()));
		}

		for (AptPavementVector::const_iterator pav = apt->pavements.begin(); pav != apt->pavements.end(); ++pav)
		{
			double heading, len;
			POINT2	center;
			EndsToCenter(pav->ends, center, len, heading);
			fprintf(fi,"%2d % 012.8lf % 013.8lf %s %6.2lf %6.0lf %4d.%04d %4d.%04d %4.0f "
					   "%d%d%d%d%d%d %02d %d %d %3.2f %d %3d.%03d" CRLF, apt_rwy_old,
				CGAL2DOUBLE(center.y()), CGAL2DOUBLE(center.x()), pav->name.c_str(), heading, len * MTR_TO_FT,
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
				pav->roughness_ratio, pav->distance_markings, pav->vasi_angle1, pav->vasi_angle2);
		}


		for(AptHelipadVector::const_iterator heli = apt->helipads.begin(); heli != apt->helipads.end(); ++heli)
		{
			fprintf(fi,"%d %s % 012.8lf % 013.8lf %6.2lf %4.2f %4.2f %d %d %d %.2f %d" CRLF,
				apt_heli_new, heli->id.c_str(), CGAL2DOUBLE(heli->location.y()), CGAL2DOUBLE(heli->location.x()), heli->heading, heli->length_mtr, heli->width_mtr,
						heli->surface_code,heli->marking_code,heli->shoulder_code,heli->roughness_ratio,heli->edge_light_code);
		}

		for (AptTaxiwayVector::const_iterator taxi = apt->taxiways.begin(); taxi != apt->taxiways.end(); ++taxi)
		{
			fprintf(fi, "%d %d %.2f %6.4f %s" CRLF, apt_taxi_new, taxi->surface_code, taxi->roughness_ratio, taxi->heading, taxi->name.c_str());
			print_apt_poly(fprintf,fi,taxi->area);
		}

		for (AptBoundaryVector::const_iterator bound = apt->boundaries.begin(); bound != apt->boundaries.end(); ++bound)
		{
			fprintf(fi, "%d %s" CRLF, apt_boundary, bound->name.c_str());
			print_apt_poly(fprintf,fi,bound->area);
		}

		for (AptMarkingVector::const_iterator lin = apt->lines.begin(); lin != apt->lines.end(); ++lin)
		{
			fprintf(fi, "%d %s" CRLF, apt_free_chain, lin->name.c_str());
			print_apt_poly(fprintf,fi,lin->area);
		}

		for (AptLightVector::const_iterator light = apt->lights.begin(); light != apt->lights.end(); ++light)
		{
			fprintf(fi,"%d % 012.8lf % 013.8lf %d %6.4lf %3.1f %s" CRLF,
					apt_papi, CGAL2DOUBLE(light->location.y()), CGAL2DOUBLE(light->location.x()), light->light_code,
					light->heading, light->angle, light->name.c_str());
		}

		for (AptSignVector::const_iterator sign = apt->signs.begin(); sign != apt->signs.end(); ++sign)
		{
			fprintf(fi,"%d % 012.8lf % 013.8lf %6.4lf %d %d %s" CRLF,
					apt_sign, CGAL2DOUBLE(sign->location.y()), CGAL2DOUBLE(sign->location.x()), sign->heading,
					sign->style_code, sign->size_code, sign->text.c_str());
		}


		if (apt->tower.draw_obj != -1)
			fprintf(fi, "%2d % 012.8lf % 013.8lf %6.2f %d %s" CRLF, apt_tower_loc,
				CGAL2DOUBLE(apt->tower.location.y()), CGAL2DOUBLE(apt->tower.location.x()), apt->tower.height_ft,
				apt->tower.draw_obj, apt->tower.name.c_str());

		for (AptGateVector::const_iterator gate = apt->gates.begin(); gate != apt->gates.end(); ++gate)
		{
			if((gate->type == atc_ramp_misc && gate->equipment == atc_traffic_all) || gate->equipment == 0 || !has_atc)
				fprintf(fi, "%2d % 012.8lf % 013.8lf %6.2f %s" CRLF, apt_startup_loc,
					CGAL2DOUBLE(gate->location.y()), CGAL2DOUBLE(gate->location.x()), gate->heading, gate->name.c_str());
			else
			{
				fprintf(fi, "%2d % 012.8lf % 013.8lf %6.2f %s ",apt_startup_loc_new, CGAL2DOUBLE(gate->location.y()), CGAL2DOUBLE(gate->location.x()), gate->heading, ramp_type_strings[gate->type]);
				print_bitfields(fprintf,fi,gate->equipment, equip_strings);
				fprintf(fi," %s" CRLF, gate->name.c_str());
			}
		}

		if (apt->beacon.color_code != apt_beacon_none)
			fprintf(fi, "%2d % 012.8lf % 013.8lf %d %s" CRLF, apt_beacon,CGAL2DOUBLE( apt->beacon.location.y()),
				CGAL2DOUBLE(apt->beacon.location.x()), apt->beacon.color_code, apt->beacon.name.c_str());

		for (AptWindsockVector::const_iterator sock = apt->windsocks.begin(); sock != apt->windsocks.end(); ++sock)
		{
			fprintf(fi, "%2d % 012.8lf % 013.8lf %d %s" CRLF, apt_windsock, CGAL2DOUBLE(sock->location.y()), CGAL2DOUBLE(sock->location.x()),
				sock->lit, sock->name.c_str());
		}

		if(has_atc)
		{
			for (AptATCFreqVector::const_iterator atc = apt->atc.begin(); atc != apt->atc.end(); ++atc)
			{
				fprintf(fi, "%2d %d %s" CRLF, atc->atc_type,
						atc->freq, atc->name.c_str());
			}

			for(AptFlowVector::const_iterator flow = apt->flows.begin(); flow != apt->flows.end(); ++flow)
			{
				fprintf(fi,"%2d %s" CRLF, apt_flow_def, flow->name.c_str());
				
				for(AptWindRuleVector::const_iterator wind = flow->wind_rules.begin(); wind != flow->wind_rules.end(); ++wind)
					fprintf(fi,"%2d %s %03d %03d %d" CRLF, apt_flow_wind, wind->icao.c_str(), wind->dir_lo_degs_mag, wind->dir_hi_degs_mag, wind->max_speed_knots);
				
				fprintf(fi,"%2d %s %d" CRLF, apt_flow_ceil, flow->icao.c_str(), flow->ceiling_ft);
				
				fprintf(fi,"%2d %s %d" CRLF, apt_flow_vis, flow->icao.c_str(), flow->visibility_sm);
				
				for(AptTimeRuleVector::const_iterator time = flow->time_rules.begin(); time != flow->time_rules.end(); ++time)
					fprintf(fi,"%2d %04d %04d" CRLF, apt_flow_time, time->start_zulu, time->end_zulu);

				if(!flow->pattern_runway.empty() && flow->pattern_side)
				{
					fprintf(fi,"%02d %s ", apt_flow_pattern, flow->pattern_runway.c_str());
					print_bitfields(fprintf,fi,flow->pattern_side,pattern_strings);
					fprintf(fi,CRLF);
				}

				for(AptRunwayRuleVector::const_iterator	rule = flow->runway_rules.begin(); rule != flow->runway_rules.end(); ++rule)
				{
					fprintf(fi,"%2d %s %6d ",apt_flow_rwy_rule, rule->runway.c_str(), rule->dep_freq);
					print_bitfields(fprintf,fi,rule->operations, op_strings);
					fprintf(fi," ");
					print_bitfields(fprintf,fi,rule->equipment, equip_strings);
					fprintf(fi," %03d%03d %03d%03d %s" CRLF, rule->dep_heading_lo, rule->dep_heading_hi, rule->ini_heading_lo, rule->ini_heading_hi, rule->name.c_str());
				}
			}

			if(!apt->taxi_route.edges.empty())
			{
				fprintf(fi,"%2d %s" CRLF, apt_taxi_header, apt->taxi_route.name.c_str());
				for(vector<AptRouteNode_t>::const_iterator n = apt->taxi_route.nodes.begin(); n != apt->taxi_route.nodes.end(); ++n)
				{
					fprintf(fi,"%2d % 012.8lf % 013.8lf both %d %s" CRLF, apt_taxi_node, n->location.y(), n->location.x(), n->id, n->name.c_str());
				}
				for(vector<AptRouteEdge_t>::const_iterator e = apt->taxi_route.edges.begin(); e != apt->taxi_route.edges.end(); ++e)
				{
					fprintf(fi,"%2d %d %d %s %s %s" CRLF, apt_taxi_edge, e->src, e->dst, e->oneway ? "oneway" : "twoway", e->runway ? "runway" : "taxiway", e->name.c_str());
					for(vector<Point2>::const_iterator s = e->shape.begin(); s != e->shape.end(); ++s)
						fprintf(fi,"%2d % 012.8lf % 013.8lf" CRLF, apt_taxi_shape, s->y(), s->x());

					if(!e->hot_depart.empty())
					{
						fprintf(fi,"%2d departure", apt_taxi_active);
						for(set<string>::const_iterator s = e->hot_depart.begin(); s != e->hot_depart.end(); ++s)
							fprintf(fi,"%c%s", s == e->hot_depart.begin() ? ' ' : ',', s->c_str());
						fprintf(fi,CRLF);
					}
					if(!e->hot_arrive.empty())
					{
						fprintf(fi,"%2d arrival", apt_taxi_active);
						for(set<string>::const_iterator s = e->hot_arrive.begin(); s != e->hot_arrive.end(); ++s)
							fprintf(fi,"%c%s", s == e->hot_arrive.begin() ? ' ' : ',', s->c_str());
						fprintf(fi,CRLF);
					}
					if(!e->hot_ils.empty())
					{
						fprintf(fi,"%2d ils", apt_taxi_active);
						for(set<string>::const_iterator s = e->hot_ils.begin(); s != e->hot_ils.end(); ++s)
							fprintf(fi,"%c%s", s == e->hot_ils.begin() ? ' ' : ',', s->c_str());
						fprintf(fi,CRLF);
					}
				}
			}
		}
	}
	fprintf(fi, "%d" CRLF, apt_done);
	return true;
}


#if OPENGL_MAP

static void OGL_push_quad(AptInfo_t *		io_airport, float r, float g, float b, const POINT2 p[4])
{
	io_airport->ogl.push_back(AptInfo_t::AptLineLoop_t());
	io_airport->ogl.back().rgb[0] = r;
	io_airport->ogl.back().rgb[1] = g;
	io_airport->ogl.back().rgb[2] = b;
	io_airport->ogl.back().pts.insert(io_airport->ogl.back().pts.end(),p,p+4);
}

static void CalcPavementBezier(AptInfo_t * io_airport, AptPolygon_t * poly, float r, float  g, float b, float simp)
{
	vector<vector<Bezier2> >	windings;
	AptPolygonToBezier(*poly, windings);

	for(vector<vector<Bezier2> >::iterator w = windings.begin(); w != windings.end(); ++w)
	{
		io_airport->ogl.push_back(AptInfo_t::AptLineLoop_t());
		AptInfo_t::AptLineLoop_t * l = &io_airport->ogl.back();
		l->rgb[0] = r; l->rgb[1] = g; l->rgb[2] = b;
		Polygon_2 temp;
		BezierToSegments(*w, temp,simp);
		for(Polygon_2::Vertex_iterator v = temp.vertices_begin(); v != temp.vertices_end(); ++v)
			l->pts.push_back(cgal2ben(*v));

//		io_airport->ogl.push_back(AptInfo_t::AptLineLoop_t());
//		l = &io_airport->ogl.back();
//		l->rgb[0] = b; l->rgb[1] = g; l->rgb[2] = r;
//		BezierToSegments(*w, l->pts,true);
	}
}

static void CalcPavementOGL(
					AptInfo_t *		io_airport,
					const SEGMENT2&	ends,
					float			width_mtr,
					float			blas1_mtr,
					float			blas2_mtr,
					float			disp1_mtr,
					float			disp2_mtr)
{
	double	aspect = cos(ends.midpoint().y() * DEG_TO_RAD);
	double MTR_TO_DEG_LON = MTR_TO_DEG_LAT / aspect;
	double DEG_TO_MTR_LON = DEG_TO_MTR_LAT * aspect;

	double rwy_len = LonLatDistMetersWithScale(ends.p1.x_, ends.p1.y_, ends.p2.x_, ends.p2.y_, DEG_TO_MTR_LON, DEG_TO_MTR_LAT);
	VECTOR2	rwy_dir(ends.p1,  ends.p2);
	rwy_dir.dx *= DEG_TO_MTR_LON;
	rwy_dir.dy *= DEG_TO_MTR_LAT;

	rwy_dir.normalize();

	VECTOR2	rwy_right = rwy_dir.perpendicular_cw();
	VECTOR2	rwy_left = rwy_dir.perpendicular_ccw();
	rwy_right *= (width_mtr * 0.5);
	rwy_left *= (width_mtr * 0.5);

	rwy_left.dx *= MTR_TO_DEG_LON;
	rwy_left.dy *= MTR_TO_DEG_LAT;
	rwy_right.dx *= MTR_TO_DEG_LON;
	rwy_right.dy *= MTR_TO_DEG_LAT;

	POINT2	pts[4];

	pts[0] = ends.p1 + rwy_left;
	pts[1] = ends.p2 + rwy_left;
	pts[2] = ends.p2 + rwy_right;
	pts[3] = ends.p1 + rwy_right;

		 if (io_airport->kind_code == apt_seaport) 	OGL_push_quad(io_airport, 0.0,0.0,0.6, pts);
	else if (io_airport->kind_code == apt_heliport)	OGL_push_quad(io_airport, 0.6,0.0,0.3, pts);
	else											OGL_push_quad(io_airport, 0.6,0.6,0.6, pts);

	if (blas1_mtr != 0.0)
	{
		pts[0] = ends.midpoint(-blas1_mtr / rwy_len) + rwy_left;
		pts[1] = ends.p1 + rwy_left;
		pts[2] = ends.p1 + rwy_right;
		pts[3] = ends.midpoint(-blas1_mtr / rwy_len) + rwy_right;
		OGL_push_quad(io_airport, 0.8,0.8,0.0, pts);
	}
	if (blas2_mtr != 0.0)
	{
		pts[0] = ends.p2 + rwy_left;
		pts[1] = ends.midpoint(1.0 + blas2_mtr / rwy_len) + rwy_left;
		pts[2] = ends.midpoint(1.0 + blas2_mtr / rwy_len) + rwy_right;
		pts[3] = ends.p2 + rwy_right;
		OGL_push_quad(io_airport, 0.8,0.8,0.0, pts);
	}
	if (disp1_mtr != 0.0)
	{
		pts[0] = ends.p1 + rwy_left;
		pts[1] = ends.midpoint(disp1_mtr / rwy_len) + rwy_left;
		pts[2] = ends.midpoint(disp1_mtr / rwy_len) + rwy_right;
		pts[3] = ends.p1 + rwy_right;
		OGL_push_quad(io_airport, 0.8,0.8,0.8, pts);
	}
	if (disp2_mtr != 0.0)
	{
		pts[0] = ends.midpoint(1.0 - disp2_mtr / rwy_len) + rwy_left;
		pts[1] = ends.p2 + rwy_left;
		pts[2] = ends.p2 + rwy_right;
		pts[3] = ends.midpoint(1.0 - disp2_mtr / rwy_len) + rwy_right;
		OGL_push_quad(io_airport, 0.8,0.8,0.8, pts);
	}
}

static void CalcPavementHelipad(AptInfo_t * io_airport, const POINT2& c, float h, float w, float rwy_len)
{
	SEGMENT2	e;
	CenterToEnds(c,h,rwy_len,e);
	CalcPavementOGL(io_airport,e,w,0,0,0,0);
}


void	GenerateOGL(AptInfo_t * a)
{
	a->ogl.clear();
	for(AptTaxiwayVector::iterator t = a->taxiways.begin(); t != a->taxiways.end(); ++t)
		CalcPavementBezier(&*a, &t->area,0.5,0.5,1.0,0.0);

	for(AptBoundaryVector::iterator b = a->boundaries.begin(); b != a->boundaries.end(); ++b)
		CalcPavementBezier(&*a, &b->area,1.0,0.5,0.5,0.0);

	for(AptRunwayVector::iterator r = a->runways.begin(); r != a->runways.end(); ++r)
		CalcPavementOGL(a, r->ends,
							r->width_mtr,
							r->blas_mtr[0],
							r->blas_mtr[1],
							r->disp_mtr[0],
							r->disp_mtr[1]);

	for (AptPavementVector::iterator p = a->pavements.begin(); p != a->pavements.end(); ++p)
		CalcPavementOGL(a, p->ends,
							p->width_ft * FT_TO_MTR,
							p->blast1_ft * FT_TO_MTR,
							p->blast2_ft * FT_TO_MTR,
							p->disp1_ft * FT_TO_MTR,
							p->disp2_ft * FT_TO_MTR);

	for(AptSealaneVector::iterator s = a->sealanes.begin(); s != a->sealanes.end(); ++s)
		CalcPavementOGL(a, s->ends,
								s->width_mtr,0,0,0,0);

	for(AptHelipadVector::iterator h = a->helipads.begin(); h != a->helipads.end(); ++h)
			CalcPavementHelipad(a,h->location,
					h->heading,
					h->width_mtr,
					h->length_mtr);

}
#endif

