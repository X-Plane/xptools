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
#include "XSBPopups.h"
#include <vector>
#include <string>
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPUIGraphics.h"
#include "XPUIGraphicsPrivate.h"
#include "XPWidgets.h"
#include "XPLMUtilities.h"
#include "XPWidgetUtils.h"
#if APL
#include <OpenGL/gl.h>
#else
#include <gl.h>
#endif

/************************************************************************
 *  X-PLANE UI INFRASTRUCTURE CODE
 ************************************************************************
 *
 * This code helps provde an x-plane compatible look.  It is copied from 
 * the source code from the widgets DLL; someday popups will be part of
 * this, so our popups are written off of the same APIs.
 *
 */



// This routine sets up a color from the above table.  Pass
// in a float[3] to get the color; pass in NULL to have the
// OpenGL color be set immediately.
void	SetupAmbientColor(int inColorID, float * outColor);

// Sets the alpha level that will be used with the above routine.
void	SetAlphaLevels(float inAlphaLevel);


// This routine draws a single quad; we must already be in GL_QUADS mode.  We pass in coordinates in
// pixels (assuming a 512x512 UI) for textures.
void	SingleQuad(int l, int b, int r, int t, int tl, int tb, int tr, int tt)
{
	glTexCoord2f((float) tl / 512.0, (float) tt / 512.0);
	glVertex2i(l, t);
	glTexCoord2f((float) tr / 512.0, (float) tt / 512.0);
	glVertex2i(r, t);
	glTexCoord2f((float) tr / 512.0, (float) tb / 512.0);
	glVertex2i(r, b);
	glTexCoord2f((float) tl / 512.0, (float) tb / 512.0);
	glVertex2i(l, b);
}

// This draws a quad with 3 sections horizontally and vertically.  The vertical stripes
// are repated, the horizontal ones are stretched.
void	DrawStretchedQuad(int left, int bottom, int right, int top,
						int tx1, int tx2, int tx3, int tx4,
						int ty1, int ty2, int ty3, int ty4)
{
	int width = right - left;
	int height = top - bottom;
	int bot_stripe = ty2 - ty1;
	int top_stripe = ty4 - ty3;
	int mid_stripe = height - bot_stripe - top_stripe;
	int mid_tex = ty3 - ty2;
	int mid_repeats = mid_stripe / mid_tex;
	int mid_slop = (mid_stripe - (mid_repeats * mid_tex) + 1) & ~1;
	
	int left_h = tx2 - tx1;
	int right_h = tx4 - tx3;
	
	glBegin(GL_QUADS);
	
	// Draw the bottom three elements.
	SingleQuad(left, bottom, left + left_h, bottom + bot_stripe,				tx1, ty1, tx2, ty2);
	SingleQuad(left + left_h, bottom, right - right_h, bottom + bot_stripe,		tx2, ty1, tx3, ty2);
	SingleQuad(right - right_h, bottom, right, bottom + bot_stripe,				tx3, ty1, tx4, ty2);
	
	// Draw horizontal stripes working up.
	for (int n = 0; n < mid_repeats; ++n)
	{
		int	offset = bottom + bot_stripe + n * mid_tex;
		SingleQuad(left, offset, left + left_h, offset + mid_tex,					tx1, ty2, tx2, ty3);
		SingleQuad(left + left_h, offset, right - right_h, offset + mid_tex,		tx2, ty2, tx3, ty3);
		SingleQuad(right - right_h, offset, right, offset + mid_tex,				tx3, ty2, tx4, ty3);
	}
	
	// If we're not an even number of repetitions, draw a partial stripe too.
	if (mid_slop != 0)
	{
		int offset = bottom + bot_stripe + mid_tex * mid_repeats;
		SingleQuad(left, offset, left + left_h, offset + mid_slop,					tx1, ty2, tx2, ty2 + mid_slop);
		SingleQuad(left + left_h, offset, right - right_h, offset + mid_slop,		tx2, ty2, tx3, ty2 + mid_slop);
		SingleQuad(right - right_h, offset, right, offset + mid_slop,				tx3, ty2, tx4, ty2 + mid_slop);
	}

	// Draw the top.
	SingleQuad(left, top - top_stripe, left + left_h, top,						tx1, ty3, tx2, ty4);
	SingleQuad(left + left_h, top - top_stripe, right - right_h, top,			tx2, ty3, tx3, ty4);
	SingleQuad(right - right_h, top - top_stripe, right, top,					tx3, ty3, tx4, ty4);
	
	glEnd();	
}						


