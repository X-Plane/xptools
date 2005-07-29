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
#include "OE_TexEd.h"

#include "XPLMGraphics.h"
#include "XPWidgets.h"
#include "XPWidgetUtils.h"
#include "XPStandardWidgets.h"
#include "OE_Notify.h"
#include "OE_Msgs.h"
#include "OE_Globals.h"
#include "OE_TexMgr.h"

const	float	kHandleRad = 3.0;		// Handles are about 6 pixels around
const	int		kMargin = 5;			// 5 pixel margin when zoomed out, so we can see handles, etc.
const	int		kZoomFactor = 16;		// Max zoom out is 16x smaller.  Makes 64x64 texture size at Max

// Data stored on the actual tex editor widget
const	long	kHScrollBarID 	= 1000;
const	long	kVScrollBarID 	= 1001;
const	long	kZoomID 		= 1002;
const	long	kCurHandle		= 1003;
const	long	kHandleSlopX	= 1004;
const	long	kHandleSlopY	= 1005;


inline float	InterpF(float mi, float ma, float v) { return mi + v * (ma - mi); }
inline float	GetInterpF(float mi, float ma, float v) { if (ma == mi) return 0.0; return (v - mi) / (ma - mi); }

// second array is s1, s2, t1, t2
int kHandleWriteFlags[9][4] = {
	//	s1	s2	t1	t2
	{ 	1,	0,	0,	1	},		// Top left
	{	0,	1,	0,	1	},		// Top right
	{	0,	1,	1,	0	},		// Bot right
	{	1,	0,	1,	0	},		// Bot left
	{	0,	0,	0,	1	},		// Top
	{	0,	1,	0,	0	},		// Right
	{	0,	0,	1,	0	},		// Bottom
	{	1,	0,	0,	0	},		// Left
	{	1,	1,	1,	1	} };	// Center


static	void	OE_TexEdNotify(int cat, int msg, void * param, void * ref);
static	int		OE_TexEdFunc(		   XPWidgetMessage      inMessage,    
	                                   XPWidgetID           inWidget,    
	                                   long                 inParam1,    
	                                   long                 inParam2);
static	void	ResyncScrollBars(XPWidgetID);




