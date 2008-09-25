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

#include "GUI_SimpleScroller.h"
#include "GUI_Messages.h"

GUI_SimpleScroller::GUI_SimpleScroller()
{
}

GUI_SimpleScroller::~GUI_SimpleScroller()
{
}

void	GUI_SimpleScroller::GetScrollBounds(float outTotalBounds[4], float outVisibleBounds[4])
{
	int me[4], child[4];
	this->GetBounds(me);
	if (this->CountChildren() < 1)
	{
		outTotalBounds[0] = outVisibleBounds[0] = 0;
		outTotalBounds[1] = outVisibleBounds[1] = 0;
		outTotalBounds[2] = outVisibleBounds[2] = me[2] - me[0];
		outTotalBounds[3] = outVisibleBounds[3] = me[3] - me[1];
		return;
	}

	this->GetNthChild(0)->GetBounds(child);

	outTotalBounds[0] = 0;	// child[0] - child[0]
	outTotalBounds[1] = 0;	// child[1] - child[1]
	outTotalBounds[2] = child[2] - child[0];
	outTotalBounds[3] = child[3] - child[1];

	outVisibleBounds[0] = me[0] - child[0];
	outVisibleBounds[1] = me[1] - child[1];
	outVisibleBounds[2] = me[2] - child[0];
	outVisibleBounds[3] = me[3] - child[1];
}

void	GUI_SimpleScroller::ScrollH(float xOffset)
{
	if (this->CountChildren() < 1) return;
	int me[4], child[4];
	this->GetBounds(me);
	this->GetNthChild(0)->GetBounds(child);

	child[2] -= child[0];
	child[0] = me[0] - xOffset;
	child[2] += child[0];

	this->GetNthChild(0)->SetBounds(child);
	this->Refresh();
}

void	GUI_SimpleScroller::ScrollV(float yOffset)
{
	if (this->CountChildren() < 1) return;
	int me[4], child[4];
	this->GetBounds(me);
	this->GetNthChild(0)->GetBounds(child);

	child[3] -= child[1];
	child[1] = me[1] - yOffset;
	child[3] += child[1];

	this->GetNthChild(0)->SetBounds(child);
	this->Refresh();
}

void	GUI_SimpleScroller::SetBounds(int x1, int y1, int x2, int y2)
{
	GUI_Pane::SetBounds(x1, y1, x2, y2);
	AlignContents();
}

void	GUI_SimpleScroller::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);
	AlignContents();
}

void	GUI_SimpleScroller::AlignContents()
{
	if (this->CountChildren() < 1) return;
	int me[4], child[4];
	this->GetBounds(me);
	this->GetNthChild(0)->GetBounds(child);
	int change = 0;

	if (child[2] < me[2])
	{
		change = 1;
		child[0] -= (child[2] - me[2]);
		child[2] -= (child[2] - me[2]);
	}

	if (child[1] > me[1])
	{
		change = 1;
		child[3] -= (child[1] - me[1]);
		child[1] -= (child[1] - me[1]);
	}

	if (child[0] > me[0])
	{
		change = 1;
		child[2] -= (child[0] - me[0]);
		child[0] -= (child[0] - me[0]);
	}

	if (child[3] < me[3])
	{
		change = 1;
		child[1] -= (child[3] - me[3]);
		child[3] -= (child[3] - me[3]);
	}

	if (change)
	{
		this->GetNthChild(0)->SetBounds(child);
		BroadcastMessage(GUI_SCROLL_CONTENT_SIZE_CHANGED, 0);
	}
}

void	GUI_SimpleScroller::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam)
{
	if (inMsg == GUI_SCROLL_CONTENT_SIZE_CHANGED)
		BroadcastMessage(inMsg, inParam);
}
