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

#include "WED_NavaidLayer.h"
#include "GUI_Pane.h"

#include "GUI_DrawUtils.h"
#include "WED_DrawUtils.h"
#include "GUI_Fonts.h"
#include "GUI_GraphState.h"

#include "WED_Colors.h"
#include "WED_MapZoomerNew.h"
#include "WED_PackageMgr.h"
#include "MemFileUtils.h"
#include "PlatformUtils.h"
#include "GISUtils.h"

#if 0
#include <chrono>
auto t0 = std::chrono::high_resolution_clock::now();
auto t1 = std::chrono::high_resolution_clock::now();
auto t2 = chrono::high_resolution_clock::now();


chrono::duration<double> elapsed = t1-t0;
printf("0 to 1 time: %lf\n", elapsed.count());

elapsed = t2-t1;
printf("1 to 2 time: %lf\n", elapsed.count());
#endif

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#define SHOW_TOWERS 0
#define SHOW_APTS_FROM_APTDAT 1
#define COMPARE_GW_TO_APTDAT 0

#define NAVAID_EXTRA_RANGE  GLOBAL_WED_ART_ASSET_FUDGE_FACTOR  // degree's lon/lat, allows ILS beams to show even if the ILS is outside of the map window

#if COMPARE_GW_TO_APTDAT

#include <json/json.h>
#include "RAII_Classes.h"
#include "WED_FileCache.h"
#include "WED_Url.h"
#include "FileUtils.h"
#include "GUI_Resources.h"

static void get_airports(map<string, navaid_t>& tAirports)
{
	WED_file_cache_request	mCacheRequest;
	
	mCacheRequest.in_cert = WED_get_GW_url();
	mCacheRequest.in_domain = cache_domain_airports_json;
	mCacheRequest.in_folder_prefix = "scenery_packs" DIR_STR "GatewayImport";
	mCacheRequest.in_url = WED_get_GW_url() + "airports";

	WED_file_cache_response res = gFileCache.request_file(mCacheRequest);

	sleep(3);

	string json_string;
	
	if(res.out_status == cache_status_available)
	{
		//Attempt to open the file we just downloaded
		RAII_FileHandle file(res.out_path.c_str(),"r");
//		RAII_FileHandle file("/home/xplane/.cache/wed_file_cache/scenery_packs/GatewayImport/airports","r");

		if(FILE_read_file_to_string(file(), json_string) == 0)
		{
			file.close();
		}
		else
		  printf("cant read\n");
	}
	
	Json::Value root;
	Json::Reader reader;
	bool success = reader.parse(json_string, root);

	//Check for errors
	if(success == false)
	{
		printf("no airports\n");
		return;
	}

	Json::Value mAirportsGET = Json::Value(Json::arrayValue);
	mAirportsGET.swap(root["airports"]);

	for (int i = 0; i < mAirportsGET.size(); i++)
	{
		//Get the current scenery object
		Json::Value tmp(Json::objectValue);
		tmp = mAirportsGET.operator[](i);       //Yes, you need the verbose operator[] form. Yes it's dumb

		navaid_t n;
		
		n.icao =	tmp["AirportCode"].asString();
		n.name =	tmp["AirportName"].asString();
		n.heading = tmp["ApprovedSceneryCount"].asInt() > 0 ?  1 : 0;  // 3D or just 2D
		n.type = 10000 + 1; // + tmp["AirportClass"].asInt();    // rowcode 17= seaport, 1=airport, 16=heliport
		n.lonlat = Point2(tmp["Longitude"].asDouble(), tmp["Latitude"].asDouble());
		
		if(tmp["ApprovedSceneryCount"].asInt() > 0)
			n.rwy = "(GW)";
		else
			n.rwy = "(WEDbot)";
			
		tAirports[n.icao]= n;
	}
}
#endif

