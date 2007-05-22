#ifndef WED_TERRASERVER_LAYER_H
#define WED_TERRASERVER_LAYER_H

#include "WED_MapLayer.h"
#include "GUI_Timer.h"

#define NUM_LEVELS 10

class AsyncImage;
class AsyncImageLocator;
class AsyncConnectionPool;

class	WED_TerraserverLayer : public WED_MapLayer, public GUI_Timer {
public:

						 WED_TerraserverLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver);
	virtual				~WED_TerraserverLayer();
	
			void		ToggleVis(void);
			bool		IsVis(void) { return mVis; }

	// These provide generalized drawing routines.  Use this to draw background images and other such stuff.
	virtual	void		DrawVisualization		(int inCurrent, GUI_GraphState * g);

	virtual	void		TimerFired(void);
	
private:

			const char *	ResString(int z);

	map<long long, AsyncImage*>		mImages[NUM_LEVELS];

	int mX1[NUM_LEVELS], mX2[NUM_LEVELS], mY1[NUM_LEVELS], mY2[NUM_LEVELS], mDomain[NUM_LEVELS], mHas[NUM_LEVELS];

	AsyncImageLocator *						mLocator[NUM_LEVELS];
	AsyncConnectionPool *					mPool;
	
	string									mData;
	int										mRes;
	bool									mVis;

};

#endif /* WED_TERRASERVER_LAYER_H */
