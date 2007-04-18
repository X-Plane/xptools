#ifndef WED_TERRASERVER_LAYER_H
#define WED_TERRASERVER_LAYER_H

#include "WED_MapLayer.h"

class AsyncImage;
class AsyncImageLocator;

class	WED_TerraserverLayer : public WED_MapLayer {
public:

						 WED_TerraserverLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_TerraserverLayer();

	// These provide generalized drawing routines.  Use this to draw background images and other such stuff.
	virtual	void		DrawVisualization		(int inCurrent, GUI_GraphState * g);
	
private:

			const char *	ResString(void);

	map<long long, AsyncImage*>		mImages;

	int mX1, mX2, mY1, mY2, mDomain, mHas;

	AsyncImageLocator *				mLocator;
	
	string							mData;
	int								mRes;


};

#endif /* WED_TERRASERVER_LAYER_H */
