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
#include "OE_Scroller.h"
#include "XPStandardWidgets.h"
#include "XPWidgets.h"
#include "XPWidgetUtils.h"
#include "XPUIGraphics.h"

const	int	kScrollBarWidth = 10;

OE_Scroller::OE_Scroller(
               int                  inLeft,    
               int                  inTop,    
               int                  inRight,    
               int                  inBottom,    
               int                  inVisible,
               OE_Pane *			inSuper,
               bool					inHScroll,
               bool					inVScroll) :
      OE_Pane(inLeft, inTop, inRight, inBottom, inVisible, "", inSuper)
{
	mHScroll = inHScroll ? new OE_Pane(inLeft, inBottom + kScrollBarWidth,
				inRight - (inVScroll ? kScrollBarWidth : 0), inBottom,
				1, "", this, xpWidgetClass_ScrollBar) : NULL;
	mVScroll = inVScroll ? new OE_Pane(inRight- kScrollBarWidth, inTop,
				inRight, inBottom + (inHScroll ? kScrollBarWidth : 0),
				1, "", this, xpWidgetClass_ScrollBar) : NULL;
				
	mClipper = new OE_Pane(inLeft, inTop, inRight - (inVScroll ? kScrollBarWidth : 0),
			inBottom + (inHScroll ? kScrollBarWidth : 0),
			1, "", this);
}
               
OE_Scroller::~OE_Scroller()
{
	delete mHScroll;
	delete mVScroll;
	delete mClipper;
}
	
void	OE_Scroller::SetContents(
									OE_Pane * 			inContents)
{
	mContents = inContents;
	XPPlaceWidgetWithin(inContents->GetWidget(), mClipper->GetWidget());
	RecomputeScrollbars();
}

void	OE_Scroller::RecomputeScrollbars()
{
	if (!mClipper || !mContents)	return;
	
	int	l,r,t,b,l1,r1,t1,b1;
	XPGetWidgetGeometry(mClipper->GetWidget(), &l1, &t1, &r1, &b1);
	XPGetWidgetGeometry(mContents->GetWidget(), &l, &t, &r, &b);
	int	w = r - l;
	int h = t - b;
	int w1 = r1 - l1;
	int h1 = t1 - b1;
	
	int	hMin = 0;
	int hMax = mHScroll ? w - w1 : 0;
	if (hMax < hMin) hMax = hMin;
	int hPos = l1 - l;
	if (hPos < hMin) hPos = hMin;
	if (hPos > hMax) hPos = hMax;
	
	int vMin = 0;
	int vMax = mVScroll? h - h1 : 0;
	if (vMax < vMin) vMax = vMin;
	int vPos = t - t1;
	if (vPos < vMin) vPos = vMin;
	if (vPos > vMax) vPos = vMax;
	
	if (mHScroll)
	{
		XPSetWidgetProperty(mHScroll->GetWidget(), xpProperty_ScrollBarMin, hMin);
		XPSetWidgetProperty(mHScroll->GetWidget(), xpProperty_ScrollBarMax, hMax);
		XPSetWidgetProperty(mHScroll->GetWidget(), xpProperty_ScrollBarSliderPosition, hPos);
	}		

	if (mVScroll)
	{
		XPSetWidgetProperty(mVScroll->GetWidget(), xpProperty_ScrollBarMin, vMin);
		XPSetWidgetProperty(mVScroll->GetWidget(), xpProperty_ScrollBarMax, vMax);
		XPSetWidgetProperty(mVScroll->GetWidget(), xpProperty_ScrollBarSliderPosition, vMax - vPos);
	}		
	
	XPSetWidgetGeometry(mContents->GetWidget(),
		l1 - hPos,	t1 + vPos,
		l1 - hPos + w, t1 + vPos - h);		
}
					
int		OE_Scroller::MessageFunc(
                                   XPWidgetMessage      inMessage,    
                                   long                 inParam1,    
                                   long                 inParam2)
{
	int	l,t,r,b;
	int	l1,t1,r1,b1;
	if (mContents == NULL)	return 0;
	switch(inMessage) {
	case xpMsg_ScrollBarSliderPositionChanged:
		if ((mHScroll && inParam1 == (long) mHScroll->GetWidget()) ||
			(mVScroll && inParam1 == (long) mVScroll->GetWidget()))
		{
			XPGetWidgetGeometry(mContents->GetWidget(), &l,&t,&r,&b);
			XPGetWidgetGeometry(mClipper->GetWidget(), &l1, &t1, &r1, &b1);
			int	w = r - l;
			int h = t - b;
			int	hScroll = mHScroll ? XPGetWidgetProperty(mHScroll->GetWidget(), xpProperty_ScrollBarSliderPosition, NULL) : 0;
			int	vScroll = mVScroll ? (XPGetWidgetProperty(mVScroll->GetWidget(), xpProperty_ScrollBarMax, NULL) - XPGetWidgetProperty(mVScroll->GetWidget(), xpProperty_ScrollBarSliderPosition, NULL)) : 0;
			XPSetWidgetGeometry(mContents->GetWidget(), l1 - hScroll, t1 + vScroll, l1 - hScroll + w, t1 + vScroll - h);
		}
		break;
	case xpMsg_Reshape:
		if ((inParam1 == (long) mWidget) || 
			(mContents && inParam1 == (long) mContents->GetWidget()))
		{
			if (DELTA_X(inParam2) || DELTA_Y(inParam2) || DELTA_H(inParam2) || DELTA_W(inParam2))
				RecomputeScrollbars();
		}
		return 1;
	}
	return OE_Pane::MessageFunc(inMessage, inParam1, inParam2);
}                                   

void		OE_Scroller::DrawSelf(void)
{
	int	l, r, t, b;

	XPGetWidgetGeometry(mWidget, &l, &t, &r, &b);
	if ((l == r) || (t == b)) 
		return;
	
	XPDrawWindow(l, b, r, t, xpWindow_Screen);	
}