static void parse_apt_dat(MFMemFile * str, map<string, navaid_t>& tAirports, const string& source)
{
	MFScanner	s;
	MFS_init(&s, str);
	int versions[] = { 1000, 1021, 1050, 1100, 1130, 0 };
		
	if(MFS_xplane_header(&s,versions,NULL,NULL))
	{
		int apt_type = 0;
		Bbox2 apt_bounds;
		navaid_t n;

		while(!MFS_done(&s))
		{
			int rowcode = MFS_int(&s);
			if (rowcode == 1 || rowcode == 16 || rowcode == 17 || rowcode == 99)   // look only for accept only ILS component overrides
			{
				if(apt_type)
				{
					n.lonlat = apt_bounds.centroid();
					tAirports[n.icao]= n;
				}
				apt_type = rowcode;
				apt_bounds = Bbox2();
				n.type = 10000 + rowcode;
				MFS_int(&s);			 // skip elevation
				n.heading = MFS_int(&s); // has ATC tower
				MFS_int(&s);

				MFS_string(&s,&n.icao);
				MFS_string_eol(&s,&n.name);
				n.rwy = source;
			}
			else if(apt_type)
			{
				if((rowcode >=  111 && rowcode <=  116) ||
					rowcode == 1201 || rowcode == 1300  ||
					(rowcode >=   18 && rowcode <=   21))
				{
					double lat = MFS_double(&s);
					double lon = MFS_double(&s);
					apt_bounds += Point2(lon,lat);
				}
				else if (rowcode == 100) // runways
				{
					MFS_double(&s);  // width
					MFS_double(&s); MFS_double(&s); MFS_double(&s);
					MFS_double(&s); MFS_double(&s); MFS_double(&s);
					MFS_string(&s, NULL);
					double lat = MFS_double(&s);
					double lon = MFS_double(&s);
					MFS_double(&s); MFS_double(&s); MFS_double(&s);
					MFS_double(&s); MFS_double(&s); MFS_double(&s);
					apt_bounds += Point2(lon,lat);
					MFS_string(&s, NULL);
					lat = MFS_double(&s);
					lon = MFS_double(&s);
					apt_bounds += Point2(lon,lat);
				}
				else if (rowcode == 101) // sealanes
				{
					MFS_double(&s);
					MFS_double(&s);
					MFS_string(&s, NULL);
					double lat = MFS_double(&s);
					double lon = MFS_double(&s);
					apt_bounds += Point2(lon,lat);
					MFS_string(&s, NULL);
					lat = MFS_double(&s);
					lon = MFS_double(&s);
					apt_bounds += Point2(lon,lat);
				}
				else if (rowcode == 102) // helipads
				{
					MFS_string(&s, NULL);
					double lat = MFS_double(&s);
					double lon = MFS_double(&s);
					apt_bounds += Point2(lon,lat);
				}
			}
			MFS_string_eol(&s,NULL);
		}
	MemFile_Close(str);
	}
}

