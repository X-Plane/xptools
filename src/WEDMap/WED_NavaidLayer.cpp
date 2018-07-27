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

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

WED_NavaidLayer::WED_NavaidLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(host,zoomer,resolver)
{

// ToDo: move this into PackageMgr, so its updated when XPlaneFolder changes and re-used when another scenery is opened

	string resourcePath;
	gPackageMgr->GetXPlaneFolder(resourcePath);
	
	// deliberately ignoring any Custom Data/earth_424.dat or Custom Data/earth_nav.dat files that a user may have ... to avoid confusion
	string globalNavaids  = resourcePath + DIR_STR "Resources" DIR_STR "default data" DIR_STR "earth_nav.dat";
	string airportNavaids = resourcePath + DIR_STR "Custom Scenery" DIR_STR "Global Airports" DIR_STR "Earth nav data" DIR_STR "earth_nav.dat";
	
	MFMemFile * str = MemFile_Open(globalNavaids.c_str());
	if(str)
	{
		MFScanner	s;
		MFS_init(&s, str);
		
		int versions[] = { 740, 810, 1100, 0 };
		
		mNavaids.reserve(25000);    // about 2.5 MBytes, as of 2018 its some 20,200 navaids
		
		if(MFS_xplane_header(&s,versions,NULL,NULL))
			while(!MFS_done(&s))
			{
				int type = MFS_int(&s);
				if (type >= 2 && type <= 9)   // NDB, VOR and ILS components
				{
					navaid_t n;
					n.type = type;
					n.lonlat.y_ = MFS_double(&s);
					n.lonlat.x_ = MFS_double(&s);
					MFS_int(&s);   // skip elevation
					n.freq  = MFS_int(&s);   // skip frequency
					MFS_int(&s);   // skip range
					n.heading   = MFS_double(&s);
					MFS_string(&s, &n.name);
					MFS_string(&s, &n.icao);
					if (type == 6)
					{
						double slope = floor(n.heading / 1000.0);
						n.heading -= slope * 1000.0;
					}	
					mNavaids.push_back(n);
				}
				MFS_string_eol(&s,NULL);
			}
		MemFile_Close(str);
	}

	str = MemFile_Open(airportNavaids.c_str());
	if(1 && str)
	{
		MFScanner	s;
		MFS_init(&s, str);
		
		int versions[] = { 740, 810, 1100, 0 };
		
		if(MFS_xplane_header(&s,versions,NULL,NULL))
			while(!MFS_done(&s))
			{
				int type = MFS_int(&s);
				if (type >= 4 && type <= 9)   // accept only ILS component overrides
				{
					navaid_t n;
					n.type = type;
					n.lonlat.y_ = MFS_double(&s);
					n.lonlat.x_ = MFS_double(&s);
					MFS_int(&s);   // skip elevation
					n.freq = MFS_int(&s);   // skip frequency
					MFS_int(&s);   // skip range
					n.heading   = MFS_double(&s);
					MFS_string(&s, &n.name);
					MFS_string(&s, &n.icao);
					if (type == 6)
					{
						double slope = floor(n.heading / 1000.0);
						n.heading -= slope * 1000.0;
					}

					// now check for duplicates before adding this one:
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
#if DEV
								printf("Replacing exact type %d, icao %s & name %s match. d=%5.1lfm\n", n.type, n.icao.c_str(), n.name.c_str(), d);
#endif
								*i = n;
								break;
							}
							if(d < closest_d)
							{
								closest_d = d;
								closest_i = i;
#if DEV
								printf("Name mismatch, keeping type %d, icao %s at d=%5.1lfm in mind, name=%s,%s\n", n.type, n.icao.c_str(), d, i->name.c_str(), n.name.c_str());
#endif
							}
						}
						++i;
					}
					
					if (i == mNavaids.end())
					{
						if (closest_d < 20.0)
						{
#if 1 // DEV
							printf("Replacing despite name %s,%s", closest_i->name.c_str(), n.name.c_str());
							if (closest_i->freq != n.freq) printf(" and frequency %d,%d", closest_i->freq, n.freq );
							printf(" mismatch, type %d, icao %s d=%5.1lfm\n", n.type, n.icao.c_str(), closest_d);
#endif
							*closest_i = n;
						}
						else
						{
#if DEV
							printf("Adding new %d %s %s\n", n.type, n.name.c_str(), n.icao.c_str());
#endif
							mNavaids.push_back(n);
						}
					}
				}
				MFS_string_eol(&s,NULL);
			}
		MemFile_Close(str);
	}
	
// Todo: speedup drawing by converting mNavaids into list sorted by longitude (aka map / multimap) - for quicker selection of visible navaids
//       improve data locality - store coords as 32 bit fixed point (9mm resolution is plenty), heading, type as short = 12 bytes total (now 96)
//       although for now Navaid map drawing is under 1 msec on a 3.6 GHz CPU at all times == good enough
}

WED_NavaidLayer::~WED_NavaidLayer()
{
}

#define NAVAID_EXTRA_RANGE 0.03  // degree's lon/lat, allows ILS beams to show even if the ILS is outside of the map window

void		WED_NavaidLayer::DrawVisualization		(bool inCurrent, GUI_GraphState * g)
{
	double ll,lb,lr,lt;	// logical boundary
	double vl,vb,vr,vt;	// visible boundry

	GetZoomer()->GetMapLogicalBounds(ll,lb,lr,lt);
	GetZoomer()->GetMapVisibleBounds(vl,vb,vr,vt);

	vl = max(vl,ll) - NAVAID_EXTRA_RANGE;
	vb = max(vb,lb) - NAVAID_EXTRA_RANGE;
	vr = min(vr,lr) + NAVAID_EXTRA_RANGE;
	vt = min(vt,lt) + NAVAID_EXTRA_RANGE;

	double PPM = GetZoomer()->GetPPM();
	double scale = GetAirportIconScale();
	double beam_len = 3300.0/scale * PPM;

	float red[4] = { 1.0, 0.4, 0.4, 0.66 };
	glLineWidth(1.8);

	if (PPM > 0.001)          // stop displaying navaids when zoomed out - gets too crowded
		for(vector<navaid_t>::iterator i = mNavaids.begin(); i != mNavaids.end(); ++i)  // this is brain dead - use list sorted by longitude
		{
			if(i->lonlat.x() > vl && i->lonlat.x() < vr && 
			   i->lonlat.y() > vb && i->lonlat.y() < vt)
			{
				glColor4fv(red);
				Point2 pt = GetZoomer()->LLToPixel(i->lonlat);
				if(i->type == 2)
					GUI_PlotIcon(g,"nav_ndb.png", pt.x(), pt.y(), 0.0, scale);
				else if(i->type == 3)
				{
					GUI_PlotIcon(g,"nav_vor.png", pt.x(), pt.y(), i->heading, scale);
				}
				else if(i->type <= 5)
				{
//					GUI_PlotIcon(g,"map_airport.png", pt.x(), pt.y(), i->heading, scale);
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
				else
					GUI_PlotIcon(g,"nav_mark.png", pt.x(), pt.y(), i->heading, scale);

				if (PPM  > 1.0)
				{
					GUI_FontDraw(g, font_UI_Basic, red, pt.x()+20.0,pt.y()-25.0, i->name.c_str());
					GUI_FontDraw(g, font_UI_Basic, red, pt.x()+20.0,pt.y()-40.0, i->icao.c_str());
				}
			}
		}
}

void		WED_NavaidLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = draw_ent_s = cares_about_sel = wants_clicks = 0;
}
