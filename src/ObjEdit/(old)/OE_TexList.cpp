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
#include "OE_Utils.h"
#include "OE_Globals.h"
#include "OE_TexList.h"
#include "OE_Notify.h"
#include "OE_Msgs.h"

#include "OE_TexMgr.h"

#include "XPLMGraphics.h"
#include "XPWidgets.h"
#include "XPWidgetUtils.h"
#include "XPStandardWidgets.h"

const	int		kTexListItemHeight = 48;
const	long	kScrollBarID = 10001;


static	void	OE_ResyncScrollbars(XPWidgetID inList);
static	void	OE_TexListNotify(int cat, int msg, void * param, void * ref);
static	int		OE_TexListFunc(		   XPWidgetMessage      inMessage,    
	                                   XPWidgetID           inWidget,    
	                                   long                 inParam1,    
	                                   long                 inParam2);


XPWidgetID		CreateTexListWindow(
							int	x1, int y1, int x2, int y2)
{
	XPWidgetID preview = XPCreateWidget(x1, y2, x2, y1, 1, "Textures", 1, NULL, xpWidgetClass_MainWindow);
	XPSetWidgetProperty(preview, xpProperty_MainWindowHasCloseBoxes, 0);

	XPWidgetID frame = XPCreateWidget(x1+5, y2-20, x2-5-16, y1+5, 1, "", 0, preview, xpWidgetClass_SubWindow);
	XPSetWidgetProperty(frame, 	xpProperty_SubWindowType, xpSubWindowStyle_ListView);
	
	XPWidgetID scrollbar = XPCreateWidget(x2-5-16, y2-20, x2-5,y1+5, 1, "", 0, preview, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(scrollbar, xpProperty_ScrollBarSliderPosition, 0);
	XPSetWidgetProperty(scrollbar, xpProperty_ScrollBarMin, 0);
	XPSetWidgetProperty(scrollbar, xpProperty_ScrollBarMax, 0);
	XPSetWidgetProperty(scrollbar, xpProperty_ScrollBarPageAmount, kTexListItemHeight);
	
	XPWidgetID pane = XPCreateCustomWidget(x1+10,y2-25,x2-10-16,y1+10, 1, "", 0, preview, OE_TexListFunc);
	XPSetWidgetProperty(pane, kScrollBarID, (long) scrollbar);
	XPSetWidgetProperty(pane, xpProperty_Clip, 1);

	OE_Register(OE_TexListNotify, pane);
	
	OE_ResyncScrollbars(pane);
	
	return preview;
}							

static	void	OE_ResyncScrollbars(XPWidgetID inList)
{
	int	top, bottom;
	XPGetWidgetGeometry(inList, NULL, &top, NULL, &bottom);
	int virtual_height = kTexListItemHeight * gTextures.size();
	int	real_height = top - bottom;
	int scroll = virtual_height - real_height;
	if (scroll < 0) scroll = 0;
	XPSetWidgetProperty((XPWidgetID) XPGetWidgetProperty(inList, kScrollBarID, NULL), xpProperty_ScrollBarMax, scroll);	
}

void	OE_TexListNotify(int cat, int msg, void * param, void * ref)
{
	OE_ResyncScrollbars(ref);
}	

int	OE_TexListFunc(		   XPWidgetMessage      inMessage,    
                           XPWidgetID           inWidget,    
                           long                 inParam1,   
                           long                 inParam2)
{
	if (XPUSelectIfNeeded(inMessage, inWidget, inParam1, inParam2, true)) return 1;

	float	black[4] = { 0.0, 0.0, 0.0, 1.0 };
	int t, b, r, l;
	XPGetWidgetGeometry(inWidget, &l, &t, &r, &b);
	XPWidgetID scrollID = (XPWidgetID) XPGetWidgetProperty(inWidget, kScrollBarID, NULL);
	int scrollPos = XPGetWidgetProperty(scrollID, xpProperty_ScrollBarSliderPosition, NULL);
	switch(inMessage) {
	case xpMsg_Destroy:
		OE_Unregister(OE_TexListNotify, inWidget);
		return 1;
	case xpMsg_Draw:
		if (gTextures.empty())
		{
			XPLMDrawString(black, l + 5, t - 24, "No textures defined.", NULL, xplmFont_Basic);			
		} else {
			for (int n = 0; n < gTextures.size(); ++n)
			{
				int	box_top = t + scrollPos - n * kTexListItemHeight;
				int box_bot = box_top - kTexListItemHeight;

				if (n == gCurTexture)
				{
					XPLMSetGraphicsState(0, 0, 0,  0, 0, 0, 0);
					glColor3f(0.0, 0.6, 0.7);
					glBegin(GL_QUADS);
					glVertex2i(l, box_top);
					glVertex2i(r, box_top);
					glVertex2i(r, box_bot);
					glVertex2i(l, box_bot);									
					glEnd();
				}
				
				XPLMDrawString(black, l + 5 + kTexListItemHeight, (box_top + box_bot) / 2 - 10, gTextures[n].name.c_str(), NULL, xplmFont_Basic);			

				XPLMSetGraphicsState(0, 0, 0,  1, 1, 0, 0);
				glColor3f(1.0, 1.0, 1.0);

				if (!gObjects.empty())
				{
					string tex = gObjects[gLevelOfDetail].texture;
					if (!tex.empty())
					{
						GLenum t = FindTexture(tex, false, NULL, NULL);
						if (t)
						{
							XPLMBindTexture2d(t, 0);
							XPLMSetGraphicsState(0, 1, 0,  1, 1, 0, 0);
						}	
					}
				}

				glBegin(GL_QUADS);
				glTexCoord2f(gTextures[n].s1, gTextures[n].t2);
				glVertex2i(l + 3, box_top - 2);
				glTexCoord2f(gTextures[n].s2, gTextures[n].t2);
				glVertex2i(l + kTexListItemHeight - 1, box_top - 2);
				glTexCoord2f(gTextures[n].s2, gTextures[n].t1);
				glVertex2i(l + kTexListItemHeight - 1, box_bot + 2);
				glTexCoord2f(gTextures[n].s1, gTextures[n].t1);
				glVertex2i(l + 3, box_bot + 2);
				glEnd();
				
								
				XPLMSetGraphicsState(0, 0, 0,  0, 0,  0, 0);
				glColor3f(0.0, 0.0, 0.0);
				glBegin(GL_LINE_LOOP);
				glVertex2i(l + 2, box_top - 1);
				glVertex2i(l + kTexListItemHeight, box_top - 1);
				glVertex2i(l + kTexListItemHeight, box_bot + 1);
				glVertex2i(l + 2, box_bot + 1);
				glEnd();
			}
		}
		return 1;
	case xpMsg_MouseDown:
	case xpMsg_MouseDrag:
	case xpMsg_MouseUp:
		{
			int item = (t - MOUSE_Y(inParam1) + scrollPos) / kTexListItemHeight;
			if (item >= gTextures.size() || item < 0)
				item = -1;
			if (gCurTexture != item)
			{
				gCurTexture = item;
				OE_Notify(catagory_Texture, msg_TexSelectionChanged, NULL);
			}
		}
		return 1;
	default:
		return 0;
	}
}	