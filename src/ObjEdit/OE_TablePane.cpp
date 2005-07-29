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
#include "OE_TablePane.h"
#include "XPWidgets.h"

OE_TablePane::OE_TablePane(
                   int                  inLeft,    
                   int                  inTop,    
                   int                  inRight,    
                   int                  inBottom,    
                   int                  inVisible,
                   OE_Pane *			inSuper) :
	OE_Pane(inLeft, inTop, inRight, inBottom, inVisible, "", inSuper)
{
	XPSetWidgetProperty(mWidget,xpProperty_Clip,1);
}	

OE_TablePane::~OE_TablePane()
{
}

	
void	OE_TablePane::GetCellDimensions(
					int			inRow,
					int			inColumn,
					int *		outLeft,
					int *		outTop,
					int *		outRight,
					int *		outBottom)
{
	if (outLeft)
		*outLeft = inColumn ? GetColRight(inColumn - 1) : 0;
	if (outRight)
		*outRight = GetColRight(inColumn);
	if (outTop)
		*outTop = inRow ? GetRowBottom(inRow - 1) : 0;
	if (outBottom)
		*outBottom = GetRowBottom(inRow);
}					
	
void	OE_TablePane::SnapToCols(void)
{
	int	rows = GetRowCount();
	int cols = GetColCount();
	int	width = GetColRight(cols-1);
	int height = GetRowBottom(rows-1);
	
	int	l,b,r,t;
	XPGetWidgetGeometry(mWidget,&l,&t,&r,&b);
	if ((width != (r-l))  || (height != (t-b)))
		XPSetWidgetGeometry(mWidget,l,t,l+width,t-height);
}
	
void	OE_TablePane::DrawSelf(void)
{
	int	l,t,r,b;
	XPGetWidgetGeometry(mWidget, &l,&t,&r,&b);
	int rows = GetRowCount();
	int	cols = GetColCount();
	for (int row = 0; row < rows; ++row)
	for (int col = 0; col < cols; ++col)
	{
		int	cl,cr,ct,cb;
		GetCellDimensions(row,col,&cl,&ct,&cr,&cb);
		cl += l;
		cr += l;
		ct = t - ct;
		cb = t - cb;
		
		if (cl < r && cr > l &&
			cb < t && ct > b)
		{
			DrawCell(row, col,cl,ct,cr,cb);
		}
	}
}

int		OE_TablePane::HandleClick(XPLMMouseStatus status, int x, int y, int button)
{
	int	l,t,r,b;
	XPGetWidgetGeometry(mWidget, &l,&t,&r,&b);
	int rows = GetRowCount();
	int	cols = GetColCount();
	for (int row = 0; row < rows; ++row)
	for (int col = 0; col < cols; ++col)
	{
		int	cl,cr,ct,cb;
		GetCellDimensions(row,col,&cl,&ct,&cr,&cb);
		cl += l;
		cr += l;
		ct = t - ct;
		cb = t - cb;

		if (x >= cl && x < cr && y > cb && y <= ct)
		{
			ClickCell(row, col,cl,ct,cr,cb);
		}
	}
	return 1;
}
