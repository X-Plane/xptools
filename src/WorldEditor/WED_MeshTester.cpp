#include "WED_MeshTester.h"
#include "MapDefs.h"
#include "XPLMGraphics.h"
#include "WED_MapZoomer.h"
#include "WED_DrawMap.h"
#include "WED_Globals.h"
#include "WED_Document.h"
#include "WED_Selection.h"
#include "WED_Notify.h"
#include "WED_Msgs.h"
#include "MeshAlgs.h"

WED_MeshTester::WED_MeshTester(WED_MapZoomer * inZoomer) : WED_MapTool(inZoomer)
{
}
WED_MeshTester::~WED_MeshTester()
{
}

void	WED_MeshTester::DrawFeedbackUnderlay(
				bool				inCurrent)
{
}

void	WED_MeshTester::DrawFeedbackOverlay(
				bool				inCurrent)
{
	int mx, my;
	XPLMGetMouseLocation(&mx, &my);
	
	vector<Point2>		fps;
	if (inCurrent && mRayShoot)
	{
		mTarget.x = GetZoomer()->XPixelToLon(mx);
		mTarget.y = GetZoomer()->YPixelToLat(my);

		double	x1 = gDocument->gDem[dem_Elevation].mWest;
		double	x2 = gDocument->gDem[dem_Elevation].mEast;
		double	y1 = gDocument->gDem[dem_Elevation].mSouth;
		double	y2 = gDocument->gDem[dem_Elevation].mNorth;
		
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
			
			MarchHeightStart(gDocument->gTriangulationHi, CDT::Point(mAnchor.x, mAnchor.y), march_info);
			try {
				MarchHeightGo(gDocument->gTriangulationHi, CDT::Point(mTarget.x, mTarget.y), march_info, found);
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

bool	WED_MeshTester::HandleClick(
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

int		WED_MeshTester::GetNumProperties(void) { return 0; }
void	WED_MeshTester::GetNthPropertyName(int, string&) { }
double	WED_MeshTester::GetNthPropertyValue(int) { return 0.0; }
void	WED_MeshTester::SetNthPropertyValue(int, double) { }

int		WED_MeshTester::GetNumButtons(void) { return 0; }
void	WED_MeshTester::GetNthButtonName(int, string&) { }
void	WED_MeshTester::NthButtonPressed(int) { }

char *	WED_MeshTester::GetStatusText(void)
{
	return NULL;
}
