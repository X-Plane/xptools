//
//  WED_ATCLayer.hpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 1/25/16.
//
//

#ifndef WED_ATCLayer_h
#define WED_ATCLayer_h

#include "WED_MapLayer.h"

class	WED_ATCLayer : public WED_MapLayer {
public:

						 WED_ATCLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_ATCLayer();

	virtual	bool		DrawEntityStructure		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected);
	virtual	void		GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks);

};

class WED_RampPosition;
void WED_ATCLayer_DrawAircraft(WED_RampPosition * pos, GUI_GraphState * g, WED_MapZoomerNew * z);

#endif /* WED_ATCLayer_h */
