#include "GUI_TabControl.h"
#include "GUI_Messages.h"
#include "GUI_Fonts.h"
#include "GUI_GraphState.h"
#include "GUI_DrawUtils.h"
#include "GUI_Resources.h"

#define		TAB_PADDING	15
#define		TAB_BASELINE 10
#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

GUI_TabControl::GUI_TabControl()
{
}

GUI_TabControl::~GUI_TabControl()
{
}


int		GUI_TabControl::GetNaturalHeight(void)
{
		GUI_TexPosition_t	metrics;

	GUI_GetTextureResource("tabs.png", 0, &metrics);
	return metrics.real_height / 3;
}

void		GUI_TabControl::SetDescriptor(const string& inDesc)
{
	GUI_Control::SetDescriptor(inDesc);

	mItems.clear();

	string::const_iterator b, e;
	b = inDesc.begin();
	while (b != inDesc.end())
	{
		e = b;
		while (e != inDesc.end() && *e != '\n') ++e;
		mItems.push_back(string(b,e));
		if (e != inDesc.end()) ++e;
		b = e;
	}
	
	mWidths.resize(mItems.size());
	for (int n = 0; n < mItems.size(); ++n)
	{
		mWidths[n] = GUI_MeasureRange(font_UI_Basic, &*mItems[n].begin(), &*mItems[n].end()) + TAB_PADDING * 2;
	}
	
	SetMax(mItems.size());
	if (GetValue() > GetMax()) SetValue(GetMax());
	Refresh();
}

void		GUI_TabControl::Draw(GUI_GraphState * state)
{
	int	bounds[4];
	GetBounds(bounds);
	int tile_line[4] = { 0, 0, 1, 3 };
	glColor3f(1,1,1);
	GUI_DrawHorizontalStretch(state,"tabs.png",bounds,tile_line);
	
	int n;
	for (n = 0; n < mItems.size(); ++n)
	{
		int tile_tab[4] = { 0, (n == GetValue()) ? 2 : 1, 1, 3 };
		bounds[2] = bounds[0] + mWidths[n];
		GUI_DrawHorizontalStretch(state,"tabs.png",bounds,tile_tab);
		bounds[0] = bounds[2];				
	}
	
	float c[4] = { 0,0,0,1};
	float ch[4] = { 0.5, 0.5, 0.5, 1 };
	GetBounds(bounds);
	for (n = 0; n < mItems.size(); ++n)
	{
		GUI_FontDraw(state, font_UI_Basic, (n == mTrackBtn && mHilite) ? ch : c, (bounds[0] + TAB_PADDING), bounds[1] + TAB_BASELINE, mItems[n].c_str());
		bounds[0] += mWidths[n];
	}
	
}

int			GUI_TabControl::MouseDown(int x, int y, int button)
{
	int bounds[4];
	GetBounds(bounds);
	mTrackBtn = -1;
	for (int n = 0; n < mWidths.size(); ++n)
	{
		bounds[2] = bounds[0] + mWidths[n];
		if (x > bounds[0] &&
			x < bounds[2])
		{
			mTrackBtn = n;
			mHilite = 1;
			Refresh();
			return 1;
		}
		bounds[0] = bounds[2];
	}
	return 0;
}

void		GUI_TabControl::MouseDrag(int x, int y, int button)
{
	if (mTrackBtn == -1) return;
	
	int bounds[4];
	GetBounds(bounds);
	for (int n = 0; n < mTrackBtn; ++n)
		bounds[0] += mWidths[n];
	
	bounds[2] = bounds[0] + mWidths[mTrackBtn];
	
	int is_in = (x > bounds[0] && x < bounds[2] &&
			  y > bounds[1] && y < bounds[3]);
	if (is_in != mHilite)
	{
		mHilite = is_in;
		Refresh();
	}
}

void		GUI_TabControl::MouseUp  (int x, int y, int button)
{
	if (mTrackBtn == -1) return;
	
	int bounds[4];
	GetBounds(bounds);
	for (int n = 0; n < mTrackBtn; ++n)
		bounds[0] += mWidths[n];
	
	bounds[2] = bounds[0] + mWidths[mTrackBtn];
	
	int is_in = (x > bounds[0] && x < bounds[2] &&
			  y > bounds[1] && y < bounds[3]);
	if (is_in)
	{
		SetValue(mTrackBtn);
	}
	mTrackBtn = -1;
	mHilite = 0;
}

void		GUI_TabControl::SetValue(float inValue)
{
	GUI_Control::SetValue(inValue);
	Refresh();
}
	
