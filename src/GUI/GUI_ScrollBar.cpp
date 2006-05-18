#include "GUI_ScrollBar.h"
#include "GUI_GraphState.h"

enum {
	sb_PartNone,
	sb_PartDownButton,
	sb_PartDownPage,
	sb_PartThumb,
	sb_PartUpPage,
	sb_PartUpButton
};

int SB_BuildMetrix(
				float M1, float M2, float MD,	// Major axis - min, max, span
				float m1, float m2, float md,	// Minor axis - min, max, span
				float vnow,						// Actual scroll bar values
				float vmin, float vmax, 
				float vpage,
				float& minbut,					// cut from min button's end to sb
				float& thumb1,					// bottom of thumb
				float& thumb2,					// top of th umb
				float& maxbut)					// cut from sb to max button's start
{
	float sbl = MD - 2.0 * md;		// real scroll bar len - buttons
	minbut = M1 + md;				// subtract square buttons from ends
	maxbut = M2 - md;	
	
	if (vmax <= vmin)
	{
		// special case - scroll bar is not scrollable
		return 0;
	}
	
	float tp = (vpage/(vpage+vmax-vmin));	// thumb percent - thumb covers what percent of track?
	if (tp > 1.0) tp = 1.0;			// clamp for sanity - apps shouldn't even let this happen!
	
	float tl = sbl * tp;			// thumb len is just percent of track
	
	float fp = sbl - tl;			// free play in scroll bar - how far it moves.
	
	float pp = (vnow - vmin) / (vmax - vmin);	// position now as percent
	
	float ts = fp * pp;				// thumb start now as dist up sb
	
	thumb1 = minbut + ts;			// start and end of thumb can now be calculated
	thumb2 = minbut + ts + tl;
	return 1;
}

GUI_ScrollBar::GUI_ScrollBar()
{
}

GUI_ScrollBar::~GUI_ScrollBar()
{
}

int			GUI_ScrollBar::MouseDown(int x, int y, int button)
{
	int b[4];
	GetBounds(b);
	float bf[6] = { b[0], b[1], b[2], b[3], b[2]-b[0],b[3]-b[1] };
	float vnow, vmin, vmax, vpage;
	float b1, b2, t1, t2;
	vnow = this->GetValue();
	vmin = this->GetMin();
	vmax = this->GetMax();
	vpage = this->GetPageSize();
	int	alive;
	float track_coord = (bf[4] > bf[5])	? x : y;

	if (bf[4] > bf[5])	
		alive = SB_BuildMetrix(bf[0], bf[2], bf[4],
					   bf[1], bf[3], bf[5],
					   vnow, vmin, vmax, vpage,
					   b1, t1, t2, b2);
	else		
		alive = SB_BuildMetrix(bf[1], bf[3], bf[5],
					   bf[0], bf[2], bf[4],
					   vnow, vmin, vmax, vpage,
					   b1, t1, t2, b2);
	
	if (!alive) return 0;
		
	if (track_coord < b1)
	{
		mClickPart = sb_PartDownButton;
		vnow = max(vmin, vnow-1);
		if (vnow != this->GetValue())
			this->SetValue(vnow);
	}
	else if (track_coord > b2)
	{
		mClickPart = sb_PartUpButton;
		vnow = min(vmax, vnow+1);
		if (vnow != this->GetValue())
			this->SetValue(vnow);
	}
	else if (track_coord < t1)
	{
		mClickPart = sb_PartDownPage;
		vnow = max(vmin, vnow-vpage);
		if (vnow != this->GetValue())
			this->SetValue(vnow);
	}
	else if (track_coord > t2)
	{
		mClickPart = sb_PartUpPage;
		vnow = min(vmax, vnow+vpage);
		if (vnow != this->GetValue())
			this->SetValue(vnow);
	}
	else 
	{
		mClickPart = sb_PartThumb;
		mSlop = track_coord - t1;
	}	
	mInPart = 1;
	return 1;
}

void		GUI_ScrollBar::MouseDrag(int x, int y, int button)
{
	int oip = mInPart;
	int b[4];
	GetBounds(b);
	float bf[6] = { b[0], b[1], b[2], b[3], b[2]-b[0],b[3]-b[1] };
	float vnow, vmin, vmax, vpage;
	float b1, b2, t1, t2;
	vnow = this->GetValue();
	vmin = this->GetMin();
	vmax = this->GetMax();
	vpage = this->GetPageSize();
	int alive;
	
	float track_coord = (bf[4] > bf[5])	? x : y;

	if (bf[4] > bf[5])	
		alive = SB_BuildMetrix(bf[0], bf[2], bf[4],
					   bf[1], bf[3], bf[5],
					   vnow, vmin, vmax, vpage,
					   b1, t1, t2, b2);
	else		
		alive = SB_BuildMetrix(bf[1], bf[3], bf[5],
					   bf[0], bf[2], bf[4],
					   vnow, vmin, vmax, vpage,
					   b1, t1, t2, b2);

	mInPart = 0;
	if (!alive) goto done;

	// If out of box and no thumb, bail
	if (mClickPart != sb_PartThumb &&
		(x < b[0] || y < b[1] || x > b[2] || y > b[3]))	goto done;

	if (track_coord < b1 && mClickPart == sb_PartDownButton)
	{
		mInPart = 1;
		vnow = max(vmin, vnow-1);
		if (vnow != this->GetValue())
			this->SetValue(vnow);
	}
	if (track_coord > b1 && track_coord < t1 && mClickPart == sb_PartDownPage)
	{
		mInPart = 1;
		vnow = max(vmin, vnow-vpage);
		if (vnow != this->GetValue())
			this->SetValue(vnow);
	}
	if (track_coord > t2 && track_coord < b2 && mClickPart == sb_PartUpPage)
	{
		mInPart = 1;
		vnow = min(vmax, vnow+vpage);
		if (vnow != this->GetValue())
			this->SetValue(vnow);
	}
	if (track_coord > b2 && mClickPart == sb_PartUpButton)
	{
		mInPart = 1;
		vnow = min(vmax, vnow+1);
		if (vnow != this->GetValue())
			this->SetValue(vnow);
	}
	if (mClickPart == sb_PartThumb)
	{
		mInPart = 1;
		vnow = (track_coord - mSlop - b1) * (vmax - vmin) / ((b2 - b1) - (t2 - t1)) + vmin;
		vnow = min(vnow, vmax);
		vnow = max(vnow, vmin);
		if (vnow != this->GetValue())
			this->SetValue(vnow);
	}
done:
	if (mInPart != oip)	Refresh();
}

