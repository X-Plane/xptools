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
#ifndef WED_PANE_H
#define WED_PANE_H

/*

	A note on handled messages:
	
	Drawing is passed on to allow WED_Pane to wrap existing drawing widget code.
	However, you cannot adorn by drawing the root class, so you cannot composite
	this way; either draw yourself or let the superclass draw.   (This is a general
	limitation of widget subclassing that can only be overcome via a child widget.)
	
	Keys and clicks must be specifically handled or not handled.
	
*/

#include "XPWidgetDefs.h"
#include "XPLMDisplay.h"

class	WED_Pane {
public:

					WED_Pane(
                                   int                  inLeft,    
                                   int                  inTop,    
                                   int                  inRight,    
                                   int                  inBottom,    
                                   int                  inVisible,
                                   const char *         inDescriptor,
                                   WED_Pane *			inSuper);

					WED_Pane(
                                   int                  inLeft,    
                                   int                  inTop,    
                                   int                  inRight,    
                                   int                  inBottom,    
                                   int                  inVisible,
                                   const char *         inDescriptor,
                                   WED_Pane *			inSuper,
                                   XPWidgetClass		inClass);

					WED_Pane(
                                   int                  inLeft,    
                                   int                  inTop,    
                                   int                  inRight,    
                                   int                  inBottom,    
                                   int                  inVisible,
                                   const char *         inDescriptor,
                                   WED_Pane *			inSuper,
                                   XPWidgetFunc_t		inFunc);


	virtual			~WED_Pane();

	static	WED_Pane *	GetPaneObj(XPWidgetID inWidget);
			XPWidgetID	GetWidget(void);

			void	Kill(bool inRecursive);
			
			void	Show(bool inVisible);
			bool	IsVisible(void) const;

	virtual	void	DrawSelf(void);
	virtual	int		HandleClick(XPLMMouseStatus status, int x, int y, int button);
	virtual	int		HandleKey(char key, XPLMKeyFlags flags, char vkey);	
	virtual	int		HandleMouseWheel(int x, int y, int direction);

	virtual	int		MessageFunc(
                                   XPWidgetMessage      inMessage,    
                                   long                 inParam1,    
                                   long                 inParam2);    

protected:

	static	int		StaticMessageFunc(
                                   XPWidgetMessage      inMessage,    
                                   XPWidgetID           inWidget,    
                                   long                 inParam1,    
                                   long                 inParam2);    

	XPWidgetID		mWidget;
	
};	

#endif
