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

class	IResolver;
class	WED_Archive;
class	GUI_Commander;

class	WED_MapPane : public GUI_Packer, GUI_Listener {
public:
		
						 WED_MapPane(GUI_Commander * cmdr, double log_bounds[4], IResolver * resolver, WED_Archive * archive);
	virtual				~WED_MapPane();

			void		ZoomShowAll(void);
			
			#if !DEV
				doc why this is NOT just a gui_commander derivative
			#endif
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

	GUI_ToolBar *			mToolbar;
	
	GUI_Table *						mTable;
	GUI_TextTable *					mTextTable;
	WED_ToolInfoAdapter *			mInfoAdapter;

};


#endif