#pragma mark -

/************************************************************************
 *  POPUP MENU IMPLEMENTATION
 ************************************************************************/


// This structure represents a popup menu internally...it consists of arrays
// per item and some general stuff.  It is implemented as a giant window; that
// window has a ptr to this struct as its refcon.  While the whole window isn't
// drawn in, we make the window full screen so if the user clicks outside the
// popup, we capture the click and dismiss the popup.
struct	XSBPopupMenu_t {

	XPLMWindowID				window;		// The window that implements us.

	// Per item:
	std::vector<std::string>	items;		// The name of the item
	std::vector<bool>			enabled;	// Is it enabled?
	std::vector<int>			lefts;		// The rectangle of the item, relative to the top left corner of the menu/
	std::vector<int>			rights;
	std::vector<int>			bottoms;
	std::vector<int>			tops;
	std::vector<int>			vstripes;	// An array of vertical stripes (relative to the left of the menu
											// for really big menus.
	int							left;		// The overall bounds of the menu in global screen coordinates.
	int							top;
	int							right;
	int							bottom;

	XSBPopupPick_f				function;	// Info for our callback function.
	void *						ref;
	
};

// This routine finds the item that is in a given point, or returns -1 if there is none.
// It simply trolls through all the items.
static int XSBItemForHeight(XSBPopupMenu_t * pmenu, int inX, int inY)
{
	inX -= pmenu->left;
	inY -= pmenu->top;
	for (int n = 0; n < pmenu->items.size(); ++n)
	{
		if ((inX >= pmenu->lefts[n]) && (inX < pmenu->rights[n]) &&
			(inY >= pmenu->bottoms[n]) && (inY < pmenu->tops[n]))
		{
			return n;
		}
	}
	return -1;	
}

// This is the drawing hook for a popup menu.
static void XSBPopupDrawWindowCB(
                                   XPLMWindowID         inWindowID,    
                                   void *               inRefcon)
{
	static	int	firstTime = true;

	XSBPopupMenu_t * pmenu = (XSBPopupMenu_t *) inRefcon;
	int	x, y;
	XPLMGetMouseLocation(&x, &y);

	// This is the index number of the currently selected item, based
	// on where the mouse is.
	int menu_offset = XSBItemForHeight(pmenu, x, y);	

		// For x-plane 7, we stretch the popup menu background from the 
		// general interface bitmap.  Use alpha blending.
		XPLMSetGraphicsState(0, 1, 0,  0, 1,  0, 0);
		XPLMBindTexture2d(XPLMGetTexture(xplm_Tex_GeneralInterface), 0);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		
		// These are offsets for the popup menu bkgnd in the texture.
		DrawStretchedQuad(pmenu->left - 8, pmenu->bottom - 13, pmenu->right + 8, pmenu->top + 2, 
							70, 70+12, 70+12+18, 70+12+18+12, // h
							315, 315+14, 315+14+10, 315+14+10+18); // verts
		
		XPLMSetGraphicsState(0, 0, 0,  0, 0,  0, 0);
			
	int	item_top = pmenu->top;
	int n;
	XPLMSetGraphicsState(0, 0, 0,  0, 0,  0, 0);
	
	// Draw any vertical stripes that must be drawn for multi-column menus.
	for (n = 0; n < pmenu->vstripes.size(); ++n)
	{
		SetupAmbientColor(xpColor_MenuDarkTinge, NULL);
		glBegin(GL_LINES);
		glVertex2i(pmenu->left + pmenu->vstripes[n] - 1, pmenu->top);
		glVertex2i(pmenu->left + pmenu->vstripes[n] - 1, pmenu->bottom);
		glEnd();
		SetupAmbientColor(xpColor_MenuLiteTinge, NULL);
		glBegin(GL_LINES);
		glVertex2i(pmenu->left + pmenu->vstripes[n], pmenu->top);
		glVertex2i(pmenu->left + pmenu->vstripes[n], pmenu->bottom);
		glEnd();	
	}
	
	// Now draw each item.
	for (n = 0; n < pmenu->items.size(); ++n)
	{
		// Calcualte the item rect in global coordinates.
		int item_bottom = pmenu->bottoms[n] + pmenu->top;
		int item_top    = pmenu->tops[n] + pmenu->top;
		int item_left   = pmenu->lefts[n] + pmenu->left;
		int item_right  = pmenu->rights[n] + pmenu->left;
		
		if (!pmenu->enabled[n] && pmenu->items[n] == "-")
		{
			// Draw two lines for dividers.
			XPLMSetGraphicsState(0, 0, 0,  0, 0,  0, 0);
			SetupAmbientColor(xpColor_MenuLiteTinge, NULL);
			glBegin(GL_LINE_STRIP);
			glVertex2i(item_left, item_top - 1);
			glVertex2i(item_right, item_top - 1);
			glEnd();
			SetupAmbientColor(xpColor_MenuDarkTinge, NULL);
			glBegin(GL_LINE_STRIP);
			glVertex2i(item_left, item_top);
			glVertex2i(item_right, item_top);
			glEnd();
		} else {
			// If we are hilited, draw the hilite bkgnd.
			if (menu_offset == n && pmenu->enabled[n])
			{
					// X-Plane 7 alpha blends the hilite!
					SetAlphaLevels(0.25);
					XPLMSetGraphicsState(0, 0, 0,  0, 1, 0, 0);

				SetupAmbientColor(xpColor_MenuHilite, NULL);
				SetAlphaLevels(1.0);
				glBegin(GL_QUADS);
				glVertex2i(item_left, item_top);
				glVertex2i(item_right, item_top);
				glVertex2i(item_right, item_bottom);
				glVertex2i(item_left, item_bottom);
				glEnd();						
			}
			
			// Draw the text for the menu item, taking into account
			// disabling as a color.
			float	text[3];
			SetupAmbientColor(pmenu->enabled[n] ? xpColor_MenuText : xpColor_MenuTextDisabled, text);
			
			XPLMDrawString(text,
						item_left + 18, item_bottom + 2,
						const_cast<char *>(pmenu->items[n].c_str()), NULL, xplmFont_Menus);
		}
	}
}                                   

