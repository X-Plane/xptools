/*
 * Copyright (c) 2004, Laminar Research.
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
#include "RF_Pane.h"
#include "XPWidgets.h"
#include "XPWidgetUtils.h"
WED_Pane::WED_Pane(
               int                  inLeft,
               int                  inTop,
               int                  inRight,
               int                  inBottom,
               int                  inVisible,
               const char *         inDescriptor,
               WED_Pane *			inSuper)
{
	XPWidgetID	super = NULL;
	if (inSuper)
		super = inSuper->GetWidget();
	mWidget = XPCreateCustomWidget(inLeft, inTop, inRight, inBottom,
			inVisible, inDescriptor, super == NULL, super, StaticMessageFunc);
	XPSetWidgetProperty(mWidget, xpProperty_Object, reinterpret_cast<long>(this));
}

WED_Pane::WED_Pane(
                                   int                  inLeft,
                                   int                  inTop,
                                   int                  inRight,
                                   int                  inBottom,
                                   int                  inVisible,
                                   const char *         inDescriptor,
                                   WED_Pane *			inSuper,
                                   XPWidgetClass		inClass)
{
	XPWidgetID	super = NULL;
	if (inSuper)
		super = inSuper->GetWidget();
	mWidget = XPCreateWidget(inLeft, inTop, inRight, inBottom,
			inVisible, inDescriptor, super == NULL, super, inClass);
	XPAddWidgetCallback(mWidget, StaticMessageFunc);
	XPSetWidgetProperty(mWidget, xpProperty_Object, reinterpret_cast<long>(this));
}

WED_Pane::WED_Pane(
                                   int                  inLeft,
                                   int                  inTop,
                                   int                  inRight,
                                   int                  inBottom,
                                   int                  inVisible,
                                   const char *         inDescriptor,
                                   WED_Pane *			inSuper,
                                   XPWidgetFunc_t		inFunc)
{
	XPWidgetID	super = NULL;
	if (inSuper)
		super = inSuper->GetWidget();
	mWidget = XPCreateCustomWidget(inLeft, inTop, inRight, inBottom,
			inVisible, inDescriptor, super == NULL, super, inFunc);
	XPAddWidgetCallback(mWidget, StaticMessageFunc);
	XPSetWidgetProperty(mWidget, xpProperty_Object, reinterpret_cast<long>(this));
}



WED_Pane::~WED_Pane()
{
}

WED_Pane *	WED_Pane::GetPaneObj(XPWidgetID inWidget)
{
	return reinterpret_cast<WED_Pane *>(XPGetWidgetProperty(inWidget, xpProperty_Object, NULL));
}

XPWidgetID	WED_Pane::GetWidget(void)
{
	return mWidget;
}

void	WED_Pane::Kill(bool inRecursive)
{
	XPDestroyWidget(mWidget,inRecursive);
}

void	WED_Pane::Show(bool inVisible)
{
	if (inVisible)
		XPShowWidget(mWidget);
	else
		XPHideWidget(mWidget);
}

bool	WED_Pane::IsVisible(void) const
{
	return XPIsWidgetVisible(mWidget);
}


void	WED_Pane::DrawSelf(void)
{
}

int		WED_Pane::HandleClick(XPLMMouseStatus status, int x, int y, int button)
{
	return 0;
}

int		WED_Pane::HandleKey(char key, XPLMKeyFlags flags, char vkey)
{
	return 0;
}

int		WED_Pane::HandleMouseWheel(int x, int y, int direction)
{
	return 0;
}

int		WED_Pane::MessageFunc(
                                   XPWidgetMessage      inMessage,
                                   long                 inParam1,
                                   long                 inParam2)
{
	switch(inMessage) {
	case xpMsg_Destroy:
		delete this;
		return 1;
	case xpMsg_Draw:
		DrawSelf();
		return 0;
	case xpMsg_KeyPress:
		return HandleKey(KEY_CHAR(inParam1), KEY_FLAGS(inParam1), KEY_VKEY(inParam1));
	case xpMsg_KeyTakeFocus:
		return 1;
	case xpMsg_KeyLoseFocus:
		return 1;
	case xpMsg_MouseDown:
		return HandleClick(xplm_MouseDown, MOUSE_X(inParam1), MOUSE_Y(inParam1), MOUSE_BUTTON(inParam1));
	case xpMsg_MouseDrag:
		return HandleClick(xplm_MouseDrag, MOUSE_X(inParam1), MOUSE_Y(inParam1),MOUSE_BUTTON(inParam1));
	case xpMsg_MouseUp:
		return HandleClick(xplm_MouseUp, MOUSE_X(inParam1), MOUSE_Y(inParam1), MOUSE_BUTTON(inParam1));
	case xpMsg_MouseWheel:
		return HandleMouseWheel(MOUSE_X(inParam1), MOUSE_Y(inParam1), MOUSE_BUTTON(inParam1));
	default:
		return 0;
	}
}

int		WED_Pane::StaticMessageFunc(
                                   XPWidgetMessage      inMessage,
                                   XPWidgetID           inWidget,
                                   long                 inParam1,
                                   long                 inParam2)
{
	WED_Pane * me = reinterpret_cast<WED_Pane *>(XPGetWidgetProperty(inWidget, xpProperty_Object, NULL));
	if (me)
		return me->MessageFunc(inMessage, inParam1, inParam2);
	return 0;
}

