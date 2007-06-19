#include "GUI_ScrollerPane.h"
#include "GUI_ScrollBar.h"
#include "GUI_Messages.h"
#include "GUI_DrawUtils.h"
#if APL
#include <OpenGL/gl.h>
#else
#include <gl/gl.h>
#endif
const int	kSBSIZE = 16;

/*
	Scroller deferred realignment:
		Scroller views attempt to recompute the scroll bar limits and sizes when the content of the pane changes.  However in the case
		of hierarchy views and other large content this can be quite expensive: O(N) where N = size of content, not size of what we can
		realy see.  SO....we don't do this immediately when signaled - we simply mark ourselves as stale, then do it when we draw.  

		(Technically this is slightly wrong -- we should also try to trap mouse events, because technically someone could show the scroller
		and click on the scrollbar itself while it is not calibrated before the refresh comes through.  But this would require super-human
		user speed and be a uesless click anyway since the user would be clicking on something not yet drawn.)
		
		The big win here is that when we have 5 hierarchies, some fully open, but most hidden, we don't "pay" for the hidden ones in any way,
		because we don't have to recompute their size.
*/	

GUI_ScrollerPane::GUI_ScrollerPane(int inHScroll, int inVScroll) :
	mScrollH(NULL),
	mScrollV(NULL),
	mContent(NULL),
	mCalibrating(false)
{
	// Note: we have to set some real bounds - default panes are zero rect.
	// but we need to establish relative dimensions to our scroll bars from the outset!
	
	SetBounds(0,0,100,100);
	
	if (inHScroll)
	{
		mScrollH = new GUI_ScrollBar();
		mScrollH->SetParent(this);
		mScrollH->AddListener(this);
		mScrollH->SetBounds(0, 0, inVScroll ? (100-mScrollH->GetMinorAxis(1)) : 100, mScrollH->GetMinorAxis(0));
		mScrollH->SetMin(0);
		mScrollH->SetMax(0);
		mScrollH->SetValue(0);
		mScrollH->SetPageSize(1);
		mScrollH->SetSticky(1,1,1,0);
		mScrollH->Show();
	}
	if (inVScroll)
	{
		mScrollV = new GUI_ScrollBar();
		mScrollV->SetParent(this);
		mScrollV->AddListener(this);
		mScrollV->SetBounds(100-mScrollV->GetMinorAxis(1), inHScroll ? mScrollV->GetMinorAxis(0) : 0, 100, 100);
		mScrollV->SetMin(0);
		mScrollV->SetMax(0);
		mScrollV->SetValue(0);
		mScrollV->SetPageSize(1);
		mScrollV->SetSticky(0,1,1,1);
		mScrollV->Show();
	}
}

GUI_ScrollerPane::~GUI_ScrollerPane()
{
}


void	GUI_ScrollerPane::Draw(GUI_GraphState * g)
{
	if (mCalibrateDirty)
		CalibrateSBs();
	int bounds[4];
	int tile[4] = { 0, 0, 1, 1 };
	
	if (!mImage.empty())
	{
		glColor3f(1,1,1);
		GetBounds(bounds);
		GUI_DrawStretched(g, mImage.c_str(), bounds, tile);
	}
	
	if (mScrollV && mScrollH)
	{
		glColor3f(1,1,1);
		GetBounds(bounds);
		GUI_DrawCentered(g, "scrollbar_corner.png", bounds, 1, -1, tile, NULL, NULL);
	}
}

void	GUI_ScrollerPane::AttachSlaveH(GUI_ScrollerPane *inSlaveH)
{
	mSlaveH.push_back(inSlaveH);
}

void	GUI_ScrollerPane::AttachSlaveV(GUI_ScrollerPane *inSlaveV)
{
	mSlaveV.push_back(inSlaveV);
}

void	GUI_ScrollerPane::SetImage(const char * image_res)
{
	mImage = image_res;
}

void	GUI_ScrollerPane::PositionInContentArea(GUI_Pane * inPane)
{
	int bounds_me[4];
	this->GetBounds(bounds_me);
	if (mScrollH)
		bounds_me[1] += mScrollH->GetMinorAxis(0);
	if (mScrollV)
		bounds_me[2] -= mScrollV->GetMinorAxis(1);
	
	inPane->SetBounds(bounds_me);
	inPane->SetSticky(1,1,1,1);
}

void	GUI_ScrollerPane::PositionSidePane(GUI_Pane * pane)
{
	int bounds_me[4];
	this->GetBounds(bounds_me);
	if (mScrollH)
		bounds_me[1] += mScrollH->GetMinorAxis(0);
	if (mScrollV)
		bounds_me[2] -= mScrollV->GetMinorAxis(1);
	int bounds_child[4];
	pane->GetBounds(bounds_child);
	bounds_child[1] = bounds_me[1];
	bounds_child[3] = bounds_me[3];
	pane->SetBounds(bounds_child);	
}

