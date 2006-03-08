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
#include "XPStandardWidgets.h"
#include "XPStandardWidgetsPrivate.h"
#include "XPUIGraphicsPrivate.h"
#include "XPLMProcessing.h"
#include "XPLMDefs.h"
#include "XPWidgets.h"
#include "XPLMGraphics.h"
#include "XPLMUtilities.h"
#include "XPWidgetUtils.h"
#include <stdio.h>
#include <algorithm>
#include <string>
#include <gl.h>
#include "XPUIGraphics.h"
#include <math.h>


#if IBM
double round(double InValue)
{
    long WholeValue;
    double Fraction;

    WholeValue = InValue;
    Fraction = InValue - (double) WholeValue;

    if (Fraction >= 0.5)
        WholeValue++;

    return (double) WholeValue;
}
#endif

static int		XPMainWindow(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2);

static int		XPSubWindow(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2);

static int		XPButton(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2);

static int		XPTextField(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2);

static int		XPScrollBar(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2);

static int		XPCaption(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2);

static int		XPGeneralGraphics(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2);

static int		XPProgress(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2);

static	void	SetMetalColor(void);

/* THIS MUST BE IN SYNC WITH THE ENUMS IN WIDGET DEFS...THEY'RE ARRAY VALUES!!! */

XPWidgetFunc_t	gStandardWidgets[] = {
	0,
	XPMainWindow,
	XPSubWindow,
	XPButton,
	XPTextField,
	XPScrollBar,
	XPCaption,
	XPGeneralGraphics,
	XPProgress
};

int		XPMainWindow(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2)
{
	// First get our geometry and font size; we need this to know if our drag bar
	// is being dispatched.
	
		int fv;
		int 		l, t, r, b;
		int			dragBarHeight;
		int			cbh, cbv;
		
	XPGetElementDefaultDimensions(xpElement_WindowDragBar, NULL, &dragBarHeight, NULL);
	XPGetElementDefaultDimensions(xpElement_WindowCloseBox, &cbh, &cbv, NULL);
	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);
	XPLMGetFontDimensions(xplmFont_Basic, NULL, &fv, NULL);
	

	// Next dispatch built-in behavior: selection to front, dragging the title bar,
	// and keeping children lined up.
	
	if (XPUSelectIfNeeded(inMessage, inWidget, inParam1, inParam2, 0/*don't eat*/))	return 1;

	int	cbl_l, cbl_t, cbl_r, cbl_b;	// close box left and right rects
	int cbr_l, cbr_t, cbr_r, cbr_b;

	cbl_l = l+7;	cbl_b = t-dragBarHeight/2-cbv/2;	cbl_r = l+7+cbh;	cbl_t = t-dragBarHeight/2+cbv/2;
	cbr_l = r-7-cbh;cbr_b = t-dragBarHeight/2-cbv/2;	cbr_r = r-7;		cbr_t = t-dragBarHeight/2+cbv/2;
	switch(inMessage) {
	case xpMsg_Draw:
		{
				float		white [4];
				int			dragging = XPGetWidgetProperty(inWidget, xpProperty_Dragging, NULL);

			char	buf[100];
			XPGetWidgetDescriptor(inWidget, buf, 100);
			long	titleLen = XPLMMeasureString(buf,xplmFont_Basic, -1);

			if (XPGetWidgetProperty(inWidget, xpProperty_MainWindowType, NULL) == xpMainWindowStyle_Translucent)
			{
				XPLMDrawTranslucentDarkBox(l, t, r, b);
			} else {
				XPDrawWindow(l, b, r, t-dragBarHeight, xpWindow_MainWindow);
				XPDrawElement(l, t-dragBarHeight, r, t, xpElement_WindowDragBar, 0/*lit*/);
				if (titleLen > 0)
					XPDrawElement((l+r)/2 - (titleLen+16) / 2,t-dragBarHeight, (l+r)/2 + (titleLen+16) / 2, t,  xpElement_WindowDragBarSmooth,0);
					
				if (XPGetWidgetProperty(inWidget, xpProperty_MainWindowHasCloseBoxes, NULL))
				{
					int	hilited = XPGetWidgetProperty(inWidget, xpProperty_Hilited, NULL);
					XPDrawElement(cbl_l, cbl_b, cbl_r, cbl_t, hilited ? xpElement_WindowCloseBoxPressed : xpElement_WindowCloseBox, 0);
					XPDrawElement(cbr_l, cbr_b, cbr_r, cbr_t, hilited ? xpElement_WindowCloseBoxPressed : xpElement_WindowCloseBox, 0);
				}
			}

			SetupAmbientColor(xpColor_SubTitleText, white);
			SetProportional(1);
			XPLMDrawString(white, (l + r) / 2 - (titleLen / 2), t-(fv/2)-(dragBarHeight/2)+2,
						buf, NULL, xplmFont_Basic);
			SetProportional(0);			
		}
		return 1;	
	case xpMsg_MouseDown:
		if (IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), cbl_l, cbl_t, cbl_r, cbl_b) ||
			IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), cbr_l, cbr_t, cbr_r, cbr_b))
		{
			XPSetWidgetProperty(inWidget, xpProperty_Hilited, 1);
			return 1;
		}
		break;
	case xpMsg_MouseDrag:
		if (!XPGetWidgetProperty(inWidget, xpProperty_Dragging, 0))
		{
			if (IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), cbl_l, cbl_t, cbl_r, cbl_b) ||
				IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), cbr_l, cbr_t, cbr_r, cbr_b))
			{
				XPSetWidgetProperty(inWidget, xpProperty_Hilited, 1);
			} else 
				XPSetWidgetProperty(inWidget, xpProperty_Hilited, 0);
		}
		break;
	case xpMsg_MouseUp:
		if (!XPGetWidgetProperty(inWidget, xpProperty_Dragging, 0))
		{
			XPSetWidgetProperty(inWidget, xpProperty_Hilited, 0);
			if (IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), cbl_l, cbl_t, cbl_r, cbl_b) ||
				IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), cbr_l, cbr_t, cbr_r, cbr_b))
			{
				// Surprise kids!!!  We MUST do this last.  Why?  Well, sending the push button closed might
				// cause a widget to nuke itself; inWidget could be bogus from here on in.
				XPSendMessageToWidget(inWidget, xpMessage_CloseButtonPushed, xpMode_UpChain, (long) inWidget, 0);
				return 1;
			}
		}
		break;
	}
	
	if (XPUDragWidget(inMessage, inWidget, inParam1, inParam2, l, t, r, t - dragBarHeight))return 1;
	if (XPUDefocusKeyboard(inMessage, inWidget, inParam1, inParam2, 0/*don't eat*/)) return 1;
	if (XPUFixedLayout(inMessage, inWidget, inParam1, inParam2))					return 1;

	return 0;	
}	

