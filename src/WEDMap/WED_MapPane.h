#ifndef WED_MAPPANE_H
#define WED_MAPPANE_H

#include "WED_Map.h"
#include "GUI_Pane.h"
#include "GUI_Packer.h"
#include "WED_MapBkgnd.h"
#include "WED_MarqueeTool.h"
#include "WED_CreatePolygonTool.h"

class	IResolver;

class	WED_MapPane : public GUI_Packer {
public:
		
						 WED_MapPane(double log_bounds[4], IResolver * resolver, GUI_Broadcaster * archive_broadcaster);
	virtual				~WED_MapPane();

			void		ZoomShowAll(void);
private:
	
	WED_Map					mMap;
	WED_MapBkgnd			mBkgnd;
	WED_MarqueeTool			mMarquee;
	WED_CreatePolygonTool	mCreatePoly;
};


#endif
