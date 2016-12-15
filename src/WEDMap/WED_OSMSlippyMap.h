//
//  WED_OSMSlippyMap.hpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 12/18/15.
//
//

#ifndef WED_OSMSlippyMap_h
#define WED_OSMSlippyMap_h

#define OSM_ZOOM_LEVELS 19

class	WED_file_cache_request;

#include "GUI_Timer.h"
#include "WED_MapLayer.h"

class	WED_OSMSlippyMap : public WED_MapLayer, public GUI_Timer {
public:

					 WED_OSMSlippyMap(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual			~WED_OSMSlippyMap();
	
	virtual	void	DrawVisualization(bool inCurrent, GUI_GraphState * g);
	virtual	void	GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks);
	virtual	void	TimerFired(void);

private:

			void	finish_loading_tile();

	WED_file_cache_request* m_cache_request;

	//The texture cache, where they key is the tile texture path on disk and the value is the texture id
	map<string,int>				m_cache;

};

#endif /* WED_OSMSlippyMap_h */