static void XSBPopupHandleKeyCB(
                                   XPLMWindowID         inWindowID,    
                                   char                 inKey,    
                                   XPLMKeyFlags         inFlags,    
                                   char                 inVirtualKey,    
                                   void *               inRefcon,    
                                   int                  losingFocus)
{
	// Nothing to do when a key is pressed; popup menus don't use keys.
}                                     

// This is the mouse click handler.
static int XSBPopupHandleMouseClickCB(
                                   XPLMWindowID         inWindowID,    
                                   int                  x,    
                                   int                  y,    
                                   XPLMMouseStatus      inMouse,    
                                   int					inButton,
                                   void *               inRefcon)
{
	// Normally we do nothing.  But when we get an up click we dismiss.
	if (inMouse == xplm_MouseUp)
	{
		XSBPopupMenu_t * pmenu = (XSBPopupMenu_t *) inRefcon;

		int menu_offset = XSBItemForHeight(pmenu, x, y);	
		
		// If we got an item click and it is not enabled,
		// pretend nothing was picked.
		if (menu_offset >= 0 && !pmenu->enabled[menu_offset])
			menu_offset = -1;
			
		// Call the callback
		if (pmenu->function)
			pmenu->function(menu_offset, pmenu->ref);

		// cleanup
		delete pmenu;
		XPLMDestroyWindow(inWindowID);
	}
	return 1;
}

