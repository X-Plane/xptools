#ifndef WED_STRUCTURELAYER_H
#define WED_STRUCTURELAYER_H

#include "WED_MapLayer.h"

class	WED_StructureLayer : public WED_MapLayer {
public:

						 WED_StructureLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_StructureLayer();


			bool		GetRealLinesShowing(void) const;
			void		SetRealLinesShowing(bool show);
			void		SetPavementTransparency(float alpha);
			float		GetPavementTransparency(void) const;

	virtual	void		DrawEntityStructure		(int inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected);

private:

	bool	mRealLines;
	float	mPavementAlpha;

};

#endif