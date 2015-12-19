//
//  WED_OSMSlippyMap.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/18/15.
//
//

#include "XDefs.h"
#include "WED_OSMSlippyMap.h"
#include "WED_MapZoomerNew.h"
#include "MathUtils.h"
#include "BitmapUtils.h"
#include "TexUtils.h"
#include "GUI_GraphState.h"
#include "GUI_Fonts.h"
#include "curl_http.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
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


WED_OSMSlippyMap::WED_OSMSlippyMap(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver) : WED_MapLayer(h, zoomer, resolver),
	m_req(NULL)
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
			
			char path_buf[256];
			sprintf(path_buf,"http://otile1.mqcdn.com/tiles/1.0.0/sat/%d/%d/%d.jpg", z, x, y);
			string path(path_buf);
			if(m_cache.count(path))
			{
				++got;
				int id = m_cache[path];
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
				} else
					++bad;
			}
			else if(m_req == NULL)
			{
				m_req_path = path;
				m_req = new curl_http_get_file(path, &m_buffer, string());
			}
		}
	}
	if(m_req)
		this->Start(0.05);
	else
		this->Stop();

	char buf[1024];
	sprintf(buf,"Zoom level %d: %d of %d (%f%% done, %d errors). %d tiles cached (%d MB)",
		z_max,
		got,
		want,
		(float) got * 100.0f / (float) want,
		bad,
		(int) m_cache.size(),
		(int) m_cache.size() / 4);
		
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
	if(m_req)
	{
		if(m_req->is_done())
		{
			if(m_req->is_ok())
			{
				struct ImageInfo info;
				if(CreateBitmapFromJPEGData(&m_buffer[0], m_buffer.size(), &info) == 0)
				{
					GLuint tex_id;
					glGenTextures(1, &tex_id);
					if(LoadTextureFromImage(info, tex_id, tex_Linear|tex_Mipmap, NULL, NULL, NULL, NULL))
					{
						m_cache[m_req_path] = tex_id;
					}
					else
					{
						printf("Failed texture load from image.\n");
						m_cache[m_req_path] = 0;
					}
				}
				else
				{
					printf("Bad JPEG data.\n");
					m_cache[m_req_path] = 0;
				}

			}
			else
			{
				int code = m_req->get_error();
				vector<char> buf;
				m_req->get_error_data(buf);
				string msg(buf.begin(),buf.end());
				
				printf("%s: %d\n%s\n", m_req_path.c_str(), code, msg.c_str());
				
				m_cache[m_req_path] = 0;
			}
			delete m_req;
			m_req = NULL;
			m_req_path.clear();
			m_buffer.clear();
		}
	}
}

void	WED_OSMSlippyMap::TimerFired()
{
	GetHost()->Refresh();
}
