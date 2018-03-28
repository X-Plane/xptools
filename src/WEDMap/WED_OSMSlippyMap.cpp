/*
 * Copyright (c) 2015, Laminar Research.
 *
 * Created by Ben Supnik on 12/18/15.
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
 */

#include "WED_OSMSlippyMap.h"

#include <sstream>

#include "WED_MapZoomerNew.h"
#include "WED_Url.h"
#include "WED_Globals.h"
#include "MathUtils.h"
#include "BitmapUtils.h"
#include "PlatformUtils.h"
#include "TexUtils.h"
#include "GUI_GraphState.h"
#include "GUI_Fonts.h"
#include "curl_http.h"

#include "WED_FileCache.h"
#define _USE_MATH_DEFINES
#include <math.h>

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#if DEV
#include <iostream>
#endif

#define MIN_ZOOM  13        // stop displaying OSM at all below this level
#define MAX_ZOOM  16
#define TILE_FACTOR 0.8     // save tiles by zooming in a bit later than at 1:1 pixel ratio.
							// Since zoom goes by 1.2x steps - it matters little w.r.t "sharpness"
							// but saves on average 34% of all tile loads

#define ZOOM_LEVELS 19
							
// This table of zoom levels comes from...
// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Resolution_and_Scale
static const double k_mpp[ZOOM_LEVELS] = {
	156543.03,
	78271.52,
	39135.76,
	19567.88,
	9783.94,
	4891.97,
	2445.98,
	1222.99,
	611.50,
	305.75,
	152.87,
	76.437,
	38.219,
	19.109,
	9.5546,
	4.7773,      // ZL16, the highest level that is always cached data rather than real-time computed
	2.3887,
	1.1943,
	0.5972 };

// These tile conversion formulas come from...
// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#C.2FC.2B.2B

#define PREDEFINED_MAPS 2

static const char * attributions[PREDEFINED_MAPS] = {
"© OpenStreetMap Contributors",
// ToDo: Update regularly from http://services.arcgisonline.com/arcgis/rest/services/World_Imagery/MapServer?f=pjson
"© Esri, DigitalGlobe, GeoEye, Earthstar Geographics, CNES/Airbus DS, USDA, USGS, AeroGRID, IGN and the GIS User Community" };

static const char * tile_url[PREDEFINED_MAPS] = {
WED_URL_OSM_TILES "${z}/${x}/${y}.png",
"http://services.arcgisonline.com/arcgis/rest/services/World_Imagery/MapServer/tile/${z}/${y}/${x}.jpg" };

int long2tilex(double lon, int z) 
{ 
	return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z))); 
}
 
int lat2tiley(double lat, int z)
{ 
	return (int)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z))); 
}
 
double tilex2long(int x, int z) 
{
	return x / pow(2.0, z) * 360.0 - 180;
}
 
