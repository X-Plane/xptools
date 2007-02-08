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
#include "WED_BezierTestTool.h"

#include "WED_MapTool.h"
#include "WED_MapZoomer.h"
#include "WED_Notify.h"
#include "WED_Msgs.h"
#include "WED_Globals.h"
#include "WED_Progress.h"
#include "MapAlgs.h"
#include "UIUtils.h"
#include "AssertUtils.h"
#include "WED_DrawMap.h"
#include "XPLMGraphics.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif

static const DragHandleInfo_t kHandleInfos[8] = {
	0,0,1,1,
	0,0,1,1,
	0,0,1,1,
	0,0,1,1,
	0,0,1,1,
	0,0,1,1,
	0,0,1,1,
	0,0,1,1 };

//static int self_intersect_recursive(const Bezier2& lhs, const Bezier2& rhs, int d, Bbox2& b1, Bbox2& b2);
//static int	intersect_recursive(const Bezier2& lhs, const Bezier2& rhs, int d, Bbox2& b1, Bbox2& b2);


WED_BezierTestTool::WED_BezierTestTool(WED_MapZoomer * inZoomer) :
	WED_MapTool(inZoomer),
	mHandles(8,kHandleInfos,4,this),
	mSegs(200),
	mSplit(0.5)
{	
}
						
WED_BezierTestTool::~WED_BezierTestTool()
{
}

void	WED_BezierTestTool::DrawFeedbackUnderlay(
							bool				inCurrent)
{
}

void	WED_BezierTestTool::DrawFeedbackOverlay(
							bool				inCurrent)
{
	if (inCurrent)
	{
		XPLMSetGraphicsState(0, 0, 0,    0, 0,  0, 0);
		glColor3f(0.0, 1.0, 0.3);
		glBegin(GL_QUADS);
		for (int n = 0; n < 8; ++n)
			mHandles.DrawHandle(n);
		glEnd();
		glBegin(GL_LINES);
		mHandles.ConnectHandle(0, 1);
		mHandles.ConnectHandle(2, 3);
		mHandles.ConnectHandle(4, 5);
		mHandles.ConnectHandle(6, 7);
		glEnd();
		
		Bezier2 a(mBezier1),b(mBezier2);
		
		int i = a.intersect(b,10);
		
		glColor3f(i || a.self_intersect(5) ? 1 : 0, i ? 0 : 1, 0);
		glBegin(GL_LINE_STRIP);
		for (int n = 0; n <= mSegs; ++n)
		{
			Point2 p = a.midpoint((float) n / (float) mSegs);
			glVertex2f(GetZoomer()->LonToXPixel(p.x),GetZoomer()->LatToYPixel(p.y));
		}
		glEnd();
		
		glColor3f(i || b.self_intersect(5) ? 1 : 0, i ? 0 : 1, 0);
		glBegin(GL_LINE_STRIP);
		for (int n = 0; n <= mSegs; ++n)
		{
			Point2 p = b.midpoint((float) n / (float) mSegs);
			glVertex2f(GetZoomer()->LonToXPixel(p.x),GetZoomer()->LatToYPixel(p.y));
		}
		glEnd();
		
		Bbox2	f1,f2;
		a.bounds(f1);
		b.bounds(f2);
		glColor3f(0.3,0.3,0.3);
		glBegin(GL_LINE_LOOP);
			glVertex2f(GetZoomer()->LonToXPixel(f1.p1.x),GetZoomer()->LatToYPixel(f1.p1.y));
			glVertex2f(GetZoomer()->LonToXPixel(f1.p1.x),GetZoomer()->LatToYPixel(f1.p2.y));
			glVertex2f(GetZoomer()->LonToXPixel(f1.p2.x),GetZoomer()->LatToYPixel(f1.p2.y));
			glVertex2f(GetZoomer()->LonToXPixel(f1.p2.x),GetZoomer()->LatToYPixel(f1.p1.y));		
		glEnd();

		glBegin(GL_LINE_LOOP);
			glVertex2f(GetZoomer()->LonToXPixel(f2.p1.x),GetZoomer()->LatToYPixel(f2.p1.y));
			glVertex2f(GetZoomer()->LonToXPixel(f2.p1.x),GetZoomer()->LatToYPixel(f2.p2.y));
			glVertex2f(GetZoomer()->LonToXPixel(f2.p2.x),GetZoomer()->LatToYPixel(f2.p2.y));
			glVertex2f(GetZoomer()->LonToXPixel(f2.p2.x),GetZoomer()->LatToYPixel(f2.p1.y));		
		glEnd();
	}
}
							
bool	WED_BezierTestTool::HandleClick(
							XPLMMouseStatus		inStatus,
							int 				inX, 
							int 				inY, 
							int 				inButton)
{
	if (inButton != 0) return false;
	switch(inStatus) {
	case xplm_MouseDown:
		return mHandles.StartDrag(inX, inY);
	case xplm_MouseDrag:
		mHandles.ContinueDrag(inX, inY);
		return true;
	case xplm_MouseUp:
		mHandles.EndDrag(inX, inY);
		return true;
	}
	return false;
}							