static void parse_nav_dat(MFMemFile * str, vector<navaid_t>& mNavaids, bool merge)
{
	MFScanner	s;
	MFS_init(&s, str);
	int versions[] = { 1000, 1021, 1050, 1100, 1130, 0 };
		
	if(MFS_xplane_header(&s,versions,NULL,NULL))
	{
		while(!MFS_done(&s))
		{
			int type = MFS_int(&s);
			int first_type = merge ? 4 : 2;         // accept only ILS component overrides when merging
			if (type >= first_type && type <= 9)    // NDB, VOR and ILS components
			{
				navaid_t n;
				n.type = type;
				double lat = MFS_double(&s);
				double lon = MFS_double(&s);
				n.lonlat = Point2(lon,lat);
				MFS_int(&s);   // skip elevation
				n.freq  = MFS_int(&s);
				MFS_int(&s);   // skip range
				n.heading   = MFS_double(&s);
				MFS_string(&s, &n.name);
				MFS_string(&s, &n.icao);
				if (type >= 4 && type <= 9)   // ILS components
				{
					MFS_string(&s, NULL);  // skip region
					MFS_string(&s, &n.rwy);
				}
				if (type == 6)
				{
					n.heading -= floor(n.heading / 1000.0) * 1000.0; // zero out the lowest 3 digits
				}
				if(merge)
				{
					// check for duplicates before adding this new one
					float closest_d = 9999.0;
					vector<navaid_t>::iterator closest_i;
					vector<navaid_t>::iterator i = mNavaids.begin();
					while(i != mNavaids.end())
					{
						if(n.type == i->type && n.icao == i->icao)
						{
							float d = LonLatDistMeters(n.lonlat, i->lonlat);
							if (n.name == i->name)
							{
//								printf("Replacing exact type %d, icao %s & name %s match. d=%5.1lfm\n", n.type, n.icao.c_str(), n.name.c_str(), d);
								*i = n;
								break;
							}
							if(d < closest_d)
							{
								closest_d = d;
								closest_i = i;
//								printf("Name mismatch, keeping type %d, icao %s at d=%5.1lfm in mind, name=%s,%s\n", n.type, n.icao.c_str(), d, i->name.c_str(), n.name.c_str());
							}
						}
						++i;
					}
					if (i == mNavaids.end())
					{
						if (closest_d < 20.0)
						{
//							printf("Replacing despite name %s,%s", closest_i->name.c_str(), n.name.c_str());
//							if (closest_i->freq != n.freq) printf(" and frequency %d,%d", closest_i->freq, n.freq );
//							printf(" mismatch, type %d, icao %s d=%5.1lfm\n", n.type, n.icao.c_str(), closest_d);
							*closest_i = n;
						}
						else
						{
//							printf("Adding new %d %s %s\n", n.type, n.name.c_str(), n.icao.c_str());
							mNavaids.push_back(n);
						}
					}
				}
				else // not mergin, take them all
					mNavaids.push_back(n);
			}
			MFS_string_eol(&s,NULL);
		}
	}
	MemFile_Close(str);
}

static void parse_atc_dat(MFMemFile * str, vector<navaid_t>& mNavaids)
{
	MFScanner	s;
	MFS_init(&s, str);
	int versions[] = { 1000, 1130, 0 };
		
	if(MFS_xplane_header(&s,versions,"ATCFILE",NULL))
	{
		navaid_t n;
		int num_rings;
		while(!MFS_done(&s))
		{
			if(MFS_string_match(&s, "CONTROLLER", 1))
			{
				n.type = 0;
				n.shape.clear();
				n.lonlat = Point2(180.0,0.0);
				n.rwy.clear();
				num_rings = 0;
			}
			if(MFS_string_match(&s, "ROLE", 0))
			{
				string role;
				MFS_string(&s, &role);
				n.type = role == "tracon" ? 9999 : 0; // navaid pseudo code for TRACON areas
#if SHOW_TOWERS
				if(role == "twr") n.type = 9998;
#endif
			}
			else if(MFS_string_match(&s, "NAME", 0))
			{
				MFS_string_eol(&s, &n.name);
			}
			else if(MFS_string_match(&s, "FACILITY_ID", 0))
			{
				MFS_string(&s, &n.icao);
			}
			else if(MFS_string_match(&s, "FREQ", 0))
			{
				string tmp;
				MFS_string(&s, &tmp);
				if(!n.rwy.empty()) n.rwy += ", ";
				n.rwy += tmp.substr(0,tmp.size()-2) + "." + tmp.substr(tmp.size()-2);
			}
			else if(MFS_string_match(&s, "CHAN", 0))
			{
				string tmp;
				MFS_string(&s, &tmp);
				if(!n.rwy.empty()) n.rwy += ", ";
				n.rwy += tmp.substr(0,tmp.size()-3) + "." + tmp.substr(tmp.size()-3);
			}
			else if(n.type)
			{
				if(MFS_string_match(&s, "POINT", 0))
				{
					double lat = MFS_double(&s);
					double lon = MFS_double(&s);
					n.shape.push_back(Point2(lon,lat));
					if( lon < n.lonlat.x())
						n.lonlat = Point2(lon,lat);  // get the left side of the area
				}
				else if(MFS_string_match(&s, "AIRSPACE_POLYGON_BEGIN", 1))
				{
					num_rings++;
				}
				else if(MFS_string_match(&s, "AIRSPACE_POLYGON_END", 1))
				{
//					printf("Adding new %d %s %s %d\n", n.type, n.name.c_str(), n.icao.c_str(), (int) n.shape.size());
					if (num_rings == 1)
					{
#if SHOW_TOWERS
						if (n.type == 9998)
							n.name += " TOWER";
						else
#endif					
						n.name += " APPROACH";
						n.rwy += " MHz";
					}
					mNavaids.push_back(n);
				}
			}
			MFS_string_eol(&s,NULL);
		}
	}
	MemFile_Close(str);
}


