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

#ifndef WED_MAPPANE_H
#define WED_MAPPANE_H

#include "GUI_Pane.h"
#include "GUI_Listener.h"
#include "GUI_Packer.h"

#include <stdint.h>

class GUI_ToolBar;
class WED_Map;
class WED_MapToolNew;
class WED_MapLayer;
class GUI_Table;
class GUI_TextTable;
class WED_ToolInfoAdapter;

#if WANT_TERRASEVER
class	WED_TerraserverLayer;
#endif
class	WED_StructureLayer;
class	WED_PreviewLayer;
class	WED_WorldMapLayer;
//class	WED_TileServerLayer;

class	WED_CreatePointTool;
class	WED_CreatePolygonTool;
class	IResolver;
class	IDocPrefs;
class	WED_Archive;
class	GUI_Commander;
class	WED_LibraryListAdapter;
class	WED_OSMSlippyMap;
#if WITHNWLINK
class	WED_NWInfoLayer;
#endif
/*

	Note: the map pane is _not_ a commander.  Commanders participate in focus in a direct chain -- that is, two commanders can't "share" focus.  But
	the map pane has to share focus with the property panes...that is, while keyboard focus is in the property pane, the map pane still has to be
	in the chain so that some of the menu items, like "show terrasever" work.  So...we develop a separate set of APIs and let the document window
	"shop around" keyboard and menu choices to everyone.

*/

class	WED_MapPane : public GUI_Packer, GUI_Listener {
public:

						 WED_MapPane(GUI_Commander * cmdr, double log_bounds[4], IResolver * resolver, WED_Archive * archive, WED_LibraryListAdapter * library);
	virtual				~WED_MapPane();

			void		ZoomShowAll(void);
			void		ZoomShowSel(void);

			void		SetResource(const string& r, int res_type);

			GUI_Pane *	GetTopBar(void);

			int				Map_KeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)	 	;
			int				Map_HandleCommand(int command) 									;
			int				Map_CanHandleCommand(int command, string& ioName, int& ioCheck) ;

			void			FromPrefs(IDocPrefs * prefs);
			void			ToPrefs(IDocPrefs * prefs);

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam);

private:

	WED_Map *				mMap;

	vector<WED_MapLayer *>	mLayers;
	vector<WED_MapToolNew *>mTools;

#if WANT_TERRASEVER
	WED_TerraserverLayer *	mTerraserver;
#endif	
	WED_OSMSlippyMap *		mOSMSlippyMap;
//	WED_TileServerLayer *	mTileserver;
	WED_StructureLayer *	mStructureLayer;
	WED_PreviewLayer *		mPreview;
	WED_WorldMapLayer *		mWorldMap;
#if WITHNWLINK
	WED_NWInfoLayer *		mNWInfoLayer;
#endif
	GUI_ToolBar *			mToolbar;

	GUI_Table *						mTable;
	GUI_TextTable *					mTextTable;
	WED_ToolInfoAdapter *			mInfoAdapter;

	IResolver *				mResolver;

	WED_CreatePointTool *	mObjTool;
	WED_CreatePolygonTool *	mFacTool;
	WED_CreatePolygonTool * mFstTool;
	WED_CreatePolygonTool * mStrTool;
	WED_CreatePolygonTool * mLinTool;
	WED_CreatePolygonTool * mPolTool;

};


#endif
