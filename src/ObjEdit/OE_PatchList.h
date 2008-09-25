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
#ifndef OE_PATCHLIST_H
#define OE_PATCHLIST_H

#include "OE_TablePane.h"
#include "OE_Notify.h"

class	OE_Scroller;

class	OE_PatchTable : public OE_TablePane, OE_Notifiable {
public:

				OE_PatchTable(
                                   int                  inLeft,
                                   int                  inTop,
	                               OE_Pane *			inSuper);
	virtual		~OE_PatchTable();

	virtual	void	DrawCell(int row, int col, int cellLeft, int cellTop, int cellRight, int cellBottom);
	virtual	void	ClickCell(int row, int col, int cellLeft, int cellTop, int cellRight, int cellBottom);

	virtual	int		GetColCount(void);
	virtual	int		GetRowCount(void);
	virtual	int		GetColRight(int col);
	virtual	int		GetRowBottom(int col);

	virtual	void	HandleNotification(int catagory, int message, void * param);

private:

			void	RecalibrateSize(void);

};

class	OE_PatchList : public OE_Pane {
public:

					OE_PatchList(
                                   	int                  inLeft,
                                   	int                  inTop,
                                   	int                  inRight,
                                   	int                  inBottom);
	virtual			~OE_PatchList();

private:

		OE_Scroller *		mScroller;
		OE_PatchTable *		mPane;

};


#endif
