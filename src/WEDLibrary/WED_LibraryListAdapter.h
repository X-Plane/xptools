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
class	GUI_AdvancedFilterPane;

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
	
	//Runs the filter based on a number of parameters
	void			DoFilter(/*vector<strings> sources, vector<strings> types, string with, string without*/);

private:
		/* What is a prefix and how do I use it? A guide about how to use
		* mCache and mOpen with all their related methods with the new prefix system.
		*
		* Keywords: mOpen, mCache, Library Pane, Library List Adapter, Library Manager, 
		* Catagories, Prefixes, Virtual Paths, Real Paths
		*
		* Intro: What is a prefix?
		*	To try and split up the Library Pane into Local Files and Library Files for
		* useability and readability sake the program added on a prefix of whatever is in
		* mLocalStr or mLibraryStr (most likely "Local/" or "Library/" repectively.) For example
		* LocalObjects/things/stuff.obj becomes Loc/LocalObjects/things/stuff.obj
		* 
		* Where is this used?
		*	Everywhere until it is not allowed to be used. Because the prefixes are added in
		* RebuildCache and RebuildCacheRecusive (the cores of this class) they are used in
		* all other methods until data is needed from the library manager or other special clauses.
		* It is especially used in drawing.
		*
		* How do I transform the strings and get/set their data? Also when should I do that?
		*	 
		*	Good news is that it is mostly done for you! In Rebuild Cache it produces all the strings
		* with the prefix attached and calculates the position of what indexs in the Vector mCache
		* are where mLocalStr and mLibraryStr are. Instead of saying string path= mCache[index] it is
		* best to use the method GetNthCacheIndex. GetNth, for short, handles the adding and removing
		* of the prefix data for you. Just pass in the index and whether or not you want the prefix
		* and it will return a string for you*
		*
		*	It is extremely recommended that you DO NOT change mCache itself or handle getting that data yourself
		* because the system is very tightly wound up with proper placement of /'s and careful adding and removing
		* of charecters. Only do this if you are very comfortable with how this class wide system works and
		* you undo your change at the end of your process
		*
		* *If you pass in mLocalStr and mLibraryStr it will give you back their vaule minus the '/'. For example
		* Local/ becomes Local. This is mainly used for drawing.
		*
		*
		* A table of how strings appear and move through the program
		* Assume mLocalStr and mLibraryStr are equal to "Loc/" and "Lib/"
		* 
		*String|_Loc/ or Lib/_         | Loc or Lib         | Buildings/FoodStands/RustBurger.obj | Loc/Buildings/FoodStands/RustBurger.obj|
		* Use  | mOpen,mCache,mSel     | Drawing            | Library Manager, reasource lookup   | Drawing, mOpen, mCache, mSel           |
		* Get  | mCatLocInd, mCatLibInd| GetNth(index, true)| GetNth(index, true)                 | GetNth(index, false)                   |
		* Set  | RebuildCache()        | Constructor        | LibraryManager(from your HDD)       | Manually change string (Danger)        |
		*
		* Common trouble shooting tip: If something is not working it means you have added/not added the prefix propperly, 
		* forgot it, or forgot to reset it.
		*/
		int		IsOpen(const string& r);
		void	SetOpen(const string& r, int open);
		void	RebuildCache();

		/*Recusively builds the cache,
		*pass in a file path and a catagory string and packType (enum),
		*and a prefix
		*/
		void	RebuildCacheRecursive(const string& r, int packType, const string& prefix);
		void	SetSel(const string& s, const string& noPrefix);
		string GetNthCacheIndex (int index, bool noPrefix);

		#if DEV
		void PrintMOpen(string path="");
		#endif
		//A hash map of open folders in the heirarchy,
		//Uses the prefix system!
		hash_map<string,int>	mOpen;

		//A cache of all paths to be shown.
		vector<string>			mCache;
		
		bool					mCacheValid;
		
		//A string to switch library panes with
		//Possible values Local or Library, listed below;
		string					mLocalStr;
		string					mLibraryStr;

		//Index of Local/ in mCache
		int						mCatLocInd;
		//Index of Library/ in mCache
		int						mCatLibInd;

		//A collection of strings for the filter to be checked against
		vector<string>			mFilter;

		
		WED_LibraryMgr *		mLibrary;
		string					mSel;

		WED_MapPane				* mMap;
		WED_LibraryPreviewPane	* mPreview;
};

#endif /* WED_LibraryListAdapter_H */
