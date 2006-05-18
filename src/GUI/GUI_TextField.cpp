#include "GUI_TextField.h"
#include "GUI_Messages.h"
#include "GUI_Fonts.h"
#include "GUI_GraphState.h"

GUI_TextField::GUI_TextField(int scrollH, GUI_Commander * parent) :
	GUI_Commander(parent),
	mState(NULL),
	mScrollH(scrollH)
	
{
	mLogicalBounds[0] = 0;
	mLogicalBounds[1] = 0;
	mLogicalBounds[2] = 100;
	mLogicalBounds[3] = 100;
}

GUI_TextField::~GUI_TextField()
{
}

void		GUI_TextField::SetWidth(float width)
{
	mLogicalBounds[2] = mLogicalBounds[0] + width;
	Repaginate();
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
	Refresh();	
}


void		GUI_TextField::Draw(GUI_GraphState * state)
{
	mState = state;
	OGLE::Draw();
	mState = NULL;
}

int			GUI_TextField::MouseDown(int x, int y, int button)
{
	if (!IsFocused())
		TakeFocus();
	Click(x,y,false);
	Refresh();
	return 1;
}

void		GUI_TextField::MouseDrag(int x, int y, int button)
{
	Drag(x,y);
	Refresh();
}

void		GUI_TextField::MouseUp(int x, int y, int button)
{
}

int			GUI_TextField::ScrollWheel(int x, int y, int dist, int axis)
{
	this->ScrollV(dist);
	return 1;
}

void		GUI_TextField::SetBounds(int x1, int y1, int x2, int y2)
{
	GUI_Pane::SetBounds(x1,y1,x2,y2);
	if (!mScrollH)
	{
		mLogicalBounds[0] = x1;
		mLogicalBounds[2] = x2;
		Repaginate();
		Refresh();
	}
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
}

void		GUI_TextField::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);
	if (!mScrollH)
	{
		mLogicalBounds[0] = inBounds[0];
		mLogicalBounds[2] = inBounds[2];
		Repaginate();
		Refresh();
	}
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
}

int			GUI_TextField::KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)
{
	Key(inKey, inFlags & gui_ShiftFlag);
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
	Refresh();
	return 1;
}

int			GUI_TextField::HandleCommand(int command)
{
	return 0;
}

int			GUI_TextField::CanHandleCommand(int command, string& ioName, int& ioCheck)
{
	return 0;
}

int			GUI_TextField::AcceptTakeFocus(void)
{
	return 1;
}

int			GUI_TextField::AcceptLoseFocus(int inForce)
{
	return 1;
}

int			GUI_TextField::AcceptFocusChain(void)
{
	return 1;
}

void		GUI_TextField::GetScrollBounds(float outTotalBounds[4], float outVisibleBounds[4])
{
	int vbounds[4];
	outTotalBounds[0] = mLogicalBounds[0];
	outTotalBounds[1] = mLogicalBounds[1];
	outTotalBounds[2] = mLogicalBounds[2];
	outTotalBounds[3] = mLogicalBounds[3];
	GetBounds(vbounds);
	outVisibleBounds[0] = vbounds[0];
	outVisibleBounds[1] = vbounds[1];
	outVisibleBounds[2] = vbounds[2];
	outVisibleBounds[3] = vbounds[3];
}

void		GUI_TextField::ScrollV(float vOffset)
{
	int	vbounds[4];
	GetBounds(vbounds);
	mLogicalBounds[3] -= mLogicalBounds[1];
	mLogicalBounds[1] = vbounds[1] - vOffset;
	mLogicalBounds[3] += mLogicalBounds[1];
	Refresh();
}

void		GUI_TextField::ScrollH(float hOffset)
{
	int	vbounds[4];
	GetBounds(vbounds);
	mLogicalBounds[2] -= mLogicalBounds[0];
	mLogicalBounds[0] = vbounds[0] - hOffset;
	mLogicalBounds[2] += mLogicalBounds[0];
	Refresh();
}

