#ifndef WED_STRUCTURELAYER_H
#define WED_STRUCTURELAYER_H

#include "WED_MapLayer.h"

class	WED_StructureLayer : public WED_MapLayer {
public:

						 WED_StructureLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_StructureLayer();

	virtual	void		DrawEntityStructure		(int inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected);
	
};

#endif