void		GUI_ScrollBar::MouseUp(int x, int y, int button)
{
	mInPart = 0;
	mClickPart = sb_PartNone;
	Refresh();
}

void		GUI_ScrollBar::Draw(GUI_GraphState * state)
{
	int b[4];
	GetBounds(b);
	float bf[6] = { b[0], b[1], b[2], b[3], b[2]-b[0],b[3]-b[1] };
	

	float vnow, vmin, vmax, vpage;
	float b1, b2, t1, t2;
	state->SetState(false, 0, false,   false, false, 	false, false);
	glColor3f(0.2,0.2,0.2);
	glBegin(GL_QUADS);

	glVertex2f(bf[0], bf[1]);
	glVertex2f(bf[0], bf[3]);
	glVertex2f(bf[2], bf[3]);
	glVertex2f(bf[2], bf[1]);
	
	vnow = this->GetValue();
	vmin = this->GetMin();
	vmax = this->GetMax();
	vpage = this->GetPageSize();
	int alive;
	
	if (bf[4] > bf[5])	
	{
		// Horizontal scrollbar
		alive = SB_BuildMetrix(bf[0], bf[2], bf[4],
					   bf[1], bf[3], bf[5],
					   vnow, vmin, vmax, vpage,
					   b1, t1, t2, b2);

		// down but
		glColor3f((mInPart && mClickPart == sb_PartDownButton) ? 1.0 : 0.5, 0.0, 0.0);		
		glVertex2f(bf[0], bf[1]);
		glVertex2f(bf[0], bf[3]);
		glVertex2f(b1, bf[3]);
		glVertex2f(b1, bf[1]);

		if (alive)
		{
			// thumb
			glColor3f(0.0, (mInPart && mClickPart == sb_PartThumb) ? 1.0 : 0.5, 0.0);				
			glVertex2f(t1, bf[1]);
			glVertex2f(t1, bf[3]);
			glVertex2f(t2, bf[3]);
			glVertex2f(t2, bf[1]);
		}
				
		// up button
		glColor3f((mInPart && mClickPart == sb_PartUpButton) ? 1.0 : 0.5, 0.0, 0.0);		
		glVertex2f(b2, bf[1]);
		glVertex2f(b2, bf[3]);
		glVertex2f(bf[2], bf[3]);
		glVertex2f(bf[2], bf[1]);
	} 
	else 
	{
		// Vertical scrollbar
		alive = SB_BuildMetrix(bf[1], bf[3], bf[5],
					   bf[0], bf[2], bf[4],
					   vnow, vmin, vmax, vpage,
					   b1, t1, t2, b2);
					   
		// down but
		glColor3f(mClickPart == sb_PartDownButton ? 1.0 : 0.5, 0.0, 0.0);		
		glVertex2f(bf[0], bf[1]);
		glVertex2f(bf[0], b1);
		glVertex2f(bf[2], b1);
		glVertex2f(bf[2], bf[1]);

		if (alive)
		{
			// thumb
			glColor3f(0.0, mClickPart == sb_PartThumb ? 1.0 : 0.5, 0.0);				
			glVertex2f(bf[0], t1);
			glVertex2f(bf[0], t2);
			glVertex2f(bf[2], t2);
			glVertex2f(bf[2], t1);
		}
				
		// up button
		glColor3f(mClickPart == sb_PartUpButton ? 1.0 : 0.5, 0.0, 0.0);		
		glVertex2f(bf[0], b2);
		glVertex2f(bf[0], bf[3]);
		glVertex2f(bf[2], bf[3]);
		glVertex2f(bf[2], b2);					   
	}
	
	glEnd();
}

void	GUI_ScrollBar::SetValue(float inValue)
{
	GUI_Control::SetValue(inValue);
	Refresh();
}

void	GUI_ScrollBar::SetMin(float inMin)
{
	GUI_Control::SetMin(inMin);
	Refresh();
}

void	GUI_ScrollBar::SetMax(float inMax)
{
	GUI_Control::SetMax(inMax);
	Refresh();
}

void	GUI_ScrollBar::SetPageSize(float inPageSize)
{
	GUI_Control::SetPageSize(inPageSize);
	Refresh();
}