int		XPSubWindow(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2)
{
	int 		l, t, r, b;

	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);

	switch(inMessage) {
	case xpMsg_Draw:
		{
			int	WindowType = XPGetWidgetProperty(inWidget, xpProperty_SubWindowType, NULL);
			int GraphicsWindowType;

			switch(WindowType)
			{
				case xpSubWindowStyle_SubWindow:
					GraphicsWindowType = xpWindow_SubWindow;
					break;
				case xpSubWindowStyle_Screen:
					GraphicsWindowType = xpWindow_Screen;
					break;
				case xpSubWindowStyle_ListView:
					GraphicsWindowType = xpWindow_ListView;
					break;
			}

			XPDrawWindow(l, b, r, t, GraphicsWindowType);
		}
		return 1;
	
	default:
		return 0;
	}
}	

int		XPButton(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2)
{
	// Select if we're in the background.
	if (XPUSelectIfNeeded(inMessage, inWidget, inParam1, inParam2, 1/*eat*/))	return 1;
	
	int fv;
	int l, t, r, b;
		
	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);
	XPLMGetFontDimensions(xplmFont_Basic, NULL, &fv, NULL);
	
	int	ButtonBehaviour = XPGetWidgetProperty(inWidget, xpProperty_ButtonBehavior, NULL);

	switch(inMessage) {
	case xpMsg_Create:
		XPSetWidgetProperty(inWidget, xpProperty_Hilited, 0);
		XPSetWidgetProperty(inWidget, xpProperty_ButtonState, 0);
		XPSetWidgetProperty(inWidget, xpProperty_Enabled, 1);
		return 1;
	case xpMsg_Draw:
		{
			float		white [4];
			float		gray [4];
			int			hilite = XPGetWidgetProperty(inWidget, xpProperty_Hilited, NULL);
			int			ButtonType = XPGetWidgetProperty(inWidget, xpProperty_ButtonType, NULL);
			int			GraphicsButtonType;
			int	ButtonState = XPGetWidgetProperty(inWidget, xpProperty_ButtonState, NULL);

			switch (ButtonType)
			{
				case xpPushButton:
					GraphicsButtonType = xpElement_PushButton;
					break;
				case xpRadioButton:
					GraphicsButtonType = xpElement_CheckBox;
					break;
				case xpWindowCloseBox:
					GraphicsButtonType = xpElement_WindowCloseBox;
					break;
				case xpLittleUpArrow:
					GraphicsButtonType = xpElement_LittleUpArrow;
					break;
				case xpLittleDownArrow:
					GraphicsButtonType = xpElement_LittleDownArrow;
					break;
			}

			if ((ButtonBehaviour == xpButtonBehaviorCheckBox) || (ButtonBehaviour == xpButtonBehaviorRadioButton))
				hilite = ButtonState;
			
			SetAlphaLevels(XPGetWidgetProperty(inWidget, xpProperty_Enabled, 0) ? 1.0 : 0.5);
			if (ButtonType == xpRadioButton)
			{
				int	w, h, cbl;
				XPGetElementDefaultDimensions(xpElement_CheckBox,
					&w, &h, &cbl);
				XPDrawElement(l, b, l+w, t, GraphicsButtonType, hilite);
			} else {
				XPDrawElement(l, b, r, t, GraphicsButtonType, hilite);
			}
			SetAlphaLevels(1.0);

			char	buf[100];
			long	titleLen = XPGetWidgetDescriptor(inWidget, buf, 100);

				SetupAmbientColor(xpColor_CaptionText, white);
				SetupAmbientColor(xpColor_MenuText, gray);
			
			SetProportional(1);			
			// BAS - for radio buttons, draw the caption to the right of the radio button.
			if (ButtonType == xpRadioButton)
			{
				int	w, h, cbl;
				XPGetElementDefaultDimensions(xpElement_CheckBox,
					&w, &h, &cbl);
								
				XPLMDrawString(XPGetWidgetProperty(inWidget, xpProperty_Enabled, 0) ? white : gray, l + w + 4,
							(t + b) / 2 - (fv / 2) + 2,
							buf, NULL, xplmFont_Basic);
			} else // for al others, center justify the caption.  Note that a caption for something like a window close box is a bit silly....

			XPLMDrawString(XPGetWidgetProperty(inWidget, xpProperty_Enabled, 0) ? white : gray, (l + r) / 2 - (XPLMMeasureString(buf,xplmFont_Basic,-1) / 2), 
						(t + b) / 2 - (fv / 2) + 2,
						buf, NULL, xplmFont_Basic);
			SetProportional(0);						
		}
		return 1;
	case xpMsg_MouseDown:
		if (XPGetWidgetProperty(inWidget, xpProperty_Enabled, 0))
		{
			int	ButtonState = XPGetWidgetProperty(inWidget, xpProperty_ButtonState, NULL);
			if (ButtonBehaviour == xpButtonBehaviorPushButton)
				XPSetWidgetProperty(inWidget, xpProperty_Hilited, 1);
			else
			{
				int	oldState = ButtonState;
				ButtonState = !ButtonState;
				if (ButtonBehaviour == xpButtonBehaviorRadioButton)
					ButtonState = 1;
				XPSetWidgetProperty(inWidget, xpProperty_ButtonState, ButtonState);
				XPSetWidgetProperty(inWidget, xpProperty_Hilited, ButtonState);
				if (ButtonState != oldState)
					XPSendMessageToWidget(inWidget, xpMsg_ButtonStateChanged, xpMode_UpChain, (long) inWidget, ButtonState);
			}
		}
		return 1;
	case xpMsg_MouseDrag:
		if (XPGetWidgetProperty(inWidget, xpProperty_Enabled, 0))
		if (ButtonBehaviour == xpButtonBehaviorPushButton)
		{
			if (IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), l, t, r, b))
				XPSetWidgetProperty(inWidget, xpProperty_Hilited, 1);
			else
				XPSetWidgetProperty(inWidget, xpProperty_Hilited, 0);
		}
		return 1;
	case xpMsg_MouseUp:
		if (XPGetWidgetProperty(inWidget, xpProperty_Enabled, 0))		
		if (ButtonBehaviour == xpButtonBehaviorPushButton)
		{
			if (IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), l, t, r, b))
				XPSendMessageToWidget(inWidget, xpMsg_PushButtonPressed, xpMode_UpChain, (long) inWidget, 0);
			XPSetWidgetProperty(inWidget, xpProperty_Hilited, 0);
		}
		return 1;
	default:
		return 0;
	}	
}					

