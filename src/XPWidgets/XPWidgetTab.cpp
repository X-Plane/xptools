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
#include "XPWidgetTab.h"
#include "XPWidgets.h"
#include "XPWidgetUtils.h"
#include "XPUIGraphics.h"
#include "XPUIGraphicsPrivate.h"
#include "XPLMGraphics.h"

#define 	SHOW_TAB_BORDER 0

XPWidgetID	XPCreateTab(
					int                  inLeft,
					int                  inTop,
					int                  inRight,
					int                  inBottom,
					int                  inVisible,
					const char *         inDescriptor,
					XPWidgetID           inContainer)
{
	return XPCreateCustomWidget(
                                   inLeft,
                                   inTop,
                                   inRight,
                                   inBottom,
                                   inVisible,
                                   inDescriptor,
                                   0,
                                   inContainer,
                                   XPTabProc);
}

#define TAB_VERT_ADJUST 45
#define TAB_WIDTH 86
#define TAB_MARGIN 14
#define TAB_VERT_START 24
#define TAB_BAR_HEIGHT 26
#define TAB_HEIGHT 17

int		XPTabProc(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2)
{
	if (XPUFixedLayout(inMessage, inWidget, inParam1, inParam2)) return 1;

	char	buf[1024];
	char *	strp, * endp;
	int		item_count;
	int		cur_item, index;
	int		x_off;
	bool	done;
	float	text[3];
	int		left, top, right, bottom;
	switch(inMessage) {
	case xpMsg_Create:
	case xpMsg_DescriptorChanged:
	case xpMsg_PropertyChanged:
		{
			cur_item = XPGetWidgetProperty(inWidget, xpProperty_CurrentTab, NULL);
			// We must clamp our item number to the numer of tabs
			XPGetWidgetDescriptor(inWidget, buf, sizeof(buf));
			item_count = 1;
			for (strp = buf; *strp; ++strp)
				if (*strp == ';') ++item_count;
			if (cur_item >= item_count)
				cur_item = item_count - 1;
			if (cur_item < 0) cur_item = 0;
			if (cur_item != XPGetWidgetProperty(inWidget, xpProperty_CurrentTab, NULL))
				XPSetWidgetProperty(inWidget, xpProperty_CurrentTab, cur_item);
		}
		return 1;
	case xpMsg_Draw:
		{
			XPGetWidgetGeometry(inWidget, &left, &top, &right, &bottom);
			XPGetWidgetDescriptor(inWidget, buf, sizeof(buf));
			cur_item = XPGetWidgetProperty(inWidget, xpProperty_CurrentTab, NULL);

			XPDrawElement(left, top-TAB_VERT_ADJUST, right, top-TAB_VERT_ADJUST + TAB_BAR_HEIGHT, xpElement_PopupBar, 0);

			x_off = left + TAB_MARGIN;
			strp = buf;
			index = 0;
			while (*strp)
			{
				XPDrawElement(x_off, top-TAB_VERT_ADJUST + TAB_VERT_START, x_off + TAB_WIDTH,
							top-TAB_VERT_ADJUST + TAB_VERT_START + TAB_HEIGHT,
							(index == cur_item) ? xpElement_PopupButtonHilite : xpElement_PopupButton, 0);

				endp = strp;
				while (*endp != 0 && *endp != ';') ++endp;
				done = (*endp == 0);
				if (*endp == ';')
					*endp = 0;

				SetupAmbientColor((index == cur_item) ? xpColor_TabFront : xpColor_TabBack, text);
				XPLMDrawString(text, x_off + 7, top-TAB_VERT_ADJUST + TAB_VERT_START + 4, strp, NULL, xplmFont_Basic);
				strp = endp;
				if (!done)
					++strp;

				++index;
				x_off += (TAB_MARGIN + TAB_WIDTH);
			}
#if SHOW_TAB_BORDER
			XPLMSetGraphicsState(0, 0, 0,   0, 0,   0, 0);
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP);
			glVertex2i(left, bottom);
			glVertex2i(left, top);
			glVertex2i(right, top);
			glVertex2i(right, bottom);
			glEnd();
#endif
		}
		return 1;
	case xpMsg_MouseDown:
		{
			XPGetWidgetGeometry(inWidget, &left, &top, &right, &bottom);
			XPGetWidgetDescriptor(inWidget, buf, sizeof(buf));
			cur_item = XPGetWidgetProperty(inWidget, xpProperty_CurrentTab, 0);
			x_off = left + TAB_MARGIN;
			item_count = 1;
			for (strp = buf; *strp; ++strp)
				if (*strp == ';') ++item_count;
			index = 0;
			while (index < item_count)
			{
				if (IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1),
							x_off, top-TAB_VERT_ADJUST + TAB_VERT_START + TAB_HEIGHT, x_off + TAB_WIDTH, top-TAB_VERT_ADJUST + TAB_VERT_START))
				{
					if (index != cur_item)
					{
						XPSetWidgetProperty(inWidget, xpProperty_CurrentTab, index);
						XPSendMessageToWidget(inWidget, xpMsg_TabChanged, xpMode_UpChain, (long) inWidget, index);
					}
					return 1;
				}

				++index;
				x_off += (TAB_MARGIN + TAB_WIDTH);
			}
		}
		return 1;
	default:
		return 0;
	}
}
