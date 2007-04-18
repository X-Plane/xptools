#include "WED_CreateToolBase.h"
#include "WED_MapZoomerNew.h"
#include "GUI_GraphState.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
#endif

const float kDoubleClickTime = 0.2;
const int kDoubleClickDist = 3;
const int kCloseLoopDist = 5;
const int BEZ_STEPS = 50;


static void ogl_bezier(const Bezier2& b, WED_MapZoomerNew * z)
{
	if (b.p1 == b.c1 && b.p2 == b.c2)
	{
		glBegin(GL_LINE_STRIP);
		glVertex2f(z->LonToXPixel(b.p1.x),z->LatToYPixel(b.p1.y));
		glVertex2f(z->LonToXPixel(b.p2.x),z->LatToYPixel(b.p2.y));
		glEnd();
	}
	else
	{
		glBegin(GL_LINE_STRIP);
		for (int t = 0; t <= BEZ_STEPS; ++t)
		{
			float r = (float) t / (float) BEZ_STEPS;
			Point2 m = b.midpoint(r);
			glVertex2f(z->LonToXPixel(m.x),z->LatToYPixel(m.y));
		}
		glEnd();		
	}
}

WED_CreateToolBase::WED_CreateToolBase(
									GUI_Pane *			host,
									WED_MapZoomerNew *	zoomer, 
									IResolver *			resolver,
									WED_Archive *		archive,									
									int					min_num_pts,
									int					max_num_pts,
									int					can_curve,
									int					must_curve,
									int					can_close,
									int					must_close) :
	WED_MapToolNew(host,zoomer,resolver),
	mArchive(archive),
	mLastTime(-9.9e9),
	mDirOpen(0),
	mCreating(0),	
	mMinPts(min_num_pts),
	mMaxPts(max_num_pts),
	mCanClose(can_close),
	mMustClose(must_close),
	mCanCurve(can_curve),
	mMustCurve(must_curve)
{
}
	
WED_CreateToolBase::~WED_CreateToolBase()
{
}

void		WED_CreateToolBase::DrawStructure(int inCurrent, GUI_GraphState * g)
{
	g->SetState(false, 0, false,    false, true, false, false);
	
	// Existing segments!
	
	glColor3f(1,0,0);
	for(int n = 1; n < mPts.size(); ++n)
	{
		Bezier2	c;
		c.p1 = mPts [n-1];
		c.c1 = mDirs[n-1];
		c.p2 = mPts [n  ];
		c.c2 = mPts [n  ]+Vector2(mDirs[n],mPts[n]);
		
		ogl_bezier(c,GetZoomer());
	}
	
	Point2	impute_pt,impute_dir;
		
	if(mCreating)
	{
		impute_pt = Point2(GetZoomer()->XPixelToLon(mStartX),GetZoomer()->YPixelToLat(mStartY));
		impute_dir = Point2(GetZoomer()->XPixelToLon(mNowX),GetZoomer()->YPixelToLat(mNowY));
		if (!mDirOpen) impute_dir = impute_pt;
	}
	else
	{
		int x, y;
		GetHost()->GetMouseLocNow(&x,&y);
		impute_pt = impute_dir = Point2(GetZoomer()->XPixelToLon(x),GetZoomer()->YPixelToLat(y));
		
	}
	
	if (mCreating && mDirOpen)
	{
		glColor3f(1,1,0);
		glBegin(GL_LINES);

		Point2 p1 = impute_pt ;
		Point2 p2 = impute_dir;
		if (!mPts.empty())
			p1 = p1 + Vector2(p2,p1);

		glVertex2f(GetZoomer()->LonToXPixel(p1.x),GetZoomer()->LatToYPixel(p1.y));
		glVertex2f(GetZoomer()->LonToXPixel(p2.x),GetZoomer()->LatToYPixel(p2.y));		
		
		glEnd();
	}	

	if (!mPts.empty())
	{
		glColor4f(1,1,1,0.5);
		Bezier2	active;
		active.p1 = mPts.back();
		active.c1 = mDirs.back();
		active.p2 = impute_pt;
		active.c2 = impute_pt+Vector2(impute_dir,impute_pt);
		ogl_bezier(active,GetZoomer());
		
		if(mMustClose)
		{
			glColor4f(1,0,0,0.5);
			Bezier2	closer;
			closer.p1 = impute_pt;
			closer.c1 = impute_dir;
			closer.p2 = mPts.front();
			closer.c2 = mPts.front()+Vector2(mDirs.front(),mPts.front());
			ogl_bezier(closer,GetZoomer());
		}	

		GetHost()->Refresh();
		#if !DEV
			we need to examine this
		#endif
	}
	
}