int		XPTextField(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2)
{
	// Roger and Anya say clicking a text field in the background should not
	// require a second click to do something useful.  So don't eat the click.
	if (XPUSelectIfNeeded(inMessage, inWidget, inParam1, inParam2, 0/*eat*/))	return 1;

		int fv;
		int 		l, t, r, b;
		int			focused = (XPGetWidgetWithFocus() == inWidget);
		long		/*charWidth,*/ scrollPos, scrollLim, descLen;
		char		buf[512];
		
	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);
	XPLMGetFontDimensions(xplmFont_Basic, NULL, &fv, NULL);
//	charWidth = (r - l - 10) / fh;	// can't calc this anymore - depends on the real string used
	scrollPos = XPGetWidgetProperty(inWidget, xpProperty_ScrollPosition, NULL);
	descLen = XPGetWidgetDescriptor(inWidget, buf, 512);
	scrollLim = descLen - XPLMFitStringBackward(buf, buf+descLen, xplmFont_Basic, r - l - 10);
//	scrollLim = descLen - charWidth;		// how many chars can we scroll in.  Compute by: reverse measure the whole string to the field width.
	if (scrollLim < 0)
		scrollLim = 0;

	switch(inMessage) {
	case xpMsg_Create:
		XPSetWidgetProperty(inWidget, xpProperty_TextFieldType, xpTextEntryField);
		XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelStart, 0);
		XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelEnd, 0);
		XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelDragStart, -1);
		XPSetWidgetProperty(inWidget, xpProperty_ScrollPosition, 0);
		XPSetWidgetProperty(inWidget, xpProperty_PasswordMode, 0);
		XPSetWidgetProperty(inWidget, xpProperty_MaxCharacters, 512);
		XPSetWidgetProperty(inWidget, xpProperty_Font, xplmFont_Basic);
		XPSetWidgetProperty(inWidget, xpProperty_Enabled, 1);
		return 1;
	case xpMsg_Draw:
		{
			int			TextFieldType = XPGetWidgetProperty(inWidget, xpProperty_TextFieldType, NULL);
			int			GraphicsTextFieldType = xpElement_TextField;

			if ((TextFieldType != xpTextTransparent) && (TextFieldType != xpTextTranslucent))
			{
				SetAlphaLevels(XPGetWidgetProperty(inWidget, xpProperty_Enabled, NULL) ? 1.0 : 0.5);
				XPDrawElement(l, b, r, t, GraphicsTextFieldType, 0);
				SetAlphaLevels(1.0);
			}
			if (TextFieldType == xpTextTranslucent)
				XPLMDrawTranslucentDarkBox(l, t, r, b);

			if (focused)
			{
				int	selStart = l + 5 + XPLMMeasureString(buf+scrollPos, xplmFont_Basic, XPGetWidgetProperty(inWidget, xpProperty_EditFieldSelStart, NULL));
				int	selEnd   = l + 5 + XPLMMeasureString(buf+scrollPos, xplmFont_Basic, XPGetWidgetProperty(inWidget, xpProperty_EditFieldSelEnd  , NULL));
				if (selStart < l)
					selStart = l;
				if (selEnd > r)
					selEnd = r;
				if (selStart != selEnd)
				{
					// If we're focused and have a band selection, hilite the area.
					XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);			
					if (TextFieldType == xpTextTranslucent)									
					{
						glColor4f(0.5, 0.5, 0.5, 0.7);
					} else {
						SetAlphaLevels(0.5);
						SetupAmbientColor(xpColor_GlassText, NULL);
						SetAlphaLevels(1.0);
					}
					glBegin(GL_QUADS);
					glVertex2i(selStart, t-2);
					glVertex2i(selEnd, t-2);
					glVertex2i(selEnd, b+3);
					glVertex2i(selStart, b+3);
					glEnd();
				} else {				
					// Otherwise draw a caret only if we're either dragging or blinking
					// and its tthat time.
					int	secs = XPLMGetElapsedTime()  * 100.0;
					secs = secs % 100;
					// Use xpProperty_EditFieldSelDragStart as a flag for whether we are dragging...
					// don't blink if dragging, stay constant!
					if ((secs > 50) || (XPGetWidgetProperty(inWidget, xpProperty_EditFieldSelDragStart, NULL) != -1))
					{
						XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0);			
						if (TextFieldType == xpTextTranslucent)									
							glColor4f(0.1, 1.0, 0.1, 1.0);
						else
							SetupAmbientColor(xpColor_CaptionText, NULL);
						glBegin(GL_LINES);
						glVertex2i(selStart, t-2);
						glVertex2i(selStart, b+3);
						glEnd();						
					}
				}
			}

