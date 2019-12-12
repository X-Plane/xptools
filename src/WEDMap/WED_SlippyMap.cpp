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

#include "WED_SlippyMap.h"

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

#define SHOW_DEBUG_INFO 0

#define MIN_ZOOM  13        // stop displaying OSM at all below this level
#define MAX_ZOOM  17        // for custom mode maps (predefined maps have their own limits below)

#define TILE_FACTOR 0.8     // save tiles by zooming in a bit later than at 1:1 pixel ratio.
							// Since zoom goes by 1.2x steps - it matters little w.r.t "sharpness"
							// but saves on average 34% of all tile loads

#define PREDEFINED_MAPS 2

static const char * attributions[PREDEFINED_MAPS] = {
"© OpenStreetMap Contributors",
// ToDo: use shorter specific ESRI attribution by downloading https://static.arcgis.com/attribution/World_Imagery
//       and decode it per https://github.com/Esri/esri-leaflet  (which is java code)
"© Esri, DigitalGlobe, GeoEye, Earthstar Geographics, CNES/Airbus DS, USDA, USGS, AeroGRID and the GIS User Community" };

static const char * tile_url[PREDEFINED_MAPS] = {
WED_URL_OSM_TILES  "${z}/${x}/${y}.png",
WED_URL_ESRI_TILES "${z}/${y}/${x}.jpg" };

static const int max_zoom[PREDEFINED_MAPS] = {
16,        // OSM tiles below this zoom are not cached, but on-demand generated. Openstreetmap foundation asks to limit their use.
17 };      // ESRI maps are available down to this level in general


static inline int long2tilex(double lon, int z)
{
	return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z)));
}

static inline int lat2tiley(double lat, int z)
{
	return (int)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z)));
}

static inline double tilex2long(int x, int z)
{
	return x / pow(2.0, z) * 360.0 - 180;
}

