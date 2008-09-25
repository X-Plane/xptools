/*
 * Copyright (c) 2007, Laminar Research.
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

#ifndef GUI_SIMPLETABLEGEOMETRY_H
#define GUI_SIMPLETABLEGEOMETRY_H

#include "GUI_Table.h"

class	GUI_SimpleTableGeometry : public GUI_TableGeometry {
public:

						 GUI_SimpleTableGeometry(
										int		num_cols,
										int *	default_col_widths,
										int		row_height);

	virtual				~GUI_SimpleTableGeometry();

//	virtual	int			GetColCount(void)=0;
//	virtual	int			GetRowCount(void)=0;

	virtual	int			GetCellLeft (int n);
	virtual	int			GetCellRight(int n);
	virtual	int			GetCellWidth(int n);

	virtual	int			GetCellBottom(int n);
	virtual	int			GetCellTop	 (int n);
	virtual	int			GetCellHeight(int n);

	virtual	int			ColForX(int n);
	virtual	int			RowForY(int n);

	virtual	bool		CanSetCellWidth (void) const;
	virtual	bool		CanSetCellHeight(void) const;
	virtual	void		SetCellWidth (int n, int w);
	virtual	void		SetCellHeight(int n, int h);

private:

			void		ExtendTo(int x);

			int			mRowHeight;
			vector<int>	mCols;
};

#endif