#if !DEV
um?
#endif			
//			if ((descLen-scrollPos) > charWidth)
//				descLen = charWidth + scrollPos;
				
			buf[descLen] = 0;
			
			if (XPGetWidgetProperty(inWidget, xpProperty_PasswordMode, NULL) > 0)
				for (int n = scrollPos; n < descLen; ++n)
					buf[n] = '*';
			
			GLfloat	white[4] = { 0.1, 1.0, 0.1, 1.0 };
			if (TextFieldType != xpTextTranslucent)			
				SetupAmbientColor(XPGetWidgetProperty(inWidget, xpProperty_Enabled, NULL) ? xpColor_CaptionText : xpColor_MenuDarkTinge, white);
			
			XPLMDrawString(white, l+5,
						(t + b) / 2 - (fv / 2) + 2,
						buf+scrollPos, NULL, XPGetWidgetProperty(inWidget, xpProperty_Font, NULL));
		}
		return 1;
	case xpMsg_MouseDown:
		{
			// Short circuit; refuse mouse clicks if we're disabled..this will prevent
			// us getting edited too.
			if (XPGetWidgetProperty(inWidget, xpProperty_Enabled, NULL) == 0)
				return 0;
		
			// Focus ourselves
			if (!focused)
				if (XPSetKeyboardFocus(inWidget) != inWidget)
					return 1;
			
			// If we can, collapse the selection and register the start of a drag.		
			
			long	selPos = XPLMFitStringForward(buf + scrollPos, buf + descLen, xplmFont_Basic, MOUSE_X(inParam1) - l - 5);
//			long	selPos = (MOUSE_X(inParam1) - l - 5) / fh;
			selPos += scrollPos;

			selPos = WIDGET_TMAX(0L, WIDGET_TMIN(selPos, descLen));

			XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelStart, selPos);
			XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelEnd, selPos);
			XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelDragStart, selPos);			
			XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 1);
		}
		return 1;
	case xpMsg_MouseDrag:
		{
			// Update the selection based on the drag.
			long 	selStart = XPGetWidgetProperty(inWidget, xpProperty_EditFieldSelDragStart, NULL);
//			long	selEnd = (MOUSE_X(inParam1) - l - 5) / fh;
			long	selEnd = XPLMFitStringForward(buf + scrollPos, buf + descLen, xplmFont_Basic, MOUSE_X(inParam1) - l - 5);
			selEnd += scrollPos;
			if (selEnd < scrollPos)
			{
				if (scrollPos > 0)
				{
					scrollPos--;
					XPSetWidgetProperty(inWidget, xpProperty_ScrollPosition, scrollPos);
				}
			}
//			if (selEnd > (scrollPos + charWidth))
			if (MOUSE_X(inParam1) > r)
			{
				if (scrollPos < scrollLim)
				{
					scrollPos++;
					XPSetWidgetProperty(inWidget, xpProperty_ScrollPosition, scrollPos);
				}
			}
			selEnd = WIDGET_TMAX(0L, WIDGET_TMIN(selEnd, descLen));
			if (selEnd < selStart)
			{
				long foo = selEnd;
				selEnd = selStart;
				selStart = foo;
				XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 0);
			} else
				XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 1);
				
			XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelStart, selStart);
			XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelEnd, selEnd);
		}
		return 1;
	case xpMsg_MouseUp:
		{
			// Update and terminate the selection based on the drag.
			long selStart = XPGetWidgetProperty(inWidget, xpProperty_EditFieldSelDragStart, NULL);
//			long	selEnd = (MOUSE_X(inParam1) - l - 5) / fh;
			long	selEnd = XPLMFitStringForward(buf + scrollPos, buf + descLen, xplmFont_Basic, MOUSE_X(inParam1) - l - 5);
			selEnd += scrollPos;

			selEnd = WIDGET_TMAX(0L, WIDGET_TMIN(selEnd, descLen));

			if (selEnd < selStart)
			{
				long foo = selEnd;
				selEnd = selStart;
				selStart = foo;
				XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 0);
			 } else
				XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 1);
			
			XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelStart, selStart);
			XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelEnd, selEnd);
			XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelDragStart, -1);
		}
		return 1;
	case xpMsg_KeyTakeFocus:
		// If we don't access this message, we can't get keyboard focus!
		return 1;
	case xpMsg_KeyPress:		
		{
			bool changed = false;
			char	theChar = KEY_CHAR(inParam1);
			bool	shiftKey = KEY_FLAGS(inParam1) & xplm_ShiftFlag;
			bool	upKey = KEY_FLAGS(inParam1) & xplm_UpFlag;
			if (upKey)
				return 1;
			
			std::string::size_type	insertStart = XPGetWidgetProperty(inWidget, xpProperty_EditFieldSelStart, NULL);
			std::string::size_type	insertEnd = XPGetWidgetProperty(inWidget, xpProperty_EditFieldSelEnd, NULL);
			
			std::string	me(buf);

			switch(theChar) {
			case 0:
				return 0;	// DO NOT INSERT NULL CHARS INTO STRINGS!  THAT WOULD BE BAD!!
			case XPLM_KEY_ESCAPE:
			case XPLM_KEY_TAB:
			case XPLM_KEY_RETURN:
				return 0;
			case XPLM_KEY_DELETE:
				if (insertStart == insertEnd)
				{
					if (insertStart > 0)
					{
						insertStart--;
						insertEnd = insertStart;
						me.erase(insertStart, 1);
					}
				} else {
					me.erase(insertStart, insertEnd - insertStart);
					insertEnd = insertStart;
				}
				changed = true;
				break;
			case XPLM_KEY_LEFT:
				{
					if (shiftKey)
					{
						if (XPGetWidgetProperty(inWidget, xpProperty_ActiveEditSide, NULL) && (insertEnd > insertStart))
						{
							if (insertEnd > 0)
								insertEnd--;
							XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, (insertEnd == insertStart) ? 0 : 1);
						} else {
							if (insertStart > 0)
								insertStart--;
							XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 0);
						}
					} else {
						if (insertStart > 0)
							insertStart--;
						insertEnd = insertStart;
						XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 1);
					}
				}
				break;
			case XPLM_KEY_RIGHT:
				if (shiftKey)
				{
					if (XPGetWidgetProperty(inWidget, xpProperty_ActiveEditSide, NULL) || (insertEnd == insertStart))
					{
						insertEnd++;
						XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 1);
					} else {
						insertStart++;
						XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, (insertEnd == insertStart) ? 1 : 0);
					}					
				} else {
					insertEnd++;
					insertStart = insertEnd;
					XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 1);
				}
				break;
			case XPLM_KEY_UP:
				if (shiftKey)
				{
					if (XPGetWidgetProperty(inWidget, xpProperty_ActiveEditSide, NULL))
					{
						insertEnd = insertStart;
						insertStart = 0;
					} else {
						insertStart = 0;
					}					
					XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 0);
				} else {
					insertStart = insertEnd = 0;
					XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 1);
				}
				break;
			case XPLM_KEY_DOWN:
				if (shiftKey)
				{
					if (XPGetWidgetProperty(inWidget, xpProperty_ActiveEditSide, NULL))
					{
						insertEnd = descLen;
					} else {
						insertStart = insertEnd;
						insertEnd = descLen;
					}
					XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 1);
				} else {
					insertStart = insertEnd = descLen;
					XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 1);
				}
				break;
			default:
				if ((XPGetWidgetProperty(inWidget, xpProperty_MaxCharacters, NULL) > 0) &&
					(me.size() + 1 - (insertEnd - insertStart) > XPGetWidgetProperty(inWidget, xpProperty_MaxCharacters, NULL)))
					return 0;
			
				if (insertStart == insertEnd)
					me.insert(insertStart, 1, theChar);
				else
					me.replace(insertStart, insertEnd - insertStart, 1, theChar);
				changed = true;
				insertStart = insertStart + 1;
				insertEnd = insertStart;
				XPSetWidgetProperty(inWidget, xpProperty_ActiveEditSide, 1);
			}			
			
			if (insertStart > me.size())
				insertStart = me.size();
			if (insertEnd > me.size())
				insertEnd = me.size();
							
			if (XPGetWidgetProperty(inWidget, xpProperty_ActiveEditSide, NULL))
			{
#if !DEV
um?
#endif			
//				if (insertStart < scrollPos)
//					scrollPos = insertStart;
//				if ((scrollPos + charWidth) < insertEnd)
//					scrollPos = insertEnd - charWidth;
			} else {
//				if ((scrollPos + charWidth) < insertEnd)
//					scrollPos = insertEnd - charWidth;
//				if (insertStart < scrollPos)
//					scrollPos = insertStart;
			}
			
			if (me.empty())
				scrollPos = 0;
