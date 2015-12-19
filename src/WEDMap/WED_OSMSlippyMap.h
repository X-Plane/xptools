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

class	curl_http_get_file;

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

	vector<char>				m_buffer;
	curl_http_get_file *		m_req;
	string						m_req_path;
	map<string,int>				m_cache;

};

#endif /* WED_OSMSlippyMap_h */