void	GUI_ScrollerPane::PositionHeaderPane(GUI_Pane * pane)
{
	int bounds_me[4];
	this->GetBounds(bounds_me);
	if (mScrollH)
		bounds_me[1] += mScrollH->GetMinorAxis(0);

// Bens: let header overhang scrollbar.  Sider does not!		

//	if (mScrollV)				
//		bounds_me[2] -= mScrollV->GetMinorAxis(1);
	int bounds_child[4];
	pane->GetBounds(bounds_child);
	bounds_child[0] = bounds_me[0];
	bounds_child[2] = bounds_me[2];
	pane->SetBounds(bounds_child);	
}

void	GUI_ScrollerPane::SetContent(GUI_ScrollerPaneContent * inPane)
{
	if (mContent != NULL)
		mContent->RemoveListener(this);
	mContent = inPane;
	if (mContent != NULL)
		mContent->AddListener(this);
	mCalibrateDirty = true;
}

void	GUI_ScrollerPane::ContentGeometryChanged(void)
{
	mCalibrateDirty = true;
}

void	GUI_ScrollerPane::SetBounds(int x1, int y1, int x2, int y2)
{
	GUI_Pane::SetBounds(x1, y1, x2, y2);
	mCalibrateDirty = true;
}

void	GUI_ScrollerPane::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);
	mCalibrateDirty = true;
}

int		GUI_ScrollerPane::ScrollWheel(int x, int y, int dist, int axis)
{
	if (mContent)
	{
		float total[6], vis[6];
		mContent->GetScrollBounds(total, vis);
		total[4] = total[2] - total[0];
		total[5] = total[3] - total[1];
		vis[4] = vis[2] - vis[0];
		vis[5] = vis[3] - vis[1];
		
		if (axis == 0)
		{		
			float minv = 0;
			float maxv = max(total[5] - vis[5], 0.0f);
			float new_v = vis[1] - total[1] + dist;
			float old_v = vis[1] - total[1];
			
			if (maxv == 0.0) return 1;
			
			if (new_v < minv) new_v = minv;
			if (new_v > maxv) new_v = maxv;

			if (old_v != new_v)
			{
				mContent->ScrollV(new_v);
				mCalibrateDirty = true;
				Refresh();
			}
		} else {
			float minv = 0;
			float maxv = max(total[4] - vis[4], 0.0f);
			float new_v = vis[0] - total[0] - dist;
			float old_v = vis[0] - total[0];
			
			if (maxv == 0.0) return 1;
			
			if (new_v < minv) new_v = minv;
			if (new_v > maxv) new_v = maxv;

			if (old_v != new_v)
			{
				mContent->ScrollH(new_v);
				mCalibrateDirty = true;
				Refresh();
			}
		}
		return 1;
	}
	return 0;
}


void	GUI_ScrollerPane::ReceiveMessage(
		GUI_Broadcaster *		inSrc,
		int						inMsg,
		int						inParam)
{
	vector<GUI_ScrollerPane*>::iterator slave;
	if (inSrc == mScrollH && inMsg == GUI_CONTROL_VALUE_CHANGED && mContent && !mCalibrating)
	{
		mContent->ScrollH(mScrollH->GetValue());
		for (slave = mSlaveH.begin(); slave != mSlaveH.end(); ++slave)
			(*slave)->mContent->ScrollH(mScrollH->GetValue());
	}
	if (inSrc == mScrollV && inMsg == GUI_CONTROL_VALUE_CHANGED && mContent && !mCalibrating)
	{
		mContent->ScrollV(mScrollV->GetValue());
		for (slave = mSlaveV.begin(); slave != mSlaveV.end(); ++slave)
			(*slave)->mContent->ScrollV(mScrollV->GetValue());
	}
		
	if (inSrc == mContent && inMsg == GUI_SCROLL_CONTENT_SIZE_CHANGED && mContent && !mCalibrating)
		mCalibrateDirty = true;
}

void	GUI_ScrollerPane::CalibrateSBs(void)
{
	mCalibrateDirty = false;
	mCalibrating = true;

	if (mContent == NULL)
	{
		if (mScrollH)
		{
			mScrollH->SetMin(0);
			mScrollH->SetMax(0);
			mScrollH->SetValue(0);
			mScrollH->SetPageSize(1);
		}
		if (mScrollV)
		{
			mScrollV->SetMin(0);
			mScrollV->SetMax(0);
			mScrollV->SetValue(0);
			mScrollV->SetPageSize(1);
		}
	} else {
		float total[6], vis[6];
		mContent->GetScrollBounds(total, vis);
		total[4] = total[2] - total[0];
		total[5] = total[3] - total[1];
		vis[4] = vis[2] - vis[0];
		vis[5] = vis[3] - vis[1];
		
		if (mScrollH)
		{
			mScrollH->SetMin(0);
			mScrollH->SetMax(max(total[4] - vis[4], 0.0f));
			mScrollH->SetValue(vis[0] - total[0]);
			mScrollH->SetPageSize(vis[4]);
		}
		if (mScrollV)
		{
			mScrollV->SetMin(0);
			mScrollV->SetMax(max(total[5] - vis[5], 0.0f));
			mScrollV->SetValue(vis[1] - total[1]);
			mScrollV->SetPageSize(vis[5]);
		}		
	}
	mCalibrating = false;
}