int			WED_CreateToolBase::HandleClickDown(int inX, int inY, int inButton)
{
	if (inButton > 0) return 0;
	int now = GetHost()->GetTimeNow();
	if (now-mLastTime < kDoubleClickTime &&
		fabs(inX-mLastX) < kDoubleClickDist &&
		fabs(inY-mLastY) < kDoubleClickDist)
	{
		if (mPts.size() >= mMinPts)
			DoEmit(0);
		mCreating = 0;
	}
	else
	{
		mNowX = mStartX = inX;
		mNowY = mStartY = inY;
		mDirOpen = mMustCurve;
		mCreating = 1;
	}

	mLastTime = now;
	mLastX = inX;
	mLastY = inY;
	return 1;
}


void		WED_CreateToolBase::HandleClickDrag(int inX, int inY, int inButton)
{
	if (!mCreating) return;
	
	if (!mDirOpen && mCanCurve && 
		(fabs(inX - mStartX) > kDoubleClickDist || fabs(inY - mStartY) > kDoubleClickDist))
	{
		mDirOpen = 1;
	}
	mNowX = inX;
	mNowY = inY;
}

void		WED_CreateToolBase::HandleClickUp  (int inX, int inY, int inButton)
{
	if (!mCreating) return;

	if(mDirOpen)
	{
		mPts.push_back(Point2(GetZoomer()->XPixelToLon(mStartX),GetZoomer()->YPixelToLat(mStartY)));
		mDirs.push_back(Point2(GetZoomer()->XPixelToLon(inX),GetZoomer()->YPixelToLat(inY)));
		mHasDirs.push_back(1);
	}
	else
	{
		mPts.push_back(Point2(GetZoomer()->XPixelToLon(mStartX),GetZoomer()->YPixelToLat(mStartY)));
		mDirs.push_back(Point2(GetZoomer()->XPixelToLon(mStartX),GetZoomer()->YPixelToLat(mStartY)));
		mHasDirs.push_back(0);
	}
	
	if(mPts.size() >= mMaxPts)
		DoEmit(0);		
	// We can only check for a closed loop on:
	// - closed-loop-possible chains with
	// - at least 3 points (so one can be redundent) and
	// - no curve at the end
	// - we have enough pts that throwing one out is okay
	else if (mCanClose && mPts.size() > 2 && !mHasDirs.back() && mPts.size() > mMinPts)
	{
		int dist_x = GetZoomer()->LonToXPixel(mPts.front().x) - GetZoomer()->LonToXPixel(mPts.back().x);
		int dist_y = GetZoomer()->LatToYPixel(mPts.front().y) - GetZoomer()->LatToYPixel(mPts.back().y);
		if (fabs(dist_x) < kCloseLoopDist && fabs(dist_y) < kCloseLoopDist)
			DoEmit(1);
	}
		
	mDirOpen = 0;
	mCreating = 0;
}


void		WED_CreateToolBase::DoEmit(int do_close)
{
	int closed = mMustClose || do_close;
		
	if (do_close)
	{
		do_close = 1;
		mPts.pop_back();
		mDirs.pop_back();
		mHasDirs.pop_back();
	}

	this->AcceptPath(mPts,mHasDirs,mDirs,closed);
	mPts.clear();
	mHasDirs.clear();
	mDirs.clear();

}

