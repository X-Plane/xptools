//
//  WED_OSMSlippyMap.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/18/15.
//
//

#include "WED_OSMSlippyMap.h"
#include "XDefs.h"

#include <sstream>

#include "WED_MapZoomerNew.h"
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

#define MIN_ZOOM 0

// This table of zoom levels comes from...
// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#Resolution_and_Scale
static const double k_mpp[OSM_ZOOM_LEVELS] = {
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
	4.7773,
	2.3887,
	1.1943,
	0.5972 };

// These tile conversion formlas come from...
// http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#C.2FC.2B.2B

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






static int get_osm_zoom_for_map_ppm(double in_ppm)
{
	double mpp = 1.0 / in_ppm;
	int osm = 0;
	while(osm < OSM_ZOOM_LEVELS && k_mpp[osm] > mpp)
		++osm;
	
	if(osm > 0) --osm;
	return osm;
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
	m_cache_request(NULL)
{
}

WED_OSMSlippyMap::~WED_OSMSlippyMap()
{
}
	
void	WED_OSMSlippyMap::DrawVisualization(bool inCurrent, GUI_GraphState * g)
{
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
	
	int z_max = get_osm_zoom_for_map_ppm(ppm);
	
	int want = 0, got = 0, bad = 0;
	
	for(int z = MIN_ZOOM; z <= z_max; ++z)
	{
		int tiles[4];
		
		get_tile_range_for_box(map_bounds,z,tiles);
		
		for(int y = tiles[3]; y <= tiles[1]; ++y)
		for(int x = tiles[0]; x <= tiles[2]; ++x)
		{
			++want;
			double tbounds[4];
			double pbounds[4];
			get_ll_box_for_tile(z, x, y, tbounds);
			
			pbounds[0] = zoomer->LonToXPixel(tbounds[0]);
			pbounds[2] = zoomer->LonToXPixel(tbounds[2]);

			pbounds[1] = zoomer->LatToYPixel(tbounds[1]);
			pbounds[3] = zoomer->LatToYPixel(tbounds[3]);
			
			/*
			g->SetState(0, 0, 0, 0, 0, 0, 0);

			float c[4] = { 1, 1, 1, 1 };
			glColor4fv(c);
			
			glBegin(GL_LINE_LOOP);
				glVertex2f(pbounds[0]+1,pbounds[1]+1);
				glVertex2f(pbounds[0]+1,pbounds[3]-1);
				glVertex2f(pbounds[2]-1,pbounds[3]-1);
				glVertex2f(pbounds[2]-1,pbounds[1]+1);
			glEnd();
			
			char msg[256];
			sprintf(msg,"%d/%d/%d", z, x, y);
			GUI_FontDraw(g, font_UI_Basic, c, (pbounds[0]+pbounds[2]) / 2, (pbounds[1] + pbounds[3])/2, msg);
			*/
			
			//cout << 
			//for (map<string,int>::iterator itr = m_cache.begin(); itr != m_cache.end(); ++itr)
			{
				//cout << "Path: " << itr->first << " Enabled: " << std::boolalpha << itr->second << "\n---\n";
			}
			
			char path_buf[256];
			sprintf(path_buf, "http://a.tile.openstreetmap.org/%d/%d/%d.png", z, x, y);
			string url(path_buf);
			//cout << url << endl;
			stringstream folder_prefix;
			folder_prefix << "OSMSlippyMap" << DIR_STR << z << DIR_STR << x;// << DIR_STR << y;
			string potential_path = WED_file_cache_url_to_cache_path(WED_file_cache_request("", cache_domain_osm_tile, folder_prefix.str() , url));

			if (m_cache.count(potential_path))
			{
				++got;

				int id = m_cache[potential_path];
				if(id != 0)
				{
					//cout << "Drawing URL: " << url << " path: " << potential_path.substr(28) << " id: " << id << endl;
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

					float clr[4] = { 0,0,0,1 };

					//stringstream ss;
					//ss << potential_path.substr(28) << " Id: " << id;
					//if (x % 3 == 0 && y % 3 == 0)
					{
						//GUI_FontDraw(g, font_UI_Basic, clr, pbounds[0] + 10, pbounds[1] + 10, ss.str().c_str());
					}
				}
				else
				{
					++bad;
				}
			}
			else if(m_cache_request == NULL)
			{
				m_cache_request = new WED_file_cache_request("",cache_domain_osm_tile, folder_prefix.str(),url);
				//cout << "Begin " << *m_cache_request << endl;
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

	char buf[1024];
	sprintf(buf, "Zoom level %d: %d of %d (%f%% done, %d errors). %d tiles cached (%d MB)",
		z_max,
		got,
		want,
		(float)got * 100.0f / (float)want,
		bad,
		(int)m_cache.size(),
		(int)m_cache.size() / 4);

	float clr[4] = { 1, 1, 1, 1 };
	int bnds[4];
	GetHost()->GetBounds(bnds);
	
	GUI_FontDraw(g, font_UI_Basic, clr, bnds[0] + 10, bnds[1] + 10, buf);

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
			if (CreateBitmapFromPNG(res.out_path.c_str(), &info, false, 0) == 0)
			{
				GLuint tex_id;
				glGenTextures(1, &tex_id);
				if (LoadTextureFromImage(info, tex_id, tex_Linear | tex_Mipmap, NULL, NULL, NULL, NULL))
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
				printf("Bad JPEG data.\n");
				m_cache[res.out_path] = 0;
			}

			//cout << "Finished " << *m_cache_request << " path: " << res.out_path << endl;

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
