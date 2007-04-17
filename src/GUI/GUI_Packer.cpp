#include "GUI_Packer.h"

GUI_Packer::GUI_Packer()
{
	GetBounds(mPackArea);
}

GUI_Packer::~GUI_Packer()
{
}

void		GUI_Packer::PackPane(GUI_Pane * child, GUI_Packer_Side side)
{
	int subsize[6];
	child->GetBounds(subsize);
	subsize[4] = subsize[2] - subsize[0];
	subsize[5] = subsize[3] - subsize[1];
	
	switch(side) {
	case gui_Pack_Left:
		subsize[0] = mPackArea[0];
		subsize[1] = mPackArea[1];
		subsize[2] = subsize[0] + subsize[4];
		subsize[3] = mPackArea[3];
		mPackArea[0] += subsize[4];
		break;
	case gui_Pack_Right:
		subsize[2] = mPackArea[2];
		subsize[1] = mPackArea[1];
		subsize[0] = subsize[2] - subsize[4];
		subsize[3] = mPackArea[3];
		mPackArea[2] -= subsize[4];
		break;
	case gui_Pack_Bottom:
		subsize[0] = mPackArea[0];
		subsize[1] = mPackArea[1];
		subsize[2] = mPackArea[2];
		subsize[3] = subsize[1] + subsize[5];
		mPackArea[1] += subsize[5];
		break;	
	case gui_Pack_Top:
		subsize[0] = mPackArea[0];
		subsize[3] = mPackArea[3];
		subsize[2] = mPackArea[2];
		subsize[1] = subsize[3] - subsize[5];
		mPackArea[3] -= subsize[5];
		break;	
	case gui_Pack_Center:
		subsize[0] = mPackArea[0];
		subsize[1] = mPackArea[1];
		subsize[2] = mPackArea[2];
		subsize[3] = mPackArea[3];
		break;
	}
	child->SetBounds(subsize);
}

void		GUI_Packer::SetBounds(int x1, int y1, int x2, int y2)
{
	GUI_Pane::SetBounds(x1,y1,x2,y2);
	mPackArea[0] = x1;
	mPackArea[1] = y1;
	mPackArea[2] = x2;
	mPackArea[3] = y2;
}

void		GUI_Packer::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);
	mPackArea[0] = inBounds[0];
	mPackArea[1] = inBounds[1];
	mPackArea[2] = inBounds[2];
	mPackArea[3] = inBounds[3];
}