void			GUI_TextField::GetVisibleBounds(
						float			bounds[4])
{
	int vbounds[4];
	GetBounds(vbounds);
	bounds[0] = vbounds[0];
	bounds[1] = vbounds[1];
	bounds[2] = vbounds[2];
	bounds[3] = vbounds[3];
}

void			GUI_TextField::GetLogicalBounds(
						float			bounds[4])
{
	bounds[0] = mLogicalBounds[0];
	bounds[1] = mLogicalBounds[1];
	bounds[2] = mLogicalBounds[2];
	bounds[3] = mLogicalBounds[3];
}

void			GUI_TextField::SetLogicalHeight(
						float 			height)
{
	mLogicalBounds[1] = mLogicalBounds[3] - height;
}

void			GUI_TextField::ScrollTo(
						float			where[2])
{
	int	vbounds[4];
	GetBounds(vbounds);
	mLogicalBounds[2] -= mLogicalBounds[0];
	mLogicalBounds[3] -= mLogicalBounds[1];
	mLogicalBounds[0] = vbounds[0] - where[0];
	mLogicalBounds[1] = vbounds[1] - where[1];
	mLogicalBounds[2] += mLogicalBounds[0];
	mLogicalBounds[3] += mLogicalBounds[1];
	BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED,0);
	Refresh();
}

void			GUI_TextField::GetText(
						const char **	start_p,
						const char **	end_p)
{
	*start_p = &*mText.begin();
	*end_p = &*mText.end();
}

void			GUI_TextField::ReplaceText(
						int				offset1,
						int				offset2,
						const char *	t1,
						const char *	t2)
{
	mText.replace(mText.begin()+offset1,mText.begin()+offset2,t1,t2);
}

float			GUI_TextField::GetLineHeight(void)
{
	return GUI_GetLineHeight(font_UI_Basic);
}

/*
float			GUI_TextField::GetBaseline(void)
{
	return GUI_GetLineDescent(font_UI_Basic);
}
*/

float			GUI_TextField::MeasureString(
						const char * 	tStart, 
						const char * 	tEnd)
{
	return GUI_MeasureString(font_UI_Basic, tStart, tEnd - tStart);
}

int				GUI_TextField::FitStringFwd(
						const char * 	tStart, 
						const char * 	tEnd, 
						float 			space)
{
	return GUI_FitForward(font_UI_Basic, tStart, tEnd, space);
}

int				GUI_TextField::FitStringRev(
						const char * 	tStart, 
						const char * 	tEnd, 
						float 			space)
{
	return GUI_FitReverse(font_UI_Basic, tStart, tEnd, space);

}

void			GUI_TextField::DrawString(
						const char *	tStart,
						const char *	tEnd,
						float			x,
						float			y)
{
	float c[4] = { 0,0,0,1 };
	int yy = y;
	GUI_FontDrawScaled(mState, font_UI_Basic, c,
						x, yy, x + 10000, yy + GUI_GetLineHeight(font_UI_Basic),
						tStart, tEnd, align_Left);
}

void			GUI_TextField::DrawSelection(	
								float			bounds[4])
{
	mState->SetState(false, 0, false, false, false, false, false);
	if (bounds[0] == bounds[2])
	{
		glColor3f(0,0,0);
		glBegin(GL_LINES);
		glVertex2f(bounds[0], bounds[1]);
		glVertex2f(bounds[0], bounds[3]);
		glEnd();
	}
	else
	{
		glColor3f(1, 1, 0);
		glBegin(GL_QUADS);
		glVertex2f(bounds[0], bounds[1]);
		glVertex2f(bounds[0], bounds[3]);
		glVertex2f(bounds[2], bounds[3]);
		glVertex2f(bounds[2], bounds[1]);
		glEnd();
	}
}


const char *	GUI_TextField::WordBreak(
						const char *	t1,
						const char *	t2)
{
	const char * p = t1;
	while (p != t2 && isspace(*p))
		++p;
	while (p != t2 && !isspace(*p))
		++p;
	return p;
}
							