#if !DEV
um?	Use scrollLim?
#endif
//			else if (scrollPos >= (me.size() - charWidth))
//				scrollPos = me.size() - charWidth;

			XPSetWidgetDescriptor(inWidget, me.c_str());
			XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelStart, insertStart);
			XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelEnd, insertEnd);			
			XPSetWidgetProperty(inWidget, xpProperty_ScrollPosition, scrollPos);
		
			if (changed)
				XPSendMessageToWidget(inWidget, xpMsg_TextFieldChanged, xpMode_UpChain, (long) inWidget, 0);
			return 1;
		} 
	case xpMsg_PropertyChanged:
		{
			if (inParam1 == xpProperty_MaxCharacters)
			{
				if (inParam2 < descLen)
				{
					buf[inParam2] = 0;
					XPSetWidgetDescriptor(inWidget, buf);
				}
			}
		}
		return 1;
	case xpMsg_Reshape:
		{	
			int myLen = XPGetWidgetDescriptor(inWidget, NULL, 0);
			if (scrollPos > scrollLim)
			{
				XPSetWidgetProperty(inWidget, xpProperty_ScrollPosition, scrollLim);
			}
		}	
		return 1;
	case xpMsg_DescriptorChanged:
		{
			if (descLen < XPGetWidgetProperty(inWidget, xpProperty_EditFieldSelEnd, NULL))
				XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelEnd, descLen);
			if (descLen < XPGetWidgetProperty(inWidget, xpProperty_EditFieldSelStart, NULL))
				XPSetWidgetProperty(inWidget, xpProperty_EditFieldSelStart, descLen);
		}
		return 1;						
	default:
		return 0;
	}
}					

