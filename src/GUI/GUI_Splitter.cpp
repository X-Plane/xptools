#include "GUI_Splitter.h"
#include "GUI_GraphState.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif

const int	gui_SplitSize = 5;

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
		
		state->SetState(false, 0, false, false, false, false, false);
		
		glColor3f(0,mClick ? 1.0 : 0.5,1.0);
		glBegin(GL_QUADS);
		
		if (mDirection == gui_Split_Horizontal)
		{
			glVertex2i(b1[2],b[1]);
			glVertex2i(b1[2],b[3]);
			glVertex2i(b2[0],b[3]);
			glVertex2i(b2[0],b[1]);
		} else {
			glVertex2i(b[0],b1[3]);
			glVertex2i(b[0],b2[1]);
			glVertex2i(b[2],b2[1]);
			glVertex2i(b[2],b1[3]);
		}
		glEnd();
		
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
			b2[0] = x - mSlop + gui_SplitSize;
			b1[1] = b2[1] = b[1];
			b1[3] = b2[3] = b[3];
		} else {
			b1[3] = y - mSlop;
			b2[1] = y - mSlop + gui_SplitSize;
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
			(b1[3] + b2[1] + gui_SplitSize) / 2 :
			(b1[2] + b2[0] + gui_SplitSize) / 2;
		
		if (mDirection == gui_Split_Horizontal)
		{
			b1[0] = b[0];
			b1[2] = split;
			b2[0] = split + gui_SplitSize;
			b2[2] = b[2];
			b1[1] = b2[1] = b[1];
			b1[3] = b2[3] = b[3];
		} else {
			b1[1] = b[1];
			b1[3] = split;
			b2[1] = split + gui_SplitSize;
			b2[3] = b[3];
			b1[0] = b2[0] = b[0];
			b1[2] = b2[2] = b[2];
		}
		GetNthChild(0)->SetBounds(b1);
		GetNthChild(1)->SetBounds(b2);		
	}

}