// This routine just has to build the popup and then the window takes care of itself.
void		XSBPickPopup(
						int				inMouseX,
						int				inMouseY,
						const char *	inItems,
						int				inCurrentItem,
						XSBPopupPick_f	inCallback,
						void *			inRefcon)
{
	int	screenWidth, screenHeight;
	int	fontWidth, fontHeight;
	XPLMGetScreenSize(&screenWidth, &screenHeight);
	XPLMGetFontDimensions(xplmFont_Menus, &fontWidth, &fontHeight, NULL);
	
	// Allocate mem for the structure and build a new window as big as teh screen.
	XSBPopupMenu_t *	info = new XSBPopupMenu_t;
	
	XPLMWindowID		windID = XPLMCreateWindow(0, screenHeight, screenWidth, 0, 1, 
		XSBPopupDrawWindowCB, XSBPopupHandleKeyCB, XSBPopupHandleMouseClickCB, info);
	
	if (inCurrentItem < 0) inCurrentItem = 0;
	
	/************ PARSE THE MENU STRING INTO MENU ITEMS **********/
	
	// Parse the itemes into arrays.  Remember how tall they are so
	// we can calculate the geometry.
	std::vector<int>	heights;
	std::string	items(inItems);
	while (!items.empty())
	{
		std::string::size_type split = items.find(';');
		if (split == items.npos)
		{
			split = items.size();
		}

		std::string	item = items.substr(0, split);

		if (item == "-")
		{
			info->items.push_back("-");
			info->enabled.push_back(false);
			heights.push_back(2);
		} else {
			if (!item.empty() && item[0] == '(')
			{
				info->enabled.push_back(false);
				info->items.push_back(item.substr(1));
				heights.push_back(12);
			} else {
				info->enabled.push_back(true);
				info->items.push_back(item);
				heights.push_back(12);
			}
		}
		
		if (item.size() == items.size())
			break;
		else
			items = items.substr(split+1);
	}
	
	/************ PLACE THE ITEMS IN COLUMNS **********/

	int	menuWidth = 0;
	int	leftOff = 0, topOff = 0;
	
	// Calculate the widest menu item anywhere.
	for (std::vector<std::string>::iterator iter = info->items.begin(); iter != info->items.end(); ++iter)
		if (iter->size() > menuWidth)
			menuWidth = iter->size();
	menuWidth *= fontWidth;
	menuWidth += 35;
	
	int cols = 1;
	int itemsInCol = 0;
	int	maxLen = 0;
	int	ourItemLeft = 0, ourItemTop = 0;
	
	// Stack up each item, building new columns as necessary.
	for (int i = 0; i < heights.size(); ++i)
	{
		itemsInCol++;
		info->lefts.push_back(leftOff);
		info->rights.push_back(leftOff + menuWidth);
		info->tops.push_back(topOff);
		info->bottoms.push_back(topOff - heights[i]);
		if (i == inCurrentItem) 
		{
			ourItemLeft = leftOff;
			ourItemTop = topOff;
		}
		topOff -= heights[i];
		if (maxLen > topOff)
			maxLen = topOff;
		
		if (topOff < -(screenHeight - 50))
		{
			itemsInCol = 0;
			cols++;
			topOff = 0;
			leftOff += menuWidth;
			info->vstripes.push_back(leftOff);
		}
	}
	
	// If we built a new column but had no items for it, throw it out.
	if (itemsInCol == 0)
	{
		cols--;
		info->vstripes.pop_back();
	}
	
	// Now place the window.  Make sure to not let it go off screen.
	info->window = windID;
	info->left = inMouseX - ourItemLeft;
	info->top = inMouseY - ourItemTop;
	info->right = inMouseX + (menuWidth * cols) - ourItemLeft;
	info->bottom = inMouseY + maxLen - ourItemTop;
	
	if (info->right > (screenWidth-10))
	{
		int deltaLeft = (info->right - (screenWidth-10));
		info->right -= deltaLeft;
		info->left -= deltaLeft;
	}
	if (info->left < 10)
	{
		int deltaRight = 10 - info->left;
		info->right += deltaRight;
		info->left += deltaRight;
	}
	if (info->bottom < 10)
	{
		int deltaUp = 10 - info->bottom;
		info->bottom += deltaUp;
		info->top += deltaUp;
	}
	if (info->top > (screenHeight-30))
	{
		int deltaDown = (info->top - (screenHeight-30));
		info->top -= deltaDown;
		info->bottom -= deltaDown;
	}
	
	info->function = inCallback;
	info->ref = inRefcon;
}

#pragma mark -

/************************************************************************
 *  POPUP MENU WIDGET
 ************************************************************************/



// This routine is called by our popup menu when it is picked; we 
// have stored our widget in our refcon, so we save the new choice
// and send a widget message.
static	void 	XSBPopupWidgetProc(int inChoice, void * inRefcon)
{
	if (inChoice != -1)
	{
		XPSetWidgetProperty(inRefcon, xpProperty_PopupCurrentItem, inChoice);
		XPSendMessageToWidget(inRefcon, xpMessage_NewItemPicked, xpMode_UpChain, (long) inRefcon, inChoice);
	}
}

// This widget Proc implements the actual button.

