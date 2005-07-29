/* 
 * Copyright (c) 2004, Ben Supnik and Sandy Barbour.
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
#include "XPWidgets.h"
#include "XPWidgetUtils.h"
#include "XPWidgetDefs.h"
#include "XPLMDisplay.h"
#include "XPLMUtilities.h"
#include <stdio.h>

void	XPUCreateWidgets(
					const XPWidgetCreate_t *	inWidgetDefs,
					long						inCount,
					XPWidgetID					inParamParent,
					XPWidgetID *				ioWidgets)
{
	for (long n = 0; n < inCount; ++n)
	{
		XPWidgetID rt = NULL;
		int ind = inWidgetDefs[n].containerIndex;
		if (ind == PARAM_PARENT)
			rt = inParamParent;
		if ((ind >= 0) && (ind < inCount))
			rt = ioWidgets[n];
		
		ioWidgets[n] = XPCreateWidget(
			inWidgetDefs[n].left,
			inWidgetDefs[n].right,
			inWidgetDefs[n].top,
			inWidgetDefs[n].bottom,
			inWidgetDefs[n].visible,
			inWidgetDefs[n].descriptor,
			inWidgetDefs[n].isRoot,
			rt,
			inWidgetDefs[n].widgetClass);
	}
}					

void	XPUMoveWidgetBy(
					XPWidgetID			inWidget,
					int					inDeltaX,
					int					inDeltaY)
{
	int	l, t, r, b;
	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);
	l += inDeltaX;
	r += inDeltaX;
	t += inDeltaY;
	b += inDeltaY;
	XPSetWidgetGeometry(inWidget, l, t, r, b);
}					

int XPUFixedLayout(		XPWidgetMessage			inMessage,
						XPWidgetID				inWidget,
						long					inParam1,
						long					inParam2)
{
	// Remember: since this message goes up, a child might be moving.  If so,
	// param 1 won't match inWidget.  If a child is moving, WE DON'T CARE!
	if ((inMessage == xpMsg_Reshape) && (inWidget == (XPWidgetID) inParam1))
	{
		if ((DELTA_X(inParam2) != 0) || (DELTA_Y(inParam2) != 0))
		{
			long children = XPCountChildWidgets(inWidget);
			for (long index = 0; index < children; ++index)
			{
				XPWidgetID child = XPGetNthChildWidget(inWidget, index);	
				XPUMoveWidgetBy(child, DELTA_X(inParam2), DELTA_Y(inParam2));
			}
		}
		return 1;
	}
	return 0;
}						

int XPUSelectIfNeeded(	XPWidgetMessage			inMessage,
						XPWidgetID				inWidget,
						long					inParam1,
						long					inParam2,
						int						inEatClick)
{
	static	bool	eating = false;
	if (inMessage == xpMsg_MouseDown)
		if (!XPIsWidgetInFront(inWidget))
		{			
			XPBringRootWidgetToFront(inWidget);
			if (inEatClick)
				eating = true;
			return inEatClick;
		}
	if (eating && inEatClick && (inMessage == xpMsg_MouseDrag))
		return 1;

	if (eating && inEatClick && (inMessage == xpMsg_MouseUp))
	{
		eating = false;
		return 1;
	}
		
	return 0;
}

int XPUDefocusKeyboard(	XPWidgetMessage			inMessage,
						XPWidgetID				inWidget,
						long					inParam1,
						long					inParam2,
						int						inEatClick)
{
	static	int	callingSelf = 0;
	if (inMessage == xpMsg_MouseDown)
	{
		callingSelf = 1;
		XPSetKeyboardFocus(inWidget);
		callingSelf = 0;
		XPLoseKeyboardFocus(inWidget);
		return inEatClick;
	}
		
	if (inMessage == xpMsg_KeyTakeFocus && callingSelf)
		return 1;
	
	return 0;
}
		
						
int XPUDragWidget(		XPWidgetMessage			inMessage,
						XPWidgetID				inWidget,
						long					inParam1,
						long					inParam2,
						int						inLeft,
						int						inTop,
						int						inRight,
						int						inBottom)
{
	int l, t, r, b;
	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);
	int ml, mt, mr, mb;
	
	XPLMGetScreenSize(&mr, &mt);
	ml = mb = 0;
//	mt -= 20; 	// Menu bar hack!	// <- BAS - this is not needed when not running in x-plane!!
	
	if ((inMessage == xpMsg_MouseDown) || (inMessage == xpMsg_MouseDrag) || (inMessage == xpMsg_MouseUp))
	{
		int x = MOUSE_X(inParam1);
		int y = MOUSE_Y(inParam1);
		
		if (x < ml) x = ml;
		if (x > mr) x = mr;
		if (y < mb) y = mb;
		if (y > mt) y = mt;
		
		switch(inMessage) {
		case	 xpMsg_MouseDown:
			{
				if (IN_RECT(x, y, inLeft, inTop, inRight, inBottom))
				{
					XPSetWidgetProperty(inWidget, xpProperty_Dragging, 1);
					XPSetWidgetProperty(inWidget, xpProperty_DragXOff, x - l);
					XPSetWidgetProperty(inWidget, xpProperty_DragYOff, y - b);
					return 1;
				}
			}
			return 0;
		case xpMsg_MouseDrag:
			{
				if (XPGetWidgetProperty(inWidget, xpProperty_Dragging, NULL))
				{
					int w = r - l;
					int	h = t - b;
					l = x - XPGetWidgetProperty(inWidget, xpProperty_DragXOff, NULL);
					b = y - XPGetWidgetProperty(inWidget, xpProperty_DragYOff, NULL);
					r = l + w;
					t = b + h;
					XPSetWidgetGeometry(inWidget, l, t, r, b);
					return 1;
				}
			}
			return 0;
		case xpMsg_MouseUp:
			{
				if (XPGetWidgetProperty(inWidget, xpProperty_Dragging, NULL))
				{
					int w = r - l;
					int	h = t - b;
					l = x - XPGetWidgetProperty(inWidget, xpProperty_DragXOff, NULL);
					b = y - XPGetWidgetProperty(inWidget, xpProperty_DragYOff, NULL);
					r = l + w;
					t = b + h;
					XPSetWidgetGeometry(inWidget, l, t, r, b);
					XPSetWidgetProperty(inWidget, xpProperty_Dragging, 0);
					return 1;
				}
			}
			return 0;
		default:
			return 0;
		}
	} else
		return 0;	// not a message we care about.
}