WED_NavaidLayer::WED_NavaidLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(host,zoomer,resolver)
{
    SetVisible(false);
	// ToDo: when using the gateway JSON data, initiate asynchronous load/update here.
}

WED_NavaidLayer::~WED_NavaidLayer()
{
}

void WED_NavaidLayer::LoadNavaids()
{
// ToDo: move this into PackageMgr, so its updated when XPlaneFolder changes and re-used when another scenery is opened
	string resourcePath;
	gPackageMgr->GetXPlaneFolder(resourcePath);

	mNavaids.reserve(25000);    // about 3 MBytes, as of 2018 its some 20,200 navaids

	// deliberately ignoring any Custom Data/earth_424.dat or Custom Data/earth_nav.dat files that a user may have ... to avoid confusion
	string defaultNavaids  = resourcePath + DIR_STR "Resources" DIR_STR "default data" DIR_STR "earth_nav.dat";
	string globalNavaids = resourcePath + DIR_STR "Custom Scenery" DIR_STR "Global Airports" DIR_STR "Earth nav data" DIR_STR "earth_nav.dat";
	
	MFMemFile * str = MemFile_Open(defaultNavaids.c_str());
	if(str) parse_nav_dat(str, mNavaids, false);
	str = MemFile_Open(globalNavaids.c_str());
	if(str)	parse_nav_dat(str, mNavaids, true);
	
	string defaultATC = resourcePath + DIR_STR "Resources" DIR_STR "default scenery" DIR_STR "default atc dat" DIR_STR "Earth nav data" DIR_STR "atc.dat";
	string seattleATC  = resourcePath + DIR_STR "Custom Scenery" DIR_STR "KSEA Demo Area" DIR_STR "Earth nav data" DIR_STR "atc.dat";

	str = MemFile_Open(defaultATC.c_str());

	// on the linux and OSX platforms this path was different before XP11.30 for some unknown reasons. So try that.
	if(!str)
	{
		defaultATC = resourcePath + DIR_STR "Resources" DIR_STR "default scenery" DIR_STR "default atc" DIR_STR "Earth nav data" DIR_STR "atc.dat";
		str = MemFile_Open(defaultATC.c_str());
	}
	if(str)	parse_atc_dat(str, mNavaids);
	str = MemFile_Open(seattleATC.c_str());
	if(str)	parse_atc_dat(str, mNavaids);
	
#if SHOW_APTS_FROM_APTDAT
	map<string,navaid_t> tAirports;
	string defaultApts = resourcePath + DIR_STR "Resources" DIR_STR "default scenery" DIR_STR "default apt dat" DIR_STR "Earth nav data" DIR_STR "apt.dat";
	string globalApts  = resourcePath + DIR_STR "Custom Scenery" DIR_STR "Global Airports" DIR_STR "Earth nav data" DIR_STR "apt.dat";

	str = MemFile_Open(defaultApts.c_str());
	if(str) parse_apt_dat(str, tAirports, "");
	str = MemFile_Open(globalApts.c_str());
	if(str) parse_apt_dat(str, tAirports, " (GW)");

#if COMPARE_GW_TO_APTDAT
	map<string,navaid_t> tAirp;
	get_airports(tAirp);
	
	for(auto a : tAirp)
	{
		auto b = tAirports.find(a.first);
		if (b != tAirports.end())
		{
			double dist = LonLatDistMeters(a.second.lonlat, b->second.lonlat);
			if (dist < 150000)
				printf("  matched %7s ll=%8.3lf %7.3lf d=%5.1lf km %s\n", a.first.c_str(), a.second.lonlat.x(), a.second.lonlat.y(), dist/1000.0, dist < 1000.0 ? "Good !" : "");
			else
				printf("  matched %7s ll=%8.3lf %7.3lf d=%5.0lf km Wow ! apt.dat ll=%8.3lf %7.3lf\n", a.first.c_str(), a.second.lonlat.x(), a.second.lonlat.y(), dist/1000.0, b->second.lonlat.x(), b->second.lonlat.y());
			if(dist > 1000.0)
				printf("UPDATE airports SET Latitude=%.3lf, Longitude=%.3lf WHERE AirportCode=\"%s\";\n", b->second.lonlat.y(), b->second.lonlat.x(), a.first.c_str());
		}
		else
			printf("unmatched %7s ll=%8.3lf %7.3lf\n", a.first.c_str(), a.second.lonlat.x(), a.second.lonlat.y());
	}
#endif

	for(map<string, navaid_t>::iterator i = tAirports.begin(); i != tAirports.end(); ++i)
		mNavaids.push_back(i->second);
#endif

// Todo: speedup drawing by sorting mNavaids into longitude buckets, so the preview function only have to go through a smalller part of the overall list.
//       although for now Navaid map drawing is under 1 msec on a 3.6 GHz CPU at all times == good enough
}

