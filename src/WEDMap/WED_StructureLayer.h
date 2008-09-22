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
			bool		GetVerticesShowing(void) const;
			void		SetVerticesShowing(bool show);

	virtual	bool		DrawEntityStructure		(intptr_t inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected);
	virtual	void		DrawStructure			(intptr_t inCurrent, GUI_GraphState * g);
	virtual	void		GetCaps(intptr_t& draw_ent_v, intptr_t& draw_ent_s, intptr_t& cares_about_sel);

private:

	bool							mRealLines;
	float							mPavementAlpha;
	bool							mVertices;

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
