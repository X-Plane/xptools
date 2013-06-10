/*
 * Copyright (c) 2008, Laminar Research.
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

#ifndef WED_LibraryListAdapter_H
#define WED_LibraryListAdapter_H

#include "GUI_TextTable.h"
#include "GUI_Broadcaster.h"
#include "GUI_SimpleTableGeometry.h"
#include "GUI_Listener.h"

class	WED_LibraryMgr;
class	WED_CreatePointTool;
class	WED_CreatePolygonTool;
class	WED_MapPane;
class	WED_LibraryPreviewPane;

class WED_LibraryListAdapter : public GUI_TextTableProvider, public GUI_SimpleTableGeometry, public GUI_TextTableHeaderProvider, public GUI_Broadcaster, public GUI_Listener {
public:
					 WED_LibraryListAdapter(WED_LibraryMgr * who);
	virtual			~WED_LibraryListAdapter();

			void	SetMap(WED_MapPane * amap, WED_LibraryPreviewPane * apreview);
			void	SetFilter(const string& filter);

	// GUI_TextTableProvider
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
						int							all);

	virtual	int		TabAdvance(
						int&						io_x,
						int&						io_y,
						int							reverse,
						GUI_CellContent&			the_content);
	virtual	int		DoubleClickCell(
						int							cell_x,
						int							cell_y);

	virtual	void					GetLegalDropOperations(
											int&						allow_between_col,
											int&						allow_between_row,
											int&						allow_into_cell) { allow_between_col = allow_between_row = allow_into_cell = 0; }
	virtual	GUI_DragOperation		CanDropIntoCell(
											int							cell_x,
											int							cell_y,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended,
											int&						whole_col,
											int&						whole_row) { return gui_Drag_None; }
	virtual	GUI_DragOperation		CanDropBetweenColumns(
											int							cell_x,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended) { return gui_Drag_None; }
	virtual	GUI_DragOperation		CanDropBetweenRows(
											int							cell_y,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended) { return gui_Drag_None; }


	virtual	GUI_DragOperation		DoDropIntoCell(
											int							cell_x,
											int							cell_y,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended) { return gui_Drag_None; }
	virtual	GUI_DragOperation		DoDropBetweenColumns(
											int							cell_x,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended) { return gui_Drag_None; }
	virtual	GUI_DragOperation		DoDropBetweenRows(
											int							cell_y,
											GUI_DragData *				drag,
											GUI_DragOperation			allowed,
											GUI_DragOperation			recommended) { return gui_Drag_None; }

	// GUI_[Simple]TableGeometry
	virtual	int		GetColCount(void);
	virtual	int		GetRowCount(void);

	// GUI_TextTableHeaderProvider
	virtual void	GetHeaderContent(
						int							cell_x,
						GUI_HeaderContent&			the_content);
	virtual	void	SelectHeaderCell(
						int							cell_x) { }

	// GUI_Listener
	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam);

private:

		int		IsOpen(const string& r);
		void	SetOpen(const string& r, int open);
		void	RebuildCache();
		void	RebuildCacheRecursive(const string& r);
		void	SetSel(const string& s);
			
		//Adds the correct prefix (for drawing)
		void	PrefixAdder(string& path);

		//Removes the correct prefix (for resource look up)
		void	PrefixStripper(string& path, int extra=0);

		//A hash map of open folders in the heirarchy
		hash_map<string,int>	mOpen;

		//A cache of all paths to be shown.
		vector<string>			mCache;
		bool					mCacheValid;
		
		//A string to switch library panes with
		//Possible values Local or Library, listed below;
		string					mCatChanger;
		string					mLocalStr;
		string					mLibraryStr;

		//Index of Local/ in mCache
		//int						mCatLocInd;
		//Index of Library/ in mCache
		//int					mCatLibInd;

		//A collection of strings for the filter to be checked against
		vector<string>			mFilter;

		WED_LibraryMgr *		mLibrary;
		string					mSel;

		WED_MapPane				* mMap;
		WED_LibraryPreviewPane	* mPreview;
};

#endif /* WED_LibraryListAdapter_H */
