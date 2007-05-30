#include "GUI_Splitter.h"
#include "GUI_GraphState.h"
#include "GUI_DrawUtils.h"
#include "GUI_Resources.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
#endif

//const int	gui_SplitSize = 5;

GUI_Splitter::GUI_Splitter(int direction)
	: mDirection(direction),
	  mClick(0)
{
} 

GUI_Splitter::~GUI_Splitter()
{
}

void		GUI_Splitter::Draw(GUI_GraphState * state)
{
	if (CountChildren() > 1)
	{
		int		b[4], b1[4], b2[4];
		GetBounds(b);
		GetNthChild(0)->GetBounds(b1);
		GetNthChild(1)->GetBounds(b2);

		
		glColor3f(1,1,1);
		if (mDirection == gui_Split_Vertical)
		{
			int tile_sel[4] = { 0, mClick ? 1 : 0, 1, 2 };
			b[1] = b1[3];
			b[3] = b2[1];
			GUI_DrawHorizontalStretch(state, "splitter_h.png", b, tile_sel);			
		} else {
			b[0] = b1[2];
			b[2] = b2[0];
			int tile_sel[4] = { mClick ? 1 : 0, 0, 2, 1 };
			GUI_DrawVerticalStretch(state, "splitter_v.png", b, tile_sel);
		}
	}
}


int			GUI_Splitter::MouseDown(int x, int y, int button)
{
	if (CountChildren() > 1)
	{
		int b[4];
		GetNthChild(0)->GetBounds(b);
		
		mSlop = (mDirection == gui_Split_Horizontal ? x : y) -
				(mDirection == gui_Split_Horizontal ? b[2] : b[3]);
		
		mClick = 1;
		Refresh();
		return 1;
	}
	return 0;
}

void		GUI_Splitter::MouseDrag(int x, int y, int button)
{
	// slop = mouse - boundary so
	// boundary = mouse - slop
	
	if (CountChildren() > 1)
	{
		int b[4], b1[4], b2[4];
		GetBounds(b);
		GetNthChild(0)->GetBounds(b1);
		GetNthChild(1)->GetBounds(b2);
		
		if (mDirection == gui_Split_Horizontal)
		{
			b1[2] = x - mSlop;
			b2[0] = x - mSlop + GetSplitSize();
			b1[1] = b2[1] = b[1];
			b1[3] = b2[3] = b[3];
		} else {
			b1[3] = y - mSlop;
			b2[1] = y - mSlop + GetSplitSize();
			b1[0] = b2[0] = b[0];
			b1[2] = b2[2] = b[2];
		}
		GetNthChild(0)->SetBounds(b1);
		GetNthChild(1)->SetBounds(b2);		
	}
}

void		GUI_Splitter::MouseUp(int x, int y, int button)
{
	mClick = 0;
	Refresh();
}

void		GUI_Splitter::AlignContents()
{
	if (CountChildren() > 1)
	{
		int b[4], b1[4], b2[4];
		GetBounds(b);
		GetNthChild(0)->GetBounds(b1);
		GetNthChild(1)->GetBounds(b2);
		
		int split = mDirection == gui_Split_Horizontal ?
			(b1[2] + b2[0] + GetSplitSize()) / 2 :
			(b1[3] + b2[1] + GetSplitSize()) / 2;
		
		if (mDirection == gui_Split_Horizontal)
		{
			b1[0] = b[0];
			b1[2] = split;
			b2[0] = split + GetSplitSize();
			b2[2] = b[2];
			b1[1] = b2[1] = b[1];
			b1[3] = b2[3] = b[3];
		} else {
			b1[1] = b[1];
			b1[3] = split;
			b2[1] = split + GetSplitSize();
			b2[3] = b[3];
			b1[0] = b2[0] = b[0];
			b1[2] = b2[2] = b[2];
		}
		GetNthChild(0)->SetBounds(b1);
		GetNthChild(1)->SetBounds(b2);		
	}

}


void		GUI_Splitter::AlignContentsAt(int split)
{
	if (CountChildren() > 1)
	{
		int b[4], b1[4], b2[4];
		GetBounds(b);
		GetNthChild(0)->GetBounds(b1);
		GetNthChild(1)->GetBounds(b2);
		
		if (mDirection == gui_Split_Horizontal)
		{
			b1[0] = b[0];
			b1[2] = split;
			b2[0] = split + GetSplitSize();
			b2[2] = b[2];
			b1[1] = b2[1] = b[1];
			b1[3] = b2[3] = b[3];
		} else {
			b1[1] = b[1];
			b1[3] = split;
			b2[1] = split + GetSplitSize();
			b2[3] = b[3];
			b1[0] = b2[0] = b[0];
			b1[2] = b2[2] = b[2];
		}
		GetNthChild(0)->SetBounds(b1);
		GetNthChild(1)->SetBounds(b2);		
	}

}

int		GUI_Splitter::GetSplitSize(void)
{
		GUI_TexPosition_t	metrics;

	if (mDirection == gui_Split_Vertical)
	{
		GUI_GetTextureResource("splitter_h.png", 0, &metrics);
		return metrics.real_height / 2;
	} else {
		GUI_GetTextureResource("splitter_v.png", 0, &metrics);
		return metrics.real_width / 2;
	}
}
