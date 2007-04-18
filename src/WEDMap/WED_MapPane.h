#ifndef WED_MAPPANE_H
#define WED_MAPPANE_H

#include "WED_Map.h"
#include "GUI_Pane.h"
#include "GUI_Packer.h"
#include "WED_MapBkgnd.h"
#include "WED_MarqueeTool.h"
#include "WED_CreatePolygonTool.h"
#include "WED_StructureLayer.h"
#include "WED_ImageOverlayTool.h"
#include "WED_TerraserverLayer.h"

class	IResolver;
class	WED_Archive;

class	WED_MapPane : public GUI_Packer {
public:
		
						 WED_MapPane(double log_bounds[4], IResolver * resolver, WED_Archive * archive);
	virtual				~WED_MapPane();

			void		ZoomShowAll(void);
			
			#if !DEV
				doc why this is NOT just a gui_commander derivative
			#endif
			int				Map_KeyPress(char inKey, int inVK, GUI_KeyFlags inFlags)	 	;
			int				Map_HandleCommand(int command) 									;
			int				Map_CanHandleCommand(int command, string& ioName, int& ioCheck) ;
			
private:
	
	WED_Map					mMap;
	WED_MapBkgnd			mBkgnd;
	WED_StructureLayer		mStructure;
	WED_ImageOverlayTool	mImageOverlay;
	WED_TerraserverLayer	mTerraserver;
	WED_MarqueeTool			mMarquee;
	WED_CreatePolygonTool	mCreatePoly;
};


#endif
