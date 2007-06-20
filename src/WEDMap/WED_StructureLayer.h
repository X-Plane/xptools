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

	virtual	bool		DrawEntityStructure		(int inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected);
	virtual	void		DrawStructure			(int inCurrent, GUI_GraphState * g);
	virtual	void		GetCaps(int& draw_ent_v, int& draw_ent_s, int& cares_about_sel);

private:

	bool							mRealLines;
	float							mPavementAlpha;

	vector<int>			mAirportIconsX;
	vector<int>			mAirportIconsY;
	vector<float>		mAirportIconsC;
	vector<int>			mSeaportIconsX;
	vector<int>			mSeaportIconsY;
	vector<float>		mSeaportIconsC;
	vector<int>			mHeliportIconsX;
	vector<int>			mHeliportIconsY;
	vector<float>		mHeliportIconsC;
};

#endif