XPWidgetID	OE_CreateTexEd(
					int x1, int y1, int x2, int y2)
{
	XPWidgetID texEd = XPCreateWidget(x1, y2, x2, y1, 1, "Texture Edit", 1, NULL, xpWidgetClass_MainWindow);
	XPSetWidgetProperty(texEd, xpProperty_MainWindowHasCloseBoxes, 0);

	XPWidgetID vScrollbar = XPCreateWidget(x2-16, y2-18, x2,y1+16, 1, "", 0, texEd, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(vScrollbar, xpProperty_ScrollBarSliderPosition, 0);
	XPSetWidgetProperty(vScrollbar, xpProperty_ScrollBarMin, 0);
	XPSetWidgetProperty(vScrollbar, xpProperty_ScrollBarMax, 100);

	XPWidgetID hScrollbar = XPCreateWidget(x1, y1+16, x2-16,y1, 1, "", 0, texEd, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(hScrollbar, xpProperty_ScrollBarSliderPosition, 0);
	XPSetWidgetProperty(hScrollbar, xpProperty_ScrollBarMin, 0);
	XPSetWidgetProperty(hScrollbar, xpProperty_ScrollBarMax, 100);
	
	XPWidgetID pane = XPCreateCustomWidget(x1, y2-18, x2-16, y1+16, 1, "", 0, texEd, OE_TexEdFunc);

	XPSetWidgetProperty(pane, kHScrollBarID, (long) hScrollbar);
	XPSetWidgetProperty(pane, kVScrollBarID, (long) vScrollbar);
	XPSetWidgetProperty(pane, xpProperty_Clip, 1);
	XPSetWidgetProperty(pane, kZoomID, kZoomFactor);
	XPSetWidgetProperty(pane, kCurHandle, -1);

	OE_Register(OE_TexEdNotify, pane);

	return texEd;
}					

void	OE_TexEdNotify(int cat, int msg, void * param, void * ref)
{
	if ((cat == catagory_Texture && msg == msg_TexLoaded) ||
		(cat == catagory_Object && msg == msg_ObjectLoaded))
	{
		ResyncScrollBars((XPWidgetID) ref);
	}
}

void	ResyncScrollBars(XPWidgetID me)
{
	if (!gObjects.empty())
	{
		string tex = gObjects[gLevelOfDetail].texture;
		if (!tex.empty())
		{
			int twidth, theight;
			if (FindTexture(tex, false, &twidth, &theight))
			{
				int zoom = XPGetWidgetProperty(me, kZoomID, NULL);
				twidth *= zoom;
				twidth /= kZoomFactor;
				theight *= zoom;
				theight /= kZoomFactor;
				
				int t, l, r, b;
				XPGetWidgetGeometry(me, &l, &t, &r, &b);
				
				int hScrollDis = twidth - (r - l);
				if (hScrollDis < 0) hScrollDis = 0;
				int vScrollDis = theight - (t - b);
				if (vScrollDis < 0) vScrollDis = 0;
				
				XPSetWidgetProperty((XPWidgetID) XPGetWidgetProperty(me, kHScrollBarID, NULL),
					xpProperty_ScrollBarMax, hScrollDis);
				XPSetWidgetProperty((XPWidgetID) XPGetWidgetProperty(me, kVScrollBarID, NULL),
					xpProperty_ScrollBarMax, vScrollDis);
			}
		}
	}		
}

int		OE_TexEdFunc(		   XPWidgetMessage      inMessage,    
	                                   XPWidgetID           inWidget,    
	                                   long                 inParam1,    
	                                   long                 inParam2)
{
	if (XPUSelectIfNeeded(inMessage, inWidget, inParam1, inParam2, true)) return 1;
	
		int		left, top, right, bottom;			// The dimensions of our total pane
		int		cleft, ctop, cright, cbottom;		// The dimensions in which we actually show the texture
		int 	tleft, ttop, tright, tbottom;		// The dimensions of the texture in screen coordinates, after zooming, etc.
		int		hScroll, vScroll;					// How far into the texture are we scrolled?
		int 	zoom;								// Zoom level
		int		twidth = 32, theight = 32;			// Texture raw dimensions
		GLenum 	texID = 0; 							// The texture's OGL ID.
		float	handles[9][2];						// 8 handles in window coords: TL, TR, BR, BL, then T, R, B, L, Center
	
	/*
	 * Before we do much of anything, we have a ton of geometry data to calculate...
  	 * stuff we'll need both in drawing and mouse tracking.
  	 *
  	 */
	
	/* Calculate the locations of the window and its parts */
		
	hScroll = XPGetWidgetProperty((XPWidgetID) XPGetWidgetProperty(inWidget, kHScrollBarID, NULL), xpProperty_ScrollBarSliderPosition, NULL);
	vScroll = XPGetWidgetProperty((XPWidgetID) XPGetWidgetProperty(inWidget, kVScrollBarID, NULL), xpProperty_ScrollBarSliderPosition, NULL);
		
	XPGetWidgetGeometry(inWidget, &left, &top, &right, &bottom);
	cleft	 = left 	+ kMargin; 
	cright	 = right 	- kMargin; 
	cbottom	 = bottom 	+ kMargin; 
	ctop	 = top 		- kMargin;
	
	/* Calculate the texture's location */
	if (!gObjects.empty())
	{
		string tex = gObjects[gLevelOfDetail].texture;
		if (!tex.empty())
			texID = FindTexture(tex, false, &twidth, &theight);
	}

	tleft = cleft - hScroll;
	tright = tleft + twidth;
	ttop = ctop + vScroll;
	tbottom = ttop - theight;

	zoom = XPGetWidgetProperty(inWidget, kZoomID, NULL);
	twidth *= zoom; twidth /= kZoomFactor;
	theight *= zoom; theight /= kZoomFactor;
	
	/* Calculate control handle positions */
	if (gCurTexture != -1)
	{
		handles[0][0] = InterpF(tleft, tright, gTextures[gCurTexture].s1);
		handles[1][0] = InterpF(tleft, tright, gTextures[gCurTexture].s2);
		handles[2][0] = InterpF(tleft, tright, gTextures[gCurTexture].s2);
		handles[3][0] = InterpF(tleft, tright, gTextures[gCurTexture].s1);
		handles[0][1] = InterpF(tbottom, ttop, gTextures[gCurTexture].t2);
		handles[1][1] = InterpF(tbottom, ttop, gTextures[gCurTexture].t2);
		handles[2][1] = InterpF(tbottom, ttop, gTextures[gCurTexture].t1);
		handles[3][1] = InterpF(tbottom, ttop, gTextures[gCurTexture].t1);

		handles[4][0] = InterpF(tleft, tright, InterpF(gTextures[gCurTexture].s1, gTextures[gCurTexture].s2, 0.5));
		handles[5][0] = InterpF(tleft, tright, gTextures[gCurTexture].s2);
		handles[6][0] = InterpF(tleft, tright, InterpF(gTextures[gCurTexture].s1, gTextures[gCurTexture].s2, 0.5));
		handles[7][0] = InterpF(tleft, tright, gTextures[gCurTexture].s1);
		handles[4][1] = InterpF(tbottom, ttop, gTextures[gCurTexture].t2);
		handles[5][1] = InterpF(tbottom, ttop, InterpF(gTextures[gCurTexture].t1, gTextures[gCurTexture].t2, 0.5));
		handles[6][1] = InterpF(tbottom, ttop, gTextures[gCurTexture].t1);
		handles[7][1] = InterpF(tbottom, ttop, InterpF(gTextures[gCurTexture].t1, gTextures[gCurTexture].t2, 0.5));

		handles[8][0] = InterpF(tleft, tright, InterpF(gTextures[gCurTexture].s1, gTextures[gCurTexture].s2, 0.5));
		handles[8][1] = InterpF(tbottom, ttop, InterpF(gTextures[gCurTexture].t1, gTextures[gCurTexture].t2, 0.5));
	}
	
	switch(inMessage) {
	case xpMsg_Destroy:
		OE_Unregister(OE_TexEdNotify, inWidget);
		return 1;
	case xpMsg_Draw:
		{
			// First fill in the whole pane with black...better to see things against black.
			XPLMSetGraphicsState(0, 0, 0,   0, 0, 0, 0);
			glColor3f(0.0, 0.0, 0.0);
			glBegin(GL_QUADS);
			glVertex2i(left, top);
			glVertex2i(right, top);
			glVertex2i(right, bottom);
			glVertex2i(left, bottom);
			glEnd();
			
			// Now draw the texture if we have one.
			if (texID)
			{
				XPLMBindTexture2d(texID, 0);
				XPLMSetGraphicsState(0, 1, 0, 1, 1, 0, 0);				
			} else 
				XPLMSetGraphicsState(0, 0, 0,    0, 0, 0, 0);
						
			glColor3f(1.0, 1.0, 1.0);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 1.0);		glVertex2i(tleft, ttop);			
			glTexCoord2f(1.0, 1.0);		glVertex2i(tright, ttop);
			glTexCoord2f(1.0, 0.0);		glVertex2i(tright, tbottom);
			glTexCoord2f(0.0, 0.0);		glVertex2i(tleft, tbottom);
			glEnd();
			
			if (gCurTexture != -1)
			{
				// Draw the outline of the texture
				XPLMSetGraphicsState(0, 0, 0,  0, 0,   0, 0);
				glColor3f(1.0, 1.0, 1.0);
				glBegin(GL_LINE_LOOP);
				for (int n = 0; n < 4; ++n)
					glVertex2fv(handles[n]);
				glEnd();
				
				// Draw all 8 control handles
				glColor3f(0.5, 1.0, 1.0);
				glBegin(GL_QUADS);
				for (int n = 0; n < 9; ++n)
				{
					glVertex2f(handles[n][0] - kHandleRad, handles[n][1] + kHandleRad);
					glVertex2f(handles[n][0] + kHandleRad, handles[n][1] + kHandleRad);
					glVertex2f(handles[n][0] + kHandleRad, handles[n][1] - kHandleRad);
					glVertex2f(handles[n][0] - kHandleRad, handles[n][1] - kHandleRad);
				}
				glEnd();
			}			
		}
		return 1;
	case xpMsg_MouseDown:
	case xpMsg_MouseDrag:
	case xpMsg_MouseUp:
		{
			if (inMessage == xpMsg_MouseDown)
			{
				XPSetWidgetProperty(inWidget, kCurHandle, -1);

				for (int n = 0; n < 9; ++n)
				{
					if (((handles[n][0] - kHandleRad) < MOUSE_X(inParam1)) &&
						((handles[n][0] + kHandleRad) > MOUSE_X(inParam1)) &&
						((handles[n][1] - kHandleRad) < MOUSE_Y(inParam1)) &&
						((handles[n][1] + kHandleRad) > MOUSE_Y(inParam1)))
					{
						XPSetWidgetProperty(inWidget, kCurHandle, n);
						XPSetWidgetProperty(inWidget, kHandleSlopX, handles[n][0] - MOUSE_X(inParam1));
						XPSetWidgetProperty(inWidget, kHandleSlopY, handles[n][1] - MOUSE_Y(inParam1));
					}
				}
			}
			
			int curHandle = XPGetWidgetProperty(inWidget, kCurHandle, NULL);
			if (curHandle != -1)
			{
				int	new_handle_x = MOUSE_X(inParam1) + XPGetWidgetProperty(inWidget, kHandleSlopX, NULL);
				int	new_handle_y = MOUSE_Y(inParam1) + XPGetWidgetProperty(inWidget, kHandleSlopY, NULL);
				
				float s_loc = GetInterpF(tleft, tright, new_handle_x);
				float t_loc = GetInterpF(tbottom, ttop, new_handle_y);
				
				if (curHandle == 8)
				{
					float s_dif = s_loc - InterpF(gTextures[gCurTexture].s1, gTextures[gCurTexture].s2, 0.5);
					float t_dif = t_loc - InterpF(gTextures[gCurTexture].t1, gTextures[gCurTexture].t2, 0.5);
					
					gTextures[gCurTexture].s1 += s_dif;
					gTextures[gCurTexture].s2 += s_dif;
					gTextures[gCurTexture].t1 += t_dif;
					gTextures[gCurTexture].t2 += t_dif;
				} else {				
					if (kHandleWriteFlags[curHandle][0])	gTextures[gCurTexture].s1 = s_loc;
					if (kHandleWriteFlags[curHandle][1])	gTextures[gCurTexture].s2 = s_loc;
					if (kHandleWriteFlags[curHandle][2])	gTextures[gCurTexture].t1 = t_loc;
					if (kHandleWriteFlags[curHandle][3])	gTextures[gCurTexture].t2 = t_loc;
				}
								
				OE_Notify(catagory_Texture, msg_TexSelectionEdited, NULL);
			}						
			
			if (inMessage == xpMsg_MouseUp)
				XPSetWidgetProperty(inWidget, kCurHandle, -1);
		}
		return 1;
	default:
		return 0;
	}
}	                                   
