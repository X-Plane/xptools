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

#ifndef WED_PackageListAdapter_H
#define WED_PackageListAdapter_H

#include "GUI_TextTable.h"
#include "GUI_Broadcaster.h"
#include "GUI_Listener.h"

class WED_PackageListAdapter : public GUI_TextTableProvider, public GUI_TableGeometry, public GUI_Broadcaster, public GUI_Listener {
public:

					 WED_PackageListAdapter();
	virtual			~WED_PackageListAdapter();

			bool	HasSelection(void);
			int		GetSelection(string * package);
			void	CreateBlankNew(void);
			void	SelectPackage(int n);
			void	LockPackage(const string& name);
			void	UnlockPackage(const string& name);
	virtual void	GetCellContent(
						int							cell_x, 
						int							cell_y, 
						GUI_CellContent&			the_content);	
	virtual	void	GetEnumDictionary(
						int							cell_x, 
						int							cell_y, 
						GUI_EnumDictionary&			out_dictionary);
	virtual	void	AcceptEdit(
						int							cell_x,
						int							cell_y,
						const GUI_CellContent&		the_content,
						int							apply_all);
	virtual	void	ToggleDisclose(
						int							cell_x,
						int							cell_y);
	virtual	void	DoDrag(
						GUI_Pane *					drag_emitter,
						int							mouse_x,
						int							mouse_y,
						int							bounds[4]);

	virtual void	SelectionStart(
						int							clear);
	virtual	int		SelectGetExtent(
						int&						low_x,
						int&						low_y,
						int&						high_x,
						int&						high_y);
	virtual	int		SelectGetLimits(
						int&						low_x,
						int&						low_y,
						int&						high_x,
						int&						high_y);
	virtual	void	SelectRange(
						int							start_x,
						int							start_y,
						int							end_x,
						int							end_y,
						int							is_toggle);
	virtual	void	SelectionEnd(void);
	virtual	int		SelectDisclose(
						int							open_it,
						int							all);		// return true if you support this op.

	virtual	int		TabAdvance(
						int&						io_x,
						int&						io_y,
						int							reverse,
						GUI_CellContent&			the_content);

	virtual	void					GetLegalDropOperations(
											int&						allow_between_col,
											int&						allow_between_row,
											int&						allow_into_cell);						
	virtual	GUI_DragOperation		CanDropIntoCell(
											int							cell_x,
											int							cell_y,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended,
											int&						whole_col,
											int&						whole_row);
	virtual	GUI_DragOperation		CanDropBetweenColumns(
											int							cell_x,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended);
	virtual	GUI_DragOperation		CanDropBetweenRows(
											int							cell_y,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended);


	virtual	GUI_DragOperation		DoDropIntoCell(
											int							cell_x,
											int							cell_y,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended);
	virtual	GUI_DragOperation		DoDropBetweenColumns(
											int							cell_x,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended);
	virtual	GUI_DragOperation		DoDropBetweenRows(
											int							cell_y,
											GUI_DragData *				drag, 
											GUI_DragOperation			allowed, 
											GUI_DragOperation			recommended);

	virtual	int			GetColCount(void);
	virtual	int			GetRowCount(void);
	
	virtual	int			GetCellLeft (int n);
	virtual	int			GetCellRight(int n);
	virtual	int			GetCellWidth(int n);

	virtual	int			GetCellBottom(int n);
	virtual	int			GetCellTop	 (int n);
	virtual	int			GetCellHeight(int n);
	
	// Index
	virtual	int			ColForX(int n);
	virtual	int			RowForY(int n);
	
	// Setting geometry
	virtual	bool		CanSetCellWidth (void) const;
	virtual	bool		CanSetCellHeight(void) const;
	virtual	void		SetCellWidth (int n, int w);
	virtual	void		SetCellHeight(int n, int h);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);

private:

	set<string>	mLock;
	int			mSel;

};

#endif /* WED_PackageListAdapter_H */
