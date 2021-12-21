/*
 * Copyright (c) 2021, Laminar Research.
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

#include "WED_TerrainLayer.h"
#include "GUI_Pane.h"
#include "GUI_Fonts.h"
#include "GUI_GraphState.h"
#include "GUI_DrawUtils.h"
#include "WED_DrawUtils.h"

#include "WED_Colors.h"
#include "WED_Globals.h"
#include "WED_MapZoomerNew.h"
#include "WED_DSFImport.h"
#include "DSFLib.h"
#include "PlatformUtils.h"
#include "GISUtils.h"
#include "XESConstants.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif


WED_TerrainLayer::WED_TerrainLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(host,zoomer,resolver)
{
    SetVisible(false);
}

WED_TerrainLayer::~WED_TerrainLayer()
{
}

static bool NextPass(int finished_pass_index, void* inRef) { return true; }
static int	AcceptTerrainDef(const char* inPartialPath, void* inRef) { return 1; }
static int	AcceptObjectDef(const char* inPartialPath, void* inRef) { return 1; }
static int	AcceptPolygonDef(const char* inPartialPath, void* inRef) { return 1; }
static int	AcceptNetworkDef(const char* inPartialPath, void* inRef) { return 1; }
static void	AcceptProperty(const char* inProp, const char* inValue, void* inRef) {}
static void	BeginPatch(unsigned int	inTerrainType, double inNearLOD, double inFarLOD,
						unsigned char inFlags, int inCoordDepth, void* inRef) {}
static void	BeginPrimitive(	int		inType,	void* inRef) {}
static void	AddPatchVertex(	double	inCoordinates[], void* inRef) {}
static void	EndPrimitive( void* inRef) {}
static void	EndPatch(	void* inRef) {}
static void	AddObjectWithMode(unsigned int	inObjectType, double inCoordinates[4], obj_elev_mode inMode, void* inRef) {}
static void	BeginSegment(unsigned int inNetworkType, unsigned int inNetworkSubtype, double inCoordinates[], bool inCurved, void* inRef) {}
static void	AddSegmentShapePoint(double inCoordinates[], bool inCurved, void* inRef) {}
static void	EndSegment(double inCoordinates[], bool inCurved, void* inRef) {}
static void	BeginPolygon(unsigned int inPolygonType, unsigned short	inParam, int inCoordDepth, void* inRef) {}
static void	BeginPolygonWinding(void* inRef) {}
static void	AddPolygonPoint(double* inCoordinates, void* inRef) {}
static void	EndPolygonWinding(void* inRef) {}
static void	EndPolygon(void* inRef) {}
static void SetFilter_(int filterId, void* inRef) {}

static void AddRasterData(DSFRasterHeader_t* header, void* data, void* inRef)
{
	auto tile = (terrain_t *) inRef;
	if (tile->index < 0)
	{
		tile->index++;
		if (tile->index == 0)
		{
			tile->width = header->width;
			tile->height = header->height;
			tile->dem.reserve(header->width * header->height);

			if (header->bytes_per_pixel == 1)
			{
				auto raw_array = (int8_t*)data;
				for (int i = 0; i < header->width * header->height; i++)
				{
					if ((header->flags & 3) == 2)
						tile->dem.push_back((*raw_array) * header->scale + header->offset);
					else
						tile->dem.push_back(((uint8_t)*raw_array) * header->scale + header->offset);
					raw_array++;
				}
			}
			else if (header->bytes_per_pixel == 2)
			{
				auto raw_array = (uint16_t*)data;
				for (int i = 0; i < header->width * header->height; i++)
				{
					if ((header->flags & 3) == 2)
						tile->dem.push_back((*raw_array) * header->scale + header->offset);
					else
						tile->dem.push_back(((uint16_t)*raw_array) * header->scale + header->offset);
					raw_array++;
				}
			}
			else if (header->bytes_per_pixel == 4 && ((header->flags & 3) == 0))
			{
				auto raw_array = (float*)data;
				for (int i = 0; i < header->width * header->height; i++)
				{
					tile->dem.push_back((*raw_array) * header->scale + header->offset);
					raw_array++;
				}
			}
			else
				printf("bad bytecount or format\n");
		}
	}
}

static int AcceptRasterDef(const char* inPath, void* inRef)
{
	auto tile = (terrain_t *) inRef;
	if (tile->index >= 0)
	{
		tile->index++;
		if (strcmp(inPath, "elevation") == 0)
//		if (strcmp(inPath, "sea_level") == 0)
			tile->index = -tile->index;
	}
	return 1;
}

void WED_TerrainLayer::LoadTerrain(Bbox2& bounds)
{
	// ToDo: move this into PackageMgr, so its updated when XPlaneFolder changes and re-used when another scenery is opened
	set<string> vpaths;
	add_all_global_DSF(bounds, vpaths);

	DSFCallbacks_t cb = { NextPass, AcceptTerrainDef, AcceptObjectDef, AcceptPolygonDef, AcceptNetworkDef, AcceptRasterDef, AcceptProperty,
					BeginPatch, BeginPrimitive, AddPatchVertex, EndPrimitive, EndPatch,
					AddObjectWithMode, BeginSegment, AddSegmentShapePoint, EndSegment,
					BeginPolygon, BeginPolygonWinding, AddPolygonPoint,EndPolygonWinding, EndPolygon, AddRasterData, SetFilter_ };

	for (auto v : vpaths)
	{
		if (mTerrains.find(v) != mTerrains.end()) continue;
		terrain_t& tile = mTerrains[v];
		if (DSFReadFile(v.c_str(), malloc, free, &cb, NULL, &tile) == dsf_ErrOK)
		{
			int lon, lat;
			if(sscanf(v.substr(v.length() - 11, 7).c_str(), "%d%d", &lat, &lon) == 2)
				tile.bounds = { {(double) lon, (double) lat}, {(double) lon + 1, (double) lat + 1} };
		}
	}
	// printf("%ld mTerrains\n", mTerrains.size());
	// consider removing the most distant tile when more than xx tiles are loaded
}

void		WED_TerrainLayer::DrawVisualization		(bool inCurrent, GUI_GraphState * g)
{
	double ll,lb,lr,lt;	// logical boundary
	double vl,vb,vr,vt;	// visible boundry
	GetZoomer()->GetMapLogicalBounds(ll,lb,lr,lt);
	GetZoomer()->GetMapVisibleBounds(vl,vb,vr,vt);

	Bbox2 map_viewport({max(vl, ll), max(vb, lb)}, {min(vr, lr), min(vt, lt)});
	double PPM = GetZoomer()->GetPPM();

	const float terrain_color[4] = { 1.0, 0.3, 0.3, 1.0 };
	
	if (PPM > 0.5)
	{
		LoadTerrain(map_viewport);
		for (auto ter : mTerrains)
			if (ter.second.bounds.overlap(map_viewport))
			{
				auto t = &ter.second;
				double x_step = t->bounds.xspan() / (t->width - 1);
				double y_step = t->bounds.yspan() / (t->height - 1);
				
				if(x_step * DEG_TO_MTR_LAT * cosf(t->bounds.ymin() * DEG_TO_RAD) * PPM < 60) continue;

				vl = x_step * floor(max(map_viewport.xmin(), t->bounds.xmin()) / x_step);
				vr = x_step * ceil(min(map_viewport.xmax(), t->bounds.xmax()) / x_step);
				vb = y_step * floor(max(map_viewport.ymin(), t->bounds.ymin()) / y_step);
				vt = y_step * ceil(min(map_viewport.ymax(), t->bounds.ymax()) / y_step);

				g->SetState(false, 0, false, false, true, false, false);
				glColor4fv(terrain_color);
				
				for (double lon = vl; lon < vr; lon += x_step)
				{
					int x = round((lon - t->bounds.xmin()) / x_step);
					for (double lat = vb; lat < vt; lat += y_step)
					{
						int y = round((lat - t->bounds.ymin()) / y_step);
						if (x >= 0 && x < t->width && y >= 0 && y < t->height)
						{
							char c[16];
							//							snprintf(c, sizeof(c), "%d %d %.1f", x, y, t->dem[x + y * r->width]);
							if (gIsFeet)
								snprintf(c, sizeof(c), "%.0f%c", t->dem[x + y * t->width] / 0.3048, '\'');
							else
							{
								float hgt = t->dem[x + y * t->width];
									snprintf(c, sizeof(c), hgt < 200.0 ? "%.1f%c" : "%.0f%c", hgt, 'm');
							}
							Point2 pt(GetZoomer()->LLToPixel({ lon, lat }));
							GUI_FontDraw(g, font_UI_Basic, terrain_color, pt.x() + 2, pt.y() + 4, c);
							g->SetState(false, 0, false, false, true, false, false);
							glColor4fv(terrain_color);
							glBegin(GL_LINES);
							glVertex2f(pt.x() - 8.0, pt.y());
							glVertex2f(pt.x() + 8.0, pt.y());
							glVertex2f(pt.x(), pt.y() - 8.0);
							glVertex2f(pt.x(), pt.y() + 8.0);
							glEnd();
						}
					}
				}
			}
	}
}

void		WED_TerrainLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = draw_ent_s = cares_about_sel = wants_clicks = 0;
}
