/*
 * Copyright (c) 2007, Laminar Research.
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

#include "GUI_Packer.h"
#include "GUI_GraphState.h"
#include "GUI_DrawUtils.h"

#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif


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

void		GUI_Packer::PackPaneToRight(GUI_Pane * child, GUI_Packer_Side side, GUI_Pane * target)
{
	int subsize[6];
	int targetsize[4];
	child->GetBounds(subsize);
	target->GetBounds(targetsize);
	subsize[4] = subsize[2] - subsize[0];
	subsize[5] = subsize[3] - subsize[1];

	switch(side) {
	case gui_Pack_Bottom:
		subsize[0] = targetsize[2];
		subsize[1] = targetsize[1];
		subsize[2] = targetsize[2] + subsize[4];
		subsize[3] = targetsize[1] + subsize[5];
		mPackArea[1] = max(subsize[3],targetsize[3]);
		break;
	case gui_Pack_Top:
		subsize[0] = targetsize[2];
		subsize[3] = targetsize[3];
		subsize[2] = targetsize[2] + subsize[4];
		subsize[1] = targetsize[3] - subsize[5];
		mPackArea[3] = min(targetsize[1], subsize[1]);
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

void		GUI_Packer::Draw(GUI_GraphState * state)
{
		int tile[4] = { 0, 0, 1, 1 };

		glColor3f(1,1,1);

	if (!mImage.empty())
	{
		int		bounds[4];
		GetBounds(bounds);
		GUI_DrawStretched(state, mImage.c_str(), bounds, tile);
	}
}

void		GUI_Packer::SetBkgkndImage(const char * image_res)
{
	mImage = image_res;
}