int		XPScrollBar(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2)
{
	// Select if we're in the background.
	if (XPUSelectIfNeeded(inMessage, inWidget, inParam1, inParam2, 1/*eat*/))	return 1;
	
	int 		l, t, r, b, x, y;
	int SliderPosition, Min, Max, OldSliderPosition;
	int IsVertical, DownBtnSize, DownPageSize, ThumbSize, UpPageSize, UpBtnSize;
	bool UpBtnSelected, DownBtnSelected, ThumbSelected, UpPageSelected, DownPageSelected;
	
	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);

	Min = XPGetWidgetProperty(inWidget, xpProperty_ScrollBarMin, NULL);
	Max = XPGetWidgetProperty(inWidget, xpProperty_ScrollBarMax, NULL);

	switch(inMessage) {
	case xpMsg_Create:
		XPSetWidgetProperty(inWidget, xpProperty_Hilited, 0);
		XPSetWidgetProperty(inWidget, xpProperty_ScrollBarPageAmount, 10);
		XPSetWidgetProperty(inWidget, xpProperty_ScrollBarType, xpTrack_ScrollBar);
		return 1;
	case xpMsg_Draw:
		{
			float		col [] = { 1.0, 1.0, 1.0 };
			int			hilite = XPGetWidgetProperty(inWidget, xpProperty_Hilited, NULL);

			SliderPosition = XPGetWidgetProperty(inWidget, xpProperty_ScrollBarSliderPosition, NULL);
			XPDrawTrack(l, b, r, t, Min, Max, SliderPosition, XPGetWidgetProperty(inWidget, xpProperty_ScrollBarType, NULL), hilite);
		}
		return 1;
	case xpMsg_MouseDown:
		OldSliderPosition = SliderPosition = XPGetWidgetProperty(inWidget, xpProperty_ScrollBarSliderPosition, NULL);
		XPGetTrackMetrics(l, b, r, t, Min, Max, SliderPosition, XPGetWidgetProperty(inWidget, xpProperty_ScrollBarType, NULL), &IsVertical, &DownBtnSize, &DownPageSize, &ThumbSize, &UpPageSize, &UpBtnSize);
		if (IsVertical)
		{
			UpBtnSelected = IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), l, t, r, t - UpBtnSize);
			DownBtnSelected = IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), l, b + DownBtnSize, r, b);
			UpPageSelected = IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), l, (t - UpBtnSize), r, (b + DownBtnSize + DownPageSize + ThumbSize));
			DownPageSelected = IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), l, (t - UpBtnSize - UpPageSize - ThumbSize), r, (b + DownBtnSize));
			ThumbSelected = IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), l, (t - UpBtnSize - UpPageSize), r, (b + DownBtnSize + DownPageSize));
		}
		else
		{
			DownBtnSelected = IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), l, t, l + UpBtnSize, b);
			UpBtnSelected = IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), r - DownBtnSize, t, r, b);
			DownPageSelected = IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), l + DownBtnSize, t, r - UpBtnSize - UpPageSize - ThumbSize, b);
			UpPageSelected = IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), l + DownBtnSize + DownPageSize + ThumbSize, t, r - UpBtnSize, b);
			ThumbSelected = IN_RECT(MOUSE_X(inParam1), MOUSE_Y(inParam1), l + DownBtnSize + DownPageSize, t, r - UpBtnSize - UpPageSize, b);
		}

		if (UpPageSelected)
		{
			SliderPosition+=XPGetWidgetProperty(inWidget, xpProperty_ScrollBarPageAmount, NULL);
			if (SliderPosition > Max)
				SliderPosition = Max;

			XPSetWidgetProperty(inWidget, xpProperty_ScrollBarSliderPosition, SliderPosition);
		}
		else if (DownPageSelected)
		{
			SliderPosition-=XPGetWidgetProperty(inWidget, xpProperty_ScrollBarPageAmount, NULL);
			if (SliderPosition < Min)
				SliderPosition = Min;
			XPSetWidgetProperty(inWidget, xpProperty_ScrollBarSliderPosition, SliderPosition);
		}
		else if (UpBtnSelected)
		{
			SliderPosition++;
			if (SliderPosition > Max)
				SliderPosition = Max;

			XPSetWidgetProperty(inWidget, xpProperty_ScrollBarSliderPosition, SliderPosition);
		}
		else if (DownBtnSelected)
		{
			SliderPosition--;
			if (SliderPosition < Min)
				SliderPosition = Min;
			XPSetWidgetProperty(inWidget, xpProperty_ScrollBarSliderPosition, SliderPosition);
		}
		else if (ThumbSelected)
		{
			XPSetWidgetProperty(inWidget, xpProperty_ScrollBarSlop, IsVertical ? 
				(b + DownBtnSize + DownPageSize + (ThumbSize/2) - MOUSE_Y(inParam1)) :  // WEIRD - messing with thumbsize seems close but never
				(l + DownBtnSize + DownPageSize + (ThumbSize/2) - MOUSE_X(inParam1)) ); // is RIGHT!??!
			XPSetWidgetProperty(inWidget, xpProperty_ScrollBarSliderPosition, SliderPosition);
			XPSetWidgetProperty(inWidget, xpProperty_Hilited, 1);
		}
		else
			XPSetWidgetProperty(inWidget, xpProperty_Hilited, 0);
			
		if (OldSliderPosition != SliderPosition)
			XPSendMessageToWidget(inWidget, xpMsg_ScrollBarSliderPositionChanged, xpMode_UpChain, (long) inWidget, 0);
		return 1;
	case xpMsg_MouseDrag:
		OldSliderPosition = SliderPosition = XPGetWidgetProperty(inWidget, xpProperty_ScrollBarSliderPosition, NULL);
		XPGetTrackMetrics(l, b, r, t, Min, Max, SliderPosition, XPGetWidgetProperty(inWidget, xpProperty_ScrollBarType, NULL), &IsVertical, &DownBtnSize, &DownPageSize, &ThumbSize, &UpPageSize, &UpBtnSize);

		ThumbSelected = XPGetWidgetProperty(inWidget, xpProperty_Hilited, NULL);

		if (ThumbSelected)
		{
			if (inParam1 != 0)
			{				
				if (IsVertical)
				{
					y = MOUSE_Y(inParam1) + XPGetWidgetProperty(inWidget, xpProperty_ScrollBarSlop, 0);
					SliderPosition = round((float)((float)(y - (b + DownBtnSize + ThumbSize/2)) / 
								(float)((t - UpBtnSize - ThumbSize/2) - (b + DownBtnSize + ThumbSize/2))) * Max);
				}
				else
				{
					x = MOUSE_X(inParam1) + XPGetWidgetProperty(inWidget, xpProperty_ScrollBarSlop, 0);
					SliderPosition = round((float)((float)(x - (l + DownBtnSize + ThumbSize/2)) / (float)((r - UpBtnSize - ThumbSize/2) - (l + DownBtnSize + ThumbSize/2))) * Max);
				}

			}
			else
				SliderPosition = 0;

			if (SliderPosition < Min)
				SliderPosition = Min;
			if (SliderPosition > Max)
				SliderPosition = Max;

			XPSetWidgetProperty(inWidget, xpProperty_ScrollBarSliderPosition, SliderPosition);
		}

		if (OldSliderPosition != SliderPosition)
			XPSendMessageToWidget(inWidget, xpMsg_ScrollBarSliderPositionChanged, xpMode_UpChain, (long) inWidget, 0);

		return 1;
	case xpMsg_MouseUp:
		XPSetWidgetProperty(inWidget, xpProperty_Hilited, 0);
		return 1;
	default:
		return 0;
	}	
}					