#pragma mark -

int		WED_BezierTestTool::GetNumProperties(void)
{
	return 2;
}

void	WED_BezierTestTool::GetNthPropertyName(int n, string& s)
{
	switch(n) {
	case 0: s = "Segments:";	break;
	case 1: s = "Split:";		break;
	}
}

double	WED_BezierTestTool::GetNthPropertyValue(int n)
{
	switch(n) {
	case 0: return mSegs;
	case 1: return mSplit;
	default: return 0;
	}
}

void	WED_BezierTestTool::SetNthPropertyValue(int n, double v)
{
	switch(n) {
	case 0: mSegs = v;	break;
	case 1: mSplit = v;	break;
	}
}

#pragma mark -
	
int		WED_BezierTestTool::GetNumButtons(void)
{
	return 1;
}

void	WED_BezierTestTool::GetNthButtonName(int n, string& s)
{
	switch(n) {
	case 0: s = "Reset";	break;
	}
}

void	WED_BezierTestTool::NthButtonPressed(int n)
{
	double bounds[4];
	switch(n) {
	case 0:
		GetZoomer()->GetMapVisibleBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
		mBezier1.p1=Point2(bounds[0],bounds[1]);
		mBezier1.c1=Point2(bounds[0],bounds[3]);
		mBezier1.c2=Point2(bounds[2],bounds[3]);
		mBezier1.p2=Point2(bounds[2],bounds[1]);	
		mBezier2.c1=Point2(bounds[0],bounds[1]);
		mBezier2.p1=Point2(bounds[0],bounds[3]);
		mBezier2.p2=Point2(bounds[2],bounds[3]);
		mBezier2.c2=Point2(bounds[2],bounds[1]);	
		break;
	}
}

char *	WED_BezierTestTool::GetStatusText(void)
{
	static char buf[512];
	double d[4];
	int n = mBezier1.monotone_regions(d);
	sprintf(buf,"(%d: %lf,%lf,%lf,%lf)", 
			n,d[0],d[1],d[2],d[3]);
	return buf;
}

#pragma mark -

double		WED_BezierTestTool::UIToLogX(double v) const
{
	return GetZoomer()->XPixelToLon(v);
}

double		WED_BezierTestTool::UIToLogY(double v) const
{
	return GetZoomer()->YPixelToLat(v);
}

double		WED_BezierTestTool::LogToUIX(double v) const
{
	return GetZoomer()->LonToXPixel(v);
}

double		WED_BezierTestTool::LogToUIY(double v) const
{
	return GetZoomer()->LatToYPixel(v);
}

double		WED_BezierTestTool::GetHandleX(int inHandle) const
{
	switch(inHandle) {
	case 0:	return mBezier1.p1.x;
	case 1:	return mBezier1.c1.x;
	case 2:	return mBezier1.c2.x;
	case 3:	return mBezier1.p2.x;
	case 4:	return mBezier2.p1.x;
	case 5:	return mBezier2.c1.x;
	case 6:	return mBezier2.c2.x;
	case 7:	return mBezier2.p2.x;
	default: return 0.0;
	}
}

double		WED_BezierTestTool::GetHandleY(int inHandle) const
{
	switch(inHandle) {
	case 0:	return mBezier1.p1.y;
	case 1:	return mBezier1.c1.y;
	case 2:	return mBezier1.c2.y;
	case 3:	return mBezier1.p2.y;
	case 4:	return mBezier2.p1.y;
	case 5:	return mBezier2.c1.y;
	case 6:	return mBezier2.c2.y;
	case 7:	return mBezier2.p2.y;
	default: return 0.0;
	}
}
	
void		WED_BezierTestTool::MoveHandleX(int handle, double deltaX)
{
	switch(handle) {
	case 0:	mBezier1.p1.x += deltaX; break;
	case 1:	mBezier1.c1.x += deltaX; break;
	case 2:	mBezier1.c2.x += deltaX; break;
	case 3:	mBezier1.p2.x += deltaX; break;
	case 4:	mBezier2.p1.x += deltaX; break;
	case 5:	mBezier2.c1.x += deltaX; break;
	case 6:	mBezier2.c2.x += deltaX; break;
	case 7:	mBezier2.p2.x += deltaX; break;
	}
}

void		WED_BezierTestTool::MoveHandleY(int handle, double deltaY)
{
	switch(handle) {
	case 0:	mBezier1.p1.y += deltaY; break;
	case 1:	mBezier1.c1.y += deltaY; break;
	case 2:	mBezier1.c2.y += deltaY; break;
	case 3:	mBezier1.p2.y += deltaY; break;
	case 4:	mBezier2.p1.y += deltaY; break;
	case 5:	mBezier2.c1.y += deltaY; break;
	case 6:	mBezier2.c2.y += deltaY; break;
	case 7:	mBezier2.p2.y += deltaY; break;
	}
}

