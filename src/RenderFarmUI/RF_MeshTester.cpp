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

#include "RF_MeshTester.h"
#include "MapDefs.h"
#include "XPLMGraphics.h"
#include "RF_MapZoomer.h"
#include "RF_DrawMap.h"
#include "RF_Globals.h"
#include "RF_Selection.h"
#include "RF_Notify.h"
#include "RF_Msgs.h"
#include "MeshAlgs.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif


RF_MeshTester::RF_MeshTester(RF_MapZoomer * inZoomer) : RF_MapTool(inZoomer)
{
}
RF_MeshTester::~RF_MeshTester()
{
}

void	RF_MeshTester::DrawFeedbackUnderlay(
				bool				inCurrent)
{
}

void	RF_MeshTester::DrawFeedbackOverlay(
				bool				inCurrent)
{
	int mx, my;
	XPLMGetMouseLocation(&mx, &my);

	vector<Point2>		fps;
	if (inCurrent && mRayShoot)
	{
		mTarget.x = GetZoomer()->XPixelToLon(mx);
		mTarget.y = GetZoomer()->YPixelToLat(my);

		double	x1 = gDem[dem_Elevation].mWest;
		double	x2 = gDem[dem_Elevation].mEast;
		double	y1 = gDem[dem_Elevation].mSouth;
		double	y2 = gDem[dem_Elevation].mNorth;

		if (mTarget.x < x1) mTarget.x = x1;
		if (mTarget.x > x2) mTarget.x = x2;
		if (mAnchor.x < x1) mAnchor.x = x1;
		if (mAnchor.x > x2) mAnchor.x = x2;

		if (mTarget.y < y1) mTarget.y = y1;
		if (mTarget.y > y2) mTarget.y = y2;
		if (mAnchor.y < y1) mAnchor.y = y1;
		if (mAnchor.y > y2) mAnchor.y = y2;

		Vector2	dist(mAnchor, mTarget);
		if (XPLMGetModifiers() & xplm_ShiftFlag)
		{
			if (abs(dist.dx) < abs(dist.dy))
				mTarget.x = mAnchor.x;
			else
				mTarget.y = mAnchor.y;
		}

		if (mTarget != mAnchor)
		{
			vector<Point3>	found;

			static CDT_MarchOverTerrain_t march_info;

			MarchHeightStart(gTriangulationHi, CDT::Point(mAnchor.x, mAnchor.y), march_info);
			try {
				MarchHeightGo(gTriangulationHi, CDT::Point(mTarget.x, mTarget.y), march_info, found);
			} catch (...) {
				found.push_back(Point3(mTarget.x, mTarget.y, 0.0));
				printf("Got err with: %lf,%lf->%lf,%lf\n", mAnchor.x,mAnchor.y,mTarget.x,mTarget.y);
				mRayShoot = false;
			}

			XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0);
			glColor4f(1.0, 0.0, 1.0, 0.8);
			glBegin(GL_LINE_STRIP);
			for (int n = 0; n < found.size(); ++n)
				glVertex2f( GetZoomer()->LonToXPixel(found[n].x),
							GetZoomer()->LatToYPixel(found[n].y));
			glEnd();

			glPointSize(3);

			glColor4f(1.0, 1.0, 0.0, 0.8);
			glBegin(GL_POINTS);
			for (int n = 0; n < found.size(); ++n)
				glVertex2f( GetZoomer()->LonToXPixel(found[n].x),
							GetZoomer()->LatToYPixel(found[n].y));
			glEnd();

			glPointSize(1);
		}
	}
}

bool	RF_MeshTester::HandleClick(
				XPLMMouseStatus		inStatus,
				int 				inX,
				int 				inY,
				int 				inButton)
{
	if (inButton > 0) return false;
	switch(inStatus) {
	case xplm_MouseDown:
		{
			mAnchor.x = GetZoomer()->XPixelToLon(inX);
			mAnchor.y = GetZoomer()->YPixelToLat(inY);
		}
		mRayShoot = true;
		break;
	case xplm_MouseUp:
		mRayShoot = false;
		break;
	}
	return 1;
}

int		RF_MeshTester::GetNumProperties(void) { return 0; }
void	RF_MeshTester::GetNthPropertyName(int, string&) { }
double	RF_MeshTester::GetNthPropertyValue(int) { return 0.0; }
void	RF_MeshTester::SetNthPropertyValue(int, double) { }

int		RF_MeshTester::GetNumButtons(void) { return 0; }
void	RF_MeshTester::GetNthButtonName(int, string&) { }
void	RF_MeshTester::NthButtonPressed(int) { }

char *	RF_MeshTester::GetStatusText(void)
{
	return NULL;
}