int		XPCaption(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2)
{
	if (XPUSelectIfNeeded(inMessage, inWidget, inParam1, inParam2, 1/*eat*/))	return 1;

		int fv;
		int 		l, t, r, b;
		int			focused = (XPGetWidgetWithFocus() == inWidget);
		
	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);
	XPLMGetFontDimensions(xplmFont_Basic, NULL, &fv, NULL);

	switch(inMessage) {
	case xpMsg_Create:
		return 1;
	case xpMsg_Draw:
		{
			float		green [4] = { 0.0, 0.0, 0.0, 1.0 };
			if (XPGetWidgetProperty(inWidget, xpProperty_CaptionLit, NULL))
				SetupAmbientColor(xpColor_GlassText, green);
			else
				SetupAmbientColor(xpColor_CaptionText, green);

			XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0);			
			
			char	buf[512];
			long	titleLen = XPGetWidgetDescriptor(inWidget, buf, 512);

			SetProportional(1);		
			XPLMDrawString(green, l+3,
						(t + b) / 2 - (fv / 2),
						buf, NULL, xplmFont_Basic);
			SetProportional(0);	
		}
		return 1;
	default:
		return 0;
	}
}					


int		XPGeneralGraphics(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2)
{
	// Select if we're in the background.
	if (XPUSelectIfNeeded(inMessage, inWidget, inParam1, inParam2, 1/*eat*/))	return 1;

	int 		l, t, r, b;

	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);

	switch(inMessage) {
		case xpMsg_Draw:
		{
			int	GeneralGraphicsType = XPGetWidgetProperty(inWidget, xpProperty_GeneralGraphicsType, NULL);
			int GraphicsElementType = xpElement_Ship;

			switch(GeneralGraphicsType)
			{
				case xpShip:
					GraphicsElementType = xpElement_Ship;
					break;
				case xpILSGlideScope:
					GraphicsElementType = xpElement_ILSGlideScope;
					break;
				case xpMarkerLeft:
					GraphicsElementType = xpElement_MarkerLeft;
					break;
				case xp_Airport:
					GraphicsElementType = xpElement_Airport;
					break;
				case xpNDB:
					GraphicsElementType = xpElement_NDB;
					break;
				case xpVOR:
					GraphicsElementType = xpElement_VOR;
					break;
				case xpRadioTower:
					GraphicsElementType = xpElement_RadioTower;
					break;
				case xpAircraftCarrier:
					GraphicsElementType = xpElement_AircraftCarrier;
					break;
				case xpFire:
					GraphicsElementType = xpElement_Fire;
					break;
				case xpMarkerRight:
					GraphicsElementType = xpElement_MarkerRight;
					break;
				case xpCustomObject:
					GraphicsElementType = xpElement_CustomObject;
					break;
				case xpCoolingTower:
					GraphicsElementType = xpElement_CoolingTower;
					break;
				case xpSmokeStack:
					GraphicsElementType = xpElement_SmokeStack;
					break;
				case xpBuilding:
					GraphicsElementType = xpElement_Building;
					break;
				case xpPowerLine:
					GraphicsElementType = xpElement_PowerLine;
					break;
				case xpVORWithCompassRose:
					GraphicsElementType = xpElement_VORWithCompassRose;
					break;
				case xpOilPlatform:
					GraphicsElementType = xpElement_OilPlatform;
					break;
				case xpOilPlatformSmall:
					GraphicsElementType = xpElement_OilPlatformSmall;
					break;
				case xpWayPoint:
					GraphicsElementType = xpElement_Waypoint;
					break;
			}

			XPDrawElement(l, b, r, t, GraphicsElementType, 0);
		}
		return 1;
	
	default:
		return 0;
	}
}	


int		XPProgress(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2)
{
	// Select if we're in the background.
	if (XPUSelectIfNeeded(inMessage, inWidget, inParam1, inParam2, 1/*eat*/))	return 1;
	
	int 		l, t, r, b;
	int ProgressPosition, Min, Max;

	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);

	Min = XPGetWidgetProperty(inWidget, xpProperty_ProgressMin, NULL);
	Max = XPGetWidgetProperty(inWidget, xpProperty_ProgressMax, NULL);

	switch(inMessage) {
	case xpMsg_Create:
		return 1;
	case xpMsg_Draw:
			ProgressPosition = XPGetWidgetProperty(inWidget, xpProperty_ProgressPosition, NULL);
			XPDrawTrack(l, b, r, t, Min, Max, ProgressPosition, xpTrack_Progress, 0);
		return 1;
	default:
		return 0;
	}	
}					

