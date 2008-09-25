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
#ifndef _XSBPopups_h_
#define _XSBPopups_h_

#include "XPWidgets.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
 * POPUP MENU PICKS
 ************************************************************************
 *
 * This code helps do a popup menu item pick.  Since x-plane must be
 * running to track the mouse, this popup menu pick is effectively
 * asynchronous and non-modal to the code...you call the function and
 * some time later your callback is called.
 *
 * However, due to the way the popup pick is structured, it will appear to
 * be somewhat modal to the user in that the next click after the popup
 * is called must belong to it.
 *
 */

/*
 * XSBPopupPick_f
 *
 * This function is called when your popup is picked.  inChoice will be the number
 * of the item picked, or -1 if no item was picked.  (You should almost always ignore
 * a -1.
 *
 */
typedef	void (* XSBPopupPick_f)(int inChoice, void * inRefcon);

/*
 * XSBPickPopup
 *
 * This routine creates a dynamic 'popup menu' on the fly.  If inCurrentItem is
 * non-negative, it is the item that will be under the mouse.  In this case, the
 * mouse X and Y should be the top left of a popup box if there is such a thing.
 * If inCurrentItem is -1, the popup menu appears at exactly inMouseX and inMouseY.
 *
 * You pass in the items, newline terminated ('\n') as well as a callback that is
 * called when an item is picked, and a ref con for that function.
 *
 */
void		XSBPickPopup(
						int				inMouseX,
						int				inMouseY,
						const char *	inItems,
						int				inCurrentItem,
						XSBPopupPick_f	inCallback,
						void *			inRefcon);

/* Impl notes: we can dispose from the mouse up.  So...on mouse up
 * we first call the popup func but then we nuke ourselves.  Internally
 * there is a data structure that is in the refcon of the xplm window that
 * contains the callback for the user and the text, etc. */

/************************************************************************
 * POPUP MENU BUTTON WIDGET
 ************************************************************************
 *
 * This widget implements a stanard pick-one-from-many-style popup menu
 * button.  The text is taken from the current item.  The descriptor is
 * the items, newline-terminated.
 *
 * A message is sent whenever a new item is picked by the user.
 *
 */

#define	xpWidgetClass_Popup					9

enum {
	// This is the item number of the current item, starting at 0.
	xpProperty_PopupCurrentItem				= 1800,

	// These are for caching, do not use!!
	xpProperty_OffsetToCurrentItem			= 1801,
	xpProperty_CurrentItemLen				= 1802
};

enum {
	// This message is sent when an item is picked.
	// param 1 is the widget that was picked, param 2
	// is the item number.
	xpMessage_NewItemPicked					= 1800
};

/*
 * XSBCreatePopup
 *
 * This routine makes a popup widget for you.  You must provide
 * a container for this widget, like a window for it to sit in.
 *
 */
XPWidgetID           XSBCreatePopup(
                                   int                  inLeft,
                                   int                  inTop,
                                   int                  inRight,
                                   int                  inBottom,
                                   int                  inVisible,
                                   const char *         inDescriptor,
                                   XPWidgetID           inContainer);

int		XSBPopupButtonProc(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2);


#ifdef __cplusplus
}
#endif


#endif