double tiley2lat(int y, int z) 
{
	double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

static int get_zl_for_map_ppm(double in_ppm)
{
	double mpp = 1.0 / in_ppm;
	int zl = 0;
	while(zl <= MAX_ZOOM && k_mpp[zl]*TILE_FACTOR > mpp)
		++zl;
	
	if(zl > 0) --zl;
	return zl;
}

static void get_ll_box_for_tile(int z, int x, int y, double bounds[4])
{
	bounds[0] = tilex2long(x,z  );
	bounds[2] = tilex2long(x+1,z);

	bounds[1] = tiley2lat (y  ,z);
	bounds[3] = tiley2lat (y+1,z);
}

// This returns an INCLUSIVE range.
static void get_tile_range_for_box(const double bounds[4], int z, int tiles[4])
{
	int max_tile = (1 << z) - 1;
	tiles[0] = intlim(long2tilex(bounds[0], z), 0, max_tile);
	tiles[2] = intlim(long2tilex(bounds[2], z), 0, max_tile);

	tiles[1] = intlim(lat2tiley(bounds[1], z), 0, max_tile);
	tiles[3] = intlim(lat2tiley(bounds[3], z), 0, max_tile);
}


WED_OSMSlippyMap::WED_OSMSlippyMap(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver)
	: WED_MapLayer(h, zoomer, resolver),
	m_cache_request(NULL),
	mMapMode(0)
{
}

WED_OSMSlippyMap::~WED_OSMSlippyMap()
{
	delete m_cache_request;
	m_cache_request = NULL;
}
	
void	WED_OSMSlippyMap::DrawVisualization(bool inCurrent, GUI_GraphState * g)
{
	if (mMapMode ==0) return;
	finish_loading_tile();

	double map_bounds[4];
	double	s, n, e, w;
	
	WED_MapZoomerNew * zoomer = GetZoomer();
	zoomer->GetMapVisibleBounds(map_bounds[0], map_bounds[1], map_bounds[2], map_bounds[3]);
	map_bounds[0] = doblim(map_bounds[0],-180.0,180.0);
	map_bounds[2] = doblim(map_bounds[2],-180.0,180.0);
	map_bounds[1] = doblim(map_bounds[1],-85.0,85.0);
	map_bounds[3] = doblim(map_bounds[3],-85.0,85.0);

	double ppm = zoomer->GetPPM();
	int z_max = get_zl_for_map_ppm(ppm);
	if(z_max < MIN_ZOOM) return;
	
	int want = 0, got = 0, bad = 0;
	for(int z = max(MIN_ZOOM,z_max-1); z <= z_max; ++z)      // Display only the next lower zoom level
	{                                                        // avoids having to load up to 4x14 extra tiles at ZL16
		int tiles[4];
		
		get_tile_range_for_box(map_bounds,z,tiles);
		
		for(int y = tiles[3]; y <= tiles[1]; ++y)
		for (int x = tiles[0]; x <= tiles[2]; ++x)
		{
			++want;
			double tbounds[4];
			double pbounds[4];
			get_ll_box_for_tile(z, x, y, tbounds);

			pbounds[0] = zoomer->LonToXPixel(tbounds[0]);
			pbounds[2] = zoomer->LonToXPixel(tbounds[2]);

			pbounds[1] = zoomer->LatToYPixel(tbounds[1]);
			pbounds[3] = zoomer->LatToYPixel(tbounds[3]);

#if DEV && 0
			//Draw border around tile
			g->SetState(0, 0, 0, 0, 0, 0, 0);
			GLfloat black[4] = { 0, 0, 0, 1 };
			glColor4fv(black);

			GLfloat prev_line_width = 0;
			glGetFloatv(GL_LINE_WIDTH, &prev_line_width);
			glLineWidth(3.0f);
			glBegin(GL_LINE_LOOP);
			glVertex2f(pbounds[0], pbounds[1]);
			glVertex2f(pbounds[0], pbounds[3]);
			glVertex2f(pbounds[2], pbounds[3]);
			glVertex2f(pbounds[2], pbounds[1]);
			glEnd();
			glLineWidth(prev_line_width);
			{
				char msg[100];
				snprintf(msg, 100, "%d/%d/%d", z, x, y);
				GUI_FontDraw(g, font_UI_Basic, black, (pbounds[0] + pbounds[2]) / 2, (pbounds[1] + pbounds[3]) / 2, msg);
			}
#endif
			char url[200]; snprintf(url,200,url_printf_fmt.c_str(),x,y,z);

			char dir[200]; snprintf(dir,200,dir_printf_fmt.c_str(),x,y,z);                      // make sure ALL args are referenced in the format string
			string folder_prefix(dir); folder_prefix.erase(folder_prefix.find_last_of(DIR_STR));

			//The potential place the tile could appear on disk, were it to be downloaded or have been downloaded
			string potential_path = WED_file_cache_url_to_cache_path(WED_file_cache_request("", cache_domain_osm_tile, folder_prefix , url));

			if (m_cache.count(potential_path))
			{
				++got;

				int id = m_cache[potential_path];
				if(id != 0)
				{
					g->SetState(0, 1, 0, 0, 0, 0, 0);
					glColor4f(1,1,1,1);
					g->BindTex(id, 0);
					glBegin(GL_QUADS);
						glTexCoord2f(1,1);
						glVertex2f(pbounds[2],pbounds[1]);

						glTexCoord2f(1,0);
						glVertex2f(pbounds[2],pbounds[3]);
						glTexCoord2f(0,0);
						glVertex2f(pbounds[0],pbounds[3]);

						glTexCoord2f(0,1);
						glVertex2f(pbounds[0],pbounds[1]);
					glEnd();

					#if DEV && 0
						stringstream ss;
						ss << potential_path.substr(28) << " Id: " << id;
						GUI_FontDraw(g, font_UI_Basic, black, pbounds[0] + 5, pbounds[1] - 15, ss.str().c_str()+20);
					#endif
				}
				else
				{
					++bad;
				}
			}
			else if(m_cache_request == NULL)
			{
				m_cache_request = new WED_file_cache_request("", cache_domain_osm_tile, folder_prefix, url);
			}
		}
	}

	if (m_cache_request)
	{
		this->Start(0.05);
	}
	else
	{
		this->Stop();
	}

	stringstream zoom_msg;
	zoom_msg << "ZL" << z_max << ": " 
			 << got << " of " << want
			 << " (" << (float)got * 100.0f / (float)want << "% done, " << bad << " errors). "
			 << (int)m_cache.size() << " tiles cached (" << (int)m_cache.size() / 4 << " MB)";
	
	int bnds[4];
	GetHost()->GetBounds(bnds);
	GLfloat white[4] = { 1, 1, 1, 1 };
	GUI_FontDraw(g, font_UI_Basic, white, bnds[0] + 10, bnds[1] + 40, zoom_msg.str().c_str());
	
	if(mMapMode <= PREDEFINED_MAPS)
	{
		int txtWidth = GUI_MeasureRange(font_UI_Small,attributions[mMapMode-1],attributions[mMapMode-1]+strlen(attributions[mMapMode-1]));
		
		g->SetState(0, 0, 0, 0, 1, 0, 0);
		glColor4f(0,0,0,0.65);
		glBegin(GL_QUADS);
			glVertex2f(bnds[2] - 10 - txtWidth, bnds[1] + 12 );
			glVertex2f(bnds[2],                 bnds[1] + 12 );
			glVertex2f(bnds[2],                 bnds[1]      );
			glVertex2f(bnds[2] - 10 - txtWidth, bnds[1]      );
		glEnd();
		GUI_FontDraw(g, font_UI_Small, white, bnds[2] - 5, bnds[1] + 2, attributions[mMapMode-1], align_Right);
	}
}

void	WED_OSMSlippyMap::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = draw_ent_s = cares_about_sel = wants_clicks = 0;
}

