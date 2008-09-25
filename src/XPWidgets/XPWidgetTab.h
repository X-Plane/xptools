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
#ifndef XPWIDGET_TAB_H
#define XPWIDGET_TAB_H

#include "XPWidgetDefs.h"

/************************************************************************
 * TABS WIDGET
 ************************************************************************/

#define xpWidgetClass_Tab		10

enum {
	xpProperty_CurrentTab = 1900
};

enum {
	xpMsg_TabChanged = 1900
};

XPWidgetID	XPCreateTab(
					int                  inLeft,
					int                  inTop,
					int                  inRight,
					int                  inBottom,
					int                  inVisible,
					const char *         inDescriptor,
					XPWidgetID           inContainer);

int		XPTabProc(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2);


#endif /* XPWIDGET_TAB_H */