void		WED_NavaidLayer::DrawVisualization		(bool inCurrent, GUI_GraphState * g)
{
	double ll,lb,lr,lt;	// logical boundary
	double vl,vb,vr,vt;	// visible boundry

	if(mNavaids.empty()) LoadNavaids();

	GetZoomer()->GetMapLogicalBounds(ll,lb,lr,lt);
	GetZoomer()->GetMapVisibleBounds(vl,vb,vr,vt);

	vl = max(vl,ll) - NAVAID_EXTRA_RANGE;
	vb = max(vb,lb) - NAVAID_EXTRA_RANGE;
	vr = min(vr,lr) + NAVAID_EXTRA_RANGE;
	vt = min(vt,lt) + NAVAID_EXTRA_RANGE;

	double PPM = GetZoomer()->GetPPM();
	double scale = GetAirportIconScale();
	double beam_len = 3300.0/scale * PPM;

	const float red[4]        = { 1.0, 0.4, 0.4, 0.66 };
	const float vfr_purple[4] = { 0.9, 0.4, 0.9, 0.8 };
	const float vfr_blue[4]   = { 0.4, 0.4, 1.0, 0.8 };

	g->SetState(false,0,false,false,true,false,false);
	glLineWidth(1.6);
	glLineStipple(1, 0xF0F0);
	glDisable(GL_LINE_STIPPLE);
	
	if (PPM > 0.0005)          // stop displaying navaids when zoomed out - gets too crowded
		for(vector<navaid_t>::iterator i = mNavaids.begin(); i != mNavaids.end(); ++i)  // this is brain dead - use list sorted by longitude
		{
			if(i->lonlat.x() > vl && i->lonlat.x() < vr &&
			   i->lonlat.y() > vb && i->lonlat.y() < vt)
			{
				glColor4fv(red);
				Point2 pt = GetZoomer()->LLToPixel(i->lonlat);
				
				// draw icons
				if(i->type == 2)
					GUI_PlotIcon(g,"nav_ndb.png", pt.x(), pt.y(), 0.0, scale);
				else if(i->type == 3)
				{
					GUI_PlotIcon(g,"nav_vor.png", pt.x(), pt.y(), i->heading, scale);
				}
				else if(i->type <= 5)
				{
					Vector2 beam_dir(0.0, beam_len);
					beam_dir.rotate_by_degrees(180.0-i->heading);
					Vector2 beam_perp(beam_dir.perpendicular_cw()*0.1);

					g->SetState(0, 0, 0, 0, 1, 0, 0);
					glBegin(GL_LINE_STRIP);
						glVertex2(pt);
						glVertex2(pt + beam_dir*1.1 + beam_perp);
						glVertex2(pt + beam_dir);
						glVertex2(pt + beam_dir*1.1 - beam_perp);
						glVertex2(pt);
						glVertex2(pt + beam_dir);
					glEnd();
/*					glColor4f(1.0, 0.0, 0.0, 0.3);
					glBegin(GL_POLYGON);
						glVertex2(pt);
						glVertex2(pt + beam_dir*1.1 - beam_perp);
						glVertex2(pt + beam_dir);
					glEnd();
*/				}
				else if(i->type == 6)
				{
					if(PPM > 0.1)
						GUI_PlotIcon(g,"nav_gs.png", pt.x(), pt.y(), i->heading, scale);
				}
				else if(i->type < 100)
					GUI_PlotIcon(g,"nav_mark.png", pt.x(), pt.y(), i->heading, scale);
				else if(i->type <= 9999)
				{
					glColor4fv(vfr_blue);
#if SHOW_TOWERS
					if (i->type == 9998)
						glEnable(GL_LINE_STIPPLE);
#endif					
					int pts = i->shape.size();
					vector<Point2> c(pts);
					GetZoomer()->LLToPixelv(&(c[0]),&(i->shape[0]),pts);
					g->SetState(0, 0, 0, 0, 1, 0, 0);
					glShape2v(GL_LINE_LOOP, &(c[0]), pts);
#if SHOW_TOWERS
					glDisable(GL_LINE_STIPPLE);
#endif					
				}
				else
				{
					if(PPM > 0.002)
					{
						glColor4fv(i->heading ? vfr_blue : vfr_purple);
						if (i->type == 10017)
						{
							if(PPM > 0.02) GUI_PlotIcon(g,"map_helipad.png", pt.x(), pt.y(), 0.0, scale);
						}
						else if (i->type == 10016)
							GUI_PlotIcon(g,"navmap_seaport.png", pt.x(), pt.y(), 0.0, scale);
						else
							GUI_PlotIcon(g,"navmap_airport.png", pt.x(), pt.y(), 0.0, scale);
					}
				}
				// draw text labels, be carefull not to clutter things
#if SHOW_TOWERS
				if((i->type == 9998 && PPM  > 0.01) || i->type == 9999)
#else
				if(i->type == 9999)
#endif					
				{
					const float * color = vfr_blue;
					GUI_FontDraw(g, font_UI_Basic, color, pt.x()+8.0,pt.y()-15.0, i->name.c_str());
					GUI_FontDraw(g, font_UI_Basic, color, pt.x()+8.0,pt.y()-30.0, i->rwy.c_str());
				}
				else if (PPM  > 0.05)
				{
					if(i->type > 10000)
					{
						const float * color = i->heading ? vfr_blue : vfr_purple;
						GUI_FontDraw(g, font_UI_Basic, color, pt.x()+15.0,pt.y()-20.0, i->name.c_str());
						GUI_FontDraw(g, font_UI_Basic, color, pt.x()+15.0,pt.y()-35.0, (string("Airport ID") + i->rwy + ": " + i->icao).c_str());
					}
					else if(PPM > 0.5)
					{
						GUI_FontDraw(g, font_UI_Basic, red, pt.x()+20.0,pt.y()-25.0, i->name.c_str());
						GUI_FontDraw(g, font_UI_Basic, red, pt.x()+20.0,pt.y()-40.0, (i->icao + " " + i->rwy).c_str());
					}
				}
			}
		}

}

void		WED_NavaidLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = draw_ent_s = cares_about_sel = wants_clicks = 0;
}
