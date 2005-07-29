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
#include "WED_TriTestTool.h"
#include "MapDefs.h"
#include "WED_Selection.h"
#include "XPLMGraphics.h"
#include "ParamDefs.h"
#include "WED_MapZoomer.h"
#include "WED_Globals.h"
#include "WED_Progress.h"
#include "MeshAlgs.h"
#include "DEMAlgs.h"
#include <gl.h>



WED_TriTestTool::WED_TriTestTool(WED_MapZoomer * inZoomer) : WED_MapTool(inZoomer)
{
}

WED_TriTestTool::~WED_TriTestTool()
{
}

void	WED_TriTestTool::DrawFeedbackUnderlay(
				bool				inCurrent)
{	
}				
		
void	WED_TriTestTool::DrawFeedbackOverlay(
				bool				inCurrent)
{
	if (inCurrent)
	{

	}
}
				
bool	WED_TriTestTool::HandleClick(
				XPLMMouseStatus		inStatus,
				int 				inX, 
				int 				inY,
				int 				inButton)
{
	if (inButton) return false;
	if (inStatus == xplm_MouseDown)
	{
		WED_MapZoomer * z = GetZoomer();
		Point2 p = Point2(z->XPixelToLon(inX), z->YPixelToLat(inY));
		gTriangulationHi.insert(CONVERT_POINT(p));
//		gTriangulationLo.insert(CONVERT_POINT(p));
	}
	return true;
}				
		
// Support for some properties that can be edited.	
int		WED_TriTestTool::GetNumProperties(void) { return 0; }
void	WED_TriTestTool::GetNthPropertyName(int, string&) { }
double	WED_TriTestTool::GetNthPropertyValue(int) { return 0.0; }
void	WED_TriTestTool::SetNthPropertyValue(int, double) { }

int		WED_TriTestTool::GetNumButtons(void) { return 3; }
void	WED_TriTestTool::GetNthButtonName(int n, string& s) { if (n == 0) s = "Retri hi"; else if (n == 1) s = "Retri lo"; else if (n == 2) s = "Road Density"; }

void	WED_TriTestTool::NthButtonPressed(int n)
{
}

char *	WED_TriTestTool::GetStatusText(void)
{
	static char buf[1024];
	
	int	x, y;
	double	lat, lon;
	XPLMGetMouseLocation(&x, &y);
	lat = GetZoomer()->YPixelToLat(y);
	lon = GetZoomer()->XPixelToLon(x);	
//	double h = MeshHeightAtPoint(gTriangulation, lon, lat);
//	if (h == NO_DATA)
	if (gDem.find(dem_Elevation) != gDem.end())
	{
		int x, y;
		float h = gDem[dem_Elevation].xy_nearest(lon, lat, x, y);
		if (h == NO_DATA)
			sprintf(buf, "Hires: %d  (%d,%d NO DATA)", gTriangulationHi.number_of_faces(), /*gTriangulationLo.number_of_faces(), */x, y);
		else
			sprintf(buf, "Hires: %d  (%d, %d h=%f)", gTriangulationHi.number_of_faces(), /*gTriangulationLo.number_of_faces(), */x, y, h);
	} else
		sprintf(buf, "Hires: %d ", gTriangulationHi.number_of_faces()/*, gTriangulationLo.number_of_faces()*/);
	return buf;
}
