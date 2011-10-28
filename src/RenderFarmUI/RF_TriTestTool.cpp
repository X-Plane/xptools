/*
 * Copyright (c) 2004, Laminar Research.
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
#include "RF_TriTestTool.h"
#include "MapDefs.h"
#include "RF_Selection.h"
#include "GUI_GraphState.h"
#include "ParamDefs.h"
#include "RF_MapZoomer.h"
#include "RF_Globals.h"
#include "RF_Progress.h"
#include "MeshAlgs.h"
#include "DEMAlgs.h"
#include "GISTool_Globals.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif




RF_TriTestTool::RF_TriTestTool(RF_MapZoomer * inZoomer) : RF_MapTool(inZoomer)
{
}

RF_TriTestTool::~RF_TriTestTool()
{
}

void	RF_TriTestTool::DrawFeedbackUnderlay(
							GUI_GraphState *	state,
				bool				inCurrent)
{
}

void	RF_TriTestTool::DrawFeedbackOverlay(
							GUI_GraphState *	state,
				bool				inCurrent)
{
	if (inCurrent)
	{

	}
}

bool	RF_TriTestTool::HandleClick(
				XPLMMouseStatus		inStatus,
				int 				inX,
				int 				inY,
				int 				inButton,
				GUI_KeyFlags		inModifiers)
{
	if (inButton) return false;
	if (inStatus == xplm_MouseDown)
	{
		RF_MapZoomer * z = GetZoomer();
		Point2 p = Point2(z->XPixelToLon(inX), z->YPixelToLat(inY));
		gTriangulationHi.insert(ben2cgal<Point_2>(p));
//		gTriangulationLo.insert(CONVERT_POINT(p));
	}
	return true;
}

// Support for some properties that can be edited.
int		RF_TriTestTool::GetNumProperties(void) { return 0; }
void	RF_TriTestTool::GetNthPropertyName(int, string&) { }
double	RF_TriTestTool::GetNthPropertyValue(int) { return 0.0; }
void	RF_TriTestTool::SetNthPropertyValue(int, double) { }

int		RF_TriTestTool::GetNumButtons(void) { return 3; }
void	RF_TriTestTool::GetNthButtonName(int n, string& s) { if (n == 0) s = "Retri hi"; else if (n == 1) s = "Retri lo"; else if (n == 2) s = "Road Density"; }

void	RF_TriTestTool::NthButtonPressed(int n)
{
}

char *	RF_TriTestTool::GetStatusText(int x, int y)
{
	static char buf[1024];

	double	lat, lon;
	lat = GetZoomer()->YPixelToLat(y);
	lon = GetZoomer()->XPixelToLon(x);
//	double h = MeshHeightAtPoint(gTriangulation, lon, lat);
//	if (h == DEM_NO_DATA)
	if (gDem.find(dem_Elevation) != gDem.end())
	{
		int x, y;
		float h = gDem[dem_Elevation].xy_nearest(lon, lat, x, y);
		if (h == DEM_NO_DATA)
			sprintf(buf, "Hires: %llu  (%d,%d NO DATA)", (unsigned long long)gTriangulationHi.number_of_faces(), /*gTriangulationLo.number_of_faces(), */x, y);
		else
			sprintf(buf, "Hires: %llu  (%d, %d h=%f)", (unsigned long long)gTriangulationHi.number_of_faces(), /*gTriangulationLo.number_of_faces(), */x, y, h);
	} else
		sprintf(buf, "Hires: %llu ", (unsigned long long)gTriangulationHi.number_of_faces()/*, gTriangulationLo.number_of_faces()*/);
	return buf;
}
