#ifndef WED_MAPPANE_H
#define WED_MAPPANE_H

#include "GUI_Pane.h"
#include "GUI_Listener.h"
#include "GUI_Packer.h"

class GUI_ToolBar;
class WED_Map;
class WED_MapToolNew;
class WED_MapLayer;
class GUI_Table;
class GUI_TextTable;
class WED_ToolInfoAdapter;

class	WED_ImageOverlayTool;
class	WED_TerraserverLayer;
class	WED_StructureLayer;
class	WED_WorldMapLayer;
//class	WED_TileServerLayer;

class	IResolver;
class	WED_Archive;
class	GUI_Commander;

/*

	Note: the map pane is _not_ a commander.  Commanders participate in focus in a direct chain -- that is, two commanders can't "share" focus.  But
	the map pane has to share focus with the property panes...that is, while keyboard focus is in the property pane, the map pane still has to be
	in the chain so that some of the menu items, like "show terrasever" work.  So...we develop a separate set of APIs and let the document window
	"shop around" keyboard and menu choices to everyone.

*/

class	WED_MapPane : public GUI_Packer, GUI_Listener {
public:
		
						 WED_MapPane(GUI_Commander * cmdr, double log_bounds[4], IResolver * resolver, WED_Archive * archive);
	virtual				~WED_MapPane();

			void		ZoomShowAll(void);
			
			GUI_Pane *	GetTopBar(void);
			
			int				Map_KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)	 	;
			int				Map_HandleCommand(int command) 									;
			int				Map_CanHandleCommand(int command, string& ioName, int& ioCheck) ;

	virtual	void	ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							int						inMsg,
							int						inParam);
			
private:
	
	WED_Map *				mMap;

	vector<WED_MapLayer *>	mLayers;
	vector<WED_MapToolNew *>mTools;
	
	WED_ImageOverlayTool *	mImageOverlay;
	WED_TerraserverLayer *	mTerraserver;
//	WED_TileServerLayer *	mTileserver;
	WED_StructureLayer *	mStructureLayer;
	WED_WorldMapLayer *		mWorldMap;

	GUI_ToolBar *			mToolbar;
	
	GUI_Table *						mTable;
	GUI_TextTable *					mTextTable;
	WED_ToolInfoAdapter *			mInfoAdapter;
	
	IResolver *				mResolver;

};


#endif