void	WED_OSMSlippyMap::finish_loading_tile()
{
	if (m_cache_request != NULL)
	{
		WED_file_cache_response res = WED_file_cache_request_file(*m_cache_request);
		if (res.out_status == cache_status_available)
		{
			struct ImageInfo info;
			int r;
			if(is_jpg_not_png)
				r = CreateBitmapFromJPEG(res.out_path.c_str(), &info);
			else
				r = CreateBitmapFromPNG(res.out_path.c_str(), &info, false, 0);
			if (r == 0)
			{
				if (info.channels == 3)                                                        // apply to color changes
					for (int x = 0; x < info.height * (info.width+info.pad) * info.channels; x += info.channels)
						{
							double BRIGHTNESS = -20; 
							double SATURATION = 1.0;
							if(mMapMode == 1) { BRIGHTNESS = -140.0; SATURATION = 0.4; }

							int val = 0.3 * info.data[x] + 0.6 * info.data[x+1] + 0.1 * info.data[x+2];  // deliberately not HSV weighing - want red's brighter
							for (int c = 0; c < info.channels; ++c)
								info.data[x+c] = intlim((1.0-SATURATION) * val + SATURATION * info.data[x+c] + BRIGHTNESS, 0, 255);
						}
			
				GLuint tex_id;
				glGenTextures(1, &tex_id);
				if (LoadTextureFromImage(info, tex_id, tex_Linear, NULL, NULL, NULL, NULL))
				{
					m_cache[res.out_path] = tex_id;
				}
				else
				{
					printf("Failed texture load from image.\n");
					m_cache[res.out_path] = 0;
				}
			}
			else
			{
				printf("Bad JPG or PNG data.\n");
				m_cache[res.out_path] = 0;
			}

			delete m_cache_request;
			m_cache_request = NULL;
		}
		else if (res.out_status == cache_status_error)
		{
			int code = res.out_error_type;

			printf("%s: %d\n%s\n", res.out_path.c_str(), code, res.out_error_human.c_str());

			m_cache[res.out_path] = 0;

			delete m_cache_request;
			m_cache_request = NULL;
		}
	}
}

void	WED_OSMSlippyMap::TimerFired()
{
	GetHost()->Refresh();
}

static bool replace_token(string& str, const string& from, const string& to) 
{
    size_t start_pos = str.find(from);
    if(start_pos == string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void	WED_OSMSlippyMap::SetMode(int mode)
{
	if(mode == 0)
	{
		mMapMode = 0;
		SetVisible(0);
		return;
	}
	
	if(mode <= PREDEFINED_MAPS)
		url_printf_fmt = tile_url[mode-1];
	else
		url_printf_fmt = gCustomSlippyMap;
		
	if(replace_token(url_printf_fmt, "${x}", "%1$d") &&
	   replace_token(url_printf_fmt, "${y}", "%2$d") &&
	   replace_token(url_printf_fmt, "${z}", "%3$d"))
	{	
		dir_printf_fmt = url_printf_fmt.substr(url_printf_fmt.find("//")+2);
		replace(dir_printf_fmt.begin(), dir_printf_fmt.end(), '/', DIR_CHAR);
		
		mMapMode = mode;
		SetVisible(1);
		
		string suffix = url_printf_fmt.substr(url_printf_fmt.length()-4);
		if (suffix == ".jpg" || suffix == ".JPG")
			is_jpg_not_png = true;
		else
			is_jpg_not_png = false;
	}
	else
	{
		mMapMode = 0;
		SetVisible(0);
		printf("Illegal URL string for SlippyMap\n");
	}
}

int		WED_OSMSlippyMap::GetMode(void)
{
	return mMapMode;
}