static inline double tiley2lat(int y, int z)
{
	double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

int WED_SlippyMap::get_zl_for_map(double in_ppm, double lattitude)
{
	double mpp = 1.0 / in_ppm;
	double zl_mpp = 156543.03 * TILE_FACTOR / 1.4 * cos(lattitude * 3.14/180.0);
	int zl = 0;
	int max_zl = mMapMode <= PREDEFINED_MAPS ? max_zoom[mMapMode-1] : MAX_ZOOM;

	while(zl < max_zl && zl_mpp > mpp)
	{
		zl_mpp *= 0.5;
		++zl;
	}
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


WED_SlippyMap::WED_SlippyMap(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver)
	: WED_MapLayer(h, zoomer, resolver),
	m_cache_request(NULL),
	mMapMode(0)
{
}

WED_SlippyMap::~WED_SlippyMap()
{
	delete m_cache_request;
	m_cache_request = NULL;
}

void	WED_SlippyMap::DrawVisualization(bool inCurrent, GUI_GraphState * g)
{
	if (mMapMode ==0) return;
	finish_loading_tile();

	double map_bounds[4];

	WED_MapZoomerNew * zoomer = GetZoomer();
	zoomer->GetMapVisibleBounds(map_bounds[0], map_bounds[1], map_bounds[2], map_bounds[3]);
	map_bounds[0] = doblim(map_bounds[0],-180.0,180.0);
	map_bounds[2] = doblim(map_bounds[2],-180.0,180.0);
	map_bounds[1] = doblim(map_bounds[1],-85.0,85.0);
	map_bounds[3] = doblim(map_bounds[3],-85.0,85.0);

	double ppm = zoomer->GetPPM();
	int z_max = get_zl_for_map(ppm, map_bounds[1]);
	int min_zoom = flt_abs(map_bounds[1]) > 60.0 ? MIN_ZOOM-1 : MIN_ZOOM; // get those ant/artic designers a bit more visibility
	if(z_max < min_zoom) return;

	int want = 0, got = 0, bad = 0;
	for(int z = max(min_zoom,z_max-1); z <= z_max; ++z)      // Display only the next lower zoom level
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

#if DEV && SHOW_DEBUG_INFO
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
			int yTransformed;
			switch(y_coordinate_math)
			{
				case yYahoo: yTransformed = (1 << (z-1)) - 1 - y; break;
				case yOSGeo: yTransformed = (1 << z) - 1 - y; break;
				default: yTransformed = y;
			}
#if IBM
			char url[200]; _sprintf_p(url, 200, url_printf_fmt.c_str(), x, yTransformed, z);
			char dir[200]; _sprintf_p(dir, 200, dir_printf_fmt.c_str(), x, yTransformed, z);
#else
			char url[200]; snprintf(url, 200, url_printf_fmt.c_str(), x, yTransformed, z);
			char dir[200]; snprintf(dir, 200, dir_printf_fmt.c_str(), x, yTransformed, z);  // make sure ALL args are referenced in the format string
#endif
			string folder_prefix(dir); folder_prefix.erase(folder_prefix.find_last_of(DIR_STR));

			//The potential place the tile could appear on disk, were it to be downloaded or have been downloaded
			string potential_path = gFileCache.url_to_cache_path(WED_file_cache_request("", cache_domain_osm_tile, folder_prefix , url));

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
						glTexCoord2f(1,0);
						glVertex2f(pbounds[2],pbounds[1]);

						glTexCoord2f(1,1);
						glVertex2f(pbounds[2],pbounds[3]);
						glTexCoord2f(0,1);
						glVertex2f(pbounds[0],pbounds[3]);

						glTexCoord2f(0,0);
						glVertex2f(pbounds[0],pbounds[1]);
					glEnd();

#if DEV && SHOW_DEBUG_INFO
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

void	WED_SlippyMap::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = draw_ent_s = cares_about_sel = wants_clicks = 0;
}

void	WED_SlippyMap::finish_loading_tile()
{
	if (m_cache_request != NULL)
	{
		WED_file_cache_response res = gFileCache.request_file(*m_cache_request);
		if (res.out_status == cache_status_available)
		{
			struct ImageInfo info;
			int r = CreateBitmapFromPNG(res.out_path.c_str(), &info, false, 0.0, false);
			if(r != 0)
				r = CreateBitmapFromJPEG(res.out_path.c_str(), &info, false);
			if(r == 0)
			{
				if (info.channels == 3)                                                        // apply to color changes
					for (int x = 0; x < info.height * (info.width+info.pad) * info.channels; x += info.channels)
						{
							double BRIGHTNESS = -20;
							double SATURATION = 1.0;
							if(mMapMode == 1) { BRIGHTNESS = -140.0; SATURATION = 0.4; }

							int val = 0.3 * info.data[x] + 0.6 * info.data[x+1] + 0.1 * info.data[x+2];  // deliberately not HSV weighing - want red's brighter, its also bgr data
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
				printf("Can not read image tile - bad PNG or JPG data.\n");
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

void	WED_SlippyMap::TimerFired()
{
	GetHost()->Refresh();
}

static bool replace_token(string& str, const string& from, const string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == string::npos)
        return false;
    str = str.substr(0,start_pos) + to + str.substr(start_pos+from.length());
    return true;
}

void	WED_SlippyMap::SetMode(int mode)
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
		
	y_coordinate_math = yNone;
	if     (replace_token(url_printf_fmt, "${y}",  "%2$d")) 	y_coordinate_math = yNormal;
	else if(replace_token(url_printf_fmt, "${!y}", "%2$d")) 	y_coordinate_math = yYahoo;
	else if(replace_token(url_printf_fmt, "${-y}", "%2$d")) 	y_coordinate_math = yOSGeo;


	if(replace_token(url_printf_fmt, "${x}", "%1$d") &&
	   y_coordinate_math != yNone &&
	   replace_token(url_printf_fmt, "${z}", "%3$d"))
	{
		dir_printf_fmt = url_printf_fmt.substr(url_printf_fmt.find("//")+2);
		replace(dir_printf_fmt.begin(), dir_printf_fmt.end(), '/', DIR_CHAR);

		mMapMode = mode;
		SetVisible(1);
	}
	else
	{
		mMapMode = 0;
		SetVisible(0);
		printf("Illegal URL string for SlippyMap\n");
	}
}

int		WED_SlippyMap::GetMode(void)
{
	return mMapMode;
}