int		XSBPopupButtonProc(
					XPWidgetMessage			inMessage,
					XPWidgetID				inWidget,
					long					inParam1,
					long					inParam2)
{
	// Select if we're in the background.
	if (XPUSelectIfNeeded(inMessage, inWidget, inParam1, inParam2, 1/*eat*/))	return 1;
	
	int fh, fv;
	int l, t, r, b;
	char	buf[4096];
	
	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);
	XPLMGetFontDimensions(xplmFont_Basic, &fh, &fv, NULL);
	
	int	curItem = XPGetWidgetProperty(inWidget, xpProperty_PopupCurrentItem, NULL);

	switch(inMessage) {
	case xpMsg_Create:
	case xpMsg_DescriptorChanged:
	case xpMsg_PropertyChanged:
		// If our data changes, reparse our descriptor to change our current item.
		if (inMessage != xpMsg_PropertyChanged || inParam1 == xpProperty_PopupCurrentItem)
		{
			XPGetWidgetDescriptor(inWidget, buf, sizeof(buf));
			char * p = buf;
			int picksToSkip = curItem;
			while (picksToSkip > 0)
			{
				while (*p && *p != ';') ++p;
				if (*p == 0) break;
				++p;
				--picksToSkip;
			}
			char * term = p;
			while (*term && *term != ';') ++term;
			// Store an offset and length of our descriptor that will show as our current text.
			XPSetWidgetProperty(inWidget, xpProperty_OffsetToCurrentItem, p - buf);
			XPSetWidgetProperty(inWidget, xpProperty_CurrentItemLen, term - p);			
			XPSetWidgetProperty(inWidget, xpProperty_Enabled, 1);
		}
		return 1;
	case xpMsg_Draw:
		{
			float		white [4];
			float		gray [4];
			
			int itemOffset = XPGetWidgetProperty(inWidget, xpProperty_OffsetToCurrentItem, NULL);
			int itemLen = XPGetWidgetProperty(inWidget, xpProperty_CurrentItemLen, NULL);

			// Drawing time.  Find the sim version once.
			static float firstTime = true;
			static	int	charWidth;
			if (firstTime)
			{
				firstTime = false;
				
				XPLMGetFontDimensions(xplmFont_Basic, &charWidth, NULL, NULL);
			}
			
			// If we are version 7 of the sim, use Sergio's great new popup item.
			// Since there is no UI element code for this, we must draw it by hand!

				XPLMSetGraphicsState(0, 1, 0,  0, 1,  0, 0);
				XPLMBindTexture2d(XPLMGetTexture(xplm_Tex_GeneralInterface), 0);
				glColor4f(1.0, 1.0, 1.0, 1.0);
				
				int center = (t + b) / 2;
				
				// These are offsets for the popup button in the texture
				DrawStretchedQuad(l - 4, center - 13, r + 4, center + 13, 
									357, 357+12,357+12+30, 357+12+30+26, 
									486, 486+8, 486+8+6, 486+8+6+12);
		
			// Now draw the button label.
			long	titleLen = XPGetWidgetDescriptor(inWidget, buf, sizeof(buf));

			SetupAmbientColor(xpColor_MenuText, white);
			SetupAmbientColor(xpColor_MenuTextDisabled, gray);
			
			if (charWidth)
			{
				long	maxCharCapacity = (r - l - 24) / charWidth;
				if (itemLen > maxCharCapacity)
					itemLen = maxCharCapacity;
			}
			
			buf[itemOffset + itemLen] = 0;
			if (buf[itemOffset] == '(')	++itemOffset;
			titleLen = strlen(buf + itemOffset);
			
			XPLMDrawString(XPGetWidgetProperty(inWidget, xpProperty_Enabled, 0) ? white : gray, l + 4, //(l + r) / 2 - (fh * titleLen / 2), 
						(t + b) / 2 - (fv / 2) + 2,
						buf + itemOffset, NULL, xplmFont_Basic);
						
		}
		return 1;
	case xpMsg_MouseDown:
		// If the mouse is clicked, do a popup pick.
		if (XPGetWidgetProperty(inWidget, xpProperty_Enabled, 0))
		{
			XPGetWidgetDescriptor(inWidget, buf, sizeof(buf));
			
			XSBPickPopup(l, t, buf, XPGetWidgetProperty(inWidget, xpProperty_PopupCurrentItem, NULL),
								XSBPopupWidgetProc, inWidget);
			return 1;
		}
	default:
		return 0;
	}	
}				

// To create a popup, make a new widget with our button proc as the widget proc.
XPWidgetID           XSBCreatePopup(
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
                                   XSBPopupButtonProc);
}                                   

#pragma mark -

